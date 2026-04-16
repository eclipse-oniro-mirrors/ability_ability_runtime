/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
#include <limits>
#define private public
#include "app_mgr_service_inner.h"
#include "iservice_registry.h"
#undef private

#include "ability_info.h"
#include "ability_running_record.h"
#include "app_mgr_service.h"
#include "app_record_id.h"
#include "app_running_record.h"
#include "application_info.h"
#include "bundle_mgr_interface.h"
#include "gtest/gtest.h"
#include "hilog_tag_wrapper.h"
#include "mock_app_scheduler.h"
#include "mock_ability_token.h"
#include "mock_app_spawn_client.h"
#include "mock_bundle_manager_service.h"
#include "param.h"

using namespace testing::ext;
using testing::_;
using testing::Return;
using testing::SetArgReferee;
using ::testing::DoAll;

namespace OHOS {
namespace AppExecFwk {
#define CHECK_POINTER_IS_NULLPTR(object) \
    do {                                 \
        if (object == nullptr) {         \
            return;                      \
        }                                \
    } while (0)

class AmsServiceLoadAbilityProcessTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    std::shared_ptr<BundleMgrHelper> bundleMgrHelper_{ nullptr };
    sptr<MockBundleManagerService> mockBundleMgr = nullptr;

protected:
    static const std::string GetTestAppName()
    {
        return "com.ohos.test.helloworld";
    }
    static const std::string GetTestAbilityName()
    {
        return "test_ability_name";
    }

    std::shared_ptr<AppRunningRecord> StartLoadAbility(const sptr<IRemoteObject>& token,
        const sptr<IRemoteObject>& preToken, const std::shared_ptr<AbilityInfo>& abilityInfo,
        const std::shared_ptr<ApplicationInfo>& appInfo, const pid_t newPid) const;

    sptr<MockAbilityToken> GetMockToken() const
    {
        return mock_token_;
    }

protected:
    std::shared_ptr<AppMgrServiceInner> service_;
    sptr<MockAbilityToken> mock_token_;
};

void AmsServiceLoadAbilityProcessTest::SetUpTestCase()
{}

void AmsServiceLoadAbilityProcessTest::TearDownTestCase()
{}

void AmsServiceLoadAbilityProcessTest::SetUp()
{
    service_.reset(new (std::nothrow) AppMgrServiceInner());
    service_->Init();
    mock_token_ = new (std::nothrow) MockAbilityToken();
    bundleMgrHelper_ = DelayedSingleton<BundleMgrHelper>::GetInstance();
    mockBundleMgr = sptr<MockBundleManagerService>::MakeSptr();
    bundleMgrHelper_->bundleMgr_ = mockBundleMgr;
}

void AmsServiceLoadAbilityProcessTest::TearDown() {}

std::shared_ptr<AppRunningRecord> AmsServiceLoadAbilityProcessTest::StartLoadAbility(const sptr<IRemoteObject>& token,
    const sptr<IRemoteObject>& preToken, const std::shared_ptr<AbilityInfo>& abilityInfo,
    const std::shared_ptr<ApplicationInfo>& appInfo, const pid_t newPid) const
{
    std::shared_ptr<MockAppSpawnClient> mockClientPtr = std::make_shared<MockAppSpawnClient>();
    EXPECT_CALL(*mockClientPtr, StartProcess(_, _)).Times(1).WillOnce(DoAll(SetArgReferee<1>(newPid), Return(ERR_OK)));

    service_->SetAppSpawnClient(mockClientPtr);
    AbilityRuntime::LoadParam loadParam;
    loadParam.token = token;
    loadParam.preToken = preToken;
    auto loadParamPtr = std::make_shared<AbilityRuntime::LoadParam>(loadParam);
    service_->LoadAbility(abilityInfo, appInfo, nullptr, loadParamPtr);
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";

    auto record = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, abilityInfo->process, appInfo->uid, bundleInfo);
    EXPECT_EQ(record->GetPriorityObject()->GetPid(), newPid);
    return record;
}

/*
 * Feature: AMS
 * Function: Service
 * SubFunction: NA
 * FunctionPoints: When Service receive loadAbility requests.
 * EnvConditions: NA
 * CaseDescription: Normal loadAbility requesets handled inner service.
 */
HWTEST_F(AmsServiceLoadAbilityProcessTest, LoadAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest LoadAbility_001 start");
    EXPECT_CALL(*mockBundleMgr, GetBundleInfoV9(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(true))
        .WillRepeatedly(testing::Return(true));
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    abilityInfo->process = GetTestAppName();
    abilityInfo->applicationInfo.bundleName = GetTestAppName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();
    const pid_t PID = 1234;
    EXPECT_TRUE(service_ != nullptr);
    sptr<IRemoteObject> token = GetMockToken();
    AbilityRuntime::LoadParam loadParam;
    loadParam.token = token;
    auto loadParamPtr = std::make_shared<AbilityRuntime::LoadParam>(loadParam);
    service_->LoadAbility(abilityInfo, appInfo, nullptr, loadParamPtr);

    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";

    auto record = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, GetTestAppName(), appInfo->uid, bundleInfo);
    EXPECT_EQ(record, nullptr);
    CHECK_POINTER_IS_NULLPTR(record);
    EXPECT_EQ(record->GetState(), ApplicationState::APP_STATE_CREATE);
    const auto& abilityMap = record->GetAbilities();
    EXPECT_EQ(abilityMap.size(), (uint32_t)1);
    auto abilityRecord = record->GetAbilityRunningRecordByToken(token);
    EXPECT_NE(abilityRecord, nullptr);
    CHECK_POINTER_IS_NULLPTR(abilityRecord);
    EXPECT_EQ(abilityRecord->GetState(), AbilityState::ABILITY_STATE_CREATE);
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest LoadAbility_001 end");
}

/*
 * Feature: AMS
 * Function: Service
 * SubFunction: NA
 * FunctionPoints: When Service receive loadAbility requests.
 * EnvConditions: NA
 * CaseDescription: Multiple different loadAbility requesets handled inner service.
 */
