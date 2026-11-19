/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
#include "ability_manager_event_subscriber.h"
#undef private
#undef protected
#include "common_event_support.h"
#include "hilog_tag_wrapper.h"

using namespace testing::ext;
using namespace testing;
namespace OHOS {
namespace AbilityRuntime {
class AbilityManagerEventSubscriberTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
void AbilityManagerEventSubscriberTest::SetUpTestCase(void) {}
void AbilityManagerEventSubscriberTest::TearDownTestCase(void) {}
void AbilityManagerEventSubscriberTest::TearDown() {}
void AbilityManagerEventSubscriberTest::SetUp() {}

/**
 * @tc.name: AbilityManagerEventSubscriberTest_ScreenUnlock_OnReceiveEvent_0001
 * @tc.desc: receive other event
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerEventSubscriberTest, ScreenUnlock_OnReceiveEvent_0001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerEventSubscriberTest ScreenUnlock_OnReceiveEvent_0001 start");
    EventFwk::CommonEventSubscribeInfo subscribeInfo;
    std::function<void(int32_t)> callback = [](int32_t) {};
    EXPECT_NE(callback, nullptr);
    auto subscriber = std::make_shared<AbilityScreenUnlockEventSubscriber>(subscribeInfo, callback);
    EventFwk::CommonEventData data;
    data.want_.operation_.action_ = EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED;
    subscriber->OnReceiveEvent(data);
    EXPECT_NE(subscriber->screenUnlockCallback_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerEventSubscriberTest ScreenUnlock_OnReceiveEvent_0001 end");
}

/**
 * @tc.name: AbilityManagerEventSubscriberTest_ScreenUnlock_OnReceiveEvent_0002
 * @tc.desc: callback is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerEventSubscriberTest, ScreenUnlock_OnReceiveEvent_0002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerEventSubscriberTest ScreenUnlock_OnReceiveEvent_0002 start");
    EventFwk::CommonEventSubscribeInfo subscribeInfo;
    std::function<void(int32_t)> callback = [](int32_t) {};
    EXPECT_NE(callback, nullptr);
    auto subscriber = std::make_shared<AbilityScreenUnlockEventSubscriber>(subscribeInfo, callback);
    EventFwk::CommonEventData data;
    data.want_.operation_.action_ = EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED;
    subscriber->screenUnlockCallback_ = nullptr;
    subscriber->OnReceiveEvent(data);
    EXPECT_EQ(subscriber->screenUnlockCallback_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerEventSubscriberTest ScreenUnlock_OnReceiveEvent_0002 end");
}

/**
 * @tc.name: AbilityManagerEventSubscriberTest_ScreenUnlock_OnReceiveEvent_0003
 * @tc.desc: receive COMMON_EVENT_SCREEN_UNLOCKED, userId valid, but not trigger callback
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerEventSubscriberTest, ScreenUnlock_OnReceiveEvent_0003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerEventSubscriberTest ScreenUnlock_OnReceiveEvent_0003 start");
    EventFwk::CommonEventSubscribeInfo subscribeInfo;
    std::function<void(int32_t)> callback = [](int32_t) {};
    EXPECT_NE(callback, nullptr);
    auto subscriber = std::make_shared<AbilityScreenUnlockEventSubscriber>(subscribeInfo, callback);
    EventFwk::CommonEventData data;
    int32_t userId = 1;
    data.want_.operation_.action_ = EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED;
    data.want_.SetParam("userId", userId);
    subscriber->OnReceiveEvent(data);
    EXPECT_NE(subscriber->screenUnlockCallback_, nullptr);
    bool screenFlag = AbilityEventMapManager::GetInstance().eventMap_[userId].screenUnlock_;
    bool userFlag = AbilityEventMapManager::GetInstance().eventMap_[userId].userUnlock_;
    EXPECT_EQ(screenFlag, true);
    EXPECT_EQ(userFlag, false);
    AbilityEventMapManager::GetInstance().RemoveUser(userId);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerEventSubscriberTest ScreenUnlock_OnReceiveEvent_0003 end");
}

/**
 * @tc.name: AbilityManagerEventSubscriberTest_ScreenUnlock_OnReceiveEvent_0004
 * @tc.desc: receive COMMON_EVENT_SCREEN_UNLOCKED, userId inValid, not trigger callback
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerEventSubscriberTest, ScreenUnlock_OnReceiveEvent_0004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerEventSubscriberTest ScreenUnlock_OnReceiveEvent_0004 start");
    EventFwk::CommonEventSubscribeInfo subscribeInfo;
    std::function<void(int32_t)> callback = [](int32_t) {};
    EXPECT_NE(callback, nullptr);
    auto subscriber = std::make_shared<AbilityScreenUnlockEventSubscriber>(subscribeInfo, callback);
    EventFwk::CommonEventData data;
    int32_t userId = -1;
    data.want_.operation_.action_ = EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED;
    data.want_.SetParam("userId", userId);
    subscriber->OnReceiveEvent(data);
    EXPECT_NE(subscriber->screenUnlockCallback_, nullptr);
    bool screenFlag = AbilityEventMapManager::GetInstance().eventMap_[userId].screenUnlock_;
    bool userFlag = AbilityEventMapManager::GetInstance().eventMap_[userId].userUnlock_;
    EXPECT_EQ(screenFlag, false);
    EXPECT_EQ(userFlag, false);
    AbilityEventMapManager::GetInstance().RemoveUser(userId);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerEventSubscriberTest ScreenUnlock_OnReceiveEvent_0004 end");
}

/**
 * @tc.name: AbilityManagerEventSubscriberTest_ScreenUnlock_OnReceiveEvent_0005
 * @tc.desc: repeated receive COMMON_EVENT_SCREEN_UNLOCKED, but not trigger callback
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerEventSubscriberTest, ScreenUnlock_OnReceiveEvent_0005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerEventSubscriberTest ScreenUnlock_OnReceiveEvent_0005 start");
    EventFwk::CommonEventSubscribeInfo subscribeInfo;
    std::function<void(int32_t)> callback = [](int32_t) {};
    EXPECT_NE(callback, nullptr);
    auto subscriber = std::make_shared<AbilityScreenUnlockEventSubscriber>(subscribeInfo, callback);
    EventFwk::CommonEventData data;
    int32_t userId = 1;
    data.want_.operation_.action_ = EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED;
    data.want_.SetParam("userId", userId);
    subscriber->OnReceiveEvent(data);
    EXPECT_NE(subscriber->screenUnlockCallback_, nullptr);
    bool screenFlag = AbilityEventMapManager::GetInstance().eventMap_[userId].screenUnlock_;
    bool userFlag = AbilityEventMapManager::GetInstance().eventMap_[userId].userUnlock_;
    EXPECT_EQ(screenFlag, true);
    EXPECT_EQ(userFlag, false);
    subscriber->OnReceiveEvent(data);
    screenFlag = AbilityEventMapManager::GetInstance().eventMap_[userId].screenUnlock_;
    userFlag = AbilityEventMapManager::GetInstance().eventMap_[userId].userUnlock_;
    EXPECT_EQ(screenFlag, true);
    EXPECT_EQ(userFlag, false);
    AbilityEventMapManager::GetInstance().RemoveUser(userId);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerEventSubscriberTest ScreenUnlock_OnReceiveEvent_0005 end");
}

/**
 * @tc.name: AbilityManagerEventSubscriberTest_userUnlock_OnReceiveEvent_0001
 * @tc.desc: receive other event
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerEventSubscriberTest, userUnlock_OnReceiveEvent_0001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerEventSubscriberTest userUnlock_OnReceiveEvent_0001 start");
    EventFwk::CommonEventSubscribeInfo subscribeInfo;
    std::function<void(int32_t)> callback = [](int32_t) {};
    std::function<void(int32_t)> callback2 = [](int32_t) {};
    EXPECT_NE(callback, nullptr);
    EXPECT_NE(callback2, nullptr);
    auto subscriber = std::make_shared<AbilityUserUnlockEventSubscriber>(subscribeInfo, callback, callback2);
    EventFwk::CommonEventData data;
    int32_t userId = 1;
    data.want_.operation_.action_ = EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED;
    data.code_ = userId;
    subscriber->OnReceiveEvent(data);
    EXPECT_NE(subscriber->screenUnlockCallback_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerEventSubscriberTest userUnlock_OnReceiveEvent_0001 end");
}

/**
 * @tc.name: AbilityManagerEventSubscriberTest_userUnlock_OnReceiveEvent_0002
 * @tc.desc: userScreenUnlockCallback_ screenUnlockCallback_ nullptr nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerEventSubscriberTest, userUnlock_OnReceiveEvent_0002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerEventSubscriberTest userUnlock_OnReceiveEvent_0002 start");
    EventFwk::CommonEventSubscribeInfo subscribeInfo;
    std::function<void(int32_t)> callback = [](int32_t) {};
    std::function<void(int32_t)> callback2 = [](int32_t) {};
    EXPECT_NE(callback, nullptr);
    EXPECT_NE(callback2, nullptr);
    auto subscriber = std::make_shared<AbilityUserUnlockEventSubscriber>(subscribeInfo, callback, callback2);
    subscriber->userScreenUnlockCallback_ = nullptr;
    subscriber->screenUnlockCallback_ = nullptr;
    EventFwk::CommonEventData data;
    int32_t userId = 1;
    data.want_.operation_.action_ = EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED;
    data.code_ = userId;
    subscriber->OnReceiveEvent(data);
    EXPECT_EQ(subscriber->screenUnlockCallback_, nullptr);
    EXPECT_EQ(subscriber->userScreenUnlockCallback_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerEventSubscriberTest userUnlock_OnReceiveEvent_0002 end");
}

/**
 * @tc.name: AbilityManagerEventSubscriberTest_userUnlock_OnReceiveEvent_0003
 * @tc.desc: userScreenUnlockCallback_ screenUnlockCallback_ nullptr valid
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerEventSubscriberTest, userUnlock_OnReceiveEvent_0003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerEventSubscriberTest userUnlock_OnReceiveEvent_0003 start");
    EventFwk::CommonEventSubscribeInfo subscribeInfo;
    std::function<void(int32_t)> callback = [](int32_t) {};
    std::function<void(int32_t)> callback2 = [](int32_t) {};
    EXPECT_NE(callback, nullptr);
    EXPECT_NE(callback2, nullptr);
    auto subscriber = std::make_shared<AbilityUserUnlockEventSubscriber>(subscribeInfo, callback, callback2);
    subscriber->userScreenUnlockCallback_ = nullptr;
    EventFwk::CommonEventData data;
    int32_t userId = 1;
    data.want_.operation_.action_ = EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED;
    data.code_ = userId;
    subscriber->OnReceiveEvent(data);
    EXPECT_NE(subscriber->screenUnlockCallback_, nullptr);
    EXPECT_EQ(subscriber->userScreenUnlockCallback_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerEventSubscriberTest userUnlock_OnReceiveEvent_0003 end");
}

/**
 * @tc.name: AbilityManagerEventSubscriberTest_userUnlock_OnReceiveEvent_0004
 * @tc.desc: userScreenUnlockCallback_ screenUnlockCallback_ valid nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerEventSubscriberTest, userUnlock_OnReceiveEvent_0004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerEventSubscriberTest userUnlock_OnReceiveEvent_0004 start");
    EventFwk::CommonEventSubscribeInfo subscribeInfo;
    std::function<void(int32_t)> callback = [](int32_t) {};
    std::function<void(int32_t)> callback2 = [](int32_t) {};
    EXPECT_NE(callback, nullptr);
    EXPECT_NE(callback2, nullptr);
    auto subscriber = std::make_shared<AbilityUserUnlockEventSubscriber>(subscribeInfo, callback, callback2);
    subscriber->screenUnlockCallback_ = nullptr;
    EventFwk::CommonEventData data;
    int32_t userId = 1;
    data.want_.operation_.action_ = EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED;
    data.code_ = userId;
    subscriber->OnReceiveEvent(data);
    EXPECT_EQ(subscriber->screenUnlockCallback_, nullptr);
    EXPECT_NE(subscriber->userScreenUnlockCallback_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerEventSubscriberTest userUnlock_OnReceiveEvent_0004 end");
}

/**
 * @tc.name: AbilityManagerEventSubscriberTest_userUnlock_OnReceiveEvent_0005
 * @tc.desc: repeated receive COMMON_EVENT_USER_UNLOCKED, but not trigger callback
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerEventSubscriberTest, userUnlock_OnReceiveEvent_0005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerEventSubscriberTest userUnlock_OnReceiveEvent_0005 start");
    EventFwk::CommonEventSubscribeInfo subscribeInfo;
    std::function<void(int32_t)> callback = [](int32_t) {};
    std::function<void(int32_t)> callback2 = [](int32_t) {};
    EXPECT_NE(callback, nullptr);
    EXPECT_NE(callback2, nullptr);
    auto subscriber = std::make_shared<AbilityUserUnlockEventSubscriber>(subscribeInfo, callback, callback2);
    EventFwk::CommonEventData data;
    int32_t userId = 1;
    data.want_.operation_.action_ = EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED;
    data.code_ = userId;
    subscriber->OnReceiveEvent(data);
    EXPECT_NE(subscriber->screenUnlockCallback_, nullptr);
    EXPECT_NE(subscriber->userScreenUnlockCallback_, nullptr);
    bool screenFlag = AbilityEventMapManager::GetInstance().eventMap_[userId].screenUnlock_;
    bool userFlag = AbilityEventMapManager::GetInstance().eventMap_[userId].userUnlock_;
    EXPECT_EQ(screenFlag, false);
    EXPECT_EQ(userFlag, true);
    subscriber->OnReceiveEvent(data);
    screenFlag = AbilityEventMapManager::GetInstance().eventMap_[userId].screenUnlock_;
    userFlag = AbilityEventMapManager::GetInstance().eventMap_[userId].userUnlock_;
    EXPECT_EQ(screenFlag, false);
    EXPECT_EQ(userFlag, true);
    AbilityEventMapManager::GetInstance().RemoveUser(userId);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerEventSubscriberTest userUnlock_OnReceiveEvent_0005 end");
}

/**
 * @tc.name: AbilityManagerEventSubscriberTest_AbilityEventSubscriber_OnReceiveEvent_0001
 * @tc.desc: receive both event, trigger callback from screenUnlockSubscriber
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerEventSubscriberTest, AbilityEventSubscriber_OnReceiveEvent_0001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerEventSubscriberTest AbilityEventSubscriber_OnReceiveEvent_0001 start");
    EventFwk::CommonEventSubscribeInfo userSubscribeInfo;
    std::function<void(int32_t)> callback = [](int32_t) {};
    std::function<void(int32_t)> callback2 = [](int32_t) {};
    EXPECT_NE(callback, nullptr);
    EXPECT_NE(callback2, nullptr);
    auto userSubscriber = std::make_shared<AbilityUserUnlockEventSubscriber>(userSubscribeInfo, callback, callback2);
    EventFwk::CommonEventData userData;
    int32_t userId = 1;
    userData.want_.operation_.action_ = EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED;
    userData.code_ = userId;
    userSubscriber->OnReceiveEvent(userData);
    EXPECT_NE(userSubscriber->screenUnlockCallback_, nullptr);
    EXPECT_NE(userSubscriber->userScreenUnlockCallback_, nullptr);
    bool screenFlag = AbilityEventMapManager::GetInstance().eventMap_[userId].screenUnlock_;
    bool userFlag = AbilityEventMapManager::GetInstance().eventMap_[userId].userUnlock_;
    EXPECT_EQ(screenFlag, false);
    EXPECT_EQ(userFlag, true);


    EventFwk::CommonEventSubscribeInfo screenSubscribeInfo;
    EXPECT_NE(callback, nullptr);
    auto screenSubscriber = std::make_shared<AbilityScreenUnlockEventSubscriber>(screenSubscribeInfo, callback);
    EventFwk::CommonEventData screenData;
    screenData.want_.operation_.action_ = EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED;
    screenData.want_.SetParam("userId", userId);
    screenSubscriber->OnReceiveEvent(screenData);
    EXPECT_NE(screenSubscriber->screenUnlockCallback_, nullptr);
    screenFlag = AbilityEventMapManager::GetInstance().eventMap_[userId].screenUnlock_;
    userFlag = AbilityEventMapManager::GetInstance().eventMap_[userId].userUnlock_;
    // removed After triggered
    EXPECT_EQ(screenFlag, false);
    EXPECT_EQ(userFlag, false);
    AbilityEventMapManager::GetInstance().RemoveUser(userId);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerEventSubscriberTest AbilityEventSubscriber_OnReceiveEvent_0001 end");
}

/**
 * @tc.name: AbilityManagerEventSubscriberTest_AbilityEventSubscriber_OnReceiveEvent_0002
 * @tc.desc: receive both event, trigger callback from userUnlockSubscriber
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerEventSubscriberTest, AbilityEventSubscriber_OnReceiveEvent_0002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerEventSubscriberTest AbilityEventSubscriber_OnReceiveEvent_0002 start");
    EventFwk::CommonEventSubscribeInfo screenSubscribeInfo;
    std::function<void(int32_t)> callback = [](int32_t) {};
    std::function<void(int32_t)> callback2 = [](int32_t) {};
    EXPECT_NE(callback, nullptr);
    EXPECT_NE(callback2, nullptr);
    int32_t userId = 1;
    auto screenSubscriber = std::make_shared<AbilityScreenUnlockEventSubscriber>(screenSubscribeInfo, callback);
    EventFwk::CommonEventData screenData;
    screenData.want_.operation_.action_ = EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED;
    screenData.want_.SetParam("userId", userId);
    screenSubscriber->OnReceiveEvent(screenData);
    EXPECT_NE(screenSubscriber->screenUnlockCallback_, nullptr);
    bool screenFlag = AbilityEventMapManager::GetInstance().eventMap_[userId].screenUnlock_;
    bool userFlag = AbilityEventMapManager::GetInstance().eventMap_[userId].userUnlock_;
    EXPECT_EQ(screenFlag, true);
    EXPECT_EQ(userFlag, false);

    EventFwk::CommonEventSubscribeInfo userSubscribeInfo;
    EXPECT_NE(callback, nullptr);
    EXPECT_NE(callback2, nullptr);
    auto userSubscriber = std::make_shared<AbilityUserUnlockEventSubscriber>(userSubscribeInfo, callback, callback2);
    EventFwk::CommonEventData userData;
    userData.want_.operation_.action_ = EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED;
    userData.code_ = userId;
    userSubscriber->OnReceiveEvent(userData);
    EXPECT_NE(userSubscriber->screenUnlockCallback_, nullptr);
    EXPECT_NE(userSubscriber->userScreenUnlockCallback_, nullptr);
    screenFlag = AbilityEventMapManager::GetInstance().eventMap_[userId].screenUnlock_;
    userFlag = AbilityEventMapManager::GetInstance().eventMap_[userId].userUnlock_;
    // removed After triggered
    EXPECT_EQ(screenFlag, false);
    EXPECT_EQ(userFlag, false);
    AbilityEventMapManager::GetInstance().RemoveUser(userId);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerEventSubscriberTest AbilityEventSubscriber_OnReceiveEvent_0002 end");
}

/**
 * @tc.name: AbilityManagerEventSubscriberTest_GetUserCount_0001
 * @tc.desc: GetUserCount reflects users added to and removed from event map
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerEventSubscriberTest, GetUserCount_0001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerEventSubscriberTest GetUserCount_0001 start");
    int32_t userId = 1;
    int32_t otherUserId = 2;
    AbilityEventMapManager::GetInstance().RemoveUser(userId);
    AbilityEventMapManager::GetInstance().RemoveUser(otherUserId);
    EXPECT_EQ(AbilityEventMapManager::GetInstance().GetUserCount(), 0u);
    AbilityEventMapManager::GetInstance().AddEvent(userId,
        EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED);
    EXPECT_EQ(AbilityEventMapManager::GetInstance().GetUserCount(), 1u);
    AbilityEventMapManager::GetInstance().AddEvent(otherUserId,
        EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED);
    EXPECT_EQ(AbilityEventMapManager::GetInstance().GetUserCount(), 2u);
    AbilityEventMapManager::GetInstance().RemoveUser(userId);
    EXPECT_EQ(AbilityEventMapManager::GetInstance().GetUserCount(), 1u);
    AbilityEventMapManager::GetInstance().RemoveUser(otherUserId);
    EXPECT_EQ(AbilityEventMapManager::GetInstance().GetUserCount(), 0u);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerEventSubscriberTest GetUserCount_0001 end");
}

/**
 * @tc.name: AbilityManagerEventSubscriberTest_AbilityEventSubscriber_OnReceiveEvent_0003
 * @tc.desc: finished user is removed from event map before screenUnlockCallback is invoked
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerEventSubscriberTest, AbilityEventSubscriber_OnReceiveEvent_0003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerEventSubscriberTest AbilityEventSubscriber_OnReceiveEvent_0003 start");
    int32_t userId = 1;
    int32_t callbackUserId = -1;
    size_t pendingUserCountWhenCallback = 1;
    std::function<void(int32_t)> callback = [&callbackUserId, &pendingUserCountWhenCallback](int32_t id) {
        callbackUserId = id;
        // UnSubscribeScreenUnlockedEvent counts pending users, the finished one must be gone already.
        pendingUserCountWhenCallback = AbilityEventMapManager::GetInstance().GetUserCount();
    };
    std::function<void(int32_t)> callback2 = [](int32_t) {};
    EventFwk::CommonEventSubscribeInfo subscribeInfo;
    auto userSubscriber = std::make_shared<AbilityUserUnlockEventSubscriber>(subscribeInfo, callback, callback2);
    EventFwk::CommonEventData userData;
    userData.want_.operation_.action_ = EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED;
    userData.code_ = userId;
    userSubscriber->OnReceiveEvent(userData);
    auto screenSubscriber = std::make_shared<AbilityScreenUnlockEventSubscriber>(subscribeInfo, callback);
    EventFwk::CommonEventData screenData;
    screenData.want_.operation_.action_ = EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED;
    screenData.want_.SetParam("userId", userId);
    screenSubscriber->OnReceiveEvent(screenData);
    EXPECT_EQ(callbackUserId, userId);
    EXPECT_EQ(pendingUserCountWhenCallback, 0u);
    AbilityEventMapManager::GetInstance().RemoveUser(userId);
    AbilityEventMapManager::GetInstance().RemovePendingUserId(userId);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerEventSubscriberTest AbilityEventSubscriber_OnReceiveEvent_0003 end");
}

/**
 * @tc.name: AbilityManagerEventSubscriberTest_AbilityEventSubscriber_OnReceiveEvent_0004
 * @tc.desc: finished user is removed from event map before userUnlockCallback completes the chain
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerEventSubscriberTest, AbilityEventSubscriber_OnReceiveEvent_0004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerEventSubscriberTest AbilityEventSubscriber_OnReceiveEvent_0004 start");
    int32_t userId = 1;
    int32_t callbackUserId = -1;
    size_t pendingUserCountWhenCallback = 1;
    std::function<void(int32_t)> callback = [&callbackUserId, &pendingUserCountWhenCallback](int32_t id) {
        callbackUserId = id;
        // UnSubscribeScreenUnlockedEvent counts pending users, the finished one must be gone already.
        pendingUserCountWhenCallback = AbilityEventMapManager::GetInstance().GetUserCount();
    };
    std::function<void(int32_t)> callback2 = [](int32_t) {};
    EventFwk::CommonEventSubscribeInfo subscribeInfo;
    auto screenSubscriber = std::make_shared<AbilityScreenUnlockEventSubscriber>(subscribeInfo, callback);
    EventFwk::CommonEventData screenData;
    screenData.want_.operation_.action_ = EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED;
    screenData.want_.SetParam("userId", userId);
    screenSubscriber->OnReceiveEvent(screenData);
    auto userSubscriber = std::make_shared<AbilityUserUnlockEventSubscriber>(subscribeInfo, callback, callback2);
    EventFwk::CommonEventData userData;
    userData.want_.operation_.action_ = EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED;
    userData.code_ = userId;
    userSubscriber->OnReceiveEvent(userData);
    EXPECT_EQ(callbackUserId, userId);
    EXPECT_EQ(pendingUserCountWhenCallback, 0u);
    AbilityEventMapManager::GetInstance().RemoveUser(userId);
    AbilityEventMapManager::GetInstance().RemovePendingUserId(userId);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerEventSubscriberTest AbilityEventSubscriber_OnReceiveEvent_0004 end");
}

/**
 * @tc.name: AbilityManagerEventSubscriberTest_PendingUserId_0001
 * @tc.desc: pending user ids are added per user, looked up by id, removed only for the matching user
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerEventSubscriberTest, PendingUserId_0001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerEventSubscriberTest PendingUserId_0001 start");
    int32_t userId = 1;
    int32_t otherUserId = 2;
    EXPECT_FALSE(AbilityEventMapManager::GetInstance().IsPendingUser(userId));
    EXPECT_FALSE(AbilityEventMapManager::GetInstance().IsPendingUser(otherUserId));
    // Each finished unlock chain records its own user, records do not overwrite each other.
    AbilityEventMapManager::GetInstance().AddPendingUserId(userId);
    AbilityEventMapManager::GetInstance().AddPendingUserId(otherUserId);
    EXPECT_TRUE(AbilityEventMapManager::GetInstance().IsPendingUser(userId));
    EXPECT_TRUE(AbilityEventMapManager::GetInstance().IsPendingUser(otherUserId));
    // A duplicate record is ignored, one unlock chain still authorizes one trigger.
    AbilityEventMapManager::GetInstance().AddPendingUserId(userId);
    EXPECT_TRUE(AbilityEventMapManager::GetInstance().IsPendingUser(userId));
    // The record belongs to another unlock chain, removing another user keeps it.
    AbilityEventMapManager::GetInstance().RemovePendingUserId(otherUserId);
    EXPECT_TRUE(AbilityEventMapManager::GetInstance().IsPendingUser(userId));
    AbilityEventMapManager::GetInstance().RemovePendingUserId(userId);
    EXPECT_FALSE(AbilityEventMapManager::GetInstance().IsPendingUser(userId));
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerEventSubscriberTest PendingUserId_0001 end");
}

/**
 * @tc.name: AbilityManagerEventSubscriberTest_AbilityEventSubscriber_OnReceiveEvent_0005
 * @tc.desc: passed unlock chain records the pending user before the user unlock callback
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerEventSubscriberTest, AbilityEventSubscriber_OnReceiveEvent_0005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerEventSubscriberTest AbilityEventSubscriber_OnReceiveEvent_0005 start");
    int32_t userId = 1;
    int32_t callbackUserId = -1;
    bool pendingUserConfirmedWhenCallback = false;
    std::function<void(int32_t)> callback = [&callbackUserId, &pendingUserConfirmedWhenCallback](int32_t id) {
        callbackUserId = id;
        // StartAutoStartupApps checks the pending user inside this callback,
        // it must be recorded before the callback is invoked.
        pendingUserConfirmedWhenCallback = AbilityEventMapManager::GetInstance().IsPendingUser(id);
    };
    std::function<void(int32_t)> callback2 = [](int32_t) {};
    EventFwk::CommonEventSubscribeInfo subscribeInfo;
    // A partial chain does not record any pending user.
    auto screenSubscriber = std::make_shared<AbilityScreenUnlockEventSubscriber>(subscribeInfo, callback);
    EventFwk::CommonEventData screenData;
    screenData.want_.operation_.action_ = EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED;
    screenData.want_.SetParam("userId", userId);
    screenSubscriber->OnReceiveEvent(screenData);
    EXPECT_FALSE(AbilityEventMapManager::GetInstance().IsPendingUser(userId));
    // The user unlock event completes the chain and records the user first.
    auto userSubscriber = std::make_shared<AbilityUserUnlockEventSubscriber>(subscribeInfo, callback, callback2);
    EventFwk::CommonEventData userData;
    userData.want_.operation_.action_ = EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED;
    userData.code_ = userId;
    userSubscriber->OnReceiveEvent(userData);
    EXPECT_EQ(callbackUserId, userId);
    EXPECT_TRUE(pendingUserConfirmedWhenCallback);
    // The subscriber only records the user, consuming it is the service's job.
    EXPECT_TRUE(AbilityEventMapManager::GetInstance().IsPendingUser(userId));
    AbilityEventMapManager::GetInstance().RemovePendingUserId(userId);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerEventSubscriberTest AbilityEventSubscriber_OnReceiveEvent_0005 end");
}

/**
 * @tc.name: AbilityManagerEventSubscriberTest_AbilityEventSubscriber_OnReceiveEvent_0006
 * @tc.desc: passed unlock chain records the pending user before the screen unlock callback
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerEventSubscriberTest, AbilityEventSubscriber_OnReceiveEvent_0006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerEventSubscriberTest AbilityEventSubscriber_OnReceiveEvent_0006 start");
    int32_t userId = 1;
    int32_t callbackUserId = -1;
    bool pendingUserConfirmedWhenCallback = false;
    std::function<void(int32_t)> callback = [&callbackUserId, &pendingUserConfirmedWhenCallback](int32_t id) {
        callbackUserId = id;
        // StartAutoStartupApps checks the pending user inside this callback,
        // it must be recorded before the callback is invoked.
        pendingUserConfirmedWhenCallback = AbilityEventMapManager::GetInstance().IsPendingUser(id);
    };
    std::function<void(int32_t)> callback2 = [](int32_t) {};
    EventFwk::CommonEventSubscribeInfo subscribeInfo;
    // A partial chain does not record any pending user.
    auto userSubscriber = std::make_shared<AbilityUserUnlockEventSubscriber>(subscribeInfo, callback, callback2);
    EventFwk::CommonEventData userData;
    userData.want_.operation_.action_ = EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED;
    userData.code_ = userId;
    userSubscriber->OnReceiveEvent(userData);
    EXPECT_FALSE(AbilityEventMapManager::GetInstance().IsPendingUser(userId));
    // The screen unlock event completes the chain and records the user first.
    auto screenSubscriber = std::make_shared<AbilityScreenUnlockEventSubscriber>(subscribeInfo, callback);
    EventFwk::CommonEventData screenData;
    screenData.want_.operation_.action_ = EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED;
    screenData.want_.SetParam("userId", userId);
    screenSubscriber->OnReceiveEvent(screenData);
    EXPECT_EQ(callbackUserId, userId);
    EXPECT_TRUE(pendingUserConfirmedWhenCallback);
    // The subscriber only records the user, consuming it is the service's job.
    EXPECT_TRUE(AbilityEventMapManager::GetInstance().IsPendingUser(userId));
    AbilityEventMapManager::GetInstance().RemovePendingUserId(userId);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerEventSubscriberTest AbilityEventSubscriber_OnReceiveEvent_0006 end");
}
} // namespace AAFwk
} // namespace OHOS