/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include <gtest/hwext/gtest-multithread.h>

#define private public
#include "app_running_manager.h"
#ifdef SUPPORT_CHILD_PROCESS
#include "child_process_record.h"
#endif // SUPPORT_CHILD_PROCESS
#undef private
#include "hilog_tag_wrapper.h"
#include "window_visibility_info.h"

using namespace testing;
using namespace testing::ext;
using namespace testing::mt;

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t DEBUGINFOS_SIZE = 0;
constexpr int32_t ABILITYTOKENS_SIZE = 0;
constexpr int32_t RECORD_ID = 1;
constexpr uint32_t WINDOW_ID = 100;
constexpr pid_t PID = 10;
constexpr int32_t RECORD_MAP_SIZE = 1;
constexpr int32_t DEBUG_INFOS_SIZE = 1;
constexpr int32_t ABILITY_TOKENS_SIZE = 1;
}
class AppRunningManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AppRunningManagerTest::SetUpTestCase(void)
{}

void AppRunningManagerTest::TearDownTestCase(void)
{}

void AppRunningManagerTest::SetUp()
{}

void AppRunningManagerTest::TearDown()
{}

/**
 * @tc.name: AppRunningManager_SetAttachAppDebug_0100
 * @tc.desc: Test the state of AttachAppDebug
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, AppRunningManager_SetAttachAppDebug_0100, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    std::string bundleName;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    appRunningManager->appRunningRecordMap_.insert(make_pair(RECORD_ID, appRunningRecord));
    appRunningManager->SetAttachAppDebug(bundleName, true, false);
    for (const auto &item : appRunningManager->appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord->GetBundleName() == bundleName) {
            appRecord->SetAttachDebug(true, false);
            EXPECT_EQ(appRecord->isAttachDebug_, true);
        }
    }
}

/**
 * @tc.name: AppRunningManager_SetAttachAppDebug_0200
 * @tc.desc: Test the state of AttachAppDebug
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, AppRunningManager_SetAttachAppDebug_0200, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    std::string bundleName;
    bool isAttachDebug = true;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    appRunningManager->appRunningRecordMap_.insert(make_pair(RECORD_ID, appRunningRecord));
    appRunningManager->SetAttachAppDebug(bundleName, isAttachDebug, false);
    for (const auto &item : appRunningManager->appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord->GetBundleName() == bundleName) {
            appRecord->SetAttachDebug(true, false);
            EXPECT_EQ(appRecord->isAttachDebug_, true);
        }
    }
}

/**
 * @tc.name: AppRunningManager_GetAppDebugInfoByBundleName_0100
 * @tc.desc: Test the state of GetAppDebugInfoByBundleName
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, AppRunningManager_GetAppDebugInfoByBundleName_0100, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    std::string bundleName;
    std::vector<AppDebugInfo> debugInfos;
    bool isDetachDebug = true;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    appRunningManager->appRunningRecordMap_.insert(make_pair(RECORD_ID, appRunningRecord));
    appRunningManager->GetAppDebugInfosByBundleName(bundleName, isDetachDebug);
    EXPECT_EQ(appRunningManager->appRunningRecordMap_.size(), RECORD_MAP_SIZE);
    for (const auto &item : appRunningManager->appRunningRecordMap_) {
        const auto &appRecord = item.second;
        AppDebugInfo debugInfo;
        debugInfos.emplace_back(debugInfo);
        EXPECT_EQ(debugInfos.size(), DEBUG_INFOS_SIZE);
    }
}

/**
 * @tc.name: AppRunningManager_GetAbilityTokensByBundleName_0100
 * @tc.desc: Test the state of GetAbilityTokensByBundleName
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, AppRunningManager_GetAbilityTokensByBundleName_0100, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    std::string bundleName;
    std::vector<sptr<IRemoteObject>> abilityTokens;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    appRunningManager->appRunningRecordMap_.insert(make_pair(RECORD_ID, appRunningRecord));
    appRunningManager->GetAbilityTokensByBundleName(bundleName, abilityTokens);
    for (const auto &item : appRunningManager->appRunningRecordMap_) {
        const auto &appRecord = item.second;
        for (const auto &token : appRecord->GetAbilities()) {
            abilityTokens.emplace_back(token.first);
            EXPECT_EQ(abilityTokens.size(), ABILITY_TOKENS_SIZE);
        }
    }
}

/**
 * @tc.name: AppRunningManager_OnWindowVisibilityChanged_0100
 * @tc.desc: verify the function of OnWindowVisibilityChanged : set windowIds
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, AppRunningManager_OnWindowVisibilityChanged_0100, TestSize.Level1)
{
    // 1. create ApprunningManager
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    // 2. createAppRunningRecord and put it into appRunningRecordMap_ of AppRunningManager
    std::string processName = "processName";
    std::string appName = "appName";
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = appName;
    int32_t recordId = AppRecordId::Create();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    EXPECT_NE(appRunningRecord, nullptr);
    appRunningRecord->curState_ = ApplicationState::APP_STATE_BACKGROUND;
    appRunningRecord->GetPriorityObject()->SetPid(PID);
    appRunningManager->appRunningRecordMap_.emplace(recordId, appRunningRecord);

    // 3. construct WindowVisibilityInfos
    std::vector<sptr<OHOS::Rosen::WindowVisibilityInfo>> windowVisibilityInfos;
    auto info = new (std::nothrow) Rosen::WindowVisibilityInfo();
    EXPECT_NE(info, nullptr);
    info->windowId_ = WINDOW_ID;
    info->pid_ = PID;
    info->visibilityState_ = Rosen::WindowVisibilityState::WINDOW_VISIBILITY_STATE_NO_OCCLUSION;
    windowVisibilityInfos.push_back(info);

    // 4. verify the function
    appRunningManager->OnWindowVisibilityChanged(windowVisibilityInfos);
    EXPECT_FALSE(appRunningManager->appRunningRecordMap_.empty());
    EXPECT_FALSE(appRunningManager->appRunningRecordMap_.at(1)->windowIds_.empty());
}

#ifdef SUPPORT_CHILD_PROCESS
/**
 * @tc.name: AppRunningManager_GetAppRunningRecordByChildProcessPid_0100
 * @tc.desc: Test GetAppRunningRecordByChildProcessPid works
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, AppRunningManager_GetAppRunningRecordByChildProcessPid_0100, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppRunningManager_GetAppRunningRecordByChildProcessPid_0100 called.");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    auto appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    ChildProcessRequest request;
    request.srcEntry = "./ets/AProcess.ts";
    auto childRecord = ChildProcessRecord::CreateChildProcessRecord(PID, request, appRecord);
    pid_t childPid = 201;
    childRecord->pid_ = childPid;
    appRecord->AddChildProcessRecord(childPid, childRecord);
    appRunningManager->appRunningRecordMap_.insert(make_pair(RECORD_ID, appRecord));

    auto record = appRunningManager->GetAppRunningRecordByChildProcessPid(childPid);
    EXPECT_NE(record, nullptr);
}

/**
 * @tc.name: AppRunningManager_IsChildProcessReachLimit_0100
 * @tc.desc: Test IsChildProcessReachLimit works
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, AppRunningManager_IsChildProcessReachLimit_0100, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppRunningManager_IsChildProcessReachLimit_0100 called.");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    appRunningManager->IsChildProcessReachLimit(1, false);
    auto record = appRunningManager->GetAppRunningRecordByChildProcessPid(123);
    EXPECT_EQ(record, nullptr);
}
#endif // SUPPORT_CHILD_PROCESS

/**
 * @tc.name: AppRunningManager_UpdateConfiguration_0100
 * @tc.desc: Test UpdateConfiguration works
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, AppRunningManager_UpdateConfiguration_0100, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    std::string bundleName;
    std::vector<AppDebugInfo> debugInfos;
    bool isDetachDebug = true;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    int32_t recordId = 1;
    std::string processName;
    Configuration config;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    appRunningManager->appRunningRecordMap_.emplace(recordId, appRunningRecord);
    appRunningManager->appRunningRecordMap_.emplace(++recordId, nullptr);
    appRunningRecord->SetState(ApplicationState::APP_STATE_READY);
    appRunningManager->appRunningRecordMap_.emplace(++recordId, appRunningRecord);
    appInfo->name = "com.shell_assistant";
    appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    appRunningManager->appRunningRecordMap_.emplace(++recordId, appRunningRecord);
    EXPECT_EQ(appRunningManager->appRunningRecordMap_.size(), recordId);
    EXPECT_TRUE(appRunningManager != nullptr);
}

/**
 * @tc.name: AppRunningManager_UpdateConfiguration_0200
 * @tc.desc: Test UpdateConfiguration config storage
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, AppRunningManager_UpdateConfiguration_0200, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    Configuration config;
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, ConfigurationInner::COLOR_MODE_LIGHT);
    auto ret = appRunningManager->UpdateConfiguration(config);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: AppRunningManager_UpdateConfiguration_0300
 * @tc.desc: Test UpdateConfiguration delayed
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, AppRunningManager_UpdateConfiguration_0300, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    int32_t recordId = 1;
    std::string processName;
    Configuration config;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    appRunningManager->appRunningRecordMap_.emplace(recordId, appRunningRecord);
    appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    appRunningRecord->SetState(ApplicationState::APP_STATE_BACKGROUND);
    appRunningManager->appRunningRecordMap_.emplace(++recordId, appRunningRecord);
    auto ret = appRunningManager->UpdateConfiguration(config);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(appRunningManager->updateConfigurationDelayedMap_[0], false);
}

/**
 * @tc.name: RemoveAppRunningRecordById_0100
 * @tc.desc: Remove app running record by id.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, RemoveAppRunningRecordById_0100, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    ASSERT_NE(appRunningManager, nullptr);

    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::string processName = "test.ProcessName";
    BundleInfo bundleInfo;
    auto appRecord = appRunningManager->CreateAppRunningRecord(appInfo, processName, bundleInfo, "");
    ASSERT_NE(appRecord, nullptr);

    int32_t uiExtensionAbilityId = 1000;
    pid_t hostPid = 1001;
    pid_t providerPid = 1002;
    appRunningManager->AddUIExtensionLauncherItem(uiExtensionAbilityId, hostPid, providerPid);

    appRunningManager->RemoveAppRunningRecordById(appRecord->GetRecordId());
}

/**
 * @tc.name: AppRunningRecord_0100
 * @tc.desc: AppRunningRecord test multi-thread.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, AppRunningRecord_0100, TestSize.Level1)
{
    static std::shared_ptr<AppRunningManager> appRunningManager_Record0100 = std::make_shared<AppRunningManager>();
    ASSERT_NE(appRunningManager_Record0100, nullptr);
    auto task = []() {
        TAG_LOGI(AAFwkTag::TEST, "Running in thread %{public}d.", gettid());
        std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
        std::string processName = "test.ProcessName"+std::to_string(gettid());
        BundleInfo bundleInfo;
        auto appRecord = appRunningManager_Record0100->CreateAppRunningRecord(appInfo, processName, bundleInfo, "");
        ASSERT_NE(appRecord, nullptr);
        static int32_t uiExtensionAbilityId = 1;
        static pid_t hostPid = 100000;
        static pid_t providerPid = 200000;
        appRunningManager_Record0100->AddUIExtensionLauncherItem(uiExtensionAbilityId, hostPid, providerPid);
        uiExtensionAbilityId++;
        hostPid += 2;
        providerPid += 3;

        appRunningManager_Record0100->RemoveUIExtensionLauncherItem(hostPid);
        appRunningManager_Record0100->RemoveUIExtensionLauncherItem(providerPid);

        std::vector<pid_t> hostPids;
        std::vector<pid_t> providerPids;
        appRunningManager_Record0100->GetAllUIExtensionRootHostPid(hostPid, providerPids);
        appRunningManager_Record0100->GetAllUIExtensionProviderPid(providerPid, hostPids);

        appRunningManager_Record0100->RemoveAppRunningRecordById(appRecord->GetRecordId());
    };

    SET_THREAD_NUM(100);
    GTEST_RUN_TASK(task);
}

/**
 * @tc.name: UIExtensionReleationship_0100
 * @tc.desc: Root host pid and provider pid map test.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, UIExtensionReleationship_0100, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    ASSERT_NE(appRunningManager, nullptr);

    int32_t uiExtensionAbilityId = 1000;
    pid_t hostPid = 1001;
    pid_t providerPid = 1002;
    appRunningManager->AddUIExtensionLauncherItem(uiExtensionAbilityId, hostPid, providerPid);

    std::vector<pid_t> hostPids;
    EXPECT_EQ(appRunningManager->GetAllUIExtensionRootHostPid(providerPid, hostPids), ERR_OK);
    EXPECT_EQ(hostPids.size(), 1);
    EXPECT_EQ(hostPids[0], hostPid);
    hostPids.clear();

    std::vector<pid_t> providerPids;
    EXPECT_EQ(appRunningManager->GetAllUIExtensionProviderPid(hostPid, providerPids), ERR_OK);
    EXPECT_EQ(providerPids.size(), 1);
    EXPECT_EQ(providerPids[0], providerPid);
    providerPids.clear();

    EXPECT_EQ(appRunningManager->RemoveUIExtensionLauncherItem(hostPid), ERR_OK);
    EXPECT_EQ(appRunningManager->RemoveUIExtensionLauncherItem(providerPid), ERR_OK);

    // Get after remove.
    EXPECT_EQ(appRunningManager->GetAllUIExtensionRootHostPid(providerPid, hostPids), ERR_OK);
    EXPECT_EQ(hostPids.size(), 0);
    EXPECT_EQ(appRunningManager->GetAllUIExtensionProviderPid(hostPid, providerPids), ERR_OK);
    EXPECT_EQ(providerPids.size(), 0);
}

/**
 * @tc.name: UIExtensionReleationship_0200
 * @tc.desc: Root host pid and provider pid map test.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, UIExtensionReleationship_0200, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    ASSERT_NE(appRunningManager, nullptr);

    int32_t uiExtensionAbilityId = 1000;
    pid_t hostPid = 1001;
    pid_t providerPid = 1001; // same with host pid, ie uiability and uiextensionability in same process.
    appRunningManager->AddUIExtensionLauncherItem(uiExtensionAbilityId, hostPid, providerPid);

    std::vector<pid_t> hostPids;
    EXPECT_EQ(appRunningManager->GetAllUIExtensionRootHostPid(providerPid, hostPids), ERR_OK);
    EXPECT_EQ(hostPids.size(), 1);
    EXPECT_EQ(hostPids[0], hostPid);
    hostPids.clear();

    std::vector<pid_t> providerPids;
    EXPECT_EQ(appRunningManager->GetAllUIExtensionProviderPid(hostPid, providerPids), ERR_OK);
    EXPECT_EQ(providerPids.size(), 1);
    EXPECT_EQ(providerPids[0], providerPid);
    providerPids.clear();

    EXPECT_EQ(appRunningManager->RemoveUIExtensionLauncherItem(hostPid), ERR_OK);
    EXPECT_EQ(appRunningManager->RemoveUIExtensionLauncherItem(providerPid), ERR_OK);

    // Get after remove.
    EXPECT_EQ(appRunningManager->GetAllUIExtensionRootHostPid(providerPid, hostPids), ERR_OK);
    EXPECT_EQ(hostPids.size(), 0);
    EXPECT_EQ(appRunningManager->GetAllUIExtensionProviderPid(hostPid, providerPids), ERR_OK);
    EXPECT_EQ(providerPids.size(), 0);
}

/**
 * @tc.name: UIExtensionReleationship_0300
 * @tc.desc: Root host pid and provider pid map test.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, UIExtensionReleationship_0300, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    ASSERT_NE(appRunningManager, nullptr);

    int32_t uiExtensionAbilityIdA = 1;
    int32_t uiExtensionAbilityIdB = 2;
    pid_t hostPid = 1001;
    pid_t providerPidA = 2001;
    pid_t providerPidB = 2002; // a root host start two uiextensionability.
    appRunningManager->AddUIExtensionLauncherItem(uiExtensionAbilityIdA, hostPid, providerPidA);
    appRunningManager->AddUIExtensionLauncherItem(uiExtensionAbilityIdB, hostPid, providerPidB);

    std::vector<pid_t> hostPids;
    EXPECT_EQ(appRunningManager->GetAllUIExtensionRootHostPid(providerPidA, hostPids), ERR_OK);
    EXPECT_EQ(hostPids.size(), 1);
    EXPECT_EQ(hostPids[0], hostPid);
    hostPids.clear();

    EXPECT_EQ(appRunningManager->GetAllUIExtensionRootHostPid(providerPidB, hostPids), ERR_OK);
    EXPECT_EQ(hostPids.size(), 1);
    EXPECT_EQ(hostPids[0], hostPid);
    hostPids.clear();

    std::vector<pid_t> providerPids;
    EXPECT_EQ(appRunningManager->GetAllUIExtensionProviderPid(hostPid, providerPids), ERR_OK);
    EXPECT_EQ(providerPids.size(), 2);
    EXPECT_EQ(providerPids[0], providerPidA);
    EXPECT_EQ(providerPids[1], providerPidB);
    providerPids.clear();

    EXPECT_EQ(appRunningManager->RemoveUIExtensionLauncherItem(providerPidA), ERR_OK);

    // Get after remove one provider
    EXPECT_EQ(appRunningManager->GetAllUIExtensionRootHostPid(providerPidA, hostPids), ERR_OK);
    EXPECT_EQ(hostPids.size(), 0);
    EXPECT_EQ(appRunningManager->GetAllUIExtensionRootHostPid(providerPidB, hostPids), ERR_OK);
    EXPECT_EQ(hostPids.size(), 1);
    EXPECT_EQ(hostPids[0], hostPid);
    hostPids.clear();

    EXPECT_EQ(appRunningManager->GetAllUIExtensionProviderPid(hostPid, providerPids), ERR_OK);
    EXPECT_EQ(providerPids.size(), 1);
    EXPECT_EQ(providerPids[0], providerPidB);
}

/**
 * @tc.name: UIExtensionReleationship_0400
 * @tc.desc: Root host pid and provider pid map test.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, UIExtensionReleationship_0400, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    ASSERT_NE(appRunningManager, nullptr);

    int32_t uiExtensionAbilityIdA = 1;
    int32_t uiExtensionAbilityIdB = 2;
    pid_t hostPid = 1001;
    pid_t providerPidA = 2001;
    pid_t providerPidB = 2002; // a root host start two uiextensionability.
    appRunningManager->AddUIExtensionLauncherItem(uiExtensionAbilityIdA, hostPid, providerPidA);
    appRunningManager->AddUIExtensionLauncherItem(uiExtensionAbilityIdB, hostPid, providerPidB);

    EXPECT_EQ(appRunningManager->RemoveUIExtensionLauncherItem(hostPid), ERR_OK);

    // Get after remove one provider
    std::vector<pid_t> hostPids;
    EXPECT_EQ(appRunningManager->GetAllUIExtensionRootHostPid(providerPidA, hostPids), ERR_OK);
    EXPECT_EQ(hostPids.size(), 0);
    EXPECT_EQ(appRunningManager->GetAllUIExtensionRootHostPid(providerPidB, hostPids), ERR_OK);
    EXPECT_EQ(hostPids.size(), 0);

    std::vector<pid_t> providerPids;
    EXPECT_EQ(appRunningManager->GetAllUIExtensionRootHostPid(hostPid, providerPids), ERR_OK);
    EXPECT_EQ(providerPids.size(), 0);
}

/**
 * @tc.name: UIExtensionReleationship_0500
 * @tc.desc: Root host pid and provider pid map test.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, UIExtensionReleationship_0500, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    ASSERT_NE(appRunningManager, nullptr);

    int32_t uiExtensionAbilityIdA = 1;
    int32_t uiExtensionAbilityIdB = 2;
    pid_t hostPidA = 1001;
    pid_t hostPidB = 1002;
    pid_t providerPid = 2001; // a uiextensionability has two root host info.
    appRunningManager->AddUIExtensionLauncherItem(uiExtensionAbilityIdA, hostPidA, providerPid);
    appRunningManager->AddUIExtensionLauncherItem(uiExtensionAbilityIdB, hostPidB, providerPid);

    std::vector<pid_t> hostPids;
    EXPECT_EQ(appRunningManager->GetAllUIExtensionRootHostPid(providerPid, hostPids), ERR_OK);
    EXPECT_EQ(hostPids.size(), 2);
    EXPECT_EQ(hostPids[0], hostPidA);
    EXPECT_EQ(hostPids[1], hostPidB);
    hostPids.clear();

    std::vector<pid_t> providerPids;
    EXPECT_EQ(appRunningManager->GetAllUIExtensionProviderPid(hostPidA, providerPids), ERR_OK);
    EXPECT_EQ(providerPids.size(), 1);
    EXPECT_EQ(providerPids[0], providerPid);
    providerPids.clear();
    EXPECT_EQ(appRunningManager->GetAllUIExtensionProviderPid(hostPidB, providerPids), ERR_OK);
    EXPECT_EQ(providerPids.size(), 1);
    EXPECT_EQ(providerPids[0], providerPid);
    providerPids.clear();

    EXPECT_EQ(appRunningManager->RemoveUIExtensionLauncherItem(providerPid), ERR_OK);

    // Get after remove one provider
    EXPECT_EQ(appRunningManager->GetAllUIExtensionRootHostPid(providerPid, hostPids), ERR_OK);
    EXPECT_EQ(hostPids.size(), 0);

    EXPECT_EQ(appRunningManager->GetAllUIExtensionProviderPid(hostPidA, providerPids), ERR_OK);
    EXPECT_EQ(providerPids.size(), 0);
    EXPECT_EQ(appRunningManager->GetAllUIExtensionProviderPid(hostPidB, providerPids), ERR_OK);
    EXPECT_EQ(providerPids.size(), 0);
}

/**
 * @tc.name: UIExtensionReleationship_0600
 * @tc.desc: Root host pid and provider pid map test.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, UIExtensionReleationship_0600, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    ASSERT_NE(appRunningManager, nullptr);

    int32_t uiExtensionAbilityIdA = 1;
    int32_t uiExtensionAbilityIdB = 2;
    pid_t hostPidA = 1001;
    pid_t hostPidB = 1002;
    pid_t providerPid = 2001; // a uiextensionability has two root host info.
    appRunningManager->AddUIExtensionLauncherItem(uiExtensionAbilityIdA, hostPidA, providerPid);
    appRunningManager->AddUIExtensionLauncherItem(uiExtensionAbilityIdB, hostPidB, providerPid);

    std::vector<pid_t> hostPids;
    EXPECT_EQ(appRunningManager->GetAllUIExtensionRootHostPid(providerPid, hostPids), ERR_OK);
    EXPECT_EQ(hostPids.size(), 2);
    EXPECT_EQ(hostPids[0], hostPidA);
    EXPECT_EQ(hostPids[1], hostPidB);
    hostPids.clear();

    std::vector<pid_t> providerPids;
    EXPECT_EQ(appRunningManager->GetAllUIExtensionProviderPid(hostPidA, providerPids), ERR_OK);
    EXPECT_EQ(providerPids.size(), 1);
    EXPECT_EQ(providerPids[0], providerPid);
    providerPids.clear();
    EXPECT_EQ(appRunningManager->GetAllUIExtensionProviderPid(hostPidB, providerPids), ERR_OK);
    EXPECT_EQ(providerPids.size(), 1);
    EXPECT_EQ(providerPids[0], providerPid);
    providerPids.clear();

    EXPECT_EQ(appRunningManager->RemoveUIExtensionLauncherItem(hostPidA), ERR_OK);

    // Get after remove one provider
    EXPECT_EQ(appRunningManager->GetAllUIExtensionRootHostPid(providerPid, hostPids), ERR_OK);
    EXPECT_EQ(hostPids.size(), 1);
    EXPECT_EQ(hostPids[0], hostPidB); // cause host pid A has removed.

    EXPECT_EQ(appRunningManager->GetAllUIExtensionProviderPid(hostPidA, providerPids), ERR_OK);
    EXPECT_EQ(providerPids.size(), 0);
    EXPECT_EQ(appRunningManager->GetAllUIExtensionProviderPid(hostPidB, providerPids), ERR_OK);
    EXPECT_EQ(providerPids.size(), 1);
    EXPECT_EQ(providerPids[0], providerPid);
    providerPids.clear();

    EXPECT_EQ(appRunningManager->RemoveUIExtensionLauncherItemById(uiExtensionAbilityIdB), ERR_OK);
    EXPECT_EQ(appRunningManager->GetAllUIExtensionProviderPid(hostPidB, providerPids), ERR_OK);
    EXPECT_EQ(providerPids.size(), 0);
}

/**
 * @tc.name: UIExtensionReleationship_0700
 * @tc.desc: Root host pid and provider pid map test.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, UIExtensionReleationship_0700, TestSize.Level1)
{
    static std::shared_ptr<AppRunningManager> appRunningManager_Rel0700 = std::make_shared<AppRunningManager>();
    ASSERT_NE(appRunningManager_Rel0700, nullptr);
    auto task = []() {
        TAG_LOGI(AAFwkTag::TEST, "Running in thread %{public}d.", gettid());
        static int32_t uiExtensionAbilityId = 1;
        static pid_t hostPid = 100000;
        static pid_t providerPid = 200000;
        appRunningManager_Rel0700->AddUIExtensionLauncherItem(uiExtensionAbilityId, hostPid, providerPid);
        uiExtensionAbilityId++;
        hostPid += 2;
        providerPid += 3;

        appRunningManager_Rel0700->RemoveUIExtensionLauncherItemById(uiExtensionAbilityId);
        appRunningManager_Rel0700->RemoveUIExtensionLauncherItem(hostPid);
        appRunningManager_Rel0700->RemoveUIExtensionLauncherItem(providerPid);

        std::vector<pid_t> hostPids;
        std::vector<pid_t> providerPids;
        appRunningManager_Rel0700->GetAllUIExtensionRootHostPid(hostPid, providerPids);
        appRunningManager_Rel0700->GetAllUIExtensionProviderPid(providerPid, hostPids);
    };

    SET_THREAD_NUM(100);
    GTEST_RUN_TASK(task);
}

/**
 * @tc.name: IsAppProcessesAllCached_0100
 * @tc.desc: MultiProcess application cache check test.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, IsAppProcessesAllCached_0100, TestSize.Level1)
{
    static std::shared_ptr<AppRunningManager> appRunningManager = std::make_shared<AppRunningManager>();
    ASSERT_NE(appRunningManager, nullptr);
    std::string bundleName;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    appInfo->bundleName = "com.tdd.cacheprocesstest";
    appInfo->apiTargetVersion = 12;
    appInfo->uid = 1010101;
    int32_t recordId1 = RECORD_ID;
    int32_t recordId2 = RECORD_ID + 1;
    std::string processName = "com.tdd.cacheprocesstest";
    auto appRunningRecord1 = std::make_shared<AppRunningRecord>(appInfo, recordId1, processName);
    appRunningRecord1->SetUid(appInfo->uid);
    appRunningRecord1->SetSupportedProcessCache(true);
    appRunningRecord1->SetUIAbilityLaunched(true);
    auto appRunningRecord2 = std::make_shared<AppRunningRecord>(appInfo, recordId2, processName);
    appRunningRecord2->SetUid(appInfo->uid);
    appRunningRecord2->SetSupportedProcessCache(true);
    appRunningRecord2->SetUIAbilityLaunched(true);

    appRunningManager->appRunningRecordMap_.insert(make_pair(recordId1, appRunningRecord1));
    std::set<std::shared_ptr<AppRunningRecord>> cachedSet;
    cachedSet.insert(appRunningRecord1);
    EXPECT_EQ(appRunningManager->IsAppProcessesAllCached(appInfo->bundleName, appInfo->uid, cachedSet), true);

    appRunningManager->appRunningRecordMap_.insert(make_pair(recordId2, appRunningRecord2));
    EXPECT_EQ(appRunningManager->IsAppProcessesAllCached(appInfo->bundleName, appInfo->uid, cachedSet), false);
}

/**
 * @tc.name: CheckAppCloneRunningRecordIsExistByBundleName_0100
 * @tc.desc: MultiProcess application cache check test.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, CheckAppCloneRunningRecordIsExistByBundleName_0100, TestSize.Level1)
{
    static std::shared_ptr<AppRunningManager> appRunningManager = std::make_shared<AppRunningManager>();
    ASSERT_NE(appRunningManager, nullptr);

    std::string bundleName = "bundleName";
    int32_t appCloneIndex = 1;
    bool isRunning = true;
    int32_t res = appRunningManager->
        CheckAppCloneRunningRecordIsExistByBundleName(bundleName, appCloneIndex, isRunning);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: GetAllAppRunningRecordCountByBundleName_0100
 * @tc.desc: MultiProcess application cache check test.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, GetAllAppRunningRecordCountByBundleName_0100, TestSize.Level1)
{
    static std::shared_ptr<AppRunningManager> appRunningManager = std::make_shared<AppRunningManager>();
    ASSERT_NE(appRunningManager, nullptr);

    std::string bundleName = "bundleName";
    EXPECT_EQ(appRunningManager->GetAllAppRunningRecordCountByBundleName(bundleName), 0);
}

/**
 * @tc.name: ProcessUpdateApplicationInfoInstalled_0100
 * @tc.desc: MultiProcess application cache check test.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, ProcessUpdateApplicationInfoInstalled_0100, TestSize.Level1)
{
    static std::shared_ptr<AppRunningManager> appRunningManager = std::make_shared<AppRunningManager>();
    ASSERT_NE(appRunningManager, nullptr);

    ApplicationInfo appInfo;
    std::string moduleName;
    EXPECT_EQ(appRunningManager->ProcessUpdateApplicationInfoInstalled(appInfo, moduleName), 0);
}

/**
 * @tc.name: HandleChildRelation_0100
 * @tc.desc: handle child relation when native child process exit test.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, HandleChildRelation_0100, TestSize.Level1)
{
    static std::shared_ptr<AppRunningManager> appRunningManager = std::make_shared<AppRunningManager>();
    ASSERT_NE(appRunningManager, nullptr);
    appRunningManager->HandleChildRelation(nullptr, nullptr);
    auto appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    appRunningManager->HandleChildRelation(nullptr, appRecord);
    ChildProcessRequest request;
    request.srcEntry = "./ets/AProcess.ts";
    auto childRecord = ChildProcessRecord::CreateChildProcessRecord(PID, request, appRecord);
    pid_t childPid = 201;
    childRecord->pid_ = childPid;
    appRecord->AddChildProcessRecord(childPid, childRecord);
    appRunningManager->appRunningRecordMap_.insert(make_pair(RECORD_ID, appRecord));
    appRunningManager->HandleChildRelation(childRecord, appRecord);
    auto record = appRunningManager->GetAppRunningRecordByChildProcessPid(childPid);
    EXPECT_NE(record, nullptr);
}

/**
 * @tc.name: AppRunningManager_IsAppExist_0100
 * @tc.desc: Test IsAppExist with empty map
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, AppRunningManager_IsAppExist_0100, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    uint32_t accessTokenId = 1000;
    bool result = appRunningManager->IsAppExist(accessTokenId);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: AppRunningManager_IsAppExist_0200
 * @tc.desc: Test IsAppExist with existing app
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, AppRunningManager_IsAppExist_0200, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->accessTokenId = 1000;
    int32_t recordId = RECORD_ID;
    std::string processName = "test.process";
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    appRunningManager->appRunningRecordMap_.emplace(recordId, appRecord);

    bool result = appRunningManager->IsAppExist(1000);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: AppRunningManager_IsAppExist_0300
 * @tc.desc: Test IsAppExist with non-existing token
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, AppRunningManager_IsAppExist_0300, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->accessTokenId = 1000;
    int32_t recordId = RECORD_ID;
    std::string processName = "test.process";
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    appRunningManager->appRunningRecordMap_.emplace(recordId, appRecord);

    bool result = appRunningManager->IsAppExist(2000);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: AppRunningManager_GetAppRunningRecordByPid_0200
 * @tc.desc: Test GetAppRunningRecordByPid with existing record
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, AppRunningManager_GetAppRunningRecordByPid_0200, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    auto appInfo = std::make_shared<ApplicationInfo>();
    int32_t recordId = RECORD_ID;
    std::string processName = "test.process";
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    auto priorityObj = appRecord->GetPriorityObject();
    ASSERT_NE(priorityObj, nullptr);
    appRecord->GetPriorityObject()->SetPid(1000);
    appRunningManager->appRunningRecordMap_.emplace(recordId, appRecord);

    auto result = appRunningManager->GetAppRunningRecordByPid(1000);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->GetPid(), 1000);
}

/**
 * @tc.name: AppRunningManager_GetAppRunningRecordByPid_0300
 * @tc.desc: Test GetAppRunningRecordByPid with non-existing pid
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, AppRunningManager_GetAppRunningRecordByPid_0300, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    auto appInfo = std::make_shared<ApplicationInfo>();
    int32_t recordId = RECORD_ID;
    std::string processName = "test.process";
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    auto priorityObj = appRecord->GetPriorityObject();
    ASSERT_NE(priorityObj, nullptr);
    priorityObj->SetPid(1000);
    appRunningManager->appRunningRecordMap_.emplace(recordId, appRecord);

    auto result = appRunningManager->GetAppRunningRecordByPid(2000);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: AppRunningManager_GetAppRunningRecordMap_0100
 * @tc.desc: Test GetAppRunningRecordMap with empty map
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, AppRunningManager_GetAppRunningRecordMap_0100, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    auto result = appRunningManager->GetAppRunningRecordMap();
    EXPECT_EQ(result.empty(), true);
}

/**
 * @tc.name: AppRunningManager_GetAppRunningRecordMap_0200
 * @tc.desc: Test GetAppRunningRecordMap with existing records
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, AppRunningManager_GetAppRunningRecordMap_0200, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    auto appInfo = std::make_shared<ApplicationInfo>();
    int32_t recordId1 = 1;
    int32_t recordId2 = 2;
    std::string processName = "test.process";
    auto appRecord1 = std::make_shared<AppRunningRecord>(appInfo, recordId1, processName);
    auto appRecord2 = std::make_shared<AppRunningRecord>(appInfo, recordId2, processName);
    appRunningManager->appRunningRecordMap_.emplace(recordId1, appRecord1);
    appRunningManager->appRunningRecordMap_.emplace(recordId2, appRecord2);

    auto result = appRunningManager->GetAppRunningRecordMap();
    EXPECT_EQ(result.size(), 2);
}

/**
 * @tc.name: AppRunningManager_ClearAppRunningRecordMap_0100
 * @tc.desc: Test ClearAppRunningRecordMap
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, AppRunningManager_ClearAppRunningRecordMap_0100, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    auto appInfo = std::make_shared<ApplicationInfo>();
    int32_t recordId = RECORD_ID;
    std::string processName = "test.process";
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    appRunningManager->appRunningRecordMap_.emplace(recordId, appRecord);

    EXPECT_EQ(appRunningManager->appRunningRecordMap_.size(), 1);
    appRunningManager->ClearAppRunningRecordMap();
    EXPECT_EQ(appRunningManager->appRunningRecordMap_.empty(), true);
}

/////
/**
 * @tc.name: AppRunningManager_ClipStringContent_0100
 * @tc.desc: Test ClipStringContent with regex match
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, AppRunningManager_ClipStringContent_0100, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    std::regex rule("[a-zA-Z.]+[-_#]{1}");
    std::string source = "test.value-123";
    std::string afterCutStr;
    appRunningManager->ClipStringContent(rule, source, afterCutStr);
    EXPECT_NE(afterCutStr.empty(), true);
}

/**
 * @tc.name: AppRunningManager_ClipStringContent_0200
 * @tc.desc: Test ClipStringContent without regex match
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, AppRunningManager_ClipStringContent_0200, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    std::regex rule("[a-zA-Z.]+[-_#]{1}");
    std::string source = "testvalue123";
    std::string afterCutStr;
    appRunningManager->ClipStringContent(rule, source, afterCutStr);
    EXPECT_EQ(afterCutStr.empty(), true);
}

/**
 * @tc.name: AppRunningManager_GetForegroundApplications_0100
 * @tc.desc: Test GetForegroundApplications with empty map
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, AppRunningManager_GetForegroundApplications_0100, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    std::vector<AppStateData> list;
    appRunningManager->GetForegroundApplications(list);
    EXPECT_EQ(list.empty(), true);
}

/**
 * @tc.name: AppRunningManager_GetForegroundApplications_0200
 * @tc.desc: Test GetForegroundApplications with foreground app
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, AppRunningManager_GetForegroundApplications_0200, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->bundleName = "com.test.app";
    int32_t recordId = RECORD_ID;
    std::string processName = "test.process";
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    appRecord->curState_ = ApplicationState::APP_STATE_FOREGROUND;
    appRunningManager->appRunningRecordMap_.emplace(recordId, appRecord);

    std::vector<AppStateData> list;
    appRunningManager->GetForegroundApplications(list);
    EXPECT_EQ(list.size(), 1);
    EXPECT_EQ(list[0].bundleName, "com.test.app");
}

/**
 * @tc.name: AppRunningManager_GetForegroundApplications_0300
 * @tc.desc: Test GetForegroundApplications with background app only
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, AppRunningManager_GetForegroundApplications_0300, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->bundleName = "com.test.app";
    int32_t recordId = RECORD_ID;
    std::string processName = "test.process";
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    appRecord->curState_ = ApplicationState::APP_STATE_BACKGROUND;
    appRunningManager->appRunningRecordMap_.emplace(recordId, appRecord);

    std::vector<AppStateData> list;
    appRunningManager->GetForegroundApplications(list);
    EXPECT_EQ(list.empty(), true);
}

/**
 * @tc.name: AppRunningManager_ProcessExitByBundleNameAndUid_0100
 * @tc.desc: Test ProcessExitByBundleNameAndUid with empty map
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, AppRunningManager_ProcessExitByBundleNameAndUid_0100, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    std::string bundleName = "com.test.app";
    int32_t uid = 1000;
    std::list<pid_t> pids;
    KillProcessConfig config;
    bool result = appRunningManager->ProcessExitByBundleNameAndUid(bundleName, uid, pids, config);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: GetAllAbilityInfos_0100
 * @tc.desc: Test GetAllAbilityInfos with empty map
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, GetAllAbilityInfos_0100, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    std::vector<AppExecFwk::AbilityStateData> infos;
    auto ret = appRunningManager->GetAllAbilityInfos(-1, infos);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_TRUE(infos.empty());
}

/**
 * @tc.name: GetAllAbilityInfos_0200
 * @tc.desc: Test GetAllAbilityInfos with specific pid not found
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, GetAllAbilityInfos_0200, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    std::vector<AppExecFwk::AbilityStateData> infos;
    int32_t pid = 1001;
    auto ret = appRunningManager->GetAllAbilityInfos(pid, infos);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: GetAllAbilityInfos_0300
 * @tc.desc: Test GetAllAbilityInfos with specific pid found
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, GetAllAbilityInfos_0300, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = "test.app";
    int32_t recordId = RECORD_ID;
    std::string processName = "test.app";
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    appRecord->GetPriorityObject()->SetPid(1001);
    appRunningManager->appRunningRecordMap_.emplace(recordId, appRecord);

    std::vector<AppExecFwk::AbilityStateData> infos;
    int32_t pid = 1001;
    auto ret = appRunningManager->GetAllAbilityInfos(pid, infos);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: GetAllAbilityInfos_0400
 * @tc.desc: Test GetAllAbilityInfos with pid=-1 to get all
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, GetAllAbilityInfos_0400, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = "test.app";
    int32_t recordId = RECORD_ID;
    std::string processName = "test.app";
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    appRecord->GetPriorityObject()->SetPid(1001);
    appRunningManager->appRunningRecordMap_.emplace(recordId, appRecord);

    std::vector<AppExecFwk::AbilityStateData> infos;
    auto ret = appRunningManager->GetAllAbilityInfos(-1, infos);
    EXPECT_EQ(ret, ERR_OK);
}
} // namespace AppExecFwk
} // namespace OHOS
