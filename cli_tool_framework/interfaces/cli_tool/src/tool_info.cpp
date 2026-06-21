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

#include "tool_info.h"

#include <nlohmann/json.hpp>
#include <set>
#include <sstream>

#include "hilog_tag_wrapper.h"
#include "securec.h"

namespace OHOS {
namespace CliTool {

namespace {
constexpr uint32_t MAX_TOOL_INFO_COUNT = 200000;
} // namespace

// ToolInfo implementation
bool ToolInfo::Marshalling(Parcel &parcel) const
{
    // Serialize subcommands map to JSON string using SubCommandInfo::ParseToJson
    std::string subcommandsJson;
    if (!subcommands.empty()) {
        nlohmann::json j;
        for (const auto &pair : subcommands) {
            j[pair.first] = pair.second.ParseToJson();
        }
        subcommandsJson = j.dump();
    }

    return parcel.WriteString(name) &&
           parcel.WriteString(version) &&
           parcel.WriteString(description) &&
           parcel.WriteString(executablePath) &&
           parcel.WriteStringVector(requirePermissions) &&
           parcel.WriteString(inputSchema) &&
           parcel.WriteString(outputSchema) &&
           parcel.WriteString(eventSchemas) &&
           parcel.WriteStringVector(eventTypes) &&
           parcel.WriteBool(hasSubCommand) &&
           parcel.WriteString(subcommandsJson) &&
           parcel.WriteBool(isLockScreenExecutionAllowed);
}

ToolInfo *ToolInfo::Unmarshalling(Parcel &parcel)
{
    auto *tool = new (std::nothrow) ToolInfo();
    if (tool == nullptr) {
        return nullptr;
    }

    std::string subcommandsJson;
    if (!parcel.ReadString(tool->name) ||
        !parcel.ReadString(tool->version) ||
        !parcel.ReadString(tool->description) ||
        !parcel.ReadString(tool->executablePath) ||
        !parcel.ReadStringVector(&tool->requirePermissions) ||
        !parcel.ReadString(tool->inputSchema) ||
        !parcel.ReadString(tool->outputSchema) ||
        !parcel.ReadString(tool->eventSchemas) ||
        !parcel.ReadStringVector(&tool->eventTypes) ||
        !parcel.ReadBool(tool->hasSubCommand) ||
        !parcel.ReadString(subcommandsJson) ||
        !parcel.ReadBool(tool->isLockScreenExecutionAllowed)) {
        delete tool;
        return nullptr;
    }

    // Parse subcommands JSON string to map using SubCommandInfo::ParseFromJson
    if (!subcommandsJson.empty()) {
        nlohmann::json j = nlohmann::json::parse(subcommandsJson, nullptr, false);
        if (!j.is_discarded() && j.is_object()) {
            for (auto it = j.begin(); it != j.end(); ++it) {
                SubCommandInfo subCmd;
                if (!SubCommandInfo::ParseFromJson(it.value(), subCmd)) {
                    delete tool;
                    return nullptr;  // subcommand parse failed
                }
                tool->subcommands[it.key()] = std::move(subCmd);
            }
        }
    }

    return tool;
}

bool ToolInfo::ValidateName(const std::string &name)
{
    if (name.empty()) {
        return false;
    }

    // Must start with "ohos-" or "hms-"
    const std::string OHOS_PREFIX = "ohos-";
    const std::string HMS_PREFIX = "hms-";
    const size_t MAX_SUFFIX_LENGTH = 32;

    bool hasValidPrefix = false;
    size_t suffixStart = 0;

    if (name.compare(0, OHOS_PREFIX.size(), OHOS_PREFIX) == 0) {
        hasValidPrefix = true;
        suffixStart = OHOS_PREFIX.size();
    } else if (name.compare(0, HMS_PREFIX.size(), HMS_PREFIX) == 0) {
        hasValidPrefix = true;
        suffixStart = HMS_PREFIX.size();
    }

    if (!hasValidPrefix) {
        return false;
    }

    // Suffix must not exceed 32 characters
    std::string suffix = name.substr(suffixStart);
    if (suffix.empty() || suffix.size() > MAX_SUFFIX_LENGTH) {
        return false;
    }

    return true;
}

bool ToolInfo::ValidateExecutablePath(const std::string &path)
{
    if (path.empty()) {
        return false;
    }

    // Must be absolute path (start with '/')
    if (path[0] != '/') {
        return false;
    }

    return true;
}

bool ToolInfo::ValidateRequirePermissions(const std::vector<std::string> &permissions)
{
    if (permissions.empty()) {
        return true;
    }

    std::set<std::string> seenPerms;
    for (const auto &perm : permissions) {
        if (seenPerms.find(perm) != seenPerms.end()) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "ValidateRequirePermissions failed: duplicate permission %{public}s",
                perm.c_str());
            return false;
        }
        seenPerms.insert(perm);
    }

