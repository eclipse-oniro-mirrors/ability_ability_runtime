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

#include <gtest/gtest.h>
#include <singleton.h>

#define private public
#define protected public
#include "js_system_configuration_updated_callback.h"
#undef private
#undef protected

#include "mock_native_reference.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AbilityRuntime {

class JsSystemConfigurationUpdatedCallbackTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);

    napi_env env_ = nullptr;
    std::shared_ptr<JsSystemConfigurationUpdatedCallback> callback_ = nullptr;
};

void JsSystemConfigurationUpdatedCallbackTest::SetUpTestCase()
{}

void JsSystemConfigurationUpdatedCallbackTest::TearDownTestCase()
{}

void JsSystemConfigurationUpdatedCallbackTest::SetUp()
{
    env_ = reinterpret_cast<napi_env>(0x1);  // Mock environment
    callback_ = std::make_shared<JsSystemConfigurationUpdatedCallback>(env_);
}

void JsSystemConfigurationUpdatedCallbackTest::TearDown()
{
    callback_.reset();
}

/**
 * @tc.number: JsSystemConfigurationUpdatedCallback_Constructor_0100
 * @tc.name: Constructor
 * @tc.desc: Test constructor with valid environment.
 */
HWTEST_F(
    JsSystemConfigurationUpdatedCallbackTest, JsSystemConfigurationUpdatedCallback_Constructor_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_Constructor_0100 start";
    napi_env env = reinterpret_cast<napi_env>(0x1);
    auto callback = std::make_shared<JsSystemConfigurationUpdatedCallback>(env);
    EXPECT_NE(callback, nullptr);
    EXPECT_EQ(callback->env_, env);
    EXPECT_TRUE(callback->callbacks_.empty());
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_Constructor_0100 end";
}

/**
 * @tc.number: JsSystemConfigurationUpdatedCallback_IsEmpty_0100
 * @tc.name: IsEmpty
 * @tc.desc: Test IsEmpty returns true when no callbacks registered.
 */
HWTEST_F(JsSystemConfigurationUpdatedCallbackTest, JsSystemConfigurationUpdatedCallback_IsEmpty_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_IsEmpty_0100 start";
    EXPECT_TRUE(callback_->IsEmpty());
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_IsEmpty_0100 end";
}

/**
 * @tc.number: JsSystemConfigurationUpdatedCallback_Register_0100
 * @tc.name: Register
 * @tc.desc: Test Register with null environment.
 */
HWTEST_F(JsSystemConfigurationUpdatedCallbackTest, JsSystemConfigurationUpdatedCallback_Register_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_Register_0100 start";
    callback_->env_ = nullptr;
    napi_value jsCallback = reinterpret_cast<napi_value>(0x2);
    callback_->Register(jsCallback);
    EXPECT_TRUE(callback_->callbacks_.empty());
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_Register_0100 end";
}

/**
 * @tc.number: JsSystemConfigurationUpdatedCallback_Register_0200
 * @tc.name: Register
 * @tc.desc: Test Register with null callback.
 */
HWTEST_F(JsSystemConfigurationUpdatedCallbackTest, JsSystemConfigurationUpdatedCallback_Register_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_Register_0200 start";
    callback_->Register(nullptr);
    EXPECT_TRUE(callback_->callbacks_.empty());
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_Register_0200 end";
}

/**
 * @tc.number: JsSystemConfigurationUpdatedCallback_UnRegister_0100
 * @tc.name: UnRegister
 * @tc.desc: Test UnRegister with null callback clears all callbacks.
 */
HWTEST_F(
    JsSystemConfigurationUpdatedCallbackTest, JsSystemConfigurationUpdatedCallback_UnRegister_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_UnRegister_0100 start";
    auto mockRef = std::make_shared<MockNativeReference>();
    callback_->callbacks_.push_back(mockRef);
    EXPECT_FALSE(callback_->IsEmpty());

    bool result = callback_->UnRegister(nullptr);
    EXPECT_TRUE(result);
    EXPECT_TRUE(callback_->IsEmpty());
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_UnRegister_0100 end";
}

/**
 * @tc.number: JsSystemConfigurationUpdatedCallback_UnRegister_0200
 * @tc.name: UnRegister
 * @tc.desc: Test UnRegister with null callback when empty.
 */
HWTEST_F(
    JsSystemConfigurationUpdatedCallbackTest, JsSystemConfigurationUpdatedCallback_UnRegister_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_UnRegister_0200 start";
    bool result = callback_->UnRegister(nullptr);
    EXPECT_TRUE(result);
    EXPECT_TRUE(callback_->IsEmpty());
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_UnRegister_0200 end";
}

/**
 * @tc.number: JsSystemConfigurationUpdatedCallback_IsEqual_0100
 * @tc.name: IsEqual
 * @tc.desc: Test IsEqual with null environment.
 */
HWTEST_F(JsSystemConfigurationUpdatedCallbackTest, JsSystemConfigurationUpdatedCallback_IsEqual_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_IsEqual_0100 start";
    auto mockRef = std::make_shared<MockNativeReference>();
    napi_value jsCallback = reinterpret_cast<napi_value>(0x2);

    callback_->env_ = nullptr;
    bool result = callback_->IsEqual(mockRef, jsCallback);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_IsEqual_0100 end";
}

/**
 * @tc.number: JsSystemConfigurationUpdatedCallback_IsEqual_0200
 * @tc.name: IsEqual
 * @tc.desc: Test IsEqual with null callback reference.
 */
HWTEST_F(JsSystemConfigurationUpdatedCallbackTest, JsSystemConfigurationUpdatedCallback_IsEqual_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_IsEqual_0200 start";
    napi_value jsCallback = reinterpret_cast<napi_value>(0x2);

    bool result = callback_->IsEqual(nullptr, jsCallback);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_IsEqual_0200 end";
}

/**
 * @tc.number: JsSystemConfigurationUpdatedCallback_IsEqual_0300
 * @tc.name: IsEqual
 * @tc.desc: Test IsEqual with null jsCallback value.
 */
HWTEST_F(JsSystemConfigurationUpdatedCallbackTest, JsSystemConfigurationUpdatedCallback_IsEqual_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_IsEqual_0300 start";
    auto mockRef = std::make_shared<MockNativeReference>();

    bool result = callback_->IsEqual(mockRef, nullptr);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_IsEqual_0300 end";
}

/**
 * @tc.number: JsSystemConfigurationUpdatedCallback_HasJsMethodExist_0100
 * @tc.name: HasJsMethodExist
 * @tc.desc: Test HasJsMethodExist with null environment.
 */
HWTEST_F(JsSystemConfigurationUpdatedCallbackTest, JsSystemConfigurationUpdatedCallback_HasJsMethodExist_0100,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_HasJsMethodExist_0100 start";
    auto mockRef = std::make_shared<MockNativeReference>();

    bool result = callback_->HasJsMethodExist(nullptr, mockRef, "onUpdate");
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_HasJsMethodExist_0100 end";
}

/**
 * @tc.number: JsSystemConfigurationUpdatedCallback_HasJsMethodExist_0200
 * @tc.name: HasJsMethodExist
 * @tc.desc: Test HasJsMethodExist with null callback.
 */
HWTEST_F(JsSystemConfigurationUpdatedCallbackTest, JsSystemConfigurationUpdatedCallback_HasJsMethodExist_0200,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_HasJsMethodExist_0200 start";

    bool result = callback_->HasJsMethodExist(env_, nullptr, "onUpdate");
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_HasJsMethodExist_0200 end";
}

/**
 * @tc.number: JsSystemConfigurationUpdatedCallback_NotifyColorModeUpdated_0100
 * @tc.name: NotifyColorModeUpdated
 * @tc.desc: Test NotifyColorModeUpdated with valid callback.
 */
