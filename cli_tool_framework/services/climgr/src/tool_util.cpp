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

#include "tool_util.h"

#include <algorithm>
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <cstdint>
#include <nlohmann/json.hpp>
#include <random>
#include <set>
#include <sstream>
#include <utility>
#include <vector>

#include "accesstoken_kit.h"
#include "array_wrapper.h"
#include "bundle_info.h"
#include "bundle_mgr_helper.h"
#include "cli_error_code.h"
#include "cli_event_report.h"
#include "double_wrapper.h"
#include "exec_cmd_param.h"
#include "exec_tool_param.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "permission_util.h"
#include "session_record.h"
#include "skill_execute_param.h"
#include "skill_execute_result.h"
#include "string_wrapper.h"
#include "tool_info.h"
#include "want_params.h"
#include "want_params_wrapper.h"
#include "bool_wrapper.h"
#include "int_wrapper.h"

namespace OHOS {
namespace CliTool {
namespace {
constexpr int32_t MILLISECOND_COEFFICIENT = 1000;
constexpr int64_t MAX_TIMEOUT = 30 * 60; // 30 m
constexpr size_t PREFIX_DOUBLE_DASH_LEN = 2;
}
int32_t ToolUtil::ValidateProperties(const ToolInfo &toolInfo, const ExecToolParam &param,
    AccessToken::AccessTokenID tokenId, std::string& detail)
{
    if (!param.subcommand.empty()) {
        if (!toolInfo.hasSubCommand) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "not have subcommand");
            return ERR_TOOL_NOT_EXIST;
        }

        auto search = toolInfo.subcommands.find(param.subcommand);
        if (search == toolInfo.subcommands.end()) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "not have subcommand, %{public}s", param.subcommand.c_str());
            detail = DETAIL_SUBCOMMAND_NOT_FOUND;
            return ERR_TOOL_NOT_EXIST;
        }
    }

    auto res = ValidateExecOptionsProperties(param.options, detail);
    if (res != ERR_OK) {
        return res;
    }

    if (param.subcommand.empty()) {
        return ValidateInputSchemaProperties(toolInfo.inputSchema, param.args, detail);
    }

    auto it = toolInfo.subcommands.find(param.subcommand);
    if (it == toolInfo.subcommands.end()) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "GetSubCommandInfo failed: subcommand=%{public}s", param.subcommand.c_str());
        return ERR_INVALID_PARAM;
    }
    return ValidateInputSchemaProperties(it->second.inputSchema, param.args, detail);
}

int32_t ToolUtil::ValidateExecOptionsProperties(const ExecOptions &options, std::string& detail)
{
    if (options.timeout < 0 || options.yieldMs < 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "yieldMs or timeout < 0");
        detail = (options.timeout < 0) ? DETAIL_TIMEOUT_NEGATIVE : DETAIL_YIELD_MS_NEGATIVE;
        return ERR_INVALID_PARAM;
    }

    if (options.timeout > MAX_TIMEOUT) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Excessively large timeout");
        detail = DETAIL_TIMEOUT_EXCEEDS_LIMIT;
        return ERR_INVALID_PARAM;
    }

    if (!options.background && options.yieldMs > options.timeout * MILLISECOND_COEFFICIENT) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "yieldTime exceeds timeout.");
        detail = DETAIL_YIELD_EXCEEDS_TIMEOUT;
        return ERR_INVALID_PARAM;
    }

    return ERR_OK;
}

int32_t ToolUtil::ValidateInputSchemaProperties(const std::string &inputSchema,
    const AAFwk::WantParams &args, std::string& detail)
{
    if (args.IsEmpty()) {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "args is empty");
        return ERR_OK;
    }

    if (inputSchema.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "inputSchema is empty");
        detail = DETAIL_INPUT_SCHEMA_EMPTY;
        return ERR_INVALID_PARAM;
    }

    nlohmann::json schema = nlohmann::json::parse(inputSchema, nullptr, false);
    if (schema.is_discarded()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "discarded error");
        return ERR_NO_INIT;
    }
    if (!schema.contains("properties") || !schema["properties"].is_object()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "properties not found or invalid");
        return ERR_INVALID_PARAM;
    }
    auto properties = schema["properties"];
    for (auto &[key, value] : args.GetParams()) {
        if (key == "help") {
            if (args.Size() != 1) {
                TAG_LOGE(AAFwkTag::CLI_TOOL, "args size > 1");
                return ERR_INVALID_PARAM;
            }
            return ERR_OK;
        }
        if (!properties.contains(key)) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "args key '%{public}s' not found in properties", key.c_str());
            detail = DETAIL_PARAM_NOT_FOUND;
            return ERR_INVALID_PARAM;
        }

        // Validate type if specified in schema
        auto &propertySchema = properties[key];
        if (propertySchema.contains("type")) {
            if (!propertySchema["type"].is_string()) {
                TAG_LOGE(AAFwkTag::CLI_TOOL, "args key '%{public}s' has invalid schema type", key.c_str());
                return ERR_INVALID_PARAM;
            }
            std::string expectedType = propertySchema["type"].get<std::string>();
            if (!ValidateParamType(value, expectedType, propertySchema, key)) {
                TAG_LOGE(AAFwkTag::CLI_TOOL, "args key '%{public}s' type mismatch, expected: %{public}s",
                    key.c_str(), expectedType.c_str());
                detail = DETAIL_PARAM_TYPE_MISMATCH;
                return ERR_INVALID_PARAM;
            }
        }
    }
    return ERR_OK;
}

