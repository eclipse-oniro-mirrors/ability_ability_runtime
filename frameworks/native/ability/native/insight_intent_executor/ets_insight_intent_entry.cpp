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

#include "ets_insight_intent_entry.h"

#include "ability_transaction_callback_info.h"
#include "ani_common_want.h"
#include "ani_enum_convert.h"
#include "ets_insight_intent_context.h"
#include "hilog_tag_wrapper.h"
#include "insight_intent_delay_result_callback_mgr.h"
#include "insight_intent_execute_result.h"
#undef STATE_PATTERN_NAIVE_H
#define STATE_PATTERN_NAIVE_STATE state_
#include "state_pattern_naive.h"

#define ETS_EXPORT __attribute__((visibility("default")))

namespace OHOS::AbilityRuntime {
using State = EtsInsightIntentUtils::State;

namespace {
constexpr const char *ENTRY_EXECUTOR_CLASS_NAME =
    "@ohos.app.ability.InsightIntentEntryExecutor.InsightIntentEntryExecutor";
constexpr const char *EXECUTE_MODE_ENUM_NAME = "@ohos.app.ability.insightIntent.insightIntent.ExecuteMode";
constexpr const char *CHECK_PROMISE_SIGNATURE = "Y:z";
constexpr const char *CALL_PROMISE_SIGNATURE = "C{std.core.Promise}:";
constexpr const char *RECORD_CLASS_NAME = "std.core.Record";
} // namespace

InsightIntentExecutor *EtsInsightIntentEntry::Create(Runtime &runtime)
{
    return new (std::nothrow) EtsInsightIntentEntry(static_cast<ETSRuntime &>(runtime));
}

EtsInsightIntentEntry::EtsInsightIntentEntry(ETSRuntime &runtime) : runtime_(runtime) {}

EtsInsightIntentEntry::~EtsInsightIntentEntry()
{
    state_ = State::DESTROYED;
    EtsInsightIntentUtils::DeleteReference(runtime_, contextObj_);
    EtsInsightIntentUtils::DeleteReference(runtime_, etsObj_);
}

std::unique_ptr<AppExecFwk::ETSNativeReference> EtsInsightIntentEntry::LoadEtsCode(
    const InsightIntentExecutorInfo &insightIntentInfo, ETSRuntime &runtime)
{
    auto executeParam = insightIntentInfo.executeParam;
    if (executeParam == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null executeParam");
        return nullptr;
    }

    std::string moduleName(executeParam->moduleName_);
    std::string srcPath(executeParam->moduleName_);

    // Convert @normalized format to original path
    std::string srcEntrance = executeParam->srcEntrance_;
    srcEntrance = srcEntrance + ':' + insightIntentInfo.decoratorClass;
    return runtime.LoadModule(moduleName, srcPath, insightIntentInfo.hapPath, insightIntentInfo.esmodule,
        false, srcEntrance);
}

bool EtsInsightIntentEntry::Init(const InsightIntentExecutorInfo &insightIntentInfo)
{
    TAG_LOGD(AAFwkTag::INTENT, "EtsInsightIntentEntry::Init called");
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

    if (etsObj_ == nullptr) {
        etsObj_ = EtsInsightIntentEntry::LoadEtsCode(insightIntentInfo, runtime_);
        if (etsObj_ == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "null etsObj_");
            STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
        }
    }

    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null Context");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }
    contextCpp_ = std::make_shared<EtsInsightIntentContext>(context);
    contextObj_ = CreateEtsInsightIntentContext(env, contextCpp_.get());
    if (contextObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null contextObj_");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    ani_status status = env->Object_SetFieldByName_Ref(etsObj_->aniObj, "context", contextObj_->aniRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "set context failed status: %{public}d", status);
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    ani_class executorClass = nullptr;
    status = env->FindClass(ENTRY_EXECUTOR_CLASS_NAME, &executorClass);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "find executor class failed status: %{public}d", status);
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    std::array functions = {
        ani_native_function {"nativeOnExecuteResult", nullptr, reinterpret_cast<void *>(OnExecuteResult)},
        ani_native_function {"nativeOnExecuteError", nullptr, reinterpret_cast<void *>(OnExecuteError)},
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

    context->SetExecuteMode(insightIntentInfo.executeParam->executeMode_);
    return true;
}

bool EtsInsightIntentEntry::HandleExecuteIntent(std::shared_ptr<InsightIntentExecuteParam> executeParam,
    void *pageLoader, std::unique_ptr<InsightIntentExecutorAsyncCallback> callback, bool &isAsync)
{
    STATE_PATTERN_NAIVE_ACCEPT(State::INITIALIZED, false);
    state_ = State::EXECUTING;

    if (callback == nullptr || callback->IsEmpty()) {
        TAG_LOGE(AAFwkTag::INTENT, "null callback");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }
    if (executeParam == nullptr || executeParam->insightIntentParam_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid execute param");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    auto *env = runtime_.GetAniEnv();
    if (env == nullptr || etsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid runtime state");
        EtsInsightIntentUtils::ReplyFailed(callback.release());
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    callback_ = std::move(callback);
    auto mode = static_cast<InsightIntentExecuteMode>(executeParam->executeMode_);
    if (!PrepareExecuteEnvironment(env, mode, pageLoader)) {
        return ExecuteIntentCheckError();
    }
    if (!AssignObject(env, *executeParam->insightIntentParam_)) {
        TAG_LOGE(AAFwkTag::INTENT, "assign object failed");
        return ExecuteIntentCheckError();
    }
    return ExecuteInsightIntent(env, isAsync);
}

bool EtsInsightIntentEntry::AssignObject(ani_env *env, const AAFwk::WantParams &wantParams)
{
    if (env == nullptr || etsObj_ == nullptr || etsObj_->aniObj == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid env or ets object");
        return false;
    }

    ani_ref srcObj = AppExecFwk::WrapWantParams(env, wantParams);
    if (srcObj == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "wrap want params failed");
        return false;
    }

    ani_class recordCls = nullptr;
    ani_method recordGetMethod = nullptr;
    ani_status status = env->FindClass(RECORD_CLASS_NAME, &recordCls);
    if (status != ANI_OK || recordCls == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "find record class failed, status: %{public}d", status);
        return false;
    }
    status = env->Class_FindMethod(recordCls, "$_get", nullptr, &recordGetMethod);
    if (status != ANI_OK || recordGetMethod == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "find $_get failed, status: %{public}d", status);
        return false;
    }

