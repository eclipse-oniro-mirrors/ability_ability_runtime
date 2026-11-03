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

#ifndef OHOS_ABILITY_RUNTIME_RESIDENT_PROCESS_MANAGER_H
#define OHOS_ABILITY_RUNTIME_RESIDENT_PROCESS_MANAGER_H

#include <list>
#include <mutex>

#include "app_scheduler.h"
#include "bundle_info.h"
#include "singleton.h"

namespace OHOS {
namespace AAFwk {
struct ResidentAbilityInfo {
    std::string bundleName;
    std::string abilityName;
    int32_t userId = 0;
    int32_t residentId = -1;
};

class ResidentAbilityInfoGuard {
public:
    ResidentAbilityInfoGuard() = default;
    ~ResidentAbilityInfoGuard();
    ResidentAbilityInfoGuard(ResidentAbilityInfoGuard &) = delete;
    void operator=(ResidentAbilityInfoGuard &) = delete;
    ResidentAbilityInfoGuard(const std::string &bundleName, const std::string &abilityName, int32_t userId);
    void SetResidentAbilityInfo(const std::string &bundleName, const std::string &abilityName, int32_t userId);
private:
    int32_t residentId_ = -1;
};
/**
 * @class ResidentProcessManager
 * ResidentProcessManager
 */
class ResidentProcessManager : public std::enable_shared_from_this<ResidentProcessManager> {
    DECLARE_DELAYED_SINGLETON(ResidentProcessManager)
public:

    /**
     * Handle tasks such as initializing databases.
     *
    */
    void Init();

    /**
     * Set the enable flag for resident processes.
     *
     * @param bundleName, The bundle name of the resident process.
     * @param callerName, The name of the caller, usually the system application.
     * @param updateEnable, Set value, if true, start the resident process, If false, stop the resident process
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t SetResidentProcessEnabled(const std::string &bundleName, const std::string &callerName, bool updateEnable);

    /**
     * Set the enable flag for resident processes by sa caller.
     *
     * @param bundleName, The bundle name of the resident process.
     * @param callerUid, The uid of the sa caller, which must be in the sa uid list of the bundle.
     * @param updateEnable, Set value, if true, start the resident process, If false, stop the resident process
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t SetResidentProcessEnabledForSA(const std::string &bundleName, int32_t callerUid, bool updateEnable);

    /**
     * start empty resident processes.
     *
     * @param bundleInfos bundles of resident processes.
     */
    void StartResidentProcess(const std::vector<AppExecFwk::BundleInfo> &bundleInfos);
    /**
     * If bundle has right main element, start the main element
     */
    void StartResidentProcessWithMainElement(std::vector<AppExecFwk::BundleInfo> &bundleInfos, int32_t userId);
    /**
     * Once one process created, query keepalive status from db and update then
     */
    void OnAppStateChanged(const AppInfo &info);
    /**
     * Before starting a resident element, store it.
     */
    int32_t PutResidentAbility(const std::string &bundleName, const std::string &abilityName, int32_t userId);
    bool IsResidentAbility(const std::string &bundleName, const std::string &abilityName, int32_t userId);
    /**
     * After a resident element being started, remove it
     */
    void RemoveResidentAbility(int32_t residentId);
    /**
     * query resident bundles for user
     */
    bool GetResidentBundleInfosForUser(std::vector<AppExecFwk::BundleInfo> &bundleInfos, int32_t userId);
    void StartFailedResidentAbilities();
private:
    int32_t SetResidentProcessEnabledInner(const std::string &bundleName, bool updateEnable);
    void UpdateResidentProcessesStatus(const std::string &bundleName, bool localEnable, bool updateEnable);
    void AddFailedResidentAbility(const std::string &bundleName, const std::string &abilityName, int32_t userId);
    void StartResidentProcessWithMainElementPerBundle(const AppExecFwk::BundleInfo &bundleInfo,
        size_t index, std::set<uint32_t> &needEraseIndexSet, int32_t userId);
    void StartResidentProcessWithMainElementPerBundleHap(const AppExecFwk::HapModuleInfo &hapModuleInfo,
        const std::string &processName, size_t index, std::set<uint32_t> &needEraseIndexSet, int32_t userId);

    std::mutex residentAbilityInfoMutex_;
    std::list<ResidentAbilityInfo> residentAbilityInfos_;
    int32_t residentId_ = 0;

    std::mutex failedResidentAbilityInfoMutex_;
    std::list<ResidentAbilityInfo> failedResidentAbilityInfos_;
    std::atomic_bool unlockedAfterBoot_ = false;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_RESIDENT_PROCESS_MANAGER_H
