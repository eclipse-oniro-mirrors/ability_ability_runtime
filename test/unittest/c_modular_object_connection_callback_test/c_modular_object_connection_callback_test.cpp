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

#include "c_modular_object_connection_callback.h"
#include "c_modular_object_utils.h"
#include "connect_options_impl.h"
#include "ipc_inner_object.h"

using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {

// Static mock state definitions
bool CModularObjectUtils::buildElementResult = true;
bool CModularObjectUtils::notifyFailedCalled = false;
int32_t CModularObjectUtils::notifyFailedCode = 0;
int32_t CModularObjectUtils::convertConnectResult = 0;

namespace {
int32_t g_connectCallbackCount = 0;
int32_t g_disconnectCallbackCount = 0;
int32_t g_failedCallbackCount = 0;

void ResetCallbackState()
{
    g_connectCallbackCount = 0;
    g_disconnectCallbackCount = 0;
    g_failedCallbackCount = 0;
    CModularObjectUtils::buildElementResult = true;
    CModularObjectUtils::notifyFailedCalled = false;
    CModularObjectUtils::notifyFailedCode = 0;
    CModularObjectUtils::convertConnectResult = 0;
}

void MockOnConnectCallback(OH_AbilityRuntime_ConnectOptions *owner,
    AbilityBase_Element *element, void *proxy)
{
    (void)owner;
    (void)proxy;
    g_connectCallbackCount++;
}

void MockOnDisconnectCallback(OH_AbilityRuntime_ConnectOptions *owner,
    AbilityBase_Element *element)
{
    (void)owner;
    (void)element;
    g_disconnectCallbackCount++;
}

class MockRemoteObject : public IRemoteObject {
public:
    MockRemoteObject() : IRemoteObject(u"mock_descriptor") {}
    ~MockRemoteObject() = default;
    int32_t GetObjectRefCount() override { return 0; }
    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
        MessageOption &option) override { return 0; }
    bool IsProxyObject() const override { return true; }
    bool CheckObjectLegality() const override { return true; }
    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) override { return true; }
    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) override { return true; }
    bool Marshalling(Parcel &parcel) const override { return true; }
    sptr<IRemoteBroker> AsInterface() override { return nullptr; }
    int Dump(int fd, const std::vector<std::u16string> &args) override { return 0; }
};
} // namespace

class CModularObjectConnectionCallbackTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override { ResetCallbackState(); }
    void TearDown() override {}
};

// ==================== InsertConnection ====================

HWTEST_F(CModularObjectConnectionCallbackTest, InsertConnection_NullCallback_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InsertConnection_NullCallback_001 start";
    auto ret = CModularObjectConnectionUtils::InsertConnection(nullptr);
    EXPECT_EQ(ret, -1);
    GTEST_LOG_(INFO) << "InsertConnection_NullCallback_001 end";
}

HWTEST_F(CModularObjectConnectionCallbackTest, InsertConnection_ValidCallback_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InsertConnection_ValidCallback_001 start";
    auto state = std::make_shared<OH_AbilityRuntime_ConnectOptionsState>();
    auto callback = sptr<CModularObjectConnectionCallback>::MakeSptr(state);
    ASSERT_NE(callback, nullptr);
    auto ret = CModularObjectConnectionUtils::InsertConnection(callback);
    EXPECT_GE(ret, 0);
    EXPECT_EQ(callback->GetConnectionId(), ret);
    // Clean up
    CModularObjectConnectionUtils::RemoveConnectionCallback(ret);
    GTEST_LOG_(INFO) << "InsertConnection_ValidCallback_001 end";
}