    for (const auto &item : wantParams.GetParams()) {
        if (!HasDeclaredProperty(env, item.first)) {
            continue;
        }

        ani_string key = AppExecFwk::GetAniString(env, item.first);
        if (key == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "create key failed, name: %{public}s", item.first.c_str());
            return false;
        }

        ani_ref valueRef = nullptr;
        status = env->Object_CallMethod_Ref(static_cast<ani_object>(srcObj), recordGetMethod, &valueRef, key);
        if (status != ANI_OK || valueRef == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "get property failed, name: %{public}s, status: %{public}d",
                item.first.c_str(), status);
            return false;
        }

        status = env->Object_SetPropertyByName_Ref(etsObj_->aniObj, item.first.c_str(), valueRef);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "set property failed, name: %{public}s, status: %{public}d",
                item.first.c_str(), status);
            return false;
        }
    }
    return true;
}

bool EtsInsightIntentEntry::HasDeclaredProperty(ani_env *env, const std::string &name)
{
    if (env == nullptr || etsObj_ == nullptr || etsObj_->aniObj == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid env or ets object");
        return false;
    }

    ani_ref executorProp = nullptr;
    ani_status status = env->Object_GetPropertyByName_Ref(etsObj_->aniObj, name.c_str(), &executorProp);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "get property failed, name: %{public}s, status: %{public}d", name.c_str(), status);
        return false;
    }

    ani_boolean isUndefined = ANI_FALSE;
    status = env->Reference_IsUndefined(executorProp, &isUndefined);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "check property failed, name: %{public}s, status: %{public}d",
            name.c_str(), status);
        return false;
    }
    return isUndefined != ANI_TRUE;
}

bool EtsInsightIntentEntry::PrepareExecuteEnvironment(ani_env *env, InsightIntentExecuteMode mode, void *pageLoader)
{
    if (env == nullptr || etsObj_ == nullptr || etsObj_->aniObj == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid env or ets object");
        return false;
    }

    ani_enum_item executeModeItem {};
    OHOS::AAFwk::AniEnumConvertUtil::EnumConvert_NativeToEts(env, EXECUTE_MODE_ENUM_NAME, mode, executeModeItem);
    ani_status status = env->Object_SetPropertyByName_Ref(etsObj_->aniObj, "executeMode", executeModeItem);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "set executeMode failed, status: %{public}d", status);
        return false;
    }

    if (mode == InsightIntentExecuteMode::UIABILITY_BACKGROUND ||
        mode == InsightIntentExecuteMode::SERVICE_EXTENSION_ABILITY) {
        return true;
    } else if (pageLoader == nullptr) {
        return false;
    }

    const char *propertyName = mode == InsightIntentExecuteMode::UIEXTENSION_ABILITY ?
        "uiExtensionSession" : "windowStage";
    status = env->Object_SetPropertyByName_Ref(etsObj_->aniObj, propertyName, reinterpret_cast<ani_ref>(pageLoader));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "set page loader failed");
        return false;
    }
    return true;
}

