/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_LAUNCH_SERVICE_MANAGER_H
#define OHOS_ABILITY_RUNTIME_LAUNCH_SERVICE_MANAGER_H

#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "daemon_service_info.h"
#include "launch_service.h"
#include "launch_service_info.h"
#include "launch_service_observer.h"
#include "task_handler_wrap.h"

namespace OHOS {
namespace AppExecFwk {
class AppMgrServiceInner;
class BundleMgrHelper;
struct BundleInfo;
struct AppSpawnStartMsg;

class LaunchServiceManager : public std::enable_shared_from_this<LaunchServiceManager> {
public:
    explicit LaunchServiceManager(const std::weak_ptr<AppMgrServiceInner> &appMgrServiceInner);

    int32_t RegisterDaemon(const AAFwk::LaunchServiceDescriptor &descriptor, int32_t callerUid);
    int32_t UnregisterDaemon(const AAFwk::LaunchServiceDescriptor &descriptor, int32_t callerUid);
    int32_t StartDaemon(const AAFwk::LaunchServiceDescriptor &descriptor, int32_t callerUid);
    int32_t StopDaemon(const AAFwk::LaunchServiceDescriptor &descriptor, int32_t callerUid);
    int32_t GetLaunchServiceInformation(const AbilityRuntime::LaunchServiceDescriptor &descriptor,
        AbilityRuntime::LaunchServiceInformation &information, int32_t callerUid);
    int32_t GetAllRegisteredInformation(
        std::vector<AbilityRuntime::LaunchServiceInformation> &informations, int32_t userId, int32_t callerUserId);
    void SetLaunchServicesEnabled(const std::string &bundleName, int32_t userId, int32_t appIndex,
        const std::vector<uint8_t> &token, bool enable, int32_t callerUid, std::function<void(int32_t)> completion);

    int32_t RegisterLaunchServiceObserver(
        const sptr<AbilityRuntime::ILaunchServiceObserver> &observer, int32_t callerUid);
    int32_t UnregisterLaunchServiceObserver(const sptr<AbilityRuntime::ILaunchServiceObserver> &observer);

    void StartLaunchServices(const std::string &bundleName, int32_t userId, int32_t appIndex, bool runAtLoadOnly);
    void HandleLaunchServiceProcessDied(int32_t pid);
    void HandleLaunchServiceUninstallOrUpgrade(const std::string &bundleName, int32_t uid, bool isUpgrade);
    void HandleLaunchServiceUpgradeEnd(int32_t uid);

private:
    struct LaunchServiceKey {
        std::string bundleName;
        std::string moduleName;
        std::string serviceName;
        int32_t userId = 0;
        int32_t appIndex = 0;

        bool operator==(const LaunchServiceKey &other) const;

        struct Hash {
            size_t operator()(const LaunchServiceKey &key) const;
        };
    };
    struct LaunchServiceRuntimeRecord {
        LaunchService service;
        int32_t uid = -1;
        int32_t pid = 0;
        uint32_t restartCount = 0;
        uint64_t generation = 0;
        std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
    };
    int32_t CheckLaunchServiceStartLocked(const LaunchServiceKey &key, AAFwk::LaunchServiceDescriptor &descriptor);
    int32_t CheckLaunchServiceWatcher(const LaunchServiceKey &key, int32_t pid, bool watching);
    std::function<void(int32_t)> CreateLaunchServicesEnabledCompletion(const std::string &bundleName, int32_t userId,
        int32_t appIndex, bool enable, std::function<void(int32_t)> completion);
    int32_t StartLaunchService(const LaunchServiceKey &key, const LaunchService &service, uint32_t restartCount = 0,
        uint64_t expectedGeneration = 0);
    int32_t StartLaunchServiceLocked(const LaunchServiceKey &key, const LaunchService &service,
        uint32_t restartCount, uint64_t expectedGeneration);
    int32_t StopLaunchService(const LaunchServiceKey &key, int32_t expectedPid = 0);
    int32_t SpawnDaemonProcess(const AAFwk::LaunchServiceDescriptor &descriptor, int32_t &pid, int32_t &uid);
    void PrepareDaemonStartMsg(const std::shared_ptr<AppMgrServiceInner> &appMgrServiceInner,
        const std::shared_ptr<BundleMgrHelper> &bundleMgrHelper, const BundleInfo &bundleInfo,
        AppSpawnStartMsg &startMsg);
    int32_t StartDaemonProcess(
        const std::shared_ptr<AppMgrServiceInner> &appMgrServiceInner, AppSpawnStartMsg &startMsg, int32_t &pid);
    void NotifyLaunchServiceStateChanged(
        const AAFwk::LaunchServiceDescriptor &descriptor, AbilityRuntime::LaunchServiceState state);
    void NotifyLaunchServiceProcessStateChanged(
        const LaunchServiceKey &key, int32_t pid, AbilityRuntime::LaunchServiceProcessState state);
    void RestartLaunchService(const LaunchServiceKey &key, uint64_t generation);
    void StopLaunchServices(const std::string &bundleName, int32_t userId, int32_t appIndex);
    void UnregisterLaunchServicesByUid(int32_t uid);
    void CleanupLaunchServicesByUid(int32_t uid);
    void CleanupRemovedLaunchServices(int32_t uid);
    void UnregisterRemovedLaunchService(const AAFwk::LaunchServiceDescriptor &descriptor, int32_t uid);
    std::string GetLaunchServiceRestartTaskName(const LaunchServiceKey &key, uint64_t generation) const;
    std::shared_ptr<AAFwk::TaskHandlerWrap> GetTaskHandler() const;

    std::weak_ptr<AppMgrServiceInner> appMgrServiceInner_;
    mutable std::shared_mutex launchServiceMutex_;
    uint64_t launchServiceGeneration_ = 0;
    std::unordered_map<LaunchServiceKey, LaunchServiceRuntimeRecord, LaunchServiceKey::Hash>
        launchServiceRuntimeRecords_;
    std::unordered_map<int32_t, std::vector<LaunchServiceKey>> upgradingLaunchServiceKeys_;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_LAUNCH_SERVICE_MANAGER_H
