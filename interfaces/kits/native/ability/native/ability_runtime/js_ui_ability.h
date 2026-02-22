/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_JS_UI_ABILITY_H
#define OHOS_ABILITY_RUNTIME_JS_UI_ABILITY_H

#include "ability_delegator_infos.h"
#include "freeze_util.h"
#include "js_embeddable_ui_ability_context.h"
#include "ui_ability.h"
#include "recovery_param.h"

class NativeReference;

namespace OHOS {
namespace AbilityRuntime {
class JsRuntime;
struct InsightIntentExecutorInfo;
using AbilityHandler = AppExecFwk::AbilityHandler;
using AbilityInfo = AppExecFwk::AbilityInfo;
using OHOSApplication = AppExecFwk::OHOSApplication;
using Want = AppExecFwk::Want;
using AbilityStartSetting = AppExecFwk::AbilityStartSetting;
using Configuration = AppExecFwk::Configuration;
using InsightIntentExecuteResult = AppExecFwk::InsightIntentExecuteResult;
using InsightIntentExecuteParam = AppExecFwk::InsightIntentExecuteParam;
using InsightIntentExecutorAsyncCallback = AppExecFwk::InsightIntentExecutorAsyncCallback;

struct CallOnSaveStateInfo {
    AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnSaveStateResult> *callbackInfo;
    AppExecFwk::WantParams wantParams;
    AppExecFwk::StateReason reason;
};

struct CallObjectMethodParams {
    bool withResult = false;
    bool showMethodNotFoundLog = true;
};

class JsUIAbility : public UIAbility {
public:
    /**
     * @brief Create a JsUIAbility instance through the singleton pattern
     * @param runtime The runtime of the ability
     * @return Returns the JsUIability Instance point
     */
    static UIAbility *Create(const std::unique_ptr<Runtime> &runtime);

    explicit JsUIAbility(JsRuntime &jsRuntime);
    ~JsUIAbility() override;

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

    static void CreateAndBindContext(const std::shared_ptr<AbilityRuntime::AbilityContext> &abilityContext,
        const std::unique_ptr<Runtime>& runtime);

    /**
     * @brief OnStart,Start JsUIability
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
     * @brief The sync callback of OnContinue.
     */
    int32_t OnContinueSyncCB(napi_value result, WantParams &wantParams, napi_value jsWantParams);

    /**
     * @brief The async callback of OnContinue.
     */
    int32_t OnContinueAsyncCB(napi_ref jsWantParams, int32_t status,
        const AppExecFwk::AbilityInfo &abilityInfo) override;

    /**
     * @brief Prepare user data of local Ability.
     * @param wantParams Indicates the user data to be saved.
     * @return If the ability is willing to continue and data saved successfully, it returns 0;
     * otherwise, it returns errcode.
     */
    int32_t OnContinue(WantParams &wantParams, bool &isAsyncOnContinue,
        const AppExecFwk::AbilityInfo &abilityInfo) override;

    /**
     * @brief Update configuration
     * @param configuration Indicates the updated configuration information.
     */
    void OnConfigurationUpdated(const Configuration &configuration) override;

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

    /**
     * @brief request a remote object of callee from this ability.
     * @return Returns the remote object of callee.
     */
    sptr<IRemoteObject> CallRequest() override;

    /**
     * @brief dump ability info
     * @param params dump params that indicate different dump targets
     * @param info dump ability info
     */
    void Dump(const std::vector<std::string> &params, std::vector<std::string> &info) override;

    void OnAfterFocusedCommon(bool isFocused) override;

    /**
     * @brief Get JsAbility
     * @return Return the JsAbility
     */
    std::shared_ptr<NativeReference> GetJsAbility();

    /**
     * @brief Callback when the ability is shared.You can override this function to implement your own sharing logic.
     * @param wantParams Indicates the user data to be saved.
     * @return the result of OnShare
     */
    int32_t OnShare(WantParams &wantParams) override;

    /**
     * @brief Set the continueState of an application to window.
     * @param state Indicates the continueState of an application.
     */
    void SetContinueState(int32_t state) override;

    void NotifyWindowDestroy() override;

#ifdef SUPPORT_SCREEN
public:
    /**
     * @brief Called after instantiating WindowScene.
     * You can override this function to implement your own processing logic.
     */
    void OnSceneCreated() override;

    /**
     * @brief Get WindowStage.
     * @return Returns the napi_value of WindowStage.
     */
    napi_value GetWindowStage() override;

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
     * @brief Call "onForeground" js function barely.
     *
     * @param want Want
     */
    void CallOnForegroundFunc(const Want &want) override;

    /**
     * @brief Request focus for current window, can be override.
     *
     * @param want Want
     */
    void RequestFocus(const Want &want) override;

    /**
     * @brief Called when this ability enters the <b>STATE_BACKGROUND</b> state.
     * The ability in the <b>STATE_BACKGROUND</b> state is invisible.
     * You can override this function to implement your own processing logic.
     */
    void OnBackground() override;

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
     * @brief Get JsWindow Stage
     * @return Returns the current NativeReference
     */
    std::shared_ptr<NativeReference> GetJsWindowStage();

