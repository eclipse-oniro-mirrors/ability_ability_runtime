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

#ifndef OHOS_ABILITY_RUNTIME_TOOL_INFO_H
#define OHOS_ABILITY_RUNTIME_TOOL_INFO_H

#include "sub_command_info.h"
#include "tool_summary.h"

#include <iremote_broker.h>
#include <iremote_object.h>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <parcel.h>
#include <string>
#include <vector>

namespace OHOS {
namespace CliTool {

class ToolInfo;

/**
 * @brief Raw data type for IDL serialization (shared memory optimization)
 */
class ToolsRawData {
public:
    std::string ownedData;
    uint32_t size = 0;
    const void* data = nullptr;
    bool isMalloc = false;

    static void FromToolInfoVec(const std::vector<ToolInfo> &tools, ToolsRawData &rawData);
    static int32_t ToToolInfoVec(const ToolsRawData &rawData, std::vector<ToolInfo> &tools);
    int32_t RawDataCpy(const void *readdata);
    ~ToolsRawData();
};

/**
 * @brief Tool information structure (full version)
 */
class ToolInfo : public Parcelable {
public:
    std::string name;
    std::string version;
    std::string description;
    std::string executablePath;
    std::vector<std::string> requirePermissions;
    std::string inputSchema;       // JSON string
    std::string outputSchema;      // JSON string
    std::vector<std::string> eventTypes;
    std::string eventSchemas;      // JSON string (map of event type to schema)
    bool hasSubCommand = false;
    std::map<std::string, SubCommandInfo> subcommands;

    ToolInfo() = default;
    ~ToolInfo() = default;

    bool Marshalling(Parcel &parcel) const override;
    static ToolInfo *Unmarshalling(Parcel &parcel);

    /**
     * @brief Validate tool name
     * @param name Tool name to validate
     * @return bool true if valid
     */
    static bool ValidateName(const std::string &name);

    /**
     * @brief Validate executable path (must be absolute path)
     * @param path Executable path to validate
     * @return bool true if valid
     */
    static bool ValidateExecutablePath(const std::string &path);

    /**
     * @brief Validate requirePermissions array (all items must be unique strings)
     * @param permissions Permissions array to validate
     * @return bool true if valid
     */
    static bool ValidateRequirePermissions(const std::vector<std::string> &permissions);

    /**
     * @brief Validate eventTypes array (all items must be unique strings)
     * @param eventTypes Event types array to validate
     * @return bool true if valid
     */
    static bool ValidateEventTypes(const std::vector<std::string> &eventTypes);

    /**
     * @brief Parse ToolInfo from JSON object
     * @param json Input JSON object
     * @param tool Output ToolInfo
     * @return bool true if parse success and required fields are valid
     */
    static bool ParseFromJson(const nlohmann::json &json, ToolInfo &tool);

    /**
     * @brief Convert ToolInfo to JSON object
     */
    nlohmann::json ParseToJson() const;

    /**
     * @brief Validate ToolInfo fields
     * @param tool ToolInfo to validate
     * @return bool true if valid
     */
    static bool Validate(const ToolInfo &tool);
};
} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_TOOL_INFO_H