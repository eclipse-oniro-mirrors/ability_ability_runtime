/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_ABILITY_CONTEXT_H

#include "ability_connect_callback.h"
#include "ability_info.h"
#include "ability_lifecycle_observer_interface.h"
#include "caller_callback.h"
#include "configuration.h"
#include "context.h"
#include "free_install_observer_interface.h"
#include "iability_callback.h"
#include "js_ui_extension_callback.h"
#include "mission_info.h"
#include "native_engine/native_reference.h"
#include "native_engine/native_value.h"
#include "start_options.h"
#include "want.h"

#ifdef SUPPORT_GRAPHICS
#ifdef SUPPORT_SCREEN
#include "pixel_map.h"
#endif
#endif

namespace OHOS {
namespace Ace {
class UIContent;
}

namespace AbilityRuntime {
using RuntimeTask = std::function<void(int, const AAFwk::Want&, bool)>;
using PermissionRequestTask = std::function<void(const std::vector<std::string>&, const std::vector<int>&)>;
using RequestDialogResultTask = std::function<void(int32_t resultCode, const AAFwk::Want&)>;
class LocalCallContainer;
constexpr int32_t DEFAULT_INVAL_VALUE = -1;
class AbilityContext : public Context {
public:
    virtual ~AbilityContext() = default;

    /**
     * @brief Starts a new ability.
     * An ability using the AbilityInfo.AbilityType.SERVICE or AbilityInfo.AbilityType.PAGE template uses this method
     * to start a specific ability. The system locates the target ability from installed abilities based on the value
     * of the want parameter and then starts it. You can specify the ability to start using the want parameter.
     *
     * @param want Indicates the Want containing information about the target ability to start.
     * @param requestCode Indicates the request code returned after the ability using the AbilityInfo.AbilityType.PAGE
     * template is started. You can define the request code to identify the results returned by abilities. The value
     * ranges from 0 to 65535. This parameter takes effect only on abilities using the AbilityInfo.AbilityType.PAGE
     * template.
     */
    virtual ErrCode StartAbility(const AAFwk::Want &want, int requestCode) = 0;

    /**
     * @brief Starts a new ability.
     * An ability using the AbilityInfo.AbilityType.SERVICE or AbilityInfo.AbilityType.PAGE template uses this method
     * to start a specific ability. The system locates the target ability from installed abilities based on the value
     * of the want parameter and then starts it. You can specify the ability to start using the want parameter.
     *
     * @param want Indicates the Want containing information about the target ability to start.
     * @param accountId ability caller accountId.
     * @param requestCode Indicates the request code returned after the ability using the AbilityInfo.AbilityType.PAGE
     * template is started. You can define the request code to identify the results returned by abilities. The value
     * ranges from 0 to 65535. This parameter takes effect only on abilities using the AbilityInfo.AbilityType.PAGE
     * template.
     */
    virtual ErrCode StartAbilityWithAccount(const AAFwk::Want &want, int accountId, int requestCode) = 0;

    /**
     * @brief Starts a new ability.
     * An ability using the AbilityInfo.AbilityType.SERVICE or AbilityInfo.AbilityType.PAGE template uses this method
     * to start a specific ability. The system locates the target ability from installed abilities based on the value
     * of the want parameter and the value of the start options and then starts it. You can specify the ability to
     * start using the two parameters.
     *
     * @param want Indicates the Want containing application side information about the target ability to start.
     * @param startOptions Indicates the StartOptions containing service side information about the target ability to
     * start.
     * @param requestCode Indicates the request code returned after the ability using the AbilityInfo.AbilityType.PAGE
     * template is started. You can define the request code to identify the results returned by abilities. The value
     * ranges from 0 to 65535. This parameter takes effect only on abilities using the AbilityInfo.AbilityType.PAGE
     * template.
     */
    virtual ErrCode StartAbility(const AAFwk::Want &want, const AAFwk::StartOptions &startOptions, int requestCode) = 0;

