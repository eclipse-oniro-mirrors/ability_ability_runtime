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

#include "js_insight_intent_func.h"

#include <want_params.h>

#include "hilog_tag_wrapper.h"
#include "insight_intent_constant.h"
#include "insight_intent_execute_result.h"
#include "js_insight_intent_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi_common_execute_result.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "native_reference.h"
#include "string_wrapper.h"

#undef STATE_PATTERN_NAIVE_H
#define STATE_PATTERN_NAIVE_STATE state_
#include "state_pattern_naive.h"

#define TMP_NAPI_ANONYMOUS_FUNC "_"

namespace OHOS {
namespace AbilityRuntime {
std::shared_ptr<JsInsightIntentFunc> JsInsightIntentFunc::Create(JsRuntime& runtime)
{
    return std::make_shared<JsInsightIntentFunc>(runtime);
}

JsInsightIntentFunc::JsInsightIntentFunc(JsRuntime& runtime) : runtime_(runtime)
{
    TAG_LOGD(AAFwkTag::INTENT, "constructor");
}

JsInsightIntentFunc::~JsInsightIntentFunc()
{
    state_ = State::DESTROYED;
    if (safeData_ != nullptr) {
        safeData_->SetAutoReleaseMem(true);
    }
    TAG_LOGI(AAFwkTag::INTENT, "destructor");
}

bool JsInsightIntentFunc::Init(const InsightIntentExecutorInfo& insightIntentInfo)
{
    TAG_LOGD(AAFwkTag::INTENT, "Init");
    STATE_PATTERN_NAIVE_ACCEPT(State::CREATED, false);
    state_ = State::INITIALIZED;
    InsightIntentExecutor::Init(insightIntentInfo);

    HandleScope handleScope(runtime_);
    auto ret = JsInsightIntentFunc::LoadJsCode(insightIntentInfo, runtime_);
    if (!ret) {
        TAG_LOGE(AAFwkTag::INTENT, "execute ohmurl failed");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    return true;
}

bool JsInsightIntentFunc::HandleExecuteIntent(
    std::shared_ptr<InsightIntentExecuteParam> executeParam,
    const std::shared_ptr<NativeReference>& pageLoader,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback,
    bool& isAsync)
{
    STATE_PATTERN_NAIVE_ACCEPT(State::INITIALIZED, false);
    state_ = State::EXECUTING;

    if (callback == nullptr || callback->IsEmpty()) {
        TAG_LOGE(AAFwkTag::INTENT, "null callback");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    callback_ = std::move(callback);
    bool successful = ExecuteInsightIntent(executeParam);
    isAsync = isAsync_;
    return successful;
}

bool JsInsightIntentFunc::LoadJsCode(const InsightIntentExecutorInfo& info, JsRuntime& runtime)
{
    TAG_LOGD(AAFwkTag::INTENT, "load module");
    auto executeParam = info.executeParam;
    if (executeParam == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null executeParam");
        return false;
    }

    std::string moduleName(executeParam->moduleName_);
    std::string hapPath(info.hapPath);
    std::string srcEntrance(executeParam->srcEntrance_);
    TAG_LOGD(AAFwkTag::INTENT, "moduleName %{public}s, hapPath %{private}s, srcEntrance %{private}s",
        moduleName.c_str(), hapPath.c_str(), srcEntrance.c_str());

    safeData_ = runtime.ExecuteSecureWithOhmUrl(moduleName, hapPath, srcEntrance);
    return safeData_ != nullptr;
}

void JsInsightIntentFunc::ReplyFailedInner(InsightIntentInnerErr innerErr)
{
    TAG_LOGD(AAFwkTag::INTENT, "reply failed");
    state_ = State::INVALID;
    auto* callback = callback_.release();
    JsInsightIntentUtils::ReplyFailed(callback, innerErr);
}

void JsInsightIntentFunc::ReplySucceededInner(std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> resultCpp)
{
    TAG_LOGD(AAFwkTag::INTENT, "reply succeed");
    state_ = State::EXECUTATION_DONE;
    auto* callback = callback_.release();
    JsInsightIntentUtils::ReplySucceeded(callback, resultCpp);
}

bool JsInsightIntentFunc::HandleResultReturnedFromJsFunc(napi_value resultJs)
{
    TAG_LOGD(AAFwkTag::INTENT, "handle result returned");
    HandleScope handleScope(runtime_);
    auto* env = runtime_.GetNapiEnv();
    if (resultJs == nullptr) {
        return ExecuteIntentCheckError();
    }

    bool isPromise = false;
    NAPI_CALL_BASE(env, napi_is_promise(env, resultJs, &isPromise), ExecuteIntentCheckError());
    isAsync_ = isPromise;

    if (isPromise) {
        TAG_LOGI(AAFwkTag::INTENT, "Is promise");
        auto* callback = callback_.release();

        napi_value then = nullptr;
        NAPI_CALL_BASE(env, napi_get_named_property(env, resultJs, "then", &then), ExecuteIntentCheckError());
        napi_value resolveCbJs = nullptr;
        NAPI_CALL_BASE(env, napi_create_function(env, TMP_NAPI_ANONYMOUS_FUNC, strlen(TMP_NAPI_ANONYMOUS_FUNC),
            ResolveCbCpp, callback, &resolveCbJs), ExecuteIntentCheckError());
        constexpr size_t argcThen = 1;
        napi_value argvThen[argcThen] = { resolveCbJs };
        NAPI_CALL_BASE(env, napi_call_function(env, resultJs, then, argcThen, argvThen, nullptr),
            ExecuteIntentCheckError());

        napi_value promiseCatch = nullptr;
        NAPI_CALL_BASE(env, napi_get_named_property(env, resultJs, "catch", &promiseCatch), ExecuteIntentCheckError());
        napi_value rejectCbJs = nullptr;
        NAPI_CALL_BASE(env, napi_create_function(env, TMP_NAPI_ANONYMOUS_FUNC, strlen(TMP_NAPI_ANONYMOUS_FUNC),
            JsInsightIntentUtils::RejectCbCpp, callback, &rejectCbJs), ExecuteIntentCheckError());
        constexpr size_t argcCatch = 1;
        napi_value argvCatch[argcCatch] = { rejectCbJs };
        NAPI_CALL_BASE(env, napi_call_function(env, resultJs, promiseCatch, argcCatch, argvCatch, nullptr),
            ExecuteIntentCheckError());
    } else {
        TAG_LOGI(AAFwkTag::INTENT, "Not promise");
        auto resultCpp = GetResultFromJs(env, resultJs);
        if (resultCpp == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "null resultCpp");
            return ExecuteIntentCheckError();
        }
        TAG_LOGD(AAFwkTag::INTENT, "Call succeed");
        ReplySucceededInner(resultCpp);
    }
    return true;
}

std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> JsInsightIntentFunc::GetResultFromJs(
    napi_env env, napi_value resultJs)
{
    TAG_LOGD(AAFwkTag::INTENT, "Get result for intent func");
    auto resultCpp = std::make_shared<AppExecFwk::InsightIntentExecuteResult>();
    auto resultStr = JsInsightIntentUtils::StringifyObject(env, resultJs);
    auto wantParams = std::make_shared<AAFwk::WantParams>();
    wantParams->SetParam("methodResult", AAFwk::String::Box(resultStr));
    resultCpp->result = wantParams;
    resultCpp->code = InsightIntentInnerErr::INSIGHT_INTENT_ERR_OK;
    return resultCpp;
}

napi_value JsInsightIntentFunc::ResolveCbCpp(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::INTENT, "Resolve function");
    constexpr size_t argc = 1;
    napi_value argv[argc] = {nullptr};
    size_t actualArgc = argc;
    void* data = nullptr;
    NAPI_CALL_BASE(env, napi_get_cb_info(env, info, &actualArgc, argv, nullptr, &data), nullptr);

    auto* callback = static_cast<InsightIntentExecutorAsyncCallback*>(data);
    napi_value resultJs = argv[0];
    if (resultJs == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "callback invalid");
        JsInsightIntentUtils::ReplyFailed(callback);
        return nullptr;
    }

