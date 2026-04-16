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

#include "gtest/gtest.h"

#include "ability_config.h"
#include "ability_record.h"
#define private public
#include "dialog_session_manager.h"
#undef private
#include "hilog_tag_wrapper.h"
#include "mock_ability_token.h"
#include "start_ability_utils.h"

using OHOS::AppExecFwk::AbilityType;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
namespace {
constexpr int32_t TEST_USER_ID = 10001;
constexpr int32_t TEST_ERMS_ISALLOW_RESULTCODE = 9;
const std::string TEST_BUNDLE_NAME = "com.test.demo";
const std::string TEST_DIALOG_SESSION_ID = "dialogSessionId";
const std::string APP_LAUNCH_TRUSTLIST = "ohos.params.appLaunchTrustList";
const std::string SHOW_DEFAULT_PICKER_FLAG = "ohos.ability.params.showDefaultPicker";
constexpr const char* KEY_REQUEST_ID = "com.ohos.param.requestId";
}

class DialogSessionManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    std::shared_ptr<AbilityRecord> MockAbilityRecord(AbilityType);
    sptr<Token> MockToken(AbilityType);
};

void DialogSessionManagerTest::SetUpTestCase(void)
{}

void DialogSessionManagerTest::TearDownTestCase(void)
{}

void DialogSessionManagerTest::SetUp()
{}

void DialogSessionManagerTest::TearDown()
{}

std::shared_ptr<AbilityRecord> DialogSessionManagerTest::MockAbilityRecord(AbilityType abilityType)
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = TEST_BUNDLE_NAME;
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = abilityType;
    return AbilityRecord::CreateAbilityRecord(abilityRequest);
}

sptr<Token> DialogSessionManagerTest::MockToken(AbilityType abilityType)
{
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(abilityType);
    if (!abilityRecord) {
        return nullptr;
    }
    return abilityRecord->GetToken();
}

/**
 * @tc.name: GetStartupSessionInfoTest_0100
 * @tc.desc: Test GetStartupSessionInfo
 * @tc.type: FUNC
 */
HWTEST_F(DialogSessionManagerTest, GetStartupSessionInfoTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetStartupSessionInfoTest_0100 start";
    DialogSessionManager dialogSessionManager;
    std::shared_ptr<StartupSessionInfo> ret = dialogSessionManager.GetStartupSessionInfo(TEST_DIALOG_SESSION_ID);
    EXPECT_EQ(ret, nullptr);

    AbilityRequest abilityRequest;
    dialogSessionManager.SetStartupSessionInfo(TEST_DIALOG_SESSION_ID, abilityRequest);
    ret = dialogSessionManager.GetStartupSessionInfo(TEST_DIALOG_SESSION_ID);
    EXPECT_NE(ret, nullptr);
    GTEST_LOG_(INFO) << "GetStartupSessionInfoTest_0100 end";
}

/**
 * @tc.name: SendDialogResultTest_0100
 * @tc.desc: Test SendDialogResult
 * @tc.type: FUNC
 */
HWTEST_F(DialogSessionManagerTest, SendDialogResultTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SendDialogResultTest_0100 start";
    AbilityRequest abilityRequest;
    DialogSessionManager dialogSessionManager;
    Want want;
    bool isAllowed = false;
    int32_t ret = dialogSessionManager.SendDialogResult(want, TEST_DIALOG_SESSION_ID, isAllowed);
    EXPECT_EQ(ret, ERR_OK);
    
    isAllowed = true;
    ret = dialogSessionManager.SendDialogResult(want, TEST_DIALOG_SESSION_ID, isAllowed);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    dialogSessionManager.SetStartupSessionInfo(TEST_DIALOG_SESSION_ID, abilityRequest);
    ret = dialogSessionManager.SendDialogResult(want, TEST_DIALOG_SESSION_ID, isAllowed);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "SendDialogResultTest_0100 end";
}

/**
 * @tc.name: SendDialogResultTest_0200
 * @tc.desc: Test SendDialogResult
 * @tc.type: FUNC
 */
HWTEST_F(DialogSessionManagerTest, SendDialogResultTest_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SendDialogResultTest_0200 start";
    AbilityRequest abilityRequest;
    DialogSessionManager dialogSessionManager;
    Want want;
    bool isAllowed = true;
    sptr<DialogSessionInfo> dilogSessionInfo = nullptr;
    std::shared_ptr<DialogCallerInfo> dialogCallerInfo = std::make_shared<DialogCallerInfo>();

    dialogSessionManager.SetDialogSessionInfo(TEST_DIALOG_SESSION_ID, dilogSessionInfo, dialogCallerInfo);
    int32_t ret = dialogSessionManager.SendDialogResult(want, TEST_DIALOG_SESSION_ID, isAllowed);
    EXPECT_EQ(ret, ERR_NULL_INTERCEPTOR_EXECUTER);

    want.SetParam(Want::PARAM_APP_CLONE_INDEX_KEY, 0);
    ret = dialogSessionManager.SendDialogResult(want, TEST_DIALOG_SESSION_ID, isAllowed);
    EXPECT_EQ(ret, ERR_NULL_INTERCEPTOR_EXECUTER);
    GTEST_LOG_(INFO) << "SendDialogResultTest_0200 end";
}

/**
 * @tc.name: SendDialogResultTest_0300
 * @tc.desc: Test SendDialogResult
 * @tc.type: FUNC
 */
