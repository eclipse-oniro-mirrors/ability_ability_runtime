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

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <cstdint>
#include <set>
#include <unordered_map>

#include "ability_manager_errors.h"
#include "array_wrapper.h"
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
constexpr const char *INSIGHT_INTENT_OPT_USER_ID = "userId";
constexpr const char *INSIGHT_INTENT_OPT_DISPLAY_ID = "displayId";
constexpr const char *INSIGHT_INTENT_OPT_DEVICE_ID = "deviceId";

constexpr int DECIMAL_BASE = 10;
constexpr int AUTO_BASE = 0;

std::string GetAlphaFirstString(std::vector<std::string> vals)
{
    if (vals.empty()) {
        return "";
    }
    std::sort(vals.begin(), vals.end());
    return vals.front();
}

int32_t ExecuteModeToInt(AppExecFwk::ExecuteMode mode)
{
    return static_cast<int32_t>(mode);
}

bool ParseInt(const std::string &s, int base, int32_t &out)
{
    if (s.empty()) {
        return false;
    }
    errno = 0;
    char *end = nullptr;
    long long val = std::strtoll(s.c_str(), &end, base);
    if (end == s.c_str() || *end != '\0' || errno == ERANGE) {
        return false;
    }
    if (val < INT32_MIN || val > INT32_MAX) {
        return false;
    }
    out = static_cast<int32_t>(val);
    return true;
}

std::string GetAbilityName(const ExtractInsightIntentGenericInfo &c)
{
    if (c.currentType == InfoType::Entry) {
        return c.get<InsightIntentEntryInfo>().abilityName;
    }
    if (c.currentType == InfoType::Page) {
        return c.get<InsightIntentPageInfo>().uiAbility;
    }
    if (c.currentType == InfoType::Form) {
        return c.get<InsightIntentFormInfo>().abilityName;
    }
    return "";
}

void CollectExecuteModes(const ExtractInsightIntentGenericInfo &c, std::set<int32_t> &out)
{
    if (c.currentType == InfoType::Entry) {
        for (auto m : c.get<InsightIntentEntryInfo>().executeMode) {
            out.insert(ExecuteModeToInt(m));
        }
        return;
    }
    if (c.currentType == InfoType::Function) {
        out.insert(ExecuteModeToInt(AppExecFwk::ExecuteMode::SERVICE_EXTENSION_ABILITY));
        return;
    }
    if (c.currentType == InfoType::Link || c.currentType == InfoType::Page ||
        c.currentType == InfoType::Form) {
        out.insert(ExecuteModeToInt(AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND));
    }
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

    std::vector<ExtractInsightIntentGenericInfo> active = candidates;
    PickActiveCandidates(active, out.ignoreAbilityName, out.openLinkExecuteFlag);
    if (!active.empty()) {
        out.representative = active.front();
    }

    auto options = ExtractOptions(wantParam);

    auto param = std::make_shared<AppExecFwk::InsightIntentExecuteParam>();
    param->bundleName_ = bundleName;
    param->insightIntentName_ = intentName;
    param->insightIntentParam_ = std::make_shared<AAFwk::WantParams>(wantParam);
    param->userId_ = callerUserId;
    param->displayId_ = AppExecFwk::INVALID_DISPLAY_ID;

    ResolveModuleName(*options, active, param->moduleName_);
    ResolveExecuteMode(*options, active, param->executeMode_);
    ResolveAbilityName(*options, active, param->abilityName_);
    ResolveUris(*options, param->uris_);
    ResolveFlags(*options, param->flags_);
    ResolveUserId(*options, callerUserId, param->userId_);
    ResolveDisplayId(*options, param->displayId_);
    ResolveDeviceId(*options, param->deviceId_);

    out.param = param;
    return ERR_OK;
}

void InsightIntentParamParser::PickActiveCandidates(std::vector<ExtractInsightIntentGenericInfo> &active,
    bool &ignoreAbilityName, bool &openLinkExecuteFlag) const
{
    ignoreAbilityName = false;
    openLinkExecuteFlag = false;
    if (active.empty()) {
        return;
    }
    auto minIt = std::min_element(active.begin(), active.end(),
        [](const ExtractInsightIntentGenericInfo &a, const ExtractInsightIntentGenericInfo &b) {
            return a.decoratorType < b.decoratorType;
        });
    std::string activeDecorator = minIt->decoratorType;
    active.erase(std::remove_if(active.begin(), active.end(),
        [&activeDecorator](const ExtractInsightIntentGenericInfo &c) {
            return c.decoratorType != activeDecorator;
        }), active.end());

    ignoreAbilityName = activeDecorator == INSIGHT_INTENTS_DECORATOR_TYPE_LINK
        || activeDecorator == INSIGHT_INTENTS_DECORATOR_TYPE_PAGE
        || activeDecorator == INSIGHT_INTENTS_DECORATOR_TYPE_FUNCTION;
    openLinkExecuteFlag = activeDecorator == INSIGHT_INTENTS_DECORATOR_TYPE_LINK;
}