std::string ToolUtil::GenerateCliSessionId(const std::string &name, std::shared_ptr<SessionRecord> record)
{
    std::random_device seed;
    std::mt19937 rng(seed());
    std::uniform_int_distribution<int> uni(0, INT_MAX);
    int randomDigit = uni(rng);
    auto timestamp = std::chrono::system_clock::now().time_since_epoch();
    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp).count();
    if (record != nullptr) {
        record->startTime = time;
    }
    return name + "_" + std::to_string(time) + "_" + std::to_string(randomDigit);
}

bool ToolUtil::GenerateSandboxConfig(const ExecToolParam &param, AccessToken::AccessTokenID tokenId,
    std::string &sandboxConfig, std::string &bundleName)
{
    AppExecFwk::BundleInfo bundleInfo;
    if (!ToolUtil::GetBundleInfoByTokenId(tokenId, bundleInfo)) {
        return false;
    }

    nlohmann::json config;
    config["callerTokenId"] = IPCSkeleton::GetCallingFullTokenID();
    config["challenge"] = param.challenge;
    config["uid"] = IPCSkeleton::GetCallingUid();
    config["callerPid"] = IPCSkeleton::GetCallingPid();
    config["gid"] = bundleInfo.gid;
    config["appId"] = bundleInfo.appId;
    config["appIdentifier"] = bundleInfo.signatureInfo.appIdentifier;
    config["bundleName"] = bundleInfo.name;
    config["cliName"] = param.toolName;
    config["subCliName"] = param.subcommand;
    config["type"] = "cli";
    nlohmann::json envConfig;
    // Capture app A's calling-identity string (cliMgr's caller IS app A) so the ForCli agentmgr
    // interface can propagate app A end-to-end. ResetCallingIdentity switches the thread to
    // cliMgr's own identity, so restore app A immediately so the GetCalling* below still see app A.
    std::string callerIdentity = IPCSkeleton::ResetCallingIdentity();
    if (!IPCSkeleton::SetCallingIdentity(callerIdentity)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "restore caller identity failed");
        return false;
    }
    envConfig["ohos_cli_callerIdentity"] = callerIdentity;
    envConfig["ohos_cli_callerBundleName"] = bundleInfo.name;
    envConfig["ohos_cli_callerUid"] = std::to_string(IPCSkeleton::GetCallingUid());
    envConfig["ohos_cli_callerTokenId"] = std::to_string(IPCSkeleton::GetCallingTokenID());
    config["env"] = envConfig;
    config["nsFlags"] = {"pid"};

    sandboxConfig = config.dump();
    bundleName = bundleInfo.name;
    TAG_LOGI(AAFwkTag::CLI_TOOL, "bundleName:%{public}s, gid:%{public}d, cliName:%{public}s, subCliName:%{public}s",
        bundleInfo.name.c_str(), bundleInfo.gid, param.toolName.c_str(), param.subcommand.c_str());
    return true;
}

bool ToolUtil::GenerateCmdSandboxConfig(const ExecCmdParam &param, AccessToken::AccessTokenID tokenId,
    std::string &sandboxConfig, std::string &bundleName)
{
    AppExecFwk::BundleInfo bundleInfo;
    if (!ToolUtil::GetBundleInfoByTokenId(tokenId, bundleInfo)) {
        return false;
    }

    nlohmann::json config;
    config["callerTokenId"] = IPCSkeleton::GetCallingFullTokenID();
    config["uid"] = IPCSkeleton::GetCallingUid();
    config["callerPid"] = IPCSkeleton::GetCallingPid();
    config["gid"] = bundleInfo.gid;
    config["appIdentifier"] = bundleInfo.signatureInfo.appIdentifier;
    config["bundleName"] = bundleInfo.name;
    config["type"] = "shell";
    config["policy"] = param.policy;
    if (param.workDir != "") {
        config["workdir"] = param.workDir;
    }
    config["env"] = param.env;
    config["nsFlags"] = {"pid"};
    sandboxConfig = config.dump();
    bundleName = bundleInfo.name;
    TAG_LOGI(AAFwkTag::CLI_TOOL, "bundleName:%{public}s, gid:%{public}d", bundleInfo.name.c_str(), bundleInfo.gid);
    return true;
}

