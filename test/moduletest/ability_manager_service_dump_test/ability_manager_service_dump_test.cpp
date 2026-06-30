/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "ability_manager_service.h"
#include "mission_list_manager.h"
#undef private
#undef protected
#include "scene_board_judgement.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
namespace {
const int32_t USER_ID = 100;
const size_t SIZE_ZERO = 0;
const std::string STRING_PROCESS_NAME = "process_name";
}  // namespace

class AbilityManagerServiceDumpTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void AbilityManagerServiceDumpTest::SetUpTestCase()
{}

void AbilityManagerServiceDumpTest::TearDownTestCase()
{}

void AbilityManagerServiceDumpTest::SetUp()
{
}

void AbilityManagerServiceDumpTest::TearDown()
{
}

/**
 * @tc.name: AbilityManagerService_GetProcessRunningInfosByUserId_0100
 * @tc.desc: GetProcessRunningInfosByUserId
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AbilityManagerServiceDumpTest, AbilityManagerService_GetProcessRunningInfosByUserId_0100, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::vector<RunningProcessInfo> info;
    auto result = abilityMs_->GetProcessRunningInfosByUserId(info, USER_ID);
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);
}

/**
 * @tc.name: AbilityManagerService_DumpSysInner_0100
 * @tc.desc: DumpSysInner
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AbilityManagerServiceDumpTest, AbilityManagerService_DumpSysInner_0100, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::string args = "-a";
    std::vector<std::string> info;
    bool isClient = false;
    bool isUserID = true;
    abilityMs_->DumpSysInner(args, info, isClient, isUserID, USER_ID);
    EXPECT_GT(info.size(), SIZE_ZERO);
}

/**
 * @tc.name: AbilityManagerService_DumpSysMissionListInner_0100
 * @tc.desc: DumpSysMissionListInner
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AbilityManagerServiceDumpTest, AbilityManagerService_DumpSysMissionListInner_0100, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::string args = "-a";
    std::vector<std::string> info;
    bool isClient = false;
    bool isUserID = true;
    EXPECT_TRUE(abilityMs_ != nullptr);
    abilityMs_->DumpSysMissionListInner(args, info, isClient, isUserID, USER_ID);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_GT(info.size(), SIZE_ZERO);
    }
}

/**
 * @tc.name: AbilityManagerService_DumpSysAbilityInner_0100
 * @tc.desc: DumpSysAbilityInner
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AbilityManagerServiceDumpTest, AbilityManagerService_DumpSysAbilityInner_0100, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::string args = "-a";
    std::vector<std::string> info;
    bool isClient = false;
    bool isUserID = true;
    abilityMs_->DumpSysAbilityInner(args, info, isClient, isUserID, USER_ID);
    EXPECT_GT(info.size(), SIZE_ZERO);
}

/**
 * @tc.name: AbilityManagerService_DumpSysStateInner_0100
 * @tc.desc: DumpSysStateInner
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AbilityManagerServiceDumpTest, AbilityManagerService_DumpSysStateInner_0100, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::string args = "-a";
    std::vector<std::string> info;
    bool isClient = false;
    bool isUserID = true;
    abilityMs_->DumpSysStateInner(args, info, isClient, isUserID, USER_ID);
    EXPECT_GT(info.size(), SIZE_ZERO);
}

/**
 * @tc.name: AbilityManagerService_DumpSysPendingInner_0100
 * @tc.desc: DumpSysPendingInner
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AbilityManagerServiceDumpTest, AbilityManagerService_DumpSysPendingInner_0100, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::string args = "-a";
    std::vector<std::string> info;
    bool isClient = false;
    bool isUserID = true;
    abilityMs_->DumpSysPendingInner(args, info, isClient, isUserID, USER_ID);
    EXPECT_GT(info.size(), SIZE_ZERO);
}

/**
 * @tc.name: AbilityManagerService_DataDumpSysStateInner_0100
 * @tc.desc: DataDumpSysStateInner
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AbilityManagerServiceDumpTest, AbilityManagerService_DataDumpSysStateInner_0100, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::string args = "-a";
    std::vector<std::string> info;
    bool isClient = false;
    bool isUserID = true;
    abilityMs_->DataDumpSysStateInner(args, info, isClient, isUserID, USER_ID);
    EXPECT_GT(info.size(), SIZE_ZERO);
}

/**
 * @tc.name: AbilityManagerService_OnAppStateChanged_0100
 * @tc.desc: OnAppStateChanged
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AbilityManagerServiceDumpTest, AbilityManagerService_OnAppStateChanged_0100, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);

    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.process = STRING_PROCESS_NAME;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    EXPECT_NE(abilityRecord, nullptr);

    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto missionListManager = std::make_shared<MissionListManager>(USER_ID);
        missionListManager->Init();
        abilityMs_->subManagersHelper_->currentMissionListManager_ = missionListManager;
        missionListManager->terminateAbilityList_.push_back(abilityRecord);

        abilityMs_->subManagersHelper_->currentDataAbilityManager_ = std::make_shared<DataAbilityManager>();
        EXPECT_NE(abilityMs_->subManagersHelper_->currentDataAbilityManager_, nullptr);

        AppInfo info;
        info.processName = STRING_PROCESS_NAME;
        info.state = AppState::TERMINATED;
        abilityMs_->OnAppStateChanged(info);

        abilityRecord = missionListManager->terminateAbilityList_.front();
        EXPECT_NE(abilityRecord, nullptr);
        EXPECT_NE(abilityRecord->GetAppState(), AppState::TERMINATED);
    }
}

/**
 * @tc.name: DumpUIExtensionRootHostRunningInfos_0100
 * @tc.desc: Dump ui extension root host info.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceDumpTest, DumpUIExtensionRootHostRunningInfos_0100, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);
    pid_t pid = 1;
    std::vector<std::string> info;
    abilityMs_->DumpUIExtensionRootHostRunningInfos(pid, info);
    EXPECT_EQ(info.size(), 0);
}

/**
 * @tc.name: DumpUIExtensionProviderRunningInfos_0100
 * @tc.desc: Dump ui extension provider info.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceDumpTest, DumpUIExtensionProviderRunningInfos_0100, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);
    pid_t hostPid = 1;
    std::vector<std::string> info;
    abilityMs_->DumpUIExtensionRootHostRunningInfos(hostPid, info);
    EXPECT_EQ(info.size(), 0);
}
}  // namespace AAFwk
}  // namespace OHOS
