/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#define private public
#include "fault_data.h"
#include "message_parcel.h"
#include "mock_app_scheduler.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class AppSchedulerHostTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<MockAppScheduler> mockAppScheduler_;

    void WriteInterfaceToken(MessageParcel& data);
};

void AppSchedulerHostTest::SetUpTestCase(void)
{}

void AppSchedulerHostTest::TearDownTestCase(void)
{}

void AppSchedulerHostTest::SetUp()
{
    GTEST_LOG_(INFO) << "AppSchedulerHostTest::SetUp()";

    mockAppScheduler_ = new MockAppScheduler();
}

void AppSchedulerHostTest::TearDown()
{}

void AppSchedulerHostTest::WriteInterfaceToken(MessageParcel& data)
{
    GTEST_LOG_(INFO) << "AppSchedulerHostTest::WriteInterfaceToken()";

    data.WriteInterfaceToken(AppSchedulerHost::GetDescriptor());
}

/**
 * @tc.name: HandleNotifyAppFault_001
 * @tc.desc: Verify that the HandleNotifyAppFault interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(AppSchedulerHostTest, HandleNotifyAppFault_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);
    FaultData faultData;
    faultData.errorObject.name = "testName";
    faultData.errorObject.message = "testMessage";
    faultData.errorObject.stack = "testStack";
    faultData.faultType = FaultDataType::UNKNOWN;
    data.WriteParcelable(&faultData);
    EXPECT_CALL(*mockAppScheduler_, ScheduleNotifyAppFault(_)).Times(1);
    auto result = mockAppScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_NOTIFY_FAULT), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: ScheduleChangeAppGcState_001
 * @tc.desc: Verify that the ScheduleChangeAppGcState interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(AppSchedulerHostTest, ScheduleChangeAppGcState_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);
    data.WriteInt32(0);
    EXPECT_CALL(*mockAppScheduler_, ScheduleChangeAppGcState(_, _)).Times(1);
    auto result = mockAppScheduler_->OnRemoteRequest(
            static_cast<uint32_t>(IAppScheduler::Message::APP_GC_STATE_CHANGE), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: HandleAttachAppDebug_001
 * @tc.desc: Verify that HandleAttachAppDebug interface calls normally.
 * @tc.type: FUNC
 */
HWTEST_F(AppSchedulerHostTest, HandleAttachAppDebug_001, TestSize.Level1)
{
    EXPECT_NE(mockAppScheduler_, nullptr);
    EXPECT_CALL(*mockAppScheduler_, AttachAppDebug(_)).Times(1);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);

    auto result = mockAppScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_ATTACH_APP_DEBUG), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: HandleAttachAppDebug_002
 * @tc.desc: Check null descriptor.
 * @tc.type: FUNC
 */
HWTEST_F(AppSchedulerHostTest, HandleAttachAppDebug_002, TestSize.Level1)
{
    EXPECT_NE(mockAppScheduler_, nullptr);
    EXPECT_CALL(*mockAppScheduler_, AttachAppDebug(_)).Times(0);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    auto result = mockAppScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_ATTACH_APP_DEBUG), data, reply, option);
    EXPECT_EQ(result, ERR_INVALID_STATE);
}

/**
 * @tc.name: HandleDetachAppDebug_001
 * @tc.desc: Verify that HandleDetachAppDebug interface calls normally.
 * @tc.type: FUNC
 */
HWTEST_F(AppSchedulerHostTest, HandleDetachAppDebug_001, TestSize.Level1)
{
    EXPECT_NE(mockAppScheduler_, nullptr);
    EXPECT_CALL(*mockAppScheduler_, DetachAppDebug()).Times(1);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);

    auto result = mockAppScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_DETACH_APP_DEBUG), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: HandleDetachAppDebug_002
 * @tc.desc: Check unnown message code.
 * @tc.type: FUNC
 */
HWTEST_F(AppSchedulerHostTest, HandleDetachAppDebug_002, TestSize.Level1)
{
    EXPECT_NE(mockAppScheduler_, nullptr);
    EXPECT_CALL(*mockAppScheduler_, DetachAppDebug()).Times(0);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);
    uint32_t UNKNOWN_CODE = -1;

    auto result = mockAppScheduler_->OnRemoteRequest(UNKNOWN_CODE, data, reply, option);
    EXPECT_EQ(result, IPC_STUB_UNKNOW_TRANS_ERR);
}

/**
 * @tc.name: HandleScheduleDumpArkWeb_001
 * @tc.desc: Verify that the HandleScheduleDumpArkWeb interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(AppSchedulerHostTest, HandleScheduleDumpArkWeb_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);
    data.WriteString("");
    EXPECT_CALL(*mockAppScheduler_, ScheduleDumpArkWeb(_, _)).Times(1);
    auto result = mockAppScheduler_->OnRemoteRequest(
            static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_DUMP_ARKWEB), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: HandleScheduleLaunchAbility_ShouldReturnNoErrWhenNoUpdateInfo
 * @tc.desc: Verify that HandleScheduleLaunchAbility interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(AppSchedulerHostTest, HandleScheduleLaunchAbility_ShouldReturnNoErrWhenNoUpdateInfo, TestSize.Level1)
{
    EXPECT_NE(mockAppScheduler_, nullptr);
    EXPECT_CALL(*mockAppScheduler_, ScheduleLaunchAbility(_, _, _, _, _)).Times(1);
    MessageParcel data;
    MessageParcel reply;
    AbilityInfo abilityInfo;
    data.WriteParcelable(&abilityInfo);
    data.WriteBool(false);
    auto want = std::make_shared<AAFwk::Want>();
    data.WriteParcelable(want.get());
    int32_t abilityRecordId = 0;
    data.WriteInt32(abilityRecordId);
    data.WriteBool(false);
    EXPECT_EQ(mockAppScheduler_->HandleScheduleLaunchAbility(data, reply), NO_ERROR);
}

/**
 * @tc.name: HandleScheduleLaunchAbility_ShouldReturnNoErrWhenHaveUpdateInfo
 * @tc.desc: Verify that HandleScheduleLaunchAbility interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(AppSchedulerHostTest, HandleScheduleLaunchAbility_ShouldReturnNoErrWhenHaveUpdateInfo, TestSize.Level1)
{
    EXPECT_NE(mockAppScheduler_, nullptr);
    EXPECT_CALL(*mockAppScheduler_, ScheduleLaunchAbility(_, _, _, _, _)).Times(1);
    MessageParcel data;
    MessageParcel reply;
    AbilityInfo abilityInfo;
    data.WriteParcelable(&abilityInfo);
    data.WriteBool(false);
    auto want = std::make_shared<AAFwk::Want>();
    data.WriteParcelable(want.get());
    int32_t abilityRecordId = 0;
    data.WriteInt32(abilityRecordId);
    auto updateInfo = std::make_shared<AppUpdateInfo>();
    data.WriteBool(true);
    data.WriteParcelable(updateInfo.get());
    EXPECT_EQ(mockAppScheduler_->HandleScheduleLaunchAbility(data, reply), NO_ERROR);
}
} // namespace AppExecFwk
} // namespace OHOS
