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

#include "cli_event_report.h"

#include "bool_wrapper.h"
#include "cli_error_code.h"
#include "hilog_tag_wrapper.h"
#include "hisysevent_report.h"
#include "int_wrapper.h"
#include "string_wrapper.h"
#include "want_params.h"

namespace OHOS {
namespace CliTool {

std::string GetEffectiveCliName(const std::string& cliName)
{
    if (cliName.empty() || cliName == "undefined") {
        return "<empty>";
    }
    return cliName;
}

void ReportCliExecuteFailed(const std::string& bundleName, const std::string& cliName,
    const std::string& reason, const std::string& detail)
{
    constexpr int32_t PARAM_COUNT = 5;
    AAFwk::HisyseventReport report(PARAM_COUNT);

    std::string effectiveCliName = GetEffectiveCliName(cliName);

    report.InsertParam(PARAM_TYPE, TYPE_FAILED);
    report.InsertParam(PARAM_BUNDLE_NAME, bundleName);
    report.InsertParam(PARAM_CLI_NAME, effectiveCliName);
    report.InsertParam(PARAM_REASON, reason);
    report.InsertParam(PARAM_DETAIL, detail);

    int32_t ret = report.Report(DOMAIN_CLI_TOOL, EVENT_CLI_EXECUTE_FAILED, HISYSEVENT_FAULT);
    TAG_LOGD(AAFwkTag::CLI_TOOL, "Report CLI_EXECUTE_FAILED: type=%{public}d, bundle=%{public}s, cli=%{public}s, "
        "reason=%{public}s, detail=%{public}s, ret=%{public}d",
        TYPE_FAILED, bundleName.c_str(), effectiveCliName.c_str(), reason.c_str(), detail.c_str(), ret);
}

void ReportCliTimeout(const std::string& bundleName, const std::string& cliName,
    const std::string& durationMs)
{
    constexpr int32_t PARAM_COUNT = 4;
    AAFwk::HisyseventReport report(PARAM_COUNT);

    std::string effectiveCliName = GetEffectiveCliName(cliName);

    report.InsertParam(PARAM_TYPE, TYPE_TIMEOUT);
    report.InsertParam(PARAM_BUNDLE_NAME, bundleName);
    report.InsertParam(PARAM_CLI_NAME, effectiveCliName);
    report.InsertParam(PARAM_DURATION_MS, static_cast<int64_t>(std::stoll(durationMs)));

    int32_t ret = report.Report(DOMAIN_CLI_TOOL, EVENT_CLI_EXECUTE_FAILED, HISYSEVENT_FAULT);
    TAG_LOGD(AAFwkTag::CLI_TOOL, "Report CLI_TIMEOUT: type=%{public}d, bundle=%{public}s, cli=%{public}s, "
        "duration=%{public}s, ret=%{public}d",
        TYPE_TIMEOUT, bundleName.c_str(), effectiveCliName.c_str(), durationMs.c_str(), ret);
}

void ReportCliSignal(const std::string& cliName, const std::string& signalNum)
{
    constexpr int32_t PARAM_COUNT = 3;
    AAFwk::HisyseventReport report(PARAM_COUNT);

    std::string effectiveCliName = GetEffectiveCliName(cliName);

    report.InsertParam(PARAM_TYPE, TYPE_SIGNAL);
    report.InsertParam(PARAM_CLI_NAME, effectiveCliName);
    report.InsertParam(PARAM_SIGNAL_NUM, static_cast<int32_t>(std::stoi(signalNum)));

    int32_t ret = report.Report(DOMAIN_CLI_TOOL, EVENT_CLI_EXECUTE_FAILED, HISYSEVENT_FAULT);
    TAG_LOGD(AAFwkTag::CLI_TOOL,
        "Report CLI_SIGNAL: type=%{public}d, cli=%{public}s, signal=%{public}s, ret=%{public}d",
        TYPE_SIGNAL, effectiveCliName.c_str(), signalNum.c_str(), ret);
}

std::string GetFailureReason(int32_t errorCode)
{
    switch (errorCode) {
        case ERR_PERMISSION_DENIED:
            return REASON_PERMISSION_DENIED;
        case ERR_TOOL_NOT_EXIST:
            return REASON_TOOL_NOT_FOUND;
        case ERR_SESSION_LIMIT_EXCEEDED:
            return REASON_SESSION_LIMIT_EXCEEDED;
        case ERR_NO_INIT:
            return REASON_PROCESS_CREATE_FAILED;
        case ERR_INVALID_PARAM:
        case ERR_INNER_PARAM_INVALID:
            return REASON_INVALID_PARAM;
        default:
            return REASON_INVALID_PARAM;
    }
}

std::string FormatWantParamsToString(const AAFwk::WantParams& args)
{
    std::string result;
    for (const auto& [key, value] : args.GetParams()) {
        if (value == nullptr) {
            continue;
        }
        result += key + "=";

        // Try String
        if (auto strObj = AAFwk::IString::Query(value)) {
            std::string strValue;
            if (strObj->GetString(strValue) == ERR_OK) {
                result += strValue + " ";
                continue;
            }
        }

        // Try Boolean
        if (auto boolObj = AAFwk::IBoolean::Query(value)) {
            bool boolValue = false;
            if (boolObj->GetValue(boolValue) == ERR_OK) {
                result += (boolValue ? std::string("true") : std::string("false")) + " ";
                continue;
            }
        }

        // Try Integer
        if (auto intObj = AAFwk::IInteger::Query(value)) {
            int intValue = 0;
            if (intObj->GetValue(intValue) == ERR_OK) {
                result += std::to_string(intValue) + " ";
                continue;
            }
        }

        // Try Long
        if (auto longObj = AAFwk::ILong::Query(value)) {
            long longValue = 0;
            if (longObj->GetValue(longValue) == ERR_OK) {
                result += std::to_string(longValue) + " ";
                continue;
            }
        }

        // Try Double
        if (auto doubleObj = AAFwk::IDouble::Query(value)) {
            double doubleValue = 0.0;
            if (doubleObj->GetValue(doubleValue) == ERR_OK) {
                result += std::to_string(doubleValue) + " ";
                continue;
            }
        }

        // Default: empty string
        result += " ";
    }
    return result;
}

} // namespace CliTool
} // namespace OHOS
