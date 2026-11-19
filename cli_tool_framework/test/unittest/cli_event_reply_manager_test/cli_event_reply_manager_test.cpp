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

using namespace testing::ext;

namespace OHOS {
namespace CliTool {
namespace {
constexpr int32_t ERR_OK = 0;
constexpr int32_t ERROR_CODE = -1;
constexpr int32_t TEST_RESULT_CODE = 1001;
}

class CliEventReplyManagerTest : public testing::Test {
public:
    void TearDown() override
    {
        CliEventReplyManager::GetInstance().ClearAllEvent();
    }
};

/**
 * @tc.name: CliEventReplyManager_0100
 * @tc.desc: Test reply manager active, deferred, missing, null callback and remove branches
 * @tc.type: FUNC
 */
HWTEST_F(CliEventReplyManagerTest, CliEventReplyManager_0100, TestSize.Level1)
{
    auto &manager = CliEventReplyManager::GetInstance();
    int32_t callbackCount = 0;
    int32_t callbackCode = 0;
    std::string activeEventId = manager.AddEventReplyCallback("active-", [&](const CliEventReplyResult &result) {
        callbackCount++;
        callbackCode = result.code;
    });

    manager.ActivateEventReplyCallback(activeEventId);
    EXPECT_EQ(manager.HandleEventReply(activeEventId, CliEventReplyResult {TEST_RESULT_CODE, std::nullopt}), ERR_OK);
    EXPECT_EQ(callbackCount, 1);
    EXPECT_EQ(callbackCode, TEST_RESULT_CODE);
    EXPECT_EQ(manager.HandleEventReply(activeEventId, CliEventReplyResult {TEST_RESULT_CODE, std::nullopt}),
        ERROR_CODE);

    std::optional<CliSessionInfo> deferredSession;
    std::string deferredEventId = manager.AddEventReplyCallback("deferred-", [&](const CliEventReplyResult &result) {
        callbackCount++;
        deferredSession = result.sessionInfo;
    });
    CliSessionInfo session;
    session.sessionId = "session";
    session.toolName = "tool";
    session.status = "running";
    EXPECT_EQ(manager.HandleEventReply(deferredEventId, CliEventReplyResult {ERR_OK, session}), ERR_OK);
    EXPECT_EQ(callbackCount, 1);
    manager.ActivateEventReplyCallback(deferredEventId);
    EXPECT_EQ(callbackCount, 2);
    ASSERT_TRUE(deferredSession.has_value());
    EXPECT_EQ(deferredSession->sessionId, "session");

    std::string nullEventId = manager.AddEventReplyCallback("null-", nullptr);
    manager.ActivateEventReplyCallback(nullEventId);
    EXPECT_EQ(manager.HandleEventReply(nullEventId, CliEventReplyResult {ERR_OK, std::nullopt}), ERROR_CODE);

    std::string removedEventId = manager.AddEventReplyCallback("removed-", [&](const CliEventReplyResult &) {});
    manager.RemoveEventReplyCallback(removedEventId);
    EXPECT_EQ(manager.HandleEventReply(removedEventId, CliEventReplyResult {ERR_OK, std::nullopt}), ERROR_CODE);

    manager.ActivateEventReplyCallback("missing-event");
    std::string inactiveEventId = manager.AddEventReplyCallback("inactive-", [&](const CliEventReplyResult &) {
        callbackCount++;
    });
    manager.ActivateEventReplyCallback(inactiveEventId);
    EXPECT_EQ(callbackCount, 2);
    EXPECT_EQ(manager.HandleEventReply(inactiveEventId, CliEventReplyResult {ERR_OK, std::nullopt}), ERR_OK);
    EXPECT_EQ(callbackCount, 3);
}
} // namespace CliTool
} // namespace OHOS
