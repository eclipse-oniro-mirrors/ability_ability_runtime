/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "cc_command.h"

#include <cerrno>
#include <climits>
#include <cstdlib>
#include <getopt.h>
#include <unordered_map>

#include <nlohmann/json.hpp>

#include "ability_manager_client.h"
#include "hilog_tag_wrapper.h"
#include "insight_intent/insight_intent_constant.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr size_t QUOTATION_MARK_PAIR_LEN = 2;
constexpr int32_t MAX_PARSE_COUNT = 256;
constexpr int32_t DEFAULT_USER_ID = 100;
constexpr int32_t QUERY_MODE_ALL = 1;
constexpr int32_t QUERY_MODE_BY_BUNDLE = 2;
constexpr int32_t QUERY_MODE_BY_INTENT = 3;

const std::string SHORT_OPTIONS_EXECUTE_INTENT = "hb:m:a:i:e:p:";
constexpr struct option LONG_OPTIONS_EXECUTE_INTENT[] = {
    {"help", no_argument, nullptr, 'h'},
    {"bundle", required_argument, nullptr, 'b'},
    {"bundleName", required_argument, nullptr, 'b'},
    {"module", required_argument, nullptr, 'm'},
    {"moduleName", required_argument, nullptr, 'm'},
    {"ability", required_argument, nullptr, 'a'},
    {"abilityName", required_argument, nullptr, 'a'},
    {"intent", required_argument, nullptr, 'i'},
    {"intentName", required_argument, nullptr, 'i'},
    {"execute-mode", required_argument, nullptr, 'e'},
    {"executeMode", required_argument, nullptr, 'e'},
    {"param", required_argument, nullptr, 'p'},
    {nullptr, 0, nullptr, 0},
};

const std::string SHORT_OPTIONS_GET_INTENT = "he:f:b:m:i:";
constexpr struct option LONG_OPTIONS_GET_INTENT[] = {
    {"help", no_argument, nullptr, 'h'},
    {"execute-mode", required_argument, nullptr, 'e'},
    {"executeMode", required_argument, nullptr, 'e'},
    {"flag", required_argument, nullptr, 'f'},
    {"bundle", required_argument, nullptr, 'b'},
    {"bundleName", required_argument, nullptr, 'b'},
    {"module", required_argument, nullptr, 'm'},
    {"moduleName", required_argument, nullptr, 'm'},
    {"intent", required_argument, nullptr, 'i'},
    {"intentName", required_argument, nullptr, 'i'},
    {nullptr, 0, nullptr, 0},
};
void StripQuotationMarks(std::string &str)
{
    if (str.size() >= QUOTATION_MARK_PAIR_LEN &&
        str.front() == '\'' && str.back() == '\'') {
        str = str.substr(1, str.size() - QUOTATION_MARK_PAIR_LEN);
    }
}
}  // namespace

using ErrCode = OHOS::ErrCode;

InsightIntentShellCommand::InsightIntentShellCommand(int argc, char* argv[])
    : ShellCommand(argc, argv, TOOL_NAME)
{
    for (int i = 0; i < argc_; i++) {
        TAG_LOGI(AAFwkTag::CC_TOOL, "argv_[%{public}d]: %{public}s", i, argv_[i]);
    }
}

ErrCode InsightIntentShellCommand::CreateCommandMap()
{
    commandMap_ = {
        {"help", [this]() { return this->RunAsHelpCommand(); }},
        {"-h", [this]() { return this->RunAsHelpCommand(); }},
        {"--help", [this]() { return this->RunAsHelpCommand(); }},
        {"execute-intent",
            [this]() { return this->RunAsExecuteIntentCommand(); }},
        {"get-intent", [this]() { return this->RunAsGetIntentCommand(); }},
    };

    return OHOS::ERR_OK;
}

ErrCode InsightIntentShellCommand::CreateMessageMap()
{
    return OHOS::ERR_OK;
}

ErrCode InsightIntentShellCommand::init()
{
    return AbilityManagerClient::GetInstance()->Connect();
}

ErrCode InsightIntentShellCommand::RunAsHelpCommand()
{
    resultReceiver_.append(HELP_MSG);
    return OHOS::ERR_OK;
}

