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

#define private public
#include "mission_manager_client.h"
#undef private
#include "ability_manager_errors.h"
#include "ability_connect_callback_stub.h"
#include "ability_manager_client.h"
#include "appexecfwk_errors.h"
#include "scene_board_judgement.h"
#include "mission_snapshot.h"
#include "mission_info.h"
#include "ability_connect_callback_stub.h"

using namespace testing::ext;
using namespace OHOS::AppExecFwk;

const std::string BUNDLE_NAME = "bundleName";

namespace OHOS {
namespace AAFwk {

class MissionManagerClientTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<MissionManagerClient> client_{ nullptr };
};

class AbilityConnectCallback : public AbilityConnectionStub {
public:
    AbilityConnectCallback() {};
    virtual ~AbilityConnectCallback() {};
    virtual void OnAbilityConnectDone(
        const AppExecFwk::ElementName& element, const sptr<IRemoteObject>& remoteObject, int resultCode) override
    {}
    virtual void OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int resultCode) override
    {}
    static int onAbilityConnectDoneCount;
    static int onAbilityDisconnectDoneCount;
};

class MockIRemoteMissionListener : public IRemoteMissionListener {
public:
    MockIRemoteMissionListener() {};
    virtual ~MockIRemoteMissionListener() {};
    void NotifyMissionsChanged(const std::string& deviceId) override
    {}

    void NotifySnapshot(const std::string& deviceId, int32_t missionId) override
    {}

    void NotifyNetDisconnect(const std::string& deviceId, int32_t state) override
    {}
    sptr<IRemoteObject> AsObject() override
    { return nullptr; }
};

void MissionManagerClientTest::SetUpTestCase(void)
{}
void MissionManagerClientTest::TearDownTestCase(void)
{}
void MissionManagerClientTest::SetUp(void)
{
    client_ = std::make_shared<MissionManagerClient>();
}
void MissionManagerClientTest::TearDown(void)
{}

/**
 * @tc.name: MissionManagerClient_ContinueMission_0100
 * @tc.desc: ContinueMission
 * @tc.type: FUNC
 */
HWTEST_F(MissionManagerClientTest, ContinueMission_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ContinueMission_0100 start";
    std::string srcDeviceId = "123";
    std::string dstDeviceId = "ABC";
    int32_t missionId = 5;
    sptr<IRemoteObject> callback = nullptr;
    WantParams wantParams;
    auto result = client_->ContinueMission(srcDeviceId, dstDeviceId, missionId, callback, wantParams);

    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "ContinueMission_0100 end";
}

/**
 * @tc.name: MissionManagerClient_ContinueMission_0200
 * @tc.desc: ContinueMission
 * @tc.type: FUNC
 */
HWTEST_F(MissionManagerClientTest, ContinueMission_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ContinueMission_0200 start";
    std::string srcDeviceId = "";
    std::string dstDeviceId = "ABC";
    int32_t missionId = 5;
    sptr<IRemoteObject> callback = nullptr;
    WantParams wantParams;
    auto result = client_->ContinueMission(srcDeviceId, dstDeviceId, missionId, callback, wantParams);

    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "ContinueMission_0200 end";
}

/**
 * @tc.name: MissionManagerClient_ContinueMission_0300
 * @tc.desc: ContinueMission
 * @tc.type: FUNC
 */
HWTEST_F(MissionManagerClientTest, ContinueMission_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ContinueMission_0300 start";
    std::string srcDeviceId = "123";
    std::string dstDeviceId = "";
    int32_t missionId = 5;
    sptr<IRemoteObject> callback = new (std::nothrow) OHOS::AAFwk::AbilityConnectCallback();
    WantParams wantParams;
    auto result = client_->ContinueMission(srcDeviceId, dstDeviceId, missionId, callback, wantParams);

    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "ContinueMission_0300 end";
}

/**
 * @tc.name: MissionManagerClient_ContinueMission_0400
 * @tc.desc: ContinueMission
 * @tc.type: FUNC
 */
