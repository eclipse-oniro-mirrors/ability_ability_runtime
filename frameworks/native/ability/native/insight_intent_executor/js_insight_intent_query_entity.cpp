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

#include "js_insight_intent_query_entity.h"

#include "hilog_tag_wrapper.h"
#include "insight_intent_constant.h"
#include "insight_intent_execute_result.h"
#include "int_wrapper.h"
#include "js_insight_intent_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi_common_util.h"
#include "napi_common_execute_result.h"
#include "napi_common_query_entity_param.h"
#include "napi_common_want.h"
#include "native_reference.h"
#include "path_utils.h"

#undef STATE_PATTERN_NAIVE_H
#define STATE_PATTERN_NAIVE_STATE state_
#include "state_pattern_naive.h"

#define TMP_NAPI_ANONYMOUS_FUNC "_"

namespace OHOS {
namespace AbilityRuntime {
std::shared_ptr<JsInsightIntentQueryEntity> JsInsightIntentQueryEntity::Create(JsRuntime& runtime)
{
    return std::make_shared<JsInsightIntentQueryEntity>(runtime);
}

JsInsightIntentQueryEntity::JsInsightIntentQueryEntity(JsRuntime& runtime) : runtime_(runtime)
{
    TAG_LOGD(AAFwkTag::INTENT, "constructor");
}

JsInsightIntentQueryEntity::~JsInsightIntentQueryEntity()
{
    state_ = State::DESTROYED;
    if (safeData_ != nullptr) {
        safeData_->SetAutoReleaseMem(true);
    }
    if (entityReference_ != nullptr) {
        runtime_.FreeNativeReference(std::move(entityReference_));
    }
    TAG_LOGI(AAFwkTag::INTENT, "destructor");
}

bool JsInsightIntentQueryEntity::Init(const InsightIntentExecutorInfo& insightIntentInfo)
{
    TAG_LOGD(AAFwkTag::INTENT, "Init");
    STATE_PATTERN_NAIVE_ACCEPT(State::CREATED, false);
    state_ = State::INITIALIZED;
    InsightIntentExecutor::Init(insightIntentInfo);

    HandleScope handleScope(runtime_);
    auto ret = JsInsightIntentQueryEntity::LoadJsCode(insightIntentInfo, runtime_);
    if (!ret) {
        TAG_LOGE(AAFwkTag::INTENT, "execute ohmurl failed");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    return true;
}

bool JsInsightIntentQueryEntity::HandleExecuteIntent(
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

    queryCallback_ = std::make_unique<JsInsightIntentQueryEntityCallback>();
    queryCallback_->queryType_ = executeParam->queryType_;
    queryCallback_->parameters_ = executeParam->queryParams_;
    queryCallback_->callback_ = std::move(callback);

    bool successful = ExecuteQueryEntity(executeParam);
    isAsync = true;
    return successful;
}

bool JsInsightIntentQueryEntity::LoadJsCode(const InsightIntentExecutorInfo& info, JsRuntime& runtime)
{
    TAG_LOGD(AAFwkTag::INTENT, "LoadJsCode start");

    auto executeParam = info.executeParam;
    if (executeParam == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null executeParam");
        return false;
    }
    if (!IsPathValid(executeParam->moduleName_) || !IsPathValid(executeParam->srcEntrance_)) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid moduleName or srcEntrance");
        return false;
    }
    if (!info.hapPath.empty() && !IsPathValid(info.hapPath)) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid hapPath");
        return false;
    }

    std::string moduleName(executeParam->moduleName_);
    std::string hapPath(info.hapPath);
    std::string srcEntrance(executeParam->srcEntrance_);
    TAG_LOGD(AAFwkTag::INTENT, "LoadJsCode moduleName:%{public}s, hapPath:%{public}s, srcEntrance:%{public}s",
        moduleName.c_str(), hapPath.c_str(), srcEntrance.c_str());

    safeData_ = runtime.ExecuteSecureWithOhmUrl(moduleName, hapPath, srcEntrance);
    if (safeData_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "execute failed");
        return false;
    }

    std::string className = executeParam->queryEntityClassName_;
    napi_value exportObj = runtime.GetExportObjectFromOhmUrl(srcEntrance, className);
    if (exportObj == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "get default object failed");
        return false;
    }

    auto* env = runtime.GetNapiEnv();
    napi_value instanceValue = nullptr;
    auto status = napi_new_instance(env, exportObj, 0, nullptr, &instanceValue);
    if (status != napi_ok || instanceValue == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "new instance failed %{public}d", status);
        return false;
    }

    napi_ref resultRef = nullptr;
    status = napi_create_reference(env, instanceValue, 1, &resultRef);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::INTENT, "create instance failed %{public}d", status);
        return false;
    }

    entityReference_ = std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference*>(resultRef));
    return true;
}

bool JsInsightIntentQueryEntity::ExecuteQueryEntity(std::shared_ptr<InsightIntentExecuteParam> executeParam)
{
    if (executeParam == nullptr || executeParam->queryParams_ == nullptr ||
        executeParam->queryEntityClassName_.empty()) {
        TAG_LOGE(AAFwkTag::INTENT, "null executeParam or queryEntityParam or className");
        return ExecuteIntentCheckError();
    }

    HandleScope handleScope(runtime_);
    auto env = runtime_.GetNapiEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return ExecuteIntentCheckError();
    }

    if (entityReference_ == nullptr || entityReference_->GetNapiValue() == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "get entity reference failed");
        return ExecuteIntentCheckError();
    }

    std::string methodName(INSIGHT_INTENT_QUERY_ENTITY_FUNC);
    napi_value entityObj = entityReference_->GetNapiValue();
    napi_value argv[] = { WrapQueryEntityParam(env, executeParam->queryType_, executeParam->queryParams_) };
    size_t argc = ArraySize(argv);

    auto method = GetTargetMethod(env, entityObj, methodName);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "get target method %{public}s failed", methodName.c_str());
        return ExecuteIntentCheckError();
    }

    napi_value result;
    auto status = napi_call_function(env, entityObj, method, argc, argv, &result);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::INTENT, "call function failed %{public}d", status);
        return ExecuteIntentCheckError();
    }

    return HandleResultReturnedFromJsFunc(result);
}