bool ToolUtil::GetBundleInfoByTokenId(AccessToken::AccessTokenID tokenId, AppExecFwk::BundleInfo &bundleInfo)
{
    auto tokenType = AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenType != AccessToken::ATokenTypeEnum::TOKEN_HAP) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "caller is not hap");
        return false;
    }
    AccessToken::HapTokenInfo hapInfo;
    auto ret = AccessToken::AccessTokenKit::GetHapTokenInfo(tokenId, hapInfo);
    if (ret != AccessToken::AccessTokenKitRet::RET_SUCCESS) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetHapTokenInfo failed, ret:%{public}d", ret);
        return false;
    }

    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "bundlerMgrHelper is invalid");
        return false;
    }
    auto flag = static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION);
    if (hapInfo.instIndex == 0) {
        if (bundleMgrHelper->GetBundleInfoV9(hapInfo.bundleName, flag, bundleInfo, hapInfo.userID) != ERR_OK) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Fail to get bundle info");
            return false;
        }
        return true;
    }
    if (bundleMgrHelper->GetCloneBundleInfo(hapInfo.bundleName, flag, hapInfo.instIndex, bundleInfo, hapInfo.userID) !=
        ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Fail to get bundle info");
        return false;
    }
    return true;
}

void ToolUtil::TransferToCmdParam(const AAFwk::WantParams &args, std::vector<std::string> &execArgs)
{
    if (args.IsEmpty()) {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "Not has arg");
        return;
    }
    for (const auto &[key, value] : args.GetParams()) {
        if (value == nullptr) {
            continue;
        }

        if (key == "help") {
            ProcessBooleanParam(key, value, execArgs);
            continue;
        }

        if (IsBooleanType(value)) {
            ProcessBooleanParam(key, value, execArgs);
            continue;
        }

        if (IsArrayType(value)) {
            ProcessArrayExpansion(key, value, execArgs);
            continue;
        }

        std::string strValue = GetParamStringValue(value);
        if (!strValue.empty()) {
            execArgs.push_back("--" + key);
            execArgs.push_back(strValue);
        }
    }
}

// ============================================================================
// ParseToolCommand: parse a concatenated tool command string (tool command mode)
// Supported syntax (long-flag only):
//   <toolName> [subcommand] [--key=value | --key value | --flag | --flag false]...
// Single-quoted values are kept as one token and the quotes are stripped.
// Repeated flags of the same key are collected into an array.
// ============================================================================

