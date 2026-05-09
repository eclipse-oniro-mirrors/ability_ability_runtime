/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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
#include "remote_client_manager.h"
#undef private
#include "ability_manager_errors.h"
#include "accesstoken_kit.h"
#include "app_scheduler.h"
#include "appspawn_util.h"
#include "app_spawn_client.h"
#include "event_handler.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "mock_ability_token.h"
#include "mock_app_scheduler.h"
#include "mock_app_utils.h"
#include "mock_bundle_manager.h"
#include "mock_configuration_observer.h"
#include "mock_iapp_state_callback.h"
#include "mock_kia_interceptor.h"
#include "mock_my_app_utils_flag.h"
#include "mock_my_flag.h"
#include "mock_native_token.h"
#include "mock_permission_verification.h"
#include "mock_render_scheduler.h"
#include "mock_res_sched_util.h"
#include "mock_sa_call.h"
#include "mock_task_handler_wrap.h"
#include "param.h"
#include "parameters.h"
#include "render_state_observer_stub.h"
#include "window_manager.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class WindowFocusChangedListener : public OHOS::Rosen::IFocusChangedListener {
public:
    WindowFocusChangedListener(const std::shared_ptr<AppMgrServiceInner>& owner,
        const std::shared_ptr<AAFwk::TaskHandlerWrap>& handler);
    virtual ~WindowFocusChangedListener();

    void OnFocused(const sptr<OHOS::Rosen::FocusChangeInfo> &focusChangeInfo) override;
    void OnUnfocused(const sptr<OHOS::Rosen::FocusChangeInfo> &focusChangeInfo) override;

private:
    std::weak_ptr<AppMgrServiceInner> owner_;
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_;
};
namespace {
constexpr int32_t RECORD_ID = 1;
constexpr int32_t APP_DEBUG_INFO_PID = 0;
constexpr int32_t APP_DEBUG_INFO_UID = 0;
constexpr const char* PERMISSION_PROTECT_SCREEN_LOCK_DATA_TEST = "ohos.permission.PROTECT_SCREEN_LOCK_DATA";
constexpr int32_t FUN_TEST_PID = 0;
}
static int recordId_ = 0;
class AppMgrServiceInnerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    void InitAppInfo(const std::string& deviceName, const std::string& abilityName,
        const std::string& appName, const std::string& bundleName, const std::string& moduleName);

public:
    std::shared_ptr<AbilityInfo> abilityInfo_;
    std::shared_ptr<ApplicationInfo> applicationInfo_;
};

class RenderStateObserverMock : public RenderStateObserverStub {
public:
    RenderStateObserverMock() = default;
    virtual ~RenderStateObserverMock() = default;
    void OnRenderStateChanged(const RenderStateData &renderStateData) override
    {}
};

void AppMgrServiceInnerTest::InitAppInfo(const std::string& deviceName,
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
}

class MockIUserCallback : public AAFwk::IUserCallback {
public:
    MockIUserCallback() = default;
    virtual ~MockIUserCallback() = default;

    void OnUserCmdDone(int userId, int errcode) override
    {}

    sptr<IRemoteObject> AsObject() override
    {
        return {};
    }
};

void AppMgrServiceInnerTest::SetUpTestCase(void)
{
    MockNativeToken::SetNativeToken();
}

void AppMgrServiceInnerTest::TearDownTestCase(void)
{}

void AppMgrServiceInnerTest::SetUp()
{
    // init test app info
    std::string deviceName = "device";
    std::string abilityName = "ServiceAbility";
    std::string appName = "hiservcie";
    std::string bundleName = "com.ix.hiservcie";
    std::string moduleName = "entry";
    InitAppInfo(deviceName, abilityName, appName, bundleName, moduleName);
}

void AppMgrServiceInnerTest::TearDown()
{}

/**
 * @tc.name: PointerDeviceCallback_0100
 * @tc.desc: set parameter, expect config update
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, PointerDeviceCallback_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PointerDeviceCallback_0100 start");

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::string key = AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE;
    auto context = new (std::nothrow) std::weak_ptr<AppMgrServiceInner>(appMgrServiceInner);
    std::shared_ptr<AppExecFwk::Configuration> config = nullptr;
    std::string value;

    // invalid parameter value
    appMgrServiceInner->PointerDeviceEventCallback(key.c_str(), "false", context);
    config = appMgrServiceInner->GetConfiguration();
    EXPECT_NE(config, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "PointerDeviceCallback_0100 end");
}

/**
 * @tc.name: PointerDeviceCallback_0200
 * @tc.desc: set parameter, expect config update
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, PointerDeviceCallback_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PointerDeviceCallback_0200 start");

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::string key = AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE;
    auto context = new (std::nothrow) std::weak_ptr<AppMgrServiceInner>(appMgrServiceInner);
    std::shared_ptr<AppExecFwk::Configuration> config = nullptr;
    std::string value;

    // invalid parameter value
    appMgrServiceInner->PointerDeviceEventCallback(key.c_str(), "true", context);
    config = appMgrServiceInner->GetConfiguration();
    EXPECT_NE(config, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "PointerDeviceCallback_0200 end");
}

/**
 * @tc.name: PointerDeviceCallback_0300
 * @tc.desc: set parameter, expect config update
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, PointerDeviceCallback_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PointerDeviceCallback_0300 start");

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::string key = AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE;
    auto context = new (std::nothrow) std::weak_ptr<AppMgrServiceInner>(appMgrServiceInner);
    std::shared_ptr<AppExecFwk::Configuration> config = nullptr;
    std::string value;

    // invalid parameter value
    appMgrServiceInner->PointerDeviceEventCallback("invalid_key", "false", context);
    config = appMgrServiceInner->GetConfiguration();
    EXPECT_NE(config, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "PointerDeviceCallback_0300 end");
}

/**
 * @tc.name: PointerDeviceCallback_0400
 * @tc.desc: set parameter, expect config update
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, PointerDeviceCallback_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PointerDeviceCallback_0400 start");

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::string key = AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE;
    auto context = new (std::nothrow) std::weak_ptr<AppMgrServiceInner>(appMgrServiceInner);
    std::shared_ptr<AppExecFwk::Configuration> config = nullptr;
    std::string value;

    // invalid parameter value
    appMgrServiceInner->PointerDeviceEventCallback(key.c_str(), "invalid", context);
    config = appMgrServiceInner->GetConfiguration();
    EXPECT_NE(config, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "PointerDeviceCallback_0400 end");
}

/**
 * @tc.name: PointerDeviceWatchParameter_0100
 * @tc.desc: set parameter, expect config update
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, PointerDeviceWatchParameter_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PointerDeviceWatchParameter_0100 start");

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::string key = AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE;
    std::shared_ptr<AppExecFwk::Configuration> config = nullptr;
    std::string value;

    appMgrServiceInner->AddWatchParameter();
    sleep(1);

    // invalid parameter value
    system::SetParameter(key.c_str(), "invalid");
    sleep(1);
    config = appMgrServiceInner->GetConfiguration();
    EXPECT_NE(config, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "PointerDeviceWatchParameter_0100 end");
}

/**
 * @tc.name: PointerDeviceWatchParameter_0200
 * @tc.desc: set parameter, expect config update
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, PointerDeviceWatchParameter_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PointerDeviceWatchParameter_0200 start");

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::string key = AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE;
    std::shared_ptr<AppExecFwk::Configuration> config = nullptr;
    std::string value;

    appMgrServiceInner->AddWatchParameter();
    sleep(1);

    // set "input.pointer.device" to false
    system::SetParameter(key.c_str(), "false");
    sleep(1);
    config = appMgrServiceInner->GetConfiguration();
    EXPECT_NE(config, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "PointerDeviceWatchParameter_0200 end");
}

/**
 * @tc.name: PointerDeviceWatchParameter_0300
 * @tc.desc: set parameter, expect config update
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, PointerDeviceWatchParameter_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PointerDeviceWatchParameter_0300 start");

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::string key = AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE;
    std::shared_ptr<AppExecFwk::Configuration> config = nullptr;
    std::string value;

    appMgrServiceInner->AddWatchParameter();
    sleep(1);

    // set "input.pointer.device" to true
    system::SetParameter(key.c_str(), "true");
    sleep(1);
    config = appMgrServiceInner->GetConfiguration();
    EXPECT_NE(config, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "PointerDeviceWatchParameter_0300 end");
}

/**
 * @tc.name: PointerDeviceUpdateConfig_0100
 * @tc.desc: set parameter, expect config update
 * @tc.type: FUNC
 * @tc.require: I581UL
 */
HWTEST_F(AppMgrServiceInnerTest, PointerDeviceUpdateConfig_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PointerDeviceUpdateConfig_0100 start");

    AAFwk::IsMockSaCall::IsMockSpecificSystemAbilityAccessPermission();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::shared_ptr<AppExecFwk::Configuration> config;
    std::string value;
    int32_t result;

    appMgrServiceInner->InitGlobalConfiguration();
    config = appMgrServiceInner->GetConfiguration();
    EXPECT_NE(config, nullptr);
    value = config->GetItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE);
    EXPECT_TRUE((value == "true") || (value == "false"));

    // config didn't change
    result = appMgrServiceInner->UpdateConfiguration(*config);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    Configuration changeConfig;
    if (value == "true") {
        changeConfig.AddItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE, "false");
        result = appMgrServiceInner->UpdateConfiguration(changeConfig);
        EXPECT_EQ(result, ERR_OK);
        config = appMgrServiceInner->GetConfiguration();
        EXPECT_NE(config, nullptr);
        value = config->GetItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE);
        EXPECT_EQ(value, "false");
    } else {
        changeConfig.AddItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE, "true");
        result = appMgrServiceInner->UpdateConfiguration(changeConfig);
        EXPECT_EQ(result, ERR_PERMISSION_DENIED);
        config = appMgrServiceInner->GetConfiguration();
        EXPECT_NE(config, nullptr);
        value = config->GetItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE);
        EXPECT_EQ(value, "true");
    }

    TAG_LOGI(AAFwkTag::TEST, "PointerDeviceUpdateConfig_0100 end");
}