HWTEST_F(DialogSessionManagerTest, SendDialogResultTest_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SendDialogResultTest_0300 start";
    AbilityRequest abilityRequest;
    DialogSessionManager dialogSessionManager;
    Want want;
    bool isAllowed = true;
    abilityRequest.isQueryERMS = true;
    dialogSessionManager.SetQueryERMSInfo(TEST_DIALOG_SESSION_ID, abilityRequest);
    int32_t ret = dialogSessionManager.SendDialogResult(want, TEST_DIALOG_SESSION_ID, isAllowed);
    EXPECT_EQ(ret, ERR_OK);
    GTEST_LOG_(INFO) << "SendDialogResultTest_0300 end";
}

/**
 * @tc.name: SendDialogResultTest_0400
 * @tc.desc: Test SendDialogResult
 * @tc.type: FUNC
 */
HWTEST_F(DialogSessionManagerTest, SendDialogResultTest_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SendDialogResultTest_0400 start";
    AbilityRequest abilityRequest;
    DialogSessionManager dialogSessionManager;
    Want want;
    bool isAllowed = false;
    sptr<DialogSessionInfo> dialogSessionInfo = nullptr;
    std::shared_ptr<DialogCallerInfo> dialogCallerInfo = std::make_shared<DialogCallerInfo>();
    dialogSessionManager.SetDialogSessionInfo(TEST_DIALOG_SESSION_ID, dialogSessionInfo, dialogCallerInfo);
    int32_t ret = dialogSessionManager.SendDialogResult(want, TEST_DIALOG_SESSION_ID, isAllowed);
    EXPECT_EQ(ret, ERR_OK);

    dialogCallerInfo->sessionToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    dialogSessionManager.SetDialogSessionInfo(TEST_DIALOG_SESSION_ID, dialogSessionInfo, dialogCallerInfo);
    ret = dialogSessionManager.SendDialogResult(want, TEST_DIALOG_SESSION_ID, isAllowed);
    EXPECT_EQ(ret, ERR_OK);
    bool cleared = dialogSessionManager.dialogSessionInfoMap_.find(TEST_DIALOG_SESSION_ID) ==
        dialogSessionManager.dialogSessionInfoMap_.end();
    EXPECT_TRUE(cleared);
    GTEST_LOG_(INFO) << "SendDialogResultTest_0400 end";
}

/**
 * @tc.name: NotifySCBToRecoveryAfterInterceptionTest_0100
 * @tc.desc: Test NotifySCBToRecoveryAfterInterception
 * @tc.type: FUNC
 */
HWTEST_F(DialogSessionManagerTest, NotifySCBToRecoveryAfterInterceptionTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifySCBToRecoveryAfterInterceptionTest_0100 start";
    AbilityRequest abilityRequest;
    DialogSessionManager dialogSessionManager;

    int32_t ret = dialogSessionManager.NotifySCBToRecoveryAfterInterception(TEST_DIALOG_SESSION_ID, abilityRequest);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "NotifySCBToRecoveryAfterInterceptionTest_0100 end";
}

/**
 * @tc.name: CreateModalDialogCommonTest_0100
 * @tc.desc: Test CreateModalDialogCommon
 * @tc.type: FUNC
 */
HWTEST_F(DialogSessionManagerTest, CreateModalDialogCommonTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateModalDialogCommonTest_0100 start";
    Want replaceWant;
    sptr<IRemoteObject> callerToken = MockToken(AbilityType::PAGE);
    DialogSessionManager dialogSessionManager;

    int32_t ret = dialogSessionManager.CreateModalDialogCommon(replaceWant, callerToken, TEST_DIALOG_SESSION_ID);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "CreateModalDialogCommonTest_0100 end";
}

/**
 * @tc.name: IsCreateCloneSelectorDialogTest_0100
 * @tc.desc: Test IsCreateCloneSelectorDialog
 * @tc.type: FUNC
 */
HWTEST_F(DialogSessionManagerTest, IsCreateCloneSelectorDialogTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsCreateCloneSelectorDialogTest_0100 start";
    DialogSessionManager dialogSessionManager;

    StartAbilityUtils::isWantWithAppCloneIndex = true;
    bool ret = dialogSessionManager.IsCreateCloneSelectorDialog(TEST_BUNDLE_NAME, TEST_USER_ID);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "IsCreateCloneSelectorDialogTest_0100 end";
}

/**
 * @tc.name: UpdateExtensionWantWithDialogCallerInfo_0100
 * @tc.desc: callerToken is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(DialogSessionManagerTest, UpdateExtensionWantWithDialogCallerInfo_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateExtensionWantWithDialogCallerInfo_0100 start";
    std::string dialogSessionId = "";
    AbilityRequest abilityRequest;
    bool isSCBCall = false;
    sptr<IRemoteObject> callerToken = nullptr;
    abilityRequest.want.SetParam(TEST_DIALOG_SESSION_ID, dialogSessionId);
    DialogSessionManager dialogSessionManager;
    auto ret = dialogSessionManager.UpdateExtensionWantWithDialogCallerInfo(abilityRequest, callerToken, isSCBCall);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "UpdateExtensionWantWithDialogCallerInfo_0100 end";
}

/**
 * @tc.name: UpdateExtensionWantWithDialogCallerInfo_0200
 * @tc.desc: dialogSessionId is empty
 * @tc.type: FUNC
 */
HWTEST_F(DialogSessionManagerTest, UpdateExtensionWantWithDialogCallerInfo_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateExtensionWantWithDialogCallerInfo_0200 start";
    std::string dialogSessionId = "";
    AbilityRequest abilityRequest;
    bool isSCBCall = false;
    sptr<IRemoteObject> callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    abilityRequest.want.SetParam(TEST_DIALOG_SESSION_ID, dialogSessionId);
    DialogSessionManager dialogSessionManager;
    auto ret = dialogSessionManager.UpdateExtensionWantWithDialogCallerInfo(abilityRequest, callerToken, isSCBCall);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "UpdateExtensionWantWithDialogCallerInfo_0200 end";
}

