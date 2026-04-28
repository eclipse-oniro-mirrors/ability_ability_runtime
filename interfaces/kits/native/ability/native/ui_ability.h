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

#ifndef OHOS_ABILITY_RUNTIME_UI_ABILITY_H
#define OHOS_ABILITY_RUNTIME_UI_ABILITY_H

#include "ability_context.h"
#include "ability_continuation_interface.h"
#include "ability_lifecycle_executor.h"
#include "ability_lifecycle_interface.h"
#include "ability_local_record.h"
#include "ability_transaction_callback_info.h"
#include "configuration.h"
#include "context.h"
#include "continuation_handler_stage.h"
#include "fa_ability_context.h"
#include "iability_callback.h"
#include "recovery_param.h"
#include "resource_config_helper.h"
#include "want.h"

#ifdef SUPPORT_SCREEN
#include "display_manager.h"
#include "session_info.h"
#include "window_scene.h"
#include "window_manager.h"
#endif

namespace OHOS {
namespace AppExecFwk {
class AbilityHandler;
class AbilityRecovery;
class OHOSApplication;
class LifeCycle;
class ContinuationHandlerStage;
class ContinuationManagerStage;
class InsightIntentExecuteParam;
struct BaseDelegatorAbilityProperty;
struct InsightIntentExecuteResult;
using InsightIntentExecutorAsyncCallback = AbilityTransactionCallbackInfo<InsightIntentExecuteResult>;
struct OnSaveStateResult {
    int32_t status;
    WantParams wantParams;
    StateReason reason;
};
} // namespace AppExecFwk
namespace AbilityRuntime {
class Runtime;
using InsightIntentExecuteResult = AppExecFwk::InsightIntentExecuteResult;
using InsightIntentExecuteParam = AppExecFwk::InsightIntentExecuteParam;
using InsightIntentExecutorAsyncCallback = AppExecFwk::InsightIntentExecutorAsyncCallback;
class UIAbility : public AppExecFwk::AbilityContext,
                  public AppExecFwk::ILifeCycle,
                  public AppExecFwk::IAbilityCallback,
                  public AppExecFwk::IAbilityContinuation,
                  public std::enable_shared_from_this<UIAbility> {
public:
    UIAbility() = default;
    virtual ~UIAbility() = default;

    /**
     * @brief Create a UIAbility instance through the singleton pattern
     * @param runtime the runtime pointer
     * @return Returns the UIAbility object of the ability
     */
    static UIAbility *Create(const std::unique_ptr<Runtime> &runtime);

    /**
     * @brief Obtains the Lifecycle object of the current ability
     * @return Returns the Lifecycle object.
     */
    std::shared_ptr<AppExecFwk::LifeCycle> GetLifecycle() override final;

    /**
     * Register lifecycle observer on UIAbility.
     *
     * @param observer the lifecycle observer to be registered on UIAbility.
     */
    void RegisterAbilityLifecycleObserver(
        const std::shared_ptr<AppExecFwk::ILifecycleObserver> &observer) override final;

    /**
     * Unregister lifecycle observer on UIAbility.
     *
     * @param observer the lifecycle observer to be unregistered on UIAbility.
     */
    void UnregisterAbilityLifecycleObserver(
        const std::shared_ptr<AppExecFwk::ILifecycleObserver> &observer) override final;

    /**
     * @brief Obtains the AbilityContext object of the ability.
     * @return Returns the AbilityContext object of the ability.
     */
    std::shared_ptr<AbilityRuntime::AbilityContext> GetAbilityContext();

    /**
     * @brief Obtains the Want object that starts this ability.
     * @return Returns the Want object that starts this ability.
     */
    std::shared_ptr<AAFwk::Want> GetWant() override;

    /**
     * @brief Init the UIability
     * @param abilityInfo Indicate the Ability information
     * @param application Indicates the main process
     * @param handler the UIability EventHandler object
     * @param token the remote token
     * @param createJsObjSuc the flag indicating whether object creation succeeded
     */
    virtual void Init(std::shared_ptr<AppExecFwk::AbilityLocalRecord> record,
        const std::shared_ptr<AppExecFwk::OHOSApplication> application,
        std::shared_ptr<AppExecFwk::AbilityHandler> &handler, const sptr<IRemoteObject> &token, bool &createObjSuc);

    /**
     * @brief Attach Ability Context
     * @param abilityContext Indicate the AbilityContext
     */
    void AttachAbilityContext(const std::shared_ptr<AbilityRuntime::AbilityContext> &abilityContext);

    /**
     * @brief Called when this ability is started. You must override this function if you want to perform some
     * initialization operations during ability startup.
     * This function can be called only once in the entire lifecycle of an ability.
     * @param Want Indicates the {@link Want} structure containing startup information about the ability.
     * @param sessionInfo Indicates the sessionInfo.
     */
    virtual void OnStart(const AAFwk::Want &want, sptr<AAFwk::SessionInfo> sessionInfo = nullptr);

    /**
     * @brief Called when this ability enters the <b>STATE_STOP</b> state.
     * The ability in the <b>STATE_STOP</b> is being destroyed.
     * You can override this function to implement your own processing logic.
     */
    virtual void OnStop();

    /**
     * @brief Called when this ability enters the <b>STATE_STOP</b> state.
     * The ability in the <b>STATE_STOP</b> is being destroyed.
     * You can override this function to implement your own processing logic.
     * @param callbackInfo Indicates the lifecycle transaction callback information
     * @param isAsyncCallback Indicates whether it is an asynchronous lifecycle callback
     */
    virtual void OnStop(AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback);

    /**
     * @brief The callback of OnStop.
     */
    virtual void OnStopCallback();

    /**
     * @brief request a remote object of callee from this ability.
     * @return Returns the remote object of callee.
     */
    virtual sptr<IRemoteObject> CallRequest();

    /**
     * @brief Called when the system configuration is updated.
     * @param configuration Indicates the updated configuration information.
     */
    void OnConfigurationUpdatedNotify(const AppExecFwk::Configuration &configuration);

    /**
     * @brief Update context.config when configuration is updated.
     */
    virtual void UpdateContextConfiguration() {}

    /**
     * @brief Called when the system configuration is updated.
     * @param level Indicates the memory trim level, which shows the current memory usage status.
     */
    virtual void OnMemoryLevel(int level);

    /**
     * @brief Obtains the class name in this ability name, without the prefixed bundle name.
     * @return Returns the class name of this ability.
     */
    std::string GetAbilityName();

    /**
     * @brief Obtains the module name in this ability name, without the prefixed bundle name.
     * @return Returns the module name of this ability.
     */
    std::string GetModuleName();

    /**
     * @brief Obtains the instance id of this ability.
     * @return Returns the instance id string of this ability.
     */
    std::string GetInstanceId() const;

    /**
     * @brief Called when startAbilityForResult(ohos.aafwk.content.Want,int) is called to start an ability and the
     * result is returned. This method is called only on Page abilities. You can start a new ability to perform some
     * calculations and use setResult (int,ohos.aafwk.content.Want) to return the calculation result. Then the system
     * calls back the current method to use the returned data to execute its own logic.
     * @param requestCode Indicates the request code returned after the ability is started. You can define the request
     * code to identify the results returned by abilities. The value ranges from 0 to 65535.
     * @param resultCode Indicates the result code returned after the ability is started. You can define the result code
     * to identify an error.
     * @param want Indicates the data returned after the ability is started. You can define the data returned. The
     * value can be null.
     */
    virtual void OnAbilityResult(int requestCode, int resultCode, const AAFwk::Want &want);

    /**
     * @brief Called when the launch mode of an ability is set to singleInstance. This happens when you re-launch an
     * ability that has been at the top of the ability stack.
     * @param want Indicates the new Want containing information about the ability.
     */
    virtual void OnNewWant(const AAFwk::Want &want);

    /**
     * @brief Restores data and states of an ability when it is restored by the system. This method should be
     * implemented by a Page ability. This method is called if an ability was destroyed at a certain time due to
     * resource reclaim or was unexpectedly destroyed and the onSaveAbilityState(ohos.utils.PacMap) method was called to
     * save its user data and states. Generally, this method is called after the onStart(ohos.aafwk.content.Want)
     * method.
     * @param inState Indicates the PacMap object used for storing data and states. This parameter can not be null.
     */
    virtual void OnRestoreAbilityState(const AppExecFwk::PacMap &inState);

    /**
     * @brief Sets the want object that can be obtained by calling getWant().
     * @param Want information of other ability
     */
    void SetWant(const AAFwk::Want &want);

    /**
     * @brief dump ability info
     * @param params dump params that indicate different dump targets
     * @param info dump ability info
     */
    virtual void Dump(const std::vector<std::string> &params, std::vector<std::string> &info);

    /**
     * @brief Save user data of local Ability generated at runtime.
     * @param saveData Indicates the user data to be saved.
     * @return If the data is saved successfully, it returns true; otherwise, it returns false.
     */
    bool OnSaveData(AAFwk::WantParams &saveData) override;

    /**
     * @brief After creating the Ability on the remote device,
     * immediately restore the user data saved during the migration of the Ability on the remote device.
     * @param restoreData Indicates the user data to be restored.
     * @return If the data is restored successfully, it returns true; otherwise, it returns false .
     */
    bool OnRestoreData(AAFwk::WantParams &restoreData) override;

#ifdef SUPPORT_SCREEN
    /**
     * @brief Get WindowStage
     * @return Returns the napi_value of WindowStage.
     */
    virtual napi_value GetWindowStage();

    /**
     * @brief Get WindowStage.
     * @return Returns the ani_object of WindowStage.
     */
    virtual ani_object GetEtsWindowStage();
#endif

    /**
     * @brief Obtains the lifecycle state of this ability.
     * @return Returns the lifecycle state of this ability.
     */
    virtual AppExecFwk::AbilityLifecycleExecutor::LifecycleState GetState() final;

    /**
     * @brief Release the ability instance.
     */
    void DestroyInstance();

    /**
     * @brief Update configuration
     * @param configuration Indicates the updated configuration information.
     */
    virtual void OnConfigurationUpdated(const AppExecFwk::Configuration &configuration);

     /**
     * @brief The async callback of OnContinue.
     */
    virtual int32_t OnContinueAsyncCB(napi_ref jsWantParams, int32_t status,
        const AppExecFwk::AbilityInfo &abilityInfo);

    virtual int32_t OnContinueAsyncCB(ani_ref aniWantParamsRef, int32_t status,
        const AppExecFwk::AbilityInfo &abilityInfo);

    /**
     * @brief Prepare user data of local Ability.
     * @param wantParams Indicates the user data to be saved.
     * @return If the ability is willing to continue and data saved successfully, it returns 0;
     * otherwise, it returns errcode.
     */
    virtual int32_t OnContinue(AAFwk::WantParams &wantParams, bool &isAsyncOnContinue,
        const AppExecFwk::AbilityInfo &abilityInfo);

    /**
     * @brief Migrates this ability to the given device on the same distributed network. The ability to migrate and its
     * ability slices must implement the IAbilityContinuation interface.
     * @param deviceId Indicates the ID of the target device where this ability will be migrated to.
     * @param versionCode Target bundle version.
     */
    virtual void ContinueAbilityWithStack(const std::string &deviceId, uint32_t versionCode) final;

    /**
     * @brief Callback function to ask the user whether to start the migration .
     * @return If the user allows migration, it returns true; otherwise, it returns false.
     */
    bool OnStartContinuation() override;

    /**
     * @brief This function can be used to implement the processing logic after the migration is completed.
     * @param result Migration result code. 0 means the migration was successful, -1 means the migration failed.
     * @return None.
     */
    void OnCompleteContinuation(int result) override;

    /**
     * @brief Used to notify the local Ability that the remote Ability has been destroyed.
     * @return None.
     */
    void OnRemoteTerminated() override;

    /**
     * @brief Prepare user data of local Ability.
     * @param reason the reason why framework invoke this function
     * @param wantParams Indicates the user data to be saved.
     * @return result code defined in abilityConstants
     */
    virtual int32_t OnSaveState(int32_t reason, AAFwk::WantParams &wantParams);
    
    virtual int32_t OnSaveState(int32_t reason, AAFwk::WantParams &wantParams,
        AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnSaveStateResult> *callbackInfo,
        bool &isAsync, AppExecFwk::StateReason stateReason);

    /**
     * @brief enable ability recovery.
     * @param abilityRecovery shared_ptr of abilityRecovery
     * @param useAppSettedRecoveryValue Indicates use default recovery or not.
     */
    void EnableAbilityRecovery(const std::shared_ptr<AppExecFwk::AbilityRecovery> &abilityRecovery,
        bool useAppSettedRecoveryValue);

    /**
     * @brief Callback when the ability is shared.You can override this function to implement your own sharing logic.
     * @param wantParams Indicates the user data to be saved.
     * @return the result of OnShare
     */
    virtual int32_t OnShare(AAFwk::WantParams &wantParams);

    void BindHybridContext(const std::shared_ptr<AppExecFwk::OHOSApplication> application,
        const std::shared_ptr<AppExecFwk::AbilityLocalRecord> record);

    bool CheckIsSilentForeground() const;

    void SetIsSilentForeground(bool isSilentForeground);

    bool CreateProperty(std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext,
        std::shared_ptr<AppExecFwk::BaseDelegatorAbilityProperty> delegatorAbilityProperty);

    // Native Module related methods
    inline void SetWithNative(bool withNative)
    {
        withNative_ = withNative;
    }

protected:
    const AAFwk::LaunchParam &GetLaunchParam() const;
    bool IsRestoredInContinuation() const;
    void NotifyContinuationResult(const AAFwk::Want &want, bool success);
    bool ShouldRecoverState(const AAFwk::Want &want);
    bool ShouldDefaultRecoverState(const AAFwk::Want &want);
    bool IsUseNewStartUpRule();

    inline bool IsWithNative() const
    {
        return withNative_;
    }

    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext_ = nullptr;
    std::shared_ptr<AppExecFwk::AbilityStartSetting> setting_ = nullptr;
    std::shared_ptr<AppExecFwk::AbilityRecovery> abilityRecovery_ = nullptr;
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo_ = nullptr;
    AAFwk::LaunchParam launchParam_;
    bool securityFlag_ = false;
    bool withNative_ = false;
    uint64_t intentId_;

private:
    friend class UIAbilityImpl;
    void DispatchLifecycleOnForeground(const AAFwk::Want &want);
    void HandleCreateAsRecovery(const AAFwk::Want &want);
    void SetStartAbilitySetting(std::shared_ptr<AppExecFwk::AbilityStartSetting> setting);
    void SetLaunchParam(const AAFwk::LaunchParam &launchParam);
    void InitConfigurationProperties(const AppExecFwk::Configuration &changeConfiguration,
        ResourceConfigHelper &resourceConfig);
    void BindEtsContext(const std::shared_ptr<AppExecFwk::OHOSApplication> application,
        const std::shared_ptr<AppExecFwk::AbilityLocalRecord> record);
    void BindJsContext(const std::shared_ptr<AppExecFwk::OHOSApplication> application,
        const std::shared_ptr<AppExecFwk::AbilityLocalRecord> record);

    std::shared_ptr<AppExecFwk::ContinuationHandlerStage> continuationHandler_ = nullptr;
    std::shared_ptr<AppExecFwk::ContinuationManagerStage> continuationManager_ = nullptr;
    std::shared_ptr<AppExecFwk::AbilityHandler> handler_ = nullptr;
    std::shared_ptr<AppExecFwk::LifeCycle> lifecycle_ = nullptr;
    std::shared_ptr<AppExecFwk::AbilityLifecycleExecutor> abilityLifecycleExecutor_ = nullptr;
    std::shared_ptr<AppExecFwk::OHOSApplication> application_ = nullptr;
    std::shared_ptr<AAFwk::Want> setWant_ = nullptr;
    sptr<IRemoteObject> reverseContinuationSchedulerReplica_ = nullptr;
    bool isNewRuleFlagSetted_ = false;
    bool startUpNewRule_ = false;
    bool isSilentForeground_ = false;
    std::atomic<bool> useAppSettedRecoveryValue_ = false;
    std::string instanceId_;

#ifdef SUPPORT_SCREEN
public:
    uint32_t sceneFlag_ = 0;

    enum CollaborateResult {
        ACCEPT = 0,
        REJECT = 1,
        ON_COLLABORATE_NOT_IMPLEMENTED = 10,
        ON_COLLABORATE_ERR = 11,
    };

    /**
     * @brief Called after instantiating WindowScene.
     * You can override this function to implement your own processing logic.
     */
    virtual void OnSceneCreated();

    /**
     * @brief Called after ability stoped.
     * You can override this function to implement your own processing logic.
     */
    virtual void OnSceneWillDestroy();

    /**
     * @brief Called after ability stoped.
     * You can override this function to implement your own processing logic.
     */
    virtual void onSceneDestroyed();

    /**
     * @brief Called after ability restored.
     * You can override this function to implement your own processing logic.
     */
    virtual void OnSceneRestored();

    /**
     * @brief Called when this ability enters the <b>STATE_FOREGROUND</b> state.
     * The ability in the <b>STATE_FOREGROUND</b> state is visible.
     * You can override this function to implement your own processing logic.
     */
    virtual void OnForeground(const AAFwk::Want &want);

    /**
     * @brief Called when this ability enters the <b>STATE_BACKGROUND</b> state.
     * The ability in the <b>STATE_BACKGROUND</b> state is invisible.
     * You can override this function to implement your own processing logic.
     */
    virtual void OnBackground();

    /**
     * @brief Called before this ability enters the <b>STATE_FOREGROUND</b> state.
     * The ability in the <b>STATE_FOREGROUND</b> state is invisible.
     * You can override this function to implement your own processing logic.
     */
    virtual void OnWillForeground();

    /**
     * @brief Called after wms show event.
     * The ability in the <b>STATE_FOREGROUND</b> state is invisible.
     * You can override this function to implement your own processing logic.
     */
    virtual void OnDidForeground();

    /**
     * @brief Called before OnBackground.
     * The ability in the <b>STATE_BACKGROUND</b> state is invisible.
     * You can override this function to implement your own processing logic.
     */
    virtual void OnWillBackground();

    /**
     * @brief Called after wms hiden event.
     * The ability in the <b>STATE_BACKGROUND</b> state is invisible.
     * You can override this function to implement your own processing logic.
     */
    virtual void OnDidBackground();

    /**
     * @brief Called after window stage focused or unfocused
     * You can override this function to implement your own processing logic.
     */
    virtual void OnAfterFocusedCommon(bool isFocused);

    /**
     * @brief Called when ability prepare terminate.
     * @return Return true if ability need to stop terminating; return false if ability need to terminate.
     */
    virtual bool OnPrepareTerminate();

    /**
     * @brief Called when ability prepare terminate.
     * @param callbackInfo The callbackInfo is used when onPrepareToTerminateAsync is implemented.
     * @param isAsync The returned flag indicates if onPrepareToTerminateAsync is implemented.
     */
    virtual void OnPrepareTerminate(AppExecFwk::AbilityTransactionCallbackInfo<bool> *callbackInfo, bool &isAsync);

    /**
     * @brief Inflates UI controls by using windowOption.
     * @param windowOption Indicates the window option defined by the user.
     */
    virtual void InitWindow(int32_t displayId, sptr<Rosen::WindowOption> option);

    /**
     * @brief Get the window belong to the ability.
     * @return Returns a Window object pointer.
     */
    virtual const sptr<Rosen::Window> GetWindow();

    /**
     * @brief get the scene belong to the ability.
     * @return Returns a WindowScene object pointer.
     */
    std::shared_ptr<Rosen::WindowScene> GetScene();

    /**
     * @brief Called when this ability is about to leave the foreground and enter the background due to a user
     * operation, for example, when the user touches the Home key.
     */
    virtual void OnLeaveForeground();

    /**
     * @brief Get page ability stack info.
     * @return A string represents page ability stack info, empty if failed;
     */
    virtual std::string GetContentInfo();
    virtual std::string GetContentInfoForRecovery();
    virtual std::string GetContentInfoForDefaultRecovery();

    /**
     * @brief Set WindowScene listener
     * @param listener WindowScene listener
     * @return None.
     */
    void SetSceneListener(const sptr<Rosen::IWindowLifeCycle> &listener);

    /**
     * @brief Called back at ability context.
     * @return current window mode of the ability.
     */
    int GetCurrentWindowMode() override;

    /**
     * @brief Set mission label of this ability.
     * @param label the label of this ability.
     * @return Returns ERR_OK if success.
     */
    ErrCode SetMissionLabel(const std::string &label) override;

    /**
     * @brief Set mission icon of this ability.
     * @param icon the icon of this ability.
     * @return Returns ERR_OK if success.
     */
    ErrCode SetMissionIcon(const std::shared_ptr<OHOS::Media::PixelMap> &icon) override;

    /**
     * @brief Get window rectangle of this ability.
     * @param left the left position of window rectangle.
     * @param top the top position of window rectangle.
     * @param width the width position of window rectangle.
     * @param height the height position of window rectangle.
     */
    void GetWindowRect(int32_t &left, int32_t &top, int32_t &width, int32_t &height) override;

    /**
     * @brief Get ui content object.
     * @return UIContent object of ACE.
     */
    Ace::UIContent *GetUIContent() override;

    /**
     * @brief Call "onForeground" js function barely.
     *
     * @param want Want
     */
    virtual void CallOnForegroundFunc(const AAFwk::Want &want);

    /**
     * @brief Request focus for current window, can be override.
     *
     * @param want Want
     */
    virtual void RequestFocus(const AAFwk::Want &want);

    /**
     * @brief Execute insight intent when an ability is in foreground, schedule it to foreground repeatly.
     *
     * @param want Want.
     * @param executeParam insight intent execute param.
     * @param callback insight intent async callback.
     */
    virtual void ExecuteInsightIntentRepeateForeground(const AAFwk::Want &want,
        const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
        std::unique_ptr<InsightIntentExecutorAsyncCallback> callback);

    /**
     * @brief Execute insight intent when an ability didn't started or in background, schedule it to foreground.
     *
     * @param want Want.
     * @param executeParam insight intent execute param.
     * @param callback insight intent async callback.
     */
    virtual void ExecuteInsightIntentMoveToForeground(const AAFwk::Want &want,
        const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
        std::unique_ptr<InsightIntentExecutorAsyncCallback> callback);

    /**
     * @brief Execute insight intent when an ability start with page insight intent.
     *
     * @param want Want.
     * @param executeParam insight intent execute param.
     * @param callback insight intent async callback.
     */
    virtual void ExecuteInsightIntentPage(const AAFwk::Want &want,
        const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
        std::unique_ptr<InsightIntentExecutorAsyncCallback> callback);

    /**
     * @brief Execute insight intent when an ability didn't started, schedule it to background.
     *
     * @param want Want.
     * @param executeParam insight intent execute param.
     * @param callback insight intent async callback.
     */
    virtual void ExecuteInsightIntentBackground(const AAFwk::Want &want,
        const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
        std::unique_ptr<InsightIntentExecutorAsyncCallback> callback);

    /**
     * @brief create modal UIExtension.
     * @param want Create modal UIExtension with want object.
     */
    int CreateModalUIExtension(const AAFwk::Want &want);

    /**
     * @brief Update sessionToken.
     * @param sessionToken The token of session.
     */
    void UpdateSessionToken(sptr<IRemoteObject> sessionToken);

    void EraseUIExtension(int32_t sessionId) override;

    void SetIdentityToken(const std::string &identityToken);
    std::string GetIdentityToken() const;

    /**
     * @brief Called when distributed system trying to collaborate remote ability.
     * @param want want with collaborative info.
     */
    void HandleCollaboration(const AAFwk::Want &want);

    /**
     * @brief Callback for collaboration event.
     *
     * @param wantParams Parameters for the collaboration event.
     * @return int32_t Returns the result code of the collaboration handling.
     */
    virtual int32_t OnCollaborate(WantParams &wantParams);

    /**
     * @brief Called when startAbility request failed.
     * @param requestId, the requestId.
     * @param element, the element to start ability.
     * @param message, the message to be returned to the calling app.
     */
    virtual void OnAbilityRequestFailure(const std::string &requestId, const AppExecFwk::ElementName &element,
        const std::string &message, int32_t resultCode = 0);

    /**
     * @brief Called when startAbility request succeeded.
     * @param requestId, the requestId.
     * @param element, the element to start ability.
     * @param message, the message to be returned to the calling app.
     */
    virtual void OnAbilityRequestSuccess(const std::string &requestId, const AppExecFwk::ElementName &element,
        const std::string &message);

protected:
    class UIAbilityDisplayListener : public OHOS::Rosen::IDisplayInfoChangedListener {
    public:
        explicit UIAbilityDisplayListener(const std::weak_ptr<UIAbility> &ability)
        {
            ability_ = ability;
        }

        void OnDisplayInfoChange(const sptr<IRemoteObject>& token, Rosen::DisplayId displayId, float density,
            Rosen::DisplayOrientation orientation) override
            {
                auto sptr = ability_.lock();
                if (sptr != nullptr) {
                    sptr->OnDisplayInfoChange(token, displayId, density, orientation);
                }
            }

    private:
        std::weak_ptr<UIAbility> ability_;
    };

    void OnCreate(Rosen::DisplayId displayId);
    void OnDestroy(Rosen::DisplayId displayId);
    void OnChange(Rosen::DisplayId displayId);
    void OnDisplayInfoChange(const sptr<IRemoteObject>& token, Rosen::DisplayId displayId, float density,
        Rosen::DisplayOrientation orientation);

    class AbilityDisplayMoveListener : public OHOS::Rosen::IDisplayMoveListener {
    public:
        explicit AbilityDisplayMoveListener(std::weak_ptr<UIAbility> &&ability) : ability_(ability) {}

        void OnDisplayMove(Rosen::DisplayId from, Rosen::DisplayId to) override
        {
            auto sptr = ability_.lock();
            if (sptr != nullptr) {
                sptr->OnDisplayMove(from, to);
            }
        }

    private:
        std::weak_ptr<UIAbility> ability_;
    };

    void OnDisplayMove(Rosen::DisplayId from, Rosen::DisplayId to);
    void UpdateConfiguration(Rosen::DisplayId to, float density, int32_t width, int32_t height);
    virtual void DoOnForeground(const AAFwk::Want &want);
    sptr<Rosen::WindowOption> GetWindowOption(const AAFwk::Want &want);
    virtual void ContinuationRestore(const AAFwk::Want &want);
    bool CheckRecoveryEnabled();
    bool CheckDefaultRecoveryEnabled();
    bool IsStartByScb();

    std::shared_ptr<Rosen::WindowScene> scene_ = nullptr;
    sptr<Rosen::IWindowLifeCycle> sceneListener_ = nullptr;
    sptr<UIAbilityDisplayListener> abilityDisplayListener_ = nullptr;
    sptr<Rosen::IDisplayMoveListener> abilityDisplayMoveListener_ = nullptr;
private:
    void OnStartForSupportGraphics(const AAFwk::Want &want);
    void OnChangeForUpdateConfiguration(const AppExecFwk::Configuration &newConfig);
    void SetSessionToken(sptr<IRemoteObject> sessionToken);

    std::string identityToken_;
    bool showOnLockScreen_ = false;
    std::mutex wantMutexlock_;
#endif
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_UI_ABILITY_H
