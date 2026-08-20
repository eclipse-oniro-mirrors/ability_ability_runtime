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

#include "process_manager.h"

#include <cerrno>
#include <chrono>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include <utility>

#include "bundle_info.h"
#include "bundle_mgr_helper.h"
#include "cli_error_code.h"
#include "cli_event_report.h"
#include "exec_cmd_param.h"
#include "exec_tool_param.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "tokenid_kit.h"
#include "tool_info.h"
#include "tool_util.h"

namespace OHOS {
namespace CliTool {
namespace {
#define ACCESS_TOKENID_SET_HAP_PTOKENID _IOW('A', 0x1A, uint64_t)

// Upper bound for the fallback fd-sweep when RLIMIT_NOFILE is RLIM_INFINITY. Bounded so the
// loop stays cheap; any fd that genuinely needs to survive into claw_sandbox is dup2'd onto
// 0/1/2 before the sweep and is therefore excluded by the fd > STDERR_FILENO condition.
constexpr rlim_t MAX_FD_SWEEP_LIMIT = 65536;
}

ProcessManager &ProcessManager::GetInstance()
{
    static ProcessManager instance;
    return instance;
}

bool ProcessManager::CreatePipes(SessionRecord &record) const
{
    // Create pipes for stdin, stdout and stderr
    if (pipe2(record.stdinPipe, O_CLOEXEC) != 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to create stdin pipe: %{public}d", errno);
        return false;
    }
    if (pipe2(record.stdoutPipe, O_CLOEXEC) != 0) {
        close(record.stdinPipe[0]);
        record.stdinPipe[0] = -1;
        close(record.stdinPipe[1]);
        record.stdinPipe[1] = -1;
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to create stdout pipe: %{public}d", errno);
        return false;
    }
    if (pipe2(record.stderrPipe, O_CLOEXEC) != 0) {
        close(record.stdinPipe[0]);
        record.stdinPipe[0] = -1;
        close(record.stdinPipe[1]);
        record.stdinPipe[1] = -1;
        close(record.stdoutPipe[0]);
        record.stdoutPipe[0] = -1;
        close(record.stdoutPipe[1]);
        record.stdoutPipe[1] = -1;
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to create stderr pipe: %{public}d", errno);
        return false;
    }
    return true;
}

void ProcessManager::CloseAllPipes(SessionRecord &record) const
{
    close(record.stdinPipe[0]);
    record.stdinPipe[0] = -1;
    close(record.stdinPipe[1]);
    record.stdinPipe[1] = -1;
    close(record.stdoutPipe[0]);
    record.stdoutPipe[0] = -1;
    close(record.stdoutPipe[1]);
    record.stdoutPipe[1] = -1;
    close(record.stderrPipe[0]);
    record.stderrPipe[0] = -1;
    close(record.stderrPipe[1]);
    record.stderrPipe[1] = -1;
}

void ProcessManager::CloseNonStdFds() const
{
    // Backstop before execvp: close every inherited fd except stdin/stdout/stderr so that
    // only the three standard streams can cross the exec boundary into claw_sandbox.
    // CreatePipes already sets O_CLOEXEC on our own pipe fds; this catches any fd that
    // escaped CLOEXEC (fds inherited from the parent SA process, or opened on a path that
    // forgot the flag). Both paths below are async-signal-safe (no malloc/stdio), so they
    // cannot deadlock on a mutex inherited locked from another thread at fork().

    // Fast path: close_range (Linux 5.9+, nr 436) closes every open fd in [first, last]
    // in a single syscall -- O(open fds), no userspace iteration. OpenHarmony musl exposes
    // SYS_close_range and the standard kernel (5.10+) implements it.
    unsigned int first = static_cast<unsigned int>(STDERR_FILENO + 1);
    if (syscall(SYS_close_range, first, ~0U, 0u) == 0) {
        return;
    }
    // Fallback (ENOSYS on kernels < 5.9, or any other failure): sweep the soft NOFILE range.
    // RLIMIT_NOFILE is "the smallest fd value that cannot be opened", so every open fd is
    // strictly below rl.rlim_cur; sweeping [STDERR_FILENO+1, rl.rlim_cur) is exhaustive.
    // No TAG_LOGW here: CloseNonStdFds runs in the post-fork child where HiLog's internal
    // mutex may be held by a vanished thread -> deadlock. Sweep silently.
    struct rlimit rl;
    if (getrlimit(RLIMIT_NOFILE, &rl) != 0) {
        return;
    }
    rlim_t limit = rl.rlim_cur;
    if (limit == RLIM_INFINITY || limit > MAX_FD_SWEEP_LIMIT) {
        limit = MAX_FD_SWEEP_LIMIT;
    }
    for (rlim_t fd = STDERR_FILENO + 1; fd < limit; ++fd) {
        close(static_cast<int>(fd)); // EBADF on non-open fds, harmless
    }
}

int32_t ProcessManager::CreateChildProcess(const ExecToolParam &param, const std::string &sandboxConfig,
    const ToolInfo &toolInfo, std::shared_ptr<SessionRecord> record) const
{
    if (!CreatePipes(*record)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to create pipes");
        ReportCliExecuteFailed(record->callerBundleName, param.toolName, REASON_PROCESS_CREATE_FAILED);
        return ERR_NO_INIT;
    }
    // ---- PARENT (pre-fork): all heap allocation happens here, never in child. ----
    // vector growth + TransferToCmdParam's string concat are malloc-heavy; doing them
    // before fork means the child only reads the already-allocated buffer (.data() is a
    // pointer deref, malloc-free, async-signal-safe).
    std::vector<std::string> tmpExecArgs;
    ToolUtil::TransferToCmdParam(param.args, tmpExecArgs);

    std::vector<char*> execArgs;
    execArgs.reserve(8 + tmpExecArgs.size());  // pre-size, no growth in child
    execArgs.push_back(const_cast<char *>("/system/bin/claw_sandbox"));  // literal in .rodata, no heap
    execArgs.push_back(const_cast<char *>("--config"));
    execArgs.push_back(const_cast<char *>(sandboxConfig.c_str()));
    execArgs.push_back(const_cast<char *>("--cmd"));
    execArgs.push_back(const_cast<char *>(toolInfo.executablePath.c_str()));
    if (!param.subcommand.empty()) {
        execArgs.push_back(const_cast<char *>(param.subcommand.c_str()));
    }
    for (const auto &element : tmpExecArgs) {
        execArgs.push_back(const_cast<char *>(element.c_str()));
    }
    execArgs.push_back(nullptr);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "args:");
    for (const auto &element : tmpExecArgs) {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "%{public}s", element.c_str());
    }
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Before fork");

