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

#include "modular_object_connection_manager.h"
#include "modular_object_ability_connection.h"
#include "ability_manager_client.h"

using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
bool AbilityManagerClient::g_connectCalled = false;
bool AbilityManagerClient::g_disconnectCalled = false;
ErrCode AbilityManagerClient::g_connectResult = ERR_OK;
ErrCode AbilityManagerClient::g_disconnectResult = ERR_OK;
} // namespace AAFwk

namespace AbilityRuntime {

// Provide stub implementations for virtual methods (avoid linking modular_object_ability_connection.cpp)
void ModularObjectAbilityConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) {}

void ModularObjectAbilityConnection::OnAbilityDisconnectDone(
    const AppExecFwk::ElementName &element, int resultCode) {}

namespace {

class MockConnectCallback : public AbilityConnectCallback {
public:
    void OnAbilityConnectDone(const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject, int resultCode) override {}
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override {}
};

AAFwk::Want BuildWant(const std::string &bundle, const std::string &module, const std::string &ability)
{
    AAFwk::Want want;
    AAFwk::Operation op;
    op.SetBundleName(bundle);
    op.SetModuleName(module);
    op.SetAbilityName(ability);
    want.SetOperation(op);
    return want;
}

} // namespace

class ModularObjectConnectionManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() override
    {
        AAFwk::AbilityManagerClient::Reset();
        // Clean up any leftover state by disconnecting
        auto &mgr = ModularObjectConnectionManager::GetInstance();
        auto cb1 = sptr<MockConnectCallback>::MakeSptr();
        mgr.DisconnectModularObjectExtension(cb1);
    }
    void TearDown() override {}
};

// ==================== ConnectModularObjectExtension ====================

HWTEST_F(ModularObjectConnectionManagerTest, Connect_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Connect_001 start";
    auto &mgr = ModularObjectConnectionManager::GetInstance();
    auto ret = mgr.ConnectModularObjectExtension(AAFwk::Want(), nullptr);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "Connect_001 end";
}

HWTEST_F(ModularObjectConnectionManagerTest, Connect_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Connect_002 start";
    auto &mgr = ModularObjectConnectionManager::GetInstance();
    AAFwk::AbilityManagerClient::g_connectResult = -1;
    auto callback = sptr<MockConnectCallback>::MakeSptr();
    auto ret = mgr.ConnectModularObjectExtension(BuildWant("b", "m", "a"), callback);
    EXPECT_EQ(ret, -1);
    AAFwk::AbilityManagerClient::g_connectResult = ERR_OK;
    GTEST_LOG_(INFO) << "Connect_002 end";
}

HWTEST_F(ModularObjectConnectionManagerTest, Connect_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Connect_003 start";
    auto &mgr = ModularObjectConnectionManager::GetInstance();
    AAFwk::AbilityManagerClient::Reset();
    auto callback = sptr<MockConnectCallback>::MakeSptr();
    auto ret = mgr.ConnectModularObjectExtension(BuildWant("b", "m", "a"), callback);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_TRUE(AAFwk::AbilityManagerClient::g_connectCalled);
    // Clean up
    mgr.DisconnectModularObjectExtension(callback);
    GTEST_LOG_(INFO) << "Connect_003 end";
}

// ==================== DisconnectModularObjectExtension ====================

