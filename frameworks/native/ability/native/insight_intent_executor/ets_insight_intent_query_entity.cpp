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

#include "ets_insight_intent_query_entity.h"

#include "ability_transaction_callback_info.h"
#include "ani_common_query_entity_param.h"
#include "ani_common_want.h"
#include "ets_insight_intent_context.h"
#include "hilog_tag_wrapper.h"
#include "insight_intent_delay_result_callback_mgr.h"
#include "insight_intent_execute_result.h"
#include "int_wrapper.h"
#include "want_params_wrapper.h"

#undef STATE_PATTERN_NAIVE_H
#define STATE_PATTERN_NAIVE_STATE state_
#include "state_pattern_naive.h"

#define ETS_EXPORT __attribute__((visibility("default")))

namespace OHOS::AbilityRuntime {
using State = EtsInsightIntentUtils::State;

namespace {
constexpr const char *SIGNATURE_APP_INTENT_ENTITY = "@ohos.app.ability.insightIntent.insightIntent.AppIntentEntity";
}

InsightIntentExecutor *EtsInsightIntentQueryEntity::Create(Runtime &runtime)
{
    return new (std::nothrow) EtsInsightIntentQueryEntity(static_cast<ETSRuntime &>(runtime));
}

EtsInsightIntentQueryEntity::EtsInsightIntentQueryEntity(ETSRuntime &runtime) : runtime_(runtime)
{}

EtsInsightIntentQueryEntity::~EtsInsightIntentQueryEntity()
{
    state_ = State::DESTROYED;
    EtsInsightIntentUtils::DeleteReference(runtime_, etsObj_);
}

std::unique_ptr<AppExecFwk::ETSNativeReference> EtsInsightIntentQueryEntity::LoadEtsCode(
    const InsightIntentExecutorInfo &insightIntentInfo, ETSRuntime &runtime)
{
    TAG_LOGD(AAFwkTag::INTENT, "LoadEtsCode called");
    auto executeParam = insightIntentInfo.executeParam;
    if (executeParam == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null executeParam");
        return nullptr;
    }

    std::string moduleName(executeParam->moduleName_);
    std::string srcPath(executeParam->moduleName_);
    // Convert @normalized format to original path
    std::string srcEntrance = executeParam->srcEntrance_;
    srcEntrance.append(":").append(executeParam->queryEntityClassName_);

    TAG_LOGD(AAFwkTag::INTENT, "module:%{public}s, srcPath:%{public}s, srcEntrance:%{public}s, className:%{public}s",
        moduleName.c_str(), srcPath.c_str(), srcEntrance.c_str(), executeParam->queryEntityClassName_.c_str());
    return runtime.LoadModule(moduleName, srcPath, insightIntentInfo.hapPath, insightIntentInfo.esmodule,
        false, srcEntrance);
}

bool EtsInsightIntentQueryEntity::Init(const InsightIntentExecutorInfo &insightIntentInfo)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    STATE_PATTERN_NAIVE_ACCEPT(State::CREATED, false);
    state_ = State::INITIALIZED;
    InsightIntentExecutor::Init(insightIntentInfo);

    if (insightIntentInfo.executeParam == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null execute param");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    auto *env = runtime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    etsObj_ = EtsInsightIntentQueryEntity::LoadEtsCode(insightIntentInfo, runtime_);
    if (etsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null etsObj_");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    ani_status status = ANI_ERROR;
    ani_class executorClass = nullptr;
    status = env->FindClass(SIGNATURE_APP_INTENT_ENTITY, &executorClass);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "find executor class failed status: %{public}d", status);
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    std::array functions = {
        ani_native_function {"nativeOnQueryEntityResult", nullptr, reinterpret_cast<void *>(OnQueryEntityResult)},
        ani_native_function {"nativeOnQueryEntityError", nullptr, reinterpret_cast<void *>(OnQueryEntityError)},
    };
    status = env->Class_BindNativeMethods(executorClass, functions.data(), functions.size());
    if (status != ANI_OK && status != ANI_ALREADY_BINDED) {
        TAG_LOGE(AAFwkTag::INTENT, "bind native methods failed status: %{public}d", status);
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    status = env->Object_SetFieldByName_Long(etsObj_->aniObj, "nativeExecutor", reinterpret_cast<ani_long>(this));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "set nativeExecutor failed status: %{public}d", status);
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }
    return true;
}

bool EtsInsightIntentQueryEntity::HandleExecuteIntent(std::shared_ptr<InsightIntentExecuteParam> executeParam,
    void *pageLoader, std::unique_ptr<InsightIntentExecutorAsyncCallback> callback, bool &isAsync)
{
    TAG_LOGD(AAFwkTag::INTENT, "HandleExecuteIntent called");
    STATE_PATTERN_NAIVE_ACCEPT(State::INITIALIZED, false);
    state_ = State::EXECUTING;

    if (callback == nullptr || callback->IsEmpty()) {
        TAG_LOGE(AAFwkTag::INTENT, "null callback");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }
    if (executeParam == nullptr || executeParam->queryParams_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid execute param");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    auto *env = runtime_.GetAniEnv();
    if (env == nullptr || etsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid runtime state");
        EtsInsightIntentUtils::ReplyFailed(callback.release());
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    queryCallback_ = std::make_unique<EtsInsightIntentQueryEntityCallback>();
    queryCallback_->queryType_ = executeParam->queryType_;
    queryCallback_->paramters_ = executeParam->queryParams_;
    queryCallback_->callback_ = std::move(callback);

    ani_ref wantParams  = WrapQueryEntityParam(env, executeParam->queryType_, executeParam->queryParams_);
    if (wantParams  == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "prepare ani params failed");
        return ExecuteIntentCheckError();
    }

    auto promise = EtsInsightIntentUtils::CallObjectMethod(runtime_, etsObj_, true, INSIGHT_INTENT_QUERY_ENTITY_FUNC,
        nullptr, wantParams);
    if (promise == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "queryEntity failed");
        return ExecuteIntentCheckError();
    }

    auto result = CallPromise(env, promise);
    if (!result) {
        TAG_LOGE(AAFwkTag::INTENT, "call object method return nullptr");
        return ExecuteIntentCheckError();
    }
    isAsync = true;
    return true;
}

