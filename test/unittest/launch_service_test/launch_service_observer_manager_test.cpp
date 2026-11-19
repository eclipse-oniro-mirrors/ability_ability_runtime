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
#include <vector>
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
 * @tc.desc: Verify duplicate registration is idempotent and one unregister removes the observer.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceObserverManagerTest, UnregisterObserver_001, TestSize.Level1)
{
    sptr<RecordingLaunchObserver> observer = sptr<RecordingLaunchObserver>::MakeSptr();
    ASSERT_EQ(manager_.RegisterObserver(observer, 0, 100), ERR_OK);
    ASSERT_EQ(manager_.RegisterObserver(observer, 0, 100), ERR_OK);
    ASSERT_EQ(manager_.UnregisterObserver(observer), ERR_OK);
    manager_.NotifyProcessStateChanged({});
    EXPECT_EQ(observer->processCount_, 0);
    EXPECT_NE(manager_.UnregisterObserver(observer), ERR_OK);
}

/**
 * @tc.name: RegisterObserver_001
 * @tc.desc: Verify the registration limit is enforced independently for each caller.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceObserverManagerTest, RegisterObserver_001, TestSize.Level1)
{
    std::vector<sptr<RecordingLaunchObserver>> observers;
    for (int i = 0; i < 10; ++i) {
        auto observer = sptr<RecordingLaunchObserver>::MakeSptr();
        ASSERT_EQ(manager_.RegisterObserver(observer, 0, 100), ERR_OK);
        observers.emplace_back(observer);
    }
    EXPECT_EQ(manager_.RegisterObserver(observers.front(), 0, 100), ERR_OK);
    EXPECT_EQ(manager_.CountObserversLocked(100), 10u);
    auto extra = sptr<RecordingLaunchObserver>::MakeSptr();
    const auto nextId = manager_.nextObserverId_;
    EXPECT_EQ(manager_.RegisterObserver(extra, 0, 100), AAFwk::ERR_LAUNCH_SERVICE_OBSERVER_LIMIT);
    EXPECT_EQ(manager_.CountObserversLocked(100), 10u);
    EXPECT_EQ(manager_.nextObserverId_, nextId);
    auto another = sptr<RecordingLaunchObserver>::MakeSptr();
    EXPECT_EQ(manager_.RegisterObserver(another, 0, 200), ERR_OK);
    ASSERT_EQ(manager_.UnregisterObserver(observers.front()), ERR_OK);
    EXPECT_EQ(manager_.RegisterObserver(extra, 0, 100), ERR_OK);
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
/**
 * @tc.name: CountAndRemoveObserver_001
 * @tc.desc: Verify distinct observer counting and explicit removal stop notification.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceObserverManagerTest, CountAndRemoveObserver_001, TestSize.Level1)
{
    auto observer = sptr<RecordingLaunchObserver>::MakeSptr();
    ASSERT_EQ(manager_.RegisterObserver(observer, 0, 100), ERR_OK);
    ASSERT_EQ(manager_.RegisterObserver(observer, 0, 100), ERR_OK);
    EXPECT_EQ(manager_.CountObserversLocked(100), 1u);
    EXPECT_EQ(manager_.CountObserversLocked(200), 0u);
    ASSERT_EQ(manager_.observers_.size(), 1u);
    const int id = manager_.observers_.begin()->first;
    auto record = manager_.observers_.begin()->second;
    manager_.RemoveObserver(id);
    EXPECT_FALSE(record->active);
    EXPECT_TRUE(manager_.observers_.empty());
    manager_.RemoveObserver(id);
    manager_.NotifyProcessStateChanged({});
    EXPECT_EQ(observer->processCount_, 0);
}

/**
 * @tc.name: AllocateObserverId_Wrap_001
 * @tc.desc: Verify ID allocation wraps safely and skips occupied IDs.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceObserverManagerTest, AllocateObserverId_Wrap_001, TestSize.Level1)
{
    manager_.nextObserverId_ = 2147483647;
    EXPECT_EQ(manager_.AllocateObserverIdLocked(), 2147483647);
    auto observer = sptr<RecordingLaunchObserver>::MakeSptr();
    ASSERT_EQ(manager_.RegisterObserver(observer, 0, 100), ERR_OK);
    manager_.nextObserverId_ = 1;
    EXPECT_EQ(manager_.AllocateObserverIdLocked(), 2);
    EXPECT_EQ(&LaunchServiceObserverManager::GetInstance(), &LaunchServiceObserverManager::GetInstance());
}


/**
 * @tc.name: ObserverDeathRecipient_001
 * @tc.desc: Remote death removes registration and stops subsequent notifications.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceObserverManagerTest, ObserverDeathRecipient_001, TestSize.Level1)
{
    auto &manager = LaunchServiceObserverManager::GetInstance();
    auto observer = sptr<RecordingLaunchObserver>::MakeSptr();
    ASSERT_EQ(manager.RegisterObserver(observer, 0, 100), ERR_OK);
    int32_t id = -1;
    for (const auto &[key, record] : manager.observers_) {
        if (record->observer.GetRefPtr() == observer.GetRefPtr()) { id = key; }
    }
    ASSERT_GT(id, 0);
    auto recipient = sptr<LaunchServiceObserverManager::ObserverDeathRecipient>::MakeSptr(id);
    recipient->OnRemoteDied({});
    EXPECT_EQ(manager.observers_.count(id), 0);
    manager.NotifyStatusChanged({});
    EXPECT_EQ(observer->statusCount_, 0);
}
} // namespace AppExecFwk
} // namespace OHOS
