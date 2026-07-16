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

#include "start_options_utils.h"

#include "ability_manager_errors.h"
#include "ability_manager_service.h"
#include "ability_permission_util.h"
#include "ability_record.h"
#include "ability_util.h"
#include "app_scheduler.h"
#include "app_utils.h"
#include "hidden_start_utils.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "ipc_skeleton.h"
#include "iremote_object.h"
#include "permission_verification.h"
#include "process_options.h"
#include "scene_board_judgement.h"
#include "start_ability_utils.h"
#include "start_options.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr const char* ACTION_CHOOSE = "ohos.want.action.select";
constexpr const char* FOUNDATION_PROCESS_NAME = "foundation";
}
int32_t StartOptionsUtils::CheckProcessOptions(const Want &want, const StartOptions &options,
    sptr<IRemoteObject> callerToken, int32_t userId)
{
    if (HiddenStartUtils::IsHiddenStart(options)) {
        return HiddenStartUtils::CheckHiddenStartSupported(options);
    }
    if (AbilityPermissionUtil::GetInstance().IsStartSelfUIAbility() &&
        options.processOptions != nullptr && options.processOptions->isStartFromNDK) {
        return CheckStartSelfUIAbilityStartOptions(want, options);
    }
    return CheckProcessOptionsInner(want, options, callerToken, userId);
}

int32_t StartOptionsUtils::CheckProcessOptionsInner(const Want &want, const StartOptions &options,
    sptr<IRemoteObject> callerToken, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (options.processOptions == nullptr ||
        (!ProcessOptions::IsValidProcessMode(options.processOptions->processMode) &&
        !options.processOptions->isRestartKeepAlive)) {
        return ERR_OK;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "start ability with process options");
    bool isEnable = AppUtils::GetInstance().IsStartOptionsWithProcessOptions();
    CHECK_TRUE_RETURN_RET(!Rosen::SceneBoardJudgement::IsSceneBoardEnabled() || !isEnable,
        ERR_CAPABILITY_NOT_SUPPORT, "not support process options");

    auto element = want.GetElement();
    CHECK_TRUE_RETURN_RET(element.GetAbilityName().empty() || want.GetActionRef().compare(ACTION_CHOOSE) == 0,
        ERR_NOT_ALLOW_IMPLICIT_START, "not allow implicit start");

    if (PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission(FOUNDATION_PROCESS_NAME)
        && options.processOptions->isRestartKeepAlive) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "restart keep-alive app.");
        return ERR_OK;
    }

    StartAbilityUtils::ResolveTargetAppCloneIndex(want, callerToken, userId);
    int32_t appIndex = 0;
    appIndex = !StartAbilityUtils::GetAppIndex(want, callerToken, appIndex) ? 0 : appIndex;
    CHECK_TRUE_RETURN_RET(!DelayedSingleton<AbilityManagerService>::GetInstance()->CheckCallingTokenId(
        element.GetBundleName(), userId, appIndex), ERR_NOT_SELF_APPLICATION, "not self application");

    auto uiAbilityManager = DelayedSingleton<AbilityManagerService>::GetInstance()->GetUIAbilityManagerByUid(
        IPCSkeleton::GetCallingUid());
    CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_INVALID_VALUE);

    auto callerPid = IPCSkeleton::GetCallingPid();
    AppExecFwk::RunningProcessInfo processInfo;
    DelayedSingleton<AppScheduler>::GetInstance()->GetRunningProcessInfoByPid(callerPid, processInfo);
    CHECK_TRUE_RETURN_RET((ProcessOptions::IsAttachToStatusBarMode(options.processOptions->processMode) &&
        !uiAbilityManager->IsCallerInStatusBar(processInfo.instanceKey)), ERR_START_OPTIONS_CHECK_FAILED,
        "not in status bar");

    auto abilityRecords = uiAbilityManager->GetAbilityRecordsByName(element, appIndex);
    CHECK_TRUE_RETURN_RET(!abilityRecords.empty() && abilityRecords[0] &&
        abilityRecords[0]->GetAbilityInfo().launchMode != AppExecFwk::LaunchMode::STANDARD &&
        abilityRecords[0]->GetAbilityInfo().launchMode != AppExecFwk::LaunchMode::SPECIFIED,
        ERR_ABILITY_ALREADY_RUNNING, "if not STANDARD or SPECIFIED mode, repeated starts not allowed");

    return ERR_OK;
}

int32_t StartOptionsUtils::CheckStartSelfUIAbilityStartOptions(const Want &want, const StartOptions &options)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (options.processOptions == nullptr) {
        return ERR_OK;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "start ability with process options");
    bool isEnable = AppUtils::GetInstance().IsStartOptionsWithProcessOptions();
    CHECK_TRUE_RETURN_RET(!Rosen::SceneBoardJudgement::IsSceneBoardEnabled() || !isEnable,
        ERR_CAPABILITY_NOT_SUPPORT, "not support process options");

    if (options.processOptions->startupVisibility == StartupVisibility::UNSPECIFIED &&
        options.processOptions->loadAbilityCallbackId > 0) {
        return ERR_OK;
    }

    auto uiAbilityManager = DelayedSingleton<AbilityManagerService>::GetInstance()->GetUIAbilityManagerByUid(
        IPCSkeleton::GetCallingUid());
    CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_INVALID_VALUE);

    auto callerPid = IPCSkeleton::GetCallingPid();
    AppExecFwk::RunningProcessInfo processInfo;
    DelayedSingleton<AppScheduler>::GetInstance()->GetRunningProcessInfoByChildProcessPid(callerPid, processInfo);
    CHECK_TRUE_RETURN_RET(!uiAbilityManager->IsCallerInStatusBar(processInfo.instanceKey),
        ERR_START_OPTIONS_CHECK_FAILED, "not in status bar");

    return ERR_OK;
}
}
}
