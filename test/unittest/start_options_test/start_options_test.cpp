/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#include <gtest/gtest.h>

#define private public
#include "start_options.h"
#undef private

#include "ability_window_configuration.h"
#include "parcel.h"

using namespace testing::ext;
namespace OHOS {
namespace AAFwk {
namespace {
const int32_t TEST_DISPLAY_ID = 1;
const bool TEST_TRUE = true;
const bool TEST_FALSE = false;
} // namespace

class StartOptionsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void StartOptionsTest::SetUpTestCase()
{}

void StartOptionsTest::TearDownTestCase()
{}

void StartOptionsTest::SetUp()
{}

void StartOptionsTest::TearDown()
{}

/**
 * @tc.name: start_options_test_001
 * @tc.desc: test class StartOptions number function
 * @tc.type: FUNC
 */
HWTEST_F(StartOptionsTest, start_options_test_001, TestSize.Level1)
{
    StartOptions startOptions;
    startOptions.SetWindowMode(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FULLSCREEN);
    startOptions.SetDisplayID(TEST_DISPLAY_ID);
    EXPECT_EQ(startOptions.GetWindowMode(), AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FULLSCREEN);
    EXPECT_EQ(startOptions.GetDisplayID(), TEST_DISPLAY_ID);

    StartOptions secondStartOptions(startOptions);
    EXPECT_EQ(startOptions.GetWindowMode(), AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FULLSCREEN);
    EXPECT_EQ(startOptions.GetDisplayID(), TEST_DISPLAY_ID);

    StartOptions thirdStartOptions = secondStartOptions;
    EXPECT_EQ(startOptions.GetWindowMode(), AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FULLSCREEN);
    EXPECT_EQ(startOptions.GetDisplayID(), TEST_DISPLAY_ID);

    Parcel parcel;
    int ret1 = startOptions.Marshalling(parcel);
    EXPECT_EQ(ret1, true);
    auto retOptions = startOptions.Unmarshalling(parcel);
    EXPECT_EQ(retOptions->GetWindowMode(), AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FULLSCREEN);
    EXPECT_EQ(retOptions->GetDisplayID(), TEST_DISPLAY_ID);

    StartOptions fourthStartOptions;
    EXPECT_TRUE(startOptions.Marshalling(parcel));
    auto ret2 = fourthStartOptions.ReadFromParcel(parcel);
    EXPECT_EQ(ret2, true);
    EXPECT_EQ(fourthStartOptions.GetWindowMode(), AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FULLSCREEN);
    EXPECT_EQ(fourthStartOptions.GetDisplayID(), TEST_DISPLAY_ID);
}

/**
 * @tc.name: start_options_test_002
 * @tc.desc: test class StartOptions SetWindowFocused function
 * @tc.type: FUNC
 */
HWTEST_F(StartOptionsTest, start_options_test_002, TestSize.Level1)
{
    StartOptions startOptions;

    bool ret = startOptions.GetWindowFocused();
    EXPECT_TRUE(ret);

    startOptions.SetWindowFocused(TEST_TRUE);
    ret = startOptions.GetWindowFocused();
    EXPECT_TRUE(ret);

    startOptions.SetWindowFocused(TEST_FALSE);
    ret = startOptions.GetWindowFocused();
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: start_options_test_003
 * @tc.desc: test class StartOptions &operator= function
 * @tc.type: FUNC
 */
HWTEST_F(StartOptionsTest, start_options_test_003, TestSize.Level1)
{
    StartOptions firstStartOptions;
    int32_t windowMode = 10;
    firstStartOptions.SetWindowMode(windowMode);
    StartOptions secondStartOptions = firstStartOptions;
    EXPECT_EQ(secondStartOptions.GetWindowMode(), firstStartOptions.windowMode_);
}

/**
 * @tc.name: start_options_test_004
 * @tc.desc: test class StartOptions HideStartWindow = false
 * @tc.type: FUNC
 */
HWTEST_F(StartOptionsTest, start_options_test_004, TestSize.Level1)
{
    StartOptions startOptions;
    bool hideStartWindow = false;
    startOptions.SetHideStartWindow(hideStartWindow);
    EXPECT_EQ(startOptions.GetHideStartWindow(), hideStartWindow);

    Parcel parcel;
    bool marshallResult = startOptions.Marshalling(parcel);
    EXPECT_EQ(marshallResult, true);
    auto unmarshallResult = startOptions.Unmarshalling(parcel);
    EXPECT_EQ(unmarshallResult->GetHideStartWindow(), hideStartWindow);
}

/**
 * @tc.name: start_options_test_005
 * @tc.desc: test class StartOptions HideStartWindow = true
 * @tc.type: FUNC
 */
HWTEST_F(StartOptionsTest, start_options_test_005, TestSize.Level1)
{
    StartOptions startOptions;
    bool hideStartWindow = true;
    startOptions.SetHideStartWindow(hideStartWindow);
    EXPECT_EQ(startOptions.GetHideStartWindow(), hideStartWindow);

    Parcel parcel;
    bool marshallResult = startOptions.Marshalling(parcel);
    EXPECT_EQ(marshallResult, true);
    auto unmarshallResult = startOptions.Unmarshalling(parcel);
    EXPECT_EQ(unmarshallResult->GetHideStartWindow(), hideStartWindow);
}

HWTEST_F(StartOptionsTest, MarshallingTwo_test_001, TestSize.Level1)
{
    StartOptions startOptions;
    Parcel parcel;
    bool marshallResult = startOptions.MarshallingTwo(parcel);
    EXPECT_EQ(marshallResult, true);
}

/**
 * @tc.name: start_options_test_006
 * @tc.desc: test class StartOptions SetCurrentProcessName function
 * @tc.type: FUNC
 */
HWTEST_F(StartOptionsTest, start_options_test_006, TestSize.Level1)
{
    StartOptions startOptions;
    std::string processName = "test";
    startOptions.SetCurrentProcessName(processName);
    EXPECT_EQ(startOptions.GetCurrentProcessName(), processName);
}

/**
 * @tc.name: start_options_test_007
 * @tc.desc: test class StartOptions SetCurrentProcessName function
 * @tc.type: FUNC
 */
HWTEST_F(StartOptionsTest, start_options_test_007, TestSize.Level1)
{
    StartOptions startOptions;
    std::string processName = "";
    startOptions.SetCurrentProcessName(processName);
    EXPECT_EQ(startOptions.GetCurrentProcessName(), processName);
}

/**
 * @tc.name: start_options_test_008
 * @tc.desc: test splitRatioPreference copy and marshalling
 * @tc.type: FUNC
 */
HWTEST_F(StartOptionsTest, start_options_test_008, TestSize.Level1)
{
    StartOptions startOptions;
    startOptions.SetSplitRatioPreference(2);

    StartOptions copiedStartOptions(startOptions);
    EXPECT_EQ(copiedStartOptions.GetSplitRatioPreference(), 2);

    StartOptions assignedStartOptions;
    assignedStartOptions = startOptions;
    EXPECT_EQ(assignedStartOptions.GetSplitRatioPreference(), 2);

    Parcel parcel;
    EXPECT_TRUE(startOptions.Marshalling(parcel));
    std::unique_ptr<StartOptions> unmarshalledOptions(startOptions.Unmarshalling(parcel));
    ASSERT_NE(unmarshalledOptions, nullptr);
    EXPECT_EQ(unmarshalledOptions->GetSplitRatioPreference(), 2);
}
}  // namespace AppExecFwk
}  // namespace OHOS
