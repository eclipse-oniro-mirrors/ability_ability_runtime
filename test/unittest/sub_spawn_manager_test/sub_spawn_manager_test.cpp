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
#include <vector>
#include <string>

#include "sub_spawn_manager.h"

using namespace testing;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

class SubSpawnManagerTest : public ::testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: ShouldUseSubSpawn_001
 * @tc.desc: Test ShouldUseSubSpawn returns false when preloadSoList is empty
 */
HWTEST_F(SubSpawnManagerTest, ShouldUseSubSpawn_001, TestSize.Level1)
{
    std::vector<std::string> emptyList;
    EXPECT_FALSE(SubSpawnManager::GetInstance().ShouldUseSubSpawn(emptyList, true));
}

/**
 * @tc.name: ShouldUseSubSpawn_002
 * @tc.desc: Test ShouldUseSubSpawn returns false when not isolated mode
 */
HWTEST_F(SubSpawnManagerTest, ShouldUseSubSpawn_002, TestSize.Level1)
{
    std::vector<std::string> soList = {"libentry.so"};
    EXPECT_FALSE(SubSpawnManager::GetInstance().ShouldUseSubSpawn(soList, false));
}

/**
 * @tc.name: ShouldUseSubSpawn_003
 * @tc.desc: Test ShouldUseSubSpawn returns false when both conditions not met
 */
HWTEST_F(SubSpawnManagerTest, ShouldUseSubSpawn_003, TestSize.Level1)
{
    std::vector<std::string> emptyList;
    EXPECT_FALSE(SubSpawnManager::GetInstance().ShouldUseSubSpawn(emptyList, false));
}

/**
 * @tc.name: IsSubSpawn_001
 * @tc.desc: Test IsSubSpawn returns false for unknown pid
 */
HWTEST_F(SubSpawnManagerTest, IsSubSpawn_001, TestSize.Level1)
{
    EXPECT_FALSE(SubSpawnManager::GetInstance().IsSubSpawn(99999));
}

/**
 * @tc.name: IsSubSpawn_002
 * @tc.desc: Test IsSubSpawn returns false for invalid pid
 */
HWTEST_F(SubSpawnManagerTest, IsSubSpawn_002, TestSize.Level1)
{
    EXPECT_FALSE(SubSpawnManager::GetInstance().IsSubSpawn(0));
    EXPECT_FALSE(SubSpawnManager::GetInstance().IsSubSpawn(-1));
}

/**
 * @tc.name: OnSubSpawnExited_001
 * @tc.desc: Test OnSubSpawnExited with unknown pid does not crash
 */
HWTEST_F(SubSpawnManagerTest, OnSubSpawnExited_001, TestSize.Level1)
{
    SubSpawnManager::GetInstance().OnSubSpawnExited(99999);
    SUCCEED();
}

/**
 * @tc.name: OnChildProcessCreated_001
 * @tc.desc: Test OnChildProcessCreated with unknown appPid does not crash
 */
HWTEST_F(SubSpawnManagerTest, OnChildProcessCreated_001, TestSize.Level1)
{
    SubSpawnManager::GetInstance().OnChildProcessCreated(99999);
    SUCCEED();
}

/**
 * @tc.name: OnChildProcessExited_001
 * @tc.desc: Test OnChildProcessExited with unknown appPid does not crash
 */
HWTEST_F(SubSpawnManagerTest, OnChildProcessExited_001, TestSize.Level1)
{
    SubSpawnManager::GetInstance().OnChildProcessExited(99999);
    SUCCEED();
}

/**
 * @tc.name: OnAppProcessDied_001
 * @tc.desc: Test OnAppProcessDied with unknown appPid does not crash
 */
HWTEST_F(SubSpawnManagerTest, OnAppProcessDied_001, TestSize.Level1)
{
    SubSpawnManager::GetInstance().OnAppProcessDied(99999);
    SUCCEED();
}

/**
 * @tc.name: StartChildViaSubSpawn_001
 * @tc.desc: Test StartChildViaSubSpawn returns <= 0 when no subspawn record exists
 */
HWTEST_F(SubSpawnManagerTest, StartChildViaSubSpawn_001, TestSize.Level1)
{
    AppSpawnStartMsg startMsg;
    startMsg.procName = "test";
    startMsg.uid = 10000;
    pid_t result = SubSpawnManager::GetInstance().StartChildViaSubSpawn(99999, 10000, startMsg);
    EXPECT_LE(result, 0);
}

/**
 * @tc.name: HandleIdleTimeout_001
 * @tc.desc: Test HandleIdleTimeout with unknown appPid does not crash
 */
HWTEST_F(SubSpawnManagerTest, HandleIdleTimeout_001, TestSize.Level1)
{
    SubSpawnManager::GetInstance().HandleIdleTimeout(99999);
    SUCCEED();
}
