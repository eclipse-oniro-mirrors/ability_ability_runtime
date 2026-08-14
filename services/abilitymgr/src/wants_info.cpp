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

#include "wants_info.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
bool WantsInfo::ReadFromParcel(Parcel &parcel)
{
    std::unique_ptr<Want> wantResquest(parcel.ReadParcelable<Want>());
    if (wantResquest == nullptr) {
        return false;
    }
    want = *wantResquest;

    resolvedTypes = Str16ToStr8(parcel.ReadString16());
    return true;
}

WantsInfo *WantsInfo::Unmarshalling(Parcel &parcel)
{
    WantsInfo *info = new (std::nothrow) WantsInfo();
    if (info == nullptr) {
        return nullptr;
    }

    if (!info->ReadFromParcel(parcel)) {
        delete info;
        info = nullptr;
    }
    return info;
}

bool WantsInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteParcelable(&want)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write want failed");
        return false;
    }
    if (!parcel.WriteString16(Str8ToStr16(resolvedTypes))) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write resolvedTypes failed");
        return false;
    }
    return true;
}
}  // namespace AAFwk
}  // namespace OHOS
