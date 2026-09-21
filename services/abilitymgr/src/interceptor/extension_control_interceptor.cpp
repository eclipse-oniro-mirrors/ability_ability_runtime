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

#include "interceptor/extension_control_interceptor.h"

#include "ability_manager_constants.h"
#include "app_scheduler.h"
#include "extension_config.h"
#include "hilog_tag_wrapper.h"
#include "start_ability_utils.h"

namespace OHOS {
namespace AAFwk {

ErrCode ExtensionControlInterceptor::DoProcess(AbilityInterceptorParam &param)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call.");
    if (param.GetContext<AbilityInterceptorParam::RemoteDispatchCtx>() != nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "remote dispatch, defer to remote device");
        return ERR_OK;
    }
    if (param.callerToken == nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "callerToken is nullptr.");
        return ERR_OK;
    }
    // get caller ability info
    AppExecFwk::AbilityInfo callerAbilityInfo;
    if (GetCallerAbilityInfo(param, callerAbilityInfo)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "caller enable.");
        return ERR_OK;
    }
    // get target ability info
    AppExecFwk::AbilityInfo targetAbilityInfo;
    if (GetTargetAbilityInfo(param, targetAbilityInfo)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "target enable.");
        return ERR_OK;
    }

    if (!DelayedSingleton<ExtensionConfig>::GetInstance()->HasAbilityAccess(callerAbilityInfo.extensionTypeName)) {
        return ProcessInterceptOld(param, targetAbilityInfo, callerAbilityInfo);
    }
    return ProcessInterceptNew(param, targetAbilityInfo, callerAbilityInfo);
}

int32_t ExtensionControlInterceptor::ProcessInterceptOld(const AbilityInterceptorParam& param,
    const AppExecFwk::AbilityInfo &targetAbilityInfo, const AppExecFwk::AbilityInfo &callerAbilityInfo)
{
    // check blocked list
    if (!targetAbilityInfo.applicationInfo.isSystemApp &&
        !DelayedSingleton<ExtensionConfig>::GetInstance()->IsExtensionStartThirdPartyAppEnable(
            callerAbilityInfo.extensionTypeName)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "third party app block extension call, bundleName: %{public}s",
            callerAbilityInfo.bundleName.c_str());
        return EXTENSION_BLOCKED_BY_THIRD_PARTY_APP_FLAG;
    }
    if ((targetAbilityInfo.extensionAbilityType == AppExecFwk::ExtensionAbilityType::SERVICE ||
        targetAbilityInfo.extensionAbilityType == AppExecFwk::ExtensionAbilityType::DATASHARE) &&
        !DelayedSingleton<ExtensionConfig>::GetInstance()->IsExtensionStartServiceEnable(
            callerAbilityInfo.extensionTypeName, param.want.GetElement())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "service list block extension call, bundleName: %{public}s",
            callerAbilityInfo.bundleName.c_str());
        return EXTENSION_BLOCKED_BY_SERVICE_LIST;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "other ok");
    return ERR_OK;
}

int32_t ExtensionControlInterceptor::ProcessInterceptNew(const AbilityInterceptorParam& param,
    const AppExecFwk::AbilityInfo &targetAbilityInfo, const AppExecFwk::AbilityInfo &callerAbilityInfo)
{
    auto extensionConfig = DelayedSingleton<ExtensionConfig>::GetInstance();
    if (!extensionConfig) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "extensionConfig null");
        return ERR_OK;
    }

    auto targetElement = param.want.GetElement();
    if (!targetAbilityInfo.applicationInfo.isSystemApp &&
        extensionConfig->HasThridPartyAppAccessFlag(callerAbilityInfo.extensionTypeName)) {
        if (!extensionConfig->IsExtensionStartThirdPartyAppEnableNew(
            callerAbilityInfo.extensionTypeName, targetElement)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "start third party app controlled by extension, bundleName: %{public}s",
                callerAbilityInfo.bundleName.c_str());
            return EXTENSION_BLOCKED_BY_THIRD_PARTY_APP_FLAG;
        }
        return ERR_OK;
    }

    auto isServiceOrDataShare = targetAbilityInfo.extensionAbilityType == AppExecFwk::ExtensionAbilityType::SERVICE ||
        targetAbilityInfo.extensionAbilityType == AppExecFwk::ExtensionAbilityType::DATASHARE;
    if (isServiceOrDataShare && extensionConfig->HasServiceAccessFlag(callerAbilityInfo.extensionTypeName)) {
        if (!extensionConfig->IsExtensionStartServiceEnableNew(callerAbilityInfo.extensionTypeName, targetElement)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR,
                "start service or datashare controlled by extension, bundleName: %{public}s",
                callerAbilityInfo.bundleName.c_str());
            return EXTENSION_BLOCKED_BY_SERVICE_LIST;
        }
        return ERR_OK;
    }

    if (extensionConfig->HasDefaultAccessFlag(callerAbilityInfo.extensionTypeName)) {
        if (!extensionConfig->IsExtensionStartDefaultEnable(callerAbilityInfo.extensionTypeName, targetElement)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "start ability controlled by extension, bundleName: %{public}s",
                callerAbilityInfo.bundleName.c_str());
            return ERR_EXTENSION_START_ABILITY_CONTROLEED;
        }
        return ERR_OK;
    }

    return ERR_OK;
}

bool ExtensionControlInterceptor::GetCallerAbilityInfo(const AbilityInterceptorParam& param,
    AppExecFwk::AbilityInfo& callerAbilityInfo)
{
    if (StartAbilityUtils::GetCallerAbilityInfo(param.callerToken, callerAbilityInfo)) {
        if (callerAbilityInfo.type != AppExecFwk::AbilityType::EXTENSION ||
            callerAbilityInfo.extensionAbilityType == AppExecFwk::ExtensionAbilityType::SERVICE ||
            callerAbilityInfo.bundleName == param.want.GetBundleNameRef()) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "not other extension.");
            return true;
        }
        auto appScheduler = DelayedSingleton<AppScheduler>::GetInstance();
        AppExecFwk::RunningProcessInfo processInfo;
        if (appScheduler != nullptr) {
            appScheduler->GetRunningProcessInfoByToken(param.callerToken, processInfo);
            if (!processInfo.isStrictMode && !param.want.GetBoolParam(STRICT_MODE, false)) {
                TAG_LOGD(AAFwkTag::ABILITYMGR, "caller and want not strict mode");
                return true;
            }
        }
    }
    return false;
}

bool ExtensionControlInterceptor::GetTargetAbilityInfo(const AbilityInterceptorParam& param,
    AppExecFwk::AbilityInfo& targetAbilityInfo)
{
    if (param.abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "param.abilityInfo is nullptr");
        return true;
    }
    targetAbilityInfo = *param.abilityInfo;
    return false;
}
} // namespace AAFwk
} // namespace OHOS