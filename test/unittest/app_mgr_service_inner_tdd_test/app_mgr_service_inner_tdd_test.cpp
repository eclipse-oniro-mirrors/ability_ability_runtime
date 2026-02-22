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
#include <thread>

#define private public
#include "app_mgr_service_inner.h"
#include "app_running_record.h"
#include "remote_client_manager.h"
#undef private
#include "app_scheduler.h"
#include "app_utils.h"
#include "app_mgr_event.h"
#include "app_spawn_client.h"
#include "event_handler.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "global_constant.h"
#include "mock_ability_token.h"
#include "mock_app_scheduler.h"
#include "mock_bundle_manager.h"
#include "mock_configuration_observer.h"
#include "mock_iapp_state_callback.h"
#include "mock_native_token.h"
#include "mock_render_scheduler.h"
#include "parameters.h"
#include "window_manager.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
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
 * @tc.name: SendProcessStartEvent_001
 * @tc.desc: Verify that the SendProcessStartEvent interface calls abnormal parameter
 * @tc.type: FUNC
 * @tc.Function: SendProcessStartEvent
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, SendProcessStartEvent_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartEvent_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->Init();
    EXPECT_NE(appMgrServiceInner, nullptr);
    EXPECT_FALSE(appMgrServiceInner->SendProcessStartEvent(nullptr, false, AppExecFwk::PreloadMode::PRESS_DOWN));
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartEvent_001 end");
}