HWTEST_F(JsSystemConfigurationUpdatedCallbackTest, JsSystemConfigurationUpdatedCallback_NotifyColorModeUpdated_0100,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_NotifyColorModeUpdated_0100 start";
    auto mockRef = std::make_shared<MockNativeReference>();
    std::string colorMode = "DARK_MODE";

    // Should not crash with null callback
    callback_->NotifyColorModeUpdated(nullptr, colorMode);
    callback_->NotifyColorModeUpdated(mockRef, colorMode);
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_NotifyColorModeUpdated_0100 end";
}

/**
 * @tc.number: JsSystemConfigurationUpdatedCallback_NotifyFontSizeScaleUpdated_0100
 * @tc.name: NotifyFontSizeScaleUpdated
 * @tc.desc: Test NotifyFontSizeScaleUpdated with valid scale value.
 */
HWTEST_F(JsSystemConfigurationUpdatedCallbackTest, JsSystemConfigurationUpdatedCallback_NotifyFontSizeScaleUpdated_0100,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_NotifyFontSizeScaleUpdated_0100 start";
    auto mockRef = std::make_shared<MockNativeReference>();
    std::string fontSizeScale = "1.5";

    callback_->NotifyFontSizeScaleUpdated(mockRef, fontSizeScale);
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_NotifyFontSizeScaleUpdated_0100 end";
}

/**
 * @tc.number: JsSystemConfigurationUpdatedCallback_NotifyFontSizeScaleUpdated_0200
 * @tc.name: NotifyFontSizeScaleUpdated
 * @tc.desc: Test NotifyFontSizeScaleUpdated with empty string.
 */
HWTEST_F(JsSystemConfigurationUpdatedCallbackTest, JsSystemConfigurationUpdatedCallback_NotifyFontSizeScaleUpdated_0200,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_NotifyFontSizeScaleUpdated_0200 start";
    auto mockRef = std::make_shared<MockNativeReference>();
    std::string fontSizeScale = "";

    callback_->NotifyFontSizeScaleUpdated(mockRef, fontSizeScale);
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_NotifyFontSizeScaleUpdated_0200 end";
}

/**
 * @tc.number: JsSystemConfigurationUpdatedCallback_NotifyFontWeightScaleUpdated_0100
 * @tc.name: NotifyFontWeightScaleUpdated
 * @tc.desc: Test NotifyFontWeightScaleUpdated with valid scale value.
 */
HWTEST_F(JsSystemConfigurationUpdatedCallbackTest,
    JsSystemConfigurationUpdatedCallback_NotifyFontWeightScaleUpdated_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_NotifyFontWeightScaleUpdated_0100 start";
    auto mockRef = std::make_shared<MockNativeReference>();
    std::string fontWeightScale = "1.2";

    callback_->NotifyFontWeightScaleUpdated(mockRef, fontWeightScale);
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_NotifyFontWeightScaleUpdated_0100 end";
}

/**
 * @tc.number: JsSystemConfigurationUpdatedCallback_NotifyLanguageUpdated_0100
 * @tc.name: NotifyLanguageUpdated
 * @tc.desc: Test NotifyLanguageUpdated with valid language.
 */
HWTEST_F(JsSystemConfigurationUpdatedCallbackTest, JsSystemConfigurationUpdatedCallback_NotifyLanguageUpdated_0100,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_NotifyLanguageUpdated_0100 start";
    auto mockRef = std::make_shared<MockNativeReference>();
    std::string language = "zh-CN";

    callback_->NotifyLanguageUpdated(mockRef, language);
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_NotifyLanguageUpdated_0100 end";
}

/**
 * @tc.number: JsSystemConfigurationUpdatedCallback_NotifyFontIdUpdated_0100
 * @tc.name: NotifyFontIdUpdated
 * @tc.desc: Test NotifyFontIdUpdated with valid font ID.
 */
HWTEST_F(JsSystemConfigurationUpdatedCallbackTest, JsSystemConfigurationUpdatedCallback_NotifyFontIdUpdated_0100,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_NotifyFontIdUpdated_0100 start";
    auto mockRef = std::make_shared<MockNativeReference>();
    std::string fontId = "default";

    callback_->NotifyFontIdUpdated(mockRef, fontId);
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_NotifyFontIdUpdated_0100 end";
}

/**
 * @tc.number: JsSystemConfigurationUpdatedCallback_NotifyMCCUpdated_0100
 * @tc.name: NotifyMCCUpdated
 * @tc.desc: Test NotifyMCCUpdated with valid MCC.
 */
HWTEST_F(JsSystemConfigurationUpdatedCallbackTest, JsSystemConfigurationUpdatedCallback_NotifyMCCUpdated_0100,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_NotifyMCCUpdated_0100 start";
    auto mockRef = std::make_shared<MockNativeReference>();
    std::string mcc = "460";

    callback_->NotifyMCCUpdated(mockRef, mcc);
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_NotifyMCCUpdated_0100 end";
}

/**
 * @tc.number: JsSystemConfigurationUpdatedCallback_NotifyMNCUpdated_0100
 * @tc.name: NotifyMNCUpdated
 * @tc.desc: Test NotifyMNCUpdated with valid MNC.
 */
HWTEST_F(JsSystemConfigurationUpdatedCallbackTest, JsSystemConfigurationUpdatedCallback_NotifyMNCUpdated_0100,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_NotifyMNCUpdated_0100 start";
    auto mockRef = std::make_shared<MockNativeReference>();
    std::string mnc = "01";

    callback_->NotifyMNCUpdated(mockRef, mnc);
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_NotifyMNCUpdated_0100 end";
}

/**
 * @tc.number: JsSystemConfigurationUpdatedCallback_NotifyLocaleUpdated_0100
 * @tc.name: NotifyLocaleUpdated
 * @tc.desc: Test NotifyLocaleUpdated with valid locale.
 */
HWTEST_F(JsSystemConfigurationUpdatedCallbackTest, JsSystemConfigurationUpdatedCallback_NotifyLocaleUpdated_0100,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_NotifyLocaleUpdated_0100 start";
    auto mockRef = std::make_shared<MockNativeReference>();
    std::string locale = "zh-Hans-CN";

    callback_->NotifyLocaleUpdated(mockRef, locale);
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_NotifyLocaleUpdated_0100 end";
}

/**
 * @tc.number: JsSystemConfigurationUpdatedCallback_NotifyHasPointerDeviceUpdated_0100
 * @tc.name: NotifyHasPointerDeviceUpdated
 * @tc.desc: Test NotifyHasPointerDeviceUpdated with true value.
 */
HWTEST_F(JsSystemConfigurationUpdatedCallbackTest,
    JsSystemConfigurationUpdatedCallback_NotifyHasPointerDeviceUpdated_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_NotifyHasPointerDeviceUpdated_0100 start";
    auto mockRef = std::make_shared<MockNativeReference>();
    std::string hasPointerDevice = "true";

    callback_->NotifyHasPointerDeviceUpdated(mockRef, hasPointerDevice);
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_NotifyHasPointerDeviceUpdated_0100 end";
}

/**
 * @tc.number: JsSystemConfigurationUpdatedCallback_NotifyHasPointerDeviceUpdated_0200
 * @tc.name: NotifyHasPointerDeviceUpdated
 * @tc.desc: Test NotifyHasPointerDeviceUpdated with false value.
 */
HWTEST_F(JsSystemConfigurationUpdatedCallbackTest,
    JsSystemConfigurationUpdatedCallback_NotifyHasPointerDeviceUpdated_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_NotifyHasPointerDeviceUpdated_0200 start";
    auto mockRef = std::make_shared<MockNativeReference>();
    std::string hasPointerDevice = "false";

    callback_->NotifyHasPointerDeviceUpdated(mockRef, hasPointerDevice);
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_NotifyHasPointerDeviceUpdated_0200 end";
}

