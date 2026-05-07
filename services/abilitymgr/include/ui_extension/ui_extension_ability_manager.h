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

#ifndef OHOS_ABILITY_RUNTIME_UI_EXTENSION_ABILITY_MANAGER_H
#define OHOS_ABILITY_RUNTIME_UI_EXTENSION_ABILITY_MANAGER_H

#include "ability_connect_manager.h"

#include "extension_record_manager.h"
#include "nocopyable.h"

namespace OHOS {
namespace AAFwk {

using UIExtensionAbilityConnectManager = AbilityRuntime::ExtensionRecordManager;
using UIExtensionSessionInfo = AbilityRuntime::UIExtensionSessionInfo;

/**
 * @class UIExtensionAbilityManager
 * UIExtensionAbilityManager provides facility for managing UI extension ability connection.
 */
class UIExtensionAbilityManager : public AbilityConnectManager {
public:
    using UIExtWindowMapValType = std::pair<std::weak_ptr<BaseExtensionRecord>, sptr<SessionInfo>>;
    using UIExtensionMapType = std::map<sptr<IRemoteObject>, UIExtWindowMapValType>;

    explicit UIExtensionAbilityManager(int userId);
    virtual ~UIExtensionAbilityManager();

    /**
     * PreloadUIExtensionAbilityLocked, preload uiextension ability.
     *
     * @param abilityRequest, Special want for service type's ability.
     * @param hostBundleName, the caller application bundle name.
     * @param hostPid, the caller hostPid.
     * @return Returns ERR_OK on success, others on failure.
     */
    int PreloadUIExtensionAbilityLocked(const AbilityRequest &abilityRequest, std::string &hostBundleName,
        int32_t hostPid = AAFwk::DEFAULT_INVAL_VALUE);
    
    /**
     * PreloadUIExtensionAbilityInner, preload uiextension ability.
     *
     * @param abilityRequest, Special want for service type's ability.
     * @param hostBundleName, the caller application bundle name.
     * @param hostPid, the caller hostPid.
     * @return Returns ERR_OK on success, others on failure.
     */
    int PreloadUIExtensionAbilityInner(const AbilityRequest &abilityRequest, std::string &hostBundleName,
        int32_t hostPid = AAFwk::DEFAULT_INVAL_VALUE);
 
    /**
     * Query preload uiextension record.
     *
     * @param element, The uiextension ElementName.
     * @param moduleName, The uiextension moduleName.
     * @param hostBundleName, The uiextension caller hostBundleName.
     * @param recordNum, The returned count of uiextension.
     * @return Returns ERR_OK on success, others on failure.
     */
    int QueryPreLoadUIExtensionRecordInner(const AppExecFwk::ElementName &element,
                                           const std::string &moduleName,
                                           const int32_t hostPid,
                                           int32_t &recordNum);
    /**
     * UnloadUIExtensionAbility, unload uiextension ability.
     *
     * @param abilityRecord, uiextension ability record.
     * @param hostBundleName, the caller application bundle name.
     * @return Returns ERR_OK on success, others on failure.
     */
    int UnloadUIExtensionAbility(const std::shared_ptr<AAFwk::BaseExtensionRecord> &abilityRecord, pid_t &hostPid);

    /**
     * ClearPreloadUIExtensionRecord, clear preload uiextension record.
     *
     * @param abilityRecord, uiextension ability record.
     */
    void ClearPreloadUIExtensionRecord(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);

    int AttachAbilityThreadInner(const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token) override;

    void OnAbilityRequestDone(const sptr<IRemoteObject> &token, const int32_t state) override;

    /**
     * GetUIExtensionBySessionInfo.
     *
     * @param sessionToken, service ability's session token.
     * @return Returns AbilityRecord shared_ptr.
     */
    std::shared_ptr<BaseExtensionRecord> GetUIExtensionBySessionInfo(const sptr<SessionInfo> &sessionInfo);

    /**
     * @brief Get extensionList by pid.
     * @param pid Process id.
     * @param extensionList UIExtensionAbility name list.
     */
    int32_t GetActiveUIExtensionList(const int32_t pid, std::vector<std::string> &extensionList);

    /**
     * @brief Get extensionList by bundleName.
     * @param bundleName The application bundle name.
     * @param extensionList UIExtensionAbility name list.
     */
    int32_t GetActiveUIExtensionList(const std::string &bundleName, std::vector<std::string> &extensionList);

