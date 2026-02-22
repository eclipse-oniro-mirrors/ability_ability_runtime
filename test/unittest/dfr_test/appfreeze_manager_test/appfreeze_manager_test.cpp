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
#include "appfreeze_util.h"

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


pid_t TestGetPidByName(const std::string& processName)
{
    pid_t pid = -1;
    std::string cmd = "pidof " + processName;

    FILE* fp = popen(cmd.c_str(), "r");
    if (fp != nullptr) {
        char buffer[256] = {'\0'};
        while (fgets(buffer, sizeof(buffer) - 1, fp) != nullptr) {}
        std::istringstream istr(buffer);
        istr >> pid;
        pclose(fp);
    }
    return pid;
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
    faultData.isInForeground = false;
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
    int32_t pid = static_cast<int32_t>(getprocpid());
    int state = AppfreezeManager::AppFreezeState::APPFREEZE_STATE_FREEZE;
    bool result = appfreezeManager->IsProcessDebug(pid, "Test");
    EXPECT_TRUE(!result);
}

/**
 * @tc.number: AppfreezeManagerTest_007
 * @tc.desc: add testcase codecoverage
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeManagerTest, AppfreezeManagerTest_007, TestSize.Level1)
{
    std::string str = "123";
    uint16_t index = 1;
    std::string ret = appfreezeManager->StrSplit(str, index);
    EXPECT_EQ(ret, "");
    str = "123:456";
    ret = appfreezeManager->StrSplit(str, index);
    EXPECT_EQ(ret, "456");
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
    appfreezeManager->IsProcessDebug(pid, bundleName);
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

#ifdef ABILITY_RUNTIME_HITRACE_ENABLE
/**
 * @tc.number: AppfreezeManagerTest_GetHitraceInfo_001
 * @tc.desc: add testcase codecoverage
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeManagerTest, AppfreezeManagerTest_GetHitraceInfo_001, TestSize.Level1)
{
    std::string ret = appfreezeManager->ParseDecToHex(1234); // test value
    EXPECT_EQ(ret, "4d2");
    ret = appfreezeManager->GetHitraceInfo();
    EXPECT_TRUE(ret.empty());
    OHOS::HiviewDFX::HiTraceChain::Begin("AppfreezeManagerTest_GetHitraceInfo_001", 0);
    appfreezeManager->GetHitraceInfo();
    FaultData faultData;
    faultData.errorObject.name = AppFreezeType::THREAD_BLOCK_6S;
    AppfreezeManager::AppInfo appInfo = {
        .pid = getpid(),
        .uid = getuid(),
        .bundleName = "AppfreezeManagerTest_GetHitraceInfo_001",
        .processName = "AppfreezeManagerTest_GetHitraceInfo_001",
    };
    int result = appfreezeManager->NotifyANR(faultData, appInfo, "test", "test");
    EXPECT_EQ(result, 0);
}
#endif

/**
 * @tc.number: AppfreezeManagerTest_InitWarningCpuDetailInfo_001
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeManagerTest, AppfreezeManagerTest_InitWarningCpuDetailInfo_001, TestSize.Level1)
{
    FaultData faultData;
    std::string bundleName = "AppfreezeManagerTest";
    faultData.appfreezeInfo = "test.txt";
    faultData.errorObject.name = AppFreezeType::APP_INPUT_BLOCK;
    int pid = getpid();
    int uid = getuid();
    AppfreezeManager::AppInfo appInfo = {
        .pid = pid,
        .uid = uid,
        .bundleName = bundleName,
        .processName = bundleName,
    };
    appfreezeManager->InitWarningCpuInfo(faultData, appInfo);
    faultData.errorObject.name = AppFreezeType::LIFECYCLE_TIMEOUT;
    appfreezeManager->InitWarningCpuInfo(faultData, appInfo);
    faultData.errorObject.name = AppFreezeType::THREAD_BLOCK_6S;
    appfreezeManager->InitWarningCpuInfo(faultData, appInfo);

    faultData.errorObject.name = AppFreezeType::THREAD_BLOCK_3S;
    appfreezeManager->InitWarningCpuInfo(faultData, appInfo);
    faultData.errorObject.name = AppFreezeType::LIFECYCLE_HALF_TIMEOUT;
    appfreezeManager->InitWarningCpuInfo(faultData, appInfo);
    int count = 10; // test value
    for (int i = 1; i <= count; i++) {
        appInfo.pid += i;
        appfreezeManager->InitWarningCpuInfo(faultData, appInfo);
    }
    while (count > 0) {
        count = sleep(count);
    }
    appInfo.pid = pid;
    appfreezeManager->InitWarningCpuInfo(faultData, appInfo);
    EXPECT_TRUE(appfreezeManager != nullptr);
}

/**
 * @tc.number: AppfreezeManagerTest_GetAppfreezeInfoPath_001
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeManagerTest, AppfreezeManagerTest_GetAppfreezeInfoPath_001, TestSize.Level1)
{
    FaultData faultData;
    std::string bundleName = "AppfreezeManagerTest";
    faultData.errorObject.name = AppFreezeType::APP_INPUT_BLOCK;
    AppfreezeManager::AppInfo appInfo = {
        .pid = getpid(),
        .uid = getuid(),
        .bundleName = bundleName,
        .processName = bundleName,
    };
    std::string ret = appfreezeManager->GetAppfreezeInfoPath(faultData, appInfo);
    EXPECT_TRUE(!ret.empty());
    faultData.appfreezeInfo = "test001";
    faultData.errorObject.name = AppFreezeType::THREAD_BLOCK_3S;
    ret = appfreezeManager->GetAppfreezeInfoPath(faultData, appInfo);
    EXPECT_EQ(ret, faultData.appfreezeInfo);
    faultData.errorObject.name = AppFreezeType::THREAD_BLOCK_6S;
    ret = appfreezeManager->GetAppfreezeInfoPath(faultData, appInfo);
    EXPECT_TRUE(!ret.empty());

    faultData.errorObject.name = AppFreezeType::LIFECYCLE_HALF_TIMEOUT;
    appfreezeManager->InitWarningCpuInfo(faultData, appInfo);
    faultData.errorObject.name = AppFreezeType::LIFECYCLE_TIMEOUT;
    appfreezeManager->GetAppfreezeInfoPath(faultData, appInfo);
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
    faultData.markedId = 10;
    faultData.processedId = 10;
    faultData.dispatchedEventId = 10;
    faultNotifyData = appfreezeManager->GetFaultNotifyData(faultData, pid);
    EXPECT_EQ(faultNotifyData.markedId, 10);
    EXPECT_EQ(faultNotifyData.processedId, 10);
    EXPECT_EQ(faultNotifyData.dispatchedEventId, 10);
}

/**
 * @tc.number: AppfreezeManagerTest_GetFirstLine_001
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeManagerTest, AppfreezeManagerTest_GetFirstLine_001, TestSize.Level1)
{
    std::string ret = appfreezeManager->GetFirstLine("../111");
    EXPECT_EQ(ret, "");
    ret = appfreezeManager->GetFirstLine("/proc/" + std::to_string(getpid()) + "/status");
    EXPECT_TRUE(!ret.empty());
    appfreezeManager->GetFirstLine("/data/log/test");
    EXPECT_TRUE(appfreezeManager != nullptr);
}

/**
 * @tc.name: CatchSyncByPid_001
 * @tc.desc: add testcase code coverage
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeManagerTest, AppfreezeManagerTest_CatchSyncByPid_Test001, TestSize.Level1)
{
    int uid = AppfreezeUtil::GetUidByPid(getpid());
    EXPECT_TRUE(uid >= 0);
    uid = AppfreezeUtil::GetUidByPid(-1);
    EXPECT_TRUE(uid < 0);

    int pid = getpid();
    std::set<int> asyncPids;
    auto systemuiPid = TestGetPidByName("com.ohos.systemui");
    auto launcherPid = TestGetPidByName("com.ohos.sceneboard");
    auto hiviewPid = TestGetPidByName("hiview");
    asyncPids.insert(pid);
    asyncPids.insert(systemuiPid);
    asyncPids.insert(launcherPid);
    asyncPids.insert(hiviewPid);
    asyncPids.insert(-1);
    int launcherUid = AppfreezeUtil::GetUidByPid(launcherPid);
    EXPECT_TRUE(launcherUid >= 20000);

    std::set<int> syncPids;
    syncPids.insert(hiviewPid);
    syncPids.insert(launcherPid);
    syncPids.insert(pid);
    std::string ret = appfreezeManager->CatchASyncByPid(asyncPids, syncPids, pid);
    printf("ret: %s\n", ret.c_str());
    ret = appfreezeManager->CatchASyncByPid(asyncPids, syncPids, pid);
    printf("ret: %s\n", ret.c_str());
    EXPECT_TRUE(pid > 0);
}

/**
 * @tc.number: AppfreezeManagerTest InsertKillThread Test
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeManagerTest, AppfreezeManagerTest_KillThreadStateManagement_Test001, TestSize.Level1)
{
    int32_t killState = 1;
    int32_t pid = getpid();
    int32_t uid = getuid();
    std::string bundleName = "Test001";
    appfreezeManager->InsertKillThread(killState, pid, uid, bundleName);
    EXPECT_TRUE(appfreezeManager->freezeKillThreadMap_.size() > 0);
    appfreezeManager->InsertKillThread(killState, pid, uid, bundleName);
    EXPECT_TRUE(appfreezeManager->freezeKillThreadMap_.size() > 0);
    int count = 60; // test value
    for (int i = 1; i <= count; i++) {
        pid += i;
        appfreezeManager->InsertKillThread(killState, pid, uid, bundleName);
    }
    while (count > 0) {
        count = sleep(count);
    }
    pid += 1;
    appfreezeManager->InsertKillThread(killState, pid, uid, bundleName);
    EXPECT_TRUE(appfreezeManager->freezeKillThreadMap_.size() > 0);
    bool ret = appfreezeManager->CheckThreadKilled(pid, uid, bundleName);
    EXPECT_TRUE(ret);
    ret = appfreezeManager->IsSkipDetect(pid, uid, bundleName, "test");
    EXPECT_TRUE(ret);
    killState = -1;
    appfreezeManager->InsertKillThread(killState, pid, uid, bundleName);
    ret = appfreezeManager->CheckThreadKilled(pid, uid, bundleName);
    EXPECT_TRUE(!ret);
    ret = appfreezeManager->CheckThreadKilled(10, 10, bundleName);
    EXPECT_TRUE(!ret);
    ret = appfreezeManager->IsSkipDetect(pid, uid, bundleName, "test");
    EXPECT_TRUE(!ret);
}

/**
 * @tc.number: AppfreezeManagerTest GetCatcherStack Test
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeManagerTest, AppfreezeManagerTest_GetCatcherStack_Test001, TestSize.Level1)
{
    FaultData faultData;
    faultData.errorObject.name = AppFreezeType::THREAD_BLOCK_6S;
    faultData.faultType = FaultDataType::APP_FREEZE;
    AppfreezeManager::AppInfo appInfo;
    int ret = appfreezeManager->AppfreezeHandleWithStack(faultData, appInfo);
    EXPECT_EQ(ret, 0);
    std::string fileName = "/data/log/freeze/freeze.txt";
    std::string catcherStack = "AppfreezeManagerTest_GetCatcherStack_Test001";
    std::string result = appfreezeManager->GetCatcherStack(fileName, catcherStack);
    EXPECT_EQ(result, catcherStack);
    constexpr int MAX_REPORT_STACK_SIZE = 300 * 1024; // 300KB
    while (catcherStack.size() <= MAX_REPORT_STACK_SIZE) {
        catcherStack += "AppfreezeManagerTest_GetCatcherStack_Test001";
    }
    result = appfreezeManager->GetCatcherStack(fileName, catcherStack);
    EXPECT_EQ(result, "");
    std::string testValue = "main test";
    faultData.errorObject.stack = testValue;
    appfreezeManager->MergeNotifyInfo(faultData, appInfo);
}

/**
 * @tc.number: AppfreezeManagerTest CheckAppfreezeHappend Test
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeManagerTest, AppfreezeManagerTest_CheckAppfreezeHappend_Test001, TestSize.Level1)
{
    int pid = getpid();
    int uid = getuid();
    std::string testValue = "AppfreezeManagerTest_CheckAppfreezeHappend_Test001";
    std::string key = std::to_string(pid) + "_" + std::to_string(uid) + "_" + testValue;
    bool ret = appfreezeManager->CheckAppfreezeHappend(key, "CheckAppfreezeHappend_Test");
    EXPECT_EQ(ret, false);
    ret = appfreezeManager->CheckAppfreezeHappend(key, "THREAD_BLOCK_3S");
    EXPECT_EQ(ret, false);
    ret = appfreezeManager->CheckAppfreezeHappend(key, "BUSSINESS_THREAD_BLOCK_3S");
    EXPECT_EQ(ret, false);
    ret = appfreezeManager->CheckAppfreezeHappend(key, "THREAD_BLOCK_6S");
    EXPECT_EQ(ret, false);
    ret = appfreezeManager->CheckAppfreezeHappend(key, "LIFECYCLE_TIMEOUT");
    EXPECT_EQ(ret, true);
    ret = appfreezeManager->CheckAppfreezeHappend(key, "THREAD_BLOCK_3S");
    EXPECT_EQ(ret, true);
    ret = appfreezeManager->CheckAppfreezeHappend(testValue, "LIFECYCLE_TIMEOUT");
    EXPECT_EQ(ret, false);
    appfreezeManager->appfreezeInfo_[key].occurTime = appfreezeManager->GetFreezeCurrentTime() +
        600000; // test value
    for (int i = 0; i < 20; i++) {
        key += "_" + std::to_string(i);
        appfreezeManager->CheckAppfreezeHappend(key, "THREAD_BLOCK_6S");
    }
    int count = 60; // test value
    while (count > 0) {
        count = sleep(count);
    }
    key += "_" + std::to_string(20);
    ret = appfreezeManager->CheckAppfreezeHappend(key, "THREAD_BLOCK_6S");
    EXPECT_EQ(ret, false);
    ret = appfreezeManager->CheckAppfreezeHappend(key, "LIFECYCLE_HALF_TIMEOUT_WARNING");
    EXPECT_EQ(ret, false);
    ret = appfreezeManager->CheckAppfreezeHappend(key, "LIFECYCLE_HALF_TIMEOUT_WARNING");
    EXPECT_EQ(ret, true);
    ret = appfreezeManager->CheckAppfreezeHappend(key, "LIFECYCLE_TIMEOUT_WARNING");
    EXPECT_EQ(ret, false);
    ret = appfreezeManager->CheckAppfreezeHappend(key, "LIFECYCLE_TIMEOUT_WARNING");
    EXPECT_EQ(ret, false);
}

/**
 * @tc.number: AppfreezeManagerTest IsNeedIgnoreFreezeEvent Test
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeManagerTest, AppfreezeManagerTest_IsNeedIgnoreFreezeEvent_Test001, TestSize.Level1)
{
    int pid = getpid();
    int uid = getuid();
    std::string bundleName = "AppfreezeManagerTest_IsNeedIgnoreFreezeEvent_Test001";
    std::string key = std::to_string(pid) + "_" + std::to_string(uid) + "_" + bundleName;
    int reportTimes = 3;
    bool ret = appfreezeManager->IsNeedIgnoreFreezeEvent(key, "THREAD_BLOCK_6S", reportTimes);
    EXPECT_EQ(ret, false);
    ret = appfreezeManager->IsNeedIgnoreFreezeEvent(key, "THREAD_BLOCK_3S", reportTimes);
    EXPECT_EQ(ret, false);
    ret = appfreezeManager->IsNeedIgnoreFreezeEvent(key, "LIFECYCLE_TIMEOUT", reportTimes);
    EXPECT_EQ(ret, false);
    ret = appfreezeManager->IsNeedIgnoreFreezeEvent(key, "THREAD_BLOCK_6S", reportTimes);
    EXPECT_EQ(ret, false);
    std::string eventName = "LIFECYCLE_TIMEOUT_WARNING";
    ret = appfreezeManager->IsNeedIgnoreFreezeEvent(eventName, eventName, reportTimes);
    EXPECT_EQ(ret, false);
    ret = appfreezeManager->IsNeedIgnoreFreezeEvent(eventName, eventName, reportTimes);
    ret = appfreezeManager->IsNeedIgnoreFreezeEvent(eventName, eventName, reportTimes);
    ret = appfreezeManager->IsNeedIgnoreFreezeEvent(eventName, eventName, reportTimes);
    eventName = "LIFECYCLE_HALF_TIMEOUT_WARNING";
    ret = appfreezeManager->IsNeedIgnoreFreezeEvent(eventName, eventName, reportTimes);
    EXPECT_EQ(ret, false);
    ret = appfreezeManager->IsNeedIgnoreFreezeEvent(eventName, eventName, reportTimes);
    ret = appfreezeManager->IsNeedIgnoreFreezeEvent(eventName, eventName, reportTimes);
    ret = appfreezeManager->IsNeedIgnoreFreezeEvent(eventName, eventName, reportTimes);
    int count = 60; // test value
    while (count > 0) {
        count = sleep(count);
    }
    ret = appfreezeManager->IsNeedIgnoreFreezeEvent(key, "THREAD_BLOCK_6S", reportTimes);
    EXPECT_EQ(ret, false);
    eventName = "LIFECYCLE_TIMEOUT_WARNING";
    ret = appfreezeManager->IsNeedIgnoreFreezeEvent(eventName, eventName, reportTimes);
    EXPECT_EQ(ret, false);
    eventName = "LIFECYCLE_HALF_TIMEOUT_WARNING";
    ret = appfreezeManager->IsNeedIgnoreFreezeEvent(eventName, eventName, reportTimes);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.number: AppfreezeManagerTest CheckThreadKilled Test
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeManagerTest, AppfreezeManagerTest_CheckThreadKilled_Test001, TestSize.Level1)
{
    int pid = getpid();
    int uid = getuid();
    std::string bundleName = "AppfreezeManagerTest_CheckThreadKilled_Test001";
    bool result = appfreezeManager->CheckThreadKilled(pid, uid, bundleName);
    EXPECT_EQ(result, false);
    int32_t state = 0;
    appfreezeManager->InsertKillThread(state, pid, uid, bundleName);
    result = appfreezeManager->CheckThreadKilled(pid, uid, bundleName);
    EXPECT_EQ(result, true);
    result = appfreezeManager->CheckThreadKilled(pid, uid, bundleName);
    EXPECT_EQ(result, true);
    bundleName = "AppfreezeManagerTest_CheckThreadKilled_Test001111111";
    result = appfreezeManager->CheckThreadKilled(pid, uid, bundleName);
    EXPECT_EQ(result, false);
    state = -1;
    appfreezeManager->InsertKillThread(state, pid, uid, bundleName);
    result = appfreezeManager->CheckThreadKilled(pid, uid, bundleName);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: AppfreezeManagerTest CheckNeedRecordAppRunningUnquieId Test
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeManagerTest, AppfreezeManagerTest_CheckNeedRecordAppRunningUnquieId_Test001, TestSize.Level1)
{
    bool result = appfreezeManager->CheckNeedRecordAppRunningUnquieId("THREAD_BLOCK_3S");
    EXPECT_EQ(result, true);
    result = appfreezeManager->CheckNeedRecordAppRunningUnquieId("THREAD_BLOCK_6S");
    EXPECT_EQ(result, true);
    result = appfreezeManager->CheckNeedRecordAppRunningUnquieId("BUSSINESS_THREAD_BLOCK_3S");
    EXPECT_EQ(result, true);
    result = appfreezeManager->CheckNeedRecordAppRunningUnquieId("BUSSINESS_THREAD_BLOCK_6S");
    EXPECT_EQ(result, true);
    result = appfreezeManager->CheckNeedRecordAppRunningUnquieId("BUSINESS_INPUT_BLOCK");
    EXPECT_EQ(result, true);
    result = appfreezeManager->CheckNeedRecordAppRunningUnquieId("TEST");
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: AppfreezeManagerTest CheckAppfreezeHappend Test
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeManagerTest, AppfreezeManagerTest_CheckAppfreezeHappend_Test002, TestSize.Level1)
{
    int pid = getpid();
    int uid = getuid();
    std::string testValue = "AppfreezeManagerTest_CheckAppfreezeHappend_Test002";
    std::string key = std::to_string(pid) + "_" + std::to_string(uid) + "_" + testValue;
    bool ret = appfreezeManager->CheckAppfreezeHappend(key, "THREAD_BLOCK_3S");
    EXPECT_EQ(ret, false);
    ret = appfreezeManager->CheckAppfreezeHappend(key, "BUSSINESS_THREAD_BLOCK_3S");
    EXPECT_EQ(ret, false);
    ret = appfreezeManager->CheckAppfreezeHappend(key, "THREAD_BLOCK_6S");
    EXPECT_EQ(ret, false);
    ret = appfreezeManager->CheckAppfreezeHappend(key, "LIFECYCLE_TIMEOUT");
    EXPECT_EQ(ret, true);
    ret = appfreezeManager->CheckAppfreezeHappend(key, "THREAD_BLOCK_3S");
    EXPECT_EQ(ret, true);
    ret = appfreezeManager->CheckAppfreezeHappend(key, "APP_INPUT_BLOCK");
    EXPECT_EQ(ret, true);
    std::string halfWarning = "LIFECYCLE_HALF_TIMEOUT_WARNING";
    std::string timeoutWarning = "LIFECYCLE_TIMEOUT_WARNING";
    ret = appfreezeManager->CheckAppfreezeHappend(halfWarning, "LIFECYCLE_HALF_TIMEOUT_WARNING");
    EXPECT_EQ(ret, false);
    ret = appfreezeManager->CheckAppfreezeHappend(halfWarning, "LIFECYCLE_HALF_TIMEOUT_WARNING");
    EXPECT_EQ(ret, true);
    ret = appfreezeManager->CheckAppfreezeHappend(timeoutWarning, "LIFECYCLE_TIMEOUT_WARNING");
    EXPECT_EQ(ret, false);
    ret = appfreezeManager->CheckAppfreezeHappend(timeoutWarning, "LIFECYCLE_TIMEOUT_WARNING");
    EXPECT_EQ(ret, true);
    int64_t testTime = appfreezeManager->GetFreezeCurrentTime() + 600000; // test value
    appfreezeManager->appfreezeInfo_[key].occurTime = testTime;
    appfreezeManager->appfreezeInfo_[halfWarning].occurTime = testTime;
    ret = appfreezeManager->CheckAppfreezeHappend(halfWarning, "LIFECYCLE_HALF_TIMEOUT_WARNING");
    EXPECT_EQ(ret, false);
    ret = appfreezeManager->CheckAppfreezeHappend(halfWarning, "LIFECYCLE_HALF_TIMEOUT_WARNING");
    EXPECT_EQ(ret, true);
    ret = appfreezeManager->CheckAppfreezeHappend(key, "THREAD_BLOCK_3S");
    EXPECT_EQ(ret, false);
    ret = appfreezeManager->CheckAppfreezeHappend(key, "THREAD_BLOCK_6S");
    EXPECT_EQ(ret, false);
    ret = appfreezeManager->CheckAppfreezeHappend(key, "LIFECYCLE_TIMEOUT");
    EXPECT_EQ(ret, true);
    ret = appfreezeManager->CheckAppfreezeHappend(key, "THREAD_BLOCK_3S");
    EXPECT_EQ(ret, true);
    ret = appfreezeManager->CheckAppfreezeHappend(key, "APP_INPUT_BLOCK");
    EXPECT_EQ(ret, true);
}
}  // namespace AppExecFwk
}  // namespace OHOS
