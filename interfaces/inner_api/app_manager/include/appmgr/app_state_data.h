
/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_APP_STATE_DATA_H
#define OHOS_ABILITY_RUNTIME_APP_STATE_DATA_H

#include <sys/types.h>

#include "ability_info.h"
#include "iremote_object.h"
#include "parcel.h"

namespace OHOS {
namespace AppExecFwk {
struct AppStateData : public Parcelable {
    /**
     * @brief read this Sequenceable object from a Parcel.
     *
     * @param inParcel Indicates the Parcel object into which the Sequenceable object has been marshaled.
     * @return Returns true if read successed; returns false otherwise.
     */
    bool ReadFromParcel(Parcel &parcel);

    /**
     * @brief Marshals this Sequenceable object into a Parcel.
     *
     * @param outParcel Indicates the Parcel object to which the Sequenceable object will be marshaled.
     */
    virtual bool Marshalling(Parcel &parcel) const override;

    /**
     * @brief Unmarshals this Sequenceable object from a Parcel.
     *
     * @param inParcel Indicates the Parcel object into which the Sequenceable object has been marshaled.
     */
    static AppStateData *Unmarshalling(Parcel &parcel);

    /**
     * @brief Check if extension type belongs to uiextension.
     *
     * @param type extension type
     * @return true extension type is a uiextension.
     * @return false extension type is not a uiextension.
     */
    static bool IsUIExtension(const AppExecFwk::ExtensionAbilityType type);

    bool isFocused = false;
    bool isSplitScreenMode = false;
    bool isFloatingWindowMode = false;
    bool isSpecifyTokenId = false;
    bool isPreloadModule = false;
    int32_t pid = -1;
    int32_t uid = 0;
    int32_t callerUid = -1;
    int32_t state = 0;
    int32_t appIndex = 0;
    uint32_t accessTokenId = 0;
    int32_t preloadMode = -1;
    ExtensionAbilityType extensionType = ExtensionAbilityType::UNSPECIFIED;
    std::vector<int32_t> renderPids;
    std::string bundleName;
    std::string callerBundleName;
    bool isFromWindowFocusChanged = false;
    int32_t byCallStatus = 0;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APP_STATE_DATA_H
