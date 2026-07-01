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

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "cli_tool_mgr_client.h"
#include "hilog_tag_wrapper.h"
#include "insight_intent_execute_param.h"

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
    optionsParam["properties"]["uris"]["type"] = "array";
    optionsParam["properties"]["uris"]["items"] = {{"type", "string"}};
    optionsParam["properties"]["flags"] = {{"type", "integer"}};
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

struct RegisterSortKey {
    std::string moduleName;
    std::string abilityName;
    bool operator<(const RegisterSortKey &o) const
    {
        if (moduleName != o.moduleName) {
            return moduleName < o.moduleName;
        }
        return abilityName < o.abilityName;
    }
};

std::string GetFunctionNameFromGeneric(const AbilityRuntime::ExtractInsightIntentInfo &info)
{
    if (info.genericInfo.currentType == AbilityRuntime::InfoType::Function) {
        return info.genericInfo.get<AbilityRuntime::InsightIntentFunctionInfo>().functionName;
    }
    return "";
}

// 检查 Entry 装饰器的 executeMode 是否含 BG UIAbility 或 SE。通过则填 outAbility 并返回 true。
bool IsQualifiedEntry(const AbilityRuntime::ExtractInsightIntentGenericInfo &generic, std::string &outAbility)
{
    if (generic.currentType != AbilityRuntime::InfoType::Entry) {
        return false;
    }
    const auto &entry = generic.get<AbilityRuntime::InsightIntentEntryInfo>();
    for (auto mode : entry.executeMode) {
        if (mode == AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND ||
            mode == AppExecFwk::ExecuteMode::SERVICE_EXTENSION_ABILITY) {
            outAbility = entry.abilityName;
            return true;
        }
    }
    return false;
}

std::string GetInputSchemaFromGeneric(const AbilityRuntime::ExtractInsightIntentInfo &info)
{
    switch (info.genericInfo.currentType) {
        case AbilityRuntime::InfoType::Link:
            return info.genericInfo.get<AbilityRuntime::InsightIntentLinkInfo>().parameters;
        case AbilityRuntime::InfoType::Page:
            return info.genericInfo.get<AbilityRuntime::InsightIntentPageInfo>().parameters;
        case AbilityRuntime::InfoType::Entry:
            return info.genericInfo.get<AbilityRuntime::InsightIntentEntryInfo>().parameters;
        case AbilityRuntime::InfoType::Function:
            return info.genericInfo.get<AbilityRuntime::InsightIntentFunctionInfo>().parameters;
        case AbilityRuntime::InfoType::Form:
            return info.genericInfo.get<AbilityRuntime::InsightIntentFormInfo>().parameters;
        default:
            return "";
    }
}

IntentOptionDefaults MakeDefaultsFromGeneric(const AbilityRuntime::ExtractInsightIntentInfo &info)
{
    IntentOptionDefaults defaults;
    defaults.moduleName = info.genericInfo.moduleName;
    if (info.genericInfo.currentType == AbilityRuntime::InfoType::Entry) {
        defaults.abilityName = info.genericInfo.get<AbilityRuntime::InsightIntentEntryInfo>().abilityName;
    } else if (info.genericInfo.currentType == AbilityRuntime::InfoType::Page) {
        defaults.abilityName = info.genericInfo.get<AbilityRuntime::InsightIntentPageInfo>().uiAbility;
    } else if (info.genericInfo.currentType == AbilityRuntime::InfoType::Form) {
        defaults.abilityName = info.genericInfo.get<AbilityRuntime::InsightIntentFormInfo>().abilityName;
    }
    return defaults;
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
}
} // namespace

bool ConvertFromExtractIntentInfo(const std::vector<AbilityRuntime::ExtractInsightIntentInfo> &intentInfos,
    std::vector<FunctionInfo> &functions)
{
    for (const auto &info : intentInfos) {
        if (info.genericInfo.intentName.empty()) {
            TAG_LOGW(AAFwkTag::CLI_TOOL, "empty intentName");
            continue;
        }
        FunctionInfo func;
        std::string functionName = GetFunctionNameFromGeneric(info);
        func.functionName = functionName.empty() ? info.genericInfo.intentName : functionName;
        func.functionNamespace = info.genericInfo.bundleName;
        func.description = info.displayDescription;
        func.inputSchema = GetInputSchemaFromGeneric(info);
        func.outputSchema = info.result;
        func.functionType = FunctionType::INTENT_FUNCTION;
        AddInsightIntentOptions(func, MakeDefaultsFromGeneric(info));
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
    const std::vector<AbilityRuntime::ExtractInsightIntentInfo> &intentInfos,
    const std::vector<AbilityRuntime::InsightIntentInfo> &configInfos,
    const std::string &bundleName,
    uint32_t versionCode)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "register intents, bundle:%{public}s intent:%{public}zu config:%{public}zu",
        bundleName.c_str(), intentInfos.size(), configInfos.size());
    std::vector<FunctionInfo> functions;
    ConvertFromConfigIntent(configInfos, functions);
    ConvertFromExtractIntentInfo(intentInfos, functions);
    if (functions.empty()) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "no functions to register after convert, bundle:%{public}s",
            bundleName.c_str());
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

