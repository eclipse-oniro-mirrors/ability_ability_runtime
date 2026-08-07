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

#include "hilog_tag_wrapper.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "mock_bundle_manager.h"
#include "mock_quick_fix_util.h"
#include "nativetoken_kit.h"
#include "quick_fix_error_utils.h"
#define private public
#include "quick_fix_manager_service.h"
#include "quick_fix_manager_apply_task.h"
#undef private
#include "quick_fix_result_info.h"
#include "system_ability_definition.h"
#include "token_setproc.h"
#include "json_utils.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
namespace {
template<typename F>
static void WaitUntilTaskCalled(const F &f, const std::shared_ptr<AppExecFwk::EventHandler> &handler,
    std::atomic<bool> &taskCalled)
{
    const uint32_t maxRetryCount = 1000;
    const uint32_t sleepTime = 1000;
    uint32_t count = 0;
    if (handler->PostTask(f)) {
        while (!taskCalled.load()) {
            ++count;
            if (count >= maxRetryCount) {
                break;
            }
            usleep(sleepTime);
        }
    }
}

static void WaitUntilTaskDone(const std::shared_ptr<AppExecFwk::EventHandler> &handler)
{
    std::atomic<bool> taskCalled(false);
    auto f = [&taskCalled]() { taskCalled.store(true); };
    WaitUntilTaskCalled(f, handler, taskCalled);
}

static void SetPermission()
{
    uint64_t tokenId;
    const char **perms = new const char *[1];
    perms[0] = "ohos.permission.RUNNING_STATE_OBSERVER";
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 1,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .aplStr = "system_basic",
    };

    infoInstance.processName = "SetUpTestCase";
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    delete[] perms;
}
} // namespace

class QuickFixManagerApplyTaskMoreTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<QuickFixManagerService> quickFixMs_ = nullptr;
    sptr<AppExecFwk::QuickFixManagerHostImpl> bundleQfMgr_ = nullptr;
    sptr<AppExecFwk::IAppMgr> appMgr_ = nullptr;
    std::shared_ptr<QuickFixUtil> quickFixUtil_ = nullptr;
};

void QuickFixManagerApplyTaskMoreTest::SetUpTestCase(void) {}

void QuickFixManagerApplyTaskMoreTest::TearDownTestCase(void) {}

void QuickFixManagerApplyTaskMoreTest::SetUp()
{
    quickFixMs_ = QuickFixManagerService::GetInstance();
    ASSERT_NE(quickFixMs_, nullptr);

    auto ret = quickFixMs_->Init();
    EXPECT_TRUE(ret);
    EXPECT_NE(quickFixMs_->eventRunner_, nullptr);
    EXPECT_NE(quickFixMs_->eventHandler_, nullptr);

    quickFixUtil_ = std::make_shared<QuickFixUtil>();
    ASSERT_NE(quickFixUtil_, nullptr);
    sptr<IRemoteObject> bundleObject = new (std::nothrow) AppExecFwk::BundleMgrService();
    quickFixUtil_->RegisterSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, bundleObject);

    bundleQfMgr_ = new (std::nothrow) AppExecFwk::QuickFixManagerHostImpl();
    ASSERT_NE(bundleQfMgr_, nullptr);

    appMgr_ = QuickFixUtil::GetAppManagerProxy();
    ASSERT_NE(appMgr_, nullptr);
}

void QuickFixManagerApplyTaskMoreTest::TearDown() {}

