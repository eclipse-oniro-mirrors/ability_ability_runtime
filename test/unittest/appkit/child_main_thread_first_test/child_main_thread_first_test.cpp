/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#define private public
#include "child_main_thread.h"
#include "sys_mgr_client.h"
#undef private
#include "app_log_wrapper.h"
#include "appexecfwk_errors.h"
#include "child_process_info.h"
#include "event_handler.h"
#include "event_runner.h"
#include "hilog_tag_wrapper.h"
#include "mock_ability_token.h"
#include "mock_app_mgr_service.h"
#include "mock_bundle_manager.h"
#include "system_ability_definition.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class MockChildMainThread : public ChildMainThread {
public:
    MockChildMainThread() {}
    virtual ~MockChildMainThread() {}
    MOCK_METHOD1(GetChildProcessInfo, int32_t(ChildProcessInfo &info));
};
class ChildMainThreadFirstTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<ChildMainThread> thread_;
};

void ChildMainThreadFirstTest::SetUpTestCase()
{
}

void ChildMainThreadFirstTest::TearDownTestCase()
{}

void ChildMainThreadFirstTest::SetUp()
{
    thread_ = sptr<ChildMainThread>(new (std::nothrow) ChildMainThread());
}

void ChildMainThreadFirstTest::TearDown()
{
    sptr<IRemoteObject> bundleMgrService = sptr<IRemoteObject>(new (std::nothrow) BundleMgrService());
    sptr<IRemoteObject> mockAppMgrService = sptr<IRemoteObject>(new (std::nothrow) MockAppMgrService());
    auto sysMgr = DelayedSingleton<SysMrgClient>::GetInstance();
    if (sysMgr == nullptr) {
        GTEST_LOG_(ERROR) << "Failed to get ISystemAbilityManager.";
        return;
    }

    sysMgr->RegisterSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, bundleMgrService);
    sysMgr->RegisterSystemAbility(APP_MGR_SERVICE_ID, mockAppMgrService);
}

/**
 * @tc.number: ScheduleLoadChild_0100
 * @tc.desc: Test ScheduleLoadChild works
 * @tc.type: FUNC
 */
HWTEST_F(ChildMainThreadFirstTest, ScheduleLoadChild_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ScheduleLoadChild_0100 start.");
    ASSERT_NE(thread_, nullptr);

    std::shared_ptr<EventRunner> runner = EventRunner::GetMainEventRunner();
    std::shared_ptr<EventHandler> handler = std::make_shared<EventHandler>(runner);
    thread_->mainHandler_ = handler;
    thread_->processInfo_ = std::make_shared<ChildProcessInfo>();

    auto ret = thread_->ScheduleLoadChild();
    EXPECT_TRUE(ret);
    TAG_LOGI(AAFwkTag::TEST, "ScheduleLoadChild_0100 end.");
}

/**
 * @tc.number: ScheduleLoadChild_0200
 * @tc.desc: Test ScheduleLoadChild works
 * @tc.type: FUNC
 */
HWTEST_F(ChildMainThreadFirstTest, ScheduleLoadChild_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ScheduleLoadChild_0200 start.");
    ASSERT_NE(thread_, nullptr);
    auto ret = thread_->ScheduleLoadChild();
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "ScheduleLoadChild_0200 end.");
}

/**
 * @tc.number: ScheduleLoadChild_0300
 * @tc.desc: Test ScheduleLoadChild works
 * @tc.type: FUNC
 */
HWTEST_F(ChildMainThreadFirstTest, ScheduleLoadChild_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ScheduleLoadChild_0300 start.");
    ASSERT_NE(thread_, nullptr);
    std::shared_ptr<EventRunner> runner = EventRunner::GetMainEventRunner();
    std::shared_ptr<EventHandler> handler = std::make_shared<EventHandler>(runner);
    thread_->mainHandler_ = handler;
    auto ret = thread_->ScheduleLoadChild();
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "ScheduleLoadChild_0300 end.");
}

/**
 * @tc.number: GetChildProcessInfo_0100
 * @tc.desc: Test GetChildProcessInfo works
 * @tc.type: FUNC
 */
HWTEST_F(ChildMainThreadFirstTest, GetChildProcessInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetChildProcessInfo_0100 start.");
    ASSERT_NE(thread_, nullptr);
    ChildProcessInfo info;
    auto ret = thread_->GetChildProcessInfo(info);
    EXPECT_EQ(ret, ERR_NAME_NOT_FOUND);
    TAG_LOGI(AAFwkTag::TEST, "GetChildProcessInfo_0100 end.");
}

