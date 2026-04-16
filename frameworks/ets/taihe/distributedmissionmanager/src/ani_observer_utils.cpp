/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "ani_observer_utils.h"
#include <algorithm>
#include <endian.h>

#include "hilog_tag_wrapper.h"

namespace ani_observerutils {

std::shared_ptr<OHOS::AppExecFwk::EventHandler> JsBaseObserver::mainHandler_;
static std::once_flag g_handlerOnceFlag;

JsBaseObserver::JsBaseObserver()
{
}

bool JsBaseObserver::SendEventToMainThread(const std::function<void()> func)
{
    if (func == nullptr) {
        return false;
    }
    std::call_once(g_handlerOnceFlag, [] {
        auto runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        if (runner) {
            mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
        }
    });
    if (!mainHandler_) {
        TAG_LOGI(AAFwkTag::MISSION, "Failed to initialize event handler");
        return false;
    }
    mainHandler_->PostTask(func, "", 0, OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});
    return true;
}

MissionObserver::MissionObserver()
{
    TAG_LOGI(AAFwkTag::MISSION, "DataObserver");
}

MissionObserver::~MissionObserver()
{
    TAG_LOGI(AAFwkTag::MISSION, "~DataObserver");
}

bool MissionObserver::AddCallback(JsOnCallbackType cb)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    for (auto &item : callbackList_) {
        if (item == cb) {
            TAG_LOGI(AAFwkTag::MISSION, "AddCallback duplicated");
            return false;
        }
    }
    callbackList_.push_back(cb);
    return true;
}

void MissionObserver::DeleteCallback(::taihe::optional_view<JsOnCallbackType> optCallback)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!optCallback.has_value()) {
        callbackList_.clear();
        TAG_LOGI(AAFwkTag::MISSION, "DeleteCallback, clear");
        return;
    }
    auto paramCallback = optCallback.value();
    for (auto iter = callbackList_.begin(); iter != callbackList_.end();) {
        if (paramCallback == *iter) {
            iter = callbackList_.erase(iter);
            TAG_LOGI(AAFwkTag::MISSION, "DeleteCallback, remove item");
        } else {
            ++iter;
        }
    }
}

bool MissionObserver::IsCallbackListEmpty()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return callbackList_.size() == 0;
}

void MissionObserver::OnCallback(const AAFwk::OnCallbackInfo &info)
{
    TAG_LOGI(AAFwkTag::MISSION, "MissionObserver::OnCallback");
    auto taiheState = ohos::distributedmissionmanager::ContinueState::from_value(info.continueState);
    ::ContinuableInfo::ContinuableInfo taiheInfo = {};
    taiheInfo.srcDeviceId = info.srcDeviceId;
    taiheInfo.bundleName = info.bundleName;
    taiheInfo.srcBundleName = ::taihe::optional<::taihe::string>::make(info.srcBundleName);
    taiheInfo.continueType = ::taihe::optional<::taihe::string>::make(info.continueType);

    // Convert appIdentifiers vector to taihe array
    std::vector<::taihe::string> appIdentifiersVector;
    for (const auto &appIdentifier : info.appIdentifiers) {
        appIdentifiersVector.push_back(::taihe::string(appIdentifier));
    }
    ::taihe::array<::taihe::string> appIdentifiersArray(appIdentifiersVector);
    taiheInfo.appIdentifiers = ::taihe::optional<::taihe::array<::taihe::string>>::make(appIdentifiersArray);

    ::ohos::distributedmissionmanager::ContinueCallbackInfo callbackInfo = { .state = taiheState, .info = taiheInfo };

    wptr<MissionObserver> weakthis = this;
    SendEventToMainThread([callbackInfo, weakthis] {
        auto sptrthis = weakthis.promote();
        if (sptrthis != nullptr) {
            sptrthis->OnCallbackInMainThread(callbackInfo);
        }
    });
}

void MissionObserver::OnCallbackInMainThread(const ::ohos::distributedmissionmanager::ContinueCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::MISSION, "MissionObserver::OnCallbackInMainThread");
    std::vector<JsOnCallbackType> callbackListTemp;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        callbackListTemp = callbackList_;
    }
    for (auto &jsfunc : callbackListTemp) {
        jsfunc(info);
    }
}

} // namespace ani_observerutils