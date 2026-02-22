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

#include <gtest/gtest.h>

#include "hilog_tag_wrapper.h"
#include "ability_manager_client.h"
#include "ability_record.h"
#include "base_extension_record.h"
#include "extension_record.h"
#include "extension_record_factory.h"
#define private public
#include "extension_record_manager.h"
#undef private
#include "mock_ability_token.h"
#include "mock_native_token.h"
#include "scene_board_judgement.h"
#include "session_info.h"
#include "preload_ui_extension_execute_callback_interface.h"
#include "ui_extension_record.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
namespace {
    constexpr int32_t EXTENSION_RECORD_KEY_0 = 0;
    constexpr int32_t EXTENSION_RECORD_KEY_1 = 1;
    constexpr int32_t EXTENSION_RECORD_KEY_2 = 2;
    constexpr const char *SEPARATOR = ":";
} // namespace
class MockPreloadCallback : public IRemoteStub<AAFwk::IPreloadUIExtensionExecuteCallback> {
public:
    MockPreloadCallback() = default;
    virtual ~MockPreloadCallback() = default;
    int32_t lastCalledId = -1;
    bool onDestroyCalled = false;
    bool onPreloadSuccessCalled = false;
    int32_t lastErrCode = -1;
    int32_t lastRequestCode = -1;

    void OnLoadedDone(int32_t extensionAbilityId) override
    {
        lastCalledId = extensionAbilityId;
    }
    void OnDestroyDone(int32_t extensionAbilityId) override
    {
        onDestroyCalled = true;
    }
    void OnPreloadSuccess(int32_t requestCode, int32_t extensionAbilityId, int32_t innerErrCode) override
    {
        onPreloadSuccessCalled = true;
        lastRequestCode = requestCode;
        lastErrCode = innerErrCode;
    }

    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        return 0;
    }
};

