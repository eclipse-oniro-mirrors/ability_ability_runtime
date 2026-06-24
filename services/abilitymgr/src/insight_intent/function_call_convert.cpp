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

#include "function_call_convert.h"

#include "cli_tool_mgr_client.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {

namespace {
struct IntentOptionDefaults {
    std::string moduleName;
    std::string abilityName;
};

void BuildOptionsSchema(nlohmann::json &schema, const IntentOptionDefaults &defaults)
{
    nlohmann::json optionsParam;
    optionsParam["type"] = "object";
    optionsParam["properties"]["executeMode"] = {{"type", "string"}, {"default", "background"}};
    optionsParam["properties"]["moduleName"] = {{"type", "string"}, {"default", defaults.moduleName}};
    optionsParam["properties"]["abilityName"] = {{"type", "string"}, {"default", defaults.abilityName}};
    optionsParam["properties"]["displayId"] = {{"type", "integer"}, {"default", 0}};
    optionsParam["properties"]["userId"] = {{"type", "integer"}, {"default", 100}};
    optionsParam["properties"]["deviceId"] = {{"type", "string"}, {"default", "local"}};
    optionsParam["properties"]["uris"]["type"] = "array";
    optionsParam["properties"]["uris"]["items"] = {{"type", "string"}};
    optionsParam["properties"]["flags"] = {{"type", "string"}};
    schema["properties"]["ohos.insightIntent.options"] = optionsParam;
}

void AddInsightIntentOptions(FunctionInfo &func, const IntentOptionDefaults &defaults = {})
{
    nlohmann::json schema;
    if (!func.inputSchema.empty()) {
        schema = nlohmann::json::parse(func.inputSchema, nullptr, false);
        if (schema.is_discarded()) {
            schema = nlohmann::json();
        }
    }
    if (!schema.is_object()) {
        schema = nlohmann::json();
    }
    if (!schema.contains("type") || schema["type"] != "object") {
        schema["type"] = "object";
    }
    if (!schema.contains("properties")) {
        schema["properties"] = nlohmann::json();
    }
    BuildOptionsSchema(schema, defaults);
    func.inputSchema = schema.dump();
}
} // namespace

namespace {
void VerifyRegisterFunction(CliToolMGRClient &client, const FunctionInfo &func)
{
    FunctionInfo queryResult;
    auto queryRet = client.GetFunctionInfo(func.functionNamespace, func.functionName, queryResult);
    if (queryRet == ERR_OK) {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "verify register success: %{public}s/%{public}s",
            func.functionNamespace.c_str(), func.functionName.c_str());
    } else {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "verify register failed: %{public}s/%{public}s, ret: %{public}d",
            func.functionNamespace.c_str(), func.functionName.c_str(), queryRet);
    }
}

void RegisterOrUpdateFunction(CliToolMGRClient &client, const FunctionInfo &func)
{
    auto ret = client.RegisterFunction(func);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "register function failed: %{public}s/%{public}s, ret: %{public}d",
            func.functionNamespace.c_str(), func.functionName.c_str(), ret);
        return;
    }
    TAG_LOGI(AAFwkTag::CLI_TOOL, "registered function: %{public}s/%{public}s",
        func.functionNamespace.c_str(), func.functionName.c_str());
    VerifyRegisterFunction(client, func);
}
} // namespace

bool ConvertFromExtractProfile(const AbilityRuntime::ExtractInsightIntentProfileInfoVec &profileInfos,
    std::vector<FunctionInfo> &functions)
{
    for (const auto &info : profileInfos.insightIntents) {
        if (info.intentName.empty()) {
            TAG_LOGW(AAFwkTag::CLI_TOOL, "empty intentName");
            continue;
        }
        FunctionInfo func;
        func.functionName = info.functionName.empty() ? info.intentName : info.functionName;
        func.functionNamespace = info.bundleName;
        func.description = info.displayDescription;
        func.inputSchema = info.parameters;
        func.outputSchema = info.result;
        func.functionType = FunctionType::INTENT_FUNCTION;
        IntentOptionDefaults defaults;
        defaults.moduleName = info.moduleName;
        defaults.abilityName = info.uiAbility.empty() ? info.abilityName : info.uiAbility;
        AddInsightIntentOptions(func, defaults);
        functions.emplace_back(std::move(func));
    }
    return true;
}

