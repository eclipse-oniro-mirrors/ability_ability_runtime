/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#include "app_mgr_service_inner.h"
#include "app_running_record.h"
#include "app_utils.h"
#include "remote_client_manager.h"
#undef private
#include "ability_manager_errors.h"
#include "appfreeze_manager.h"
#include "hilog_tag_wrapper.h"
#include "mock_ability_token.h"
#include "mock_app_mgr_service_inner.h"
#include "mock_ipc_skeleton.h"
#include "mock_my_flag.h"
#include "mock_native_token.h"
#include "mock_kia_interceptor.h"
#include "mock_permission_verification.h"
#include "mock_sa_call.h"
#include "mock_system_ability_manager.h"
#include "param.h"
#include "ability_debug_response_proxy.h"
#include "appspawn_util.h"
#include "mock_ability_debug_response_stub.h"
#include "mock_task_handler_wrap.h"
#include "mock_parameters.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AAFwk;

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t APP_DEBUG_INFO_PID = 0;
constexpr int32_t APP_DEBUG_INFO_UID = 0;
constexpr int32_t DEFAULT_INVAL_VALUE = -1;
constexpr int32_t USER_SCALE = 200000;
constexpr int32_t TEST_PID_100 = 100;
constexpr int32_t PID_1000 = 1000;
constexpr int32_t FOUNDATION_UID = 5523;
constexpr int32_t UID_ONE = 1;
constexpr int32_t UID_TWO = 2;
const std::string PARAM_SPECIFIED_PROCESS_FLAG = "ohoSpecifiedProcessFlag";
const std::string TEST_FLAG = "testFlag";
const std::string TEST_PROCESS_NAME = "testProcessName";
const std::string TEST_BUNDLE_NAME = "testBundleName";
constexpr const char* DEVELOPER_MODE_STATE = "const.security.developermode.state";
static constexpr int64_t NANOSECONDS = 1000000000;  // NANOSECONDS mean 10^9 nano second
static constexpr int64_t MICROSECONDS = 1000000;    // MICROSECONDS mean 10^6 millias second
constexpr const char* KEY_WATERMARK_BUSINESS_NAME = "com.ohos.param.watermarkBusinessName";
constexpr const char* KEY_IS_WATERMARK_ENABLED = "com.ohos.param.isWatermarkEnabled";
constexpr const char* UIEXTENSION_ABILITY_ID = "ability.want.params.uiExtensionAbilityId";
}
int32_t g_recordId = 0;
class AppMgrServiceInnerSecondTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    void InitAppInfo(const std::string& deviceName, const std::string& abilityName,
        const std::string& appName, const std::string& bundleName, const std::string& moduleName);
    std::shared_ptr<BundleMgrHelper> bundleMgrHelper_{ nullptr };
public:
    std::shared_ptr<AbilityInfo> abilityInfo_;
    std::shared_ptr<ApplicationInfo> applicationInfo_;
    std::shared_ptr<Want> want_;
    sptr<MockAbilityToken> token_;
    sptr<MockAbilityToken> preToken_;
};

void AppMgrServiceInnerSecondTest::InitAppInfo(const std::string& deviceName,
    const std::string& abilityName, const std::string& appName, const std::string& bundleName,
    const std::string& moduleName)
{
    ApplicationInfo applicationInfo;
    applicationInfo.name = appName;
    applicationInfo.bundleName = bundleName;
    applicationInfo_ = std::make_shared<ApplicationInfo>(applicationInfo);

    AbilityInfo abilityInfo;
    abilityInfo.visible = true;
    abilityInfo.applicationName = appName;
    abilityInfo.type = AbilityType::EXTENSION;
    abilityInfo.name = abilityName;
    abilityInfo.bundleName = bundleName;
    abilityInfo.moduleName = moduleName;
    abilityInfo.deviceId = deviceName;
    abilityInfo_ = std::make_shared<AbilityInfo>(abilityInfo);
    want_ = std::make_shared<Want>();
    token_ = sptr<MockAbilityToken>::MakeSptr();
    preToken_ = sptr<MockAbilityToken>::MakeSptr();
}

void AppMgrServiceInnerSecondTest::SetUpTestCase(void)
{
    MockNativeToken::SetNativeToken();
}

void AppMgrServiceInnerSecondTest::TearDownTestCase(void)
{}

void AppMgrServiceInnerSecondTest::SetUp()
{
    // init test app info
    std::string deviceName = "device";
    std::string abilityName = "ServiceAbility";
    std::string appName = "hiservcie";
    std::string bundleName = TEST_BUNDLE_NAME;
    std::string moduleName = "entry";
    InitAppInfo(deviceName, abilityName, appName, bundleName, moduleName);
    bundleMgrHelper_ = DelayedSingleton<BundleMgrHelper>::GetInstance();
}

void AppMgrServiceInnerSecondTest::TearDown()
{}

class IKiaInterceptorTest : public IKiaInterceptor {
    public:
    IKiaInterceptorTest() = default;
    virtual ~IKiaInterceptorTest() = default;
    int OnIntercept(AAFwk::Want &want) override { return 0; }
};

