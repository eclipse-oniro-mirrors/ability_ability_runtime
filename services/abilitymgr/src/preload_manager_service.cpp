/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "preload_manager_service.h"

#include "ability_manager_errors.h"
#include "ability_manager_service.h"
#include "ability_util.h"
#include "app_scheduler.h"
#include "app_utils.h"
#include "hilog_tag_wrapper.h"
#include "in_process_call_wrapper.h"
#include "global_constant.h"
#include "parameters.h"
#include "permission_verification.h"
#include "process_options.h"
#include "start_ability_utils.h"
#include "start_options.h"
#include "bundle_mgr_interface.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr const char* DEVELOPER_MODE_STATE = "const.security.developermode.state";
}  // namespace
PreloadManagerService &PreloadManagerService::GetInstance()
{
    static PreloadManagerService instance;
    return instance;
}

PreloadManagerService::PreloadManagerService() {}

PreloadManagerService::~PreloadManagerService() {}

int32_t PreloadManagerService::PreloadApplication(const std::string &bundleName, int32_t userId, int32_t appIndex)
{
    CHECK_TRUE_RETURN_RET(!AppUtils::GetInstance().IsPreloadApplicationEnabled() || appIndex != 0,
        ERR_CAPABILITY_NOT_SUPPORT, "preload application not supported");
    CHECK_TRUE_RETURN_RET(!PermissionVerification::GetInstance()->VerifyPreloadApplicationPermission(),
        ERR_PERMISSION_DENIED, "no preload permission");
    Want launchWant;
    AppExecFwk::AbilityInfo abilityInfo;
    if (auto ret = PreloadApplicationVerification(bundleName, userId, appIndex, launchWant,
        abilityInfo); ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "verify PreloadApplication failed");
        return ret;
    }

    AppExecFwk::AppPreloadPhase appPreloadPhase = abilityInfo.applicationInfo.appPreloadPhase;
    if (appPreloadPhase <= AppExecFwk::AppPreloadPhase::DEFAULT ||
        appPreloadPhase > AppExecFwk::AppPreloadPhase::WINDOW_STAGE_CREATED) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid preload phase:%{public}d", static_cast<int32_t>(appPreloadPhase));
        return ERR_INVALID_APP_PRELOAD_PHASE;
    }

    if (appPreloadPhase <= AppExecFwk::AppPreloadPhase::ABILITY_STAGE_CREATED) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "preload to phase:%{public}d", static_cast<int32_t>(appPreloadPhase));
        AppExecFwk::PreloadPhase preloadPhase = static_cast<AppExecFwk::PreloadPhase>(appPreloadPhase);
        return DelayedSingleton<AppScheduler>::GetInstance()->PreloadApplicationByPhase(
            bundleName, userId, appIndex, preloadPhase);
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "preload to window stage create");
    CHECK_TRUE_RETURN_RET(abilityInfo.launchMode == AppExecFwk::LaunchMode::STANDARD, ERR_CAPABILITY_NOT_SUPPORT,
        "not support multiton");
    StartOptions options;
    options.processOptions = std::make_shared<ProcessOptions>();
    options.processOptions->startupVisibility = StartupVisibility::STARTUP_HIDE;
    options.processOptions->isPreloadStart = true;
    return DelayedSingleton<AbilityManagerService>::GetInstance()->StartAbility(launchWant, options, nullptr, userId);
}

int32_t PreloadManagerService::LaunchGameCustomized(const std::string &bundleName, int32_t userId, int32_t appIndex)
{
    Want launchWant;
    AppExecFwk::AbilityInfo abilityInfo;
    if (auto ret = PreloadApplicationVerification(bundleName, userId, appIndex, launchWant,
        abilityInfo); ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "verify preload game failed");
        return ret;
    }

    bool isGameSACall = IPCSkeleton::GetCallingUid() == AbilityRuntime::GlobalConstant::GAME_SA_UID;
    bool isShellCall = AAFwk::PermissionVerification::GetInstance()->IsShellCall();
    bool isDevelopMode = OHOS::system::GetBoolParameter(DEVELOPER_MODE_STATE, false);
    bool isDebugApp = abilityInfo.applicationInfo.appProvisionType == AppExecFwk::Constants::APP_PROVISION_TYPE_DEBUG;
    if (!(isGameSACall || (isShellCall && isDevelopMode && isDebugApp))) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Permission check failed: callingUid: %{public}d, isGameSACall = %{public}d, "
            "isShellCall = %{public}d, isDevelopMode = %{public}d, isDebugApp = %{public}d",
            IPCSkeleton::GetCallingUid(), isGameSACall, isShellCall, isDevelopMode, isDebugApp);
        return CHECK_PERMISSION_FAILED;
    }

    // Set game prelaunch flag
    launchWant.SetParam(AbilityRuntime::GlobalConstant::GAME_PRELAUNCH, true);

    StartAbilityWrapParam startAbilityWrapParam;
    startAbilityWrapParam.want = launchWant;
    startAbilityWrapParam.userId = userId;
    startAbilityWrapParam.isGamePrelaunch = true;
    return DelayedSingleton<AbilityManagerService>::GetInstance()->StartAbilityInner(startAbilityWrapParam);
}

int32_t PreloadManagerService::PreloadApplicationVerification(const std::string &bundleName, int32_t userId,
    int32_t appIndex, Want &want, AppExecFwk::AbilityInfo &abilityInfo)
{
    userId = DelayedSingleton<AbilityManagerService>::GetInstance()->GetValidUserId(userId);
    CHECK_TRUE_RETURN_RET(!DelayedSingleton<AbilityManagerService>::GetInstance()->JudgeMultiUserConcurrency(userId),
        ERR_CROSS_USER, "multi-user non-concurrent unsatisfied");

    bool isExist = false;
    int32_t ret = ERR_OK;
    CHECK_TRUE_RETURN_RET((ret = DelayedSingleton<AppScheduler>::GetInstance()->CheckPreloadAppRecordExist(
        bundleName, userId, appIndex, isExist)) != ERR_OK, ret, "CheckPreloadAppRecordExist failed");
    CHECK_TRUE_RETURN_RET(isExist, ERR_PRELOAD_APP_RECORD_ALREADY_EXIST, "already started");
    
    auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
    CHECK_POINTER_AND_RETURN(bundleMgrHelper, INNER_ERR);

    auto errCode = IN_PROCESS_CALL(bundleMgrHelper->GetLaunchWantForBundle(bundleName, want, userId));
    if (errCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "getLaunchWantForBundle returns %{public}d", errCode);
        return errCode;
    }
    CHECK_TRUE_RETURN_RET(!IN_PROCESS_CALL(bundleMgrHelper->QueryAbilityInfo(want,
        AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION, userId, abilityInfo)),
        RESOLVE_ABILITY_ERR, "failed to get abilityInfo");
    return ERR_OK;
}
}  // namespace AAFwk
}  // namespace OHOS
