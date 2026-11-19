/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include "running_process_info.h"

#include "nlohmann/json.hpp"
#include "string_ex.h"

#include "hilog_tag_wrapper.h"
#include "parcel_macro_base.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string JSON_KEY_PROCESSNAME = "processName";
const std::string JSON_KEY_PID = "pid";
const std::string JSON_KEY_STATE = "state";
}  // namespace

bool RunningProcessInfo::ReadFromParcel(Parcel &parcel)
{
    processName_ = Str16ToStr8(parcel.ReadString16());
    int32_t typeData;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, typeData);
    pid_ = static_cast<int32_t>(typeData);
    int32_t uidData;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, uidData);
    uid_ = static_cast<int32_t>(uidData);
    uint32_t accessTokenIdData;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Uint32, parcel, accessTokenIdData);
    accessTokenId_ = accessTokenIdData;
    int32_t stateData;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, stateData);
    state_ = static_cast<AppProcessState>(stateData);
    isContinuousTask = parcel.ReadBool();
    isKeepAlive = parcel.ReadBool();
    isFocused = parcel.ReadBool();
    isTestProcess = parcel.ReadBool();
    isAbilityForegrounding = parcel.ReadBool();
    isTestMode = parcel.ReadBool();
    isDebugApp = parcel.ReadBool();
    isExiting = parcel.ReadBool();
    isPreForeground = parcel.ReadBool();
    isPreload = parcel.ReadBool();
    int32_t bundleTypeData;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, bundleTypeData);
    bundleType = static_cast<int32_t>(bundleTypeData);
    if (!parcel.ReadStringVector(&bundleNames)) {
        TAG_LOGE(AAFwkTag::APPMGR, "read bundleNames failed.");
        return false;
    }
    int32_t processType;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, processType);
    processType_ = static_cast<ProcessType>(processType);
    int32_t extensionType;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, extensionType);
    extensionType_ = static_cast<ExtensionAbilityType>(extensionType);
    appCloneIndex = parcel.ReadInt32();
    int32_t preloadMode;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, preloadMode);
    preloadMode_ = static_cast<PreloadMode>(preloadMode);
    instanceKey = Str16ToStr8(parcel.ReadString16());
    int32_t appModeType;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, appModeType);
    appMode = static_cast<AppExecFwk::MultiAppModeType>(appModeType);
    int32_t rssData;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, rssData);
    rssValue = static_cast<int32_t>(rssData);
    int32_t pssData;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, pssData);
    pssValue = static_cast<int32_t>(pssData);
    isCached = parcel.ReadBool();
    return true;
}

RunningProcessInfo *RunningProcessInfo::Unmarshalling(Parcel &parcel)
{
    RunningProcessInfo *info = new (std::nothrow) RunningProcessInfo();
    if (info && !info->ReadFromParcel(parcel)) {
        TAG_LOGW(AAFwkTag::APPMGR, "read from parcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}

bool RunningProcessInfo::Marshalling(Parcel &parcel) const
{
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(processName_));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, static_cast<int32_t>(pid_));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, static_cast<int32_t>(uid_));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Uint32, parcel, accessTokenId_);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, static_cast<int32_t>(state_));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Bool, parcel, isContinuousTask);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Bool, parcel, isKeepAlive);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Bool, parcel, isFocused);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Bool, parcel, isTestProcess);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Bool, parcel, isAbilityForegrounding);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Bool, parcel, isTestMode);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Bool, parcel, isDebugApp);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Bool, parcel, isExiting);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Bool, parcel, isPreForeground);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Bool, parcel, isPreload);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, static_cast<int32_t>(bundleType));
    if (!parcel.WriteStringVector(bundleNames)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write bundleNames failed.");
        return false;
    }
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, static_cast<int32_t>(processType_));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, static_cast<int32_t>(extensionType_));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, appCloneIndex);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, static_cast<int32_t>(preloadMode_));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(instanceKey));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, static_cast<int32_t>(appMode));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, static_cast<int32_t>(rssValue));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, static_cast<int32_t>(pssValue));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Bool, parcel, isCached);
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
