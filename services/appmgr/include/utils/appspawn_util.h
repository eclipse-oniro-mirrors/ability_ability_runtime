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

#ifndef OHOS_ABILITY_RUNTIME_APPSPAWN_UTIL_H
#define OHOS_ABILITY_RUNTIME_APPSPAWN_UTIL_H

#include "ability_info.h"
#include "app_spawn_client.h"
#include "global_constant.h"
#include "hitrace_meter.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
namespace AppspawnUtil {
constexpr const char* DLP_PARAMS_INDEX = "ohos.dlp.params.index";
constexpr const char*
    JIT_PERMISSION_ALLOW_WRITABLE_CODE_MEMORY = "ohos.permission.kernel.ALLOW_WRITABLE_CODE_MEMORY";
constexpr const char*
    JIT_PERMISSION_DISABLE_CODE_MEMORY_PROTECTION = "ohos.permission.kernel.DISABLE_CODE_MEMORY_PROTECTION";
constexpr const char*
    JIT_PERMISSION_ALLOW_EXECUTABLE_FORT_MEMORY = "ohos.permission.kernel.ALLOW_EXECUTABLE_FORT_MEMORY";
constexpr const char*
    JIT_PERMISSION_DISABLE_GOTPLT_RO_PROTECTION = "ohos.permission.kernel.DISABLE_GOTPLT_RO_PROTECTION";
constexpr const char* DLP_PARAMS_SECURITY_FLAG = "ohos.dlp.params.securityFlag";

static uint64_t BuildStartFlags(const AAFwk::Want &want, const ApplicationInfo &applicationInfo)
{
    uint64_t startFlags = 0x0;
    if (want.GetBoolParam("coldStart", false)) {
        startFlags = startFlags | (START_FLAG_BASE << StartFlags::COLD_START);
    }
#ifdef WITH_DLP
    if (want.GetIntParam(DLP_PARAMS_INDEX, 0) != 0) {
        if (want.GetBoolParam(DLP_PARAMS_SECURITY_FLAG, false)) {
            startFlags = startFlags | (START_FLAG_BASE << StartFlags::DLP_MANAGER_READ_ONLY);
        } else {
            startFlags = startFlags | (START_FLAG_BASE << StartFlags::DLP_MANAGER_FULL_CONTROL);
        }
    }
#endif // WITH_DLP
    if (applicationInfo.debug && applicationInfo.appProvisionType == AppExecFwk::Constants::APP_PROVISION_TYPE_DEBUG) {
        startFlags = startFlags | (START_FLAG_BASE << StartFlags::DEBUGGABLE);
    }
    if (applicationInfo.asanEnabled) {
        startFlags = startFlags | (START_FLAG_BASE << StartFlags::ASANENABLED);
    }
    if (want.GetBoolParam("nativeDebug", false)) {
        startFlags = startFlags | (START_FLAG_BASE << StartFlags::NATIVEDEBUG);
    }
    if (applicationInfo.gwpAsanEnabled) {
        startFlags = startFlags | (START_FLAG_BASE << StartFlags::GWP_ENABLED_FORCE);
    }
    if (applicationInfo.isSystemApp) {
        startFlags = startFlags | (START_FLAG_BASE << StartFlags::GWP_ENABLED_NORMAL);
    }
    if (applicationInfo.tsanEnabled) {
        startFlags = startFlags | (START_FLAG_BASE << StartFlags::TSANENABLED);
    }
    if (want.GetBoolParam("ohos.ability.params.extensionControl", false)) {
        startFlags = startFlags | (START_FLAG_BASE << StartFlags::EXTENSION_CONTROLLED);
    }
    if (applicationInfo.multiAppMode.multiAppModeType == MultiAppModeType::APP_CLONE && applicationInfo.appIndex > 0 &&
        applicationInfo.appIndex <= AbilityRuntime::GlobalConstant::MAX_APP_CLONE_INDEX) {
        startFlags = startFlags | (START_FLAG_BASE << APP_FLAGS_CLONE_ENABLE);
    }
    if (applicationInfo.hwasanEnabled) {
        startFlags = startFlags | (START_FLAG_BASE << StartFlags::HWASANENABLED);
    }
    if (applicationInfo.ubsanEnabled) {
        startFlags = startFlags | (START_FLAG_BASE << StartFlags::UBSANENABLED);
    }
    if (applicationInfo.cloudFileSyncEnabled) {
        startFlags = startFlags | (START_FLAG_BASE << APP_FLAGS_CLOUD_FILE_SYNC_ENABLED);
    }
    if (applicationInfo.appProvisionType == AppExecFwk::Constants::APP_PROVISION_TYPE_DEBUG) {
        startFlags = startFlags | (START_FLAG_BASE << APP_FLAGS_DEBUG_SIGN);
    }
    if (applicationInfo.skillEnabled) {
        startFlags = startFlags | (START_FLAG_BASE << APP_FLAGS_SKILLS);
    }
    return startFlags;
}

static uint64_t BuildStartFlags(const AAFwk::Want &want, const AbilityInfo &abilityInfo)
{
    uint64_t startFlags = BuildStartFlags(want, abilityInfo.applicationInfo);

    if (abilityInfo.extensionAbilityType == ExtensionAbilityType::BACKUP) {
        startFlags = startFlags | (START_FLAG_BASE << StartFlags::BACKUP_EXTENSION);
    }

    return startFlags;
}

static void SetJITPermissions(uint32_t accessTokenId, std::vector<std::string> &jitPermissionsList)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    std::vector<std::string> tmpPermissionList = {
        JIT_PERMISSION_ALLOW_WRITABLE_CODE_MEMORY,
        JIT_PERMISSION_DISABLE_CODE_MEMORY_PROTECTION,
        JIT_PERMISSION_ALLOW_EXECUTABLE_FORT_MEMORY,
        JIT_PERMISSION_DISABLE_GOTPLT_RO_PROTECTION
    };

    std::vector<int32_t> permStateList;
    auto result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(accessTokenId, tmpPermissionList,
        permStateList, true);
    if (result != ERR_OK || permStateList.size() != tmpPermissionList.size()) {
        return;
    }
    for (size_t i = 0; i < permStateList.size(); i++) {
        if (permStateList[i] == Security::AccessToken::PERMISSION_GRANTED) {
            jitPermissionsList.emplace_back(tmpPermissionList[i]);
        }
    }
}
}  // namespace AppspawnUtil
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_APPSPAWN_UTIL_H