HWTEST_F(AmsServiceLoadAbilityProcessTest, LoadAbility_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest LoadAbility_002 start");

    EXPECT_TRUE(service_ != nullptr);

    sptr<IRemoteObject> token = GetMockToken();
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    abilityInfo->process = GetTestAppName();
    abilityInfo->applicationInfo.bundleName = GetTestAppName();

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();
    appInfo->process = GetTestAppName();

    const pid_t PID = 1234;
    AbilityRuntime::LoadParam loadParam;
    loadParam.token = token;
    auto loadParamPtr = std::make_shared<AbilityRuntime::LoadParam>(loadParam);
    service_->LoadAbility(abilityInfo, appInfo, nullptr, loadParamPtr);

    const auto& recordMap = service_->appRunningManager_->GetAppRunningRecordMap();
    EXPECT_EQ(recordMap.size(), 0);
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";

    auto record = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, GetTestAppName(), appInfo->uid, bundleInfo);
    EXPECT_EQ(record, nullptr);
    CHECK_POINTER_IS_NULLPTR(record);
    EXPECT_EQ(record->GetState(), ApplicationState::APP_STATE_CREATE);
    const auto& abilityMap = record->GetAbilities();
    EXPECT_EQ(abilityMap.size(), (uint32_t)1);

    auto abilityRecord = record->GetAbilityRunningRecordByToken(token);
    EXPECT_NE(abilityRecord, nullptr);
    CHECK_POINTER_IS_NULLPTR(abilityRecord);
    EXPECT_EQ(abilityRecord->GetState(), AbilityState::ABILITY_STATE_CREATE);

    sptr<IRemoteObject> token2 = GetMockToken();
    auto abilityInfo2 = std::make_shared<AbilityInfo>();
    abilityInfo2->name = GetTestAbilityName() + "_1";
    abilityInfo2->applicationName = "com.ohos.test.special";
    abilityInfo2->process = "com.ohos.test.special";
    abilityInfo2->applicationInfo.bundleName = "com.ohos.test.special";

    auto appInfo2 = std::make_shared<ApplicationInfo>();
    appInfo2->name = "com.ohos.test.special";
    appInfo2->bundleName = "com.ohos.test.special";
    const pid_t PID2 = 2234;
    AbilityRuntime::LoadParam loadParam2;
    loadParam2.token = token2;
    loadParam2.preToken = token;
    auto loadParamPtr2 = std::make_shared<AbilityRuntime::LoadParam>(loadParam2);
    service_->LoadAbility(abilityInfo2, appInfo2, nullptr, loadParamPtr2);
    const uint32_t EXPECT_MAP_SIZE = 2;
    if (recordMap.size() == EXPECT_MAP_SIZE) {
        auto record2 = service_->appRunningManager_->CheckAppRunningRecordIsExist(
            appInfo2->name, "com.ohos.test.special", appInfo2->uid, bundleInfo);
        EXPECT_NE(record2, nullptr);
        CHECK_POINTER_IS_NULLPTR(record2);
        EXPECT_EQ(record2->GetState(), ApplicationState::APP_STATE_CREATE);
        auto abilityRecord2 = record2->GetAbilityRunningRecordByToken(token2);
        EXPECT_NE(abilityRecord2, nullptr);
        CHECK_POINTER_IS_NULLPTR(abilityRecord2);
        EXPECT_EQ(abilityRecord2->GetState(), AbilityState::ABILITY_STATE_CREATE);
        TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest LoadAbility_002 end");
    }
}

/*
 * Feature: AMS
 * Function: Service
 * SubFunction: NA
 * FunctionPoints: When Service receive loadAbility requests.
 * EnvConditions: NA
 * CaseDescription: Null abilityId loadAbility requesets handled inner service.
 */
HWTEST_F(AmsServiceLoadAbilityProcessTest, LoadAbility_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest LoadAbility_003 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    abilityInfo->applicationInfo.bundleName = GetTestAppName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();
    EXPECT_TRUE(service_ != nullptr);

    std::shared_ptr<MockAppSpawnClient> mockClientPtr = std::make_shared<MockAppSpawnClient>();
    const void* clientPtr = static_cast<const void*>(mockClientPtr.get());
    EXPECT_CALL(*mockClientPtr, StartProcess(_, _)).Times(0);

    service_->SetAppSpawnClient(mockClientPtr);
    auto loadParamPtr = std::make_shared<AbilityRuntime::LoadParam>();
    service_->LoadAbility(abilityInfo, appInfo, nullptr, loadParamPtr);

    const auto& recordMap = service_->appRunningManager_->GetAppRunningRecordMap();
    EXPECT_EQ(recordMap.size(), (uint32_t)0);
    testing::Mock::AllowLeak(clientPtr);
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest LoadAbility_003 end");
}

/*
 * Feature: AMS
 * Function: Service
 * SubFunction: NA
 * FunctionPoints: When Service receive loadAbility requests.
 * EnvConditions: NA
 * CaseDescription: Null abilityInfo name loadAbility requesets handled inner service.
 */
HWTEST_F(AmsServiceLoadAbilityProcessTest, LoadAbility_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest LoadAbility_004 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "";
    abilityInfo->applicationName = GetTestAppName();
    abilityInfo->applicationInfo.bundleName = GetTestAppName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();
    EXPECT_TRUE(service_ != nullptr);

    std::shared_ptr<MockAppSpawnClient> mockClientPtr = std::make_shared<MockAppSpawnClient>();
    const void* clientPtr = static_cast<const void*>(mockClientPtr.get());
    EXPECT_CALL(*mockClientPtr, StartProcess(_, _)).Times(0);

    service_->SetAppSpawnClient(mockClientPtr);
    AbilityRuntime::LoadParam loadParam;
    loadParam.token = GetMockToken();
    auto loadParamPtr = std::make_shared<AbilityRuntime::LoadParam>(loadParam);
    service_->LoadAbility(abilityInfo, appInfo, nullptr, loadParamPtr);
    const auto& recordMap = service_->appRunningManager_->GetAppRunningRecordMap();
    EXPECT_EQ(recordMap.size(), (uint32_t)0);
    testing::Mock::AllowLeak(clientPtr);
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest LoadAbility_004 end");
}

/*
 * Feature: AMS
 * Function: Service
 * SubFunction: NA
 * FunctionPoints: When Service receive loadAbility requests.
 * EnvConditions: NA
 * CaseDescription: Null appInfo name loadAbility requesets handled inner service.
 */
HWTEST_F(AmsServiceLoadAbilityProcessTest, LoadAbility_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest LoadAbility_005 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = "";
    abilityInfo->applicationInfo.bundleName = "";
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = "";
    appInfo->bundleName = "";
    EXPECT_TRUE(service_ != nullptr);
    std::shared_ptr<MockAppSpawnClient> mockClientPtr = std::make_shared<MockAppSpawnClient>();
    const void* clientPtr = static_cast<const void*>(mockClientPtr.get());
    EXPECT_CALL(*mockClientPtr, StartProcess(_, _)).Times(0);

    service_->SetAppSpawnClient(mockClientPtr);
    AbilityRuntime::LoadParam loadParam;
    loadParam.token = GetMockToken();
    auto loadParamPtr = std::make_shared<AbilityRuntime::LoadParam>(loadParam);
    service_->LoadAbility(abilityInfo, appInfo, nullptr, loadParamPtr);
    const auto& recordMap = service_->appRunningManager_->GetAppRunningRecordMap();
    EXPECT_EQ(recordMap.size(), (uint32_t)0);
    testing::Mock::AllowLeak(clientPtr);
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest LoadAbility_005 end");
}

/*
 * Feature: AMS
 * Function: Service
 * SubFunction: NA
 * FunctionPoints: When Service receive loadAbility requests.
 * EnvConditions: NA
 * CaseDescription: Different name loadAbility requesets handled inner service.
 */
