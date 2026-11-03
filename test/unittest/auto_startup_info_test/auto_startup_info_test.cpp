/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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
#include "auto_startup_info.h"
#undef private
#undef protected

#include "string_ex.h"
#include "types.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AbilityRuntime {
class AutoStartupInfoTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AutoStartupInfoTest::SetUpTestCase() {}

void AutoStartupInfoTest::TearDownTestCase() {}

void AutoStartupInfoTest::SetUp() {}

void AutoStartupInfoTest::TearDown() {}

/**
 * Feature: AutoStartupInfo
 * Function: ReadFromParcel
 * SubFunction: NA
 * FunctionPoints: AutoStartupInfo ReadFromParcel
 */
HWTEST_F(AutoStartupInfoTest, ReadFromParcel_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadFromParcel_100 start";
    Parcel parcel;
    auto autoStartupInfo = new (std::nothrow) AutoStartupInfo();
    auto result = autoStartupInfo->ReadFromParcel(parcel);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "ReadFromParcel_100 end";
}

/**
 * Feature: AutoStartupInfo
 * Function: Unmarshalling
 * SubFunction: NA
 * FunctionPoints: AutoStartupInfo Unmarshalling
 */
HWTEST_F(AutoStartupInfoTest, Unmarshalling_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Unmarshalling_100 start";
    Parcel parcel;
    auto autoStartupInfo = new (std::nothrow) AutoStartupInfo();
    auto result = autoStartupInfo->Unmarshalling(parcel);
    EXPECT_NE(result, nullptr);
    GTEST_LOG_(INFO) << "Unmarshalling_100 end";
}

/**
 * Feature: AutoStartupInfo
 * Function: Marshalling
 * SubFunction: NA
 * FunctionPoints: AutoStartupInfo Marshalling
 */
HWTEST_F(AutoStartupInfoTest, Marshalling_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Marshalling_100 start";
    auto autoStartupInfo = new (std::nothrow) AutoStartupInfo();
    autoStartupInfo->bundleName = "com.example.testbundle";
    autoStartupInfo->abilityName = "test.app.Ability";
    autoStartupInfo->moduleName = "test.app.Moudle";
    autoStartupInfo->abilityTypeName = "test.app.mainAbility";
    autoStartupInfo->appCloneIndex = 0;
    Parcel parcel;
    bool result = autoStartupInfo->Marshalling(parcel);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "Marshalling_100 end";
}

/**
 * Feature: AutoStartupInfo
 * Function: Marshalling
 * FunctionPoints: Mixed startup modes do not consume the next parcelable or following argument.
 */
HWTEST_F(AutoStartupInfoTest, BootAppParcelBoundary, TestSize.Level1)
{
    AutoStartupInfo boot;
    boot.bundleName = "com.example.boot";
    boot.isDirectBoot = true;
    boot.isHiddenStart = true;
    AutoStartupInfo ordinary;
    ordinary.bundleName = "com.example.ordinary";
    Parcel parcel;
    ASSERT_TRUE(boot.Marshalling(parcel));
    ASSERT_TRUE(ordinary.Marshalling(parcel));
    ASSERT_TRUE(parcel.WriteInt32(101));
    AutoStartupInfo first;
    AutoStartupInfo second;
    ASSERT_TRUE(first.ReadFromParcel(parcel));
    ASSERT_TRUE(second.ReadFromParcel(parcel));
    EXPECT_TRUE(first.isDirectBoot);
    EXPECT_TRUE(first.isHiddenStart);
    EXPECT_FALSE(second.isDirectBoot);
    EXPECT_EQ(second.bundleName, ordinary.bundleName);
    EXPECT_EQ(parcel.ReadInt32(), 101);
}
} // namespace AbilityRuntime
} // namespace OHOS