HWTEST_F(MissionManagerClientTest, ContinueMission_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ContinueMission_0400 start";
    std::string srcDeviceId = "";
    std::string dstDeviceId = "ABC";
    int32_t missionId = 5;
    sptr<IRemoteObject> callback = new (std::nothrow) OHOS::AAFwk::AbilityConnectCallback();
    WantParams wantParams;
    auto result = client_->ContinueMission(srcDeviceId, dstDeviceId, missionId, callback, wantParams);

    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "ContinueMission_0400 end";
}

/**
 * @tc.name: MissionManagerClient_ContinueMission_0500
 * @tc.desc: ContinueMission
 * @tc.type: FUNC
 */
HWTEST_F(MissionManagerClientTest, ContinueMission_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ContinueMission_0500 start";
    std::string srcDeviceId = "";
    std::string dstDeviceId = "";
    int32_t missionId = 5;
    sptr<IRemoteObject> callback = new (std::nothrow) OHOS::AAFwk::AbilityConnectCallback();
    WantParams wantParams;
    auto result = client_->ContinueMission(srcDeviceId, dstDeviceId, missionId, callback, wantParams);

    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "ContinueMission_0500 end";
}

/**
 * @tc.name: MissionManagerClient_ContinueMission_0600
 * @tc.desc: ContinueMission
 * @tc.type: FUNC
 */
HWTEST_F(MissionManagerClientTest, ContinueMission_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ContinueMission_0600 start";
    std::string srcDeviceId = "123";
    std::string dstDeviceId = "";
    int32_t missionId = 5;
    sptr<IRemoteObject> callback = nullptr;
    WantParams wantParams;
    auto result = client_->ContinueMission(srcDeviceId, dstDeviceId, missionId, callback, wantParams);

    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "ContinueMission_0600 end";
}

/**
 * @tc.name: MissionManagerClient_ContinueMission_0700
 * @tc.desc: ContinueMission
 * @tc.type: FUNC
 */
HWTEST_F(MissionManagerClientTest, ContinueMission_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ContinueMission_0700 start";
    std::string srcDeviceId = "123";
    std::string dstDeviceId = "ABC";
    int32_t missionId = 5;
    sptr<IRemoteObject> callback = new (std::nothrow) OHOS::AAFwk::AbilityConnectCallback();
    WantParams wantParams;
    auto result = client_->ContinueMission(srcDeviceId, dstDeviceId, missionId, callback, wantParams);

    EXPECT_EQ(result, ERR_INVALID_STATE);
    GTEST_LOG_(INFO) << "ContinueMission_0700 end";
}

/**
 * @tc.name: MissionManagerClient_ContinueMission_0800
 * @tc.desc: ContinueMission
 * @tc.type: FUNC
 */
HWTEST_F(MissionManagerClientTest, ContinueMission_0800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ContinueMission_0800 start";
    AAFwk::ContinueMissionInfo continueMissionInfo;
    continueMissionInfo.srcDeviceId = "123";
    continueMissionInfo.dstDeviceId = "ABC";
    sptr<IRemoteObject> callback = nullptr;
    auto result = client_->ContinueMission(continueMissionInfo, callback);

    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "ContinueMission_0800 end";
}

/**
 * @tc.name: MissionManagerClient_ContinueMission_0900
 * @tc.desc: ContinueMission
 * @tc.type: FUNC
 */
HWTEST_F(MissionManagerClientTest, ContinueMission_0900, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ContinueMission_0900 start";
    AAFwk::ContinueMissionInfo continueMissionInfo;
    continueMissionInfo.srcDeviceId = "";
    continueMissionInfo.dstDeviceId = "ABC";
    sptr<IRemoteObject> callback = nullptr;
    auto result = client_->ContinueMission(continueMissionInfo, callback);

    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "ContinueMission_0900 end";
}

/**
 * @tc.name: MissionManagerClient_ContinueMission_1000
 * @tc.desc: ContinueMission
 * @tc.type: FUNC
 */
HWTEST_F(MissionManagerClientTest, ContinueMission_1000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ContinueMission_1000 start";
    AAFwk::ContinueMissionInfo continueMissionInfo;
    continueMissionInfo.srcDeviceId = "123";
    continueMissionInfo.dstDeviceId = "";
    sptr<IRemoteObject> callback = new (std::nothrow) OHOS::AAFwk::AbilityConnectCallback();
    auto result = client_->ContinueMission(continueMissionInfo, callback);

    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "ContinueMission_1000 end";
}

