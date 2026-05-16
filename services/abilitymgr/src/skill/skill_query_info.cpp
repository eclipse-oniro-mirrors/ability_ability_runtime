/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "skill_query_info.h"

namespace OHOS {
namespace AppExecFwk {

bool SkillQueryInfo::ReadFromParcel(Parcel &parcel)
{
    bundleName = Str16ToStr8(parcel.ReadString16());
    moduleName = Str16ToStr8(parcel.ReadString16());
    skillName = Str16ToStr8(parcel.ReadString16());
    abilityName = Str16ToStr8(parcel.ReadString16());
    type = parcel.ReadInt32();

    int32_t srcCount = parcel.ReadInt32();
    for (int32_t i = 0; i < srcCount; i++) {
        srcEntries.push_back(Str16ToStr8(parcel.ReadString16()));
    }

    int32_t permCount = parcel.ReadInt32();
    for (int32_t i = 0; i < permCount; i++) {
        permissions.push_back(Str16ToStr8(parcel.ReadString16()));
    }
    return true;
}

bool SkillQueryInfo::Marshalling(Parcel &parcel) const
{
    parcel.WriteString16(Str8ToStr16(bundleName));
    parcel.WriteString16(Str8ToStr16(moduleName));
    parcel.WriteString16(Str8ToStr16(skillName));
    parcel.WriteString16(Str8ToStr16(abilityName));
    parcel.WriteInt32(type);

    parcel.WriteInt32(static_cast<int32_t>(srcEntries.size()));
    for (const auto &entry : srcEntries) {
        parcel.WriteString16(Str8ToStr16(entry));
    }

    parcel.WriteInt32(static_cast<int32_t>(permissions.size()));
    for (const auto &perm : permissions) {
        parcel.WriteString16(Str8ToStr16(perm));
    }
    return true;
}

SkillQueryInfo *SkillQueryInfo::Unmarshalling(Parcel &parcel)
{
    auto *info = new (std::nothrow) SkillQueryInfo();
    if (info == nullptr) {
        return nullptr;
    }
    if (!info->ReadFromParcel(parcel)) {
        delete info;
        return nullptr;
    }
    return info;
}

} // namespace AppExecFwk
} // namespace OHOS