/**
 * @tc.name: UpdateExtensionWantWithDialogCallerInfo_0300
 * @tc.desc: can not find dialogCallerInfo by dialogSessionId
 * @tc.type: FUNC
 */
HWTEST_F(DialogSessionManagerTest, UpdateExtensionWantWithDialogCallerInfo_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateExtensionWantWithDialogCallerInfo_0300 start";
    std::string dialogSessionId = "1000001";
    AbilityRequest abilityRequest;
    bool isSCBCall = false;
    sptr<IRemoteObject> callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    abilityRequest.want.SetParam(TEST_DIALOG_SESSION_ID, dialogSessionId);
    DialogSessionManager dialogSessionManager;
    auto ret = dialogSessionManager.UpdateExtensionWantWithDialogCallerInfo(abilityRequest, callerToken, isSCBCall);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "UpdateExtensionWantWithDialogCallerInfo_0300 end";
}

/**
 * @tc.name: UpdateExtensionWantWithDialogCallerInfo_0400
 * @tc.desc: dialog callerInfo do not need grant uri permission
 * @tc.type: FUNC
 */
HWTEST_F(DialogSessionManagerTest, UpdateExtensionWantWithDialogCallerInfo_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateExtensionWantWithDialogCallerInfo_0400 start";
    std::string dialogSessionId = "1000001";
    AbilityRequest abilityRequest;
    bool isSCBCall = false;
    sptr<IRemoteObject> callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    abilityRequest.want.SetParam(TEST_DIALOG_SESSION_ID, dialogSessionId);

    std::shared_ptr<DialogCallerInfo> dialogCallerInfo = std::make_shared<DialogCallerInfo>();
    dialogCallerInfo->needGrantUriPermission = false;
    DialogSessionManager dialogSessionManager;
    dialogSessionManager.dialogCallerInfoMap_[dialogSessionId] = dialogCallerInfo;
    auto ret = dialogSessionManager.UpdateExtensionWantWithDialogCallerInfo(abilityRequest, callerToken, isSCBCall);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "UpdateExtensionWantWithDialogCallerInfo_0400 end";
}

/**
 * @tc.name: UpdateExtensionWantWithDialogCallerInfo_0500
 * @tc.desc: do not have uri permission flag
 * @tc.type: FUNC
 */
HWTEST_F(DialogSessionManagerTest, UpdateExtensionWantWithDialogCallerInfo_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateExtensionWantWithDialogCallerInfo_0500 start";
    std::string dialogSessionId = "1000001";
    AbilityRequest abilityRequest;
    bool isSCBCall = false;
    sptr<IRemoteObject> callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    abilityRequest.want.SetParam(TEST_DIALOG_SESSION_ID, dialogSessionId);

    std::shared_ptr<DialogCallerInfo> dialogCallerInfo = std::make_shared<DialogCallerInfo>();
    dialogCallerInfo->needGrantUriPermission = true;
    DialogSessionManager dialogSessionManager;
    dialogSessionManager.dialogCallerInfoMap_[dialogSessionId] = dialogCallerInfo;
    auto ret = dialogSessionManager.UpdateExtensionWantWithDialogCallerInfo(abilityRequest, callerToken, isSCBCall);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "UpdateExtensionWantWithDialogCallerInfo_0500 end";
}

/**
 * @tc.name: UpdateExtensionWantWithDialogCallerInfo_0600
 * @tc.desc: not scb call and have uri permission flag
 * @tc.type: FUNC
 */
HWTEST_F(DialogSessionManagerTest, UpdateExtensionWantWithDialogCallerInfo_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateExtensionWantWithDialogCallerInfo_0600 start";
    std::string dialogSessionId = "1000001";
    AbilityRequest abilityRequest;
    bool isSCBCall = false;
    sptr<IRemoteObject> callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    abilityRequest.want.SetParam(TEST_DIALOG_SESSION_ID, dialogSessionId);

    std::shared_ptr<DialogCallerInfo> dialogCallerInfo = std::make_shared<DialogCallerInfo>();
    uint32_t flag = 1;
    dialogCallerInfo->targetWant.SetFlags(flag);
    dialogCallerInfo->needGrantUriPermission = true;

    DialogSessionManager dialogSessionManager;
    dialogSessionManager.dialogCallerInfoMap_[dialogSessionId] = dialogCallerInfo;
    auto ret = dialogSessionManager.UpdateExtensionWantWithDialogCallerInfo(abilityRequest, callerToken, isSCBCall);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "UpdateExtensionWantWithDialogCallerInfo_0600 end";
}

/**
 * @tc.name: UpdateExtensionWantWithDialogCallerInfo_0700
 * @tc.desc: scb call and have uri permission flag
 * @tc.type: FUNC
 */
