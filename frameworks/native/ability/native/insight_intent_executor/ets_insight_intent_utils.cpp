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

#include "ets_insight_intent_utils.h"

#include <cstdarg>

#include "ability_transaction_callback_info.h"
#include "ani_common_execute_result.h"
#include "hilog_tag_wrapper.h"
#include "insight_intent_execute_result.h"

namespace OHOS::AbilityRuntime {
void EtsInsightIntentUtils::DeleteReference(
    ETSRuntime &runtime, const std::unique_ptr<AppExecFwk::ETSNativeReference> &ref)
{
    auto *env = runtime.GetAniEnv();
    if (env != nullptr && ref != nullptr && ref->aniRef != nullptr) {
        env->GlobalReference_Delete(ref->aniRef);
    }
}

ani_ref EtsInsightIntentUtils::CallObjectMethod(ETSRuntime &runtime,
    const std::unique_ptr<AppExecFwk::ETSNativeReference> &etsObj, bool withResult, const char *name,
    const char *signature, ...)
{
    if (etsObj == nullptr || etsObj->aniCls == nullptr || etsObj->aniObj == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid ets object");
        return nullptr;
    }

    auto *env = runtime.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return nullptr;
    }

    ani_method method = nullptr;
    auto status = env->Class_FindMethod(etsObj->aniCls, name, signature, &method);
    if (status != ANI_OK || method == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "find method failed status: %{public}d", status);
        return nullptr;
    }

    ani_ref result = nullptr;
    va_list args;
    if (withResult) {
        va_start(args, signature);
        status = env->Object_CallMethod_Ref_V(etsObj->aniObj, method, &result, args);
        va_end(args);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "call method failed status: %{public}d", status);
            return nullptr;
        }
        return result;
    }

    va_start(args, signature);
    status = env->Object_CallMethod_Void_V(etsObj->aniObj, method, args);
    va_end(args);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "call method failed status: %{public}d", status);
    }
    return nullptr;
}

std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> EtsInsightIntentUtils::GetResultFromEts(
    ani_env *env, ani_ref result, bool isDecorator)
{
    if (env == nullptr || result == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid result env or object");
        return nullptr;
    }

    AppExecFwk::InsightIntentExecuteResult resultInner;
    ani_object resultObject = static_cast<ani_object>(result);
    if (!UnwrapExecuteResult(env, resultObject, resultInner, isDecorator)) {
        TAG_LOGE(AAFwkTag::INTENT, "UnwrapExecuteResult failed");
        return nullptr;
    }
    return std::make_shared<AppExecFwk::InsightIntentExecuteResult>(resultInner);
}

void EtsInsightIntentUtils::ReplyFailed(InsightIntentExecutorAsyncCallback *callback, InsightIntentInnerErr innerErr)
{
    if (callback == nullptr) {
        return;
    }
    AppExecFwk::InsightIntentExecuteResult errorResult {};
    errorResult.innerErr = innerErr;
    callback->Call(errorResult);
    delete callback;
}

void EtsInsightIntentUtils::ReplySucceeded(
    InsightIntentExecutorAsyncCallback *callback, std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> resultCpp)
{
    if (callback == nullptr) {
        return;
    }
    if (resultCpp == nullptr) {
        ReplyFailed(callback);
        return;
    }
    resultCpp->innerErr = InsightIntentInnerErr::INSIGHT_INTENT_ERR_OK;
    callback->Call(*resultCpp);
    delete callback;
}
} // namespace OHOS::AbilityRuntime
