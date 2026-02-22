/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_ETS_UI_ABILITY_H
#define OHOS_ABILITY_RUNTIME_ETS_UI_ABILITY_H

#include "ui_ability.h"

#include "ability_delegator_infos.h"
#include "ets_native_reference.h"
#include "ets_runtime.h"

namespace OHOS {
namespace AbilityRuntime {
struct InsightIntentExecutorInfo;
using AbilityHandler = AppExecFwk::AbilityHandler;
using AbilityInfo = AppExecFwk::AbilityInfo;
using OHOSApplication = AppExecFwk::OHOSApplication;
using Want = AppExecFwk::Want;
using InsightIntentExecuteResult = AppExecFwk::InsightIntentExecuteResult;
using InsightIntentExecuteParam = AppExecFwk::InsightIntentExecuteParam;
using InsightIntentExecutorAsyncCallback = AppExecFwk::InsightIntentExecutorAsyncCallback;
using Configuration = AppExecFwk::Configuration;

class EtsUIAbility : public UIAbility {
public:
    /**
     * @brief Create a EtsUIAbility instance through the singleton pattern
     * @param runtime The runtime of the ability
     * @return Returns the EtsUIability Instance point
     */
    static UIAbility *Create(const std::unique_ptr<Runtime> &runtime);

    explicit EtsUIAbility(ETSRuntime &etsRuntime);
    ~EtsUIAbility() override;

    /**
     * @brief Init the UIability
     * @param abilityInfo Indicate the Ability information
     * @param application Indicates the main process
     * @param handler the UIability EventHandler object
     * @param token the remote token
     * @param createJsObjSuc the flag indicating whether object creation succeeded
     */
    void Init(std::shared_ptr<AppExecFwk::AbilityLocalRecord> record,
        const std::shared_ptr<OHOSApplication> application,
        std::shared_ptr<AbilityHandler> &handler, const sptr<IRemoteObject> &token, bool &createObjSuc) override;

    /**
     * @brief OnStart,Start EtssUIability
     * @param want Indicates the {@link Want} structure containing startup information about the ability
     * @param sessionInfo Indicates the sessionInfo
     */
    void OnStart(const Want &want, sptr<AAFwk::SessionInfo> sessionInfo = nullptr) override;

    /**
     * @brief Called when this ability enters the <b>STATE_STOP</b> state.
     * The ability in the <b>STATE_STOP</b> is being destroyed.
     * You can override this function to implement your own processing logic.
     */
    void OnStop() override;

    /**
     * @brief Called when this ability enters the <b>STATE_STOP</b> state.
     * The ability in the <b>STATE_STOP</b> is being destroyed.
     * You can override this function to implement your own processing logic.
     * @param callbackInfo Indicates the lifecycle transaction callback information
     * @param isAsyncCallback Indicates whether it is an asynchronous lifecycle callback
     */
    void OnStop(AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback) override;

    /**
     * @brief The callback of OnStop.
     */
    void OnStopCallback() override;

    /**
     * @brief Update configuration
     * @param configuration Indicates the updated configuration information.
     */
    void OnConfigurationUpdated(const Configuration &configuration) override;

    int32_t OnContinue(AAFwk::WantParams &wantParams, bool &isAsyncOnContinue,
        const AppExecFwk::AbilityInfo &abilityInfo) override;
    int32_t OnContinueAsyncCB(ani_ref aniWantParamsRef, int32_t status,
        const AppExecFwk::AbilityInfo &abilityInfo) override;
    bool OnContinueReturnPromise(ani_env* aniEnv, ani_ref aniWantParamRef, const AppExecFwk::AbilityInfo &abilityInfo);
    bool CallObjectOnContinueMethod(ani_method method, ani_ref wantParamRef, int32_t& onContinueResult,
        bool& isPromise);

    /**
     * @brief Update Contextconfiguration
     */
    void UpdateContextConfiguration() override;

    /**
     * @brief Called when the system configuration is updated.
     * @param level Indicates the memory trim level, which shows the current memory usage status.
     */
    void OnMemoryLevel(int level) override;

    /**
     * @brief Called when the launch mode of an ability is set to singleInstance. This happens when you re-launch an
     * ability that has been at the top of the ability stack.
     * @param want Indicates the new Want containing information about the ability.
     */
    void OnNewWant(const Want &want) override;

