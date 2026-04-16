/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ability_connect_callback_interface.h"
#include "agent_card.h"

#define private public
#include "ability_manager_errors.h"
#include "agent_manager_client.h"
#include "agent_load_callback.h"
#undef private
#include "hilog_tag_wrapper.h"
#include "ipc_object_stub.h"
#include "iremote_object.h"
#include "mock_agent_manager_service.h"
#include "mock_my_flag.h"
#include "want.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AgentRuntime;
using namespace OHOS::AAFwk;

namespace OHOS {
namespace AgentRuntime {
class AgentManagerClientTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
};

void AgentManagerClientTest::SetUpTestCase(void)
{}

void AgentManagerClientTest::TearDownTestCase(void)
{}

void AgentManagerClientTest::SetUp(void)
{
    MyFlag::retGetAllAgentCards = ERR_OK;
    MyFlag::retGetAgentCardsByBundleName = ERR_OK;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::retGetCallerAgentCardByAgentId = ERR_OK;
    MyFlag::retConnectAgentExtensionAbility = ERR_OK;
    MyFlag::retDisconnectAgentExtensionAbility = ERR_OK;
    MyFlag::retConnectServiceExtensionAbility = ERR_OK;
    MyFlag::retDisconnectServiceExtensionAbility = ERR_OK;
    MyFlag::retNotifyLowCodeAgentComplete = ERR_OK;
    MyFlag::nullSystemAbility = false;
    MyFlag::retRegisterAgentCard = ERR_OK;
    MyFlag::retUpdateAgentCard = ERR_OK;
    MyFlag::retDeleteAgentCard = ERR_OK;
}

void AgentManagerClientTest::TearDown(void)
{}

/**
* @tc.name  : GetAllAgentCards_ShouldReturnError_WhenProxyIsNull
* @tc.number: GetAllAgentCards_001
* @tc.desc  : Test that GetAllAgentCards returns ERR_NULL_AGENT_MGR_PROXY when the agent manager proxy is null.
*/
HWTEST_F(AgentManagerClientTest, GetAllAgentCards_001, TestSize.Level1)
{
    AgentManagerClient client;
    MyFlag::nullSystemAbility = true;
    
    std::vector<AgentCard> cards;
    int32_t result = client.GetAllAgentCards(cards);
    EXPECT_EQ(result, ERR_NULL_AGENT_MGR_PROXY);
}

/**
* @tc.name  : GetAllAgentCards_ShouldReturnError_WhenRawDataRetrievalFails
* @tc.number: GetAllAgentCards_002
* @tc.desc  : Test that GetAllAgentCards returns the error code when retrieving raw data fails.
*/
HWTEST_F(AgentManagerClientTest, GetAllAgentCards_002, TestSize.Level1)
{
    AgentManagerClient client;
    MyFlag::nullSystemAbility = false;
    auto mockAgentMgr = sptr<MockAgentManagerService>::MakeSptr();
    client.agentMgr_ = mockAgentMgr;
    MyFlag::retGetAllAgentCards = -1;

    std::vector<AgentCard> cards;
    int32_t result = client.GetAllAgentCards(cards);
    EXPECT_EQ(result, -1);
}

/**
* @tc.name  : GetAllAgentCards_ShouldReturnError_WhenToAgentCardVecFails
* @tc.number: GetAllAgentCards_003
* @tc.desc  : Test that GetAllAgentCards returns the error code when retrieving raw data fails.
*/
HWTEST_F(AgentManagerClientTest, GetAllAgentCards_003, TestSize.Level1)
{
    AgentManagerClient client;
    MyFlag::nullSystemAbility = false;
    auto mockAgentMgr = sptr<MockAgentManagerService>::MakeSptr();
    client.agentMgr_ = mockAgentMgr;
    MyFlag::retGetAllAgentCards = ERR_OK;
    MyFlag::retToAgentCardVec = -1;

    std::vector<AgentCard> cards;
    int32_t result = client.GetAllAgentCards(cards);
    EXPECT_EQ(result, -1);
}

/**
* @tc.name  : GetAllAgentCards_ShouldReturnSuccess_WhenAllOperationsSucceed
* @tc.number: GetAllAgentCards_004
* @tc.desc  : Test that GetAllAgentCards returns ERR_OK when all operations succeed.
*/
HWTEST_F(AgentManagerClientTest, GetAllAgentCards_004, TestSize.Level1)
{
    AgentManagerClient client;
    MyFlag::nullSystemAbility = false;
    auto mockAgentMgr = sptr<MockAgentManagerService>::MakeSptr();
    client.agentMgr_ = mockAgentMgr;
    MyFlag::retGetAllAgentCards = ERR_OK;
    MyFlag::retToAgentCardVec = ERR_OK;

    std::vector<AgentCard> cards;
    int32_t result = client.GetAllAgentCards(cards);
    EXPECT_EQ(result, ERR_OK);
}

/**
* @tc.name  : GetAgentCardsByBundleName_ShouldReturnError_WhenProxyIsNull
* @tc.number: GetAgentCardsByBundleName_001
* @tc.desc  : Test that GetAgentCardsByBundleName returns ERR_NULL_AGENT_MGR_PROXY when the agent manager proxy is null.
*/
HWTEST_F(AgentManagerClientTest, GetAgentCardsByBundleName_001, TestSize.Level1)
{
    AgentManagerClient client;
    MyFlag::nullSystemAbility = true;
    
    std::vector<AgentCard> cards;
    std::string bundleName = "bundle";
    int32_t result = client.GetAgentCardsByBundleName(bundleName, cards);
    EXPECT_EQ(result, ERR_NULL_AGENT_MGR_PROXY);
}

/**
* @tc.name  : GetAgentCardsByBundleName_ShouldReturnError_WhenRawDataRetrievalFails
* @tc.number: GetAgentCardsByBundleName_002
* @tc.desc  : Test that GetAgentCardsByBundleName returns the error code when retrieving raw data fails.
*/
HWTEST_F(AgentManagerClientTest, GetAgentCardsByBundleName_002, TestSize.Level1)
{
    AgentManagerClient client;
    MyFlag::nullSystemAbility = false;
    auto mockAgentMgr = sptr<MockAgentManagerService>::MakeSptr();
    client.agentMgr_ = mockAgentMgr;
    MyFlag::retGetAgentCardsByBundleName = -1;

    std::vector<AgentCard> cards;
    std::string bundleName = "bundle";
    int32_t result = client.GetAgentCardsByBundleName(bundleName, cards);
    EXPECT_EQ(result, -1);
}

/**
* @tc.name  : GetAgentCardsByBundleName_ShouldReturnSuccess_WhenAllOperationsSucceed
* @tc.number: GetAgentCardsByBundleName_003
* @tc.desc  : Test that GetAgentCardsByBundleName returns ERR_OK when all operations succeed.
*/
HWTEST_F(AgentManagerClientTest, GetAgentCardsByBundleName_003, TestSize.Level1)
{
    AgentManagerClient client;
    MyFlag::nullSystemAbility = false;
    auto mockAgentMgr = sptr<MockAgentManagerService>::MakeSptr();
    client.agentMgr_ = mockAgentMgr;
    MyFlag::retGetAgentCardsByBundleName = ERR_OK;

    std::vector<AgentCard> cards;
    std::string bundleName = "bundle";
    int32_t result = client.GetAgentCardsByBundleName(bundleName, cards);
    EXPECT_EQ(result, ERR_OK);
}

/**
* @tc.name  : GetAgentCardByAgentId_ShouldReturnError_WhenProxyIsNull
* @tc.number: GetAgentCardByAgentId_001
* @tc.desc  : Test that GetAgentCardByAgentId returns ERR_NULL_AGENT_MGR_PROXY when the agent manager proxy is null.
*/
HWTEST_F(AgentManagerClientTest, GetAgentCardByAgentId_001, TestSize.Level1)
{
    AgentManagerClient client;
    MyFlag::nullSystemAbility = true;

    AgentCard card;
    std::string bundleName = "bundle";
    std::string agentId = "agentId";
    int32_t result = client.GetAgentCardByAgentId(bundleName, agentId, card);
    EXPECT_EQ(result, ERR_NULL_AGENT_MGR_PROXY);
}

/**
* @tc.name  : GetAgentCardByAgentId_ShouldReturnError_WhenRawDataRetrievalFails
* @tc.number: GetAgentCardByAgentId_002
* @tc.desc  : Test that GetAgentCardByAgentId returns the error code when retrieving raw data fails.
*/
HWTEST_F(AgentManagerClientTest, GetAgentCardByAgentId_002, TestSize.Level1)
{
    AgentManagerClient client;
    MyFlag::nullSystemAbility = false;
    auto mockAgentMgr = sptr<MockAgentManagerService>::MakeSptr();
    client.agentMgr_ = mockAgentMgr;
    MyFlag::retGetAgentCardByAgentId = -1;

    AgentCard card;
    std::string bundleName = "bundle";
    std::string agentId = "agentId";
    int32_t result = client.GetAgentCardByAgentId(bundleName, agentId, card);
    EXPECT_EQ(result, -1);
}

/**
* @tc.name  : GetAgentCardByAgentId_ShouldReturnSuccess_WhenAllOperationsSucceed
* @tc.number: GetAgentCardByAgentId_003
* @tc.desc  : Test that GetAgentCardByAgentId returns ERR_OK when all operations succeed.
*/
HWTEST_F(AgentManagerClientTest, GetAgentCardByAgentId_003, TestSize.Level1)
{
    AgentManagerClient client;
    MyFlag::nullSystemAbility = false;
    auto mockAgentMgr = sptr<MockAgentManagerService>::MakeSptr();
    client.agentMgr_ = mockAgentMgr;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;

    AgentCard card;
    std::string bundleName = "bundle";
    std::string agentId = "agentId";
    int32_t result = client.GetAgentCardByAgentId(bundleName, agentId, card);
    EXPECT_EQ(result, ERR_OK);
}

/**
* @tc.name  : GetCallerAgentCardByAgentId_ShouldReturnError_WhenProxyIsNull
* @tc.number: GetCallerAgentCardByAgentId_001
 * @tc.desc : Test that GetCallerAgentCardByAgentId returns ERR_NULL_AGENT_MGR_PROXY when the agent manager proxy is
 * null.
*/
HWTEST_F(AgentManagerClientTest, GetCallerAgentCardByAgentId_001, TestSize.Level1)
{
    AgentManagerClient client;
    MyFlag::nullSystemAbility = true;

    AgentCard card;
    std::string agentId = "agentId";
    int32_t result = client.GetCallerAgentCardByAgentId(agentId, card);
    EXPECT_EQ(result, ERR_NULL_AGENT_MGR_PROXY);
}

/**
* @tc.name  : GetCallerAgentCardByAgentId_ShouldReturnError_WhenRetrievalFails
* @tc.number: GetCallerAgentCardByAgentId_002
* @tc.desc  : Test that GetCallerAgentCardByAgentId returns the error code when agent card retrieval fails.
*/
HWTEST_F(AgentManagerClientTest, GetCallerAgentCardByAgentId_002, TestSize.Level1)
{
    AgentManagerClient client;
    auto mockAgentMgr = sptr<MockAgentManagerService>::MakeSptr();
    client.agentMgr_ = mockAgentMgr;
    MyFlag::retGetCallerAgentCardByAgentId = -1;

    AgentCard card;
    std::string agentId = "agentId";
    int32_t result = client.GetCallerAgentCardByAgentId(agentId, card);
    EXPECT_EQ(result, -1);
}

/**
* @tc.name  : GetCallerAgentCardByAgentId_ShouldReturnSuccess_WhenAllOperationsSucceed
* @tc.number: GetCallerAgentCardByAgentId_003
* @tc.desc  : Test that GetCallerAgentCardByAgentId returns ERR_OK when all operations succeed.
*/
HWTEST_F(AgentManagerClientTest, GetCallerAgentCardByAgentId_003, TestSize.Level1)
{
    AgentManagerClient client;
    auto mockAgentMgr = sptr<MockAgentManagerService>::MakeSptr();
    client.agentMgr_ = mockAgentMgr;
    MyFlag::retGetCallerAgentCardByAgentId = ERR_OK;

    AgentCard card;
    std::string agentId = "agentId";
    int32_t result = client.GetCallerAgentCardByAgentId(agentId, card);
    EXPECT_EQ(result, ERR_OK);
}

/**
* @tc.name  : UpdateAgentCard_001
* @tc.number: UpdateAgentCard_001
* @tc.desc  : Test UpdateAgentCard returns ERR_NULL_AGENT_MGR_PROXY when proxy is null
*/
HWTEST_F(AgentManagerClientTest, UpdateAgentCard_001, TestSize.Level1)
{
    AgentManagerClient client;
    MyFlag::nullSystemAbility = true;

    AgentCard card;
    int32_t result = client.UpdateAgentCard(card);
    EXPECT_EQ(result, ERR_NULL_AGENT_MGR_PROXY);
}

/**
* @tc.name  : UpdateAgentCard_002
* @tc.number: UpdateAgentCard_002
* @tc.desc  : Test UpdateAgentCard returns service error when agent mgr call fails
*/
HWTEST_F(AgentManagerClientTest, UpdateAgentCard_002, TestSize.Level1)
{
    AgentManagerClient client;
    MyFlag::nullSystemAbility = false;
    auto mockAgentMgr = sptr<MockAgentManagerService>::MakeSptr();
    client.agentMgr_ = mockAgentMgr;
    MyFlag::retUpdateAgentCard = AAFwk::ERR_AGENT_CARD_VERSION_TOO_OLD;

    AgentCard card;
    int32_t result = client.UpdateAgentCard(card);
    EXPECT_EQ(result, AAFwk::ERR_AGENT_CARD_VERSION_TOO_OLD);
}

/**
* @tc.name  : UpdateAgentCard_003
* @tc.number: UpdateAgentCard_003
* @tc.desc  : Test UpdateAgentCard returns ERR_OK when all operations succeed
*/
HWTEST_F(AgentManagerClientTest, UpdateAgentCard_003, TestSize.Level1)
{
    AgentManagerClient client;
    MyFlag::nullSystemAbility = false;
    auto mockAgentMgr = sptr<MockAgentManagerService>::MakeSptr();
    client.agentMgr_ = mockAgentMgr;
    MyFlag::retUpdateAgentCard = ERR_OK;

    AgentCard card;
    int32_t result = client.UpdateAgentCard(card);
    EXPECT_EQ(result, ERR_OK);
}

/**
* @tc.name  : RegisterAgentCard_001
* @tc.number: RegisterAgentCard_001
* @tc.desc  : Test RegisterAgentCard returns ERR_NULL_AGENT_MGR_PROXY when proxy is null
*/
HWTEST_F(AgentManagerClientTest, RegisterAgentCard_001, TestSize.Level1)
{
    AgentManagerClient client;
    MyFlag::nullSystemAbility = true;

    AgentCard card;
    int32_t result = client.RegisterAgentCard(card);
    EXPECT_EQ(result, ERR_NULL_AGENT_MGR_PROXY);
}

/**
* @tc.name  : RegisterAgentCard_002
* @tc.number: RegisterAgentCard_002
* @tc.desc  : Test RegisterAgentCard returns service error when agent mgr call fails
*/
HWTEST_F(AgentManagerClientTest, RegisterAgentCard_002, TestSize.Level1)
{
    AgentManagerClient client;
    MyFlag::nullSystemAbility = false;
    auto mockAgentMgr = sptr<MockAgentManagerService>::MakeSptr();
    client.agentMgr_ = mockAgentMgr;
    MyFlag::retRegisterAgentCard = AAFwk::ERR_AGENT_CARD_DUPLICATE_REGISTER;

    AgentCard card;
    int32_t result = client.RegisterAgentCard(card);
    EXPECT_EQ(result, AAFwk::ERR_AGENT_CARD_DUPLICATE_REGISTER);
}

/**
* @tc.name  : RegisterAgentCard_003
* @tc.number: RegisterAgentCard_003
* @tc.desc  : Test RegisterAgentCard returns ERR_OK when all operations succeed
*/
HWTEST_F(AgentManagerClientTest, RegisterAgentCard_003, TestSize.Level1)
{
    AgentManagerClient client;
    MyFlag::nullSystemAbility = false;
    auto mockAgentMgr = sptr<MockAgentManagerService>::MakeSptr();
    client.agentMgr_ = mockAgentMgr;
    MyFlag::retRegisterAgentCard = ERR_OK;

    AgentCard card;
    int32_t result = client.RegisterAgentCard(card);
    EXPECT_EQ(result, ERR_OK);
}

/**
* @tc.name  : DeleteAgentCard_001
* @tc.number: DeleteAgentCard_001
* @tc.desc  : Test DeleteAgentCard returns ERR_NULL_AGENT_MGR_PROXY when proxy is null
*/
HWTEST_F(AgentManagerClientTest, DeleteAgentCard_001, TestSize.Level1)
{
    AgentManagerClient client;
    MyFlag::nullSystemAbility = true;

    int32_t result = client.DeleteAgentCard("bundle", "agentId");
    EXPECT_EQ(result, ERR_NULL_AGENT_MGR_PROXY);
}

/**
* @tc.name  : DeleteAgentCard_002
* @tc.number: DeleteAgentCard_002
* @tc.desc  : Test DeleteAgentCard returns service error when agent mgr call fails
*/
HWTEST_F(AgentManagerClientTest, DeleteAgentCard_002, TestSize.Level1)
{
    AgentManagerClient client;
    MyFlag::nullSystemAbility = false;
    auto mockAgentMgr = sptr<MockAgentManagerService>::MakeSptr();
    client.agentMgr_ = mockAgentMgr;
    MyFlag::retDeleteAgentCard = AAFwk::ERR_INVALID_AGENT_CARD_ID;

    int32_t result = client.DeleteAgentCard("bundle", "agentId");
    EXPECT_EQ(result, AAFwk::ERR_INVALID_AGENT_CARD_ID);
}

/**
* @tc.name  : DeleteAgentCard_003
* @tc.number: DeleteAgentCard_003
* @tc.desc  : Test DeleteAgentCard returns ERR_OK when all operations succeed
*/
HWTEST_F(AgentManagerClientTest, DeleteAgentCard_003, TestSize.Level1)
{
    AgentManagerClient client;
    MyFlag::nullSystemAbility = false;
    auto mockAgentMgr = sptr<MockAgentManagerService>::MakeSptr();
    client.agentMgr_ = mockAgentMgr;
    MyFlag::retDeleteAgentCard = ERR_OK;

    int32_t result = client.DeleteAgentCard("bundle", "agentId");
    EXPECT_EQ(result, ERR_OK);
}

/**
* @tc.name  : GetAgentMgrProxy_ShouldReturnAgentMgr_WhenAgentMgrIsAlreadyStarted
* @tc.number: GetAgentMgrProxy_001
* @tc.desc  : Test that GetAgentMgrProxy returns the existing agent manager when it is already started.
*/
HWTEST_F(AgentManagerClientTest, GetAgentMgrProxy_001, TestSize.Level1)
{
    AgentManagerClient client;
    auto mockAgentMgr = sptr<MockAgentManagerService>::MakeSptr();
    client.agentMgr_ = mockAgentMgr;

    auto result = client.GetAgentMgrProxy();
    EXPECT_EQ(result->AsObject(), mockAgentMgr->AsObject());
}

/**
* @tc.name  : GetAgentMgrProxy_ShouldReturnNull_WhenLoadAgentMgrServiceFails
* @tc.number: GetAgentMgrProxy_002
* @tc.desc  : Test that GetAgentMgrProxy returns nullptr when loading the agent manager service fails.
*/
HWTEST_F(AgentManagerClientTest, GetAgentMgrProxy_002, TestSize.Level1)
{
    AgentManagerClient client;
    MyFlag::nullSystemAbility = true;

    auto result = client.GetAgentMgrProxy();
    EXPECT_EQ(result, nullptr);
}

/**
* @tc.name  : GetAgentMgrProxy_ShouldReturnNull_WhenGetAgentMgrFailsAfterLoad
* @tc.number: GetAgentMgrProxy_003
* @tc.desc  : Test that GetAgentMgrProxy returns nullptr when getting the agent manager fails after loading the service.
*/
HWTEST_F(AgentManagerClientTest, GetAgentMgrProxy_003, TestSize.Level1)
{
    MyFlag::nullSystemAbility = false;
    MyFlag::retLoadSystemAbility = 0;
    MyFlag::agentMgr = nullptr;
    MyFlag::shouldCallback = true;

    auto result = AgentManagerClient::GetInstance().GetAgentMgrProxy();
    EXPECT_EQ(result, nullptr);
}

/**
* @tc.name  : GetAgentMgrProxy_ShouldReturnAgentMgr_WhenAgentMgrIsSuccessfullyLoaded
* @tc.number: GetAgentMgrProxy_004
* @tc.desc  : Test that GetAgentMgrProxy returns the agent manager when it is successfully loaded.
*/
HWTEST_F(AgentManagerClientTest, GetAgentMgrProxy_004, TestSize.Level1)
{
    MyFlag::nullSystemAbility = false;
    MyFlag::retLoadSystemAbility = 0;
    auto mockAgentMgr = sptr<MockAgentManagerService>::MakeSptr();
    MyFlag::agentMgr = mockAgentMgr;
    MyFlag::shouldCallback = true;

    auto result = AgentManagerClient::GetInstance().GetAgentMgrProxy();
    EXPECT_EQ(result->AsObject(), mockAgentMgr->AsObject());
}

/**
* @tc.name  : ClearProxy_ShouldSetAgentMgrToNull_WhenCalled
* @tc.number: ClearProxy_001
* @tc.desc  : Verify that the agentMgr_ pointer is set to nullptr when ClearProxy is called.
*/
HWTEST_F(AgentManagerClientTest, ClearProxy_001, TestSize.Level1)
{
    AgentManagerClient client;
    auto mockAgentMgr = sptr<MockAgentManagerService>::MakeSptr();
    client.agentMgr_ = mockAgentMgr;

    // Act: Call ClearProxy
    client.ClearProxy();
    EXPECT_EQ(client.agentMgr_, nullptr);
}

/**
* @tc.name  : LoadAgentMgrService_ShouldReturnFalse_WhenSystemAbilityManagerIsNull
* @tc.number: LoadAgentMgrService_001
* @tc.desc  : Test that LoadAgentMgrService returns false when SystemAbilityManager is null
*/
HWTEST_F(AgentManagerClientTest, LoadAgentMgrService_001, TestSize.Level1)
{
    MyFlag::nullSystemAbility = true;

    EXPECT_FALSE(AgentManagerClient::GetInstance().LoadAgentMgrService());
}

/**
* @tc.name  : LoadAgentMgrService_ShouldReturnFalse_WhenLoadSystemAbilityFails
* @tc.number: LoadAgentMgrService_002
* @tc.desc  : Test that LoadAgentMgrService returns false when LoadSystemAbility fails
*/
HWTEST_F(AgentManagerClientTest, LoadAgentMgrService_002, TestSize.Level1)
{
    MyFlag::nullSystemAbility = false;
    MyFlag::retLoadSystemAbility = -1;

    EXPECT_FALSE(AgentManagerClient::GetInstance().LoadAgentMgrService());
}

/**
* @tc.name  : LoadAgentMgrService_ShouldReturnFalse_WhenLoadSaTimeout
* @tc.number: LoadAgentMgrService_003
* @tc.desc  : Test that LoadAgentMgrService returns false when waiting for system ability loading times out
*/
HWTEST_F(AgentManagerClientTest, LoadAgentMgrService_003, TestSize.Level1)
{
    MyFlag::nullSystemAbility = false;
    MyFlag::retLoadSystemAbility = 0;
    MyFlag::shouldCallback = false;

    EXPECT_FALSE(AgentManagerClient::GetInstance().LoadAgentMgrService());
}

/**
* @tc.name  : LoadAgentMgrService_ShouldReturnTrue_WhenAllStepsSucceed
* @tc.number: LoadAgentMgrService_004
* @tc.desc  : Test that LoadAgentMgrService returns true when all steps succeed
*/
HWTEST_F(AgentManagerClientTest, LoadAgentMgrService_004, TestSize.Level1)
{
    MyFlag::nullSystemAbility = false;
    MyFlag::retLoadSystemAbility = 0;
    MyFlag::shouldCallback = true;

    EXPECT_TRUE(AgentManagerClient::GetInstance().LoadAgentMgrService());
}

/**
* @tc.name  : SetAgentMgr_ShouldSetAgentMgr_WhenValidRemoteObjectIsPassed
* @tc.number: SetAgentMgr_001
* @tc.desc  : Test that the agentMgr_ is set correctly when a valid remoteObject is passed.
*/
HWTEST_F(AgentManagerClientTest, SetAgentMgr_001, TestSize.Level1)
{
    AgentManagerClient client;
    auto mockAgentMgr = sptr<MockAgentManagerService>::MakeSptr();
    ASSERT_NE(mockAgentMgr, nullptr);

    client.SetAgentMgr(mockAgentMgr);

    EXPECT_NE(client.agentMgr_, nullptr);
}

/**
* @tc.name  : SetAgentMgr_ShouldSetAgentMgrToNull_WhenNullRemoteObjectIsPassed
* @tc.number: SetAgentMgr_002
* @tc.desc  : Test that the agentMgr_ is set to nullptr when a nullptr is passed.
*/
HWTEST_F(AgentManagerClientTest, SetAgentMgr_002, TestSize.Level1)
{
    AgentManagerClient client;

    client.SetAgentMgr(nullptr);

    EXPECT_EQ(client.agentMgr_, nullptr);
}

/**
* @tc.name  : GetAgentMgr_ShouldReturnAgentMgr_WhenCalled
* @tc.number: GetAgentMgr_001
* @tc.desc  : Test that GetAgentMgr returns the agentMgr_ object when called.
*/
HWTEST_F(AgentManagerClientTest, GetAgentMgr_001, TestSize.Level1)
{
    AgentManagerClient client;
    auto mockAgentMgr = sptr<MockAgentManagerService>::MakeSptr();
    client.agentMgr_ = mockAgentMgr;

    // Act
    sptr<IAgentManager> actualAgentMgr = client.GetAgentMgr();

    // Assert
    EXPECT_EQ(mockAgentMgr->AsObject(), actualAgentMgr->AsObject());
}

/**
* @tc.name  : GetAgentMgr_ShouldReturnAgentMgr_WhenCalled
* @tc.number: GetAgentMgr_002
* @tc.desc  : Test that GetAgentMgr returns the agentMgr_ object when called.
*/
HWTEST_F(AgentManagerClientTest, GetAgentMgr_002, TestSize.Level1)
{
    AgentManagerClient client;

    // Act
    sptr<IAgentManager> actualAgentMgr = client.GetAgentMgr();

    // Assert
    EXPECT_EQ(actualAgentMgr, nullptr);
}

/**
* @tc.name  : SetAgentMgr_ShouldBeCalledWithRemoteObject_WhenOnLoadSystemAbilitySuccessIsCalled
* @tc.number: OnLoadSystemAbilitySuccess_001
* @tc.desc  : Verify that SetAgentMgr is called with the correct remoteObject
*/
HWTEST_F(AgentManagerClientTest, OnLoadSystemAbilitySuccess_001, TestSize.Level1)
{
    AgentManagerClient client;
    auto mockAgentMgr = sptr<MockAgentManagerService>::MakeSptr();

    // Act
    client.OnLoadSystemAbilitySuccess(mockAgentMgr);

    EXPECT_EQ(client.agentMgr_->AsObject(), mockAgentMgr->AsObject());
    EXPECT_TRUE(client.loadSaFinished_);
}

/**
* @tc.name  : SetAgentMgr_ShouldBeCalledWithRemoteObject_WhenOnLoadSystemAbilityFailIsCalled
* @tc.number: OnLoadSystemAbilityFail_001
* @tc.desc  : Verify that SetAgentMgr is called with the correct remoteObject
*/
HWTEST_F(AgentManagerClientTest, OnLoadSystemAbilityFail_001, TestSize.Level1)
{
    AgentManagerClient client;

    // Act
    client.OnLoadSystemAbilityFail();

    EXPECT_EQ(client.agentMgr_, nullptr);
    EXPECT_TRUE(client.loadSaFinished_);
}

/**
* @tc.name  : OnRemoteDied_001
* @tc.number: OnRemoteDied_001
* @tc.desc  : Test that the OnRemoteDied function calls the proxy callback when proxy_ is not nullptr.
*/
HWTEST_F(AgentManagerClientTest, OnRemoteDied_001, TestSize.Level1)
{
    // Mock the proxy callback
    bool proxyCalled = false;
    auto proxy = [&proxyCalled](const wptr<IRemoteObject> &) {
        proxyCalled = true;
    };
    
    // Arrange
    auto deathRecipient = sptr<AgentManagerClient::AgentManagerServiceDeathRecipient>::MakeSptr(proxy);
    wptr<IRemoteObject> remoteObject = nullptr;

    // Act
    deathRecipient->OnRemoteDied(remoteObject);

    // Assert
    EXPECT_TRUE(proxyCalled);
}

/**
* @tc.name  : OnRemoteDied_002
* @tc.number: OnRemoteDied_002
* @tc.desc  : Test that the OnRemoteDied function does not call the proxy callback when proxy_ is nullptr.
*/
HWTEST_F(AgentManagerClientTest, OnRemoteDied_002, TestSize.Level1)
{
    // Mock the proxy callback
    bool proxyCalled = false;
    auto proxy = [&proxyCalled](const wptr<IRemoteObject> &) {
        proxyCalled = true;
    };

    // Arrange
    auto deathRecipient = sptr<AgentManagerClient::AgentManagerServiceDeathRecipient>::MakeSptr(proxy);
    wptr<IRemoteObject> remoteObject = nullptr;

    // Act
    deathRecipient->proxy_ = nullptr;
    deathRecipient->OnRemoteDied(remoteObject);

    // Assert
    EXPECT_FALSE(proxyCalled);
}

namespace {
class MockAbilityConnection : public IRemoteStub<AAFwk::IAbilityConnection> {
public:
    MockAbilityConnection() = default;
    ~MockAbilityConnection() override = default;

    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        return 0;
    }

    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject,
        int32_t resultCode) override
    {}

    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int32_t resultCode) override
    {}
};
}

