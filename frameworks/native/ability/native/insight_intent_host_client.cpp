/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "insight_intent_host_client.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
sptr<InsightIntentHostClient> InsightIntentHostClient::instance_ = nullptr;
std::mutex InsightIntentHostClient::instanceMutex_;
std::once_flag InsightIntentHostClient::singletonFlag_;

sptr<InsightIntentHostClient> InsightIntentHostClient::GetInstance()
{
    std::call_once(singletonFlag_, []() {
        instance_ = new (std::nothrow) InsightIntentHostClient();
        if (instance_ == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "failed to create InsightIntentHostClient.");
        }
    });
    return instance_;
}

uint64_t InsightIntentHostClient::AddInsightIntentExecute(
    const std::shared_ptr<InsightIntentExecuteCallbackInterface> &callback)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    std::lock_guard<std::mutex> lock(insightIntentExecutebackMutex_);
    callbackMap_.emplace(++key_, callback);
    return key_;
}

void InsightIntentHostClient::RemoveInsightIntentExecute(uint64_t key)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    std::lock_guard<std::mutex> lock(insightIntentExecutebackMutex_);
    auto iter = callbackMap_.find(key);
    if (iter != callbackMap_.end()) {
        callbackMap_.erase(key);
    }
}

void InsightIntentHostClient::OnExecuteDone(uint64_t key, int32_t resultCode,
    const AppExecFwk::InsightIntentExecuteResult &executeResult)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");

    std::shared_ptr<InsightIntentExecuteCallbackInterface> callback = nullptr;
    {
        std::lock_guard<std::mutex> lock(insightIntentExecutebackMutex_);
        auto iter = callbackMap_.find(key);
        if (iter == callbackMap_.end()) {
            TAG_LOGI(AAFwkTag::INTENT, "InsightIntent execute callback not found");
        } else {
            callback = iter->second;
            callbackMap_.erase(key);
        }
    }

    if (callback != nullptr) {
        callback->ProcessInsightIntentExecute(resultCode, executeResult);
    }
}
} // namespace AbilityRuntime
} // namespace OHOS