    /**
     * @brief Prepare user data of local Ability.
     * @param reason the reason why framework invoke this function
     * @param wantParams Indicates the user data to be saved.
     * @return result code defined in abilityConstants
     */
    int32_t OnSaveState(int32_t reason, WantParams &wantParams,
        AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnSaveStateResult> *callbackInfo,
        bool &isAsync, AppExecFwk::StateReason stateReason) override;

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
    void OnAbilityResult(int requestCode, int resultCode, const Want &resultData) override;

    sptr<IRemoteObject> CallRequest() override;

    void OnAfterFocusedCommon(bool isFocused) override;

    static void CreateAndBindContext(const std::shared_ptr<AbilityRuntime::AbilityContext> &abilityContext,
        const std::unique_ptr<Runtime>& runtime);

    /**
     * @brief dump ability info
     * @param params dump params that indicate different dump targets
     * @param info dump ability info
     */
    void Dump(const std::vector<std::string> &params, std::vector<std::string> &info) override;

    /**
     * @brief Callback when the ability is shared.You can override this function to implement your own sharing logic.
     * @param wantParams Indicates the user data to be saved.
     * @return the result of OnShare
     */
    int32_t OnShare(WantParams &wantParams) override;

#ifdef SUPPORT_SCREEN
public:
    /**
     * @brief Called after instantiating WindowScene.
     * You can override this function to implement your own processing logic.
     */
    void OnSceneCreated() override;

    /**
     * @brief Called after ability stoped.
     * You can override this function to implement your own processing logic.
     */
    void OnSceneWillDestroy() override;

    /**
     * @brief Called after ability stoped.
     * You can override this function to implement your own processing logic.
     */
    void onSceneDestroyed() override;

    /**
     * @brief Called after ability restored.
     * You can override this function to implement your own processing logic.
     */
    void OnSceneRestored() override;

    /**
     * @brief Called when this ability enters the <b>STATE_FOREGROUND</b> state.
     * The ability in the <b>STATE_FOREGROUND</b> state is visible.
     * You can override this function to implement your own processing logic.
     */
    void OnForeground(const Want &want) override;

    /**
     * @brief Call "onForeground" ets function barely.
     *
     * @param want Want
     */
    void CallOnForegroundFunc(const Want &want) override;

    /**
     * @brief Called before this ability enters the <b>STATE_FOREGROUND</b> state.
     * The ability in the <b>STATE_FOREGROUND</b> state is invisible.
     * You can override this function to implement your own processing logic.
     */
    void OnWillForeground() override;

    /**
     * @brief Called after wms show event.
     * The ability in the <b>STATE_FOREGROUND</b> state is invisible.
     * You can override this function to implement your own processing logic.
     */
    void OnDidForeground() override;

    /**
     * @brief Called when this ability enters the <b>STATE_BACKGROUND</b> state.
     * The ability in the <b>STATE_BACKGROUND</b> state is invisible.
     * You can override this function to implement your own processing logic.
     */
    void OnBackground() override;

    /**
     * @brief Called before OnBackground.
     * The ability in the <b>STATE_BACKGROUND</b> state is invisible.
     * You can override this function to implement your own processing logic.
     */
    void OnWillBackground() override;

    /**
     * @brief Called after wms hiden event.
     * The ability in the <b>STATE_BACKGROUND</b> state is invisible.
     * You can override this function to implement your own processing logic.
     */
    void OnDidBackground() override;

    /**
     * Called when back press is dispatched.
     * Return true if ability will be moved to background; return false if will be terminated
     */
    bool OnBackPress() override;

    /**
     * @brief Called when ability prepare terminate.
     * @param callbackInfo The callbackInfo is used when onPrepareToTerminateAsync is implemented.
     * @param isAsync The returned flag indicates if onPrepareToTerminateAsync is implemented.
     */
    void OnPrepareTerminate(AppExecFwk::AbilityTransactionCallbackInfo<bool> *callbackInfo, bool &isAsync) override;

    /**
     * @brief Execute insight intent when an ability is in foreground, schedule it to foreground repeatly.
     *
     * @param want Want.
     * @param executeParam insight intent execute param.
     * @param callback insight intent async callback.
     */
    void ExecuteInsightIntentRepeateForeground(const Want &want,
        const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
        std::unique_ptr<InsightIntentExecutorAsyncCallback> callback) override;

