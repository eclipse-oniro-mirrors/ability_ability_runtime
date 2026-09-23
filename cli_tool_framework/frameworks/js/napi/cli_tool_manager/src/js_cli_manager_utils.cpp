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

#include "js_cli_manager_utils.h"

#include <nlohmann/json.hpp>

#include "cli_session_info.h"
#include "cli_tool_event.h"
#include "exec_cmd_param.h"
#include "exec_options.h"
#include "exec_result.h"
#include "exec_result_wrap.h"
#include "exec_tool_param.h"
#include "hilog_tag_wrapper.h"
#include "icli_tool_data.h"
#include "js_cli_session_event_callback.h"
#include "napi_common_util.h"
#include "napi_common_want.h"

using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace CliTool {
namespace {
constexpr int64_t MAX_TIMEOUT = 30 * 60; // 30 m

// Forward declarations
napi_value ConvertJsonToNapiValue(napi_env env, const nlohmann::json &value);

napi_value ParseJsonStringToJsObject(napi_env env, const std::string &jsonStr)
{
    napi_value jsObj = nullptr;
    napi_create_object(env, &jsObj);

    if (jsonStr.empty()) {
        return jsObj;
    }

    nlohmann::json jsonObj = nlohmann::json::parse(jsonStr, nullptr, false);
    if (jsonObj.is_discarded()) {
        return jsObj;
    }

    napi_value jsValue = nullptr;
    if (jsonObj.is_object()) {
        napi_create_object(env, &jsValue);
        for (auto it = jsonObj.begin(); it != jsonObj.end(); ++it) {
            napi_value jsItem = ConvertJsonToNapiValue(env, it.value());
            napi_set_named_property(env, jsValue, it.key().c_str(), jsItem);
        }
    } else if (jsonObj.is_array()) {
        napi_status status = napi_create_array(env, &jsValue);
        if (status != napi_ok) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "napi_create_array failed, %{public}d", status);
            napi_create_object(env, &jsValue);
            return jsValue;
        }
        size_t index = 0;
        for (auto &item : jsonObj) {
            napi_value jsItem = ConvertJsonToNapiValue(env, item);
            napi_set_element(env, jsValue, index++, jsItem);
        }
    } else {
        napi_create_object(env, &jsValue);
    }

    return jsValue;
}

// Optional trace id; an empty string is a valid provided value.
bool UnwrapOptionalTraceId(napi_env env, napi_value obj, const char *name,
    std::string &value, std::string &msg)
{
    bool hasProperty = false;
    if (napi_has_named_property(env, obj, name, &hasProperty) != napi_ok) {
        msg = std::string("has ") + name + " failed";
        TAG_LOGE(AAFwkTag::CLI_TOOL, "%{public}s", msg.c_str());
        return false;
    }
    if (!hasProperty) {
        return true;
    }
    napi_value prop = nullptr;
    if (napi_get_named_property(env, obj, name, &prop) != napi_ok) {
        msg = std::string("invalid ") + name + " property";
        TAG_LOGE(AAFwkTag::CLI_TOOL, "%{public}s", msg.c_str());
        return false;
    }
    // Explicit undefined/null is treated as absent (spec AC-1.7: "not provided").
    napi_valuetype valueType = napi_undefined;
    if (napi_typeof(env, prop, &valueType) != napi_ok) {
        msg = std::string("invalid ") + name + " property";
        TAG_LOGE(AAFwkTag::CLI_TOOL, "%{public}s", msg.c_str());
        return false;
    }
    if (valueType == napi_undefined || valueType == napi_null) {
        return true;
    }
    std::string traceId;
    if (!AppExecFwk::UnwrapStringFromJS2(env, prop, traceId)) {
        msg = std::string("Parameter error. The type of \"") + name + "\" must be string.";
        TAG_LOGE(AAFwkTag::CLI_TOOL, "%{public}s", msg.c_str());
        return false;
    }
    if (!IsValidTraceId(traceId)) {
        msg = std::string("Parameter error. The ") + name + " is invalid.";
        TAG_LOGE(AAFwkTag::CLI_TOOL, "%{public}s", msg.c_str());
        return false;
    }
    value = std::move(traceId);
    return true;
}

// Helper: Convert single JSON value to N-API value
napi_value ConvertJsonToNapiValue(napi_env env, const nlohmann::json &value)
{
    if (value.is_string()) {
        return AppExecFwk::WrapStringToJS(env, value.get<std::string>());
    } else if (value.is_number_integer()) {
        return AppExecFwk::WrapInt32ToJS(env, value.get<int32_t>());
    } else if (value.is_boolean()) {
        return AppExecFwk::WrapBoolToJS(env, value.get<bool>());
    } else if (value.is_object() || value.is_array()) {
        return ParseJsonStringToJsObject(env, value.dump());
    }
    napi_value undefined;
    napi_get_undefined(env, &undefined);
    return undefined;
}

