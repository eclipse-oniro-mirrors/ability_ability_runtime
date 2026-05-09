/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "app_state_data.h"
#include "app_mgr_constants.h"

#include "hilog_tag_wrapper.h"
#include "ui_extension_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
bool AppStateData::Marshalling(Parcel &parcel) const
{
    return parcel.WriteString(bundleName) && parcel.WriteInt32(uid) && parcel.WriteInt32(state) &&
        parcel.WriteInt32(pid) && parcel.WriteUint32(accessTokenId) && parcel.WriteBool(isFocused) &&
        parcel.WriteInt32(static_cast<int32_t>(extensionType)) && parcel.WriteInt32Vector(renderPids) &&
        parcel.WriteString(callerBundleName) && parcel.WriteBool(isSplitScreenMode) && parcel.WriteInt32(callerUid) &&
        parcel.WriteBool(isFloatingWindowMode) && parcel.WriteInt32(appIndex) && parcel.WriteBool(isPreloadModule) &&
        parcel.WriteBool(isFromWindowFocusChanged) && parcel.WriteInt32(preloadMode) &&
        parcel.WriteInt32(byCallStatus);
}

bool AppStateData::ReadFromParcel(Parcel &parcel)
{
    bundleName = parcel.ReadString();
    uid = parcel.ReadInt32();
    state = parcel.ReadInt32();
    pid = parcel.ReadInt32();
    accessTokenId = parcel.ReadUint32();
    isFocused = parcel.ReadBool();
    extensionType = static_cast<ExtensionAbilityType>(parcel.ReadInt32());
    parcel.ReadInt32Vector(&renderPids);
    callerBundleName = parcel.ReadString();
    isSplitScreenMode = parcel.ReadBool();
    callerUid = parcel.ReadInt32();
    isFloatingWindowMode = parcel.ReadBool();
    appIndex = parcel.ReadInt32();
    isPreloadModule = parcel.ReadBool();
    isFromWindowFocusChanged = parcel.ReadBool();
    preloadMode = parcel.ReadInt32();
    byCallStatus = parcel.ReadInt32();
    return true;
}

AppStateData *AppStateData::Unmarshalling(Parcel &parcel)
{
    AppStateData *appStateData = new (std::nothrow) AppStateData();
    if (appStateData && !appStateData->ReadFromParcel(parcel)) {
        TAG_LOGW(AAFwkTag::APPMGR, "appStateData failed, because ReadFromParcel failed");
        delete appStateData;
        appStateData = nullptr;
    }
    return appStateData;
}

bool AppStateData::IsUIExtension(const AppExecFwk::ExtensionAbilityType type)
{
    return AAFwk::UIExtensionWrapper::IsUIExtension(type);
}
}  // namespace AppExecFwk
}  // namespace OHOS
