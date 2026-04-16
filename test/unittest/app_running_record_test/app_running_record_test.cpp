/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include <set>

#define private public
#define protected public
#include "ability_running_record.h"
#include "ability_window_configuration.h"
#include "app_running_record.h"
#include "app_mgr_service_event_handler.h"
#include "app_mgr_service_inner.h"
#ifdef SUPPORT_CHILD_PROCESS
#include "child_process_record.h"
#endif // SUPPORT_CHILD_PROCESS
#include "module_running_record.h"
#include "want_params.h"
#undef private
#undef protected
#include "app_mgr_service_dump_error_code.h"
#include "hilog_tag_wrapper.h"
#include "insight_intent_execute_param.h"
#include "mock_ability_token.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int64_t TIMEOUT = 1;
constexpr int32_t RECORD_ID = 1;
constexpr int32_t START_PROCESS_SPECIFY_ABILITY_EVENT_ID = 1;
constexpr int32_t ADD_ABILITY_STAGE_INFO_EVENT_ID = 2;
constexpr int32_t TERMINATE_ABILITY_SIZE = 0;
}
class AppRunningRecordTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AppRunningRecordTest::SetUpTestCase(void)
{}

void AppRunningRecordTest::TearDownTestCase(void)
{}

void AppRunningRecordTest::SetUp()
{}

void AppRunningRecordTest::TearDown()
{}

