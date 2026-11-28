/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_IAPPLICATION_STATE_OBSERVER_H
#define OHOS_ABILITY_RUNTIME_IAPPLICATION_STATE_OBSERVER_H

#include "ability_state_data.h"
#include "app_state_data.h"
#include "page_state_data.h"
#include "preload_process_data.h"
#include "process_data.h"
#include "process_bind_data.h"
#include "iremote_broker.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
class IApplicationStateObserver : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.appexecfwk.IApplicationStateObserver");

    /**
     * Application foreground state changed callback.
     *
     * @param appStateData Application state data.
     */
    virtual void OnForegroundApplicationChanged(const AppStateData &appStateData) = 0;

    /**
     * Will be called when the ability state changes.
     *
     * @param abilityStateData Ability state data.
     */
    virtual void OnAbilityStateChanged(const AbilityStateData &abilityStateData) = 0;

    /**
     * Will be called when the extension state changes.
     *
     * @param abilityStateData Extension state data.
     */
    virtual void OnExtensionStateChanged(const AbilityStateData &abilityStateData) = 0;

    /**
     * Will be called when the process start.
     *
     * @param processData Process data.
     */
    virtual void OnProcessCreated(const ProcessData &processData) = 0;

    /**
     * Will be called when the process state change.
     *
     * @param processData Process data.
     */
    virtual void OnProcessStateChanged(const ProcessData &processData) {}

    /**
     * Will be called when the window show.
     *
     * @param processData Process data.
     */
    virtual void OnWindowShow(const ProcessData &processData) {}

    /**
     * Will be called when the window hidden.
     *
     * @param processData Process data.
     */
    virtual void OnWindowHidden(const ProcessData &processData) {}

    /**
     * Will be called when the process die.
     *
     * @param processData Process data.
     */
    virtual void OnProcessDied(const ProcessData &processData) = 0;

    /**
     * Application state changed callback.
     * Only observe APP_STATE_CREATE and APP_STATE_TERMINATED
     *
     * @param appStateData Application state data.
     */
    virtual void OnApplicationStateChanged(const AppStateData &appStateData) = 0;

    /**
     * Application state changed callback.
     * Only observe APP_STATE_FOREGROUND and APP_STATE_BACKGROUND
     *
     * @param appStateData Application state data.
     */
    virtual void OnAppStateChanged(const AppStateData &appStateData) {}

    /**
     * Called when one process is reused.
     *
     * @param processData Process data.
     */
    virtual void OnProcessReused(const ProcessData &processData) {}

    /**
     * Will be called when the application start.
     *
     * @param appStateData Application state data.
     */
    virtual void OnAppStarted(const AppStateData &appStateData) {};

    /**
     * Will be called when the application stop.
     *
     * @param appStateData Application state data.
     */
    virtual void OnAppStopped(const AppStateData &appStateData) {};

    /**
     * Will be called when page show.
     *
     * @param pageStateData Page state data.
     */
    virtual void OnPageShow(const PageStateData &pageStateData) {};

    /**
     * Will be called whe page hide.
     *
     * @param pageStateData Page state data.
     */
    virtual void OnPageHide(const PageStateData &pageStateData) {};

    /**
     * Will be called when application cache state change.
     *
     * @param appStateData Application state data.
     */
    virtual void OnAppCacheStateChanged(const AppStateData &appStateData) {};

    /**
     * Will be called when bindingRelation change.
     *
     * @param processBindData Process bind data.
     */
    virtual void OnProcessBindingRelationChanged(const ProcessBindData &processBindData) {};

    /**
     * Will be called when the process keepalive state change.
     *
     * @param processData Process data.
     */
    virtual void OnKeepAliveStateChanged(const ProcessData &processData) {};
    
    /**
     * Will be called when preload process state change.
     *
     * @param preloadProcessData Preload process data.
     */
    virtual void OnProcessPreForegroundChanged(const PreloadProcessData &preloadProcessData) {};

    /**
     * Will be called when process type to normal.
     *
     * @param processData Process data.
     */
    virtual void OnProcessTypeChanged(const ProcessData &processData) {};

    /**
     * Will be called when a game prelaunched to the foreground lifecycle is started to the
     * foreground display again, for example the user clicks the icon to launch the preloaded
     * game. The appStateData carries the GAME_PRELAUNCH preload mode, and this is notified
     * only once per game prelaunch.
     *
     * @param appStateData Application state data.
     */
    virtual void OnGamePrelaunchForegroundStateChanged(const AppStateData &appStateData) {};

    enum class Message {
        TRANSACT_ON_FOREGROUND_APPLICATION_CHANGED = 0,
        TRANSACT_ON_ABILITY_STATE_CHANGED,
        TRANSACT_ON_EXTENSION_STATE_CHANGED,
        TRANSACT_ON_PROCESS_CREATED,
        TRANSACT_ON_PROCESS_STATE_CHANGED,
        TRANSACT_ON_PROCESS_DIED,
        TRANSACT_ON_APPLICATION_STATE_CHANGED,
        TRANSACT_ON_APP_STATE_CHANGED,
        TRANSACT_ON_PROCESS_REUSED,
        TRANSACT_ON_APP_STARTED,
        TRANSACT_ON_APP_STOPPED,
        TRANSACT_ON_PAGE_SHOW,
        TRANSACT_ON_PAGE_HIDE,
        TRANSACT_ON_APP_CACHE_STATE_CHANGED,
        TRANSACT_ON_WINDOW_SHOW,
        TRANSACT_ON_WINDOW_HIDDEN,
        TRANSACT_ON_PROCESS_BINDINGRELATION_CHANGED,
        TRANSACT_ON_KEEP_ALIVE_STATE_CHANGED,
        TRANSACT_ON_PRELOAD_PROCESS_STATE_CHANGED,
        TRANSACT_ON_PROCESS_TYPE_CHANGED,
        TRANSACT_ON_GAME_PRELAUNCH_FOREGROUND_STATE_CHANGED,
    };
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_IAPPLICATION_STATE_OBSERVER_H
