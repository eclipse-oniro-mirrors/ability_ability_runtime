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
#include "sub_spawn_record.h"

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
 * @tc.desc: Test OnChildProcessCreated with unknown uid does not crash
 */
HWTEST_F(SubSpawnManagerTest, OnChildProcessCreated_001, TestSize.Level1)
{
    SubSpawnManager::GetInstance().OnChildProcessCreated(99999, 88888);
    SUCCEED();
}

/**
 * @tc.name: OnChildProcessExited_001
 * @tc.desc: Test OnChildProcessExited with unknown uid does not crash
 */
HWTEST_F(SubSpawnManagerTest, OnChildProcessExited_001, TestSize.Level1)
{
    SubSpawnManager::GetInstance().OnChildProcessExited(99999, 88888);
    SUCCEED();
}

/**
 * @tc.name: OnAppProcessDied_001
 * @tc.desc: Test OnAppProcessDied with unknown uid does not crash
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
    pid_t result = SubSpawnManager::GetInstance().StartChildViaSubSpawn(10000, startMsg);
    EXPECT_LE(result, 0);
}

/**
 * @tc.name: HandleIdleTimeout_001
 * @tc.desc: Test HandleIdleTimeout with unknown uid does not crash
 */
HWTEST_F(SubSpawnManagerTest, HandleIdleTimeout_001, TestSize.Level1)
{
    SubSpawnManager::GetInstance().HandleIdleTimeout(99999);
    SUCCEED();
}

/**
 * @tc.name: SubSpawnRecord_Create_001
 * @tc.desc: Test SubSpawnRecord creation with valid params
 */
HWTEST_F(SubSpawnManagerTest, SubSpawnRecord_Create_001, TestSize.Level1)
{
    AppSpawnStartMsg startMsg;
    auto record = SubSpawnRecord::CreateSubSpawnRecord(20000, "com.test.app", startMsg);
    EXPECT_NE(record, nullptr);
    EXPECT_EQ(record->GetUid(), 20000);
    EXPECT_EQ(record->GetBundleName(), "com.test.app");
}

/**
 * @tc.name: SubSpawnRecord_Create_002
 * @tc.desc: Test SubSpawnRecord creation with invalid params returns nullptr
 */
HWTEST_F(SubSpawnManagerTest, SubSpawnRecord_Create_002, TestSize.Level1)
{
    AppSpawnStartMsg startMsg;
    EXPECT_EQ(SubSpawnRecord::CreateSubSpawnRecord(0, "com.test.app", startMsg), nullptr);
    EXPECT_EQ(SubSpawnRecord::CreateSubSpawnRecord(20000, "", startMsg), nullptr);
}

/**
 * @tc.name: SubSpawnRecord_State_001
 * @tc.desc: Test SubSpawnRecord state transitions
 */
HWTEST_F(SubSpawnManagerTest, SubSpawnRecord_State_001, TestSize.Level1)
{
    AppSpawnStartMsg startMsg;
    auto record = SubSpawnRecord::CreateSubSpawnRecord(20000, "com.test.app", startMsg);
    ASSERT_NE(record, nullptr);
    EXPECT_FALSE(record->IsReady());
    EXPECT_FALSE(record->IsCreating());
    EXPECT_FALSE(record->IsTerminating());
    record->SetState(SubSpawnState::CREATING);
    EXPECT_TRUE(record->IsCreating());
    EXPECT_FALSE(record->IsReady());
    record->SetState(SubSpawnState::READY);
    EXPECT_TRUE(record->IsReady());
    EXPECT_FALSE(record->IsCreating());
    record->SetState(SubSpawnState::TERMINATING);
    EXPECT_TRUE(record->IsTerminating());
    EXPECT_FALSE(record->IsReady());
}

/**
 * @tc.name: SubSpawnRecord_ChildPids_001
 * @tc.desc: Test SubSpawnRecord child pid tracking
 */
HWTEST_F(SubSpawnManagerTest, SubSpawnRecord_ChildPids_001, TestSize.Level1)
{
    AppSpawnStartMsg startMsg;
    auto record = SubSpawnRecord::CreateSubSpawnRecord(20000, "com.test.app", startMsg);
    ASSERT_NE(record, nullptr);
    EXPECT_EQ(record->GetChildProcessCount(), 0u);
    EXPECT_FALSE(record->HasChildProcesses());
    record->AddChildPid(3000);
    record->AddChildPid(3001);
    EXPECT_EQ(record->GetChildProcessCount(), 2u);
    EXPECT_TRUE(record->HasChildProcesses());
    record->RemoveChildPid(3000);
    EXPECT_EQ(record->GetChildProcessCount(), 1u);
    record->RemoveChildPid(3001);
    EXPECT_EQ(record->GetChildProcessCount(), 0u);
    EXPECT_FALSE(record->HasChildProcesses());
}

/**
 * @tc.name: SubSpawnRecord_Diagnostics_001
 * @tc.desc: Test SubSpawnRecord kill reason and exit signal
 */
HWTEST_F(SubSpawnManagerTest, SubSpawnRecord_Diagnostics_001, TestSize.Level1)
{
    AppSpawnStartMsg startMsg;
    auto record = SubSpawnRecord::CreateSubSpawnRecord(20000, "com.test.app", startMsg);
    ASSERT_NE(record, nullptr);
    EXPECT_EQ(record->GetKillReason(), "");
    EXPECT_EQ(record->GetExitSignal(), -1);
    record->SetKillReason("app_died");
    record->SetExitSignal(9);
    EXPECT_EQ(record->GetKillReason(), "app_died");
    EXPECT_EQ(record->GetExitSignal(), 9);
}

/**
 * @tc.name: SubSpawnRecord_CreatedNotified_001
 * @tc.desc: Test SubSpawnRecord created-notified flag defaults false and toggles
 */
HWTEST_F(SubSpawnManagerTest, SubSpawnRecord_CreatedNotified_001, TestSize.Level1)
{
    AppSpawnStartMsg startMsg;
    auto record = SubSpawnRecord::CreateSubSpawnRecord(20000, "com.test.app", startMsg);
    ASSERT_NE(record, nullptr);
    EXPECT_FALSE(record->IsCreatedNotified());
    record->SetCreatedNotified(true);
    EXPECT_TRUE(record->IsCreatedNotified());
    record->SetCreatedNotified(false);
    EXPECT_FALSE(record->IsCreatedNotified());
}