ErrCode InsightIntentShellCommand::RunAsExecuteIntentCommand()
{
    TAG_LOGI(AAFwkTag::CC_TOOL, "execute-intent command called");

    std::string bundleName;
    std::string moduleName;
    std::string abilityName;
    std::string insightIntentName;
    std::string intentParamJson;
    int32_t executeMode = -1;

    auto result = ParseExecuteIntentOptions(bundleName, moduleName, abilityName,
        insightIntentName, intentParamJson, executeMode);
    if (result != OHOS::ERR_OK) {
        return result;
    }

    result = ValidateExecuteMode(executeMode);
    if (result != OHOS::ERR_OK) {
        return result;
    }

    AbilityRuntime::InsightIntentInfoForQuery queryInfo;
    result = ValidateIntentFromDatabase(bundleName, moduleName,
        insightIntentName, queryInfo);
    if (result != OHOS::ERR_OK) {
        return result;
    }

    InsightIntentExecuteParam param;
    BuildExecuteParam(param, bundleName, moduleName, abilityName,
        insightIntentName, executeMode, queryInfo.intentType, intentParamJson);
    return ExecuteIntentWithParam(param);
}

void InsightIntentShellCommand::BuildExecuteParam(
    InsightIntentExecuteParam &param,
    const std::string &bundleName, const std::string &moduleName,
    const std::string &abilityName, const std::string &insightIntentName,
    int32_t executeMode, const std::string &intentType,
    const std::string &intentParamJson)
{
    param.bundleName_ = bundleName;
    param.moduleName_ = moduleName;
    param.abilityName_ = abilityName;
    param.insightIntentName_ = insightIntentName;
    param.executeMode_ = executeMode;
    param.userId_ = DEFAULT_USER_ID;
    param.decoratorType_ = ConvertIntentTypeToDecoratorType(intentType);
    if (intentParamJson.empty()) {
        param.insightIntentParam_ = std::make_shared<WantParams>();
        return;
    }
    try {
        auto jsonObj = nlohmann::json::parse(intentParamJson);
        if (jsonObj.contains("uri") && jsonObj["uri"].is_string()) {
            param.uris_.push_back(jsonObj["uri"].get<std::string>());
            jsonObj.erase("uri");
        }
        param.insightIntentParam_ = std::make_shared<WantParams>(
            CcParamParser::BuildWantParamsFromJson(jsonObj.dump()));
    } catch (const nlohmann::json::exception &e) {
        TAG_LOGE(AAFwkTag::CC_TOOL, "parse param json failed: %{public}s", e.what());
        param.insightIntentParam_ = std::make_shared<WantParams>(
            CcParamParser::BuildWantParamsFromJson(intentParamJson));
    }
}

ErrCode InsightIntentShellCommand::ParseExecuteIntentOptions(
    std::string &bundleName, std::string &moduleName,
    std::string &abilityName, std::string &insightIntentName,
    std::string &intentParamJson, int32_t &executeMode)
{
    int option = -1;
    int counter = 0;

    while (counter < MAX_PARSE_COUNT) {
        counter++;
        option = getopt_long(argc_, argv_, SHORT_OPTIONS_EXECUTE_INTENT.c_str(),
            LONG_OPTIONS_EXECUTE_INTENT, nullptr);

        TAG_LOGI(AAFwkTag::CC_TOOL,
            "option: %{public}d, optopt: %{public}d, optind: %{public}d",
            option, optopt, optind);

        if (optind < 0 || optind > argc_) {
            AppendErrorEvent("ERR_ARG_MISSING",
                "Missing required parameters for execute-intent.",
                "Please provide --bundleName, --moduleName and --intentName.");
            return OHOS::ERR_INVALID_VALUE;
        }

        if (option == -1) {
            if (counter == 1 && optind < argc_ &&
                strcmp(argv_[optind], cmd_.c_str()) == 0) {
                AppendErrorEvent("ERR_ARG_MISSING",
                    "No options specified for execute-intent.",
                    "Run 'ohos-claw-cc execute-intent --help' for usage.");
            }
            break;
        }

        if (option == '?') {
            TAG_LOGI(AAFwkTag::CC_TOOL, "'ohos-claw-cc execute-intent' option unknown");
            AppendErrorEvent("ERR_ARG_INVALID",
                "Unknown option for execute-intent.",
                "Run 'ohos-claw-cc execute-intent --help' for valid options.");
            return OHOS::ERR_INVALID_VALUE;
        }

        if (HandleExecuteIntentOption(option, bundleName, moduleName, abilityName,
            insightIntentName, intentParamJson, executeMode) != OHOS::ERR_OK) {
            return OHOS::ERR_INVALID_VALUE;
        }
    }

    return CheckAllExecuteParams(bundleName, moduleName,
        insightIntentName, abilityName, executeMode, intentParamJson);
}

