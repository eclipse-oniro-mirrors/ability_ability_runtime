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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_STATE_DATA_H
#define OHOS_ABILITY_RUNTIME_ABILITY_STATE_DATA_H

#include <sys/types.h>

#include "parcel.h"

#include "app_mgr_constants.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
struct AbilityStateData : public Parcelable {
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
    bool MarshallingOne(Parcel &parcel) const;

    /**
     * @brief Unmarshals this Sequenceable object from a Parcel.
     *
     * @param inParcel Indicates the Parcel object into which the Sequenceable object has been marshaled.
     */
    static AbilityStateData *Unmarshalling(Parcel &parcel);

    bool isFocused = false;
    bool isAtomicService = false;
    int32_t uid = 0;
    int32_t callerUid = -1;
    int32_t callerPid = 0;
    int32_t abilityRecordId = 0;
    int32_t appCloneIndex = -1;
    int32_t extensionAbilityType = -1;
    int32_t processType = 0;
    int32_t abilityType = 0;
    int32_t abilityState = 0;
    pid_t pid = 0;
    sptr<IRemoteObject> token;
    std::string moduleName;
    std::string bundleName;
    std::string abilityName;
    std::string callerBundleName;
    std::string callerAbilityName;
    bool isInnerNotify = false;
    int32_t preloadMode = -1;
    int32_t lastUIAbilityCallerUid = -1;
    std::string lastUIAbilityCallerName;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_STATE_DATA_H