HWTEST_F(AmsServiceLoadAbilityProcessTest, LoadAbility_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest LoadAbility_006 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName() + "_1";
    abilityInfo->applicationInfo.bundleName = GetTestAppName() + "_1";
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();
    EXPECT_TRUE(service_ != nullptr);

    std::shared_ptr<MockAppSpawnClient> mockClientPtr = std::make_shared<MockAppSpawnClient>();
    const void* clientPtr = static_cast<const void*>(mockClientPtr.get());
    EXPECT_CALL(*mockClientPtr, StartProcess(_, _)).Times(0);

    service_->SetAppSpawnClient(mockClientPtr);
    AbilityRuntime::LoadParam loadParam;
    loadParam.token = GetMockToken();
    auto loadParamPtr = std::make_shared<AbilityRuntime::LoadParam>(loadParam);
    service_->LoadAbility(abilityInfo, appInfo, nullptr, loadParamPtr);
    const auto& recordMap = service_->appRunningManager_->GetAppRunningRecordMap();
    EXPECT_EQ(recordMap.size(), (uint32_t)0);
    testing::Mock::AllowLeak(clientPtr);
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest LoadAbility_006 end");
}

/*
 * Feature: AMS
 * Function: Service
 * SubFunction: NA
 * FunctionPoints: When Service receive loadAbility requests.
 * EnvConditions: NA
 * CaseDescription: Multiple same loadAbility requesets handled inner service.
 */
HWTEST_F(AmsServiceLoadAbilityProcessTest, LoadAbility_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest LoadAbility_007 start");
    sptr<IRemoteObject> token = GetMockToken();
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    abilityInfo->process = GetTestAppName();
    abilityInfo->applicationInfo.bundleName = GetTestAppName();

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();
    const pid_t PID = 1234;
    EXPECT_TRUE(service_ != nullptr);
    AbilityRuntime::LoadParam loadParam;
    loadParam.token = token;
    auto loadParamPtr = std::make_shared<AbilityRuntime::LoadParam>(loadParam);
    service_->LoadAbility(abilityInfo, appInfo, nullptr, loadParamPtr);
    const auto& recordMap = service_->appRunningManager_->GetAppRunningRecordMap();
    EXPECT_EQ(recordMap.size(), 0);
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";

    auto record = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, GetTestAppName(), appInfo->uid, bundleInfo);
    EXPECT_EQ(record, nullptr);
    CHECK_POINTER_IS_NULLPTR(record);
    EXPECT_EQ(record->GetState(), ApplicationState::APP_STATE_CREATE);
    const auto& abilityMap = record->GetAbilities();
    EXPECT_EQ(abilityMap.size(), (uint32_t)1);
    auto abilityRecord = record->GetAbilityRunningRecordByToken(token);
    EXPECT_NE(abilityRecord, nullptr);
    CHECK_POINTER_IS_NULLPTR(abilityRecord);
    EXPECT_EQ(abilityRecord->GetState(), AbilityState::ABILITY_STATE_CREATE);
    std::shared_ptr<MockAppSpawnClient> mockClientPtr = std::make_shared<MockAppSpawnClient>();
    EXPECT_CALL(*mockClientPtr, StartProcess(_, _)).Times(0);

    service_->SetAppSpawnClient(mockClientPtr);
    service_->LoadAbility(abilityInfo, appInfo, nullptr, loadParamPtr);
    EXPECT_EQ(recordMap.size(), (uint32_t)1);

    auto record2 = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, GetTestAppName(), appInfo->uid, bundleInfo);
    EXPECT_EQ(record2, record);
    const auto& abilityMap2 = record2->GetAbilities();
    EXPECT_EQ(abilityMap2.size(), (uint32_t)1);
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest LoadAbility_007 end");
}

/*
 * Feature: AMS
 * Function: Service
 * SubFunction: NA
 * FunctionPoints: When Service receive loadAbility requests.
 * EnvConditions: NA
 * CaseDescription: Multiple different ability with same appName loadAbility requesets handled inner service.
 */
HWTEST_F(AmsServiceLoadAbilityProcessTest, LoadAbility_008, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest LoadAbility_008 start");
    sptr<IRemoteObject> token = GetMockToken();
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    abilityInfo->process = GetTestAppName();
    abilityInfo->applicationInfo.bundleName = GetTestAppName();

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();
    const pid_t PID = 1234;
    EXPECT_TRUE(service_ != nullptr);
    AbilityRuntime::LoadParam loadParam;
    loadParam.token = token;
    auto loadParamPtr = std::make_shared<AbilityRuntime::LoadParam>(loadParam);
    service_->LoadAbility(abilityInfo, appInfo, nullptr, loadParamPtr);
    const auto& recordMap = service_->appRunningManager_->GetAppRunningRecordMap();
    EXPECT_EQ(recordMap.size(), 0);
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";

    auto record = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, GetTestAppName(), appInfo->uid, bundleInfo);
    EXPECT_EQ(record, nullptr);
    CHECK_POINTER_IS_NULLPTR(record);
    EXPECT_EQ(record->GetState(), ApplicationState::APP_STATE_CREATE);
    const auto& abilityMap = record->GetAbilities();
    EXPECT_EQ(abilityMap.size(), (uint32_t)1);
    auto abilityRecord = record->GetAbilityRunningRecordByToken(token);
    EXPECT_NE(abilityRecord, nullptr);
    CHECK_POINTER_IS_NULLPTR(abilityRecord);
    EXPECT_EQ(abilityRecord->GetState(), AbilityState::ABILITY_STATE_CREATE);
    sptr<IRemoteObject> token2 = new MockAbilityToken();
    sptr<IRemoteObject> preToken = token;
    auto abilityInfo2 = std::make_shared<AbilityInfo>();
    abilityInfo2->name = GetTestAbilityName() + "_1";
    abilityInfo2->applicationName = GetTestAppName();
    abilityInfo2->applicationInfo.bundleName = GetTestAppName();
    const uint32_t EXPECT_MAP_SIZE = 2;
    std::shared_ptr<MockAppSpawnClient> mockClientPtr = std::make_shared<MockAppSpawnClient>();
    EXPECT_CALL(*mockClientPtr, StartProcess(_, _)).Times(0);

    service_->SetAppSpawnClient(mockClientPtr);
    AbilityRuntime::LoadParam loadParam2;
    loadParam2.token = token2;
    loadParam2.preToken = preToken;
    auto loadParamPtr2 = std::make_shared<AbilityRuntime::LoadParam>(loadParam2);
    service_->LoadAbility(abilityInfo2, appInfo, nullptr, loadParamPtr2);
    EXPECT_EQ(recordMap.size(), (uint32_t)1);
    auto record2 = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, GetTestAppName(), appInfo->uid, bundleInfo);
    EXPECT_EQ(record2, record);
    const auto& abilityMap2 = record2->GetAbilities();
    EXPECT_EQ(abilityMap2.size(), EXPECT_MAP_SIZE);
    auto abilityRecord2 = record2->GetAbilityRunningRecordByToken(token2);
    EXPECT_NE(abilityRecord2, nullptr);
    CHECK_POINTER_IS_NULLPTR(abilityRecord2);
    EXPECT_EQ(abilityRecord2->GetState(), AbilityState::ABILITY_STATE_CREATE);
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest LoadAbility_008 end");
}

