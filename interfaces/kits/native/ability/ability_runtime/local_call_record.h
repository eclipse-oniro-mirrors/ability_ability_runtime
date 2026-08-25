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

#ifndef OHOS_ABILITY_RUNTIME_LOCAL_CALL_RECORD_H
#define OHOS_ABILITY_RUNTIME_LOCAL_CALL_RECORD_H

#include <mutex>

#include "caller_callback.h"
#include "element_name.h"
#include "iremote_object.h"

namespace OHOS {
namespace AbilityRuntime {
/**
 * @class LocalCallRecord
 * LocalCallRecord record local call info.
 */
class LocalCallRecord : public std::enable_shared_from_this<LocalCallRecord> {
public:
    explicit LocalCallRecord(const AppExecFwk::ElementName &elementName);
    virtual ~LocalCallRecord();

    void ClearData();
    void SetRemoteObject(const sptr<IRemoteObject> &call, sptr<IRemoteObject::DeathRecipient> callRecipient);
    void AddCaller(const std::shared_ptr<CallerCallBack> &callback);
    bool RemoveCaller(const std::shared_ptr<CallerCallBack> &callback);
    void OnCallStubDied();
    void NotifyRemoteStateChanged(int32_t abilityState);
    sptr<IRemoteObject> GetRemoteObject() const;
    void InvokeCallBack() const;
    AppExecFwk::ElementName GetElementName() const;
    bool IsExistCallBack() const;
    int GetRecordId() const;
    std::vector<std::shared_ptr<CallerCallBack>> GetCallers() const;
    bool IsSameObject(const sptr<IRemoteObject> &remote) const;
    void SetIsSingleton(bool flag);
    bool IsSingletonRemote();
    void SetConnection(const sptr<IRemoteObject> &connect);
    sptr<IRemoteObject> GetConnection();
    void SetUserId(int32_t userId);
    int32_t GetUserId() const;
private:
    static int64_t callRecordId;
    int recordId_ = 0;
    int32_t userId_ = -1;
    sptr<IRemoteObject> remoteObject_ = nullptr;
    wptr<IRemoteObject> connection_ = nullptr;
    mutable std::mutex remoteMutex_;  // Protect remoteObject_ and callRecipient_ from concurrent access
    sptr<IRemoteObject::DeathRecipient> callRecipient_ = nullptr;
    mutable std::mutex callersMutex_;  // Protect callers_ from concurrent access
    std::vector<std::shared_ptr<CallerCallBack>> callers_;
    AppExecFwk::ElementName elementName_ = {};
    bool isSingleton_ = false;
};

/**
 * @class CallRecipient
 * CallRecipient notices IRemoteBroker died.
 */
class CallRecipient : public IRemoteObject::DeathRecipient {
public:
    using RemoteDiedHandler = std::function<void(const wptr<IRemoteObject> &)>;

    explicit CallRecipient(RemoteDiedHandler handler) : handler_(handler) {};
    virtual ~CallRecipient() = default;

    void OnRemoteDied(const wptr<IRemoteObject> &__attribute__((unused)) remote) override
    {
        if (handler_) {
            handler_(remote);
        }
    };

private:
    RemoteDiedHandler handler_ = nullptr;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_LOCAL_CALL_RECORD_H
