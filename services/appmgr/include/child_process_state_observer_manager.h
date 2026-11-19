/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_CHILD_PROCESS_STATE_OBSERVER_MANAGER_H
#define OHOS_ABILITY_RUNTIME_CHILD_PROCESS_STATE_OBSERVER_MANAGER_H

#include <vector>
#include "app_running_record.h"
#include "ichild_process_state_observer.h"
#include "singleton.h"
#include "task_handler_wrap.h"

namespace OHOS {
namespace AppExecFwk {
class ChildProcessStateObserverManager : public std::enable_shared_from_this<ChildProcessStateObserverManager> {
    DECLARE_DELAYED_SINGLETON(ChildProcessStateObserverManager);
public:
    void Init();
    int32_t RegisterChildProcessStateObserver(const sptr<IChildProcessStateObserver> &observer);
    int32_t UnregisterChildProcessStateObserver(const sptr<IChildProcessStateObserver> &observer);
    int32_t OnChildProcessStateChanged(int32_t pid, int32_t uid, int32_t callingPid, int32_t callingUid, int32_t state);
private:
    void OnObserverDied(const wptr<IRemoteObject> &remote);
    void HandleObserverDied(const sptr<IRemoteObject> &observerObj);
    void HandleRegisterChildProcessStateObserver(const sptr<IChildProcessStateObserver> &observer);
    void HandleUnregisterChildProcessStateObserver(const sptr<IChildProcessStateObserver> &observer);
    void HandleOnChildProcessStateChanged(int32_t pid, int32_t uid,
        int32_t callingPid, int32_t callingUid, int32_t state);
    bool IsObserverRegistered(const sptr<IRemoteObject> &observerObj) const;
    bool RemoveObserverFromList(const sptr<IRemoteObject> &observerObj);
    ChildProcessStateData WrapChildProcessStateData(int32_t pid, int32_t uid,
        int32_t callingPid, int32_t callingUid, int32_t state);

    sptr<IRemoteObject::DeathRecipient> deathRecipient_;
    std::vector<sptr<IChildProcessStateObserver>> observerList_;
    std::shared_ptr<AAFwk::TaskHandlerWrap> handler_;
};

class ChildProcessStateObserverRecipient : public IRemoteObject::DeathRecipient {
public:
    using RemoteDiedHandler = std::function<void(const wptr<IRemoteObject> &)>;
    explicit ChildProcessStateObserverRecipient(RemoteDiedHandler handler);
    virtual ~ChildProcessStateObserverRecipient();
    virtual void OnRemoteDied(const wptr<IRemoteObject> &remote);
private:
    RemoteDiedHandler handler_;
};
} // namespace AppExecFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CHILD_PROCESS_STATE_OBSERVER_MANAGER_H