bool ConvertFromExtractIntentInfo(const std::vector<AbilityRuntime::ExtractInsightIntentInfo> &intentInfos,
    std::vector<FunctionInfo> &functions)
{
    for (const auto &info : intentInfos) {
        if (info.genericInfo.intentName.empty()) {
            TAG_LOGW(AAFwkTag::CLI_TOOL, "empty intentName");
            continue;
        }
        FunctionInfo func;
        std::string functionName;
        if (info.genericInfo.currentType == AbilityRuntime::InfoType::Function) {
            auto &funcInfo = info.genericInfo.get<AbilityRuntime::InsightIntentFunctionInfo>();
            functionName = funcInfo.functionName;
            func.inputSchema = funcInfo.parameters;
        }
        func.functionName = functionName.empty() ? info.genericInfo.intentName : functionName;
        func.functionNamespace = info.genericInfo.bundleName;
        func.description = info.displayDescription;
        func.outputSchema = info.result;
        func.functionType = FunctionType::INTENT_FUNCTION;
        functions.emplace_back(std::move(func));
    }
    return true;
}

bool ConvertFromConfigIntent(const std::vector<AbilityRuntime::InsightIntentInfo> &configInfos,
    std::vector<FunctionInfo> &functions)
{
    for (const auto &info : configInfos) {
        if (info.intentName.empty()) {
            TAG_LOGW(AAFwkTag::CLI_TOOL, "empty intentName");
            continue;
        }

        FunctionInfo func;
        func.functionName = info.intentName;
        func.functionNamespace = info.bundleName;
        func.description = info.displayDescription;
        func.functionType = FunctionType::INTENT_FUNCTION;

        if (!info.inputParams.empty()) {
            nlohmann::json inputSchema;
            inputSchema["type"] = "object";
            nlohmann::json properties;
            for (const auto &param : info.inputParams) {
                properties[param] = {{"type", "string"}};
            }
            inputSchema["properties"] = properties;
            func.inputSchema = inputSchema.dump();
        }

        if (!info.outputParams.empty()) {
            nlohmann::json outputSchema;
            outputSchema["type"] = "object";
            nlohmann::json properties;
            for (const auto &param : info.outputParams) {
                properties[param] = {{"type", "string"}};
            }
            outputSchema["properties"] = properties;
            func.outputSchema = outputSchema.dump();
        }

        IntentOptionDefaults defaults;
        defaults.moduleName = info.moduleName;
        defaults.abilityName = info.uiAbilityIntentInfo.abilityName;
        AddInsightIntentOptions(func, defaults);

        functions.emplace_back(std::move(func));
    }
    return true;
}

bool RegisterInsightIntentFunctions(
    const AbilityRuntime::ExtractInsightIntentProfileInfoVec &profileInfos,
    const std::vector<AbilityRuntime::InsightIntentInfo> &configInfos,
    const std::string &bundleName,
    uint32_t versionCode)
{
    std::vector<FunctionInfo> functions;
    ConvertFromExtractProfile(profileInfos, functions);
    ConvertFromConfigIntent(configInfos, functions);
    if (functions.empty()) {
        return true;
    }

    for (auto &func : functions) {
        if (func.functionNamespace.empty() && !bundleName.empty()) {
            func.functionNamespace = bundleName;
        }
        func.version = std::to_string(versionCode);
    }

    auto &client = CliToolMGRClient::GetInstance();
    for (const auto &func : functions) {
        RegisterOrUpdateFunction(client, func);
    }
    return true;
}

bool RegisterInsightIntentFunctions(
    const std::vector<AbilityRuntime::ExtractInsightIntentInfo> &intentInfos,
    const std::vector<AbilityRuntime::InsightIntentInfo> &configInfos,
    const std::string &bundleName,
    uint32_t versionCode)
{
    std::vector<FunctionInfo> functions;
    ConvertFromExtractIntentInfo(intentInfos, functions);
    ConvertFromConfigIntent(configInfos, functions);
    if (functions.empty()) {
        return true;
    }

    for (auto &func : functions) {
        if (func.functionNamespace.empty() && !bundleName.empty()) {
            func.functionNamespace = bundleName;
        }
        func.version = std::to_string(versionCode);
    }

    auto &client = CliToolMGRClient::GetInstance();
    for (const auto &func : functions) {
        RegisterOrUpdateFunction(client, func);
    }
    return true;
}

bool UnregisterInsightIntentFunctions(const std::string &bundleName)
{
    if (bundleName.empty()) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "empty bundleName");
        return false;
    }
    auto &client = CliToolMGRClient::GetInstance();
    auto ret = client.UnregisterIntentFunctionsByNamespace(bundleName);
    if (ret < 0) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "unregister functions failed: %{public}s, ret: %{public}d",
            bundleName.c_str(), ret);
        return false;
    }
    TAG_LOGI(AAFwkTag::CLI_TOOL, "unregistered functions for bundle: %{public}s, count: %{public}d",
        bundleName.c_str(), ret);
    return true;
}

} // namespace CliTool
} // namespace OHOS
