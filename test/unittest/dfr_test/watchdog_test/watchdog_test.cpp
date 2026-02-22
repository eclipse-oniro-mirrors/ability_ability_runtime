/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include <thread>

#define private public
#define protected public
#include "watchdog.h"
#undef private
#undef protected

#include "main_thread.h"
#include "mock_app_thread.h"
#ifdef ABILITY_RUNTIME_HITRACE_ENABLE
#include "hitrace/hitracechain.h"
#endif

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {
constexpr int64_t TEST_INTERVAL_TIME = 5000;
class WatchdogTest : public testing::Test {
public:
    WatchdogTest()
    {}
    ~WatchdogTest()
    {}
    std::shared_ptr<MockHandler> mockHandler_ = nullptr;
    std::shared_ptr<EventRunner> runner_ = nullptr;
    std::shared_ptr<Watchdog> watchdog_ = nullptr;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void WatchdogTest::SetUpTestCase(void)
{}

void WatchdogTest::TearDownTestCase(void)
{}

void WatchdogTest::SetUp(void)
{
    runner_ = EventRunner::Create("");
    mockHandler_ = std::make_shared<MockHandler>(runner_);

    watchdog_ = std::make_shared<Watchdog>();
    watchdog_->Init(mockHandler_);
}

void WatchdogTest::TearDown(void)
{
    watchdog_->Stop();
}

/**
 * @tc.number: AppExecFwk_watchdog_IsReportEvent_0001
 * @tc.name: IsReportEvent
 * @tc.desc: Test the abnormal state of IsReportEvent.
 * @tc.require: issueI5MGFU
 */
HWTEST_F(WatchdogTest, AppExecFwk_watchdog_IsReportEvent_0001, Function | MediumTest | Level3)
{
    watchdog_->SetAppMainThreadState(false);
    bool ret = watchdog_->IsReportEvent();
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: AppExecFwk_watchdog_IsReportEvent_0002
 * @tc.name: IsReportEvent
 * @tc.desc: Test the change state of IsReportEvent.
 * @tc.require: issueI5MGFU
 */
HWTEST_F(WatchdogTest, AppExecFwk_watchdog_IsReportEvent_0002, Function | MediumTest | Level3)
{
    watchdog_->SetAppMainThreadState(true);
    watchdog_->AllowReportEvent();
    bool ret = watchdog_->IsReportEvent();
    EXPECT_FALSE(ret);
}

/**
 * @tc.number: AppExecFwk_watchdog_ReportEvent_0001
 * @tc.name: ReportEvent
 * @tc.desc: Test ReportEvent.
 * @tc.require: I5UL6H
 */
HWTEST_F(WatchdogTest, AppExecFwk_watchdog_ReportEvent_0001, Function | MediumTest | Level3)
{
    // be ready for ReportEvent
    watchdog_->lastWatchTime_ = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::
        steady_clock::now().time_since_epoch()).count() - TEST_INTERVAL_TIME;
    watchdog_->needReport_ = true;

    watchdog_->isSixSecondEvent_.store(true);

    watchdog_->ReportEvent();
    EXPECT_TRUE(1);
}

/**
 * @tc.number: AppExecFwk_watchdog_ReportEvent_0002
 * @tc.name: ReportEvent
 * @tc.desc: Test ReportEvent.
 * @tc.require: I5UL6H
 */
HWTEST_F(WatchdogTest, AppExecFwk_watchdog_ReportEvent_0002, Function | MediumTest | Level3)
{
    // be ready for ReportEvent
    watchdog_->lastWatchTime_ = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::
        system_clock::now().time_since_epoch()).count() - TEST_INTERVAL_TIME;
    watchdog_->needReport_ = true;

    watchdog_->isSixSecondEvent_.store(false);

    watchdog_->ReportEvent();
    EXPECT_TRUE(1);
}

/**
 * @tc.number: AppExecFwk_watchdog_ReportEvent_0003
 * @tc.name: ReportEvent
 * @tc.desc: Test ReportEvent.
 * @tc.require: I5UL6H
 */
HWTEST_F(WatchdogTest, AppExecFwk_watchdog_ReportEvent_0003, Function | MediumTest | Level3)
{
    watchdog_->isBgWorkingThread_.store(true);
    watchdog_->ReportEvent();
    EXPECT_TRUE(watchdog_->needReport_);
}

/**
 * @tc.number: AppExecFwk_watchdog_ReportEvent_0004
 * @tc.name: ReportEvent
 * @tc.desc: Test ReportEvent.
 * @tc.require: I5UL6H
 */
HWTEST_F(WatchdogTest, AppExecFwk_watchdog_ReportEvent_0004, Function | MediumTest | Level3)
{
    watchdog_->isBgWorkingThread_.store(false);
    watchdog_->lastWatchTime_ = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::
        system_clock::now().time_since_epoch()).count() - TEST_INTERVAL_TIME;
    watchdog_->needReport_ = true;
    watchdog_->isSixSecondEvent_.store(true);
    watchdog_->isInBackground_.store(false);
    watchdog_->ReportEvent();
    watchdog_->needReport_ = false;
    watchdog_->ReportEvent();
    EXPECT_TRUE(!watchdog_->needReport_);
}