/**
* @tc.name  : ConnectAgentExtensionAbility_001
* @tc.number: ConnectAgentExtensionAbility_001
* @tc.desc  : Test ConnectAgentExtensionAbility returns ERR_NULL_AGENT_MGR_PROXY when proxy is null
*/
HWTEST_F(AgentManagerClientTest, ConnectAgentExtensionAbility_001, TestSize.Level1)
{
    AgentManagerClient client;
    MyFlag::nullSystemAbility = true;

    AAFwk::Want want;
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    int32_t result = client.ConnectAgentExtensionAbility(want, connection);
    EXPECT_EQ(result, ERR_NULL_AGENT_MGR_PROXY);
}

/**
* @tc.name  : ConnectAgentExtensionAbility_002
* @tc.number: ConnectAgentExtensionAbility_002
* @tc.desc  : Test ConnectAgentExtensionAbility returns error when agent mgr call fails
*/
HWTEST_F(AgentManagerClientTest, ConnectAgentExtensionAbility_002, TestSize.Level1)
{
    AgentManagerClient client;
    MyFlag::nullSystemAbility = false;
    auto mockAgentMgr = sptr<MockAgentManagerService>::MakeSptr();
    client.agentMgr_ = mockAgentMgr;
    MyFlag::retConnectAgentExtensionAbility = -1;

    AAFwk::Want want;
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    int32_t result = client.ConnectAgentExtensionAbility(want, connection);
    EXPECT_EQ(result, -1);
}