/**
 * @tc.name: MissionManagerClient_ContinueMission_1100
 * @tc.desc: ContinueMission
 * @tc.type: FUNC
 */
HWTEST_F(MissionManagerClientTest, ContinueMission_1100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ContinueMission_1100 start";
    AAFwk::ContinueMissionInfo continueMissionInfo;
    continueMissionInfo.srcDeviceId = "";
    continueMissionInfo.dstDeviceId = "ABC";
    sptr<IRemoteObject> callback = new (std::nothrow) OHOS::AAFwk::AbilityConnectCallback();
    auto result = client_->ContinueMission(continueMissionInfo, callback);

    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "ContinueMission_1100 end";
}

/**
 * @tc.name: MissionManagerClient_ContinueMission_1200
 * @tc.desc: ContinueMission
 * @tc.type: FUNC
 */
HWTEST_F(MissionManagerClientTest, ContinueMission_1200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ContinueMission_1200 start";
    AAFwk::ContinueMissionInfo continueMissionInfo;
    continueMissionInfo.srcDeviceId = "";
    continueMissionInfo.dstDeviceId = "";
    sptr<IRemoteObject> callback = new (std::nothrow) OHOS::AAFwk::AbilityConnectCallback();
    auto result = client_->ContinueMission(continueMissionInfo, callback);

    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "ContinueMission_1200 end";
}

/**
 * @tc.name: MissionManagerClient_ContinueMission_1300
 * @tc.desc: ContinueMission
 * @tc.type: FUNC
 */
HWTEST_F(MissionManagerClientTest, ContinueMission_1300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ContinueMission_1300 start";
    AAFwk::ContinueMissionInfo continueMissionInfo;
    continueMissionInfo.srcDeviceId = "123";
    continueMissionInfo.dstDeviceId = "";
    sptr<IRemoteObject> callback = nullptr;
    auto result = client_->ContinueMission(continueMissionInfo, callback);

    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "ContinueMission_1300 end";
}

/**
 * @tc.name: MissionManagerClient_ContinueMission_1400
 * @tc.desc: ContinueMission
 * @tc.type: FUNC
 */
HWTEST_F(MissionManagerClientTest, ContinueMission_1400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ContinueMission_1400 start";
    AAFwk::ContinueMissionInfo continueMissionInfo;
    continueMissionInfo.srcDeviceId = "123";
    continueMissionInfo.dstDeviceId = "ABC";
    sptr<IRemoteObject> callback = new (std::nothrow) OHOS::AAFwk::AbilityConnectCallback();
    auto result = client_->ContinueMission(continueMissionInfo, callback);

    EXPECT_EQ(result, ERR_INVALID_STATE);
    GTEST_LOG_(INFO) << "ContinueMission_1400 end";
}

/**
 * @tc.name: MissionManagerClient_LockMissionForCleanup_0100
 * @tc.desc: LockMissionForCleanup
 * @tc.type: FUNC
 * @tc.require: issueI5NRWT
 */
HWTEST_F(MissionManagerClientTest, LockMissionForCleanup_0100, TestSize.Level1)
{
    EXPECT_TRUE(client_ != nullptr);
    auto result = client_->LockMissionForCleanup(1);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(ERR_INVALID_STATE, result);
    }
}

/**
 * @tc.name: MissionManagerClient_LockMissionForCleanup_0200
 * @tc.desc: LockMissionForCleanup
 * @tc.type: FUNC
 * @tc.require: issueI5NRWT
 */
HWTEST_F(MissionManagerClientTest, LockMissionForCleanup_0200, TestSize.Level1)
{
    EXPECT_TRUE(client_ != nullptr);
    auto result = client_->LockMissionForCleanup(1);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, ERR_INVALID_STATE);
    }
}

