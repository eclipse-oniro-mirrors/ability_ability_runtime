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

#ifndef OHOS_ABILITY_RUNTIME_QUERY_ERMS_MANAGER_H
#define OHOS_ABILITY_RUNTIME_QUERY_ERMS_MANAGER_H

#include <future>
#include "cpp/mutex.h"

#include <iremote_object.h>
#include <iremote_stub.h>
#include <memory>

#include "query_erms_observer_manager.h"
#include "singleton.h"

namespace OHOS {
namespace AAFwk {
class AbilityManagerService;

/**
 * @class QueryERMSManager
 * QueryERMSManager.
 */
class QueryERMSManager {
public:
    /**
     * GetInstance, get the singleton instance of QueryERMSManager.
     *
     * @return Returns the singleton instance.
     */
    static QueryERMSManager &GetInstance();

    /**
     * OnQueryFinished, StartQueryERMS is complete.
     *
     * @param appId, appId.
     * @param startTime, startTime.
     * @param rule, atomic service startup rule.
     * @param resultCode, ERR_OK on success, others on failure.
     */
    void OnQueryFinished(int32_t recordId, const std::string &appId, const std::string &startTime,
        const AtomicServiceStartupRule &rule, int32_t resultCode);

    /**
     * Add an observer from application into queryERMSObserverManager.
     * @param observer, the observer of the ability to free install.
     * @return Returns ERR_OK on success, others on failure.
     */
    int AddQueryERMSObserver(sptr<IRemoteObject> callerToken, sptr<AbilityRuntime::IQueryERMSObserver> observer);

private:
    /**
     * Default constructor.
     */
    QueryERMSManager();
    /**
     * Default destructor.
     */
    ~QueryERMSManager();

    /**
     * HandleQueryERMSResult, dispatch the query result to success or fail handler.
     *
     * @param recordId, the record id of the ability.
     * @param appId, appId.
     * @param startTime, startTime.
     * @param rule, atomic service startup rule.
     * @param resultCode, ERR_OK on success, others on failure.
     */
    void HandleQueryERMSResult(int32_t recordId, const std::string &appId, const std::string &startTime,
        const AtomicServiceStartupRule &rule, int32_t resultCode);
    /**
     * HandleOnQueryERMSSuccess, handle the query success and notify observers.
     *
     * @param recordId, the record id of the ability.
     * @param appId, appId.
     * @param startTime, startTime.
     * @param rule, atomic service startup rule.
     */
    void HandleOnQueryERMSSuccess(int32_t recordId, const std::string &appId, const std::string &startTime,
        const AtomicServiceStartupRule &rule);
    /**
     * HandleOnQueryERMSFail, handle the query failure and notify observers.
     *
     * @param recordId, the record id of the ability.
     * @param appId, appId.
     * @param startTime, startTime.
     * @param rule, atomic service startup rule.
     * @param resultCode, ERR_OK on success, others on failure.
     */
    void HandleOnQueryERMSFail(int32_t recordId, const std::string &appId, const std::string &startTime,
        const AtomicServiceStartupRule &rule, int resultCode);

    DISALLOW_COPY_AND_MOVE(QueryERMSManager);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_QUERY_ERMS_MANAGER_H
