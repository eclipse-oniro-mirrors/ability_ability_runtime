/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include <singleton.h>

#define private public
#define protected public
#include "ability_connection.h"
#undef private
#undef protected
#include "connection_manager.h"
#include "mock_ability_connection_callback.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AAFwk;

namespace OHOS {
namespace AbilityRuntime {
class AbilityConnectionTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
};

void AbilityConnectionTest::SetUpTestCase()
{}

void AbilityConnectionTest::TearDownTestCase()
{}

void AbilityConnectionTest::SetUp()
{}

void AbilityConnectionTest::TearDown()
{}

/**
 * @tc.number: AbilityRuntime_AbilityConnection_OnAbilityConnectDone_0100
 * @tc.name: OnAbilityConnectDone
 * @tc.desc: Test whether OnAbilityConnectDone is called normally(abilityConnectCallbackList_ is empty).
 */
HWTEST_F(AbilityConnectionTest, AbilityRuntime_AbilityConnection_OnAbilityConnectDone_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityConnection_OnAbilityConnectDone_0100 start";
    sptr<AbilityConnection> abilityConnection = new AbilityConnection();
    AppExecFwk::ElementName element;
    sptr<IRemoteObject> remoteObject = nullptr;
    EXPECT_NE(abilityConnection->GetConnectionState(), 0);
    abilityConnection->OnAbilityConnectDone(element, remoteObject, -1);
    EXPECT_EQ(abilityConnection->abilityConnectCallbackList_.size(), 0);
    EXPECT_NE(abilityConnection->GetConnectionState(), 0);
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityConnection_OnAbilityConnectDone_0100 end";
}

/**
 * @tc.number: AbilityRuntime_AbilityConnection_OnAbilityConnectDone_0200
 * @tc.name: OnAbilityConnectDone
 * @tc.desc: Test whether OnAbilityConnectDone is called normally(abilityConnectCallbackList_ is not empty).
 */
HWTEST_F(AbilityConnectionTest, AbilityRuntime_AbilityConnection_OnAbilityConnectDone_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityConnection_OnAbilityConnectDone_0200 start";
    sptr<AbilityConnection> abilityConnection = new AbilityConnection();
    AppExecFwk::ElementName element;
    sptr<IRemoteObject> remoteObject = nullptr;
    sptr<MockAbilityConnectCallback> mockAbilityConnectCallback(new MockAbilityConnectCallback());
    abilityConnection->AddConnectCallback(mockAbilityConnectCallback);
    EXPECT_NE(abilityConnection->GetConnectionState(), 0);
    abilityConnection->OnAbilityConnectDone(element, remoteObject, -1);
    EXPECT_NE(abilityConnection->abilityConnectCallbackList_.size(), 0);
    EXPECT_EQ(abilityConnection->GetConnectionState(), 0);
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityConnection_OnAbilityConnectDone_0200 end";
}

/**
 * @tc.number: AbilityRuntime_AbilityConnection_OnAbilityDisconnectDone_0100
 * @tc.name: OnAbilityDisconnectDone
 * @tc.desc: Test whether OnAbilityDisconnectDone is called normally(abilityConnectCallbackList_ is empty).
 */
HWTEST_F(AbilityConnectionTest, AbilityRuntime_AbilityConnection_OnAbilityDisconnectDone_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityConnection_OnAbilityDisconnectDone_0100 start";
    sptr<AbilityConnection> abilityConnection = new AbilityConnection();
    AppExecFwk::ElementName element;
    abilityConnection->OnAbilityDisconnectDone(element, -1);
    EXPECT_EQ(abilityConnection->abilityConnectCallbackList_.size(), 0);
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityConnection_OnAbilityDisconnectDone_0100 end";
}

/**
 * @tc.number: AbilityRuntime_AbilityConnection_OnAbilityDisconnectDone_0200
 * @tc.name: OnAbilityDisconnectDone
 * @tc.desc: Test whether OnAbilityDisconnectDone is called normally.
 *           (abilityConnectCallbackList_ is empty,resultCode != DIED)
 */
HWTEST_F(AbilityConnectionTest, AbilityRuntime_AbilityConnection_OnAbilityDisconnectDone_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityConnection_OnAbilityDisconnectDone_0200 start";
    sptr<AbilityConnection> abilityConnection = new AbilityConnection();
    AppExecFwk::ElementName element;
    EXPECT_EQ(abilityConnection->abilityConnectCallbackList_.size(), 0);
    sptr<MockAbilityConnectCallback> mockAbilityConnectCallback(new MockAbilityConnectCallback());
    abilityConnection->AddConnectCallback(mockAbilityConnectCallback);
    EXPECT_NE(abilityConnection->abilityConnectCallbackList_.size(), 0);
    int32_t resultCode = 0;
    abilityConnection->OnAbilityDisconnectDone(element, resultCode);
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityConnection_OnAbilityDisconnectDone_0200 end";
}

/**
 * @tc.number: AbilityRuntime_AbilityConnection_OnAbilityDisconnectDone_0300
 * @tc.name: OnAbilityDisconnectDone
 * @tc.desc: Test whether OnAbilityDisconnectDone is called normally.
 *           (abilityConnectCallbackList_ is empty,resultCode == DIED)
 */
HWTEST_F(AbilityConnectionTest, AbilityRuntime_AbilityConnection_OnAbilityDisconnectDone_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityConnection_OnAbilityDisconnectDone_0300 start";
    sptr<AbilityConnection> abilityConnection = new AbilityConnection();
    AppExecFwk::ElementName element;
    EXPECT_EQ(abilityConnection->abilityConnectCallbackList_.size(), 0);
    sptr<MockAbilityConnectCallback> mockAbilityConnectCallback(new MockAbilityConnectCallback());
    abilityConnection->AddConnectCallback(mockAbilityConnectCallback);
    EXPECT_NE(abilityConnection->abilityConnectCallbackList_.size(), 0);
    int32_t resultCode = -1;
    abilityConnection->OnAbilityDisconnectDone(element, resultCode);
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityConnection_OnAbilityDisconnectDone_0300 end";
}

/**
 * @tc.number: AbilityRuntime_AbilityConnection_AddConnectCallback_0100
 * @tc.name: AddConnectCallback
 * @tc.desc: Test whether AddConnectCallbackis called normally.
 */
HWTEST_F(AbilityConnectionTest, AbilityRuntime_AbilityConnection_AddConnectCallback_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityConnection_AddConnectCallback_0100 start";
    sptr<AbilityConnection> abilityConnection = new AbilityConnection();
    AppExecFwk::ElementName element;
    EXPECT_EQ(abilityConnection->abilityConnectCallbackList_.size(), 0);
    sptr<MockAbilityConnectCallback> mockAbilityConnectCallback(new MockAbilityConnectCallback());
    abilityConnection->AddConnectCallback(mockAbilityConnectCallback);
    EXPECT_NE(abilityConnection->abilityConnectCallbackList_.size(), 0);
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityConnection_AddConnectCallback_0100 end";
}

/**
 * @tc.number: AbilityRuntime_AbilityConnection_RemoveConnectCallback_0100
 * @tc.name: RemoveConnectCallback
 * @tc.desc: Test whether RemoveConnectCallback is called normally.
 */
HWTEST_F(AbilityConnectionTest, AbilityRuntime_AbilityConnection_RemoveConnectCallback_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityConnection_RemoveConnectCallback_0100 start";
    sptr<AbilityConnection> abilityConnection = new AbilityConnection();
    AppExecFwk::ElementName element;
    EXPECT_EQ(abilityConnection->abilityConnectCallbackList_.size(), 0);
    sptr<MockAbilityConnectCallback> mockAbilityConnectCallback(new MockAbilityConnectCallback());
    abilityConnection->AddConnectCallback(mockAbilityConnectCallback);
    EXPECT_NE(abilityConnection->abilityConnectCallbackList_.size(), 0);
    abilityConnection->RemoveConnectCallback(mockAbilityConnectCallback);
    EXPECT_EQ(abilityConnection->abilityConnectCallbackList_.size(), 0);
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityConnection_RemoveConnectCallback_0100 end";
}

/**
 * @tc.number: AbilityRuntime_AbilityConnection_SetRemoteObject_0100
 * @tc.name: SetRemoteObject
 * @tc.desc: Test whether SetRemoteObject is called normally.
 */
HWTEST_F(AbilityConnectionTest, AbilityRuntime_AbilityConnection_SetRemoteObject_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityConnection_SetRemoteObject_0100 start";
    sptr<AbilityConnection> abilityConnection = new AbilityConnection();
    sptr<IRemoteObject> remoteObject = nullptr;
    abilityConnection->SetRemoteObject(remoteObject);
    EXPECT_EQ(abilityConnection->remoteObject_, nullptr);
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityConnection_SetRemoteObject_0100 end";
}

/**
 * @tc.number: AbilityRuntime_AbilityConnection_GetRemoteObject_0100
 * @tc.name: GetRemoteObject
 * @tc.desc: Test whether GetRemoteObject is called normally.
 */
HWTEST_F(AbilityConnectionTest, AbilityRuntime_AbilityConnection_GetRemoteObject_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityConnection_GetRemoteObject_0100 start";
    sptr<AbilityConnection> abilityConnection = new AbilityConnection();
    sptr<IRemoteObject> remoteObject = nullptr;
    abilityConnection->SetRemoteObject(remoteObject);
    EXPECT_EQ(abilityConnection->GetRemoteObject(), nullptr);
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityConnection_GetRemoteObject_0100 end";
}

/**
 * @tc.number: AbilityRuntime_AbilityConnection_SetResultCode_0100
 * @tc.name: SetResultCode
 * @tc.desc: Test whether SetResultCode is called normally.
 */
HWTEST_F(AbilityConnectionTest, AbilityRuntime_AbilityConnection_SetResultCode_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityConnection_SetResultCode_0100 start";
    sptr<AbilityConnection> abilityConnection = new AbilityConnection();
    int resultCode = 0;
    abilityConnection->SetResultCode(resultCode);
    EXPECT_EQ(abilityConnection->GetResultCode(), 0);
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityConnection_SetResultCode_0100 end";
}

/**
 * @tc.number: AbilityRuntime_AbilityConnection_GetResultCode_0100
 * @tc.name: GetResultCode
 * @tc.desc: Test whether GetResultCode is called normally.
 */
HWTEST_F(AbilityConnectionTest, AbilityRuntime_AbilityConnection_GetResultCode_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityConnection_GetResultCode_0100 start";
    sptr<AbilityConnection> abilityConnection = new AbilityConnection();
    abilityConnection->SetResultCode(abilityConnection->GetResultCode());
    EXPECT_EQ(abilityConnection->GetResultCode(), -1);
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityConnection_GetResultCode_0100 end";
}

/**
 * @tc.number: AbilityRuntime_AbilityConnection_SetConnectionState_0100
 * @tc.name: SetConnectionState
 * @tc.desc: Test whether SetConnectionState is called normally.
 */
HWTEST_F(AbilityConnectionTest, AbilityRuntime_AbilityConnection_SetConnectionState_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityConnection_SetConnectionState_0100 start";
    sptr<AbilityConnection> abilityConnection = new AbilityConnection();
    int connectionState = 0;
    abilityConnection->SetConnectionState(connectionState);
    EXPECT_EQ(abilityConnection->GetConnectionState(), 0);
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityConnection_SetConnectionState_0100 end";
}

/**
 * @tc.number: AbilityRuntime_AbilityConnection_GetConnectionState_0100
 * @tc.name: GetConnectionState
 * @tc.desc: Test whether GetConnectionState is called normally.
 */
HWTEST_F(AbilityConnectionTest, AbilityRuntime_AbilityConnection_GetConnectionState_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityConnection_GetConnectionState_0100 start";
    sptr<AbilityConnection> abilityConnection = new AbilityConnection();
    abilityConnection->SetConnectionState(CONNECTION_STATE_CONNECTED);
    EXPECT_EQ(abilityConnection->GetConnectionState(), CONNECTION_STATE_CONNECTED);
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityConnection_GetConnectionState_0100 end";
}

/**
 * @tc.number: AbilityRuntime_AbilityConnection_GetCallbackList_0100
 * @tc.name: GetCallbackList
 * @tc.desc: Test whether GetCallbackList is called normally.
 */
HWTEST_F(AbilityConnectionTest, AbilityRuntime_AbilityConnection_GetCallbackList_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityConnection_GetCallbackList_0100 start";
    sptr<AbilityConnection> abilityConnection = new AbilityConnection();
    EXPECT_EQ(abilityConnection->GetCallbackList(), abilityConnection->abilityConnectCallbackList_);
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityConnection_GetCallbackList_0100 end";
}
}  // namespace AAFwk
}  // namespace OHOS