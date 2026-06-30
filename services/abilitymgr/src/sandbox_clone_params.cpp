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

#include "sandbox_clone_params.h"

namespace OHOS {
namespace AAFwk {
bool SandboxCloneParams::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadString(callerBundleName)) {
        return false;
    }
    if (!parcel.ReadInt32(callerUid)) {
        return false;
    }
    if (!parcel.ReadUint32(callerTokenId)) {
        return false;
    }
    return true;
}

SandboxCloneParams *SandboxCloneParams::Unmarshalling(Parcel &parcel)
{
    SandboxCloneParams *params = new (std::nothrow) SandboxCloneParams();
    if (params == nullptr) {
        return nullptr;
    }

    if (!params->ReadFromParcel(parcel)) {
        delete params;
        params = nullptr;
    }
    return params;
}

bool SandboxCloneParams::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(callerBundleName)) {
        return false;
    }
    if (!parcel.WriteInt32(callerUid)) {
        return false;
    }
    if (!parcel.WriteUint32(callerTokenId)) {
        return false;
    }
    return true;
}
}  // namespace AAFwk
}  // namespace OHOS
