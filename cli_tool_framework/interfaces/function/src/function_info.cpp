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

#include "function_info.h"

#include <cstring>
#include <memory>
#include <securec.h>
#include <sstream>
#include <nlohmann/json.hpp>

#include "cli_error_code.h"
#include "hilog_tag_wrapper.h"

namespace {
constexpr uint32_t MAX_FUNCTION_INFO_COUNT = 10000;  // Maximum number of functions in single transfer
}

namespace OHOS {
namespace CliTool {

namespace {
bool ParseRequiredStringField(const nlohmann::json &json, const std::string &fieldName,
    std::string &output, bool allowEmpty = false)
{
    if (!json.contains(fieldName) || !json[fieldName].is_string()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ParseFromJson failed: %{public}s is missing or not a string", fieldName.c_str());
        return false;
    }
    output = json[fieldName].get<std::string>();
    if (!allowEmpty && output.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ParseFromJson failed: %{public}s is empty", fieldName.c_str());
        return false;
    }
    return true;
}

bool ParseInputSchema(const nlohmann::json &json, std::string &output)
{
    if (!json.contains("inputSchema")) {
        return true;
    }
    const auto &inputSchema = json["inputSchema"];
    if (inputSchema.is_string()) {
        output = inputSchema.get<std::string>();
        if (!output.empty()) {
            nlohmann::json parsed = nlohmann::json::parse(output, nullptr, false);
            if (parsed.is_discarded()) {
                TAG_LOGE(AAFwkTag::CLI_TOOL, "ParseFromJson failed: inputSchema is not valid JSON");
                return false;
            }
        }
        return true;
    }
    TAG_LOGE(AAFwkTag::CLI_TOOL, "ParseFromJson failed: inputSchema must be a string");
    return false;
}

bool ParseOutputSchema(const nlohmann::json &json, std::string &output)
{
    if (!json.contains("outputSchema")) {
        return true;
    }
    const auto &outputSchema = json["outputSchema"];
    if (outputSchema.is_string()) {
        output = outputSchema.get<std::string>();
        if (!output.empty()) {
            nlohmann::json parsed = nlohmann::json::parse(output, nullptr, false);
            if (parsed.is_discarded()) {
                TAG_LOGE(AAFwkTag::CLI_TOOL, "ParseFromJson failed: outputSchema is not valid JSON");
                return false;
            }
        }
        return true;
    }
    TAG_LOGE(AAFwkTag::CLI_TOOL, "ParseFromJson failed: outputSchema must be a string");
    return false;
}

bool ParseFunctionType(const nlohmann::json &json, FunctionType &output)
{
    if (!json.contains("functionType")) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ParseFunctionType failed: functionType field is missing");
        return false;
    }
    const auto &functionType = json["functionType"];
    if (!functionType.is_number_integer()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Invalid functionType type in JSON, must be integer");
        return false;
    }
    int32_t typeValue = functionType.get<int32_t>();
    if (typeValue >= 0 && typeValue < static_cast<int32_t>(FunctionType::END)) {
        output = static_cast<FunctionType>(typeValue);
        return true;
    }
    TAG_LOGE(AAFwkTag::CLI_TOOL, "Invalid functionType value: %{public}d, out of range [0, %{public}d)",
        typeValue, static_cast<int32_t>(FunctionType::END));
    return false;
}

} // namespace

bool FunctionInfo::Marshalling(Parcel &parcel) const
{
    int32_t typeValue = static_cast<int32_t>(functionType);
    if (!parcel.WriteString(functionName)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write functionName failed");
        return false;
    }
    if (!parcel.WriteString(functionNamespace)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write namespace failed");
        return false;
    }
    if (!parcel.WriteString(version)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write version failed");
        return false;
    }
    if (!parcel.WriteString(description)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write description failed");
        return false;
    }
    if (!parcel.WriteString(inputSchema)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write inputSchema failed");
        return false;
    }
    if (!parcel.WriteString(outputSchema)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write outputSchema failed");
        return false;
    }
    if (!parcel.WriteInt32(typeValue)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write functionType failed");
        return false;
    }
    return true;
}

FunctionInfo *FunctionInfo::Unmarshalling(Parcel &parcel)
{
    auto function = std::make_unique<FunctionInfo>();

    int32_t typeValue = 0;
    if (!parcel.ReadString(function->functionName)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Read functionName failed");
        return nullptr;
    }
    if (!parcel.ReadString(function->functionNamespace)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Read namespace failed");
        return nullptr;
    }
    if (!parcel.ReadString(function->version)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Read version failed");
        return nullptr;
    }
    if (!parcel.ReadString(function->description)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Read description failed");
        return nullptr;
    }
    if (!parcel.ReadString(function->inputSchema)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Read inputSchema failed");
        return nullptr;
    }
    if (!parcel.ReadString(function->outputSchema)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Read outputSchema failed");
        return nullptr;
    }
    if (!parcel.ReadInt32(typeValue)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Read functionType failed");
        return nullptr;
    }

    // Validate functionType range
    if (typeValue < 0 || typeValue >= static_cast<int32_t>(FunctionType::END)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Invalid functionType value: %{public}d, out of range [0, %{public}d)",
            typeValue, static_cast<int32_t>(FunctionType::END));
        return nullptr;
    }
    function->functionType = static_cast<FunctionType>(typeValue);
    return function.release();
}

