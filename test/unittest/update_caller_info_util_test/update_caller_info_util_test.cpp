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
} // namespace AAFwk
} // namespace OHOS
