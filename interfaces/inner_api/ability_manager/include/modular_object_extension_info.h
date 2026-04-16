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

#ifndef OHOS_ABILITY_RUNTIME_MODULAR_OBJECT_EXTENSION_INFO_H
#define OHOS_ABILITY_RUNTIME_MODULAR_OBJECT_EXTENSION_INFO_H

#include <string>
#include "nlohmann/json.hpp"
#include "parcel.h"

namespace OHOS {
namespace AAFwk {
/**
 * @enum MoeLaunchMode
 * Launch mode of modular object extension.
 */
enum class MoeLaunchMode : int32_t {
    IN_PROCESS = 0,      // All extensions under the same bundle share a single process.
    CROSS_PROCESS = 1,   // Allow modular object extension ability to be started cross the same process.
};

/**
 * @enum MoeThreadMode
 * Thread mode of modular object extension.
 */
enum class MoeThreadMode : int32_t {
    BUNDLE = 0,          // All modular object extensions under the same bundle share a single thread.
    TYPE = 1,            // Modular object extensions with the same name share a single thread.
    INSTANCE = 2,        // Each modular object extension instance is a thread.
};

/**
 * @enum ProcessMode
 * Process mode of modular object extension.
 */
enum class MoeProcessMode : int32_t {
    BUNDLE = 0,          // All modular object extensions under the same bundle share a single process.
    TYPE = 1,            // Modular object extensions with the same name share a single process.
    INSTANCE = 2,        // Each modular object extension instance is a process.
};

/**
 * @struct ModularObjectExtensionInfo
 * Information of a modular object extension.
 */
struct ModularObjectExtensionInfo : public Parcelable {
    std::string bundleName;
    std::string moduleName;
    std::string abilityName;
    int32_t appIndex = 0;
    MoeLaunchMode launchMode = MoeLaunchMode::IN_PROCESS;
    MoeProcessMode processMode = MoeProcessMode::BUNDLE;
    MoeThreadMode threadMode = MoeThreadMode::BUNDLE;
    bool isDisabled = false;

    /**
     * @brief Default constructor.
     */
    ModularObjectExtensionInfo() = default;

    /**
     * @brief Destructor.
     */
    ~ModularObjectExtensionInfo() = default;

    /**
     * @brief Read this object from a Parcel.
     *
     * @param parcel The parcel to read from.
     * @return true if successful, false otherwise.
     */
    bool ReadFromParcel(Parcel &parcel);

    /**
     * @brief Write this object to a Parcel.
     *
     * @param parcel The parcel to write to.
     * @return true if successful, false otherwise.
     */
    bool Marshalling(Parcel &parcel) const override;

    /**
     * @brief Unmarshall a ModularObjectExtensionInfo object from a Parcel.
     *
     * @param parcel The parcel to read from.
     * @return Pointer to the created ModularObjectExtensionInfo object, nullptr if failed.
     */
    static ModularObjectExtensionInfo *Unmarshalling(Parcel &parcel);

    /**
     * @brief Convert this object to a JSON object.
     *
     * @return nlohmann::json The JSON representation.
     */
    nlohmann::json ToJson() const;

    /**
     * @brief Create a ModularObjectExtensionInfo object from a JSON object.
     *
     * @param jsonObject The JSON object.
     * @return ModularObjectExtensionInfo The constructed object.
     */
    static ModularObjectExtensionInfo FromJson(const nlohmann::json &jsonObject);
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_MODULAR_OBJECT_EXTENSION_INFO_H