bool UnwrapStringProperty(napi_env env, napi_value obj, const char* name, std::string& out)
{
    bool hasProperty = false;
    if (napi_has_named_property(env, obj, name, &hasProperty) != napi_ok || !hasProperty) {
        return false;
    }
    napi_value prop = nullptr;
    napi_get_named_property(env, obj, name, &prop);
    return AppExecFwk::UnwrapStringFromJS2(env, prop, out);
}

bool UnwrapBoolProperty(napi_env env, napi_value obj, const char* name, bool& out)
{
    bool hasProperty = false;
    if (napi_has_named_property(env, obj, name, &hasProperty) != napi_ok || !hasProperty) {
        return false;
    }
    napi_value prop = nullptr;
    napi_get_named_property(env, obj, name, &prop);
    return AppExecFwk::UnwrapBoolFromJS2(env, prop, out);
}

bool UnwrapInt64Property(napi_env env, napi_value obj, const char* name, int64_t& out)
{
    bool hasProperty = false;
    if (napi_has_named_property(env, obj, name, &hasProperty) != napi_ok || !hasProperty) {
        return false;
    }
    napi_value prop = nullptr;
    napi_get_named_property(env, obj, name, &prop);
    return AppExecFwk::UnwrapInt64FromJS2(env, prop, out);
}
}
bool UnwrapStringMap(napi_env env, napi_value obj,
    std::map<std::string, std::string> &values)
{
    if (obj == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Input object is null");
        return false;
    }

    napi_valuetype valueType = napi_undefined;
    napi_status status = napi_typeof(env, obj, &valueType);
    if (status != napi_ok || valueType != napi_object) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Input is not an object");
        return false;
    }

    napi_value propertyNames = nullptr;
    if (napi_get_property_names(env, obj, &propertyNames) != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to get property names");
        return false;
    }
    uint32_t propertyCount = 0;
    if (napi_get_array_length(env, propertyNames, &propertyCount) != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to get array length");
        return false;
    }

    values.clear();
    for (uint32_t i = 0; i < propertyCount; i++) {
        napi_value key = nullptr;
        if (napi_get_element(env, propertyNames, i, &key) != napi_ok) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to get element");
            return false;
        }

        std::string keyStr;
        if (!AppExecFwk::UnwrapStringFromJS2(env, key, keyStr)) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to unwrap key");
            return false;
        }

        napi_value value = nullptr;
        if (napi_get_named_property(env, obj, keyStr.c_str(), &value) != napi_ok) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to get value");
            return false;
        }

        std::string valueStr;
        if (!AppExecFwk::UnwrapStringFromJS2(env, value, valueStr)) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to unwrap value");
            return false;
        }

        values.emplace(std::make_pair(keyStr, valueStr));
    }

    return true;
}

bool UnwrapExecOptions(napi_env env, napi_value obj, ExecOptions &options, std::string &msg)
{
    if (obj == nullptr) {
        return true;
    }

    napi_valuetype valueType = napi_undefined;
    napi_status status = napi_typeof(env, obj, &valueType);
    if (status != napi_ok || valueType != napi_object) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Input is not an object");
        return false;
    }

    bool hasProperty = false;
    if (napi_has_named_property(env, obj, "background", &hasProperty) != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "has background failed");
        return false;
    }
    if (hasProperty) {
        napi_value backgroundProp = nullptr;
        if (napi_get_named_property(env, obj, "background", &backgroundProp) != napi_ok) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "invalid background property");
            return false;
        }
        if (!AppExecFwk::UnwrapBoolFromJS2(env, backgroundProp, options.background)) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "unwrap background failed");
            return false;
        }
    }

    if (napi_has_named_property(env, obj, "yieldMs", &hasProperty) != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "has yieldMs failed");
        return false;
    }
    if (hasProperty) {
        napi_value yieldMsProp = nullptr;
        if (napi_get_named_property(env, obj, "yieldMs", &yieldMsProp) != napi_ok) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "invalid yieldMs property");
            return false;
        }
        if (!AppExecFwk::UnwrapInt64FromJS2(env, yieldMsProp, options.yieldMs)) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "unwrap yieldMs failed");
            return false;
        }
    }

    if (napi_has_named_property(env, obj, "timeout", &hasProperty) != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "has timeout failed");
        return false;
    }
    if (hasProperty) {
        napi_value timeoutProp = nullptr;
        if (napi_get_named_property(env, obj, "timeout", &timeoutProp) != napi_ok) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "invalid timeout property");
            return false;
        }
        if (!AppExecFwk::UnwrapInt64FromJS2(env, timeoutProp, options.timeout)) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "unwrap timeout failed");
            return false;
        }
    } else {
        options.timeout = MAX_TIMEOUT;
    }
    if (options.timeout < 0 || options.timeout > MAX_TIMEOUT || options.yieldMs < 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ExecOptions timeout/yieldMs out of range");
        return false;
    }

    if (!UnwrapOptionalTraceId(env, obj, "toolCallId", options.toolCallId, msg)) {
        return false;
    }

    if (!UnwrapOptionalTraceId(env, obj, "dmSessionId", options.dmSessionId, msg)) {
        return false;
    }
    return true;
}