    /**
     * @brief Starts a new ability using the original caller information.
     * Start a new ability as if it was started by the ability that started current ability. This is for the confirm
     * ability and selection ability, which passthrough their want to the target.
     *
     * @param want Indicates the Want containing information about the target ability to start.
     * @param requestCode Indicates the request code returned after the ability using the AbilityInfo.AbilityType.PAGE
     * template is started. You can define the request code to identify the results returned by abilities. The value
     * ranges from 0 to 65535. This parameter takes effect only on abilities using the AbilityInfo.AbilityType.PAGE
     * template.
     */
    virtual ErrCode StartAbilityAsCaller(const AAFwk::Want &want, int requestCode) = 0;

    /**
     * @brief Starts a new ability using the original caller information.
     * Start a new ability as if it was started by the ability that started current ability. This is for the confirm
     * ability and selection ability, which passthrough their want to the target.
     *
     * @param want Indicates the Want containing information about the target ability to start.
     * @param startOptions Indicates the StartOptions containing service side information about the target ability to
     * start.
     * @param requestCode Indicates the request code returned after the ability using the AbilityInfo.AbilityType.PAGE
     * template is started. You can define the request code to identify the results returned by abilities. The value
     * ranges from 0 to 65535. This parameter takes effect only on abilities using the AbilityInfo.AbilityType.PAGE
     * template.
     */
    virtual ErrCode StartAbilityAsCaller(const AAFwk::Want &want, const AAFwk::StartOptions &startOptions,
        int requestCode) = 0;

    /**
     * @brief Starts a new ability.
     * An ability using the AbilityInfo.AbilityType.SERVICE or AbilityInfo.AbilityType.PAGE template uses this method
     * to start a specific ability. The system locates the target ability from installed abilities based on the value
     * of the want parameter and the value of the start options and then starts it. You can specify the ability to
     * start using the two parameters.
     *
     * @param want Indicates the Want containing application side information about the target ability to start.
     * @param accountId caller userId.
     * @param startOptions Indicates the StartOptions containing service side information about the target ability to
     * start.
     * @param requestCode Indicates the request code returned after the ability using the AbilityInfo.AbilityType.PAGE
     * template is started. You can define the request code to identify the results returned by abilities. The value
     * ranges from 0 to 65535. This parameter takes effect only on abilities using the AbilityInfo.AbilityType.PAGE
     * template.
     */
    virtual ErrCode StartAbilityWithAccount(
        const AAFwk::Want &want, int accountId, const AAFwk::StartOptions &startOptions, int requestCode) = 0;

    virtual ErrCode StartAbilityForResult(const AAFwk::Want &Want, int requestCode, RuntimeTask &&task) = 0;

    virtual ErrCode StartAbilityForResultWithAccount(
        const AAFwk::Want &Want, int accountId, int requestCode, RuntimeTask &&task) = 0;

    virtual ErrCode StartAbilityForResult(const AAFwk::Want &Want, const AAFwk::StartOptions &startOptions,
        int requestCode, RuntimeTask &&task) = 0;

    virtual ErrCode StartAbilityForResultWithAccount(const AAFwk::Want &Want, int accountId,
        const AAFwk::StartOptions &startOptions, int requestCode, RuntimeTask &&task) = 0;

    virtual ErrCode StartServiceExtensionAbility(const AAFwk::Want &want, int32_t accountId = -1) = 0;

    virtual ErrCode StartUIServiceExtensionAbility(const AAFwk::Want &want, int32_t accountId = -1) = 0;

    virtual ErrCode StopServiceExtensionAbility(const AAFwk::Want& want, int32_t accountId = -1) = 0;

    virtual ErrCode TerminateAbilityWithResult(const AAFwk::Want &want, int resultCode) = 0;

    virtual ErrCode BackToCallerAbilityWithResult(const AAFwk::Want &want, int resultCode, int64_t requestCode) = 0;

    virtual ErrCode RestoreWindowStage(napi_env env, napi_value contentStorage) = 0;

    virtual void OnAbilityResult(int requestCode, int resultCode, const AAFwk::Want &resultData) = 0;

    virtual ErrCode RequestModalUIExtension(const AAFwk::Want& want) = 0;

    virtual ErrCode OpenLink(const AAFwk::Want& want, int requestCode) = 0;

    virtual ErrCode OpenAtomicService(AAFwk::Want& want, const AAFwk::StartOptions &options, int requestCode,
        RuntimeTask &&task) = 0;

