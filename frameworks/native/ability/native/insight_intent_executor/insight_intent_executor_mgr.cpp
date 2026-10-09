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

#include "insight_intent_executor_mgr.h"

#include "ability_business_error.h"
#include "ability_manager_client.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
InsightIntentExecutorMgr::InsightIntentExecutorMgr()
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
}

InsightIntentExecutorMgr::~InsightIntentExecutorMgr()
{
    TAG_LOGI(AAFwkTag::INTENT, "called");
}

bool InsightIntentExecutorMgr::ExecuteInsightIntent(Runtime& runtime, const InsightIntentExecutorInfo& executeInfo,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback)
{
    auto executeParam = executeInfo.executeParam;
    if (callback == nullptr) {
        TAG_LOGW(AAFwkTag::INTENT, "null callback");
        return false;
    }
    if (executeParam == nullptr || executeParam->insightIntentParam_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null executeParam or insightIntentParam_");
        TriggerCallbackInner(std::move(callback), static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
        return false;
    }

    auto asyncCallback =
        [weak = weak_from_this(), intentId = executeParam->insightIntentId_](InsightIntentExecuteResult& result) {
            // erase map when called
            TAG_LOGD(AAFwkTag::INTENT, "called");
            auto executorMgr = weak.lock();
            if (executorMgr == nullptr) {
                TAG_LOGE(AAFwkTag::INTENT, "null executorMgr");
                return;
            }
            executorMgr->RemoveInsightIntentExecutor(intentId);
        };
    callback->Push(asyncCallback);

    // Notify AMS that this intent execution has started (AMS records the process pid).
    AAFwk::AbilityManagerClient::GetInstance()->NotifyInsightIntentExecute(
        executeParam->insightIntentId_);

    // Create insight intent executor
    InsightIntentType type = static_cast<InsightIntentType>(executeParam->decoratorType_);
    auto intentExecutor = CreateExecutor(runtime, type, executeInfo);
    if (intentExecutor == nullptr) {
        TriggerCallbackInner(std::move(callback), static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
        return false;
    }

    TAG_LOGI(AAFwkTag::INTENT, "execute insight intent, type: %{public}hhu", type);
    bool isAsync = false;
    bool ret = false;
    if (runtime.GetLanguage() == AbilityRuntime::Runtime::Language::ETS) {
        ret = intentExecutor->HandleExecuteIntent(executeParam, executeInfo.etsPageLoader,
            std::move(callback), isAsync);
    } else {
        ret = intentExecutor->HandleExecuteIntent(executeParam, executeInfo.pageLoader, std::move(callback), isAsync);
    }
    if (!ret) {
        TAG_LOGE(AAFwkTag::INTENT, "Handle Execute intent failed");
        // callback has removed, if execute insight intent failed, call in sub function.
        return false;
    }

    return true;
}

std::shared_ptr<InsightIntentExecutor> InsightIntentExecutorMgr::CreateExecutor(
    Runtime& runtime, InsightIntentType type, const InsightIntentExecutorInfo &executeInfo)
{
    auto executeParam = executeInfo.executeParam;
    auto intentExecutor = InsightIntentExecutor::Create(runtime, type);
    if (intentExecutor == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null intentExecutor");
        return nullptr;
    }
    if (!intentExecutor->Init(executeInfo)) {
        TAG_LOGE(AAFwkTag::INTENT, "Init intent executor failed");
        return nullptr;
    }
    AddInsightIntentExecutor(executeParam->insightIntentId_, intentExecutor);
    return intentExecutor;
}

void InsightIntentExecutorMgr::AddInsightIntentExecutor(uint64_t intentId,
    const std::shared_ptr<InsightIntentExecutor>& executor)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    std::lock_guard<std::mutex> lock(mutex_);
    insightIntentExecutors_[intentId] = executor;
}

void InsightIntentExecutorMgr::RemoveInsightIntentExecutor(uint64_t intentId)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    std::lock_guard<std::mutex> lock(mutex_);
    insightIntentExecutors_.erase(intentId);
}

void InsightIntentExecutorMgr::TriggerCallbackInner(
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback, int32_t errCode)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    AppExecFwk::InsightIntentExecuteResult result;
    result.innerErr = errCode;
    if (callback) {
        callback->Call(result);
    }
    callback.reset();
}
} // namespace AbilityRuntime
} // namespace OHOS
