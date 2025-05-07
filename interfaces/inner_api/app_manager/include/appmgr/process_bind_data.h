/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_PROCESS_BIND_DATA_H
#define OHOS_ABILITY_RUNTIME_PROCESS_BIND_DATA_H

#include <sys/types.h>

#include "iremote_object.h"
#include "parcel.h"
#include "running_process_info.h"

namespace OHOS {
namespace AppExecFwk {
struct UIExtensionProcessBindInfo {
    std::string bundleName;
    int32_t pid = 0;
    int32_t uid = 0;
    bool isKeepAlive = false;
    ProcessType processType = ProcessType::EXTENSION;
    ExtensionAbilityType extensionType = ExtensionAbilityType::UNSPECIFIED;
    int32_t callerPid = -1;
    int32_t callerUid = -1;
    std::string callerBundleName;
    int32_t notifyProcessBind = -1;
};
struct ProcessBindData : public Parcelable {
    /**
     * @brief read this Sequenceable object from a Parcel.
     *
     * @param inParcel Indicates the Parcel object into which the Sequenceable
     * object has been marshaled.
     * @return Returns true if read successed; returns false otherwise.
     */
    bool ReadFromParcel(Parcel &parcel);

    /**
     * @brief Marshals this Sequenceable object into a Parcel.
     *
     * @param outParcel Indicates the Parcel object to which the Sequenceable
     * object will be marshaled.
     */
    virtual bool Marshalling(Parcel &parcel) const override;

    /**
     * @brief Unmarshals this Sequenceable object from a Parcel.
     *
     * @param inParcel Indicates the Parcel object into which the Sequenceable
     * object has been marshaled.
     */
    static ProcessBindData *Unmarshalling(Parcel &parcel);

    std::string bundleName;
    int32_t pid = 0;
    int32_t uid = 0;
    bool isKeepAlive = false;
    ProcessType processType = ProcessType::EXTENSION;
    ExtensionAbilityType extensionType = ExtensionAbilityType::UNSPECIFIED;
    int32_t callerPid = -1;
    int32_t callerUid = -1;
    std::string callerBundleName;
    // 0：unBind 1:Bind
    int32_t bindingRelation;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_PROCESS_BIND_DATA_H
