/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ability_manager_proxy.h"
#include "ability_manager_errors.h"
#include "ability_manager_stub_mock.h"
#include "mock_ability_connect_callback.h"
#include "mock_ability_token.h"
#include "ability_scheduler_mock.h"
#include "ability_record.h"
#include "app_debug_listener_stub_mock.h"
#include "ability_scheduler.h"
#include "hilog_tag_wrapper.h"
#include "mission_snapshot.h"
#include "want_sender_info.h"
#include "skill_execute_result.h"

using namespace testing::ext;
using namespace testing;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {

class MockIQueryERMSObserver : public AbilityRuntime::IQueryERMSObserver {
public:
    void OnQueryFinished(const std::string &appId, const std::string &startTime,
        const AtomicServiceStartupRule &rule, int resultCode)
    {}
    sptr<IRemoteObject> AsObject()
    {
        return token_;
    }
    sptr<IRemoteObject> token_ = nullptr;
};

class MockIHiddenStartObserver : public IHiddenStartObserver {
public:
    bool IsHiddenStart(int32_t pid)
    {
        return false;
    }
    sptr<IRemoteObject> AsObject()
    {
        return token_;
    }
    sptr<IRemoteObject> token_ = nullptr;
};

class AbilityManagerProxySixthTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<AbilityManagerProxy> proxy_{ nullptr };
    sptr<AbilityManagerStubMock> mock_{ nullptr };
};

void AbilityManagerProxySixthTest::SetUpTestCase(void)
{}
void AbilityManagerProxySixthTest::TearDownTestCase(void)
{}
void AbilityManagerProxySixthTest::TearDown()
{}

void AbilityManagerProxySixthTest::SetUp()
{
    mock_ = new AbilityManagerStubMock();
    proxy_ = std::make_shared<AbilityManagerProxy>(mock_);
}

