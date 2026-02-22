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
#include "ability_manager_stub_impl_mock.h"
#include "ability_scheduler.h"
#include "app_debug_listener_stub_mock.h"
#include "hilog_tag_wrapper.h"
#include "iremote_proxy.h"
#include "mock_ability_connect_callback.h"
#include "mock_ability_token.h"
#include "mock_sa_interceptor_stub.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AAFwk {
namespace {
const int USER_ID = 100;
}  // namespace

class AbilityManagerStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void WriteInterfaceToken(MessageParcel& data);
    sptr<AbilityManagerStubImplMock> stub_{ nullptr };
};

void AbilityManagerStubTest::SetUpTestCase(void)
{}
void AbilityManagerStubTest::TearDownTestCase(void)
{}
void AbilityManagerStubTest::TearDown()
{}

void AbilityManagerStubTest::SetUp()
{
    stub_ = new AbilityManagerStubImplMock();
}

void AbilityManagerStubTest::WriteInterfaceToken(MessageParcel& data)
{
    data.WriteInterfaceToken(AbilityManagerStub::GetDescriptor());
}

/**
 * @tc.name: AbilityManagerStub_DumpSysStateInner_0100
 * @tc.desc: DumpSysStateInner
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_DumpSysStateInner_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerStub_DumpSysStateInner_0100 start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);

    std::string args = "-a";
    data.WriteString16(Str8ToStr16(args));

    bool isClient = false;
    data.WriteBool(isClient);

    bool isUserID = true;
    data.WriteBool(isUserID);

    data.WriteInt32(USER_ID);

    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::DUMPSYS_STATE),
        data, reply, option);
    EXPECT_EQ(res, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerStub_DumpSysStateInner_0100 end");
}  // namespace AAFwk

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is START_ABILITY
 * CaseDescription: Verify that on remote request is normal and abnormal
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    Want want;
    WriteInterfaceToken(data);
    want.SetFlags(10);
    data.WriteParcelable(&want);
    data.WriteInt32(1);
    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY),
        data, reply, option);

    EXPECT_EQ(res, NO_ERROR);

    data.WriteParcelable(nullptr);
    data.WriteInt32(1);
    int res1 = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY),
        data, reply, option);
    EXPECT_NE(res1, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is TERMINATE_ABILITY
 * CaseDescription: Verify that on remote request is normal and abnormal
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_002, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    Want want;
    want.SetFlags(10);
    OHOS::sptr<IRemoteObject> token = nullptr;
    WriteInterfaceToken(data);
    data.WriteParcelable(token);
    data.WriteParcelable(&want);
    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::TERMINATE_ABILITY),
        data, reply, option);

    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is CONNECT_ABILITY
 * CaseDescription: Verify that on remote request is normal
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_004, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    Want want;
    want.SetFlags(10);
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    sptr<IRemoteObject> callerToken = nullptr;
    WriteInterfaceToken(data);
    data.WriteParcelable(&want);
    data.WriteParcelable(connect->AsObject());
    data.WriteParcelable(callerToken);
    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::CONNECT_ABILITY),
        data, reply, option);

    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: OnRemoteRequest static_cast<uint32_t>(AbilityManagerInterfaceCode::CONNECT_ABILITY
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_005, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    Want want;
    want.SetFlags(10);
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    sptr<IRemoteObject> callerToken = nullptr;
    WriteInterfaceToken(data);
    data.WriteParcelable(&want);
    data.WriteParcelable(connect->AsObject());
    data.WriteParcelable(callerToken);
    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::CONNECT_ABILITY),
        data, reply, option);

    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: OnRemoteRequest static_cast<uint32_t>(AbilityManagerInterfaceCode::CONNECT_ABILITY
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_006, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    sptr<IRemoteObject> callerToken = nullptr;
    WriteInterfaceToken(data);
    data.WriteParcelable(nullptr);
    data.WriteParcelable(connect->AsObject());
    data.WriteParcelable(callerToken);
    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::CONNECT_ABILITY),
        data, reply, option);

    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is DISCONNECT_ABILITY
 * CaseDescription: Verify that on remote request is normal
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_007, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    WriteInterfaceToken(data);
    data.WriteParcelable(connect->AsObject());
    stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::DISCONNECT_ABILITY),
        data, reply, option);

    EXPECT_TRUE(stub_ != nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is ATTACH_ABILITY_THREAD
 * CaseDescription: Verify that on remote request is normal
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_008, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    sptr<IAbilityScheduler> scheduler = new AbilityScheduler();
    sptr<IRemoteObject> token = nullptr;
    WriteInterfaceToken(data);
    data.WriteParcelable(scheduler->AsObject());
    data.WriteParcelable(token);
    stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::ATTACH_ABILITY_THREAD),
        data, reply, option);

    EXPECT_TRUE(stub_ != nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is ABILITY_TRANSITION_DONE
 * CaseDescription: Verify that on remote request is normal
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_009, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    sptr<IRemoteObject> token = sptr<AppExecFwk::MockAbilityToken>(new (std::nothrow) AppExecFwk::MockAbilityToken());
    WriteInterfaceToken(data);
    bool ret = data.WriteRemoteObject(token);
    ret |= data.WriteInt32(1);
    PacMap pMap;
    pMap.PutIntValue(std::string("1"), 1);
    ret |= data.WriteParcelable(&pMap);
    if (ret) {
        int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::ABILITY_TRANSITION_DONE),
        data, reply, option);
        EXPECT_EQ(res, NO_ERROR);
    }
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is CONNECT_ABILITY_DONE
 * CaseDescription: Verify that on remote request is normal
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_010, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> remoteObject = nullptr;
    WriteInterfaceToken(data);
    data.WriteParcelable(token);
    data.WriteParcelable(remoteObject);
    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::CONNECT_ABILITY_DONE),
        data, reply, option);

    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is DISCONNECT_ABILITY_DONE
 * CaseDescription: Verify that on remote request is normal
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_011, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    sptr<IRemoteObject> token = nullptr;
    WriteInterfaceToken(data);
    data.WriteParcelable(token);
    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::DISCONNECT_ABILITY_DONE),
        data, reply, option);

    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is SEND_RESULT_TO_ABILITY
 * CaseDescription: Verify that on remote request is normal
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_012, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    int requestCode = -1;
    int resultCode = -1;
    Want want;
    data.WriteInt32(requestCode);
    data.WriteInt32(resultCode);
    data.WriteParcelable(&want);
    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::SEND_RESULT_TO_ABILITY),
        data, reply, option);

    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is DUMP_STATE
 * CaseDescription: Verify that on remote request is normal
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_013, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    std::string args = "aaa";
    std::vector<std::string> info;
    WriteInterfaceToken(data);
    data.WriteString16(Str8ToStr16(args));
    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::DUMP_STATE),
        data, reply, option);

    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is default
 * CaseDescription: Verify that on remote request is normal
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_016, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);
    int res = stub_->OnRemoteRequest(5000, data, reply, option);

    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is START_CALL_ABILITY
 * CaseDescription: Verify that on remote request is normal
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_018, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    Want want;
    WriteInterfaceToken(data);
    want.SetFlags(10);
    data.WriteParcelable(&want);
    stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_CALL_ABILITY),
        data, reply, option);

    EXPECT_TRUE(stub_ != nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is START_CALL_ABILITY
 * CaseDescription: Verify that on remote request is ERR_INVALID_VALUE
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_019, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);
    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_CALL_ABILITY),
        data, reply, option);

    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is START_CALL_ABILITY
 * CaseDescription: Verify that on remote request is normal
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_020, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);
    AppExecFwk::ElementName element;
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    data.WriteParcelable(connect->AsObject());
    data.WriteParcelable(&element);
    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::RELEASE_CALL_ABILITY),
        data, reply, option);

    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is START_CALL_ABILITY
 * CaseDescription: Verify that on remote request is ERR_INVALID_VALUE
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_021, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);
    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::RELEASE_CALL_ABILITY),
        data, reply, option);

    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is GET_ABILITY_MANAGER_COLLABORATOR
 * CaseDescription: Verify that getting the collaborator returns the expected error when none is installed
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetAbilityManagerCollaborator_OnRemote_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    int res = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_ABILITY_MANAGER_COLLABORATOR),
        data, reply, option);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is GET_ALL_INSIGHT_INTENT_INFO
 * CaseDescription: Verify that requesting insight intent infos succeeds
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetAllInsightIntentInfo_OnRemote_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    auto flag = AbilityRuntime::GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT;
    data.WriteUint32(static_cast<uint32_t>(flag));
    data.WriteInt32(USER_ID);
    int res = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_ALL_INSIGHT_INTENT_INFO),
        data, reply, option);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is GET_INSIGHT_INTENT_INFO_BY_BUNDLE_NAME
 * CaseDescription: Verify that requesting insight intent info by bundle name succeeds
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetInsightIntentInfoByBundleName_OnRemote_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    auto flag = AbilityRuntime::GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT;
    std::string bundleName = "com.ohos.test";
    data.WriteUint32(static_cast<uint32_t>(flag));
    data.WriteString(bundleName);
    data.WriteInt32(USER_ID);
    int res = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_INSIGHT_INTENT_INFO_BY_BUNDLE_NAME),
        data, reply, option);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is GET_INSIGHT_INTENT_INFO_BY_INTENT_NAME
 * CaseDescription: Verify that requesting insight intent info by intent name succeeds
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetInsightIntentInfoByIntentName_OnRemote_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    auto flag = AbilityRuntime::GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT;
    std::string bundleName = "com.ohos.test";
    std::string moduleName = "module";
    std::string intentName = "intent";
    data.WriteUint32(static_cast<uint32_t>(flag));
    data.WriteString(bundleName);
    data.WriteString(moduleName);
    data.WriteString(intentName);
    data.WriteInt32(USER_ID);
    int res = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_INSIGHT_INTENT_INFO_BY_INTENT_NAME),
        data, reply, option);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is REGISTER_SA_INTERCEPTOR
 * CaseDescription: Verify that registering an SA interceptor handles null and valid interceptors
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_RegisterSAInterceptor_OnRemote_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    int res = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_SA_INTERCEPTOR),
        data, reply, option);
    EXPECT_EQ(res, ERR_NULL_SA_INTERCEPTOR_EXECUTER);

    MessageParcel data2;
    MessageParcel reply2;
    WriteInterfaceToken(data2);
    sptr<AbilityRuntime::ISAInterceptor> interceptor = new AbilityRuntime::MockSAInterceptorStub(0);
    EXPECT_TRUE(data2.WriteRemoteObject(interceptor->AsObject()));
    res = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_SA_INTERCEPTOR),
        data2, reply2, option);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is SEND_LOCAL_PENDING_WANT_SENDER
 * CaseDescription: Verify that sending a local want sender succeeds
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_SendLocalPendingWantSender_OnRemote_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    data.WriteParcelable(nullptr);
    int res = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::SEND_LOCAL_PENDING_WANT_SENDER),
        data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is START_SELF_UI_ABILITY_IN_CURRENT_PROCESS
 * CaseDescription: Verify that starting a self UI ability in the current process succeeds
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartSelfUIAbilityInCurrentProcess_OnRemote_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    Want want;
    data.WriteParcelable(&want);
    std::string specifiedFlag = "flag";
    data.WriteString(specifiedFlag);
    StartOptions startOptions;
    data.WriteParcelable(&startOptions);
    data.WriteBool(true);
    data.WriteBool(false);
    int res = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_SELF_UI_ABILITY_IN_CURRENT_PROCESS),
        data, reply, option);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is GET_ABILITY_RUNNING_INFO
 * CaseDescription: Verify ability running info request succeeds
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetAbilityRunningInfo_OnRemote_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_ABILITY_RUNNING_INFO),
        data, reply, option);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is GET_EXTENSION_RUNNING_INFO
 * CaseDescription: Verify extension running info request succeeds
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetExtensionRunningInfo_OnRemote_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_EXTENSION_RUNNING_INFO),
        data, reply, option);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is GET_PROCESS_RUNNING_INFO
 * CaseDescription: Verify process running info request succeeds
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetProcessRunningInfo_OnRemote_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PROCESS_RUNNING_INFO),
        data, reply, option);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is GET_INTENT_EXEMPTION_INFO
 * CaseDescription: Verify intent exemption info request succeeds
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetIntentExemptionInfo_OnRemote_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_INTENT_EXEMPTION_INFO),
        data, reply, option);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is START_UI_EXTENSION_ABILITY
 * CaseDescription: Verify that starting a UI extension ability succeeds
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartUIExtensionAbility_OnRemote_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    data.WriteBool(false);
    data.WriteInt32(USER_ID);

    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_EXTENSION_ABILITY),
        data, reply, option);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is START_UI_EXTENSION_ABILITY_EMBEDDED
 * CaseDescription: Verify that starting an embedded UI extension ability succeeds
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartUIExtensionAbilityEmbedded_OnRemote_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    data.WriteBool(false);
    data.WriteInt32(USER_ID);

    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(
        AbilityManagerInterfaceCode::START_UI_EXTENSION_ABILITY_EMBEDDED), data, reply, option);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is START_UI_EXTENSION_CONSTRAINED_EMBEDDED
 * CaseDescription: Verify that starting a constrained embedded UI extension ability succeeds
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartUIExtensionAbility0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    data.WriteBool(false);
    data.WriteInt32(USER_ID);

    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(
        AbilityManagerInterfaceCode::START_UI_EXTENSION_CONSTRAINED_EMBEDDED), data, reply, option);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is START_UI_EXTENSION_PRE_VIEW_EMBEDDED
 * CaseDescription: Verify that starting a preview embedded UI extension ability succeeds
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartUIExtensionPreViewEmbedded_OnRemote_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    data.WriteBool(false);
    data.WriteInt32(USER_ID);

    int res = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_EXTENSION_PRE_VIEW_EMBEDDED), data, reply, option);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is PRELOAD_APPLICATION
 * CaseDescription: Verify that preloading an application succeeds
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_PreloadApplication_OnRemote_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    data.WriteString("com.ohos.test");
    data.WriteInt32(USER_ID);
    data.WriteInt32(0);

    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::PRELOAD_APPLICATION),
        data, reply, option);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is START_SELF_UI_ABILITY_WITH_PID_RESULT
 * CaseDescription: Verify that starting a self UI ability with PID result succeeds
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartSelfUIAbilityWithPidResult_OnRemote_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    Want want;
    StartOptions startOptions;
    data.WriteParcelable(&want);
    data.WriteParcelable(&startOptions);
    data.WriteUint64(0);

    int res = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_SELF_UI_ABILITY_WITH_PID_RESULT), data, reply, option);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: GetTopAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetTopAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetTopAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetTopAbilityInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetTopAbilityInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: GetElementNameByTokenInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetElementNameByTokenInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetElementNameByTokenInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetElementNameByTokenInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetElementNameByTokenInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: MoveAbilityToBackgroundInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MoveAbilityToBackgroundInner
 * EnvConditions: NA
 * CaseDescription: Verify the function MoveAbilityToBackgroundInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_MoveAbilityToBackgroundInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->MoveAbilityToBackgroundInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: MoveUIAbilityToBackgroundInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MoveUIAbilityToBackgroundInner
 * EnvConditions: NA
 * CaseDescription: Verify the function MoveUIAbilityToBackgroundInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_MoveUIAbilityToBackgroundInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->MoveUIAbilityToBackgroundInner(data, reply);
    EXPECT_EQ(res, IPC_STUB_ERR);
}

/*
 * Feature: AbilityManagerService
 * Function: TerminateAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function TerminateAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_TerminateAbilityInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->TerminateAbilityInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: TerminateUIExtensionAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateUIExtensionAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function TerminateUIExtensionAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_TerminateUIExtensionAbilityInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->TerminateUIExtensionAbilityInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: SendResultToAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SendResultToAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function SendResultToAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_SendResultToAbilityInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->SendResultToAbilityInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: MinimizeAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MinimizeAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function MinimizeAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_MinimizeAbilityInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->MinimizeAbilityInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: MinimizeUIExtensionAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MinimizeUIExtensionAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function MinimizeUIExtensionAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_MinimizeUIExtensionAbilityInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->MinimizeUIExtensionAbilityInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: MinimizeUIAbilityBySCBInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MinimizeUIAbilityBySCBInner
 * EnvConditions: NA
 * CaseDescription: Verify the function MinimizeUIAbilityBySCBInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_MinimizeUIAbilityBySCBInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->MinimizeUIAbilityBySCBInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: AttachAbilityThreadInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AttachAbilityThreadInner
 * EnvConditions: NA
 * CaseDescription: Verify the function AttachAbilityThreadInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_AttachAbilityThreadInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    stub_->AttachAbilityThreadInner(data, reply);
    EXPECT_TRUE(stub_ != nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: AbilityTransitionDoneInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AbilityTransitionDoneInner
 * EnvConditions: NA
 * CaseDescription: Verify the function AbilityTransitionDoneInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_AbilityTransitionDoneInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->AbilityTransitionDoneInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleConnectAbilityDoneInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleConnectAbilityDoneInner
 * EnvConditions: NA
 * CaseDescription: Verify the function ScheduleConnectAbilityDoneInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_ScheduleConnectAbilityDoneInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->ScheduleConnectAbilityDoneInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleDisconnectAbilityDoneInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleDisconnectAbilityDoneInner
 * EnvConditions: NA
 * CaseDescription: Verify the function ScheduleDisconnectAbilityDoneInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_ScheduleDisconnectAbilityDoneInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->ScheduleDisconnectAbilityDoneInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleCommandAbilityDoneInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleCommandAbilityDoneInner
 * EnvConditions: NA
 * CaseDescription: Verify the function ScheduleCommandAbilityDoneInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_ScheduleCommandAbilityDoneInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->ScheduleCommandAbilityDoneInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleCommandAbilityWindowDoneInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleCommandAbilityWindowDoneInner
 * EnvConditions: NA
 * CaseDescription: Verify the function ScheduleCommandAbilityWindowDoneInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_ScheduleCommandAbilityWindowDoneInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->ScheduleCommandAbilityWindowDoneInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: AcquireDataAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AcquireDataAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function AcquireDataAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_AcquireDataAbilityInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->AcquireDataAbilityInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: ReleaseDataAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReleaseDataAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function ReleaseDataAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_ReleaseDataAbilityInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    stub_->ReleaseDataAbilityInner(data, reply);
    EXPECT_TRUE(stub_ != nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: KillProcessInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService KillProcessInner
 * EnvConditions: NA
 * CaseDescription: Verify the function KillProcessInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_KillProcessInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->KillProcessInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: UninstallAppInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UninstallAppInner
 * EnvConditions: NA
 * CaseDescription: Verify the function UninstallAppInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_UninstallAppInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->UninstallAppInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: UpgradeAppInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UpgradeAppInner
 * EnvConditions: NA
 * CaseDescription: Verify the function UpgradeAppInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_UpgradeAppInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->UpgradeAppInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}


/*
 * Feature: AbilityManagerService
 * Function: StartAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StartAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartAbilityInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StartAbilityInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityInnerSpecifyTokenId
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityInnerSpecifyTokenId
 * EnvConditions: NA
 * CaseDescription: Verify the function StartAbilityInnerSpecifyTokenId is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartAbilityInnerSpecifyTokenId_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StartAbilityInnerSpecifyTokenId(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByUIContentSessionAddCallerInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityByUIContentSessionAddCallerInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StartAbilityByUIContentSessionAddCallerInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartAbilityByUIContentSessionAddCallerInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StartAbilityByUIContentSessionAddCallerInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByUIContentSessionForOptionsInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityByUIContentSessionForOptionsInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StartAbilityByUIContentSessionForOptionsInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartAbilityByUIContentSessionForOptionsInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StartAbilityByUIContentSessionForOptionsInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartExtensionAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartExtensionAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StartExtensionAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartExtensionAbilityInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StartExtensionAbilityInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: RequestModalUIExtensionInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RequestModalUIExtensionInner
 * EnvConditions: NA
 * CaseDescription: Verify the function RequestModalUIExtensionInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_RequestModalUIExtensionInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->RequestModalUIExtensionInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: PreloadUIExtensionAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService PreloadUIExtensionAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function PreloadUIExtensionAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_PreloadUIExtensionAbilityInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->PreloadUIExtensionAbilityInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIExtensionAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIExtensionAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StartUIExtensionAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartUIExtensionAbilityInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StartUIExtensionAbilityInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIExtensionAbilityEmbeddedInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIExtensionAbilityEmbeddedInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StartUIExtensionAbilityEmbeddedInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartUIExtensionAbilityEmbeddedInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StartUIExtensionAbilityEmbeddedInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIExtensionConstrainedEmbeddedInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIExtensionConstrainedEmbeddedInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StartUIExtensionConstrainedEmbeddedInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartUIExtensionConstrainedEmbeddedInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StartUIExtensionConstrainedEmbeddedInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIExtensionPreViewEmbeddedInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIExtensionPreViewEmbeddedInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StartUIExtensionPreViewEmbeddedInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartUIExtensionPreViewEmbeddedInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StartUIExtensionPreViewEmbeddedInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StopExtensionAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopExtensionAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StopExtensionAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StopExtensionAbilityInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StopExtensionAbilityInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityAddCallerInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityAddCallerInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StartAbilityAddCallerInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartAbilityAddCallerInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StartAbilityAddCallerInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityAsCallerByTokenInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityAsCallerByTokenInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StartAbilityAsCallerByTokenInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartAbilityAsCallerByTokenInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StartAbilityAsCallerByTokenInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityAsCallerForOptionInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityAsCallerForOptionInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StartAbilityAsCallerForOptionInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartAbilityAsCallerForOptionInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StartAbilityAsCallerForOptionInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityForResultAsCallerInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityForResultAsCallerInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StartAbilityForResultAsCallerInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartAbilityForResultAsCallerInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StartAbilityForResultAsCallerInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityForResultAsCallerForOptionsInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityForResultAsCallerForOptionsInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StartAbilityForResultAsCallerForOptionsInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartAbilityForResultAsCallerForOptionsInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StartAbilityForResultAsCallerForOptionsInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function ConnectAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_ConnectAbilityInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->ConnectAbilityInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectAbilityWithTypeInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectAbilityWithTypeInner
 * EnvConditions: NA
 * CaseDescription: Verify the function ConnectAbilityWithTypeInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_ConnectAbilityWithTypeInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->ConnectAbilityWithTypeInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectUIExtensionAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectUIExtensionAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function ConnectUIExtensionAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_ConnectUIExtensionAbilityInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->ConnectUIExtensionAbilityInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: DisconnectAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DisconnectAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function DisconnectAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_DisconnectAbilityInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    stub_->DisconnectAbilityInner(data, reply);
    EXPECT_TRUE(stub_ != nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: StopServiceAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopServiceAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StopServiceAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StopServiceAbilityInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StopServiceAbilityInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: DumpSysStateInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DumpSysStateInner
 * EnvConditions: NA
 * CaseDescription: Verify the function DumpSysStateInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_DumpSysStateInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->DumpSysStateInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: DumpStateInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DumpStateInner
 * EnvConditions: NA
 * CaseDescription: Verify the function DumpStateInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_DumpStateInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->DumpStateInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityForSettingsInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityForSettingsInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StartAbilityForSettingsInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartAbilityForSettingsInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StartAbilityForSettingsInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityForOptionsInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityForOptionsInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StartAbilityForOptionsInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartAbilityForOptionsInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StartAbilityForOptionsInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: CloseUIAbilityBySCBInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CloseUIAbilityBySCBInner
 * EnvConditions: NA
 * CaseDescription: Verify the function CloseUIAbilityBySCBInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_CloseUIAbilityBySCBInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->CloseUIAbilityBySCBInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: GetWantSenderInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetWantSenderInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetWantSenderInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetWantSenderInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetWantSenderInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: SendWantSenderInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SendWantSenderInner
 * EnvConditions: NA
 * CaseDescription: Verify the function SendWantSenderInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_SendWantSenderInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->SendWantSenderInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: CancelWantSenderInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CancelWantSenderInner
 * EnvConditions: NA
 * CaseDescription: Verify the function CancelWantSenderInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_CancelWantSenderInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->CancelWantSenderInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: GetPendingWantUidInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingWantUidInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetPendingWantUidInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetPendingWantUidInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetPendingWantUidInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: GetPendingWantUserIdInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingWantUserIdInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetPendingWantUserIdInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetPendingWantUserIdInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetPendingWantUserIdInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: GetPendingWantBundleNameInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingWantBundleNameInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetPendingWantBundleNameInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetPendingWantBundleNameInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetPendingWantBundleNameInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: GetPendingWantCodeInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingWantCodeInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetPendingWantCodeInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetPendingWantCodeInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetPendingWantCodeInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: GetPendingWantTypeInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingWantTypeInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetPendingWantTypeInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetPendingWantTypeInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetPendingWantTypeInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterCancelListenerInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterCancelListenerInner
 * EnvConditions: NA
 * CaseDescription: Verify the function RegisterCancelListenerInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_RegisterCancelListenerInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->RegisterCancelListenerInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: UnregisterCancelListenerInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UnregisterCancelListenerInner
 * EnvConditions: NA
 * CaseDescription: Verify the function UnregisterCancelListenerInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_UnregisterCancelListenerInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->UnregisterCancelListenerInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: GetPendingRequestWantInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingRequestWantInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetPendingRequestWantInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetPendingRequestWantInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetPendingRequestWantInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: GetWantSenderInfoInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetWantSenderInfoInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetWantSenderInfoInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetWantSenderInfoInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetWantSenderInfoInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: GetAppMemorySizeInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetAppMemorySizeInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetAppMemorySizeInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetAppMemorySizeInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetAppMemorySizeInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: IsRamConstrainedDeviceInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsRamConstrainedDeviceInner
 * EnvConditions: NA
 * CaseDescription: Verify the function IsRamConstrainedDeviceInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_IsRamConstrainedDeviceInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->IsRamConstrainedDeviceInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: ContinueMissionInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ContinueMissionInner
 * EnvConditions: NA
 * CaseDescription: Verify the function ContinueMissionInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_ContinueMissionInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->ContinueMissionInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: AbilityManagerService
 * Function: ContinueMissionOfBundleNameInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ContinueMissionOfBundleNameInner
 * EnvConditions: NA
 * CaseDescription: Verify the function ContinueMissionOfBundleNameInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_ContinueMissionOfBundleNameInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->ContinueMissionOfBundleNameInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: AbilityManagerService
 * Function: ContinueMissionOfBundleNameInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ContinueMissionOfBundleNameInner
 * EnvConditions: NA
 * CaseDescription: Verify the function ContinueMissionOfBundleNameInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_ContinueMissionOfBundleNameInner_002, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    Want want;
    data.WriteParcelable(&want);
    auto res = stub_->ContinueMissionOfBundleNameInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: AbilityManagerService
 * Function: ContinueAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ContinueAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function ContinueAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_ContinueAbilityInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->ContinueAbilityInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartContinuationInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartContinuationInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StartContinuationInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartContinuationInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StartContinuationInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: AbilityManagerService
 * Function: NotifyCompleteContinuationInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService NotifyCompleteContinuationInner
 * EnvConditions: NA
 * CaseDescription: Verify the function NotifyCompleteContinuationInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_NotifyCompleteContinuationInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->NotifyCompleteContinuationInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: NotifyContinuationResultInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService NotifyContinuationResultInner
 * EnvConditions: NA
 * CaseDescription: Verify the function NotifyContinuationResultInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_NotifyContinuationResultInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->NotifyContinuationResultInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: LockMissionForCleanupInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService LockMissionForCleanupInner
 * EnvConditions: NA
 * CaseDescription: Verify the function LockMissionForCleanupInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_LockMissionForCleanupInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->LockMissionForCleanupInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: UnlockMissionForCleanupInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UnlockMissionForCleanupInner
 * EnvConditions: NA
 * CaseDescription: Verify the function UnlockMissionForCleanupInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_UnlockMissionForCleanupInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->UnlockMissionForCleanupInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: SetLockedStateInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetLockedStateInner
 * EnvConditions: NA
 * CaseDescription: Verify the function SetLockedStateInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_SetLockedStateInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->SetLockedStateInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterMissionListenerInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterMissionListenerInner
 * EnvConditions: NA
 * CaseDescription: Verify the function RegisterMissionListenerInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_RegisterMissionListenerInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->RegisterMissionListenerInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: UnRegisterMissionListenerInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UnRegisterMissionListenerInner
 * EnvConditions: NA
 * CaseDescription: Verify the function UnRegisterMissionListenerInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_UnRegisterMissionListenerInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->UnRegisterMissionListenerInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: GetMissionInfosInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetMissionInfosInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetMissionInfosInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetMissionInfosInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetMissionInfosInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: GetMissionInfoInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetMissionInfoInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetMissionInfoInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetMissionInfoInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetMissionInfoInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: CleanMissionInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CleanMissionInner
 * EnvConditions: NA
 * CaseDescription: Verify the function CleanMissionInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_CleanMissionInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->CleanMissionInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: CleanAllMissionsInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CleanAllMissionsInner
 * EnvConditions: NA
 * CaseDescription: Verify the function CleanAllMissionsInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_CleanAllMissionsInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->CleanAllMissionsInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: MoveMissionToFrontInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MoveMissionToFrontInner
 * EnvConditions: NA
 * CaseDescription: Verify the function MoveMissionToFrontInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_MoveMissionToFrontInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->MoveMissionToFrontInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: MoveMissionsToForegroundInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MoveMissionsToForegroundInner
 * EnvConditions: NA
 * CaseDescription: Verify the function MoveMissionsToForegroundInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_MoveMissionsToForegroundInner_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "%{public}s is called.", __func__);
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->MoveMissionsToForegroundInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: MoveMissionsToBackgroundInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MoveMissionsToBackgroundInner
 * EnvConditions: NA
 * CaseDescription: Verify the function MoveMissionsToBackgroundInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_MoveMissionsToBackgroundInner_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "%{public}s is called.", __func__);
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->MoveMissionsToBackgroundInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: GetMissionIdByTokenInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetMissionIdByTokenInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetMissionIdByTokenInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetMissionIdByTokenInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetMissionIdByTokenInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: MoveMissionToFrontByOptionsInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MoveMissionToFrontByOptionsInner
 * EnvConditions: NA
 * CaseDescription: Verify the function MoveMissionToFrontByOptionsInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_MoveMissionToFrontByOptionsInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->MoveMissionToFrontByOptionsInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByCallInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityByCallInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StartAbilityByCallInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartAbilityByCallInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StartAbilityByCallInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIAbilityBySCBInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilityBySCBInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StartUIAbilityBySCBInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartUIAbilityBySCBInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StartUIAbilityBySCBInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: ReleaseCallInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReleaseCallInner
 * EnvConditions: NA
 * CaseDescription: Verify the function ReleaseCallInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_ReleaseCallInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->ReleaseCallInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUserInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUserInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StartUserInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartUserInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StartUserInner(data, reply);
    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StopUserInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopUserInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StopUserInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StopUserInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StopUserInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: LogoutUserInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService LogoutUserInner
 * EnvConditions: NA
 * CaseDescription: Verify the function LogoutUserInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_LogoutUserInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->LogoutUserInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: GetAbilityRunningInfosInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetAbilityRunningInfosInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetAbilityRunningInfosInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetAbilityRunningInfosInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetAbilityRunningInfosInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: GetExtensionRunningInfosInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetExtensionRunningInfosInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetExtensionRunningInfosInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetExtensionRunningInfosInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetExtensionRunningInfosInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: GetProcessRunningInfosInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetProcessRunningInfosInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetProcessRunningInfosInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetProcessRunningInfosInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetProcessRunningInfosInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartSyncRemoteMissionsInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartSyncRemoteMissionsInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StartSyncRemoteMissionsInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartSyncRemoteMissionsInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StartSyncRemoteMissionsInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StopSyncRemoteMissionsInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopSyncRemoteMissionsInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StopSyncRemoteMissionsInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StopSyncRemoteMissionsInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StopSyncRemoteMissionsInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterRemoteMissionListenerInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterRemoteMissionListenerInner
 * EnvConditions: NA
 * CaseDescription: Verify the function RegisterRemoteMissionListenerInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_RegisterRemoteMissionListenerInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->RegisterRemoteMissionListenerInner(data, reply);
    EXPECT_EQ(res, INVALID_PARAMETERS_ERR);
}

/*
 * Feature: AbilityManagerService
 * Function: UnRegisterRemoteMissionListenerInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UnRegisterRemoteMissionListenerInner
 * EnvConditions: NA
 * CaseDescription: Verify the function UnRegisterRemoteMissionListenerInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_UnRegisterRemoteMissionListenerInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->UnRegisterRemoteMissionListenerInner(data, reply);
    EXPECT_EQ(res, INVALID_PARAMETERS_ERR);
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterRemoteOnListenerInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterRemoteOnListenerInner
 * EnvConditions: NA
 * CaseDescription: Verify the function RegisterRemoteOnListenerInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_RegisterRemoteOnListenerInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->RegisterRemoteOnListenerInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/**
 * @tc.name: SetResidentProcessEnableInner_001
 * @tc.desc: SetResidentProcessEnableInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, SetResidentProcessEnableInner_001, TestSize.Level1)
{
    ASSERT_NE(stub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    auto result = stub_->SetResidentProcessEnableInner(data, reply);
    EXPECT_EQ(result, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterRemoteOffListenerInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterRemoteOffListenerInner
 * EnvConditions: NA
 * CaseDescription: Verify the function RegisterRemoteOffListenerInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_RegisterRemoteOffListenerInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->RegisterRemoteOffListenerInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterSnapshotHandlerInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterSnapshotHandlerInner
 * EnvConditions: NA
 * CaseDescription: Verify the function RegisterSnapshotHandlerInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_RegisterSnapshotHandlerInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->RegisterSnapshotHandlerInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: AbilityManagerService
 * Function: GetMissionSnapshotInfoInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetMissionSnapshotInfoInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetMissionSnapshotInfoInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetMissionSnapshotInfoInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetMissionSnapshotInfoInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: SetAbilityControllerInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetAbilityControllerInner
 * EnvConditions: NA
 * CaseDescription: Verify the function SetAbilityControllerInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_SetAbilityControllerInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->SetAbilityControllerInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: AbilityManagerService
 * Function: IsRunningInStabilityTestInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsRunningInStabilityTestInner
 * EnvConditions: NA
 * CaseDescription: Verify the function IsRunningInStabilityTestInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_IsRunningInStabilityTestInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->IsRunningInStabilityTestInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUserTestInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUserTestInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StartUserTestInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartUserTestInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StartUserTestInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: FinishUserTestInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService FinishUserTestInner
 * EnvConditions: NA
 * CaseDescription: Verify the function FinishUserTestInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_FinishUserTestInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->FinishUserTestInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: GetTopAbilityTokenInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetTopAbilityTokenInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetTopAbilityTokenInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetTopAbilityTokenInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetTopAbilityTokenInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerStub
 * Function: CheckUIExtensionIsFocusedInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckUIExtensionIsFocusedInner
 * EnvConditions: NA
 * CaseDescription: Verify the function CheckUIExtensionIsFocused is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_CheckUIExtensionIsFocusedInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->CheckUIExtensionIsFocusedInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}


/*
 * Feature: AbilityManagerService
 * Function: DelegatorDoAbilityForegroundInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DelegatorDoAbilityForegroundInner
 * EnvConditions: NA
 * CaseDescription: Verify the function DelegatorDoAbilityForegroundInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_DelegatorDoAbilityForegroundInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->DelegatorDoAbilityForegroundInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: DelegatorDoAbilityBackgroundInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DelegatorDoAbilityBackgroundInner
 * EnvConditions: NA
 * CaseDescription: Verify the function DelegatorDoAbilityBackgroundInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_DelegatorDoAbilityBackgroundInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->DelegatorDoAbilityBackgroundInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: DoAbilityForeground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DoAbilityForeground
 * EnvConditions: NA
 * CaseDescription: Verify the function DoAbilityForeground is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_DoAbilityForeground_001, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    uint32_t flag = 0;
    auto res = stub_->DoAbilityForeground(token, flag);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: DoAbilityBackground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DoAbilityBackground
 * EnvConditions: NA
 * CaseDescription: Verify the function DoAbilityBackground is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_DoAbilityBackground_001, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    uint32_t flag = 0;
    auto res = stub_->DoAbilityBackground(token, flag);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: DoAbilityForegroundInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DoAbilityForegroundInner
 * EnvConditions: NA
 * CaseDescription: Verify the function DoAbilityForegroundInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_DoAbilityForegroundInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->DoAbilityForegroundInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: DoAbilityBackgroundInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DoAbilityBackgroundInner
 * EnvConditions: NA
 * CaseDescription: Verify the function DoAbilityBackgroundInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_DoAbilityBackgroundInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->DoAbilityBackgroundInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterObserver
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterObserver
 * EnvConditions: NA
 * CaseDescription: Verify the function RegisterObserver is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_RegisterObserver_001, TestSize.Level1)
{
    sptr<AbilityRuntime::IConnectionObserver> observer = nullptr;
    auto res = stub_->RegisterObserver(observer);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: UnregisterObserver
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UnregisterObserver
 * EnvConditions: NA
 * CaseDescription: Verify the function UnregisterObserver is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_UnregisterObserver_001, TestSize.Level1)
{
    sptr<AbilityRuntime::IConnectionObserver> observer = nullptr;
    auto res = stub_->UnregisterObserver(observer);
    EXPECT_EQ(res, NO_ERROR);
}

#ifdef WITH_DLP
/*
 * Feature: AbilityManagerService
 * Function: GetDlpConnectionInfos
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetDlpConnectionInfos
 * EnvConditions: NA
 * CaseDescription: Verify the function GetDlpConnectionInfos is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetDlpConnectionInfos_001, TestSize.Level1)
{
    std::vector<AbilityRuntime::DlpConnectionInfo> infos;
    auto res = stub_->GetDlpConnectionInfos(infos);
    EXPECT_EQ(res, NO_ERROR);
}
#endif // WITH_DLP

/*
 * Feature: AbilityManagerService
 * Function: GetConnectionData
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetConnectionData
 * EnvConditions: NA
 * CaseDescription: Verify the function GetConnectionData is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetConnectionData_001, TestSize.Level1)
{
    std::vector<AbilityRuntime::ConnectionData> connectionData;
    auto res = stub_->GetConnectionData(connectionData);
    EXPECT_EQ(res, NO_ERROR);
}

#ifdef ABILITY_COMMAND_FOR_TEST
/*
 * Feature: AbilityManagerService
 * Function: ForceTimeoutForTestInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ForceTimeoutForTestInner
 * EnvConditions: NA
 * CaseDescription: Verify the function ForceTimeoutForTestInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_ForceTimeoutForTestInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->ForceTimeoutForTestInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}
#endif

/*
 * Feature: AbilityManagerService
 * Function: FreeInstallAbilityFromRemoteInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService FreeInstallAbilityFromRemoteInner
 * EnvConditions: NA
 * CaseDescription: Verify the function FreeInstallAbilityFromRemoteInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_FreeInstallAbilityFromRemoteInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->FreeInstallAbilityFromRemoteInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: AddFreeInstallObserverInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AddFreeInstallObserverInner
 * EnvConditions: NA
 * CaseDescription: Verify the function AddFreeInstallObserverInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_AddFreeInstallObserverInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->AddFreeInstallObserverInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: DumpAbilityInfoDoneInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DumpAbilityInfoDoneInner
 * EnvConditions: NA
 * CaseDescription: Verify the function DumpAbilityInfoDoneInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_DumpAbilityInfoDoneInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->DumpAbilityInfoDoneInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: UpdateMissionSnapShotFromWMSInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UpdateMissionSnapShotFromWMSInner
 * EnvConditions: NA
 * CaseDescription: Verify the function UpdateMissionSnapShotFromWMSInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_UpdateMissionSnapShotFromWMSInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->UpdateMissionSnapShotFromWMSInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: AbilityManagerService
 * Function: EnableRecoverAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService EnableRecoverAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function EnableRecoverAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_EnableRecoverAbilityInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->EnableRecoverAbilityInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: AbilityManagerService
 * Function: HandleRequestDialogService
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService HandleRequestDialogService
 * EnvConditions: NA
 * CaseDescription: Verify the function HandleRequestDialogService is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_HandleRequestDialogService_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->HandleRequestDialogService(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.number: HandleReportDrawnCompleted_001
 * @tc.name: HandleReportDrawnCompleted
 * @tc.desc: Returns ERR_INVALID_VALUE after passing in parameters
 */
