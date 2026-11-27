/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_JS_AUTO_FILL_EXTENSION_H
#define OHOS_ABILITY_RUNTIME_JS_AUTO_FILL_EXTENSION_H

#include "auto_fill_extension.h"
#include "auto_fill_extension_context.h"
#include "configuration.h"
#include "session_info.h"
#include "view_data.h"

namespace OHOS {
namespace AbilityRuntime {
class AutoFillExtension;
class JsRuntime;
class JsUIExtensionBase;
/**
 * @brief Basic js auto fill extension.
 */
class JsAutoFillExtension : public AutoFillExtension, public IAutoFillExtensionCallback {
public:
    explicit JsAutoFillExtension(JsRuntime &jsRuntime);
    virtual ~JsAutoFillExtension() override;

    static JsAutoFillExtension *Create(const std::unique_ptr<Runtime> &runtime);

    /**
     * @brief Init the auto fill extension.
     *
     * @param record the auto fill extension record.
     * @param application the application info.
     * @param handler the auto fill extension handler.
     * @param token the remote token.
     */
    void Init(const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &record,
        const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
        std::shared_ptr<AppExecFwk::AbilityHandler> &handler, const sptr<IRemoteObject> &token) override;

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
     * @brief Called back when ui extension is started.
     *
     * This method can be called only by ui extension. You can use the StartAbility(Want) method to start
     * ui extension. Then the system calls back the current method to use the transferred want parameter to
     * execute its own logic.
     *
     * @param want Indicates the want of ui extension to start.
     * @param restart Indicates the startup mode. The value true indicates that ui extension is restarted after being
     * destroyed, and the value false indicates a normal startup.
     * @param startId Indicates the number of times the ui extension has been started. The startId is incremented
     * by 1 every time the ui extension is started. For example, if the ui extension has been started for six times, the
     * value of startId is 6.
     */
    void OnCommand(const AAFwk::Want &want, bool restart, int startId) override;

    void OnCommandWindow(
        const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd) override;

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
    void OnStopCallBack() override;

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
     * @brief Used to create an update request.
     *
     * @param wantParams Indicates the view data of the update request.
     */
    void UpdateRequest(const AAFwk::WantParams &wantParams);

    int32_t OnReloadInModal(const sptr<AAFwk::SessionInfo> &sessionInfo, const CustomData &customData) override;

private:
    virtual void BindContext(napi_env env, napi_value obj);
    napi_value CallObjectMethod(const char *name, napi_value const *argv = nullptr, size_t argc = 0,
        bool withResult = false);
    bool CheckPromise(napi_value result);
    bool CallPromise(napi_value result, AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo);
    void ForegroundWindow(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo);
    void BackgroundWindow(const sptr<AAFwk::SessionInfo> &sessionInfo);
    void DestroyWindow(const sptr<AAFwk::SessionInfo> &sessionInfo);

    void OnCommandWindowDone(const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd) override;
    bool HandleAutoFillCreate(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo);
    void RefreshAutoFillSession(const sptr<IRemoteObject> &obj, const sptr<AAFwk::SessionInfo> &info);
    sptr<Rosen::WindowOption> CreateWindowOption(const sptr<AAFwk::SessionInfo> &sessionInfo,
        const AAFwk::Want &want);
    void CallJsOnRequest(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo,
        const sptr<Rosen::Window> &uiWindow);
    void RegisterTransferComponentDataListener(const sptr<Rosen::Window> &uiWindow);

    JsRuntime& jsRuntime_;
    std::shared_ptr<NativeReference> jsObj_;
    std::shared_ptr<NativeReference> shellContextRef_ = nullptr;
    std::map<sptr<IRemoteObject>, sptr<Rosen::Window>> uiWindowMap_;
    std::set<sptr<IRemoteObject>> foregroundWindows_;
    std::map<sptr<IRemoteObject>, std::shared_ptr<NativeReference>> contentSessions_;
    std::map<sptr<IRemoteObject>, std::shared_ptr<NativeReference>> callbacks_;
    bool isPopup_ = false;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_AUTO_FILL_EXTENSION_H