namespace {
constexpr char FLAG_PREFIX[] = "--";
constexpr size_t FLAG_PREFIX_LEN = 2;
constexpr char HELP_KEY[] = "help";
// Argument start index: tokens[0] is always toolName.
constexpr size_t ARG_START_TOOLNAME_ONLY = 1;       // no subcommand
constexpr size_t ARG_START_WITH_SUBCOMMAND = 2;     // tokens[1] is subcommand
// Tokens consumed by "--key value": the flag token plus its value token at i + 1.
// "--key=value", boolean "--key" and "--help" consume only the flag token.
constexpr size_t FLAG_WITH_VALUE_TOKEN_COUNT = 2;

struct ParsedCommandArgs {
    std::map<std::string, std::vector<std::string>> values;
};

struct SubCommandResult {
    std::string inputSchema;
    size_t argStart = ARG_START_TOOLNAME_ONLY;
    std::string subcommand;
};

// Only single-quote is supported. Double quotes and backslash escapes are not.
bool TokenizeCommand(const std::string &cmd, std::vector<std::string> &tokens)
{
    std::string current;
    bool inQuote = false;
    for (char c : cmd) {
        if (c == '\'') {
            inQuote = !inQuote;
            continue;
        }
        if ((c == ' ' || c == '\t') && !inQuote) {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
            continue;
        }
        current.push_back(c);
    }
    if (inQuote) {
        return false;
    }
    if (!current.empty()) {
        tokens.push_back(current);
    }
    return true;
}

bool IsFlagToken(const std::string &token)
{
    return token.size() >= FLAG_PREFIX_LEN && token.compare(0, FLAG_PREFIX_LEN, FLAG_PREFIX) == 0;
}

bool IsBooleanSchema(const nlohmann::json &properties, const std::string &key)
{
    auto it = properties.find(key);
    if (it == properties.end() || !it->contains("type") || !it->at("type").is_string()) {
        return false;
    }
    return it->at("type").get<std::string>() == "boolean";
}

int32_t ParseArgTokens(const std::vector<std::string> &tokens, size_t start,
    const nlohmann::json &properties, ParsedCommandArgs &parsed, std::string &detail)
{
    size_t i = start;
    while (i < tokens.size()) {
        const std::string &token = tokens[i];
        if (!IsFlagToken(token)) {
            // Bare token without "--" prefix (short flags / custom prefix) is not supported.
            detail = DETAIL_PARAM_NOT_FOUND;
            return ERR_INVALID_PARAM;
        }
        std::string flagText = token.substr(FLAG_PREFIX_LEN);
        auto eqPos = flagText.find('=');
        std::string key = (eqPos == std::string::npos) ? flagText : flagText.substr(0, eqPos);
        if (key.empty()) {
            detail = DETAIL_PARAM_NOT_FOUND;
            return ERR_INVALID_PARAM;
        }
        // "help" is always allowed, matching ValidateInputSchemaProperties.
        if (key == HELP_KEY) {
            parsed.values[key].push_back("true");
            ++i;
            continue;
        }
        if (!properties.contains(key)) {
            detail = DETAIL_PARAM_NOT_FOUND;
            return ERR_INVALID_PARAM;
        }
        // --key=value
        if (eqPos != std::string::npos) {
            parsed.values[key].push_back(flagText.substr(eqPos + 1));
            ++i;
            continue;
        }
        // --key with no inline value: decide whether it takes a value or is a boolean flag.
        bool isBool = IsBooleanSchema(properties, key);
        if (i + 1 >= tokens.size() || IsFlagToken(tokens[i + 1])) {
            if (!isBool) {
                detail = DETAIL_PARAM_TYPE_MISMATCH;
                return ERR_INVALID_PARAM;
            }
            parsed.values[key].push_back("true");
            ++i;
            continue;
        }
        // --key value: the value is taken from the next token, so an explicit "false"
        // needs no special case here (ConvertRawValue turns it into Boolean(false)).
        parsed.values[key].push_back(tokens[i + 1]);
        i += FLAG_WITH_VALUE_TOKEN_COUNT;
    }
    return ERR_OK;
}

bool ConvertRawValue(const std::string &raw, const std::string &type, sptr<AAFwk::IInterface> &value)
{
    if (type == "boolean") {
        if (raw == "true") {
            value = AAFwk::Boolean::Box(true);
            return true;
        }
        if (raw == "false") {
            value = AAFwk::Boolean::Box(false);
            return true;
        }
        return false;
    }
    if (type == "integer") {
        errno = 0;
        char *end = nullptr;
        long long intVal = std::strtoll(raw.c_str(), &end, 10);
        if (errno != 0 || end == raw.c_str() || *end != '\0' ||
            intVal < INT32_MIN || intVal > INT32_MAX) {
            return false;
        }
        value = AAFwk::Integer::Box(static_cast<int32_t>(intVal));
        return true;
    }
    if (type == "number") {
        errno = 0;
        char *end = nullptr;
        double dblVal = std::strtod(raw.c_str(), &end);
        if (errno != 0 || end == raw.c_str() || *end != '\0') {
            return false;
        }
        value = AAFwk::Double::Box(dblVal);
        return true;
    }
    // string and other unknown types are kept as string (compatible with
    // ValidateBasicType which allows unknown types).
    value = AAFwk::String::Box(raw);
    return true;
}

AAFwk::InterfaceID GetArrayTypeId(const std::string &type)
{
    if (type == "boolean") {
        return AAFwk::g_IID_IBoolean;
    }
    if (type == "integer") {
        return AAFwk::g_IID_IInteger;
    }
    if (type == "number") {
        return AAFwk::g_IID_IDouble;
    }
    return AAFwk::g_IID_IString;
}

int32_t ConvertArrayArg(const nlohmann::json &prop, const std::vector<std::string> &values,
    AAFwk::WantParams &args, const std::string &key, std::string &detail)
{
    std::string itemType = "string";
    if (prop.contains("items") && prop["items"].is_object() &&
        prop["items"].contains("type") && prop["items"]["type"].is_string()) {
        itemType = prop["items"]["type"].get<std::string>();
    }
    sptr<AAFwk::IArray> array = new (std::nothrow) AAFwk::Array(
        static_cast<long>(values.size()), GetArrayTypeId(itemType));
    if (array == nullptr) {
        detail = DETAIL_PARAM_TYPE_MISMATCH;
        return ERR_INVALID_PARAM;
    }
    for (size_t i = 0; i < values.size(); ++i) {
        sptr<AAFwk::IInterface> element;
        if (!ConvertRawValue(values[i], itemType, element)) {
            detail = DETAIL_PARAM_TYPE_MISMATCH;
            return ERR_INVALID_PARAM;
        }
        array->Set(static_cast<long>(i), element);
    }
    args.SetParam(key, array);
    return ERR_OK;
}

int32_t ConvertParsedArgs(const ParsedCommandArgs &parsed, const nlohmann::json &properties,
    AAFwk::WantParams &args, std::string &detail)
{
    for (const auto &[key, values] : parsed.values) {
        if (key == HELP_KEY) {
            args.SetParam(key, AAFwk::Boolean::Box(true));
            continue;
        }
        auto propIt = properties.find(key);
        if (propIt == properties.end()) {
            continue;
        }
        const nlohmann::json &prop = propIt.value();
        std::string type = "string";
        if (prop.contains("type") && prop["type"].is_string()) {
            type = prop["type"].get<std::string>();
        }
        if (type == "array") {
            if (ConvertArrayArg(prop, values, args, key, detail) != ERR_OK) {
                return ERR_INVALID_PARAM;
            }
            continue;
        }
        // Non-array parameter must not be repeated.
        if (values.size() > 1) {
            detail = DETAIL_PARAM_TYPE_MISMATCH;
            return ERR_INVALID_PARAM;
        }
        sptr<AAFwk::IInterface> value;
        if (!ConvertRawValue(values[0], type, value)) {
            detail = DETAIL_PARAM_TYPE_MISMATCH;
            return ERR_INVALID_PARAM;
        }
        args.SetParam(key, value);
    }
    return ERR_OK;
}

int32_t ResolveSubCommand(const std::vector<std::string> &tokens, const ToolInfo &toolInfo,
    SubCommandResult &result, std::string &detail)
{
    result.inputSchema = toolInfo.inputSchema;
    result.argStart = ARG_START_TOOLNAME_ONLY;
    if (tokens.size() <= 1 || IsFlagToken(tokens[1])) {
        return ERR_OK;
    }
    if (!toolInfo.hasSubCommand) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "tool has no subcommand");
        detail = DETAIL_SUBCOMMAND_NOT_FOUND;
        return ERR_TOOL_NOT_EXIST;
    }
    auto it = toolInfo.subcommands.find(tokens[1]);
    if (it == toolInfo.subcommands.end()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "subcommand not found: %{public}s", tokens[1].c_str());
        detail = DETAIL_SUBCOMMAND_NOT_FOUND;
        return ERR_TOOL_NOT_EXIST;
    }
    result.subcommand = tokens[1];
    result.inputSchema = it->second.inputSchema;
    result.argStart = ARG_START_WITH_SUBCOMMAND;
    return ERR_OK;
}
} // namespace

