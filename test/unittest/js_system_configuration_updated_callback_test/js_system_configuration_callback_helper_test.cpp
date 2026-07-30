/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

/**
 * @file js_system_configuration_callback_helper_test.cpp
 * @brief Unit tests for helper functions in js_system_configuration_updated_callback.cpp
 * @description This file contains tests for the utility functions used in the callback system.
 *              Since these functions are in an anonymous namespace, we test equivalent logic here.
 */

#include <gtest/gtest.h>
#include <cerrno>
#include <cstdlib>
#include <string>

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {

/**
 * @brief Test utility class for string to double conversion validation
 * @description Replicates the logic of IsValidValue and ConvertToDouble functions
 *             from js_system_configuration_updated_callback.cpp for testing purposes.
 */
class StringConversionUtilsTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;
    static void SetUpTestCase();
    static void TearDownTestCase();

    /**
     * @brief Check if the parsed value is valid
     * @param end Pointer to the end of the parsed value
     * @param str Original string
     * @return true if valid, false otherwise
     */
    static bool IsValidValue(const char *end, const std::string &str)
    {
        if (!end) {
            return false;
        }

        if (end == str.c_str() || errno == ERANGE || *end != '\0') {
            return false;
        }
        return true;
    }

    /**
     * @brief Convert string to double
     * @param str Input string
     * @param outValue Output value
     * @return true if conversion successful, false otherwise
     */
    static bool ConvertToDouble(const std::string &str, double &outValue)
    {
        if (str.empty()) {
            return false;
        }
        char *end = nullptr;
        errno = 0;
        double value = std::strtod(str.c_str(), &end);
        if (!IsValidValue(end, str)) {
            return false;
        }
        outValue = value;
        return true;
    }
};

void StringConversionUtilsTest::SetUpTestCase()
{}

void StringConversionUtilsTest::TearDownTestCase()
{}

void StringConversionUtilsTest::SetUp()
{
    errno = 0;
}

void StringConversionUtilsTest::TearDown()
{}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0100
 * @tc.name: ConvertToDouble - Valid integer string
 * @tc.desc: Test converting a valid integer string to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0100 start";
    std::string str = "123";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 123.0);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0100 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0200
 * @tc.name: ConvertToDouble - Valid decimal string
 * @tc.desc: Test converting a valid decimal string to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0200 start";
    std::string str = "123.456";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 123.456);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0200 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0300
 * @tc.name: ConvertToDouble - Negative number
 * @tc.desc: Test converting a negative number string to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0300 start";
    std::string str = "-99.99";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, -99.99);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0300 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0400
 * @tc.name: ConvertToDouble - Empty string
 * @tc.desc: Test converting an empty string to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0400 start";
    std::string str = "";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0400 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0500
 * @tc.name: ConvertToDouble - Invalid string with letters
 * @tc.desc: Test converting an invalid string with letters to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0500 start";
    std::string str = "abc123";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0500 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0600
 * @tc.name: ConvertToDouble - String with trailing characters
 * @tc.desc: Test converting a string with trailing characters to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0600 start";
    std::string str = "123.45abc";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0600 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0700
 * @tc.name: ConvertToDouble - Scientific notation
 * @tc.desc: Test converting scientific notation string to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0700 start";
    std::string str = "1.23e2";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 123.0);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0700 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0800
 * @tc.name: ConvertToDouble - Zero value
 * @tc.desc: Test converting zero string to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble_0800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0800 start";
    std::string str = "0";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 0.0);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0800 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0900
 * @tc.name: ConvertToDouble - Decimal zero
 * @tc.desc: Test converting decimal zero string to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble_0900, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0900 start";
    std::string str = "0.0";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 0.0);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0900 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_1300
 * @tc.name: ConvertToDouble - Font size scale typical value
 * @tc.desc: Test converting typical font size scale value.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble_1300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_1300 start";
    std::string str = "1.5";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 1.5);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_1300 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_1400
 * @tc.name: ConvertToDouble - Font weight scale typical value
 * @tc.desc: Test converting typical font weight scale value.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble_1400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_1400 start";
    std::string str = "1.0";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 1.0);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_1400 end";
}

