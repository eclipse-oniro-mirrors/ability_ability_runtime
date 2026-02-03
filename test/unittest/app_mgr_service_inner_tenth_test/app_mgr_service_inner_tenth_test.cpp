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
#include "ability_util.h"  // Mock header to override CHECK_POINTER_AND_RETURN_LOG
#include "mock_task_handler_wrap.h"  // Mock TaskHandlerWrap for testing
#define private public
#include "app_mgr_service_inner.h"
#include "app_running_record.h"
#include "app_spawn_client.h"
#include "app_utils.h"
#include "render_record.h"
#include "child_process_record.h"
#include "cache_process_manager.h"
#undef private
#include "user_record_manager.h"
#include "mock_my_status.h"
#include "ability_manager_errors.h"
#include "overlay_manager_proxy.h"
#include "ability_connect_callback_stub.h"
#include "app_scheduler_const.h"
#include "want.h"
#include "application_info.h"
#include "mock_app_scheduler.h"
#include "parameters.h"
using namespace testing;
using namespace testing::ext;
using namespace OHOS::AAFwk;
using OHOS::AppExecFwk::ExtensionAbilityType;
constexpr int32_t FOUNDATION_UID = 5523;
constexpr int32_t QUICKFIX_UID = 5524;
constexpr int32_t SHADER_CACHE_GROUPID = 3099;
constexpr int32_t RESOURCE_MANAGER_UID = 1096;
constexpr int32_t DEFAULT_USER_ID = 0;
constexpr int32_t INVALID_DIED_PID = -1;
constexpr const char* DEVELOPER_MODE_STATE = "const.security.developermode.state";
constexpr const char* DEBUG_APP = "debugApp";
constexpr const char* DLP_PARAMS_SECURITY_FLAG = "ohos.dlp.params.securityFlag";
static int g_scheduleLoadChildCall = 0;
constexpr const char* UIEXTENSION_ABILITY_ID = "ability.want.params.uiExtensionAbilityId";
constexpr const char* UIEXTENSION_ROOT_HOST_PID = "ability.want.params.uiExtensionRootHostPid";
constexpr const char* UIEXTENSION_HOST_PID = "ability.want.params.uiExtensionHostPid";
constexpr const char* UIEXTENSION_HOST_UID = "ability.want.params.uiExtensionHostUid";
constexpr const char* UIEXTENSION_HOST_BUNDLENAME = "ability.want.params.uiExtensionHostBundleName";
constexpr const char* UIEXTENSION_BIND_ABILITY_ID = "ability.want.params.uiExtensionBindAbilityId";
constexpr const char* UIEXTENSION_NOTIFY_BIND = "ohos.uiextension.params.notifyProcessBind";
namespace OHOS {
namespace AppExecFwk {
class AppMgrServiceInnerTenthTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

class MyAbilityDebugResponse : public IAbilityDebugResponse {
public:
    void OnAbilitysDebugStarted(const std::vector<sptr<IRemoteObject>> &tokens) override
    {}

    void OnAbilitysDebugStoped(const std::vector<sptr<IRemoteObject>> &tokens) override
    {}

    void OnAbilitysAssertDebugChange(const std::vector<sptr<IRemoteObject>> &tokens,
        bool isAssertDebug) override {}
    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }
};

class MyStartSpecifiedAbilityResponse : public IStartSpecifiedAbilityResponse {
public:
    void OnAcceptWantResponse(const AAFwk::Want &want, const std::string &flag, int32_t requestId,
        int32_t userId) override
    {}
    void OnTimeoutResponse(int32_t requestId, int32_t userId) override
    {}
    void OnNewProcessRequestResponse(const std::string &flag, int32_t userId, int32_t requestId,
        const std::string &callerProcessName, int32_t recordId) override
    {}
    void OnNewProcessRequestTimeoutResponse(int32_t requestId, int32_t userId) override
    {}
    void OnStartSpecifiedFailed(int32_t requestId, int32_t userId) override
    {}
    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }
};

class MockIAppStateCallback : public IAppStateCallback {
public:
    MockIAppStateCallback() = default;
    virtual ~MockIAppStateCallback() = default;
    MOCK_METHOD1(OnAppStateChanged, void(const AppProcessData &appProcessData));
    MOCK_METHOD2(OnAbilityRequestDone, void(const sptr<IRemoteObject> &token, const AbilityState state));
    void NotifyAppPreCache(int32_t pid, int32_t userId) override
    {
        AAFwk::MyStatus::GetInstance().notifyAppPreCacheCalled_ = true;
    }
    void NotifyStartResidentProcess(std::vector<AppExecFwk::BundleInfo> &bundleInfos) override
    {
        AAFwk::MyStatus::GetInstance().notifyStartResidentProcessCalled_ = true;
    }
    void NotifyStartKeepAliveProcess(std::vector<AppExecFwk::BundleInfo> &bundleInfos,
        pid_t diedPid = INVALID_DIED_PID) override
    {
        AAFwk::MyStatus::GetInstance().notifyStartKeepAliveProcessCalled_ = true;
    }
    void RecordAppExitSignalReason(int32_t pid, int32_t uid, int32_t signal, std::string &bundleName) override
    {
        AAFwk::MyStatus::GetInstance().recordAppExitSignalReasonCall_ = true;
    }
    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }
};
void AppMgrServiceInnerTenthTest::SetUpTestCase() {}

void AppMgrServiceInnerTenthTest::TearDownTestCase() {}

void AppMgrServiceInnerTenthTest::SetUp() {}

void AppMgrServiceInnerTenthTest::TearDown() {}
/**
 * @tc.name: OnAppStarted_001
 * @tc.desc: Test AddMountPermission with valid token and permissions granted
 * @tc.type: FUNC
 * @tc.require: Test OnAppStarted method
 */
HWTEST_F(AppMgrServiceInnerTenthTest, OnAppStarted_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnAppStarted_001 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->OnAppStarted(nullptr);
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().getPriorityObjectCalled_);
    TAG_LOGI(AAFwkTag::TEST, "OnAppStarted_001 end");
}

/**
 * @tc.name: OnAppStarted_002
 * @tc.desc: Test OnAppStarted function with valid app record
 * @tc.type: FUNC
 * @tc.require: Test OnAppStarted method
 */
HWTEST_F(AppMgrServiceInnerTenthTest, OnAppStarted_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnAppStarted_002 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    const int32_t testRecordId = 1234;
    const std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, testRecordId, processName);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = appRecord;
    appRecord->priorityObject_ = nullptr;
    appMgrServiceInner->OnAppStarted(appRecord);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().getPriorityObjectCalled_);
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().getPidCall_);
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    TAG_LOGI(AAFwkTag::TEST, "OnAppStarted_002 end");
}

/**
 * @tc.name: OnAppStarted_003
 * @tc.desc: Test OnAppStarted function
 * @tc.type: FUNC
 * @tc.require: Test OnAppStarted method
 */
HWTEST_F(AppMgrServiceInnerTenthTest, OnAppStarted_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnAppStarted_003 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    const int32_t testRecordId = 1234;
    const std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, testRecordId, processName);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = appRecord;
    appRecord->priorityObject_ = std::make_shared<AppExecFwk::PriorityObject>();
    appMgrServiceInner->OnAppStarted(appRecord);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().getPriorityObjectCalled_);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().getPidCall_);
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    TAG_LOGI(AAFwkTag::TEST, "OnAppStarted_003 end");
}

/**
 * @tc.name: AddMountPermission_001
 * @tc.desc: Test AddMountPermission
 * @tc.type: FUNC
 * @tc.require: Test AddMountPermission method
 */
HWTEST_F(AppMgrServiceInnerTenthTest, AddMountPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AddMountPermission_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->remoteClientManager_ = nullptr; // Reset remote client manager
    uint32_t accessTokenId = 1001;
    std::set<std::string> permissions;
    appMgrServiceInner->AddMountPermission(accessTokenId, permissions);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getSpawnClientCall_, 0);
    TAG_LOGI(AAFwkTag::TEST, "AddMountPermission_001 end");
}

/**
 * @tc.name: AddMountPermission_002
 * @tc.desc: Test AddMountPermission
 * @tc.type: FUNC
 * @tc.require: Test AddMountPermission method
 */
HWTEST_F(AppMgrServiceInnerTenthTest, AddMountPermission_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AddMountPermission_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->remoteClientManager_ = std::make_shared<RemoteClientManager>();
    uint32_t accessTokenId = 1001;
    std::set<std::string> permissions;
    AAFwk::MyStatus::GetInstance().getSpawnClient_ = nullptr;
    appMgrServiceInner->AddMountPermission(accessTokenId, permissions);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getSpawnClientCall_, 1);
    TAG_LOGI(AAFwkTag::TEST, "AddMountPermission_002 end");
}

/**
 * @tc.name: AddMountPermission_003
 * @tc.desc: Test AddMountPermission
 * @tc.type: FUNC
 * @tc.require: Test AddMountPermission method
 */
HWTEST_F(AppMgrServiceInnerTenthTest, AddMountPermission_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AddMountPermission_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->remoteClientManager_ = std::make_shared<RemoteClientManager>();
    uint32_t accessTokenId = 1001;
    std::set<std::string> permissions;
    AAFwk::MyStatus::GetInstance().getSpawnClient_ = std::make_shared<AppSpawnClient>();
    appMgrServiceInner->AddMountPermission(accessTokenId, permissions);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getSpawnClientCall_, 2);
    TAG_LOGI(AAFwkTag::TEST, "AddMountPermission_003 end");
}

/**
 * @tc.name: AddMountPermission_004
 * @tc.desc: Test AddMountPermission
 * @tc.type: FUNC
 * @tc.require: Test AddMountPermission method
 */
HWTEST_F(AppMgrServiceInnerTenthTest, AddMountPermission_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AddMountPermission_004 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->remoteClientManager_ = std::make_shared<RemoteClientManager>();
    uint32_t accessTokenId = 1001;
    std::set<std::string> permissions;
    AAFwk::MyStatus::GetInstance().getSpawnClient_ = std::make_shared<AppSpawnClient>();
    appMgrServiceInner->AddMountPermission(accessTokenId, permissions);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getSpawnClientCall_, 3);
    EXPECT_EQ(permissions.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "AddMountPermission_004 end");
}

