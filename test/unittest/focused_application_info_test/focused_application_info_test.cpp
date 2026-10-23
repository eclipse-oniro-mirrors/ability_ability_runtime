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

#define private public
#define protected public
#include "focused_application_info.h"
#include "message_parcel.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class FocusedApplicationInfoTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void FocusedApplicationInfoTest::SetUpTestCase(void) {}
void FocusedApplicationInfoTest::TearDownTestCase(void) {}
void FocusedApplicationInfoTest::SetUp() {}
void FocusedApplicationInfoTest::TearDown() {}

/**
 * @tc.number: Marshalling_0100
 * @tc.desc: Test Marshalling with valid FocusedApplicationInfo.
 * @tc.type: FUNC
 */
HWTEST_F(FocusedApplicationInfoTest, Marshalling_0100, TestSize.Level1)
{
    FocusedApplicationInfo data;
    data.focusedUid = 10001;
    data.focusedBundleName = "com.example.focused";
    MessageParcel parcel;
    EXPECT_TRUE(data.Marshalling(parcel));
}

/**
 * @tc.number: Marshalling_0200
 * @tc.desc: Test Marshalling with default values (-1 and empty strings).
 * @tc.type: FUNC
 */
HWTEST_F(FocusedApplicationInfoTest, Marshalling_0200, TestSize.Level1)
{
    FocusedApplicationInfo data;
    MessageParcel parcel;
    EXPECT_TRUE(data.Marshalling(parcel));
    EXPECT_EQ(data.focusedUid, -1);
}

/**
 * @tc.number: Unmarshalling_0100
 * @tc.desc: Test Unmarshalling with valid data.
 * @tc.type: FUNC
 */
HWTEST_F(FocusedApplicationInfoTest, Unmarshalling_0100, TestSize.Level1)
{
    FocusedApplicationInfo data;
    data.focusedUid = 10001;
    data.focusedBundleName = "com.example.focused";
    MessageParcel parcel;
    EXPECT_TRUE(data.Marshalling(parcel));
    auto result = FocusedApplicationInfo::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->focusedUid, 10001);
    EXPECT_EQ(result->focusedBundleName, "com.example.focused");
    delete result;
}

/**
 * @tc.number: Unmarshalling_0200
 * @tc.desc: Test Unmarshalling with focused bundle name only.
 * @tc.type: FUNC
 */
HWTEST_F(FocusedApplicationInfoTest, Unmarshalling_0200, TestSize.Level1)
{
    FocusedApplicationInfo data;
    data.focusedUid = 10001;
    data.focusedBundleName = "com.example.first";
    MessageParcel parcel;
    EXPECT_TRUE(data.Marshalling(parcel));
    auto result = FocusedApplicationInfo::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->focusedUid, 10001);
    EXPECT_EQ(result->focusedBundleName, "com.example.first");
    delete result;
}
}  // namespace AppExecFwk
}  // namespace OHOS