    /**
     * @brief Get JsRuntime
     * @return Returns the current JsRuntime
     */
    const JsRuntime &GetJsRuntime();

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
     * @brief Execute insight intent when an ability start with page insight intent.
     *
     * @param want Want.
     * @param executeParam insight intent execute param.
     * @param callback insight intent async callback.
     */
    void ExecuteInsightIntentPage(const AAFwk::Want &want,
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
     * @brief Callback for collaboration event.
     *
     * @param wantParams Parameters for the collaboration event.
     * @return int32_t Returns the result code of the collaboration handling.
     */
    int32_t OnCollaborate(WantParams &wantParams) override;

    /**
     * @brief Called when startAbility request failed.
     * @param requestId, the requestId.
     * @param element, the element to start ability.
     * @param message, the message to be returned to the calling app.
     */
    void OnAbilityRequestFailure(const std::string &requestId, const AppExecFwk::ElementName &element,
        const std::string &message, int32_t resultCode = 0) override;

    /**
     * @brief Called when startAbility request succeeded.
     * @param requestId, the requestId.
     * @param element, the element to start ability.
     * @param message, the message to be returned to the calling app.
     */
    void OnAbilityRequestSuccess(const std::string &requestId, const AppExecFwk::ElementName &element,
        const std::string &message) override;

protected:
    void DoOnForeground(const Want &want) override;
    void ContinuationRestore(const Want &want) override;

private:
    bool IsRestorePageStack(const Want &want);
    void RestorePageStack(const Want &want);
    void GetPageStackFromWant(const Want &want, std::string &pageStack);
    void AbilityContinuationOrRecover(const Want &want);
    void UpdateJsWindowStage(napi_value windowStage);
    inline bool GetInsightIntentExecutorInfo(const Want &want,
        const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
        InsightIntentExecutorInfo& executeInfo);
    void SetInsightIntentParam(const Want &want, bool coldStart);
    void RegisterDelayResultCallback(const std::shared_ptr<InsightIntentExecuteParam> &executeParam);

    std::shared_ptr<NativeReference> jsWindowStageObj_;
    int32_t windowMode_ = 0;
#endif

private:
    napi_value CallObjectMethod(const char *name, napi_value const *argv = nullptr, size_t argc = 0,
        bool withResult = false, bool showMethodNotFoundLog = true);
    napi_value CallObjectMethod(const char *name, bool &hasCaughtException,
        const CallObjectMethodParams &callObjectMethodParams, napi_value const *argv = nullptr, size_t argc = 0);
    bool CheckPromise(napi_value result);
    bool CallPromise(napi_value result, AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo);
    bool CallPromise(napi_value result, AppExecFwk::AbilityTransactionCallbackInfo<int32_t> *callbackInfo);
    bool CallPromise(napi_value result, AppExecFwk::AbilityTransactionCallbackInfo<bool> *callbackInfo);
    int32_t CallSaveStatePromise(napi_value result, CallOnSaveStateInfo info);
    std::unique_ptr<NativeReference> CreateAppWindowStage();
    sptr<IRemoteObject> SetNewRuleFlagToCallee(napi_env env, napi_value remoteJsObj);
    void SetAbilityContext(std::shared_ptr<AbilityInfo> abilityInfo, std::shared_ptr<AAFwk::Want> want,
        const std::string &moduleName, const std::string &srcPath, bool &createObjSuc);
    void DoOnForegroundForSceneIsNull(const Want &want);
    void GetDumpInfo(
        napi_env env, napi_value dumpInfo, napi_value onDumpInfo, std::vector<std::string> &info);
    void AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState state, const std::string &methodName) const;
    void AddLifecycleEventAfterJSCall(FreezeUtil::TimeoutState state, const std::string &methodName) const;
    void CreateJSContext(napi_env env, napi_value &contextObj, int32_t screenMode);
    bool CheckSatisfyTargetAPIVersion(int32_t targetAPIVersion);
    bool BackPressDefaultValue();
    void UpdateAbilityObj(std::shared_ptr<AbilityInfo> abilityInfo,
        const std::string &moduleName, const std::string &srcPath);
    void ReleaseOnContinueAsset(const napi_env env, napi_value &promise,
        napi_ref &jsWantParamsRef, AppExecFwk::AbilityTransactionCallbackInfo<int32_t> *callbackInfo);
    void RemoveShareRouterByBundleType(const Want &want);
    void WriteLifecycleSwitchLog(const std::string lifecycleName);
    void HandleAbilityDelegatorStart();

    JsRuntime &jsRuntime_;
    std::shared_ptr<NativeReference> shellContextRef_;
    std::shared_ptr<NativeReference> jsAbilityObj_;
    sptr<IRemoteObject> remoteCallee_;
    bool reusingWindow_ = false;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_UI_ABILITY_H
