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

#ifndef OHOS_ABILITY_RUNTIME_UI_ABILITY_IMPL_H
#define OHOS_ABILITY_RUNTIME_UI_ABILITY_IMPL_H

#include "insight_intent_execute_result.h"
#include "native_ability_util.h"
#include "skill/skill_execute_param.h"
#include "ui_ability.h"

namespace OHOS {
namespace AppExecFwk {
class AbilityHandler;
class AbilityLocalRecord;
class AbilityLifecycleCallbacks;
class OHOSApplication;
} // namespace AppExecFwk
namespace AbilityRuntime {
class UIAbility;

enum class LocalNativeState : uint8_t {
    NONE,
    INIT_PRE_WINDOW,
    INIT_PRE_FOREGROUND,
    HALF_FOREGROUND
};

class UIAbilityImpl : public std::enable_shared_from_this<UIAbilityImpl> {
public:
    UIAbilityImpl() = default;
    ~UIAbilityImpl() = default;

    /**
     * @brief Init the UIAbilityImpl object.
     * @param application Indicates the main process.
     * @param record current running ability record
     * @param ability current uiability object.
     * @param handler the UIability EventHandler object
     * @param token the remote token
     * @param createJsObjSuc the flag indicating whether object creation succeeded
     */
    void Init(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
        const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &record, std::shared_ptr<UIAbility> &ability,
        std::shared_ptr<AppExecFwk::AbilityHandler> &handler, const sptr<IRemoteObject> &token, bool &createObjSuc);

    /**
     * @brief Prepare terminate the ability.
     *
     * @param callback The callback to be run if onPrepareToTerminateAsync is implemented.
     * @param isAsync The returned flag indicates if onPrepareToTerminateAsync is implemented.
     * @return whether the function is successful
     */
    bool PrepareTerminateAbility(std::function<void(bool)> callback, bool &isAsync);

    /**
     * @brief Save data and states of an ability when it is restored by the system. and Calling information back to
     * Ability. This method should be implemented by a Page ability.
     */
    void DispatchSaveAbilityState();

    /**
     * @brief Restores data and states of an ability when it is restored by the system. and Calling information back
     * to Ability. This method should be implemented by a Page ability.
     * @param instate The Want object to connect to.
     */
    void DispatchRestoreAbilityState(const AppExecFwk::PacMap &inState);

    /**
     * @brief Page Service Ability has different AbilityTransaction
     * @param want Indicates the Want containing information about the ability.
     * @param targetState The life cycle state to switch to.
     * @param sessionInfo Indicates the sessionInfo
     */
    void HandleAbilityTransaction(const AAFwk::Want &want, const AAFwk::LifeCycleStateInfo &targetState,
        sptr<AAFwk::SessionInfo> sessionInfo = nullptr);

    /**
     * @brief The life cycle callback.
     * @param state The life cycle state to switch to.
     */
    void AbilityTransactionCallback(const AAFwk::AbilityLifeCycleState &state);

    /**
     * @brief Execute insight intent done, similar to AbilityTransactionCallback.
     *
     * @param intentId insight intent id.
     * @param result insight intent execute result.
     */
    void ExecuteInsightIntentDone(uint64_t intentId, const AppExecFwk::InsightIntentExecuteResult &result);

    /**
     * @brief Handling the life cycle switching of NewAbility in switch.
     * @param want Indicates the structure containing information about the ability.
     * @param targetState The life cycle state to switch to.
     * @return return true if the lifecycle transaction successfully, otherwise return false.
     */
    bool AbilityTransaction(const AAFwk::Want &want, const AAFwk::LifeCycleStateInfo &targetState);

    void HandleInitialState(bool &ret);

    /**
     * @brief Send the result code and data to be returned by this Page ability to the caller.
     * When a Page ability is destroyed, the caller overrides the AbilitySlice#onAbilityResult(int, int, Want)
     * method to receive the result set in the current method. This method can be called only after the ability has
     * been initialized.
     * @param requestCode Indicates the request code.
     * @param resultCode Indicates the result code returned after the ability is destroyed. You can define the
     * result code to identify an error.
     * @param resultData Indicates the data returned after the ability is destroyed. You can define the data
     * returned. This parameter can be null.
     */
    void SendResult(int requestCode, int resultCode, const AAFwk::Want &resultData);

    /**
     * @brief Called when the launch mode of an ability is set to singleInstance. This happens when you re-launch
     * an ability that has been at the top of the ability stack.
     * @param want Indicates the new Want containing information about the ability.
     */
    void NewWant(const AAFwk::Want &want);

    /**
     * @brief Set deviceId/bundleName/abilityName of the calling ability
     * @param deviceId deviceId of the calling ability
     * @param bundleName bundleName of the calling ability
     * @param abilityName abilityName of the calling ability
     * @param moduleName moduleName of the calling ability
     */
    void SetCallingContext(const std::string &deviceId, const std::string &bundleName, const std::string &abilityName,
        const std::string &moduleName);

    /**
     * @brief ScheduleUpdateConfiguration, scheduling update configuration.
     */
    void ScheduleUpdateConfiguration(const AppExecFwk::Configuration &config);

    /**
     * @brief continue ability to target device.
     * @param deviceId target deviceId
     * @param versionCode Target bundle version.
     */
    void ContinueAbility(const std::string &deviceId, uint32_t versionCode);

    /**
     * @brief Notify continuation result to ability.
     * @param result Continuaton result.
     */
    void NotifyContinuationResult(int32_t result);

    /**
     * @brief Notify current memory level to ability.
     * @param level Current memory level.
     */
    void NotifyMemoryLevel(int32_t level);

