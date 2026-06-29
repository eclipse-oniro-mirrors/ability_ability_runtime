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

#include "insight_intent_param_parser.h"

#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <cstdint>
#include <unordered_map>

#include "ability_manager_errors.h"
#include "array_wrapper.h"
#include "function_call_convert.h"
#include "hilog_tag_wrapper.h"
#include "string_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char *INSIGHT_INTENT_OPTIONS_KEY = "ohos.insightIntent.options";
constexpr const char *INSIGHT_INTENT_OPT_MODULE_NAME = "moduleName";
constexpr const char *INSIGHT_INTENT_OPT_EXECUTE_MODE = "executeMode";
constexpr const char *INSIGHT_INTENT_OPT_ABILITY_NAME = "abilityName";
constexpr const char *INSIGHT_INTENT_OPT_URIS = "uris";
constexpr const char *INSIGHT_INTENT_OPT_FLAGS = "flags";

constexpr int DECIMAL_BASE = 10;
constexpr int AUTO_BASE = 0;
constexpr int32_t DEFAULT_USER_ID = -1;

bool ParseInt(const std::string &str, int base, int32_t &out)
{
    if (str.empty()) {
        return false;
    }
    errno = 0;
    char *end = nullptr;
    long long val = std::strtoll(str.c_str(), &end, base);
    if (end == str.c_str() || *end != '\0' || errno == ERANGE) {
        return false;
    }
    if (val < INT32_MIN || val > INT32_MAX) {
        return false;
    }
    out = static_cast<int32_t>(val);
    return true;
}

// 从代表候选的 variant 提取 abilityName（Entry/Page/Form 三种装饰器有 abilityName 字段）。
std::string GetAbilityNameFromMatched(const ExtractInsightIntentGenericInfo &info)
{
    if (info.currentType == InfoType::Entry) {
        return info.get<InsightIntentEntryInfo>().abilityName;
    }
    if (info.currentType == InfoType::Page) {
        return info.get<InsightIntentPageInfo>().uiAbility;
    }
    if (info.currentType == InfoType::Form) {
        return info.get<InsightIntentFormInfo>().abilityName;
    }
    return "";
}

// 从代表候选的 variant 提取 executeMode（Entry 取 executeMode 首个；Function 强制 SE）。
int32_t GetExecuteModeFromMatched(const ExtractInsightIntentGenericInfo &info)
{
    if (info.currentType == InfoType::Entry) {
        const auto &entry = info.get<InsightIntentEntryInfo>();
        if (!entry.executeMode.empty()) {
            return static_cast<int32_t>(entry.executeMode.front());
        }
    }
    if (info.currentType == InfoType::Function) {
        return static_cast<int32_t>(AppExecFwk::ExecuteMode::SERVICE_EXTENSION_ABILITY);
    }
    return static_cast<int32_t>(AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND);
}

// options.executeMode 字符串映射 + 数字解析；空字符串返回 false 表示未覆写。
bool ResolveExecuteModeFromOption(const std::string &str, int32_t &out)
{
    if (str.empty()) {
        return false;
    }
    static const std::unordered_map<std::string, int32_t> MODE_MAP = {
        {"UI_ABILITY_FOREGROUND", static_cast<int32_t>(AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND)},
        {"UI_ABILITY_BACKGROUND", static_cast<int32_t>(AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND)},
        {"UI_EXTENSION_ABILITY", static_cast<int32_t>(AppExecFwk::ExecuteMode::UI_EXTENSION_ABILITY)},
        {"SERVICE_EXTENSION_ABILITY", static_cast<int32_t>(AppExecFwk::ExecuteMode::SERVICE_EXTENSION_ABILITY)},
        {"foreground", static_cast<int32_t>(AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND)},
        {"background", static_cast<int32_t>(AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND)},
        {"uiextension", static_cast<int32_t>(AppExecFwk::ExecuteMode::UI_EXTENSION_ABILITY)},
        {"serviceextension", static_cast<int32_t>(AppExecFwk::ExecuteMode::SERVICE_EXTENSION_ABILITY)},
    };
    auto it = MODE_MAP.find(str);
    if (it != MODE_MAP.end()) {
        out = it->second;
        return true;
    }
    return false;
}
} // namespace