HWTEST_F(CModularObjectConnectionCallbackTest, InsertConnection_Multiple_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InsertConnection_Multiple_001 start";
    auto state1 = std::make_shared<OH_AbilityRuntime_ConnectOptionsState>();
    auto cb1 = sptr<CModularObjectConnectionCallback>::MakeSptr(state1);
    auto state2 = std::make_shared<OH_AbilityRuntime_ConnectOptionsState>();
    auto cb2 = sptr<CModularObjectConnectionCallback>::MakeSptr(state2);
    auto id1 = CModularObjectConnectionUtils::InsertConnection(cb1);
    auto id2 = CModularObjectConnectionUtils::InsertConnection(cb2);
    EXPECT_GE(id1, 0);
    EXPECT_GE(id2, 0);
    EXPECT_NE(id1, id2);
    // Clean up
    CModularObjectConnectionUtils::RemoveConnectionCallback(id1);
    CModularObjectConnectionUtils::RemoveConnectionCallback(id2);
    GTEST_LOG_(INFO) << "InsertConnection_Multiple_001 end";
}

// ==================== FindConnection ====================

HWTEST_F(CModularObjectConnectionCallbackTest, FindConnection_NotFound_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindConnection_NotFound_001 start";
    sptr<CModularObjectConnectionCallback> callback;
    CModularObjectConnectionUtils::FindConnection(-1, callback);
    EXPECT_EQ(callback, nullptr);
    GTEST_LOG_(INFO) << "FindConnection_NotFound_001 end";
}

HWTEST_F(CModularObjectConnectionCallbackTest, FindConnection_Found_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindConnection_Found_001 start";
    auto state = std::make_shared<OH_AbilityRuntime_ConnectOptionsState>();
    auto cb = sptr<CModularObjectConnectionCallback>::MakeSptr(state);
    auto id = CModularObjectConnectionUtils::InsertConnection(cb);
    sptr<CModularObjectConnectionCallback> found;
    CModularObjectConnectionUtils::FindConnection(id, found);
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->GetConnectionId(), id);
    // Clean up
    CModularObjectConnectionUtils::RemoveConnectionCallback(id);
    GTEST_LOG_(INFO) << "FindConnection_Found_001 end";
}

// ==================== RemoveConnectionCallback ====================

HWTEST_F(CModularObjectConnectionCallbackTest, RemoveConnection_Valid_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemoveConnection_Valid_001 start";
    auto state = std::make_shared<OH_AbilityRuntime_ConnectOptionsState>();
    auto cb = sptr<CModularObjectConnectionCallback>::MakeSptr(state);
    auto id = CModularObjectConnectionUtils::InsertConnection(cb);
    CModularObjectConnectionUtils::RemoveConnectionCallback(id);
    // Verify removed
    sptr<CModularObjectConnectionCallback> found;
    CModularObjectConnectionUtils::FindConnection(id, found);
    EXPECT_EQ(found, nullptr);
    GTEST_LOG_(INFO) << "RemoveConnection_Valid_001 end";
}

// ==================== OnAbilityConnectDone ====================

HWTEST_F(CModularObjectConnectionCallbackTest, OnConnectDone_NullRemote_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnConnectDone_NullRemote_001 start";
    auto state = std::make_shared<OH_AbilityRuntime_ConnectOptionsState>();
    state->alive = true;
    state->onConnectCallback = MockOnConnectCallback;
    auto cb = sptr<CModularObjectConnectionCallback>::MakeSptr(state);
    AppExecFwk::ElementName element("", "com.test", "Ability");
    cb->OnAbilityConnectDone(element, nullptr, 0);
    EXPECT_EQ(g_connectCallbackCount, 0);
    GTEST_LOG_(INFO) << "OnConnectDone_NullRemote_001 end";
}

HWTEST_F(CModularObjectConnectionCallbackTest, OnConnectDone_ExpiredState_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnConnectDone_ExpiredState_001 start";
    auto state = std::make_shared<OH_AbilityRuntime_ConnectOptionsState>();
    state->alive = true;
    state->onConnectCallback = MockOnConnectCallback;
    auto cb = sptr<CModularObjectConnectionCallback>::MakeSptr(state);
    // Reset the shared_ptr so state_ becomes expired
    state.reset();
    sptr<IRemoteObject> remote = sptr<IRemoteObject>(new MockRemoteObject());
    AppExecFwk::ElementName element("", "com.test", "Ability");
    cb->OnAbilityConnectDone(element, remote, 0);
    EXPECT_EQ(g_connectCallbackCount, 0);
    GTEST_LOG_(INFO) << "OnConnectDone_ExpiredState_001 end";
}

