/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_CONTEXT_IMPL_H
#define OHOS_ABILITY_RUNTIME_ABILITY_CONTEXT_IMPL_H

#include "ability_context.h"

#include <uv.h>
#include <vector>
#include <unordered_map>

#include "context_impl.h"
#include "configuration.h"
#include "local_call_container.h"

namespace OHOS {
namespace Rosen {
enum class WSError;
}
namespace AbilityRuntime {
class AbilityContextImpl : public AbilityContext {
public:
    AbilityContextImpl();
    virtual ~AbilityContextImpl() = default;

    Global::Resource::DeviceType GetDeviceType() const override;
    std::string GetBaseDir() const override;
    std::string GetBundleCodeDir() override;
    std::string GetCacheDir() override;
    std::string GetTempDir() override;
    std::string GetResourceDir(const std::string &moduleName = "") override;
    std::string GetFilesDir() override;
    bool IsUpdatingConfigurations() override;
    bool PrintDrawnCompleted() override;
    std::string GetDatabaseDir() override;
    std::string GetGroupDir(std::string groupId) override;
    std::string GetPreferencesDir() override;
    std::string GetDistributedFilesDir() override;
    std::string GetCloudFileDir() override;
    std::string GetLogFileDir() override;
    int32_t GetSystemDatabaseDir(const std::string &groupId, bool checkExist, std::string &databaseDir) override;
    int32_t GetSystemPreferencesDir(const std::string &groupId, bool checkExist, std::string &preferencesDir) override;
    void SwitchArea(int mode) override;
    int GetArea() override;
    std::string GetProcessName() override;
    std::string GetBundleName() const override;
    std::shared_ptr<AppExecFwk::ApplicationInfo> GetApplicationInfo() const override;
    std::shared_ptr<Global::Resource::ResourceManager> GetResourceManager() const override;
    void SetAbilityResourceManager(std::shared_ptr<Global::Resource::ResourceManager> abilityResourceMgr) override;
    void RegisterAbilityConfigUpdateCallback(AbilityConfigUpdateCallback abilityConfigUpdateCallback) override;
    std::shared_ptr<AppExecFwk::Configuration> GetAbilityConfiguration() const override;
    void SetAbilityConfiguration(const AppExecFwk::Configuration &config) override;
    void SetAbilityColorMode(int32_t colorMode) override;
    void RegisterBindingObjectConfigUpdateCallback(BindingObjectConfigUpdateCallback callback) override;
    void NotifyBindingObjectConfigUpdate() override;
    std::shared_ptr<Context> CreateBundleContext(const std::string &bundleName) override;
    std::shared_ptr<Context> CreateModuleContext(const std::string &moduleName) override;
    std::shared_ptr<Context> CreateModuleContext(const std::string &bundleName, const std::string &moduleName) override;
    std::shared_ptr<Global::Resource::ResourceManager> CreateModuleResourceManager(
        const std::string &bundleName, const std::string &moduleName) override;
    int32_t CreateSystemHspModuleResourceManager(const std::string &bundleName,
        const std::string &moduleName, std::shared_ptr<Global::Resource::ResourceManager> &resourceManager) override;
    std::shared_ptr<Context> CreateAreaModeContext(int areaMode) override;
    std::shared_ptr<Context> CreateModuleOrPluginContext(const std::string &bundleName,
        const std::string &moduleName) override;
#ifdef SUPPORT_GRAPHICS
    std::shared_ptr<Context> CreateDisplayContext(uint64_t displayId) override;
#endif