/**
* @tc.name  : ConnectAgentExtensionAbility_003
* @tc.number: ConnectAgentExtensionAbility_003
* @tc.desc  : Test ConnectAgentExtensionAbility returns ERR_OK when all operations succeed
*/
HWTEST_F(AgentManagerClientTest, ConnectAgentExtensionAbility_003, TestSize.Level1)
{
    AgentManagerClient client;
    MyFlag::nullSystemAbility = false;
    auto mockAgentMgr = sptr<MockAgentManagerService>::MakeSptr();
    client.agentMgr_ = mockAgentMgr;
    MyFlag::retConnectAgentExtensionAbility = ERR_OK;

    AAFwk::Want want;
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    int32_t result = client.ConnectAgentExtensionAbility(want, connection);
    EXPECT_EQ(result, ERR_OK);
}

/**
* @tc.name  : DisconnectAgentExtensionAbility_001
* @tc.number: DisconnectAgentExtensionAbility_001
* @tc.desc  : Test DisconnectAgentExtensionAbility returns ERR_NULL_AGENT_MGR_PROXY when proxy is null
*/
HWTEST_F(AgentManagerClientTest, DisconnectAgentExtensionAbility_001, TestSize.Level1)
{
    AgentManagerClient client;
    MyFlag::nullSystemAbility = true;

    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    int32_t result = client.DisconnectAgentExtensionAbility(connection);
    EXPECT_EQ(result, ERR_NULL_AGENT_MGR_PROXY);
}