std::shared_ptr<AAFwk::WantParams> InsightIntentParamParser::ExtractOptions(
    const AAFwk::WantParams &wantParam) const
{
    if (!wantParam.HasParam(INSIGHT_INTENT_OPTIONS_KEY)) {
        return std::make_shared<AAFwk::WantParams>();
    }
    return std::make_shared<AAFwk::WantParams>(wantParam.GetWantParams(INSIGHT_INTENT_OPTIONS_KEY));
}

void InsightIntentParamParser::ResolveModuleName(const AAFwk::WantParams &opts,
    const std::vector<ExtractInsightIntentGenericInfo> &active, std::string &out) const
{
    std::string opt = opts.GetStringParam(INSIGHT_INTENT_OPT_MODULE_NAME);
    if (!opt.empty()) {
        out = std::move(opt);
        return;
    }
    std::vector<std::string> names;
    for (const auto &c : active) {
        if (!c.moduleName.empty()) {
            names.push_back(c.moduleName);
        }
    }
    out = GetAlphaFirstString(std::move(names));
}

void InsightIntentParamParser::ResolveExecuteMode(const AAFwk::WantParams &opts,
    const std::vector<ExtractInsightIntentGenericInfo> &active, int32_t &out) const
{
    std::string opt = opts.GetStringParam(INSIGHT_INTENT_OPT_EXECUTE_MODE);
    if (!opt.empty()) {
        static const std::unordered_map<std::string, int32_t> MODE_MAP = {
            {"UI_ABILITY_FOREGROUND", ExecuteModeToInt(AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND)},
            {"UI_ABILITY_BACKGROUND", ExecuteModeToInt(AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND)},
            {"UI_EXTENSION_ABILITY", ExecuteModeToInt(AppExecFwk::ExecuteMode::UI_EXTENSION_ABILITY)},
            {"SERVICE_EXTENSION_ABILITY", ExecuteModeToInt(AppExecFwk::ExecuteMode::SERVICE_EXTENSION_ABILITY)},
        };
        auto it = MODE_MAP.find(opt);
        if (it != MODE_MAP.end()) {
            out = it->second;
            return;
        }
        if (ParseInt(opt, DECIMAL_BASE, out)) {
            return;
        }
    }

    std::set<int32_t> modeSet;
    for (const auto &c : active) {
        CollectExecuteModes(c, modeSet);
    }
    if (modeSet.size() > 1) {
        out = ExecuteModeToInt(AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND);
    } else if (modeSet.size() == 1) {
        out = *modeSet.begin();
    } else {
        out = ExecuteModeToInt(AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND);
    }
}

void InsightIntentParamParser::ResolveAbilityName(const AAFwk::WantParams &opts,
    const std::vector<ExtractInsightIntentGenericInfo> &active, std::string &out) const
{
    std::string opt = opts.GetStringParam(INSIGHT_INTENT_OPT_ABILITY_NAME);
    if (!opt.empty()) {
        out = std::move(opt);
        return;
    }
    std::vector<std::string> names;
    for (const auto &c : active) {
        std::string name = GetAbilityName(c);
        if (!name.empty()) {
            names.push_back(name);
        }
    }
    out = GetAlphaFirstString(std::move(names));
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

void InsightIntentParamParser::ResolveUserId(const AAFwk::WantParams &opts,
    int32_t callerUserId, int32_t &out) const
{
    out = callerUserId;
    std::string userIdStr = opts.GetStringParam(INSIGHT_INTENT_OPT_USER_ID);
    int32_t val = 0;
    if (ParseInt(userIdStr, DECIMAL_BASE, val)) {
        out = val;
    }
}

void InsightIntentParamParser::ResolveDisplayId(const AAFwk::WantParams &opts, int32_t &out) const
{
    out = AppExecFwk::INVALID_DISPLAY_ID;
    std::string displayIdStr = opts.GetStringParam(INSIGHT_INTENT_OPT_DISPLAY_ID);
    int32_t val = 0;
    if (ParseInt(displayIdStr, DECIMAL_BASE, val)) {
        out = val;
    }
}

void InsightIntentParamParser::ResolveDeviceId(const AAFwk::WantParams &opts, std::string &out) const
{
    out.clear();
    std::string dev = opts.GetStringParam(INSIGHT_INTENT_OPT_DEVICE_ID);
    if (!dev.empty()) {
        out = std::move(dev);
    }
}
} // namespace AbilityRuntime
} // namespace OHOS