/**
 * @tc.name: PreStartNWebSpawnProcess_001
 * @tc.desc: prestart nwebspawn process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, PreStartNWebSpawnProcess_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    int callingPid = IPCSkeleton::GetCallingPid();
    int ret = appMgrServiceInner->PreStartNWebSpawnProcess(callingPid);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: HandleExistingAppRecordAfterFound_001
 * @tc.desc: verify keepAlive/mainElement flags and preload reset without process cache
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, HandleExistingAppRecordAfterFound_001, TestSize.Level1)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);

    HapModuleInfo hapModuleInfo;
    auto want = std::make_shared<AAFwk::Want>();
    BundleInfo info;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, info, "");

    appRecord->SetKeepAliveDkv(false);
    appRecord->SetMainElementRunning(false);
    appRecord->SetPreloadMode(PreloadMode::PRE_LAUNCH);
    appRecord->SetPreloadState(PreloadState::PRELOADING);

    AbilityRuntime::LoadParam loadParam;
    loadParam.isKeepAlive = true;
    loadParam.isMainElementRunning = true;

    appMgrServiceInner->HandleExistingAppRecordAfterFound(appRecord, abilityInfo_, hapModuleInfo, want, false,
        std::make_shared<AbilityRuntime::LoadParam>(loadParam));

    EXPECT_TRUE(appRecord->IsKeepAliveDkv());
    EXPECT_TRUE(appRecord->IsMainElementRunning());
    EXPECT_EQ(appRecord->GetPreloadMode(), PreloadMode::PRELOAD_NONE);
}

/**
 * @tc.name: HandleExistingAppRecordAfterFound_002
 * @tc.desc: verify request proc code propagation from want to record
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, HandleExistingAppRecordAfterFound_002, TestSize.Level1)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);

    HapModuleInfo hapModuleInfo;
    auto want = std::make_shared<AAFwk::Want>();
    want->SetParam(Want::PARAM_RESV_REQUEST_PROC_CODE, 5);
    BundleInfo info;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, info, "");

    EXPECT_EQ(appRecord->GetRequestProcCode(), 0);

    AbilityRuntime::LoadParam loadParam;
    appMgrServiceInner->HandleExistingAppRecordAfterFound(appRecord, abilityInfo_, hapModuleInfo, want, false,
        std::make_shared<AbilityRuntime::LoadParam>(loadParam));

    EXPECT_EQ(appRecord->GetRequestProcCode(), 5);
}

/**
 * @tc.name: HandleExistingAppRecordAfterFound_003
 * @tc.desc: when extension type mismatches in EXTENSION process, reset to NORMAL
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, HandleExistingAppRecordAfterFound_003, TestSize.Level1)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);

    HapModuleInfo hapModuleInfo;
    auto want = std::make_shared<AAFwk::Want>();
    BundleInfo info;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, info, "");

    appRecord->SetProcessType(ProcessType::EXTENSION);
    appRecord->extensionType_ = ExtensionAbilityType::SERVICE;
    abilityInfo_->extensionAbilityType = ExtensionAbilityType::UI_SERVICE;

    AbilityRuntime::LoadParam loadParam;
    appMgrServiceInner->HandleExistingAppRecordAfterFound(appRecord, abilityInfo_, hapModuleInfo, want, false,
        std::make_shared<AbilityRuntime::LoadParam>(loadParam));

    EXPECT_EQ(appRecord->GetProcessType(), ProcessType::NORMAL);
}

/**
 * @tc.name: HandleExistingAppRecordAfterFound_004
 * @tc.desc: verify preload-by-phase state is preserved when loadParam is preload start
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, HandleExistingAppRecordAfterFound_004, TestSize.Level1)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);

    HapModuleInfo hapModuleInfo;
    auto want = std::make_shared<AAFwk::Want>();
    BundleInfo info;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, info, "");

    appRecord->SetPreloadMode(PreloadMode::PRELOAD_BY_PHASE);
    appRecord->SetPreloadPhase(PreloadPhase::WINDOW_STAGE_CREATED);
    appRecord->SetPreloadState(PreloadState::PRELOADED);

    AbilityRuntime::LoadParam loadParam;
    loadParam.isPreloadStart = true;

    appMgrServiceInner->HandleExistingAppRecordAfterFound(appRecord, abilityInfo_, hapModuleInfo, want, false,
        std::make_shared<AbilityRuntime::LoadParam>(loadParam));

    EXPECT_EQ(appRecord->GetPreloadMode(), PreloadMode::PRELOAD_BY_PHASE);
    EXPECT_EQ(appRecord->GetPreloadPhase(), PreloadPhase::WINDOW_STAGE_CREATED);
}

/**
 * @tc.name: HandleExistingAppRecordAfterFound_005
 * @tc.desc: verify preload state is cleared only when reused preload process already contains multiple abilities
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, HandleExistingAppRecordAfterFound_005, TestSize.Level1)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);

    HapModuleInfo hapModuleInfo;
    auto want = std::make_shared<AAFwk::Want>();
    BundleInfo info;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, info, "");
    ASSERT_NE(appRecord, nullptr);

    auto moduleRecord = std::make_shared<ModuleRunningRecord>(applicationInfo_, nullptr);
    ASSERT_NE(moduleRecord, nullptr);
    auto firstAbilityInfo = std::make_shared<AbilityInfo>(*abilityInfo_);
    auto secondAbilityInfo = std::make_shared<AbilityInfo>(*abilityInfo_);
    sptr<IRemoteObject> firstToken = new (std::nothrow) MockAbilityToken();
    sptr<IRemoteObject> secondToken = new (std::nothrow) MockAbilityToken();
    ASSERT_NE(firstToken, nullptr);
    ASSERT_NE(secondToken, nullptr);
    ASSERT_NE(moduleRecord->AddAbility(firstToken, firstAbilityInfo, nullptr, recordId_), nullptr);
    ASSERT_NE(moduleRecord->AddAbility(secondToken, secondAbilityInfo, nullptr, recordId_ + 1), nullptr);
    appRecord->hapModules_[applicationInfo_->bundleName].emplace_back(moduleRecord);

    appRecord->SetPreloadMode(PreloadMode::PRELOAD_BY_PHASE);
    appRecord->SetPreloadPhase(PreloadPhase::WINDOW_STAGE_CREATED);
    appRecord->SetPreloadState(PreloadState::PRELOADED);

    AbilityRuntime::LoadParam loadParam;
    appMgrServiceInner->HandleExistingAppRecordAfterFound(appRecord, abilityInfo_, hapModuleInfo, want, false,
        std::make_shared<AbilityRuntime::LoadParam>(loadParam));

    EXPECT_EQ(appRecord->GetAbilities().size(), 2);
    EXPECT_EQ(appRecord->GetPreloadMode(), PreloadMode::PRELOAD_NONE);
}

/**
 * @tc.name: PreStartNWebSpawnProcess_002
 * @tc.desc: prestart nwebspawn process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, PreStartNWebSpawnProcess_002, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    int callingPid = 0;
    int ret = appMgrServiceInner->PreStartNWebSpawnProcess(callingPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: LoadAbility_001
 * @tc.desc: load ability.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, LoadAbility_001, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "LoadAbility_001 start");
    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->appRunningManager_ = nullptr;
    AbilityRuntime::LoadParam loadParam;
    loadParam.token = token;
    auto loadParamPtr = std::make_shared<AbilityRuntime::LoadParam>(loadParam);
    appMgrServiceInner->LoadAbility(abilityInfo_, applicationInfo_, nullptr, loadParamPtr);

    auto appMgrServiceInner1 = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner1, nullptr);

    appMgrServiceInner1->remoteClientManager_->SetBundleManagerHelper(nullptr);
    appMgrServiceInner1->LoadAbility(abilityInfo_, applicationInfo_, nullptr, loadParamPtr);

    auto appMgrServiceInner2 = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner2, nullptr);

    appMgrServiceInner2->LoadAbility(abilityInfo_, applicationInfo_, nullptr, loadParamPtr);
    TAG_LOGI(AAFwkTag::TEST, "LoadAbility_001 end");
}

/**
 * @tc.name: LoadAbility_002
 * @tc.desc: test LoadAbility with isPreloadUIExtension flag set to false
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, LoadAbility_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "LoadAbility_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);
    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(
            applicationInfo_, processName, bundleInfo, "");
    ASSERT_NE(appRecord, nullptr);
    appRecord->SetUIExtensionPreloadState(false);
    EXPECT_FALSE(appRecord->GetUIExtensionPreloadState());
    AbilityRuntime::LoadParam loadParam;
    loadParam.token = token;
    loadParam.isPreloadUIExtension = false;
    auto loadParamPtr = std::make_shared<AbilityRuntime::LoadParam>(loadParam);
    appMgrServiceInner->remoteClientManager_->SetBundleManagerHelper(nullptr);
    appMgrServiceInner->LoadAbility(abilityInfo_, applicationInfo_, nullptr, loadParamPtr);
    EXPECT_FALSE(appRecord->GetUIExtensionPreloadState());
    TAG_LOGI(AAFwkTag::TEST, "LoadAbility_002 end");
}

/**
 * @tc.name: UpdateUIExtensionPreloadState_001
 * @tc.desc: test UpdateUIExtensionPreloadState function
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, UpdateUIExtensionPreloadState_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateUIExtensionPreloadState_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);
    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(
            applicationInfo_, processName, bundleInfo, "");
    ASSERT_NE(appRecord, nullptr);
    appRecord->extensionType_ = ExtensionAbilityType::UI;
    appRecord->SetUIExtensionPreloadState(false);
    EXPECT_FALSE(appRecord->GetUIExtensionPreloadState());
    appMgrServiceInner->UpdateUIExtensionPreloadState(appRecord, true);
    EXPECT_TRUE(appRecord->GetUIExtensionPreloadState());
    appMgrServiceInner->UpdateUIExtensionPreloadState(appRecord, false);
    EXPECT_FALSE(appRecord->GetUIExtensionPreloadState());
    appMgrServiceInner->UpdateUIExtensionPreloadState(appRecord, false);
    EXPECT_FALSE(appRecord->GetUIExtensionPreloadState());
    appRecord->extensionType_ = ExtensionAbilityType::SERVICE;
    appRecord->SetUIExtensionPreloadState(false);
    appMgrServiceInner->UpdateUIExtensionPreloadState(appRecord, true);
    EXPECT_FALSE(appRecord->GetUIExtensionPreloadState());
    appMgrServiceInner->UpdateUIExtensionPreloadState(nullptr, true);
    appRecord->extensionType_ = ExtensionAbilityType::UI;
    appRecord->SetUIExtensionPreloadState(false);
    appMgrServiceInner->UpdateUIExtensionPreloadState(appRecord, true);
    EXPECT_TRUE(appRecord->GetUIExtensionPreloadState());
    TAG_LOGI(AAFwkTag::TEST, "UpdateUIExtensionPreloadState_001 end");
}

/**
 * @tc.name: UpdataWindowStageCreatPreloadState_001
 * @tc.desc: test UpdateWindowStageCreatedPreloadState function with valid appRecord
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, UpdataWindowStageCreatPreloadState_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdataWindowStageCreatPreloadState_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);
    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(
            applicationInfo_, processName, bundleInfo, "");
    ASSERT_NE(appRecord, nullptr);
    appRecord->SetPreloadMode(AppExecFwk::PreloadMode::PRELOAD_NONE);
    appRecord->SetPreloadPhase(AppExecFwk::PreloadPhase::UNSPECIFIED);
    appRecord->SetPreloadState(AppExecFwk::PreloadState::NONE);
    appMgrServiceInner->UpdateWindowStageCreatedPreloadState(appRecord);
    EXPECT_EQ(appRecord->GetPreloadMode(), AppExecFwk::PreloadMode::PRELOAD_BY_PHASE);
    EXPECT_EQ(appRecord->GetPreloadPhase(), AppExecFwk::PreloadPhase::WINDOW_STAGE_CREATED);
    TAG_LOGI(AAFwkTag::TEST, "UpdataWindowStageCreatPreloadState_001 end");
}

/**
 * @tc.name: UpdataWindowStageCreatPreloadState_002
 * @tc.desc: test UpdateWindowStageCreatedPreloadState also sets preload state to PRELOADING
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, UpdataWindowStageCreatPreloadState_002, TestSize.Level1)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);
    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo, "");
    ASSERT_NE(appRecord, nullptr);

    appRecord->SetPreloadState(AppExecFwk::PreloadState::NONE);
    appMgrServiceInner->UpdateWindowStageCreatedPreloadState(appRecord);

    EXPECT_EQ(appRecord->GetPreloadMode(), AppExecFwk::PreloadMode::PRELOAD_BY_PHASE);
    EXPECT_EQ(appRecord->GetPreloadPhase(), AppExecFwk::PreloadPhase::WINDOW_STAGE_CREATED);
}

/**
 * @tc.name: SetPreloadFlagForProcessInfo_001
 * @tc.desc: test SetPreloadFlagForProcessInfo function
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, SetPreloadFlagForProcessInfo_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetPreloadFlagForProcessInfo_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);
    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(
            applicationInfo_, processName, bundleInfo, "");
    ASSERT_NE(appRecord, nullptr);
    appRecord->SetPreloadMode(AppExecFwk::PreloadMode::PRELOAD_NONE);
    appRecord->SetUIExtensionPreloadState(false);
    bool result = appMgrServiceInner->SetPreloadFlagForProcessInfo(appRecord);
    EXPECT_FALSE(result);
    appRecord->SetUIExtensionPreloadState(true);
    result = appMgrServiceInner->SetPreloadFlagForProcessInfo(appRecord);
    EXPECT_TRUE(result);
    appRecord->SetUIExtensionPreloadState(false);
    appRecord->SetPreloadMode(AppExecFwk::PreloadMode::PRE_LAUNCH);
    result = appMgrServiceInner->SetPreloadFlagForProcessInfo(appRecord);
    EXPECT_TRUE(result);
    result = appMgrServiceInner->SetPreloadFlagForProcessInfo(nullptr);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "SetPreloadFlagForProcessInfo_001 end");
}

/**
 * @tc.name: CheckLoadAbilityConditions_001
 * @tc.desc: check load ability conditions.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, ReportUIExtensionProcColdStartToRss_001, TestSize.Level1)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    // do test1
    appMgrServiceInner->ReportUIExtensionProcColdStartToRss(nullptr, nullptr, false);

    int32_t hostPid = 102;
    // fill want
    std::shared_ptr<AAFwk::Want> want = std::make_shared<Want>();
    want->SetParam("ability.want.params.uiExtensionRootHostPid", hostPid);
    want->SetElementName("bundleName", "abilityName");
    want->SetModuleName("moduleName");

    // do test2
    appMgrServiceInner->ReportUIExtensionProcColdStartToRss(nullptr, want, false);
    // do test3
    appMgrServiceInner->ReportUIExtensionProcColdStartToRss(abilityInfo_, nullptr, false);

    // fill extensionAbilityType
    abilityInfo_->extensionAbilityType = AppExecFwk::ExtensionAbilityType::SYSPICKER_PHOTOEDITOR; // 504
    // EXPECT_CALL
    EXPECT_CALL(ResSchedUtil::GetInstance(), ReportUIExtensionProcColdStartToRss(
        Eq(static_cast<int32_t>(AppExecFwk::ExtensionAbilityType::SYSPICKER_PHOTOEDITOR)),
        Eq(hostPid),
        StrEq(""),
        StrEq("bundleName"),
        StrEq("abilityName"),
        StrEq("moduleName"),
        Eq(false)
    )).Times(1);
    // do test4
    appMgrServiceInner->ReportUIExtensionProcColdStartToRss(abilityInfo_, want, false);

    // fill appRunningManager_
    std::string hostBundleName = "wantHostBundleName";
    std::shared_ptr<ApplicationInfo> infoHost = std::make_shared<ApplicationInfo>();
    infoHost->bundleName = hostBundleName;
    int32_t recordIdHost = RECORD_ID + 1;
    std::string processNameHost = "appRunningRecordProcessNameHost";
    auto appRunningRecordHost = std::make_shared<AppRunningRecord>(infoHost, recordIdHost, processNameHost);
    ASSERT_NE(appRunningRecordHost, nullptr);
    appRunningRecordHost->priorityObject_->pid_ = hostPid;
    appMgrServiceInner->appRunningManager_->appRunningRecordMap_.emplace(recordIdHost, appRunningRecordHost);

    // fill abilityInfo_
    abilityInfo_->extensionAbilityType = AppExecFwk::ExtensionAbilityType::SYSPICKER_PHOTOPICKER; // 404

    // EXPECT_CALL
    EXPECT_CALL(ResSchedUtil::GetInstance(), ReportUIExtensionProcColdStartToRss(
        Eq(static_cast<int32_t>(AppExecFwk::ExtensionAbilityType::SYSPICKER_PHOTOEDITOR)),
        Eq(hostPid),
        StrEq(hostBundleName),
        StrEq("bundleName"),
        StrEq("abilityName"),
        StrEq("moduleName"),
        Eq(false)
    )).Times(1);
    // do test5
    appMgrServiceInner->ReportUIExtensionProcColdStartToRss(abilityInfo_, want, false);

    // fill abilityInfo_ with not UIExtension
    abilityInfo_->extensionAbilityType = AppExecFwk::ExtensionAbilityType::BACKUP;
    // do test6
    appMgrServiceInner->ReportUIExtensionProcColdStartToRss(abilityInfo_, want, false);
}

/**
 * @tc.name: CheckLoadAbilityConditions_001
 * @tc.desc: check load ability conditions.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, CheckLoadAbilityConditions_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckLoadAbilityConditions_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());

    appMgrServiceInner->CheckLoadAbilityConditions(nullptr, nullptr, nullptr);

    appMgrServiceInner->CheckLoadAbilityConditions(nullptr, abilityInfo_, nullptr);

    appMgrServiceInner->CheckLoadAbilityConditions(nullptr, nullptr, applicationInfo_);

    appMgrServiceInner->CheckLoadAbilityConditions(token, nullptr, nullptr);

    appMgrServiceInner->CheckLoadAbilityConditions(token, abilityInfo_, nullptr);

    appMgrServiceInner->CheckLoadAbilityConditions(nullptr, abilityInfo_, applicationInfo_);

    appMgrServiceInner->CheckLoadAbilityConditions(token, nullptr, applicationInfo_);

    appMgrServiceInner->CheckLoadAbilityConditions(token, abilityInfo_, applicationInfo_);

    EXPECT_NE(appMgrServiceInner, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "CheckLoadAbilityConditions_001 end");
}

/**
 * @tc.name: MakeProcessName_001
 * @tc.desc: make process name.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, MakeProcessName_001, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "MakeProcessName_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    std::string processName = "test_processName";
    appMgrServiceInner->MakeProcessName(nullptr, nullptr, hapModuleInfo, 1, "", processName, false);
    appMgrServiceInner->MakeProcessName(nullptr, applicationInfo_, hapModuleInfo, 1, "", processName, false);
    appMgrServiceInner->MakeProcessName(abilityInfo_, nullptr, hapModuleInfo, 1, "", processName, false);
    appMgrServiceInner->MakeProcessName(abilityInfo_, applicationInfo_, hapModuleInfo, 1, "", processName, false);

    EXPECT_NE(appMgrServiceInner, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "MakeProcessName_001 end");
}

/**
 * @tc.name: MakeProcessName_002
 * @tc.desc: make process name.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, MakeProcessName_002, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "MakeProcessName_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    std::string processName = "test_processName";
    appMgrServiceInner->MakeProcessName(nullptr, hapModuleInfo, processName);
    appMgrServiceInner->MakeProcessName(applicationInfo_, hapModuleInfo, processName);
    hapModuleInfo.isStageBasedModel = false;
    hapModuleInfo.process = "";
    appMgrServiceInner->MakeProcessName(applicationInfo_, hapModuleInfo, processName);

    hapModuleInfo.isStageBasedModel = true;
    hapModuleInfo.process = "";
    appMgrServiceInner->MakeProcessName(applicationInfo_, hapModuleInfo, processName);

    hapModuleInfo.isStageBasedModel = false;
    hapModuleInfo.process = "test_process";
    appMgrServiceInner->MakeProcessName(applicationInfo_, hapModuleInfo, processName);

    hapModuleInfo.isStageBasedModel = true;
    hapModuleInfo.process = "test_process";
    appMgrServiceInner->MakeProcessName(applicationInfo_, hapModuleInfo, processName);

    hapModuleInfo.isStageBasedModel = false;
    applicationInfo_->process = "";
    appMgrServiceInner->MakeProcessName(applicationInfo_, hapModuleInfo, processName);

    hapModuleInfo.isStageBasedModel = false;
    applicationInfo_->process = "test_process";
    appMgrServiceInner->MakeProcessName(applicationInfo_, hapModuleInfo, processName);

    EXPECT_NE(appMgrServiceInner, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "MakeProcessName_002 end");
}

/**
 * @tc.name: QueryExtensionSandBox_001
 * @tc.desc: query extension sandBox without permission.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, QueryExtensionSandBox_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "QueryExtensionSandBox_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);
    const string moduleName = "entry";
    const string abilityName = "abilityName";
    BundleInfo bundleInfo;
    AppSpawnStartMsg startMsg;
    DataGroupInfoList dataGroupInfoList;
    appMgrServiceInner->QueryExtensionSandBox(moduleName, abilityName, bundleInfo, startMsg, dataGroupInfoList,
        nullptr);
    EXPECT_FALSE(startMsg.isScreenLockDataProtect);
    TAG_LOGI(AAFwkTag::TEST, "QueryExtensionSandBox_001 end");
}

/**
 * @tc.name: QueryExtensionSandBox_002
 * @tc.desc: query extension sandBox with permission.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, QueryExtensionSandBox_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "QueryExtensionSandBox_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);
    const string moduleName = "entry";
    const string abilityName = "abilityName";
    BundleInfo bundleInfo;
    bundleInfo.reqPermissions.push_back(PERMISSION_PROTECT_SCREEN_LOCK_DATA_TEST);
    AppSpawnStartMsg startMsg;
    DataGroupInfoList dataGroupInfoList;
    appMgrServiceInner->QueryExtensionSandBox(moduleName, abilityName, bundleInfo, startMsg, dataGroupInfoList,
        nullptr);
    EXPECT_TRUE(startMsg.isScreenLockDataProtect);
    TAG_LOGI(AAFwkTag::TEST, "QueryExtensionSandBox_002 end");
}

/**
 * @tc.name: GetBundleAndHapInfo_001
 * @tc.desc: get bundle and hapInfo.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, GetBundleAndHapInfo_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "GetBundleAndHapInfo_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    appMgrServiceInner->GetBundleAndHapInfo(*abilityInfo_, applicationInfo_, bundleInfo, hapModuleInfo, 1);

    appMgrServiceInner->remoteClientManager_->SetBundleManagerHelper(nullptr);
    appMgrServiceInner->GetBundleAndHapInfo(*abilityInfo_, applicationInfo_, bundleInfo, hapModuleInfo, 1);
    TAG_LOGI(AAFwkTag::TEST, "GetBundleAndHapInfo_001 end");
}

/**
 * @tc.name: AttachApplication_001
 * @tc.desc: attach application.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, AttachApplication_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AttachApplication_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->AttachApplication(0, nullptr);

    appMgrServiceInner->AttachApplication(1, nullptr);

    sptr<MockAppScheduler> mockAppScheduler = new (std::nothrow) MockAppScheduler();
    sptr<IAppScheduler> client = iface_cast<IAppScheduler>(mockAppScheduler.GetRefPtr());
    appMgrServiceInner->AttachApplication(1, client);
    TAG_LOGI(AAFwkTag::TEST, "AttachApplication_001 end");
}

/**
 * @tc.name: LaunchApplication_001
 * @tc.desc: launch application.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, LaunchApplication_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "LaunchApplication_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->LaunchApplication(nullptr);

    BundleInfo info;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, info, "");
    recordId_ += 1;
    appMgrServiceInner->LaunchApplication(appRecord);

    appRecord->SetState(ApplicationState::APP_STATE_FOREGROUND);
    appMgrServiceInner->LaunchApplication(appRecord);

    appRecord->SetState(ApplicationState::APP_STATE_CREATE);
    appMgrServiceInner->LaunchApplication(appRecord);

    appRecord->SetEmptyKeepAliveAppState(true);
    appRecord->SetKeepAliveEnableState(false);
    appRecord->SetKeepAliveDkv(false);
    appMgrServiceInner->LaunchApplication(appRecord);

    appRecord->SetKeepAliveEnableState(true);
    appRecord->SetKeepAliveDkv(true);
    appRecord->SetEmptyKeepAliveAppState(false);
    appMgrServiceInner->LaunchApplication(appRecord);

    appRecord->SetKeepAliveEnableState(true);
    appRecord->SetKeepAliveDkv(true);
    appRecord->SetEmptyKeepAliveAppState(true);
    appMgrServiceInner->LaunchApplication(appRecord);

    appRecord->SetKeepAliveEnableState(false);
    appRecord->SetKeepAliveDkv(false);
    appRecord->SetEmptyKeepAliveAppState(false);
    appMgrServiceInner->LaunchApplication(appRecord);

    Want want;
    appRecord->SetSpecifiedAbilityFlagAndWant(-1, want, "");
    appMgrServiceInner->LaunchApplication(appRecord);

    appRecord->SetSpecifiedAbilityFlagAndWant(1, want, "");
    appMgrServiceInner->LaunchApplication(appRecord);

    appMgrServiceInner->LaunchApplication(appRecord);
    TAG_LOGI(AAFwkTag::TEST, "LaunchApplication_001 end");
}

/**
 * @tc.name: AddAbilityStageDone_001
 * @tc.desc: add ability stage done.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, AddAbilityStageDone_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AddAbilityStageDone_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->AddAbilityStageDone(99);

    BundleInfo info;
    std::string processName = "test_processName";
    appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, info, "");
    recordId_ += 1;

    appMgrServiceInner->AddAbilityStageDone(recordId_);
    TAG_LOGI(AAFwkTag::TEST, "AddAbilityStageDone_001 end");
}

/**
 * @tc.name: ApplicationForegrounded_001
 * @tc.desc: application foregrounded.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, ApplicationForegrounded_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ApplicationForegrounded_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->ApplicationForegrounded(99);

    BundleInfo info;
    std::string processName = "test_processName";
    appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, info, "");
    recordId_ += 1;

    appMgrServiceInner->ApplicationForegrounded(recordId_);
    TAG_LOGI(AAFwkTag::TEST, "ApplicationForegrounded_001 end");
}

/**
 * @tc.name: ApplicationForegrounded_002
 * @tc.desc: application foregrounded.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, ApplicationForegrounded_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ApplicationForegrounded_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    BundleInfo info;
    std::string processName = "test_processName";
    auto record =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, info, "");
    recordId_ += 1;

    appMgrServiceInner->ApplicationForegrounded(recordId_);
    TAG_LOGI(AAFwkTag::TEST, "ApplicationForegrounded_002 end");
}

/**
 * @tc.name: ApplicationForegrounded_003
 * @tc.desc: application foregrounded.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, ApplicationForegrounded_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ApplicationForegrounded_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    BundleInfo info;
    std::string processName = "test_processName";
    auto record =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, info, "");
    recordId_ += 1;
    auto record2 =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, info, "");
    recordId_ += 1;
    std::shared_ptr<PriorityObject> priorityObject = std::make_shared<PriorityObject>();
    std::string callerBundleName = "callerBundleName";
    priorityObject->SetPid(1);
    record2->priorityObject_ = priorityObject;
    record2->mainBundleName_ = callerBundleName;
    record->SetCallerPid(1);

    appMgrServiceInner->ApplicationForegrounded(--recordId_);
    TAG_LOGI(AAFwkTag::TEST, "ApplicationForegrounded_003 end");
}

/**
 * @tc.name: ApplicationBackgrounded_001
 * @tc.desc: application backgrounded.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, ApplicationBackgrounded_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ApplicationBackgrounded_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    appMgrServiceInner->Init();

    appMgrServiceInner->ApplicationBackgrounded(99);

    BundleInfo info;
    std::string processName = "test_processName";
    auto appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, info, "");
    EXPECT_NE(appRecord, nullptr);
    recordId_ += 1;

    appMgrServiceInner->ApplicationBackgrounded(recordId_);

    appRecord->SetState(ApplicationState::APP_STATE_FOREGROUND);
    appMgrServiceInner->ApplicationBackgrounded(recordId_);

    TAG_LOGI(AAFwkTag::TEST, "ApplicationBackgrounded_001 end");
}

/**
 * @tc.name: ApplicationTerminated_001
 * @tc.desc: application terminated.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, ApplicationTerminated_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ApplicationTerminated_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->ApplicationTerminated(99);

    BundleInfo info;
    std::string processName = "test_processName";
    auto appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, info, "");
    EXPECT_NE(appRecord, nullptr);
    recordId_ += 1;

    appMgrServiceInner->ApplicationTerminated(recordId_);

    appRecord->SetKeepAliveEnableState(false);
    appRecord->SetKeepAliveDkv(false);
    appRecord->SetEmptyKeepAliveAppState(true);
    appMgrServiceInner->ApplicationTerminated(recordId_);

    appRecord->SetKeepAliveEnableState(true);
    appRecord->SetKeepAliveDkv(true);
    appRecord->SetEmptyKeepAliveAppState(false);
    appMgrServiceInner->ApplicationTerminated(recordId_);

    appRecord->SetKeepAliveEnableState(true);
    appRecord->SetKeepAliveDkv(true);
    appRecord->SetEmptyKeepAliveAppState(true);
    appMgrServiceInner->ApplicationTerminated(recordId_);

    appRecord->SetKeepAliveEnableState(false);
    appRecord->SetKeepAliveDkv(false);
    appRecord->SetEmptyKeepAliveAppState(false);
    appMgrServiceInner->ApplicationTerminated(recordId_);

    appRecord->SetState(ApplicationState::APP_STATE_FOREGROUND);
    appMgrServiceInner->ApplicationTerminated(recordId_);

    appRecord->SetState(ApplicationState::APP_STATE_BACKGROUND);
    appMgrServiceInner->ApplicationTerminated(recordId_);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->ApplicationTerminated(recordId_);

    TAG_LOGI(AAFwkTag::TEST, "ApplicationTerminated_001 end");
}

/**
 * @tc.name: KillApplication_001
 * @tc.desc: kill application.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, KillApplication_001, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "KillApplication_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "test_bundleName";
    appMgrServiceInner->KillApplication(bundleName);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->KillApplication(bundleName);

    TAG_LOGI(AAFwkTag::TEST, "KillApplication_001 end");
}

/**
 * @tc.name: KillApplicationByUid_001
 * @tc.desc: kill application by uid.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, KillApplicationByUid_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillApplicationByUid_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "test_bundleName";
    appMgrServiceInner->KillApplicationByUid(bundleName, 0);

    appMgrServiceInner->remoteClientManager_->SetBundleManagerHelper(nullptr);
    appMgrServiceInner->KillApplicationByUid(bundleName, 0);

    appMgrServiceInner->remoteClientManager_ = nullptr;
    appMgrServiceInner->KillApplicationByUid(bundleName, 0);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->KillApplicationByUid(bundleName, 0);

    TAG_LOGI(AAFwkTag::TEST, "KillApplicationByUid_001 end");
}

/**
 * @tc.name: KillApplicationSelf_001
 * @tc.desc: kill application self.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, KillApplicationSelf_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillApplicationSelf_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    EXPECT_EQ(appMgrServiceInner->KillApplicationSelf(), ERR_INVALID_VALUE);

    appMgrServiceInner->appRunningManager_ = nullptr;
    EXPECT_EQ(appMgrServiceInner->KillApplicationSelf(), ERR_NO_INIT);

    TAG_LOGI(AAFwkTag::TEST, "KillApplicationSelf_001 end");
}

/**
 * @tc.name: KillApplicationByUserId_001
 * @tc.desc: kill application by user id.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, KillApplicationByUserId_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillApplicationByUserId_001 start");
    AAFwk::IsMockSaCall::IsMockSpecificSystemAbilityAccessPermission();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "test_bundleName";
    int result = appMgrServiceInner->KillApplicationByUserId(bundleName, 0, 0);
    EXPECT_NE(result, 0);

    appMgrServiceInner->remoteClientManager_->SetBundleManagerHelper(nullptr);
    appMgrServiceInner->KillApplicationByUserId(bundleName, 0, 0);
    EXPECT_NE(result, 0);

    appMgrServiceInner->remoteClientManager_ = nullptr;
    appMgrServiceInner->KillApplicationByUserId(bundleName, 0, 0);
    EXPECT_NE(result, 0);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->KillApplicationByUserId(bundleName, 0, 0);
    EXPECT_NE(result, 0);

    TAG_LOGI(AAFwkTag::TEST, "KillApplicationByUserId_001 end");
}

/**
 * @tc.name: KillApplicationByUserIdLocked_001
 * @tc.desc: kill application by user id locked.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, KillApplicationByUserIdLocked_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillApplicationByUserIdLocked_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "test_bundleName";
    int result = appMgrServiceInner->KillApplicationByUserIdLocked(bundleName, 0, 0);
    EXPECT_EQ(result, 0);

    appMgrServiceInner->remoteClientManager_->SetBundleManagerHelper(nullptr);
    appMgrServiceInner->KillApplicationByUserIdLocked(bundleName, 0, 0);
    EXPECT_EQ(result, 0);

    appMgrServiceInner->remoteClientManager_ = nullptr;
    appMgrServiceInner->KillApplicationByUserIdLocked(bundleName, 0, 0);
    EXPECT_EQ(result, 0);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->KillApplicationByUserIdLocked(bundleName, 0, 0);
    EXPECT_EQ(result, 0);

    TAG_LOGI(AAFwkTag::TEST, "KillApplicationByUserIdLocked_001 end");
}

/**
 * @tc.name: ClearUpApplicationData_001
 * @tc.desc: clear up application data.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, ClearUpApplicationData_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClearUpApplicationData_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "test_bundleName";
    appMgrServiceInner->ClearUpApplicationData(bundleName, 0, 0, 0);

    TAG_LOGI(AAFwkTag::TEST, "ClearUpApplicationData_001 end");
}

/**
 * @tc.name: ClearUpApplicationDataByUserId_001
 * @tc.desc: clear up application data by user id.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, ClearUpApplicationDataByUserId_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClearUpApplicationDataByUserId_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "test_bundleName";
    appMgrServiceInner->ClearUpApplicationDataByUserId(bundleName, 0, 0, 0, 0);
    appMgrServiceInner->ClearUpApplicationDataByUserId(bundleName, 1, 0, 0, 0);
    appMgrServiceInner->ClearUpApplicationDataByUserId(bundleName, 1, 1, 0, 0);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->ClearUpApplicationDataByUserId(bundleName, 1, 1, 0, 0);

    appMgrServiceInner->remoteClientManager_->SetBundleManagerHelper(nullptr);
    appMgrServiceInner->ClearUpApplicationDataByUserId(bundleName, 1, 1, 0, 0);

    TAG_LOGI(AAFwkTag::TEST, "ClearUpApplicationDataByUserId_001 end");
}

/**
 * @tc.name: GetAllRunningProcesses_001
 * @tc.desc: get all running processes.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, GetAllRunningProcesses_001, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningProcesses_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::vector<RunningProcessInfo> info;
    appMgrServiceInner->GetAllRunningProcesses(info);

    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningProcesses_001 end");
}

/**
 * @tc.name: GetProcessRunningInfosByUserId_001
 * @tc.desc: get process running infos by user id.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, GetProcessRunningInfosByUserId_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetProcessRunningInfosByUserId_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::vector<RunningProcessInfo> info;
    appMgrServiceInner->GetProcessRunningInfosByUserId(info, 0);

    TAG_LOGI(AAFwkTag::TEST, "GetProcessRunningInfosByUserId_001 end");
}

/**
 * @tc.name: GetAllRenderProcesses_001
 * @tc.desc: get all render processes.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, GetAllRenderProcesses_001, TestSize.Level1)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::vector<RenderProcessInfo> info;
    appMgrServiceInner->GetAllRenderProcesses(info);
}

#ifdef SUPPORT_CHILD_PROCESS
/**
 * @tc.name: GetAllChildrenProcesses_001
 * @tc.desc: get all children processes.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, GetAllChildrenProcesses_001, TestSize.Level1)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::vector<ChildProcessInfo> info;
    auto result = appMgrServiceInner->GetAllChildrenProcesses(info);
    EXPECT_EQ(result, ERR_OK);
}
#endif // SUPPORT_CHILD_PROCESS

/**
 * @tc.name: NotifyMemoryLevel_001
 * @tc.desc: notify memory level.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, NotifyMemoryLevel_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyMemoryLevel_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::vector<RunningProcessInfo> info;
    int result = appMgrServiceInner->NotifyMemoryLevel(0);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    result = appMgrServiceInner->NotifyMemoryLevel(1);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    result = appMgrServiceInner->NotifyMemoryLevel(2);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    result = appMgrServiceInner->NotifyMemoryLevel(3);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    appMgrServiceInner->appRunningManager_ = nullptr;
    result = appMgrServiceInner->NotifyMemoryLevel(3);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "NotifyMemoryLevel_001 end");
}

/**
 * @tc.name: KillProcessByPid_001
 * @tc.desc: kill process by pid.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, KillProcessByPid_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "KillProcessByPid_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    int result = appMgrServiceInner->KillProcessByPid(0, "KillProcessByPid_001");
    EXPECT_EQ(result, AAFwk::ERR_KILL_PROCESS_NOT_EXIST);

    result = appMgrServiceInner->KillProcessByPid(1, "KillProcessByPid_001");
    EXPECT_EQ(result, 0);

    TAG_LOGI(AAFwkTag::TEST, "KillProcessByPid_001 end");
}

/**
 * @tc.name: KillProcessByPid_002
 * @tc.desc: kill process by pid.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, KillProcessByPid_002, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "KillProcessByPid_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    int pid = 0;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        std::make_shared<AppRunningRecord>(applicationInfo_, ++recordId_, processName);
    auto appRunningManager = std::make_shared<AppRunningManager>();
    auto priorityObject = std::make_shared<PriorityObject>();
    priorityObject->SetPid(0);
    appRecord->priorityObject_ = priorityObject;
    appRunningManager->appRunningRecordMap_.emplace(recordId_, appRecord);

    int result = appMgrServiceInner->KillProcessByPid(pid, "KillProcessByPid_002");
    EXPECT_EQ(result, AAFwk::ERR_KILL_PROCESS_NOT_EXIST);

    TAG_LOGI(AAFwkTag::TEST, "KillProcessByPid_002 end");
}

/**
 * @tc.name: KillProcessByPidInner_001
 * @tc.desc: kill process by pid.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, KillProcessByPidInner_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "KillProcessByPidInner_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);

    pid_t pid = 99999999;
    auto ret = appMgrServiceInner->KillProcessByPidInner(pid, "ReasonTdd", "KillReasonTdd", nullptr, false);
    EXPECT_EQ(ret, AAFwk::ERR_KILL_PROCESS_NOT_EXIST);
    TAG_LOGI(AAFwkTag::TEST, "KillProcessByPidInner_001 end");
}

/**
 * @tc.name: AddToKillProcessMap_001
 * @tc.desc: add to kill process map.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, AddToKillProcessMap_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AddToKillProcessMap_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->killedProcessMap_.clear();
    appMgrServiceInner->AddToKillProcessMap("com.test.demo");
    EXPECT_EQ(appMgrServiceInner->killedProcessMap_.size(), 1);
    TAG_LOGI(AAFwkTag::TEST, "AddToKillProcessMap_001 end");
}

/**
 * @tc.name: ProcessExist_001
 * @tc.desc: process exist.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, ProcessExist_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ProcessExist_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t pid = 0;
    bool result = ProcessUtil::ProcessExist(pid);
    EXPECT_FALSE(result);

    TAG_LOGI(AAFwkTag::TEST, "ProcessExist_001 end");
}

/**
 * @tc.name: CreateAppRunningRecord_001
 * @tc.desc: create app running record.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, CreateAppRunningRecord_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateAppRunningRecord_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    std::shared_ptr<AAFwk::Want> want;
    std::string processName = "test_processName";

    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    std::shared_ptr<AppRunningRecord> appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam,
        nullptr, nullptr, "", bundleInfo, hapModuleInfo, nullptr);
    EXPECT_EQ(appRecord, nullptr);

    loadParam->token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam,
        nullptr, nullptr, "", bundleInfo, hapModuleInfo, nullptr);
    EXPECT_EQ(appRecord, nullptr);

    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam,
        applicationInfo_, nullptr, "", bundleInfo, hapModuleInfo, nullptr);
    EXPECT_EQ(appRecord, nullptr);

    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam,
        applicationInfo_, abilityInfo_, "", bundleInfo, hapModuleInfo, nullptr);
    EXPECT_EQ(appRecord, nullptr);

    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam,
        applicationInfo_, abilityInfo_, processName, bundleInfo, hapModuleInfo, nullptr);
    EXPECT_NE(appRecord, nullptr);

    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam,
        applicationInfo_, abilityInfo_, processName, bundleInfo, hapModuleInfo, nullptr);
    EXPECT_NE(appRecord, nullptr);

    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam,
        applicationInfo_, abilityInfo_, processName, bundleInfo, hapModuleInfo, nullptr);
    EXPECT_NE(appRecord, nullptr);

    std::shared_ptr<AppRunningRecord> appRecord1 = appMgrServiceInner->CreateAppRunningRecord(loadParam,
        nullptr, abilityInfo_, processName, bundleInfo, hapModuleInfo, want);
    EXPECT_EQ(appRecord1, nullptr);

    std::shared_ptr<AppRunningRecord> appRecord2 = appMgrServiceInner->CreateAppRunningRecord(loadParam,
        applicationInfo_, abilityInfo_, processName, bundleInfo, hapModuleInfo, want);
    EXPECT_NE(appRecord2, nullptr);

    want = std::make_shared<Want>();
    const std::string COLD_START = "coldStart";
    want->SetParam(COLD_START, true);
    std::shared_ptr<AppRunningRecord> appRecord3 = appMgrServiceInner->CreateAppRunningRecord(loadParam,
        applicationInfo_, abilityInfo_, processName, bundleInfo, hapModuleInfo, want);
    EXPECT_NE(appRecord3, nullptr);

    want->SetParam(COLD_START, false);
    std::shared_ptr<AppRunningRecord> appRecord4 = appMgrServiceInner->CreateAppRunningRecord(loadParam,
        applicationInfo_, abilityInfo_, processName, bundleInfo, hapModuleInfo, want);
    EXPECT_NE(appRecord4, nullptr);

    appMgrServiceInner->appRunningManager_ = nullptr;
    std::shared_ptr<AppRunningRecord> appRecord5 = appMgrServiceInner->CreateAppRunningRecord(loadParam,
        applicationInfo_, abilityInfo_, processName, bundleInfo, hapModuleInfo, want);
    EXPECT_EQ(appRecord5, nullptr);

    appMgrServiceInner->appRunningManager_ = nullptr;
    want->SetParam("multiThread", false);
    std::shared_ptr<AppRunningRecord> appRecord6 = appMgrServiceInner->CreateAppRunningRecord(loadParam,
        applicationInfo_, abilityInfo_, processName, bundleInfo, hapModuleInfo, want);
    EXPECT_EQ(appRecord6, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "CreateAppRunningRecord_001 end");
}

/**
 * @tc.name: TerminateAbility_001
 * @tc.desc: terminate ability.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, TerminateAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TerminateAbility_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->TerminateAbility(nullptr, true);
    appMgrServiceInner->TerminateAbility(nullptr, false);

    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    appMgrServiceInner->TerminateAbility(token, true);
    appMgrServiceInner->TerminateAbility(token, false);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->TerminateAbility(token, true);

    TAG_LOGI(AAFwkTag::TEST, "TerminateAbility_001 end");
}

/**
 * @tc.name: UpdateAbilityState_001
 * @tc.desc: update ability state.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, UpdateAbilityState_001, TestSize.Level1)
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
    appMgrServiceInner->UpdateAbilityState(token1, AbilityState::ABILITY_STATE_CREATE);

    auto abilityRecord1 =
        appMgrServiceInner->GetAppRunningRecordByAbilityToken(token1)->GetAbilityRunningRecordByToken(token1);
    abilityRecord1->SetState(AbilityState::ABILITY_STATE_TERMINATED);
    appMgrServiceInner->UpdateAbilityState(token1, AbilityState::ABILITY_STATE_TERMINATED);

    abilityRecord1->SetState(AbilityState::ABILITY_STATE_CONNECTED);
    appMgrServiceInner->UpdateAbilityState(token1, AbilityState::ABILITY_STATE_CONNECTED);

    abilityRecord1->SetState(AbilityState::ABILITY_STATE_DISCONNECTED);
    appMgrServiceInner->UpdateAbilityState(token1, AbilityState::ABILITY_STATE_DISCONNECTED);

    abilityRecord1->SetState(AbilityState::ABILITY_STATE_END);
    appMgrServiceInner->UpdateAbilityState(token1, AbilityState::ABILITY_STATE_END);

    abilityRecord1->SetState(AbilityState::ABILITY_STATE_BACKGROUND);
    appMgrServiceInner->UpdateAbilityState(token1, AbilityState::ABILITY_STATE_BACKGROUND);

    OHOS::sptr<IRemoteObject> token2 = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    abilityInfo_->type = AbilityType::SERVICE;
    loadParam->token = token2;
    std::shared_ptr<AppRunningRecord> appRecord2 = appMgrServiceInner->CreateAppRunningRecord(loadParam,
        applicationInfo_, abilityInfo_, processName, bundleInfo, hapModuleInfo, want);
    EXPECT_NE(appRecord2, nullptr);
    appMgrServiceInner->UpdateAbilityState(token2, AbilityState::ABILITY_STATE_CREATE);

    auto abilityRecord2 =
        appMgrServiceInner->GetAppRunningRecordByAbilityToken(token2)->GetAbilityRunningRecordByToken(token2);
    abilityRecord2->SetState(AbilityState::ABILITY_STATE_TERMINATED);
    appMgrServiceInner->UpdateAbilityState(token2, AbilityState::ABILITY_STATE_TERMINATED);

    abilityRecord2->SetState(AbilityState::ABILITY_STATE_CONNECTED);
    appMgrServiceInner->UpdateAbilityState(token2, AbilityState::ABILITY_STATE_CONNECTED);

    abilityRecord2->SetState(AbilityState::ABILITY_STATE_DISCONNECTED);
    appMgrServiceInner->UpdateAbilityState(token2, AbilityState::ABILITY_STATE_DISCONNECTED);

    abilityRecord2->SetState(AbilityState::ABILITY_STATE_END);
    appMgrServiceInner->UpdateAbilityState(token2, AbilityState::ABILITY_STATE_END);

    abilityRecord2->SetState(AbilityState::ABILITY_STATE_BACKGROUND);
    appMgrServiceInner->UpdateAbilityState(token2, AbilityState::ABILITY_STATE_BACKGROUND);

    TAG_LOGI(AAFwkTag::TEST, "UpdateAbilityState_001 end");
}

/**
 * @tc.name: UpdateExtensionState_001
 * @tc.desc: update extension state.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, UpdateExtensionState_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateExtensionState_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->UpdateExtensionState(nullptr, ExtensionState::EXTENSION_STATE_CREATE);

    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    appMgrServiceInner->UpdateExtensionState(token, ExtensionState::EXTENSION_STATE_CREATE);

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    std::shared_ptr<AAFwk::Want> want;
    std::string processName = "test_processName";
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token;
    std::shared_ptr<AppRunningRecord> appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam,
        applicationInfo_, abilityInfo_, processName, bundleInfo, hapModuleInfo, want);
    EXPECT_NE(appRecord, nullptr);
    appMgrServiceInner->UpdateExtensionState(token, ExtensionState::EXTENSION_STATE_CREATE);

    TAG_LOGI(AAFwkTag::TEST, "UpdateExtensionState_001 end");
}

/**
 * @tc.name: OpenAppSpawnConnection_001
 * @tc.desc: open app spawn connection.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, OpenAppSpawnConnection_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "OpenAppSpawnConnection_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->remoteClientManager_->SetSpawnClient(nullptr);
    auto errorCode = appMgrServiceInner->OpenAppSpawnConnection();
    EXPECT_EQ(errorCode, ERR_APPEXECFWK_BAD_APPSPAWN_CLIENT);

    appMgrServiceInner->remoteClientManager_ = nullptr;
    auto errorCode1 = appMgrServiceInner->OpenAppSpawnConnection();
    EXPECT_EQ(errorCode1, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "OpenAppSpawnConnection_001 end");
}

/**
 * @tc.name: CloseAppSpawnConnection_001
 * @tc.desc: close app spawn connection.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, CloseAppSpawnConnection_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "CloseAppSpawnConnection_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->CloseAppSpawnConnection();

    appMgrServiceInner->remoteClientManager_ = nullptr;
    appMgrServiceInner->CloseAppSpawnConnection();

    TAG_LOGI(AAFwkTag::TEST, "CloseAppSpawnConnection_001 end");
}

/**
 * @tc.name: QueryAppSpawnConnectionState_001
 * @tc.desc: query app spawn connection.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, QueryAppSpawnConnectionState_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "QueryAppSpawnConnectionState_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    auto connectionState = appMgrServiceInner->QueryAppSpawnConnectionState();
    EXPECT_EQ(connectionState, SpawnConnectionState::STATE_NOT_CONNECT);

    appMgrServiceInner->remoteClientManager_->SetSpawnClient(nullptr);
    connectionState = appMgrServiceInner->QueryAppSpawnConnectionState();
    EXPECT_EQ(connectionState, SpawnConnectionState::STATE_NOT_CONNECT);


    appMgrServiceInner->remoteClientManager_ = nullptr;
    connectionState = appMgrServiceInner->QueryAppSpawnConnectionState();
    EXPECT_EQ(connectionState, SpawnConnectionState::STATE_NOT_CONNECT);

    TAG_LOGI(AAFwkTag::TEST, "QueryAppSpawnConnectionState_001 end");
}

/**
 * @tc.name: SetAppSpawnClient_001
 * @tc.desc: set app spawn client.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, SetAppSpawnClient_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetAppSpawnClient_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::shared_ptr<AppSpawnClient> spawnClient;
    appMgrServiceInner->SetAppSpawnClient(spawnClient);

    appMgrServiceInner->remoteClientManager_ = nullptr;
    appMgrServiceInner->SetAppSpawnClient(spawnClient);

    TAG_LOGI(AAFwkTag::TEST, "SetAppSpawnClient_001 end");
}

/**
 * @tc.name: SetBundleManager_001
 * @tc.desc: set bundle manager.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, SetBundleManager_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "SetBundleManager_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::shared_ptr<BundleMgrHelper> bundleManager;
    appMgrServiceInner->SetBundleManagerHelper(bundleManager);

    appMgrServiceInner->remoteClientManager_ = nullptr;
    appMgrServiceInner->SetBundleManagerHelper(bundleManager);

    TAG_LOGI(AAFwkTag::TEST, "SetBundleManager_001 end");
}

/**
 * @tc.name: RegisterAppStateCallback_001
 * @tc.desc: register app state call back.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, RegisterAppStateCallback_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RegisterAppStateCallback_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->RegisterAppStateCallback(nullptr);

    sptr<IAppStateCallback> callback;
    appMgrServiceInner->RegisterAppStateCallback(callback);

    TAG_LOGI(AAFwkTag::TEST, "RegisterAppStateCallback_001 end");
}

/**
 * @tc.name: KillProcessByAbilityToken_001
 * @tc.desc: kill process by ability token.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, KillProcessByAbilityToken_001, TestSize.Level1)
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

    appRecord->SetKeepAliveEnableState(true);
    appRecord->SetKeepAliveDkv(true);
    appRecord->SetEmptyKeepAliveAppState(true);
    appMgrServiceInner->KillProcessByAbilityToken(token);

    TAG_LOGI(AAFwkTag::TEST, "KillProcessByAbilityToken_001 end");
}

/**
 * @tc.name: KillProcessesByUserId_001
 * @tc.desc: kill process by user id.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, KillProcessesByUserId_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillProcessesByUserId_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->KillProcessesByUserId(0, false, nullptr);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->KillProcessesByUserId(0, false, nullptr);

    appMgrServiceInner->KillProcessesByUserId(0, true, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "KillProcessesByUserId_001 end");
}

/**
 * @tc.name: WaitProcessesExitAndKill_001
 * @tc.desc: kill process by user id.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
 HWTEST_F(AppMgrServiceInnerTest, WaitProcessesExitAndKill_001, TestSize.Level1)
 {
     TAG_LOGI(AAFwkTag::TEST, "WaitProcessesExitAndKill_001 start");
     auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
     EXPECT_NE(appMgrServiceInner, nullptr);
     sptr<AAFwk::IUserCallback> callback = new MockIUserCallback();
     std::list<SimpleProcessInfo> processInfos;
     int64_t startTime = 0;
     std::string reason = "KillProcessesByUserId";
     int32_t userId = 0;
     int ret = appMgrServiceInner->WaitProcessesExitAndKill(processInfos, startTime, reason, userId, callback);
     EXPECT_EQ(ret, ERR_OK);
     TAG_LOGI(AAFwkTag::TEST, "WaitProcessesExitAndKill_001 end");
 }

 /**
 * @tc.name: WaitProcessesExitAndKill_002
 * @tc.desc: kill process by user id.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
 HWTEST_F(AppMgrServiceInnerTest, WaitProcessesExitAndKill_002, TestSize.Level1)
 {
     TAG_LOGI(AAFwkTag::TEST, "WaitProcessesExitAndKill_002 start");
     auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
     EXPECT_NE(appMgrServiceInner, nullptr);
     std::list<SimpleProcessInfo> processInfos;
     int64_t startTime = 0;
     std::string reason = "KillProcessesByUserId";
     int32_t userId = 0;
     int ret = appMgrServiceInner->WaitProcessesExitAndKill(processInfos, startTime, reason, userId, nullptr);
     EXPECT_EQ(ret, ERR_OK);
     TAG_LOGI(AAFwkTag::TEST, "WaitProcessesExitAndKill_001 end");
 }

/**
 * @tc.name: StartAbility_001
 * @tc.desc: start ability.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbility_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    HapModuleInfo hapModuleInfo;
    std::shared_ptr<AAFwk::Want> want;
    std::shared_ptr<AppRunningRecord> appRecord;
    appMgrServiceInner->StartAbility(nullptr, nullptr, abilityInfo_, nullptr, hapModuleInfo, nullptr, 0);
    appMgrServiceInner->StartAbility(nullptr, nullptr, abilityInfo_, appRecord, hapModuleInfo, nullptr, 0);
    appMgrServiceInner->StartAbility(nullptr, nullptr, abilityInfo_, appRecord, hapModuleInfo, want, 0);

    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    OHOS::sptr<IRemoteObject> preToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    appMgrServiceInner->StartAbility(token, nullptr, abilityInfo_, appRecord, hapModuleInfo, want, 0);
    appMgrServiceInner->StartAbility(nullptr, preToken, abilityInfo_, appRecord, hapModuleInfo, want, 0);
    appMgrServiceInner->StartAbility(token, preToken, abilityInfo_, appRecord, hapModuleInfo, want, 0);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token;
    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam,
        applicationInfo_, abilityInfo_, processName, bundleInfo, hapModuleInfo, want);
    EXPECT_NE(appRecord, nullptr);
    appMgrServiceInner->StartAbility(token, nullptr, abilityInfo_, appRecord, hapModuleInfo, want, 0);
    appMgrServiceInner->StartAbility(token, preToken, abilityInfo_, appRecord, hapModuleInfo, want, 0);

    abilityInfo_->applicationInfo.name = "hiservcie";
    abilityInfo_->applicationInfo.bundleName = "com.ix.hiservcie";
    appMgrServiceInner->StartAbility(token, preToken, abilityInfo_, appRecord, hapModuleInfo, want, 0);

    TAG_LOGI(AAFwkTag::TEST, "StartAbility_001 end");
}

/**
 * @tc.name: GetAppRunningRecordByAbilityToken_001
 * @tc.desc: get app running record by ability token.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, GetAppRunningRecordByAbilityToken_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAppRunningRecordByAbilityToken_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    appMgrServiceInner->GetAppRunningRecordByAbilityToken(token);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->GetAppRunningRecordByAbilityToken(token);

    TAG_LOGI(AAFwkTag::TEST, "GetAppRunningRecordByAbilityToken_001 end");
}

/**
 * @tc.name: AbilityTerminated_001
 * @tc.desc: ability terminated.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, AbilityTerminated_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityTerminated_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->AbilityTerminated(nullptr);

    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    appMgrServiceInner->AbilityTerminated(token);

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    std::shared_ptr<AAFwk::Want> want;
    std::string processName = "test_processName";
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token;
    std::shared_ptr<AppRunningRecord> appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam,
        applicationInfo_, abilityInfo_, processName, bundleInfo, hapModuleInfo, want);
    EXPECT_NE(appRecord, nullptr);
    appMgrServiceInner->AbilityTerminated(token);

    TAG_LOGI(AAFwkTag::TEST, "AbilityTerminated_001 end");
}

/**
 * @tc.name: GetAppRunningRecordByAppRecordId_001
 * @tc.desc: get app running record by app record id.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, GetAppRunningRecordByAppRecordId_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAppRunningRecordByAppRecordId_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->GetAppRunningRecordByAppRecordId(0);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->GetAppRunningRecordByAppRecordId(0);

    TAG_LOGI(AAFwkTag::TEST, "GetAppRunningRecordByAppRecordId_001 end");
}

/**
 * @tc.name: OnAppStateChanged_001
 * @tc.desc: on app state changed.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, OnAppStateChanged_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "OnAppStateChanged_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->OnAppStateChanged(nullptr, ApplicationState::APP_STATE_CREATE, true, false, false);
    appMgrServiceInner->OnAppStateChanged(nullptr, ApplicationState::APP_STATE_CREATE, false, false, false);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo, "");
    EXPECT_NE(appRecord, nullptr);
    appMgrServiceInner->OnAppStateChanged(appRecord, ApplicationState::APP_STATE_CREATE, true, false, false);

    sptr<MockAppStateCallback> mockCallback(new MockAppStateCallback());
    EXPECT_CALL(*mockCallback, OnAppStateChanged(_)).Times(2);
    sptr<IAppStateCallback> callback1 = iface_cast<IAppStateCallback>(mockCallback);
    appMgrServiceInner->appStateCallbacks_.push_back({ callback1, 100 });
    appMgrServiceInner->OnAppStateChanged(appRecord, ApplicationState::APP_STATE_CREATE, true, false, false);

    sptr<IAppStateCallback> callback;
    appMgrServiceInner->appStateCallbacks_.push_back({ callback, 100 });
    appMgrServiceInner->OnAppStateChanged(appRecord, ApplicationState::APP_STATE_CREATE, true, false, false);

    TAG_LOGI(AAFwkTag::TEST, "OnAppStateChanged_001 end");
}

/**
 * @tc.name: OnAbilityStateChanged_001
 * @tc.desc: on ability state changed.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, OnAbilityStateChanged_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "OnAbilityStateChanged_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->OnAbilityStateChanged(nullptr, AbilityState::ABILITY_STATE_CREATE);

    sptr<IRemoteObject> token = new MockAbilityToken();
    std::shared_ptr<AbilityRunningRecord> abilityRunningRecord =
        std::make_shared<AbilityRunningRecord>(abilityInfo_, token, 0);
    appMgrServiceInner->OnAbilityStateChanged(abilityRunningRecord, AbilityState::ABILITY_STATE_CREATE);

    sptr<MockAppStateCallback> mockCallback(new MockAppStateCallback());
    EXPECT_CALL(*mockCallback, OnAbilityRequestDone(_, _)).Times(2);
    sptr<IAppStateCallback> callback1 = iface_cast<IAppStateCallback>(mockCallback);
    appMgrServiceInner->appStateCallbacks_.push_back({ callback1, 100 });
    appMgrServiceInner->OnAbilityStateChanged(abilityRunningRecord, AbilityState::ABILITY_STATE_CREATE);

    sptr<IAppStateCallback> callback;
    appMgrServiceInner->appStateCallbacks_.push_back({ callback, 100 });
    appMgrServiceInner->OnAbilityStateChanged(abilityRunningRecord, AbilityState::ABILITY_STATE_CREATE);

    TAG_LOGI(AAFwkTag::TEST, "OnAbilityStateChanged_001 end");
}

/**
 * @tc.name: StartProcess_001
 * @tc.desc: start process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartProcess_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartProcess_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    BundleInfo bundleInfo;
    std::string appName = "test_appName";
    std::string processName = "test_processName";
    std::string bundleName = "test_bundleName";
    sptr<IRemoteObject> token = new MockAbilityToken();
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo, "");
    EXPECT_NE(appRecord, nullptr);
    appMgrServiceInner->StartProcess(appName, processName, 0, nullptr, 0, bundleInfo, bundleName, 0);
    appMgrServiceInner->StartProcess(appName, processName, 0, appRecord, 0, bundleInfo, bundleName, 0);

    TAG_LOGI(AAFwkTag::TEST, "StartProcess_001 end");
}

/**
 * @tc.name: StartProcess_null_spawnclient_001
 * @tc.desc: start process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartProcess_null_spawnclient_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartProcess_null_spawnclient_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    BundleInfo bundleInfo;
    std::string appName = "test_appName";
    std::string processName = "test_processName";
    std::string bundleName = "test_bundleName";
    sptr<IRemoteObject> token = new MockAbilityToken();
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo, "");
    EXPECT_NE(appRecord, nullptr);
    appMgrServiceInner->remoteClientManager_ = nullptr;
    appMgrServiceInner->StartProcess(appName, processName, 0, nullptr, 0, bundleInfo, bundleName, 0);
    uint32_t ret = appMgrServiceInner->StartProcess(appName, processName, 0, appRecord, 0, bundleInfo, bundleName, 0);
    EXPECT_EQ(ret, AAFwk::ERR_GET_SPAWN_CLIENT_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "StartProcess_null_spawnclient_001 end");
}

/**
 * @tc.name: OnRemoteDied_001
 * @tc.desc: on remote died.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, OnRemoteDied_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "OnRemoteDied_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    sptr<IRemoteObject> remoteObject;
    appMgrServiceInner->OnRemoteDied(remoteObject, true);
    appMgrServiceInner->OnRemoteDied(remoteObject, false);

    TAG_LOGI(AAFwkTag::TEST, "OnRemoteDied_001 end");
}

/**
 * @tc.name: ClearAppRunningData_001
 * @tc.desc: clear app running data.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, ClearAppRunningData_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ClearAppRunningData_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->ClearAppRunningData(nullptr);
    TAG_LOGI(AAFwkTag::TEST, "ClearAppRunningData_001 end");
}

/**
 * @tc.name: ClearAppRunningData_002
 * @tc.desc: clear app running data.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, ClearAppRunningData_002, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ClearAppRunningData_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    BundleInfo info;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, info, "");
    appMgrServiceInner->ClearAppRunningData(appRecord);
    TAG_LOGI(AAFwkTag::TEST, "ClearAppRunningData_002 end");
}

/**
 * @tc.name: HandleTimeOut_001
 * @tc.desc: handle time out.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, HandleTimeOut_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleTimeOut_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    AAFwk::EventWrap innerEvent(0);
    appMgrServiceInner->HandleTimeOutInner(innerEvent);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->HandleTimeOutInner(innerEvent);

    TAG_LOGI(AAFwkTag::TEST, "HandleTimeOut_001 end");
}

/**
 * @tc.name: HandleAbilityAttachTimeOut_001
 * @tc.desc: handle ability attach time out.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, HandleAbilityAttachTimeOut_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleAbilityAttachTimeOut_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->HandleAbilityAttachTimeOut(nullptr);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->HandleAbilityAttachTimeOut(nullptr);

    TAG_LOGI(AAFwkTag::TEST, "HandleAbilityAttachTimeOut_001 end");
}

/**
 * @tc.name: PrepareTerminate_001
 * @tc.desc: prepare terminate.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, PrepareTerminate_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "PrepareTerminate_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->PrepareTerminate(nullptr);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->PrepareTerminate(nullptr);

    TAG_LOGI(AAFwkTag::TEST, "PrepareTerminate_001 end");
}

/**
 * @tc.name: HandleTerminateApplicationTimeOut_001
 * @tc.desc: handle terminate application time out.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, HandleTerminateApplicationTimeOut_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleTerminateApplicationTimeOut_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->HandleTerminateApplicationTimeOut(nullptr);

    BundleInfo bundleInfo;
    std::string appName = "test_appName";
    std::string processName = "test_processName";
    std::string bundleName = "test_bundleName";
    sptr<IRemoteObject> token = new MockAbilityToken();
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo, "");
    EXPECT_NE(appRecord, nullptr);

    pid_t pid = 1;
    appRecord->GetPriorityObject()->SetPid(pid);
    appMgrServiceInner->HandleTerminateApplicationTimeOut(appRecord);

    appMgrServiceInner->taskHandler_ = nullptr;
    appMgrServiceInner->HandleTerminateApplicationTimeOut(appRecord);

    TAG_LOGI(AAFwkTag::TEST, "HandleTerminateApplicationTimeOut_001 end");
}

/**
 * @tc.name: HandleAddAbilityStageTimeOut_001
 * @tc.desc: handle add ability stage time out.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, HandleAddAbilityStageTimeOut_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleAddAbilityStageTimeOut_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->HandleAddAbilityStageTimeOut(nullptr);

    BundleInfo bundleInfo;
    std::string appName = "test_appName";
    std::string processName = "test_processName";
    std::string bundleName = "test_bundleName";
    sptr<IRemoteObject> token = new MockAbilityToken();
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo, "");
    EXPECT_NE(appRecord, nullptr);

    appRecord->specifiedAbilityRequest_ = std::make_shared<SpecifiedRequest>();
    appRecord->specifiedAbilityRequest_->requestId = 1;
    appMgrServiceInner->HandleAddAbilityStageTimeOut(appRecord);

    sptr<IStartSpecifiedAbilityResponse> response;
    appMgrServiceInner->startSpecifiedAbilityResponse_ = response;
    appMgrServiceInner->HandleAddAbilityStageTimeOut(appRecord);

    TAG_LOGI(AAFwkTag::TEST, "HandleAddAbilityStageTimeOut_001 end");
}

/**
 * @tc.name: GetRunningProcessInfoByToken_001
 * @tc.desc: get running process info by token.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, GetRunningProcessInfoByToken_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcessInfoByToken_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    AppExecFwk::RunningProcessInfo info;
    appMgrServiceInner->GetRunningProcessInfoByToken(nullptr, info);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->GetRunningProcessInfoByToken(nullptr, info);

    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcessInfoByToken_001 end");
}

/**
 * @tc.name: GetRunningProcessInfoByPid_001
 * @tc.desc: get running process info by pid.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, GetRunningProcessInfoByPid_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcessInfoByPid_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    AppExecFwk::RunningProcessInfo info;
    appMgrServiceInner->GetRunningProcessInfoByPid(0, info);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->GetRunningProcessInfoByPid(0, info);

    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcessInfoByPid_001 end");
}

/**
 * @tc.name: CheckGetRunningInfoPermission_001
 * @tc.desc: check get running info permission.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, CheckGetRunningInfoPermission_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckGetRunningInfoPermission_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->CheckGetRunningInfoPermission();

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->CheckGetRunningInfoPermission();

    TAG_LOGI(AAFwkTag::TEST, "CheckGetRunningInfoPermission_001 end");
}

/**
 * @tc.name: IsMemorySizeSufficient_001
 * @tc.desc: check get running info permission.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, IsMemorySizeSufficient_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "IsMemorySizeSufficient start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->IsMemorySizeSufficient();

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->IsMemorySizeSufficient();

    TAG_LOGI(AAFwkTag::TEST, "IsMemorySizeSufficient_001 end");
}

/**
 * @tc.name: LoadResidentProcess_001
 * @tc.desc: load resident process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, LoadResidentProcess_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "LoadResidentProcess_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::vector<BundleInfo> infos;
    appMgrServiceInner->LoadResidentProcess(infos);

    TAG_LOGI(AAFwkTag::TEST, "LoadResidentProcess_001 end");
}

/**
 * @tc.name: StartResidentProcess_001
 * @tc.desc: start resident process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartResidentProcess_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StartResidentProcess_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::vector<BundleInfo> infos;
    appMgrServiceInner->StartResidentProcess(infos, 0, true);

    BundleInfo info;
    infos.push_back(info);

    BundleInfo info1;
    info1.applicationInfo.process = "";
    infos.push_back(info1);

    BundleInfo info2;
    info2.applicationInfo.process = "test_process";
    infos.push_back(info2);
    appMgrServiceInner->StartResidentProcess(infos, 0, true);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->StartResidentProcess(infos, 0, true);

    TAG_LOGI(AAFwkTag::TEST, "StartResidentProcess_001 end");
}

/**
 * @tc.name: StartEmptyResidentProcess_001
 * @tc.desc: start empty resident process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartEmptyResidentProcess_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StartEmptyResidentProcess_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    BundleInfo info;
    info.applicationInfo = *applicationInfo_;
    std::string processName = "test_process";
    appMgrServiceInner->StartEmptyResidentProcess(info, processName, 0, true);

    appMgrServiceInner->StartEmptyResidentProcess(info, processName, 1, true);

    appMgrServiceInner->StartEmptyResidentProcess(info, "", 0, true);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->StartEmptyResidentProcess(info, processName, 0, true);

    appMgrServiceInner->remoteClientManager_ = nullptr;
    appMgrServiceInner->StartEmptyResidentProcess(info, processName, 0, true);

    TAG_LOGI(AAFwkTag::TEST, "StartEmptyResidentProcess_001 end");
}

/**
 * @tc.name: CheckRemoteClient_001
 * @tc.desc: check remote client.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, CheckRemoteClient_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckRemoteClient_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->CheckRemoteClient();

    appMgrServiceInner->remoteClientManager_->SetSpawnClient(nullptr);
    appMgrServiceInner->CheckRemoteClient();

    appMgrServiceInner->remoteClientManager_->SetBundleManagerHelper(nullptr);
    appMgrServiceInner->CheckRemoteClient();

    appMgrServiceInner->remoteClientManager_ = nullptr;
    appMgrServiceInner->CheckRemoteClient();

    TAG_LOGI(AAFwkTag::TEST, "CheckRemoteClient_001 end");
}

/**
 * @tc.name: RestartResidentProcess_001
 * @tc.desc: restart resident process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, RestartResidentProcess_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "RestartResidentProcess_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->RestartResidentProcess(nullptr);

    std::shared_ptr<AppRunningRecord> appRecord;
    appMgrServiceInner->RestartResidentProcess(appRecord);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo, "");
    EXPECT_NE(appRecord, nullptr);
    appRecord->mainBundleName_ = "com.ohos.settings";
    appMgrServiceInner->RestartResidentProcess(appRecord);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->RestartResidentProcess(appRecord);

    appMgrServiceInner->remoteClientManager_ = nullptr;
    appMgrServiceInner->RestartResidentProcess(appRecord);

    TAG_LOGI(AAFwkTag::TEST, "RestartResidentProcess_001 end");
}

/**
 * @tc.name: NotifyAppStatusByCallerUid_001
 * @tc.desc: notify app status by caller uid.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, NotifyAppStatusByCallerUid_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyAppStatusByCallerUid_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "test_bundle_name";
    std::string eventData = "test_event_data";
    appMgrServiceInner->NotifyAppStatusByCallerUid(bundleName, 0, 0, 0, 0, eventData);

    TAG_LOGI(AAFwkTag::TEST, "NotifyAppStatusByCallerUid_001 end");
}

/**
 * @tc.name: IsSubscriberControlledAndNotifyAppStatusByCommonEventName_001
 * @tc.desc: notify app status by special common event.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, IsSubscriberControlledAndNotifyAppStatusByCommonEventName_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "IsSubscriberControlledAndNotifyAppStatusByCommonEventName_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "test_bundle_name";
    std::string eventData = "usual.event.PACKAGE_DATA_CLEARED";
    AAFwk::Want want;
    bool ret = appMgrServiceInner->IsSubscriberControlledAndNotifyAppStatusByCommonEventName(bundleName,
        eventData, want);
    EXPECT_TRUE(ret);

    eventData = "test_event_name";
    ret = appMgrServiceInner->IsSubscriberControlledAndNotifyAppStatusByCommonEventName(bundleName, eventData, want);
    EXPECT_FALSE(ret);

    eventData = "usual.event.PACKAGE_RESTARTED";
    ret = appMgrServiceInner->IsSubscriberControlledAndNotifyAppStatusByCommonEventName(bundleName, eventData, want);
    EXPECT_TRUE(ret);
    TAG_LOGI(AAFwkTag::TEST, "IsSubscriberControlledAndNotifyAppStatusByCommonEventName_001 end");
}

/**
 * @tc.name: RegisterApplicationStateObserver_001
 * @tc.desc: register application state observer.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, RegisterApplicationStateObserver_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "RegisterApplicationStateObserver_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    sptr<IApplicationStateObserver> observer;
    std::vector<std::string> bundleNameList;
    appMgrServiceInner->RegisterApplicationStateObserver(observer, bundleNameList);

    TAG_LOGI(AAFwkTag::TEST, "RegisterApplicationStateObserver_001 end");
}

/**
 * @tc.name: UnregisterApplicationStateObserver_001
 * @tc.desc: unregister application state observer.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, UnregisterApplicationStateObserver_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "UnregisterApplicationStateObserver_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    sptr<IApplicationStateObserver> observer;
    appMgrServiceInner->UnregisterApplicationStateObserver(observer);

    TAG_LOGI(AAFwkTag::TEST, "UnregisterApplicationStateObserver_001 end");
}

/**
 * @tc.name: RegisterNativeChildExitNotify_001
 * @tc.desc: register native chlid exit notify.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, RegisterNativeChildExitNotify_001, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "RegisterNativeChildExitNotify_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    sptr<INativeChildNotify> notify;
    auto ret = appMgrServiceInner->RegisterNativeChildExitNotify(notify);
    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "RegisterNativeChildExitNotify_001 end");
}

/**
 * @tc.name: UnregisterNativeChildExitNotify_001
 * @tc.desc: unregister native chlid exit notify.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, UnregisterNativeChildExitNotify_001, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "UnregisterNativeChildExitNotify_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    sptr<INativeChildNotify> notify;
    auto ret = appMgrServiceInner->UnregisterNativeChildExitNotify(notify);
    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "UnregisterNativeChildExitNotify_001 end");
}

/**
 * @tc.name: GetForegroundApplications_001
 * @tc.desc: get foreground applications.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, GetForegroundApplications_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "GetForegroundApplications_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::vector<AppStateData> list;
    appMgrServiceInner->GetForegroundApplications(list);

    TAG_LOGI(AAFwkTag::TEST, "GetForegroundApplications_001 end");
}

/**
 * @tc.name: StartUserTestProcess_001
 * @tc.desc: start user test process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartUserTestProcess_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StartUserTestProcess_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    AAFwk::Want want;
    sptr<IRemoteObject> observer;
    BundleInfo bundleInfo;
    appMgrServiceInner->StartUserTestProcess(want, nullptr, bundleInfo, 0);

    appMgrServiceInner->StartUserTestProcess(want, observer, bundleInfo, 0);

    std::string bundle_name = "test_bundle_name";
    want.SetParam("-b", bundle_name);
    appMgrServiceInner->StartUserTestProcess(want, observer, bundleInfo, 0);

    std::string moduleName = "test_module_name";
    want.SetParam("-m", moduleName);
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = moduleName;
    bundleInfo.hapModuleInfos.push_back(hapModuleInfo);
    appMgrServiceInner->StartUserTestProcess(want, observer, bundleInfo, 0);

    appMgrServiceInner->remoteClientManager_ = nullptr;
    appMgrServiceInner->StartUserTestProcess(want, observer, bundleInfo, 0);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->StartUserTestProcess(want, observer, bundleInfo, 0);

    TAG_LOGI(AAFwkTag::TEST, "StartUserTestProcess_001 end");
}

/**
 * @tc.name: GetHapModuleInfoForTestRunner_001
 * @tc.desc: get hap module info for test runner.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, GetHapModuleInfoForTestRunner_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "GetHapModuleInfoForTestRunner_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    AAFwk::Want want;
    sptr<IRemoteObject> observer;
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    appMgrServiceInner->GetHapModuleInfoForTestRunner(want, nullptr, bundleInfo, hapModuleInfo);

    appMgrServiceInner->GetHapModuleInfoForTestRunner(want, observer, bundleInfo, hapModuleInfo);

    hapModuleInfo.moduleName = "test_module_name";
    bundleInfo.hapModuleInfos.push_back(hapModuleInfo);
    appMgrServiceInner->GetHapModuleInfoForTestRunner(want, observer, bundleInfo, hapModuleInfo);

    bundleInfo.hapModuleInfos.back().isModuleJson = true;
    appMgrServiceInner->GetHapModuleInfoForTestRunner(want, observer, bundleInfo, hapModuleInfo);

    std::string testmoduleName = "test_XXX";
    want.SetParam("-m", testmoduleName);
    appMgrServiceInner->GetHapModuleInfoForTestRunner(want, observer, bundleInfo, hapModuleInfo);

    std::string moduleName = "test_module_name";
    want.SetParam("-m", moduleName);
    appMgrServiceInner->GetHapModuleInfoForTestRunner(want, observer, bundleInfo, hapModuleInfo);

    TAG_LOGI(AAFwkTag::TEST, "GetHapModuleInfoForTestRunner_001 end");
}

/**
 * @tc.name: UserTestAbnormalFinish_001
 * @tc.desc: user test abnormal finish.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, UserTestAbnormalFinish_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "UserTestAbnormalFinish_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    sptr<IRemoteObject> observer;
    std::string msg = "testmsg";
    appMgrServiceInner->UserTestAbnormalFinish(nullptr, "");
    appMgrServiceInner->UserTestAbnormalFinish(nullptr, msg);
    appMgrServiceInner->UserTestAbnormalFinish(observer, "");
    appMgrServiceInner->UserTestAbnormalFinish(observer, msg);

    TAG_LOGI(AAFwkTag::TEST, "UserTestAbnormalFinish_001 end");
}

/**
 * @tc.name: StartEmptyProcess_001
 * @tc.desc: start empty process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartEmptyProcess_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StartEmptyProcess_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    AAFwk::Want want;
    sptr<IRemoteObject> observer;
    BundleInfo info;
    HapModuleInfo hapModuleInfo;
    std::string processName = "test_processName";
    appMgrServiceInner->StartEmptyProcess(want, nullptr, info, "", 0);
    appMgrServiceInner->StartEmptyProcess(want, observer, info, "", 0);
    appMgrServiceInner->StartEmptyProcess(want, observer, info, processName, 0);

    info.applicationInfo = *applicationInfo_;
    appMgrServiceInner->StartEmptyProcess(want, observer, info, processName, 0);

    want.SetParam("coldStart", true);
    appMgrServiceInner->StartEmptyProcess(want, observer, info, processName, 0);

    appMgrServiceInner->remoteClientManager_ = nullptr;
    appMgrServiceInner->StartEmptyProcess(want, observer, info, processName, 0);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->StartEmptyProcess(want, observer, info, processName, 0);

    TAG_LOGI(AAFwkTag::TEST, "StartEmptyProcess_001 end");
}

/**
 * @tc.name: FinishUserTest_001
 * @tc.desc: finish user test.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, FinishUserTest_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "FinishUserTest_001 start");
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
    std::string processName = "test_processName";
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = new MockAbilityToken();
    std::shared_ptr<AppRunningRecord> appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam,
        applicationInfo_, abilityInfo_, processName, bundleInfo, hapModuleInfo, want);
    EXPECT_NE(appRecord, nullptr);
    pid = appRecord->GetPriorityObject()->GetPid();
    appMgrServiceInner->FinishUserTest(msg, 0, bundleName, pid);

    std::shared_ptr<UserTestRecord> record = std::make_shared<UserTestRecord>();
    appRecord->SetUserTestInfo(record);
    appMgrServiceInner->FinishUserTest(msg, 0, bundleName, pid);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->FinishUserTest(msg, 0, bundleName, pid);

    TAG_LOGI(AAFwkTag::TEST, "FinishUserTest_001 end");
}

/**
 * @tc.name: FinishUserTestLocked_001
 * @tc.desc: finish user test locked.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, FinishUserTestLocked_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "FinishUserTestLocked_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->FinishUserTestLocked("", 0, nullptr);

    std::shared_ptr<AppRunningRecord> appRecord;
    appMgrServiceInner->FinishUserTestLocked("", 0, appRecord);

    std::string msg = "testmsg";
    appMgrServiceInner->FinishUserTestLocked(msg, 0, nullptr);
    appMgrServiceInner->FinishUserTestLocked(msg, 0, appRecord);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo, "");
    EXPECT_NE(appRecord, nullptr);
    std::shared_ptr<UserTestRecord> record = std::make_shared<UserTestRecord>();
    appRecord->SetUserTestInfo(record);
    appMgrServiceInner->FinishUserTestLocked(msg, 0, appRecord);

    record->isFinished = true;
    appRecord->SetUserTestInfo(record);
    appMgrServiceInner->FinishUserTestLocked(msg, 0, appRecord);

    record->observer = nullptr;
    appRecord->SetUserTestInfo(record);
    appMgrServiceInner->FinishUserTestLocked(msg, 0, appRecord);

    TAG_LOGI(AAFwkTag::TEST, "FinishUserTestLocked_001 end");
}

/**
 * @tc.name: StartSpecifiedAbility_001
 * @tc.desc: start specified ability.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartSpecifiedAbility_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StartSpecifiedAbility_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    AAFwk::Want want;
    AbilityInfo abilityInfo;
    appMgrServiceInner->StartSpecifiedAbility(want, abilityInfo);

    appMgrServiceInner->StartSpecifiedAbility(want, *abilityInfo_);

    abilityInfo_->applicationInfo = *applicationInfo_;
    appMgrServiceInner->StartSpecifiedAbility(want, *abilityInfo_);

    appMgrServiceInner->remoteClientManager_->SetBundleManagerHelper(nullptr);
    appMgrServiceInner->StartSpecifiedAbility(want, *abilityInfo_);

    appMgrServiceInner->remoteClientManager_ = nullptr;
    appMgrServiceInner->StartSpecifiedAbility(want, *abilityInfo_);

    TAG_LOGI(AAFwkTag::TEST, "StartSpecifiedAbility_001 end");
}

/**
 * @tc.name: RegisterStartSpecifiedAbilityResponse_001
 * @tc.desc: register start specified ability response.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, RegisterStartSpecifiedAbilityResponse_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "RegisterStartSpecifiedAbilityResponse_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->RegisterStartSpecifiedAbilityResponse(nullptr);

    sptr<IStartSpecifiedAbilityResponse> response;
    appMgrServiceInner->RegisterStartSpecifiedAbilityResponse(response);

    TAG_LOGI(AAFwkTag::TEST, "RegisterStartSpecifiedAbilityResponse_001 end");
}

/**
 * @tc.name: ScheduleAcceptWantDone_001
 * @tc.desc: schedule accept want done.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, ScheduleAcceptWantDone_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ScheduleAcceptWantDone_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    AAFwk::Want want;
    std::string flag = "test_flag";
    appMgrServiceInner->ScheduleAcceptWantDone(0, want, flag);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo, "");
    appMgrServiceInner->ScheduleAcceptWantDone(appRecord->GetRecordId(), want, flag);

    sptr<IStartSpecifiedAbilityResponse> response;
    appMgrServiceInner->RegisterStartSpecifiedAbilityResponse(response);
    appMgrServiceInner->ScheduleAcceptWantDone(appRecord->GetRecordId(), want, flag);

    appRecord->SetSpecifiedAbilityFlagAndWant(0, AAFwk::Want(), "");
    appMgrServiceInner->ScheduleAcceptWantDone(appRecord->GetRecordId(), want, flag);
    EXPECT_FALSE(appRecord->IsStartSpecifiedAbility());

    TAG_LOGI(AAFwkTag::TEST, "ScheduleAcceptWantDone_001 end");
}

/**
 * @tc.name: HandleStartSpecifiedAbilityTimeOut_001
 * @tc.desc: handle start specified ability time out.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, HandleStartSpecifiedAbilityTimeOut_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleStartSpecifiedAbilityTimeOut_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->HandleStartSpecifiedAbilityTimeOut(nullptr);

    BundleInfo bundleInfo;
    std::string appName = "test_appName";
    std::string processName = "test_processName";
    std::string bundleName = "test_bundleName";
    sptr<IRemoteObject> token = new MockAbilityToken();
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo, "");
    EXPECT_NE(appRecord, nullptr);

    appRecord->specifiedAbilityRequest_ = std::make_shared<SpecifiedRequest>();
    appRecord->specifiedAbilityRequest_->requestId = 1;
    appMgrServiceInner->HandleStartSpecifiedAbilityTimeOut(appRecord);

    sptr<IStartSpecifiedAbilityResponse> response;
    appMgrServiceInner->startSpecifiedAbilityResponse_ = response;
    appMgrServiceInner->HandleStartSpecifiedAbilityTimeOut(appRecord);

    appRecord->specifiedAbilityRequest_ = nullptr;
    appMgrServiceInner->HandleStartSpecifiedAbilityTimeOut(appRecord);

    TAG_LOGI(AAFwkTag::TEST, "HandleStartSpecifiedAbilityTimeOut_001 end");
}

/**
 * @tc.name: UpdateConfiguration_001
 * @tc.desc: update configuration.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, UpdateConfiguration_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateConfiguration_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    Configuration config;
    appMgrServiceInner->UpdateConfiguration(config);

    auto testLanguge = "ch-zh";
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, testLanguge);
    appMgrServiceInner->UpdateConfiguration(config);

    auto appRunningRecordMap = appMgrServiceInner->appRunningManager_->appRunningRecordMap_;
    for (const auto& item : appRunningRecordMap) {
        const auto& appRecord = item.second;
        if (appRecord) {
            appRecord->appLifeCycleDeal_ = nullptr;
        }
    }
    appMgrServiceInner->UpdateConfiguration(config);

    sptr<MockConfigurationObserver> observer(new (std::nothrow) MockConfigurationObserver());
    appMgrServiceInner->configurationObservers_.push_back({ observer, 100 });
    sptr<IConfigurationObserver> observer1;
    appMgrServiceInner->configurationObservers_.push_back({ observer1, 100 });
    appMgrServiceInner->configurationObservers_.push_back({ nullptr, 100 });
    appMgrServiceInner->UpdateConfiguration(config);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->UpdateConfiguration(config);

    TAG_LOGI(AAFwkTag::TEST, "UpdateConfiguration_001 end");
}

/**
 * @tc.name: RegisterConfigurationObserver_001
 * @tc.desc: register configuration observer.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, RegisterConfigurationObserver_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "RegisterConfigurationObserver_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->configurationObservers_.clear();

    appMgrServiceInner->RegisterConfigurationObserver(nullptr, -1);

    sptr<MockConfigurationObserver> observer(new (std::nothrow) MockConfigurationObserver());
    appMgrServiceInner->RegisterConfigurationObserver(observer, -1);
    appMgrServiceInner->RegisterConfigurationObserver(observer, -1);

    TAG_LOGI(AAFwkTag::TEST, "RegisterConfigurationObserver_001 end");
}

/**
 * @tc.name: UnregisterConfigurationObserver_001
 * @tc.desc: unregister configuration observer.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, UnregisterConfigurationObserver_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "UnregisterConfigurationObserver_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->configurationObservers_.clear();

    appMgrServiceInner->UnregisterConfigurationObserver(nullptr);

    sptr<MockConfigurationObserver> observer(new (std::nothrow) MockConfigurationObserver());
    appMgrServiceInner->UnregisterConfigurationObserver(observer);

    appMgrServiceInner->RegisterConfigurationObserver(observer, -1);
    appMgrServiceInner->UnregisterConfigurationObserver(observer);

    TAG_LOGI(AAFwkTag::TEST, "UnregisterConfigurationObserver_001 end");
}

/**
 * @tc.name: InitGlobalConfiguration_001
 * @tc.desc: init global configuration.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, InitGlobalConfiguration_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "InitGlobalConfiguration_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->InitGlobalConfiguration();

    appMgrServiceInner->InitGlobalConfiguration();

    TAG_LOGI(AAFwkTag::TEST, "InitGlobalConfiguration_001 end");
}

/**
 * @tc.name: KillApplicationByRecord_001
 * @tc.desc: kill application by record.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, KillApplicationByRecord_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "KillApplicationByRecord_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord1 =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo, "");
    EXPECT_NE(appRecord1, nullptr);
    appMgrServiceInner->KillApplicationByRecord(appRecord);
    appMgrServiceInner->KillApplicationByRecord(appRecord1);

    appMgrServiceInner->taskHandler_ = nullptr;
    appMgrServiceInner->KillApplicationByRecord(appRecord);
    appMgrServiceInner->KillApplicationByRecord(appRecord1);

    TAG_LOGI(AAFwkTag::TEST, "KillApplicationByRecord_001 end");
}

/**
 * @tc.name: SendHiSysEvent_001
 * @tc.desc: send hi sys event.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, SendHiSysEvent_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "SendHiSysEvent_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->SendHiSysEvent(0, nullptr);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo, "");
    EXPECT_NE(appRecord, nullptr);
    appMgrServiceInner->SendHiSysEvent(0, appRecord);
    appMgrServiceInner->SendHiSysEvent(1, appRecord);
    appMgrServiceInner->SendHiSysEvent(2, appRecord);
    appMgrServiceInner->SendHiSysEvent(3, appRecord);
    appMgrServiceInner->SendHiSysEvent(4, appRecord);

    TAG_LOGI(AAFwkTag::TEST, "SendHiSysEvent_001 end");
}

/**
 * @tc.name: GetAbilityRecordsByProcessID_001
 * @tc.desc: get ability records by process id.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, GetAbilityRecordsByProcessID_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAbilityRecordsByProcessID_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::vector<sptr<IRemoteObject>> tokens;
    appMgrServiceInner->GetAbilityRecordsByProcessID(0, tokens);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo, "");
    EXPECT_NE(appRecord, nullptr);
    int pid = appRecord->GetPriorityObject()->GetPid();
    appMgrServiceInner->GetAbilityRecordsByProcessID(pid, tokens);

    TAG_LOGI(AAFwkTag::TEST, "GetAbilityRecordsByProcessID_001 end");
}

/**
 * @tc.name: GetApplicationInfoByProcessID_001
 * @tc.desc: get applicationInfo by process id.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, GetApplicationInfoByProcessID_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "GetApplicationInfoByProcessID_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    ApplicationInfo application;
    bool debug = false;
    appMgrServiceInner->GetApplicationInfoByProcessID(0, application, debug);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo, "");
    EXPECT_NE(appRecord, nullptr);
    int pid = appRecord->GetPriorityObject()->GetPid();
    appMgrServiceInner->GetApplicationInfoByProcessID(pid, application, debug);

    appRecord->appInfo_ = nullptr;
    appMgrServiceInner->GetApplicationInfoByProcessID(pid, application, debug);

    TAG_LOGI(AAFwkTag::TEST, "GetApplicationInfoByProcessID_001 end");
}

/**
 * @tc.name: NotifyAppMgrRecordExitReason_001
 * @tc.desc: NotifyAppMgrRecordExitReason.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, NotifyAppMgrRecordExitReason_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyAppMgrRecordExitReason_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    int32_t reason = 0;
    int32_t pid = 1;
    std::string exitMsg = "JsError";
    auto ret = appMgrServiceInner->NotifyAppMgrRecordExitReason(reason, pid, exitMsg);
    EXPECT_NE(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "NotifyAppMgrRecordExitReason_001 end");
}

/**
 * @tc.name: VerifyKillProcessPermission_001
 * @tc.desc: verify process permission.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, VerifyKillProcessPermission_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "VerifyKillProcessPermission_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->VerifyKillProcessPermission("");

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->VerifyKillProcessPermission("");

    TAG_LOGI(AAFwkTag::TEST, "VerifyKillProcessPermission_001 end");
}

/**
 * @tc.name: VerifyAPL_001
 * @tc.desc: verify APL.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, VerifyAPL_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "VerifyAPL_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->VerifyAPL();

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->VerifyAPL();

    TAG_LOGI(AAFwkTag::TEST, "VerifyAPL_001 end");
}

/**
 * @tc.name: VerifyAccountPermission_001
 * @tc.desc: verify account permission.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, VerifyAccountPermission_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "VerifyAccountPermission_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::string permissionName = "test_permissionName";
    appMgrServiceInner->VerifyAccountPermission(permissionName, 0);

    TAG_LOGI(AAFwkTag::TEST, "VerifyAccountPermission_001 end");
}

/**
 * @tc.name: PreStartNWebSpawnProcess_003
 * @tc.desc: prestart nwebspawn process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, PreStartNWebSpawnProcess_003, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    int callingPid = 1;
    appMgrServiceInner->remoteClientManager_->nwebSpawnClient_ = nullptr;
    int ret = appMgrServiceInner->PreStartNWebSpawnProcess(callingPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: StartRenderProcess_001
 * @tc.desc: start render process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartRenderProcess_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t hostPid = 0;
    std::string renderParam = "";
    pid_t renderPid = 0;
    int ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, FdGuard(-1), FdGuard(-1), FdGuard(-1),
        renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_001 end");
}

/**
 * @tc.name: StartRenderProcess_003
 * @tc.desc: start render process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartRenderProcess_003, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t hostPid = 0;
    std::string renderParam = "";
    FdGuard ipcFd(0);
    FdGuard sharedFd(0);
    FdGuard crashFd(1);
    pid_t renderPid = 0;
    int ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, std::move(ipcFd), std::move(sharedFd),
        std::move(crashFd), renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ipcFd.Release();
    sharedFd.Release();
    crashFd.Release();
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_003 end");
}

/**
 * @tc.name: StartRenderProcess_004
 * @tc.desc: start render process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartRenderProcess_004, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_004 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t hostPid = 0;
    std::string renderParam = "";
    FdGuard ipcFd(0);
    FdGuard sharedFd(1);
    FdGuard crashFd(0);
    pid_t renderPid = 0;
    int ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, std::move(ipcFd), std::move(sharedFd),
        std::move(crashFd), renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ipcFd.Release();
    sharedFd.Release();
    crashFd.Release();
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_004 end");
}

/**
 * @tc.name: StartRenderProcess_005
 * @tc.desc: start render process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartRenderProcess_005, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_005 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t hostPid = 0;
    std::string renderParam = "";
    FdGuard ipcFd(0);
    FdGuard sharedFd(1);
    FdGuard crashFd(1);
    pid_t renderPid = 0;
    int ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, std::move(ipcFd), std::move(sharedFd),
        std::move(crashFd), renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ipcFd.Release();
    sharedFd.Release();
    crashFd.Release();
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_005 end");
}

/**
 * @tc.name: StartRenderProcess_006
 * @tc.desc: start render process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartRenderProcess_006, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_006 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t hostPid = 0;
    std::string renderParam = "";
    FdGuard ipcFd(1);
    FdGuard sharedFd(0);
    FdGuard crashFd(0);
    pid_t renderPid = 0;
    int ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, std::move(ipcFd), std::move(sharedFd),
        std::move(crashFd), renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ipcFd.Release();
    sharedFd.Release();
    crashFd.Release();
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_006 end");
}

/**
 * @tc.name: StartRenderProcess_007
 * @tc.desc: start render process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartRenderProcess_007, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_007 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t hostPid = 0;
    std::string renderParam = "";
    FdGuard ipcFd(1);
    FdGuard sharedFd(0);
    FdGuard crashFd(1);
    pid_t renderPid = 0;
    int ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, std::move(ipcFd), std::move(sharedFd),
        std::move(crashFd), renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ipcFd.Release();
    sharedFd.Release();
    crashFd.Release();
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_007 end");
}

/**
 * @tc.name: StartRenderProcess_008
 * @tc.desc: start render process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartRenderProcess_008, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_008 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t hostPid = 0;
    std::string renderParam = "";
    FdGuard ipcFd(1);
    FdGuard sharedFd(1);
    FdGuard crashFd(0);
    pid_t renderPid = 0;
    int ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, std::move(ipcFd), std::move(sharedFd),
        std::move(crashFd), renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ipcFd.Release();
    sharedFd.Release();
    crashFd.Release();
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_008 end");
}

/**
 * @tc.name: StartRenderProcess_009
 * @tc.desc: start render process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartRenderProcess_009, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_009 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t hostPid = 0;
    std::string renderParam = "";
    FdGuard ipcFd(1);
    FdGuard sharedFd(1);
    FdGuard crashFd(1);
    pid_t renderPid = 0;
    int ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, std::move(ipcFd), std::move(sharedFd),
        std::move(crashFd), renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ipcFd.Release();
    sharedFd.Release();
    crashFd.Release();
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_009 end");
}

/**
 * @tc.name: StartRenderProcess_010
 * @tc.desc: start render process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartRenderProcess_010, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_010 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t hostPid = 0;
    std::string renderParam = "test_renderParam";
    FdGuard ipcFd(0);
    FdGuard sharedFd(0);
    FdGuard crashFd(0);
    pid_t renderPid = 0;
    int ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, std::move(ipcFd), std::move(sharedFd),
        std::move(crashFd), renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ipcFd.Release();
    sharedFd.Release();
    crashFd.Release();
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_010 end");
}

/**
 * @tc.name: StartRenderProcess_011
 * @tc.desc: start render process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartRenderProcess_011, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_011 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t hostPid = 0;
    std::string renderParam = "test_renderParam";
    FdGuard ipcFd(0);
    FdGuard sharedFd(0);
    FdGuard crashFd(1);
    pid_t renderPid = 0;
    int ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, std::move(ipcFd), std::move(sharedFd),
        std::move(crashFd), renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ipcFd.Release();
    sharedFd.Release();
    crashFd.Release();
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_011 end");
}

/**
 * @tc.name: StartRenderProcess_012
 * @tc.desc: start render process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartRenderProcess_012, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_012 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t hostPid = 0;
    std::string renderParam = "test_renderParam";
    FdGuard ipcFd(0);
    FdGuard sharedFd(1);
    FdGuard crashFd(0);
    pid_t renderPid = 0;
    int ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, std::move(ipcFd), std::move(sharedFd),
        std::move(crashFd), renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ipcFd.Release();
    sharedFd.Release();
    crashFd.Release();
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_012 end");
}

/**
 * @tc.name: StartRenderProcess_013
 * @tc.desc: start render process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartRenderProcess_013, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_013 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t hostPid = 0;
    std::string renderParam = "test_renderParam";
    FdGuard ipcFd(0);
    FdGuard sharedFd(1);
    FdGuard crashFd(1);
    pid_t renderPid = 0;
    int ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, std::move(ipcFd), std::move(sharedFd),
        std::move(crashFd), renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ipcFd.Release();
    sharedFd.Release();
    crashFd.Release();
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_013 end");
}

/**
 * @tc.name: StartRenderProcess_014
 * @tc.desc: start render process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartRenderProcess_014, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_014 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t hostPid = 0;
    std::string renderParam = "test_renderParam";
    FdGuard ipcFd(1);
    FdGuard sharedFd(0);
    FdGuard crashFd(0);
    pid_t renderPid = 0;
    int ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, std::move(ipcFd), std::move(sharedFd),
        std::move(crashFd), renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ipcFd.Release();
    sharedFd.Release();
    crashFd.Release();
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_014 end");
}

/**
 * @tc.name: StartRenderProcess_015
 * @tc.desc: start render process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartRenderProcess_015, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_015 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t hostPid = 0;
    std::string renderParam = "test_renderParam";
    FdGuard ipcFd(1);
    FdGuard sharedFd(0);
    FdGuard crashFd(1);
    pid_t renderPid = 0;
    int ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, std::move(ipcFd), std::move(sharedFd),
        std::move(crashFd), renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ipcFd.Release();
    sharedFd.Release();
    crashFd.Release();
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_015 end");
}

/**
 * @tc.name: StartRenderProcess_016
 * @tc.desc: start render process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartRenderProcess_016, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_016 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t hostPid = 0;
    std::string renderParam = "test_renderParam";
    FdGuard ipcFd(1);
    FdGuard sharedFd(1);
    FdGuard crashFd(0);
    pid_t renderPid = 0;
    int ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, std::move(ipcFd), std::move(sharedFd),
        std::move(crashFd), renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ipcFd.Release();
    sharedFd.Release();
    crashFd.Release();
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_016 end");
}

/**
 * @tc.name: StartRenderProcess_017
 * @tc.desc: start render process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartRenderProcess_017, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_017 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t hostPid = 0;
    std::string renderParam = "test_renderParam";
    FdGuard ipcFd(1);
    FdGuard sharedFd(1);
    FdGuard crashFd(1);
    pid_t renderPid = 0;
    int ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, std::move(ipcFd), std::move(sharedFd),
        std::move(crashFd), renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ipcFd.Release();
    sharedFd.Release();
    crashFd.Release();
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_017 end");
}

/**
 * @tc.name: StartRenderProcess_018
 * @tc.desc: start render process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartRenderProcess_018, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_018 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t hostPid = 1;
    std::string renderParam = "";
    FdGuard ipcFd(0);
    FdGuard sharedFd(0);
    FdGuard crashFd(0);
    pid_t renderPid = 0;
    int ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, std::move(ipcFd), std::move(sharedFd),
        std::move(crashFd), renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ipcFd.Release();
    sharedFd.Release();
    crashFd.Release();
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_018 end");
}

/**
 * @tc.name: StartRenderProcess_019
 * @tc.desc: start render process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartRenderProcess_019, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_019 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t hostPid = 1;
    std::string renderParam = "";
    FdGuard ipcFd(0);
    FdGuard sharedFd(0);
    FdGuard crashFd(1);
    pid_t renderPid = 0;
    int ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, std::move(ipcFd), std::move(sharedFd),
        std::move(crashFd), renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ipcFd.Release();
    sharedFd.Release();
    crashFd.Release();
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_019 end");
}

/**
 * @tc.name: StartRenderProcess_020
 * @tc.desc: start render process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartRenderProcess_020, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_020 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t hostPid = 1;
    std::string renderParam = "";
    FdGuard ipcFd(0);
    FdGuard sharedFd(1);
    FdGuard crashFd(0);
    pid_t renderPid = 0;
    int ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, std::move(ipcFd), std::move(sharedFd),
        std::move(crashFd), renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ipcFd.Release();
    sharedFd.Release();
    crashFd.Release();
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_020 end");
}

/**
 * @tc.name: StartRenderProcess_021
 * @tc.desc: start render process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartRenderProcess_021, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_021 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t hostPid = 1;
    std::string renderParam = "";
    FdGuard ipcFd(0);
    FdGuard sharedFd(1);
    FdGuard crashFd(1);
    pid_t renderPid = 0;
    int ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, std::move(ipcFd), std::move(sharedFd),
        std::move(crashFd), renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ipcFd.Release();
    sharedFd.Release();
    crashFd.Release();
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_021 end");
}

/**
 * @tc.name: StartRenderProcess_002
 * @tc.desc: start render process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartRenderProcess_002, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t hostPid = 1;
    std::string renderParam = "";
    FdGuard ipcFd(1);
    FdGuard sharedFd(0);
    FdGuard crashFd(0);
    pid_t renderPid = 0;
    int ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, std::move(ipcFd), std::move(sharedFd),
        std::move(crashFd), renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ipcFd.Release();
    sharedFd.Release();
    crashFd.Release();
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_002 end");
}

/**
 * @tc.name: StartRenderProcess_022
 * @tc.desc: start render process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartRenderProcess_022, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_022 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t hostPid = 1;
    std::string renderParam = "";
    FdGuard ipcFd(1);
    FdGuard sharedFd(0);
    FdGuard crashFd(1);
    pid_t renderPid = 0;
    int ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, std::move(ipcFd), std::move(sharedFd),
        std::move(crashFd), renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ipcFd.Release();
    sharedFd.Release();
    crashFd.Release();
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_022 end");
}

/**
 * @tc.name: StartRenderProcess_032
 * @tc.desc: start render process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartRenderProcess_032, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_032 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t hostPid = 1;
    std::string renderParam = "";
    FdGuard ipcFd(1);
    FdGuard sharedFd(1);
    FdGuard crashFd(0);
    pid_t renderPid = 0;
    int ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, std::move(ipcFd), std::move(sharedFd),
        std::move(crashFd), renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ipcFd.Release();
    sharedFd.Release();
    crashFd.Release();
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_032 end");
}

/**
 * @tc.name: StartRenderProcess_023
 * @tc.desc: start render process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartRenderProcess_023, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_023 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t hostPid = 1;
    std::string renderParam = "";
    FdGuard ipcFd(1);
    FdGuard sharedFd(1);
    FdGuard crashFd(1);
    pid_t renderPid = 0;
    int ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, std::move(ipcFd), std::move(sharedFd),
        std::move(crashFd), renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ipcFd.Release();
    sharedFd.Release();
    crashFd.Release();
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_023 end");
}

/**
 * @tc.name: StartRenderProcess_024
 * @tc.desc: start render process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartRenderProcess_024, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_024 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t hostPid = 1;
    std::string renderParam = "test_renderParam";
    FdGuard ipcFd(0);
    FdGuard sharedFd(0);
    FdGuard crashFd(0);
    pid_t renderPid = 0;
    int ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, std::move(ipcFd), std::move(sharedFd),
        std::move(crashFd), renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ipcFd.Release();
    sharedFd.Release();
    crashFd.Release();
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_024 end");
}

/**
 * @tc.name: StartRenderProcess_025
 * @tc.desc: start render process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartRenderProcess_025, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_025 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t hostPid = 1;
    std::string renderParam = "test_renderParam";
    FdGuard ipcFd(0);
    FdGuard sharedFd(0);
    FdGuard crashFd(1);
    pid_t renderPid = 0;
    int ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, std::move(ipcFd), std::move(sharedFd),
        std::move(crashFd), renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ipcFd.Release();
    sharedFd.Release();
    crashFd.Release();
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_025 end");
}

/**
 * @tc.name: StartRenderProcess_026
 * @tc.desc: start render process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartRenderProcess_026, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_026 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t hostPid = 1;
    std::string renderParam = "test_renderParam";
    FdGuard ipcFd(0);
    FdGuard sharedFd(1);
    FdGuard crashFd(0);
    pid_t renderPid = 0;
    int ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, std::move(ipcFd), std::move(sharedFd),
        std::move(crashFd), renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ipcFd.Release();
    sharedFd.Release();
    crashFd.Release();
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_026 end");
}

/**
 * @tc.name: StartRenderProcess_027
 * @tc.desc: start render process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartRenderProcess_027, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_027 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t hostPid = 1;
    std::string renderParam = "test_renderParam";
    FdGuard ipcFd(0);
    FdGuard sharedFd(1);
    FdGuard crashFd(1);
    pid_t renderPid = 0;
    int ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, std::move(ipcFd), std::move(sharedFd),
        std::move(crashFd), renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ipcFd.Release();
    sharedFd.Release();
    crashFd.Release();
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_027 end");
}

/**
 * @tc.name: StartRenderProcess_028
 * @tc.desc: start render process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartRenderProcess_028, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_028 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t hostPid = 1;
    std::string renderParam = "test_renderParam";
    FdGuard ipcFd(1);
    FdGuard sharedFd(0);
    FdGuard crashFd(0);
    pid_t renderPid = 0;
    int ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, std::move(ipcFd), std::move(sharedFd),
        std::move(crashFd), renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ipcFd.Release();
    sharedFd.Release();
    crashFd.Release();
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_028 end");
}

/**
 * @tc.name: StartRenderProcess_029
 * @tc.desc: start render process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartRenderProcess_029, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_029 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t hostPid = 1;
    std::string renderParam = "test_renderParam";
    FdGuard ipcFd(1);
    FdGuard sharedFd(0);
    FdGuard crashFd(1);
    pid_t renderPid = 0;
    int ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, std::move(ipcFd), std::move(sharedFd),
        std::move(crashFd), renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ipcFd.Release();
    sharedFd.Release();
    crashFd.Release();
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_029 end");
}

/**
 * @tc.name: StartRenderProcess_030
 * @tc.desc: start render process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartRenderProcess_030, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_030 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t hostPid = 1;
    std::string renderParam = "test_renderParam";
    FdGuard ipcFd(1);
    FdGuard sharedFd(1);
    FdGuard crashFd(0);
    pid_t renderPid = 0;
    int ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, std::move(ipcFd), std::move(sharedFd),
        std::move(crashFd), renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ipcFd.Release();
    sharedFd.Release();
    crashFd.Release();
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_030 end");
}

/**
 * @tc.name: StartRenderProcess_031
 * @tc.desc: start render process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, StartRenderProcess_031, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_031 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t hostPid = 1;
    std::string renderParam = "test_renderParam";
    FdGuard ipcFd(1);
    FdGuard sharedFd(1);
    FdGuard crashFd(1);
    pid_t renderPid = 0;
    int ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, std::move(ipcFd), std::move(sharedFd),
        std::move(crashFd), renderPid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ipcFd.Release();
    sharedFd.Release();
    crashFd.Release();
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_031 end");
}

/**
 * @tc.name: AttachRenderProcess_001
 * @tc.desc: attach render process.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, AttachRenderProcess_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AttachRenderProcess_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t pid = 0;
    sptr<IRenderScheduler> scheduler;
    appMgrServiceInner->AttachRenderProcess(pid, scheduler);
    TAG_LOGI(AAFwkTag::TEST, "AttachRenderProcess_001 end");
}

/**
 * @tc.name: AttachRenderProcess_002
 * @tc.desc: attach render process.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, AttachRenderProcess_002, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AttachRenderProcess_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t pid = 1;
    sptr<IRenderScheduler> scheduler;
    appMgrServiceInner->AttachRenderProcess(pid, scheduler);
    TAG_LOGI(AAFwkTag::TEST, "AttachRenderProcess_002 end");
}

#ifdef WITH_DLP
/**
 * @tc.name: BuildStartFlags_001
 * @tc.desc: build start flags.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, BuildStartFlags_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "BuildStartFlags_001 start");

    AAFwk::Want want;
    AbilityInfo abilityInfo;
    AppspawnUtil::BuildStartFlags(want, abilityInfo);

    want.SetParam("coldStart", true);
    want.SetParam("ohos.dlp.params.index", 1);
    abilityInfo.extensionAbilityType = ExtensionAbilityType::BACKUP;
    uint64_t result = AppspawnUtil::BuildStartFlags(want, abilityInfo);
    
    uint64_t flag = 0x0;
    uint64_t baseFlag = 1;
    flag = flag | (baseFlag << StartFlags::COLD_START);
    flag = flag | (baseFlag << StartFlags::DLP_MANAGER_FULL_CONTROL);
    flag = flag | (baseFlag << StartFlags::BACKUP_EXTENSION);
    EXPECT_EQ(result, flag);

    TAG_LOGI(AAFwkTag::TEST, "BuildStartFlags_001 end");
}

/**
 * @tc.name: BuildStartFlags_002
 * @tc.desc: build start flags.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, BuildStartFlags_002, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "BuildStartFlags_002 start");

    AAFwk::Want want;
    AbilityInfo abilityInfo;
    AppspawnUtil::BuildStartFlags(want, abilityInfo);

    want.SetParam("coldStart", false);
    want.SetParam("ohos.dlp.params.index", 1);
    want.SetParam("ohos.dlp.params.securityFlag", true);
    uint64_t result = AppspawnUtil::BuildStartFlags(want, abilityInfo);

    uint64_t flag = 0x0;
    uint64_t baseFlag = 1;
    flag = flag | (baseFlag << StartFlags::DLP_MANAGER_READ_ONLY);
    EXPECT_EQ(result, flag);

    TAG_LOGI(AAFwkTag::TEST, "BuildStartFlags_002 end");
}
#endif // WITH_DLP

/**
 * @tc.name: BuildStartFlags_003
 * @tc.desc: build start flags.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, BuildStartFlags_003, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "BuildStartFlags_003 start");
    AAFwk::Want want;
    AbilityInfo abilityInfo;
    abilityInfo.applicationInfo.debug = true;
    abilityInfo.applicationInfo.appProvisionType = AppExecFwk::Constants::APP_PROVISION_TYPE_DEBUG;
    uint64_t result = AppspawnUtil::BuildStartFlags(want, abilityInfo);
    uint64_t flag = 0x0;
    uint64_t baseFlag = 1;
    flag = flag | (baseFlag << StartFlags::DEBUGGABLE);
    flag = flag | (baseFlag << APP_FLAGS_DEBUG_SIGN);
    EXPECT_EQ(result, flag);
    TAG_LOGI(AAFwkTag::TEST, "BuildStartFlags_003 end");
}

/**
 * @tc.name: BuildStartFlags_004
 * @tc.desc: build start flags.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, BuildStartFlags_004, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "BuildStartFlags_004 start");
    AAFwk::Want want;
    AbilityInfo abilityInfo;
    abilityInfo.applicationInfo.cloudFileSyncEnabled = true;
    uint64_t result = AppspawnUtil::BuildStartFlags(want, abilityInfo);
    uint64_t flag = 0x0;
    uint64_t baseFlag = 1;
    flag = flag | (baseFlag << APP_FLAGS_CLOUD_FILE_SYNC_ENABLED);
    EXPECT_EQ(result, flag);
    TAG_LOGI(AAFwkTag::TEST, "BuildStartFlags_004 end");
}

/**
 * @tc.name: BuildStartFlags_005
 * @tc.desc: build start flags.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, BuildStartFlags_005, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "BuildStartFlags_005 start");
    AAFwk::Want want;
    AbilityInfo abilityInfo;
    abilityInfo.applicationInfo.appProvisionType = AppExecFwk::Constants::APP_PROVISION_TYPE_DEBUG;
    uint64_t result = AppspawnUtil::BuildStartFlags(want, abilityInfo);
    uint64_t flag = 0x0;
    uint64_t baseFlag = 1;
    flag = flag | (baseFlag << APP_FLAGS_DEBUG_SIGN);
    EXPECT_EQ(result, flag);
    TAG_LOGI(AAFwkTag::TEST, "BuildStartFlags_005 end");
}

/**
 * @tc.name: RegisterFocusListener_001
 * @tc.desc: register focus listener.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, RegisterFocusListener_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "RegisterFocusListener_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->RegisterFocusListener();

    appMgrServiceInner->focusListener_ = nullptr;
    appMgrServiceInner->RegisterFocusListener();

    TAG_LOGI(AAFwkTag::TEST, "RegisterFocusListener_001 end");
}

/**
 * @tc.name: HandleFocused_001
 * @tc.desc: handle focused.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, HandleFocused_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleFocused_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    sptr<Rosen::FocusChangeInfo> focusChangeInfo;
    appMgrServiceInner->HandleFocused(focusChangeInfo);

    pid_t pid = 1;
    focusChangeInfo = new Rosen::FocusChangeInfo();
    appMgrServiceInner->HandleFocused(focusChangeInfo);

    focusChangeInfo->pid_ = pid;
    appMgrServiceInner->HandleFocused(focusChangeInfo);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo, "");
    EXPECT_NE(appRecord, nullptr);
    appRecord->GetPriorityObject()->SetPid(pid);
    appMgrServiceInner->HandleFocused(focusChangeInfo);

    TAG_LOGI(AAFwkTag::TEST, "HandleFocused_001 end");
}

/**
 * @tc.name: HandleUnfocused_001
 * @tc.desc: handle unfocused.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, HandleUnfocused_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleUnfocused_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    sptr<Rosen::FocusChangeInfo> focusChangeInfo;
    appMgrServiceInner->HandleUnfocused(focusChangeInfo);

    pid_t pid = 1;
    focusChangeInfo = new Rosen::FocusChangeInfo();
    appMgrServiceInner->HandleUnfocused(focusChangeInfo);

    focusChangeInfo->pid_ = pid;
    appMgrServiceInner->HandleUnfocused(focusChangeInfo);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo, "");
    EXPECT_NE(appRecord, nullptr);
    appRecord->GetPriorityObject()->SetPid(pid);
    appMgrServiceInner->HandleUnfocused(focusChangeInfo);

    TAG_LOGI(AAFwkTag::TEST, "HandleUnfocused_001 end");
}

/**
 * @tc.name: GetAppRunningStateByBundleName_001
 * @tc.desc: get app running state by bundle name.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, GetAppRunningStateByBundleName_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAppRunningStateByBundleName_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "test_bundleName";
    appMgrServiceInner->GetAppRunningStateByBundleName(bundleName);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->GetAppRunningStateByBundleName(bundleName);

    TAG_LOGI(AAFwkTag::TEST, "GetAppRunningStateByBundleName_001 end");
}

/**
 * @tc.name: NotifyLoadRepairPatch_001
 * @tc.desc: notify load repair patch.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, NotifyLoadRepairPatch_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyLoadRepairPatch_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "test_bundleName";
    sptr<IQuickFixCallback> callback;
    appMgrServiceInner->NotifyLoadRepairPatch(bundleName, callback);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->NotifyLoadRepairPatch(bundleName, callback);

    TAG_LOGI(AAFwkTag::TEST, "NotifyLoadRepairPatch_001 end");
}

/**
 * @tc.name: NotifyHotReloadPage_001
 * @tc.desc: notify hot reload page.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, NotifyHotReloadPage_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyHotReloadPage_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "test_bundleName";
    sptr<IQuickFixCallback> callback;
    appMgrServiceInner->NotifyHotReloadPage(bundleName, callback);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->NotifyHotReloadPage(bundleName, callback);

    TAG_LOGI(AAFwkTag::TEST, "NotifyHotReloadPage_001 end");
}

/**
 * @tc.name: SetContinuousTaskProcess_001
 * @tc.desc: set continuous task process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
HWTEST_F(AppMgrServiceInnerTest, SetContinuousTaskProcess_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "SetContinuousTaskProcess_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    int32_t ret = appMgrServiceInner->SetContinuousTaskProcess(0, true);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo, "");
    EXPECT_NE(appRecord, nullptr);
    appRecord->GetPriorityObject()->SetPid(0);
    ret = appMgrServiceInner->SetContinuousTaskProcess(0, true);
    EXPECT_EQ(ret, 0);

    appMgrServiceInner->appRunningManager_ = nullptr;
    ret = appMgrServiceInner->SetContinuousTaskProcess(0, true);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);

    TAG_LOGI(AAFwkTag::TEST, "SetContinuousTaskProcess_001 end");
}
#endif

/**
 * @tc.name: NotifyUnLoadRepairPatch_001
 * @tc.desc: notify unload repair patch.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, NotifyUnLoadRepairPatch_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyUnLoadRepairPatch_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "test_bundleName";
    sptr<IQuickFixCallback> callback;
    appMgrServiceInner->NotifyUnLoadRepairPatch(bundleName, callback);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->NotifyUnLoadRepairPatch(bundleName, callback);

    TAG_LOGI(AAFwkTag::TEST, "NotifyUnLoadRepairPatch_001 end");
}

/**
 * @tc.name: GetProcessMemoryByPid_001
 * @tc.desc: Get memorySize by pid.
 * @tc.type: FUNC
 * @tc.require: issueI76JBF
 */
