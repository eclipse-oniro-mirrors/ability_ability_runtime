/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

    /**
     * AppUtils, destructor.
     *
     */
    ~AppUtils();

    /**
     * IsLauncher, check if it is a launcher.
     *
     * @param bundleName The bundle name.
     * @return Whether it is a launcher.
     */
    bool IsLauncher(const std::string &bundleName) const;

    /**
     * IsLauncherAbility, check if it is a launcher ability.
     *
     * @param abilityName The ability name.
     * @return Whether it is a launcher ability.
     */
    bool IsLauncherAbility(const std::string &abilityName) const;

    /**
     * IsInheritWindowSplitScreenMode, check if it is inherit window split screen mode.
     *
     * @return Whether it is inherit window split screen mode.
     */
    bool IsInheritWindowSplitScreenMode();

    /**
     * IsSupportAncoApp, check if it supports anco app.
     *
     * @return Whether it supports anco app.
     */
    bool IsSupportAncoApp();

    /**
     * GetTimeoutUnitTimeRatio, get timeout unit time ratio.
     *
     * @return Timeout unit time ratio.
     */
    int32_t GetTimeoutUnitTimeRatio();

    /**
     * IsSelectorDialogDefaultPossion, check if selector dialog is on the default position.
     *
     * @return Whether selector dialog is on the default position.
     */
    bool IsSelectorDialogDefaultPossion();

    /**
     * IsStartSpecifiedProcess, check whether or not to start specified process.
     *
     * @return Whether or not to start specified process.
     */
    bool IsStartSpecifiedProcess();

    /**
     * IsUseMultiRenderProcess, check whether uses multi-render process.
     *
     * @return Whether uses multi-render process.
     */
    bool IsUseMultiRenderProcess();

    /**
     * IsLimitMaximumOfRenderProcess, check limit maximum of render process.
     *
     * @return Whether limit maximum of render process.
     */
    bool IsLimitMaximumOfRenderProcess();

    /**
     * IsGrantPersistUriPermission, check whether to grant persist uri permission.
     *
     * @return Whether to grant persist uri permission.
     */
    bool IsGrantPersistUriPermission();

    /**
     * IsStartOptionsWithAnimation, check whether the start options have animation.
     *
     * @return Whether the start options have animation.
     */
    bool IsStartOptionsWithAnimation();

    /**
     * IsStartOptionsWithAnimation, check whether it is a multi-process model.
     *
     * @return Whether it is a multi-process model.
     */
    bool IsMultiProcessModel();

    /**
     * IsStartOptionsWithProcessOptions, check whether the start options have process options.
     *
     * @return Whether the start options have process options.
     */
    bool IsStartOptionsWithProcessOptions();

    /**
     * EnableMoveUIAbilityToBackgroundApi, enable move ui-ability to background api.
     *
     * @return Whether the enable move ui-ability to background api is successful.
     */
    bool EnableMoveUIAbilityToBackgroundApi();

    /**
     * IsLaunchEmbededUIAbility, check if it is to launch embedded ui-ability.
     *
     * @return Whether it is to launch embedded ui-ability.
     */
    bool IsLaunchEmbededUIAbility();

    /**
     * IsSupportNativeChildProcess, check if it supports native child process.
     *
     * @return Whether it supports native child process.
     */
    bool IsSupportNativeChildProcess();

    /**
     * IsSupportMultiInstance, check if it supports multi-instance.
     *
     * @return Whether it supports multi-instance.
     */
    bool IsSupportMultiInstance();

    /**
     * IsAllowResidentInExtremeMemory, check if it allows resident in extrem low memory.
     *
     * @param bundleName The bundle name.
     * @param abilityName The ability name.
     * @return Whether it allows resident in extrem low memory.
     */
    bool IsAllowResidentInExtremeMemory(const std::string& bundleName, const std::string& abilityName = "");

    /**
     * IsAllowNativeChildProcess, check if it allows native child process.
     *
     * @param appIdentifier The app identifier.
     * @return Whether it allows native child process.
     */
    bool IsAllowNativeChildProcess(const std::string &appIdentifier);

    /**
     * GetLimitMaximumExtensionsPerProc, get limit max extensions per proc.
     *
     * @return Limit max extensions per proc.
     */
    int32_t GetLimitMaximumExtensionsPerProc();

    /**
     * GetLimitMaximumExtensionsPerDevice, get limit max extensions per device.
     *
     * @return Limit max extensions per device.
     */
    int32_t GetLimitMaximumExtensionsPerDevice();

    /**
     * GetCacheExtensionTypeList, get cache extension type list.
     *
     * @return Cache extension type list.
     */
    std::string GetCacheExtensionTypeList();

    /**
     * IsAllowStartAbilityWithoutCallerToken, check if it allows start ability without caller token.
     *
     * @param bundleName The bundle name.
     * @param abilityName The ability name.
     * @return Whether it allows start ability without caller token.
     */
    bool IsAllowStartAbilityWithoutCallerToken(const std::string& bundleName, const std::string& abilityName);

    /**
     * GetBrokerDelegateBundleName, get broker delegate bundle name.
     *
     * @return Broker delegate bundle name.
     */
    std::string GetBrokerDelegateBundleName();

    /**
     * GetCollaboratorBrokerUID, get collaborator broker id.
     *
     * @return Collaborator broker id.
     */
    int32_t GetCollaboratorBrokerUID();

    /**
     * GetCollaboratorBrokerReserveUID, get collaborator broker reserve uid.
     *
     * @return Collaborator broker reserve uid.
     */
    int32_t GetCollaboratorBrokerReserveUID();

    /**
     * MaxChildProcess, get max child process.
     *
     * @return Max child process.
     */
    int32_t MaxChildProcess();

    /**
     * GetMigrateClientBundleName, get migrate client bundle name.
     *
     * @return Migrate client bundle name.
     */
    std::string GetMigrateClientBundleName();

    /**
     * IsConnectSupportCrossUser, check if it support cross-user.
     *
     * @return Whether it supports cross-user.
     */
    bool IsConnectSupportCrossUser();

    /**
     * IsPrepareTerminateEnabled, check if it supports prepare terminate.
     *
     * @return Whether it supports prepare terminate.
     */
    bool IsPrepareTerminateEnabled();

    /**
     * IsCacheExtensionAbilityByList, check if it allows cache extension ability by list.
     *
     * @param bundleName The bundle name.
     * @param abilityName The ability name.
     * @return Whether it allows cache extensionability.
     */
    bool IsCacheExtensionAbilityByList(const std::string& bundleName, const std::string& abilityName);