/**
 * @tc.number: StringConversionUtils_IsValidValue_0100
 * @tc.name: IsValidValue - Null end pointer
 * @tc.desc: Test IsValidValue with null end pointer.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_IsValidValue_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0100 start";
    std::string str = "123";
    bool ret = IsValidValue(nullptr, str);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0100 end";
}

/**
 * @tc.number: StringConversionUtils_IsValidValue_0200
 * @tc.name: IsValidValue - End equals string start
 * @tc.desc: Test IsValidValue when end pointer equals string start (no conversion).
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_IsValidValue_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0200 start";
    std::string str = "abc";
    bool ret = IsValidValue(str.c_str(), str);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0200 end";
}

/**
 * @tc.number: StringConversionUtils_IsValidValue_0300
 * @tc.name: IsValidValue - ERANGE error
 * @tc.desc: Test IsValidValue when errno is set to ERANGE.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_IsValidValue_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0300 start";
    std::string str = "123";
    errno = ERANGE;
    bool ret = IsValidValue(str.c_str() + str.length(), str);
    EXPECT_FALSE(ret);
    errno = 0;  // Reset errno
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0300 end";
}

/**
 * @tc.number: StringConversionUtils_IsValidValue_0400
 * @tc.name: IsValidValue - Valid conversion
 * @tc.desc: Test IsValidValue with a valid conversion.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_IsValidValue_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0400 start";
    std::string str = "123.45";
    bool ret = IsValidValue(str.c_str() + str.length(), str);
    EXPECT_TRUE(ret);
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0400 end";
}

/**
 * @tc.number: StringConversionUtils_IsValidValue_0500
 * @tc.name: IsValidValue - Partial conversion
 * @tc.desc: Test IsValidValue with partial string conversion.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_IsValidValue_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0500 start";
    std::string str = "123abc";
    bool ret = IsValidValue(str.c_str() + 3, str);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0500 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0100
 * @tc.name: ConvertToDouble - Valid integer string
 * @tc.desc: Test converting a valid integer string to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble1_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0100 start";
    std::string str = "123";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 123.0);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0100 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0200
 * @tc.name: ConvertToDouble - Valid decimal string
 * @tc.desc: Test converting a valid decimal string to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble1_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0200 start";
    std::string str = "123.456";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 123.456);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0200 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0300
 * @tc.name: ConvertToDouble - Negative number
 * @tc.desc: Test converting a negative number string to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble1_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0300 start";
    std::string str = "-99.99";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, -99.99);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0300 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0400
 * @tc.name: ConvertToDouble - Empty string
 * @tc.desc: Test converting an empty string to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble1_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0400 start";
    std::string str = "";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0400 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0500
 * @tc.name: ConvertToDouble - Invalid string with letters
 * @tc.desc: Test converting an invalid string with letters to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble1_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0500 start";
    std::string str = "abc123";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0500 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0600
 * @tc.name: ConvertToDouble - String with trailing characters
 * @tc.desc: Test converting a string with trailing characters to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble1_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0600 start";
    std::string str = "123.45abc";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0600 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0700
 * @tc.name: ConvertToDouble - Scientific notation
 * @tc.desc: Test converting scientific notation string to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble1_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0700 start";
    std::string str = "1.23e2";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 123.0);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0700 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0800
 * @tc.name: ConvertToDouble - Zero value
 * @tc.desc: Test converting zero string to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble1_0800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0800 start";
    std::string str = "0";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 0.0);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0800 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0900
 * @tc.name: ConvertToDouble - Decimal zero
 * @tc.desc: Test converting decimal zero string to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble1_0900, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0900 start";
    std::string str = "0.0";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 0.0);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0900 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_1300
 * @tc.name: ConvertToDouble - Font size scale typical value
 * @tc.desc: Test converting typical font size scale value.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble1_1300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_1300 start";
    std::string str = "1.5";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 1.5);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_1300 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_1400
 * @tc.name: ConvertToDouble - Font weight scale typical value
 * @tc.desc: Test converting typical font weight scale value.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble1_1400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_1400 start";
    std::string str = "1.0";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 1.0);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_1400 end";
}

/**
 * @tc.number: StringConversionUtils_IsValidValue_0100
 * @tc.name: IsValidValue - Null end pointer
 * @tc.desc: Test IsValidValue with null end pointer.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_IsValidValue1_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0100 start";
    std::string str = "123";
    bool ret = IsValidValue(nullptr, str);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0100 end";
}

/**
 * @tc.number: StringConversionUtils_IsValidValue_0200
 * @tc.name: IsValidValue - End equals string start
 * @tc.desc: Test IsValidValue when end pointer equals string start (no conversion).
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_IsValidValue1_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0200 start";
    std::string str = "abc";
    bool ret = IsValidValue(str.c_str(), str);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0200 end";
}

/**
 * @tc.number: StringConversionUtils_IsValidValue_0300
 * @tc.name: IsValidValue - ERANGE error
 * @tc.desc: Test IsValidValue when errno is set to ERANGE.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_IsValidValue1_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0300 start";
    std::string str = "123";
    errno = ERANGE;
    bool ret = IsValidValue(str.c_str() + str.length(), str);
    EXPECT_FALSE(ret);
    errno = 0;  // Reset errno
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0300 end";
}

/**
 * @tc.number: StringConversionUtils_IsValidValue_0400
 * @tc.name: IsValidValue - Valid conversion
 * @tc.desc: Test IsValidValue with a valid conversion.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_IsValidValue1_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0400 start";
    std::string str = "123.45";
    bool ret = IsValidValue(str.c_str() + str.length(), str);
    EXPECT_TRUE(ret);
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0400 end";
}

/**
 * @tc.number: StringConversionUtils_IsValidValue_0500
 * @tc.name: IsValidValue - Partial conversion
 * @tc.desc: Test IsValidValue with partial string conversion.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_IsValidValue1_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0500 start";
    std::string str = "123abc";
    bool ret = IsValidValue(str.c_str() + 3, str);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0500 end";
}


/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0100
 * @tc.name: ConvertToDouble - Valid integer string
 * @tc.desc: Test converting a valid integer string to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble2_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0100 start";
    std::string str = "123";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 123.0);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0100 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0200
 * @tc.name: ConvertToDouble - Valid decimal string
 * @tc.desc: Test converting a valid decimal string to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble2_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0200 start";
    std::string str = "123.456";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 123.456);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0200 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0300
 * @tc.name: ConvertToDouble - Negative number
 * @tc.desc: Test converting a negative number string to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble2_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0300 start";
    std::string str = "-99.99";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, -99.99);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0300 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0400
 * @tc.name: ConvertToDouble - Empty string
 * @tc.desc: Test converting an empty string to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble2_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0400 start";
    std::string str = "";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0400 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0500
 * @tc.name: ConvertToDouble - Invalid string with letters
 * @tc.desc: Test converting an invalid string with letters to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble2_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0500 start";
    std::string str = "abc123";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0500 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0600
 * @tc.name: ConvertToDouble - String with trailing characters
 * @tc.desc: Test converting a string with trailing characters to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble2_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0600 start";
    std::string str = "123.45abc";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0600 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0700
 * @tc.name: ConvertToDouble - Scientific notation
 * @tc.desc: Test converting scientific notation string to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble2_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0700 start";
    std::string str = "1.23e2";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 123.0);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0700 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0800
 * @tc.name: ConvertToDouble - Zero value
 * @tc.desc: Test converting zero string to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble2_0800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0800 start";
    std::string str = "0";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 0.0);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0800 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0900
 * @tc.name: ConvertToDouble - Decimal zero
 * @tc.desc: Test converting decimal zero string to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble2_0900, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0900 start";
    std::string str = "0.0";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 0.0);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0900 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_1300
 * @tc.name: ConvertToDouble - Font size scale typical value
 * @tc.desc: Test converting typical font size scale value.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble2_1300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_1300 start";
    std::string str = "1.5";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 1.5);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_1300 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_1400
 * @tc.name: ConvertToDouble - Font weight scale typical value
 * @tc.desc: Test converting typical font weight scale value.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble2_1400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_1400 start";
    std::string str = "1.0";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 1.0);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_1400 end";
}

/**
 * @tc.number: StringConversionUtils_IsValidValue_0100
 * @tc.name: IsValidValue - Null end pointer
 * @tc.desc: Test IsValidValue with null end pointer.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_IsValidValue2_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0100 start";
    std::string str = "123";
    bool ret = IsValidValue(nullptr, str);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0100 end";
}

/**
 * @tc.number: StringConversionUtils_IsValidValue_0200
 * @tc.name: IsValidValue - End equals string start
 * @tc.desc: Test IsValidValue when end pointer equals string start (no conversion).
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_IsValidValue2_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0200 start";
    std::string str = "abc";
    bool ret = IsValidValue(str.c_str(), str);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0200 end";
}

/**
 * @tc.number: StringConversionUtils_IsValidValue_0300
 * @tc.name: IsValidValue - ERANGE error
 * @tc.desc: Test IsValidValue when errno is set to ERANGE.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_IsValidValue2_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0300 start";
    std::string str = "123";
    errno = ERANGE;
    bool ret = IsValidValue(str.c_str() + str.length(), str);
    EXPECT_FALSE(ret);
    errno = 0;  // Reset errno
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0300 end";
}

/**
 * @tc.number: StringConversionUtils_IsValidValue_0400
 * @tc.name: IsValidValue - Valid conversion
 * @tc.desc: Test IsValidValue with a valid conversion.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_IsValidValue2_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0400 start";
    std::string str = "123.45";
    bool ret = IsValidValue(str.c_str() + str.length(), str);
    EXPECT_TRUE(ret);
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0400 end";
}

/**
 * @tc.number: StringConversionUtils_IsValidValue_0500
 * @tc.name: IsValidValue - Partial conversion
 * @tc.desc: Test IsValidValue with partial string conversion.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_IsValidValue2_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0500 start";
    std::string str = "123abc";
    bool ret = IsValidValue(str.c_str() + 3, str);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0500 end";
}


/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0100
 * @tc.name: ConvertToDouble - Valid integer string
 * @tc.desc: Test converting a valid integer string to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble3_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0100 start";
    std::string str = "123";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 123.0);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0100 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0200
 * @tc.name: ConvertToDouble - Valid decimal string
 * @tc.desc: Test converting a valid decimal string to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble3_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0200 start";
    std::string str = "123.456";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 123.456);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0200 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0300
 * @tc.name: ConvertToDouble - Negative number
 * @tc.desc: Test converting a negative number string to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble3_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0300 start";
    std::string str = "-99.99";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, -99.99);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0300 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0400
 * @tc.name: ConvertToDouble - Empty string
 * @tc.desc: Test converting an empty string to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble3_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0400 start";
    std::string str = "";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0400 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0500
 * @tc.name: ConvertToDouble - Invalid string with letters
 * @tc.desc: Test converting an invalid string with letters to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble3_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0500 start";
    std::string str = "abc123";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0500 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0600
 * @tc.name: ConvertToDouble - String with trailing characters
 * @tc.desc: Test converting a string with trailing characters to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble3_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0600 start";
    std::string str = "123.45abc";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0600 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0700
 * @tc.name: ConvertToDouble - Scientific notation
 * @tc.desc: Test converting scientific notation string to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble3_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0700 start";
    std::string str = "1.23e2";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 123.0);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0700 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0800
 * @tc.name: ConvertToDouble - Zero value
 * @tc.desc: Test converting zero string to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble3_0800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0800 start";
    std::string str = "0";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 0.0);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0800 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0900
 * @tc.name: ConvertToDouble - Decimal zero
 * @tc.desc: Test converting decimal zero string to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble3_0900, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0900 start";
    std::string str = "0.0";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 0.0);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0900 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_1300
 * @tc.name: ConvertToDouble - Font size scale typical value
 * @tc.desc: Test converting typical font size scale value.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble3_1300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_1300 start";
    std::string str = "1.5";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 1.5);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_1300 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_1400
 * @tc.name: ConvertToDouble - Font weight scale typical value
 * @tc.desc: Test converting typical font weight scale value.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble3_1400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_1400 start";
    std::string str = "1.0";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 1.0);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_1400 end";
}

/**
 * @tc.number: StringConversionUtils_IsValidValue_0100
 * @tc.name: IsValidValue - Null end pointer
 * @tc.desc: Test IsValidValue with null end pointer.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_IsValidValue3_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0100 start";
    std::string str = "123";
    bool ret = IsValidValue(nullptr, str);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0100 end";
}

/**
 * @tc.number: StringConversionUtils_IsValidValue_0200
 * @tc.name: IsValidValue - End equals string start
 * @tc.desc: Test IsValidValue when end pointer equals string start (no conversion).
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_IsValidValue3_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0200 start";
    std::string str = "abc";
    bool ret = IsValidValue(str.c_str(), str);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0200 end";
}

/**
 * @tc.number: StringConversionUtils_IsValidValue_0300
 * @tc.name: IsValidValue - ERANGE error
 * @tc.desc: Test IsValidValue when errno is set to ERANGE.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_IsValidValue3_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0300 start";
    std::string str = "123";
    errno = ERANGE;
    bool ret = IsValidValue(str.c_str() + str.length(), str);
    EXPECT_FALSE(ret);
    errno = 0;  // Reset errno
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0300 end";
}

/**
 * @tc.number: StringConversionUtils_IsValidValue_0400
 * @tc.name: IsValidValue - Valid conversion
 * @tc.desc: Test IsValidValue with a valid conversion.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_IsValidValue3_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0400 start";
    std::string str = "123.45";
    bool ret = IsValidValue(str.c_str() + str.length(), str);
    EXPECT_TRUE(ret);
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0400 end";
}

/**
 * @tc.number: StringConversionUtils_IsValidValue_0500
 * @tc.name: IsValidValue - Partial conversion
 * @tc.desc: Test IsValidValue with partial string conversion.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_IsValidValue3_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0500 start";
    std::string str = "123abc";
    bool ret = IsValidValue(str.c_str() + 3, str);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0500 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0100
 * @tc.name: ConvertToDouble - Valid integer string
 * @tc.desc: Test converting a valid integer string to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble4_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0100 start";
    std::string str = "123";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 123.0);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0100 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0200
 * @tc.name: ConvertToDouble - Valid decimal string
 * @tc.desc: Test converting a valid decimal string to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble4_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0200 start";
    std::string str = "123.456";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 123.456);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0200 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0300
 * @tc.name: ConvertToDouble - Negative number
 * @tc.desc: Test converting a negative number string to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble4_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0300 start";
    std::string str = "-99.99";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, -99.99);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0300 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0400
 * @tc.name: ConvertToDouble - Empty string
 * @tc.desc: Test converting an empty string to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble4_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0400 start";
    std::string str = "";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0400 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0500
 * @tc.name: ConvertToDouble - Invalid string with letters
 * @tc.desc: Test converting an invalid string with letters to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble4_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0500 start";
    std::string str = "abc123";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0500 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0600
 * @tc.name: ConvertToDouble - String with trailing characters
 * @tc.desc: Test converting a string with trailing characters to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble4_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0600 start";
    std::string str = "123.45abc";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0600 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0700
 * @tc.name: ConvertToDouble - Scientific notation
 * @tc.desc: Test converting scientific notation string to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble4_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0700 start";
    std::string str = "1.23e2";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 123.0);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0700 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0800
 * @tc.name: ConvertToDouble - Zero value
 * @tc.desc: Test converting zero string to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble4_0800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0800 start";
    std::string str = "0";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 0.0);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0800 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_0900
 * @tc.name: ConvertToDouble - Decimal zero
 * @tc.desc: Test converting decimal zero string to double.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble4_0900, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0900 start";
    std::string str = "0.0";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 0.0);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_0900 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_1300
 * @tc.name: ConvertToDouble - Font size scale typical value
 * @tc.desc: Test converting typical font size scale value.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble4_1300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_1300 start";
    std::string str = "1.5";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 1.5);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_1300 end";
}

/**
 * @tc.number: StringConversionUtils_ConvertToDouble_1400
 * @tc.name: ConvertToDouble - Font weight scale typical value
 * @tc.desc: Test converting typical font weight scale value.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_ConvertToDouble4_1400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_1400 start";
    std::string str = "1.0";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 1.0);
    GTEST_LOG_(INFO) << "StringConversionUtils_ConvertToDouble_1400 end";
}

/**
 * @tc.number: StringConversionUtils_IsValidValue_0100
 * @tc.name: IsValidValue - Null end pointer
 * @tc.desc: Test IsValidValue with null end pointer.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_IsValidValue4_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0100 start";
    std::string str = "123";
    bool ret = IsValidValue(nullptr, str);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0100 end";
}

/**
 * @tc.number: StringConversionUtils_IsValidValue_0200
 * @tc.name: IsValidValue - End equals string start
 * @tc.desc: Test IsValidValue when end pointer equals string start (no conversion).
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_IsValidValue4_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0200 start";
    std::string str = "abc";
    bool ret = IsValidValue(str.c_str(), str);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0200 end";
}

/**
 * @tc.number: StringConversionUtils_IsValidValue_0300
 * @tc.name: IsValidValue - ERANGE error
 * @tc.desc: Test IsValidValue when errno is set to ERANGE.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_IsValidValue4_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0300 start";
    std::string str = "123";
    errno = ERANGE;
    bool ret = IsValidValue(str.c_str() + str.length(), str);
    EXPECT_FALSE(ret);
    errno = 0;  // Reset errno
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0300 end";
}

/**
 * @tc.number: StringConversionUtils_IsValidValue_0400
 * @tc.name: IsValidValue - Valid conversion
 * @tc.desc: Test IsValidValue with a valid conversion.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_IsValidValue4_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0400 start";
    std::string str = "123.45";
    bool ret = IsValidValue(str.c_str() + str.length(), str);
    EXPECT_TRUE(ret);
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0400 end";
}

/**
 * @tc.number: StringConversionUtils_IsValidValue_0500
 * @tc.name: IsValidValue - Partial conversion
 * @tc.desc: Test IsValidValue with partial string conversion.
 */
HWTEST_F(StringConversionUtilsTest, StringConversionUtils_IsValidValue4_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0500 start";
    std::string str = "123abc";
    bool ret = IsValidValue(str.c_str() + 3, str);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "StringConversionUtils_IsValidValue_0500 end";
}
}  // namespace AbilityRuntime
}  // namespace OHOS