ErrCode InsightIntentShellCommand::CheckRequiredExecuteParams(
    const std::string &bundleName, const std::string &moduleName,
    const std::string &insightIntentName)
{
    if (bundleName.empty() || moduleName.empty() ||
        insightIntentName.empty()) {
        TAG_LOGE(AAFwkTag::CC_TOOL, "missing required parameters");
        AppendErrorEvent("ERR_ARG_MISSING",
            "Missing required parameters for execute-intent.",
            "Please provide --bundleName, --moduleName and --intentName.");
        return OHOS::ERR_INVALID_VALUE;
    }
    return OHOS::ERR_OK;
}

ErrCode InsightIntentShellCommand::CheckAllExecuteParams(
    const std::string &bundleName, const std::string &moduleName,
    const std::string &insightIntentName, const std::string &abilityName,
    int32_t executeMode, const std::string &intentParamJson)
{
    if (bundleName.empty() || moduleName.empty() ||
        insightIntentName.empty() || executeMode < 0 ||
        intentParamJson.empty()) {
        TAG_LOGE(AAFwkTag::CC_TOOL, "missing required parameters");
        AppendErrorEvent("ERR_ARG_MISSING",
            "Missing required parameters for execute-intent.",
            "Please provide --bundleName, --moduleName, --intentName, "
            "and --param. "
            "Pass empty string '' for --abilityName or '{}' for --param "
            "if not needed.");
        return OHOS::ERR_INVALID_VALUE;
    }
    return OHOS::ERR_OK;
}

ErrCode InsightIntentShellCommand::HandleExecuteIntentOption(
    int option, std::string &bundleName, std::string &moduleName,
    std::string &abilityName, std::string &insightIntentName,
    std::string &intentParamJson, int32_t &executeMode)
{
    switch (option) {
        case 'h':
            resultReceiver_.append(HELP_MSG_EXECUTE_INTENT + "\n");
            return OHOS::ERR_INVALID_VALUE;
        case 'b':
            bundleName = optarg;
            TAG_LOGI(AAFwkTag::CC_TOOL, "bundleName: %{public}s", bundleName.c_str());
            break;
        case 'm':
            moduleName = optarg;
            TAG_LOGI(AAFwkTag::CC_TOOL, "moduleName: %{public}s", moduleName.c_str());
            break;
        case 'a':
            abilityName = optarg;
            TAG_LOGI(AAFwkTag::CC_TOOL, "abilityName: %{public}s", abilityName.c_str());
            break;
        case 'i':
            insightIntentName = optarg;
            TAG_LOGI(AAFwkTag::CC_TOOL,
                "insightIntentName: %{public}s", insightIntentName.c_str());
            break;
        case 'e':
            if (ParseIntOption(optarg, executeMode) != OHOS::ERR_OK) {
                AppendErrorEvent("ERR_ARG_INVALID",
                    "Invalid executeMode value for execute-intent.",
                    "ExecuteMode must be an integer 0-3.");
                return OHOS::ERR_INVALID_VALUE;
            }
            break;
        case 'p':
            intentParamJson = optarg;
            StripQuotationMarks(intentParamJson);
            TAG_LOGI(AAFwkTag::CC_TOOL,
                "intentParamJson: %{public}s", intentParamJson.c_str());
            break;
        default:
            break;
    }
    return OHOS::ERR_OK;
}

ErrCode InsightIntentShellCommand::ValidateIntentFromDatabase(
    const std::string &bundleName, const std::string &moduleName,
    const std::string &insightIntentName,
    AbilityRuntime::InsightIntentInfoForQuery &queryInfo)
{
    auto err = AbilityManagerClient::GetInstance()->GetInsightIntentInfoByIntentName(
        AbilityRuntime::GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT,
        bundleName, moduleName, insightIntentName, queryInfo, DEFAULT_USER_ID);
    if (err != OHOS::ERR_OK || queryInfo.intentName.empty()) {
        TAG_LOGE(AAFwkTag::CC_TOOL,
            "intent not found: %{public}s, err: %{public}d",
            insightIntentName.c_str(), err);
        AppendErrorEvent("ERR_RESOURCE_NOT_FOUND",
            "Intent not found in database: " + insightIntentName,
            "Check --bundleName, --moduleName and --intentName.");
        return OHOS::ERR_INVALID_VALUE;
    }

    TAG_LOGI(AAFwkTag::CC_TOOL,
        "intent validated: %{public}s, intentType: %{public}s",
        insightIntentName.c_str(), queryInfo.intentType.c_str());
    return OHOS::ERR_OK;
}