int32_t ToolUtil::ParseToolCommand(const std::string &cmd, const ToolInfo &toolInfo,
    ExecToolParam &param, std::string &detail)
{
    // Stage 0: tokenize.
    std::vector<std::string> tokens;
    if (!TokenizeCommand(cmd, tokens) || tokens.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "invalid tool command string");
        detail = DETAIL_PARAM_NOT_FOUND;
        return ERR_INVALID_PARAM;
    }

    // Stage 1: validate toolName.
    const std::string &toolName = tokens[0];
    if (toolName.empty() || toolName[0] == '/') {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "invalid toolName");
        detail = DETAIL_PARAM_NOT_FOUND;
        return ERR_INVALID_PARAM;
    }
    if (!toolInfo.name.empty() && toolInfo.name != toolName) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "tool not found: %{public}s", toolName.c_str());
        detail = DETAIL_TOOL_NOT_FOUND;
        return ERR_TOOL_NOT_EXIST;
    }
    param.toolName = toolName;

    // Stage 2: resolve subcommand.
    SubCommandResult subResult;
    if (auto ret = ResolveSubCommand(tokens, toolInfo, subResult, detail); ret != ERR_OK) {
        return ret;
    }
    param.subcommand = std::move(subResult.subcommand);
    const std::string &inputSchema = subResult.inputSchema;
    size_t argStart = subResult.argStart;

    // Load properties from schema.
    nlohmann::json properties = nlohmann::json::object();
    if (!inputSchema.empty()) {
        nlohmann::json schema = nlohmann::json::parse(inputSchema, nullptr, false);
        if (!schema.is_discarded() && schema.contains("properties") && schema["properties"].is_object()) {
            properties = schema["properties"];
        }
    }

    // Stage 3: parse arguments.
    ParsedCommandArgs parsed;
    auto res = ParseArgTokens(tokens, argStart, properties, parsed, detail);
    if (res != ERR_OK) {
        return res;
    }

    // Stage 4: type recovery.
    return ConvertParsedArgs(parsed, properties, param.args, detail);
}