/**
 * @tc.name: MissionManagerClient_UnlockMissionForCleanup_0100
 * @tc.desc: UnlockMissionForCleanup
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(MissionManagerClientTest, UnlockMissionForCleanup_0100, TestSize.Level1)
{
    EXPECT_TRUE(client_ != nullptr);
    auto result = client_->UnlockMissionForCleanup(5);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, ERR_INVALID_STATE);
    }
}

/**
 * @tc.name: MissionManagerClient_UnlockMissionForCleanup_0200
 * @tc.desc: UnlockMissionForCleanup
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(MissionManagerClientTest, UnlockMissionForCleanup_0200, TestSize.Level1)
{
    EXPECT_TRUE(client_ != nullptr);
    auto result = client_->UnlockMissionForCleanup(5);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, ERR_INVALID_STATE);
    }
}


/**
 * @tc.name: MissionManagerClient_UnlockMissionForCleanup_0300
 * @tc.desc: UnlockMissionForCleanup
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(MissionManagerClientTest, UnlockMissionForCleanup_0300, TestSize.Level1)
{
    EXPECT_TRUE(client_ != nullptr);
    auto result = client_->UnlockMissionForCleanup(5);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, ERR_INVALID_STATE);
    }
}

/**
 * @tc.name: MissionManagerClient_UnlockMissionForCleanup_0400
 * @tc.desc: UnlockMissionForCleanup
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(MissionManagerClientTest, UnlockMissionForCleanup_0400, TestSize.Level1)
{
    EXPECT_TRUE(client_ != nullptr);
    auto result = client_->UnlockMissionForCleanup(5);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, ERR_INVALID_STATE);
    }
}

/**
 * @tc.name: MissionManagerClient_RegisterMissionListener_0100
 * @tc.desc: RegisterMissionListener
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(MissionManagerClientTest, RegisterMissionListener_0100, TestSize.Level1)
{
    EXPECT_TRUE(client_ != nullptr);
    sptr<IMissionListener> listener = nullptr;
    auto result = client_->RegisterMissionListener(listener);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, INVALID_PARAMETERS_ERR);
    }
}

/**
 * @tc.name: MissionManagerClient_UnRegisterMissionListener_0100
 * @tc.desc: UnRegisterMissionListener
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(MissionManagerClientTest, UnRegisterMissionListener_0100, TestSize.Level1)
{
    EXPECT_TRUE(client_ != nullptr);
    sptr<IMissionListener> listener = nullptr;
    auto result = client_->UnRegisterMissionListener(listener);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, INVALID_PARAMETERS_ERR);
    }
}

/**
 * @tc.name: MissionManagerClient_RegisterMissionListener_0200
 * @tc.desc: RegisterMissionListener
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(MissionManagerClientTest, RegisterMissionListener_0200, TestSize.Level1)
{
    EXPECT_TRUE(client_ != nullptr);
    std::string deviceId = "123";
    sptr<MockIRemoteMissionListener> listener = new (std::nothrow) MockIRemoteMissionListener();
    auto result = client_->RegisterMissionListener(deviceId, listener);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, INVALID_PARAMETERS_ERR);
    }
}

/**
 * @tc.name: MissionManagerClient_UnRegisterMissionListener_0200
 * @tc.desc: UnRegisterMissionListener
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(MissionManagerClientTest, UnRegisterMissionListener_0200, TestSize.Level1)
{
    EXPECT_TRUE(client_ != nullptr);
    std::string deviceId = "123";
    sptr<MockIRemoteMissionListener> listener = new (std::nothrow) MockIRemoteMissionListener();
    auto result = client_->UnRegisterMissionListener(deviceId, listener);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, INVALID_PARAMETERS_ERR);
    }
}

/**
 * @tc.name: MissionManagerClient_RegisterMissionListener_0300
 * @tc.desc: RegisterMissionListener with null remote listener
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(MissionManagerClientTest, RegisterMissionListener_0300, TestSize.Level1)
{
    EXPECT_TRUE(client_ != nullptr);
    std::string deviceId = "123";
    sptr<IRemoteMissionListener> listener = nullptr;
    auto result = client_->RegisterMissionListener(deviceId, listener);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, INVALID_PARAMETERS_ERR);
    }
}

/**
 * @tc.name: MissionManagerClient_UnRegisterMissionListener_0300
 * @tc.desc: UnRegisterMissionListener with null remote listener
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(MissionManagerClientTest, UnRegisterMissionListener_0300, TestSize.Level1)
{
    EXPECT_TRUE(client_ != nullptr);
    std::string deviceId = "123";
    sptr<IRemoteMissionListener> listener = nullptr;
    auto result = client_->UnRegisterMissionListener(deviceId, listener);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, INVALID_PARAMETERS_ERR);
    }
}

/**
 * @tc.name: MissionManagerClient_GetMissionInfos_0100
 * @tc.desc: GetMissionInfos
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(MissionManagerClientTest, GetMissionInfos_0100, TestSize.Level1)
{
    EXPECT_TRUE(client_ != nullptr);
    std::string deviceId = "123";
    std::vector<MissionInfo> missionInfos;
    auto result = client_->GetMissionInfos(deviceId, 10, missionInfos);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, ERR_INVALID_STATE);
    }
}

/**
 * @tc.name: MissionManagerClient_GetMissionInfo_0100
 * @tc.desc: GetMissionInfo
 * @tc.type: FUNC
 */