/**
 * @tc.name: AppMgrServiceInnerSecondTest_GetSpecifiedProcessFlag_0100
 * @tc.desc: Test GetSpecifiedProcessFlag
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_GetSpecifiedProcessFlag_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetSpecifiedProcessFlag_0100 start");
    std::shared_ptr<AbilityInfo> abilityInfo = nullptr;
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    AppMgrServiceInner appMgrServiceInner;
    EXPECT_EQ(appMgrServiceInner.GetSpecifiedProcessFlag(abilityInfo, want), "");
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetSpecifiedProcessFlag_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_GetSpecifiedProcessFlag_0200
 * @tc.desc: Test GetSpecifiedProcessFlag
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_GetSpecifiedProcessFlag_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetSpecifiedProcessFlag_0200 start");
    std::shared_ptr<AAFwk::Want> want = nullptr;
    AppMgrServiceInner appMgrServiceInner;
    EXPECT_EQ(appMgrServiceInner.GetSpecifiedProcessFlag(abilityInfo_, want), "");
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetSpecifiedProcessFlag_0200 start");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_GetSpecifiedProcessFlag_0300
 * @tc.desc: Test GetSpecifiedProcessFlag
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_GetSpecifiedProcessFlag_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetSpecifiedProcessFlag_0300 start");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    want->SetParam(PARAM_SPECIFIED_PROCESS_FLAG, TEST_FLAG);
    AppMgrServiceInner appMgrServiceInner;
    EXPECT_EQ(appMgrServiceInner.GetSpecifiedProcessFlag(abilityInfo, want), "");
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetSpecifiedProcessFlag_0300 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_GetSpecifiedProcessFlag_0400
 * @tc.desc: Test GetSpecifiedProcessFlag
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_GetSpecifiedProcessFlag_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetSpecifiedProcessFlag_0400 start");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type == AppExecFwk::AbilityType::PAGE;
    abilityInfo->isStageBasedModel = true;
    abilityInfo->isolationProcess = true;
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    want->SetParam(PARAM_SPECIFIED_PROCESS_FLAG, TEST_FLAG);
    AppMgrServiceInner appMgrServiceInner;
    EXPECT_EQ(appMgrServiceInner.GetSpecifiedProcessFlag(abilityInfo, want), "");
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetSpecifiedProcessFlag_0400 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_LoadAbilityNoAppRecord_0100
 * @tc.desc: Test GetSpecifiedProcessFlag
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_LoadAbilityNoAppRecord_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_LoadAbilityNoAppRecord_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    const BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.isStageBasedModel = true;
    hapModuleInfo.process = "testMainProcess";
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    auto appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    ASSERT_NE(appRecord, nullptr);
    appRecord->SetEmptyKeepAliveAppState(true);
    appRecord->SetMainProcess(true);

    appMgrServiceInner->LoadAbilityNoAppRecord(appRecord, true, applicationInfo_, abilityInfo_, TEST_PROCESS_NAME,
        TEST_FLAG, bundleInfo, hapModuleInfo, want_, false, false, AppExecFwk::PreloadMode::PRESS_DOWN, token_);
    EXPECT_EQ(appRecord->GetSpecifiedProcessFlag(), TEST_FLAG);
    EXPECT_FALSE(appRecord->IsEmptyKeepAliveApp());
    EXPECT_FALSE(appRecord->IsMainProcess());
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_LoadAbilityNoAppRecord_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_LoadAbilityNoAppRecord_0200
 * @tc.desc: Test GetSpecifiedProcessFlag
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_LoadAbilityNoAppRecord_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_LoadAbilityNoAppRecord_0200 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    const BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.isStageBasedModel = true;
    hapModuleInfo.process = TEST_PROCESS_NAME;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    auto appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    ASSERT_NE(appRecord, nullptr);
    appRecord->SetEmptyKeepAliveAppState(true);

    appMgrServiceInner->LoadAbilityNoAppRecord(appRecord, true, applicationInfo_, abilityInfo_, TEST_PROCESS_NAME,
        "", bundleInfo, hapModuleInfo, want_, false, false, AppExecFwk::PreloadMode::PRESS_DOWN, token_);
    EXPECT_EQ(appRecord->GetSpecifiedProcessFlag(), "");
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_LoadAbilityNoAppRecord_0200 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_LoadAbilityNoAppRecord_0300
 * @tc.desc: Test GetSpecifiedProcessFlag
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_LoadAbilityNoAppRecord_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_LoadAbilityNoAppRecord_0300 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    const BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.isStageBasedModel = true;
    hapModuleInfo.process = TEST_PROCESS_NAME;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    auto appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    ASSERT_NE(appRecord, nullptr);
    appRecord->SetEmptyKeepAliveAppState(true);

    appMgrServiceInner->LoadAbilityNoAppRecord(appRecord, true, applicationInfo_, abilityInfo_, TEST_PROCESS_NAME,
        "", bundleInfo, hapModuleInfo, nullptr, false, false, AppExecFwk::PreloadMode::PRESS_DOWN, token_);
    EXPECT_EQ(appRecord->GetSpecifiedProcessFlag(), "");
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_LoadAbilityNoAppRecord_0300 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_ForceKillApplicationInner_0100
 * @tc.desc: Test GetSpecifiedProcessFlag
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_ForceKillApplicationInner_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_ForceKillApplicationInner_0100 start");
    int32_t appIndex = 0;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto ret = appMgrServiceInner->ForceKillApplicationInner(TEST_BUNDLE_NAME, DEFAULT_INVAL_VALUE, appIndex);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_ForceKillApplicationInner_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_ForceKillApplicationInner_0200
 * @tc.desc: Test GetSpecifiedProcessFlag
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_ForceKillApplicationInner_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_ForceKillApplicationInner_0200 start");
    int32_t appIndex = 0;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    const BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    auto appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    appRecord->GetPriorityObject()->SetPid(1); // kill Init process
    auto ret = appMgrServiceInner->ForceKillApplicationInner(TEST_BUNDLE_NAME, DEFAULT_INVAL_VALUE, appIndex);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_ForceKillApplicationInner_0200 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_ForceKillApplicationInner_0300
 * @tc.desc: Test GetSpecifiedProcessFlag
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_ForceKillApplicationInner_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_ForceKillApplicationInner_0300 start");
    int32_t appIndex = 0;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    const BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    auto appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    appRecord->GetPriorityObject()->SetPid(INT_MAX);

    // kill not exist pid, expect ERR_KILL_PROCESS_NOT_EXIST
    auto ret = appMgrServiceInner->ForceKillApplicationInner(TEST_BUNDLE_NAME, DEFAULT_INVAL_VALUE, appIndex);
    EXPECT_EQ(ret, ERR_KILL_PROCESS_NOT_EXIST);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_ForceKillApplicationInner_0300 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_KillProcessesByAccessTokenId_0100
 * @tc.desc: Test KillProcessesByAccessTokenId
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_KillProcessesByAccessTokenId_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_KillProcessesByAccessTokenId_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    MyFlag::flag_ = 0;
    auto ret = appMgrServiceInner->KillProcessesByAccessTokenId(0);
    EXPECT_EQ(ret, ERR_NOT_SYSTEM_APP);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_KillProcessesByAccessTokenId_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_KillProcessesByAccessTokenId_0200
 * @tc.desc: Test KillProcessesByAccessTokenId
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_KillProcessesByAccessTokenId_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_KillProcessesByAccessTokenId_0200 start");
    MyFlag::flag_ = MyFlag::IS_SA_CALL;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto ret = appMgrServiceInner->KillProcessesByAccessTokenId(0);

    //kill not exist pid
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_KillProcessesByAccessTokenId_0200 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_KillProcessesByAccessTokenId_0300
 * @tc.desc: Test KillProcessesByAccessTokenId
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_KillProcessesByAccessTokenId_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_KillProcessesByAccessTokenId_0300 start");
    MyFlag::flag_ = MyFlag::IS_SA_CALL;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    const BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    const int32_t accessTokenId = 123; // 123 means tokenid
    applicationInfo_->accessTokenId = accessTokenId;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    auto appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    appRecord->SetSpawned();
    appRecord->GetPriorityObject()->SetPid(INT_MAX);
    auto ret = appMgrServiceInner->KillProcessesByAccessTokenId(accessTokenId);

    //kill not exist pid
    EXPECT_EQ(ret, ERR_KILL_PROCESS_NOT_EXIST);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_KillProcessesByAccessTokenId_0300 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_KillProcessesByAccessTokenId_0400
 * @tc.desc: Test KillProcessesByAccessTokenId
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_KillProcessesByAccessTokenId_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_KillProcessesByAccessTokenId_0400 start");
    MyFlag::flag_ = MyFlag::IS_SA_CALL;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    const BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    const int32_t accessTokenId = 123;
    applicationInfo_->accessTokenId = accessTokenId;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    auto appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    appRecord->SetSpawned();
    appRecord->GetPriorityObject()->SetPid(1);
    auto ret = appMgrServiceInner->KillProcessesByAccessTokenId(accessTokenId);

    //kill init process
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_KillProcessesByAccessTokenId_0400 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_GetRunningMultiAppInfoByBundleName_0100
 * @tc.desc: Test KillProcessesByAccessTokenId
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_GetRunningMultiAppInfoByBundleName_0100,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetRunningMultiAppInfoByBundleName_0100 start");
    RunningMultiAppInfo info;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto ret = appMgrServiceInner->GetRunningMultiAppInfoByBundleName("", info);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetRunningMultiAppInfoByBundleName_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_GetRunningMultiAppInfoByBundleName_0200
 * @tc.desc: Test GetRunningMultiAppInfoByBundleName
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_GetRunningMultiAppInfoByBundleName_0200,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetRunningMultiAppInfoByBundleName_0200 start");
    RunningMultiAppInfo info;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto ret = appMgrServiceInner->GetRunningMultiAppInfoByBundleName(TEST_BUNDLE_NAME, info);
    EXPECT_EQ(ret, ERR_BUNDLE_NOT_EXIST);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetRunningMultiAppInfoByBundleName_0200 end");
}

/**
 * @tc.name: GetAllRunningInstanceKeysBySelf_0100
 * @tc.desc: Test GetAllRunningInstanceKeysBySelf
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, GetAllRunningInstanceKeysBySelf_0100,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningInstanceKeysBySelf_0100 start");
    RunningMultiAppInfo info;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::vector<std::string> instanceKeys;
    auto ret = appMgrServiceInner->GetAllRunningInstanceKeysBySelf(instanceKeys);
    EXPECT_NE(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningInstanceKeysBySelf_0100 end");
}

/**
 * @tc.name: GetAllRunningInstanceKeysByBundleName_0100
 * @tc.desc: Test GetAllRunningInstanceKeysByBundleName
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, GetAllRunningInstanceKeysByBundleName_0100,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningInstanceKeysByBundleName_0100 start");
    RunningMultiAppInfo info;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::string bundleName = "testBundleName";
    std::vector<std::string> instanceKeys;
    int32_t userId = 100;
    auto ret = appMgrServiceInner->GetAllRunningInstanceKeysByBundleName(bundleName, instanceKeys, userId);
    EXPECT_NE(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningInstanceKeysByBundleName_0100 end");
}

#ifdef SUPPORT_CHILD_PROCESS
/**
 * @tc.name: AppMgrServiceInnerSecondTest_GetAllChildrenProcesses_0100
 * @tc.desc: Test GetAllChildrenProcesses
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_GetAllChildrenProcesses_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetAllChildrenProcesses_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    auto appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    ChildProcessRequest request;
    auto record1 = std::make_shared<ChildProcessRecord>(IPCSkeleton::GetCallingPid(), request, appRecord);
    appRecord->AddChildProcessRecord(1, record1);
    MyFlag::flag_ = MyFlag::IS_SA_CALL;

    std::vector<ChildProcessInfo> info;
    auto ret = appMgrServiceInner->GetAllChildrenProcesses(info);
    EXPECT_EQ(ret, ERR_OK);
    if (!info.empty()) {
        EXPECT_EQ(info.size(), 1);
        EXPECT_EQ(info[0].bundleName, TEST_BUNDLE_NAME);
    }
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetAllChildrenProcesses_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_GetAllChildrenProcesses_0200
 * @tc.desc: Test GetAllChildrenProcesses
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_GetAllChildrenProcesses_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetAllChildrenProcesses_0200 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    const int32_t accessTokenId = 123; // 123 means tokenid
    applicationInfo_->accessTokenId = accessTokenId;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    auto appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    ChildProcessRequest request;
    auto record1 = std::make_shared<ChildProcessRecord>(IPCSkeleton::GetCallingPid(), request, appRecord);
    appRecord->AddChildProcessRecord(1, record1);
    MyFlag::flag_ = 0;

    IPCSkeleton::SetCallingTokenID(accessTokenId);
    std::vector<ChildProcessInfo> info;
    auto ret = appMgrServiceInner->GetAllChildrenProcesses(info);
    EXPECT_EQ(ret, ERR_OK);
    if (!info.empty()) {
        EXPECT_EQ(info.size(), 1);
        EXPECT_EQ(info[0].bundleName, TEST_BUNDLE_NAME);
    }
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetAllChildrenProcesses_0200 end");
}
#endif // SUPPORT_CHILD_PROCESS

/**
 * @tc.name: AppMgrServiceInnerSecondTest_NotifyMemoryLevel_0100
 * @tc.desc: Test NotifyMemoryLevel
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_NotifyMemoryLevel_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_NotifyMemoryLevel_0100 start");
    MyFlag::flag_ = 0;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto ret = appMgrServiceInner->NotifyMemoryLevel(1);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_NotifyMemoryLevel_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_NotifyMemoryLevel_0200
 * @tc.desc: Test NotifyMemoryLevel
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_NotifyMemoryLevel_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_NotifyMemoryLevel_0100 start");
    MyFlag::flag_ = MyFlag::IS_SA_CALL;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto ret = appMgrServiceInner->NotifyMemoryLevel(-1); // -1 means invalid level
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_NotifyMemoryLevel_0200 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_NotifyMemoryLevel_0300
 * @tc.desc: Test NotifyMemoryLevel
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_NotifyMemoryLevel_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_NotifyMemoryLevel_0300 start");
    MyFlag::flag_ = MyFlag::IS_SA_CALL;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto ret = appMgrServiceInner->NotifyMemoryLevel(0);
    EXPECT_EQ(ret, ERR_OK);
    ret = appMgrServiceInner->NotifyMemoryLevel(1);
    EXPECT_EQ(ret, ERR_OK);
    ret = appMgrServiceInner->NotifyMemoryLevel(2);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_NotifyMemoryLevel_0300 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_NotifyMemoryLevel_0400
 * @tc.desc: Test NotifyMemoryLevel
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_NotifyMemoryLevel_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_NotifyMemoryLevel_0400 start");
    MyFlag::flag_ = MyFlag::IS_SA_CALL;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto ret = appMgrServiceInner->NotifyMemoryLevel(3);
    EXPECT_EQ(ret, ERR_OK);
    ret = appMgrServiceInner->NotifyMemoryLevel(4);
    EXPECT_EQ(ret, ERR_OK);
    ret = appMgrServiceInner->NotifyMemoryLevel(5);
    EXPECT_EQ(ret, ERR_OK);
    ret = appMgrServiceInner->NotifyMemoryLevel(6);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_NotifyMemoryLevel_0400 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_NotifyMemoryLevel_0500
 * @tc.desc: Test NotifyMemoryLevel
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_NotifyMemoryLevel_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_NotifyMemoryLevel_0500 start");
    MyFlag::flag_ = MyFlag::IS_SA_CALL;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto ret = appMgrServiceInner->NotifyMemoryLevel(7); // 7 means invalid level
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    appMgrServiceInner->appRunningManager_ = nullptr;
    ret = appMgrServiceInner->NotifyMemoryLevel(3);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_NotifyMemoryLevel_0500 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_NotifyAppFault_0100
 * @tc.desc: Test NotifyAppFault
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_NotifyAppFault_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_NotifyAppFault_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    auto appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    appRecord->GetPriorityObject()->SetPid(1);
    appRecord->SetState(ApplicationState::APP_STATE_TERMINATED);

    FaultData faultData;
    auto ret = appMgrServiceInner->NotifyAppFault(faultData);
    EXPECT_EQ(ret, ERR_OK);

    appRecord->SetState(ApplicationState::APP_STATE_END);
    ret = appMgrServiceInner->NotifyAppFault(faultData);
    EXPECT_EQ(ret, ERR_OK);

    appRecord->SetState(ApplicationState::APP_STATE_FOREGROUND);
    // expect in appfreezeManager return OK
    AppfreezeManager::GetInstance()->CancelAppFreezeDetect(1, TEST_BUNDLE_NAME);
    ret = appMgrServiceInner->NotifyAppFault(faultData);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_NotifyAppFault_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_BuildEventInfo_0100
 * @tc.desc: Test BuildEventInfo
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_BuildEventInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_BuildEventInfo_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    auto info = appMgrServiceInner->BuildEventInfo(appRecord);
    EXPECT_EQ(info.bundleName, "");
    EXPECT_EQ(info.versionName, "");

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);

    AAFwk::EventInfo eventInfo {.bundleName = applicationInfo_->name,
        .versionName = applicationInfo_->versionName,
        .versionCode = applicationInfo_->versionCode,
        .bundleType = static_cast<int32_t>(applicationInfo_->bundleType),
        .processName = appRecord->GetProcessName()
    };
    info = appMgrServiceInner->BuildEventInfo(appRecord);
    EXPECT_EQ(info.bundleName, eventInfo.bundleName);
    EXPECT_EQ(info.versionName, eventInfo.versionName);
    EXPECT_EQ(info.versionCode, eventInfo.versionCode);
    EXPECT_EQ(info.bundleType, eventInfo.bundleType);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_BuildEventInfo_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_UpdateRenderState_0100
 * @tc.desc: Test UpdateRenderState
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_UpdateRenderState_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_UpdateRenderState_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto ret = appMgrServiceInner->UpdateRenderState(1, 0);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    auto appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    appRecord->GetPriorityObject()->SetPid(1);
    ret = appMgrServiceInner->UpdateRenderState(INT_MAX, 0);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    auto renderRecord = std::make_shared<RenderRecord>(1, "", FdGuard(-1), FdGuard(-1), FdGuard(-1), appRecord);
    renderRecord->SetPid(100); // 100 means pid
    appRecord->AddRenderRecord(renderRecord);
    ret = appMgrServiceInner->UpdateRenderState(100, 1); // 100 means pid
    EXPECT_EQ(renderRecord->GetState(), 1);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_UpdateRenderState_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_NotifyMemMgrPriorityChanged_0100
 * @tc.desc: Test NotifyMemMgrPriorityChanged
 * @tc.type: FUNC
 */
