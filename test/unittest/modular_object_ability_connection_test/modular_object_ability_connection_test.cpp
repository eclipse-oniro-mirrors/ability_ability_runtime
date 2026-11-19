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

#include "modular_object_ability_connection.h"
#include "modular_object_connection_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {

bool ModularObjectConnectionManager::g_disconnectNonexistentResult = false;
bool ModularObjectConnectionManager::g_removeConnectionCalled = false;

class MockConnectCallback : public AbilityConnectCallback {
public:
    bool connectCalled = false;
    bool disconnectCalled = false;
    int lastResultCode = 0;

    void OnAbilityConnectDone(const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject, int resultCode) override
    {
        connectCalled = true;
        lastResultCode = resultCode;
    }

    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override
    {
        disconnectCalled = true;
        lastResultCode = resultCode;
    }
};

class ModularObjectAbilityConnectionTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override
    {
        ModularObjectConnectionManager::Reset();
    }
    void TearDown() override {}
};

// ==================== OnAbilityConnectDone ====================

HWTEST_F(ModularObjectAbilityConnectionTest, OnAbilityConnectDone_EmptyCallbackList_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnAbilityConnectDone_EmptyCallbackList_001 start";
    auto conn = sptr<ModularObjectAbilityConnection>(new ModularObjectAbilityConnection());
    AppExecFwk::ElementName element("device", "bundle", "module", "ability");
    sptr<IRemoteObject> remote;
    conn->OnAbilityConnectDone(element, remote, 0);
    // Should return early without crash
    EXPECT_EQ(conn->GetConnectionState(), CONNECTION_STATE_DISCONNECTED);
    GTEST_LOG_(INFO) << "OnAbilityConnectDone_EmptyCallbackList_001 end";
}

HWTEST_F(ModularObjectAbilityConnectionTest, OnAbilityConnectDone_WithCallback_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnAbilityConnectDone_WithCallback_001 start";
    auto conn = sptr<ModularObjectAbilityConnection>(new ModularObjectAbilityConnection());
    auto cb = sptr<MockConnectCallback>(new MockConnectCallback());
    conn->AddConnectCallback(cb);

    AppExecFwk::ElementName element("device", "bundle", "module", "ability");
    sptr<IRemoteObject> remote;
    conn->OnAbilityConnectDone(element, remote, 0);

    EXPECT_TRUE(cb->connectCalled);
    EXPECT_EQ(conn->GetConnectionState(), CONNECTION_STATE_CONNECTED);
    GTEST_LOG_(INFO) << "OnAbilityConnectDone_WithCallback_001 end";
}

HWTEST_F(ModularObjectAbilityConnectionTest, OnAbilityConnectDone_SetsResultCode_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnAbilityConnectDone_SetsResultCode_001 start";
    auto conn = sptr<ModularObjectAbilityConnection>(new ModularObjectAbilityConnection());
    auto cb = sptr<MockConnectCallback>(new MockConnectCallback());
    conn->AddConnectCallback(cb);

    AppExecFwk::ElementName element("device", "bundle", "module", "ability");
    sptr<IRemoteObject> remote;
    conn->OnAbilityConnectDone(element, remote, 42);

    EXPECT_EQ(cb->lastResultCode, 42);
    EXPECT_EQ(conn->GetResultCode(), 42);
    GTEST_LOG_(INFO) << "OnAbilityConnectDone_SetsResultCode_001 end";
}

HWTEST_F(ModularObjectAbilityConnectionTest, OnAbilityConnectDone_SetsRemoteObject_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnAbilityConnectDone_SetsRemoteObject_001 start";
    auto conn = sptr<ModularObjectAbilityConnection>(new ModularObjectAbilityConnection());
    auto cb = sptr<MockConnectCallback>(new MockConnectCallback());
    conn->AddConnectCallback(cb);

    AppExecFwk::ElementName element("device", "bundle", "module", "ability");
    sptr<IRemoteObject> remote;
    conn->OnAbilityConnectDone(element, remote, 0);

    EXPECT_EQ(conn->GetRemoteObject(), remote);
    GTEST_LOG_(INFO) << "OnAbilityConnectDone_SetsRemoteObject_001 end";
}

