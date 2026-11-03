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

#include "hilog_tag_wrapper.h"
#include "kiosk_status.h"
#include "string_ex.h"

namespace OHOS {
namespace AAFwk {
bool KioskStatus::ReadFromParcel(Parcel &parcel)
{
    isKioskMode_ = parcel.ReadBool();
    kioskBundleName_ = Str16ToStr8(parcel.ReadString16());
    kioskBundleUid_ = parcel.ReadInt32();
    kioskType_ = parcel.ReadInt32();
    return true;
}

KioskStatus *KioskStatus::Unmarshalling(Parcel &parcel)
{
    KioskStatus *info = new KioskStatus();
    if (!info->ReadFromParcel(parcel)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ReadFromParcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}

bool KioskStatus::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteBool(isKioskMode_)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write isKioskMode_ failed");
        return false;
    }

    if (!parcel.WriteString16(Str8ToStr16(kioskBundleName_))) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write kioskBundleName_ failed");
        return false;
    }

    if (!parcel.WriteInt32(kioskBundleUid_)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write kioskBundleUid_ failed");
        return false;
    }

    if (!parcel.WriteInt32(kioskType_)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write kioskType_ failed");
        return false;
    }
    return true;
}

void KioskStatus::Clear()
{
    isKioskMode_ = false;
    kioskBundleUid_ = 0;
    kioskBundleName_.clear();
    kioskToken_ = nullptr;
    kioskType_ = 0; // Rosen::KioskType::DEFAULT
    kioskCallerUid_ = 0;
    isProxyEnter_ = false;
}
} // namespace AAFwk
} // namespace OHOS