/**
 * @tc.name: AppMgrServiceInnerSecondTest_NotifyMemMgrPriorityChanged_0100
 * @tc.desc: Test NotifyMemMgrPriorityChanged
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_NotifyMemMgrPriorityChanged_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_NotifyMemMgrPriorityChanged_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto ret = appMgrServiceInner->NotifyMemMgrPriorityChanged(nullptr);
    EXPECT_FALSE(ret);

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);

    ret = appMgrServiceInner->NotifyMemMgrPriorityChanged(appRecord);
    EXPECT_FALSE(ret); // stub err
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_NotifyMemMgrPriorityChanged_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_MakeKiaProcess_0100
 * @tc.desc: Test BuildEventInfo
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_MakeKiaProcess_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_MakeKiaProcess_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::shared_ptr<AAFwk::Want> want = nullptr;
    bool isKia = false;
    std::string watermarkBusinessName = "123";
    bool isWatermarkEnabled = false;
    bool isFileUri = false;
    std::string processName = "MakeKiaProcess";
    AAFwk::AppUtils::GetInstance().isStartOptionsWithAnimation_.isLoaded = true;
    AAFwk::AppUtils::GetInstance().isStartOptionsWithAnimation_.value = false;
    int32_t res = appMgrServiceInner->MakeKiaProcess(want, isKia, watermarkBusinessName,
        isWatermarkEnabled, isFileUri, processName);
    EXPECT_EQ(res, ERR_OK);
    AAFwk::AppUtils::GetInstance().isStartOptionsWithAnimation_.isLoaded = true;
    AAFwk::AppUtils::GetInstance().isStartOptionsWithAnimation_.value = true;
    res = appMgrServiceInner->MakeKiaProcess(want, isKia, watermarkBusinessName,
        isWatermarkEnabled, isFileUri, processName);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
    want = std::make_shared<AAFwk::Want>();
    isFileUri = true;
    Uri uri("file://docs/DestTop/Text/test_001.txt");
    want->SetUri(uri);
    appMgrServiceInner->kiaInterceptor_ = new MockKiaInterceptor();
    EXPECT_NE(appMgrServiceInner->kiaInterceptor_, nullptr);
    want->SetParam(KEY_WATERMARK_BUSINESS_NAME, std::string("com.ohos.param.watermarkBusinessName"));
    want->SetParam(KEY_IS_WATERMARK_ENABLED, true);
    res = appMgrServiceInner->MakeKiaProcess(want, isKia, watermarkBusinessName,
        isWatermarkEnabled, isFileUri, processName);
    EXPECT_EQ(res, ERR_OK);
    want->SetParam(KEY_IS_WATERMARK_ENABLED, false);
    res = appMgrServiceInner->MakeKiaProcess(want, isKia, watermarkBusinessName,
        isWatermarkEnabled, isFileUri, processName);
    EXPECT_EQ(res, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_MakeKiaProcess_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_MakeKiaProcess_0200
 * @tc.desc: Test BuildEventInfo
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_MakeKiaProcess_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_MakeKiaProcess_0200 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    bool isKia = false;
    std::string watermarkBusinessName = "123";
    bool isWatermarkEnabled = false;
    bool isFileUri = true;
    std::string processName = "MakeKiaProcess";
    Uri uri("file");
    want->SetUri(uri);
    appMgrServiceInner->kiaInterceptor_ = new MockKiaInterceptor();
    EXPECT_NE(appMgrServiceInner->kiaInterceptor_, nullptr);
    want->SetParam(KEY_WATERMARK_BUSINESS_NAME, std::string("com.ohos.param.watermarkBusinessName"));
    want->SetParam(KEY_IS_WATERMARK_ENABLED, true);
    int32_t res = appMgrServiceInner->MakeKiaProcess(want, isKia, watermarkBusinessName,
                                                    isWatermarkEnabled, isFileUri, processName);
    EXPECT_EQ(res, ERR_OK);
    Uri urk("file://docs/DestTop/Text/test_001.txt");
    want->SetUri(urk);
    appMgrServiceInner->kiaInterceptor_ = nullptr;
    res = appMgrServiceInner->MakeKiaProcess(want, isKia, watermarkBusinessName,
                                            isWatermarkEnabled, isFileUri, processName);
    EXPECT_EQ(res, ERR_OK);
    Uri urj("file");
    want->SetUri(urj);
    appMgrServiceInner->kiaInterceptor_ = nullptr;
    res = appMgrServiceInner->MakeKiaProcess(want, isKia, watermarkBusinessName,
                                            isWatermarkEnabled, isFileUri, processName);
    EXPECT_EQ(res, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_MakeKiaProcess_0200 end");
}

/**
 * @tc.name: MakeProcessName_001
 * @tc.desc: MakeProcessName.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_MakeProcessName_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_MakeProcessName_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    EXPECT_NE(abilityInfo_, nullptr);
    EXPECT_NE(applicationInfo_, nullptr);
    HapModuleInfo hapModuleInfo;
    int32_t appIndex = 0;
    std::string specifiedProcessFlag = "akeProcessName";
    std::string processName = "akeProcessName";
    appMgrServiceInner->MakeProcessName(abilityInfo_, applicationInfo_, hapModuleInfo,
                                        appIndex, specifiedProcessFlag, processName, false);
    abilityInfo_->process = "akeProcessName";
    appMgrServiceInner->MakeProcessName(abilityInfo_, applicationInfo_, hapModuleInfo,
                                        appIndex, specifiedProcessFlag, processName, false);
    abilityInfo_->process = "";
    appIndex = 1;
    appMgrServiceInner->MakeProcessName(abilityInfo_, applicationInfo_, hapModuleInfo,
                                        appIndex, specifiedProcessFlag, processName, false);
    abilityInfo_ = nullptr;
    appMgrServiceInner->MakeProcessName(abilityInfo_, applicationInfo_, hapModuleInfo,
                                        appIndex, specifiedProcessFlag, processName, false);
    applicationInfo_ = nullptr;
    appMgrServiceInner->MakeProcessName(abilityInfo_, applicationInfo_, hapModuleInfo,
                                        appIndex, specifiedProcessFlag, processName, false);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_MakeProcessName_0100 end");
}

/**
 * @tc.name: MakeProcessName_001
 * @tc.desc: MakeProcessName.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_MakeProcessName_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_MakeProcessName_0200 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    EXPECT_NE(abilityInfo_, nullptr);
    EXPECT_NE(applicationInfo_, nullptr);
    HapModuleInfo hapModuleInfo;
    std::string processName = "akeProcessName";
    std::shared_ptr<ApplicationInfo> applicationInfo = nullptr;
    appMgrServiceInner->MakeProcessName(applicationInfo, hapModuleInfo, processName);
    hapModuleInfo.isStageBasedModel = true;
    hapModuleInfo.process = "akeProcessName";
    appMgrServiceInner->MakeProcessName(applicationInfo_, hapModuleInfo, processName);
    hapModuleInfo.isStageBasedModel = true;
    hapModuleInfo.process = "";
    appMgrServiceInner->MakeProcessName(applicationInfo_, hapModuleInfo, processName);
    hapModuleInfo.isStageBasedModel = false;
    applicationInfo_->process = "";
    appMgrServiceInner->MakeProcessName(applicationInfo_, hapModuleInfo, processName);
    applicationInfo_->process = "akeProcessName";
    appMgrServiceInner->MakeProcessName(applicationInfo_, hapModuleInfo, processName);
    hapModuleInfo.isStageBasedModel = true;
    hapModuleInfo.process = applicationInfo_->bundleName;
    appMgrServiceInner->MakeProcessName(applicationInfo_, hapModuleInfo, processName);
    hapModuleInfo.isStageBasedModel = true;
    hapModuleInfo.process = "akeProcessName";
    appMgrServiceInner->MakeProcessName(applicationInfo_, hapModuleInfo, processName);
    hapModuleInfo.isStageBasedModel = true;
    hapModuleInfo.process = "";
    hapModuleInfo.process = applicationInfo_->bundleName;
    hapModuleInfo.isolationMode = IsolationMode::ISOLATION_FIRST;
    appMgrServiceInner->supportIsolationMode_  = "true";
    appMgrServiceInner->MakeProcessName(applicationInfo_, hapModuleInfo, processName);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_MakeProcessName_0200 end");
}

/**
 * @tc.name: CheckIsolationMode_0100
 * @tc.desc: CheckIsolationMode.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_CheckIsolationMode_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_CheckIsolationMode_0200 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    EXPECT_NE(abilityInfo_, nullptr);
    EXPECT_NE(applicationInfo_, nullptr);
    HapModuleInfo hapModuleInfo;
    appMgrServiceInner->supportIsolationMode_ = "true";
    hapModuleInfo.isolationMode = IsolationMode::ISOLATION_FIRST;
    bool res = appMgrServiceInner->CheckIsolationMode(hapModuleInfo);
    EXPECT_EQ(res, true);
    hapModuleInfo.isolationMode = IsolationMode::ISOLATION_ONLY;
    res = appMgrServiceInner->CheckIsolationMode(hapModuleInfo);
    hapModuleInfo.isolationMode = IsolationMode::NONISOLATION_FIRST;
    EXPECT_EQ(res, true);
    res = appMgrServiceInner->CheckIsolationMode(hapModuleInfo);
    EXPECT_EQ(res, false);
    appMgrServiceInner->supportIsolationMode_  = "false";
    res = appMgrServiceInner->CheckIsolationMode(hapModuleInfo);
    EXPECT_EQ(res, false);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_CheckIsolationMode_0100 end");
}

/**
 * @tc.name: LaunchApplication_001
 * @tc.desc: launch application.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_LaunchApplication_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_LaunchApplication_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    appMgrServiceInner->LaunchApplication(nullptr);
    BundleInfo info;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, info, "");
    g_recordId += 1;
    appMgrServiceInner->LaunchApplication(appRecord);
    int32_t uid = USER_SCALE * 101;
    appRecord->SetUid(uid);
    appRecord->SetState(ApplicationState::APP_STATE_CREATE);
    appMgrServiceInner->LaunchApplication(appRecord);
    appRecord->SetState(ApplicationState::APP_STATE_FOREGROUND);
    appMgrServiceInner->LaunchApplication(appRecord);
    appRecord->SetEmptyKeepAliveAppState(true);
    appRecord->isEmptyKeepAliveApp_ = true;
    appRecord->SetState(ApplicationState::APP_STATE_CREATE);
    appMgrServiceInner->LaunchApplication(appRecord);
    appRecord->SetState(ApplicationState::APP_STATE_FOREGROUND);
    appRecord->SetEmptyKeepAliveAppState(true);
    appRecord->SetKeepAliveEnableState(false);
    appRecord->SetKeepAliveDkv(false);
    appMgrServiceInner->LaunchApplication(appRecord);
    appRecord->SetKeepAliveEnableState(true);
    appRecord->SetEmptyKeepAliveAppState(false);
    appMgrServiceInner->LaunchApplication(appRecord);
    appRecord->SetKeepAliveEnableState(true);
    appRecord->SetEmptyKeepAliveAppState(true);
    appMgrServiceInner->LaunchApplication(appRecord);
    appRecord->SetKeepAliveEnableState(false);
    appRecord->SetEmptyKeepAliveAppState(false);
    appMgrServiceInner->LaunchApplication(appRecord);
    Want want;
    appRecord->ResetSpecifiedRequest();
    appMgrServiceInner->LaunchApplication(appRecord);
    appRecord->SetSpecifiedAbilityFlagAndWant(1, want, "");
    appMgrServiceInner->LaunchApplication(appRecord);
    appRecord->SetState(ApplicationState::APP_STATE_CREATE);
    appMgrServiceInner->LaunchApplication(appRecord);
    appMgrServiceInner->LaunchApplication(appRecord);
    appRecord->appInfo_ = nullptr;
    appMgrServiceInner->LaunchApplication(appRecord);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_LaunchApplication_0100 end");
}

/**
 * @tc.name: KillApplication_001
 * @tc.desc: kill application.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_KillApplication_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_KillApplication_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::string bundleName = "test_bundleName";
    std::string processName = "test_processName";
    bool clearPageStack = true;
    appMgrServiceInner->appRunningManager_ = nullptr;
    int32_t res = appMgrServiceInner->KillApplication(bundleName, clearPageStack);
    EXPECT_EQ(res, ERR_NO_INIT);
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    res = appMgrServiceInner->KillApplication(bundleName, clearPageStack);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_KillApplication_0100 end");
}

/**
 * @tc.name: ApplicationForegrounded_001
 * @tc.desc: application foregrounded.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_ApplicationForegrounded_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_ApplicationForegrounded_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    appMgrServiceInner->ApplicationForegrounded(99);

    std::string processName = "test_processName";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo_, ++g_recordId, processName);
    EXPECT_NE(appRecord, nullptr);
    appMgrServiceInner->appRunningManager_->appRunningRecordMap_.emplace(g_recordId, appRecord);
    appMgrServiceInner->ApplicationForegrounded(g_recordId);
    appRecord->SetApplicationScheduleState(ApplicationScheduleState::SCHEDULE_FOREGROUNDING);
    appRecord->SetState(ApplicationState::APP_STATE_BACKGROUND);
    appMgrServiceInner->ApplicationForegrounded(g_recordId);

    appRecord->SetApplicationScheduleState(ApplicationScheduleState::SCHEDULE_FOREGROUNDING);
    appRecord->SetState(ApplicationState::APP_STATE_READY);
    appMgrServiceInner->ApplicationForegrounded(g_recordId);

    appRecord->SetApplicationScheduleState(ApplicationScheduleState::SCHEDULE_FOREGROUNDING);
    appRecord->SetState(ApplicationState::APP_STATE_TERMINATED);
    appMgrServiceInner->ApplicationForegrounded(g_recordId);

    appRecord->SetApplicationScheduleState(ApplicationScheduleState::SCHEDULE_FOREGROUNDING);
    appRecord->SetApplicationPendingState(ApplicationPendingState::BACKGROUNDING);
    appMgrServiceInner->ApplicationForegrounded(g_recordId);

    appRecord->SetApplicationScheduleState(ApplicationScheduleState::SCHEDULE_FOREGROUNDING);
    appRecord->SetApplicationPendingState(ApplicationPendingState::FOREGROUNDING);
    int32_t callerPid = 1;
    appRecord->SetCallerPid(callerPid);
    appMgrServiceInner->appRunningManager_->appRunningRecordMap_.emplace(callerPid, appRecord);
    appRecord->GetPriorityObject()->SetPid(1);
    appMgrServiceInner->ApplicationForegrounded(g_recordId);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_ApplicationForegrounded_001 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_KillApplicationByUid_0100
 * @tc.desc: Test KillApplicationByUid
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, KillApplicationByUid_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_KillApplicationByUid_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    int uid = 1;
    appMgrServiceInner->appRunningManager_ = nullptr;
    auto ret = appMgrServiceInner->KillApplicationByUid(TEST_BUNDLE_NAME, uid);
    EXPECT_EQ(ret, ERR_NO_INIT); //appRunningManager_ null

    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    MyFlag::flag_ = 0;
    ret = appMgrServiceInner->KillApplicationByUid(TEST_BUNDLE_NAME, uid);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED); //permission verification fail

    MyFlag::flag_ = MyFlag::IS_SA_CALL;
    appMgrServiceInner->remoteClientManager_ = nullptr;
    ret = appMgrServiceInner->KillApplicationByUid(TEST_BUNDLE_NAME, uid);
    EXPECT_EQ(ret, 0); //remoteClientManager_ null

    appMgrServiceInner->remoteClientManager_ = std::make_shared<RemoteClientManager>();
    ret = appMgrServiceInner->KillApplicationByUid(TEST_BUNDLE_NAME, uid);
    EXPECT_EQ(ret, ERR_OK); //unstart

    AppRunningManager appRunningManager;
    applicationInfo_->uid = uid;
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    auto appRecord = appMgrServiceInner->CreateAppRunningRecord(
        loadParam, applicationInfo_, abilityInfo_, TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    appRecord->GetPriorityObject()->SetPid(INT_MAX);
    appRecord->appInfos_.emplace("test", applicationInfo_);
    appRunningManager.appRunningRecordMap_.emplace(1, appRecord);
    ret = appMgrServiceInner->KillApplicationByUid(TEST_BUNDLE_NAME, uid);
    EXPECT_EQ(ret, ERR_OK); //remote process exited successs

    appRecord->GetPriorityObject()->SetPid(1);
    ret = appMgrServiceInner->KillApplicationByUid(TEST_BUNDLE_NAME, uid);
    EXPECT_EQ(ret, ERR_OK); //KillProcessByPid
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_KillApplicationByUid_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_SendProcessExitEventTask_0100
 * @tc.desc: Test SendProcessExitEventTask
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, SendProcessExitEventTask_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_SendProcessExitEventTask_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    time_t exitTime = 0;
    int32_t count = 2;
    auto pid = 1;
    std::string processName = "com.example";
    auto extensionType = 1;
    auto exitReason = 1;
    appMgrServiceInner->SendProcessExitEventTask(pid, processName, extensionType, exitReason, exitTime, count);
    EXPECT_EQ(appRecord, nullptr); //appRecord null

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_, TEST_PROCESS_NAME,
        bundleInfo, hapModuleInfo, want_, false);
    appRecord->priorityObject_ = nullptr;
    pid = appRecord->GetPid();
    processName = appRecord->GetProcessName();
    extensionType = static_cast<int32_t>(appRecord->GetExtensionType());
    exitReason = appRecord->GetExitReason();
    appMgrServiceInner->SendProcessExitEventTask(pid, processName, extensionType, exitReason, exitTime, count);
    EXPECT_NE(appRecord, nullptr);
    EXPECT_EQ(appRecord->priorityObject_, nullptr); //priorityObject null
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_SendProcessExitEventTask_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_SendProcessExitEventTask_0200
 * @tc.desc: Test SendProcessExitEventTask
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, SendProcessExitEventTask_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_SendProcessExitEventTask_0200 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    time_t exitTime = 0;
    int32_t count = 2;
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_, TEST_PROCESS_NAME,
        bundleInfo, hapModuleInfo, want_, false);
    appRecord->GetPriorityObject()->SetPid(INT_MAX);
    auto pid = appRecord->GetPid();
    auto processName = appRecord->GetProcessName();
    auto extensionType = static_cast<int32_t>(appRecord->GetExtensionType());
    auto exitReason = appRecord->GetExitReason();
    appMgrServiceInner->SendProcessExitEventTask(pid, processName, extensionType, exitReason, exitTime, count);
    EXPECT_NE(appRecord->priorityObject_, nullptr); //exitResult = true
    EXPECT_FALSE(--count <= 0);
    count = 1;
    appRecord->GetPriorityObject()->SetPid(1);
    appMgrServiceInner->SendProcessExitEventTask(pid, processName, extensionType, exitReason, exitTime, count);
    EXPECT_TRUE(--count <= 0); //--count <= 0
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_SendProcessExitEventTask_0200 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_ClearUpApplicationData_0100
 * @tc.desc: Test ClearUpApplicationData
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, ClearUpApplicationData_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_ClearUpApplicationData_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::string bundleName = TEST_BUNDLE_NAME;
    int32_t callerUid = 1;
    pid_t callerPid = 1;
    int32_t appCloneIndex = 0;
    int32_t userId = DEFAULT_INVAL_VALUE;
    auto ret = appMgrServiceInner->ClearUpApplicationData(bundleName, callerUid, callerPid, appCloneIndex, userId);
    EXPECT_EQ(ret, ERR_APP_CLONE_INDEX_INVALID);
    callerUid = -1;
    ret = appMgrServiceInner->ClearUpApplicationData(bundleName, callerUid, callerPid, appCloneIndex, userId);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    userId = 1;
    ret = appMgrServiceInner->ClearUpApplicationData(bundleName, callerUid, callerPid, appCloneIndex, userId);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_ClearUpApplicationData_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_ClearUpApplicationDataByUserId_0100
 * @tc.desc: Test ClearUpApplicationDataByUserId
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, ClearUpApplicationDataByUserId_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_ClearUpApplicationDataByUserId_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::string bundleName = TEST_BUNDLE_NAME;
    int32_t callerUid = 1;
    pid_t callerPid = -1;
    int32_t appCloneIndex = 0;
    int32_t userId = 1;
    bool isBySelf = false;
    auto res = appMgrServiceInner->ClearUpApplicationDataByUserId(bundleName, callerUid, callerPid, appCloneIndex,
                                                                  userId, isBySelf);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
    callerPid = 1;
    callerUid = -1;
    res = appMgrServiceInner->ClearUpApplicationDataByUserId(bundleName, callerUid, callerPid, appCloneIndex, userId,
                                                             isBySelf);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
    callerUid = 1;
    res = appMgrServiceInner->ClearUpApplicationDataByUserId(bundleName, callerUid, callerPid, appCloneIndex, userId,
                                                             isBySelf);
    EXPECT_EQ(res, ERR_APP_CLONE_INDEX_INVALID); //clearUserGrantedPermissionState fail
    Security::AccessToken::HapInfoParams info;
    info.userID = userId;
    info.bundleName = bundleName;
    info.instIndex = 0;
    info.dlpType = 0;
    info.appIDDesc = "test_bundle";
    info.apiVersion = 1;
    Security::AccessToken::HapPolicyParams policy;
    policy.domain = "com.ohos.test";
    policy.apl = Security::AccessToken::TypeATokenAplEnum::APL_NORMAL;
    Security::AccessToken::AccessTokenIDEx fullTokenId;
    fullTokenId.tokenIdExStruct.tokenID = 1;
    Security::AccessToken::AccessTokenKit::InitHapToken(info, policy, fullTokenId);
    res = appMgrServiceInner->ClearUpApplicationDataByUserId(bundleName, callerUid, callerPid, appCloneIndex, userId,
                                                             isBySelf);
    EXPECT_EQ(res, ERR_APP_CLONE_INDEX_INVALID); //delete user data fail
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_ClearUpApplicationDataByUserId_0100 end");
}

/**
 * @tc.name: GetAllRunningInstanceKeysByBundleName_1000
 * @tc.desc: Test GetAllRunningInstanceKeysByBundleName
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, GetAllRunningInstanceKeysByBundleName_1000, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetAllRunningInstanceKeysByBundleName_1000 start");
    std::string bundleName = "";
    std::vector<std::string> instanceKeys;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    int32_t userId = 100;
    auto res = appMgrServiceInner->GetAllRunningInstanceKeysByBundleName(bundleName, instanceKeys, userId);
    EXPECT_EQ(res, AAFwk::INVALID_PARAMETERS_ERR);
    bundleName = TEST_BUNDLE_NAME;
    appMgrServiceInner->remoteClientManager_ = nullptr;
    res = appMgrServiceInner->GetAllRunningInstanceKeysByBundleName(bundleName, instanceKeys, userId);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
    appMgrServiceInner->remoteClientManager_ = std::make_shared<RemoteClientManager>();
    res = appMgrServiceInner->GetAllRunningInstanceKeysByBundleName(bundleName, instanceKeys, userId);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetAllRunningInstanceKeysByBundleName_1000 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_CheckAppRecordAndPriorityObject_0100
 * @tc.desc: Test CheckAppRecordAndPriorityObject
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, CheckAppRecordAndPriorityObject_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_CheckAppRecordAndPriorityObject_0100 start");
    std::string bundleName = "";
    std::vector<std::string> instanceKeys;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    auto res = appMgrServiceInner->CheckAppRecordAndPriorityObject(appRecord);
    EXPECT_EQ(appRecord, nullptr);
    EXPECT_EQ(res, false);
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_, TEST_PROCESS_NAME,
        bundleInfo, hapModuleInfo, want_, false);
    appRecord->priorityObject_ = nullptr;
    res = appMgrServiceInner->CheckAppRecordAndPriorityObject(appRecord);
    EXPECT_NE(appRecord, nullptr);
    EXPECT_EQ(res, false);
    appRecord->priorityObject_ = std::make_shared<PriorityObject>();
    res = appMgrServiceInner->CheckAppRecordAndPriorityObject(appRecord);
    EXPECT_EQ(res, true);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_CheckAppRecordAndPriorityObject_0100 end");
}

#ifdef SUPPORT_CHILD_PROCESS
/**
 * @tc.name: AppMgrServiceInnerSecondTest_GetAppCloneInfo_0100
 * @tc.desc: Test GetAppCloneInfo
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, GetAppCloneInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetAppCloneInfo_0100 start");
    std::string bundleName = "";
    std::vector<std::string> instanceKeys;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    RunningMultiAppInfo info;
    appMgrServiceInner->GetAppCloneInfo(appRecord, info);
    EXPECT_EQ(appMgrServiceInner->CheckAppRecordAndPriorityObject(appRecord), false);
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_, TEST_PROCESS_NAME,
        bundleInfo, hapModuleInfo, want_, false);
    appMgrServiceInner->GetAppCloneInfo(appRecord, info);
    EXPECT_EQ(appMgrServiceInner->CheckAppRecordAndPriorityObject(appRecord), true);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetAppCloneInfo_0100 end");
}
#endif // SUPPORT_CHILD_PROCESS

/**
 * @tc.name: AppMgrServiceInnerSecondTest_CheckAppFault_0100
 * @tc.desc: Test CheckAppFault
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_CheckAppFault_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_CheckAppFault_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    FaultData faultData;

    bool ret = appMgrServiceInner->CheckAppFault(appRecord, faultData);
    EXPECT_FALSE(ret);

    faultData.timeoutMarkers = "timeout";
    appMgrServiceInner->dfxTaskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler("dfx_freeze_task_queue_test");
    ret = appMgrServiceInner->CheckAppFault(appRecord, faultData);
    EXPECT_TRUE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_CheckAppFault_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_CheckAppFault_0200
 * @tc.desc: Test CheckAppFault
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_CheckAppFault_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_CheckAppFault_0200 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    FaultData faultData;

    appRecord->isDebugApp_ = true;
    bool ret = appMgrServiceInner->CheckAppFault(appRecord, faultData);
    EXPECT_TRUE(ret);

    appRecord->isDebugApp_ = false;
    appRecord->isAssertPause_ = true;
    ret = appMgrServiceInner->CheckAppFault(appRecord, faultData);
    EXPECT_TRUE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_CheckAppFault_0200 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_TransformedNotifyAppFault_0100
 * @tc.desc: Test KillFaultApp
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_TransformedNotifyAppFault_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_TransformedNotifyAppFault_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AppFaultDataBySA faultData;

    int32_t ret = appMgrServiceInner->TransformedNotifyAppFault(faultData);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    appRecord->GetPriorityObject()->SetPid(TEST_PID_100);
    faultData.pid = TEST_PID_100;
    ret = appMgrServiceInner->TransformedNotifyAppFault(faultData);
    EXPECT_EQ(ret, ERR_OK);

    faultData.timeoutMarkers = "timeoutMarkers";
    ret = appMgrServiceInner->TransformedNotifyAppFault(faultData);
    EXPECT_EQ(ret, ERR_OK);

    faultData.errorObject.name = "appRecovery";
    ret = appMgrServiceInner->TransformedNotifyAppFault(faultData);
    EXPECT_EQ(ret, ERR_OK);

    appRecord->GetPriorityObject()->SetPid(1);
    faultData.pid = 1;
    ret = appMgrServiceInner->TransformedNotifyAppFault(faultData);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_TransformedNotifyAppFault_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_TransformedNotifyAppFault_0200
 * @tc.desc: Test KillFaultApp
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_TransformedNotifyAppFault_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_TransformedNotifyAppFault_0200 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AppFaultDataBySA faultData;
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);

    appRecord->GetPriorityObject()->SetPid(TEST_PID_100);
    faultData.pid = TEST_PID_100;
    faultData.faultType = FaultDataType::APP_FREEZE;
    appMgrServiceInner->dfxTaskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler("dfx_freeze_task_queue_test");
    int32_t ret = appMgrServiceInner->TransformedNotifyAppFault(faultData);
    EXPECT_EQ(ret, ERR_OK);

    appRecord->isDebugApp_ = true;
    ret = appMgrServiceInner->TransformedNotifyAppFault(faultData);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_TransformedNotifyAppFault_0200 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_GetBundleNameByPid_0100
 * @tc.desc: Test GetBundleNameByPid
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_GetBundleNameByPid_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetBundleNameByPid_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::string bundleName = "testbundleName";
    int32_t uid = 101;

    int32_t ret = appMgrServiceInner->GetBundleNameByPid(TEST_PID_100, bundleName, uid);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);

    AppFaultDataBySA faultData;
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    appRecord->GetPriorityObject()->SetPid(TEST_PID_100);
    ret = appMgrServiceInner->GetBundleNameByPid(TEST_PID_100, bundleName, uid);
    EXPECT_EQ(ret, ERR_OK);

    MyFlag::flag_ = 0;
    ret = appMgrServiceInner->GetBundleNameByPid(TEST_PID_100, bundleName, uid);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetBundleNameByPid_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_NotifyAbilitiesAssertDebugChange_0100
 * @tc.desc: Test NotifyAbilitiesAssertDebugChange
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_NotifyAbilitiesAssertDebugChange_0100,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_NotifyAbilitiesAssertDebugChange_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    bool isAssertDebug = true;

    int32_t ret = appMgrServiceInner->NotifyAbilitiesAssertDebugChange(appRecord, isAssertDebug);
    EXPECT_EQ(ret, ERR_NO_INIT);

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    ret = appMgrServiceInner->NotifyAbilitiesAssertDebugChange(appRecord, isAssertDebug);
    EXPECT_EQ(ret, ERR_NO_INIT);

    auto mockStub = new (std::nothrow) MockAbilityDebugResponseStub();
    appMgrServiceInner->abilityDebugResponse_ = new AbilityDebugResponseProxy(mockStub);
    ret = appMgrServiceInner->NotifyAbilitiesAssertDebugChange(appRecord, isAssertDebug);
    EXPECT_EQ(ret, ERR_OK);

    appRecord->hapModules_.clear();
    ret = appMgrServiceInner->NotifyAbilitiesAssertDebugChange(appRecord, isAssertDebug);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_NotifyAbilitiesAssertDebugChange_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_ApplicationTerminatedSendProcessEvent_0100
 * @tc.desc: Test ApplicationTerminatedSendProcessEvent
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_ApplicationTerminatedSendProcessEvent_0100,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_ApplicationTerminatedSendProcessEvent_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    appMgrServiceInner->ApplicationTerminatedSendProcessEvent(appRecord);

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    std::vector<BaseSharedBundleInfo> baseSharedBundleInfoList;

    appMgrServiceInner->SetRunningSharedBundleList(TEST_BUNDLE_NAME, baseSharedBundleInfoList);
    appMgrServiceInner->ApplicationTerminatedSendProcessEvent(appRecord);
    EXPECT_EQ(appMgrServiceInner->runningSharedBundleList_.size(), 0);

    MyFlag::flag_ = MyFlag::IS_SA_CALL;
    appMgrServiceInner->appDebugManager_ = nullptr;
    appMgrServiceInner->SetRunningSharedBundleList(TEST_BUNDLE_NAME, baseSharedBundleInfoList);
    appMgrServiceInner->ApplicationTerminatedSendProcessEvent(appRecord);
    EXPECT_EQ(appMgrServiceInner->runningSharedBundleList_.size(), 1);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->ApplicationTerminatedSendProcessEvent(appRecord);
    EXPECT_EQ(appMgrServiceInner->runningSharedBundleList_.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_ApplicationTerminatedSendProcessEvent_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_ApplicationTerminatedSendProcessEvent_0200
 * @tc.desc: Test ApplicationTerminatedSendProcessEvent
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_ApplicationTerminatedSendProcessEvent_0200,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_ApplicationTerminatedSendProcessEvent_0200 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);

    appMgrServiceInner->appRunningManager_->ClearAppRunningRecordMap();
    appMgrServiceInner->ApplicationTerminatedSendProcessEvent(appRecord);
    EXPECT_EQ(appMgrServiceInner->appDebugManager_->debugInfos_.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_ApplicationTerminatedSendProcessEvent_0200 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_ClearAppRunningDataForKeepAlive_0100
 * @tc.desc: Test ClearAppRunningDataForKeepAlive
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_ClearAppRunningDataForKeepAlive_0100,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_ClearAppRunningDataForKeepAlive_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    appMgrServiceInner->ClearAppRunningDataForKeepAlive(appRecord);

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    appMgrServiceInner->ClearAppRunningDataForKeepAlive(appRecord);

    int32_t uid = 101;
    appRecord->SetUid(uid);
    appRecord->SetKeepAliveBundle(true);
    appRecord->SetKeepAliveEnableState(true);
    appRecord->SetKeepAliveDkv(true);
    appRecord->SetSingleton(true);
    appMgrServiceInner->ClearAppRunningDataForKeepAlive(appRecord);

    uid = 200000;
    appRecord->SetUid(uid);
    appMgrServiceInner->ClearAppRunningDataForKeepAlive(appRecord);

    appMgrServiceInner->ClearAppRunningDataForKeepAlive(appRecord);

    appMgrServiceInner->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler("AppMgrServiceInnerSecondTest");
    appMgrServiceInner->ClearAppRunningDataForKeepAlive(appRecord);

    appRecord->SetRestartResidentProcCount(-1);
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);
    int64_t systemTimeMillis = static_cast<int64_t>(((t.tv_sec) * NANOSECONDS + t.tv_nsec) / MICROSECONDS);
    appRecord->restartTimeMillis_ = systemTimeMillis + 1000;
    appMgrServiceInner->ClearAppRunningDataForKeepAlive(appRecord);
    EXPECT_TRUE(appMgrServiceInner != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_ClearAppRunningDataForKeepAlive_0100 end");
}

#ifdef SUPPORT_CHILD_PROCESS
/**
 * @tc.name: GetMultiInstanceInfo_001
 * @tc.desc: Test GetMultiInstanceInfo.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AppMgrServiceInnerSecondTest, GetMultiInstanceInfo_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetMultiInstanceInfo_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    std::shared_ptr<AppRunningRecord> appRecord = appMgrServiceInner->CreateAppRunningRecord(
        loadParam, applicationInfo_, abilityInfo_, TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    EXPECT_NE(appRecord, nullptr);

    RunningMultiAppInfo info;
    EXPECT_NO_THROW(appMgrServiceInner->GetMultiInstanceInfo(nullptr, info));
    EXPECT_EQ(info.runningMultiIntanceInfos.size(), 0);

    EXPECT_NO_THROW(appMgrServiceInner->GetMultiInstanceInfo(appRecord, info));
    EXPECT_EQ(info.runningMultiIntanceInfos.size(), 1);

    info.runningMultiIntanceInfos.emplace_back(RunningMultiInstanceInfo());
    EXPECT_NO_THROW(appMgrServiceInner->GetMultiInstanceInfo(appRecord, info));
    EXPECT_EQ(info.runningMultiIntanceInfos.size(), 2);

    EXPECT_NO_THROW(appMgrServiceInner->GetMultiInstanceInfo(appRecord, info));
    EXPECT_EQ(info.runningMultiIntanceInfos.size(), 2);

    TAG_LOGI(AAFwkTag::TEST, "GetMultiInstanceInfo_001 end");
}
#endif // SUPPORT_CHILD_PROCESS

/**
 * @tc.name: GetRunningMultiInstanceKeys_001
 * @tc.desc: Test GetRunningMultiInstanceKeys.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AppMgrServiceInnerSecondTest, GetRunningMultiInstanceKeys_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetRunningMultiInstanceKeys_001 start");
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    std::vector<std::string> instanceKeys;
    AppMgrServiceInner appMgrServiceInner;
    appMgrServiceInner.GetRunningMultiInstanceKeys(appRecord, instanceKeys);
    EXPECT_TRUE(instanceKeys.empty());

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    appRecord = appMgrServiceInner.CreateAppRunningRecord(
        loadParam, applicationInfo_, abilityInfo_, TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    EXPECT_NE(appRecord, nullptr);
    appMgrServiceInner.GetRunningMultiInstanceKeys(appRecord, instanceKeys);
    EXPECT_FALSE(instanceKeys.empty());

    instanceKeys.push_back(appRecord->GetInstanceKey());
    appMgrServiceInner.GetRunningMultiInstanceKeys(appRecord, instanceKeys);
    EXPECT_FALSE(instanceKeys.size() == 1);

    appRecord->priorityObject_ = nullptr;
    appMgrServiceInner.GetRunningMultiInstanceKeys(appRecord, instanceKeys);
    EXPECT_FALSE(instanceKeys.size() == 1 && instanceKeys[0] == appRecord->GetInstanceKey());
    TAG_LOGI(AAFwkTag::TEST, "GetRunningMultiInstanceKeys_001 end");
}

/**
 * @tc.name: UpdateAbilityState_001
 * @tc.desc: update ability state.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerSecondTest, UpdateAbilityState_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateAbilityState_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->UpdateAbilityState(nullptr, AbilityState::ABILITY_STATE_CREATE);

    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    appMgrServiceInner->UpdateAbilityState(token, AbilityState::ABILITY_STATE_CREATE);

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    std::shared_ptr<AAFwk::Want> want;
    std::string processName = "test_processName";
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token;
    std::shared_ptr<AppRunningRecord> appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam,
        applicationInfo_, nullptr, processName, bundleInfo, hapModuleInfo, want);
    EXPECT_NE(appRecord, nullptr);
    appMgrServiceInner->UpdateAbilityState(token, AbilityState::ABILITY_STATE_CREATE);

    OHOS::sptr<IRemoteObject> token1 = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    loadParam->token = token1;
    std::shared_ptr<AppRunningRecord> appRecord1 = appMgrServiceInner->CreateAppRunningRecord(loadParam,
        applicationInfo_, abilityInfo_, processName, bundleInfo, hapModuleInfo, want);
    EXPECT_NE(appRecord1, nullptr);

    appMgrServiceInner->UpdateAbilityState(token1, AbilityState::ABILITY_STATE_READY);

    auto abilityRecord1 =
        appMgrServiceInner->GetAppRunningRecordByAbilityToken(token1)->GetAbilityRunningRecordByToken(token1);
    EXPECT_NE(abilityRecord1, nullptr);
    abilityRecord1->info_ = nullptr;
    appMgrServiceInner->UpdateAbilityState(token1, AbilityState::ABILITY_STATE_READY);
    EXPECT_EQ(abilityRecord1->GetAbilityInfo(), nullptr);

    appMgrServiceInner->UpdateAbilityState(token1, AbilityState::ABILITY_STATE_CREATE);

    abilityRecord1 =
        appMgrServiceInner->GetAppRunningRecordByAbilityToken(token1)->GetAbilityRunningRecordByToken(token1);
    abilityRecord1->SetState(AbilityState::ABILITY_STATE_TERMINATED);
    appMgrServiceInner->UpdateAbilityState(token1, AbilityState::ABILITY_STATE_TERMINATED);
    EXPECT_EQ(abilityRecord1->GetState(), AbilityState::ABILITY_STATE_TERMINATED);

    TAG_LOGI(AAFwkTag::TEST, "UpdateAbilityState_001 end");
}

/**
 * @tc.name: UpdateAbilityState_002
 * @tc.desc: update ability state.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerSecondTest, UpdateAbilityState_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateAbilityState_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);


    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    std::shared_ptr<AAFwk::Want> want;
    std::string processName = "test_processName";
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    loadParam->token = token;
    std::shared_ptr<AppRunningRecord> appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam,
        applicationInfo_, nullptr, processName, bundleInfo, hapModuleInfo, want);
    ASSERT_NE(appRecord, nullptr);
    appRecord->AddModule(applicationInfo_, abilityInfo_, token, hapModuleInfo, nullptr, 0);

    appRecord->SetPreForeground(false);
    appMgrServiceInner->UpdateAbilityState(token, AbilityState::ABILITY_STATE_FOREGROUND);
    EXPECT_EQ(appRecord->IsPreForeground(), false);

    appRecord->SetPreForeground(true);
    appMgrServiceInner->UpdateAbilityState(token, AbilityState::ABILITY_STATE_CREATE);
    EXPECT_EQ(appRecord->IsPreForeground(), true);

    appRecord->SetPreForeground(true);
    appMgrServiceInner->UpdateAbilityState(token, AbilityState::ABILITY_STATE_FOREGROUND);
    EXPECT_EQ(appRecord->IsPreForeground(), false);

    TAG_LOGI(AAFwkTag::TEST, "UpdateAbilityState_002 end");
}

/**
 * @tc.name: KillProcessByAbilityToken_001
 * @tc.desc: kill process by ability token.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerSecondTest, KillProcessByAbilityToken_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillProcessByAbilityToken_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->KillProcessByAbilityToken(nullptr);

    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    appMgrServiceInner->KillProcessByAbilityToken(token);

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    std::shared_ptr<AAFwk::Want> want;
    std::string processName = "test_processName";
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token;
    std::shared_ptr<AppRunningRecord> appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam,
        applicationInfo_, abilityInfo_, processName, bundleInfo, hapModuleInfo, want);
    EXPECT_NE(appRecord, nullptr);
    appMgrServiceInner->KillProcessByAbilityToken(token);

    appRecord->GetPriorityObject()->SetPid(1);
    appRecord->SetKeepAliveEnableState(true);
    appRecord->SetKeepAliveDkv(true);
    appRecord->SetEmptyKeepAliveAppState(true);
    appMgrServiceInner->KillProcessByAbilityToken(token);

    TAG_LOGI(AAFwkTag::TEST, "KillProcessByAbilityToken_001 end");
}

/**
 * @tc.name: SetOverlayInfo_001
 * @tc.desc: SetOverlayInfo.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AppMgrServiceInnerSecondTest, SetOverlayInfo_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetOverlayInfo_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    appMgrServiceInner->remoteClientManager_ = nullptr;
    AppSpawnStartMsg startMsg;
    appMgrServiceInner->SetOverlayInfo("testBundleName", 1, startMsg);
    EXPECT_EQ(startMsg.flags, 0);
    EXPECT_EQ(startMsg.overlayInfo, "");

    appMgrServiceInner->remoteClientManager_ = std::make_shared<RemoteClientManager>();
    appMgrServiceInner->SetOverlayInfo("testBundleName", 1, startMsg);
    EXPECT_EQ(startMsg.flags, 0);
    EXPECT_EQ(startMsg.overlayInfo, "");

    std::shared_ptr<BundleMgrHelper> bundleManager = nullptr;
    appMgrServiceInner->remoteClientManager_->SetBundleManagerHelper(bundleManager);
    appMgrServiceInner->SetOverlayInfo("testBundleName", 1, startMsg);
    EXPECT_EQ(startMsg.flags, 0);
    EXPECT_EQ(startMsg.overlayInfo, "");

    TAG_LOGI(AAFwkTag::TEST, "SetOverlayInfo_001 end");
}

/**
 * @tc.name: StartProcessVerifyPermission_001
 * @tc.desc: Test StartProcessVerifyPermission.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AppMgrServiceInnerSecondTest, StartProcessVerifyPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartProcessVerifyPermission_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    BundleInfo bundleInfo;
    bool hasAccessBundleDirReq;
    uint8_t setAllowInternet;
    uint8_t allowInternet;
    std::vector<int32_t> gids;
    bundleInfo.reqPermissions.push_back("ohos.permission.ACCESS_BUNDLE_DIR");
    appMgrServiceInner->StartProcessVerifyPermission(
        bundleInfo, hasAccessBundleDirReq, setAllowInternet, allowInternet, gids);
    EXPECT_FALSE(hasAccessBundleDirReq);

    bundleInfo.reqPermissions.clear();
    bundleInfo.reqPermissions.push_back("ohos.permission.INTERNET");
    appMgrServiceInner->StartProcessVerifyPermission(
        bundleInfo, hasAccessBundleDirReq, setAllowInternet, allowInternet, gids);
    EXPECT_FALSE(hasAccessBundleDirReq);

    bundleInfo.reqPermissions.clear();
    bundleInfo.reqPermissions.push_back("ohos.permission.INTERNET");
    bundleInfo.applicationInfo.accessTokenId = 1;
    appMgrServiceInner->StartProcessVerifyPermission(
        bundleInfo, hasAccessBundleDirReq, setAllowInternet, allowInternet, gids);
    EXPECT_EQ(setAllowInternet, 1);
    EXPECT_EQ(allowInternet, 0);

    bundleInfo.reqPermissions.clear();
    bundleInfo.reqPermissions.push_back("ohos.permission.MANAGE_VPN");
    bundleInfo.applicationInfo.accessTokenId = 0;
    appMgrServiceInner->StartProcessVerifyPermission(
        bundleInfo, hasAccessBundleDirReq, setAllowInternet, allowInternet, gids);
    EXPECT_EQ(setAllowInternet, 1);
    EXPECT_EQ(allowInternet, 0);

    bundleInfo.reqPermissions.clear();
    bundleInfo.reqPermissions.push_back("ohos.permission.ACCESS_BUNDLE_DIR");
    bundleInfo.applicationInfo.accessTokenId = 1;
    appMgrServiceInner->StartProcessVerifyPermission(
        bundleInfo, hasAccessBundleDirReq, setAllowInternet, allowInternet, gids);
    EXPECT_FALSE(std::find(gids.begin(), gids.end(), 1097) != gids.end());

    bundleInfo.reqPermissions.clear();
    bundleInfo.applicationInfo.accessTokenId = 1;
    appMgrServiceInner->StartProcessVerifyPermission(
        bundleInfo, hasAccessBundleDirReq, setAllowInternet, allowInternet, gids);
    EXPECT_TRUE(std::find(gids.begin(), gids.end(), 1097) == gids.end());

    TAG_LOGI(AAFwkTag::TEST, "StartProcessVerifyPermission_001 end");
}

/**
 * @tc.name: ProcessAppDebug_0010
 * @tc.desc: Test ProcessAppDebug.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AppMgrServiceInnerSecondTest, ProcessAppDebug_0010, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ProcessAppDebug_0010 start");
    ApplicationInfo applicationInfo;
    applicationInfo.name = "hiservcie";
    applicationInfo.bundleName = "com.ix.hiservcie";
    std::shared_ptr<ApplicationInfo> applicationInfo_ = std::make_shared<ApplicationInfo>(applicationInfo);
    BundleInfo info;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->appDebugManager_ = std::make_shared<AppDebugManager>();
    appMgrServiceInner->ProcessAppDebug(nullptr, true);
    EXPECT_NE(appMgrServiceInner->appDebugManager_, nullptr);

    appMgrServiceInner->appDebugManager_ = nullptr;
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    EXPECT_NE(appRecord, nullptr);
    appMgrServiceInner->ProcessAppDebug(appRecord, true);
    EXPECT_EQ(appRecord->IsDebugApp(), false);

    appMgrServiceInner->appDebugManager_ = std::make_shared<AppDebugManager>();
    appMgrServiceInner->ProcessAppDebug(appRecord, true);
    EXPECT_EQ(appRecord->IsDebugApp(), true);

    appRecord->SetDebugApp(false);
    appMgrServiceInner->ProcessAppDebug(appRecord, true);
    EXPECT_EQ(appRecord->IsDebugApp(), true);

    appRecord->SetDebugApp(true);
    appMgrServiceInner->ProcessAppDebug(appRecord, true);
    EXPECT_EQ(appRecord->IsDebugApp(), true);

    AppDebugInfo adinfo;
    adinfo.bundleName = "com.ix.hiservcie";
    appMgrServiceInner->appDebugManager_->debugInfos_.push_back(adinfo);
    appRecord->SetDebugApp(false);
    appMgrServiceInner->ProcessAppDebug(appRecord, false);
    EXPECT_EQ(appRecord->IsDebugApp(), false);

    appRecord->SetDebugApp(true);
    appMgrServiceInner->ProcessAppDebug(appRecord, false);
    EXPECT_EQ(appRecord->IsDebugApp(), true);

    TAG_LOGI(AAFwkTag::TEST, "ProcessAppDebug_0010 end");
}

/**
 * @tc.name: FinishUserTest_001
 * @tc.desc: Test FinishUserTest.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AppMgrServiceInnerSecondTest, FinishUserTest_0010, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FinishUserTest_0010 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t pid = 0;
    appMgrServiceInner->FinishUserTest("", 0, "", pid);

    std::string msg = "testmsg";
    std::string bundleName = "test_bundle_name";
    appMgrServiceInner->FinishUserTest("", 0, bundleName, pid);
    appMgrServiceInner->FinishUserTest(msg, 0, "", pid);
    appMgrServiceInner->FinishUserTest(msg, 0, bundleName, pid);

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    std::shared_ptr<AAFwk::Want> want;
    sptr<IRemoteObject> token = new MockAbilityToken();
    std::string processName = "test_processName";
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    std::shared_ptr<AppRunningRecord> appRecord = appMgrServiceInner->CreateAppRunningRecord(
        loadParam, applicationInfo_, abilityInfo_, TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    EXPECT_NE(appRecord, nullptr);
    pid = appRecord->GetPriorityObject()->GetPid();
    appMgrServiceInner->FinishUserTest(msg, 0, bundleName, pid);

    std::shared_ptr<UserTestRecord> record = std::make_shared<UserTestRecord>();
    appRecord->SetUserTestInfo(record);
    appMgrServiceInner->FinishUserTest(msg, 0, bundleName, pid);

    appMgrServiceInner->remoteClientManager_ = nullptr;
    appMgrServiceInner->FinishUserTest(msg, 0, bundleName, pid);

    TAG_LOGI(AAFwkTag::TEST, "FinishUserTest_0010 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_JudgeSelfCalledByToken_0100
 * @tc.desc: Test JudgeSelfCalledByToken
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_JudgeSelfCalledByToken_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_JudgeSelfCalledByToken_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    PageStateData pageStateData;
    auto ret = appMgrServiceInner->JudgeSelfCalledByToken(nullptr, pageStateData);
    EXPECT_FALSE(ret);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_JudgeSelfCalledByToken_0100 step1");
    ret = appMgrServiceInner->JudgeSelfCalledByToken(token_, pageStateData);
    EXPECT_FALSE(ret);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_JudgeSelfCalledByToken_0100 step2");
    const BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.isStageBasedModel = true;
    hapModuleInfo.process = TEST_PROCESS_NAME;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    auto appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    ret = appMgrServiceInner->JudgeSelfCalledByToken(token_, pageStateData);
    EXPECT_FALSE(ret);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_JudgeSelfCalledByToken_0100 step3");
    const int32_t accessTokenId = 123; // 123 means tokenid
    applicationInfo_->accessTokenId = accessTokenId;
    IPCSkeleton::SetCallingTokenID(accessTokenId);
    ret = appMgrServiceInner->JudgeSelfCalledByToken(token_, pageStateData);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_JudgeSelfCalledByToken_0100 step4");
    pageStateData.bundleName = TEST_BUNDLE_NAME;
    ret = appMgrServiceInner->JudgeSelfCalledByToken(token_, pageStateData);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_JudgeSelfCalledByToken_0100 step5");
    pageStateData.moduleName = "entry";
    ret = appMgrServiceInner->JudgeSelfCalledByToken(token_, pageStateData);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_JudgeSelfCalledByToken_0100 step6");
    pageStateData.abilityName = "ServiceAbility";
    ret = appMgrServiceInner->JudgeSelfCalledByToken(token_, pageStateData);
    EXPECT_TRUE(ret);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_JudgeSelfCalledByToken_0100 end");
}

#ifdef SUPPORT_CHILD_PROCESS
/**
 * @tc.name: AppMgrServiceInnerSecondTest_StartChildProcessPreCheck_0100
 * @tc.desc: Test StartChildProcessPreCheck
 * @tc.type: FUNC
 */
