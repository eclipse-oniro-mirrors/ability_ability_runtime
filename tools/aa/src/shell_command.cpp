/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "shell_command.h"

#include <getopt.h>
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
ShellCommand::ShellCommand(int argc, char* argv[], std::string name)
{
    TAG_LOGD(AAFwkTag::AA_TOOL, "start");
    opterr = 0;
    argc_ = argc;
    argv_ = argv;
    name_ = name;

    if (argc < MIN_ARGUMENT_NUMBER || argc > MAX_ARGUMENT_NUMBER) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "Invalid argument count: %{public}d", argc);
        cmd_ = "help";
        return;
    }
    if (argv == nullptr) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "null argv");
        return;
    }
    cmd_ = argv[1];
    for (int i = 2; i < argc; i++) {
        argList_.push_back(argv[i]);
    }
    TAG_LOGD(AAFwkTag::AA_TOOL, "exit");
}

ShellCommand::~ShellCommand()
{}

ErrCode ShellCommand::OnCommand()
{
    TAG_LOGD(AAFwkTag::AA_TOOL, "start");
    int result = OHOS::ERR_OK;

    auto respond = commandMap_[cmd_];
    if (respond == nullptr) {
        resultReceiver_.append(GetCommandErrorMsg());
        respond = commandMap_["help"];
    }

    if (init() == OHOS::ERR_OK) {
        respond();
    } else {
        result = OHOS::ERR_INVALID_VALUE;
    }

    TAG_LOGD(AAFwkTag::AA_TOOL, "end");
    return result;
}

std::string ShellCommand::ExecCommand()
{
    TAG_LOGD(AAFwkTag::AA_TOOL, "start");
    int result = CreateCommandMap();
    if (result != OHOS::ERR_OK) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "CreateCommandMap failed\n");
    }

    result = CreateMessageMap();
    if (result != OHOS::ERR_OK) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "CreateMessageMap failed\n");
    }

    result = OnCommand();
    if (result != OHOS::ERR_OK) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "failed to execute your command\n");

        resultReceiver_ = "error: failed to execute your command.\n";
    }

    TAG_LOGD(AAFwkTag::AA_TOOL, "end");
    return resultReceiver_;
}

std::string ShellCommand::GetCommandErrorMsg() const
{
    TAG_LOGD(AAFwkTag::AA_TOOL, "start");
    std::string commandErrorMsg =
        name_ + ": '" + cmd_ + "' is not a valid " + name_ + " command. See '" + name_ + " help'.\n";

    return commandErrorMsg;
}

std::string ShellCommand::GetUnknownOptionMsg(std::string& unknownOption) const
{
    TAG_LOGD(AAFwkTag::AA_TOOL, "start");
    std::string result = "";

    if (optind < 0 || optind > argc_) {
        return result;
    }

    result.append("fail: unknown option");
    result.append(".\n");

    return result;
}

std::string ShellCommand::GetMessageFromCode(const int32_t code) const
{
    TAG_LOGI(AAFwkTag::AA_TOOL, "code = %{public}d", code);

    std::string result = "";
    if (messageMap_.find(code) != messageMap_.end()) {
        std::string message = messageMap_.at(code);
        if (message.size() != 0) {
            result.append(message + "\n");
        }
    }

    TAG_LOGI(AAFwkTag::AA_TOOL, "result: %{public}s", result.c_str());

    return result;
}
}  // namespace AAFwk
}  // namespace OHOS
