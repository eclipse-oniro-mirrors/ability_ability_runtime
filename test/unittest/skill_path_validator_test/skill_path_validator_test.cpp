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
#include <string>

#include "skill/skill_path_validator.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {

namespace {
std::string StringWithNull()
{
    return std::string("a\0b", 3);
}
} // namespace

class SkillPathValidatorTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void SkillPathValidatorTest::SetUpTestCase(void) {}
void SkillPathValidatorTest::TearDownTestCase(void) {}
void SkillPathValidatorTest::SetUp(void) {}
void SkillPathValidatorTest::TearDown(void) {}

// ---------------- IsSafeSkillPath: valid inputs ----------------

HWTEST_F(SkillPathValidatorTest, IsSafeSkillPath_ValidSimple_0100, TestSize.Level0)
{
    EXPECT_TRUE(IsSafeSkillPath("entry"));
    EXPECT_TRUE(IsSafeSkillPath("feature"));
}

HWTEST_F(SkillPathValidatorTest, IsSafeSkillPath_ValidWithUnderscoreDash_0100, TestSize.Level0)
{
    EXPECT_TRUE(IsSafeSkillPath("my_module-1"));
}

HWTEST_F(SkillPathValidatorTest, IsSafeSkillPath_ValidNestedPath_0100, TestSize.Level0)
{
    EXPECT_TRUE(IsSafeSkillPath("ets/pages/Index.ets"));
    EXPECT_TRUE(IsSafeSkillPath("ets/feature/Foo.ts"));
}

// ---------------- IsSafeSkillPath: invalid inputs ----------------

HWTEST_F(SkillPathValidatorTest, IsSafeSkillPath_Empty_0100, TestSize.Level0)
{
    EXPECT_FALSE(IsSafeSkillPath(""));
}

HWTEST_F(SkillPathValidatorTest, IsSafeSkillPath_AbsolutePath_0100, TestSize.Level0)
{
    EXPECT_FALSE(IsSafeSkillPath("/etc/passwd"));
    EXPECT_FALSE(IsSafeSkillPath("/data/app/foo.hap"));
}

HWTEST_F(SkillPathValidatorTest, IsSafeSkillPath_FdLoading_0100, TestSize.Level0)
{
    EXPECT_FALSE(IsSafeSkillPath("/proc/self/fd/3"));
    EXPECT_FALSE(IsSafeSkillPath("/dev/fd/3"));
}

HWTEST_F(SkillPathValidatorTest, IsSafeSkillPath_Traversal_0100, TestSize.Level0)
{
    EXPECT_FALSE(IsSafeSkillPath("../secret"));
    EXPECT_FALSE(IsSafeSkillPath("a/../../etc"));
    EXPECT_FALSE(IsSafeSkillPath("foo/..bar"));
}

/**
 * @tc.number: IsSafeSkillPath_Traversal_0200
 * @tc.name: IsSafeSkillPath
 * @tc.desc: Multi-level traversal prefixes (../../, ../../../) are rejected.
 */
HWTEST_F(SkillPathValidatorTest, IsSafeSkillPath_Traversal_0200, TestSize.Level0)
{
    EXPECT_FALSE(IsSafeSkillPath("../../etc/passwd"));
    EXPECT_FALSE(IsSafeSkillPath("../../../etc/passwd"));
    EXPECT_FALSE(IsSafeSkillPath("../../../../data/secret"));
    EXPECT_FALSE(IsSafeSkillPath("./../../etc"));
    EXPECT_FALSE(IsSafeSkillPath("module/../../etc"));
    EXPECT_FALSE(IsSafeSkillPath("a/b/../../../c"));
    EXPECT_FALSE(IsSafeSkillPath("ets/../../shared/secret"));
}

/**
 * @tc.number: IsSafeSkillPath_Traversal_0300
 * @tc.name: IsSafeSkillPath
 * @tc.desc: Trailing ".." and ".." inside the path are rejected.
 */
