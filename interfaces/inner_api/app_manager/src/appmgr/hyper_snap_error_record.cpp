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

#include "hyper_snap_error_record.h"

#include <new>

#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {

bool HyperSnapErrorRecord::ReadFromParcel(Parcel &parcel)
{
    int32_t codeValue = 0;
    if (!parcel.ReadInt32(codeValue)) {
        return false;
    }
    code = static_cast<HyperSnapErrorCode>(codeValue);
    msg = Str16ToStr8(parcel.ReadString16());
    if (!parcel.ReadInt64(occurTimeStamp)) {
        return false;
    }
    return true;
}

bool HyperSnapErrorRecord::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(static_cast<int32_t>(code)) ||
        !parcel.WriteString16(Str8ToStr16(msg)) ||
        !parcel.WriteInt64(occurTimeStamp)) {
        return false;
    }
    return true;
}

HyperSnapErrorRecord *HyperSnapErrorRecord::Unmarshalling(Parcel &parcel)
{
    auto *record = new (std::nothrow) HyperSnapErrorRecord();
    if (record != nullptr && !record->ReadFromParcel(parcel)) {
        delete record;
        record = nullptr;
    }
    return record;
}

} // namespace AppExecFwk
} // namespace OHOS