/**
* @tc.name  : DisconnectAgentExtensionAbility_002
* @tc.number: DisconnectAgentExtensionAbility_002
* @tc.desc  : Test DisconnectAgentExtensionAbility returns error when agent mgr call fails
*/
HWTEST_F(AgentManagerClientTest, DisconnectAgentExtensionAbility_002, TestSize.Level1)
{
    AgentManagerClient client;
    MyFlag::nullSystemAbility = false;
    auto mockAgentMgr = sptr<MockAgentManagerService>::MakeSptr();
    client.agentMgr_ = mockAgentMgr;
    MyFlag::retDisconnectAgentExtensionAbility = -1;

    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    int32_t result = client.DisconnectAgentExtensionAbility(connection);
    EXPECT_EQ(result, -1);
}

/**
* @tc.name  : DisconnectAgentExtensionAbility_003
* @tc.number: DisconnectAgentExtensionAbility_003
* @tc.desc  : Test DisconnectAgentExtensionAbility returns ERR_OK when all operations succeed
*/
HWTEST_F(AgentManagerClientTest, DisconnectAgentExtensionAbility_003, TestSize.Level1)
{
    AgentManagerClient client;
    MyFlag::nullSystemAbility = false;
    auto mockAgentMgr = sptr<MockAgentManagerService>::MakeSptr();
    client.agentMgr_ = mockAgentMgr;
    MyFlag::retDisconnectAgentExtensionAbility = ERR_OK;

    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    int32_t result = client.DisconnectAgentExtensionAbility(connection);
    EXPECT_EQ(result, ERR_OK);
}

