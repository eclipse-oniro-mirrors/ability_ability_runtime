/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "insight_intent_will_execute_callback_mgr.h"

#include "ability_manager_client.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
std::mutex InsightIntentWillExecuteCallbackMgr::instanceMutex_;
sptr<InsightIntentWillExecuteCallbackMgr> InsightIntentWillExecuteCallbackMgr::instance_ = nullptr;

sptr<InsightIntentWillExecuteCallbackMgr> InsightIntentWillExecuteCallbackMgr::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(instanceMutex_);
        if (instance_ == nullptr) {
            instance_ = sptr<InsightIntentWillExecuteCallbackMgr>::MakeSptr();
        }
    }
    return instance_;
}

int32_t InsightIntentWillExecuteCallbackMgr::Register(const WillExecuteCallback &callback, uint32_t &callbackId)
{
    TAG_LOGD(AAFwkTag::INTENT, "will execute callback mgr register called");
    bool shouldRegisterToAms = false;
    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        shouldRegisterToAms = callbacks_.empty();
        callbackId = serialNumber_++;
        callbacks_[callbackId] = callback;
    }
    if (shouldRegisterToAms) {
        auto ret = AAFwk::AbilityManagerClient::GetInstance()->RegisterWillInsightIntentExecuteCallback(AsObject());
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "register will execute callback failed, ret:%{public}d", ret);
            std::lock_guard<std::mutex> lock(callbackMutex_);
            callbacks_.erase(callbackId);
            return ret;
        }
    }
    return ERR_OK;
}

void InsightIntentWillExecuteCallbackMgr::Unregister(uint32_t callbackId)
{
    TAG_LOGD(AAFwkTag::INTENT, "will execute callback mgr unregister called, callbackId: %{public}u", callbackId);
    bool shouldUnregisterToAms = false;
    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        callbacks_.erase(callbackId);
        shouldUnregisterToAms = callbacks_.empty();
    }
    if (shouldUnregisterToAms) {
        AAFwk::AbilityManagerClient::GetInstance()->UnregisterWillInsightIntentExecuteCallback();
    }
}

void InsightIntentWillExecuteCallbackMgr::UnregisterAll()
{
    TAG_LOGD(AAFwkTag::INTENT, "will execute callback mgr unregister all called");
    bool shouldUnregisterToAms = false;
    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        shouldUnregisterToAms = !callbacks_.empty();
        callbacks_.clear();
    }
    if (shouldUnregisterToAms) {
        AAFwk::AbilityManagerClient::GetInstance()->UnregisterWillInsightIntentExecuteCallback();
    }
}

void InsightIntentWillExecuteCallbackMgr::OnWillInsightIntentExecute(const InsightIntentExecutionInfo &info)
{
    std::map<uint32_t, WillExecuteCallback> callbacks;
    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        callbacks = callbacks_;
    }
    TAG_LOGD(AAFwkTag::INTENT, "will execute callback mgr dispatch, callback count:%{public}zu", callbacks.size());
    for (auto &entry : callbacks) {
        if (entry.second != nullptr) {
            entry.second(info);
        }
    }
}
} // namespace AbilityRuntime
} // namespace OHOS