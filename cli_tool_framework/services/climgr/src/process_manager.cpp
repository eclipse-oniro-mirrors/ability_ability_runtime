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
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include <utility>

#include "cli_error_code.h"
#include "exec_tool_param.h"
#include "hilog_tag_wrapper.h"
#include "tool_info.h"
#include "tool_util.h"

namespace OHOS {
namespace CliTool {

ProcessManager &ProcessManager::GetInstance()
{
    static ProcessManager instance;
    return instance;
}

bool ProcessManager::CreatePipes(SessionRecord &record) const
{
    // Create pipes for stdin, stdout and stderr
    if (pipe(record.stdinPipe) != 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to create stdin pipe: %{public}d", errno);
        return false;
    }
    if (pipe(record.stdoutPipe) != 0) {
        close(record.stdinPipe[0]);
        record.stdinPipe[0] = -1;
        close(record.stdinPipe[1]);
        record.stdinPipe[1] = -1;
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to create stdout pipe: %{public}d", errno);
        return false;
    }
    if (pipe(record.stderrPipe) != 0) {
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

void ProcessManager::CloseFatherSessionPipes(
    const std::vector<std::shared_ptr<SessionRecord>> &fatherSessionRecords) const
{
    for (const auto &fatherRecord : fatherSessionRecords) {
        if (fatherRecord == nullptr) {
            continue;
        }
        CloseAllPipes(*fatherRecord);
    }
}

int32_t ProcessManager::CreateChildProcess(const ExecToolParam &param, const std::string &sandboxConfig,
    const ToolInfo &toolInfo, std::shared_ptr<SessionRecord> record,
    const std::vector<std::shared_ptr<SessionRecord>> &fatherSessionRecords) const
{
    if (CreatePipes(*record) == false) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to create pipes");
        return ERR_NO_INIT;
    }
    pid_t pid = fork();
    if (pid < 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to fork: %{public}d", errno);
        CloseAllPipes(*record);
        return ERR_NO_INIT;
    }

    if (pid == 0) {
        close(record->stdinPipe[1]);
        close(record->stdoutPipe[0]);
        close(record->stderrPipe[0]);
        dup2(record->stdinPipe[0], STDIN_FILENO);
        dup2(record->stdoutPipe[1], STDOUT_FILENO);
        dup2(record->stderrPipe[1], STDERR_FILENO);
        close(record->stdinPipe[0]);
        close(record->stdoutPipe[1]);
        close(record->stderrPipe[1]);
        CloseFatherSessionPipes(fatherSessionRecords);

        std::string clawSandbox = "/system/bin/claw_sandbox";
        std::string configPrompt = "--config";
        std::string cmdPrompt = "--cmd";
        std::vector<char*> execArgs;
        execArgs.push_back(const_cast<char *>(clawSandbox.c_str()));
        execArgs.push_back(const_cast<char *>(configPrompt.c_str()));
        execArgs.push_back(const_cast<char *>(sandboxConfig.c_str()));
        execArgs.push_back(const_cast<char *>(cmdPrompt.c_str()));
        execArgs.push_back(const_cast<char *>(toolInfo.executablePath.c_str()));
        if (!param.subcommand.empty()) {
            execArgs.push_back(const_cast<char *>(param.subcommand.c_str()));
        }
        std::vector<std::string> tmpExecArgs;
        ToolUtil::TransferToCmdParam(param.args, tmpExecArgs);
        for (auto &element : tmpExecArgs) {
            execArgs.push_back(const_cast<char *>(element.c_str()));
        }
        execArgs.push_back(nullptr);
        TAG_LOGI(AAFwkTag::CLI_TOOL, "Before execvp");
        execvp(execArgs[0], execArgs.data());
        TAG_LOGE(AAFwkTag::CLI_TOOL, "execvp failed:%{public}d", errno);
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

bool ProcessManager::Killpg(pid_t pid) const
{
    int32_t killRet = kill(0 - pid, SIGKILL);
    if (killRet != 0) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "killpg result:%{public}d", killRet);
        return false;
    }
    return true;
}

} // namespace CliTool
} // namespace OHOS
