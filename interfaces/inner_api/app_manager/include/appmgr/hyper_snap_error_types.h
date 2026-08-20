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

#ifndef OHOS_ABILITY_RUNTIME_HYPER_SNAP_ERROR_TYPES_H
#define OHOS_ABILITY_RUNTIME_HYPER_SNAP_ERROR_TYPES_H

#include <new>
#include <string>

#include "parcel.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {

enum class HyperSnapErrorCode {
    ERR_OK = 0,
    ERR_SYSTEM_INNER = 1,
    ERR_SNAPSHOT_EXIST = 2,
    ERR_PROCESS_IS_RUNNING = 3,
    ERR_SNAPSHOT_PROCESS_IS_DIED = 4,
    ERR_SNAPSHOT_IS_INTERRUPTED = 5,
    ERR_EXISTS_ILLEGAL_BINDER = 6,
    ERR_LAST_PROCESS_NOT_FULLY_EXITED = 7,
};

enum class HyperSnapErrorType {
    CREATE_SNAPSHOT = 0,
    FORK_FROM_SNAPSHOT = 1,
};

struct HyperSnapErrorRecord : public Parcelable {
    HyperSnapErrorCode code = HyperSnapErrorCode::ERR_OK;
    std::string msg;
    std::string occurTimeStamp;

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static HyperSnapErrorRecord *Unmarshalling(Parcel &parcel);
};

inline bool HyperSnapErrorRecord::ReadFromParcel(Parcel &parcel)
{
    int32_t codeValue = 0;
    if (!parcel.ReadInt32(codeValue)) {
        return false;
    }
    code = static_cast<HyperSnapErrorCode>(codeValue);
    msg = Str16ToStr8(parcel.ReadString16());
    occurTimeStamp = Str16ToStr8(parcel.ReadString16());
    return true;
}

inline bool HyperSnapErrorRecord::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(static_cast<int32_t>(code)) ||
        !parcel.WriteString16(Str8ToStr16(msg)) ||
        !parcel.WriteString16(Str8ToStr16(occurTimeStamp))) {
        return false;
    }
    return true;
}

inline HyperSnapErrorRecord *HyperSnapErrorRecord::Unmarshalling(Parcel &parcel)
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

#endif // OHOS_ABILITY_RUNTIME_HYPER_SNAP_ERROR_TYPES_H
