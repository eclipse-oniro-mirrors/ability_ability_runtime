/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "ability_manager_proxy.h"
#include "ability_manager_errors.h"
#include "ability_record.h"
#include "ability_scheduler.h"
#include "hilog_tag_wrapper.h"
#include "mission_snapshot.h"
#include "want_sender_info.h"
#include "ability_manager_stub_mock.h"

using namespace testing::ext;
using namespace testing;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
namespace {
const int32_t ZERO = 0;
const std::string EMPTY_STRING = "";
}  // namespace

class IRemoteObjectMocker : public IRemoteObject {
public:
    IRemoteObjectMocker() : IRemoteObject {u"IRemoteObjectMocker"}
    {
    }

    ~IRemoteObjectMocker()
    {
    }

    int32_t GetObjectRefCount()
    {
        return 0;
    }

    int SendRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
    {
        return 0;
    }

    bool IsProxyObject() const
    {
        return true;
    }

    bool CheckObjectLegality() const
    {
        return true;
    }

    bool AddDeathRecipient(const sptr<DeathRecipient>& recipient)
    {
        return true;
    }

    bool RemoveDeathRecipient(const sptr<DeathRecipient>& recipient)
    {
        return true;
    }

    bool AddRefreshRecipient(const sptr<RefreshRecipient> &recipient)
    {
        return true;
    }

    bool RemoveRefreshRecipient(const sptr<RefreshRecipient> &recipient)
    {
        return true;
    }

    sptr<IRemoteBroker> AsInterface()
    {
        return nullptr;
    }

    int Dump(int fd, const std::vector<std::u16string>& args)
    {
        return 0;
    }
};

class MockIWantSender : public IWantSender {
public:
    virtual ~MockIWantSender() {};

    sptr<IRemoteObject> AsObject() override
    {
        return iRemoteObjectFlags_;
    };
    void SetIRemoteObjectFlags(sptr<IRemoteObject> iRemoteObjectFlags)
    {
        iRemoteObjectFlags_ = iRemoteObjectFlags;
    };

    void Send(SenderInfo& senderInfo) override {};

private:
    sptr<IRemoteObject> iRemoteObjectFlags_ = nullptr;
};

class AbilityManagerProxyFifthTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<AbilityManagerProxy> proxy_{ nullptr };
    sptr<AbilityManagerStubMock> mock_{ nullptr };
};

void AbilityManagerProxyFifthTest::SetUpTestCase(void)
{}
void AbilityManagerProxyFifthTest::TearDownTestCase(void)
{}
void AbilityManagerProxyFifthTest::TearDown()
{}

void AbilityManagerProxyFifthTest::SetUp()
{
    mock_ = new AbilityManagerStubMock();
    proxy_ = std::make_shared<AbilityManagerProxy>(mock_);
}

