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

#include "session_protocol.h"

#include <algorithm>
#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <poll.h>
#include <string>
#include <unistd.h>

#include <nlohmann/json.hpp>

#include "agent_manager_client.h"  // AgentRuntime::AgentManagerClient (DisconnectAgentExtensionAbility)
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {

namespace {
// Byte cap for the internal line buffer; keeps memory bounded on a broken/hostile stream.
constexpr size_t MAX_LINE_BYTES = 200 * 1024;

// No-throw string field extraction; returns "" if absent or non-string.
std::string GetJsonString(const nlohmann::json &j, const char *key)
{
    if (j.contains(key) && j[key].is_string()) {
        return j[key].get<std::string>();
    }
    return "";
}

// Dispatch one command; all paths are non-fatal (fatal exits are poll/read failures in
// RunCommandLoop).
void DispatchLine(const std::string &line, const sptr<CliAgentConnection> &connection,
    const sptr<AgentRuntime::IAgentReceiver> &receiver, const sptr<IRemoteObject> &hostObj)
{
    // allow_exceptions=false: parse failure returns a discarded json instead of throwing.
    nlohmann::json cmd = nlohmann::json::parse(line, nullptr, false);
    if (cmd.is_discarded() || !cmd.is_object()) {
        EmitAgentEvent(BuildErrorEvent("bad-json", "not a JSON object", false));
        return;
    }
    std::string op = GetJsonString(cmd, "op");
    if (op == "sendData" || op == "authorize") {
        if (receiver == nullptr) {
            EmitAgentEvent(BuildErrorEvent("no-receiver", "agent receiver proxy is null", false));
            return;
        }
        // JS/ETS-aligned: data must be present and a string (empty string passes through).
        if (!cmd.contains("data") || !cmd["data"].is_string()) {
            EmitAgentEvent(BuildErrorEvent("bad-data", "data field missing or not a string", false));
            return;
        }
        std::string data = cmd["data"].get<std::string>();
        int32_t ret = (op == "sendData") ? receiver->SendData(hostObj, data)
                                         : receiver->Authorize(hostObj, data);
        if (ret != 0) {
            const char *opName = (op == "sendData") ? "SendData" : "Authorize";
            const char *evt = (op == "sendData") ? "send-failed" : "authorize-failed";
            EmitAgentEvent(BuildErrorEvent(evt, std::string(opName) + " ret=" + std::to_string(ret), false));
        }
        return;
    }
    if (op == "disconnect") {
        // A peer disconnect may have raced ahead and already emitted disconnect-done.
        if (connection->IsDisconnectInitiated()) {
            return;
        }
        connection->SetDisconnectReason(NormalizeDisconnectReason(GetJsonString(cmd, "reason")));
        const char *idEnv = getenv("ohos_cli_callerIdentity");
        std::string callerIdentity = (idEnv != nullptr) ? idEnv : "";
        int32_t ret = AgentRuntime::AgentManagerClient::GetInstance().DisconnectAgentExtensionAbilityForCli(
            connection, callerIdentity);
        if (ret != 0) {
            // Non-fatal: keep the flag clear so the caller may retry; suppress the error event
            // if a peer disconnect already emitted disconnect-done.
            if (!connection->IsDisconnectInitiated()) {
                EmitAgentEvent(BuildErrorEvent("disconnect-failed",
                    "DisconnectAgentExtensionAbilityForCli ret=" + std::to_string(ret), false));
            }
            return;
        }
        // RPC accepted: mark Disconnecting (idempotent) and wait for OnAbilityDisconnectDone to
        // emit disconnect-done and wake the loop.
        (void)connection->TryInitiateDisconnect();
        return;
    }
    EmitAgentEvent(BuildErrorEvent("unknown-op", "unknown op: " + op, false));
}

// Self-delimiting JSON reassembly (no '\n' framing): a JSON value defines its own boundary.
// BoundarySax tracks whether a top-level value completed and the parse_error position.
class BoundarySax : public nlohmann::json_sax<nlohmann::json> {
public:
    int valuesCompleted = 0;   // 1 if a top-level value fully parsed
    std::size_t errorPos = 0;  // parse_error position
    std::string errorMsg;

    bool null() override
    {
        OnScalar();
        return true;
    }

    bool boolean(bool) override
    {
        OnScalar();
        return true;
    }

    bool number_integer(number_integer_t) override
    {
        OnScalar();
        return true;
    }

    bool number_unsigned(number_unsigned_t) override
    {
        OnScalar();
        return true;
    }

    bool number_float(number_float_t, const string_t &) override
    {
        OnScalar();
        return true;
    }

    bool string(string_t &) override
    {
        OnScalar();
        return true;
    }

    bool binary(binary_t &) override
    {
        OnScalar();
        return true;
    }

    bool start_object(std::size_t) override
    {
        ++depth_;
        return true;
    }

    bool start_array(std::size_t) override
    {
        ++depth_;
        return true;
    }

    bool end_object() override
    {
        if (--depth_ == 0) {
            valuesCompleted = 1;
        }
        return true;
    }

    bool end_array() override
    {
        if (--depth_ == 0) {
            valuesCompleted = 1;
        }
        return true;
    }

    bool key(string_t &) override
    {
        return true;
    }

    bool parse_error(std::size_t position, const std::string &,
        const nlohmann::detail::exception &ex) override
    {
        errorPos = position;
        errorMsg = ex.what();
        return false;
    }

private:
    void OnScalar()
    {
        if (depth_ == 0) {  // a top-level scalar completes a value
            valuesCompleted = 1;
        }
    }

    int depth_ = 0;
};

// Length of the first complete JSON value in `buf`. `hint` (parse_error position) is a 1-indexed
// column and may point past EOF for an incomplete trailing literal — re-parse prefixes downward.
// Iterations are bounded by the 4KB read chunking.
std::size_t FindFirstCompleteValueEnd(const std::string &buf, std::size_t hint)
{
    std::size_t upper = std::min(hint, buf.size());
    if (upper == 0) {
        upper = buf.size();  // defensive
    }
    for (std::size_t e = upper; e > 0; --e) {  // largest clean prefix first
        nlohmann::json j = nlohmann::json::parse(buf.substr(0, e), nullptr, false);
        if (!j.is_discarded()) {
            return e;
        }
    }
    return buf.size();  // fallback
}

// Dispatch every complete JSON value in `buf`; keep the incomplete tail. Malformed mid-buffer
// content is dropped byte-by-byte until a value parses (non-fatal; consecutive malformed bytes
// are aggregated into one bad-json event per call).
void DrainCompleteJsonValues(std::string &buf, const sptr<CliAgentConnection> &connection,
    const sptr<AgentRuntime::IAgentReceiver> &receiver, const sptr<IRemoteObject> &hostObj)
{
    size_t resyncBytes = 0;
    std::string firstErr;
    while (!buf.empty()) {
        BoundarySax sax;
        bool ok = nlohmann::json::sax_parse(buf, &sax);  // strict: trailing bytes trigger parse_error
        if (sax.valuesCompleted >= 1) {
            // ok -> the whole buf is one value; !ok -> locate the first value's exact end.
            std::size_t end = ok ? buf.size() : FindFirstCompleteValueEnd(buf, sax.errorPos);
            std::string one = buf.substr(0, end);
            buf.erase(0, end);
            if (!connection->IsDisconnectInitiated()) {
                DispatchLine(one, connection, receiver, hostObj);
            }
            continue;
        }
        if (sax.errorPos < buf.size()) {
            // Malformed mid-buffer: drop through the offending byte and resync so later valid
            // values survive (e.g. {"a":1}\x01{"b":2} keeps {"b":2}).
            if (resyncBytes == 0) {
                firstErr = sax.errorMsg;
            }
            std::size_t drop = (sax.errorPos > 0) ? sax.errorPos : 1;
            resyncBytes += drop;
            buf.erase(0, drop);
            continue;
        }
        break;  // incomplete (EOF mid-value): keep the buffer, wait for more bytes
    }
    if (resyncBytes > 0) {
        EmitAgentEvent(BuildErrorEvent("bad-json",
            "malformed JSON, dropped " + std::to_string(resyncBytes) + " bytes: " + firstErr, false));
    }
}
}  // namespace

bool RunCommandLoop(const sptr<CliAgentConnection> &connection,
    const sptr<AgentRuntime::IAgentReceiver> &receiver, const sptr<IRemoteObject> &hostObj)
{
    if (connection == nullptr) {
        return false;  // nothing to loop on; not a fatal infra error
    }
    int wakeupFd = connection->GetWakeupFd();
    std::string lineBuf;
    while (true) {
        struct pollfd pfds[2];
        pfds[0].fd = STDIN_FILENO;
        pfds[0].events = POLLIN;
        pfds[0].revents = 0;
        pfds[1].fd = wakeupFd;
        pfds[1].events = POLLIN;
        pfds[1].revents = 0;
        nfds_t nfds = (wakeupFd >= 0) ? 2 : 1;
        int ret = poll(pfds, nfds, -1);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
            EmitAgentEvent(BuildErrorEvent("poll-error", "poll failed", true));
            return true;  // fatal: caller exits non-zero
        }
        // fd errors would otherwise make poll ready forever without a matching branch -> busy spin.
        if ((pfds[0].revents & (POLLERR | POLLNVAL)) != 0 ||
            (wakeupFd >= 0 && (pfds[1].revents & (POLLERR | POLLNVAL)) != 0)) {
            EmitAgentEvent(BuildErrorEvent("stdin-read-error", "poll revents error", true));
            return true;  // fatal
        }
        // Peer disconnect -> normal exit (disconnect-done already emitted by OnAbilityDisconnectDone).
        if (wakeupFd >= 0 && (pfds[1].revents & (POLLIN | POLLHUP))) {
            return false;
        }
        // POLLHUP without POLLIN marks a pipe at EOF; without it the loop would busy-spin on EOF.
        if (pfds[0].revents & (POLLIN | POLLHUP)) {
            char buf[4096];
            ssize_t n = read(STDIN_FILENO, buf, sizeof(buf));
            if (n < 0) {
                if (errno == EINTR) {
                    continue;
                }
                EmitAgentEvent(BuildErrorEvent("stdin-read-error", "read stdin failed", true));
                return true;  // fatal: caller exits non-zero
            }
            if (n == 0) {
                return false;  // EOF: stdin closed -> normal exit
            }
            lineBuf.append(buf, static_cast<size_t>(n));
            // Self-delimiting reassembly (no '\n' framing); value boundary is the JSON itself.
            DrainCompleteJsonValues(lineBuf, connection, receiver, hostObj);
            // Non-fatal cap on the remaining incomplete tail: keeps memory bounded; complete
            // values were already drained and dispatched above.
            if (lineBuf.size() > MAX_LINE_BYTES) {
                EmitAgentEvent(BuildErrorEvent("bad-json", "exceeds 200KB limit", false));
                lineBuf.clear();
            }
        }
    }
}

std::string NormalizeDisconnectReason(const std::string &reason)
{
    if (reason == "peer" || reason == "timeout" || reason == "error") {
        return reason;
    }
    return "user";
}

}  // namespace AAFwk
}  // namespace OHOS
