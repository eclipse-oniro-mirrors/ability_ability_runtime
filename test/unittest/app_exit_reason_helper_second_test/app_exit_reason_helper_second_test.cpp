/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "app_exit_reason_helper.h"
#include "task_handler_wrap.h"
#include "ability_event_handler.h"
#include "ability_manager_service.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
namespace {
}  // namespace
class AppExitReasonHelperTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AppExitReasonHelperTest::SetUpTestCase(void)
{}

void AppExitReasonHelperTest::TearDownTestCase(void)
{}

void AppExitReasonHelperTest::SetUp()
{}

void AppExitReasonHelperTest::TearDown()
{}

/**
 * @tc.name: GetRunningProcessInfo_0100
 * @tc.desc: GetRunningProcessInfo
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AppExitReasonHelperTest, GetRunningProcessInfo_0100, TestSize.Level1)
{
    auto appExitReasonHelper = std::make_shared<AAFwk::AppExitReasonHelper>(nullptr);

    auto pid = AAFwk::NO_PID;
    int32_t userId = -1;
    std::string bundleName;
    AppExecFwk::RunningProcessInfo processInfo;
    appExitReasonHelper->GetRunningProcessInfo(pid, userId, bundleName, processInfo);
    EXPECT_TRUE(processInfo.processName_.empty());
    userId = 1;
    appExitReasonHelper->GetRunningProcessInfo(pid, userId, bundleName, processInfo);
    EXPECT_TRUE(processInfo.processName_.empty());
    bundleName = "testBUncleName";
    appExitReasonHelper->GetRunningProcessInfo(pid, userId, bundleName, processInfo);
    EXPECT_TRUE(processInfo.processName_.empty());
    pid = 1;
    appExitReasonHelper->GetRunningProcessInfo(pid, userId, bundleName, processInfo);
    EXPECT_TRUE(processInfo.processName_.empty());
}

/**
 * @tc.name: RecordUIAbilityExitReason_0100
 * @tc.desc: RecordUIAbilityExitReason
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AppExitReasonHelperTest, RecordUIAbilityExitReason_0100, TestSize.Level1)
{
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_;
    taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(AAFwk::AbilityConfig::NAME_ABILITY_MGR_SERVICE);
    std::weak_ptr<AAFwk::AbilityManagerService> service;
    service = AAFwk::AbilityManagerService::GetPubInstance();
    std::shared_ptr<AAFwk::AbilityEventHandler> eventHandler_;
    eventHandler_ = std::make_shared<AAFwk::AbilityEventHandler>(taskHandler_, service);
    std::shared_ptr<AAFwk::SubManagersHelper> subManagersHelper_;
    subManagersHelper_ = std::make_shared<AAFwk::SubManagersHelper>(taskHandler_, eventHandler_);
    std::shared_ptr<AAFwk::AppExitReasonHelper> appExitReasonHelper_;
    appExitReasonHelper_ = std::make_shared<AAFwk::AppExitReasonHelper>(subManagersHelper_);

    int32_t pid = 0;
    std::string abilityName("entry111");
    AAFwk::ExitReason exitReason = { AAFwk::REASON_USER_REQUEST, "User Request" };
    int32_t result = appExitReasonHelper_->RecordUIAbilityExitReason(pid, abilityName, exitReason);
    EXPECT_NE(result, ERR_OK);
    result = appExitReasonHelper_->RecordUIAbilityExitReason(pid, abilityName, exitReason, 1234);
    EXPECT_NE(result, ERR_OK);
}

/**
 * @tc.name: RecordKillCallerInfo_0100
 * @tc.desc: RecordKillCallerInfo
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AppExitReasonHelperTest, RecordKillCallerInfo_0100, TestSize.Level1)
{
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_;
    taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(AAFwk::AbilityConfig::NAME_ABILITY_MGR_SERVICE);
    std::weak_ptr<AAFwk::AbilityManagerService> service;
    service = AAFwk::AbilityManagerService::GetPubInstance();
    std::shared_ptr<AAFwk::AbilityEventHandler> eventHandler_;
    eventHandler_ = std::make_shared<AAFwk::AbilityEventHandler>(taskHandler_, service);
    std::shared_ptr<AAFwk::SubManagersHelper> subManagersHelper_;
    subManagersHelper_ = std::make_shared<AAFwk::SubManagersHelper>(taskHandler_, eventHandler_);
    std::shared_ptr<AAFwk::AppExitReasonHelper> appExitReasonHelper_;
    appExitReasonHelper_ = std::make_shared<AAFwk::AppExitReasonHelper>(subManagersHelper_);

    EXPECT_EQ(appExitReasonHelper_->RecordKillCallerInfo(0, 1234), ERR_INVALID_VALUE);
    EXPECT_EQ(appExitReasonHelper_->RecordKillCallerInfo(-1, 1234), ERR_INVALID_VALUE);
    EXPECT_EQ(appExitReasonHelper_->RecordKillCallerInfo(1, -1), ERR_INVALID_VALUE);
    EXPECT_NE(appExitReasonHelper_->RecordKillCallerInfo(1, 1234), ERR_OK);
}

/**
 * @tc.name: RecordAppRestartExitReason_0100
 * @tc.desc: RecordAppRestartExitReason
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AppExitReasonHelperTest, RecordAppRestartExitReason_0100, TestSize.Level1)
{
    auto abilityMs = AAFwk::AbilityManagerService::GetPubInstance();
    ASSERT_NE(abilityMs, nullptr);
    // app recovery path: return early without recording
    abilityMs->RecordAppRestartExitReason(true, 1234, 5678);
    // normal restart path: record chain (no appmgr service in UT, best-effort log only)
    abilityMs->RecordAppRestartExitReason(false, 1234, 5678);
    // null helper branch: return without recording
    auto appExitReasonHelper = abilityMs->appExitReasonHelper_;
    abilityMs->appExitReasonHelper_ = nullptr;
    abilityMs->RecordAppRestartExitReason(false, 1234, 5678);
    abilityMs->appExitReasonHelper_ = appExitReasonHelper;
}

/**
 * @tc.name: GetActiveAbilityListWithPid_0100
 * @tc.desc: GetActiveAbilityListWithPid
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AppExitReasonHelperTest, GetActiveAbilityListWithPid_0100, TestSize.Level1)
{
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_;
    taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(AAFwk::AbilityConfig::NAME_ABILITY_MGR_SERVICE);
    std::weak_ptr<AAFwk::AbilityManagerService> service;
    service = AAFwk::AbilityManagerService::GetPubInstance();
    std::shared_ptr<AAFwk::AbilityEventHandler> eventHandler_;
    eventHandler_ = std::make_shared<AAFwk::AbilityEventHandler>(taskHandler_, service);
    std::shared_ptr<AAFwk::SubManagersHelper> subManagersHelper_;
    subManagersHelper_ = std::make_shared<AAFwk::SubManagersHelper>(taskHandler_, eventHandler_);
    std::shared_ptr<AAFwk::AppExitReasonHelper> appExitReasonHelper_;
    appExitReasonHelper_ = std::make_shared<AAFwk::AppExitReasonHelper>(subManagersHelper_);

    int32_t uid = 0;
    int32_t pid = 0;
    std::vector<std::string> abilityList = {};
    int32_t result = appExitReasonHelper_->GetActiveAbilityListWithPid(uid, abilityList, pid);
    EXPECT_EQ(abilityList.size(), 0);
}

/**
 * @tc.name: GetRunningProcessInfos_0100
 * @tc.desc: GetRunningProcessInfos
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AppExitReasonHelperTest, GetRunningProcessInfos_0100, TestSize.Level1)
{
    auto appExitReasonHelper = std::make_shared<AAFwk::AppExitReasonHelper>(nullptr);

    auto pid = AAFwk::NO_PID;
    int32_t userId = -1;
    std::string bundleName = "com.example.app";
    auto res = appExitReasonHelper->GetRunningProcessInfos(userId, bundleName);
    EXPECT_TRUE(res.empty());
}

/**
 * @tc.name: GetRunningProcessInfos_0200
 * @tc.desc: GetRunningProcessInfos
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AppExitReasonHelperTest, GetRunningProcessInfos_0200, TestSize.Level1)
{
    auto appExitReasonHelper = std::make_shared<AAFwk::AppExitReasonHelper>(nullptr);

    auto pid = AAFwk::NO_PID;
    int32_t userId = 100;
    std::string bundleName = "";
    auto res = appExitReasonHelper->GetRunningProcessInfos(userId, bundleName);
    EXPECT_TRUE(res.empty());
}

/**
 * @tc.name: GetRunningProcessInfos_0300
 * @tc.desc: GetRunningProcessInfos
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AppExitReasonHelperTest, GetRunningProcessInfos_0300, TestSize.Level1)
{
    auto appExitReasonHelper = std::make_shared<AAFwk::AppExitReasonHelper>(nullptr);

    auto pid = AAFwk::NO_PID;
    int32_t userId = -1;
    std::string bundleName = "";
    auto res = appExitReasonHelper->GetRunningProcessInfos(userId, bundleName);
    EXPECT_TRUE(res.empty());
}
}  // namespace AbilityRuntime
}  // namespace OHOS
