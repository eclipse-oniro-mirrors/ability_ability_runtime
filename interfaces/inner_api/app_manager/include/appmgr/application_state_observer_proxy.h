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

#ifndef OHOS_ABILITY_RUNTIME_APPLICATION_STATE_OBSERVER_PROXY_H
#define OHOS_ABILITY_RUNTIME_APPLICATION_STATE_OBSERVER_PROXY_H

#include "iremote_proxy.h"

#include "app_mgr_constants.h"
#include "iapplication_state_observer.h"

namespace OHOS {
namespace AppExecFwk {
class ApplicationStateObserverProxy : public IRemoteProxy<IApplicationStateObserver> {
public:
    explicit ApplicationStateObserverProxy(const sptr<IRemoteObject> &impl);
    virtual ~ApplicationStateObserverProxy() = default;

    /**
     * Application foreground state changed callback.
     *
     * @param appStateData Application Process data.
     */
    virtual void OnForegroundApplicationChanged(const AppStateData &appStateData) override;

    /**
     * Will be called when the ability state changes.
     *
     * @param abilityStateData Ability state data.
     */
    virtual void OnAbilityStateChanged(const AbilityStateData &abilityStateData) override;

    /**
     * Will be called when the extension state changes.
     *
     * @param abilityStateData Extension state data.
     */
    virtual void OnExtensionStateChanged(const AbilityStateData &abilityStateData) override;

    /**
     * Will be called when the process start.
     *
     * @param processData Process data.
     */
    virtual void OnProcessCreated(const ProcessData &processData) override;

    /**
     * Will be called when the process state change.
     *
     * @param processData Process data.
     */
    virtual void OnProcessStateChanged(const ProcessData &processData) override;

    /**
     * Will be called when the window show.
     *
     * @param processData Process data.
     */
    virtual void OnWindowShow(const ProcessData &processData) override;

    /**
     * Will be called when the window hidden.
     *
     * @param processData Process data.
     */
    virtual void OnWindowHidden(const ProcessData &processData) override;

    /**
     * Will be called when the process die.
     *
     * @param processData Process data.
     */
    virtual void OnProcessDied(const ProcessData &processData) override;

    /**
     * Application state changed callback.
     * Only observe APP_STATE_CREATE and APP_STATE_TERMINATED
     *
     * @param appStateData Application state data.
     */
    virtual void OnApplicationStateChanged(const AppStateData &appStateData) override;

    /**
     * Application state changed callback.
     * Only observe APP_STATE_FOREGROUND and APP_STATE_BACKGROUND
     *
     * @param appStateData Application state data.
     */
    virtual void OnAppStateChanged(const AppStateData &appStateData) override;

     /**
     * Called when one process is reused.
     *
     * @param processData Process data.
     */
    virtual void OnProcessReused(const ProcessData &processData) override;

    /**
     * Will be called when the application start.
     *
     * @param appStateData Application state data.
     */
    virtual void OnAppStarted(const AppStateData &appStateData) override;

    /**
     * Will be called when the application stop.
     *
     * @param appStateData Application state data.
     */
    virtual void OnAppStopped(const AppStateData &appStateData) override;

    /**
     * Will be called when page show.
     *
     * @param pageStateData Page state data.
     */
    virtual void OnPageShow(const PageStateData &pageStateData) override;

    /**
     * Will be called whe page hide.
     *
     * @param pageStateData Page state data.
     */
    virtual void OnPageHide(const PageStateData &pageStateData) override;

    /**
     * Will be called when application cache state change.
     *
     * @param appStateData Application state data.
     */
    virtual void OnAppCacheStateChanged(const AppStateData &appStateData) override;

    /**
     * Will be called when bindingRelation change.
     *
     * @param processBindData Process bind data.
     */
    virtual void OnProcessBindingRelationChanged(const ProcessBindData &processBindData) override;

    /**
     * Will be called when the process keepalive state change.
     *
     * @param processData Process data.
     */
    virtual void OnKeepAliveStateChanged(const ProcessData &processData) override;

    /**
     * Will be called when preload process state change.
     *
     * @param preloadProcessData Preload process data.
     */
    virtual void OnProcessPreForegroundChanged(const PreloadProcessData &preloadProcessData) override;

    /**
     * Will be called when process type to normal.
     *
     * @param processData Process data.
     */
    virtual void OnProcessTypeChanged(const ProcessData &processData) override;

    /**
     * Will be called when a game prelaunched to the foreground lifecycle is started to the
     * foreground display again, for example the user clicks the icon to launch the preloaded
     * game. The appStateData carries the GAME_PRELAUNCH preload mode, and this is notified
     * only once per game prelaunch.
     *
     * @param appStateData Application state data.
     */
    virtual void OnGamePrelaunchForegroundStateChanged(const AppStateData &appStateData) override;

private:
    bool WriteInterfaceToken(MessageParcel &data);
    static inline BrokerDelegator<ApplicationStateObserverProxy> delegator_;
    int32_t SendTransactCmd(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif
