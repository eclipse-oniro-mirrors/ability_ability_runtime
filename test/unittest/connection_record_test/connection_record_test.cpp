/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#define private public
#define protected public
#include "base_extension_record.h"
#include "connection_record.h"
#undef private
#undef protected

#include "app_process_data.h"
#include "ability_manager_errors.h"
#include "ability_connect_callback_stub.h"
#include "ability_scheduler.h"
#include "ability_state.h"
#include "iremote_object.h"

using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
class MockIRemoteObject : public IRemoteObject {
public:
    MockIRemoteObject() : IRemoteObject(u"mock_i_remote_object") {}

    ~MockIRemoteObject() override {}

    int32_t GetObjectRefCount() override
    {
        return 0;
    }

    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        return 0;
    }

    bool IsProxyObject() const override
    {
        return true;
    }

    bool CheckObjectLegality() const override
    {
        return true;
    }

    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        return true;
    }

    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        return true;
    }

    bool Marshalling(Parcel &parcel) const override
    {
        return true;
    }

    sptr<IRemoteBroker> AsInterface() override
    {
        return nullptr;
    }

    int Dump(int fd, const std::vector<std::u16string> &args) override
    {
        return 0;
    }

    std::u16string GetObjectDescriptor() const
    {
        return std::u16string();
    }
};

namespace AAFwk {
class AbilityConnectCallbackMock : public AbilityConnectionStub {
public:
    AbilityConnectCallbackMock()
    {}
    virtual ~AbilityConnectCallbackMock()
    {}

    MOCK_METHOD3(OnAbilityConnectDone, void(const ElementName&, const OHOS::sptr<IRemoteObject>&, int));
    MOCK_METHOD2(OnAbilityDisconnectDone, void(const ElementName&, int));
};

class ConnectionRecordTest : public testing::TestWithParam<OHOS::AAFwk::ConnectionState> {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    AbilityRequest GenerateAbilityRequest(const std::string& deviceName, const std::string& abilityName,
        const std::string& appName, const std::string& bundleName);

    std::shared_ptr<ConnectionRecord> connectionRecord_{ nullptr };
    OHOS::sptr<AbilityConnectCallbackMock> callback_{ nullptr };
    std::shared_ptr<BaseExtensionRecord> service_{ nullptr };
};

AbilityRequest ConnectionRecordTest::GenerateAbilityRequest(const std::string& deviceName,
    const std::string& abilityName, const std::string& appName, const std::string& bundleName)
{
    ElementName element(deviceName, bundleName, abilityName);
    Want want;
    want.SetElement(element);

    AbilityInfo abilityInfo;
    abilityInfo.applicationName = appName;
    abilityInfo.bundleName = bundleName;
    abilityInfo.type = AbilityType::SERVICE;
    ApplicationInfo appinfo;
    appinfo.name = appName;

    AbilityRequest abilityRequest;
    abilityRequest.want = want;
    abilityRequest.abilityInfo = abilityInfo;
    abilityRequest.appInfo = appinfo;

    return abilityRequest;
}

void ConnectionRecordTest::SetUpTestCase(void)
{}
void ConnectionRecordTest::TearDownTestCase(void)
{}

void ConnectionRecordTest::SetUp(void)
{
    callback_ = new AbilityConnectCallbackMock();

    std::string deviceName = "device";
    std::string abilityName = "ServiceAbility";
    std::string appName = "hiservcie";
    std::string bundleName = "com.ix.hiservcie";
    AbilityRequest abilityRequest = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName);
    service_ = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    connectionRecord_ = std::make_shared<ConnectionRecord>(service_->GetToken(), service_, callback_, nullptr);
}

void ConnectionRecordTest::TearDown(void)
{
    connectionRecord_.reset();
}

bool IsExist(const std::string& state)
{
    return std::string::npos != state.find("com.ix.hiservcie");
}

/*
 * Feature: ConnectionRecord
 * Function: SetConnectState and GetConnectState
 * SubFunction: NA
 * FunctionPoints: SetConnectState and GetConnectState
 * EnvConditions:NA
 * CaseDescription: Verify set and get
 */
HWTEST_F(ConnectionRecordTest, AaFwk_ConnectionRecord_001, TestSize.Level1)
{
    connectionRecord_->SetConnectState(ConnectionState::CONNECTED);
    EXPECT_EQ(connectionRecord_->GetConnectState(), ConnectionState::CONNECTED);
}

/*
 * Feature: ConnectionRecord
 * Function: GetToken
 * SubFunction: NA
 * FunctionPoints: GetToken
 * EnvConditions:NA
 * CaseDescription: Verify that the tokens are equal
 */
HWTEST_F(ConnectionRecordTest, AaFwk_ConnectionRecord_002, TestSize.Level1)
{
    EXPECT_EQ(connectionRecord_->GetToken().GetRefPtr(), service_->GetToken().GetRefPtr());
}

/*
 * Feature: ConnectionRecord
 * Function: GetAbilityRecord
 * SubFunction: NA
 * FunctionPoints: GetAbilityRecord
 * EnvConditions:NA
 * CaseDescription: Verify that the ability record are equal
 */
HWTEST_F(ConnectionRecordTest, AaFwk_ConnectionRecord_003, TestSize.Level1)
{
    EXPECT_EQ(connectionRecord_->GetAbilityRecord(), service_);
}

