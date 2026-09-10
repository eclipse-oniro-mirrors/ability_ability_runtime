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
 * @brief Parameters for executing a raw shell command.
 */
class ExecCmdParam : public Parcelable {
public:
    std::string cmd;
    std::string workDir;
    std::string env;
    std::string policy;
    ExecOptions options;
    bool isShellCommand = true;
    std::string challenge;

    bool Marshalling(Parcel &parcel) const;
    static ExecCmdParam *Unmarshalling(Parcel &parcel);
    // Extract the first whitespace-delimited token (toolName) from cmd.
    static std::string ExtractToolName(const std::string &cmd);
};
} // namespace CliTool
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_EXEC_CMD_PARAM_H
