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

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include "param_update/ability_cloud_push_reader.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class AbilityCloudPushReaderTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.number: AbilityCloudPushReaderTest_VersionStrToNumber_0100
 * @tc.desc: Test VersionStrToNumber parses valid 4-segment version
 * @tc.type: FUNC
 */
HWTEST_F(AbilityCloudPushReaderTest, AbilityCloudPushReaderTest_VersionStrToNumber_0100, TestSize.Level2)
{
    AbilityCloudPushReader reader;
    std::vector<std::string> num;
    EXPECT_TRUE(reader.VersionStrToNumber("10.10.26.101", num));
    EXPECT_EQ(num.size(), 4u);
    EXPECT_EQ(num[0], "10");
    EXPECT_EQ(num[3], "101");
}

/**
 * @tc.number: AbilityCloudPushReaderTest_VersionStrToNumber_0200
 * @tc.desc: Test VersionStrToNumber rejects invalid version
 * @tc.type: FUNC
 */
HWTEST_F(AbilityCloudPushReaderTest, AbilityCloudPushReaderTest_VersionStrToNumber_0200, TestSize.Level2)
{
    AbilityCloudPushReader reader;
    std::vector<std::string> num;
    EXPECT_FALSE(reader.VersionStrToNumber("1.2.3", num));
    EXPECT_FALSE(reader.VersionStrToNumber("", num));
    EXPECT_FALSE(reader.VersionStrToNumber("abc.def.ghi.jkl.mno", num));
}

/**
 * @tc.number: AbilityCloudPushReaderTest_CompareVersion_0100
 * @tc.desc: Test CompareVersion returns true when cloud newer than local
 * @tc.type: FUNC
 */
HWTEST_F(AbilityCloudPushReaderTest, AbilityCloudPushReaderTest_CompareVersion_0100, TestSize.Level2)
{
    AbilityCloudPushReader reader;
    std::vector<std::string> local = { "1", "0", "0", "0" };
    std::vector<std::string> cloud = { "10", "10", "26", "101" };
    EXPECT_TRUE(reader.CompareVersion(local, cloud));
}

/**
 * @tc.number: AbilityCloudPushReaderTest_CompareVersion_0200
 * @tc.desc: Test CompareVersion returns false for equal versions
 * @tc.type: FUNC
 */
HWTEST_F(AbilityCloudPushReaderTest, AbilityCloudPushReaderTest_CompareVersion_0200, TestSize.Level2)
{
    AbilityCloudPushReader reader;
    std::vector<std::string> v = { "10", "10", "26", "101" };
    EXPECT_FALSE(reader.CompareVersion(v, v));
}

/**
 * @tc.number: AbilityCloudPushReaderTest_CompareVersion_0300
 * @tc.desc: Test CompareVersion returns false when cloud older than local
 * @tc.type: FUNC
 */
HWTEST_F(AbilityCloudPushReaderTest, AbilityCloudPushReaderTest_CompareVersion_0300, TestSize.Level2)
{
    AbilityCloudPushReader reader;
    std::vector<std::string> local = { "10", "10", "26", "101" };
    std::vector<std::string> cloud = { "1", "0", "0", "0" };
    EXPECT_FALSE(reader.CompareVersion(local, cloud));
}

/**
 * @tc.number: AbilityCloudPushReaderTest_GetVersionInfoStr_0100
 * @tc.desc: Test GetVersionInfoStr returns default for non-existent file
 * @tc.type: FUNC
 */
HWTEST_F(AbilityCloudPushReaderTest, AbilityCloudPushReaderTest_GetVersionInfoStr_0100, TestSize.Level2)
{
    AbilityCloudPushReader reader;
    EXPECT_EQ(reader.GetVersionInfoStr("/data/local/tmp/cp_reader_not_exist.txt"),
        AbilityCloudPushPaths::DEFAULT_VERSION);
}

/**
 * @tc.number: AbilityCloudPushReaderTest_GetVersionInfoStr_0200
 * @tc.desc: Test GetVersionInfoStr parses version from file
 * @tc.type: FUNC
 */
HWTEST_F(AbilityCloudPushReaderTest, AbilityCloudPushReaderTest_GetVersionInfoStr_0200, TestSize.Level2)
{
    const std::string path = "/data/local/tmp/cp_reader_version.txt";
    std::ofstream ofs(path, std::ios::binary);
    ASSERT_TRUE(ofs.is_open());
    ofs << "version=10.10.26.101";
    ofs.close();
    AbilityCloudPushReader reader;
    EXPECT_EQ(reader.GetVersionInfoStr(path), "10.10.26.101");
    std::remove(path.c_str());
}
}  // namespace AAFwk
}  // namespace OHOS
