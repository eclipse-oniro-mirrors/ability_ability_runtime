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

#include "message_parcel.h"
#include "param.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t TEST_RECORD_ID = 100;
constexpr pid_t TEST_CALLER_PID = 12345;
constexpr pid_t TEST_CALLING_PID = 9999;
constexpr int32_t TEST_LOAD_TIMEOUT = 5000;
constexpr int32_t TEST_BY_CALL_STATUS = 1;
}

class LoadParamTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void LoadParamTest::SetUpTestCase()
{}

void LoadParamTest::TearDownTestCase()
{}

void LoadParamTest::SetUp()
{}

void LoadParamTest::TearDown()
{}

/**
 * @tc.name: LoadParam_ShouldPreserveDefaultReusePidWhenMarshallingWithMinusOne
 * @tc.desc: Test default reusePid (-1) marshalling and unmarshalling
 * @tc.type: FUNC
 */
HWTEST_F(LoadParamTest, LoadParam_ShouldPreserveDefaultReusePidWhenMarshallingWithMinusOne, TestSize.Level1)
{
    MessageParcel parcel;
    LoadParam original;
    original.abilityRecordId = TEST_RECORD_ID;
    original.isShellCall = false;
    original.instanceKey = "testInstanceKey";
    original.isKeepAlive = false;
    original.isKeepAliveAppService = false;
    original.extensionProcessMode = 0;
    original.isStartupHide = false;
    original.reusePid = -1;

    EXPECT_TRUE(original.Marshalling(parcel));

    Parcel &baseParcel = parcel;
    LoadParam *result = LoadParam::Unmarshalling(baseParcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->abilityRecordId, TEST_RECORD_ID);
    EXPECT_EQ(result->isShellCall, false);
    EXPECT_EQ(result->instanceKey, "testInstanceKey");
    EXPECT_EQ(result->reusePid, -1);
    delete result;
}

/**
 * @tc.name: LoadParam_ShouldPreserveReusePidWhenMarshallingWithPositiveValue
 * @tc.desc: Test reusePid with positive value marshalling and unmarshalling
 * @tc.type: FUNC
 */
HWTEST_F(LoadParamTest, LoadParam_ShouldPreserveReusePidWhenMarshallingWithPositiveValue, TestSize.Level1)
{
    MessageParcel parcel;
    LoadParam original;
    original.abilityRecordId = TEST_RECORD_ID;
    original.isShellCall = true;
    original.instanceKey = "instanceKey123";
    original.reusePid = TEST_CALLER_PID;
    original.callingPid = TEST_CALLING_PID;
    original.loadTimeout = TEST_LOAD_TIMEOUT;
    original.byCallStatus = TEST_BY_CALL_STATUS;

    EXPECT_TRUE(original.Marshalling(parcel));

    Parcel &baseParcel = parcel;
    LoadParam *result = LoadParam::Unmarshalling(baseParcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->abilityRecordId, TEST_RECORD_ID);
    EXPECT_EQ(result->isShellCall, true);
    EXPECT_EQ(result->instanceKey, "instanceKey123");
    EXPECT_EQ(result->reusePid, TEST_CALLER_PID);
    EXPECT_EQ(result->callingPid, TEST_CALLING_PID);
    EXPECT_EQ(result->loadTimeout, TEST_LOAD_TIMEOUT);
    EXPECT_EQ(result->byCallStatus, TEST_BY_CALL_STATUS);
    delete result;
}

/**
 * @tc.name: LoadParam_ShouldPreserveAllFieldsWhenMarshallingFullRoundTrip
 * @tc.desc: Test full round-trip with all fields set
 * @tc.type: FUNC
 */
HWTEST_F(LoadParamTest, LoadParam_ShouldPreserveAllFieldsWhenMarshallingFullRoundTrip, TestSize.Level1)
{
    MessageParcel parcel;
    LoadParam original;
    original.abilityRecordId = 42;
    original.isShellCall = true;
    original.instanceKey = "roundTripKey";
    original.isKeepAlive = true;
    original.isKeepAliveAppService = true;
    original.extensionProcessMode = 2;
    original.isStartupHide = true;
    original.isMainElementRunning = true;
    original.callingPid = 1111;
    original.loadAbilityCallbackId = 2222;
    original.isPrelaunch = true;
    original.isPreloadStart = true;
    original.loadTimeout = 3333;
    original.byCallStatus = 2;
    original.reusePid = 4444;

    EXPECT_TRUE(original.Marshalling(parcel));

    Parcel &baseParcel = parcel;
    LoadParam *result = LoadParam::Unmarshalling(baseParcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->abilityRecordId, 42);
    EXPECT_EQ(result->isShellCall, true);
    EXPECT_EQ(result->instanceKey, "roundTripKey");
    EXPECT_EQ(result->isKeepAlive, true);
    EXPECT_EQ(result->isKeepAliveAppService, true);
    EXPECT_EQ(result->extensionProcessMode, 2u);
    EXPECT_EQ(result->isStartupHide, true);
    EXPECT_EQ(result->isMainElementRunning, true);
    EXPECT_EQ(result->callingPid, 1111);
    EXPECT_EQ(result->loadAbilityCallbackId, 2222u);
    EXPECT_EQ(result->isPrelaunch, true);
    EXPECT_EQ(result->isPreloadStart, true);
    EXPECT_EQ(result->loadTimeout, 3333);
    EXPECT_EQ(result->byCallStatus, 2);
    EXPECT_EQ(result->reusePid, 4444);
    delete result;
}

/**
 * @tc.name: LoadParam_ShouldPreserveReusePidWhenMarshallingWithZeroValue
 * @tc.desc: Test reusePid = 0 boundary value preserved after marshalling
 * @tc.type: FUNC
 */
HWTEST_F(LoadParamTest, LoadParam_ShouldPreserveReusePidWhenMarshallingWithZeroValue, TestSize.Level1)
{
    MessageParcel parcel;
    LoadParam original;
    // Use all defaults, only set reusePid
    original.reusePid = 0;

    EXPECT_TRUE(original.Marshalling(parcel));

    Parcel &baseParcel = parcel;
    LoadParam *result = LoadParam::Unmarshalling(baseParcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->reusePid, 0);
    EXPECT_EQ(result->abilityRecordId, -1);
    EXPECT_EQ(result->callingPid, -1);
    EXPECT_EQ(result->loadTimeout, 0);
    EXPECT_EQ(result->byCallStatus, 0);
    delete result;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
