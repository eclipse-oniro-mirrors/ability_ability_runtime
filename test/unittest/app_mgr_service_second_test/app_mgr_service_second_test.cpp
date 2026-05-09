/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ability_manager_errors.h"
#define private public
#define protected public
#include "app_mgr_service.h"
#undef protected
#undef private
#include "mock_ipc_skeleton.h"
#include "mock_my_flag.h"
#include "mock_permission_verification.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string BUNDLE_NAME = "com.example.test";
constexpr int UID = 1000;
constexpr pid_t PID = 1000;
}

class AppMgrServiceSecondTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AppMgrServiceSecondTest::SetUpTestCase(void) {}

void AppMgrServiceSecondTest::TearDownTestCase(void) {}

void AppMgrServiceSecondTest::SetUp() {}

void ReadyToRun(std::shared_ptr<AppMgrService> appMgrService)
{
    appMgrService->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    appMgrService->appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(appMgrService->taskHandler_,
        appMgrService->appMgrServiceInner_);
}

void AppMgrServiceSecondTest::TearDown() {}

/**
 * @tc.name: NotifyPageHide_0100
 * @tc.desc: NotifyPageHide.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceSecondTest, NotifyPageHide_0100, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);

    sptr<OHOS::IRemoteObject> token = nullptr;
    PageStateData pageStateData;
    auto ret = appMgrService->NotifyPageHide(token, pageStateData);
    EXPECT_NE(ret, ERR_OK);

    ReadyToRun(appMgrService);
    ret = appMgrService->NotifyPageHide(token, pageStateData);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: NotifyPageShow_0100
 * @tc.desc: NotifyPageShow.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceSecondTest, NotifyPageShow_0100, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    sptr<OHOS::IRemoteObject> token = nullptr;
    PageStateData pageStateData;
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->NotifyPageShow(token, pageStateData);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/**
 * @tc.name: RegisterAppRunningStatusListener_0100
 * @tc.desc: RegisterAppRunningStatusListener.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceSecondTest, RegisterAppRunningStatusListener_0100, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);

    sptr<IRemoteObject> listener = nullptr;
    auto ret = appMgrService->RegisterAppRunningStatusListener(listener);
    EXPECT_NE(ret, ERR_OK);

    ReadyToRun(appMgrService);
    ret = appMgrService->RegisterAppRunningStatusListener(listener);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: UnregisterAppRunningStatusListener_0100
 * @tc.desc: UnregisterAppRunningStatusListener.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceSecondTest, UnregisterAppRunningStatusListener_0100, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);

    sptr<IRemoteObject> listener = nullptr;
    auto ret = appMgrService->UnregisterAppRunningStatusListener(listener);
    EXPECT_NE(ret, ERR_OK);

    ReadyToRun(appMgrService);
    ret = appMgrService->UnregisterAppRunningStatusListener(listener);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: IsAppRunningByBundleNameAndUserId_0100
 * @tc.desc: IsAppRunningByBundleNameAndUserId.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceSecondTest, IsAppRunningByBundleNameAndUserId_0100, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);

    bool isRunning = false;
    auto ret = appMgrService->IsAppRunningByBundleNameAndUserId(BUNDLE_NAME, UID, isRunning);
    EXPECT_NE(ret, ERR_OK);

    ReadyToRun(appMgrService);
    ret = appMgrService->IsAppRunningByBundleNameAndUserId(BUNDLE_NAME, UID, isRunning);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: SignRestartAppFlag_0100
 * @tc.desc: SignRestartAppFlag.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceSecondTest, SignRestartAppFlag_0100, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);

    std::string instanceKey_ = "test";
    auto ret = appMgrService->SignRestartAppFlag(UID, instanceKey_);
    EXPECT_NE(ret, ERR_OK);

    ReadyToRun(appMgrService);
    AAFwk::MyFlag::flag_ = 0;
    ret = appMgrService->SignRestartAppFlag(UID, instanceKey_);
    EXPECT_NE(ret, ERR_OK);

    AAFwk::MyFlag::flag_ = 1;
    ret = appMgrService->SignRestartAppFlag(UID, instanceKey_);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: GetAppRunningUniqueIdByPid_0100
 * @tc.desc: GetAppRunningUniqueIdByPid.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceSecondTest, GetAppRunningUniqueIdByPid_0100, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);

    std::string appRunningUniqueId = "";
    auto ret = appMgrService->GetAppRunningUniqueIdByPid(PID, appRunningUniqueId);
    EXPECT_NE(ret, ERR_OK);

    ReadyToRun(appMgrService);
    AAFwk::MyFlag::flag_ = 0;
    ret = appMgrService->GetAppRunningUniqueIdByPid(PID, appRunningUniqueId);
    EXPECT_NE(ret, ERR_OK);

    AAFwk::MyFlag::flag_ = 1;
    ret = appMgrService->GetAppRunningUniqueIdByPid(UID, appRunningUniqueId);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: NotifyMemorySizeStateChanged_0100
 * @tc.desc: NotifyMemorySizeStateChanged.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceSecondTest, NotifyMemorySizeStateChanged_0100, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);

    int32_t memorySizeState = 1;
    auto ret = appMgrService->NotifyMemorySizeStateChanged(memorySizeState);
    EXPECT_NE(ret, ERR_OK);

    ReadyToRun(appMgrService);
    ret = appMgrService->NotifyMemorySizeStateChanged(memorySizeState);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: IsSharedBundleRunning_0100
 * @tc.desc: NA
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceSecondTest, IsSharedBundleRunning_0100, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    EXPECT_NE(appMgrService, nullptr);

    std::string bundleName = "com.example.test";
    uint32_t versionCode = 1;
    bool ret = appMgrService->IsSharedBundleRunning(bundleName, versionCode);
    EXPECT_FALSE(ret);

    ReadyToRun(appMgrService);
    ret = appMgrService->IsSharedBundleRunning(bundleName, versionCode);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: GetBundleNameByPid_0100
 * @tc.desc: NA
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceSecondTest, GetBundleNameByPid_0100, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    EXPECT_NE(appMgrService, nullptr);

    std::string bundleName = "com.example.test";
    int32_t pid = 1;
    int32_t uid = 0;
    auto ret = appMgrService->GetBundleNameByPid(pid, bundleName, uid);
    EXPECT_NE(ret, ERR_OK);
    ReadyToRun(appMgrService);
    ret = appMgrService->GetBundleNameByPid(pid, bundleName, uid);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: GetRunningProcessInfoByPid_0100
 * @tc.desc: NA
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceSecondTest, GetRunningProcessInfoByPid_0100, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    EXPECT_NE(appMgrService, nullptr);

    pid_t pid = 1;
    OHOS::AppExecFwk::RunningProcessInfo info = {};
    auto ret = appMgrService->GetRunningProcessInfoByPid(pid, info);
    EXPECT_NE(ret, ERR_OK);
    ReadyToRun(appMgrService);
    ret = appMgrService->GetRunningProcessInfoByPid(pid, info);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: GetRunningProcessInfoByChildProcessPid_0100
 * @tc.desc: NA
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceSecondTest, GetRunningProcessInfoByChildProcessPid_0100, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    EXPECT_NE(appMgrService, nullptr);

    pid_t childPid = 1;
    OHOS::AppExecFwk::RunningProcessInfo info = {};
    auto ret = appMgrService->GetRunningProcessInfoByChildProcessPid(childPid, info);
    EXPECT_NE(ret, ERR_OK);
    ReadyToRun(appMgrService);
    ret = appMgrService->GetRunningProcessInfoByChildProcessPid(childPid, info);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: SetAppFreezeFilter_0100
 * @tc.desc: NA
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceSecondTest, SetAppFreezeFilter_0100, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    EXPECT_NE(appMgrService, nullptr);

    int32_t pid = 1;
    bool ret = appMgrService->SetAppFreezeFilter(pid);
    EXPECT_FALSE(ret);
    ReadyToRun(appMgrService);
    ret = appMgrService->SetAppFreezeFilter(pid);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: StartNativeProcessForDebugger_0100
 * @tc.desc: NA
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceSecondTest, StartNativeProcessForDebugger_0100, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    EXPECT_NE(appMgrService, nullptr);

    AAFwk::Want want;
    auto ret = appMgrService->StartNativeProcessForDebugger(want);
    EXPECT_NE(ret, ERR_OK);
    ReadyToRun(appMgrService);
    ret = appMgrService->StartNativeProcessForDebugger(want);
    EXPECT_NE(ret, ERR_OK);
}


/**
 * @tc.name: CheckCallingIsUserTestMode_0100
 * @tc.desc: CheckCallingIsUserTestMode.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceSecondTest, CheckCallingIsUserTestMode_0100, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    int pid = 1;
    bool isUserTest = false;
    appMgrService->appMgrServiceInner_ = nullptr;
    auto ret = appMgrService->CheckCallingIsUserTestMode(pid, isUserTest);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    appMgrService->appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    ret = appMgrService->CheckCallingIsUserTestMode(pid, isUserTest);
    EXPECT_NE(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: NotifyProcessDependedOnWeb_0100
 * @tc.desc: NotifyProcessDependedOnWeb.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceSecondTest, NotifyProcessDependedOnWeb_0100, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    appMgrService->appMgrServiceInner_ = nullptr;
    auto ret = appMgrService->NotifyProcessDependedOnWeb();
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    appMgrService->appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    ret = appMgrService->NotifyProcessDependedOnWeb();
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: KillAppSelfWithInstanceKey_0100
 * @tc.desc: KillAppSelfWithInstanceKey.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceSecondTest, KillAppSelfWithInstanceKey_0100, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::string instanceKey;
    bool clearPageStack = false;
    std::string reason;
    appMgrService->appMgrServiceInner_ = nullptr;
    auto ret = appMgrService->KillAppSelfWithInstanceKey(instanceKey, clearPageStack, reason);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    appMgrService->appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    ret = appMgrService->KillAppSelfWithInstanceKey(instanceKey, clearPageStack, reason);
    EXPECT_NE(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: IsSpecifiedModuleLoaded_0100
 * @tc.desc: IsSpecifiedModuleLoaded.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceSecondTest, IsSpecifiedModuleLoaded_0100, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    AAFwk::Want want;
    AbilityInfo abilityInfo;
    bool result;
    bool isDebug = false;
    auto ret = appMgrService->IsSpecifiedModuleLoaded(want, abilityInfo, result, isDebug);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);

    appMgrService->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    appMgrService->appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(appMgrService->taskHandler_,
        appMgrService->appMgrServiceInner_);
    IPCSkeleton::pid_ = 0;
    ret = appMgrService->IsSpecifiedModuleLoaded(want, abilityInfo, result, isDebug);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);

    IPCSkeleton::pid_ = getprocpid();
    ret = appMgrService->IsSpecifiedModuleLoaded(want, abilityInfo, result, isDebug);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: UpdateProcessMemoryState_0100
 * @tc.desc: UpdateProcessMemoryState.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceSecondTest, UpdateProcessMemoryState_0100, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::vector<ProcessMemoryState> procMemState;
    auto ret = appMgrService->UpdateProcessMemoryState(procMemState);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);

    appMgrService->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    appMgrService->appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(appMgrService->taskHandler_,
        appMgrService->appMgrServiceInner_);
    AAFwk::MyFlag::flag_ = 1;
    ret = appMgrService->UpdateProcessMemoryState(procMemState);
    EXPECT_NE(ret, ERR_PERMISSION_DENIED);

    AAFwk::MyFlag::flag_ = 0;
    ret = appMgrService->UpdateProcessMemoryState(procMemState);
    EXPECT_NE(ret, ERR_INVALID_OPERATION);
}

/**
 * @tc.name: IsProcessCacheSupported_0100
 * @tc.desc: IsProcessCacheSupported.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceSecondTest, IsProcessCacheSupported_0100, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->appMgrServiceInner_ = nullptr;
    int32_t pid = 1;
    bool isSupport = false;
    auto ret = appMgrService->IsProcessCacheSupported(pid, isSupport);
    EXPECT_EQ(ret, AAFwk::ERR_NULL_APP_MGR_SERVICE_INNER);
}

/**
 * @tc.name: IsProcessCacheSupported_0200
 * @tc.desc: IsProcessCacheSupported.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceSecondTest, IsProcessCacheSupported_0200, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    appMgrService->appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    appMgrService->eventHandler_ =
        std::make_shared<AMSEventHandler>(appMgrService->taskHandler_, appMgrService->appMgrServiceInner_);
    int32_t pid = 1;
    bool isSupport = false;
    auto ret = appMgrService->IsProcessCacheSupported(pid, isSupport);
    EXPECT_EQ(ret, AAFwk::ERR_NO_PERMISSION_CALLER);
}

/**
 * @tc.name: SetProcessCacheEnable_0100
 * @tc.desc: SetProcessCacheEnable.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceSecondTest, SetProcessCacheEnable_0100, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->appMgrServiceInner_ = nullptr;
    int32_t pid = 1;
    bool enable = false;
    auto ret = appMgrService->SetProcessCacheEnable(pid, enable);
    EXPECT_EQ(ret, AAFwk::ERR_NULL_APP_MGR_SERVICE_INNER);
}

/**
 * @tc.name: SetProcessCacheEnable_0200
 * @tc.desc: SetProcessCacheEnable.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceSecondTest, SetProcessCacheEnable_0200, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    appMgrService->appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    appMgrService->eventHandler_ =
        std::make_shared<AMSEventHandler>(appMgrService->taskHandler_, appMgrService->appMgrServiceInner_);
    int32_t pid = 1;
    bool enable = false;
    auto ret = appMgrService->SetProcessCacheEnable(pid, enable);
    EXPECT_EQ(ret, AAFwk::ERR_NO_PERMISSION_CALLER);
}

/**
 * @tc.name: LockProcessCache_001
 * @tc.desc: LockProcessCache.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceSecondTest, LockProcessCache_001, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->appMgrServiceInner_ = nullptr;
    int32_t pid = 1;
    bool isLock = false;
    auto ret = appMgrService->LockProcessCache(pid, isLock);
    EXPECT_EQ(ret, AAFwk::ERR_NULL_APP_MGR_SERVICE_INNER);
}

/**
 * @tc.name: LockProcessCache_002
 * @tc.desc: LockProcessCache.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceSecondTest, LockProcessCache_002, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    appMgrService->appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    appMgrService->eventHandler_ =
        std::make_shared<AMSEventHandler>(appMgrService->taskHandler_, appMgrService->appMgrServiceInner_);
    int32_t pid = 1;
    bool isLock = false;
    //无权限CheckSpecificSystemAbilityAccessPermission返回false
    AAFwk::MyFlag::flag_ = 0;
    auto ret = appMgrService->LockProcessCache(pid, isLock);
    EXPECT_EQ(ret, AAFwk::ERR_NO_PERMISSION_CALLER);

    //有权限CheckSpecificSystemAbilityAccessPermission返回true
    AAFwk::MyFlag::flag_ = 1;
    ret = appMgrService->LockProcessCache(pid, isLock);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: UpdateFreezeExcludedPid_0100
 * @tc.desc: NA
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceSecondTest, UpdateFreezeExcludedPid_0100, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    EXPECT_NE(appMgrService, nullptr);

    int32_t pid = 1;
    int32_t profilerPid = 2;
    constexpr int32_t HIPROFILER_UID = 3063;
    IPCSkeleton::uid_ = HIPROFILER_UID;
    appMgrService->UpdateFreezeExcludedPid(true, pid, profilerPid);
}
} // namespace AppExecFwk
} // namespace OHOS