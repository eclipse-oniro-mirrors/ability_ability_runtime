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

#ifndef OHOS_ABILITY_RUNTIME_ETS_UI_EXTENSION_H
#define OHOS_ABILITY_RUNTIME_ETS_UI_EXTENSION_H

#include <mutex>
#include <unordered_set>

#include "ability_handler.h"
#include "ani.h"
#include "configuration.h"
#include "ets_runtime.h"
#include "insight_intent_executor_info.h"
#include "ets_ui_extension_content_session.h"
#include "ui_extension.h"
#include "ui_extension_context.h"
#ifdef SUPPORT_GRAPHICS
#include "display_manager.h"
#include "window_manager.h"
#endif // SUPPORT_GRAPHICS

namespace OHOS {
namespace AbilityRuntime {
class UIExtension;
class UIExtensionContext;
struct ContentSessionType {
    ani_ref etsContentSession = nullptr;
    std::shared_ptr<NativeReference> jsContentSession = nullptr;
};

/**
 * @brief Basic ui extension components.
 */
class EtsUIExtension : public UIExtension {
public:
    explicit EtsUIExtension(ETSRuntime &etsRuntime);
    virtual ~EtsUIExtension() override;

    /**
     * @brief Create EtsUIExtension.
     *
     * @param runtime The runtime.
     * @return The EtsUIExtension instance.
     */
    static EtsUIExtension* Create(const std::unique_ptr<Runtime> &etsRuntime);

    /**
     * @brief Init the ui extension.
     *
     * @param record the ui extension record.
     * @param application the application info.
     * @param handler the ui extension handler.
     * @param token the remote token.
     */
    void Init(const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &record,
        const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
        std::shared_ptr<AppExecFwk::AbilityHandler> &handler,
        const sptr<IRemoteObject> &token) override;

    /**
     * @brief Called when this ui extension is started. You must override this function if you want to perform some
     *        initialization operations during ui extension startup.
     *
     * This function can be called only once in the entire lifecycle of an ui extension.
     *
     * @param Want Indicates the {@link Want} structure containing startup information about the ui extension.
     * @param sessionInfo The session info of the ability.
     */
    void OnStart(const AAFwk::Want &want, sptr<AAFwk::SessionInfo> sessionInfo) override;

    /**
     * @brief Called when this ui extension enters the <b>STATE_STOP</b> state.
     *
     * The ui extension in the <b>STATE_STOP</b> is being destroyed.
     * You can override this function to implement your own processing logic.
     */
    void OnStop() override;
    void OnStop(AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback) override;
    /**
     * @brief The callback of OnStop.
     */
    virtual void OnStopCallBack() override;
    /**
     * @brief Called when this extension enters the <b>STATE_FOREGROUND</b> state.
     *
     *
     * The extension in the <b>STATE_FOREGROUND</b> state is visible.
     * You can override this function to implement your own processing logic.
     */
    void OnForeground(const Want &want, sptr<AAFwk::SessionInfo> sessionInfo) override;

    /**
     * @brief Called when this extension enters the <b>STATE_BACKGROUND</b> state.
     *
     *
     * The extension in the <b>STATE_BACKGROUND</b> state is invisible.
     * You can override this function to implement your own processing logic.
     */
    void OnBackground() override;

    /**
     * @brief Called when ui extension need dump info.
     *
     * @param params The params from ui extension.
     * @param info The dump info to show.
     */
    void Dump(const std::vector<std::string> &params, std::vector<std::string> &info) override;

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
    void OnAbilityResult(int requestCode, int resultCode, const Want &resultData) override;

    /**
     * @brief Called when configuration changed, including system configuration and window configuration.
     */
    void ConfigurationUpdated() override;

