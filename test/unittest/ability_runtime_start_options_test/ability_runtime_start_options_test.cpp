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

#include "start_options.h"

#include <cstring>
#include "hilog_tag_wrapper.h"
#ifdef START_WINDOW_OPTIONS_WITH_PIXELMAP
#include "pixelmap_native_impl.h"
#endif
#include "start_options_impl.h"

using namespace testing::ext;
namespace OHOS {
namespace AAFwk {
constexpr int MAX_SUPPOPRT_WINDOW_MODES_SIZE = 10;

class AbilityRuntimeStartOptionsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void AbilityRuntimeStartOptionsTest::SetUpTestCase()
{}

void AbilityRuntimeStartOptionsTest::TearDownTestCase()
{}

void AbilityRuntimeStartOptionsTest::SetUp()
{}

void AbilityRuntimeStartOptionsTest::TearDown()
{}

/**
 * @tc.name: OH_AbilityRuntime_CreateStartOptions_001
 * @tc.desc: OH_AbilityRuntime_CreateStartOptions
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_CreateStartOptions_001, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions* startOptions = OH_AbilityRuntime_CreateStartOptions();

    // Act & Assert
    ASSERT_NE(startOptions, nullptr);
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_DestroyStartOptions_001
 * @tc.desc: OH_AbilityRuntime_DestroyStartOptions
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_DestroyStartOptions_001, TestSize.Level1)
{
    AbilityRuntime_StartOptions* startOptions = nullptr;
    EXPECT_EQ(OH_AbilityRuntime_DestroyStartOptions(&startOptions), ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(startOptions, nullptr);
}

/**
 * @tc.name: OH_AbilityRuntime_DestroyStartOptions_002
 * @tc.desc: OH_AbilityRuntime_DestroyStartOptions
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_DestroyStartOptions_002, TestSize.Level1)
{
    AbilityRuntime_StartOptions* startOptions = OH_AbilityRuntime_CreateStartOptions();
    EXPECT_EQ(OH_AbilityRuntime_DestroyStartOptions(&startOptions), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(startOptions, nullptr);
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsWindowMode_001
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsWindowMode
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsWindowMode_001, TestSize.Level1)
{
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsWindowMode(nullptr,
        ABILITY_RUNTIME_WINDOW_MODE_FULL_SCREEN);
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, result);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsWindowMode_002
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsWindowMode
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsWindowMode_002, TestSize.Level1)
{
    AbilityRuntime_StartOptions* startOptions = OH_AbilityRuntime_CreateStartOptions();
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsWindowMode(startOptions,
        ABILITY_RUNTIME_WINDOW_MODE_FULL_SCREEN);
    // Assuming SetStartOptionsWindowMode returns ABILITY_RUNTIME_OK on success
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsWindowMode_003
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsWindowMode
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsWindowMode_003, TestSize.Level1)
{
    AbilityRuntime_StartOptions* startOptions = OH_AbilityRuntime_CreateStartOptions();
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsWindowMode(startOptions,
        static_cast<AbilityRuntime_WindowMode>(-1));
    // Assuming ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID is returned for invalid window mode
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, result);
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsWindowModeValue_001
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsWindowModeValue
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsWindowModeValue_001, TestSize.Level2)
{
    AbilityRuntime_WindowMode windowMode;
    AbilityRuntime_ErrorCode errorCode = OH_AbilityRuntime_GetStartOptionsWindowModeValue(nullptr, &windowMode);
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, errorCode);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsWindowModeValue_002
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsWindowModeValue
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsWindowModeValue_002, TestSize.Level2)
{
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsWindowMode(startOptions,
        ABILITY_RUNTIME_WINDOW_MODE_FULL_SCREEN);
    // Assuming SetStartOptionsWindowMode returns ABILITY_RUNTIME_OK on success
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);

    AbilityRuntime_WindowMode windowMode;
    AbilityRuntime_ErrorCode errorCode = OH_AbilityRuntime_GetStartOptionsWindowModeValue(startOptions, &windowMode);
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, errorCode);
    EXPECT_EQ(ABILITY_RUNTIME_WINDOW_MODE_FULL_SCREEN, windowMode);
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsWindowModeReference_001
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsWindowMode with C++ reference output
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsWindowModeReference_001, TestSize.Level2)
{
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsWindowMode(startOptions,
        ABILITY_RUNTIME_WINDOW_MODE_FULL_SCREEN);
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);

    AbilityRuntime_WindowMode windowMode;
    AbilityRuntime_ErrorCode errorCode = OH_AbilityRuntime_GetStartOptionsWindowMode(startOptions, windowMode);
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, errorCode);
    EXPECT_EQ(ABILITY_RUNTIME_WINDOW_MODE_FULL_SCREEN, windowMode);
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsDisplayId_001
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsDisplayId
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsDisplayId_001, TestSize.Level1)
{
    AbilityRuntime_StartOptions* startOptions = nullptr;
    int32_t displayId = 1;
    EXPECT_EQ(OH_AbilityRuntime_SetStartOptionsDisplayId(startOptions, displayId),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsDisplayId_002
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsDisplayId
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsDisplayId_002, TestSize.Level1)
{
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    int32_t displayId = 1;
    EXPECT_EQ(OH_AbilityRuntime_SetStartOptionsDisplayId(startOptions, displayId),
        startOptions->SetStartOptionsDisplayId(displayId));
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsDisplayId_003
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsDisplayId
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsDisplayId_003, TestSize.Level1)
{
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    int32_t displayId = -1;
    EXPECT_EQ(OH_AbilityRuntime_SetStartOptionsDisplayId(startOptions, displayId),
        startOptions->SetStartOptionsDisplayId(displayId));
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsDisplayId_004
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsDisplayId
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsDisplayId_004, TestSize.Level1)
{
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    int32_t displayId = 0;
    EXPECT_EQ(OH_AbilityRuntime_SetStartOptionsDisplayId(startOptions, displayId),
        startOptions->SetStartOptionsDisplayId(displayId));
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsDisplayIdValue_001
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsDisplayIdValue
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsDisplayIdValue_001, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = nullptr;
    int32_t displayId;

    // Act
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_GetStartOptionsDisplayIdValue(startOptions, &displayId);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, result);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsDisplayIdValue_002
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsDisplayIdValue
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsDisplayIdValue_002, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsDisplayId(startOptions, 1);
    // Assuming OH_AbilityRuntime_SetStartOptionsWindowMode returns success
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);

    int32_t displayId;

    // Act
    result = OH_AbilityRuntime_GetStartOptionsDisplayIdValue(startOptions, &displayId);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);
    EXPECT_EQ(1, displayId);
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsDisplayIdValue_003
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsDisplayIdValue
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsDisplayIdValue_003, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsDisplayId(startOptions, -1);
    // Assuming OH_AbilityRuntime_SetStartOptionsWindowMode returns success
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);

    int32_t displayId;

    // Act
    result = OH_AbilityRuntime_GetStartOptionsDisplayIdValue(startOptions, &displayId);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);
    EXPECT_EQ(-1, displayId);
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsWithAnimation_001
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsWithAnimation
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsWithAnimation_001, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = nullptr;
    bool withAnimation = true;

    // Act
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsWithAnimation(startOptions, withAnimation);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, result);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsWithAnimation_002
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsWithAnimation
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsWithAnimation_002, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();

    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsWithAnimation(startOptions, true);

    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);

    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsWithAnimation_003
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsWithAnimation
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsWithAnimation_003, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();

    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsWithAnimation(startOptions, false);

    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);

    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsWithAnimationValue_001
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsWithAnimationValue
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsWithAnimationValue_001, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = nullptr;
    bool withAnimation;

    // Act
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_GetStartOptionsWithAnimationValue(startOptions, &withAnimation);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, result);
}
 
/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsWithAnimationValue_002
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsWithAnimationValue
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsWithAnimationValue_002, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsWithAnimation(startOptions, true);
    // Assuming OH_AbilityRuntime_SetStartOptionsWindowMode returns success
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);

    bool withAnimation;

    // Act
    result = OH_AbilityRuntime_GetStartOptionsWithAnimationValue(startOptions, &withAnimation);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);
    EXPECT_EQ(true, withAnimation);
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsWithAnimationValue_003
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsWithAnimationValue
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsWithAnimationValue_003, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsWithAnimation(startOptions, false);
    // Assuming OH_AbilityRuntime_SetStartOptionsWindowMode returns success
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);

    bool withAnimation;

    // Act
    result = OH_AbilityRuntime_GetStartOptionsWithAnimationValue(startOptions, &withAnimation);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);
    EXPECT_EQ(false, withAnimation);
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsWindowLeft_001
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsWindowLeft
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsWindowLeft_001, TestSize.Level1)
{
    AbilityRuntime_StartOptions* startOptions = nullptr;
    int32_t windowLeft = 100;
    EXPECT_EQ(OH_AbilityRuntime_SetStartOptionsWindowLeft(startOptions, windowLeft),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsWindowLeft_002
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsWindowLeft
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsWindowLeft_002, TestSize.Level1)
{
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    int32_t windowLeft = 100;
    EXPECT_EQ(OH_AbilityRuntime_SetStartOptionsWindowLeft(startOptions, windowLeft),
        startOptions->SetStartOptionsWindowLeft(windowLeft));
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsWindowLeft_003
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsWindowLeft
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsWindowLeft_003, TestSize.Level1)
{
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    int32_t windowLeft = -100;
    EXPECT_EQ(OH_AbilityRuntime_SetStartOptionsWindowLeft(startOptions, windowLeft),
        startOptions->SetStartOptionsWindowLeft(windowLeft));
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsWindowLeft_004
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsWindowLeft
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsWindowLeft_004, TestSize.Level1)
{
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    int32_t windowLeft = 0;
    EXPECT_EQ(OH_AbilityRuntime_SetStartOptionsWindowLeft(startOptions, windowLeft),
        startOptions->SetStartOptionsWindowLeft(windowLeft));
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsWindowLeftValue_001
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsWindowLeftValue
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsWindowLeftValue_001, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = nullptr;
    int32_t windowLeft;

    // Act
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_GetStartOptionsWindowLeftValue(startOptions, &windowLeft);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, result);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsWindowLeftValue_002
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsWindowLeftValue
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsWindowLeftValue_002, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsWindowLeft(startOptions, 100);
    // Assuming OH_AbilityRuntime_SetStartOptionsWindowMode returns success
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);

    int32_t windowLeft;

    // Act
    result = OH_AbilityRuntime_GetStartOptionsWindowLeftValue(startOptions, &windowLeft);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);
    EXPECT_EQ(100, windowLeft);
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsWindowLeftValue_003
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsWindowLeftValue
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsWindowLeftValue_003, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsWindowLeft(startOptions, -100);
    // Assuming OH_AbilityRuntime_SetStartOptionsWindowMode returns success
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);

    int32_t windowLeft;

    // Act
    result = OH_AbilityRuntime_GetStartOptionsWindowLeftValue(startOptions, &windowLeft);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);
    EXPECT_EQ(-100, windowLeft);
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsWindowTop_001
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsWindowTop
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsWindowTop_001, TestSize.Level1)
{
    AbilityRuntime_StartOptions* startOptions = nullptr;
    int32_t windowTop = 100;
    EXPECT_EQ(OH_AbilityRuntime_SetStartOptionsWindowTop(startOptions, windowTop),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsWindowTop_002
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsWindowTop
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsWindowTop_002, TestSize.Level1)
{
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    int32_t windowTop = 100;
    EXPECT_EQ(OH_AbilityRuntime_SetStartOptionsWindowTop(startOptions, windowTop),
        startOptions->SetStartOptionsWindowTop(windowTop));
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsWindowTop_003
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsWindowTop
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsWindowTop_003, TestSize.Level1)
{
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    int32_t windowTop = -100;
    EXPECT_EQ(OH_AbilityRuntime_SetStartOptionsWindowTop(startOptions, windowTop),
        startOptions->SetStartOptionsWindowTop(windowTop));
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsWindowTop_004
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsWindowTop
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsWindowTop_004, TestSize.Level1)
{
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    int32_t windowTop = 0;
    EXPECT_EQ(OH_AbilityRuntime_SetStartOptionsWindowTop(startOptions, windowTop),
        startOptions->SetStartOptionsWindowTop(windowTop));
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsWindowTopValue_001
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsWindowTopValue
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsWindowTopValue_001, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = nullptr;
    int32_t windowTop;

    // Act
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_GetStartOptionsWindowTopValue(startOptions, &windowTop);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, result);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsWindowTopValue_002
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsWindowTopValue
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsWindowTopValue_002, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsWindowTop(startOptions, 100);
    // Assuming OH_AbilityRuntime_SetStartOptionsWindowMode returns success
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);

    int32_t windowTop;

    // Act
    result = OH_AbilityRuntime_GetStartOptionsWindowTopValue(startOptions, &windowTop);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);
    EXPECT_EQ(100, windowTop);
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsWindowTopValue_003
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsWindowTopValue
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsWindowTopValue_003, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsWindowTop(startOptions, -100);
    // Assuming OH_AbilityRuntime_SetStartOptionsWindowMode returns success
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);

    int32_t windowTop;

    // Act
    result = OH_AbilityRuntime_GetStartOptionsWindowTopValue(startOptions, &windowTop);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);
    EXPECT_EQ(-100, windowTop);
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsWindowHeight_001
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsWindowHeight
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsWindowHeight_001, TestSize.Level1)
{
    AbilityRuntime_StartOptions* startOptions = nullptr;
    int32_t windowHeight = 100;
    EXPECT_EQ(OH_AbilityRuntime_SetStartOptionsWindowHeight(startOptions, windowHeight),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsWindowHeight_002
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsWindowHeight
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsWindowHeight_002, TestSize.Level1)
{
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    int32_t windowHeight = 100;
    EXPECT_EQ(OH_AbilityRuntime_SetStartOptionsWindowHeight(startOptions, windowHeight),
        startOptions->SetStartOptionsWindowHeight(windowHeight));
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsWindowHeight_003
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsWindowHeight
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsWindowHeight_003, TestSize.Level1)
{
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    int32_t windowHeight = -100;
    EXPECT_EQ(OH_AbilityRuntime_SetStartOptionsWindowHeight(startOptions, windowHeight),
        startOptions->SetStartOptionsWindowHeight(windowHeight));
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsWindowHeight_004
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsWindowHeight
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsWindowHeight_004, TestSize.Level1)
{
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    int32_t windowHeight = 0;
    EXPECT_EQ(OH_AbilityRuntime_SetStartOptionsWindowHeight(startOptions, windowHeight),
        startOptions->SetStartOptionsWindowHeight(windowHeight));
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsWindowHeightValue_001
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsWindowHeightValue
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsWindowHeightValue_001, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = nullptr;
    int32_t windowHeight;

    // Act
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_GetStartOptionsWindowHeightValue(startOptions, &windowHeight);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, result);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsWindowHeightValue_002
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsWindowHeightValue
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsWindowHeightValue_002, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsWindowHeight(startOptions, 100);
    // Assuming OH_AbilityRuntime_SetStartOptionsWindowMode returns success
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);

    int32_t windowHeight;

    // Act
    result = OH_AbilityRuntime_GetStartOptionsWindowHeightValue(startOptions, &windowHeight);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);
    EXPECT_EQ(100, windowHeight);
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsWindowHeightValue_003
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsWindowHeightValue
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsWindowHeightValue_003, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsWindowHeight(startOptions, -100);
    // Assuming OH_AbilityRuntime_SetStartOptionsWindowMode returns success
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);

    int32_t windowHeight;

    // Act
    result = OH_AbilityRuntime_GetStartOptionsWindowHeightValue(startOptions, &windowHeight);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);
    EXPECT_EQ(-100, windowHeight);
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsWindowWidth_001
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsWindowWidth
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsWindowWidth_001, TestSize.Level1)
{
    AbilityRuntime_StartOptions* startOptions = nullptr;
    int32_t windowWidth = 100;
    EXPECT_EQ(OH_AbilityRuntime_SetStartOptionsWindowWidth(startOptions, windowWidth),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsWindowWidth_002
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsWindowWidth
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsWindowWidth_002, TestSize.Level1)
{
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    int32_t windowWidth = 100;
    EXPECT_EQ(OH_AbilityRuntime_SetStartOptionsWindowWidth(startOptions, windowWidth),
        startOptions->SetStartOptionsWindowWidth(windowWidth));
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsWindowWidth_003
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsWindowWidth
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsWindowWidth_003, TestSize.Level1)
{
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    int32_t windowWidth = -100;
    EXPECT_EQ(OH_AbilityRuntime_SetStartOptionsWindowWidth(startOptions, windowWidth),
        startOptions->SetStartOptionsWindowWidth(windowWidth));
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsWindowWidth_004
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsWindowWidth
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsWindowWidth_004, TestSize.Level1)
{
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    int32_t windowWidth = 0;
    EXPECT_EQ(OH_AbilityRuntime_SetStartOptionsWindowWidth(startOptions, windowWidth),
        startOptions->SetStartOptionsWindowWidth(windowWidth));
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsWindowWidthValue_001
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsWindowWidthValue
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsWindowWidthValue_001, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = nullptr;
    int32_t windowWidth;

    // Act
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_GetStartOptionsWindowWidthValue(startOptions, &windowWidth);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, result);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsWindowWidthValue_002
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsWindowWidthValue
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsWindowWidthValue_002, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsWindowWidth(startOptions, 100);
    // Assuming OH_AbilityRuntime_SetStartOptionsWindowMode returns success
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);

    int32_t windowWidth;

    // Act
    result = OH_AbilityRuntime_GetStartOptionsWindowWidthValue(startOptions, &windowWidth);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);
    EXPECT_EQ(100, windowWidth);
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsWindowWidthValue_003
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsWindowWidthValue
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsWindowWidthValue_003, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsWindowWidth(startOptions, -100);
    // Assuming OH_AbilityRuntime_SetStartOptionsWindowMode returns success
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);

    int32_t windowWidth;

    // Act
    result = OH_AbilityRuntime_GetStartOptionsWindowWidthValue(startOptions, &windowWidth);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);
    EXPECT_EQ(-100, windowWidth);
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsStartVisibility_001
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsStartVisibility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsStartVisibility_001, TestSize.Level1)
{
    AbilityRuntime_StartOptions* startOptions = nullptr;
    AbilityRuntime_StartVisibility startVisibility = AbilityRuntime_StartVisibility::ABILITY_RUNTIME_HIDE_UPON_START;
    EXPECT_EQ(OH_AbilityRuntime_SetStartOptionsStartVisibility(startOptions, startVisibility),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsStartVisibility_002
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsStartVisibility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsStartVisibility_002, TestSize.Level1)
{
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    AbilityRuntime_StartVisibility startVisibility = AbilityRuntime_StartVisibility::ABILITY_RUNTIME_HIDE_UPON_START;
    EXPECT_EQ(OH_AbilityRuntime_SetStartOptionsStartVisibility(startOptions, startVisibility),
        startOptions->SetStartOptionsStartVisibility(startVisibility));
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsStartVisibilityValue_001
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsStartVisibilityValue
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsStartVisibilityValue_001, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = nullptr;
    AbilityRuntime_StartVisibility startVisibility;

    // Act
    AbilityRuntime_ErrorCode result =
        OH_AbilityRuntime_GetStartOptionsStartVisibilityValue(startOptions, &startVisibility);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, result);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsStartVisibilityValue_002
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsStartVisibilityValue
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsStartVisibilityValue_002, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsStartVisibility(startOptions,
        AbilityRuntime_StartVisibility::ABILITY_RUNTIME_SHOW_UPON_START);
    // Assuming OH_AbilityRuntime_SetStartOptionsWindowMode returns success
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);

    AbilityRuntime_StartVisibility startVisibility;

    // Act
    result = OH_AbilityRuntime_GetStartOptionsStartVisibilityValue(startOptions, &startVisibility);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);
    EXPECT_EQ(AbilityRuntime_StartVisibility::ABILITY_RUNTIME_SHOW_UPON_START, startVisibility);
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);

    result = OH_AbilityRuntime_GetStartOptionsStartVisibilityValue(startOptions, &startVisibility);
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, result);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsStartVisibilityValue_003
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsStartVisibilityValue
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsStartVisibilityValue_003, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    AbilityRuntime_StartVisibility startVisibility;

    AbilityRuntime_ErrorCode result =
        OH_AbilityRuntime_GetStartOptionsStartVisibilityValue(startOptions, &startVisibility);
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, result);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsMinWindowWidth_001
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsMinWindowWidth
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsMinWindowWidth_001, TestSize.Level1)
{
    AbilityRuntime_StartOptions* startOptions = nullptr;
    int32_t minWindowWidth = 100;
    EXPECT_EQ(OH_AbilityRuntime_SetStartOptionsMinWindowWidth(startOptions, minWindowWidth),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsMinWindowWidth_002
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsMinWindowWidth
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsMinWindowWidth_002, TestSize.Level1)
{
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    int32_t minWindowWidth = 100;
    EXPECT_EQ(OH_AbilityRuntime_SetStartOptionsMinWindowWidth(startOptions, minWindowWidth),
        startOptions->SetStartOptionsMinWindowWidth(minWindowWidth));
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsMinWindowWidth_003
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsMinWindowWidth
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsMinWindowWidth_003, TestSize.Level1)
{
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    int32_t minWindowWidth = -100;
    EXPECT_EQ(OH_AbilityRuntime_SetStartOptionsMinWindowWidth(startOptions, minWindowWidth),
        startOptions->SetStartOptionsMinWindowWidth(minWindowWidth));
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsMinWindowWidth_004
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsMinWindowWidth
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsMinWindowWidth_004, TestSize.Level1)
{
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    int32_t minWindowWidth = 0;
    EXPECT_EQ(OH_AbilityRuntime_SetStartOptionsMinWindowWidth(startOptions, minWindowWidth),
        startOptions->SetStartOptionsMinWindowWidth(minWindowWidth));
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsMinWindowWidthValue_001
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsMinWindowWidthValue
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsMinWindowWidthValue_001, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = nullptr;
    int32_t minWindowWidth;

    // Act
    AbilityRuntime_ErrorCode result =
        OH_AbilityRuntime_GetStartOptionsMinWindowWidthValue(startOptions, &minWindowWidth);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, result);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsMinWindowWidthValue_002
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsMinWindowWidthValue
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsMinWindowWidthValue_002, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsMinWindowWidth(startOptions, 100);
    // Assuming OH_AbilityRuntime_SetStartOptionsWindowMode returns success
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);

    int32_t minWindowWidth;

    // Act
    result = OH_AbilityRuntime_GetStartOptionsMinWindowWidthValue(startOptions, &minWindowWidth);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);
    EXPECT_EQ(100, minWindowWidth);
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsMinWindowWidthValue_003
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsMinWindowWidthValue
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsMinWindowWidthValue_003, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsMinWindowWidth(startOptions, -100);
    // Assuming OH_AbilityRuntime_SetStartOptionsWindowMode returns success
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);

    int32_t minWindowWidth;

    // Act
    result = OH_AbilityRuntime_GetStartOptionsMinWindowWidthValue(startOptions, &minWindowWidth);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);
    EXPECT_EQ(-100, minWindowWidth);
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsMaxWindowWidth_001
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsMaxWindowWidth
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsMaxWindowWidth_001, TestSize.Level1)
{
    AbilityRuntime_StartOptions* startOptions = nullptr;
    int32_t maxWindowWidth = 100;
    EXPECT_EQ(OH_AbilityRuntime_SetStartOptionsMaxWindowWidth(startOptions, maxWindowWidth),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsMaxWindowWidth_002
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsMaxWindowWidth
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsMaxWindowWidth_002, TestSize.Level1)
{
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    int32_t maxWindowWidth = 100;
    EXPECT_EQ(OH_AbilityRuntime_SetStartOptionsMaxWindowWidth(startOptions, maxWindowWidth),
        startOptions->SetStartOptionsMaxWindowWidth(maxWindowWidth));
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsMaxWindowWidth_003
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsMaxWindowWidth
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsMaxWindowWidth_003, TestSize.Level1)
{
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    int32_t maxWindowWidth = -100;
    EXPECT_EQ(OH_AbilityRuntime_SetStartOptionsMaxWindowWidth(startOptions, maxWindowWidth),
        startOptions->SetStartOptionsMaxWindowWidth(maxWindowWidth));
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsMaxWindowWidth_004
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsMaxWindowWidth
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsMaxWindowWidth_004, TestSize.Level1)
{
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    int32_t maxWindowWidth = 0;
    EXPECT_EQ(OH_AbilityRuntime_SetStartOptionsMaxWindowWidth(startOptions, maxWindowWidth),
        startOptions->SetStartOptionsMaxWindowWidth(maxWindowWidth));
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsMaxWindowWidthValue_001
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsMaxWindowWidthValue
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsMaxWindowWidthValue_001, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = nullptr;
    int32_t maxWindowWidth;

    // Act
    AbilityRuntime_ErrorCode result =
        OH_AbilityRuntime_GetStartOptionsMaxWindowWidthValue(startOptions, &maxWindowWidth);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, result);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsMaxWindowWidthValue_002
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsMaxWindowWidthValue
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsMaxWindowWidthValue_002, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsMaxWindowWidth(startOptions, 100);
    // Assuming OH_AbilityRuntime_SetStartOptionsWindowMode returns success
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);

    int32_t maxWindowWidth;

    // Act
    result = OH_AbilityRuntime_GetStartOptionsMaxWindowWidthValue(startOptions, &maxWindowWidth);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);
    EXPECT_EQ(100, maxWindowWidth);
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsMaxWindowWidthValue_003
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsMaxWindowWidthValue
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsMaxWindowWidthValue_003, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsMaxWindowWidth(startOptions, -100);
    // Assuming OH_AbilityRuntime_SetStartOptionsWindowMode returns success
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);

    int32_t maxWindowWidth;

    // Act
    result = OH_AbilityRuntime_GetStartOptionsMaxWindowWidthValue(startOptions, &maxWindowWidth);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);
    EXPECT_EQ(-100, maxWindowWidth);
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsMaxWindowHeight_001
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsMaxWindowHeight
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsMaxWindowHeight_001, TestSize.Level1)
{
    AbilityRuntime_StartOptions* startOptions = nullptr;
    int32_t maxWindowHeight = 100;
    EXPECT_EQ(OH_AbilityRuntime_SetStartOptionsMaxWindowHeight(startOptions, maxWindowHeight),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsMaxWindowHeight_002
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsMaxWindowHeight
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsMaxWindowHeight_002, TestSize.Level1)
{
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    int32_t maxWindowHeight = 100;
    EXPECT_EQ(OH_AbilityRuntime_SetStartOptionsMaxWindowHeight(startOptions, maxWindowHeight),
        startOptions->SetStartOptionsMaxWindowHeight(maxWindowHeight));
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsMaxWindowHeight_003
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsMaxWindowHeight
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsMaxWindowHeight_003, TestSize.Level1)
{
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    int32_t maxWindowHeight = -100;
    EXPECT_EQ(OH_AbilityRuntime_SetStartOptionsMaxWindowHeight(startOptions, maxWindowHeight),
        startOptions->SetStartOptionsMaxWindowHeight(maxWindowHeight));
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsMaxWindowHeight_004
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsMaxWindowHeight
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsMaxWindowHeight_004, TestSize.Level1)
{
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    int32_t maxWindowHeight = 0;
    EXPECT_EQ(OH_AbilityRuntime_SetStartOptionsMaxWindowHeight(startOptions, maxWindowHeight),
        startOptions->SetStartOptionsMaxWindowHeight(maxWindowHeight));
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsMaxWindowHeightValue_001
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsMaxWindowHeightValue
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsMaxWindowHeightValue_001, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = nullptr;
    int32_t maxWindowHeight;

    // Act
    AbilityRuntime_ErrorCode result =
        OH_AbilityRuntime_GetStartOptionsMaxWindowHeightValue(startOptions, &maxWindowHeight);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, result);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsMaxWindowHeightValue_002
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsMaxWindowHeightValue
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsMaxWindowHeightValue_002, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsMaxWindowHeight(startOptions, 100);
    // Assuming OH_AbilityRuntime_SetStartOptionsWindowMode returns success
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);

    int32_t maxWindowHeight;

    // Act
    result = OH_AbilityRuntime_GetStartOptionsMaxWindowHeightValue(startOptions, &maxWindowHeight);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);
    EXPECT_EQ(100, maxWindowHeight);
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsMaxWindowHeightValue_003
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsMaxWindowHeightValue
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsMaxWindowHeightValue_003, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsMaxWindowHeight(startOptions, -100);
    // Assuming OH_AbilityRuntime_SetStartOptionsWindowMode returns success
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);

    int32_t maxWindowHeight;

    // Act
    result = OH_AbilityRuntime_GetStartOptionsMaxWindowHeightValue(startOptions, &maxWindowHeight);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);
    EXPECT_EQ(-100, maxWindowHeight);
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsMinWindowHeight_001
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsMinWindowHeight
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsMinWindowHeight_001, TestSize.Level1)
{
    AbilityRuntime_StartOptions* startOptions = nullptr;
    int32_t minWindowHeight = 100;
    EXPECT_EQ(OH_AbilityRuntime_SetStartOptionsMinWindowHeight(startOptions, minWindowHeight),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsMinWindowHeight_002
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsMinWindowHeight
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsMinWindowHeight_002, TestSize.Level1)
{
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    int32_t minWindowHeight = 100;
    EXPECT_EQ(OH_AbilityRuntime_SetStartOptionsMinWindowHeight(startOptions, minWindowHeight),
        startOptions->SetStartOptionsMinWindowHeight(minWindowHeight));
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsMinWindowHeight_003
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsMinWindowHeight
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsMinWindowHeight_003, TestSize.Level1)
{
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    int32_t minWindowHeight = -100;
    EXPECT_EQ(OH_AbilityRuntime_SetStartOptionsMinWindowHeight(startOptions, minWindowHeight),
        startOptions->SetStartOptionsMinWindowHeight(minWindowHeight));
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsMinWindowHeight_004
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsMinWindowHeight
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsMinWindowHeight_004, TestSize.Level1)
{
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    int32_t minWindowHeight = 0;
    EXPECT_EQ(OH_AbilityRuntime_SetStartOptionsMinWindowHeight(startOptions, minWindowHeight),
        startOptions->SetStartOptionsMinWindowHeight(minWindowHeight));
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsMinWindowHeightValue_001
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsMinWindowHeightValue
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsMinWindowHeightValue_001, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = nullptr;
    int32_t minWindowHeight;

    // Act
    AbilityRuntime_ErrorCode result =
        OH_AbilityRuntime_GetStartOptionsMinWindowHeightValue(startOptions, &minWindowHeight);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, result);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsMinWindowHeightValue_002
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsMinWindowHeightValue
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsMinWindowHeightValue_002, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsMinWindowHeight(startOptions, 100);
    // Assuming OH_AbilityRuntime_SetStartOptionsWindowMode returns success
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);

    int32_t minWindowHeight;

    // Act
    result = OH_AbilityRuntime_GetStartOptionsMinWindowHeightValue(startOptions, &minWindowHeight);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);
    EXPECT_EQ(100, minWindowHeight);
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsMinWindowHeightValue_003
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsMinWindowHeightValue
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsMinWindowHeightValue_003, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsMinWindowHeight(startOptions, -100);
    // Assuming OH_AbilityRuntime_SetStartOptionsWindowMode returns success
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);

    int32_t minWindowHeight;

    // Act
    result = OH_AbilityRuntime_GetStartOptionsMinWindowHeightValue(startOptions, &minWindowHeight);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);
    EXPECT_EQ(-100, minWindowHeight);
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsStartWindowIcon_001
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsStartWindowIcon
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsStartWindowIcon_001, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = nullptr;
    OH_PixelmapNative *startWindowIcon = nullptr;
#ifdef START_WINDOW_OPTIONS_WITH_PIXELMAP
    startWindowIcon = new OH_PixelmapNative(nullptr);
#endif

    // Act
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsStartWindowIcon(startOptions, startWindowIcon);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, result);
#ifdef START_WINDOW_OPTIONS_WITH_PIXELMAP
    delete startWindowIcon;
#endif
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsStartWindowIcon_002
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsStartWindowIcon
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsStartWindowIcon_002, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    EXPECT_NE(startOptions, nullptr);
    OH_PixelmapNative *startWindowIcon = nullptr;
#ifdef START_WINDOW_OPTIONS_WITH_PIXELMAP
    startWindowIcon = new OH_PixelmapNative(nullptr);
#endif
    // Act
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsStartWindowIcon(startOptions, startWindowIcon);

    // Assert
    EXPECT_EQ(startOptions->SetStartOptionsStartWindowIcon(startWindowIcon), result);

    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
#ifdef START_WINDOW_OPTIONS_WITH_PIXELMAP
    delete startWindowIcon;
#endif
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsStartWindowIcon_001
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsStartWindowIcon
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsStartWindowIcon_001, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = nullptr;
    OH_PixelmapNative *startWindowIcon = nullptr;

    // Act
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_GetStartOptionsStartWindowIcon(startOptions, &startWindowIcon);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, result);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsStartWindowIcon_002
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsStartWindowIcon
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsStartWindowIcon_002, TestSize.Level1)
{
    // Arrange
    TAG_LOGI(AAFwkTag::APPKIT, "OH_AbilityRuntime_GetStartOptionsStartWindowIcon_002 begin");
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    EXPECT_NE(startOptions, nullptr);

    AbilityRuntime_ErrorCode result = ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
#ifdef START_WINDOW_OPTIONS_WITH_PIXELMAP
    OH_PixelmapNative *startWindowIcon = new OH_PixelmapNative(nullptr);
    result = OH_AbilityRuntime_SetStartOptionsStartWindowIcon(startOptions, startWindowIcon);
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);
#endif

    // Act
    OH_PixelmapNative *newStartWindowIcon = nullptr;
    result = OH_AbilityRuntime_GetStartOptionsStartWindowIcon(startOptions, &newStartWindowIcon);

    // Assert
#ifdef START_WINDOW_OPTIONS_WITH_PIXELMAP
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);
    EXPECT_EQ(newStartWindowIcon, nullptr);
#else
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, result);
#endif

#ifdef START_WINDOW_OPTIONS_WITH_PIXELMAP
    delete startWindowIcon;
#endif
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
    TAG_LOGI(AAFwkTag::APPKIT, "OH_AbilityRuntime_GetStartOptionsStartWindowIcon_002 end");
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsStartWindowIcon_003
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsStartWindowIcon
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsStartWindowIcon_003, TestSize.Level1)
{
    // Arrange
    TAG_LOGI(AAFwkTag::APPKIT, "OH_AbilityRuntime_GetStartOptionsStartWindowIcon_003 begin");
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    EXPECT_NE(startOptions, nullptr);

    OH_PixelmapNative *startWindowIcon = nullptr;

    // Act
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_GetStartOptionsStartWindowIcon(startOptions, &startWindowIcon);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, result);

    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
    TAG_LOGI(AAFwkTag::APPKIT, "OH_AbilityRuntime_GetStartOptionsStartWindowIcon_003 end");
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsStartWindowBackgroundColor_001
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsStartWindowBackgroundColor
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest,
    OH_AbilityRuntime_SetStartOptionsStartWindowBackgroundColor_001, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = nullptr;
    const char *startWindowBackgroundColor = "FFFFFF";

    // Act
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsStartWindowBackgroundColor(
        startOptions, startWindowBackgroundColor);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, result);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsStartWindowBackgroundColor_002
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsStartWindowBackgroundColor
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest,
    OH_AbilityRuntime_SetStartOptionsStartWindowBackgroundColor_002, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    EXPECT_NE(startOptions, nullptr);
    const char *startWindowBackgroundColor = "FFFFFF";

    // Act
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsStartWindowBackgroundColor(
        startOptions, startWindowBackgroundColor);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsStartWindowBackgroundColor_003
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsStartWindowBackgroundColor
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest,
    OH_AbilityRuntime_SetStartOptionsStartWindowBackgroundColor_003, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    EXPECT_NE(startOptions, nullptr);
    const char *startWindowBackgroundColor = nullptr;

    // Act
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsStartWindowBackgroundColor(
        startOptions, startWindowBackgroundColor);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, result);
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsStartWindowBackgroundColorValue_001
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsStartWindowBackgroundColorValue
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest,
    OH_AbilityRuntime_GetStartOptionsStartWindowBackgroundColorValue_001, TestSize.Level1)
{
    AbilityRuntime_StartOptions *startOptions = nullptr;
    char *startWindowBackgroundColor;
    size_t size;
    EXPECT_EQ(OH_AbilityRuntime_GetStartOptionsStartWindowBackgroundColorValue(
        startOptions, &startWindowBackgroundColor, &size), ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsStartWindowBackgroundColorValue_002
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsStartWindowBackgroundColorValue
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest,
    OH_AbilityRuntime_GetStartOptionsStartWindowBackgroundColorValue_002, TestSize.Level1)
{
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    EXPECT_NE(startOptions, nullptr);

    char originalStartWindowBackgroundColor[] = "FFFFFF";
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsStartWindowBackgroundColor(
        startOptions, originalStartWindowBackgroundColor);
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);

    char *startWindowBackgroundColor = nullptr;
    size_t size = 0;
    EXPECT_EQ(OH_AbilityRuntime_GetStartOptionsStartWindowBackgroundColorValue(startOptions,
        &startWindowBackgroundColor, &size), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(strcmp(startWindowBackgroundColor, originalStartWindowBackgroundColor), 0);
    EXPECT_EQ(size, strlen(originalStartWindowBackgroundColor));
    free(startWindowBackgroundColor);
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsStartWindowBackgroundColorValue_003
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsStartWindowBackgroundColorValue
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest,
    OH_AbilityRuntime_GetStartOptionsStartWindowBackgroundColorValue_003, TestSize.Level1)
{
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    EXPECT_NE(startOptions, nullptr);

    char originalStartWindowBackgroundColor[] = "";
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsStartWindowBackgroundColor(
        startOptions, originalStartWindowBackgroundColor);
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);

    char *startWindowBackgroundColor = nullptr;
    size_t size = 0;
    EXPECT_EQ(OH_AbilityRuntime_GetStartOptionsStartWindowBackgroundColorValue(
        startOptions, &startWindowBackgroundColor, &size), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(size, 0);
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsSupportedWindowModes_001
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsSupportedWindowModes
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsSupportedWindowModes_001, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = nullptr;
    AbilityRuntime_SupportedWindowMode *supportedWindowModes = nullptr;
    size_t size = 0;

    // Act
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsSupportedWindowModes(
        startOptions, supportedWindowModes, size);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, result);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsSupportedWindowModes_002
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsSupportedWindowModes
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsSupportedWindowModes_002, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    EXPECT_NE(startOptions, nullptr);

    AbilityRuntime_SupportedWindowMode supportedWindowModes[1] = { ABILITY_RUNTIME_SUPPORTED_WINDOW_MODE_FULL_SCREEN };
    size_t size = 1;

    // Act
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsSupportedWindowModes(
        startOptions, supportedWindowModes, size);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsSupportedWindowModes_003
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsSupportedWindowModes
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsSupportedWindowModes_003, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    EXPECT_NE(startOptions, nullptr);

    AbilityRuntime_SupportedWindowMode *supportedWindowModes = nullptr;
    size_t size = 0; // Invalid size

    // Act
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsSupportedWindowModes(
        startOptions, supportedWindowModes, size);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, result);
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_SetStartOptionsSupportedWindowModes_004
 * @tc.desc: OH_AbilityRuntime_SetStartOptionsSupportedWindowModes
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_SetStartOptionsSupportedWindowModes_004, TestSize.Level1)
{
    // Arrange
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    EXPECT_NE(startOptions, nullptr);

    AbilityRuntime_SupportedWindowMode supportedWindowModes[1] = { ABILITY_RUNTIME_SUPPORTED_WINDOW_MODE_FULL_SCREEN };
    size_t size = MAX_SUPPOPRT_WINDOW_MODES_SIZE + 1; // Invalid size

    // Act
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsSupportedWindowModes(
        startOptions, supportedWindowModes, size);

    // Assert
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, result);
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsSupportedWindowModesValue_001
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsSupportedWindowModesValue
 * @tc.type: FUNC
 */
HWTEST_F(
    AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsSupportedWindowModesValue_001, TestSize.Level1)
{
    AbilityRuntime_SupportedWindowMode *supportedWindowModes = nullptr;
    size_t size = 0;
    EXPECT_EQ(OH_AbilityRuntime_GetStartOptionsSupportedWindowModesValue(nullptr, &supportedWindowModes, &size),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsSupportedWindowModesValue_002
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsSupportedWindowModesValue
 * @tc.type: FUNC
 */
HWTEST_F(
    AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsSupportedWindowModesValue_002, TestSize.Level1)
{
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    EXPECT_NE(startOptions, nullptr);

    AbilityRuntime_SupportedWindowMode supportedWindowModes[1] = { ABILITY_RUNTIME_SUPPORTED_WINDOW_MODE_FULL_SCREEN };
    size_t size = 1;

    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_SetStartOptionsSupportedWindowModes(
        startOptions, supportedWindowModes, size);
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);

    // Act
    AbilityRuntime_SupportedWindowMode *newSupportWindowModes = nullptr;
    size_t newSize = 0;
    EXPECT_EQ(
        OH_AbilityRuntime_GetStartOptionsSupportedWindowModesValue(startOptions, &newSupportWindowModes, &newSize),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_NE(newSupportWindowModes, nullptr);
    EXPECT_EQ(newSize, 1);
    free(newSupportWindowModes);
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsSupportedWindowModesValue_003
 * @tc.desc: OH_AbilityRuntime_GetStartOptionsSupportedWindowModesValue
 * @tc.type: FUNC
 */
HWTEST_F(
    AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsSupportedWindowModesValue_003, TestSize.Level1)
{
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    EXPECT_NE(startOptions, nullptr);

    // Act
    AbilityRuntime_SupportedWindowMode *newSupportWindowModes = nullptr;
    size_t newSize = 0;
    EXPECT_EQ(
        OH_AbilityRuntime_GetStartOptionsSupportedWindowModesValue(startOptions, &newSupportWindowModes, &newSize),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(newSupportWindowModes, nullptr);
    EXPECT_EQ(newSize, 0);
    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsNullOutput_001
 * @tc.desc: OH_AbilityRuntime_GetStartOptions with null output pointers
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsNullOutput_001, TestSize.Level1)
{
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    ASSERT_NE(startOptions, nullptr);

    char *startWindowBackgroundColor = nullptr;
    size_t size = 0;
    AbilityRuntime_SupportedWindowMode *supportedWindowModes = nullptr;

    EXPECT_EQ(OH_AbilityRuntime_GetStartOptionsWindowModeValue(startOptions, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_GetStartOptionsDisplayIdValue(startOptions, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_GetStartOptionsWithAnimationValue(startOptions, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_GetStartOptionsWindowLeftValue(startOptions, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_GetStartOptionsWindowTopValue(startOptions, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_GetStartOptionsWindowHeightValue(startOptions, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_GetStartOptionsWindowWidthValue(startOptions, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_GetStartOptionsStartVisibilityValue(startOptions, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_GetStartOptionsStartWindowBackgroundColorValue(startOptions, nullptr, &size),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_GetStartOptionsStartWindowBackgroundColorValue(
        startOptions, &startWindowBackgroundColor, nullptr), ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_GetStartOptionsSupportedWindowModesValue(startOptions, nullptr, &size),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_GetStartOptionsSupportedWindowModesValue(startOptions, &supportedWindowModes, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_GetStartOptionsMinWindowWidthValue(startOptions, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_GetStartOptionsMaxWindowWidthValue(startOptions, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_GetStartOptionsMinWindowHeightValue(startOptions, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_GetStartOptionsMaxWindowHeightValue(startOptions, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}

/**
 * @tc.name: OH_AbilityRuntime_GetStartOptionsNeverSet_001
 * @tc.desc: Value getters keep existing never-set semantics.
 *           Out-params are seeded with a non-zero sentinel to detect an unwritten result.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeStartOptionsTest, OH_AbilityRuntime_GetStartOptionsNeverSet_001, TestSize.Level1)
{
    AbilityRuntime_StartOptions *startOptions = OH_AbilityRuntime_CreateStartOptions();
    ASSERT_NE(startOptions, nullptr);

    const int32_t SENTINEL = 0x7F;
    int32_t windowLeft = SENTINEL;
    EXPECT_EQ(OH_AbilityRuntime_GetStartOptionsWindowLeftValue(startOptions, &windowLeft),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(0, windowLeft);

    int32_t windowTop = SENTINEL;
    EXPECT_EQ(OH_AbilityRuntime_GetStartOptionsWindowTopValue(startOptions, &windowTop),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(0, windowTop);

    int32_t windowHeight = SENTINEL;
    EXPECT_EQ(OH_AbilityRuntime_GetStartOptionsWindowHeightValue(startOptions, &windowHeight),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(0, windowHeight);

    int32_t windowWidth = SENTINEL;
    EXPECT_EQ(OH_AbilityRuntime_GetStartOptionsWindowWidthValue(startOptions, &windowWidth),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(0, windowWidth);

    int32_t minWindowWidth = SENTINEL;
    EXPECT_EQ(OH_AbilityRuntime_GetStartOptionsMinWindowWidthValue(startOptions, &minWindowWidth),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(0, minWindowWidth);

    int32_t maxWindowWidth = SENTINEL;
    EXPECT_EQ(OH_AbilityRuntime_GetStartOptionsMaxWindowWidthValue(startOptions, &maxWindowWidth),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(0, maxWindowWidth);

    int32_t minWindowHeight = SENTINEL;
    EXPECT_EQ(OH_AbilityRuntime_GetStartOptionsMinWindowHeightValue(startOptions, &minWindowHeight),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(0, minWindowHeight);

    int32_t maxWindowHeight = SENTINEL;
    EXPECT_EQ(OH_AbilityRuntime_GetStartOptionsMaxWindowHeightValue(startOptions, &maxWindowHeight),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(0, maxWindowHeight);

    char *startWindowBackgroundColor = nullptr;
    size_t colorSize = 0x7F;
    EXPECT_EQ(OH_AbilityRuntime_GetStartOptionsStartWindowBackgroundColorValue(
        startOptions, &startWindowBackgroundColor, &colorSize), ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(nullptr, startWindowBackgroundColor);
    EXPECT_EQ(static_cast<size_t>(0), colorSize);

    AbilityRuntime_SupportedWindowMode *supportedWindowModes = nullptr;
    size_t modesSize = 0x7F;
    EXPECT_EQ(OH_AbilityRuntime_GetStartOptionsSupportedWindowModesValue(
        startOptions, &supportedWindowModes, &modesSize), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(nullptr, supportedWindowModes);
    EXPECT_EQ(static_cast<size_t>(0), modesSize);

    OH_AbilityRuntime_DestroyStartOptions(&startOptions);
}
}  // namespace AppExecFwk
}  // namespace OHOS
