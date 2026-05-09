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

#include "ets_insight_intent_func.h"

#include <algorithm>
#include <initializer_list>

#include "ability_transaction_callback_info.h"
#include "ani_common_want.h"
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
constexpr const char *RECORD_CLASS_NAME = "std.core.Record";
constexpr const char *ABILITY_UTILS_CLASS_NAME = "utils.AbilityUtils.AbilityUtils";
constexpr const char *FUNCTION_UTILS_CLASS_NAME = "utils.AbilityUtils.InsightIntentFunctionUtils";
constexpr const char *METHOD_RESULT_KEY = "methodResult";
constexpr const char *CALL_PROMISE_SIGNATURE = "C{std.core.Promise}:";
constexpr int ANI_ALREADY_BINDED = 8;
constexpr const char *CLASSNAME_STRING = "std.core.String";
constexpr const char *CLASSNAME_INT = "std.core.Int";
constexpr const char *CLASSNAME_LONG = "std.core.Long";
constexpr const char *CLASSNAME_SHORT = "std.core.Short";
constexpr const char *CLASSNAME_FLOAT = "std.core.Float";
constexpr const char *CLASSNAME_DOUBLE = "std.core.Double";
constexpr const char *CLASSNAME_BOOLEAN = "std.core.Boolean";
constexpr const char *CLASSNAME_ARRAY = "std.core.Array";
constexpr const char *FUNCTION_RETURN_TYPE_VOID = "void";

bool IsInstanceOfClass(ani_env *env, ani_ref value, const char *className)
{
    ani_class cls = nullptr;
    ani_status status = env->FindClass(className, &cls);
    if (status != ANI_OK || cls == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "find class failed, className: %{public}s, status: %{public}d",
            className, status);
        return false;
    }
    ani_boolean result = ANI_FALSE;
    status = env->Object_InstanceOf(static_cast<ani_object>(value), cls, &result);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "instanceof failed, className: %{public}s, status: %{public}d",
            className, status);
        return false;
    }
    return static_cast<bool>(result);
}

bool ValidateAniValueType(
    ani_env *env, ani_ref value, const char *typeName, std::initializer_list<const char *> classNames)
{
    if (std::any_of(classNames.begin(), classNames.end(),
        [env, value](const char *className) { return IsInstanceOfClass(env, value, className); })) {
        return true;
    }
    TAG_LOGE(AAFwkTag::INTENT, "type mismatch: expected %{public}s", typeName);
    return false;
}

bool ValidateParamType(ani_env *env, ani_ref value, AppExecFwk::ParamType expectedType)
{
    if (expectedType == AppExecFwk::ParamType::UNKNOWN) {
        TAG_LOGD(AAFwkTag::INTENT, "unknown type, skip validation");
        return true;
    }
    if (value == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null value for validation");
        return false;
    }
    switch (expectedType) {
        case AppExecFwk::ParamType::STRING:
            return ValidateAniValueType(env, value, "string", { CLASSNAME_STRING });
        case AppExecFwk::ParamType::NUMBER:
            return ValidateAniValueType(env, value, "number",
                { CLASSNAME_INT, CLASSNAME_LONG, CLASSNAME_SHORT, CLASSNAME_FLOAT, CLASSNAME_DOUBLE });
        case AppExecFwk::ParamType::INTEGER:
            return ValidateAniValueType(env, value, "integer", { CLASSNAME_INT, CLASSNAME_LONG, CLASSNAME_SHORT });
        case AppExecFwk::ParamType::BOOLEAN:
            return ValidateAniValueType(env, value, "boolean", { CLASSNAME_BOOLEAN });
        case AppExecFwk::ParamType::OBJECT:
            return ValidateAniValueType(env, value, "object", { RECORD_CLASS_NAME });
        case AppExecFwk::ParamType::ARRAY:
            return ValidateAniValueType(env, value, "array", { CLASSNAME_ARRAY });
        default:
            TAG_LOGD(AAFwkTag::INTENT, "unknown type, skip validation");
            return true;
    }
}