bool UnwrapExecCmdOptions(napi_env env, napi_value obj, ExecCmdOptions &options, std::string &msg)
{
    if (obj == nullptr) {
        return true;
    }
    napi_valuetype valueType = napi_undefined;
    napi_status status = napi_typeof(env, obj, &valueType);
    if (status != napi_ok || valueType != napi_object) {
        msg = "ExecCmdOptions is not an object";
        TAG_LOGE(AAFwkTag::CLI_TOOL, "%{public}s", msg.c_str());
        return false;
    }
    UnwrapStringProperty(env, obj, "workDir", options.workDir);

    bool hasProperty = false;
    if (napi_has_named_property(env, obj, "env", &hasProperty) == napi_ok && hasProperty) {
        napi_value prop = nullptr;
        napi_get_named_property(env, obj, "env", &prop);
        if (prop != nullptr) {
            UnwrapStringFromRecord(env, prop, options.env);
        }
    }

    UnwrapStringProperty(env, obj, "policy", options.policy);
    UnwrapBoolProperty(env, obj, "background", options.background);
    UnwrapInt64Property(env, obj, "yieldMs", options.yieldMs);
    UnwrapInt64Property(env, obj, "timeout", options.timeout);
    if (options.timeout < 0 || options.timeout > MAX_TIMEOUT || options.yieldMs < 0) {
        msg = "ExecCmdOptions timeout/yieldMs out of range";
        TAG_LOGE(AAFwkTag::CLI_TOOL, "%{public}s", msg.c_str());
        return false;
    }
    UnwrapBoolProperty(env, obj, "isShellCommand", options.isShellCommand);
    UnwrapStringProperty(env, obj, "challenge", options.challenge);
    // Optional trace identifiers: absent / undefined / null means not provided;
    // a present value must satisfy the shared trace-id rule (1~256 [A-Za-z0-9_-]).
    std::string traceMsg;
    if (!UnwrapOptionalTraceId(env, obj, "toolCallId", options.toolCallId, traceMsg) ||
        !UnwrapOptionalTraceId(env, obj, "dmSessionId", options.dmSessionId, traceMsg)) {
        msg = traceMsg;
        TAG_LOGE(AAFwkTag::CLI_TOOL, "%{public}s", msg.c_str());
        return false;
    }
    return true;
}

bool UnwrapExecCmdParam(napi_env env, napi_value obj, ExecCmdParam &param,
    std::shared_ptr<JsCliSessionEventCallbackImpl>& callback, std::string &msg)
{
    if (obj == nullptr) {
        return true;
    }

    napi_valuetype valueType = napi_undefined;
    napi_status status = napi_typeof(env, obj, &valueType);
    if (status != napi_ok || valueType != napi_object) {
        msg = "Input is not an object";
        TAG_LOGE(AAFwkTag::CLI_TOOL, "%{public}s", msg.c_str());
        return false;
    }

    bool hasProperty = false;
    param.execCmdOptions.timeout = MAX_TIMEOUT;
    if (!UnwrapExecCmdOptions(env, obj, param.execCmdOptions, msg)) {
        if (msg.empty()) {
            msg = "Invalid ExecCmdOptions";
        }
        TAG_LOGE(AAFwkTag::CLI_TOOL, "%{public}s", msg.c_str());
        return false;
    }

    if (napi_has_named_property(env, obj, "callback", &hasProperty) != napi_ok) {
        msg = "has callback failed";
        TAG_LOGE(AAFwkTag::CLI_TOOL, "%{public}s", msg.c_str());
        return false;
    }
    if (hasProperty) {
        napi_value callbackProp = nullptr;
        if (napi_get_named_property(env, obj, "callback", &callbackProp) != napi_ok) {
            msg = "invalid callback property";
            TAG_LOGE(AAFwkTag::CLI_TOOL, "%{public}s", msg.c_str());
            return false;
        }
        if (!IsValidToolEventCallback(env, callbackProp)) {
            msg = "invalid callback property";
            TAG_LOGE(AAFwkTag::CLI_TOOL, "%{public}s", msg.c_str());
            return false;
        }
        callback = std::make_shared<JsCliSessionEventCallbackImpl>(env, callbackProp);
        if (callback == nullptr || !callback->IsValid()) {
            msg = "invalid callback property";
            TAG_LOGE(AAFwkTag::CLI_TOOL, "%{public}s", msg.c_str());
            return false;
        }
    }

    return true;
}