bool FunctionInfo::ParseFromJson(const nlohmann::json &json, FunctionInfo &function)
{
    return ParseRequiredStringField(json, "functionName", function.functionName) &&
           ParseRequiredStringField(json, "functionNamespace", function.functionNamespace) &&
           ParseRequiredStringField(json, "version", function.version, true) &&
           ParseRequiredStringField(json, "description", function.description, true) &&
           ParseInputSchema(json, function.inputSchema) &&
           ParseOutputSchema(json, function.outputSchema) &&
           ParseFunctionType(json, function.functionType);
}

nlohmann::json FunctionInfo::ParseToJson() const
{
    nlohmann::json j;

    j["functionName"] = functionName;
    j["functionNamespace"] = functionNamespace;
    j["version"] = version;
    j["description"] = description;

    if (!inputSchema.empty()) {
        j["inputSchema"] = inputSchema;
    }

    if (!outputSchema.empty()) {
        j["outputSchema"] = outputSchema;
    }

    j["functionType"] = static_cast<int32_t>(functionType);

    return j;
}

bool FunctionInfo::Validate(const FunctionInfo &function)
{
    if (function.functionName.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Validate failed: functionName is empty");
        return false;
    }

    if (function.functionNamespace.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Validate failed: namespace is empty");
        return false;
    }

    if (!function.inputSchema.empty()) {
        nlohmann::json inputSchemaJson = nlohmann::json::parse(function.inputSchema, nullptr, false);
        if (inputSchemaJson.is_discarded()) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Validate failed: inputSchema is not valid JSON");
            return false;
        }
    }

    if (!function.outputSchema.empty()) {
        nlohmann::json outputSchemaJson = nlohmann::json::parse(function.outputSchema, nullptr, false);
        if (outputSchemaJson.is_discarded()) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Validate failed: outputSchema is not valid JSON");
            return false;
        }
    }

    return true;
}

// ==================== FunctionsRawData Implementation ====================

FunctionsRawData::~FunctionsRawData()
{
    if (data != nullptr && isMalloc) {
        free(const_cast<void*>(data));
        isMalloc = false;
        data = nullptr;
    }
}

int32_t FunctionsRawData::RawDataCpy(const void *readdata)
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

void FunctionsRawData::FromFunctionInfoVec(const std::vector<FunctionInfo> &functions, FunctionsRawData &rawData)
{
    std::stringstream ss;
    uint32_t count = functions.size();
    ss.write(reinterpret_cast<const char*>(&count), sizeof(count));

    for (uint32_t i = 0; i < count; ++i) {
        std::string dumped = functions[i].ParseToJson().dump();
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

int32_t FunctionsRawData::ToFunctionInfoVec(const FunctionsRawData &rawData, std::vector<FunctionInfo> &functions)
{
    std::stringstream ss;
    ss.write(reinterpret_cast<const char *>(rawData.data), rawData.size);
    ss.seekg(0, std::ios::beg);
    uint32_t ssLength = static_cast<uint32_t>(ss.str().length());
    uint32_t count = 0;
    ss.read(reinterpret_cast<char *>(&count), sizeof(count));
    if (count > MAX_FUNCTION_INFO_COUNT) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "functions exceed maxSize %{public}d, count: %{public}d",
            MAX_FUNCTION_INFO_COUNT, count);
        return ERR_INVALID_VALUE;
    }
    functions.resize(count);
    for (uint32_t i = 0; i < count; ++i) {
        uint32_t functionSize = 0;
        ss.read(reinterpret_cast<char *>(&functionSize), sizeof(functionSize));
        if (functionSize > ssLength - static_cast<uint32_t>(ss.tellg())) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "functionSize:%{public}u is invalid", functionSize);
            return ERR_INVALID_VALUE;
        }
        std::string functionStr(functionSize, '\0');
        ss.read(functionStr.data(), functionSize);
        nlohmann::json j = nlohmann::json::parse(functionStr, nullptr, false);
        if (j.is_discarded()) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to parse JSON for function %{public}d", i);
            return ERR_JSON_PARSE_FAILED;
        }
        if (!FunctionInfo::ParseFromJson(j, functions[i])) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to parse FunctionInfo from JSON for function %{public}d", i);
            return ERR_JSON_PARSE_FAILED;
        }
    }
    return ERR_OK;
}

} // namespace CliTool
} // namespace OHOS