/**
* @tc.name  : ConnectServiceExtensionAbility_001
* @tc.number: ConnectServiceExtensionAbility_001
* @tc.desc  : Test ConnectServiceExtensionAbility returns ERR_NULL_AGENT_MGR_PROXY when proxy is null
*/
HWTEST_F(AgentManagerClientTest, ConnectServiceExtensionAbility_001, TestSize.Level1)
{
    AgentManagerClient client;
    MyFlag::nullSystemAbility = true;

    AAFwk::Want want;
    sptr<IRemoteObject> callerToken = new IPCObjectStub(u"caller.token");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    int32_t result = client.ConnectServiceExtensionAbility(callerToken, want, connection);
    EXPECT_EQ(result, ERR_NULL_AGENT_MGR_PROXY);
}

/**
* @tc.name  : ConnectServiceExtensionAbility_002
* @tc.number: ConnectServiceExtensionAbility_002
* @tc.desc  : Test ConnectServiceExtensionAbility returns error when agent mgr call fails
*/
HWTEST_F(AgentManagerClientTest, ConnectServiceExtensionAbility_002, TestSize.Level1)
{
    AgentManagerClient client;
    MyFlag::nullSystemAbility = false;
    auto mockAgentMgr = sptr<MockAgentManagerService>::MakeSptr();
    client.agentMgr_ = mockAgentMgr;
    MyFlag::retConnectServiceExtensionAbility = -1;

    AAFwk::Want want;
    sptr<IRemoteObject> callerToken = new IPCObjectStub(u"caller.token");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    int32_t result = client.ConnectServiceExtensionAbility(callerToken, want, connection);
    EXPECT_EQ(result, -1);
}

