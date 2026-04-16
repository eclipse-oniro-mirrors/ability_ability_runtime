/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "bundlemgr/mock_bundle_manager.h"
#include "mock_native_token.h"
#include "ability_manager_errors.h"
#define private public
#define protected public
#include "ability_event_handler.h"
#undef private
#undef protected
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#define private public
#define protected public
#include "pending_want_record.h"
#include "pending_want_manager.h"
#undef private
#undef protected
#include "sa_mgr_client.h"
#include "sender_info.h"
#include "system_ability_definition.h"
#include "wants_info.h"
#include "want_receiver_stub.h"
#include "want_sender_stub.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using OHOS::AppExecFwk::ElementName;

namespace OHOS {
namespace AAFwk {

namespace {
inline void Sleep(int64_t milli)
{
    std::this_thread::sleep_for(std::chrono::seconds(milli));
}
}  // namespace
class PendingWantManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    WantSenderInfo MakeWantSenderInfo(Want& want, int32_t flags, int32_t userId, int32_t type = 1);
    WantSenderInfo MakeWantSenderInfo(std::vector<Want>& wants, int32_t flags, int32_t userId, int32_t type = 1);
    std::shared_ptr<PendingWantKey> MakeWantKey(WantSenderInfo& wantSenderInfo);
    static constexpr int DEFAULT_COUNT = 100;
    static constexpr int TEST_WAIT_TIME = 100000;

