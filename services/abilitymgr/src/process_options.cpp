/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include <cinttypes>

#include "process_options.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
bool ProcessOptions::ReadFromParcel(Parcel &parcel)
{
    processMode = ConvertInt32ToProcessMode(parcel.ReadInt32());
    startupVisibility = ConvertInt32ToStartupVisibility(parcel.ReadInt32());
    isRestartKeepAlive = parcel.ReadBool();
    isStartFromNDK = parcel.ReadBool();
    isPreloadStart = parcel.ReadBool();
    loadAbilityCallbackId = parcel.ReadUint64();
    callingPid = parcel.ReadInt32();
    selfPid = parcel.ReadInt32();
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
    if (!parcel.WriteBool(isRestartKeepAlive)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "isRestartKeepAlive write failed");
        return false;
    }
    if (!parcel.WriteBool(isStartFromNDK)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "isStartFromNDK write failed");
        return false;
    }
    if (!parcel.WriteBool(isPreloadStart)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "isPreloadStart write failed");
        return false;
    }
    if (!parcel.WriteUint64(loadAbilityCallbackId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "loadAbilityCallbackId write failed");
        return false;
    }
    if (!parcel.WriteInt32(callingPid)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "callingPid write failed");
        return false;
    }
    if (!parcel.WriteInt32(selfPid)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "selfPid write failed");
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

StartupVisibility ProcessOptions::ConvertInt32ToStartupVisibility(int32_t value)
{
    if (value <= static_cast<int32_t>(StartupVisibility::UNSPECIFIED) ||
        value >= static_cast<int32_t>(StartupVisibility::END)) {
        return StartupVisibility::UNSPECIFIED;
    }
    return static_cast<StartupVisibility>(value);
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

bool ProcessOptions::IsNewHiddenProcessMode(ProcessMode value)
{
    return (value == ProcessMode::NEW_HIDDEN_PROCESS);
}

void ProcessOptions::SanitizeSystemFields()
{
    TAG_LOGW(AAFwkTag::ABILITYMGR, "Sanitize system-only fields: isRestartKeepAlive=%{public}d, "
        "isStartFromNDK=%{public}d, isPreloadStart=%{public}d, loadAbilityCallbackId=%{public}" PRIu64 ", "
        "callingPid=%{public}d, selfPid=%{public}d",
        isRestartKeepAlive, isStartFromNDK, isPreloadStart, loadAbilityCallbackId, callingPid, selfPid);
    isRestartKeepAlive = false;
    isStartFromNDK = false;
    isPreloadStart = false;
    loadAbilityCallbackId = 0;
    callingPid = -1;
    selfPid = -1;
}
}  // namespace AAFwk
}  // namespace OHOS
