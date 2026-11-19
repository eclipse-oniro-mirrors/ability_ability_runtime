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

#include "launch_service_info.h"

#include <memory>

#include "daemon_service_info.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
bool IsValidState(int32_t state)
{
    return state >= static_cast<int32_t>(LaunchServiceState::NOT_REGISTERED) &&
        state <= static_cast<int32_t>(LaunchServiceState::DISABLED);
}

bool IsValidProcessState(int32_t state)
{
    return state >= static_cast<int32_t>(LaunchServiceProcessState::CREATED) &&
        state <= static_cast<int32_t>(LaunchServiceProcessState::TERMINATED);
}
} // namespace

bool LaunchServiceDescriptor::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadString(bundleName) || !parcel.ReadString(moduleName) || !parcel.ReadString(serviceName) ||
        !parcel.ReadInt32(userId) || !parcel.ReadInt32(appIndex)) {
        return false;
    }
    return IsValid();
}

bool LaunchServiceDescriptor::Marshalling(Parcel &parcel) const
{
    return IsValid() && parcel.WriteString(bundleName) && parcel.WriteString(moduleName) &&
        parcel.WriteString(serviceName) && parcel.WriteInt32(userId) && parcel.WriteInt32(appIndex);
}

bool LaunchServiceDescriptor::IsValid() const
{
    return (userId >= 0) &&
        AAFwk::LaunchServiceDescriptor::IsValidIdentity(
            bundleName, moduleName, serviceName, userId, appIndex);
}

LaunchServiceDescriptor *LaunchServiceDescriptor::Unmarshalling(Parcel &parcel)
{
    auto descriptor = std::make_unique<LaunchServiceDescriptor>();
    if (!descriptor->ReadFromParcel(parcel)) {
        return nullptr;
    }
    return descriptor.release();
}

bool LaunchServiceInformation::ReadFromParcel(Parcel &parcel)
{
    std::unique_ptr<LaunchServiceDescriptor> descriptorValue(parcel.ReadParcelable<LaunchServiceDescriptor>());
    int32_t stateValue = 0;
    if (descriptorValue == nullptr || !parcel.ReadInt32(stateValue) || !IsValidState(stateValue) ||
        !parcel.ReadInt32(pid) || pid < 0) {
        return false;
    }
    descriptor = *descriptorValue;
    state = static_cast<LaunchServiceState>(stateValue);
    return state != LaunchServiceState::NOT_REGISTERED || pid == 0;
}

bool LaunchServiceInformation::Marshalling(Parcel &parcel) const
{
    if (!IsValidState(static_cast<int32_t>(state)) || pid < 0 ||
        (state == LaunchServiceState::NOT_REGISTERED && pid != 0)) {
        return false;
    }
    return parcel.WriteParcelable(&descriptor) && parcel.WriteInt32(static_cast<int32_t>(state)) &&
        parcel.WriteInt32(pid);
}

LaunchServiceInformation *LaunchServiceInformation::Unmarshalling(Parcel &parcel)
{
    auto information = std::make_unique<LaunchServiceInformation>();
    if (!information->ReadFromParcel(parcel)) {
        return nullptr;
    }
    return information.release();
}

bool LaunchServiceStateData::ReadFromParcel(Parcel &parcel)
{
    std::unique_ptr<LaunchServiceDescriptor> descriptorValue(parcel.ReadParcelable<LaunchServiceDescriptor>());
    int32_t stateValue = 0;
    if (descriptorValue == nullptr || !parcel.ReadInt32(stateValue) || !IsValidState(stateValue)) {
        return false;
    }
    descriptor = *descriptorValue;
    state = static_cast<LaunchServiceState>(stateValue);
    return true;
}

bool LaunchServiceStateData::Marshalling(Parcel &parcel) const
{
    return descriptor.IsValid() && IsValidState(static_cast<int32_t>(state)) && parcel.WriteParcelable(&descriptor) &&
        parcel.WriteInt32(static_cast<int32_t>(state));
}

LaunchServiceStateData *LaunchServiceStateData::Unmarshalling(Parcel &parcel)
{
    auto data = std::make_unique<LaunchServiceStateData>();
    return data->ReadFromParcel(parcel) ? data.release() : nullptr;
}

bool LaunchServiceProcessStateData::ReadFromParcel(Parcel &parcel)
{
    std::unique_ptr<LaunchServiceDescriptor> descriptorValue(parcel.ReadParcelable<LaunchServiceDescriptor>());
    int32_t stateValue = 0;
    if (descriptorValue == nullptr || !parcel.ReadInt32(pid) || pid < 0 || !parcel.ReadInt32(stateValue) ||
        !IsValidProcessState(stateValue)) {
        return false;
    }
    descriptor = *descriptorValue;
    state = static_cast<LaunchServiceProcessState>(stateValue);
    return true;
}

bool LaunchServiceProcessStateData::Marshalling(Parcel &parcel) const
{
    return descriptor.IsValid() && pid >= 0 && IsValidProcessState(static_cast<int32_t>(state)) &&
        parcel.WriteParcelable(&descriptor) && parcel.WriteInt32(pid) && parcel.WriteInt32(static_cast<int32_t>(state));
}

LaunchServiceProcessStateData *LaunchServiceProcessStateData::Unmarshalling(Parcel &parcel)
{
    auto data = std::make_unique<LaunchServiceProcessStateData>();
    return data->ReadFromParcel(parcel) ? data.release() : nullptr;
}

} // namespace AbilityRuntime
} // namespace OHOS
