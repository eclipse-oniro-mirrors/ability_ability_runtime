/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_QUERY_ERMS_OBSERVER_MANAGER_H
#define OHOS_ABILITY_RUNTIME_QUERY_ERMS_OBSERVER_MANAGER_H

#include <map>
#include <mutex>
#include <unordered_map>
#include "cpp/mutex.h"

#include "query_erms_observer_interface.h"
#include "singleton.h"

namespace OHOS {
namespace AAFwk {
using namespace OHOS::AbilityRuntime;
class QueryERMSObserverManager {
public:
    /**
     * GetInstance, get the singleton instance of QueryERMSObserverManager.
     *
     * @return Returns the singleton instance.
     */
    static QueryERMSObserverManager &GetInstance();

    /**
     * AddObserver, register an ERMS query observer for the given record id.
     *
     * @param recordId, the record id of the ability.
     * @param observer, the ERMS query observer to register.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t AddObserver(int32_t recordId, const sptr<IQueryERMSObserver> &observer);

    /**
     * OnQueryFinished, post an async task to dispatch the query result to the observer.
     *
     * @param recordId, the record id of the ability.
     * @param appId, appId.
     * @param startTime, startTime.
     * @param rule, atomic service startup rule.
     * @param resultCode, ERR_OK on success, others on failure.
     */
    void OnQueryFinished(int32_t recordId, const std::string &appId, const std::string &startTime,
        const AtomicServiceStartupRule &rule, int resultCode);

    /**
     * OnObserverDied, handle the observer death event and remove it from the observer map.
     *
     * @param remote, the weak reference of the dead remote object.
     */
    void OnObserverDied(const wptr<IRemoteObject> &remote);

    /**
     * HandleOnQueryFinished, invoke the observer callback with the query result.
     *
     * @param recordId, the record id of the ability.
     * @param appId, appId.
     * @param startTime, startTime.
     * @param rule, atomic service startup rule.
     * @param resultCode, ERR_OK on success, others on failure.
     */
    void HandleOnQueryFinished(int32_t recordId, const std::string &appId, const std::string &startTime,
        const AtomicServiceStartupRule &rule, int resultCode);

private:
    /**
     * Default constructor.
     */
    QueryERMSObserverManager();
    /**
     * Default destructor.
     */
    ~QueryERMSObserverManager();

private:
    ffrt::mutex observerLock_;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_;
    std::unordered_map<int32_t, sptr<IQueryERMSObserver>> observerMap_;
    DISALLOW_COPY_AND_MOVE(QueryERMSObserverManager);
};

class QueryERMSObserverRecipient : public IRemoteObject::DeathRecipient {
public:
    using RemoteDiedHandler = std::function<void(const wptr<IRemoteObject> &)>;
    /**
     * Constructor with the remote-died handler.
     *
     * @param handler, the callback invoked when the remote object dies.
     */
    explicit QueryERMSObserverRecipient(RemoteDiedHandler handler);
    /**
     * Default destructor.
     */
    virtual ~QueryERMSObserverRecipient();
    /**
     * OnRemoteDied, called when the observed remote object dies.
     *
     * @param remote, the weak reference of the dead remote object.
     */
    virtual void OnRemoteDied(const wptr<IRemoteObject> &remote);

private:
    RemoteDiedHandler handler_;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_QUERY_ERMS_OBSERVER_MANAGER_H