ani_object CreateMethodResultRecord(ani_env *env, ani_ref result)
{
    if (env == nullptr || result == nullptr) {
        return nullptr;
    }
    ani_class recordCls = nullptr;
    ani_method recordCtor = nullptr;
    ani_method recordSetMethod = nullptr;
    ani_status status = env->FindClass(RECORD_CLASS_NAME, &recordCls);
    if (status != ANI_OK || recordCls == nullptr) {
        return nullptr;
    }
    status = env->Class_FindMethod(recordCls, "<ctor>", nullptr, &recordCtor);
    if (status != ANI_OK || recordCtor == nullptr) {
        return nullptr;
    }
    status = env->Class_FindMethod(recordCls, "$_set", nullptr, &recordSetMethod);
    if (status != ANI_OK || recordSetMethod == nullptr) {
        return nullptr;
    }
    ani_object recordObject = nullptr;
    status = env->Object_New(recordCls, recordCtor, &recordObject);
    if (status != ANI_OK || recordObject == nullptr) {
        return nullptr;
    }

    ani_string key = AppExecFwk::GetAniString(env, METHOD_RESULT_KEY);
    if (key == nullptr) {
        return nullptr;
    }
    status = env->Object_CallMethod_Void(recordObject, recordSetMethod, key, static_cast<ani_object>(result));
    if (status != ANI_OK) {
        return nullptr;
    }
    return recordObject;
}
} // namespace

InsightIntentExecutor *EtsInsightIntentFunc::Create(Runtime &runtime)
{
    return new (std::nothrow) EtsInsightIntentFunc(static_cast<ETSRuntime &>(runtime));
}

EtsInsightIntentFunc::EtsInsightIntentFunc(ETSRuntime &runtime) : runtime_(runtime) {}

EtsInsightIntentFunc::~EtsInsightIntentFunc()
{
    state_ = State::DESTROYED;
    EtsInsightIntentUtils::DeleteReference(runtime_, etsObj_);
    EtsInsightIntentUtils::DeleteReference(runtime_, promiseHelperObj_);
}

std::unique_ptr<AppExecFwk::ETSNativeReference> EtsInsightIntentFunc::LoadEtsCode(
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
    srcEntrance = srcEntrance + ':' + executeParam->className_;
    return runtime.LoadModule(moduleName, srcPath, insightIntentInfo.hapPath, insightIntentInfo.esmodule,
        false, srcEntrance);
}

bool EtsInsightIntentFunc::Init(const InsightIntentExecutorInfo &insightIntentInfo)
{
    TAG_LOGD(AAFwkTag::INTENT, "EtsInsightIntentFunc::Init called");
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

    etsObj_ = EtsInsightIntentFunc::LoadEtsCode(insightIntentInfo, runtime_);
    if (etsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null etsObj_");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }
    if (!CreatePromiseHelper()) {
        TAG_LOGE(AAFwkTag::INTENT, "create promise helper failed");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }
    return true;
}

bool EtsInsightIntentFunc::CreatePromiseHelper()
{
    auto *env = runtime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return false;
    }

    ani_class helperCls = nullptr;
    ani_status status = env->FindClass(FUNCTION_UTILS_CLASS_NAME, &helperCls);
    if (status != ANI_OK || helperCls == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "find function utils class failed, status: %{public}d", status);
        return false;
    }

    return BindPromiseHelperMethods(env, helperCls) && InitPromiseHelperReference(env, helperCls);
}

bool EtsInsightIntentFunc::BindPromiseHelperMethods(ani_env *env, ani_class helperCls)
{
    if (env == nullptr || helperCls == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid env or helper class");
        return false;
    }

    std::array functions = {
        ani_native_function {"nativeOnExecuteResult", nullptr, reinterpret_cast<void *>(ResolvePromiseCbCpp)},
        ani_native_function {"nativeOnExecuteError", nullptr, reinterpret_cast<void *>(RejectPromiseCbCpp)},
    };
    ani_status status = env->Class_BindNativeMethods(helperCls, functions.data(), functions.size());
    if (status != ANI_OK && status != ANI_ALREADY_BINDED) {
        TAG_LOGE(AAFwkTag::INTENT, "bind promise helper methods failed, status: %{public}d", status);
        return false;
    }
    return true;
}