    return true;
}

bool ToolInfo::ValidateEventTypes(const std::vector<std::string> &eventTypes)
{
    if (eventTypes.empty()) {
        return true;
    }

    std::set<std::string> seenEvents;
    for (const auto &evt : eventTypes) {
        if (seenEvents.find(evt) != seenEvents.end()) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "ValidateEventTypes failed: duplicate eventType %{public}s",
                evt.c_str());
            return false;
        }
        seenEvents.insert(evt);
    }

    return true;
}

bool ToolInfo::ParseFromJson(const nlohmann::json &json, ToolInfo &tool)
{
    // name is required and must be valid
    if (!json.contains("name") || !json["name"].is_string()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ParseFromJson failed: name is missing or not a string");
        return false;
    }

    std::string name = json["name"];
    if (!ValidateName(name)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ParseFromJson failed: name %{public}s is invalid", name.c_str());
        return false;
    }
    tool.name = name;

    // version is required and must be non-empty
    if (!json.contains("version") || !json["version"].is_string()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ParseFromJson failed: version is missing or not a string");
        return false;
    }
    std::string version = json["version"];
    if (version.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ParseFromJson failed: version is empty");
        return false;
    }
    tool.version = version;

    // description is required and must be non-empty
    if (!json.contains("description") || !json["description"].is_string()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ParseFromJson failed: description is missing or not a string");
        return false;
    }
    std::string description = json["description"];
    if (description.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ParseFromJson failed: description is empty");
        return false;
    }
    tool.description = description;

    // executablePath is required and must be absolute path
    if (!json.contains("executablePath") || !json["executablePath"].is_string()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ParseFromJson failed: executablePath is missing or not a string");
        return false;
    }
    std::string executablePath = json["executablePath"];
    if (!ValidateExecutablePath(executablePath)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ParseFromJson failed: executablePath %{public}s is invalid",
            executablePath.c_str());
        return false;
    }
    tool.executablePath = executablePath;

    // requirePermissions is required and must be array
    if (!json.contains("requirePermissions") || !json["requirePermissions"].is_array()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ParseFromJson failed: requirePermissions is missing or not an array");
        return false;
    }
    std::vector<std::string> perms;
    for (const auto &perm : json["requirePermissions"]) {
        if (!perm.is_string()) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "ParseFromJson failed: requirePermissions contains non-string item");
            return false;
        }
        std::string permStr = perm.get<std::string>();
        if (!permStr.empty()) {
            perms.push_back(std::move(permStr));
        }
    }
    tool.requirePermissions = std::move(perms);

    // inputSchema is required and must be a JSON object
    if (!json.contains("inputSchema") || !json["inputSchema"].is_object()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ParseFromJson failed: inputSchema is missing or not a JSON object");
        return false;
    }
    tool.inputSchema = json["inputSchema"].dump();

    // outputSchema is required and must be a JSON object
    if (!json.contains("outputSchema") || !json["outputSchema"].is_object()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ParseFromJson failed: outputSchema is missing or not a JSON object");
        return false;
    }
    tool.outputSchema = json["outputSchema"].dump();

    if (json.contains("eventSchemas")) {
        if (!json["eventSchemas"].is_object()) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "ParseFromJson failed: eventSchemas is not a JSON object");
            return false;
        }
        tool.eventSchemas = json["eventSchemas"].dump();
    }
    if (json.contains("eventTypes")) {
        if (!json["eventTypes"].is_array()) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "ParseFromJson failed: eventTypes is not an array");
            return false;
        }
        std::vector<std::string> types;
        for (const auto &item : json["eventTypes"]) {
            if (!item.is_string()) {
                TAG_LOGE(AAFwkTag::CLI_TOOL, "ParseFromJson failed: eventTypes contains non-string item");
                return false;
            }
            std::string typeStr = item.get<std::string>();
            if (!typeStr.empty()) {
                types.push_back(std::move(typeStr));
            }
        }
        tool.eventTypes = std::move(types);
    }
    if (json.contains("hasSubCommand")) {
        if (!json["hasSubCommand"].is_boolean()) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "ParseFromJson failed: hasSubCommand is not a boolean");
            return false;
        }
        tool.hasSubCommand = json["hasSubCommand"];
    }
    if (tool.hasSubCommand) {
        if (!json.contains("subcommands") || !json["subcommands"].is_object() || json["subcommands"].empty()) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "ParseFromJson failed: subcommands is required when hasSubCommand is true");
            return false;
        }
        for (auto it = json["subcommands"].begin(); it != json["subcommands"].end(); ++it) {
            SubCommandInfo subCmd;
            if (!SubCommandInfo::ParseFromJson(it.value(), subCmd)) {
                TAG_LOGE(AAFwkTag::CLI_TOOL, "ParseFromJson failed: subcommand %{public}s parse failed",
                    it.key().c_str());
                return false;
            }
            tool.subcommands[it.key()] = std::move(subCmd);
        }
    }
    if (json.contains("isLockScreenExecutionAllowed")) {
        if (!json["isLockScreenExecutionAllowed"].is_boolean()) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "ParseFromJson failed: isLockScreenExecutionAllowed is not a boolean");
            return false;
        }
        tool.isLockScreenExecutionAllowed = json["isLockScreenExecutionAllowed"];
    }

    return true;
}

