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
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <climits>
#include <memory>

#include "c/executor_task.h"
#include "app_pidfd_manager.h"
#include "mock_app_mgr_service_inner.h"
#include "mock_task_handler_wrap.h"
#include "hilog_tag_wrapper.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AAFwk;

namespace OHOS {
namespace AppExecFwk {

class AppPidFdManagerTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;

    void ClearPidFdMap();
    AppPidFdManager &GetManager();

protected:
    std::shared_ptr<MockAppMgrServiceInner> inner_;
    std::shared_ptr<MockTaskHandlerWrap> handler_;
};

AppPidFdManager &AppPidFdManagerTest::GetManager()
{
    return AppPidFdManager::GetInstance();
}

void AppPidFdManagerTest::ClearPidFdMap()
{
    auto &mgr = GetManager();
    std::lock_guard<std::mutex> lock(mgr.mapMutex_);
    for (auto &item : mgr.pidfdMap_) {
        if (item.second.pidfd >= 0) {
            ffrt_qos_t qos = 0;
            ffrt_epoll_ctl(qos, EPOLL_CTL_DEL, item.second.pidfd, 0, nullptr, nullptr);
            close(item.second.pidfd);
            item.second.pidfd = -1;
        }
    }
    mgr.pidfdMap_.clear();
}

void AppPidFdManagerTest::SetUp()
{
    ClearPidFdMap();
    inner_ = std::make_shared<MockAppMgrServiceInner>();
    handler_ = MockTaskHandlerWrap::CreateQueueHandler("app_pidfd_test_queue");
    ASSERT_NE(inner_, nullptr);
    ASSERT_NE(handler_, nullptr);
    GetManager().Init(inner_, handler_);
}

void AppPidFdManagerTest::TearDown()
{
    ClearPidFdMap();
    auto &mgr = GetManager();
    mgr.appMgrServiceInner_.reset();
    mgr.taskHandler_.reset();
    if (inner_) {
        Mock::VerifyAndClearExpectations(inner_.get());
    }
    if (handler_) {
        Mock::VerifyAndClearExpectations(handler_.get());
    }
    inner_.reset();
    handler_.reset();
}

/**
 * @tc.number: AppPidFdManagerTest_GetInstance_001
 * @tc.desc: Verify GetInstance returns the same singleton instance across calls.
 * @tc.type: FUNC
 * @tc.Function: GetInstance
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPidFdManagerTest, AppPidFdManagerTest_GetInstance_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPidFdManagerTest_GetInstance_001 start.");
    auto &first = AppPidFdManager::GetInstance();
    auto &second = AppPidFdManager::GetInstance();
    EXPECT_EQ(&first, &second);
}

/**
 * @tc.number: AppPidFdManagerTest_Init_001
 * @tc.desc: Verify Init stores weak_ptr references so that DispatchCleanup can submit tasks.
 * @tc.type: FUNC
 * @tc.Function: Init
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPidFdManagerTest, AppPidFdManagerTest_Init_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPidFdManagerTest_Init_001 start.");
    EXPECT_CALL(*handler_, SubmitTaskInner(_, _)).Times(1);
    GetManager().DispatchCleanup(100, PidFdType::CHILD);
}

/**
 * @tc.number: AppPidFdManagerTest_AddWatcher_001
 * @tc.desc: AddWatcher with pid <= 0 should return early without touching the map or submitting tasks.
 * @tc.type: FUNC
 * @tc.Function: AddWatcher
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPidFdManagerTest, AppPidFdManagerTest_AddWatcher_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPidFdManagerTest_AddWatcher_001 start.");
    GetManager().AddWatcher(0, PidFdType::CHILD);
    std::lock_guard<std::mutex> lock(GetManager().mapMutex_);
    EXPECT_TRUE(GetManager().pidfdMap_.empty());
}

/**
 * @tc.number: AppPidFdManagerTest_AddWatcher_002
 * @tc.desc: AddWatcher with negative pid should return early.
 * @tc.type: FUNC
 * @tc.Function: AddWatcher
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPidFdManagerTest, AppPidFdManagerTest_AddWatcher_002, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPidFdManagerTest_AddWatcher_002 start.");
    GetManager().AddWatcher(-1, PidFdType::RENDER);
    std::lock_guard<std::mutex> lock(GetManager().mapMutex_);
    EXPECT_TRUE(GetManager().pidfdMap_.empty());
}

/**
 * @tc.number: AppPidFdManagerTest_AddWatcher_003
 * @tc.desc: AddWatcher with non-existent pid (OpenPidFd fails) dispatches cleanup for CHILD type.
 * @tc.type: FUNC
 * @tc.Function: AddWatcher
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPidFdManagerTest, AppPidFdManagerTest_AddWatcher_003, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPidFdManagerTest_AddWatcher_003 start.");
    pid_t invalidPid = static_cast<pid_t>(INT_MAX);
    EXPECT_CALL(*handler_, SubmitTaskInner(_, _)).Times(1);
    GetManager().AddWatcher(invalidPid, PidFdType::CHILD);
    std::lock_guard<std::mutex> lock(GetManager().mapMutex_);
    EXPECT_TRUE(GetManager().pidfdMap_.empty());
}

/**
 * @tc.number: AppPidFdManagerTest_AddWatcher_004
 * @tc.desc: AddWatcher with non-existent pid and RENDER type dispatches cleanup.
 * @tc.type: FUNC
 * @tc.Function: AddWatcher
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPidFdManagerTest, AppPidFdManagerTest_AddWatcher_004, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPidFdManagerTest_AddWatcher_004 start.");
    pid_t invalidPid = static_cast<pid_t>(INT_MAX);
    EXPECT_CALL(*handler_, SubmitTaskInner(_, _)).Times(1);
    GetManager().AddWatcher(invalidPid, PidFdType::RENDER);
    std::lock_guard<std::mutex> lock(GetManager().mapMutex_);
    EXPECT_TRUE(GetManager().pidfdMap_.empty());
}

/**
 * @tc.number: AppPidFdManagerTest_AddWatcher_005
 * @tc.desc: AddWatcher with non-existent pid and no Init (expired weak_ptrs) should not crash or submit tasks.
 * @tc.type: FUNC
 * @tc.Function: AddWatcher
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPidFdManagerTest, AppPidFdManagerTest_AddWatcher_005, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPidFdManagerTest_AddWatcher_005 start.");
    GetManager().appMgrServiceInner_.reset();
    GetManager().taskHandler_.reset();
    pid_t invalidPid = static_cast<pid_t>(INT_MAX);
    GetManager().AddWatcher(invalidPid, PidFdType::CHILD);
    std::lock_guard<std::mutex> lock(GetManager().mapMutex_);
    EXPECT_TRUE(GetManager().pidfdMap_.empty());
}

/**
 * @tc.number: AppPidFdManagerTest_OnChildPidfdEvent_001
 * @tc.desc: OnChildPidfdEvent with null data should return early without side effects.
 * @tc.type: FUNC
 * @tc.Function: OnChildPidfdEvent
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPidFdManagerTest, AppPidFdManagerTest_OnChildPidfdEvent_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPidFdManagerTest_OnChildPidfdEvent_001 start.");
    AppPidFdManager::OnChildPidfdEvent(nullptr, EPOLLIN);
    std::lock_guard<std::mutex> lock(GetManager().mapMutex_);
    EXPECT_TRUE(GetManager().pidfdMap_.empty());
}

/**
 * @tc.number: AppPidFdManagerTest_OnChildPidfdEvent_002
 * @tc.desc: OnChildPidfdEvent with pid not in map should call OnPidfdFired which returns early.
 * @tc.type: FUNC
 * @tc.Function: OnChildPidfdEvent
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPidFdManagerTest, AppPidFdManagerTest_OnChildPidfdEvent_002, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPidFdManagerTest_OnChildPidfdEvent_002 start.");
    int32_t pidValue = 9999;
    AppPidFdManager::OnChildPidfdEvent(&pidValue, EPOLLIN);
    std::lock_guard<std::mutex> lock(GetManager().mapMutex_);
    EXPECT_TRUE(GetManager().pidfdMap_.empty());
}

/**
 * @tc.number: AppPidFdManagerTest_OnChildPidfdEvent_003
 * @tc.desc: OnChildPidfdEvent with pid present in map (pidfd=-1) triggers OnPidfdFired cleanup and DispatchCleanup.
 * @tc.type: FUNC
 * @tc.Function: OnChildPidfdEvent
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPidFdManagerTest, AppPidFdManagerTest_OnChildPidfdEvent_003, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPidFdManagerTest_OnChildPidfdEvent_003 start.");
    int32_t pidValue = 8888;
    {
        std::lock_guard<std::mutex> lock(GetManager().mapMutex_);
        GetManager().pidfdMap_[pidValue] = AppPidFdManager::PidFdEntry{-1, pidValue};
    }
    EXPECT_CALL(*handler_, SubmitTaskInner(_, _)).Times(1);
    AppPidFdManager::OnChildPidfdEvent(&pidValue, EPOLLIN);
    std::lock_guard<std::mutex> lock(GetManager().mapMutex_);
    EXPECT_EQ(GetManager().pidfdMap_.count(pidValue), 0u);
}

/**
 * @tc.number: AppPidFdManagerTest_OnRenderPidfdEvent_001
 * @tc.desc: OnRenderPidfdEvent with null data should return early.
 * @tc.type: FUNC
 * @tc.Function: OnRenderPidfdEvent
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPidFdManagerTest, AppPidFdManagerTest_OnRenderPidfdEvent_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPidFdManagerTest_OnRenderPidfdEvent_001 start.");
    AppPidFdManager::OnRenderPidfdEvent(nullptr, EPOLLIN);
    std::lock_guard<std::mutex> lock(GetManager().mapMutex_);
    EXPECT_TRUE(GetManager().pidfdMap_.empty());
}

/**
 * @tc.number: AppPidFdManagerTest_OnRenderPidfdEvent_002
 * @tc.desc: OnRenderPidfdEvent with pid not in map should return early via OnPidfdFired.
 * @tc.type: FUNC
 * @tc.Function: OnRenderPidfdEvent
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPidFdManagerTest, AppPidFdManagerTest_OnRenderPidfdEvent_002, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPidFdManagerTest_OnRenderPidfdEvent_002 start.");
    int32_t pidValue = 7777;
    AppPidFdManager::OnRenderPidfdEvent(&pidValue, EPOLLIN);
    std::lock_guard<std::mutex> lock(GetManager().mapMutex_);
    EXPECT_TRUE(GetManager().pidfdMap_.empty());
}

/**
 * @tc.number: AppPidFdManagerTest_OnRenderPidfdEvent_003
 * @tc.desc: OnRenderPidfdEvent with pid in map (pidfd=-1) triggers cleanup and DispatchCleanup for RENDER type.
 * @tc.type: FUNC
 * @tc.Function: OnRenderPidfdEvent
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPidFdManagerTest, AppPidFdManagerTest_OnRenderPidfdEvent_003, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPidFdManagerTest_OnRenderPidfdEvent_003 start.");
    int32_t pidValue = 6666;
    {
        std::lock_guard<std::mutex> lock(GetManager().mapMutex_);
        GetManager().pidfdMap_[pidValue] = AppPidFdManager::PidFdEntry{-1, pidValue};
    }
    EXPECT_CALL(*handler_, SubmitTaskInner(_, _)).Times(1);
    AppPidFdManager::OnRenderPidfdEvent(&pidValue, EPOLLIN);
    std::lock_guard<std::mutex> lock(GetManager().mapMutex_);
    EXPECT_EQ(GetManager().pidfdMap_.count(pidValue), 0u);
}

/**
 * @tc.number: AppPidFdManagerTest_OnPidfdFired_001
 * @tc.desc: OnPidfdFired with pid not in map should return early without side effects.
 * @tc.type: FUNC
 * @tc.Function: OnPidfdFired
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPidFdManagerTest, AppPidFdManagerTest_OnPidfdFired_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPidFdManagerTest_OnPidfdFired_001 start.");
    GetManager().OnPidfdFired(5555, PidFdType::CHILD);
    std::lock_guard<std::mutex> lock(GetManager().mapMutex_);
    EXPECT_TRUE(GetManager().pidfdMap_.empty());
}

/**
 * @tc.number: AppPidFdManagerTest_OnPidfdFired_002
 * @tc.desc: OnPidfdFired with pid in map and pidfd=-1 should erase entry, skip close/epoll_del, and dispatch cleanup.
 * @tc.type: FUNC
 * @tc.Function: OnPidfdFired
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPidFdManagerTest, AppPidFdManagerTest_OnPidfdFired_002, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPidFdManagerTest_OnPidfdFired_002 start.");
    int32_t pidValue = 4444;
    {
        std::lock_guard<std::mutex> lock(GetManager().mapMutex_);
        GetManager().pidfdMap_[pidValue] = AppPidFdManager::PidFdEntry{-1, pidValue};
    }
    EXPECT_CALL(*handler_, SubmitTaskInner(_, _)).Times(1);
    GetManager().OnPidfdFired(pidValue, PidFdType::CHILD);
    std::lock_guard<std::mutex> lock(GetManager().mapMutex_);
    EXPECT_EQ(GetManager().pidfdMap_.count(pidValue), 0u);
}

/**
 * @tc.number: AppPidFdManagerTest_OnPidfdFired_003
 * @tc.desc: OnPidfdFired with pid in map, pidfd=-1, but no Init should erase entry and skip DispatchCleanup.
 * @tc.type: FUNC
 * @tc.Function: OnPidfdFired
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPidFdManagerTest, AppPidFdManagerTest_OnPidfdFired_003, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPidFdManagerTest_OnPidfdFired_003 start.");
    int32_t pidValue = 4321;
    {
        std::lock_guard<std::mutex> lock(GetManager().mapMutex_);
        GetManager().pidfdMap_[pidValue] = AppPidFdManager::PidFdEntry{-1, pidValue};
    }
    GetManager().appMgrServiceInner_.reset();
    GetManager().taskHandler_.reset();
    GetManager().OnPidfdFired(pidValue, PidFdType::CHILD);
    std::lock_guard<std::mutex> lock(GetManager().mapMutex_);
    EXPECT_EQ(GetManager().pidfdMap_.count(pidValue), 0u);
}

/**
 * @tc.number: AppPidFdManagerTest_DispatchCleanup_001
 * @tc.desc: DispatchCleanup with no Init (null inner and handler) should return early.
 * @tc.type: FUNC
 * @tc.Function: DispatchCleanup
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPidFdManagerTest, AppPidFdManagerTest_DispatchCleanup_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPidFdManagerTest_DispatchCleanup_001 start.");
    GetManager().appMgrServiceInner_.reset();
    GetManager().taskHandler_.reset();
    GetManager().DispatchCleanup(3333, PidFdType::CHILD);
    std::lock_guard<std::mutex> lock(GetManager().mapMutex_);
    EXPECT_TRUE(GetManager().pidfdMap_.empty());
}

/**
 * @tc.number: AppPidFdManagerTest_DispatchCleanup_002
 * @tc.desc: DispatchCleanup with expired inner (handler still valid) should return early.
 * @tc.type: FUNC
 * @tc.Function: DispatchCleanup
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPidFdManagerTest, AppPidFdManagerTest_DispatchCleanup_002, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPidFdManagerTest_DispatchCleanup_002 start.");
    EXPECT_CALL(*handler_, SubmitTaskInner(_, _)).Times(0);
    inner_.reset();
    GetManager().appMgrServiceInner_.reset();
    GetManager().DispatchCleanup(3333, PidFdType::RENDER);
    std::lock_guard<std::mutex> lock(GetManager().mapMutex_);
    EXPECT_TRUE(GetManager().pidfdMap_.empty());
}

/**
 * @tc.number: AppPidFdManagerTest_DispatchCleanup_003
 * @tc.desc: DispatchCleanup with expired handler (inner still valid) should return early.
 * @tc.type: FUNC
 * @tc.Function: DispatchCleanup
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPidFdManagerTest, AppPidFdManagerTest_DispatchCleanup_003, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPidFdManagerTest_DispatchCleanup_003 start.");
    GetManager().taskHandler_.reset();
    GetManager().DispatchCleanup(3333, PidFdType::CHILD);
    std::lock_guard<std::mutex> lock(GetManager().mapMutex_);
    EXPECT_TRUE(GetManager().pidfdMap_.empty());
}

/**
 * @tc.number: AppPidFdManagerTest_DispatchCleanup_004
 * @tc.desc: DispatchCleanup with valid inner and handler, RENDER type should submit a task.
 * @tc.type: FUNC
 * @tc.Function: DispatchCleanup
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPidFdManagerTest, AppPidFdManagerTest_DispatchCleanup_004, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPidFdManagerTest_DispatchCleanup_004 start.");
    EXPECT_CALL(*handler_, SubmitTaskInner(_, _)).Times(1);
    GetManager().DispatchCleanup(3333, PidFdType::RENDER);
}

/**
 * @tc.number: AppPidFdManagerTest_DispatchCleanup_005
 * @tc.desc: DispatchCleanup with valid inner and handler, CHILD type should submit a task.
 * @tc.type: FUNC
 * @tc.Function: DispatchCleanup
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPidFdManagerTest, AppPidFdManagerTest_DispatchCleanup_005, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPidFdManagerTest_DispatchCleanup_005 start.");
    EXPECT_CALL(*handler_, SubmitTaskInner(_, _)).Times(1);
    GetManager().DispatchCleanup(3333, PidFdType::CHILD);
}

/**
 * @tc.number: AppPidFdManagerTest_Destructor_001
 * @tc.desc: Destructor with entries having pidfd=-1 should clear the map without calling close or epoll_del.
 * @tc.type: FUNC
 * @tc.Function: ~AppPidFdManager
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPidFdManagerTest, AppPidFdManagerTest_Destructor_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPidFdManagerTest_Destructor_001 start.");
    AppPidFdManager mgr;
    mgr.pidfdMap_[1111] = AppPidFdManager::PidFdEntry{-1, 1111};
    mgr.pidfdMap_[2222] = AppPidFdManager::PidFdEntry{-1, 2222};
    EXPECT_EQ(mgr.pidfdMap_.size(), 2u);
}

/**
 * @tc.number: AppPidFdManagerTest_Destructor_002
 * @tc.desc: Destructor with entries having valid fds should close them and call ffrt_epoll_ctl DEL.
 *          Uses /dev/null fds which are safe to close and were never registered with epoll.
 * @tc.type: FUNC
 * @tc.Function: ~AppPidFdManager
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPidFdManagerTest, AppPidFdManagerTest_Destructor_002, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPidFdManagerTest_Destructor_002 start.");
    int fd1 = open("/dev/null", O_RDONLY);
    ASSERT_GE(fd1, 0);
    int fd2 = open("/dev/null", O_RDONLY);
    ASSERT_GE(fd2, 0);
    {
        AppPidFdManager mgr;
        mgr.pidfdMap_[1111] = AppPidFdManager::PidFdEntry{fd1, 1111};
        mgr.pidfdMap_[2222] = AppPidFdManager::PidFdEntry{fd2, 2222};
        EXPECT_EQ(mgr.pidfdMap_.size(), 2u);
    }
    close(fd1);
    close(fd2);
}

/**
 * @tc.number: AppPidFdManagerTest_OnPidfdFired_004
 * @tc.desc: OnPidfdFired with pid in map and valid fd should erase entry, call ffrt_epoll_ctl DEL,
 *          close the fd, and dispatch cleanup. Uses /dev/null fd which is safe to close.
 * @tc.type: FUNC
 * @tc.Function: OnPidfdFired
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPidFdManagerTest, AppPidFdManagerTest_OnPidfdFired_004, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPidFdManagerTest_OnPidfdFired_004 start.");
    int fd = open("/dev/null", O_RDONLY);
    ASSERT_GE(fd, 0);
    int32_t pidValue = 3333;
    {
        std::lock_guard<std::mutex> lock(GetManager().mapMutex_);
        GetManager().pidfdMap_[pidValue] = AppPidFdManager::PidFdEntry{fd, pidValue};
    }
    EXPECT_CALL(*handler_, SubmitTaskInner(_, _)).Times(1);
    GetManager().OnPidfdFired(pidValue, PidFdType::CHILD);
    std::lock_guard<std::mutex> lock(GetManager().mapMutex_);
    EXPECT_EQ(GetManager().pidfdMap_.count(pidValue), 0u);
    close(fd);
}
}  // namespace AppExecFwk
}  // namespace OHOS