int8_t InsightIntentShellCommand::ConvertIntentTypeToDecoratorType(
    const std::string &intentType)
{
    static const std::unordered_map<std::string, int8_t> mapping = {
        {"@InsightIntentLink", 1},
        {"@InsightIntentPage", 2},
        {"@InsightIntentFunctionMethod", 3},
        {"@InsightIntentForm", 4},
        {"@InsightIntentEntry", 5},
    };
    auto it = mapping.find(intentType);
    if (it != mapping.end()) {
        return it->second;
    }
    return 0;
}

ErrCode InsightIntentShellCommand::ExecuteIntentWithParam(
    const InsightIntentExecuteParam &param)
{
    TAG_LOGI(AAFwkTag::CC_TOOL,
        "Executing insight intent: bundle=%{public}s, module=%{public}s, "
        "ability=%{public}s, intent=%{public}s, mode=%{public}d",
        param.bundleName_.c_str(), param.moduleName_.c_str(),
        param.abilityName_.c_str(), param.insightIntentName_.c_str(),
        param.executeMode_);

    InsightIntentExecuteResult executeResult;
    auto err = AbilityManagerClient::GetInstance()->ExecuteIntentWithResult(
        const_cast<InsightIntentExecuteParam &>(param), executeResult, 30000);
    if (err == OHOS::ERR_OK) {
        TAG_LOGI(AAFwkTag::CC_TOOL,
            "execute intent successfully, innerErr=%{public}d, code=%{public}d",
            executeResult.innerErr, executeResult.code);
        AppendExecuteResult(executeResult);
    } else {
        TAG_LOGE(AAFwkTag::CC_TOOL, "execute intent failed: %{public}d", err);
        AppendErrorEvent("ERR_INTERNAL_ERROR",
            "Failed to execute intent: " + GetMessageFromCode(err),
            "Check if the target application is installed and the intent is registered.");
    }

    return err;
}

void InsightIntentShellCommand::AppendExecuteResult(
    const InsightIntentExecuteResult &executeResult)
{
    nlohmann::json data;
    data["innerErr"] = executeResult.innerErr;
    data["code"] = executeResult.code;
    data["flags"] = executeResult.flags;

    if (executeResult.result != nullptr) {
        data["result"] = executeResult.result->ToString();
    }
    if (!executeResult.uris.empty()) {
        data["uris"] = executeResult.uris;
    }

    AppendResultEvent(data);
}

ErrCode InsightIntentShellCommand::RunAsGetIntentCommand()
{
    TAG_LOGI(AAFwkTag::CC_TOOL, "get-intent command called");

    int32_t flag = -1;
    int32_t executeMode = -1;
    std::string bundleName;
    std::string moduleName;
    std::string intentName;

    auto result = ParseGetIntentOptions(flag, executeMode,
        bundleName, moduleName, intentName);
    if (result != OHOS::ERR_OK) {
        return result;
    }

    return DispatchGetIntentMode(executeMode, flag,
        bundleName, moduleName, intentName);
}

