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

#include "sem_ver.h"

using namespace testing::ext;

namespace OHOS {
namespace AgentRuntime {

class SemVerTest : public testing::Test {
};

/**
 * @tc.name: SemVerCompareTest_001
 * @tc.desc: Verify normal release versions are compared by major, minor and patch.
 * @tc.type: FUNC
 */
HWTEST_F(SemVerTest, SemVerCompareTest_001, TestSize.Level1)
{
    EXPECT_EQ(CompareSemVer("1.2.3", "1.2.4"), SemVerCompareResult::LESS);
    EXPECT_EQ(CompareSemVer("2.0.0", "1.9.9"), SemVerCompareResult::GREATER);
    EXPECT_EQ(CompareSemVer("1.2.0", "1.3.0"), SemVerCompareResult::LESS);
    EXPECT_EQ(CompareSemVer("1.10.0", "1.2.0"), SemVerCompareResult::GREATER);
    EXPECT_EQ(CompareSemVer("1.0.0", "1.0.0"), SemVerCompareResult::EQUAL);
}

/**
 * @tc.name: SemVerCompareTest_002
 * @tc.desc: Verify prerelease precedence follows SemVer ordering rules.
 * @tc.type: FUNC
 */
HWTEST_F(SemVerTest, SemVerCompareTest_002, TestSize.Level1)
{
    EXPECT_EQ(CompareSemVer("1.0.0-alpha", "1.0.0-alpha.1"), SemVerCompareResult::LESS);
    EXPECT_EQ(CompareSemVer("1.0.0-alpha.1", "1.0.0-alpha.beta"), SemVerCompareResult::LESS);
    EXPECT_EQ(CompareSemVer("1.0.0-alpha.2", "1.0.0-alpha.10"), SemVerCompareResult::LESS);
    EXPECT_EQ(CompareSemVer("1.0.0-alpha.1", "1.0.0-alpha"), SemVerCompareResult::GREATER);
    EXPECT_EQ(CompareSemVer("1.0.0-beta", "1.0.0-alpha.beta"), SemVerCompareResult::GREATER);
    EXPECT_EQ(CompareSemVer("1.0.0-rc.1", "1.0.0"), SemVerCompareResult::LESS);
}

/**
 * @tc.name: SemVerCompareTest_003
 * @tc.desc: Verify build metadata does not change precedence.
 * @tc.type: FUNC
 */
HWTEST_F(SemVerTest, SemVerCompareTest_003, TestSize.Level1)
{
    EXPECT_EQ(CompareSemVer("1.0.0+build.1", "1.0.0+build.2"), SemVerCompareResult::EQUAL);
    EXPECT_EQ(CompareSemVer("1.0.0-alpha+001", "1.0.0-alpha+exp.sha"), SemVerCompareResult::EQUAL);
}

/**
 * @tc.name: SemVerCompareTest_004
 * @tc.desc: Verify numeric prerelease identifiers have lower precedence than non-numeric identifiers.
 * @tc.type: FUNC
 */
HWTEST_F(SemVerTest, SemVerCompareTest_004, TestSize.Level1)
{
    EXPECT_EQ(CompareSemVer("1.0.0-1", "1.0.0-alpha"), SemVerCompareResult::LESS);
    EXPECT_EQ(CompareSemVer("1.0.0-alpha", "1.0.0-1"), SemVerCompareResult::GREATER);
    EXPECT_EQ(CompareSemVer("1.0.0", "1.0.0-rc.1"), SemVerCompareResult::GREATER);
}

/**
 * @tc.name: SemVerCompareTest_005
 * @tc.desc: Verify invalid SemVer input is rejected.
 * @tc.type: FUNC
 */
HWTEST_F(SemVerTest, SemVerCompareTest_005, TestSize.Level1)
{
    EXPECT_TRUE(IsValidSemVer("0.0.0"));
    EXPECT_TRUE(IsValidSemVer("1.2.3-alpha.1+build.5"));
    EXPECT_FALSE(IsValidSemVer(""));
    EXPECT_FALSE(IsValidSemVer("1.0"));
    EXPECT_FALSE(IsValidSemVer("01.0.0"));
    EXPECT_FALSE(IsValidSemVer("1.0.0-01"));
    EXPECT_FALSE(IsValidSemVer("1.0.0+"));
    EXPECT_FALSE(IsValidSemVer("1.0.0-alpha..1"));
    EXPECT_FALSE(IsValidSemVer("1.0.0-alpha_1"));
    EXPECT_FALSE(IsValidSemVer("1.0.0+build_1"));
    EXPECT_FALSE(IsValidSemVer("1.a.0"));
    EXPECT_EQ(CompareSemVer("1.0", "1.0.0"), SemVerCompareResult::INVALID);
    EXPECT_EQ(CompareSemVer("1.0.0-alpha_1", "1.0.0"), SemVerCompareResult::INVALID);
}

/**
 * @tc.name: SemVerCompareTest_006
 * @tc.desc: Verify equal prerelease versions and lexical prerelease ordering.
 * @tc.type: FUNC
 */
HWTEST_F(SemVerTest, SemVerCompareTest_006, TestSize.Level1)
{
    EXPECT_EQ(CompareSemVer("1.0.0-alpha.1", "1.0.0-alpha.1"), SemVerCompareResult::EQUAL);
    EXPECT_EQ(CompareSemVer("1.0.0-beta", "1.0.0-gamma"), SemVerCompareResult::LESS);
}

/**
 * @tc.name: SemVerCompareTest_007
 * @tc.desc: Verify empty version strings are rejected directly.
 * @tc.type: FUNC
 */
HWTEST_F(SemVerTest, SemVerCompareTest_007, TestSize.Level1)
{
    EXPECT_FALSE(IsValidSemVer(""));
    EXPECT_EQ(CompareSemVer("", "1.0.0"), SemVerCompareResult::INVALID);
    EXPECT_EQ(CompareSemVer("1.0.0", ""), SemVerCompareResult::INVALID);
}
} // namespace AgentRuntime
} // namespace OHOS
