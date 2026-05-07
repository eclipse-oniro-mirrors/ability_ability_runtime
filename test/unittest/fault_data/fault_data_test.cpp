/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#define private public
#include "fault_data.h"
#include "message_parcel.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class FaultDataTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void FaultDataTest::SetUpTestCase(void)
{}

void FaultDataTest::TearDownTestCase(void)
{}

void FaultDataTest::SetUp()
{}

void FaultDataTest::TearDown()
{}

void TestFaultDataUnmarshalling(MessageParcel& message)
{
    std::string helloWord = "HelloWord";
    int testValue1 = 12; // test value
    int testValue2 = 1; // test value
    uint64_t testValue3 = 12; // test value
    uint64_t testValue4 = 32; // test value
    message.WriteBool(true);
    message.WriteString(helloWord);
    message.WriteString(helloWord);
    message.WriteString(helloWord);
    message.WriteBool(true);
    message.WriteBool(true);
    message.WriteBool(true);
    message.WriteString(helloWord);
    message.WriteString(helloWord);
    message.WriteInt32(testValue1);
    message.WriteInt32(testValue1);
    message.WriteInt32(testValue1);
    message.WriteInt32(testValue2);
    message.WriteUint64(testValue3);
    message.WriteUint64(testValue3);
    message.WriteUint64(testValue3);
    message.WriteUint64(testValue3);
    message.WriteUint64(testValue3);
    message.WriteUint64(testValue3);
    message.WriteUint64(testValue3);
    message.WriteString(helloWord);
    message.WriteUint64(testValue4);
}

