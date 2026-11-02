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

#define private public
#define protected public
#include "focused_application_info.h"
#include "focused_application_observer_stub.h"
#include "app_state_observer_manager.h"
#include "mock_native_token.h"
#include "mock_my_flag.h"
#include "mock_permission_verification.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace AppExecFwk {
using OHOS::AAFwk::MyFlag;

namespace {
constexpr int32_t TEST_ERR_FOCUS_OBSERVER_EXCEED_LIMIT = 16000165;
constexpr int32_t FOCUSED_APPLICATION_OBSERVER_LIMIT = 10;
constexpr int32_t FOCUSED_UID = 10001;
const std::string TEST_BUNDLE_NAME = "com.test.focused";
const std::string EMPTY_BUNDLE_NAME = "";
}  // namespace

class MockFocusedApplicationObserver : public FocusedApplicationObserverStub {
public:
    MockFocusedApplicationObserver() = default;
    ~MockFocusedApplicationObserver() override = default;

    void OnFocusedApplicationChanged(const FocusedApplicationInfo &focusedApplicationInfo) override
    {
        called_ = true;
        focusedUid_ = focusedApplicationInfo.focusedUid;
        focusedBundleName_ = focusedApplicationInfo.focusedBundleName;
    }

    bool IsCalled() const { return called_; }
    int32_t GetFocusedUid() const { return focusedUid_; }
    const std::string &GetFocusedBundleName() const { return focusedBundleName_; }

private:
    bool called_ = false;
    int32_t focusedUid_ = -1;
    std::string focusedBundleName_;
};

class AppStateObserverManagerFocusedAppTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::shared_ptr<AppStateObserverManager> manager_;
};

void AppStateObserverManagerFocusedAppTest::SetUpTestCase()
{
    MockNativeToken::SetNativeToken();
}

void AppStateObserverManagerFocusedAppTest::TearDownTestCase() {}

void AppStateObserverManagerFocusedAppTest::SetUp()
{
    MyFlag::flag_ = 1;
    manager_ = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager_, nullptr);
}

void AppStateObserverManagerFocusedAppTest::TearDown()
{
    if (manager_) {
        std::lock_guard<ffrt::mutex> lock(manager_->focusedApplicationObserverLock_);
        manager_->focusedApplicationObserverMap_.clear();
    }
    manager_.reset();
    MyFlag::flag_ = 1;
}