private:
    /**
     * LoadResidentProcessInExtremeMemory, load resident process in extreme low memory.
     *
     */
    void LoadResidentProcessInExtremeMemory();

    /**
     * LoadAllowNativeChildProcessApps, load allow native child process apps.
     *
     */
    void LoadAllowNativeChildProcessApps();

    /**
     * LoadStartAbilityWithoutCallerToken, load start ability without caller token.
     *
     */
    void LoadStartAbilityWithoutCallerToken();

    /**
     * IsCacheAbilityEnabled, check cache ability parameter switch.
     *
     */
    bool IsCacheAbilityEnabled();

     /**
     * LoadCacheAbilityList, load cache ability list from file.
     *
     */
    void LoadCacheAbilityList();

    /**
     * AppUtils, private constructor.
     *
     */
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
    DeviceConfiguration<std::vector<std::pair<std::string, std::string>>>
        residentProcessInExtremeMemory_ = {false, {}};
    std::mutex residentProcessInExtremeMemoryMutex_;
    DeviceConfiguration<std::vector<std::string>>
        allowStartNativeProcessApps_ = {false, {}};
    volatile DeviceConfiguration<int32_t> limitMaximumExtensionsPerProc_ = {false, DEFAULT_MAX_EXT_PER_PROC};
    volatile DeviceConfiguration<int32_t> limitMaximumExtensionsPerDevice_ = {false, DEFAULT_MAX_EXT_PER_DEV};
    DeviceConfiguration<std::vector<std::pair<std::string, std::string>>>
        startAbilityWithoutCallerToken_ = {false, {}};
    std::mutex startAbilityWithoutCallerTokenMutex_;
    DeviceConfiguration<std::string> brokerDelegateBundleName_ = {false, ""};
    volatile DeviceConfiguration<int32_t> collaboratorBrokerUid_ = {false, DEFAULT_INVALID_VALUE};
    volatile DeviceConfiguration<int32_t> collaboratorBrokerReserveUid_ = {false, DEFAULT_INVALID_VALUE};
    volatile DeviceConfiguration<int32_t> maxChildProcess_ = {false, DEFAULT_MAX_CHILD_PROCESS};
    DeviceConfiguration<std::string> migrateClientBundleName_ = {true, "com.huwei.hmos.migratecilent"};
    DeviceConfiguration<std::vector<std::pair<std::string, std::string>>>
        cacheAbilityList_ = {false, {}};
    DISALLOW_COPY_AND_MOVE(AppUtils);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APP_UTILS_H