HWTEST_F(CModularObjectConnectionCallbackTest, OnConnectDone_NotAlive_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnConnectDone_NotAlive_001 start";
    auto state = std::make_shared<OH_AbilityRuntime_ConnectOptionsState>();
    state->alive = false;
    state->onConnectCallback = MockOnConnectCallback;
    auto cb = sptr<CModularObjectConnectionCallback>::MakeSptr(state);
    sptr<IRemoteObject> remote = sptr<IRemoteObject>(new MockRemoteObject());
    AppExecFwk::ElementName element("", "com.test", "Ability");
    cb->OnAbilityConnectDone(element, remote, 0);
    EXPECT_EQ(g_connectCallbackCount, 0);
    GTEST_LOG_(INFO) << "OnConnectDone_NotAlive_001 end";
}

HWTEST_F(CModularObjectConnectionCallbackTest, OnConnectDone_NullCallback_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnConnectDone_NullCallback_001 start";
    auto state = std::make_shared<OH_AbilityRuntime_ConnectOptionsState>();
    state->alive = true;
    state->onConnectCallback = nullptr;
    auto cb = sptr<CModularObjectConnectionCallback>::MakeSptr(state);
    sptr<IRemoteObject> remote = sptr<IRemoteObject>(new MockRemoteObject());
    AppExecFwk::ElementName element("", "com.test", "Ability");
    cb->OnAbilityConnectDone(element, remote, 0);
    EXPECT_EQ(g_connectCallbackCount, 0);
    GTEST_LOG_(INFO) << "OnConnectDone_NullCallback_001 end";
}

HWTEST_F(CModularObjectConnectionCallbackTest, OnConnectDone_BuildElementFails_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnConnectDone_BuildElementFails_001 start";
    auto state = std::make_shared<OH_AbilityRuntime_ConnectOptionsState>();
    state->alive = true;
    state->onConnectCallback = MockOnConnectCallback;
    CModularObjectUtils::buildElementResult = false;
    auto cb = sptr<CModularObjectConnectionCallback>::MakeSptr(state);
    sptr<IRemoteObject> remote = sptr<IRemoteObject>(new MockRemoteObject());
    AppExecFwk::ElementName element("", "com.test", "Ability");
    cb->OnAbilityConnectDone(element, remote, 0);
    EXPECT_EQ(g_connectCallbackCount, 0);
    EXPECT_TRUE(CModularObjectUtils::notifyFailedCalled);
    GTEST_LOG_(INFO) << "OnConnectDone_BuildElementFails_001 end";
}

HWTEST_F(CModularObjectConnectionCallbackTest, OnConnectDone_Success_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnConnectDone_Success_001 start";
    OH_AbilityRuntime_ConnectOptions owner;
    auto state = std::make_shared<OH_AbilityRuntime_ConnectOptionsState>();
    state->alive = true;
    state->owner = &owner;
    state->onConnectCallback = MockOnConnectCallback;
    auto cb = sptr<CModularObjectConnectionCallback>::MakeSptr(state);
    sptr<IRemoteObject> remote = sptr<IRemoteObject>(new MockRemoteObject());
    AppExecFwk::ElementName element("", "com.test", "Ability");
    cb->OnAbilityConnectDone(element, remote, 0);
    EXPECT_EQ(g_connectCallbackCount, 1);
    GTEST_LOG_(INFO) << "OnConnectDone_Success_001 end";
}

// ==================== OnAbilityDisconnectDone ====================

HWTEST_F(CModularObjectConnectionCallbackTest, OnDisconnectDone_ExpiredState_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnDisconnectDone_ExpiredState_001 start";
    auto state = std::make_shared<OH_AbilityRuntime_ConnectOptionsState>();
    state->alive = true;
    state->onDisconnectCallback = MockOnDisconnectCallback;
    auto cb = sptr<CModularObjectConnectionCallback>::MakeSptr(state);
    state.reset(); // expire the weak_ptr
    AppExecFwk::ElementName element("", "com.test", "Ability");
    cb->OnAbilityDisconnectDone(element, 0);
    EXPECT_EQ(g_disconnectCallbackCount, 0);
    GTEST_LOG_(INFO) << "OnDisconnectDone_ExpiredState_001 end";
}