/**
* @tc.name  : ConnectServiceExtensionAbility_003
* @tc.number: ConnectServiceExtensionAbility_003
* @tc.desc  : Test ConnectServiceExtensionAbility returns ERR_OK when all operations succeed
*/
HWTEST_F(AgentManagerClientTest, ConnectServiceExtensionAbility_003, TestSize.Level1)
{
    AgentManagerClient client;
    MyFlag::nullSystemAbility = false;
    auto mockAgentMgr = sptr<MockAgentManagerService>::MakeSptr();
    client.agentMgr_ = mockAgentMgr;
    MyFlag::retConnectServiceExtensionAbility = ERR_OK;

    AAFwk::Want want;
    sptr<IRemoteObject> callerToken = new IPCObjectStub(u"caller.token");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    int32_t result = client.ConnectServiceExtensionAbility(callerToken, want, connection);
    EXPECT_EQ(result, ERR_OK);
}

/**
* @tc.name  : DisconnectServiceExtensionAbility_001
* @tc.number: DisconnectServiceExtensionAbility_001
* @tc.desc  : Test DisconnectServiceExtensionAbility returns ERR_NULL_AGENT_MGR_PROXY when proxy is null
*/
HWTEST_F(AgentManagerClientTest, DisconnectServiceExtensionAbility_001, TestSize.Level1)
{
    AgentManagerClient client;
    MyFlag::nullSystemAbility = true;

    auto callerToken = sptr<IRemoteObject>(new IPCObjectStub(u"caller.token"));
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    int32_t result = client.DisconnectServiceExtensionAbility(callerToken, connection);
    EXPECT_EQ(result, ERR_NULL_AGENT_MGR_PROXY);
}

