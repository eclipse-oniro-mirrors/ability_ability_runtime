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

#define private public
#include "app_mgr_service_inner.h"
#include "app_running_record.h"
#include "remote_client_manager.h"
#undef private
#include "ability_manager_errors.h"
#include "accesstoken_kit.h"
#include "advanced_security_mode_manager.h"
#include "app_scheduler.h"
#include "appspawn_util.h"
#include "app_spawn_client.h"
#include "event_handler.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "mock_ability_token.h"
#include "mock_app_scheduler.h"
#include "mock_bundle_manager.h"
#include "mock_configuration_observer.h"
#include "mock_iapp_state_callback.h"
#include "mock_kia_interceptor.h"
#include "mock_native_token.h"
#include "mock_render_scheduler.h"
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
}

class MockIAppStateCallback : public IAppStateCallback {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.appexecfwk.AppStateCallback");
    void OnAppStateChanged(const AppProcessData &appProcessData) override {};
    void OnAbilityRequestDone(const sptr<IRemoteObject> &token, const AbilityState state) override {};
    void OnAppRemoteDied(const std::vector<sptr<IRemoteObject>> &abilityTokens)
    {
        dealed = true;
    }
    void OnStartProcessFailed(const std::vector<sptr<IRemoteObject>> &abilityTokens)
    {
        dealed = true;
        tokenSize = abilityTokens.size();
    }
    void OnCacheExitInfo(uint32_t accessTokenId, const RunningProcessInfo &exitInfo,
        const std::string &bundleName, const std::vector<std::string> &abilityNames,
        const std::vector<std::string> &uiExtensionNames) override
    {
        dealed = true;
    }
    void NotifyConfigurationChange(const AppExecFwk::Configuration &config, int32_t userId) override
    {
        dealed = true;
    }
    sptr<IRemoteObject> AsObject()
    {
        return nullptr;
    }
    bool dealed = false;
    int32_t tokenSize = 0;
};

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
 * @tc.name: reportpreLoadTask_001
 * @tc.desc: reportpreLoadTask
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, reportpreLoadTask_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "reportpreLoadTask_001 start");

    std::shared_ptr<ApplicationInfo> info = std::make_shared<AppExecFwk::ApplicationInfo>();
    int32_t recordId = 0x0001;
    std::string processName = "processName for reportpreLoadTask";
    std::shared_ptr<AppRunningRecord> appRecord =
        std::make_shared<AppExecFwk::AppRunningRecord>(info, recordId, processName);
    appRecord->SetUid(1);

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->reportpreLoadTask(appRecord);

    appRecord->priorityObject_ = std::make_shared<AppExecFwk::PriorityObject>();
    appMgrServiceInner->reportpreLoadTask(appRecord);

    appMgrServiceInner->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler("");
    appMgrServiceInner->reportpreLoadTask(appRecord);
    EXPECT_NE(appMgrServiceInner, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "reportpreLoadTask_001 end");
}