HWTEST_F(AppMgrServiceInnerTest, GetProcessMemoryByPid_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "GetProcessMemoryByPid_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    int32_t pid = 0;
    int32_t memorySize = 0;
    int32_t ret = appMgrServiceInner->GetProcessMemoryByPid(pid, memorySize);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "GetProcessMemoryByPid_001 end");
}

/**
 * @tc.name: GetRunningProcessInformation_001
 * @tc.desc: Get application processes information list by bundleName.
 * @tc.type: FUNC
 * @tc.require: issueI76JBF
 */
HWTEST_F(AppMgrServiceInnerTest, GetRunningProcessInformation_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcessInformation_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "testBundleName";
    int32_t userId = 100;
    std::vector<RunningProcessInfo> info;
    int32_t ret = appMgrServiceInner->GetRunningProcessInformation(bundleName, userId, info);
    EXPECT_EQ(ret, ERR_OK);

    appMgrServiceInner->remoteClientManager_ = nullptr;
    ret = appMgrServiceInner->GetRunningProcessInformation(bundleName, userId, info);
    EXPECT_EQ(ret, ERR_NO_INIT);

    appMgrServiceInner->appRunningManager_ = nullptr;
    ret = appMgrServiceInner->GetRunningProcessInformation(bundleName, userId, info);
    EXPECT_EQ(ret, ERR_NO_INIT);

    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcessInformation_001 end");
}

