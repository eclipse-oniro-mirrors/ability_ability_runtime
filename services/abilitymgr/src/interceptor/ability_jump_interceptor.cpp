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

#include "interceptor/ability_jump_interceptor.h"

#include "ability_util.h"
#include "accesstoken_kit.h"
#include "hitrace_meter.h"
#include "permission_constants.h"
#include "start_ability_utils.h"
#include "system_dialog_scheduler.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr const char* JUMP_DIALOG_CALLER_BUNDLE_NAME = "interceptor_callerBundleName";
constexpr const char* JUMP_DIALOG_CALLER_MODULE_NAME = "interceptor_callerModuleName";
constexpr const char* JUMP_DIALOG_CALLER_LABEL_ID = "interceptor_callerLabelId";
constexpr const char* JUMP_DIALOG_TARGET_MODULE_NAME = "interceptor_targetModuleName";
constexpr const char* JUMP_DIALOG_TARGET_LABEL_ID = "interceptor_targetLabelId";
}
ErrCode AbilityJumpInterceptor::DoProcess(const AbilityInterceptorParam &param)
{
    if (!param.isWithUI) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "startup not foreground");
        return ERR_OK;
    }
    bool isStartIncludeAtomicService = AbilityUtil::IsStartIncludeAtomicService(param.want, param.userId);
    if (isStartIncludeAtomicService) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "startup atomic service");
        return ERR_OK;
    }
    // get bms
    auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
    CHECK_POINTER_AND_RETURN(bundleMgrHelper, ERR_OK);
    AppExecFwk::AbilityInfo targetAbilityInfo;
    if (StartAbilityUtils::startAbilityInfo != nullptr &&
        StartAbilityUtils::startAbilityInfo->abilityInfo.bundleName == param.want.GetBundleNameRef() &&
        StartAbilityUtils::startAbilityInfo->abilityInfo.name == param.want.GetAbilityNameRef()) {
        targetAbilityInfo = StartAbilityUtils::startAbilityInfo->abilityInfo;
    } else {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "abilityName: %{public}s, userId: %{public}d",
            param.want.GetAbilityNameRef().c_str(), param.userId);
        IN_PROCESS_CALL_WITHOUT_RET(bundleMgrHelper->QueryAbilityInfo(param.want,
            AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION, param.userId, targetAbilityInfo));
    }
    if (targetAbilityInfo.type != AppExecFwk::AbilityType::PAGE) {
        TAG_LOGI(AAFwkTag::ABILITYMGR,
            "no page Ability,type:%{public}d", targetAbilityInfo.type);
        return ERR_OK;
    }
    AppExecFwk::AppJumpControlRule controlRule;
    if (CheckControl(bundleMgrHelper, param.want, param.userId, controlRule)) {
#ifdef SUPPORT_GRAPHICS
        TAG_LOGI(AAFwkTag::ABILITYMGR, "intercept app jump,caller:%{public}s, target:%{public}s",
            controlRule.callerPkg.c_str(), controlRule.targetPkg.c_str());
        auto sysDialogScheduler = DelayedSingleton<SystemDialogScheduler>::GetInstance();
        Want targetWant = param.want;
        CHECK_POINTER_AND_RETURN(sysDialogScheduler, ERR_INVALID_VALUE);
        Want dialogWant = sysDialogScheduler->GetJumpInterceptorDialogWant(targetWant);
        AbilityUtil::ParseJumpInterceptorWant(dialogWant, controlRule.callerPkg);
        LoadAppLabelInfo(dialogWant, controlRule, param.userId);
        int ret = IN_PROCESS_CALL(AbilityManagerClient::GetInstance()->StartAbility(dialogWant,
            param.requestCode, param.userId));
        if (ret != ERR_OK) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "appInterceptor Dialog StartAbility error, ret:%{public}d", ret);
            return ret;
        }
#endif
        return ERR_APP_JUMP_INTERCEPTOR_STATUS;
    }
    return ERR_OK;
}