nlohmann::json ToolInfo::ParseToJson() const
{
    nlohmann::json j;

    j["name"] = name;
    j["version"] = version;
    j["description"] = description;
    j["executablePath"] = executablePath;
    j["requirePermissions"] = requirePermissions;
    if (!inputSchema.empty()) {
        nlohmann::json inputSchemaJson = nlohmann::json::parse(inputSchema, nullptr, false);
        if (!inputSchemaJson.is_discarded()) {
            j["inputSchema"] = inputSchemaJson;
        } else {
            j["inputSchema"] = inputSchema;
        }
    }
    if (!outputSchema.empty()) {
        nlohmann::json outputSchemaJson = nlohmann::json::parse(outputSchema, nullptr, false);
        if (!outputSchemaJson.is_discarded()) {
            j["outputSchema"] = outputSchemaJson;
        } else {
            j["outputSchema"] = outputSchema;
        }
    }
    if (!eventSchemas.empty()) {
        nlohmann::json eventSchemasJson = nlohmann::json::parse(eventSchemas, nullptr, false);
        if (!eventSchemasJson.is_discarded()) {
            j["eventSchemas"] = eventSchemasJson;
        } else {
            j["eventSchemas"] = eventSchemas;
        }
    }
    j["eventTypes"] = eventTypes;
    j["hasSubCommand"] = hasSubCommand;
    if (!subcommands.empty()) {
        nlohmann::json subcommandsJson;
        for (const auto &pair : subcommands) {
            subcommandsJson[pair.first] = pair.second.ParseToJson();
        }
        j["subcommands"] = subcommandsJson;
    }
    j["isLockScreenExecutionAllowed"] = isLockScreenExecutionAllowed;

    return j;
}

bool ToolInfo::Validate(const ToolInfo &tool)
{
    // name must be valid
    if (!ValidateName(tool.name)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Validate failed: name %{public}s is invalid", tool.name.c_str());
        return false;
    }

    // version is required and must be non-empty
    if (tool.version.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Validate failed: version is empty");
        return false;
    }

    // description is required and must be non-empty
    if (tool.description.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Validate failed: description is empty");
        return false;
    }

    // executablePath must be valid absolute path
    if (!ValidateExecutablePath(tool.executablePath)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Validate failed: executablePath %{public}s is invalid",
            tool.executablePath.c_str());
        return false;
    }

    // inputSchema is required and must be valid JSON string
    if (tool.inputSchema.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Validate failed: inputSchema is empty");
        return false;
    }
    nlohmann::json inputSchemaJson = nlohmann::json::parse(tool.inputSchema, nullptr, false);
    if (inputSchemaJson.is_discarded()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Validate failed: inputSchema is not valid JSON");
        return false;
    }

    // outputSchema is required and must be valid JSON string
    if (tool.outputSchema.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Validate failed: outputSchema is empty");
        return false;
    }
    nlohmann::json outputSchemaJson = nlohmann::json::parse(tool.outputSchema, nullptr, false);
    if (outputSchemaJson.is_discarded()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Validate failed: outputSchema is not valid JSON");
        return false;
    }

    // eventSchemas: if not empty, must be valid JSON string
    if (!tool.eventSchemas.empty()) {
        nlohmann::json eventSchemasJson = nlohmann::json::parse(tool.eventSchemas, nullptr, false);
        if (eventSchemasJson.is_discarded()) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Validate failed: eventSchemas is not valid JSON");
            return false;
        }
    }

    // if hasSubCommand is true, subcommands must not be empty
    if (tool.hasSubCommand && tool.subcommands.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Validate failed: hasSubCommand is true but subcommands is empty");
        return false;
    }

    return true;
}