/**
 * @tc.name: GetBundleNameByPid_001
 * @tc.desc: get bundle name by Pid.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, GetBundleNameByPid_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetBundleNameByPid_001 start");

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    int32_t pid = 0;
    std::string name = "test_name";
    int32_t uid = 0;
    auto ret  = appMgrServiceInner->GetBundleNameByPid(pid, name, uid);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);

    TAG_LOGI(AAFwkTag::TEST, "GetBundleNameByPid_001 end");
}

/**
 * @tc.name: GetBundleNameByPid_002
 * @tc.desc: get bundle name by Pid.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, GetBundleNameByPid_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetBundleNameByPid_002 start");

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    BundleInfo info;
    std::string processName = "test_processName";
    appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, info, "");
    int32_t pid = 0;
    std::string name = "test_name";
    int32_t uid = 0;
    auto ret  = appMgrServiceInner->GetBundleNameByPid(pid, name, uid);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "GetBundleNameByPid_002 end");
}

/**
 * @tc.name: AppRecoveryNotifyApp_001
 * @tc.desc: AppRecovery NotifyApp.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, AppRecoveryNotifyApp_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRecoveryNotifyApp_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    int32_t pid = 0;
    std::string bundleName = "com.is.hiserice";
    appMgrServiceInner->AppRecoveryNotifyApp(pid, bundleName, FaultDataType::RESOURCE_CONTROL, "appRecovery", -1);
    appMgrServiceInner->AppRecoveryNotifyApp(pid, bundleName, FaultDataType::APP_FREEZE, "recovery", -1);
    TAG_LOGI(AAFwkTag::TEST, "AppRecoveryNotifyApp_001 end");
}

/**
 * @tc.name: RecordAppRecoveryNotifyAppReason_001
 * @tc.desc: AppRecovery NotifyApp.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, RecordAppRecoveryNotifyAppReason_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RecordAppRecoveryNotifyAppReason_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    int pid = getpid();
    appMgrServiceInner->RecordAppRecoveryNotifyAppReason(pid);
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    int32_t recordId = RECORD_ID;
    std::string processName = "processName";
    std::shared_ptr<AppRunningRecord> appRecord = std::make_shared<AppRunningRecord>(info, recordId, processName);
    EXPECT_NE(appRecord, nullptr);
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    EXPECT_NE(appMgrServiceInner->appRunningManager_, nullptr);
    appMgrServiceInner->RecordAppRecoveryNotifyAppReason(pid);
    pid = 1;
    appMgrServiceInner->RecordAppRecoveryNotifyAppReason(pid);
    TAG_LOGI(AAFwkTag::TEST, "RecordAppRecoveryNotifyAppReason_001 end");
}

/**
 * @tc.name: NotifyAppFault_001
 * @tc.desc: Notify AppFault.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, NotifyAppFault_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyAppFault_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    FaultData faultData1;
    faultData1.errorObject.name = "1234";
    faultData1.timeoutMarkers = "456";
    int32_t ret1 = appMgrServiceInner->NotifyAppFault(faultData1);
    EXPECT_EQ(ret1, ERR_INVALID_VALUE);
}

/**
 * @tc.name: TimeoutNotifyApp_001
 * @tc.desc: Timeout Notify App.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, TimeoutNotifyApp_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TimeoutNotifyApp_001 start");
    std::shared_ptr<AppMgrServiceInner> appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::shared_ptr<MockTaskHandlerWrap> taskHandler = MockTaskHandlerWrap::CreateQueueHandler("app_mgr_task_queue");
    appMgrServiceInner->SetTaskHandler(taskHandler);

    int32_t pid = 0;
    int32_t uid = 0;
    std::string bundleName = "test_processName";
    std::string processName = "test_processName";
    FaultData faultData;
    faultData.errorObject.name = "1234";
    faultData.faultType = FaultDataType::APP_FREEZE;
    appMgrServiceInner->TimeoutNotifyApp(pid, uid, bundleName, processName, faultData, -1);
    EXPECT_NE(taskHandler, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "TimeoutNotifyApp_001 end");
}

/**
 * @tc.name: NotifyAppFaultBySA_001
 * @tc.desc: Notify Fault Data By SA
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, NotifyAppFaultBySA_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyAppFaultBySA_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    AppFaultDataBySA faultData;
    faultData.pid = 8142;
    faultData.errorObject.name = "appRecovery";
    int32_t ret = appMgrServiceInner->NotifyAppFaultBySA(faultData);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "NotifyAppFaultBySA_001 end");
}

/**
 * @tc.name: RegisterAppDebugListener_001
 * @tc.desc: Test the status of RegisterAppDebugListener.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, RegisterAppDebugListener_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    sptr<IAppDebugListener> listener = nullptr;
    appMgrServiceInner->appDebugManager_ = std::make_shared<AppDebugManager>();
    auto result = appMgrServiceInner->RegisterAppDebugListener(listener);
    EXPECT_EQ(result, ERR_INVALID_DATA);
    appMgrServiceInner->appDebugManager_ = nullptr;
    result = appMgrServiceInner->RegisterAppDebugListener(listener);
    EXPECT_EQ(result, ERR_NO_INIT);
}

/**
 * @tc.name: UnregisterAppDebugListener_001
 * @tc.desc: Test the status of UnregisterAppDebugListener.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, UnregisterAppDebugListener_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    sptr<IAppDebugListener> listener = nullptr;
    appMgrServiceInner->appDebugManager_ = std::make_shared<AppDebugManager>();
    auto result = appMgrServiceInner->UnregisterAppDebugListener(listener);
    EXPECT_EQ(result, ERR_INVALID_DATA);
    appMgrServiceInner->appDebugManager_ = nullptr;
    result = appMgrServiceInner->UnregisterAppDebugListener(listener);
    EXPECT_EQ(result, ERR_NO_INIT);
}

/**
 * @tc.name: AttachAppDebug_001
 * @tc.desc: Test the status of AttachAppDebug.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, AttachAppDebug_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::string bundleName;
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    appMgrServiceInner->appDebugManager_ = std::make_shared<AppDebugManager>();
    auto result = appMgrServiceInner->AttachAppDebug(bundleName, false);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AttachAppDebug_002
 * @tc.desc: Test the status of AttachAppDebug, check nullptr AppRunningManager.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, AttachAppDebug_002, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::string bundleName;
    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->appDebugManager_ = std::make_shared<AppDebugManager>();
    auto result = appMgrServiceInner->AttachAppDebug(bundleName, false);
    EXPECT_EQ(result, ERR_NO_INIT);
}

/**
 * @tc.name: DetachAppDebug_001
 * @tc.desc: Test the status of DetachAppDebug.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, DetachAppDebug_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::string bundleName;
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    appMgrServiceInner->appDebugManager_ = std::make_shared<AppDebugManager>();
    auto result = appMgrServiceInner->DetachAppDebug(bundleName);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: DetachAppDebug_002
 * @tc.desc: Test the status of DetachAppDebug, check nullptr AppRunningManager.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, DetachAppDebug_002, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::string bundleName;
    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->appDebugManager_ = std::make_shared<AppDebugManager>();
    auto result = appMgrServiceInner->DetachAppDebug(bundleName);
    EXPECT_EQ(result, ERR_NO_INIT);
}

/**
 * @tc.name: SetAppWaitingDebug_001
 * @tc.desc: Test function SetAppWaitingDebug.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, SetAppWaitingDebug_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::string bundleName("test");
    auto result = appMgrServiceInner->SetAppWaitingDebug(bundleName, false);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: CancelAppWaitingDebug_001
 * @tc.desc: Test function CancelAppWaitingDebug.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, CancelAppWaitingDebug_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    auto result = appMgrServiceInner->CancelAppWaitingDebug();
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: GetWaitingDebugApp_001
 * @tc.desc: Test function GetWaitingDebugApp.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, GetWaitingDebugApp_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::vector<std::string> debugInfoList;
    auto result = appMgrServiceInner->GetWaitingDebugApp(debugInfoList);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: RegisterAbilityDebugResponse_001
 * @tc.desc: Test the status of RegisterAbilityDebugResponse.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, RegisterAbilityDebugResponse_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    sptr<IAbilityDebugResponse> response = nullptr;
    appMgrServiceInner->RegisterAbilityDebugResponse(response);
    EXPECT_TRUE(appMgrServiceInner != nullptr);
}

/**
 * @tc.name: NotifyAbilitysDebugChange_001
 * @tc.desc: Test the status of NotifyAbilitiesDebugChange.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, NotifyAbilitysDebugChange_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::string bundleName;
    bool isAppDebug = true;
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    appMgrServiceInner->appDebugManager_ = std::make_shared<AppDebugManager>();
    auto result = appMgrServiceInner->NotifyAbilitiesDebugChange(bundleName, isAppDebug);
    EXPECT_EQ(result, ERR_NO_INIT);
}

/**
 * @tc.name: ProcessAppDebug_001
 * @tc.desc: Test the status of ProcessAppDebug.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, ProcessAppDebug_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    int32_t recordId = RECORD_ID;
    std::string processName = "processName";
    std::shared_ptr<AppRunningRecord> appRecord = std::make_shared<AppRunningRecord>(info, recordId, processName);
    bool isDebugStart = true;
    appRecord->SetDebugApp(false);
    appMgrServiceInner->ProcessAppDebug(appRecord, isDebugStart);
    EXPECT_EQ(appRecord->IsDebugApp(), true);
}

/**
 * @tc.name: MakeAppDebugInfo_001
 * @tc.desc: Test the status of MakeAppDebugInfo.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, MakeAppDebugInfo_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    int32_t recordId = RECORD_ID;
    std::string processName = "processName";
    std::shared_ptr<AppRunningRecord> appRecord = std::make_shared<AppRunningRecord>(info, recordId, processName);
    bool isDebugStart = true;
    appRecord->SetDebugApp(false);
    auto appDebugInfo = appMgrServiceInner->MakeAppDebugInfo(appRecord, isDebugStart);
    EXPECT_EQ(appDebugInfo.bundleName, "");
    EXPECT_EQ(appDebugInfo.pid, APP_DEBUG_INFO_PID);
    EXPECT_EQ(appDebugInfo.isDebugStart, true);
}

/**
 * @tc.name: ChangeAppGcState_001
 * @tc.desc: Change app Gc state
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, ChangeAppGcState_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ChangeAppGcState_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    int32_t pid = 0;
    int32_t state = 0;
    int32_t ret = appMgrServiceInner->ChangeAppGcState(pid, state);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "ChangeAppGcState_001 end");
}

/**
 * @tc.name: ChangeAppGcState_002
 * @tc.desc: Change app Gc state
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, ChangeAppGcState_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ChangeAppGcState_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    int32_t pid = 0;
    int32_t state = 0;
    uint64_t tid = 1;
    int32_t ret = appMgrServiceInner->ChangeAppGcState(pid, state, tid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "ChangeAppGcState_002 end");
}

/**
 * @tc.name: SendReStartProcessEvent_001
 * @tc.desc: Change app Gc state
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, SendReStartProcessEvent_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendReStartProcessEvent_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    AAFwk::EventInfo eventInfo;
    appMgrServiceInner->SendReStartProcessEvent(eventInfo, 0);
    TAG_LOGI(AAFwkTag::TEST, "SendReStartProcessEvent_001 end");
}

/**
 * @tc.name: SendReStartProcessEvent_002
 * @tc.desc: Change app Gc state
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, SendReStartProcessEvent_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendReStartProcessEvent_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    AAFwk::EventInfo eventInfo;
    BundleInfo info;
    std::string processName = "test_processName";
    auto record =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, info, "");
    recordId_ += 1;
    int64_t restartTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
        system_clock::now().time_since_epoch()).count();
    int64_t killedTime = restartTime - 3000;
    appMgrServiceInner->killedProcessMap_.emplace(killedTime, processName);
    appMgrServiceInner->SendReStartProcessEvent(eventInfo, record->GetUid());
    TAG_LOGI(AAFwkTag::TEST, "SendReStartProcessEvent_002 end");
}

/**
 * @tc.name: SendReStartProcessEvent_003
 * @tc.desc: Change app Gc state
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, SendReStartProcessEvent_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendReStartProcessEvent_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    AAFwk::EventInfo eventInfo;
    eventInfo.bundleName = "bundleName";
    eventInfo.callerBundleName = "callerBundleName";
    BundleInfo info;
    std::string processName = "test_processName";
    auto record =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, info, "");
    recordId_ += 1;
    int64_t restartTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
        system_clock::now().time_since_epoch()).count();
    int64_t killedTime = restartTime - 1000;
    appMgrServiceInner->killedProcessMap_.emplace(killedTime, processName);
    appMgrServiceInner->SendReStartProcessEvent(eventInfo, record->GetUid());
    TAG_LOGI(AAFwkTag::TEST, "SendReStartProcessEvent_003 end");
}

/**
 * @tc.name: SendReStartProcessEvent_004
 * @tc.desc: Change app Gc state
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, SendReStartProcessEvent_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendReStartProcessEvent_004 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    AAFwk::EventInfo eventInfo;
    BundleInfo info;
    std::string processName = "test_processName";
    eventInfo.bundleName = "bundleName";
    eventInfo.callerBundleName = "bundleName";
    eventInfo.callerProcessName = processName;
    auto record =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, info, "");
    recordId_ += 1;
    int64_t restartTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
        system_clock::now().time_since_epoch()).count();
    int64_t killedTime = restartTime - 1000;
    appMgrServiceInner->killedProcessMap_.emplace(killedTime, processName);
    appMgrServiceInner->SendReStartProcessEvent(eventInfo, record->GetUid());
    TAG_LOGI(AAFwkTag::TEST, "SendReStartProcessEvent_004 end");
}

/**
 * @tc.name: SendReStartProcessEvent_005
 * @tc.desc: Change app Gc state
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, SendReStartProcessEvent_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendReStartProcessEvent_005 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    AAFwk::EventInfo eventInfo;
    BundleInfo info;
    std::string processName = "test_processName";
    eventInfo.bundleName = "bundleName";
    eventInfo.callerBundleName = "bundleName";
    eventInfo.callerProcessName = "processName";
    auto record =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, info, "");
    recordId_ += 1;
    int64_t restartTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
        system_clock::now().time_since_epoch()).count();
    int64_t killedTime = restartTime - 1000;
    appMgrServiceInner->killedProcessMap_.emplace(killedTime, processName);
    appMgrServiceInner->SendReStartProcessEvent(eventInfo, record->GetUid());
    TAG_LOGI(AAFwkTag::TEST, "SendReStartProcessEvent_005 end");
}

/**
 * @tc.name: SendAppLaunchEvent_001
 * @tc.desc: launch application.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, SendAppLaunchEvent_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "SendAppLaunchEvent_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->SendAppLaunchEvent(nullptr);
    BundleInfo info;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, info, "");
    recordId_ += 1;
    std::shared_ptr<AppRunningRecord> appRecord2 =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, info, "");
    recordId_ += 1;
    appRecord->SetState(ApplicationState::APP_STATE_CREATE);
    appRecord->SetKeepAliveEnableState(false);
    appRecord->SetKeepAliveDkv(false);
    appRecord->SetEmptyKeepAliveAppState(false);
    Want want;
    appRecord->SetSpecifiedAbilityFlagAndWant(-1, want, "");
    appMgrServiceInner->SendAppLaunchEvent(appRecord);
    appRecord->SetCallerPid(appRecord2->GetPriorityObject()->GetPid());
    appMgrServiceInner->SendAppLaunchEvent(appRecord);
    appRecord->appInfo_ = nullptr;
    appRecord2->appInfo_ = nullptr;
    appMgrServiceInner->SendAppLaunchEvent(appRecord);
    TAG_LOGI(AAFwkTag::TEST, "SendAppLaunchEvent_001 end");
}

HWTEST_F(AppMgrServiceInnerTest, IsMainProcess_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "IsMainProcess_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module123";
    applicationInfo_->process = "";
    EXPECT_EQ(appMgrServiceInner->IsMainProcess(nullptr, ""), true);
    EXPECT_EQ(appMgrServiceInner->IsMainProcess(applicationInfo_, ""), false);
    EXPECT_EQ(appMgrServiceInner->IsMainProcess(applicationInfo_, "processName1"), false);
    EXPECT_EQ(appMgrServiceInner->IsMainProcess(applicationInfo_, applicationInfo_->bundleName), true);
    applicationInfo_->process = "processName2";
    EXPECT_EQ(appMgrServiceInner->IsMainProcess(applicationInfo_, applicationInfo_->bundleName), false);
    EXPECT_EQ(appMgrServiceInner->IsMainProcess(applicationInfo_, "processName2"), true);
    applicationInfo_->process = "";

    TAG_LOGI(AAFwkTag::TEST, "IsMainProcess_001 end");
}

/**
 * @tc.name: IsApplicationRunning_001
 * @tc.desc: Obtain application running status through bundleName.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, IsApplicationRunning_001, TestSize.Level1)
{
    AAFwk::IsMockSaCall::IsMockSpecificSystemAbilityAccessPermission();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::string bundleName = "com.is.hiserice";
    std::string processName = "test_processName";
    bool isRunning = false;
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo_, ++recordId_, processName);
    EXPECT_NE(appRecord, nullptr);
    appRecord->mainBundleName_ = "com.is.hiserice";
    appMgrServiceInner->appRunningManager_->appRunningRecordMap_.emplace(recordId_, appRecord);
    int32_t ret = appMgrServiceInner->IsApplicationRunning(bundleName, isRunning);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_TRUE(isRunning);
}

/**
 * @tc.name: IsApplicationRunning_002
 * @tc.desc: Not passing in bundleName, unable to obtain application running status.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, IsApplicationRunning_002, TestSize.Level1)
{
    AAFwk::IsMockSaCall::IsMockSpecificSystemAbilityAccessPermission();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::string bundleName = "com.is.hiserice";
    std::string processName = "test_processName";
    bool isRunning = false;
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo_, ++recordId_, processName);
    EXPECT_NE(appRecord, nullptr);
    appMgrServiceInner->appRunningManager_->appRunningRecordMap_.emplace(recordId_, appRecord);
    int32_t ret = appMgrServiceInner->IsApplicationRunning(bundleName, isRunning);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_FALSE(isRunning);
}

/**
 * @tc.name: InitWindowVisibilityChangedListener_001
 * @tc.desc: init windowVisibilityChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, InitWindowVisibilityChangedListener_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitWindowVisibilityChangedListener_001 start" ;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->InitWindowVisibilityChangedListener();
    EXPECT_NE(appMgrServiceInner->windowVisibilityChangedListener_, nullptr);
    GTEST_LOG_(INFO) << "InitWindowVisibilityChangedListener_001 end";
}

/**
 * @tc.name: FreeWindowVisibilityChangedListener_001
 * @tc.desc: free windowVisibilityChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, FreeWindowVisibilityChangedListener_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FreeWindowVisibilityChangedListener_001 start";
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->FreeWindowVisibilityChangedListener();
    EXPECT_EQ(appMgrServiceInner->windowVisibilityChangedListener_, nullptr);
    GTEST_LOG_(INFO) << "FreeWindowVisibilityChangedListener_001 end";
}

/**
 * @tc.name: HandleWindowVisibilityChanged_001
 * @tc.desc: handle window visibility changed
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, HandleWindowVisibilityChanged_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleWindowVisibilityChanged_001 start";
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::vector<sptr<Rosen::WindowVisibilityInfo>> visibilityInfos;
    appMgrServiceInner->HandleWindowVisibilityChanged(visibilityInfos);
    EXPECT_NE(appMgrServiceInner, nullptr);
    GTEST_LOG_(INFO) << "HandleWindowVisibilityChanged_001 end";
}

/**
 * @tc.name: InitWindowPidVisibilityChangedListener_001
 * @tc.desc: init windowPidVisibilityChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, InitWindowPidVisibilityChangedListener_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitWindowPidVisibilityChangedListener_001 start" ;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->FreeWindowPidVisibilityChangedListener();
    EXPECT_EQ(appMgrServiceInner->windowPidVisibilityChangedListener_, nullptr);
    appMgrServiceInner->InitWindowPidVisibilityChangedListener();
    EXPECT_NE(appMgrServiceInner->windowPidVisibilityChangedListener_, nullptr);

    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_ = nullptr;
    appMgrServiceInner->SetTaskHandler(taskHandler_);
    appMgrServiceInner->InitWindowPidVisibilityChangedListener();
    EXPECT_EQ(appMgrServiceInner->taskHandler_, nullptr);
    GTEST_LOG_(INFO) << "InitWindowPidVisibilityChangedListener_001 end";
}

/**
 * @tc.name: FreeWindowPidVisibilityChangedListener_001
 * @tc.desc: free windowPidVisibilityChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, FreeWindowPidVisibilityChangedListener_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FreeWindowPidVisibilityChangedListener_001 start";
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->FreeWindowPidVisibilityChangedListener();
    EXPECT_EQ(appMgrServiceInner->windowPidVisibilityChangedListener_, nullptr);

    appMgrServiceInner->FreeWindowPidVisibilityChangedListener();
    GTEST_LOG_(INFO) << "FreeWindowPidVisibilityChangedListener_001 end";
}

/**
 * @tc.name: HandleWindowPidVisibilityChanged_001
 * @tc.desc: handle window pid visibility changed
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, HandleWindowPidVisibilityChanged_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleWindowPidVisibilityChanged_001 start";
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    sptr<Rosen::WindowPidVisibilityInfo> windowPidVisibilityInfo;
    appMgrServiceInner->HandleWindowPidVisibilityChanged(windowPidVisibilityInfo);
    EXPECT_NE(appMgrServiceInner, nullptr);
    GTEST_LOG_(INFO) << "HandleWindowPidVisibilityChanged_001 end";
}

/**
 * @tc.name: IsAppRunning_001
 * @tc.desc: Obtain application running status through bundleName.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, IsAppRunning_001, TestSize.Level1)
{
    AAFwk::IsMockSaCall::IsMockSpecificSystemAbilityAccessPermission();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::string bundleName = "com.is.hiserice";
    std::string processName = "test_processName";
    int32_t appCloneIndex = 0;
    bool isRunning = false;
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo_, ++recordId_, processName);
    EXPECT_NE(appRecord, nullptr);
    appRecord->mainBundleName_ = "com.is.hiserice";
    appMgrServiceInner->appRunningManager_->appRunningRecordMap_.emplace(recordId_, appRecord);
    int32_t ret = appMgrServiceInner->IsAppRunning(bundleName, appCloneIndex, isRunning);
    EXPECT_EQ(ret, AAFwk::ERR_APP_CLONE_INDEX_INVALID);
    EXPECT_FALSE(isRunning);
}

/**
 * @tc.name: IsAppRunning_002
 * @tc.desc: Not passing in bundleName, unable to obtain application running status.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, IsAppRunning_002, TestSize.Level1)
{
    AAFwk::IsMockSaCall::IsMockSpecificSystemAbilityAccessPermission();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::string bundleName = "com.is.hiserice";
    std::string processName = "test_processName";
    int32_t appCloneIndex = 0;
    bool isRunning = false;
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo_, ++recordId_, processName);
    EXPECT_NE(appRecord, nullptr);
    appMgrServiceInner->appRunningManager_->appRunningRecordMap_.emplace(recordId_, appRecord);
    int32_t ret = appMgrServiceInner->IsAppRunning(bundleName, appCloneIndex, isRunning);
    EXPECT_EQ(ret, AAFwk::ERR_APP_CLONE_INDEX_INVALID);
    EXPECT_FALSE(isRunning);
}

/**
 * @tc.name: RegisterAbilityForegroundStateObserver_0100
 * @tc.desc: Verify it when observer is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, RegisterAbilityForegroundStateObserver_0100, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    auto res = appMgrServiceInner->RegisterAbilityForegroundStateObserver(nullptr);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: UnregisterAbilityForegroundStateObserver_0100
 * @tc.desc: Verify it when observer is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, UnregisterAbilityForegroundStateObserver_0100, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    auto res = appMgrServiceInner->UnregisterAbilityForegroundStateObserver(nullptr);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: RegisterAppForegroundStateObserver_0100
 * @tc.desc: Test the return when observer is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, RegisterAppForegroundStateObserver_0100, TestSize.Level1)
{
    sptr<IAppForegroundStateObserver> observer = nullptr;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto res = appMgrServiceInner->RegisterAppForegroundStateObserver(observer);
    EXPECT_EQ(ERR_INVALID_VALUE, res);
}

/**
 * @tc.name: UnregisterAppForegroundStateObserver_0100
 * @tc.desc: Test the return when observer is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, UnregisterAppForegroundStateObserver_0100, TestSize.Level1)
{
    sptr<IAppForegroundStateObserver> observer = nullptr;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto res = appMgrServiceInner->RegisterAppForegroundStateObserver(observer);
    EXPECT_EQ(ERR_INVALID_VALUE, res);
}

/**
 * @tc.name: RegisterStateStateObserver_0100
 * @tc.desc: Test unregister by nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, RegisterRenderStateObserver_0100, TestSize.Level1)
{
    sptr<IRenderStateObserver> observer = nullptr;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto res = appMgrServiceInner->RegisterRenderStateObserver(observer);
    EXPECT_EQ(ERR_INVALID_VALUE, res);
}

/**
 * @tc.name: RegisterStateStateObserver_0200
 * @tc.desc: Test unregister without permission.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, RegisterRenderStateObserver_0200, TestSize.Level1)
{
    AAFwk::IsMockSaCall::IsMockSaCallWithPermission();
    sptr<IRenderStateObserver> observer = new (std::nothrow) RenderStateObserverMock();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto res = appMgrServiceInner->RegisterRenderStateObserver(observer);
    EXPECT_EQ(ERR_OK, res);
}

/**
 * @tc.name: UnregisterRenderStateObserver_0100
 * @tc.desc: Test unregister by nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, UnregisterRenderStateObserver_0100, TestSize.Level1)
{
    sptr<IRenderStateObserver> observer = nullptr;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto res = appMgrServiceInner->RegisterRenderStateObserver(observer);
    EXPECT_EQ(ERR_INVALID_VALUE, res);
}

/**
 * @tc.name: UnregisterRenderStateObserver_0200
 * @tc.desc: Test unregister without permission.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, UnregisterRenderStateObserver_0200, TestSize.Level1)
{
    AAFwk::IsMockSaCall::IsMockSaCallWithPermission();
    sptr<IRenderStateObserver> observer = new (std::nothrow) RenderStateObserverMock();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto res = appMgrServiceInner->RegisterRenderStateObserver(observer);
    EXPECT_EQ(ERR_OK, res);
}

/**
 * @tc.name: GetAllUIExtensionRootHostPid_0100
 * @tc.desc: Get all ui extension root host pid.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, GetAllUIExtensionRootHostPid_0100, TestSize.Level1)
{
    AAFwk::IsMockSaCall::IsMockSpecificSystemAbilityAccessPermission();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);
    pid_t pid = 0;
    std::vector<pid_t> hostPids;
    auto ret = appMgrServiceInner->GetAllUIExtensionRootHostPid(pid, hostPids);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: GetAllUIExtensionProviderPid_0100
 * @tc.desc: Get all ui extension provider pid.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, GetAllUIExtensionProviderPid_0100, TestSize.Level1)
{
    AAFwk::IsMockSaCall::IsMockSpecificSystemAbilityAccessPermission();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);
    pid_t hostPid = 0;
    std::vector<pid_t> providerPids;
    auto ret = appMgrServiceInner->GetAllUIExtensionProviderPid(hostPid, providerPids);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: AddUIExtensionLauncherItem_0100
 * @tc.desc: Add ui extension launcher item.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, AddUIExtensionLauncherItem_0100, TestSize.Level1)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);

    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    ASSERT_NE(want, nullptr);
    want->SetParam("ability.want.params.uiExtensionAbilityId", 1);
    want->SetParam("ability.want.params.uiExtensionRootHostPid", 1000);

    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    ASSERT_NE(appInfo, nullptr);
    int32_t recordId = 0;
    std::string processName = "";
    std::shared_ptr<AppRunningRecord> appRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    ASSERT_NE(appRecord, nullptr);
    appRecord->GetPriorityObject()->SetPid(1001);

    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());

    appMgrServiceInner->AddUIExtensionLauncherItem(want, appRecord, token);
    // check want param has been erased.
    EXPECT_EQ(want->HasParameter("ability.want.params.uiExtensionAbilityId"), false);
    EXPECT_EQ(want->HasParameter("ability.want.params.uiExtensionRootHostPid"), false);
    appMgrServiceInner->RemoveUIExtensionLauncherItem(appRecord, token);
}

/**
 * @tc.name: PreloadApplication_0100
 * @tc.desc: Preload Application.
 * @tc.type: FUNC
 * @tc.Function: PreloadApplication
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, PreloadApplication_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "com.acts.preloadtest";
    int32_t userId = 100;
    PreloadMode preloadMode = PreloadMode::PRE_MAKE;
    int32_t appIndex = 0;
    int32_t ret = appMgrServiceInner->PreloadApplication(bundleName, userId, preloadMode, appIndex);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_0100 end");
}

/**
 * @tc.name: PreloadApplication_0200
 * @tc.desc: Preload Application.
 * @tc.type: FUNC
 * @tc.Function: PreloadApplication
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, PreloadApplication_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_0200 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "";
    int32_t userId = 100;
    PreloadMode preloadMode = PreloadMode::PRE_MAKE;
    int32_t appIndex = 0;
    int32_t ret = appMgrServiceInner->PreloadApplication(bundleName, userId, preloadMode, appIndex);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_0200 end");
}

/**
 * @tc.name: PreloadApplication_0300
 * @tc.desc: Preload Application.
 * @tc.type: FUNC
 * @tc.Function: PreloadApplication
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, PreloadApplication_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_0300 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "com.acts.preloadtest";
    int32_t userId = 100;
    PreloadMode preloadMode = PreloadMode::PRESS_DOWN;
    int32_t appIndex = 0;
    int32_t ret = appMgrServiceInner->PreloadApplication(bundleName, userId, preloadMode, appIndex);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_0300 end");
}

/**
 * @tc.name: PreloadApplication_0400
 * @tc.desc: Preload Application.
 * @tc.type: FUNC
 * @tc.Function: PreloadApplication
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, PreloadApplication_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_0400 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "";
    int32_t userId = 100;
    PreloadMode preloadMode = PreloadMode::PRESS_DOWN;
    int32_t appIndex = 0;
    int32_t ret = appMgrServiceInner->PreloadApplication(bundleName, userId, preloadMode, appIndex);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_0400 end");
}

/**
 * @tc.name: PreloadApplication_0500
 * @tc.desc: Preload Application.
 * @tc.type: FUNC
 * @tc.Function: PreloadApplication
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, PreloadApplication_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_0500 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "com.acts.preloadtest";
    int32_t userId = 1;
    PreloadMode preloadMode = PreloadMode::PRE_MAKE;
    int32_t appIndex = 0;
    int32_t ret = appMgrServiceInner->PreloadApplication(bundleName, userId, preloadMode, appIndex);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_0500 end");
}

/**
 * @tc.name: PreloadApplication_0600
 * @tc.desc: Preload Application.
 * @tc.type: FUNC
 * @tc.Function: PreloadApplication
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, PreloadApplication_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_0600 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "com.acts.preloadtest";
    int32_t userId = 1;
    PreloadMode preloadMode = PreloadMode::PRESS_DOWN;
    int32_t appIndex = 0;
    int32_t ret = appMgrServiceInner->PreloadApplication(bundleName, userId, preloadMode, appIndex);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_0600 end");
}

/**
 * @tc.name: PreloadApplication_0700
 * @tc.desc: Preload Application.
 * @tc.type: FUNC
 * @tc.Function: PreloadApplication
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, PreloadApplication_0700, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_0700 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "";
    int32_t userId = 1;
    PreloadMode preloadMode = PreloadMode::PRE_MAKE;
    int32_t appIndex = 0;
    int32_t ret = appMgrServiceInner->PreloadApplication(bundleName, userId, preloadMode, appIndex);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_0700 end");
}

/**
 * @tc.name: PreloadApplication_0800
 * @tc.desc: Preload Application.
 * @tc.type: FUNC
 * @tc.Function: PreloadApplication
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, PreloadApplication_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_0800 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "";
    int32_t userId = 1;
    PreloadMode preloadMode = PreloadMode::PRESS_DOWN;
    int32_t appIndex = 0;
    int32_t ret = appMgrServiceInner->PreloadApplication(bundleName, userId, preloadMode, appIndex);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_0800 end");
}

/**
 * @tc.name: PreloadApplication_0900
 * @tc.desc: Preload Application.
 * @tc.type: FUNC
 * @tc.Function: PreloadApplication
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, PreloadApplication_0900, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_0900 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "com.acts.preloadtest";
    int32_t userId = 0;
    PreloadMode preloadMode = PreloadMode::PRE_MAKE;
    int32_t appIndex = 1;
    int32_t ret = appMgrServiceInner->PreloadApplication(bundleName, userId, preloadMode, appIndex);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_0900 end");
}

/**
 * @tc.name: PreloadApplication_1000
 * @tc.desc: Preload Application.
 * @tc.type: FUNC
 * @tc.Function: PreloadApplication
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, PreloadApplication_1000, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_1000 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "";
    int32_t userId = 0;
    PreloadMode preloadMode = PreloadMode::PRE_MAKE;
    int32_t appIndex = 1;
    int32_t ret = appMgrServiceInner->PreloadApplication(bundleName, userId, preloadMode, appIndex);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_1000 end");
}

/**
 * @tc.name: PreloadApplication_1100
 * @tc.desc: Preload Application.
 * @tc.type: FUNC
 * @tc.Function: PreloadApplication
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, PreloadApplication_1100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_1100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "com.acts.preloadtest";
    int32_t userId = 0;
    PreloadMode preloadMode = PreloadMode::PRESS_DOWN;
    int32_t appIndex = 1;
    int32_t ret = appMgrServiceInner->PreloadApplication(bundleName, userId, preloadMode, appIndex);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_1100 end");
}

/**
 * @tc.name: PreloadApplication_1200
 * @tc.desc: Preload Application.
 * @tc.type: FUNC
 * @tc.Function: PreloadApplication
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, PreloadApplication_1200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_1200 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "";
    int32_t userId = 0;
    PreloadMode preloadMode = PreloadMode::PRESS_DOWN;
    int32_t appIndex = 1;
    int32_t ret = appMgrServiceInner->PreloadApplication(bundleName, userId, preloadMode, appIndex);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "PreloadApplication_1200 end");
}

/**
 * @tc.name: LPreloadModuleFinished_0001
 * @tc.desc: PreloadModuleFinished
 * @tc.type: FUNC
 * @tc.Function: PreloadModuleFinished
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, PreloadModuleFinished_0001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadModuleFinished_0001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    int32_t appRecord = 0;

    std::shared_ptr<MockTaskHandlerWrap> taskHandler = MockTaskHandlerWrap::CreateQueueHandler("app_mgr_tasks_queue");
    EXPECT_CALL(*taskHandler, SubmitTaskInner(_, _)).Times(AnyNumber());
    appMgrServiceInner->SetTaskHandler(taskHandler);

    appMgrServiceInner->PreloadModuleFinished(appRecord);
    TAG_LOGI(AAFwkTag::TEST, "PreloadModuleFinished_0001 end");
}

/**
 * @tc.name: SetSupportedProcessCacheSelf_001
 * @tc.desc: The application sets itself whether or not to support process cache.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, SetSupportedProcessCacheSelf_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "SetSupportedProcessCacheSelf_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    bool isSupported = false;
    EXPECT_EQ(appMgrServiceInner->SetSupportedProcessCacheSelf(isSupported), ERR_INVALID_VALUE);

    appMgrServiceInner->appRunningManager_ = nullptr;
    EXPECT_EQ(appMgrServiceInner->SetSupportedProcessCacheSelf(isSupported), ERR_NO_INIT);

    TAG_LOGI(AAFwkTag::TEST, "SetSupportedProcessCacheSelf_001 end");
}

/**
 * @tc.name: OnAppCacheStateChanged_001
 * @tc.desc: on application cache state changed.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, OnAppCacheStateChanged_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "OnAppCacheStateChanged_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->OnAppCacheStateChanged(nullptr, ApplicationState::APP_STATE_CACHED);

    std::string bundleName = "com.is.hiserice";
    std::string processName = "test_processName";
    bool isRunning = false;
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo_, ++recordId_, processName);
    EXPECT_NE(appRecord, nullptr);
    appRecord->mainBundleName_ = "com.is.hiserice";
    appRecord->SetState(ApplicationState::APP_STATE_CACHED);

    appRecord->priorityObject_ = nullptr;
    appMgrServiceInner->OnAppCacheStateChanged(appRecord, ApplicationState::APP_STATE_CACHED);

    appRecord->priorityObject_ = std::make_shared<PriorityObject>();
    appMgrServiceInner->OnAppCacheStateChanged(appRecord, ApplicationState::APP_STATE_CACHED);


    TAG_LOGI(AAFwkTag::TEST, "OnAppCacheStateChanged_001 end");
}

/**
 * @tc.name: GetRunningMultiAppInfoByBundleName_001
 * @tc.desc: Get multiApp information list by bundleName.
 * @tc.type: FUNC
 * @tc.require: issueI9HMAO
 */
