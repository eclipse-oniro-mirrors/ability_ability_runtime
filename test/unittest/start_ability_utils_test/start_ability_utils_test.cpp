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
#define protected public
#include "ability_record.h"
#undef private
#undef protected

#include "ability_manager_errors.h"
#include "accesstoken_kit.h"
#include "extension_ability_info.h"
#include "hilog_tag_wrapper.h"
#include "mock_my_flag.h"
#include "mock_my_status.h"
#include "start_ability_utils.h"
#include "want.h"

using namespace testing::ext;
namespace OHOS {
namespace AAFwk {

class StartAbilityUtilsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void StartAbilityUtilsTest::SetUpTestCase()
{}

void StartAbilityUtilsTest::TearDownTestCase()
{}

void StartAbilityUtilsTest::SetUp()
{}

void StartAbilityUtilsTest::TearDown()
{}

/**
 * @tc.name: GetApplicationInfo_001
 * @tc.desc: test class StartAbilityUtil GetApplicationInfo function
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, GetApplicationInfo_001, TestSize.Level1)
{
    std::string bundleName = "";
    int32_t userId = 0;
    AppExecFwk::ApplicationInfo appInfo;
    bool ret = StartAbilityUtils::GetApplicationInfo(bundleName, userId, appInfo);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: GetApplicationInfo_002
 * @tc.desc: test class StartAbilityUtil GetApplicationInfo function
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, GetApplicationInfo_002, TestSize.Level1)
{
    std::string bundleName = "test";
    int32_t userId = 0;
    AppExecFwk::ApplicationInfo appInfo;
    bool ret = StartAbilityUtils::GetApplicationInfo(bundleName, userId, appInfo);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: GetApplicationInfo_003
 * @tc.desc: test class StartAbilityUtil GetApplicationInfo function
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, GetApplicationInfo_003, TestSize.Level1)
{
    std::string bundleName = "test";
    int32_t userId = 0;
    AppExecFwk::ApplicationInfo appInfo;
    AAFwk::MyStatus::GetInstance().retValue_ = true;
    bool ret = StartAbilityUtils::GetApplicationInfo(bundleName, userId, appInfo);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: CheckAppProvisionMode_004
 * @tc.desc: test class StartAbilityUtil CheckAppProvisionMode function
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, CheckAppProvisionMode_004, TestSize.Level1)
{
    Want want;
    int32_t userId = 1001;
    StartAbilityUtils::startAbilityInfo = std::make_shared<StartAbilityInfo>();
    int32_t ret = StartAbilityUtils::CheckAppProvisionMode(want, userId, nullptr);
    EXPECT_EQ(ret, ERR_NOT_IN_APP_PROVISION_MODE);
}

/**
 * @tc.name: CheckAppProvisionMode_005
 * @tc.desc: test class StartAbilityUtil CheckAppProvisionMode function
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, CheckAppProvisionMode_005, TestSize.Level1)
{
    Want want;
    int32_t userId = 1001;
    StartAbilityUtils::startAbilityInfo = std::make_shared<StartAbilityInfo>();
    StartAbilityUtils::startAbilityInfo->abilityInfo.bundleName = "bundleName";
    StartAbilityUtils::startAbilityInfo->abilityInfo.name = "name";
    StartAbilityUtils::startAbilityInfo->status = 1;
    int32_t ret = StartAbilityUtils::CheckAppProvisionMode(want, userId, nullptr);
    EXPECT_EQ(ret, StartAbilityUtils::startAbilityInfo->status);
}

/**
 * @tc.name: CheckAppProvisionMode_006
 * @tc.desc: test class StartAbilityUtil CheckAppProvisionMode function
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, CheckAppProvisionMode_006, TestSize.Level1)
{
    std::string bundleName = "testName";
    Want want;
    want.SetBundle(bundleName);
    int32_t userId = 0;
    StartAbilityUtils::startAbilityInfo = std::make_shared<StartAbilityInfo>();
    AppExecFwk::ApplicationInfo applicationInfo;
    applicationInfo.bundleName = bundleName;
    StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo = applicationInfo;
    auto ret = StartAbilityUtils::CheckAppProvisionMode(want, userId, nullptr);
    EXPECT_EQ(ret, ERR_NOT_IN_APP_PROVISION_MODE);
}

/**
 * @tc.name: CheckAppProvisionMode_007
 * @tc.desc: test class StartAbilityUtil CheckAppProvisionMode function
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, CheckAppProvisionMode_007, TestSize.Level1)
{
    std::string bundleName = "testName";
    Want want;
    want.SetParam("ohos.dlp.params.index", 5);
    int32_t userId = 0;
    StartAbilityUtils::startAbilityInfo = std::make_shared<StartAbilityInfo>();
    AppExecFwk::ApplicationInfo applicationInfo;
    applicationInfo.bundleName = bundleName;
    StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo = applicationInfo;
    auto ret = StartAbilityUtils::CheckAppProvisionMode(want, userId, nullptr);
    EXPECT_EQ(ret, ERR_APP_CLONE_INDEX_INVALID);
}

/**
 * @tc.name: CheckAppProvisionMode_008
 * @tc.desc: test class StartAbilityUtil CheckAppProvisionMode function
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, CheckAppProvisionMode_008, TestSize.Level1)
{
    std::string bundleName = "testName";
    Want want;
    want.SetBundle(bundleName);
    int32_t userId = 0;
    StartAbilityUtils::startAbilityInfo = std::make_shared<StartAbilityInfo>();
    AppExecFwk::ApplicationInfo applicationInfo;
    applicationInfo.bundleName = bundleName;
    applicationInfo.appProvisionType = AppExecFwk::Constants::APP_PROVISION_TYPE_DEBUG;
    StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo = applicationInfo;
    auto ret = StartAbilityUtils::CheckAppProvisionMode(want, userId, nullptr);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: CheckAppProvisionMode_009
 * @tc.desc: test class StartAbilityUtil CheckAppProvisionMode function
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, CheckAppProvisionMode_009, TestSize.Level1)
{
    std::string bundleName = "testName";
    Want want;
    int32_t dlpIndex = 1001;
    want.SetParam("ohos.dlp.params.index", dlpIndex);
    want.SetParam(AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY, 0);
    want.SetParam(AAFwk::Want::DESTINATION_PLUGIN_ABILITY, true);
    int32_t userId = 0;
    StartAbilityUtils::startAbilityInfo = std::make_shared<StartAbilityInfo>();
    AppExecFwk::ApplicationInfo applicationInfo;
    applicationInfo.bundleName = bundleName;
    StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo = applicationInfo;
    auto ret = StartAbilityUtils::CheckAppProvisionMode(want, userId, nullptr);
    EXPECT_EQ(ret, RESOLVE_ABILITY_ERR);
}

/**
 * @tc.name: CreateStartExtensionInfo_001
 * @tc.desc: test class StartAbilityUtil CreateStartExtensionInfo function
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, CreateStartExtensionInfo_001, TestSize.Level1)
{
    Want want;
    int32_t userId = 0;
    int32_t appIndex = 1;
    auto ret = StartAbilityInfo::CreateStartExtensionInfo(want, userId, appIndex);
    ASSERT_NE(ret, nullptr);
}

/**
 * @tc.name: CreateStartExtensionInfo_002
 * @tc.desc: test class StartAbilityUtil CreateStartExtensionInfo function
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, CreateStartExtensionInfo_002, TestSize.Level1)
{
    Want want;
    int32_t userId = 0;
    int32_t appIndex = 10000;
    auto ret = StartAbilityInfo::CreateStartExtensionInfo(want, userId, appIndex);
    ASSERT_NE(ret, nullptr);
}

/**
 * @tc.name: CreateStartExtensionInfo_003
 * @tc.desc: test class StartAbilityUtil CreateStartExtensionInfo function
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, CreateStartExtensionInfo_003, TestSize.Level1)
{
    Want want;
    int32_t userId = 0;
    int32_t appIndex = 0;
    int32_t validUserId = 0;
    StartAbilityUtils::startAbilityInfo = std::make_shared<StartAbilityInfo>();
    StartAbilityInfoWrap threadLocalInfo(want, validUserId, appIndex, nullptr);
    auto ret = StartAbilityInfo::CreateStartExtensionInfo(want, userId, appIndex);
    ASSERT_NE(ret, nullptr);
}

/**
 * @tc.name: CreateStartExtensionInfo_004
 * @tc.desc: test class StartAbilityUtil CreateStartExtensionInfo function
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, CreateStartExtensionInfo_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest CreateStartExtensionInfo_004 start");
    Want want;
    want.SetParam(AAFwk::Want::DESTINATION_PLUGIN_ABILITY, true);
    int32_t userId = 100;
    int32_t appIndex = 0;
    std::string hostBundleName = "com.example.unittest";
    auto abilityInfo = StartAbilityInfo::CreateStartExtensionInfo(want, userId, appIndex, hostBundleName);
    EXPECT_EQ(abilityInfo->status, RESOLVE_ABILITY_ERR);
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest CreateStartExtensionInfo_004 end");
}

/**
 * @tc.name: CreateStartExtensionInfo_005
 * @tc.desc: test class StartAbilityUtil CreateStartExtensionInfo function
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, CreateStartExtensionInfo_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest CreateStartExtensionInfo_005 start");
    Want want;
    want.SetParam(AAFwk::Want::DESTINATION_PLUGIN_ABILITY, false);
    int32_t userId = 100;
    int32_t appIndex = 0;
    std::string hostBundleName = "com.example.unittest";
    auto abilityInfo = StartAbilityInfo::CreateStartExtensionInfo(want, userId, appIndex, hostBundleName);
    EXPECT_EQ(abilityInfo->status, RESOLVE_ABILITY_ERR);
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest CreateStartExtensionInfo_005 end");
}

/**
 * @tc.name: CreateStartExtensionInfo_006
 * @tc.desc: test class StartAbilityUtil CreateStartExtensionInfo function
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, CreateStartExtensionInfo_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest CreateStartExtensionInfo_006 start");
    Want want;
    want.SetParam(AAFwk::Want::DESTINATION_PLUGIN_ABILITY, true);
    int32_t userId = 100;
    int32_t appIndex = 1;
    std::string hostBundleName = "com.example.unittest";
    auto abilityInfo = StartAbilityInfo::CreateStartExtensionInfo(want, userId, appIndex, hostBundleName);
    EXPECT_EQ(abilityInfo->status, RESOLVE_ABILITY_ERR);
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest CreateStartExtensionInfo_006 end");
}

/**
 * @tc.name: CreateStartExtensionInfo_007
 * @tc.desc: test class StartAbilityUtil CreateStartExtensionInfo function
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, CreateStartExtensionInfo_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest CreateStartExtensionInfo_007 start");
    Want want;
    want.SetParam(AAFwk::Want::DESTINATION_PLUGIN_ABILITY, false);
    int32_t userId = 100;
    int32_t appIndex = 1;
    std::string hostBundleName = "com.example.unittest";
    auto abilityInfo = StartAbilityInfo::CreateStartExtensionInfo(want, userId, appIndex, hostBundleName);
    EXPECT_EQ(abilityInfo->status, RESOLVE_ABILITY_ERR);
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest CreateStartExtensionInfo_007 end");
}

/**
 * @tc.name: SetStartAbilityInfo_001
 * @tc.desc: test class StartAbilityUtil SetStartAbilityInfo_ function
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, SetStartAbilityInfo_001, TestSize.Level1)
{
    StartAbilityUtils::startAbilityInfo = nullptr;
    AppExecFwk::AbilityInfo abilityInfo;
    StartAbilityInfoWrap startAbilityInfoWrap;
    startAbilityInfoWrap.SetStartAbilityInfo(abilityInfo);
    ASSERT_NE(StartAbilityUtils::startAbilityInfo, nullptr);

    StartAbilityUtils::startAbilityInfo = std::make_shared<StartAbilityInfo>();
    abilityInfo.name = "abilityName";
    startAbilityInfoWrap.SetStartAbilityInfo(abilityInfo);
    auto abilityName = StartAbilityUtils::startAbilityInfo->abilityInfo.name;
    EXPECT_NE(abilityName, abilityInfo.name);
}

/**
 * @tc.name: StartAbilityInfoWrap_001
 * @tc.desc: test class StartAbilityUtil StartAbilityInfoWrap constructor
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, StartAbilityInfoWrap_001, TestSize.Level1)
{
    Want want;
    int32_t userId = 0;
    int32_t appIndex = 0;
    int32_t validUserId = 0;
    StartAbilityUtils::startSpecifiedBySCB = true;
    StartAbilityInfoWrap threadLocalInfo(want, validUserId, appIndex, nullptr);
    EXPECT_TRUE(StartAbilityUtils::isWantWithAppCloneIndex);
    StartAbilityUtils::startSpecifiedBySCB = false;
}

/**
 * @tc.name: StartAbilityInfoWrap_002
 * @tc.desc: test class StartAbilityUtil StartAbilityInfoWrap constructor
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, StartAbilityInfoWrap_002, TestSize.Level1)
{
    Want want;
    int32_t userId = 0;
    int32_t appIndex = 0;
    int32_t validUserId = 0;
    StartAbilityUtils::startSpecifiedBySCB = false;
    StartAbilityInfoWrap threadLocalInfo(want, validUserId, appIndex, nullptr);
    EXPECT_FALSE(StartAbilityUtils::isWantWithAppCloneIndex);
}

/**
 * @tc.name: CreateStartAbilityInfo_001
 * @tc.desc: test class StartAbilityUtil CreateStartAbilityInfo function
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, CreateStartAbilityInfo_001, TestSize.Level1)
{
    Want want;
    int32_t userId = 0;
    int32_t appIndex = 0;
    auto ret = StartAbilityInfo::CreateStartAbilityInfo(want, userId, appIndex, nullptr);
    ASSERT_NE(ret, nullptr);
}


/**
 * @tc.name: SetTargetCloneIndexInSameBundle_001
 * @tc.desc: test class StartAbilityUtil SetTargetCloneIndexInSameBundle caller nullptr
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, SetTargetCloneIndexInSameBundle_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest SetTargetCloneIndexInSameBundle_001 start");
    Want want;
    want.SetParam(Want::PARAM_APP_CLONE_INDEX_KEY, 0);
    sptr<IRemoteObject> callerToken = nullptr;

    StartAbilityUtils::SetTargetCloneIndexInSameBundle(want, callerToken);
    int32_t appCloneIndex = want.GetIntParam(Want::PARAM_APP_CLONE_INDEX_KEY, -1);
    EXPECT_EQ(appCloneIndex, 0);

    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest SetTargetCloneIndexInSameBundle_001 end");
}

/**
 * @tc.name: SetTargetCloneIndexInSameBundle_002
 * @tc.desc: test class StartAbilityUtil SetTargetCloneIndexInSameBundle bundleName not equal
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, SetTargetCloneIndexInSameBundle_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest SetTargetCloneIndexInSameBundle_002 start");
    Want want;
    want.SetElementName("com.ohos.test", "MainAbility");
    AbilityRequest abilityRequest;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    abilityRecord->abilityInfo_.bundleName = "com.ohos.test1";
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();

    StartAbilityUtils::SetTargetCloneIndexInSameBundle(want, callerToken);
    int32_t appCloneIndex = want.GetIntParam(Want::PARAM_APP_CLONE_INDEX_KEY, -1);
    EXPECT_EQ(appCloneIndex, -1);

    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest SetTargetCloneIndexInSameBundle_002 end");
}

/**
 * @tc.name: SetTargetCloneIndexInSameBundle_003
 * @tc.desc: test class StartAbilityUtil SetTargetCloneIndexInSameBundle has key
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, SetTargetCloneIndexInSameBundle_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest SetTargetCloneIndexInSameBundle_003 start");
    Want want;
    want.SetElementName("com.ohos.test", "MainAbility");
    want.SetParam(Want::PARAM_APP_CLONE_INDEX_KEY, 0);
    AbilityRequest abilityRequest;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    abilityRecord->abilityInfo_.bundleName = "com.ohos.test";
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();

    StartAbilityUtils::SetTargetCloneIndexInSameBundle(want, callerToken);
    int32_t appCloneIndex = want.GetIntParam(Want::PARAM_APP_CLONE_INDEX_KEY, -1);
    EXPECT_EQ(appCloneIndex, 0);

    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest SetTargetCloneIndexInSameBundle_003 end");
}

/**
 * @tc.name: SetTargetCloneIndexInSameBundle_004
 * @tc.desc: test class StartAbilityUtil SetTargetCloneIndexInSameBundle caller index valid
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, SetTargetCloneIndexInSameBundle_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest SetTargetCloneIndexInSameBundle_004 start");
    Want want;
    want.SetElementName("com.ohos.test", "MainAbility");
    AbilityRequest abilityRequest;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    abilityRecord->abilityInfo_.bundleName = "com.ohos.test";
    abilityRecord->abilityInfo_.applicationInfo.appIndex = 1;
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();

    StartAbilityUtils::SetTargetCloneIndexInSameBundle(want, callerToken);
    int32_t appCloneIndex = want.GetIntParam(Want::PARAM_APP_CLONE_INDEX_KEY, -1);
    EXPECT_EQ(appCloneIndex, 1);

    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest SetTargetCloneIndexInSameBundle_004 end");
}

/**
 * @tc.name: SetTargetCloneIndexInSameBundle_005
 * @tc.desc: test class StartAbilityUtil SetTargetCloneIndexInSameBundle caller index invalid
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, SetTargetCloneIndexInSameBundle_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest SetTargetCloneIndexInSameBundle_005 start");
    Want want;
    want.SetElementName("com.ohos.test", "MainAbility");
    AbilityRequest abilityRequest;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    abilityRecord->abilityInfo_.bundleName = "com.ohos.test";
    abilityRecord->abilityInfo_.applicationInfo.appIndex = -2;
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();

    StartAbilityUtils::SetTargetCloneIndexInSameBundle(want, callerToken);
    int32_t appCloneIndex = want.GetIntParam(Want::PARAM_APP_CLONE_INDEX_KEY, -1);
    EXPECT_EQ(appCloneIndex, -1);

    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest SetTargetCloneIndexInSameBundle_005 end");
}

/**
 * @tc.name: StartUIAbilitiesProcessAppIndex_001
 * @tc.desc: test class StartAbilityUtil StartUIAbilitiesProcessAppIndex bundleName not equal; no index
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, StartUIAbilitiesProcessAppIndex_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest StartUIAbilitiesProcessAppIndex_001 start");
    int32_t appIndex;
    Want want;
    want.SetElementName("com.ohos.test", "MainAbility");
    AbilityRequest abilityRequest;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    abilityRecord->abilityInfo_.bundleName = "com.ohos.test1";
    abilityRecord->abilityInfo_.applicationInfo.appIndex = -2;
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();

    StartAbilityUtils::StartUIAbilitiesProcessAppIndex(want, callerToken, appIndex);
    int32_t appCloneIndex = want.GetIntParam(Want::PARAM_APP_CLONE_INDEX_KEY, -1);
    EXPECT_EQ(appCloneIndex, appIndex);
    EXPECT_EQ(appIndex, 0);

    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest StartUIAbilitiesProcessAppIndex_001 end");
}

/**
 * @tc.name: StartUIAbilitiesProcessAppIndex_002
 * @tc.desc: test class StartAbilityUtil StartUIAbilitiesProcessAppIndex has index valid
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, StartUIAbilitiesProcessAppIndex_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest StartUIAbilitiesProcessAppIndex_002 start");
    int32_t appIndex;
    Want want;
    want.SetElementName("com.ohos.test", "MainAbility");
    want.SetParam(Want::PARAM_APP_CLONE_INDEX_KEY, 1);
    AbilityRequest abilityRequest;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    abilityRecord->abilityInfo_.bundleName = "com.ohos.test";
    abilityRecord->abilityInfo_.applicationInfo.appIndex = -2;
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();

    int32_t ret = StartAbilityUtils::StartUIAbilitiesProcessAppIndex(want, callerToken, appIndex);
    int32_t appCloneIndex = want.GetIntParam(Want::PARAM_APP_CLONE_INDEX_KEY, -1);
    EXPECT_EQ(appCloneIndex, appIndex);
    EXPECT_EQ(appIndex, 1);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest StartUIAbilitiesProcessAppIndex_002 end");
}

/**
 * @tc.name: StartUIAbilitiesProcessAppIndex_003
 * @tc.desc: test class StartAbilityUtil StartUIAbilitiesProcessAppIndex has index invalid
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, StartUIAbilitiesProcessAppIndex_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest StartUIAbilitiesProcessAppIndex_003 start");
    int32_t appIndex;
    Want want;
    want.SetElementName("com.ohos.test", "MainAbility");
    want.SetParam(Want::PARAM_APP_CLONE_INDEX_KEY, -5);
    AbilityRequest abilityRequest;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    abilityRecord->abilityInfo_.bundleName = "com.ohos.test";
    abilityRecord->abilityInfo_.applicationInfo.appIndex = -2;
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();

    int32_t ret = StartAbilityUtils::StartUIAbilitiesProcessAppIndex(want, callerToken, appIndex);
    int32_t appCloneIndex = want.GetIntParam(Want::PARAM_APP_CLONE_INDEX_KEY, -1);
    EXPECT_EQ(appCloneIndex, appIndex);
    EXPECT_EQ(appIndex, -5);
    EXPECT_EQ(ret, ERR_APP_CLONE_INDEX_INVALID);

    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest StartUIAbilitiesProcessAppIndex_003 end");
}

/**
 * @tc.name: HandleSelfRedirection_001
 * @tc.desc: HandleSelfRedirection
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, HandleSelfRedirection_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest HandleSelfRedirection_001 start");
    AppExecFwk::AbilityInfo abilityInfo;
    std::vector<AppExecFwk::AbilityInfo> infos = { abilityInfo };
    int32_t ret = StartAbilityUtils::HandleSelfRedirection(false, infos);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest HandleSelfRedirection_001 end");
}

/**
 * @tc.name: HandleSelfRedirection_002
 * @tc.desc: HandleSelfRedirection
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, HandleSelfRedirection_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest HandleSelfRedirection_002 start");
    AppExecFwk::AbilityInfo abilityInfo;
    std::vector<AppExecFwk::AbilityInfo> infos;
    int32_t ret = StartAbilityUtils::HandleSelfRedirection(true, infos);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest HandleSelfRedirection_002 end");
}

/**
 * @tc.name: HandleSelfRedirection_003
 * @tc.desc: HandleSelfRedirection
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, HandleSelfRedirection_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest HandleSelfRedirection_003 start");
    AppExecFwk::AbilityInfo abilityInfo;
    std::vector<AppExecFwk::AbilityInfo> infos = { abilityInfo, abilityInfo };
    int32_t ret = StartAbilityUtils::HandleSelfRedirection(true, infos);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest HandleSelfRedirection_003 end");
}

/**
 * @tc.name: HandleSelfRedirection_004
 * @tc.desc: HandleSelfRedirection
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, HandleSelfRedirection_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest HandleSelfRedirection_004 start");
    AppExecFwk::AbilityInfo abilityInfo;
    std::vector<AppExecFwk::AbilityInfo> infos = { abilityInfo };
    int32_t ret = StartAbilityUtils::HandleSelfRedirection(true, infos);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest HandleSelfRedirection_004 end");
}

/**
 * @tc.name: HandleSelfRedirection_005
 * @tc.desc: HandleSelfRedirection
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, HandleSelfRedirection_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest HandleSelfRedirection_005 start");
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.allowSelfRedirect = false;
    std::vector<AppExecFwk::AbilityInfo> infos = { abilityInfo };
    int32_t ret = StartAbilityUtils::HandleSelfRedirection(true, infos);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest HandleSelfRedirection_005 end");
}

/**
 * @tc.name: HandleSelfRedirection_006
 * @tc.desc: HandleSelfRedirection
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, HandleSelfRedirection_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest HandleSelfRedirection_006 start");
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.allowSelfRedirect = false;
    abilityInfo.applicationInfo.uid = 100;
    abilityInfo.linkType = AppExecFwk::LinkType::APP_LINK;
    std::vector<AppExecFwk::AbilityInfo> infos = { abilityInfo };
    MyStatus::GetInstance().processInfo_.uid_ = 200;
    int32_t ret = StartAbilityUtils::HandleSelfRedirection(true, infos);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest HandleSelfRedirection_006 end");
}

/**
 * @tc.name: HandleSelfRedirection_007
 * @tc.desc: HandleSelfRedirection
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, HandleSelfRedirection_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest HandleSelfRedirection_007 start");
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.allowSelfRedirect = false;
    abilityInfo.applicationInfo.uid = 100;
    abilityInfo.linkType = AppExecFwk::LinkType::APP_LINK;
    std::vector<AppExecFwk::AbilityInfo> infos = { abilityInfo };
    MyStatus::GetInstance().processInfo_.uid_ = 100;
    int32_t ret = StartAbilityUtils::HandleSelfRedirection(true, infos);
    EXPECT_EQ(ret, ERR_SELF_REDIRECTION_DISALLOWED);

    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest HandleSelfRedirection_007 end");
}

/**
 * @tc.name: FindExtensionInfo_001
 * @tc.desc: FindExtensionInfo
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, FindExtensionInfo_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest FindExtensionInfo_001 start");
    Want want;
    want.SetParam(AAFwk::Want::DESTINATION_PLUGIN_ABILITY, false);
    int32_t abilityInfoFlag = 0;
    int32_t userId = 100;
    int32_t appIndex = 1;
    std::shared_ptr<StartAbilityInfo> abilityInfo = std::make_shared<StartAbilityInfo>();
    StartAbilityInfo::FindExtensionInfo(want, abilityInfoFlag, userId, appIndex, abilityInfo);
    EXPECT_EQ(abilityInfo->status, RESOLVE_ABILITY_ERR);
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest FindExtensionInfo_001 end");
}

/**
 * @tc.name: FindExtensionInfo_002
 * @tc.desc: FindExtensionInfo
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, FindExtensionInfo_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest FindExtensionInfo_002 start");
    Want want;
    want.SetParam(AAFwk::Want::DESTINATION_PLUGIN_ABILITY, true);
    int32_t abilityInfoFlag = 0;
    int32_t userId = 100;
    int32_t appIndex = 1;
    std::shared_ptr<StartAbilityInfo> abilityInfo = std::make_shared<StartAbilityInfo>();
    StartAbilityInfo::FindExtensionInfo(want, abilityInfoFlag, userId, appIndex, abilityInfo);
    EXPECT_EQ(abilityInfo->status, RESOLVE_ABILITY_ERR);
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest FindExtensionInfo_002 end");
}

/**
 * @tc.name: RemoveAtomicServiceShareRouterIfNeeded_001
 * @tc.desc: test RemoveAtomicServiceShareRouterIfNeeded when want has no ATOMIC_SERVICE_SHARE_ROUTER parameter
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, RemoveAtomicServiceShareRouterIfNeeded_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest RemoveAtomicServiceShareRouterIfNeeded_001 start");
    Want want;
    AppExecFwk::AbilityInfo targetAbilityInfo;
    targetAbilityInfo.applicationInfo.bundleType = AppExecFwk::BundleType::APP;

    StartAbilityUtils::RemoveAtomicServiceShareRouterIfNeeded(want, targetAbilityInfo);

    EXPECT_FALSE(want.HasParameter(Want::ATOMIC_SERVICE_SHARE_ROUTER));
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest RemoveAtomicServiceShareRouterIfNeeded_001 end");
}

/**
 * @tc.name: RemoveAtomicServiceShareRouterIfNeeded_002
 * @tc.desc: test RemoveAtomicServiceShareRouterIfNeeded when target is atomic service (keep parameter)
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, RemoveAtomicServiceShareRouterIfNeeded_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest RemoveAtomicServiceShareRouterIfNeeded_002 start");
    Want want;
    want.SetParam(Want::ATOMIC_SERVICE_SHARE_ROUTER, std::string("test_router"));
    AppExecFwk::AbilityInfo targetAbilityInfo;
    targetAbilityInfo.applicationInfo.bundleType = AppExecFwk::BundleType::ATOMIC_SERVICE;
    AAFwk::MyFlag::retVerifyAccessTokenId_ = Security::AccessToken::PermissionState::PERMISSION_GRANTED;

    StartAbilityUtils::RemoveAtomicServiceShareRouterIfNeeded(want, targetAbilityInfo);

    EXPECT_TRUE(want.HasParameter(Want::ATOMIC_SERVICE_SHARE_ROUTER));
    AAFwk::MyFlag::retVerifyAccessTokenId_ = 0;
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest RemoveAtomicServiceShareRouterIfNeeded_002 end");
}

/**
 * @tc.name: RemoveAtomicServiceShareRouterIfNeeded_003
 * @tc.desc: test RemoveAtomicServiceShareRouterIfNeeded when target is not atomic service and has permission
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, RemoveAtomicServiceShareRouterIfNeeded_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest RemoveAtomicServiceShareRouterIfNeeded_003 start");
    Want want;
    want.SetParam(Want::ATOMIC_SERVICE_SHARE_ROUTER, std::string("test_router"));
    AppExecFwk::AbilityInfo targetAbilityInfo;
    targetAbilityInfo.applicationInfo.bundleType = AppExecFwk::BundleType::APP;
    AAFwk::MyFlag::retVerifyAccessTokenId_ = Security::AccessToken::PermissionState::PERMISSION_GRANTED;

    StartAbilityUtils::RemoveAtomicServiceShareRouterIfNeeded(want, targetAbilityInfo);

    EXPECT_TRUE(want.HasParameter(Want::ATOMIC_SERVICE_SHARE_ROUTER));
    AAFwk::MyFlag::retVerifyAccessTokenId_ = 0;
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest RemoveAtomicServiceShareRouterIfNeeded_003 end");
}

/**
 * @tc.name: RemoveAtomicServiceShareRouterIfNeeded_004
 * @tc.desc: test RemoveAtomicServiceShareRouterIfNeeded when target is not atomic service and no permission
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, RemoveAtomicServiceShareRouterIfNeeded_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest RemoveAtomicServiceShareRouterIfNeeded_004 start");
    Want want;
    want.SetParam(Want::ATOMIC_SERVICE_SHARE_ROUTER, std::string("test_router"));
    AppExecFwk::AbilityInfo targetAbilityInfo;
    targetAbilityInfo.applicationInfo.bundleType = AppExecFwk::BundleType::APP;
    AAFwk::MyFlag::retVerifyAccessTokenId_ = Security::AccessToken::PermissionState::PERMISSION_DENIED;

    StartAbilityUtils::RemoveAtomicServiceShareRouterIfNeeded(want, targetAbilityInfo);

    EXPECT_FALSE(want.HasParameter(Want::ATOMIC_SERVICE_SHARE_ROUTER));
    AAFwk::MyFlag::retVerifyAccessTokenId_ = 0;
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest RemoveAtomicServiceShareRouterIfNeeded_004 end");
}

/**
 * @tc.name: RemoveAtomicServiceShareRouterIfNeeded_005
 * @tc.desc: test RemoveAtomicServiceShareRouterIfNeeded when target is atomic service and no permission
 * @tc.type: FUNC
 */
HWTEST_F(StartAbilityUtilsTest, RemoveAtomicServiceShareRouterIfNeeded_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest RemoveAtomicServiceShareRouterIfNeeded_005 start");
    Want want;
    want.SetParam(Want::ATOMIC_SERVICE_SHARE_ROUTER, std::string("test_router"));
    AppExecFwk::AbilityInfo targetAbilityInfo;
    targetAbilityInfo.applicationInfo.bundleType = AppExecFwk::BundleType::ATOMIC_SERVICE;
    AAFwk::MyFlag::retVerifyAccessTokenId_ = Security::AccessToken::PermissionState::PERMISSION_DENIED;

    StartAbilityUtils::RemoveAtomicServiceShareRouterIfNeeded(want, targetAbilityInfo);

    EXPECT_TRUE(want.HasParameter(Want::ATOMIC_SERVICE_SHARE_ROUTER));
    AAFwk::MyFlag::retVerifyAccessTokenId_ = 0;
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityUtilsTest RemoveAtomicServiceShareRouterIfNeeded_005 end");
}
}  // namespace AAFwk
}  // namespace OHOS