HWTEST_F(AbilityManagerStubTest, HandleReportDrawnCompleted_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->HandleReportDrawnCompleted(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: AcquireShareDataInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AcquireShareDataInner
 * EnvConditions: NA
 * CaseDescription: Verify the function AcquireShareDataInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_AcquireShareDataInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->AcquireShareDataInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: ShareDataDoneInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ShareDataDoneInner
 * EnvConditions: NA
 * CaseDescription: Verify the function ShareDataDoneInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_ShareDataDoneInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->ShareDataDoneInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: AbilityManagerService
 * Function: GetAbilityTokenByCalleeObjInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetAbilityTokenByCalleeObjInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetAbilityTokenByCalleeObjInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetAbilityTokenByCalleeObjInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetAbilityTokenByCalleeObjInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

#ifdef ABILITY_COMMAND_FOR_TEST
/*
 * Feature: AbilityManagerService
 * Function: ScheduleRecoverAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleRecoverAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function ScheduleRecoverAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_ScheduleRecoverAbilityInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->ScheduleRecoverAbilityInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}
#endif

#ifdef ABILITY_COMMAND_FOR_TEST
/*
 * Feature: AbilityManagerService
 * Function: EnableRecoverAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService EnableRecoverAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function EnableRecoverAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_EnableRecoverAbilityInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->EnableRecoverAbilityInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}
#endif

/*
 * Feature: AbilityManagerService
 * Function: RegisterConnectionObserverInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterConnectionObserverInner
 * EnvConditions: NA
 * CaseDescription: Verify the function RegisterConnectionObserverInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_RegisterConnectionObserverInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->RegisterConnectionObserverInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: AbilityManagerService
 * Function: UnregisterConnectionObserverInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UnregisterConnectionObserverInner
 * EnvConditions: NA
 * CaseDescription: Verify the function UnregisterConnectionObserverInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_UnregisterConnectionObserverInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->UnregisterConnectionObserverInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

#ifdef WITH_DLP
/*
 * Feature: AbilityManagerService
 * Function: GetDlpConnectionInfosInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetDlpConnectionInfosInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetDlpConnectionInfosInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetDlpConnectionInfosInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetDlpConnectionInfosInner(data, reply);
    EXPECT_EQ(res, ERR_OK);
}
#endif // WITH_DLP

/*
 * Feature: AbilityManagerService
 * Function: GetConnectionDataInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetConnectionDataInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetConnectionDataInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetConnectionDataInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetConnectionDataInner(data, reply);
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: AbilityManagerService
 * Function: SetMissionContinueStateInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetMissionContinueStateInner
 * EnvConditions: NA
 * CaseDescription: Verify the function SetMissionContinueStateInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_SetMissionContinueStateInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->SetMissionContinueStateInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: AbilityManagerService
 * Function: SetMissionLabelInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetMissionLabelInner
 * EnvConditions: NA
 * CaseDescription: Verify the function SetMissionLabelInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_SetMissionLabelInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->SetMissionLabelInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: AbilityManagerService
 * Function: SetMissionIconInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetMissionIconInner
 * EnvConditions: NA
 * CaseDescription: Verify the function SetMissionIconInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_SetMissionIconInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->SetMissionIconInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterWindowManagerServiceHandlerInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterWindowManagerServiceHandlerInner
 * EnvConditions: NA
 * CaseDescription: Verify the function RegisterWindowManagerServiceHandlerInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_RegisterWindowManagerServiceHandlerInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->RegisterWindowManagerServiceHandlerInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: AbilityManagerService
 * Function: CompleteFirstFrameDrawingInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CompleteFirstFrameDrawingInner
 * EnvConditions: NA
 * CaseDescription: Verify the function CompleteFirstFrameDrawingInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_CompleteFirstFrameDrawingInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->CompleteFirstFrameDrawingInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: AbilityManagerService
 * Function: CompleteFirstFrameDrawingBySCBInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CompleteFirstFrameDrawingBySCBInner
 * EnvConditions: NA
 * CaseDescription: Verify the function CompleteFirstFrameDrawingBySCBInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_CompleteFirstFrameDrawingBySCBInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->CompleteFirstFrameDrawingBySCBInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: GetDialogSessionInfoInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetDialogSessionInfoInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetDialogSessionInfoInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetDialogSessionInfoInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetDialogSessionInfoInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: SendDialogResultInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SendDialogResultInner
 * EnvConditions: NA
 * CaseDescription: Verify the function SendDialogResultInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_SendDialogResultInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->SendDialogResultInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterAbilityFirstFrameStateObserverInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterAbilityFirstFrameStateObserverInner
 * EnvConditions: NA
 * CaseDescription: Verify the function RegisterAbilityFirstFrameStateObserverInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_RegisterAbilityFirstFrameStateObserverInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->RegisterAbilityFirstFrameStateObserverInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: UnregisterAbilityFirstFrameStateObserverInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UnregisterAbilityFirstFrameStateObserverInner
 * EnvConditions: NA
 * CaseDescription: Verify the function UnregisterAbilityFirstFrameStateObserverInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_UnregisterAbilityFirstFrameStateObserverInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->UnregisterAbilityFirstFrameStateObserverInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: CallRequestDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CallRequestDone
 * EnvConditions: NA
 * CaseDescription: Verify the function CallRequestDone is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_CallRequestDone_001, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> callStub = nullptr;
    stub_->CallRequestDone(token, callStub);
    EXPECT_TRUE(stub_ != nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: IsValidMissionIdsInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsValidMissionIdsInner
 * EnvConditions: NA
 * CaseDescription: Verify the function IsValidMissionIdsInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_IsValidMissionIdsInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto isValidMissionIdsTask = [&](const std::vector<int32_t>& ids, std::vector<MissionValidResult>& results) {
        MissionValidResult result;
        result.missionId = 1;
        result.isValid = true;
        results.push_back(result);
        return ERR_OK;
    };
    EXPECT_CALL(*stub_, IsValidMissionIds(_, _)).Times(1).WillOnce(testing::Invoke(isValidMissionIdsTask));
    EXPECT_EQ(stub_->IsValidMissionIdsInner(data, reply), NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: IsValidMissionIdsInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsValidMissionIdsInner
 * EnvConditions: NA
 * CaseDescription: Verify the function IsValidMissionIdsInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_IsValidMissionIdsInner_002, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto isValidMissionIdsTask = [&](const std::vector<int32_t>& ids, std::vector<MissionValidResult>& results) {
        GTEST_LOG_(INFO) << "AbilityManagerStub_IsValidMissionIdsInner_002 caller";
        return ERR_INVALID_VALUE;
    };
    EXPECT_CALL(*stub_, IsValidMissionIds(_, _)).Times(1).WillOnce(testing::Invoke(isValidMissionIdsTask));
    EXPECT_EQ(stub_->IsValidMissionIdsInner(data, reply), NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: ForceExitAppInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ForceExitAppInner
 * EnvConditions: NA
 * CaseDescription: Verify the function ForceExitAppInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_ForceExitAppInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->ForceExitAppInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: RecordAppExitReasonInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RecordAppExitReasonInner
 * EnvConditions: NA
 * CaseDescription: Verify the function RecordAppExitReasonInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_RecordAppExitReasonInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->RecordAppExitReasonInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: RecordProcessExitReasonInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RecordProcessExitReasonInner
 * EnvConditions: NA
 * CaseDescription: Verify the function RecordProcessExitReasonInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_RecordProcessExitReasonInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->RecordProcessExitReasonInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: KillAppWithReasonInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService KillAppWithReasonInner
 * EnvConditions: NA
 * CaseDescription: Verify the function KillAppWithReasonInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_KillAppWithReasonInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->KillAppWithReasonInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
    ExitReasonCompability exitReason;
    data.WriteInt32(1);
    data.WriteParcelable(&exitReason);
    res = stub_->KillAppWithReasonInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: KillBundleWithReasonInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService KillBundleWithReasonInner
 * EnvConditions: NA
 * CaseDescription: Verify the function KillBundleWithReasonInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_KillBundleWithReasonInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->KillBundleWithReasonInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
    ExitReasonCompability exitReason;
    data.WriteString("test");
    data.WriteInt32(1);
    data.WriteInt32(1);
    data.WriteParcelable(&exitReason);
    res = stub_->KillBundleWithReasonInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: RecordAppWithReasonInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RecordAppWithReasonInner
 * EnvConditions: NA
 * CaseDescription: Verify the function RecordAppWithReasonInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_RecordAppWithReasonInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->RecordAppWithReasonInner(data, reply);
    EXPECT_EQ(res, ERR_READ_EXIT_REASON_FAILED);
    ExitReasonCompability exitReason;
    data.WriteInt32(1);
    data.WriteInt32(1);
    data.WriteParcelable(&exitReason);
    res = stub_->RecordAppWithReasonInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: SetRootSceneSessionInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetRootSceneSessionInner
 * EnvConditions: NA
 * CaseDescription: Verify the function SetRootSceneSessionInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_SetRootSceneSessionInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->SetRootSceneSessionInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: CallUIAbilityBySCBInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CallUIAbilityBySCBInner
 * EnvConditions: NA
 * CaseDescription: Verify the function CallUIAbilityBySCBInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_CallUIAbilityBySCBInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->CallUIAbilityBySCBInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: SetSessionManagerServiceInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetSessionManagerServiceInner
 * EnvConditions: NA
 * CaseDescription: Verify the function SetSessionManagerServiceInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_SetSessionManagerServiceInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->SetSessionManagerServiceInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterIAbilityManagerCollaboratorInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterIAbilityManagerCollaboratorInner
 * EnvConditions: NA
 * CaseDescription: Verify the function RegisterIAbilityManagerCollaboratorInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_RegisterIAbilityManagerCollaboratorInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->RegisterIAbilityManagerCollaboratorInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: AbilityManagerService
 * Function: UnregisterIAbilityManagerCollaboratorInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UnregisterIAbilityManagerCollaboratorInner
 * EnvConditions: NA
 * CaseDescription: Verify the function UnregisterIAbilityManagerCollaboratorInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_UnregisterIAbilityManagerCollaboratorInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->UnregisterIAbilityManagerCollaboratorInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: GetAbilityManagerCollaboratorInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetAbilityManagerCollaboratorInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetAbilityManagerCollaboratorInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetAbilityManagerCollaboratorInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetAbilityManagerCollaboratorInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: AbilityManagerService
 * Function: PrepareTerminateAbilityBySCBInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService PrepareTerminateAbilityBySCBInner
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of PrepareTerminateAbilityBySCB.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_PrepareTerminateAbilityBySCBInner_002, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto prepareTerminateAbilityBySCBTask = [&](const sptr<SessionInfo> &sessionInfo, bool &isTerminate) {
        GTEST_LOG_(INFO) << "AbilityManagerStub_IsValidMissionIdsInner_002 caller";
        return ERR_INVALID_VALUE;
    };
    EXPECT_CALL(*stub_, PrepareTerminateAbilityBySCB(_, _)).Times(1).WillOnce(
        testing::Invoke(prepareTerminateAbilityBySCBTask));
    EXPECT_EQ(stub_->PrepareTerminateAbilityBySCBInner(data, reply), ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterStatusBarDelegateInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterStatusBarDelegateInner
 * EnvConditions: NA
 * CaseDescription: Verify the function RegisterStatusBarDelegateInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_RegisterStatusBarDelegateInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->RegisterStatusBarDelegateInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: AbilityManagerService
 * Function: KillProcessWithPrepareTerminateInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService KillProcessWithPrepareTerminateInner
 * EnvConditions: NA
 * CaseDescription: Verify the function KillProcessWithPrepareTerminateInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_KillProcessWithPrepareTerminateInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->KillProcessWithPrepareTerminateInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterSessionHandlerInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterSessionHandlerInner
 * EnvConditions: NA
 * CaseDescription: Verify the function RegisterSessionHandlerInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_RegisterSessionHandlerInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->RegisterSessionHandlerInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartSpecifiedAbilityBySCBInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartSpecifiedAbilityBySCBInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StartSpecifiedAbilityBySCBInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartSpecifiedAbilityBySCBInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StartSpecifiedAbilityBySCBInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartSpecifiedAbilityBySCBInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartSpecifiedAbilityBySCBInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StartSpecifiedAbilityBySCBInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartSpecifiedAbilityBySCBInner_002, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    Want want;
    data.WriteParcelable(&want);
    StartSpecifiedAbilityParams params;
    data.WriteParcelable(&params);
    EXPECT_EQ(stub_->StartSpecifiedAbilityBySCBInner(data, reply), NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: IsAbilityControllerStartInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsAbilityControllerStartInner
 * EnvConditions: NA
 * CaseDescription: Verify the function IsAbilityControllerStartInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_IsAbilityControllerStartInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    Want want;
    data.WriteParcelable(&want);
    EXPECT_EQ(stub_->IsAbilityControllerStartInner(data, reply), NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: ExecuteIntentInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ExecuteIntentInner
 * EnvConditions: NA
 * CaseDescription: Verify the function ExecuteIntentInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_ExecuteIntentInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->ExecuteIntentInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByInsightIntentInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityByInsightIntentInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StartAbilityByInsightIntentInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartAbilityByInsightIntentInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StartAbilityByInsightIntentInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: ExecuteInsightIntentDoneInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ExecuteInsightIntentDoneInner
 * EnvConditions: NA
 * CaseDescription: Verify the function ExecuteInsightIntentDoneInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_ExecuteInsightIntentDoneInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->ExecuteInsightIntentDoneInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: SetApplicationAutoStartupByEDMInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetApplicationAutoStartupByEDMInner
 * EnvConditions: NA
 * CaseDescription: Verify the function SetApplicationAutoStartupByEDMInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_SetApplicationAutoStartupByEDMInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->SetApplicationAutoStartupByEDMInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: CancelApplicationAutoStartupByEDMInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CancelApplicationAutoStartupByEDMInner
 * EnvConditions: NA
 * CaseDescription: Verify the function CancelApplicationAutoStartupByEDMInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_CancelApplicationAutoStartupByEDMInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->CancelApplicationAutoStartupByEDMInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: OpenFileInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OpenFileInner
 * EnvConditions: NA
 * CaseDescription: Verify the function OpenFileInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_OpenFileInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->OpenFileInner(data, reply);
    EXPECT_EQ(res, ERR_DEAD_OBJECT);
}

/*
 * Feature: AbilityManagerService
 * Function: RequestAssertFaultDialogInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RequestAssertFaultDialogInner
 * EnvConditions: NA
 * CaseDescription: Verify the function RequestAssertFaultDialogInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_RequestAssertFaultDialogInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->RequestAssertFaultDialogInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: AbilityManagerService
 * Function: NotifyDebugAssertResultInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService NotifyDebugAssertResultInner
 * EnvConditions: NA
 * CaseDescription: Verify the function NotifyDebugAssertResultInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_NotifyDebugAssertResultInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->NotifyDebugAssertResultInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: UpdateSessionInfoBySCBInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UpdateSessionInfoBySCBInner
 * EnvConditions: NA
 * CaseDescription: Verify the function UpdateSessionInfoBySCBInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_UpdateSessionInfoBySCBInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->UpdateSessionInfoBySCBInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: OpenAtomicServiceInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OpenAtomicServiceInner
 * EnvConditions: NA
 * CaseDescription: Verify the function OpenAtomicServiceInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_OpenAtomicServiceInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->OpenAtomicServiceInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: IsEmbeddedOpenAllowedInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsEmbeddedOpenAllowedInner
 * EnvConditions: NA
 * CaseDescription: Verify the function IsEmbeddedOpenAllowedInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_IsEmbeddedOpenAllowedInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->IsEmbeddedOpenAllowedInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartShortcutInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartShortcutInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StartShortcutInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartShortcutInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StartShortcutInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: GetAbilityStateByPersistentIdInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetAbilityStateByPersistentIdInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetAbilityStateByPersistentIdInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetAbilityStateByPersistentIdInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetAbilityStateByPersistentIdInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: TransferAbilityResultForExtensionInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TransferAbilityResultForExtensionInner
 * EnvConditions: NA
 * CaseDescription: Verify the function TransferAbilityResultForExtensionInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_TransferAbilityResultForExtensionInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->TransferAbilityResultForExtensionInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilityManagerStub_RegisterAppDebugListenerInner_001
 * @tc.desc: Test the status of RegisterAppDebugListenerInner, check empty AppDebugListener.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_RegisterAppDebugListenerInner_001, TestSize.Level1)
{
    EXPECT_NE(stub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->RegisterAppDebugListenerInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilityManagerStub_RegisterAppDebugListenerInner_002
 * @tc.desc: Test the status of RegisterAppDebugListenerInner.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_RegisterAppDebugListenerInner_002, TestSize.Level1)
{
    EXPECT_NE(stub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    auto token = new AppExecFwk::AppDebugListenerStubMock();
    EXPECT_NE(token, nullptr);
    auto ret = data.WriteRemoteObject(token);
    EXPECT_EQ(ret, true);
    int res = stub_->RegisterAppDebugListenerInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerStub_UnregisterAppDebugListenerInner_001
 * @tc.desc: Test the status of UnregisterAppDebugListenerInner, check empty appDebugListener.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_UnregisterAppDebugListenerInner_001, TestSize.Level1)
{
    EXPECT_NE(stub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->UnregisterAppDebugListenerInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilityManagerStub_UnregisterAppDebugListenerInner_002
 * @tc.desc: Test the status of UnregisterAppDebugListenerInner.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_UnregisterAppDebugListenerInner_002, TestSize.Level1)
{
    EXPECT_NE(stub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    auto token = new AppExecFwk::AppDebugListenerStubMock();
    EXPECT_NE(token, nullptr);
    bool ret = data.WriteRemoteObject(token);
    EXPECT_EQ(ret, true);
    auto res = stub_->UnregisterAppDebugListenerInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerStub_AttachAppDebugInner_001
 * @tc.desc: Test the state of AttachAppDebugInner.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_AttachAppDebugInner_001, TestSize.Level1)
{
    EXPECT_NE(stub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    std::string bundleName = "bundleName";
    data.WriteString(bundleName);
    auto res = stub_->AttachAppDebugInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerStub_AttachAppDebugInner_002
 * @tc.desc: Test the state of AttachAppDebugInner, check empty bundleName;
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_AttachAppDebugInner_002, TestSize.Level1)
{
    EXPECT_NE(stub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->AttachAppDebugInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilityManagerStub_DetachAppDebugInner_001
 * @tc.desc: Test the state of DetachAppDebugInner.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_DetachAppDebugInner_001, TestSize.Level1)
{
    EXPECT_NE(stub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    std::string bundleName = "bundleName";
    data.WriteString(bundleName);
    auto res = stub_->DetachAppDebugInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerStub_DetachAppDebugInner_002
 * @tc.desc: Test the state of DetachAppDebugInner, check empty bundleName.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_DetachAppDebugInner_002, TestSize.Level1)
{
    EXPECT_NE(stub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->DetachAppDebugInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilityManagerStub_GetForegroundUIAbilitiesInner_001
 * @tc.desc: Test function GetForegroundUIAbilitiesInner when normally.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetForegroundUIAbilitiesInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetForegroundUIAbilitiesInner(data, reply);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: AbilityManagerStubTest_RegisterAutoStartupSystemCallbackInner_0100
 * @tc.desc: Test the state of RegisterAutoStartupSystemCallbackInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, RegisterAutoStartupSystemCallbackInner_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    sptr<AppExecFwk::MockAbilityToken> token = new (std::nothrow) AppExecFwk::MockAbilityToken();
    EXPECT_NE(token, nullptr);
    data.WriteRemoteObject(token);
    auto result = stub_->RegisterAutoStartupSystemCallbackInner(data, reply);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerStubTest_RegisterAutoStartupSystemCallbackInner_0200
 * @tc.desc: Test the state of RegisterAutoStartupSystemCallbackInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, RegisterAutoStartupSystemCallbackInner_0200, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto result = stub_->RegisterAutoStartupSystemCallbackInner(data, reply);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilityManagerStubTest_UnregisterAutoStartupSystemCallbackInner_0100
 * @tc.desc: Test the state of UnregisterAutoStartupSystemCallbackInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, UnregisterAutoStartupSystemCallbackInner_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    sptr<AppExecFwk::MockAbilityToken> token = new (std::nothrow) AppExecFwk::MockAbilityToken();
    EXPECT_NE(token, nullptr);
    data.WriteRemoteObject(token);
    auto result = stub_->UnregisterAutoStartupSystemCallbackInner(data, reply);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerStubTest_UnregisterAutoStartupSystemCallbackInner_0200
 * @tc.desc: Test the state of UnregisterAutoStartupSystemCallbackInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, UnregisterAutoStartupSystemCallbackInner_0200, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto result = stub_->UnregisterAutoStartupSystemCallbackInner(data, reply);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}
/**
 * @tc.name: AbilityManagerStubTest_SetApplicationAutoStartupInner_0100
 * @tc.desc: Test the state of SetApplicationAutoStartupInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, SetApplicationAutoStartupInner_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    Want want;
    data.WriteParcelable(&want);
    auto result = stub_->SetApplicationAutoStartupInner(data, reply);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerStubTest_SetApplicationAutoStartupInner_0200
 * @tc.desc: Test the state of SetApplicationAutoStartupInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, SetApplicationAutoStartupInner_0200, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto result = stub_->SetApplicationAutoStartupInner(data, reply);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilityManagerStubTest_CancelApplicationAutoStartupInner_0100
 * @tc.desc: Test the state of CancelApplicationAutoStartupInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, CancelApplicationAutoStartupInner_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    Want want;
    data.WriteParcelable(&want);
    auto result = stub_->CancelApplicationAutoStartupInner(data, reply);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerStubTest_CancelApplicationAutoStartupInner_0200
 * @tc.desc: Test the state of CancelApplicationAutoStartupInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, CancelApplicationAutoStartupInner_0200, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto result = stub_->CancelApplicationAutoStartupInner(data, reply);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilityManagerStubTest_QueryAllAutoStartupApplicationsInner_0100
 * @tc.desc: Test the state of QueryAllAutoStartupApplicationsInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, QueryAllAutoStartupApplicationsInner_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    Want want;
    data.WriteParcelable(&want);
    auto result = stub_->QueryAllAutoStartupApplicationsInner(data, reply);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerStubTest_GetAutoStartupStatusForSelfInner_0100
 * @tc.desc: Test the state of GetAutoStartupStatusForSelfInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, GetAutoStartupStatusForSelfInner_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto result = stub_->GetAutoStartupStatusForSelfInner(data, reply);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerStubTest_GetAutoStartupStatusForSelfInner_0200
 * @tc.desc: Test the state of GetAutoStartupStatusForSelfInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, GetAutoStartupStatusForSelfInner_0200, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*stub_, GetAutoStartupStatusForSelf(_)).WillOnce(Return(ERR_INVALID_VALUE));
    auto result = stub_->GetAutoStartupStatusForSelfInner(data, reply);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: GetUIExtensionRootHostInfo_0100
 * @tc.desc: GetUIExtensionRootHostInfo
 * @tc.type: FUNC
 * @tc.require: issueI92G6Z
 */
HWTEST_F(AbilityManagerStubTest, GetUIExtensionRootHostInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");

    MessageParcel data;
    bool writeRet = data.WriteInterfaceToken(AbilityManagerStubImplMock::GetDescriptor());
    auto token = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    auto userId = USER_ID;
    writeRet &= data.WriteBool(true);
    writeRet &= data.WriteRemoteObject(token);
    writeRet &= data.WriteInt32(userId);
    EXPECT_EQ(writeRet, true);

    EXPECT_CALL(*stub_, GetUIExtensionRootHostInfo(_, _, _)).Times(1);

    MessageParcel reply;
    MessageOption option;
    auto ret = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_UI_EXTENSION_ROOT_HOST_INFO), data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: RestartAppInner_0100
 * @tc.desc: RestartAppInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, RestartAppInner_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    Want want;
    data.WriteParcelable(&want);
    auto result = stub_->RestartAppInner(data, reply);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: ChangeAbilityVisibility_0100
 * @tc.desc: ChangeAbilityVisibility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, ChangeAbilityVisibility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");

    MessageParcel data;
    MessageParcel reply;
    auto token = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    data.WriteRemoteObject(token);
    data.WriteBool(true);

    auto ret = stub_->ChangeAbilityVisibilityInner(data, reply);
    EXPECT_EQ(ret, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: ChangeUIAbilityVisibilityBySCB_0100
 * @tc.desc: ChangeUIAbilityVisibilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, ChangeUIAbilityVisibilityBySCB_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");

    MessageParcel data;
    MessageParcel reply;
    sptr<SessionInfo> session = new (std::nothrow) SessionInfo();
    data.WriteParcelable(session);
    data.WriteBool(true);

    auto ret = stub_->ChangeUIAbilityVisibilityBySCBInner(data, reply);
    EXPECT_EQ(ret, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIAbilitiesInSplitWindowModeInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilitiesInSplitWindowModeInner
 * EnvConditions: NA
 * CaseDescription: want is nullptr
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartUIAbilitiesInSplitWindowModeInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    int32_t primaryWindowId = 1;
    data.WriteInt32(primaryWindowId);

    data.WriteParcelable(nullptr);

    sptr<IRemoteObject> token =
        sptr<AppExecFwk::MockAbilityToken>(new (std::nothrow) AppExecFwk::MockAbilityToken());
    data.WriteRemoteObject(token);

    int res = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_ABILITIES_IN_SPLIT_WINDOW_MODE), data, reply,
        option);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIAbilitiesInSplitWindowModeInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilitiesInSplitWindowModeInner
 * EnvConditions: NA
 * CaseDescription: callToken is nullptr
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartUIAbilitiesInSplitWindowModeInner_002, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    int32_t primaryWindowId = 1;
    data.WriteInt32(primaryWindowId);

    Want want;
    data.WriteParcelable(&want);

    sptr<IRemoteObject> token = nullptr;
    data.WriteRemoteObject(token);

    int res = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_ABILITIES_IN_SPLIT_WINDOW_MODE), data, reply,
        option);
    EXPECT_EQ(res, INVALID_CALLER_TOKEN);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIAbilitiesInSplitWindowModeInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilitiesInSplitWindowModeInner
 * EnvConditions: NA
 * CaseDescription: NO_ERROR
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartUIAbilitiesInSplitWindowModeInner_003, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    int32_t primaryWindowId = 1;
    data.WriteInt32(primaryWindowId);

    Want want;
    data.WriteParcelable(&want);

    sptr<IRemoteObject> token =
        sptr<AppExecFwk::MockAbilityToken>(new (std::nothrow) AppExecFwk::MockAbilityToken());
    data.WriteRemoteObject(token);

    int res = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_ABILITIES_IN_SPLIT_WINDOW_MODE), data, reply,
        option);
    EXPECT_EQ(res, NO_ERROR);
}
/*
 * Feature: AbilityManagerService
 * Function: StartUIAbilitiesInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilitiesInner
 * EnvConditions: NA
 * CaseDescription: nothing in data
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartUIAbilitiesInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_ABILITIES),
        data, reply, option);
    EXPECT_EQ(res, START_UI_ABILITIES_WANT_LIST_SIZE_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIAbilitiesInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilitiesInner
 * EnvConditions: NA
 * CaseDescription: size in data but invalid
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartUIAbilitiesInner_002, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    int32_t size = 0;
    data.WriteInt32(size);
    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_ABILITIES),
        data, reply, option);
    EXPECT_EQ(res, START_UI_ABILITIES_WANT_LIST_SIZE_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIAbilitiesInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilitiesInner
 * EnvConditions: NA
 * CaseDescription: size in data but invalid
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartUIAbilitiesInner_003, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    int32_t size = 5;
    data.WriteInt32(size);
    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_ABILITIES),
        data, reply, option);
    EXPECT_EQ(res, START_UI_ABILITIES_WANT_LIST_SIZE_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIAbilitiesInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilitiesInner
 * EnvConditions: NA
 * CaseDescription: callerToken nullptr
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartUIAbilitiesInner_004, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    int32_t size = 1;
    data.WriteInt32(size);

    Want want;
    data.WriteParcelable(&want);

    std::string requestKey = "12345";
    data.WriteString(requestKey);

    sptr<IRemoteObject> token = nullptr;
    data.WriteRemoteObject(token);

    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_ABILITIES),
        data, reply, option);
    EXPECT_EQ(res, INVALID_CALLER_TOKEN);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIAbilitiesInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilitiesInner
 * EnvConditions: NA
 * CaseDescription: callerToken nullptr
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartUIAbilitiesInner_005, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    int32_t size = 1;
    data.WriteInt32(size);

    Want want;
    data.WriteParcelable(&want);

    std::string requestKey = "12345";
    data.WriteString(requestKey);

    sptr<IRemoteObject> token =
        sptr<AppExecFwk::MockAbilityToken>(new (std::nothrow) AppExecFwk::MockAbilityToken());
    data.WriteRemoteObject(token);

    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_ABILITIES),
        data, reply, option);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: TerminateMissionInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateMissionInner
 * EnvConditions: NA
 * CaseDescription: Verify the function TerminateMissionInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_TerminateMissionInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->TerminateMissionInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: GetAllInsightIntentInfo_0100
 * @tc.desc: GetAllInsightIntentInfo
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, GetAllInsightIntentInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");

    MessageParcel data;
    MessageParcel reply;
    auto flag = AbilityRuntime::GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT;
    data.WriteUint32(static_cast<uint32_t>(flag));
    auto ret = stub_->GetAllInsightIntentInfoInner(data, reply);
    EXPECT_EQ(ret, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: GetInsightIntentInfoByBundleName_0100
 * @tc.desc: GetInsightIntentInfoByBundleName
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, GetInsightIntentInfoByBundleName_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");

    MessageParcel data;
    MessageParcel reply;
    auto flag = AbilityRuntime::GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT;
    std::string bundleName = "com.example.bundleName";
    data.WriteUint32(static_cast<uint32_t>(flag));
    data.WriteString(bundleName);
    auto ret = stub_->GetInsightIntentInfoByBundleNameInner(data, reply);
    EXPECT_EQ(ret, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: GetInsightIntentInfoByIntentName_0100
 * @tc.desc: GetInsightIntentInfoByIntentName
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, GetInsightIntentInfoByIntentName_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");

    MessageParcel data;
    MessageParcel reply;
    auto flag = AbilityRuntime::GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT;
    std::string bundleName = "com.example.bundleName";
    std::string moduleName = "entry";
    std::string intentName = "test";
    data.WriteUint32(static_cast<uint32_t>(flag));
    data.WriteString(bundleName);
    data.WriteString(moduleName);
    data.WriteString(intentName);
    auto ret = stub_->GetInsightIntentInfoByIntentNameInner(data, reply);
    EXPECT_EQ(ret, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: UpdateKioskApplicationListInner
 * @tc.desc: UpdateKioskApplicationListInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, UpdateKioskApplicationListInner, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    MessageParcel data;
    bool writeRet = data.WriteInterfaceToken(AbilityManagerStubImplMock::GetDescriptor());
    std::vector<std::string> appList = {"com.ohos.test1"};
    writeRet &= data.WriteStringVector(appList);
    EXPECT_EQ(writeRet, true);

    MessageParcel reply;
    MessageOption option;
    auto ret = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::UPDATE_KIOSK_APP_LIST), data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: EnterKioskModeInner
 * @tc.desc: EnterKioskModeInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, EnterKioskModeInner, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    MessageParcel data;
    bool writeRet = data.WriteInterfaceToken(AbilityManagerStubImplMock::GetDescriptor());
    auto token = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    writeRet &= data.WriteRemoteObject(token);
    EXPECT_EQ(writeRet, true);

    MessageParcel reply;
    MessageOption option;
    auto ret = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::ENTER_KIOSK_MODE), data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: ExitKioskModeInner
 * @tc.desc: ExitKioskModeInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, ExitKioskModeInner, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    MessageParcel data;
    bool writeRet = data.WriteInterfaceToken(AbilityManagerStubImplMock::GetDescriptor());
    auto token = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    writeRet &= data.WriteRemoteObject(token);
    EXPECT_EQ(writeRet, true);

    MessageParcel reply;
    MessageOption option;
    auto ret = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::EXIT_KIOSK_MODE), data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: GetKioskStatusInner
 * @tc.desc: GetKioskStatusInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, GetKioskStatusInner, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    MessageParcel data;
    bool writeRet = data.WriteInterfaceToken(AbilityManagerStubImplMock::GetDescriptor());
    EXPECT_EQ(writeRet, true);

    MessageParcel reply;
    MessageOption option;
    auto ret = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_KIOSK_INFO), data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: RegisterSAInterceptorInner_0100
 * @tc.desc: RegisterSAInterceptorInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, RegisterSAInterceptorInner_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    MessageParcel data;
    MessageParcel reply;
    auto ret = stub_->RegisterSAInterceptorInner(data, reply);
    EXPECT_EQ(ret, ERR_NULL_SA_INTERCEPTOR_EXECUTER);
    sptr<AbilityRuntime::ISAInterceptor> interceptor = new AbilityRuntime::MockSAInterceptorStub(0);
    EXPECT_EQ(data.WriteRemoteObject(interceptor->AsObject()), true);
    ret = stub_->RegisterSAInterceptorInner(data, reply);
    EXPECT_EQ(ret, NO_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: SetAppServiceExtensionKeepAlive_0100
 * @tc.desc: SetAppServiceExtensionKeepAlive
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, SetAppServiceExtensionKeepAlive_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");

    MessageParcel data;
    MessageParcel reply;
    std::string bundleName = "bundleName";
    bool flag = true;
    data.WriteString(bundleName);
    data.WriteUint32(static_cast<uint32_t>(flag));
    auto ret = stub_->SetAppServiceExtensionKeepAliveInner(data, reply);
    EXPECT_EQ(ret, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: QueryKeepAliveAppServiceExtensions_0100
 * @tc.desc: QueryKeepAliveAppServiceExtensions
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, QueryKeepAliveAppServiceExtensions_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");

    MessageParcel data;
    MessageParcel reply;
    auto ret = stub_->QueryKeepAliveAppServiceExtensionsInner(data, reply);
    EXPECT_EQ(ret, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "end");
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerStub StartSelfUIAbilityInCurrentProcessInner
 * EnvConditions: NA
 * CaseDescription: Verify the normal and invalid behavior of StartSelfUIAbilityInCurrentProcessInner
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartSelfUIAbilityInCurrentProcessInner_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    ASSERT_TRUE(data.WriteInterfaceToken(AbilityManagerStub::GetDescriptor()));
    Want want;
    ASSERT_TRUE(data.WriteParcelable(&want));
    std::string specifiedFlag = "testFlag";
    ASSERT_TRUE(data.WriteString(specifiedFlag));
    auto startOptions = new (std::nothrow) StartOptions();
    ASSERT_NE(startOptions, nullptr);
    ASSERT_TRUE(data.WriteParcelable(startOptions));
    ASSERT_TRUE(data.WriteBool(true));
    ASSERT_TRUE(data.WriteBool(false));
    auto result = stub_->StartSelfUIAbilityInCurrentProcessInner(data, reply);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: UnPreloadUIExtensionAbilityInner_0100
 * @tc.desc: UnPreloadUIExtensionAbilityInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, UnPreloadUIExtensionAbilityInner_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");

    MessageParcel data;
    MessageParcel reply;
    ASSERT_TRUE(data.WriteInterfaceToken(AbilityManagerStub::GetDescriptor()));
    
    int32_t extensionAbilityId = 12345;
    ASSERT_TRUE(data.WriteInt32(extensionAbilityId));
    
    int32_t userId = DEFAULT_INVAL_VALUE;
    ASSERT_TRUE(data.WriteInt32(userId));
    
    auto result = stub_->UnPreloadUIExtensionAbilityInner(data, reply);
    EXPECT_EQ(result, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: RegisterPreloadUIExtensionHostClientInner_0100
 * @tc.desc: RegisterPreloadUIExtensionHostClientInner with invalid token
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, RegisterPreloadUIExtensionHostClientInner_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");

    MessageParcel data;
    MessageParcel reply;
    ASSERT_TRUE(data.WriteInterfaceToken(AbilityManagerStub::GetDescriptor()));

    std::string hostBundleName = "com.ohos.uiextensionuser";
    ASSERT_TRUE(data.WriteString(hostBundleName));
    
    int32_t hostPid = 12345;
    ASSERT_TRUE(data.WriteInt32(hostPid));
    
    auto result = stub_->RegisterPreloadUIExtensionHostClientInner(data, reply);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: UnRegisterPreloadUIExtensionHostClientInner_0100
 * @tc.desc: UnRegisterPreloadUIExtensionHostClientInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, UnRegisterPreloadUIExtensionHostClientInner_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");

    MessageParcel data;
    MessageParcel reply;
    ASSERT_TRUE(data.WriteInterfaceToken(AbilityManagerStub::GetDescriptor()));
    
    std::string hostBundleName = "com.ohos.uiextensionuser";
    ASSERT_TRUE(data.WriteString(hostBundleName));
    
    int32_t hostPid = 12345;
    ASSERT_TRUE(data.WriteInt32(hostPid));
    
    auto result = stub_->UnRegisterPreloadUIExtensionHostClientInner(data, reply);
    EXPECT_EQ(result, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: ClearAllPreloadUIExtensionAbilityInner_0100
 * @tc.desc: ClearAllPreloadUIExtensionAbilityInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, ClearAllPreloadUIExtensionAbilityInner_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");

    MessageParcel data;
    MessageParcel reply;
    ASSERT_TRUE(data.WriteInterfaceToken(AbilityManagerStub::GetDescriptor()));
    int32_t userId = 100;
    ASSERT_TRUE(data.WriteInt32(userId));
    auto result = stub_->ClearAllPreloadUIExtensionAbilityInner(data, reply);

    EXPECT_EQ(result, NO_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: QueryCallerTokenIdForAncoInner_0100
 * @tc.desc: QueryCallerTokenIdForAncoInner with valid dialogSessionId
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, QueryCallerTokenIdForAncoInner_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "QueryCallerTokenIdForAncoInner_0100 begin");

    MessageParcel data;
    MessageParcel reply;
    int32_t userId = 101;
    std::string asCallerForAncoSessionId = "520";
    ASSERT_TRUE(data.WriteInt32(userId));
    ASSERT_TRUE(data.WriteString(asCallerForAncoSessionId));
    auto result = stub_->QueryCallerTokenIdForAncoInner(data, reply);
    EXPECT_EQ(result, NO_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "QueryCallerTokenIdForAncoInner_0100 end");
}

/**
 * @tc.name: QueryCallerTokenIdForAncoInner_0200
 * @tc.desc: QueryCallerTokenIdForAncoInner with invalid dialogSessionId
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, QueryCallerTokenIdForAncoInner_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "QueryCallerTokenIdForAncoInner_0200 begin");

    MessageParcel data;
    MessageParcel reply;
    int32_t userId = -1;
    std::string asCallerForAncoSessionId = "-1";
    ASSERT_TRUE(data.WriteInt32(userId));
    ASSERT_TRUE(data.WriteString(asCallerForAncoSessionId));
    auto result = stub_->QueryCallerTokenIdForAncoInner(data, reply);
    EXPECT_EQ(result, NO_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "QueryCallerTokenIdForAncoInner_0200 end");
}

/**
 * @tc.name: QueryCallerTokenIdForAncoInner_0300
 * @tc.desc: QueryCallerTokenIdForAncoInner with OnRemoteRequest
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, QueryCallerTokenIdForAncoInner_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "QueryCallerTokenIdForAncoInner_0300 begin");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    ASSERT_TRUE(data.WriteInterfaceToken(AbilityManagerStub::GetDescriptor()));
    int32_t userId = 100;
    std::string asCallerForAncoSessionId = "100";
    ASSERT_TRUE(data.WriteInt32(userId));
    ASSERT_TRUE(data.WriteString(asCallerForAncoSessionId));

    auto result = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::QUERY_CALLER_TOKEN_ID_FOR_ANCO), data, reply, option);

    EXPECT_EQ(result, NO_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "QueryCallerTokenIdForAncoInner_0300 end");
}

/**
 * @tc.name: ManualStartAutoStartupAppsInner_0100
 * @tc.desc: ManualStartAutoStartupAppsInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, ManualStartAutoStartupAppsInner_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ManualStartAutoStartupAppsInner_0100 begin");

    MessageParcel data;
    MessageParcel reply;
    ASSERT_TRUE(data.WriteInterfaceToken(AbilityManagerStub::GetDescriptor()));
    int32_t userId = 100;
    ASSERT_TRUE(data.WriteInt32(userId));
    auto result = stub_->ManualStartAutoStartupAppsInner(data, reply);

    EXPECT_EQ(result, NO_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "ManualStartAutoStartupAppsInner_0100 end");
}

/**
 * @tc.name: ManualStartAutoStartupAppsInner_0200
 * @tc.desc: ManualStartAutoStartupAppsInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, ManualStartAutoStartupAppsInner_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ManualStartAutoStartupAppsInner_0200 begin");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    ASSERT_TRUE(data.WriteInterfaceToken(AbilityManagerStub::GetDescriptor()));
    int32_t userId = 100;
    ASSERT_TRUE(data.WriteInt32(userId));

    auto result = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::MANUAL_START_AUTO_STARTUP_APPS), data, reply, option);

    EXPECT_EQ(result, NO_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "ManualStartAutoStartupAppsInner_0200 end");
}

/**
 * @tc.name: GetUserLockedBundleListInner_0100
 * @tc.desc: Test normal call of GetUserLockedBundleListInner with empty bundle list
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(AbilityManagerStubTest, GetUserLockedBundleListInner_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetUserLockedBundleListInner_0100 begin");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ASSERT_TRUE(data.WriteInterfaceToken(AbilityManagerStub::GetDescriptor()));

    int32_t testUserId = USER_ID;
    ASSERT_TRUE(data.WriteInt32(testUserId));

    auto result = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_USER_LOCKED_BUNDLE_LIST), data, reply, option);

    int32_t businessRet = reply.ReadInt32();
    EXPECT_EQ(businessRet, ERR_OK);

    int32_t listSize = reply.ReadInt32();
    EXPECT_NE(listSize, 0);

    TAG_LOGI(AAFwkTag::TEST, "GetUserLockedBundleListInner_0100 end");
}
} // namespace AAFwk
} // namespace OHOS
