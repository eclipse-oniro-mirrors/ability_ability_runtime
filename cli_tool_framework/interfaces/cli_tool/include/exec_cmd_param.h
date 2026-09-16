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

#ifndef OHOS_ABILITY_RUNTIME_EXEC_CMD_PARAM_H
#define OHOS_ABILITY_RUNTIME_EXEC_CMD_PARAM_H

#include <cstdint>
#include <string>

#include "exec_options.h"
#include "parcel.h"

namespace OHOS {
namespace CliTool {
/**
 * @brief Maximum allowed length of the command string, in bytes.
 *
 * Applies to both shell mode and tool command mode. Enforced at the NAPI entry,
 * the client entry and the service entry so that an oversized command is rejected
 * before it reaches IPC or process creation.
 */
constexpr uint32_t MAX_CMD_LENGTH = 8 * 1024;

/**
 * @brief Options for executing a raw shell command.
 *
 * Mirrors the JS API ExecCmdOptions (cliManager.d.ts): workDir/env/policy,
 * background/yieldMs/timeout, isShellCommand and challenge are all flat members.
 * env is stored as a JSON string natively and surfaced as a Record<string,string>
 * object at the NAPI boundary. callback is a JS-only field (ToolEventCallback) and
 * is not represented natively.
 */
class ExecCmdOptions : public Parcelable {
public:
    std::string workDir;
    std::string env;
    std::string policy;
    bool background = false;
    int64_t yieldMs = 0;
    int64_t timeout = 0;
    bool isShellCommand = true;
    std::string challenge;

    bool Marshalling(Parcel &parcel) const;
    static ExecCmdOptions *Unmarshalling(Parcel &parcel);

    /**
     * @brief View the background/yieldMs/timeout subset as ExecOptions so that
     * service helpers shared with the ExecTool path (RegisterSessionWithMonitors,
     * ValidateExecOptionsProperties) can consume the cmd options unchanged.
     */
    ExecOptions AsExecOptions() const
    {
        ExecOptions o;
        o.background = background;
        o.yieldMs = yieldMs;
        o.timeout = timeout;
        return o;
    }
};

/**
 * @brief Parameters for executing a raw shell command.
 *
 * Mirrors the JS API ExecCmdParam (CliHook.d.ts): { cmd, execCmdOptions }.
 */
class ExecCmdParam : public Parcelable {
public:
    std::string cmd;
    ExecCmdOptions execCmdOptions;

    bool Marshalling(Parcel &parcel) const;
    static ExecCmdParam *Unmarshalling(Parcel &parcel);
    // Extract the first whitespace-delimited token (toolName) from cmd.
    static std::string ExtractToolName(const std::string &cmd);
};
} // namespace CliTool
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_EXEC_CMD_PARAM_H