    auto resultCpp = GetResultFromJs(env, resultJs);
    JsInsightIntentUtils::ReplySucceeded(callback, resultCpp);
    return nullptr;
}

bool JsInsightIntentFunc::ExecuteIntentCheckError()
{
    ReplyFailedInner();
    STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
}

bool JsInsightIntentFunc::ExecuteInsightIntent(std::shared_ptr<InsightIntentExecuteParam> executeParam)
{
    if (executeParam == nullptr || executeParam->insightIntentParam_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null executeParam");
        return ExecuteIntentCheckError();
    }

    auto name = executeParam->insightIntentName_;
    TAG_LOGD(AAFwkTag::INTENT, "execute intent %{public}s", name.c_str());

    HandleScope handleScope(runtime_);
    auto env = runtime_.GetNapiEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return ExecuteIntentCheckError();
    }

    std::string srcEntrance = executeParam->srcEntrance_;
    std::string className = executeParam->className_;
    std::string methodName = executeParam->methodName_;
    std::vector<std::string> methodParams = executeParam->methodParams_;
    std::unordered_map<std::string, size_t> paramMap;
    for (size_t i = 0; i < methodParams.size(); i++) {
        paramMap[methodParams[i]] = i;
    }

    size_t argc = 0;
    std::vector<napi_value> argv(1);
    auto param = executeParam->insightIntentParam_;
    auto ret = ParseParams(env, *param, paramMap, argc, argv);
    if (!ret) {
        TAG_LOGE(AAFwkTag::INTENT, "parse params failed");
        return ExecuteIntentCheckError();
    }

