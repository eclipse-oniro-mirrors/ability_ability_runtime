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

#ifndef OHOS_ABILITY_RUNTIME_FUNCTION_INFO_H
#define OHOS_ABILITY_RUNTIME_FUNCTION_INFO_H

#include <parcel.h>
#include <string>
#include <nlohmann/json.hpp>

namespace OHOS {
namespace CliTool {

/**
 * @brief Function type enumeration
 */
enum class FunctionType : int32_t {
    INTENT_FUNCTION = 0,
    END,
};

/**
 * @brief Function information structure
 */
class FunctionInfo : public Parcelable {
public:
    std::string functionName;
    std::string functionNamespace;
    std::string version;
    std::string description;
    std::string inputSchema;
    std::string outputSchema;
    FunctionType functionType;

    FunctionInfo() : functionType(FunctionType::INTENT_FUNCTION) {}
    ~FunctionInfo() override = default;

    bool Marshalling(Parcel &parcel) const override;
    static FunctionInfo *Unmarshalling(Parcel &parcel);

    /**
     * @brief Parse FunctionInfo from JSON object
     * @param json Input JSON object
     * @param function Output FunctionInfo
     * @return bool true if parse success
     */
    static bool ParseFromJson(const nlohmann::json &json, FunctionInfo &function);

    /**
     * @brief Convert FunctionInfo to JSON object
     * @return nlohmann::json JSON object
     */
    nlohmann::json ParseToJson() const;

    /**
     * @brief Validate FunctionInfo fields
     * @param function FunctionInfo to validate
     * @return bool true if valid
     */
    static bool Validate(const FunctionInfo &function);
};

/**
 * @brief Raw data type for IDL serialization (shared memory optimization)
 */
class FunctionsRawData {
public:
    std::string ownedData;
    uint32_t size = 0;
    const void* data = nullptr;
    bool isMalloc = false;

    /**
     * @brief Convert vector of FunctionInfo to FunctionsRawData
     * @param functions Input vector of FunctionInfo
     * @param rawData Output FunctionsRawData
     */
    static void FromFunctionInfoVec(const std::vector<FunctionInfo> &functions, FunctionsRawData &rawData);

    /**
     * @brief Convert FunctionsRawData to vector of FunctionInfo
     * @param rawData Input FunctionsRawData
     * @param functions Output vector of FunctionInfo
     * @return int32_t ERR_OK on success, error code otherwise
     */
    static int32_t ToFunctionInfoVec(const FunctionsRawData &rawData, std::vector<FunctionInfo> &functions);

    /**
     * @brief Copy data from raw pointer
     * @param readdata Pointer to data to copy
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t RawDataCpy(const void *readdata);

    ~FunctionsRawData();
};

} // namespace CliTool
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_FUNCTION_INFO_H
