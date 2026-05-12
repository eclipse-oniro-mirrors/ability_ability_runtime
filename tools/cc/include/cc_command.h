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

#ifndef OHOS_ABILITY_RUNTIME_CC_COMMAND_H
#define OHOS_ABILITY_RUNTIME_CC_COMMAND_H

#include <nlohmann/json.hpp>

#include "ability_manager_interface.h"
#include "cc_param_parser.h"
#include "insight_intent/insight_intent_info_for_query.h"
#include "shell_command.h"

namespace OHOS {
namespace AAFwk {
namespace {
const std::string TOOL_NAME = "ohos-claw-cc";

const std::string HELP_MSG =
    "ohos-claw-cc - InsightIntent framework CLI tool for executing and querying insight intents\n"
    "\n"
    "Usage:\n"
    "  ohos-claw-cc <command> [options]\n"
    "\n"
    "Parameters:\n"
    "  --help                  Display this help message\n"
    "\n"
    "SubCommands:\n"
    "  execute-intent          Execute an insight intent and return the result\n"
    "  get-intent              Query insight intent registration information\n"
    "\n"
    "Examples:\n"
    "  ohos-claw-cc execute-intent --bundleName com.example --moduleName entry --intentName MyIntent\n"
    "  ohos-claw-cc get-intent --executeMode 1 --flag 1\n";

const std::string HELP_MSG_EXECUTE_INTENT =
    "ohos-claw-cc execute-intent - Execute an insight intent synchronously and return the result\n"
    "\n"
    "Usage:\n"
    "  ohos-claw-cc execute-intent [options]\n"
    "\n"
    "Parameters:\n"
    "  --bundleName <name>     Target application bundle name (required)\n"
    "  --moduleName <name>     Module name within the bundle (required)\n"
    "  --intentName <name>     Insight intent name to execute (required)\n"
    "  --abilityName <name>    Ability name (required, pass empty string '' if not needed)\n"
    "  --executeMode <mode>    Execute mode (required, range: 0-3)\n"
    "                          0=UI_ABILITY_FOREGROUND, 1=UI_ABILITY_BACKGROUND,\n"
    "                          2=UI_EXTENSION_ABILITY, 3=SERVICE_EXTENSION_ABILITY\n"
    "  --param <json>          Intent parameters as JSON string (required, pass '{}' if not needed)\n"
    "                          For link-type intents, include \"uri\" field in JSON, e.g.\n"
    "                          '{\"uri\":\"https://example.com/page\",\"key\":\"value\"}'\n"
    "  --help                  Display this help message\n"
    "\n"
    "Examples:\n"
    "  ohos-claw-cc execute-intent --bundleName com.example --moduleName entry --intentName MyIntent\n"
    "\n"
    "  ohos-claw-cc execute-intent --bundleName com.example --moduleName entry\n"
    "          --intentName MyIntent --executeMode 0 --param '{\"key1\":\"value1\"}'\n";

const std::string HELP_MSG_GET_INTENT =
    "ohos-claw-cc get-intent - Query insight intent registration information\n"
    "\n"
    "Usage:\n"
    "  ohos-claw-cc get-intent [options]\n"
    "\n"
    "Parameters:\n"
    "  --executeMode <mode>    Query mode (required, values: 1-3)\n"
    "                          1=query all, 2=query by bundle, 3=query by intent name\n"
    "  --flag <flag>           Query flag (required, values: 1, 2, 5, 6)\n"
    "                          1=GET_FULL_INSIGHT_INTENT, 2=GET_SUMMARY_INSIGHT_INTENT,\n"
    "                          5=FULL+ENTITY_INFO, 6=SUMMARY+ENTITY_INFO\n"
    "  --bundleName <name>     Bundle name (required when executeMode is 2 or 3)\n"
    "  --moduleName <name>     Module name (required when executeMode is 3)\n"
    "  --intentName <name>     Intent name (required when executeMode is 3)\n"
    "  --help                  Display this help message\n"
    "\n"
    "Examples:\n"
    "  ohos-claw-cc get-intent --executeMode 1 --flag 1\n"
    "\n"
    "  ohos-claw-cc get-intent --executeMode 2 --flag 1 --bundleName com.example.bundle\n"
    "\n"
    "  ohos-claw-cc get-intent --executeMode 3 --flag 1\n"
    "          --bundleName com.example.bundle --moduleName entry --intentName MyIntent\n";
}  // namespace

class InsightIntentShellCommand : public ShellCommand {
public:
    InsightIntentShellCommand(int argc, char* argv[]);
    ~InsightIntentShellCommand() override = default;

