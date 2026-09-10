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

#include "cli_agent_connection.h"

#include <atomic>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

#include "hilog_tag_wrapper.h"
#include "securec.h"

namespace OHOS {
namespace AAFwk {

namespace {
// Serialize stdout emission across the agentmgr binder threads (OnAbilityConnectDone /
// OnAbilityDisconnectDone / agent->host stub calls) and the main thread.
std::mutex &StdoutMutex()
{
    static std::mutex m;
    return m;
}

// Set by DrainStdoutForExit under StdoutMutex before _exit(); EmitAgentEvent checks it before
// and under the lock so a late binder-thread write is dropped instead of truncated by _exit.
std::atomic<bool> &ExitingFlag()
{
    static std::atomic<bool> flag;
    return flag;
}

// Length of the UTF-8 sequence at s[i], or 0 if invalid (bad lead byte, truncated tail, bad
// continuation, overlong encoding, surrogate, or beyond U+10FFFF).
size_t Utf8SequenceLength(const std::string &s, size_t i)
{
    unsigned char lead = static_cast<unsigned char>(s[i]);
    size_t len = (lead >= 0xC2 && lead <= 0xDF) ? 2
        : (lead >= 0xE0 && lead <= 0xEF) ? 3
        : (lead >= 0xF0 && lead <= 0xF4) ? 4 : 0;
    if (len == 0 || s.size() - i < len) {
        return 0;
    }
    for (size_t k = 1; k < len; ++k) {
        unsigned char cc = static_cast<unsigned char>(s[i + k]);
        if (cc < 0x80 || cc > 0xBF) {
            return 0;
        }
    }
    unsigned char c1 = static_cast<unsigned char>(s[i + 1]);
    bool outOfRange = (lead == 0xE0 && c1 < 0xA0) || (lead == 0xED && c1 > 0x9F) ||
        (lead == 0xF0 && c1 < 0x90) || (lead == 0xF4 && c1 > 0x8F);
    return outOfRange ? 0 : len;
}
}  // namespace

void EmitAgentEvent(const std::string &eventJsonLine)
{
    if (ExitingFlag().load(std::memory_order_acquire)) {
        return;  // tearing down: drop rather than risk a write truncated by _exit
    }
    std::lock_guard<std::mutex> lock(StdoutMutex());
    if (ExitingFlag().load(std::memory_order_acquire)) {  // re-check under the lock
        return;
    }
    std::cout << eventJsonLine << "\n";
    std::cout.flush();
}

void DrainStdoutForExit()
{
    // Set the flag and flush under the lock: in-flight writers finish first, later ones drop.
    std::lock_guard<std::mutex> lock(StdoutMutex());
    ExitingFlag().store(true, std::memory_order_release);
    std::cout.flush();
}

std::string EscapeJsonString(const std::string &s)
{
    // Reserve slack: a control byte expands to 6 chars (\uXXXX).
    constexpr size_t ESCAPE_SLACK = 8;
    std::string out;
    out.reserve(s.size() + ESCAPE_SLACK);
    for (size_t i = 0; i < s.size();) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        switch (c) {
            case '"': out += "\\\""; ++i; continue;
            case '\\': out += "\\\\"; ++i; continue;
            case '\b': out += "\\b"; ++i; continue;
            case '\f': out += "\\f"; ++i; continue;
            case '\n': out += "\\n"; ++i; continue;
            case '\r': out += "\\r"; ++i; continue;
            case '\t': out += "\\t"; ++i; continue;
            default: break;
        }
        if (c < 0x20) {
            char buf[8];
            // Cast: %x expects unsigned int; the promoted int would trip -Wformat.
            // snprintf_s returns the printed length, or -1 on error/truncation (securec).
            int ret = snprintf_s(buf, sizeof(buf), sizeof(buf) - 1, "\\u%04x",
                static_cast<unsigned int>(c));
            if (ret < 0) {
                // Unreachable for this fixed format (6 chars + NUL fits buf[8]); fall back to
                // the replacement char so the emitted line stays valid JSON either way.
                out += "\\ufffd";
            } else {
                out += buf;
            }
            ++i;
            continue;
        }
        if (c < 0x80) {
            out += static_cast<char>(c);  // plain ASCII
            ++i;
            continue;
        }
        // Pass valid UTF-8 through; replace invalid bytes so the event line stays valid JSON
        // (RFC 8259 requires UTF-8).
        size_t len = Utf8SequenceLength(s, i);
        if (len > 0) {
            out.append(s, i, len);
            i += len;
        } else {
            out += "\\uFFFD";
            ++i;
        }
    }
    return out;
}

std::string BuildConnectDoneEvent(bool ok, const std::string &agentId, int32_t errCode, const std::string &errMsg)
{
    std::string s;
    s += "{\"type\":\"connect-done\",\"status\":\"";
    s += (ok ? "ok" : "fail");
    s += "\",\"agentId\":\"";
    s += EscapeJsonString(agentId);
    s += "\",\"errCode\":\"";
    s += std::to_string(errCode);
    s += "\",\"errMsg\":\"";
    s += EscapeJsonString(errMsg);
    s += "\"}";
    return s;
}

std::string BuildDisconnectDoneEvent(const std::string &reason)
{
    return "{\"type\":\"disconnect-done\",\"reason\":\"" + EscapeJsonString(reason) + "\"}";
}

std::string BuildOnDataEvent(const std::string &data)
{
    return "{\"type\":\"OnData\",\"data\":\"" + EscapeJsonString(data) + "\"}";
}

std::string BuildOnAuthorizeEvent(const std::string &data)
{
    return "{\"type\":\"OnAuthorize\",\"data\":\"" + EscapeJsonString(data) + "\"}";
}

std::string BuildErrorEvent(const std::string &errCode, const std::string &errMsg, bool fatal)
{
    std::string s;
    s += "{\"type\":\"error\",\"errCode\":\"";
    s += EscapeJsonString(errCode);
    s += "\",\"errMsg\":\"";
    s += EscapeJsonString(errMsg);
    s += "\",\"fatal\":";
    s += (fatal ? "true" : "false");
    s += "}";
    return s;
}

// ---- CliAgentConnectorStub (agent->host) ----

int32_t CliAgentConnectorStub::SendData(const std::string &data)
{
    TAG_LOGI(AAFwkTag::AA_TOOL, "agent->host SendData, len=%{public}zu", data.size());
    EmitAgentEvent(BuildOnDataEvent(data));
    return 0;  // ERR_OK
}

int32_t CliAgentConnectorStub::Authorize(const std::string &data)
{
    TAG_LOGI(AAFwkTag::AA_TOOL, "agent->host Authorize, len=%{public}zu", data.size());
    EmitAgentEvent(BuildOnAuthorizeEvent(data));
    return 0;  // ERR_OK
}

// ---- CliAgentConnection (IAbilityConnection) ----

CliAgentConnection::CliAgentConnection()
{
    // Self-pipe wakeup so a peer disconnect can unblock the loop's idle stdin poll.
    if (pipe2(wakeupPipe_, O_CLOEXEC | O_NONBLOCK) != 0) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "pipe2 for disconnect wakeup failed");
        wakeupPipe_[0] = -1;
        wakeupPipe_[1] = -1;
    }
}