bool EtsInsightIntentFunc::InitPromiseHelperReference(ani_env *env, ani_class helperCls)
{
    if (env == nullptr || helperCls == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid env or helper class");
        return false;
    }

    ani_method ctor = nullptr;
    ani_status status = env->Class_FindMethod(helperCls, "<ctor>", ":", &ctor);
    if (status != ANI_OK || ctor == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "find promise helper ctor failed, status: %{public}d", status);
        return false;
    }

    ani_object helperObj = nullptr;
    status = env->Object_New(helperCls, ctor, &helperObj);
    if (status != ANI_OK || helperObj == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "create promise helper failed, status: %{public}d", status);
        return false;
    }

    ani_ref helperRef = nullptr;
    status = env->GlobalReference_Create(helperObj, &helperRef);
    if (status != ANI_OK || helperRef == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "create promise helper ref failed, status: %{public}d", status);
        return false;
    }

    status = env->Object_SetFieldByName_Long(helperObj, "nativeExecutor", reinterpret_cast<ani_long>(this));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "set promise helper nativeExecutor failed, status: %{public}d", status);
        env->GlobalReference_Delete(helperRef);
        return false;
    }

    promiseHelperObj_ = std::make_unique<AppExecFwk::ETSNativeReference>();
    promiseHelperObj_->aniCls = helperCls;
    promiseHelperObj_->aniObj = helperObj;
    promiseHelperObj_->aniRef = helperRef;
    return true;
}

bool EtsInsightIntentFunc::HandleExecuteIntent(std::shared_ptr<InsightIntentExecuteParam> executeParam,
    void *pageLoader, std::unique_ptr<InsightIntentExecutorAsyncCallback> callback, bool &isAsync)
{
    (void)pageLoader;
    TAG_LOGD(AAFwkTag::INTENT, "EtsInsightIntentFunc::HandleExecuteIntent called");
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
    return ExecuteInsightIntent(env, executeParam, isAsync);
}

bool EtsInsightIntentFunc::GetDecodedMethodParam(const std::string &encodedMethodParam,
    AppExecFwk::InsightIntentParam &methodParamInfo) const
{
    if (!AppExecFwk::DecodeMethodParam(encodedMethodParam, methodParamInfo)) {
        TAG_LOGE(AAFwkTag::INTENT, "decode method param failed");
        return false;
    }
    return true;
}

bool EtsInsightIntentFunc::GetMethodArg(ani_env *env, ani_object wantParams, ani_method recordGetMethod,
    const std::string &encodedMethodParam, ani_ref &valueRef)
{
    AppExecFwk::InsightIntentParam methodParamInfo;
    if (!GetDecodedMethodParam(encodedMethodParam, methodParamInfo)) {
        return false;
    }
    const std::string &paramName = methodParamInfo.paramName;
    ani_string key = AppExecFwk::GetAniString(env, paramName);
    if (key == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "create key failed, name: %{public}s", paramName.c_str());
        return false;
    }
    ani_status status = env->Object_CallMethod_Ref(wantParams, recordGetMethod, &valueRef, key);
    if (status != ANI_OK || valueRef == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "get method param failed, name: %{public}s, status: %{public}d",
            paramName.c_str(), status);
        return false;
    }
    if (!ValidateParamType(env, valueRef, methodParamInfo.type)) {
        TAG_LOGE(AAFwkTag::INTENT, "param type validation failed, name: %{public}s", paramName.c_str());
        return false;
    }
    return true;
}