HWTEST_F(DialogSessionManagerTest, UpdateExtensionWantWithDialogCallerInfo_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateExtensionWantWithDialogCallerInfo_0600 start";
    std::string dialogSessionId = "1000001";
    AbilityRequest abilityRequest;
    bool isSCBCall = true;
    sptr<IRemoteObject> callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    abilityRequest.want.SetParam(TEST_DIALOG_SESSION_ID, dialogSessionId);

    std::shared_ptr<DialogCallerInfo> dialogCallerInfo = std::make_shared<DialogCallerInfo>();
    uint32_t flag = 1;
    std::string uri = "file://com.example.test/temp.txt";
    std::vector<std::string> uriVec = { uri };
    dialogCallerInfo->targetWant.SetFlags(flag);
    dialogCallerInfo->targetWant.SetUri(uri);
    dialogCallerInfo->targetWant.SetParam(AbilityConfig::PARAMS_STREAM, uriVec);
    dialogCallerInfo->needGrantUriPermission = true;

    DialogSessionManager dialogSessionManager;
    dialogSessionManager.dialogCallerInfoMap_[dialogSessionId] = dialogCallerInfo;
    auto ret = dialogSessionManager.UpdateExtensionWantWithDialogCallerInfo(abilityRequest, callerToken, isSCBCall);
    EXPECT_FALSE(ret);

    EXPECT_EQ(abilityRequest.want.GetFlags(), flag);
    EXPECT_EQ(abilityRequest.want.GetUriString(), uri);
    EXPECT_EQ(abilityRequest.want.GetStringArrayParam(AbilityConfig::PARAMS_STREAM).size(), 1);
    GTEST_LOG_(INFO) << "UpdateExtensionWantWithDialogCallerInfo_0700 end";
}

/**
 * @tc.name: OnlySetDialogCallerInfo_0100
 * @tc.desc: OnlySetDialogCallerInfo
 * @tc.type: FUNC
 */
HWTEST_F(DialogSessionManagerTest, OnlySetDialogCallerInfo_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnlySetDialogCallerInfo_0100 start";
    SelectorType type = SelectorType::INTERCEPTOR_SELECTOR;
    AbilityRequest abilityRequest;
    abilityRequest.want.SetParam(TEST_DIALOG_SESSION_ID, TEST_DIALOG_SESSION_ID);

    DialogSessionManager dialogSessionManager;
    dialogSessionManager.OnlySetDialogCallerInfo(abilityRequest, TEST_USER_ID, type, TEST_DIALOG_SESSION_ID, false);
    EXPECT_NE(dialogSessionManager.GetDialogCallerInfo(TEST_DIALOG_SESSION_ID), nullptr);
    GTEST_LOG_(INFO) << "OnlySetDialogCallerInfo_0100 end";
}

/**
 * @tc.name: OnlySetDialogCallerInfo_0200
 * @tc.desc: OnlySetDialogCallerInfo
 * @tc.type: FUNC
 */
HWTEST_F(DialogSessionManagerTest, OnlySetDialogCallerInfo_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnlySetDialogCallerInfo_0200 start";
    SelectorType type = SelectorType::INTERCEPTOR_SELECTOR;
    AbilityRequest abilityRequest;
    abilityRequest.sessionInfo = sptr<SessionInfo>::MakeSptr();
    sptr<IRemoteObject> sessionToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    abilityRequest.sessionInfo->sessionToken = sessionToken;

    DialogSessionManager dialogSessionManager;
    dialogSessionManager.OnlySetDialogCallerInfo(abilityRequest, TEST_USER_ID, type, TEST_DIALOG_SESSION_ID, false);
    auto info = dialogSessionManager.GetDialogCallerInfo(TEST_DIALOG_SESSION_ID);
    EXPECT_NE(info, nullptr);
    EXPECT_EQ(info->sessionToken, sessionToken);
    GTEST_LOG_(INFO) << "OnlySetDialogCallerInfo_0200 end";
}

/**
 * @tc.name: CreateImplicitSelectorModalDialog_0001
 * @tc.desc: No APP_LAUNCH_TRUSTLIST
 * @tc.type: FUNC
 */
