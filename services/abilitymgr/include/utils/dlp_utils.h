/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_DLP_UTILS_H
#define OHOS_ABILITY_RUNTIME_DLP_UTILS_H

#include "ability_record.h"
#include "bundle_mgr_helper.h"
#ifdef WITH_DLP
#include "dlp_permission_kit.h"
#endif // WITH_DLP
#include "global_constant.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "in_process_call_wrapper.h"
#include "iremote_object.h"
#include "permission_verification.h"
#include "server_constant.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
namespace DlpUtils {
#ifdef WITH_DLP
using Dlp = Security::DlpPermission::DlpPermissionKit;
#endif // WITH_DLP

[[maybe_unused]]static bool DlpAccessOtherAppsCheck(const sptr<IRemoteObject> &callerToken, const Want &want)
{
#ifdef WITH_DLP
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (isSaCall) {
        return true;
    }
    if (callerToken == nullptr) {
        return true;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "caller null");
        return true;
    }
    int32_t appIndex = abilityRecord->GetAppIndex();
    if (AbilityRuntime::GlobalConstant::IsAppCloneIndex(appIndex) ||
        AbilityRuntime::GlobalConstant::IsSandboxCloneIndex(appIndex)) {
        return true;
    }
    if (abilityRecord->GetApplicationInfo().bundleName == want.GetBundle()) {
        return true;
    }
    int32_t uid = abilityRecord->GetApplicationInfo().uid;
    Security::DlpPermission::SandBoxExternalAuthorType authResult;
    int result = Dlp::GetSandboxExternalAuthorization(uid, want, authResult);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetSandboxExternalAuthorization failed %{public}d", result);
        return false;
    }
    if (authResult != Security::DlpPermission::SandBoxExternalAuthorType::ALLOW_START_ABILITY) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Auth failed, not allow start %{public}d", uid);
        return false;
    }
#endif // WITH_DLP
    return true;
}

[[maybe_unused]]static bool OtherAppsAccessDlpCheck(const sptr<IRemoteObject> &callerToken, const Want &want)
{
#ifdef WITH_DLP
    int32_t dlpIndex = want.GetIntParam(AbilityRuntime::ServerConstant::DLP_INDEX, 0);
    if (dlpIndex != 0 &&
        (AbilityRuntime::GlobalConstant::IsAppCloneIndex(dlpIndex) ||
         AbilityRuntime::GlobalConstant::IsSandboxCloneIndex(dlpIndex))) {
        return false;
    }

    if (callerToken != nullptr) {
        auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
        if (abilityRecord != nullptr &&
            AbilityRuntime::GlobalConstant::IsDlpIndex(abilityRecord->GetAppIndex())) {
            return true;
        }
    }

    return PermissionVerification::GetInstance()->VerifyDlpPermission(const_cast<Want &>(want));
#else
    return true;
#endif // WITH_DLP
}


[[maybe_unused]]static bool SandboxAuthCheck(const AbilityRecord &callerRecord, const Want &want)
{
#ifdef WITH_DLP
    int32_t uid = callerRecord.GetApplicationInfo().uid;
    Security::DlpPermission::SandBoxExternalAuthorType authResult;
    int result = Dlp::GetSandboxExternalAuthorization(uid, want, authResult);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetSandboxExternalAuthorization failed %{public}d", result);
        return false;
    }
    if (authResult != Security::DlpPermission::SandBoxExternalAuthorType::ALLOW_START_ABILITY) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Auth failed, not allow start %{public}d", uid);
        return false;
    }
#endif // WITH_DLP
    return true;
}

static bool CheckCallerIsDlpManager(const std::shared_ptr<AppExecFwk::BundleMgrHelper> &bundleManager)
{
    if (!bundleManager) {
        return false;
    }

    std::string bundleName;
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (IN_PROCESS_CALL(bundleManager->GetNameForUid(callerUid, bundleName)) != ERR_OK) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "Get Bundle Name failed");
        return false;
    }
    if (bundleName != "com.ohos.dlpmanager") {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "Wrong Caller");
        return false;
    }
    return true;
}

static bool AccessCheck(const sptr<IRemoteObject> &callerToken, const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "CHECK_DLP");
    return DlpAccessOtherAppsCheck(callerToken, want) && OtherAppsAccessDlpCheck(callerToken, want);
}
}  // namespace DlpUtils
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_DLP_UTILS_H
