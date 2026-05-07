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

#include "param.h"

namespace OHOS::AbilityRuntime {
bool LoadParam::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(abilityRecordId)) {
        return false;
    }
    if (!parcel.WriteBool(isShellCall)) {
        return false;
    }
    if (!parcel.WriteString(instanceKey)) {
        return false;
    }
    if (token == nullptr) {
        if (!parcel.WriteBool(false)) {
            return false;
        }
    } else {
        if (!parcel.WriteBool(true)) {
            return false;
        }
        if (!(static_cast<MessageParcel*>(&parcel))->WriteRemoteObject(token)) {
            return false;
        }
    }
    if (preToken == nullptr) {
        if (!parcel.WriteBool(false)) {
            return false;
        }
    } else {
        if (!parcel.WriteBool(true)) {
            return false;
        }
        if (!(static_cast<MessageParcel*>(&parcel))->WriteRemoteObject(preToken)) {
            return false;
        }
    }
    if (!parcel.WriteBool(isKeepAlive)) {
        return false;
    }
    if (!parcel.WriteBool(isKeepAliveAppService)) {
        return false;
    }
    if (!parcel.WriteUint32(extensionProcessMode)) {
        return false;
    }
    if (!parcel.WriteParcelable(&extensionLoadParam)) {
        return false;
    }
    if (!parcel.WriteBool(isStartupHide)) {
        return false;
    }
    return MarshallingTwo(parcel);
}

bool LoadParam::MarshallingTwo(Parcel &parcel) const
{
    if (!parcel.WriteBool(isMainElementRunning)) {
        return false;
    }
    if (!parcel.WriteInt32(callingPid)) {
        return false;
    }
    if (!parcel.WriteUint64(loadAbilityCallbackId)) {
        return false;
    }
    if (!parcel.WriteBool(isPrelaunch)) {
        return false;
    }
    if (!parcel.WriteBool(isPreloadStart)) {
        return false;
    }
    if (!parcel.WriteInt32(loadTimeout)) {
        return false;
    }
    if (!parcel.WriteInt32(byCallStatus)) {
        return false;
    }
    return true;
}

bool LoadParam::ReadFromParcel(Parcel &parcel)
{
    abilityRecordId = parcel.ReadInt32();
    isShellCall = parcel.ReadBool();
    instanceKey = parcel.ReadString();
    if (parcel.ReadBool()) {
        token = (static_cast<MessageParcel*>(&parcel))->ReadRemoteObject();
        if (token == nullptr) {
            return false;
        }
    }
    if (parcel.ReadBool()) {
        preToken = (static_cast<MessageParcel*>(&parcel))->ReadRemoteObject();
        if (preToken == nullptr) {
            return false;
        }
    }
    isKeepAlive = parcel.ReadBool();
    isKeepAliveAppService = parcel.ReadBool();
    extensionProcessMode = parcel.ReadUint32();
    std::unique_ptr<ExtensionLoadParam> extensionParamRead(parcel.ReadParcelable<ExtensionLoadParam>());
    if (!extensionParamRead) {
        return false;
    }
    extensionLoadParam = *extensionParamRead;
    isStartupHide = parcel.ReadBool();
    isMainElementRunning = parcel.ReadBool();
    callingPid = parcel.ReadInt32();
    loadAbilityCallbackId = parcel.ReadUint64();
    isPrelaunch = parcel.ReadBool();
    isPreloadStart = parcel.ReadBool();
    loadTimeout = parcel.ReadInt32();
    byCallStatus = parcel.ReadInt32();
    return true;
}

LoadParam *LoadParam::Unmarshalling(Parcel &parcel)
{
    LoadParam *loadParam = new (std::nothrow) LoadParam();
    if (loadParam && !loadParam->ReadFromParcel(parcel)) {
        delete loadParam;
        loadParam = nullptr;
    }
    return loadParam;
}
}