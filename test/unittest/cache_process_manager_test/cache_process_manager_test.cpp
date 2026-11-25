/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
#include "app_utils.h"
#undef private
#undef protected
#include "mock_app_mgr_service_inner.h"
#include "mock_ability_token.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {

const std::string ABILITY_RECORD_NAME = "Ability_Name_Z";
const std::string DEFAULT_BUNDLE_NAME = "com.tdd.cacheprocessmanager";
const int32_t DEFAULT_UID = 101010;
constexpr int32_t DEFAULT_API_LEVEL = 12;

class CacheProcessManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    std::shared_ptr<AppRunningRecord> MockAppRecord(int apiLevel = DEFAULT_API_LEVEL,
        const std::string &bundleName = DEFAULT_BUNDLE_NAME);
    int recordId = 0;
};


void CacheProcessManagerTest::SetUpTestCase(void)
{}

void CacheProcessManagerTest::TearDownTestCase(void)
{}

void CacheProcessManagerTest::SetUp()
{}

void CacheProcessManagerTest::TearDown()
{}

std::shared_ptr<AppRunningRecord> CacheProcessManagerTest::MockAppRecord(int apiLevel, const std::string &bundleName)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    appInfo->bundleName = bundleName;
    appInfo->uid = DEFAULT_UID;
    appInfo->accessTokenId = 1;
    appInfo->apiTargetVersion = apiLevel;
    std::shared_ptr<AppRunningRecord> appRecord = std::make_shared<AppRunningRecord>(appInfo, recordId++, "process");
    std::shared_ptr<PriorityObject> priorityObject = std::make_shared<PriorityObject>();
    priorityObject->SetPid(1);
    appRecord->priorityObject_ = priorityObject;
    appRecord->SetUid(DEFAULT_UID);
    appRecord->SetState(ApplicationState::APP_STATE_CREATE);
    appRecord->SetContinuousTaskAppState(false);
    appRecord->SetKeepAliveEnableState(false);
    appRecord->SetKeepAliveDkv(false);
    appRecord->SetEmptyKeepAliveAppState(false);
    appRecord->SetRequestProcCode(1);
    appRecord->isFocused_ = false;
    return appRecord;
}
/**
 * @tc.name: CacheProcessManager_QueryEnableProcessCache_0100
 * @tc.desc: Test the state of QueryEnableProcessCache
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerTest, CacheProcessManager_QueryEnableProcessCache_0100, TestSize.Level1)
{
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    EXPECT_NE(cacheProcMgr, nullptr);
    cacheProcMgr->maxProcCacheNum_ = 0;
    cacheProcMgr->warmStartProcesEnable_  = false;
    EXPECT_EQ(cacheProcMgr->QueryEnableProcessCache(), false);
}

/**
 * @tc.name: CacheProcessManager_QueryEnableProcessCache_0200
 * @tc.desc: Test the state of QueryEnableProcessCache
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerTest, CacheProcessManager_QueryEnableProcessCache_0200, TestSize.Level1)
{
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    EXPECT_NE(cacheProcMgr, nullptr);
    cacheProcMgr->maxProcCacheNum_ = 100;
    cacheProcMgr->warmStartProcesEnable_  = false;
    EXPECT_EQ(cacheProcMgr->QueryEnableProcessCache(), true);
}

/**
 * @tc.name: CacheProcessManager_SetAppMgr_0100
 * @tc.desc: Test the state of SetAppMgr
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerTest, CacheProcessManager_SetAppMgr_0100, TestSize.Level1)
{
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    auto appMgrInner = std::make_shared<MockAppMgrServiceInner>();
    EXPECT_NE(cacheProcMgr, nullptr);
    cacheProcMgr->SetAppMgr(appMgrInner);
    EXPECT_NE(appMgrInner, nullptr);
}

/**
 * @tc.name: CacheProcessManager_PenddingCacheProcess_0100
 * @tc.desc: Test the state of PenddingCacheProcess
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerTest, CacheProcessManager_PenddingCacheProcess_0100, TestSize.Level1)
{
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    EXPECT_NE(cacheProcMgr, nullptr);
    cacheProcMgr->maxProcCacheNum_ = 2;
    // keepalive not allowed
    auto appRecord = MockAppRecord();
    EXPECT_NE(appRecord, nullptr);
    appRecord->SetKeepAliveEnableState(true);
    appRecord->SetKeepAliveDkv(true);
    appRecord->SetSingleton(true);
    appRecord->SetEmptyKeepAliveAppState(true);
    appRecord->SetKeepAliveBundle(true);
    appRecord->SetMainProcess(true);
    EXPECT_EQ(cacheProcMgr->PenddingCacheProcess(appRecord), false);
    // nullptr not allowed
    std::shared_ptr<AppRunningRecord> appRecord2 = nullptr;
    EXPECT_EQ(cacheProcMgr->PenddingCacheProcess(appRecord2), false);
    // pending without real shrink
    auto appRecord3 = MockAppRecord();
    EXPECT_NE(appRecord3, nullptr);
    EXPECT_EQ(cacheProcMgr->PenddingCacheProcess(appRecord3), true);
    // pending without real shrink
    auto appRecord4 = MockAppRecord();
    EXPECT_NE(appRecord4, nullptr);
    EXPECT_EQ(cacheProcMgr->PenddingCacheProcess(appRecord4), true);
    // pending with shrinking
    auto appRecord5 = MockAppRecord();
    EXPECT_NE(appRecord5, nullptr);
    EXPECT_EQ(cacheProcMgr->PenddingCacheProcess(appRecord5), true);
}

/**
 * @tc.name: CacheProcessManager_CheckAndCacheProcess_0100
 * @tc.desc: Test the state of CheckAndCacheProcess
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerTest, CacheProcessManager_CheckAndCacheProcess_0100, TestSize.Level1)
{
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    EXPECT_NE(cacheProcMgr, nullptr);
    cacheProcMgr->maxProcCacheNum_ = 2;
    // nullptr not allowed
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    EXPECT_EQ(cacheProcMgr->CheckAndCacheProcess(appRecord), false);
    // not cached
    auto appRecord2 = MockAppRecord();
    EXPECT_NE(appRecord2, nullptr);
    EXPECT_EQ(cacheProcMgr->CheckAndCacheProcess(appRecord2), false);
    // cached but no appMgrSerInner
    EXPECT_EQ(cacheProcMgr->PenddingCacheProcess(appRecord2), true);
    EXPECT_EQ(cacheProcMgr->CheckAndCacheProcess(appRecord2), true);
    // all normal
    auto appMgrInner = std::make_shared<MockAppMgrServiceInner>();
    cacheProcMgr->SetAppMgr(appMgrInner);
    EXPECT_EQ(cacheProcMgr->CheckAndCacheProcess(appRecord2), true);
}

/**
 * @tc.name: CacheProcessManager_IsCachedProcess_0100
 * @tc.desc: Test the state of IsCachedProcess
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerTest, CacheProcessManager_IsCachedProcess_0100, TestSize.Level1)
{
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    EXPECT_NE(cacheProcMgr, nullptr);
    cacheProcMgr->maxProcCacheNum_ = 2;
    // nullptr not allowed
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    EXPECT_EQ(cacheProcMgr->IsCachedProcess(appRecord), false);
    // all normal
    auto appRecord2 = MockAppRecord();
    EXPECT_EQ(cacheProcMgr->PenddingCacheProcess(appRecord2), true);
    EXPECT_EQ(cacheProcMgr->IsCachedProcess(appRecord2), true);
}

/**
 * @tc.name: CacheProcessManager_OnProcessKilled_0100
 * @tc.desc: Test the state of OnProcessKilled
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerTest, CacheProcessManager_OnProcessKilled_0100, TestSize.Level1)
{
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    EXPECT_NE(cacheProcMgr, nullptr);
    cacheProcMgr->maxProcCacheNum_ = 2;
    // nullptr not allowed
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    cacheProcMgr->OnProcessKilled(appRecord);
    // not cached
    auto appRecord2 = MockAppRecord();
    cacheProcMgr->OnProcessKilled(appRecord2);
    // cached, but appMgr is nullptr
    EXPECT_EQ(cacheProcMgr->PenddingCacheProcess(appRecord2), true);
    cacheProcMgr->OnProcessKilled(appRecord2);
    // all normal
    auto appMgrInner = std::make_shared<MockAppMgrServiceInner>();
    cacheProcMgr->SetAppMgr(appMgrInner);
    EXPECT_EQ(cacheProcMgr->PenddingCacheProcess(appRecord2), true);
    cacheProcMgr->OnProcessKilled(appRecord2);
}

/**
 * @tc.name: CacheProcessManager_ReuseCachedProcess_0100
 * @tc.desc: Test the state of ReuseCachedProcess
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerTest, CacheProcessManager_ReuseCachedProcess_0100, TestSize.Level1)
{
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    EXPECT_NE(cacheProcMgr, nullptr);
    cacheProcMgr->maxProcCacheNum_ = 2;
    // nullptr not allowed
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    cacheProcMgr->ReuseCachedProcess(appRecord);
    // not cached
    auto appRecord2 = MockAppRecord();
    cacheProcMgr->ReuseCachedProcess(appRecord2);
    // no appMgr
    EXPECT_EQ(cacheProcMgr->PenddingCacheProcess(appRecord2), true);
    cacheProcMgr->ReuseCachedProcess(appRecord2);
    // all normal
    auto appMgrInner = std::make_shared<MockAppMgrServiceInner>();
    cacheProcMgr->SetAppMgr(appMgrInner);
    EXPECT_EQ(cacheProcMgr->PenddingCacheProcess(appRecord2), true);
    cacheProcMgr->OnProcessKilled(appRecord2);
}

/**
 * @tc.name: CacheProcessManager_IsAppSupportProcessCache_0100
 * @tc.desc: Test the state of IsAppSupportProcessCache
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerTest, CacheProcessManager_IsAppSupportProcessCache_0100, TestSize.Level1)
{
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    EXPECT_NE(cacheProcMgr, nullptr);
    cacheProcMgr->maxProcCacheNum_ = 2;
    // nullptr not allowed
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    EXPECT_EQ(cacheProcMgr->IsAppSupportProcessCache(appRecord), false);
    // API earlier than 12 not allowed
    auto appRecord2 = MockAppRecord(11);
    EXPECT_NE(appRecord2, nullptr);
    EXPECT_EQ(cacheProcMgr->IsAppSupportProcessCache(appRecord2), false);
    // different supportState
    auto appRecord3 = MockAppRecord(12);
    EXPECT_NE(appRecord3, nullptr);
    EXPECT_EQ(cacheProcMgr->IsAppSupportProcessCache(appRecord3), false);
    appRecord3->SetSupportedProcessCache(true);
    EXPECT_EQ(cacheProcMgr->IsAppSupportProcessCache(appRecord3), false);
    appRecord3->SetUIAbilityLaunched(true);
    EXPECT_EQ(cacheProcMgr->IsAppSupportProcessCache(appRecord3), true);
    appRecord3->procCacheSupportState_ = SupportProcessCacheState::NOT_SUPPORT;
    EXPECT_EQ(cacheProcMgr->IsAppSupportProcessCache(appRecord3), false);
}

/**
 * @tc.name: CacheProcessManager_RefreshCacheNum_0100
 * @tc.desc: Test the state of RefreshCacheNum
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerTest, CacheProcessManager_RefreshCacheNum_0100, TestSize.Level1)
{
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    EXPECT_NE(cacheProcMgr, nullptr);
    cacheProcMgr->maxProcCacheNum_ = 2;
    cacheProcMgr->RefreshCacheNum();
}

/**
 * @tc.name: CacheProcessManager_GetCurrentCachedProcNum_0100
 * @tc.desc: Test the state of GetCurrentCachedProcNum
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerTest, CacheProcessManager_GetCurrentCachedProcNum_0100, TestSize.Level1)
{
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    EXPECT_NE(cacheProcMgr, nullptr);
    cacheProcMgr->maxProcCacheNum_ = 2;
    auto appRecord = MockAppRecord();
    EXPECT_NE(appRecord, nullptr);
    EXPECT_EQ(cacheProcMgr->PenddingCacheProcess(appRecord), true);
    EXPECT_EQ(cacheProcMgr->GetCurrentCachedProcNum(), 1);
    auto appRecord2 = MockAppRecord();
    EXPECT_NE(appRecord2, nullptr);
    EXPECT_EQ(cacheProcMgr->PenddingCacheProcess(appRecord2), true);
    EXPECT_EQ(cacheProcMgr->GetCurrentCachedProcNum(), 2);
}

/**
 * @tc.name: CacheProcessManager_KillProcessByRecord_0100
 * @tc.desc: Test the state of KillProcessByRecord
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerTest, CacheProcessManager_KillProcessByRecord_0100, TestSize.Level1)
{
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    EXPECT_NE(cacheProcMgr, nullptr);
    cacheProcMgr->maxProcCacheNum_ = 2;
    auto appRecord = MockAppRecord();
    EXPECT_NE(appRecord, nullptr);
    EXPECT_EQ(cacheProcMgr->KillProcessByRecord(appRecord), false);
}

/**
 * @tc.name: CacheProcessManager_KillProcessByRecord_0200
 * @tc.desc: Test the state of KillProcessByRecord
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerTest, CacheProcessManager_KillProcessByRecord_0200, TestSize.Level1)
{
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    EXPECT_EQ(cacheProcMgr->KillProcessByRecord(nullptr), false);

    auto appRecord = MockAppRecord();
    EXPECT_NE(appRecord, nullptr);
    auto appMgrInner = std::make_shared<MockAppMgrServiceInner>();
    cacheProcMgr->SetAppMgr(appMgrInner);
    EXPECT_EQ(cacheProcMgr->KillProcessByRecord(appRecord), true);
}

/**
 * @tc.name: CacheProcessManager_IsAppShouldCache_0100
 * @tc.desc: Test the state of IsAppShouldCache
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerTest, CacheProcessManager_IsAppShouldCache_0100, TestSize.Level1)
{
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    EXPECT_NE(cacheProcMgr, nullptr);

    // nullptr check
    EXPECT_EQ(cacheProcMgr->IsAppShouldCache(nullptr), false);

    // Not enable
    cacheProcMgr->maxProcCacheNum_ = 0;
    cacheProcMgr->warmStartProcesEnable_  = false;
    EXPECT_EQ(cacheProcMgr->IsAppShouldCache(nullptr), false);

    // Cached app
    cacheProcMgr->maxProcCacheNum_ = 2;
    auto appRecord = MockAppRecord();
    EXPECT_NE(appRecord, nullptr);
    appRecord->procCacheSupportState_ = SupportProcessCacheState::SUPPORT;
    cacheProcMgr->cachedAppRecordQueue_.push_back(appRecord);
    EXPECT_EQ(cacheProcMgr->IsAppShouldCache(appRecord), true);

    // App not enable
    cacheProcMgr->cachedAppRecordQueue_.clear();
    appRecord->SetEnableProcessCache(false);
    EXPECT_EQ(cacheProcMgr->IsAppShouldCache(appRecord), false);

    // App not support cache
    cacheProcMgr->cachedAppRecordQueue_.clear();
    appRecord->procCacheSupportState_ = SupportProcessCacheState::NOT_SUPPORT;
    EXPECT_EQ(cacheProcMgr->IsAppShouldCache(appRecord), false);
}

/**
 * @tc.name: CacheProcessManager_IsAppAbilitiesEmpty_0100
 * @tc.desc: Test the state of IsAppAbilitiesEmpty
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerTest, CacheProcessManager_IsAppAbilitiesEmpty_0100, TestSize.Level1)
{
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    EXPECT_NE(cacheProcMgr, nullptr);
    cacheProcMgr->maxProcCacheNum_ = 2;

    // Abilities empty
    auto appRecord = MockAppRecord();
    EXPECT_NE(appRecord, nullptr);
    EXPECT_EQ(cacheProcMgr->IsAppAbilitiesEmpty(appRecord), true);
    
    // Not empty
    auto caseAbilityInfo = std::make_shared<AbilityInfo>();
    caseAbilityInfo->name = ABILITY_RECORD_NAME;
    sptr<IRemoteObject> token = new MockAbilityToken();
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "Module";
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->bundleName = "com.ohos.test.helloworld";
    appRecord->AddModule(appInfo, caseAbilityInfo, token, hapModuleInfo, nullptr, 0);
    auto moduleRecord = appRecord->GetModuleRecordByModuleName(appInfo->bundleName,
        hapModuleInfo.moduleName);
    auto caseAbilityRunningRecord = moduleRecord->AddAbility(token, caseAbilityInfo, nullptr, 0);
    EXPECT_TRUE(caseAbilityRunningRecord == nullptr);
    EXPECT_EQ(cacheProcMgr->IsAppAbilitiesEmpty(appRecord), false);
}

/**
 * @tc.name: CacheProcessManager_ShrinkAndKillCache_0100
 * @tc.desc: Test the state of ShrinkAndKillCache
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerTest, CacheProcessManager_ShrinkAndKillCache_0100, TestSize.Level1)
{
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    EXPECT_NE(cacheProcMgr, nullptr);
    cacheProcMgr->maxProcCacheNum_ = 2;

    auto appRecord1 = MockAppRecord();
    EXPECT_NE(appRecord1, nullptr);
    auto appRecord2 = MockAppRecord();
    EXPECT_NE(appRecord2, nullptr);
    auto appRecord3 = MockAppRecord();
    EXPECT_NE(appRecord3, nullptr);

    cacheProcMgr->cachedAppRecordQueue_.push_back(appRecord1);
    cacheProcMgr->cachedAppRecordQueue_.push_back(appRecord2);
    cacheProcMgr->cachedAppRecordQueue_.push_back(appRecord3);

    cacheProcMgr->ShrinkAndKillCache();
}

/**
 * @tc.name: CacheProcessManager_PrintCacheQueue_0100
 * @tc.desc: Test the state of PrintCacheQueue
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerTest, CacheProcessManager_PrintCacheQueue_0100, TestSize.Level1)
{
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    EXPECT_NE(cacheProcMgr, nullptr);
    cacheProcMgr->maxProcCacheNum_ = 2;

    auto appRecord1 = MockAppRecord();
    EXPECT_NE(appRecord1, nullptr);
    auto appRecord2 = MockAppRecord();
    EXPECT_NE(appRecord2, nullptr);

    EXPECT_NE(cacheProcMgr->PrintCacheQueue(), "");
}

/**
 * @tc.name: CacheProcessManager_AddToApplicationSet_0100
 * @tc.desc: Test the state of AddToApplicationSet
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerTest, CacheProcessManager_AddToApplicationSet_0100, TestSize.Level1)
{
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    EXPECT_NE(cacheProcMgr, nullptr);
    cacheProcMgr->maxProcCacheNum_ = 2;

    auto appRecord1 = MockAppRecord();
    EXPECT_NE(appRecord1, nullptr);
    auto appRecord2 = MockAppRecord();
    EXPECT_NE(appRecord2, nullptr);

    cacheProcMgr->AddToApplicationSet(appRecord1);
    cacheProcMgr->AddToApplicationSet(appRecord2);

    EXPECT_TRUE(cacheProcMgr->sameAppSet.find(DEFAULT_BUNDLE_NAME) != cacheProcMgr->sameAppSet.end());
    EXPECT_TRUE(cacheProcMgr->sameAppSet[DEFAULT_BUNDLE_NAME].find(DEFAULT_UID) !=
        cacheProcMgr->sameAppSet[DEFAULT_BUNDLE_NAME].end());
}

/**
 * @tc.name: CacheProcessManager_RemoveFromApplicationSet_0100
 * @tc.desc: Test the state of RemoveFromApplicationSet
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerTest, CacheProcessManager_RemoveFromApplicationSet_0100, TestSize.Level1)
{
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    EXPECT_NE(cacheProcMgr, nullptr);
    cacheProcMgr->maxProcCacheNum_ = 2;

    auto appRecord1 = MockAppRecord();
    EXPECT_NE(appRecord1, nullptr);
    auto appRecord2 = MockAppRecord();
    EXPECT_NE(appRecord2, nullptr);

    cacheProcMgr->AddToApplicationSet(appRecord1);
    cacheProcMgr->RemoveFromApplicationSet(appRecord2);
    EXPECT_TRUE(cacheProcMgr->sameAppSet.find(DEFAULT_BUNDLE_NAME) != cacheProcMgr->sameAppSet.end());

    cacheProcMgr->RemoveFromApplicationSet(appRecord1);
    EXPECT_TRUE(cacheProcMgr->sameAppSet.find(DEFAULT_BUNDLE_NAME) == cacheProcMgr->sameAppSet.end());
}

/**
 * @tc.name: CacheProcessManager_RemoveFromApplicationSet_0100
 * @tc.desc: Test the state of RemoveFromApplicationSet
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerTest, CacheProcessManager_IsAppContainsSrvExt_0100, TestSize.Level1)
{
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    EXPECT_NE(cacheProcMgr, nullptr);
    cacheProcMgr->maxProcCacheNum_ = 2;

    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "test_ability_name1";
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = "test_app_name1";
    std::string processName = "com.ohos.test.helloworld";
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), processName);
    EXPECT_TRUE(appRunningRecord != nullptr);
    sptr<IRemoteObject> token = new MockAbilityToken();
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    abilityInfo->type = AppExecFwk::AbilityType::EXTENSION;
    abilityInfo->extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    hapModuleInfo.abilityInfos.push_back(*abilityInfo);
    appRunningRecord->AddModule(appInfo, abilityInfo, token, hapModuleInfo, nullptr, 0);
    auto moduleRecord = appRunningRecord->GetModuleRecordByModuleName(appInfo->bundleName, hapModuleInfo.moduleName);
    EXPECT_TRUE(moduleRecord != nullptr);
    auto abilityRunningRecord = moduleRecord->GetAbilityRunningRecordByToken(token);
    EXPECT_TRUE(abilityRunningRecord != nullptr);
    
    EXPECT_EQ(cacheProcMgr->IsAppContainsSrvExt(appRunningRecord), false);
}
class CacheProcessManagerOverseaTest : public CacheProcessManagerTest {
public:
    static constexpr int32_t TEST_MAX_PROC_CACHE_NUM = 2;
    static constexpr const char *testAssistantBundleName = "com.example.fusionassistant";

    void SetUp() override
    {
        auto &appUtils = AAFwk::AppUtils::GetInstance();
        originalBrokerDelegateBundleName_ = appUtils.brokerDelegateBundleName_;
        appUtils.brokerDelegateBundleName_ = {true, testAssistantBundleName};
        cacheProcMgr_ = std::make_shared<CacheProcessManager>();
        cacheProcMgr_->maxProcCacheNum_ = TEST_MAX_PROC_CACHE_NUM;
        cacheProcMgr_->warmStartProcesEnable_ = false;
        cacheProcMgr_->shouldCheckApi = true;
        cacheProcMgr_->shouldCheckSupport = true;
    }

    void TearDown() override
    {
        AAFwk::AppUtils::GetInstance().brokerDelegateBundleName_ = originalBrokerDelegateBundleName_;
    }

    std::shared_ptr<AppRunningRecord> MockEligibleRecord(const std::string &bundleName = testAssistantBundleName)
    {
        auto appRecord = MockAppRecord(DEFAULT_API_LEVEL, bundleName);
        appRecord->SetUIAbilityLaunched(true);
        appRecord->SetSupportedProcessCache(true);
        appRecord->SetEnableProcessCache(true);
        return appRecord;
    }

    std::shared_ptr<CacheProcessManager> cacheProcMgr_;
    AAFwk::DeviceConfiguration<std::string> originalBrokerDelegateBundleName_;
};

/**
 * @tc.name: OverseasAssistantSupportsCache
 * @tc.desc: The overseas assistant can use process cache when all normal conditions are satisfied.
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerOverseaTest, OverseasAssistantSupportsCache, TestSize.Level1)
{
    cacheProcMgr_->isOversea_ = true;
    auto appRecord = MockEligibleRecord();
    ASSERT_EQ(appRecord->GetBundleName(), testAssistantBundleName);
    EXPECT_TRUE(cacheProcMgr_->IsAppShouldCache(appRecord));
}

/**
 * @tc.name: NonOverseasAssistantRejectsCache
 * @tc.desc: The configured assistant remains excluded from process cache outside overseas regions.
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerOverseaTest, NonOverseasAssistantRejectsCache, TestSize.Level1)
{
    cacheProcMgr_->isOversea_ = false;
    auto appRecord = MockEligibleRecord();
    EXPECT_FALSE(cacheProcMgr_->IsAppShouldCache(appRecord));
}

/**
 * @tc.name: OtherApplicationsKeepCacheEligibility
 * @tc.desc: The region does not change the cache eligibility of other applications.
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerOverseaTest, OtherApplicationsKeepCacheEligibility, TestSize.Level1)
{
    auto appRecord = MockEligibleRecord(DEFAULT_BUNDLE_NAME);
    cacheProcMgr_->isOversea_ = false;
    EXPECT_TRUE(cacheProcMgr_->IsAppShouldCache(appRecord));
    cacheProcMgr_->isOversea_ = true;
    EXPECT_TRUE(cacheProcMgr_->IsAppShouldCache(appRecord));
}

/**
 * @tc.name: OverseasAssistantRequiresSupportDeclaration
 * @tc.desc: The enabled declaration check rejects an unspecified or unsupported overseas assistant.
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerOverseaTest, OverseasAssistantRequiresSupportDeclaration, TestSize.Level1)
{
    cacheProcMgr_->isOversea_ = true;
    auto appRecord = MockEligibleRecord();
    appRecord->procCacheSupportState_ = SupportProcessCacheState::UNSPECIFIED;
    EXPECT_FALSE(cacheProcMgr_->IsAppShouldCache(appRecord));
    appRecord->SetSupportedProcessCache(false);
    EXPECT_FALSE(cacheProcMgr_->IsAppShouldCache(appRecord));
}

/**
 * @tc.name: OverseasAssistantFollowsCommonSupportPolicy
 * @tc.desc: Disabling the declaration check permits unspecified support for the overseas assistant and other apps.
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerOverseaTest, OverseasAssistantFollowsCommonSupportPolicy, TestSize.Level1)
{
    cacheProcMgr_->isOversea_ = true;
    cacheProcMgr_->shouldCheckSupport = false;
    auto assistant = MockEligibleRecord();
    auto otherApp = MockEligibleRecord(DEFAULT_BUNDLE_NAME);
    assistant->procCacheSupportState_ = SupportProcessCacheState::UNSPECIFIED;
    otherApp->procCacheSupportState_ = SupportProcessCacheState::UNSPECIFIED;
    EXPECT_TRUE(cacheProcMgr_->IsAppShouldCache(assistant));
    EXPECT_TRUE(cacheProcMgr_->IsAppShouldCache(otherApp));
    assistant->SetSupportedProcessCache(false);
    EXPECT_FALSE(cacheProcMgr_->IsAppShouldCache(assistant));
    assistant->SetSupportedProcessCache(true);
    EXPECT_TRUE(cacheProcMgr_->IsAppShouldCache(assistant));
}

/**
 * @tc.name: OverseasAssistantHonorsCacheBlock
 * @tc.desc: Temporary process cache blocking still applies to the overseas assistant.
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerOverseaTest, OverseasAssistantHonorsCacheBlock, TestSize.Level1)
{
    cacheProcMgr_->isOversea_ = true;
    auto appRecord = MockEligibleRecord();
    appRecord->SetProcessCacheBlocked(true);
    EXPECT_FALSE(cacheProcMgr_->IsAppShouldCache(appRecord));
}

/**
 * @tc.name: OverseasAssistantRequiresUIForWarmStart
 * @tc.desc: Warm start cache still requires a launched UI ability for the overseas assistant.
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerOverseaTest, OverseasAssistantRequiresUIForWarmStart, TestSize.Level1)
{
    cacheProcMgr_->isOversea_ = true;
    cacheProcMgr_->maxProcCacheNum_ = 0;
    cacheProcMgr_->warmStartProcesEnable_ = true;
    auto appRecord = MockEligibleRecord();
    appRecord->SetUIAbilityLaunched(false);
    EXPECT_FALSE(cacheProcMgr_->IsAppShouldCache(appRecord));
    appRecord->SetUIAbilityLaunched(true);
    EXPECT_TRUE(cacheProcMgr_->IsAppShouldCache(appRecord));
}

/**
 * @tc.name: OverseasAssistantHonorsCacheEnableState
 * @tc.desc: Overseas eligibility does not force process cache to be enabled.
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerOverseaTest, OverseasAssistantHonorsCacheEnableState, TestSize.Level1)
{
    cacheProcMgr_->isOversea_ = true;
    auto appRecord = MockEligibleRecord();
    appRecord->SetEnableProcessCache(false);
    EXPECT_FALSE(cacheProcMgr_->IsAppShouldCache(appRecord));
    appRecord->SetEnableProcessCache(true);
    cacheProcMgr_->maxProcCacheNum_ = 0;
    EXPECT_FALSE(cacheProcMgr_->IsAppShouldCache(appRecord));
}
} // namespace AppExecFwk
} // namespace OHOS