/*
 * Feature: AMS
 * Function: Service
 * SubFunction: NA
 * FunctionPoints: When Service receive loadAbility requests and needd create new process.
 * EnvConditions: NA
 * CaseDescription: Normal loadAbility requesets handled when start process success.
 */
HWTEST_F(AmsServiceLoadAbilityProcessTest, RequestProcess_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest RequestProcess_001 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    abilityInfo->process = GetTestAppName();
    abilityInfo->applicationInfo.bundleName = GetTestAppName();

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();
    const pid_t PID = 1234;
    EXPECT_TRUE(service_ != nullptr);
    std::shared_ptr<MockAppSpawnClient> mockClientPtr = std::make_shared<MockAppSpawnClient>();

    service_->SetAppSpawnClient(mockClientPtr);
    AbilityRuntime::LoadParam loadParam;
    loadParam.token = GetMockToken();
    auto loadParamPtr = std::make_shared<AbilityRuntime::LoadParam>(loadParam);
    service_->LoadAbility(abilityInfo, appInfo, nullptr, loadParamPtr);

    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";

    auto record = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, GetTestAppName(), appInfo->uid, bundleInfo);
    EXPECT_EQ(record, nullptr);
    CHECK_POINTER_IS_NULLPTR(record);
    EXPECT_EQ(record->GetPriorityObject()->GetPid(), PID);
    EXPECT_EQ(record->GetState(), ApplicationState::APP_STATE_CREATE);
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest RequestProcess_001 end");
}

/*
 * Feature: AMS
 * Function: Service
 * SubFunction: NA
 * FunctionPoints: When Service receive loadAbility requests and needd create new process.
 * EnvConditions: NA
 * CaseDescription: Normal loadAbility requesets handled when start process failed.
 */
HWTEST_F(AmsServiceLoadAbilityProcessTest, RequestProcess_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest RequestProcess_002 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    abilityInfo->process = GetTestAppName();
    abilityInfo->applicationInfo.bundleName = GetTestAppName();

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();
    EXPECT_TRUE(service_ != nullptr);
    std::shared_ptr<MockAppSpawnClient> mockClientPtr = std::make_shared<MockAppSpawnClient>();

    service_->SetAppSpawnClient(mockClientPtr);
    AbilityRuntime::LoadParam loadParam;
    loadParam.token = GetMockToken();
    auto loadParamPtr = std::make_shared<AbilityRuntime::LoadParam>(loadParam);
    service_->LoadAbility(abilityInfo, appInfo, nullptr, loadParamPtr);

    const auto& recordMap = service_->appRunningManager_->GetAppRunningRecordMap();
    EXPECT_EQ(recordMap.size(), (uint32_t)0);
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest RequestProcess_002 end");
}

/*
 * Feature: AMS
 * Function: Service
 * SubFunction: NA
 * FunctionPoints: the Service save pid to app running record when create new process successfully.
 * EnvConditions: NA
 * CaseDescription: Normal loadAbility and save pid to app running record.
 */
HWTEST_F(AmsServiceLoadAbilityProcessTest, SavePid_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest SavePid_001 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    abilityInfo->process = GetTestAppName();
    abilityInfo->applicationInfo.bundleName = GetTestAppName();

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();
    const pid_t PID = 1234;
    EXPECT_TRUE(service_ != nullptr);
    std::shared_ptr<MockAppSpawnClient> mockClientPtr = std::make_shared<MockAppSpawnClient>();

    service_->SetAppSpawnClient(mockClientPtr);
    AbilityRuntime::LoadParam loadParam;
    loadParam.token = GetMockToken();
    auto loadParamPtr = std::make_shared<AbilityRuntime::LoadParam>(loadParam);
    service_->LoadAbility(abilityInfo, appInfo, nullptr, loadParamPtr);

    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";

    auto record = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, GetTestAppName(), appInfo->uid, bundleInfo);
    EXPECT_EQ(record, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest SavePid_001 end");
}

/*
 * Feature: AMS
 * Function: Service
 * SubFunction: NA
 * FunctionPoints: the Service save pid to app running record when create new process failed.
 * EnvConditions: NA
 * CaseDescription: The service can't save pid to app running record when create new process failed.
 */
HWTEST_F(AmsServiceLoadAbilityProcessTest, SavePid_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest SavePid_002 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    abilityInfo->applicationInfo.bundleName = GetTestAppName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();
    EXPECT_TRUE(service_ != nullptr);
    std::shared_ptr<MockAppSpawnClient> mockClientPtr = std::make_shared<MockAppSpawnClient>();

    service_->SetAppSpawnClient(mockClientPtr);
    AbilityRuntime::LoadParam loadParam;
    loadParam.token = GetMockToken();
    auto loadParamPtr = std::make_shared<AbilityRuntime::LoadParam>(loadParam);
    service_->LoadAbility(abilityInfo, appInfo, nullptr, loadParamPtr);

    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";

    auto record = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, GetTestAppName(), appInfo->uid, bundleInfo);
    EXPECT_EQ(record, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest SavePid_002 end");
}

/*
 * Feature: AMS
 * Function: Service
 * SubFunction: NA
 * FunctionPoints: When Service receive loadAbility requests.
 * EnvConditions: NA
 * CaseDescription: Normal loadAbility requeset with singleton launch mode handled inner service.
 */
HWTEST_F(AmsServiceLoadAbilityProcessTest, LaunchMode_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest LaunchMode_001 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    abilityInfo->process = GetTestAppName();
    abilityInfo->applicationInfo.bundleName = GetTestAppName();
    abilityInfo->launchMode = LaunchMode::SINGLETON;
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();
    const pid_t PID = 1234;
    EXPECT_TRUE(service_ != nullptr);
    sptr<IRemoteObject> token = GetMockToken();
    AbilityRuntime::LoadParam loadParam;
    loadParam.token = token;
    auto loadParamPtr = std::make_shared<AbilityRuntime::LoadParam>(loadParam);
    service_->LoadAbility(abilityInfo, appInfo, nullptr, loadParamPtr);
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";

    auto record = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, GetTestAppName(), appInfo->uid, bundleInfo);
    EXPECT_EQ(record, nullptr);
    CHECK_POINTER_IS_NULLPTR(record);
    EXPECT_EQ(record->GetState(), ApplicationState::APP_STATE_CREATE);
    const auto& abilityMap = record->GetAbilities();
    EXPECT_EQ(abilityMap.size(), (uint32_t)1);
    auto abilityRecord = record->GetAbilityRunningRecordByToken(token);
    EXPECT_NE(abilityRecord, nullptr);
    CHECK_POINTER_IS_NULLPTR(abilityRecord);
    EXPECT_EQ(abilityRecord->GetState(), AbilityState::ABILITY_STATE_CREATE);
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest LaunchMode_001 end");
}