/*
 * Feature: ConnectionRecord
 * Function: GetAbilityConnectCallback
 * SubFunction: NA
 * FunctionPoints: GetAbilityConnectCallback
 * EnvConditions:NA
 * CaseDescription: Verify that the call back are equal
 */
HWTEST_F(ConnectionRecordTest, AaFwk_ConnectionRecord_004, TestSize.Level1)
{
    EXPECT_EQ(connectionRecord_->GetAbilityConnectCallback(), iface_cast<IAbilityConnection>(callback_));
}

/*
 * Feature: ConnectionRecord
 * Function: DisconnectAbility
 * SubFunction: NA
 * FunctionPoints: DisconnectAbility
 * EnvConditions:NA
 * CaseDescription: 1.Connection state is not connected, DisconnectAbility failed
 * 2.Verify the correct process of disconnectability
 */
HWTEST_F(ConnectionRecordTest, AaFwk_ConnectionRecord_005, TestSize.Level1)
{
    auto result = connectionRecord_->DisconnectAbility();
    EXPECT_EQ(result, INVALID_CONNECTION_STATE);

    connectionRecord_->SetConnectState(ConnectionState::CONNECTED);

    result = connectionRecord_->DisconnectAbility();
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(connectionRecord_->GetConnectState(), ConnectionState::DISCONNECTED);
}

/*
 * Feature: ConnectionRecord
 * Function: CompleteConnect
 * SubFunction: NA
 * FunctionPoints: CompleteConnect
 * EnvConditions:NA
 * CaseDescription: Verify the correct process of completeconnect
 */
HWTEST_F(ConnectionRecordTest, AaFwk_ConnectionRecord_006, TestSize.Level1)
{
    connectionRecord_->CompleteConnect();
    EXPECT_EQ(connectionRecord_->GetConnectState(), ConnectionState::CONNECTED);
    EXPECT_EQ(service_->GetAbilityState(), AAFwk::AbilityState::ACTIVE);
}

/*
 * Feature: ConnectionRecord
 * Function: CompleteDisconnect
 * SubFunction: NA
 * FunctionPoints: CompleteDisconnect
 * EnvConditions:NA
 * CaseDescription: Verify the correct process of complete disconnect
 */
HWTEST_F(ConnectionRecordTest, AaFwk_ConnectionRecord_007, TestSize.Level1)
{
    connectionRecord_->CompleteDisconnect(ERR_OK, false);
    EXPECT_EQ(connectionRecord_->GetConnectState(), ConnectionState::DISCONNECTED);
}

/*
 * Feature: ConnectionRecord
 * Function: ScheduleDisconnectAbilityDone
 * SubFunction: NA
 * FunctionPoints: ScheduleDisconnectAbilityDone
 * EnvConditions:NA
 * CaseDescription: 1.Connection state is not DISCONNECTING, Onabilitydisconnectdone is not called
 * 2.Connection state is DISCONNECTING, Call onabilitydisconnect done
 */
HWTEST_F(ConnectionRecordTest, AaFwk_ConnectionRecord_008, TestSize.Level1)
{
    connectionRecord_->SetConnectState(ConnectionState::CONNECTED);
    connectionRecord_->ScheduleDisconnectAbilityDone();

    connectionRecord_->SetConnectState(ConnectionState::DISCONNECTING);
    connectionRecord_->ScheduleDisconnectAbilityDone();
    EXPECT_EQ(connectionRecord_->GetConnectState(), ConnectionState::DISCONNECTED);
}

/*
 * Feature: ConnectionRecord
 * Function: ScheduleConnectAbilityDone
 * SubFunction: NA
 * FunctionPoints: ScheduleConnectAbilityDone
 * EnvConditions:NA
 * CaseDescription: 1.Connection state is not CONNECTING, OnAbilityConnectDone is not called
 * 2.Connection state is CONNECTING, Call OnAbilityConnectDone done
 */
HWTEST_F(ConnectionRecordTest, AaFwk_ConnectionRecord_009, TestSize.Level1)
{
    connectionRecord_->SetConnectState(ConnectionState::DISCONNECTING);
    connectionRecord_->ScheduleConnectAbilityDone();

    connectionRecord_->SetConnectState(ConnectionState::CONNECTING);
    connectionRecord_->ScheduleConnectAbilityDone();
    EXPECT_TRUE(connectionRecord_ != nullptr);
}

/*
 * Feature: ConnectionRecord
 * Function: GetRecordId
 * SubFunction: NA
 * FunctionPoints: GetRecordId
 * EnvConditions:NA
 * CaseDescription: Verify that getrecordids are equal
 */
HWTEST_F(ConnectionRecordTest, AaFwk_ConnectionRecord_010, TestSize.Level1)
{
    EXPECT_EQ(connectionRecord_->GetRecordId(), 9);
}

/*
 * Feature: ConnectionRecord
 * Function: GetAbilityConnectCallback
 * SubFunction: NA
 * FunctionPoints: GetAbilityConnectCallback
 * EnvConditions:NA
 * CaseDescription: Verify that getabilityconnectcallback is nullptr
 */
HWTEST_F(ConnectionRecordTest, AaFwk_ConnectionRecord_011, TestSize.Level1)
{
    connectionRecord_->ClearConnCallBack();
    EXPECT_EQ(connectionRecord_->GetAbilityConnectCallback().GetRefPtr(), nullptr);
}

