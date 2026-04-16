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
#include <thread>
#include <chrono>

#include "hilog_tag_wrapper.h"
#include "ability_manager_client.h"

#define private public
#define protected public
#define inline
#include "extension_record.h"
#include "extension_record_manager.h"
#include "extension_running_timeout_monitor.h"
#include "extension_config.h"
#define inline
#undef protected
#undef private

#include "mock_ability_token.h"
#include "mock_native_token.h"
#include "scene_board_judgement.h"
#include "session_info.h"
#include "bool_wrapper.h"
#include "mock_my_flag.h"
#include "mock_permission_verification.h"
#include "mock_sa_call.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class ExtensionRecordManagerSecondTest : public testing::Test {
public:

    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void ExtensionRecordManagerSecondTest::SetUpTestCase(void)
{}

void ExtensionRecordManagerSecondTest::TearDownTestCase(void)
{}

void ExtensionRecordManagerSecondTest::SetUp()
{}

void ExtensionRecordManagerSecondTest::TearDown()
{}

/**
 * @tc.name: RemovePreloadUIExtensionRecord_0100
 * @tc.desc: Test RemovePreloadUIExtensionRecord.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerSecondTest, RemovePreloadUIExtensionRecord_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ExtensionRecordManager::PreLoadUIExtensionMapKey extensionRecordMapKey;
    EXPECT_FALSE(extRecordMgr->RemovePreloadUIExtensionRecord(extensionRecordMapKey));

    std::vector<std::shared_ptr<ExtensionRecord>> v_ExtensionRecord;
    extRecordMgr->preloadUIExtensionMap_[extensionRecordMapKey] = v_ExtensionRecord;
    EXPECT_TRUE(extRecordMgr->RemovePreloadUIExtensionRecord(extensionRecordMapKey));

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    v_ExtensionRecord.push_back(extRecord);
    extRecordMgr->preloadUIExtensionMap_[extensionRecordMapKey] = v_ExtensionRecord;
    EXPECT_TRUE(extRecordMgr->RemovePreloadUIExtensionRecord(extensionRecordMapKey));
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GetRootCallerTokenLocked_0100
 * @tc.desc: Test GetRootCallerTokenLocked.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerSecondTest, GetRootCallerTokenLocked_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    int32_t extensionRecordId = 0;
    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_EQ(extRecordMgr->GetRootCallerTokenLocked(extensionRecordId, abilityRecord), nullptr);

    std::shared_ptr<ExtensionRecord> extRecord = nullptr;
    extRecordMgr->extensionRecords_[extensionRecordId] = extRecord;
    EXPECT_EQ(extRecordMgr->GetRootCallerTokenLocked(extensionRecordId, abilityRecord), nullptr);

    extRecordMgr->extensionRecords_.clear();
    extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    sptr<IRemoteObject> rootCallerToken = nullptr;
    extRecord->SetRootCallerToken(rootCallerToken);
    extRecordMgr->extensionRecords_[extensionRecordId] = extRecord;
    EXPECT_EQ(extRecordMgr->GetRootCallerTokenLocked(extensionRecordId, abilityRecord), nullptr);

    rootCallerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    extRecord->SetRootCallerToken(rootCallerToken);
    extRecordMgr->extensionRecords_.clear();
    extRecordMgr->extensionRecords_[extensionRecordId] = extRecord;
    EXPECT_NE(extRecordMgr->GetRootCallerTokenLocked(extensionRecordId, abilityRecord), nullptr);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GetUIExtensionSessionInfo_0100
 * @tc.desc: Test GetUIExtensionSessionInfo.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerSecondTest, GetUIExtensionSessionInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    sptr<IRemoteObject> token = nullptr;
    UIExtensionSessionInfo uiExtensionSessionInfo;
    EXPECT_EQ(extRecordMgr->GetUIExtensionSessionInfo(token, uiExtensionSessionInfo), ERR_NULL_OBJECT);

    token = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    EXPECT_EQ(extRecordMgr->GetUIExtensionSessionInfo(token, uiExtensionSessionInfo), ERR_NULL_OBJECT);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    token = abilityRecord->GetToken();
    EXPECT_EQ(extRecordMgr->GetUIExtensionSessionInfo(token, uiExtensionSessionInfo), ERR_INVALID_VALUE);

    abilityRecord->abilityInfo_.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SHARE;
    EXPECT_EQ(extRecordMgr->GetUIExtensionSessionInfo(token, uiExtensionSessionInfo), ERR_NULL_OBJECT);

    abilityRecord->sessionInfo_ = sptr<AAFwk::SessionInfo>::MakeSptr();
    EXPECT_EQ(extRecordMgr->GetUIExtensionSessionInfo(token, uiExtensionSessionInfo), ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: CreateExtensionRecord_0100
 * @tc.desc: Test CreateExtensionRecord.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerSecondTest, CreateExtensionRecord_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    AAFwk::AbilityRequest abilityRequest;
    std::string hostBundleName("hostBundleName");
    std::shared_ptr<ExtensionRecord> extensionRecord;
    int32_t extensionRecordId = 0;
    EXPECT_EQ(extRecordMgr->CreateExtensionRecord(
        abilityRequest, hostBundleName, extensionRecord, extensionRecordId), ERR_INVALID_VALUE);

    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    abilityRequest.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SHARE;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    extensionRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    EXPECT_EQ(extRecordMgr->CreateExtensionRecord(
        abilityRequest, hostBundleName, extensionRecord, extensionRecordId), ERR_OK);

    abilityRequest.extensionType = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    EXPECT_EQ(extRecordMgr->CreateExtensionRecord(
        abilityRequest, hostBundleName, extensionRecord, extensionRecordId), ERR_INVALID_VALUE);

    abilityRequest.extensionType = AppExecFwk::ExtensionAbilityType::SHARE;
    std::string key("ability.want.params.is_preload_uiextension_ability");
    AAFwk::WantParams wantParams;
    wantParams.SetParam(key, AAFwk::Boolean::Box(true));
    AAFwk::Want want;
    want.SetParams(wantParams);
    abilityRequest.want = want;
    EXPECT_EQ(extRecordMgr->CreateExtensionRecord(
        abilityRequest, hostBundleName, extensionRecord, extensionRecordId), ERR_INVALID_VALUE);

    AAFwk::IsMockSaCall::IsMockSaCallWithPermission();
    AAFwk::MyFlag::flag_ = AAFwk::MyFlag::IS_SA_CALL;
    EXPECT_EQ(extRecordMgr->CreateExtensionRecord(
        abilityRequest, hostBundleName, extensionRecord, extensionRecordId), ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: RemovePreloadUIExtensionRecordById_0100
 * @tc.desc: Test RemovePreloadUIExtensionRecordById.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerSecondTest, RemovePreloadUIExtensionRecordById_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    std::tuple<std::string, std::string, std::string, int32_t> extensionRecordMapKey;
    int32_t extensionRecordId = 0;
    EXPECT_FALSE(extRecordMgr->RemovePreloadUIExtensionRecordById(extensionRecordMapKey, extensionRecordId));

    std::vector<std::shared_ptr<ExtensionRecord>> v_ExtensionRecord;
    extRecordMgr->preloadUIExtensionMap_[extensionRecordMapKey] = v_ExtensionRecord;
    EXPECT_FALSE(extRecordMgr->RemovePreloadUIExtensionRecordById(extensionRecordMapKey, extensionRecordId));

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    extRecord->extensionRecordId_ = 0;
    v_ExtensionRecord.push_back(extRecord);
    extRecordMgr->preloadUIExtensionMap_[extensionRecordMapKey] = v_ExtensionRecord;
    EXPECT_TRUE(extRecordMgr->RemovePreloadUIExtensionRecordById(extensionRecordMapKey, extensionRecordId));

    extRecord->extensionRecordId_ = 1;
    v_ExtensionRecord.clear();
    v_ExtensionRecord.push_back(extRecord);
    extRecordMgr->preloadUIExtensionMap_[extensionRecordMapKey] = v_ExtensionRecord;
    EXPECT_FALSE(extRecordMgr->RemovePreloadUIExtensionRecordById(extensionRecordMapKey, extensionRecordId));
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GetHostBundleNameForExtensionId_0100
 * @tc.desc: GetHostBundleNameForExtensionId
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerSecondTest, GetHostBundleNameForExtensionId_0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    extRecordMgr->extensionRecords_.clear();
    int32_t hostPid = 0;

    auto ret = extRecordMgr->GetHostPidForExtensionId(1, hostPid);

    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: RemoveAllPreloadUIExtensionRecord_0100
 * @tc.desc: RemoveAllPreloadUIExtensionRecord
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerSecondTest, RemoveAllPreloadUIExtensionRecord_0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ExtensionRecordManager::PreLoadUIExtensionMapKey preLoadUIExtensionInfo("1", "2", "3", 4);
    extRecordMgr->preloadUIExtensionMap_ = {
        {preLoadUIExtensionInfo, {}}
    };

    extRecordMgr->RemoveAllPreloadUIExtensionRecord(preLoadUIExtensionInfo);

    EXPECT_EQ(extRecordMgr->preloadUIExtensionMap_.size(), 0);
}

/**
 * @tc.name: RemoveAllPreloadUIExtensionRecord_0200
 * @tc.desc: RemoveAllPreloadUIExtensionRecord
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerSecondTest, RemoveAllPreloadUIExtensionRecord_0200, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ExtensionRecordManager::PreLoadUIExtensionMapKey preLoadUIExtensionInfo1("1", "2", "3", 4);
    ExtensionRecordManager::PreLoadUIExtensionMapKey preLoadUIExtensionInfo2("5", "6", "7", 8);
    extRecordMgr->preloadUIExtensionMap_ = {
        {preLoadUIExtensionInfo1, {}}
    };

    extRecordMgr->RemoveAllPreloadUIExtensionRecord(preLoadUIExtensionInfo2);

    EXPECT_EQ(extRecordMgr->preloadUIExtensionMap_.size(), 1);
}

/**
 * @tc.name: RemovePreloadUIExtensionRecordById_0300
 * @tc.desc: RemovePreloadUIExtensionRecordById
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerSecondTest, RemovePreloadUIExtensionRecordById_0300, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    std::tuple<std::string, std::string, std::string, int32_t> extensionRecordMapKey = {
        "1", "2", "3", 4
    };
    int32_t extensionRecordId = 1;
    ExtensionRecordManager::PreLoadUIExtensionMapKey preLoadUIExtensionInfo1("1", "2", "3", 4);
    ExtensionRecordManager::PreLoadUIExtensionMapKey preLoadUIExtensionInfo2("5", "6", "7", 8);
    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    auto extensionRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    extensionRecord->extensionRecordId_ = extensionRecordId;
    std::vector<std::shared_ptr<ExtensionRecord>> extensionRecords = {
        extensionRecord
    };
    extRecordMgr->preloadUIExtensionMap_ = {
        {preLoadUIExtensionInfo1, extensionRecords}
    };

    auto ret = extRecordMgr->RemovePreloadUIExtensionRecordById(extensionRecordMapKey, extensionRecordId);

    EXPECT_EQ(extRecordMgr->preloadUIExtensionMap_.size(), 0);
    EXPECT_EQ(ret, true);
}


/**
 * @tc.name: RemovePreloadUIExtensionRecordById_0400
 * @tc.desc: RemovePreloadUIExtensionRecordById
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerSecondTest, RemovePreloadUIExtensionRecordById_0400, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    std::tuple<std::string, std::string, std::string, int32_t> extensionRecordMapKey = {
        "1", "2", "3", 4
    };
    int32_t extensionRecordId = 1;
    int32_t extensionRecordId2 = 2;
    ExtensionRecordManager::PreLoadUIExtensionMapKey preLoadUIExtensionInfo1("1", "2", "3", 4);
    ExtensionRecordManager::PreLoadUIExtensionMapKey preLoadUIExtensionInfo2("5", "6", "7", 8);
    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    auto extensionRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    extensionRecord->extensionRecordId_ = extensionRecordId;
    auto extensionRecord2 = std::make_shared<ExtensionRecord>(abilityRecord);
    extensionRecord2->extensionRecordId_ = extensionRecordId2;
    std::vector<std::shared_ptr<ExtensionRecord>> extensionRecords = {
        extensionRecord, extensionRecord2
    };
    extRecordMgr->preloadUIExtensionMap_ = {
        {preLoadUIExtensionInfo1, extensionRecords}
    };

    auto ret = extRecordMgr->RemovePreloadUIExtensionRecordById(extensionRecordMapKey, extensionRecordId);

    EXPECT_EQ(extRecordMgr->preloadUIExtensionMap_.size(), 1);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: RemovePreloadUIExtensionRecordById_0500
 * @tc.desc: RemovePreloadUIExtensionRecordById
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerSecondTest, RemovePreloadUIExtensionRecordById_0500, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);

    std::tuple<std::string, std::string, std::string, int32_t> extensionRecordMapKey = {
        "1", "2", "3", 4
    };
    int32_t extensionRecordId = 1;
    int32_t extensionRecordId2 = 2;
    int32_t extensionRecordId3 = 3;
    ExtensionRecordManager::PreLoadUIExtensionMapKey preLoadUIExtensionInfo1("1", "2", "3", 4);
    ExtensionRecordManager::PreLoadUIExtensionMapKey preLoadUIExtensionInfo2("5", "6", "7", 8);
    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    auto extensionRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    extensionRecord->extensionRecordId_ = extensionRecordId;
    auto extensionRecord2 = std::make_shared<ExtensionRecord>(abilityRecord);
    extensionRecord2->extensionRecordId_ = extensionRecordId2;
    std::vector<std::shared_ptr<ExtensionRecord>> extensionRecords = {
        extensionRecord, extensionRecord2
    };
    extRecordMgr->preloadUIExtensionMap_ = {
        {preLoadUIExtensionInfo1, extensionRecords}
    };

    auto ret = extRecordMgr->RemovePreloadUIExtensionRecordById(extensionRecordMapKey, extensionRecordId3);

    EXPECT_EQ(extRecordMgr->preloadUIExtensionMap_.size(), 1);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: RemovePreloadUIExtensionRecord_0200
 * @tc.desc: RemovePreloadUIExtensionRecord
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerSecondTest, RemovePreloadUIExtensionRecord_0200, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);

    std::tuple<std::string, std::string, std::string, int32_t> extensionRecordMapKey = {
        "1", "2", "3", 4
    };
    extRecordMgr->preloadUIExtensionMap_.clear();

    auto ret = extRecordMgr->RemovePreloadUIExtensionRecord(extensionRecordMapKey);

    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: GetRootCallerTokenLocked_0200
 * @tc.desc: GetRootCallerTokenLocked
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerSecondTest, GetRootCallerTokenLocked_0200, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);

    int32_t extensionRecordId = 0;
    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    extRecordMgr->extensionRecords_.clear();

    auto ret = extRecordMgr->GetRootCallerTokenLocked(extensionRecordId, abilityRecord);

    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: GetUIExtensionRootHostInfo_0100
 * @tc.desc: GetUIExtensionRootHostInfo
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerSecondTest, GetUIExtensionRootHostInfo_0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);

    auto ret = extRecordMgr->GetUIExtensionRootHostInfo(nullptr);

    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: GetUIExtensionSessionInfo_0200
 * @tc.desc: GetUIExtensionSessionInfo
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerSecondTest, GetUIExtensionSessionInfo_0200, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    UIExtensionSessionInfo uiExtensionSessionInfo;

    auto ret = extRecordMgr->GetUIExtensionSessionInfo(nullptr, uiExtensionSessionInfo);

    EXPECT_EQ(ret, ERR_NULL_OBJECT);
}

/**
 * @tc.name: GetExtensionRecordById_0100
 * @tc.desc: GetExtensionRecordById
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerSecondTest, GetExtensionRecordById_0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);

    int32_t extensionRecordId = 1;
    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);

    auto extensionRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    extensionRecord->extensionRecordId_ = extensionRecordId;

    extRecordMgr->extensionRecords_ = {
        {extensionRecordId, extensionRecord}
    };

    auto ret = extRecordMgr->GetExtensionRecordById(extensionRecordId);

    EXPECT_EQ(ret, extensionRecord);
}

/**
 * @tc.name: GetExtensionRecordById_0200
 * @tc.desc: GetExtensionRecordById
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerSecondTest, GetExtensionRecordById_0200, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);

    int32_t extensionRecordId = 1;
    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    auto extensionRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    extensionRecord->extensionRecordId_ = extensionRecordId;
    extRecordMgr->extensionRecords_.clear();
    extRecordMgr->terminateRecords_.clear();

    auto ret = extRecordMgr->GetExtensionRecordById(extensionRecordId);

    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: GetExtensionRecordById_0300
 * @tc.desc: GetExtensionRecordById
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerSecondTest, GetExtensionRecordById_0300, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    int32_t extensionRecordId = 1;

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    auto extensionRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    extensionRecord->extensionRecordId_ = extensionRecordId;
    extRecordMgr->extensionRecords_.clear();
    extRecordMgr->terminateRecords_ = {
        {extensionRecordId, extensionRecord}
    };

    auto ret = extRecordMgr->GetExtensionRecordById(extensionRecordId);

    EXPECT_EQ(ret, extensionRecord);
}

/**
 * @tc.name: QueryPreLoadUIExtensionRecord_0100
 * @tc.desc: QueryPreLoadUIExtensionRecord
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerSecondTest, QueryPreLoadUIExtensionRecord_0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);

    AppExecFwk::ElementName element;
    element.SetAbilityName("");
    element.SetBundleName("HeavenlyMe");
    std::string moduleName = "HeavenlyMe";
    int32_t hostPid = 0;
    int32_t recordNum = 1;

    auto ret = extRecordMgr->QueryPreLoadUIExtensionRecord(
        element, moduleName, hostPid, recordNum);

    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: QueryPreLoadUIExtensionRecord_0200
 * @tc.desc: QueryPreLoadUIExtensionRecord
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerSecondTest, QueryPreLoadUIExtensionRecord_0200, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);

    AppExecFwk::ElementName element;
    element.SetAbilityName("HeavenlyMe");
    element.SetBundleName("HeavenlyMe");
    std::string moduleName = "HeavenlyMe";
    int32_t hostPid = 0;
    int32_t recordNum = 1;
    extRecordMgr->preloadUIExtensionMap_.clear();

    auto ret = extRecordMgr->QueryPreLoadUIExtensionRecord(
        element, moduleName, hostPid, recordNum);

    EXPECT_EQ(recordNum, 0);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: QueryPreLoadUIExtensionRecord_0300
 * @tc.desc: QueryPreLoadUIExtensionRecord
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerSecondTest, QueryPreLoadUIExtensionRecord_0300, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);

    AppExecFwk::ElementName element;
    element.SetAbilityName("HeavenlyMeAbility");
    element.SetBundleName("HeavenlyMeBundle");
    std::string moduleName = "HeavenlyMeMoudle";
    int32_t hostPid = 0;
    int32_t recordNum = 1;
    ExtensionRecordManager::PreLoadUIExtensionMapKey extensionMapKey(
        "HeavenlyMeAbility", "HeavenlyMeBundle", "HeavenlyMeMoudle", hostPid);
    extRecordMgr->preloadUIExtensionMap_ = {
        {extensionMapKey, {}}
    };

    auto ret = extRecordMgr->QueryPreLoadUIExtensionRecord(
        element, moduleName, hostPid, recordNum);

    EXPECT_EQ(recordNum, 0);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: QueryPreLoadUIExtensionRecord_0400
 * @tc.desc: QueryPreLoadUIExtensionRecord
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerSecondTest, QueryPreLoadUIExtensionRecord_0400, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);

    AppExecFwk::ElementName element;
    element.SetAbilityName("HeavenlyMeAbility");
    element.SetBundleName("HeavenlyMeBundle");
    std::string moduleName = "HeavenlyMeMoudle";
    int32_t hostPid = 100;
    int32_t recordNum = 1;
    ExtensionRecordManager::PreLoadUIExtensionMapKey extensionMapKey(
        "HeavenlyMeAbility", "HeavenlyMeBundle", "HeavenlyMeMoudle", hostPid);
    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    auto extensionRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    extRecordMgr->preloadUIExtensionMap_ = {
        {extensionMapKey, {extensionRecord}}
    };

    auto ret = extRecordMgr->QueryPreLoadUIExtensionRecord(
        element, moduleName, hostPid, recordNum);

    EXPECT_EQ(recordNum, 1);
    EXPECT_EQ(ret, ERR_OK);
}


/**
 * @tc.name: GetOrCreateExtensionRecordInner_0100
 * @tc.desc: Test GetOrCreateExtensionRecordInner.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerSecondTest, GetOrCreateExtensionRecordInner_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    std::string bundleName = "com.example.unittest";
    std::string name = "MainAbility";
    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = bundleName;
    abilityRequest.abilityInfo.name = name;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    abilityRequest.abilityInfo.extensionAbilityType = static_cast<AppExecFwk::ExtensionAbilityType>(1000);

    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    std::shared_ptr<ExtensionRecord> extRecord = std::make_shared<ExtensionRecord>(abilityRecord);

    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    bool boolResult;
    EXPECT_EQ(extRecordMgr->GetOrCreateExtensionRecordInner(abilityRequest, bundleName, extRecord, boolResult),
        ERR_INVALID_VALUE);

    abilityRequest.extensionType = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    abilityRequest.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    EXPECT_EQ(extRecordMgr->GetOrCreateExtensionRecordInner(abilityRequest, bundleName, extRecord, boolResult),
        ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GetOrCreateExtensionRecordInner_0200
 * @tc.desc: Test GetOrCreateExtensionRecordInner.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerSecondTest, GetOrCreateExtensionRecordInner_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    std::string bundleName = "com.example.unittest";
    std::string name = "MainAbility";
    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = bundleName;
    abilityRequest.abilityInfo.name = name;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    abilityRequest.extensionType = AppExecFwk::ExtensionAbilityType::STATUS_BAR_VIEW;
    abilityRequest.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::STATUS_BAR_VIEW;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    std::shared_ptr<ExtensionRecord> extRecord = std::make_shared<ExtensionRecord>(abilityRecord);

    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    bool boolResult;
    EXPECT_EQ(extRecordMgr->GetOrCreateExtensionRecordInner(abilityRequest, bundleName, extRecord, boolResult),
        ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GetHostBundleNameForExtensionId_0200
 * @tc.desc: Test GetHostBundleNameForExtensionId.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerSecondTest, GetHostBundleNameForExtensionId_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    int32_t extensionRecordId = 0;
    int32_t hostPid = 0;
    std::string bundleName = "com.example.unittest";

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = bundleName;
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    std::shared_ptr<ExtensionRecord> extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    EXPECT_EQ(extRecordMgr->GetHostPidForExtensionId(extensionRecordId, hostPid), ERR_INVALID_VALUE);
    extRecordMgr->extensionRecords_[extensionRecordId] = nullptr;
    EXPECT_EQ(extRecordMgr->GetHostPidForExtensionId(extensionRecordId, hostPid), ERR_INVALID_VALUE);
    extRecordMgr->extensionRecords_[extensionRecordId] = extRecord;
    EXPECT_EQ(extRecordMgr->GetHostPidForExtensionId(extensionRecordId, hostPid), ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: IsHostSpecifiedProcessValid_SceneBoard_0100
 * @tc.desc: Test IsHostSpecifiedProcessValid with SceneBoard caller.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerSecondTest, IsHostSpecifiedProcessValid_SceneBoard_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    std::shared_ptr<ExtensionRecord> extRecord = std::make_shared<ExtensionRecord>(abilityRecord);

    std::string process = "testProcess";
    abilityRecord->SetProcessName(process);
    abilityRecord->SetAppIndex(1);

    // Create caller record as SceneBoard
    AAFwk::AbilityRequest callerRequest;
    callerRequest.appInfo.bundleName = "com.ohos.sceneboard";
    callerRequest.abilityInfo.name = "SceneBoardAbility";
    auto callerAbilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(callerRequest);
    sptr<IRemoteObject> callerToken = callerAbilityRecord->GetToken();
    abilityRequest.callerToken = callerToken;

    // Set different appIndex
    abilityRequest.want.SetParam(AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY, 2);

    extRecordMgr->extensionRecords_[1] = extRecord;

    bool result = extRecordMgr->IsHostSpecifiedProcessValid(abilityRequest, extRecord, process);
    EXPECT_FALSE(result);

    // Set matching appIndex
    abilityRequest.want.SetParam(AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY, 1);
    result = extRecordMgr->IsHostSpecifiedProcessValid(abilityRequest, extRecord, process);
    EXPECT_TRUE(result);

    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: UpdateProcessName_NullRecord_0100
 * @tc.desc: Test UpdateProcessName with null record.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerSecondTest, UpdateProcessName_NullRecord_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = "testBundleName";
    abilityRequest.abilityInfo.name = "testInfoName";

    std::shared_ptr<ExtensionRecord> extRecord = nullptr;
    auto result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: UpdateProcessName_0400
 * @tc.desc: Test UpdateProcessName with moduleProcess set.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerSecondTest, UpdateProcessName_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = "testBundleName";
    abilityRequest.abilityInfo.name = "testInfoName";
    abilityRequest.moduleProcess = "testModuleProcess";

    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    std::shared_ptr<ExtensionRecord> extRecord = std::make_shared<ExtensionRecord>(abilityRecord);

    auto result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(abilityRecord->GetProcessName(), "testModuleProcess");

    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GetCallerTokenList_0200
 * @tc.desc: Test GetCallerTokenList.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerSecondTest, GetCallerTokenList_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.loop";
    abilityRequest.abilityInfo.name = "LoopAbility";
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    int32_t recordId = 400;
    abilityRecord->SetUIExtensionAbilityId(recordId);

    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->callerToken = abilityRecord->GetToken();
    abilityRecord->SetSessionInfo(sessionInfo);

    auto callerAbilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    callerAbilityRecord->SetUIExtensionAbilityId(recordId);

    std::list<sptr<IRemoteObject>> callerList;
    extRecordMgr->GetCallerTokenList(abilityRecord, callerList);

    EXPECT_EQ(callerList.empty(), true);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GetOrCreateExtensionRecordInner_0300
 * @tc.desc: Test GetOrCreateExtensionRecordInner.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerSecondTest, GetOrCreateExtensionRecordInner_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.reuse";
    abilityRequest.abilityInfo.name = "ReuseAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    abilityRequest.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SYS_COMMON_UI;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    int32_t reuseId = 500;
    extRecord->hostBundleName_ = "com.test.reuse";
    extRecordMgr->AddExtensionRecord(reuseId, extRecord);

    std::string hostBundleName = "com.test.reuse";
    std::shared_ptr<ExtensionRecord> outExtRecord;
    bool isLoaded = false;
    int32_t ret = extRecordMgr->GetOrCreateExtensionRecordInner(abilityRequest, hostBundleName, outExtRecord, isLoaded);

    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: ClearAllPreloadUIExtensionRecordForHost_0100
 * @tc.desc: Test ClearAllPreloadUIExtensionRecordForHost.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerSecondTest, ClearAllPreloadUIExtensionRecordForHost_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    int32_t hostPid = IPCSkeleton::GetCallingPid();
    ExtensionRecordManager::PreLoadUIExtensionMapKey mapKey("TestAbility", "com.test.clear", "entry", hostPid);
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(AAFwk::AbilityRequest());
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    std::vector<std::shared_ptr<ExtensionRecord>> records = {extRecord};
    extRecordMgr->preloadUIExtensionMap_[mapKey] = records;

    int32_t ret = extRecordMgr->ClearAllPreloadUIExtensionRecordForHost();
    EXPECT_EQ(ret, ERR_OK);

    EXPECT_EQ(extRecordMgr->preloadUIExtensionMap_.count(mapKey), 0);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: SetAbilityProcessName_0100
 * @tc.desc: Test SetAbilityProcessName with isolationProcess true
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerSecondTest, SetAbilityProcessName_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.test";
    abilityRequest.abilityInfo.name = "TestAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    abilityRequest.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    abilityRequest.abilityInfo.isolationProcess = true;

    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    ASSERT_NE(extRecord, nullptr);

    int32_t ret = extRecordMgr->SetAbilityProcessName(abilityRequest, abilityRecord, extRecord);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: SetAbilityProcessName_0200
 * @tc.desc: Test SetAbilityProcessName with isolationProcess false (call UpdateProcessName)
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerSecondTest, SetAbilityProcessName_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.test";
    abilityRequest.abilityInfo.name = "TestAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    abilityRequest.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SHARE;
    abilityRequest.abilityInfo.isolationProcess = false;

    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    ASSERT_NE(extRecord, nullptr);

    int32_t ret = extRecordMgr->SetAbilityProcessName(abilityRequest, abilityRecord, extRecord);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: StartAbility_0100
 * @tc.desc: Test StartAbility return value
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerSecondTest, StartAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    AAFwk::AbilityRequest abilityRequest;
    int32_t ret = extRecordMgr->StartAbility(abilityRequest);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: LoadTimeout_0200
 * @tc.desc: Test LoadTimeout with invalid record id
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerSecondTest, LoadTimeout_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    extRecordMgr->LoadTimeout(9999);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: BackgroundTimeout_0100
 * @tc.desc: Test BackgroundTimeout with valid UIExtensionRecord
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerSecondTest, BackgroundTimeout_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    extRecordMgr->BackgroundTimeout(9999);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: TerminateTimeout_0100
 * @tc.desc: Test TerminateTimeout with valid UIExtensionRecord
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerSecondTest, TerminateTimeout_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    extRecordMgr->TerminateTimeout(9999);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

// ========== ExtensionRunningTimeoutMonitor TDD Tests ==========

class ExtensionRunningTimeoutMonitorTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void ExtensionRunningTimeoutMonitorTest::SetUpTestCase()
{}

void ExtensionRunningTimeoutMonitorTest::TearDownTestCase()
{}

void ExtensionRunningTimeoutMonitorTest::SetUp()
{
    auto monitor = DelayedSingleton<AAFwk::ExtensionRunningTimeoutMonitor>::GetInstance();
    monitor->runningExtensions_.clear();
    monitor->cachedEvents_.clear();
}

void ExtensionRunningTimeoutMonitorTest::TearDown()
{
    auto monitor = DelayedSingleton<AAFwk::ExtensionRunningTimeoutMonitor>::GetInstance();
    monitor->StopMonitor();
    monitor->runningExtensions_.clear();
    monitor->cachedEvents_.clear();
}

/**
 * @tc.name: OnExtensionStarted_0100
 * @tc.desc: Test OnExtensionStarted records extension info correctly.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRunningTimeoutMonitorTest, OnExtensionStarted_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto monitor = DelayedSingleton<AAFwk::ExtensionRunningTimeoutMonitor>::GetInstance();
    ASSERT_NE(monitor, nullptr);

    monitor->OnExtensionStarted(100, "ServiceExtension", "com.test.bundle", "TestAbility");
    EXPECT_EQ(monitor->runningExtensions_.size(), static_cast<size_t>(1));
    EXPECT_EQ(monitor->runningExtensions_.count(100), static_cast<size_t>(1));
    EXPECT_EQ(monitor->runningExtensions_[100].extensionTypeName, "ServiceExtension");
    EXPECT_EQ(monitor->runningExtensions_[100].bundleName, "com.test.bundle");
    EXPECT_EQ(monitor->runningExtensions_[100].abilityName, "TestAbility");
    EXPECT_GT(monitor->runningExtensions_[100].startTimeMillis, static_cast<int64_t>(0));

    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: OnExtensionStarted_0200
 * @tc.desc: Test OnExtensionStarted with multiple extensions.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRunningTimeoutMonitorTest, OnExtensionStarted_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto monitor = DelayedSingleton<AAFwk::ExtensionRunningTimeoutMonitor>::GetInstance();
    ASSERT_NE(monitor, nullptr);

    monitor->OnExtensionStarted(1, "ServiceExtension", "com.test.a", "AbilityA");
    monitor->OnExtensionStarted(2, "UIExtension", "com.test.b", "AbilityB");
    monitor->OnExtensionStarted(3, "DataShareExtension", "com.test.c", "AbilityC");

    EXPECT_EQ(monitor->runningExtensions_.size(), static_cast<size_t>(3));
    EXPECT_EQ(monitor->runningExtensions_[1].extensionTypeName, "ServiceExtension");
    EXPECT_EQ(monitor->runningExtensions_[2].extensionTypeName, "UIExtension");
    EXPECT_EQ(monitor->runningExtensions_[3].extensionTypeName, "DataShareExtension");

    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: OnExtensionStarted_0300
 * @tc.desc: Test OnExtensionStarted with same recordId overwrites previous entry.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRunningTimeoutMonitorTest, OnExtensionStarted_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto monitor = DelayedSingleton<AAFwk::ExtensionRunningTimeoutMonitor>::GetInstance();
    ASSERT_NE(monitor, nullptr);

    monitor->OnExtensionStarted(100, "ServiceExtension", "com.test.old", "OldAbility");
    EXPECT_EQ(monitor->runningExtensions_.size(), static_cast<size_t>(1));

    monitor->OnExtensionStarted(100, "UIExtension", "com.test.new", "NewAbility");
    EXPECT_EQ(monitor->runningExtensions_.size(), static_cast<size_t>(1));
    EXPECT_EQ(monitor->runningExtensions_[100].extensionTypeName, "UIExtension");
    EXPECT_EQ(monitor->runningExtensions_[100].bundleName, "com.test.new");

    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: OnExtensionTerminated_0100
 * @tc.desc: Test OnExtensionTerminated with unknown recordId does nothing.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRunningTimeoutMonitorTest, OnExtensionTerminated_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto monitor = DelayedSingleton<AAFwk::ExtensionRunningTimeoutMonitor>::GetInstance();
    ASSERT_NE(monitor, nullptr);

    monitor->OnExtensionTerminated(9999);
    EXPECT_EQ(monitor->runningExtensions_.size(), static_cast<size_t>(0));
    EXPECT_EQ(monitor->cachedEvents_.size(), static_cast<size_t>(0));

    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: OnExtensionTerminated_0200
 * @tc.desc: Test OnExtensionTerminated removes extension from running list.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRunningTimeoutMonitorTest, OnExtensionTerminated_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto monitor = DelayedSingleton<AAFwk::ExtensionRunningTimeoutMonitor>::GetInstance();
    ASSERT_NE(monitor, nullptr);

    monitor->OnExtensionStarted(100, "ServiceExtension", "com.test.bundle", "TestAbility");
    EXPECT_EQ(monitor->runningExtensions_.size(), static_cast<size_t>(1));

    // No timeout configured (default -1), so no event should be cached
    monitor->OnExtensionTerminated(100);
    EXPECT_EQ(monitor->runningExtensions_.size(), static_cast<size_t>(0));

    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: OnExtensionTerminated_0300
 * @tc.desc: Test OnExtensionTerminated detects timeout and caches event.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRunningTimeoutMonitorTest, OnExtensionTerminated_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto monitor = DelayedSingleton<AAFwk::ExtensionRunningTimeoutMonitor>::GetInstance();
    ASSERT_NE(monitor, nullptr);

    // Configure timeout for this extension type via ExtensionConfig
    auto config = DelayedSingleton<AAFwk::ExtensionConfig>::GetInstance();
    ASSERT_NE(config, nullptr);
    AAFwk::ExtensionConfigItem item;
    item.extensionRunningTimeoutTime = 1; // 1 second
    config->configMap_["TestTimeoutExtension"] = item;

    monitor->OnExtensionStarted(200, "TestTimeoutExtension", "com.test.timeout", "TimeoutAbility");
    EXPECT_EQ(monitor->runningExtensions_.size(), static_cast<size_t>(1));

    // Wait for timeout to be exceeded
    std::this_thread::sleep_for(std::chrono::seconds(2));

    monitor->OnExtensionTerminated(200);
    EXPECT_EQ(monitor->runningExtensions_.size(), static_cast<size_t>(0));
    EXPECT_EQ(monitor->cachedEvents_.size(), static_cast<size_t>(1));

    auto &event = monitor->cachedEvents_.front();
    EXPECT_EQ(event.extensionTypeName, "TestTimeoutExtension");
    EXPECT_EQ(event.bundleName, "com.test.timeout");
    EXPECT_EQ(event.abilityName, "TimeoutAbility");
    EXPECT_GE(event.runningDuration, static_cast<int64_t>(1));
    EXPECT_EQ(event.stillAlive, false);
    EXPECT_EQ(event.cnt, 1);

    // Clean up config
    config->configMap_.erase("TestTimeoutExtension");

    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: OnExtensionTerminated_0400
 * @tc.desc: Test OnExtensionTerminated within timeout does not cache event.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRunningTimeoutMonitorTest, OnExtensionTerminated_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto monitor = DelayedSingleton<AAFwk::ExtensionRunningTimeoutMonitor>::GetInstance();
    ASSERT_NE(monitor, nullptr);

    // Configure long timeout
    auto config = DelayedSingleton<AAFwk::ExtensionConfig>::GetInstance();
    AAFwk::ExtensionConfigItem item;
    item.extensionRunningTimeoutTime = 3600; // 1 hour
    config->configMap_["TestLongTimeout"] = item;

    monitor->OnExtensionStarted(300, "TestLongTimeout", "com.test.quick", "QuickAbility");

    // Terminate immediately, well within 1 hour timeout
    monitor->OnExtensionTerminated(300);
    EXPECT_EQ(monitor->runningExtensions_.size(), static_cast<size_t>(0));
    EXPECT_EQ(monitor->cachedEvents_.size(), static_cast<size_t>(0));

    config->configMap_.erase("TestLongTimeout");

    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: AddOrUpdateTimeoutEvent_0100
 * @tc.desc: Test AddOrUpdateTimeoutEvent adds new event when no duplicate.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRunningTimeoutMonitorTest, AddOrUpdateTimeoutEvent_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto monitor = DelayedSingleton<AAFwk::ExtensionRunningTimeoutMonitor>::GetInstance();
    ASSERT_NE(monitor, nullptr);

    AAFwk::ExtensionTimeoutEvent event;
    event.extensionTypeName = "ServiceExtension";
    event.bundleName = "com.test.bundle";
    event.abilityName = "Ability1";
    event.runningDuration = 100;
    event.stillAlive = false;
    event.cnt = 1;

    monitor->AddOrUpdateTimeoutEvent(event);
    EXPECT_EQ(monitor->cachedEvents_.size(), static_cast<size_t>(1));

    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: AddOrUpdateTimeoutEvent_0200
 * @tc.desc: Test AddOrUpdateTimeoutEvent increments cnt for duplicate event.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRunningTimeoutMonitorTest, AddOrUpdateTimeoutEvent_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto monitor = DelayedSingleton<AAFwk::ExtensionRunningTimeoutMonitor>::GetInstance();
    ASSERT_NE(monitor, nullptr);

    AAFwk::ExtensionTimeoutEvent event1;
    event1.extensionTypeName = "ServiceExtension";
    event1.bundleName = "com.test.dup";
    event1.abilityName = "DupAbility";
    event1.runningDuration = 10;
    event1.stillAlive = true;
    event1.cnt = 1;

    monitor->AddOrUpdateTimeoutEvent(event1);
    EXPECT_EQ(monitor->cachedEvents_.size(), static_cast<size_t>(1));
    EXPECT_EQ(monitor->cachedEvents_.front().cnt, 1);

    // Same extension detected again
    AAFwk::ExtensionTimeoutEvent event2;
    event2.extensionTypeName = "ServiceExtension";
    event2.bundleName = "com.test.dup";
    event2.abilityName = "DupAbility";
    event2.runningDuration = 20;
    event2.stillAlive = false;
    event2.cnt = 1;

    monitor->AddOrUpdateTimeoutEvent(event2);
    EXPECT_EQ(monitor->cachedEvents_.size(), static_cast<size_t>(1));
    EXPECT_EQ(monitor->cachedEvents_.front().cnt, 2);
    EXPECT_EQ(monitor->cachedEvents_.front().runningDuration, static_cast<int64_t>(20));
    EXPECT_EQ(monitor->cachedEvents_.front().stillAlive, false);

    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: AddOrUpdateTimeoutEvent_0300
 * @tc.desc: Test AddOrUpdateTimeoutEvent discards event when cache is full.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRunningTimeoutMonitorTest, AddOrUpdateTimeoutEvent_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto monitor = DelayedSingleton<AAFwk::ExtensionRunningTimeoutMonitor>::GetInstance();
    ASSERT_NE(monitor, nullptr);

    // Fill cache to MAX_CACHED_EVENTS (5)
    for (int32_t i = 0; i < monitor->MAX_CACHED_EVENTS; i++) {
        AAFwk::ExtensionTimeoutEvent event;
        event.extensionTypeName = "ServiceExtension";
        event.bundleName = "com.test.bundle" + std::to_string(i);
        event.abilityName = "Ability" + std::to_string(i);
        event.runningDuration = i + 1;
        event.cnt = 1;
        monitor->AddOrUpdateTimeoutEvent(event);
    }
    EXPECT_EQ(monitor->cachedEvents_.size(), static_cast<size_t>(monitor->MAX_CACHED_EVENTS));

    // 6th different event should be discarded
    AAFwk::ExtensionTimeoutEvent overflowEvent;
    overflowEvent.extensionTypeName = "ServiceExtension";
    overflowEvent.bundleName = "com.test.overflow";
    overflowEvent.abilityName = "OverflowAbility";
    overflowEvent.runningDuration = 999;
    overflowEvent.cnt = 1;
    monitor->AddOrUpdateTimeoutEvent(overflowEvent);

    EXPECT_EQ(monitor->cachedEvents_.size(), static_cast<size_t>(monitor->MAX_CACHED_EVENTS));

    // Verify overflow event is not in cache
    bool found = false;
    for (const auto &e : monitor->cachedEvents_) {
        if (e.bundleName == "com.test.overflow") {
            found = true;
            break;
        }
    }
    EXPECT_FALSE(found);

    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: IsDuplicateEvent_0100
 * @tc.desc: Test IsDuplicateEvent returns true for matching events.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRunningTimeoutMonitorTest, IsDuplicateEvent_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto monitor = DelayedSingleton<AAFwk::ExtensionRunningTimeoutMonitor>::GetInstance();
    ASSERT_NE(monitor, nullptr);

    AAFwk::ExtensionTimeoutEvent event;
    event.extensionTypeName = "ServiceExtension";
    event.bundleName = "com.test.dup";
    event.abilityName = "TestAbility";
    monitor->cachedEvents_.push_back(event);

    AAFwk::ExtensionTimeoutEvent candidate;
    candidate.extensionTypeName = "ServiceExtension";
    candidate.bundleName = "com.test.dup";
    candidate.abilityName = "TestAbility";

    std::list<AAFwk::ExtensionTimeoutEvent>::iterator dupIter;
    EXPECT_TRUE(monitor->IsDuplicateEvent(candidate, dupIter));
    EXPECT_EQ(dupIter->bundleName, "com.test.dup");

    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: IsDuplicateEvent_0200
 * @tc.desc: Test IsDuplicateEvent returns false for non-matching events.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRunningTimeoutMonitorTest, IsDuplicateEvent_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto monitor = DelayedSingleton<AAFwk::ExtensionRunningTimeoutMonitor>::GetInstance();
    ASSERT_NE(monitor, nullptr);

    AAFwk::ExtensionTimeoutEvent event;
    event.extensionTypeName = "ServiceExtension";
    event.bundleName = "com.test.exist";
    event.abilityName = "ExistAbility";
    monitor->cachedEvents_.push_back(event);

    AAFwk::ExtensionTimeoutEvent candidate;
    candidate.extensionTypeName = "UIExtension";
    candidate.bundleName = "com.test.exist";
    candidate.abilityName = "ExistAbility";

    std::list<AAFwk::ExtensionTimeoutEvent>::iterator dupIter;
    EXPECT_FALSE(monitor->IsDuplicateEvent(candidate, dupIter));

    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: ReportTimeoutEvents_0100
 * @tc.desc: Test ReportTimeoutEvents clears cache after reporting.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRunningTimeoutMonitorTest, ReportTimeoutEvents_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto monitor = DelayedSingleton<AAFwk::ExtensionRunningTimeoutMonitor>::GetInstance();
    ASSERT_NE(monitor, nullptr);

    AAFwk::ExtensionTimeoutEvent event;
    event.extensionTypeName = "ServiceExtension";
    event.bundleName = "com.test.report";
    event.abilityName = "ReportAbility";
    event.runningDuration = 100;
    event.stillAlive = false;
    event.cnt = 1;
    monitor->cachedEvents_.push_back(event);
    EXPECT_EQ(monitor->cachedEvents_.size(), static_cast<size_t>(1));

    monitor->ReportTimeoutEvents();
    EXPECT_EQ(monitor->cachedEvents_.size(), static_cast<size_t>(0));

    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: ReportTimeoutEvents_0200
 * @tc.desc: Test ReportTimeoutEvents with empty cache does nothing.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRunningTimeoutMonitorTest, ReportTimeoutEvents_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto monitor = DelayedSingleton<AAFwk::ExtensionRunningTimeoutMonitor>::GetInstance();
    ASSERT_NE(monitor, nullptr);

    EXPECT_EQ(monitor->cachedEvents_.size(), static_cast<size_t>(0));
    monitor->ReportTimeoutEvents();
    EXPECT_EQ(monitor->cachedEvents_.size(), static_cast<size_t>(0));

    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: CheckAliveExtensions_0100
 * @tc.desc: Test CheckAliveExtensions detects running extension exceeding timeout.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRunningTimeoutMonitorTest, CheckAliveExtensions_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto monitor = DelayedSingleton<AAFwk::ExtensionRunningTimeoutMonitor>::GetInstance();
    ASSERT_NE(monitor, nullptr);

    // Configure timeout
    auto config = DelayedSingleton<AAFwk::ExtensionConfig>::GetInstance();
    AAFwk::ExtensionConfigItem item;
    item.extensionRunningTimeoutTime = 1; // 1 second
    config->configMap_["AliveTestExtension"] = item;

    monitor->OnExtensionStarted(400, "AliveTestExtension", "com.test.alive", "AliveAbility");

    // Wait for timeout
    std::this_thread::sleep_for(std::chrono::seconds(2));

    monitor->CheckAliveExtensions();

    EXPECT_EQ(monitor->cachedEvents_.size(), static_cast<size_t>(1));
    auto &event = monitor->cachedEvents_.front();
    EXPECT_EQ(event.bundleName, "com.test.alive");
    EXPECT_EQ(event.abilityName, "AliveAbility");
    EXPECT_EQ(event.stillAlive, true); // detected via alive check
    EXPECT_GE(event.runningDuration, static_cast<int64_t>(1));

    // Extension should still be in running list
    EXPECT_EQ(monitor->runningExtensions_.size(), static_cast<size_t>(1));

    config->configMap_.erase("AliveTestExtension");

    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: CheckAliveExtensions_0200
 * @tc.desc: Test CheckAliveExtensions skips extensions with no timeout config.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRunningTimeoutMonitorTest, CheckAliveExtensions_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto monitor = DelayedSingleton<AAFwk::ExtensionRunningTimeoutMonitor>::GetInstance();
    ASSERT_NE(monitor, nullptr);

    // No config for "UnconfiguredExtension", default timeout is -1
    monitor->OnExtensionStarted(500, "UnconfiguredExtension", "com.test.noconfig", "NoConfigAbility");

    std::this_thread::sleep_for(std::chrono::seconds(2));

    monitor->CheckAliveExtensions();

    // Should not create any events since timeout is -1 (disabled)
    EXPECT_EQ(monitor->cachedEvents_.size(), static_cast<size_t>(0));

    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: StartStopMonitor_0100
 * @tc.desc: Test StartMonitor and StopMonitor do not crash.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRunningTimeoutMonitorTest, StartStopMonitor_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto monitor = DelayedSingleton<AAFwk::ExtensionRunningTimeoutMonitor>::GetInstance();
    ASSERT_NE(monitor, nullptr);

    monitor->StartMonitor();
    monitor->StopMonitor();

    // Double stop should not crash
    monitor->StopMonitor();

    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: ConcurrentAccess_0100
 * @tc.desc: Test concurrent OnExtensionStarted and OnExtensionTerminated do not crash.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRunningTimeoutMonitorTest, ConcurrentAccess_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto monitor = DelayedSingleton<AAFwk::ExtensionRunningTimeoutMonitor>::GetInstance();
    ASSERT_NE(monitor, nullptr);

    const int32_t threadCount = 10;
    std::vector<std::thread> threads;

    for (int32_t i = 0; i < threadCount; i++) {
        threads.emplace_back([monitor, i]() {
            monitor->OnExtensionStarted(i, "ServiceExtension",
                "com.test.concurrent", "Ability" + std::to_string(i));
        });
    }
    for (auto &t : threads) {
        t.join();
    }
    EXPECT_EQ(monitor->runningExtensions_.size(), static_cast<size_t>(threadCount));

    threads.clear();
    for (int32_t i = 0; i < threadCount; i++) {
        threads.emplace_back([monitor, i]() {
            monitor->OnExtensionTerminated(i);
        });
    }
    for (auto &t : threads) {
        t.join();
    }
    EXPECT_EQ(monitor->runningExtensions_.size(), static_cast<size_t>(0));

    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: ExtensionConfigGetRunningTimeoutTime_0100
 * @tc.desc: Test ExtensionConfig GetExtensionRunningTimeoutTime returns default when not configured.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRunningTimeoutMonitorTest, ExtensionConfigGetRunningTimeoutTime_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto config = DelayedSingleton<AAFwk::ExtensionConfig>::GetInstance();
    ASSERT_NE(config, nullptr);

    int32_t timeout = config->GetExtensionRunningTimeoutTime("NonExistentType");
    EXPECT_EQ(timeout, AAFwk::DEFAULT_EXTENSION_RUNNING_TIMEOUT_TIME);

    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: ExtensionConfigGetRunningTimeoutTime_0200
 * @tc.desc: Test ExtensionConfig GetExtensionRunningTimeoutTime returns configured value.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRunningTimeoutMonitorTest, ExtensionConfigGetRunningTimeoutTime_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto config = DelayedSingleton<AAFwk::ExtensionConfig>::GetInstance();
    ASSERT_NE(config, nullptr);

    AAFwk::ExtensionConfigItem item;
    item.extensionRunningTimeoutTime = 5000;
    config->configMap_["ConfiguredType"] = item;

    int32_t timeout = config->GetExtensionRunningTimeoutTime("ConfiguredType");
    EXPECT_EQ(timeout, 5000);

    config->configMap_.erase("ConfiguredType");

    TAG_LOGI(AAFwkTag::TEST, "end.");
}

} // namespace AbilityRuntime
} // namespace OHOS
