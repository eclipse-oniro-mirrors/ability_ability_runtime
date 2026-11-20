/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "ability_running_record.h"
#include "app_state_callback_host.h"
#include "hilog_tag_wrapper.h"
#include "mock_ability_token.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr const char* BUNDLE_NAME_SCENEBOARD = "com.ohos.sceneboard";
constexpr const char* SCENEBOARD_ABILITY_NAME = "com.ohos.sceneboard.MainAbility";
static const std::string EMPTY_NAME;
}
class AbilityRunningRecordTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
public:
protected:
    static const std::string GetTestBundleName()
    {
        return "test_bundle_name";
    }
    static const std::string GetTestAbilityInfoName()
    {
        return "test_ability_info_name";
    }
    static const std::string GetTestModuleName()
    {
        return "test_module_name";
    }
};

void AbilityRunningRecordTest::SetUpTestCase()
{}

void AbilityRunningRecordTest::TearDownTestCase()
{}

void AbilityRunningRecordTest::SetUp()
{}

void AbilityRunningRecordTest::TearDown()
{}


/*
 * Feature: AbilityRunningRecord
 * Function: GetName
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord GetName
 * EnvConditions: NA
 * CaseDescription: GetName
 */
HWTEST_F(AbilityRunningRecordTest, GetName_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "GetName_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityInfoName();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    auto name = record->GetName();
    ASSERT_EQ(name, "test_ability_info_name");
    TAG_LOGD(AAFwkTag::TEST, "GetName_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: GetName
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord GetName
 * EnvConditions: NA
 * CaseDescription: GetName
 */
HWTEST_F(AbilityRunningRecordTest, GetName_002, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "GetName_002 start.");
    auto record = std::make_shared<AbilityRunningRecord>(nullptr, nullptr, 0);
    ASSERT_NE(record, nullptr);
    EXPECT_EQ(record->GetName(), EMPTY_NAME);
    TAG_LOGD(AAFwkTag::TEST, "GetName_002 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: GetBundleName
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord GetBundleName
 * EnvConditions: NA
 * CaseDescription: GetBundleName
 */
HWTEST_F(AbilityRunningRecordTest, GetBundleName_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "GetBundleName_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->bundleName = GetTestBundleName();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    auto name = record->GetBundleName();
    ASSERT_EQ(name, "test_bundle_name");
    TAG_LOGD(AAFwkTag::TEST, "GetBundleName_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: GetBundleName
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord GetBundleName
 * EnvConditions: NA
 * CaseDescription: GetBundleName
 */
HWTEST_F(AbilityRunningRecordTest, GetBundleName_002, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "GetBundleName_002 start.");
    auto record = std::make_shared<AbilityRunningRecord>(nullptr, nullptr, 0);
    ASSERT_NE(record, nullptr);
    EXPECT_EQ(record->GetBundleName(), EMPTY_NAME);
    TAG_LOGD(AAFwkTag::TEST, "GetBundleName_002 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: GetModuleName
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord GetModuleName
 * EnvConditions: NA
 * CaseDescription: GetModuleName
 */
HWTEST_F(AbilityRunningRecordTest, GetModuleName_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "GetModuleName_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->moduleName = GetTestModuleName();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    auto name = record->GetModuleName();
    ASSERT_EQ(name, "test_module_name");
    TAG_LOGD(AAFwkTag::TEST, "GetModuleName_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: GetModuleName
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord GetModuleName
 * EnvConditions: NA
 * CaseDescription: GetModuleName
 */
HWTEST_F(AbilityRunningRecordTest, GetModuleName_002, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "GetModuleName_002 start.");
    auto record = std::make_shared<AbilityRunningRecord>(nullptr, nullptr, 0);
    ASSERT_NE(record, nullptr);
    EXPECT_EQ(record->GetModuleName(), EMPTY_NAME);
    TAG_LOGD(AAFwkTag::TEST, "GetModuleName_002 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: GetAbilityInfo
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord GetAbilityInfo
 * EnvConditions: NA
 * CaseDescription: GetAbilityInfo
 */
HWTEST_F(AbilityRunningRecordTest, GetAbilityInfo_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "GetAbilityInfo_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    auto iret = record->GetAbilityInfo();
    ASSERT_NE(iret, nullptr);
    TAG_LOGD(AAFwkTag::TEST, "GetAbilityInfo_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: SetWant
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord SetWant
 * EnvConditions: NA
 * CaseDescription: SetWant
 */
HWTEST_F(AbilityRunningRecordTest, SetWant_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "SetWant_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    const std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    record->SetWant(want);
    auto iret = record->GetWant();
    ASSERT_EQ(iret, want);
    TAG_LOGD(AAFwkTag::TEST, "SetWant_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: GetToken
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord GetToken
 * EnvConditions: NA
 * CaseDescription: GetToken
 */
HWTEST_F(AbilityRunningRecordTest, GetToken_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "GetToken_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    auto iret = record->GetToken();
    ASSERT_NE(iret, nullptr);
    TAG_LOGD(AAFwkTag::TEST, "GetToken_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: SetState
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord SetState
 * EnvConditions: NA
 * CaseDescription: SetState
 */
HWTEST_F(AbilityRunningRecordTest, SetState_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "SetState_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    record->SetState(AbilityState::ABILITY_STATE_CREATE);
    auto iret = record->GetState();
    ASSERT_EQ(iret, AbilityState::ABILITY_STATE_CREATE);
    TAG_LOGD(AAFwkTag::TEST, "SetState_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: SetEventId
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord SetEventId
 * EnvConditions: NA
 * CaseDescription: SetEventId
 */
HWTEST_F(AbilityRunningRecordTest, SetEventId_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "SetEventId_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    const int64_t eventId = 1;
    record->SetEventId(eventId);
    auto iret = record->GetEventId();
    ASSERT_EQ(iret, 1);
    TAG_LOGD(AAFwkTag::TEST, "SetEventId_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: SetTerminating
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord SetTerminating
 * EnvConditions: NA
 * CaseDescription: SetTerminating
 */
HWTEST_F(AbilityRunningRecordTest, SetTerminating_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "SetTerminating_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    record->SetTerminating();
    auto iret = record->IsTerminating();
    ASSERT_EQ(iret, true);
    TAG_LOGD(AAFwkTag::TEST, "SetTerminating_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: SetOwnerUserId
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord SetOwnerUserId
 * EnvConditions: NA
 * CaseDescription: SetOwnerUserId
 */
HWTEST_F(AbilityRunningRecordTest, SetOwnerUserId_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "SetOwnerUserId_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    int32_t ownerUserId = 1;
    record->SetOwnerUserId(ownerUserId);
    auto iret = record->GetOwnerUserId();
    ASSERT_EQ(iret, 1);
    TAG_LOGD(AAFwkTag::TEST, "SetOwnerUserId_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: SetIsSingleUser
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord SetIsSingleUser
 * EnvConditions: NA
 * CaseDescription: SetIsSingleUser
 */
HWTEST_F(AbilityRunningRecordTest, SetIsSingleUser_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "SetIsSingleUser_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    bool flag = true;
    record->SetIsSingleUser(flag);
    auto iret = record->IsSingleUser();
    ASSERT_EQ(iret, true);
    TAG_LOGD(AAFwkTag::TEST, "SetIsSingleUser_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: UpdateFocusState
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord UpdateFocusState
 * EnvConditions: NA
 * CaseDescription: UpdateFocusState
 */
HWTEST_F(AbilityRunningRecordTest, UpdateFocusState_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "UpdateFocusState_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    bool isFocus = true;
    record->UpdateFocusState(isFocus);
    auto iret = record->GetFocusFlag();
    ASSERT_EQ(iret, true);
    TAG_LOGD(AAFwkTag::TEST, "UpdateFocusState_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: SetUIExtensionAbilityId
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord SetUIExtensionAbilityId
 * EnvConditions: NA
 * CaseDescription: SetUIExtensionAbilityId
 */
HWTEST_F(AbilityRunningRecordTest, SetUIExtensionAbilityId_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "SetUIExtensionAbilityId_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    const int32_t uiExtensionAbilityId = 1;
    record->SetUIExtensionAbilityId(uiExtensionAbilityId);
    auto iret = record->GetUIExtensionAbilityId();
    ASSERT_EQ(iret, 1);
    TAG_LOGD(AAFwkTag::TEST, "SetUIExtensionAbilityId_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: IsHook
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord IsHook
 * EnvConditions: NA
 * CaseDescription: IsHook
 */
HWTEST_F(AbilityRunningRecordTest, IsHook_001, TestSize.Level1)
{
    auto record = std::make_shared<AbilityRunningRecord>(nullptr, nullptr, 1);
    auto want = std::make_shared<AAFwk::Want>();
    record->SetWant(want);
    EXPECT_FALSE(record->IsHook());
}

/*
 * Feature: AbilityRunningRecord
 * Function: IsSceneBoard
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord IsSceneBoard
 * EnvConditions: NA
 * CaseDescription: IsSceneBoard
 */
HWTEST_F(AbilityRunningRecordTest, IsSceneBoard_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "IsSceneBoard_001 start.");
    auto record = std::make_shared<AbilityRunningRecord>(nullptr, nullptr, 1);
    ASSERT_NE(record, nullptr);
    EXPECT_FALSE(record->IsSceneBoard());
    TAG_LOGD(AAFwkTag::TEST, "IsSceneBoard_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: IsSceneBoard
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord IsSceneBoard
 * EnvConditions: NA
 * CaseDescription: IsSceneBoard
 */
HWTEST_F(AbilityRunningRecordTest, IsSceneBoard_002, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "IsSceneBoard_002 start.");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = SCENEBOARD_ABILITY_NAME;
    abilityInfo->bundleName = BUNDLE_NAME_SCENEBOARD;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, nullptr, 1);
    ASSERT_NE(record, nullptr);
    EXPECT_TRUE(record->IsSceneBoard());

    abilityInfo->bundleName = "invalid_bundle_name";
    EXPECT_FALSE(record->IsSceneBoard());
    TAG_LOGD(AAFwkTag::TEST, "IsSceneBoard_002 end.");
}

/**
 * @tc.name: SetClearSessionStatus_001
 * @tc.desc: Test IsClearSession default value is false
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRunningRecordTest, SetClearSessionStatus_001, TestSize.Level1)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityInfoName();
    abilityInfo->bundleName = GetTestBundleName();
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, nullptr, 1);
    ASSERT_NE(record, nullptr);
    EXPECT_FALSE(record->IsClearSession());
}

/**
 * @tc.name: SetClearSessionStatus_002
 * @tc.desc: Test IsClearSession returns true after SetClearSessionStatus
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRunningRecordTest, SetClearSessionStatus_002, TestSize.Level1)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityInfoName();
    abilityInfo->bundleName = GetTestBundleName();
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, nullptr, 1);
    ASSERT_NE(record, nullptr);
    record->SetClearSessionStatus();
    EXPECT_TRUE(record->IsClearSession());
}
}  // namespace AppExecFwk
}  // namespace OHOS
