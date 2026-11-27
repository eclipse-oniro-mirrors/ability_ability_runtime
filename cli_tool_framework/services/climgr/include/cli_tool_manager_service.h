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

#ifndef OHOS_ABILITY_RUNTIME_CLI_TOOL_MGR_SERVICE_H
#define OHOS_ABILITY_RUNTIME_CLI_TOOL_MGR_SERVICE_H

#include <atomic>
#include <chrono>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "cli_tool_manager_stub.h"
#include "cli_tool_data_manager.h"
#include "exec_cmd_param.h"
#include "exec_options.h"
#include "exec_result_wrap.h"
#include "ffrt.h"
#include "function_result_wrap.h"
#include "invoke_function_param.h"
#include "invoke_function_result.h"
#include "iremote_object.h"
#include "system_ability.h"
#include "system_ability_definition.h"
#include "want_params.h"

#include "io_monitor.h"
#include "process_manager.h"
#include "session_record.h"
#include "skill/skill_execute_callback_stub.h"

namespace OHOS {
namespace AppExecFwk {
class IApplicationStateObserver;
struct SkillExecuteResult;
}
namespace CliTool {
class SessionRecord;
class SkillCallbackAdapter;
class CliToolManagerService;

enum class HookType { CLI, FUNCTION };

class CliToolManagerService : public SystemAbility,
                              public CliToolManagerStub {
    DECLARE_SYSTEM_ABILITY(CliToolManagerService);

    friend class SkillCallbackAdapter;

public:
    static sptr<CliToolManagerService> GetInstance();
    virtual ~CliToolManagerService() = default;

    /**
     * @brief Query all available tools
     */
    int32_t GetAllToolInfos(ToolsRawData &tools) override;

    /**
     * @brief Query tool summaries (lightweight for listing)
     */
    int32_t GetAllToolSummaries(std::vector<ToolSummary> &summaries) override;

    /**
     * @brief Get tool information by name
     */
    int32_t GetToolInfoByName(const std::string &name, ToolInfo &tool) override;

    /**
     * @brief Execute a CLI tool with key-value pairs (convenience method).
     * @param param The CLI tool param.
     * @param objectCallback The callback RemoteObject.
     * @return Returns ERR_OK on success, error code otherwise.
     */
    int32_t ExecTool(const ExecToolParam &param, const std::string &eventId,
        const sptr<ICliToolManagerScheduler> &scheduler) override;

    int32_t ExecCmd(const ExecCmdParam &param, const std::string &eventId,
        const sptr<ICliToolManagerScheduler> &scheduler, const std::string &subscriptionId) override;

    int32_t ClearSession(const std::string &sessionId) override;
    int32_t CancelSession(const std::string &sessionId, int32_t signal) override;
    int32_t SubscribeSession(const std::string &sessionId, const std::string &subscriptionId,
        const sptr<ICliToolManagerScheduler> &scheduler) override;
    int32_t UnsubscribeSession(const std::string &sessionId, const std::string &subscriptionId) override;
    
    int32_t QuerySession(const std::string &sessionId, CliSessionInfo &session) override;
    int32_t SendMessage(const std::string &sessionId,
        const std::string &inputText, const std::string &eventId,
        const sptr<ICliToolManagerScheduler> &scheduler) override;

    /**
     * @brief Batch query command permissions (Inner API, for SA only).
     */
    int32_t BatchQueryPermissionBySubCommand(const std::vector<Command> &cmds,
        std::vector<CommandPermission> &cmdPermissions) override;

    /**
     * @brief Register a function
     * @param function FunctionInfo to register
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t RegisterFunction(const FunctionInfo &function) override;

    /**
     * @brief Batch register functions
     * @param functions Vector of FunctionInfo to register
     * @param successCount Output count of successfully registered functions
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t BatchRegisterFunctions(const FunctionsRawData &functions, int32_t &successCount) override;

    /**
     * @brief Get function information by bundleName and functionName; always scoped to
     *        the IPC caller's own user
     * @param bundleName Bundle name
     * @param functionName Function name
     * @param function Output FunctionInfo
     * @return int32_t ERR_OK if found, error code otherwise
     */
    int32_t GetFunctionInfo(const std::string &functionNamespace,
        const std::string &functionName, FunctionInfo &function) override;

    /**
     * @brief Unregister a function
     * @param userId Owner user whose record is deleted (caller-supplied, must be >= 0)
     * @param functionNamespace Namespace
     * @param functionName Function name
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t UnregisterFunction(int32_t userId, const std::string &functionNamespace,
        const std::string &functionName) override;

    /**
     * @brief Batch unregister intentFunctions by namespace
     * @param userId Owner user whose records are deleted (caller-supplied, must be >= 0)
     * @param functionNamespace Namespace to delete all functions from
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t UnregisterIntentFunctionsByNamespace(int32_t userId, const std::string &functionNamespace) override;

    /**
     * @brief Reset all functions by namespace (delete all existing and add new ones)
     * @param userId Owner user whose records are reset (caller-supplied, must be >= 0)
     * @param functionNamespace Namespace to reset functions for
     * @param functions New function list to replace existing ones
     * @param successCount Output count of successfully reset functions
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t ResetNamespaceFunctions(int32_t userId, const std::string &functionNamespace,
        const FunctionsRawData &functions, int32_t &successCount) override;

    /**
     * @brief Get all functions via raw data (for large data transfer); always scoped to
     *        the IPC caller's own user
     * @param functions Output FunctionsRawData
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t GetAllFunctions(FunctionsRawData &functions) override;

    int32_t RegisterCliHook(const sptr<ICliHookInterface> &hook, int32_t activeMethods) override;
    int32_t UnregisterCliHook(const sptr<ICliHookInterface> &hook) override;
    int32_t RegisterFunctionHook(const sptr<IFunctionHookInterface> &hook, int32_t activeMethods) override;
    int32_t UnregisterFunctionHook(const sptr<IFunctionHookInterface> &hook) override;
    int32_t BeforeInvokeFunction(InvokeFunctionParam &param) override;
    int32_t AfterInvokeFunction(FunctionResultWrap &functionResultWrap) override;

    /**
     * @brief Batch register functions (one-way, result not reported to caller)
     * @param functions FunctionsRawData to register
     * @return int32_t ERR_OK on success, error code otherwise (local result only, not sent back)
     */
    int32_t BatchRegisterFunctionsAsync(const FunctionsRawData &functions) override;

    /**
     * @brief Batch unregister intentFunctions by namespace (one-way, result not reported to caller)
     * @param userId Owner user whose records are deleted (caller-supplied, must be >= 0)
     * @param functionNamespace Namespace to delete all functions from
     * @return int32_t ERR_OK on success, error code otherwise (local result only, not sent back)
     */
    int32_t UnregisterIntentFunctionsByNamespaceAsync(int32_t userId,
        const std::string &functionNamespace) override;

    /**
     * @brief Reset all functions by namespace (one-way, result not reported to caller)
     * @param userId Owner user whose records are reset (caller-supplied, must be >= 0)
     * @param functionNamespace Namespace to reset functions for
     * @param functions New function list to replace existing ones
     * @return int32_t ERR_OK on success, error code otherwise (local result only, not sent back)
     */
    int32_t ResetNamespaceFunctionsAsync(int32_t userId, const std::string &functionNamespace,
        const FunctionsRawData &functions) override;

    void OnHookDied(const wptr<IRemoteObject> &remote, HookType type);

protected:
    void OnStart() override;
    void OnStop() override;
    int32_t OnIdle(const SystemAbilityOnDemandReason &idlReason) override;

private:
    CliToolManagerService() : SystemAbility(CLI_TOOL_MGR_SERVICE_ID, false) {};

    // RAII wrapper for interface call counter
    class InterfaceCallCounter {
    public:
        explicit InterfaceCallCounter(std::atomic<int32_t>& counter) : counter_(counter)
        {
            counter_.fetch_add(1, std::memory_order_acq_rel);
        }
        ~InterfaceCallCounter()
        {
            counter_.fetch_sub(1, std::memory_order_acq_rel);
        }
        InterfaceCallCounter(const InterfaceCallCounter&) = delete;
        InterfaceCallCounter& operator=(const InterfaceCallCounter&) = delete;
    private:
        std::atomic<int32_t>& counter_;
    };

    enum class ServiceRunningState { STATE_NOT_START, STATE_RUNNING };

    /**
     * @brief Caller identity and event-routing context for tool command mode.
     *
     * Groups the per-call plumbing arguments shared by ExecCmdToolMode/SetupCmdSession
     * so their signatures stay within the 5-parameter limit.
     */
    struct CmdSessionContext {
        std::string eventId;
        std::string subscriptionId;
        sptr<ICliToolManagerScheduler> scheduler;
        int32_t callerPid = 0;
        int32_t callerUid = 0;
        uint32_t tokenId = 0;
        std::string callerBundleName;
    };

    /**
     * @brief Initialize service-side signal handling and I/O callbacks.
     *
     * Installs a minimal async-signal-safe SIGCHLD handler that only writes one wake byte to a non-blocking
     * pipe, then spawns a dedicated reaper thread (ReaperLoop) that owns all waitpid()/WaitPid()/Killpg() work
     * in normal (non-signal) context. I/O callbacks look up the singleton and return when it is unavailable.
     */
    void Init();

    /**
     * @brief Schedule delayed unloading of the CLI manager system ability.
     *
     * The service is unloaded only when no IPC call is active, no session remains, and no observed caller process
     * is still tracked. Otherwise a new delayed check is scheduled.
     */
    void DelayUnloadTask();

    std::shared_ptr<SessionRecord> CreateSessionRecord(const ExecToolParam &param, const std::string &eventId);
    // Cancel eligibility snapshotted at execTool session creation: registered tools return
    // the subcommand's own declared value when a subcommand dimension exists (no
    // tool-level fallback), the tool-level declaration otherwise; unregistered tools and
    // subcommands default to false.
    bool SnapshotCancelable(const std::string &toolName, const std::string &subcommand) const;
    // Cancel eligibility snapshotted at execCmd (shell mode) session creation. The command
    // is resolved against the tool registry first: a registered CLI tool is governed by its
    // own declaration (same model as execTool sessions, so a tool's cancelable value is
    // honored no matter which exec path was taken); anything else (arbitrary shell command,
    // unresolvable input) has no declaration to consult and returns true (best-effort).
    bool SnapshotShellCancelable(const std::string &cmd) const;
    void AddSessionRecord(const std::shared_ptr<SessionRecord> &record);
    int32_t TryAddSessionRecord(const std::shared_ptr<SessionRecord> &record);
    std::shared_ptr<SessionRecord> GetSessionRecord(const std::string &sessionId);
    std::vector<std::shared_ptr<SessionRecord>> GetSessionRecords();
    void RemoveSessionRecord(const std::string &sessionId);

    void AttachTaskAttribution(const std::shared_ptr<SessionRecord> &record);
    // Async continuation of AttachTaskAttribution on an ffrt worker: snapshots the chain state
    // (seams read ONCE here) and runs the first discovery round immediately. The chain polls
    // ProcScanner for the sandboxed command pid by re-submitting each round with an ffrt delay
    // — never sleep_for on a worker — so an undiscoverable child (exec-in-place form never
    // forks one, or /proc blindness) does not park an ffrt worker for the whole window while
    // sibling tasks (detach IPC, delayed unload, timeout scheduling) need the pool. Aborts
    // without registering when the teardown consumed the PENDING state mid-chain (nothing
    // was registered, so nothing needs compensation).
    void ResolveCommandAndAttach(const std::shared_ptr<SessionRecord> &record, const std::string &taskId,
        pid_t anchor, int32_t callerUid);
    // Everything one discovery round needs that must survive the ffrt task that ran it:
    // held by shared_ptr across the delay-resubmit chain. pollMs/deadline are snapshotted at
    // chain start so a mid-chain seam mutation (test restoring the production window) cannot
    // retime a discovery already in flight; observedAlive threads the ALIVE->DEAD gate
    // across rounds.
    struct DiscoveryState {
        std::shared_ptr<SessionRecord> record;
        std::string taskId;
        pid_t anchor = -1;
        int32_t callerUid = -1;
        int32_t pollMs = 50;
        std::chrono::steady_clock::time_point deadline;
        bool observedAlive = false;
    };
    // One round of the discovery chain, in the historical order: pending-check =>
    // FindCommandProcess => deadline-check => liveness gate. Terminal rounds hand off to
    // FinishCommandAttribution (child found, or window lapsed into the unresolved,
    // no-registration outcome) or simply return (teardown consumed the PENDING state /
    // observed ALIVE->DEAD skip); every other round re-submits itself through
    // ResubmitCommandDiscovery instead of sleeping.
    void PollCommandDiscovery(std::shared_ptr<DiscoveryState> state);
    // Delayed self-resubmission of the next discovery round (ffrt::task_attr().delay): the
    // singleton is re-fetched inside the submitted task because the service may unload
    // between rounds.
    void ResubmitCommandDiscovery(const std::shared_ptr<DiscoveryState> &state);
    // Shared terminal of the found and deadline outcomes. commandPid > 0: stores the valid
    // pid on the record, re-checks PENDING, runs the attach IPC, and reconciles via
    // WriteBackAttachResult (posting the compensating detach when a teardown consumed the
    // record while the IPC was in flight). commandPid <= 0 (window lapsed, no visible
    // child): registers NOTHING — the anchor is only the sandbox monitor — and concludes the
    // record through the same write-back (PENDING -> FAILED with zero IPC; a teardown that
    // already consumed the PENDING state makes the finisher a strict no-op).
    void FinishCommandAttribution(std::shared_ptr<DiscoveryState> state, pid_t commandPid);
    // Discovery-poll guard: true only while the record is still the live table entry and its
    // attribution is still PENDING. Races starting after a true verdict are arbitrated by
    // WriteBackAttachResult on the attach side.
    bool IsAttributionPending(const std::shared_ptr<SessionRecord> &record);
    // Test seam only: command-process discovery tuning. Production keeps the defaults.
    static inline std::atomic<int32_t> commandDiscoveryPollMs_ { 50 };
    static inline std::atomic<int32_t> commandDiscoveryTimeoutMs_ { 2000 };
    void PostDetachTaskAttribution(const std::string &taskId, int64_t attributionId, int32_t callerUid);
    // Caller must hold sessionsMutex_. Consumes an ATTACHED attribution into the detach triple
    // (returns true) or relays a PENDING one to the write-back (CONSUMED, nothing to detach now).
    bool TryConsumeAttribution(SessionRecord &record, std::string &taskId, int64_t &attributionId,
        int32_t &callerUid);
    // Writes the IPC verdict back under sessionsMutex_ (PENDING -> ATTACHED/FAILED). Returns true
    // when a teardown consumed the record while the IPC was in flight: the caller must post the
    // compensating detach (it is the single finisher).
    bool WriteBackAttachResult(const std::shared_ptr<SessionRecord> &record, const char *action,
        bool attributed, int32_t ret, pid_t reportedPid, const std::string &taskId,
        int64_t attributionId);
    // Caller must hold sessionsMutex_. Collects an ATTACHED record's detach triple; a PENDING one
    // is relayed to the write-back instead.
    void CollectDetachRequest(SessionRecord &record, std::vector<std::string> &taskIds,
        std::vector<int64_t> &attributionIds, std::vector<int32_t> &uids);
    // Posts every triple collected by CollectDetachRequest (off-lock, off-thread).
    void PostDetachRequests(const std::vector<std::string> &taskIds,
        const std::vector<int64_t> &attributionIds, const std::vector<int32_t> &uids);

    bool IsSessionOwner(const std::shared_ptr<SessionRecord> &record, const char *action) const;

    /**
     * @brief CancelSession-specific session-owner check with a dedicated error code.
     *
     * Same pid-comparison semantics as IsSessionOwner, but reports the failure through the
     * cancelSession-only ERR_CLI_NOT_SESSION_OWNER instead of the generic permission error,
     * so JS callers can distinguish "not the session owner" from missing permissions.
     * Other session APIs (clearSession/subscribeSession/querySession/sendMessage) keep using
     * IsSessionOwner and must not be affected.
     *
     * @param record Session record to check against the calling pid.
     * @return int32_t ERR_OK when the caller owns the session, ERR_CLI_NOT_SESSION_OWNER otherwise.
     */
    int32_t CheckCancelSessionOwner(const std::shared_ptr<SessionRecord> &record) const;

    /**
     * @brief CancelSession eligibility checks: owner, state, cancelable snapshot, signal
     *        whitelist and processId validity. Call after the record lookup in CancelSession.
     *
     * @param record Non-null session record.
     * @param sessionId For failure logging only.
     * @param signal Requested signal number.
     * @return int32_t ERR_OK when cancellable, the matching error code otherwise.
     */
    int32_t CheckCancelEligibility(const std::shared_ptr<SessionRecord> &record,
        const std::string &sessionId, int32_t signal) const;

    /**
     * @brief Resolve the tool process pid for signal delivery: re-verify the cached
     *        commandPid against a fresh ppid-filtered scan (pid-reuse guard), then bounded
     *        polling when unresolved (no-taskId cold path; the sandbox forks the command
     *        child some delay after the anchor). Caches any resolved pid back into the record.
     *
     * @param record Non-null session record with a valid processId.
     * @param sessionId For failure logging only.
     * @return pid_t The live tool process pid, or -1 when not found within the window.
     */
    pid_t ResolveCommandPid(const std::shared_ptr<SessionRecord> &record, const std::string &sessionId);

    /**
     * @brief Mutex-guarded snapshot of ioMonitor_.
     *
     * OnStop (samgr unload thread) may clear ioMonitor_ while in-flight IPC/ffrt readers still use it;
     * an unsynchronized read/write of a plain shared_ptr is a data race (UB, worst case UAF). Readers must
     * take a snapshot through this accessor (the refcount keeps a still-reading monitor alive) instead of
     * touching ioMonitor_ directly.
     */
    std::shared_ptr<IOMonitor> GetIOMonitor();

    bool RegisterSessionWithMonitors(const std::shared_ptr<SessionRecord> &record, const ExecOptions &options);
    void UnregisterSessionWithMonitors(const std::string &sessionId);

    int32_t ValidateExecToolPermissions();
    int32_t ValidateExecCmdPublicPermissions(bool isShellCommand);
    int32_t SubscribeSessionInternal(const std::string &sessionId, const std::string &subscriptionId,
        const sptr<ICliToolManagerScheduler> &scheduler);
    int32_t ValidateSessionLimit();
    int32_t ValidateSessionPermissions();

    /**
     * @brief Reject callers other than the foundation process (native token) for function write APIs.
     * @param apiName Interface name used in permission-denied logs.
     * @return Returns ERR_OK if the caller is foundation, ERR_PERMISSION_DENIED otherwise.
     */
    int32_t ValidateFoundationCaller(const char *apiName);

    /**
     * @brief Filter functions for ResetNamespaceFunctions: skip entries failing FunctionInfo::Validate and
     * reject the whole batch when any entry's namespace/userId mismatches. An empty result is valid and
     * degrades the reset to deleting all existing functions under (userId, namespace).
     * @return Returns ERR_OK with validFunctions filled, ERR_INVALID_PARAM on mismatch.
     */
    int32_t FilterFunctionsForReset(int32_t userId, const std::string &functionNamespace,
        const std::vector<FunctionInfo> &functionList, std::vector<FunctionInfo> &validFunctions);
    int32_t ValidateAndPrepareTool(const ExecToolParam &param, uint32_t tokenId,
        ToolInfo &toolInfo, std::string &sandboxConfig, std::string &bundleName, std::string& detail);

    int32_t ValidateAndPrepareCmd(const ExecCmdParam &param, uint32_t tokenId,
        std::string &sandboxConfig, std::string &bundleName);
    int32_t ExecCmdToolMode(const ExecCmdParam &param, CmdSessionContext context);
    int32_t SetupCmdSession(const ExecToolParam &toolParam, const ToolInfo &toolInfo,
        const std::string &sandboxConfig, const std::string &toolName, const CmdSessionContext &context);
    int32_t SetupAndStartSession(const ExecToolParam &param, const std::string &eventId,
        const ToolInfo &toolInfo, const std::string &sandboxConfig, const std::string &bundleName);

    int32_t TryDispatchSkillSession(const ExecToolParam &param,
        const std::string &eventId, const ToolInfo &toolInfo, bool &dispatched);
    int32_t SetupAndStartSkillSession(const ExecToolParam &param,
        const std::string &eventId, const ToolInfo &toolInfo);
    int32_t ValidateSkillTypeFromParam(ExecToolParam &param, int32_t &skillType);
    int32_t ValidateSkillType(const std::string &bundleName,
        const std::string &moduleName, const std::string &skillName, int32_t &skillType);
    void HandleSkillSessionComplete(const std::string &sessionId, int32_t callerPid, int32_t callerUid,
        const std::string &eventId, int32_t resultCode, const CliSessionInfo &session);
    void HandleSkillSessionTimeout(const std::string &sessionId);

    void HandleBackgroundSessionReply(const std::shared_ptr<SessionRecord> &record, const std::string &eventId);
    void HandleProcessTimeout(const std::string &sessionId);
    void HandleProcessYieldTimeout(const std::string &sessionId);
    void HandleOutputClosed(const std::string &sessionId, bool isStdout);
    void HandleOutputDrained(const std::string &sessionId);
    void FinalizeBackgroundSession(const std::shared_ptr<SessionRecord> &record);

    DISALLOW_COPY_AND_MOVE(CliToolManagerService);

private:
    static sptr<CliToolManagerService> instance_;

    static void sigchld_handler(int32_t sig);

    // Dedicated thread that owns all SIGCHLD reaping and post-exit cleanup. The signal handler only writes one
    // wake byte to reapPipeWriteFd_; all waitpid()/WaitPid()/Killpg() work runs here in normal (non-signal)
    // context, so locks, IPC, and HiSysEvent are safe.
    void ReaperLoop();
    // Bounded teardown of the reaper subsystem: installs SIG_IGN, stops the loop, wakes + joins the reaper, and
    // closes the pipe. Idempotent (no-op if the reaper was never started).
    void StopReaper();

    void PostExecToolTask(int64_t time, const std::string &sessionId, bool isTimeout);
    void WaitPid(pid_t pid, int32_t status, int32_t sig);
    void RegisterAppStateObserver(const std::string &bundleName, pid_t callerPid);
    void OnProcessDied(const std::string &bundleName, pid_t diedPid);
    void AddCallerPid(int64_t pid);
    void RemoveCallerPid(int64_t pid);
    int32_t GetCallerPidCount();

    bool IsDeveloperMode() const;
    sptr<ICliHookInterface> CheckCliHook(uint32_t flag);
    sptr<IFunctionHookInterface> CheckFunctionHook(uint32_t flag);
    void InvokeBeforeCallTool(ExecToolParam &param);
    void InvokeAfterCallTool(CliSessionInfo &session, const SessionRecord &record);
    void InvokeBeforeCallCmd(ExecCmdParam &param);

    bool initialized_ = false;
    bool isDeveloperMode_ = false;
    // ioMonitor_ is shared between OnStop (writes nullptr on unload) and in-flight IPC/ffrt paths
    // (register/unregister/send); every access after Init must go through GetIOMonitor() or hold
    // ioMonitorMutex_. Init() itself runs before Publish(), single-threaded, and may assign directly.
    ffrt::mutex ioMonitorMutex_;
    std::shared_ptr<IOMonitor> ioMonitor_ = nullptr;

    // ---- SIGCHLD self-pipe + dedicated reaper thread ----
    // reapPipeWriteFd_ is static+atomic so the static signal handler can reach it directly WITHOUT calling
    // GetInstance() (which locks the non-recursive g_mutex and would re-enter/deadlock from signal context).
    static std::atomic<int> reapPipeWriteFd_;
    int reapPipeReadFd_ = -1;                 // touched only by the reaper thread + Init/StopReaper
    std::thread reaperThread_;
    std::atomic<bool> reaperRunning_{false};
    std::atomic<bool> reaperStarted_{false};  // start-guard, mirrors IOMonitor::Start running_.exchange(true)
    static constexpr int32_t REAP_POLL_TIMEOUT_MS = 100;  // bounds lost-wake recovery, NOT reaping latency

    std::atomic<int32_t> interfaceCalledCount_ = 0;
    ffrt::mutex sessionsMutex_;
    std::unordered_map<std::string, std::shared_ptr<SessionRecord>> sessionRecords_;
    ffrt::mutex observerMutex_;
    std::unordered_map<std::string, sptr<AppExecFwk::IApplicationStateObserver>> bundleObservers_;
    ffrt::mutex callerPidsMutex_;
    std::unordered_set<int64_t> callerPids;

    ffrt::mutex hookMutex_;
    sptr<ICliHookInterface> cliHook_;
    sptr<IFunctionHookInterface> functionHook_;
    sptr<IRemoteObject::DeathRecipient> cliHookDeathRecipient_;
    sptr<IRemoteObject::DeathRecipient> functionHookDeathRecipient_;

    uint32_t cliHookActiveMethods_ = 0;     // bitmask: 0 = all methods active (for non-NAPI callers)
    uint32_t functionHookActiveMethods_ = 0; // bitmask: 0 = all methods active (for non-NAPI callers)
    static constexpr uint32_t CLI_HOOK_BEFORE_CALL_TOOL = 1 << 0;
    static constexpr uint32_t CLI_HOOK_AFTER_CALL_TOOL = 1 << 1;
    static constexpr uint32_t CLI_HOOK_BEFORE_CALL_CMD = 1 << 2;
    static constexpr uint32_t CLI_HOOK_AFTER_CALL_CMD = 1 << 3;
    static constexpr uint32_t FUNC_HOOK_BEFORE_INVOKE = 1 << 0;
    static constexpr uint32_t FUNC_HOOK_AFTER_INVOKE = 1 << 1;

    static constexpr int32_t HOOK_TIMEOUT_SECONDS = 6;
    template<typename ParamType, typename HookFunc>
    bool InvokeHookAsync(HookFunc &&hookFunc, ParamType &param)
    {
        auto mtx = std::make_shared<ffrt::mutex>();
        auto cv = std::make_shared<ffrt::condition_variable>();
        auto completed = std::make_shared<std::atomic<bool>>(false);
        auto paramCopy = std::make_shared<ParamType>(param);

        ffrt::submit([hookFunc = std::forward<HookFunc>(hookFunc), paramCopy, mtx, cv, completed]() {
            hookFunc(*paramCopy);
            {
                std::lock_guard<ffrt::mutex> lock(*mtx);
                completed->store(true);
            }
            cv->notify_one();
        });

        std::unique_lock<ffrt::mutex> lock(*mtx);
        bool success = cv->wait_for(lock, std::chrono::seconds(HOOK_TIMEOUT_SECONDS),
            [&] { return completed->load(); });
        if (success) {
            param = *paramCopy;
        }
        return success;
    }
};

class SkillCallbackAdapter : public AAFwk::SkillExecuteCallbackStub {
public:
    SkillCallbackAdapter(const std::string &sessionId,
        int32_t callerPid, int32_t callerUid, const std::string &eventId);
    void OnExecuteDone(const std::string &requestCode, int32_t resultCode,
        const AppExecFwk::SkillExecuteResult &result) override;

private:
    std::string sessionId_;
    int32_t callerPid_;
    int32_t callerUid_;
    std::string eventId_;
};
} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_TOOL_MGR_SERVICE_H
