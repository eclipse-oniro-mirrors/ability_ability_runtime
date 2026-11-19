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
#include <string>
#include <vector>

#include "cli_session_subscription_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace CliTool {
namespace {
constexpr int32_t ERR_OK = 0;
constexpr int32_t ERROR_CODE = -1;
constexpr int32_t TEST_EXIT_CODE = 7;
}

class CliSessionSubscriptionManagerTest : public testing::Test {
public:
    void TearDown() override
    {
        CliSessionSubscriptionManager::GetInstance().ClearAllSubscriptions();
    }
};

/**
 * @tc.name: CliSessionSubscriptionManager_0100
 * @tc.desc: Test subscription manager active, deferred, exit, invalid and remove branches
 * @tc.type: FUNC
 */
HWTEST_F(CliSessionSubscriptionManagerTest, CliSessionSubscriptionManager_0100, TestSize.Level1)
{
    auto &manager = CliSessionSubscriptionManager::GetInstance();
    CliToolEvent stdoutEvent;
    stdoutEvent.type = "stdout";
    stdoutEvent.eventData = "hello";
    CliToolEvent exitEvent;
    exitEvent.type = "exit";
    exitEvent.exitCode = TEST_EXIT_CODE;

    int32_t callbackCount = 0;
    std::vector<std::string> eventTypes;
    std::string subscriptionId;
    subscriptionId = manager.AddProvisionalSubscription("session", [&](const std::string &sessionId,
        const std::string &callbackSubscriptionId, const CliToolEvent &event) {
        EXPECT_EQ(sessionId, "session");
        EXPECT_EQ(callbackSubscriptionId, subscriptionId);
        callbackCount++;
        eventTypes.push_back(event.type);
    });
    ASSERT_FALSE(subscriptionId.empty());
    EXPECT_EQ(manager.HandleSessionEvent("session", subscriptionId, stdoutEvent), ERR_OK);
    EXPECT_EQ(callbackCount, 0);
    manager.ActivateSubscription(subscriptionId);
    EXPECT_EQ(callbackCount, 1);
    EXPECT_EQ(eventTypes.back(), "stdout");
    EXPECT_EQ(manager.HandleSessionEvent("session", subscriptionId, exitEvent), ERR_OK);
    EXPECT_EQ(callbackCount, 2);
    EXPECT_EQ(eventTypes.back(), "exit");
    EXPECT_EQ(manager.HandleSessionEvent("session", subscriptionId, stdoutEvent), ERROR_CODE);

    EXPECT_TRUE(manager.AddProvisionalSubscription("", [&](const std::string &, const std::string &,
        const CliToolEvent &) {}).empty());
    EXPECT_TRUE(manager.AddProvisionalSubscription("session", nullptr).empty());
    EXPECT_EQ(manager.HandleSessionEvent("bad-session", "bad-subscription", stdoutEvent), ERROR_CODE);

    int32_t pendingExitCount = 0;
    std::string pendingExitId = manager.AddProvisionalSubscription("exit-session", [&](const std::string &,
        const std::string &, const CliToolEvent &) {
        pendingExitCount++;
    });
    ASSERT_FALSE(pendingExitId.empty());
    EXPECT_EQ(manager.HandleSessionEvent("exit-session", pendingExitId, stdoutEvent), ERR_OK);
    EXPECT_EQ(manager.HandleSessionEvent("exit-session", pendingExitId, exitEvent), ERR_OK);
    manager.ActivateSubscription(pendingExitId);
    EXPECT_EQ(pendingExitCount, 2);
    EXPECT_EQ(manager.HandleSessionEvent("exit-session", pendingExitId, stdoutEvent), ERROR_CODE);

    std::string removedId = manager.AddProvisionalSubscription("remove-session", [&](const std::string &,
        const std::string &, const CliToolEvent &) {});
    ASSERT_FALSE(removedId.empty());
    manager.RemoveSubscription(removedId);
    manager.ActivateSubscription(removedId);
    EXPECT_EQ(manager.HandleSessionEvent("remove-session", removedId, stdoutEvent), ERROR_CODE);
}
} // namespace CliTool
} // namespace OHOS