bool UnwrapStringFromRecord(napi_env env, napi_value obj, std::string &paramEnv)
{
    if (obj == nullptr) {
        return false;
    }
    napi_valuetype valueType = napi_undefined;
    napi_status status = napi_typeof(env, obj, &valueType);
    if (status != napi_ok || valueType != napi_object) {
        return false;
    }
    std::vector<std::string> propNames;
    napi_value array = nullptr;
    if (napi_get_property_names(env, obj, &array) != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to get property names");
        return false;
    }
    if (!ParseArrayStringValue(env, array, propNames)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "invalid propNames");
        return false;
    }
    std::map<std::string, std::string> recordList;
    for (const auto &propName : propNames) {
        napi_value prop = nullptr;
        if (napi_get_named_property(env, obj, propName.c_str(), &prop) != napi_ok) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "napi_get_named_property failed");
            return false;
        }
        if (!CheckTypeForNapiValue(env, prop, napi_string)) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "prop not string: %{public}s", propName.c_str());
            return false;
        }
        std::string valName;
        if (!ConvertFromJsValue(env, prop, valName)) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "convert valName failed: %{public}s", propName.c_str());
            return false;
        }
        TAG_LOGD(AAFwkTag::CLI_TOOL, "key: %{public}s, value: %{public}s", propName.c_str(),
            valName.c_str());
        recordList.emplace(propName, valName);
    }

    nlohmann::json jsons;
    for (const auto &record : recordList) {
        jsons[record.first] = record.second;
    }
    paramEnv = jsons.dump();
    return true;
}

bool ParseArrayStringValue(napi_env env, napi_value array, std::vector<std::string> &vector)
{
    if (array == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "null array");
        return false;
    }
    bool isArray = false;
    if (napi_is_array(env, array, &isArray) != napi_ok || !isArray) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "not array");
        return false;
    }

    uint32_t arrayLen = 0;
    napi_get_array_length(env, array, &arrayLen);
    if (arrayLen == 0) {
        return true;
    }
    vector.reserve(arrayLen);
    for (uint32_t i = 0; i < arrayLen; i++) {
        std::string strItem;
        napi_value jsValue = nullptr;
        if (napi_get_element(env, array, i, &jsValue) != napi_ok) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to get element");
            return false;
        }
        if (!ConvertFromJsValue(env, jsValue, strItem)) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "convert jsValue failed, i: %{public}u", i);
            return false;
        }
        vector.emplace_back(std::move(strItem));
    }
    return true;
}

napi_value CreateJsCliSessionInfo(napi_env env, const CliSessionInfo &session)
{
    AbilityRuntime::HandleEscape handleEscape(env);
    napi_value jsObj = nullptr;
    napi_status status = napi_create_object(env, &jsObj);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to create JS CliSessionInfo");
        return nullptr;
    }

    napi_set_named_property(env, jsObj, "sessionId", AppExecFwk::WrapStringToJS(env, session.sessionId));
    napi_set_named_property(env, jsObj, "toolName", AppExecFwk::WrapStringToJS(env, session.toolName));
    napi_set_named_property(env, jsObj, "status", AppExecFwk::WrapStringToJS(env, session.status));

    // Set result if present. Delegate field wrapping to CreateJsExecResult so the
    // ExecResult -> JS mapping stays single-sourced (shared with the hook path).
    if (session.status != "running" && session.result != nullptr) {
        napi_value jsResult = CreateJsExecResult(env, *session.result);
        if (jsResult == nullptr) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to create JS ExecResult");
            return nullptr;
        }
        napi_set_named_property(env, jsObj, "result", jsResult);
    }

    return handleEscape.Escape(jsObj);
}

