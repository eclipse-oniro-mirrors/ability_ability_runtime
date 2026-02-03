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
#include "ability_connect_callback_stub.h"
#include "ability_manager_errors.h"
#include "app_scheduler_const.h"
#include "application_info.h"
#include "mock_app_scheduler.h"
#include "mock_my_status.h"
#include "mock_start_specified_ability_response.h"
#include "overlay_manager_proxy.h"
#include "parameters.h"
#include "start_specified_ability_response_proxy.h"
#include "user_record_manager.h"
#include "want.h"
using namespace testing;
using namespace testing::ext;
using namespace OHOS::AAFwk;
using OHOS::AppExecFwk::ExtensionAbilityType;
constexpr int32_t FOUNDATION_UID = 5523;
constexpr int32_t QUICKFIX_UID = 5524;
constexpr int32_t SHADER_CACHE_GROUPID = 3099;
constexpr int32_t RESOURCE_MANAGER_UID = 1096;
constexpr int32_t DEFAULT_USER_ID = 0;
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
class AppMgrServiceInnerNinthTest : public testing::Test {
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
        pid_t diedPid) override
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
void AppMgrServiceInnerNinthTest::SetUpTestCase() {}

void AppMgrServiceInnerNinthTest::TearDownTestCase() {}

void AppMgrServiceInnerNinthTest::SetUp() {}

void AppMgrServiceInnerNinthTest::TearDown() {}