/**
 * @tc.number: AppExecFwk_watchdog_ReportEvent_0005
 * @tc.name: ReportEvent
 * @tc.desc: Test ReportEvent.
 */
HWTEST_F(WatchdogTest, AppExecFwk_watchdog_ReportEvent_0005, Function | MediumTest | Level3)
{
    EXPECT_EQ(watchdog_->backgroundReportCount_, 0);
    watchdog_->lastWatchTime_ = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::
        system_clock::now().time_since_epoch()).count() - TEST_INTERVAL_TIME;
    watchdog_->isBgWorkingThread_.store(false);
    watchdog_->isInBackground_.store(false);
    watchdog_->bundleName_ = "AppExecFwk_watchdog_ReportEvent_0005";
    watchdog_->ReportEvent();
    watchdog_->isInBackground_.store(true);
    watchdog_->bundleName_ = "AppExecFwk_watchdog_ReportEvent_0005";
    watchdog_->backgroundReportCount_.store(0);
    watchdog_->ReportEvent();
    watchdog_->isInBackground_.store(true);
    watchdog_->bundleName_ = "com.ohos.sceneboard";
    watchdog_->ReportEvent();
    watchdog_->lastBackGroundWatchTime_ = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count() + TEST_INTERVAL_TIME;
    watchdog_->ReportEvent();
    watchdog_->backgroundReportCount_.store(5);
    watchdog_->ReportEvent();
}

/**
 * @tc.number: WatchdogTest_Init_001
 * @tc.name: Init
 * @tc.desc: Verify that function Init.
 */
HWTEST_F(WatchdogTest, WatchdogTest_Init_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "WatchdogTest_Init_001 start";
    std::shared_ptr<EventHandler> eventHandler = std::make_shared<EventHandler>();
    watchdog_->Init(eventHandler);
    EXPECT_TRUE(watchdog_->appMainHandler_ != nullptr);
    GTEST_LOG_(INFO) << "WatchdogTest_Init_001 end";
}

/**
 * @tc.number: WatchdogTest_Init_002
 * @tc.name: Init
 * @tc.desc: Verify that function Init.
 */
HWTEST_F(WatchdogTest, WatchdogTest_Init_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatchdogTest_Init_002 start";
    std::shared_ptr<EventHandler> eventHandler = nullptr;
    watchdog_->lastWatchTime_ = 2;
    watchdog_->Init(eventHandler);
    EXPECT_EQ(watchdog_->lastWatchTime_, 0);
    GTEST_LOG_(INFO) << "WatchdogTest_Init_002 end";
}

/**
 * @tc.number: WatchdogTest_Stop_001
 * @tc.name: Stop
 * @tc.desc: Verify that function Init.
 */
HWTEST_F(WatchdogTest, WatchdogTest_Stop_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "WatchdogTest_Stop_001 start";
    std::shared_ptr<EventHandler> eventHandler = std::make_shared<EventHandler>();
    watchdog_->Init(eventHandler);
    EXPECT_TRUE(watchdog_->appMainHandler_ != nullptr);
    watchdog_->Stop();
    EXPECT_TRUE(watchdog_->appMainHandler_ == nullptr);
    GTEST_LOG_(INFO) << "WatchdogTest_Stop_001 end";
}

/**
 * @tc.number: WatchdogTest_Stop_002
 * @tc.name: Stop
 * @tc.desc: Verify that function Stop.
 */
HWTEST_F(WatchdogTest, WatchdogTest_Stop_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatchdogTest_Stop_002 start";
    std::shared_ptr<EventHandler> eventHandler = nullptr;
    watchdog_->Stop();
    EXPECT_TRUE(watchdog_->stopWatchdog_);
    GTEST_LOG_(INFO) << "WatchdogTest_Stop_002 end";
}