    pid_t pid = fork();
    if (pid < 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to fork: %{public}d", errno);
        CloseAllPipes(*record);
        ReportCliExecuteFailed(record->callerBundleName, param.toolName, REASON_PROCESS_CREATE_FAILED);
        return ERR_NO_INIT;
    }

    if (pid == 0) {
        // ---- CHILD: only async-signal-safe calls below (close/dup2/execvp/_exit/write).
        // No std::string ctor, no vector push_back, no HiLog, no malloc. ----
        close(record->stdinPipe[1]);
        close(record->stdoutPipe[0]);
        close(record->stderrPipe[0]);
        dup2(record->stdinPipe[0], STDIN_FILENO);
        dup2(record->stdoutPipe[1], STDOUT_FILENO);
        dup2(record->stderrPipe[1], STDERR_FILENO);
        close(record->stdinPipe[0]);
        close(record->stdoutPipe[1]);
        close(record->stderrPipe[1]);
        CloseNonStdFds();  // now TAG-free, safe to call from child
        execvp(execArgs[0], execArgs.data());
        // execvp only returns on failure; tell parent via stderr pipe (write is async-signal-safe).
        static const char msg[] = "claw_sandbox execvp failed\n";
        (void)write(STDERR_FILENO, msg, sizeof(msg) - 1);
        _exit(EXIT_FAILURE);
    }