void ToolUtil::ProcessBooleanParam(const std::string &key, const sptr<AAFwk::IInterface> &value,
    std::vector<std::string> &execArgs)
{
    if (!IsBooleanType(value)) {
        return;
    }

    bool boolValue = false;
    if (GetParamBoolValue(value, boolValue) && boolValue) {
        execArgs.push_back("--" + key);
    }
}

std::string ToolUtil::GetParamStringValue(const sptr<AAFwk::IInterface> &value)
{
    if (value == nullptr) {
        return "";
    }

    // Try String
    if (auto strObj = AAFwk::IString::Query(value)) {
        std::string strValue;
        if (strObj->GetString(strValue) == ERR_OK) {
            return strValue;
        }
        return "";
    }

    // Try Boolean
    if (auto boolObj = AAFwk::IBoolean::Query(value)) {
        bool boolValue = false;
        if (boolObj->GetValue(boolValue) == ERR_OK) {
            return boolValue ? "true" : "false";
        }
        return "";
    }

    // Try Integer/Int
    if (auto intObj = AAFwk::IInteger::Query(value)) {
        int intValue = 0;
        if (intObj->GetValue(intValue) == ERR_OK) {
            return std::to_string(intValue);
        }
        return "";
    }

    // Try Long
    if (auto longObj = AAFwk::ILong::Query(value)) {
        long longValue = 0;
        if (longObj->GetValue(longValue) == ERR_OK) {
            return std::to_string(longValue);
        }
        return "";
    }

    // Try Float
    if (auto floatObj = AAFwk::IFloat::Query(value)) {
        float floatValue = 0.0f;
        if (floatObj->GetValue(floatValue) == ERR_OK) {
            return std::to_string(floatValue);
        }
        return "";
    }

    // Try Double
    if (auto doubleObj = AAFwk::IDouble::Query(value)) {
        double doubleValue = 0.0;
        if (doubleObj->GetValue(doubleValue) == ERR_OK) {
            return std::to_string(doubleValue);
        }
        return "";
    }

    // If no type matches, return empty string
    return "";
}

bool ToolUtil::GetParamBoolValue(const sptr<AAFwk::IInterface> &value, bool &result)
{
    if (value == nullptr) {
        return false;
    }

    // Try Boolean type directly
    if (auto boolObj = AAFwk::IBoolean::Query(value)) {
        if (boolObj->GetValue(result) == ERR_OK) {
            return true;
        }
        return false;
    }

    return false;
}

void ToolUtil::ProcessArrayExpansion(const std::string &key, const sptr<AAFwk::IInterface> &value,
    std::vector<std::string> &execArgs)
{
    auto arrayValue = AAFwk::IArray::Query(value);
    if (arrayValue == nullptr) {
        return;
    }

    long arrayLength = 0;
    if (arrayValue->GetLength(arrayLength) != ERR_OK || arrayLength <= 0) {
        return;
    }

    // Iterate through array elements and expand to command line
    std::string tmpKey = "--" + key;
    for (long i = 0; i < arrayLength; ++i) {
        sptr<AAFwk::IInterface> elementValue;
        if (arrayValue->Get(i, elementValue) != ERR_OK || elementValue == nullptr) {
            continue;
        }

        // Skip nested arrays
        if (AAFwk::IArray::Query(elementValue) != nullptr) {
            continue;
        }

        std::string elementStr = GetParamStringValue(elementValue);
        if (!elementStr.empty()) {
            execArgs.push_back(tmpKey);
            execArgs.push_back(elementStr);
        }
    }
}

// ============================================================================
// Type validation helpers for ValidateInputSchemaProperties
// ============================================================================

bool ToolUtil::ValidateParamType(const sptr<AAFwk::IInterface> &value, const std::string &expectedType,
    const nlohmann::json &propertySchema, const std::string &key)
{
    if (value == nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "value is null for key '%{public}s'", key.c_str());
        return false;
    }
    if (expectedType == "array") {
        return ValidateArrayType(value, propertySchema, key);
    }
    return ValidateBasicType(value, expectedType);
}

