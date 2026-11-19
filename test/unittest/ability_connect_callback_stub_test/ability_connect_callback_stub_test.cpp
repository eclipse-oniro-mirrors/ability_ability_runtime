/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include <gmock/gmock.h>
#include "iremote_proxy.h"

#include "ability_connect_callback_stub.h"

#include "mock_ability_connect_callback_stub.h"
#include "ability_connect_callback_proxy.h"

using namespace testing::ext;
using namespace OHOS::AAFwk;
using namespace OHOS;
using namespace testing;

class AbilityConnectCallBackStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    void WriteInterfaceToken(MessageParcel& data);
};

void AbilityConnectCallBackStubTest::SetUpTestCase(void)
{}
void AbilityConnectCallBackStubTest::TearDownTestCase(void)
{}
void AbilityConnectCallBackStubTest::SetUp()
{}
void AbilityConnectCallBackStubTest::TearDown()
{}

void AbilityConnectCallBackStubTest::WriteInterfaceToken(MessageParcel& data)
{
    data.WriteInterfaceToken(AbilityConnectionProxy::GetDescriptor());
}

/*
 * Feature: AbilityConnectionStub
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityConnectionStub OnRemoteRequest
 * EnvConditions: ElementName is nullptr
 * CaseDescription: Verify that on remote request is normal and abnormal
 */