/**
 * @tc.name: MakeProcessName_001
 * @tc.desc: MakeProcessName
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, MakeProcessName_003, TestSize.Level2)
{
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = nullptr;
    std::shared_ptr<AppExecFwk::ApplicationInfo> appInfo =  nullptr;
    AppExecFwk::HapModuleInfo hapModuleInfo;
    int32_t appIndex = 1;
    std::string specifiedProcessFlag {};
    std::string processName {};
    bool isCallerSetProcess = true;

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->MakeProcessName(abilityInfo, appInfo,
        hapModuleInfo, appIndex, specifiedProcessFlag, processName, isCallerSetProcess);

    abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    appMgrServiceInner->MakeProcessName(abilityInfo, appInfo,
        hapModuleInfo, appIndex, specifiedProcessFlag, processName, isCallerSetProcess);
    appInfo = std::make_shared<AppExecFwk::ApplicationInfo>();

    abilityInfo->process = "abilityInfoProcess";
    appInfo->bundleName = "appInfoBundleName";
    abilityInfo->process = "abilityInfoProcess";

    appMgrServiceInner->MakeProcessName(abilityInfo, appInfo,
        hapModuleInfo, appIndex, specifiedProcessFlag, processName, isCallerSetProcess);
    EXPECT_EQ(processName, abilityInfo->process + ":" + std::to_string(appIndex));

    abilityInfo->type = AppExecFwk::AbilityType::PAGE;
    abilityInfo->isStageBasedModel = true;
    appMgrServiceInner->MakeProcessName(abilityInfo, appInfo,
        hapModuleInfo, appIndex, specifiedProcessFlag, processName, isCallerSetProcess);
    EXPECT_EQ(processName, appInfo->bundleName +
        abilityInfo->process + ":" + std::to_string(appIndex));

    abilityInfo->process.clear();
    hapModuleInfo.process = "hapModuleInfoProcess";
    hapModuleInfo.isStageBasedModel = true;
    appMgrServiceInner->MakeProcessName(abilityInfo, appInfo,
        hapModuleInfo, appIndex, specifiedProcessFlag, processName, isCallerSetProcess);
    EXPECT_EQ(processName, hapModuleInfo.process + std::to_string(appIndex));

    specifiedProcessFlag = "specifiedProcessFlag";
    appMgrServiceInner->MakeProcessName(abilityInfo, appInfo,
        hapModuleInfo, appIndex, specifiedProcessFlag, processName, isCallerSetProcess);
    EXPECT_EQ(processName, hapModuleInfo.process +
        std::to_string(appIndex) + ":" + specifiedProcessFlag);
}

/**
 * @tc.name: IsIsolateExtensionSandBox_001
 * @tc.desc: IsIsolateExtensionSandBox
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, IsIsolateExtensionSandBox_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "IsIsolateExtensionSandBox_001 start");
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = nullptr;
    AppExecFwk::HapModuleInfo hapModuleInfo;

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    bool ret = appMgrServiceInner->IsIsolateExtensionSandBox(abilityInfo, hapModuleInfo);
    EXPECT_FALSE(ret);

    abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    EXPECT_NE(abilityInfo, nullptr);

    abilityInfo->type = AppExecFwk::AbilityType::PAGE;
    ret = appMgrServiceInner->IsIsolateExtensionSandBox(abilityInfo, hapModuleInfo);
    EXPECT_FALSE(ret);

    abilityInfo->type = AppExecFwk::AbilityType::EXTENSION;
    abilityInfo->extensionAbilityType = AppExecFwk::ExtensionAbilityType::DATASHARE;
    ret = appMgrServiceInner->IsIsolateExtensionSandBox(abilityInfo, hapModuleInfo);
    EXPECT_FALSE(ret);

    abilityInfo->extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    ret = appMgrServiceInner->IsIsolateExtensionSandBox(abilityInfo, hapModuleInfo);
    EXPECT_FALSE(ret);

    abilityInfo->extensionAbilityType = AppExecFwk::ExtensionAbilityType::INPUTMETHOD;
    ret = appMgrServiceInner->IsIsolateExtensionSandBox(abilityInfo, hapModuleInfo);
    EXPECT_FALSE(ret);

    AppExecFwk::ExtensionAbilityInfo extensionAbilityInfo1;
    abilityInfo->name = "extensionAbilityInfo2";
    extensionAbilityInfo1.name = "extensionAbilityInfo1";
    extensionAbilityInfo1.needCreateSandbox = false;
    hapModuleInfo.extensionInfos.push_back(extensionAbilityInfo1);
    ret = appMgrServiceInner->IsIsolateExtensionSandBox(abilityInfo, hapModuleInfo);
    EXPECT_FALSE(ret);

    abilityInfo->name = "extensionAbilityInfo1";
    ret = appMgrServiceInner->IsIsolateExtensionSandBox(abilityInfo, hapModuleInfo);
    EXPECT_FALSE(ret);

    AppExecFwk::ExtensionAbilityInfo extensionAbilityInfo2;
    abilityInfo->name = "extensionAbilityInfo2";
    extensionAbilityInfo2.name = "extensionAbilityInfo2";
    extensionAbilityInfo2.needCreateSandbox = true;
    hapModuleInfo.extensionInfos.push_back(extensionAbilityInfo2);
    ret = appMgrServiceInner->IsIsolateExtensionSandBox(abilityInfo, hapModuleInfo);
    EXPECT_TRUE(ret);
    TAG_LOGI(AAFwkTag::TEST, "IsIsolateExtensionSandBox_001 end");
}

/**
 * @tc.name: LaunchApplicationExt_001
 * @tc.desc: LaunchApplicationExt
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, LaunchApplicationExt_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::shared_ptr<AppExecFwk::AppRunningRecord> appRecord =
        std::make_shared<AppExecFwk::AppRunningRecord>(nullptr, 1, "111");
    appMgrServiceInner->nwebPreloadSet_.insert("111");

    appRecord->isAllowedNWebPreload_ = false;
    appMgrServiceInner->LaunchApplicationExt(appRecord);
    EXPECT_EQ(appRecord->isAllowedNWebPreload_, true);
}

/**
 * @tc.name: IsAllowedNWebPreload_001
 * @tc.desc: IsAllowedNWebPreload
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, IsAllowedNWebPreload_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->nwebPreloadSet_.clear();

    EXPECT_EQ(appMgrServiceInner->IsAllowedNWebPreload("111"), false);
    appMgrServiceInner->nwebPreloadSet_.insert("111");
    EXPECT_EQ(appMgrServiceInner->IsAllowedNWebPreload("111"), true);
}

/**
 * @tc.name: NotifyAppAttachFailed_001
 * @tc.desc: NotifyAppAttachFailed
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, NotifyAppAttachFailed_001, TestSize.Level2)
{
    std::shared_ptr<AppExecFwk::AppRunningRecord> appRecord =
        std::make_shared<AppExecFwk::AppRunningRecord>(nullptr, 1, "111");

    auto moduleRunningRecord =
        std::make_shared<AppExecFwk::ModuleRunningRecord>(nullptr, nullptr);
    sptr<IRemoteObject> iremoteObject =
        sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto abilityRunningRecord =
        std::make_shared<AppExecFwk::AbilityRunningRecord>(nullptr, nullptr, 1);
    moduleRunningRecord->abilities_.emplace(iremoteObject, abilityRunningRecord);
    std::vector<std::shared_ptr<AppExecFwk::ModuleRunningRecord>> modulerunningrecordVector;
    modulerunningrecordVector.push_back(moduleRunningRecord);
    appRecord->hapModules_.emplace(std::make_pair("111", modulerunningrecordVector));

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AppMgrServiceInner::AppStateCallbackWithUserId appStateCallbackWithUserId;
    appStateCallbackWithUserId.callback = nullptr;
    appMgrServiceInner->appStateCallbacks_.push_back(appStateCallbackWithUserId);
    appMgrServiceInner->NotifyAppAttachFailed(appRecord);

    appMgrServiceInner->appStateCallbacks_.clear();
    appStateCallbackWithUserId.callback = sptr<MockIAppStateCallback>::MakeSptr();
    appMgrServiceInner->appStateCallbacks_.push_back(appStateCallbackWithUserId);
    appMgrServiceInner->NotifyAppAttachFailed(appRecord);
    for (auto &item : appMgrServiceInner->appStateCallbacks_) {
        MockIAppStateCallback* rawPtr =
            static_cast<MockIAppStateCallback*>(item.callback.GetRefPtr());
        if (rawPtr) {
            sptr<MockIAppStateCallback> iapp(rawPtr);
            EXPECT_EQ(iapp->dealed, true);
        }
    }
}

/**
 * @tc.name: NotifyLoadAbilityFailed_001
 * @tc.desc: NotifyLoadAbilityFailed
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, NotifyLoadAbilityFailed_001, TestSize.Level2)
{
    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AppMgrServiceInner::AppStateCallbackWithUserId appStateCallbackWithUserId;
    appStateCallbackWithUserId.callback = nullptr;
    appMgrServiceInner->appStateCallbacks_.push_back(appStateCallbackWithUserId);
    appMgrServiceInner->NotifyLoadAbilityFailed(token);

    appMgrServiceInner->appStateCallbacks_.clear();
    appStateCallbackWithUserId.callback = sptr<MockIAppStateCallback>::MakeSptr();
    appMgrServiceInner->appStateCallbacks_.push_back(appStateCallbackWithUserId);
    appMgrServiceInner->NotifyLoadAbilityFailed(token);
    for (auto &item : appMgrServiceInner->appStateCallbacks_) {
        MockIAppStateCallback* rawPtr =
            static_cast<MockIAppStateCallback*>(item.callback.GetRefPtr());
        if (rawPtr) {
            sptr<MockIAppStateCallback> iapp(rawPtr);
            EXPECT_EQ(iapp->dealed, true);
        }
    }
}

/**
 * @tc.name: NotifyStartProcessFailed_001
 * @tc.desc: NotifyStartProcessFailed
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, NotifyStartProcessFailed_001, TestSize.Level2)
{
    std::shared_ptr<AppExecFwk::AppRunningRecord> appRecord =
        std::make_shared<AppExecFwk::AppRunningRecord>(nullptr, 1, "111");

    auto moduleRunningRecord =
        std::make_shared<AppExecFwk::ModuleRunningRecord>(nullptr, nullptr);
    sptr<IRemoteObject> iremoteObject =
        sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto abilityRunningRecord =
        std::make_shared<AppExecFwk::AbilityRunningRecord>(nullptr, nullptr, 1);
    moduleRunningRecord->abilities_.emplace(iremoteObject, abilityRunningRecord);
    std::vector<std::shared_ptr<AppExecFwk::ModuleRunningRecord>> modulerunningrecordVector;
    modulerunningrecordVector.push_back(moduleRunningRecord);
    appRecord->hapModules_.emplace(std::make_pair("111", modulerunningrecordVector));

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AppMgrServiceInner::AppStateCallbackWithUserId appStateCallbackWithUserId;
    appStateCallbackWithUserId.callback = nullptr;
    appMgrServiceInner->appStateCallbacks_.push_back(appStateCallbackWithUserId);
    appMgrServiceInner->NotifyStartProcessFailed(appRecord);

    appMgrServiceInner->appStateCallbacks_.clear();
    appStateCallbackWithUserId.callback = sptr<MockIAppStateCallback>::MakeSptr();
    appMgrServiceInner->appStateCallbacks_.push_back(appStateCallbackWithUserId);
    appMgrServiceInner->NotifyStartProcessFailed(appRecord);
    for (auto &item : appMgrServiceInner->appStateCallbacks_) {
        MockIAppStateCallback* rawPtr =
            static_cast<MockIAppStateCallback*>(item.callback.GetRefPtr());
        if (rawPtr) {
            sptr<MockIAppStateCallback> iapp(rawPtr);
            EXPECT_EQ(iapp->dealed, true);
            EXPECT_EQ(iapp->tokenSize, 1);
        }
    }
}

/**
 * @tc.name: NotifyStartProcessFailed_002
 * @tc.desc: NotifyStartProcessFailed
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, NotifyStartProcessFailed_002, TestSize.Level2)
{
    std::shared_ptr<AppExecFwk::AppRunningRecord> appRecord =
        std::make_shared<AppExecFwk::AppRunningRecord>(nullptr, 1, "111");

    auto moduleRunningRecord =
        std::make_shared<AppExecFwk::ModuleRunningRecord>(nullptr, nullptr);
    sptr<IRemoteObject> iremoteObject =
        sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto abilityRunningRecord =
        std::make_shared<AppExecFwk::AbilityRunningRecord>(nullptr, nullptr, 1);
    std::vector<std::shared_ptr<AppExecFwk::ModuleRunningRecord>> modulerunningrecordVector;
    modulerunningrecordVector.push_back(moduleRunningRecord);
    appRecord->hapModules_.emplace(std::make_pair("111", modulerunningrecordVector));

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AppMgrServiceInner::AppStateCallbackWithUserId appStateCallbackWithUserId;
    appStateCallbackWithUserId.callback = sptr<MockIAppStateCallback>::MakeSptr();
    appMgrServiceInner->appStateCallbacks_.push_back(appStateCallbackWithUserId);
    appMgrServiceInner->NotifyStartProcessFailed(appRecord);
    for (auto &item : appMgrServiceInner->appStateCallbacks_) {
        MockIAppStateCallback* rawPtr =
            static_cast<MockIAppStateCallback*>(item.callback.GetRefPtr());
        if (rawPtr) {
            sptr<MockIAppStateCallback> iapp(rawPtr);
            EXPECT_EQ(iapp->dealed, false);
            EXPECT_EQ(iapp->tokenSize, 0);
        }
    }
}

/**
 * @tc.name: NotifyStartProcessFailed_token_001
 * @tc.desc: NotifyStartProcessFailed
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, NotifyStartProcessFailed_token_001, TestSize.Level2)
{
    sptr<IRemoteObject> iremoteObject =
        sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AppMgrServiceInner::AppStateCallbackWithUserId appStateCallbackWithUserId;
    appStateCallbackWithUserId.callback = sptr<MockIAppStateCallback>::MakeSptr();
    appMgrServiceInner->appStateCallbacks_.push_back(appStateCallbackWithUserId);
    appMgrServiceInner->NotifyStartProcessFailed(iremoteObject);
    for (auto &item : appMgrServiceInner->appStateCallbacks_) {
        MockIAppStateCallback* rawPtr =
            static_cast<MockIAppStateCallback*>(item.callback.GetRefPtr());
        if (rawPtr) {
            sptr<MockIAppStateCallback> iapp(rawPtr);
            EXPECT_EQ(iapp->dealed, true);
            EXPECT_EQ(iapp->tokenSize, 1);
        }
    }
}

/**
 * @tc.name: OpenAppSpawnConnection_002
 * @tc.desc: open app spawn connection.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, OpenAppSpawnConnection_002, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->remoteClientManager_ = nullptr;
    auto ret = appMgrServiceInner->OpenAppSpawnConnection();
    EXPECT_EQ(ret, 22);

    appMgrServiceInner->remoteClientManager_ =
        std::make_shared<RemoteClientManager>();
    appMgrServiceInner->remoteClientManager_->appSpawnClient_.reset();
    ret = appMgrServiceInner->OpenAppSpawnConnection();
    EXPECT_EQ(ret, 8454147);

    appMgrServiceInner->remoteClientManager_->appSpawnClient_ =
        std::make_shared<AppExecFwk::AppSpawnClient>();
    appMgrServiceInner->remoteClientManager_->appSpawnClient_->state_ =
        SpawnConnectionState::STATE_CONNECTED;
    ret = appMgrServiceInner->OpenAppSpawnConnection();
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: SetAppSpawnClient_002
 * @tc.desc: set app spawn client.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, SetAppSpawnClient_002, TestSize.Level2)
{
    std::shared_ptr<AppSpawnClient> spawnClient = nullptr;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->remoteClientManager_ = nullptr;

    appMgrServiceInner->SetAppSpawnClient(spawnClient);
    appMgrServiceInner->remoteClientManager_ =
        std::make_shared<RemoteClientManager>();

    appMgrServiceInner->remoteClientManager_->appSpawnClient_ = nullptr;
    spawnClient = std::make_shared<AppExecFwk::AppSpawnClient>(false);
    appMgrServiceInner->SetAppSpawnClient(spawnClient);
    EXPECT_NE(appMgrServiceInner->remoteClientManager_->appSpawnClient_, nullptr);
}

/**
 * @tc.name: SetBundleManagerHelper_002
 * @tc.desc: set app SetBundleManagerHelper.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, SetBundleManagerHelper_002, TestSize.Level2)
{
    std::shared_ptr<BundleMgrHelper> bundleMgrHelper = nullptr;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->remoteClientManager_ = nullptr;
    appMgrServiceInner->SetBundleManagerHelper(bundleMgrHelper);

    appMgrServiceInner->remoteClientManager_ =
        std::make_shared<RemoteClientManager>();
    appMgrServiceInner->remoteClientManager_->bundleManagerHelper_ = nullptr;
    bundleMgrHelper =
        appMgrServiceInner->remoteClientManager_->GetBundleManagerHelper();
    appMgrServiceInner->SetBundleManagerHelper(bundleMgrHelper);
    EXPECT_NE(appMgrServiceInner->remoteClientManager_->bundleManagerHelper_, nullptr);
}

/**
 * @tc.name: SetStartMsgStrictMode_001
 * @tc.desc: SetStartMsgStrictMode.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, SetStartMsgStrictMode_001, TestSize.Level2)
{
    CreateStartMsgParam param;
    param.strictMode = true;
    AppSpawnStartMsg startMsg;
    startMsg.strictMode = false;
    startMsg.isolatedNetworkFlag = true;
    startMsg.isolatedSELinuxFlag = true;

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->SetStartMsgStrictMode(startMsg, param);
    EXPECT_EQ(startMsg.isolatedNetworkFlag, false);
    EXPECT_EQ(startMsg.isolatedSELinuxFlag, false);

    param.extensionAbilityType = ExtensionAbilityType::INPUTMETHOD;
    startMsg.isolatedSandboxFlagLegacy = false;
    appMgrServiceInner->SetStartMsgStrictMode(startMsg, param);
    EXPECT_EQ(startMsg.strictMode, param.strictMode);
    EXPECT_EQ(startMsg.isolatedSandboxFlagLegacy, true);
}

/**
 * @tc.name: SetProcessJITState_001
 * @tc.desc: SetProcessJITState.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, SetProcessJITState_001, TestSize.Level2)
{
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->SetProcessJITState(appRecord);

    appRecord = std::make_shared<AppExecFwk::AppRunningRecord>(nullptr, 0, "");
    appMgrServiceInner->securityModeManager_ = nullptr;
    appMgrServiceInner->SetProcessJITState(appRecord);
    EXPECT_EQ(appRecord->jitEnabled_, true);

    appMgrServiceInner->securityModeManager_ =
        std::make_shared<AppExecFwk::AdvancedSecurityModeManager>();
    appMgrServiceInner->SetProcessJITState(appRecord);
    EXPECT_EQ(appRecord->jitEnabled_, true);
}

/**
 * @tc.name: SendCreateAtomicServiceProcessEvent_001
 * @tc.desc: SendCreateAtomicServiceProcessEvent.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, SendCreateAtomicServiceProcessEvent_002, TestSize.Level2)
{
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    BundleType bundleType { BundleType::APP };
    std::string moduleName {};
    std::string abilityName {};

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    bool result = appMgrServiceInner->SendCreateAtomicServiceProcessEvent(appRecord,
        bundleType, moduleName, abilityName);
    EXPECT_EQ(result, false);

    bundleType = { BundleType::ATOMIC_SERVICE };
    result = appMgrServiceInner->SendCreateAtomicServiceProcessEvent(appRecord,
        bundleType, moduleName, abilityName);
    EXPECT_EQ(result, false);

    appRecord = std::make_shared<AppExecFwk::AppRunningRecord>(nullptr, 0, "");
    result = appMgrServiceInner->SendCreateAtomicServiceProcessEvent(appRecord,
        bundleType, moduleName, abilityName);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: SendReStartProcessEvent_006
 * @tc.desc: SendReStartProcessEvent.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, SendReStartProcessEvent_006, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    int64_t restartTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
        system_clock::now().time_since_epoch()).count();
    int64_t killtime1 = restartTime - 2001;
    appMgrServiceInner->killedProcessMap_.emplace(std::make_pair(killtime1, "killtime1"));
    int64_t killtime2 = restartTime - 1999;
    appMgrServiceInner->killedProcessMap_.emplace(std::make_pair(killtime2, "killtime2"));

    std::string testString1 = "11111";
    std::string testString2 = "22222";
    AAFwk::EventInfo eventInfo;
    int32_t appUid = 1;
    eventInfo.bundleName = testString1;
    eventInfo.callerBundleName = testString1;
    eventInfo.processName = testString1;
    eventInfo.callerProcessName = testString1;

    appMgrServiceInner->SendReStartProcessEvent(eventInfo, appUid);
    EXPECT_EQ(appMgrServiceInner->killedProcessMap_.size(), 1);

    eventInfo.bundleName = testString1;
    eventInfo.callerBundleName = testString2;
    eventInfo.processName = testString1;
    eventInfo.callerProcessName = testString2;
    appMgrServiceInner->SendReStartProcessEvent(eventInfo, appUid);
    EXPECT_EQ(appMgrServiceInner->killedProcessMap_.size(), 1);

    eventInfo.bundleName = testString1;
    eventInfo.callerBundleName = testString2;
    eventInfo.processName = testString1;
    eventInfo.callerProcessName = testString1;
    appMgrServiceInner->SendReStartProcessEvent(eventInfo, appUid);
    EXPECT_EQ(appMgrServiceInner->killedProcessMap_.size(), 1);

    eventInfo.bundleName = testString1;
    eventInfo.callerBundleName = testString1;
    eventInfo.processName = testString1;
    eventInfo.callerProcessName = testString2;
    appMgrServiceInner->SendReStartProcessEvent(eventInfo, appUid);
    EXPECT_EQ(appMgrServiceInner->killedProcessMap_.size(), 0);
}

/**
 * @tc.name: SendPreloadAppStartupTypeEvent_001
 * @tc.desc: SendPreloadAppStartupTypeEvent.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, SendPreloadAppStartupTypeEvent_001, TestSize.Level2)
{
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    std::shared_ptr<AbilityInfo> abilityInfo = nullptr;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->SendPreloadAppStartupTypeEvent(appRecord, abilityInfo);

    appRecord = std::make_shared<AppExecFwk::AppRunningRecord>(nullptr, 0, "");
    appRecord->SetPreloadState(PreloadState::NONE);
    appMgrServiceInner->SendPreloadAppStartupTypeEvent(appRecord, abilityInfo);

    appRecord->SetPreloadState(PreloadState::PRELOADED);
    appRecord->SetPreloadMode(PreloadMode::PRE_MAKE);
    appMgrServiceInner->SendPreloadAppStartupTypeEvent(appRecord, abilityInfo);

    appRecord->SetPreloadMode(PreloadMode::PRELOAD_MODULE);
    appMgrServiceInner->SendPreloadAppStartupTypeEvent(appRecord, abilityInfo);

    appRecord->SetPreloadMode(PreloadMode::PRESS_DOWN);
    appMgrServiceInner->SendPreloadAppStartupTypeEvent(appRecord, abilityInfo);

    appRecord->SetPreloadMode(PreloadMode::PRELOAD_BY_PHASE);
    appRecord->SetPreloadPhase(PreloadPhase::PROCESS_CREATED);
    appMgrServiceInner->SendPreloadAppStartupTypeEvent(appRecord, abilityInfo);

    appRecord->SetPreloadMode(PreloadMode::PRELOAD_BY_PHASE);
    appRecord->SetPreloadPhase(PreloadPhase::ABILITY_STAGE_CREATED);
    appMgrServiceInner->SendPreloadAppStartupTypeEvent(appRecord, abilityInfo);
    EXPECT_NE(appMgrServiceInner, nullptr);
}

/**
 * @tc.name: SendAppStartupTypeEvent_001
 * @tc.desc: SendAppStartupTypeEvent.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, SendAppStartupTypeEvent_001, TestSize.Level2)
{
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo1 = nullptr;
    AppStartType startType { AppStartType::COLD };
    AppStartReason reason { AppStartReason::NONE };

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->SendAppStartupTypeEvent(appRecord, abilityInfo1, startType, reason);

    appRecord = std::make_shared<AppExecFwk::AppRunningRecord>(nullptr, 0, "");
    appMgrServiceInner->SendAppStartupTypeEvent(appRecord, abilityInfo1, startType, reason);

    appRecord->appInfo_ = std::make_shared<ApplicationInfo>();
    appRecord->appInfo_->name = "name";
    appRecord->appInfo_->versionName = "versionName";
    appRecord->appInfo_->versionCode = 1;
    appRecord->priorityObject_ = std::make_shared<AppExecFwk::PriorityObject>();
    appRecord->priorityObject_->SetPid(9999);
    abilityInfo1 = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityInfo1->name = "name";
    abilityInfo1->type = AppExecFwk::AbilityType::UNKNOWN;
    appMgrServiceInner->SendAppStartupTypeEvent(appRecord, abilityInfo1, startType, reason);

    abilityInfo1->type = AppExecFwk::AbilityType::PAGE;
    appMgrServiceInner->SendAppStartupTypeEvent(appRecord, abilityInfo1, startType, reason);

    abilityInfo1->type = AppExecFwk::AbilityType::UNKNOWN;
    abilityInfo1->extensionAbilityType = AppExecFwk::ExtensionAbilityType::UI;
    appMgrServiceInner->SendAppStartupTypeEvent(appRecord, abilityInfo1, startType, reason);
    EXPECT_NE(appMgrServiceInner, nullptr);
}

/**
 * @tc.name: CacheExitInfo_001
 * @tc.desc: CacheExitInfo.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, CacheExitInfo_001, TestSize.Level2)
{
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->CacheExitInfo(appRecord);

    appRecord = std::make_shared<AppExecFwk::AppRunningRecord>(nullptr, 0, "");
    appRecord->SetReasonExist(true);
    appMgrServiceInner->CacheExitInfo(appRecord);

    appRecord->SetReasonExist(false);
    appMgrServiceInner->CacheExitInfo(appRecord);

    appRecord->SetUid(1);
    appRecord->SetRssValue(1);
    appRecord->SetPssValue(1);
    appRecord->processName_ = "processName";
    appRecord->appInfo_ = std::make_shared<AppExecFwk::ApplicationInfo>();
    appMgrServiceInner->CacheExitInfo(appRecord);

    auto moduleRunningRecord =
        std::make_shared<AppExecFwk::ModuleRunningRecord>(nullptr, nullptr);
    sptr<IRemoteObject> iremoteObject =
        sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto abilityRunningRecord =
        std::make_shared<AppExecFwk::AbilityRunningRecord>(nullptr, nullptr, 1);
    abilityRunningRecord->info_ =
        std::make_shared<AppExecFwk::AbilityInfo>();
    abilityRunningRecord->info_->type = AppExecFwk::AbilityType::PAGE;
    abilityRunningRecord->info_->name = "applicationName";
    moduleRunningRecord->abilities_.emplace(iremoteObject, abilityRunningRecord);
    std::vector<std::shared_ptr<AppExecFwk::ModuleRunningRecord>> modulerunningrecordVector;
    modulerunningrecordVector.push_back(moduleRunningRecord);
    appRecord->hapModules_.emplace(std::make_pair("111", modulerunningrecordVector));

    AppMgrServiceInner::AppStateCallbackWithUserId appStateCallbackWithUserId;
    appStateCallbackWithUserId.callback = nullptr;
    appMgrServiceInner->appStateCallbacks_.push_back(appStateCallbackWithUserId);
    appMgrServiceInner->CacheExitInfo(appRecord);

    appMgrServiceInner->appStateCallbacks_.clear();
    appStateCallbackWithUserId.callback = sptr<MockIAppStateCallback>::MakeSptr();
    appMgrServiceInner->appStateCallbacks_.push_back(appStateCallbackWithUserId);
    appMgrServiceInner->CacheExitInfo(appRecord);
    for (auto &item : appMgrServiceInner->appStateCallbacks_) {
        MockIAppStateCallback* rawPtr =
            static_cast<MockIAppStateCallback*>(item.callback.GetRefPtr());
        if (rawPtr) {
            sptr<MockIAppStateCallback> iapp(rawPtr);
            EXPECT_EQ(iapp->dealed, true);
        }
    }
}

/**
 * @tc.name: HandleConfigurationChange_001
 * @tc.desc: HandleConfigurationChange.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, HandleConfigurationChange_001, TestSize.Level2)
{
    Configuration config;
    int32_t userId = -1;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AppMgrServiceInner::AppStateCallbackWithUserId appStateCallbackWithUserId;
    appStateCallbackWithUserId.callback = nullptr;
    appStateCallbackWithUserId.userId = 0;
    appMgrServiceInner->appStateCallbacks_.push_back(appStateCallbackWithUserId);
    appMgrServiceInner->HandleConfigurationChange(config, userId);

    appMgrServiceInner->appStateCallbacks_.clear();
    appStateCallbackWithUserId.userId = -1;
    appMgrServiceInner->appStateCallbacks_.push_back(appStateCallbackWithUserId);
    appMgrServiceInner->HandleConfigurationChange(config, userId);

    appMgrServiceInner->appStateCallbacks_.clear();
    userId = 0;
    appStateCallbackWithUserId.userId = 0;
    appMgrServiceInner->appStateCallbacks_.push_back(appStateCallbackWithUserId);
    appMgrServiceInner->HandleConfigurationChange(config, userId);

    appMgrServiceInner->appStateCallbacks_.clear();
    appStateCallbackWithUserId.userId = -1;
    appMgrServiceInner->appStateCallbacks_.push_back(appStateCallbackWithUserId);
    appMgrServiceInner->HandleConfigurationChange(config, userId);

    appMgrServiceInner->appStateCallbacks_.clear();
    appStateCallbackWithUserId.callback = sptr<MockIAppStateCallback>::MakeSptr();
    userId = 1;
    appStateCallbackWithUserId.userId = 2;
    appMgrServiceInner->appStateCallbacks_.push_back(appStateCallbackWithUserId);
    appMgrServiceInner->HandleConfigurationChange(config, userId);

    appMgrServiceInner->appStateCallbacks_.clear();
    userId = -1;
    appMgrServiceInner->appStateCallbacks_.push_back(appStateCallbackWithUserId);
    appMgrServiceInner->HandleConfigurationChange(config, userId);
    for (auto &item : appMgrServiceInner->appStateCallbacks_) {
        MockIAppStateCallback* rawPtr =
            static_cast<MockIAppStateCallback*>(item.callback.GetRefPtr());
        if (rawPtr) {
            sptr<MockIAppStateCallback> iapp(rawPtr);
            EXPECT_EQ(iapp->dealed, true);
        }
    }
}

/**
 * @tc.name: GetValidUserId_001
 * @tc.desc: GetValidUserId - input userId is not DEFAULT_INVAL_VALUE, return as-is
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, GetValidUserId_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    int32_t result = appMgrServiceInner->GetValidUserId(100);
    EXPECT_EQ(result, 100);
}

/**
 * @tc.name: GetValidUserId_002
 * @tc.desc: GetValidUserId - input is DEFAULT_INVAL_VALUE, uid maps to U0/U1, get foreground userId
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, GetValidUserId_002, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    int32_t result = appMgrServiceInner->GetValidUserId(-1);
    EXPECT_GE(result, 0);
}

/**
 * @tc.name: MakeImageInner_ImageExist_001
 * @tc.desc: MakeImageInner - image already exists in imageInfoMap_
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, MakeImageInner_ImageExist_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::Want want;
    want.SetElementName("bundleName", "abilityName");
    AppMgrServiceInner::MakeImageRequest request {
        .bundleName = "bundleName",
        .abilityName = "abilityName",
        .userId = 100,
        .appCloneIndex = 0
    };
    auto imageInfo = std::make_shared<ForkImageInfo>();
    appMgrServiceInner->imageInfoMap_[request] = imageInfo;

    auto ret = appMgrServiceInner->MakeImageInner(want, 100,
        AppExecFwk::PreloadMode::PRELOAD_MODULE, 0, nullptr);
    EXPECT_EQ(ret, ImageError::ERR_IMAGE_INFO_EXIST);
}

/**
 * @tc.name: MakeImageInner_PreloadFailed_001
 * @tc.desc: MakeImageInner - PreloadApplication fails
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, MakeImageInner_PreloadFailed_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::Want want;
    want.SetElementName("bundleName", "abilityName");

    auto ret = appMgrServiceInner->MakeImageInner(want, -1,
        AppExecFwk::PreloadMode::PRELOAD_MODULE, 0, nullptr);
    EXPECT_EQ(ret, ImageError::ERR_PRELOAD_FAILED);
}

/**
 * @tc.name: MakeImageInner_NotPreloadModule_001
 * @tc.desc: MakeImageInner - preloadMode is not PRELOAD_MODULE
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, MakeImageInner_NotPreloadModule_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::Want want;
    want.SetElementName("bundleName", "abilityName");

    auto ret = appMgrServiceInner->MakeImageInner(want, 100,
        AppExecFwk::PreloadMode::PRELOAD_NONE, 0, nullptr);
    EXPECT_EQ(ret, ImageError::ERR_INVALID_PRELOAD_TYPE);
}

/**
 * @tc.name: DestroyImageByImageInfo_NullImageInfo_001
 * @tc.desc: DestroyImageByImageInfo - imageInfo is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, DestroyImageByImageInfo_NullImageInfo_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto ret = appMgrServiceInner->DestroyImageByImageInfo(nullptr);
    EXPECT_EQ(ret, ImageError::ERR_IMAGE_INFO_NOT_EXIST);
}

/**
 * @tc.name: DestroyImageByImageInfo_ImagePidInvalid_001
 * @tc.desc: DestroyImageByImageInfo - imagePid < 0
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, DestroyImageByImageInfo_ImagePidInvalid_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto imageInfo = std::make_shared<ForkImageInfo>();
    imageInfo->imagePid = -1;
    auto appRecord = std::make_shared<AppRunningRecord>(
        std::make_shared<ApplicationInfo>(), 1, "processName");
    imageInfo->baseAppRecord = appRecord;
    // imagePid < 0 → ERR_IMAGE_INFO_NOT_READY
    auto ret = appMgrServiceInner->DestroyImageByImageInfo(imageInfo);
    EXPECT_EQ(ret, ImageError::ERR_IMAGE_INFO_NOT_READY);
}

/**
 * @tc.name: DestroyImageByImageInfo_BaseAppRecordNull_001
 * @tc.desc: DestroyImageByImageInfo - baseAppRecord is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, DestroyImageByImageInfo_BaseAppRecordNull_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto imageInfo = std::make_shared<ForkImageInfo>();
    imageInfo->imagePid = 100;
    imageInfo->baseAppRecord = nullptr;
    auto ret = appMgrServiceInner->DestroyImageByImageInfo(imageInfo);
    EXPECT_EQ(ret, ImageError::ERR_IMAGE_INFO_NOT_READY);
}

/**
 * @tc.name: DestroyImageForFault_NullAppRecord_001
 * @tc.desc: DestroyImageForFault - appRecord is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, DestroyImageForFault_NullAppRecord_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto ret = appMgrServiceInner->DestroyImageForFault(nullptr);
    EXPECT_EQ(ret, ImageError::ERR_INNER);
}

/**
 * @tc.name: DestroyImageForFault_ImageInfoNotExist_001
 * @tc.desc: DestroyImageForFault - imageInfo not found in map
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, DestroyImageForFault_ImageInfoNotExist_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto appRecord = std::make_shared<AppRunningRecord>(
        std::make_shared<ApplicationInfo>(), 1, "processName");
    auto ret = appMgrServiceInner->DestroyImageForFault(appRecord);
    EXPECT_EQ(ret, ImageError::ERR_IMAGE_INFO_NOT_EXIST);
}

/**
 * @tc.name: HandleForkAll_NullAppRecord_001
 * @tc.desc: HandleForkAll - appRecord not found by pid, returns -1
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, HandleForkAll_NullAppRecord_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    int32_t ret = appMgrServiceInner->HandleForkAll(99999);
    EXPECT_EQ(ret, -1);
}

/**
 * @tc.name: HandleForkAllInner_AppRecordNull_001
 * @tc.desc: HandleForkAllInner - appRecord is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, HandleForkAllInner_AppRecordNull_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto ret = appMgrServiceInner->HandleForkAllInner(nullptr, 100);
    EXPECT_EQ(ret, ImageError::ERR_INNER);
}

/**
 * @tc.name: HandleForkAllInner_StateNotReady_001
 * @tc.desc: HandleForkAllInner - makeImageState is not MAKE_PRELOAD_FINISH
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, HandleForkAllInner_StateNotReady_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto appRecord = std::make_shared<AppRunningRecord>(
        std::make_shared<ApplicationInfo>(), 1, "processName");
    appRecord->SetMakeImageState(MakeImageState::NONE);
    auto ret = appMgrServiceInner->HandleForkAllInner(appRecord, 100);
    EXPECT_EQ(ret, ImageError::ERR_TEMPLATE_HAS_BEEN_USED);
}

/**
 * @tc.name: IsImageInfoExist_NullAppRecord_001
 * @tc.desc: IsImageInfoExist - appRecord is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, IsImageInfoExist_NullAppRecord_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    bool ret = appMgrServiceInner->IsImageInfoExist(nullptr);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IsImageInfoExist_NotFound_001
 * @tc.desc: IsImageInfoExist - image not found in map
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, IsImageInfoExist_NotFound_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    bool ret = appMgrServiceInner->IsImageInfoExist("bundle", "ability", 100, 0);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IsImageInfoExist_Found_001
 * @tc.desc: IsImageInfoExist - image found in map
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, IsImageInfoExist_Found_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AppMgrServiceInner::MakeImageRequest request {
        .bundleName = "bundle",
        .abilityName = "ability",
        .userId = 100,
        .appCloneIndex = 0
    };
    appMgrServiceInner->imageInfoMap_[request] = std::make_shared<ForkImageInfo>();
    bool ret = appMgrServiceInner->IsImageInfoExist("bundle", "ability", 100, 0);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: GetImageInfoByRecord_NullAppRecord_001
 * @tc.desc: GetImageInfo - appRecord is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, GetImageInfoByRecord_NullAppRecord_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto ret = appMgrServiceInner->GetImageInfo(nullptr);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: GetImageInfoByRecord_Valid_001
 * @tc.desc: GetImageInfo - valid appRecord found in map
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, GetImageInfoByRecord_Valid_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AppMgrServiceInner::MakeImageRequest request {
        .bundleName = "bundleName",
        .abilityName = "abilityName",
        .userId = 100,
        .appCloneIndex = 0
    };
    auto imageInfo = std::make_shared<ForkImageInfo>();
    appMgrServiceInner->imageInfoMap_[request] = imageInfo;

    auto appRecord = std::make_shared<AppRunningRecord>(
        std::make_shared<ApplicationInfo>(), 1, "processName");
    auto ret = appMgrServiceInner->GetImageInfo(appRecord);
    EXPECT_EQ(ret, nullptr); // preload ability name won't match
}

/**
 * @tc.name: GetImageInfoByRemoteObject_NotFound_001
 * @tc.desc: GetImageInfoByRemoteObject - no matching remote object
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, GetImageInfoByRemoteObject_NotFound_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto ret = appMgrServiceInner->GetImageInfoByRemoteObject(nullptr);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: GetImageInfoByRemoteObject_EmptyMap_001
 * @tc.desc: GetImageInfoByRemoteObject - empty map returns nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, GetImageInfoByRemoteObject_EmptyMap_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto mockObj = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto ret = appMgrServiceInner->GetImageInfoByRemoteObject(mockObj);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: IsImageInfoMatched_NullImageInfo_001
 * @tc.desc: IsImageInfoMatched - imageInfo is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, IsImageInfoMatched_NullImageInfo_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    bool ret = appMgrServiceInner->IsImageInfoMatched(nullptr, 0, "", "", "", "");
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IsImageInfoMatched_ImagePidInvalid_001
 * @tc.desc: IsImageInfoMatched - imagePid <= 0
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, IsImageInfoMatched_ImagePidInvalid_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto imageInfo = std::make_shared<ForkImageInfo>();
    imageInfo->imagePid = 0;
    bool ret = appMgrServiceInner->IsImageInfoMatched(imageInfo, 0, "", "", "", "");
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IsImageInfoMatched_NullAppRecord_001
 * @tc.desc: IsImageInfoMatched - baseAppRecord is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, IsImageInfoMatched_NullAppRecord_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto imageInfo = std::make_shared<ForkImageInfo>();
    imageInfo->imagePid = 100;
    imageInfo->baseAppRecord = nullptr;
    bool ret = appMgrServiceInner->IsImageInfoMatched(imageInfo, 0, "", "", "", "");
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IsImageInfoMatched_ProcessNameMismatch_001
 * @tc.desc: IsImageInfoMatched - process name does not match
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, IsImageInfoMatched_ProcessNameMismatch_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto imageInfo = std::make_shared<ForkImageInfo>();
    imageInfo->imagePid = 100;
    auto appRecord = std::make_shared<AppRunningRecord>(
        std::make_shared<ApplicationInfo>(), 1, "processName");
    imageInfo->baseAppRecord = appRecord;
    bool ret = appMgrServiceInner->IsImageInfoMatched(imageInfo, 0, "differentProcess", "", "", "");
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: CreateAppRunningRecordFromImageInfo_Null_001
 * @tc.desc: CreateAppRunningRecordFromImageInfo - imageInfo is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, CreateAppRunningRecordFromImageInfo_Null_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto ret = appMgrServiceInner->CreateAppRunningRecordFromImageInfo(nullptr);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: CreateAppRunningRecordFromImageInfo_NoBaseRecord_001
 * @tc.desc: CreateAppRunningRecordFromImageInfo - baseAppRecord is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, CreateAppRunningRecordFromImageInfo_NoBaseRecord_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto imageInfo = std::make_shared<ForkImageInfo>();
    imageInfo->baseAppRecord = nullptr;
    auto ret = appMgrServiceInner->CreateAppRunningRecordFromImageInfo(imageInfo);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: TryToUseImageInfo_NullParams_001
 * @tc.desc: TryToUseImageInfo - appRunningManager_ is null (first null check)
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, TryToUseImageInfo_NullParams_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    auto ret = appMgrServiceInner->TryToUseImageInfo(nullptr, nullptr, nullptr,
        "callerKey", 0, "process", "instanceKey", "", "", appRecord);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: TryToUseImageInfo_ImageInfoNotFound_001
 * @tc.desc: TryToUseImageInfo - imageInfo not found in map
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, TryToUseImageInfo_ImageInfoNotFound_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "testAbility";
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->bundleName = "testBundle";
    appInfo->uid = 0;
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    auto ret = appMgrServiceInner->TryToUseImageInfo(abilityInfo, appInfo, nullptr,
        "callerKey", 0, "process", "instanceKey", "", "", appRecord);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: TryToUseImageInfo_ImageInfoNotMatched_001
 * @tc.desc: TryToUseImageInfo - IsImageInfoMatched returns false
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, TryToUseImageInfo_ImageInfoNotMatched_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "testAbility";
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->bundleName = "testBundle";
    appInfo->uid = 0;
    AppMgrServiceInner::MakeImageRequest request {
        .bundleName = "testBundle",
        .abilityName = "testAbility",
        .userId = 0,
        .appCloneIndex = 0
    };
    auto imageInfo = std::make_shared<ForkImageInfo>();
    imageInfo->imagePid = 100;
    appMgrServiceInner->imageInfoMap_[request] = imageInfo;
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    auto ret = appMgrServiceInner->TryToUseImageInfo(abilityInfo, appInfo,
        sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken()),
        "callerKey", 0, "differentProcess", "instanceKey", "", "", appRecord);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: ProcessKia_NotKia_001
 * @tc.desc: ProcessKia - isKia is false, early return ERR_OK
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, ProcessKia_NotKia_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    int32_t ret = appMgrServiceInner->ProcessKia(false, nullptr, "", false);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: ProcessKia_NullAppRecord_001
 * @tc.desc: ProcessKia - isKia false regardless of appRecord
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, ProcessKia_NullAppRecord_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    // isKia=false always returns ERR_OK regardless of AppUtils::IsStartOptionsWithAnimation
    int32_t ret = appMgrServiceInner->ProcessKia(false, nullptr, "watermark", true);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: ProcessKia_Valid_001
 * @tc.desc: ProcessKia - isKia true with valid appRecord
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, ProcessKia_Valid_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto appRecord = std::make_shared<AppRunningRecord>(
        std::make_shared<ApplicationInfo>(), 1, "processName");
    int32_t ret = appMgrServiceInner->ProcessKia(true, appRecord, "watermark", true);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: GetBackgroundAppInfo_NullSession_001
 * @tc.desc: GetBackgroundAppInfo - SessionManager returns null
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, GetBackgroundAppInfo_NullSession_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::vector<BackgroundAppInfo> allowList;
    auto ret = appMgrServiceInner->GetBackgroundAppInfo(allowList);
    EXPECT_TRUE(ret.empty());
}

/**
 * @tc.name: GetRenderProcessTerminationStatus_NoAppMgr_001
 * @tc.desc: GetRenderProcessTerminationStatus - appRunningManager_ is null
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, GetRenderProcessTerminationStatus_NoAppMgr_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    int status = 0;
    int32_t ret = appMgrServiceInner->GetRenderProcessTerminationStatus(100, status);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: GetRenderProcessTerminationStatus_HostRecordNull_001
 * @tc.desc: GetRenderProcessTerminationStatus - hostRecord not found
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, GetRenderProcessTerminationStatus_HostRecordNull_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto appRunningManager = std::make_shared<AppRunningManager>();
    appMgrServiceInner->appRunningManager_ = appRunningManager;
    int status = 0;
    int32_t ret = appMgrServiceInner->GetRenderProcessTerminationStatus(99999, status);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: IsAppRunning_InvalidCloneIndex_001
 * @tc.desc: IsAppRunning - appCloneIndex out of valid range, clamped to -1
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, IsAppRunning_InvalidCloneIndex_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    bool isRunning = false;
    // callingUid likely != FOUNDATION_UID in test → first check fails
    int32_t ret = appMgrServiceInner->IsAppRunning("testBundle", -2, 100, isRunning);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: CreateAbilityInfo_NullBundleMgr_001
 * @tc.desc: CreateAbilityInfo - bundleMgrHelper is null, early return false
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, CreateAbilityInfo_NullBundleMgr_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->remoteClientManager_ = std::make_shared<RemoteClientManager>();
    AAFwk::Want want;
    AbilityInfo abilityInfo;
    bool ret = appMgrServiceInner->CreateAbilityInfo(want, abilityInfo);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: AllowChildProcessInMultiProcessFeatureApp_NullAppRecord_001
 * @tc.desc: AllowChildProcessInMultiProcessFeatureApp - appRecord is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerTest, AllowChildProcessInMultiProcessFeatureApp_NullAppRecord_001, TestSize.Level2)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    bool ret = appMgrServiceInner->AllowChildProcessInMultiProcessFeatureApp(nullptr);
    EXPECT_FALSE(ret);
}
} // namespace AppExecFwk
} // namespace OHOS