HWTEST_F(AppMgrServiceInnerTest, GetRunningMultiAppInfoByBundleName_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetRunningMultiAppInfoByBundleName_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "testBundleName";
    RunningMultiAppInfo info;
    int32_t ret = appMgrServiceInner->GetRunningMultiAppInfoByBundleName(bundleName, info);
    EXPECT_NE(ret, ERR_OK);

    appMgrServiceInner->remoteClientManager_ = nullptr;
    ret = appMgrServiceInner->GetRunningMultiAppInfoByBundleName(bundleName, info);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "GetRunningMultiAppInfoByBundleName_001 end");
}

/**
 * @tc.name: GetRunningMultiAppInfoByBundleName_002
 * @tc.desc: Get multiApp information list by bundleName.
 * @tc.type: FUNC
 * @tc.require: issueI9HMAO
 */
HWTEST_F(AppMgrServiceInnerTest, GetRunningMultiAppInfoByBundleName_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetRunningMultiAppInfoByBundleName_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "";
    RunningMultiAppInfo info;
    int32_t ret = appMgrServiceInner->GetRunningMultiAppInfoByBundleName(bundleName, info);
    EXPECT_EQ(ret, AAFwk::INVALID_PARAMETERS_ERR);

    TAG_LOGI(AAFwkTag::TEST, "GetRunningMultiAppInfoByBundleName_002 end");
}