bool ToolUtil::ValidateBasicType(const sptr<AAFwk::IInterface> &value, const std::string &expectedType)
{
    if (expectedType == "string") {
        return IsStringType(value);
    }
    if (expectedType == "boolean") {
        return IsBooleanType(value);
    }
    if (expectedType == "integer") {
        return IsIntegerType(value);
    }
    if (expectedType == "number") {
        return IsNumberType(value);
    }
    TAG_LOGW(AAFwkTag::CLI_TOOL, "Unknown type '%{public}s', allowing for compatibility", expectedType.c_str());
    return true;
}

bool ToolUtil::ValidateArrayType(const sptr<AAFwk::IInterface> &value,
    const nlohmann::json &propertySchema, const std::string &key)
{
    if (!IsArrayType(value)) {
        return false;
    }
    if (!propertySchema.contains("items") || !propertySchema["items"].is_object()) {
        return true;
    }
    auto arrayObj = AAFwk::IArray::Query(value);
    if (arrayObj == nullptr) {
        return false;
    }
    return ValidateArrayItems(arrayObj, propertySchema["items"], key);
}

bool ToolUtil::ValidateArrayItems(sptr<AAFwk::IArray> arrayObj,
    const nlohmann::json &itemsSchema, const std::string &key)
{
    long arrayLength = 0;
    if (arrayObj->GetLength(arrayLength) != ERR_OK || arrayLength == 0) {
        return true;
    }
    if (!itemsSchema.contains("type")) {
        return true;
    }
    if (!itemsSchema["type"].is_string()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Array '%{public}s' has invalid item schema type", key.c_str());
        return false;
    }
    std::string itemType = itemsSchema["type"].get<std::string>();
    if (itemType.empty()) {
        return true;
    }
    for (long i = 0; i < arrayLength; ++i) {
        sptr<AAFwk::IInterface> elementValue;
        if (arrayObj->Get(i, elementValue) != ERR_OK || elementValue == nullptr) {
            continue;
        }
        std::string elementKey = key + "[" + std::to_string(i) + "]";
        if (!ValidateParamType(elementValue, itemType, itemsSchema, elementKey)) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Array '%{public}s' element %{public}ld type mismatch, expected: %{public}s",
                key.c_str(), i, itemType.c_str());
            return false;
        }
    }
    return true;
}

bool ToolUtil::IsStringType(const sptr<AAFwk::IInterface> &value)
{
    return AAFwk::IString::Query(value) != nullptr;
}

bool ToolUtil::IsBooleanType(const sptr<AAFwk::IInterface> &value)
{
    return AAFwk::IBoolean::Query(value) != nullptr;
}

bool ToolUtil::IsIntegerType(const sptr<AAFwk::IInterface> &value)
{
    return AAFwk::IInteger::Query(value) != nullptr || AAFwk::ILong::Query(value) != nullptr;
}

bool ToolUtil::IsNumberType(const sptr<AAFwk::IInterface> &value)
{
    return AAFwk::IInteger::Query(value) != nullptr ||
           AAFwk::ILong::Query(value) != nullptr ||
           AAFwk::IFloat::Query(value) != nullptr ||
           AAFwk::IDouble::Query(value) != nullptr;
}

bool ToolUtil::IsArrayType(const sptr<AAFwk::IInterface> &value)
{
    return AAFwk::IArray::Query(value) != nullptr;
}

bool ToolUtil::IsSkillTool(const std::string &toolName)
{
    return toolName == "ohos-arkTSScript";
}

void ToolUtil::NormalizeSkillParamKeys(AAFwk::WantParams &args)
{
    auto &params = args.GetParams();
    std::vector<std::pair<std::string, sptr<AAFwk::IInterface>>> toRename;
    for (const auto &[key, value] : params) {
        std::string bareKey = key;
        if (bareKey.size() > PREFIX_DOUBLE_DASH_LEN && bareKey.compare(0, PREFIX_DOUBLE_DASH_LEN, "--") == 0) {
            bareKey.erase(0, PREFIX_DOUBLE_DASH_LEN);
        } else if (bareKey.size() > 1 && bareKey[0] == '-') {
            bareKey.erase(0, 1);
        } else {
            continue;
        }
        if (params.find(bareKey) == params.end()) {
            toRename.emplace_back(key, value);
        }
    }
    for (auto &[prefixedKey, value] : toRename) {
        std::string bareKey = prefixedKey;
        if (bareKey.compare(0, PREFIX_DOUBLE_DASH_LEN, "--") == 0) {
            bareKey.erase(0, PREFIX_DOUBLE_DASH_LEN);
        } else {
            bareKey.erase(0, 1);
        }
        args.Remove(prefixedKey);
        args.SetParam(bareKey, value);
    }
}

