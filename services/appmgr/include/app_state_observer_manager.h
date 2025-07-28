/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_APP_STATE_OBSERVER_MANAGER_H
#define OHOS_ABILITY_RUNTIME_APP_STATE_OBSERVER_MANAGER_H

#include <map>
#include <mutex>
#include <string>
#include <unordered_map>

#include "ability_foreground_state_observer_interface.h"
#include "app_foreground_state_observer_interface.h"
#include "app_running_record.h"
#include "app_state_data.h"
#include "cpp/mutex.h"
#include "iapp_state_callback.h"
#include "iapplication_state_observer.h"
#include "page_state_data.h"
#include "permission_constants.h"
#include "permission_verification.h"
#include "singleton.h"
#include "task_handler_wrap.h"
#ifdef SUPPORT_UPMS
#include "uri_permission_manager_client.h"
#endif // SUPPORT_UPMS

namespace OHOS {
namespace AppExecFwk {
struct AppStateObserverInfo {
    int32_t uid = 0;
    std::vector<std::string> bundleNames;
};

using AppStateObserverMap = std::map<sptr<IApplicationStateObserver>, AppStateObserverInfo>;
using AppForegroundStateObserverMap = std::map<sptr<IAppForegroundStateObserver>, int32_t>;
using AbilityForegroundObserverMap = std::map<sptr<IAbilityForegroundStateObserver>, int32_t>;

enum class ObserverType {
    APPLICATION_STATE_OBSERVER,
    APP_FOREGROUND_STATE_OBSERVER,
    ABILITY_FOREGROUND_STATE_OBSERVER,
};
class AppStateObserverManager : public std::enable_shared_from_this<AppStateObserverManager> {
    DECLARE_DELAYED_SINGLETON(AppStateObserverManager)
public:
    void Init();
    int32_t RegisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer,
        const std::vector<std::string> &bundleNameList = {});
    int32_t UnregisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer);
    int32_t RegisterAppForegroundStateObserver(const sptr<IAppForegroundStateObserver> &observer);
    int32_t UnregisterAppForegroundStateObserver(const sptr<IAppForegroundStateObserver> &observer);
    int32_t RegisterAbilityForegroundStateObserver(const sptr<IAbilityForegroundStateObserver> &observer);
    int32_t UnregisterAbilityForegroundStateObserver(const sptr<IAbilityForegroundStateObserver> &observer);
    void StateChangedNotifyObserver(
        const AbilityStateData abilityStateData, bool isAbility, bool isFromWindowFocusChanged);
    void OnAppStateChanged(const std::shared_ptr<AppRunningRecord> &appRecord, const ApplicationState state,
        bool needNotifyApp, bool isFromWindowFocusChanged);
    void OnAppStarted(const std::shared_ptr<AppRunningRecord> &appRecord);
    void OnAppStopped(const std::shared_ptr<AppRunningRecord> &appRecord);
    void OnProcessCreated(const std::shared_ptr<AppRunningRecord> &appRecord, bool isPreload);
    void OnProcessStateChanged(
        const std::shared_ptr<AppRunningRecord> &appRecord, bool isFromWindowFocusChanged = false);
    void OnProcessBindingRelationChanged(const std::shared_ptr<AppRunningRecord> &appRecord,
                                         const UIExtensionProcessBindInfo &bindInfo, int32_t bindingRelation);
    void OnWindowShow(const std::shared_ptr<AppRunningRecord> &appRecord);
    void OnWindowHidden(const std::shared_ptr<AppRunningRecord> &appRecord);
    void OnRenderProcessCreated(const std::shared_ptr<RenderRecord> &RenderRecord, const bool isPreload);
#ifdef SUPPORT_CHILD_PROCESS
    void OnChildProcessCreated(std::shared_ptr<ChildProcessRecord> childRecord);
#endif // SUPPORT_CHILD_PROCESS
    void OnProcessDied(const std::shared_ptr<AppRunningRecord> &appRecord);
    void OnRenderProcessDied(const std::shared_ptr<RenderRecord> &renderRecord);
#ifdef SUPPORT_CHILD_PROCESS
    void OnChildProcessDied(std::shared_ptr<ChildProcessRecord> childRecord);
#endif // SUPPORT_CHILD_PROCESS
    void OnProcessReused(const std::shared_ptr<AppRunningRecord> &appRecord);
    void OnPageShow(const PageStateData pageStateData);
    void OnPageHide(const PageStateData pageStateData);
    void OnAppCacheStateChanged(const std::shared_ptr<AppRunningRecord> &appRecord, ApplicationState state);
    void OnKeepAliveStateChanged(const std::shared_ptr<AppRunningRecord> &appRecord);
    void OnProcessPreForegroundChanged(std::shared_ptr<AppRunningRecord> appRecord);

private:
    void HandleOnWindowShow(const std::shared_ptr<AppRunningRecord> &appRecord);
    void HandleOnWindowHidden(const std::shared_ptr<AppRunningRecord> &appRecord);
    void HandleAppStateChanged(const std::shared_ptr<AppRunningRecord> &appRecord, const ApplicationState state,
        bool needNotifyApp, bool isFromWindowFocusChanged);
    void HandleOnAppStarted(const std::shared_ptr<AppRunningRecord> &appRecord);
    void HandleOnAppStopped(const std::shared_ptr<AppRunningRecord> &appRecord);
    void HandleStateChangedNotifyObserver(
        const AbilityStateData abilityStateData, bool isAbility, bool isFromWindowFocusChanged);
    void HandleOnAppProcessCreated(const std::shared_ptr<AppRunningRecord> &appRecord, bool isPreload);
    void HandleOnRenderProcessCreated(const std::shared_ptr<RenderRecord> &RenderRecord, const bool isPreload);
#ifdef SUPPORT_CHILD_PROCESS
    void HandleOnChildProcessCreated(std::shared_ptr<ChildProcessRecord> childRecord);
#endif // SUPPORT_CHILD_PROCESS
    void HandleOnAppProcessDied(const std::shared_ptr<AppRunningRecord> &appRecord);
    void HandleOnRenderProcessDied(const std::shared_ptr<RenderRecord> &RenderRecord);
#ifdef SUPPORT_CHILD_PROCESS
    void HandleOnChildProcessDied(std::shared_ptr<ChildProcessRecord> childRecord);
#endif // SUPPORT_CHILD_PROCESS
    bool ObserverExist(const sptr<IRemoteBroker> &observer);
    bool IsAppForegroundObserverExist(const sptr<IRemoteBroker> &observer);
    bool IsAbilityForegroundObserverExist(const sptr<IRemoteBroker> &observer);
    void AddObserverDeathRecipient(const sptr<IRemoteBroker> &observer, const ObserverType &type);
    void RemoveObserverDeathRecipient(const sptr<IRemoteBroker> &observer);
    AppStateObserverMap GetAppStateObserverMapCopy();
    AppForegroundStateObserverMap GetAppForegroundStateObserverMapCopy();
    AbilityForegroundObserverMap GetAbilityForegroundObserverMapCopy();
    ProcessData WrapProcessData(
        const std::shared_ptr<AppRunningRecord> &appRecord, bool isFromWindowFocusChanged = false);
    ProcessData WrapRenderProcessData(const std::shared_ptr<RenderRecord> &renderRecord);
    ProcessBindData WrapProcessBindData(const UIExtensionProcessBindInfo &bindInfo, int32_t bindingRelation);
#ifdef SUPPORT_CHILD_PROCESS
    int32_t WrapChildProcessData(ProcessData &processData, std::shared_ptr<ChildProcessRecord> childRecord);
#endif // SUPPORT_CHILD_PROCESS
    void OnObserverDied(const wptr<IRemoteObject> &remote, const ObserverType &type);
    AppStateData WrapAppStateData(const std::shared_ptr<AppRunningRecord> &appRecord,
        const ApplicationState state, bool isFromWindowFocusChanged = false);
    void HandleOnProcessCreated(const ProcessData &data);
    void HandleOnProcessStateChanged(
        const std::shared_ptr<AppRunningRecord> &appRecordm, bool isFromWindowFocusChanged = false);
    void HandleOnProcessDied(const ProcessData &data);
    void HandleOnProcessResued(const std::shared_ptr<AppRunningRecord> &appRecord);
    void HandleOnPageShow(const PageStateData pageStateData);
    void HandleOnPageHide(const PageStateData pageStateData);
    void HandleOnAppCacheStateChanged(const std::shared_ptr<AppRunningRecord> &appRecord, ApplicationState state);
    void AddObserverCount(int32_t uid);
    void DecreaseObserverCount(int32_t uid);
    void HandleOnProcessBindingRelationChanged(const std::shared_ptr<AppRunningRecord> &appRecord,
        const UIExtensionProcessBindInfo &bindInfo, int32_t bindingRelation);
    void HandleOnKeepAliveStateChanged(const std::shared_ptr<AppRunningRecord> &appRecord);
    void HandleOnProcessPreForegroundChanged(std::shared_ptr<AppRunningRecord> appRecord);

private:
    std::shared_ptr<AAFwk::TaskHandlerWrap> handler_;
    AppStateObserverMap appStateObserverMap_;
    AppForegroundStateObserverMap appForegroundStateObserverMap_;
    AbilityForegroundObserverMap abilityForegroundObserverMap_;
    std::map<sptr<IRemoteObject>, sptr<IRemoteObject::DeathRecipient>> recipientMap_;
    std::unordered_map<int32_t, int32_t> observerCountMap_;  // <uid, count>
    int32_t observerAmount_ = 0;
    ffrt::mutex observerLock_;
    ffrt::mutex appForegroundObserverLock_;
    ffrt::mutex abilityForegroundObserverLock_;
    ffrt::mutex recipientMapMutex_;
    ffrt::mutex observerCountMapMutex_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APP_STATE_OBSERVER_MANAGER_H