/**
 * @tc.name: GetAllRunningInstanceKeysBySelf_001
 * @tc.desc: GetAllRunningInstanceKeysBySelf.
 * @tc.type: FUNC
 * @tc.require: issueI9HMAO
 */
HWTEST_F(AppMgrServiceInnerTest, GetAllRunningInstanceKeysBySelf_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningInstanceKeysBySelf_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::vector<std::string> instanceKeys;
    int32_t ret = appMgrServiceInner->GetAllRunningInstanceKeysBySelf(instanceKeys);
    EXPECT_NE(ret, ERR_OK);

    appMgrServiceInner->remoteClientManager_ = nullptr;
    ret = appMgrServiceInner->GetAllRunningInstanceKeysBySelf(instanceKeys);
    EXPECT_EQ(ret, ERR_NO_INIT);

    appMgrServiceInner->remoteClientManager_ = std::make_shared<RemoteClientManager>();
    ret = appMgrServiceInner->GetAllRunningInstanceKeysBySelf(instanceKeys);
    EXPECT_NE(ret, ERR_NO_INIT);

    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningInstanceKeysBySelf_001 end");
}

/**
 * @tc.name: GetAllRunningInstanceKeysByBundleName_001
 * @tc.desc: GetAllRunningInstanceKeysByBundleName.
 * @tc.type: FUNC
 * @tc.require: issueI9HMAO
 */