class ExtensionRecordManagerTest : public testing::Test {
public:

    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void ExtensionRecordManagerTest::SetUpTestCase(void)
{}

void ExtensionRecordManagerTest::TearDownTestCase(void)
{}

void ExtensionRecordManagerTest::SetUp()
{}

void ExtensionRecordManagerTest::TearDown()
{}

/**
 * @tc.name: IsFocused_0100
 * @tc.desc: check is focused.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, IsFocused_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    int32_t extensionRecordId = 1;
    extRecordMgr->AddExtensionRecord(extensionRecordId, extRecord);

    sptr<IRemoteObject> token = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    ASSERT_NE(token, nullptr);
    extRecordMgr->SetCachedFocusedCallerToken(extensionRecordId, token);

    bool isFocused = extRecordMgr->IsFocused(extensionRecordId, token, token);
    EXPECT_EQ(isFocused, true);

    auto focusedToken = extRecordMgr->GetCachedFocusedCallerToken(extensionRecordId);
    EXPECT_EQ(focusedToken, token);

    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GetCallerTokenList_0100
 * @tc.desc: get caller token list.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, GetCallerTokenList_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    ASSERT_NE(extRecord, nullptr);

    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    ASSERT_NE(sessionInfo, nullptr);
    sptr<IRemoteObject> callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    EXPECT_NE(callerToken, nullptr);
    sessionInfo->callerToken = callerToken;
    extRecord->abilityRecord_->SetSessionInfo(sessionInfo);
    extRecord->SetFocusedCallerToken(callerToken);
    extRecord->GetFocusedCallerToken();

    int32_t extensionRecordId = 1;
    extRecordMgr->AddExtensionRecord(extensionRecordId, extRecord);

    std::list<sptr<IRemoteObject>> callerList;
    extRecordMgr->GetCallerTokenList(abilityRecord, callerList);
    ASSERT_NE(extRecordMgr, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GenerateExtensionRecordId_0100
 * @tc.desc: generate extension record id.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, GenerateExtensionRecordId_0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);
    int32_t extensionRecordId = 2;
    EXPECT_EQ(extRecordMgr->GenerateExtensionRecordId(INVALID_EXTENSION_RECORD_ID), 1);
    EXPECT_EQ(extRecordMgr->GenerateExtensionRecordId(extensionRecordId), 2);
    EXPECT_EQ(extRecordMgr->GenerateExtensionRecordId(extensionRecordId), 3);
}

/**
 * @tc.name: AddExtensionRecord_0100
 * @tc.desc: AddExtensionRecord RemoveExtensionRecord
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, AddExtensionRecord_0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    int32_t extensionRecordId = 5;
    extRecordMgr->AddExtensionRecord(extensionRecordId, extRecord);
    extRecordMgr->AddExtensionRecordToTerminatedList(extensionRecordId);
    extRecordMgr->RemoveExtensionRecord(extensionRecordId);
    extRecordMgr->AddExtensionRecordToTerminatedList(extensionRecordId);

    int32_t extensionRecordId2 = 6;
    extRecordMgr->AddExtensionRecord(extensionRecordId2, extRecord);
    bool  bIsload = false;
    auto extRecord2 = std::make_shared<ExtensionRecord>(abilityRecord);
    auto result = extRecordMgr->GetExtensionRecord(extensionRecordId2, "com.example.unittest", extRecord2, bIsload);

    auto result2 = extRecordMgr->GetExtensionRecord(extensionRecordId2, "aa", extRecord2, bIsload);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: GetActiveUIExtensionList_0100
 * @tc.desc: GetActiveUIExtensionList
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, GetActiveUIExtensionList_0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    int32_t extensionRecordId = 5;
    extRecordMgr->AddExtensionRecord(extensionRecordId, extRecord);
    int32_t pId = abilityRecord->GetPid();
    std::vector<std::string> extensionList;
    extRecordMgr->GetActiveUIExtensionList(pId, extensionList);
    extRecordMgr->GetActiveUIExtensionList("com.example.unittest", extensionList);
    extRecordMgr->GetActiveUIExtensionList("aa", extensionList);
}

/**
 * @tc.name: GetAbilityRecordBySessionInfo_0100
 * @tc.desc: GetAbilityRecordBySessionInfo
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, GetAbilityRecordBySessionInfo_0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    sptr<AAFwk::SessionInfo> sessionInfo(new AAFwk::SessionInfo());
    sessionInfo->uiExtensionComponentId = 10;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    int32_t extensionRecordId = 5;
    extRecordMgr->AddExtensionRecord(extensionRecordId, extRecord);

    sptr<AAFwk::SessionInfo> sessionParam(new AAFwk::SessionInfo());
    sessionParam->uiExtensionComponentId = INVALID_EXTENSION_RECORD_ID;
    EXPECT_EQ(extRecordMgr->GetAbilityRecordBySessionInfo(sessionParam), nullptr);

    sessionParam->uiExtensionComponentId = 10;
    EXPECT_EQ(extRecordMgr->GetAbilityRecordBySessionInfo(sessionParam), abilityRecord);
    sessionParam->uiExtensionComponentId = 9;
    EXPECT_EQ(extRecordMgr->GetAbilityRecordBySessionInfo(sessionParam), nullptr);

    auto extRecord2 = std::make_shared<ExtensionRecord>(nullptr);
    int32_t extensionRecordId2 = 6;
    extRecordMgr->AddExtensionRecord(extensionRecordId2, extRecord2);
    extRecordMgr->GetAbilityRecordBySessionInfo(sessionParam);

    AAFwk::AbilityRequest abilityRequest2;
    auto abilityRecord2 = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    bool bLoaded = false;
    std::string  bundleName = "";
    extRecordMgr->GetOrCreateExtensionRecord(abilityRequest, bundleName, abilityRecord2, bLoaded);
    extRecordMgr->GetOrCreateExtensionRecord(abilityRequest2, bundleName, abilityRecord2, bLoaded);
    int32_t hostPid = 0;
    extRecordMgr->GetHostPidForExtensionId(extensionRecordId, hostPid);
}

/**
 * @tc.name: AddPreloadUIExtensionRecord_0100
 * @tc.desc: AddPreloadUIExtensionRecord
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, AddPreloadUIExtensionRecord_0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;

    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);


    extRecordMgr->AddPreloadUIExtensionRecord(abilityRecord);
    int32_t extId = 12;
    abilityRecord->SetUIExtensionAbilityId(extId);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);

    extRecordMgr->AddExtensionRecord(extId, extRecord);
    extRecordMgr->AddPreloadUIExtensionRecord(abilityRecord);
    extRecordMgr->StartAbility(abilityRequest);
    extRecordMgr->GetCachedFocusedCallerToken(extId);
}

/**
 * @tc.name: CreateExtensionRecord_0100
 * @tc.desc: CreateExtensionRecord
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, CreateExtensionRecord_0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);
    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    abilityRequest.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED;

    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);

    std::string bundleName = "com.example.unittest";
    int32_t extRecordId = 9;
    std::shared_ptr<ExtensionRecord> extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    auto result = extRecordMgr->CreateExtensionRecord(abilityRequest, bundleName, extRecord, extRecordId);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    abilityRequest.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::PHOTO_EDITOR;
}

/**
 * @tc.name: IsHostSpecifiedProcessValid_0100
 * @tc.desc: IsHostSpecifiedProcessValid
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, IsHostSpecifiedProcessValid_0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    AAFwk::AbilityRequest abilityRequest;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    std::shared_ptr<ExtensionRecord> extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    std::string process = "testProcess";

    bool result = extRecordMgr->IsHostSpecifiedProcessValid(abilityRequest, extRecord, process);
    EXPECT_FALSE(result);

    std::shared_ptr<ExtensionRecord> nullExtRecord = std::make_shared<ExtensionRecord>(nullptr);
    extRecordMgr->extensionRecords_.insert({EXTENSION_RECORD_KEY_0, nullptr});
    extRecordMgr->extensionRecords_.insert({EXTENSION_RECORD_KEY_1, nullExtRecord});
    extRecordMgr->extensionRecords_.insert({EXTENSION_RECORD_KEY_2, extRecord});
    result = extRecordMgr->IsHostSpecifiedProcessValid(abilityRequest, extRecord, process);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsHostSpecifiedProcessValid_0200
 * @tc.desc: IsHostSpecifiedProcessValid
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, IsHostSpecifiedProcessValid_0200, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    std::shared_ptr<ExtensionRecord> extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    std::string process = "testProcess";

    bool result = extRecordMgr->IsHostSpecifiedProcessValid(abilityRequest, extRecord, process);
    EXPECT_FALSE(result);

    extRecord->abilityRecord_->SetProcessName(process);
    extRecordMgr->extensionRecords_.insert({EXTENSION_RECORD_KEY_0, extRecord});
    result = extRecordMgr->IsHostSpecifiedProcessValid(abilityRequest, extRecord, process);
    EXPECT_TRUE(result);

    abilityRequest.abilityInfo.name = "ability";
    result = extRecordMgr->IsHostSpecifiedProcessValid(abilityRequest, extRecord, process);
    EXPECT_FALSE(result);

    abilityRequest.abilityInfo.bundleName = "testBundleName";
    result = extRecordMgr->IsHostSpecifiedProcessValid(abilityRequest, extRecord, process);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: UpdateProcessName_0100
 * @tc.desc: UpdateProcessName
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, UpdateProcessName_0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = "testBundleName";
    abilityRequest.abilityInfo.name = "testInfoName";
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    std::shared_ptr<ExtensionRecord> extRecord = std::make_shared<ExtensionRecord>(abilityRecord);

    int32_t result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_EQ(result, ERR_OK);

    extRecord->processMode_ = PROCESS_MODE_INSTANCE;
    std::string process = abilityRequest.abilityInfo.bundleName + SEPARATOR + abilityRequest.abilityInfo.name
        + SEPARATOR + std::to_string(abilityRecord->GetUIExtensionAbilityId());
    result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_EQ(abilityRecord->GetProcessName(), process);
    EXPECT_EQ(result, ERR_OK);

    extRecord->processMode_ = PROCESS_MODE_TYPE;
    process = abilityRequest.abilityInfo.bundleName + SEPARATOR + abilityRequest.abilityInfo.name;
    result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_EQ(abilityRecord->GetProcessName(), process);
    EXPECT_EQ(result, ERR_OK);

    extRecord->processMode_ = PROCESS_MODE_CUSTOM;
    process = abilityRequest.abilityInfo.bundleName + abilityRequest.customProcess;
    result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_EQ(abilityRecord->GetProcessName(), process);
    EXPECT_EQ(result, ERR_OK);

    extRecord->processMode_ = PROCESS_MODE_HOST_SPECIFIED;
    process = abilityRequest.want.GetStringParam(PROCESS_MODE_HOST_SPECIFIED_KEY);
    result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    extRecord->abilityRecord_->SetProcessName(process);
    extRecordMgr->extensionRecords_.insert({EXTENSION_RECORD_KEY_0, extRecord});
    result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_EQ(abilityRecord->GetProcessName(), process);
    EXPECT_EQ(result, ERR_OK);

    extRecord->processMode_ = PROCESS_MODE_RUN_WITH_MAIN_PROCESS;
    process = abilityRequest.abilityInfo.bundleName;
    result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_EQ(abilityRecord->GetProcessName(), process);
    EXPECT_EQ(result, ERR_OK);

    abilityRequest.appInfo.process = "testProcess";
    process = abilityRequest.appInfo.process;
    result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_EQ(abilityRecord->GetProcessName(), process);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: UpdateProcessName_0200
 * @tc.desc: UpdateProcessName
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, UpdateProcessName_0200, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = "testBundleName";
    abilityRequest.abilityInfo.name = "testInfoName";
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    std::shared_ptr<ExtensionRecord> extRecord = std::make_shared<ExtensionRecord>(abilityRecord);

    extRecord->processMode_ = PROCESS_MODE_PLUGIN;
    extRecord->hostBundleName_ = "testHostBundleName";
    const char *embeddedUI = "embeddedUI";
    std::string hostBundleName = "testHostBundleName";
    std::string process = hostBundleName + SEPARATOR + abilityRequest.abilityInfo.bundleName
        + SEPARATOR + embeddedUI + SEPARATOR + std::to_string(abilityRequest.abilityInfo.appIndex);
    int32_t result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_EQ(abilityRecord->GetProcessName(), process);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: IsPreloadExtensionRecord_0100
 * @tc.desc: IsPreloadExtensionRecord
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, IsPreloadExtensionRecord_0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    AAFwk::AbilityRequest abilityRequest;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    std::shared_ptr<ExtensionRecord> extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    std::string hostBundleName = "testHostBundleName";
    bool isLoaded = false;
    int32_t hostPid = 0;

    bool result = extRecordMgr->IsPreloadExtensionRecord(abilityRequest, hostPid, extRecord, isLoaded);
    EXPECT_FALSE(result);

    std::string deviceId = "testDeviceId";
    std::string bundleName = "testBundleName";
    std::string abilityName = "testAbilityName";
    std::string moduleName = "testModuleName";
    abilityRequest.want.SetElementName(deviceId, bundleName, abilityName, moduleName);
    auto extensionRecordMapKey = std::make_tuple(abilityName, bundleName, moduleName, hostPid);
    std::vector<std::shared_ptr<ExtensionRecord>> nullExtensions;
    std::vector<std::shared_ptr<ExtensionRecord>> extensions;

    extensions.push_back(extRecord);
    extRecordMgr->preloadUIExtensionMap_.insert({extensionRecordMapKey, extensions});
    result = extRecordMgr->IsPreloadExtensionRecord(abilityRequest, hostPid, extRecord, isLoaded);
    EXPECT_TRUE(isLoaded);
    EXPECT_TRUE(result);

    extRecordMgr->preloadUIExtensionMap_[extensionRecordMapKey] = nullExtensions;
    result = extRecordMgr->IsPreloadExtensionRecord(abilityRequest, hostPid, extRecord, isLoaded);
    EXPECT_FALSE(result);

    nullExtensions.push_back(nullptr);
    extRecordMgr->preloadUIExtensionMap_[extensionRecordMapKey] = nullExtensions;
    result = extRecordMgr->IsPreloadExtensionRecord(abilityRequest, hostPid, extRecord, isLoaded);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsBelongToManager_0100
 * @tc.desc: IsBelongToManager
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, IsBelongToManager_0100, TestSize.Level1)
{
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.extensionAbilityType =  AppExecFwk::ExtensionAbilityType::SYSDIALOG_COMMON;
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    EXPECT_TRUE(extRecordMgr->IsBelongToManager(abilityInfo));
}

/**
 * @tc.name: GetOrCreateExtensionRecord_0100
 * @tc.desc: GetOrCreateExtensionRecord
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, GetOrCreateExtensionRecord_0100, TestSize.Level1)
{
    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    abilityRequest.sessionInfo->uiExtensionComponentId = 0;

    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    std::shared_ptr<ExtensionRecord> extRecord =
        std::make_shared<ExtensionRecord>(abilityRecord);
    extRecord->abilityRecord_ = abilityRecord;
    int32_t hostPid = 0;
    std::string hostBundleName = "testHostBundleName";
    bool isLoaded = false;

    std::string deviceId = "testDeviceId";
    std::string bundleName = "testBundleName";
    std::string abilityName = "testAbilityName";
    std::string moduleName = "testModuleName";
    abilityRequest.want.SetElementName(deviceId, bundleName, abilityName, moduleName);
    auto extensionRecordMapKey = std::make_tuple(abilityName,
        bundleName, moduleName, hostPid);
    std::vector<std::shared_ptr<ExtensionRecord>> nullExtensions;
    std::vector<std::shared_ptr<ExtensionRecord>> extensions;

    extensions.push_back(extRecord);
    extRecordMgr->preloadUIExtensionMap_.insert({extensionRecordMapKey, extensions});
    auto ret = extRecordMgr->GetOrCreateExtensionRecord(abilityRequest,
        hostBundleName, abilityRecord, isLoaded);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: GetOrCreateExtensionRecord_0200
 * @tc.desc: GetOrCreateExtensionRecord
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, GetOrCreateExtensionRecord_0200, TestSize.Level1)
{
    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    abilityRequest.sessionInfo->uiExtensionComponentId = 0;
    std::string hostBundleName = "testHostBundleName";
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    bool isLoaded = false;
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    auto ret = extRecordMgr->GetOrCreateExtensionRecord(abilityRequest,
        hostBundleName, abilityRecord, isLoaded);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: GetUIExtensionSessionInfo_0100
 * @tc.desc: GetUIExtensionSessionInfo
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, GetUIExtensionSessionInfo_0100, TestSize.Level1)
{
    sptr<IRemoteObject> token;
    UIExtensionSessionInfo uiExtensionSessionInfo;
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    auto ret = extRecordMgr->GetUIExtensionSessionInfo(token, uiExtensionSessionInfo);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
}

/**
 * @tc.name: GetUIExtensionSessionInfo_0200
 * @tc.desc: GetUIExtensionSessionInfo
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, GetUIExtensionSessionInfo_0200, TestSize.Level1)
{
    sptr<IRemoteObject> token = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    ASSERT_NE(token, nullptr);
    UIExtensionSessionInfo uiExtensionSessionInfo;
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    auto ret = extRecordMgr->GetUIExtensionSessionInfo(token, uiExtensionSessionInfo);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
}

/**
 * @tc.name: GetUIExtensionSessionInfo_0300
 * @tc.desc: GetUIExtensionSessionInfo
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, GetUIExtensionSessionInfo_0300, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    abilityRequest.abilityInfo.extensionAbilityType =  AppExecFwk::ExtensionAbilityType::UNSPECIFIED;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    int32_t extensionRecordId = 1;
    extRecordMgr->AddExtensionRecord(extensionRecordId, extRecord);

    auto token = abilityRecord->GetToken();
    ASSERT_NE(token, nullptr);

    UIExtensionSessionInfo uiExtensionSessionInfo;
    auto ret = extRecordMgr->GetUIExtensionSessionInfo(token, uiExtensionSessionInfo);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: GetUIExtensionSessionInfo_0400
 * @tc.desc: GetUIExtensionSessionInfo
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, GetUIExtensionSessionInfo_0400, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    abilityRequest.abilityInfo.extensionAbilityType =  AppExecFwk::ExtensionAbilityType::SYSDIALOG_COMMON;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    int32_t extensionRecordId = 1;
    extRecordMgr->AddExtensionRecord(extensionRecordId, extRecord);

    auto token = abilityRecord->GetToken();
    ASSERT_NE(token, nullptr);

    UIExtensionSessionInfo uiExtensionSessionInfo;
    auto ret = extRecordMgr->GetUIExtensionSessionInfo(token, uiExtensionSessionInfo);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
}

/**
 * @tc.name: GetUIExtensionSessionInfo_0500
 * @tc.desc: GetUIExtensionSessionInfo
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, GetUIExtensionSessionInfo_0500, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    abilityRequest.abilityInfo.extensionAbilityType =  AppExecFwk::ExtensionAbilityType::SYSDIALOG_COMMON;
    std::string deviceId = "testDeviceId";
    std::string bundleName = "testBundleName";
    std::string abilityName = "testAbilityName";
    std::string moduleName = "testModuleName";
    abilityRequest.want.SetElementName(deviceId, bundleName, abilityName, moduleName);
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    int32_t extensionRecordId = 1;
    extRecordMgr->AddExtensionRecord(extensionRecordId, extRecord);

    auto token = abilityRecord->GetToken();
    ASSERT_NE(token, nullptr);

    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->persistentId  = 1;
    sessionInfo->hostWindowId  = 1;
    sessionInfo->uiExtensionUsage  = AAFwk::UIExtensionUsage::MODAL;
    ASSERT_NE(sessionInfo, nullptr);
    extRecord->abilityRecord_->SetSessionInfo(sessionInfo);

    UIExtensionSessionInfo uiExtensionSessionInfo;
    auto ret = extRecordMgr->GetUIExtensionSessionInfo(token, uiExtensionSessionInfo);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: GetUIExtensionSessionInfo_0600
 * @tc.desc: GetUIExtensionSessionInfo
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, GetUIExtensionSessionInfo_0600, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    abilityRequest.abilityInfo.extensionAbilityType =  AppExecFwk::ExtensionAbilityType::SYSDIALOG_COMMON;
    std::string deviceId = "testDeviceId";
    std::string bundleName = "testBundleName";
    std::string abilityName = "testAbilityName";
    std::string moduleName = "testModuleName";
    abilityRequest.want.SetElementName(deviceId, bundleName, abilityName, moduleName);
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    int32_t extensionRecordId = 1;
    extRecordMgr->AddExtensionRecord(extensionRecordId, extRecord);

    auto token = abilityRecord->GetToken();

    AAFwk::AbilityRequest callerAbilityRequest;
    callerAbilityRequest.appInfo.bundleName = "com.example.unittest";
    callerAbilityRequest.abilityInfo.name = "MainAbility";
    callerAbilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    callerAbilityRequest.abilityInfo.extensionAbilityType =  AppExecFwk::ExtensionAbilityType::SYSDIALOG_COMMON;
    std::string callerDeviceId = "testDeviceId";
    std::string callerBundleName = "testBundleName";
    std::string callerAbilityName = "testAbilityName";
    std::string callerModuleName = "testModuleName";
    callerAbilityRequest.want.SetElementName(callerDeviceId, callerBundleName, callerAbilityName, callerModuleName);
    auto callerAbilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(callerAbilityRequest);
    ASSERT_NE(callerAbilityRecord, nullptr);
    auto callerExtRecord = std::make_shared<ExtensionRecord>(callerAbilityRecord);
    int32_t callerExtensionRecordId = 2;
    extRecordMgr->AddExtensionRecord(callerExtensionRecordId, callerExtRecord);
    sptr<IRemoteObject> callerToken = callerAbilityRecord->GetToken();
    ASSERT_NE(callerToken, nullptr);

    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->persistentId  = 1;
    sessionInfo->hostWindowId  = 1;
    sessionInfo->uiExtensionUsage  = AAFwk::UIExtensionUsage::MODAL;
    sessionInfo->callerToken = callerToken;
    ASSERT_NE(sessionInfo, nullptr);
    extRecord->abilityRecord_->SetSessionInfo(sessionInfo);

    UIExtensionSessionInfo uiExtensionSessionInfo;
    auto ret = extRecordMgr->GetUIExtensionSessionInfo(token, uiExtensionSessionInfo);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: GetRemoteCallback_0100
 * @tc.desc: Test GetRemoteCallback when uiExtensionRecord is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, GetRemoteCallback_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetRemoteCallback_0100 begin.");
    
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    auto result = extRecordMgr->GetRemoteCallback(nullptr);
    EXPECT_EQ(result, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "GetRemoteCallback_0100 end.");
}

/**
 * @tc.name: GetRemoteCallback_0200
 * @tc.desc: Test GetRemoteCallback when hostPid is 0.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, GetRemoteCallback_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetRemoteCallback_0200 begin.");
    
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    
    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);

    extRecord->hostPid_ = 0;

    auto result = extRecordMgr->GetRemoteCallback(extRecord);
    EXPECT_EQ(result, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "GetRemoteCallback_0200 end.");
}

/**
 * @tc.name: GetRemoteCallback_0300
 * @tc.desc: Test GetRemoteCallback when Map lookup fails (pid not found).
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, GetRemoteCallback_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetRemoteCallback_0300 begin.");
    
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);

    extRecord->hostPid_ = 9999;

    auto result = extRecordMgr->GetRemoteCallback(extRecord);
    EXPECT_EQ(result, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "GetRemoteCallback_0300 end.");
}

/**
 * @tc.name: GetRemoteCallback_0400
 * @tc.desc: Test GetRemoteCallback when Map lookup succeeds but Cast fails.
 *           This covers the map finding logic without needing a specific Mock class.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, GetRemoteCallback_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetRemoteCallback_0400 begin.");
    
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);

    int32_t testPid = 1001;
    extRecord->hostPid_ = testPid;

    sptr<IRemoteObject> token = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    ASSERT_NE(token, nullptr);
    {
        std::lock_guard<std::mutex> lock(extRecordMgr->preloadUIExtensionHostClientMutex_);
        extRecordMgr->preloadUIExtensionHostClientCallerTokens_[testPid] = token;
    }
    auto result = extRecordMgr->GetRemoteCallback(extRecord);
    
    EXPECT_EQ(result, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "GetRemoteCallback_0400 end.");
}

/**
 * @tc.name: HandlePreloadUIExtensionLoaded_0100
 * @tc.desc: Test Success Scenario.
 *           Verify OnLoadedDone is actually called.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, HandlePreloadUIExtensionLoaded_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandlePreloadUIExtensionLoaded_0100 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test";
    abilityRequest.abilityInfo.name = "TestAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);

    int32_t recordId = 200;
    int32_t hostPid = 8888;
    
    extRecord->hostPid_ = hostPid;
    extRecord->extensionRecordId_ = recordId;
    extRecordMgr->AddExtensionRecord(recordId, extRecord);

    sptr<MockPreloadCallback> mockCallback = new (std::nothrow) MockPreloadCallback();
    ASSERT_NE(mockCallback, nullptr);
    {
        std::lock_guard<std::mutex> lock(extRecordMgr->preloadUIExtensionHostClientMutex_);
        extRecordMgr->preloadUIExtensionHostClientCallerTokens_[hostPid] = mockCallback->AsObject();
    }
    extRecordMgr->HandlePreloadUIExtensionLoaded(extRecord);
    EXPECT_EQ(mockCallback->lastCalledId, recordId);
    extRecordMgr->RemoveExtensionRecord(recordId);
    TAG_LOGI(AAFwkTag::TEST, "HandlePreloadUIExtensionLoaded_0100 end");
}

/**
 * @tc.name: HandlePreloadUIExtensionDestroyed_0100
 * @tc.desc: Test destroyed logic - Success scenario.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, HandlePreloadUIExtensionDestroyed_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandlePreloadUIExtensionDestroyed_0100 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.destroy";
    abilityRequest.abilityInfo.name = "DestroyAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);

    int32_t recordId = 300;
    int32_t hostPid = 3333;
    extRecord->hostPid_ = hostPid;
    extRecord->extensionRecordId_ = recordId;
    extRecordMgr->AddExtensionRecord(recordId, extRecord);

    sptr<MockPreloadCallback> mockCallback = new (std::nothrow) MockPreloadCallback();
    ASSERT_NE(mockCallback, nullptr);
    {
        std::lock_guard<std::mutex> lock(extRecordMgr->preloadUIExtensionHostClientMutex_);
        extRecordMgr->preloadUIExtensionHostClientCallerTokens_[hostPid] = mockCallback->AsObject();
    }
    extRecordMgr->HandlePreloadUIExtensionDestroyed(extRecord);
    EXPECT_TRUE(mockCallback->onDestroyCalled);
    extRecordMgr->RemoveExtensionRecord(recordId);
    TAG_LOGI(AAFwkTag::TEST, "HandlePreloadUIExtensionDestroyed_0100 end");
}

/**
 * @tc.name: RegisterPreloadUIExtensionHostClient_0100
 * @tc.desc: Test RegisterPreloadUIExtensionHostClient with nullptr token
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, RegisterPreloadUIExtensionHostClient_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RegisterPreloadUIExtensionHostClient_0100 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);
    
    sptr<IRemoteObject> nullToken = nullptr;
    extRecordMgr->RegisterPreloadUIExtensionHostClient(nullToken);
    
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    {
        std::lock_guard<std::mutex> lock(extRecordMgr->preloadUIExtensionHostClientMutex_);
        auto it = extRecordMgr->preloadUIExtensionHostClientCallerTokens_.find(callerPid);
        EXPECT_EQ(it, extRecordMgr->preloadUIExtensionHostClientCallerTokens_.end());
    }
    
    TAG_LOGI(AAFwkTag::TEST, "RegisterPreloadUIExtensionHostClient_0100 end");
}

/**
 * @tc.name: RegisterPreloadUIExtensionHostClient_0200
 * @tc.desc: Test RegisterPreloadUIExtensionHostClient with valid token
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, RegisterPreloadUIExtensionHostClient_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RegisterPreloadUIExtensionHostClient_0200 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);
    
    sptr<MockPreloadCallback> mockCallback = new (std::nothrow) MockPreloadCallback();
    ASSERT_NE(mockCallback, nullptr);
    
    sptr<IRemoteObject> validToken = mockCallback->AsObject();
    ASSERT_NE(validToken, nullptr);
    
    extRecordMgr->RegisterPreloadUIExtensionHostClient(validToken);
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    {
        std::lock_guard<std::mutex> lock(extRecordMgr->preloadUIExtensionHostClientMutex_);
        auto it = extRecordMgr->preloadUIExtensionHostClientCallerTokens_.find(callerPid);
        EXPECT_NE(it, extRecordMgr->preloadUIExtensionHostClientCallerTokens_.end());
        EXPECT_EQ(it->second, validToken);
    }
    
    TAG_LOGI(AAFwkTag::TEST, "RegisterPreloadUIExtensionHostClient_0200 end");
}

/**
 * @tc.name: UnRegisterPreloadUIExtensionHostClient_0100
 * @tc.desc: Test UnRegisterPreloadUIExtensionHostClient with non-existent key
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, UnRegisterPreloadUIExtensionHostClient_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UnRegisterPreloadUIExtensionHostClient_0100 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);
    int32_t nonExistentKey = 99999;
    extRecordMgr->UnRegisterPreloadUIExtensionHostClient(nonExistentKey, nullptr);
    
    {
        std::lock_guard<std::mutex> lock(extRecordMgr->preloadUIExtensionHostClientMutex_);
        auto it = extRecordMgr->preloadUIExtensionHostClientCallerTokens_.find(nonExistentKey);
        EXPECT_EQ(it, extRecordMgr->preloadUIExtensionHostClientCallerTokens_.end());
    }
    
    TAG_LOGI(AAFwkTag::TEST, "UnRegisterPreloadUIExtensionHostClient_0100 end");
}

/**
 * @tc.name: UnRegisterPreloadUIExtensionHostClient_0200
 * @tc.desc: Test UnRegisterPreloadUIExtensionHostClient with valid token
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, UnRegisterPreloadUIExtensionHostClient_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UnRegisterPreloadUIExtensionHostClient_0200 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);
    sptr<MockPreloadCallback> mockCallback = new (std::nothrow) MockPreloadCallback();
    ASSERT_NE(mockCallback, nullptr);
    sptr<IRemoteObject> validToken = mockCallback->AsObject();
    ASSERT_NE(validToken, nullptr);
    
    extRecordMgr->RegisterPreloadUIExtensionHostClient(validToken);
    
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    {
        std::lock_guard<std::mutex> lock(extRecordMgr->preloadUIExtensionHostClientMutex_);
        auto it = extRecordMgr->preloadUIExtensionHostClientCallerTokens_.find(callerPid);
        EXPECT_NE(it, extRecordMgr->preloadUIExtensionHostClientCallerTokens_.end());
        EXPECT_EQ(it->second, validToken);
    }
    extRecordMgr->UnRegisterPreloadUIExtensionHostClient(callerPid, nullptr);
    {
        std::lock_guard<std::mutex> lock(extRecordMgr->preloadUIExtensionHostClientMutex_);
        auto it = extRecordMgr->preloadUIExtensionHostClientCallerTokens_.find(callerPid);
        EXPECT_EQ(it, extRecordMgr->preloadUIExtensionHostClientCallerTokens_.end());
    }
    TAG_LOGI(AAFwkTag::TEST, "UnRegisterPreloadUIExtensionHostClient_0200 end");
}

/**
 * @tc.name: HandlePreloadUIExtensionSuccess_0200
 * @tc.desc: Test success logic - isPreloadedSuccess = true.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, HandlePreloadUIExtensionSuccess_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandlePreloadUIExtensionSuccess_0200 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.success";
    abilityRequest.abilityInfo.name = "SuccessAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    auto extRecord = std::make_shared<UIExtensionRecord>(abilityRecord);

    int32_t recordId = 400;
    int32_t hostPid = 4444;
    int32_t requestCode = 12345;
    
    extRecord->hostPid_ = hostPid;
    extRecord->requestCode_ = requestCode;
    extRecordMgr->AddExtensionRecord(recordId, extRecord);

    sptr<MockPreloadCallback> mockCallback = new (std::nothrow) MockPreloadCallback();
    ASSERT_NE(mockCallback, nullptr);
    {
        std::lock_guard<std::mutex> lock(extRecordMgr->preloadUIExtensionHostClientMutex_);
        extRecordMgr->preloadUIExtensionHostClientCallerTokens_[hostPid] = mockCallback->AsObject();
    }

    extRecordMgr->HandlePreloadUIExtensionSuccess(recordId, true);

    EXPECT_TRUE(mockCallback->onPreloadSuccessCalled);
    EXPECT_EQ(mockCallback->lastRequestCode, requestCode);
    EXPECT_EQ(mockCallback->lastErrCode, 0);

    TAG_LOGI(AAFwkTag::TEST, "HandlePreloadUIExtensionSuccess_0200 end");
}

/**
 * @tc.name: HandlePreloadUIExtensionSuccess_0300
 * @tc.desc: Test success logic - isPreloadedSuccess = false.
 *           Should verify error code and that unload logic is triggered (no crash).
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, HandlePreloadUIExtensionSuccess_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandlePreloadUIExtensionSuccess_0300 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.fail";
    abilityRequest.abilityInfo.name = "FailAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    auto extRecord = std::make_shared<UIExtensionRecord>(abilityRecord);

    int32_t recordId = 500;
    int32_t hostPid = 5555;
    
    extRecord->hostPid_ = hostPid;
    extRecordMgr->AddExtensionRecord(recordId, extRecord);
    sptr<MockPreloadCallback> mockCallback = new (std::nothrow) MockPreloadCallback();
    ASSERT_NE(mockCallback, nullptr);
    {
        std::lock_guard<std::mutex> lock(extRecordMgr->preloadUIExtensionHostClientMutex_);
        extRecordMgr->preloadUIExtensionHostClientCallerTokens_[hostPid] = mockCallback->AsObject();
    }
    extRecordMgr->HandlePreloadUIExtensionSuccess(recordId, false);

    EXPECT_TRUE(mockCallback->onPreloadSuccessCalled);
    EXPECT_NE(mockCallback->lastErrCode, 0);
    EXPECT_EQ(mockCallback->lastErrCode, static_cast<int32_t>(AAFwk::INNER_ERR));

    TAG_LOGI(AAFwkTag::TEST, "HandlePreloadUIExtensionSuccess_0300 end");
}

/**
 * @tc.name: ClearPreloadedUIExtensionAbility_0100
 * @tc.desc: Test ID not found.
 *           Expected: AAFwk::ERR_CODE_INVALID_ID
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, ClearPreloadedUIExtensionAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClearPreloadedUIExtensionAbility_0100 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);

    int32_t invalidId = 9999;
    int32_t ret = extRecordMgr->ClearPreloadedUIExtensionAbility(invalidId);

    EXPECT_EQ(ret, AAFwk::ERR_CODE_INVALID_ID);
    TAG_LOGI(AAFwkTag::TEST, "ClearPreloadedUIExtensionAbility_0100 end");
}

/**
 * @tc.name: ClearPreloadedUIExtensionAbility_0200
 * @tc.desc: Test Record is nullptr in map.
 *           Expected: ERR_INVALID_VALUE
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, ClearPreloadedUIExtensionAbility_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClearPreloadedUIExtensionAbility_0200 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);

    int32_t recordId = 100;
    {
        std::lock_guard<std::mutex> lock(extRecordMgr->mutex_);
        extRecordMgr->extensionRecords_[recordId] = nullptr;
    }

    int32_t ret = extRecordMgr->ClearPreloadedUIExtensionAbility(recordId);

    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    EXPECT_EQ(extRecordMgr->extensionRecords_.count(recordId), 0);

    TAG_LOGI(AAFwkTag::TEST, "ClearPreloadedUIExtensionAbility_0200 end");
}

/**
 * @tc.name: ClearPreloadedUIExtensionAbility_0300
 * @tc.desc: Test PID mismatch (Safety check).
 *           Expected: AAFwk::ERR_CODE_INVALID_ID
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, ClearPreloadedUIExtensionAbility_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClearPreloadedUIExtensionAbility_0300 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test";
    abilityRequest.abilityInfo.name = "TestAbility";
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);

    int32_t currentPid = IPCSkeleton::GetCallingPid();
    extRecord->hostPid_ = currentPid + 100;

    int32_t recordId = 200;
    extRecordMgr->AddExtensionRecord(recordId, extRecord);

    int32_t ret = extRecordMgr->ClearPreloadedUIExtensionAbility(recordId);
    EXPECT_EQ(ret, AAFwk::ERR_CODE_INVALID_ID);
    TAG_LOGI(AAFwkTag::TEST, "ClearPreloadedUIExtensionAbility_0300 end");
}

/**
 * @tc.name: ClearPreloadedUIExtensionAbility_0400
 * @tc.desc: Test BaseExtensionRecord is null inside ExtensionRecord.
 *           Expected: ERR_INVALID_VALUE
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, ClearPreloadedUIExtensionAbility_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClearPreloadedUIExtensionAbility_0400 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test";
    abilityRequest.abilityInfo.name = "TestAbility";
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    
    extRecord->abilityRecord_ = nullptr;
    extRecord->hostPid_ = IPCSkeleton::GetCallingPid();

    int32_t recordId = 300;
    extRecordMgr->AddExtensionRecord(recordId, extRecord);

    int32_t ret = extRecordMgr->ClearPreloadedUIExtensionAbility(recordId);

    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    EXPECT_EQ(extRecordMgr->extensionRecords_.count(recordId), 0);

    TAG_LOGI(AAFwkTag::TEST, "ClearPreloadedUIExtensionAbility_0400 end");
}

/**
 * @tc.name: ClearPreloadedUIExtensionAbility_0500
 * @tc.desc: Test RemovePreloadUIExtensionRecordById fail (Map inconsistency).
 *           Record exists in extensionRecords_ but not in the index Map.
 *           Expected: ERR_INVALID_VALUE
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, ClearPreloadedUIExtensionAbility_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClearPreloadedUIExtensionAbility_0500 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.fail";
    abilityRequest.abilityInfo.name = "FailAbility";
    abilityRequest.abilityInfo.moduleName = "entry";
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    
    extRecord->hostPid_ = IPCSkeleton::GetCallingPid();
    int32_t recordId = 400;
    
    extRecordMgr->AddExtensionRecord(recordId, extRecord);

    int32_t ret = extRecordMgr->ClearPreloadedUIExtensionAbility(recordId);

    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "ClearPreloadedUIExtensionAbility_0500 end");
}

/**
 * @tc.name: ConvertToUnloadExtensionRecords0100
 * @tc.desc: Coverage test: Input list contains both valid record and nullptr.
 *           Covers both 'if' (valid) and 'else' (null) branches in one go.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, ConvertToUnloadExtensionRecords0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);

    AAFwk::AbilityRequest request;
    request.abilityInfo.name = "CoverageTest";
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(request);
    auto validRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    std::vector<std::shared_ptr<ExtensionRecord>> sourceRecords;
    sourceRecords.push_back(validRecord);
    sourceRecords.push_back(nullptr);

    std::vector<std::shared_ptr<ExtensionRecord>> recordsToUnload;

    extRecordMgr->ConvertToUnloadExtensionRecords(sourceRecords, recordsToUnload);
    
    EXPECT_EQ(recordsToUnload.size(), 1);
    EXPECT_EQ(sourceRecords.size(), 1);
}

/**
 * @tc.name: UpdateProcessName_0800
 * @tc.desc: UpdateProcessName
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, UpdateProcessName_0800, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = "testBundleName";
    abilityRequest.abilityInfo.name = "testInfoName";
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    std::shared_ptr<ExtensionRecord> extRecord = std::make_shared<ExtensionRecord>(abilityRecord);

    extRecord->processMode_ = PROCESS_MODE_HOST_SPECIFIED;
    abilityRecord->SetAppIndex(1);
    abilityRequest.want.SetParam(PROCESS_MODE_HOST_SPECIFIED_KEY, std::string("processName1"));
    int result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_NE(result, ERR_OK);

    extRecord->processMode_ = PROCESS_MODE_HOST_SPECIFIED;
    abilityRecord->SetAppIndex(1);
    abilityRequest.want.SetParam(PROCESS_MODE_HOST_SPECIFIED_KEY, std::string("processName:1"));
    extRecordMgr->AddExtensionRecord(1, extRecord);
    result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_NE(result, ERR_OK);

    extRecord->processMode_ = PROCESS_MODE_HOST_SPECIFIED;
    abilityRecord->SetAppIndex(1);
    abilityRequest.want.SetParam(PROCESS_MODE_HOST_SPECIFIED_KEY, std::string("processName:1"));
    abilityRecord->SetProcessName("processName");
    extRecordMgr->AddExtensionRecord(1, extRecord);
    result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_NE(result, ERR_OK);

    extRecord->processMode_ = PROCESS_MODE_HOST_SPECIFIED;
    abilityRecord->SetAppIndex(2);
    abilityRequest.want.SetParam(PROCESS_MODE_HOST_SPECIFIED_KEY, std::string("processName:1"));
    abilityRecord->SetProcessName("processName");
    extRecordMgr->AddExtensionRecord(1, extRecord);
    result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_NE(result, ERR_OK);

    extRecord->processMode_ = PROCESS_MODE_HOST_SPECIFIED;
    abilityRecord->SetAppIndex(1);
    abilityRequest.want.SetParam(PROCESS_MODE_HOST_SPECIFIED_KEY, std::string("processName123:1"));
    abilityRecord->SetProcessName("processName");
    extRecordMgr->AddExtensionRecord(1, extRecord);
    result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_NE(result, ERR_OK);

    extRecord->processMode_ = PROCESS_MODE_HOST_SPECIFIED;
    extRecord->abilityRecord_->SetAppIndex(1);
    abilityRecord->SetAppIndex(1);
    abilityRequest.want.SetParam(PROCESS_MODE_HOST_SPECIFIED_KEY, std::string("processName:1"));
    abilityRecord->SetProcessName("processName:1");
    extRecordMgr->AddExtensionRecord(1, extRecord);
    result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: UpdateProcessName_0900
 * @tc.desc: UpdateProcessName
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, UpdateProcessName_0900, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = "com.ohos.sceneboard";
    abilityRequest.abilityInfo.name = "test";
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    std::shared_ptr<ExtensionRecord> extRecord = std::make_shared<ExtensionRecord>(abilityRecord);

    extRecord->processMode_ = PROCESS_MODE_HOST_SPECIFIED;
    abilityRecord->SetAppIndex(2);
    abilityRequest.want.SetParam(PROCESS_MODE_HOST_SPECIFIED_KEY, std::string("processName2"));
    int result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_NE(result, ERR_OK);

    extRecord->processMode_ = PROCESS_MODE_HOST_SPECIFIED;
    abilityRecord->SetAppIndex(2);
    abilityRequest.want.SetParam(PROCESS_MODE_HOST_SPECIFIED_KEY, std::string("processName:2"));
    abilityRecord->SetProcessName("newProcessName");
    extRecordMgr->AddExtensionRecord(1, extRecord);
    result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_NE(result, ERR_OK);

    extRecord->processMode_ = PROCESS_MODE_HOST_SPECIFIED;
    abilityRecord->SetAppIndex(1);
    abilityRequest.want.SetParam(PROCESS_MODE_HOST_SPECIFIED_KEY, std::string("processName123:2"));
    abilityRecord->SetProcessName("processName");
    extRecordMgr->AddExtensionRecord(1, extRecord);
    result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_NE(result, ERR_OK);
}

/**
 * @tc.name: CheckAgentUILaunchLimit_0100
 * @tc.desc: Test CheckAgentUILaunchLimit when no record exists.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, CheckAgentUILaunchLimit_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckAgentUILaunchLimit_0100 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    int32_t callerUid = 1001;
    std::string bundleName = "com.test.agentui";

    // No record exists, should return ERR_OK
    auto ret = extRecordMgr->CheckAgentUILaunchLimit(callerUid, bundleName);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "CheckAgentUILaunchLimit_0100 end");
}

/**
 * @tc.name: CheckAgentUILaunchLimit_0200
 * @tc.desc: Test CheckAgentUILaunchLimit when callerUid exists but bundleName not exists.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, CheckAgentUILaunchLimit_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckAgentUILaunchLimit_0200 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    int32_t callerUid = 1002;
    std::string bundleName1 = "com.test.bundle1";
    std::string bundleName2 = "com.test.bundle2";

    // Add record for bundle1
    extRecordMgr->AddAgentUILaunchRecord(callerUid, bundleName1, 1);

    // Check for bundle2 (not exists), should return ERR_OK
    auto ret = extRecordMgr->CheckAgentUILaunchLimit(callerUid, bundleName2);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "CheckAgentUILaunchLimit_0200 end");
}

/**
 * @tc.name: CheckAgentUILaunchLimit_0300
 * @tc.desc: Test CheckAgentUILaunchLimit when record exists and under limit.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, CheckAgentUILaunchLimit_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckAgentUILaunchLimit_0300 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    int32_t callerUid = 1003;
    std::string bundleName = "com.test.bundle";

    // Add one record
    extRecordMgr->AddAgentUILaunchRecord(callerUid, bundleName, 1);

    // Should return ERR_OK (under limit)
    auto ret = extRecordMgr->CheckAgentUILaunchLimit(callerUid, bundleName);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "CheckAgentUILaunchLimit_0300 end");
}

/**
 * @tc.name: AddAgentUILaunchRecord_0100
 * @tc.desc: Test AddAgentUILaunchRecord add first record for new callerUid.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, AddAgentUILaunchRecord_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AddAgentUILaunchRecord_0100 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    int32_t callerUid = 2001;
    std::string bundleName = "com.test.add";
    int32_t extensionAbilityId = 100;

    // Add first record
    extRecordMgr->AddAgentUILaunchRecord(callerUid, bundleName, extensionAbilityId);

    // Verify record was added
    auto ret = extRecordMgr->CheckAgentUILaunchLimit(callerUid, bundleName);
    EXPECT_EQ(ret, ERR_OK);

    // Check internal map size
    EXPECT_EQ(extRecordMgr->agentUIExtensionRecords_.size(), 1);
    TAG_LOGI(AAFwkTag::TEST, "AddAgentUILaunchRecord_0100 end");
}

/**
 * @tc.name: AddAgentUILaunchRecord_0200
 * @tc.desc: Test AddAgentUILaunchRecord add multiple records for same callerUid and bundleName.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, AddAgentUILaunchRecord_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AddAgentUILaunchRecord_0200 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    int32_t callerUid = 2002;
    std::string bundleName = "com.test.multi";

    // Add multiple records
    extRecordMgr->AddAgentUILaunchRecord(callerUid, bundleName, 1);
    extRecordMgr->AddAgentUILaunchRecord(callerUid, bundleName, 2);
    extRecordMgr->AddAgentUILaunchRecord(callerUid, bundleName, 3);

    // Verify all records were added
    EXPECT_EQ(extRecordMgr->agentUIExtensionRecords_[callerUid][bundleName].size(), 3);

    TAG_LOGI(AAFwkTag::TEST, "AddAgentUILaunchRecord_0200 end");
}

/**
 * @tc.name: AddAgentUILaunchRecord_0300
 * @tc.desc: Test AddAgentUILaunchRecord add duplicate record (same extensionAbilityId).
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, AddAgentUILaunchRecord_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AddAgentUILaunchRecord_0300 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    int32_t callerUid = 2003;
    std::string bundleName = "com.test.dup";
    int32_t extensionAbilityId = 100;

    // Add same record twice
    extRecordMgr->AddAgentUILaunchRecord(callerUid, bundleName, extensionAbilityId);
    extRecordMgr->AddAgentUILaunchRecord(callerUid, bundleName, extensionAbilityId);

    // Should only have one record (duplicate ignored)
    EXPECT_EQ(extRecordMgr->agentUIExtensionRecords_[callerUid][bundleName].size(), 1);

    TAG_LOGI(AAFwkTag::TEST, "AddAgentUILaunchRecord_0300 end");
}

/**
 * @tc.name: RemoveAgentUILaunchRecord_0100
 * @tc.desc: Test RemoveAgentUILaunchRecord when callerUid not exists.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, RemoveAgentUILaunchRecord_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoveAgentUILaunchRecord_0100 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    int32_t callerUid = 3001;
    std::string bundleName = "com.test.remove";
    int32_t extensionAbilityId = 100;

    // Remove from empty map, should not crash
    extRecordMgr->RemoveAgentUILaunchRecord(callerUid, bundleName, extensionAbilityId);

    // Map should still be empty
    EXPECT_EQ(extRecordMgr->agentUIExtensionRecords_.size(), 0);

    TAG_LOGI(AAFwkTag::TEST, "RemoveAgentUILaunchRecord_0100 end");
}

/**
 * @tc.name: RemoveAgentUILaunchRecord_0200
 * @tc.desc: Test RemoveAgentUILaunchRecord when bundleName not exists.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, RemoveAgentUILaunchRecord_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoveAgentUILaunchRecord_0200 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    int32_t callerUid = 3002;
    std::string bundleName1 = "com.test.bundle1";
    std::string bundleName2 = "com.test.bundle2";
    int32_t extensionAbilityId = 100;

    // Add record for bundle1
    extRecordMgr->AddAgentUILaunchRecord(callerUid, bundleName1, extensionAbilityId);

    // Remove from bundle2 (not exists), should not crash
    extRecordMgr->RemoveAgentUILaunchRecord(callerUid, bundleName2, extensionAbilityId);

    // bundle1 record should still exist
    EXPECT_EQ(extRecordMgr->agentUIExtensionRecords_[callerUid][bundleName1].size(), 1);

    TAG_LOGI(AAFwkTag::TEST, "RemoveAgentUILaunchRecord_0200 end");
}

/**
 * @tc.name: RemoveAgentUILaunchRecord_0300
 * @tc.desc: Test RemoveAgentUILaunchRecord remove existing record.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, RemoveAgentUILaunchRecord_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoveAgentUILaunchRecord_0300 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    int32_t callerUid = 3003;
    std::string bundleName = "com.test.remove";
    int32_t extensionAbilityId = 100;

    // Add record
    extRecordMgr->AddAgentUILaunchRecord(callerUid, bundleName, extensionAbilityId);
    EXPECT_EQ(extRecordMgr->agentUIExtensionRecords_[callerUid][bundleName].size(), 1);

    // Remove record
    extRecordMgr->RemoveAgentUILaunchRecord(callerUid, bundleName, extensionAbilityId);

    // Record should be removed, and map entries should be cleaned up
    EXPECT_EQ(extRecordMgr->agentUIExtensionRecords_.count(callerUid), 0);

    TAG_LOGI(AAFwkTag::TEST, "RemoveAgentUILaunchRecord_0300 end");
}

/**
 * @tc.name: RemoveAgentUILaunchRecord_0400
 * @tc.desc: Test RemoveAgentUILaunchRecord remove one of multiple records.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, RemoveAgentUILaunchRecord_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoveAgentUILaunchRecord_0400 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    int32_t callerUid = 3004;
    std::string bundleName = "com.test.multi";

    // Add multiple records
    extRecordMgr->AddAgentUILaunchRecord(callerUid, bundleName, 1);
    extRecordMgr->AddAgentUILaunchRecord(callerUid, bundleName, 2);
    extRecordMgr->AddAgentUILaunchRecord(callerUid, bundleName, 3);
    EXPECT_EQ(extRecordMgr->agentUIExtensionRecords_[callerUid][bundleName].size(), 3);

    // Remove one record
    extRecordMgr->RemoveAgentUILaunchRecord(callerUid, bundleName, 2);
    EXPECT_EQ(extRecordMgr->agentUIExtensionRecords_[callerUid][bundleName].size(), 2);

    // Verify the correct record was removed
    auto& recordSet = extRecordMgr->agentUIExtensionRecords_[callerUid][bundleName];
    EXPECT_EQ(recordSet.count(1), 1);
    EXPECT_EQ(recordSet.count(2), 0);
    EXPECT_EQ(recordSet.count(3), 1);

    TAG_LOGI(AAFwkTag::TEST, "RemoveAgentUILaunchRecord_0400 end");
}

/**
 * @tc.name: RemoveAgentUILaunchRecord_0500
 * @tc.desc: Test RemoveAgentUILaunchRecord remove non-existent extensionAbilityId.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, RemoveAgentUILaunchRecord_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoveAgentUILaunchRecord_0500 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    int32_t callerUid = 3005;
    std::string bundleName = "com.test.remove";
    int32_t extensionAbilityId = 100;

    // Add record
    extRecordMgr->AddAgentUILaunchRecord(callerUid, bundleName, extensionAbilityId);

    // Remove non-existent extensionAbilityId
    extRecordMgr->RemoveAgentUILaunchRecord(callerUid, bundleName, 999);

    // Original record should still exist
    EXPECT_EQ(extRecordMgr->agentUIExtensionRecords_[callerUid][bundleName].size(), 1);

    TAG_LOGI(AAFwkTag::TEST, "RemoveAgentUILaunchRecord_0500 end");
}
} // namespace AbilityRuntime
} // namespace OHOS