HWTEST_F(CModularObjectConnectionCallbackTest, OnDisconnectDone_NotAlive_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnDisconnectDone_NotAlive_001 start";
    auto state = std::make_shared<OH_AbilityRuntime_ConnectOptionsState>();
    state->alive = false;
    state->onDisconnectCallback = MockOnDisconnectCallback;
    auto cb = sptr<CModularObjectConnectionCallback>::MakeSptr(state);
    sptr<IRemoteObject> remote = sptr<IRemoteObject>(new MockRemoteObject());
    AppExecFwk::ElementName element("", "com.test", "Ability");
    cb->OnAbilityDisconnectDone(element, 0);
    EXPECT_EQ(g_disconnectCallbackCount, 0);
    GTEST_LOG_(INFO) << "OnDisconnectDone_NotAlive_001 end";
}

HWTEST_F(CModularObjectConnectionCallbackTest, OnDisconnectDone_NullCallback_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnDisconnectDone_NullCallback_001 start";
    auto state = std::make_shared<OH_AbilityRuntime_ConnectOptionsState>();
    state->alive = true;
    state->onDisconnectCallback = nullptr;
    auto cb = sptr<CModularObjectConnectionCallback>::MakeSptr(state);
    AppExecFwk::ElementName element("", "com.test", "Ability");
    cb->OnAbilityDisconnectDone(element, 0);
    EXPECT_EQ(g_disconnectCallbackCount, 0);
    GTEST_LOG_(INFO) << "OnDisconnectDone_NullCallback_001 end";
}

HWTEST_F(CModularObjectConnectionCallbackTest, OnDisconnectDone_Success_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnDisconnectDone_Success_001 start";
    OH_AbilityRuntime_ConnectOptions owner;
    auto state = std::make_shared<OH_AbilityRuntime_ConnectOptionsState>();
    state->alive = true;
    state->owner = &owner;
    state->onDisconnectCallback = MockOnDisconnectCallback;
    auto cb = sptr<CModularObjectConnectionCallback>::MakeSptr(state);
    AppExecFwk::ElementName element("", "com.test", "Ability");
    cb->OnAbilityDisconnectDone(element, 0);
    EXPECT_EQ(g_disconnectCallbackCount, 1);
    GTEST_LOG_(INFO) << "OnDisconnectDone_Success_001 end";
}

HWTEST_F(CModularObjectConnectionCallbackTest, OnDisconnectDone_BuildElementFails_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnDisconnectDone_BuildElementFails_001 start";
    auto state = std::make_shared<OH_AbilityRuntime_ConnectOptionsState>();
    state->alive = true;
    state->onDisconnectCallback = MockOnDisconnectCallback;
    CModularObjectUtils::buildElementResult = false;
    auto cb = sptr<CModularObjectConnectionCallback>::MakeSptr(state);
    AppExecFwk::ElementName element("", "com.test", "Ability");
    cb->OnAbilityDisconnectDone(element, 0);
    EXPECT_EQ(g_disconnectCallbackCount, 0);
    EXPECT_TRUE(CModularObjectUtils::notifyFailedCalled);
    GTEST_LOG_(INFO) << "OnDisconnectDone_BuildElementFails_001 end";
}

// ==================== SetConnectionId / GetConnectionId ====================

HWTEST_F(CModularObjectConnectionCallbackTest, ConnectionId_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConnectionId_001 start";
    auto state = std::make_shared<OH_AbilityRuntime_ConnectOptionsState>();
    auto cb = sptr<CModularObjectConnectionCallback>::MakeSptr(state);
    EXPECT_EQ(cb->GetConnectionId(), 0);
    cb->SetConnectionId(42);
    EXPECT_EQ(cb->GetConnectionId(), 42);
    GTEST_LOG_(INFO) << "ConnectionId_001 end";
}

} // namespace AbilityRuntime
} // namespace OHOS
