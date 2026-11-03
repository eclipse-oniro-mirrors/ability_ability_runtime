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
#include "direct_boot_test_support.h"

#include "accesstoken_kit.h"
#include "permission_constants.h"

namespace OHOS::AAFwk {
bool GetDirectBootBundleInfo(const std::string &bundleName, AppExecFwk::BundleInfo &bundleInfo, int32_t userId)
{
    auto &state = AAFwk::GetDirectBootTestState();
    auto target = state.targets.find(bundleName);
    if (!state.bundleAvailable || target == state.targets.end() || target->second.userId != userId) {
        return false;
    }
    bundleInfo.applicationInfo.accessTokenId = target->second.tokenId;
    bundleInfo.applicationInfo.appIndex = target->second.appIndex;
    bundleInfo.applicationInfo.uid = userId * AppExecFwk::Constants::BASE_USER_RANGE;
    AppExecFwk::AbilityInfo ability;
    ability.bundleName = bundleName;
    ability.moduleName = "entry";
    ability.name = "EntryAbility";
    ability.visible = true;
    ability.type = AppExecFwk::AbilityType::PAGE;
    AppExecFwk::HapModuleInfo module;
    module.abilityInfos.push_back(ability);
    bundleInfo.hapModuleInfos.push_back(module);
    return true;
}
} // namespace OHOS::AAFwk

namespace OHOS::Security::AccessToken {
uint32_t AccessTokenKit::GetHapTokenID(int32_t userId, const std::string &bundleName, int32_t appIndex)
{
    auto &state = AAFwk::GetDirectBootTestState();
    ++state.tokenQueries;
    state.queriedUser = userId;
    state.queriedBundle = bundleName;
    state.queriedClone = appIndex;
    auto target = state.targets.find(bundleName);
    if (target == state.targets.end() || target->second.userId != userId || appIndex != target->second.appIndex) {
        return INVALID_TOKENID;
    }
    return target->second.tokenId;
}

int AccessTokenKit::VerifyAccessToken(AccessTokenID tokenId, const std::string &permissionName)
{
    auto &state = AAFwk::GetDirectBootTestState();
    ++state.permissionQueries;
    for (const auto &[bundle, target] : state.targets) {
        if (target.tokenId != tokenId) {
            continue;
        }
        if (permissionName == AAFwk::PermissionConstants::PERMISSION_ALLOW_DIRECT_BOOT) {
            return target.allowDirectBoot ? PERMISSION_GRANTED : PERMISSION_DENIED;
        }
        if (permissionName == "ohos.permission.START_WINDOW_BELOW_LOCK_SCREEN") {
            ++state.windowPermissionQueries;
            return target.allowWindow ? PERMISSION_GRANTED : PERMISSION_DENIED;
        }
    }
    return PERMISSION_DENIED;
}
} // namespace OHOS::Security::AccessToken
