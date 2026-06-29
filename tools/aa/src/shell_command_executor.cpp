/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "shell_command_executor.h"

#include <chrono>
#include <cinttypes>
#include <iostream>
#include <sstream>

#include "hilog_tag_wrapper.h"
#include "shell_command_config_loader.h"

using namespace std::chrono_literals;
namespace OHOS {
namespace AAFwk {
ShellCommandExecutor::ShellCommandExecutor(const std::string& cmd, const int64_t timeoutSec)
    : timeoutSec_(timeoutSec), cmd_(cmd)
{
    handler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::Create());
}

ShellCommandResult ShellCommandExecutor::WaitWorkDone()
{
    TAG_LOGD(AAFwkTag::AA_TOOL, "enter");

    if (!DoWork()) {
        TAG_LOGI(AAFwkTag::AA_TOOL, "Failed cmd: \"%{public}s\"", cmd_.data());
        return cmdResult_;
    }

    std::unique_lock<std::mutex> workLock(mtxWork_);

    auto condition = [this]() { return isDone_; };
    if (timeoutSec_ <= 0) {
        cvWork_.wait(workLock, condition);
    } else if (!cvWork_.wait_for(workLock, timeoutSec_ * 1s, condition)) {
        TAG_LOGW(AAFwkTag::AA_TOOL, "cmd timed out! cmd : \"%{public}s\", timeoutSec : %{public}" PRId64,
            cmd_.data(), timeoutSec_);
        std::cout << "Warning! Command execution timed out! cmd : " << cmd_ << ", timeoutSec : " << timeoutSec_
            << std::endl;

        ShellCommandResult realResult;
        realResult.exitCode = -1;
        {
            std::lock_guard<std::mutex> copyLock(mtxCopy_);
            realResult.stdResult = cmdResult_.stdResult;
        }
        return realResult;
    }

    TAG_LOGI(AAFwkTag::AA_TOOL, "cmd complete, cmd : \"%{public}s\", exitCode : %{public}d",
        cmd_.data(), cmdResult_.exitCode);
    return cmdResult_;
}

bool ShellCommandExecutor::DoWork()
{
    TAG_LOGD(AAFwkTag::AA_TOOL, "enter");

    if (cmd_.empty()) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "Invalid cmd_");
        return false;
    }

    if (!handler_) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "Invalid handler_");
        return false;
    }
    
    if (!CheckCommand()) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "Invalid command");
        return false;
    }

    auto self(shared_from_this());
    handler_->PostTask([this, self]() {
        TAG_LOGI(AAFwkTag::AA_TOOL, "DoWork task begin, cmd: \"%{public}s\"", cmd_.data());

        FILE* file = popen(cmd_.c_str(), "r");
        if (!file) {
            TAG_LOGE(AAFwkTag::AA_TOOL, "popen failed, cmd: \"%{public}s\"", cmd_.data());

            {
                std::unique_lock<std::mutex> workLock(mtxWork_);
                isDone_ = true;
            }
            cvWork_.notify_one();
            TAG_LOGI(AAFwkTag::AA_TOOL, "DoWork task end, cmd: \"%{public}s\"", cmd_.data());
            return;
        }

        char commandResult[1024] = { '\0' };
        while ((fgets(commandResult, sizeof(commandResult), file)) != nullptr) {
            {
                std::lock_guard<std::mutex> copyLock(mtxCopy_);
                cmdResult_.stdResult.append(commandResult);
            }
            std::cout << commandResult;
        }

        cmdResult_.exitCode = pclose(file);
        file = nullptr;

        {
            std::unique_lock<std::mutex> workLock(mtxWork_);
            isDone_ = true;
        }
        cvWork_.notify_one();
        TAG_LOGI(AAFwkTag::AA_TOOL, "DoWork task end, cmd: \"%{public}s\"", cmd_.data());
    });

    return true;
}

bool ShellCommandExecutor::CheckCommand()
{
    std::istringstream iss(cmd_);
    std::string firstCommand = "";
    iss >> firstCommand;
    if (ShellCommandConfigLoader::commands_.find(firstCommand) != ShellCommandConfigLoader::commands_.end()) {
        return true;
    }
    return false;
}
}  // namespace AAFwk
}  // namespace OHOS
