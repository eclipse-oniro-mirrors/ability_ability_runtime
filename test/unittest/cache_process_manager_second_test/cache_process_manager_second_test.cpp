/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "cache_process_manager.h"
#include "app_mgr_service_inner.h"
#include "app_running_record.h"
#include "mock_my_status.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class CacheProcessManagerSecondTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void CacheProcessManagerSecondTest::SetUpTestCase(void)
{}

void CacheProcessManagerSecondTest::TearDownTestCase(void)
{}

void CacheProcessManagerSecondTest::SetUp()
{}

void CacheProcessManagerSecondTest::TearDown()
{}

/**
 * @tc.name: CacheProcessManager_CheckAndSetProcessCacheEnable_0100
 * @tc.desc: Test the state of CheckAndSetProcessCacheEnable
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerSecondTest, CheckAndSetProcessCacheEnable_0100, TestSize.Level1)
{
    // branch 1, return directly invoke nothing
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    appRecord->procCacheSupportState_ = SupportProcessCacheState::SUPPORT;
    appRecord->priorityObject_ = std::make_shared<PriorityObject>();
    appRecord->processCacheBlocked_.store(false);
    AAFwk::MyStatus::GetInstance().isShouldKillProcess_ = true;
    cacheProcMgr->CheckAndSetProcessCacheEnable(nullptr);
    EXPECT_EQ(appRecord->GetProcessCacheBlocked(), false);
}

/**
 * @tc.name: CacheProcessManager_CheckAndSetProcessCacheEnable_0200
 * @tc.desc: Test the state of CheckAndSetProcessCacheEnable
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerSecondTest, CheckAndSetProcessCacheEnable_0200, TestSize.Level1)
{
    // branch 2, invoke GetSupportProcessCacheState then return
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    cacheProcMgr->warmStartProcesEnable_ = true;
    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    appRecord->procCacheSupportState_ = SupportProcessCacheState::NOT_SUPPORT;
    appRecord->priorityObject_ = std::make_shared<PriorityObject>();
    appRecord->processCacheBlocked_.store(false);
    AAFwk::MyStatus::GetInstance().isShouldKillProcess_ = true;
    cacheProcMgr->CheckAndSetProcessCacheEnable(appRecord);
    EXPECT_EQ(appRecord->GetProcessCacheBlocked(), false);
}

/**
 * @tc.name: CacheProcessManager_CheckAndSetProcessCacheEnable_0300
 * @tc.desc: Test the state of CheckAndSetProcessCacheEnable
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerSecondTest, CheckAndSetProcessCacheEnable_0300, TestSize.Level1)
{
    // branch 3, invoke GetPriorityObject then return
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    cacheProcMgr->warmStartProcesEnable_ = true;
    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    appRecord->procCacheSupportState_ = SupportProcessCacheState::SUPPORT;
    appRecord->priorityObject_ = nullptr;
    appRecord->processCacheBlocked_.store(false);
    AAFwk::MyStatus::GetInstance().isShouldKillProcess_ = true;
    cacheProcMgr->CheckAndSetProcessCacheEnable(appRecord);
    EXPECT_EQ(appRecord->GetProcessCacheBlocked(), false);
}

/**
 * @tc.name: CacheProcessManager_CheckAndSetProcessCacheEnable_0400
 * @tc.desc: Test the state of CheckAndSetProcessCacheEnable
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerSecondTest, CheckAndSetProcessCacheEnable_0400, TestSize.Level1)
{
    // branch 4, invoke GetProcessCacheBlocked then return
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    cacheProcMgr->warmStartProcesEnable_ = true;
    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    appRecord->procCacheSupportState_ = SupportProcessCacheState::SUPPORT;
    appRecord->priorityObject_ = std::make_shared<PriorityObject>();
    appRecord->processCacheBlocked_.store(true);
    AAFwk::MyStatus::GetInstance().setProcessCacheBlockedTimes_ = 0;
    AAFwk::MyStatus::GetInstance().isShouldKillProcess_ = true;
    cacheProcMgr->CheckAndSetProcessCacheEnable(appRecord);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().setProcessCacheBlockedTimes_, 0);
}

/**
 * @tc.name: CacheProcessManager_CheckAndSetProcessCacheEnable_0500
 * @tc.desc: Test the state of CheckAndSetProcessCacheEnable
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerSecondTest, CheckAndSetProcessCacheEnable_0500, TestSize.Level1)
{
    // branch 5, invoke SetProcessCacheBlocked then return
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    cacheProcMgr->warmStartProcesEnable_ = true;
    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    appRecord->procCacheSupportState_ = SupportProcessCacheState::SUPPORT;
    appRecord->priorityObject_ = std::make_shared<PriorityObject>();
    appRecord->processCacheBlocked_.store(false);
    AAFwk::MyStatus::GetInstance().isShouldKillProcess_ = true;
    cacheProcMgr->CheckAndSetProcessCacheEnable(appRecord);
    EXPECT_EQ(appRecord->GetProcessCacheBlocked(), true);
}

/**
 * @tc.name: CacheProcessManager_IsAppSupportProcessCache_0100
 * @tc.desc: Test the state of IsAppSupportProcessCache
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerSecondTest, IsAppSupportProcessCache_0100, TestSize.Level1)
{
    // branch 1, invoke nothing then return
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    EXPECT_EQ(cacheProcMgr->IsAppSupportProcessCache(nullptr), false);
}

/**
 * @tc.name: CacheProcessManager_IsAppSupportProcessCache_0200
 * @tc.desc: Test the state of IsAppSupportProcessCache
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerSecondTest, IsAppSupportProcessCache_0200, TestSize.Level1)
{
    // branch 2, invoke IsAttachedToStatusBar then return
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    appRecord->isAttachedToStatusBar = true;
    EXPECT_EQ(cacheProcMgr->IsAppSupportProcessCache(appRecord), false);
}

/**
 * @tc.name: CacheProcessManager_IsAppSupportProcessCache_0300
 * @tc.desc: Test the state of IsAppSupportProcessCache
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerSecondTest, IsAppSupportProcessCache_0300, TestSize.Level1)
{
    // branch 3, invoke IsKeepAliveApp then return
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    appRecord->isAttachedToStatusBar = false;
    AAFwk::MyStatus::GetInstance().isKeepAliveApp_= true;
    EXPECT_EQ(cacheProcMgr->IsAppSupportProcessCache(appRecord), false);
}

/**
 * @tc.name: CacheProcessManager_IsAppSupportProcessCache_0400
 * @tc.desc: Test the state of IsAppSupportProcessCache
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerSecondTest, IsAppSupportProcessCache_0400, TestSize.Level1)
{
    // branch 4, invoke GetParentAppRecord then return
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    appRecord->isAttachedToStatusBar = false;
    AAFwk::MyStatus::GetInstance().isKeepAliveApp_= false;
    auto parentRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    appRecord->SetParentAppRecord(parentRecord);
    EXPECT_EQ(cacheProcMgr->IsAppSupportProcessCache(appRecord), false);
}

/**
 * @tc.name: CacheProcessManager_IsAppSupportProcessCache_0500
 * @tc.desc: Test the state of IsAppSupportProcessCache
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerSecondTest, IsAppSupportProcessCache_0500, TestSize.Level1)
{
    // branch 5, maxProcCacheNum_ > 0 then return
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    appRecord->isAttachedToStatusBar = false;
    AAFwk::MyStatus::GetInstance().isKeepAliveApp_= false;
    auto parentRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    appRecord->SetParentAppRecord(parentRecord);
    appRecord->parentAppRecord_.reset();
    cacheProcMgr->maxProcCacheNum_ = 1;
    EXPECT_EQ(cacheProcMgr->IsAppSupportProcessCache(appRecord), false);
}

/**
 * @tc.name: CacheProcessManager_IsAppShouldCache_0100
 * @tc.desc: Test the state of IsAppShouldCache
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerSecondTest, IsAppShouldCache_0100, TestSize.Level1)
{
    // branch 2, invoke QueryEnableProcessCache then true
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");

    cacheProcMgr->maxProcCacheNum_ = 0;
    cacheProcMgr->warmStartProcesEnable_ = false;
    EXPECT_EQ(cacheProcMgr->IsAppShouldCache(appRecord), false);
}

/**
 * @tc.name: CacheProcessManager_KillProcessByRecord_0100
 * @tc.desc: Test KillProcessByRecord with nullptr app record
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerSecondTest, KillProcessByRecord_0100, TestSize.Level1)
{
    // Test with nullptr app record
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    EXPECT_EQ(cacheProcMgr->KillProcessByRecord(nullptr), false);
}

/**
 * @tc.name: CacheProcessManager_KillProcessByRecord_0200
 * @tc.desc: Test the state of KillProcessByRecord
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerSecondTest, KillProcessByRecord_0200, TestSize.Level1)
{
    // Test with nullptr app record
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    auto appInfo = std::make_shared<ApplicationInfo>();
    appRecord->appInfo_ = appInfo;
    EXPECT_EQ(cacheProcMgr->KillProcessByRecord(appRecord), false);
}

/**
 * @tc.name: CacheProcessManager_KillProcessByRecord_300
 * @tc.desc: Test the state of KillProcessByRecord
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerSecondTest, KillProcessByRecord_300, TestSize.Level1)
{
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    auto appInfo = std::make_shared<ApplicationInfo>();
    auto appMgr = std::make_shared<AppMgrServiceInner>();
    cacheProcMgr->appMgr_ = appMgr;
    appRecord->appInfo_ = appInfo;
    EXPECT_EQ(cacheProcMgr->KillProcessByRecord(appRecord), true);
}

/**
 * @tc.name: CacheProcessManager_IsAppContainsSrvExt_0100
 * @tc.desc: Test IsAppContainsSrvExt with nullptr app record
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerSecondTest, IsAppContainsSrvExt_0100, TestSize.Level1)
{
    // Test with nullptr app record
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    EXPECT_EQ(cacheProcMgr->IsAppContainsSrvExt(nullptr), false);
}

/**
 * @tc.name: CacheProcessManager_IsAppContainsSrvExt_0200
 * @tc.desc: Test IsAppContainsSrvExt when record is already in checked flag
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerSecondTest, IsAppContainsSrvExt_0200, TestSize.Level1)
{
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    cacheProcMgr->srvExtCheckedFlag.insert(appRecord);
    cacheProcMgr->srvExtRecords.insert(appRecord);
    EXPECT_EQ(cacheProcMgr->IsAppContainsSrvExt(appRecord), true);
}

/**
 * @tc.name: CacheProcessManager_IsAppContainsSrvExt_0300
 * @tc.desc: Test IsAppContainsSrvExt when record is already in checked flag but not in service records
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerSecondTest, IsAppContainsSrvExt_0300, TestSize.Level1)
{
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    cacheProcMgr->srvExtCheckedFlag.insert(appRecord);
    EXPECT_EQ(cacheProcMgr->IsAppContainsSrvExt(appRecord), false);
}

} // namespace AppExecFwk
} // namespace OHOS