    std::string GetBundleCodePath() const override;
    ErrCode StartAbility(const AAFwk::Want &want, int requestCode) override;
    ErrCode StartAbilityWithAccount(const AAFwk::Want &want, int accountId, int requestCode) override;
    ErrCode StartAbility(const AAFwk::Want &want, const AAFwk::StartOptions &startOptions, int requestCode) override;
    ErrCode StartAbilityAsCaller(const AAFwk::Want &want, int requestCode) override;
    ErrCode StartAbilityAsCaller(const AAFwk::Want &want, const AAFwk::StartOptions &startOptions,
        int requestCode) override;
    ErrCode StartAbilityWithAccount(
        const AAFwk::Want &want, int accountId, const AAFwk::StartOptions &startOptions, int requestCode) override;
    ErrCode StartAbilityForResult(const AAFwk::Want &want, int requestCode, RuntimeTask &&task) override;
    ErrCode StartAbilityForResultWithAccount(
        const AAFwk::Want &want, int accountId, int requestCode, RuntimeTask &&task) override;
    ErrCode StartAbilityForResultWithAccount(const AAFwk::Want &want, int accountId,
        const AAFwk::StartOptions &startOptions, int requestCode, RuntimeTask &&task) override;
    ErrCode StartAbilityForResult(const AAFwk::Want &want, const AAFwk::StartOptions &startOptions,
        int requestCode, RuntimeTask &&task) override;
    ErrCode StartUIServiceExtensionAbility(const AAFwk::Want &want, int32_t accountId = -1) override;
    ErrCode StartServiceExtensionAbility(const Want &want, int32_t accountId = -1) override;
    ErrCode StopServiceExtensionAbility(const Want& want, int32_t accountId = -1) override;
    ErrCode TerminateAbilityWithResult(const AAFwk::Want &want, int resultCode) override;
    ErrCode BackToCallerAbilityWithResult(const AAFwk::Want &want, int resultCode, int64_t requestCode) override;
    void OnAbilityResult(int requestCode, int resultCode, const AAFwk::Want &resultData) override;
    ErrCode ConnectAbility(const AAFwk::Want &want,
                        const sptr<AbilityConnectCallback> &connectCallback) override;
    ErrCode ConnectAbilityWithAccount(const AAFwk::Want &want, int accountId,
                        const sptr<AbilityConnectCallback> &connectCallback) override;
    ErrCode ConnectUIServiceExtensionAbility(const AAFwk::Want& want,
        const sptr<AbilityConnectCallback>& connectCallback) override;
    void DisconnectAbility(const AAFwk::Want &want, const sptr<AbilityConnectCallback> &connectCallback,
        int32_t accountId = -1) override;
    std::shared_ptr<AppExecFwk::HapModuleInfo> GetHapModuleInfo() const override;
    std::shared_ptr<AppExecFwk::AbilityInfo> GetAbilityInfo() const override;
    void MinimizeAbility(bool fromUser = false) override;

    ErrCode OnBackPressedCallBack(bool &needMoveToBackground) override;

    ErrCode MoveAbilityToBackground() override;

    ErrCode MoveUIAbilityToBackground() override;

    ErrCode TerminateSelf() override;

    ErrCode StartSelf() override;

    ErrCode CloseAbility() override;

    sptr<IRemoteObject> GetToken() override;

    ErrCode RestoreWindowStage(napi_env env, napi_value contentStorage) override;

    ErrCode RestoreWindowStage(void *contentStorage) override;

    void SetStageContext(const std::shared_ptr<AbilityRuntime::Context> &stageContext);

    /**
     * @brief Set the Ability Info object
     *
     * set ability info to ability context
     */
    void SetAbilityInfo(const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo);

    /**
     * @brief Attachs ability's token.
     *
     * @param token The token represents ability.
     */
    void SetToken(const sptr<IRemoteObject> &token) override
    {
        token_ = token;
    }

    /**
     * @brief Get ContentStorage.
     *
     * @return Returns the ContentStorage.
     */
    std::unique_ptr<NativeReference>& GetContentStorage() override
    {
        return contentStorage_;
    }

    /**
     * @brief Get ContentStorage.
     *
     * @return Returns the ContentStorage.
     */
    void *GetEtsContentStorage() override
    {
        return etsContentStorage_;
    }

