/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#include "ability_manager_client_c.h"

#include "ability_manager_client.h"
#include "ability_state.h"
#include "exit_reason.h"

int RecordAppExitReason(int exitReason, const char *exitMsg)
{
    if (exitReason < static_cast<int>(OHOS::AAFwk::Reason::REASON_MIN) ||
        exitReason > static_cast<int>(OHOS::AAFwk::Reason::REASON_MAX)) {
        return -1;
    }

    OHOS::AAFwk::Reason reason = static_cast<OHOS::AAFwk::Reason>(exitReason);
    std::string exitMsgStr = (exitMsg != nullptr) ? std::string(exitMsg) : std::string();
    OHOS::AAFwk::ExitReason exitReasonData = { reason, exitMsgStr };

    auto instance = OHOS::AAFwk::AbilityManagerClient::GetInstance();
    if (!instance) {
        return -1;
    }
    return instance->RecordAppExitReason(exitReasonData);
}

int RecordAppWithReason(int pid, int uid, int exitReason, int killId, const char *exitMsg)
{
    OHOS::AAFwk::Reason reason = static_cast<OHOS::AAFwk::Reason>(exitReason);
    std::string exitMsgStr = (exitMsg != nullptr) ? std::string(exitMsg) : std::string();
    OHOS::AAFwk::ExitReasonCompability exitReasonData = { reason, exitMsgStr };
    exitReasonData.killId = killId;

    auto instance = OHOS::AAFwk::AbilityManagerClient::GetInstance();
    if (!instance) {
        return -1;
    }
    return instance->RecordAppWithReason(pid, uid, exitReasonData);
}