HWTEST_F(AbilityConnectCallBackStubTest, AbilityConnectionCallBack_IPC_001, TestSize.Level1)
{
    sptr<MockAbilityConnectCallback> mockAbilityConnectStub(new MockAbilityConnectCallback());

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    sptr<IRemoteObject> remoteObject;
    int32_t reqCode = 10;

    WriteInterfaceToken(data);
    data.WriteParcelable(nullptr);
    data.WriteParcelable(remoteObject);
    data.WriteInt32(reqCode);

    int res = mockAbilityConnectStub->OnRemoteRequest(IAbilityConnection::ON_ABILITY_CONNECT_DONE, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityConnectionStub
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityConnectionStub OnRemoteRequest
 * EnvConditions: ElementName is not nullptr
 * CaseDescription: Verify that on remote request is normal and abnormal
 */
HWTEST_F(AbilityConnectCallBackStubTest, AbilityConnectionCallBack_IPC_002, TestSize.Level1)
{
    sptr<MockAbilityConnectCallback> mockAbilityConnectStub(new MockAbilityConnectCallback());

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    auto element = std::make_shared<AppExecFwk::ElementName>("bundlename", "appname", "abilityname");
    int32_t reqCode = 10;

    WriteInterfaceToken(data);
    data.WriteParcelable(element.get());
    data.WriteRemoteObject(mockAbilityConnectStub);
    data.WriteInt32(reqCode);

    EXPECT_CALL(*mockAbilityConnectStub, OnAbilityConnectDone(_, _, _)).Times(1);
    int res = mockAbilityConnectStub->OnRemoteRequest(IAbilityConnection::ON_ABILITY_CONNECT_DONE, data, reply, option);
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: AbilityConnectionStub
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityConnectionStub OnRemoteRequest
 * EnvConditions: ElementName is nullptr
 * CaseDescription: Verify that on remote request is normal and abnormal
 */
HWTEST_F(AbilityConnectCallBackStubTest, AbilityConnectionCallBack_IPC_003, TestSize.Level1)
{
    sptr<MockAbilityConnectCallback> mockAbilityConnectStub(new MockAbilityConnectCallback());

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    sptr<IRemoteObject> remoteObject;
    int32_t reqCode = 10;

    WriteInterfaceToken(data);
    data.WriteParcelable(nullptr);
    data.WriteInt32(reqCode);

    int res =
        mockAbilityConnectStub->OnRemoteRequest(IAbilityConnection::ON_ABILITY_DISCONNECT_DONE, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityConnectionStub
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityConnectionStub OnRemoteRequest
 * EnvConditions: ElementName is not nullptr
 * CaseDescription: Verify that on remote request is normal and abnormal
 */
HWTEST_F(AbilityConnectCallBackStubTest, AbilityConnectionCallBack_IPC_004, TestSize.Level1)
{
    sptr<MockAbilityConnectCallback> mockAbilityConnectStub(new MockAbilityConnectCallback());

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    auto element = std::make_shared<AppExecFwk::ElementName>("bundlename", "appname", "abilityname");
    sptr<IRemoteObject> remoteObject;
    int32_t reqCode = 10;

    WriteInterfaceToken(data);
    data.WriteParcelable(element.get());
    data.WriteInt32(reqCode);

    EXPECT_CALL(*mockAbilityConnectStub, OnAbilityDisconnectDone(_, _)).Times(1);
    int res =
        mockAbilityConnectStub->OnRemoteRequest(IAbilityConnection::ON_ABILITY_DISCONNECT_DONE, data, reply, option);
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: AbilityConnectionStub
 * Function: OnRemoteStateChanged
 * SubFunction: NA
 * FunctionPoints: AbilityConnectionStub OnRemoteStateChanged
 */
HWTEST_F(AbilityConnectCallBackStubTest, AbilityConnectionCallBack_IPC_005, TestSize.Level1)
{
    sptr<MockAbilityConnectCallback> mockAbilityConnectStub(new MockAbilityConnectCallback());
    EXPECT_TRUE(mockAbilityConnectStub != nullptr);
    AppExecFwk::ElementName element;
    mockAbilityConnectStub->OnRemoteStateChanged(element, 10);
}

/*
 * Feature: AbilityConnectionStub
 * Function: CompareDMSDescriptor
 * SubFunction: NA
 * FunctionPoints: non-matching descriptor returns false
 * EnvConditions: remote descriptor does not match the stub descriptor nor any DMS descriptor
 * CaseDescription: CompareDMSDescriptor is invoked directly with a descriptor that is neither
 *                 the stub descriptor nor any DMS descriptor. The result must be false on every
 *                 device regardless of whether libabilitywantadapter.z.so exists: when the .so
 *                 is absent dlopen fails and g_dmsDescriptors stays empty; when present the test
 *                 descriptor is not in the list. No assertion is made on the dlopen result itself.
 *                 Covers the one-time load via std::call_once and the no-match return false branch.
 */
HWTEST_F(AbilityConnectCallBackStubTest, AbilityConnectionCallBack_DMS_001, TestSize.Level1)
{
    sptr<MockAbilityConnectCallback> mockAbilityConnectStub(new MockAbilityConnectCallback());
    std::u16string nonMatchDescriptor = u"NOT_A_DMS_DESCRIPTOR";

    bool res = mockAbilityConnectStub->CompareDMSDescriptor(nonMatchDescriptor);
    EXPECT_FALSE(res);
}

/*
 * Feature: AbilityConnectionStub
 * Function: CompareDMSDescriptor
 * SubFunction: NA
 * FunctionPoints: DMS descriptor loaded once, second call still returns false for non-match
 * EnvConditions: remote descriptor does not match the stub descriptor nor any DMS descriptor
 * CaseDescription: The DMS descriptor table is loaded once per process via std::call_once.
 *                 The second call with a non-matching descriptor must still return false.
 *                 The second call must not re-load the .so (guaranteed by std::call_once).
 *                 Covers the skip-load branch after one-time initialization.
 */
HWTEST_F(AbilityConnectCallBackStubTest, AbilityConnectionCallBack_DMS_002, TestSize.Level1)
{
    sptr<MockAbilityConnectCallback> mockAbilityConnectStub(new MockAbilityConnectCallback());
    std::u16string nonMatchDescriptor = u"NOT_A_DMS_DESCRIPTOR";

    mockAbilityConnectStub->CompareDMSDescriptor(nonMatchDescriptor);

    bool res = mockAbilityConnectStub->CompareDMSDescriptor(nonMatchDescriptor);
    EXPECT_FALSE(res);
}