napi_value CreateJsExecResult(napi_env env, const ExecResult &result)
{
    napi_value jsObj = nullptr;
    napi_create_object(env, &jsObj);
    // Per the public ExecResult API contract (cliManager.d.ts) exitCode/outputText/
    // errorText/signalNumber are optional; omit them when they carry no meaningful
    // value so the JS shape matches the contract. timeOut/executionTime are required.
    if (!result.timeout) {
        napi_set_named_property(env, jsObj, "exitCode", AppExecFwk::WrapInt32ToJS(env, result.exitCode));
    }
    if (!result.outputText.empty()) {
        napi_set_named_property(env, jsObj, "outputText", AppExecFwk::WrapStringToJS(env, result.outputText));
    }
    if (!result.errorText.empty()) {
        napi_set_named_property(env, jsObj, "errorText", AppExecFwk::WrapStringToJS(env, result.errorText));
    }
    if (result.signalNumber != 0) {
        napi_set_named_property(env, jsObj, "signalNumber", AppExecFwk::WrapInt32ToJS(env, result.signalNumber));
    }
    napi_set_named_property(env, jsObj, "timeOut", AppExecFwk::WrapBoolToJS(env, result.timeout));
    napi_set_named_property(env, jsObj, "executionTime", AppExecFwk::WrapInt64ToJS(env, result.executionTime));
    return jsObj;
}

void UnwrapExecResult(napi_env env, napi_value jsObj, ExecResult &result)
{
    if (jsObj == nullptr) {
        return;
    }
    bool hasProp = false;
    if (napi_has_named_property(env, jsObj, "exitCode", &hasProp) == napi_ok && hasProp) {
        napi_value prop = nullptr;
        napi_get_named_property(env, jsObj, "exitCode", &prop);
        AppExecFwk::UnwrapInt32FromJS2(env, prop, result.exitCode);
    }
    if (napi_has_named_property(env, jsObj, "outputText", &hasProp) == napi_ok && hasProp) {
        napi_value prop = nullptr;
        napi_get_named_property(env, jsObj, "outputText", &prop);
        AppExecFwk::UnwrapStringFromJS2(env, prop, result.outputText);
    }
    if (napi_has_named_property(env, jsObj, "errorText", &hasProp) == napi_ok && hasProp) {
        napi_value prop = nullptr;
        napi_get_named_property(env, jsObj, "errorText", &prop);
        AppExecFwk::UnwrapStringFromJS2(env, prop, result.errorText);
    }
    if (napi_has_named_property(env, jsObj, "signalNumber", &hasProp) == napi_ok && hasProp) {
        napi_value prop = nullptr;
        napi_get_named_property(env, jsObj, "signalNumber", &prop);
        AppExecFwk::UnwrapInt32FromJS2(env, prop, result.signalNumber);
    }
    if (napi_has_named_property(env, jsObj, "timeOut", &hasProp) == napi_ok && hasProp) {
        napi_value prop = nullptr;
        napi_get_named_property(env, jsObj, "timeOut", &prop);
        AppExecFwk::UnwrapBoolFromJS2(env, prop, result.timeout);
    }
    if (napi_has_named_property(env, jsObj, "executionTime", &hasProp) == napi_ok && hasProp) {
        napi_value prop = nullptr;
        napi_get_named_property(env, jsObj, "executionTime", &prop);
        AppExecFwk::UnwrapInt64FromJS2(env, prop, result.executionTime);
    }
}

napi_value CreateJsExecOptions(napi_env env, const ExecOptions &opts)
{
    napi_value jsObj = nullptr;
    napi_create_object(env, &jsObj);
    napi_set_named_property(env, jsObj, "background", AppExecFwk::WrapBoolToJS(env, opts.background));
    napi_set_named_property(env, jsObj, "yieldMs", AppExecFwk::WrapInt64ToJS(env, opts.yieldMs));
    napi_set_named_property(env, jsObj, "timeout", AppExecFwk::WrapInt64ToJS(env, opts.timeout));
    // Trace identifiers: only surfaced when provided so the before-hook can
    // observe them (read-only for the hook; modifications do not propagate).
    if (!opts.toolCallId.empty()) {
        napi_set_named_property(env, jsObj, "toolCallId", AppExecFwk::WrapStringToJS(env, opts.toolCallId));
    }
    if (!opts.dmSessionId.empty()) {
        napi_set_named_property(env, jsObj, "dmSessionId", AppExecFwk::WrapStringToJS(env, opts.dmSessionId));
    }
    return jsObj;
}

