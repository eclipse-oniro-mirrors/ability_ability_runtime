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

#define protected public
#include "extension_record_factory.h"
#undef protected

#include "hilog_tag_wrapper.h"

using namespace testing;
using namespace testing::ext;
using OHOS::AppExecFwk::ExtensionAbilityType;

namespace OHOS {
namespace AAFwk {
class ExtensionRecordFactoryTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void ExtensionRecordFactoryTest::SetUpTestCase() {}

void ExtensionRecordFactoryTest::TearDownTestCase() {}

void ExtensionRecordFactoryTest::SetUp() {}

void ExtensionRecordFactoryTest::TearDown() {}

/*
 * Feature: PreCheck_0100
 * Function: PreCheck
 * SubFunction: NA
 */
HWTEST_F(ExtensionRecordFactoryTest, PreCheck_0100, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);
    AAFwk::AbilityRequest abilityRequest;
    char data[] = {"123"};
    int32_t int32Param = 123;
    extensionRecordFactory->NeedReuse(abilityRequest, int32Param);
    std::string strParam("123,1");
    EXPECT_EQ(extensionRecordFactory->PreCheck(abilityRequest, strParam), ERR_OK);
    abilityRequest.extensionType = ExtensionAbilityType::WORK_SCHEDULER;
    EXPECT_EQ(extensionRecordFactory->PreCheck(abilityRequest, strParam), ERR_OK);
    abilityRequest.extensionType = ExtensionAbilityType::INPUTMETHOD;
    EXPECT_EQ(extensionRecordFactory->PreCheck(abilityRequest, strParam), ERR_OK);
}

/*
 * Feature: PreCheck_0200
 * Function: PreCheck
 * SubFunction: NA
 */
HWTEST_F(ExtensionRecordFactoryTest, PreCheck_0200, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);
    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.extensionType = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    std::string hostBundleName = "com.example.unittest";
    int32_t result = extensionRecordFactory->PreCheck(abilityRequest, "");
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    abilityRequest.abilityInfo.applicationName = "com.example.unittest";
    abilityRequest.sessionInfo =  new (std::nothrow) AAFwk::SessionInfo();
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::SERVICE;
    std::shared_ptr<AbilityRecord> callerAbilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    auto callerToken = callerAbilityRecord->GetToken();
    abilityRequest.sessionInfo->callerToken = callerToken;
    result = extensionRecordFactory->PreCheck(abilityRequest, "");
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    result = extensionRecordFactory->PreCheck(abilityRequest, hostBundleName);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRequest.sessionInfo->callerToken = abilityRecord->GetToken();
    result = extensionRecordFactory->PreCheck(abilityRequest, hostBundleName);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: GetExtensionProcessMode_0100
 * Function: GetExtensionProcessMode
 * SubFunction: NA
 */
HWTEST_F(ExtensionRecordFactoryTest, GetExtensionProcessMode_0100, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);
    AAFwk::AbilityRequest abilityRequest01;
    bool boolParam = false;
    EXPECT_NE(extensionRecordFactory->GetExtensionProcessMode(abilityRequest01, boolParam), ERR_OK);
    boolParam = true;
    EXPECT_NE(extensionRecordFactory->GetExtensionProcessMode(abilityRequest01, boolParam), ERR_OK);
}

/*
 * Feature: CreateRecord_0100
 * Function: CreateRecord
 * SubFunction: NA
 */
HWTEST_F(ExtensionRecordFactoryTest, CreateRecord_0100, TestSize.Level1)
{
    auto extensionRecordFactory = std::make_shared<AbilityRuntime::ExtensionRecordFactory>();
    EXPECT_NE(extensionRecordFactory, nullptr);
    AAFwk::AbilityRequest abilityRequest;
    std::shared_ptr<AbilityRuntime::ExtensionRecord> extensionRecord;
    EXPECT_EQ(extensionRecordFactory->CreateRecord(abilityRequest, extensionRecord), ERR_OK);
}
} // namespace AAFwk
} // namespace OHOS
