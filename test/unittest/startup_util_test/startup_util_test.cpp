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

#include "ability_info.h"
#include "extension_ability_info.h"
#include "startup_util.h"
#include "want.h"

using namespace testing::ext;
namespace OHOS {
namespace AbilityRuntime {

class StartupUtilTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void StartupUtilTest::SetUpTestCase()
{}

void StartupUtilTest::TearDownTestCase()
{}

void StartupUtilTest::SetUp()
{}

void StartupUtilTest::TearDown()
{}

/**
 * @tc.name: IsSupportAppClone_001
 * @tc.desc: test class StartupUtil number function
 * @tc.type: FUNC
 */
HWTEST_F(StartupUtilTest, IsSupportAppClone_001, TestSize.Level1)
{
    AppExecFwk::ExtensionAbilityType type = AppExecFwk::ExtensionAbilityType::WORK_SCHEDULER;
    auto queryRet = StartupUtil::IsSupportAppClone(type);
    EXPECT_TRUE(queryRet);
}

/**
 * @tc.name: IsSupportAppClone_002
 * @tc.desc: test class StartupUtil number function
 * @tc.type: FUNC
 */
HWTEST_F(StartupUtilTest, IsSupportAppClone_002, TestSize.Level1)
{
    AppExecFwk::ExtensionAbilityType type = AppExecFwk::ExtensionAbilityType::FORM;
    auto queryRet = StartupUtil::IsSupportAppClone(type);
    EXPECT_FALSE(queryRet);
}

/**
 * @tc.name: IsSupportAppClone_003
 * @tc.desc: test FAULT_LOG support app clone
 * @tc.type: FUNC
 */
HWTEST_F(StartupUtilTest, IsSupportAppClone_003, TestSize.Level1)
{
    AppExecFwk::ExtensionAbilityType type = AppExecFwk::ExtensionAbilityType::FAULT_LOG;
    auto queryRet = StartupUtil::IsSupportAppClone(type);
    EXPECT_TRUE(queryRet);
}

/**
 * @tc.name: GenerateFullRequestCode_001
 * @tc.desc: test class StartupUtil number function GenerateFullRequestCode
 * @tc.type: FUNC
 */
HWTEST_F(StartupUtilTest, GenerateFullRequestCode_001, TestSize.Level1)
{
    auto requestCode = StartupUtil::GenerateFullRequestCode(1, true, 0);
    EXPECT_EQ(requestCode, 0);
    
    requestCode = StartupUtil::GenerateFullRequestCode(0, true, 1);
    EXPECT_EQ(requestCode, 0);

    requestCode = StartupUtil::GenerateFullRequestCode(1, true, 1);
    uint64_t tempNum = 1;
    EXPECT_EQ((requestCode & tempNum), 1);
    EXPECT_EQ((requestCode & (tempNum << 32)), (tempNum << 32));
    EXPECT_EQ((requestCode & (tempNum << 48)), (tempNum << 48));
}

/**
 * @tc.name: GenerateFullRequestCode_001
 * @tc.desc: test class StartupUtil number function ParseFullRequestCode
 * @tc.type: FUNC
 */
HWTEST_F(StartupUtilTest, ParseFullRequestCode_001, TestSize.Level1)
{
    auto requestInfo = StartupUtil::ParseFullRequestCode(-1);
    EXPECT_EQ(requestInfo.requestCode, 0);
    
    requestInfo = StartupUtil::ParseFullRequestCode(0);
    EXPECT_EQ(requestInfo.requestCode, 0);

    requestInfo = StartupUtil::ParseFullRequestCode(1);
    EXPECT_EQ(requestInfo.requestCode, 1);
    EXPECT_EQ(requestInfo.pid, 0);
    EXPECT_EQ(requestInfo.backFlag, false);

    uint64_t tempNum = 1;
    requestInfo = StartupUtil::ParseFullRequestCode((tempNum << 49));
    EXPECT_FALSE(requestInfo.backFlag);
    
    requestInfo = StartupUtil::ParseFullRequestCode((tempNum << 48));
    EXPECT_TRUE(requestInfo.backFlag);

    auto requestCode = StartupUtil::GenerateFullRequestCode(1, true, 1);
    requestInfo = StartupUtil::ParseFullRequestCode(requestCode);
    EXPECT_EQ(requestInfo.requestCode, 1);
    EXPECT_EQ(requestInfo.pid, 1);
    EXPECT_EQ(requestInfo.backFlag, true);

    requestCode = StartupUtil::GenerateFullRequestCode(1, false, 1);
    requestInfo = StartupUtil::ParseFullRequestCode(requestCode);
    EXPECT_EQ(requestInfo.requestCode, 1);
    EXPECT_EQ(requestInfo.pid, 1);
    EXPECT_EQ(requestInfo.backFlag, false);
}

/**
 * @tc.name: InitAbilityInfoFromExtension_001
 * @tc.desc: test class StartupUtil number function
 * @tc.type: FUNC
 */
HWTEST_F(StartupUtilTest, InitAbilityInfoFromExtension_001, TestSize.Level1)
{
    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    AppExecFwk::AbilityInfo abilityInfo;
    extensionInfo.uid = 1000;
    extensionInfo.bundleName = "test";
    extensionInfo.moduleName = "moduleName";
    extensionInfo.name = "name";
    extensionInfo.extensionTypeName = "extensionTypeName";
    StartupUtil::InitAbilityInfoFromExtension(extensionInfo, abilityInfo);
    EXPECT_EQ(abilityInfo.bundleName, extensionInfo.bundleName);
    EXPECT_EQ(abilityInfo.package, extensionInfo.moduleName);
    EXPECT_EQ(abilityInfo.moduleName, extensionInfo.moduleName);
    EXPECT_EQ(abilityInfo.name, extensionInfo.name);
    EXPECT_TRUE(abilityInfo.isModuleJson);
    EXPECT_TRUE(abilityInfo.isStageBasedModel);
    EXPECT_EQ(abilityInfo.extensionTypeName, extensionInfo.extensionTypeName);
    EXPECT_EQ(abilityInfo.uid, extensionInfo.uid);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