HWTEST_F(MissionManagerClientTest, GetMissionInfo_0100, TestSize.Level1)
{
    EXPECT_TRUE(client_ != nullptr);
    std::string deviceId = "123";
    int32_t missionId = 1;
    MissionInfo missionInfo;
    auto result = client_->GetMissionInfo(deviceId, missionId, missionInfo);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, ERR_INVALID_STATE);
    }
}


/**
 * @tc.name: MissionManagerClient_GetMissionSnapshot_0100
 * @tc.desc: GetMissionSnapshot
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(MissionManagerClientTest, GetMissionSnapshot_0100, TestSize.Level1)
{
    EXPECT_TRUE(client_ != nullptr);
    std::string deviceId = "123";
    MissionSnapshot snapshot;
    bool isLowResolution = false;
    auto result = client_->GetMissionSnapshot(deviceId, 10, snapshot, isLowResolution);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, ERR_INVALID_STATE);
    }
}

/**
 * @tc.name: MissionManagerClient_CleanMission_0100
 * @tc.desc: CleanMission
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(MissionManagerClientTest, CleanMission_0100, TestSize.Level1)
{
    EXPECT_TRUE(client_ != nullptr);
    auto result = client_->CleanMission(10);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, ERR_INVALID_STATE);
    }
}

/**
 * @tc.name: MissionManagerClient_CleanAllMissions_0100
 * @tc.desc: CleanAllMissions
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(MissionManagerClientTest, CleanAllMissions_0100, TestSize.Level1)
{
    EXPECT_TRUE(client_ != nullptr);
    auto result = client_->CleanAllMissions();
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, ERR_INVALID_STATE);
    }
}

/**
 * @tc.name: MissionManagerClient_MoveMissionToFront_0100
 * @tc.desc: MoveMissionToFront
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(MissionManagerClientTest, MoveMissionToFront_0100, TestSize.Level1)
{
    auto result = client_->MoveMissionToFront(10);
    EXPECT_EQ(result, ERR_INVALID_STATE);
}

/**
 * @tc.name: MissionManagerClient_MoveMissionToFront_0200
 * @tc.desc: MoveMissionToFront
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(MissionManagerClientTest, MoveMissionToFront_0200, TestSize.Level1)
{
    StartOptions startOptions;
    auto result = client_->MoveMissionToFront(1, startOptions);
    EXPECT_EQ(result, ERR_INVALID_STATE);
}

/**
 * @tc.name: MissionManagerClient_MoveMissionsToForeground_0100
 * @tc.desc: MoveMissionsToForeground
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(MissionManagerClientTest, MoveMissionsToForeground_0100, TestSize.Level1)
{
    EXPECT_TRUE(client_ != nullptr);
    auto result = client_->MoveMissionsToForeground({1, 2, 3}, 1);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, ERR_INVALID_STATE);
    }
}

/**
 * @tc.name: MissionManagerClient_MoveMissionsToForeground_0200
 * @tc.desc: MoveMissionsToForeground
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(MissionManagerClientTest, MoveMissionsToForeground_0200, TestSize.Level1)
{
    EXPECT_TRUE(client_ != nullptr);
    auto result = client_->MoveMissionsToForeground({1, 2, 3}, 1);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, ERR_INVALID_STATE);
    }
}

/**
 * @tc.name: MissionManagerClient_MoveMissionsToBackground_0100
 * @tc.desc: MoveMissionsToBackground
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(MissionManagerClientTest, MoveMissionsToBackground_0100, TestSize.Level1)
{
    EXPECT_TRUE(client_ != nullptr);
    std::vector<int32_t> result;
    auto ret = client_->MoveMissionsToBackground({1, 2, 3}, result);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(ret, ERR_INVALID_STATE);
    }
}

/**
 * @tc.name: MissionManagerClient_MoveMissionsToBackground_0200
 * @tc.desc: MoveMissionsToBackground
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(MissionManagerClientTest, MoveMissionsToBackground_0200, TestSize.Level1)
{
    EXPECT_TRUE(client_ != nullptr);
    std::vector<int32_t> result;
    auto ret = client_->MoveMissionsToBackground({1, 2, 3}, result);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(ret, ERR_INVALID_STATE);
    }
}

/**
 * @tc.name: MissionManagerClient_GetMissionIdByToken_0100
 * @tc.desc: GetMissionIdByToken
 * @tc.type: FUNC
 */