/**
 * @tc.number: WatchdogTest_SetAppMainThreadState_001
 * @tc.name: SetAppMainThreadState
 * @tc.desc: Verify that function SetAppMainThreadState.
 */
HWTEST_F(WatchdogTest, WatchdogTest_SetAppMainThreadState_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "WatchdogTest_SetAppMainThreadState_001 start";
    bool appMainThreadState = true;
    EXPECT_FALSE(watchdog_->appMainThreadIsAlive_);
    watchdog_->SetAppMainThreadState(appMainThreadState);
    EXPECT_TRUE(watchdog_->appMainThreadIsAlive_);
    GTEST_LOG_(INFO) << "WatchdogTest_SetAppMainThreadState_001 end";
}

/**
 * @tc.number: WatchdogTest_SetBackgroundStatus_001
 * @tc.name: SetBackgroundStatus
 * @tc.desc: Verify that function SetBackgroundStatus.
 */
HWTEST_F(WatchdogTest, WatchdogTest_SetBackgroundStatus_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "WatchdogTest_SetBackgroundStatus_001 start";
    bool isInBackground = false;
    EXPECT_TRUE(watchdog_->isInBackground_);
    watchdog_->SetBackgroundStatus(isInBackground);
    EXPECT_FALSE(watchdog_->isInBackground_);
    GTEST_LOG_(INFO) << "WatchdogTest_SetBackgroundStatus_001 end";
}

/**
 * @tc.number: WatchdogTest_AllowReportEvent_001
 * @tc.name: AllowReportEvent
 * @tc.desc: Verify that function AllowReportEvent.
 */
HWTEST_F(WatchdogTest, WatchdogTest_AllowReportEvent_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatchdogTest_AllowReportEvent_001 start";
    watchdog_->needReport_ = false;
    watchdog_->AllowReportEvent();
    EXPECT_TRUE(watchdog_->needReport_);
    GTEST_LOG_(INFO) << "WatchdogTest_AllowReportEvent_001 end";
}

/**
 * @tc.number: WatchdogTest_IsReportEvent_001
 * @tc.name: IsReportEvent
 * @tc.desc: Verify that function IsReportEvent.
 */
HWTEST_F(WatchdogTest, WatchdogTest_IsReportEvent_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatchdogTest_IsReportEvent_003 start";
    watchdog_->SetAppMainThreadState(false);
    auto result = watchdog_->IsReportEvent();
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "WatchdogTest_IsReportEvent_003 end";
}

/**
 * @tc.number: WatchdogTest_IsReportEvent_002
 * @tc.name: IsReportEvent
 * @tc.desc: Verify that function IsReportEvent.
 */
HWTEST_F(WatchdogTest, WatchdogTest_IsReportEvent_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatchdogTest_IsReportEvent_004 start";
    bool appMainThreadState = true;
    EXPECT_FALSE(watchdog_->appMainThreadIsAlive_);
    watchdog_->SetAppMainThreadState(appMainThreadState);
    auto result = watchdog_->IsReportEvent();
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "WatchdogTest_IsReportEvent_004 end";
}

/**
 * @tc.number: WatchdogTest_IsStopwatchdog_001
 * @tc.name: IsStopWatchdog
 * @tc.desc: Verify that function IsStopWatchdog.
 */
HWTEST_F(WatchdogTest, WatchdogTest_IsStopwatchdog_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatchdogTest_IsStopwatchdog_001 start";
    auto result = watchdog_->IsStopWatchdog();
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "WatchdogTest_IsStopwatchdog_001 end";
}

/**
 * @tc.number: WatchdogTest_Timer_001
 * @tc.name: Timer
 * @tc.desc: Verify that function Timer.
 */
HWTEST_F(WatchdogTest, WatchdogTest_Timer_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatchdogTest_Timer_001 start";
    watchdog_->needReport_ = false;
    watchdog_->Timer();
    EXPECT_TRUE(!watchdog_->needReport_);
    GTEST_LOG_(INFO) << "WatchdogTest_Timer_001 end";
}

/**
 * @tc.number: WatchdogTest_Timer_002
 * @tc.name: Timer
 * @tc.desc: Verify that function Timer.
 */
