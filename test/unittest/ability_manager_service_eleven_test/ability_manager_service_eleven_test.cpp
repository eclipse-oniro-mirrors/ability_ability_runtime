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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ability_connect_manager.h"
#include "ui_extension_ability_manager.h"
#include "common_extension_manager.h"
#include "ability_connection.h"
#include "ability_manager_errors.h"
#include "ability_manager_service.h"
#include "ability_scheduler.h"
#include "app_utils.h"
#include "connection_observer_errors.h"
#include "mock_ability_connect_callback.h"
#include "mock_ability_controller.h"
#include "mock_ability_manager_collaborator.h"
#include "mock_ability_token.h"
#include "mock_app_mgr_service.h"
#include "mock_mission_list_manager_interface.h"
#include "mock_my_flag.h"
#include "mock_permission_verification.h"
#include "mock_scene_board_judgement.h"
#include "mock_ability_connect_callback.h"
#include "sender_info.h"
#include "session/host/include/session.h"
#include "ui_ability_lifecycle_manager.h"

#include "utils/window_options_utils.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using OHOS::AppExecFwk::AbilityType;
using OHOS::AppExecFwk::ExtensionAbilityType;
namespace OHOS {
namespace AAFwk {
class AbilityManagerServiceElevenTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    std::shared_ptr<AbilityRecord> MockAbilityRecord(AbilityType);
    sptr<Token> MockToken(AbilityType);
    sptr<SessionInfo> MockSessionInfo(int32_t persistentId);
};

class MockIRemoteOnListener : public IRemoteOnListener {
public:
    virtual ~MockIRemoteOnListener() {}
    void OnCallback(const OnCallbackInfo &info) override
    {}
    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }
};

class MockPendingWantRecord : public PendingWantRecord {
public:
    MockPendingWantRecord() : PendingWantRecord() {};
    MockPendingWantRecord(const std::shared_ptr<PendingWantManager> &pendingWantManager, int32_t uid,
        int32_t callerTokenId, const sptr<IRemoteObject> &callerToken, std::shared_ptr<PendingWantKey> key)
        : PendingWantRecord(pendingWantManager, uid, callerTokenId, callerToken, key) {};
    virtual ~MockPendingWantRecord() {};
    bool IsProxyObject() const override
    {
        return proxyObject_;
    }

    void SetProxyObject(bool proxyObject)
    {
        proxyObject_ = proxyObject;
    }

    std::shared_ptr<PendingWantKey> GetKey()
    {
        return key_;
    }

    void SetKey(std::shared_ptr<PendingWantKey> key)
    {
        key_ = key;
    }

private:
    bool proxyObject_ = false;
    std::shared_ptr<PendingWantKey> key_;
};

class MockIWantSender : public PendingWantRecord {
public:
    virtual ~MockIWantSender() {};

    sptr<IRemoteObject> AsObject() override
    {
        asObjectfunctionFrequency++;
        return iRemoteObjectFlags_;
    };
    void SetIRemoteObjectFlags(sptr<IRemoteObject> iRemoteObjectFlags)
    {
        iRemoteObjectFlags_ = iRemoteObjectFlags;
    };

    void Send(SenderInfo& senderInfo) override {};

    int32_t GetasObjectfunctionFrequency() const
    {
        return asObjectfunctionFrequency;
    }

private:
    sptr<IRemoteObject> iRemoteObjectFlags_ = nullptr;
    int32_t asObjectfunctionFrequency = 0;
};

#ifdef SUPPORT_SCREEN
class MockWMSHandler : public IWindowManagerServiceHandler {
public:
    void NotifyWindowTransition(
        sptr<AbilityTransitionInfo> fromInfo, sptr<AbilityTransitionInfo> toInfo, bool& animaEnabled) override
    {}

    int32_t GetFocusWindow(sptr<IRemoteObject>& abilityToken) override
    {
        return 0;
    }

    void StartingWindow(
        sptr<AbilityTransitionInfo> info, std::shared_ptr<Media::PixelMap> pixelMap, uint32_t bgColor) override
    {}

    void StartingWindow(sptr<AbilityTransitionInfo> info, std::shared_ptr<Media::PixelMap> pixelMap) override {}

    void CancelStartingWindow(sptr<IRemoteObject> abilityToken) override {}

    void NotifyAnimationAbilityDied(sptr<AbilityTransitionInfo> info) override {}

    int32_t MoveMissionsToForeground(const std::vector<int32_t>& missionIds, int32_t topMissionId) override
    {
        return 0;
    }

    int32_t MoveMissionsToBackground(const std::vector<int32_t>& missionIds, std::vector<int32_t>& result) override
    {
        if (missionIds.size() == 0 && result.size() == 0) {
            return 1;
        } else {
            return 0;
        }
    }

    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }
};
#endif

std::shared_ptr<AbilityRecord> AbilityManagerServiceElevenTest::MockAbilityRecord(AbilityType abilityType)
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.demo";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = abilityType;
    return AbilityRecord::CreateAbilityRecord(abilityRequest);
}

sptr<Token> AbilityManagerServiceElevenTest::MockToken(AbilityType abilityType)
{
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(abilityType);
    if (!abilityRecord) {
        return nullptr;
    }
    return abilityRecord->GetToken();
}

sptr<SessionInfo> AbilityManagerServiceElevenTest::MockSessionInfo(int32_t persistentId)
{
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    if (!sessionInfo) {
        TAG_LOGE(AAFwkTag::TEST, "sessionInfo is nullptr");
        return nullptr;
    }
    sessionInfo->persistentId = persistentId;
    return sessionInfo;
}

void AbilityManagerServiceElevenTest::SetUpTestCase() {}

void AbilityManagerServiceElevenTest::TearDownTestCase() {}

void AbilityManagerServiceElevenTest::SetUp() {}

void AbilityManagerServiceElevenTest::TearDown() {}

/*
 * Feature: RegisterOffListener_0001
 * Function: RegisterOffListener
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterOffListener
 */
HWTEST_F(AbilityManagerServiceElevenTest, RegisterOffListener_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterOffListener_0001 start";

    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    std::string type = "test";
    sptr<MockIRemoteOnListener> listener = new (std::nothrow) MockIRemoteOnListener();
    EXPECT_NE(listener, nullptr);
    MyFlag::flag_ = 0;
    auto result = abilityMs->RegisterOffListener(type, listener);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
    MyFlag::flag_ = 1;
    result = abilityMs->RegisterOffListener(type, listener);
    EXPECT_NE(result, CHECK_PERMISSION_FAILED);

    GTEST_LOG_(INFO) << "RegisterOffListener_0001 end";
}

/*
 * Feature: RegisterOnListener_0001
 * Function: RegisterOnListener
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterOnListener
 * EnvConditions: permission flag toggled
 * CaseDescription: Expect CHECK_PERMISSION_FAILED without SACall flag yet success otherwise
 */
