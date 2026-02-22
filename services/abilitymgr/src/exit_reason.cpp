/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#include "exit_reason.h"

#include "hilog_tag_wrapper.h"
#include "parcel_macro_base.h"
#include "string_ex.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr size_t MAX_EXIT_MSG_LENGTH = 128;

std::string TruncateExitMsg(const std::string& msg)
{
    if (msg.length() <= MAX_EXIT_MSG_LENGTH) {
        return msg;
    }
    return msg.substr(0, MAX_EXIT_MSG_LENGTH);
}
}

ExitReason::ExitReason(const Reason reason, const std::string &exitMsg)
{
    this->reason = reason;
    this->exitMsg = exitMsg;
}

ExitReason::ExitReason(const Reason &reason, int32_t subReason, const std::string &exitMsg)
{
    this->reason = reason;
    this->subReason = subReason;
    this->exitMsg = exitMsg;
}

bool ExitReason::ReadFromParcel(Parcel &parcel)
{
    int32_t reasonData;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, reasonData);
    reason = static_cast<Reason>(reasonData);
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, reasonData);
    subReason = reasonData;
    exitMsg = Str16ToStr8(parcel.ReadString16());
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Bool, parcel, shouldKillForeground);
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Bool, parcel, shouldSkipKillInStartup);
    return true;
}

ExitReason *ExitReason::Unmarshalling(Parcel &parcel)
{
    ExitReason *data = new (std::nothrow) ExitReason();
    if (!data) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null data");
        return nullptr;
    }
    if (!data->ReadFromParcel(parcel)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "read failed");
        delete data;
        data = nullptr;
    }
    return data;
}

bool ExitReason::Marshalling(Parcel &parcel) const
{
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, static_cast<int32_t>(reason));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, subReason);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(exitMsg));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Bool, parcel, shouldKillForeground);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Bool, parcel, shouldSkipKillInStartup);
    return true;
}

ExitReasonCompability::ExitReasonCompability(const Reason reason, const std::string &exitMsg)
{
    this->reason = reason;
    this->exitMsg = exitMsg;
}

ExitReasonCompability::ExitReasonCompability(const Reason &reason, int32_t subReason, const std::string &exitMsg)
{
    this->reason = reason;
    this->subReason = subReason;
    this->exitMsg = exitMsg;
}

bool ExitReasonCompability::ReadFromParcel(Parcel &parcel)
{
    int32_t reasonData;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, reasonData);
    reason = static_cast<Reason>(reasonData);
    exitMsg = Str16ToStr8(parcel.ReadString16());
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, subReason);
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, killId);
    killMsg = Str16ToStr8(parcel.ReadString16());
    innerMsg = Str16ToStr8(parcel.ReadString16());
    return true;
}

ExitReasonCompability *ExitReasonCompability::Unmarshalling(Parcel &parcel)
{
    ExitReasonCompability *data = new (std::nothrow) ExitReasonCompability();
    if (!data) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null data");
        return nullptr;
    }
    if (!data->ReadFromParcel(parcel)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "read failed");
        delete data;
        data = nullptr;
    }
    return data;
}

bool ExitReasonCompability::Marshalling(Parcel &parcel) const
{
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, static_cast<int32_t>(reason));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(TruncateExitMsg(exitMsg)));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, subReason);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, killId);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(killMsg));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(innerMsg));
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