/**
 * @tc.name: PreloadApplication_001
 * @tc.desc: test PreloadApplication with null appPreloader
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, PreloadApplication_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);
    AAFwk::MyStatus::GetInstance().judgeCallerIsAllowed_ = true;
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = true;
    AAFwk::MyStatus::GetInstance().isSACall_ = true;
    appMgrServiceInner->appPreloader_ = nullptr;

    std::string bundleName = "com.test.preload";
    int32_t userId = 100;
    PreloadMode preloadMode = PreloadMode::PRE_MAKE;
    int32_t appIndex = 0;
    
    int32_t ret = appMgrServiceInner->PreloadApplication(bundleName, userId, preloadMode, appIndex);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_001 end");
}

/**
 * @tc.name: PreloadApplication_002
 * @tc.desc: test PreloadApplication with permission denied
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, PreloadApplication_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);
    AAFwk::MyStatus::GetInstance().judgeCallerIsAllowed_ = true;
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = false;
    AAFwk::MyStatus::GetInstance().isSACall_ = true;

    std::string bundleName = "com.test.preload";
    int32_t userId = 100;
    PreloadMode preloadMode = PreloadMode::PRE_MAKE;
    int32_t appIndex = 0;
    
    int32_t ret = appMgrServiceInner->PreloadApplication(bundleName, userId, preloadMode, appIndex);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_002 end");
}

/**
 * @tc.name: PreloadApplication_003
 * @tc.desc: test PreloadApplication with current user ID
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, PreloadApplication_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);
    AAFwk::MyStatus::GetInstance().judgeCallerIsAllowed_ = true;
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = true;
    AAFwk::MyStatus::GetInstance().isSACall_ = true;
    appMgrServiceInner->appPreloader_ = nullptr;

    std::string bundleName = "com.test.preload";
    int32_t userId = -2; // CURRENT_USER_ID
    PreloadMode preloadMode = PreloadMode::PRE_MAKE;
    int32_t appIndex = 0;
    
    int32_t ret = appMgrServiceInner->PreloadApplication(bundleName, userId, preloadMode, appIndex);
    EXPECT_EQ(ret, ERR_INVALID_VALUE); // Due to null appPreloader
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_003 end");
}

/**
 * @tc.name: PreloadApplication_004
 * @tc.desc: test PreloadApplication with logout user
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, PreloadApplication_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_004 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);
    AAFwk::MyStatus::GetInstance().judgeCallerIsAllowed_ = true;
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = true;
    AAFwk::MyStatus::GetInstance().isSACall_ = true;
    AAFwk::MyStatus::GetInstance().isLogoutUser_ = true;
    appMgrServiceInner->appPreloader_ = std::make_shared<AppPreloader>(nullptr);

    std::string bundleName = "com.test.preload";
    int32_t userId = 100;
    PreloadMode preloadMode = PreloadMode::PRE_MAKE;
    int32_t appIndex = 0;
    
    int32_t ret = appMgrServiceInner->PreloadApplication(bundleName, userId, preloadMode, appIndex);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_004 end");
}

/**
 * @tc.name: PreloadApplication_005
 * @tc.desc: test PreloadApplication with preload not allowed
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, PreloadApplication_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_005 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);
    AAFwk::MyStatus::GetInstance().judgeCallerIsAllowed_ = true;
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = true;
    AAFwk::MyStatus::GetInstance().isSACall_ = true;
    AAFwk::MyStatus::GetInstance().isLogoutUser_ = false;
    AAFwk::MyStatus::GetInstance().allowPreload_ = false;
    appMgrServiceInner->appPreloader_ = std::make_shared<AppPreloader>(nullptr);

    std::string bundleName = "com.test.preload";
    int32_t userId = 100;
    PreloadMode preloadMode = PreloadMode::PRESS_DOWN;
    int32_t appIndex = 0;
    
    int32_t ret = appMgrServiceInner->PreloadApplication(bundleName, userId, preloadMode, appIndex);
    EXPECT_EQ(ret, AAFwk::ERR_NOT_ALLOW_PRELOAD_BY_RSS);
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_005 end");
}

/**
 * @tc.name: PreloadApplication_006
 * @tc.desc: test PreloadApplication
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, PreloadApplication_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_006 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    AAFwk::MyStatus::GetInstance().judgeCallerIsAllowed_ = false;
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = true;
    AAFwk::MyStatus::GetInstance().isSACall_ = false;
    AAFwk::MyStatus::GetInstance().isLogoutUser_ = false;
    AAFwk::MyStatus::GetInstance().allowPreload_ = true;
    AAFwk::MyStatus::GetInstance().generatePreloadRequestRet_ = ERR_INVALID_VALUE;
    appMgrServiceInner->appPreloader_ = std::make_shared<AppPreloader>(nullptr);

    std::string bundleName = "com.test.preload";
    int32_t userId = 100;
    PreloadMode preloadMode = PreloadMode::PRE_MAKE;
    int32_t appIndex = 0;
    
    int32_t ret = appMgrServiceInner->PreloadApplication(bundleName, userId, preloadMode, appIndex);
    EXPECT_EQ(ret, AAFwk::ERR_NOT_SYSTEM_APP);
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_006 end");
}
/**
 * @tc.name: PreloadApplication_007
 * @tc.desc: test PreloadApplication
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, PreloadApplication_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_007 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    AAFwk::MyStatus::GetInstance().judgeCallerIsAllowed_ = true;
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = true;
    AAFwk::MyStatus::GetInstance().isLogoutUser_ = false;
    AAFwk::MyStatus::GetInstance().allowPreload_ = true;
    AAFwk::MyStatus::GetInstance().generatePreloadRequestRet_ = ERR_OK;
    appMgrServiceInner->appPreloader_ = std::make_shared<AppPreloader>(nullptr);
    appMgrServiceInner->taskHandler_ = nullptr;

    std::string bundleName = "com.test.preload";
    int32_t userId = 100;
    PreloadMode preloadMode = PreloadMode::PRE_MAKE;
    int32_t appIndex = 0;
    
    int32_t ret = appMgrServiceInner->PreloadApplication(bundleName, userId, preloadMode, appIndex);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_007 end");
}

/**
 * @tc.name: PreloadApplication_008
 * @tc.desc: test PreloadApplication
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, PreloadApplication_008, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_008 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    AAFwk::MyStatus::GetInstance().judgeCallerIsAllowed_ = true;
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = true;
    AAFwk::MyStatus::GetInstance().isLogoutUser_ = false;
    AAFwk::MyStatus::GetInstance().allowPreload_ = true;
    AAFwk::MyStatus::GetInstance().generatePreloadRequestRet_ = ERR_INVALID_VALUE;
    appMgrServiceInner->appPreloader_ = std::make_shared<AppPreloader>(nullptr);

    std::string bundleName = "com.test.preload";
    int32_t userId = 100;
    PreloadMode preloadMode = PreloadMode::PRE_MAKE;
    int32_t appIndex = 0;
    
    int32_t ret = appMgrServiceInner->PreloadApplication(bundleName, userId, preloadMode, appIndex);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_008 end");
}

/**
 * @tc.name: PreloadApplication_009
 * @tc.desc: test PreloadApplication
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, PreloadApplication_009, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_009 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    AAFwk::MyStatus::GetInstance().judgeCallerIsAllowed_ = true;
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = true;
    AAFwk::MyStatus::GetInstance().isLogoutUser_ = false;
    AAFwk::MyStatus::GetInstance().allowPreload_ = true;
    AAFwk::MyStatus::GetInstance().generatePreloadRequestRet_ = ERR_OK;
    appMgrServiceInner->appPreloader_ = std::make_shared<AppPreloader>(nullptr);
    appMgrServiceInner->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler("test_queue2");

    std::string bundleName = "com.test.preload";
    int32_t userId = 100;
    PreloadMode preloadMode = PreloadMode::PRESS_DOWN;
    int32_t appIndex = 0;
    
    int32_t ret = appMgrServiceInner->PreloadApplication(bundleName, userId, preloadMode, appIndex);
    EXPECT_EQ(ret, ERR_OK);
    appMgrServiceInner->taskHandler_.reset();
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_009 end");
}

/**
 * @tc.name: PreloadApplication_010
 * @tc.desc: test PreloadApplication
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, PreloadApplication_010, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_010 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    AAFwk::MyStatus::GetInstance().judgeCallerIsAllowed_ = true;
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = true;
    AAFwk::MyStatus::GetInstance().isLogoutUser_ = false;
    AAFwk::MyStatus::GetInstance().allowPreload_ = true;
    AAFwk::MyStatus::GetInstance().generatePreloadRequestRet_ = ERR_OK;
    appMgrServiceInner->appPreloader_ = std::make_shared<AppPreloader>(nullptr);
    appMgrServiceInner->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler("test_queue3");

    std::string bundleName = "com.test.preload";
    int32_t userId = 100;
    PreloadMode preloadMode = PreloadMode::PRELOAD_MODULE;
    int32_t appIndex = 0;
    
    int32_t ret = appMgrServiceInner->PreloadApplication(bundleName, userId, preloadMode, appIndex);
    EXPECT_EQ(ret, ERR_OK);
    appMgrServiceInner->taskHandler_.reset();
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_010 end");
}

/**
 * @tc.name: PreloadApplication_011
 * @tc.desc: test PreloadApplication
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, PreloadApplication_011, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_011 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    AAFwk::MyStatus::GetInstance().judgeCallerIsAllowed_ = true;
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = true;
    AAFwk::MyStatus::GetInstance().isLogoutUser_ = false;
    AAFwk::MyStatus::GetInstance().allowPreload_ = true;
    AAFwk::MyStatus::GetInstance().generatePreloadRequestRet_ = ERR_INVALID_VALUE;
    appMgrServiceInner->appPreloader_ = std::make_shared<AppPreloader>(nullptr);

    std::string bundleName = "";
    int32_t userId = 100;
    PreloadMode preloadMode = PreloadMode::PRE_MAKE;
    int32_t appIndex = 0;
    
    int32_t ret = appMgrServiceInner->PreloadApplication(bundleName, userId, preloadMode, appIndex);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_011 end");
}

/**
 * @tc.name: PreloadApplication_012
 * @tc.desc: test PreloadApplication
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, PreloadApplication_012, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_012 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    AAFwk::MyStatus::GetInstance().judgeCallerIsAllowed_ = true;
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = true;
    AAFwk::MyStatus::GetInstance().isLogoutUser_ = false;
    AAFwk::MyStatus::GetInstance().allowPreload_ = true;
    AAFwk::MyStatus::GetInstance().generatePreloadRequestRet_ = ERR_OK;
    appMgrServiceInner->appPreloader_ = std::make_shared<AppPreloader>(nullptr);
    appMgrServiceInner->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler("test_queue4");

    std::string bundleName = "com.test.preload";
    int32_t userId = 100;
    PreloadMode preloadMode = PreloadMode::PRE_MAKE;
    int32_t appIndex = 1;
    
    int32_t ret = appMgrServiceInner->PreloadApplication(bundleName, userId, preloadMode, appIndex);
    EXPECT_EQ(ret, ERR_OK);
    appMgrServiceInner->taskHandler_.reset();
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_012 end");
}

/**
 * @tc.name: HandlePreloadApplication_001
 * @tc.desc: Test HandlePreloadApplication with null abilityInfo
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, HandlePreloadApplication_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandlePreloadApplication_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    // Reset tracking flags
    AAFwk::MyStatus::GetInstance().resetHandlePreloadApplicationFlags();
    AAFwk::MyStatus::GetInstance().checkAppRunningCall_ = 0;
    
    // Setup request with null abilityInfo
    PreloadRequest request;
    request.abilityInfo = nullptr;
    request.appInfo = std::make_shared<ApplicationInfo>();
    request.bundleInfo.name = "com.test.app";
    request.hapModuleInfo.name = "entry";
    request.hapModuleInfo.bundleName = "com.test.app";
    request.want = std::make_shared<AAFwk::Want>();
    request.preloadMode = PreloadMode::PRE_MAKE;
    request.appIndex = 0;
    appMgrServiceInner->HandlePreloadApplication(request);
    
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().checkAppRunningCall_, 0);
    
    TAG_LOGI(AAFwkTag::TEST, "HandlePreloadApplication_001 end");
}

/**
 * @tc.name: HandlePreloadApplication_002
 * @tc.desc: Test HandlePreloadApplication with null appRunningManager
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, HandlePreloadApplication_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandlePreloadApplication_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    // Reset tracking flags
    AAFwk::MyStatus::GetInstance().resetHandlePreloadApplicationFlags();
    AAFwk::MyStatus::GetInstance().checkAppRunningCall_ = 0;
    
    // Set appRunningManager to null
    appMgrServiceInner->appRunningManager_ = nullptr;
    
    // Setup valid request
    PreloadRequest request;
    request.abilityInfo = std::make_shared<AbilityInfo>();
    request.appInfo = std::make_shared<ApplicationInfo>();
    request.bundleInfo.name = "com.test.app";
    request.hapModuleInfo.name = "entry";
    request.hapModuleInfo.bundleName = "com.test.app";
    request.want = std::make_shared<AAFwk::Want>();
    request.preloadMode = PreloadMode::PRE_MAKE;
    request.appIndex = 0;
    
    // Call HandlePreloadApplication
    appMgrServiceInner->HandlePreloadApplication(request);
    
    // Verify that CheckAppRunningRecordIsExist was never called (early return)
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().checkAppRunningCall_, 0);
    
    TAG_LOGI(AAFwkTag::TEST, "HandlePreloadApplication_002 end");
}

/**
 * @tc.name: HandlePreloadApplication_003
 * @tc.desc: Test HandlePreloadApplication with existing app record
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, HandlePreloadApplication_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandlePreloadApplication_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    // Reset tracking flags
    AAFwk::MyStatus::GetInstance().resetHandlePreloadApplicationFlags();
    AAFwk::MyStatus::GetInstance().checkAppRunningCall_ = 0;
    
    // Setup appRunningManager
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    
    // Configure mock to return existing app record
    AAFwk::MyStatus::GetInstance().checkAppRunning_ = std::make_shared<AppRunningRecord>(
        std::make_shared<ApplicationInfo>(), 0, "test");
    
    // Setup valid request
    PreloadRequest request;
    request.abilityInfo = std::make_shared<AbilityInfo>();
    request.appInfo = std::make_shared<ApplicationInfo>();
    request.bundleInfo.name = "com.test.app";
    request.hapModuleInfo.name = "entry";
    request.hapModuleInfo.bundleName = "com.test.app";
    request.want = std::make_shared<AAFwk::Want>();
    request.preloadMode = PreloadMode::PRE_MAKE;
    request.appIndex = 0;
    
    // Call HandlePreloadApplication
    appMgrServiceInner->HandlePreloadApplication(request);
    
    // Verify that CheckAppRunningRecordIsExist was called and existing app record was detected
    EXPECT_GT(AAFwk::MyStatus::GetInstance().checkAppRunningCall_, 0);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().handlePreloadApplication_existingAppRecord_called_);
    
    // Cleanup
    AAFwk::MyStatus::GetInstance().checkAppRunning_ = nullptr;
    
    TAG_LOGI(AAFwkTag::TEST, "HandlePreloadApplication_003 end");
}

/**
 * @tc.name: HandlePreloadApplication_004
 * @tc.desc: Test HandlePreloadApplication with app multi-user not existing
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, HandlePreloadApplication_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandlePreloadApplication_004 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    // Reset tracking flags
    AAFwk::MyStatus::GetInstance().resetHandlePreloadApplicationFlags();
    AAFwk::MyStatus::GetInstance().checkAppRunningCall_ = 0;
    
    // Setup appRunningManager
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    
    // Configure mock to return no existing app record and no multi-user app
    AAFwk::MyStatus::GetInstance().checkAppRunning_ = nullptr;
    AAFwk::MyStatus::GetInstance().checkAppRunningByUid_ = false;
    
    // Setup valid request
    PreloadRequest request;
    request.abilityInfo = std::make_shared<AbilityInfo>();
    request.appInfo = std::make_shared<ApplicationInfo>();
    request.bundleInfo.name = "com.test.app";
    request.bundleInfo.uid = 1000;
    request.hapModuleInfo.name = "entry";
    request.hapModuleInfo.bundleName = "com.test.app";
    request.want = std::make_shared<AAFwk::Want>();
    request.preloadMode = PreloadMode::PRE_MAKE;
    request.appIndex = 0;
    
    // Call HandlePreloadApplication
    appMgrServiceInner->HandlePreloadApplication(request);
    
    // Verify that CheckAppRunningRecordIsExist was called and app multi-user not existing was detected
    EXPECT_GT(AAFwk::MyStatus::GetInstance().checkAppRunningCall_, 0);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().handlePreloadApplication_appMultiUserNotExist_called_);
    
    TAG_LOGI(AAFwkTag::TEST, "HandlePreloadApplication_004 end");
}

/**
 * @tc.name: HandlePreloadApplication_005
 * @tc.desc: Test HandlePreloadApplication with successful app record creation for PRE_MAKE mode
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, HandlePreloadApplication_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandlePreloadApplication_005 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    // Reset tracking flags
    AAFwk::MyStatus::GetInstance().resetHandlePreloadApplicationFlags();
    AAFwk::MyStatus::GetInstance().checkAppRunningCall_ = 0;
    
    // Setup appRunningManager
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    
    // Configure mock for successful flow
    AAFwk::MyStatus::GetInstance().checkAppRunning_ = nullptr;
    AAFwk::MyStatus::GetInstance().checkAppRunningByUid_ = true;
    
    // Setup valid request with PRE_MAKE mode
    PreloadRequest request;
    request.abilityInfo = std::make_shared<AbilityInfo>();
    request.abilityInfo->name = "TestAbility";
    request.appInfo = std::make_shared<ApplicationInfo>();
    request.appInfo->name = "com.test.app";
    request.bundleInfo.name = "com.test.app";
    request.bundleInfo.uid = 1000;
    request.hapModuleInfo.name = "entry";
    request.hapModuleInfo.bundleName = "com.test.app";
    request.want = std::make_shared<AAFwk::Want>();
    request.preloadMode = PreloadMode::PRE_MAKE;
    request.appIndex = 0;
    
    // Call HandlePreloadApplication
    appMgrServiceInner->HandlePreloadApplication(request);
    
    // Verify that CheckAppRunningRecordIsExist was called (normal flow)
    EXPECT_GT(AAFwk::MyStatus::GetInstance().checkAppRunningCall_, 0);
    
    TAG_LOGI(AAFwkTag::TEST, "HandlePreloadApplication_005 end");
}

/**
 * @tc.name: HandlePreloadApplication_006
 * @tc.desc: Test HandlePreloadApplication with PRELOAD_MODULE mode
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, HandlePreloadApplication_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandlePreloadApplication_006 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    // Reset tracking flags
    AAFwk::MyStatus::GetInstance().resetHandlePreloadApplicationFlags();
    AAFwk::MyStatus::GetInstance().checkAppRunningCall_ = 0;
    
    // Setup appRunningManager
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    
    // Configure mock for successful flow
    AAFwk::MyStatus::GetInstance().checkAppRunning_ = nullptr;
    AAFwk::MyStatus::GetInstance().checkAppRunningByUid_ = true;
    
    // Setup valid request with PRELOAD_MODULE mode
    PreloadRequest request;
    request.abilityInfo = std::make_shared<AbilityInfo>();
    request.abilityInfo->name = "TestAbility";
    request.appInfo = std::make_shared<ApplicationInfo>();
    request.appInfo->name = "com.test.app";
    request.bundleInfo.name = "com.test.app";
    request.bundleInfo.uid = 1000;
    request.hapModuleInfo.name = "entry";
    request.hapModuleInfo.bundleName = "com.test.app";
    request.want = std::make_shared<AAFwk::Want>();
    request.preloadMode = PreloadMode::PRELOAD_MODULE;
    request.appIndex = 0;
    
    appMgrServiceInner->HandlePreloadApplication(request);
    
    EXPECT_GT(AAFwk::MyStatus::GetInstance().checkAppRunningCall_, 0);
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().handlePreloadApplication_existingAppRecord_called_);
    
    TAG_LOGI(AAFwkTag::TEST, "HandlePreloadApplication_006 end");
}

/**
 * @tc.name: SetSceneBoardAttachFlag_001
 * @tc.desc: Test SetSceneBoardAttachFlag with different flag values
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, SetSceneBoardAttachFlag_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetSceneBoardAttachFlag_001 start");
    
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    // Test setting flag to true
    appMgrServiceInner->SetSceneBoardAttachFlag(true);
    EXPECT_TRUE(appMgrServiceInner->sceneBoardAttachFlag_);
    
    // Test setting flag to false
    appMgrServiceInner->SetSceneBoardAttachFlag(false);
    EXPECT_FALSE(appMgrServiceInner->sceneBoardAttachFlag_);
    
    TAG_LOGI(AAFwkTag::TEST, "SetSceneBoardAttachFlag_001 end");
}

/**
 * @tc.name: RestartResidentProcessDependedOnWeb_001
 * @tc.desc: Test RestartResidentProcessDependedOnWeb when bundleNames is empty
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, RestartResidentProcessDependedOnWeb_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RestartResidentProcessDependedOnWeb_001 start");
    
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    // Reset mock flags before test
    AAFwk::MyStatus::GetInstance().resetRestartResidentProcessDependedOnWebFlags();
    
    // Setup mock to return empty bundleNames (default behavior)
    AAFwk::MyStatus::GetInstance().mockBundleNames_.clear();
    
    // Call RestartResidentProcessDependedOnWeb - should return early due to empty bundleNames
    appMgrServiceInner->RestartResidentProcessDependedOnWeb();
    
    // Verify HandleExitResidentBundleDependedOnWeb was called
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().handleExitResidentBundleDependedOnWeb_called_);
    
    // Verify early return path was taken (emptyBundleNames flag should be set)
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().restartResidentProcessDependedOnWeb_emptyBundleNames_called_);
    
    TAG_LOGI(AAFwkTag::TEST, "RestartResidentProcessDependedOnWeb_001 end");
}

/**
 * @tc.name: RestartResidentProcessDependedOnWeb_002
 * @tc.desc: Test RestartResidentProcessDependedOnWeb when taskHandler_ is null
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, RestartResidentProcessDependedOnWeb_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RestartResidentProcessDependedOnWeb_002 start");
    
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    // Reset mock flags before test
    AAFwk::MyStatus::GetInstance().resetRestartResidentProcessDependedOnWebFlags();
    
    // Setup mock to return non-empty bundleNames to pass the first check
    AAFwk::MyStatus::GetInstance().mockBundleNames_.clear();
    AAFwk::MyStatus::GetInstance().mockBundleNames_.push_back(
        AppExecFwk::ExitResidentProcessInfo("com.test.bundle", 1000));
    
    // Ensure taskHandler_ is null (it should be null by default)
    EXPECT_EQ(appMgrServiceInner->taskHandler_, nullptr);
    
    // Call RestartResidentProcessDependedOnWeb - should return early due to null taskHandler_
    appMgrServiceInner->RestartResidentProcessDependedOnWeb();
    // Verify HandleExitResidentBundleDependedOnWeb was called
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().handleExitResidentBundleDependedOnWeb_called_);
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().restartResidentProcessDependedOnWeb_taskSubmitted_called_);
    TAG_LOGI(AAFwkTag::TEST, "RestartResidentProcessDependedOnWeb_002 end");
}

/**
 * @tc.name: RestartResidentProcessDependedOnWeb_003
 * @tc.desc: Test RestartResidentProcessDependedOnWeb normal flow with valid taskHandler
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, RestartResidentProcessDependedOnWeb_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RestartResidentProcessDependedOnWeb_003 start");
    
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    // Reset mock flags before test
    AAFwk::MyStatus::GetInstance().resetRestartResidentProcessDependedOnWebFlags();
    
    // Setup mock to return non-empty bundleNames
    AAFwk::MyStatus::GetInstance().mockBundleNames_.clear();
    AAFwk::MyStatus::GetInstance().mockBundleNames_.push_back(
        AppExecFwk::ExitResidentProcessInfo("com.test.bundle1", 1000));
    AAFwk::MyStatus::GetInstance().mockBundleNames_.push_back(
        AppExecFwk::ExitResidentProcessInfo("com.test.bundle2", 1001));
    
    appMgrServiceInner->taskHandler_ =
        std::make_shared<AAFwk::MockTaskHandlerWrapForRestart>("RestartResidentProcessTest");

    appMgrServiceInner->RestartResidentProcessDependedOnWeb();
    
    // Verify HandleExitResidentBundleDependedOnWeb was called
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().handleExitResidentBundleDependedOnWeb_called_);
    
    // Verify task was submitted to taskHandler (normal flow)
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().restartResidentProcessDependedOnWeb_taskSubmitted_called_);
    
    // Verify that early return paths were not taken
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().restartResidentProcessDependedOnWeb_emptyBundleNames_called_);
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().restartResidentProcessDependedOnWeb_nullTaskHandler_called_);
    
    TAG_LOGI(AAFwkTag::TEST, "RestartResidentProcessDependedOnWeb_003 end");
}

/**
 * @tc.name: AttachApplication_001
 * @tc.desc: Test AttachApplication calls GetApplicationInfo to verify execution
 * @tc.type: FUNC
 * @tc.require: Test AttachApplication method
 */
