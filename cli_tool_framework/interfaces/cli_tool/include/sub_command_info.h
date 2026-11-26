/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"),
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

#ifndef OHOS_ABILITY_RUNTIME_SUB_COMMAND_INFO_H
#define OHOS_ABILITY_RUNTIME_SUB_COMMAND_INFO_H

#include <memory>
#include <nlohmann/json.hpp>
#include <parcel.h>
#include <string>
#include <vector>

namespace OHOS {
namespace CliTool {

/**
 * @brief Subcommand information structure
 */
class SubCommandInfo : public Parcelable {
public:
    std::string description;
    std::vector<std::string> requirePermissions;
    std::string inputSchema;    // JSON string
    std::string outputSchema;   // JSON string
    std::vector<std::string> eventTypes;
    std::string eventSchemas;   // JSON string
    // Declares whether the subcommand supports cooperative cancellation (cancelSession).
    // The value is this subcommand's own declaration only: it is independent of the
    // tool-level flag, and an undeclared subcommand stays false (no inheritance).
    bool cancelable = false;
    // Whether this subcommand is long-running. Independent subcommand-level declaration:
    // when absent it stays false (short-running by default) and does not inherit the
    // tool-level declaration.
    // Declaration only: does not change the timeout behavior of execTool.
    bool isLongRunning = false;

    SubCommandInfo() = default;
    ~SubCommandInfo() = default;

    bool Marshalling(Parcel &parcel) const override;
    static SubCommandInfo *Unmarshalling(Parcel &parcel);

    /**
     * @brief Parse SubCommandInfo from JSON object
     * @param json Input JSON object
     * @param subCmd Output SubCommandInfo
     * @return bool true if parse success
     */
    static bool ParseFromJson(const nlohmann::json &json, SubCommandInfo &subCmd);

    /**
     * @brief Convert SubCommandInfo to JSON object
     */
    nlohmann::json ParseToJson() const;

    /**
     * @brief Validate SubCommandInfo fields
     * @param subCmd SubCommandInfo to validate
     * @return bool true if valid
     */
    static bool Validate(const SubCommandInfo &subCmd);
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_SUB_COMMAND_INFO_H
