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

#ifndef OHOS_ABILITY_RUNTIME_SANDBOX_CLONE_PARAMS_H
#define OHOS_ABILITY_RUNTIME_SANDBOX_CLONE_PARAMS_H

#include <parcel.h>
#include <string>

namespace OHOS {
namespace AAFwk {
/**
 * @struct SandboxCloneParams
 * SandboxCloneParams is used to pass parameters for starting sandbox clone ability.
 * Contains caller information needed for sandbox clone application launch.
 */
struct SandboxCloneParams : public Parcelable {
    std::string callerBundleName;
    int32_t callerUid = -1;
    uint32_t callerTokenId = 0;

    /**
     * @brief Constructor
     */
    SandboxCloneParams() = default;

    /**
     * @brief Destructor
     */
    ~SandboxCloneParams() override = default;

    /**
     * @brief Read data from parcel
     * @param parcel The parcel object to read from
     * @return Returns true on success, false on failure
     */
    bool ReadFromParcel(Parcel &parcel);

    /**
     * @brief Write data to parcel
     * @param parcel The parcel object to write to
     * @return Returns true on success, false on failure
     */
    virtual bool Marshalling(Parcel &parcel) const override;

    /**
     * @brief Create SandboxCloneParams from parcel
     * @param parcel The parcel object to read from
     * @return Returns pointer to SandboxCloneParams, or nullptr on failure
     */
    static SandboxCloneParams *Unmarshalling(Parcel &parcel);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_SANDBOX_CLONE_PARAMS_H
