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

#include "resource_config_helper.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace testing;
using namespace testing::ext;

class ResourceConfigHelperTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: ConvertStringToUint32_0100
 * @tc.desc: ConvertStringToUint32 with normal valid value "100".
 * @tc.type: FUNC
 */
HWTEST_F(ResourceConfigHelperTest, ConvertStringToUint32_0100, TestSize.Level1)
{
    uint32_t result = 0;
    EXPECT_TRUE(ResourceConfigHelper::ConvertStringToUint32("100", result));
    EXPECT_EQ(result, 100u);
}

/**
 * @tc.name: ConvertStringToUint32_0200
 * @tc.desc: ConvertStringToUint32 with boundary value "0".
 * @tc.type: FUNC
 */
HWTEST_F(ResourceConfigHelperTest, ConvertStringToUint32_0200, TestSize.Level1)
{
    uint32_t result = 1;
    EXPECT_TRUE(ResourceConfigHelper::ConvertStringToUint32("0", result));
    EXPECT_EQ(result, 0u);
}

/**
 * @tc.name: ConvertStringToUint32_0300
 * @tc.desc: ConvertStringToUint32 with UINT32_MAX boundary.
 * @tc.type: FUNC
 */
HWTEST_F(ResourceConfigHelperTest, ConvertStringToUint32_0300, TestSize.Level1)
{
    uint32_t result = 0;
    EXPECT_TRUE(ResourceConfigHelper::ConvertStringToUint32("4294967295", result));
    EXPECT_EQ(result, 4294967295u);
}

/**
 * @tc.name: ConvertStringToUint32_0400
 * @tc.desc: ConvertStringToUint32 with value exceeding UINT32_MAX must fail.
 * @tc.type: FUNC
 */
HWTEST_F(ResourceConfigHelperTest, ConvertStringToUint32_0400, TestSize.Level1)
{
    uint32_t result = 0;
    EXPECT_FALSE(ResourceConfigHelper::ConvertStringToUint32("4294967296", result));
}

/**
 * @tc.name: ConvertStringToUint32_0500
 * @tc.desc: Negative input "-1" must be rejected; stoi used to silently overflow to 4294967295.
 * @tc.type: FUNC
 */
HWTEST_F(ResourceConfigHelperTest, ConvertStringToUint32_0500, TestSize.Level1)
{
    uint32_t result = 0;
    EXPECT_FALSE(ResourceConfigHelper::ConvertStringToUint32("-1", result));
}

/**
 * @tc.name: ConvertStringToUint32_0600
 * @tc.desc: Value above INT_MAX but within uint32_t range must parse; stoi used to throw out_of_range.
 * @tc.type: FUNC
 */
HWTEST_F(ResourceConfigHelperTest, ConvertStringToUint32_0600, TestSize.Level1)
{
    uint32_t result = 0;
    EXPECT_TRUE(ResourceConfigHelper::ConvertStringToUint32("3000000000", result));
    EXPECT_EQ(result, 3000000000u);
}

/**
 * @tc.name: ConvertStringToUint32_0700
 * @tc.desc: Empty string must fail.
 * @tc.type: FUNC
 */
HWTEST_F(ResourceConfigHelperTest, ConvertStringToUint32_0700, TestSize.Level1)
{
    uint32_t result = 0;
    EXPECT_FALSE(ResourceConfigHelper::ConvertStringToUint32("", result));
}

/**
 * @tc.name: ConvertStringToUint32_0800
 * @tc.desc: Non-numeric string must fail.
 * @tc.type: FUNC
 */
HWTEST_F(ResourceConfigHelperTest, ConvertStringToUint32_0800, TestSize.Level1)
{
    uint32_t result = 0;
    EXPECT_FALSE(ResourceConfigHelper::ConvertStringToUint32("abc", result));
}

/**
 * @tc.name: ConvertStringToUint32_0900
 * @tc.desc: String with illegal trailing chars must fail (strict full consumption).
 * @tc.type: FUNC
 */
HWTEST_F(ResourceConfigHelperTest, ConvertStringToUint32_0900, TestSize.Level1)
{
    uint32_t result = 0;
    EXPECT_FALSE(ResourceConfigHelper::ConvertStringToUint32("123abc", result));
}

/**
 * @tc.name: ConvertStringToUint32_1000
 * @tc.desc: Leading whitespace must fail; from_chars is stricter than stoi (config values should not
 *           contain whitespace). This is an intentional behavior tightening.
 * @tc.type: FUNC
 */
HWTEST_F(ResourceConfigHelperTest, ConvertStringToUint32_1000, TestSize.Level1)
{
    uint32_t result = 0;
    EXPECT_FALSE(ResourceConfigHelper::ConvertStringToUint32(" 100", result));
}
} // namespace AbilityRuntime
} // namespace OHOS
