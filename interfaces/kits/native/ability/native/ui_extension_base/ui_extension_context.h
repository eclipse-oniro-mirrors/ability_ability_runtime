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

#ifndef OHOS_ABILITY_RUNTIME_UI_EXTENSION_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_UI_EXTENSION_CONTEXT_H

#include <map>

#include "ability_connect_callback.h"
#include "extension_context.h"
#include "free_install_observer_interface.h"
#include "start_options.h"
#include "ui_holder_extension_context.h"
#include "want.h"
#include "js_ui_extension_callback.h"
#include "string_wrapper.h"
#ifdef SUPPORT_SCREEN
#include "window.h"
#endif // SUPPORT_SCREEN

namespace OHOS {
namespace AbilityRuntime {
using RuntimeTask = std::function<void(int, const AAFwk::Want &, bool)>;
using AbilityConfigUpdateCallback = std::function<void(AppExecFwk::Configuration &config)>;
/**
 * @brief context supply for UIExtension
 *
 */
class UIExtensionContext : public UIHolderExtensionContext {
public:
    UIExtensionContext() = default;
    virtual ~UIExtensionContext() = default;

    /**
     * @brief Starts a new ability.
     * An ability using the AbilityInfo.AbilityType.EXTENSION or AbilityInfo.AbilityType.PAGE template uses this method
     * to start a specific ability. The system locates the target ability from installed abilities based on the value
     * of the want parameter and then starts it. You can specify the ability to start using the want parameter.
     *
     * @param want Indicates the Want containing information about the target ability to start.
     *
     * @return errCode ERR_OK on success, others on failure.
     */
    virtual ErrCode StartAbility(const AAFwk::Want &want) const;
    virtual ErrCode StartAbility(const AAFwk::Want &want, const AAFwk::StartOptions &startOptions) const;
    virtual ErrCode StartAbility(const AAFwk::Want &want, int requestCode) const;
    virtual ErrCode StartUIServiceExtension(const AAFwk::Want& want, int32_t accountId = -1) const;
    /**
     * @brief Destroys the current ui extension ability.
     *
     * @return errCode ERR_OK on success, others on failure.
     */
    virtual ErrCode TerminateSelf();
        /**
     * @brief Connects the current ability to an ability using the AbilityInfo.AbilityType.SERVICE template.
     *
     * @param want Indicates the want containing information about the ability to connect
     *
     * @param conn Indicates the callback object when the target ability is connected.
     *
     * @return Returns zero on success, others on failure.
     */
    virtual ErrCode ConnectAbility(
        const AAFwk::Want &want, const sptr<AbilityConnectCallback> &connectCallback) const;

    virtual ErrCode ConnectUIServiceExtensionAbility(
        const AAFwk::Want &want, const sptr<AbilityConnectCallback> &connectCallback) const;

    /**
     * @brief Disconnects the current ability from an ability.
     *
     * @param conn Indicates the IAbilityConnection callback object passed by connectAbility after the connection
     * is set up. The IAbilityConnection object uniquely identifies a connection between two abilities.
     *
     * @return errCode ERR_OK on success, others on failure.
     */
    virtual ErrCode DisconnectAbility(
        const AAFwk::Want &want, const sptr<AbilityConnectCallback> &connectCallback) const;

    /**
     * @brief Start service extension ability.
     *
     * @param want The want indicates a service extension ability.
     * @param accountId The account id.
     * @return ErrCode ERR_OK on success, others on failure.
     */
    ErrCode StartServiceExtensionAbility(const AAFwk::Want &want, int32_t accountId = -1);

    ErrCode StartUIAbilities(const std::vector<AAFwk::Want> &wantList, const std::string &requestKey);

    /**
     * Start other ability for result.
     *
     * @param want Information of other ability.
     * @param startOptions Indicates the StartOptions containing service side information about the target ability to
     * start.
     * @param requestCode Request code for abilityMS to return result.
     * @param task Represent std::function<void(int, const AAFwk::Want &, bool)>.
     *
     * @return errCode ERR_OK on success, others on failure.
     */
    virtual ErrCode StartAbilityForResult(const AAFwk::Want &want, int requestCode, RuntimeTask &&task);
    virtual ErrCode StartAbilityForResult(
        const AAFwk::Want &want, const AAFwk::StartOptions &startOptions, int requestCode, RuntimeTask &&task);

    /**
     * Starts a new ability for result using the original caller information.
     *
     * @param want Information of other ability.
     * @param requestCode Request code for abilityMS to return result.
     * @param task Represent std::function<void(int, const AAFwk::Want &, bool)>.
     *
     * @return errCode ERR_OK on success, others on failure.
     */
    virtual ErrCode StartAbilityForResultAsCaller(const AAFwk::Want &want, int requestCode, RuntimeTask &&task);