HWTEST_F(MissionManagerClientTest, GetMissionIdByToken_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetMissionIdByToken_0100 start";
    sptr<IRemoteObject> token = nullptr;
    int32_t missionId = 1;
    auto result = client_->GetMissionIdByToken(token, missionId);

    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "GetMissionIdByToken_0100 end";
}

/**
 * @tc.name: MissionManagerClient_StartSyncRemoteMissions_0100
 * @tc.desc: StartSyncRemoteMissions
 * @tc.type: FUNC
 */
HWTEST_F(MissionManagerClientTest, StartSyncRemoteMissions_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartSyncRemoteMissions_0100 start";
    std::string devId = BUNDLE_NAME;
    bool fixConflict = true;
    int64_t tag = 1;
    auto result = client_->StartSyncRemoteMissions(devId, fixConflict, tag);

    EXPECT_EQ(result, ERR_INVALID_STATE);
    GTEST_LOG_(INFO) << "StartSyncRemoteMissions_0100 end";
}

/**
 * @tc.name: MissionManagerClient_StopSyncRemoteMissions_0100
 * @tc.desc: StopSyncRemoteMissions
 * @tc.type: FUNC
 */
HWTEST_F(MissionManagerClientTest, StopSyncRemoteMissions_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopSyncRemoteMissions_0100 start";
    std::string devId = BUNDLE_NAME;
    auto result = client_->StopSyncRemoteMissions(devId);

    EXPECT_EQ(result, ERR_INVALID_STATE);
    GTEST_LOG_(INFO) << "StopSyncRemoteMissions_0100 end";
}


/**
 * @tc.name: MissionManagerClient_SetMissionContinueState_0100
 * @tc.desc: SetMissionContinueState
 * @tc.type: FUNC
 */
HWTEST_F(MissionManagerClientTest, SetMissionContinueState_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetMissionContinueState_0100 start";
    EXPECT_NE(client_, nullptr);
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> sessionToken = nullptr;
    AAFwk::ContinueState state = AAFwk::ContinueState::CONTINUESTATE_ACTIVE;
    auto result = client_->SetMissionContinueState(token, state, sessionToken);
    EXPECT_EQ(INVALID_PARAMETERS_ERR, result);
    GTEST_LOG_(INFO) << "SetMissionContinueState_0100 end";
}

/**
 * @tc.name: MissionManagerClient_SetMissionContinueState_0200
 * @tc.desc: SetMissionContinueState
 * @tc.type: FUNC
 */
HWTEST_F(MissionManagerClientTest, SetMissionContinueState_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetMissionContinueState_0200 start";
    EXPECT_NE(client_, nullptr);
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> sessionToken = nullptr;
    AAFwk::ContinueState state = AAFwk::ContinueState::CONTINUESTATE_INACTIVE;
    auto result = client_->SetMissionContinueState(token, state, sessionToken);
    EXPECT_EQ(INVALID_PARAMETERS_ERR, result);
    GTEST_LOG_(INFO) << "SetMissionContinueState_0200 end";
}

