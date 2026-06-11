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

#include <memory>
#include <nlohmann/json.hpp>

#include "hilog_tag_wrapper.h"

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
    if (!json.contains("inputSchema") || !json["inputSchema"].is_object()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ParseFromJson failed: inputSchema is missing or not an object");
        return false;
    }
    output = json["inputSchema"].dump();
    return true;
}

bool ParseOutputSchema(const nlohmann::json &json, std::string &output)
{
    if (!json.contains("outputSchema")) {
        return true;  // optional
    }
    const auto &outputSchema = json["outputSchema"];
    if (outputSchema.is_object()) {
        output = outputSchema.dump();
    } else if (outputSchema.is_string()) {
        output = outputSchema.get<std::string>();
    } else if (!outputSchema.is_null()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ParseFromJson failed: outputSchema has invalid type");
        return false;
    }
    return true;
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
    if (!parcel.WriteString(funcNamespace)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write namespace failed");
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
    if (!parcel.ReadString(function->funcNamespace)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Read namespace failed");
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
           ParseRequiredStringField(json, "namespace", function.funcNamespace) &&
           ParseRequiredStringField(json, "description", function.description, true) &&
           ParseInputSchema(json, function.inputSchema) &&
           ParseOutputSchema(json, function.outputSchema) &&
           ParseFunctionType(json, function.functionType);
}

nlohmann::json FunctionInfo::ParseToJson() const
{
    nlohmann::json j;

    j["functionName"] = functionName;
    j["namespace"] = funcNamespace;
    j["description"] = description;

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

    j["functionType"] = static_cast<int32_t>(functionType);

    return j;
}

bool FunctionInfo::Validate(const FunctionInfo &function)
{
    // functionName must not be empty
    if (function.functionName.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Validate failed: functionName is empty");
        return false;
    }

    // namespace must not be empty
    if (function.funcNamespace.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Validate failed: namespace is empty");
        return false;
    }

    // inputSchema is required and must be valid JSON
    if (function.inputSchema.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Validate failed: inputSchema is empty");
        return false;
    }
    nlohmann::json inputSchemaJson = nlohmann::json::parse(function.inputSchema, nullptr, false);
    if (inputSchemaJson.is_discarded()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Validate failed: inputSchema is not valid JSON");
        return false;
    }

    // outputSchema: if not empty, must be valid JSON
    if (!function.outputSchema.empty()) {
        nlohmann::json outputSchemaJson = nlohmann::json::parse(function.outputSchema, nullptr, false);
        if (outputSchemaJson.is_discarded()) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Validate failed: outputSchema is not valid JSON");
            return false;
        }
    }

    return true;
}

} // namespace CliTool
} // namespace OHOS