bool ToolUtil::ExpandArgsFromJson(AAFwk::WantParams &args, const std::string &argsStr)
{
    static const std::set<std::string> RESERVED_KEYS = {
        "bundleName", "moduleName", "skillName", "scriptPath", "functionName"
    };

    auto jsonObj = nlohmann::json::parse(argsStr, nullptr, false);
    if (!jsonObj.is_object() || jsonObj.is_discarded()) {
        return false;
    }
    args.Remove("args");
    for (auto &[key, val] : jsonObj.items()) {
        if (RESERVED_KEYS.count(key) > 0) {
            continue;
        }
        if (val.is_string()) {
            args.SetParam(key, AAFwk::String::Box(val.get<std::string>()));
        } else if (val.is_number_integer()) {
            int64_t intVal = val.get<int64_t>();
            if (intVal >= INT32_MIN && intVal <= INT32_MAX) {
                args.SetParam(key, AAFwk::Integer::Box(static_cast<int32_t>(intVal)));
            }
        } else if (val.is_boolean()) {
            args.SetParam(key, AAFwk::Boolean::Box(val.get<bool>()));
        }
    }
    return true;
}

void ToolUtil::ExpandArgsFromWantParams(AAFwk::WantParams &args)
{
    static const std::set<std::string> RESERVED_KEYS = {
        "bundleName", "moduleName", "skillName", "scriptPath", "functionName"
    };

    auto &params = args.GetParams();
    auto it = params.find("args");
    if (it == params.end()) {
        return;
    }
    auto *wantParams = AAFwk::IWantParams::Query(it->second);
    if (wantParams == nullptr) {
        return;
    }
    AAFwk::WantParams nestedParams;
    if (wantParams->GetValue(nestedParams) != ERR_OK) {
        return;
    }
    args.Remove("args");
    for (auto &[key, value] : nestedParams.GetParams()) {
        if (RESERVED_KEYS.count(key) > 0) {
            continue;
        }
        args.SetParam(key, value);
    }
}

void ToolUtil::ExpandArgsJsonString(AAFwk::WantParams &args)
{
    auto argsStr = args.GetStringParam("args");
    if (!argsStr.empty() && ExpandArgsFromJson(args, argsStr)) {
        return;
    }
    ExpandArgsFromWantParams(args);
}

std::shared_ptr<AAFwk::WantParams> ToolUtil::FilterSkillArgs(const AAFwk::WantParams &args)
{
    static const std::set<std::string> RESERVED_KEYS = {
        "bundleName", "moduleName", "skillName", "scriptPath", "functionName"
    };

    auto skillArgs = std::make_shared<AAFwk::WantParams>();
    auto &params = args.GetParams();
    for (auto &[key, value] : params) {
        if (RESERVED_KEYS.count(key) == 0) {
            skillArgs->SetParam(key, value);
        }
    }
    return skillArgs;
}

namespace {
std::string ExtractSkillErrorMsg(const AAFwk::WantParams &params)
{
    const auto &all = params.GetParams();
    auto it = all.find(AppExecFwk::SKILL_ERROR_MSG_KEY);
    if (it == all.end() || it->second == nullptr) {
        return "";
    }
    auto *iStr = AAFwk::IString::Query(it->second);
    if (iStr == nullptr) {
        return "";
    }
    return AAFwk::String::Unbox(iStr);
}
} // namespace

CliSessionInfo ToolUtil::BuildSkillSessionInfo(const std::string &sessionId,
    int32_t resultCode, const AppExecFwk::SkillExecuteResult &skillResult)
{
    CliSessionInfo session;
    session.sessionId = sessionId;
    session.toolName = "ohos-arkTSScript";
    session.status = (resultCode == ERR_OK) ? "completed" : "failed";
    session.result = std::make_shared<ExecResult>();
    int32_t exitCode = skillResult.code;
    if (exitCode == 0 && resultCode != ERR_OK) {
        exitCode = resultCode;
    }
    session.result->exitCode = exitCode;
    if (skillResult.result != nullptr) {
        session.result->outputText = skillResult.result->ToString();
        if (resultCode != ERR_OK) {
            session.result->errorText = ExtractSkillErrorMsg(*(skillResult.result));
        }
    } else if (resultCode != ERR_OK) {
        session.result->errorText = "skill execute failed, code=" + std::to_string(resultCode);
    }
    return session;
}

} // namespace CliTool
} // namespace OHOS