#ifdef SUPPORT_SCREEN
/**
 * @tc.name: MissionManagerClient_SetMissionLabel_0100
 * @tc.desc: SetMissionLabel
 * @tc.type: FUNC
 */
HWTEST_F(MissionManagerClientTest, MissionManagerClient_SetMissionLabel_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MissionManagerClient_SetMissionLabel_0100 start";
    sptr<IRemoteObject> token = nullptr;
    std::string label = "label";
    ErrCode ret = client_->SetMissionLabel(token, label);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    }
    EXPECT_TRUE(client_ != nullptr);
    GTEST_LOG_(INFO) << "MissionManagerClient_SetMissionLabel_0100 end";
}

/**
 * @tc.name: MissionManagerClient_SetMissionIcon_0100
 * @tc.desc: SetMissionIcon
 * @tc.type: FUNC
 * @tc.require: SR000GVIJQ
 */
HWTEST_F(MissionManagerClientTest, MissionManagerClient_SetMissionIcon_0100, TestSize.Level1)
{
    sptr<IRemoteObject> abilityToken = nullptr;
    std::shared_ptr<OHOS::Media::PixelMap> icon = nullptr;

    auto result = client_->SetMissionIcon(abilityToken, icon);
    EXPECT_NE(result, ERR_OK);
}

/**
 * @tc.number: UpdateMissionSnapShot_0100
 * @tc.name: UpdateMissionSnapShot
 * @tc.desc: The caller is expected to be dlp manager.
 */
HWTEST_F(MissionManagerClientTest, UpdateMissionSnapShot_0100, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    auto pixelMap = std::shared_ptr<Media::PixelMap>();
    client_->UpdateMissionSnapShot(token, pixelMap);
    EXPECT_TRUE(client_ != nullptr);
}
#endif

/**
 * @tc.name: MissionManagerClient_IsValidMissionIds_0100
 * @tc.desc: IsValidMissionIds
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(MissionManagerClientTest, IsValidMissionIds_0100, TestSize.Level1)
{
    EXPECT_TRUE(client_ != nullptr);
    std::vector<MissionValidResult> results;
    auto result = client_->IsValidMissionIds({1, 2, 3}, results);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, ERR_INVALID_STATE);
    }
}

/**
 * @tc.name: MissionManagerClient_PreStartMission_0100
 * @tc.desc: CallRequestDone
 * @tc.type: FUNC
 */
HWTEST_F(MissionManagerClientTest, PreStartMission_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PreStartMission_0100 start";
    EXPECT_TRUE(client_ != nullptr);
    std::string bundleName = "bundleName";
    std::string moduleName = "moduleName";
    std::string abilityName = "abilityName";
    std::string startTime = "startTime";
    ErrCode result = client_->PreStartMission(bundleName, moduleName, abilityName, startTime);
    EXPECT_EQ(result, ERR_INVALID_STATE);
    GTEST_LOG_(INFO) << "PreStartMission_0100 end";
}

/**
 * @tc.name: MissionManagerClient_TerminateMission_0100
 * @tc.desc: TerminateMission
 * @tc.type: FUNC
 */
HWTEST_F(MissionManagerClientTest, MissionManagerClient_TerminateMission_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MissionManagerClient_TerminateMission_0100 start";
    int32_t missionId = 1;
    ErrCode result = client_->TerminateMission(missionId);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, ERR_INVALID_STATE);
    }
    EXPECT_NE(client_, nullptr);
    GTEST_LOG_(INFO) << "MissionManagerClient_TerminateMission_0100 end";
}

/**
 * @tc.name: MissionManagerClient_Release_0100
 * @tc.desc: Release
 * @tc.type: FUNC
 */
HWTEST_F(MissionManagerClientTest, MissionManagerClient_Release_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MissionManagerClient_Release_0100 start";
    auto &client = MissionManagerClient::GetInstance();
    auto ret = client.Release();
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
    GTEST_LOG_(INFO) << "MissionManagerClient_Release_0100 end";
}
}
}