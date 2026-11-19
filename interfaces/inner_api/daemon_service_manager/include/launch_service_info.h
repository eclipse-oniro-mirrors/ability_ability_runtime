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

#ifndef OHOS_ABILITY_RUNTIME_LAUNCH_SERVICE_INFO_H
#define OHOS_ABILITY_RUNTIME_LAUNCH_SERVICE_INFO_H

#include <cstdint>
#include <string>

#include "parcel.h"

namespace OHOS {
namespace AbilityRuntime {
enum class LaunchServiceState : int32_t {
    NOT_REGISTERED = 0,
    ENABLED = 1,
    DISABLED = 2,
};

enum class LaunchServiceProcessState : int32_t {
    CREATED = 0,
    TERMINATED = 1,
};

struct LaunchServiceDescriptor : public Parcelable {
    std::string bundleName;
    std::string moduleName;
    std::string serviceName;
    int32_t userId = 0;
    int32_t appIndex = 0;

    bool ReadFromParcel(Parcel &parcel);
    bool Marshalling(Parcel &parcel) const override;
    bool IsValid() const;
    static LaunchServiceDescriptor *Unmarshalling(Parcel &parcel);
};

struct LaunchServiceInformation : public Parcelable {
    LaunchServiceDescriptor descriptor;
    LaunchServiceState state = LaunchServiceState::NOT_REGISTERED;
    int32_t pid = 0;

    bool ReadFromParcel(Parcel &parcel);
    bool Marshalling(Parcel &parcel) const override;
    static LaunchServiceInformation *Unmarshalling(Parcel &parcel);
};

struct LaunchServiceStateData : public Parcelable {
    LaunchServiceDescriptor descriptor;
    LaunchServiceState state = LaunchServiceState::NOT_REGISTERED;

    bool ReadFromParcel(Parcel &parcel);
    bool Marshalling(Parcel &parcel) const override;
    static LaunchServiceStateData *Unmarshalling(Parcel &parcel);
};

struct LaunchServiceProcessStateData : public Parcelable {
    LaunchServiceDescriptor descriptor;
    int32_t pid = 0;
    LaunchServiceProcessState state = LaunchServiceProcessState::CREATED;

    bool ReadFromParcel(Parcel &parcel);
    bool Marshalling(Parcel &parcel) const override;
    static LaunchServiceProcessStateData *Unmarshalling(Parcel &parcel);
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_LAUNCH_SERVICE_INFO_H