HWTEST_F(AppMgrServiceInnerNinthTest, AttachApplication_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AttachApplication_001 start");
    AAFwk::MyStatus::GetInstance().resetGetApplicationInfoFlag();
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().getApplicationInfoCalled_);
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    const int32_t testPid = 1234;
    const std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, 1, processName);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = appRecord;
    sptr<AppExecFwk::MockAppScheduler> mockAppScheduler = sptr<AppExecFwk::MockAppScheduler>::MakeSptr();
    appMgrServiceInner->AttachApplication(testPid, mockAppScheduler);
    // Verify that GetApplicationInfo was called (line 1243 was reached)
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().getApplicationInfoCalled_);
    TAG_LOGI(AAFwkTag::TEST, "AttachApplication_001 end");
}

/**
 * @tc.name: AttachApplication_002
 * @tc.desc: Test AttachApplication calls GetApplicationInfo to verify execution
 * @tc.type: FUNC
 * @tc.require: Test AttachApplication method
 */
HWTEST_F(AppMgrServiceInnerNinthTest, AttachApplication_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AttachApplication_002 start");
    AAFwk::MyStatus::GetInstance().resetAppDeathRecipientSetTaskHandlerFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    const int32_t testPid = 1234;
    const std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, 1, processName);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = appRecord;
    sptr<AppExecFwk::MockAppScheduler> mockAppScheduler = sptr<AppExecFwk::MockAppScheduler>::MakeSptr();
    appMgrServiceInner->AttachApplication(testPid, mockAppScheduler);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().appDeathRecipientSetTaskHandlerCalled_);
    AAFwk::MyStatus::GetInstance().resetAppDeathRecipientSetTaskHandlerFlag();
    TAG_LOGI(AAFwkTag::TEST, "AttachApplication_002 end");
}

/**
 * @tc.name: AttachApplication_003
 * @tc.desc: Test AttachApplication calls GetApplicationInfo to verify execution
 * @tc.type: FUNC
 * @tc.require: Test AttachApplication method
 */
HWTEST_F(AppMgrServiceInnerNinthTest, AttachApplication_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AttachApplication_003 start");
    AAFwk::MyStatus::GetInstance().resetGetApplicationInfoFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    std::shared_ptr<ApplicationInfo> applicationInfo = nullptr;
    const int32_t testPid = 1234;
    const std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, 1, processName);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = appRecord;
    sptr<AppExecFwk::MockAppScheduler> mockAppScheduler = nullptr;
    appMgrServiceInner->AttachApplication(testPid, mockAppScheduler);
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().appDeathRecipientSetTaskHandlerCalled_);
    AAFwk::MyStatus::GetInstance().resetAppDeathRecipientSetTaskHandlerFlag();
    TAG_LOGI(AAFwkTag::TEST, "AttachApplication_003 end");
}

/**
 * @tc.name: AttachApplication_004
 * @tc.desc: Test AttachApplication calls GetApplicationInfo to verify execution
 * @tc.type: FUNC
 * @tc.require: Test AttachApplication method
 */
HWTEST_F(AppMgrServiceInnerNinthTest, AttachApplication_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AttachApplication_004 start");
    AAFwk::MyStatus::GetInstance().resetGetApplicationInfoFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    std::shared_ptr<ApplicationInfo> applicationInfo = nullptr;
    const int32_t testPid = 1234;
    const std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, 1, processName);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = appRecord;
    sptr<AppExecFwk::MockAppScheduler> mockAppScheduler = sptr<AppExecFwk::MockAppScheduler>::MakeSptr();
    appMgrServiceInner->AttachApplication(testPid, mockAppScheduler);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().appDeathRecipientSetTaskHandlerCalled_);
    AAFwk::MyStatus::GetInstance().resetAppDeathRecipientSetTaskHandlerFlag();
    TAG_LOGI(AAFwkTag::TEST, "AttachApplication_004 end");
}

/**
 * @tc.name: AttachApplication_005
 * @tc.desc: Test AttachApplication calls GetApplicationInfo to verify execution
 * @tc.type: FUNC
 * @tc.require: Test AttachApplication method
 */
HWTEST_F(AppMgrServiceInnerNinthTest, AttachApplication_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AttachApplication_005 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    std::shared_ptr<ApplicationInfo> applicationInfo = nullptr;
    const int32_t testPid = 1234;
    const std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, 1, processName);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = appRecord;
    sptr<AppExecFwk::MockAppScheduler> mockAppScheduler = sptr<AppExecFwk::MockAppScheduler>::MakeSptr();
    AAFwk::MyStatus::GetInstance().addDeathRecipientReturn_=false;
    appMgrServiceInner->AttachApplication(testPid, mockAppScheduler);
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().appRunningRecordSetAppDeathRecipientCalled_);
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    TAG_LOGI(AAFwkTag::TEST, "AttachApplication_005 end");
}