HWTEST_F(ModularObjectAbilityConnectionTest, OnAbilityConnectDone_DisconnectNonexistent_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnAbilityConnectDone_DisconnectNonexistent_001 start";
    auto conn = sptr<ModularObjectAbilityConnection>(new ModularObjectAbilityConnection());
    auto cb = sptr<MockConnectCallback>(new MockConnectCallback());
    conn->AddConnectCallback(cb);
    ModularObjectConnectionManager::g_disconnectNonexistentResult = true;

    AppExecFwk::ElementName element("device", "bundle", "module", "ability");
    sptr<IRemoteObject> remote;
    conn->OnAbilityConnectDone(element, remote, 0);

    // Should NOT call callback because DisconnectNonexistentService returns true
    EXPECT_FALSE(cb->connectCalled);
    GTEST_LOG_(INFO) << "OnAbilityConnectDone_DisconnectNonexistent_001 end";
}

HWTEST_F(ModularObjectAbilityConnectionTest, OnAbilityConnectDone_MultipleCallbacks_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnAbilityConnectDone_MultipleCallbacks_001 start";
    auto conn = sptr<ModularObjectAbilityConnection>(new ModularObjectAbilityConnection());
    auto cb1 = sptr<MockConnectCallback>(new MockConnectCallback());
    auto cb2 = sptr<MockConnectCallback>(new MockConnectCallback());
    conn->AddConnectCallback(cb1);
    conn->AddConnectCallback(cb2);

    AppExecFwk::ElementName element("device", "bundle", "module", "ability");
    sptr<IRemoteObject> remote;
    conn->OnAbilityConnectDone(element, remote, 0);

    EXPECT_TRUE(cb1->connectCalled);
    EXPECT_TRUE(cb2->connectCalled);
    GTEST_LOG_(INFO) << "OnAbilityConnectDone_MultipleCallbacks_001 end";
}

// ==================== OnAbilityDisconnectDone ====================

HWTEST_F(ModularObjectAbilityConnectionTest, OnAbilityDisconnectDone_EmptyCallbackList_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnAbilityDisconnectDone_EmptyCallbackList_001 start";
    auto conn = sptr<ModularObjectAbilityConnection>(new ModularObjectAbilityConnection());
    AppExecFwk::ElementName element("device", "bundle", "module", "ability");
    conn->OnAbilityDisconnectDone(element, 0);
    EXPECT_EQ(conn->GetConnectionState(), CONNECTION_STATE_DISCONNECTED);
    GTEST_LOG_(INFO) << "OnAbilityDisconnectDone_EmptyCallbackList_001 end";
}

HWTEST_F(ModularObjectAbilityConnectionTest, OnAbilityDisconnectDone_WithCallback_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnAbilityDisconnectDone_WithCallback_001 start";
    auto conn = sptr<ModularObjectAbilityConnection>(new ModularObjectAbilityConnection());
    auto cb = sptr<MockConnectCallback>(new MockConnectCallback());
    conn->AddConnectCallback(cb);

    AppExecFwk::ElementName element("device", "bundle", "module", "ability");
    conn->OnAbilityDisconnectDone(element, 0);

    EXPECT_TRUE(cb->disconnectCalled);
    EXPECT_EQ(conn->GetConnectionState(), CONNECTION_STATE_DISCONNECTED);
    GTEST_LOG_(INFO) << "OnAbilityDisconnectDone_WithCallback_001 end";
}

HWTEST_F(ModularObjectAbilityConnectionTest, OnAbilityDisconnectDone_SetsDisconnectedState_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnAbilityDisconnectDone_SetsDisconnectedState_001 start";
    auto conn = sptr<ModularObjectAbilityConnection>(new ModularObjectAbilityConnection());
    auto cb = sptr<MockConnectCallback>(new MockConnectCallback());
    conn->AddConnectCallback(cb);

    // First connect
    AppExecFwk::ElementName element("device", "bundle", "module", "ability");
    sptr<IRemoteObject> remote;
    conn->OnAbilityConnectDone(element, remote, 0);
    EXPECT_EQ(conn->GetConnectionState(), CONNECTION_STATE_CONNECTED);

    // Then disconnect
    conn->OnAbilityDisconnectDone(element, 0);
    EXPECT_EQ(conn->GetConnectionState(), CONNECTION_STATE_DISCONNECTED);
    GTEST_LOG_(INFO) << "OnAbilityDisconnectDone_SetsDisconnectedState_001 end";
}