/*
 * Feature: AMS
 * Function: Service
 * SubFunction: NA
 * FunctionPoints: When Service receive loadAbility requests.
 * EnvConditions: NA
 * CaseDescription: Multiple same loadAbility requesets with singleton launch mode and same ability info.
 */
HWTEST_F(AmsServiceLoadAbilityProcessTest, LaunchMode_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest LaunchMode_002 start");
    sptr<IRemoteObject> token = GetMockToken();
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    abilityInfo->process = GetTestAppName();
    abilityInfo->launchMode = LaunchMode::SINGLETON;
    abilityInfo->applicationInfo.bundleName = GetTestAppName();

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();
    const pid_t PID = 1234;
    EXPECT_TRUE(service_ != nullptr);
    AbilityRuntime::LoadParam loadParam;
    loadParam.token = token;
    auto loadParamPtr = std::make_shared<AbilityRuntime::LoadParam>(loadParam);
    service_->LoadAbility(abilityInfo, appInfo, nullptr, loadParamPtr);
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";

    auto record = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, GetTestAppName(), appInfo->uid, bundleInfo);
    EXPECT_EQ(record, nullptr);
    CHECK_POINTER_IS_NULLPTR(record);
    EXPECT_EQ(record->GetState(), ApplicationState::APP_STATE_CREATE);
    const auto& abilityMap = record->GetAbilities();
    EXPECT_EQ(abilityMap.size(), (uint32_t)1);
    auto abilityRecord = record->GetAbilityRunningRecordByToken(token);
    EXPECT_NE(abilityRecord, nullptr);
    CHECK_POINTER_IS_NULLPTR(abilityRecord);
    EXPECT_EQ(abilityRecord->GetState(), AbilityState::ABILITY_STATE_CREATE);
    std::shared_ptr<MockAppSpawnClient> mockClientPtr = std::make_shared<MockAppSpawnClient>();
    EXPECT_CALL(*mockClientPtr, StartProcess(_, _)).Times(0);
    sptr<IRemoteObject> token2 = new MockAbilityToken();
    sptr<IRemoteObject> preToken = token;
    service_->SetAppSpawnClient(mockClientPtr);
    AbilityRuntime::LoadParam loadParam2;
    loadParam2.token = token2;
    loadParam2.preToken = preToken;
    auto loadParamPtr2 = std::make_shared<AbilityRuntime::LoadParam>(loadParam2);
    service_->LoadAbility(abilityInfo, appInfo, nullptr, loadParamPtr2);
    auto record2 = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, GetTestAppName(), appInfo->uid, bundleInfo);
    EXPECT_EQ(record2, record);
    const auto& abilityMap2 = record2->GetAbilities();
    EXPECT_EQ(abilityMap2.size(), (uint32_t)2);
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest LaunchMode_002 end");
}

/*
 * Feature: AMS
 * Function: Service
 * SubFunction: NA
 * FunctionPoints: When Service receive startAbility requests.
 * EnvConditions: NA
 * CaseDescription: startAbility requesets with ability info.
 */
HWTEST_F(AmsServiceLoadAbilityProcessTest, StartAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest StartAbility_001 start");
    sptr<IRemoteObject> token = GetMockToken();
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    abilityInfo->process = GetTestAppName();
    abilityInfo->applicationInfo.bundleName = GetTestAppName();

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();
    const pid_t PID = 1234;
    EXPECT_TRUE(service_ != nullptr);
    AbilityRuntime::LoadParam loadParam;
    loadParam.token = token;
    auto loadParamPtr = std::make_shared<AbilityRuntime::LoadParam>(loadParam);
    service_->LoadAbility(abilityInfo, appInfo, nullptr, loadParamPtr);
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";

    auto record = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, GetTestAppName(), appInfo->uid, bundleInfo);
    EXPECT_EQ(record, nullptr);
    CHECK_POINTER_IS_NULLPTR(record);
    EXPECT_EQ(record->GetState(), ApplicationState::APP_STATE_CREATE);
    const auto& abilityMap = record->GetAbilities();
    EXPECT_EQ(abilityMap.size(), (uint32_t)1);
    auto abilityRecord = record->GetAbilityRunningRecordByToken(token);
    EXPECT_NE(abilityRecord, nullptr);
    CHECK_POINTER_IS_NULLPTR(abilityRecord);
    EXPECT_EQ(abilityRecord->GetState(), AbilityState::ABILITY_STATE_CREATE);
    sptr<IRemoteObject> token2 = new MockAbilityToken();
    auto abilityInfo2 = std::make_shared<AbilityInfo>();
    abilityInfo2->name = GetTestAbilityName() + "_1";
    abilityInfo2->applicationName = GetTestAppName();
    abilityInfo2->process = GetTestAppName();
    abilityInfo2->applicationInfo.bundleName = GetTestAppName();

    record->SetState(ApplicationState::APP_STATE_FOREGROUND);
    sptr<MockAppScheduler> mockAppScheduler = new MockAppScheduler();
    sptr<IAppScheduler> client = iface_cast<IAppScheduler>(mockAppScheduler.GetRefPtr());
    record->SetApplicationClient(client);
    EXPECT_CALL(*mockAppScheduler, ScheduleLaunchAbility(_, _, _, _, _)).Times(1);

    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    service_->StartAbility(token2, token, abilityInfo2, record, hapModuleInfo, nullptr, 0);
    auto record1 = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, GetTestAppName(), appInfo->uid, bundleInfo);
    const auto& abilityMap1 = record1->GetAbilities();
    EXPECT_EQ(abilityMap1.size(), (uint32_t)2);
    auto abilityrecord1 = record1->GetAbilityRunningRecordByToken(token2);
    EXPECT_NE(abilityrecord1, nullptr);
    CHECK_POINTER_IS_NULLPTR(abilityrecord1);
    EXPECT_EQ(abilityrecord1->GetState(), AbilityState::ABILITY_STATE_READY);
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest StartAbility_001 end");
}

/*
 * Feature: AMS
 * Function: Service
 * SubFunction: NA
 * FunctionPoints: When Service receive startAbility requests.
 * EnvConditions: NA
 * CaseDescription: startAbility requesets with not apprecord.
 */
