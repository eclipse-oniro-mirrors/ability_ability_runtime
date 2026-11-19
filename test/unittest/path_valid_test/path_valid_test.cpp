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

#include "path_utils.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {

namespace {
constexpr size_t STRING_WITH_NULL_LENGTH = 3;
constexpr size_t ABSOLUTE_PATH_WITH_NULL_LENGTH = 11;

std::string StringWithNull()
{
    return std::string("a\0b", STRING_WITH_NULL_LENGTH);
}

std::string AbsolutePathWithNull()
{
    return std::string("/data/\0evil", ABSOLUTE_PATH_WITH_NULL_LENGTH);
}
} // namespace

class PathValidTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void PathValidTest::SetUpTestCase(void) {}
void PathValidTest::TearDownTestCase(void) {}
void PathValidTest::SetUp(void) {}
void PathValidTest::TearDown(void) {}

// ---------------- valid ----------------

HWTEST_F(PathValidTest, Valid_Relative_0100, TestSize.Level0)
{
    EXPECT_TRUE(IsPathValid("entry"));
    EXPECT_TRUE(IsPathValid("my_module-1"));
    EXPECT_TRUE(IsPathValid("ets/pages/Index.ets"));
}

HWTEST_F(PathValidTest, Valid_Absolute_0100, TestSize.Level0)
{
    EXPECT_TRUE(IsPathValid("/data/app/el1/bundle/public/com.example/foo.hap"));
    EXPECT_TRUE(IsPathValid("/system/app/foo.hap"));
}

// Direction is not part of validity: the same path with or without a leading
// slash is accepted, consumers fail safely on both.
HWTEST_F(PathValidTest, Valid_DirectionAgnostic_0100, TestSize.Level0)
{
    EXPECT_TRUE(IsPathValid("/data/app/foo.hap"));
    EXPECT_TRUE(IsPathValid("data/app/foo.hap"));
}

// ---------------- invalid ----------------

HWTEST_F(PathValidTest, Invalid_Empty_0100, TestSize.Level0)
{
    EXPECT_FALSE(IsPathValid(""));
}

HWTEST_F(PathValidTest, Invalid_Traversal_0100, TestSize.Level0)
{
    EXPECT_FALSE(IsPathValid(".."));
    EXPECT_FALSE(IsPathValid("../"));
    EXPECT_FALSE(IsPathValid("../secret"));
    EXPECT_FALSE(IsPathValid("foo/.."));
    EXPECT_FALSE(IsPathValid("a/b/.."));
    EXPECT_FALSE(IsPathValid("foo/..bar"));
    EXPECT_FALSE(IsPathValid("a/../../etc"));
    EXPECT_FALSE(IsPathValid("../../../etc/passwd"));
    EXPECT_FALSE(IsPathValid("module/../../etc"));
}

HWTEST_F(PathValidTest, Invalid_TraversalAbsolute_0100, TestSize.Level0)
{
    EXPECT_FALSE(IsPathValid("/../secret"));
    EXPECT_FALSE(IsPathValid("/../.."));
    EXPECT_FALSE(IsPathValid("/data/app/../../etc/passwd"));
    EXPECT_FALSE(IsPathValid("/data/../.././etc"));
    EXPECT_FALSE(IsPathValid("/data/app/foo/.."));
    EXPECT_FALSE(IsPathValid("/data/app/../etc"));
}

HWTEST_F(PathValidTest, Invalid_NullByte_0100, TestSize.Level0)
{
    EXPECT_FALSE(IsPathValid(StringWithNull()));
    EXPECT_FALSE(IsPathValid(AbsolutePathWithNull()));
}

HWTEST_F(PathValidTest, Invalid_Backslash_0100, TestSize.Level0)
{
    EXPECT_FALSE(IsPathValid("a\\b"));
    EXPECT_FALSE(IsPathValid("/data/a\\b.hap"));
}

HWTEST_F(PathValidTest, Invalid_ControlChar_0100, TestSize.Level0)
{
    EXPECT_FALSE(IsPathValid("a\tb"));
    EXPECT_FALSE(IsPathValid("a\nb"));
    EXPECT_FALSE(IsPathValid("/data/app/foo\t.hap"));
}

// Relaxed on purpose (modeled after BundleUtil::IsFileNameValid): characters
// outside the old whitelist and ".." inside a file name are accepted.
HWTEST_F(PathValidTest, Relaxed_Characters_0100, TestSize.Level0)
{
    EXPECT_TRUE(IsPathValid("a..b"));
    EXPECT_TRUE(IsPathValid("a b"));
    EXPECT_TRUE(IsPathValid("a|b"));
    EXPECT_TRUE(IsPathValid("a;b"));
    EXPECT_TRUE(IsPathValid("/data/app/foo|.hap"));
    EXPECT_TRUE(IsPathValid("/data/app/foo hap.hap"));
}

// Relaxed on purpose: procfs/devfs targets are no longer rejected, the
// traversal check still holds for them.
HWTEST_F(PathValidTest, Relaxed_ProcDev_0100, TestSize.Level0)
{
    EXPECT_TRUE(IsPathValid("/proc/self/fd/3"));
    EXPECT_TRUE(IsPathValid("/dev/fd/3"));
}

}  // namespace AbilityRuntime
}  // namespace OHOS