    /**
     * @brief Get extensionList by uid.
     * @param uid The application uid.
     * @param extensionList UIExtensionAbility name list.
     */
    int32_t GetActiveUIExtensionListByUid(int32_t uid, std::vector<std::string> &extensionList);

    void BackgroundAbilityWindowLocked(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo);

    bool IsUIExtensionFocused(uint32_t uiExtensionTokenId, const sptr<IRemoteObject>& focusToken);

    sptr<IRemoteObject> GetUIExtensionSourceToken(const sptr<IRemoteObject> &token);

    int32_t GetUIExtensionSessionInfo(const sptr<IRemoteObject> token, UIExtensionSessionInfo &uiExtensionSessionInfo);

    void GetUIExtensionCallerTokenList(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        std::list<sptr<IRemoteObject>> &callerList);

    std::shared_ptr<AAFwk::AbilityRecord> GetUIExtensionRootHostInfo(const sptr<IRemoteObject> token);

    sptr<IRemoteObject> GetUIExtensionRootHostToken(const sptr<IRemoteObject> token);

    int UnPreloadUIExtensionAbilityLocked(int32_t extensionAbilityId);
    int UnPreloadUIExtensionAbilityInner(int32_t extensionAbilityId);
    int ClearAllPreloadUIExtensionAbilityLocked();
    int ClearAllPreloadUIExtensionAbilityInner();
    int32_t RegisterPreloadUIExtensionHostClient(const sptr<IRemoteObject> &callerToken);
    int32_t UnRegisterPreloadUIExtensionHostClient(int32_t callerPid) override;

protected:
    int32_t StartAbilityLocked(const AbilityRequest &abilityRequest) override;
    void HandleLoadAbilityOrStartSpecifiedProcess(
        const AbilityRuntime::LoadParam &loadParam, const std::shared_ptr<BaseExtensionRecord> &abilityRecord) override;
    int RemoveUIExtensionBySessionInfoToken(sptr<IRemoteObject> token) override;
private:

    int TerminateAbilityLocked(const sptr<IRemoteObject> &token) override;

    void SetLastExitReason(const AbilityRequest &abilityRequest, std::shared_ptr<BaseExtensionRecord> &targetService);

    bool IsCallerValid(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);

    void HandleUIExtensionDied(const std::shared_ptr<BaseExtensionRecord> &abilityRecord) override;

    void DoForegroundUIExtension(
        std::shared_ptr<BaseExtensionRecord> abilityRecord, const AbilityRequest &abilityRequest);

    void DoBackgroundAbilityWindow(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo) override;

    void AddUIExtWindowDeathRecipient(const sptr<IRemoteObject> &session);

    void RemoveUIExtWindowDeathRecipient(const sptr<IRemoteObject> &session) override;

    void OnUIExtWindowDied(const wptr<IRemoteObject> &remote);

    void HandleUIExtWindowDiedTask(const sptr<IRemoteObject> &remote);

    bool IsUIExtensionAbility(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);

    bool CheckUIExtensionAbilitySessionExist(const std::shared_ptr<BaseExtensionRecord> &abilityRecord) override;

    void RollbackAgentUILaunchRecord(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);

    void RemoveUIExtensionAbilityRecord(const std::shared_ptr<BaseExtensionRecord> &abilityRecord) override;

    void AddUIExtensionAbilityRecordToTerminatedList(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);

    int32_t GetOrCreateExtensionRecord(const AbilityRequest &abilityRequest, bool isCreatedByConnect,
        const std::string &hostBundleName, std::shared_ptr<BaseExtensionRecord> &extensionRecord, bool &isLoaded);

    int32_t GetOrCreateExtensionRecord(const AbilityRequest &abilityRequest,
        std::shared_ptr<BaseExtensionRecord> &targetService, bool &isLoadedAbility) override;

    void UpdateUIExtensionInfo(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        int32_t hostPid = AAFwk::DEFAULT_INVAL_VALUE);

    void UpdateUIExtensionBindInfo(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        std::string callerBundleName, int32_t notifyProcessBind);

    int32_t AddPreloadUIExtensionRecord(const std::shared_ptr<AAFwk::BaseExtensionRecord> abilityRecord) override;

    int TerminateAbilityInner(const sptr<IRemoteObject> &token) override;

