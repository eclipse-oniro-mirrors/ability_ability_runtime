/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_KEEP_ALIVE_PROCESS_MANAGER_H
#define OHOS_ABILITY_RUNTIME_KEEP_ALIVE_PROCESS_MANAGER_H

#include <functional>
#include <mutex>

#include "ability_manager_service.h"
#include "app_scheduler.h"
#include "bundle_info.h"
#include "ffrt.h"
#include "singleton.h"

namespace OHOS {
namespace AAFwk {
using AbilityKeepAliveService = AbilityRuntime::AbilityKeepAliveService;

struct KeepAliveAbilityInfo {
    int32_t userId = 0;
    int32_t appCloneIndex = 0;
    int32_t uid = 0;
    std::string bundleName;
    std::string moduleName;
    std::string abilityName;
};

struct KeepAliveRetryContext {
    KeepAliveAbilityInfo info;
    uint32_t accessTokenId = -1;
    bool isMultiInstance = false;
    int triedCount = 0;
};

class CheckStatusBarTask {
public:
    CheckStatusBarTask() = delete;

    CheckStatusBarTask(int32_t uid, std::function<void(void)>&& task)
        : uid_(uid), task_(task) {};

    ~CheckStatusBarTask() {};

    void Cancel();

    void Run();

    inline int32_t GetUid()
    {
        return uid_;
    }

private:
    int32_t uid_;
    ffrt::mutex cancelMutex_;
    std::function<void(void)> task_;
};

/**
 * @class KeepAliveProcessManager
 * KeepAliveProcessManager
 */
class KeepAliveProcessManager {
public:
    /**
     * Get the instance of KeepAliveProcessManager.
     *
     * @return Returns the instance of KeepAliveProcessManager.
     */
    static KeepAliveProcessManager &GetInstance();

    /**
     * Set the enable flag for keep-alive processes.
     *
     * @param bundleName, The bundle name of the keep-alive process.
     * @param userId, The user ID of the bundle.
     * @param updateEnable, Set value.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t SetApplicationKeepAlive(const std::string &bundleName, int32_t userId, bool updateEnable,
        bool isByEDM, bool isInner);

    /**
     * @brief Query keep-alive applications.
     * @param appType App type.
     * @param userId User id.
     * @param infoList Output parameters, return keep-alive info list.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t QueryKeepAliveApplications(int32_t appType, int32_t userId, std::vector<KeepAliveInfo> &infoList,
        bool isByEDM);

    /**
     * If bundle has right main element, start the main element
     *
     * @param bundleInfos bundles of keep-alive processes.
     * @param userId, The user ID of the bundle.
     */
    void StartKeepAliveProcessWithMainElement(std::vector<AppExecFwk::BundleInfo> &bundleInfos, int32_t userId);

    /**
     * Once one process created, query keep-alive status from db and update then
     *
     * @param appInfo The App info.
     */
    void OnAppStateChanged(const AppInfo &info);

    /**
     * Check if it is a keep-alive bundle under the specified user.
     *
     * @param bundleName, The bundle name of the keep-alive process.
     * @param userId, The user ID of the bundle.
     */
    bool IsKeepAliveBundle(const std::string &bundleName, int32_t userId);

    /**
     * query keep-alive bundles for user
     *
     * @param bundleInfos bundles of keep-alive processes.
     * @param userId, The user ID of the bundle.
     */
    bool GetKeepAliveBundleInfosForUser(std::vector<AppExecFwk::BundleInfo> &bundleInfos, int32_t userId);

    int32_t StartKeepAliveMainAbility(const KeepAliveAbilityInfo &info);

    void RemoveCheckStatusBarTask(int32_t uid, bool shouldCancel);

    /**
     * Set the enable flag for keep-alive app service extension.
     *
     * @param bundleName, The bundle name of the keep-alive app service extension.
     * @param updateEnable, Set value.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t SetAppServiceExtensionKeepAlive(const std::string &bundleName, bool updateEnable,
        bool isByEDM, bool isAllowUserToCancel);

     /**
     * @brief Query keep-alive app service extensions.
     * @param infoList Output parameters, return keep-alive info list.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t QueryKeepAliveAppServiceExtensions(std::vector<KeepAliveInfo> &infoList, bool isByEDM);

    /**
     * Start keep-alive app service extension.
     *
     * @param bundleInfos bundles of keep-alive app service extension.
     */
    void StartKeepAliveAppServiceExtension(std::vector<AppExecFwk::BundleInfo> &bundleInfos);

    int32_t ClearKeepAliveAppServiceExtension(int32_t userId);

    void SaveAppServiceRestartAfterUpgrade(const std::string &bundleName, int32_t uid);

    void SaveKeepAliveAppRestartAfterUpgrade(const std::string &bundleName, int32_t uid);

    bool CheckNeedRestartAfterUpgrade(int32_t uid);
    bool KeepAliveIsRestartAfterUpdate(int32_t uid);
    bool KeepAliveCheckNeedRestartAfterUpgrade(int32_t uid);
    void StartKeepAliveAfterAppUpgrade(std::vector<AppExecFwk::BundleInfo> &bundleInfos, int32_t uid);

    void FilterNeedRestartKeepAliveBundleInfos(std::vector<AppExecFwk::BundleInfo> &bundleInfos);
    void AddNeedRestartKeepAliveUid(int32_t uid);

private:
    KeepAliveProcessManager();
    ~KeepAliveProcessManager();

    int32_t CheckPermission();
    int32_t CheckPermissionForEDM();
    void StartKeepAliveProcessWithMainElementPerBundle(const AppExecFwk::BundleInfo &bundleInfo,
        int32_t userId);
    void AfterStartKeepAliveApp(const std::string &bundleName, uint32_t accessTokenId, int32_t uid, int32_t userId,
        bool isMultiInstance);
    bool IsRunningAppInStatusBar(const AppExecFwk::BundleInfo &bundleInfo);
    void StartKeepAliveAppServiceExtensionPerBundle(const AppExecFwk::BundleInfo &bundleInfo);
    int32_t StartKeepAliveAppServiceExtensionInner(const KeepAliveAbilityInfo &info);
    void ScheduleRetryTask(std::shared_ptr<KeepAliveRetryContext> context);

    ffrt::mutex checkStatusBarTasksMutex_;
    std::vector<std::shared_ptr<CheckStatusBarTask>> checkStatusBarTasks_;
    ffrt::mutex restartAfterUpgradeMutex_;
    std::set<int32_t> restartAfterUpgradeList_;
    std::set<int32_t> keepAliveRestartAfterUpgradeList_;

    DISALLOW_COPY_AND_MOVE(KeepAliveProcessManager);

    ffrt::mutex needRestartKeepAliveUidSetLock_;
    std::unordered_set<int32_t> needRestartKeepAliveUidSet_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_KEEP_ALIVE_PROCESS_MANAGER_H