const char* MULTI_PROCESS_MODEL = "persist.sys.abilityms.multi_process_model";
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_StartChildProcessPreCheck_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_StartChildProcessPreCheck_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    const BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.isStageBasedModel = true;
    hapModuleInfo.process = TEST_PROCESS_NAME;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    auto appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    ChildProcessRequest childRequest;
    auto childProcessRecord = std::make_shared<ChildProcessRecord>(appRecord->GetRecordId(), childRequest, appRecord);
    childProcessRecord->SetPid(PID_1000);
    callingPid = childProcessRecord->GetPid();
    appRecord->AddChildProcessRecord(callingPid, childProcessRecord);
    auto ret = appMgrServiceInner->StartChildProcessPreCheck(callingPid, 1);
    EXPECT_EQ(ret, ERR_ALREADY_IN_CHILD_PROCESS);

    auto appRecord2 = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    auto pid = appRecord2->GetPriorityObject()->GetPid();
    ret = appMgrServiceInner->StartChildProcessPreCheck(pid, 2);
    EXPECT_EQ(ret, ERR_NOT_SUPPORT_CHILD_PROCESS);
    ret = appMgrServiceInner->StartChildProcessPreCheck(pid, 1);
    EXPECT_EQ(ret, ERR_NOT_SUPPORT_CHILD_PROCESS);

    system::SetBoolParameter(MULTI_PROCESS_MODEL, true);
    auto& utils = AAFwk::AppUtils::GetInstance();
    utils.isMultiProcessModel_.isLoaded = false;
    utils.maxChildProcess_.isLoaded = true;
    utils.maxChildProcess_.value = 1000000;
    ret = appMgrServiceInner->StartChildProcessPreCheck(pid, 1);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_StartChildProcessPreCheck_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_StartChildProcess_0100
 * @tc.desc: Test StartChildProcess
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_StartChildProcess_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_StartChildProcess_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    pid_t callingPid = PID_1000;
    pid_t childPid = PID_1000;
    ChildProcessRequest request;
    auto ret = appMgrServiceInner->StartChildProcess(callingPid, childPid, request);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);

    const BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.isStageBasedModel = true;
    hapModuleInfo.process = TEST_PROCESS_NAME;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    auto appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    ChildProcessRequest childRequest;
    childRequest.childProcessType = 1;
    auto childProcessRecord = std::make_shared<ChildProcessRecord>(appRecord->GetRecordId(), childRequest, appRecord);
    childProcessRecord->SetPid(PID_1000 + 1);
    childPid = childProcessRecord->GetPid();
    appRecord->AddChildProcessRecord(childPid, childProcessRecord);
    auto pid = appRecord->GetPriorityObject()->GetPid();
    system::SetBoolParameter(MULTI_PROCESS_MODEL, true);
    auto& utils = AAFwk::AppUtils::GetInstance();
    utils.isMultiProcessModel_.isLoaded = false;
    utils.maxChildProcess_.isLoaded = true;
    utils.maxChildProcess_.value = 1000000;
    ret = appMgrServiceInner->StartChildProcess(pid, childPid, request);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    appRecord->GetPriorityObject()->SetPid(1000);
    pid = appRecord->GetPriorityObject()->GetPid();
    ret = appMgrServiceInner->StartChildProcess(pid, childPid, request);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    request.srcEntry = "1001";
    ret = appMgrServiceInner->StartChildProcess(pid, childPid, request);
    EXPECT_EQ(ret, ERR_APPEXECFWK_BAD_APPSPAWN_CLIENT);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_StartChildProcess_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_KillChildProcess_0100
 * @tc.desc: Test KillChildProcess
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_KillChildProcess_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_KillChildProcess_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    const BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.isStageBasedModel = true;
    hapModuleInfo.process = TEST_PROCESS_NAME;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    auto appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);

    appMgrServiceInner->KillChildProcess(nullptr);
    EXPECT_EQ(appRecord->GetChildProcessRecordMap().size(), 0);

    appMgrServiceInner->KillChildProcess(appRecord);
    EXPECT_EQ(appRecord->GetChildProcessRecordMap().size(), 0);

    ChildProcessRequest childRequest;
    childRequest.childProcessType = 1;
    auto childAppRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    childAppRecord->GetPriorityObject()->SetPid(1);
    auto childProcessRecord = std::make_shared<ChildProcessRecord>(childAppRecord->GetRecordId(), childRequest,
        childAppRecord);
    childProcessRecord->SetPid(1);
    auto childPid = childProcessRecord->GetPid();
    appRecord->AddChildProcessRecord(childPid, childProcessRecord);
    appMgrServiceInner->KillChildProcess(appRecord);
    EXPECT_EQ(appMgrServiceInner->killedProcessMap_.size(), 1);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_KillChildProcess_0100 end");
}
#endif // SUPPORT_CHILD_PROCESS