    void HandleStartTimeoutTaskInner(const std::shared_ptr<BaseExtensionRecord> &abilityRecord) override;

    void LoadTimeout(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);

    void HandleForegroundTimeoutTaskInner(const std::shared_ptr<BaseExtensionRecord> &abilityRecord) override;

    /**
    * Handle ability foreground timeout.
    *
    * @param abilityRecord The ability record that timed out.
    */
    void ForegroundTimeout(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);

    void HandleStopTimeoutTaskInner(const std::shared_ptr<BaseExtensionRecord> &abilityRecord) override;

    /**
    * Handle ability termination timeout.
    *
    * @param abilityRecord The ability record that timed out.
    */
    void TerminateTimeout(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);

    void CleanActivatingTimeoutAbilityInner(std::shared_ptr<BaseExtensionRecord> abilityRecord) override;

    void TerminateDone(const std::shared_ptr<BaseExtensionRecord> &abilityRecord) override;

    bool HandleExtensionAbilityRemove(const std::shared_ptr<BaseExtensionRecord> &abilityRecord) override;

    void HandleAbilityDiedTaskInner(const std::shared_ptr<BaseExtensionRecord> &abilityRecord) override;

    void PostLoadTimeoutTask(const std::shared_ptr<BaseExtensionRecord> &abilityRecord, int32_t loadTimeout) override;

    int DispatchForeground(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        const sptr<IRemoteObject> &token) override;
    int DispatchBackground(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        const sptr<IRemoteObject> &token) override;
    int DispatchInactive(const std::shared_ptr<BaseExtensionRecord> &abilityRecord, int state,
        const sptr<IRemoteObject> &token) override;

    int CheckAbilityStateForDisconnect(const std::shared_ptr<BaseExtensionRecord> &abilityRecord) override;
    int CleanupConnectionAndTerminateIfNeeded(std::shared_ptr<BaseExtensionRecord> &abilityRecord) override;

    /**
     * @brief schedule to background
     *
     * @param abilityRecord the ability to move
     */
    void MoveToBackground(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);
    
    void BackgroundTimeout(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);

    /**
    * Handle successful preload of UI Extension.
    *
    * @param extensionRecordId The ID of the extension record.
    * @param isPreloadedSuccess Whether preloading was successful.
    */
    void HandlePreloadUIExtensionSuccess(int32_t extensionRecordId, bool isPreloadedSuccess);

    void CompleteForegroundInner(const std::shared_ptr<BaseExtensionRecord> &abilityRecord) override;

    int32_t ConnectAbilityLockedInner(bool isLoadedAbility,
        std::shared_ptr<BaseExtensionRecord>& targetService, const AbilityRequest& abilityRequest,
        std::shared_ptr<ConnectionRecord>& connectRecord) override;

    void TerminateOrCacheAbility(std::shared_ptr<BaseExtensionRecord> abilityRecord) override;
    void HandleCommandDestroy(const sptr<SessionInfo> &sessionInfo) override;
    void CompleteBackground(const std::shared_ptr<BaseExtensionRecord> &abilityRecord) override;

    class PreloadUIExtensionHostClientDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        using PreloadUIExtensionHostClientDiedHandler = std::function<void(const wptr<IRemoteObject> &)>;
        explicit PreloadUIExtensionHostClientDeathRecipient(PreloadUIExtensionHostClientDiedHandler handler);
        ~PreloadUIExtensionHostClientDeathRecipient() = default;
        void OnRemoteDied(const wptr<IRemoteObject> &remote) final;

    private:
        PreloadUIExtensionHostClientDiedHandler diedHandler_;
    };
private:
    RecipientMapType uiExtRecipientMap_;
    UIExtensionMapType uiExtensionMap_;
    std::unique_ptr<UIExtensionAbilityConnectManager> uiExtensionAbilityRecordMgr_ = nullptr;
    std::mutex uiExtRecipientMapMutex_;
    std::mutex uiExtensionMapMutex_;
    std::mutex preloadUIExtRecipientMapMutex_;
    std::map<int32_t, sptr<IRemoteObject::DeathRecipient>> preloadUIExtensionHostClientDeathRecipients_;

    DISALLOW_COPY_AND_MOVE(UIExtensionAbilityManager);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_UI_EXTENSION_ABILITY_MANAGER_H