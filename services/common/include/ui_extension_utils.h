/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_AAFwk_UI_EXTENSION_UTILS_H
#define OHOS_AAFwk_UI_EXTENSION_UTILS_H

#include <unordered_set>

#include "extension_ability_info.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr int EDM_SA_UID = 3057;
}
namespace UIExtensionUtils {
// ui extension type list
inline std::unordered_set<AppExecFwk::ExtensionAbilityType> GetUiExtensionSet()
{
    return std::unordered_set<AppExecFwk::ExtensionAbilityType> {
        AppExecFwk::ExtensionAbilityType::SHARE,
        AppExecFwk::ExtensionAbilityType::ACTION,
        AppExecFwk::ExtensionAbilityType::AGENT_UI,
        AppExecFwk::ExtensionAbilityType::EMBEDDED_UI,
        AppExecFwk::ExtensionAbilityType::INSIGHT_INTENT_UI,
        AppExecFwk::ExtensionAbilityType::AUTO_FILL_PASSWORD,
        AppExecFwk::ExtensionAbilityType::UI,
        AppExecFwk::ExtensionAbilityType::SYSPICKER_MEDIACONTROL,
        AppExecFwk::ExtensionAbilityType::SYSDIALOG_USERAUTH,
        AppExecFwk::ExtensionAbilityType::SYSDIALOG_COMMON,
        AppExecFwk::ExtensionAbilityType::SYSDIALOG_ATOMICSERVICEPANEL,
        AppExecFwk::ExtensionAbilityType::SYSDIALOG_POWER,
        AppExecFwk::ExtensionAbilityType::SYSPICKER_SHARE,
        AppExecFwk::ExtensionAbilityType::HMS_ACCOUNT,
        AppExecFwk::ExtensionAbilityType::ADS,
        AppExecFwk::ExtensionAbilityType::VOIP,
        AppExecFwk::ExtensionAbilityType::STATUS_BAR_VIEW,
        AppExecFwk::ExtensionAbilityType::SYSDIALOG_MEETIMECALL,
        AppExecFwk::ExtensionAbilityType::SYSDIALOG_MEETIMECONTACT,
        AppExecFwk::ExtensionAbilityType::SYSDIALOG_MEETIMEMESSAGE,
        AppExecFwk::ExtensionAbilityType::SYSDIALOG_PRINT,
        AppExecFwk::ExtensionAbilityType::SYSPICKER_MEETIMECONTACT,
        AppExecFwk::ExtensionAbilityType::SYSPICKER_MEETIMECALLLOG,
        AppExecFwk::ExtensionAbilityType::SYSPICKER_PHOTOPICKER,
        AppExecFwk::ExtensionAbilityType::SYS_COMMON_UI,
        AppExecFwk::ExtensionAbilityType::SYSPICKER_NAVIGATION,
        AppExecFwk::ExtensionAbilityType::SYSPICKER_APPSELECTOR,
        AppExecFwk::ExtensionAbilityType::SYSPICKER_CAMERA,
        AppExecFwk::ExtensionAbilityType::SYSPICKER_FILEPICKER,
        AppExecFwk::ExtensionAbilityType::AUTO_FILL_SMART,
        AppExecFwk::ExtensionAbilityType::LIVEVIEW_LOCKSCREEN,
        AppExecFwk::ExtensionAbilityType::SYSPICKER_PHOTOEDITOR,
        AppExecFwk::ExtensionAbilityType::PHOTO_EDITOR,
        AppExecFwk::ExtensionAbilityType::SYSPICKER_AUDIOPICKER,
        AppExecFwk::ExtensionAbilityType::SYS_VISUAL,
        AppExecFwk::ExtensionAbilityType::RECENT_PHOTO,
        AppExecFwk::ExtensionAbilityType::FORM_EDIT,
        AppExecFwk::ExtensionAbilityType::AWC_WEBPAGE,
        AppExecFwk::ExtensionAbilityType::AWC_NEWSFEED,
        AppExecFwk::ExtensionAbilityType::LIVE_FORM,
        AppExecFwk::ExtensionAbilityType::EMBEDDED_CASHIER
    };
}

inline bool IsUIExtension(const AppExecFwk::ExtensionAbilityType type)
{
    return GetUiExtensionSet().count(type) > 0;
}

inline bool IsSystemUIExtension(const AppExecFwk::ExtensionAbilityType type)
{
    const std::unordered_set<AppExecFwk::ExtensionAbilityType> systemUiExtensionSet = {
        AppExecFwk::ExtensionAbilityType::SYSDIALOG_COMMON,
        AppExecFwk::ExtensionAbilityType::SYSDIALOG_ATOMICSERVICEPANEL,
        AppExecFwk::ExtensionAbilityType::SYSDIALOG_POWER,
        AppExecFwk::ExtensionAbilityType::SYSDIALOG_MEETIMECALL,
        AppExecFwk::ExtensionAbilityType::SYSDIALOG_MEETIMECONTACT,
        AppExecFwk::ExtensionAbilityType::SYSDIALOG_MEETIMEMESSAGE,
        AppExecFwk::ExtensionAbilityType::SYSDIALOG_PRINT,
        AppExecFwk::ExtensionAbilityType::SYSPICKER_MEDIACONTROL,
        AppExecFwk::ExtensionAbilityType::SYSPICKER_SHARE,
        AppExecFwk::ExtensionAbilityType::SYSPICKER_MEETIMECONTACT,
        AppExecFwk::ExtensionAbilityType::SYSPICKER_MEETIMECALLLOG,
        AppExecFwk::ExtensionAbilityType::SYSPICKER_PHOTOPICKER,
        AppExecFwk::ExtensionAbilityType::SYSPICKER_NAVIGATION,
        AppExecFwk::ExtensionAbilityType::SYSPICKER_APPSELECTOR,
        AppExecFwk::ExtensionAbilityType::UI,
        AppExecFwk::ExtensionAbilityType::SYS_COMMON_UI,
        AppExecFwk::ExtensionAbilityType::SYSPICKER_PHOTOEDITOR,
        AppExecFwk::ExtensionAbilityType::ADS,
        AppExecFwk::ExtensionAbilityType::SYSPICKER_AUDIOPICKER,
        AppExecFwk::ExtensionAbilityType::AUTO_FILL_PASSWORD,
        AppExecFwk::ExtensionAbilityType::SYSPICKER_CAMERA,
        AppExecFwk::ExtensionAbilityType::AUTO_FILL_SMART,
        AppExecFwk::ExtensionAbilityType::SYSPICKER_FILEPICKER,
        AppExecFwk::ExtensionAbilityType::SYSDIALOG_USERAUTH,
        AppExecFwk::ExtensionAbilityType::HMS_ACCOUNT,
        AppExecFwk::ExtensionAbilityType::SYS_VISUAL,
        AppExecFwk::ExtensionAbilityType::RECENT_PHOTO,
        AppExecFwk::ExtensionAbilityType::AWC_WEBPAGE,
        AppExecFwk::ExtensionAbilityType::AWC_NEWSFEED,
        AppExecFwk::ExtensionAbilityType::EMBEDDED_CASHIER
    };
    return systemUiExtensionSet.find(type) != systemUiExtensionSet.end();
}

// In this case, extension which be starting needs that caller should be the system app, otherwise not supported.
inline bool IsSystemCallerNeeded(const AppExecFwk::ExtensionAbilityType type)
{
    const std::unordered_set<AppExecFwk::ExtensionAbilityType> uiExtensionStartingSet = {
        AppExecFwk::ExtensionAbilityType::PHOTO_EDITOR,
        AppExecFwk::ExtensionAbilityType::INSIGHT_INTENT_UI,
        AppExecFwk::ExtensionAbilityType::LIVEVIEW_LOCKSCREEN,
        AppExecFwk::ExtensionAbilityType::SHARE,
        AppExecFwk::ExtensionAbilityType::ACTION,
        AppExecFwk::ExtensionAbilityType::STATUS_BAR_VIEW,
        AppExecFwk::ExtensionAbilityType::VOIP,
        AppExecFwk::ExtensionAbilityType::FORM_EDIT,
        AppExecFwk::ExtensionAbilityType::LIVE_FORM
    };
    return uiExtensionStartingSet.find(type) != uiExtensionStartingSet.end();
}

// In this collection, extension can be embedded by public app, which requires vertical businesses to ensure security.
inline bool IsPublicForEmbedded(const AppExecFwk::ExtensionAbilityType type)
{
    const std::unordered_set<AppExecFwk::ExtensionAbilityType> publicForEmbeddedSet = {
        AppExecFwk::ExtensionAbilityType::EMBEDDED_UI, // EMBEDDED_UI usage within the app
        AppExecFwk::ExtensionAbilityType::ADS,
        AppExecFwk::ExtensionAbilityType::SYSPICKER_MEDIACONTROL,
        AppExecFwk::ExtensionAbilityType::SYS_VISUAL,
        AppExecFwk::ExtensionAbilityType::AUTO_FILL_SMART,
        AppExecFwk::ExtensionAbilityType::AUTO_FILL_PASSWORD,
        AppExecFwk::ExtensionAbilityType::EMBEDDED_CASHIER
    };
    return publicForEmbeddedSet.find(type) != publicForEmbeddedSet.end();
}

// In this collection, extension can be embedded by public app, which some UX effects are constrained
inline bool IsPublicForConstrainedEmbedded(const AppExecFwk::ExtensionAbilityType type)
{
    const std::unordered_set<AppExecFwk::ExtensionAbilityType> publicForConstrainedEmbeddedSet = {
        AppExecFwk::ExtensionAbilityType::SYSPICKER_PHOTOPICKER,
        AppExecFwk::ExtensionAbilityType::RECENT_PHOTO
    };
    return publicForConstrainedEmbeddedSet.find(type) != publicForConstrainedEmbeddedSet.end();
}

inline bool IsEnterpriseAdmin(const AppExecFwk::ExtensionAbilityType type)
{
    bool enterpriseAdminSa = (IPCSkeleton::GetCallingUid() == EDM_SA_UID);
    bool isEnterpriseAdmin = (type == AppExecFwk::ExtensionAbilityType::ENTERPRISE_ADMIN);
    return enterpriseAdminSa && isEnterpriseAdmin;
}

inline bool IsWindowExtension(const AppExecFwk::ExtensionAbilityType type)
{
    return type == AppExecFwk::ExtensionAbilityType::WINDOW;
}

inline bool IsProcessUdkeyExtension(const AppExecFwk::ExtensionAbilityType type)
{
    const std::unordered_set<AppExecFwk::ExtensionAbilityType> processUdkeyExtensionSet = {
        AppExecFwk::ExtensionAbilityType::SYSPICKER_FILEPICKER
    };
    return processUdkeyExtensionSet.find(type) != processUdkeyExtensionSet.end();
}

inline bool IsAgentUIExtension(const AppExecFwk::ExtensionAbilityType type)
{
    return type == AppExecFwk::ExtensionAbilityType::AGENT_UI;
}
} // namespace UIExtensionUtils
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_AAFwk_UI_EXTENSION_UTILS_H
