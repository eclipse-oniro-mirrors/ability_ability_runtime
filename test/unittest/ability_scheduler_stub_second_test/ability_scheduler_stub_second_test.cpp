/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "ability_schedule_stub_second_mock.h"
#include "data_ability_predicates.h"

using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
constexpr int CYCLE_LIMIT = 2000;
class AbilitySchedulerStubSecondTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void WriteInterfaceToken(MessageParcel& data);
    sptr<AbilitySchedulerStubMock> stub_{ nullptr };
};

void AbilitySchedulerStubSecondTest::SetUpTestCase(void)
{}
void AbilitySchedulerStubSecondTest::TearDownTestCase(void)
{}
void AbilitySchedulerStubSecondTest::TearDown(void)
{}

void AbilitySchedulerStubSecondTest::SetUp(void)
{
    stub_ = new AbilitySchedulerStubMock();
}
void AbilitySchedulerStubSecondTest::WriteInterfaceToken(MessageParcel& data)
{
    data.WriteInterfaceToken(AbilitySchedulerStub::GetDescriptor());
}

/**
 * @tc.name: AbilitySchedulerStubSecond_001
 * @tc.desc: test BatchInsertInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSecondTest, AbilitySchedulerStubSecond_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    auto res = stub_->BatchInsertInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    auto uri = std::make_shared<Uri>("test_uri");
    data.WriteParcelable(uri.get());
    res = stub_->BatchInsertInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    MessageParcel data2;
    data2.WriteParcelable(uri.get());
    int largeCount = CYCLE_LIMIT + 1;
    data2.WriteInt32(largeCount);
    res = stub_->BatchInsertInner(data2, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    MessageParcel data3;
    Want want;
    LifeCycleStateInfo stateInfo;
    data3.WriteParcelable(&want);
    data3.WriteParcelable(&stateInfo);
    res = stub_->BatchInsertInner(data3, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilitySchedulerStubSecond_002
 * @tc.desc: test RegisterObserverInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSecondTest, AbilitySchedulerStubSecond_002, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    auto res = stub_->RegisterObserverInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    auto uri = std::make_shared<Uri>("test_uri");
    data.WriteParcelable(uri.get());
    res = stub_->RegisterObserverInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    MessageParcel data2;
    data2.WriteParcelable(uri.get());
    sptr<IDataAbilityObserver> dataObserver(new (std::nothrow) MockDataAbilityObserverStub());
    data2.WriteRemoteObject(dataObserver->AsObject());
    res = stub_->RegisterObserverInner(data2, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilitySchedulerStubSecond_003
 * @tc.desc: test UnregisterObserverInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSecondTest, AbilitySchedulerStubSecond_003, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    auto res = stub_->UnregisterObserverInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    auto uri = std::make_shared<Uri>("test_uri");
    data.WriteParcelable(uri.get());
    res = stub_->UnregisterObserverInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    MessageParcel data2;
    data2.WriteParcelable(uri.get());
    sptr<IDataAbilityObserver> dataObserver(new (std::nothrow) MockDataAbilityObserverStub());
    data2.WriteRemoteObject(dataObserver->AsObject());
    res = stub_->UnregisterObserverInner(data2, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilitySchedulerStubSecond_004
 * @tc.desc: test NotifyChangeInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSecondTest, AbilitySchedulerStubSecond_004, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    auto res = stub_->NotifyChangeInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    auto uri = std::make_shared<Uri>("test_uri");
    data.WriteParcelable(uri.get());
    res = stub_->NotifyChangeInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilitySchedulerStubSecond_005
 * @tc.desc: test NormalizeUriInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSecondTest, AbilitySchedulerStubSecond_005, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    auto res = stub_->NormalizeUriInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    auto uri = std::make_shared<Uri>("test_uri");
    data.WriteParcelable(uri.get());
    res = stub_->NormalizeUriInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilitySchedulerStubSecond_006
 * @tc.desc: test DenormalizeUriInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSecondTest, AbilitySchedulerStubSecond_006, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    auto res = stub_->DenormalizeUriInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    auto uri = std::make_shared<Uri>("test_uri");
    data.WriteParcelable(uri.get());
    res = stub_->DenormalizeUriInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilitySchedulerStubSecond_007
 * @tc.desc: test ExecuteBatchInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSecondTest, AbilitySchedulerStubSecond_007, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    auto res = stub_->ExecuteBatchInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    int largeCount = CYCLE_LIMIT + 1;
    data.WriteInt32(largeCount);
    res = stub_->ExecuteBatchInner(data, reply);

    MessageParcel data2;
    int count = 10;
    data2.WriteInt32(count);
    res = stub_->ExecuteBatchInner(data2, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilitySchedulerStubSecond_008
 * @tc.desc: test DumpAbilityInfoInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSecondTest, AbilitySchedulerStubSecond_008, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteInt32(200);
    auto res = stub_->DumpAbilityInfoInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilitySchedulerStubSecond_009
 * @tc.desc: test NotifyContinuationResultInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSecondTest, AbilitySchedulerStubSecond_009, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteInt32(200);
    auto res = stub_->NotifyContinuationResultInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilitySchedulerStubSecond_010
 * @tc.desc: test OnExecuteIntentInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSecondTest, AbilitySchedulerStubSecond_010, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    auto res = stub_->OnExecuteIntentInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    Want want;
    data.WriteParcelable(&want);
    res = stub_->OnExecuteIntentInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilitySchedulerStubSecond_011
 * @tc.desc: test ContinueAbilityInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSecondTest, AbilitySchedulerStubSecond_011, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    std::string expectedDeviceId = "test_device_id";
    uint32_t expectedVersionCode = 12345;
    data.WriteString(expectedDeviceId);
    data.WriteUint32(expectedVersionCode);
    auto res = stub_->ContinueAbilityInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilitySchedulerStubSecond_012
 * @tc.desc: test ShareDataInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSecondTest, AbilitySchedulerStubSecond_012, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    int32_t requestCode = 200;
    data.WriteInt32(requestCode);
    auto res = stub_->ShareDataInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilitySchedulerStubSecond_013
 * @tc.desc: test CreateModalUIExtensionInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSecondTest, AbilitySchedulerStubSecond_013, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    auto res = stub_->CreateModalUIExtensionInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    Want want;
    data.WriteParcelable(&want);
    res = stub_->CreateModalUIExtensionInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilitySchedulerStubSecond_014
 * @tc.desc: test UpdateSessionTokenInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSecondTest, AbilitySchedulerStubSecond_014, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    sptr<IRemoteObject> token;
    data.WriteRemoteObject(token);
    auto res = stub_->UpdateSessionTokenInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilitySchedulerStubSecond_015
 * @tc.desc: test CollaborateDataInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSecondTest, AbilitySchedulerStubSecond_015, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    auto res = stub_->CollaborateDataInner(data);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    Want want;
    data.WriteParcelable(&want);
    res = stub_->CollaborateDataInner(data);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilitySchedulerStubSecond_016
 * @tc.desc: test OnRemoteRequestInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSecondTest, AbilitySchedulerStubSecond_016, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    auto res = stub_->OnRemoteRequestInner(IAbilityScheduler::SCHEDULE_SHARE_DATA, data, reply, option);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilitySchedulerStubSecond_017
 * @tc.desc: test OnRemoteRequestInnerFirst
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSecondTest, AbilitySchedulerStubSecond_017, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    int res = stub_->OnRemoteRequestInnerFirst(IAbilityScheduler::SCHEDULE_ABILITY_COMMAND, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    res = stub_->OnRemoteRequestInnerFirst(IAbilityScheduler::SCHEDULE_ABILITY_PREPARE_TERMINATE, data, reply, option);
    EXPECT_EQ(res, NO_ERROR);

    res = stub_->OnRemoteRequestInnerFirst(IAbilityScheduler::SCHEDULE_ABILITY_COMMAND_WINDOW, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    res = stub_->OnRemoteRequestInnerFirst(IAbilityScheduler::SCHEDULE_RESTORE_ABILITY_STATE, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    res = stub_->OnRemoteRequestInnerFirst(IAbilityScheduler::SCHEDULE_GETFILETYPES, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    res = stub_->OnRemoteRequestInnerFirst(IAbilityScheduler::SCHEDULE_OPENFILE, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    res = stub_->OnRemoteRequestInnerFirst(IAbilityScheduler::SCHEDULE_OPENRAWFILE, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    res = stub_->OnRemoteRequestInnerFirst(IAbilityScheduler::SCHEDULE_INSERT, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    res = stub_->OnRemoteRequestInnerFirst(IAbilityScheduler::SCHEDULE_UPDATE, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    res = stub_->OnRemoteRequestInnerFirst(IAbilityScheduler::SCHEDULE_DELETE, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilitySchedulerStubSecond_018
 * @tc.desc: test OnRemoteRequestInnerSecond
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSecondTest, AbilitySchedulerStubSecond_018, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    int res = stub_->OnRemoteRequestInnerSecond(IAbilityScheduler::SCHEDULE_QUERY, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    res = stub_->OnRemoteRequestInnerSecond(IAbilityScheduler::SCHEDULE_CALL, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    res = stub_->OnRemoteRequestInnerSecond(IAbilityScheduler::SCHEDULE_GETTYPE, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    res = stub_->OnRemoteRequestInnerSecond(IAbilityScheduler::SCHEDULE_RELOAD, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    res = stub_->OnRemoteRequestInnerSecond(IAbilityScheduler::SCHEDULE_BATCHINSERT, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    res = stub_->OnRemoteRequestInnerSecond(IAbilityScheduler::SCHEDULE_REGISTEROBSERVER, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    res = stub_->OnRemoteRequestInnerSecond(IAbilityScheduler::SCHEDULE_UNREGISTEROBSERVER, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    res = stub_->OnRemoteRequestInnerSecond(IAbilityScheduler::SCHEDULE_NOTIFYCHANGE, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    res = stub_->OnRemoteRequestInnerSecond(IAbilityScheduler::SCHEDULE_NORMALIZEURI, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    res = stub_->OnRemoteRequestInnerSecond(IAbilityScheduler::SCHEDULE_DENORMALIZEURI, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    res = stub_->OnRemoteRequestInnerSecond(IAbilityScheduler::SCHEDULE_EXECUTEBATCH, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    res = stub_->OnRemoteRequestInnerSecond(IAbilityScheduler::NOTIFY_CONTINUATION_RESULT, data, reply, option);
    EXPECT_EQ(res, NO_ERROR);

    res = stub_->OnRemoteRequestInnerSecond(IAbilityScheduler::CONTINUE_ABILITY, data, reply, option);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilitySchedulerStubSecond_019
 * @tc.desc: test OnRemoteRequestInnerThird
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSecondTest, AbilitySchedulerStubSecond_019, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    uint32_t code = IAbilityScheduler::SCHEDULE_SHARE_DATA;
    int res = stub_->OnRemoteRequestInnerThird(code, data, reply, option);
    EXPECT_EQ(res, NO_ERROR);

    code = 31;
    res = stub_->OnRemoteRequestInnerThird(code, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    code = IAbilityScheduler::CREATE_MODAL_UI_EXTENSION;
    res = stub_->OnRemoteRequestInnerThird(code, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    code = IAbilityScheduler::UPDATE_SESSION_TOKEN;
    res = stub_->OnRemoteRequestInnerThird(code, data, reply, option);
    EXPECT_EQ(res, NO_ERROR);

    code = IAbilityScheduler::SCHEDULE_COLLABORATE_DATA;
    res = stub_->OnRemoteRequestInnerThird(code, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilitySchedulerStubSecond_020
 * @tc.desc: test OnRemoteRequest
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSecondTest, AbilitySchedulerStubSecond_020, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    auto res = stub_->OnRemoteRequest(INT_MAX, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_STATE);
}

/**
 * @tc.name: AbilitySchedulerStubSecond_021
 * @tc.desc: test AbilityTransactionInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSecondTest, AbilitySchedulerStubSecond_021, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->AbilityTransactionInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    Want want;
    data.WriteParcelable(&want);
    res = stub_->AbilityTransactionInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilitySchedulerStubSecond_022
 * @tc.desc: test AbilityTransactionInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSecondTest, AbilitySchedulerStubSecond_022, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    Want want;
    LifeCycleStateInfo stateInfo;
    data.WriteParcelable(&want);
    data.WriteParcelable(&stateInfo);
    data.WriteBool(true);
    auto res = stub_->AbilityTransactionInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilitySchedulerStubSecond_023
 * @tc.desc: test DisconnectAbilityInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSecondTest, AbilitySchedulerStubSecond_023, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    Want want;
    data.WriteParcelable(&want);
    auto res = stub_->DisconnectAbilityInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilitySchedulerStubSecond_024
 * @tc.desc: test CommandAbilityInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSecondTest, AbilitySchedulerStubSecond_024, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->CommandAbilityInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    Want want;
    data.WriteParcelable(&want);
    res = stub_->CommandAbilityInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilitySchedulerStubSecond_025
 * @tc.desc: test PrepareTerminateAbilityInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSecondTest, AbilitySchedulerStubSecond_025, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->PrepareTerminateAbilityInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilitySchedulerStubSecond_026
 * @tc.desc: test CommandAbilityWindowInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSecondTest, AbilitySchedulerStubSecond_026, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->CommandAbilityWindowInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    Want want;
    data.WriteParcelable(&want);
    res = stub_->CommandAbilityWindowInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilitySchedulerStubSecond_027
 * @tc.desc: test RestoreAbilityStateInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSecondTest, AbilitySchedulerStubSecond_027, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->RestoreAbilityStateInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    PacMap pacMap;
    data.WriteParcelable(&pacMap);
    res = stub_->RestoreAbilityStateInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilitySchedulerStubSecond_028
 * @tc.desc: test GetFileTypesInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSecondTest, AbilitySchedulerStubSecond_028, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    auto res = stub_->GetFileTypesInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    Uri uri("test_uri");
    data.WriteParcelable(&uri);
    res = stub_->GetFileTypesInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    MessageParcel data2;
    data2.WriteParcelable(&uri);
    data2.WriteString("test");
    res = stub_->GetFileTypesInner(data2, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilitySchedulerStubSecond_029
 * @tc.desc: test OpenFileInner wangyi test failed
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSecondTest, AbilitySchedulerStubSecond_029, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    auto res = stub_->OpenFileInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    Uri uri("test_uri");
    data.WriteParcelable(&uri);
    res = stub_->OpenFileInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    MessageParcel data2;
    data2.WriteParcelable(&uri);
    data2.WriteString("r");
    res = stub_->OpenFileInner(data2, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilitySchedulerStubSecond_030
 * @tc.desc: test OpenRawFileInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSecondTest, AbilitySchedulerStubSecond_030, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    auto res = stub_->OpenRawFileInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    Uri uri("test_uri");
    data.WriteParcelable(&uri);
    res = stub_->OpenRawFileInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    MessageParcel data2;
    data2.WriteParcelable(&uri);
    data2.WriteString("w");
    res = stub_->OpenRawFileInner(data2, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilitySchedulerStubSecond_031
 * @tc.desc: test InsertInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSecondTest, AbilitySchedulerStubSecond_031, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    auto res = stub_->InsertInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    Uri uri("test_uri");
    data.WriteParcelable(&uri);
    res = stub_->InsertInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilitySchedulerStubSecond_032
 * @tc.desc: test CallInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSecondTest, AbilitySchedulerStubSecond_032, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    auto res = stub_->CallInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    Uri uri("test_uri");
    data.WriteParcelable(&uri);
    res = stub_->CallInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    MessageParcel data2;
    data2.WriteParcelable(&uri);
    data2.WriteString("test_str");
    data2.WriteString("test_str1");
    res = stub_->CallInner(data2, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    MessageParcel data3;
    PacMap pacMap;
    data3.WriteParcelable(&uri);
    data3.WriteString("test_str");
    data3.WriteString("test_str1");
    data3.WriteParcelable(&pacMap);
    res = stub_->CallInner(data3, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilitySchedulerStubSecond_033
 * @tc.desc: test UpdatetInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSecondTest, AbilitySchedulerStubSecond_033, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    auto res = stub_->UpdatetInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    Uri uri("test_uri");
    data.WriteParcelable(&uri);
    res = stub_->UpdatetInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    MessageParcel data2;
    NativeRdb::DataAbilityPredicates predicates;
    WriteInterfaceToken(data2);
    data2.WriteParcelable(&uri);
    data2.WriteParcelable(&predicates);
    res = stub_->UpdatetInner(data2, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilitySchedulerStubSecond_034
 * @tc.desc: test DeleteInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSecondTest, AbilitySchedulerStubSecond_034, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    auto res = stub_->DeleteInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    Uri uri("test_uri");
    data.WriteParcelable(&uri);
    res = stub_->DeleteInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    MessageParcel data2;
    NativeRdb::DataAbilityPredicates predicates;
    data2.WriteParcelable(&uri);
    data2.WriteParcelable(&predicates);
    res = stub_->DeleteInner(data2, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilitySchedulerStubSecond_035
 * @tc.desc: test QueryInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSecondTest, AbilitySchedulerStubSecond_035, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    auto res = stub_->QueryInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    Uri uri("test_uri");
    data.WriteParcelable(&uri);
    res = stub_->QueryInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    MessageParcel data2;
    NativeRdb::DataAbilityPredicates predicates;
    WriteInterfaceToken(data2);
    data2.WriteParcelable(&uri);
    data2.WriteParcelable(&predicates);
    res = stub_->QueryInner(data2, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilitySchedulerStubSecond_036
 * @tc.desc: test GetTypeInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSecondTest, AbilitySchedulerStubSecond_036, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    auto res = stub_->GetTypeInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    Uri uri("test_uri");
    data.WriteParcelable(&uri);
    res = stub_->GetTypeInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilitySchedulerStubSecond_037
 * @tc.desc: test ReloadInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSecondTest, AbilitySchedulerStubSecond_037, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    auto res = stub_->ReloadInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    Uri uri("test_uri");
    data.WriteParcelable(&uri);
    res = stub_->ReloadInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    MessageParcel data2;
    PacMap pacMap;
    data2.WriteParcelable(&uri);
    data2.WriteParcelable(&pacMap);
    res = stub_->ReloadInner(data2, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilitySchedulerStubSecond_038
 * @tc.desc: test ExecuteSkillInner with null want
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSecondTest, AbilitySchedulerStubSecond_038, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    auto res = stub_->ExecuteSkillInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilitySchedulerStubSecond_039
 * @tc.desc: test ExecuteSkillInner with valid want
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSecondTest, AbilitySchedulerStubSecond_039, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    Want want;
    data.WriteParcelable(&want);
    auto res = stub_->ExecuteSkillInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}
}  // namespace AAFwk
}  // namespace OHOS