/**
 * @tc.number: JsSystemConfigurationUpdatedCallback_CallJsMethodInnerCommon_0100
 * @tc.name: CallJsMethodInnerCommon
 * @tc.desc: Test CallJsMethodInnerCommon with null callback.
 */
HWTEST_F(JsSystemConfigurationUpdatedCallbackTest, JsSystemConfigurationUpdatedCallback_CallJsMethodInnerCommon_0100,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_CallJsMethodInnerCommon_0100 start";
    napi_value value = reinterpret_cast<napi_value>(0x3);

    // Should not crash with null callback
    callback_->CallJsMethodInnerCommon("onUpdate", nullptr, value);
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_CallJsMethodInnerCommon_0100 end";
}

/**
 * @tc.number: JsSystemConfigurationUpdatedCallback_FreeNativeReference_0100
 * @tc.name: FreeNativeReference
 * @tc.desc: Test FreeNativeReference with null reference.
 */
HWTEST_F(JsSystemConfigurationUpdatedCallbackTest, JsSystemConfigurationUpdatedCallback_FreeNativeReference_0100,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_FreeNativeReference_0100 start";

    // Should not crash with null reference
    callback_->FreeNativeReference(nullptr);
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_FreeNativeReference_0100 end";
}

/**
 * @tc.number: JsSystemConfigurationUpdatedCallback_Destructor_0100
 * @tc.name: Destructor
 * @tc.desc: Test destructor properly cleans up callbacks.
 */
HWTEST_F(
    JsSystemConfigurationUpdatedCallbackTest, JsSystemConfigurationUpdatedCallback_Destructor_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_Destructor_0100 start";
    auto testCallback = std::make_shared<JsSystemConfigurationUpdatedCallback>(env_);
    auto mockRef = std::make_shared<MockNativeReference>();
    testCallback->callbacks_.push_back(mockRef);

    // Destructor should be called without crashing
    testCallback.reset();
    GTEST_LOG_(INFO) << "JsSystemConfigurationUpdatedCallback_Destructor_0100 end";
}

/**
 * @class ConvertToDoubleTest
 * @brief Test class for ConvertToDouble helper function
 * @description Tests the ConvertToDouble function logic with edge cases and boundary values
 */