/**
 * @tc.number: RegisterFocusedApplicationObserver_0100
 * @tc.desc: AC-1 register null observer returns ERR_INVALID_VALUE.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusedAppTest, RegisterFocusedApplicationObserver_0100, TestSize.Level1)
{
    EXPECT_NE(manager_, nullptr);
    auto ret = manager_->RegisterFocusedApplicationObserver(nullptr);
    EXPECT_EQ(ERR_INVALID_VALUE, ret);
}

/**
 * @tc.number: RegisterFocusedApplicationObserver_0200
 * @tc.desc: AC-1 register valid observer returns ERR_OK and map contains it.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusedAppTest, RegisterFocusedApplicationObserver_0200, TestSize.Level1)
{
    EXPECT_NE(manager_, nullptr);
    auto observer = sptr<MockFocusedApplicationObserver>(new MockFocusedApplicationObserver());
    ASSERT_NE(observer, nullptr);
    auto ret = manager_->RegisterFocusedApplicationObserver(observer);
    EXPECT_EQ(ERR_OK, ret);
    EXPECT_EQ(1, static_cast<int32_t>(manager_->focusedApplicationObserverMap_.size()));
    manager_->UnregisterFocusedApplicationObserver(observer);
}

/**
 * @tc.number: RegisterFocusedApplicationObserver_0300
 * @tc.desc: AC-14 register same observer twice returns error.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusedAppTest, RegisterFocusedApplicationObserver_0300, TestSize.Level1)
{
    EXPECT_NE(manager_, nullptr);
    auto observer = sptr<MockFocusedApplicationObserver>(new MockFocusedApplicationObserver());
    ASSERT_NE(observer, nullptr);
    auto ret = manager_->RegisterFocusedApplicationObserver(observer);
    EXPECT_EQ(ERR_OK, ret);
    auto ret2 = manager_->RegisterFocusedApplicationObserver(observer);
    EXPECT_EQ(ERR_INVALID_VALUE, ret2);
    manager_->UnregisterFocusedApplicationObserver(observer);
}

/**
 * @tc.number: RegisterFocusedApplicationObserver_0400
 * @tc.desc: AC-14 register 11 observers (no upper limit now), all should succeed.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusedAppTest, RegisterFocusedApplicationObserver_0400, TestSize.Level1)
{
    EXPECT_NE(manager_, nullptr);
    std::vector<sptr<MockFocusedApplicationObserver>> observers;
    for (int32_t i = 0; i < FOCUSED_APPLICATION_OBSERVER_LIMIT; ++i) {
        auto observer = sptr<MockFocusedApplicationObserver>(new MockFocusedApplicationObserver());
        ASSERT_NE(observer, nullptr);
        auto ret = manager_->RegisterFocusedApplicationObserver(observer);
        EXPECT_EQ(ERR_OK, ret) << "observer index: " << i;
        observers.push_back(observer);
    }
    auto observer = sptr<MockFocusedApplicationObserver>(new MockFocusedApplicationObserver());
    ASSERT_NE(observer, nullptr);
    auto ret = manager_->RegisterFocusedApplicationObserver(observer);
    EXPECT_EQ(TEST_ERR_FOCUS_OBSERVER_EXCEED_LIMIT, ret);
    for (auto &obs : observers) {
        manager_->UnregisterFocusedApplicationObserver(obs);
    }
}

/**
 * @tc.number: RegisterFocusedApplicationObserver_0600
 * @tc.desc: AC-14 unregister one observer then re-register should succeed (count recovered).
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusedAppTest, RegisterFocusedApplicationObserver_0600, TestSize.Level1)
{
    EXPECT_NE(manager_, nullptr);
    std::vector<sptr<MockFocusedApplicationObserver>> observers;
    for (int32_t i = 0; i < FOCUSED_APPLICATION_OBSERVER_LIMIT; ++i) {
        auto observer = sptr<MockFocusedApplicationObserver>(new MockFocusedApplicationObserver());
        ASSERT_NE(observer, nullptr);
        EXPECT_EQ(ERR_OK, manager_->RegisterFocusedApplicationObserver(observer));
        observers.push_back(observer);
    }
    manager_->UnregisterFocusedApplicationObserver(observers[0]);
    observers.erase(observers.begin());
    auto observer = sptr<MockFocusedApplicationObserver>(new MockFocusedApplicationObserver());
    ASSERT_NE(observer, nullptr);
    EXPECT_EQ(ERR_OK, manager_->RegisterFocusedApplicationObserver(observer));
    observers.push_back(observer);
    for (auto &obs : observers) {
        manager_->UnregisterFocusedApplicationObserver(obs);
    }
}

/**
 * @tc.number: RegisterFocusedApplicationObserver_0500
 * @tc.desc: AC-2 no permission, register returns ERR_PERMISSION_DENIED, observer not in map.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusedAppTest, RegisterFocusedApplicationObserver_0500, TestSize.Level1)
{
    EXPECT_NE(manager_, nullptr);
    MyFlag::flag_ = 0;
    auto observer = sptr<MockFocusedApplicationObserver>(new MockFocusedApplicationObserver());
    ASSERT_NE(observer, nullptr);
    auto ret = manager_->RegisterFocusedApplicationObserver(observer);
    EXPECT_EQ(ERR_PERMISSION_DENIED, ret);
    EXPECT_EQ(0, static_cast<int32_t>(manager_->focusedApplicationObserverMap_.size()));
    MyFlag::flag_ = 1;
}

/**
 * @tc.number: UnregisterFocusedApplicationObserver_0100
 * @tc.desc: AC-6 unregister null returns ERR_INVALID_VALUE.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusedAppTest, UnregisterFocusedApplicationObserver_0100, TestSize.Level1)
{
    EXPECT_NE(manager_, nullptr);
    auto ret = manager_->UnregisterFocusedApplicationObserver(nullptr);
    EXPECT_EQ(ERR_INVALID_VALUE, ret);
}

/**
 * @tc.number: UnregisterFocusedApplicationObserver_0200
 * @tc.desc: AC-6 unregister valid observer succeeds, map empty.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusedAppTest, UnregisterFocusedApplicationObserver_0200, TestSize.Level1)
{
    EXPECT_NE(manager_, nullptr);
    auto observer = sptr<MockFocusedApplicationObserver>(new MockFocusedApplicationObserver());
    ASSERT_NE(observer, nullptr);
    auto ret = manager_->RegisterFocusedApplicationObserver(observer);
    EXPECT_EQ(ERR_OK, ret);
    auto ret2 = manager_->UnregisterFocusedApplicationObserver(observer);
    EXPECT_EQ(ERR_OK, ret2);
    EXPECT_EQ(0, static_cast<int32_t>(manager_->focusedApplicationObserverMap_.size()));
}

/**
 * @tc.number: UnregisterFocusedApplicationObserver_0300
 * @tc.desc: AC-6 unregister non-existent observer returns ERR_INVALID_VALUE.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusedAppTest, UnregisterFocusedApplicationObserver_0300, TestSize.Level1)
{
    EXPECT_NE(manager_, nullptr);
    auto observer = sptr<MockFocusedApplicationObserver>(new MockFocusedApplicationObserver());
    ASSERT_NE(observer, nullptr);
    auto ret = manager_->UnregisterFocusedApplicationObserver(observer);
    EXPECT_EQ(ERR_INVALID_VALUE, ret);
}

/**
 * @tc.number: HandleNotifyFocusedApplicationChanged_0100
 * @tc.desc: AC-4 register observer, call HandleNotifyFocusedApplicationChanged, verify callback.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusedAppTest, HandleNotifyFocusedApplicationChanged_0100, TestSize.Level1)
{
    EXPECT_NE(manager_, nullptr);
    auto observer = sptr<MockFocusedApplicationObserver>(new MockFocusedApplicationObserver());
    ASSERT_NE(observer, nullptr);
    auto ret = manager_->RegisterFocusedApplicationObserver(observer);
    EXPECT_EQ(ERR_OK, ret);

    FocusedApplicationInfo data;
    data.focusedUid = FOCUSED_UID;
    data.focusedBundleName = TEST_BUNDLE_NAME;
    manager_->HandleNotifyFocusedApplicationChanged(data);

    EXPECT_TRUE(observer->IsCalled());
    EXPECT_EQ(FOCUSED_UID, observer->GetFocusedUid());
    EXPECT_EQ(TEST_BUNDLE_NAME, observer->GetFocusedBundleName());

    manager_->UnregisterFocusedApplicationObserver(observer);
}

/**
 * @tc.number: HandleNotifyFocusedApplicationChanged_0200
 * @tc.desc: AC-4 call with no observers registered, no crash.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusedAppTest, HandleNotifyFocusedApplicationChanged_0200, TestSize.Level1)
{
    EXPECT_NE(manager_, nullptr);
    FocusedApplicationInfo data;
    data.focusedUid = FOCUSED_UID;
    data.focusedBundleName = TEST_BUNDLE_NAME;
    manager_->HandleNotifyFocusedApplicationChanged(data);
    EXPECT_EQ(0, static_cast<int32_t>(manager_->focusedApplicationObserverMap_.size()));
}

/**
 * @tc.number: HandleNotifyFocusedApplicationChanged_0300
 * @tc.desc: AC-17 call with empty focusedBundleName, verify observer received it.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusedAppTest, HandleNotifyFocusedApplicationChanged_0300, TestSize.Level1)
{
    EXPECT_NE(manager_, nullptr);
    auto observer = sptr<MockFocusedApplicationObserver>(new MockFocusedApplicationObserver());
    ASSERT_NE(observer, nullptr);
    auto ret = manager_->RegisterFocusedApplicationObserver(observer);
    EXPECT_EQ(ERR_OK, ret);

    FocusedApplicationInfo data;
    data.focusedUid = FOCUSED_UID;
    data.focusedBundleName = EMPTY_BUNDLE_NAME;
    manager_->HandleNotifyFocusedApplicationChanged(data);

    EXPECT_TRUE(observer->IsCalled());
    EXPECT_EQ(FOCUSED_UID, observer->GetFocusedUid());
    EXPECT_EQ(EMPTY_BUNDLE_NAME, observer->GetFocusedBundleName());

    manager_->UnregisterFocusedApplicationObserver(observer);
}

/**
 * @tc.number: HandleNotifyFocusedApplicationChanged_0400
 * @tc.desc: AC-13 register 2 observers, notify, verify both receive callback.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusedAppTest, HandleNotifyFocusedApplicationChanged_0400, TestSize.Level1)
{
    EXPECT_NE(manager_, nullptr);
    auto observer1 = sptr<MockFocusedApplicationObserver>(new MockFocusedApplicationObserver());
    auto observer2 = sptr<MockFocusedApplicationObserver>(new MockFocusedApplicationObserver());
    ASSERT_NE(observer1, nullptr);
    ASSERT_NE(observer2, nullptr);
    EXPECT_EQ(ERR_OK, manager_->RegisterFocusedApplicationObserver(observer1));
    EXPECT_EQ(ERR_OK, manager_->RegisterFocusedApplicationObserver(observer2));

    FocusedApplicationInfo data;
    data.focusedUid = FOCUSED_UID;
    data.focusedBundleName = TEST_BUNDLE_NAME;
    manager_->HandleNotifyFocusedApplicationChanged(data);

    EXPECT_TRUE(observer1->IsCalled());
    EXPECT_EQ(FOCUSED_UID, observer1->GetFocusedUid());
    EXPECT_EQ(TEST_BUNDLE_NAME, observer1->GetFocusedBundleName());
    EXPECT_TRUE(observer2->IsCalled());
    EXPECT_EQ(FOCUSED_UID, observer2->GetFocusedUid());
    EXPECT_EQ(TEST_BUNDLE_NAME, observer2->GetFocusedBundleName());

    manager_->UnregisterFocusedApplicationObserver(observer1);
    manager_->UnregisterFocusedApplicationObserver(observer2);
}

/**
 * @tc.number: OnObserverDied_0100
 * @tc.desc: AC-8 register observer, simulate death, verify removed from map.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusedAppTest, OnObserverDied_0100, TestSize.Level1)
{
    EXPECT_NE(manager_, nullptr);
    auto observer = sptr<MockFocusedApplicationObserver>(new MockFocusedApplicationObserver());
    ASSERT_NE(observer, nullptr);
    auto ret = manager_->RegisterFocusedApplicationObserver(observer);
    EXPECT_EQ(ERR_OK, ret);

    manager_->OnObserverDied(observer->AsObject(), ObserverType::FOCUSED_APPLICATION_OBSERVER);

    EXPECT_EQ(0, static_cast<int32_t>(manager_->focusedApplicationObserverMap_.size()));
}

/**
 * @tc.number: OnObserverDied_0200
 * @tc.desc: AC-8 register 2 observers, kill one, verify other still in map.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusedAppTest, OnObserverDied_0200, TestSize.Level1)
{
    EXPECT_NE(manager_, nullptr);
    auto observer1 = sptr<MockFocusedApplicationObserver>(new MockFocusedApplicationObserver());
    ASSERT_NE(observer1, nullptr);
    auto observer2 = sptr<MockFocusedApplicationObserver>(new MockFocusedApplicationObserver());
    ASSERT_NE(observer2, nullptr);
    auto ret1 = manager_->RegisterFocusedApplicationObserver(observer1);
    EXPECT_EQ(ERR_OK, ret1);
    auto ret2 = manager_->RegisterFocusedApplicationObserver(observer2);
    EXPECT_EQ(ERR_OK, ret2);

    manager_->OnObserverDied(observer1->AsObject(), ObserverType::FOCUSED_APPLICATION_OBSERVER);

    EXPECT_EQ(1, static_cast<int32_t>(manager_->focusedApplicationObserverMap_.size()));

    manager_->UnregisterFocusedApplicationObserver(observer2);
}

/**
 * @tc.number: UnregisterFocusedApplicationObserver_0400
 * @tc.desc: AC-6 unregister cleans death recipient from recipientMap_.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusedAppTest, UnregisterFocusedApplicationObserver_0400, TestSize.Level1)
{
    EXPECT_NE(manager_, nullptr);
    auto observer = sptr<MockFocusedApplicationObserver>(new MockFocusedApplicationObserver());
    ASSERT_NE(observer, nullptr);
    EXPECT_EQ(ERR_OK, manager_->RegisterFocusedApplicationObserver(observer));
    {
        std::lock_guard<ffrt::mutex> lock(manager_->recipientMapMutex_);
        EXPECT_TRUE(manager_->recipientMap_.find(observer->AsObject()) != manager_->recipientMap_.end());
    }
    EXPECT_EQ(ERR_OK, manager_->UnregisterFocusedApplicationObserver(observer));
    {
        std::lock_guard<ffrt::mutex> lock(manager_->recipientMapMutex_);
        EXPECT_TRUE(manager_->recipientMap_.find(observer->AsObject()) == manager_->recipientMap_.end());
    }
}

} // namespace AppExecFwk
} // namespace OHOS