HWTEST_F(WatchdogTest, WatchdogTest_Timer_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatchdogTest_Timer_002 start";
    bool isInBackground = true;
    EXPECT_TRUE(watchdog_->isInBackground_);
    watchdog_->SetBackgroundStatus(isInBackground);
    watchdog_->SetAppMainThreadState(true);
    watchdog_->Timer();
    EXPECT_FALSE(watchdog_->appMainThreadIsAlive_);
    GTEST_LOG_(INFO) << "WatchdogTest_Timer_002 end";
}

/**
 * @tc.number: WatchdogTest_Timer_003
 * @tc.name: Timer
 * @tc.desc: Verify that function Timer.
 */
HWTEST_F(WatchdogTest, WatchdogTest_Timer_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatchdogTest_Timer_003 start";
    bool appMainThreadState = true;
    EXPECT_FALSE(watchdog_->appMainThreadIsAlive_);
    watchdog_->SetAppMainThreadState(appMainThreadState);
    watchdog_->Timer();
    bool isInBackground = false;
    watchdog_->SetBackgroundStatus(isInBackground);
    watchdog_->Timer();
    watchdog_->stopWatchdog_ = true;
    watchdog_->Timer();
    EXPECT_TRUE(watchdog_->needReport_);
    EXPECT_FALSE(watchdog_->isInBackground_);
    EXPECT_FALSE(watchdog_->appMainThreadIsAlive_);
    GTEST_LOG_(INFO) << "WatchdogTest_Timer_003 end";
}

/**
 * @tc.number: WatchdogTest_Timer_004
 * @tc.name: Timer
 * @tc.desc: Verify that function Timer.
 */
HWTEST_F(WatchdogTest, WatchdogTest_Timer_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatchdogTest_Timer_004 start";
    bool appMainThreadState = true;
    EXPECT_FALSE(watchdog_->appMainThreadIsAlive_);
    watchdog_->SetAppMainThreadState(appMainThreadState);
    bool isInBackground = false;
    watchdog_->SetBackgroundStatus(isInBackground);
    watchdog_->appMainHandler_ = std::make_shared<EventHandler>();
    watchdog_->Timer();
    EXPECT_TRUE(watchdog_->needReport_);
    EXPECT_FALSE(watchdog_->isInBackground_);
    EXPECT_FALSE(watchdog_->appMainThreadIsAlive_);
    EXPECT_TRUE(watchdog_->appMainHandler_ != nullptr);
    GTEST_LOG_(INFO) << "WatchdogTest_Timer_004 end";
}

/**
 * @tc.number: WatchdogTest_ReportEvent_003
 * @tc.name: ReportEvent
 * @tc.desc: Verify that function ReportEvent.
 */
HWTEST_F(WatchdogTest, WatchdogTest_ReportEvent_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatchdogTest_ReportEvent_003 start";
    watchdog_->ReportEvent();
    EXPECT_FALSE(watchdog_->isSixSecondEvent_);
    EXPECT_TRUE(watchdog_->needReport_);
    GTEST_LOG_(INFO) << "WatchdogTest_ReportEvent_003 end";
}

/**
 * @tc.number: WatchdogTest_ReportEvent_004
 * @tc.name: ReportEvent
 * @tc.desc: Verify that function ReportEvent.
 */
HWTEST_F(WatchdogTest, WatchdogTest_ReportEvent_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatchdogTest_ReportEvent_002 start";
    watchdog_->lastWatchTime_ = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    watchdog_->ReportEvent();
    EXPECT_TRUE(watchdog_->appMainHandler_ != nullptr);
    GTEST_LOG_(INFO) << "WatchdogTest_ReportEvent_002 end";
}

/**
 * @tc.number: WatchdogTest_ReportEvent_005
 * @tc.name: ReportEvent
 * @tc.desc: Verify that function ReportEvent.
 */
HWTEST_F(WatchdogTest, WatchdogTest_ReportEvent_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatchdogTest_ReportEvent_005 start";
    watchdog_->lastWatchTime_ = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    watchdog_->needReport_ = false;
    watchdog_->ReportEvent();
    EXPECT_TRUE(watchdog_->needReport_ == false);
    GTEST_LOG_(INFO) << "WatchdogTest_ReportEvent_005 end";
}

/**
 * @tc.number: WatchdogTest_ReportEvent_006
 * @tc.name: ReportEvent
 * @tc.desc: Verify that function ReportEvent.
 */
HWTEST_F(WatchdogTest, WatchdogTest_ReportEvent_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatchdogTest_ReportEvent_006 start";
    watchdog_->lastWatchTime_ = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    watchdog_->isSixSecondEvent_ = true;
    watchdog_->ReportEvent();
    EXPECT_TRUE(watchdog_->needReport_);
    GTEST_LOG_(INFO) << "WatchdogTest_ReportEvent_006 end";
}

