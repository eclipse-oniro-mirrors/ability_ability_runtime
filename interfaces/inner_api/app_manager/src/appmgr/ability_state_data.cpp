/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "ability_state_data.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
bool AbilityStateData::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(moduleName)) {
        return false;
    }
    if (!parcel.WriteString(bundleName)) {
        return false;
    }
    if (!parcel.WriteString(abilityName)) {
        return false;
    }
    if (!parcel.WriteInt32(abilityState)) {
        return false;
    }
    if (!parcel.WriteInt32(pid)) {
        return false;
    }
    if (!parcel.WriteInt32(uid)) {
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
        if (!parcel.WriteRemoteObject(token)) {
            return false;
        }
    }
    if (!MarshallingOne(parcel)) {
        return false;
    }
    return true;
}

bool AbilityStateData::MarshallingOne(Parcel &parcel) const
{
    if (!parcel.WriteInt32(abilityType)) {
        return false;
    }
    if (!parcel.WriteBool(isFocused)) {
        return false;
    }
    if (!parcel.WriteString(callerBundleName)) {
        return false;
    }
    if (!parcel.WriteString(callerAbilityName)) {
        return false;
    }
    if (!parcel.WriteBool(isAtomicService) || !parcel.WriteInt32(abilityRecordId)) {
        return false;
    }
    if (!parcel.WriteInt32(appCloneIndex)) {
        return false;
    }
    if (!parcel.WriteInt32(extensionAbilityType)) {
        return false;
    }
    if (!parcel.WriteInt32(processType)) {
        return false;
    }
    if (!parcel.WriteInt32(callerUid)) {
        return false;
    }
    if (!parcel.WriteBool(isInnerNotify)) {
        return false;
    }
    if (!parcel.WriteInt32(callerPid)) {
        return false;
    }
    if (!parcel.WriteInt32(preloadMode)) {
        return false;
    }
    if (!parcel.WriteInt32(lastUIAbilityCallerUid)) {
        return false;
    }
    if (!parcel.WriteString(lastUIAbilityCallerName)) {
        return false;
    }
    return true;
}

bool AbilityStateData::ReadFromParcel(Parcel &parcel)
{
    moduleName = parcel.ReadString();

    bundleName = parcel.ReadString();

    abilityName = parcel.ReadString();

    abilityState = parcel.ReadInt32();

    pid = parcel.ReadInt32();

    uid = parcel.ReadInt32();

    if (parcel.ReadBool()) {
        token = (static_cast<MessageParcel*>(&parcel))->ReadRemoteObject();
    }

    abilityType = parcel.ReadInt32();

    isFocused = parcel.ReadBool();

    callerBundleName = parcel.ReadString();

    callerAbilityName = parcel.ReadString();
    isAtomicService = parcel.ReadBool();
    abilityRecordId = parcel.ReadInt32();
    appCloneIndex = parcel.ReadInt32();
    extensionAbilityType = parcel.ReadInt32();
    processType = parcel.ReadInt32();
    callerUid = parcel.ReadInt32();
    isInnerNotify = parcel.ReadBool();
    callerPid = parcel.ReadInt32();
    preloadMode = parcel.ReadInt32();
    lastUIAbilityCallerUid = parcel.ReadInt32();
    lastUIAbilityCallerName = parcel.ReadString();
    return true;
}

AbilityStateData *AbilityStateData::Unmarshalling(Parcel &parcel)
{
    AbilityStateData *abilityStateData = new (std::nothrow) AbilityStateData();
    if (abilityStateData && !abilityStateData->ReadFromParcel(parcel)) {
        TAG_LOGW(AAFwkTag::APPMGR, "ReadFromParcel failed");
        delete abilityStateData;
        abilityStateData = nullptr;
    }
    return abilityStateData;
}
}  // namespace AppExecFwk
}  // namespace OHOS
