/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_PARAM_H
#define OHOS_ABILITY_RUNTIME_PARAM_H

#include "extension_load_param.h"
#include "iremote_object.h"
#include "parcel.h"
#include "refbase.h"

namespace OHOS {
namespace AbilityRuntime {
struct LoadParam : public Parcelable {
    /**
     * @brief Marshals this object into a Parcel.
     *
     * @param parcel Indicates the Parcel object to which the object will be marshaled.
     */
    virtual bool Marshalling(Parcel &parcel) const override;

    bool MarshallingTwo(Parcel &parcel) const;

    /**
     * @brief Unmarshals this object from a Parcel.
     *
     * @param parcel Indicates the Parcel object into which the object has been marshaled.
     */
    static LoadParam *Unmarshalling(Parcel &parcel);
    bool ReadFromParcel(Parcel &parcel);

    int32_t abilityRecordId = -1;
    bool isShellCall = false;
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> preToken = nullptr;
    std::string instanceKey = "";
    bool isKeepAlive = false;
    bool isMainElementRunning = false;
    bool isKeepAliveAppService = false;
    bool isCallerSetProcess = false;
    std::string customProcessFlag = "";
    uint32_t extensionProcessMode = 0;
    ExtensionLoadParam extensionLoadParam;
    bool isStartupHide = false;
    pid_t callingPid = -1;
    uint64_t loadAbilityCallbackId = 0;
    bool isPrelaunch = false;
    bool isPreloadStart = false;
    pid_t selfPid = -1;
    bool isPreloadUIExtension = false;
    int32_t loadTimeout = 0;
    int32_t byCallStatus = 0;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_PARAM_H