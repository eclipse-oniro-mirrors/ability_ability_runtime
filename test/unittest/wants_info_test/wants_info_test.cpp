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

#include <cstdlib>
#include <gtest/gtest.h>

#include "parcel.h"
#define private public
#define protected public
#include "wants_info.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using OHOS::AppExecFwk::ElementName;

namespace OHOS {
namespace AAFwk {
namespace {
class LimitedAllocator final : public Allocator {
public:
    explicit LimitedAllocator(size_t maxAllocationSize) : maxAllocationSize_(maxAllocationSize) {}

    ~LimitedAllocator() override = default;

    void *Realloc(void *data, size_t newSize) override
    {
        if (newSize > maxAllocationSize_) {
            return nullptr;
        }
        return std::realloc(data, newSize);
    }

    void *Alloc(size_t size) override
    {
        if (size > maxAllocationSize_) {
            return nullptr;
        }
        return std::malloc(size);
    }

    void Dealloc(void *data) override
    {
        std::free(data);
    }

private:
    size_t maxAllocationSize_;
};
}  // namespace
class WantsInfoTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

public:
};

void WantsInfoTest::SetUpTestCase()
{}

void WantsInfoTest::TearDownTestCase()
{}

void WantsInfoTest::SetUp()
{}

void WantsInfoTest::TearDown()
{}

/*
 * @tc.number    : WantsInfoTest_0100
 * @tc.name      : Marshalling/UnMarshalling
 * @tc.desc      : 1.Marshalling/UnMarshalling
 */
HWTEST_F(WantsInfoTest, WantsInfoTest_0100, TestSize.Level1)
{
    WantsInfo info;
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    info.want = want;
    info.resolvedTypes = "nihao";
    Parcel parcel;
    ASSERT_TRUE(info.Marshalling(parcel));
    auto unInfo = WantsInfo::Unmarshalling(parcel);
    ASSERT_NE(unInfo, nullptr);
    EXPECT_EQ(unInfo->want.GetElement().GetBundleName(), "com.ix.hiMusic");
    EXPECT_EQ(unInfo->want.GetElement().GetAbilityName(), "MusicSAbility");
    EXPECT_EQ(unInfo->resolvedTypes, "nihao");
    delete unInfo;
}

/*
 * @tc.number    : WantsInfoTest_0200
 * @tc.name      : Marshalling want failure
 * @tc.desc      : Marshalling returns false when the Want cannot be written.
 */
HWTEST_F(WantsInfoTest, WantsInfoTest_0200, TestSize.Level1)
{
    WantsInfo info;
    Parcel parcel(new LimitedAllocator(0));

    EXPECT_FALSE(info.Marshalling(parcel));
}

/*
 * @tc.number    : WantsInfoTest_0300
 * @tc.name      : Marshalling resolvedTypes failure
 * @tc.desc      : Marshalling returns false when resolvedTypes cannot be written after the Want.
 */
HWTEST_F(WantsInfoTest, WantsInfoTest_0300, TestSize.Level1)
{
    WantsInfo info;
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    info.want = want;

    Parcel wantParcel;
    ASSERT_TRUE(wantParcel.WriteParcelable(&info.want));
    const size_t wantParcelCapacity = wantParcel.GetDataCapacity();
    ASSERT_GT(wantParcelCapacity, 0);
    info.resolvedTypes.assign(wantParcelCapacity, 'a');
    Parcel parcel(new LimitedAllocator(wantParcelCapacity));

    EXPECT_FALSE(info.Marshalling(parcel));
}
}  // namespace AAFwk
}  // namespace OHOS