napi_value CreateJsExecToolParam(napi_env env, const ExecToolParam &param)
{
    napi_value jsObj = nullptr;
    napi_create_object(env, &jsObj);
    napi_set_named_property(env, jsObj, "toolName", AppExecFwk::WrapStringToJS(env, param.toolName));
    napi_set_named_property(env, jsObj, "subCommand", AppExecFwk::WrapStringToJS(env, param.subcommand));
    napi_set_named_property(env, jsObj, "challenge", AppExecFwk::WrapStringToJS(env, param.challenge));
    napi_value jsArgs = AppExecFwk::WrapWantParams(env, param.args);
    napi_set_named_property(env, jsObj, "args", jsArgs);
    napi_set_named_property(env, jsObj, "execOptions", CreateJsExecOptions(env, param.options));
    return jsObj;
}

napi_value CreateJsExecCmdParam(napi_env env, const ExecCmdParam &param)
{
    napi_value jsObj = nullptr;
    napi_create_object(env, &jsObj);
    napi_set_named_property(env, jsObj, "cmd", AppExecFwk::WrapStringToJS(env, param.cmd));
    // Matches CliHook.d.ts ExecCmdParam { cmd, execCmdOptions }. env is a JSON
    // string natively but Record<string,string> on the JS side.
    napi_value jsOpts = nullptr;
    napi_create_object(env, &jsOpts);
    napi_set_named_property(env, jsOpts, "workDir",
        AppExecFwk::WrapStringToJS(env, param.execCmdOptions.workDir));
    if (!param.execCmdOptions.env.empty()) {
        napi_set_named_property(env, jsOpts, "env",
            ParseJsonStringToJsObject(env, param.execCmdOptions.env));
    }
    napi_set_named_property(env, jsOpts, "policy",
        AppExecFwk::WrapStringToJS(env, param.execCmdOptions.policy));
    napi_set_named_property(env, jsOpts, "background",
        AppExecFwk::WrapBoolToJS(env, param.execCmdOptions.background));
    napi_set_named_property(env, jsOpts, "yieldMs",
        AppExecFwk::WrapInt64ToJS(env, param.execCmdOptions.yieldMs));
    napi_set_named_property(env, jsOpts, "timeout",
        AppExecFwk::WrapInt64ToJS(env, param.execCmdOptions.timeout));
    napi_set_named_property(env, jsOpts, "isShellCommand",
        AppExecFwk::WrapBoolToJS(env, param.execCmdOptions.isShellCommand));
    napi_set_named_property(env, jsOpts, "challenge",
        AppExecFwk::WrapStringToJS(env, param.execCmdOptions.challenge));
    // Trace identifiers: only surfaced when provided so the before-hook can
    // observe them (read-only for the hook; modifications do not propagate).
    if (!param.execCmdOptions.toolCallId.empty()) {
        napi_set_named_property(env, jsOpts, "toolCallId",
            AppExecFwk::WrapStringToJS(env, param.execCmdOptions.toolCallId));
    }
    if (!param.execCmdOptions.dmSessionId.empty()) {
        napi_set_named_property(env, jsOpts, "dmSessionId",
            AppExecFwk::WrapStringToJS(env, param.execCmdOptions.dmSessionId));
    }
    napi_set_named_property(env, jsObj, "execCmdOptions", jsOpts);
    return jsObj;
}

napi_value CreateJsExecResultWrap(napi_env env, const ExecResultWrap &wrap)
{
    napi_value jsParam = nullptr;
    napi_create_object(env, &jsParam);
    napi_value jsInner = CreateJsExecResult(env, wrap.execResult);
    napi_set_named_property(env, jsParam, "execResult", jsInner);
    // Trace identifiers actually used by the execution (stamped service-side);
    // omitted when not provided.
    if (!wrap.toolCallId.empty()) {
        napi_set_named_property(env, jsParam, "toolCallId", AppExecFwk::WrapStringToJS(env, wrap.toolCallId));
    }
    if (!wrap.dmSessionId.empty()) {
        napi_set_named_property(env, jsParam, "dmSessionId", AppExecFwk::WrapStringToJS(env, wrap.dmSessionId));
    }
    return jsParam;
}