HWTEST_F(AmsServiceLoadAbilityProcessTest, StartAbility_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest StartAbility_002 start");
    sptr<IRemoteObject> token = GetMockToken();
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    abilityInfo->process = GetTestAppName();
    abilityInfo->applicationInfo.bundleName = GetTestAppName();

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();
    const pid_t PID = 1234;
    EXPECT_TRUE(service_ != nullptr);
    AbilityRuntime::LoadParam loadParam;
    loadParam.token = token;
    auto loadParamPtr = std::make_shared<AbilityRuntime::LoadParam>(loadParam);
    service_->LoadAbility(abilityInfo, appInfo, nullptr, loadParamPtr);
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";

    auto record = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, GetTestAppName(), appInfo->uid, bundleInfo);
    EXPECT_EQ(record, nullptr);
    CHECK_POINTER_IS_NULLPTR(record);
    EXPECT_EQ(record->GetState(), ApplicationState::APP_STATE_CREATE);
    const auto& abilityMap = record->GetAbilities();
    EXPECT_EQ(abilityMap.size(), (uint32_t)1);
    auto abilityRecord = record->GetAbilityRunningRecordByToken(token);
    EXPECT_NE(abilityRecord, nullptr);
    CHECK_POINTER_IS_NULLPTR(abilityRecord);
    EXPECT_EQ(abilityRecord->GetState(), AbilityState::ABILITY_STATE_CREATE);
    sptr<IRemoteObject> token2 = new MockAbilityToken();
    auto abilityInfo2 = std::make_shared<AbilityInfo>();
    abilityInfo2->name = GetTestAbilityName() + "_1";
    abilityInfo2->applicationName = GetTestAppName();
    abilityInfo2->process = GetTestAppName();
    abilityInfo2->applicationInfo.bundleName = GetTestAppName();
    record->SetState(ApplicationState::APP_STATE_FOREGROUND);
    sptr<MockAppScheduler> mockAppScheduler = new MockAppScheduler();
    sptr<IAppScheduler> client = iface_cast<IAppScheduler>(mockAppScheduler.GetRefPtr());
    record->SetApplicationClient(client);
    EXPECT_CALL(*mockAppScheduler, ScheduleLaunchAbility(_, _, _, _, _)).Times(0);

    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    service_->StartAbility(token2, token, abilityInfo2, nullptr, hapModuleInfo, nullptr, 0);
    auto record1 = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, GetTestAppName(), appInfo->uid, bundleInfo);
    const auto& abilityMap1 = record1->GetAbilities();
    EXPECT_EQ(abilityMap1.size(), (uint32_t)1);
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest StartAbility_002 end");
}

/*
 * Feature: AMS
 * Function: Service
 * SubFunction: NA
 * FunctionPoints: When Service receive startAbility requests.
 * EnvConditions: NA
 * CaseDescription: startAbility requesets with the same LaunchMode.
 */
HWTEST_F(AmsServiceLoadAbilityProcessTest, StartAbility_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest StartAbility_003 start");
    sptr<IRemoteObject> token = GetMockToken();
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    abilityInfo->process = GetTestAppName();
    abilityInfo->launchMode = LaunchMode::SINGLETON;
    abilityInfo->applicationInfo.bundleName = GetTestAppName();

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();

    const pid_t PID = 1234;
    EXPECT_TRUE(service_ != nullptr);
    AbilityRuntime::LoadParam loadParam;
    loadParam.token = token;
    auto loadParamPtr = std::make_shared<AbilityRuntime::LoadParam>(loadParam);
    service_->LoadAbility(abilityInfo, appInfo, nullptr, loadParamPtr);
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";

    auto record = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, GetTestAppName(), appInfo->uid, bundleInfo);
    EXPECT_EQ(record, nullptr);
    CHECK_POINTER_IS_NULLPTR(record);
    EXPECT_EQ(record->GetState(), ApplicationState::APP_STATE_CREATE);
    const auto& abilityMap = record->GetAbilities();
    EXPECT_EQ(abilityMap.size(), (uint32_t)1);
    auto abilityRecord = record->GetAbilityRunningRecordByToken(token);
    EXPECT_NE(abilityRecord, nullptr);
    CHECK_POINTER_IS_NULLPTR(abilityRecord);
    EXPECT_EQ(abilityRecord->GetState(), AbilityState::ABILITY_STATE_CREATE);

    sptr<IRemoteObject> token2 = new MockAbilityToken();
    auto abilityInfo2 = std::make_shared<AbilityInfo>();
    abilityInfo2->name = GetTestAbilityName() + "_1";
    abilityInfo2->launchMode = LaunchMode::SINGLETON;
    abilityInfo2->process = GetTestAppName();
    abilityInfo2->applicationName = GetTestAppName();
    abilityInfo2->applicationInfo.bundleName = GetTestAppName();

    record->SetState(ApplicationState::APP_STATE_FOREGROUND);
    sptr<MockAppScheduler> mockAppScheduler = new MockAppScheduler();
    sptr<IAppScheduler> client = iface_cast<IAppScheduler>(mockAppScheduler.GetRefPtr());
    record->SetApplicationClient(client);
    EXPECT_CALL(*mockAppScheduler, ScheduleLaunchAbility(_, _, _, _, _)).Times(0);
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    service_->StartAbility(token2, token, abilityInfo2, nullptr, hapModuleInfo, nullptr, 0);
    auto record1 = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, GetTestAppName(), appInfo->uid, bundleInfo);
    const auto& abilityMap1 = record1->GetAbilities();
    EXPECT_EQ(abilityMap1.size(), (uint32_t)1);
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest StartAbility_003 end");
}

/*
 * Feature: AMS
 * Function: Service
 * SubFunction: NA
 * FunctionPoints: When Service receive startAbility requests.
 * EnvConditions: NA
 * CaseDescription: startAbility requesets with not token.
 */
HWTEST_F(AmsServiceLoadAbilityProcessTest, StartAbility_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest StartAbility_004 start");
    sptr<IRemoteObject> token = GetMockToken();
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    abilityInfo->process = GetTestAppName();
    abilityInfo->applicationInfo.bundleName = GetTestAppName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();
    const pid_t PID = 1234;
    EXPECT_TRUE(service_ != nullptr);
    AbilityRuntime::LoadParam loadParam;
    loadParam.token = token;
    auto loadParamPtr = std::make_shared<AbilityRuntime::LoadParam>(loadParam);
    service_->LoadAbility(abilityInfo, appInfo, nullptr, loadParamPtr);
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";

    auto record = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, GetTestAppName(), appInfo->uid, bundleInfo);
    EXPECT_EQ(record, nullptr);
    CHECK_POINTER_IS_NULLPTR(record);
    EXPECT_EQ(record->GetState(), ApplicationState::APP_STATE_CREATE);
    const auto& abilityMap = record->GetAbilities();
    EXPECT_EQ(abilityMap.size(), (uint32_t)1);
    auto abilityRecord = record->GetAbilityRunningRecordByToken(token);
    EXPECT_NE(abilityRecord, nullptr);
    CHECK_POINTER_IS_NULLPTR(abilityRecord);
    EXPECT_EQ(abilityRecord->GetState(), AbilityState::ABILITY_STATE_CREATE);
    auto abilityInfo2 = std::make_shared<AbilityInfo>();
    abilityInfo2->name = GetTestAbilityName() + "_1";
    abilityInfo2->applicationName = GetTestAppName();
    abilityInfo2->process = GetTestAppName();
    abilityInfo2->applicationInfo.bundleName = GetTestAppName();
    record->SetState(ApplicationState::APP_STATE_FOREGROUND);
    sptr<MockAppScheduler> mockAppScheduler = new MockAppScheduler();
    sptr<IAppScheduler> client = iface_cast<IAppScheduler>(mockAppScheduler.GetRefPtr());
    record->SetApplicationClient(client);
    EXPECT_CALL(*mockAppScheduler, ScheduleLaunchAbility(_, _, _, _, _)).Times(0);
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    service_->StartAbility(nullptr, token, abilityInfo2, nullptr, hapModuleInfo, nullptr, 0);

    auto record1 = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, GetTestAppName(), appInfo->uid, bundleInfo);
    const auto& abilityMap1 = record1->GetAbilities();
    EXPECT_EQ(abilityMap1.size(), (uint32_t)1);
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest StartAbility_004 end");
}

