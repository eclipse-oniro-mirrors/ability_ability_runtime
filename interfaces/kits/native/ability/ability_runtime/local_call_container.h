/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_LOCAL_CALL_CONTAINER_H
#define OHOS_ABILITY_RUNTIME_LOCAL_CALL_CONTAINER_H

#include <mutex>

#include "ability_context.h"
#include "ability_connect_callback_stub.h"
#include "ability_connect_callback_proxy.h"
#include "local_call_record.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
using Want = OHOS::AAFwk::Want;
using AbilityConnectionStub = OHOS::AAFwk::AbilityConnectionStub;
class CallerConnection;
class LocalCallContainer : public std::enable_shared_from_this<LocalCallContainer> {
public:
    LocalCallContainer() = default;
    virtual ~LocalCallContainer() = default;

    int StartAbilityByCallInner(const Want &want, std::shared_ptr<CallerCallBack> callback,
        sptr<IRemoteObject> callerToken, int32_t accountId = DEFAULT_INVAL_VALUE);

    int ReleaseCall(const std::shared_ptr<CallerCallBack> &callback);

    void ClearFailedCallConnection(const std::shared_ptr<CallerCallBack> &callback);

    void DumpCalls(std::vector<std::string> &info);

    void SetCallLocalRecord(std::shared_ptr<LocalCallRecord> localCallRecord);
    void SetMultipleCallLocalRecord(std::shared_ptr<LocalCallRecord> localCallRecord);

    void OnCallStubDied(const wptr<IRemoteObject> &remote);

    // Methods to remove call records, made public for CallerConnection
    int32_t RemoveSingletonCallLocalRecord(const std::shared_ptr<LocalCallRecord> &record);
    int32_t RemoveMultipleCallLocalRecord(const std::shared_ptr<LocalCallRecord> &record);

    /**
     * @brief Remove caller connection from container
     * @param connection The connection to remove
     */
    void RemoveCallerConnection(sptr<CallerConnection> connection);

private:
    bool GetCallLocalRecord(const AppExecFwk::ElementName &elementName,
        std::shared_ptr<LocalCallRecord> &localCallRecord, int32_t accountId);
    void OnSingletonCallStubDied(const wptr<IRemoteObject> &remote);
    void OnMultipleCallStubDied(const wptr<IRemoteObject> &remote);
    int32_t GetCurrentUserId();
    int32_t GetValidUserId(int32_t accountId);
    bool IsCallBackCalled(const std::vector<std::shared_ptr<CallerCallBack>> &callers) const;

private:
    int32_t currentUserId_ = DEFAULT_INVAL_VALUE;
    // used to store single instance call records
    std::map<std::string, std::set<std::shared_ptr<LocalCallRecord>>> callProxyRecords_;
    // used to store multi instance call records
    std::map<std::string, std::set<std::shared_ptr<LocalCallRecord>>> multipleCallProxyRecords_;
    std::mutex connectionsMutex_;
    std::set<sptr<CallerConnection>> connections_;
    std::mutex mutex_;
    std::mutex multipleMutex_;
};

class CallerConnection : public AbilityConnectionStub {
public:
    CallerConnection() = default;
    virtual ~CallerConnection() = default;

    void ClearCallRecord();

    void SetRecordAndContainer(const std::shared_ptr<LocalCallRecord> &localCallRecord,
        const std::weak_ptr<LocalCallContainer> &container);

    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int code) override;

    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int code) override;

    void OnRemoteStateChanged(const AppExecFwk::ElementName &element, int32_t abilityState) override;
private:
    std::shared_ptr<LocalCallRecord> GetLocalCallRecord();
    std::shared_ptr<LocalCallContainer> GetContainer();
    std::shared_ptr<LocalCallRecord> localCallRecord_;
    std::weak_ptr<LocalCallContainer> container_;
    std::mutex mutex_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_LOCAL_CALL_CONTAINER_H
