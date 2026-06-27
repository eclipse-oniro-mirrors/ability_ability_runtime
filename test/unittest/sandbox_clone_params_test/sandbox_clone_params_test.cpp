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

#include "sandbox_clone_params.h"
#include "parcel.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;

namespace {
const std::string TEST_CALLER_BUNDLE_NAME = "com.test.caller.bundle";
const std::string EMPTY_STRING = "";
const int32_t TEST_CALLER_UID = 10001;
const uint32_t TEST_CALLER_TOKEN_ID = 12345678;
const int32_t TEST_INVALID_UID = -1;
}  // namespace

class SandboxCloneParamsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SandboxCloneParamsTest::SetUpTestCase()
{}

void SandboxCloneParamsTest::TearDownTestCase()
{}

void SandboxCloneParamsTest::SetUp()
{}

void SandboxCloneParamsTest::TearDown()
{}

/**
 * @tc.name: SandboxCloneParams_001
 * @tc.desc: Test SandboxCloneParams default construction
 * @tc.type: FUNC
 */
HWTEST_F(SandboxCloneParamsTest, SandboxCloneParams_001, TestSize.Level1)
{
    SandboxCloneParams params;
    EXPECT_TRUE(params.callerBundleName.empty());
    EXPECT_EQ(params.callerUid, -1);
    EXPECT_EQ(params.callerTokenId, 0);
}

/**
 * @tc.name: SandboxCloneParams_002
 * @tc.desc: Test SandboxCloneParams with values
 * @tc.type: FUNC
 */
HWTEST_F(SandboxCloneParamsTest, SandboxCloneParams_002, TestSize.Level1)
{
    SandboxCloneParams params;
    params.callerBundleName = TEST_CALLER_BUNDLE_NAME;
    params.callerUid = TEST_CALLER_UID;
    params.callerTokenId = TEST_CALLER_TOKEN_ID;

    EXPECT_EQ(params.callerBundleName, TEST_CALLER_BUNDLE_NAME);
    EXPECT_EQ(params.callerUid, TEST_CALLER_UID);
    EXPECT_EQ(params.callerTokenId, TEST_CALLER_TOKEN_ID);
}

/**
 * @tc.name: SandboxCloneParams_Marshalling_001
 * @tc.desc: Test SandboxCloneParams Marshalling with valid data
 * @tc.type: FUNC
 */