    /**
     * @brief Get LocalCallContainer.
     *
     * @return Returns the LocalCallContainer.
     */
    std::shared_ptr<LocalCallContainer> GetLocalCallContainer() override;

    void SetConfiguration(const std::shared_ptr<AppExecFwk::Configuration> &config) override;

    std::shared_ptr<AppExecFwk::Configuration> GetConfiguration() const override;

    /**
     * call function by callback objectS
     *
     * @param want Request info for ability.
     * @param callback Indicates the callback object.
     * @param accountId Indicates the account to start.
     *
     * @return Returns zero on success, others on failure.
     */
    ErrCode StartAbilityByCall(const AAFwk::Want& want, const std::shared_ptr<CallerCallBack> &callback,
        int32_t accountId = DEFAULT_INVAL_VALUE) override;

    /**
     * caller release by callback object
     *
     * @param callback Indicates the callback object.
     *
     * @return Returns zero on success, others on failure.
     */
    ErrCode ReleaseCall(const std::shared_ptr<CallerCallBack> &callback) override;

    /**
     * clear failed call connection by callback object
     *
     * @param callback Indicates the callback object.
     *
     * @return void.
     */
    void ClearFailedCallConnection(const std::shared_ptr<CallerCallBack> &callback) override;

    /**
     * register ability callback
     *
     * @param abilityCallback Indicates the abilityCallback object.
     */
    void RegisterAbilityCallback(std::weak_ptr<AppExecFwk::IAbilityCallback> abilityCallback) override;

    bool IsTerminating() override
    {
        return isTerminating_.load();
    }

    void SetWeakSessionToken(const wptr<IRemoteObject>& sessionToken) override;
    void SetAbilityRecordId(int32_t abilityRecordId) override;
    int32_t GetAbilityRecordId() override;

    void SetTerminating(bool state) override
    {
        isTerminating_.store(state);
    }

    ErrCode RequestDialogService(napi_env env, AAFwk::Want &want, RequestDialogResultTask &&task) override;

    ErrCode RequestDialogService(AAFwk::Want &want, RequestDialogResultTask &&task) override;

    ErrCode ReportDrawnCompleted() override;

    ErrCode GetMissionId(int32_t &missionId) override;

    /**
     * @brief Set mission continue state of this ability.
     *
     * @param state the mission continuation state of this ability.
     * @return Returns ERR_OK if success.
     */
    ErrCode SetMissionContinueState(const AAFwk::ContinueState &state) override;
#ifdef SUPPORT_SCREEN
    ErrCode StartAbilityByType(const std::string &type, AAFwk::WantParams &wantParam,
        std::shared_ptr<UIExtensionCallback> uiExtensionCallback) override;
#endif
    ErrCode RequestModalUIExtension(const Want &want) override;

    ErrCode ChangeAbilityVisibility(bool isShow) override;

    ErrCode AddFreeInstallObserver(const sptr<AbilityRuntime::IFreeInstallObserver> &observer) override;

    ErrCode OpenLink(const AAFwk::Want &want, int requestCode, bool hideFailureTipDialog = false) override;

    ErrCode OpenAtomicService(AAFwk::Want& want, const AAFwk::StartOptions &options, int requestCode,
        RuntimeTask &&task) override;

    void RegisterAbilityLifecycleObserver(const std::shared_ptr<AppExecFwk::ILifecycleObserver> &observer) override;

    void UnregisterAbilityLifecycleObserver(const std::shared_ptr<AppExecFwk::ILifecycleObserver> &observer) override;

    void InsertResultCallbackTask(int requestCode, RuntimeTask&& task) override;

    void RemoveResultCallbackTask(int requestCode) override;

    void SetRestoreEnabled(bool enabled) override;
    bool GetRestoreEnabled() override;