HWTEST_F(AbilityManagerServiceElevenTest, RegisterOnListener_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterOnListener_0001 start";

    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    sptr<MockIRemoteOnListener> listener = new (std::nothrow) MockIRemoteOnListener();
    ASSERT_NE(listener, nullptr);
    MyFlag::flag_ = 0;
    auto result = abilityMs->RegisterOnListener("test", listener);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
    MyFlag::flag_ = 1;
    result = abilityMs->RegisterOnListener("test", listener);
    EXPECT_NE(result, CHECK_PERMISSION_FAILED);
    MyFlag::flag_ = 0;

    GTEST_LOG_(INFO) << "RegisterOnListener_0001 end";
}

/*
 * Feature: GetWantSender_0002
 * Function: GetWantSender
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetWantSender
 */
HWTEST_F(AbilityManagerServiceElevenTest, GetWantSender_0002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetWantSender_0002 start";

    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(abilityMs->subManagersHelper_, nullptr);
    abilityMs->subManagersHelper_->currentPendingWantManager_ = std::make_shared<PendingWantManager>(nullptr);
    EXPECT_NE(abilityMs->subManagersHelper_->currentPendingWantManager_, nullptr);

    WantSenderInfo wantSenderInfo;
    wantSenderInfo.userId = -1;
    int32_t uid = -1;
    MyFlag::flag_ = 0;
    EXPECT_EQ(wantSenderInfo.allWants.size(), 0);
    auto result = abilityMs->GetWantSender(wantSenderInfo, nullptr, uid);
    EXPECT_EQ(result, nullptr);

    AAFwk::Want want;
    AAFwk::Operation operation;
    std::string bundleName = "test";
    operation.SetBundleName(bundleName);
    want.SetOperation(operation);
    WantsInfo wantsInfo;
    wantsInfo.want = want;
    wantsInfo.resolvedTypes = want.GetType();
    wantSenderInfo.allWants.emplace_back(wantsInfo);
    EXPECT_EQ(wantSenderInfo.allWants.size(), 1);
    result = abilityMs->GetWantSender(wantSenderInfo, nullptr, uid);
    EXPECT_EQ(result, nullptr);

    MyFlag::flag_ = 1;
    wantSenderInfo.userId = 100;
    bundleName = "com.ohos.settings";
    operation.SetBundleName(bundleName);
    want.SetOperation(operation);
    wantsInfo.want = want;
    wantSenderInfo.allWants.clear();
    wantSenderInfo.allWants.emplace_back(wantsInfo);
    abilityMs->subManagersHelper_->pendingWantManagers_.emplace(
        wantSenderInfo.userId, std::make_shared<PendingWantManager>(nullptr));
    EXPECT_EQ(wantSenderInfo.allWants.size(), 1);
    result = abilityMs->GetWantSender(wantSenderInfo, nullptr, uid);
    EXPECT_NE(result, nullptr);

    GTEST_LOG_(INFO) << "GetWantSender_0002 end";
}

/*
 * Feature: GetWantSender_0003
 * Function: GetWantSender
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetWantSender
 */
HWTEST_F(AbilityManagerServiceElevenTest, GetWantSender_0003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetWantSender_0003 start";
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(abilityMs->subManagersHelper_, nullptr);

    MyFlag::flag_ = 1;
    WantSenderInfo wantSenderInfo;
    wantSenderInfo.userId = 100;
    AAFwk::Want want;
    AAFwk::Operation operation;
    std::string bundleName = "com.ohos.settings";
    operation.SetBundleName(bundleName);
    want.SetOperation(operation);
    WantsInfo wantsInfo;
    wantsInfo.want = want;
    wantsInfo.resolvedTypes = want.GetType();
    wantSenderInfo.allWants.clear();
    wantSenderInfo.allWants.emplace_back(wantsInfo);
    abilityMs->subManagersHelper_->pendingWantManagers_.emplace(
        wantSenderInfo.userId, std::make_shared<PendingWantManager>(nullptr));
    EXPECT_EQ(wantSenderInfo.allWants.size(), 1);
    auto result = abilityMs->GetWantSender(wantSenderInfo, nullptr, -1);
    EXPECT_NE(result, nullptr);
    GTEST_LOG_(INFO) << "GetWantSender_0003 end";
}

/*
 * Feature: GetWantSenderByUserId_0001
 * Function: GetWantSenderByUserId
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetWantSenderByUserId
 * EnvConditions: NA
 * CaseDescription: Test GetWantSenderByUserId when isSACall is true and userId is specified
 */
HWTEST_F(AbilityManagerServiceElevenTest, GetWantSenderByUserId_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetWantSenderByUserId_0001 start";
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    ASSERT_NE(abilityMs->subManagersHelper_, nullptr);
    // Mock SACall permission
    MyFlag::flag_ = 1;
    WantSenderInfo wantSenderInfo;
    wantSenderInfo.userId = 100;
    AAFwk::Want want;
    AAFwk::Operation operation;
    std::string bundleName = "com.ohos.settings";
    operation.SetBundleName(bundleName);
    want.SetOperation(operation);
    WantsInfo wantsInfo;
    wantsInfo.want = want;
    wantsInfo.resolvedTypes = want.GetType();
    wantSenderInfo.allWants.emplace_back(wantsInfo);

    int32_t callerUid = 20010080;
    int32_t callerUserId = 0;
    abilityMs->GetWantSenderByUserId(wantSenderInfo, nullptr, -1, callerUid, callerUserId);
    // Mock pending want manager
    abilityMs->subManagersHelper_->pendingWantManagers_.emplace(
        wantSenderInfo.userId, std::make_shared<PendingWantManager>(nullptr));
    auto result = abilityMs->GetWantSenderByUserId(wantSenderInfo, nullptr, callerUid, callerUid, callerUserId);
    EXPECT_NE(result, nullptr);
    GTEST_LOG_(INFO) << "GetWantSenderByUserId_0001 end";
}

/*
 * Feature: GetWantSenderByUserId_0002
 * Function: GetWantSenderByUserId
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetWantSenderByUserId
 * EnvConditions: NA
 * CaseDescription: Test GetWantSenderByUserId when isSACall is true and userId is not specified
 */
HWTEST_F(AbilityManagerServiceElevenTest, GetWantSenderByUserId_0002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetWantSenderByUserId_0002 start";
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    ASSERT_NE(abilityMs->subManagersHelper_, nullptr);
    MyFlag::flag_ = 1;

    WantSenderInfo wantSenderInfo;
    wantSenderInfo.userId = -1;
    AAFwk::Want want;
    AAFwk::Operation operation;
    std::string bundleName = "com.ohos.settings";
    operation.SetBundleName(bundleName);
    want.SetOperation(operation);
    WantsInfo wantsInfo;
    wantsInfo.want = want;
    wantsInfo.resolvedTypes = want.GetType();
    wantSenderInfo.allWants.emplace_back(wantsInfo);
    int32_t callerUid = 20010080;
    int32_t callerUserId = 0;
    abilityMs->subManagersHelper_->pendingWantManagers_.clear();
    auto result = abilityMs->GetWantSenderByUserId(wantSenderInfo, nullptr, callerUid, callerUid, callerUserId);
    EXPECT_EQ(result, nullptr);
    GTEST_LOG_(INFO) << "GetWantSenderByUserId_0002 end";
}