/**
 * @tc.name: AttachApplication_006
 * @tc.desc: Test AttachApplication calls GetApplicationInfo to verify execution
 * @tc.type: FUNC
 * @tc.require: Test AttachApplication method
 */
HWTEST_F(AppMgrServiceInnerNinthTest, AttachApplication_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AttachApplication_006 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().appRunningRecordSetAppDeathRecipientCalled_);
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    std::shared_ptr<ApplicationInfo> applicationInfo = nullptr;
    const int32_t testPid = 1234;
    const std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, 1, processName);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = appRecord;
    sptr<AppExecFwk::MockAppScheduler> mockAppScheduler = sptr<AppExecFwk::MockAppScheduler>::MakeSptr();
    AAFwk::MyStatus::GetInstance().addDeathRecipientReturn_=true;
    appMgrServiceInner->AttachApplication(testPid, mockAppScheduler);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().appRunningRecordSetAppDeathRecipientCalled_);
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    TAG_LOGI(AAFwkTag::TEST, "AttachApplication_006 end");
}

/**
 * @tc.name: AttachApplication_007
 * @tc.desc: Test AttachApplication calls GetApplicationInfo to verify execution
 * @tc.type: FUNC
 * @tc.require: Test AttachApplication method
 */
HWTEST_F(AppMgrServiceInnerNinthTest, AttachApplication_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AttachApplication_007 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    std::shared_ptr<ApplicationInfo> applicationInfo = nullptr;
    const int32_t testPid = 1234;
    const std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, 1, processName);
    
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = appRecord;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_->SetState(ApplicationState::APP_STATE_CREATE);
    
    sptr<AppExecFwk::MockAppScheduler> mockAppScheduler = sptr<AppExecFwk::MockAppScheduler>::MakeSptr();
    AAFwk::MyStatus::GetInstance().addDeathRecipientReturn_=true;
    appMgrServiceInner->AttachApplication(testPid, mockAppScheduler);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().setNWebPreloadCalled_);
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    TAG_LOGI(AAFwkTag::TEST, "AttachApplication_007 end");
}

/**
 * @tc.name: AttachApplication_008
 * @tc.desc: Test AttachApplication calls GetApplicationInfo to verify execution
 * @tc.type: FUNC
 * @tc.require: Test AttachApplication method
 */
HWTEST_F(AppMgrServiceInnerNinthTest, AttachApplication_008, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AttachApplication_008 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    std::shared_ptr<ApplicationInfo> applicationInfo = nullptr;
    const int32_t testPid = 1234;
    const std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, 1, processName);
    
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = appRecord;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_->SetState(ApplicationState::APP_STATE_READY);
    
    sptr<AppExecFwk::MockAppScheduler> mockAppScheduler = sptr<AppExecFwk::MockAppScheduler>::MakeSptr();
    AAFwk::MyStatus::GetInstance().addDeathRecipientReturn_=true;
    appMgrServiceInner->AttachApplication(testPid, mockAppScheduler);
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().setNWebPreloadCalled_);
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    TAG_LOGI(AAFwkTag::TEST, "AttachApplication_008 end");
}

/**
 * @tc.name: AttachApplication_009
 * @tc.desc: Test AttachApplication calls GetApplicationInfo to verify execution
 * @tc.type: FUNC
 * @tc.require: Test AttachApplication method
 */
HWTEST_F(AppMgrServiceInnerNinthTest, AttachApplication_009, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AttachApplication_009 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->sceneBoardAttachFlag_ = false;
    
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    const int32_t testPid = 1234;
    const std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, 1, processName);
    
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = appRecord;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_->SetState(ApplicationState::APP_STATE_CREATE);
    
    sptr<AppExecFwk::MockAppScheduler> mockAppScheduler = sptr<AppExecFwk::MockAppScheduler>::MakeSptr();
    AAFwk::MyStatus::GetInstance().addDeathRecipientReturn_=true;
    appMgrServiceInner->AttachApplication(testPid, mockAppScheduler);
    EXPECT_FALSE(appMgrServiceInner->sceneBoardAttachFlag_);
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    TAG_LOGI(AAFwkTag::TEST, "AttachApplication_009 end");
}

/**
 * @tc.name: AttachApplication_010
 * @tc.desc: Test AttachApplication calls GetApplicationInfo to verify execution
 * @tc.type: FUNC
 * @tc.require: Test AttachApplication method
 */
HWTEST_F(AppMgrServiceInnerNinthTest, AttachApplication_010, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AttachApplication_010 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->sceneBoardAttachFlag_ = false;
    
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = Constants::SCENE_BOARD_BUNDLE_NAME;
    const int32_t testPid = 1234;
    const std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, 1, processName);
    
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = appRecord;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_->SetState(ApplicationState::APP_STATE_CREATE);
    
    sptr<AppExecFwk::MockAppScheduler> mockAppScheduler = sptr<AppExecFwk::MockAppScheduler>::MakeSptr();
    AAFwk::MyStatus::GetInstance().addDeathRecipientReturn_=true;
    appMgrServiceInner->AttachApplication(testPid, mockAppScheduler);
    EXPECT_TRUE(appMgrServiceInner->sceneBoardAttachFlag_);
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    TAG_LOGI(AAFwkTag::TEST, "AttachApplication_010 end");
}

/**
 * @tc.name: ApplicationBackgrounded_001
 * @tc.desc: Test ApplicationBackgrounded with null app record
 * @tc.type: FUNC
 * @tc.require: Test ApplicationBackgrounded method
 */
HWTEST_F(AppMgrServiceInnerNinthTest, ApplicationBackgrounded_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ApplicationBackgrounded_001 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    const int32_t testRecordId = 1234;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = nullptr;
    appMgrServiceInner->ApplicationBackgrounded(testRecordId);
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().addAppLifecycleEventCalled_);
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    TAG_LOGI(AAFwkTag::TEST, "ApplicationBackgrounded_001 end");
}

/**
 * @tc.name: ApplicationBackgrounded_002
 * @tc.desc: Test ApplicationBackgrounded with invalid schedule state
 * @tc.type: FUNC
 * @tc.require: Test ApplicationBackgrounded method
 */
HWTEST_F(AppMgrServiceInnerNinthTest, ApplicationBackgrounded_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ApplicationBackgrounded_002 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    const int32_t testRecordId = 1234;
    const std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, testRecordId, processName);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = appRecord;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.clear();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.insert({testRecordId, appRecord});
    appRecord->SetApplicationScheduleState(ApplicationScheduleState::SCHEDULE_FOREGROUNDING);
    appMgrServiceInner->ApplicationBackgrounded(testRecordId);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().addAppLifecycleEventCalled_);
    EXPECT_NE(appRecord->GetApplicationScheduleState(), ApplicationScheduleState::SCHEDULE_READY);
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    TAG_LOGI(AAFwkTag::TEST, "ApplicationBackgrounded_002 end");
}

/**
 * @tc.name: ApplicationBackgrounded_003
 * @tc.desc: Test ApplicationBackgrounded with valid foreground app
 * @tc.type: FUNC
 * @tc.require: Test ApplicationBackgrounded method
 */
HWTEST_F(AppMgrServiceInnerNinthTest, ApplicationBackgrounded_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ApplicationBackgrounded_003 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    const int32_t testRecordId = 1234;
    const std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, testRecordId, processName);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = appRecord;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.clear();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.insert({testRecordId, appRecord});
    
    // Set valid conditions for backgrounding
    appRecord->SetApplicationScheduleState(ApplicationScheduleState::SCHEDULE_BACKGROUNDING);
    
    appMgrServiceInner->ApplicationBackgrounded(testRecordId);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().setApplicationScheduleStateCalled_);
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    TAG_LOGI(AAFwkTag::TEST, "ApplicationBackgrounded_003 end");
}

/**
 * @tc.name: ApplicationBackgrounded_004
 * @tc.desc: Test ApplicationBackgrounded with non-foreground app
 * @tc.type: FUNC
 * @tc.require: Test ApplicationBackgrounded method
 */
HWTEST_F(AppMgrServiceInnerNinthTest, ApplicationBackgrounded_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ApplicationBackgrounded_004 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    const int32_t testRecordId = 1234;
    const std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, testRecordId, processName);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = appRecord;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.clear();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.insert({testRecordId, appRecord});
    appRecord->SetApplicationScheduleState(ApplicationScheduleState::SCHEDULE_BACKGROUNDING);
    appRecord->SetState(ApplicationState::APP_STATE_FOREGROUND);
    appMgrServiceInner->ApplicationBackgrounded(testRecordId);
    EXPECT_EQ(appRecord->GetState(), ApplicationState::APP_STATE_BACKGROUND);
    
    TAG_LOGI(AAFwkTag::TEST, "ApplicationBackgrounded_004 end");
}

/**
 * @tc.name: ApplicationBackgrounded_005
 * @tc.desc: Test ApplicationBackgrounded with FOREGROUNDING pending state
 * @tc.type: FUNC
 * @tc.require: Test ApplicationBackgrounded method
 */
HWTEST_F(AppMgrServiceInnerNinthTest, ApplicationBackgrounded_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ApplicationBackgrounded_005 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    const int32_t testRecordId = 1234;
    const std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, testRecordId, processName);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = appRecord;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.clear();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.insert({testRecordId, appRecord});
    
    // Set valid conditions with FOREGROUNDING pending state
    appRecord->SetApplicationScheduleState(ApplicationScheduleState::SCHEDULE_BACKGROUNDING);
    appRecord->SetState(ApplicationState::APP_STATE_BACKGROUND);
    appMgrServiceInner->ApplicationBackgrounded(testRecordId);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().getNameCalled_);
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    TAG_LOGI(AAFwkTag::TEST, "ApplicationBackgrounded_005 end");
}

/**
 * @tc.name: ApplicationBackgrounded_006
 * @tc.desc: Test ApplicationBackgrounded with BACKGROUNDING pending state
 * @tc.type: FUNC
 * @tc.require: Test ApplicationBackgrounded method
 */
HWTEST_F(AppMgrServiceInnerNinthTest, ApplicationBackgrounded_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ApplicationBackgrounded_006 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    const int32_t testRecordId = 1234;
    const std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, testRecordId, processName);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = appRecord;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.clear();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.insert({testRecordId, appRecord});
    
    // Set valid conditions with BACKGROUNDING pending state
    appRecord->SetApplicationScheduleState(ApplicationScheduleState::SCHEDULE_BACKGROUNDING);
    appRecord->SetState(ApplicationState::APP_STATE_FOREGROUND);
    appRecord->SetApplicationPendingState(ApplicationPendingState::FOREGROUNDING);
    appMgrServiceInner->ApplicationBackgrounded(testRecordId);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().scheduleForegroundRunningCalled_);
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    TAG_LOGI(AAFwkTag::TEST, "ApplicationBackgrounded_006 end");
}