    /**
     * @brief Called when the configuration changes, including system and window configuration.
     *
     * @param configuration The updated configuration information.
     */
    void OnAbilityConfigurationUpdated(const AppExecFwk::Configuration &configuration) override;

private:
    virtual void BindContext(ani_env *env, std::shared_ptr<AAFwk::Want> want);
    ani_object CreateETSContext(ani_env *env, std::shared_ptr<UIExtensionContext> context, int32_t screenMode);

    bool CallObjectMethod(bool withResult, const char *name, const char *signature, ...);
    ani_status CallOnDisconnect(const AAFwk::Want &want, bool withResult = false);
    void DestroyWindow(const sptr<AAFwk::SessionInfo> &sessionInfo) override;
    bool ForegroundWindowInitInsightIntentExecutorInfo(const AAFwk::Want &want,
        const sptr<AAFwk::SessionInfo> &sessionInfo, InsightIntentExecutorInfo &executorInfo,
        const std::string &arkTSMode);
    bool ForegroundWindowWithInsightIntent(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo,
        bool needForeground) override;
    bool HandleSessionCreate(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo) override;
    void PostInsightIntentExecuted(const sptr<AAFwk::SessionInfo> &sessionInfo,
        const AppExecFwk::InsightIntentExecuteResult &result, bool needForeground);
    std::unique_ptr<AppExecFwk::ETSNativeReference> CreateAppWindowStage(sptr<Rosen::Window> uiWindow,
        sptr<AAFwk::SessionInfo> sessionInfo);
    sptr<Rosen::Window> CreateUIWindow(const std::shared_ptr<UIExtensionContext> context,
        const sptr<AAFwk::SessionInfo> &sessionInfo, const AAFwk::Want &want);
    bool IsEmbeddableStart(int32_t screenMode);
    void ExecuteInsightIntentDone(uint64_t intentId, const InsightIntentExecuteResult &result);
    bool BindNativeMethods();

    ETSRuntime &etsRuntime_;
    std::shared_ptr<AppExecFwk::ETSNativeReference> etsObj_ = nullptr;
    std::shared_ptr<AppExecFwk::ETSNativeReference> shellContextRef_ = nullptr;
    std::mutex uiWindowMutex_;
    std::map<uint64_t, ContentSessionType> contentSessions_;
    int32_t screenMode_ = AAFwk::IDLE_SCREEN_MODE;
    std::shared_ptr<int32_t> screenModePtr_;
    sptr<IRemoteObject> token_ = nullptr;
    std::shared_ptr<AbilityHandler> handler_ = nullptr;
    std::shared_ptr<EtsAbilityResultListeners> abilityResultListeners_ = nullptr;
    ani_ref contentSession_ = nullptr;
    std::shared_ptr<EtsUIExtensionContentSession> etsUiExtContentSession_;

#ifdef SUPPORT_GRAPHICS
    class EtsUIExtensionAbilityDisplayListener : public OHOS::Rosen::IDisplayInfoChangedListener {
    public:
        explicit EtsUIExtensionAbilityDisplayListener(const std::weak_ptr<EtsUIExtension> &etsUIExtension)
        {
            etsUIExtension_ = etsUIExtension;
        }

        void OnDisplayInfoChange(const sptr<IRemoteObject> &token, Rosen::DisplayId displayId, float density,
            Rosen::DisplayOrientation orientation) override
        {
            auto sptr = etsUIExtension_.lock();
            if (sptr != nullptr) {
                sptr->OnDisplayInfoChange(token, displayId, density, orientation);
            }
        }

    private:
        std::weak_ptr<EtsUIExtension> etsUIExtension_;
    };

    void RegisterDisplayInfoChangedListener();
    void UnregisterDisplayInfoChangedListener();
    void OnDisplayInfoChange(const sptr<IRemoteObject> &token, Rosen::DisplayId displayId, float density,
        Rosen::DisplayOrientation orientation);

    sptr<EtsUIExtensionAbilityDisplayListener> etsUIExtensionAbilityDisplayListener_ = nullptr;
#endif
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ETS_UI_EXTENSION_H