/**
 * @tc.name: AppMgrServiceInnerSecondTest_VerifyKillProcessPermission_0100
 * @tc.desc: Test VerifyKillProcessPermission
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_VerifyKillProcessPermission_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "VerifyKillProcessPermission_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    auto ret = appMgrServiceInner->VerifyKillProcessPermission(TEST_BUNDLE_NAME);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "VerifyKillProcessPermission_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_VerifyKillProcessPermission_0200
 * @tc.desc: Test VerifyKillProcessPermission
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_VerifyKillProcessPermission_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "VerifyKillProcessPermission_0200 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    auto ret = appMgrServiceInner->VerifyKillProcessPermission(token_);
    EXPECT_EQ(ret, ERR_OK);
    appMgrServiceInner->appRunningManager_ = nullptr;
    ret = appMgrServiceInner->VerifyKillProcessPermission(token_);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "VerifyKillProcessPermission_0200 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_DetachAppDebug_0100
 * @tc.desc: Test DetachAppDebug
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_DetachAppDebug_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DetachAppDebug_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    MyFlag::flag_ = MyFlag::IS_SA_CALL;
    auto ret = appMgrServiceInner->DetachAppDebug(TEST_BUNDLE_NAME);
    EXPECT_EQ(ret, ERR_OK);

    MyFlag::flag_ = MyFlag::IS_SHELL_CALL;
    ret = appMgrServiceInner->DetachAppDebug(TEST_BUNDLE_NAME);
    EXPECT_EQ(ret, ERR_OK);

    appMgrServiceInner->appRunningManager_ = nullptr;
    ret = appMgrServiceInner->DetachAppDebug(TEST_BUNDLE_NAME);
    EXPECT_NE(ret, ERR_OK);

    MyFlag::flag_ = 3;
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    ret = appMgrServiceInner->DetachAppDebug(TEST_BUNDLE_NAME);
    EXPECT_EQ(ret, ERR_OK);

    MyFlag::flag_ = 0;
    ret = appMgrServiceInner->DetachAppDebug(TEST_BUNDLE_NAME);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "DetachAppDebug_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_DetachAppDebug_0200
 * @tc.desc: Test DetachAppDebug
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_DetachAppDebug_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DetachAppDebug_0200 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    MyFlag::flag_ = MyFlag::IS_SHELL_CALL;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    EXPECT_NE(appInfo, nullptr);

    int32_t recordId = 1;
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, TEST_PROCESS_NAME);
    EXPECT_NE(appRecord, nullptr);

    appRecord->mainBundleName_ = TEST_BUNDLE_NAME;
    appRecord->SetDebugApp(false);
    appMgrServiceInner->appRunningManager_->appRunningRecordMap_.emplace(recordId, appRecord);
    int32_t ret = appMgrServiceInner->DetachAppDebug(TEST_BUNDLE_NAME);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "DetachAppDebug_0200 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_KillRenderProcess_0100
 * @tc.desc: Test KillRenderProcess
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_KillRenderProcess_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillRenderProcess_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    appMgrServiceInner->KillRenderProcess(nullptr);

    auto appInfo = std::make_shared<ApplicationInfo>();
    EXPECT_NE(appInfo, nullptr);
    appInfo->bundleName = "testBundleName";
    appInfo->name = "testBundleName";
    std::string processName = "testProcess";
    int32_t recordId = 1;
    pid_t hostPid = 1001;
    auto hostRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    EXPECT_NE(hostRecord, nullptr);
    hostRecord->priorityObject_->SetPid(hostPid);
    hostRecord->SetUid(100);
    std::string renderParam = "test_render_param";
    int32_t ipcFd = -1;
    int32_t sharedFd = -1;
    int32_t crashFd = -1;

    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, TEST_PROCESS_NAME);
    EXPECT_NE(appRunningRecord, nullptr);

    std::shared_ptr<RenderRecord> renderRecord = std::make_shared<RenderRecord>(hostPid, renderParam,
        FdGuard(ipcFd), FdGuard(sharedFd), FdGuard(crashFd), hostRecord);
    EXPECT_NE(renderRecord, nullptr);

    renderRecord->SetPid(10);
    renderRecord->SetUid(10);
    appRunningRecord->AddRenderRecord(renderRecord);
    appMgrServiceInner->KillRenderProcess(appRunningRecord);

    renderRecord->SetPid(0);
    appRunningRecord->AddRenderRecord(renderRecord);
    appMgrServiceInner->KillRenderProcess(appRunningRecord);
    TAG_LOGI(AAFwkTag::TEST, "KillRenderProcess_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_LaunchAbility_0100
 * @tc.type: FUNC
 * @tc.Function: LaunchAbility
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_LaunchAbility_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LaunchAbility_0100 start";

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    int32_t result = appMgrServiceInner->LaunchAbility(nullptr);
    EXPECT_EQ(result, AAFwk::ERR_NULL_APP_RUNNING_MANAGER);

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    std::shared_ptr<AAFwk::Want> want;
    std::string processName = "test_processName";
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    OHOS::sptr<IRemoteObject> token1 = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    loadParam->token = token1;
    std::shared_ptr<AppRunningRecord> appRecord1 = appMgrServiceInner->CreateAppRunningRecord(loadParam,
        applicationInfo_, abilityInfo_, processName, bundleInfo, hapModuleInfo, want);
    EXPECT_NE(appRecord1, nullptr);
    result = appMgrServiceInner->LaunchAbility(token1);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    GTEST_LOG_(INFO) << "LaunchAbility_0100 end";
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_SetKeepAliveEnableState_0100
 * @tc.desc: Test SetKeepAliveEnableState
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_SetKeepAliveEnableState_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_SetKeepAliveEnableState_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    const BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.isStageBasedModel = true;
    hapModuleInfo.process = TEST_PROCESS_NAME;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    auto appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    ASSERT_NE(appRecord, nullptr);

    // case 1, bundleName is empty.
    appMgrServiceInner->SetKeepAliveEnableState("", true, 0);
    EXPECT_FALSE(appRecord->isKeepAliveRdb_);

    // case 2, appRunningManager_ is nullptr.
    std::shared_ptr<AppRunningManager> appRunningManagerBack = appMgrServiceInner->appRunningManager_;
    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->SetKeepAliveEnableState(TEST_BUNDLE_NAME, true, 0);
    EXPECT_FALSE(appRecord->isKeepAliveRdb_);
    appMgrServiceInner->appRunningManager_ = appRunningManagerBack;

    // case 3, not foundation call.
    appMgrServiceInner->SetKeepAliveEnableState(TEST_BUNDLE_NAME, true, 0);
    EXPECT_FALSE(appRecord->isKeepAliveRdb_);

    // case 4, caller bundleName and appRecord bundleName are not equal.
    IPCSkeleton::SetCallingUid(FOUNDATION_UID);
    appMgrServiceInner->SetKeepAliveEnableState("InvalidBundleName", true, 0);
    EXPECT_FALSE(appRecord->isKeepAliveRdb_);

    // case 5, set success.
    appMgrServiceInner->SetKeepAliveEnableState(TEST_BUNDLE_NAME, true, 0);
    EXPECT_TRUE(appRecord->isKeepAliveRdb_);

    IPCSkeleton::SetCallingUid(0);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_SetKeepAliveEnableState_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_SetKeepAliveDkv_0100
 * @tc.desc: Test SetKeepAliveDkv
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_SetKeepAliveDkv_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_SetKeepAliveDkv_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    const BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.isStageBasedModel = true;
    hapModuleInfo.process = TEST_PROCESS_NAME;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    auto appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    ASSERT_NE(appRecord, nullptr);

    // case 1, bundleName is empty.
    appMgrServiceInner->SetKeepAliveDkv("", true, 0);
    EXPECT_FALSE(appRecord->isKeepAliveDkv_);

    // case 2, appRunningManager_ is nullptr.
    std::shared_ptr<AppRunningManager> appRunningManagerBack = appMgrServiceInner->appRunningManager_;
    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->SetKeepAliveDkv(TEST_BUNDLE_NAME, true, 0);
    EXPECT_FALSE(appRecord->isKeepAliveDkv_);
    appMgrServiceInner->appRunningManager_ = appRunningManagerBack;

    // case 3, not foundation call.
    appMgrServiceInner->SetKeepAliveDkv(TEST_BUNDLE_NAME, true, 0);
    EXPECT_FALSE(appRecord->isKeepAliveDkv_);

    // case 4, caller bundleName and appRecord bundleName are not equal.
    IPCSkeleton::SetCallingUid(FOUNDATION_UID);
    appMgrServiceInner->SetKeepAliveDkv("InvalidBundleName", true, 0);
    EXPECT_FALSE(appRecord->isKeepAliveDkv_);

    // case 5, not mainElement running.
    appRecord->extensionType_ = AppExecFwk::ExtensionAbilityType::APP_SERVICE;
    appMgrServiceInner->SetKeepAliveDkv(TEST_BUNDLE_NAME, true, 0);
    EXPECT_FALSE(appRecord->isKeepAliveDkv_);

    // case 6, set success.
    appRecord->extensionType_ = AppExecFwk::ExtensionAbilityType::APP_SERVICE;
    appRecord->isMainElementRunning_ = true;
    appMgrServiceInner->SetKeepAliveDkv(TEST_BUNDLE_NAME, true, 0);
    EXPECT_TRUE(appRecord->isKeepAliveDkv_);

    IPCSkeleton::SetCallingUid(0);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_SetKeepAliveDkv_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_GetKeepAliveState_0100
 * @tc.desc: Test GetKeepAliveState
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_GetKeepAliveState_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetKeepAliveState_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    const BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.isStageBasedModel = true;
    hapModuleInfo.process = TEST_PROCESS_NAME;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    auto appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    ASSERT_NE(appRecord, nullptr);

    // case 1, appRecord is nullptr.
    auto resultState = appMgrServiceInner->GetKeepAliveState(nullptr);
    EXPECT_FALSE(resultState);

    // case 2, get success.
    appRecord->isKeepAliveAppService_ = true;
    resultState = appMgrServiceInner->GetKeepAliveState(appRecord);
    EXPECT_TRUE(resultState);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetKeepAliveState_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_SetKeepAliveAppService_0100
 * @tc.desc: Test SetKeepAliveAppService
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_SetKeepAliveAppService_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_SetKeepAliveAppService_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    const BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.isStageBasedModel = true;
    hapModuleInfo.process = TEST_PROCESS_NAME;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    auto appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    ASSERT_NE(appRecord, nullptr);

    // case 1, bundleName is empty.
    appMgrServiceInner->SetKeepAliveAppService("", true, -1);
    EXPECT_FALSE(appRecord->isKeepAliveAppService_);

    // case 2, appRunningManager_ is nullptr.
    std::shared_ptr<AppRunningManager> appRunningManagerBack = appMgrServiceInner->appRunningManager_;
    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->SetKeepAliveAppService(TEST_BUNDLE_NAME, true, -1);
    EXPECT_FALSE(appRecord->isKeepAliveAppService_);
    appMgrServiceInner->appRunningManager_ = appRunningManagerBack;

    // case 3, not foundation call.
    appMgrServiceInner->SetKeepAliveAppService(TEST_BUNDLE_NAME, true, -1);
    EXPECT_FALSE(appRecord->isKeepAliveAppService_);

    // case 4, caller bundleName and appRecord bundleName are not equal.
    IPCSkeleton::SetCallingUid(FOUNDATION_UID);
    appMgrServiceInner->SetKeepAliveAppService("InvalidBundleName", true, -1);
    EXPECT_FALSE(appRecord->isKeepAliveAppService_);

    // case 5, set success.
    appRecord->extensionType_ = AppExecFwk::ExtensionAbilityType::APP_SERVICE;
    int32_t uid = 0;
    appRecord->SetUid(uid);
    appMgrServiceInner->SetKeepAliveAppService(TEST_BUNDLE_NAME, true, uid);
    EXPECT_TRUE(appRecord->isKeepAliveAppService_);

    IPCSkeleton::SetCallingUid(0);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_SetKeepAliveAppService_0100 end");
}

/**
 * @tc.name: PreCheckStartProcess_001
 * @tc.desc: Test PreCheckStartProcess
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, PreCheckStartProcess_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreCheckStartProcess_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::string bundleName = "test.bundle.name";
    int32_t uid = UID_ONE;
    int32_t appIndex = 0;
    auto ret = appMgrServiceInner->PreCheckStartProcess(bundleName, uid, appIndex);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "PreCheckStartProcess_001 end");
}

/**
 * @tc.name: PreCheckStartProcess_002
 * @tc.desc: Test PreCheckStartProcess
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, PreCheckStartProcess_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreCheckStartProcess_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::string bundleName = "test.bundle.name";
    int32_t uid = UID_ONE;
    appMgrServiceInner->InsertUninstallOrUpgradeUidSet(UID_ONE);
    bool flag = appMgrServiceInner->IsUninstallingOrUpgrading(UID_ONE);
    EXPECT_EQ(flag, true);
    int32_t appIndex = 0;
    auto ret = appMgrServiceInner->PreCheckStartProcess(bundleName, uid, appIndex);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "PreCheckStartProcess_002 end");
}

/**
 * @tc.name: RemoveUninstallOrUpgradeUidSet_001
 * @tc.desc: Test RemoveUninstallOrUpgradeUidSet
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, RemoveUninstallOrUpgradeUidSet_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoveUninstallOrUpgradeUidSet_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->InsertUninstallOrUpgradeUidSet(UID_ONE);
    appMgrServiceInner->InsertUninstallOrUpgradeUidSet(UID_TWO);
    auto ret = appMgrServiceInner->IsUninstallingOrUpgrading(UID_ONE);
    EXPECT_EQ(ret, true);
    appMgrServiceInner->RemoveUninstallOrUpgradeUidSet(UID_TWO);
    ret = appMgrServiceInner->IsUninstallingOrUpgrading(UID_ONE);
    EXPECT_EQ(ret, true);
    appMgrServiceInner->RemoveUninstallOrUpgradeUidSet(UID_ONE);
    ret = appMgrServiceInner->IsUninstallingOrUpgrading(UID_ONE);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "RemoveUninstallOrUpgradeUidSet_001 end");
}

/**
 * @tc.name: NotifyUninstallOrUpgradeApp_001
 * @tc.desc: Test NotifyUninstallOrUpgradeApp
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, NotifyUninstallOrUpgradeApp_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyUninstallOrUpgradeApp_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::string bundleName = "test.bundle.name";
    int32_t uid = UID_ONE;
    bool isUpgrade = true;
    appMgrServiceInner->appRunningManager_ = nullptr;
    auto ret = appMgrServiceInner->NotifyUninstallOrUpgradeApp(bundleName, uid, isUpgrade);
    EXPECT_EQ(ret, ERR_NO_INIT); //appRunningManager_ null

    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    MyFlag::flag_ = 0;
    ret = appMgrServiceInner->NotifyUninstallOrUpgradeApp(bundleName, uid, isUpgrade);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED); //permission verification fail

    MyFlag::flag_ = MyFlag::IS_SA_CALL;
    appMgrServiceInner->remoteClientManager_ = nullptr;
    ret = appMgrServiceInner->NotifyUninstallOrUpgradeApp(bundleName, uid, isUpgrade);
    EXPECT_EQ(ret, ERR_OK); //remoteClientManager_ null

    appMgrServiceInner->remoteClientManager_ = std::make_shared<RemoteClientManager>();
    ret = appMgrServiceInner->NotifyUninstallOrUpgradeApp(bundleName, uid, isUpgrade);
    EXPECT_EQ(ret, ERR_OK); //unstart
    TAG_LOGI(AAFwkTag::TEST, "NotifyUninstallOrUpgradeApp_001 end");
}

} // namespace AppExecFwk
} // namespace OHOS