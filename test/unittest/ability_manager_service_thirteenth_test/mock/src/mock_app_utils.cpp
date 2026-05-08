/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "mock_app_utils.h"
#include "mock_my_status.h"


namespace OHOS {
namespace AAFwk {

AppUtils::~AppUtils() {}

AppUtils::AppUtils() {}

AppUtils &AppUtils::GetInstance()
{
    static AppUtils utils;
    return utils;
}

bool AppUtils::IsLauncher(const std::string &bundleName) const
{
    return true;
}

bool AppUtils::IsLauncherAbility(const std::string &abilityName) const
{
    return true;
}

bool AppUtils::IsInheritWindowSplitScreenMode()
{
    return true;
}

bool AppUtils::IsSupportAncoApp()
{
    return true;
}

int32_t AppUtils::GetTimeoutUnitTimeRatio()
{
    return 0;
}

bool AppUtils::IsSelectorDialogDefaultPossion()
{
    return true;
}

bool AppUtils::IsStartSpecifiedProcess()
{
    return true;
}

bool AppUtils::IsUseMultiRenderProcess()
{
    return true;
}

bool AppUtils::IsLimitMaximumOfRenderProcess()
{
    return true;
}

bool AppUtils::IsGrantPersistUriPermission()
{
    return true;
}

bool AppUtils::IsStartOptionsWithAnimation()
{
    return true;
}

bool AppUtils::IsMultiProcessModel()
{
    return true;
}

bool AppUtils::IsStartOptionsWithProcessOptions()
{
    return true;
}

bool AppUtils::EnableMoveUIAbilityToBackgroundApi()
{
    return true;
}

bool AppUtils::IsLaunchEmbededUIAbility()
{
    return MyStatus::GetInstance().auIsLaunchEmbededUIAbility_;
}

bool AppUtils::IsSupportNativeChildProcess()
{
    return true;
}

bool AppUtils::IsAllowResidentInExtremeMemory(const std::string& bundleName, const std::string& abilityName)
{
    return true;
}

bool AppUtils::IsBigMemoryUnrelatedKeepAliveProc(const std::string &bundleName)
{
    return true;
}

bool AppUtils::IsRequireBigMemoryProcess(const std::string &bundleName)
{
    return true;
}

void AppUtils::LoadProcessProhibitedFromRestarting()
{
}

void AppUtils::LoadRequireBigMemoryApp()
{
}

void AppUtils::LoadResidentProcessInExtremeMemory()
{
}

bool AppUtils::IsAllowNativeChildProcess(const std::string &appIdentifier)
{
    return true;
}

void AppUtils::LoadAllowNativeChildProcessApps()
{
}

int32_t AppUtils::GetLimitMaximumExtensionsPerProc()
{
    return 0;
}

int32_t AppUtils::GetLimitMaximumExtensionsPerDevice()
{
    return 0;
}

std::string AppUtils::GetCacheExtensionTypeList()
{
    return "";
}

bool AppUtils::IsAllowStartAbilityWithoutCallerToken(const std::string& bundleName, const std::string& abilityName)
{
    return true;
}

void AppUtils::LoadStartAbilityWithoutCallerToken()
{
}

std::string AppUtils::GetBrokerDelegateBundleName()
{
    return "";
}

int32_t AppUtils::GetCollaboratorBrokerUID()
{
    return 0;
}

int32_t AppUtils::GetCollaboratorBrokerReserveUID()
{
    return 0;
}

int32_t AppUtils::MaxChildProcess()
{
    return 0;
}

bool AppUtils::IsSupportMultiInstance()
{
    return MyStatus::GetInstance().isSupportMultiInstance_;
}

std::string AppUtils::GetMigrateClientBundleName()
{
    return "";
}

bool AppUtils::IsConnectSupportCrossUser()
{
    return true;
}

bool AppUtils::IsPrepareTerminateEnabled()
{
    return true;
}

bool AppUtils::IsSystemReasonMessage(const std::string &reasonMessage)
{
    return true;
}

bool AppUtils::IsCacheAbilityEnabled()
{
    return true;
}

void AppUtils::LoadCacheAbilityList()
{
}

bool AppUtils::IsCacheExtensionAbilityByList(const std::string& bundleName, const std::string& abilityName)
{
    return true;
}

void AppUtils::LoadResidentWhiteList()
{
}

const std::vector<std::string>& AppUtils::GetResidentWhiteList()
{
    return {};
}

bool AppUtils::InResidentWhiteList(const std::string &bundleName)
{
    return true;
}

bool AppUtils::IsSupportAppServiceExtension()
{
    return true;
}

bool AppUtils::IsSupportStartAbilities()
{
    return MyStatus::GetInstance().isSupportStartAbilities_;
}

bool AppUtils::IsPreloadApplicationEnabled()
{
    return false;
}

bool AppUtils::IsForbidStart()
{
    return false;
}

bool AppUtils::IsSupportDelayedProcessExit()
{
    return MyStatus::GetInstance().auIsSupportDelayedProcessExit_;
}
}  // namespace AAFwk
}  // namespace OHOS