    /**
     * @brief Provide operating system ShareData information to the observer
     * @param uniqueId Indicates the Id of request
     */
    void HandleShareData(int32_t uniqueId);

    /**
     * @param want Indicates the structure containing information about the ability.
     * @param onlyExecuteIntent Indicates whether only the intent needs to be executed.
     * @return return true if the lifecycle background successfully, otherwise return false.
     */
    bool HandleExecuteInsightIntentBackground(const AAFwk::Want &want, bool onlyExecuteIntent = false);

    bool HandleExecuteSkill(const AAFwk::Want &want, bool onlyExecuteSkill = false);

    void SetAbilityRecordId(int32_t abilityRecordId)
    {
        if (ability_ != nullptr) {
            ability_->SetAbilityRecordId(abilityRecordId);
        }
    }

    // Native Module related methods
    void SetNativeModuleMetaData(const AAFwk::NativeAbilityMetaData& metaData);

    void ScheduleCollaborate(const Want &want);

    void ScheduleAbilityRequestFailure(const std::string &requestId, const AppExecFwk::ElementName &element,
        const std::string &message, int32_t resultCode = 0);

    void ScheduleAbilityRequestSuccess(const std::string &requestId, const AppExecFwk::ElementName &element);

#ifdef SUPPORT_SCREEN
    /**
     * @brief Get the UIability of the current thread.
     * @return Returns a null pointer if the UIAbility has not been created or has been destroyed.
     */
    std::shared_ptr<UIAbility> GetUIAbility();
#endif

#ifdef SUPPORT_GRAPHICS
public:

    /**
     * @brief Notify caller that window is UnFocused.
     */
    void AfterUnFocused();

    /**
     * @brief Notify caller that window is Focused.
     */
    void AfterFocused();

    /**
     * @brief Post action when insight intent has executed.
     *
     */
    void PostForegroundInsightIntent();

protected:
    /**
     * @brief Toggles the lifecycle status of Ability to AAFwk::ABILITY_STATE_INACTIVE. And notifies the application
     * that it belongs to of the lifecycle status.
     * @param want The Want object to switch the life cycle.
     */
    void Foreground(const AAFwk::Want &want);

    /**
     * @brief Toggles the lifecycle status of Ability to AAFwk::ABILITY_STATE_BACKGROUND. And notifies the
     * application that it belongs to of the lifecycle status.
     */
    void Background();
#endif

protected:
    void Start(const AAFwk::Want &want, sptr<AAFwk::SessionInfo> sessionInfo = nullptr);
    void Stop();
    void Stop(bool &isAsyncCallback);
    void StopCallback();
    void SetLifeCycleStateInfo(const AAFwk::LifeCycleStateInfo &info);
    bool CheckAndRestore();
    int32_t Share(AAFwk::WantParams &wantParam);
    AppExecFwk::PacMap &GetRestoreData();

    bool isStageBasedModel_ = false;
    int32_t lifecycleState_ = AAFwk::ABILITY_STATE_INITIAL;
    sptr<IRemoteObject> token_;
    std::shared_ptr<UIAbility> ability_;
    std::shared_ptr<AppExecFwk::AbilityHandler> handler_;
    bool notifyForegroundByWindow_ = false;
    bool notifyForegroundByAbility_ = false;
    std::mutex notifyForegroundLock_;

private:
    void AfterFocusedCommon(bool isFocused);
    void UpdateSilentForeground(const AAFwk::LifeCycleStateInfo &targetState, sptr<AAFwk::SessionInfo> sessionInfo);
    void OnWillBackground();

    bool hasSaveData_ = false;
    bool needSaveDate_ = false;
    AppExecFwk::PacMap restoreData_;
    AAFwk::NativeAbilityMetaData nativeModuleMetaData_;
    LocalNativeState localNativeState_ = LocalNativeState::NONE;

#ifdef SUPPORT_SCREEN
private:
    void HandleForegroundNewState(const AAFwk::Want &want, bool &bFlag);
    void HandleExecuteInsightIntentForeground(const AAFwk::Want &want, bool &bflag);
    inline void ExecuteInsightIntentRepeateForeground(const Want &want,
        const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
        std::unique_ptr<InsightIntentExecutorAsyncCallback> callback);
    void ExecuteInsightIntentMoveToForeground(const Want &want,
        const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
        std::unique_ptr<InsightIntentExecutorAsyncCallback> callback);
    inline void ExecuteInsightIntentPage(const Want &want,
        const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
        std::unique_ptr<InsightIntentExecutorAsyncCallback> callback);
    inline void ExecuteInsightIntentBackgroundByColdBoot(const Want &want,
        const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
        std::unique_ptr<InsightIntentExecutorAsyncCallback> callback);
    inline void ExecuteInsightIntentBackgroundAlreadyStart(const Want &want,
        const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
        std::unique_ptr<InsightIntentExecutorAsyncCallback> callback);

    class WindowLifeCycleImpl : public Rosen::IWindowLifeCycle {
    public:
        WindowLifeCycleImpl(const sptr<IRemoteObject> &token, const std::shared_ptr<UIAbilityImpl> &owner)
            : token_(token), owner_(owner)
        {}
        virtual ~WindowLifeCycleImpl() {}
        void AfterForeground() override;
        void AfterBackground() override;
        void AfterFocused() override;
        void AfterUnfocused() override;
        void ForegroundFailed(int32_t type) override;
        void BackgroundFailed(int32_t type) override;
        void AfterDidForeground() override;
        void AfterDidBackground() override;

    private:
        sptr<IRemoteObject> token_ = nullptr;
        std::weak_ptr<UIAbilityImpl> owner_;
    };
#endif
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_UI_ABILITY_IMPL_H