/*
 * Feature: SendLocalWantSender_0001
 * Function: SendLocalWantSender
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SendLocalWantSender
 * EnvConditions: caller lacks local wantagent permission
 * CaseDescription: Expect CHECK_PERMISSION_FAILED when triggering SendLocalWantSender without required permission
 */
HWTEST_F(AbilityManagerServiceElevenTest, SendLocalWantSender_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SendLocalWantSender_0001 start";
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    ASSERT_NE(abilityMs->subManagersHelper_, nullptr);
    abilityMs->subManagersHelper_->currentPendingWantManager_ = std::make_shared<PendingWantManager>(nullptr);
    ASSERT_NE(abilityMs->subManagersHelper_->currentPendingWantManager_, nullptr);

    SenderInfo senderInfo;
    senderInfo.tokenId = 1;
    MyFlag::flag_ = 0;
    auto result = abilityMs->SendLocalWantSender(senderInfo);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
    GTEST_LOG_(INFO) << "SendLocalWantSender_0001 end";
}

/*
 * Feature: GetWantSenderByUserId_0003
 * Function: GetWantSenderByUserId
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetWantSenderByUserId
 * EnvConditions: NA
 * CaseDescription: Test GetWantSenderByUserId when isSACall is true and userId is not specified
 */
HWTEST_F(AbilityManagerServiceElevenTest, GetWantSenderByUserId_0003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetWantSenderByUserId_0003 start";
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    ASSERT_NE(abilityMs->subManagersHelper_, nullptr);

    MyFlag::flag_ = 1;
    WantSenderInfo wantSenderInfo;
    wantSenderInfo.userId = -1;
    AAFwk::Want want;
    AAFwk::Operation operation;
    std::string bundleName = "com.ohos.settings";
    operation.SetBundleName(bundleName);
    want.SetOperation(operation);
    WantsInfo wantsInfo;
    wantsInfo.want = want;
    wantsInfo.resolvedTypes = want.GetType();
    wantSenderInfo.allWants.emplace_back(wantsInfo);
    int32_t uid = -1;
    int32_t callerUid = 20010080;
    int32_t callerUserId = 0;
    abilityMs->subManagersHelper_->pendingWantManagers_.clear();
    auto result = abilityMs->GetWantSenderByUserId(wantSenderInfo, nullptr, uid, callerUid, callerUserId);
    EXPECT_EQ(result, nullptr);
    GTEST_LOG_(INFO) << "GetWantSenderByUserId_0003 end";
}

/*
 * Feature: GetWantSenderByUserId_0004
 * Function: GetWantSenderByUserId
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetWantSenderByUserId
 * EnvConditions: NA
 * CaseDescription: Test GetWantSenderByUserId when isSystemApp is true and userId is specified
 */
HWTEST_F(AbilityManagerServiceElevenTest, GetWantSenderByUserId_0004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetWantSenderByUserId_0004 start";
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    ASSERT_NE(abilityMs->subManagersHelper_, nullptr);
    MyFlag::flag_ = 0;
    WantSenderInfo wantSenderInfo;
    wantSenderInfo.userId = 100;
    AAFwk::Want want;
    AAFwk::Operation operation;
    std::string bundleName = "com.ohos.testexample";
    operation.SetBundleName(bundleName);
    want.SetOperation(operation);
    WantsInfo wantsInfo;
    wantsInfo.want = want;
    wantsInfo.resolvedTypes = want.GetType();
    wantSenderInfo.allWants.emplace_back(wantsInfo);
    int32_t uid = -1;
    int32_t callerUid = 20010080;
    int32_t callerUserId = 100;
    AppExecFwk::MockAppMgrService::retCode_ = 400;
    abilityMs->subManagersHelper_->pendingWantManagers_.clear();
    auto result = abilityMs->GetWantSenderByUserId(wantSenderInfo, nullptr, uid, callerUid, callerUserId);
    EXPECT_EQ(result, nullptr);
    GTEST_LOG_(INFO) << "GetWantSenderByUserId_0004 end";
}

/*
 * Feature: CancelWantSender_0003
 * Function: CancelWantSender
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CancelWantSender
 */
HWTEST_F(AbilityManagerServiceElevenTest, CancelWantSender_0003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CancelWantSender_0003 start";

    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(abilityMs->subManagersHelper_, nullptr);
    abilityMs->subManagersHelper_->currentPendingWantManager_ = std::make_shared<PendingWantManager>(nullptr);
    EXPECT_NE(abilityMs->subManagersHelper_->currentPendingWantManager_, nullptr);

    sptr<MockIWantSender> sender = new (std::nothrow) MockIWantSender();
    EXPECT_NE(sender, nullptr);
    int32_t asObjectfunctionFrequency = 0;
    EXPECT_EQ(sender->GetasObjectfunctionFrequency(), asObjectfunctionFrequency);
    abilityMs->CancelWantSender(sender);
    asObjectfunctionFrequency = 1;
    EXPECT_EQ(sender->GetasObjectfunctionFrequency(), asObjectfunctionFrequency);

    sptr<MockPendingWantRecord> record = new (std::nothrow) MockPendingWantRecord();
    EXPECT_NE(record, nullptr);
    record->SetProxyObject(true);
    sender->SetIRemoteObjectFlags(record);
    abilityMs->CancelWantSender(sender);
    asObjectfunctionFrequency = 2;
    EXPECT_EQ(sender->GetasObjectfunctionFrequency(), asObjectfunctionFrequency);

    record->SetProxyObject(false);
    abilityMs->CancelWantSender(sender);
    asObjectfunctionFrequency = 4;
    EXPECT_EQ(sender->GetasObjectfunctionFrequency(), asObjectfunctionFrequency);

    GTEST_LOG_(INFO) << "CancelWantSender_0003 end";
}

/*
 * Feature: CancelWantSender_0004
 * Function: CancelWantSender
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CancelWantSender
 */