ErrCode InsightIntentShellCommand::ParseGetIntentOptions(
    int32_t &flag, int32_t &executeMode,
    std::string &bundleName, std::string &moduleName,
    std::string &intentName)
{
    int option = -1;
    int counter = 0;

    while (counter < MAX_PARSE_COUNT) {
        counter++;
        option = getopt_long(argc_, argv_, SHORT_OPTIONS_GET_INTENT.c_str(),
            LONG_OPTIONS_GET_INTENT, nullptr);

        TAG_LOGI(AAFwkTag::CC_TOOL,
            "option: %{public}d, optopt: %{public}d, optind: %{public}d",
            option, optopt, optind);

        if (optind < 0 || optind > argc_) {
            AppendErrorEvent("ERR_ARG_MISSING",
                "Missing required parameters for get-intent.",
                "Please provide --executeMode and --flag.");
            return OHOS::ERR_INVALID_VALUE;
        }

        if (option == -1) {
            if (counter == 1 && optind < argc_ &&
                strcmp(argv_[optind], cmd_.c_str()) == 0) {
                AppendErrorEvent("ERR_ARG_MISSING",
                    "No options specified for get-intent.",
                    "Run 'ohos-claw-cc get-intent --help' for usage.");
            }
            break;
        }

        if (option == '?') {
            TAG_LOGI(AAFwkTag::CC_TOOL, "'ohos-claw-cc get-intent' option unknown");
            AppendErrorEvent("ERR_ARG_INVALID",
                "Unknown option for get-intent.",
                "Run 'ohos-claw-cc get-intent --help' for valid options.");
            return OHOS::ERR_INVALID_VALUE;
        }

        if (HandleGetIntentOption(option, flag, executeMode,
            bundleName, moduleName, intentName) != OHOS::ERR_OK) {
            return OHOS::ERR_INVALID_VALUE;
        }
    }

    return ValidateGetIntentParams(flag, executeMode);
}

ErrCode InsightIntentShellCommand::ValidateGetIntentParams(
    int32_t flag, int32_t executeMode)
{
    if (executeMode < 0 || flag < 0) {
        TAG_LOGE(AAFwkTag::CC_TOOL, "missing required parameters -e or -f");
        AppendErrorEvent("ERR_ARG_MISSING",
            "Missing required parameters --executeMode and --flag.",
            "Please provide --executeMode (1-3) and --flag (1, 2, 5 or 6).");
        return OHOS::ERR_INVALID_VALUE;
    }

    if (flag != AbilityRuntime::GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT &&
        flag != AbilityRuntime::GetInsightIntentFlag::GET_SUMMARY_INSIGHT_INTENT &&
        flag != AbilityRuntime::GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT_ENTITY &&
        flag != AbilityRuntime::GetInsightIntentFlag::GET_SUMMARY_INSIGHT_INTENT_ENTITY) {
        TAG_LOGE(AAFwkTag::CC_TOOL, "invalid flag: %{public}d", flag);
        AppendErrorEvent("ERR_ARG_INVALID",
            "Invalid flag value: " + std::to_string(flag),
            "Flag must be one of: 1 (GET_FULL_INSIGHT_INTENT), "
            "2 (GET_SUMMARY_INSIGHT_INTENT), "
            "5 (FULL + ENTITY_INFO), 6 (SUMMARY + ENTITY_INFO).");
        return OHOS::ERR_INVALID_VALUE;
    }

    return OHOS::ERR_OK;
}

ErrCode InsightIntentShellCommand::HandleGetIntentOption(
    int option, int32_t &flag, int32_t &executeMode,
    std::string &bundleName, std::string &moduleName,
    std::string &intentName)
{
    switch (option) {
        case 'h':
            resultReceiver_.append(HELP_MSG_GET_INTENT + "\n");
            return OHOS::ERR_INVALID_VALUE;
        case 'e':
            if (ParseIntOption(optarg, executeMode) != OHOS::ERR_OK) {
                AppendErrorEvent("ERR_ARG_INVALID",
                    "Invalid executeMode value for get-intent.",
                    "ExecuteMode must be an integer 1-3.");
                return OHOS::ERR_INVALID_VALUE;
            }
            TAG_LOGI(AAFwkTag::CC_TOOL, "executeMode: %{public}d", executeMode);
            break;
        case 'f':
            if (ParseIntOption(optarg, flag) != OHOS::ERR_OK) {
                AppendErrorEvent("ERR_ARG_INVALID",
                    "Invalid flag value for get-intent.",
                    "Flag must be an integer 1-7.");
                return OHOS::ERR_INVALID_VALUE;
            }
            TAG_LOGI(AAFwkTag::CC_TOOL, "flag: %{public}d", flag);
            break;
        case 'b':
            bundleName = optarg;
            TAG_LOGI(AAFwkTag::CC_TOOL,
                "bundleName: %{public}s", bundleName.c_str());
            break;
        case 'm':
            moduleName = optarg;
            TAG_LOGI(AAFwkTag::CC_TOOL,
                "moduleName: %{public}s", moduleName.c_str());
            break;
        case 'i':
            intentName = optarg;
            TAG_LOGI(AAFwkTag::CC_TOOL,
                "intentName: %{public}s", intentName.c_str());
            break;
        default:
            break;
    }
    return OHOS::ERR_OK;
}