HWTEST_F(AppMgrServiceInnerTest, GetAllRunningInstanceKeysByBundleName_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningInstanceKeysByBundleName_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "testBundleName";
    std::vector<std::string> instanceKeys;
    int32_t userId = 100;
    int32_t ret = appMgrServiceInner->GetAllRunningInstanceKeysByBundleName(bundleName, instanceKeys, userId);
    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningInstanceKeysByBundleName_001 end");
}

/**
 * @tc.name: SendCreateAtomicServiceProcessEvent_001
 * @tc.desc: Report event of create atomic service process.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, SendCreateAtomicServiceProcessEvent_001, TestSize.Level1)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::string processName = "test_processName";
    std::string moduleName = "test_modulenName";
    std::string abilityName = "test_abilityName";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo_, ++recordId_, processName);
    auto bundleType = BundleType::ATOMIC_SERVICE;
    auto ret = appMgrServiceInner->SendCreateAtomicServiceProcessEvent(nullptr, bundleType, moduleName, abilityName);
    EXPECT_EQ(ret, false);
    ret = appMgrServiceInner->SendCreateAtomicServiceProcessEvent(appRecord, bundleType, moduleName, abilityName);
    EXPECT_EQ(ret, true);
    bundleType = BundleType::APP;
    ret = appMgrServiceInner->SendCreateAtomicServiceProcessEvent(appRecord, bundleType, moduleName, abilityName);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: AttachedToStatusBar_001
 * @tc.desc: Attach one ability to status bar.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, AttachedToStatusBar_001, TestSize.Level1)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->AttachedToStatusBar(nullptr);

    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    appMgrServiceInner->AttachedToStatusBar(token);

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    std::shared_ptr<AAFwk::Want> want;
    std::string processName = "test_processName";
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token;
    std::shared_ptr<AppRunningRecord> appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam,
    applicationInfo_, abilityInfo_, processName, bundleInfo, hapModuleInfo, want);
    EXPECT_NE(appRecord, nullptr);
    appMgrServiceInner->AttachedToStatusBar(token);
}

/**
 * @tc.name: BlockProcessCacheByPids_001
 * @tc.desc: Block process cache feature using pids.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, BlockProcessCacheByPids_001, TestSize.Level1)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    BundleInfo info;
    std::string processName = "test_processName";
    auto record =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, info, "");
    std::shared_ptr<PriorityObject> priorityObject = std::make_shared<PriorityObject>();
    EXPECT_NE(priorityObject, nullptr);
    std::string callerBundleName = "callerBundleName";
    priorityObject->SetPid(2);
    record->priorityObject_ = priorityObject;
    record->mainBundleName_ = callerBundleName;
    record->SetCallerPid(1);

    std::vector<int32_t> pids{2};
    appMgrServiceInner->BlockProcessCacheByPids(pids);
}

/**
 * @tc.name: GetSupportedProcessCachePids_001
 * @tc.desc: Get pids of processes which belong to specific bundle name and support process cache feature.
 * @tc.type: FUNC
 * @tc.require: issueI76JBF
 */
