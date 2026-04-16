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

#define private public
#include "appfreeze_inner.h"
#include "application_anr_listener.h"
#undef private

#include "xcollie/watchdog.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {
class AppfreezeInnerTest : public testing::Test {
public:
    AppfreezeInnerTest()
    {}
    ~AppfreezeInnerTest()
    {}
    std::shared_ptr<AppfreezeInner> appfreezeInner = nullptr;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AppfreezeInnerTest::SetUpTestCase(void)
{}

void AppfreezeInnerTest::TearDownTestCase(void)
{
    AppfreezeInner::DestroyInstance();
}

void AppfreezeInnerTest::SetUp(void)
{
    appfreezeInner = AppfreezeInner::GetInstance();
}

void AppfreezeInnerTest::TearDown(void)
{
    OHOS::HiviewDFX::Watchdog::GetInstance().StopWatchdog();
    AppfreezeInner::DestroyInstance();
}

/**
 * @tc.number: AppfreezeInnerTest_SetMainHandler_001
 * @tc.name: SetMainHandler
 * @tc.desc: Verify that function SetMainHandler.
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInner__SetMainHandler_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "AppfreezeInner__SetMainHandler_001 start";
    std::shared_ptr<EventHandler> eventHandler = std::make_shared<EventHandler>();
    EXPECT_TRUE(eventHandler != nullptr);
    AppfreezeInner::SetMainHandler(eventHandler);
    GTEST_LOG_(INFO) << "AppfreezeInner__SetMainHandler_001 end";
}

/**
 * @tc.number: AppfreezeInnerTest_SetApplicationInfo_001
 * @tc.name: SetApplicationInfo
 * @tc.desc: Verify that function SetApplicationInfo.
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInner__SetApplicationInfo_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "AppfreezeInner__SetApplicationInfo_001 start";
    std::shared_ptr<ApplicationInfo> applicationInfo = std::make_shared<ApplicationInfo>();
    EXPECT_TRUE(applicationInfo != nullptr);
    appfreezeInner->SetApplicationInfo(applicationInfo);
    GTEST_LOG_(INFO) << "AppfreezeInner__SetApplicationInfo_001 end";
}

/**
 * @tc.number: AppfreezeInnerTest_ThreadBlock_001
 * @tc.name: ThreadBlock
 * @tc.desc: Verify that function ThreadBlock.
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInner__ThreadBlock_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppfreezeInner__ThreadBlock_001 start";
    std::atomic_bool isSixSecondEvent = false;
    appfreezeInner->ThreadBlock(isSixSecondEvent);
    EXPECT_TRUE(isSixSecondEvent);
    GTEST_LOG_(INFO) << "AppfreezeInner__ThreadBlock_001 end";
}

/**
 * @tc.number: AppfreezeInnerTest_ThreadBlock_002
 * @tc.name: ThreadBlock
 * @tc.desc: Verify that function ThreadBlock.
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInner__ThreadBlock_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppfreezeInner__ThreadBlock_002 start";
    std::atomic_bool isSixSecondEvent = true;
    appfreezeInner->ThreadBlock(isSixSecondEvent);
    EXPECT_TRUE(isSixSecondEvent);

    appfreezeInner->SetAppDebug(true);
    appfreezeInner->ThreadBlock(isSixSecondEvent);
    EXPECT_TRUE(isSixSecondEvent);
    GTEST_LOG_(INFO) << "AppfreezeInner__ThreadBlock_002 end";
}

/**
 * @tc.number: AppfreezeInner_IsNeedIgnoreFreezeEvent_001
 * @tc.name: IsNeedIgnoreFreezeEvent
 * @tc.desc: Verify that function IsNeedIgnoreFreezeEvent.
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInner_IsNeedIgnoreFreezeEvent_001, TestSize.Level1)
{
    std::atomic_bool isSixSecondEvent = true;
    appfreezeInner->isAppDebug_ = false;
    appfreezeInner->ThreadBlock(isSixSecondEvent);
    EXPECT_TRUE(isSixSecondEvent);
}

/**
 * @tc.number: AppfreezeInner__AppfreezeHandle_001
 * @tc.name: AppfreezeHandle
 * @tc.desc: Verify that function AppfreezeHandle.
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInner__AppfreezeHandle_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppfreezeInner__AppfreezeHandle_001 start";
    FaultData faultData;
    faultData.state = 1;
    faultData.errorObject.message = AppFreezeType::THREAD_BLOCK_6S;
    faultData.faultType = FaultDataType::APP_FREEZE;
    faultData.timeoutMarkers = "";
    bool onlyMainThread = true;
    std::shared_ptr<ApplicationInfo> applicationInfo = std::make_shared<ApplicationInfo>();
    appfreezeInner->SetApplicationInfo(applicationInfo);
    int ret = appfreezeInner->AppfreezeHandle(faultData, onlyMainThread);
    EXPECT_EQ(ret, 0);
    appfreezeInner->ReportAppfreezeTask(faultData, onlyMainThread);
    ret = appfreezeInner->AcquireStack(faultData, onlyMainThread);
    EXPECT_EQ(ret, 0);
    appfreezeInner->SetAppDebug(true);
    ret = appfreezeInner->AppfreezeHandle(faultData, onlyMainThread);
    appfreezeInner->ReportAppfreezeTask(faultData, onlyMainThread);
    EXPECT_EQ(ret, -1);
    GTEST_LOG_(INFO) << "AppfreezeInner__AppfreezeHandle_001 end";
}

/**
 * @tc.number: AppfreezeInner__AcquireStack_001
 * @tc.name: AcquireStack
 * @tc.desc: Verify that function AcquireStack.
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInner__AcquireStack_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppfreezeInner__AcquireStack_001 start";

    FaultData faultData;
    faultData.errorObject.name = AppFreezeType::THREAD_BLOCK_6S;
    bool onlyMainThread = true;
    int ret = appfreezeInner->AcquireStack(faultData, onlyMainThread);
    EXPECT_EQ(ret, 0);
    GTEST_LOG_(INFO) << "AppfreezeInner__AcquireStack_001 end";
}

/**
 * @tc.number: AppfreezeInner_IsExitApp_001
 * @tc.name: IsExitApp
 * @tc.desc: Verify that function IsExitApp.
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInner_IsExitApp_001, TestSize.Level1)
{
    bool ret = appfreezeInner->IsExitApp(AppFreezeType::THREAD_BLOCK_6S);
    EXPECT_EQ(ret, true);
    ret = appfreezeInner->IsExitApp(AppFreezeType::LIFECYCLE_HALF_TIMEOUT);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.number: AppfreezeInner_NotifyANR
 * @tc.name: NotifyANR
 * @tc.desc: Verify that function NotifyANR.
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInner_NotifyANR, TestSize.Level1)
{
    FaultData faultData;
    faultData.errorObject.name = AppFreezeType::THREAD_BLOCK_6S;
    int ret = appfreezeInner->NotifyANR(faultData);
    EXPECT_EQ(ret, -1);
    std::shared_ptr<EventHandler> eventHandler = std::make_shared<EventHandler>();
    EXPECT_TRUE(eventHandler != nullptr);
    AppfreezeInner::SetMainHandler(eventHandler);
    ret = appfreezeInner->NotifyANR(faultData);
    EXPECT_EQ(ret, -1);
    ret = appfreezeInner->AcquireStack(faultData, true);
    EXPECT_EQ(ret, 0);
    appfreezeInner->AppFreezeRecovery();
}

/**
 * @tc.number: AppfreezeInner__Dump_001
 * @tc.name: Dump
 * @tc.desc: Verify that function Dump.
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInner__Dump_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppfreezeInner__Dump_001 start";
    std::string msgContent = "App main thread is not response!";
    MainHandlerDumper handlerDumper;
    handlerDumper.Dump(msgContent);
    EXPECT_EQ(msgContent, handlerDumper.GetDumpInfo());
    GTEST_LOG_(INFO) << "AppfreezeInner__Dump_001 end";
}

/**
 * @tc.number: AppfreezeInner__Dump_002
 * @tc.name: Dump
 * @tc.desc: Verify that function Dump.
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInner__Dump_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppfreezeInner__Dump_002 start";
    MainHandlerDumper handlerDumper;
    EXPECT_EQ("", handlerDumper.GetTag());
    GTEST_LOG_(INFO) << "AppfreezeInner__Dump_002 end";
}

/**
 * @tc.number: AppfreezeInner_AppfreezeHandleOverReportCount_001
 * @tc.name: AppfreezeHandleOverReportCount
 * @tc.desc: Verify that function AppfreezeHandleOverReportCount.
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInner_AppfreezeHandleOverReportCount_001, TestSize.Level1)
{
    bool isSixSecondEvent = true;
    appfreezeInner->AppfreezeHandleOverReportCount(isSixSecondEvent);
    isSixSecondEvent = false;
    appfreezeInner->AppfreezeHandleOverReportCount(isSixSecondEvent);
    EXPECT_TRUE(!isSixSecondEvent);
}

/**
 * @tc.number: AppfreezeInner_GetProcStatm_001
 * @tc.name: GetProcStatm
 * @tc.desc: Verify that function GetProcStatm.
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInner_GetProcStatm_001, TestSize.Level1)
{
    int32_t pid = static_cast<int32_t>(getpid());
    std::string procStatm = appfreezeInner->GetProcStatm(pid);
    EXPECT_TRUE(!procStatm.empty());
    pid = -1;
    procStatm = appfreezeInner->GetProcStatm(pid);
    EXPECT_TRUE(procStatm.empty());
}

/**
 * @tc.number: AppfreezeInner_EnableFreezeSample_001
 * @tc.name: EnableFreezeSample
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInner_EnableFreezeSample_001, TestSize.Level1)
{
    FaultData faultData;
    faultData.errorObject.name = AppFreezeType::THREAD_BLOCK_6S;
    appfreezeInner->EnableFreezeSample(faultData);
    faultData.errorObject.name = AppFreezeType::THREAD_BLOCK_3S;
    appfreezeInner->EnableFreezeSample(faultData);
    EXPECT_TRUE(appfreezeInner != nullptr);
}

/**
 * @tc.number: AppfreezeInner_AppInForeground_001
 * @tc.name: SetAppInForeground and GetAppInForeground
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInner_AppInForeground_001, TestSize.Level1)
{
    appfreezeInner->SetAppInForeground(true);
    EXPECT_TRUE(appfreezeInner->GetAppInForeground());
    appfreezeInner->SetAppInForeground(false);
    EXPECT_TRUE(!appfreezeInner->GetAppInForeground());
}

/**
 * @tc.number: AppfreezeInner_MainThreadSample_001
 * @tc.name: SetMainThreadSample and GetMainThreadSample
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInner_MainThreadSample_001, TestSize.Level1)
{
    appfreezeInner->SetMainThreadSample(true);
    EXPECT_TRUE(appfreezeInner->GetMainThreadSample());
    appfreezeInner->SetMainThreadSample(false);
    EXPECT_TRUE(!appfreezeInner->GetMainThreadSample());
}
/**
 * @tc.number: AppfreezeInner_GetApplicationInfo_001
 * @tc.name: GetApplicationInfo
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInner_GetApplicationInfo_001, TestSize.Level1)
{
    FaultData faultData;
    faultData.errorObject.name = AppFreezeType::THREAD_BLOCK_6S;
    appfreezeInner->GetApplicationInfo(faultData);
    appfreezeInner->application_ == nullptr;
    appfreezeInner->GetApplicationInfo(faultData);
    EXPECT_TRUE(appfreezeInner != nullptr);
}

/**
 * @tc.number: AppfreezeInner_LogFormat_001
 * @tc.name: LogFormat
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInner_LogFormat_001, TestSize.Level1)
{
    std::string ret = appfreezeInner->LogFormat(0, 0);
    EXPECT_TRUE(!ret.empty());
    ret = appfreezeInner->LogFormat(123, 123);
    EXPECT_TRUE(!ret.empty());
}

/**
 * @tc.number: AppfreezeInner_GetProcessLifeCycle_001
 * @tc.name: GetProcessLifeCycle
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInner_GetProcessLifeCycle_001, TestSize.Level1)
{
    std::string ret = appfreezeInner->GetProcessLifeCycle();
    printf("%s\n", ret.c_str());
    ret = appfreezeInner->GetProcessLifeCycle();
    printf("%s\n", ret.c_str());
    EXPECT_TRUE(appfreezeInner);
}

/**
 * @tc.number: AppfreezeInner_GetProcessStartTime_001
 * @tc.name: GetProcessStartTime
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInner_GetProcessStartTime_001, TestSize.Level1)
{
    unsigned long long startTime = 0;
    bool ret = appfreezeInner->GetProcessStartTime(gettid(), startTime);
    printf("%d\n", ret);
    startTime = 123456;
    ret = appfreezeInner->GetProcessStartTime(gettid(), startTime);
    printf("%d\n", ret);
    EXPECT_TRUE(appfreezeInner);
}

/**
 * @tc.number: AppfreezeInner_IsAppFreeze_001
 * @tc.name: IsAppFreeze
 * @tc.desc: Verify that function IsAppFreeze.
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInner_IsAppFreeze_001, TestSize.Level1)
{
    bool ret = appfreezeInner->IsAppFreeze(AppFreezeType::THREAD_BLOCK_3S);
    EXPECT_EQ(ret, false);
    ret = appfreezeInner->IsAppFreeze(AppFreezeType::THREAD_BLOCK_6S);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.number: AppfreezeInner_IsAppFreezeWarning_001
 * @tc.name: IsAppFreezeWarning
 * @tc.desc: Verify that function IsAppFreezeWarning.
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInner_IsAppFreezeWarning_001, TestSize.Level1)
{
    bool ret = appfreezeInner->IsAppFreezeWarning(AppFreezeType::THREAD_BLOCK_3S);
    EXPECT_EQ(ret, true);
    ret = appfreezeInner->IsAppFreezeWarning(AppFreezeType::THREAD_BLOCK_6S);
    EXPECT_EQ(ret, false);
    ret = appfreezeInner->IsAppFreezeWarning(AppFreezeType::LIFECYCLE_HALF_TIMEOUT);
    EXPECT_EQ(ret, true);
    ret = appfreezeInner->IsAppFreezeWarning(AppFreezeType::LIFECYCLE_TIMEOUT);
    EXPECT_EQ(ret, false);
}


/**
 * @tc.number: AppfreezeInner_ChangeFaultDateInfo_001
 * @tc.name: ChangeFaultDateInfo
 * @tc.desc: Verify that function ChangeFaultDateInfo.
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInner_ChangeFaultDateInfo_001, TestSize.Level1)
{
    FaultData faultData;
    std::string msgContent = "AppfreezeInner_ChangeFaultDateInfo_001";
    appfreezeInner->ChangeFaultDateInfo(faultData, msgContent);
    EXPECT_EQ(faultData.errorObject.message, msgContent);
    faultData.errorObject.name = AppFreezeType::APP_INPUT_BLOCK;
    appfreezeInner->ChangeFaultDateInfo(faultData, msgContent);
    std::string ret = "AppfreezeInner_ChangeFaultDateInfo_001AppfreezeInner_ChangeFaultDateInfo_001";
    EXPECT_EQ(faultData.errorObject.message, ret);
}

/**
 * @tc.number: AppfreezeInner_ChangeFaultDateInfo_002
 * @tc.name: ChangeFaultDateInfo
 * @tc.desc: Verify that function ChangeFaultDateInfo.
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInner_ChangeFaultDateInfo_002, TestSize.Level1)
{
    FaultData faultData;
    std::string testValue = "AppfreezeInner_ChangeFaultDateInfo_002";
    std::string msgContent = testValue;
    EXPECT_EQ(faultData.errorObject.message, "");
    faultData.errorObject.name = AppFreezeType::THREAD_BLOCK_3S;
    appfreezeInner->ChangeFaultDateInfo(faultData, msgContent);
    std::string ret = "AppfreezeInner_ChangeFaultDateInfo_002";
    EXPECT_EQ(faultData.errorObject.message, ret);
    EXPECT_EQ(faultData.forceExit, false);
}

/**
 * @tc.number: AppfreezeInner_ChangeFaultDateInfo_003
 * @tc.name: ChangeFaultDateInfo
 * @tc.desc: Verify that function ChangeFaultDateInfo.
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInner_ChangeFaultDateInfo_003, TestSize.Level1)
{
    FaultData faultData;
    std::string testValue = "AppfreezeInner_ChangeFaultDateInfo_003";
    std::string msgContent = testValue;
    faultData.errorObject.name = testValue;
    faultData.timeoutMarkers = testValue;
    faultData.eventId = 123;
    faultData.needKillProcess = false;
    faultData.appfreezeInfo = testValue;
    faultData.appRunningUniqueId = testValue;
    faultData.procStatm = testValue;
    faultData.isEnableMainThreadSample = false;
    uint64_t testTime = 12345;
    faultData.schedTime = testTime;
    faultData.detectTime = testTime;
    int32_t count = 123;
    faultData.appStatus = count;
    faultData.samplerStartTime = testTime;
    faultData.samplerFinishTime = testTime;
    faultData.samplerCount = count;
    faultData.pid = getpid();
    appfreezeInner->ChangeFaultDateInfo(faultData, msgContent);
    EXPECT_EQ(faultData.errorObject.message, msgContent);
    faultData.errorObject.name = AppFreezeType::THREAD_BLOCK_6S;
    faultData.needKillProcess = true;
    appfreezeInner->ChangeFaultDateInfo(faultData, msgContent);
    std::string ret = "AppfreezeInner_ChangeFaultDateInfo_003AppfreezeInner_ChangeFaultDateInfo_003";
    EXPECT_EQ(faultData.errorObject.message, ret);
    EXPECT_EQ(faultData.forceExit, true);
}

/**
 * @tc.number: AppfreezeInner_ChangeFaultDateInfo_004
 * @tc.name: ChangeFaultDateInfo
 * @tc.desc: Verify that function ChangeFaultDateInfo.
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInner_ChangeFaultDateInfo_004, TestSize.Level1)
{
    FaultData faultData;
    std::string msgContent = "AppfreezeInner_ChangeFaultDateInfo_004";
    faultData.errorObject.name = AppFreezeType::APP_INPUT_BLOCK;
    appfreezeInner->ChangeFaultDateInfo(faultData, msgContent);
    EXPECT_TRUE(!faultData.errorObject.message.empty());
    EXPECT_TRUE(faultData.forceExit);
}

/**
 * @tc.number: AppfreezeInnerTest
 * @tc.name: add test
 * @tc.desc: Verify that function SetMainHandler.
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInnerTest_SetMainHandler_001, TestSize.Level1)
{
    std::shared_ptr<EventHandler> eventHandler;
    AppfreezeInner::SetMainHandler(eventHandler);
    AppfreezeInner::SetMainHandler(nullptr);
    std::shared_ptr<EventHandler> eventHandler1 = std::make_shared<EventHandler>();
    EXPECT_TRUE(eventHandler1 != nullptr);
    AppfreezeInner::SetMainHandler(eventHandler1);
}

/**
 * @tc.number: AppfreezeInnerTest
 * @tc.name: add test
 * @tc.desc: Verify that function SetApplicationInfo.
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInnerTest_SetApplicationInfo_001, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> applicationInfo1;
    appfreezeInner->SetApplicationInfo(applicationInfo1);
    appfreezeInner->SetApplicationInfo(nullptr);
    std::shared_ptr<ApplicationInfo> applicationInfo = std::make_shared<ApplicationInfo>();
    EXPECT_TRUE(applicationInfo != nullptr);
    appfreezeInner->SetApplicationInfo(applicationInfo);
}

/**
 * @tc.number: AppfreezeInnerTest
 * @tc.name: add test
 * @tc.desc: Verify that function IsHandleAppfreeze.
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInnerTest_IsHandleAppfreeze_001, TestSize.Level1)
{
    appfreezeInner->isAppDebug_ = true;
    bool ret = appfreezeInner->IsHandleAppfreeze();
    EXPECT_TRUE(!ret);
    appfreezeInner->isAppDebug_ = false;
    ret = appfreezeInner->IsHandleAppfreeze();
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: AppfreezeInnerTest
 * @tc.name: add test
 * @tc.desc: Verify that function GetMainHandlerDump.
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInnerTest_GetMainHandlerDump_001, TestSize.Level1)
{
    std::string testValue = "appfreeze";
    std::string msgContent = testValue;
    appfreezeInner->GetMainHandlerDump(msgContent);
    bool ret = msgContent.find(testValue) != std::string::npos;
    EXPECT_TRUE(!ret);
    msgContent = "";
    appfreezeInner->GetMainHandlerDump(msgContent);
    ret = !msgContent.empty();
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: AppfreezeInnerTest
 * @tc.name: add test
 * @tc.desc: Verify that function GetProcessStartTime.
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInnerTest_GetProcessStartTime_001, TestSize.Level1)
{
    pid_t tid = 0;
    unsigned long long startTime = 0;
    bool ret = appfreezeInner->GetProcessStartTime(tid, startTime);
    EXPECT_TRUE(!ret);
    tid = gettid();
    startTime = 0;
    ret = appfreezeInner->GetProcessStartTime(tid, startTime);
    EXPECT_TRUE(ret);
    tid = gettid();
    startTime = 1234;
    ret = appfreezeInner->GetProcessStartTime(tid, startTime);
    EXPECT_TRUE(ret);
    tid = -1234;
    startTime = 0;
    ret = appfreezeInner->GetProcessStartTime(tid, startTime);
    EXPECT_TRUE(!ret);
    tid = -1234;
    startTime = 1234;
    ret = appfreezeInner->GetProcessStartTime(tid, startTime);
    EXPECT_TRUE(!ret);
}

/**
 * @tc.number: AppfreezeInnerTest
 * @tc.name: add test
 * @tc.desc: Verify that function LogFormat.
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInnerTest_LogFormat_001, TestSize.Level1)
{
    size_t totalSize = 0;
    size_t objectSize = 0;
    std::string ret = appfreezeInner->LogFormat(totalSize, objectSize);
    std::string testValue = "HEAP_TOTAL_SIZE:0,HEAP_OBJECT_SIZE:0,";
    EXPECT_EQ(ret, testValue);
    ret = appfreezeInner->LogFormat(totalSize, objectSize);
    totalSize = 1234;
    objectSize = 1234;
    ret = appfreezeInner->LogFormat(totalSize, objectSize);
    testValue = "HEAP_TOTAL_SIZE:1234,HEAP_OBJECT_SIZE:1234,";
    EXPECT_EQ(ret, testValue);
}

/**
 * @tc.number: AppfreezeInnerTest
 * @tc.name: add test
 * @tc.desc: Verify that function GetApplicationInfo.
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInnerTest_GetApplicationInfo_001, TestSize.Level1)
{
    FaultData faultData;
    std::string testValue = "";
    faultData.errorObject.name = testValue;
    faultData.timeoutMarkers = testValue;
    appfreezeInner->GetApplicationInfo(faultData);
    EXPECT_TRUE(faultData.timeoutMarkers.empty());
}

/**
 * @tc.number: AppfreezeInnerTest
 * @tc.name: add test
 * @tc.desc: Verify that function GetApplicationInfo.
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInnerTest_GetApplicationInfo_002, TestSize.Level1)
{
    FaultData faultData;
    std::string testValue = "AppfreezeInnerTest_GetApplicationInfo_002";
    faultData.errorObject.name = testValue;
    faultData.timeoutMarkers = testValue;
    faultData.eventId = 123;
    faultData.needKillProcess = false;
    faultData.appfreezeInfo = testValue;
    faultData.appRunningUniqueId = testValue;
    faultData.procStatm = testValue;
    faultData.isEnableMainThreadSample = false;
    uint64_t testTime = 12345;
    faultData.schedTime = testTime;
    faultData.detectTime = testTime;
    int32_t count = 123;
    faultData.appStatus = count;
    faultData.samplerStartTime = testTime;
    faultData.samplerFinishTime = testTime;
    faultData.samplerCount = count;
    faultData.pid = getpid();
    appfreezeInner->GetApplicationInfo(faultData);
    EXPECT_TRUE(!faultData.timeoutMarkers.empty());
}

/**
 * @tc.number: AppfreezeInnerTest
 * @tc.name: add test
 * @tc.desc: TransformHicollieFaultNumber Test.
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInnerTest_TransformHicollieFaultNumber_001, TestSize.Level1)
{
    const int businessInputBlockType = 7;
    int ret = appfreezeInner->TransformHicollieFaultNumber(AppFreezeType::BUSINESS_INPUT_BLOCK);
    EXPECT_EQ(ret, businessInputBlockType);
}

/**
 * @tc.number: AppfreezeInner_ReportLifeCycleAsAppfreeze_001
 * @tc.name: SetReportLifeCycleAsAppfreeze and GetReportLifeCycleAsAppfreeze
 */
HWTEST_F(AppfreezeInnerTest, AppfreezeInner_ReportLifeCycleAsAppfreeze_001, TestSize.Level1)
{
    appfreezeInner->SetReportLifeCycleAsAppfreeze(true);
    EXPECT_TRUE(appfreezeInner->GetReportLifeCycleAsAppfreeze());
    appfreezeInner->SetReportLifeCycleAsAppfreeze(false);
    EXPECT_TRUE(!appfreezeInner->GetReportLifeCycleAsAppfreeze());
}
}  // namespace AppExecFwk
}  // namespace OHOS