void EtsInsightIntentQueryEntity::OnQueryEntityResult(ani_env *env, ani_object aniObj, ani_object result)
{
    TAG_LOGD(AAFwkTag::INTENT, "OnQueryEntityResult called");
    if (env == nullptr || aniObj == nullptr || result == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid onQueryEntity result callback input");
        return;
    }

    ani_long nativeExecutor = 0;
    if (env->Object_GetFieldByName_Long(aniObj, "nativeExecutor", &nativeExecutor) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "get nativeExecutor failed");
        return;
    }
    auto *executor = reinterpret_cast<EtsInsightIntentQueryEntity *>(nativeExecutor);
    if (executor == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null native executor");
        return;
    }
    executor->HandleEtsResultReturned(env, result);
}

void EtsInsightIntentQueryEntity::OnQueryEntityError(ani_env *env, ani_object aniObj, ani_object err)
{
    TAG_LOGD(AAFwkTag::INTENT, "OnQueryEntityError called");
    if (env == nullptr || aniObj == nullptr || err == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid onQueryEntity error callback input");
        return;
    }

    ani_long nativeExecutor = 0;
    if (env->Object_GetFieldByName_Long(aniObj, "nativeExecutor", &nativeExecutor) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "get nativeExecutor failed");
        return;
    }
    auto *executor = reinterpret_cast<EtsInsightIntentQueryEntity *>(nativeExecutor);
    if (executor == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null native executor");
        return;
    }
    executor->ReplyFailedInner();
}

void EtsInsightIntentQueryEntity::ReplyFailedInner(InsightIntentInnerErr innerErr)
{
    state_ = State::INVALID;
    if (queryCallback_ == nullptr || queryCallback_->callback_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "queryCallback_ or callback is nullptr");
    } else {
        EtsInsightIntentUtils::ReplyFailed(queryCallback_->callback_.release(), innerErr);
    }
}

void EtsInsightIntentQueryEntity::ReplySucceededInner(std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> resultCpp)
{
    state_ = State::EXECUTATION_DONE;
    if (queryCallback_ == nullptr || queryCallback_->callback_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "queryCallback_ or callback is nullptr");
    } else {
        EtsInsightIntentUtils::ReplySucceeded(queryCallback_->callback_.release(), resultCpp);
    }
}

bool EtsInsightIntentQueryEntity::ExecuteIntentCheckError()
{
    ReplyFailedInner();
    STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
}

bool EtsInsightIntentQueryEntity::HandleEtsResultReturned(ani_env *env, ani_ref result)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    if (env == nullptr || result == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid env or result");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    if (queryCallback_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null queryCallback");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    ani_size size = 0;
    ani_status status = ANI_ERROR;
    if ((status = env->Array_GetLength(reinterpret_cast<ani_array>(result), &size)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "Array_GetLength failed status: %{public}d", status);
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    ani_ref ref;
    std::vector<std::shared_ptr<AAFwk::WantParams>> queryResults;
    for (ani_size idx = 0; idx < size; idx++) {
        if ((status = env->Array_Get(reinterpret_cast<ani_array>(result), idx, &ref)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "Array_Get %{public}zu failed, status: %{public}d", idx, status);
            continue;
        }
        if (ref == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "null ref");
            continue;
        }
        auto wantParams = std::make_shared<AAFwk::WantParams>();
        if (!AppExecFwk::UnwrapWantParams(env, ref, *wantParams)) {
            TAG_LOGE(AAFwkTag::INTENT, "unwrap want parameter failed");
            continue;
        }
        queryResults.push_back(wantParams);
    }

    auto resultCpp = std::make_shared<AppExecFwk::InsightIntentExecuteResult>();
    resultCpp->isQueryEntity = true;
    resultCpp->queryResults = queryResults;
    ReplySucceededInner(resultCpp);
    return true;
}

bool EtsInsightIntentQueryEntity::CallPromise(ani_env *env, ani_ref promise)
{
    TAG_LOGD(AAFwkTag::INTENT, "CallPromise");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return false;
    }

    ani_method method = nullptr;
    if (env->Class_FindMethod(etsObj_->aniCls, "callPromise", nullptr, &method) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "find callPromise failed");
        return false;
    }

    ani_status status = env->Object_CallMethod_Void(etsObj_->aniObj, method, reinterpret_cast<ani_object>(promise));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "callPromise failed, status: %{public}d", status);
        return false;
    }
    return true;
}
} // namespace OHOS::AbilityRuntime

ETS_EXPORT extern "C" OHOS::AbilityRuntime::InsightIntentExecutor *OHOS_ETS_Insight_Intent_QueryEntity_Create(
    OHOS::AbilityRuntime::Runtime &runtime)
{
    return OHOS::AbilityRuntime::EtsInsightIntentQueryEntity::Create(runtime);
}