/**
 * @tc.name: GetPendingWantBundleName_0100
 * @tc.desc: Test the GetPendingWantBundleName
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, GetPendingWantBundleName_0100, TestSize.Level1)
{
    sptr<IWantSender> testTarget = nullptr;
    auto res1 = proxy_->GetPendingWantBundleName(testTarget);
    EXPECT_EQ(res1, EMPTY_STRING);

    auto target = new MockIWantSender();
    ASSERT_NE(target, nullptr);
    auto object = new IRemoteObjectMocker();
    target->SetIRemoteObjectFlags(object);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    auto res2 = proxy_->GetPendingWantBundleName(target);
    EXPECT_EQ(res2, EMPTY_STRING);
}

/**
 * @tc.name: GetPendingWantBundleName_0200
 * @tc.desc: Test the GetPendingWantBundleName
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, GetPendingWantBundleName_0200, TestSize.Level1)
{
    auto target = new MockIWantSender();
    ASSERT_NE(target, nullptr);
    auto object = new IRemoteObjectMocker();
    target->SetIRemoteObjectFlags(object);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    auto res = proxy_->GetPendingWantBundleName(target);
    EXPECT_EQ(res, EMPTY_STRING);
}

/**
 * @tc.name: GetPendingWantCode_0100
 * @tc.desc: Test the GetPendingWantCode
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, GetPendingWantCode_0100, TestSize.Level1)
{
    sptr<IWantSender> testTarget = nullptr;
    auto res1 = proxy_->GetPendingWantCode(testTarget);
    EXPECT_EQ(res1, ERR_INVALID_VALUE);

    auto target = new MockIWantSender();
    ASSERT_NE(target, nullptr);
    auto object = new IRemoteObjectMocker();
    target->SetIRemoteObjectFlags(object);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    auto res2 = proxy_->GetPendingWantCode(target);
    EXPECT_EQ(res2, INNER_ERR);
}

/**
 * @tc.name: GetPendingWantCode_0200
 * @tc.desc: Test the GetPendingWantCode
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, GetPendingWantCode_0200, TestSize.Level1)
{
    auto target = new MockIWantSender();
    ASSERT_NE(target, nullptr);
    auto object = new IRemoteObjectMocker();
    target->SetIRemoteObjectFlags(object);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    auto res = proxy_->GetPendingWantCode(target);
    EXPECT_EQ(res, ZERO);
}

/**
 * @tc.name: GetPendingWantType_0100
 * @tc.desc: Test the GetPendingWantType
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, GetPendingWantType_0100, TestSize.Level1)
{
    sptr<IWantSender> testTarget = nullptr;
    auto res1 = proxy_->GetPendingWantType(testTarget);
    EXPECT_EQ(res1, ERR_INVALID_VALUE);

    auto target = new MockIWantSender();
    ASSERT_NE(target, nullptr);
    auto object = new IRemoteObjectMocker();
    target->SetIRemoteObjectFlags(object);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    auto res2 = proxy_->GetPendingWantType(target);
    EXPECT_EQ(res2, INNER_ERR);
}

/**
 * @tc.name: GetPendingWantType_0200
 * @tc.desc: Test the GetPendingWantType
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, GetPendingWantType_0200, TestSize.Level1)
{
    auto target = new MockIWantSender();
    ASSERT_NE(target, nullptr);
    auto object = new IRemoteObjectMocker();
    target->SetIRemoteObjectFlags(object);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    auto res = proxy_->GetPendingWantType(target);
    EXPECT_EQ(res, ZERO);
}

/**
 * @tc.name: GetPendingRequestWant_0100
 * @tc.desc: Test the GetPendingRequestWant
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, GetPendingRequestWant_0100, TestSize.Level1)
{
    sptr<IWantSender> testTarget = nullptr;
    std::shared_ptr<Want> want = nullptr;
    auto res1 = proxy_->GetPendingRequestWant(testTarget, want);
    EXPECT_EQ(res1, INNER_ERR);

    auto target = new MockIWantSender();
    ASSERT_NE(target, nullptr);
    auto object = new IRemoteObjectMocker();
    target->SetIRemoteObjectFlags(object);
    auto res2 = proxy_->GetPendingRequestWant(target, want);
    EXPECT_EQ(res2, INNER_ERR);

    auto target2 = new MockIWantSender();
    ASSERT_NE(target2, nullptr);
    auto object2 = new IRemoteObjectMocker();
    target2->SetIRemoteObjectFlags(object2);
    want = std::make_shared<Want>();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    auto res3 = proxy_->GetPendingRequestWant(target2, want);
    EXPECT_EQ(res3, INVALID_PARAMETERS_ERR);
}

/**
 * @tc.name: GetPendingRequestWant_0200
 * @tc.desc: Test the GetPendingRequestWant
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, GetPendingRequestWant_0200, TestSize.Level1)
{
    auto target = new MockIWantSender();
    ASSERT_NE(target, nullptr);
    auto object = new IRemoteObjectMocker();
    target->SetIRemoteObjectFlags(object);
    auto want = std::make_shared<Want>();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    auto res = proxy_->GetPendingRequestWant(target, want);
    EXPECT_EQ(res, INNER_ERR);
}

/**
 * @tc.name: GetWantSenderInfo_0100
 * @tc.desc: Test the GetWantSenderInfo
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, GetWantSenderInfo_0100, TestSize.Level1)
{
    sptr<IWantSender> testTarget = nullptr;
    std::shared_ptr<WantSenderInfo> info = nullptr;
    auto res1 = proxy_->GetWantSenderInfo(testTarget, info);
    EXPECT_EQ(res1, INNER_ERR);

    auto target = new MockIWantSender();
    ASSERT_NE(target, nullptr);
    auto object = new IRemoteObjectMocker();
    target->SetIRemoteObjectFlags(object);
    auto res2 = proxy_->GetWantSenderInfo(target, info);
    EXPECT_EQ(res2, INNER_ERR);

    auto target2 = new MockIWantSender();
    ASSERT_NE(target2, nullptr);
    auto object2 = new IRemoteObjectMocker();
    target2->SetIRemoteObjectFlags(object2);
    info = std::make_shared<WantSenderInfo>();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    auto res3 = proxy_->GetWantSenderInfo(target2, info);
    EXPECT_EQ(res3, INVALID_PARAMETERS_ERR);
}

/**
 * @tc.name: GetWantSenderInfo_0200
 * @tc.desc: Test the GetWantSenderInfo
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, GetWantSenderInfo_0200, TestSize.Level1)
{
    auto target = new MockIWantSender();
    ASSERT_NE(target, nullptr);
    auto object = new IRemoteObjectMocker();
    target->SetIRemoteObjectFlags(object);
    auto info = std::make_shared<WantSenderInfo>();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    auto res = proxy_->GetWantSenderInfo(target, info);
    EXPECT_EQ(res, INNER_ERR);
}

/**
 * @tc.name: GetAppMemorySize_0100
 * @tc.desc: Test the GetAppMemorySize
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, GetAppMemorySize_0100, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    auto res = proxy_->GetAppMemorySize();
    EXPECT_EQ(res, ZERO);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    auto res2 = proxy_->GetAppMemorySize();
    EXPECT_EQ(res2, INVALID_PARAMETERS_ERR);
}

/**
 * @tc.name: IsRamConstrainedDevice_0100
 * @tc.desc: Test the IsRamConstrainedDevice
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, IsRamConstrainedDevice_0100, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    auto res = proxy_->IsRamConstrainedDevice();
    EXPECT_EQ(res, false);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    auto res2 = proxy_->IsRamConstrainedDevice();
    EXPECT_EQ(res2, false);
}

/**
 * @tc.name: ContinueMission_0100
 * @tc.desc: Test the ContinueMission
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, ContinueMission_0100, TestSize.Level1)
{
    std::string srcDeviceId = "001";
    std::string dstDeviceId = "002";
    int32_t missionId = -1;
    sptr<IRemoteObject> callBack = nullptr;
    AAFwk::WantParams wantParams;
    auto res1 = proxy_->ContinueMission(srcDeviceId, dstDeviceId, missionId, callBack, wantParams);
    EXPECT_EQ(res1, INNER_ERR);

    sptr<IRemoteObject> callBack1 = new IRemoteObjectMocker();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    auto res2 = proxy_->ContinueMission(srcDeviceId, dstDeviceId, missionId, callBack1, wantParams);
    EXPECT_EQ(res2, INVALID_PARAMETERS_ERR);

    sptr<IRemoteObject> callBack2 = new IRemoteObjectMocker();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    auto res3 = proxy_->ContinueMission(srcDeviceId, dstDeviceId, missionId, callBack2, wantParams);
    EXPECT_EQ(res3, ZERO);
}

/**
 * @tc.name: ContinueMission_0200
 * @tc.desc: Test the ContinueMission
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, ContinueMission_0200, TestSize.Level1)
{
    AAFwk::ContinueMissionInfo continueMissionInfo;
    sptr<IRemoteObject> callBack = nullptr;
    auto res1 = proxy_->ContinueMission(continueMissionInfo, callBack);
    EXPECT_EQ(res1, INNER_ERR);

    sptr<IRemoteObject> callBack1 = new IRemoteObjectMocker();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    auto res2 = proxy_->ContinueMission(continueMissionInfo, callBack1);
    EXPECT_EQ(res2, INVALID_PARAMETERS_ERR);

    sptr<IRemoteObject> callBack2 = new IRemoteObjectMocker();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    auto res3 = proxy_->ContinueMission(continueMissionInfo, callBack2);
    EXPECT_EQ(res3, ZERO);
}

/**
 * @tc.name: ContinueAbility_0100
 * @tc.desc: Test the ContinueAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, ContinueAbility_0100, TestSize.Level1)
{
    std::string deviceId = "001";
    int32_t missionId = 2;
    uint32_t versionCode = 3;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    auto res = proxy_->ContinueAbility(deviceId, missionId, versionCode);
    EXPECT_EQ(res, INVALID_PARAMETERS_ERR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    auto res2 = proxy_->ContinueAbility(deviceId, missionId, versionCode);
    EXPECT_EQ(res2, ZERO);
}

/**
 * @tc.name: StartContinuation_0100
 * @tc.desc: Test the StartContinuation
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, StartContinuation_0100, TestSize.Level1)
{
    Want want;
    sptr<IRemoteObject> abilityToken = nullptr;
    int32_t status = 1;
    auto res1 = proxy_->StartContinuation(want, abilityToken, status);
    EXPECT_EQ(res1, INNER_ERR);

    sptr<IRemoteObject> abilityToken2 = new IRemoteObjectMocker();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    auto res2 = proxy_->StartContinuation(want, abilityToken2, status);
    EXPECT_EQ(res2, INVALID_PARAMETERS_ERR);

    sptr<IRemoteObject> abilityToken3 = new IRemoteObjectMocker();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    auto res3 = proxy_->StartContinuation(want, abilityToken3, status);
    EXPECT_EQ(res3, ZERO);
}

/**
 * @tc.name: NotifyContinuationResult_0100
 * @tc.desc: Test the NotifyContinuationResult
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, NotifyContinuationResult_0100, TestSize.Level1)
{
    int32_t missionId = 10;
    int32_t result = 12;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    auto res1 = proxy_->NotifyContinuationResult(missionId, result);
    EXPECT_EQ(res1, INVALID_PARAMETERS_ERR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    auto res2 = proxy_->NotifyContinuationResult(missionId, result);
    EXPECT_EQ(res2, ZERO);
}

/**
 * @tc.name: StartSyncRemoteMissions_0100
 * @tc.desc: Test the StartSyncRemoteMissions
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, StartSyncRemoteMissions_0100, TestSize.Level1)
{
    std::string devId = "100";
    bool fixConflict = false;
    int64_t tag = 1;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    auto res1 = proxy_->StartSyncRemoteMissions(devId, fixConflict, tag);
    EXPECT_EQ(res1, INVALID_PARAMETERS_ERR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    auto res2 = proxy_->StartSyncRemoteMissions(devId, fixConflict, tag);
    EXPECT_EQ(res2, ZERO);
}

/**
 * @tc.name: StopSyncRemoteMissions_0100
 * @tc.desc: Test the StopSyncRemoteMissions
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, StopSyncRemoteMissions_0100, TestSize.Level1)
{
    std::string devId = "100";
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    auto res1 = proxy_->StopSyncRemoteMissions(devId);
    EXPECT_EQ(res1, INVALID_PARAMETERS_ERR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    auto res2 = proxy_->StopSyncRemoteMissions(devId);
    EXPECT_EQ(res2, ZERO);
}

/**
 * @tc.name: LockMissionForCleanup_0100
 * @tc.desc: Test the LockMissionForCleanup
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, LockMissionForCleanup_0100, TestSize.Level1)
{
    int32_t missionId = 1;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    auto res1 = proxy_->LockMissionForCleanup(missionId);
    EXPECT_EQ(res1, INVALID_PARAMETERS_ERR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    auto res2 = proxy_->LockMissionForCleanup(missionId);
    EXPECT_EQ(res2, ZERO);
}

/**
 * @tc.name: UnlockMissionForCleanup_0100
 * @tc.desc: Test the UnlockMissionForCleanup
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, UnlockMissionForCleanup_0100, TestSize.Level1)
{
    int32_t missionId = 1;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    auto res1 = proxy_->UnlockMissionForCleanup(missionId);
    EXPECT_EQ(res1, INVALID_PARAMETERS_ERR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    auto res2 = proxy_->UnlockMissionForCleanup(missionId);
    EXPECT_EQ(res2, ZERO);
}

/**
 * @tc.name: GetMissionInfos_0100
 * @tc.desc: Test the GetMissionInfos
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, GetMissionInfos_0100, TestSize.Level1)
{
    std::string deviceId = "100";
    int32_t numMax = 100;
    std::vector<MissionInfo> missionInfos;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    auto res1 = proxy_->GetMissionInfos(deviceId, numMax, missionInfos);
    EXPECT_EQ(res1, INVALID_PARAMETERS_ERR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    auto res2 = proxy_->GetMissionInfos(deviceId, numMax, missionInfos);
    EXPECT_EQ(res2, ZERO);
}

/**
 * @tc.name: GetMissionInfo_0100
 * @tc.desc: Test the GetMissionInfo
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, GetMissionInfo_0100, TestSize.Level1)
{
    std::string deviceId = "100";
    int32_t numMax = 100;
    MissionInfo missionInfo;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    auto res1 = proxy_->GetMissionInfo(deviceId, numMax, missionInfo);
    EXPECT_EQ(res1, INVALID_PARAMETERS_ERR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    auto res2 = proxy_->GetMissionInfo(deviceId, numMax, missionInfo);
    EXPECT_EQ(res2, ERR_UNKNOWN_OBJECT);
}

/**
 * @tc.name: KillProcessWithReason_0100
 * @tc.desc: Test the KillProcessWithReason
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, KillProcessWithReason_0100, TestSize.Level1)
{
    int32_t pid = 100;
    ExitReason reason;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    auto res1 = proxy_->KillProcessWithReason(pid, reason);
    EXPECT_EQ(res1, INVALID_PARAMETERS_ERR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    auto res2 = proxy_->KillProcessWithReason(pid, reason);
    EXPECT_EQ(res2, ZERO);
}

/**
 * @tc.name: RegisterAutoStartupSystemCallback_0100
 * @tc.desc: Test the RegisterAutoStartupSystemCallback
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, RegisterAutoStartupSystemCallback_0100, TestSize.Level1)
{
    sptr<IRemoteObject> callback = nullptr;
    auto res = proxy_->RegisterAutoStartupSystemCallback(callback);
    EXPECT_EQ(res, INNER_ERR);

    sptr<IRemoteObject> callback2 = new IRemoteObjectMocker();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    auto res1 = proxy_->RegisterAutoStartupSystemCallback(callback2);
    EXPECT_EQ(res1, INVALID_PARAMETERS_ERR);

    sptr<IRemoteObject> callback3 = new IRemoteObjectMocker();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    auto res2 = proxy_->RegisterAutoStartupSystemCallback(callback3);
    EXPECT_EQ(res2, ZERO);
}

/**
 * @tc.name: UnregisterAutoStartupSystemCallback_0100
 * @tc.desc: Test the UnregisterAutoStartupSystemCallback
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, UnregisterAutoStartupSystemCallback_0100, TestSize.Level1)
{
    sptr<IRemoteObject> callback = nullptr;
    auto res = proxy_->UnregisterAutoStartupSystemCallback(callback);
    EXPECT_EQ(res, INNER_ERR);

    sptr<IRemoteObject> callback2 = new IRemoteObjectMocker();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    auto res1 = proxy_->UnregisterAutoStartupSystemCallback(callback2);
    EXPECT_EQ(res1, INVALID_PARAMETERS_ERR);

    sptr<IRemoteObject> callback3 = new IRemoteObjectMocker();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    auto res2 = proxy_->UnregisterAutoStartupSystemCallback(callback3);
    EXPECT_EQ(res2, ZERO);
}

/**
 * @tc.name: SetApplicationAutoStartup_0100
 * @tc.desc: Test the SetApplicationAutoStartup
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, SetApplicationAutoStartup_0100, TestSize.Level1)
{
    AutoStartupInfo info;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    auto res1 = proxy_->SetApplicationAutoStartup(info);
    EXPECT_EQ(res1, INVALID_PARAMETERS_ERR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    auto res2 = proxy_->SetApplicationAutoStartup(info);
    EXPECT_EQ(res2, ZERO);
}

/**
 * @tc.name: CancelApplicationAutoStartup_0100
 * @tc.desc: Test the CancelApplicationAutoStartup
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, CancelApplicationAutoStartup_0100, TestSize.Level1)
{
    AutoStartupInfo info;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    auto res1 = proxy_->CancelApplicationAutoStartup(info);
    EXPECT_EQ(res1, INVALID_PARAMETERS_ERR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    auto res2 = proxy_->CancelApplicationAutoStartup(info);
    EXPECT_EQ(res2, ZERO);
}

/**
 * @tc.name: QueryAllAutoStartupApplications_0100
 * @tc.desc: Test the QueryAllAutoStartupApplications
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, QueryAllAutoStartupApplications_0100, TestSize.Level1)
{
    std::vector<AutoStartupInfo> infoList;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    auto res1 = proxy_->QueryAllAutoStartupApplications(infoList);
    EXPECT_EQ(res1, INVALID_PARAMETERS_ERR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    auto res2 = proxy_->QueryAllAutoStartupApplications(infoList);
    EXPECT_EQ(res2, ZERO);
}

/**
 * @tc.name: GetAutoStartupStatusForSelf_0100
 * @tc.desc: Test the GetAutoStartupStatusForSelf
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, GetAutoStartupStatusForSelf_0100, TestSize.Level1)
{
    bool isAutoStartEnabled = false;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    auto res1 = proxy_->GetAutoStartupStatusForSelf(isAutoStartEnabled);
    EXPECT_EQ(res1, INVALID_PARAMETERS_ERR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    auto res2 = proxy_->GetAutoStartupStatusForSelf(isAutoStartEnabled);
    EXPECT_EQ(res2, ZERO);
}

/**
 * @tc.name: RegisterSessionHandler_0100
 * @tc.desc: Test the RegisterSessionHandler
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, RegisterSessionHandler_0100, TestSize.Level1)
{
    sptr<IRemoteObject> object = nullptr;
    auto res = proxy_->RegisterSessionHandler(object);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    sptr<IRemoteObject> object2 = new IRemoteObjectMocker();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    auto res1 = proxy_->RegisterSessionHandler(object2);
    EXPECT_EQ(res1, INVALID_PARAMETERS_ERR);

    sptr<IRemoteObject> object3 = new IRemoteObjectMocker();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    auto res2 = proxy_->RegisterSessionHandler(object3);
    EXPECT_EQ(res2, ZERO);
}

/**
 * @tc.name: RegisterAppDebugListener_0100
 * @tc.desc: Test the RegisterAppDebugListener
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, RegisterAppDebugListener_0100, TestSize.Level1)
{
    sptr<AppExecFwk::IAppDebugListener> listener = nullptr;
    auto res = proxy_->RegisterAppDebugListener(listener);
    EXPECT_EQ(res, INNER_ERR);
}

/**
 * @tc.name: UnregisterAppDebugListener_0100
 * @tc.desc: Test the UnregisterAppDebugListener
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, UnregisterAppDebugListener_0100, TestSize.Level1)
{
    sptr<AppExecFwk::IAppDebugListener> listener = nullptr;
    auto res = proxy_->UnregisterAppDebugListener(listener);
    EXPECT_EQ(res, INNER_ERR);
}

/**
 * @tc.name: AttachAppDebug_0100
 * @tc.desc: Test the AttachAppDebug
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, AttachAppDebug_0100, TestSize.Level1)
{
    std::string bundleName = "bundleName";
    bool isDebugFromLocal = false;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    auto res1 = proxy_->AttachAppDebug(bundleName, isDebugFromLocal);
    EXPECT_EQ(res1, INVALID_PARAMETERS_ERR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    auto res2 = proxy_->AttachAppDebug(bundleName, isDebugFromLocal);
    EXPECT_EQ(res2, ZERO);
}

/**
 * @tc.name: DetachAppDebug_0100
 * @tc.desc: Test the DetachAppDebug
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, DetachAppDebug_0100, TestSize.Level1)
{
    std::string bundleName = "bundleName";
    bool isDebugFromLocal = false;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    auto res1 = proxy_->DetachAppDebug(bundleName, isDebugFromLocal);
    EXPECT_EQ(res1, INVALID_PARAMETERS_ERR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    auto res2 = proxy_->DetachAppDebug(bundleName, isDebugFromLocal);
    EXPECT_EQ(res2, ZERO);
}

/**
 * @tc.name: ExecuteIntent_0100
 * @tc.desc: Test the ExecuteIntent
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, ExecuteIntent_0100, TestSize.Level1)
{
    uint64_t key = 1;
    InsightIntentExecuteParam param;
    sptr<IRemoteObject> callerToken = nullptr;
    auto res = proxy_->ExecuteIntent(key, callerToken, param);
    EXPECT_EQ(res, INNER_ERR);

    sptr<IRemoteObject> callerToken2 = new IRemoteObjectMocker();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    auto res1 = proxy_->ExecuteIntent(key, callerToken2, param);
    EXPECT_EQ(res1, INVALID_PARAMETERS_ERR);

    sptr<IRemoteObject> callerToken3 = new IRemoteObjectMocker();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    auto res2 = proxy_->ExecuteIntent(key, callerToken3, param);
    EXPECT_EQ(res2, ZERO);
}

/**
 * @tc.name: IsAbilityControllerStart_0100
 * @tc.desc: Test the IsAbilityControllerStart
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, IsAbilityControllerStart_0100, TestSize.Level1)
{
    Want want;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    auto res1 = proxy_->IsAbilityControllerStart(want);
    EXPECT_EQ(res1, true);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    auto res2 = proxy_->IsAbilityControllerStart(want);
    EXPECT_EQ(res2, false);
}

/**
 * @tc.name: SetApplicationAutoStartupByEDM_0100
 * @tc.desc: Test the SetApplicationAutoStartupByEDM
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, SetApplicationAutoStartupByEDM_0100, TestSize.Level1)
{
    AutoStartupInfo info;
    bool flag = false;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    auto res1 = proxy_->SetApplicationAutoStartupByEDM(info, flag);
    EXPECT_EQ(res1, INVALID_PARAMETERS_ERR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    auto res2 = proxy_->SetApplicationAutoStartupByEDM(info, flag);
    EXPECT_EQ(res2, ZERO);
}

/**
 * @tc.name: CancelApplicationAutoStartupByEDM_0100
 * @tc.desc: Test the CancelApplicationAutoStartupByEDM
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, CancelApplicationAutoStartupByEDM_0100, TestSize.Level1)
{
    AutoStartupInfo info;
    bool flag = false;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    auto res1 = proxy_->CancelApplicationAutoStartupByEDM(info, flag);
    EXPECT_EQ(res1, INVALID_PARAMETERS_ERR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    auto res2 = proxy_->CancelApplicationAutoStartupByEDM(info, flag);
    EXPECT_EQ(res2, ZERO);
}

/**
 * @tc.name: GetForegroundUIAbilities_0100
 * @tc.desc: Test the GetForegroundUIAbilities
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, GetForegroundUIAbilities_0100, TestSize.Level1)
{
    std::vector<AppExecFwk::AbilityStateData> list;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    auto res1 = proxy_->GetForegroundUIAbilities(list);
    EXPECT_EQ(res1, INVALID_PARAMETERS_ERR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    auto res2 = proxy_->GetForegroundUIAbilities(list);
    EXPECT_EQ(res2, ZERO);
}

/**
 * @tc.name: OpenFile_0100
 * @tc.desc: Test the OpenFile
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyFifthTest, OpenFile_0100, TestSize.Level1)
{
    Uri uri("test");
    uint32_t flag = 1;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    auto res1 = proxy_->OpenFile(uri, flag);
    EXPECT_EQ(res1, INVALID_PARAMETERS_ERR);
}
} // namespace AAFwk
} // namespace OHOS
