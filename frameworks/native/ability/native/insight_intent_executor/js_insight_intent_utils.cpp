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

#include "js_insight_intent_utils.h"
#include "js_runtime_utils.h"
#include "event_report.h"
#include "hilog_tag_wrapper.h"
#include "insight_intent_execute_result.h"
#include "napi_common_execute_result.h"
#include "napi_common_util.h"

namespace OHOS {
namespace AbilityRuntime {
napi_value JsInsightIntentUtils::CallJsFunctionWithResult(
    napi_env env,
    napi_value obj,
    const char* funcName,
    size_t argc,
    const napi_value* argv)
{
    HandleEscape handleEscape(env);
    TAG_LOGD(AAFwkTag::INTENT, "call js function");
    napi_value result = nullptr;
    napi_value method = AppExecFwk::GetPropertyValueByPropertyName(env, obj, funcName, napi_valuetype::napi_function);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null method");
        return nullptr;
    }
    TAG_LOGI(AAFwkTag::INTENT, "call js function");
    auto status = napi_call_function(env, obj, method, argc, argv, &result);
    TAG_LOGI(AAFwkTag::INTENT, "call js function end");
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::INTENT, "napi call function failed %{public}d", status);
        return nullptr;
    }
    return handleEscape.Escape(result);
}

std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> JsInsightIntentUtils::GetResultFromJs(
    napi_env env, napi_value resultJs)
{
    TAG_LOGD(AAFwkTag::INTENT, "get result from js");
    auto resultCpp = std::make_shared<AppExecFwk::InsightIntentExecuteResult>();
    if (!UnwrapExecuteResult(env, resultJs, *resultCpp, true)) {
        // error log has printed
        return nullptr;
    }
    return resultCpp;
}

napi_value JsInsightIntentUtils::ResolveCbCpp(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::INTENT, "resolve function");
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

    auto resultCpp = JsInsightIntentUtils::GetResultFromJs(env, resultJs);
    JsInsightIntentUtils::ReplySucceeded(callback, resultCpp);
    return nullptr;
}

napi_value JsInsightIntentUtils::ResolveExecuteResultWithDelay(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::INTENT, "resolve function");
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

    auto resultCpp = JsInsightIntentUtils::GetResultFromJs(env, resultJs);
    resultCpp->isNeedDelayResult = true;
    JsInsightIntentUtils::ReplySucceeded(callback, resultCpp);
    return nullptr;
}

napi_value JsInsightIntentUtils::RejectCbCpp(napi_env env, napi_callback_info info)
{
    TAG_LOGW(AAFwkTag::INTENT, "reject function");
    constexpr size_t argc = 1;
    napi_value argv[argc] = { nullptr };
    size_t actualArgc = argc;
    void* data = nullptr;
    auto status = napi_get_cb_info(env, info, &actualArgc, argv, nullptr, &data);
    if (status != napi_ok) {
        TAG_LOGW(AAFwkTag::INTENT, "get cb info failed %{public}d", status);
    }

    auto* callback = static_cast<InsightIntentExecutorAsyncCallback*>(data);
    napi_value rejectJs = argv[0];
    if (rejectJs != nullptr) {
        auto rejectStr = StringifyObject(env, rejectJs);
        TAG_LOGW(AAFwkTag::INTENT, "reject %{public}s", rejectStr.c_str());
    }

    JsInsightIntentUtils::ReplyFailed(callback);
    return nullptr;
}

void JsInsightIntentUtils::ReplyFailed(InsightIntentExecutorAsyncCallback* callback,
    InsightIntentInnerErr innerErr)
{
    TAG_LOGE(AAFwkTag::INTENT, "reply failed");
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "callback null");
        return;
    }
    AppExecFwk::InsightIntentExecuteResult errorResult{};
    errorResult.innerErr = innerErr;
    AAFwk::EventInfo eventInfo;
    eventInfo.errCode = innerErr;
    eventInfo.errReason = "ReplyFailed";
    AAFwk::EventReport::SendExecuteIntentEvent(
        AAFwk::EventName::EXECUTE_INSIGHT_INTENT_ERROR, HISYSEVENT_FAULT, eventInfo);
    callback->Call(errorResult);
    delete callback;
}

void JsInsightIntentUtils::ReplySucceeded(InsightIntentExecutorAsyncCallback* callback,
    std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> resultCpp)
{
    TAG_LOGD(AAFwkTag::INTENT, "reply succeed");
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "callback null");
        return;
    }
    if (resultCpp == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "result cpp null");
        ReplyFailed(callback);
        return;
    }
    resultCpp->innerErr = InsightIntentInnerErr::INSIGHT_INTENT_ERR_OK;
    callback->Call(*resultCpp);
    delete callback;
}

std::string JsInsightIntentUtils::StringifyObject(napi_env env, napi_value result)
{
    TAG_LOGD(AAFwkTag::INTENT, "stringify object");
    HandleScope handleScope(env);
    napi_value global;
    auto status = napi_get_global(env, &global);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::INTENT, "get global failed %{public}d", status);
        return "";
    }

    napi_value jsonObj;
    status = napi_get_named_property(env, global, "JSON", &jsonObj);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::INTENT, "get JSON object failed %{public}d", status);
        return "";
    }

    napi_value stringifyFunc;
    status = napi_get_named_property(env, jsonObj, "stringify", &stringifyFunc);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::INTENT, "get stringify object failed %{public}d", status);
        return "";
    }

    napi_value stringifyResult;
    constexpr auto argc = 1;
    napi_value argv[argc] = { result };
    status = napi_call_function(env, jsonObj, stringifyFunc, argc, argv, &stringifyResult);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::INTENT, "call JSON.stringify failed %{public}d", status);
        return "";
    }

    std::string str;
    if (!ConvertFromJsValue(env, stringifyResult, str)) {
        TAG_LOGW(AAFwkTag::INTENT, "convert napi value failed");
        return "";
    }

    TAG_LOGD(AAFwkTag::INTENT, "stringify object %{private}s", str.c_str());
    return str;
}
} // namespace AbilityRuntime
} // namespace OHOS