HWTEST_F(AbilityManagerServiceElevenTest, CancelWantSender_0004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CancelWantSender_0004 start";

    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    ASSERT_NE(abilityMs->subManagersHelper_, nullptr);
    abilityMs->subManagersHelper_->currentPendingWantManager_ = std::make_shared<PendingWantManager>(nullptr);
    ASSERT_NE(abilityMs->subManagersHelper_->currentPendingWantManager_, nullptr);

    sptr<MockIWantSender> sender = new (std::nothrow) MockIWantSender();
    ASSERT_NE(sender, nullptr);
    abilityMs->CancelWantSender(sender);

    sptr<MockPendingWantRecord> record = new (std::nothrow) MockPendingWantRecord();
    ASSERT_NE(record, nullptr);
    record->SetProxyObject(true);
    sender->SetIRemoteObjectFlags(record);
    abilityMs->CancelWantSender(sender);

    record->SetProxyObject(false);
    abilityMs->CancelWantSender(sender);
    int32_t asObjectfunctionFrequency = 4;
    EXPECT_EQ(sender->GetasObjectfunctionFrequency(), asObjectfunctionFrequency);

    int32_t userId = 100;
    std::shared_ptr<PendingWantKey> pendingKey = std::make_shared<PendingWantKey>();
    ASSERT_NE(pendingKey, nullptr);
    pendingKey->SetUserId(userId);
    sptr<MockPendingWantRecord> recordWithKey =
        new (std::nothrow) MockPendingWantRecord(nullptr, 0, 0, nullptr, pendingKey);
    sender->SetIRemoteObjectFlags(recordWithKey);
    abilityMs->CancelWantSender(sender);
    asObjectfunctionFrequency = 6;
    EXPECT_EQ(sender->GetasObjectfunctionFrequency(), asObjectfunctionFrequency);

    GTEST_LOG_(INFO) << "CancelWantSender_0004 end";
}

/*
 * Feature: CancelWantSenderByFlags_0004
 * Function: CancelWantSenderByFlags
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CancelWantSenderByFlags
 */
HWTEST_F(AbilityManagerServiceElevenTest, CancelWantSenderByFlags_0004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CancelWantSenderByFlags_0004 start";

    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(abilityMs->subManagersHelper_, nullptr);
    abilityMs->subManagersHelper_->currentPendingWantManager_ = std::make_shared<PendingWantManager>(nullptr);
    EXPECT_NE(abilityMs->subManagersHelper_->currentPendingWantManager_, nullptr);
    uint32_t flags = 1;

    sptr<MockIWantSender> sender = new (std::nothrow) MockIWantSender();
    EXPECT_NE(sender, nullptr);
    int32_t asObjectfunctionFrequency = 0;
    EXPECT_EQ(sender->GetasObjectfunctionFrequency(), asObjectfunctionFrequency);
    abilityMs->CancelWantSenderByFlags(sender, flags);
    asObjectfunctionFrequency = 1;
    EXPECT_EQ(sender->GetasObjectfunctionFrequency(), asObjectfunctionFrequency);

    sptr<MockPendingWantRecord> record = new (std::nothrow) MockPendingWantRecord();
    EXPECT_NE(record, nullptr);
    record->SetProxyObject(true);
    sender->SetIRemoteObjectFlags(record);
    abilityMs->CancelWantSenderByFlags(sender, flags);
    asObjectfunctionFrequency = 2;
    EXPECT_EQ(sender->GetasObjectfunctionFrequency(), asObjectfunctionFrequency);

    record->SetProxyObject(false);
    abilityMs->CancelWantSenderByFlags(sender, flags);
    asObjectfunctionFrequency = 4;
    EXPECT_EQ(sender->GetasObjectfunctionFrequency(), asObjectfunctionFrequency);

    GTEST_LOG_(INFO) << "CancelWantSenderByFlags_0004 end";
}

/*
 * Feature: GetPendingWantUid_0005
 * Function: GetPendingWantUid
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingWantUid
 */
HWTEST_F(AbilityManagerServiceElevenTest, GetPendingWantUid_0005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetPendingWantUid_0005 start";

    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(abilityMs->subManagersHelper_, nullptr);
    abilityMs->subManagersHelper_->currentPendingWantManager_ = std::make_shared<PendingWantManager>(nullptr);
    EXPECT_NE(abilityMs->subManagersHelper_->currentPendingWantManager_, nullptr);

    int32_t asObjectfunctionFrequency = 0;
    auto result = abilityMs->GetPendingWantUid(nullptr);
    EXPECT_EQ(result, -1);

    sptr<MockIWantSender> sender = new (std::nothrow) MockIWantSender();
    EXPECT_NE(sender, nullptr);
    asObjectfunctionFrequency = 1;
    result = abilityMs->GetPendingWantUid(sender);
    EXPECT_EQ(result, -1);
    EXPECT_EQ(sender->GetasObjectfunctionFrequency(), asObjectfunctionFrequency);

    GTEST_LOG_(INFO) << "GetPendingWantUid_0005 end";
}

/*
 * Feature: GetPendingWantUid_0006
 * Function: GetPendingWantUid
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingWantUid
 */
HWTEST_F(AbilityManagerServiceElevenTest, GetPendingWantUid_0006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetPendingWantUid_0006 start";

    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    ASSERT_NE(abilityMs->subManagersHelper_, nullptr);

    int32_t asObjectfunctionFrequency = 0;
    auto result = abilityMs->GetPendingWantUid(nullptr);
    EXPECT_EQ(result, -1);

    sptr<MockIWantSender> sender = new (std::nothrow) MockIWantSender();
    ASSERT_NE(sender, nullptr);
    asObjectfunctionFrequency = 1;
    result = abilityMs->GetPendingWantUid(sender);
    EXPECT_EQ(result, -1);
    EXPECT_EQ(sender->GetasObjectfunctionFrequency(), asObjectfunctionFrequency);

    sptr<MockPendingWantRecord> record = new (std::nothrow) MockPendingWantRecord();
    ASSERT_NE(record, nullptr);
    record->SetProxyObject(false);
    sender->SetIRemoteObjectFlags(record);
    abilityMs->GetPendingWantUid(sender);

    int32_t userId = 100;
    std::shared_ptr<PendingWantKey> pendingKey = std::make_shared<PendingWantKey>();
    ASSERT_NE(pendingKey, nullptr);
    pendingKey->SetUserId(userId);
    sptr<MockPendingWantRecord> recordWithKey =
        new (std::nothrow) MockPendingWantRecord(nullptr, 0, 0, nullptr, pendingKey);
    sender->SetIRemoteObjectFlags(recordWithKey);
    result = abilityMs->GetPendingWantUid(sender);
    EXPECT_EQ(result, -1);

    GTEST_LOG_(INFO) << "GetPendingWantUid_0006 end";
}

/*
 * Feature: GetPendingWantBundleName_0001
 * Function: GetPendingWantBundleName
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingWantBundleName
 */
HWTEST_F(AbilityManagerServiceElevenTest, GetPendingWantBundleName_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetPendingWantBundleName_0001 start";

    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    ASSERT_NE(abilityMs->subManagersHelper_, nullptr);

    int32_t asObjectfunctionFrequency = 0;
    auto bundleName = abilityMs->GetPendingWantBundleName(nullptr);
    EXPECT_TRUE(bundleName.empty());

    sptr<MockIWantSender> sender = new (std::nothrow) MockIWantSender();
    EXPECT_NE(sender, nullptr);
    asObjectfunctionFrequency = 1;
    bundleName = abilityMs->GetPendingWantBundleName(sender);
    EXPECT_TRUE(bundleName.empty());

    sptr<MockPendingWantRecord> record = new (std::nothrow) MockPendingWantRecord();
    ASSERT_NE(record, nullptr);
    record->SetProxyObject(false);
    sender->SetIRemoteObjectFlags(record);
    bundleName = abilityMs->GetPendingWantBundleName(sender);
    EXPECT_TRUE(bundleName.empty());

    int32_t userId = 100;
    std::shared_ptr<PendingWantKey> pendingKey = std::make_shared<PendingWantKey>();
    ASSERT_NE(pendingKey, nullptr);
    pendingKey->SetUserId(userId);
    sptr<MockPendingWantRecord> recordWithKey =
        new (std::nothrow) MockPendingWantRecord(nullptr, 0, 0, nullptr, pendingKey);
    sender->SetIRemoteObjectFlags(recordWithKey);
    bundleName = abilityMs->GetPendingWantBundleName(sender);
    EXPECT_TRUE(bundleName.empty());

    GTEST_LOG_(INFO) << "GetPendingWantBundleName_0001 end";
}

