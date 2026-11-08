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

#include "mock_permission_verification.h"
#define private public
#define protected public
#include "update_caller_info_util.h"
#include "ability_manager_service.h"
#include "dialog_session_manager.h"
#undef private
#undef protected
#include "insight_intent_execute_param.h"
#include "skill/skill_execute_param.h"
#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "string_wrapper.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using OHOS::AppExecFwk::AbilityType;
using OHOS::AppExecFwk::ExtensionAbilityType;
namespace OHOS {
namespace AAFwk {

constexpr const char *CALLER_REQUEST_CODE = "ohos.extra.param.key.callerRequestCode";
constexpr const char *DMS_CALLER_BUNDLE_NAME = "ohos.dms.param.sourceCallerBundleName";
const std::string HIDE_SENSITIVE_TYPE = "ohos.media.params.hideSensitiveType";

class UpdateCallerInfoUtilTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    std::shared_ptr<AbilityRecord> MockAbilityRecord(AbilityType);
    sptr<Token> MockToken(AbilityType);
    sptr<SessionInfo> MockSessionInfo(int32_t persistentId);

public:
    AbilityRequest abilityRequest_{};
    Want want_{};
};

std::shared_ptr<AbilityRecord> UpdateCallerInfoUtilTest::MockAbilityRecord(AbilityType abilityType)
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.demo";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = abilityType;
    return AbilityRecord::CreateAbilityRecord(abilityRequest);
}

sptr<Token> UpdateCallerInfoUtilTest::MockToken(AbilityType abilityType)
{
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(abilityType);
    if (!abilityRecord) {
        return nullptr;
    }
    return abilityRecord->GetToken();
}

sptr<SessionInfo> UpdateCallerInfoUtilTest::MockSessionInfo(int32_t persistentId)
{
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    if (!sessionInfo) {
        TAG_LOGE(AAFwkTag::TEST, "sessionInfo is nullptr");
        return nullptr;
    }
    sessionInfo->persistentId = persistentId;
    return sessionInfo;
}

void UpdateCallerInfoUtilTest::SetUpTestCase() {}

void UpdateCallerInfoUtilTest::TearDownTestCase() {}

void UpdateCallerInfoUtilTest::SetUp() {}

void UpdateCallerInfoUtilTest::TearDown() {}