HWTEST_F(SkillPathValidatorTest, IsSafeSkillPath_Traversal_0300, TestSize.Level0)
{
    EXPECT_FALSE(IsSafeSkillPath("foo/.."));
    EXPECT_FALSE(IsSafeSkillPath("a/b/.."));
    EXPECT_FALSE(IsSafeSkillPath(".."));
    EXPECT_FALSE(IsSafeSkillPath("../"));
}

HWTEST_F(SkillPathValidatorTest, IsSafeSkillPath_NullByte_0100, TestSize.Level0)
{
    EXPECT_FALSE(IsSafeSkillPath(StringWithNull()));
}

HWTEST_F(SkillPathValidatorTest, IsSafeSkillPath_SpecialChars_0100, TestSize.Level0)
{
    EXPECT_FALSE(IsSafeSkillPath("a|b"));
    EXPECT_FALSE(IsSafeSkillPath("a;b"));
    EXPECT_FALSE(IsSafeSkillPath("a b"));
    EXPECT_FALSE(IsSafeSkillPath("a\\b"));
}

// ---------------- IsSafeHapPath: valid inputs ----------------

HWTEST_F(SkillPathValidatorTest, IsSafeHapPath_ValidAbsolute_0100, TestSize.Level0)
{
    EXPECT_TRUE(IsSafeHapPath("/data/app/el1/bundle/public/com.example/foo.hap"));
    EXPECT_TRUE(IsSafeHapPath("/system/app/foo.hap"));
}

// ---------------- IsSafeHapPath: invalid inputs ----------------

HWTEST_F(SkillPathValidatorTest, IsSafeHapPath_Empty_0100, TestSize.Level0)
{
    EXPECT_FALSE(IsSafeHapPath(""));
}

HWTEST_F(SkillPathValidatorTest, IsSafeHapPath_Relative_0100, TestSize.Level0)
{
    EXPECT_FALSE(IsSafeHapPath("data/app/foo.hap"));
    EXPECT_FALSE(IsSafeHapPath("foo.hap"));
}

HWTEST_F(SkillPathValidatorTest, IsSafeHapPath_ProcFs_0100, TestSize.Level0)
{
    EXPECT_FALSE(IsSafeHapPath("/proc/self/fd/3"));
    EXPECT_FALSE(IsSafeHapPath("/proc/version"));
}

HWTEST_F(SkillPathValidatorTest, IsSafeHapPath_DevFs_0100, TestSize.Level0)
{
    EXPECT_FALSE(IsSafeHapPath("/dev/fd/3"));
    EXPECT_FALSE(IsSafeHapPath("/dev/null"));
}

HWTEST_F(SkillPathValidatorTest, IsSafeHapPath_Traversal_0100, TestSize.Level0)
{
    EXPECT_FALSE(IsSafeHapPath("/data/app/../../etc/passwd"));
    EXPECT_FALSE(IsSafeHapPath("/../secret"));
    EXPECT_FALSE(IsSafeHapPath("/data/app/../../../etc/passwd"));
    EXPECT_FALSE(IsSafeHapPath("/data/../.././etc"));
    EXPECT_FALSE(IsSafeHapPath("/../.."));
    EXPECT_FALSE(IsSafeHapPath("/data/app/foo/.."));
    EXPECT_FALSE(IsSafeHapPath("/data/app/../etc"));
}

HWTEST_F(SkillPathValidatorTest, IsSafeHapPath_NullByte_0100, TestSize.Level0)
{
    std::string bad = std::string("/data/\0evil", 10);
    EXPECT_FALSE(IsSafeHapPath(bad));
}

HWTEST_F(SkillPathValidatorTest, IsSafeHapPath_SpecialChars_0100, TestSize.Level0)
{
    EXPECT_FALSE(IsSafeHapPath("/data/app/foo|.hap"));
    EXPECT_FALSE(IsSafeHapPath("/data/app/foo;hap"));
}

}  // namespace AbilityRuntime
}  // namespace OHOS