/**
* @tc.name  : DisconnectServiceExtensionAbility_002
* @tc.number: DisconnectServiceExtensionAbility_002
* @tc.desc  : Test DisconnectServiceExtensionAbility returns error when agent mgr call fails
*/
HWTEST_F(AgentManagerClientTest, DisconnectServiceExtensionAbility_002, TestSize.Level1)
{
    AgentManagerClient client;
    MyFlag::nullSystemAbility = false;
    auto mockAgentMgr = sptr<MockAgentManagerService>::MakeSptr();
    client.agentMgr_ = mockAgentMgr;
    MyFlag::retDisconnectServiceExtensionAbility = -1;

    auto callerToken = sptr<IRemoteObject>(new IPCObjectStub(u"caller.token"));
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    int32_t result = client.DisconnectServiceExtensionAbility(callerToken, connection);
    EXPECT_EQ(result, -1);
}

/**
* @tc.name  : DisconnectServiceExtensionAbility_003
* @tc.number: DisconnectServiceExtensionAbility_003
* @tc.desc  : Test DisconnectServiceExtensionAbility returns ERR_OK when all operations succeed
*/
HWTEST_F(AgentManagerClientTest, DisconnectServiceExtensionAbility_003, TestSize.Level1)
{
    AgentManagerClient client;
    MyFlag::nullSystemAbility = false;
    auto mockAgentMgr = sptr<MockAgentManagerService>::MakeSptr();
    client.agentMgr_ = mockAgentMgr;
    MyFlag::retDisconnectServiceExtensionAbility = ERR_OK;

    auto callerToken = sptr<IRemoteObject>(new IPCObjectStub(u"caller.token"));
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    int32_t result = client.DisconnectServiceExtensionAbility(callerToken, connection);
    EXPECT_EQ(result, ERR_OK);
}