/**
 * @tc.name: HandlePatchSwitched_0200
 * @tc.desc: when isRunning_ true and not so-contained, HandlePatchSwitched posts notify load repair patch task.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, HandlePatchSwitched_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    applyTask->isRunning_ = true;
    applyTask->isSoContained_ = false;
    applyTask->HandlePatchSwitched();
    WaitUntilTaskDone(quickFixMs_->eventHandler_);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandlePatchSwitched_0300
 * @tc.desc: when isRunning_ false, HandlePatchSwitched posts delete quick fix task.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, HandlePatchSwitched_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    applyTask->isRunning_ = false;
    EXPECT_CALL(*bundleQfMgr_, DeleteQuickFix(_, _)).Times(1);
    applyTask->HandlePatchSwitched();
    WaitUntilTaskDone(quickFixMs_->eventHandler_);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandlePatchDeleted_0200
 * @tc.desc: running, not so-contained, HOT_RELOAD type should post notify hot reload page task.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, HandlePatchDeleted_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    applyTask->isRunning_ = true;
    applyTask->isSoContained_ = false;
    applyTask->type_ = AppExecFwk::QuickFixType::HOT_RELOAD;
    applyTask->HandlePatchDeleted();
    WaitUntilTaskDone(quickFixMs_->eventHandler_);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandlePatchDeleted_0300
 * @tc.desc: running, not so-contained, PATCH type should notify apply status and remove self.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, HandlePatchDeleted_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    applyTask->isRunning_ = true;
    applyTask->isSoContained_ = false;
    applyTask->type_ = AppExecFwk::QuickFixType::PATCH;
    applyTask->bundleName_ = "testBundle";
    applyTask->HandlePatchDeleted();
    WaitUntilTaskDone(quickFixMs_->eventHandler_);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandlePatchDeleted_0400
 * @tc.desc: not running should notify apply status and remove self.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, HandlePatchDeleted_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    applyTask->isRunning_ = false;
    applyTask->bundleName_ = "testBundle";
    applyTask->HandlePatchDeleted();
    WaitUntilTaskDone(quickFixMs_->eventHandler_);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleRevokeQuickFixAppRunning_0300
 * @tc.desc: isSoContained true should register app state observer and remove timeout task.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, HandleRevokeQuickFixAppRunning_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    SetPermission();
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    applyTask->isSoContained_ = true;
    applyTask->bundleName_ = "testBundle";
    applyTask->HandleRevokeQuickFixAppRunning();
    WaitUntilTaskDone(quickFixMs_->eventHandler_);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleRevokeQuickFixAppRunning_0400
 * @tc.desc: isSoContained false should fall through to HandleRevokeQuickFixAppStop.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, HandleRevokeQuickFixAppRunning_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    applyTask->isSoContained_ = false;
    applyTask->bundleName_ = "testBundle";
    EXPECT_CALL(*bundleQfMgr_, SwitchQuickFix(_, _, _)).Times(1);
    applyTask->HandleRevokeQuickFixAppRunning();
    WaitUntilTaskDone(quickFixMs_->eventHandler_);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleRevokeQuickFixAppStop_0300
 * @tc.desc: null bundleQfMgr should notify bundlemgr invalid and remove self.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, HandleRevokeQuickFixAppStop_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(nullptr, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    applyTask->bundleName_ = "testBundle";
    applyTask->HandleRevokeQuickFixAppStop();
    WaitUntilTaskDone(quickFixMs_->eventHandler_);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleRevokeQuickFixAppStop_0400
 * @tc.desc: SwitchQuickFix returning non-zero should notify switch failed and remove self.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, HandleRevokeQuickFixAppStop_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    applyTask->bundleName_ = "testBundle";
    ON_CALL(*bundleQfMgr_, SwitchQuickFix(_, _, _)).WillByDefault(Return(1));
    applyTask->HandleRevokeQuickFixAppStop();
    WaitUntilTaskDone(quickFixMs_->eventHandler_);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: PostRevokeQuickFixDeleteTask_0200
 * @tc.desc: null bundleQfMgr should notify bundlemgr invalid and remove self.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, PostRevokeQuickFixDeleteTask_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(nullptr, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    applyTask->bundleName_ = "testBundle";
    applyTask->PostRevokeQuickFixDeleteTask();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: PostRevokeQuickFixDeleteTask_0300
 * @tc.desc: DeleteQuickFix returning non-zero should notify delete failed and remove self.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, PostRevokeQuickFixDeleteTask_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    applyTask->bundleName_ = "testBundle";
    ON_CALL(*bundleQfMgr_, DeleteQuickFix(_, _)).WillByDefault(Return(1));
    applyTask->PostRevokeQuickFixDeleteTask();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: PostRevokeQuickFixNotifyUnloadPatchTask_0200
 * @tc.desc: null appMgr should notify appmgr invalid and remove self.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, PostRevokeQuickFixNotifyUnloadPatchTask_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, nullptr,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    applyTask->bundleName_ = "testBundle";
    applyTask->PostRevokeQuickFixNotifyUnloadPatchTask();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleRevokePatchSwitched_0200
 * @tc.desc: not running should post revoke delete task which calls DeleteQuickFix.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, HandleRevokePatchSwitched_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    applyTask->bundleName_ = "testBundle";
    EXPECT_CALL(*bundleQfMgr_, DeleteQuickFix(_, _)).Times(1);
    applyTask->HandleRevokePatchSwitched();
    WaitUntilTaskDone(quickFixMs_->eventHandler_);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleRevokePatchDeleted_0200
 * @tc.desc: revoke patch deleted should notify apply status OK and remove self.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, HandleRevokePatchDeleted_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    applyTask->bundleName_ = "testBundle";
    applyTask->taskType_ = QuickFixManagerApplyTask::TaskType::QUICK_FIX_REVOKE;
    applyTask->HandleRevokePatchDeleted();
    WaitUntilTaskDone(quickFixMs_->eventHandler_);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: PostDeployQuickFixTask_0300
 * @tc.desc: null bundleQfMgr should notify bundlemgr invalid and remove self.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, PostDeployQuickFixTask_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(nullptr, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    std::vector<std::string> files;
    applyTask->PostDeployQuickFixTask(files, false, false, false);
    WaitUntilTaskDone(quickFixMs_->eventHandler_);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: PostDeployQuickFixTask_0400
 * @tc.desc: DeployQuickFix returning non-zero should notify deploy failed and remove self.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, PostDeployQuickFixTask_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    ON_CALL(*bundleQfMgr_, DeployQuickFix(_, _, _, _, _, _)).WillByDefault(Return(1));
    std::vector<std::string> files;
    applyTask->PostDeployQuickFixTask(files, false, false, false);
    WaitUntilTaskDone(quickFixMs_->eventHandler_);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: PostSwitchQuickFixTask_0300
 * @tc.desc: null bundleQfMgr should notify bundlemgr invalid and remove self.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, PostSwitchQuickFixTask_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(nullptr, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    applyTask->bundleName_ = "testBundle";
    applyTask->PostSwitchQuickFixTask();
    WaitUntilTaskDone(quickFixMs_->eventHandler_);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: PostSwitchQuickFixTask_0400
 * @tc.desc: SwitchQuickFix returning non-zero should notify switch failed and remove self.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, PostSwitchQuickFixTask_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    applyTask->bundleName_ = "testBundle";
    ON_CALL(*bundleQfMgr_, SwitchQuickFix(_, _, _)).WillByDefault(Return(1));
    applyTask->PostSwitchQuickFixTask();
    WaitUntilTaskDone(quickFixMs_->eventHandler_);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: PostDeleteQuickFixTask_0300
 * @tc.desc: null bundleQfMgr should notify bundlemgr invalid and remove self.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, PostDeleteQuickFixTask_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(nullptr, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    applyTask->bundleName_ = "testBundle";
    applyTask->PostDeleteQuickFixTask();
    WaitUntilTaskDone(quickFixMs_->eventHandler_);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: PostDeleteQuickFixTask_0400
 * @tc.desc: DeleteQuickFix returning non-zero should notify delete failed and remove self.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, PostDeleteQuickFixTask_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    applyTask->bundleName_ = "testBundle";
    ON_CALL(*bundleQfMgr_, DeleteQuickFix(_, _)).WillByDefault(Return(1));
    applyTask->PostDeleteQuickFixTask();
    WaitUntilTaskDone(quickFixMs_->eventHandler_);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: PostTimeOutTask_0300
 * @tc.desc: post timeout task then remove it should clear the pending timeout task without crash.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, PostTimeOutTask_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    applyTask->PostTimeOutTask();
    applyTask->RemoveTimeoutTask();
    EXPECT_NE(quickFixMs_->eventHandler_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: NotifyApplyStatus_0500
 * @tc.desc: apply task with valid bundleName and OK code should publish common event without crash.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, NotifyApplyStatus_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    applyTask->bundleName_ = "testBundle";
    applyTask->bundleVersionCode_ = 1;
    applyTask->patchVersionCode_ = 100;
    applyTask->taskType_ = QuickFixManagerApplyTask::TaskType::QUICK_FIX_APPLY;
    applyTask->NotifyApplyStatus(QUICK_FIX_OK);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: NotifyApplyStatus_0600
 * @tc.desc: revoke task with valid bundleName should publish revoke result common event.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, NotifyApplyStatus_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    applyTask->bundleName_ = "testBundle";
    applyTask->taskType_ = QuickFixManagerApplyTask::TaskType::QUICK_FIX_REVOKE;
    applyTask->NotifyApplyStatus(QUICK_FIX_OK);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: NotifyApplyStatus_0700
 * @tc.desc: apply task with empty bundleName should still publish common event.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, NotifyApplyStatus_0700, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    applyTask->bundleName_ = "";
    applyTask->taskType_ = QuickFixManagerApplyTask::TaskType::QUICK_FIX_APPLY;
    applyTask->NotifyApplyStatus(QUICK_FIX_OK);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: NotifyApplyStatus_0800
 * @tc.desc: revoke task with deploy-failed code should publish revoke result.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, NotifyApplyStatus_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    applyTask->bundleName_ = "testBundle";
    applyTask->taskType_ = QuickFixManagerApplyTask::TaskType::QUICK_FIX_REVOKE;
    applyTask->NotifyApplyStatus(QUICK_FIX_DEPLOY_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: NotifyApplyStatus_0900
 * @tc.desc: apply task with process-timeout code should publish apply result.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, NotifyApplyStatus_0900, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    applyTask->bundleName_ = "testBundle";
    applyTask->taskType_ = QuickFixManagerApplyTask::TaskType::QUICK_FIX_APPLY;
    applyTask->NotifyApplyStatus(QUICK_FIX_PROCESS_TIMEOUT);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: Run_0200
 * @tc.desc: Run should set taskType to APPLY.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, Run_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    EXPECT_CALL(*bundleQfMgr_, DeployQuickFix(_, _, _, _, _, _)).Times(1);
    std::vector<std::string> files;
    applyTask->Run(files);
    WaitUntilTaskDone(quickFixMs_->eventHandler_);
    EXPECT_EQ(applyTask->taskType_, QuickFixManagerApplyTask::TaskType::QUICK_FIX_APPLY);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: RunRevoke_0200
 * @tc.desc: RunRevoke should set taskType to REVOKE.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, RunRevoke_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    applyTask->RunRevoke();
    WaitUntilTaskDone(quickFixMs_->eventHandler_);
    EXPECT_EQ(applyTask->taskType_, QuickFixManagerApplyTask::TaskType::QUICK_FIX_REVOKE);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: ExtractQuickFixDataFromJson_1200
 * @tc.desc: empty json object should return false due to missing bundleName.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, ExtractQuickFixDataFromJson_1200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    nlohmann::json json = nlohmann::json::object();
    EXPECT_FALSE(applyTask->ExtractQuickFixDataFromJson(json));
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: ExtractQuickFixDataFromJson_1300
 * @tc.desc: json with only bundleName should return false due to missing bundleVersionCode.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, ExtractQuickFixDataFromJson_1300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    nlohmann::json json;
    json["bundleName"] = "com.example.bundle";
    EXPECT_FALSE(applyTask->ExtractQuickFixDataFromJson(json));
    EXPECT_EQ(applyTask->bundleName_, "com.example.bundle");
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: ExtractQuickFixDataFromJson_1400
 * @tc.desc: json with empty bundleName string should return true and set bundleName empty.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, ExtractQuickFixDataFromJson_1400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    nlohmann::json json;
    json["bundleName"] = "";
    json["bundleVersionCode"] = 1;
    json["patchVersionCode"] = 1;
    json["isSoContained"] = false;
    json["type"] = 0;
    EXPECT_TRUE(applyTask->ExtractQuickFixDataFromJson(json));
    EXPECT_EQ(applyTask->bundleName_, "");
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: ExtractQuickFixDataFromJson_1500
 * @tc.desc: json with negative type value should return true and cast to QuickFixType.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, ExtractQuickFixDataFromJson_1500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    nlohmann::json json;
    json["bundleName"] = "com.example.bundle";
    json["bundleVersionCode"] = 10;
    json["patchVersionCode"] = 20;
    json["isSoContained"] = true;
    json["type"] = -1;
    EXPECT_TRUE(applyTask->ExtractQuickFixDataFromJson(json));
    EXPECT_EQ(applyTask->isSoContained_, true);
    EXPECT_EQ(static_cast<int32_t>(applyTask->type_), -1);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: SetQuickFixInfo_0500
 * @tc.desc: valid result with bundleName and PATCH type should return true and populate fields.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, SetQuickFixInfo_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    auto result = std::make_shared<AppExecFwk::DeployQuickFixResult>();
    result->bundleName = "com.example.bundle";
    result->type = AppExecFwk::QuickFixType::PATCH;
    EXPECT_TRUE(applyTask->SetQuickFixInfo(result));
    EXPECT_EQ(applyTask->bundleName_, "com.example.bundle");
    EXPECT_EQ(applyTask->type_, AppExecFwk::QuickFixType::PATCH);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: SetQuickFixInfo_0600
 * @tc.desc: valid result with HOT_RELOAD type should return true and set type field.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, SetQuickFixInfo_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    auto result = std::make_shared<AppExecFwk::DeployQuickFixResult>();
    result->bundleName = "com.example.bundle";
    result->type = AppExecFwk::QuickFixType::HOT_RELOAD;
    bool res = applyTask->SetQuickFixInfo(result);
    ASSERT_TRUE(res);
    EXPECT_EQ(applyTask->type_, AppExecFwk::QuickFixType::HOT_RELOAD);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetRunningState_0200
 * @tc.desc: null appMgr should return false.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, GetRunningState_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, nullptr,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    EXPECT_FALSE(applyTask->GetRunningState());
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetRunningState_0300
 * @tc.desc: valid appMgr with default mock return should return false.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, GetRunningState_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    applyTask->bundleName_ = "testBundle";
    EXPECT_FALSE(applyTask->GetRunningState());
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: InitRevokeTask_0200
 * @tc.desc: InitRevokeTask should set bundleName and isSoContained.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, InitRevokeTask_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    applyTask->InitRevokeTask("revokeBundle", true);
    EXPECT_EQ(applyTask->GetBundleName(), "revokeBundle");
    EXPECT_EQ(applyTask->isSoContained_, true);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: RemoveTimeoutTask_0300
 * @tc.desc: remove timeout task with valid handler should complete without crash.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, RemoveTimeoutTask_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    applyTask->RemoveTimeoutTask();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: RegAppStateObserver_0300
 * @tc.desc: valid appMgr with permission should register observer and set appStateCallback.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, RegAppStateObserver_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    SetPermission();
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    applyTask->bundleName_ = "testBundle";
    applyTask->RegAppStateObserver();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UnregAppStateObserver_0300
 * @tc.desc: null appStateCallback should early-return without crash.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, UnregAppStateObserver_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    applyTask->appStateCallback_ = nullptr;
    applyTask->UnregAppStateObserver();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: PostNotifyLoadRepairPatchTask_0300
 * @tc.desc: valid handler should post load repair patch task without crash.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, PostNotifyLoadRepairPatchTask_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    applyTask->bundleName_ = "testBundle";
    applyTask->PostNotifyLoadRepairPatchTask();
    WaitUntilTaskDone(quickFixMs_->eventHandler_);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: PostNotifyUnloadRepairPatchTask_0300
 * @tc.desc: valid handler should post unload repair patch task without crash.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, PostNotifyUnloadRepairPatchTask_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    applyTask->bundleName_ = "testBundle";
    applyTask->PostNotifyUnloadRepairPatchTask();
    WaitUntilTaskDone(quickFixMs_->eventHandler_);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: PostNotifyHotReloadPageTask_0300
 * @tc.desc: valid handler should post hot reload page task without crash.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, PostNotifyHotReloadPageTask_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    applyTask->bundleName_ = "testBundle";
    applyTask->PostNotifyHotReloadPageTask();
    WaitUntilTaskDone(quickFixMs_->eventHandler_);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: PostRevokeQuickFixTask_0300
 * @tc.desc: valid handler with revoke taskType should post revoke task without crash.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, PostRevokeQuickFixTask_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    applyTask->bundleName_ = "testBundle";
    applyTask->taskType_ = QuickFixManagerApplyTask::TaskType::QUICK_FIX_REVOKE;
    applyTask->PostRevokeQuickFixTask();
    WaitUntilTaskDone(quickFixMs_->eventHandler_);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: PostRevokeQuickFixProcessDiedTask_0200
 * @tc.desc: valid handler should post process died task without crash.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskMoreTest, PostRevokeQuickFixProcessDiedTask_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    applyTask->bundleName_ = "testBundle";
    applyTask->PostRevokeQuickFixProcessDiedTask();
    WaitUntilTaskDone(quickFixMs_->eventHandler_);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}
} // namespace AAFwk
} // namespace OHOS
