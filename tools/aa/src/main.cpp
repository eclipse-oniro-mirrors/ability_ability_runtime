/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "ability_command.h"
#include "xcollie/xcollie.h"
#include "xcollie/xcollie_define.h"
#ifdef A11Y_ENABLE
#include "accessibility_ability_command.h"
#endif // A11Y_ENABLE

using namespace OHOS;
constexpr uint32_t COMMAND_TIME_OUT = 60;

class CommandTimer {
public:
    CommandTimer(const std::string &timerName, uint32_t timeout, const std::string &operation)
    {
        if (operation != "test") {
            setTimer_ = true;
            timerId_ = HiviewDFX::XCollie::GetInstance().SetTimer("ability::aa_command", timeout,
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

    if (argc > 0 && strstr(argv[0], "aa") != nullptr) {
        CommandTimer commandTimer("ability::aa_command", COMMAND_TIME_OUT, operation);
        OHOS::AAFwk::AbilityManagerShellCommand cmd(argc, argv);
        std::cout << cmd.ExecCommand();
    }
    fflush(stdout);
    _exit(0);
}