    virtual ErrCode AddFreeInstallObserver(const sptr<AbilityRuntime::IFreeInstallObserver> &observer) = 0;

    virtual ErrCode ChangeAbilityVisibility(bool isShow) { return 0; }

    /**
    * @brief Connects the current ability to an ability using the AbilityInfo.AbilityType.SERVICE template.
    *
    * @param want Indicates the want containing information about the ability to connect
    * @param connectCallback Indicates the callback object when the target ability is connected.
    * @return True means success and false means failure
    */
    virtual ErrCode ConnectAbility(const AAFwk::Want &want, const sptr<AbilityConnectCallback> &connectCallback) = 0;

    /**
     * @brief Connects the current ability to an ability using the AbilityInfo.AbilityType.SERVICE template.
     * @param want Indicates the want containing information about the ability to connect
     * @param accountId caller userId.
     * @param connectCallback Indicates the callback object when the target ability is connected.
     * @return True means success and false means failure
     */
    virtual ErrCode ConnectAbilityWithAccount(const AAFwk::Want &want, int accountId,
                                const sptr<AbilityConnectCallback> &connectCallback) = 0;

    /**
    * @brief Disconnects the current ability from an ability
    *
    * @param want Indicates the want containing information about the ability to disconnect
    * @param connectCallback Indicates the callback object when the target ability is connected.
    * is set up. The IAbilityConnection object uniquely identifies a connection between two abilities.
    */
    virtual void DisconnectAbility(const AAFwk::Want &want, const sptr<AbilityConnectCallback> &connectCallback,
        int32_t accountId = -1) = 0;

    /**
     * @brief get ability info of the current ability
     *
     * @return the ability info of the current ability
     */
    virtual std::shared_ptr<AppExecFwk::AbilityInfo> GetAbilityInfo() const = 0;

    /**
     * @brief Minimize the current ability.
     *
     * @param fromUser mark the minimize operation source.
     */
    virtual void MinimizeAbility(bool fromUser = false) = 0;

    /**
     * @brief Get OnBackPressedCallBack.
     * @param needMoveToBackground true if ability will be moved to background; false if ability will be terminated.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual ErrCode OnBackPressedCallBack(bool &needMoveToBackground) = 0;

    virtual ErrCode MoveAbilityToBackground() = 0;

    virtual ErrCode MoveUIAbilityToBackground() = 0;

    virtual ErrCode TerminateSelf() = 0;

    virtual ErrCode CloseAbility() = 0;

    /**
     * @brief Get ContentStorage.
     *
     * @return Returns the ContentStorage.
     */
    virtual std::unique_ptr<NativeReference>& GetContentStorage() = 0;

    /**
     * call function by callback object
     *
     * @param want Request info for ability.
     * @param callback Indicates the callback object.
     * @param accountId Indicates the account to start.
     *
     * @return Returns zero on success, others on failure.
     */
    virtual ErrCode StartAbilityByCall(const AAFwk::Want& want, const std::shared_ptr<CallerCallBack> &callback,
        int32_t accountId = DEFAULT_INVAL_VALUE) = 0;

    /**
     * caller release by callback object
     *
     * @param callback Indicates the callback object.
     *
     * @return Returns zero on success, others on failure.
     */
    virtual ErrCode ReleaseCall(const std::shared_ptr<CallerCallBack> &callback) = 0;

    /**
     * clear failed call connection by callback object
     *
     * @param callback Indicates the callback object.
     *
     * @return void.
     */
    virtual void ClearFailedCallConnection(const std::shared_ptr<CallerCallBack> &callback) = 0;

    /**
     * @brief Get LocalCallContainer.
     *
     * @return Returns the LocalCallContainer.
     */
    virtual std::shared_ptr<LocalCallContainer> GetLocalCallContainer() = 0;

    virtual void SetConfiguration(const std::shared_ptr<AppExecFwk::Configuration> &config) = 0;

    virtual void RegisterAbilityCallback(std::weak_ptr<AppExecFwk::IAbilityCallback> abilityCallback) = 0;

    virtual void SetWeakSessionToken(const wptr<IRemoteObject>& sessionToken) = 0;
    virtual void SetAbilityRecordId(int32_t abilityRecordId) = 0;
    virtual int32_t GetAbilityRecordId() = 0;