HWTEST_F(DialogSessionManagerTest, CreateImplicitSelectorModalDialog_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateImplicitSelectorModalDialog_0001 start";
    std::string dialogSessionId = "1000001";
    AbilityRequest abilityRequest;
    abilityRequest.want.SetParam(TEST_DIALOG_SESSION_ID, dialogSessionId);
    AAFwk::Want want;
    int32_t userId = 0;
    std::vector<DialogAppInfo> dialogAppInfos;
    abilityRequest.want.SetParam("deviceType", 1);
    abilityRequest.want.SetParam("userId", userId);
    std::string mockAction = "mockAction";
    abilityRequest.want.SetParam("action", mockAction);
    std::string mockType = "mockType";
    abilityRequest.want.SetParam("wantType", mockType);
    std::string mockUri = "mockUri";
    abilityRequest.want.SetParam("uri", mockUri);
    std::vector<std::string> mockEntities = {"mockEntities"};
    abilityRequest.want.SetParam("entities", mockEntities);
    abilityRequest.want.SetParam("appselector.selectorType", static_cast<int>(SelectorType::IMPLICIT_START_SELECTOR));
    abilityRequest.want.SetParam("showCaller", false);
    abilityRequest.want.SetParam(SHOW_DEFAULT_PICKER_FLAG, false);

    std::shared_ptr<DialogCallerInfo> dialogCallerInfo = std::make_shared<DialogCallerInfo>();
    DialogSessionManager dialogSessionManager;
    dialogSessionManager.dialogCallerInfoMap_[dialogSessionId] = dialogCallerInfo;
    auto ret = dialogSessionManager.CreateImplicitSelectorModalDialog(abilityRequest,
        want, userId, dialogAppInfos, false);

    EXPECT_NE(ret, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "CreateImplicitSelectorModalDialog_0001 end";
}

/**
 * @tc.name: CreateImplicitSelectorModalDialog_0002
 * @tc.desc: Has APP_LAUNCH_TRUSTLIST
 * @tc.type: FUNC
 */
HWTEST_F(DialogSessionManagerTest, CreateImplicitSelectorModalDialog_0002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateImplicitSelectorModalDialog_0002 start";
    std::string dialogSessionId = "1000001";
    AbilityRequest abilityRequest;
    abilityRequest.want.SetParam(TEST_DIALOG_SESSION_ID, dialogSessionId);
    AAFwk::Want want;
    int32_t userId = 0;
    std::vector<DialogAppInfo> dialogAppInfos;
    abilityRequest.want.SetParam("deviceType", 1);
    abilityRequest.want.SetParam("userId", userId);
    std::string mockAction = "mockAction";
    abilityRequest.want.SetParam("action", mockAction);
    std::string mockType = "mockType";
    abilityRequest.want.SetParam("wantType", mockType);
    std::string mockUri = "mockUri";
    abilityRequest.want.SetParam("uri", mockUri);
    std::vector<std::string> mockEntities = {"mockEntities"};
    abilityRequest.want.SetParam("entities", mockEntities);
    abilityRequest.want.SetParam("appselector.selectorType", static_cast<int>(SelectorType::IMPLICIT_START_SELECTOR));
    abilityRequest.want.SetParam("showCaller", false);
    abilityRequest.want.SetParam(SHOW_DEFAULT_PICKER_FLAG, false);
    std::vector<std::string> mockTrustlist = {"abc", "bca", "cab"};
    abilityRequest.want.SetParam(APP_LAUNCH_TRUSTLIST, mockTrustlist);

    std::shared_ptr<DialogCallerInfo> dialogCallerInfo = std::make_shared<DialogCallerInfo>();
    DialogSessionManager dialogSessionManager;
    dialogSessionManager.dialogCallerInfoMap_[dialogSessionId] = dialogCallerInfo;
    auto ret = dialogSessionManager.CreateImplicitSelectorModalDialog(abilityRequest,
        want, userId, dialogAppInfos, false);

    EXPECT_NE(ret, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "CreateImplicitSelectorModalDialog_0002 end";
}

/**
 * @tc.name: NotifyAbilityRequestFailure_0100
 * @tc.desc: Test NotifyAbilityRequestFailure
 * @tc.type: FUNC
 */
HWTEST_F(DialogSessionManagerTest, NotifyAbilityRequestFailure_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyAbilityRequestFailure_0100 start";
    DialogSessionManager dialogSessionManager;
    Want want;
    EXPECT_EQ(dialogSessionManager.GetDialogCallerInfo(TEST_DIALOG_SESSION_ID), nullptr);

    dialogSessionManager.NotifyAbilityRequestFailure(TEST_DIALOG_SESSION_ID, want);

    GTEST_LOG_(INFO) << "NotifyAbilityRequestFailure_0100 end";
}

/**
 * @tc.name: NotifyAbilityRequestFailure_0200
 * @tc.desc: Test NotifyAbilityRequestFailure
 * @tc.type: FUNC
 */
HWTEST_F(DialogSessionManagerTest, NotifyAbilityRequestFailure_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyAbilityRequestFailure_0200 start";
    DialogSessionManager dialogSessionManager;
    Want want;
    sptr<DialogSessionInfo> dilogSessionInfo = nullptr;
    std::shared_ptr<DialogCallerInfo> dialogCallerInfo = std::make_shared<DialogCallerInfo>();
    EXPECT_NE(dialogCallerInfo, nullptr);

    dialogSessionManager.SetDialogSessionInfo(TEST_DIALOG_SESSION_ID, dilogSessionInfo, dialogCallerInfo);
    auto callerInfo = dialogSessionManager.GetDialogCallerInfo(TEST_DIALOG_SESSION_ID);
    EXPECT_NE(dialogSessionManager.GetDialogCallerInfo(TEST_DIALOG_SESSION_ID), nullptr);

    auto requestId = callerInfo->targetWant.GetStringParam(KEY_REQUEST_ID);
    EXPECT_EQ(requestId.empty(), true);

    dialogSessionManager.NotifyAbilityRequestFailure(TEST_DIALOG_SESSION_ID, want);

    GTEST_LOG_(INFO) << "NotifyAbilityRequestFailure_0200 end";
}

/**
 * @tc.name: NotifyAbilityRequestFailure_0300
 * @tc.desc: Test NotifyAbilityRequestFailure
 * @tc.type: FUNC
 */
HWTEST_F(DialogSessionManagerTest, NotifyAbilityRequestFailure_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyAbilityRequestFailure_0300 start";
    DialogSessionManager dialogSessionManager;
    Want want;
    want.SetParam(KEY_REQUEST_ID, std::string("1234567890"));
    sptr<DialogSessionInfo> dilogSessionInfo = nullptr;
    std::shared_ptr<DialogCallerInfo> dialogCallerInfo = std::make_shared<DialogCallerInfo>();
    EXPECT_NE(dialogCallerInfo, nullptr);
    dialogCallerInfo->targetWant = want;

    dialogSessionManager.SetDialogSessionInfo(TEST_DIALOG_SESSION_ID, dilogSessionInfo, dialogCallerInfo);
    auto callerInfo = dialogSessionManager.GetDialogCallerInfo(TEST_DIALOG_SESSION_ID);
    EXPECT_NE(dialogSessionManager.GetDialogCallerInfo(TEST_DIALOG_SESSION_ID), nullptr);

    auto requestId = callerInfo->targetWant.GetStringParam(KEY_REQUEST_ID);
    EXPECT_EQ(requestId.empty(), false);
    EXPECT_EQ(callerInfo->callerToken, nullptr);

    dialogSessionManager.NotifyAbilityRequestFailure(TEST_DIALOG_SESSION_ID, want);

    GTEST_LOG_(INFO) << "NotifyAbilityRequestFailure_0300 end";
}

/**
 * @tc.name: NotifyAbilityRequestFailure_0400
 * @tc.desc: Test NotifyAbilityRequestFailure
 * @tc.type: FUNC
 */
HWTEST_F(DialogSessionManagerTest, NotifyAbilityRequestFailure_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyAbilityRequestFailure_0400 start";
    DialogSessionManager dialogSessionManager;
    Want want;
    want.SetParam(KEY_REQUEST_ID, std::string("1234567890"));
    sptr<DialogSessionInfo> dilogSessionInfo = nullptr;
    std::shared_ptr<DialogCallerInfo> dialogCallerInfo = std::make_shared<DialogCallerInfo>();
    EXPECT_NE(dialogCallerInfo, nullptr);
    dialogCallerInfo->targetWant = want;

    AbilityRequest abilityRequest;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    dialogCallerInfo->callerToken = MockToken(AbilityType::PAGE);
    EXPECT_NE(dialogCallerInfo->callerToken, nullptr);
    EXPECT_EQ(Token::GetAbilityRecordByToken(dialogCallerInfo->callerToken), nullptr);

    dialogSessionManager.SetDialogSessionInfo(TEST_DIALOG_SESSION_ID, dilogSessionInfo, dialogCallerInfo);
    auto callerInfo = dialogSessionManager.GetDialogCallerInfo(TEST_DIALOG_SESSION_ID);
    EXPECT_NE(dialogSessionManager.GetDialogCallerInfo(TEST_DIALOG_SESSION_ID), nullptr);

    auto requestId = callerInfo->targetWant.GetStringParam(KEY_REQUEST_ID);
    EXPECT_EQ(requestId.empty(), false);
    EXPECT_NE(callerInfo->callerToken, nullptr);
    EXPECT_EQ(Token::GetAbilityRecordByToken(callerInfo->callerToken), nullptr);

    dialogSessionManager.NotifyAbilityRequestFailure(TEST_DIALOG_SESSION_ID, want);

    GTEST_LOG_(INFO) << "NotifyAbilityRequestFailure_0400 end";
}

/**
 * @tc.name: NotifyAbilityRequestFailure_0500
 * @tc.desc: Test NotifyAbilityRequestFailure
 * @tc.type: FUNC
 */
HWTEST_F(DialogSessionManagerTest, NotifyAbilityRequestFailure_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyAbilityRequestFailure_0500 start";
    DialogSessionManager dialogSessionManager;
    Want want;
    want.SetParam(KEY_REQUEST_ID, std::string("1234567890"));
    sptr<DialogSessionInfo> dilogSessionInfo = nullptr;
    std::shared_ptr<DialogCallerInfo> dialogCallerInfo = std::make_shared<DialogCallerInfo>();
    EXPECT_NE(dialogCallerInfo, nullptr);
    dialogCallerInfo->targetWant = want;

    AbilityRequest abilityRequest;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    dialogCallerInfo->callerToken = new (std::nothrow) Token(abilityRecord);
    EXPECT_NE(dialogCallerInfo->callerToken, nullptr);
    EXPECT_NE(Token::GetAbilityRecordByToken(dialogCallerInfo->callerToken), nullptr);

    dialogSessionManager.SetDialogSessionInfo(TEST_DIALOG_SESSION_ID, dilogSessionInfo, dialogCallerInfo);
    auto callerInfo = dialogSessionManager.GetDialogCallerInfo(TEST_DIALOG_SESSION_ID);
    EXPECT_NE(dialogSessionManager.GetDialogCallerInfo(TEST_DIALOG_SESSION_ID), nullptr);

    auto requestId = callerInfo->targetWant.GetStringParam(KEY_REQUEST_ID);
    EXPECT_EQ(requestId.empty(), false);
    EXPECT_NE(callerInfo->callerToken, nullptr);
    EXPECT_NE(Token::GetAbilityRecordByToken(callerInfo->callerToken), nullptr);

    dialogSessionManager.NotifyAbilityRequestFailure(TEST_DIALOG_SESSION_ID, want);

    GTEST_LOG_(INFO) << "NotifyAbilityRequestFailure_0500 end";
}

/**
 * @tc.name: NotifyAbilityRequestFailure_0600
 * @tc.desc: Test NotifyAbilityRequestFailure
 * @tc.type: FUNC
 */
HWTEST_F(DialogSessionManagerTest, NotifyAbilityRequestFailure_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyAbilityRequestFailure_0600 start";
    DialogSessionManager dialogSessionManager;
    Want want;
    want.SetParam(KEY_REQUEST_ID, std::string("1234567890"));
    sptr<DialogSessionInfo> dilogSessionInfo = nullptr;
    std::shared_ptr<DialogCallerInfo> dialogCallerInfo = std::make_shared<DialogCallerInfo>();
    EXPECT_NE(dialogCallerInfo, nullptr);
    dialogCallerInfo->targetWant = want;

    AbilityRequest abilityRequest;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    dialogCallerInfo->callerToken = new (std::nothrow) Token(abilityRecord);
    EXPECT_NE(dialogCallerInfo->callerToken, nullptr);
    EXPECT_NE(Token::GetAbilityRecordByToken(dialogCallerInfo->callerToken), nullptr);
    dialogCallerInfo->type = SelectorType::IMPLICIT_START_SELECTOR;

    dialogSessionManager.SetDialogSessionInfo(TEST_DIALOG_SESSION_ID, dilogSessionInfo, dialogCallerInfo);
    auto callerInfo = dialogSessionManager.GetDialogCallerInfo(TEST_DIALOG_SESSION_ID);
    EXPECT_NE(dialogSessionManager.GetDialogCallerInfo(TEST_DIALOG_SESSION_ID), nullptr);

    auto requestId = callerInfo->targetWant.GetStringParam(KEY_REQUEST_ID);
    EXPECT_EQ(requestId.empty(), false);
    EXPECT_NE(callerInfo->callerToken, nullptr);
    EXPECT_NE(Token::GetAbilityRecordByToken(callerInfo->callerToken), nullptr);
    EXPECT_EQ(callerInfo->type, SelectorType::IMPLICIT_START_SELECTOR);

    dialogSessionManager.NotifyAbilityRequestFailure(TEST_DIALOG_SESSION_ID, want);

    GTEST_LOG_(INFO) << "NotifyAbilityRequestFailure_0600 end";
}

/**
 * @tc.name: NotifyAbilityRequestFailure_0700
 * @tc.desc: Test NotifyAbilityRequestFailure
 * @tc.type: FUNC
 */
HWTEST_F(DialogSessionManagerTest, NotifyAbilityRequestFailure_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyAbilityRequestFailure_0700 start";
    DialogSessionManager dialogSessionManager;
    Want want;
    want.SetParam(KEY_REQUEST_ID, std::string("1234567890"));
    sptr<DialogSessionInfo> dilogSessionInfo = nullptr;
    std::shared_ptr<DialogCallerInfo> dialogCallerInfo = std::make_shared<DialogCallerInfo>();
    EXPECT_NE(dialogCallerInfo, nullptr);
    dialogCallerInfo->targetWant = want;

    AbilityRequest abilityRequest;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    dialogCallerInfo->callerToken = new (std::nothrow) Token(abilityRecord);
    EXPECT_NE(dialogCallerInfo->callerToken, nullptr);
    EXPECT_NE(Token::GetAbilityRecordByToken(dialogCallerInfo->callerToken), nullptr);
    dialogCallerInfo->type = SelectorType::APP_CLONE_SELECTOR;

    dialogSessionManager.SetDialogSessionInfo(TEST_DIALOG_SESSION_ID, dilogSessionInfo, dialogCallerInfo);
    auto callerInfo = dialogSessionManager.GetDialogCallerInfo(TEST_DIALOG_SESSION_ID);
    EXPECT_NE(dialogSessionManager.GetDialogCallerInfo(TEST_DIALOG_SESSION_ID), nullptr);

    auto requestId = callerInfo->targetWant.GetStringParam(KEY_REQUEST_ID);
    EXPECT_EQ(requestId.empty(), false);
    EXPECT_NE(callerInfo->callerToken, nullptr);
    EXPECT_NE(Token::GetAbilityRecordByToken(callerInfo->callerToken), nullptr);
    EXPECT_EQ(callerInfo->type, SelectorType::APP_CLONE_SELECTOR);

    dialogSessionManager.NotifyAbilityRequestFailure(TEST_DIALOG_SESSION_ID, want);

    GTEST_LOG_(INFO) << "NotifyAbilityRequestFailure_0700 end";
}

/**
 * @tc.name: SetQueryERMSInfo_001
 * @tc.desc: test SetQueryERMSInfo function
 * @tc.type: FUNC
 */
HWTEST_F(DialogSessionManagerTest, SetQueryERMSInfo_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetQueryERMSInfo_001 start";
    DialogSessionManager dialogSessionManager;
    AbilityRequest abilityRequest;
    abilityRequest.isQueryERMS = true;
    dialogSessionManager.SetQueryERMSInfo(TEST_DIALOG_SESSION_ID, abilityRequest);
    bool found = dialogSessionManager.queryERMSInfoMap_.find(TEST_DIALOG_SESSION_ID) !=
        dialogSessionManager.queryERMSInfoMap_.end();
    EXPECT_TRUE(found);
    GTEST_LOG_(INFO) << "SetQueryERMSInfo_001 end";
}

/**
 * @tc.name: NotifyQueryERMSFinished_001
 * @tc.desc: test NotifyQueryERMSFinished function
 * @tc.type: FUNC
 */
HWTEST_F(DialogSessionManagerTest, NotifyQueryERMSFinished_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyQueryERMSFinished_001 start";
    DialogSessionManager dialogSessionManager;
    AbilityRequest abilityRequest;
    abilityRequest.isQueryERMS = true;
    dialogSessionManager.SetQueryERMSInfo(TEST_DIALOG_SESSION_ID, abilityRequest);
    bool result = dialogSessionManager.NotifyQueryERMSFinished(TEST_DIALOG_SESSION_ID, true);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "NotifyQueryERMSFinished_001 end";
}

/**
 * @tc.name: NotifyQueryERMSFinished_002
 * @tc.desc: test NotifyQueryERMSFinished function
 * @tc.type: FUNC
 */
HWTEST_F(DialogSessionManagerTest, NotifyQueryERMSFinished_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyQueryERMSFinished_002 start";
    DialogSessionManager dialogSessionManager;
    AbilityRequest abilityRequest;
    abilityRequest.isQueryERMS = true;
    dialogSessionManager.SetQueryERMSInfo(TEST_DIALOG_SESSION_ID, abilityRequest);
    bool result = dialogSessionManager.NotifyQueryERMSFinished("OTHERS", true);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "NotifyQueryERMSFinished_002 end";
}

/**
 * @tc.name: ClearDialogContext_001
 * @tc.desc: test ClearDialogContext function
 * @tc.type: FUNC
 */
HWTEST_F(DialogSessionManagerTest, ClearDialogContext_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ClearDialogContext_001 start";
    DialogSessionManager dialogSessionManager;
    dialogSessionManager.dialogSessionInfoMap_[TEST_DIALOG_SESSION_ID] = nullptr;
    dialogSessionManager.dialogCallerInfoMap_[TEST_DIALOG_SESSION_ID] = nullptr;
    dialogSessionManager.startupSessionInfoMap_[TEST_DIALOG_SESSION_ID] = nullptr;
    dialogSessionManager.ClearDialogContext(TEST_DIALOG_SESSION_ID);
    bool found1 = dialogSessionManager.dialogSessionInfoMap_.find(TEST_DIALOG_SESSION_ID) ==
        dialogSessionManager.dialogSessionInfoMap_.end();
    bool found2 = dialogSessionManager.dialogCallerInfoMap_.find(TEST_DIALOG_SESSION_ID) ==
        dialogSessionManager.dialogCallerInfoMap_.end();
    bool found3 = dialogSessionManager.startupSessionInfoMap_.find(TEST_DIALOG_SESSION_ID) ==
        dialogSessionManager.startupSessionInfoMap_.end();
    EXPECT_TRUE(found1 && found2 && found3);
    GTEST_LOG_(INFO) << "ClearDialogContext_001 end";
}

/**
 * @tc.name: GenerateSelectorTargetAbilityInfos_001
 * @tc.desc: test GenerateSelectorTargetAbilityInfos function
 * @tc.type: FUNC
 */
HWTEST_F(DialogSessionManagerTest, GenerateSelectorTargetAbilityInfos_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenerateSelectorTargetAbilityInfos_001 start";
    DialogSessionManager dialogSessionManager;
    std::vector<DialogAppInfo> dialogAppInfos;
    std::vector<DialogAbilityInfo> targetAbilityInfos;
    DialogAppInfo dialogAppInfo;
    dialogAppInfos.push_back(dialogAppInfo);
    dialogSessionManager.GenerateSelectorTargetAbilityInfos(dialogAppInfos, targetAbilityInfos);
    bool insertSuccess = targetAbilityInfos.size() > 0;
    EXPECT_TRUE(insertSuccess);
    GTEST_LOG_(INFO) << "GenerateSelectorTargetAbilityInfos_001 end";
}

/**
 * @tc.name: CreateJumpModalDialog_001
 * @tc.desc: test CreateJumpModalDialog function
 * @tc.type: FUNC
 */
HWTEST_F(DialogSessionManagerTest, CreateJumpModalDialog_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateJumpModalDialog_001 start";
    DialogSessionManager dialogSessionManager;
    AbilityRequest abilityRequest;
    Want replaceWant;
    int result = dialogSessionManager.CreateJumpModalDialog(abilityRequest, 0, replaceWant);
    EXPECT_NE(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "CreateJumpModalDialog_001 end";
}

/**
 * @tc.name: CreateModalDialogCommon_001
 * @tc.desc: test CreateModalDialogCommon function
 * @tc.type: FUNC
 */
HWTEST_F(DialogSessionManagerTest, CreateModalDialogCommon_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateModalDialogCommon_001 start";
    DialogSessionManager dialogSessionManager;
    Want replaceWant;
    sptr<IRemoteObject> callerToken = nullptr;
    int result = dialogSessionManager.CreateModalDialogCommon(replaceWant, callerToken, TEST_DIALOG_SESSION_ID);
    EXPECT_EQ(result, INNER_ERR);
    GTEST_LOG_(INFO) << "CreateModalDialogCommon_001 end";
}

/**
 * @tc.name: CreateModalDialogCommon_002
 * @tc.desc: test CreateModalDialogCommon function
 * @tc.type: FUNC
 */
HWTEST_F(DialogSessionManagerTest, CreateModalDialogCommon_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateModalDialogCommon_002 start";
    DialogSessionManager dialogSessionManager;
    Want replaceWant;
    sptr<IRemoteObject> callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    int result = dialogSessionManager.CreateModalDialogCommon(replaceWant, callerToken, TEST_DIALOG_SESSION_ID);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "CreateModalDialogCommon_002 end";
}

/**
 * @tc.name: HandleErmsResult_001
 * @tc.desc: test HandleErmsResult function
 * @tc.type: FUNC
 */
HWTEST_F(DialogSessionManagerTest, HandleErmsResult_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleErmsResult_001 start";
    DialogSessionManager dialogSessionManager;
    AbilityRequest abilityRequest;
    Want replaceWant;
    int result = dialogSessionManager.HandleErmsResult(abilityRequest, 0, replaceWant);
    EXPECT_NE(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "HandleErmsResult_001 end";
}

/**
 * @tc.name: HandleErmsResultBySCB_001
 * @tc.desc: test HandleErmsResultBySCB function
 * @tc.type: FUNC
 */
HWTEST_F(DialogSessionManagerTest, HandleErmsResultBySCB_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleErmsResultBySCB_001 start";
    DialogSessionManager dialogSessionManager;
    AbilityRequest abilityRequest;
    Want replaceWant;
    int result = dialogSessionManager.HandleErmsResultBySCB(abilityRequest, replaceWant);
    EXPECT_NE(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "HandleErmsResultBySCB_001 end";
}
}  // namespace AAFwk
}  // namespace OHOS