/**
 * @tc.name: ApplicationBackgrounded_007
 * @tc.desc: Test ApplicationBackgrounded with UI extension type
 * @tc.type: FUNC
 * @tc.require: Test ApplicationBackgrounded method
 */
HWTEST_F(AppMgrServiceInnerNinthTest, ApplicationBackgrounded_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ApplicationBackgrounded_007 start");
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    const int32_t testRecordId = 1234;
    const std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, testRecordId, processName);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = appRecord;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.clear();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.insert({testRecordId, appRecord});
    
    // Set valid conditions with BACKGROUNDING pending state
    appRecord->SetApplicationScheduleState(ApplicationScheduleState::SCHEDULE_BACKGROUNDING);
    appRecord->SetState(ApplicationState::APP_STATE_FOREGROUND);
    appRecord->SetApplicationPendingState(ApplicationPendingState::BACKGROUNDING);
    appMgrServiceInner->ApplicationBackgrounded(testRecordId);
    EXPECT_TRUE(appRecord->GetApplicationPendingState() == ApplicationPendingState::READY);
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    
    appRecord->SetApplicationScheduleState(ApplicationScheduleState::SCHEDULE_BACKGROUNDING);
    appRecord->SetState(ApplicationState::APP_STATE_READY);
    appRecord->SetApplicationPendingState(ApplicationPendingState::BACKGROUNDING);
    appMgrServiceInner->ApplicationBackgrounded(testRecordId);
    EXPECT_TRUE(appRecord->GetApplicationPendingState() == ApplicationPendingState::READY);
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();

    TAG_LOGI(AAFwkTag::TEST, "ApplicationBackgrounded_007 end");
}

/**
 * @tc.name: GetRunningProcesses_001
 * @tc.desc: Test GetRunningProcesses function
 * @tc.type: FUNC
 * @tc.require: Test GetRunningProcesses
 */
HWTEST_F(AppMgrServiceInnerNinthTest, GetRunningProcesses_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcesses_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.bundle.name";
    applicationInfo->bundleName = "test.bundle.name";
    const int32_t testRecordId = 1234;
    const std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, testRecordId, processName);
    EXPECT_NE(appRecord, nullptr);
    std::vector<RunningProcessInfo> info;
    EXPECT_EQ(info.size(), 0);
    appMgrServiceInner->GetRunningProcesses(appRecord, info);
    EXPECT_EQ(info.size(), 1);
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcesses_001 end");
}

/**
 * @tc.name: GetRunningProcess_002
 * @tc.desc: Test GetRunningProcess function
 * @tc.type: FUNC
 * @tc.require: Test GetRunningProcess method
 */
HWTEST_F(AppMgrServiceInnerNinthTest, GetRunningProcess_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcess_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    RunningProcessInfo info;
    appMgrServiceInner->GetRunningProcess(appRecord, info);
    EXPECT_TRUE(info.processName_.empty());
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcess_002 end");
}

/**
 * @tc.name: GetRunningProcess_003
 * @tc.desc: Test GetRunningProcess function
 * @tc.type: FUNC
 * @tc.require: Test GetRunningProcess method
 */
HWTEST_F(AppMgrServiceInnerNinthTest, GetRunningProcess_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcess_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    // Create valid application info
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.app.name";
    applicationInfo->bundleName = "test.bundle.name";
    const int32_t testRecordId = 1234;
    const std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, testRecordId, processName);
    EXPECT_NE(appRecord, nullptr);
    
    RunningProcessInfo info;
    appMgrServiceInner->GetRunningProcess(appRecord, info);
    EXPECT_EQ(info.processName_, processName);
    
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcess_003 end");
}

/**
 * @tc.name: GetRunningProcess_004
 * @tc.desc: Test GetRunningProcess
 * @tc.type: FUNC
 * @tc.require: Test GetRunningProcess
 */
HWTEST_F(AppMgrServiceInnerNinthTest, GetRunningProcess_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcess_004 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.app.name";
    applicationInfo->bundleName = "test.bundle.name";
    const int32_t testRecordId = 1234;
    const std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, testRecordId, processName);
    auto userTestInfo = std::make_shared<UserTestRecord>();
    appRecord->SetUserTestInfo(userTestInfo);
    RunningProcessInfo info;
    AAFwk::MyStatus::GetInstance().getBoolParameter_ = true;
    info.isTestMode = false;
    appMgrServiceInner->GetRunningProcess(appRecord, info);
    EXPECT_TRUE(info.isTestMode);
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcess_004 end");
}

/**
 * @tc.name: GetRunningProcess_005
 * @tc.desc: Test GetRunningProcess function
 * @tc.type: FUNC
 * @tc.require: Test GetRunningProcess method
 */
HWTEST_F(AppMgrServiceInnerNinthTest, GetRunningProcess_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcess_005 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.app.name";
    applicationInfo->bundleName = "test.bundle.name";
    applicationInfo->bundleType = BundleType::APP;
    const int32_t testRecordId = 1234;
    const std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, testRecordId, processName);
    EXPECT_NE(appRecord, nullptr);
    
    RunningProcessInfo info;
    appMgrServiceInner->GetRunningProcess(appRecord, info);
    EXPECT_EQ(info.bundleType, static_cast<int32_t>(BundleType::APP));
    
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcess_005 end");
}

/**
 * @tc.name: GetRunningProcess_006
 * @tc.desc: Test GetRunningProcess function
 * @tc.type: FUNC
 * @tc.require: Test GetRunningProcess
 */
HWTEST_F(AppMgrServiceInnerNinthTest, GetRunningProcess_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcess_006 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = "test.app.name";
    applicationInfo->bundleName = "test.bundle.name";
    applicationInfo->multiAppMode.multiAppModeType = MultiAppModeType::APP_CLONE;
    const int32_t testRecordId = 1234;
    const std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, testRecordId, processName);
    EXPECT_NE(appRecord, nullptr);
    const int32_t testAppIndex = 5;
    appRecord->SetAppIndex(testAppIndex);
    RunningProcessInfo info;
    appMgrServiceInner->GetRunningProcess(appRecord, info);
    EXPECT_EQ(info.appCloneIndex, testAppIndex);

    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcess_006 end");
}

