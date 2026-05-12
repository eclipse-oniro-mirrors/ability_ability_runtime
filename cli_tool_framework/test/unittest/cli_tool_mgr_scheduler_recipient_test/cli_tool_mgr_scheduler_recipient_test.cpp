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

#include <gtest/gtest.h>
#include <optional>
#include <string>

#include "cli_event_reply_manager.h"
#include "cli_session_subscription_manager.h"
#include "cli_tool_mgr_scheduler_recipient.h"

using namespace testing::ext;

namespace OHOS {
namespace CliTool {
namespace {
constexpr int32_t ERR_OK = 0;
constexpr int32_t TEST_RESULT_CODE = 1001;
}

class CliToolMgrSchedulerRecipientTest : public testing::Test {
public:
    void TearDown() override
    {
        CliEventReplyManager::GetInstance().ClearAllEvent();
        CliSessionSubscriptionManager::GetInstance().ClearAllSubscriptions();
    }
};

/**
 * @tc.name: CliToolManagerSchedulerRecipient_0100
 * @tc.desc: Test scheduler recipient forwards events to managers
 * @tc.type: FUNC
 */
HWTEST_F(CliToolMgrSchedulerRecipientTest, CliToolManagerSchedulerRecipient_0100, TestSize.Level1)
{
    CliToolManagerSchedulerRecipient recipient;

    int32_t replyCode = 0;
    std::string eventId = CliEventReplyManager::GetInstance().AddEventReplyCallback("reply-",
        [&](const CliEventReplyResult &result) {
            replyCode = result.code;
        });
    CliEventReplyManager::GetInstance().ActivateEventReplyCallback(eventId);
    EXPECT_EQ(recipient.SchedulerInputReplyEvent(eventId, TEST_RESULT_CODE), ERR_OK);
    EXPECT_EQ(replyCode, TEST_RESULT_CODE);

    std::optional<CliSessionInfo> replySession;
    std::string execEventId = CliEventReplyManager::GetInstance().AddEventReplyCallback("exec-",
        [&](const CliEventReplyResult &result) {
            replySession = result.sessionInfo;
        });
    CliEventReplyManager::GetInstance().ActivateEventReplyCallback(execEventId);
    CliSessionInfo session;
    session.sessionId = "scheduler-session";
    EXPECT_EQ(recipient.SchedulerExecToolReplyEvent(execEventId, ERR_OK, session), ERR_OK);
    ASSERT_TRUE(replySession.has_value());
    EXPECT_EQ(replySession->sessionId, "scheduler-session");

    int32_t sessionEventCount = 0;
    CliToolEvent event;
    event.type = "stdout";
    std::string subscriptionId = CliSessionSubscriptionManager::GetInstance().AddProvisionalSubscription("session",
        [&](const std::string &, const std::string &, const CliToolEvent &) {
            sessionEventCount++;
        });
    CliSessionSubscriptionManager::GetInstance().ActivateSubscription(subscriptionId);
    EXPECT_EQ(recipient.SchedulerSessionEvent("session", subscriptionId, event), ERR_OK);
    EXPECT_EQ(sessionEventCount, 1);
}
} // namespace CliTool
} // namespace OHOS
