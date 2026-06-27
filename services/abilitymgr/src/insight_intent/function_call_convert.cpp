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
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
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
    optionsParam["properties"]["displayId"] = {{"type", "integer"}};
    optionsParam["properties"]["userId"] = {{"type", "integer"}};
    optionsParam["properties"]["deviceId"] = {{"type", "string"}, {"default", ""}};
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

struct FilterCandidate {
    std::string intentName;
    std::string moduleName;
    std::string abilityName;
    bool isUIAbility = false;
};

struct CandidateOrdering {
    bool operator()(const FilterCandidate &a, const FilterCandidate &b) const
    {
        if (a.moduleName != b.moduleName) {
            return a.moduleName < b.moduleName;
        }
        if (a.isUIAbility != b.isUIAbility) {
            return a.isUIAbility;
        }
        return a.abilityName < b.abilityName;
    }
};

bool HasExecuteMode(const std::vector<std::string> &modes, const std::string &target)
{
    return std::find(modes.begin(), modes.end(), target) != modes.end();
}

std::optional<FilterCandidate> ExtractFromProfileInfo(
    const AbilityRuntime::ExtractInsightIntentProfileInfo &info)
{
    if (info.intentName.empty() || info.moduleName.empty()) {
        return std::nullopt;
    }
    FilterCandidate c;
    c.intentName = info.intentName;
    c.moduleName = info.moduleName;
    bool isBgUiAbility = !info.uiAbility.empty() && HasExecuteMode(info.executeMode, "background");
    bool isServiceExt = !info.abilityName.empty() && HasExecuteMode(info.executeMode, "serviceextension");
    if (isBgUiAbility) {
        c.abilityName = info.uiAbility;
        c.isUIAbility = true;
    } else if (isServiceExt) {
        c.abilityName = info.abilityName;
        c.isUIAbility = false;
    } else {
        return std::nullopt;
    }
    return c;
}

std::optional<FilterCandidate> ExtractFromConfigInfo(
    const AbilityRuntime::InsightIntentInfo &info)
{
    if (info.intentName.empty() || info.moduleName.empty()) {
        return std::nullopt;
    }
    FilterCandidate c;
    c.intentName = info.intentName;
    c.moduleName = info.moduleName;
    const auto &ui = info.uiAbilityIntentInfo;
    const auto &se = info.serviceExtensionIntentInfo;
    bool isBgUiAbility = !ui.abilityName.empty() &&
        std::find(ui.supportExecuteMode.begin(), ui.supportExecuteMode.end(),
            AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND) != ui.supportExecuteMode.end();
    if (isBgUiAbility) {
        c.abilityName = ui.abilityName;
        c.isUIAbility = true;
    } else if (!se.abilityName.empty()) {
        c.abilityName = se.abilityName;
        c.isUIAbility = false;
    } else {
        return std::nullopt;
    }
    return c;
}

std::optional<FilterCandidate> ExtractFromGenericInfo(
    const AbilityRuntime::ExtractInsightIntentInfo &info)
{
    const auto &g = info.genericInfo;
    if (g.intentName.empty() || g.moduleName.empty()) {
        return std::nullopt;
    }
    FilterCandidate c;
    c.intentName = g.intentName;
    c.moduleName = g.moduleName;
    if (g.currentType == AbilityRuntime::InfoType::Function) {
        c.isUIAbility = false;
        c.abilityName.clear();
        return c;
    }
    if (g.currentType == AbilityRuntime::InfoType::Entry) {
        const auto &entry = g.get<AbilityRuntime::InsightIntentEntryInfo>();
        for (auto mode : entry.executeMode) {
            if (mode == AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND) {
                c.isUIAbility = true;
                c.abilityName = entry.abilityName;
                return c;
            }
            if (mode == AppExecFwk::ExecuteMode::SERVICE_EXTENSION_ABILITY) {
                c.isUIAbility = false;
                c.abilityName = entry.abilityName;
                return c;
            }
        }
    }
    return std::nullopt;
}

// 按候选分 intentName 分组，每组按 CandidateOrdering 选出胜出者的原容器索引。
std::map<std::string, size_t> GroupAndPickWinners(
    const std::vector<std::optional<FilterCandidate>> &cands)
{
    std::unordered_map<std::string, std::vector<size_t>> groups;
    for (size_t i = 0; i < cands.size(); ++i) {
        if (cands[i].has_value()) {
            groups[cands[i]->intentName].push_back(i);
        }
    }
    std::map<std::string, size_t> winners;
    for (const auto &entry : groups) {
        const auto &idxs = entry.second;
        size_t winner = idxs[0];
        for (size_t i = 1; i < idxs.size(); ++i) {
            if (CandidateOrdering{}(*cands[idxs[i]], *cands[winner])) {
                winner = idxs[i];
            }
        }
        winners[entry.first] = winner;
    }
    return winners;
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

void IntentFilterUtil::FilterProfile(AbilityRuntime::ExtractInsightIntentProfileInfoVec &profileInfos)
{
    auto &items = profileInfos.insightIntents;
    std::vector<std::optional<FilterCandidate>> cands;
    cands.reserve(items.size());
    for (const auto &item : items) {
        cands.push_back(ExtractFromProfileInfo(item));
    }
    auto winners = GroupAndPickWinners(cands);
    std::set<size_t> keepIdx;
    for (const auto &w : winners) {
        keepIdx.insert(w.second);
    }
    std::vector<AbilityRuntime::ExtractInsightIntentProfileInfo> kept;
    kept.reserve(keepIdx.size());
    for (size_t i = 0; i < items.size(); ++i) {
        if (keepIdx.count(i) > 0) {
            kept.emplace_back(std::move(items[i]));
        }
    }
    items = std::move(kept);
}

void IntentFilterUtil::FilterConfig(std::vector<AbilityRuntime::InsightIntentInfo> &configInfos)
{
    std::vector<std::optional<FilterCandidate>> cands;
    cands.reserve(configInfos.size());
    for (const auto &item : configInfos) {
        cands.push_back(ExtractFromConfigInfo(item));
    }
    auto winners = GroupAndPickWinners(cands);
    std::set<size_t> keepIdx;
    for (const auto &w : winners) {
        keepIdx.insert(w.second);
    }
    std::vector<AbilityRuntime::InsightIntentInfo> kept;
    kept.reserve(keepIdx.size());
    for (size_t i = 0; i < configInfos.size(); ++i) {
        if (keepIdx.count(i) > 0) {
            kept.emplace_back(std::move(configInfos[i]));
        }
    }
    configInfos = std::move(kept);
}

void IntentFilterUtil::FilterAndDedup(std::vector<AbilityRuntime::ExtractInsightIntentInfo> &intentInfos)
{
    std::vector<std::optional<FilterCandidate>> cands;
    cands.reserve(intentInfos.size());
    for (const auto &item : intentInfos) {
        cands.push_back(ExtractFromGenericInfo(item));
    }
    auto winners = GroupAndPickWinners(cands);
    std::set<size_t> keepIdx;
    for (const auto &w : winners) {
        keepIdx.insert(w.second);
    }
    std::vector<AbilityRuntime::ExtractInsightIntentInfo> kept;
    kept.reserve(keepIdx.size());
    for (size_t i = 0; i < intentInfos.size(); ++i) {
        if (keepIdx.count(i) > 0) {
            kept.emplace_back(std::move(intentInfos[i]));
        }
    }
    intentInfos = std::move(kept);
}

} // namespace CliTool
} // namespace OHOS