    ErrCode CreateMessageMap() override;

private:
    ErrCode CreateCommandMap() override;
    ErrCode init() override;

    ErrCode RunAsHelpCommand();
    ErrCode RunAsExecuteIntentCommand();
    ErrCode RunAsGetIntentCommand();

    ErrCode ParseExecuteIntentOptions(std::string &bundleName, std::string &moduleName,
        std::string &abilityName, std::string &insightIntentName,
        std::string &intentParamJson, int32_t &executeMode);
    ErrCode HandleExecuteIntentOption(int option, std::string &bundleName,
        std::string &moduleName, std::string &abilityName,
        std::string &insightIntentName, std::string &intentParamJson,
        int32_t &executeMode);
    ErrCode CheckRequiredExecuteParams(const std::string &bundleName,
        const std::string &moduleName,
        const std::string &insightIntentName);
    ErrCode CheckAllExecuteParams(const std::string &bundleName,
        const std::string &moduleName,
        const std::string &insightIntentName,
        const std::string &abilityName, int32_t executeMode,
        const std::string &intentParamJson);
    ErrCode ValidateGetIntentParams(int32_t flag, int32_t executeMode);
    ErrCode ValidateIntentFromDatabase(const std::string &bundleName,
        const std::string &moduleName,
        const std::string &insightIntentName,
        AbilityRuntime::InsightIntentInfoForQuery &queryInfo);
    int8_t ConvertIntentTypeToDecoratorType(const std::string &intentType);
    void BuildExecuteParam(InsightIntentExecuteParam &param,
        const std::string &bundleName, const std::string &moduleName,
        const std::string &abilityName, const std::string &insightIntentName,
        int32_t executeMode, const std::string &intentType,
        const std::string &intentParamJson);
    ErrCode ExecuteIntentWithParam(const InsightIntentExecuteParam &param);
    void AppendExecuteResult(const InsightIntentExecuteResult &executeResult);

    ErrCode ParseGetIntentOptions(int32_t &flag, int32_t &executeMode,
        std::string &bundleName,
        std::string &moduleName, std::string &intentName);
    ErrCode HandleGetIntentOption(int option, int32_t &flag,
        int32_t &executeMode, std::string &bundleName,
        std::string &moduleName, std::string &intentName);
    ErrCode DispatchGetIntentMode(int32_t executeMode, int32_t flag,
        const std::string &bundleName,
        const std::string &moduleName, const std::string &intentName);
    ErrCode RunGetIntentModeAll(int32_t flag);
    ErrCode RunGetIntentModeByBundle(int32_t flag,
        const std::string &bundleName);
    ErrCode RunGetIntentModeByIntent(int32_t flag,
        const std::string &bundleName, const std::string &moduleName,
        const std::string &intentName);

    ErrCode ParseIntOption(const char *arg, int32_t &value);
    bool ShouldAppendEntityInfo(int32_t flag) const;
    bool IsFullInfo(int32_t flag) const;
    ErrCode ValidateExecuteMode(int32_t executeMode);

    void AppendResultEvent(const nlohmann::json &data);
    void AppendErrorEvent(const std::string &errCode,
        const std::string &errMsg, const std::string &suggestion);
    nlohmann::json BuildIntentInfoListJson(
        const std::vector<AbilityRuntime::InsightIntentInfoForQuery> &infos,
        int32_t flag);
    nlohmann::json BuildIntentInfoJson(
        const AbilityRuntime::InsightIntentInfoForQuery &info, int32_t flag);
    nlohmann::json BuildEntityJson(
        const AbilityRuntime::EntityInfoForQuery &entity);
    nlohmann::json CollectSupportedModesJson(
        const AbilityRuntime::InsightIntentInfoForQuery &queryInfo);
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_CC_COMMAND_H