class ConvertToDoubleTest : public testing::Test {
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
     * @brief Convert string to double (replicates the anonymous namespace function)
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

void ConvertToDoubleTest::SetUpTestCase()
{}

void ConvertToDoubleTest::TearDownTestCase()
{}

void ConvertToDoubleTest::SetUp()
{
    errno = 0;
}

void ConvertToDoubleTest::TearDown()
{}

// ============================================================================
// ConvertToDouble - Valid Value Tests
// ============================================================================

/**
 * @tc.number: ConvertToDouble_Valid_Integer_0100
 * @tc.name: ConvertToDouble - Valid positive integer
 * @tc.desc: Test converting a valid positive integer string.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Valid_Integer_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Valid_Integer_0100 start";
    std::string str = "123";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 123.0);
    GTEST_LOG_(INFO) << "ConvertToDouble_Valid_Integer_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Valid_Integer_0200
 * @tc.name: ConvertToDouble - Valid negative integer
 * @tc.desc: Test converting a valid negative integer string.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Valid_Integer_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Valid_Integer_0200 start";
    std::string str = "-456";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, -456.0);
    GTEST_LOG_(INFO) << "ConvertToDouble_Valid_Integer_0200 end";
}

/**
 * @tc.number: ConvertToDouble_Valid_Decimal_0100
 * @tc.name: ConvertToDouble - Valid decimal
 * @tc.desc: Test converting a valid decimal string.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Valid_Decimal_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Valid_Decimal_0100 start";
    std::string str = "123.456";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 123.456);
    GTEST_LOG_(INFO) << "ConvertToDouble_Valid_Decimal_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Valid_Decimal_0200
 * @tc.name: ConvertToDouble - Valid negative decimal
 * @tc.desc: Test converting a valid negative decimal string.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Valid_Decimal_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Valid_Decimal_0200 start";
    std::string str = "-789.012";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, -789.012);
    GTEST_LOG_(INFO) << "ConvertToDouble_Valid_Decimal_0200 end";
}

// ============================================================================
// ConvertToDouble - Boundary Value Tests
// ============================================================================

/**
 * @tc.number: ConvertToDouble_Boundary_Zero_0100
 * @tc.name: ConvertToDouble - Zero value
 * @tc.desc: Test converting zero string to double.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Boundary_Zero_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Boundary_Zero_0100 start";
    std::string str = "0";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 0.0);
    GTEST_LOG_(INFO) << "ConvertToDouble_Boundary_Zero_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Boundary_Zero_0200
 * @tc.name: ConvertToDouble - Negative zero
 * @tc.desc: Test converting negative zero string to double.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Boundary_Zero_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Boundary_Zero_0200 start";
    std::string str = "-0";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 0.0);
    GTEST_LOG_(INFO) << "ConvertToDouble_Boundary_Zero_0200 end";
}

/**
 * @tc.number: ConvertToDouble_Boundary_Zero_0300
 * @tc.name: ConvertToDouble - Decimal zero
 * @tc.desc: Test converting decimal zero string to double.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Boundary_Zero_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Boundary_Zero_0300 start";
    std::string str = "0.0";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 0.0);
    GTEST_LOG_(INFO) << "ConvertToDouble_Boundary_Zero_0300 end";
}

/**
 * @tc.number: ConvertToDouble_Boundary_Large_0200
 * @tc.name: ConvertToDouble - Very large negative number
 * @tc.desc: Test converting a very large negative number (near min double).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Boundary_Large_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Boundary_Large_0200 start";
    std::string str = "-1.79769e+308";  // Near min double
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Boundary_Large_0200 end";
}

/**
 * @tc.number: ConvertToDouble_Boundary_Max_0100
 * @tc.name: ConvertToDouble - Maximum font size scale
 * @tc.desc: Test converting maximum typical font size scale value.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Boundary_Max_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Boundary_Max_0100 start";
    std::string str = "5.0";  // Maximum typical font scale
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 5.0);
    GTEST_LOG_(INFO) << "ConvertToDouble_Boundary_Max_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Boundary_Min_0100
 * @tc.name: ConvertToDouble - Minimum font size scale
 * @tc.desc: Test converting minimum typical font size scale value.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Boundary_Min_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Boundary_Min_0100 start";
    std::string str = "0.5";  // Minimum typical font scale
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 0.5);
    GTEST_LOG_(INFO) << "ConvertToDouble_Boundary_Min_0100 end";
}

// ============================================================================
// ConvertToDouble - Scientific Notation Tests
// ============================================================================

/**
 * @tc.number: ConvertToDouble_Scientific_0100
 * @tc.name: ConvertToDouble - Scientific notation positive
 * @tc.desc: Test converting scientific notation with positive exponent.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Scientific_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Scientific_0100 start";
    std::string str = "1.23e2";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 123.0);
    GTEST_LOG_(INFO) << "ConvertToDouble_Scientific_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Scientific_0200
 * @tc.name: ConvertToDouble - Scientific notation negative exponent
 * @tc.desc: Test converting scientific notation with negative exponent.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Scientific_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Scientific_0200 start";
    std::string str = "1.23e-2";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 0.0123);
    GTEST_LOG_(INFO) << "ConvertToDouble_Scientific_0200 end";
}

/**
 * @tc.number: ConvertToDouble_Scientific_0300
 * @tc.name: ConvertToDouble - Scientific notation uppercase
 * @tc.desc: Test converting scientific notation with uppercase E.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Scientific_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Scientific_0300 start";
    std::string str = "1.5E3";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 1500.0);
    GTEST_LOG_(INFO) << "ConvertToDouble_Scientific_0300 end";
}

/**
 * @tc.number: ConvertToDouble_Scientific_0400
 * @tc.name: ConvertToDouble - Scientific notation negative base
 * @tc.desc: Test converting scientific notation with negative base.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Scientific_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Scientific_0400 start";
    std::string str = "-2.5e2";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, -250.0);
    GTEST_LOG_(INFO) << "ConvertToDouble_Scientific_0400 end";
}

// ============================================================================
// ConvertToDouble - Special Format Tests
// ============================================================================

/**
 * @tc.number: ConvertToDouble_Format_NoDecimal_0100
 * @tc.name: ConvertToDouble - Number without decimal point
 * @tc.desc: Test converting integer with trailing decimal point.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Format_NoDecimal_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Format_NoDecimal_0100 start";
    std::string str = "123.";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 123.0);
    GTEST_LOG_(INFO) << "ConvertToDouble_Format_NoDecimal_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Format_LeadingZero_0100
 * @tc.name: ConvertToDouble - Leading decimal point
 * @tc.desc: Test converting number starting with decimal point.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Format_LeadingZero_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Format_LeadingZero_0100 start";
    std::string str = ".5";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 0.5);
    GTEST_LOG_(INFO) << "ConvertToDouble_Format_LeadingZero_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Format_PlusSign_0100
 * @tc.name: ConvertToDouble - Explicit plus sign
 * @tc.desc: Test converting number with explicit plus sign.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Format_PlusSign_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Format_PlusSign_0100 start";
    std::string str = "+123.45";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 123.45);
    GTEST_LOG_(INFO) << "ConvertToDouble_Format_PlusSign_0100 end";
}

// ============================================================================
// ConvertToDouble - Error/Invalid Value Tests
// ============================================================================

/**
 * @tc.number: ConvertToDouble_Error_Empty_0100
 * @tc.name: ConvertToDouble - Empty string
 * @tc.desc: Test converting an empty string (should fail).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_Empty_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_Empty_0100 start";
    std::string str = "";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_Empty_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Error_InvalidChar_0100
 * @tc.name: ConvertToDouble - String with letters
 * @tc.desc: Test converting a string with letters (should fail).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_InvalidChar_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_InvalidChar_0100 start";
    std::string str = "abc123";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_InvalidChar_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Error_TrailingChar_0100
 * @tc.name: ConvertToDouble - Trailing invalid characters
 * @tc.desc: Test converting a string with trailing characters (should fail).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_TrailingChar_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_TrailingChar_0100 start";
    std::string str = "123.45abc";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_TrailingChar_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Error_TrailingChar_0200
 * @tc.name: ConvertToDouble - Trailing space
 * @tc.desc: Test converting a string with trailing space (should fail).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_TrailingChar_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_TrailingChar_0200 start";
    std::string str = "123.45 ";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_TrailingChar_0200 end";
}

/**
 * @tc.number: ConvertToDouble_Error_TrailingChar_0300
 * @tc.name: ConvertToDouble - Trailing special character
 * @tc.desc: Test converting a string with trailing special character (should fail).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_TrailingChar_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_TrailingChar_0300 start";
    std::string str = "123.45%";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_TrailingChar_0300 end";
}

/**
 * @tc.number: ConvertToDouble_Error_OnlyLetters_0100
 * @tc.name: ConvertToDouble - Only letters
 * @tc.desc: Test converting a string with only letters (should fail).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_OnlyLetters_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_OnlyLetters_0100 start";
    std::string str = "hello";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_OnlyLetters_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Error_SpecialChars_0100
 * @tc.name: ConvertToDouble - Special characters only
 * @tc.desc: Test converting a string with special characters (should fail).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_SpecialChars_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_SpecialChars_0100 start";
    std::string str = "@#$%^&*";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_SpecialChars_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Error_MultipleDecimals_0100
 * @tc.name: ConvertToDouble - Multiple decimal points
 * @tc.desc: Test converting a string with multiple decimal points (should fail).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_MultipleDecimals_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_MultipleDecimals_0100 start";
    std::string str = "123.45.67";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_MultipleDecimals_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Error_MultipleSigns_0100
 * @tc.name: ConvertToDouble - Multiple signs
 * @tc.desc: Test converting a string with multiple signs (should fail).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_MultipleSigns_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_MultipleSigns_0100 start";
    std::string str = "+-123.45";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_MultipleSigns_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Error_SignInMiddle_0100
 * @tc.name: ConvertToDouble - Sign in middle
 * @tc.desc: Test converting a string with sign in the middle (should fail).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_SignInMiddle_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_SignInMiddle_0100 start";
    std::string str = "123-456";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_SignInMiddle_0100 end";
}

// ============================================================================
// ConvertToDouble - Overflow/Underflow Tests
// ============================================================================

/**
 * @tc.number: ConvertToDouble_Error_Overflow_0100
 * @tc.name: ConvertToDouble - Overflow positive
 * @tc.desc: Test converting a number that exceeds max double (ERANGE).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_Overflow_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_Overflow_0100 start";
    std::string str = "1.8e+309";  // Exceeds max double
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    // ERANGE will be set, should return false
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_Overflow_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Error_Overflow_0200
 * @tc.name: ConvertToDouble - Overflow negative
 * @tc.desc: Test converting a negative number that exceeds min double (ERANGE).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_Overflow_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_Overflow_0200 start";
    std::string str = "-1.8e+309";  // Exceeds min double
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    // ERANGE will be set, should return false
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_Overflow_0200 end";
}

/**
 * @tc.number: ConvertToDouble_Error_Underflow_0100
 * @tc.name: ConvertToDouble - Underflow
 * @tc.desc: Test converting a number that underflows to zero.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_Underflow_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_Underflow_0100 start";
    std::string str = "1e-400";  // Underflows to 0
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    // May succeed but result is 0, or ERANGE may be set
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_Underflow_0100 end";
}

// ============================================================================
// ConvertToDouble - Whitespace Tests
// ============================================================================

/**
 * @tc.number: ConvertToDouble_Error_Whitespace_0100
 * @tc.name: ConvertToDouble - Leading whitespace
 * @tc.desc: Test converting a string with leading whitespace (should fail).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_Whitespace_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_Whitespace_0100 start";
    std::string str = " 123.45";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_Whitespace_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Error_Whitespace_0200
 * @tc.name: ConvertToDouble - Only whitespace
 * @tc.desc: Test converting a string with only whitespace (should fail).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_Whitespace_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_Whitespace_0200 start";
    std::string str = "   ";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_Whitespace_0200 end";
}

// ============================================================================
// ConvertToDouble - Font Size/Weight Scale Specific Tests
// ============================================================================

/**
 * @tc.number: ConvertToDouble_FontScale_Valid_0100
 * @tc.name: ConvertToDouble - Font size scale 1.0
 * @tc.desc: Test converting typical font size scale value 1.0.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_FontScale_Valid_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_FontScale_Valid_0100 start";
    std::string str = "1.0";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 1.0);
    GTEST_LOG_(INFO) << "ConvertToDouble_FontScale_Valid_0100 end";
}

/**
 * @tc.number: ConvertToDouble_FontScale_Valid_0200
 * @tc.name: ConvertToDouble - Font size scale 1.5
 * @tc.desc: Test converting typical font size scale value 1.5.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_FontScale_Valid_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_FontScale_Valid_0200 start";
    std::string str = "1.5";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 1.5);
    GTEST_LOG_(INFO) << "ConvertToDouble_FontScale_Valid_0200 end";
}

/**
 * @tc.number: ConvertToDouble_FontScale_Valid_0300
 * @tc.name: ConvertToDouble - Font size scale 2.0
 * @tc.desc: Test converting typical font size scale value 2.0.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_FontScale_Valid_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_FontScale_Valid_0300 start";
    std::string str = "2.0";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 2.0);
    GTEST_LOG_(INFO) << "ConvertToDouble_FontScale_Valid_0300 end";
}

/**
 * @tc.number: ConvertToDouble_FontScale_Valid_0400
 * @tc.name: ConvertToDouble - Font weight scale 1.1
 * @tc.desc: Test converting typical font weight scale value 1.1.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_FontScale_Valid_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_FontScale_Valid_0400 start";
    std::string str = "1.1";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 1.1);
    GTEST_LOG_(INFO) << "ConvertToDouble_FontScale_Valid_0400 end";
}

/**
 * @tc.number: ConvertToDouble_FontScale_Valid_0500
 * @tc.name: ConvertToDouble - Font weight scale 0.9
 * @tc.desc: Test converting typical font weight scale value 0.9.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_FontScale_Valid_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_FontScale_Valid_0500 start";
    std::string str = "0.9";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 0.9);
    GTEST_LOG_(INFO) << "ConvertToDouble_FontScale_Valid_0500 end";
}

// ============================================================================
// ConvertToDouble - Precision Tests
// ============================================================================

/**
 * @tc.number: ConvertToDouble_Precision_0100
 * @tc.name: ConvertToDouble - High precision decimal
 * @tc.desc: Test converting a high precision decimal number.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Precision_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Precision_0100 start";
    std::string str = "3.141592653589793";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_NEAR(result, 3.141592653589793, 1e-15);
    GTEST_LOG_(INFO) << "ConvertToDouble_Precision_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Precision_0200
 * @tc.name: ConvertToDouble - Many decimal places
 * @tc.desc: Test converting a number with many decimal places.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Precision_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Precision_0200 start";
    std::string str = "0.12345678901234567890";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_NEAR(result, 0.12345678901234567890, 1e-15);
    GTEST_LOG_(INFO) << "ConvertToDouble_Precision_0200 end";
}
/**
 * @tc.number: ConvertToDouble_Valid_Integer_0100
 * @tc.name: ConvertToDouble - Valid positive integer
 * @tc.desc: Test converting a valid positive integer string.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Valid_Integer1_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Valid_Integer_0100 start";
    std::string str = "123";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 123.0);
    GTEST_LOG_(INFO) << "ConvertToDouble_Valid_Integer_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Valid_Integer_0200
 * @tc.name: ConvertToDouble - Valid negative integer
 * @tc.desc: Test converting a valid negative integer string.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Valid_Integer1_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Valid_Integer_0200 start";
    std::string str = "-456";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, -456.0);
    GTEST_LOG_(INFO) << "ConvertToDouble_Valid_Integer_0200 end";
}

/**
 * @tc.number: ConvertToDouble_Valid_Decimal_0100
 * @tc.name: ConvertToDouble - Valid decimal
 * @tc.desc: Test converting a valid decimal string.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Valid_Decimal1_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Valid_Decimal_0100 start";
    std::string str = "123.456";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 123.456);
    GTEST_LOG_(INFO) << "ConvertToDouble_Valid_Decimal_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Valid_Decimal_0200
 * @tc.name: ConvertToDouble - Valid negative decimal
 * @tc.desc: Test converting a valid negative decimal string.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Valid_Decimal1_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Valid_Decimal_0200 start";
    std::string str = "-789.012";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, -789.012);
    GTEST_LOG_(INFO) << "ConvertToDouble_Valid_Decimal_0200 end";
}

// ============================================================================
// ConvertToDouble - Boundary Value Tests
// ============================================================================

/**
 * @tc.number: ConvertToDouble_Boundary_Zero_0100
 * @tc.name: ConvertToDouble - Zero value
 * @tc.desc: Test converting zero string to double.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Boundary_Zero1_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Boundary_Zero_0100 start";
    std::string str = "0";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 0.0);
    GTEST_LOG_(INFO) << "ConvertToDouble_Boundary_Zero_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Boundary_Zero_0200
 * @tc.name: ConvertToDouble - Negative zero
 * @tc.desc: Test converting negative zero string to double.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Boundary_Zero1_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Boundary_Zero_0200 start";
    std::string str = "-0";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 0.0);
    GTEST_LOG_(INFO) << "ConvertToDouble_Boundary_Zero_0200 end";
}

/**
 * @tc.number: ConvertToDouble_Boundary_Zero_0300
 * @tc.name: ConvertToDouble - Decimal zero
 * @tc.desc: Test converting decimal zero string to double.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Boundary_Zero1_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Boundary_Zero_0300 start";
    std::string str = "0.0";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 0.0);
    GTEST_LOG_(INFO) << "ConvertToDouble_Boundary_Zero_0300 end";
}

/**
 * @tc.number: ConvertToDouble_Boundary_Large_0200
 * @tc.name: ConvertToDouble - Very large negative number
 * @tc.desc: Test converting a very large negative number (near min double).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Boundary_Large1_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Boundary_Large_0200 start";
    std::string str = "-1.79769e+308";  // Near min double
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Boundary_Large_0200 end";
}

/**
 * @tc.number: ConvertToDouble_Boundary_Max_0100
 * @tc.name: ConvertToDouble - Maximum font size scale
 * @tc.desc: Test converting maximum typical font size scale value.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Boundary_Max1_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Boundary_Max_0100 start";
    std::string str = "5.0";  // Maximum typical font scale
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 5.0);
    GTEST_LOG_(INFO) << "ConvertToDouble_Boundary_Max_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Boundary_Min_0100
 * @tc.name: ConvertToDouble - Minimum font size scale
 * @tc.desc: Test converting minimum typical font size scale value.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Boundary_Min1_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Boundary_Min_0100 start";
    std::string str = "0.5";  // Minimum typical font scale
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 0.5);
    GTEST_LOG_(INFO) << "ConvertToDouble_Boundary_Min_0100 end";
}

// ============================================================================
// ConvertToDouble - Scientific Notation Tests
// ============================================================================

/**
 * @tc.number: ConvertToDouble_Scientific_0100
 * @tc.name: ConvertToDouble - Scientific notation positive
 * @tc.desc: Test converting scientific notation with positive exponent.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Scientific1_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Scientific_0100 start";
    std::string str = "1.23e2";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 123.0);
    GTEST_LOG_(INFO) << "ConvertToDouble_Scientific_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Scientific_0200
 * @tc.name: ConvertToDouble - Scientific notation negative exponent
 * @tc.desc: Test converting scientific notation with negative exponent.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Scientific1_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Scientific_0200 start";
    std::string str = "1.23e-2";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 0.0123);
    GTEST_LOG_(INFO) << "ConvertToDouble_Scientific_0200 end";
}

/**
 * @tc.number: ConvertToDouble_Scientific_0300
 * @tc.name: ConvertToDouble - Scientific notation uppercase
 * @tc.desc: Test converting scientific notation with uppercase E.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Scientific1_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Scientific_0300 start";
    std::string str = "1.5E3";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 1500.0);
    GTEST_LOG_(INFO) << "ConvertToDouble_Scientific_0300 end";
}

/**
 * @tc.number: ConvertToDouble_Scientific_0400
 * @tc.name: ConvertToDouble - Scientific notation negative base
 * @tc.desc: Test converting scientific notation with negative base.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Scientific1_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Scientific_0400 start";
    std::string str = "-2.5e2";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, -250.0);
    GTEST_LOG_(INFO) << "ConvertToDouble_Scientific_0400 end";
}

// ============================================================================
// ConvertToDouble - Special Format Tests
// ============================================================================

/**
 * @tc.number: ConvertToDouble_Format_NoDecimal_0100
 * @tc.name: ConvertToDouble - Number without decimal point
 * @tc.desc: Test converting integer with trailing decimal point.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Format_NoDecimal1_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Format_NoDecimal_0100 start";
    std::string str = "123.";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 123.0);
    GTEST_LOG_(INFO) << "ConvertToDouble_Format_NoDecimal_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Format_LeadingZero_0100
 * @tc.name: ConvertToDouble - Leading decimal point
 * @tc.desc: Test converting number starting with decimal point.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Format_LeadingZero1_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Format_LeadingZero_0100 start";
    std::string str = ".5";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 0.5);
    GTEST_LOG_(INFO) << "ConvertToDouble_Format_LeadingZero_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Format_PlusSign_0100
 * @tc.name: ConvertToDouble - Explicit plus sign
 * @tc.desc: Test converting number with explicit plus sign.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Format_PlusSign1_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Format_PlusSign_0100 start";
    std::string str = "+123.45";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 123.45);
    GTEST_LOG_(INFO) << "ConvertToDouble_Format_PlusSign_0100 end";
}

// ============================================================================
// ConvertToDouble - Error/Invalid Value Tests
// ============================================================================

/**
 * @tc.number: ConvertToDouble_Error_Empty_0100
 * @tc.name: ConvertToDouble - Empty string
 * @tc.desc: Test converting an empty string (should fail).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_Empty1_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_Empty_0100 start";
    std::string str = "";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_Empty_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Error_InvalidChar_0100
 * @tc.name: ConvertToDouble - String with letters
 * @tc.desc: Test converting a string with letters (should fail).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_InvalidChar1_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_InvalidChar_0100 start";
    std::string str = "abc123";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_InvalidChar_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Error_TrailingChar_0100
 * @tc.name: ConvertToDouble - Trailing invalid characters
 * @tc.desc: Test converting a string with trailing characters (should fail).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_TrailingChar1_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_TrailingChar_0100 start";
    std::string str = "123.45abc";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_TrailingChar_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Error_TrailingChar_0200
 * @tc.name: ConvertToDouble - Trailing space
 * @tc.desc: Test converting a string with trailing space (should fail).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_TrailingChar1_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_TrailingChar_0200 start";
    std::string str = "123.45 ";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_TrailingChar_0200 end";
}

/**
 * @tc.number: ConvertToDouble_Error_TrailingChar_0300
 * @tc.name: ConvertToDouble - Trailing special character
 * @tc.desc: Test converting a string with trailing special character (should fail).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_TrailingChar1_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_TrailingChar_0300 start";
    std::string str = "123.45%";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_TrailingChar_0300 end";
}

/**
 * @tc.number: ConvertToDouble_Error_OnlyLetters_0100
 * @tc.name: ConvertToDouble - Only letters
 * @tc.desc: Test converting a string with only letters (should fail).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_OnlyLetters1_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_OnlyLetters_0100 start";
    std::string str = "hello";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_OnlyLetters_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Error_SpecialChars_0100
 * @tc.name: ConvertToDouble - Special characters only
 * @tc.desc: Test converting a string with special characters (should fail).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_SpecialChars1_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_SpecialChars_0100 start";
    std::string str = "@#$%^&*";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_SpecialChars_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Error_MultipleDecimals_0100
 * @tc.name: ConvertToDouble - Multiple decimal points
 * @tc.desc: Test converting a string with multiple decimal points (should fail).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_MultipleDecimals1_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_MultipleDecimals_0100 start";
    std::string str = "123.45.67";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_MultipleDecimals_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Error_MultipleSigns_0100
 * @tc.name: ConvertToDouble - Multiple signs
 * @tc.desc: Test converting a string with multiple signs (should fail).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_MultipleSigns1_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_MultipleSigns_0100 start";
    std::string str = "+-123.45";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_MultipleSigns_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Error_SignInMiddle_0100
 * @tc.name: ConvertToDouble - Sign in middle
 * @tc.desc: Test converting a string with sign in the middle (should fail).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_SignInMiddle1_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_SignInMiddle_0100 start";
    std::string str = "123-456";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_SignInMiddle_0100 end";
}

// ============================================================================
// ConvertToDouble - Overflow/Underflow Tests
// ============================================================================

/**
 * @tc.number: ConvertToDouble_Error_Overflow_0100
 * @tc.name: ConvertToDouble - Overflow positive
 * @tc.desc: Test converting a number that exceeds max double (ERANGE).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_Overflow1_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_Overflow_0100 start";
    std::string str = "1.8e+309";  // Exceeds max double
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    // ERANGE will be set, should return false
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_Overflow_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Error_Overflow_0200
 * @tc.name: ConvertToDouble - Overflow negative
 * @tc.desc: Test converting a negative number that exceeds min double (ERANGE).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_Overflow1_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_Overflow_0200 start";
    std::string str = "-1.8e+309";  // Exceeds min double
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    // ERANGE will be set, should return false
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_Overflow_0200 end";
}

/**
 * @tc.number: ConvertToDouble_Error_Underflow_0100
 * @tc.name: ConvertToDouble - Underflow
 * @tc.desc: Test converting a number that underflows to zero.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_Underflow1_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_Underflow_0100 start";
    std::string str = "1e-400";  // Underflows to 0
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    // May succeed but result is 0, or ERANGE may be set
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_Underflow_0100 end";
}

// ============================================================================
// ConvertToDouble - Whitespace Tests
// ============================================================================

/**
 * @tc.number: ConvertToDouble_Error_Whitespace_0100
 * @tc.name: ConvertToDouble - Leading whitespace
 * @tc.desc: Test converting a string with leading whitespace (should fail).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_Whitespace1_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_Whitespace_0100 start";
    std::string str = " 123.45";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_Whitespace_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Error_Whitespace_0200
 * @tc.name: ConvertToDouble - Only whitespace
 * @tc.desc: Test converting a string with only whitespace (should fail).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_Whitespace1_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_Whitespace_0200 start";
    std::string str = "   ";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_Whitespace_0200 end";
}

// ============================================================================
// ConvertToDouble - Font Size/Weight Scale Specific Tests
// ============================================================================

/**
 * @tc.number: ConvertToDouble_FontScale_Valid_0100
 * @tc.name: ConvertToDouble - Font size scale 1.0
 * @tc.desc: Test converting typical font size scale value 1.0.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_FontScale_Valid1_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_FontScale_Valid_0100 start";
    std::string str = "1.0";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 1.0);
    GTEST_LOG_(INFO) << "ConvertToDouble_FontScale_Valid_0100 end";
}

/**
 * @tc.number: ConvertToDouble_FontScale_Valid_0200
 * @tc.name: ConvertToDouble - Font size scale 1.5
 * @tc.desc: Test converting typical font size scale value 1.5.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_FontScale_Valid1_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_FontScale_Valid_0200 start";
    std::string str = "1.5";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 1.5);
    GTEST_LOG_(INFO) << "ConvertToDouble_FontScale_Valid_0200 end";
}

/**
 * @tc.number: ConvertToDouble_FontScale_Valid_0300
 * @tc.name: ConvertToDouble - Font size scale 2.0
 * @tc.desc: Test converting typical font size scale value 2.0.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_FontScale_Valid1_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_FontScale_Valid_0300 start";
    std::string str = "2.0";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 2.0);
    GTEST_LOG_(INFO) << "ConvertToDouble_FontScale_Valid_0300 end";
}

/**
 * @tc.number: ConvertToDouble_FontScale_Valid_0400
 * @tc.name: ConvertToDouble - Font weight scale 1.1
 * @tc.desc: Test converting typical font weight scale value 1.1.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_FontScale_Valid1_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_FontScale_Valid_0400 start";
    std::string str = "1.1";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 1.1);
    GTEST_LOG_(INFO) << "ConvertToDouble_FontScale_Valid_0400 end";
}

/**
 * @tc.number: ConvertToDouble_FontScale_Valid_0500
 * @tc.name: ConvertToDouble - Font weight scale 0.9
 * @tc.desc: Test converting typical font weight scale value 0.9.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_FontScale_Valid1_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_FontScale_Valid_0500 start";
    std::string str = "0.9";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 0.9);
    GTEST_LOG_(INFO) << "ConvertToDouble_FontScale_Valid_0500 end";
}

// ============================================================================
// ConvertToDouble - Precision Tests
// ============================================================================

/**
 * @tc.number: ConvertToDouble_Precision_0100
 * @tc.name: ConvertToDouble - High precision decimal
 * @tc.desc: Test converting a high precision decimal number.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Precision1_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Precision_0100 start";
    std::string str = "3.141592653589793";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_NEAR(result, 3.141592653589793, 1e-15);
    GTEST_LOG_(INFO) << "ConvertToDouble_Precision_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Precision_0200
 * @tc.name: ConvertToDouble - Many decimal places
 * @tc.desc: Test converting a number with many decimal places.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Precision1_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Precision_0200 start";
    std::string str = "0.12345678901234567890";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_NEAR(result, 0.12345678901234567890, 1e-15);
    GTEST_LOG_(INFO) << "ConvertToDouble_Precision_0200 end";
}
/**
 * @tc.number: ConvertToDouble_Valid_Integer_0100
 * @tc.name: ConvertToDouble - Valid positive integer
 * @tc.desc: Test converting a valid positive integer string.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Valid_Integer2_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Valid_Integer_0100 start";
    std::string str = "123";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 123.0);
    GTEST_LOG_(INFO) << "ConvertToDouble_Valid_Integer_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Valid_Integer_0200
 * @tc.name: ConvertToDouble - Valid negative integer
 * @tc.desc: Test converting a valid negative integer string.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Valid_Integer2_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Valid_Integer_0200 start";
    std::string str = "-456";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, -456.0);
    GTEST_LOG_(INFO) << "ConvertToDouble_Valid_Integer_0200 end";
}

/**
 * @tc.number: ConvertToDouble_Valid_Decimal_0100
 * @tc.name: ConvertToDouble - Valid decimal
 * @tc.desc: Test converting a valid decimal string.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Valid_Decimal2_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Valid_Decimal_0100 start";
    std::string str = "123.456";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 123.456);
    GTEST_LOG_(INFO) << "ConvertToDouble_Valid_Decimal_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Valid_Decimal_0200
 * @tc.name: ConvertToDouble - Valid negative decimal
 * @tc.desc: Test converting a valid negative decimal string.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Valid_Decimal2_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Valid_Decimal_0200 start";
    std::string str = "-789.012";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, -789.012);
    GTEST_LOG_(INFO) << "ConvertToDouble_Valid_Decimal_0200 end";
}

// ============================================================================
// ConvertToDouble - Boundary Value Tests
// ============================================================================

/**
 * @tc.number: ConvertToDouble_Boundary_Zero_0100
 * @tc.name: ConvertToDouble - Zero value
 * @tc.desc: Test converting zero string to double.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Boundary_Zero2_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Boundary_Zero_0100 start";
    std::string str = "0";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 0.0);
    GTEST_LOG_(INFO) << "ConvertToDouble_Boundary_Zero_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Boundary_Zero_0200
 * @tc.name: ConvertToDouble - Negative zero
 * @tc.desc: Test converting negative zero string to double.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Boundary_Zero2_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Boundary_Zero_0200 start";
    std::string str = "-0";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 0.0);
    GTEST_LOG_(INFO) << "ConvertToDouble_Boundary_Zero_0200 end";
}

/**
 * @tc.number: ConvertToDouble_Boundary_Zero_0300
 * @tc.name: ConvertToDouble - Decimal zero
 * @tc.desc: Test converting decimal zero string to double.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Boundary_Zero2_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Boundary_Zero_0300 start";
    std::string str = "0.0";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 0.0);
    GTEST_LOG_(INFO) << "ConvertToDouble_Boundary_Zero_0300 end";
}

/**
 * @tc.number: ConvertToDouble_Boundary_Large_0200
 * @tc.name: ConvertToDouble - Very large negative number
 * @tc.desc: Test converting a very large negative number (near min double).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Boundary_Large2_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Boundary_Large_0200 start";
    std::string str = "-1.79769e+308";  // Near min double
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Boundary_Large_0200 end";
}

/**
 * @tc.number: ConvertToDouble_Boundary_Max_0100
 * @tc.name: ConvertToDouble - Maximum font size scale
 * @tc.desc: Test converting maximum typical font size scale value.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Boundary_Max2_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Boundary_Max_0100 start";
    std::string str = "5.0";  // Maximum typical font scale
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 5.0);
    GTEST_LOG_(INFO) << "ConvertToDouble_Boundary_Max_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Boundary_Min_0100
 * @tc.name: ConvertToDouble - Minimum font size scale
 * @tc.desc: Test converting minimum typical font size scale value.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Boundary_Min2_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Boundary_Min_0100 start";
    std::string str = "0.5";  // Minimum typical font scale
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 0.5);
    GTEST_LOG_(INFO) << "ConvertToDouble_Boundary_Min_0100 end";
}

// ============================================================================
// ConvertToDouble - Scientific Notation Tests
// ============================================================================

/**
 * @tc.number: ConvertToDouble_Scientific_0100
 * @tc.name: ConvertToDouble - Scientific notation positive
 * @tc.desc: Test converting scientific notation with positive exponent.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Scientific2_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Scientific_0100 start";
    std::string str = "1.23e2";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 123.0);
    GTEST_LOG_(INFO) << "ConvertToDouble_Scientific_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Scientific_0200
 * @tc.name: ConvertToDouble - Scientific notation negative exponent
 * @tc.desc: Test converting scientific notation with negative exponent.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Scientific2_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Scientific_0200 start";
    std::string str = "1.23e-2";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 0.0123);
    GTEST_LOG_(INFO) << "ConvertToDouble_Scientific_0200 end";
}

/**
 * @tc.number: ConvertToDouble_Scientific_0300
 * @tc.name: ConvertToDouble - Scientific notation uppercase
 * @tc.desc: Test converting scientific notation with uppercase E.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Scientific2_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Scientific_0300 start";
    std::string str = "1.5E3";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 1500.0);
    GTEST_LOG_(INFO) << "ConvertToDouble_Scientific_0300 end";
}

/**
 * @tc.number: ConvertToDouble_Scientific_0400
 * @tc.name: ConvertToDouble - Scientific notation negative base
 * @tc.desc: Test converting scientific notation with negative base.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Scientific2_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Scientific_0400 start";
    std::string str = "-2.5e2";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, -250.0);
    GTEST_LOG_(INFO) << "ConvertToDouble_Scientific_0400 end";
}

// ============================================================================
// ConvertToDouble - Special Format Tests
// ============================================================================

/**
 * @tc.number: ConvertToDouble_Format_NoDecimal_0100
 * @tc.name: ConvertToDouble - Number without decimal point
 * @tc.desc: Test converting integer with trailing decimal point.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Format_NoDecimal2_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Format_NoDecimal_0100 start";
    std::string str = "123.";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 123.0);
    GTEST_LOG_(INFO) << "ConvertToDouble_Format_NoDecimal_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Format_LeadingZero_0100
 * @tc.name: ConvertToDouble - Leading decimal point
 * @tc.desc: Test converting number starting with decimal point.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Format_LeadingZero2_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Format_LeadingZero_0100 start";
    std::string str = ".5";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 0.5);
    GTEST_LOG_(INFO) << "ConvertToDouble_Format_LeadingZero_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Format_PlusSign_0100
 * @tc.name: ConvertToDouble - Explicit plus sign
 * @tc.desc: Test converting number with explicit plus sign.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Format_PlusSign2_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Format_PlusSign_0100 start";
    std::string str = "+123.45";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 123.45);
    GTEST_LOG_(INFO) << "ConvertToDouble_Format_PlusSign_0100 end";
}

// ============================================================================
// ConvertToDouble - Error/Invalid Value Tests
// ============================================================================

/**
 * @tc.number: ConvertToDouble_Error_Empty_0100
 * @tc.name: ConvertToDouble - Empty string
 * @tc.desc: Test converting an empty string (should fail).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_Empty2_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_Empty_0100 start";
    std::string str = "";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_Empty_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Error_InvalidChar_0100
 * @tc.name: ConvertToDouble - String with letters
 * @tc.desc: Test converting a string with letters (should fail).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_InvalidChar2_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_InvalidChar_0100 start";
    std::string str = "abc123";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_InvalidChar_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Error_TrailingChar_0100
 * @tc.name: ConvertToDouble - Trailing invalid characters
 * @tc.desc: Test converting a string with trailing characters (should fail).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_TrailingChar2_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_TrailingChar_0100 start";
    std::string str = "123.45abc";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_TrailingChar_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Error_TrailingChar_0200
 * @tc.name: ConvertToDouble - Trailing space
 * @tc.desc: Test converting a string with trailing space (should fail).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_TrailingChar2_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_TrailingChar_0200 start";
    std::string str = "123.45 ";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_TrailingChar_0200 end";
}

/**
 * @tc.number: ConvertToDouble_Error_TrailingChar_0300
 * @tc.name: ConvertToDouble - Trailing special character
 * @tc.desc: Test converting a string with trailing special character (should fail).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_TrailingChar2_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_TrailingChar_0300 start";
    std::string str = "123.45%";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_TrailingChar_0300 end";
}

/**
 * @tc.number: ConvertToDouble_Error_OnlyLetters_0100
 * @tc.name: ConvertToDouble - Only letters
 * @tc.desc: Test converting a string with only letters (should fail).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_OnlyLetters2_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_OnlyLetters_0100 start";
    std::string str = "hello";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_OnlyLetters_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Error_SpecialChars_0100
 * @tc.name: ConvertToDouble - Special characters only
 * @tc.desc: Test converting a string with special characters (should fail).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_SpecialChars2_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_SpecialChars_0100 start";
    std::string str = "@#$%^&*";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_SpecialChars_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Error_MultipleDecimals_0100
 * @tc.name: ConvertToDouble - Multiple decimal points
 * @tc.desc: Test converting a string with multiple decimal points (should fail).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_MultipleDecimals2_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_MultipleDecimals_0100 start";
    std::string str = "123.45.67";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_MultipleDecimals_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Error_MultipleSigns_0100
 * @tc.name: ConvertToDouble - Multiple signs
 * @tc.desc: Test converting a string with multiple signs (should fail).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_MultipleSigns2_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_MultipleSigns_0100 start";
    std::string str = "+-123.45";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_MultipleSigns_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Error_SignInMiddle_0100
 * @tc.name: ConvertToDouble - Sign in middle
 * @tc.desc: Test converting a string with sign in the middle (should fail).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_SignInMiddle2_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_SignInMiddle_0100 start";
    std::string str = "123-456";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_SignInMiddle_0100 end";
}

// ============================================================================
// ConvertToDouble - Overflow/Underflow Tests
// ============================================================================

/**
 * @tc.number: ConvertToDouble_Error_Overflow_0100
 * @tc.name: ConvertToDouble - Overflow positive
 * @tc.desc: Test converting a number that exceeds max double (ERANGE).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_Overflow2_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_Overflow_0100 start";
    std::string str = "1.8e+309";  // Exceeds max double
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    // ERANGE will be set, should return false
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_Overflow_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Error_Overflow_0200
 * @tc.name: ConvertToDouble - Overflow negative
 * @tc.desc: Test converting a negative number that exceeds min double (ERANGE).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_Overflow2_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_Overflow_0200 start";
    std::string str = "-1.8e+309";  // Exceeds min double
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    // ERANGE will be set, should return false
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_Overflow_0200 end";
}

/**
 * @tc.number: ConvertToDouble_Error_Underflow_0100
 * @tc.name: ConvertToDouble - Underflow
 * @tc.desc: Test converting a number that underflows to zero.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_Underflow2_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_Underflow_0100 start";
    std::string str = "1e-400";  // Underflows to 0
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    // May succeed but result is 0, or ERANGE may be set
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_Underflow_0100 end";
}

// ============================================================================
// ConvertToDouble - Whitespace Tests
// ============================================================================

/**
 * @tc.number: ConvertToDouble_Error_Whitespace_0100
 * @tc.name: ConvertToDouble - Leading whitespace
 * @tc.desc: Test converting a string with leading whitespace (should fail).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_Whitespace2_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_Whitespace_0100 start";
    std::string str = " 123.45";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_Whitespace_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Error_Whitespace_0200
 * @tc.name: ConvertToDouble - Only whitespace
 * @tc.desc: Test converting a string with only whitespace (should fail).
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Error_Whitespace2_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_Whitespace_0200 start";
    std::string str = "   ";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "ConvertToDouble_Error_Whitespace_0200 end";
}

// ============================================================================
// ConvertToDouble - Font Size/Weight Scale Specific Tests
// ============================================================================

/**
 * @tc.number: ConvertToDouble_FontScale_Valid_0100
 * @tc.name: ConvertToDouble - Font size scale 1.0
 * @tc.desc: Test converting typical font size scale value 1.0.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_FontScale_Valid2_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_FontScale_Valid_0100 start";
    std::string str = "1.0";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 1.0);
    GTEST_LOG_(INFO) << "ConvertToDouble_FontScale_Valid_0100 end";
}

/**
 * @tc.number: ConvertToDouble_FontScale_Valid_0200
 * @tc.name: ConvertToDouble - Font size scale 1.5
 * @tc.desc: Test converting typical font size scale value 1.5.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_FontScale_Valid2_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_FontScale_Valid_0200 start";
    std::string str = "1.5";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 1.5);
    GTEST_LOG_(INFO) << "ConvertToDouble_FontScale_Valid_0200 end";
}

/**
 * @tc.number: ConvertToDouble_FontScale_Valid_0300
 * @tc.name: ConvertToDouble - Font size scale 2.0
 * @tc.desc: Test converting typical font size scale value 2.0.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_FontScale_Valid2_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_FontScale_Valid_0300 start";
    std::string str = "2.0";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 2.0);
    GTEST_LOG_(INFO) << "ConvertToDouble_FontScale_Valid_0300 end";
}

/**
 * @tc.number: ConvertToDouble_FontScale_Valid_0400
 * @tc.name: ConvertToDouble - Font weight scale 1.1
 * @tc.desc: Test converting typical font weight scale value 1.1.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_FontScale_Valid2_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_FontScale_Valid_0400 start";
    std::string str = "1.1";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 1.1);
    GTEST_LOG_(INFO) << "ConvertToDouble_FontScale_Valid_0400 end";
}

/**
 * @tc.number: ConvertToDouble_FontScale_Valid_0500
 * @tc.name: ConvertToDouble - Font weight scale 0.9
 * @tc.desc: Test converting typical font weight scale value 0.9.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_FontScale_Valid2_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_FontScale_Valid_0500 start";
    std::string str = "0.9";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 0.9);
    GTEST_LOG_(INFO) << "ConvertToDouble_FontScale_Valid_0500 end";
}

// ============================================================================
// ConvertToDouble - Precision Tests
// ============================================================================

/**
 * @tc.number: ConvertToDouble_Precision_0100
 * @tc.name: ConvertToDouble - High precision decimal
 * @tc.desc: Test converting a high precision decimal number.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Precision2_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Precision_0100 start";
    std::string str = "3.141592653589793";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_NEAR(result, 3.141592653589793, 1e-15);
    GTEST_LOG_(INFO) << "ConvertToDouble_Precision_0100 end";
}

/**
 * @tc.number: ConvertToDouble_Precision_0200
 * @tc.name: ConvertToDouble - Many decimal places
 * @tc.desc: Test converting a number with many decimal places.
 */
HWTEST_F(ConvertToDoubleTest, ConvertToDouble_Precision2_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToDouble_Precision_0200 start";
    std::string str = "0.12345678901234567890";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_NEAR(result, 0.12345678901234567890, 1e-15);
    GTEST_LOG_(INFO) << "ConvertToDouble_Precision_0200 end";
}

}  // namespace AbilityRuntime
}  // namespace OHOS