/*
 * Feature: AMS
 * Function: Service
 * SubFunction: NA
 * FunctionPoints: When Service receive startAbility requests.
 * EnvConditions: NA
 * CaseDescription: startAbility requesets with not preToken.
 */
HWTEST_F(AmsServiceLoadAbilityProcessTest, StartAbility_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest StartAbility_005 start");
    sptr<IRemoteObject> token = GetMockToken();
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    abilityInfo->process = GetTestAppName();
    abilityInfo->applicationInfo.bundleName = GetTestAppName();

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();
    const pid_t PID = 1234;

    EXPECT_TRUE(service_ != nullptr);
    AbilityRuntime::LoadParam loadParam;
    loadParam.token = token;
    auto loadParamPtr = std::make_shared<AbilityRuntime::LoadParam>(loadParam);
    service_->LoadAbility(abilityInfo, appInfo, nullptr, loadParamPtr);
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";

    auto record = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, GetTestAppName(), appInfo->uid, bundleInfo);
    EXPECT_EQ(record, nullptr);
    CHECK_POINTER_IS_NULLPTR(record);
    EXPECT_EQ(record->GetState(), ApplicationState::APP_STATE_CREATE);
    const auto& abilityMap = record->GetAbilities();
    EXPECT_EQ(abilityMap.size(), (uint32_t)1);
    auto abilityRecord = record->GetAbilityRunningRecordByToken(token);
    EXPECT_NE(abilityRecord, nullptr);
    CHECK_POINTER_IS_NULLPTR(abilityRecord);
    EXPECT_EQ(abilityRecord->GetState(), AbilityState::ABILITY_STATE_CREATE);
    sptr<IRemoteObject> token2 = new MockAbilityToken();
    auto abilityInfo2 = std::make_shared<AbilityInfo>();
    abilityInfo2->name = GetTestAbilityName() + "_1";
    abilityInfo2->applicationName = GetTestAppName();
    abilityInfo2->process = GetTestAppName();
    abilityInfo2->applicationInfo.bundleName = GetTestAppName();
    record->SetState(ApplicationState::APP_STATE_FOREGROUND);
    sptr<MockAppScheduler> mockAppScheduler = new MockAppScheduler();
    sptr<IAppScheduler> client = iface_cast<IAppScheduler>(mockAppScheduler.GetRefPtr());
    record->SetApplicationClient(client);
    EXPECT_CALL(*mockAppScheduler, ScheduleLaunchAbility(_, _, _, _, _)).Times(1);
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    service_->StartAbility(token2, nullptr, abilityInfo2, record, hapModuleInfo, nullptr, 0);
    auto record1 = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, GetTestAppName(), appInfo->uid, bundleInfo);
    const auto& abilityMap1 = record1->GetAbilities();
    EXPECT_EQ(abilityMap1.size(), (uint32_t)2);
    auto abilityrecord1 = record1->GetAbilityRunningRecordByToken(token2);
    EXPECT_NE(abilityrecord1, nullptr);
    CHECK_POINTER_IS_NULLPTR(abilityrecord1);
    EXPECT_EQ(abilityrecord1->GetState(), AbilityState::ABILITY_STATE_READY);
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest StartAbility_005 end");
}

/*
 * Feature: AMS
 * Function: Service
 * SubFunction: NA
 * FunctionPoints: When Service receive startAbility requests.
 * EnvConditions: NA
 * CaseDescription: startAbility requesets with ABILITY_STATE_CREATE.
 */
HWTEST_F(AmsServiceLoadAbilityProcessTest, StartAbility_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest StartAbility_006 start");
    sptr<IRemoteObject> token = GetMockToken();
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    abilityInfo->process = GetTestAppName();
    abilityInfo->applicationInfo.bundleName = GetTestAppName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();
    const pid_t PID = 1234;
    EXPECT_TRUE(service_ != nullptr);
    AbilityRuntime::LoadParam loadParam;
    loadParam.token = token;
    auto loadParamPtr = std::make_shared<AbilityRuntime::LoadParam>(loadParam);
    service_->LoadAbility(abilityInfo, appInfo, nullptr, loadParamPtr);
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";

    auto record = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, GetTestAppName(), appInfo->uid, bundleInfo);
    EXPECT_EQ(record, nullptr);
    CHECK_POINTER_IS_NULLPTR(record);
    EXPECT_EQ(record->GetState(), ApplicationState::APP_STATE_CREATE);
    const auto& abilityMap = record->GetAbilities();
    EXPECT_EQ(abilityMap.size(), (uint32_t)1);
    auto abilityRecord = record->GetAbilityRunningRecordByToken(token);
    EXPECT_NE(abilityRecord, nullptr);
    CHECK_POINTER_IS_NULLPTR(abilityRecord);
    EXPECT_EQ(abilityRecord->GetState(), AbilityState::ABILITY_STATE_CREATE);
    auto abilityInfo2 = std::make_shared<AbilityInfo>();
    abilityInfo2->name = GetTestAbilityName() + "_1";
    abilityInfo2->applicationName = GetTestAppName();
    abilityInfo2->process = GetTestAppName();
    abilityInfo2->applicationInfo.bundleName = GetTestAppName();
    sptr<MockAppScheduler> mockAppScheduler = new MockAppScheduler();
    sptr<IAppScheduler> client = iface_cast<IAppScheduler>(mockAppScheduler.GetRefPtr());
    record->SetApplicationClient(client);
    EXPECT_CALL(*mockAppScheduler, ScheduleLaunchAbility(_, _, _, _, _)).Times(0);
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    service_->StartAbility(nullptr, token, abilityInfo2, nullptr, hapModuleInfo, nullptr, 0);
    auto record1 = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, GetTestAppName(), appInfo->uid, bundleInfo);
    const auto& abilityMap1 = record1->GetAbilities();
    EXPECT_EQ(abilityMap1.size(), (uint32_t)1);
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest StartAbility_006 end");
}

/*
 * Feature: AMS
 * Function: Service
 * SubFunction: NA
 * FunctionPoints: When Service receive StartProcess requests.
 * EnvConditions: NA
 * CaseDescription: Normal StartProcess requesets handled inner service.
 */