/*
 * Feature: GetPendingWantCode_0001
 * Function: GetPendingWantCode
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingWantCode
 */
HWTEST_F(AbilityManagerServiceElevenTest, GetPendingWantCode_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetPendingWantCode_0001 start";

    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    ASSERT_NE(abilityMs->subManagersHelper_, nullptr);

    int32_t asObjectfunctionFrequency = 0;
    auto result = abilityMs->GetPendingWantCode(nullptr);
    EXPECT_EQ(result, -1);

    sptr<MockIWantSender> sender = new (std::nothrow) MockIWantSender();
    ASSERT_NE(sender, nullptr);
    asObjectfunctionFrequency = 1;
    result = abilityMs->GetPendingWantCode(sender);
    EXPECT_EQ(result, -1);
    EXPECT_EQ(sender->GetasObjectfunctionFrequency(), asObjectfunctionFrequency);

    sptr<MockPendingWantRecord> record = new (std::nothrow) MockPendingWantRecord();
    ASSERT_NE(record, nullptr);
    record->SetProxyObject(false);
    sender->SetIRemoteObjectFlags(record);
    abilityMs->GetPendingWantCode(sender);

    std::shared_ptr<PendingWantKey> pendingKey = std::make_shared<PendingWantKey>();
    ASSERT_NE(pendingKey, nullptr);
    int32_t userId = 100;
    pendingKey->SetUserId(userId);
    sptr<MockPendingWantRecord> recordWithKey =
        new (std::nothrow) MockPendingWantRecord(nullptr, 0, 0, nullptr, pendingKey);
    sender->SetIRemoteObjectFlags(recordWithKey);
    result = abilityMs->GetPendingWantCode(sender);
    EXPECT_EQ(result, -1);

    GTEST_LOG_(INFO) << "GetPendingWantCode_0001 end";
}

/*
 * Feature: GetPendingWantType_0001
 * Function: GetPendingWantType
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingWantType
 */
HWTEST_F(AbilityManagerServiceElevenTest, GetPendingWantType_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetPendingWantType_0001 start";

    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    ASSERT_NE(abilityMs->subManagersHelper_, nullptr);

    int32_t asObjectfunctionFrequency = 0;
    auto result = abilityMs->GetPendingWantType(nullptr);
    EXPECT_EQ(result, -1);

    sptr<MockIWantSender> sender = new (std::nothrow) MockIWantSender();
    ASSERT_NE(sender, nullptr);
    asObjectfunctionFrequency = 1;
    result = abilityMs->GetPendingWantType(sender);
    EXPECT_EQ(result, -1);
    EXPECT_EQ(sender->GetasObjectfunctionFrequency(), asObjectfunctionFrequency);

    sptr<MockPendingWantRecord> record = new (std::nothrow) MockPendingWantRecord();
    ASSERT_NE(record, nullptr);
    record->SetProxyObject(false);
    sender->SetIRemoteObjectFlags(record);
    abilityMs->GetPendingWantType(sender);

    std::shared_ptr<PendingWantKey> pendingKey = std::make_shared<PendingWantKey>();
    ASSERT_NE(pendingKey, nullptr);
    int32_t userId = 100;
    pendingKey->SetUserId(userId);
    sptr<MockPendingWantRecord> recordWithKey =
        new (std::nothrow) MockPendingWantRecord(nullptr, 0, 0, nullptr, pendingKey);
    sender->SetIRemoteObjectFlags(recordWithKey);
    result = abilityMs->GetPendingWantType(sender);
    EXPECT_EQ(result, -1);

    GTEST_LOG_(INFO) << "GetPendingWantType_0001 end";
}

/*
 * Feature: GetPendingRequestWant_0001
 * Function: GetPendingRequestWant
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingRequestWant
 */
HWTEST_F(AbilityManagerServiceElevenTest, GetPendingRequestWant_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetPendingRequestWant_0001 start";

    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    ASSERT_NE(abilityMs->subManagersHelper_, nullptr);

    int32_t asObjectfunctionFrequency = 0;
    std::shared_ptr<Want> want;
    sptr<MockIWantSender> sender = new (std::nothrow) MockIWantSender();
    ASSERT_NE(sender, nullptr);
    auto result = abilityMs->GetPendingRequestWant(sender, want);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    sptr<MockPendingWantRecord> record = new (std::nothrow) MockPendingWantRecord();
    ASSERT_NE(record, nullptr);
    record->SetProxyObject(false);
    sender->SetIRemoteObjectFlags(record);
    abilityMs->GetPendingRequestWant(sender, want);

    std::shared_ptr<PendingWantKey> pendingKey = std::make_shared<PendingWantKey>();
    ASSERT_NE(pendingKey, nullptr);
    int32_t userId = 100;
    pendingKey->SetUserId(userId);
    sptr<MockPendingWantRecord> recordWithKey =
        new (std::nothrow) MockPendingWantRecord(nullptr, 0, 0, nullptr, pendingKey);
    sender->SetIRemoteObjectFlags(recordWithKey);
    abilityMs->GetPendingRequestWant(sender, want);
    result = abilityMs->GetPendingRequestWant(nullptr, want);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    GTEST_LOG_(INFO) << "GetPendingRequestWant_0001 end";
}

/*
 * Feature: GetPendingWantUserId_0006
 * Function: GetPendingWantUserId
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingWantUserId
 */
HWTEST_F(AbilityManagerServiceElevenTest, GetPendingWantUserId_0006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetPendingWantUserId_0006 start";

    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(abilityMs->subManagersHelper_, nullptr);
    abilityMs->subManagersHelper_->currentPendingWantManager_ = std::make_shared<PendingWantManager>(nullptr);
    EXPECT_NE(abilityMs->subManagersHelper_->currentPendingWantManager_, nullptr);

    int32_t asObjectfunctionFrequency = 0;
    auto result = abilityMs->GetPendingWantUserId(nullptr);
    EXPECT_EQ(result, -1);

    sptr<MockIWantSender> sender = new (std::nothrow) MockIWantSender();
    EXPECT_NE(sender, nullptr);
    asObjectfunctionFrequency = 1;
    result = abilityMs->GetPendingWantUserId(sender);
    EXPECT_EQ(result, -1);
    EXPECT_EQ(sender->GetasObjectfunctionFrequency(), asObjectfunctionFrequency);

    GTEST_LOG_(INFO) << "GetPendingWantUserId_0006 end";
}

