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
#include "app_image_observer_manager.h"
#include "application_update_callback.h"
#include "mock_application_update_callback.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class AppImageObserverManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AppImageObserverManagerTest::SetUpTestCase(void) {}

void AppImageObserverManagerTest::TearDownTestCase(void) {}

void AppImageObserverManagerTest::SetUp() {}

void AppImageObserverManagerTest::TearDown() {}

/**
 * @tc.name: NotifyApplicationPreAbilityCreate_0100
 * @tc.desc: NotifyApplicationPreAbilityCreate with empty callback list
 * @tc.type: FUNC
 */
HWTEST_F(AppImageObserverManagerTest, NotifyApplicationPreAbilityCreate_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyApplicationPreAbilityCreate_0100 start";
    AppImageObserverManager::GetInstance().appImageLifeCycleCallback_.clear();
    AppImageObserverManager::GetInstance().NotifyApplicationPreAbilityCreate();
    GTEST_LOG_(INFO) << "NotifyApplicationPreAbilityCreate_0100 end";
}

/**
 * @tc.name: NotifyApplicationPreAbilityCreate_0200
 * @tc.desc: NotifyApplicationPreAbilityCreate with expired callback
 * @tc.type: FUNC
 */
HWTEST_F(AppImageObserverManagerTest, NotifyApplicationPreAbilityCreate_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyApplicationPreAbilityCreate_0200 start";
    AppImageObserverManager::GetInstance().appImageLifeCycleCallback_.clear();
    std::weak_ptr<AbilityRuntime::AppImageLifeCycleCallback> expiredCallback = {};
    AppImageObserverManager::GetInstance().appImageLifeCycleCallback_.push_back(expiredCallback);
    AppImageObserverManager::GetInstance().NotifyApplicationPreAbilityCreate();
    GTEST_LOG_(INFO) << "NotifyApplicationPreAbilityCreate_0200 end";
}

/**
 * @tc.name: NotifyApplicationPreAbilityCreate_0300
 * @tc.desc: NotifyApplicationPreAbilityCreate with valid callback
 * @tc.type: FUNC
 */
HWTEST_F(AppImageObserverManagerTest, NotifyApplicationPreAbilityCreate_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyApplicationPreAbilityCreate_0300 start";
    AppImageObserverManager::GetInstance().appImageLifeCycleCallback_.clear();
    auto callback = std::make_shared<AbilityRuntime::MockApplicationUpdateCallback>();
    std::weak_ptr<AbilityRuntime::AppImageLifeCycleCallback> weakCallback = callback;
    AppImageObserverManager::GetInstance().appImageLifeCycleCallback_.push_back(weakCallback);
    AppImageObserverManager::GetInstance().NotifyApplicationPreAbilityCreate();
    GTEST_LOG_(INFO) << "NotifyApplicationPreAbilityCreate_0300 end";
}

/**
 * @tc.name: IsAbilityCreated_0100
 * @tc.desc: IsAbilityCreated with default value false
 * @tc.type: FUNC
 */
HWTEST_F(AppImageObserverManagerTest, IsAbilityCreated_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsAbilityCreated_0100 start";
    AppImageObserverManager::GetInstance().SetAbilityCreated(false);
    auto ret = AppImageObserverManager::GetInstance().IsAbilityCreated();
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "IsAbilityCreated_0100 end";
}

/**
 * @tc.name: IsAbilityCreated_0200
 * @tc.desc: IsAbilityCreated with value true
 * @tc.type: FUNC
 */
HWTEST_F(AppImageObserverManagerTest, IsAbilityCreated_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsAbilityCreated_0200 start";
    AppImageObserverManager::GetInstance().SetAbilityCreated(true);
    auto ret = AppImageObserverManager::GetInstance().IsAbilityCreated();
    EXPECT_TRUE(ret);
    AppImageObserverManager::GetInstance().SetAbilityCreated(false);
    GTEST_LOG_(INFO) << "IsAbilityCreated_0200 end";
}

/**
 * @tc.name: IsBeforeImageCreationPoint_0100
 * @tc.desc: IsBeforeImageCreationPoint when imageProcessType=1 and isAbilityCreated=false
 * @tc.type: FUNC
 */
HWTEST_F(AppImageObserverManagerTest, IsBeforeImageCreationPoint_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsBeforeImageCreationPoint_0100 start";
    AppImageObserverManager::GetInstance().SetImageProcessType(1);
    AppImageObserverManager::GetInstance().SetAbilityCreated(false);
    auto ret = AppImageObserverManager::GetInstance().IsBeforeImageCreationPoint();
    EXPECT_TRUE(ret);
    GTEST_LOG_(INFO) << "IsBeforeImageCreationPoint_0100 end";
}

/**
 * @tc.name: IsBeforeImageCreationPoint_0200
 * @tc.desc: IsBeforeImageCreationPoint when imageProcessType=1 and isAbilityCreated=true
 * @tc.type: FUNC
 */
HWTEST_F(AppImageObserverManagerTest, IsBeforeImageCreationPoint_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsBeforeImageCreationPoint_0200 start";
    AppImageObserverManager::GetInstance().SetImageProcessType(1);
    AppImageObserverManager::GetInstance().SetAbilityCreated(true);
    auto ret = AppImageObserverManager::GetInstance().IsBeforeImageCreationPoint();
    EXPECT_FALSE(ret);
    AppImageObserverManager::GetInstance().SetAbilityCreated(false);
    GTEST_LOG_(INFO) << "IsBeforeImageCreationPoint_0200 end";
}

/**
 * @tc.name: IsBeforeImageCreationPoint_0300
 * @tc.desc: IsBeforeImageCreationPoint when imageProcessType!=1 and isAbilityCreated=false
 * @tc.type: FUNC
 */
HWTEST_F(AppImageObserverManagerTest, IsBeforeImageCreationPoint_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsBeforeImageCreationPoint_0300 start";
    AppImageObserverManager::GetInstance().SetImageProcessType(0);
    AppImageObserverManager::GetInstance().SetAbilityCreated(false);
    auto ret = AppImageObserverManager::GetInstance().IsBeforeImageCreationPoint();
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "IsBeforeImageCreationPoint_0300 end";
}

/**
 * @tc.name: IsBeforeImageCreationPoint_0400
 * @tc.desc: IsBeforeImageCreationPoint when imageProcessType!=1 and isAbilityCreated=true
 * @tc.type: FUNC
 */
HWTEST_F(AppImageObserverManagerTest, IsBeforeImageCreationPoint_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsBeforeImageCreationPoint_0400 start";
    AppImageObserverManager::GetInstance().SetImageProcessType(2);
    AppImageObserverManager::GetInstance().SetAbilityCreated(true);
    auto ret = AppImageObserverManager::GetInstance().IsBeforeImageCreationPoint();
    EXPECT_FALSE(ret);
    AppImageObserverManager::GetInstance().SetAbilityCreated(false);
    GTEST_LOG_(INFO) << "IsBeforeImageCreationPoint_0400 end";
}
} // namespace AppExecFwk
} // namespace OHOS