/**
 * @tc.name: SetAtomicServiceInfo_001
 * @tc.desc: Test SetAtomicServiceInfo method with successful account info retrieval
 * @tc.type: FUNC
 * @tc.require: Test SetAtomicServiceInfo method when errCode == ERR_OK
 */
HWTEST_F(AppMgrServiceInnerTenthTest, SetAtomicServiceInfo_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetAtomicServiceInfo_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AppSpawnStartMsg startMsg = {};
    startMsg.atomicServiceFlag = false; // Initially false
    BundleType bundleType = BundleType::ATOMIC_SERVICE;
#ifdef OHOS_ACCOUNT_ENABLED
    appMgrServiceInner->SetAtomicServiceInfo(bundleType, startMsg);
    EXPECT_TRUE(startMsg.atomicServiceFlag);
#else
    appMgrServiceInner->SetAtomicServiceInfo(bundleType, startMsg);
    EXPECT_FALSE(startMsg.atomicServiceFlag);
#endif
    TAG_LOGI(AAFwkTag::TEST, "SetAtomicServiceInfo_001 end");
}

/**
 * @tc.name: SetAtomicServiceInfo_002
 * @tc.desc: Test SetAtomicServiceInfo method with non-atomic service bundle type
 * @tc.type: FUNC
 * @tc.require: Test SetAtomicServiceInfo method when bundleType != ATOMIC_SERVICE
 */
HWTEST_F(AppMgrServiceInnerTenthTest, SetAtomicServiceInfo_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetAtomicServiceInfo_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AppSpawnStartMsg startMsg = {};
    startMsg.atomicServiceFlag = false; // Initially false
    BundleType bundleType = BundleType::APP;
    appMgrServiceInner->SetAtomicServiceInfo(bundleType, startMsg);
    EXPECT_FALSE(startMsg.atomicServiceFlag);
    TAG_LOGI(AAFwkTag::TEST, "SetAtomicServiceInfo_002 end");
}

