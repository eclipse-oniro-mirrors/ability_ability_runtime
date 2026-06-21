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

#include <cstdint>
#include <vector>

#include <gtest/gtest.h>

#include "agent_remote_object_key.h"
#include "ipc_object_proxy.h"
#include "ipc_object_stub.h"
#include "iremote_object.h"
#include "message_option.h"
#include "message_parcel.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AgentRuntime;

class AgentRemoteObjectKeyTest : public testing::Test {
public:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name      NullRemoteObjectReturnsEmptyKey
 * @tc.desc      A null remote object must produce an all-zero key.
 */
HWTEST_F(AgentRemoteObjectKeyTest, NullRemoteObjectReturnsEmptyKey, TestSize.Level1)
{
    AgentRemoteObjectKey key = BuildAgentRemoteObjectKey(nullptr);
    EXPECT_EQ(key.handle, 0u);
    EXPECT_EQ(key.localObject, reinterpret_cast<uintptr_t>(nullptr));
}

/**
 * @tc.name      ProxyRemoteObjectPopulatesHandle
 * @tc.desc      A proxy object must populate handle and leave localObject as zero.
 */
HWTEST_F(AgentRemoteObjectKeyTest, ProxyRemoteObjectPopulatesHandle, TestSize.Level1)
{
    constexpr uint32_t testHandle = 42;
    sptr<IPCObjectProxy> proxy = new IPCObjectProxy(testHandle);
    ASSERT_NE(proxy, nullptr);
    EXPECT_TRUE(proxy->IsProxyObject());

    AgentRemoteObjectKey key = BuildAgentRemoteObjectKey(proxy);
    EXPECT_EQ(key.handle, testHandle);
    EXPECT_EQ(key.localObject, 0u);
}

/**
 * @tc.name      ProxyRemoteObjectZeroHandle
 * @tc.desc      A proxy with the default (zero) handle reports handle == 0 and localObject == 0.
 */
HWTEST_F(AgentRemoteObjectKeyTest, ProxyRemoteObjectZeroHandle, TestSize.Level1)
{
    sptr<IPCObjectProxy> proxy = new IPCObjectProxy(0);
    ASSERT_NE(proxy, nullptr);

    AgentRemoteObjectKey key = BuildAgentRemoteObjectKey(proxy);
    EXPECT_EQ(key.handle, 0u);
    EXPECT_EQ(key.localObject, 0u);
}

/**
 * @tc.name      StubRemoteObjectPopulatesLocalPointer
 * @tc.desc      A non-proxy stub object must populate localObject and leave handle as zero.
 */
HWTEST_F(AgentRemoteObjectKeyTest, StubRemoteObjectPopulatesLocalPointer, TestSize.Level1)
{
    sptr<IPCObjectStub> stub = new IPCObjectStub(u"test_stub");
    ASSERT_NE(stub, nullptr);
    EXPECT_FALSE(stub->IsProxyObject());

    AgentRemoteObjectKey key = BuildAgentRemoteObjectKey(stub);
    EXPECT_EQ(key.handle, 0u);
    EXPECT_EQ(key.localObject, reinterpret_cast<uintptr_t>(stub.GetRefPtr()));
}

/**
 * @tc.name      DistinctStubsProduceDistinctLocalPointers
 * @tc.desc      Two distinct stub instances must yield distinct local-object key values.
 */
HWTEST_F(AgentRemoteObjectKeyTest, DistinctStubsProduceDistinctLocalPointers, TestSize.Level1)
{
    sptr<IPCObjectStub> stubA = new IPCObjectStub(u"test_stub_a");
    sptr<IPCObjectStub> stubB = new IPCObjectStub(u"test_stub_b");
    ASSERT_NE(stubA.GetRefPtr(), stubB.GetRefPtr());

    AgentRemoteObjectKey keyA = BuildAgentRemoteObjectKey(stubA);
    AgentRemoteObjectKey keyB = BuildAgentRemoteObjectKey(stubB);
    EXPECT_NE(keyA.localObject, keyB.localObject);
}

/**
 * @tc.name      KeyOrderingByHandle
 * @tc.desc      AgentRemoteObjectKey::operator< orders by handle first.
 */
HWTEST_F(AgentRemoteObjectKeyTest, KeyOrderingByHandle, TestSize.Level1)
{
    AgentRemoteObjectKey low { 1, 0 };
    AgentRemoteObjectKey high { 2, 0 };
    EXPECT_TRUE(low < high);
    EXPECT_FALSE(high < low);
    EXPECT_FALSE(low < low);
}

/**
 * @tc.name      KeyOrderingByLocalObject
 * @tc.desc      AgentRemoteObjectKey::operator< falls back to localObject when handles tie.
 */
HWTEST_F(AgentRemoteObjectKeyTest, KeyOrderingByLocalObject, TestSize.Level1)
{
    AgentRemoteObjectKey low { 7, 100 };
    AgentRemoteObjectKey high { 7, 200 };
    EXPECT_TRUE(low < high);
    EXPECT_FALSE(high < low);
}