bool AbilityJumpInterceptor::CheckControl(std::shared_ptr<AppExecFwk::BundleMgrHelper> &bundleMgrHelper,
    const Want &want, int32_t userId, AppExecFwk::AppJumpControlRule &controlRule)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    int callerUid = IPCSkeleton::GetCallingUid();
    std::string callerBundleName;
    auto result = IN_PROCESS_CALL(bundleMgrHelper->GetNameForUid(callerUid, callerBundleName));
    const auto &targetBundleName = want.GetBundleNameRef();
    controlRule.callerPkg = callerBundleName;
    controlRule.targetPkg = targetBundleName;
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "getBundleName failed");
        return false;
    }
    if (controlRule.callerPkg.empty() || controlRule.targetPkg.empty()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "explicit startup");
        return false;
    }
    if (controlRule.callerPkg == controlRule.targetPkg) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "jump within the same app");
        return false;
    }
    if (CheckIfJumpExempt(controlRule, userId)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "jump to system/exempt apps");
        return false;
    }
    // get disposed status
    auto appControlMgr = bundleMgrHelper->GetAppControlProxy();
    if (appControlMgr == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null appControlMgr");
        return false;
    }

    if (IN_PROCESS_CALL(appControlMgr->GetAppJumpControlRule(callerBundleName, targetBundleName,
        userId, controlRule)) != ERR_OK) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "no jump rule");
        return true;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Get appJumpControlRule, jumpMode:%{public}d.", controlRule.jumpMode);
    return controlRule.jumpMode != AppExecFwk::AbilityJumpMode::DIRECT;
}

bool AbilityJumpInterceptor::CheckIfJumpExempt(AppExecFwk::AppJumpControlRule &controlRule, int32_t userId)
{
    if (CheckIfExemptByBundleName(controlRule.callerPkg,
        PermissionConstants::PERMISSION_EXEMPT_AS_CALLER, userId)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "jump from exempt caller app");
        return true;
    }
    if (CheckIfExemptByBundleName(controlRule.targetPkg,
        PermissionConstants::PERMISSION_EXEMPT_AS_TARGET, userId)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "jump to exempt app");
        return true;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "third-party apps to third-party apps");
    return false;
}

bool AbilityJumpInterceptor::CheckIfExemptByBundleName(const std::string &bundleName,
    const std::string &permission, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    AppExecFwk::ApplicationInfo appInfo;
    if (!StartAbilityUtils::GetApplicationInfo(bundleName, userId, appInfo)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "getAppInfo failed");
        return false;
    }

    if (appInfo.isSystemApp) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "system app, bundle:%{public}s", bundleName.c_str());
        return true;
    }
    int32_t ret = Security::AccessToken::AccessTokenKit::VerifyAccessToken(appInfo.accessTokenId, permission, false);
    if (ret == Security::AccessToken::PermissionState::PERMISSION_DENIED) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "PERMISSION_DENIED.");
        return false;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "bundle:%{public}s verify permission:%{public}s succeed", bundleName.c_str(), permission.c_str());
    return true;
}

bool AbilityJumpInterceptor::LoadAppLabelInfo(Want &want,
    AppExecFwk::AppJumpControlRule &controlRule, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    AppExecFwk::ApplicationInfo callerAppInfo;
    StartAbilityUtils::GetApplicationInfo(controlRule.callerPkg, userId, callerAppInfo);
    AppExecFwk::ApplicationInfo targetAppInfo;
    StartAbilityUtils::GetApplicationInfo(controlRule.targetPkg, userId, targetAppInfo);
    want.SetParam(JUMP_DIALOG_CALLER_BUNDLE_NAME, controlRule.callerPkg);
    want.SetParam(JUMP_DIALOG_CALLER_MODULE_NAME, callerAppInfo.labelResource.moduleName);
    want.SetParam(JUMP_DIALOG_CALLER_LABEL_ID, static_cast<long long>(callerAppInfo.labelId));
    want.SetParam(JUMP_DIALOG_TARGET_MODULE_NAME, targetAppInfo.labelResource.moduleName);
    want.SetParam(JUMP_DIALOG_TARGET_LABEL_ID, static_cast<long long>(targetAppInfo.labelId));
    return true;
}
} // namespace AAFwk
} // namespace OHOS