/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "process_options.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
bool ProcessOptions::ReadFromParcel(Parcel &parcel)
{
    processMode = static_cast<ProcessMode>(parcel.ReadInt32());
    startupVisibility = static_cast<StartupVisibility>(parcel.ReadInt32());
    processName = parcel.ReadString();
    return true;
}

ProcessOptions *ProcessOptions::Unmarshalling(Parcel &parcel)
{
    ProcessOptions *option = new (std::nothrow) ProcessOptions();
    if (option == nullptr) {
        return nullptr;
    }

    if (!option->ReadFromParcel(parcel)) {
        delete option;
        option = nullptr;
    }

    return option;
}

bool ProcessOptions::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(static_cast<int32_t>(processMode))) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ProcessMode write failed");
        return false;
    }
    if (!parcel.WriteInt32(static_cast<int32_t>(startupVisibility))) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "StartupVisibility write failed");
        return false;
    }
    if (!parcel.WriteString(processName)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ProcessName write failed");
        return false;
    }
    return true;
}

ProcessMode ProcessOptions::ConvertInt32ToProcessMode(int32_t value)
{
    if (value <= static_cast<int32_t>(ProcessMode::UNSPECIFIED) ||
        value >= static_cast<int32_t>(ProcessMode::END)) {
        return ProcessMode::UNSPECIFIED;
    }
    return static_cast<ProcessMode>(value);
}

//   enum ProcessMode {
//     NEW_PROCESS_ATTACH_TO_PARENT = 1,
//     NEW_PROCESS_ATTACH_TO_STATUS_BAR_ITEM = 2,
//     ATTACH_TO_STATUS_BAR_ITEM = 3
//   }
ProcessMode ProcessOptions::ConvertStsToProcessMode(int32_t index)
{
    if (index < 0 || index > 2) {
        return ProcessMode::UNSPECIFIED;
    }
    return static_cast<ProcessMode>(index + 1);
}
int32_t ProcessOptions::ConvertProcessModeToSts(const ProcessMode mode)
{
    int32_t value = static_cast<int32_t>(mode);
    if (value >= 1 && value <= 3) {
        return value - 1;
    }
    return static_cast<int32_t>(ProcessMode::NEW_PROCESS_ATTACH_TO_PARENT);
}

StartupVisibility ProcessOptions::ConvertInt32ToStartupVisibility(int32_t value)
{
    if (value <= static_cast<int32_t>(StartupVisibility::UNSPECIFIED) ||
        value >= static_cast<int32_t>(StartupVisibility::END)) {
        return StartupVisibility::UNSPECIFIED;
    }
    return static_cast<StartupVisibility>(value);
}
// enum StartupVisibility {
//     STARTUP_HIDE = 0,
//     STARTUP_SHOW = 1
//   }StartupVisibility
StartupVisibility ConvertStsToStartupVisibility(int32_t index)
{
    if (index < 0 || index > 1) {
        return StartupVisibility::UNSPECIFIED;
    }
    return static_cast<StartupVisibility>(index);
}
int32_t ConvertStartupVisibilityToSts(const StartupVisibility value)
{
    if (value == StartupVisibility::STARTUP_HIDE) {
        return 0;
    } else if (value == StartupVisibility::STARTUP_SHOW) {
        return 1;
    }

    return 0;
}

bool ProcessOptions::IsNewProcessMode(ProcessMode value)
{
    return (value == ProcessMode::NEW_PROCESS_ATTACH_TO_PARENT) ||
        (value == ProcessMode::NEW_PROCESS_ATTACH_TO_STATUS_BAR_ITEM);
}

bool ProcessOptions::IsAttachToStatusBarMode(ProcessMode value)
{
    return (value == ProcessMode::NEW_PROCESS_ATTACH_TO_STATUS_BAR_ITEM) ||
        (value == ProcessMode::ATTACH_TO_STATUS_BAR_ITEM);
}

bool ProcessOptions::IsValidProcessMode(ProcessMode value)
{
    return (value > ProcessMode::UNSPECIFIED) && (value < ProcessMode::END);
}

bool ProcessOptions::IsNoAttachmentMode(ProcessMode value)
{
    return (value == ProcessMode::NO_ATTACHMENT);
}

bool ProcessOptions::IsAttachToStatusBarItemMode(ProcessMode value)
{
    return (value == ProcessMode::ATTACH_TO_STATUS_BAR_ITEM);
}
}  // namespace AAFwk
}  // namespace OHOS