ErrCode InsightIntentShellCommand::DispatchGetIntentMode(
    int32_t executeMode, int32_t flag,
    const std::string &bundleName, const std::string &moduleName,
    const std::string &intentName)
{
    if (executeMode == QUERY_MODE_ALL) {
        return RunGetIntentModeAll(flag);
    } else if (executeMode == QUERY_MODE_BY_BUNDLE) {
        if (bundleName.empty()) {
            TAG_LOGE(AAFwkTag::CC_TOOL, "mode 2 requires -b bundleName");
            AppendErrorEvent("ERR_ARG_MISSING",
                "executeMode 2 requires --bundleName.",
                "Please provide --bundleName for query by bundle.");
            return OHOS::ERR_INVALID_VALUE;
        }
        return RunGetIntentModeByBundle(flag, bundleName);
    } else if (executeMode == QUERY_MODE_BY_INTENT) {
        if (bundleName.empty() || moduleName.empty() || intentName.empty()) {
            TAG_LOGE(AAFwkTag::CC_TOOL, "mode 3 requires -b, -m, -i");
            AppendErrorEvent("ERR_ARG_MISSING",
                "executeMode 3 requires --bundleName, --moduleName and --intentName.",
                "Please provide all three parameters for query by intent.");
            return OHOS::ERR_INVALID_VALUE;
        }
        return RunGetIntentModeByIntent(flag,
            bundleName, moduleName, intentName);
    }

    TAG_LOGE(AAFwkTag::CC_TOOL,
        "invalid execute mode: %{public}d", executeMode);
    AppendErrorEvent("ERR_ARG_INVALID",
        "Invalid executeMode value: " + std::to_string(executeMode),
        "executeMode must be 1 (all), 2 (by bundle), or 3 (by intent).");
    return OHOS::ERR_INVALID_VALUE;
}

ErrCode InsightIntentShellCommand::ParseIntOption(
    const char *arg, int32_t &value)
{
    if (arg == nullptr || *arg == '\0') {
        TAG_LOGE(AAFwkTag::CC_TOOL, "invalid integer value: empty");
        return OHOS::ERR_INVALID_VALUE;
    }

    char *end = nullptr;
    errno = 0;
    long result = strtol(arg, &end, 10);
    if (errno != 0 || end == arg || *end != '\0' ||
        result < INT32_MIN || result > INT32_MAX) {
        TAG_LOGE(AAFwkTag::CC_TOOL,
            "invalid integer value: %{public}s", arg);
        return OHOS::ERR_INVALID_VALUE;
    }
    value = static_cast<int32_t>(result);
    return OHOS::ERR_OK;
}

ErrCode InsightIntentShellCommand::ValidateExecuteMode(int32_t executeMode)
{
    if (executeMode < 0) {
        return OHOS::ERR_OK;
    }

    constexpr int32_t MAX_EXECUTE_MODE =
        static_cast<int32_t>(AppExecFwk::ExecuteMode::SERVICE_EXTENSION_ABILITY);
    if (executeMode > MAX_EXECUTE_MODE) {
        TAG_LOGE(AAFwkTag::CC_TOOL,
            "invalid execute mode: %{public}d", executeMode);
        AppendErrorEvent("ERR_ARG_OUT_OF_RANGE",
            "executeMode out of range: " + std::to_string(executeMode),
            "executeMode must be 0-3.");
        return OHOS::ERR_INVALID_VALUE;
    }
    return OHOS::ERR_OK;
}

ErrCode InsightIntentShellCommand::RunGetIntentModeAll(int32_t flag)
{
    std::vector<AbilityRuntime::InsightIntentInfoForQuery> infos;
    TAG_LOGI(AAFwkTag::CC_TOOL,
        "GetAllInsightIntentInfo with flag: %{public}d", flag);

    auto err = AbilityManagerClient::GetInstance()->GetAllInsightIntentInfo(
        static_cast<AbilityRuntime::GetInsightIntentFlag>(flag), infos,
        DEFAULT_USER_ID);
    if (err == OHOS::ERR_OK) {
        TAG_LOGI(AAFwkTag::CC_TOOL,
            "get all insight intent info successfully, count: %{public}zu",
            infos.size());
        nlohmann::json data;
        data["mode"] = "all";
        data["total"] = infos.size();
        data["intents"] = BuildIntentInfoListJson(infos, flag);
        AppendResultEvent(data);
    } else {
        TAG_LOGE(AAFwkTag::CC_TOOL,
            "get all insight intent info failed: %{public}d", err);
        AppendErrorEvent("ERR_INTERNAL_ERROR",
            "Failed to get all insight intent info: " + GetMessageFromCode(err),
            "Check if AbilityManagerService is running.");
    }

    return err;
}

