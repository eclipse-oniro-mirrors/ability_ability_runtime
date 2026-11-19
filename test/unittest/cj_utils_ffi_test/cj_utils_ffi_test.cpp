/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#include "cj_utils_ffi.h"

#include "securec.h"
#include "configuration.h"
#include "global_configuration_key.h"
#include <cstdlib>

using namespace testing;
using namespace testing::ext;

class CjUtilsFfiTest : public testing::Test {
public:
    CjUtilsFfiTest()
    {}
    ~CjUtilsFfiTest()
    {}
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void CjUtilsFfiTest::SetUpTestCase()
{}

void CjUtilsFfiTest::TearDownTestCase()
{}

void CjUtilsFfiTest::SetUp()
{}

void CjUtilsFfiTest::TearDown()
{}

/**
 * @tc.name: CjElementNameFfiTestContext_0100
 * @tc.desc: CjUtilsFfiTest test for CreateCStringFromString.
 * @tc.type: FUNC
 */
HWTEST_F(CjUtilsFfiTest, CjUtilsFfiTestCreateCStringFromString_0100, TestSize.Level1)
{
    // 测试用例1：空字符串
    std::string emptyStr = "";
    const char* result1 = CreateCStringFromString(emptyStr);
    EXPECT_TRUE(result1 == nullptr);

    // 测试用例2：正常字符串
    std::string normalStr = "Hello, world!";
    const char* result2 = CreateCStringFromString(normalStr);
    EXPECT_TRUE(result2 != nullptr);

    // 测试用例3：包含特殊字符的字符串
    std::string specialStr = "Hello, \0world!";
    const char* result3 = CreateCStringFromString(specialStr);
    EXPECT_TRUE(result3 != nullptr);
}

/**
 * @tc.name: CjUtilsFfiTestCreateCConfigurationV2_0100
 * @tc.desc: CjUtilsFfiTest test for CreateCConfigurationV2.
 * @tc.type: FUNC
 */
HWTEST_F(CjUtilsFfiTest, CjUtilsFfiTestCreateCConfigurationV2_0100, TestSize.Level1)
{
    OHOS::AppExecFwk::Configuration configuration;
    configuration.AddItem(OHOS::AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, "en_US");
    configuration.AddItem(OHOS::AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, "dark");
    configuration.AddItem(OHOS::AppExecFwk::ConfigurationInner::APPLICATION_DIRECTION, "vertical");
    configuration.AddItem(OHOS::AppExecFwk::ConfigurationInner::APPLICATION_DENSITYDPI, "xldpi");
    configuration.AddItem(OHOS::AppExecFwk::ConfigurationInner::APPLICATION_DISPLAYID, "1");
    configuration.AddItem(OHOS::AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE, "true");
    configuration.AddItem(OHOS::AAFwk::GlobalConfigurationKey::SYSTEM_FONT_SIZE_SCALE, "1.5");
    configuration.AddItem(OHOS::AAFwk::GlobalConfigurationKey::SYSTEM_FONT_WEIGHT_SCALE, "1.2");
    configuration.AddItem(OHOS::AAFwk::GlobalConfigurationKey::SYSTEM_MCC, "460");
    configuration.AddItem(OHOS::AAFwk::GlobalConfigurationKey::SYSTEM_MNC, "01");
    configuration.AddItem(OHOS::AAFwk::GlobalConfigurationKey::SYSTEM_FONT_ID, "test_font_id");
    configuration.AddItem(OHOS::AAFwk::GlobalConfigurationKey::SYSTEM_LOCALE, "zh_CN");

    OHOS::AbilityRuntime::CConfigurationV2 cfg = OHOS::AbilityRuntime::CreateCConfigurationV2(configuration);
    
    EXPECT_TRUE(cfg.language != nullptr);
    EXPECT_EQ(cfg.colorMode, 0);
    EXPECT_EQ(cfg.direction, 0);
    EXPECT_EQ(cfg.screenDensity, 320);
    EXPECT_EQ(cfg.displayId, 1);
    EXPECT_TRUE(cfg.hasPointerDevice);
    EXPECT_DOUBLE_EQ(cfg.fontSizeScale, 1.5);
    EXPECT_DOUBLE_EQ(cfg.fontWeightScale, 1.2);
    EXPECT_TRUE(cfg.mcc != nullptr);
    EXPECT_TRUE(cfg.mnc != nullptr);
    EXPECT_TRUE(cfg.fontId != nullptr);
    EXPECT_TRUE(cfg.locale != nullptr);

    OHOS::AbilityRuntime::FreeCConfigurationV2(&cfg);
}

/**
 * @tc.name: CjUtilsFfiTestCreateCConfigurationV2_0200
 * @tc.desc: CjUtilsFfiTest test for CreateCConfigurationV2 with empty configuration.
 * @tc.type: FUNC
 */
HWTEST_F(CjUtilsFfiTest, CjUtilsFfiTestCreateCConfigurationV2_0200, TestSize.Level1)
{
    OHOS::AppExecFwk::Configuration configuration;

    OHOS::AbilityRuntime::CConfigurationV2 cfg = OHOS::AbilityRuntime::CreateCConfigurationV2(configuration);

    EXPECT_TRUE(cfg.language == nullptr);
    EXPECT_EQ(cfg.colorMode, -1);
    EXPECT_EQ(cfg.direction, -1);
    EXPECT_EQ(cfg.screenDensity, 0);
    EXPECT_EQ(cfg.displayId, -1);
    EXPECT_FALSE(cfg.hasPointerDevice);
    EXPECT_DOUBLE_EQ(cfg.fontSizeScale, 1.0);
    EXPECT_DOUBLE_EQ(cfg.fontWeightScale, 1.0);
    EXPECT_TRUE(cfg.mcc == nullptr);
    EXPECT_TRUE(cfg.mnc == nullptr);
    EXPECT_TRUE(cfg.fontId == nullptr);
    EXPECT_TRUE(cfg.locale == nullptr);

    OHOS::AbilityRuntime::FreeCConfigurationV2(&cfg);
}

/**
 * @tc.name: CjUtilsFfiTestFreeCConfigurationV2_0100
 * @tc.desc: CjUtilsFfiTest test for FreeCConfigurationV2.
 * @tc.type: FUNC
 */
HWTEST_F(CjUtilsFfiTest, CjUtilsFfiTestFreeCConfigurationV2_0100, TestSize.Level1)
{
    OHOS::AppExecFwk::Configuration configuration;
    configuration.AddItem(OHOS::AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, "en_US");
    configuration.AddItem(OHOS::AAFwk::GlobalConfigurationKey::SYSTEM_MCC, "460");
    configuration.AddItem(OHOS::AAFwk::GlobalConfigurationKey::SYSTEM_MNC, "01");
    configuration.AddItem(OHOS::AAFwk::GlobalConfigurationKey::SYSTEM_FONT_ID, "test_font_id");
    configuration.AddItem(OHOS::AAFwk::GlobalConfigurationKey::SYSTEM_LOCALE, "zh_CN");

    OHOS::AbilityRuntime::CConfigurationV2 cfg = OHOS::AbilityRuntime::CreateCConfigurationV2(configuration);

    EXPECT_TRUE(cfg.language != nullptr);
    EXPECT_TRUE(cfg.mcc != nullptr);
    EXPECT_TRUE(cfg.mnc != nullptr);
    EXPECT_TRUE(cfg.fontId != nullptr);
    EXPECT_TRUE(cfg.locale != nullptr);

    OHOS::AbilityRuntime::FreeCConfigurationV2(&cfg);
}