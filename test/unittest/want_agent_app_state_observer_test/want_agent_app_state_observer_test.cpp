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

#include "want_agent_app_state_observer.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AAFwk;

namespace OHOS {
namespace AAFwk {

class WantAgentAppStateObserverTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: OnProcessDied_Callback_0100
 * @tc.desc: OnProcessDied invokes the injected handler with bundleName and pid.
 * @tc.type: FUNC
 */
HWTEST_F(WantAgentAppStateObserverTest, OnProcessDied_Callback_0100, TestSize.Level1)
{
    std::string capturedBundle;
    pid_t capturedPid = 0;
    auto observer = sptr<WantAgentAppStateObserver>::MakeSptr(
        [&capturedBundle, &capturedPid](const std::string &bundleName, pid_t pid) {
            capturedBundle = bundleName;
            capturedPid = pid;
        });
    ASSERT_NE(observer, nullptr);

    AppExecFwk::ProcessData data;
    data.bundleName = "com.test.bundle";
    data.pid = 1234;
    observer->OnProcessDied(data);

    EXPECT_EQ(capturedBundle, "com.test.bundle");
    EXPECT_EQ(capturedPid, 1234);
}

/**
 * @tc.name: OnProcessDied_NullHandler_0100
 * @tc.desc: OnProcessDied with a null handler is a no-op and does not crash.
 * @tc.type: FUNC
 */
HWTEST_F(WantAgentAppStateObserverTest, OnProcessDied_NullHandler_0100, TestSize.Level1)
{
    auto observer = sptr<WantAgentAppStateObserver>::MakeSptr(nullptr);
    ASSERT_NE(observer, nullptr);
    AppExecFwk::ProcessData data;
    data.bundleName = "com.test.bundle";
    data.pid = 1234;

    EXPECT_NO_FATAL_FAILURE(observer->OnProcessDied(data));
}
}  // namespace AAFwk
}  // namespace OHOS