    napi_value constructor = runtime_.GetExportObjectFromOhmUrl(srcEntrance, className);
    if (constructor == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "get class %{public}s failed", className.c_str());
        return ExecuteIntentCheckError();
    }

    auto method = GetTargetMethod(env, constructor, methodName);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "get target method %{public}s failed", methodName.c_str());
        return ExecuteIntentCheckError();
    }

    // Call target method
    napi_value result;
    auto status = napi_call_function(env, constructor, method, argc, argv.data(), &result);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::INTENT, "call function failed %{public}d", status);
        return ExecuteIntentCheckError();
    }

    return HandleResultReturnedFromJsFunc(result);
}

bool JsInsightIntentFunc::ParseParams(napi_env env, const AAFwk::WantParams& param,
    const std::unordered_map<std::string, size_t> &paramMap, size_t &argc, std::vector<napi_value> &argv)
{
    AAFwk::WantParamWrapper wrapper(param);
    std::string parametersString = wrapper.ToString();
    TAG_LOGD(AAFwkTag::INTENT, "param string %{private}s", parametersString.c_str());

    napi_value srcObj = AppExecFwk::WrapWantParams(env, param);
    if (srcObj == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "wrap params failed");
        return false;
    }

    napi_value propertys;
    NAPI_CALL_BASE(env, napi_get_property_names(env, srcObj, &propertys), false);

    uint32_t propCnt;
    NAPI_CALL_BASE(env, napi_get_array_length(env, propertys, &propCnt), false);

    argc = paramMap.size();
    argv.resize(argc);

    for (uint32_t i = 0; i < propCnt; i++) {
        napi_value propKey;
        NAPI_CALL_BASE(env, napi_get_element(env, propertys, i, &propKey), false);

        std::string propName;
        if (!ConvertFromJsValue(env, propKey, propName)) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "convert napi value failed");
            return false;
        }
        TAG_LOGD(AAFwkTag::INTENT, "param %{public}s", propName.c_str());

        napi_value propValue;
        NAPI_CALL_BASE(env, napi_get_property(env, srcObj, propKey, &propValue), false);

        auto it = paramMap.find(propName);
        if (it != paramMap.end()) {
            auto iter = it->second;
            TAG_LOGD(AAFwkTag::INTENT, "param %{public}s matched, id %{public}zu", propName.c_str(), iter);
            argv[iter] = propValue;
        }
    }

    return true;
}

napi_value JsInsightIntentFunc::GetTargetMethod(napi_env env, napi_value constructor, const std::string &methodName)
{
    TAG_LOGD(AAFwkTag::INTENT, "methodName %{private}s", methodName.c_str());
    napi_value method;
    auto status = napi_get_named_property(env, constructor, methodName.c_str(), &method);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::INTENT, "get method failed %{public}d", status);
        return nullptr;
    }

    // Check validity of method
    napi_value undefined;
    status = napi_get_undefined(env, &undefined);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::INTENT, "get undefined failed %{public}d", status);
        return nullptr;
    }

    bool isUndefined;
    status = napi_strict_equals(env, method, undefined, &isUndefined);
    if (isUndefined) {
        TAG_LOGE(AAFwkTag::INTENT, "target method %{private}s didn't exist", methodName.c_str());
        return nullptr;
    }

    return method;
}
} // namespace AbilityRuntime
} // namespace OHOS