/**
 * @tc.name: SetAppInfo_001
 * @tc.desc: Test SetAppInfo function with apiTargetVersion % API_VERSION_MOD < API15
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, SetAppInfo_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetAppInfo_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    BundleInfo bundleInfo;
    bundleInfo.applicationInfo.apiTargetVersion = 14; // 14 % 100 = 14 < 15
    bundleInfo.applicationInfo.maxChildProcess = 5;
    AppSpawnStartMsg startMsg;
    startMsg.maxChildProcess = 3; // Set to non-zero initially
    appMgrServiceInner->SetAppInfo(bundleInfo, startMsg);
    EXPECT_EQ(startMsg.maxChildProcess, 0);
    TAG_LOGI(AAFwkTag::TEST, "SetAppInfo_001 end");
}

/**
 * @tc.name: SetAppInfo_002
 * @tc.desc: Test SetAppInfo function with apiTargetVersion % API_VERSION_MOD >= API15
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, SetAppInfo_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetAppInfo_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    BundleInfo bundleInfo;
    bundleInfo.applicationInfo.apiTargetVersion = 115; // 115 % 100 = 15 >= 15
    bundleInfo.applicationInfo.maxChildProcess = 8;
    AppSpawnStartMsg startMsg;
    startMsg.maxChildProcess = 0; // Set to 0 initially to trigger the assignment
    appMgrServiceInner->SetAppInfo(bundleInfo, startMsg);
    EXPECT_EQ(startMsg.maxChildProcess, bundleInfo.applicationInfo.maxChildProcess);
    EXPECT_EQ(startMsg.maxChildProcess, 8);
    TAG_LOGI(AAFwkTag::TEST, "SetAppInfo_002 end");
}

/**
 * @tc.name: SetAppInfo_003
 * @tc.desc: Test SetAppInfo function with apiTargetVersion % API_VERSION_MOD >= API15 but maxChildProcess != 0
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, SetAppInfo_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetAppInfo_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    BundleInfo bundleInfo;
    bundleInfo.applicationInfo.apiTargetVersion = 215; // 215 % 100 = 15 >= 15
    bundleInfo.applicationInfo.maxChildProcess = 10;
    AppSpawnStartMsg startMsg;
    startMsg.maxChildProcess = 3; // Set to non-zero initially
    appMgrServiceInner->SetAppInfo(bundleInfo, startMsg);
    EXPECT_EQ(startMsg.maxChildProcess, 3);
    EXPECT_NE(startMsg.maxChildProcess, bundleInfo.applicationInfo.maxChildProcess);
    TAG_LOGI(AAFwkTag::TEST, "SetAppInfo_003 end");
}

/**
 * @tc.name: OnRemoteDied_001
 * @tc.desc: Test OnRemoteDied function
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, OnRemoteDied_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnRemoteDied_001 start");
    AAFwk::MyStatus::GetInstance().resetModuleRunningFlags();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    const int32_t testRecordId = 1234;
    const std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, testRecordId, processName);
    AAFwk::MyStatus::GetInstance().runningRecord_ = appRecord;
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().getBundleNameCalled_);
    appMgrServiceInner->OnRemoteDied(nullptr, false, false);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().getBundleNameCalled_);
    TAG_LOGI(AAFwkTag::TEST, "OnRemoteDied_001 end");
}

/**
 * @tc.name: OnRemoteDied_002
 * @tc.desc: Test OnRemoteDied function
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, OnRemoteDied_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnRemoteDied_002 start");
    AAFwk::MyStatus::GetInstance().resetModuleRunningFlags();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    const int32_t testRecordId = 1234;
    const std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, testRecordId, processName);
    AAFwk::MyStatus::GetInstance().runningRecord_ = appRecord;
    AAFwk::MyStatus::GetInstance().isStartSpecifiedAbility_ = true;
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getSpecifiedRequestIdCall_, 0);
    appMgrServiceInner->startSpecifiedAbilityResponse_ = sptr<MyStartSpecifiedAbilityResponse>::MakeSptr();
    appMgrServiceInner->OnRemoteDied(nullptr, false, false);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getSpecifiedRequestIdCall_, 1);
    TAG_LOGI(AAFwkTag::TEST, "OnRemoteDied_002 end");
}

/**
 * @tc.name: OnRemoteDied_003
 * @tc.desc: Test OnRemoteDied function
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, OnRemoteDied_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnRemoteDied_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    const int32_t testRecordId = 1234;
    const std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, testRecordId, processName);
    AAFwk::MyStatus::GetInstance().runningRecord_ = appRecord;
    AAFwk::MyStatus::GetInstance().isNewProcessRequest_ = true;
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getNewProcessRequestIdCall_, 0);
    appMgrServiceInner->startSpecifiedAbilityResponse_ = sptr<MyStartSpecifiedAbilityResponse>::MakeSptr();
    appMgrServiceInner->OnRemoteDied(nullptr, false, false);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getNewProcessRequestIdCall_, 1);
    TAG_LOGI(AAFwkTag::TEST, "OnRemoteDied_003 end");
}

/**
 * @tc.name: HandleTimeOut_001
 * @tc.desc: Test HandleTimeOut function
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, HandleTimeOut_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleTimeOut_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    const int32_t testRecordId = 1234;
    const std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, testRecordId, processName);
    AAFwk::MyStatus::GetInstance().runningRecord_ = appRecord;
    AAFwk::EventWrap innerEvent(AppExecFwk::AMSEventHandler::TERMINATE_APPLICATION_TIMEOUT_MSG);
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().setStateCalled_);
    appMgrServiceInner->HandleTimeOut(innerEvent);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().setStateCalled_);
    TAG_LOGI(AAFwkTag::TEST, "HandleTimeOut_001 end");
}

/**
 * @tc.name: HandleTimeOut_002
 * @tc.desc: Test HandleTimeOut function
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, HandleTimeOut_002, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleTimeOut_002 start");
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    const int32_t testRecordId = 1234;
    const std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, testRecordId, processName);
    AAFwk::MyStatus::GetInstance().runningRecord_ = appRecord;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().resetNewProcessRequestCall_, 0);
    AAFwk::EventWrap innerEvent(AMSEventHandler::START_SPECIFIED_PROCESS_TIMEOUT_MSG);
    appMgrServiceInner->HandleTimeOut(innerEvent);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().resetNewProcessRequestCall_, 1);
    TAG_LOGI(AAFwkTag::TEST, "HandleTimeOut_002 end");
}

/**
 * @tc.name: HandleTimeOut_003
 * @tc.desc: Test HandleTimeOut function
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, HandleTimeOut_003, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleTimeOut_003 start");
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    const int32_t testRecordId = 1234;
    const std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, testRecordId, processName);
    AAFwk::MyStatus::GetInstance().runningRecord_ = appRecord;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    AAFwk::EventWrap innerEvent(AMSEventHandler::START_PROCESS_SPECIFIED_ABILITY_TIMEOUT_MSG);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().resetSpecifiedRequestCall_, 0);
    appMgrServiceInner->HandleTimeOut(innerEvent);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().resetSpecifiedRequestCall_, 1);
    TAG_LOGI(AAFwkTag::TEST, "HandleTimeOut_003 end");
}

/**
 * @tc.name: HandleTimeOut_004
 * @tc.desc: Test HandleTimeOut function
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, HandleTimeOut_004, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleTimeOut_004 start");
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    const int32_t testRecordId = 1234;
    const std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, testRecordId, processName);
    AAFwk::MyStatus::GetInstance().runningRecord_ = appRecord;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    AAFwk::EventWrap innerEvent(AMSEventHandler::START_SPECIFIED_ABILITY_TIMEOUT_MSG);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().resetSpecifiedRequestCall_, 0);
    appMgrServiceInner->HandleTimeOut(innerEvent);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().resetSpecifiedRequestCall_, 1);
    TAG_LOGI(AAFwkTag::TEST, "HandleTimeOut_004 end");
}

/**
 * @tc.name: KillApplicationByRecord_001
 * @tc.desc: Test KillApplicationByRecord function
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, KillApplicationByRecord_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "KillApplicationByRecord_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    const int32_t testRecordId = 1234;
    const std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, testRecordId, processName);
    appMgrServiceInner->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler("test_queueApplicationByRecord");
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().getPidCall_);
    appMgrServiceInner->KillApplicationByRecord(appRecord);
    appMgrServiceInner->taskHandler_.reset();
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().getPidCall_);
    TAG_LOGI(AAFwkTag::TEST, "KillApplicationByRecord_001 end");
}

/**
 * @tc.name: OnChildProcessRemoteDied_001
 * @tc.desc: Test OnChildProcessRemoteDied function
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, OnChildProcessRemoteDied_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "OnChildProcessRemoteDied_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getChildProcessRecordByPidCall_, 0);
    appMgrServiceInner->OnChildProcessRemoteDied(nullptr);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getChildProcessRecordByPidCall_, 1);
    TAG_LOGI(AAFwkTag::TEST, "OnChildProcessRemoteDied_001 end");
}

/**
 * @tc.name: ExitChildProcessSafelyByChildPid_001
 * @tc.desc: Test ExitChildProcessSafelyByChildPid function with child process pid
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, ExitChildProcessSafelyByChildPid_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ExitChildProcessSafelyByChildPid_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getAppRunningProcessPidCall_, 0);
    appMgrServiceInner->ExitChildProcessSafelyByChildPid(0);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getAppRunningProcessPidCall_, 0);
    appMgrServiceInner->ExitChildProcessSafelyByChildPid(1);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getAppRunningProcessPidCall_, 1);
    TAG_LOGI(AAFwkTag::TEST, "ExitChildProcessSafelyByChildPid_001 end");
}

/**
 * @tc.name: ExitChildProcessSafelyByChildPid_002
 * @tc.desc: Test ExitChildProcessSafelyByChildPid function with child process type
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, ExitChildProcessSafelyByChildPid_002, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ExitChildProcessSafelyByChildPid_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    AAFwk::MyStatus::GetInstance().getAppRunningProcessPid_ = nullptr;
    ChildProcessRequest request;
    AAFwk::MyStatus::GetInstance().getChildProcessRecordByPid_ =
        std::make_shared<ChildProcessRecord>(0, request, nullptr);
    AAFwk::MyStatus::GetInstance().getChildProcessRecordByPid_->childProcessType_ = CHILD_PROCESS_TYPE_NATIVE;
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getChildProcessRecordByPidCall_, 0);
    appMgrServiceInner->ExitChildProcessSafelyByChildPid(1);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getChildProcessRecordByPidCall_, 0);
    AAFwk::MyStatus::GetInstance().getAppRunningProcessPid_ = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    appMgrServiceInner->ExitChildProcessSafelyByChildPid(1);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getChildProcessRecordByPidCall_, 1);
    TAG_LOGI(AAFwkTag::TEST, "ExitChildProcessSafelyByChildPid_002 end");
}

/**
 * @tc.name: ExitChildProcessSafelyByChildPid_003
 * @tc.desc: Test ExitChildProcessSafelyByChildPid function with child process type
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, ExitChildProcessSafelyByChildPid_003, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ExitChildProcessSafelyByChildPid_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    AAFwk::MyStatus::GetInstance().getAppRunningProcessPid_ = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    ChildProcessRequest request;
    AAFwk::MyStatus::GetInstance().getChildProcessRecordByPid_ =
        std::make_shared<ChildProcessRecord>(0, request, nullptr);
    AAFwk::MyStatus::GetInstance().getChildProcessRecordByPid_->childProcessType_ = CHILD_PROCESS_TYPE_NATIVE;
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().removeChildProcessRecordCall_, 0);
    appMgrServiceInner->ExitChildProcessSafelyByChildPid(99999);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().removeChildProcessRecordCall_, 1);
    TAG_LOGI(AAFwkTag::TEST, "ExitChildProcessSafelyByChildPid_003 end");
}

/**
 * @tc.name: KillAttachedChildProcess_001
 * @tc.desc: Test member function
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, KillAttachedChildProcess_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "KillAttachedChildProcess_001 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getParentAppRecordCall_, 0);
    appMgrServiceInner->KillAttachedChildProcess(nullptr);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getParentAppRecordCall_, 0);
    appMgrServiceInner->KillAttachedChildProcess(std::make_shared<AppRunningRecord>(nullptr, 0, ""));
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getParentAppRecordCall_, 1);
    TAG_LOGI(AAFwkTag::TEST, "KillAttachedChildProcess_001 end");
}

/**
 * @tc.name: ClearProcessByToken_001
 * @tc.desc: Test ClearProcessByToken function with valid token
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, ClearProcessByToken_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ClearProcessByToken_001 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().getAbilityRunningRecordByTokenCalled_);
    appMgrServiceInner->ClearProcessByToken(nullptr);
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().getAbilityRunningRecordByTokenCalled_);
    sptr<IRemoteObject> token = new MockAppScheduler();
    appMgrServiceInner->ClearProcessByToken(token);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().getAbilityRunningRecordByTokenCalled_);
    TAG_LOGI(AAFwkTag::TEST, "ClearProcessByToken_001 end");
}

/**
 * @tc.name: ClearProcessByToken_002
 * @tc.desc: Test ClearProcessByToken function with valid token and appRunningManager
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, ClearProcessByToken_002, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ClearProcessByToken_002 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    AAFwk::MyStatus::GetInstance().getAppRunningByToken_ = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().setApplicationClientCalled_);
    sptr<IRemoteObject> token = new MockAppScheduler();
    appMgrServiceInner->ClearProcessByToken(token);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().setApplicationClientCalled_);
    TAG_LOGI(AAFwkTag::TEST, "ClearProcessByToken_002 end");
}

/**
 * @tc.name: ClearProcessByToken_003
 * @tc.desc: Test ClearProcessByToken function with null appRunningManager
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, ClearProcessByToken_003, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ClearProcessByToken_003 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = nullptr;
    sptr<IRemoteObject> token = new MockAppScheduler();
    appMgrServiceInner->ClearProcessByToken(token);
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().setApplicationClientCalled_);
    TAG_LOGI(AAFwkTag::TEST, "ClearProcessByToken_003 end");
}

/**
 * @tc.name: ClearData_001
 * @tc.desc: Test ClearData
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, ClearData_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ClearData_001 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->ClearData(nullptr);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getRecordIdCalled_, 0);
    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    appMgrServiceInner->ClearData(appRecord);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getRecordIdCalled_, 1);
    TAG_LOGI(AAFwkTag::TEST, "ClearData_001 end");
}

/**
 * @tc.name: ClearData_002
 * @tc.desc: Test ClearData
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, ClearData_002, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ClearData_002 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();

    BaseSharedBundleInfo baseSharedBundleInfo;
    baseSharedBundleInfo.bundleName = "111";
    baseSharedBundleInfo.versionCode = 1;
    AAFwk::MyStatus::GetInstance().isSACall_ = false;
    appMgrServiceInner->SetRunningSharedBundleList("1", {baseSharedBundleInfo});
    EXPECT_EQ(appMgrServiceInner->runningSharedBundleList_.size(), 1);
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "1";

    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, 0, "");
    appMgrServiceInner->ClearData(appRecord);
    EXPECT_EQ(appMgrServiceInner->runningSharedBundleList_.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "ClearData_002 end");
}

/**
 * @tc.name: ClearData_003
 * @tc.desc: Test ClearData
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, ClearData_003, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ClearData_003 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    BaseSharedBundleInfo baseSharedBundleInfo;
    baseSharedBundleInfo.bundleName = "111";
    baseSharedBundleInfo.versionCode = 1;
    appMgrServiceInner->SetRunningSharedBundleList("1", {baseSharedBundleInfo});
    AAFwk::MyStatus::GetInstance().isSACall_ = true;
    AAFwk::MyStatus::GetInstance().isGetAppRunningByBundleName_ = true;
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "1";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, 0, "");
    appMgrServiceInner->ClearData(appRecord);
    EXPECT_EQ(appMgrServiceInner->runningSharedBundleList_.size(), 1);
    TAG_LOGI(AAFwkTag::TEST, "ClearData_003 end");
}

/**
 * @tc.name: SetAppAssertionPauseState_001
 * @tc.desc: Test SetAppAssertionPauseState
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, SetAppAssertionPauseState_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "SetAppAssertionPauseState_001 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    AAFwk::MyStatus::GetInstance().getDialogEnabled_ = true;
    AAFwk::MyStatus::GetInstance().getBoolParameter_ = true;
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getCallingPid_, 0);
    appMgrServiceInner->SetAppAssertionPauseState(true);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getCallingPid_, 1);
    TAG_LOGI(AAFwkTag::TEST, "SetAppAssertionPauseState_001 end");
}

/**
 * @tc.name: SetAppAssertionPauseState_002
 * @tc.desc: Test SetAppAssertionPauseState
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, SetAppAssertionPauseState_002, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "SetAppAssertionPauseState_002 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    AAFwk::MyStatus::GetInstance().getDialogEnabled_ = false;
    AAFwk::MyStatus::GetInstance().getBoolParameter_ = true;
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getCallingPid_, 0);
    appMgrServiceInner->SetAppAssertionPauseState(true);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getCallingPid_, 0);
    AAFwk::MyStatus::GetInstance().getDialogEnabled_ = true;
    appMgrServiceInner->SetAppAssertionPauseState(true);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getCallingPid_, 1);
    TAG_LOGI(AAFwkTag::TEST, "SetAppAssertionPauseState_002 end");
}

/**
 * @tc.name: SetAppAssertionPauseState_003
 * @tc.desc: Test SetAppAssertionPauseState
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, SetAppAssertionPauseState_003, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "SetAppAssertionPauseState_003 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    AAFwk::MyStatus::GetInstance().getDialogEnabled_ = true;
    AAFwk::MyStatus::GetInstance().getBoolParameter_ = false;
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getCallingPid_, 0);
    appMgrServiceInner->SetAppAssertionPauseState(true);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getCallingPid_, 0);
    AAFwk::MyStatus::GetInstance().getBoolParameter_ = true;
    appMgrServiceInner->SetAppAssertionPauseState(true);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getCallingPid_, 1);
    TAG_LOGI(AAFwkTag::TEST, "SetAppAssertionPauseState_003 end");
}

/**
 * @tc.name: SetAppAssertionPauseState_004
 * @tc.desc: Test SetAppAssertionPauseState
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, SetAppAssertionPauseState_004, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "SetAppAssertionPauseState_004 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    AAFwk::MyStatus::GetInstance().getDialogEnabled_ = true;
    AAFwk::MyStatus::GetInstance().getBoolParameter_ = true;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_->SetDebugApp(false);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_->isAttachDebug_ = false;
    appMgrServiceInner->SetAppAssertionPauseState(true);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().getPriorityObjectCalled_);
    TAG_LOGI(AAFwkTag::TEST, "SetAppAssertionPauseState_004 end");
}

/**
 * @tc.name: SetAppAssertionPauseState_005
 * @tc.desc: Test SetAppAssertionPauseState
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, SetAppAssertionPauseState_005, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "SetAppAssertionPauseState_005 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    appMgrServiceInner->abilityDebugResponse_ = sptr<MyAbilityDebugResponse>::MakeSptr();
    AAFwk::MyStatus::GetInstance().getDialogEnabled_ = true;
    AAFwk::MyStatus::GetInstance().getBoolParameter_ = true;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_->SetDebugApp(false);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_->isAttachDebug_ = false;
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().getAbilitiesCalled_);
    appMgrServiceInner->SetAppAssertionPauseState(true);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().getAbilitiesCalled_);
    TAG_LOGI(AAFwkTag::TEST, "SetAppAssertionPauseState_005 end");
}

/**
 * @tc.name: NotifyAppPreCache_001
 * @tc.desc: Test NotifyAppPreCache
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, NotifyAppPreCache_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyAppPreCache_001 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto mockCallback = new MockIAppStateCallback();
    appMgrServiceInner->appStateCallbacks_.push_back(
            AppMgrServiceInner::AppStateCallbackWithUserId{mockCallback, -1});
    appMgrServiceInner->NotifyAppPreCache(1, 1);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().notifyAppPreCacheCalled_);
    TAG_LOGI(AAFwkTag::TEST, "NotifyAppPreCache_001 end");
}

/**
 * @tc.name: NotifyAppPreCache_002
 * @tc.desc: Test NotifyAppPreCache
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, NotifyAppPreCache_002, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyAppPreCache_002 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto mockCallback = sptr<MockIAppStateCallback>::MakeSptr();
    appMgrServiceInner->NotifyAppPreCache(1, 1);
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().notifyAppPreCacheCalled_);
    TAG_LOGI(AAFwkTag::TEST, "NotifyAppPreCache_002 end");
}

/**
 * @tc.name: NotifyStartResidentProcess_001
 * @tc.desc: Test NotifyStartResidentProcess
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, NotifyStartResidentProcess_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyStartResidentProcess_001 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto mockCallback = sptr<MockIAppStateCallback>::MakeSptr();
    appMgrServiceInner->RegisterAppStateCallback(mockCallback);
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    AppExecFwk::BundleInfo bundleInfo;
    bundleInfos.push_back(bundleInfo);
    appMgrServiceInner->NotifyStartResidentProcess(bundleInfos);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().notifyStartResidentProcessCalled_);
    TAG_LOGI(AAFwkTag::TEST, "NotifyStartResidentProcess_001 end");
}

/**
 * @tc.name: NotifyStartResidentProcess_002
 * @tc.desc: Test NotifyStartResidentProcess
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, NotifyStartResidentProcess_002, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyStartResidentProcess_002 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto mockCallback = sptr<MockIAppStateCallback>::MakeSptr();
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    AppExecFwk::BundleInfo bundleInfo;
    bundleInfos.push_back(bundleInfo);
    appMgrServiceInner->NotifyStartResidentProcess(bundleInfos);
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().notifyStartResidentProcessCalled_);
    TAG_LOGI(AAFwkTag::TEST, "NotifyStartResidentProcess_002 end");
}

/**
 * @tc.name: RecordAppExitSignalReason_001
 * @tc.desc: Test RecordAppExitSignalReason
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, RecordAppExitSignalReason_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "RecordAppExitSignalReason_001 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto mockCallback = sptr<MockIAppStateCallback>::MakeSptr();
    appMgrServiceInner->RegisterAppStateCallback(mockCallback);
    int32_t pid = 12345;
    int32_t uid = 1000;
    int32_t signal = 9;
    std::string bundleName = "com.example.test";
    appMgrServiceInner->RecordAppExitSignalReason(pid, uid, signal, bundleName);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().recordAppExitSignalReasonCall_);
    TAG_LOGI(AAFwkTag::TEST, "RecordAppExitSignalReason_001 end");
}

/**
 * @tc.name: NotifyStartKeepAliveProcess_001
 * @tc.desc: Test NotifyStartKeepAliveProcess
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, NotifyStartKeepAliveProcess_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyStartKeepAliveProcess_001 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto mockCallback = sptr<MockIAppStateCallback>::MakeSptr();
    appMgrServiceInner->RegisterAppStateCallback(mockCallback);
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    AppExecFwk::BundleInfo bundleInfo;
    bundleInfos.push_back(bundleInfo);
    appMgrServiceInner->NotifyStartKeepAliveProcess(bundleInfos);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().notifyStartKeepAliveProcessCalled_);
    TAG_LOGI(AAFwkTag::TEST, "NotifyStartKeepAliveProcess_001 end");
}

/**
 * @tc.name: NotifyStartKeepAliveProcess_002
 * @tc.desc: Test NotifyStartKeepAliveProcess
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, NotifyStartKeepAliveProcess_002, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyStartKeepAliveProcess_002 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto mockCallback = sptr<MockIAppStateCallback>::MakeSptr();
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    AppExecFwk::BundleInfo bundleInfo;
    bundleInfos.push_back(bundleInfo);
    appMgrServiceInner->NotifyStartKeepAliveProcess(bundleInfos);
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().notifyStartKeepAliveProcessCalled_);
    TAG_LOGI(AAFwkTag::TEST, "NotifyStartKeepAliveProcess_002 end");
}

/**
 * @tc.name: SetKeepAliveEnableState_001
 * @tc.desc: Test SetKeepAliveEnableState
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, SetKeepAliveEnableState_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "SetKeepAliveEnableState_001 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getCallingUidCalledTimes_, 0);
    appMgrServiceInner->SetKeepAliveEnableState("not_empty", true, 0);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getCallingUidCalledTimes_, 1);
    TAG_LOGI(AAFwkTag::TEST, "SetKeepAliveEnableState_001 end");
}

/**
 * @tc.name: SetKeepAliveEnableState_002
 * @tc.desc: Test SetKeepAliveEnableState
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, SetKeepAliveEnableState_002, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "SetKeepAliveEnableState_002 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->SetKeepAliveEnableState("not_empty", true, 0);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getCallingUidCalledTimes_, 0);
    TAG_LOGI(AAFwkTag::TEST, "SetKeepAliveEnableState_002 end");
}

/**
 * @tc.name: SetKeepAliveEnableState_003
 * @tc.desc: Test SetKeepAliveEnableState
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, SetKeepAliveEnableState_003, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "SetKeepAliveEnableState_003 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    appMgrServiceInner->SetKeepAliveEnableState("", true, 0); //empty bundleName
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getCallingUidCalledTimes_, 0);
    TAG_LOGI(AAFwkTag::TEST, "SetKeepAliveEnableState_003 end");
}

/**
 * @tc.name: SetKeepAliveEnableState_004
 * @tc.desc: Test SetKeepAliveEnableState
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, SetKeepAliveEnableState_004, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "SetKeepAliveEnableState_004 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    AAFwk::MyStatus::GetInstance().getCallingUid_ = FOUNDATION_UID;
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getAppRunningRecordMapCall_, 0);
    appMgrServiceInner->SetKeepAliveEnableState("not_empty", true, 0);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getAppRunningRecordMapCall_, 1);
    TAG_LOGI(AAFwkTag::TEST, "SetKeepAliveEnableState_004 end");
}

/**
 * @tc.name: SetKeepAliveEnableState_005
 * @tc.desc: Test SetKeepAliveEnableState
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, SetKeepAliveEnableState_005, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "SetKeepAliveEnableState_005 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    AAFwk::MyStatus::GetInstance().getCallingUid_ = FOUNDATION_UID + 1;
    appMgrServiceInner->SetKeepAliveEnableState("not_empty", true, 0);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getAppRunningRecordMapCall_, 0);
    TAG_LOGI(AAFwkTag::TEST, "SetKeepAliveEnableState_005 end");
}

/**
 * @tc.name: SetKeepAliveEnableState_006
 * @tc.desc: Test SetKeepAliveEnableState
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, SetKeepAliveEnableState_006, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "SetKeepAliveEnableState_006 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    AAFwk::MyStatus::GetInstance().getCallingUid_ = FOUNDATION_UID;
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    const int32_t testRecordId = 1234;
    const std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, testRecordId, processName);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = appRecord;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.clear();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.insert({testRecordId, appRecord});
    appMgrServiceInner->SetKeepAliveEnableState("test.bundle.name", true, 0);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().setKeepAliveEnableStateCalled_);
    TAG_LOGI(AAFwkTag::TEST, "SetKeepAliveEnableState_006 end");
}

/**
 * @tc.name: SetKeepAliveEnableState_007
 * @tc.desc: Test SetKeepAliveEnableState
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, SetKeepAliveEnableState_007, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "SetKeepAliveEnableState_007 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    AAFwk::MyStatus::GetInstance().getCallingUid_ = FOUNDATION_UID;
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    const int32_t testRecordId = 1234;
    const std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, testRecordId, processName);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = appRecord;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.clear();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.insert({testRecordId, appRecord});
    appMgrServiceInner->SetKeepAliveEnableState("test.bundle.name", true, 2);
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().setKeepAliveEnableStateCalled_);
    TAG_LOGI(AAFwkTag::TEST, "SetKeepAliveEnableState_007 end");
}

/**
 * @tc.name: SetKeepAliveDkv_001
 * @tc.desc: Test SetKeepAliveDkv
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, SetKeepAliveDkv_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "SetKeepAliveDkv_001 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getCallingUidCalledTimes_, 0);
    appMgrServiceInner->SetKeepAliveDkv("not_empty", true, 0);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getCallingUidCalledTimes_, 1);
    TAG_LOGI(AAFwkTag::TEST, "SetKeepAliveDkv_001 end");
}

/**
 * @tc.name: SetKeepAliveDkv_002
 * @tc.desc: Test SetKeepAliveDkv
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, SetKeepAliveDkv_002, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "SetKeepAliveDkv_002 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->SetKeepAliveDkv("not_empty", true, 0);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getCallingUidCalledTimes_, 0);
    TAG_LOGI(AAFwkTag::TEST, "SetKeepAliveDkv_002 end");
}

/**
 * @tc.name: SetKeepAliveDkv_003
 * @tc.desc: Test SetKeepAliveDkv
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, SetKeepAliveDkv_003, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "SetKeepAliveDkv_003 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    appMgrServiceInner->SetKeepAliveDkv("", true, 0); //empty bundleName
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getCallingUidCalledTimes_, 0);
    TAG_LOGI(AAFwkTag::TEST, "SetKeepAliveDkv_003 end");
}

/**
 * @tc.name: SetKeepAliveDkv_004
 * @tc.desc: Test SetKeepAliveDkv
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, SetKeepAliveDkv_004, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "SetKeepAliveDkv_004 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    AAFwk::MyStatus::GetInstance().getCallingUid_ = FOUNDATION_UID;
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getAppRunningRecordMapCall_, 0);
    appMgrServiceInner->SetKeepAliveDkv("not_empty", true, 0);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getAppRunningRecordMapCall_, 1);
    TAG_LOGI(AAFwkTag::TEST, "SetKeepAliveDkv_004 end");
}

/**
 * @tc.name: SetKeepAliveDkv_005
 * @tc.desc: Test SetKeepAliveDkv
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, SetKeepAliveDkv_005, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "SetKeepAliveDkv_005 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    AAFwk::MyStatus::GetInstance().getCallingUid_ = FOUNDATION_UID + 1;
    appMgrServiceInner->SetKeepAliveDkv("not_empty", true, 0);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getAppRunningRecordMapCall_, 0);
    TAG_LOGI(AAFwkTag::TEST, "SetKeepAliveDkv_005 end");
}

/**
 * @tc.name: SetKeepAliveDkv_006
 * @tc.desc: Test SetKeepAliveDkv
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, SetKeepAliveDkv_006, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "SetKeepAliveDkv_006 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    AAFwk::MyStatus::GetInstance().getCallingUid_ = FOUNDATION_UID;
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    const int32_t testRecordId = 1234;
    const std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, testRecordId, processName);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.clear();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.insert({testRecordId, appRecord});
    appRecord->isMainElementRunning_ = true;
    appMgrServiceInner->SetKeepAliveDkv("test.bundle.name", true, 0);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().setKeepAliveDkvCalled_);
    TAG_LOGI(AAFwkTag::TEST, "SetKeepAliveDkv_006 end");
}

/**
 * @tc.name: SetKeepAliveDkv_007
 * @tc.desc: Test SetKeepAliveDkv
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, SetKeepAliveDkv_007, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "SetKeepAliveDkv_007 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    AAFwk::MyStatus::GetInstance().getCallingUid_ = FOUNDATION_UID;
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    const int32_t testRecordId = 1234;
    const std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, testRecordId, processName);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = appRecord;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.clear();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.insert({testRecordId, appRecord});
    appMgrServiceInner->SetKeepAliveDkv("test.bundle.name", true, 2);
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().setKeepAliveDkvCalled_);
    TAG_LOGI(AAFwkTag::TEST, "SetKeepAliveDkv_007 end");
}

/**
 * @tc.name: SetKeepAliveDkv_008
 * @tc.desc: Test SetKeepAliveDkv
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, SetKeepAliveDkv_008, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "SetKeepAliveDkv_008 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    AAFwk::MyStatus::GetInstance().getCallingUid_ = FOUNDATION_UID;
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    const int32_t testRecordId = 1234;
    const std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, testRecordId, processName);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.clear();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.insert({testRecordId, appRecord});
    appRecord->isMainElementRunning_ = false;
    appMgrServiceInner->SetKeepAliveDkv("test.bundle.name", true, 0);
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().setKeepAliveDkvCalled_);
    TAG_LOGI(AAFwkTag::TEST, "SetKeepAliveDkv_008 end");
}

/**
 * @tc.name: CacheLoadAbilityTask_001
 * @tc.desc: Test CacheLoadAbilityTask
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, CacheLoadAbilityTask_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "CacheLoadAbilityTask_001 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::function<void()> loadAbilityTaskFunc = []() {
        TAG_LOGI(AAFwkTag::TEST, "Load ability task function called");
    };
    EXPECT_EQ(appMgrServiceInner->loadAbilityTaskFuncList_.size(), 0);
    appMgrServiceInner->CacheLoadAbilityTask(std::move(loadAbilityTaskFunc));
    EXPECT_EQ(appMgrServiceInner->loadAbilityTaskFuncList_.size(), 1);
    TAG_LOGI(AAFwkTag::TEST, "CacheLoadAbilityTask_001 end");
}

/**
 * @tc.name: SubmitCacheLoadAbilityTask_001
 * @tc.desc: Test SubmitCacheLoadAbilityTask
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, SubmitCacheLoadAbilityTask_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "SubmitCacheLoadAbilityTask_001 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::function<void()> loadAbilityTaskFunc = []() {
        TAG_LOGI(AAFwkTag::TEST, "Load ability task function called");
    };
    EXPECT_EQ(appMgrServiceInner->loadAbilityTaskFuncList_.size(), 0);
    appMgrServiceInner->CacheLoadAbilityTask(std::move(loadAbilityTaskFunc));
    EXPECT_EQ(appMgrServiceInner->loadAbilityTaskFuncList_.size(), 1);
    appMgrServiceInner->SubmitCacheLoadAbilityTask();
    EXPECT_EQ(appMgrServiceInner->loadAbilityTaskFuncList_.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "SubmitCacheLoadAbilityTask_001 end");
}

/**
 * @tc.name: KillProcessDependedOnWeb_001
 * @tc.desc: Test KillProcessDependedOnWeb
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, KillProcessDependedOnWeb_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "KillProcessDependedOnWeb_001 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->KillProcessDependedOnWeb();
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getAppRunningRecordMapCall_, 0);
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    appMgrServiceInner->KillProcessDependedOnWeb();
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getAppRunningRecordMapCall_, 1);
    TAG_LOGI(AAFwkTag::TEST, "KillProcessDependedOnWeb_001 end");
}

/**
 * @tc.name: KillProcessDependedOnWeb_002
 * @tc.desc: Test KillProcessDependedOnWeb
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, KillProcessDependedOnWeb_002, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "KillProcessDependedOnWeb_002 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 1234, "test_process");
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.clear();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.insert({1234, appRecord});
    appMgrServiceInner->KillProcessDependedOnWeb();
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().getBundleNameCalled_);
    TAG_LOGI(AAFwkTag::TEST, "KillProcessDependedOnWeb_002 end");
}

/**
 * @tc.name: KillProcessDependedOnWeb_003
 * @tc.desc: Test KillProcessDependedOnWeb
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, KillProcessDependedOnWeb_003, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "KillProcessDependedOnWeb_003 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 1234, "test_process");
    appRecord->SetSpawned();
    appRecord->SetIsDependedOnArkWeb(true);
    appRecord->priorityObject_ = std::make_shared<PriorityObject>();
    AAFwk::MyStatus::GetInstance().isKeepAliveApp_ = true;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.clear();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.insert({1234, appRecord});
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().getUidCalled_);
    appMgrServiceInner->KillProcessDependedOnWeb();
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().getUidCalled_);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().getBundleNameCalled_);
    TAG_LOGI(AAFwkTag::TEST, "KillProcessDependedOnWeb_003 end");
}

/**
 * @tc.name: KillProcessDependedOnWeb_004
 * @tc.desc: Test KillProcessDependedOnWeb
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, KillProcessDependedOnWeb_004, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "KillProcessDependedOnWeb_004 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 1234, "test_process");
    AAFwk::MyStatus::GetInstance().isKeepAliveApp_ = false;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.clear();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.insert({1234, appRecord});
    appMgrServiceInner->KillProcessDependedOnWeb();
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().getUidCalled_);
    TAG_LOGI(AAFwkTag::TEST, "KillProcessDependedOnWeb_004 end");
}

/**
 * @tc.name: BindUIExtensionProcess_001
 * @tc.desc: Test BindUIExtensionProcess with new pid and callerPid (first bind)
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, BindUIExtensionProcess_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "BindUIExtensionProcess_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, 1234, "test_process");
    UIExtensionProcessBindInfo bindInfo;
    bindInfo.pid = 1001;
    bindInfo.callerPid = 2001;
    bindInfo.uid = 1000;
    bindInfo.callerUid = 2000;
    bindInfo.callerBundleName = "caller.bundle.name";
    EXPECT_EQ(appMgrServiceInner->uiExtensionBindReleations_.size(), 0);
    appMgrServiceInner->BindUIExtensionProcess(appRecord, bindInfo);
    EXPECT_EQ(appMgrServiceInner->uiExtensionBindReleations_.size(), 1);
    TAG_LOGI(AAFwkTag::TEST, "BindUIExtensionProcess_001 end");
}

/**
 * @tc.name: BindUIExtensionProcess_002
 * @tc.desc: Test BindUIExtensionProcess with existing pid and callerPid (increment count)
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, BindUIExtensionProcess_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "BindUIExtensionProcess_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, 1234, "test_process");
    UIExtensionProcessBindInfo bindInfo;
    bindInfo.pid = 1001;
    bindInfo.callerPid = 2001;
    bindInfo.uid = 1000;
    bindInfo.callerUid = 2000;
    bindInfo.callerBundleName = "caller.bundle.name";
    appMgrServiceInner->uiExtensionBindReleations_[bindInfo.pid][bindInfo.callerPid] = 2;
    appMgrServiceInner->BindUIExtensionProcess(appRecord, bindInfo);
    EXPECT_EQ(appMgrServiceInner->uiExtensionBindReleations_[bindInfo.pid][bindInfo.callerPid], 3);
    TAG_LOGI(AAFwkTag::TEST, "BindUIExtensionProcess_002 end");
}

/**
 * @tc.name: UnBindUIExtensionProcess_001
 * @tc.desc: Test UnBindUIExtensionProcess with count going to 0 (erase innerIt)
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, UnBindUIExtensionProcess_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UnBindUIExtensionProcess_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, 1234, "test_process");
    UIExtensionProcessBindInfo bindInfo;
    bindInfo.pid = 1001;
    bindInfo.callerPid = 2001;
    bindInfo.uid = 1000;
    bindInfo.callerUid = 2000;
    bindInfo.callerBundleName = "caller.bundle.name";
    appMgrServiceInner->uiExtensionBindReleations_[bindInfo.pid][bindInfo.callerPid] = 1;
    appMgrServiceInner->uiExtensionBindReleations_[bindInfo.pid][3001] = 1;
    EXPECT_EQ(appMgrServiceInner->uiExtensionBindReleations_[bindInfo.pid].size(), 2);
    appMgrServiceInner->UnBindUIExtensionProcess(appRecord, bindInfo);
    EXPECT_EQ(appMgrServiceInner->uiExtensionBindReleations_[bindInfo.pid].size(), 1);
    TAG_LOGI(AAFwkTag::TEST, "UnBindUIExtensionProcess_001 end");
}

/**
 * @tc.name: UnBindUIExtensionProcess_002
 * @tc.desc: Test UnBindUIExtensionProcess with last callerPid (erase entire pid entry)
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, UnBindUIExtensionProcess_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UnBindUIExtensionProcess_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, 1234, "test_process");
    UIExtensionProcessBindInfo bindInfo;
    bindInfo.pid = 1001;
    bindInfo.callerPid = 2001;
    bindInfo.uid = 1000;
    bindInfo.callerUid = 2000;
    bindInfo.callerBundleName = "caller.bundle.name";
    appMgrServiceInner->uiExtensionBindReleations_[bindInfo.pid][bindInfo.callerPid] = 1;
    EXPECT_EQ(appMgrServiceInner->uiExtensionBindReleations_.size(), 1);
    appMgrServiceInner->UnBindUIExtensionProcess(appRecord, bindInfo);
    EXPECT_EQ(appMgrServiceInner->uiExtensionBindReleations_.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "UnBindUIExtensionProcess_002 end");
}

/**
 * @tc.name: AddUIExtensionBindItem_001
 * @tc.desc: Test AddUIExtensionBindItem with null parameters
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, AddUIExtensionBindItem_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AddUIExtensionBindItem_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    appMgrServiceInner->AddUIExtensionBindItem(nullptr, nullptr, nullptr);
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().addUIExtensionBindItemCalled_);

    TAG_LOGI(AAFwkTag::TEST, "AddUIExtensionBindItem_001 end");
}

/**
 * @tc.name: AddUIExtensionBindItem_002
 * @tc.desc: Test AddUIExtensionBindItem with no bind permission (second if return)
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, AddUIExtensionBindItem_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AddUIExtensionBindItem_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto want = std::make_shared<AAFwk::Want>();
    want->SetParam(UIEXTENSION_HOST_PID, -1);
    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 1, "test_process");
    sptr<IRemoteObject> token = new MockAppScheduler();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    want->SetParam(UIEXTENSION_NOTIFY_BIND, 0);
    appMgrServiceInner->AddUIExtensionBindItem(want, appRecord, token);
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().addUIExtensionBindItemCalled_);
    EXPECT_TRUE(want->HasParameter(UIEXTENSION_HOST_PID));
    TAG_LOGI(AAFwkTag::TEST, "AddUIExtensionBindItem_002 end");
}

/**
 * @tc.name: AddUIExtensionBindItem_003
 * @tc.desc: Test AddUIExtensionBindItem with WrapBindInfo failure (third if return)
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, AddUIExtensionBindItem_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AddUIExtensionBindItem_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto want = std::make_shared<AAFwk::Want>();
    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 1, "test_process");
    sptr<IRemoteObject> token = new MockAppScheduler();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    want->SetParam(UIEXTENSION_NOTIFY_BIND, 1);
    want->SetParam(UIEXTENSION_BIND_ABILITY_ID, -1); // Invalid ability ID
    want->SetParam(UIEXTENSION_HOST_PID, -1);
    want->SetParam(UIEXTENSION_HOST_UID, -1);
    want->SetParam(UIEXTENSION_HOST_BUNDLENAME, std::string("")); // Empty bundle name
    appRecord->priorityObject_ = nullptr;
    appMgrServiceInner->AddUIExtensionBindItem(want, appRecord, token);
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().addUIExtensionBindItemCalled_);
    EXPECT_TRUE(want->HasParameter(UIEXTENSION_HOST_PID));
    TAG_LOGI(AAFwkTag::TEST, "AddUIExtensionBindItem_003 end");
}

/**
 * @tc.name: AddUIExtensionBindItem_004
 * @tc.desc: Test AddUIExtensionBindItem successful execution (reaches end of function)
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, AddUIExtensionBindItem_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AddUIExtensionBindItem_004 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto want = std::make_shared<AAFwk::Want>();
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, 1, "test_process");
    sptr<IRemoteObject> token = new MockAppScheduler();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    want->SetParam(UIEXTENSION_NOTIFY_BIND, 1); // Has bind permission
    want->SetParam(UIEXTENSION_BIND_ABILITY_ID, 123); // Valid ability ID
    want->SetParam(UIEXTENSION_HOST_PID, 2001); // Valid caller PID
    want->SetParam(UIEXTENSION_HOST_UID, 2000); // Valid caller UID
    want->SetParam(UIEXTENSION_HOST_BUNDLENAME, std::string("caller.bundle.name")); // Valid bundle name

    appRecord->priorityObject_ = std::make_shared<PriorityObject>();
    appRecord->priorityObject_->SetPid(1001);
    appRecord->SetUid(1000);
    EXPECT_TRUE(want->HasParameter(UIEXTENSION_HOST_PID));
    appMgrServiceInner->AddUIExtensionBindItem(want, appRecord, token);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().addUIExtensionBindItemCalled_);
    EXPECT_FALSE(want->HasParameter(UIEXTENSION_HOST_PID));
    TAG_LOGI(AAFwkTag::TEST, "AddUIExtensionBindItem_004 end");
}

/**
 * @tc.name: CheckCleanAbilityByUserRequest_001
 * @tc.desc: Test CheckCleanAbilityByUserRequest with null appRecord
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, CheckCleanAbilityByUserRequest_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckCleanAbilityByUserRequest_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new MockAppScheduler();
    auto abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 1);
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    appMgrServiceInner->CheckCleanAbilityByUserRequest(appRecord, abilityRecord,
        AbilityState::ABILITY_STATE_BACKGROUND);
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().getAbilityInfoCalled_);
    TAG_LOGI(AAFwkTag::TEST, "CheckCleanAbilityByUserRequest_001 end");
}

/**
 * @tc.name: CheckCleanAbilityByUserRequest_002
 * @tc.desc: Test CheckCleanAbilityByUserRequest with state != ABILITY_STATE_BACKGROUND (second if return)
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, CheckCleanAbilityByUserRequest_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckCleanAbilityByUserRequest_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, 1234, "test_process");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new MockAppScheduler();
    auto abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 1);
    appMgrServiceInner->CheckCleanAbilityByUserRequest(appRecord, abilityRecord,
        AbilityState::ABILITY_STATE_FOREGROUND);
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().getAbilityInfoCalled_);
    TAG_LOGI(AAFwkTag::TEST, "CheckCleanAbilityByUserRequest_002 end");
}

/**
 * @tc.name: CheckCleanAbilityByUserRequest_003
 * @tc.desc: Test CheckCleanAbilityByUserRequest with ability type != PAGE (third if return)
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, CheckCleanAbilityByUserRequest_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckCleanAbilityByUserRequest_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, 1234, "test_process");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AppExecFwk::AbilityType::SERVICE; // Not PAGE type
    sptr<IRemoteObject> token = new MockAppScheduler();
    auto abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 1);
    appMgrServiceInner->CheckCleanAbilityByUserRequest(appRecord, abilityRecord,
        AbilityState::ABILITY_STATE_BACKGROUND);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().getAbilityInfoCalled_);
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().isKeepAliveAppCalled_);

    TAG_LOGI(AAFwkTag::TEST, "CheckCleanAbilityByUserRequest_003 end");
}

/**
 * @tc.name: CheckCleanAbilityByUserRequest_004
 * @tc.desc: Test CheckCleanAbilityByUserRequest with KeepAlive app (fourth if return)
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, CheckCleanAbilityByUserRequest_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckCleanAbilityByUserRequest_004 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, 1234, "test_process");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AppExecFwk::AbilityType::PAGE; // Correct PAGE type
    sptr<IRemoteObject> token = new MockAppScheduler();
    auto abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 1);
    AAFwk::MyStatus::GetInstance().isKeepAliveApp_ = true;
    appMgrServiceInner->CheckCleanAbilityByUserRequest(appRecord, abilityRecord,
        AbilityState::ABILITY_STATE_BACKGROUND);
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().isAllAbilityReadyToCleanedByUserRequestCalled_);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().isKeepAliveAppCalled_);
    TAG_LOGI(AAFwkTag::TEST, "CheckCleanAbilityByUserRequest_004 end");
}

/**
 * @tc.name: CheckCleanAbilityByUserRequest_005
 * @tc.desc: Test CheckCleanAbilityByUserRequest with not ready to clean (fifth if return)
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, CheckCleanAbilityByUserRequest_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckCleanAbilityByUserRequest_005 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, 1234, "test_process");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AppExecFwk::AbilityType::PAGE; // Correct PAGE type
    sptr<IRemoteObject> token = new MockAppScheduler();
    auto abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 1);
    AAFwk::MyStatus::GetInstance().isKeepAliveApp_ = false;
    AAFwk::MyStatus::GetInstance().isAllAbilityReadyToCleanedByUserRequest_ = false; // Not ready to clean
    appMgrServiceInner->CheckCleanAbilityByUserRequest(appRecord, abilityRecord,
        AbilityState::ABILITY_STATE_BACKGROUND);
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().setUserRequestCleaningCalled_);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().isAllAbilityReadyToCleanedByUserRequestCalled_);
    TAG_LOGI(AAFwkTag::TEST, "CheckCleanAbilityByUserRequest_005 end");
}

/**
 * @tc.name: CheckCleanAbilityByUserRequest_006
 * @tc.desc: Test CheckCleanAbilityByUserRequest successful execution with null priority object
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, CheckCleanAbilityByUserRequest_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckCleanAbilityByUserRequest_006 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, 1234, "test_process");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AppExecFwk::AbilityType::PAGE; // Correct PAGE type
    sptr<IRemoteObject> token = new MockAppScheduler();
    auto abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 1);
    AAFwk::MyStatus::GetInstance().isKeepAliveApp_ = false;
    AAFwk::MyStatus::GetInstance().isAllAbilityReadyToCleanedByUserRequest_ = true; // Ready to clean
    appRecord->priorityObject_ = nullptr; // Null priority object (pid will be 0)
    appMgrServiceInner->CheckCleanAbilityByUserRequest(appRecord, abilityRecord,
        AbilityState::ABILITY_STATE_BACKGROUND);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().getPriorityObjectCalled_);
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().getPidCall_);
    TAG_LOGI(AAFwkTag::TEST, "CheckCleanAbilityByUserRequest_006 end");
}

/**
 * @tc.name: CheckCleanAbilityByUserRequest_007
 * @tc.desc: Test CheckCleanAbilityByUserRequest successful execution with valid priority object
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, CheckCleanAbilityByUserRequest_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckCleanAbilityByUserRequest_007 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, 1234, "test_process");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AppExecFwk::AbilityType::PAGE; // Correct PAGE type
    sptr<IRemoteObject> token = new MockAppScheduler();
    auto abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 1);
    AAFwk::MyStatus::GetInstance().isKeepAliveApp_ = false;
    AAFwk::MyStatus::GetInstance().isAllAbilityReadyToCleanedByUserRequest_ = true; // Ready to clean
    appRecord->priorityObject_ = std::make_shared<PriorityObject>();
    appRecord->priorityObject_->SetPid(5678); // Set specific PID
    appMgrServiceInner->CheckCleanAbilityByUserRequest(appRecord, abilityRecord,
        AbilityState::ABILITY_STATE_BACKGROUND);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().getPriorityObjectCalled_);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().getPidCall_);
    TAG_LOGI(AAFwkTag::TEST, "CheckCleanAbilityByUserRequest_007 end");
}

/**
 * @tc.name: RemoveUIExtensionBindItem_001
 * @tc.desc: Test RemoveUIExtensionBindItem with null appRecord (first if return)
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, RemoveUIExtensionBindItem_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoveUIExtensionBindItem_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    sptr<IRemoteObject> token = new MockAppScheduler();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    appMgrServiceInner->RemoveUIExtensionBindItem(appRecord, token);
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().getAbilityRunningRecordByTokenCalledAppRecord_);
    TAG_LOGI(AAFwkTag::TEST, "RemoveUIExtensionBindItem_001 end");
}

/**
 * @tc.name: RemoveUIExtensionBindItem_002
 * @tc.desc: Test RemoveUIExtensionBindItem with null abilityRunningRecord (second if return)
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, RemoveUIExtensionBindItem_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoveUIExtensionBindItem_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, 1234, "test_process");
    sptr<IRemoteObject> token = new MockAppScheduler();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    AAFwk::MyStatus::GetInstance().getAbilityRunningRecordByToken_ = nullptr;
    appMgrServiceInner->RemoveUIExtensionBindItem(appRecord, token);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().getAbilityRunningRecordByTokenCalledAppRecord_);
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().getAbilityInfoCalled_);
    TAG_LOGI(AAFwkTag::TEST, "RemoveUIExtensionBindItem_002 end");
}

/**
 * @tc.name: RemoveUIExtensionBindItem_003
 * @tc.desc: Test RemoveUIExtensionBindItem with null abilityInfo (third if return)
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, RemoveUIExtensionBindItem_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoveUIExtensionBindItem_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, 1234, "test_process");
    sptr<IRemoteObject> token = new MockAppScheduler();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    auto abilityRunningRecord = std::make_shared<AbilityRunningRecord>(nullptr, token, 1);
    AAFwk::MyStatus::GetInstance().getAbilityRunningRecordByToken_ = abilityRunningRecord;
    appMgrServiceInner->RemoveUIExtensionBindItem(appRecord, token);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().getAbilityRunningRecordByTokenCalledAppRecord_);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().getAbilityInfoCalled_);
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().getUIExtensionBindAbilityIdCalled_);
    TAG_LOGI(AAFwkTag::TEST, "RemoveUIExtensionBindItem_003 end");
}

/**
 * @tc.name: RemoveUIExtensionBindItem_004
 * @tc.desc: Test RemoveUIExtensionBindItem with non-UIExtension ability type (fourth if return)
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, RemoveUIExtensionBindItem_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoveUIExtensionBindItem_004 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, 1234, "test_process");
    sptr<IRemoteObject> token = new MockAppScheduler();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->extensionAbilityType = ExtensionAbilityType::SERVICE; // Not UI extension type
    auto abilityRunningRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 1);
    AAFwk::MyStatus::GetInstance().getAbilityRunningRecordByToken_ = abilityRunningRecord;
    appMgrServiceInner->RemoveUIExtensionBindItem(appRecord, token);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().getAbilityRunningRecordByTokenCalledAppRecord_);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().getAbilityInfoCalled_);
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().getUIExtensionBindAbilityIdCalled_);
    TAG_LOGI(AAFwkTag::TEST, "RemoveUIExtensionBindItem_004 end");
}

/**
 * @tc.name: RemoveUIExtensionBindItem_005
 * @tc.desc: Test RemoveUIExtensionBindItem with QueryUIExtensionBindItemById failure (fifth if return)
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, RemoveUIExtensionBindItem_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoveUIExtensionBindItem_005 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, 1234, "test_process");
    sptr<IRemoteObject> token = new MockAppScheduler();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->extensionAbilityType = ExtensionAbilityType::UI; // UI extension type
    auto abilityRunningRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 1);
    abilityRunningRecord->SetUIExtensionBindAbilityId(123); // Set valid bind ability ID
    AAFwk::MyStatus::GetInstance().getAbilityRunningRecordByToken_ = abilityRunningRecord;
    appMgrServiceInner->RemoveUIExtensionBindItem(appRecord, token);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().getUIExtensionBindAbilityIdCalled_);
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().removeUIExtensionBindItemByIdCalled_);
    TAG_LOGI(AAFwkTag::TEST, "RemoveUIExtensionBindItem_005 end");
}

/**
 * @tc.name: RemoveUIExtensionBindItem_006
 * @tc.desc: Test RemoveUIExtensionBindItem with no unbind permission (sixth if return)
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, RemoveUIExtensionBindItem_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoveUIExtensionBindItem_006 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, 1234, "test_process");
    sptr<IRemoteObject> token = new MockAppScheduler();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->extensionAbilityType = ExtensionAbilityType::UI; // UI extension type
    auto abilityRunningRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 1);
    abilityRunningRecord->SetUIExtensionBindAbilityId(123); // Set valid bind ability ID
    AAFwk::MyStatus::GetInstance().getAbilityRunningRecordByToken_ = abilityRunningRecord;
    UIExtensionProcessBindInfo bindInfo;
    bindInfo.notifyProcessBind = 0; // No unbind permission (sixth if condition)
    bindInfo.pid = 1001;
    bindInfo.callerPid = 2001;
    appMgrServiceInner->appRunningManager_->uiExtensionBindMap_[123] = bindInfo;
    AAFwk::MyStatus::GetInstance().notifyProcessBind_ = -1;
    appMgrServiceInner->RemoveUIExtensionBindItem(appRecord, token);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().getAbilityRunningRecordByTokenCalledAppRecord_);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().getAbilityInfoCalled_);
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().removeUIExtensionBindItemByIdCalled_);
    TAG_LOGI(AAFwkTag::TEST, "RemoveUIExtensionBindItem_006 end");
}

/**
 * @tc.name: RemoveUIExtensionBindItem_007
 * @tc.desc: Test RemoveUIExtensionBindItem successful execution (reaches end of function)
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, RemoveUIExtensionBindItem_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoveUIExtensionBindItem_007 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, 1234, "test_process");
    sptr<IRemoteObject> token = new MockAppScheduler();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->extensionAbilityType = ExtensionAbilityType::UI; // UI extension type
    auto abilityRunningRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 1);
    abilityRunningRecord->SetUIExtensionBindAbilityId(123); // Set valid bind ability ID
    AAFwk::MyStatus::GetInstance().getAbilityRunningRecordByToken_ = abilityRunningRecord;
    AAFwk::MyStatus::GetInstance().queryUIExtensionBindItemById_ = ERR_OK;
    appMgrServiceInner->uiExtensionBindReleations_[1001][2001] = 1;
    AAFwk::MyStatus::GetInstance().notifyProcessBind_ = 1; // Has unbind permission
    appMgrServiceInner->RemoveUIExtensionBindItem(appRecord, token);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().getAbilityRunningRecordByTokenCalledAppRecord_);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().getUIExtensionBindAbilityIdCalled_);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().removeUIExtensionBindItemByIdCalled_);
    TAG_LOGI(AAFwkTag::TEST, "RemoveUIExtensionBindItem_007 end");
}

/**
 * @tc.name: NotifyPreloadAbilityStateChanged_001
 * @tc.desc: Test NotifyPreloadAbilityStateChanged
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, NotifyPreloadAbilityStateChanged_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyPreloadAbilityStateChanged_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();

    auto ret = appMgrServiceInner->NotifyPreloadAbilityStateChanged(nullptr, true);
    EXPECT_EQ(ret, AAFwk::INVALID_CALLER_TOKEN);
    TAG_LOGI(AAFwkTag::TEST, "NotifyPreloadAbilityStateChanged_001 end");
}

/**
 * @tc.name: NotifyPreloadAbilityStateChanged_002
 * @tc.desc: Test NotifyPreloadAbilityStateChanged
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, NotifyPreloadAbilityStateChanged_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyPreloadAbilityStateChanged_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();

    AAFwk::MyStatus::GetInstance().getAppRunningByToken_ = nullptr;
    sptr<IRemoteObject> token = new MockAppScheduler();
    auto ret = appMgrServiceInner->NotifyPreloadAbilityStateChanged(token, true);
    EXPECT_NE(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "NotifyPreloadAbilityStateChanged_002 end");
}

/**
 * @tc.name: NotifyPreloadAbilityStateChanged_003
 * @tc.desc: Test NotifyPreloadAbilityStateChanged
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, NotifyPreloadAbilityStateChanged_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyPreloadAbilityStateChanged_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();

    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    AAFwk::MyStatus::GetInstance().getAppRunningByToken_ = appRecord;
    appRecord->SetPreForeground(true);
    sptr<IRemoteObject> token = new MockAppScheduler();
    auto ret = appMgrServiceInner->NotifyPreloadAbilityStateChanged(token, true);
    EXPECT_NE(ret, ERR_OK);

    AAFwk::MyStatus::GetInstance().getAppRunningByToken_ = nullptr;
    TAG_LOGI(AAFwkTag::TEST, "NotifyPreloadAbilityStateChanged_004 end");
}

/**
 * @tc.name: NotifyPreloadAbilityStateChanged_004
 * @tc.desc: Test NotifyPreloadAbilityStateChanged
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, NotifyPreloadAbilityStateChanged_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyPreloadAbilityStateChanged_004 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();

    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    AAFwk::MyStatus::GetInstance().getAppRunningByToken_ = appRecord;
    appRecord->SetPreForeground(true);
    sptr<IRemoteObject> token = new MockAppScheduler();
    auto ret = appMgrServiceInner->NotifyPreloadAbilityStateChanged(token, false);
    EXPECT_EQ(ret, ERR_OK);

    AAFwk::MyStatus::GetInstance().getAppRunningByToken_ = nullptr;
    TAG_LOGI(AAFwkTag::TEST, "NotifyPreloadAbilityStateChanged_004 end");
}

/**
 * @tc.name: NotifyPreloadAbilityStateChanged_005
 * @tc.desc: Test NotifyPreloadAbilityStateChanged
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, NotifyPreloadAbilityStateChanged_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyPreloadAbilityStateChanged_005 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();

    AAFwk::MyStatus::GetInstance().getAppRunningByToken_ = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    auto &abilitiesMap = AAFwk::MyStatus::GetInstance().abilitiesMap_;

    auto abilityInfo1 = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token1 = new MockAppScheduler();
    auto abilityRecord1 = std::make_shared<AbilityRunningRecord>(abilityInfo1, token1, 1);

    auto abilityInfo2 = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token2 = new MockAppScheduler();
    auto abilityRecord2 = std::make_shared<AbilityRunningRecord>(abilityInfo2, token2, 2);

    abilitiesMap[token1] = abilityRecord1;
    abilitiesMap[token2] = abilityRecord2;

    sptr<IRemoteObject> token = new MockAppScheduler();
    auto ret = appMgrServiceInner->NotifyPreloadAbilityStateChanged(token, true);
    EXPECT_NE(ret, ERR_OK);

    AAFwk::MyStatus::GetInstance().getAppRunningByToken_ = nullptr;
    TAG_LOGI(AAFwkTag::TEST, "NotifyPreloadAbilityStateChanged_005 end");
}

/**
 * @tc.name: NotifyPreloadAbilityStateChanged_006
 * @tc.desc: Test NotifyPreloadAbilityStateChanged
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, NotifyPreloadAbilityStateChanged_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyPreloadAbilityStateChanged_006 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();

    AAFwk::MyStatus::GetInstance().getAppRunningByToken_ = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    sptr<IRemoteObject> token = new MockAppScheduler();
    auto ret = appMgrServiceInner->NotifyPreloadAbilityStateChanged(token, true);
    EXPECT_EQ(ret, ERR_OK);

    AAFwk::MyStatus::GetInstance().getAppRunningByToken_ = nullptr;
    TAG_LOGI(AAFwkTag::TEST, "NotifyPreloadAbilityStateChanged_006 end");
}

/**
 * @tc.name: CheckPreloadAppRecordExist_001
 * @tc.desc: Test CheckPreloadAppRecordExist
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, CheckPreloadAppRecordExist_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckPreloadAppRecordExist_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    appMgrServiceInner->appPreloader_ = nullptr;

    auto ret = appMgrServiceInner->CheckPreloadAppRecordExist("com.test.demo", 100, 0);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "CheckPreloadAppRecordExist_001 end");
}

/**
 * @tc.name: CheckPreloadAppRecordExist_002
 * @tc.desc: Test CheckPreloadAppRecordExist
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, CheckPreloadAppRecordExist_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckPreloadAppRecordExist_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();

    auto ret = appMgrServiceInner->CheckPreloadAppRecordExist("com.test.demo", 100, 0);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "CheckPreloadAppRecordExist_002 end");
}

/**
 * @tc.name: CheckAppRecordExistByPreloadRequest_001
 * @tc.desc: Test CheckAppRecordExistByPreloadRequest
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, CheckAppRecordExistByPreloadRequest_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckAppRecordExistByPreloadRequest_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    appMgrServiceInner->appRunningManager_ = nullptr;

    PreloadRequest request;
    auto appInfo = std::make_shared<ApplicationInfo>();
    request.appInfo = appInfo;
    std::string processName;
    std::string specifiedProcessFlag;
    auto ret = appMgrServiceInner->CheckAppRecordExistByPreloadRequest(request, processName, specifiedProcessFlag);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "CheckAppRecordExistByPreloadRequest_001 end");
}

/**
 * @tc.name: CheckAppRecordExistByPreloadRequest_002
 * @tc.desc: Test CheckAppRecordExistByPreloadRequest
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, CheckAppRecordExistByPreloadRequest_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckAppRecordExistByPreloadRequest_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();

    PreloadRequest request;
    auto appInfo = std::make_shared<ApplicationInfo>();
    request.appInfo = appInfo;
    std::string processName;
    std::string specifiedProcessFlag;
    auto ret = appMgrServiceInner->CheckAppRecordExistByPreloadRequest(request, processName, specifiedProcessFlag);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "CheckAppRecordExistByPreloadRequest_002 end");
}

/**
 * @tc.name: PreloadApplicationByPhase_001
 * @tc.desc: Test PreloadApplicationByPhase
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, PreloadApplicationByPhase_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplicationByPhase_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();

    AAFwk::AppUtils::GetInstance().isPreloadApplicationEnabled_.isLoaded = true;
    AAFwk::AppUtils::GetInstance().isPreloadApplicationEnabled_.value = false;

    auto ret = appMgrServiceInner->PreloadApplicationByPhase("com.test.demo", 100, 0, PreloadPhase::PROCESS_CREATED);
    EXPECT_EQ(ret, AAFwk::ERR_CAPABILITY_NOT_SUPPORT);
    AAFwk::AppUtils::GetInstance().isPreloadApplicationEnabled_.isLoaded = false;
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplicationByPhase_001 end");
}

/**
 * @tc.name: PreloadApplicationByPhase_002
 * @tc.desc: Test PreloadApplicationByPhase
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, PreloadApplicationByPhase_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplicationByPhase_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();

    AAFwk::AppUtils::GetInstance().isPreloadApplicationEnabled_.isLoaded = true;
    AAFwk::AppUtils::GetInstance().isPreloadApplicationEnabled_.value = true;

    auto ret = appMgrServiceInner->PreloadApplicationByPhase("com.test.demo", 100, 0, PreloadPhase::PROCESS_CREATED);
    EXPECT_NE(ret, AAFwk::ERR_CAPABILITY_NOT_SUPPORT);
    AAFwk::AppUtils::GetInstance().isPreloadApplicationEnabled_.isLoaded = false;
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplicationByPhase_002 end");
}

/**
 * @tc.name: QueryRunningSharedBundles_001
 * @tc.desc: Test QueryRunningSharedBundles
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, QueryRunningSharedBundles_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "QueryRunningSharedBundles_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();

    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = nullptr;
    pid_t pid = 1;
    std::map<std::string, uint32_t> sharedBundles;
    auto ret = appMgrServiceInner->QueryRunningSharedBundles(pid, sharedBundles);
    EXPECT_EQ(ret, AAFwk::ERR_NO_APP_RECORD);
    TAG_LOGI(AAFwkTag::TEST, "QueryRunningSharedBundles_001 end");
}

/**
 * @tc.name: QueryRunningSharedBundles_002
 * @tc.desc: Test QueryRunningSharedBundles
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTenthTest, QueryRunningSharedBundles_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "QueryRunningSharedBundles_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();

    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    pid_t pid = 1;
    std::map<std::string, uint32_t> sharedBundles;
    auto ret = appMgrServiceInner->QueryRunningSharedBundles(pid, sharedBundles);
    EXPECT_EQ(ret, ERR_OK);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = nullptr;
    TAG_LOGI(AAFwkTag::TEST, "QueryRunningSharedBundles_002 end");
}
}  // namespace AppExecFwk
}  // namespace OHOS