bool EtsInsightIntentFunc::BuildMethodArgs(ani_env *env,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam, std::vector<ani_value> &args)
{
    if (env == nullptr || executeParam == nullptr || executeParam->insightIntentParam_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid execute param");
        return false;
    }

    ani_ref wantParams = AppExecFwk::WrapWantParams(env, *executeParam->insightIntentParam_);
    if (wantParams == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "prepare ani params failed");
        return false;
    }

    ani_class recordCls = nullptr;
    ani_method recordGetMethod = nullptr;
    ani_status status = env->FindClass(RECORD_CLASS_NAME, &recordCls);
    if (status != ANI_OK || recordCls == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "find record failed, status: %{public}d", status);
        return false;
    }
    status = env->Class_FindMethod(recordCls, "$_get", nullptr, &recordGetMethod);
    if (status != ANI_OK || recordGetMethod == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "find $_get failed, status: %{public}d", status);
        return false;
    }

    args.reserve(executeParam->methodParams_.size());
    for (const auto &encodedMethodParam : executeParam->methodParams_) {
        ani_ref valueRef = nullptr;
        if (!GetMethodArg(env, static_cast<ani_object>(wantParams), recordGetMethod, encodedMethodParam, valueRef)) {
            TAG_LOGE(AAFwkTag::INTENT, "GetMethodArg failed");
            return false;
        }
        ani_value arg { .r = valueRef };
        args.emplace_back(arg);
    }
    return true;
}

bool EtsInsightIntentFunc::IsVoidReturnType(const std::shared_ptr<InsightIntentExecuteParam> &executeParam) const
{
    if (executeParam == nullptr) {
        return false;
    }
    return !executeParam->methodReturnType_.empty() &&
        executeParam->methodReturnType_ == FUNCTION_RETURN_TYPE_VOID;
}

bool EtsInsightIntentFunc::ExecuteInsightIntent(ani_env *env,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam, bool &isAsync)
{
    std::vector<ani_value> args;
    if (!BuildMethodArgs(env, executeParam, args)) {
        return ExecuteIntentCheckError();
    }

    if (IsVoidReturnType(executeParam)) {
        ani_status status = env->Class_CallStaticMethodByName_Void_A(
            etsObj_->aniCls, executeParam->methodName_.c_str(), nullptr, args.empty() ? nullptr : args.data());
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "call static void method failed %{public}d", status);
            return ExecuteIntentCheckError();
        }
        return HandleVoidExecuteResult();
    }

    ani_ref result = nullptr;
    ani_status status = env->Class_CallStaticMethodByName_Ref_A(etsObj_->aniCls, executeParam->methodName_.c_str(),
        nullptr, &result, args.empty() ? nullptr : args.data());
    if (status != ANI_OK || result == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "call static method failed %{public}d", status);
        return ExecuteIntentCheckError();
    }

    return HandleExecuteResult(env, result, isAsync);
}

bool EtsInsightIntentFunc::HandleVoidExecuteResult()
{
    auto resultCpp = std::make_shared<AppExecFwk::InsightIntentExecuteResult>();
    resultCpp->result = std::make_shared<AAFwk::WantParams>();
    resultCpp->code = InsightIntentInnerErr::INSIGHT_INTENT_ERR_OK;
    ReplySucceededInner(resultCpp);
    return true;
}

bool EtsInsightIntentFunc::HandleExecuteResult(ani_env *env, ani_ref result, bool &isAsync)
{
    if (env == nullptr || result == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid env or result");
        return ExecuteIntentCheckError();
    }

    ani_class abilityUtilsCls = nullptr;
    ani_static_method isPromiseMethod = nullptr;
    ani_status status = env->FindClass(ABILITY_UTILS_CLASS_NAME, &abilityUtilsCls);
    if (status != ANI_OK || abilityUtilsCls == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "FindClass AbilityUtils failed %{public}d", status);
        return ExecuteIntentCheckError();
    }
    status = env->Class_FindStaticMethod(abilityUtilsCls, "isPromise", nullptr, &isPromiseMethod);
    if (status != ANI_OK || isPromiseMethod == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "find isPromise failed %{public}d", status);
        return ExecuteIntentCheckError();
    }
    ani_boolean isPromise = ANI_FALSE;
    status = env->Class_CallStaticMethod_Boolean(abilityUtilsCls, isPromiseMethod, &isPromise,
        static_cast<ani_object>(result));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "call isPromise failed %{public}d", status);
        return ExecuteIntentCheckError();
    }
    isAsync_ = static_cast<bool>(isPromise);
    isAsync = isAsync_;
    if (!isAsync_) {
        return HandleResultReturnedFromEtsFunc(env, result, false);
    }

    if (promiseHelperObj_ == nullptr || promiseHelperObj_->aniCls == nullptr || promiseHelperObj_->aniObj == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid promise helper");
        return ExecuteIntentCheckError();
    }

    ani_method callPromiseMethod = nullptr;
    status = env->Class_FindMethod(promiseHelperObj_->aniCls, "callPromise", CALL_PROMISE_SIGNATURE,
        &callPromiseMethod);
    if (status != ANI_OK || callPromiseMethod == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "find callPromise failed %{public}d", status);
        return ExecuteIntentCheckError();
    }

    status = env->Object_CallMethod_Void(promiseHelperObj_->aniObj, callPromiseMethod,
        reinterpret_cast<ani_object>(result));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "callPromise failed %{public}d", status);
        return ExecuteIntentCheckError();
    }
    return true;
}

