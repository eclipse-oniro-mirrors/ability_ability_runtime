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
#include "hilog_tag_wrapper.h"
#include "icli_tool_data.h"
#include "js_cli_session_event_callback.h"
#include "napi_common_util.h"

using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace CliTool {
namespace {
constexpr int64_t MAX_TIMEOUT = 30 * 60; // 30 m
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
            napi_value jsItem = nullptr;
            if (it.value().is_string()) {
                jsItem = AppExecFwk::WrapStringToJS(env, it.value().get<std::string>());
            } else if (it.value().is_number_integer()) {
                jsItem = AppExecFwk::WrapInt32ToJS(env, it.value().get<int32_t>());
            } else if (it.value().is_boolean()) {
                jsItem = AppExecFwk::WrapBoolToJS(env, it.value().get<bool>());
            } else if (it.value().is_object() || it.value().is_array()) {
                jsItem = ParseJsonStringToJsObject(env, it.value().dump());
            } else {
                napi_get_undefined(env, &jsItem);
            }
            napi_set_named_property(env, jsValue, it.key().c_str(), jsItem);
        }
    } else if (jsonObj.is_array()) {
        napi_create_array(env, &jsValue);
        size_t index = 0;
        for (auto &item : jsonObj) {
            napi_value jsItem = nullptr;
            if (item.is_string()) {
                jsItem = AppExecFwk::WrapStringToJS(env, item.get<std::string>());
            } else if (item.is_number_integer()) {
                jsItem = AppExecFwk::WrapInt32ToJS(env, item.get<int32_t>());
            } else if (item.is_boolean()) {
                jsItem = AppExecFwk::WrapBoolToJS(env, item.get<bool>());
            } else if (item.is_object() || item.is_array()) {
                jsItem = ParseJsonStringToJsObject(env, item.dump());
            } else {
                napi_get_undefined(env, &jsItem);
            }
            napi_set_element(env, jsValue, index++, jsItem);
        }
    } else {
        napi_create_object(env, &jsValue);
    }

    return jsValue;
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

bool UnwrapExecOptions(napi_env env, napi_value obj, ExecOptions &options)
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
    return true;
}

bool UnwrapExecCmdOptions(napi_env env, napi_value obj, ExecCmdParam &param,
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

    if (napi_has_named_property(env, obj, "workDir", &hasProperty) != napi_ok) {
        msg = "has workDir failed";
        TAG_LOGE(AAFwkTag::CLI_TOOL, "%{public}s", msg.c_str());
        return false;
    }
    if (hasProperty) {
        napi_value workDirProp = nullptr;
        if (napi_get_named_property(env, obj, "workDir", &workDirProp) != napi_ok) {
            msg = "invalid workDir property";
            TAG_LOGE(AAFwkTag::CLI_TOOL, "%{public}s", msg.c_str());
            return false;
        }
        if (!AppExecFwk::UnwrapStringFromJS2(env, workDirProp, param.workDir)) {
            msg = "unwrap workDir failed";
            TAG_LOGE(AAFwkTag::CLI_TOOL, "%{public}s", msg.c_str());
            return false;
        }
    }

    if (napi_has_named_property(env, obj, "env", &hasProperty) != napi_ok) {
        msg = "has env failed";
        TAG_LOGE(AAFwkTag::CLI_TOOL, "%{public}s", msg.c_str());
        return false;
    }
    if (hasProperty) {
        napi_value envProp = nullptr;
        if (napi_get_named_property(env, obj, "env", &envProp) != napi_ok) {
            msg = "invalid env property";
            TAG_LOGE(AAFwkTag::CLI_TOOL, "%{public}s", msg.c_str());
            return false;
        }
        if (!UnwrapStringFromRecord(env, envProp, param.env)) {
            msg = "unwrap env failed";
            TAG_LOGE(AAFwkTag::CLI_TOOL, "%{public}s", msg.c_str());
            return false;
        }
    }

    if (napi_has_named_property(env, obj, "policy", &hasProperty) != napi_ok) {
        msg = "has policy failed";
        TAG_LOGE(AAFwkTag::CLI_TOOL, "%{public}s", msg.c_str());
        return false;
    }
    if (hasProperty) {
        napi_value policyProp = nullptr;
        if (napi_get_named_property(env, obj, "policy", &policyProp) != napi_ok) {
            msg = "invalid policy property";
            TAG_LOGE(AAFwkTag::CLI_TOOL, "%{public}s", msg.c_str());
            return false;
        }
        if (!AppExecFwk::UnwrapStringFromJS2(env, policyProp, param.policy)) {
            msg = "unwrap policy failed";
            TAG_LOGE(AAFwkTag::CLI_TOOL, "%{public}s", msg.c_str());
            return false;
        }
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

    // Reuse UnwrapExecOptions for background, yieldMs, timeout
    if (!UnwrapExecOptions(env, obj, param.options)) {
        return false;
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

    // Set result if present
    if (session.status != "running" && session.result != nullptr) {
        napi_value jsResult = nullptr;
        status = napi_create_object(env, &jsResult);
        if (status != napi_ok) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to create JS ExecResult");
            return nullptr;
        }
        if (!session.result->timeout) {
            napi_value jsExitCode = AppExecFwk::WrapInt32ToJS(env, session.result->exitCode);
            napi_set_named_property(env, jsResult, "exitCode", jsExitCode);
        }
        if (!session.result->outputText.empty()) {
            napi_value jsOutputText = AppExecFwk::WrapStringToJS(env, session.result->outputText);
            napi_set_named_property(env, jsResult, "outputText", jsOutputText);
        }
        if (!session.result->errorText.empty()) {
            napi_value jsErrorText = AppExecFwk::WrapStringToJS(env, session.result->errorText);
            napi_set_named_property(env, jsResult, "errorText", jsErrorText);
        }
        if (session.result->signalNumber != 0) {
            napi_value jsSignalNumber = AppExecFwk::WrapInt32ToJS(env, session.result->signalNumber);
            napi_set_named_property(env, jsResult, "signalNumber", jsSignalNumber);
        }
        // Set timeOut
        napi_set_named_property(env, jsResult, "timeOut", AppExecFwk::WrapBoolToJS(env, session.result->timeout));
        // Set executionTime
        napi_value jsExecutionTime = AppExecFwk::WrapInt64ToJS(env, session.result->executionTime);
        napi_set_named_property(env, jsResult, "executionTime", jsExecutionTime);
        napi_set_named_property(env, jsObj, "result", jsResult);
    }

    return handleEscape.Escape(jsObj);
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
    napi_create_array(env, &jsPermissions);
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
    napi_create_array(env, &jsEventTypes);
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
    napi_create_array(env, &jsPermissions);
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
    napi_create_array(env, &jsEventTypes);
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

} // namespace CliTool
} // namespace OHOS
