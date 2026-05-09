/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_SHELL_COMMAND_H
#define OHOS_ABILITY_RUNTIME_SHELL_COMMAND_H

#include <map>
#include <sstream>
#include <string>
#include <functional>
#include <vector>

#include "errors.h"

namespace OHOS {
namespace AAFwk {
namespace {
const std::string HELP_MSG_NO_OPTION = "error: you must specify an option at least.";

const int OFFSET_REQUIRED_ARGUMENT = 2;
}  // namespace

struct AaToolErrorInfo;

class ShellCommand {
public:
    ShellCommand(int argc, char* argv[], std::string name);
    virtual ~ShellCommand();

    ErrCode OnCommand();
    std::string ExecCommand();
    std::string GetCommandErrorMsg() const;
    std::string GetUnknownOptionMsg(std::string& unknownOption) const;
    std::string GetMessageFromCode(const int32_t code) const;

    virtual ErrCode CreateCommandMap() = 0;
    virtual ErrCode CreateMessageMap() = 0;
    virtual ErrCode init() = 0;

protected:
    static constexpr int MIN_ARGUMENT_NUMBER = 2;
    static constexpr int MAX_ARGUMENT_NUMBER = 4096;

    char** argv_ = nullptr;
    int argc_ = 0;
    int64_t startTime_ = 0;
    std::string resultReceiver_ = "";

    std::string cmd_;
    std::vector<std::string> argList_;

    std::string name_;
    std::map<std::string, std::function<int()>> commandMap_;
    std::map<int32_t, std::string> messageMap_;
};

struct AaToolErrorInfo {
    std::string code;
    std::string message;
    std::string cause;
    std::vector<std::string> solutions;

    std::string ToString()
    {
        std::ostringstream oss;

        oss << "Error Code:" << code << "  Error Message:" << message << "\n";
        oss << "Error cause: " << cause << "\n";
        oss << "  Try the following:\n";
        for (const auto& solution : solutions) {
            oss << "  > " << solution << "\n";
        }
        return oss.str();
    }
    
    std::string SolutionsToString() const
    {
        std::ostringstream oss;
        for (const auto& solution : solutions) {
            oss << solution << "\n";
        }
        return oss.str();
    }
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_SHELL_COMMAND_H