HWTEST_F(ModularObjectAbilityConnectionTest, OnAbilityDisconnectDone_DiedResultCode_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnAbilityDisconnectDone_DiedResultCode_001 start";
    auto conn = sptr<ModularObjectAbilityConnection>(new ModularObjectAbilityConnection());
    auto cb = sptr<MockConnectCallback>(new MockConnectCallback());
    conn->AddConnectCallback(cb);

    AppExecFwk::ElementName element("device", "bundle", "module", "ability");
    conn->OnAbilityDisconnectDone(element, -1); // DIED

    // DIED (-1) triggers RemoveConnection and changes resultCode to 0
    EXPECT_TRUE(ModularObjectConnectionManager::g_removeConnectionCalled);
    EXPECT_EQ(cb->lastResultCode, 0); // DIED + 1 = 0
    GTEST_LOG_(INFO) << "OnAbilityDisconnectDone_DiedResultCode_001 end";
}

HWTEST_F(ModularObjectAbilityConnectionTest, OnAbilityDisconnectDone_NormalResultCode_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnAbilityDisconnectDone_NormalResultCode_001 start";
    auto conn = sptr<ModularObjectAbilityConnection>(new ModularObjectAbilityConnection());
    auto cb = sptr<MockConnectCallback>(new MockConnectCallback());
    conn->AddConnectCallback(cb);

    AppExecFwk::ElementName element("device", "bundle", "module", "ability");
    conn->OnAbilityDisconnectDone(element, 5);

    EXPECT_FALSE(ModularObjectConnectionManager::g_removeConnectionCalled);
    EXPECT_EQ(cb->lastResultCode, 5);
    GTEST_LOG_(INFO) << "OnAbilityDisconnectDone_NormalResultCode_001 end";
}

HWTEST_F(ModularObjectAbilityConnectionTest, OnAbilityDisconnectDone_ClearsRemoteObject_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnAbilityDisconnectDone_ClearsRemoteObject_001 start";
    auto conn = sptr<ModularObjectAbilityConnection>(new ModularObjectAbilityConnection());
    auto cb = sptr<MockConnectCallback>(new MockConnectCallback());
    conn->AddConnectCallback(cb);

    AppExecFwk::ElementName element("device", "bundle", "module", "ability");
    sptr<IRemoteObject> remote;
    conn->OnAbilityConnectDone(element, remote, 0);

    conn->OnAbilityDisconnectDone(element, 0);
    EXPECT_EQ(conn->GetRemoteObject(), nullptr);
    GTEST_LOG_(INFO) << "OnAbilityDisconnectDone_ClearsRemoteObject_001 end";
}

HWTEST_F(ModularObjectAbilityConnectionTest, OnAbilityDisconnectDone_MultipleCallbacks_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnAbilityDisconnectDone_MultipleCallbacks_001 start";
    auto conn = sptr<ModularObjectAbilityConnection>(new ModularObjectAbilityConnection());
    auto cb1 = sptr<MockConnectCallback>(new MockConnectCallback());
    auto cb2 = sptr<MockConnectCallback>(new MockConnectCallback());
    conn->AddConnectCallback(cb1);
    conn->AddConnectCallback(cb2);

    AppExecFwk::ElementName element("device", "bundle", "module", "ability");
    conn->OnAbilityDisconnectDone(element, 0);

    EXPECT_TRUE(cb1->disconnectCalled);
    EXPECT_TRUE(cb2->disconnectCalled);
    GTEST_LOG_(INFO) << "OnAbilityDisconnectDone_MultipleCallbacks_001 end";
}

} // namespace AbilityRuntime
} // namespace OHOS