ErrCode InsightIntentShellCommand::RunGetIntentModeByBundle(
    int32_t flag, const std::string &bundleName)
{
    std::vector<AbilityRuntime::InsightIntentInfoForQuery> infos;
    TAG_LOGI(AAFwkTag::CC_TOOL,
        "GetInsightIntentInfoByBundleName with flag: %{public}d, "
        "bundle: %{public}s",
        flag, bundleName.c_str());

    auto err = AbilityManagerClient::GetInstance()->GetInsightIntentInfoByBundleName(
        static_cast<AbilityRuntime::GetInsightIntentFlag>(flag),
        bundleName, infos, DEFAULT_USER_ID);
    if (err == OHOS::ERR_OK) {
        TAG_LOGI(AAFwkTag::CC_TOOL,
            "get insight intent info by bundle successfully, count: %{public}zu",
            infos.size());
        nlohmann::json data;
        data["mode"] = "by bundle";
        data["bundleName"] = bundleName;
        data["total"] = infos.size();
        data["intents"] = BuildIntentInfoListJson(infos, flag);
        AppendResultEvent(data);
    } else {
        TAG_LOGE(AAFwkTag::CC_TOOL,
            "get insight intent info by bundle failed: %{public}d", err);
        AppendErrorEvent("ERR_INTERNAL_ERROR",
            "Failed to get insight intent info by bundle: " + GetMessageFromCode(err),
            "Check if --bundleName is correct and AbilityManagerService is running.");
    }

    return err;
}

ErrCode InsightIntentShellCommand::RunGetIntentModeByIntent(
    int32_t flag, const std::string &bundleName,
    const std::string &moduleName, const std::string &intentName)
{
    AbilityRuntime::InsightIntentInfoForQuery info;
    TAG_LOGI(AAFwkTag::CC_TOOL,
        "GetInsightIntentInfoByIntentName with flag: %{public}d, "
        "bundle: %{public}s, module: %{public}s, intent: %{public}s",
        flag, bundleName.c_str(), moduleName.c_str(),
        intentName.c_str());

    auto err = AbilityManagerClient::GetInstance()->GetInsightIntentInfoByIntentName(
        static_cast<AbilityRuntime::GetInsightIntentFlag>(flag),
        bundleName, moduleName, intentName, info, DEFAULT_USER_ID);
    if (err == OHOS::ERR_OK && info.intentName.empty()) {
        TAG_LOGE(AAFwkTag::CC_TOOL, "intent not found: %{public}s",
            intentName.c_str());
        AppendErrorEvent("ERR_RESOURCE_NOT_FOUND",
            "Intent not found: " + intentName,
            "Check --bundleName, --moduleName and --intentName are correct.");
        return OHOS::ERR_INVALID_VALUE;
    }

    if (err == OHOS::ERR_OK) {
        TAG_LOGI(AAFwkTag::CC_TOOL,
            "get insight intent info by intent name successfully");
        nlohmann::json data;
        data["mode"] = "by intent";
        data["intent"] = BuildIntentInfoJson(info, flag);
        AppendResultEvent(data);
    } else {
        TAG_LOGE(AAFwkTag::CC_TOOL,
            "get insight intent info by intent name failed: %{public}d", err);
        AppendErrorEvent("ERR_INTERNAL_ERROR",
            "Failed to get insight intent info by intent name: " +
            GetMessageFromCode(err),
            "Check if AbilityManagerService is running.");
    }

    return err;
}

void InsightIntentShellCommand::AppendResultEvent(const nlohmann::json &data)
{
    nlohmann::json event;
    event["type"] = "result";
    event["status"] = "success";
    event["data"] = data;
    resultReceiver_.append(event.dump());
}