    std::shared_ptr<AAFwk::Want> GetWant() override;

#ifdef SUPPORT_SCREEN
    /**
     * @brief Set mission label of this ability.
     *
     * @param label the label of this ability.
     * @return Returns ERR_OK if success.
     */
    ErrCode SetMissionLabel(const std::string &label) override;

    /**
     * @brief Set mission icon of this ability.
     *
     * @param icon the icon of this ability.
     * @return Returns ERR_OK if success.
     */
    ErrCode SetMissionIcon(const std::shared_ptr<OHOS::Media::PixelMap> &icon) override;

    /**
     * @brief Set mission window icon of this ability.
     *
     * @param windowIcon the window icon of this ability.
     * @return Returns ERR_OK if success.
     */
    ErrCode SetMissionWindowIcon(std::shared_ptr<OHOS::Media::PixelMap> windowIcon) override;

     /**
     * @brief Set ability label and icon of this ability.
     *
     * @param label the label of this ability.
     * @param icon the icon of this ability.
     * @return Returns ERR_OK if success.
     */
    ErrCode SetAbilityInstanceInfo(const std::string& label, std::shared_ptr<OHOS::Media::PixelMap> icon) override;

    /**
     * @brief get current window mode.
     *
     * @return Returns the current window mode.
     */
    int GetCurrentWindowMode() override;

    /**
     * @brief Get window rectangle of this ability.
     *
     * @param the left position of window rectangle.
     * @param the top position of window rectangle.
     * @param the width position of window rectangle.
     * @param the height position of window rectangle.
     */
    void GetWindowRect(int32_t &left, int32_t &top, int32_t &width, int32_t &height) override;

    /**
     * @brief Get ui content object.
     *
     * @return UIContent object of ACE.
     */
    Ace::UIContent* GetUIContent() override;

    /**
     * @brief create modal UIExtension.
     * @param want Create modal UIExtension with want object.
     */
    ErrCode CreateModalUIExtensionWithApp(const Want &want) override;
    void EraseUIExtension(int32_t sessionId) override;
    bool IsUIExtensionExist(const AAFwk::Want &want);
    ErrCode RevokeDelegator() override;
    inline bool GetHookOff() override
    {
        return hookOff_;
    }
    inline void SetHookOff(bool hookOff) override
    {
        hookOff_ = hookOff;
    }
    bool IsHook() override
    {
        return isHook_;
    }
    void SetHook(bool isHook) override
    {
        isHook_ = isHook;
    }

    static int32_t TransferRestartWSError(Rosen::WSError srcError);
#endif

    /**
     * @brief Add CompletioHandler.
     *
     * @param requestId, the requestId.
     * @param onRequestSucc, the callback ot be called upon request success.
     * @param onRequestFail, the callback ot be called upon request failure.
     * @return ERR_OK on success, otherwise failure.
     */
    ErrCode AddCompletionHandler(const std::string &requestId, OnRequestResult onRequestSucc,
        OnRequestResult onRequestFail) override;

    /**
     * @brief Callback on request success.
     *
     * @param requestId, the requestId.
     * @param element, the want element of startAbility.
     * @param message, the message returned to the callback.
     */
    void OnRequestSuccess(const std::string &requestId, const AppExecFwk::ElementName &element,
        const std::string &message) override;

    /**
     * @brief Callback on request failure.
     *
     * @param requestId, the requestId.
     * @param element, the want element of startAbility.
     * @param message, the message returned to the callback.
     */
    void OnRequestFailure(const std::string &requestId, const AppExecFwk::ElementName &element,
        const std::string &message, int32_t resultCode = 0) override;

    void OnOpenLinkRequestSuccess(const std::string &requestId, const AppExecFwk::ElementName &element,
        const std::string &message) override;
    void OnOpenLinkRequestFailure(const std::string &requestId, const AppExecFwk::ElementName &element,
    const std::string &message) override;