HWTEST_F(SandboxCloneParamsTest, SandboxCloneParams_Marshalling_001, TestSize.Level1)
{
    SandboxCloneParams params;
    params.callerBundleName = TEST_CALLER_BUNDLE_NAME;
    params.callerUid = TEST_CALLER_UID;
    params.callerTokenId = TEST_CALLER_TOKEN_ID;

    Parcel parcel;
    bool result = params.Marshalling(parcel);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: SandboxCloneParams_Marshalling_002
 * @tc.desc: Test SandboxCloneParams Marshalling with empty bundle name
 * @tc.type: FUNC
 */
HWTEST_F(SandboxCloneParamsTest, SandboxCloneParams_Marshalling_002, TestSize.Level1)
{
    SandboxCloneParams params;
    params.callerBundleName = EMPTY_STRING;
    params.callerUid = TEST_CALLER_UID;
    params.callerTokenId = TEST_CALLER_TOKEN_ID;

    Parcel parcel;
    bool result = params.Marshalling(parcel);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: SandboxCloneParams_Marshalling_003
 * @tc.desc: Test SandboxCloneParams Marshalling with negative UID
 * @tc.type: FUNC
 */
HWTEST_F(SandboxCloneParamsTest, SandboxCloneParams_Marshalling_003, TestSize.Level1)
{
    SandboxCloneParams params;
    params.callerBundleName = TEST_CALLER_BUNDLE_NAME;
    params.callerUid = TEST_INVALID_UID;
    params.callerTokenId = TEST_CALLER_TOKEN_ID;

    Parcel parcel;
    bool result = params.Marshalling(parcel);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: SandboxCloneParams_Marshalling_004
 * @tc.desc: Test SandboxCloneParams Marshalling with zero token ID
 * @tc.type: FUNC
 */
HWTEST_F(SandboxCloneParamsTest, SandboxCloneParams_Marshalling_004, TestSize.Level1)
{
    SandboxCloneParams params;
    params.callerBundleName = TEST_CALLER_BUNDLE_NAME;
    params.callerUid = TEST_CALLER_UID;
    params.callerTokenId = 0;

    Parcel parcel;
    bool result = params.Marshalling(parcel);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: SandboxCloneParams_ReadFromParcel_001
 * @tc.desc: Test SandboxCloneParams ReadFromParcel with valid data
 * @tc.type: FUNC
 */
HWTEST_F(SandboxCloneParamsTest, SandboxCloneParams_ReadFromParcel_001, TestSize.Level1)
{
    SandboxCloneParams originalParams;
    originalParams.callerBundleName = TEST_CALLER_BUNDLE_NAME;
    originalParams.callerUid = TEST_CALLER_UID;
    originalParams.callerTokenId = TEST_CALLER_TOKEN_ID;

    Parcel parcel;
    ASSERT_TRUE(originalParams.Marshalling(parcel));

    SandboxCloneParams readParams;
    bool result = readParams.ReadFromParcel(parcel);
    EXPECT_TRUE(result);
    EXPECT_EQ(readParams.callerBundleName, TEST_CALLER_BUNDLE_NAME);
    EXPECT_EQ(readParams.callerUid, TEST_CALLER_UID);
    EXPECT_EQ(readParams.callerTokenId, TEST_CALLER_TOKEN_ID);
}

/**
 * @tc.name: SandboxCloneParams_ReadFromParcel_002
 * @tc.desc: Test SandboxCloneParams ReadFromParcel with empty bundle name
 * @tc.type: FUNC
 */
HWTEST_F(SandboxCloneParamsTest, SandboxCloneParams_ReadFromParcel_002, TestSize.Level1)
{
    SandboxCloneParams originalParams;
    originalParams.callerBundleName = EMPTY_STRING;
    originalParams.callerUid = TEST_CALLER_UID;
    originalParams.callerTokenId = TEST_CALLER_TOKEN_ID;

    Parcel parcel;
    ASSERT_TRUE(originalParams.Marshalling(parcel));

    SandboxCloneParams readParams;
    bool result = readParams.ReadFromParcel(parcel);
    EXPECT_TRUE(result);
    EXPECT_TRUE(readParams.callerBundleName.empty());
    EXPECT_EQ(readParams.callerUid, TEST_CALLER_UID);
    EXPECT_EQ(readParams.callerTokenId, TEST_CALLER_TOKEN_ID);
}

/**
 * @tc.name: SandboxCloneParams_ReadFromParcel_003
 * @tc.desc: Test SandboxCloneParams ReadFromParcel with negative UID
 * @tc.type: FUNC
 */
HWTEST_F(SandboxCloneParamsTest, SandboxCloneParams_ReadFromParcel_003, TestSize.Level1)
{
    SandboxCloneParams originalParams;
    originalParams.callerBundleName = TEST_CALLER_BUNDLE_NAME;
    originalParams.callerUid = TEST_INVALID_UID;
    originalParams.callerTokenId = TEST_CALLER_TOKEN_ID;

    Parcel parcel;
    ASSERT_TRUE(originalParams.Marshalling(parcel));

    SandboxCloneParams readParams;
    bool result = readParams.ReadFromParcel(parcel);
    EXPECT_TRUE(result);
    EXPECT_EQ(readParams.callerBundleName, TEST_CALLER_BUNDLE_NAME);
    EXPECT_EQ(readParams.callerUid, TEST_INVALID_UID);
    EXPECT_EQ(readParams.callerTokenId, TEST_CALLER_TOKEN_ID);
}

/**
 * @tc.name: SandboxCloneParams_ReadFromParcel_004
 * @tc.desc: Test SandboxCloneParams ReadFromParcel with maximum values
 * @tc.type: FUNC
 */
HWTEST_F(SandboxCloneParamsTest, SandboxCloneParams_ReadFromParcel_004, TestSize.Level1)
{
    SandboxCloneParams originalParams;
    originalParams.callerBundleName = TEST_CALLER_BUNDLE_NAME;
    originalParams.callerUid = INT32_MAX;
    originalParams.callerTokenId = UINT32_MAX;

    Parcel parcel;
    ASSERT_TRUE(originalParams.Marshalling(parcel));

    SandboxCloneParams readParams;
    bool result = readParams.ReadFromParcel(parcel);
    EXPECT_TRUE(result);
    EXPECT_EQ(readParams.callerBundleName, TEST_CALLER_BUNDLE_NAME);
    EXPECT_EQ(readParams.callerUid, INT32_MAX);
    EXPECT_EQ(readParams.callerTokenId, UINT32_MAX);
}

/**
 * @tc.name: SandboxCloneParams_Unmarshalling_001
 * @tc.desc: Test SandboxCloneParams Unmarshalling with valid data
 * @tc.type: FUNC
 */
HWTEST_F(SandboxCloneParamsTest, SandboxCloneParams_Unmarshalling_001, TestSize.Level1)
{
    SandboxCloneParams originalParams;
    originalParams.callerBundleName = TEST_CALLER_BUNDLE_NAME;
    originalParams.callerUid = TEST_CALLER_UID;
    originalParams.callerTokenId = TEST_CALLER_TOKEN_ID;

    Parcel parcel;
    ASSERT_TRUE(originalParams.Marshalling(parcel));

    SandboxCloneParams* unmarshalledParams = SandboxCloneParams::Unmarshalling(parcel);
    ASSERT_NE(unmarshalledParams, nullptr);
    EXPECT_EQ(unmarshalledParams->callerBundleName, TEST_CALLER_BUNDLE_NAME);
    EXPECT_EQ(unmarshalledParams->callerUid, TEST_CALLER_UID);
    EXPECT_EQ(unmarshalledParams->callerTokenId, TEST_CALLER_TOKEN_ID);
    delete unmarshalledParams;
}

/**
 * @tc.name: SandboxCloneParams_Unmarshalling_002
 * @tc.desc: Test SandboxCloneParams Unmarshalling with empty parcel
 * @tc.type: FUNC
 */
HWTEST_F(SandboxCloneParamsTest, SandboxCloneParams_Unmarshalling_002, TestSize.Level1)
{
    Parcel parcel;
    // Empty parcel

    SandboxCloneParams* unmarshalledParams = SandboxCloneParams::Unmarshalling(parcel);
    // Unmarshalling should succeed but with empty/default values
    ASSERT_NE(unmarshalledParams, nullptr);
    delete unmarshalledParams;
}

/**
 * @tc.name: SandboxCloneParams_Unmarshalling_003
 * @tc.desc: Test SandboxCloneParams Unmarshalling round trip
 * @tc.type: FUNC
 */
HWTEST_F(SandboxCloneParamsTest, SandboxCloneParams_Unmarshalling_003, TestSize.Level1)
{
    SandboxCloneParams originalParams;
    originalParams.callerBundleName = "com.example.bundle";
    originalParams.callerUid = 99999;
    originalParams.callerTokenId = 88888888;

    Parcel parcel;
    ASSERT_TRUE(originalParams.Marshalling(parcel));

    SandboxCloneParams* unmarshalledParams = SandboxCloneParams::Unmarshalling(parcel);
    ASSERT_NE(unmarshalledParams, nullptr);
    EXPECT_EQ(unmarshalledParams->callerBundleName, "com.example.bundle");
    EXPECT_EQ(unmarshalledParams->callerUid, 99999);
    EXPECT_EQ(unmarshalledParams->callerTokenId, 88888888);
    delete unmarshalledParams;
}

/**
 * @tc.name: SandboxCloneParams_RoundTrip_001
 * @tc.desc: Test SandboxCloneParams complete round trip (Marshalling -> Unmarshalling)
 * @tc.type: FUNC
 */
HWTEST_F(SandboxCloneParamsTest, SandboxCloneParams_RoundTrip_001, TestSize.Level1)
{
    SandboxCloneParams originalParams;
    originalParams.callerBundleName = TEST_CALLER_BUNDLE_NAME;
    originalParams.callerUid = TEST_CALLER_UID;
    originalParams.callerTokenId = TEST_CALLER_TOKEN_ID;

    Parcel parcel;
    ASSERT_TRUE(originalParams.Marshalling(parcel));

    SandboxCloneParams* restoredParams = SandboxCloneParams::Unmarshalling(parcel);
    ASSERT_NE(restoredParams, nullptr);
    EXPECT_EQ(restoredParams->callerBundleName, originalParams.callerBundleName);
    EXPECT_EQ(restoredParams->callerUid, originalParams.callerUid);
    EXPECT_EQ(restoredParams->callerTokenId, originalParams.callerTokenId);
    delete restoredParams;
}

/**
 * @tc.name: SandboxCloneParams_LongString_001
 * @tc.desc: Test SandboxCloneParams with very long bundle name
 * @tc.type: FUNC
 */
HWTEST_F(SandboxCloneParamsTest, SandboxCloneParams_LongString_001, TestSize.Level1)
{
    SandboxCloneParams params;
    std::string longBundleName(1000, 'a');  // 1000 character string
    params.callerBundleName = longBundleName;
    params.callerUid = TEST_CALLER_UID;
    params.callerTokenId = TEST_CALLER_TOKEN_ID;

    Parcel parcel;
    bool result = params.Marshalling(parcel);
    EXPECT_TRUE(result);

    SandboxCloneParams* readParams = SandboxCloneParams::Unmarshalling(parcel);
    ASSERT_NE(readParams, nullptr);
    EXPECT_EQ(readParams->callerBundleName, longBundleName);
    delete readParams;
}

/**
 * @tc.name: SandboxCloneParams_SpecialCharacters_001
 * @tc.desc: Test SandboxCloneParams with special characters in bundle name
 * @tc.type: FUNC
 */
HWTEST_F(SandboxCloneParamsTest, SandboxCloneParams_SpecialCharacters_001, TestSize.Level1)
{
    SandboxCloneParams params;
    std::string specialBundleName = "com.test.bund!e@nam#e$v%i&l*";
    params.callerBundleName = specialBundleName;
    params.callerUid = TEST_CALLER_UID;
    params.callerTokenId = TEST_CALLER_TOKEN_ID;

    Parcel parcel;
    bool result = params.Marshalling(parcel);
    EXPECT_TRUE(result);

    SandboxCloneParams* readParams = SandboxCloneParams::Unmarshalling(parcel);
    ASSERT_NE(readParams, nullptr);
    EXPECT_EQ(readParams->callerBundleName, specialBundleName);
    delete readParams;
}

/**
 * @tc.name: SandboxCloneParams_MultipleOperations_001
 * @tc.desc: Test multiple Marshalling and Unmarshalling operations
 * @tc.type: FUNC
 */
HWTEST_F(SandboxCloneParamsTest, SandboxCloneParams_MultipleOperations_001, TestSize.Level1)
{
    Parcel parcel;

    for (int i = 0; i < 10; i++) {
        SandboxCloneParams params;
        params.callerBundleName = "com.bundle" + std::to_string(i);
        params.callerUid = 10000 + i;
        params.callerTokenId = 100000 + i;

        ASSERT_TRUE(params.Marshalling(parcel));
    }

    // Read back all params
    for (int i = 0; i < 10; i++) {
        SandboxCloneParams* params = SandboxCloneParams::Unmarshalling(parcel);
        ASSERT_NE(params, nullptr);
        EXPECT_EQ(params->callerBundleName, "com.bundle" + std::to_string(i));
        EXPECT_EQ(params->callerUid, 10000 + i);
        EXPECT_EQ(params->callerTokenId, 100000 + i);
        delete params;
    }
}
