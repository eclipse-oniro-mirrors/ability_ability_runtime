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

#include "ability_manager_collaborator_proxy.h"
#include "ability_manager_collaborator_stub_mock.h"
#include "configuration.h"
#include "uri.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
class AbilityManagerCollaboratorProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    sptr<AbilityManagerCollaboratorProxy> proxy_ {nullptr};
    sptr<AbilityManagerCollaboratorStubMock> mock_;
};

void AbilityManagerCollaboratorProxyTest::SetUpTestCase()
{}

void AbilityManagerCollaboratorProxyTest::TearDownTestCase()
{}

void AbilityManagerCollaboratorProxyTest::SetUp()
{
    mock_ = new AbilityManagerCollaboratorStubMock();
    proxy_ = new AbilityManagerCollaboratorProxy(mock_);
}

void AbilityManagerCollaboratorProxyTest::TearDown()
{}

/**
 * @tc.number: NotifyStartAbility_0100
 * @tc.desc: NotifyStartAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerCollaboratorProxyTest, NotifyStartAbility_0100, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerCollaboratorStubMock::InvokeSendRequest));
    AbilityInfo info;
    Want want;
    int32_t userId = 0;
    uint64_t accessTokenIDEx = 0;
    int32_t windowMode = 0;
    int32_t res = proxy_->NotifyStartAbility(info, userId, want, accessTokenIDEx, windowMode);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
    EXPECT_EQ(static_cast<uint32_t>(IAbilityManagerCollaborator::NOTIFY_START_ABILITY), mock_->GetCode());
}

/**
 * @tc.number: NotifyPreloadAbility_0100
 * @tc.desc: NotifyPreloadAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerCollaboratorProxyTest, NotifyPreloadAbility_0100, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerCollaboratorStubMock::InvokeSendRequest));
    std::string bundleName = "";
    int32_t res = proxy_->NotifyPreloadAbility(bundleName);
    EXPECT_EQ(res, NO_ERROR);
    EXPECT_EQ(static_cast<uint32_t>(IAbilityManagerCollaborator::NOTIFY_PRELOAD_ABILITY), mock_->GetCode());
}

/**
 * @tc.number: NotifyMissionCreated_0100
 * @tc.desc: NotifyMissionCreated
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerCollaboratorProxyTest, NotifyMissionCreated_0100, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerCollaboratorStubMock::InvokeSendRequest));
    sptr<SessionInfo> sessionInfo;
    int32_t res = proxy_->NotifyMissionCreated(sessionInfo);
    EXPECT_EQ(res, NO_ERROR);
    EXPECT_EQ(static_cast<uint32_t>(IAbilityManagerCollaborator::NOTIFY_MISSION_CREATED_BY_SCB), mock_->GetCode());
}

/**
 * @tc.number: NotifyLoadAbility_0100
 * @tc.desc: NotifyLoadAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerCollaboratorProxyTest, NotifyLoadAbility_0100, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerCollaboratorStubMock::InvokeSendRequest));
    AppExecFwk::AbilityInfo abilityInfo;
    sptr<SessionInfo> sessionInfo;
    int32_t res = proxy_->NotifyLoadAbility(abilityInfo, sessionInfo);
    EXPECT_EQ(res, NO_ERROR);
    EXPECT_EQ(static_cast<uint32_t>(IAbilityManagerCollaborator::NOTIFY_LOAD_ABILITY_BY_SCB), mock_->GetCode());
}

/**
 * @tc.number: NotifyMoveMissionToBackground_0100
 * @tc.desc: NotifyMoveMissionToBackground
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerCollaboratorProxyTest, NotifyMoveMissionToBackground_0100, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerCollaboratorStubMock::InvokeSendRequest));
    int32_t missionId = 0;
    int32_t userId = 100;
    bool isFromScreenOffBackground = false;
    int32_t res = proxy_->NotifyMoveMissionToBackground(missionId, userId, isFromScreenOffBackground);
    EXPECT_EQ(res, NO_ERROR);
    EXPECT_EQ(static_cast<uint32_t>(IAbilityManagerCollaborator::NOTIFY_MOVE_MISSION_TO_BACKGROUND), mock_->GetCode());
}

/**
 * @tc.number: NotifyMoveMissionToForeground_0100
 * @tc.desc: NotifyMoveMissionToForeground
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerCollaboratorProxyTest, NotifyMoveMissionToForeground_0100, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerCollaboratorStubMock::InvokeSendRequest));
    int32_t missionId = 0;
    int32_t userId = 100;
    int32_t res = proxy_->NotifyMoveMissionToForeground(missionId, userId);
    EXPECT_EQ(res, NO_ERROR);
    EXPECT_EQ(static_cast<uint32_t>(IAbilityManagerCollaborator::NOTIFY_MOVE_MISSION_TO_FOREGROUND), mock_->GetCode());
}

/**
 * @tc.number: NotifyTerminateMission_0100
 * @tc.desc: NotifyTerminateMission
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerCollaboratorProxyTest, NotifyTerminateMission_0100, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerCollaboratorStubMock::InvokeSendRequest));
    int32_t missionId = 0;
    int32_t userId = 100;
    int32_t res = proxy_->NotifyTerminateMission(missionId, userId);
    EXPECT_EQ(res, NO_ERROR);
    EXPECT_EQ(static_cast<uint32_t>(IAbilityManagerCollaborator::NOTIFY_TERMINATE_MISSION), mock_->GetCode());
}

/**
 * @tc.number: NotifyClearMission_0100
 * @tc.desc: NotifyClearMission
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerCollaboratorProxyTest, NotifyClearMission_0100, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerCollaboratorStubMock::InvokeSendRequest));
    int32_t missionId = 0;
    int32_t userId = 100;
    int32_t res = proxy_->NotifyClearMission(missionId, userId);
    EXPECT_EQ(res, NO_ERROR);
    EXPECT_EQ(static_cast<uint32_t>(IAbilityManagerCollaborator::NOTIFY_CLEAR_MISSION), mock_->GetCode());
}

/**
 * @tc.number: NotifyRemoveShellProcess_0100
 * @tc.desc: NotifyRemoveShellProcess
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerCollaboratorProxyTest, NotifyRemoveShellProcess_0100, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerCollaboratorStubMock::InvokeSendRequest));
    int32_t pid = 0;
    int32_t type = 0;
    std::string reason = "";
    int32_t res = proxy_->NotifyRemoveShellProcess(pid, type, reason);
    EXPECT_EQ(res, NO_ERROR);
    EXPECT_EQ(static_cast<uint32_t>(IAbilityManagerCollaborator::NOTIFY_REMOVE_SHELL_PROCESS), mock_->GetCode());
}

/**
 * @tc.number: UpdateMissionInfo_0100
 * @tc.desc: UpdateMissionInfo
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerCollaboratorProxyTest, UpdateMissionInfo_0100, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerCollaboratorStubMock::InvokeSendRequest));
    sptr<SessionInfo> sessionInfo;
    proxy_->UpdateMissionInfo(sessionInfo);
    EXPECT_EQ(static_cast<uint32_t>(IAbilityManagerCollaborator::UPDATE_MISSION_INFO_BY_SCB), mock_->GetCode());
}

/**
 * @tc.number: CheckCallAbilityPermission_0100
 * @tc.desc: CheckCallAbilityPermission
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerCollaboratorProxyTest, CheckCallAbilityPermission_0100, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerCollaboratorStubMock::InvokeSendRequest));
    Want want;
    int32_t res = proxy_->CheckCallAbilityPermission(want);
    EXPECT_EQ(res, NO_ERROR);
    EXPECT_EQ(static_cast<uint32_t>(IAbilityManagerCollaborator::CHECK_CALL_ABILITY_PERMISSION), mock_->GetCode());
}

/**
 * @tc.number: UpdateConfiguration_0100
 * @tc.desc: UpdateConfiguration
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerCollaboratorProxyTest, UpdateConfiguration_0100, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerCollaboratorStubMock::InvokeSendRequest));
    AppExecFwk::Configuration config;
    int32_t userId = 0;
    bool res = proxy_->UpdateConfiguration(config, userId);
    EXPECT_EQ(res, true);
    EXPECT_EQ(static_cast<uint32_t>(IAbilityManagerCollaborator::UPDATE_CONFIGURATION), mock_->GetCode());
}

/**
 * @tc.number: OpenFile_0100
 * @tc.desc: OpenFile
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerCollaboratorProxyTest, OpenFile_0100, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerCollaboratorStubMock::InvokeSendRequest));
    Uri uri("nullptr");
    uint32_t flag = 0;
    uint32_t tokenId = 0;
    int32_t res = proxy_->OpenFile(uri, flag, tokenId);
    EXPECT_EQ(res, -1);
    EXPECT_EQ(static_cast<uint32_t>(IAbilityManagerCollaborator::OPEN_FILE), mock_->GetCode());
}

/**
 * @tc.number: NotifyMissionBindPid_0100
 * @tc.desc: NotifyMissionBindPid
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerCollaboratorProxyTest, NotifyMissionBindPid_0100, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerCollaboratorStubMock::InvokeSendRequest));
    int32_t missionId = 0;
    int32_t pid = 0;
    int32_t userId = 100;
    proxy_->NotifyMissionBindPid(missionId, pid, userId);
    EXPECT_EQ(static_cast<uint32_t>(IAbilityManagerCollaborator::NOTIFY_MISSION_BIND_PID), mock_->GetCode());
}

/**
 * @tc.number: CheckStaticCfgPermission_0100
 * @tc.desc: CheckStaticCfgPermission
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerCollaboratorProxyTest, CheckStaticCfgPermission_0100, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerCollaboratorStubMock::InvokeSendRequest));
    Want want;
    bool isImplicit = true;
    int32_t res = proxy_->CheckStaticCfgPermission(want, isImplicit);
    EXPECT_EQ(res, NO_ERROR);
    EXPECT_EQ(static_cast<uint32_t>(IAbilityManagerCollaborator::CHECK_STATIC_CFG_PERMISSION), mock_->GetCode());
}

/**
 * @tc.number: NotifyKillProcesses_0100
 * @tc.desc: NotifyKillProcesses
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerCollaboratorProxyTest, NotifyKillProcesses_0100, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerCollaboratorStubMock::InvokeSendRequest));
    std::string bundleName;
    int32_t res = proxy_->NotifyKillProcesses(bundleName, 0);
    EXPECT_EQ(res, NO_ERROR);
    EXPECT_EQ(static_cast<uint32_t>(IAbilityManagerCollaborator::NOTIFY_KILL_PROCESSES), mock_->GetCode());
}

/**
 * @tc.number: GrantUriPermission_0100
 * @tc.desc: GrantUriPermission
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerCollaboratorProxyTest, GrantUriPermission_0100, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerCollaboratorStubMock::InvokeSendRequest));
    std::vector<std::string> uriVec = { "file://invalid/temp.txt" };
    uint32_t flag = 1;
    uint32_t targetTokenId = 0;
    std::string targetBundleName = "";
    int32_t res = proxy_->GrantUriPermission(uriVec, flag, targetTokenId, targetBundleName);
    EXPECT_EQ(res, NO_ERROR);
    EXPECT_EQ(static_cast<uint32_t>(IAbilityManagerCollaborator::GRANT_URI_PERMISSION), mock_->GetCode());
}

/**
 * @tc.number: RevokeUriPermission_0100
 * @tc.desc: RevokeUriPermission
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerCollaboratorProxyTest, RevokeUriPermission_0100, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerCollaboratorStubMock::InvokeSendRequest));
    uint32_t tokenId = 0;
    int32_t res = proxy_->RevokeUriPermission(tokenId);
    EXPECT_EQ(res, NO_ERROR);
    EXPECT_EQ(static_cast<uint32_t>(IAbilityManagerCollaborator::REVOKE_URI_PERMISSION), mock_->GetCode());
}

/**
 * @tc.number: NotifyGrantUriPermissionStart_0100
 * @tc.desc: NotifyGrantUriPermissionStart
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerCollaboratorProxyTest, NotifyGrantUriPermissionStart_0100, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerCollaboratorStubMock::InvokeSendRequest));
    int32_t userId = 0;
    uint32_t flag = 1;
    std::vector<std::string> uris = { "file://com.example.test/temp.txt" };
    int32_t res = proxy_->NotifyGrantUriPermissionStart(uris, flag, userId);
    EXPECT_EQ(res, NO_ERROR);
    EXPECT_EQ(static_cast<uint32_t>(IAbilityManagerCollaborator::NOTIFY_GRANT_URI_PERMISSION_START), mock_->GetCode());
}

/**
 * @tc.number: NotifyGrantUriPermissionEnd_0100
 * @tc.desc: NotifyGrantUriPermissionEnd
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerCollaboratorProxyTest, NotifyGrantUriPermissionEnd_0100, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerCollaboratorStubMock::InvokeSendRequest));
    int32_t userId = 0;
    uint32_t flag = 1;
    std::vector<std::string> uris = { "file://com.example.test/temp.txt" };
    std::vector<bool> checkResults = { true };
    int32_t res = proxy_->NotifyGrantUriPermissionEnd(uris, flag, userId, checkResults);
    EXPECT_EQ(res, NO_ERROR);
    EXPECT_EQ(static_cast<uint32_t>(IAbilityManagerCollaborator::NOTIFY_GRANT_URI_PERMISSION_END), mock_->GetCode());
}

/**
 * @tc.number: NotifyGrantUriPermissionEnd_0200
 * @tc.desc: NotifyGrantUriPermissionEnd
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerCollaboratorProxyTest, NotifyGrantUriPermissionEnd_0200, TestSize.Level1)
{
    int32_t userId = 0;
    uint32_t flag = 1;
    std::vector<std::string> uris = { "file://com.example.test/temp.txt" };
    std::vector<bool> checkResults;
    int32_t res = proxy_->NotifyGrantUriPermissionEnd(uris, flag, userId, checkResults);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/**
 * @tc.number: NotifyGrantUriPermissionEnd_0300
 * @tc.desc: NotifyGrantUriPermissionEnd
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerCollaboratorProxyTest, NotifyGrantUriPermissionEnd_0300, TestSize.Level1)
{
    int32_t userId = 0;
    uint32_t flag = 1;
    std::vector<std::string> uris(501, "file://com.example.test/temp.txt");
    std::vector<bool> checkResults(501, false);
    int32_t res = proxy_->NotifyGrantUriPermissionEnd(uris, flag, userId, checkResults);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}
} // namespace AbilityRuntime
} // namespace OHOS