    ErrCode StartExtensionAbilityWithExtensionType(const AAFwk::Want &want,
        AppExecFwk::ExtensionAbilityType extensionType) override;
    ErrCode StopExtensionAbilityWithExtensionType(const AAFwk::Want& want,
        AppExecFwk::ExtensionAbilityType extensionType) override;
    ErrCode ConnectExtensionAbilityWithExtensionType(const AAFwk::Want& want,
        const sptr<AbilityConnectCallback>& connectCallback, AppExecFwk::ExtensionAbilityType extensionType) override;
    ErrCode SetOnNewWantSkipScenarios(int32_t scenarios) override;
    ErrCode RestartAppWithWindow(const Want &want) override;
    ErrCode NotifyCancelGamePreLaunch() override;
    ErrCode NotifyCompleteGamePreLaunch() override;

    ErrCode AddCompletionHandlerForAtomicService(const std::string &requestId, OnAtomicRequestSuccess onRequestSucc,
        OnAtomicRequestFailure onRequestFail, const std::string &appId) override;

    ErrCode AddCompletionHandlerForOpenLink(const std::string &requestId,
        OnRequestResult onRequestSucc, OnRequestResult onRequestFail) override;

    ErrCode StartSelfUIAbilityInCurrentProcess(const AAFwk::Want &want, const std::string &specifiedFlag,
        const AAFwk::StartOptions &startOptions, bool hasOptions) override;

private:
    sptr<IRemoteObject> token_ = nullptr;
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo_ = nullptr;
    std::shared_ptr<AbilityRuntime::Context> stageContext_ = nullptr;
    std::map<int, RuntimeTask> resultCallbacks_;
    std::unique_ptr<NativeReference> contentStorage_ = nullptr;
    void *etsContentStorage_ = nullptr;
    std::shared_ptr<AppExecFwk::Configuration> config_ = nullptr;
    std::mutex callContainerMutex_;
    std::shared_ptr<LocalCallContainer> localCallContainer_ = nullptr;
    std::weak_ptr<AppExecFwk::IAbilityCallback> abilityCallback_;
    std::atomic<bool> isTerminating_ = false;
    int32_t missionId_ = -1;
    int32_t abilityRecordId_ = 0;
    std::mutex sessionTokenMutex_;
    wptr<IRemoteObject> sessionToken_;
    std::mutex uiExtensionMutex_;
    std::map<int32_t, Want> uiExtensionMap_;
    std::atomic<bool> restoreEnabled_ = false;
    std::shared_ptr<Global::Resource::ResourceManager> abilityResourceMgr_ = nullptr;
    AbilityConfigUpdateCallback abilityConfigUpdateCallback_ = nullptr;
    BindingObjectConfigUpdateCallback bindingObjectConfigUpdateCallback_ = nullptr;
    std::shared_ptr<AppExecFwk::Configuration> abilityConfiguration_ = nullptr;
    bool isHook_ = false;
    bool hookOff_ = false;

    static void RequestDialogResultJSThreadWorker(uv_work_t* work, int status);
    void OnAbilityResultInner(int requestCode, int resultCode, const AAFwk::Want &resultData);
    sptr<IRemoteObject> GetSessionToken();
    void SetWindowRectangleParams(AAFwk::Want &want);
    void GetFailureInfoByMessage(const std::string &message, int32_t &failureCode,
        std::string &failureMessage, int32_t resultCode);

    std::mutex onRequestResultMutex_;
    std::mutex onAtomicRequestResultMutex_;
    std::mutex onOpenLinkRequestResultMutex_;
    std::vector<std::shared_ptr<OnRequestResultElement>> onRequestResults_;
    std::vector<std::shared_ptr<OnAtomicRequestResult>> onAtomicRequestResults_;
    std::vector<std::shared_ptr<AAFwk::OnOpenLinkRequestResult>> onOpenLinkRequestResults_;
    std::mutex contentStorageMutex_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ABILITY_CONTEXT_IMPL_H