int32_t InsightIntentParamParser::Build(const std::string &bundleName, const std::string &intentName,
    const AAFwk::WantParams &wantParam, const std::vector<ExtractInsightIntentGenericInfo> &candidates,
    int32_t callerUserId, ParseResult &out)
{
    if (bundleName.empty() || intentName.empty()) {
        TAG_LOGE(AAFwkTag::INTENT, "bundleName or intentName empty");
        return ERR_INVALID_VALUE;
    }

    // 规则 1 过滤 + 按 (moduleName, abilityName) 字典序排序。与注册侧 IntentFilterUtil::FilterGeneric 一致。
    // 取末条作为代表，等价于注册侧 KVStore last-wins 覆盖语义。
    std::vector<ExtractInsightIntentInfo> wrapped;
    wrapped.reserve(candidates.size());
    for (const auto &candidate : candidates) {
        ExtractInsightIntentInfo info;
        info.genericInfo = candidate;
        wrapped.push_back(std::move(info));
    }
    CliTool::IntentFilterUtil filter;
    filter.FilterGeneric(wrapped);
    if (wrapped.empty()) {
        TAG_LOGE(AAFwkTag::INTENT, "no qualified candidate after rule-1 filter");
        return ERR_INVALID_VALUE;
    }
    const auto &matched = wrapped.back().genericInfo;
    out.matchedInfo = matched;
    out.ignoreAbilityName = matched.decoratorType == INSIGHT_INTENTS_DECORATOR_TYPE_LINK
        || matched.decoratorType == INSIGHT_INTENTS_DECORATOR_TYPE_PAGE
        || matched.decoratorType == INSIGHT_INTENTS_DECORATOR_TYPE_FUNCTION;
    out.openLinkExecuteFlag = matched.decoratorType == INSIGHT_INTENTS_DECORATOR_TYPE_LINK;

    auto options = ExtractOptions(wantParam);

    auto param = std::make_shared<AppExecFwk::InsightIntentExecuteParam>();
    param->bundleName_ = bundleName;
    param->insightIntentName_ = intentName;
    param->insightIntentParam_ = std::make_shared<AAFwk::WantParams>(wantParam);
    param->userId_ = DEFAULT_USER_ID;
    param->displayId_ = AppExecFwk::INVALID_DISPLAY_ID;

    std::string optModuleName = options->GetStringParam(INSIGHT_INTENT_OPT_MODULE_NAME);
    param->moduleName_ = optModuleName.empty() ? matched.moduleName : optModuleName;

    std::string optAbilityName = options->GetStringParam(INSIGHT_INTENT_OPT_ABILITY_NAME);
    param->abilityName_ = optAbilityName.empty() ? GetAbilityNameFromMatched(matched) : optAbilityName;

    int32_t optMode = 0;
    param->executeMode_ = ResolveExecuteModeFromOption(options->GetStringParam(INSIGHT_INTENT_OPT_EXECUTE_MODE),
        optMode)
        ? optMode
        : GetExecuteModeFromMatched(matched);

    ResolveUris(*options, param->uris_);
    ResolveFlags(*options, param->flags_);

    out.param = param;
    return ERR_OK;
}

std::shared_ptr<AAFwk::WantParams> InsightIntentParamParser::ExtractOptions(
    const AAFwk::WantParams &wantParam) const
{
    if (!wantParam.HasParam(INSIGHT_INTENT_OPTIONS_KEY)) {
        return std::make_shared<AAFwk::WantParams>();
    }
    return std::make_shared<AAFwk::WantParams>(wantParam.GetWantParams(INSIGHT_INTENT_OPTIONS_KEY));
}

void InsightIntentParamParser::ResolveUris(const AAFwk::WantParams &opts,
    std::vector<std::string> &out) const
{
    AAFwk::IArray *ao = AAFwk::IArray::Query(opts.GetParam(INSIGHT_INTENT_OPT_URIS));
    if (ao == nullptr || !AAFwk::Array::IsStringArray(ao)) {
        return;
    }
    std::vector<std::string> uris;
    AAFwk::Array::ForEach(ao, [&uris](AAFwk::IInterface *object) {
        if (object == nullptr) {
            return;
        }
        AAFwk::IString *value = AAFwk::IString::Query(object);
        if (value != nullptr) {
            uris.push_back(AAFwk::String::Unbox(value));
        }
    });
    if (!uris.empty()) {
        out = std::move(uris);
    }
}

void InsightIntentParamParser::ResolveFlags(const AAFwk::WantParams &opts, int32_t &out) const
{
    std::string flagsStr = opts.GetStringParam(INSIGHT_INTENT_OPT_FLAGS);
    int32_t val = 0;
    if (ParseInt(flagsStr, AUTO_BASE, val)) {
        out = val;
    }
}
} // namespace AbilityRuntime
} // namespace OHOS
