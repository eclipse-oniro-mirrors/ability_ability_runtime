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

#include <gtest/gtest.h>

#include "ability_connect_callback_interface.h"

#define private public
#include "agent_manager_service.h"
#include "agent_service_connection.h"
#undef private
#include "ipc_object_stub.h"
#include "mock_my_flag.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AgentRuntime;

namespace OHOS {
namespace AgentRuntime {
namespace {
class TrackingAbilityConnection : public IRemoteStub<AAFwk::IAbilityConnection> {
public:
    int connectDoneCount = 0;
    int disconnectDoneCount = 0;
    int32_t lastResultCode = ERR_OK;
    sptr<IRemoteObject> lastRemoteObject = nullptr;

    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        return 0;
    }

    void OnAbilityConnectDone(const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject, int32_t resultCode) override
    {
        connectDoneCount++;
        lastResultCode = resultCode;
        lastRemoteObject = remoteObject;
    }

    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int32_t resultCode) override
    {
        disconnectDoneCount++;
        lastResultCode = resultCode;
    }
};
}  // namespace

class AgentServiceConnectionTest : public testing::Test {
public:
    void SetUp() override
    {
        MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
        MyFlag::retDisconnectAbility = ERR_OK;
        MyFlag::lastConnectAbilityConnection = nullptr;
        MyFlag::lastDisconnectAbilityConnection = nullptr;
        auto service = AgentManagerService::GetInstance();
        service->trackedConnections_.clear();
        service->callerConnectionCounts_.clear();
    }
};

/**
* @tc.name  : OnAbilityConnectDone_001
* @tc.number: OnAbilityConnectDone_001
* @tc.desc  : Test AgentServiceConnection forwards connect callback and keeps successful tracking state
*/
HWTEST_F(AgentServiceConnectionTest, OnAbilityConnectDone_001, TestSize.Level1)
{
    auto callerConnection = sptr<TrackingAbilityConnection>::MakeSptr();
    auto service = AgentManagerService::GetInstance();
    service->callerConnectionCounts_[100] = 1;
    AgentManagerService::TrackedConnectionRecord record;
    record.callerUid = 100;
    service->trackedConnections_.emplace(callerConnection->AsObject(), record);

    auto agentConnection = sptr<AgentServiceConnection>::MakeSptr(callerConnection);
    sptr<IRemoteObject> remoteObject = new (std::nothrow) IPCObjectStub(u"test.remote");
    AppExecFwk::ElementName element;
    agentConnection->OnAbilityConnectDone(element, remoteObject, ERR_OK);

    EXPECT_EQ(callerConnection->connectDoneCount, 1);
    EXPECT_EQ(callerConnection->lastResultCode, ERR_OK);
    EXPECT_EQ(callerConnection->lastRemoteObject, remoteObject);
    EXPECT_EQ(service->trackedConnections_.size(), 1);
    EXPECT_EQ(service->callerConnectionCounts_[100], 1);
}

/**
* @tc.name  : OnAbilityConnectDone_002
* @tc.number: OnAbilityConnectDone_002
* @tc.desc  : Test AgentServiceConnection releases tracked state on connect failure
*/
HWTEST_F(AgentServiceConnectionTest, OnAbilityConnectDone_002, TestSize.Level1)
{
    auto callerConnection = sptr<TrackingAbilityConnection>::MakeSptr();
    auto service = AgentManagerService::GetInstance();
    service->callerConnectionCounts_[100] = 1;
    AgentManagerService::TrackedConnectionRecord record;
    record.callerUid = 100;
    service->trackedConnections_.emplace(callerConnection->AsObject(), record);

    auto agentConnection = sptr<AgentServiceConnection>::MakeSptr(callerConnection);
    AppExecFwk::ElementName element;
    agentConnection->OnAbilityConnectDone(element, nullptr, ERR_INVALID_VALUE);

    EXPECT_EQ(callerConnection->connectDoneCount, 1);
    EXPECT_EQ(callerConnection->lastResultCode, ERR_INVALID_VALUE);
    EXPECT_TRUE(service->trackedConnections_.empty());
    EXPECT_TRUE(service->callerConnectionCounts_.empty());
}

/**
* @tc.name  : OnAbilityDisconnectDone_001
* @tc.number: OnAbilityDisconnectDone_001
* @tc.desc  : Test AgentServiceConnection forwards disconnect callback and releases tracked state
*/
HWTEST_F(AgentServiceConnectionTest, OnAbilityDisconnectDone_001, TestSize.Level1)
{
    auto callerConnection = sptr<TrackingAbilityConnection>::MakeSptr();
    auto service = AgentManagerService::GetInstance();
    service->callerConnectionCounts_[100] = 1;
    AgentManagerService::TrackedConnectionRecord record;
    record.callerUid = 100;
    service->trackedConnections_.emplace(callerConnection->AsObject(), record);

    auto agentConnection = sptr<AgentServiceConnection>::MakeSptr(callerConnection);
    AppExecFwk::ElementName element;
    agentConnection->OnAbilityDisconnectDone(element, ERR_OK);

    EXPECT_EQ(callerConnection->disconnectDoneCount, 1);
    EXPECT_EQ(callerConnection->lastResultCode, ERR_OK);
    EXPECT_TRUE(service->trackedConnections_.empty());
    EXPECT_TRUE(service->callerConnectionCounts_.empty());
}
}  // namespace AgentRuntime
}  // namespace OHOS