/**
 * @tc.name: AbilityManagerProxy_GetUIExtensionSessionInfo_0100
 * @tc.desc: GetUIExtensionSessionInfo
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_GetUIExtensionSessionInfo_0100, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    UIExtensionSessionInfo uiExtensionSessionInfo;
    int32_t userId = 100;
    auto res = proxy_->GetUIExtensionSessionInfo(token, uiExtensionSessionInfo, userId);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    token = sptr<MockAbilityToken>::MakeSptr();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    res = proxy_->GetUIExtensionSessionInfo(token, uiExtensionSessionInfo, userId);
    EXPECT_EQ(res, -1);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    res = proxy_->GetUIExtensionSessionInfo(token, uiExtensionSessionInfo, userId);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_UI_EXTENSION_SESSION_INFO), mock_->code_);
    EXPECT_EQ(res, INNER_ERR);
}

/**
 * @tc.name: AbilityManagerProxy_OpenAtomicService_0100
 * @tc.desc: OpenAtomicService
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_OpenAtomicService_0100, TestSize.Level1)
{
    Want want;
    StartOptions options;
    sptr<IRemoteObject> callerToken = nullptr;
    int32_t requestCode = 1;
    int32_t userId = 100;

    callerToken = sptr<MockAbilityToken>::MakeSptr();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    auto res = proxy_->OpenAtomicService(want, options, callerToken, requestCode, userId);
    EXPECT_EQ(res, -1);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    res = proxy_->OpenAtomicService(want, options, callerToken, requestCode, userId);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::OPEN_ATOMIC_SERVICE), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxy_IsEmbeddedOpenAllowed_0100
 * @tc.desc: IsEmbeddedOpenAllowed
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_IsEmbeddedOpenAllowed_0100, TestSize.Level1)
{
    sptr<IRemoteObject> callerToken = nullptr;
    std::string appId;
    auto res = proxy_->IsEmbeddedOpenAllowed(callerToken, appId);
    EXPECT_EQ(res, false);

    callerToken = sptr<MockAbilityToken>::MakeSptr();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    res = proxy_->IsEmbeddedOpenAllowed(callerToken, appId);
    EXPECT_EQ(res, false);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    res = proxy_->IsEmbeddedOpenAllowed(callerToken, appId);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::IS_EMBEDDED_OPEN_ALLOWED), mock_->code_);
    EXPECT_EQ(res, false);
}

/**
 * @tc.name: AbilityManagerProxy_equestAssertFaultDialog_0100
 * @tc.desc: RequestAssertFaultDialog
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_RequestAssertFaultDialog_0100, TestSize.Level1)
{
    sptr<IRemoteObject> callback = nullptr;
    AAFwk::WantParams wantParams;
    auto res = proxy_->RequestAssertFaultDialog(callback, wantParams);
    EXPECT_EQ(res, INNER_ERR);

    callback = sptr<MockAbilityToken>::MakeSptr();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    res = proxy_->RequestAssertFaultDialog(callback, wantParams);
    EXPECT_EQ(res, -1);
    
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    res = proxy_->RequestAssertFaultDialog(callback, wantParams);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::REQUEST_ASSERT_FAULT_DIALOG), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxy_NotifyDebugAssertResult_0100
 * @tc.desc: NotifyDebugAssertResult
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_NotifyDebugAssertResult_0100, TestSize.Level1)
{
    uint64_t assertFaultSessionId = 0;
    AAFwk::UserStatus userStatus = ASSERT_TERMINATE;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    auto res = proxy_->NotifyDebugAssertResult(assertFaultSessionId, userStatus);
    EXPECT_EQ(res, -1);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    res = proxy_->NotifyDebugAssertResult(assertFaultSessionId, userStatus);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::NOTIFY_DEBUG_ASSERT_RESULT), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxy_StartShortcut_0100
 * @tc.desc: StartShortcut
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_StartShortcut_0100, TestSize.Level1)
{
    Want want;
    StartOptions startOptions;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    auto res = proxy_->StartShortcut(want, startOptions);
    EXPECT_EQ(res, -1);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    res = proxy_->StartShortcut(want, startOptions);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_SHORTCUT), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxy_GetAbilityStateByPersistentId_0100
 * @tc.desc: GetAbilityStateByPersistentId
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_GetAbilityStateByPersistentId_0100, TestSize.Level1)
{
    int32_t persistentId = 0;
    bool state = false;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    auto res = proxy_->GetAbilityStateByPersistentId(persistentId, state);
    EXPECT_EQ(res, -1);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    res = proxy_->GetAbilityStateByPersistentId(persistentId, state);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_ABILITY_STATE_BY_PERSISTENT_ID), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxy_TransferAbilityResultForExtension_0100
 * @tc.desc: TransferAbilityResultForExtension
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_TransferAbilityResultForExtension_0100, TestSize.Level1)
{
    sptr<IRemoteObject> callerToken = nullptr;
    int32_t resultCode = 0;
    Want want;
    auto res = proxy_->TransferAbilityResultForExtension(callerToken, resultCode, want);
    EXPECT_EQ(res, INNER_ERR);

    callerToken = sptr<MockAbilityToken>::MakeSptr();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    res = proxy_->TransferAbilityResultForExtension(callerToken, resultCode, want);
    EXPECT_EQ(res, -1);
    
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    res = proxy_->TransferAbilityResultForExtension(callerToken, resultCode, want);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::TRANSFER_ABILITY_RESULT), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxy_PreStartMission_0100
 * @tc.desc: PreStartMission
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_PreStartMission_0100, TestSize.Level1)
{
    std::string bundleName;
    std::string moduleName;
    std::string abilityName;
    std::string startTime;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    auto res = proxy_->PreStartMission(bundleName, moduleName, abilityName, startTime);
    EXPECT_EQ(res, -1);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    res = proxy_->PreStartMission(bundleName, moduleName, abilityName, startTime);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::PRE_START_MISSION), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxy_CleanUIAbilityBySCB_0100
 * @tc.desc: CleanUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_CleanUIAbilityBySCB_0100, TestSize.Level1)
{
    sptr<SessionInfo> sessionInfo = nullptr;
    bool isUserRequestedExit = false;
    uint32_t sceneFlag = 1;

    sessionInfo = new SessionInfo();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    auto res = proxy_->CleanUIAbilityBySCB(sessionInfo, isUserRequestedExit, sceneFlag);
    EXPECT_EQ(res, -1);
    
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    res = proxy_->CleanUIAbilityBySCB(sessionInfo, isUserRequestedExit, sceneFlag);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::CLEAN_UI_ABILITY_BY_SCB), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxy_OpenLink_0100
 * @tc.desc: OpenLink
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_OpenLink_0100, TestSize.Level1)
{
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    int32_t userId = 100;
    int requestCode = 1;
    auto res = proxy_->OpenLink(want, callerToken, userId, requestCode);
    EXPECT_EQ(res, INNER_ERR);

    callerToken = sptr<MockAbilityToken>::MakeSptr();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    res = proxy_->OpenLink(want, callerToken, userId, requestCode);
    EXPECT_EQ(res, -1);
    
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    res = proxy_->OpenLink(want, callerToken, userId, requestCode);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::OPEN_LINK), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxy_BlockAllAppStart_0100
 * @tc.desc: BlockAllAppStart
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_BlockAllAppStart_0100, TestSize.Level1)
{
    bool flag = false;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    auto res = proxy_->BlockAllAppStart(flag);
    EXPECT_EQ(res, -1);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    res = proxy_->BlockAllAppStart(flag);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::BLOCK_ALL_APP_START), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxy_UpdateAssociateConfigList_0100
 * @tc.desc: UpdateAssociateConfigList
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_UpdateAssociateConfigList_0100, TestSize.Level1)
{
    std::map<std::string, std::list<std::string>> configs;
    std::list<std::string> exportConfigs;
    int32_t flag = 0;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    auto res = proxy_->UpdateAssociateConfigList(configs, exportConfigs, flag);
    EXPECT_EQ(res, -1);
    
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    res = proxy_->UpdateAssociateConfigList(configs, exportConfigs, flag);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::UPDATE_ASSOCIATE_CONFIG_LIST), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxy_SetApplicationKeepAlive_0100
 * @tc.desc: SetApplicationKeepAlive
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_SetApplicationKeepAlive_0100, TestSize.Level1)
{
    std::string bundleName;
    int32_t userId = 100;
    bool flag = false;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    auto res = proxy_->SetApplicationKeepAlive(bundleName, userId, flag);
    EXPECT_EQ(res, -1);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    res = proxy_->SetApplicationKeepAlive(bundleName, userId, flag);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_APPLICATION_KEEP_ALLIVE), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxy_QueryKeepAliveApplications_0100
 * @tc.desc: QueryKeepAliveApplications
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_QueryKeepAliveApplications_0100, TestSize.Level1)
{
    int32_t appType = 0;
    int32_t userId = 100;
    std::vector<KeepAliveInfo> list;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    auto res = proxy_->QueryKeepAliveApplications(appType, userId, list);
    EXPECT_EQ(res, -1);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    res = proxy_->QueryKeepAliveApplications(appType, userId, list);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_APPLICATIONS_KEEP_ALIVE), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxy_SetApplicationKeepAliveByEDM_0100
 * @tc.desc: SetApplicationKeepAliveByEDM
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_SetApplicationKeepAliveByEDM_0100, TestSize.Level1)
{
    std::string bundleName;
    int32_t userId = 100;
    bool flag = false;
    bool allowUserToCancel = false;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    auto res = proxy_->SetApplicationKeepAliveByEDM(bundleName, userId, flag, allowUserToCancel);
    EXPECT_EQ(res, -1);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    res = proxy_->SetApplicationKeepAliveByEDM(bundleName, userId, flag, allowUserToCancel);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_APPLICATION_KEEP_ALLIVE_BY_EDM), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxy_QueryKeepAliveApplicationsByEDM_0100
 * @tc.desc: QueryKeepAliveApplicationsByEDM
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_QueryKeepAliveApplicationsByEDM_0100, TestSize.Level1)
{
    int32_t appType = 0;
    int32_t userId = 100;
    std::vector<KeepAliveInfo> list;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    auto res = proxy_->QueryKeepAliveApplicationsByEDM(appType, userId, list);
    EXPECT_EQ(res, -1);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    res = proxy_->QueryKeepAliveApplicationsByEDM(appType, userId, list);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_APPLICATIONS_KEEP_ALIVE_BY_EDM), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxy_GetAllIntentExemptionInfo_0100
 * @tc.desc: GetAllIntentExemptionInfo
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_GetAllIntentExemptionInfo_0100, TestSize.Level1)
{
    std::vector<AppExecFwk::IntentExemptionInfo> info;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    auto res = proxy_->GetAllIntentExemptionInfo(info);
    EXPECT_EQ(res, -1);
    
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    res = proxy_->GetAllIntentExemptionInfo(info);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxy_AddQueryERMSObserver_0100
 * @tc.desc: AddQueryERMSObserver
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_AddQueryERMSObserver_0100, TestSize.Level1)
{
    sptr<IRemoteObject> callerToken = nullptr;
    sptr<MockIQueryERMSObserver> observer = nullptr;
    auto res = proxy_->AddQueryERMSObserver(callerToken, observer);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    callerToken = sptr<MockAbilityToken>::MakeSptr();
    res = proxy_->AddQueryERMSObserver(callerToken, observer);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    observer = new MockIQueryERMSObserver();
    res = proxy_->AddQueryERMSObserver(callerToken, observer);
    EXPECT_EQ(res, INNER_ERR);

    observer->token_ = sptr<MockAbilityToken>::MakeSptr();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    res = proxy_->AddQueryERMSObserver(callerToken, observer);
    EXPECT_EQ(res, -1);
    
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    res = proxy_->AddQueryERMSObserver(callerToken, observer);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::ADD_QUERY_ERMS_OBSERVER), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxy_QueryAtomicServiceStartupRule_0100
 * @tc.desc: QueryAtomicServiceStartupRule
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_QueryAtomicServiceStartupRule_0100, TestSize.Level1)
{
    sptr<IRemoteObject> callerToken = nullptr;
    std::string appId;
    std::string startTime;
    AtomicServiceStartupRule rule;
    auto res = proxy_->QueryAtomicServiceStartupRule(callerToken, appId, startTime, rule);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    callerToken = sptr<MockAbilityToken>::MakeSptr();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    res = proxy_->QueryAtomicServiceStartupRule(callerToken, appId, startTime, rule);
    EXPECT_EQ(res, -1);
    
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    res = proxy_->QueryAtomicServiceStartupRule(callerToken, appId, startTime, rule);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::QUERY_ATOMIC_SERVICE_STARTUP_RULE), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxy_KillProcessForPermissionUpdate_0100
 * @tc.desc: KillProcessForPermissionUpdate
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_KillProcessForPermissionUpdate_0100, TestSize.Level1)
{
    uint32_t accessTokenId = 0;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    auto res = proxy_->KillProcessForPermissionUpdate(accessTokenId);
    EXPECT_EQ(res, IPC_PROXY_ERR);
        
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    res = proxy_->KillProcessForPermissionUpdate(accessTokenId);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::KILL_PROCESS_FOR_PERMISSION_UPDATE), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxy_RegisterHiddenStartObserver_0100
 * @tc.desc: RegisterHiddenStartObserver
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_RegisterHiddenStartObserver_0100, TestSize.Level1)
{
    sptr<MockIHiddenStartObserver> observer = nullptr;
    auto res = proxy_->RegisterHiddenStartObserver(observer);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    observer = new MockIHiddenStartObserver();
    res = proxy_->RegisterHiddenStartObserver(observer);
    EXPECT_EQ(res, ERR_FLATTEN_OBJECT);

    observer->token_ = sptr<MockAbilityToken>::MakeSptr();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    res = proxy_->RegisterHiddenStartObserver(observer);
    EXPECT_EQ(res, -1);
    
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    res = proxy_->RegisterHiddenStartObserver(observer);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_HIDDEN_START_OBSERVER), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxy_UnregisterHiddenStartObserver_0100
 * @tc.desc: UnregisterHiddenStartObserver
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_UnregisterHiddenStartObserver_0100, TestSize.Level1)
{
    sptr<MockIHiddenStartObserver> observer = nullptr;
    auto res = proxy_->UnregisterHiddenStartObserver(observer);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
    
    observer = new MockIHiddenStartObserver();
    res = proxy_->UnregisterHiddenStartObserver(observer);
    EXPECT_EQ(res, ERR_FLATTEN_OBJECT);

    observer->token_ = sptr<MockAbilityToken>::MakeSptr();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    res = proxy_->UnregisterHiddenStartObserver(observer);
    EXPECT_EQ(res, -1);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    res = proxy_->UnregisterHiddenStartObserver(observer);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::UNREGISTER_HIDDEN_START_OBSERVER), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxy_QueryPreLoadUIExtensionRecord_0100
 * @tc.desc: QueryPreLoadUIExtensionRecord
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_QueryPreLoadUIExtensionRecord_0100, TestSize.Level1)
{
    AppExecFwk::ElementName element;
    std::string moduleName;
    int32_t hostPid = 0;
    int32_t recordNum = 0;
    int32_t userId = 100;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    auto res = proxy_->QueryPreLoadUIExtensionRecord(element, moduleName, hostPid, recordNum, userId);
    EXPECT_EQ(res, -1);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    res = proxy_->QueryPreLoadUIExtensionRecord(element, moduleName, hostPid, recordNum, userId);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::QUERY_PRELOAD_UIEXTENSION_RECORD), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxy_GetParcelableInfos_0100
 * @tc.desc: GetParcelableInfos
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_GetParcelableInfos_0100, TestSize.Level1)
{
    MessageParcel reply;
    std::vector<AbilityRunningInfo> parcelableInfos;
    auto res = proxy_->GetParcelableInfos<AbilityRunningInfo>(reply, parcelableInfos);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxy_WriteInterfaceToken_0100
 * @tc.desc: WriteInterfaceToken
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_WriteInterfaceToken_0100, TestSize.Level1)
{
    MessageParcel data;
    auto res = proxy_->WriteInterfaceToken(data);
    EXPECT_EQ(res, true);
}

/**
 * @tc.name: AbilityManagerProxy_SendRequest_0100
 * @tc.desc: SendRequest
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_SendRequest_0100, TestSize.Level1)
{
    auto proxy = std::make_shared<AbilityManagerProxy>(nullptr);
    AbilityManagerInterfaceCode code = AbilityManagerInterfaceCode::TERMINATE_ABILITY;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    auto res = proxy->SendRequest(code, data, reply, option);
    EXPECT_EQ(res, INNER_ERR);
}

/**
 * @tc.name: AbilityManagerProxy_CheckUISessionParams_0100
 * @tc.desc: CheckUISessionParams
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_CheckUISessionParams_0100, TestSize.Level1)
{
    MessageParcel data;
    sptr<IRemoteObject> callerToken = nullptr;
    sptr<SessionInfo> sessionInfo = nullptr;
    int32_t userId = 100;
    int requestCode = 0;
    auto res = proxy_->CheckUISessionParams(data, callerToken, sessionInfo, userId, requestCode);
    EXPECT_EQ(res, NO_ERROR);
    
    callerToken = sptr<MockAbilityToken>::MakeSptr();
    res = proxy_->CheckUISessionParams(data, callerToken, sessionInfo, userId, requestCode);
    EXPECT_EQ(res, NO_ERROR);
    
    sessionInfo = new SessionInfo();
    res = proxy_->CheckUISessionParams(data, callerToken, sessionInfo, userId, requestCode);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxy_UpdateAssociateConfigInner_0100
 * @tc.desc: UpdateAssociateConfigInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_UpdateAssociateConfigInner_0100, TestSize.Level1)
{
    std::map<std::string, std::list<std::string>> configs;
    MessageParcel data;
    std::list<std::string> second;
    second.push_back("test");
    configs["test"] = second;
    auto res = proxy_->UpdateAssociateConfigInner(configs, data);
    EXPECT_EQ(res, true);
}

/**
 * @tc.name: AbilityManagerProxy_ExtendMaxIpcCapacityForWant_0100
 * @tc.desc: ExtendMaxIpcCapacityForWant
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_ExtendMaxIpcCapacityForWant_0100, TestSize.Level1)
{
    Want want;
    MessageParcel data;
    want.SetParam("isCallBySCB", true);
    auto res = proxy_->ExtendMaxIpcCapacityForWant(want, data);
    EXPECT_EQ(res, false);

    want.SetParam("isCallBySCB", false);
    res = proxy_->ExtendMaxIpcCapacityForWant(want, data);
    EXPECT_EQ(res, true);
}

/**
 * @tc.name: AbilityManagerProxy_TerminateAbility_0100
 * @tc.desc: TerminateAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_TerminateAbility_0100, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    int resultCode = 0;
    Want resultWant;
    bool flag = false;

    token = sptr<MockAbilityToken>::MakeSptr();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    auto res = proxy_->TerminateAbility(token, resultCode, &resultWant, flag);
    EXPECT_EQ(res, -1);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    res = proxy_->TerminateAbility(token, resultCode, &resultWant, flag);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::TERMINATE_ABILITY), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxy_RestartApp_0100
 * @tc.desc: RestartApp
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_RestartApp_0100, TestSize.Level1)
{
    Want want;
    bool isAppRecovery = false;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    auto res = proxy_->RestartApp(want, isAppRecovery);
    EXPECT_EQ(res, -1);
}

/**
 * @tc.name: RestartSelfAtomicService_0100
 * @tc.desc: RestartSelfAtomicService
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, RestartSelfAtomicService_0100, TestSize.Level1)
{
    auto token = sptr<MockAbilityToken>::MakeSptr();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    auto res = proxy_->RestartSelfAtomicService(token);
    EXPECT_EQ(res, -1);
}

/**
 * @tc.name: AbilityManagerProxy_GetUIExtensionRootHostInfo_0100
 * @tc.desc: GetUIExtensionRootHostInfo
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_GetUIExtensionRootHostInfo_0100, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    UIExtensionHostInfo hostInfo;
    int32_t userId = 100;
    auto res = proxy_->GetUIExtensionRootHostInfo(token, hostInfo, userId);

    token = sptr<MockAbilityToken>::MakeSptr();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    res = proxy_->GetUIExtensionRootHostInfo(token, hostInfo, userId);
    EXPECT_EQ(res, -1);
}

/**
 * @tc.name: AbilityManagerProxy_SetResidentProcessEnabled_0100
 * @tc.desc: SetResidentProcessEnabled
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_SetResidentProcessEnable_0100, TestSize.Level1)
{
    std::string bundleName;
    bool enable = true;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    auto res = proxy_->SetResidentProcessEnabled(bundleName, enable);
    EXPECT_EQ(res, -1);
}

/*
 * @tc.name: AbilityManagerProxy_AbilityManagerService_0100
 * Function: TerminateMission
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_TerminateMission_001, TestSize.Level1)
{
    int32_t missionId = 1;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    auto res = proxy_->TerminateMission(missionId);
    EXPECT_EQ(res, -1);
}

/*
 * @tc.name: AbilityManagerProxy_SetAppServiceExtensionKeepAlive_001
 * Function: SetAppServiceExtensionKeepAlive
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_SetAppServiceExtensionKeepAlive_001, TestSize.Level1)
{
    std::string bundleName = "bundleName";
    bool flag = true;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    auto res = proxy_->SetAppServiceExtensionKeepAlive(bundleName, flag);
    EXPECT_EQ(res, -1);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    res = proxy_->SetAppServiceExtensionKeepAlive(bundleName, flag);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_APP_SERVICE_EXTENSION_KEEP_ALIVE), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * @tc.name: AbilityManagerProxy_QueryKeepAliveAppServiceExtensions_001
 * Function: QueryKeepAliveAppServiceExtensions
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_QueryKeepAliveAppServiceExtensions_001, TestSize.Level1)
{
    std::vector<KeepAliveInfo> list;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    auto res = proxy_->QueryKeepAliveAppServiceExtensions(list);
    EXPECT_EQ(res, -1);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    res = proxy_->QueryKeepAliveAppServiceExtensions(list);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_APP_SERVICE_EXTENSIONS_KEEP_ALIVE), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * @tc.name: AbilityManagerProxy_SetOnNewWantSkipScenarios_001
 * Function: SetOnNewWantSkipScenarios
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_SetOnNewWantSkipScenarios_001, TestSize.Level1)
{
    sptr<IRemoteObject> callerToken = nullptr;
    int32_t scenarios = 0;
    EXPECT_NE(proxy_, nullptr);
    auto res = proxy_->SetOnNewWantSkipScenarios(callerToken, scenarios);
    EXPECT_EQ(res, ERR_INVALID_CONTEXT);
}

/*
 * @tc.name: AbilityManagerProxy_SetOnNewWantSkipScenarios_002
 * Function: SetOnNewWantSkipScenarios
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_SetOnNewWantSkipScenarios_002, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    auto token = sptr<MockAbilityToken>::MakeSptr();
    auto res = proxy_->SetOnNewWantSkipScenarios(token, 0);
    EXPECT_EQ(res, -1);
}

/*
 * @tc.name: AbilityManagerProxy_SetOnNewWantSkipScenarios_003
 * Function: SetOnNewWantSkipScenarios
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_SetOnNewWantSkipScenarios_003, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    auto token = sptr<MockAbilityToken>::MakeSptr();
    auto res = proxy_->SetOnNewWantSkipScenarios(token, 0);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * @tc.name: AbilityManagerProxy_NotifyStartupExceptionBySCB_001
 * Function: NotifyStartupExceptionBySCB
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_NotifyStartupExceptionBySCB_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    int32_t requestId = 0;
    auto res = proxy_->NotifyStartupExceptionBySCB(requestId);
    EXPECT_EQ(res, -1);
}

/*
 * @tc.name: AbilityManagerProxy_NotifyStartupExceptionBySCB_002
 * Function: NotifyStartupExceptionBySCB
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_NotifyStartupExceptionBySCB_002, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    int32_t requestId = 0;
    auto res = proxy_->NotifyStartupExceptionBySCB(requestId);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * @tc.name: AbilityManagerProxy_PreloadApplication_001
 * Function: PreloadApplication
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_PreloadApplication_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    std::string bundleName = "bundleName";
    int32_t userId = -1;
    int32_t appIndex = 0;
    auto res = proxy_->PreloadApplication(bundleName, userId, appIndex);
    EXPECT_EQ(res, -1);
}

/*
 * @tc.name: AbilityManagerProxy_PreloadApplication_002
 * Function: PreloadApplication
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_PreloadApplication_002, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(0));
    std::string bundleName = "bundleName";
    int32_t userId = -1;
    int32_t appIndex = 0;
    auto res = proxy_->PreloadApplication(bundleName, userId, appIndex);
    EXPECT_EQ(res, 0);
}

/*
 * @tc.name: AbilityManagerProxy_ManualStartAutoStartupApps_001
 * Function: ManualStartAutoStartupApps
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_ManualStartAutoStartupApps_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(0));
    int32_t userId = -1;
    auto res = proxy_->ManualStartAutoStartupApps(userId);
    EXPECT_EQ(res, 0);
}

/*
 * @tc.name: AbilityManagerProxy_ManualStartAutoStartupApps_002
 * Function: ManualStartAutoStartupApps
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_ManualStartAutoStartupApps_002, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    int32_t userId = -1;
    auto res = proxy_->ManualStartAutoStartupApps(userId);
    EXPECT_EQ(res, -1);
}

/**
 * @tc.name: AbilityManagerProxy_QueryCallerTokenIdForAnco_001
 * @tc.desc: QueryCallerTokenIdForAnco with valid dialogSessionId
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_QueryCallerTokenIdForAnco_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    int32_t userId = 101;
    std::string asCallerForAncoSessionId = "520";
    uint32_t callerTokenId = 0;
    auto res = proxy_->QueryCallerTokenIdForAnco(userId, asCallerForAncoSessionId, callerTokenId);
    EXPECT_EQ(res, -1);
}

/**
 * @tc.name: AbilityManagerProxy_QueryCallerTokenIdForAnco_002
 * @tc.desc: QueryCallerTokenIdForAnco with successful request
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_QueryCallerTokenIdForAnco_002, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    int32_t userId = 101;
    std::string asCallerForAncoSessionId = "520";
    uint32_t callerTokenId = 0;
    auto res = proxy_->QueryCallerTokenIdForAnco(userId, asCallerForAncoSessionId, callerTokenId);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: AbilityManagerProxy_QueryCallerTokenIdForAnco_003
 * @tc.desc: QueryCallerTokenIdForAnco with invalid dialogSessionId
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_QueryCallerTokenIdForAnco_003, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    int32_t userId = -1;
    std::string asCallerForAncoSessionId = "-1";
    uint32_t callerTokenId = 0;
    auto res = proxy_->QueryCallerTokenIdForAnco(userId, asCallerForAncoSessionId, callerTokenId);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: AbilityManagerProxy_ExecuteInAppSkill_001
 * @tc.desc: ExecuteInAppSkill with SendRequest success
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_ExecuteInAppSkill_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    auto skillArgs = std::make_shared<AAFwk::WantParams>();
    sptr<ISkillExecuteCallback> callback = nullptr;
    auto res = proxy_->ExecuteInAppSkill("bundle", "module", "skill", "path", "func", skillArgs, callback);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: AbilityManagerProxy_ExecuteInAppSkill_002
 * @tc.desc: ExecuteInAppSkill with SendRequest failure
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_ExecuteInAppSkill_002, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    auto skillArgs = std::make_shared<AAFwk::WantParams>();
    sptr<ISkillExecuteCallback> callback = nullptr;
    auto res = proxy_->ExecuteInAppSkill("bundle", "module", "skill", "path", "func", skillArgs, callback);
    EXPECT_EQ(res, -1);
}

/**
 * @tc.name: AbilityManagerProxy_ExecuteSkillDone_001
 * @tc.desc: ExecuteSkillDone with SendRequest success
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_ExecuteSkillDone_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    sptr<IRemoteObject> token = sptr<MockAbilityToken>::MakeSptr();
    AppExecFwk::SkillExecuteResult result;
    auto res = proxy_->ExecuteSkillDone(token, "requestCode", 0, result);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: AbilityManagerProxy_ExecuteSkillDone_002
 * @tc.desc: ExecuteSkillDone with SendRequest failure
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_ExecuteSkillDone_002, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    sptr<IRemoteObject> token = sptr<MockAbilityToken>::MakeSptr();
    AppExecFwk::SkillExecuteResult result;
    auto res = proxy_->ExecuteSkillDone(token, "requestCode", 0, result);
    EXPECT_EQ(res, -1);
}

/**
 * @tc.name: AbilityManagerProxy_QuerySkillType_001
 * @tc.desc: QuerySkillType with SendRequest success
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_QuerySkillType_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    int32_t skillType = 0;
    auto res = proxy_->QuerySkillType("bundle", "module", "skill", skillType);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: AbilityManagerProxy_QuerySkillType_002
 * @tc.desc: QuerySkillType with SendRequest failure
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_QuerySkillType_002, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    int32_t skillType = 0;
    auto res = proxy_->QuerySkillType("bundle", "module", "skill", skillType);
    EXPECT_EQ(res, -1);
}

/**
 * @tc.name: AbilityManagerProxy_NotifySkillFunctionInvoked_001
 * @tc.desc: NotifySkillFunctionInvoked with SendRequest success
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_NotifySkillFunctionInvoked_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    sptr<IRemoteObject> token = sptr<MockAbilityToken>::MakeSptr();
    auto res = proxy_->NotifySkillFunctionInvoked(token, "requestCode");
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: AbilityManagerProxy_NotifySkillFunctionInvoked_002
 * @tc.desc: NotifySkillFunctionInvoked with SendRequest failure
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySixthTest, AbilityManagerProxy_NotifySkillFunctionInvoked_002, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    sptr<IRemoteObject> token = sptr<MockAbilityToken>::MakeSptr();
    auto res = proxy_->NotifySkillFunctionInvoked(token, "requestCode");
    EXPECT_EQ(res, -1);
}
} // namespace AAFwk
} // namespace OHOS
