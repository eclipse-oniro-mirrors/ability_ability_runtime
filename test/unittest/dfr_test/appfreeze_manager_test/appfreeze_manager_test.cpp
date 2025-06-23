/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include "appfreeze_manager.h"
#undef private

#include "cpp/mutex.h"
#include "cpp/condition_variable.h"
#include "fault_data.h"
#include "freeze_util.h"
#ifdef ABILITY_RUNTIME_HITRACE_ENABLE
#include "hitrace/hitracechain.h"
#endif

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {
class AppfreezeManagerTest : public testing::Test {
public:
    AppfreezeManagerTest()
    {}
    ~AppfreezeManagerTest()
    {}
    std::shared_ptr<AppfreezeManager> appfreezeManager = nullptr;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AppfreezeManagerTest::SetUpTestCase(void)
{}

void AppfreezeManagerTest::TearDownTestCase(void)
{}

void AppfreezeManagerTest::SetUp(void)
{
    appfreezeManager = AppfreezeManager::GetInstance();
}

void AppfreezeManagerTest::TearDown(void)
{
    AppfreezeManager::DestroyInstance();
}

/**
 * @tc.number: AppfreezeManagerTest_001
 * @tc.desc: add testcase codecoverage
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeManagerTest, AppfreezeManagerTest_001, TestSize.Level0)
{
    bool ret = appfreezeManager->IsHandleAppfreeze("");
    EXPECT_TRUE(ret);
    ret = appfreezeManager->IsHandleAppfreeze("AppfreezeManagerTest_001");
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: AppfreezeManagerTest_002
 * @tc.desc: add testcase codecoverage
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeManagerTest, AppfreezeManagerTest_002, TestSize.Level1)
{
    FaultData faultData;
    faultData.errorObject.name = AppFreezeType::THREAD_BLOCK_6S;
    faultData.faultType = FaultDataType::APP_FREEZE;
    AppfreezeManager::AppInfo appInfo;
    int ret = appfreezeManager->AppfreezeHandle(faultData, appInfo);
    EXPECT_EQ(ret, 0);
    ret = appfreezeManager->AppfreezeHandleWithStack(faultData, appInfo);
    EXPECT_EQ(ret, 0);
    ret = appfreezeManager->AcquireStack(faultData, appInfo, "test");
    EXPECT_EQ(ret, 0);

    faultData.errorObject.name = AppFreezeType::APP_INPUT_BLOCK;
    ret = appfreezeManager->AppfreezeHandle(faultData, appInfo);
    EXPECT_EQ(ret, 0);
    ret = appfreezeManager->AppfreezeHandleWithStack(faultData, appInfo);
    EXPECT_EQ(ret, 0);
    ret = appfreezeManager->AcquireStack(faultData, appInfo, "test");
    EXPECT_EQ(ret, 0);
    faultData.errorObject.name = AppFreezeType::LIFECYCLE_HALF_TIMEOUT;
    ret = appfreezeManager->AppfreezeHandleWithStack(faultData, appInfo);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: AppfreezeManagerTest_003
 * @tc.desc: add testcase codecoverage
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeManagerTest, AppfreezeManagerTest_003, TestSize.Level1)
{
    FaultData faultData;
    faultData.errorObject.name = AppFreezeType::APP_INPUT_BLOCK;
    AppfreezeManager::AppInfo appInfo = {
        .pid = 1,
        .uid = 1,
        .bundleName = "AppfreezeManagerTest_003",
        .processName = "AppfreezeManagerTest_003",
    };
    int ret = appfreezeManager->NotifyANR(faultData, appInfo, "", "");
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: AppfreezeManagerTest_004
 * @tc.desc: add testcase codecoverage
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeManagerTest, AppfreezeManagerTest_004, TestSize.Level1)
{
    AppfreezeManager::ParamInfo info;
    int ret = appfreezeManager->LifecycleTimeoutHandle(info);
    EXPECT_EQ(ret, -1);
    AppfreezeManager::ParamInfo info1 = {
        .typeId = AppfreezeManager::TypeAttribute::NORMAL_TIMEOUT,
        .eventName = AppFreezeType::APP_INPUT_BLOCK,
        .bundleName = "",
        .msg = "Test",
    };
    ret = appfreezeManager->LifecycleTimeoutHandle(info1);
    EXPECT_EQ(ret, -1);
    AppfreezeManager::ParamInfo info2 = {
        .typeId = AppfreezeManager::TypeAttribute::CRITICAL_TIMEOUT,
        .eventName = AppFreezeType::APP_INPUT_BLOCK,
        .bundleName = "",
        .msg = "Test",
    };
    ret = appfreezeManager->LifecycleTimeoutHandle(info2);
    EXPECT_EQ(ret, -1);
    AppfreezeManager::ParamInfo info3 = {
        .typeId = AppfreezeManager::TypeAttribute::CRITICAL_TIMEOUT,
        .eventName = AppFreezeType::LIFECYCLE_HALF_TIMEOUT,
        .bundleName = "",
        .msg = "Test",
    };
    FreezeUtil::LifecycleFlow flow;
    flow.state = AbilityRuntime::FreezeUtil::TimeoutState::FOREGROUND;
    ret = appfreezeManager->LifecycleTimeoutHandle(info3, flow);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: AppfreezeManagerTest_005
 * @tc.desc: add testcase codecoverage
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeManagerTest, AppfreezeManagerTest_005, TestSize.Level1)
{
    std::map<int, std::list<AppfreezeManager::PeerBinderInfo>> binderInfos;
    AppfreezeManager::PeerBinderInfo infoOne= {1, 2, 3, 5};
    binderInfos[infoOne.clientPid].push_back(infoOne);
    AppfreezeManager::PeerBinderInfo infoTwo= {1, 3, 4, 0};
    binderInfos[infoTwo.clientPid].push_back(infoTwo);
    AppfreezeManager::PeerBinderInfo infoThree= {4, 0, 5, 6};
    binderInfos[infoThree.clientPid].push_back(infoThree);
    AppfreezeManager::PeerBinderInfo infoFour= {5, 6, 11, 7};
    binderInfos[infoFour.clientPid].push_back(infoFour);

    std::set<int> pids;
    AppfreezeManager::TerminalBinder terminalBinder = {0, 0};
    AppfreezeManager::ParseBinderParam params = {1, 3, 2, 0};
    appfreezeManager->ParseBinderPids(binderInfos, pids, params, true, terminalBinder);
    EXPECT_EQ(pids.size(), 0);
    params = {1, 3, 1, 0};
    appfreezeManager->ParseBinderPids(binderInfos, pids, params, true, terminalBinder);
    EXPECT_EQ(pids.size(), 4);
    EXPECT_EQ(terminalBinder.pid, infoFour.serverPid);
    EXPECT_EQ(terminalBinder.tid, infoFour.serverTid);
}

/**
 * @tc.number: AppfreezeManagerTest_006
 * @tc.desc: add testcase codecoverage
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeManagerTest, AppfreezeManagerTest_006, TestSize.Level1)
{
    std::string ret = appfreezeManager->CatcherStacktrace(0, "");
    printf("ret: %s\n", ret.c_str());
    ret = appfreezeManager->CatcherStacktrace(2, "");
    printf("ret: %s\n", ret.c_str());
    EXPECT_TRUE(!ret.empty());
    appfreezeManager->ClearOldInfo();
    int32_t pid = static_cast<int32_t>(getprocpid());
    int state = AppfreezeManager::AppFreezeState::APPFREEZE_STATE_FREEZE;
    bool result = appfreezeManager->IsNeedIgnoreFreezeEvent(pid, "Test");
    EXPECT_TRUE(!result);
    appfreezeManager->ClearOldInfo();
    result = appfreezeManager->IsProcessDebug(pid, "Test");
    EXPECT_TRUE(!result);
    result = appfreezeManager->IsNeedIgnoreFreezeEvent(pid, "Test");
    EXPECT_TRUE(result);
    std::string errorName = "THREAD_BLOCK_3S";
    result = appfreezeManager->IsNeedIgnoreFreezeEvent(12000, errorName);
    EXPECT_TRUE(!result);
}

/**
 * @tc.number: AppfreezeManagerTest_007
 * @tc.desc: add testcase codecoverage
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeManagerTest, AppfreezeManagerTest_007, TestSize.Level1)
{
    int32_t pid = static_cast<int32_t>(getprocpid());
    int state = AppfreezeManager::AppFreezeState::APPFREEZE_STATE_IDLE;
    EXPECT_EQ(appfreezeManager->GetFreezeState(pid), state);
    appfreezeManager->SetFreezeState(pid,
        AppfreezeManager::AppFreezeState::APPFREEZE_STATE_FREEZE, "Test");
    appfreezeManager->SetFreezeState(pid, state, "Test");
    EXPECT_EQ(appfreezeManager->GetFreezeState(pid), state);
}

/**
 * @tc.number: AppfreezeManagerTest_AppFreezeFilter_001
 * @tc.desc: add testcase codecoverage
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeManagerTest, AppfreezeManagerTest_AppFreezeFilter_001, TestSize.Level0)
{
    int32_t pid = static_cast<int32_t>(getprocpid());
    EXPECT_TRUE(!appfreezeManager->CancelAppFreezeDetect(pid, ""));
    appfreezeManager->ResetAppfreezeState(pid, "");
    EXPECT_TRUE(appfreezeManager->IsValidFreezeFilter(pid, ""));
    appfreezeManager->RemoveDeathProcess("");
}

/**
 * @tc.number: AppfreezeManagerTest_AppFreezeFilter_002
 * @tc.desc: add testcase codecoverage
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeManagerTest, AppfreezeManagerTest_AppFreezeFilter_002, TestSize.Level1)
{
    int32_t pid = static_cast<int32_t>(getprocpid());
    std::string bundleName = "AppfreezeManagerTest_AppFreezeFilter_002";
    EXPECT_TRUE(appfreezeManager->CancelAppFreezeDetect(pid, bundleName));
    EXPECT_TRUE(appfreezeManager->IsProcessDebug(pid, bundleName));
    appfreezeManager->RemoveDeathProcess(bundleName);
}

/**
 * @tc.number: AppfreezeManagerTest_CatchStack_001
 * @tc.desc: add testcase codecoverage
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeManagerTest, AppfreezeManagerTest_CatchStack_001, TestSize.Level1)
{
    int32_t pid = static_cast<int32_t>(getprocpid());
    std::string ret = "";
    appfreezeManager->FindStackByPid(ret, pid);
    EXPECT_TRUE(ret.empty());
    appfreezeManager->catchStackMap_[pid] = "AppfreezeManagerTest_CatchStack_001";
    appfreezeManager->FindStackByPid(ret, pid);
    EXPECT_TRUE(!ret.empty());
    EXPECT_TRUE(!appfreezeManager->catchStackMap_.empty());
    appfreezeManager->DeleteStack(pid);
    EXPECT_TRUE(appfreezeManager->catchStackMap_.empty());
}

/**
 * @tc.number: AppfreezeManagerTest_ParseDecToHex_001
 * @tc.desc: add testcase codecoverage
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeManagerTest, AppfreezeManagerTest_ParseDecToHex_001, TestSize.Level1)
{
    std::string ret = appfreezeManager->ParseDecToHex(1234); // test value
    EXPECT_EQ(ret, "4d2");
}

#ifdef ABILITY_RUNTIME_HITRACE_ENABLE
/**
 * @tc.number: AppfreezeManagerTest_GetHitraceId_001
 * @tc.desc: add testcase codecoverage
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeManagerTest, AppfreezeManagerTest_GetHitraceId_001, TestSize.Level1)
{
    AppfreezeManager::HitraceInfo info;
    bool ret = appfreezeManager->GetHitraceId(info);
    EXPECT_TRUE(!ret);
    OHOS::HiviewDFX::HiTraceChain::Begin("AppfreezeManagerTest_GetHitraceId_001", 0);
    appfreezeManager->GetHitraceId(info);
    FaultData faultData;
    faultData.errorObject.name = AppFreezeType::THREAD_BLOCK_6S;
    AppfreezeManager::AppInfo appInfo = {
        .pid = getpid(),
        .uid = getuid(),
        .bundleName = "AppfreezeManagerTest_GetHitraceId_001",
        .processName = "AppfreezeManagerTest_GetHitraceId_001",
    };
    int result = appfreezeManager->NotifyANR(faultData, appInfo, "test", "test");
    EXPECT_EQ(result, 0);
}
#endif

/**
 * @tc.number: AppfreezeManagerTest_ReportAppfreezeCpuInfo_001
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeManagerTest, AppfreezeManagerTest_ReportAppfreezeCpuInfo_001, TestSize.Level1)
{
    FaultData faultData;
    std::string bundleName = "AppfreezeManagerTest";
    faultData.appfreezeInfo = "test.txt";
    faultData.errorObject.name = AppFreezeType::THREAD_BLOCK_3S;
    AppfreezeManager::AppInfo appInfo = {
        .pid = getpid(),
        .uid = getuid(),
        .bundleName = bundleName,
        .processName = bundleName,
    };
    std::string freezeInfoFile = appfreezeManager->ReportAppfreezeCpuInfo(faultData, appInfo);
    EXPECT_EQ(freezeInfoFile, "");
    faultData.errorObject.name = AppFreezeType::THREAD_BLOCK_6S;
    freezeInfoFile = appfreezeManager->ReportAppfreezeCpuInfo(faultData, appInfo);
    EXPECT_TRUE(!freezeInfoFile.empty());
    faultData.errorObject.name = AppFreezeType::LIFECYCLE_HALF_TIMEOUT;
    freezeInfoFile = appfreezeManager->ReportAppfreezeCpuInfo(faultData, appInfo);
    EXPECT_EQ(freezeInfoFile, "");
    faultData.errorObject.name = AppFreezeType::LIFECYCLE_TIMEOUT;
    freezeInfoFile = appfreezeManager->ReportAppfreezeCpuInfo(faultData, appInfo);
    EXPECT_TRUE(!freezeInfoFile.empty());
}

/**
 * @tc.number: AppfreezeManagerTest_GetFaultNotifyData_001
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeManagerTest, AppfreezeManagerTest_GetFaultNotifyData_001, TestSize.Level1)
{
    FaultData faultData;
    int pid = getpid();
    FaultData faultNotifyData = appfreezeManager->GetFaultNotifyData(faultData, pid);
    EXPECT_EQ(faultNotifyData.eventId, -1);
    faultData.eventId = 10;
    faultNotifyData = appfreezeManager->GetFaultNotifyData(faultData, pid);
    EXPECT_EQ(faultNotifyData.eventId, 10);
}
}  // namespace AppExecFwk
}  // namespace OHOS
