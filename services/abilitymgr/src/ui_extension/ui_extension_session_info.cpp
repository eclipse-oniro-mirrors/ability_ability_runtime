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

#include "ui_extension/ui_extension_session_info.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
UIExtensionSessionInfo *UIExtensionSessionInfo::Unmarshalling(Parcel &parcel)
{
    UIExtensionSessionInfo *info = new (std::nothrow) UIExtensionSessionInfo();
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "create info failed");
        return nullptr;
    }
    if (!parcel.ReadInt32(info->persistentId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "read persistentId failed");
        delete info;
        return nullptr;
    }
    if (!parcel.ReadUint32(info->hostWindowId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "read hostWindowId failed");
        delete info;
        return nullptr;
    }
    int32_t intVal = 0;
    uint32_t uintVal = 0;
    if (!parcel.ReadUint32(uintVal)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "read uiExtensionUsage failed");
        delete info;
        return nullptr;
    }
    info->uiExtensionUsage = static_cast<AAFwk::UIExtensionUsage>(uintVal);
    std::unique_ptr<AppExecFwk::ElementName> element(parcel.ReadParcelable<AppExecFwk::ElementName>());
    if (element == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "get element failed");
        delete info;
        return nullptr;
    }
    info->elementName = *element;
    if (!parcel.ReadInt32(intVal)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "read extensionAbilityType failed");
        delete info;
        return nullptr;
    }
    info->extensionAbilityType = static_cast<AppExecFwk::ExtensionAbilityType>(intVal);
    std::unique_ptr<AppExecFwk::ElementName> hostElement(parcel.ReadParcelable<AppExecFwk::ElementName>());
    if (hostElement == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "get hostElement failed");
        delete info;
        return nullptr;
    }
    info->hostElementName = *hostElement;
    if (!parcel.ReadBool(info->isBlockSubwindow)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "read isBlockSubwindow failed");
        delete info;
        return nullptr;
    }
    return info;
}

bool UIExtensionSessionInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(persistentId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write persistentId failed");
        return false;
    }

    if (!parcel.WriteUint32(hostWindowId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write hostWindowId failed");
        return false;
    }

    if (!parcel.WriteUint32(static_cast<uint32_t>(uiExtensionUsage))) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write uiExtensionUsage failed");
        return false;
    }

    if (!parcel.WriteParcelable(&elementName)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write elementName failed");
        return false;
    }

    if (!parcel.WriteInt32(static_cast<int32_t>(extensionAbilityType))) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write extensionAbilityType failed");
        return false;
    }

    if (!parcel.WriteParcelable(&hostElementName)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write hostElementName failed");
        return false;
    }

    if (!parcel.WriteBool(isBlockSubwindow)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write isBlockSubwindow failed");
        return false;
    }

    return true;
}
} // namespace AbilityRuntime
} // namespace OHOS