/**
 * @tc.name: ReadFromParcel_001
 * @tc.desc: Verify that the ReadFromParcel interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(FaultDataTest, ReadFromParcel_001, TestSize.Level1)
{
    auto faultData = std::make_shared<FaultData>();
    MessageParcel messageFirst;
    bool retFirst = faultData->ReadFromParcel(messageFirst);
    EXPECT_EQ(false, retFirst);

    MessageParcel messageSecond;
    std::string helloWord = "HelloWord";
    messageSecond.WriteString(helloWord);
    bool retSecond = faultData->ReadFromParcel(messageSecond);
    EXPECT_EQ(false, retSecond);

    MessageParcel messageThird;
    messageThird.WriteString(helloWord);
    messageThird.WriteString(helloWord);
    bool retThird = faultData->ReadFromParcel(messageThird);
    EXPECT_EQ(false, retThird);

    MessageParcel messageFourth;
    messageFourth.WriteString(helloWord);
    messageFourth.WriteString(helloWord);
    messageFourth.WriteString(helloWord);
    bool retFourth = faultData->ReadFromParcel(messageFourth);
    EXPECT_EQ(false, retFourth);

    MessageParcel messageFifth;
    messageFifth.WriteString(helloWord);
    messageFifth.WriteString(helloWord);
    messageFifth.WriteString(helloWord);
    messageFifth.WriteInt32(12);
    bool retFifth = faultData->ReadFromParcel(messageFifth);
    EXPECT_EQ(false, retFifth);
}

/**
 * @tc.name: FaultDataTest_ReadFromParcel_002
 * @tc.desc: Verify that the ReadFromParcel interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(FaultDataTest, FaultDataTest_ReadFromParcel_002, TestSize.Level1)
{
    auto faultData = std::make_shared<FaultData>();
    std::string helloWord = "HelloWord";
    MessageParcel messageSixth;
    messageSixth.WriteString(helloWord);
    messageSixth.WriteString(helloWord);
    messageSixth.WriteString(helloWord);
    messageSixth.WriteString(helloWord);
    messageSixth.WriteInt32(1);
    messageSixth.WriteString(helloWord);
    messageSixth.WriteBool(true);
    messageSixth.WriteBool(true);
    messageSixth.WriteBool(true);
    messageSixth.WriteBool(true);
    messageSixth.WriteUint32(12);
    messageSixth.WriteInt32(12);
    messageSixth.WriteUint64(12);
    messageSixth.WriteUint64(12);
    messageSixth.WriteInt32(12);
    messageSixth.WriteUint64(12);
    messageSixth.WriteUint64(12);
    messageSixth.WriteUint32(12);
    messageSixth.WriteInt32(12);
    messageSixth.WriteInt32(12);
    messageSixth.WriteUint32(12);
    messageSixth.WriteBool(true);
    messageSixth.WriteString(helloWord);
    messageSixth.WriteString(helloWord);
    messageSixth.WriteString(helloWord);
    messageSixth.WriteBool(true);
    messageSixth.WriteBool(true);
    messageSixth.WriteBool(true);
    messageSixth.WriteString(helloWord);
    messageSixth.WriteString(helloWord);
    messageSixth.WriteInt32(12);
    messageSixth.WriteInt32(12);
    messageSixth.WriteInt32(12);
    messageSixth.WriteInt32(1);
    messageSixth.WriteUint64(12);
    messageSixth.WriteUint64(12);
    messageSixth.WriteUint64(12);
    messageSixth.WriteUint64(12);
    messageSixth.WriteUint64(12);
    messageSixth.WriteUint64(12);
    messageSixth.WriteUint64(12);
    messageSixth.WriteString(helloWord);
    messageSixth.WriteUint64(32);
    bool retSixth = faultData->ReadFromParcel(messageSixth);
    EXPECT_EQ(true, retSixth);
}

/**
 * @tc.name: Unmarshalling_001
 * @tc.desc: Verify that the Unmarshalling interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(FaultDataTest, Unmarshalling_001, TestSize.Level1)
{
    auto faultData = std::make_shared<FaultData>();
    MessageParcel message;
    auto retFirst = faultData->Unmarshalling(message);
    EXPECT_EQ(nullptr, retFirst);

    std::string helloWord = "HelloWord";
    message.WriteString(helloWord);
    message.WriteString(helloWord);
    message.WriteString(helloWord);
    message.WriteString(helloWord);
    message.WriteInt32(1);
    message.WriteString(helloWord);
    message.WriteBool(true);
    message.WriteBool(true);
    message.WriteBool(true);
    message.WriteBool(true);
    message.WriteUint32(12);
    message.WriteInt32(12);
    message.WriteUint32(12);
    message.WriteInt32(12);
    message.WriteUint64(12);
    message.WriteUint64(12);
    message.WriteInt32(12);
    message.WriteUint64(12);
    message.WriteUint64(12);
    message.WriteInt32(12);
    message.WriteUint32(12);
    TestFaultDataUnmarshalling(message);
    auto retSecond = faultData->Unmarshalling(message);
    EXPECT_NE(nullptr, retSecond);
}

/**
 * @tc.name: Marshalling_001
 * @tc.desc: Verify that the Marshalling interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(FaultDataTest, Marshalling_001, TestSize.Level1)
{
    auto faultData = std::make_shared<FaultData>();
    faultData->errorObject.name = "1234";
    faultData->errorObject.message = "5678";
    faultData->errorObject.stack = "90";

    MessageParcel message;
    bool ret = faultData->Marshalling(message);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name: ReadFromParcel_002
 * @tc.desc: Verify that the ReadFromParcel interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(FaultDataTest, ReadFromParcel_002, TestSize.Level1)
{
    auto appFaultDataBySA = std::make_shared<AppFaultDataBySA>();
    MessageParcel messageFirst;
    bool retFirst = appFaultDataBySA->ReadFromParcel(messageFirst);
    EXPECT_EQ(false, retFirst);

    MessageParcel messageSecond;
    std::string helloWord = "HelloWord";
    messageSecond.WriteString(helloWord);
    bool retSecond = appFaultDataBySA->ReadFromParcel(messageSecond);
    EXPECT_EQ(false, retSecond);

    MessageParcel messageThird;
    messageThird.WriteString(helloWord);
    messageThird.WriteString(helloWord);
    bool retThird = appFaultDataBySA->ReadFromParcel(messageThird);
    EXPECT_EQ(false, retThird);

    MessageParcel messageFourth;
    messageFourth.WriteString(helloWord);
    messageFourth.WriteString(helloWord);
    messageFourth.WriteString(helloWord);
    bool retFourth = appFaultDataBySA->ReadFromParcel(messageFourth);
    EXPECT_EQ(false, retFourth);

    MessageParcel messageFifth;
    messageFifth.WriteString(helloWord);
    messageFifth.WriteString(helloWord);
    messageFifth.WriteString(helloWord);
    messageFifth.WriteInt32(12);
    bool retFifth = appFaultDataBySA->ReadFromParcel(messageFifth);
    EXPECT_EQ(false, retFifth);
}

/**
 * @tc.name: ReadFromParcel_003
 * @tc.desc: Verify that the ReadFromParcel interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(FaultDataTest, ReadFromParcel_003, TestSize.Level1)
{
    auto appFaultDataBySA = std::make_shared<AppFaultDataBySA>();
    std::string helloWord = "HelloWord";
    MessageParcel messageSixth;
    messageSixth.WriteString(helloWord);
    messageSixth.WriteString(helloWord);
    messageSixth.WriteString(helloWord);
    messageSixth.WriteString(helloWord);
    messageSixth.WriteInt32(1);
    messageSixth.WriteInt32(1234);
    messageSixth.WriteString(helloWord);
    messageSixth.WriteBool(true);
    messageSixth.WriteBool(true);
    messageSixth.WriteBool(true);
    messageSixth.WriteBool(true);
    messageSixth.WriteUint32(12);
    messageSixth.WriteInt32(12);
    messageSixth.WriteUint64(12);
    messageSixth.WriteUint64(12);
    messageSixth.WriteInt32(12);
    messageSixth.WriteBool(true);
    messageSixth.WriteString(helloWord);
    messageSixth.WriteString(helloWord);
    messageSixth.WriteString(helloWord);
    messageSixth.WriteBool(true);
    messageSixth.WriteBool(true);
    messageSixth.WriteBool(true);
    messageSixth.WriteString(helloWord);
    messageSixth.WriteString(helloWord);
    messageSixth.WriteInt32(1);
    messageSixth.WriteUint64(12);
    messageSixth.WriteUint64(12);
    messageSixth.WriteUint64(12);
    messageSixth.WriteUint64(12);
    messageSixth.WriteUint64(12);
    messageSixth.WriteUint64(12);
    messageSixth.WriteUint64(12);
    bool retSixth = appFaultDataBySA->ReadFromParcel(messageSixth);
    EXPECT_EQ(true, retSixth);
}

/**
 * @tc.name: Unmarshalling_002
 * @tc.desc: Verify that the Unmarshalling interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(FaultDataTest, Unmarshalling_002, TestSize.Level1)
{
    auto appFaultDataBySA = std::make_shared<AppFaultDataBySA>();
    MessageParcel message;
    auto retFirst = appFaultDataBySA->Unmarshalling(message);
    EXPECT_EQ(nullptr, retFirst);

    std::string helloWord = "HelloWord";
    message.WriteString(helloWord);
    message.WriteString(helloWord);
    message.WriteString(helloWord);
    message.WriteString(helloWord);
    message.WriteInt32(1);
    message.WriteInt32(1234);
    message.WriteString(helloWord);
    message.WriteBool(true);
    message.WriteBool(true);
    message.WriteBool(true);
    message.WriteBool(true);
    message.WriteUint32(12);
    message.WriteInt32(12);
    message.WriteUint64(12);
    message.WriteUint64(12);
    message.WriteInt32(12);
    message.WriteBool(true);
    message.WriteString(helloWord);
    message.WriteString(helloWord);
    message.WriteString(helloWord);
    message.WriteBool(true);
    message.WriteBool(true);
    message.WriteBool(true);
    message.WriteString(helloWord);
    message.WriteString(helloWord);
    message.WriteInt32(1);
    message.WriteUint64(12);
    message.WriteUint64(12);
    message.WriteUint64(12);
    message.WriteUint64(12);
    message.WriteUint64(12);
    message.WriteUint64(12);
    message.WriteUint64(12);
    auto retSecond = appFaultDataBySA->Unmarshalling(message);
    EXPECT_NE(nullptr, retSecond);
}

/**
 * @tc.name: Marshalling_002
 * @tc.desc: Verify that the Marshalling interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(FaultDataTest, Marshalling_002, TestSize.Level1)
{
    auto appFaultDataBySA = std::make_shared<AppFaultDataBySA>();
    appFaultDataBySA->errorObject.name = "1234";
    appFaultDataBySA->errorObject.message = "5678";
    appFaultDataBySA->errorObject.stack = "90";

    MessageParcel message;
    bool ret = appFaultDataBySA->Marshalling(message);
    EXPECT_EQ(true, ret);
}
} // namespace AppExecFwk
} // namespace OHOS
