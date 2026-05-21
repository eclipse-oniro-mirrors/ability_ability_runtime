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

#include "skill_execute_result.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t MAX_URI_COUNT = 500;
} // namespace

bool SkillExecuteResult::ReadFromParcel(Parcel &parcel)
{
    code = parcel.ReadInt32();
    auto wantParams = parcel.ReadParcelable<WantParams>();
    if (wantParams == nullptr) {
        return false;
    }
    result = std::shared_ptr<WantParams>(wantParams);
    int32_t uriCount = parcel.ReadInt32();
    if (uriCount < 0 || uriCount > MAX_URI_COUNT) {
        return false;
    }
    for (int32_t i = 0; i < uriCount; i++) {
        uris.push_back(Str16ToStr8(parcel.ReadString16()));
    }
    flags = parcel.ReadUint32();
    return true;
}

bool SkillExecuteResult::Marshalling(Parcel &parcel) const
{
    parcel.WriteInt32(code);
    parcel.WriteParcelable(result.get());
    parcel.WriteInt32(static_cast<int32_t>(uris.size()));
    for (const auto &uri : uris) {
        parcel.WriteString16(Str8ToStr16(uri));
    }
    parcel.WriteUint32(flags);
    return true;
}

SkillExecuteResult *SkillExecuteResult::Unmarshalling(Parcel &parcel)
{
    auto *res = new (std::nothrow) SkillExecuteResult();
    if (res == nullptr) {
        return nullptr;
    }
    if (!res->ReadFromParcel(parcel)) {
        delete res;
        return nullptr;
    }
    return res;
}

} // namespace AppExecFwk
} // namespace OHOS
