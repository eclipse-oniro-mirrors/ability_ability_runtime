/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_MISSION_INFO_H
#define OHOS_ABILITY_RUNTIME_MISSION_INFO_H

#include <string>

#include "parcel.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {

enum ContinueState {
    CONTINUESTATE_UNKNOWN = -1,
    CONTINUESTATE_ACTIVE = 0,
    CONTINUESTATE_INACTIVE = 1,
    CONTINUESTATE_MAX
};

/**
 * @struct MissionInfo
 * MissionInfo is used to save information about mission information.
 */
struct MissionInfo : public Parcelable {
    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static MissionInfo *Unmarshalling(Parcel &parcel);

    bool lockedState = false;
    bool continuable = false;
    bool unclearable = false;
    ContinueState continueState = ContinueState::CONTINUESTATE_ACTIVE;
    int32_t id = -1;
    int32_t runningState = -1;
    int32_t abilityState = -1;
    std::string time;
    std::string label;
    std::string iconPath;
    Want want;
};

/**
 * @struct MissionValidResult
 * Mission is valid result.
 */
struct MissionValidResult : public Parcelable {
    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static MissionValidResult *Unmarshalling(Parcel &parcel);

    int32_t missionId = -1;
    bool isValid = false;
};
// enum ContinueState {
//     ACTIVE = 0,
//     INACTIVE = 1
//   }
[[maybe_unused]] static ContinueState ContinueState_ConvertStsToNative(const int32_t index);
[[maybe_unused]] static  int32_t ContinueState_ConvertNativeToSts(const ContinueState index);
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_MISSION_INFO_H