/**
 * @tc.number: WatchdogTest_ReportEvent_007
 * @tc.name: ReportEvent
 * @tc.desc: Verify that function ReportEvent.
 */
HWTEST_F(WatchdogTest, WatchdogTest_ReportEvent_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatchdogTest_ReportEvent_007 start";
    watchdog_->lastWatchTime_ = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    watchdog_->ReportEvent();
    EXPECT_FALSE(watchdog_->isSixSecondEvent_);
    GTEST_LOG_(INFO) << "WatchdogTest_ReportEvent_007 end";
}

/**
 * @tc.number: WatchdogTest_ReportEvent_008
 * @tc.name: ReportEvent
 * @tc.desc: Verify that function ReportEvent.
 */
HWTEST_F(WatchdogTest, WatchdogTest_ReportEvent_008, TestSize.Level1)
{
    watchdog_->lastWatchTime_ = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
        system_clock::now().time_since_epoch()).count() - TEST_INTERVAL_TIME;
    std::shared_ptr<EventHandler> eventHandler = std::make_shared<EventHandler>();
    watchdog_->needReport_ = true;
    watchdog_->isSixSecondEvent_.store(true);
    EXPECT_TRUE(watchdog_->needReport_);
    watchdog_->ReportEvent();
}

/**
 * @tc.number: WatchdogTest_ReportEvent_009
 * @tc.name: ReportEvent
 * @tc.desc: Verify that function ReportEvent.
 */
HWTEST_F(WatchdogTest, WatchdogTest_ReportEvent_009, TestSize.Level1)
{
    watchdog_->lastWatchTime_ = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
        system_clock::now().time_since_epoch()).count() - TEST_INTERVAL_TIME;
    watchdog_->isInBackground_ = true;
    watchdog_->ReportEvent();
    watchdog_->SetBundleInfo("test", "1.1.0", true);
    watchdog_->SetBgWorkingThreadStatus(false);
}

/**
 * @tc.number: WatchdogTest_ReportEvent_010
 * @tc.desc: Verify that function ReportEvent.
 */
HWTEST_F(WatchdogTest, WatchdogTest_ReportEvent_010, TestSize.Level1)
{
    watchdog_->lastWatchTime_ = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
        system_clock::now().time_since_epoch()).count() - TEST_INTERVAL_TIME;
    watchdog_->isInBackground_ = false;
    watchdog_->isBgWorkingThread_ = false;
    watchdog_->needReport_ = false;
    watchdog_->ReportEvent();
    watchdog_->needReport_ = true;
    watchdog_->isSixSecondEvent_ = true;
    watchdog_->ReportEvent();
    watchdog_->isSixSecondEvent_ = false;
    watchdog_->ReportEvent();
    EXPECT_TRUE(watchdog_ != nullptr);
}

#ifdef ABILITY_RUNTIME_HITRACE_ENABLE
/**
 * @tc.number: WatchdogTest_SetHiTraceChainId_001
 * @tc.name: SetHiTraceChainId
 * @tc.desc: Verify that function SetHiTraceChainId.
 */
HWTEST_F(WatchdogTest, WatchdogTest_SetHiTraceChainId_001, TestSize.Level1)
{
    watchdog_->SetHiTraceChainId();
    OHOS::HiviewDFX::HiTraceChain::Begin("WatchdogTest_SetHiTraceChainId_001", 0);
    watchdog_->SetHiTraceChainId();
    EXPECT_TRUE(watchdog_->hitraceId_ != nullptr);
}
#endif

/**
 * @tc.number: WatchdogTest_CheckBgThread_000
 * @tc.desc: Verify that function ReportEvent.
 */
HWTEST_F(WatchdogTest, WatchdogTest_CheckBgThread_000, TestSize.Level1)
{
    watchdog_->isInBackground_.store(true);
    watchdog_->bundleName_ = "com.ohos.sceneboard";
    watchdog_->lastWatchTime_ = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
        system_clock::now().time_since_epoch()).count() - 3000; // 3000: test value
    int count = 5; // 5: test value
    for (int i = 0; i <= count; i++) {
        watchdog_->ReportEvent();
    }
    EXPECT_TRUE(watchdog_->backgroundReportCount_.load() >= count);
}
}  // namespace AppExecFwk
}  // namespace OHOS