    /**
     * @brief Requests dialogService from the system.
     * This method is called for dialog request. This is an asynchronous method. When it is executed,
     * the task will be called back.
     *
     * @param env js env.
     * @param want Indicates the dialog service to be requested.
     * @param task The callback or promise fo js interface.
     * @return Returns ERR_OK if success.
     */
    virtual ErrCode RequestDialogService(napi_env env, AAFwk::Want &want, RequestDialogResultTask &&task) = 0;

    /**
     * @brief Requests dialogService from the system.
     * This method is called for dialog request. This is an asynchronous method. When it is executed,
     * the task will be called back.
     *
     * @param want Indicates the dialog service to be requested.
     * @param task The callback or promise fo js interface.
     * @return Returns ERR_OK if success.
     */
    virtual ErrCode RequestDialogService(AAFwk::Want &want, RequestDialogResultTask &&task) = 0;

    /**
     * @brief Report drawn completed.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual ErrCode ReportDrawnCompleted() = 0;

    virtual ErrCode GetMissionId(int32_t &missionId) = 0;

    /**
     * Set mission continue state of this ability.
     *
     * @param state the mission continuation state of this ability.
     * @return Returns ERR_OK if success.
     */
    virtual ErrCode SetMissionContinueState(const AAFwk::ContinueState &state) = 0;

    /**
     * Register lifecycle observer on ability.
     *
     * @param observer the lifecycle observer to be registered on ability.
     */
    virtual void RegisterAbilityLifecycleObserver(const std::shared_ptr<AppExecFwk::ILifecycleObserver> &observer) = 0;

    /**
     * Unregister lifecycle observer on ability.
     *
     * @param observer the lifecycle observer to be unregistered on ability.
     */
    virtual void UnregisterAbilityLifecycleObserver(
        const std::shared_ptr<AppExecFwk::ILifecycleObserver> &observer) = 0;

    virtual void SetRestoreEnabled(bool enabled) = 0;
    virtual bool GetRestoreEnabled() = 0;

    virtual std::shared_ptr<AAFwk::Want> GetWant() = 0;

#ifdef SUPPORT_GRAPHICS
#ifdef SUPPORT_SCREEN
    /**
     * @brief Set mission label of this ability.
     *
     * @param label the label of this ability.
     * @return Returns ERR_OK if success.
     */
    virtual ErrCode SetMissionLabel(const std::string &label) = 0;

    /**
     * @brief Set mission icon of this ability.
     *
     * @param icon the icon of this ability.
     * @return Returns ERR_OK if success.
     */
    virtual ErrCode SetMissionIcon(const std::shared_ptr<OHOS::Media::PixelMap> &icon) = 0;

    virtual int GetCurrentWindowMode() = 0;

    /**
     * @brief Get window rectangle of this ability.
     *
     * @param the left position of window rectangle.
     * @param the top position of window rectangle.
     * @param the width position of window rectangle.
     * @param the height position of window rectangle.
     */
    virtual void GetWindowRect(int32_t &left, int32_t &top, int32_t &width, int32_t &height) = 0;

    /**
     * @brief Get ui content object.
     *
     * @return UIContent object of ACE.
     */
    virtual Ace::UIContent* GetUIContent() = 0;
    virtual ErrCode StartAbilityByType(const std::string &type, AAFwk::WantParams &wantParam,
        const std::shared_ptr<JsUIExtensionCallback> &uiExtensionCallbacks) = 0;
    virtual ErrCode CreateModalUIExtensionWithApp(const AAFwk::Want &want) = 0;
    virtual void EraseUIExtension(int32_t sessionId) = 0;
#endif
#endif
    virtual bool IsTerminating() = 0;
    virtual void SetTerminating(bool state) = 0;
    virtual void InsertResultCallbackTask(int requestCode, RuntimeTask&& task) = 0;
    virtual void RemoveResultCallbackTask(int requestCode) = 0;
    using SelfType = AbilityContext;
    static const size_t CONTEXT_TYPE_ID;

protected:
    bool IsContext(size_t contextTypeId) override
    {
        return contextTypeId == CONTEXT_TYPE_ID || Context::IsContext(contextTypeId);
    }
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ABILITY_CONTEXT_H
