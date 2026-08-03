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

#ifndef OHOS_ABILITY_RUNTIME_APP_UTILS_H
#define OHOS_ABILITY_RUNTIME_APP_UTILS_H

#include <mutex>
#include <string>
#include "nocopyable.h"

namespace OHOS {
namespace AAFwk {
constexpr const int32_t DEFAULT_MAX_EXT_PER_PROC = 10;
constexpr const int32_t DEFAULT_MAX_EXT_PER_DEV = 100;
constexpr const int32_t DEFAULT_INVALID_VALUE = -1;
constexpr const int32_t DEFAULT_MAX_CHILD_PROCESS = 0;
template<typename T>
class DeviceConfiguration {
public:
    bool isLoaded = false;
    T value;
};

/**
 * @class AppUtils
 * provides app utils.
 */
class AppUtils {
public:
    /**
     * GetInstance, get an instance of AppUtils.
     *
     * @return An instance of AppUtils.
     */
    static AppUtils &GetInstance();

    ~AppUtils();

    bool IsLauncher(const std::string &bundleName) const;
    bool IsLauncherAbility(const std::string &abilityName) const;
    bool IsInheritWindowSplitScreenMode();
    bool IsSupportAncoApp();
    int32_t GetTimeoutUnitTimeRatio();
    bool IsSelectorDialogDefaultPossion();
    bool IsStartSpecifiedProcess();
    bool IsUseMultiRenderProcess();
    bool IsLimitMaximumOfRenderProcess();
    bool IsGrantPersistUriPermission();
    bool IsStartOptionsWithAnimation();
    bool IsMultiProcessModel();
    bool IsStartOptionsWithProcessOptions();
    bool EnableMoveUIAbilityToBackgroundApi();
    bool IsLaunchEmbededUIAbility();
    bool IsSupportNativeChildProcess();
    bool IsSupportMultiInstance();
    bool IsAllowResidentInExtremeMemory(const std::string& bundleName, const std::string& abilityName = "");
    bool IsAllowNativeChildProcess(const std::string &appIdentifier);
    int32_t GetLimitMaximumExtensionsPerProc();
    bool IsBigMemoryUnrelatedKeepAliveProc(const std::string &bundleName);
    bool IsRequireBigMemoryProcess(const std::string &bundleName);
    int32_t GetLimitMaximumExtensionsPerDevice();
    std::string GetCacheExtensionTypeList();
    std::string GetBrokerDelegateBundleName();
    int32_t GetCollaboratorBrokerUID();
    int32_t GetCollaboratorBrokerReserveUID();
    int32_t MaxChildProcess();
    std::string GetMigrateClientBundleName();
    bool IsConnectSupportCrossUser();
    bool IsSupportAppServiceExtension();
    bool IsPrepareTerminateEnabled();
    bool IsCacheExtensionAbilityByList(const std::string& bundleName, const std::string& abilityName);
    bool IsSystemReasonMessage(const std::string &reasonMessage);
    bool InResidentWhiteList(const std::string &bundleName);
    const std::vector<std::string> &GetResidentWhiteList();

private:
    void LoadResidentProcessInExtremeMemory();
    void LoadAllowNativeChildProcessApps();
    bool IsCacheAbilityEnabled();
    void LoadCacheAbilityList();
    void LoadProcessProhibitedFromRestarting();
    void LoadRequireBigMemoryApp();
    void LoadResidentWhiteList();
    AppUtils();

    volatile bool isSceneBoard_ = false;
    volatile DeviceConfiguration<bool> isInheritWindowSplitScreenMode_ = {false, true};
    volatile DeviceConfiguration<bool> isSupportAncoApp_ = {false, false};
    volatile DeviceConfiguration<int32_t> timeoutUnitTimeRatio_ = {false, 1};
    volatile DeviceConfiguration<bool> isSelectorDialogDefaultPossion_ = {false, true};
    volatile DeviceConfiguration<bool> isStartSpecifiedProcess_ = {false, false};
    volatile DeviceConfiguration<bool> isUseMultiRenderProcess_ = {false, true};
    volatile DeviceConfiguration<bool> isLimitMaximumOfRenderProcess_ = {false, true};
    volatile DeviceConfiguration<bool> isGrantPersistUriPermission_ = {false, false};
    volatile DeviceConfiguration<bool> isStartOptionsWithAnimation_ = {false, false};
    volatile DeviceConfiguration<bool> isMultiProcessModel_ = {false, false};
    volatile DeviceConfiguration<bool> isStartOptionsWithProcessOptions_ = {false, false};
    volatile DeviceConfiguration<bool> enableMoveUIAbilityToBackgroundApi_ = {false, true};
    volatile DeviceConfiguration<bool> isLaunchEmbededUIAbility_ = {false, false};
    volatile DeviceConfiguration<bool> isSupportNativeChildProcess_ = {false, false};
    volatile DeviceConfiguration<bool> isSupportMultiInstance_ = {false, false};
    std::mutex isConnectSupportCrossUserMutex_;
    volatile DeviceConfiguration<bool> isConnectSupportCrossUser_ = {false, false};
    volatile DeviceConfiguration<bool> isSupportAppServiceExtension_ = {false, false};
    DeviceConfiguration<std::vector<std::pair<std::string, std::string>>>
        residentProcessInExtremeMemory_ = {false, {}};
    std::mutex residentProcessInExtremeMemoryMutex_;
    DeviceConfiguration<std::vector<std::string>> processProhibitedFromRestarting_ = {false, {}};
    std::mutex processProhibitedFromRestartingMutex_;
    DeviceConfiguration<std::vector<std::string>> requireBigMemoryApp_ = {false, {}};
    std::mutex requireBigMemoryAppMutex_;
    DeviceConfiguration<std::vector<std::string>>
        allowStartNativeProcessApps_ = {false, {}};
    volatile DeviceConfiguration<int32_t> limitMaximumExtensionsPerProc_ = {false, DEFAULT_MAX_EXT_PER_PROC};
    volatile DeviceConfiguration<int32_t> limitMaximumExtensionsPerDevice_ = {false, DEFAULT_MAX_EXT_PER_DEV};
    DeviceConfiguration<std::string> brokerDelegateBundleName_ = {false, ""};
    volatile DeviceConfiguration<int32_t> collaboratorBrokerUid_ = {false, DEFAULT_INVALID_VALUE};
    volatile DeviceConfiguration<int32_t> collaboratorBrokerReserveUid_ = {false, DEFAULT_INVALID_VALUE};
    volatile DeviceConfiguration<int32_t> maxChildProcess_ = {false, DEFAULT_MAX_CHILD_PROCESS};
    DeviceConfiguration<std::string> migrateClientBundleName_ = {true, "com.huwei.hmos.migratecilent"};
    DeviceConfiguration<std::vector<std::pair<std::string, std::string>>>
        cacheAbilityList_ = {false, {}};
    DeviceConfiguration<std::vector<std::string>> residentWhiteList_ = {false, {}};
    std::mutex residentWhiteListMutex_;
    DISALLOW_COPY_AND_MOVE(AppUtils);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APP_UTILS_H