/**
 * @tc.name: AppRunningRecord_SendEvent_0100
 * @tc.desc: Test the status of SendEvent when msg is a value of 4.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_SendEvent_0100, TestSize.Level1)
{
    uint32_t msg = AMSEventHandler::START_PROCESS_SPECIFIED_ABILITY_TIMEOUT_MSG;
    int64_t timeOut = TIMEOUT;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    EXPECT_NE(appRunningRecord, nullptr);
    appRunningRecord->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    appRunningRecord->eventHandler_ =
        std::make_shared<AMSEventHandler>(appRunningRecord->taskHandler_, appRunningRecord->appMgrServiceInner_);
    appRunningRecord->isDebugApp_ = false;
    appRunningRecord->isNativeDebug_ = false;
    appRunningRecord->isAttachDebug_ = false;
    appRunningRecord->SendEvent(msg, timeOut);
}

/**
 * @tc.name: AppRunningRecord_SendEvent_0200
 * @tc.desc: Test the status of SendEvent when msg is a value of 2.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_SendEvent_0200, TestSize.Level1)
{
    uint32_t msg = AMSEventHandler::ADD_ABILITY_STAGE_INFO_TIMEOUT_MSG;
    int64_t timeOut = TIMEOUT;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    EXPECT_NE(appRunningRecord, nullptr);
    appRunningRecord->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    appRunningRecord->eventHandler_ =
        std::make_shared<AMSEventHandler>(appRunningRecord->taskHandler_, appRunningRecord->appMgrServiceInner_);
    appRunningRecord->isDebugApp_ = false;
    appRunningRecord->isNativeDebug_ = false;
    appRunningRecord->isAttachDebug_ = false;
    appRunningRecord->SendEvent(msg, timeOut);
}

/**
 * @tc.name: AppRunningRecord_SetAttachDebug_0100
 * @tc.desc: Test the status of SetAttachDebug based on the isAttachDebug value being true.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_SetAttachDebug_0100, TestSize.Level1)
{
    bool isAttachDebug = true;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    EXPECT_NE(appRunningRecord, nullptr);
    appRunningRecord->SetAttachDebug(isAttachDebug, false);
    EXPECT_EQ(appRunningRecord->isAttachDebug_, true);
}

/**
 * @tc.name: AppRunningRecord_SetAttachDebug_0200
 * @tc.desc: Test the status of SetAttachDebug based on the isAttachDebug value being false.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_SetAttachDebug_0200, TestSize.Level1)
{
    bool isAttachDebug = false;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    EXPECT_NE(appRunningRecord, nullptr);
    appRunningRecord->SetAttachDebug(isAttachDebug, false);
    EXPECT_EQ(appRunningRecord->isAttachDebug_, false);
}

/**
 * @tc.name: AppRunningRecord_TerminateAbility_0100
 * @tc.desc: Test the status of TerminateAbility.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_TerminateAbility_0100, TestSize.Level1)
{
    bool isForce = true;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    EXPECT_NE(appRunningRecord, nullptr);
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    auto info = nullptr;
    auto eventHandler = nullptr;
    auto moduleRecord = std::make_shared<ModuleRunningRecord>(info, eventHandler);
    EXPECT_NE(moduleRecord, nullptr);
    appRunningRecord->TerminateAbility(token, isForce);
    EXPECT_EQ(moduleRecord->terminateAbilities_.size(), TERMINATE_ABILITY_SIZE);
}

/**
 * @tc.name: AppRunningRecord_AbilityTerminated_0100
 * @tc.desc: Test the status of AbilityTerminated.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_AbilityTerminated_0100, TestSize.Level1)
{
    sptr<IRemoteObject> token;
    bool isForce = true;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    EXPECT_NE(appRunningRecord, nullptr);
    appRunningRecord->AbilityTerminated(token);
    EXPECT_EQ(appRunningRecord->processType_, ProcessType::NORMAL);
}

/**
 * @tc.name: AppRunningRecord_HasAgentExtensionAbility_0100
 * @tc.desc: HasAgentExtensionAbility returns false when process has no agent extension ability.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_HasAgentExtensionAbility_0100, TestSize.Level1)
{
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->bundleName = "com.test.bundle";
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.test.process");
    ASSERT_NE(appRunningRecord, nullptr);

    auto moduleRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);
    ASSERT_NE(moduleRecord, nullptr);
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::EXTENSION;
    abilityInfo->extensionAbilityType = ExtensionAbilityType::SERVICE;
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    ASSERT_NE(token, nullptr);
    ASSERT_NE(moduleRecord->AddAbility(token, abilityInfo, nullptr, RECORD_ID), nullptr);
    appRunningRecord->hapModules_[appInfo->bundleName].emplace_back(moduleRecord);

    EXPECT_FALSE(appRunningRecord->HasAgentExtensionAbility());
}

/**
 * @tc.name: AppRunningRecord_HasAgentExtensionAbility_0200
 * @tc.desc: HasAgentExtensionAbility returns true when process has agent extension ability.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_HasAgentExtensionAbility_0200, TestSize.Level1)
{
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->bundleName = "com.test.bundle";
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.test.process");
    ASSERT_NE(appRunningRecord, nullptr);

    auto moduleRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);
    ASSERT_NE(moduleRecord, nullptr);
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::EXTENSION;
    abilityInfo->extensionAbilityType = ExtensionAbilityType::AGENT;
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    ASSERT_NE(token, nullptr);
    ASSERT_NE(moduleRecord->AddAbility(token, abilityInfo, nullptr, RECORD_ID), nullptr);
    appRunningRecord->hapModules_[appInfo->bundleName].emplace_back(moduleRecord);

    EXPECT_TRUE(appRunningRecord->HasAgentExtensionAbility());
}

/**
 * @tc.name: AppRunningRecord_IsLastAgentExtensionAbility_0100
 * @tc.desc: IsLastAgentExtensionAbility returns false when token does not match any ability record.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_IsLastAgentExtensionAbility_0100, TestSize.Level1)
{
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->bundleName = "com.test.bundle";
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.test.process");
    ASSERT_NE(appRunningRecord, nullptr);

    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    ASSERT_NE(token, nullptr);

    EXPECT_FALSE(appRunningRecord->IsLastAgentExtensionAbility(token));
}

/**
 * @tc.name: AppRunningRecord_IsLastAgentExtensionAbility_0200
 * @tc.desc: IsLastAgentExtensionAbility returns false when target token is not an agent extension ability.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_IsLastAgentExtensionAbility_0200, TestSize.Level1)
{
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->bundleName = "com.test.bundle";
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.test.process");
    ASSERT_NE(appRunningRecord, nullptr);

    auto moduleRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);
    ASSERT_NE(moduleRecord, nullptr);
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::EXTENSION;
    abilityInfo->extensionAbilityType = ExtensionAbilityType::SERVICE;
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    ASSERT_NE(token, nullptr);
    ASSERT_NE(moduleRecord->AddAbility(token, abilityInfo, nullptr, RECORD_ID), nullptr);
    appRunningRecord->hapModules_[appInfo->bundleName].emplace_back(moduleRecord);

    EXPECT_FALSE(appRunningRecord->IsLastAgentExtensionAbility(token));
}

/**
 * @tc.name: AppRunningRecord_IsLastAgentExtensionAbility_0300
 * @tc.desc: IsLastAgentExtensionAbility returns true when target token is the only agent extension ability.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_IsLastAgentExtensionAbility_0300, TestSize.Level1)
{
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->bundleName = "com.test.bundle";
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.test.process");
    ASSERT_NE(appRunningRecord, nullptr);

    auto moduleRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);
    ASSERT_NE(moduleRecord, nullptr);

    auto agentAbilityInfo = std::make_shared<AbilityInfo>();
    agentAbilityInfo->type = AbilityType::EXTENSION;
    agentAbilityInfo->extensionAbilityType = ExtensionAbilityType::AGENT;
    sptr<IRemoteObject> agentToken = new (std::nothrow) MockAbilityToken();
    ASSERT_NE(agentToken, nullptr);
    ASSERT_NE(moduleRecord->AddAbility(agentToken, agentAbilityInfo, nullptr, RECORD_ID), nullptr);

    auto serviceAbilityInfo = std::make_shared<AbilityInfo>();
    serviceAbilityInfo->type = AbilityType::EXTENSION;
    serviceAbilityInfo->extensionAbilityType = ExtensionAbilityType::SERVICE;
    sptr<IRemoteObject> serviceToken = new (std::nothrow) MockAbilityToken();
    ASSERT_NE(serviceToken, nullptr);
    ASSERT_NE(moduleRecord->AddAbility(serviceToken, serviceAbilityInfo, nullptr, RECORD_ID + 1), nullptr);

    appRunningRecord->hapModules_[appInfo->bundleName].emplace_back(moduleRecord);

    EXPECT_TRUE(appRunningRecord->IsLastAgentExtensionAbility(agentToken));
}

/**
 * @tc.name: AppRunningRecord_IsLastAgentExtensionAbility_0400
 * @tc.desc: IsLastAgentExtensionAbility returns false when process has more than one agent extension ability.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_IsLastAgentExtensionAbility_0400, TestSize.Level1)
{
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->bundleName = "com.test.bundle";
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.test.process");
    ASSERT_NE(appRunningRecord, nullptr);

    auto firstModuleRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);
    auto secondModuleRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);
    ASSERT_NE(firstModuleRecord, nullptr);
    ASSERT_NE(secondModuleRecord, nullptr);

    auto firstAgentAbilityInfo = std::make_shared<AbilityInfo>();
    firstAgentAbilityInfo->type = AbilityType::EXTENSION;
    firstAgentAbilityInfo->extensionAbilityType = ExtensionAbilityType::AGENT;
    sptr<IRemoteObject> firstAgentToken = new (std::nothrow) MockAbilityToken();
    ASSERT_NE(firstAgentToken, nullptr);
    ASSERT_NE(firstModuleRecord->AddAbility(firstAgentToken, firstAgentAbilityInfo, nullptr, RECORD_ID), nullptr);

    auto secondAgentAbilityInfo = std::make_shared<AbilityInfo>();
    secondAgentAbilityInfo->type = AbilityType::EXTENSION;
    secondAgentAbilityInfo->extensionAbilityType = ExtensionAbilityType::AGENT;
    sptr<IRemoteObject> secondAgentToken = new (std::nothrow) MockAbilityToken();
    ASSERT_NE(secondAgentToken, nullptr);
    ASSERT_NE(secondModuleRecord->AddAbility(secondAgentToken, secondAgentAbilityInfo, nullptr, RECORD_ID + 1),
        nullptr);

    appRunningRecord->hapModules_[appInfo->bundleName].emplace_back(firstModuleRecord);
    appRunningRecord->hapModules_[appInfo->bundleName].emplace_back(secondModuleRecord);

    EXPECT_FALSE(appRunningRecord->IsLastAgentExtensionAbility(firstAgentToken));
}

#ifdef SUPPORT_CHILD_PROCESS
/**
 * @tc.name: AppRunningRecord_AddChildProcessRecord_0100
 * @tc.desc: Test AddChildProcessRecord works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_AddChildProcessRecord_0100, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppRunningRecord_AddChildProcessRecord_0100 called.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    EXPECT_NE(appRecord, nullptr);

    ChildProcessRequest request;
    request.srcEntry = "./ets/AProcess.ts";
    auto childRecord = std::make_shared<ChildProcessRecord>(101, request, appRecord);
    pid_t childPid = 201;
    childRecord->SetPid(childPid);
    appRecord->AddChildProcessRecord(childPid, childRecord);

    auto childProcessRecordMap = appRecord->childProcessRecordMap_;
    auto iter = childProcessRecordMap.find(childPid);
    EXPECT_NE(iter, childProcessRecordMap.end());
}

/**
 * @tc.name: AppRunningRecord_RemoveChildProcessRecord_0100
 * @tc.desc: Test RemoveChildProcessRecord works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_RemoveChildProcessRecord_0100, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppRunningRecord_RemoveChildProcessRecord_0100 called.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    EXPECT_NE(appRecord, nullptr);

    ChildProcessRequest request;
    request.srcEntry = "./ets/AProcess.ts";
    auto childRecord = std::make_shared<ChildProcessRecord>(101, request, appRecord);
    pid_t childPid = 201;
    childRecord->SetPid(childPid);
    appRecord->childProcessRecordMap_.emplace(childPid, childRecord);

    appRecord->RemoveChildProcessRecord(childRecord);
    auto childProcessRecordMap = appRecord->childProcessRecordMap_;
    auto iter = childProcessRecordMap.find(childPid);
    EXPECT_EQ(iter, childProcessRecordMap.end());
}

/**
 * @tc.name: AppRunningRecord_GetChildProcessRecordByPid_0100
 * @tc.desc: Test GetChildProcessRecordByPid works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_GetChildProcessRecordByPid_0100, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppRunningRecord_GetChildProcessRecordByPid_0100 called.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    EXPECT_NE(appRecord, nullptr);

    ChildProcessRequest request;
    request.srcEntry = "./ets/AProcess.ts";
    auto childRecord = std::make_shared<ChildProcessRecord>(101, request, appRecord);
    pid_t childPid = 201;
    childRecord->SetPid(childPid);
    appRecord->childProcessRecordMap_.emplace(childPid, childRecord);

    auto record = appRecord->GetChildProcessRecordByPid(childPid);
    EXPECT_NE(record, nullptr);
}

/**
 * @tc.name: AppRunningRecord_GetChildProcessRecordMap_0100
 * @tc.desc: Test GetChildProcessRecordByPid works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_GetChildProcessRecordMap_0100, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppRunningRecord_GetChildProcessRecordMap_0100 called.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    EXPECT_NE(appRecord, nullptr);

    auto childProcessRecordMap = appRecord->GetChildProcessRecordMap();
    EXPECT_EQ(childProcessRecordMap.size(), 0);
}
#endif // SUPPORT_CHILD_PROCESS

/**
 * @tc.name: GetSplitModeAndFloatingMode_0100
 * @tc.desc: Test the return when abilityWant is not nullptr and
 *      the first determine of windowMode is conformed.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, GetSplitModeAndFloatingMode_001, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> info = nullptr;
    int32_t recordId = 0;
    std::string processName = "appRunningRecordProcessName";
    auto appRunningRecord = std::make_shared<AppRunningRecord>(info, recordId, processName);

    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<ModuleRunningRecord> moduleRunningRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = nullptr;
    auto abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 0);

    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    want->SetParam(AAFwk::Want::PARAM_RESV_WINDOW_MODE,
        AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING);
    abilityRecord->SetWant(want);
    moduleRunningRecord->abilities_.emplace(nullptr, abilityRecord);

    std::vector<std::shared_ptr<ModuleRunningRecord>> hapModulesVector;
    hapModulesVector.emplace_back(moduleRunningRecord);
    std::string hapModulesString = "hapModulesString";
    appRunningRecord->hapModules_.emplace(hapModulesString, hapModulesVector);

    bool isSplitScreenMode = false;
    bool isFloatingWindowMode = false;
    appRunningRecord->GetSplitModeAndFloatingMode(isSplitScreenMode, isFloatingWindowMode);
    EXPECT_EQ(true, isFloatingWindowMode);
}

/**
 * @tc.name: AppRunningRecord_GetAssignTokenId_0100
 * @tc.desc: Test GetAssignTokenId works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_GetAssignTokenId_0100, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppRunningRecord_GetAssignTokenId_0100 called.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    EXPECT_NE(appRecord, nullptr);

    int32_t assignTokenId = appRecord->GetAssignTokenId();
    EXPECT_EQ(assignTokenId, 0);
}

/**
 * @tc.name: AppRunningRecord_SetAssignTokenId_0100
 * @tc.desc: Test SetAssignTokenId works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_SetAssignTokenId_0100, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppRunningRecord_GetAssignTokenId_0100 called.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    EXPECT_NE(appRecord, nullptr);
    int32_t setId = 100;
    appRecord->SetAssignTokenId(setId);
    int32_t assignTokenId = appRecord->GetAssignTokenId();
    EXPECT_EQ(assignTokenId, setId);
}

/**
 * @tc.name: AppRunningRecord_SetDebugFromLocal_0100
 * @tc.desc: Test SetDebugFromLocal works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_SetDebugFromLocal_0100, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppRunningRecord_SetDebugFromLocal_0100 called.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    EXPECT_NE(appRecord, nullptr);
    bool isDebugFromLocal = false;
    appRecord->SetDebugFromLocal(isDebugFromLocal);
    bool resultOne = appRecord->GetDebugFromLocal();
    EXPECT_EQ(isDebugFromLocal, resultOne);
    isDebugFromLocal = true;
    appRecord->SetDebugFromLocal(isDebugFromLocal);
    bool resultTwo = appRecord->GetDebugFromLocal();
    EXPECT_EQ(isDebugFromLocal, resultTwo);
}

/**
 * @tc.name: AppRunningRecord_IsSupportMultiProcessDeviceFeature_0100
 * @tc.desc: Test IsSupportMultiProcessDeviceFeature works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_IsSupportMultiProcessDeviceFeature_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_IsSupportMultiProcessDeviceFeature_0100 called.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    EXPECT_NE(appRecord, nullptr);
    
    appRecord->SetSupportMultiProcessDeviceFeature(true);
    auto support = appRecord->IsSupportMultiProcessDeviceFeature();
    EXPECT_TRUE(support.value());

    appRecord->SetSupportMultiProcessDeviceFeature(false);
    support = appRecord->IsSupportMultiProcessDeviceFeature();
    EXPECT_FALSE(support.value());
}

/**
 * @tc.name: AppRunningRecord_IsLastAbilityByFlag_0100
 * @tc.desc: Test IsLastAbilityByFlag works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_IsLastAbilityByFlag_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_IsLastAbilityByFlag_0100 start.");
    auto appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    appRecord->isPrepareExit_ = false;
    bool ret = appRecord->IsLastAbilityByFlag(nullptr, true);
    EXPECT_FALSE(ret);
    ret = appRecord->IsLastAbilityByFlag(nullptr, false);
    EXPECT_FALSE(ret);

    appRecord->isPrepareExit_ = true;
    ret = appRecord->IsLastAbilityByFlag(nullptr, true);
    EXPECT_FALSE(ret);
    ret = appRecord->IsLastAbilityByFlag(nullptr, false);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_IsLastAbilityByFlag_0100 end.");
}

/**
 * @tc.name: AppRunningRecord_SetStartupTaskData_0100
 * @tc.desc: Test SetStartupTaskData works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_SetStartupTaskData_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_SetStartupTaskData_0100 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    EXPECT_NE(appRecord, nullptr);
    
    AAFwk::Want want;
    appRecord->SetStartupTaskData(want);
    EXPECT_EQ(appRecord->startupTaskData_->insightIntentName, "");

    std::string param("intentName1");
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME, param);
    appRecord->SetStartupTaskData(want);
    EXPECT_EQ(appRecord->startupTaskData_->insightIntentName, "intentName1");
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_SetStartupTaskData_0100 end.");
}

/**
 * @tc.name: AppRunningRecord_IsLastAbilityRecord_0100
 * @tc.desc: Test IsLastAbilityRecord works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_IsLastAbilityRecord_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_IsLastAbilityRecord_0100 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    ASSERT_NE(appRecord, nullptr);
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    bool ret = appRecord->IsLastAbilityRecord(token);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_IsLastAbilityRecord_0100 end.");
}

/**
 * @tc.name: AppRunningRecord_IsLastAbilityRecord_0200
 * @tc.desc: Test IsLastAbilityRecord works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_IsLastAbilityRecord_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_IsLastAbilityRecord_0200 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    ASSERT_NE(appRecord, nullptr);
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    appRecord->specifiedAbilityRequest_ = std::make_shared<SpecifiedRequest>();
    bool ret = appRecord->IsLastAbilityRecord(token);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_IsLastAbilityRecord_0200 end.");
}


/**
 * @tc.name: AppRunningRecord_IsLastAbilityRecord_0300
 * @tc.desc: Test IsLastAbilityRecord works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_IsLastAbilityRecord_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_IsLastAbilityRecord_0300 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    ASSERT_NE(appRecord, nullptr);
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    appRecord->specifiedAbilityRequest_ = nullptr;
    bool ret = appRecord->IsLastAbilityRecord(token);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_IsLastAbilityRecord_0300 end.");
}

/**
 * @tc.name: AppRunningRecord_ExtensionAbilityRecordExists_0100
 * @tc.desc: Test ExtensionAbilityRecordExists works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_ExtensionAbilityRecordExists_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_ExtensionAbilityRecordExists_0100 start.");
    std::shared_ptr<ApplicationInfo> info = nullptr;
    int32_t recordId = 0;
    std::string processName = "appRunningRecordProcessName";
    auto appRunningRecord = std::make_shared<AppRunningRecord>(info, recordId, processName);
    ASSERT_NE(appRunningRecord, nullptr);
    bool ret = appRunningRecord->ExtensionAbilityRecordExists();
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_IsLastAbilityRecord_0100 end.");
}

/**
 * @tc.name: AppRunningRecord_ExtensionAbilityRecordExists_0200
 * @tc.desc: Test ExtensionAbilityRecordExists works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_ExtensionAbilityRecordExists_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_ExtensionAbilityRecordExists_0200 start.");
    std::shared_ptr<ApplicationInfo> info = nullptr;
    int32_t recordId = 0;
    std::string processName = "appRunningRecordProcessName";
    auto appRunningRecord = std::make_shared<AppRunningRecord>(info, recordId, processName);

    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<ModuleRunningRecord> moduleRunningRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = nullptr;
    auto abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 0);

    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    want->SetParam(AAFwk::Want::PARAM_RESV_WINDOW_MODE,
        AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING);
    abilityRecord->SetWant(want);
    moduleRunningRecord->abilities_.emplace(nullptr, abilityRecord);

    std::vector<std::shared_ptr<ModuleRunningRecord>> hapModulesVector;
    hapModulesVector.emplace_back(moduleRunningRecord);
    std::string hapModulesString = "hapModulesString";
    appRunningRecord->hapModules_.emplace(hapModulesString, hapModulesVector);
    bool ret = appRunningRecord->ExtensionAbilityRecordExists();
    EXPECT_TRUE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_ExtensionAbilityRecordExists_0200 end.");
}

/**
 * @tc.name: AppRunningRecord_IsLastPageAbilityRecord_0100
 * @tc.desc: Test IsLastPageAbilityRecord works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_IsLastPageAbilityRecord_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_IsLastPageAbilityRecord_0100 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    ASSERT_NE(appRecord, nullptr);
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    bool ret = appRecord->IsLastPageAbilityRecord(token);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_IsLastPageAbilityRecord_0100 end.");
}

/**
 * @tc.name: AppRunningRecord_IsLastPageAbilityRecord_0200
 * @tc.desc: Test IsLastPageAbilityRecord works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_IsLastPageAbilityRecord_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_IsLastPageAbilityRecord_0200 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    ASSERT_NE(appRecord, nullptr);
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    appRecord->specifiedAbilityRequest_ = std::make_shared<SpecifiedRequest>();
    bool ret = appRecord->IsLastPageAbilityRecord(token);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_IsLastPageAbilityRecord_0200 end.");
}

/**
 * @tc.name: AppRunningRecord_IsLastPageAbilityRecord_0300
 * @tc.desc: Test IsLastPageAbilityRecord works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_IsLastPageAbilityRecord_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_IsLastPageAbilityRecord_0300 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    ASSERT_NE(appRecord, nullptr);
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    appRecord->specifiedAbilityRequest_ = nullptr;
    bool ret = appRecord->IsLastPageAbilityRecord(token);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_IsLastPageAbilityRecord_0300 end.");
}

/**
 * @tc.name: AppRunningRecord_GetBundleNames_0100
 * @tc.desc: Test GetBundleNames works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_GetBundleNames_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_GetBundleNames_0100 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    ASSERT_NE(appRecord, nullptr);
    appRecord->appInfos_.emplace(appInfo->bundleName, appInfo);
    std::vector<std::string> bundleNames;
    appRecord->GetBundleNames(bundleNames);
    EXPECT_EQ(bundleNames.size(), 1);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_GetBundleNames_0100 end.");
}

/**
 * @tc.name: AppRunningRecord_SetScheduleNewProcessRequestState_0100
 * @tc.desc: Test SetScheduleNewProcessRequestState works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_SetScheduleNewProcessRequestState_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_SetScheduleNewProcessRequestState_0100 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    ASSERT_NE(appRecord, nullptr);
    int32_t requestId = 100;
    AAFwk::Want want;
    std::string moduleName = "com.example.module";
    appRecord->SetScheduleNewProcessRequestState(requestId, want, moduleName);
    ASSERT_NE(appRecord->specifiedProcessRequest_, nullptr);
    EXPECT_EQ(appRecord->specifiedProcessRequest_->requestId, requestId);
    EXPECT_EQ(appRecord->moduleName_, moduleName);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_SetScheduleNewProcessRequestState_0100 end.");
}

/**
 * @tc.name: AppRunningRecord_IsNewProcessRequest_0100
 * @tc.desc: Test IsNewProcessRequest works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_IsNewProcessRequest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_IsNewProcessRequest_0100 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    ASSERT_NE(appRecord, nullptr);
    appRecord->specifiedProcessRequest_ = std::make_shared<SpecifiedRequest>();
    bool ret  = appRecord->IsNewProcessRequest();
    EXPECT_TRUE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_IsNewProcessRequest_0100 end.");
}

/**
 * @tc.name: AppRunningRecord_IsStartSpecifiedAbility_0100
 * @tc.desc: Test IsStartSpecifiedAbility works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_IsStartSpecifiedAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_IsStartSpecifiedAbility_0100 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    ASSERT_NE(appRecord, nullptr);
    appRecord->specifiedAbilityRequest_ = std::make_shared<SpecifiedRequest>();
    bool ret  = appRecord->IsStartSpecifiedAbility();
    EXPECT_TRUE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_IsStartSpecifiedAbility_0100 end.");
}

/**
 * @tc.name: AppRunningRecord_GetNewProcessRequestWant_0100
 * @tc.desc: Test GetNewProcessRequestWant works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_GetNewProcessRequestWant_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_GetNewProcessRequestWant_0100 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    ASSERT_NE(appRecord, nullptr);
    appRecord->specifiedProcessRequest_ = std::make_shared<SpecifiedRequest>();
    AAFwk::Want want;
    want.SetParam(AAFwk::Want::PARAM_RESV_WINDOW_MODE,
        AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING);
    appRecord->specifiedProcessRequest_->want = want;
    auto retWant = appRecord->GetNewProcessRequestWant();
    auto ret = retWant.GetIntParam(Want::PARAM_RESV_WINDOW_MODE, -1);
    EXPECT_EQ(ret, AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_GetNewProcessRequestWant_0100 end.");
}

/**
 * @tc.name: AppRunningRecord_GetNewProcessRequestWant_0200
 * @tc.desc: Test GetNewProcessRequestWant works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_GetNewProcessRequestWant_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_GetNewProcessRequestWant_0200 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    ASSERT_NE(appRecord, nullptr);
    appRecord->specifiedProcessRequest_ = nullptr;
    auto retWant  = appRecord->GetNewProcessRequestWant();
    auto ret = retWant.GetIntParam(Want::PARAM_RESV_WINDOW_MODE, -1);
    EXPECT_EQ(ret, -1);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_GetNewProcessRequestWant_0200 end.");
}

/**
 * @tc.name: AppRunningRecord_GetNewProcessRequestId_0100
 * @tc.desc: Test GetNewProcessRequestId works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_GetNewProcessRequestId_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_GetNewProcessRequestId_0100 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    ASSERT_NE(appRecord, nullptr);
    appRecord->specifiedProcessRequest_ = std::make_shared<SpecifiedRequest>();
    appRecord->specifiedProcessRequest_->requestId = 0;
    int32_t result  = appRecord->GetNewProcessRequestId();
    EXPECT_EQ(result, 0);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_GetNewProcessRequestId_0100 end.");
}

/**
 * @tc.name: AppRunningRecord_GetNewProcessRequestId_0200
 * @tc.desc: Test GetNewProcessRequestId works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_GetNewProcessRequestId_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_GetNewProcessRequestId_0200 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    ASSERT_NE(appRecord, nullptr);
    appRecord->specifiedProcessRequest_ = nullptr;
    int32_t result  = appRecord->GetNewProcessRequestId();
    EXPECT_EQ(result, -1);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_GetNewProcessRequestId_0200 end.");
}

/**
 * @tc.name: AppRunningRecord_IsDebug_0100
 * @tc.desc: Test IsDebug works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_IsDebug_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_IsDebug_0100 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    ASSERT_NE(appRecord, nullptr);
    appRecord->isDebugApp_ = true;
    appRecord->isNativeDebug_ = false;
    appRecord->perfCmd_ = "";
    appRecord->isAttachDebug_ = false;
    appRecord->isAssertPause_ = false;
    bool ret  = appRecord->IsDebug();
    EXPECT_TRUE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_IsDebug_0100 end.");
}

/**
 * @tc.name: AppRunningRecord_IsDebug_0100
 * @tc.desc: Test IsDebug works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_IsDebug_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_IsDebug_0200 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    ASSERT_NE(appRecord, nullptr);
    appRecord->isDebugApp_ = false;
    appRecord->isNativeDebug_ = true;
    appRecord->perfCmd_ = "";
    appRecord->isAttachDebug_ = false;
    appRecord->isAssertPause_ = false;
    bool ret  = appRecord->IsDebug();
    EXPECT_TRUE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_IsDebug_0200 end.");
}

/**
 * @tc.name: AppRunningRecord_IsDebug_0300
 * @tc.desc: Test IsDebug works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_IsDebug_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_IsDebug_0300 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    ASSERT_NE(appRecord, nullptr);
    appRecord->isDebugApp_ = false;
    appRecord->isNativeDebug_ = false;
    appRecord->perfCmd_ = "test";
    appRecord->isAttachDebug_ = false;
    appRecord->isAssertPause_ = false;
    bool ret  = appRecord->IsDebug();
    EXPECT_TRUE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_IsDebug_0300 end.");
}

/**
 * @tc.name: AppRunningRecord_IsDebug_0400
 * @tc.desc: Test IsDebug works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_IsDebug_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_IsDebug_0400 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    ASSERT_NE(appRecord, nullptr);
    appRecord->isDebugApp_ = false;
    appRecord->isNativeDebug_ = false;
    appRecord->perfCmd_ = "";
    appRecord->isAttachDebug_ = true;
    appRecord->isAssertPause_ = false;
    bool ret  = appRecord->IsDebug();
    EXPECT_TRUE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_IsDebug_0400 end.");
}

/**
 * @tc.name: AppRunningRecord_IsDebug_0500
 * @tc.desc: Test IsDebug works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_IsDebug_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_IsDebug_0500 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    ASSERT_NE(appRecord, nullptr);
    appRecord->isDebugApp_ = false;
    appRecord->isNativeDebug_ = false;
    appRecord->perfCmd_ = "";
    appRecord->isAttachDebug_ = false;
    appRecord->isAssertPause_ = true;
    bool ret  = appRecord->IsDebug();
    EXPECT_TRUE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_IsDebug_0500 end.");
}

/**
 * @tc.name: AppRunningRecord_IsDebug_0600
 * @tc.desc: Test IsDebug works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_IsDebug_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_IsDebug_0600 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    ASSERT_NE(appRecord, nullptr);
    appRecord->isDebugApp_ = false;
    appRecord->isNativeDebug_ = false;
    appRecord->perfCmd_ = "";
    appRecord->isAttachDebug_ = false;
    appRecord->isAssertPause_ = false;
    bool ret  = appRecord->IsDebug();
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_IsDebug_0600 end.");
}

/**
 * @tc.name: AppRunningRecord_IsNWebPreload_0100
 * @tc.desc: Test IsNWebPreload works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_IsNWebPreload_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_IsNWebPreload_0100 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    ASSERT_NE(appRecord, nullptr);
    appRecord->isAllowedNWebPreload_ = true;
    bool ret  = appRecord->IsNWebPreload();
    EXPECT_TRUE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_IsNWebPreload_0100 end.");
}

/**
 * @tc.name: AppRunningRecord_GetNeedLimitPrio_0100
 * @tc.desc: Test GetNeedLimitPrio works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_GetNeedLimitPrio_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_GetNeedLimitPrio_0100 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    ASSERT_NE(appRecord, nullptr);
    appRecord->isNeedLimitPrio_ = true;
    bool ret  = appRecord->GetNeedLimitPrio();
    EXPECT_TRUE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_GetNeedLimitPrio_0100 end.");
}

/**
 * @tc.name: AppRunningRecord_GetSignCode_0100
 * @tc.desc: Test GetSignCode works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_GetSignCode_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_GetSignCode_0100 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    ASSERT_NE(appRecord, nullptr);
    std::string signCodeString = "testSignCode";
    appRecord->signCode_ = signCodeString;
    std::string result  = appRecord->GetSignCode();
    EXPECT_EQ(result, signCodeString);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_GetSignCode_0100 end.");
}

/**
 * @tc.name: AppRunningRecord_SetNeedLimitPrio_0100
 * @tc.desc: Test SetNeedLimitPrio works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_SetNeedLimitPrio_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_SetNeedLimitPrio_0100 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    ASSERT_NE(appRecord, nullptr);
    appRecord->SetNeedLimitPrio(true);
    EXPECT_TRUE(appRecord->isNeedLimitPrio_);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_SetNeedLimitPrio_0100 end.");
}

/**
 * @tc.name: AppRunningRecord_SetJointUserId_0100
 * @tc.desc: Test SetJointUserId works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_SetJointUserId_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_SetJointUserId_0100 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    ASSERT_NE(appRecord, nullptr);
    std::string jointUserId = "testJointUserId";
    appRecord->SetJointUserId(jointUserId);
    EXPECT_EQ(appRecord->jointUserId_, jointUserId);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_SetJointUserId_0100 end.");
}

/**
 * @tc.name: AppRunningRecord_GetUserId_0100
 * @tc.desc: Test GetUserId works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_GetUserId_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_GetUserId_0100 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    ASSERT_NE(appRecord, nullptr);
    appRecord->mainUid_ = BASE_USER_RANGE;
    int32_t result = appRecord->GetUserId();
    EXPECT_EQ(result, 1);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_GetUserId_0100 end.");
}

/**
 * @tc.name: AppRunningRecord_UpdateApplicationInfoInstalled_0100
 * @tc.desc: Test UpdateApplicationInfoInstalled works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_UpdateApplicationInfoInstalled_0100, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    ASSERT_NE(appInfo, nullptr);
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    ASSERT_NE(appRecord, nullptr);
    appRecord->isStageBasedModel_ = true;
    appRecord->appLifeCycleDeal_ == nullptr;
    std::string moduleName = "moduleName";
    ApplicationInfo applicationInfo;
    appRecord->UpdateApplicationInfoInstalled(applicationInfo, moduleName);
    EXPECT_TRUE(applicationInfo.name.empty());
}

/**
 * @tc.name: AppRunningRecord_UpdateApplicationInfoInstalled_0200
 * @tc.desc: Test UpdateApplicationInfoInstalled works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_UpdateApplicationInfoInstalled_0200, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    ASSERT_NE(appInfo, nullptr);
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    ASSERT_NE(appRecord, nullptr);
    appRecord->isStageBasedModel_ = true;
    appRecord->appLifeCycleDeal_ = std::make_shared<AppLifeCycleDeal>();
    std::string moduleName = "moduleName";
    ApplicationInfo applicationInfo;
    appRecord->UpdateApplicationInfoInstalled(applicationInfo, moduleName);
    EXPECT_TRUE(applicationInfo.name.empty());
}

/**
 * @tc.name: AppRunningRecord_SetModuleLoaded_0100
 * @tc.desc: Test SetModuleLoaded works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_SetModuleLoaded_0100, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    ASSERT_NE(appInfo, nullptr);
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    ASSERT_NE(appRecord, nullptr);
    std::string moduleName = "moduleName";
    std::vector<std::shared_ptr<ModuleRunningRecord>> moduleRecords;
    std::shared_ptr<ModuleRunningRecord> moduleRunningRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);
    moduleRunningRecord->moduleName_ = moduleName;
    moduleRecords.emplace_back(moduleRunningRecord);
    appRecord->hapModules_.emplace("key", moduleRecords);
    appRecord->SetModuleLoaded(moduleName);
    EXPECT_TRUE(moduleRunningRecord->IsLoaded());
}

/**
 * @tc.name: AppRunningRecord_ScheduleForegroundRunning_0100
 * @tc.desc: Test ScheduleForegroundRunning works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_ScheduleForegroundRunning_0100, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    ASSERT_NE(appInfo, nullptr);
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    ASSERT_NE(appRecord, nullptr);
    appRecord->appLifeCycleDeal_ = std::make_shared<AppLifeCycleDeal>();
    appRecord->appLifeCycleDeal_->SetApplicationClient(nullptr);
    appRecord->ScheduleTerminate();
    appRecord->ScheduleClearPageStack();
    EXPECT_FALSE(appRecord->ScheduleForegroundRunning());
}

/**
 * @tc.name: AppRunningRecord_StateChangedNotifyObserver_0100
 * @tc.desc: Test StateChangedNotifyObserver works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_StateChangedNotifyObserver_0100, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    appInfo->multiAppMode.multiAppModeType = MultiAppModeType::APP_CLONE;
    ASSERT_NE(appInfo, nullptr);
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    ASSERT_NE(appRecord, nullptr);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    ASSERT_NE(abilityInfo, nullptr);
    sptr<IRemoteObject> token = nullptr;
    auto abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 0);
    ASSERT_NE(abilityRecord, nullptr);
    appRecord->StateChangedNotifyObserver(abilityRecord, 1, true, true);
    auto applicationInfo = appRecord->GetApplicationInfo();
    EXPECT_EQ(applicationInfo->multiAppMode.multiAppModeType, MultiAppModeType::APP_CLONE);
}

/**
 * @tc.name: AppRunningRecord_StateChangedNotifyObserver_0200
 * @tc.desc: Test StateChangedNotifyObserver works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_StateChangedNotifyObserver_0200, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    appInfo->multiAppMode.multiAppModeType = MultiAppModeType::APP_CLONE;
    ASSERT_NE(appInfo, nullptr);
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    ASSERT_NE(appRecord, nullptr);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    ASSERT_NE(abilityInfo, nullptr);
    sptr<IRemoteObject> token = nullptr;
    auto abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 0);
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    abilityRecord->SetWant(want);
    ASSERT_NE(abilityRecord, nullptr);
    appRecord->StateChangedNotifyObserver(abilityRecord, 1, true, true);
    auto applicationInfo = appRecord->GetApplicationInfo();
    EXPECT_EQ(applicationInfo->multiAppMode.multiAppModeType, MultiAppModeType::APP_CLONE);
}

/**
 * @tc.name: AppRunningRecord_StateChangedNotifyObserver_0300
 * @tc.desc: Test StateChangedNotifyObserver works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_StateChangedNotifyObserver_0300, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    appInfo->multiAppMode.multiAppModeType = MultiAppModeType::APP_CLONE;
    appInfo->bundleType = AppExecFwk::BundleType::ATOMIC_SERVICE;
    ASSERT_NE(appInfo, nullptr);
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    ASSERT_NE(appRecord, nullptr);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    ASSERT_NE(abilityInfo, nullptr);
    sptr<IRemoteObject> token = nullptr;
    auto abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 0);
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    abilityRecord->SetWant(want);
    ASSERT_NE(abilityRecord, nullptr);
    appRecord->StateChangedNotifyObserver(abilityRecord, 1, true, true);
    auto applicationInfo = appRecord->GetApplicationInfo();
    EXPECT_EQ(applicationInfo->multiAppMode.multiAppModeType, MultiAppModeType::APP_CLONE);
    EXPECT_EQ(applicationInfo->bundleType, AppExecFwk::BundleType::ATOMIC_SERVICE);
}

/**
 * @tc.name: AppRunningRecord_StateChangedNotifyObserver_0400
 * @tc.desc: Test StateChangedNotifyObserver works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_StateChangedNotifyObserver_0400, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    appInfo->multiAppMode.multiAppModeType = MultiAppModeType::APP_CLONE;
    appInfo->bundleType = AppExecFwk::BundleType::ATOMIC_SERVICE;
    ASSERT_NE(appInfo, nullptr);
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    ASSERT_NE(appRecord, nullptr);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    ASSERT_NE(abilityInfo, nullptr);
    abilityInfo->extensionAbilityType = AppExecFwk::ExtensionAbilityType::SHARE;
    sptr<IRemoteObject> token = nullptr;
    auto abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 0);
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    abilityRecord->SetWant(want);
    ASSERT_NE(abilityRecord, nullptr);
    appRecord->StateChangedNotifyObserver(abilityRecord, 1, true, true);
    auto applicationInfo = appRecord->GetApplicationInfo();
    EXPECT_EQ(applicationInfo->multiAppMode.multiAppModeType, MultiAppModeType::APP_CLONE);
    EXPECT_EQ(applicationInfo->bundleType, AppExecFwk::BundleType::ATOMIC_SERVICE);
}

/**
 * @tc.name: AppRunningRecord_StateChangedNotifyObserver_0500
 * @tc.desc: Test StateChangedNotifyObserver works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_StateChangedNotifyObserver_0500, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    appInfo->multiAppMode.multiAppModeType = MultiAppModeType::APP_CLONE;
    appInfo->bundleType = AppExecFwk::BundleType::ATOMIC_SERVICE;
    ASSERT_NE(appInfo, nullptr);
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    ASSERT_NE(appRecord, nullptr);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    ASSERT_NE(abilityInfo, nullptr);
    abilityInfo->extensionAbilityType = AppExecFwk::ExtensionAbilityType::SHARE;
    sptr<IRemoteObject> token = nullptr;
    auto abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 0);
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    abilityRecord->SetWant(want);
    ASSERT_NE(abilityRecord, nullptr);
    appRecord->StateChangedNotifyObserver(abilityRecord, 1, false, true);
    auto applicationInfo = appRecord->GetApplicationInfo();
    EXPECT_EQ(applicationInfo->multiAppMode.multiAppModeType, MultiAppModeType::APP_CLONE);
    EXPECT_EQ(applicationInfo->bundleType, AppExecFwk::BundleType::ATOMIC_SERVICE);
}

/**
 * @tc.name: AppRunningRecord_PopForegroundingAbilityTokens_0100
 * @tc.desc: Test PopForegroundingAbilityTokens works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_PopForegroundingAbilityTokens_0100, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    ASSERT_NE(appInfo, nullptr);
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    ASSERT_NE(appRecord, nullptr);
    appRecord->foregroundingAbilityTokens_.insert(nullptr);
    appRecord->PopForegroundingAbilityTokens();
    EXPECT_EQ(appRecord->foregroundingAbilityTokens_.size(), 0);
}

/**
 * @tc.name: AppRunningRecord_SendEvent_0300
 * @tc.desc: Test SendEvent works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_SendEvent_0300, TestSize.Level1)
{
    uint32_t msg = AMSEventHandler::START_PROCESS_SPECIFIED_ABILITY_TIMEOUT_MSG;
    int64_t timeOut = TIMEOUT;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    ASSERT_NE(appInfo, nullptr);
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    ASSERT_NE(appRunningRecord, nullptr);
    appRunningRecord->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    appRunningRecord->eventHandler_ =
        std::make_shared<AMSEventHandler>(appRunningRecord->taskHandler_, appRunningRecord->appMgrServiceInner_);
    appRunningRecord->isDebugApp_ = true;
    appRunningRecord->isNativeDebug_ = false;
    appRunningRecord->isAttachDebug_ = false;
    appRunningRecord->SendEvent(msg, timeOut);
    EXPECT_EQ(appRunningRecord->appEventId_, 2);
}

/**
 * @tc.name: AppRunningRecord_SendEvent_0400
 * @tc.desc: Test SendEvent works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_SendEvent_0400, TestSize.Level1)
{
    uint32_t msg = AMSEventHandler::START_PROCESS_SPECIFIED_ABILITY_TIMEOUT_MSG;
    int64_t timeOut = TIMEOUT;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    ASSERT_NE(appInfo, nullptr);
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    ASSERT_NE(appRunningRecord, nullptr);
    appRunningRecord->appEventId_ = 0;
    appRunningRecord->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    appRunningRecord->eventHandler_ =
        std::make_shared<AMSEventHandler>(appRunningRecord->taskHandler_, appRunningRecord->appMgrServiceInner_);
    appRunningRecord->isDebugApp_ = false;
    appRunningRecord->isNativeDebug_ = true;
    appRunningRecord->isAttachDebug_ = false;
    appRunningRecord->SendEvent(msg, timeOut);
    EXPECT_EQ(appRunningRecord->appEventId_, 0);
}

/**
 * @tc.name: AppRunningRecord_SendEvent_0500
 * @tc.desc: Test SendEvent works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_SendEvent_0500, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    ASSERT_NE(appInfo, nullptr);
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    ASSERT_NE(appRunningRecord, nullptr);
    appRunningRecord->appEventId_ = 0;
    appRunningRecord->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    appRunningRecord->eventHandler_ =
        std::make_shared<AMSEventHandler>(appRunningRecord->taskHandler_, appRunningRecord->appMgrServiceInner_);
    appRunningRecord->isDebugApp_ = false;
    appRunningRecord->isNativeDebug_ = false;
    appRunningRecord->isAttachDebug_ = true;
    uint32_t msg = AMSEventHandler::START_PROCESS_SPECIFIED_ABILITY_TIMEOUT_MSG;
    int64_t timeOut = TIMEOUT;
    appRunningRecord->SendEvent(msg, timeOut);
    EXPECT_EQ(appRunningRecord->appEventId_, 0);
}

/**
 * @tc.name: AppRunningRecord_OnWindowVisibilityChangedWithPendingState_0100
 * @tc.desc: Test OnWindowVisibilityChangedWithPendingState works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_OnWindowVisibilityChangedWithPendingState_0100, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    ASSERT_NE(appInfo, nullptr);
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    ASSERT_NE(appRunningRecord, nullptr);
    appRunningRecord->pendingState_ == ApplicationPendingState::FOREGROUNDING;
    appRunningRecord->hapModules_.clear();
    appRunningRecord->windowIds_.clear();
    appRunningRecord->foregroundingAbilityTokens_.clear();
    appRunningRecord->OnWindowVisibilityChangedWithPendingState();
    EXPECT_EQ(appRunningRecord->pendingState_, ApplicationPendingState::READY);
}

/**
 * @tc.name: AppRunningRecord_OnWindowVisibilityChangedWithPendingState_0200
 * @tc.desc: Test OnWindowVisibilityChangedWithPendingState works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_OnWindowVisibilityChangedWithPendingState_0200, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    ASSERT_NE(appInfo, nullptr);
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    ASSERT_NE(appRunningRecord, nullptr);
    appRunningRecord->pendingState_ == ApplicationPendingState::BACKGROUNDING;
    appRunningRecord->windowIds_.emplace(100);
    appRunningRecord->OnWindowVisibilityChangedWithPendingState();
    EXPECT_EQ(appRunningRecord->pendingState_, ApplicationPendingState::FOREGROUNDING);
}

/**
 * @tc.name: AppRunningRecord_OnWindowVisibilityChangedWithPendingState_0300
 * @tc.desc: Test OnWindowVisibilityChangedWithPendingState works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_OnWindowVisibilityChangedWithPendingState_0300, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    ASSERT_NE(appInfo, nullptr);
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    ASSERT_NE(appRunningRecord, nullptr);
    appRunningRecord->pendingState_ == ApplicationPendingState::READY;
    appRunningRecord->curState_ == ApplicationState::APP_STATE_FOREGROUND;
    appRunningRecord->hapModules_.clear();
    appRunningRecord->OnWindowVisibilityChangedWithPendingState();
    EXPECT_EQ(appRunningRecord->pendingState_, ApplicationPendingState::READY);
}

/**
 * @tc.name: AppRunningRecord_OnWindowVisibilityChangedWithPendingState_0400
 * @tc.desc: Test OnWindowVisibilityChangedWithPendingState works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_OnWindowVisibilityChangedWithPendingState_0400, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    ASSERT_NE(appInfo, nullptr);
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    ASSERT_NE(appRunningRecord, nullptr);
    appRunningRecord->pendingState_ == ApplicationPendingState::READY;
    appRunningRecord->OnWindowVisibilityChangedWithPendingState();
    EXPECT_EQ(appRunningRecord->pendingState_, ApplicationPendingState::READY);
}

/**
 * @tc.name: AppRunningRecord_OnWindowVisibilityChangedWithPendingState_0500
 * @tc.desc: Test OnWindowVisibilityChangedWithPendingState works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_OnWindowVisibilityChangedWithPendingState_0500, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    ASSERT_NE(appInfo, nullptr);
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    ASSERT_NE(appRunningRecord, nullptr);
    appRunningRecord->pendingState_ == ApplicationPendingState::READY;
    appRunningRecord->watchdogVisibilityState_ = WatchdogVisibilityState::WATCHDOG_STATE_UNVISIBILITY;
    appRunningRecord->OnWindowVisibilityChangedWithPendingState();
    EXPECT_EQ(appRunningRecord->pendingState_, ApplicationPendingState::READY);
    EXPECT_EQ(appRunningRecord->watchdogVisibilityState_, WatchdogVisibilityState::WATCHDOG_STATE_UNVISIBILITY);
}

/**
 * @tc.name: AppRunningRecord_OnWindowVisibilityChangedWithPendingState_0600
 * @tc.desc: Test OnWindowVisibilityChangedWithPendingState works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_OnWindowVisibilityChangedWithPendingState_0600, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    ASSERT_NE(appInfo, nullptr);
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    ASSERT_NE(appRunningRecord, nullptr);
    appRunningRecord->pendingState_ == ApplicationPendingState::READY;
    appRunningRecord->watchdogVisibilityState_ = WatchdogVisibilityState::WATCHDOG_STATE_VISIBILITY;
    appRunningRecord->OnWindowVisibilityChangedWithPendingState();
    EXPECT_EQ(appRunningRecord->pendingState_, ApplicationPendingState::READY);
    EXPECT_EQ(appRunningRecord->watchdogVisibilityState_, WatchdogVisibilityState::WATCHDOG_STATE_UNVISIBILITY);
}

/**
 * @tc.name: AppRunningRecord_SetPreForeground_0100
 * @tc.desc: Test SetPreForeground works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_SetPreForeground_0100, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    ASSERT_NE(appInfo, nullptr);
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    ASSERT_NE(appRunningRecord, nullptr);

    appRunningRecord->SetPreForeground(true);
    EXPECT_EQ(appRunningRecord->IsPreForeground(), true);

    appRunningRecord->SetPreForeground(false);
    EXPECT_EQ(appRunningRecord->IsPreForeground(), false);
}

/**
 * @tc.name: AppRunningRecord_SetProcessType_0001
 * @tc.desc: Test SetProcessType works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_SetProcessType_0001, TestSize.Level1)
{
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    ASSERT_NE(abilityInfo, nullptr);
    abilityInfo->extensionAbilityType = ExtensionAbilityType::UNSPECIFIED;
    abilityInfo->type = AbilityType::SERVICE;

    std::shared_ptr<ApplicationInfo> info = nullptr;
    int32_t recordId = 0;
    std::string processName = "TestSetProcessType";
    auto appRunningRecord = std::make_shared<AppRunningRecord>(info, recordId, processName);
    appRunningRecord->SetProcessAndExtensionType(abilityInfo, 0);
    ExtensionAbilityType extensionAbilityType = appRunningRecord->GetExtensionType();
    ProcessType processType = appRunningRecord->GetProcessType();
    EXPECT_EQ(extensionAbilityType, ExtensionAbilityType::SERVICE);
    EXPECT_EQ(processType, ProcessType::EXTENSION);

    appRunningRecord->SetProcessType(ProcessType::NORMAL);
    ProcessType normalType = appRunningRecord->GetProcessType();
    EXPECT_EQ(normalType, ProcessType::NORMAL);
}

/**
 * @tc.name: AppRunningRecord_DumpArkWeb_0100
 * @tc.desc: Test DumpArkWeb works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_DumpArkWeb_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_DumpArkWeb_0100 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::string processName;
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, processName);
    ASSERT_NE(appRecord, nullptr);

    appRecord->appLifeCycleDeal_ == nullptr;
    std::string customArgs;
    std::string result;
    auto ret = appRecord->DumpArkWeb(customArgs, result);
    EXPECT_EQ(ret, DumpErrorCode::ERR_INTERNAL_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_DumpArkWeb_0100 end.");
}

/**
 * @tc.name: AppRunningRecord_DumpArkWeb_0200
 * @tc.desc: Test DumpArkWeb works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_DumpArkWeb_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_DumpArkWeb_0200 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::string processName;
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, processName);
    ASSERT_NE(appRecord, nullptr);

    appRecord->SetApplicationClient(nullptr);
    std::string customArgs;
    std::string result;
    auto ret = appRecord->DumpArkWeb(customArgs, result);
    std::string result2;
    auto lifeCycleRet = appRecord->appLifeCycleDeal_->DumpArkWeb(customArgs, result2);
    EXPECT_EQ(ret, lifeCycleRet);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_DumpArkWeb_0200 end.");
}

/**
 * @tc.name: AppRunningRecord_ShouldSkipTimeout_0100
 * @tc.desc: Test ShouldSkipTimeout works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_ShouldSkipTimeout_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_ShouldSkipTimeout_0100 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::string processName;
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, processName);
    ASSERT_NE(appRecord, nullptr);
    
    appRecord->SetDebugApp(true);
    appRecord->SetNativeDebug(false);
    appRecord->SetAttachDebug(false, false);
    auto ret = appRecord->ShouldSkipTimeout();
    EXPECT_EQ(ret, true);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_ShouldSkipTimeout_0100 end.");
}

/**
 * @tc.name: AppRunningRecord_ShouldSkipTimeout_0200
 * @tc.desc: Test ShouldSkipTimeout works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_ShouldSkipTimeout_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_ShouldSkipTimeout_0200 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::string processName;
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, processName);
    ASSERT_NE(appRecord, nullptr);
    
    appRecord->SetDebugApp(false);
    appRecord->SetNativeDebug(true);
    appRecord->SetAttachDebug(false, false);
    auto ret = appRecord->ShouldSkipTimeout();
    EXPECT_EQ(ret, true);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_ShouldSkipTimeout_0200 end.");
}

/**
 * @tc.name: AppRunningRecord_ShouldSkipTimeout_0300
 * @tc.desc: Test ShouldSkipTimeout works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_ShouldSkipTimeout_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_ShouldSkipTimeout_0300 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::string processName;
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, processName);
    ASSERT_NE(appRecord, nullptr);
    
    appRecord->SetDebugApp(false);
    appRecord->SetNativeDebug(false);
    appRecord->SetAttachDebug(true, false);
    auto ret = appRecord->ShouldSkipTimeout();
    EXPECT_EQ(ret, true);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_ShouldSkipTimeout_0300 end.");
}

/**
 * @tc.name: AppRunningRecord_ShouldSkipTimeout_0400
 * @tc.desc: Test ShouldSkipTimeout works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_ShouldSkipTimeout_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_ShouldSkipTimeout_0400 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::string processName;
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, processName);
    ASSERT_NE(appRecord, nullptr);
    
    appRecord->SetDebugApp(false);
    appRecord->SetNativeDebug(false);
    appRecord->SetAttachDebug(false, false);
    auto ret = appRecord->ShouldSkipTimeout();
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_ShouldSkipTimeout_0400 end.");
}

/**
 * @tc.name: AppRunningRecord_GetAppRunningUniqueId_0100
 * @tc.desc: Test GetAppRunningUniqueId returns non-zero value.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_GetAppRunningUniqueId_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_GetAppRunningUniqueId_0100 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::string processName;
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, processName);
    ASSERT_NE(appRecord, nullptr);
    
    uint64_t uniqueId = appRecord->GetAppRunningUniqueId();
    EXPECT_NE(uniqueId, 0);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_GetAppRunningUniqueId_0100 end.");
}

/**
 * @tc.name: AppRunningRecord_GetAppRunningUniqueId_0200
 * @tc.desc: Test GetAppRunningUniqueId returns consistent value.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_GetAppRunningUniqueId_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_GetAppRunningUniqueId_0200 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::string processName;
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, processName);
    ASSERT_NE(appRecord, nullptr);
    
    uint64_t firstCall = appRecord->GetAppRunningUniqueId();
    uint64_t secondCall = appRecord->GetAppRunningUniqueId();
    uint64_t thirdCall = appRecord->GetAppRunningUniqueId();
    
    EXPECT_EQ(firstCall, secondCall);
    EXPECT_EQ(secondCall, thirdCall);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_GetAppRunningUniqueId_0200 end.");
}

/**
 * @tc.name: AppRunningRecord_GenerateRunningUniqueId_Uniqueness_0100
 * @tc.desc: Test generated unique IDs are unique.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_GenerateRunningUniqueId_Uniqueness_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_GenerateRunningUniqueId_Uniqueness_0100 start.");
    std::set<uint64_t> uniqueIds;
    const int iterationCount = 100;
    
    for (int i = 0; i < iterationCount; i++) {
        std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
        std::string processName;
        auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, processName);
        ASSERT_NE(appRecord, nullptr);
        
        uint64_t uniqueId = appRecord->GetAppRunningUniqueId();
        uniqueIds.insert(uniqueId);
    }
    
    EXPECT_EQ(uniqueIds.size(), iterationCount);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_GenerateRunningUniqueId_Uniqueness_0100 end.");
}

/**
 * @tc.name: AppRunningRecord_GetAppStartTime_Unchanged_0100
 * @tc.desc: Test GetAppStartTime still works correctly.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_GetAppStartTime_Unchanged_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_GetAppStartTime_Unchanged_0100 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::string processName;
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, processName);
    ASSERT_NE(appRecord, nullptr);
    
    int64_t startTime = appRecord->GetAppStartTime();
    EXPECT_GT(startTime, 0);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_GetAppStartTime_Unchanged_0100 end.");
}

/**
 * @tc.name: AppRunningRecord_UniqueId_DifferentFromStartTime_0100
 * @tc.desc: Test unique ID is different from start time.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_UniqueId_DifferentFromStartTime_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_UniqueId_DifferentFromStartTime_0100 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::string processName;
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, processName);
    ASSERT_NE(appRecord, nullptr);
    
    uint64_t uniqueId = appRecord->GetAppRunningUniqueId();
    int64_t startTime = appRecord->GetAppStartTime();
    
    // Verify uniqueId is not accidentally equal to startTime
    // If uniqueId <= INT64_MAX and equals startTime, it indicates a problem with implementation
    EXPECT_TRUE(uniqueId > static_cast<uint64_t>(std::numeric_limits<int64_t>::max()) ||
                static_cast<int64_t>(uniqueId) != startTime);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_UniqueId_DifferentFromStartTime_0100 end.");
}

/**
 * @tc.name: AppRunningRecord_SetKillAppReason_0100
 * @tc.desc: Test SetKillAppReason works.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_SetKillAppReason_0100, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    ASSERT_NE(appInfo, nullptr);
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    ASSERT_NE(appRunningRecord, nullptr);

    appRunningRecord->SetKillId(1);
    EXPECT_EQ(appRunningRecord->GetKillId(), 1);
    std::string reason = "TestReason";
    appRunningRecord->SetKillMsg(reason);
    EXPECT_EQ(appRunningRecord->GetKillMsg(), reason);
    appRunningRecord->SetInnerMsg(reason);
    EXPECT_EQ(appRunningRecord->GetInnerMsg(), reason);
}

/**
 * @tc.name: GetAllAbilityInfos_0100
 * @tc.desc: Test GetAllAbilityInfos with empty ability list
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, GetAllAbilityInfos_0100, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    ASSERT_NE(appInfo, nullptr);
    int32_t recordId = RECORD_ID;
    std::string processName = "test.process";
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    ASSERT_NE(appRunningRecord, nullptr);
    appRunningRecord->GetPriorityObject()->SetPid(1001);

    std::vector<AppExecFwk::AbilityStateData> infos;
    appRunningRecord->GetAllAbilityInfos(infos);
    EXPECT_TRUE(infos.empty());
}

/**
 * @tc.name: GetAllAbilityInfos_0200
 * @tc.desc: Test GetAllAbilityInfos with empty ability list
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, GetAllAbilityInfos_0200, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    appInfo->bundleName = "com.test.bundle";
    ASSERT_NE(appInfo, nullptr);
    int32_t recordId = RECORD_ID;
    std::string processName = "com.test.bundle";
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    ASSERT_NE(appRunningRecord, nullptr);
    appRunningRecord->GetPriorityObject()->SetPid(1001);

    auto moduleRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);
    ASSERT_NE(moduleRecord, nullptr);
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::EXTENSION;
    abilityInfo->extensionAbilityType = ExtensionAbilityType::SERVICE;
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    ASSERT_NE(token, nullptr);
    ASSERT_NE(moduleRecord->AddAbility(token, abilityInfo, nullptr, RECORD_ID), nullptr);
    appRunningRecord->hapModules_[appInfo->bundleName].emplace_back(moduleRecord);
    std::vector<AppExecFwk::AbilityStateData> infos;
    appRunningRecord->GetAllAbilityInfos(infos);
    EXPECT_EQ(infos.size(), 1);
}

/**
 * @tc.name: AppRunningRecord_HasOnlyOneExtensionType_0100
 * @tc.desc: Test HasOnlyOneExtensionType with no abilities.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_HasOnlyOneExtensionType_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_HasOnlyOneExtensionType_0100 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "test.process");
    ASSERT_NE(appRecord, nullptr);

    bool result = appRecord->HasOnlyOneExtensionType();
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_HasOnlyOneExtensionType_0100 end.");
}

/**
 * @tc.name: AppRunningRecord_HasOnlyOneExtensionType_0200
 * @tc.desc: Test HasOnlyOneExtensionType with a single Page ability.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_HasOnlyOneExtensionType_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_HasOnlyOneExtensionType_0200 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "test.process");
    ASSERT_NE(appRecord, nullptr);

    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    auto abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 0);

    auto moduleRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);
    moduleRecord->abilities_.emplace(token, abilityRecord);
    appRecord->hapModules_["bundleName"].push_back(moduleRecord);

    bool result = appRecord->HasOnlyOneExtensionType();
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_HasOnlyOneExtensionType_0200 end.");
}

/**
 * @tc.name: AppRunningRecord_HasOnlyOneExtensionType_0300
 * @tc.desc: Test HasOnlyOneExtensionType with a single Extension ability.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_HasOnlyOneExtensionType_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_HasOnlyOneExtensionType_0300 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "test.process");
    ASSERT_NE(appRecord, nullptr);

    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::EXTENSION;
    abilityInfo->extensionAbilityType = ExtensionAbilityType::SERVICE;
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    auto abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 0);

    auto moduleRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);
    moduleRecord->abilities_.emplace(token, abilityRecord);
    appRecord->hapModules_["bundleName"].push_back(moduleRecord);

    bool result = appRecord->HasOnlyOneExtensionType();
    EXPECT_TRUE(result);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_HasOnlyOneExtensionType_0300 end.");
}

/**
 * @tc.name: AppRunningRecord_HasOnlyOneExtensionType_0400
 * @tc.desc: Test HasOnlyOneExtensionType with multiple Extensions of the same type.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_HasOnlyOneExtensionType_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_HasOnlyOneExtensionType_0400 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "test.process");
    ASSERT_NE(appRecord, nullptr);

    auto moduleRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);

    auto abilityInfo1 = std::make_shared<AbilityInfo>();
    abilityInfo1->type = AbilityType::EXTENSION;
    abilityInfo1->extensionAbilityType = ExtensionAbilityType::SERVICE;
    sptr<IRemoteObject> token1 = new (std::nothrow) MockAbilityToken();
    auto abilityRecord1 = std::make_shared<AbilityRunningRecord>(abilityInfo1, token1, 0);
    moduleRecord->abilities_.emplace(token1, abilityRecord1);

    auto abilityInfo2 = std::make_shared<AbilityInfo>();
    abilityInfo2->type = AbilityType::EXTENSION;
    abilityInfo2->extensionAbilityType = ExtensionAbilityType::SERVICE;
    sptr<IRemoteObject> token2 = new (std::nothrow) MockAbilityToken();
    auto abilityRecord2 = std::make_shared<AbilityRunningRecord>(abilityInfo2, token2, 0);
    moduleRecord->abilities_.emplace(token2, abilityRecord2);

    appRecord->hapModules_["bundleName"].push_back(moduleRecord);

    bool result = appRecord->HasOnlyOneExtensionType();
    EXPECT_TRUE(result);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_HasOnlyOneExtensionType_0400 end.");
}

/**
 * @tc.name: AppRunningRecord_HasOnlyOneExtensionType_0500
 * @tc.desc: Test HasOnlyOneExtensionType with Extensions of different types.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_HasOnlyOneExtensionType_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_HasOnlyOneExtensionType_0500 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "test.process");
    ASSERT_NE(appRecord, nullptr);

    auto moduleRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);

    auto abilityInfo1 = std::make_shared<AbilityInfo>();
    abilityInfo1->type = AbilityType::EXTENSION;
    abilityInfo1->extensionAbilityType = ExtensionAbilityType::SERVICE;
    sptr<IRemoteObject> token1 = new (std::nothrow) MockAbilityToken();
    auto abilityRecord1 = std::make_shared<AbilityRunningRecord>(abilityInfo1, token1, 0);
    moduleRecord->abilities_.emplace(token1, abilityRecord1);

    auto abilityInfo2 = std::make_shared<AbilityInfo>();
    abilityInfo2->type = AbilityType::EXTENSION;
    abilityInfo2->extensionAbilityType = ExtensionAbilityType::DATASHARE;
    sptr<IRemoteObject> token2 = new (std::nothrow) MockAbilityToken();
    auto abilityRecord2 = std::make_shared<AbilityRunningRecord>(abilityInfo2, token2, 0);
    moduleRecord->abilities_.emplace(token2, abilityRecord2);

    appRecord->hapModules_["bundleName"].push_back(moduleRecord);

    bool result = appRecord->HasOnlyOneExtensionType();
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_HasOnlyOneExtensionType_0500 end.");
}

/**
 * @tc.name: AppRunningRecord_HasOnlyOneExtensionType_0600
 * @tc.desc: Test HasOnlyOneExtensionType with a Page and an Extension.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_HasOnlyOneExtensionType_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_HasOnlyOneExtensionType_0600 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "test.process");
    ASSERT_NE(appRecord, nullptr);

    auto moduleRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);

    auto pageInfo = std::make_shared<AbilityInfo>();
    pageInfo->type = AbilityType::PAGE;
    sptr<IRemoteObject> pageToken = new (std::nothrow) MockAbilityToken();
    auto pageRecord = std::make_shared<AbilityRunningRecord>(pageInfo, pageToken, 0);
    moduleRecord->abilities_.emplace(pageToken, pageRecord);

    auto extInfo = std::make_shared<AbilityInfo>();
    extInfo->type = AbilityType::EXTENSION;
    extInfo->extensionAbilityType = ExtensionAbilityType::SERVICE;
    sptr<IRemoteObject> extToken = new (std::nothrow) MockAbilityToken();
    auto extRecord = std::make_shared<AbilityRunningRecord>(extInfo, extToken, 0);
    moduleRecord->abilities_.emplace(extToken, extRecord);

    appRecord->hapModules_["bundleName"].push_back(moduleRecord);

    bool result = appRecord->HasOnlyOneExtensionType();
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_HasOnlyOneExtensionType_0600 end.");
}

/**
 * @tc.name: AppRunningRecord_HasOnlyOneExtensionType_0700
 * @tc.desc: Test HasOnlyOneExtensionType with same-type Extensions and a Page (Page should cause false).
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_HasOnlyOneExtensionType_0700, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_HasOnlyOneExtensionType_0700 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "test.process");
    ASSERT_NE(appRecord, nullptr);

    auto moduleRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);

    auto extInfo1 = std::make_shared<AbilityInfo>();
    extInfo1->type = AbilityType::EXTENSION;
    extInfo1->extensionAbilityType = ExtensionAbilityType::SERVICE;
    sptr<IRemoteObject> extToken1 = new (std::nothrow) MockAbilityToken();
    auto extRecord1 = std::make_shared<AbilityRunningRecord>(extInfo1, extToken1, 0);
    moduleRecord->abilities_.emplace(extToken1, extRecord1);

    auto extInfo2 = std::make_shared<AbilityInfo>();
    extInfo2->type = AbilityType::EXTENSION;
    extInfo2->extensionAbilityType = ExtensionAbilityType::SERVICE;
    sptr<IRemoteObject> extToken2 = new (std::nothrow) MockAbilityToken();
    auto extRecord2 = std::make_shared<AbilityRunningRecord>(extInfo2, extToken2, 0);
    moduleRecord->abilities_.emplace(extToken2, extRecord2);

    auto pageInfo = std::make_shared<AbilityInfo>();
    pageInfo->type = AbilityType::PAGE;
    sptr<IRemoteObject> pageToken = new (std::nothrow) MockAbilityToken();
    auto pageRecord = std::make_shared<AbilityRunningRecord>(pageInfo, pageToken, 0);
    moduleRecord->abilities_.emplace(pageToken, pageRecord);

    appRecord->hapModules_["bundleName"].push_back(moduleRecord);

    bool result = appRecord->HasOnlyOneExtensionType();
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_HasOnlyOneExtensionType_0700 end.");
}

/**
 * @tc.name: AppRunningRecord_HasOnlyOneExtensionType_0800
 * @tc.desc: Test HasOnlyOneExtensionType with Extensions and a null ability (null should be ignored).
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_HasOnlyOneExtensionType_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_HasOnlyOneExtensionType_0800 start.");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "test.process");
    ASSERT_NE(appRecord, nullptr);

    auto moduleRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);

    sptr<IRemoteObject> nullToken = new (std::nothrow) MockAbilityToken();
    moduleRecord->abilities_.emplace(nullToken, nullptr);

    auto extInfo = std::make_shared<AbilityInfo>();
    extInfo->type = AbilityType::EXTENSION;
    extInfo->extensionAbilityType = ExtensionAbilityType::SERVICE;
    sptr<IRemoteObject> extToken = new (std::nothrow) MockAbilityToken();
    auto extRecord = std::make_shared<AbilityRunningRecord>(extInfo, extToken, 0);
    moduleRecord->abilities_.emplace(extToken, extRecord);

    appRecord->hapModules_["bundleName"].push_back(moduleRecord);

    bool result = appRecord->HasOnlyOneExtensionType();
    EXPECT_TRUE(result);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningRecord_HasOnlyOneExtensionType_0800 end.");
}

/**
 * @tc.name: AppRunningRecord_GetAbilityRunningRecord_0100
 * @tc.desc: Test GetAbilityRunningRecord returns valid record.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_GetAbilityRunningRecord_0100, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "processName");
    ASSERT_NE(appRunningRecord, nullptr);

    auto result = appRunningRecord->GetAbilityRunningRecord(START_PROCESS_SPECIFY_ABILITY_EVENT_ID);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: AppRunningRecord_LaunchAbility_0100
 * @tc.desc: Test LaunchAbility with null appLifeCycleDeal_.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_LaunchAbility_0100, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "processName");
    ASSERT_NE(appRunningRecord, nullptr);

    appRunningRecord->appLifeCycleDeal_ = nullptr;
    std::shared_ptr<AbilityRunningRecord> ability = nullptr;
    appRunningRecord->LaunchAbility(ability);
}

/**
 * @tc.name: AppRunningRecord_LaunchAbility_0200
 * @tc.desc: Test LaunchAbility with null ability.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_LaunchAbility_0200, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "processName");
    ASSERT_NE(appRunningRecord, nullptr);

    std::shared_ptr<AbilityRunningRecord> ability = nullptr;
    appRunningRecord->LaunchAbility(ability);
}

/**
 * @tc.name: AppRunningRecord_LaunchAbility_0300
 * @tc.desc: Test LaunchAbility with null moduleRecord.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_LaunchAbility_0300, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "processName");
    ASSERT_NE(appRunningRecord, nullptr);

    auto appLifeCycleDeal = std::make_shared<AppLifeCycleDeal>();
    appRunningRecord->appLifeCycleDeal_ = appLifeCycleDeal;
    std::shared_ptr<AbilityRunningRecord> ability = nullptr;
    appRunningRecord->LaunchAbility(ability);
}

/**
 * @tc.name: AppRunningRecord_SendAppStartupTypeEvent_0100
 * @tc.desc: Test SendAppStartupTypeEvent with null ability.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_SendAppStartupTypeEvent_0100, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "processName");
    ASSERT_NE(appRunningRecord, nullptr);

    std::shared_ptr<AbilityRunningRecord> ability = nullptr;
    appRunningRecord->SendAppStartupTypeEvent(ability, AppStartType::COLD);
}

/**
 * @tc.name: AppRunningRecord_SendAppStartupTypeEvent_0200
 * @tc.desc: Test SendAppStartupTypeEvent with null applicationInfo.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_SendAppStartupTypeEvent_0200, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = nullptr;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "processName");
    ASSERT_NE(appRunningRecord, nullptr);

    std::shared_ptr<AbilityRunningRecord> ability = nullptr;
    appRunningRecord->SendAppStartupTypeEvent(ability, AppStartType::COLD);
}

/**
 * @tc.name: AppRunningRecord_SendAppStartupTypeEvent_0300
 * @tc.desc: Test SendAppStartupTypeEvent with null abilityInfo.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_SendAppStartupTypeEvent_0300, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = "testBundle";
    appInfo->versionName = "1.0";
    appInfo->versionCode = 1;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "processName");
    ASSERT_NE(appRunningRecord, nullptr);

    std::shared_ptr<AbilityRunningRecord> ability = nullptr;
    appRunningRecord->SendAppStartupTypeEvent(ability, AppStartType::COLD);
}

/**
 * @tc.name: AppRunningRecord_IsAbilitiesBackground_0100
 * @tc.desc: Test IsAbilitiesBackground returns true.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_IsAbilitiesBackground_0100, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "processName");
    ASSERT_NE(appRunningRecord, nullptr);

    auto result = appRunningRecord->IsAbilitiesBackground();
    EXPECT_TRUE(result);
}

/**
 * @tc.name: AppRunningRecord_IsProcessAttached_0100
 * @tc.desc: Test IsProcessAttached with null appLifeCycleDeal_.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_IsProcessAttached_0100, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "processName");
    ASSERT_NE(appRunningRecord, nullptr);

    appRunningRecord->appLifeCycleDeal_ = nullptr;
    auto result = appRunningRecord->IsProcessAttached();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: AppRunningRecord_ChangeAppGcState_0100
 * @tc.desc: Test ChangeAppGcState with null appLifeCycleDeal_.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_ChangeAppGcState_0100, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "processName");
    ASSERT_NE(appRunningRecord, nullptr);

    appRunningRecord->appLifeCycleDeal_ = nullptr;
    auto result = appRunningRecord->ChangeAppGcState(1, 0);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AppRunningRecord_GetAddStageTimeout_0100
 * @tc.desc: Test GetAddStageTimeout with empty keep alive app.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_GetAddStageTimeout_0100, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "processName");
    ASSERT_NE(appRunningRecord, nullptr);

    auto result = appRunningRecord->GetAddStageTimeout();
    EXPECT_EQ(result, AMSEventHandler::ADD_ABILITY_STAGE_INFO_TIMEOUT);
}

/**
 * @tc.name: AppRunningRecord_AbilityUnfocused_0100
 * @tc.desc: Test AbilityUnfocused with null ability.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_AbilityUnfocused_0100, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "processName");
    ASSERT_NE(appRunningRecord, nullptr);

    std::shared_ptr<AbilityRunningRecord> ability = nullptr;
    auto result = appRunningRecord->AbilityUnfocused(ability);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: AppRunningRecord_AbilityUnfocused_0200
 * @tc.desc: Test AbilityUnfocused when isFocused_ is false.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_AbilityUnfocused_0200, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "processName");
    ASSERT_NE(appRunningRecord, nullptr);

    appRunningRecord->isFocused_ = false;
    sptr<IRemoteObject> token = nullptr;
    auto abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<AbilityRunningRecord> ability = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 1);
    auto result = appRunningRecord->AbilityUnfocused(ability);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: AppRunningRecord_AbilityUnfocused_0300
 * @tc.desc: Test AbilityUnfocused when another ability has focus.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_AbilityUnfocused_0300, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "processName");
    ASSERT_NE(appRunningRecord, nullptr);

    appRunningRecord->isFocused_ = true;
    sptr<IRemoteObject> token = nullptr;
    auto abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<AbilityRunningRecord> ability = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 1);
    auto result = appRunningRecord->AbilityUnfocused(ability);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: AppRunningRecord_AbilityUnfocused_0400
 * @tc.desc: Test AbilityUnfocused normal case.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_AbilityUnfocused_0400, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "processName");
    ASSERT_NE(appRunningRecord, nullptr);

    appRunningRecord->isFocused_ = true;
    sptr<IRemoteObject> token = nullptr;
    auto abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<AbilityRunningRecord> ability = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 1);
    auto result = appRunningRecord->AbilityUnfocused(ability);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: AppRunningRecord_CanRestartResidentProc_0100
 * @tc.desc: Test CanRestartResidentProc when restartResidentProcCount >= 0.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_CanRestartResidentProc_0100, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "processName");
    ASSERT_NE(appRunningRecord, nullptr);

    appRunningRecord->restartResidentProcCount_ = 1;
    auto result = appRunningRecord->CanRestartResidentProc();
    EXPECT_TRUE(result);
}

/**
 * @tc.name: AppRunningRecord_CanRestartResidentProc_0200
 * @tc.desc: Test CanRestartResidentProc when time interval satisfied.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_CanRestartResidentProc_0200, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "processName");
    ASSERT_NE(appRunningRecord, nullptr);

    appRunningRecord->restartResidentProcCount_ = -1;
    appRunningRecord->restartTimeMillis_ = 0;
    auto result = appRunningRecord->CanRestartResidentProc();
    EXPECT_TRUE(result);
}

/**
 * @tc.name: AppRunningRecord_CanRestartResidentProc_0300
 * @tc.desc: Test CanRestartResidentProc when time interval not satisfied.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_CanRestartResidentProc_0300, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "processName");
    ASSERT_NE(appRunningRecord, nullptr);

    appRunningRecord->restartResidentProcCount_ = -1;
    appRunningRecord->restartTimeMillis_ = INT64_MAX;
    auto result = appRunningRecord->CanRestartResidentProc();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: AppRunningRecord_TryToUpdateWorkProcessInfo_0100
 * @tc.desc: Test TryToUpdateWorkProcessInfo when no need to update.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_TryToUpdateWorkProcessInfo_0100, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "processName");
    ASSERT_NE(appRunningRecord, nullptr);

    appRunningRecord->imageProcessType_ = ImageProcessType::TEMPLATE;
    appRunningRecord->needUpdate_ = false;
    appRunningRecord->TryToUpdateWorkProcessInfo();
}

/**
 * @tc.name: AppRunningRecord_TryToUpdateWorkProcessInfo_0200
 * @tc.desc: Test TryToUpdateWorkProcessInfo when appLifeCycleDeal is null.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_TryToUpdateWorkProcessInfo_0200, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "processName");
    ASSERT_NE(appRunningRecord, nullptr);

    appRunningRecord->imageProcessType_ = ImageProcessType::WORK;
    appRunningRecord->needUpdate_ = true;
    appRunningRecord->appLifeCycleDeal_ = nullptr;
    appRunningRecord->TryToUpdateWorkProcessInfo();
}

/**
 * @tc.name: AppRunningRecord_TryToUpdateWorkProcessInfo_0300
 * @tc.desc: Test TryToUpdateWorkProcessInfo normal case.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_TryToUpdateWorkProcessInfo_0300, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "processName");
    ASSERT_NE(appRunningRecord, nullptr);

    appRunningRecord->imageProcessType_ = ImageProcessType::WORK;
    appRunningRecord->needUpdate_ = true;
    auto appLifeCycleDeal = std::make_shared<AppLifeCycleDeal>();
    appRunningRecord->appLifeCycleDeal_ = appLifeCycleDeal;
    appRunningRecord->TryToUpdateWorkProcessInfo();
    EXPECT_FALSE(appRunningRecord->needUpdate_);
}

/**
 * @tc.name: AppRunningRecord_ScheduleNewProcessRequest_0100
 * @tc.desc: Test ScheduleNewProcessRequest when appLifeCycleDeal is null.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_ScheduleNewProcessRequest_0100, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "processName");
    ASSERT_NE(appRunningRecord, nullptr);

    appRunningRecord->appLifeCycleDeal_ = nullptr;
    AAFwk::Want want;
    appRunningRecord->ScheduleNewProcessRequest(want, "moduleName");
}

/**
 * @tc.name: AppRunningRecord_ScheduleNewProcessRequest_0200
 * @tc.desc: Test ScheduleNewProcessRequest normal case.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_ScheduleNewProcessRequest_0200, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "processName");
    ASSERT_NE(appRunningRecord, nullptr);

    appRunningRecord->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    appRunningRecord->eventHandler_ = std::make_shared<AMSEventHandler>(appRunningRecord->taskHandler_,
        appRunningRecord->appMgrServiceInner_);
    auto appLifeCycleDeal = std::make_shared<AppLifeCycleDeal>();
    appRunningRecord->appLifeCycleDeal_ = appLifeCycleDeal;
    AAFwk::Want want;
    appRunningRecord->ScheduleNewProcessRequest(want, "moduleName");
}

/**
 * @tc.name: AppRunningRecord_IsAbilitiesBackground_0200
 * @tc.desc: Test IsAbilitiesBackground when there is ability not backgrounded.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_IsAbilitiesBackground_0200, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "processName");
    ASSERT_NE(appRunningRecord, nullptr);

    appRunningRecord->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    appRunningRecord->eventHandler_ = std::make_shared<AMSEventHandler>(appRunningRecord->taskHandler_,
        appRunningRecord->appMgrServiceInner_);
    auto moduleRecord = std::make_shared<ModuleRunningRecord>(appInfo, appRunningRecord->eventHandler_);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    sptr<IRemoteObject> token = nullptr;
    auto abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token, 1);
    abilityRecord->SetState(AbilityState::ABILITY_STATE_FOREGROUND);
    moduleRecord->abilities_.emplace(token, abilityRecord);
    appRunningRecord->hapModules_["bundleName"] = { moduleRecord };
    auto result = appRunningRecord->IsAbilitiesBackground();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: AppRunningRecord_IsProcessAttached_0200
 * @tc.desc: Test IsProcessAttached when GetApplicationClient is not null.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_IsProcessAttached_0200, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "processName");
    ASSERT_NE(appRunningRecord, nullptr);

    auto appLifeCycleDeal = std::make_shared<AppLifeCycleDeal>();
    appRunningRecord->appLifeCycleDeal_ = appLifeCycleDeal;
    auto result = appRunningRecord->IsProcessAttached();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: AppRunningRecord_ChangeAppGcState_0200
 * @tc.desc: Test ChangeAppGcState normal case.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_ChangeAppGcState_0200, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "processName");
    ASSERT_NE(appRunningRecord, nullptr);

    auto appLifeCycleDeal = std::make_shared<AppLifeCycleDeal>();
    appRunningRecord->appLifeCycleDeal_ = appLifeCycleDeal;
    auto result = appRunningRecord->ChangeAppGcState(1, 0);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AppRunningRecord_GetAddStageTimeout_0200
 * @tc.desc: Test GetAddStageTimeout when IsEmptyKeepAliveApp returns true.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, AppRunningRecord_GetAddStageTimeout_0200, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "processName");
    ASSERT_NE(appRunningRecord, nullptr);

    appRunningRecord->isEmptyKeepAliveApp_ = true;
    auto result = appRunningRecord->GetAddStageTimeout();
    EXPECT_EQ(result, AMSEventHandler::ADD_ABILITY_STAGE_EMPTY_RESIDENT_TIMEOUT);
}
} // namespace AppExecFwk
} // namespace OHOS
