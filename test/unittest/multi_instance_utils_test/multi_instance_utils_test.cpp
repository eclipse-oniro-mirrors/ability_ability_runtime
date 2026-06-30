/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "ability_record.h"
#include "app_utils.h"
#undef private
#include "hilog_tag_wrapper.h"
#include "multi_instance_utils.h"
#include "parameters.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class MultiInstanceUtilsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    std::shared_ptr<AbilityRecord> MockAbilityRecord(AppExecFwk::AbilityType abilityType);
};

void MultiInstanceUtilsTest::SetUpTestCase(void) {}
void MultiInstanceUtilsTest::TearDownTestCase(void) {}
void MultiInstanceUtilsTest::SetUp() {}
void MultiInstanceUtilsTest::TearDown() {}

std::shared_ptr<AbilityRecord> MultiInstanceUtilsTest::MockAbilityRecord(AppExecFwk::AbilityType abilityType)
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.demo";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = abilityType;
    return AbilityRecord::CreateAbilityRecord(abilityRequest);
}

/**
 * @tc.name: MultiInstanceUtils_GetInstanceKey_0100
 * @tc.desc: GetInstanceKey
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(MultiInstanceUtilsTest, MultiInstanceUtils_GetInstanceKey_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "MultiInstanceUtils_GetInstanceKey_0100 start");
    Want want;
    want.SetParam(Want::APP_INSTANCE_KEY, std::string("app_instance_0"));
    auto key = MultiInstanceUtils::GetInstanceKey(want);
    EXPECT_EQ(key, "app_instance_0");

    TAG_LOGI(AAFwkTag::TEST, "MultiInstanceUtils_GetInstanceKey_0100 end");
}

/**
 * @tc.name: MultiInstanceUtils_GetValidExtensionInstanceKey_0100
 * @tc.desc: GetValidExtensionInstanceKey
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(MultiInstanceUtilsTest, MultiInstanceUtils_GetValidExtensionInstanceKey_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "MultiInstanceUtils_GetValidExtensionInstanceKey_0100 start");
    AbilityRequest abilityRequest;
    auto key = MultiInstanceUtils::GetValidExtensionInstanceKey(abilityRequest);
    EXPECT_EQ(key, "app_instance_0");

    TAG_LOGI(AAFwkTag::TEST, "MultiInstanceUtils_GetValidExtensionInstanceKey_0100 end");
}

/**
 * @tc.name: MultiInstanceUtils_GetValidExtensionInstanceKey_0200
 * @tc.desc: GetValidExtensionInstanceKey
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(MultiInstanceUtilsTest, MultiInstanceUtils_GetValidExtensionInstanceKey_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "MultiInstanceUtils_GetValidExtensionInstanceKey_0100 start");
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::WORK_SCHEDULER;
    abilityRequest.want.SetParam(Want::APP_INSTANCE_KEY, std::string("app_instance_1"));
    auto key = MultiInstanceUtils::GetValidExtensionInstanceKey(abilityRequest);
    EXPECT_EQ(key, "app_instance_1");

    TAG_LOGI(AAFwkTag::TEST, "MultiInstanceUtils_GetValidExtensionInstanceKey_0200 end");
}

/**
 * @tc.name: MultiInstanceUtils_GetValidExtensionInstanceKey_0300
 * @tc.desc: GetValidExtensionInstanceKey
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(MultiInstanceUtilsTest, MultiInstanceUtils_GetValidExtensionInstanceKey_0300, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::WORK_SCHEDULER;
    sptr<IRemoteObject> callerToken = nullptr;
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(AppExecFwk::AbilityType::PAGE);
    ASSERT_NE(abilityRecord, nullptr);
    callerToken = abilityRecord->GetToken();
    abilityRequest.callerToken = callerToken;
    auto key = MultiInstanceUtils::GetValidExtensionInstanceKey(abilityRequest);
    EXPECT_EQ(key, "app_instance_0");
}

/**
 * @tc.name: MultiInstanceUtils_GetValidExtensionInstanceKey_0400
 * @tc.desc: GetValidExtensionInstanceKey
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(MultiInstanceUtilsTest, MultiInstanceUtils_GetValidExtensionInstanceKey_0400, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::WORK_SCHEDULER;
    std::string bundleName = "";
    Want want;
    want.SetBundle(bundleName);
    sptr<IRemoteObject> callerToken = nullptr;
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(AppExecFwk::AbilityType::PAGE);
    ASSERT_NE(abilityRecord, nullptr);
    std::string instanceKey = "instanceKey";
    abilityRecord->SetInstanceKey(instanceKey);
    callerToken = abilityRecord->GetToken();
    abilityRequest.callerToken = callerToken;
    auto key = MultiInstanceUtils::GetValidExtensionInstanceKey(abilityRequest);
    EXPECT_EQ(key, instanceKey);
}

/**
 * @tc.name: MultiInstanceUtils_GetSelfCallerInstanceKey_0100
 * @tc.desc: GetSelfCallerInstanceKey
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(MultiInstanceUtilsTest, MultiInstanceUtils_GetSelfCallerInstanceKey_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "MultiInstanceUtils_GetSelfCallerInstanceKey_0100 start");
    AbilityRequest abilityRequest;
    auto key = MultiInstanceUtils::GetSelfCallerInstanceKey(abilityRequest);
    EXPECT_EQ(key, "");

    TAG_LOGI(AAFwkTag::TEST, "MultiInstanceUtils_GetSelfCallerInstanceKey_0100 end");
}

/**
 * @tc.name: MultiInstanceUtils_IsDefaultInstanceKey_0100
 * @tc.desc: IsDefaultInstanceKey
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(MultiInstanceUtilsTest, MultiInstanceUtils_IsDefaultInstanceKey_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "MultiInstanceUtils_IsDefaultInstanceKey_0100 start");
    auto ret = MultiInstanceUtils::IsDefaultInstanceKey("app_instance_0");
    EXPECT_EQ(ret, true);

    TAG_LOGI(AAFwkTag::TEST, "MultiInstanceUtils_IsDefaultInstanceKey_0100 end");
}

/**
 * @tc.name: MultiInstanceUtils_IsMultiInstanceApp_0100
 * @tc.desc: IsMultiInstanceApp
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(MultiInstanceUtilsTest, MultiInstanceUtils_IsMultiInstanceApp_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "MultiInstanceUtils_IsMultiInstanceApp_0100 start");
    AbilityRequest abilityRequest;
    auto ret = MultiInstanceUtils::IsMultiInstanceApp(abilityRequest.appInfo);
    EXPECT_EQ(ret, false);

    TAG_LOGI(AAFwkTag::TEST, "MultiInstanceUtils_IsMultiInstanceApp_0100 end");
}

/**
 * @tc.name: MultiInstanceUtils_IsSupportedExtensionType_0100
 * @tc.desc: IsSupportedExtensionType
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(MultiInstanceUtilsTest, MultiInstanceUtils_IsSupportedExtensionType_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "MultiInstanceUtils_IsSupportedExtensionType_0100 start");
    AbilityRequest abilityRequest;
    auto ret = MultiInstanceUtils::IsSupportedExtensionType(AppExecFwk::ExtensionAbilityType::WORK_SCHEDULER);
    EXPECT_EQ(ret, true);
    ret = MultiInstanceUtils::IsSupportedExtensionType(AppExecFwk::ExtensionAbilityType::BACKUP);
    EXPECT_EQ(ret, true);
    ret = MultiInstanceUtils::IsSupportedExtensionType(AppExecFwk::ExtensionAbilityType::SHARE);
    EXPECT_EQ(ret, true);
    ret = MultiInstanceUtils::IsSupportedExtensionType(AppExecFwk::ExtensionAbilityType::WINDOW);
    EXPECT_EQ(ret, false);

    TAG_LOGI(AAFwkTag::TEST, "MultiInstanceUtils_IsSupportedExtensionType_0100 end");
}

/**
 * @tc.name: MultiInstanceUtils_IsInstanceKeyExist_0100
 * @tc.desc: IsInstanceKeyExist
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(MultiInstanceUtilsTest, MultiInstanceUtils_IsInstanceKeyExist_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "MultiInstanceUtils_IsInstanceKeyExist_0100 start");
    auto ret = MultiInstanceUtils::IsInstanceKeyExist("", "");
    EXPECT_EQ(ret, false);

    TAG_LOGI(AAFwkTag::TEST, "MultiInstanceUtils_IsInstanceKeyExist_0100 end");
}
}  // namespace AAFwk
}  // namespace OHOS
