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

#ifndef OHOS_ABILITY_RUNTIME_EXIT_REASON_H
#define OHOS_ABILITY_RUNTIME_EXIT_REASON_H

#include <string>

#include "ability_state.h"
#include "parcel.h"

namespace OHOS {
namespace AAFwk {
struct ExitReason : public Parcelable {
    ExitReason() = default;
    ExitReason(const Reason reason, const std::string &exitMsg);
    ExitReason(const Reason &reason, int32_t subReason, const std::string &exitMsg);

    Reason reason = Reason::REASON_UNKNOWN;
    int32_t subReason = -1;
    std::string exitMsg = "";
    bool shouldKillForeground = true;
    bool shouldSkipKillInStartup = false; // set true in low-memory, return ERR_KILL_APP_WHILE_STARTING and do not kill

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static ExitReason *Unmarshalling(Parcel &parcel);
};

struct ExitReasonCompability : public Parcelable {
    ExitReasonCompability() = default;
    ExitReasonCompability(const Reason reason, const std::string &exitMsg);
    ExitReasonCompability(const Reason &reason, int32_t subReason, const std::string &exitMsg);

    Reason reason = Reason::REASON_UNKNOWN;
    std::string exitMsg = "";
    int32_t subReason = -1;

    int32_t killId = -1;
    std::string killMsg = "";
    std::string innerMsg = "";

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static ExitReasonCompability *Unmarshalling(Parcel &parcel);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_EXIT_REASON_H
