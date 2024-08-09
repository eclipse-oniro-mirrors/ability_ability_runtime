/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "sr_common_event_subscriber.h"

#include "common_event_support.h"
#include "hilog_tag_wrapper.h"
#include "service_router_data_mgr.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
SrCommonEventSubscriber::SrCommonEventSubscriber(const EventFwk::CommonEventSubscribeInfo &subscribeInfo)
    : EventFwk::CommonEventSubscriber(subscribeInfo)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "created");
}

SrCommonEventSubscriber::~SrCommonEventSubscriber()
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "destroyed");
}

void SrCommonEventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &eventData)
{
    const AAFwk::Want& want = eventData.GetWant();
    std::string action = want.GetAction();
    std::string bundleName = want.GetElement().GetBundleName();
    TAG_LOGI(AAFwkTag::SER_ROUTER, "action:%{public}s.", action.c_str());
    if (action.empty() || eventHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Invalid action or event handler");
        return;
    }
    if (bundleName.empty() && action != EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "invalid param, bundleName: %{public}s",
            bundleName.c_str());
        return;
    }
    std::weak_ptr<SrCommonEventSubscriber> weakThis = shared_from_this();
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED) {
        int32_t userId = eventData.GetCode();
        auto task = [weakThis, userId]() {
            std::shared_ptr<SrCommonEventSubscriber> sharedThis = weakThis.lock();
            if (sharedThis) {
                TAG_LOGI(AAFwkTag::SER_ROUTER, "%{public}d COMMON_EVENT_USER_SWITCHED", userId);
                ServiceRouterDataMgr::GetInstance().LoadAllBundleInfos();
            }
        };
        eventHandler_->PostTask(task, "SrCommonEventSubscriber:LoadAllBundleInfos");
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED ||
        action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_CHANGED) {
        auto task = [weakThis, bundleName]() {
            TAG_LOGI(AAFwkTag::SER_ROUTER, "bundle changed, bundleName: %{public}s", bundleName.c_str());
            std::shared_ptr<SrCommonEventSubscriber> sharedThis = weakThis.lock();
            if (sharedThis) {
                ServiceRouterDataMgr::GetInstance().LoadBundleInfo(bundleName);
            }
        };
        eventHandler_->PostTask(task, "SrCommonEventSubscriber:LoadBundleInfo");
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED) {
        auto task = [weakThis, bundleName]() {
            TAG_LOGI(AAFwkTag::SER_ROUTER, "bundle remove, bundleName: %{public}s", bundleName.c_str());
            std::shared_ptr<SrCommonEventSubscriber> sharedThis = weakThis.lock();
            if (sharedThis) {
                ServiceRouterDataMgr::GetInstance().DeleteBundleInfo(bundleName);
            }
        };
        eventHandler_->PostTask(task, "SrCommonEventSubscriber:DeleteBundleInfo");
    } else {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "Invalid action");
    }
}
} // AbilityRuntime
} // OHOS