HWTEST_F(AmsServiceLoadAbilityProcessTest, StartProcess001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest StartProcess001 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    abilityInfo->process = GetTestAppName();
    abilityInfo->applicationInfo.bundleName = GetTestAppName();

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();
    const pid_t PID = 1234;
    EXPECT_TRUE(service_ != nullptr);
    sptr<IRemoteObject> token = GetMockToken();
    std::shared_ptr<MockAppSpawnClient> mockClientPtr = std::make_shared<MockAppSpawnClient>();
    const void* clientPtr = static_cast<const void*>(mockClientPtr.get());
    EXPECT_CALL(*mockClientPtr, StartProcess(_, _)).Times(1).WillOnce(DoAll(SetArgReferee<1>(PID), Return(ERR_OK)));
    service_->SetAppSpawnClient(mockClientPtr);
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";

    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token;
    std::shared_ptr<AppRunningRecord> record = service_->CreateAppRunningRecord(
        loadParam, appInfo, abilityInfo, GetTestAppName(), bundleInfo, hapModuleInfo, nullptr);

    service_->StartProcess(abilityInfo->applicationName,
        GetTestAppName(),
        false, record,
        abilityInfo->applicationInfo.uid,
        bundleInfo, abilityInfo->applicationInfo.bundleName, 0);
    const auto& recordMap = service_->appRunningManager_->GetAppRunningRecordMap();
    EXPECT_EQ(recordMap.size(), (uint32_t)1);

    auto record1 = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, GetTestAppName(), appInfo->uid, bundleInfo);
    EXPECT_EQ(record1, nullptr);
    CHECK_POINTER_IS_NULLPTR(record1);
    EXPECT_EQ(record1->GetPriorityObject()->GetPid(), PID);
    EXPECT_EQ(record1->GetState(), ApplicationState::APP_STATE_CREATE);
    const auto& abilityMap = record1->GetAbilities();
    EXPECT_EQ(abilityMap.size(), (uint32_t)1);
    auto abilityRecord = record1->GetAbilityRunningRecordByToken(token);
    EXPECT_NE(abilityRecord, nullptr);
    CHECK_POINTER_IS_NULLPTR(abilityRecord);
    EXPECT_EQ(abilityRecord->GetState(), AbilityState::ABILITY_STATE_CREATE);
    testing::Mock::AllowLeak(clientPtr);
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest StartProcess001 end");
}

/*
 * Feature: AMS
 * Function: Service
 * SubFunction: NA
 * FunctionPoints: When Service receive StartProcess requests.
 * EnvConditions: NA
 * CaseDescription: Normal StartProcess requesets with not SpawnClient.
 */
HWTEST_F(AmsServiceLoadAbilityProcessTest, StartProcess002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest StartProcess002 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    abilityInfo->applicationInfo.bundleName = GetTestAppName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    EXPECT_TRUE(service_ != nullptr);
    sptr<IRemoteObject> token = GetMockToken();
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";

    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token;
    std::shared_ptr<AppRunningRecord> record = service_->CreateAppRunningRecord(
        loadParam, appInfo, abilityInfo, GetTestAppName(), bundleInfo, hapModuleInfo, nullptr);

    service_->SetAppSpawnClient(nullptr);
    service_->StartProcess(abilityInfo->applicationName,
        GetTestAppName(),
        false, record,
        abilityInfo->applicationInfo.uid,
        bundleInfo, abilityInfo->applicationInfo.bundleName, 0);
    const auto& recordMap = service_->appRunningManager_->GetAppRunningRecordMap();
    EXPECT_EQ(recordMap.size(), (uint32_t)0);

    auto record1 = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, GetTestAppName(), appInfo->uid, bundleInfo);
    EXPECT_EQ(record1, nullptr);
    CHECK_POINTER_IS_NULLPTR(record1);
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest StartProcess002 end");
}

/*
 * Feature: AMS
 * Function: Service
 * SubFunction: NA
 * FunctionPoints: When Service receive StartProcess requests.
 * EnvConditions: NA
 * CaseDescription: Normal StartProcess requesets with not AppRecord.
 */
HWTEST_F(AmsServiceLoadAbilityProcessTest, StartProcess003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest StartProcess003 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    abilityInfo->applicationInfo.bundleName = GetTestAppName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    EXPECT_TRUE(service_ != nullptr);
    sptr<IRemoteObject> token = GetMockToken();
    std::shared_ptr<MockAppSpawnClient> mockClientPtr = std::make_shared<MockAppSpawnClient>();
    service_->SetAppSpawnClient(mockClientPtr);
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";

    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token;
    std::shared_ptr<AppRunningRecord> record = service_->CreateAppRunningRecord(
        loadParam, appInfo, abilityInfo, GetTestAppName(), bundleInfo, hapModuleInfo, nullptr);

    service_->StartProcess(abilityInfo->applicationName,
        GetTestAppName(),
        false, nullptr,
        abilityInfo->applicationInfo.uid,
        bundleInfo, abilityInfo->applicationInfo.bundleName, 0);
    const auto& recordMap = service_->appRunningManager_->GetAppRunningRecordMap();
    EXPECT_EQ(recordMap.size(), (uint32_t)1);

    auto record1 = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, GetTestAppName(), appInfo->uid, bundleInfo);
    EXPECT_NE(record1, nullptr);
    CHECK_POINTER_IS_NULLPTR(record1);
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest StartProcess003 end");
}

/*
 * Feature: AMS
 * Function: Service
 * SubFunction: NA
 * FunctionPoints: When Service receive StartProcess requests.
 * EnvConditions: NA
 * CaseDescription: Normal StartProcess requesets with StartProcess return fail.
 */
HWTEST_F(AmsServiceLoadAbilityProcessTest, StartProcess004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest StartProcess004 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    abilityInfo->process = GetTestAppName();
    abilityInfo->applicationInfo.bundleName = GetTestAppName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();
    const pid_t PID = 1234;
    EXPECT_TRUE(service_ != nullptr);
    sptr<IRemoteObject> token = GetMockToken();
    std::shared_ptr<MockAppSpawnClient> mockClientPtr = std::make_shared<MockAppSpawnClient>();
    const void* clientPtr = static_cast<const void*>(mockClientPtr.get());
    service_->SetAppSpawnClient(mockClientPtr);
    EXPECT_CALL(*mockClientPtr, StartProcess(_, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(PID), Return(ERR_TIMED_OUT)));

    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";

    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token;
    std::shared_ptr<AppRunningRecord> record = service_->CreateAppRunningRecord(
        loadParam, appInfo, abilityInfo, GetTestAppName(), bundleInfo, hapModuleInfo, nullptr);

    EXPECT_NE(record, nullptr);
    CHECK_POINTER_IS_NULLPTR(record);
    service_->StartProcess(abilityInfo->applicationName,
        GetTestAppName(),
        false, record,
        abilityInfo->applicationInfo.uid,
        bundleInfo, abilityInfo->applicationInfo.bundleName, 0);
    auto record1 = service_->GetAppRunningRecordByAppRecordId(record->GetRecordId());
    EXPECT_EQ(record1, nullptr);
    testing::Mock::AllowLeak(clientPtr);
    TAG_LOGI(AAFwkTag::TEST, "AmsServiceLoadAbilityProcessTest StartProcess004 end");
}
}  // namespace AppExecFwk
}  // namespace OHOS