    /**
     * @brief Execute insight intent when an ability didn't started or in background, schedule it to foreground.
     *
     * @param want Want.
     * @param executeParam insight intent execute param.
     * @param callback insight intent async callback.
     */
    void ExecuteInsightIntentMoveToForeground(const Want &want,
        const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
        std::unique_ptr<InsightIntentExecutorAsyncCallback> callback) override;

    /**
     * @brief Execute insight intent when an ability didn't started, schedule it to background.
     *
     * @param want Want.
     * @param executeParam insight intent execute param.
     * @param callback insight intent async callback.
     */
    virtual void ExecuteInsightIntentBackground(const AAFwk::Want &want,
        const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
        std::unique_ptr<InsightIntentExecutorAsyncCallback> callback) override;
    
    /**
     * @brief Called when startAbility request failed.
     * @param requestId, the requestId.
     * @param element, the element to start ability.
     * @param message, the message to be returned to the calling app.
     */
    void OnAbilityRequestFailure(const std::string &requestId, const AppExecFwk::ElementName &element,
        const std::string &message, int32_t resultCode = 0) override;

    /**
     * @brief Callback for collaboration event.
     *
     * @param wantParams Parameters for the collaboration event.
     * @return int32_t Returns the result code of the collaboration handling.
     */
    int32_t OnCollaborate(WantParams &wantParams) override;

    /**
     * @brief Called when startAbility request succeeded.
     * @param requestId, the requestId.
     * @param element, the element to start ability.
     * @param message, the message to be returned to the calling app.
     */
    void OnAbilityRequestSuccess(const std::string &requestId, const AppExecFwk::ElementName &element,
        const std::string &message) override;

    /**
     * @brief Get WindowStage.
     * @return Returns the ani_object of WindowStage.
     */
    ani_object GetEtsWindowStage() override;

protected:
    void DoOnForeground(const Want &want) override;
    void ContinuationRestore(const Want &want) override;

private:
    bool IsRestorePageStack(const Want &want);
    void RestorePageStack(const Want &want);
    void GetPageStackFromWant(const Want &want, std::string &pageStack);
    void AbilityContinuationOrRecover(const Want &want);
    void UpdateEtsWindowStage(ani_ref windowStage);
    std::unique_ptr<NativeReference> CreateJsAppWindowStage();
    inline bool GetInsightIntentExecutorInfo(const Want &want,
        const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
        InsightIntentExecutorInfo& executeInfo, std::string arkTSMode = AbilityRuntime::CODE_LANGUAGE_ARKTS_1_2);
    
    std::shared_ptr<AppExecFwk::ETSNativeReference> etsWindowStageObj_;
    std::shared_ptr<NativeReference> jsWindowStageObj_;
    int32_t windowMode_ = 0;
#endif

private:
    void OnStartInner(ani_env *env, const Want &want, sptr<AAFwk::SessionInfo> sessionInfo);
    void SetSelfSpecifiedId(ani_env *env, sptr<AAFwk::SessionInfo> sessionInfo);
    bool CallObjectMethod(bool withResult, const char *name, const char *signature, ...);
    ani_object CreateAppWindowStage();
    void SetAbilityContext(std::shared_ptr<AbilityInfo> abilityInfo, std::shared_ptr<Want> want,
        const std::string &moduleName, const std::string &srcPath);
    void DoOnForegroundForSceneIsNull(const Want &want);
    void UpdateAbilityObj(std::shared_ptr<AbilityInfo> abilityInfo,
        const std::string &moduleName, const std::string &srcPath);
    void CreateEtsContext(int32_t screenMode);
    bool BindNativeMethods();
    void RemoveShareRouterByBundleType(const Want &want);
    bool CheckSatisfyTargetAPIVersion(int32_t targetAPIVersion);
    bool BackPressDefaultValue();
    void WriteLifecycleSwitchLog(const std::string lifecycleName);
    int32_t CallSaveState(ani_value args[], WantParams &wantParams, AppExecFwk::StateReason stateReason,
        AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnSaveStateResult> *callbackInfo);

    ETSRuntime &etsRuntime_;
    std::shared_ptr<AppExecFwk::ETSNativeReference> shellContextRef_;
    std::shared_ptr<AppExecFwk::ETSNativeReference> etsAbilityObj_;
    static std::once_flag singletonFlag_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ETS_UI_ABILITY_H