bool EtsInsightIntentEntry::ExecuteInsightIntent(ani_env *env, bool &isAsync)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return ExecuteIntentCheckError();
    }

    ani_ref result = EtsInsightIntentUtils::CallObjectMethod(runtime_, etsObj_, true, "onExecute", nullptr);
    if (result == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "onExecute failed");
        return ExecuteIntentCheckError();
    }

    ani_method method = nullptr;
    if (env->Class_FindMethod(etsObj_->aniCls, "checkPromise", CHECK_PROMISE_SIGNATURE, &method) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "find checkPromise failed");
        return ExecuteIntentCheckError();
    }

    ani_boolean isPromise = ANI_FALSE;
    if (env->Object_CallMethod_Boolean(etsObj_->aniObj, method, &isPromise,
        reinterpret_cast<ani_object>(result)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "call checkPromise failed");
        return ExecuteIntentCheckError();
    }

    isAsync_ = static_cast<bool>(isPromise);
    isAsync = isAsync_;
    if (!isAsync_) {
        TAG_LOGE(AAFwkTag::INTENT, "onExecute returned non-promise");
        return ExecuteIntentCheckError();
    }

    if (env->Class_FindMethod(etsObj_->aniCls, "callPromise", CALL_PROMISE_SIGNATURE, &method) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "find callPromise failed");
        return ExecuteIntentCheckError();
    }

    ani_status status = env->Object_CallMethod_Void(etsObj_->aniObj, method, reinterpret_cast<ani_object>(result));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "callPromise failed, status: %{public}d", status);
        return ExecuteIntentCheckError();
    }
    return true;
}

void EtsInsightIntentEntry::ReplyFailedInner(InsightIntentInnerErr innerErr)
{
    state_ = State::INVALID;
    EtsInsightIntentUtils::ReplyFailed(callback_.release(), innerErr);
}

void EtsInsightIntentEntry::ReplySucceededInner(std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> resultCpp)
{
    state_ = State::EXECUTATION_DONE;
    EtsInsightIntentUtils::ReplySucceeded(callback_.release(), resultCpp);
}

bool EtsInsightIntentEntry::ExecuteIntentCheckError()
{
    ReplyFailedInner();
    STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
}

bool EtsInsightIntentEntry::HandleResultReturnedFromEtsFunc(ani_env *env, ani_ref result, bool isAsync)
{
    if (env == nullptr || result == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid env or result");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }
    auto resultCpp = EtsInsightIntentUtils::GetResultFromEts(env, result, true);
    if (resultCpp == nullptr) {
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null context");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }
    if (context->GetDelayReturnMode() == InsightIntentReturnMode::FUNCTION) {
        resultCpp->isNeedDelayResult = true;
    } else {
        InsightIntentDelayResultCallbackMgr::GetInstance().RemoveDelayResultCallback(context->GetIntentId());
    }

    if (isAsync) {
        EtsInsightIntentUtils::ReplySucceeded(callback_.release(), resultCpp);
        return true;
    }
    ReplySucceededInner(resultCpp);
    return true;
}

void EtsInsightIntentEntry::OnExecuteResult(ani_env *env, ani_object aniObj, ani_object result)
{
    if (env == nullptr || aniObj == nullptr || result == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid onExecute result callback input");
        return;
    }
    ani_long nativeExecutor = 0;
    if (env->Object_GetFieldByName_Long(aniObj, "nativeExecutor", &nativeExecutor) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "get nativeExecutor failed");
        return;
    }
    auto *executor = reinterpret_cast<EtsInsightIntentEntry *>(nativeExecutor);
    if (executor == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null native executor");
        return;
    }
    executor->HandleResultReturnedFromEtsFunc(env, result, true);
}

void EtsInsightIntentEntry::OnExecuteError(ani_env *env, ani_object aniObj, ani_object error)
{
    (void)error;
    if (env == nullptr || aniObj == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid onExecute error callback input");
        return;
    }
    ani_long nativeExecutor = 0;
    if (env->Object_GetFieldByName_Long(aniObj, "nativeExecutor", &nativeExecutor) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "get nativeExecutor failed");
        return;
    }
    auto *executor = reinterpret_cast<EtsInsightIntentEntry *>(nativeExecutor);
    if (executor == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null native executor");
        return;
    }
    executor->ReplyFailedInner();
}
} // namespace OHOS::AbilityRuntime

ETS_EXPORT extern "C" OHOS::AbilityRuntime::InsightIntentExecutor *OHOS_ETS_Insight_Intent_Entry_Create(
    OHOS::AbilityRuntime::Runtime &runtime)
{
    return OHOS::AbilityRuntime::EtsInsightIntentEntry::Create(runtime);
}