HWTEST_F(ModularObjectConnectionManagerTest, Disconnect_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Disconnect_001 start";
    auto &mgr = ModularObjectConnectionManager::GetInstance();
    auto ret = mgr.DisconnectModularObjectExtension(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "Disconnect_001 end";
}

HWTEST_F(ModularObjectConnectionManagerTest, Disconnect_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Disconnect_002 start";
    auto &mgr = ModularObjectConnectionManager::GetInstance();
    AAFwk::AbilityManagerClient::Reset();
    auto callback = sptr<MockConnectCallback>::MakeSptr();
    auto ret = mgr.DisconnectModularObjectExtension(callback);
    EXPECT_EQ(ret, AAFwk::CONNECTION_NOT_EXIST);
    GTEST_LOG_(INFO) << "Disconnect_002 end";
}

HWTEST_F(ModularObjectConnectionManagerTest, Disconnect_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Disconnect_003 start";
    auto &mgr = ModularObjectConnectionManager::GetInstance();
    AAFwk::AbilityManagerClient::Reset();
    auto callback = sptr<MockConnectCallback>::MakeSptr();
    // Connect first
    auto ret = mgr.ConnectModularObjectExtension(BuildWant("b2", "m2", "a2"), callback);
    EXPECT_EQ(ret, ERR_OK);
    // Disconnect
    AAFwk::AbilityManagerClient::Reset();
    ret = mgr.DisconnectModularObjectExtension(callback);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_TRUE(AAFwk::AbilityManagerClient::g_disconnectCalled);
    GTEST_LOG_(INFO) << "Disconnect_003 end";
}

HWTEST_F(ModularObjectConnectionManagerTest, Disconnect_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Disconnect_004 start";
    auto &mgr = ModularObjectConnectionManager::GetInstance();
    AAFwk::AbilityManagerClient::Reset();
    auto callback1 = sptr<MockConnectCallback>::MakeSptr();
    auto callback2 = sptr<MockConnectCallback>::MakeSptr();
    // Connect same want twice with different callbacks
    auto ret = mgr.ConnectModularObjectExtension(BuildWant("b3", "m3", "a3"), callback1);
    EXPECT_EQ(ret, ERR_OK);
    ret = mgr.ConnectModularObjectExtension(BuildWant("b3", "m3", "a3"), callback2);
    EXPECT_EQ(ret, ERR_OK);
    // Disconnect first callback - record should remain
    AAFwk::AbilityManagerClient::Reset();
    ret = mgr.DisconnectModularObjectExtension(callback1);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_TRUE(AAFwk::AbilityManagerClient::g_disconnectCalled);
    // Clean up
    mgr.DisconnectModularObjectExtension(callback2);
    GTEST_LOG_(INFO) << "Disconnect_004 end";
}

HWTEST_F(ModularObjectConnectionManagerTest, Disconnect_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Disconnect_005 start";
    auto &mgr = ModularObjectConnectionManager::GetInstance();
    AAFwk::AbilityManagerClient::g_disconnectResult = -2;
    auto callback = sptr<MockConnectCallback>::MakeSptr();
    // Connect first
    mgr.ConnectModularObjectExtension(BuildWant("b4", "m4", "a4"), callback);
    // Disconnect should return error
    auto ret = mgr.DisconnectModularObjectExtension(callback);
    EXPECT_EQ(ret, -2);
    AAFwk::AbilityManagerClient::g_disconnectResult = ERR_OK;
    GTEST_LOG_(INFO) << "Disconnect_005 end";
}

// ==================== RemoveConnection ====================

HWTEST_F(ModularObjectConnectionManagerTest, RemoveConnection_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemoveConnection_001 start";
    auto &mgr = ModularObjectConnectionManager::GetInstance();
    auto connection = sptr<ModularObjectAbilityConnection>::MakeSptr();
    auto ret = mgr.RemoveConnection(connection);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "RemoveConnection_001 end";
}

HWTEST_F(ModularObjectConnectionManagerTest, RemoveConnection_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemoveConnection_002 start";
    auto &mgr = ModularObjectConnectionManager::GetInstance();
    AAFwk::AbilityManagerClient::Reset();
    auto callback = sptr<MockConnectCallback>::MakeSptr();
    // Connect first
    auto ret = mgr.ConnectModularObjectExtension(BuildWant("b5", "m5", "a5"), callback);
    EXPECT_EQ(ret, ERR_OK);
    // RemoveConnection with wrong connection should return false
    auto wrongConnection = sptr<ModularObjectAbilityConnection>::MakeSptr();
    ret = mgr.RemoveConnection(wrongConnection);
    EXPECT_FALSE(ret);
    // Clean up
    mgr.DisconnectModularObjectExtension(callback);
    GTEST_LOG_(INFO) << "RemoveConnection_002 end";
}

// ==================== DisconnectNonexistentService ====================

HWTEST_F(ModularObjectConnectionManagerTest, DisconnectNonexistent_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DisconnectNonexistent_001 start";
    auto &mgr = ModularObjectConnectionManager::GetInstance();
    AAFwk::AbilityManagerClient::Reset();
    auto connection = sptr<ModularObjectAbilityConnection>::MakeSptr();
    AppExecFwk::ElementName element("", "com.test.noexist", "Ability");
    // No records exist, should disconnect
    auto ret = mgr.DisconnectNonexistentService(element, connection);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(AAFwk::AbilityManagerClient::g_disconnectCalled);
    GTEST_LOG_(INFO) << "DisconnectNonexistent_001 end";
}

HWTEST_F(ModularObjectConnectionManagerTest, DisconnectNonexistent_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DisconnectNonexistent_002 start";
    auto &mgr = ModularObjectConnectionManager::GetInstance();
    AAFwk::AbilityManagerClient::Reset();
    auto callback = sptr<MockConnectCallback>::MakeSptr();
    // Connect first
    auto want = BuildWant("com.test.exist", "module", "Ability");
    auto ret = mgr.ConnectModularObjectExtension(want, callback);
    EXPECT_EQ(ret, ERR_OK);
    // Check with matching bundle name - should find it (not disconnect)
    AppExecFwk::ElementName element("", "com.test.exist", "Ability");
    // We need the connection object used internally. RemoveConnection cleans up.
    // Since DisconnectNonexistentService checks by connection pointer AND bundleName,
    // passing a different connection won't match
    auto otherConnection = sptr<ModularObjectAbilityConnection>::MakeSptr();
    auto result = mgr.DisconnectNonexistentService(element, otherConnection);
    EXPECT_TRUE(result); // not found -> disconnect
    // Clean up
    mgr.DisconnectModularObjectExtension(callback);
    GTEST_LOG_(INFO) << "DisconnectNonexistent_002 end";
}

} // namespace AbilityRuntime
} // namespace OHOS