HWTEST_F(AppMgrServiceInnerTest, GetSupportedProcessCachePids_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "GetSupportedProcessCachePids_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "testBundleName";
    std::vector<int32_t> pidList;
    int32_t ret = appMgrServiceInner->GetSupportedProcessCachePids(bundleName, pidList);
    EXPECT_EQ(ret, ERR_OK);

    appMgrServiceInner->appRunningManager_ = nullptr;
    ret = appMgrServiceInner->GetSupportedProcessCachePids(bundleName, pidList);
    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "GetSupportedProcessCachePids_001 end");
}

/**
 * @tc.name: RegisterKiaInterceptor_001
 * @tc.desc: verify RegisterKiaInterceptor.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, RegisterKiaInterceptor_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "RegisterKiaInterceptor_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    sptr<IKiaInterceptor> interceptor = new MockKiaInterceptor();
    appMgrServiceInner->RegisterKiaInterceptor(interceptor);

    TAG_LOGI(AAFwkTag::TEST, "RegisterKiaInterceptor_001 end");
}

/**
 * @tc.name: CheckIsKiaProcess_001
 * @tc.desc: verify CheckIsKiaProcess.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, CheckIsKiaProcess_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckIsKiaProcess_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    pid_t pid = 1234;
    bool isKia = false;
    appMgrServiceInner->CheckIsKiaProcess(pid, isKia);

    TAG_LOGI(AAFwkTag::TEST, "CheckIsKiaProcess_001 end");
}

/**
 * @tc.name: SetJITPermissions_001
 * @tc.desc: set jit permissions.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, SetJITPermissions_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "SetJITPermissions_001 start");
    uint32_t accessTokenId = 0;
    AppSpawnStartMsg startMsg = {0};
    std::vector<std::string> permissionsList;
    AppspawnUtil::SetJITPermissions(accessTokenId, permissionsList);
    EXPECT_EQ(permissionsList.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "SetJITPermissions_001 end");
}

/**
 * @tc.name: SendAppSpawnUninstallDebugHapMsg_001
 * @tc.desc: SendAppSpawnUninstallDebugHapMsg
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, SendAppSpawnUninstallDebugHapMsg_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "SendAppSpawnUninstallDebugHapMsg_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    appMgrServiceInner->SendAppSpawnUninstallDebugHapMsg(0);
    TAG_LOGI(AAFwkTag::TEST, "SendAppSpawnUninstallDebugHapMsg_001 end");
}

/**
 * @tc.name: DoAllProcessExitCallback_001
 * @tc.desc: DoAllProcessExitCallback
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, DoAllProcessExitCallback_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "DoAllProcessExitCallback_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::list<SimpleProcessInfo> processInfos;
    int32_t userId = 100;
    sptr<AAFwk::IUserCallback> callback = nullptr;
    int64_t startTime = 0;
    auto ret = appMgrServiceInner->DoAllProcessExitCallback(processInfos, userId, callback, startTime);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "DoAllProcessExitCallback_001 end");
}

/**
 * @tc.name: DoAllProcessExitCallback_002
 * @tc.desc: DoAllProcessExitCallback
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, DoAllProcessExitCallback_002, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "DoAllProcessExitCallback_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::list<SimpleProcessInfo> processInfos;
    int32_t userId = 100;
    sptr<AAFwk::IUserCallback> callback = new MockIUserCallback();
    int64_t startTime = 0;
    auto ret = appMgrServiceInner->DoAllProcessExitCallback(processInfos, userId, callback, startTime);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "DoAllProcessExitCallback_002 end");
}

/**
 * @tc.name: IsProcessCacheSupported_001
 * @tc.desc: IsProcessCacheSupported.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, IsProcessCacheSupported_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    int32_t pid = 1;
    bool isSupport = false;
    EXPECT_EQ(appMgrServiceInner->IsProcessCacheSupported(pid, isSupport), AAFwk::ERR_NO_APP_RECORD);

    appMgrServiceInner->appRunningManager_ = nullptr;
    EXPECT_EQ(appMgrServiceInner->IsProcessCacheSupported(pid, isSupport), AAFwk::ERR_NULL_APP_RUNNING_MANAGER);
}

/**
 * @tc.name: SetProcessCacheEnable_001
 * @tc.desc: SetProcessCacheEnable.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, SetProcessCacheEnable_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    int32_t pid = 1;
    bool enable = false;
    EXPECT_EQ(appMgrServiceInner->SetProcessCacheEnable(pid, enable), AAFwk::ERR_NO_APP_RECORD);

    appMgrServiceInner->appRunningManager_ = nullptr;
    EXPECT_EQ(appMgrServiceInner->SetProcessCacheEnable(pid, enable), AAFwk::ERR_NULL_APP_RUNNING_MANAGER);
}

/**
 * @tc.name: ReportEventToRSS_001
 * @tc.desc: test ReportEventToRSS
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, ReportEventToRSS_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ReportEventToRSS_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AbilityInfo abilityInfo;
    std::string temp = "";
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(info, 0, temp);
    appMgrServiceInner->taskHandler_ = nullptr;
    appMgrServiceInner->ReportEventToRSS(abilityInfo, appRecord);
    EXPECT_EQ(appRecord->GetPid(), 0);
    appMgrServiceInner->ReportEventToRSS(abilityInfo, nullptr);

    appRecord->priorityObject_->SetPid(FUN_TEST_PID);
    appMgrServiceInner->ReportEventToRSS(abilityInfo, nullptr);
    EXPECT_EQ(appRecord->GetPid(), FUN_TEST_PID);
    TAG_LOGI(AAFwkTag::TEST, "ReportEventToRSS_001 end");
}

/**
 * @tc.name: CreateAppRunningRecord_0001
 * @tc.desc: launch application.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, CreateAppRunningRecord_0001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateAppRunningRecord_0001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->LaunchApplication(nullptr);

    BundleInfo info;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, info, "");

    ASSERT_NE(appRecord, nullptr);
}

/**
 * @tc.name: GetSpecifiedProcessFlag_0001
 * @tc.desc: Verify that GetSpecifiedProcessFlag returns the correct flag when AbilityInfo is a UIExtension and
 * isolationProcess is true.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, GetSpecifiedProcessFlag_0001, TestSize.Level1)
{
    AbilityInfo abilityInfo;
    abilityInfo.type = AbilityType::PAGE;
    abilityInfo.isStageBasedModel = false;
    abilityInfo.isolationProcess = true;
    abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;

    AAFwk::Want want;
    std::string flag = "uiext_flag";
    want.SetParam("ohoSpecifiedProcessFlag", flag);
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    MockMyAppUtilsFlag::flag_ = true;
    std::string result = appMgrServiceInner->GetSpecifiedProcessFlag(abilityInfo, want);
    EXPECT_EQ(result, flag);
}

/**
 * @tc.name: GetSpecifiedProcessFlag_0002
 * @tc.desc: Verify that GetSpecifiedProcessFlag returns an empty string when AbilityInfo does not meet the conditions.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, GetSpecifiedProcessFlag_0002, TestSize.Level1)
{
    AbilityInfo abilityInfo;
    abilityInfo.type = AbilityType::PAGE;
    abilityInfo.isStageBasedModel = false;
    abilityInfo.isolationProcess = false;
    abilityInfo.extensionAbilityType = ExtensionAbilityType::UNSPECIFIED;

    AAFwk::Want want;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::string result = appMgrServiceInner->GetSpecifiedProcessFlag(abilityInfo, want);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name: GetSpecifiedProcessFlag_0003
 * @tc.desc: Verify that GetSpecifiedProcessFlag returns an empty string when the flag is set in the want but
 * AbilityInfo does not meet the conditions.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, GetSpecifiedProcessFlag_0003, TestSize.Level1)
{
    AbilityInfo abilityInfo;
    abilityInfo.type = AbilityType::PAGE;
    abilityInfo.isStageBasedModel = false;
    abilityInfo.isolationProcess = false;
    abilityInfo.extensionAbilityType = ExtensionAbilityType::UNSPECIFIED;
    AAFwk::Want want;
    std::string flag = "uiext_flag";
    want.SetParam("ohoSpecifiedProcessFlag", flag);

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::string result = appMgrServiceInner->GetSpecifiedProcessFlag(abilityInfo, want);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name: GetSpecifiedProcessFlag_0004
 * @tc.desc: Verify that GetSpecifiedProcessFlag returns an empty string when AbilityInfo is a UIExtension and
 * isolationProcess is false, even if the flag is set in want.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, GetSpecifiedProcessFlag_0004, TestSize.Level1)
{
    AbilityInfo abilityInfo;
    abilityInfo.type = AbilityType::PAGE;
    abilityInfo.isStageBasedModel = false;
    abilityInfo.isolationProcess = false;
    abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;

    AAFwk::Want want;
    std::string flag = "uiext_flag";
    want.SetParam("ohoSpecifiedProcessFlag", flag);
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::string result = appMgrServiceInner->GetSpecifiedProcessFlag(abilityInfo, want);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name: GetSpecifiedProcessFlag_0005
 * @tc.desc: Verify that GetSpecifiedProcessFlag returns the correct flag when AbilityInfo is a UIExtension and
 * isolationProcess is true.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, GetSpecifiedProcessFlag_0005, TestSize.Level1)
{
    AbilityInfo abilityInfo;
    abilityInfo.type = AbilityType::PAGE;
    abilityInfo.isStageBasedModel = false;
    abilityInfo.isolationProcess = true;
    abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;

    AAFwk::Want want;
    std::string flag = "uiext_flag";
    want.SetParam("ohoSpecifiedProcessFlag", flag);
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    MockMyAppUtilsFlag::flag_ = true;
    std::string result = appMgrServiceInner->GetSpecifiedProcessFlag(abilityInfo, want);
    EXPECT_EQ(result, flag);
}

/**
 * @tc.name: GetSpecifiedProcessFlag_0006
 * @tc.desc: Verify that GetSpecifiedProcessFlag returns the correct flag when AbilityInfo is a UIExtension and
 * isolationProcess is true.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, GetSpecifiedProcessFlag_0006, TestSize.Level1)
{
    AbilityInfo abilityInfo;
    abilityInfo.type = AbilityType::PAGE;
    abilityInfo.isStageBasedModel = false;
    abilityInfo.isolationProcess = true;
    abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;

    AAFwk::Want want;
    std::string flag = "uiext_flag";
    want.SetParam("ohoSpecifiedProcessFlag", flag);
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    MockMyAppUtilsFlag::flag_ = false;
    std::string result = appMgrServiceInner->GetSpecifiedProcessFlag(abilityInfo, want);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name: AfterLoadAbility_0001
 * @tc.desc: AfterLoadAbility
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, AfterLoadAbility_0001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AfterLoadAbility_0001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    BundleInfo info;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, info, "");
    AbilityInfo abilityInfo;
    abilityInfo.visible = true;
    abilityInfo.applicationName = "hiUiextension";
    abilityInfo.type = AbilityType::EXTENSION;
    abilityInfo.name = "UiextensionAbility";
    abilityInfo.bundleName = "com.ix.hiuiextension";
    abilityInfo.moduleName = "entry";
    abilityInfo.deviceId = "device";
    abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    std::shared_ptr<AbilityInfo> abilityInfoSptr = std::make_shared<AbilityInfo>(abilityInfo);
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    appMgrServiceInner->taskHandler_ = nullptr;
    appMgrServiceInner->AfterLoadAbility(appRecord, abilityInfoSptr, loadParam, false);
    ASSERT_NE(appRecord, nullptr);
}

/**
 * @tc.name: SetGameSAPrelaunch_001
 * @tc.desc: SetGameSAPrelaunch with null token.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, SetGameSAPrelaunch_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetGameSAPrelaunch_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    sptr<IRemoteObject> token = nullptr;
    bool isGameSAPrelaunch = true;
    auto result = appMgrServiceInner->SetGameSAPrelaunch(token, isGameSAPrelaunch);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "SetGameSAPrelaunch_001 end");
}

/**
 * @tc.name: SetGameSAPrelaunch_002
 * @tc.desc: SetGameSAPrelaunch with token not found in app records.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, SetGameSAPrelaunch_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetGameSAPrelaunch_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    EXPECT_NE(token, nullptr);
    bool isGameSAPrelaunch = true;
    auto result = appMgrServiceInner->SetGameSAPrelaunch(token, isGameSAPrelaunch);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "SetGameSAPrelaunch_002 end");
}

/**
 * @tc.name: SetGameSAPrelaunch_003
 * @tc.desc: SetGameSAPrelaunch with valid token and isGameSAPrelaunch true.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, SetGameSAPrelaunch_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetGameSAPrelaunch_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    // Create app running record with token
    std::string processName = "testProcess";
    BundleInfo info;
    info.name = "com.test.bundle";
    HapModuleInfo hapModuleInfo;
    std::shared_ptr<AAFwk::Want> want;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    loadParam->token = token;

    std::shared_ptr<AppRunningRecord> appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam,
        applicationInfo_, abilityInfo_, processName, info, hapModuleInfo, want);
    EXPECT_NE(appRecord, nullptr);

    // Register token by updating ability state
    appMgrServiceInner->UpdateAbilityState(token, AbilityState::ABILITY_STATE_CREATE);

    bool isGameSAPrelaunch = true;
    auto result = appMgrServiceInner->SetGameSAPrelaunch(token, isGameSAPrelaunch);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(appRecord->GetPreloadMode(), PreloadMode::GAME_PRELAUNCH);
    TAG_LOGI(AAFwkTag::TEST, "SetGameSAPrelaunch_003 end");
}

/**
 * @tc.name: SetGameSAPrelaunch_004
 * @tc.desc: SetGameSAPrelaunch with valid token and isGameSAPrelaunch false.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, SetGameSAPrelaunch_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetGameSAPrelaunch_004 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    // Create app running record with token
    // Create app running record with token
    std::string processName = "testProcess";
    BundleInfo info;
    info.name = "com.test.bundle";
    HapModuleInfo hapModuleInfo;
    std::shared_ptr<AAFwk::Want> want;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    loadParam->token = token;

    std::shared_ptr<AppRunningRecord> appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam,
        applicationInfo_, abilityInfo_, processName, info, hapModuleInfo, want);
    EXPECT_NE(appRecord, nullptr);

    // Register token by updating ability state
    appMgrServiceInner->UpdateAbilityState(token, AbilityState::ABILITY_STATE_CREATE);

    bool isGameSAPrelaunch = false;
    auto result = appMgrServiceInner->SetGameSAPrelaunch(token, isGameSAPrelaunch);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(appRecord->GetPreloadMode(), PreloadMode::PRELOAD_NONE);
    TAG_LOGI(AAFwkTag::TEST, "SetGameSAPrelaunch_004 end");
}

/**
 * @tc.name: UpdateConfigurationByUserIds_0001
 * @tc.desc: UpdateConfigurationByUserIds
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, UpdateConfigurationByUserIds_0001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateConfigurationByUserIds_0001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    MyFlag::flag_ = 0;
    Configuration changeConfig;
    std::vector<int32_t> userIds = {0, 1};
    auto res = appMgrServiceInner->UpdateConfigurationByUserIds(changeConfig, userIds);
    EXPECT_EQ(res, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "UpdateConfigurationByUserIds_0001 end");
}

/**
 * @tc.name: UpdateConfigurationByUserIds_0002
 * @tc.desc: UpdateConfigurationByUserIds
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, UpdateConfigurationByUserIds_0002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateConfigurationByUserIds_0002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    MyFlag::flag_ = 0;
    Configuration changeConfig;
    std::vector<int32_t> userIds;
    auto res = appMgrServiceInner->UpdateConfigurationByUserIds(changeConfig, userIds);
    EXPECT_EQ(res, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "UpdateConfigurationByUserIds_0002 end");
}

/**
 * @tc.name: GetAllAbilityInfos_0001
 * @tc.desc: GetAllAbilityInfos when appRunningManager is null
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, GetAllAbilityInfos_0001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAllAbilityInfos_0001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    appMgrServiceInner->appRunningManager_ = nullptr;

    std::vector<AppExecFwk::AbilityStateData> infos;
    auto ret = appMgrServiceInner->GetAllAbilityInfos(-1, infos);
    EXPECT_EQ(ret, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "GetAllAbilityInfos_0001 end");
}

/**
 * @tc.name: GetAllAbilityInfos_0002
 * @tc.desc: GetAllAbilityInfos with valid appRunningManager
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, GetAllAbilityInfos_0002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAllAbilityInfos_0002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();

    std::vector<AppExecFwk::AbilityStateData> infos;
    auto ret = appMgrServiceInner->GetAllAbilityInfos(-1, infos);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "GetAllAbilityInfos_0002 end");
}

/**
 * @tc.name: GetAllAbilityInfos_0003
 * @tc.desc: GetAllAbilityInfos with valid appRunningManager
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, GetAllAbilityInfos_0003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAllAbilityInfos_0003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();

    std::vector<AppExecFwk::AbilityStateData> infos;
    int32_t pid = 1001;
    auto ret = appMgrServiceInner->GetAllAbilityInfos(pid, infos);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "GetAllAbilityInfos_0003 end");
}
} // namespace AppExecFwk
} // namespace OHOS