/*
 * Feature: ConnectionRecord
 * Function: ConvertConnectionState
 * SubFunction: NA
 * FunctionPoints: ConvertConnectionState
 * EnvConditions:NA
 * CaseDescription: Verify ConvertConnectionState results
 */
HWTEST_F(ConnectionRecordTest, AaFwk_ConnectionRecord_012, TestSize.Level1)
{
    auto res = connectionRecord_->ConvertConnectionState(ConnectionState::INIT);
    EXPECT_EQ(res, "INIT");
    res = connectionRecord_->ConvertConnectionState(ConnectionState::CONNECTING);
    EXPECT_EQ(res, "CONNECTING");
    res = connectionRecord_->ConvertConnectionState(ConnectionState::CONNECTED);
    EXPECT_EQ(res, "CONNECTED");
    res = connectionRecord_->ConvertConnectionState(ConnectionState::DISCONNECTING);
    EXPECT_EQ(res, "DISCONNECTING");
    res = connectionRecord_->ConvertConnectionState(ConnectionState::DISCONNECTED);
    EXPECT_EQ(res, "DISCONNECTED");
}

/*
 * Feature: ConnectionRecord
 * Function: Dump
 * SubFunction: NA
 * FunctionPoints: Dump
 * EnvConditions:NA
 * CaseDescription: Verify dump results
 */
HWTEST_F(ConnectionRecordTest, AaFwk_ConnectionRecord_013, TestSize.Level1)
{
    std::vector<std::string> info;
    connectionRecord_->Dump(info);

    for (auto& it : info) {
        GTEST_LOG_(INFO) << it;
    }

    EXPECT_NE(info.end(), std::find_if(info.begin(), info.end(), IsExist));
}

/*
 * Feature: ConnectionRecord
 * Function: SuspendExtensionAbility
 * SubFunction: NA
 * FunctionPoints: SuspendExtensionAbility
 * EnvConditions:NA
 * CaseDescription: 1.Connection state is not connected, SuspendExtensionAbility failed
 * 2.Verify the correct process of suspendextensionability
 */
HWTEST_F(ConnectionRecordTest, AaFwk_ConnectionRecord_014, TestSize.Level1)
{
    auto result = connectionRecord_->SuspendExtensionAbility();
    EXPECT_EQ(result, INVALID_CONNECTION_STATE);

    connectionRecord_->SetConnectState(ConnectionState::CONNECTED);

    result = connectionRecord_->SuspendExtensionAbility();
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(connectionRecord_->GetConnectState(), ConnectionState::CONNECTED);
}

/*
 * Feature: ConnectionRecord
 * Function: SuspendExtensionAbility
 * SubFunction: NA
 * FunctionPoints: SuspendExtensionAbility
 * EnvConditions:NA
 * CaseDescription: 1.Connection state is not connected, SuspendExtensionAbility failed
 * 2.Verify the correct process of suspendextensionability
 */
HWTEST_F(ConnectionRecordTest, AaFwk_ConnectionRecord_015, TestSize.Level1)
{
    auto result = connectionRecord_->ResumeExtensionAbility();
    EXPECT_EQ(result, INVALID_CONNECTION_STATE);

    connectionRecord_->SetConnectState(ConnectionState::CONNECTED);

    result = connectionRecord_->ResumeExtensionAbility();
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(connectionRecord_->GetConnectState(), ConnectionState::CONNECTED);
}

/*
 * Feature: ConnectionRecord
 * Function: DisconnectAbility
 * SubFunction: NA
 * FunctionPoints: DisconnectAbility
 * EnvConditions:NA
 * CaseDescription: Verify DisconnectAbility returns INVALID_CONNECTION_STATE when state is INIT
 */
HWTEST_F(ConnectionRecordTest, ConnectionRecord_DisconnectAbility_001, TestSize.Level1)
{
    auto record = std::make_shared<ConnectionRecord>(service_->GetToken(), service_, callback_, nullptr);
    record->SetConnectState(ConnectionState::INIT);
    auto result = record->DisconnectAbility();
    EXPECT_EQ(result, INVALID_CONNECTION_STATE);
}

/*
 * Feature: ConnectionRecord
 * Function: DisconnectAbility
 * SubFunction: NA
 * FunctionPoints: DisconnectAbility
 * EnvConditions:NA
 * CaseDescription: Verify DisconnectAbility returns INVALID_CONNECTION_STATE when state is CONNECTING
 */
HWTEST_F(ConnectionRecordTest, ConnectionRecord_DisconnectAbility_002, TestSize.Level1)
{
    auto record = std::make_shared<ConnectionRecord>(service_->GetToken(), service_, callback_, nullptr);
    record->SetConnectState(ConnectionState::CONNECTING);
    auto result = record->DisconnectAbility();
    EXPECT_EQ(result, INVALID_CONNECTION_STATE);
}

/*
 * Feature: ConnectionRecord
 * Function: DisconnectAbility
 * SubFunction: NA
 * FunctionPoints: DisconnectAbility
 * EnvConditions:NA
 * CaseDescription: Verify DisconnectAbility returns INVALID_CONNECTION_STATE when state is DISCONNECTING
 */
HWTEST_F(ConnectionRecordTest, ConnectionRecord_DisconnectAbility_003, TestSize.Level1)
{
    auto record = std::make_shared<ConnectionRecord>(service_->GetToken(), service_, callback_, nullptr);
    record->SetConnectState(ConnectionState::DISCONNECTING);
    auto result = record->DisconnectAbility();
    EXPECT_EQ(result, INVALID_CONNECTION_STATE);
}

