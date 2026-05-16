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

#include <cstring>
#include <unistd.h>

#include "ohos_aa_command.h"
#include "xcollie/xcollie.h"
#include "xcollie/xcollie_define.h"
#include <nlohmann/json.hpp>

using namespace OHOS;
using json = nlohmann::json;
constexpr uint32_t COMMAND_TIME_OUT = 60;
const std::string EXECUTE_COMMAND_FAIL = "error: failed to execute your command.\n";

class CommandTimer {
public:
    CommandTimer(const std::string &timerName, uint32_t timeout, const std::string &operation)
    {
        if (operation != "test") {
            setTimer_ = true;
            timerId_ = HiviewDFX::XCollie::GetInstance().SetTimer("ability::claw_aa_cli_command", timeout,
                nullptr, nullptr, HiviewDFX::XCOLLIE_FLAG_LOG |  HiviewDFX::XCOLLIE_FLAG_RECOVERY);
        }
    }
    ~CommandTimer()
    {
        if (setTimer_) {
            HiviewDFX::XCollie::GetInstance().CancelTimer(timerId_);
        }
    }
private:
    bool setTimer_ = false;
    int32_t timerId_ = 0;
};

int main(int argc, char* argv[])
{
    std::string operation;
    if (argc > 1) {
        operation = argv[1];
    }

    if (strstr(argv[0], "ohos-aa") != nullptr) {
        CommandTimer commandTimer("ability::claw_aa_cli_command", COMMAND_TIME_OUT, operation);
        OHOS::AAFwk::ClawAaShellCommand cmd(argc, argv);
        cmd.CreateErrorInfoMap();
        if (cmd.ExecCommand() == EXECUTE_COMMAND_FAIL) {
            json response;
            std::string msg = EXECUTE_COMMAND_FAIL.substr(0, EXECUTE_COMMAND_FAIL.find_last_not_of("\n") + 1);
            response["type"] = "result";
            response["status"] = "failed";
            response["errCode"] = msg;
            response["errMsg"] = msg;
            response["suggestion"] = msg;
            std::cout << response.dump() << std::endl;
        }
    }
    fflush(stdout);
    _exit(0);
}