/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "parcel.h"
#define private public
#define protected public
#include "want_sender_info.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using OHOS::AppExecFwk::ElementName;

namespace OHOS {
namespace AAFwk {
class WantSenderInfoTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void WantSenderInfoTest::SetUpTestCase()
{}

void WantSenderInfoTest::TearDownTestCase()
{}

void WantSenderInfoTest::SetUp()
{}

void WantSenderInfoTest::TearDown()
{}

/*
 * @tc.number    : WantSenderInfoTest_0100
 * @tc.name      : Marshalling/UnMarshalling
 * @tc.desc      : 1.Marshalling/UnMarshalling
 */
HWTEST_F(WantSenderInfoTest, WantSenderInfoTest_0100, TestSize.Level1)
{
    WantSenderInfo info;
    info.type = 10;
    info.bundleName = "bundleName";
    info.resultWho = "abilityA";
    info.requestCode = 100;
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantsInfo wantInfo;
    wantInfo.want = want;
    wantInfo.resolvedTypes = "nihao";
    info.allWants.emplace_back(wantInfo);
    info.flags = 3;
    info.userId = 99;
    Parcel parcel;
    info.Marshalling(parcel);
    auto unInfo = WantSenderInfo::Unmarshalling(parcel);
    EXPECT_NE(unInfo, nullptr);
    if (!unInfo) {
        return;
    }
    EXPECT_EQ(unInfo->type, 10);
    EXPECT_EQ(unInfo->bundleName, "bundleName");
    EXPECT_EQ(unInfo->resultWho, "abilityA");
    EXPECT_EQ(static_cast<int>(unInfo->allWants.size()), 1);
    EXPECT_EQ(unInfo->allWants.at(0).want.GetElement().GetBundleName(), "com.ix.hiMusic");
    EXPECT_EQ(unInfo->allWants.at(0).want.GetElement().GetAbilityName(), "MusicSAbility");
    EXPECT_EQ(unInfo->allWants.at(0).resolvedTypes, "nihao");
    EXPECT_EQ(unInfo->requestCode, 100);
    EXPECT_EQ(static_cast<int>(unInfo->flags), 3);
    EXPECT_EQ(unInfo->userId, 99);
    delete unInfo;
}

/*
 * @tc.number    : WantSenderInfoTest_0200
 * @tc.name      : Marshalling/UnMarshalling
 * @tc.desc      : 1.Marshalling/UnMarshalling
 */
HWTEST_F(WantSenderInfoTest, WantSenderInfoTest_0200, TestSize.Level1)
{
    WantSenderInfo info;

    for (int i = 0; i < 999; i++) {
        Want want;
        WantsInfo wantInfo;
        wantInfo.want = want;
        info.allWants.emplace_back(wantInfo);
    }
    EXPECT_EQ(info.allWants.size(), 999);
    Parcel parcel;
    info.Marshalling(parcel);
    // succeed as the iteration time is under the limit
    auto unInfo = WantSenderInfo::Unmarshalling(parcel);
    EXPECT_NE(unInfo, nullptr);
}

/*
 * @tc.number    : WantSenderInfoTest_0300
 * @tc.name      : Marshalling/UnMarshalling
 * @tc.desc      : 1.Marshalling/UnMarshalling
 */
HWTEST_F(WantSenderInfoTest, WantSenderInfoTest_0300, TestSize.Level1)
{
    WantSenderInfo info;

    for (int i = 0; i < 1000; i++) {
        Want want;
        WantsInfo wantInfo;
        wantInfo.want = want;
        info.allWants.emplace_back(wantInfo);
    }
    EXPECT_EQ(info.allWants.size(), 1000);
    Parcel parcel;
    info.Marshalling(parcel);
    // succeed as the iteration time equals the limit.
    auto unInfo = WantSenderInfo::Unmarshalling(parcel);
    EXPECT_NE(unInfo, nullptr);
}

/*
 * @tc.number    : WantSenderInfoTest_0400
 * @tc.name      : Marshalling/UnMarshalling
 * @tc.desc      : 1.Marshalling/UnMarshalling
 */
HWTEST_F(WantSenderInfoTest, WantSenderInfoTest_0400, TestSize.Level1)
{
    WantSenderInfo info;

    for (int i = 0; i < 1001; i++) {
        Want want;
        WantsInfo wantInfo;
        wantInfo.want = want;
        info.allWants.emplace_back(wantInfo);
    }
    EXPECT_EQ(info.allWants.size(), 1001);
    Parcel parcel;
    info.Marshalling(parcel);
    // fail as the iteration time exceeds the limit.
    auto unInfo = WantSenderInfo::Unmarshalling(parcel);
    EXPECT_EQ(unInfo, nullptr);
}
}  // namespace AAFwk
}  // namespace OHOS