void IntentFilterUtil::FilterConfig(std::vector<AbilityRuntime::InsightIntentInfo> &configInfos)
{
    size_t inputCount = configInfos.size();
    std::vector<std::pair<RegisterSortKey, AbilityRuntime::InsightIntentInfo>> qualified;
    for (auto &info : configInfos) {
        if (info.intentName.empty() || info.moduleName.empty()) {
            TAG_LOGW(AAFwkTag::CLI_TOOL, "config intent dropped: empty intentName or moduleName, "
                "intentName:%{public}s moduleName:%{public}s",
                info.intentName.c_str(), info.moduleName.c_str());
            continue;
        }
        const auto &ui = info.uiAbilityIntentInfo;
        const auto &se = info.serviceExtensionIntentInfo;
        RegisterSortKey key { info.moduleName, "" };
        bool isBgUiAbility = !ui.abilityName.empty() &&
            std::find(ui.supportExecuteMode.begin(), ui.supportExecuteMode.end(),
                AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND) != ui.supportExecuteMode.end();
        if (isBgUiAbility) {
            key.abilityName = ui.abilityName;
        } else if (!se.abilityName.empty()) {
            key.abilityName = se.abilityName;
        } else {
            TAG_LOGW(AAFwkTag::CLI_TOOL, "config intent dropped: not BG UIAbility or SE, "
                "intentName:%{public}s moduleName:%{public}s uiAbility:%{public}s seAbility:%{public}s",
                info.intentName.c_str(), info.moduleName.c_str(),
                ui.abilityName.c_str(), se.abilityName.c_str());
            continue;
        }
        qualified.emplace_back(std::move(key), std::move(info));
    }
    std::sort(qualified.begin(), qualified.end(),
        [](const auto &a, const auto &b) { return a.first < b.first; });
    configInfos.clear();
    for (auto &item : qualified) {
        configInfos.push_back(std::move(item.second));
    }
    TAG_LOGI(AAFwkTag::CLI_TOOL, "FilterConfig: input=%{public}zu output=%{public}zu",
        inputCount, configInfos.size());
}

void IntentFilterUtil::FilterGeneric(std::vector<AbilityRuntime::ExtractInsightIntentInfo> &intentInfos)
{
    size_t inputCount = intentInfos.size();
    std::vector<std::pair<RegisterSortKey, AbilityRuntime::ExtractInsightIntentInfo>> qualified;
    for (auto &info : intentInfos) {
        const auto &generic = info.genericInfo;
        if (generic.intentName.empty() || generic.moduleName.empty()) {
            TAG_LOGW(AAFwkTag::CLI_TOOL, "generic intent dropped: empty intentName or moduleName, "
                "intentName:%{public}s moduleName:%{public}s",
                generic.intentName.c_str(), generic.moduleName.c_str());
            continue;
        }
        RegisterSortKey key { generic.moduleName, "" };
        if (generic.currentType != AbilityRuntime::InfoType::Function &&
            generic.currentType != AbilityRuntime::InfoType::Page &&
            generic.currentType != AbilityRuntime::InfoType::Link &&
            !IsQualifiedEntry(generic, key.abilityName)) {
            TAG_LOGW(AAFwkTag::CLI_TOOL, "generic intent dropped: not Function/Page/Link or qualified Entry, "
                "intentName:%{public}s moduleName:%{public}s decoratorType:%{public}s",
                generic.intentName.c_str(), generic.moduleName.c_str(), generic.decoratorType.c_str());
            continue;
        }
        qualified.emplace_back(std::move(key), std::move(info));
    }
    std::sort(qualified.begin(), qualified.end(),
        [](const auto &a, const auto &b) { return a.first < b.first; });
    intentInfos.clear();
    for (auto &item : qualified) {
        intentInfos.push_back(std::move(item.second));
    }
    TAG_LOGI(AAFwkTag::CLI_TOOL, "FilterGeneric: input=%{public}zu output=%{public}zu",
        inputCount, intentInfos.size());
}

bool BatchRegisterInsightIntentFunctions(
    const std::vector<AbilityRuntime::ExtractInsightIntentInfo> &intentInfos,
    const std::vector<AbilityRuntime::InsightIntentInfo> &configInfos,
    const std::unordered_map<std::string, uint32_t> &bundleVersionMap,
    int32_t &successCount)
{
    successCount = 0;
    std::vector<FunctionInfo> functions;
    ConvertFromConfigIntent(configInfos, functions);
    ConvertFromExtractIntentInfo(intentInfos, functions);
    if (functions.empty()) {
        return true;
    }
    for (auto &func : functions) {
        if (func.functionNamespace.empty() && bundleVersionMap.size() == 1) {
            func.functionNamespace = bundleVersionMap.begin()->first;
        }
        auto it = bundleVersionMap.find(func.functionNamespace);
        if (it != bundleVersionMap.end()) {
            func.version = std::to_string(it->second);
        }
    }
    auto &client = CliToolMGRClient::GetInstance();
    ErrCode ret = client.BatchRegisterFunctions(functions, successCount);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "batch register failed, ret=%{public}d, success=%{public}d",
            ret, successCount);
    }
    return ret == ERR_OK;
}

} // namespace CliTool
} // namespace OHOS
