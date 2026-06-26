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

#ifndef OHOS_ABILITY_RUNTIME_CLI_EVENT_REPORT_H
#define OHOS_ABILITY_RUNTIME_CLI_EVENT_REPORT_H

#include <string>

namespace OHOS {
namespace AAFwk {
class WantParams;
}

namespace CliTool {

// Event names
constexpr const char* EVENT_CLI_EXECUTE_FAILED = "EXECUTE_CLI_FAILED";

// Event domain
constexpr const char* DOMAIN_CLI_TOOL = "AAFWK";

// HiSysEvent parameter names
constexpr const char* PARAM_TYPE = "TYPE";
constexpr const char* PARAM_BUNDLE_NAME = "BUNDLE_NAME";
constexpr const char* PARAM_CLI_NAME = "CLI_NAME";
constexpr const char* PARAM_REASON = "REASON";
constexpr const char* PARAM_DETAIL = "DETAIL";
constexpr const char* PARAM_DURATION_MS = "DURATION_MS";
constexpr const char* PARAM_SIGNAL_NUM = "SIGNAL_NUM";

// Failure types for TYPE field (INT32)
constexpr int32_t TYPE_FAILED = 1;
constexpr int32_t TYPE_TIMEOUT = 2;
constexpr int32_t TYPE_SIGNAL = 3;

// Failure reasons
constexpr const char* REASON_PERMISSION_DENIED = "PERMISSION_DENIED";
constexpr const char* REASON_TOOL_NOT_FOUND = "TOOL_NOT_FOUND";
constexpr const char* REASON_SESSION_LIMIT_EXCEEDED = "SESSION_LIMIT_EXCEEDED";
constexpr const char* REASON_PROCESS_CREATE_FAILED = "PROCESS_CREATE_FAILED";
constexpr const char* REASON_INVALID_PARAM = "INVALID_PARAM";

// Detail codes for TOOL_NOT_FOUND
constexpr const char* DETAIL_TOOL_NOT_FOUND = "tool_not_found";
constexpr const char* DETAIL_SUBCOMMAND_NOT_FOUND = "subcommand_not_found";

// Detail codes for INVALID_PARAM
constexpr const char* DETAIL_TIMEOUT_NEGATIVE = "timeout_negative";
constexpr const char* DETAIL_YIELD_MS_NEGATIVE = "yieldMs_negative";
constexpr const char* DETAIL_TIMEOUT_EXCEEDS_LIMIT = "timeout_exceeds_limit";
constexpr const char* DETAIL_YIELD_EXCEEDS_TIMEOUT = "yield_exceeds_timeout";
constexpr const char* DETAIL_INPUT_SCHEMA_EMPTY = "input_schema_empty";
constexpr const char* DETAIL_PARAM_NOT_FOUND = "param_not_found";
constexpr const char* DETAIL_PARAM_TYPE_MISMATCH = "param_type_mismatch";

/**
 * @brief Report CLI execution failed event (TYPE=FAILED)
 * @param bundleName Caller bundle name
 * @param cliName CLI tool name
 * @param reason Failure reason
 * @param detail Detailed failure information
 */
void ReportCliExecuteFailed(const std::string& bundleName, const std::string& cliName,
    const std::string& reason, const std::string& detail = "");

/**
 * @brief Report CLI timeout event (TYPE=TIMEOUT)
 * @param bundleName Caller bundle name
 * @param cliName CLI tool name
 * @param durationMs Timeout duration in milliseconds
 */
void ReportCliTimeout(const std::string& bundleName, const std::string& cliName,
    const std::string& durationMs);

/**
 * @brief Report CLI signal event (TYPE=SIGNAL)
 * @param cliName CLI tool name
 * @param signalNum Signal number
 */
void ReportCliSignal(const std::string& cliName, const std::string& signalNum);

/**
 * @brief Get effective CLI name with placeholder handling
 * @param cliName Original CLI name
 * @return Effective CLI name (returns "<empty>" for empty or "undefined")
 */
std::string GetEffectiveCliName(const std::string& cliName);

/**
 * @brief Get failure reason string from error code
 * @param errorCode Error code
 * @return Failure reason string
 */
std::string GetFailureReason(int32_t errorCode);

/**
 * @brief Format WantParams to string for event reporting
 * @param args WantParams to format
 * @return Formatted string
 */
std::string FormatWantParamsToString(const AAFwk::WantParams& args);

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_EVENT_REPORT_H