/**
* @tc.name  : NotifyLowCodeAgentComplete_001
* @tc.number: NotifyLowCodeAgentComplete_001
* @tc.desc  : Test NotifyLowCodeAgentComplete returns ERR_NULL_AGENT_MGR_PROXY when proxy is null
*/
HWTEST_F(AgentManagerClientTest, NotifyLowCodeAgentComplete_001, TestSize.Level1)
{
    AgentManagerClient client;
    MyFlag::nullSystemAbility = true;

    int32_t result = client.NotifyLowCodeAgentComplete("agentA");
    EXPECT_EQ(result, ERR_NULL_AGENT_MGR_PROXY);
}

/**
* @tc.name  : NotifyLowCodeAgentComplete_002
* @tc.number: NotifyLowCodeAgentComplete_002
* @tc.desc  : Test NotifyLowCodeAgentComplete returns error when agent mgr call fails
*/
HWTEST_F(AgentManagerClientTest, NotifyLowCodeAgentComplete_002, TestSize.Level1)
{
    AgentManagerClient client;
    MyFlag::nullSystemAbility = false;
    auto mockAgentMgr = sptr<MockAgentManagerService>::MakeSptr();
    client.agentMgr_ = mockAgentMgr;
    MyFlag::retNotifyLowCodeAgentComplete = -1;

    int32_t result = client.NotifyLowCodeAgentComplete("agentA");
    EXPECT_EQ(result, -1);
}

/**
* @tc.name  : NotifyLowCodeAgentComplete_003
* @tc.number: NotifyLowCodeAgentComplete_003
* @tc.desc  : Test NotifyLowCodeAgentComplete returns ERR_OK when all operations succeed
*/
HWTEST_F(AgentManagerClientTest, NotifyLowCodeAgentComplete_003, TestSize.Level1)
{
    AgentManagerClient client;
    MyFlag::nullSystemAbility = false;
    auto mockAgentMgr = sptr<MockAgentManagerService>::MakeSptr();
    client.agentMgr_ = mockAgentMgr;
    MyFlag::retNotifyLowCodeAgentComplete = ERR_OK;

    int32_t result = client.NotifyLowCodeAgentComplete("agentA");
    EXPECT_EQ(result, ERR_OK);
}
} // namespace AgentRuntime
} // namespace OHOS