/**
 * @tc.name: SendProcessStartEvent_002
 * @tc.desc: Verify that the SendProcessStartEvent interface calls just like a service called
 * @tc.type: FUNC
 * @tc.Function: SendProcessStartEvent
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, SendProcessStartEvent_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartEvent_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->Init();
    EXPECT_NE(appMgrServiceInner, nullptr);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo, "");
    EXPECT_NE(appRecord, nullptr);
    EXPECT_TRUE(appMgrServiceInner->SendProcessStartEvent(appRecord, false, AppExecFwk::PreloadMode::PRESS_DOWN));
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartEvent_002 end");
}

/**
 * @tc.name: SendProcessStartEvent_003
 * @tc.desc: Verify that the SendProcessStartEvent interface calls like a service called
 * @tc.type: FUNC
 * @tc.Function: SendProcessStartEvent
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, SendProcessStartEvent_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartEvent_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->Init();
    EXPECT_NE(appMgrServiceInner, nullptr);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo, "");
    EXPECT_NE(appRecord, nullptr);
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<ModuleRunningRecord> moduleRunningRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);
    auto abilityRecordEmpty = std::make_shared<AbilityRunningRecord>(nullptr, token, 0);
    moduleRunningRecord->abilities_[token] = abilityRecordEmpty;
    std::vector<std::shared_ptr<ModuleRunningRecord>> moduleRecordList = { moduleRunningRecord };
    appRecord->hapModules_["moduleRecordList"] = moduleRecordList;
    EXPECT_TRUE(appMgrServiceInner->SendProcessStartEvent(appRecord, false, AppExecFwk::PreloadMode::PRESS_DOWN));
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartEvent_003 end");
}

/**
 * @tc.name: SendProcessStartEvent_004
 * @tc.desc: Verify that the SendProcessStartEvent interface calls like a service called
 * @tc.type: FUNC
 * @tc.Function: SendProcessStartEvent
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, SendProcessStartEvent_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartEvent_004 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->Init();
    EXPECT_NE(appMgrServiceInner, nullptr);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo, "");
    EXPECT_NE(appRecord, nullptr);
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<ModuleRunningRecord> moduleRunningRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);
    auto abilityRecordEmpty = std::make_shared<AbilityRunningRecord>(nullptr, token, 0);
    moduleRunningRecord->abilities_[token] = abilityRecordEmpty;
    std::vector<std::shared_ptr<ModuleRunningRecord>> moduleRecordList = { moduleRunningRecord };
    appRecord->hapModules_["moduleRecordList"] = moduleRecordList;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    auto abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 0);
    moduleRunningRecord->abilities_[token] = abilityRecord;
    EXPECT_TRUE(appMgrServiceInner->SendProcessStartEvent(appRecord, false, AppExecFwk::PreloadMode::PRESS_DOWN));
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartEvent_004 end");
}

/**
 * @tc.name: SendProcessStartEvent_005
 * @tc.desc: Verify that the SendProcessStartEvent interface calls like a service called
 * @tc.type: FUNC
 * @tc.Function: SendProcessStartEvent
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, SendProcessStartEvent_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartEvent_005 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->Init();
    EXPECT_NE(appMgrServiceInner, nullptr);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo, "");
    EXPECT_NE(appRecord, nullptr);
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<ModuleRunningRecord> moduleRunningRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);
    auto abilityRecordEmpty = std::make_shared<AbilityRunningRecord>(nullptr, token, 0);
    moduleRunningRecord->abilities_[token] = abilityRecordEmpty;
    std::vector<std::shared_ptr<ModuleRunningRecord>> moduleRecordList = { moduleRunningRecord };
    appRecord->hapModules_["moduleRecordList"] = moduleRecordList;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    auto abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 0);
    moduleRunningRecord->abilities_[token] = abilityRecord;
    appRecord->SetCallerTokenId(IPCSkeleton::GetCallingTokenID());
    EXPECT_TRUE(appMgrServiceInner->SendProcessStartEvent(appRecord, false, AppExecFwk::PreloadMode::PRESS_DOWN));
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartEvent_005 end");
}

/**
 * @tc.name: SendProcessStartEvent_006
 * @tc.desc: Verify that the SendProcessStartEvent interface calls like a service called
 * @tc.type: FUNC
 * @tc.Function: SendProcessStartEvent
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, SendProcessStartEvent_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartEvent_006 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->Init();
    EXPECT_NE(appMgrServiceInner, nullptr);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo, "");
    EXPECT_NE(appRecord, nullptr);
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<ModuleRunningRecord> moduleRunningRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);
    auto abilityRecordEmpty = std::make_shared<AbilityRunningRecord>(nullptr, token, 0);
    moduleRunningRecord->abilities_[token] = abilityRecordEmpty;
    std::vector<std::shared_ptr<ModuleRunningRecord>> moduleRecordList = { moduleRunningRecord };
    appRecord->hapModules_["moduleRecordList"] = moduleRecordList;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    auto abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 0);
    moduleRunningRecord->abilities_[token] = abilityRecord;
    appRecord->SetCallerTokenId(IPCSkeleton::GetCallingTokenID());
    appRecord->SetCallerUid(IPCSkeleton::GetCallingUid());
    EXPECT_TRUE(appMgrServiceInner->SendProcessStartEvent(appRecord, false, AppExecFwk::PreloadMode::PRESS_DOWN));
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartEvent_006 end");
}

/**
 * @tc.name: SendProcessStartEvent_007
 * @tc.desc: Verify that the SendProcessStartEvent interface calls like a application called
 * @tc.type: FUNC
 * @tc.Function: SendProcessStartEvent
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, SendProcessStartEvent_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartEvent_007 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->Init();
    EXPECT_NE(appMgrServiceInner, nullptr);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo, "");
    EXPECT_NE(appRecord, nullptr);
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<ModuleRunningRecord> moduleRunningRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);
    auto abilityRecordEmpty = std::make_shared<AbilityRunningRecord>(nullptr, token, 0);
    moduleRunningRecord->abilities_[token] = abilityRecordEmpty;
    std::vector<std::shared_ptr<ModuleRunningRecord>> moduleRecordList = { moduleRunningRecord };
    appRecord->hapModules_["moduleRecordList"] = moduleRecordList;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    auto abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 0);
    moduleRunningRecord->abilities_[token] = abilityRecord;
    appRecord->SetCallerTokenId(IPCSkeleton::GetCallingTokenID());
    appRecord->SetCallerUid(IPCSkeleton::GetCallingUid());
    auto &recordMap = appMgrServiceInner->appRunningManager_->appRunningRecordMap_;
    auto iter = recordMap.find(IPCSkeleton::GetCallingPid());
    if (iter == recordMap.end()) {
        recordMap.insert({IPCSkeleton::GetCallingPid(), appRecord});
    } else {
        recordMap.erase(iter);
        recordMap.insert({IPCSkeleton::GetCallingPid(), appRecord});
    }
    appRecord->GetPriorityObject()->pid_ = IPCSkeleton::GetCallingPid();
    appRecord->SetCallerPid(IPCSkeleton::GetCallingPid());
    EXPECT_TRUE(appMgrServiceInner->SendProcessStartEvent(appRecord, false, AppExecFwk::PreloadMode::PRESS_DOWN));
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartEvent_007 end");
}

/**
 * @tc.name: SendProcessStartEvent_008
 * @tc.desc: Verify that the SendProcessStartEvent interface calls like a application called
 * @tc.type: FUNC
 * @tc.Function: SendProcessStartEvent
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, SendProcessStartEvent_008, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartEvent_008 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->Init();
    EXPECT_NE(appMgrServiceInner, nullptr);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    applicationInfo_->bundleName = "";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo, "");
    EXPECT_NE(appRecord, nullptr);
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<ModuleRunningRecord> moduleRunningRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);
    auto abilityRecordEmpty = std::make_shared<AbilityRunningRecord>(nullptr, token, 0);
    moduleRunningRecord->abilities_[token] = abilityRecordEmpty;
    std::vector<std::shared_ptr<ModuleRunningRecord>> moduleRecordList = { moduleRunningRecord };
    appRecord->hapModules_["moduleRecordList"] = moduleRecordList;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    auto abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 0);
    moduleRunningRecord->abilities_[token] = abilityRecord;
    appRecord->SetCallerTokenId(IPCSkeleton::GetCallingTokenID());
    appRecord->SetCallerUid(IPCSkeleton::GetCallingUid());
    auto &recordMap = appMgrServiceInner->appRunningManager_->appRunningRecordMap_;
    auto iter = recordMap.find(IPCSkeleton::GetCallingPid());
    if (iter == recordMap.end()) {
        recordMap.insert({IPCSkeleton::GetCallingPid(), appRecord});
    } else {
        recordMap.erase(iter);
        recordMap.insert({IPCSkeleton::GetCallingPid(), appRecord});
    }
    appRecord->GetPriorityObject()->pid_ = IPCSkeleton::GetCallingPid();
    appRecord->SetCallerPid(IPCSkeleton::GetCallingPid());
    EXPECT_TRUE(appMgrServiceInner->SendProcessStartEvent(appRecord, false, AppExecFwk::PreloadMode::PRESS_DOWN));
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartEvent_008 end");
}

/**
 * @tc.name: SendProcessStartFailedEvent_001
 * @tc.desc: Verify that the SendProcessStartFailedEvent interface calls abnormal parameter
 * @tc.type: FUNC
 * @tc.Function: SendProcessStartFailedEvent
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, SendProcessStartFailedEvent_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartFailedEvent_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->Init();
    EXPECT_NE(appMgrServiceInner, nullptr);

    EXPECT_FALSE(appMgrServiceInner->SendProcessStartFailedEvent(nullptr,
        ProcessStartFailedReason::APPSPAWN_FAILED, 0));
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartFailedEvent_001 end");
}

/**
 * @tc.name: SendProcessStartFailedEvent_002
 * @tc.desc: Verify that the SendProcessStartFailedEvent interface calls abnormal parameter
 * @tc.type: FUNC
 * @tc.Function: SendProcessStartFailedEvent
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, SendProcessStartFailedEvent_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartFailedEvent_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->Init();
    EXPECT_NE(appMgrServiceInner, nullptr);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo, "");
    EXPECT_TRUE(
        appMgrServiceInner->SendProcessStartFailedEvent(appRecord, ProcessStartFailedReason::APPSPAWN_FAILED, 0));
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartFailedEvent_002 end");
}

/**
 * @tc.number: SendProcessStartFailedEvent_003
 * @tc.desc: Verify that the SendProcessStartFailedEvent interface calls like a application called
 * @tc.type: FUNC
 * @tc.Function: SendProcessStartFailedEvent
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, SendProcessStartFailedEvent_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartFailedEvent_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->Init();
    EXPECT_NE(appMgrServiceInner, nullptr);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    applicationInfo_->bundleName = "testBundleName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo, "");
    EXPECT_NE(appRecord, nullptr);
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<ModuleRunningRecord> moduleRunningRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);
    auto abilityRecordEmpty = std::make_shared<AbilityRunningRecord>(nullptr, token, 0);
    moduleRunningRecord->abilities_[token] = abilityRecordEmpty;
    std::vector<std::shared_ptr<ModuleRunningRecord>> moduleRecordList = { moduleRunningRecord };
    appRecord->hapModules_["moduleRecordList"] = moduleRecordList;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    auto abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 0);
    moduleRunningRecord->abilities_[token] = abilityRecord;
    appRecord->SetCallerTokenId(IPCSkeleton::GetCallingTokenID());
    appRecord->SetCallerUid(IPCSkeleton::GetCallingUid());
    auto &recordMap = appMgrServiceInner->appRunningManager_->appRunningRecordMap_;
    auto iter = recordMap.find(IPCSkeleton::GetCallingPid());
    if (iter == recordMap.end()) {
        recordMap.insert({IPCSkeleton::GetCallingPid(), appRecord});
    } else {
        recordMap.erase(iter);
        recordMap.insert({IPCSkeleton::GetCallingPid(), appRecord});
    }
    appRecord->GetPriorityObject()->pid_ = IPCSkeleton::GetCallingPid();
    appRecord->SetCallerPid(IPCSkeleton::GetCallingPid());
    EXPECT_TRUE(appMgrServiceInner->SendProcessStartFailedEvent(appRecord, ProcessStartFailedReason::APPSPAWN_FAILED,
        1));
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartFailedEvent_003 end");
}

/**
 * @tc.number: SendProcessStartFailedEvent_004
 * @tc.desc: Verify that the SendProcessStartFailedEvent interface calls like a application called
 * @tc.type: FUNC
 * @tc.Function: SendProcessStartFailedEvent
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, SendProcessStartFailedEvent_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartFailedEvent_004 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->Init();
    EXPECT_NE(appMgrServiceInner, nullptr);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    applicationInfo_->bundleName = "testBundleName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo, "");
    EXPECT_NE(appRecord, nullptr);
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<ModuleRunningRecord> moduleRunningRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);
    auto abilityRecordEmpty = std::make_shared<AbilityRunningRecord>(nullptr, token, 0);
    moduleRunningRecord->abilities_[token] = abilityRecordEmpty;
    std::vector<std::shared_ptr<ModuleRunningRecord>> moduleRecordList = { moduleRunningRecord };
    appRecord->hapModules_["moduleRecordList"] = moduleRecordList;
    appRecord->SetCallerTokenId(IPCSkeleton::GetCallingTokenID());
    appRecord->SetCallerUid(IPCSkeleton::GetCallingUid());
    auto &recordMap = appMgrServiceInner->appRunningManager_->appRunningRecordMap_;
    auto iter = recordMap.find(IPCSkeleton::GetCallingPid());
    if (iter == recordMap.end()) {
        recordMap.insert({IPCSkeleton::GetCallingPid(), appRecord});
    } else {
        recordMap.erase(iter);
        recordMap.insert({IPCSkeleton::GetCallingPid(), appRecord});
    }
    appRecord->GetPriorityObject()->pid_ = IPCSkeleton::GetCallingPid();
    appRecord->SetCallerPid(IPCSkeleton::GetCallingPid());
    EXPECT_TRUE(appMgrServiceInner->SendProcessStartFailedEvent(appRecord, ProcessStartFailedReason::APPSPAWN_FAILED,
        1));
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartFailedEvent_004 end");
}

/**
 * @tc.number: SendProcessStartFailedEvent_005
 * @tc.desc: Verify that the SendProcessStartFailedEvent interface calls like a application called
 * @tc.type: FUNC
 * @tc.Function: SendProcessStartFailedEvent
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, SendProcessStartFailedEvent_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartFailedEvent_005 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->Init();
    EXPECT_NE(appMgrServiceInner, nullptr);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    applicationInfo_->bundleName = "testBundleName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo, "");
    EXPECT_NE(appRecord, nullptr);
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<ModuleRunningRecord> moduleRunningRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);
    std::vector<std::shared_ptr<ModuleRunningRecord>> moduleRecordList = { moduleRunningRecord };
    appRecord->hapModules_["moduleRecordList"] = moduleRecordList;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    auto abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 0);
    moduleRunningRecord->abilities_[token] = abilityRecord;
    appRecord->SetCallerTokenId(IPCSkeleton::GetCallingTokenID());
    appRecord->SetCallerUid(IPCSkeleton::GetCallingUid());
    auto &recordMap = appMgrServiceInner->appRunningManager_->appRunningRecordMap_;
    auto iter = recordMap.find(IPCSkeleton::GetCallingPid());
    if (iter == recordMap.end()) {
        recordMap.insert({IPCSkeleton::GetCallingPid(), appRecord});
    } else {
        recordMap.erase(iter);
        recordMap.insert({IPCSkeleton::GetCallingPid(), appRecord});
    }
    appRecord->GetPriorityObject()->pid_ = IPCSkeleton::GetCallingPid();
    appRecord->SetCallerPid(IPCSkeleton::GetCallingPid());
    EXPECT_TRUE(appMgrServiceInner->SendProcessStartFailedEvent(appRecord, ProcessStartFailedReason::APPSPAWN_FAILED,
        1));
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartFailedEvent_005 end");
}

/**
 * @tc.number: SendProcessStartFailedEvent_006
 * @tc.desc: Verify that the SendProcessStartFailedEvent interface calls like a application called
 * @tc.type: FUNC
 * @tc.Function: SendProcessStartFailedEvent
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, SendProcessStartFailedEvent_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartFailedEvent_006 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->Init();
    EXPECT_NE(appMgrServiceInner, nullptr);

    BundleInfo bundleInfo;
    std::string processName = "test.process";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    applicationInfo_->bundleName = "testBundleName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo, "");
    EXPECT_NE(appRecord, nullptr);
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<ModuleRunningRecord> moduleRunningRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);
    std::vector<std::shared_ptr<ModuleRunningRecord>> moduleRecordList = { moduleRunningRecord };
    appRecord->hapModules_["moduleRecordList"] = moduleRecordList;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    auto abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 0);
    moduleRunningRecord->abilities_[token] = abilityRecord;
    appRecord->SetCallerTokenId(IPCSkeleton::GetCallingTokenID());
    appRecord->SetCallerUid(IPCSkeleton::GetCallingUid());
    auto &recordMap = appMgrServiceInner->appRunningManager_->appRunningRecordMap_;
    auto iter = recordMap.find(IPCSkeleton::GetCallingPid());
    if (iter == recordMap.end()) {
        recordMap.insert({IPCSkeleton::GetCallingPid(), appRecord});
    } else {
        recordMap.erase(iter);
        recordMap.insert({IPCSkeleton::GetCallingPid(), appRecord});
    }
    appRecord->GetPriorityObject()->pid_ = IPCSkeleton::GetCallingPid();
    appRecord->SetCallerPid(IPCSkeleton::GetCallingPid());
    EXPECT_TRUE(appMgrServiceInner->SendProcessStartFailedEvent(appRecord, ProcessStartFailedReason::APPSPAWN_FAILED,
        1));
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartFailedEvent_006 end");
}

/**
 * @tc.number: SendProcessStartFailedEvent_007
 * @tc.desc: Verify that the SendProcessStartFailedEvent interface calls like a application called
 * @tc.type: FUNC
 * @tc.Function: SendProcessStartFailedEvent
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, SendProcessStartFailedEvent_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartFailedEvent_007 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->Init();
    EXPECT_NE(appMgrServiceInner, nullptr);

    BundleInfo bundleInfo;
    std::string processName = "test.process";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    applicationInfo_->bundleName = "";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo, "");
    EXPECT_NE(appRecord, nullptr);
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<ModuleRunningRecord> moduleRunningRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);
    std::vector<std::shared_ptr<ModuleRunningRecord>> moduleRecordList = { moduleRunningRecord };
    appRecord->hapModules_["moduleRecordList"] = moduleRecordList;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    auto abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 0);
    moduleRunningRecord->abilities_[token] = abilityRecord;
    appRecord->SetCallerTokenId(IPCSkeleton::GetCallingTokenID());
    appRecord->SetCallerUid(IPCSkeleton::GetCallingUid());
    auto &recordMap = appMgrServiceInner->appRunningManager_->appRunningRecordMap_;
    auto iter = recordMap.find(IPCSkeleton::GetCallingPid());
    if (iter == recordMap.end()) {
        recordMap.insert({IPCSkeleton::GetCallingPid(), appRecord});
    } else {
        recordMap.erase(iter);
        recordMap.insert({IPCSkeleton::GetCallingPid(), appRecord});
    }
    appRecord->GetPriorityObject()->pid_ = IPCSkeleton::GetCallingPid();
    appRecord->SetCallerPid(IPCSkeleton::GetCallingPid());
    EXPECT_TRUE(appMgrServiceInner->SendProcessStartFailedEvent(appRecord, ProcessStartFailedReason::APPSPAWN_FAILED,
        1));
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartFailedEvent_007 end");
}

/**
 * @tc.number: SendProcessStartFailedEvent_008
 * @tc.desc: Verify that the SendProcessStartFailedEvent interface calls like a application called
 * @tc.type: FUNC
 * @tc.Function: SendProcessStartFailedEvent
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, SendProcessStartFailedEvent_008, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartFailedEvent_008 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->Init();
    EXPECT_NE(appMgrServiceInner, nullptr);

    BundleInfo bundleInfo;
    std::string processName = "test.process";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    applicationInfo_->bundleName = "testBundleName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo, "");
    EXPECT_NE(appRecord, nullptr);
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<ModuleRunningRecord> moduleRunningRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);
    auto abilityRecordEmpty = std::make_shared<AbilityRunningRecord>(nullptr, token, 0);
    moduleRunningRecord->abilities_[token] = abilityRecordEmpty;
    std::vector<std::shared_ptr<ModuleRunningRecord>> moduleRecordList = { moduleRunningRecord };
    appRecord->hapModules_["moduleRecordList"] = moduleRecordList;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    auto abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 0);
    moduleRunningRecord->abilities_[token] = abilityRecord;
    appRecord->SetCallerTokenId(IPCSkeleton::GetCallingTokenID());
    appRecord->SetCallerUid(IPCSkeleton::GetCallingUid());
    auto &recordMap = appMgrServiceInner->appRunningManager_->appRunningRecordMap_;
    auto iter = recordMap.find(IPCSkeleton::GetCallingPid());
    if (iter == recordMap.end()) {
        recordMap.insert({IPCSkeleton::GetCallingPid(), appRecord});
    } else {
        recordMap.erase(iter);
        recordMap.insert({IPCSkeleton::GetCallingPid(), appRecord});
    }
    appRecord->GetPriorityObject()->pid_ = IPCSkeleton::GetCallingPid();
    appRecord->SetCallerPid(IPCSkeleton::GetCallingPid());
    EXPECT_TRUE(appMgrServiceInner->SendProcessStartFailedEvent(appRecord, ProcessStartFailedReason::APPSPAWN_FAILED,
        1));
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartFailedEvent_008 end");
}

/**
 * @tc.number: SendProcessStartFailedEvent_009
 * @tc.desc: Verify that the SendProcessStartFailedEvent interface calls like a application called
 * @tc.type: FUNC
 * @tc.Function: SendProcessStartFailedEvent
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, SendProcessStartFailedEvent_009, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartFailedEvent_009 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->Init();
    EXPECT_NE(appMgrServiceInner, nullptr);

    BundleInfo bundleInfo;
    std::string processName = "test.process";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    applicationInfo_->bundleName = "";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo, "");
    EXPECT_NE(appRecord, nullptr);
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<ModuleRunningRecord> moduleRunningRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);
    auto abilityRecordEmpty = std::make_shared<AbilityRunningRecord>(nullptr, token, 0);
    moduleRunningRecord->abilities_[token] = abilityRecordEmpty;
    std::vector<std::shared_ptr<ModuleRunningRecord>> moduleRecordList = { moduleRunningRecord };
    appRecord->hapModules_["moduleRecordList"] = moduleRecordList;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    auto abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 0);
    moduleRunningRecord->abilities_[token] = abilityRecord;
    appRecord->SetCallerTokenId(IPCSkeleton::GetCallingTokenID());
    appRecord->SetCallerUid(IPCSkeleton::GetCallingUid());
    auto &recordMap = appMgrServiceInner->appRunningManager_->appRunningRecordMap_;
    auto iter = recordMap.find(IPCSkeleton::GetCallingPid());
    if (iter == recordMap.end()) {
        recordMap.insert({IPCSkeleton::GetCallingPid(), appRecord});
    } else {
        recordMap.erase(iter);
        recordMap.insert({IPCSkeleton::GetCallingPid(), appRecord});
    }
    appRecord->GetPriorityObject()->pid_ = IPCSkeleton::GetCallingPid();
    appRecord->SetCallerPid(IPCSkeleton::GetCallingPid());
    EXPECT_TRUE(appMgrServiceInner->SendProcessStartFailedEvent(appRecord, ProcessStartFailedReason::APPSPAWN_FAILED,
        1));
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartFailedEvent_009 end");
}

/**
 * @tc.number: SendProcessStartFailedEvent_010
 * @tc.desc: Verify that the SendProcessStartFailedEvent interface calls like a application called
 * @tc.type: FUNC
 * @tc.Function: SendProcessStartFailedEvent
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, SendProcessStartFailedEvent_010, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartFailedEvent_010 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->Init();
    EXPECT_NE(appMgrServiceInner, nullptr);

    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    applicationInfo_->bundleName = "";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo, "");
    EXPECT_NE(appRecord, nullptr);
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<ModuleRunningRecord> moduleRunningRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);
    std::vector<std::shared_ptr<ModuleRunningRecord>> moduleRecordList = { moduleRunningRecord };
    appRecord->hapModules_["moduleRecordList"] = moduleRecordList;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    auto abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 0);
    moduleRunningRecord->abilities_[token] = abilityRecord;
    appRecord->SetCallerTokenId(IPCSkeleton::GetCallingTokenID());
    appRecord->SetCallerUid(IPCSkeleton::GetCallingUid());
    auto &recordMap = appMgrServiceInner->appRunningManager_->appRunningRecordMap_;
    auto iter = recordMap.find(IPCSkeleton::GetCallingPid());
    if (iter == recordMap.end()) {
        recordMap.insert({IPCSkeleton::GetCallingPid(), appRecord});
    } else {
        recordMap.erase(iter);
        recordMap.insert({IPCSkeleton::GetCallingPid(), appRecord});
    }
    appRecord->GetPriorityObject()->pid_ = IPCSkeleton::GetCallingPid();
    appRecord->SetCallerPid(IPCSkeleton::GetCallingPid());
    EXPECT_TRUE(appMgrServiceInner->SendProcessStartFailedEvent(appRecord, ProcessStartFailedReason::APPSPAWN_FAILED,
        1));
    TAG_LOGI(AAFwkTag::TEST, "SendProcessStartFailedEvent_010 end");
}

/**
 * @tc.name: SendProcessExitEvent_001
 * @tc.desc: Verify that the SendProcessExitEvent interface calls normally
 * @tc.type: FUNC
 * @tc.Function: SendProcessExitEvent
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, SendProcessExitEvent_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendProcessExitEvent_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->Init();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord;
    appMgrServiceInner->SendProcessExitEvent(appRecord);
    TAG_LOGI(AAFwkTag::TEST, "SendProcessExitEvent_001 end");
}

/**
 * @tc.name: SendProcessExitEvent_002
 * @tc.desc: Verify that the SendProcessExitEvent interface calls normally
 * @tc.type: FUNC
 * @tc.Function: SendProcessExitEvent
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, SendProcessExitEvent_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendProcessExitEvent_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->Init();
    EXPECT_NE(appMgrServiceInner, nullptr);
    appMgrServiceInner->SendProcessExitEvent(nullptr);
    TAG_LOGI(AAFwkTag::TEST, "SendProcessExitEvent_002 end");
}

/**
 * @tc.name: SendProcessKillEvent_001
 * @tc.desc: Verify that the SendProcessKillEvent interface calls normally
 * @tc.type: FUNC
 * @tc.Function: SendProcessKillEvent
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, SendProcessKillEvent_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendProcessKillEvent_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->Init();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    appMgrServiceInner->SendProcessKillEvent(appRecord, "OnRemoteDied");
    TAG_LOGI(AAFwkTag::TEST, "SendProcessKillEvent_001 end");
}

/**
 * @tc.name: SendProcessKillEvent_002
 * @tc.desc: Verify that the SendProcessKillEvent interface calls normally
 * @tc.type: FUNC
 * @tc.Function: SendProcessKillEvent
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, SendProcessKillEvent_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendProcessKillEvent_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->Init();
    EXPECT_NE(appMgrServiceInner, nullptr);
    appMgrServiceInner->SendProcessKillEvent(nullptr, "OnRemoteDied");
    TAG_LOGI(AAFwkTag::TEST, "SendProcessKillEvent_002 end");
}

/**
 * @tc.name: CheckIsolationMode_001
 * @tc.desc: CheckIsolationMode
 * @tc.type: FUNC
 * @tc.Function: CheckIsolationMode
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, CheckIsolationMode_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckIsolationMode_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->Init();
    EXPECT_NE(appMgrServiceInner, nullptr);
    HapModuleInfo hapModuleInfo;
    string supportIsolationMode = OHOS::system::GetParameter("persist.bms.supportIsolationMode", "false");
    if (supportIsolationMode.compare("true") == 0) {
        hapModuleInfo.isolationMode = IsolationMode::ISOLATION_FIRST;
        EXPECT_TRUE(appMgrServiceInner->CheckIsolationMode(hapModuleInfo));
        hapModuleInfo.isolationMode = IsolationMode::ISOLATION_ONLY;
        EXPECT_TRUE(appMgrServiceInner->CheckIsolationMode(hapModuleInfo));
        hapModuleInfo.isolationMode = IsolationMode::NONISOLATION_FIRST;
        EXPECT_FALSE(appMgrServiceInner->CheckIsolationMode(hapModuleInfo));
        hapModuleInfo.isolationMode = IsolationMode::NONISOLATION_ONLY;
        EXPECT_FALSE(appMgrServiceInner->CheckIsolationMode(hapModuleInfo));
    } else {
        hapModuleInfo.isolationMode = IsolationMode::ISOLATION_FIRST;
        EXPECT_FALSE(appMgrServiceInner->CheckIsolationMode(hapModuleInfo));
        hapModuleInfo.isolationMode = IsolationMode::ISOLATION_ONLY;
        EXPECT_FALSE(appMgrServiceInner->CheckIsolationMode(hapModuleInfo));
        hapModuleInfo.isolationMode = IsolationMode::NONISOLATION_FIRST;
        EXPECT_FALSE(appMgrServiceInner->CheckIsolationMode(hapModuleInfo));
        hapModuleInfo.isolationMode = IsolationMode::NONISOLATION_ONLY;
        EXPECT_FALSE(appMgrServiceInner->CheckIsolationMode(hapModuleInfo));
    }
    TAG_LOGI(AAFwkTag::TEST, "CheckIsolationMode_001 end");
}

/**
 * @tc.name: NotifyAppFault_001
 * @tc.desc: Verify that the NotifyAppFault interface calls normally
 * @tc.type: FUNC
 * @tc.Function: NotifyAppFault
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, NotifyAppFault_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyAppFault_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->Init();
    EXPECT_NE(appMgrServiceInner, nullptr);
    FaultData faultData;
    EXPECT_EQ(ERR_INVALID_VALUE, appMgrServiceInner->NotifyAppFault(faultData));
    TAG_LOGI(AAFwkTag::TEST, "NotifyAppFault_001 end");
}

/**
 * @tc.name: NotifyAppFaultBySA_001
 * @tc.desc: Verify that the NotifyAppFaultBySA interface calls normally
 * @tc.type: FUNC
 * @tc.Function: NotifyAppFaultBySA
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, NotifyAppFaultBySA_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyAppFaultBySA_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->Init();
    EXPECT_NE(appMgrServiceInner, nullptr);
    AppFaultDataBySA faultData;
    appMgrServiceInner->appRunningManager_ = nullptr;
    EXPECT_EQ(ERR_INVALID_VALUE, appMgrServiceInner->NotifyAppFaultBySA(faultData));
    TAG_LOGI(AAFwkTag::TEST, "NotifyAppFaultBySA_001 end");
}

/**
 * @tc.name: SubmitDfxFaultTask_001
 * @tc.desc: Verify that the SubmitDfxFaultTask interface calls normally
 * @tc.type: FUNC
 * @tc.Function: SubmitDfxFaultTask
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, SubmitDfxFaultTask_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SubmitDfxFaultTask start");
    FaultData faultData;
    std::string bundleName = "SubmitDfxFaultTask_001";
    int32_t recordId = 100;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::string processName = "SubmitDfxFaultTask_001";
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    int32_t pid = getpid();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->Init();
    EXPECT_NE(appMgrServiceInner, nullptr);
    appMgrServiceInner->SubmitDfxFaultTask(faultData, bundleName, appRecord, pid);
    TAG_LOGI(AAFwkTag::TEST, "SubmitDfxFaultTask end");
}

/**
 * @tc.name: FaultTypeToString_001
 * @tc.desc: Verify that the FaultTypeToString interface calls normally
 * @tc.type: FUNC
 * @tc.Function: FaultTypeToString
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, FaultTypeToString_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FaultTypeToString_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->Init();
    EXPECT_NE(appMgrServiceInner, nullptr);
    EXPECT_EQ("CPP_CRASH", appMgrServiceInner->FaultTypeToString(AppExecFwk::FaultDataType::CPP_CRASH));
    EXPECT_EQ("JS_ERROR", appMgrServiceInner->FaultTypeToString(AppExecFwk::FaultDataType::JS_ERROR));
    EXPECT_EQ("APP_FREEZE", appMgrServiceInner->FaultTypeToString(AppExecFwk::FaultDataType::APP_FREEZE));
    EXPECT_EQ("PERFORMANCE_CONTROL",
        appMgrServiceInner->FaultTypeToString(AppExecFwk::FaultDataType::PERFORMANCE_CONTROL));
    EXPECT_EQ("RESOURCE_CONTROL", appMgrServiceInner->FaultTypeToString(AppExecFwk::FaultDataType::RESOURCE_CONTROL));
    TAG_LOGI(AAFwkTag::TEST, "FaultTypeToString_001 end");
}

/**
 * @tc.name: ChangeAppGcState_001
 * @tc.desc: Change app Gc state
 * @tc.type: FUNC
 * @tc.Function: ChangeAppGcState
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, ChangeAppGcState_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ChangeAppGcState_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->Init();
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
 * @tc.Function: ChangeAppGcState
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, ChangeAppGcState_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ChangeAppGcState_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->Init();
    EXPECT_NE(appMgrServiceInner, nullptr);
    int32_t pid = 0;
    int32_t state = 0;
    uint64_t tid = 1;
    int32_t ret = appMgrServiceInner->ChangeAppGcState(pid, state, tid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "ChangeAppGcState_002 end");
}

/**
 * @tc.name: QueryExtensionSandBox_001
 * @tc.desc: query extension sandBox.
 * @tc.type: FUNC
 * @tc.Function: QueryExtensionSandBox
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, QueryExtensionSandBox_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "QueryExtensionSandBox_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->Init();
    EXPECT_NE(appMgrServiceInner, nullptr);
    const string moduleName = "entry";
    const string extensionName = "inputMethod";
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    ExtensionAbilityInfo extensionAbilityInfo;
    extensionAbilityInfo.name = "inputMethod";
    extensionAbilityInfo.moduleName = "entry";
    extensionAbilityInfo.needCreateSandbox = true;
    extensionAbilityInfo.dataGroupIds = {"test1"};
    hapModuleInfo.extensionInfos.emplace_back(extensionAbilityInfo);
    bundleInfo.hapModuleInfos.emplace_back(hapModuleInfo);
    AppSpawnStartMsg startMsg;
    DataGroupInfoList dataGroupInfoList;
    DataGroupInfo dataGroupInfo;
    dataGroupInfo.dataGroupId = "test1";
    dataGroupInfoList.emplace_back(dataGroupInfo);
    appMgrServiceInner->QueryExtensionSandBox(moduleName, extensionName, bundleInfo, startMsg, dataGroupInfoList,
        nullptr);
    EXPECT_EQ(startMsg.dataGroupInfoList.size(), 1);
    TAG_LOGI(AAFwkTag::TEST, "QueryExtensionSandBox_001 end");
}

/**
 * @tc.name: QueryExtensionSandBox_002
 * @tc.desc: query extension sandBox.
 * @tc.type: FUNC
 * @tc.Function: QueryExtensionSandBox
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, QueryExtensionSandBox_002, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "QueryExtensionSandBox_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->Init();
    EXPECT_NE(appMgrServiceInner, nullptr);
    const string moduleName = "entry";
    const string extensionName = "inputMethod";
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    ExtensionAbilityInfo extensionAbilityInfo;
    extensionAbilityInfo.name = "inputMethod";
    extensionAbilityInfo.moduleName = "entry";
    extensionAbilityInfo.needCreateSandbox = true;
    extensionAbilityInfo.dataGroupIds = {"test2"};
    hapModuleInfo.extensionInfos.emplace_back(extensionAbilityInfo);
    bundleInfo.hapModuleInfos.emplace_back(hapModuleInfo);
    AppSpawnStartMsg startMsg;
    DataGroupInfoList dataGroupInfoList;
    DataGroupInfo dataGroupInfo;
    dataGroupInfo.dataGroupId = "test3";
    dataGroupInfoList.emplace_back(dataGroupInfo);
    appMgrServiceInner->QueryExtensionSandBox(moduleName, extensionName, bundleInfo, startMsg, dataGroupInfoList,
        nullptr);
    EXPECT_EQ(startMsg.dataGroupInfoList.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "QueryExtensionSandBox_002 end");
}

/**
 * @tc.name: QueryExtensionSandBox_003
 * @tc.desc: query extension sandBox.
 * @tc.type: FUNC
 * @tc.Function: QueryExtensionSandBox
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, QueryExtensionSandBox_003, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "QueryExtensionSandBox_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->Init();
    EXPECT_NE(appMgrServiceInner, nullptr);
    const string moduleName = "entry";
    const string extensionName = "inputMethod";
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    ExtensionAbilityInfo extensionAbilityInfo;
    extensionAbilityInfo.name = "inputMethod";
    extensionAbilityInfo.moduleName = "entry";
    extensionAbilityInfo.needCreateSandbox = false;
    AppSpawnStartMsg startMsg;
    DataGroupInfoList dataGroupInfoList;
    appMgrServiceInner->QueryExtensionSandBox(moduleName, extensionName, bundleInfo, startMsg, dataGroupInfoList,
        nullptr);
    EXPECT_EQ(startMsg.dataGroupInfoList.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "QueryExtensionSandBox_003 end");
}

/**
 * @tc.name: QueryExtensionSandBox_004
 * @tc.desc: query extension sandBox.
 * @tc.type: FUNC
 * @tc.Function: QueryExtensionSandBox
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, QueryExtensionSandBox_004, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "QueryExtensionSandBox_004 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->Init();
    EXPECT_NE(appMgrServiceInner, nullptr);
    const string moduleName = "entry";
    const string extensionName = "inputMethod";
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    ExtensionAbilityInfo extensionAbilityInfo;
    extensionAbilityInfo.name = "inputMethod1";
    extensionAbilityInfo.moduleName = "entry";
    extensionAbilityInfo.needCreateSandbox = true;
    AppSpawnStartMsg startMsg;
    DataGroupInfoList dataGroupInfoList;
    appMgrServiceInner->QueryExtensionSandBox(moduleName, extensionName, bundleInfo, startMsg, dataGroupInfoList,
        nullptr);
    EXPECT_EQ(startMsg.dataGroupInfoList.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "QueryExtensionSandBox_004 end");
}

/**
 * @tc.name: SetAppEnvInfo_001
 * @tc.desc: The hwasanEnabled is true.
 * @tc.type: FUNC
 * @tc.Function: SetAppEnvInfo
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, SetAppEnvInfo_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetAppEnvInfo_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    BundleInfo info;
    info.applicationInfo.hwasanEnabled = true;
    AppSpawnStartMsg startMsg;
    appMgrServiceInner->SetAppEnvInfo(info, startMsg);
    std::string hwasanEnabled = "hwasanEnabled";
    EXPECT_EQ(startMsg.appEnv.find(hwasanEnabled)->second, "1");
    TAG_LOGI(AAFwkTag::TEST, "SetAppEnvInfo_001 end");
}

/**
 * @tc.name: SetAppEnvInfo_002
 * @tc.desc: The hwasanEnabled is false.
 * @tc.type: FUNC
 * @tc.Function: SetAppEnvInfo
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, SetAppEnvInfo_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetAppEnvInfo_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    BundleInfo info;
    info.applicationInfo.hwasanEnabled = false;
    AppSpawnStartMsg startMsg;
    appMgrServiceInner->SetAppEnvInfo(info, startMsg);
    std::string hwasanEnabled = "hwasanEnabled";
    EXPECT_EQ(startMsg.appEnv.find(hwasanEnabled)->second, "0");
    TAG_LOGI(AAFwkTag::TEST, "SetAppEnvInfo_002 end");
}

/**
 * @tc.name: SetAppEnvInfo_003
 * @tc.desc: The ubsanEnabled is false.
 * @tc.type: FUNC
 * @tc.Function: SetAppEnvInfo
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, SetAppEnvInfo_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetAppEnvInfo_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    BundleInfo info;
    info.applicationInfo.ubsanEnabled = false;
    AppSpawnStartMsg startMsg;
    appMgrServiceInner->SetAppEnvInfo(info, startMsg);
    std::string ubsanEnabled = "ubsanEnabled";
    EXPECT_EQ(startMsg.appEnv.find(ubsanEnabled)->second, "0");
    TAG_LOGI(AAFwkTag::TEST, "SetAppEnvInfo_003 end");
}

/**
 * @tc.name: SetAppEnvInfo_004
 * @tc.desc: The ubsanEnabled is true.
 * @tc.type: FUNC
 * @tc.Function: SetAppEnvInfo
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, SetAppEnvInfo_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetAppEnvInfo_004 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    BundleInfo info;
    info.applicationInfo.ubsanEnabled = true;
    AppSpawnStartMsg startMsg;
    appMgrServiceInner->SetAppEnvInfo(info, startMsg);
    std::string ubsanEnabled = "ubsanEnabled";
    EXPECT_EQ(startMsg.appEnv.find(ubsanEnabled)->second, "1");
    TAG_LOGI(AAFwkTag::TEST, "SetAppEnvInfo_004 end");
}

/**
 * @tc.name: GetKernelPermissions_001
 * @tc.desc: Get kernel permissions
 * @tc.type: FUNC
 * @tc.Function: GetKernelPermissions
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, GetKernelPermissions_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetKernelPermissions_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    AppSpawnStartMsg startMsg;
    std::map<std::string, std::string> permissionsMap;
    uint32_t accessTokenId = 0;
    appMgrServiceInner->GetKernelPermissions(accessTokenId, permissionsMap);
    EXPECT_EQ(permissionsMap.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "GetKernelPermissions_001 end");
}

/**
 * @tc.name: KillSubProcessBypidInner
 * @tc.desc: Kill subProcess inner
 * @tc.type: FUNC
 * @tc.Function: KillSubProcessBypidInner
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, KillSubProcessBypidInner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillSubProcessBypidInner_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    pid_t pid = 0;
    std::string reason = "test";
    AAFwk::EventInfo eventInfo;
    auto ret = appMgrServiceInner->KillSubProcessBypidInner(pid, reason, eventInfo);
    EXPECT_EQ(ret, AAFwk::ERR_KILL_PROCESS_NOT_EXIST);

    pid = 65536;
    ret = appMgrServiceInner->KillSubProcessBypidInner(pid, reason, eventInfo);
    EXPECT_NE(ret, 0);

    TAG_LOGI(AAFwkTag::TEST, "KillSubProcessBypidInner_001 end");
}

/**
 * @tc.name: KillSubProcessBypidInner
 * @tc.desc: Kill subProcess inner
 * @tc.type: FUNC
 * @tc.Function: KillSubProcessBypidInner
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, KillSubProcessBypidInner_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillSubProcessBypidInner_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    pid_t pid = 0;
    std::string reason = "test";
    AAFwk::EventInfo eventInfo;

    pid = fork();
    if (pid == -1) {
        TAG_LOGI(AAFwkTag::TEST, "fork process failed");
    } else if (pid == 0) {
        TAG_LOGI(AAFwkTag::TEST, "fork process success");
        sleep(3);
        exit(0);
    }
    auto ret = appMgrServiceInner->KillSubProcessBypidInner(pid, reason, eventInfo);
    EXPECT_EQ(ret, 0);

    TAG_LOGI(AAFwkTag::TEST, "KillSubProcessBypidInner_002 end");
}

/**
 * @tc.name: KillSubProcessBypid
 * @tc.desc: Kill subProcess
 * @tc.type: FUNC
 * @tc.Function: KillSubProcessBypid
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, KillSubProcessBypid_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillSubProcessBypid_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    pid_t pid = 111;
    std::string reason = "test";
    auto ret = 0;
    appMgrServiceInner->appRunningManager_ = nullptr;
    ret = appMgrServiceInner->KillSubProcessBypid(pid, reason);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "KillSubProcessBypid_001 end");
}

/**
 * @tc.name: KillSubProcessBypid
 * @tc.desc: Kill subProcess
 * @tc.type: FUNC
 * @tc.Function: KillSubProcessBypid
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, KillSubProcessBypid_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillSubProcessBypid_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    pid_t pid = 1;
    std::string reason = "test";
    auto ret = 0;
    auto appRunningManager = std::make_shared<AppRunningManager>();

    int32_t recordId = 1;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::string processName = "testProcess";
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    
    std::map<int32_t, std::shared_ptr<RenderRecord>> renderRecordMap;
    int32_t uid = 1;
    std::shared_ptr<RenderRecord> renderRecord = std::make_shared<RenderRecord>(
        pid, "param", FdGuard(-1), FdGuard(-1), FdGuard(-1), appRecord);
    renderRecord->SetPid(pid);
    renderRecordMap.emplace(uid, renderRecord);
    appRecord->renderRecordMap_ = renderRecordMap;

    appRunningManager->appRunningRecordMap_.emplace(recordId, appRecord);

    appMgrServiceInner->appRunningManager_ = appRunningManager;
    ret = appMgrServiceInner->KillSubProcessBypid(pid, reason);
    EXPECT_EQ(ret, 0);

    TAG_LOGI(AAFwkTag::TEST, "KillSubProcessBypid_002 end");
}

/**
 * @tc.name: KillSubProcessBypid
 * @tc.desc: Kill subProcess
 * @tc.type: FUNC
 * @tc.Function: KillSubProcessBypid
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, KillSubProcessBypid_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillSubProcessBypid_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    pid_t pid = 1;
    std::string reason = "test";
    auto ret = 0;
    auto appRunningManager = std::make_shared<AppRunningManager>();

    int32_t recordId = 1;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::string processName = "testProcess";
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    
    std::map<pid_t, std::shared_ptr<ChildProcessRecord>> childProcessRecordMap;
    std::string subProcessName = "subTestProcess";
    std::shared_ptr<ApplicationInfo> subAppInfo = std::make_shared<ApplicationInfo>();
    auto subAppRecord = std::make_shared<AppRunningRecord>(subAppInfo, recordId, subProcessName);
    ChildProcessRequest request;
    auto childProcessRecord = std::make_shared<ChildProcessRecord>(pid, request, subAppRecord);

    childProcessRecordMap.emplace(pid, childProcessRecord);
    appRecord->childProcessRecordMap_ = childProcessRecordMap;

    appRunningManager->appRunningRecordMap_.emplace(recordId, appRecord);
    appMgrServiceInner->appRunningManager_ = appRunningManager;
    ret = appMgrServiceInner->KillSubProcessBypid(pid, reason);
    EXPECT_EQ(ret, 0);

    TAG_LOGI(AAFwkTag::TEST, "KillSubProcessBypid_003 end");
}

/**
 * @tc.name: KillSubProcessBypid
 * @tc.desc: Kill subProcess
 * @tc.type: FUNC
 * @tc.Function: KillSubProcessBypid
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, KillSubProcessBypid_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillSubProcessBypid_004 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    pid_t pid = 1;
    std::string reason = "test";
    auto ret = 0;
    auto appRunningManager = std::make_shared<AppRunningManager>();

    int32_t recordId = 1;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::string processName = "testProcess";
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    
    std::map<pid_t, std::weak_ptr<AppRunningRecord>> childAppRecordMap;
    std::shared_ptr<ApplicationInfo> subAppInfo = std::make_shared<ApplicationInfo>();
    std::string subProcessName = "subTestProcess";
    auto subAppRecord = std::make_shared<AppRunningRecord>(subAppInfo, recordId, subProcessName);
    childAppRecordMap.emplace(pid, subAppRecord);
    appRecord->childAppRecordMap_ = childAppRecordMap;

    appRunningManager->appRunningRecordMap_.emplace(recordId, appRecord);
    appMgrServiceInner->appRunningManager_ = appRunningManager;
    ret = appMgrServiceInner->KillSubProcessBypid(pid, reason);
    EXPECT_EQ(ret, 0);

    TAG_LOGI(AAFwkTag::TEST, "KillSubProcessBypid_004 end");
}

/**
 * @tc.name: KillSubProcessBypid
 * @tc.desc: Kill subProcess
 * @tc.type: FUNC
 * @tc.Function: KillSubProcessBypid
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, KillSubProcessBypid_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillSubProcessBypid_005 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    pid_t pid = 2;
    std::string reason = "test";
    auto ret = 0;
    auto appRunningManager = std::make_shared<AppRunningManager>();

    int32_t recordId = 1;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::string processName = "testProcess";
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    
    std::map<pid_t, std::weak_ptr<AppRunningRecord>> childAppRecordMap;
    std::shared_ptr<ApplicationInfo> subAppInfo = std::make_shared<ApplicationInfo>();
    std::string subProcessName = "subTestProcess";
    auto subAppRecord = std::make_shared<AppRunningRecord>(subAppInfo, recordId, subProcessName);
    childAppRecordMap.emplace(pid, subAppRecord);
    appRecord->childAppRecordMap_ = childAppRecordMap;

    appRunningManager->appRunningRecordMap_.emplace(recordId, appRecord);
    appMgrServiceInner->appRunningManager_ = appRunningManager;
    ret = appMgrServiceInner->KillSubProcessBypid(pid, reason);
    EXPECT_EQ(ret, 0);

    TAG_LOGI(AAFwkTag::TEST, "KillSubProcessBypid_005 end");
}

/**
 * @tc.name: GetLoadTimeout
 * @tc.desc: Get load timeout
 * @tc.type: FUNC
 * @tc.Function: GetLoadTimeout
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, GetLoadTimeout_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetLoadTimeout_001 start");
    auto timeout = AppMgrServiceInner::GetLoadTimeout(12);
    EXPECT_EQ(timeout, 12);
    TAG_LOGI(AAFwkTag::TEST, "GetLoadTimeout_001 end");
}

/**
 * @tc.name: GetLoadTimeout
 * @tc.desc: Get load timeout
 * @tc.type: FUNC
 * @tc.Function: GetLoadTimeout
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, GetLoadTimeout_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetLoadTimeout_002 start");
    auto timeout = AppMgrServiceInner::GetLoadTimeout(0);
    auto timeoutDefault = AbilityRuntime::GlobalConstant::GetLoadTimeOutBase() *
        AAFwk::AppUtils::GetInstance().GetTimeoutUnitTimeRatio();
    EXPECT_EQ(timeout, timeoutDefault);
    TAG_LOGI(AAFwkTag::TEST, "GetLoadTimeout_002 end");
}
} // namespace AppExecFwk
} // namespace OHOS