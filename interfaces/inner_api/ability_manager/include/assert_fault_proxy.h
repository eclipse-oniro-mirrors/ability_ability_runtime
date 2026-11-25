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

#ifndef OHOS_ABILITY_RUNTIME_ASSERT_FAULT_PROXY_H
#define OHOS_ABILITY_RUNTIME_ASSERT_FAULT_PROXY_H

#include <atomic>
#include <chrono>
#include <mutex>
#include <queue>

#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_proxy.h"
#include "ability_connect_callback_stub.h"
#include "assert_fault_interface.h"
#include "singleton.h"

namespace OHOS {
namespace AbilityRuntime {
class AssertFaultProxy : public IRemoteProxy<IAssertFaultInterface> {
public:
    explicit AssertFaultProxy(const sptr<IRemoteObject> &impl);
    virtual ~AssertFaultProxy() = default;
    /**
     * Notify listeners of user operation results.
     *
     * @param status - User action result.
     */
    void NotifyDebugAssertResult(AAFwk::UserStatus status) override;

private:
    static inline BrokerDelegator<AssertFaultProxy> delegator_;
};

class AssertFaultRemoteDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    using RemoteDiedHandler = std::function<void(const wptr<IRemoteObject> &)>;
    explicit AssertFaultRemoteDeathRecipient(RemoteDiedHandler handler);
    virtual ~AssertFaultRemoteDeathRecipient() = default;
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;

private:
    RemoteDiedHandler handler_;
};

class ModalSystemAssertUIExtension {
public:
    static ModalSystemAssertUIExtension &GetInstance();
    ModalSystemAssertUIExtension() = default;
    virtual ~ModalSystemAssertUIExtension();

    bool CreateModalUIExtension(const AAFwk::Want &want);

    friend class AssertFaultProxy;

private:
    class AssertDialogConnection : public OHOS::AAFwk::AbilityConnectionStub {
    public:
        AssertDialogConnection() = default;
        virtual ~AssertDialogConnection() = default;

        void SetReqeustAssertDialogWant(const AAFwk::Want &want);
        void OnAbilityConnectDone(const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject,
            int resultCode) override;
        void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;

    private:
        AAFwk::Want want_;
        std::atomic<bool> notifyOnDisconnect_{ true };
    };

private:
    bool DisconnectSystemUI();
    void TryNotifyOneWaitingThread();
    void TryNotifyOneWaitingThreadInner();

    bool isBusy_ = false;
    sptr<AssertDialogConnection> GetConnection();
    sptr<AssertDialogConnection> dialogConnectionCallback_;
    std::mutex assertResultMutex_;
    std::mutex dialogConnectionMutex_;
    std::condition_variable assertResultCV_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ASSERT_FAULT_PROXY_H