napi_value CreateJsSubCommandInfo(napi_env env, const SubCommandInfo &subcmd)
{
    napi_value jsObj = nullptr;
    napi_status status = napi_create_object(env, &jsObj);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to create JS object");
        return nullptr;
    }

    // Set description
    napi_value jsDescription = AppExecFwk::WrapStringToJS(env, subcmd.description);
    napi_set_named_property(env, jsObj, "description", jsDescription);

    // Set requirePermissions (array)
    napi_value jsPermissions = nullptr;
    status = napi_create_array(env, &jsPermissions);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "napi_create_array failed, %{public}d", status);
        return nullptr;
    }
    for (size_t i = 0; i < subcmd.requirePermissions.size(); i++) {
        napi_value jsPerm = AppExecFwk::WrapStringToJS(env, subcmd.requirePermissions[i]);
        napi_set_element(env, jsPermissions, i, jsPerm);
    }
    napi_set_named_property(env, jsObj, "requirePermissions", jsPermissions);

    // Set inputSchema (parse JSON string to object)
    napi_value jsInputSchema = ParseJsonStringToJsObject(env, subcmd.inputSchema);
    napi_set_named_property(env, jsObj, "inputSchema", jsInputSchema);

    // Set outputSchema (parse JSON string to object)
    napi_value jsOutputSchema = ParseJsonStringToJsObject(env, subcmd.outputSchema);
    napi_set_named_property(env, jsObj, "outputSchema", jsOutputSchema);

    // Set eventTypes (array)
    napi_value jsEventTypes = nullptr;
    status = napi_create_array(env, &jsEventTypes);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "napi_create_array failed, %{public}d", status);
        return nullptr;
    }
    for (size_t i = 0; i < subcmd.eventTypes.size(); i++) {
        napi_value jsEventType = AppExecFwk::WrapStringToJS(env, subcmd.eventTypes[i]);
        napi_set_element(env, jsEventTypes, i, jsEventType);
    }
    napi_set_named_property(env, jsObj, "eventTypes", jsEventTypes);

    // Set eventSchemas (parse JSON string to object)
    napi_value jsEventSchemas = ParseJsonStringToJsObject(env, subcmd.eventSchemas);
    napi_set_named_property(env, jsObj, "eventSchemas", jsEventSchemas);

    return jsObj;
}

bool IsValidToolEventCallback(napi_env env, napi_value obj)
{
    if (obj == nullptr) {
        return false;
    }
    napi_valuetype valueType = napi_undefined;
    napi_status status = napi_typeof(env, obj, &valueType);
    if (status != napi_ok || valueType != napi_object) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Input is not an object");
        return false;
    }

    napi_value onEventProp = nullptr;
    if (napi_get_named_property(env, obj, "onEvent", &onEventProp) != napi_ok || !onEventProp) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "invalid onEvent property");
        return false;
    }

    napi_valuetype callbackType = napi_undefined;
    if (napi_typeof(env, onEventProp, &callbackType) != napi_ok || callbackType != napi_function) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "onEvent must be a function");
        return false;
    }

    return true;
}

napi_value CreateJsCliToolEvent(napi_env env, const CliToolEvent &event)
{
    HandleEscape handleEscape(env);
    napi_value objValue = nullptr;
    napi_status createStatus = napi_create_object(env, &objValue);
    if (createStatus != napi_ok || objValue == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to create CliToolEvent object");
        return nullptr;
    }

    napi_set_named_property(env, objValue, "toolEventType", CreateJsValue(env, event.type));
    napi_set_named_property(env, objValue, "data", CreateJsValue(env, event.eventData));

    return handleEscape.Escape(objValue);
}