void InsightIntentShellCommand::AppendErrorEvent(const std::string &errCode,
    const std::string &errMsg, const std::string &suggestion)
{
    nlohmann::json event;
    event["type"] = "result";
    event["status"] = "failed";
    event["errCode"] = errCode;
    event["errMsg"] = errMsg;
    event["suggestion"] = suggestion;
    resultReceiver_.append(event.dump());
    (void)fprintf(stderr, "%s\n", event.dump().c_str());
}

bool InsightIntentShellCommand::ShouldAppendEntityInfo(int32_t flag) const
{
    return (flag & AbilityRuntime::GetInsightIntentFlag::GET_ENTITY_INFO) != 0;
}

bool InsightIntentShellCommand::IsFullInfo(int32_t flag) const
{
    return (flag & AbilityRuntime::GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT) != 0;
}

nlohmann::json InsightIntentShellCommand::BuildIntentInfoListJson(
    const std::vector<AbilityRuntime::InsightIntentInfoForQuery> &infos,
    int32_t flag)
{
    nlohmann::json arr = nlohmann::json::array();
    for (const auto &info : infos) {
        arr.emplace_back(BuildIntentInfoJson(info, flag));
    }
    return arr;
}

nlohmann::json InsightIntentShellCommand::BuildIntentInfoJson(
    const AbilityRuntime::InsightIntentInfoForQuery &info, int32_t flag)
{
    nlohmann::json j;
    j["bundleName"] = info.bundleName;
    j["moduleName"] = info.moduleName;
    j["intentName"] = info.intentName;
    j["displayName"] = info.displayName;
    j["intentType"] = info.intentType;
    j["developType"] = info.develoType;
    j["parameters"] = info.parameters;

    if (IsFullInfo(flag)) {
        j["domain"] = info.domain;
        j["intentVersion"] = info.intentVersion;
        j["displayDescription"] = info.displayDescription;
        j["schema"] = info.schema;
        j["icon"] = info.icon;
        j["llmDescription"] = info.llmDescription;
        if (!info.keywords.empty()) {
            j["keywords"] = info.keywords;
        }
    }

    if (!info.linkInfo.uri.empty()) {
        j["linkInfo"]["uri"] = info.linkInfo.uri;
    }

    const auto &page = info.pageInfo;
    if (!page.uiAbility.empty() || !page.pagePath.empty()) {
        j["pageInfo"]["uiAbility"] = page.uiAbility;
        j["pageInfo"]["pagePath"] = page.pagePath;
        j["pageInfo"]["navigationId"] = page.navigationId;
        j["pageInfo"]["navDestinationName"] = page.navDestinationName;
    }

    const auto &entry = info.entryInfo;
    if (!entry.abilityName.empty()) {
        j["entryInfo"]["abilityName"] = entry.abilityName;
        j["entryInfo"]["executeMode"] = CollectSupportedModesJson(info);
    }

    const auto &form = info.formInfo;
    if (!form.abilityName.empty()) {
        j["formInfo"]["abilityName"] = form.abilityName;
        j["formInfo"]["formName"] = form.formName;
    }

    if (ShouldAppendEntityInfo(flag) && !info.entities.empty()) {
        nlohmann::json entities = nlohmann::json::array();
        for (const auto &entity : info.entities) {
            entities.emplace_back(BuildEntityJson(entity));
        }
        j["entities"] = entities;
    }

    return j;
}

nlohmann::json InsightIntentShellCommand::BuildEntityJson(
    const AbilityRuntime::EntityInfoForQuery &entity)
{
    nlohmann::json j;
    j["className"] = entity.className;
    j["entityId"] = entity.entityId;
    j["entityCategory"] = entity.entityCategory;
    j["parameters"] = entity.parameters;
    j["parentClassName"] = entity.parentClassName;
    return j;
}

nlohmann::json InsightIntentShellCommand::CollectSupportedModesJson(
    const AbilityRuntime::InsightIntentInfoForQuery &queryInfo)
{
    nlohmann::json arr = nlohmann::json::array();
    for (auto mode : queryInfo.entryInfo.executeMode) {
        arr.emplace_back(static_cast<int32_t>(mode));
    }
    for (auto mode : queryInfo.uiAbilityIntentInfo.supportExecuteMode) {
        arr.emplace_back(static_cast<int32_t>(mode));
    }
    return arr;
}
}  // namespace AAFwk
}  // namespace OHOS
