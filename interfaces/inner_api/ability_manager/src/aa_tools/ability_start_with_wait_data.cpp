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

#include "ability_start_with_wait_observer_data.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
bool AbilityStartWithWaitObserverData::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteBool(coldStart)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write coldStart error");
        return false;
    }
    if (!parcel.WriteUint32(reason)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write reason error");
        return false;
    }
    if (!parcel.WriteInt64(startTime)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write startTime error");
        return false;
    }
    if (!parcel.WriteInt64(foregroundTime)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write foregroundTime error");
        return false;
    }
    if (!parcel.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write bundleName error");
        return false;
    }
    if (!parcel.WriteString(abilityName)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write abilityName error");
        return false;
    }
    return true;
}

bool AbilityStartWithWaitObserverData::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadBool(coldStart)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Read coldStart error");
        return false;
    }
    if (!parcel.ReadUint32(reason)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Read reason error");
        return false;
    }
    if (!parcel.ReadInt64(startTime)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Read startTime error");
        return false;
    }
    if (!parcel.ReadInt64(foregroundTime)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Read foregroundTime error");
        return false;
    }
    if (!parcel.ReadString(bundleName)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Read bundleName error");
        return false;
    }
    if (!parcel.ReadString(abilityName)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Read abilityName error");
        return false;
    }
    return true;
}

AbilityStartWithWaitObserverData *AbilityStartWithWaitObserverData::Unmarshalling(Parcel &parcel)
{
    AbilityStartWithWaitObserverData *abilityStartWithWaitData = new (std::nothrow) AbilityStartWithWaitObserverData();
    if (abilityStartWithWaitData && !abilityStartWithWaitData->ReadFromParcel(parcel)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ReadFromParcel failed");
        delete abilityStartWithWaitData;
        abilityStartWithWaitData = nullptr;
    }
    return abilityStartWithWaitData;
}
} // namespace AAFwk
} // namespace OHOS