napi_value CreateJsToolInfo(napi_env env, const ToolInfo &tool)
{
    napi_value jsObj = nullptr;
    napi_status status = napi_create_object(env, &jsObj);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to create JS object");
        return nullptr;
    }

    // Set name
    napi_value jsName = AppExecFwk::WrapStringToJS(env, tool.name);
    napi_set_named_property(env, jsObj, "name", jsName);

    // Set version
    napi_value jsVersion = AppExecFwk::WrapStringToJS(env, tool.version);
    napi_set_named_property(env, jsObj, "version", jsVersion);

    // Set description
    napi_value jsDescription = AppExecFwk::WrapStringToJS(env, tool.description);
    napi_set_named_property(env, jsObj, "description", jsDescription);

    // Set executablePath
    napi_value jsExecutablePath = AppExecFwk::WrapStringToJS(env, tool.executablePath);
    napi_set_named_property(env, jsObj, "executablePath", jsExecutablePath);

    // Set requirePermissions (array)
    napi_value jsPermissions = nullptr;
    status = napi_create_array(env, &jsPermissions);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "napi_create_array failed, %{public}d", status);
        return nullptr;
    }
    for (size_t i = 0; i < tool.requirePermissions.size(); i++) {
        napi_value jsPerm = AppExecFwk::WrapStringToJS(env, tool.requirePermissions[i]);
        napi_set_element(env, jsPermissions, i, jsPerm);
    }
    napi_set_named_property(env, jsObj, "requirePermissions", jsPermissions);

    // Set inputSchema (parse JSON string to object)
    napi_value jsInputSchema = ParseJsonStringToJsObject(env, tool.inputSchema);
    napi_set_named_property(env, jsObj, "inputSchema", jsInputSchema);

    // Set outputSchema (parse JSON string to object)
    napi_value jsOutputSchema = ParseJsonStringToJsObject(env, tool.outputSchema);
    napi_set_named_property(env, jsObj, "outputSchema", jsOutputSchema);

    // Set eventTypes (array)
    napi_value jsEventTypes = nullptr;
    status = napi_create_array(env, &jsEventTypes);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "napi_create_array failed, %{public}d", status);
        return nullptr;
    }
    for (size_t i = 0; i < tool.eventTypes.size(); i++) {
        napi_value jsEventType = AppExecFwk::WrapStringToJS(env, tool.eventTypes[i]);
        napi_set_element(env, jsEventTypes, i, jsEventType);
    }
    napi_set_named_property(env, jsObj, "eventTypes", jsEventTypes);

    // Set eventSchemas (parse JSON string to object)
    napi_value jsEventSchemas = ParseJsonStringToJsObject(env, tool.eventSchemas);
    napi_set_named_property(env, jsObj, "eventSchemas", jsEventSchemas);

    // Set hasSubCommand
    napi_value jsHasSubCommand = AppExecFwk::WrapBoolToJS(env, tool.hasSubCommand);
    napi_set_named_property(env, jsObj, "hasSubCommand", jsHasSubCommand);

    // Set subcommands (map)
    napi_value jsSubcommands = nullptr;
    napi_create_object(env, &jsSubcommands);
    for (const auto &pair : tool.subcommands) {
        napi_value jsSubcmd = CreateJsSubCommandInfo(env, pair.second);
        if (jsSubcmd != nullptr) {
            napi_set_named_property(env, jsSubcommands, pair.first.c_str(), jsSubcmd);
        }
    }
    napi_set_named_property(env, jsObj, "subcommands", jsSubcommands);

    // Set isLockScreenExecutionAllowed
    napi_value jsIsLockScreenExecutionAllowed = AppExecFwk::WrapBoolToJS(env, tool.isLockScreenExecutionAllowed);
    napi_set_named_property(env, jsObj, "isLockScreenExecutionAllowed", jsIsLockScreenExecutionAllowed);

    return jsObj;
}

napi_value CreateJsToolSummary(napi_env env, const ToolSummary &summary)
{
    napi_value jsObj = nullptr;
    napi_status status = napi_create_object(env, &jsObj);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to create JS object");
        return nullptr;
    }

    // Set name
    napi_value jsName = AppExecFwk::WrapStringToJS(env, summary.name);
    napi_set_named_property(env, jsObj, "name", jsName);

    // Set version
    napi_value jsVersion = AppExecFwk::WrapStringToJS(env, summary.version);
    napi_set_named_property(env, jsObj, "version", jsVersion);

    // Set description
    napi_value jsDescription = AppExecFwk::WrapStringToJS(env, summary.description);
    napi_set_named_property(env, jsObj, "description", jsDescription);

    return jsObj;
}

napi_value CreateJsSessionStatus(napi_env env)
{
    napi_value objValue = nullptr;
    napi_status createStatus = napi_create_object(env, &objValue);
    if (createStatus != napi_ok || objValue == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "napi_create_object failed, %{public}d", createStatus);
        return nullptr;
    }

    napi_value runningVal = CreateJsValue(env, std::string("running"));
    napi_value completedVal = CreateJsValue(env, std::string("completed"));
    napi_value failedVal = CreateJsValue(env, std::string("failed"));
    if (runningVal == nullptr || completedVal == nullptr || failedVal == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "CreateJsValue failed");
        return nullptr;
    }
    napi_status setStatus = napi_set_named_property(env, objValue, "RUNNING", runningVal);
    if (setStatus != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "napi_set_named_property RUNNING failed");
        return nullptr;
    }
    setStatus = napi_set_named_property(env, objValue, "COMPLETED", completedVal);
    if (setStatus != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "napi_set_named_property COMPLETED failed");
        return nullptr;
    }
    setStatus = napi_set_named_property(env, objValue, "FAILED", failedVal);
    if (setStatus != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "napi_set_named_property FAILED failed");
        return nullptr;
    }

    return objValue;
}

} // namespace CliTool
} // namespace OHOS
