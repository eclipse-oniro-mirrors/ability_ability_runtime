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
#include <iterator>
#include <string>

#define private public
#include "param_update/ability_cloud_push_manager.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class AbilityCloudPushManagerTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.number: AbilityCloudPushManagerTest_GetInstance_0100
 * @tc.desc: Test GetInstance returns the same singleton
 * @tc.type: FUNC
 */
HWTEST_F(AbilityCloudPushManagerTest, AbilityCloudPushManagerTest_GetInstance_0100, TestSize.Level2)
{
    auto &a = AbilityCloudPushManager::GetInstance();
    auto &b = AbilityCloudPushManager::GetInstance();
    EXPECT_EQ(&a, &b);
}

/**
 * @tc.number: AbilityCloudPushManagerTest_CopyToTmp_0100
 * @tc.desc: Test CopyToTmp copies file content to tmp path
 * @tc.type: FUNC
 */
HWTEST_F(AbilityCloudPushManagerTest, AbilityCloudPushManagerTest_CopyToTmp_0100, TestSize.Level2)
{
    const std::string src = "/data/local/tmp/cp_mgr_copytotmp_src.txt";
    const std::string tmp = "/data/local/tmp/cp_mgr_copytotmp_tmp.txt";
    std::ofstream ofs(src, std::ios::binary);
    ASSERT_TRUE(ofs.is_open());
    ofs << "content to copy";
    ofs.close();
    auto &mgr = AbilityCloudPushManager::GetInstance();
    EXPECT_TRUE(mgr.CopyToTmp(src, tmp));
    std::ifstream ifs(tmp, std::ios::binary);
    ASSERT_TRUE(ifs.is_open());
    std::string got((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    EXPECT_EQ(got, "content to copy");
    std::remove(src.c_str());
    std::remove(tmp.c_str());
}

/**
 * @tc.number: AbilityCloudPushManagerTest_CopyToTmp_0200
 * @tc.desc: Test CopyToTmp fails when src does not exist
 * @tc.type: FUNC
 */
HWTEST_F(AbilityCloudPushManagerTest, AbilityCloudPushManagerTest_CopyToTmp_0200, TestSize.Level2)
{
    auto &mgr = AbilityCloudPushManager::GetInstance();
    EXPECT_FALSE(mgr.CopyToTmp("/data/local/tmp/cp_mgr_not_exist_src.txt",
        "/data/local/tmp/cp_mgr_not_exist_tmp.txt"));
}

/**
 * @tc.number: AbilityCloudPushManagerTest_LoadVersion_0100
 * @tc.desc: Test LoadVersion returns non-empty when no local param present
 * @tc.type: FUNC
 */
HWTEST_F(AbilityCloudPushManagerTest, AbilityCloudPushManagerTest_LoadVersion_0100, TestSize.Level2)
{
    auto &mgr = AbilityCloudPushManager::GetInstance();
    std::string ver = mgr.LoadVersion();
    EXPECT_FALSE(ver.empty());
}
}  // namespace AAFwk
}  // namespace OHOS
