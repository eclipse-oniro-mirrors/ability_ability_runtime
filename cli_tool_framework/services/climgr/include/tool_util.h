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

#ifndef OHOS_ABILITY_RUNTIME_TOOL_UTIL_H
#define OHOS_ABILITY_RUNTIME_TOOL_UTIL_H

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include <access_token.h>
#include <iremote_object.h>
#include <functional>

namespace OHOS {
namespace AAFwk {
class WantParams;
struct IInterface;
struct IArray;
}
namespace AppExecFwk {
struct BundleInfo;
}
namespace CliTool {
class ExecToolParam;
class SessionRecord;
class ToolInfo;

using namespace OHOS::Security;

class ToolUtil {
public:
    static int32_t ValidateProperties(const ToolInfo &toolInfo, ExecToolParam &param,
        AccessToken::AccessTokenID tokenId);

    static std::string GenerateCliSessionId(const std::string &name, std::shared_ptr<SessionRecord> record);

    static bool GenerateSandboxConfig(const ExecToolParam &param, AccessToken::AccessTokenID tokenId,
        std::string &sandboxConfig, std::string &bundleName);

    static void TransferToCmdParam(const ToolInfo &toolInfo, const AAFwk::WantParams &args, std::string &cmdLine);

private:
    static bool GetBundleInfoByTokenId(AccessToken::AccessTokenID tokenId,
        AppExecFwk::BundleInfo &bundleInfo);
    static int32_t ValidateInputSchemaProperties(const std::string &inputSchema, const AAFwk::WantParams &args);

    // Helper methods for type validation
    static bool ValidateParamType(const sptr<AAFwk::IInterface> &value, const std::string &expectedType,
        const nlohmann::json &propertySchema, const std::string &key = "");
    static bool ValidateArrayType(const sptr<AAFwk::IInterface> &value,
        const nlohmann::json &propertySchema, const std::string &key);
    static bool ValidateArrayItems(sptr<AAFwk::IArray> arrayObj,
        const nlohmann::json &itemsSchema, const std::string &key);
    static bool ValidateBasicType(const sptr<AAFwk::IInterface> &value, const std::string &expectedType);
    static bool IsStringType(const sptr<AAFwk::IInterface> &value);
    static bool IsBooleanType(const sptr<AAFwk::IInterface> &value);
    static bool IsIntegerType(const sptr<AAFwk::IInterface> &value);
    static bool IsNumberType(const sptr<AAFwk::IInterface> &value);
    static bool IsArrayType(const sptr<AAFwk::IInterface> &value);

    // Helper methods for mode processing (extracted to reduce nesting depth)
    static void ProcessBooleanParam(const std::string &key, const sptr<AAFwk::IInterface> &value,
        std::string &cmdLine);
    static void ProcessArrayExpansion(const std::string &key, const sptr<AAFwk::IInterface> &value,
        std::string &cmdLine);

    // Type conversion helpers
    // GetParamStringValue: only supports basic types (bool, int, long, float, double, string)
    // Note: Nested arrays and byte/char/short types are not supported
    static std::string GetParamStringValue(const sptr<AAFwk::IInterface> &value);
    static bool GetParamBoolValue(const sptr<AAFwk::IInterface> &value, bool &result);
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_TOOL_UTIL_H