    class CancelReceiver : public AAFwk::WantReceiverStub {
    public:
        static int performReceiveCount;
        static int sendCount;
        void Send(const int32_t resultCode) override;
        void PerformReceive(const AAFwk::Want& want, int resultCode, const std::string& data,
            const AAFwk::WantParams& extras, bool serialized, bool sticky, int sendingUser) override;
        virtual sptr<IRemoteObject> AsObject() override
        {
            return nullptr;
        }
    };

public:
    std::shared_ptr<PendingWantManager> pendingManager_{ nullptr };
    bool isSystemApp = false;
};

int PendingWantManagerTest::CancelReceiver::performReceiveCount = 0;
int PendingWantManagerTest::CancelReceiver::sendCount = 0;

void PendingWantManagerTest::CancelReceiver::Send(const int32_t resultCode)
{
    sendCount = DEFAULT_COUNT;
}
void PendingWantManagerTest::CancelReceiver::PerformReceive(const AAFwk::Want& want, int resultCode,
    const std::string& data, const AAFwk::WantParams& extras, bool serialized, bool sticky, int sendingUser)
{
    performReceiveCount = DEFAULT_COUNT;
}

void PendingWantManagerTest::SetUpTestCase()
{
    MockNativeToken::SetNativeToken();
    OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->RegisterSystemAbility(
        OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, new BundleMgrService());
}

void PendingWantManagerTest::TearDownTestCase()
{
    OHOS::DelayedSingleton<SaMgrClient>::DestroyInstance();
}

void PendingWantManagerTest::SetUp()
{
}

void PendingWantManagerTest::TearDown()
{
}

WantSenderInfo PendingWantManagerTest::MakeWantSenderInfo(Want& want, int32_t flags, int32_t userId, int32_t type)
{
    WantSenderInfo wantSenderInfo;
    wantSenderInfo.type = type;
    // wantSenderInfo.type is OperationType::START_ABILITY
    wantSenderInfo.bundleName = "com.ix.hiRadio";
    wantSenderInfo.resultWho = "RadioTopAbility";
    int requestCode = 10;
    wantSenderInfo.requestCode = requestCode;
    std::vector<WantsInfo> allWant;
    WantsInfo wantInfo;
    wantInfo.want = want;
    wantInfo.resolvedTypes = "nihao";
    allWant.emplace_back(wantInfo);
    wantSenderInfo.allWants = allWant;
    wantSenderInfo.flags = flags;
    wantSenderInfo.userId = userId;
    return wantSenderInfo;
}

WantSenderInfo PendingWantManagerTest::MakeWantSenderInfo(std::vector<Want>& wants,
    int32_t flags, int32_t userId, int32_t type)
{
    WantSenderInfo wantSenderInfo;
    wantSenderInfo.type = type;
    // wantSenderInfo.type is OperationType::START_ABILITY
    wantSenderInfo.bundleName = "com.ix.hiRadio";
    wantSenderInfo.resultWho = "RadioTopAbility";
    int requestCode = 10;
    wantSenderInfo.requestCode = requestCode;
    std::vector<WantsInfo> allWant;
    for (auto want : wants) {
        WantsInfo wantsInfo;
        wantsInfo.want = want;
        wantsInfo.resolvedTypes = "";
        wantSenderInfo.allWants.push_back(wantsInfo);
    }
    wantSenderInfo.flags = flags;
    wantSenderInfo.userId = userId;
    return wantSenderInfo;
}


std::shared_ptr<PendingWantKey> PendingWantManagerTest::MakeWantKey(WantSenderInfo& wantSenderInfo)
{
    std::shared_ptr<PendingWantKey> pendingKey = std::make_shared<PendingWantKey>();
    pendingKey->SetBundleName(wantSenderInfo.bundleName);
    pendingKey->SetRequestWho(wantSenderInfo.resultWho);
    pendingKey->SetRequestCode(wantSenderInfo.requestCode);
    pendingKey->SetFlags(wantSenderInfo.flags);
    pendingKey->SetUserId(wantSenderInfo.userId);
    pendingKey->SetType(wantSenderInfo.type);
    if (wantSenderInfo.allWants.size() > 0) {
        pendingKey->SetRequestWant(wantSenderInfo.allWants.back().want);
        pendingKey->SetRequestResolvedType(wantSenderInfo.allWants.back().resolvedTypes);
        pendingKey->SetAllWantsInfos(wantSenderInfo.allWants);
    }
    return pendingKey;
}

/*
 * @tc.number    : PendingWantManagerTest_0100
 * @tc.name      : PendingWantManager GetWantSender
 * @tc.desc      : 1.GetWantSender function callingUid = 0
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_0100, TestSize.Level1)
{
    int32_t callingUid = 0;
    int32_t uid = 0;
    WantSenderInfo wantSenderInfo;
    wantSenderInfo.flags = 0;
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    EXPECT_NE(pendingManager_->GetWantSender(callingUid, uid, isSystemApp, wantSenderInfo, nullptr), nullptr);
}

/*
 * @tc.number    : PendingWantManagerTest_0300
 * @tc.name      : PendingWantManager GetWantSender
 * @tc.desc      : 1.GetWantSender function callingUid = 1 && callingUid != uid
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_0300, TestSize.Level1)
{
    int32_t callingUid = 1;
    int32_t uid = 0;
    WantSenderInfo wantSenderInfo;
    wantSenderInfo.flags = 0;
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    EXPECT_NE(pendingManager_->GetWantSender(callingUid, uid, isSystemApp, wantSenderInfo, nullptr), nullptr);
}

/*
 * @tc.number    : PendingWantManagerTest_0400
 * @tc.name      : PendingWantManager GetWantSender
 * @tc.desc      : 1.GetWantSender function callingUid = 1 && callingUid == uid
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_0400, TestSize.Level1)
{
    int32_t callingUid = 1;
    int32_t uid = 1;
    WantSenderInfo wantSenderInfo;
    wantSenderInfo.flags = 0;
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    EXPECT_NE(pendingManager_->GetWantSender(callingUid, uid, isSystemApp, wantSenderInfo, nullptr), nullptr);
}

/*
 * @tc.number    : PendingWantManagerTest_0500
 * @tc.name      : PendingWantManager GetWantSenderLocked
 * @tc.desc      : 1.GetWantSenderLocked Flags::NO_BUILD_FLAG
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_0500, TestSize.Level1)
{
    Want want;
    ElementName element("device", "bundleName", "abilityName");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, static_cast<int32_t>(Flags::NO_BUILD_FLAG), 0);
    EXPECT_TRUE(((unsigned int)wantSenderInfo.flags & (unsigned int)Flags::NO_BUILD_FLAG) != 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    EXPECT_EQ(pendingManager_->GetWantSenderLocked(1, 1, wantSenderInfo.userId, wantSenderInfo, nullptr), nullptr);
}

/*
 * @tc.number    : PendingWantManagerTest_0600
 * @tc.name      : PendingWantManager GetWantSenderLocked
 * @tc.desc      : 1.GetWantSenderLocked  Flags::ONE_TIME_FLAG ,Creat PendingWantRecord
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_0600, TestSize.Level1)
{
    Want want;
    ElementName element("device", "bundleName", "abilityName");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    EXPECT_FALSE(((unsigned int)wantSenderInfo.flags & (unsigned int)Flags::NO_BUILD_FLAG) != 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    EXPECT_NE(pendingManager_->GetWantSenderLocked(1, 1, wantSenderInfo.userId, wantSenderInfo, nullptr), nullptr);
}

/*
 * @tc.number    : PendingWantManagerTest_0700
 * @tc.name      : PendingWantManager GetWantSenderLocked
 * @tc.desc      : 1.GetWantSenderLocked Flags::NO_BUILD_FLAG ,cancel PendingWantRecord
 *                 2.CANCEL_PRESENT_FLAG means delet existing wantagent before create a new one
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_0700, TestSize.Level1)
{
    Want want;
    ElementName element("device", "bundleName", "abilityName");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    EXPECT_FALSE(((unsigned int)wantSenderInfo.flags & (unsigned int)Flags::NO_BUILD_FLAG) != 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    EXPECT_NE(pendingManager_->GetWantSenderLocked(1, 1, wantSenderInfo.userId, wantSenderInfo, nullptr), nullptr);
    EXPECT_EQ((int)pendingManager_->wantRecords_.size(), 1);
    WantSenderInfo wantSenderInfo1 = MakeWantSenderInfo(want, static_cast<int32_t>(Flags::CANCEL_PRESENT_FLAG), 0);
    EXPECT_TRUE(((unsigned int)wantSenderInfo1.flags & (unsigned int)Flags::CANCEL_PRESENT_FLAG) != 0);
    EXPECT_NE(pendingManager_->GetWantSenderLocked(1, 1, wantSenderInfo1.userId, wantSenderInfo1, nullptr), nullptr);
    EXPECT_EQ((int)pendingManager_->wantRecords_.size(), 1);
}

/*
 * @tc.number    : PendingWantManagerTest_0800
 * @tc.name      : PendingWantManager GetWantSenderLocked
 * @tc.desc      : 1.GetWantSenderLocked,Flags::UPDATE_PRESENT_FLAG ,update PendingWantRecord
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_0800, TestSize.Level1)
{
    Want want;
    ElementName element("device", "bundleName", "abilityName");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    EXPECT_FALSE(((unsigned int)wantSenderInfo.flags & (unsigned int)Flags::NO_BUILD_FLAG) != 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    EXPECT_NE(pendingManager_->GetWantSenderLocked(1, 1, wantSenderInfo.userId, wantSenderInfo, nullptr), nullptr);
    EXPECT_EQ((int)pendingManager_->wantRecords_.size(), 1);
    WantSenderInfo wantSenderInfo1 = MakeWantSenderInfo(want, static_cast<int32_t>(Flags::UPDATE_PRESENT_FLAG), 0);
    EXPECT_TRUE(((unsigned int)wantSenderInfo1.flags & (unsigned int)Flags::UPDATE_PRESENT_FLAG) != 0);
    EXPECT_NE(pendingManager_->GetWantSenderLocked(1, 1, wantSenderInfo1.userId, wantSenderInfo1, nullptr), nullptr);
    EXPECT_EQ((int)pendingManager_->wantRecords_.size(), 1);
}

/*
 * @tc.number    : PendingWantManagerTest_0900
 * @tc.name      : PendingWantManager GetPendingWantRecordByKey
 * @tc.desc      : 1.GetPendingWantRecordByKey,wantSenderInfo contain want info
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_0900, TestSize.Level1)
{
    Want want;
    ElementName element("device", "bundleName", "abilityName");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    EXPECT_FALSE(((unsigned int)wantSenderInfo.flags & (unsigned int)Flags::NO_BUILD_FLAG) != 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    EXPECT_NE(pendingManager_->GetWantSenderLocked(1, 1, wantSenderInfo.userId, wantSenderInfo, nullptr), nullptr);
    EXPECT_EQ((int)pendingManager_->wantRecords_.size(), 1);
    std::shared_ptr<PendingWantKey> pendingKey = std::make_shared<PendingWantKey>();
    pendingKey->SetBundleName(wantSenderInfo.bundleName);
    pendingKey->SetRequestWho(wantSenderInfo.resultWho);
    pendingKey->SetRequestCode(wantSenderInfo.requestCode);
    pendingKey->SetFlags(wantSenderInfo.flags);
    pendingKey->SetUserId(wantSenderInfo.userId);
    pendingKey->SetType(wantSenderInfo.type);
    if (wantSenderInfo.allWants.size() > 0) {
        pendingKey->SetRequestWant(wantSenderInfo.allWants.back().want);
        pendingKey->SetRequestResolvedType(wantSenderInfo.allWants.back().resolvedTypes);
        pendingKey->SetAllWantsInfos(wantSenderInfo.allWants);
    }
    EXPECT_NE(pendingManager_->GetPendingWantRecordByKey(pendingKey), nullptr);
}

/*
 * @tc.number    : PendingWantManagerTest_1000
 * @tc.name      : PendingWantManager GetPendingWantRecordByKey
 * @tc.desc      : 1.GetPendingWantRecordByKey,wantSenderInfo not contain want info
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_1000, TestSize.Level1)
{
    Want want;
    ElementName element("device", "bundleName", "abilityName");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    wantSenderInfo.allWants.clear();
    EXPECT_FALSE(((unsigned int)wantSenderInfo.flags & (unsigned int)Flags::NO_BUILD_FLAG) != 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    EXPECT_NE(pendingManager_->GetWantSenderLocked(1, 1, wantSenderInfo.userId, wantSenderInfo, nullptr), nullptr);
    EXPECT_EQ((int)pendingManager_->wantRecords_.size(), 1);
    std::shared_ptr<PendingWantKey> pendingKey = std::make_shared<PendingWantKey>();
    pendingKey->SetBundleName(wantSenderInfo.bundleName);
    pendingKey->SetRequestWho(wantSenderInfo.resultWho);
    pendingKey->SetRequestCode(wantSenderInfo.requestCode);
    pendingKey->SetFlags(wantSenderInfo.flags);
    pendingKey->SetUserId(wantSenderInfo.userId);
    pendingKey->SetType(wantSenderInfo.type);
    if (wantSenderInfo.allWants.size() > 0) {
        pendingKey->SetRequestWant(wantSenderInfo.allWants.back().want);
        pendingKey->SetRequestResolvedType(wantSenderInfo.allWants.back().resolvedTypes);
        pendingKey->SetAllWantsInfos(wantSenderInfo.allWants);
    }
    EXPECT_NE(pendingManager_->GetPendingWantRecordByKey(pendingKey), nullptr);
}

/*
 * @tc.number    : PendingWantManagerTest_1100
 * @tc.name      : PendingWantManager GetPendingWantRecordByKey
 * @tc.desc      : 1.GetPendingWantRecordByKey,pendingKey info different wantSenderInfo
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_1100, TestSize.Level1)
{
    Want want;
    ElementName element("device", "bundleName", "abilityName");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    std::shared_ptr<PendingWantKey> pendingKey = std::make_shared<PendingWantKey>();
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    pendingKey->SetBundleName(wantSenderInfo.bundleName + "A");
    pendingKey->SetRequestWho(wantSenderInfo.resultWho);
    pendingKey->SetRequestCode(wantSenderInfo.requestCode);
    pendingKey->SetFlags(wantSenderInfo.flags);
    pendingKey->SetUserId(wantSenderInfo.userId);
    pendingKey->SetType(wantSenderInfo.type);
    if (wantSenderInfo.allWants.size() > 0) {
        pendingKey->SetRequestWant(wantSenderInfo.allWants.back().want);
        pendingKey->SetRequestResolvedType(wantSenderInfo.allWants.back().resolvedTypes);
        pendingKey->SetAllWantsInfos(wantSenderInfo.allWants);
    }
    EXPECT_EQ(pendingManager_->GetPendingWantRecordByKey(pendingKey), nullptr);
}

/*
 * @tc.number    : PendingWantManagerTest_1200
 * @tc.name      : PendingWantManager CheckPendingWantRecordByKey
 * @tc.desc      : 1.CheckPendingWantRecordByKey,pendingKey and pendingKey1 is the same
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_1200, TestSize.Level1)
{
    Want want;
    ElementName element("device", "bundleName", "abilityName");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    std::shared_ptr<PendingWantKey> pendingKey = std::make_shared<PendingWantKey>();
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    pendingKey->SetBundleName(wantSenderInfo.bundleName);
    pendingKey->SetRequestWho(wantSenderInfo.resultWho);
    pendingKey->SetRequestCode(wantSenderInfo.requestCode);
    pendingKey->SetFlags(wantSenderInfo.flags);
    pendingKey->SetUserId(wantSenderInfo.userId);
    pendingKey->SetType(wantSenderInfo.type);
    if (wantSenderInfo.allWants.size() > 0) {
        pendingKey->SetRequestWant(wantSenderInfo.allWants.back().want);
        pendingKey->SetRequestResolvedType(wantSenderInfo.allWants.back().resolvedTypes);
        pendingKey->SetAllWantsInfos(wantSenderInfo.allWants);
    }
    std::shared_ptr<PendingWantKey> pendingKey1 = std::make_shared<PendingWantKey>();
    pendingKey1->SetBundleName(wantSenderInfo.bundleName);
    pendingKey1->SetRequestWho(wantSenderInfo.resultWho);
    pendingKey1->SetRequestCode(wantSenderInfo.requestCode);
    pendingKey1->SetFlags(wantSenderInfo.flags);
    pendingKey1->SetUserId(wantSenderInfo.userId);
    pendingKey1->SetType(wantSenderInfo.type);
    if (wantSenderInfo.allWants.size() > 0) {
        pendingKey1->SetRequestWant(wantSenderInfo.allWants.back().want);
        pendingKey1->SetRequestResolvedType(wantSenderInfo.allWants.back().resolvedTypes);
        pendingKey1->SetAllWantsInfos(wantSenderInfo.allWants);
    }
    EXPECT_EQ(pendingManager_->CheckPendingWantRecordByKey(pendingKey, pendingKey1), true);
}

/*
 * @tc.number    : PendingWantManagerTest_1300
 * @tc.name      : PendingWantManager CheckPendingWantRecordByKey
 * @tc.desc      : 1.CheckPendingWantRecordByKey,pendingKey and pendingKey1 is the different
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_1300, TestSize.Level1)
{
    Want want;
    ElementName element("device", "bundleName", "abilityName");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    WantSenderInfo wantSenderInfo1 = MakeWantSenderInfo(want, 0, 1);
    pendingManager_ = std::make_shared<PendingWantManager>();
    std::shared_ptr<PendingWantKey> pendingKey = std::make_shared<PendingWantKey>();
    EXPECT_NE(pendingManager_, nullptr);
    pendingKey->SetBundleName(wantSenderInfo.bundleName);
    pendingKey->SetRequestWho(wantSenderInfo.resultWho);
    pendingKey->SetRequestCode(wantSenderInfo.requestCode);
    pendingKey->SetFlags(wantSenderInfo.flags);
    pendingKey->SetUserId(wantSenderInfo.userId);
    pendingKey->SetType(wantSenderInfo.type);
    if (wantSenderInfo.allWants.size() > 0) {
        pendingKey->SetRequestWant(wantSenderInfo.allWants.back().want);
        pendingKey->SetRequestResolvedType(wantSenderInfo.allWants.back().resolvedTypes);
        pendingKey->SetAllWantsInfos(wantSenderInfo.allWants);
    }
    std::shared_ptr<PendingWantKey> pendingKey1 = std::make_shared<PendingWantKey>();
    pendingKey1->SetBundleName(wantSenderInfo1.bundleName);
    pendingKey1->SetRequestWho(wantSenderInfo1.resultWho);
    pendingKey1->SetRequestCode(wantSenderInfo1.requestCode);
    pendingKey1->SetFlags(wantSenderInfo1.flags);
    pendingKey1->SetUserId(wantSenderInfo1.userId);
    pendingKey1->SetType(wantSenderInfo1.type);
    if (wantSenderInfo1.allWants.size() > 0) {
        pendingKey1->SetRequestWant(wantSenderInfo1.allWants.back().want);
        pendingKey1->SetRequestResolvedType(wantSenderInfo1.allWants.back().resolvedTypes);
        pendingKey1->SetAllWantsInfos(wantSenderInfo1.allWants);
    }
    EXPECT_EQ(pendingManager_->CheckPendingWantRecordByKey(pendingKey, pendingKey1), false);
}

/*
 * @tc.number    : PendingWantManagerTest_1400
 * @tc.name      : PendingWantManager SendWantSender
 * @tc.desc      : 1.SendWantSender,call the function(PerformReceive) of the callback class
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_1400, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo =
        MakeWantSenderInfo(want, (int32_t)Flags::CONSTANT_FLAG, 0, (int32_t)OperationType::SEND_COMMON_EVENT);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    sptr<PendingWantRecord> pendingWantRecord = new PendingWantRecord(pendingManager_, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    SenderInfo info;
    info.finishedReceiver = new CancelReceiver();
    EXPECT_EQ(pendingManager_->SendWantSender(pendingWantRecord, info), ERR_INVALID_VALUE);
    EXPECT_TRUE(CancelReceiver::performReceiveCount == 100);
    CancelReceiver::performReceiveCount = 0;
}

/*
 * @tc.number    : PendingWantManagerTest_1500
 * @tc.name      : PendingWantManager GetPendingWantRecordByCode
 * @tc.desc      : 1.GetPendingWantRecordByCode,parameter not nullptr
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_1500, TestSize.Level1)
{
    Want want;
    ElementName element("device", "bundleName", "abilityName");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    wantSenderInfo.allWants.clear();
    EXPECT_FALSE(((unsigned int)wantSenderInfo.flags & (unsigned int)Flags::NO_BUILD_FLAG) != 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    auto pendingRecord = pendingManager_->GetWantSenderLocked(1, 1, wantSenderInfo.userId, wantSenderInfo, nullptr);
    EXPECT_NE(pendingRecord, nullptr);
    EXPECT_EQ((int)pendingManager_->wantRecords_.size(), 1);
    EXPECT_NE(pendingManager_->GetPendingWantRecordByCode(pendingManager_->GetPendingWantCode(pendingRecord)), nullptr);
}

/*
 * @tc.number    : PendingWantManagerTest_1600
 * @tc.name      : PendingWantManager GetPendingWantRecordByCode
 * @tc.desc      : 1.GetPendingWantRecordByCode,no pendingWantRecord
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_1600, TestSize.Level1)
{
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    EXPECT_EQ((int)pendingManager_->wantRecords_.size(), 0);
    EXPECT_EQ(pendingManager_->GetPendingWantRecordByCode(1), nullptr);
}

/*
 * @tc.number    : PendingWantManagerTest_1700
 * @tc.name      : PendingWantManager GetPendingWantRecordByCode
 * @tc.desc      : 1.GetPendingWantRecordByCode,parameter not nullptr
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_1700, TestSize.Level1)
{
    Want want;
    ElementName element("device", "bundleName", "abilityName");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    wantSenderInfo.allWants.clear();
    EXPECT_FALSE(((unsigned int)wantSenderInfo.flags & (unsigned int)Flags::NO_BUILD_FLAG) != 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    EXPECT_NE(pendingManager_->GetWantSenderLocked(1, 1, wantSenderInfo.userId, wantSenderInfo, nullptr), nullptr);
    EXPECT_EQ((int)pendingManager_->wantRecords_.size(), 1);
    EXPECT_EQ(pendingManager_->GetPendingWantRecordByCode(100), nullptr);
}

/*
 * @tc.number    : PendingWantManagerTest_1800
 * @tc.name      : PendingWantManager GetPendingWantUid
 * @tc.desc      : 1.GetPendingWantUid, parameter nullptr
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_1800, TestSize.Level1)
{
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    EXPECT_EQ(pendingManager_->GetPendingWantUid(nullptr), -1);
}

/*
 * @tc.number    : PendingWantManagerTest_1900
 * @tc.name      : PendingWantManager GetPendingWantUid
 * @tc.desc      : 1.GetPendingWantUid, parameter not nullptr
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_1900, TestSize.Level1)
{
    Want want;
    ElementName element("device", "bundleName", "abilityName");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    EXPECT_FALSE(((unsigned int)wantSenderInfo.flags & (unsigned int)Flags::NO_BUILD_FLAG) != 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    auto pendingRecord = pendingManager_->GetWantSenderLocked(1, 1, wantSenderInfo.userId, wantSenderInfo, nullptr);
    EXPECT_NE(pendingRecord, nullptr);
    EXPECT_EQ(pendingManager_->GetPendingWantUid(pendingRecord), 1);
}

/*
 * @tc.number    : PendingWantManagerTest_2000
 * @tc.name      : PendingWantManager GetPendingWantUserId
 * @tc.desc      : 1.GetPendingWantUserId, parameter nullptr
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_2000, TestSize.Level1)
{
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    EXPECT_EQ(pendingManager_->GetPendingWantUserId(nullptr), -1);
}

/*
 * @tc.number    : PendingWantManagerTest_2100
 * @tc.name      : PendingWantManager GetPendingWantUserId
 * @tc.desc      : 1.GetPendingWantUserId,parameter not nullptr
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_2100, TestSize.Level1)
{
    Want want;
    ElementName element("device", "bundleName", "abilityName");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    EXPECT_FALSE(((unsigned int)wantSenderInfo.flags & (unsigned int)Flags::NO_BUILD_FLAG) != 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    auto pendingRecord = pendingManager_->GetWantSenderLocked(1, 1, wantSenderInfo.userId, wantSenderInfo, nullptr);
    EXPECT_NE(pendingRecord, nullptr);
    EXPECT_EQ(pendingManager_->GetPendingWantUserId(pendingRecord), 0);
}

/*
 * @tc.number    : PendingWantManagerTest_2200
 * @tc.name      : PendingWantManager GetPendingWantBundleName
 * @tc.desc      : 1.GetPendingWantBundleName, parameter nullptr
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_2200, TestSize.Level1)
{
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    EXPECT_EQ(pendingManager_->GetPendingWantBundleName(nullptr), "");
}

/*
 * @tc.number    : PendingWantManagerTest_2300
 * @tc.name      : PendingWantManager GetPendingWantBundleName
 * @tc.desc      : 1.GetPendingWantBundleName, parameter not nullptr
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_2300, TestSize.Level1)
{
    Want want;
    ElementName element("device", "bundleName", "abilityName");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    EXPECT_FALSE(((unsigned int)wantSenderInfo.flags & (unsigned int)Flags::NO_BUILD_FLAG) != 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    auto pendingRecord = pendingManager_->GetWantSenderLocked(1, 1, wantSenderInfo.userId, wantSenderInfo, nullptr);
    EXPECT_NE(pendingRecord, nullptr);
    EXPECT_EQ(pendingManager_->GetPendingWantBundleName(pendingRecord), "com.ix.hiRadio");
}

/*
 * @tc.number    : PendingWantManagerTest_2400
 * @tc.name      : PendingWantManager GetPendingWantCode
 * @tc.desc      : 1.GetPendingWantCode, parameter nullptr
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_2400, TestSize.Level1)
{
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    EXPECT_EQ(pendingManager_->GetPendingWantCode(nullptr), -1);
}

/*
 * @tc.number    : PendingWantManagerTest_2500
 * @tc.name      : PendingWantManager GetPendingWantCode
 * @tc.desc      : 1.GetPendingWantCode, parameter not nullptr
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_2500, TestSize.Level1)
{
    Want want;
    ElementName element("device", "bundleName", "abilityName");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    EXPECT_FALSE(((unsigned int)wantSenderInfo.flags & (unsigned int)Flags::NO_BUILD_FLAG) != 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    auto pendingRecord = pendingManager_->GetWantSenderLocked(1, 1, wantSenderInfo.userId, wantSenderInfo, nullptr);
    EXPECT_NE(pendingRecord, nullptr);
    EXPECT_TRUE(pendingManager_->GetPendingWantCode(pendingRecord) != -1);
}

/*
 * @tc.number    : PendingWantManagerTest_2600
 * @tc.name      : PendingWantManager GetPendingWantType
 * @tc.desc      : 1.GetPendingWantType, parameter nullptr
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_2600, TestSize.Level1)
{
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    EXPECT_EQ(pendingManager_->GetPendingWantType(nullptr), -1);
}

/*
 * @tc.number    : PendingWantManagerTest_2700
 * @tc.name      : PendingWantManager GetPendingWantType
 * @tc.desc      : 1.GetPendingWantType, parameter not nullptr
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_2700, TestSize.Level1)
{
    Want want;
    ElementName element("device", "bundleName", "abilityName");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    EXPECT_FALSE(((unsigned int)wantSenderInfo.flags & (unsigned int)Flags::NO_BUILD_FLAG) != 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    auto pendingRecord = pendingManager_->GetWantSenderLocked(1, 1, wantSenderInfo.userId, wantSenderInfo, nullptr);
    EXPECT_NE(pendingRecord, nullptr);
    EXPECT_TRUE(pendingManager_->GetPendingWantType(pendingRecord) != -1);
}

/*
 * @tc.number    : PendingWantManagerTest_2800
 * @tc.name      : PendingWantManager RegisterCancelListener
 * @tc.desc      : 1.RegisterCancelListener,Canceled == false
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_2800, TestSize.Level1)
{
    sptr<CancelReceiver> cance = new CancelReceiver();
    Want want;
    ElementName element("device", "bundleName", "abilityName");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    EXPECT_FALSE(((unsigned int)wantSenderInfo.flags & (unsigned int)Flags::NO_BUILD_FLAG) != 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    auto pendingRecord = iface_cast<PendingWantRecord>(
        pendingManager_->GetWantSenderLocked(1, 1, wantSenderInfo.userId, wantSenderInfo, nullptr)->AsObject());
    EXPECT_NE(pendingRecord, nullptr);
    pendingManager_->RegisterCancelListener(pendingRecord, cance);
    EXPECT_EQ(static_cast<int>(pendingRecord->GetCancelCallbacks().size()), 1);
}

/*
 * @tc.number    : PendingWantManagerTest_2900
 * @tc.name      : PendingWantManager RegisterCancelListener
 * @tc.desc      : 1.RegisterCancelListener,Canceled == true
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_2900, TestSize.Level1)
{
    sptr<CancelReceiver> cance = new CancelReceiver();
    Want want;
    ElementName element("device", "bundleName", "abilityName");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    EXPECT_FALSE(((unsigned int)wantSenderInfo.flags & (unsigned int)Flags::NO_BUILD_FLAG) != 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    auto pendingRecord = iface_cast<PendingWantRecord>(
        pendingManager_->GetWantSenderLocked(1, 1, wantSenderInfo.userId, wantSenderInfo, nullptr)->AsObject());
    EXPECT_NE(pendingRecord, nullptr);
    pendingRecord->SetCanceled();
    pendingManager_->RegisterCancelListener(pendingRecord, cance);
    EXPECT_EQ(static_cast<int>(pendingRecord->GetCancelCallbacks().size()), 0);
}

/*
 * @tc.number    : PendingWantManagerTest_3000
 * @tc.name      : PendingWantManager UnregisterCancelListener
 * @tc.desc      : 1.UnregisterCancelListener
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_3000, TestSize.Level1)
{
    sptr<CancelReceiver> cance = new CancelReceiver();
    Want want;
    ElementName element("device", "bundleName", "abilityName");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    EXPECT_FALSE(((unsigned int)wantSenderInfo.flags & (unsigned int)Flags::NO_BUILD_FLAG) != 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    auto pendingRecord = iface_cast<PendingWantRecord>(
        pendingManager_->GetWantSenderLocked(1, 1, wantSenderInfo.userId, wantSenderInfo, nullptr)->AsObject());
    EXPECT_NE(pendingRecord, nullptr);
    pendingManager_->RegisterCancelListener(pendingRecord, cance);
    EXPECT_EQ(static_cast<int>(pendingRecord->GetCancelCallbacks().size()), 1);
    pendingManager_->UnregisterCancelListener(pendingRecord, cance);
    EXPECT_EQ(static_cast<int>(pendingRecord->GetCancelCallbacks().size()), 0);
}

/*
 * @tc.number    : PendingWantManagerTest_3100
 * @tc.name      : PendingWantManager CancelWantSender
 * @tc.desc      : 1.CancelWantSender,,call the function(Send) of the callback class
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_3100, TestSize.Level1)
{
    sptr<CancelReceiver> cance = new CancelReceiver();
    Want want;
    ElementName element("device", "bundleName", "abilityName");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    EXPECT_FALSE(((unsigned int)wantSenderInfo.flags & (unsigned int)Flags::NO_BUILD_FLAG) != 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    auto pendingRecord = iface_cast<PendingWantRecord>(
        pendingManager_->GetWantSenderLocked(1, 1, wantSenderInfo.userId, wantSenderInfo, nullptr)->AsObject());
    EXPECT_NE(pendingRecord, nullptr);
    pendingManager_->RegisterCancelListener(pendingRecord, cance);
    bool isSystemApp = false;
    pendingManager_->CancelWantSender(isSystemApp, pendingRecord);
    EXPECT_TRUE(CancelReceiver::sendCount == 100);
    EXPECT_TRUE((int)pendingManager_->wantRecords_.size() == 0);
}

/*
 * @tc.number    : PendingWantManagerTest_3200
 * @tc.name      : PendingWantManager GetPendingRequestWant
 * @tc.desc      : 1.GetPendingRequestWant, Get pendingRecord want object.
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_3200, TestSize.Level1)
{
    Want want;
    ElementName element("device", "bundleName", "abilityName");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    EXPECT_FALSE(((unsigned int)wantSenderInfo.flags & (unsigned int)Flags::NO_BUILD_FLAG) != 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    auto pendingRecord = iface_cast<PendingWantRecord>(
        pendingManager_->GetWantSenderLocked(1, 1, wantSenderInfo.userId, wantSenderInfo, nullptr)->AsObject());
    EXPECT_NE(pendingRecord, nullptr);
    std::shared_ptr<Want> getWantInfo = std::make_shared<Want>();
    EXPECT_EQ(pendingManager_->GetPendingRequestWant(pendingRecord, getWantInfo), NO_ERROR);
    EXPECT_EQ(getWantInfo->GetElement().GetBundleName(), "bundleName");
    EXPECT_EQ(getWantInfo->GetElement().GetAbilityName(), "abilityName");
}

/*
 * @tc.number    : PendingWantManagerTest_3300
 * @tc.name      : PendingWantManager GetPendingRequestWant
 * @tc.desc      : 1.GetPendingRequestWant, Get pendingRecord want object,pendingRecord is nullptr.
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_3300, TestSize.Level1)
{
    Want want;
    ElementName element("device", "bundleName", "abilityName");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    EXPECT_FALSE(((unsigned int)wantSenderInfo.flags & (unsigned int)Flags::NO_BUILD_FLAG) != 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    auto pendingRecord = iface_cast<PendingWantRecord>(
        pendingManager_->GetWantSenderLocked(1, 1, wantSenderInfo.userId, wantSenderInfo, nullptr)->AsObject());
    EXPECT_NE(pendingRecord, nullptr);
    std::shared_ptr<Want> getWantInfo = std::make_shared<Want>();
    EXPECT_EQ(pendingManager_->GetPendingRequestWant(nullptr, getWantInfo), ERR_INVALID_VALUE);
}

/*
 * @tc.number    : PendingWantManagerTest_3400
 * @tc.name      : PendingWantManager GetPendingRequestWant
 * @tc.desc      : 1.GetPendingRequestWant, Get pendingRecord want object,want is nullptr.
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_3400, TestSize.Level1)
{
    Want want;
    ElementName element("device", "bundleName", "abilityName");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    EXPECT_FALSE(((unsigned int)wantSenderInfo.flags & (unsigned int)Flags::NO_BUILD_FLAG) != 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    auto pendingRecord = iface_cast<PendingWantRecord>(
        pendingManager_->GetWantSenderLocked(1, 1, wantSenderInfo.userId, wantSenderInfo, nullptr)->AsObject());
    EXPECT_NE(pendingRecord, nullptr);
    std::shared_ptr<Want> getWantInfo;
    EXPECT_EQ(pendingManager_->GetPendingRequestWant(pendingRecord, getWantInfo), ERR_INVALID_VALUE);
}

/*
 * @tc.number    : PendingWantManagerTest_3500
 * @tc.name      : PendingWantManager ClearPendingWantRecordTask
 * @tc.desc      : 1.ClearPendingWantRecordTask.
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_3500, TestSize.Level1)
{
    Want want1;
    ElementName element("device", "bundleName1", "abilityName1");
    want1.SetElement(element);
    Want want2;
    ElementName element2("device", "bundleName2", "abilityName2");
    want2.SetElement(element2);
    std::vector<Want> wants;
    wants.emplace_back(want1);
    wants.emplace_back(want2);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(wants, 0, 0);
    EXPECT_FALSE(((unsigned int)wantSenderInfo.flags & (unsigned int)Flags::NO_BUILD_FLAG) != 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    auto pendingRecord = iface_cast<PendingWantRecord>(
        pendingManager_->GetWantSenderLocked(1, 1, wantSenderInfo.userId, wantSenderInfo, nullptr)->AsObject());
    EXPECT_NE(pendingRecord, nullptr);
    EXPECT_EQ((int)pendingManager_->wantRecords_.size(), 1);
    Want want3;
    ElementName element3("device", "bundleName2", "abilityName2");
    want3.SetElement(element3);
    WantSenderInfo wantSenderInfo1 = MakeWantSenderInfo(want3, 0, 0);
    EXPECT_FALSE(((unsigned int)wantSenderInfo1.flags & (unsigned int)Flags::NO_BUILD_FLAG) != 0);
    auto pendingRecord1 = iface_cast<PendingWantRecord>(
        pendingManager_->GetWantSenderLocked(1, 1, wantSenderInfo1.userId, wantSenderInfo1, nullptr)->AsObject());
    EXPECT_NE(pendingRecord1, nullptr);
    EXPECT_EQ((int)pendingManager_->wantRecords_.size(), 2);
    pendingManager_->ClearPendingWantRecordTask("bundleName2", 1);
    EXPECT_EQ((int)pendingManager_->wantRecords_.size(), 0);
}

/*
 * @tc.number    : PendingWantManagerTest_3600
 * @tc.name      : PendingWantManager ClearPendingWantRecordTask
 * @tc.desc      : 1.ClearPendingWantRecordTask.
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_3600, TestSize.Level1)
{
    Want want1;
    ElementName element("device", "bundleName1", "abilityName1");
    want1.SetElement(element);
    Want want2;
    ElementName element2("device", "bundleName2", "abilityName2");
    want2.SetElement(element2);
    std::vector<Want> wants;
    wants.emplace_back(want1);
    wants.emplace_back(want2);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(wants, 0, 0);
    EXPECT_FALSE(((unsigned int)wantSenderInfo.flags & (unsigned int)Flags::NO_BUILD_FLAG) != 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    auto pendingRecord = iface_cast<PendingWantRecord>(
        pendingManager_->GetWantSenderLocked(1, 1, wantSenderInfo.userId, wantSenderInfo, nullptr)->AsObject());
    EXPECT_NE(pendingRecord, nullptr);
    EXPECT_EQ((int)pendingManager_->wantRecords_.size(), 1);
    Want want3;
    ElementName element3("device", "bundleName2", "abilityName2");
    want3.SetElement(element3);
    WantSenderInfo wantSenderInfo1 = MakeWantSenderInfo(want3, 0, 0);
    EXPECT_FALSE(((unsigned int)wantSenderInfo1.flags & (unsigned int)Flags::NO_BUILD_FLAG) != 0);
    auto pendingRecord1 = iface_cast<PendingWantRecord>(
        pendingManager_->GetWantSenderLocked(1, 1, wantSenderInfo1.userId, wantSenderInfo1, nullptr)->AsObject());
    EXPECT_NE(pendingRecord1, nullptr);
    EXPECT_EQ((int)pendingManager_->wantRecords_.size(), 2);
    pendingManager_->ClearPendingWantRecordTask("bundleName1", 1);
    EXPECT_EQ((int)pendingManager_->wantRecords_.size(), 1);
}

/*
 * @tc.number    : PendingWantManagerTest_3700
 * @tc.name      : PendingWantManager ClearPendingWantRecordTask
 * @tc.desc      : 1.ClearPendingWantRecordTask.
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_3700, TestSize.Level1)
{
    Want want1;
    ElementName element("device", "bundleName1", "abilityName1");
    want1.SetElement(element);
    Want want2;
    ElementName element2("device", "bundleName2", "abilityName2");
    want2.SetElement(element2);
    std::vector<Want> wants;
    wants.emplace_back(want1);
    wants.emplace_back(want2);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(wants, 0, 0);
    EXPECT_FALSE(((unsigned int)wantSenderInfo.flags & (unsigned int)Flags::NO_BUILD_FLAG) != 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    auto pendingRecord = iface_cast<PendingWantRecord>(
        pendingManager_->GetWantSenderLocked(1, 1, wantSenderInfo.userId, wantSenderInfo, nullptr)->AsObject());
    EXPECT_NE(pendingRecord, nullptr);
    EXPECT_EQ((int)pendingManager_->wantRecords_.size(), 1);
    Want want3;
    ElementName element3("device", "bundleName2", "abilityName2");
    want3.SetElement(element3);
    WantSenderInfo wantSenderInfo1 = MakeWantSenderInfo(want3, 0, 0);
    EXPECT_FALSE(((unsigned int)wantSenderInfo1.flags & (unsigned int)Flags::NO_BUILD_FLAG) != 0);
    auto pendingRecord1 = iface_cast<PendingWantRecord>(
        pendingManager_->GetWantSenderLocked(1, 1, wantSenderInfo1.userId, wantSenderInfo1, nullptr)->AsObject());
    EXPECT_NE(pendingRecord1, nullptr);
    EXPECT_EQ((int)pendingManager_->wantRecords_.size(), 2);
    pendingManager_->ClearPendingWantRecordTask("bundleName3", 1);
    EXPECT_EQ((int)pendingManager_->wantRecords_.size(), 2);
}

/*
 * @tc.number    : PendingWantManagerTest_3800
 * @tc.name      : PendingWantManager ClearPendingWantRecordTask
 * @tc.desc      : 1.ClearPendingWantRecordTask.
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_3800, TestSize.Level1)
{
    pendingManager_ = std::make_shared<PendingWantManager>();
    int32_t pid = 0;
    pendingManager_->CheckWindowState(pid);
    EXPECT_TRUE(pendingManager_ != nullptr);
}

/*
 * @tc.number    : PendingWantManagerTest_3900
 * @tc.name      : PendingWantManager GetAllRunningInstanceKeysByBundleName
 * @tc.desc      : 1.GetAllRunningInstanceKeysByBundleName.
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_3900, TestSize.Level1)
{
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::string bundleName;
    std::vector<std::string> appKeyVec;
    int32_t ret = pendingManager_->GetAllRunningInstanceKeysByBundleName(bundleName, appKeyVec);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/*
 * @tc.number    : PendingWantManagerTest_4000
 * @tc.name      : PendingWantManager GetAllRunningInstanceKeysByBundleName
 * @tc.desc      : 1.GetAllRunningInstanceKeysByBundleName.
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_4000, TestSize.Level1)
{
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::string bundleName{ "com.example.test" };
    std::vector<std::string> appKeyVec;
    int32_t ret = pendingManager_->GetAllRunningInstanceKeysByBundleName(bundleName, appKeyVec);
    EXPECT_NE(ret, ERR_INVALID_VALUE);
}

/*
 * @tc.number    : PendingWantManagerTest_4100
 * @tc.name      : PendingWantManager ClearPendingWantRecordTask
 * @tc.desc      : 1.ClearPendingWantRecordTask.
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_4100, TestSize.Level1)
{
    Want want1;
    ElementName element("device", "bundleName1", "abilityName1");
    want1.SetElement(element);
    Want want2;
    ElementName element2("device", "bundleName2", "abilityName2");
    want2.SetElement(element2);
    std::vector<Want> wants;
    wants.emplace_back(want1);
    wants.emplace_back(want2);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(wants, 0, 0);
    EXPECT_FALSE(((unsigned int)wantSenderInfo.flags & (unsigned int)Flags::NO_BUILD_FLAG) != 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    auto pendingRecord = iface_cast<PendingWantRecord>(
    pendingManager_->GetWantSenderLocked(1, 1, wantSenderInfo.userId, wantSenderInfo, nullptr)->AsObject());
    EXPECT_NE(pendingRecord, nullptr);
    EXPECT_EQ((int)pendingManager_->wantRecords_.size(), 1);
    pendingManager_->ClearPendingWantRecordTask("bundleName2", 1);
    EXPECT_EQ((int)pendingManager_->wantRecords_.size(), 0);
}

/*
 * @tc.number    : PendingWantManagerTest_4200
 * @tc.name      : PendingWantManager CheckPendingWantRecordByKey
 * @tc.desc      : 1.CheckPendingWantRecordByKey
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_4200, TestSize.Level1)
{
    pendingManager_ = std::make_shared<PendingWantManager>();
    std::shared_ptr<PendingWantKey> pendingKey = nullptr;
    std::shared_ptr<PendingWantKey> pendingKey1 = nullptr;

    auto ret = pendingManager_->CheckPendingWantRecordByKey(pendingKey, pendingKey1);
    EXPECT_EQ(ret, false);
}

/*
 * @tc.number    : PendingWantManagerTest_4300
 * @tc.name      : PendingWantManager CheckPendingWantRecordByKey
 * @tc.desc      : 1.CheckPendingWantRecordByKey
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_4300, TestSize.Level1)
{
    pendingManager_ = std::make_shared<PendingWantManager>();

    std::shared_ptr<PendingWantKey> pendingKey = std::make_shared<PendingWantKey>();
    pendingKey->appIndex_ = 1;

    std::shared_ptr<PendingWantKey> pendingKey1 = std::make_shared<PendingWantKey>();
    pendingKey1->appIndex_ = 2;

    auto ret = pendingManager_->CheckPendingWantRecordByKey(pendingKey, pendingKey1);
    EXPECT_EQ(ret, false);
}

/*
 * @tc.number    : PendingWantManagerTest_4400
 * @tc.name      : PendingWantManager CheckPendingWantRecordByKey
 * @tc.desc      : 1.CheckPendingWantRecordByKey
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_4400, TestSize.Level1)
{
    pendingManager_ = std::make_shared<PendingWantManager>();

    std::shared_ptr<PendingWantKey> pendingKey = std::make_shared<PendingWantKey>();
    pendingKey->appIndex_ = 1;
    pendingKey->bundleName_ = "ByeBye";

    std::shared_ptr<PendingWantKey> pendingKey1 = std::make_shared<PendingWantKey>();
    pendingKey1->appIndex_ = 1;
    pendingKey1->bundleName_ = "HelloWorld";

    auto ret = pendingManager_->CheckPendingWantRecordByKey(pendingKey, pendingKey1);
    EXPECT_EQ(ret, false);
}

/*
 * @tc.number    : PendingWantManagerTest_4500
 * @tc.name      : PendingWantManager CheckPendingWantRecordByKey
 * @tc.desc      : 1.CheckPendingWantRecordByKey
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_4500, TestSize.Level1)
{
    pendingManager_ = std::make_shared<PendingWantManager>();

    std::shared_ptr<PendingWantKey> pendingKey = std::make_shared<PendingWantKey>();
    pendingKey->appIndex_ = 1;
    pendingKey->bundleName_ = "ByeBye";

    std::shared_ptr<PendingWantKey> pendingKey1 = std::make_shared<PendingWantKey>();
    pendingKey1->appIndex_ = 1;
    pendingKey1->bundleName_ = "HelloWorld";

    auto ret = pendingManager_->CheckPendingWantRecordByKey(pendingKey, pendingKey1);
    EXPECT_EQ(ret, false);
}

/*
 * @tc.number    : PendingWantManagerTest_4600
 * @tc.name      : PendingWantManager CheckPendingWantRecordByKey
 * @tc.desc      : 1.CheckPendingWantRecordByKey
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_4600, TestSize.Level1)
{
    pendingManager_ = std::make_shared<PendingWantManager>();

    std::shared_ptr<PendingWantKey> pendingKey = std::make_shared<PendingWantKey>();
    pendingKey->appIndex_ = 1;
    pendingKey->bundleName_ = "HelloWorld";
    pendingKey->type_ = 1;

    std::shared_ptr<PendingWantKey> pendingKey1 = std::make_shared<PendingWantKey>();
    pendingKey1->appIndex_ = 1;
    pendingKey1->bundleName_ = "HelloWorld";
    pendingKey1->type_ = 2;

    auto ret = pendingManager_->CheckPendingWantRecordByKey(pendingKey, pendingKey1);
    EXPECT_EQ(ret, false);
}

/*
 * @tc.number    : PendingWantManagerTest_4700
 * @tc.name      : PendingWantManager CheckPendingWantRecordByKey
 * @tc.desc      : 1.CheckPendingWantRecordByKey
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_4700, TestSize.Level1)
{
    pendingManager_ = std::make_shared<PendingWantManager>();

    std::shared_ptr<PendingWantKey> pendingKey = std::make_shared<PendingWantKey>();
    pendingKey->appIndex_ = 1;
    pendingKey->bundleName_ = "HelloWorld";
    pendingKey->type_ = 1;
    pendingKey->requestWho_ = "ByeBye";

    std::shared_ptr<PendingWantKey> pendingKey1 = std::make_shared<PendingWantKey>();
    pendingKey1->appIndex_ = 1;
    pendingKey1->bundleName_ = "HelloWorld";
    pendingKey1->type_ = 1;
    pendingKey1->requestWho_ = "HelloWorld";

    auto ret = pendingManager_->CheckPendingWantRecordByKey(pendingKey, pendingKey1);
    EXPECT_EQ(ret, false);
}

/*
 * @tc.number    : PendingWantManagerTest_4800
 * @tc.name      : PendingWantManager CheckPendingWantRecordByKey
 * @tc.desc      : 1.CheckPendingWantRecordByKey
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_4800, TestSize.Level1)
{
    pendingManager_ = std::make_shared<PendingWantManager>();

    std::shared_ptr<PendingWantKey> pendingKey = std::make_shared<PendingWantKey>();
    pendingKey->appIndex_ = 1;
    pendingKey->bundleName_ = "HelloWorld";
    pendingKey->type_ = 1;
    pendingKey->requestWho_ = "HelloWorld";
    pendingKey->requestCode_ = 1;

    std::shared_ptr<PendingWantKey> pendingKey1 = std::make_shared<PendingWantKey>();
    pendingKey1->appIndex_ = 1;
    pendingKey1->bundleName_ = "HelloWorld";
    pendingKey1->type_ = 1;
    pendingKey1->requestWho_ = "HelloWorld";
    pendingKey1->requestCode_ = 2;

    auto ret = pendingManager_->CheckPendingWantRecordByKey(pendingKey, pendingKey1);
    EXPECT_EQ(ret, false);
}

/*
 * @tc.number    : PendingWantManagerTest_4900
 * @tc.name      : PendingWantManager CheckPendingWantRecordByKey
 * @tc.desc      : 1.CheckPendingWantRecordByKey
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_4900, TestSize.Level1)
{
    pendingManager_ = std::make_shared<PendingWantManager>();

    std::shared_ptr<PendingWantKey> pendingKey = std::make_shared<PendingWantKey>();
    pendingKey->appIndex_ = 1;
    pendingKey->bundleName_ = "HelloWorld";
    pendingKey->type_ = 1;
    pendingKey->requestWho_ = "HelloWorld";
    pendingKey->requestCode_ = 1;
    pendingKey->requestResolvedType_ = "HelloWorld";
    pendingKey->userId_ = 1;

    std::shared_ptr<PendingWantKey> pendingKey1 = std::make_shared<PendingWantKey>();
    pendingKey1->appIndex_ = 1;
    pendingKey1->bundleName_ = "HelloWorld";
    pendingKey1->type_ = 1;
    pendingKey1->requestWho_ = "HelloWorld";
    pendingKey1->requestCode_ = 1;
    pendingKey1->requestResolvedType_ = "HelloWorld";
    pendingKey1->userId_ = 1;
    pendingKey1->requestWant_.SetParam("HelloWorld", true);

    auto ret = pendingManager_->CheckPendingWantRecordByKey(pendingKey, pendingKey1);
    EXPECT_EQ(ret, false);
}

/*
 * @tc.number    : PendingWantManagerTest_5000
 * @tc.name      : PendingWantManager SendWantSender
 * @tc.desc      : 1.SendWantSender
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_5000, TestSize.Level1)
{
    pendingManager_ = std::make_shared<PendingWantManager>();

    SenderInfo senderInfo;
    auto ret = pendingManager_->SendWantSender(nullptr, senderInfo);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/*
 * @tc.number    : PendingWantManagerTest_5100
 * @tc.name      : PendingWantManager CancelWantSender
 * @tc.desc      : 1.CancelWantSender
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_5100, TestSize.Level1)
{
    pendingManager_ = std::make_shared<PendingWantManager>();
    auto pendingKey = std::make_shared<PendingWantKey>();
    auto pendingRecord = sptr<PendingWantRecord>::MakeSptr();
    pendingManager_->wantRecords_.clear();
    pendingManager_->wantRecords_.emplace(pendingKey, pendingRecord);

    pendingManager_->CancelWantSender(false, nullptr);
    auto wantRcdSize = pendingManager_->wantRecords_.size();
    EXPECT_EQ(wantRcdSize, 1);
}

/*
 * @tc.number    : PendingWantManagerTest_5200
 * @tc.name      : PendingWantManager CancelWantSender
 * @tc.desc      : 1.CancelWantSender
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_5200, TestSize.Level1)
{
    pendingManager_ = std::make_shared<PendingWantManager>();
    auto pendingKey = std::make_shared<PendingWantKey>();
    auto pendingRecord = sptr<PendingWantRecord>::MakeSptr();
    pendingManager_->wantRecords_.clear();
    pendingManager_->wantRecords_.emplace(pendingKey, pendingRecord);
    sptr<IWantSender> sender = sptr<PendingWantRecord>::MakeSptr();

    auto wantRcdSize = pendingManager_->wantRecords_.size();
    EXPECT_EQ(wantRcdSize, 1);
    pendingManager_->CancelWantSender(true, sender);
    wantRcdSize = pendingManager_->wantRecords_.size();
    EXPECT_EQ(wantRcdSize, 1);
}

/*
 * @tc.number    : PendingWantManagerTest_5300
 * @tc.name      : PendingWantManager PendingWantStartServiceExtension
 * @tc.desc      : 1.PendingWantStartServiceExtension
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_5300, TestSize.Level1)
{
    pendingManager_ = std::make_shared<PendingWantManager>();
    Want want;

    auto ret = pendingManager_->PendingWantStartServiceExtension(want, nullptr);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/*
 * @tc.number    : PendingWantManagerTest_5400
 * @tc.name      : PendingWantManager PendingWantPublishCommonEvent
 * @tc.desc      : 1.PendingWantPublishCommonEvent
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_5400, TestSize.Level1)
{
    pendingManager_ = std::make_shared<PendingWantManager>();
    Want want;
    want.SetBundle("HelloWorld");
    SenderInfo senderInfo;
    senderInfo.requiredPermission = "HelloWorld";

    auto ret = pendingManager_->PendingWantPublishCommonEvent(want, senderInfo, 1, 1);
    EXPECT_EQ(ret, -1);
}

/*
 * @tc.number    : PendingWantManagerTest_5500
 * @tc.name      : PendingWantManager PendingWantPublishCommonEvent
 * @tc.desc      : 1.PendingWantPublishCommonEvent
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_5500, TestSize.Level1)
{
    pendingManager_ = std::make_shared<PendingWantManager>();
    std::shared_ptr<WantSenderInfo> info = nullptr;

    auto ret = pendingManager_->GetWantSenderInfo(nullptr, info);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/*
 * @tc.number    : PendingWantManagerTest_5600
 * @tc.name      : PendingWantManager ClearPendingWantRecord
 * @tc.desc      : 1.ClearPendingWantRecord
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_5600, TestSize.Level1)
{
    pendingManager_ = std::make_shared<PendingWantManager>();
    pendingManager_->taskHandler_ = nullptr;

    pendingManager_->ClearPendingWantRecord("HelloWorld", 1);
    EXPECT_EQ(pendingManager_->taskHandler_,  nullptr);
}

/*
 * @tc.number    : PendingWantManagerTest_5700
 * @tc.name      : PendingWantManager CheckPermission
 * @tc.desc      : 1.CheckPermission
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_5700, TestSize.Level1)
{
    pendingManager_ = std::make_shared<PendingWantManager>();

    auto ret = pendingManager_->CheckPermission(nullptr);
    EXPECT_EQ(ret,  false);
}

/*
 * @tc.number    : PendingWantManagerTest_5800
 * @tc.name      : PendingWantManager CheckPermission
 * @tc.desc      : 1.CheckPermission
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_5800, TestSize.Level1)
{
    pendingManager_ = std::make_shared<PendingWantManager>();
    auto record = sptr<PendingWantRecord>::MakeSptr();
    record->canceled_ = false;
    record->key_ = std::make_shared<PendingWantKey>();
    record->key_->type_ = static_cast<int32_t>(OperationType::START_ABILITY);

    auto ret = pendingManager_->CheckPermission(record);
    EXPECT_EQ(ret,  true);
}

/*
 * @tc.number    : PendingWantManagerTest_5900
 * @tc.name      : PendingWantManager CheckPermission
 * @tc.desc      : 1.CheckPermission
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_5900, TestSize.Level1)
{
    pendingManager_ = std::make_shared<PendingWantManager>();
    auto record = sptr<PendingWantRecord>::MakeSptr();
    record->canceled_ = true;

    auto ret = pendingManager_->CheckPermission(record);
    EXPECT_EQ(ret,  true);
}

/*
 * @tc.number    : PendingWantManagerTest_6000
 * @tc.name      : PendingWantManager CheckCallerPermission
 * @tc.desc      : 1.CheckCallerPermission
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_6000, TestSize.Level1)
{
    pendingManager_ = std::make_shared<PendingWantManager>();

    auto ret = pendingManager_->CheckCallerPermission();
    EXPECT_EQ(ret,  true);
}

/*
 * @tc.number    : PendingWantManagerTest_6100
 * @tc.name      : PendingWantManager GetAllRunningInstanceKeysByBundleName
 * @tc.desc      : 1.GetAllRunningInstanceKeysByBundleName
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_6100, TestSize.Level1)
{
    pendingManager_ = std::make_shared<PendingWantManager>();
    std::vector<std::string> appKeyVec = {"", ""};

    auto ret = pendingManager_->GetAllRunningInstanceKeysByBundleName("HelloWorld", appKeyVec);
    EXPECT_NE(ret,  OBJECT_NULL);
}

/*
 * @tc.number    : PendingWantManagerTest_6200
 * @tc.name      : PendingWantManager AddWantAgentNumber
 * @tc.desc      : 1.AddWantAgentNumber
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_6200, TestSize.Level1)
{
    Want want;
    std::string bundleName = "bundleName";
    ElementName element("device", "bundleName", "abilityName");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, static_cast<int32_t>(Flags::NO_BUILD_FLAG), 0);
    EXPECT_TRUE(((unsigned int)wantSenderInfo.flags & (unsigned int)Flags::NO_BUILD_FLAG) != 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);

    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    pendingManager_->HandleAddWantAgentNumber(key);

    auto it = pendingManager_->wantAgentCount_.find(key->GetBundleName());
    EXPECT_EQ(it->second.currentNumber, 1);

    // check DFX THRESHOLD
    it->second.currentNumber = 289;
    ElementName element2("device", bundleName, "abilityName2");
    WantSenderInfo wantSenderInfo2 = MakeWantSenderInfo(want, static_cast<int32_t>(Flags::NO_BUILD_FLAG), 0);
    std::shared_ptr<PendingWantKey> key2 = MakeWantKey(wantSenderInfo2);
    pendingManager_->HandleAddWantAgentNumber(key2);

    auto it2 = pendingManager_->wantAgentCount_.find(key2->GetBundleName());
    EXPECT_EQ(it->second.currentNumber, 290);
}

/*
 * @tc.number    : PendingWantManagerTest_6300
 * @tc.name      : PendingWantManager ReduceWantAgentNumber
 * @tc.desc      : 1.ReduceWantAgentNumber
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_6300, TestSize.Level1)
{
    Want want;
    std::string bundleName = "bundleName";
    ElementName element("device", "bundleName", "abilityName");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, static_cast<int32_t>(Flags::NO_BUILD_FLAG), 0);
    EXPECT_TRUE(((unsigned int)wantSenderInfo.flags & (unsigned int)Flags::NO_BUILD_FLAG) != 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);

    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    pendingManager_->HandleAddWantAgentNumber(key);

    auto it = pendingManager_->wantAgentCount_.find(key->GetBundleName());
    EXPECT_EQ(it->second.currentNumber, 1);

    // check Reduce wantAgentNumber
    it->second.currentNumber = 10;
    ElementName element2("device", bundleName, "abilityName2");
    WantSenderInfo wantSenderInfo2 = MakeWantSenderInfo(want, static_cast<int32_t>(Flags::NO_BUILD_FLAG), 0);
    std::shared_ptr<PendingWantKey> key2 = MakeWantKey(wantSenderInfo2);
    pendingManager_->HandleReduceWantAgentNumber(key2);

    auto it2 = pendingManager_->wantAgentCount_.find(key2->GetBundleName());
    EXPECT_EQ(it->second.currentNumber, 9);
}

/*
 * @tc.number    : PendingWantManagerTest_6400
 * @tc.name      : PendingWantManager GetPendingRequestWantFromProxy
 * @tc.desc      : 1.GetPendingRequestWantFromProxy, target is nullptr
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_6400, TestSize.Level1)
{
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<Want> getWantInfo = std::make_shared<Want>();
    EXPECT_EQ(pendingManager_->GetPendingRequestWantFromProxy(nullptr, getWantInfo), ERR_INVALID_VALUE);
}

/*
 * @tc.number    : PendingWantManagerTest_6500
 * @tc.name      : PendingWantManager GetPendingRequestWantFromProxy
 * @tc.desc      : 1.GetPendingRequestWantFromProxy, want is nullptr
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_6500, TestSize.Level1)
{
    Want want;
    ElementName element("device", "bundleName", "abilityName");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    EXPECT_FALSE(((unsigned int)wantSenderInfo.flags & (unsigned int)Flags::NO_BUILD_FLAG) != 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    auto pendingRecord = iface_cast<PendingWantRecord>(
        pendingManager_->GetWantSenderLocked(1, 1, wantSenderInfo.userId, wantSenderInfo, nullptr)->AsObject());
    EXPECT_NE(pendingRecord, nullptr);
    std::shared_ptr<Want> getWantInfo;
    EXPECT_EQ(pendingManager_->GetPendingRequestWantFromProxy(pendingRecord, getWantInfo), ERR_INVALID_VALUE);
}

/*
 * @tc.number    : PendingWantManagerTest_6600
 * @tc.name      : PendingWantManager GetPendingRequestWantFromProxy
 * @tc.desc      : 1.GetPendingRequestWantFromProxy, normal scenario
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_6600, TestSize.Level1)
{
    Want want;
    ElementName element("device", "bundleName", "abilityName");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    EXPECT_FALSE(((unsigned int)wantSenderInfo.flags & (unsigned int)Flags::NO_BUILD_FLAG) != 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    auto pendingRecord = iface_cast<PendingWantRecord>(
        pendingManager_->GetWantSenderLocked(1, 1, wantSenderInfo.userId, wantSenderInfo, nullptr)->AsObject());
    EXPECT_NE(pendingRecord, nullptr);
    std::shared_ptr<Want> getWantInfo = std::make_shared<Want>();
    EXPECT_EQ(pendingManager_->GetPendingRequestWantFromProxy(pendingRecord, getWantInfo), NO_ERROR);
    EXPECT_EQ(getWantInfo->GetElement().GetBundleName(), "bundleName");
    EXPECT_EQ(getWantInfo->GetElement().GetAbilityName(), "abilityName");
}

/*
 * @tc.number    : PendingWantManagerTest_6700
 * @tc.name      : PendingWantManager GetPendingRequestWantFromProxy
 * @tc.desc      : 1.GetPendingRequestWantFromProxy, with parameters in want
 */
HWTEST_F(PendingWantManagerTest, PendingWantManagerTest_6700, TestSize.Level1)
{
    Want want;
    ElementName element("device", "bundleName", "abilityName");
    want.SetElement(element);
    want.SetParam("test_key", std::string("test_value"));
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    EXPECT_FALSE(((unsigned int)wantSenderInfo.flags & (unsigned int)Flags::NO_BUILD_FLAG) != 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    auto pendingRecord = iface_cast<PendingWantRecord>(
        pendingManager_->GetWantSenderLocked(1, 1, wantSenderInfo.userId, wantSenderInfo, nullptr)->AsObject());
    EXPECT_NE(pendingRecord, nullptr);
    std::shared_ptr<Want> getWantInfo = std::make_shared<Want>();
    EXPECT_EQ(pendingManager_->GetPendingRequestWantFromProxy(pendingRecord, getWantInfo), NO_ERROR);
    EXPECT_EQ(getWantInfo->GetElement().GetBundleName(), "bundleName");
    EXPECT_EQ(getWantInfo->GetElement().GetAbilityName(), "abilityName");
    EXPECT_EQ(getWantInfo->GetParams().GetStringParam("test_key"), "test_value");
}
}  // namespace AAFwk
}  // namespace OHOS