/*
 * Feature: MoveMissionsToBackground_0007
 * Function: MoveMissionsToBackground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MoveMissionsToBackground
 */
HWTEST_F(AbilityManagerServiceElevenTest, MoveMissionsToBackground_0007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveMissionsToBackground_0007 start";

    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);

    std::vector<int32_t> missionIds;
    std::vector<int32_t> result;
    MyFlag::flag_ = 0;
    auto ret = abilityMs->MoveMissionsToBackground(missionIds, result);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);

    MyFlag::flag_ = 1;
    ret = abilityMs->MoveMissionsToBackground(missionIds, result);
    EXPECT_EQ(ret, ERR_NO_INIT);

#ifdef SUPPORT_SCREEN
    sptr<MockWMSHandler> wmsHandler = new (std::nothrow) MockWMSHandler();
    EXPECT_NE(wmsHandler, nullptr);
    abilityMs->wmsHandler_ = wmsHandler;
    MyFlag::flag_ = 1;
    ret = abilityMs->MoveMissionsToBackground(missionIds, result);
    EXPECT_NE(ret, 0);

    missionIds.emplace_back(1);
    result.emplace_back(1);
    ret = abilityMs->MoveMissionsToBackground(missionIds, result);
    EXPECT_EQ(ret, 0);
#endif // SUPPORT_SCREEN

    GTEST_LOG_(INFO) << "MoveMissionsToBackground_0007 end";
}

/*
 * Feature: GetMissionIdByToken_0008
 * Function: GetMissionIdByToken
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetMissionIdByToken
 */
HWTEST_F(AbilityManagerServiceElevenTest, GetMissionIdByToken_0008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetMissionIdByToken_0008 start";

    EXPECT_CALL(Rosen::SceneBoardJudgement::GetInstance(), MockIsSceneBoardEnabled())
        .Times(AnyNumber())
        .WillRepeatedly(Return(false));

    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    auto result = abilityMs->GetMissionIdByToken(nullptr);
    EXPECT_NE(result, 0);

    auto token = MockToken(AbilityType::PAGE);
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    EXPECT_NE(abilityRecord, nullptr);
    token->abilityRecord_ = abilityRecord;
    auto abilityRecordTest = Token::GetAbilityRecordByToken(token);
    EXPECT_NE(abilityRecordTest, nullptr);
    abilityRecordTest->abilityInfo_.applicationInfo.accessTokenId = IPCSkeleton::GetCallingTokenID();
    bool resultBranch = abilityMs->JudgeSelfCalled(abilityRecordTest);
    EXPECT_TRUE(resultBranch);
    resultBranch = abilityMs->CheckCallerIsDmsProcess();
    EXPECT_FALSE(resultBranch);
    abilityMs->subManagersHelper_ = nullptr;
    result = abilityMs->GetMissionIdByToken(token);
    EXPECT_NE(result, 0);

    GTEST_LOG_(INFO) << "GetMissionIdByToken_0008 end";
}

/*
 * Feature: AttachAbilityThread_0009
 * Function: AttachAbilityThread
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AttachAbilityThread
 */
