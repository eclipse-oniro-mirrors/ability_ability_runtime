/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifdef SUPPORT_GRAPHICS
#include "ability_first_frame_state_data.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
bool AbilityFirstFrameStateData::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(bundleName)) {
        return false;
    }
    if (!parcel.WriteString(moduleName)) {
        return false;
    }
    if (!parcel.WriteString(abilityName)) {
        return false;
    }
    if (!parcel.WriteInt32(appIndex)) {
        return false;
    }
    if (!parcel.WriteBool(coldStart)) {
        return false;
    }
    return true;
}

bool AbilityFirstFrameStateData::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadString(bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Read bundleName error");
        return false;
    }
    if (!parcel.ReadString(moduleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Read moduleName error");
        return false;
    }
    if (!parcel.ReadString(abilityName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Read abilityName error");
        return false;
    }
    if (!parcel.ReadInt32(appIndex)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Read appIndex error");
        return false;
    }
    if (!parcel.ReadBool(coldStart)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Read coldStart error");
        return false;
    }
    return true;
}

AbilityFirstFrameStateData *AbilityFirstFrameStateData::Unmarshalling(Parcel &parcel)
{
    AbilityFirstFrameStateData *abilityFirstFrameStateData = new (std::nothrow) AbilityFirstFrameStateData();
    if (abilityFirstFrameStateData && !abilityFirstFrameStateData->ReadFromParcel(parcel)) {
        TAG_LOGW(AAFwkTag::APPMGR, "ReadFromParcel failed");
        delete abilityFirstFrameStateData;
        abilityFirstFrameStateData = nullptr;
    }
    return abilityFirstFrameStateData;
}
}  // namespace AppExecFwk
}  // namespace OHOS
#endif