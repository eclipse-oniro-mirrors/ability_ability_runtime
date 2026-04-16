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

#ifndef OHOS_ABILITY_RUNTIME_PROCESS_DATA_H
#define OHOS_ABILITY_RUNTIME_PROCESS_DATA_H

#include <sys/types.h>

#include "iremote_object.h"
#include "parcel.h"
#include "running_process_info.h"

namespace OHOS {
namespace AppExecFwk {
struct ProcessData : public Parcelable {
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
    static ProcessData *Unmarshalling(Parcel &parcel);

    std::string bundleName;
    int32_t pid = 0;
    int32_t uid = 0; // host uid
    int32_t hostPid = 0;
    int32_t gpuPid = 0;
    int32_t renderUid = -1;
    AppProcessState state;
    bool isContinuousTask = false;
    bool isKeepAlive = false;
    bool isFocused = false;
    int32_t requestProcCode = 0;
    int32_t processChangeReason = 0;
    std::string processName;
    ProcessType processType = ProcessType::NORMAL;
    ExtensionAbilityType extensionType = ExtensionAbilityType::UNSPECIFIED;
    uint32_t accessTokenId = 0;
    bool isTestMode = false; // Indicates whether the process is started by aa test
    int32_t exitReason = 0;
    std::string exitMsg = "";
    int32_t childUid = -1;
    bool isPreload = false;
    bool isPreloadModule = false;
    int32_t callerPid = -1;
    int32_t callerUid = -1;
    std::string killReason;
    bool isFromWindowFocusChanged = false;
    int32_t preloadMode = -1;
    int32_t imageProcessType = 0;
    bool isPreloadUIExtension = false;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_PROCESS_DATA_H