ToolsRawData::~ToolsRawData()
{
    if (data != nullptr && isMalloc) {
        free(const_cast<void*>(data));
        isMalloc = false;
        data = nullptr;
    }
}

int32_t ToolsRawData::RawDataCpy(const void *readdata)
{
    if (readdata == nullptr || size == 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "null data or zero size");
        return ERR_INVALID_VALUE;
    }
    void* newData = malloc(size);
    if (newData == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "malloc failed");
        return ERR_INVALID_VALUE;
    }
    if (memcpy_s(newData, size, readdata, size) != EOK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "memcpy_s failed");
        free(newData);
        return ERR_INVALID_VALUE;
    }
    if (data != nullptr && isMalloc) {
        free(const_cast<void*>(data));
        data = nullptr;
    }
    data = newData;
    isMalloc = true;
    return ERR_OK;
}

void ToolsRawData::FromToolInfoVec(const std::vector<ToolInfo> &tools, ToolsRawData &rawData)
{
    std::stringstream ss;
    uint32_t count = tools.size();
    ss.write(reinterpret_cast<const char*>(&count), sizeof(count));

    for (uint32_t i = 0; i < count; ++i) {
        std::string dumped = tools[i].ParseToJson().dump();
        uint32_t strLen = dumped.length();
        ss.write(reinterpret_cast<const char*>(&strLen), sizeof(strLen));
        ss.write(dumped.c_str(), strLen);
    }
    std::string result = ss.str();
    rawData.ownedData = std::move(result);
    rawData.data = rawData.ownedData.data();
    rawData.size = rawData.ownedData.size();
    rawData.isMalloc = false;
}

int32_t ToolsRawData::ToToolInfoVec(const ToolsRawData &rawData, std::vector<ToolInfo> &tools)
{
    std::stringstream ss;
    ss.write(reinterpret_cast<const char *>(rawData.data), rawData.size);
    ss.seekg(0, std::ios::beg);
    uint32_t ssLength = static_cast<uint32_t>(ss.str().length());
    uint32_t count = 0;
    ss.read(reinterpret_cast<char *>(&count), sizeof(count));
    if (count > MAX_TOOL_INFO_COUNT) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "tools exceed maxSize %{public}d, count: %{public}d",
            MAX_TOOL_INFO_COUNT, count);
        return ERR_INVALID_VALUE;
    }
    tools.resize(count);
    for (uint32_t i = 0; i < count; ++i) {
        uint32_t toolSize = 0;
        ss.read(reinterpret_cast<char *>(&toolSize), sizeof(toolSize));
        if (toolSize > ssLength - static_cast<uint32_t>(ss.tellg())) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "toolSize:%{public}u is invalid", toolSize);
            return ERR_INVALID_VALUE;
        }
        std::string toolStr(toolSize, '\0');
        ss.read(toolStr.data(), toolSize);
        nlohmann::json jsonObject = nlohmann::json::parse(toolStr, nullptr, false, true);
        if (jsonObject.is_discarded()) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "json parse failed, index: %{public}u", i);
            return ERR_INVALID_VALUE;
        }
        if (!ToolInfo::ParseFromJson(jsonObject, tools[i])) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "ParseFromJson failed, index: %{public}u", i);
            return ERR_INVALID_VALUE;
        }
    }
    return ERR_OK;
}

} // namespace CliTool
} // namespace OHOS