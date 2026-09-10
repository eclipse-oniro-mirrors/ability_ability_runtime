/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_ABILITY_RUNTIME_CLI_AGENT_CONNECTION_H
#define OHOS_ABILITY_RUNTIME_CLI_AGENT_CONNECTION_H

#include <atomic>
#include <cstdio>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <string>

#include "ability_connect_callback_stub.h"  // OHOS::AAFwk::AbilityConnectionStub (IPC stub base for IAbilityConnection)
#include "agent_connector_stub.h"           // OHOS::AgentRuntime::AgentConnectorStub (host-side IAgentConnector stub)
#include "element_name.h"
#include "agent_receiver_proxy.h"            // AgentReceiverProxy -> IAgentReceiver (via BrokerDelegator)

namespace OHOS {
namespace AAFwk {

// Maximum wall-clock wait for the async OnAbilityConnectDone callback; matches the AbilityMgr
// connect timeout (1000ms * 10), the same path JS/ETS uses.
constexpr int32_t CONNECT_WAIT_TIMEOUT_MS = 10 * 1000;

// OnAbilityConnectDone reported success but no usable IAgentReceiver proxy was delivered
// (CR-1). Distinct from -1 (WaitForConnect timeout); treated as connect failure so the caller
// exits instead of idling on a receiver-less connection.
constexpr int32_t CONNECT_RESULT_PROXY_NULL = -2;

// Thread-safe JSON-line emission to stdout (the cliMgr session stream consumed by the host
// via SubscribeSession). Appends a '\n' frame delimiter and flushes.
void EmitAgentEvent(const std::string &eventJsonLine);
// Lock stdout + flush, called just before _exit() so no in-flight binder-thread EmitAgentEvent
// (OnData/OnAuthorize push) is truncated by process exit (TO-3).
void DrainStdoutForExit();

// Escape an arbitrary string for safe inclusion as a JSON string value (per RFC 8259).
std::string EscapeJsonString(const std::string &s);

// JSON-line event builders (return a single-line JSON string WITHOUT the trailing newline).
std::string BuildConnectDoneEvent(bool ok, const std::string &agentId, int32_t errCode, const std::string &errMsg);
std::string BuildDisconnectDoneEvent(const std::string &reason);
std::string BuildOnDataEvent(const std::string &data);
std::string BuildOnAuthorizeEvent(const std::string &data);
std::string BuildErrorEvent(const std::string &errCode, const std::string &errMsg, bool fatal);

// Host-side IAgentConnector stub: RECEIVES agent->host data pushes (the agent holds a proxy
// delivered via Want AGENTEXTENSIONHOSTPROXY_KEY and calls the single-param SendData/Authorize).
// Each push is emitted to stdout as an OnData/OnAuthorize JSON-line.
class CliAgentConnectorStub : public AgentRuntime::AgentConnectorStub {
public:
    CliAgentConnectorStub() = default;
    ~CliAgentConnectorStub() override = default;

    int32_t SendData(const std::string &data) override;
    int32_t Authorize(const std::string &data) override;
};

// CLI's IAbilityConnection stub: agentmgr calls back OnAbilityConnectDone (delivering the
// agent's IAgentReceiver IRemoteObject) and OnAbilityDisconnectDone.
class CliAgentConnection : public AbilityConnectionStub {
public:
    CliAgentConnection();
    ~CliAgentConnection() override;

    // Set the host stub before connect; its AsObject() is both embedded in the Want under
    // AGENTEXTENSIONHOSTPROXY_KEY and passed as the connectorProxy arg on host->agent SendData.
    void SetHostStub(const sptr<CliAgentConnectorStub> &stub)
    {
        std::lock_guard<std::mutex> lock(mtx_);
        hostStub_ = stub;
    }
    sptr<IRemoteObject> GetHostStubObject() const
    {
        std::lock_guard<std::mutex> lock(mtx_);
        return hostStub_ != nullptr ? hostStub_->AsObject() : nullptr;
    }
    void SetAgentId(const std::string &agentId) { agentId_ = agentId; }

    // IAbilityConnection overrides (dispatched by AbilityConnectionStub::OnRemoteRequest).
    void OnAbilityConnectDone(const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject, int resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;

    // Block until OnAbilityConnectDone fires (or timeout). Returns the resultCode; on success
    // fills outProxy (host->agent IAgentReceiver) + outHostObj (the connectorProxy to pass).
    int32_t WaitForConnect(int32_t timeoutMs, sptr<AgentRuntime::IAgentReceiver> &outProxy,
        sptr<IRemoteObject> &outHostObj);

    // Disconnect wakeup channel: a self-pipe read fd the command loop polls alongside stdin so a
    // peer disconnect (OnAbilityDisconnectDone on a binder thread) can unblock an idle stdin read.
    int GetWakeupFd() const { return wakeupPipe_[0]; }
    // Whether the self-pipe wakeup channel was created (pipe2 may fail under fd exhaustion).
    bool HasWakeup() const { return wakeupPipe_[0] >= 0; }
    // Set the disconnect-done reason ("user" for host-initiated, "peer"/"error" otherwise).
    // Guarded by mtx_: OnAbilityDisconnectDone reads it under the same lock (DR-1).
    void SetDisconnectReason(const std::string &reason)
    {
        std::lock_guard<std::mutex> lock(mtx_);
        disconnectReason_ = reason;
    }
    // Called from OnAbilityDisconnectDone to wake the command loop.
    void SignalDisconnect();

    // State-machine guard: once a disconnect has been initiated (stdin `disconnect` RPC or peer
    // disconnect), further disconnect RPCs and stdin command dispatch are suppressed (the
    // connection is tearing down).
    bool TryInitiateDisconnect()
    {
        bool expected = false;
        return disconnectInitiated_.compare_exchange_strong(expected, true);
    }
    bool IsDisconnectInitiated() const { return disconnectInitiated_.load(); }

private:
    mutable std::mutex mtx_;
    std::condition_variable connectedCv_;
    bool connected_ = false;
    int32_t connectResult_ = -1;
    sptr<AgentRuntime::IAgentReceiver> receiverProxy_;
    sptr<CliAgentConnectorStub> hostStub_;
    std::string agentId_;
    std::string disconnectReason_ = "peer";  // default: peer-initiated disconnect
    std::atomic<bool> disconnectInitiated_{false};  // set on first disconnect (stdin RPC or peer)
    int wakeupPipe_[2] = {-1, -1};            // [0]=read (poll), [1]=write (signal)
};

}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_CLI_AGENT_CONNECTION_H