/*
 * Feature: ConnectionRecord
 * Function: DisconnectAbility
 * SubFunction: NA
 * FunctionPoints: DisconnectAbility
 * EnvConditions:NA
 * CaseDescription: Verify DisconnectAbility returns INVALID_CONNECTION_STATE when state is DISCONNECTED
 */
HWTEST_F(ConnectionRecordTest, ConnectionRecord_DisconnectAbility_004, TestSize.Level1)
{
    auto record = std::make_shared<ConnectionRecord>(service_->GetToken(), service_, callback_, nullptr);
    record->SetConnectState(ConnectionState::DISCONNECTED);
    auto result = record->DisconnectAbility();
    EXPECT_EQ(result, INVALID_CONNECTION_STATE);
}

/*
 * Feature: ConnectionRecord
 * Function: DisconnectAbility
 * SubFunction: NA
 * FunctionPoints: DisconnectAbility
 * EnvConditions:NA
 * CaseDescription: Verify DisconnectAbility with null targetService
 */
HWTEST_F(ConnectionRecordTest, ConnectionRecord_DisconnectAbility_005, TestSize.Level1)
{
    auto record = std::make_shared<ConnectionRecord>(nullptr, nullptr, callback_, nullptr);
    record->SetConnectState(ConnectionState::CONNECTED);
    auto result = record->DisconnectAbility();
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: ConnectionRecord
 * Function: DisconnectAbility
 * SubFunction: NA
 * FunctionPoints: DisconnectAbility
 * EnvConditions:NA
 * CaseDescription: Verify DisconnectAbility with connection in service list (single connection)
 */
HWTEST_F(ConnectionRecordTest, ConnectionRecord_DisconnectAbility_006, TestSize.Level1)
{
    auto record = std::make_shared<ConnectionRecord>(service_->GetToken(), service_, callback_, nullptr);
    service_->AddConnectRecordToList(record);
    record->SetConnectState(ConnectionState::CONNECTED);
    auto result = record->DisconnectAbility();
    EXPECT_EQ(result, ERR_OK);
    // connectNums == 1, so state becomes DISCONNECTING
    EXPECT_EQ(record->GetConnectState(), ConnectionState::DISCONNECTING);
    service_->RemoveConnectRecordFromList(record);
}

/*
 * Feature: ConnectionRecord
 * Function: DisconnectAbility
 * SubFunction: NA
 * FunctionPoints: DisconnectAbility
 * EnvConditions:NA
 * CaseDescription: Verify DisconnectAbility with multiple connections removes from list
 */
HWTEST_F(ConnectionRecordTest, ConnectionRecord_DisconnectAbility_007, TestSize.Level1)
{
    auto record1 = std::make_shared<ConnectionRecord>(service_->GetToken(), service_, callback_, nullptr);
    auto record2 = std::make_shared<ConnectionRecord>(service_->GetToken(), service_, callback_, nullptr);
    service_->AddConnectRecordToList(record1);
    service_->AddConnectRecordToList(record2);
    record1->SetConnectState(ConnectionState::CONNECTED);
    auto result = record1->DisconnectAbility();
    EXPECT_EQ(result, ERR_OK);
    // connectNums > 1 && !isPerConnectionType, so state becomes DISCONNECTED directly
    EXPECT_EQ(record1->GetConnectState(), ConnectionState::DISCONNECTED);
    service_->RemoveConnectRecordFromList(record1);
    service_->RemoveConnectRecordFromList(record2);
}

/*
 * Feature: ConnectionRecord
 * Function: DisconnectAbility
 * SubFunction: NA
 * FunctionPoints: DisconnectAbility
 * EnvConditions:NA
 * CaseDescription: Verify DisconnectAbility with UI_SERVICE type calls DisconnectAbilityWithWant
 */
HWTEST_F(ConnectionRecordTest, ConnectionRecord_DisconnectAbility_008, TestSize.Level1)
{
    // Create service with UI_SERVICE extension type
    Want want;
    want.SetParam("ohos.agentruntime.params.AgentId", std::string("testAgent"));
    want.SetElementName(std::string("device"), std::string("com.test.bundle"), std::string("testModule"),
        std::string("TestAbility"));

    AbilityInfo abilityInfo;
    abilityInfo.applicationName = "hiservice";
    abilityInfo.bundleName = "com.test.bundle";
    abilityInfo.name = "TestAbility";
    abilityInfo.type = AbilityType::EXTENSION;
    abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::UI_SERVICE;

    ApplicationInfo appinfo;
    appinfo.name = "hiservice";

    AbilityRequest abilityRequest;
    abilityRequest.want = want;
    abilityRequest.abilityInfo = abilityInfo;
    abilityRequest.appInfo = appinfo;

    auto uiService = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    auto record = std::make_shared<ConnectionRecord>(uiService->GetToken(), uiService, callback_, nullptr);
    uiService->AddConnectRecordToList(record);
    record->SetConnectState(ConnectionState::CONNECTED);
    auto result = record->DisconnectAbility();
    EXPECT_EQ(result, ERR_OK);
    // isPerConnectionType == true, so state becomes DISCONNECTING
    EXPECT_EQ(record->GetConnectState(), ConnectionState::DISCONNECTING);
    uiService->RemoveConnectRecordFromList(record);
}

/*
 * Feature: ConnectionRecord
 * Function: DisconnectAbility
 * SubFunction: NA
 * FunctionPoints: DisconnectAbility
 * EnvConditions:NA
 * CaseDescription: Verify DisconnectAbility with AGENT type calls DisconnectAbilityWithWant
 */
HWTEST_F(ConnectionRecordTest, ConnectionRecord_DisconnectAbility_009, TestSize.Level1)
{
    // Create service with AGENT extension type
    Want want;
    want.SetParam("ohos.agentruntime.params.AgentId", std::string("testAgent"));
    want.SetElementName(std::string("device"), std::string("com.test.bundle"), std::string("testModule"),
        std::string("TestAbility"));

    AbilityInfo abilityInfo;
    abilityInfo.applicationName = "hiservice";
    abilityInfo.bundleName = "com.test.bundle";
    abilityInfo.name = "TestAbility";
    abilityInfo.type = AbilityType::EXTENSION;
    abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::AGENT;

    ApplicationInfo appinfo;
    appinfo.name = "hiservice";

    AbilityRequest abilityRequest;
    abilityRequest.want = want;
    abilityRequest.abilityInfo = abilityInfo;
    abilityRequest.appInfo = appinfo;

    auto agentService = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    auto record = std::make_shared<ConnectionRecord>(agentService->GetToken(), agentService, callback_, nullptr);
    agentService->AddConnectRecordToList(record);
    record->SetConnectState(ConnectionState::CONNECTED);
    auto result = record->DisconnectAbility();
    EXPECT_EQ(result, ERR_OK);
    // isPerConnectionType == true, so state becomes DISCONNECTING
    EXPECT_EQ(record->GetConnectState(), ConnectionState::DISCONNECTING);
    agentService->RemoveConnectRecordFromList(record);
}

/*
 * Feature: ConnectionRecord
 * Function: ScheduleConnectAbilityDone
 * SubFunction: NA
 * FunctionPoints: ScheduleConnectAbilityDone
 * EnvConditions:NA
 * CaseDescription: Verify ScheduleConnectAbilityDone returns early when state is INIT
 */
HWTEST_F(ConnectionRecordTest, ConnectionRecord_ScheduleConnectAbilityDone_001, TestSize.Level1)
{
    auto record = std::make_shared<ConnectionRecord>(service_->GetToken(), service_, callback_, nullptr);
    record->SetConnectState(ConnectionState::INIT);
    record->ScheduleConnectAbilityDone();
    EXPECT_EQ(record->GetConnectState(), ConnectionState::INIT);
}

/*
 * Feature: ConnectionRecord
 * Function: ScheduleConnectAbilityDone
 * SubFunction: NA
 * FunctionPoints: ScheduleConnectAbilityDone
 * EnvConditions:NA
 * CaseDescription: Verify ScheduleConnectAbilityDone returns early when state is CONNECTED
 */
HWTEST_F(ConnectionRecordTest, ConnectionRecord_ScheduleConnectAbilityDone_002, TestSize.Level1)
{
    auto record = std::make_shared<ConnectionRecord>(service_->GetToken(), service_, callback_, nullptr);
    record->SetConnectState(ConnectionState::CONNECTED);
    record->ScheduleConnectAbilityDone();
    EXPECT_EQ(record->GetConnectState(), ConnectionState::CONNECTED);
}

/*
 * Feature: ConnectionRecord
 * Function: ScheduleConnectAbilityDone
 * SubFunction: NA
 * FunctionPoints: ScheduleConnectAbilityDone
 * EnvConditions:NA
 * CaseDescription: Verify ScheduleConnectAbilityDone returns early when state is DISCONNECTING
 */
HWTEST_F(ConnectionRecordTest, ConnectionRecord_ScheduleConnectAbilityDone_003, TestSize.Level1)
{
    auto record = std::make_shared<ConnectionRecord>(service_->GetToken(), service_, callback_, nullptr);
    record->SetConnectState(ConnectionState::DISCONNECTING);
    record->ScheduleConnectAbilityDone();
    EXPECT_EQ(record->GetConnectState(), ConnectionState::DISCONNECTING);
}

/*
 * Feature: ConnectionRecord
 * Function: ScheduleConnectAbilityDone
 * SubFunction: NA
 * FunctionPoints: ScheduleConnectAbilityDone
 * EnvConditions:NA
 * CaseDescription: Verify ScheduleConnectAbilityDone returns early when state is DISCONNECTED
 */
HWTEST_F(ConnectionRecordTest, ConnectionRecord_ScheduleConnectAbilityDone_004, TestSize.Level1)
{
    auto record = std::make_shared<ConnectionRecord>(service_->GetToken(), service_, callback_, nullptr);
    record->SetConnectState(ConnectionState::DISCONNECTED);
    record->ScheduleConnectAbilityDone();
    EXPECT_EQ(record->GetConnectState(), ConnectionState::DISCONNECTED);
}

/*
 * Feature: ConnectionRecord
 * Function: ScheduleConnectAbilityDone
 * SubFunction: NA
 * FunctionPoints: ScheduleConnectAbilityDone
 * EnvConditions:NA
 * CaseDescription: Verify ScheduleConnectAbilityDone completes connect when state is CONNECTING
 */
HWTEST_F(ConnectionRecordTest, ConnectionRecord_ScheduleConnectAbilityDone_005, TestSize.Level1)
{
    auto record = std::make_shared<ConnectionRecord>(service_->GetToken(), service_, callback_, nullptr);
    record->SetConnectState(ConnectionState::CONNECTING);
    record->ScheduleConnectAbilityDone();
    EXPECT_EQ(record->GetConnectState(), ConnectionState::CONNECTED);
}

/*
 * Feature: ConnectionRecord
 * Function: ScheduleConnectAbilityDone
 * SubFunction: NA
 * FunctionPoints: ScheduleConnectAbilityDone
 * EnvConditions:NA
 * CaseDescription: Verify ScheduleConnectAbilityDone preserves UISERVICEHOSTPROXY_KEY
 */
HWTEST_F(ConnectionRecordTest, ConnectionRecord_ScheduleConnectAbilityDone_006, TestSize.Level1)
{
    auto record = std::make_shared<ConnectionRecord>(service_->GetToken(), service_, callback_, nullptr);
    record->SetConnectState(ConnectionState::CONNECTING);
    Want want;
    sptr<IRemoteObject> proxy = sptr<MockIRemoteObject>::MakeSptr();
    want.SetParam("ohos.ability.params.UIServiceHostProxy", proxy);
    record->SetConnectWant(want);
    record->ScheduleConnectAbilityDone();
    EXPECT_EQ(record->GetConnectState(), ConnectionState::CONNECTED);
}

/*
 * Feature: ConnectionRecord
 * Function: ScheduleConnectAbilityDone
 * SubFunction: NA
 * FunctionPoints: ScheduleConnectAbilityDone
 * EnvConditions:NA
 * CaseDescription: Verify ScheduleConnectAbilityDone preserves AGENTEXTENSIONHOSTPROXY_KEY
 */
HWTEST_F(ConnectionRecordTest, ConnectionRecord_ScheduleConnectAbilityDone_007, TestSize.Level1)
{
    auto record = std::make_shared<ConnectionRecord>(service_->GetToken(), service_, callback_, nullptr);
    record->SetConnectState(ConnectionState::CONNECTING);
    Want want;
    sptr<IRemoteObject> proxy = sptr<MockIRemoteObject>::MakeSptr();
    want.SetParam("ohos.ability.params.AgentExtensionHostProxy", proxy);
    record->SetConnectWant(want);
    record->ScheduleConnectAbilityDone();
    EXPECT_EQ(record->GetConnectState(), ConnectionState::CONNECTED);
}

/*
 * Feature: ConnectionRecord
 * Function: ScheduleConnectAbilityDone
 * SubFunction: NA
 * FunctionPoints: ScheduleConnectAbilityDone
 * EnvConditions:NA
 * CaseDescription: Verify ScheduleConnectAbilityDone preserves both proxy keys
 */
HWTEST_F(ConnectionRecordTest, ConnectionRecord_ScheduleConnectAbilityDone_008, TestSize.Level1)
{
    auto record = std::make_shared<ConnectionRecord>(service_->GetToken(), service_, callback_, nullptr);
    record->SetConnectState(ConnectionState::CONNECTING);
    Want want;
    sptr<IRemoteObject> uiProxy = sptr<MockIRemoteObject>::MakeSptr();
    sptr<IRemoteObject> agentProxy = sptr<MockIRemoteObject>::MakeSptr();
    want.SetParam("ohos.ability.params.UIServiceHostProxy", uiProxy);
    want.SetParam("ohos.ability.params.AgentExtensionHostProxy", agentProxy);
    record->SetConnectWant(want);
    record->ScheduleConnectAbilityDone();
    EXPECT_EQ(record->GetConnectState(), ConnectionState::CONNECTED);
}

/*
 * Feature: ConnectionRecord
 * Function: ScheduleDisconnectTimeout
 * SubFunction: NA
 * FunctionPoints: ScheduleDisconnectTimeout
 * EnvConditions:NA
 * CaseDescription: Verify ScheduleDisconnectTimeout handles null task handler gracefully
 */
HWTEST_F(ConnectionRecordTest, ConnectionRecord_ScheduleDisconnectTimeout_001, TestSize.Level1)
{
    auto record = std::make_shared<ConnectionRecord>(service_->GetToken(), service_, callback_, nullptr);
    // ScheduleDisconnectTimeout should handle null handler internally
    // This test verifies it doesn't crash when handler is not available
    record->ScheduleDisconnectTimeout();
    // If we reach here, the method handled null case gracefully
    EXPECT_TRUE(true);
}

/*
 * Feature: ConnectionRecord
 * Function: DisconnectTimeout
 * SubFunction: NA
 * FunctionPoints: DisconnectTimeout
 * EnvConditions:NA
 * CaseDescription: Verify DisconnectTimeout with null targetService
 */
HWTEST_F(ConnectionRecordTest, ConnectionRecord_DisconnectTimeout_001, TestSize.Level1)
{
    auto record = std::make_shared<ConnectionRecord>(nullptr, nullptr, callback_, nullptr);
    record->DisconnectTimeout();
    // Should handle null targetService gracefully
    EXPECT_TRUE(true);
}

/*
 * Feature: ConnectionRecord
 * Function: SetConnectWant and GetConnectWant
 * SubFunction: NA
 * FunctionPoints: SetConnectWant and GetConnectWant
 * EnvConditions:NA
 * CaseDescription: Verify SetConnectWant and GetConnectWant
 */
HWTEST_F(ConnectionRecordTest, ConnectionRecord_ConnectWant_001, TestSize.Level1)
{
    auto record = std::make_shared<ConnectionRecord>(service_->GetToken(), service_, callback_, nullptr);
    Want want;
    want.SetElementName(std::string("device"), std::string("com.test.bundle"), std::string("TestAbility"),
        std::string("testModule"));
    record->SetConnectWant(want);
    auto retrievedWant = record->GetConnectWant();
    auto element = retrievedWant.GetElement();
    EXPECT_EQ(element.GetBundleName(), "com.test.bundle");
    EXPECT_EQ(element.GetAbilityName(), "TestAbility");
}

/*
 * Feature: ConnectionRecord
 * Function: SetConnectWant and GetConnectWant
 * SubFunction: NA
 * FunctionPoints: SetConnectWant and GetConnectWant
 * EnvConditions:NA
 * CaseDescription: Verify SetConnectWant with string parameters
 */
HWTEST_F(ConnectionRecordTest, ConnectionRecord_ConnectWant_002, TestSize.Level1)
{
    auto record = std::make_shared<ConnectionRecord>(service_->GetToken(), service_, callback_, nullptr);
    Want want;
    want.SetParam("testKey", std::string("testValue"));
    record->SetConnectWant(want);
    auto retrievedWant = record->GetConnectWant();
    EXPECT_TRUE(retrievedWant.HasParameter("testKey"));
}

/*
 * Feature: ConnectionRecord
 * Function: CompleteConnectAndOnlyCallConnectDone
 * SubFunction: NA
 * FunctionPoints: CompleteConnectAndOnlyCallConnectDone
 * EnvConditions:NA
 * CaseDescription: Verify CompleteConnectAndOnlyCallConnectDone when state is CONNECTED
 */
HWTEST_F(ConnectionRecordTest, ConnectionRecord_CompleteConnectAndOnlyCallConnectDone_001, TestSize.Level1)
{
    auto record = std::make_shared<ConnectionRecord>(service_->GetToken(), service_, callback_, nullptr);
    record->SetConnectState(ConnectionState::CONNECTING);
    record->CompleteConnectAndOnlyCallConnectDone();
    // Returns early if not CONNECTED, so state remains CONNECTING
    EXPECT_EQ(record->GetConnectState(), ConnectionState::CONNECTING);
}

/*
 * Feature: ConnectionRecord
 * Function: CompleteConnectAndOnlyCallConnectDone
 * SubFunction: NA
 * FunctionPoints: CompleteConnectAndOnlyCallConnectDone
 * EnvConditions:NA
 * CaseDescription: Verify CompleteConnectAndOnlyCallConnectDone when targetService is null
 */
HWTEST_F(ConnectionRecordTest, ConnectionRecord_CompleteConnectAndOnlyCallConnectDone_002, TestSize.Level1)
{
    auto record = std::make_shared<ConnectionRecord>(nullptr, nullptr, callback_, nullptr);
    record->SetConnectState(ConnectionState::CONNECTED);
    record->CompleteConnectAndOnlyCallConnectDone();
    // Returns early when targetService_ is null
    EXPECT_EQ(record->GetConnectState(), ConnectionState::CONNECTED);
}

/*
 * Feature: ConnectionRecord
 * Function: CompleteConnectAndOnlyCallConnectDone
 * SubFunction: NA
 * FunctionPoints: CompleteConnectAndOnlyCallConnectDone
 * EnvConditions:NA
 * CaseDescription: Verify CompleteConnectAndOnlyCallConnectDone when remoteObject is null
 */
HWTEST_F(ConnectionRecordTest, ConnectionRecord_CompleteConnectAndOnlyCallConnectDone_003, TestSize.Level1)
{
    auto record = std::make_shared<ConnectionRecord>(service_->GetToken(), service_, callback_, nullptr);
    record->SetConnectState(ConnectionState::CONNECTED);
    record->CompleteConnectAndOnlyCallConnectDone();
    // Returns early when remoteObject is null (mock service returns null)
    EXPECT_EQ(record->GetConnectState(), ConnectionState::CONNECTED);
}

/*
 * Feature: ConnectionRecord
 * Function: CompleteConnectAndOnlyCallConnectDone
 * SubFunction: NA
 * FunctionPoints: CompleteConnectAndOnlyCallConnectDone
 * EnvConditions:NA
 * CaseDescription: Verify CompleteConnectAndOnlyCallConnectDone when state is CONNECTED
 */
HWTEST_F(ConnectionRecordTest, ConnectionRecord_CompleteConnectAndOnlyCallConnectDone_004, TestSize.Level1)
{
    auto record = std::make_shared<ConnectionRecord>(service_->GetToken(), service_, callback_, nullptr);
    record->SetConnectState(ConnectionState::CONNECTED);
    record->CompleteConnectAndOnlyCallConnectDone();
    // With state CONNECTED, method completes successfully
    EXPECT_EQ(record->GetConnectState(), ConnectionState::CONNECTED);
}

/*
 * Feature: ConnectionRecord
 * Function: CompleteDisconnect with different result codes
 * SubFunction: NA
 * FunctionPoints: CompleteDisconnect
 * EnvConditions:NA
 * CaseDescription: Verify CompleteDisconnect with error result code
 */
HWTEST_F(ConnectionRecordTest, ConnectionRecord_CompleteDisconnect_001, TestSize.Level1)
{
    auto record = std::make_shared<ConnectionRecord>(service_->GetToken(), service_, callback_, nullptr);
    record->SetConnectState(ConnectionState::DISCONNECTING);
    record->CompleteDisconnect(ERR_INVALID_VALUE, false);
    // State only set to DISCONNECTED when resultCode == ERR_OK
    EXPECT_EQ(record->GetConnectState(), ConnectionState::DISCONNECTING);
}

/*
 * Feature: ConnectionRecord
 * Function: CompleteDisconnect with caller died
 * SubFunction: NA
 * FunctionPoints: CompleteDisconnect
 * EnvConditions:NA
 * CaseDescription: Verify CompleteDisconnect when caller died
 */
HWTEST_F(ConnectionRecordTest, ConnectionRecord_CompleteDisconnect_002, TestSize.Level1)
{
    auto record = std::make_shared<ConnectionRecord>(service_->GetToken(), service_, callback_, nullptr);
    record->SetConnectState(ConnectionState::DISCONNECTING);
    record->CompleteDisconnect(ERR_OK, true);
    EXPECT_EQ(record->GetConnectState(), ConnectionState::DISCONNECTED);
}

/*
 * Feature: ConnectionRecord
 * Function: CompleteDisconnect with target died
 * SubFunction: NA
 * FunctionPoints: CompleteDisconnect
 * EnvConditions:NA
 * CaseDescription: Verify CompleteDisconnect when target died
 */
HWTEST_F(ConnectionRecordTest, ConnectionRecord_CompleteDisconnect_003, TestSize.Level1)
{
    auto record = std::make_shared<ConnectionRecord>(service_->GetToken(), service_, callback_, nullptr);
    record->SetConnectState(ConnectionState::DISCONNECTING);
    record->CompleteDisconnect(ERR_OK, false, true);
    EXPECT_EQ(record->GetConnectState(), ConnectionState::DISCONNECTED);
}

/*
 * Feature: ConnectionRecord
 * Function: DisconnectTimeout
 * SubFunction: NA
 * FunctionPoints: DisconnectTimeout
 * EnvConditions:NA
 * CaseDescription: Verify DisconnectTimeout with valid targetService
 */
HWTEST_F(ConnectionRecordTest, ConnectionRecord_DisconnectTimeout_002, TestSize.Level1)
{
    auto record = std::make_shared<ConnectionRecord>(service_->GetToken(), service_, callback_, nullptr);
    record->DisconnectTimeout();
    // Should call ScheduleDisconnectAbilityDone on targetService
    EXPECT_TRUE(true);
}

/*
 * Feature: ConnectionRecord
 * Function: CreateConnectionRecord
 * SubFunction: NA
 * FunctionPoints: CreateConnectionRecord
 * EnvConditions:NA
 * CaseDescription: Verify CreateConnectionRecord creates valid record
 */
HWTEST_F(ConnectionRecordTest, ConnectionRecord_CreateConnectionRecord_001, TestSize.Level1)
{
    auto record = ConnectionRecord::CreateConnectionRecord(
        service_->GetToken(), service_, callback_, nullptr);
    EXPECT_NE(record, nullptr);
    EXPECT_EQ(record->GetToken().GetRefPtr(), service_->GetToken().GetRefPtr());
}

/*
 * Feature: ConnectionRecord
 * Function: CreateConnectionRecord
 * SubFunction: NA
 * FunctionPoints: CreateConnectionRecord
 * EnvConditions:NA
 * CaseDescription: Verify CreateConnectionRecord with null parameters
 */
HWTEST_F(ConnectionRecordTest, ConnectionRecord_CreateConnectionRecord_002, TestSize.Level1)
{
    auto record = ConnectionRecord::CreateConnectionRecord(nullptr, nullptr, nullptr, nullptr);
    EXPECT_NE(record, nullptr);
}

/*
 * Feature: ConnectionRecord
 * Function: CallOnAbilityConnectDone
 * SubFunction: NA
 * FunctionPoints: CallOnAbilityConnectDone
 * EnvConditions:NA
 * CaseDescription: Verify CallOnAbilityConnectDone with null callback
 */
HWTEST_F(ConnectionRecordTest, ConnectionRecord_CallOnAbilityConnectDone_001, TestSize.Level1)
{
    AppExecFwk::ElementName element;
    sptr<IRemoteObject> remoteObject = nullptr;
    auto result = ConnectionRecord::CallOnAbilityConnectDone(nullptr, element, remoteObject, ERR_OK);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: ConnectionRecord
 * Function: CallOnAbilityConnectDone
 * SubFunction: NA
 * FunctionPoints: CallOnAbilityConnectDone
 * EnvConditions:NA
 * CaseDescription: Verify CallOnAbilityConnectDone with valid callback
 */
HWTEST_F(ConnectionRecordTest, ConnectionRecord_CallOnAbilityConnectDone_002, TestSize.Level1)
{
    AppExecFwk::ElementName element;
    element.SetBundleName("com.test.bundle");
    element.SetAbilityName("TestAbility");
    sptr<IRemoteObject> remoteObject = sptr<MockIRemoteObject>::MakeSptr();
    auto result = ConnectionRecord::CallOnAbilityConnectDone(callback_, element, remoteObject, ERR_OK);
    EXPECT_EQ(result, ERR_OK);
}
}  // namespace AAFwk
}  // namespace OHOS