void EtsInsightIntentFunc::ReplyFailedInner(InsightIntentInnerErr innerErr)
{
    state_ = State::INVALID;
    EtsInsightIntentUtils::ReplyFailed(callback_.release(), innerErr);
}

void EtsInsightIntentFunc::ReplySucceededInner(std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> resultCpp)
{
    state_ = State::EXECUTATION_DONE;
    EtsInsightIntentUtils::ReplySucceeded(callback_.release(), resultCpp);
}

bool EtsInsightIntentFunc::ExecuteIntentCheckError()
{
    ReplyFailedInner();
    STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
}

bool EtsInsightIntentFunc::HandleResultReturnedFromEtsFunc(ani_env *env, ani_ref result, bool isAsync)
{
    if (env == nullptr || result == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid env or result");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    ani_object resultRecord = isAsync ? static_cast<ani_object>(result) : CreateMethodResultRecord(env, result);
    if (resultRecord == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "create method result record failed");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    auto methodResult = std::make_shared<AAFwk::WantParams>();
    if (!AppExecFwk::UnwrapWantParams(env, reinterpret_cast<ani_ref>(resultRecord), *methodResult)) {
        TAG_LOGE(AAFwkTag::INTENT, "unwrap method result failed");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    auto resultCpp = std::make_shared<AppExecFwk::InsightIntentExecuteResult>();
    resultCpp->result = methodResult;
    resultCpp->code = InsightIntentInnerErr::INSIGHT_INTENT_ERR_OK;

    if (isAsync) {
        EtsInsightIntentUtils::ReplySucceeded(callback_.release(), resultCpp);
        return true;
    }

    ReplySucceededInner(resultCpp);
    return true;
}

void EtsInsightIntentFunc::ResolvePromiseCbCpp(ani_env *env, ani_object aniObj, ani_ref result)
{
    if (env == nullptr || aniObj == nullptr || result == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid resolve callback input");
        return;
    }

    ani_long nativeExecutor = 0;
    if (env->Object_GetFieldByName_Long(aniObj, "nativeExecutor", &nativeExecutor) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "get nativeExecutor failed");
        return;
    }

    auto *executor = reinterpret_cast<EtsInsightIntentFunc *>(nativeExecutor);
    if (executor == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null native executor");
        return;
    }
    executor->HandleResultReturnedFromEtsFunc(env, result, true);
}

void EtsInsightIntentFunc::RejectPromiseCbCpp(ani_env *env, ani_object aniObj, ani_ref error)
{
    (void)error;
    if (env == nullptr || aniObj == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid reject callback input");
        return;
    }

    ani_long nativeExecutor = 0;
    if (env->Object_GetFieldByName_Long(aniObj, "nativeExecutor", &nativeExecutor) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "get nativeExecutor failed");
        return;
    }

    auto *executor = reinterpret_cast<EtsInsightIntentFunc *>(nativeExecutor);
    if (executor == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null native executor");
        return;
    }
    executor->ReplyFailedInner();
}

} // namespace OHOS::AbilityRuntime

ETS_EXPORT extern "C" OHOS::AbilityRuntime::InsightIntentExecutor *OHOS_ETS_Insight_Intent_Func_Create(
    OHOS::AbilityRuntime::Runtime &runtime)
{
    return OHOS::AbilityRuntime::EtsInsightIntentFunc::Create(runtime);
}
