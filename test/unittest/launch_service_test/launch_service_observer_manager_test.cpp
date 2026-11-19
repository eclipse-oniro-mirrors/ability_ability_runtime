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
#include <map>
#include <memory>
#include <mutex>
#include "launch_service_observer.h"
#define private public
#include "launch_service_observer_manager.h"
#undef private
#include "ability_manager_errors.h"
#include "ipc_types.h"

using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class RecordingLaunchObserver : public AbilityRuntime::LaunchServiceObserverStub {
public:
    int statusCount_ = 0;
    int processCount_ = 0;

    void OnLaunchServiceStatusChanged(const AbilityRuntime::LaunchServiceStateData &) override
    {
        ++statusCount_;
    }

    void OnLaunchServiceProcessStateChanged(const AbilityRuntime::LaunchServiceProcessStateData &) override
    {
        ++processCount_;
    }
};

class LaunchServiceObserverManagerTest : public testing::Test {
public:
    LaunchServiceObserverManager manager_;
};

/**
 * @tc.name: NotifyStatusChanged_001
 * @tc.desc: Verify the first notification is delivered and unregister stops subsequent notifications.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceObserverManagerTest, NotifyStatusChanged_001, TestSize.Level1)
{
    sptr<RecordingLaunchObserver> observer = sptr<RecordingLaunchObserver>::MakeSptr();
    ASSERT_EQ(manager_.RegisterObserver(observer, 0, 100), ERR_OK);
    AbilityRuntime::LaunchServiceStateData data;
    data.state = AbilityRuntime::LaunchServiceState::ENABLED;
    manager_.NotifyStatusChanged(data);
    EXPECT_EQ(observer->statusCount_, 1);
    ASSERT_EQ(manager_.UnregisterObserver(observer), ERR_OK);
    manager_.NotifyStatusChanged(data);
    EXPECT_EQ(observer->statusCount_, 1);
}

/**
 * @tc.name: UnregisterObserver_001
 * @tc.desc: Verify duplicate registrations are removed by reference count.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceObserverManagerTest, UnregisterObserver_001, TestSize.Level1)
{
    sptr<RecordingLaunchObserver> observer = sptr<RecordingLaunchObserver>::MakeSptr();
    ASSERT_EQ(manager_.RegisterObserver(observer, 0, 100), ERR_OK);
    ASSERT_EQ(manager_.RegisterObserver(observer, 0, 100), ERR_OK);
    ASSERT_EQ(manager_.UnregisterObserver(observer), ERR_OK);
    manager_.NotifyProcessStateChanged({});
    EXPECT_EQ(observer->processCount_, 1);
    ASSERT_EQ(manager_.UnregisterObserver(observer), ERR_OK);
    manager_.NotifyProcessStateChanged({});
    EXPECT_EQ(observer->processCount_, 1);
    EXPECT_NE(manager_.UnregisterObserver(observer), ERR_OK);
}

/**
 * @tc.name: RegisterObserver_001
 * @tc.desc: Verify the registration limit is enforced independently for each caller.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceObserverManagerTest, RegisterObserver_001, TestSize.Level1)
{
    sptr<RecordingLaunchObserver> observer = sptr<RecordingLaunchObserver>::MakeSptr();
    for (int i = 0; i < 10; ++i) {
        ASSERT_EQ(manager_.RegisterObserver(observer, 0, 100), ERR_OK);
    }
    EXPECT_EQ(manager_.RegisterObserver(observer, 0, 100), AAFwk::ERR_LAUNCH_SERVICE_OBSERVER_LIMIT);
    sptr<RecordingLaunchObserver> another = sptr<RecordingLaunchObserver>::MakeSptr();
    EXPECT_EQ(manager_.RegisterObserver(another, 0, 200), ERR_OK);
    ASSERT_EQ(manager_.UnregisterObserver(observer), ERR_OK);
    EXPECT_EQ(manager_.RegisterObserver(observer, 0, 100), ERR_OK);
}

/**
 * @tc.name: RegisterObserver_002
 * @tc.desc: Verify the same remote observer cannot register under a different caller.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceObserverManagerTest, RegisterObserver_002, TestSize.Level1)
{
    sptr<RecordingLaunchObserver> observer = sptr<RecordingLaunchObserver>::MakeSptr();
    ASSERT_EQ(manager_.RegisterObserver(observer, 0, 100), ERR_OK);
    EXPECT_EQ(manager_.RegisterObserver(observer, 0, 200), ERR_INVALID_VALUE);
}

/**
 * @tc.name: RegisterObserver_003
 * @tc.desc: Verify registering and unregistering a null observer are rejected.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceObserverManagerTest, RegisterObserver_003, TestSize.Level1)
{
    EXPECT_EQ(manager_.RegisterObserver(nullptr, 0, 100), ERR_INVALID_VALUE);
    EXPECT_EQ(manager_.UnregisterObserver(nullptr), ERR_INVALID_VALUE);
}
} // namespace AppExecFwk
} // namespace OHOS