    /**
     * Starts a new ability for result using the original caller information.
     *
     * @param want Information of other ability.
     * @param startOptions Indicates the StartOptions containing service side information about the target ability to
     * start.
     * @param requestCode Request code for abilityMS to return result.
     * @param task Represent std::function<void(int, const AAFwk::Want &, bool)>.
     *
     * @return errCode ERR_OK on success, others on failure.
     */
    virtual ErrCode StartAbilityForResultAsCaller(
        const AAFwk::Want &want, const AAFwk::StartOptions &startOptions, int requestCode, RuntimeTask &&task);

    /**
     * @brief Called when startAbilityForResult(ohos.aafwk.content.Want,int) is called to start an extension ability
     * and the result is returned.
     * @param requestCode Indicates the request code returned after the ability is started. You can define the request
     * code to identify the results returned by abilities. The value ranges from 0 to 65535.
     * @param resultCode Indicates the result code returned after the ability is started. You can define the result
     * code to identify an error.
     * @param resultData Indicates the data returned after the ability is started. You can define the data returned. The
     * value can be null.
     */
    virtual void OnAbilityResult(int requestCode, int resultCode, const AAFwk::Want &resultData);

    virtual int GenerateCurRequestCode();

    virtual ErrCode ReportDrawnCompleted();

    std::shared_ptr<Global::Resource::ResourceManager> GetResourceManager() const override;
    void SetAbilityResourceManager(std::shared_ptr<Global::Resource::ResourceManager> abilityResourceMgr);
    void RegisterAbilityConfigUpdateCallback(AbilityConfigUpdateCallback abilityConfigUpdateCallback);
    std::shared_ptr<AppExecFwk::Configuration> GetAbilityConfiguration() const;
    void SetAbilityConfiguration(const AppExecFwk::Configuration &config);
    void SetAbilityColorMode(int32_t colorMode);

    /**
     * @brief Send destroy request to the host component.
     */
    void RequestComponentTerminate();

#ifdef SUPPORT_SCREEN
    void SetWindow(sptr<Rosen::Window> window);

    sptr<Rosen::Window> GetWindow();

    Ace::UIContent* GetUIContent() override;
#endif // SUPPORT_SCREEN

    ErrCode OpenLink(const AAFwk::Want& want, int reuqestCode);

    ErrCode OpenAtomicService(AAFwk::Want& want, const AAFwk::StartOptions &options, int requestCode,
        RuntimeTask &&task);

    ErrCode AddFreeInstallObserver(const sptr<AbilityRuntime::IFreeInstallObserver> &observer);

    void InsertResultCallbackTask(int requestCode, RuntimeTask&& task);

    void RemoveResultCallbackTask(int requestCode);

    ErrCode AddCompletionHandler(const std::string &requestId, OnRequestResult onRequestSucc,
        OnRequestResult onRequestFail) override;

    void OnRequestSuccess(const std::string &requestId, const AppExecFwk::ElementName &element,
        const std::string &message) override;

    void OnRequestFailure(const std::string &requestId, const AppExecFwk::ElementName &element,
        const std::string &message) override;

    /**
     * @brief Start a new ability using type;
     * @return errCode ERR_OK on success, others on failure.
    */
    ErrCode StartAbilityByType(const std::string &type,
        AAFwk::WantParams &wantParam, const std::shared_ptr<JsUIExtensionCallback> &uiExtensionCallbacks);
    bool IsTerminating();
    void SetTerminating(bool state);

    int32_t GetScreenMode() const;
    void SetScreenMode(int32_t screenMode);
    using SelfType = UIExtensionContext;
    static const size_t CONTEXT_TYPE_ID;
    int32_t isNotAllow = -1;
#ifdef SUPPORT_SCREEN
protected:
    bool IsContext(size_t contextTypeId) override
    {
        return contextTypeId == CONTEXT_TYPE_ID || UIHolderExtensionContext::IsContext(contextTypeId);
    }

    sptr<Rosen::Window> window_ = nullptr;
#endif // SUPPORT_SCREEN
private:
    static int ILLEGAL_REQUEST_CODE;
    std::map<int, RuntimeTask> resultCallbacks_;
    static int32_t curRequestCode_;
    static std::mutex requestCodeMutex_;
    std::mutex mutexlock_;
    int32_t screenMode_ = AAFwk::IDLE_SCREEN_MODE;
    std::shared_ptr<Global::Resource::ResourceManager> abilityResourceMgr_ = nullptr;
    AbilityConfigUpdateCallback abilityConfigUpdateCallback_ = nullptr;
    std::shared_ptr<AppExecFwk::Configuration> abilityConfiguration_ = nullptr;
    bool isTerminating_ = false;
    /**
     * @brief Get Current Ability Type
     *
     * @return Current Ability Type
     */
    OHOS::AppExecFwk::AbilityType GetAbilityInfoType() const;

    void OnAbilityResultInner(int requestCode, int resultCode, const AAFwk::Want &resultData);
    
    std::mutex onRequestResultMutex_;
    std::vector<OnRequestResultElement> onRequestResults_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_UI_EXTENSION_CONTEXT_H