    // Parent process: close write ends of pipes
    close(record->stdoutPipe[1]);
    close(record->stderrPipe[1]);

    // close read
    close(record->stdinPipe[0]);
    record->processId = pid;
    return ERR_OK;
}

int32_t ProcessManager::CreateShellProcess(const ExecCmdParam &param, const std::string &sandboxConfig,
    std::shared_ptr<SessionRecord> record) const
{
    if (!CreatePipes(*record)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to create pipes");
        return ERR_NO_INIT;
    }
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    // ---- PARENT (pre-fork): compute token flag here. AddCliBinaryInvokerTokenFlag is
    // pure bitwise OR on the happy path, but its invalid-token branch calls LOGE (HiLog,
    // not async-signal-safe); computing before fork eliminates even that edge case. ----
    auto atmTokenId = AccessToken::TokenIdKit::AddCliBinaryInvokerTokenFlag(tokenId);

    // ---- PARENT (pre-fork): build argv with heap allocation here, not in child. ----
    std::vector<char*> execArgs;
    execArgs.reserve(8);
    execArgs.push_back(const_cast<char *>("/system/bin/claw_sandbox"));
    execArgs.push_back(const_cast<char *>("--config"));
    execArgs.push_back(const_cast<char *>(sandboxConfig.c_str()));
    execArgs.push_back(const_cast<char *>("--cmd"));
    execArgs.push_back(const_cast<char *>("/bin/sh"));
    execArgs.push_back(const_cast<char *>("-c"));
    execArgs.push_back(const_cast<char *>(param.cmd.c_str()));
    execArgs.push_back(nullptr);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Before fork");
    TAG_LOGD(AAFwkTag::CLI_TOOL, "sandboxConfig: %{public}s", sandboxConfig.c_str());

    pid_t pid = fork();
    if (pid < 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to fork: %{public}d", errno);
        CloseAllPipes(*record);
        return ERR_NO_INIT;
    }
    if (pid == 0) {
        // ---- CHILD: only async-signal-safe calls (open/ioctl/close/dup2/execvp/_exit/write). ----
        // Apply parent-HAP token: open/ioctl/close are POSIX async-signal-safe; no HiLog on
        // error (HiLog mutex may be held by a vanished thread at fork -> deadlock).
        int32_t tfd = open("/dev/access_token_id", O_RDWR | O_CLOEXEC);
        if (tfd >= 0) {
            (void)ioctl(tfd, ACCESS_TOKENID_SET_HAP_PTOKENID, &atmTokenId);
            (void)close(tfd);
        }

        close(record->stdinPipe[1]);
        close(record->stdoutPipe[0]);
        close(record->stderrPipe[0]);
        dup2(record->stdinPipe[0], STDIN_FILENO);
        dup2(record->stdoutPipe[1], STDOUT_FILENO);
        dup2(record->stderrPipe[1], STDERR_FILENO);
        close(record->stdinPipe[0]);
        close(record->stdoutPipe[1]);
        close(record->stderrPipe[1]);
        CloseNonStdFds();
        execvp(execArgs[0], execArgs.data());
        static const char msg[] = "claw_sandbox execvp failed\n";
        (void)write(STDERR_FILENO, msg, sizeof(msg) - 1);
        _exit(EXIT_FAILURE);
    }
    // Parent process: close write ends of pipes
    close(record->stdoutPipe[1]);
    close(record->stderrPipe[1]);
    // close read end of stdin
    close(record->stdinPipe[0]);
    record->processId = pid;
    return ERR_OK;
}

bool ProcessManager::Killpg(pid_t pid) const
{
    if (pid <= 0) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "Killpg skipped: invalid pid=%{public}d", pid);
        return false;
    }
    int32_t killRet = kill(0 - pid, SIGKILL);
    if (killRet != 0) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "killpg result:%{public}d", killRet);
        return false;
    }
    return true;
}

} // namespace CliTool
} // namespace OHOS