bool JsInsightIntentQueryEntity::HandleResultReturnedFromJsFunc(napi_value resultJs)
{
    TAG_LOGD(AAFwkTag::INTENT, "handle result returned");
    HandleScope handleScope(runtime_);
    auto* env = runtime_.GetNapiEnv();
    if (resultJs == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "resultJs is nullptr");
        return ExecuteIntentCheckError();
    }

    if (queryCallback_ == nullptr || queryCallback_->callback_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "queryCallback_ or callback is nullptr");
        return ExecuteIntentCheckError();
    }

    bool isPromise = false;
    NAPI_CALL_BASE(env, napi_is_promise(env, resultJs, &isPromise), ExecuteIntentCheckError());
    if (!isPromise) {
        TAG_LOGE(AAFwkTag::INTENT, "resultJs is not promise");
        return ExecuteIntentCheckError();
    }

    auto* callback = queryCallback_.release();
    napi_value then = nullptr;
    NAPI_CALL_BASE(env, napi_get_named_property(env, resultJs, "then", &then), ExecuteIntentCheckError());
    napi_value resolveCbJs = nullptr;
    NAPI_CALL_BASE(env, napi_create_function(env, TMP_NAPI_ANONYMOUS_FUNC, strlen(TMP_NAPI_ANONYMOUS_FUNC),
        HandleJsResultReturned, callback, &resolveCbJs), ExecuteIntentCheckError());
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
    return true;
}

bool JsInsightIntentQueryEntity::ExecuteIntentCheckError()
{
    TAG_LOGD(AAFwkTag::INTENT, "reply failed");
    state_ = State::INVALID;
    if (queryCallback_ == nullptr || queryCallback_->callback_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "queryCallback_ or callback is nullptr");
    } else {
        auto* callback = queryCallback_->callback_.release();
        JsInsightIntentUtils::ReplyFailed(callback, InsightIntentInnerErr::INSIGHT_INTENT_EXECUTE_REPLY_FAILED);
    }
    STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
}

napi_value JsInsightIntentQueryEntity::GetTargetMethod(napi_env env, napi_value constructor,
    const std::string &methodName)
{
    TAG_LOGD(AAFwkTag::INTENT, "methodName %{public}s", methodName.c_str());
    napi_value method;
    auto status = napi_get_named_property(env, constructor, methodName.c_str(), &method);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::INTENT, "get method failed %{public}d", status);
        return nullptr;
    }

    napi_value undefined;
    status = napi_get_undefined(env, &undefined);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::INTENT, "get undefined failed %{public}d", status);
        return nullptr;
    }

    bool isUndefined;
    status = napi_strict_equals(env, method, undefined, &isUndefined);
    if (isUndefined) {
        TAG_LOGE(AAFwkTag::INTENT, "target method %{public}s didn't exist", methodName.c_str());
        return nullptr;
    }

    return method;
}

napi_value JsInsightIntentQueryEntity::HandleJsResultReturned(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    constexpr size_t argc = 1;
    napi_value argv[argc] = {nullptr};
    size_t actualArgc = argc;
    void* data = nullptr;
    NAPI_CALL_BASE(env, napi_get_cb_info(env, info, &actualArgc, argv, nullptr, &data), nullptr);

    auto* entityCallback = static_cast<JsInsightIntentQueryEntityCallback*>(data);
    napi_value resultJs = argv[0];
    if (resultJs == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "callback invalid");
        JsInsightIntentUtils::ReplyFailed(entityCallback->callback_.release());
        return nullptr;
    }

    if (!AppExecFwk::IsTypeForNapiValue(env, resultJs, napi_valuetype::napi_object)) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid params");
        JsInsightIntentUtils::ReplyFailed(entityCallback->callback_.release());
        return nullptr;
    }

    AppExecFwk::ComplexArrayData jsArrayResult;
    if (!AppExecFwk::UnwrapArrayComplexFromJS(env, resultJs, jsArrayResult)) {
        TAG_LOGE(AAFwkTag::INTENT, "result is not array");
        JsInsightIntentUtils::ReplyFailed(entityCallback->callback_.release());
        return nullptr;
    }

    std::vector<std::shared_ptr<AAFwk::WantParams>> queryResults;
    for (size_t i = 0; i < jsArrayResult.objectList.size(); i++) {
        auto item = std::make_shared<AAFwk::WantParams>();
        if (!AppExecFwk::UnwrapWantParams(env, jsArrayResult.objectList[i], *item)) {
            TAG_LOGE(AAFwkTag::INTENT, "UnwrapWantParams failed");
            continue;
        }
        queryResults.push_back(item);
    }

    auto resultCpp = std::make_shared<AppExecFwk::InsightIntentExecuteResult>();
    resultCpp->isQueryEntity = true;
    resultCpp->queryResults = queryResults;
    JsInsightIntentUtils::ReplySucceeded(entityCallback->callback_.release(), resultCpp);
    return nullptr;
}
} // namespace AbilityRuntime
} // namespace OHOS
