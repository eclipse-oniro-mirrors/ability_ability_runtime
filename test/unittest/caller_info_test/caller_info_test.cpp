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

#include "caller_info.h"
#include "parcel.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
namespace {
    constexpr uint32_t TOKEN_ID = 123456;
    constexpr int32_t CALLER_UID = 1000;
    constexpr int32_t CALLER_PID = 8888;
}

class IndirectCallerInfoTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<IndirectCallerInfo> indirectCallerInfo_;
};

void IndirectCallerInfoTest::SetUpTestCase(void)
{}

void IndirectCallerInfoTest::TearDownTestCase(void)
{}

void IndirectCallerInfoTest::SetUp()
{
    indirectCallerInfo_ = new IndirectCallerInfo();
}

void IndirectCallerInfoTest::TearDown()
{}

/**
 * @tc.name: ReadFromParcel_0100
 * @tc.desc: Verify read data from parcel normally.
 * @tc.type: FUNC
 */
HWTEST_F(IndirectCallerInfoTest, ReadFromParcel_0100, TestSize.Level1)
{
    EXPECT_NE(indirectCallerInfo_, nullptr);
    Parcel data;
    data.WriteUint32(TOKEN_ID);
    data.WriteInt32(CALLER_UID);
    data.WriteInt32(CALLER_PID);

    EXPECT_TRUE(indirectCallerInfo_->ReadFromParcel(data));
    EXPECT_EQ(indirectCallerInfo_->tokenId, TOKEN_ID);
    EXPECT_EQ(indirectCallerInfo_->callerUid, CALLER_UID);
    EXPECT_EQ(indirectCallerInfo_->callerPid, CALLER_PID);
}

/**
 * @tc.name: UnMarshalling_0100
 * @tc.desc: Verify unmarshall parcel normally.
 * @tc.type: FUNC
 */
HWTEST_F(IndirectCallerInfoTest, UnMarshalling_0100, TestSize.Level1)
{
    EXPECT_NE(indirectCallerInfo_, nullptr);
    Parcel data;
    data.WriteUint32(TOKEN_ID);
    data.WriteInt32(CALLER_UID);
    data.WriteInt32(CALLER_PID);

    auto info = IndirectCallerInfo::Unmarshalling(data);
    EXPECT_NE(info, nullptr);
    EXPECT_EQ(info->tokenId, TOKEN_ID);
    EXPECT_EQ(info->callerUid, CALLER_UID);
    EXPECT_EQ(info->callerPid, CALLER_PID);
    delete info;
}

/**
 * @tc.name: Marshalling_0100
 * @tc.desc: Verify marshall into parcel normally.
 * @tc.type: FUNC
 */
HWTEST_F(IndirectCallerInfoTest, Marshalling_0100, TestSize.Level1)
{
    EXPECT_NE(indirectCallerInfo_, nullptr);
    Parcel data;
    indirectCallerInfo_->tokenId = TOKEN_ID;
    indirectCallerInfo_->callerUid = CALLER_UID;
    indirectCallerInfo_->callerPid = CALLER_PID;

    EXPECT_TRUE(indirectCallerInfo_->Marshalling(data));
    EXPECT_EQ(data.ReadUint32(), TOKEN_ID);
    EXPECT_EQ(data.ReadInt32(), CALLER_UID);
    EXPECT_EQ(data.ReadInt32(), CALLER_PID);
}
} // namespace AAFwk
} // namespace OHOS