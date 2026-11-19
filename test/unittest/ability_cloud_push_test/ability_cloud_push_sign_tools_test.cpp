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

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <string>

#include "param_update/ability_cloud_push_sign_tools.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class AbilityCloudPushSignToolTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.number: AbilityCloudPushSignToolTest_CalcBase64_0100
 * @tc.desc: Test CalcBase64 encodes known input
 * @tc.type: FUNC
 */
HWTEST_F(AbilityCloudPushSignToolTest, AbilityCloudPushSignToolTest_CalcBase64_0100, TestSize.Level2)
{
    unsigned char input[] = { 'a', 'b', 'c' };
    std::string out;
    AbilityCloudPushSignTool::CalcBase64(input, sizeof(input), out);
    EXPECT_EQ(out, "YWJj");
}

/**
 * @tc.number: AbilityCloudPushSignToolTest_CalcBase64_0200
 * @tc.desc: Test CalcBase64 handles empty input
 * @tc.type: FUNC
 */
HWTEST_F(AbilityCloudPushSignToolTest, AbilityCloudPushSignToolTest_CalcBase64_0200, TestSize.Level2)
{
    unsigned char input[] = { 0 };
    std::string out = "sentinel";
    AbilityCloudPushSignTool::CalcBase64(input, 0, out);
    EXPECT_TRUE(out.empty());
}

/**
 * @tc.number: AbilityCloudPushSignToolTest_ForEachFileSegment_0100
 * @tc.desc: Test ForEachFileSegment reads whole file content
 * @tc.type: FUNC
 */
HWTEST_F(AbilityCloudPushSignToolTest, AbilityCloudPushSignToolTest_ForEachFileSegment_0100, TestSize.Level2)
{
    const std::string path = "/data/local/tmp/cp_sign_segment_test.txt";
    std::ofstream ofs(path, std::ios::binary);
    ASSERT_TRUE(ofs.is_open());
    ofs << "hello world";
    ofs.close();
    std::string collected;
    int err = AbilityCloudPushSignTool::ForEachFileSegment(path,
        [&collected](char *buf, size_t len) { collected.append(buf, len); });
    EXPECT_EQ(err, 0);
    EXPECT_EQ(collected, "hello world");
    std::remove(path.c_str());
}

/**
 * @tc.number: AbilityCloudPushSignToolTest_CalcFileSha256Digest_0100
 * @tc.desc: Test CalcFileSha256Digest computes known digest
 * @tc.type: FUNC
 */
HWTEST_F(AbilityCloudPushSignToolTest, AbilityCloudPushSignToolTest_CalcFileSha256Digest_0100, TestSize.Level2)
{
    const std::string path = "/data/local/tmp/cp_sign_sha_test.txt";
    std::ofstream ofs(path, std::ios::binary);
    ASSERT_TRUE(ofs.is_open());
    ofs << "abc";
    ofs.close();
    std::tuple<int, std::string> ret = AbilityCloudPushSignTool::CalcFileSha256Digest(path);
    EXPECT_EQ(std::get<0>(ret), 0);
    EXPECT_EQ(std::get<1>(ret), "ungWv48Bz+pBQUDeXa4iI7ADYaOWF3qctBD/YfIAFa0=");
    std::remove(path.c_str());
}

/**
 * @tc.number: AbilityCloudPushSignToolTest_CalcFileSha256Digest_0200
 * @tc.desc: Test CalcFileSha256Digest fails on non-existent file
 * @tc.type: FUNC
 */
HWTEST_F(AbilityCloudPushSignToolTest, AbilityCloudPushSignToolTest_CalcFileSha256Digest_0200, TestSize.Level2)
{
    std::tuple<int, std::string> ret =
        AbilityCloudPushSignTool::CalcFileSha256Digest("/data/local/tmp/cp_sign_not_exist.txt");
    EXPECT_NE(std::get<0>(ret), 0);
    EXPECT_TRUE(std::get<1>(ret).empty());
}
}  // namespace AAFwk
}  // namespace OHOS