/**
 * @tc.number: HandleLoadNative_0100
 * @tc.desc: Test HandleLoadNative works
 * @tc.type: FUNC
 */
HWTEST_F(ChildMainThreadFirstTest, HandleLoadNative_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleLoadNative_0100 start.");
    ChildMainThread childMainThread;
    childMainThread.processInfo_ = nullptr;
    childMainThread.HandleLoadNative();
    EXPECT_NE(childMainThread.processArgs_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "HandleLoadNative_0100 end.");
}

/**
 * @tc.number: HandleLoadNative_0200
 * @tc.desc: Test HandleLoadNative works
 * @tc.type: FUNC
 */
HWTEST_F(ChildMainThreadFirstTest, HandleLoadNative_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleLoadNative_0200 start.");
    ChildMainThread childMainThread;
    childMainThread.processInfo_ = std::make_shared<ChildProcessInfo>();
    childMainThread.processArgs_ = nullptr;
    childMainThread.HandleLoadNative();
    EXPECT_NE(childMainThread.processInfo_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "HandleLoadNative_0200 end.");
}

/**
 * @tc.number: HandleLoadNative_0300
 * @tc.desc: Test HandleLoadNative works with valid process info and args
 * @tc.type: FUNC
 */
HWTEST_F(ChildMainThreadFirstTest, HandleLoadNative_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleLoadNative_0300 start.");
    ChildMainThread childMainThread;
    childMainThread.processInfo_ = std::make_shared<ChildProcessInfo>();
    childMainThread.processInfo_->srcEntry = "libchildproc.so";
    childMainThread.processInfo_->entryFunc = "ChildProcessMain";
    childMainThread.processInfo_->bundleName = "com.ohos.test";
    childMainThread.processArgs_ = std::make_shared<ChildProcessArgs>();
    childMainThread.appMgr_ = sptr<MockAppMgrService>(new (std::nothrow) MockAppMgrService());

    childMainThread.HandleLoadNative();

    EXPECT_NE(childMainThread.processInfo_, nullptr);
    EXPECT_NE(childMainThread.processArgs_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "HandleLoadNative_0300 end.");
}

/**
 * @tc.number: HandleRunNativeProc_0100
 * @tc.desc: Test HandleLoadNative works
 * @tc.type: FUNC
 */
HWTEST_F(ChildMainThreadFirstTest, HandleRunNativeProc_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleRunNativeProc_0100 start.");
    ChildMainThread childMainThread;
    sptr<IRemoteObject> mainProcessCb = nullptr;
    childMainThread.processInfo_ = nullptr;
    childMainThread.HandleRunNativeProc(mainProcessCb);
    childMainThread.processInfo_ = std::make_shared<ChildProcessInfo>();
    EXPECT_NE(childMainThread.processInfo_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "HandleRunNativeProc_0100 end.");
}

/**
 * @tc.number: HandleRunNativeProc_0200
 * @tc.desc: Test HandleRunNativeProc works with valid process info
 * @tc.type: FUNC
 */
HWTEST_F(ChildMainThreadFirstTest, HandleRunNativeProc_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleRunNativeProc_0200 start.");
    ChildMainThread childMainThread;
    childMainThread.processInfo_ = std::make_shared<ChildProcessInfo>();
    childMainThread.processInfo_->srcEntry = "libchildproc.so";
    childMainThread.processInfo_->bundleName = "com.ohos.test";
    childMainThread.appMgr_ = sptr<MockAppMgrService>(new (std::nothrow) MockAppMgrService());
    sptr<IRemoteObject> mainProcessCb = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());

    childMainThread.HandleRunNativeProc(mainProcessCb);

    EXPECT_NE(childMainThread.processInfo_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "HandleRunNativeProc_0200 end.");
}

/**
 * @tc.number: OnLoadAbilityFinished_0100
 * @tc.desc: Test HandleLoadNative works
 * @tc.type: FUNC
 */
HWTEST_F(ChildMainThreadFirstTest, OnLoadAbilityFinished_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnLoadAbilityFinished_0100 start.");
    ChildMainThread childMainThread;
    childMainThread.OnLoadAbilityFinished(1234, 1234);
    EXPECT_EQ(childMainThread.processInfo_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "OnLoadAbilityFinished_0100 end.");
}
} // namespace AppExecFwk
} // namespace OHOS
