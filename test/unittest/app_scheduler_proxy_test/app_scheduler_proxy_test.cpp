/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "app_scheduler_proxy.h"
#include "fault_data.h"
#include "mock_app_scheduler.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
    const std::string STRING_BUNDLE_NAME = "bundleName";
    const std::string EMPTY_BUNDLE_NAME = "";
}
class AppSchedulerProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<MockAppScheduler> mockAppScheduler_;
};

void AppSchedulerProxyTest::SetUpTestCase(void)
{}

void AppSchedulerProxyTest::TearDownTestCase(void)
{}

void AppSchedulerProxyTest::SetUp()
{
    mockAppScheduler_ = new MockAppScheduler();
}

void AppSchedulerProxyTest::TearDown()
{}

/**
 * @tc.name: ScheduleNotifyAppFault_001
 * @tc.desc: Verify that the ScheduleNotifyAppFault interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(AppSchedulerProxyTest, ScheduleNotifyAppFault_001, TestSize.Level1)
{
    sptr<IRemoteObject> impl;
    sptr<AppSchedulerProxy> appSchedulerProxy = new (std::nothrow) AppSchedulerProxy(impl);
    FaultData faultData;
    faultData.faultType = FaultDataType::APP_FREEZE;
    faultData.errorObject.message = "msgContent";
    faultData.errorObject.stack = "stack";
    faultData.errorObject.name = "eventType";
    int32_t result = appSchedulerProxy->ScheduleNotifyAppFault(faultData);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: ScheduleChangeAppGcState_001
 * @tc.desc: Verify that the ScheduleChangeAppGcState interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(AppSchedulerProxyTest, ScheduleChangeAppGcState_001, TestSize.Level1)
{
    sptr<IRemoteObject> impl;
    sptr<AppSchedulerProxy> appSchedulerProxy = new (std::nothrow) AppSchedulerProxy(impl);
    int32_t result = appSchedulerProxy->ScheduleChangeAppGcState(0);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: ScheduleChangeAppGcState_002
 * @tc.desc: Verify that the ScheduleChangeAppGcState interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(AppSchedulerProxyTest, ScheduleChangeAppGcState_002, TestSize.Level1)
{
    sptr<IRemoteObject> impl;
    sptr<AppSchedulerProxy> appSchedulerProxy = new (std::nothrow) AppSchedulerProxy(impl);
    int32_t result = appSchedulerProxy->ScheduleChangeAppGcState(0, 1);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: AttachAppDebug_001
 * @tc.desc: Verify that AttachAppDebug interface calls normally.
 * @tc.type: FUNC
 */
HWTEST_F(AppSchedulerProxyTest, AttachAppDebug_001, TestSize.Level1)
{
    EXPECT_NE(mockAppScheduler_, nullptr);
    sptr<AppSchedulerProxy> appSchedulerProxy = new AppSchedulerProxy(mockAppScheduler_);
    EXPECT_NE(appSchedulerProxy, nullptr);

    EXPECT_CALL(*mockAppScheduler_, AttachAppDebug(_)).Times(1);
    appSchedulerProxy->AttachAppDebug(false);
}

/**
 * @tc.name: DetachAppDebug_001
 * @tc.desc: Verify that DetachAppDebug interface calls normally.
 * @tc.type: FUNC
 */
HWTEST_F(AppSchedulerProxyTest, DetachAppDebug_001, TestSize.Level1)
{
    EXPECT_NE(mockAppScheduler_, nullptr);
    sptr<AppSchedulerProxy> appSchedulerProxy = new AppSchedulerProxy(mockAppScheduler_);;
    EXPECT_NE(appSchedulerProxy, nullptr);
    
    EXPECT_CALL(*mockAppScheduler_, DetachAppDebug()).Times(1);
    appSchedulerProxy->DetachAppDebug();
}
} // namespace AppExecFwk
} // namespace OHOS