HWTEST_F(AbilityManagerServiceElevenTest, AttachAbilityThread_0009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AttachAbilityThread_0009 start";

    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    sptr<IAbilityScheduler> scheduler = new (std::nothrow) AbilityScheduler();
    EXPECT_NE(scheduler, nullptr);
    auto token = MockToken(AbilityType::PAGE);
    EXPECT_NE(token, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    EXPECT_NE(abilityRecord, nullptr);
    token->abilityRecord_ = abilityRecord;

    // IsSceneBoardEnabled() is true
    EXPECT_CALL(Rosen::SceneBoardJudgement::GetInstance(), MockIsSceneBoardEnabled())
        .Times(AnyNumber())
        .WillRepeatedly(Return(true));
    {
        auto result = abilityMs->AttachAbilityThread(nullptr, nullptr);
        EXPECT_EQ(result, ERR_INVALID_VALUE);

        result = abilityMs->AttachAbilityThread(scheduler, nullptr);
        EXPECT_EQ(result, ERR_INVALID_VALUE);

        auto abilityRecordTest = Token::GetAbilityRecordByToken(token);
        EXPECT_NE(abilityRecordTest, nullptr);
        abilityRecordTest->abilityInfo_.applicationInfo.accessTokenId = IPCSkeleton::GetCallingTokenID();
        abilityRecordTest->abilityInfo_.applicationInfo.uid = 0;
        bool resultBranch = abilityMs->JudgeSelfCalled(abilityRecord);
        EXPECT_TRUE(resultBranch);
        abilityMs->timeoutMap_.emplace("INITIAL", "test");
        abilityRecordTest->abilityInfo_.name = "test";
        resultBranch = abilityMs->IsNeedTimeoutForTest(
            abilityRecordTest->GetAbilityInfo().name, AbilityRecord::ConvertAbilityState(AbilityState::INITIAL));
        EXPECT_TRUE(resultBranch);
        result = abilityMs->AttachAbilityThread(scheduler, token);
        EXPECT_EQ(result, 0);

        abilityMs->timeoutMap_.clear();
        abilityMs->subManagersHelper_ = nullptr;
        abilityRecordTest->abilityInfo_.type = AppExecFwk::AbilityType::SERVICE;
        abilityRecordTest->abilityInfo_.applicationInfo.uid = 0;
        result = abilityMs->AttachAbilityThread(scheduler, token);
        EXPECT_EQ(result, ERR_INVALID_VALUE);
    }

    GTEST_LOG_(INFO) << "AttachAbilityThread_0009 end";
}

/*
 * Feature: AttachAbilityThread_0010
 * Function: AttachAbilityThread
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AttachAbilityThread
 */
HWTEST_F(AbilityManagerServiceElevenTest, AttachAbilityThread_0010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AttachAbilityThread_0010 start";

    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    sptr<IAbilityScheduler> scheduler = new (std::nothrow) AbilityScheduler();
    EXPECT_NE(scheduler, nullptr);
    auto token = MockToken(AbilityType::PAGE);
    EXPECT_NE(token, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    EXPECT_NE(abilityRecord, nullptr);
    token->abilityRecord_ = abilityRecord;

    auto abilityRecordTest = Token::GetAbilityRecordByToken(token);
    EXPECT_NE(abilityRecordTest, nullptr);
    abilityRecordTest->abilityInfo_.applicationInfo.accessTokenId = IPCSkeleton::GetCallingTokenID();
    abilityRecordTest->abilityInfo_.applicationInfo.uid = 0;
    bool resultBranch = abilityMs->JudgeSelfCalled(abilityRecord);
    EXPECT_TRUE(resultBranch);

    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(abilityMs->subManagersHelper_, nullptr);

    EXPECT_CALL(Rosen::SceneBoardJudgement::GetInstance(), MockIsSceneBoardEnabled())
        .Times(AnyNumber())
        .WillRepeatedly(Return(true));
    {
        abilityRecordTest->abilityInfo_.type = AppExecFwk::AbilityType::EXTENSION;
        abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
        EXPECT_NE(abilityMs->subManagersHelper_, nullptr);
        std::shared_ptr<CommonExtensionManager> connectManager = std::make_shared<CommonExtensionManager>(0);
        EXPECT_NE(connectManager, nullptr);
        abilityMs->subManagersHelper_->commonExtensionManagers_.emplace(0, connectManager);
        auto result = abilityMs->AttachAbilityThread(scheduler, token);
        EXPECT_NE(result, 0);

        abilityRecordTest->abilityInfo_.type = AppExecFwk::AbilityType::DATA;
        result = abilityMs->AttachAbilityThread(scheduler, token);
        EXPECT_EQ(result, ERR_INVALID_VALUE);

        std::shared_ptr<DataAbilityManager> dataAbilityManager = std::make_shared<DataAbilityManager>();
        EXPECT_NE(dataAbilityManager, nullptr);
        abilityMs->subManagersHelper_->dataAbilityManagers_.emplace(0, dataAbilityManager);
        result = abilityMs->AttachAbilityThread(scheduler, token);
        EXPECT_NE(result, 0);
    }

    GTEST_LOG_(INFO) << "AttachAbilityThread_0010 end";
}

/*
 * Feature: AttachAbilityThread_0011
 * Function: AttachAbilityThread
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AttachAbilityThread
 */
HWTEST_F(AbilityManagerServiceElevenTest, AttachAbilityThread_0011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AttachAbilityThread_0011 start";

    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    sptr<IAbilityScheduler> scheduler = new (std::nothrow) AbilityScheduler();
    EXPECT_NE(scheduler, nullptr);
    auto token = MockToken(AbilityType::PAGE);
    EXPECT_NE(token, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    EXPECT_NE(abilityRecord, nullptr);
    token->abilityRecord_ = abilityRecord;
    auto abilityRecordTest = Token::GetAbilityRecordByToken(token);
    EXPECT_NE(abilityRecordTest, nullptr);

    abilityRecordTest->abilityInfo_.applicationInfo.accessTokenId = IPCSkeleton::GetCallingTokenID();
    abilityRecordTest->abilityInfo_.applicationInfo.uid = 0;
    bool resultBranch = abilityMs->JudgeSelfCalled(abilityRecord);
    EXPECT_TRUE(resultBranch);

    abilityRecordTest->abilityInfo_.type = AppExecFwk::AbilityType::PAGE;
    abilityRecordTest->SetOwnerMissionUserId(0);
    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);

    EXPECT_CALL(Rosen::SceneBoardJudgement::GetInstance(), MockIsSceneBoardEnabled())
        .Times(AnyNumber())
        .WillRepeatedly(Return(true));
    {
        auto result = abilityMs->AttachAbilityThread(scheduler, token);
        EXPECT_NE(result, 0);

        std::shared_ptr<UIAbilityLifecycleManager> uiAbilityManager = std::make_shared<UIAbilityLifecycleManager>(0);
        abilityMs->subManagersHelper_->uiAbilityManagers_.emplace(0, uiAbilityManager);
        result = abilityMs->AttachAbilityThread(scheduler, token);
        EXPECT_NE(result, 0);
    }

    GTEST_LOG_(INFO) << "AttachAbilityThread_0011 end";
}

/*
 * Feature: AttachAbilityThread_0012
 * Function: AttachAbilityThread
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AttachAbilityThread
 */
HWTEST_F(AbilityManagerServiceElevenTest, AttachAbilityThread_0012, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AttachAbilityThread_0012 start";

    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    sptr<IAbilityScheduler> scheduler = new (std::nothrow) AbilityScheduler();
    EXPECT_NE(scheduler, nullptr);
    auto token = MockToken(AbilityType::PAGE);
    EXPECT_NE(token, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    EXPECT_NE(abilityRecord, nullptr);
    token->abilityRecord_ = abilityRecord;
    auto abilityRecordTest = Token::GetAbilityRecordByToken(token);
    EXPECT_NE(abilityRecordTest, nullptr);

    abilityRecordTest->abilityInfo_.applicationInfo.accessTokenId = IPCSkeleton::GetCallingTokenID();
    abilityRecordTest->abilityInfo_.applicationInfo.uid = 0;
    bool resultBranch = abilityMs->JudgeSelfCalled(abilityRecord);
    EXPECT_TRUE(resultBranch);

    abilityRecordTest->abilityInfo_.type = AppExecFwk::AbilityType::PAGE;
    abilityRecordTest->SetOwnerMissionUserId(0);
    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);

    EXPECT_CALL(Rosen::SceneBoardJudgement::GetInstance(), MockIsSceneBoardEnabled())
        .Times(AnyNumber())
        .WillRepeatedly(Return(false));
    {
        abilityMs->subManagersHelper_->missionListManagers_.clear();
        AbilityRequest req;
        std::shared_ptr<DataAbilityRecord> dataAbilityRecord = std::make_shared<DataAbilityRecord>(req);
        EXPECT_NE(dataAbilityRecord, nullptr);
        dataAbilityRecord->ability_ = abilityRecord;
        std::map<std::string, std::shared_ptr<DataAbilityRecord>> dataAbilityRecordsLoaded;
        dataAbilityRecordsLoaded.emplace("test", dataAbilityRecord);
        std::shared_ptr<DataAbilityManager> dataAbilityManager = std::make_shared<DataAbilityManager>();
        abilityMs->subManagersHelper_->dataAbilityManagers_.emplace(0, dataAbilityManager);
        abilityMs->subManagersHelper_->dataAbilityManagers_[0]->dataAbilityRecordsLoaded_ = dataAbilityRecordsLoaded;
        resultBranch = abilityMs->VerificationAllToken(token);
        EXPECT_TRUE(resultBranch);
        auto result = abilityMs->AttachAbilityThread(scheduler, token);
        EXPECT_NE(result, 0);

        std::shared_ptr<MockMissionListManagerInterface> missionListManager =
            std::make_shared<MockMissionListManagerInterface>();
        abilityMs->subManagersHelper_->missionListManagers_.emplace(0, missionListManager);
        result = abilityMs->AttachAbilityThread(scheduler, token);
        EXPECT_EQ(result, 0);
    }

    GTEST_LOG_(INFO) << "AttachAbilityThread_0012 end";
}

/*
 * Feature: AbilityManagerService
 * Name: ReleaseCall_001
 * Function: ReleaseCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReleaseCall
 */
HWTEST_F(AbilityManagerServiceElevenTest, ReleaseCall_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceElevenTest ReleaseCall_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    AppExecFwk::ElementName validElement("device", "com.example.demo", "MainAbility");
    int32_t retCode = abilityMs->ReleaseCall(nullptr, validElement);
    EXPECT_EQ(retCode, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceElevenTest ReleaseCall_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: ReleaseCall_002
 * Function: ReleaseCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReleaseCall
 */
HWTEST_F(AbilityManagerServiceElevenTest, ReleaseCall_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceElevenTest ReleaseCall_002 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);

    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs->subManagersHelper_->missionListManagers_.clear();

    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    AppExecFwk::ElementName emptyElement;
    EXPECT_CALL(Rosen::SceneBoardJudgement::GetInstance(), MockIsSceneBoardEnabled())
        .Times(AnyNumber())
        .WillRepeatedly(Return(true));
    int32_t retCode = abilityMs->ReleaseCall(connect, emptyElement);
    EXPECT_EQ(retCode, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceElevenTest ReleaseCall_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: ReleaseCall_003
 * Function: ReleaseCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReleaseCall
 */
HWTEST_F(AbilityManagerServiceElevenTest, ReleaseCall_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceElevenTest ReleaseCall_003 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);

    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs->subManagersHelper_->missionListManagers_.clear();

    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    AppExecFwk::ElementName emptyElement;
    EXPECT_CALL(Rosen::SceneBoardJudgement::GetInstance(), MockIsSceneBoardEnabled())
        .Times(AnyNumber())
        .WillRepeatedly(Return(false));
    int32_t retCode = abilityMs->ReleaseCall(connect, emptyElement);
    EXPECT_EQ(retCode, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceElevenTest ReleaseCall_003 end");
}

/*
 * Feature: AbilityManagerService
 * Name: CheckStartCallHasFloatingWindowForUIExtension_001
 * Function: CheckStartCallHasFloatingWindowForUIExtension
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckStartCallHasFloatingWindowForUIExtension
 */
HWTEST_F(AbilityManagerServiceElevenTest, CheckStartCallHasFloatingWindowForUIExtension_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceElevenTest CheckStartCallHasFloatingWindowForUIExtension_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);

    sptr<IRemoteObject> token = nullptr;
    EXPECT_CALL(Rosen::SceneBoardJudgement::GetInstance(), MockIsSceneBoardEnabled())
        .Times(AnyNumber())
        .WillRepeatedly(Return(false));
    int result = abilityMs->CheckStartCallHasFloatingWindowForUIExtension(token);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceElevenTest CheckStartCallHasFloatingWindowForUIExtension_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: CheckStartCallHasFloatingWindowForUIExtension_002
 * Function: CheckStartCallHasFloatingWindowForUIExtension
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckStartCallHasFloatingWindowForUIExtension
 */
HWTEST_F(AbilityManagerServiceElevenTest, CheckStartCallHasFloatingWindowForUIExtension_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceElevenTest CheckStartCallHasFloatingWindowForUIExtension_002 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);

    sptr<IRemoteObject> token = nullptr;
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        int result = abilityMs->CheckStartCallHasFloatingWindowForUIExtension(token);
        EXPECT_TRUE(result);
    }
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceElevenTest CheckStartCallHasFloatingWindowForUIExtension_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: CheckStartCallHasFloatingWindowForUIExtension_003
 * Function: CheckStartCallHasFloatingWindowForUIExtension
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckStartCallHasFloatingWindowForUIExtension
 */
HWTEST_F(AbilityManagerServiceElevenTest, CheckStartCallHasFloatingWindowForUIExtension_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceElevenTest CheckStartCallHasFloatingWindowForUIExtension_003 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);

    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        int result = abilityMs->CheckStartCallHasFloatingWindowForUIExtension(token);
        EXPECT_TRUE(result);
    }
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceElevenTest CheckStartCallHasFloatingWindowForUIExtension_003 end");
}

/*
 * Feature: AbilityManagerService
 * Name: CheckUIExtensionCallerIsUIAbility_001
 * Function: CheckUIExtensionCallerIsUIAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckUIExtensionCallerIsUIAbility
 */
HWTEST_F(AbilityManagerServiceElevenTest, CheckUIExtensionCallerIsUIAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceElevenTest CheckUIExtensionCallerIsUIAbility_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.demo";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    abilityRequest.callerToken = nullptr;
    int result = abilityMs->CheckUIExtensionCallerIsUIAbility(abilityRequest);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceElevenTest CheckUIExtensionCallerIsUIAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: SetBackgroundCall_001
 * Function: SetBackgroundCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetBackgroundCall
 */
HWTEST_F(AbilityManagerServiceElevenTest, SetBackgroundCall_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceElevenTest SetBackgroundCall_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    RunningProcessInfo info;
    AbilityRequest abilityRequest;
    bool isBackgroundCall = false;

    abilityMs->backgroundJudgeFlag_ = true;
    int result = abilityMs->SetBackgroundCall(info, abilityRequest, isBackgroundCall);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceElevenTest SetBackgroundCall_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: SetBackgroundCall_002
 * Function: SetBackgroundCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetBackgroundCall
 */
HWTEST_F(AbilityManagerServiceElevenTest, SetBackgroundCall_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceElevenTest SetBackgroundCall_002 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    RunningProcessInfo info;
    AbilityRequest abilityRequest;
    bool isBackgroundCall = false;

    abilityMs->backgroundJudgeFlag_ = false;
    int result = abilityMs->SetBackgroundCall(info, abilityRequest, isBackgroundCall);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceElevenTest SetBackgroundCall_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: SetResidentProcessEnabled_001
 * Function: SetResidentProcessEnabled
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetResidentProcessEnabled
 */
HWTEST_F(AbilityManagerServiceElevenTest, SetResidentProcessEnabled_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceElevenTest SetResidentProcessEnabled_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    std::string bundleName = "";
    bool enable = false;
    int result = abilityMs->SetResidentProcessEnabled(bundleName, enable);
    EXPECT_EQ(result, INNER_ERR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceElevenTest SetResidentProcessEnabled_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: GetUidByCloneBundleInfo_001
 * Function: GetUidByCloneBundleInfo
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetUidByCloneBundleInfo
 */
HWTEST_F(AbilityManagerServiceElevenTest, GetUidByCloneBundleInfo_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetUidByCloneBundleInfo_001 start";
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(abilityMs->subManagersHelper_, nullptr);

    std::string bundleName = "com.ohos.settings";
    int uid = -1;
    int userId = 100;
    AppExecFwk::MockAppMgrService::retCode_ = uid;
    int appIndex = 0;
    abilityMs->GetUidByCloneBundleInfo(bundleName, uid, userId, appIndex);

    bundleName = "com.applicaion.test";
    userId = 101;
    auto result = abilityMs->GetUidByCloneBundleInfo(bundleName, uid, userId, appIndex);
    EXPECT_EQ(result, uid);

    GTEST_LOG_(INFO) << "GetUidByCloneBundleInfo_001 end";
}
} // namespace AAFwk
} // namespace OHOS