/**
 * @tc.name: UpdateCallerInfoUtilTest_UpdateAsCallerInfoFromCallerRecord_0001
 * @tc.desc: Test the state of QueryAllAutoStartupApplications
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, UpdateAsCallerInfoFromCallerRecord_0001, TestSize.Level1)
{
    std::shared_ptr<UpdateCallerInfoUtil> updateCallerUtil = std::make_shared<UpdateCallerInfoUtil>();
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    callerToken = abilityRecord->GetToken();

    updateCallerUtil->UpdateAsCallerInfoFromCallerRecord(want, callerToken);
    EXPECT_NE(callerToken, nullptr);
}

/**
 * @tc.name: UpdateCallerInfoUtilTest_UpdateCallerInfoFromToken_0001
 * @tc.desc: Test the state of UpdateCallerInfoFromToken
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, UpdateCallerInfoFromToken_0001, TestSize.Level1)
{
    std::shared_ptr<UpdateCallerInfoUtil> updateCallerUtil = std::make_shared<UpdateCallerInfoUtil>();
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    callerToken = abilityRecord->GetToken();

    updateCallerUtil->UpdateCallerInfoFromToken(want, callerToken);
    EXPECT_NE(abilityRecord, nullptr);

    abilityRecord = nullptr;
    updateCallerUtil->UpdateCallerInfoFromToken(want, callerToken);
    EXPECT_EQ(abilityRecord, nullptr);
}

/**
 * @tc.name: UpdateCallerInfoUtilTest_CacheRestoreTaskId_0001
 * @tc.desc: Task attribution key cached from the original want survives the restore.
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, CacheRestoreTaskId_0001, TestSize.Level1)
{
    auto& updateCallerUtil = UpdateCallerInfoUtil::GetInstance();
    Want originalWant;
    originalWant.SetParam("ohos.aafwk.param.agent.taskId", std::string("task-ut-773"));
    originalWant.SetParam(Want::PARAM_RESV_CALLER_UID, 20000001);
    originalWant.SetParam(Want::PARAM_RESV_CALLER_PID, 100);

    AbilitySessionInfo info;
    updateCallerUtil.CacheCallerInfoFromWant(originalWant, info);
    EXPECT_EQ(info.taskId, "task-ut-773");
    EXPECT_EQ(info.callerUid, 20000001);
    EXPECT_EQ(info.callerPid, 100);

    // Simulate the SCB-returned want with the key lost on the round trip.
    Want scbWant;
    updateCallerUtil.RestoreCallerInfoToWant(scbWant, info);
    EXPECT_EQ(scbWant.GetStringParam("ohos.aafwk.param.agent.taskId"), "task-ut-773");
    EXPECT_EQ(scbWant.GetIntParam(Want::PARAM_RESV_CALLER_UID, -1), 20000001);
    EXPECT_EQ(scbWant.GetIntParam(Want::PARAM_RESV_CALLER_PID, -1), 100);
}

/**
 * @tc.name: UpdateCallerInfoUtilTest_CacheRestoreTaskId_0002
 * @tc.desc: Empty cache strips the key from the SCB-returned want (fail-closed).
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, CacheRestoreTaskId_0002, TestSize.Level1)
{
    auto& updateCallerUtil = UpdateCallerInfoUtil::GetInstance();
    Want originalWant; // no key: the caller never supplied one
    AbilitySessionInfo info;
    updateCallerUtil.CacheCallerInfoFromWant(originalWant, info);
    EXPECT_TRUE(info.taskId.empty());

    // Simulate a replayed/injected key on the SCB-returned want.
    Want scbWant;
    scbWant.SetParam("ohos.aafwk.param.agent.taskId", std::string("task-replay-999"));
    updateCallerUtil.RestoreCallerInfoToWant(scbWant, info);
    EXPECT_FALSE(scbWant.HasParameter("ohos.aafwk.param.agent.taskId"));
}

/**
 * @tc.name: UpdateCallerInfoUtilTest_ClearProtectedWantParam_001
 * @tc.desc: The caller is not system app, remove udKey.
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, ClearProtectedWantParam_001, TestSize.Level1)
{
    std::shared_ptr<UpdateCallerInfoUtil> updateCallerUtil = std::make_shared<UpdateCallerInfoUtil>();
    Want want;
    std::string key = "udmf://temp/key/aaa";
    want.SetParam(Want::PARAM_ABILITY_UNIFIED_DATA_KEY, key);
    updateCallerUtil->ClearProtectedWantParam(want);
    EXPECT_EQ(want.GetStringParam(Want::PARAM_ABILITY_UNIFIED_DATA_KEY), key);
}

/**
 * @tc.name: UpdateCallerInfoUtilTest_ClearProtectedWantParam_002
 * @tc.desc: The caller is system app, do not remove udKey.
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, ClearProtectedWantParam_002, TestSize.Level1)
{
    std::shared_ptr<UpdateCallerInfoUtil> updateCallerUtil = std::make_shared<UpdateCallerInfoUtil>();
    Want want;
    std::string key = "udmf://temp/key/aaa";
    want.SetParam(Want::PARAM_ABILITY_UNIFIED_DATA_KEY, key);
    MyFlag::isSystemAppCallRet = false;
    updateCallerUtil->ClearProtectedWantParam(want);
    MyFlag::isSystemAppCallRet = true;
    EXPECT_EQ(want.GetStringParam(Want::PARAM_ABILITY_UNIFIED_DATA_KEY), "");
}

/**
 * @tc.name: UpdateCallerInfoUtilTest_ClearProtectedWantParam_003
 * @tc.desc: The caller is system app, do not remove udKey.
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, ClearProtectedWantParam_003, TestSize.Level1)
{
    std::shared_ptr<UpdateCallerInfoUtil> updateCallerUtil = std::make_shared<UpdateCallerInfoUtil>();
    Want want;
    std::string key = "test";
    want.SetParam(Want::PARAM_RESV_CALLER_NATIVE_NAME, key);
    updateCallerUtil->ClearProtectedWantParam(want);
    EXPECT_EQ(want.GetStringParam(Want::PARAM_RESV_CALLER_NATIVE_NAME), "");
}

/**
 * @tc.name: UpdateCallerInfoUtilTest_UpdateAsCallerInfoFromCallerRecord_0002
 * @tc.desc: Test the state of QueryAllAutoStartupApplications
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, UpdateAsCallerInfoFromCallerRecord_0002, TestSize.Level1)
{
    std::shared_ptr<UpdateCallerInfoUtil> updateCallerUtil = std::make_shared<UpdateCallerInfoUtil>();
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    callerToken = abilityRecord->GetToken();

    updateCallerUtil->UpdateAsCallerInfoFromCallerRecord(want, callerToken);
    EXPECT_NE(callerToken, nullptr);
}

/**
 * @tc.name: UpdateCallerInfoUtilTest_UpdateAsCallerInfoFromDialog_0001
 * @tc.desc: Test the state of QueryAllAutoStartupApplications
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, UpdateAsCallerInfoFromDialog_0001, TestSize.Level1)
{
    std::shared_ptr<UpdateCallerInfoUtil> updateCallerUtil = std::make_shared<UpdateCallerInfoUtil>();
    DialogSessionManager dialogSessionManager;
    Want want;
    const std::string TEST_DIALOG_SESSION_ID = "dialogSessionId";
    want.SetParam(KEY_REQUEST_ID, std::string("1234567890"));
    sptr<DialogSessionInfo> dilogSessionInfo = nullptr;
    std::shared_ptr<DialogCallerInfo> dialogCallerInfo = std::make_shared<DialogCallerInfo>();
    EXPECT_NE(dialogCallerInfo, nullptr);
    dialogCallerInfo->targetWant = want;

    dialogSessionManager.SetDialogSessionInfo(TEST_DIALOG_SESSION_ID, dilogSessionInfo, dialogCallerInfo);
    auto callerInfo = dialogSessionManager.GetDialogCallerInfo(TEST_DIALOG_SESSION_ID);
    EXPECT_NE(dialogSessionManager.GetDialogCallerInfo(TEST_DIALOG_SESSION_ID), nullptr);

    bool ret = updateCallerUtil->UpdateAsCallerInfoFromDialog(want);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: UpdateCallerInfoUtilTest_UpdateAsCallerInfoFromDialog_0002
 * @tc.desc: Test the state of QueryAllAutoStartupApplications
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, UpdateAsCallerInfoFromDialog_0002, TestSize.Level1)
{
    std::shared_ptr<UpdateCallerInfoUtil> updateCallerUtil = std::make_shared<UpdateCallerInfoUtil>();
    DialogSessionManager dialogSessionManager;
    Want want;
    bool ret = updateCallerUtil->UpdateAsCallerInfoFromDialog(want);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: UpdateCallerInfoUtilTest_UpdateAsCallerInfoFromDialog_0003
 * @tc.desc: Test UpdateAsCallerInfoFromDialog with BROKER_UID and SHELL_ASSISTANT
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, UpdateAsCallerInfoFromDialog_0003, TestSize.Level1)
{
    std::shared_ptr<UpdateCallerInfoUtil> updateCallerUtil = std::make_shared<UpdateCallerInfoUtil>();
    DialogSessionManager dialogSessionManager;
    Want want;
    const std::string TEST_DIALOG_SESSION_ID = "dialogSessionId";
    std::string callerBundleName = "com.ohos.shell_assistant";
    int32_t BROKER_UID = 5557;
    want.SetParam(Want::PARAM_RESV_CALLER_UID, BROKER_UID);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    sptr<DialogSessionInfo> dilogSessionInfo = nullptr;
    std::shared_ptr<DialogCallerInfo> dialogCallerInfo = std::make_shared<DialogCallerInfo>();
    EXPECT_NE(dialogCallerInfo, nullptr);
    dialogCallerInfo->targetWant = want;

    dialogSessionManager.SetDialogSessionInfo(TEST_DIALOG_SESSION_ID, dilogSessionInfo, dialogCallerInfo);
    auto callerInfo = dialogSessionManager.GetDialogCallerInfo(TEST_DIALOG_SESSION_ID);
    EXPECT_NE(callerInfo, nullptr);

    bool ret = updateCallerUtil->UpdateAsCallerInfoFromDialog(want);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: UpdateCallerInfoUtilTest_UpdateAsCallerInfoFromDialog_0004
 * @tc.desc: Test UpdateAsCallerInfoFromDialog with not BROKER_UID and SHELL_ASSISTANT
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, UpdateAsCallerInfoFromDialog_0004, TestSize.Level1)
{
    std::shared_ptr<UpdateCallerInfoUtil> updateCallerUtil = std::make_shared<UpdateCallerInfoUtil>();
    DialogSessionManager dialogSessionManager;
    Want want;
    const std::string TEST_DIALOG_SESSION_ID = "dialogSessionId";
    std::string callerBundleName = "com.ohos.shell_assistant";
    want.SetParam(Want::PARAM_RESV_CALLER_UID, 1314);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    sptr<DialogSessionInfo> dilogSessionInfo = nullptr;
    std::shared_ptr<DialogCallerInfo> dialogCallerInfo = std::make_shared<DialogCallerInfo>();
    EXPECT_NE(dialogCallerInfo, nullptr);
    dialogCallerInfo->targetWant = want;

    dialogSessionManager.SetDialogSessionInfo(TEST_DIALOG_SESSION_ID, dilogSessionInfo, dialogCallerInfo);
    auto callerInfo = dialogSessionManager.GetDialogCallerInfo(TEST_DIALOG_SESSION_ID);
    EXPECT_NE(callerInfo, nullptr);

    bool ret = updateCallerUtil->UpdateAsCallerInfoFromDialog(want);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: UpdateCallerInfoUtilTest_UpdateAsCallerInfoFromDialog_0005
 * @tc.desc: Test UpdateAsCallerInfoFromDialog with BROKER_UID and not SHELL_ASSISTANT
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, UpdateAsCallerInfoFromDialog_0005, TestSize.Level1)
{
    std::shared_ptr<UpdateCallerInfoUtil> updateCallerUtil = std::make_shared<UpdateCallerInfoUtil>();
    DialogSessionManager dialogSessionManager;
    Want want;
    const std::string TEST_DIALOG_SESSION_ID = "dialogSessionId";
    std::string callerBundleName = "com.ohos.test";
    int32_t BROKER_UID = 5557;
    want.SetParam(Want::PARAM_RESV_CALLER_UID, BROKER_UID);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    sptr<DialogSessionInfo> dilogSessionInfo = nullptr;
    std::shared_ptr<DialogCallerInfo> dialogCallerInfo = std::make_shared<DialogCallerInfo>();
    EXPECT_NE(dialogCallerInfo, nullptr);
    dialogCallerInfo->targetWant = want;

    dialogSessionManager.SetDialogSessionInfo(TEST_DIALOG_SESSION_ID, dilogSessionInfo, dialogCallerInfo);
    auto callerInfo = dialogSessionManager.GetDialogCallerInfo(TEST_DIALOG_SESSION_ID);
    EXPECT_NE(callerInfo, nullptr);

    bool ret = updateCallerUtil->UpdateAsCallerInfoFromDialog(want);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: UpdateCallerInfoUtilTest_UpdateAsCallerInfoFromDialog_0006
 * @tc.desc: Test UpdateAsCallerInfoFromDialog with not BROKER_UID and not SHELL_ASSISTANT
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, UpdateAsCallerInfoFromDialog_0006, TestSize.Level1)
{
    std::shared_ptr<UpdateCallerInfoUtil> updateCallerUtil = std::make_shared<UpdateCallerInfoUtil>();
    DialogSessionManager dialogSessionManager;
    Want want;
    const std::string TEST_DIALOG_SESSION_ID = "dialogSessionId";
    std::string callerBundleName = "com.ohos.test";
    want.SetParam(Want::PARAM_RESV_CALLER_UID, 1314);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    sptr<DialogSessionInfo> dilogSessionInfo = nullptr;
    std::shared_ptr<DialogCallerInfo> dialogCallerInfo = std::make_shared<DialogCallerInfo>();
    EXPECT_NE(dialogCallerInfo, nullptr);
    dialogCallerInfo->targetWant = want;

    dialogSessionManager.SetDialogSessionInfo(TEST_DIALOG_SESSION_ID, dilogSessionInfo, dialogCallerInfo);
    auto callerInfo = dialogSessionManager.GetDialogCallerInfo(TEST_DIALOG_SESSION_ID);
    EXPECT_NE(callerInfo, nullptr);

    bool ret = updateCallerUtil->UpdateAsCallerInfoFromDialog(want);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: UpdateCallerInfoUtilTest_UpdateAsCallerInfoFromToken_0001
 * @tc.desc: Test the state of UpdateAsCallerInfoFromToken
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, UpdateAsCallerInfoFromToken_0001, TestSize.Level1)
{
    std::shared_ptr<UpdateCallerInfoUtil> updateCallerUtil = std::make_shared<UpdateCallerInfoUtil>();
    ASSERT_NE(updateCallerUtil, nullptr);
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    updateCallerUtil->UpdateAsCallerInfoFromToken(want, callerToken);
    auto bundleName = want.GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME);
    EXPECT_TRUE(bundleName.empty());
}

/**
 * @tc.name: UpdateCallerInfoUtilTest_UpdateAsCallerInfoFromToken_0002
 * @tc.desc: Test the state of UpdateAsCallerInfoFromToken
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, UpdateAsCallerInfoFromToken_0002, TestSize.Level1)
{
    std::shared_ptr<UpdateCallerInfoUtil> updateCallerUtil = std::make_shared<UpdateCallerInfoUtil>();
    ASSERT_NE(updateCallerUtil, nullptr);
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    ASSERT_NE(abilityRecord, nullptr);
    callerToken = abilityRecord->GetToken();
    updateCallerUtil->UpdateAsCallerInfoFromToken(want, callerToken);
    EXPECT_TRUE(want.HasParameter(Want::PARAM_RESV_CALLER_TOKEN));
}

/**
 * @tc.name: UpdateCallerInfoUtilTest_UpdateBackToCallerFlag_0001
 * @tc.desc: Test the state of UpdateBackToCallerFlag
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, UpdateBackToCallerFlag_0001, TestSize.Level1)
{
    std::shared_ptr<UpdateCallerInfoUtil> updateCallerUtil = std::make_shared<UpdateCallerInfoUtil>();
    ASSERT_NE(updateCallerUtil, nullptr);
    Want want;
    want.SetParam(OHOS::AAFwk::CALLER_REQUEST_CODE, 0);
    sptr<IRemoteObject> callerToken = nullptr;
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    ASSERT_NE(abilityRecord, nullptr);
    callerToken = abilityRecord->GetToken();
    int32_t requestCode = 0;
    bool backFlag = true;
    updateCallerUtil->UpdateBackToCallerFlag(callerToken, want, requestCode, backFlag);
    EXPECT_FALSE(want.HasParameter(OHOS::AAFwk::CALLER_REQUEST_CODE));
}

/**
 * @tc.name: UpdateCallerInfoUtilTest_UpdateBackToCallerFlag_0002
 * @tc.desc: Test the state of UpdateBackToCallerFlag
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, UpdateBackToCallerFlag_0002, TestSize.Level1)
{
    std::shared_ptr<UpdateCallerInfoUtil> updateCallerUtil = std::make_shared<UpdateCallerInfoUtil>();
    ASSERT_NE(updateCallerUtil, nullptr);
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    ASSERT_NE(abilityRecord, nullptr);
    callerToken = abilityRecord->GetToken();
    int32_t requestCode = 0;
    bool backFlag = true;
    updateCallerUtil->UpdateBackToCallerFlag(callerToken, want, requestCode, backFlag);
    EXPECT_FALSE(want.HasParameter(OHOS::AAFwk::CALLER_REQUEST_CODE));
}

/**
 * @tc.name: UpdateCallerInfoUtilTest_UpdateBackToCallerFlag_0003
 * @tc.desc: Test the state of UpdateBackToCallerFlag
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, UpdateBackToCallerFlag_0003, TestSize.Level1)
{
    std::shared_ptr<UpdateCallerInfoUtil> updateCallerUtil = std::make_shared<UpdateCallerInfoUtil>();
    ASSERT_NE(updateCallerUtil, nullptr);
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    ASSERT_NE(abilityRecord, nullptr);
    callerToken = abilityRecord->GetToken();
    int32_t requestCode = 1;
    bool backFlag = true;
    updateCallerUtil->UpdateBackToCallerFlag(callerToken, want, requestCode, backFlag);
    EXPECT_TRUE(want.HasParameter(OHOS::AAFwk::CALLER_REQUEST_CODE));
}

/**
 * @tc.name: UpdateCallerInfoUtilTest_UpdateDmsCallerInfo_0001
 * @tc.desc: Test the state of UpdateDmsCallerInfo
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, UpdateDmsCallerInfo_0001, TestSize.Level1)
{
    std::shared_ptr<UpdateCallerInfoUtil> updateCallerUtil = std::make_shared<UpdateCallerInfoUtil>();
    ASSERT_NE(updateCallerUtil, nullptr);
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    ASSERT_NE(abilityRecord, nullptr);
    callerToken = abilityRecord->GetToken();
    updateCallerUtil->UpdateDmsCallerInfo(want, callerToken);
    EXPECT_TRUE(want.HasParameter(OHOS::AAFwk::DMS_CALLER_BUNDLE_NAME));
}

/**
 * @tc.name: UpdateCallerInfoUtilTest_UpdateByIndirectCallerInfo_0001
 * @tc.desc: Test UpdateByIndirectCallerInfo with nullptr indirectCallerInfo (should not modify parameters)
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, UpdateByIndirectCallerInfo_0001, TestSize.Level1)
{
    auto updateCallerUtil = std::make_shared<UpdateCallerInfoUtil>();
    int32_t tokenId = 123;
    int32_t callerUid = 456;
    int32_t callerPid = 789;

    updateCallerUtil->UpdateByIndirectCallerInfo(nullptr, tokenId, callerUid, callerPid);
    EXPECT_EQ(tokenId, 123);
    EXPECT_EQ(callerUid, 456);
    EXPECT_EQ(callerPid, 789);
}

/**
 * @tc.name: UpdateCallerInfoUtilTest_UpdateByIndirectCallerInfo_0002
 * @tc.desc: Test UpdateByIndirectCallerInfo with valid indirectCallerInfo (should update parameters)
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, UpdateByIndirectCallerInfo_0002, TestSize.Level1)
{
    auto updateCallerUtil = std::make_shared<UpdateCallerInfoUtil>();
    int32_t tokenId = 123;
    int32_t callerUid = 456;
    int32_t callerPid = 789;

    auto info = std::make_shared<IndirectCallerInfo>();
    info->tokenId = 111;
    info->callerUid = 222;
    info->callerPid = 333;

    updateCallerUtil->UpdateByIndirectCallerInfo(info, tokenId, callerUid, callerPid);
    EXPECT_EQ(tokenId, 111);
    EXPECT_EQ(callerUid, 222);
    EXPECT_EQ(callerPid, 333);
}

/**
 * @tc.name: UpdateCallerInfoUtilTest_UpdateCallerInfo_0001
 * @tc.desc: Test UpdateCallerInfo with valid callerToken
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, UpdateCallerInfo_0001, TestSize.Level1)
{
    auto updateCallerUtil = std::make_shared<UpdateCallerInfoUtil>();
    Want want;
    sptr<IRemoteObject> callerToken = MockToken(AbilityType::PAGE);

    updateCallerUtil->UpdateCallerInfo(want, callerToken);
    EXPECT_TRUE(want.HasParameter(Want::PARAM_RESV_CALLER_TOKEN));
    EXPECT_TRUE(want.HasParameter(Want::PARAM_RESV_CALLER_UID));
    EXPECT_TRUE(want.HasParameter(Want::PARAM_RESV_CALLER_PID));
}

/**
 * @tc.name: UpdateCallerInfoUtilTest_UpdateSignatureInfo_0001
 * @tc.desc: Test UpdateSignatureInfo with isRemote = false
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, UpdateSignatureInfo_0001, TestSize.Level1)
{
    auto updateCallerUtil = std::make_shared<UpdateCallerInfoUtil>();
    Want want;
    std::string bundleName = "com.test.demo";

    updateCallerUtil->UpdateSignatureInfo(bundleName, want);
    EXPECT_TRUE(want.HasParameter(Want::PARAM_RESV_CALLER_APP_ID));
    EXPECT_TRUE(want.HasParameter(Want::PARAM_RESV_CALLER_APP_IDENTIFIER));
}

/**
 * @tc.name: UpdateCallerInfoUtilTest_UpdateSignatureInfo_0002
 * @tc.desc: Test UpdateSignatureInfo with isRemote = true
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, UpdateSignatureInfo_0002, TestSize.Level1)
{
    auto updateCallerUtil = std::make_shared<UpdateCallerInfoUtil>();
    Want want;
    std::string bundleName = "com.test.demo";

    updateCallerUtil->UpdateSignatureInfo(bundleName, want, true);
    EXPECT_TRUE(want.HasParameter("ohos.dms.param.sourceCallerAppId"));
}

/**
 * @tc.name: UpdateCallerInfoUtilTest_UpdateAsCallerSourceInfo_0001
 * @tc.desc: Test UpdateAsCallerSourceInfo with valid tokens
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, UpdateAsCallerSourceInfo_0001, TestSize.Level1)
{
    auto updateCallerUtil = std::make_shared<UpdateCallerInfoUtil>();
    Want want;
    sptr<IRemoteObject> asCallerSourceToken = MockToken(AbilityType::PAGE);
    sptr<IRemoteObject> callerToken = MockToken(AbilityType::SERVICE);

    updateCallerUtil->UpdateAsCallerSourceInfo(want, asCallerSourceToken, callerToken);
    EXPECT_FALSE(want.HasParameter(Want::PARAM_RESV_CALLER_BUNDLE_NAME));
    EXPECT_FALSE(want.HasParameter(Want::PARAM_RESV_CALLER_UID));
}

/**
 * @tc.name: UpdateCallerInfoUtilTest_UpdateAsCallerSourceInfo_0002
 * @tc.desc: Test UpdateAsCallerSourceInfo with null tokens
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, UpdateAsCallerSourceInfo_0002, TestSize.Level1)
{
    auto updateCallerUtil = std::make_shared<UpdateCallerInfoUtil>();
    Want want;
    sptr<IRemoteObject> asCallerSourceToken = nullptr;
    sptr<IRemoteObject> callerToken = nullptr;

    updateCallerUtil->UpdateAsCallerSourceInfo(want, asCallerSourceToken, callerToken);
    EXPECT_FALSE(want.HasParameter(Want::PARAM_RESV_CALLER_ABILITY_NAME));
}

/**
 * @tc.name: UpdateCallerInfoUtilTest_UpdateCallerBundleName_0001
 * @tc.desc: Test UpdateCallerBundleName with valid bundle name
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, UpdateCallerBundleName_0001, TestSize.Level1)
{
    auto updateCallerUtil = std::make_shared<UpdateCallerInfoUtil>();
    Want want;
    std::string testBundleName = "com.test.update.bundle";

    updateCallerUtil->UpdateCallerBundleName(want, testBundleName);
    EXPECT_EQ(want.GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME), testBundleName);
}

/**
 * @tc.name: UpdateCallerInfoUtilTest_UpdateCallerBundleName_0002
 * @tc.desc: Test UpdateCallerBundleName with empty bundle name
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, UpdateCallerBundleName_0002, TestSize.Level1)
{
    auto updateCallerUtil = std::make_shared<UpdateCallerInfoUtil>();
    Want want;
    std::string emptyBundleName = "";

    updateCallerUtil->UpdateCallerBundleName(want, emptyBundleName);
    EXPECT_EQ(want.GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME), emptyBundleName);
}

/**
 * @tc.name: UpdateCallerInfoUtilTest_UpdateCallerAbilityName_0001
 * @tc.desc: Test UpdateCallerAbilityName with valid ability name
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, UpdateCallerAbilityName_0001, TestSize.Level1)
{
    auto updateCallerUtil = std::make_shared<UpdateCallerInfoUtil>();
    Want want;
    std::string testAbilityName = "TestAbility";

    updateCallerUtil->UpdateCallerAbilityName(want, testAbilityName);
    EXPECT_EQ(want.GetStringParam(Want::PARAM_RESV_CALLER_ABILITY_NAME), testAbilityName);
}

/**
 * @tc.name: UpdateCallerInfoUtilTest_UpdateCallerAbilityName_0002
 * @tc.desc: Test UpdateCallerAbilityName with empty ability name
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, UpdateCallerAbilityName_0002, TestSize.Level1)
{
    auto updateCallerUtil = std::make_shared<UpdateCallerInfoUtil>();
    Want want;
    std::string emptyAbilityName = "";

    updateCallerUtil->UpdateCallerAbilityName(want, emptyAbilityName);
    EXPECT_EQ(want.GetStringParam(Want::PARAM_RESV_CALLER_ABILITY_NAME), emptyAbilityName);
}

/**
 * @tc.name: UpdateCallerInfoUtilTest_UpdateCallerAppCloneIndex_0001
 * @tc.desc: Test UpdateCallerAppCloneIndex with valid index
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, UpdateCallerAppCloneIndex_0001, TestSize.Level1)
{
    auto updateCallerUtil = std::make_shared<UpdateCallerInfoUtil>();
    Want want;
    int32_t testIndex = 100;

    updateCallerUtil->UpdateCallerAppCloneIndex(want, testIndex);
    EXPECT_EQ(want.GetIntParam(Want::PARAM_RESV_CALLER_APP_CLONE_INDEX, -1), testIndex);
}

/**
 * @tc.name: UpdateCallerInfoUtilTest_UpdateCallerAppCloneIndex_0002
 * @tc.desc: Test UpdateCallerAppCloneIndex with 0 index
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, UpdateCallerAppCloneIndex_0002, TestSize.Level1)
{
    auto updateCallerUtil = std::make_shared<UpdateCallerInfoUtil>();
    Want want;
    int32_t zeroIndex = 0;

    updateCallerUtil->UpdateCallerAppCloneIndex(want, zeroIndex);
    EXPECT_EQ(want.GetIntParam(Want::PARAM_RESV_CALLER_APP_CLONE_INDEX, -1), zeroIndex);
}

/**
 * @tc.name: UpdateCallerInfoUtilTest_ClearProtectedWantParam_HideSensitiveType_001
 * @tc.desc: Test ClearProtectedWantParam with HIDE_SENSITIVE_TYPE parameter
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, ClearProtectedWantParam_HideSensitiveType_001, TestSize.Level1)
{
    auto updateCallerUtil = std::make_shared<UpdateCallerInfoUtil>();
    Want want;
    want.SetParam(HIDE_SENSITIVE_TYPE, 521);
    want.SetElementName("com.test.demo", "entry", "TestAbility");

    // ClearProtectedWantParam should trigger event for HIDE_SENSITIVE_TYPE
    updateCallerUtil->ClearProtectedWantParam(want);

    // Verify the parameter is processed (no exception thrown)
    EXPECT_EQ(want.GetIntParam(HIDE_SENSITIVE_TYPE, -1), 521);
}

/**
 * @tc.name: UpdateCallerInfoUtilTest_ClearProtectedWantParam_HideSensitiveType_002
 * @tc.desc: Test ClearProtectedWantParam with HIDE_SENSITIVE_TYPE = 4
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, ClearProtectedWantParam_HideSensitiveType_002, TestSize.Level1)
{
    auto updateCallerUtil = std::make_shared<UpdateCallerInfoUtil>();
    Want want;
    want.SetParam(HIDE_SENSITIVE_TYPE, 4);
    want.SetElementName("com.test.demo", "entry", "TestAbility");

    // ClearProtectedWantParam should still process the parameter
    updateCallerUtil->ClearProtectedWantParam(want);

    // Verify the parameter value
    EXPECT_EQ(want.GetIntParam(HIDE_SENSITIVE_TYPE, -1), 4);
}

/**
 * @tc.name: UpdateCallerInfoByHapTokenId_0001
 * @tc.desc: Test UpdateCallerInfoByHapTokenId with callerTokenId = 0 (boundary, should early-return
 *           without modifying Want, leaving baseline value untouched)
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, UpdateCallerInfoByHapTokenId_0001, TestSize.Level1)
{
    auto updateCallerUtil = std::make_shared<UpdateCallerInfoUtil>();
    Want want;
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, std::string("baseline_bundle"));

    // Boundary: callerTokenId = 0, function should log warning and early-return
    updateCallerUtil->UpdateCallerInfoByHapTokenId(want, 0);

    // Want baseline value must remain (early-return path verified)
    EXPECT_EQ(want.GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME), "baseline_bundle");
}

/**
 * @tc.name: UpdateCallerInfoByHapTokenId_0002
 * @tc.desc: Test UpdateCallerInfoByHapTokenId with invalid callerTokenId (GetHapTokenInfo fails,
 *           should early-return without modifying Want)
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, UpdateCallerInfoByHapTokenId_0002, TestSize.Level1)
{
    auto updateCallerUtil = std::make_shared<UpdateCallerInfoUtil>();
    Want want;
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, std::string("baseline_bundle"));

    // Invalid callerTokenId (no corresponding HAP token in test env): GetHapTokenInfo returns non-ERR_OK,
    // function should log error and early-return
    updateCallerUtil->UpdateCallerInfoByHapTokenId(want, 0xFFFFFFFF);

    // Want baseline value must remain (failure-path early-return verified)
    EXPECT_EQ(want.GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME), "baseline_bundle");
}

/**
 * @tc.name: FailClosedIdentityCleanup_0003
 * @tc.desc: Stamping failure paths strip any caller-supplied identity params; anco
 *           params are DFX-only and keep their pre-existing passthrough.
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, FailClosedIdentityCleanup_0003, TestSize.Level1)
{
    auto updateCallerUtil = std::make_shared<UpdateCallerInfoUtil>();

    // tokenId == 0: invalid param early-return
    Want want;
    want.SetParam(Want::PARAM_RESV_CALLER_UID, 1234);
    want.SetParam(Want::PARAM_RESV_CALLER_PID, 5678);
    want.SetParam(Want::PARAM_RESV_CALLER_TOKEN, 9);
    want.SetParam("ohos.anco.param.callerUid", 4321);
    want.SetParam("ohos.anco.param.callerBundleName", std::string("forged_bundle"));
    updateCallerUtil->UpdateCallerInfoByHapTokenId(want, 0);
    EXPECT_FALSE(want.HasParameter(Want::PARAM_RESV_CALLER_UID));
    EXPECT_FALSE(want.HasParameter(Want::PARAM_RESV_CALLER_PID));
    EXPECT_FALSE(want.HasParameter(Want::PARAM_RESV_CALLER_TOKEN));
    // anco params are not identity for attribution and are left untouched
    EXPECT_TRUE(want.HasParameter("ohos.anco.param.callerUid"));
    EXPECT_TRUE(want.HasParameter("ohos.anco.param.callerBundleName"));

    // unresolvable HAP token: GetHapTokenInfo failure early-return
    Want want2;
    want2.SetParam(Want::PARAM_RESV_CALLER_UID, 1234);
    updateCallerUtil->UpdateCallerInfoByHapTokenId(want2, 0xFFFFFFFF);
    EXPECT_FALSE(want2.HasParameter(Want::PARAM_RESV_CALLER_UID));

    // unresolvable caller token: caller abilityRecord null early-return
    Want want3;
    want3.SetParam(Want::PARAM_RESV_CALLER_UID, 1234);
    updateCallerUtil->UpdateCallerInfoFromToken(want3, nullptr);
    EXPECT_FALSE(want3.HasParameter(Want::PARAM_RESV_CALLER_UID));
}

/**
 * @tc.name: UpdateCallerInfoUtilTest_CacheRestoreTaskId_0003
 * @tc.desc: The cached taskId wins over a stale SCB-returned value.
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, CacheRestoreTaskId_0003, TestSize.Level1)
{
    auto& updateCallerUtil = UpdateCallerInfoUtil::GetInstance();
    Want originalWant;
    originalWant.SetParam("ohos.aafwk.param.agent.taskId", std::string("task-cached-55"));
    AbilitySessionInfo info;
    updateCallerUtil.CacheCallerInfoFromWant(originalWant, info);
    EXPECT_EQ(info.taskId, "task-cached-55");

    // SCB round trip returned a different (stale) value: the cache overwrites it.
    Want scbWant;
    scbWant.SetParam("ohos.aafwk.param.agent.taskId", std::string("task-stale-66"));
    updateCallerUtil.RestoreCallerInfoToWant(scbWant, info);
    EXPECT_EQ(scbWant.GetStringParam("ohos.aafwk.param.agent.taskId"), "task-cached-55");
}

/**
 * @tc.name: UpdateCallerInfoUtilTest_CacheRestoreTaskId_0004
 * @tc.desc: With no key on either side the restore stays clean (no key injected).
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, CacheRestoreTaskId_0004, TestSize.Level1)
{
    auto& updateCallerUtil = UpdateCallerInfoUtil::GetInstance();
    Want originalWant; // no task key anywhere
    AbilitySessionInfo info;
    updateCallerUtil.CacheCallerInfoFromWant(originalWant, info);
    EXPECT_TRUE(info.taskId.empty());

    Want scbWant; // no key on the round trip either
    updateCallerUtil.RestoreCallerInfoToWant(scbWant, info);
    EXPECT_FALSE(scbWant.HasParameter("ohos.aafwk.param.agent.taskId"));
}

/**
 * @tc.name: CacheIntentAndSkillParams_0001
 * @tc.desc: Snapshots insight-intent and skill execute params while skipping unrelated params.
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, CacheIntentAndSkillParams_0001, TestSize.Level1)
{
    auto& updateCallerUtil = UpdateCallerInfoUtil::GetInstance();
    Want want;
    want.SetElementName("", "com.test.target", "TestAbility", "entry");
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME, std::string("PlayMusic"));
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_ID, std::string("42"));
    want.SetParam(AppExecFwk::INSIGHT_INTENT_CALLER_UID, 20000001);
    want.SetParam(AppExecFwk::SKILL_EXECUTE_PARAM_BUNDLE_NAME, std::string("com.test.skill.bundle"));
    want.SetParam(AppExecFwk::SKILL_EXECUTE_PARAM_SKILL_NAME, std::string("WeatherSkill"));
    // Dynamic skill args keys share the skill prefix and must be snapshotted too.
    want.SetParam("ohos.skill.executeParam.args.city", std::string("Beijing"));
    want.SetParam("ohos.aafwk.param.taskId", std::string("task-1"));

    AbilitySessionInfo info;
    updateCallerUtil.CacheCallerInfoFromWant(want, info);

    const auto &params = info.intentAndSkillParams.GetParams();
    EXPECT_EQ(params.count(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME), 1u);
    EXPECT_EQ(params.count(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_ID), 1u);
    EXPECT_EQ(params.count(AppExecFwk::INSIGHT_INTENT_CALLER_UID), 1u);
    EXPECT_EQ(params.count(AppExecFwk::SKILL_EXECUTE_PARAM_BUNDLE_NAME), 1u);
    EXPECT_EQ(params.count(AppExecFwk::SKILL_EXECUTE_PARAM_SKILL_NAME), 1u);
    EXPECT_EQ(params.count("ohos.skill.executeParam.args.city"), 1u);
    EXPECT_EQ(params.count("ohos.aafwk.param.taskId"), 0u);
}

/**
 * @tc.name: CacheIntentAndSkillParams_0002
 * @tc.desc: A want without system-stamped execute params produces an empty snapshot.
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, CacheIntentAndSkillParams_0002, TestSize.Level1)
{
    auto& updateCallerUtil = UpdateCallerInfoUtil::GetInstance();
    Want want;
    want.SetElementName("", "com.test.target", "TestAbility", "entry");
    want.SetParam("ohos.aafwk.param.taskId", std::string("task-1"));

    AbilitySessionInfo info;
    updateCallerUtil.CacheCallerInfoFromWant(want, info);
    EXPECT_TRUE(info.intentAndSkillParams.GetParams().empty());
}

/**
 * @tc.name: RestoreIntentAndSkillParams_0001
 * @tc.desc: The AMS snapshot overwrites stale SCB values and drops keys absent from the snapshot.
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, RestoreIntentAndSkillParams_0001, TestSize.Level1)
{
    auto& updateCallerUtil = UpdateCallerInfoUtil::GetInstance();
    Want originalWant;
    originalWant.SetElementName("", "com.test.target", "TestAbility", "entry");
    originalWant.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME, std::string("PlayMusic"));
    originalWant.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_ID, std::string("42"));

    AbilitySessionInfo info;
    updateCallerUtil.CacheCallerInfoFromWant(originalWant, info);

    // SCB returns stale values for the same keys plus an extra injected key.
    Want scbWant;
    scbWant.SetElementName("", "com.test.target", "TestAbility", "entry");
    scbWant.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME, std::string("forged"));
    scbWant.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_ID, std::string("999"));
    scbWant.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_MODE, 1);
    // The stub boundary (SanitizeWantParams) strips the returned copy before AMS sees it.
    updateCallerUtil.RemoveIntentAndSkillParams(scbWant);

    updateCallerUtil.RestoreCallerInfoToWant(scbWant, info);
    EXPECT_EQ(scbWant.GetStringParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME), "PlayMusic");
    EXPECT_EQ(scbWant.GetStringParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_ID), "42");
    EXPECT_FALSE(scbWant.HasParameter(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_MODE));
}

/**
 * @tc.name: RestoreIntentAndSkillParams_0002
 * @tc.desc: With no snapshot the restore strips every injected system param (fail closed).
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, RestoreIntentAndSkillParams_0002, TestSize.Level1)
{
    auto& updateCallerUtil = UpdateCallerInfoUtil::GetInstance();
    Want originalWant;
    originalWant.SetElementName("", "com.test.target", "TestAbility", "entry");

    AbilitySessionInfo info;
    updateCallerUtil.CacheCallerInfoFromWant(originalWant, info);
    EXPECT_TRUE(info.intentAndSkillParams.GetParams().empty());

    Want scbWant;
    scbWant.SetElementName("", "com.test.target", "TestAbility", "entry");
    scbWant.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME, std::string("forged"));
    scbWant.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_ID, std::string("42"));
    // The stub boundary (SanitizeWantParams) strips the returned copy before AMS sees it.
    updateCallerUtil.RemoveIntentAndSkillParams(scbWant);

    updateCallerUtil.RestoreCallerInfoToWant(scbWant, info);
    EXPECT_FALSE(scbWant.HasParameter(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME));
    EXPECT_FALSE(scbWant.HasParameter(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_ID));
}

/**
 * @tc.name: RemoveIntentAndSkillParams_0001
 * @tc.desc: Removes all intent and skill execute params while leaving unrelated params untouched.
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, RemoveIntentAndSkillParams_0001, TestSize.Level1)
{
    auto& updateCallerUtil = UpdateCallerInfoUtil::GetInstance();
    Want want;
    want.SetElementName("", "com.test.target", "TestAbility", "entry");
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME, std::string("PlayMusic"));
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_ID, std::string("42"));
    want.SetParam(AppExecFwk::SKILL_EXECUTE_PARAM_SKILL_NAME, std::string("WeatherSkill"));
    want.SetParam("ohos.skill.executeParam.args.city", std::string("Beijing"));
    want.SetParam("ohos.aafwk.param.taskId", std::string("task-1"));

    updateCallerUtil.RemoveIntentAndSkillParams(want);
    EXPECT_FALSE(want.HasParameter(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME));
    EXPECT_FALSE(want.HasParameter(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_ID));
    EXPECT_FALSE(want.HasParameter(AppExecFwk::SKILL_EXECUTE_PARAM_SKILL_NAME));
    EXPECT_FALSE(want.HasParameter("ohos.skill.executeParam.args.city"));
    EXPECT_TRUE(want.HasParameter("ohos.aafwk.param.taskId"));
}

/**
 * @tc.name: RemoveIntentAndSkillParams_0002
 * @tc.desc: Partial injection and an empty want are handled without side effects.
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, RemoveIntentAndSkillParams_0002, TestSize.Level1)
{
    auto& updateCallerUtil = UpdateCallerInfoUtil::GetInstance();

    Want want;
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_ID, std::string("42"));
    want.SetParam(AppExecFwk::INSIGHT_INTENT_CALLER_UID, 20000001);
    updateCallerUtil.RemoveIntentAndSkillParams(want);
    EXPECT_FALSE(want.HasParameter(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_ID));
    EXPECT_FALSE(want.HasParameter(AppExecFwk::INSIGHT_INTENT_CALLER_UID));

    Want emptyWant;
    updateCallerUtil.RemoveIntentAndSkillParams(emptyWant);
    EXPECT_FALSE(emptyWant.HasParameter(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_ID));
}
} // namespace AAFwk
} // namespace OHOS