/**
 * @tc.name: WaitForRemoteProcessExit_001
 * @tc.desc: Test WaitForRemoteProcessExit when all processes exit immediately
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, WaitForRemoteProcessExit_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "WaitForRemoteProcessExit_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    // Create a list of PIDs that don't exist (simulating processes that have already exited)
    std::list<pid_t> pids = {99999, 88888}; // Non-existent PIDs
    int64_t startTime = appMgrServiceInner->SystemTimeMillisecond();
    
    // Since ProcessUtil::CheckAllProcessExit will return true for non-existent PIDs,
    // the function should return true immediately
    bool result = appMgrServiceInner->WaitForRemoteProcessExit(pids, startTime);
    EXPECT_TRUE(result);

    TAG_LOGI(AAFwkTag::TEST, "WaitForRemoteProcessExit_001 end");
}

/**
 * @tc.name: WaitForRemoteProcessExit_002
 * @tc.desc: Test WaitForRemoteProcessExit timeout scenario
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, WaitForRemoteProcessExit_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "WaitForRemoteProcessExit_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    // Create a list with PID 1 (init process that will always exist)
    std::list<pid_t> pids = {1};
    // Set start time far in the past to simulate timeout condition
    int64_t startTime = appMgrServiceInner->SystemTimeMillisecond() - 2000; // 2 seconds ago
    
    // The function should timeout and return false since PID 1 will always exist
    bool result = appMgrServiceInner->WaitForRemoteProcessExit(pids, startTime);
    EXPECT_FALSE(result);

    TAG_LOGI(AAFwkTag::TEST, "WaitForRemoteProcessExit_002 end");
}

/**
 * @tc.name: WaitForRemoteProcessExit_003
 * @tc.desc: Test WaitForRemoteProcessExit with empty PID list
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, WaitForRemoteProcessExit_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "WaitForRemoteProcessExit_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    // Create an empty list of PIDs
    std::list<pid_t> pids;
    int64_t startTime = appMgrServiceInner->SystemTimeMillisecond();
    
    // With an empty list, CheckAllProcessExit should return true immediately
    bool result = appMgrServiceInner->WaitForRemoteProcessExit(pids, startTime);
    EXPECT_TRUE(result);

    TAG_LOGI(AAFwkTag::TEST, "WaitForRemoteProcessExit_003 end");
}

/**
 * @tc.name: StartAbility_001
 * @tc.desc: Test StartAbility with null abilityInfo parameter
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, StartAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbility_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    // Reset flags
    AAFwk::MyStatus::GetInstance().resetStartAbilityFlags();

    // Create test parameters
    sptr<IRemoteObject> token = new MockAppScheduler();
    sptr<IRemoteObject> preToken = nullptr;
    std::shared_ptr<AbilityInfo> abilityInfo = nullptr; // Null abilityInfo
    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 1, "test_process");
    HapModuleInfo hapModuleInfo;
    auto want = std::make_shared<AAFwk::Want>();
    int32_t abilityRecordId = 1;

    // Call StartAbility with null abilityInfo
    appMgrServiceInner->StartAbility(token, preToken, abilityInfo, appRecord, hapModuleInfo, want, abilityRecordId);

    // Verify that the function returns early due to null abilityInfo
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().startAbility_nullAbilityInfo_called_);

    TAG_LOGI(AAFwkTag::TEST, "StartAbility_001 end");
}

/**
 * @tc.name: StartAbility_002
 * @tc.desc: Test StartAbility with null appRecord parameter
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, StartAbility_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbility_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    // Reset flags
    AAFwk::MyStatus::GetInstance().resetStartAbilityFlags();

    // Create test parameters
    sptr<IRemoteObject> token = new MockAppScheduler();
    sptr<IRemoteObject> preToken = nullptr;
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->bundleName = "com.test.bundle";
    abilityInfo->name = "TestAbility";
    std::shared_ptr<AppRunningRecord> appRecord = nullptr; // Null appRecord
    HapModuleInfo hapModuleInfo;
    auto want = std::make_shared<AAFwk::Want>();
    int32_t abilityRecordId = 1;

    // Call StartAbility with null appRecord
    appMgrServiceInner->StartAbility(token, preToken, abilityInfo, appRecord, hapModuleInfo, want, abilityRecordId);

    // Verify that the function returns early due to null appRecord
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().startAbility_nullAppRecord_called_);

    TAG_LOGI(AAFwkTag::TEST, "StartAbility_002 end");
}

/**
 * @tc.name: StartAbility_003
 * @tc.desc: Test StartAbility with singleton mode and existing ability
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, StartAbility_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbility_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    // Reset flags
    AAFwk::MyStatus::GetInstance().resetStartAbilityFlags();

    // Create test parameters
    sptr<IRemoteObject> token = new MockAppScheduler();
    sptr<IRemoteObject> preToken = nullptr;
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->bundleName = "com.test.bundle";
    abilityInfo->name = "TestAbility";
    abilityInfo->launchMode = LaunchMode::SINGLETON;
    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 1, "test_process");
    HapModuleInfo hapModuleInfo;
    auto want = std::make_shared<AAFwk::Want>();
    int32_t abilityRecordId = 1;

    // Set up mock to return existing ability for singleton mode
    auto existingAbility = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 1);
    AAFwk::MyStatus::GetInstance().getAbilityRunningRecordByToken_ = existingAbility;
    appRecord->securityFlag_=true; // Simulate security flag for singleton ability
    // Call StartAbility with singleton mode and existing ability
    appMgrServiceInner->StartAbility(token, preToken, abilityInfo, appRecord, hapModuleInfo, want, abilityRecordId);

    EXPECT_TRUE(want->GetBoolParam(DLP_PARAMS_SECURITY_FLAG, false));

    TAG_LOGI(AAFwkTag::TEST, "StartAbility_003 end");
}

/**
 * @tc.name: StartAbility_004
 * @tc.desc: Test StartAbility with existing ability and preToken
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, StartAbility_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbility_004 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    // Reset flags
    AAFwk::MyStatus::GetInstance().resetStartAbilityFlags();

    // Create test parameters
    sptr<IRemoteObject> token = new MockAppScheduler();
    sptr<IRemoteObject> preToken = new MockAppScheduler(); // Non-null preToken
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->bundleName = "com.test.bundle";
    abilityInfo->name = "TestAbility";
    abilityInfo->launchMode = LaunchMode::STANDARD;
    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 1, "test_process");
    HapModuleInfo hapModuleInfo;
    auto want = std::make_shared<AAFwk::Want>();
    want->SetParam(DEBUG_APP, true);
    appRecord->SetDebugApp(false);
    int32_t abilityRecordId = 1;

    // Set up mock to return existing ability
    auto existingAbility = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 1);
    AAFwk::MyStatus::GetInstance().getAbilityRunningRecordByToken_ = existingAbility;

    // Call StartAbility with existing ability and preToken
    appMgrServiceInner->StartAbility(token, preToken, abilityInfo, appRecord, hapModuleInfo, want, abilityRecordId);

    // Verify that the function returns early due to ability already existing with preToken
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().debugAppCalledTimes_ == 2);

    TAG_LOGI(AAFwkTag::TEST, "StartAbility_004 end");
}

/**
 * @tc.name: StartAbility_005
 * @tc.desc: Test StartAbility with module record creation failure
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, StartAbility_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbility_005 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    // Reset flags
    AAFwk::MyStatus::GetInstance().resetStartAbilityFlags();

    // Create test parameters
    sptr<IRemoteObject> token = new MockAppScheduler();
    sptr<IRemoteObject> preToken = nullptr;
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->bundleName = "com.test.bundle";
    abilityInfo->name = "TestAbility";
    abilityInfo->launchMode = LaunchMode::SINGLETON;
    abilityInfo->applicationInfo.bundleName = "com.test.bundle";
    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 1, "test_process");
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "entry";
    auto want = std::make_shared<AAFwk::Want>();
    int32_t abilityRecordId = 1;
    AAFwk::MyStatus::GetInstance().getAbilityRunningRecordByToken_ =
        std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);

    appMgrServiceInner->StartAbility(token, preToken, abilityInfo, appRecord, hapModuleInfo, want, abilityRecordId);

    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().addModuleCalled_);

    TAG_LOGI(AAFwkTag::TEST, "StartAbility_005 end");
}

/**
 * @tc.name: StartAbility_006
 * @tc.desc: Test StartAbility with ability creation failure
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, StartAbility_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbility_006 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    // Reset flags
    AAFwk::MyStatus::GetInstance().resetStartAbilityFlags();

    // Create test parameters
    sptr<IRemoteObject> token = new MockAppScheduler();
    sptr<IRemoteObject> preToken = new MockAppScheduler(); // Non-null preToken
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->bundleName = "com.test.bundle";
    abilityInfo->name = "TestAbility";
    abilityInfo->launchMode = LaunchMode::STANDARD;
    abilityInfo->applicationInfo.bundleName = "com.test.bundle";
    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 1, "test_process");
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "entry";
    auto want = std::make_shared<AAFwk::Want>();
    int32_t abilityRecordId = 1;

    AAFwk::MyStatus::GetInstance().getAbilityRunningRecordByToken_ =
        std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);

    appMgrServiceInner->StartAbility(token, preToken, abilityInfo, appRecord, hapModuleInfo, want, abilityRecordId);

    // Verify that the function returns early due to ability creation failure
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().addModuleCalledTimes_, 0);
    TAG_LOGI(AAFwkTag::TEST, "StartAbility_006 end");
}

/**
 * @tc.name: StartAbility_007
 * @tc.desc: Test StartAbility with app state CREATE (should not launch)
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, StartAbility_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbility_007 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    // Reset flags
    AAFwk::MyStatus::GetInstance().resetStartAbilityFlags();

    // Create test parameters
    sptr<IRemoteObject> token = new MockAppScheduler();
    sptr<IRemoteObject> preToken = new MockAppScheduler(); // Non-null preToken
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->bundleName = "com.test.bundle";
    abilityInfo->name = "TestAbility";
    abilityInfo->launchMode = LaunchMode::STANDARD;
    abilityInfo->applicationInfo.bundleName = "com.test.bundle";
    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 1, "test_process");
    appRecord->SetState(ApplicationState::APP_STATE_CREATE);
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "entry";
    auto want = std::make_shared<AAFwk::Want>();
    int32_t abilityRecordId = 1;

    AAFwk::MyStatus::GetInstance().getAbilityRunningRecordByToken_ = nullptr; // No existing ability

    appMgrServiceInner->StartAbility(token, preToken, abilityInfo, appRecord, hapModuleInfo, want, abilityRecordId);

    // Verify that the function returns early due to ability creation failure
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().addModuleCalled_);

    TAG_LOGI(AAFwkTag::TEST, "StartAbility_007 end");
}

/**
 * @tc.name: StartAbility_008
 * @tc.desc: Test StartAbility normal flow with successful ability launch
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, StartAbility_008, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbility_008 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    // Reset flags
    AAFwk::MyStatus::GetInstance().resetStartAbilityFlags();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    // Create test parameters
    sptr<IRemoteObject> token = new MockAppScheduler();
    sptr<IRemoteObject> preToken = new MockAppScheduler(); // Non-null preToken
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->bundleName = "com.test.bundle";
    abilityInfo->name = "TestAbility";
    abilityInfo->launchMode = LaunchMode::STANDARD;
    abilityInfo->applicationInfo.bundleName = "com.test.bundle";
    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 1, "test_process");
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "entry";
    auto want = std::make_shared<AAFwk::Want>();
    int32_t abilityRecordId = 1;

    AAFwk::MyStatus::GetInstance().getAbilityRunningRecordByToken_ = nullptr; // No existing ability
    // Set up mock to return valid module record
    auto appInfo = std::make_shared<ApplicationInfo>(abilityInfo->applicationInfo);
    //auto moduleRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);
    AAFwk::MyStatus::GetInstance().getModuleRecord_ = nullptr;

    AAFwk::MyStatus::GetInstance().simulateAddModuleFails_ = true;
    appMgrServiceInner->StartAbility(token, preToken, abilityInfo, appRecord, hapModuleInfo, want, abilityRecordId);

    // Verify that the function completed the normal flow and launched ability
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().getModuleRecordByModuleNameCalled_);
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().getAbilityRunningRecordByTokenCalled_);
    TAG_LOGI(AAFwkTag::TEST, "StartAbility_008 end");
}

/**
 * @tc.name: StartAbility_009
 * @tc.desc: Test StartAbility with debug app processing
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, StartAbility_009, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbility_009 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    AAFwk::MyStatus::GetInstance().resetStartAbilityFlags();
    AAFwk::MyStatus::GetInstance().resetRunningRecordFunctionFlag();
    sptr<IRemoteObject> token = new MockAppScheduler();
    sptr<IRemoteObject> preToken = nullptr;
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->bundleName = "com.test.bundle";
    abilityInfo->name = "TestAbility";
    abilityInfo->launchMode = LaunchMode::STANDARD;
    abilityInfo->applicationInfo.bundleName = "com.test.bundle";
    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 1, "test_process");
    appRecord->SetState(ApplicationState::APP_STATE_READY);
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "entry";
    auto want = std::make_shared<AAFwk::Want>();
    // Set debug app parameter
    want->SetParam(DEBUG_APP, true);
    int32_t abilityRecordId = 1;

    // Set up mock to return null for GetAbilityRunningRecordByToken (no existing ability)
    AAFwk::MyStatus::GetInstance().getAbilityRunningRecordByTokenModule_ = nullptr;
    // Set up mock to return valid module record
    auto appInfo = std::make_shared<ApplicationInfo>(abilityInfo->applicationInfo);
    auto moduleRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);
    AAFwk::MyStatus::GetInstance().getModuleRecord_ = moduleRecord;

    // Call StartAbility with debug app
    appMgrServiceInner->StartAbility(token, preToken, abilityInfo, appRecord, hapModuleInfo, want, abilityRecordId);

    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().getAbilityRunningRecordByTokenCalled_);
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().getStateCalled_);

    TAG_LOGI(AAFwkTag::TEST, "StartAbility_009 end");
}
/**
 * @tc.name: StartAbility_010
 * @tc.desc: Test StartAbility with DLP security flag processing
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, StartAbility_010, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbility_010 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    sptr<IRemoteObject> token = new MockAppScheduler();
    sptr<IRemoteObject> preToken = nullptr;
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->bundleName = "com.test.bundle";
    abilityInfo->name = "TestAbility";
    abilityInfo->launchMode = LaunchMode::STANDARD;
    abilityInfo->applicationInfo.bundleName = "com.test.bundle";
    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 1, "test_process");
    appRecord->SetState(ApplicationState::APP_STATE_CREATE);
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "entry";
    auto want = std::make_shared<AAFwk::Want>();
    // Set debug app parameter
    want->SetParam(DEBUG_APP, true);
    int32_t abilityRecordId = 1;
    AAFwk::MyStatus::GetInstance().getAbilityRunningRecordByTokenModule_ =
        std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    // Set up mock to return valid module record
    auto appInfo = std::make_shared<ApplicationInfo>(abilityInfo->applicationInfo);
    auto moduleRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);
    AAFwk::MyStatus::GetInstance().getModuleRecord_ = moduleRecord;
    appMgrServiceInner->StartAbility(token, preToken, abilityInfo, appRecord, hapModuleInfo, want, abilityRecordId);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().getStateCalled_);
    EXPECT_FALSE(AAFwk::MyStatus::GetInstance().startAbility_launchAbility_called_);
    TAG_LOGI(AAFwkTag::TEST, "StartAbility_010 end");
}

/**
 * @tc.name: StartAbility_011
 * @tc.desc: Test StartAbility with DLP security flag processing
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, StartAbility_011, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbility_011 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    sptr<IRemoteObject> token = new MockAppScheduler();
    sptr<IRemoteObject> preToken = nullptr;
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->bundleName = "com.test.bundle";
    abilityInfo->name = "TestAbility";
    abilityInfo->launchMode = LaunchMode::STANDARD;
    abilityInfo->applicationInfo.bundleName = "com.test.bundle";
    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 1, "test_process");
    appRecord->SetState(ApplicationState::APP_STATE_READY);
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "entry";
    auto want = std::make_shared<AAFwk::Want>();
    // Set debug app parameter
    want->SetParam(DEBUG_APP, true);
    int32_t abilityRecordId = 1;

    AAFwk::MyStatus::GetInstance().getAbilityRunningRecordByTokenModule_ =
        std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    auto appInfo = std::make_shared<ApplicationInfo>(abilityInfo->applicationInfo);
    auto moduleRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);
    AAFwk::MyStatus::GetInstance().getModuleRecord_ = moduleRecord;

    appMgrServiceInner->StartAbility(token, preToken, abilityInfo, appRecord, hapModuleInfo, want, abilityRecordId);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().getStateCalled_);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().startAbility_launchAbility_called_);
    TAG_LOGI(AAFwkTag::TEST, "StartAbility_011 end");
}

/**
 * @tc.name: StartSpecifiedProcess_0100
 * @tc.desc: Test StartSpecifiedProcess
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, StartSpecifiedProcess_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartSpecifiedProcess_0100 start");
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = nullptr;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);
    ASSERT_NE(appMgrServiceInner->remoteClientManager_, nullptr);
    EXPECT_EQ(appMgrServiceInner->remoteClientManager_->GetBundleManagerHelper(), nullptr);
    AAFwk::Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    int32_t requestId = 100;
    std::string customProcess = "customProcess";
    appMgrServiceInner->StartSpecifiedProcess(want, abilityInfo, requestId, customProcess);
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto appInfo = std::make_shared<ApplicationInfo>(abilityInfo.applicationInfo);
    EXPECT_EQ(appMgrServiceInner->GetBundleAndHapInfo(abilityInfo, appInfo, bundleInfo, hapModuleInfo, 0), false);
    TAG_LOGI(AAFwkTag::TEST, "StartSpecifiedProcess_0100 end");
}

/**
 * @tc.name: StartSpecifiedProcess_0200
 * @tc.desc: Test StartSpecifiedProcess
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, StartSpecifiedProcess_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartSpecifiedProcess_0200 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);
    ASSERT_NE(appMgrServiceInner->remoteClientManager_, nullptr);
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    EXPECT_NE(appMgrServiceInner->remoteClientManager_->GetBundleManagerHelper(), nullptr);
    AAFwk::MyStatus::GetInstance().getBundleInfoV9_ = 0;
    AAFwk::MyStatus::GetInstance().getHapModuleInfo_ = true;
    AAFwk::MyStatus::GetInstance().isLogoutUser_ = true;
    AAFwk::Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    int32_t requestId = 100;
    std::string customProcess = "customProcess";
    appMgrServiceInner->StartSpecifiedProcess(want, abilityInfo, requestId, customProcess);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().isLogoutUserCalled_);
    TAG_LOGI(AAFwkTag::TEST, "StartSpecifiedProcess_0200 end");
}

/**
 * @tc.name: StartSpecifiedProcess_0300
 * @tc.desc: Test StartSpecifiedProcess
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, StartSpecifiedProcess_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartSpecifiedProcess_0300 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    ASSERT_NE(appMgrServiceInner->remoteClientManager_, nullptr);
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    EXPECT_NE(appMgrServiceInner->remoteClientManager_->GetBundleManagerHelper(), nullptr);
    AAFwk::MyStatus::GetInstance().getBundleInfoV9_ = 0;
    AAFwk::MyStatus::GetInstance().getHapModuleInfo_ = true;
    AAFwk::MyStatus::GetInstance().isLogoutUser_ = false;
    AAFwk::MyStatus::GetInstance().masterProcessRunningRecord_ = nullptr;
    AAFwk::MyStatus::GetInstance().checkAppRunningCall_ = 0;
    ApplicationInfo appInfo;
    appInfo.name = "app";
    appInfo.bundleName = "bundleName";
    auto app = std::make_shared<ApplicationInfo>(appInfo);
    AAFwk::MyStatus::GetInstance().checkAppRunning_ =
        std::make_shared<AppRunningRecord>(app, 123, "bundleName");
    AAFwk::Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SHARE;
    int32_t requestId = 100;
    std::string customProcess = "customProcess";
    appMgrServiceInner->StartSpecifiedProcess(want, abilityInfo, requestId, customProcess);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().isFindMasterProcessAppRunningRecordCalled_);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().checkAppRunningCall_, 1);
    TAG_LOGI(AAFwkTag::TEST, "StartSpecifiedProcess_0300 end");
}

/**
 * @tc.name: StartSpecifiedProcess_0400
 * @tc.desc: Test StartSpecifiedProcess
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, StartSpecifiedProcess_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartSpecifiedProcess_0400 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    ASSERT_NE(appMgrServiceInner->remoteClientManager_, nullptr);
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    EXPECT_NE(appMgrServiceInner->remoteClientManager_->GetBundleManagerHelper(), nullptr);
    AAFwk::MyStatus::GetInstance().getBundleInfoV9_ = 0;
    AAFwk::MyStatus::GetInstance().getHapModuleInfo_ = true;
    AAFwk::MyStatus::GetInstance().isLogoutUser_ = false;
    AAFwk::MyStatus::GetInstance().masterProcessRunningRecord_ = nullptr;
    AAFwk::MyStatus::GetInstance().checkAppRunningCall_ = 0;
    ApplicationInfo appInfo;
    appInfo.name = "app";
    appInfo.bundleName = "bundleName";
    auto app = std::make_shared<ApplicationInfo>(appInfo);
    AAFwk::MyStatus::GetInstance().checkAppRunning_ =
        std::make_shared<AppRunningRecord>(app, 123, "bundleName");
    AAFwk::Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    int32_t requestId = 100;
    std::string customProcess = "customProcess";
    appMgrServiceInner->StartSpecifiedProcess(want, abilityInfo, requestId, customProcess);
    EXPECT_NE(AAFwk::MyStatus::GetInstance().checkAppRunningCall_, 0);
    TAG_LOGI(AAFwkTag::TEST, "StartSpecifiedProcess_0400 end");
}

/**
 * @tc.name: StartSpecifiedProcess_0500
 * @tc.desc: Test StartSpecifiedProcess
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, StartSpecifiedProcess_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartSpecifiedProcess_0500 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    ASSERT_NE(appMgrServiceInner->remoteClientManager_, nullptr);
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    EXPECT_NE(appMgrServiceInner->remoteClientManager_->GetBundleManagerHelper(), nullptr);
    AAFwk::MyStatus::GetInstance().getBundleInfoV9_ = 0;
    AAFwk::MyStatus::GetInstance().getHapModuleInfo_ = true;
    AAFwk::MyStatus::GetInstance().isLogoutUser_ = false;
    AAFwk::MyStatus::GetInstance().v9BundleInfo_.name = "com.test.app";
    ApplicationInfo appInfo;
    appInfo.name = "app";
    appInfo.bundleName = "bundleName";
    auto app = std::make_shared<ApplicationInfo>(appInfo);
    AAFwk::MyStatus::GetInstance().masterProcessRunningRecord_ = nullptr;
    AAFwk::MyStatus::GetInstance().checkAppRunning_ = nullptr;
    AAFwk::MyStatus::GetInstance().appRecordForSpecifiedProcess_ =
        std::make_shared<AppRunningRecord>(app, 123, "bundleName1");
    AAFwk::Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    int32_t requestId = 100;
    std::string customProcess = "customProcess";
    std::string bundleName = "com.test.app";
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.onNewProcessEnableList_.isLoaded = true;
    appUtils.onNewProcessEnableList_.value.emplace_back("com.test.app");
    appMgrServiceInner->StartSpecifiedProcess(want, abilityInfo, requestId, customProcess);
    EXPECT_TRUE(AAFwk::MyStatus::GetInstance().isCheckAppRunningRecordForSpecifiedProcessCalled_);
    TAG_LOGI(AAFwkTag::TEST, "StartSpecifiedProcess_0500 end");
}

/**
 * @tc.name: StartSpecifiedProcess_0600
 * @tc.desc: Test StartSpecifiedProcess
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, StartSpecifiedProcess_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartSpecifiedProcess_0600 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    auto response = sptr<MockStartSpecifiedAbilityResponse>::MakeSptr();
    appMgrServiceInner->startSpecifiedAbilityResponse_ = response;
    bool isOnNewProcessRequestResponseCalled = false;
    EXPECT_CALL(*response, OnNewProcessRequestResponse)
        .WillOnce([&isOnNewProcessRequestResponseCalled](const std::string&, int32_t, int32_t,
            const std::string&, const int32_t &) {
            isOnNewProcessRequestResponseCalled = true;
        });
    ASSERT_NE(appMgrServiceInner->startSpecifiedAbilityResponse_, nullptr);
    ASSERT_NE(appMgrServiceInner->remoteClientManager_, nullptr);
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    EXPECT_NE(appMgrServiceInner->remoteClientManager_->GetBundleManagerHelper(), nullptr);
    AAFwk::MyStatus::GetInstance().getBundleInfoV9_ = 0;
    AAFwk::MyStatus::GetInstance().getHapModuleInfo_ = true;
    AAFwk::MyStatus::GetInstance().getSandboxHapModuleInfo_ = 0;
    AAFwk::MyStatus::GetInstance().isLogoutUser_ = false;
    AAFwk::MyStatus::GetInstance().masterProcessRunningRecord_ = nullptr;
    AAFwk::MyStatus::GetInstance().checkAppRunning_ = nullptr;
    AAFwk::MyStatus::GetInstance().appRecordForSpecifiedProcess_ = nullptr;
    AAFwk::Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    int32_t requestId = 100;
    std::string customProcess = "customProcess";
    appMgrServiceInner->StartSpecifiedProcess(want, abilityInfo, requestId, customProcess);
    EXPECT_TRUE(isOnNewProcessRequestResponseCalled);
    TAG_LOGI(AAFwkTag::TEST, "StartSpecifiedProcess_0600 end");
}

/**
 * @tc.name: StartSpecifiedAbility_0100
 * @tc.desc: Test StartSpecifiedAbilityResponseProxy
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, StartSpecifiedAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartSpecifiedAbility_0100 start");
    auto response = sptr<MockStartSpecifiedAbilityResponse>::MakeSptr();
    auto proxy = std::make_shared<StartSpecifiedAbilityResponseProxy>(response);
    AAFwk::Want want;
    std::string flag = "";
    int32_t requestId = 0;
    int32_t userId = 100;
    proxy->OnAcceptWantResponse(want, flag, requestId, userId);
    EXPECT_EQ(userId, 100);
    TAG_LOGI(AAFwkTag::TEST, "StartSpecifiedAbility_0100 end");
}

/**
 * @tc.name: StartSpecifiedAbility_0200
 * @tc.desc: Test StartSpecifiedAbilityResponseProxy
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, StartSpecifiedAbility_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartSpecifiedAbility_0200 start");
    auto response = sptr<MockStartSpecifiedAbilityResponse>::MakeSptr();
    auto proxy = std::make_shared<StartSpecifiedAbilityResponseProxy>(response);
    int32_t requestId = 0;
    int32_t userId = 100;
    proxy->OnTimeoutResponse(requestId, userId);
    EXPECT_EQ(userId, 100);
    TAG_LOGI(AAFwkTag::TEST, "StartSpecifiedAbility_0200 end");
}

/**
 * @tc.name: StartSpecifiedAbility_0300
 * @tc.desc: Test StartSpecifiedAbilityResponseProxy
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, StartSpecifiedAbility_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartSpecifiedAbility_0300 start");
    auto response = sptr<MockStartSpecifiedAbilityResponse>::MakeSptr();
    auto proxy = std::make_shared<StartSpecifiedAbilityResponseProxy>(response);
    std::string flag = "";
    int32_t userId = 100;
    int32_t requestId = 0;
    std::string callerProcessName = "";
    int32_t recordId = 0;
    proxy->OnNewProcessRequestResponse(flag, userId, requestId, callerProcessName, recordId);
    EXPECT_EQ(userId, 100);
    TAG_LOGI(AAFwkTag::TEST, "StartSpecifiedAbility_0300 end");
}

/**
 * @tc.name: StartSpecifiedAbility_0400
 * @tc.desc: Test StartSpecifiedAbilityResponseProxy
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, StartSpecifiedAbility_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartSpecifiedAbility_0400 start");
    auto response = sptr<MockStartSpecifiedAbilityResponse>::MakeSptr();
    auto proxy = std::make_shared<StartSpecifiedAbilityResponseProxy>(response);
    int32_t requestId = 0;
    int32_t userId = 100;
    proxy->OnNewProcessRequestTimeoutResponse(requestId, userId);
    EXPECT_EQ(userId, 100);
    TAG_LOGI(AAFwkTag::TEST, "StartSpecifiedAbility_0400 end");
}

/**
 * @tc.name: StartSpecifiedAbility_0500
 * @tc.desc: Test StartSpecifiedAbilityResponseProxy
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, StartSpecifiedAbility_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartSpecifiedAbility_0500 start");
    auto response = sptr<MockStartSpecifiedAbilityResponse>::MakeSptr();
    auto proxy = std::make_shared<StartSpecifiedAbilityResponseProxy>(response);
    int32_t requestId = 0;
    int32_t userId = 100;
    proxy->OnStartSpecifiedFailed(requestId, userId);
    EXPECT_EQ(userId, 100);
    TAG_LOGI(AAFwkTag::TEST, "StartSpecifiedAbility_0500 end");
}

/**
* @tc.name: AllowScbProcessMoveToBackground_001
* @tc.desc: test AllowScbProcessMoveToBackground_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerNinthTest, AllowScbProcessMoveToBackground_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AllowScbProcessMoveToBackground_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);
    std::string temp = "";
    std::shared_ptr<ApplicationInfo> info1 = std::make_shared<ApplicationInfo>();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(info1, 0, temp);
    appMgrServiceInner->AllowScbProcessMoveToBackground();
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_->IsAllowScbProcessMoveToBackground(), false);
    TAG_LOGI(AAFwkTag::TEST, "AllowScbProcessMoveToBackground_001 end");
}

/**
 * @tc.name: KillProcessByPidForExit_0100
 * @tc.desc: Test KillProcessByPidForExit
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, KillProcessByPidForExit_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillProcessByPidForExit_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = nullptr;
    auto ret = appMgrServiceInner->KillProcessByPidForExit(0, "");
    EXPECT_EQ(ret, ERR_NO_INIT);

    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    AAFwk::MyStatus::GetInstance().exitByPidstatus_ = false;
    ret = appMgrServiceInner->KillProcessByPidForExit(0, "");
    EXPECT_EQ(ret, ERR_OK);

    AAFwk::MyStatus::GetInstance().exitByPidstatus_ = true;
    ret = appMgrServiceInner->KillProcessByPidForExit(0, "");
    EXPECT_EQ(ret, ERR_OK);

    AAFwk::MyStatus::GetInstance().exitByPidstatus_ = false;
    TAG_LOGI(AAFwkTag::TEST, "KillProcessByPidForExit_0100 end");
}

/**
 * @tc.name: SignRestartProcess_001
 * @tc.desc: test SignRestartProcess_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, SignRestartProcess_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SignRestartProcess_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto pid = 100;
    auto ret = appMgrServiceInner->SignRestartProcess(pid);
    EXPECT_EQ(ret, ERR_OK);

    appMgrServiceInner->appRunningManager_ = nullptr;
    ret = appMgrServiceInner->SignRestartProcess(pid);
    EXPECT_EQ(ret, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "SignRestartProcess_001 end");
}

/**
 * @tc.name: SetProcessPrepareExit_0100
 * @tc.desc: Test SetProcessPrepareExit
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, SetProcessPrepareExit_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetProcessPrepareExit_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = nullptr;
    appMgrServiceInner->SetProcessPrepareExit(0);

    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = appRecord;
    appMgrServiceInner->SetProcessPrepareExit(0);
    EXPECT_TRUE(appRecord->isPrepareExit_);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = nullptr;
    TAG_LOGI(AAFwkTag::TEST, "SetProcessPrepareExit_0100 end");
}

/**
 * @tc.name: KillChildProcessByPid_0100
 * @tc.desc: Test KillChildProcessByPid
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, KillChildProcessByPid_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillChildProcessByPid_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    ASSERT_NE(applicationInfo, nullptr);
    applicationInfo->name = "test.app.name";
    applicationInfo->bundleName = "test.bundle.name";
    const int32_t hostPid = 1234;
    const std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, 1, processName);
    ASSERT_NE(appRecord, nullptr);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = appRecord;
    int32_t childPid = 1;
    ChildProcessRequest childRequest;
    childRequest.childProcessType = 1;
    auto childProcessRecord = std::make_shared<ChildProcessRecord>(hostPid, childRequest, appRecord);
    childProcessRecord->SetPid(childPid);
    AAFwk::MyStatus::GetInstance().getChildProcessRecordByPid_ = childProcessRecord;
    auto ret = appMgrServiceInner->KillChildProcessByPid(childPid);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "KillChildProcessByPid_0100 end");
}

/**
 * @tc.name: KillChildProcessByPid_0200
 * @tc.desc: Test KillChildProcessByPid
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, KillChildProcessByPid_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillChildProcessByPid_0200 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    ASSERT_NE(applicationInfo, nullptr);
    applicationInfo->name = "test.app.name";
    applicationInfo->bundleName = "test.bundle.name";
    const int32_t hostPid = 1234;
    const std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, 1, processName);
    ASSERT_NE(appRecord, nullptr);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = appRecord;
    int32_t childPid = 1;
    ChildProcessRequest childRequest;
    childRequest.childProcessType = 1;
    auto childProcessRecord = std::make_shared<ChildProcessRecord>(hostPid, childRequest, appRecord);
    childProcessRecord->SetPid(childPid);
    AAFwk::MyStatus::GetInstance().getChildProcessRecordByPid_ = nullptr;
    auto ret = appMgrServiceInner->KillChildProcessByPid(childPid);
    EXPECT_EQ(ret, AAFwk::ERR_INVALID_PID);
    TAG_LOGI(AAFwkTag::TEST, "KillChildProcessByPid_0200 end");
}

/**
 * @tc.name: KillChildProcessByPid_0300
 * @tc.desc: Test KillChildProcessByPid
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerNinthTest, KillChildProcessByPid_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillChildProcessByPid_0300 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    ASSERT_NE(applicationInfo, nullptr);
    applicationInfo->name = "test.app.name";
    applicationInfo->bundleName = "test.bundle.name";
    const int32_t hostPid = 1234;
    const std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, 1, processName);
    ASSERT_NE(appRecord, nullptr);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = nullptr;
    int32_t childPid = 1;
    ChildProcessRequest childRequest;
    childRequest.childProcessType = 1;
    auto childProcessRecord = std::make_shared<ChildProcessRecord>(hostPid, childRequest, appRecord);
    childProcessRecord->SetPid(childPid);
    AAFwk::MyStatus::GetInstance().getChildProcessRecordByPid_ = childProcessRecord;
    auto ret = appMgrServiceInner->KillChildProcessByPid(childPid);
    EXPECT_EQ(ret, AAFwk::ERR_INVALID_PID);
    AAFwk::MyStatus::GetInstance().getChildProcessRecordByPid_ = nullptr;
    TAG_LOGI(AAFwkTag::TEST, "KillChildProcessByPid_0300 end");
}
}  // namespace AppExecFwk
}  // namespace OHOS