CliAgentConnection::~CliAgentConnection()
{
    if (wakeupPipe_[0] >= 0) {
        (void)close(wakeupPipe_[0]);
    }
    if (wakeupPipe_[1] >= 0) {
        (void)close(wakeupPipe_[1]);
    }
}

void CliAgentConnection::SignalDisconnect()
{
    if (wakeupPipe_[1] >= 0) {
        char c = 1;  // non-blocking; EAGAIN is fine (a pending wakeup is already armed)
        (void)write(wakeupPipe_[1], &c, 1);
    }
}

void CliAgentConnection::OnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    bool ok = (resultCode == 0);
    std::string agentId;
    sptr<AgentRuntime::IAgentReceiver> proxy;
    if (ok && remoteObject != nullptr) {
        // The IRemoteObject handed back is the agent's IAgentReceiver stub; wrap it in the proxy.
        proxy = iface_cast<AgentRuntime::IAgentReceiver>(remoteObject);
    }
    // A "successful" connect that did not deliver a usable receiver proxy is not usable; treat
    // it as failure so the caller exits instead of idling with no-receiver errors.
    if (ok && proxy == nullptr) {
        ok = false;
    }
    int32_t reportResult = ok ? resultCode
                               : ((resultCode == 0) ? CONNECT_RESULT_PROXY_NULL : resultCode);
    std::string errMsg = ok ? ""
                            : ((reportResult == CONNECT_RESULT_PROXY_NULL) ? "receiver proxy null"
                                                                             : "OnAbilityConnectDone non-zero");
    {
        std::lock_guard<std::mutex> lock(mtx_);
        agentId = agentId_;
        receiverProxy_ = proxy;  // null on failure
        connectResult_ = reportResult;
        connected_ = true;
    }
    TAG_LOGI(AAFwkTag::AA_TOOL, "OnAbilityConnectDone result=%{public}d proxy=%{public}s",
        resultCode, (proxy != nullptr ? "ok" : "null"));
    EmitAgentEvent(BuildConnectDoneEvent(ok, agentId, reportResult, errMsg));
    connectedCv_.notify_one();
}

void CliAgentConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    // Mark Disconnecting (idempotent): a stdin `disconnect` may have set it already; the
    // disconnect-done event and wakeup below still fire either way.
    (void)TryInitiateDisconnect();
    std::string reason;
    {
        std::lock_guard<std::mutex> lock(mtx_);
        reason = disconnectReason_;  // "user" when host-initiated, else default "peer"
        if (resultCode != 0) {
            reason = "error";
        }
    }
    TAG_LOGI(AAFwkTag::AA_TOOL, "OnAbilityDisconnectDone result=%{public}d reason=%{public}s",
        resultCode, reason.c_str());
    EmitAgentEvent(BuildDisconnectDoneEvent(reason));
    SignalDisconnect();  // wake the command loop's poll()
}

int32_t CliAgentConnection::WaitForConnect(int32_t timeoutMs, sptr<AgentRuntime::IAgentReceiver> &outProxy,
    sptr<IRemoteObject> &outHostObj)
{
    std::unique_lock<std::mutex> lock(mtx_);
    if (!connectedCv_.wait_for(lock, std::chrono::milliseconds(timeoutMs), [this] { return connected_; })) {
        return -1;  // timeout
    }
    outProxy = receiverProxy_;
    outHostObj = (hostStub_ != nullptr ? hostStub_->AsObject() : nullptr);
    return connectResult_;
}

}  // namespace AAFwk
}  // namespace OHOS
