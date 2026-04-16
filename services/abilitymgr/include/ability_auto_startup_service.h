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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_AUTO_STARTUP_SERVICE_H
#define OHOS_ABILITY_RUNTIME_ABILITY_AUTO_STARTUP_SERVICE_H

#include <functional>
#include <map>
#include <mutex>
#include <vector>

#include "auto_startup_info.h"
#include "ability_record.h"
#include "bundle_mgr_client.h"
#include "bundle_mgr_helper.h"
#include "iremote_object.h"
#include "singleton.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char* ABILITY_TYPE_UI_ABILITY = "UIAbility";
constexpr const char* EXTENSION_TYPE_APP_SERVICE = "AppServiceExtension";
constexpr const char* EXTENSION_TYPE_SERVICE_EXTENSION = "ServiceExtension";
} // namespace

class AbilityAutoStartupService : public std::enable_shared_from_this<AbilityAutoStartupService> {
public:
    explicit AbilityAutoStartupService();

    virtual ~AbilityAutoStartupService();

    /**
     * @brief Register auto start up callback for system api.
     * @param callback The point of JsAbilityAutoStartupCallBack.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t RegisterAutoStartupSystemCallback(const sptr<IRemoteObject> &callback);

    /**
     * @brief Unregister auto start up callback for system api.
     * @param callback The point of JsAbilityAutoStartupCallBack.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t UnregisterAutoStartupSystemCallback(const sptr<IRemoteObject> &callback);

    /**
     * @brief Set every application auto start up state.
     * @param info The auto startup info,include bundle name, module name, ability name.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t SetApplicationAutoStartup(const AutoStartupInfo &info);

    /**
     * @brief Cancel every application auto start up .
     * @param info The auto startup info,include bundle name, module name, ability name.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t CancelApplicationAutoStartup(const AutoStartupInfo &info);

    /**
     * @brief Query auto startup state all application.
     * @param infoList Output parameters, return auto startup info list.
     * @param infoList Input parameters, return userid.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t QueryAllAutoStartupApplications(std::vector<AutoStartupInfo> &infoList, int32_t userId);

    /**
     * @brief Retrieves the auto startup status of the current application.
     * @param callerTokenId the tokenId of caller application.
     * @param isAutoStartEnabled Indicates whether auto startup is enabled for the current application.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t GetAutoStartupStatusForSelf(uint32_t callerTokenId, bool &isAutoStartEnabled);

    /**
     * @brief Query auto startup state all application without permission.
     * @param infoList Output parameters, return auto startup info list.
     * @param infoList Input parameters, return userid.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t QueryAllAutoStartupApplicationsWithoutPermission(std::vector<AutoStartupInfo> &infoList, int32_t userId);

    /**
     * @brief Delete current bundleName auto start up data.
     * @param bundleName The current bundleName.
     * @param accessTokenId The accessTokenId.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t DeleteAutoStartupData(const std::string &bundleName, int32_t accessTokenId);

    /**
     * @brief Check current bundleName auto start up data.
     * @param bundleName The current bundleName.
     * @param uid The uid.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t CheckAutoStartupData(const std::string &bundleName, int32_t uid);

    /**
     * @brief Set application auto start up state by EDM.
     * @param info The auto startup info, include bundle name, module name, ability name.
     * @param flag Indicate whether the application is prohibited from changing the auto start up state.
     * @param isHiddenStart Indicate whether the application is hidden start.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t SetApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag, bool isHiddenStart = false);

    /**
     * @brief Cancel application auto start up state by EDM.
     * @param info The auto startup info, include bundle name, module name, ability name.
     * @param flag Indicate whether the application is prohibited from changing the auto start up state.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t CancelApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag);

    void AddHandledAutoStartupUsers(int32_t userId);
    void RemoveHandledAutoStartupUsers(int32_t userId);
    bool FindHandledAutoStartupUsers(int32_t userId);
    int32_t HiddenStartAutoStartupApp(AAFwk::Want &want, int32_t userId);
    std::function<void()> CreateHiddenStartCheckTask(std::shared_ptr<AAFwk::AbilityRecord> abilityRecord);

    /**
     * @class ClientDeathRecipient
     * notices IRemoteBroker died.
     */
    class ClientDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        /**
         * @brief Constructor
         */
        explicit ClientDeathRecipient(const std::weak_ptr<AbilityAutoStartupService> &weakPtr);
        virtual ~ClientDeathRecipient() = default;
        /**
         * @brief handle remote object died event.
         * @param remote remote object.
         */
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override;

    private:
        std::weak_ptr<AbilityAutoStartupService> weakPtr_;
    };

private:
    int32_t InnerSetApplicationAutoStartup(const AutoStartupInfo &info);
    int32_t InnerCancelApplicationAutoStartup(const AutoStartupInfo &info);
    void ExecuteCallbacks(bool isCallOn, const AutoStartupInfo &info);
    void SetDeathRecipient(
        const sptr<IRemoteObject> &callback, const sptr<IRemoteObject::DeathRecipient> &deathRecipient);
    void CleanResource(const wptr<IRemoteObject> &remote);
    std::string GetSelfApplicationBundleName();
    bool CheckSelfApplication(const std::string &bundleName);
    int32_t GetValidUserId(int32_t userId);
    bool GetBundleInfo(const std::string &bundleName, int32_t userId, int32_t appIndex,
        AppExecFwk::BundleInfo &bundleInfo);
    bool GetAbilityData(const AutoStartupInfo &info, AutoStartupAbilityData &abilityData);
    bool IsTargetAbility(const AutoStartupInfo &info, const AppExecFwk::AbilityInfo &abilityInfo);
    bool IsTargetExtension(const AutoStartupInfo &info, const AppExecFwk::ExtensionAbilityInfo &extensionInfo);
    std::string GetAbilityTypeName(const AppExecFwk::AbilityInfo &abilityInfo);
    std::string GetExtensionTypeName(const AppExecFwk::ExtensionAbilityInfo &extensionInfo);
    std::shared_ptr<AppExecFwk::BundleMgrClient> GetBundleMgrClient();
    int32_t CheckPermissionForSystem();
    int32_t CheckPermissionForSelf(const std::string &bundleName);
    int32_t CheckPermissionForEDM();
    int32_t InnerApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool isSet, bool flag);
    int32_t GetAbilityInfo(const AutoStartupInfo &info, AutoStartupAbilityData &abilityData);
    int32_t CheckHiddenStartConditions(bool isHiddenStart, const AutoStartupAbilityData &abilityData);
    void GetCallbackVector(std::vector<sptr<IRemoteObject>>& callbackVector);

    mutable std::mutex autoStartUpMutex_;
    mutable std::mutex deathRecipientsMutex_;
    std::vector<int32_t> handledAutoStartupUsers_;
    std::vector<sptr<IRemoteObject>> callbackVector_;
    std::map<sptr<IRemoteObject>, sptr<IRemoteObject::DeathRecipient>> deathRecipients_;
    std::shared_ptr<AppExecFwk::BundleMgrClient> bundleMgrClient_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ABILITY_AUTO_STARTUP_SERVICE_H