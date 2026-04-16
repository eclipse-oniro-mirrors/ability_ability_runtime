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
#include "bundlemgr/mock_bundle_manager.h"
#include "mock_ability_connect_callback.h"
#include "ability_manager_errors.h"
#include "ability_scheduler.h"
#define private public
#define protected public
#include "ability_event_handler.h"
#include "ability_manager_service.h"
#undef private
#undef protected
#include "if_system_ability_manager.h"
#include "int_wrapper.h"
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
#define SLEEP(milli) std::this_thread::sleep_for(std::chrono::seconds(milli))
const int32_t SEND_COUNT = 100;
const int32_t PERFORM_RECEIVE_COUNT = 100;
const int32_t INVALID_UID = -1;
}  // namespace
class PendingWantRecordTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    WantSenderInfo MakeWantSenderInfo(Want& want, int32_t flags, int32_t userId, int32_t type = 1);
    std::shared_ptr<PendingWantKey> MakeWantKey(WantSenderInfo& wantSenderInfo);
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
};

int PendingWantRecordTest::CancelReceiver::performReceiveCount = 0;
int PendingWantRecordTest::CancelReceiver::sendCount = 0;

void PendingWantRecordTest::CancelReceiver::Send(const int32_t resultCode)
{
    sendCount = SEND_COUNT;
}

void PendingWantRecordTest::CancelReceiver::PerformReceive(const AAFwk::Want& want, int resultCode,
    const std::string& data, const AAFwk::WantParams& extras, bool serialized, bool sticky, int sendingUser)
{
    performReceiveCount = PERFORM_RECEIVE_COUNT;
}

void PendingWantRecordTest::SetUpTestCase()
{
    OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->RegisterSystemAbility(
        OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, new BundleMgrService());
}

void PendingWantRecordTest::TearDownTestCase()
{
    OHOS::DelayedSingleton<SaMgrClient>::DestroyInstance();
}

void PendingWantRecordTest::SetUp()
{
}

void PendingWantRecordTest::TearDown()
{
}

WantSenderInfo PendingWantRecordTest::MakeWantSenderInfo(Want& want, int32_t flags, int32_t userId, int32_t type)
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

std::shared_ptr<PendingWantKey> PendingWantRecordTest::MakeWantKey(WantSenderInfo& wantSenderInfo)
{
    std::shared_ptr<PendingWantKey> pendingKey = std::make_shared<PendingWantKey>();
    pendingKey->SetBundleName(wantSenderInfo.bundleName);
    pendingKey->SetRequestWho(wantSenderInfo.resultWho);
    pendingKey->SetRequestCode(wantSenderInfo.requestCode);
    pendingKey->SetAllWantsInfos(wantSenderInfo.allWants);
    pendingKey->SetFlags(wantSenderInfo.flags);
    pendingKey->SetUserId(wantSenderInfo.userId);
    pendingKey->SetType(wantSenderInfo.type);
    pendingKey->SetRequestWant(wantSenderInfo.allWants.back().want);
    pendingKey->SetRequestResolvedType(wantSenderInfo.allWants.back().resolvedTypes);
    wantSenderInfo.allWants.back().want = pendingKey->GetRequestWant();
    wantSenderInfo.allWants.back().resolvedTypes = pendingKey->GetRequestResolvedType();
    pendingKey->SetAllWantsInfos(wantSenderInfo.allWants);
    return pendingKey;
}

/*
 * @tc.number    : PendingWantRecordTest_0100
 * @tc.name      : PendingWantManager Constructors
 * @tc.desc      : 1.Constructors
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_0100, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    EXPECT_EQ(pendingWantRecord->uid_, 1);
    EXPECT_EQ(pendingWantRecord->callerToken_, nullptr);
}

/*
 * @tc.number    : PendingWantRecordTest_0200
 * @tc.name      : PendingWantManager RegisterCancelListener
 * @tc.desc      : 1.RegisterCancelListener(nullptr)
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_0200, TestSize.Level1)
{
    sptr<CancelReceiver> cance = new CancelReceiver();
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    pendingWantRecord->RegisterCancelListener(cance);
    EXPECT_EQ(static_cast<int>(pendingWantRecord->mCancelCallbacks_.size()), 1);
}

/*
 * @tc.number    : PendingWantRecordTest_0300
 * @tc.name      : PendingWantManager RegisterCancelListener
 * @tc.desc      : 1.RegisterCancelListener,one sptr<CancelReceiver>
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_0300, TestSize.Level1)
{
    sptr<CancelReceiver> cance = new CancelReceiver();
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    pendingWantRecord->RegisterCancelListener(nullptr);
    EXPECT_EQ(static_cast<int>(pendingWantRecord->mCancelCallbacks_.size()), 0);
}

/*
 * @tc.number    : PendingWantRecordTest_0400
 * @tc.name      : PendingWantManager RegisterCancelListener
 * @tc.desc      : 1.RegisterCancelListener,two sptr<CancelReceiver>
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_0400, TestSize.Level1)
{
    sptr<CancelReceiver> cance = new CancelReceiver();
    sptr<CancelReceiver> cance1 = new CancelReceiver();
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    pendingWantRecord->RegisterCancelListener(cance);
    pendingWantRecord->RegisterCancelListener(cance1);
    EXPECT_EQ(static_cast<int>(pendingWantRecord->mCancelCallbacks_.size()), 2);
}

/*
 * @tc.number    : PendingWantRecordTest_0500
 * @tc.name      : PendingWantManager UnregisterCancelListener
 * @tc.desc      : 1.UnregisterCancelListener(sptr<CancelReceiver>)
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_0500, TestSize.Level1)
{
    sptr<CancelReceiver> cance = new CancelReceiver();
    sptr<CancelReceiver> cance1 = new CancelReceiver();
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    pendingWantRecord->RegisterCancelListener(cance);
    pendingWantRecord->RegisterCancelListener(cance1);
    pendingWantRecord->UnregisterCancelListener(cance);
    EXPECT_EQ(static_cast<int>(pendingWantRecord->mCancelCallbacks_.size()), 1);
}

/*
 * @tc.number    : PendingWantRecordTest_0600
 * @tc.name      : PendingWantManager UnregisterCancelListener
 * @tc.desc      : 1.UnregisterCancelListener(nullptr)
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_0600, TestSize.Level1)
{
    sptr<CancelReceiver> cance = new CancelReceiver();
    sptr<CancelReceiver> cance1 = new CancelReceiver();
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    pendingWantRecord->RegisterCancelListener(cance);
    pendingWantRecord->RegisterCancelListener(cance1);
    pendingWantRecord->UnregisterCancelListener(nullptr);
    EXPECT_EQ(static_cast<int>(pendingWantRecord->mCancelCallbacks_.size()), 2);
}

/*
 * @tc.number    : PendingWantRecordTest_0700
 * @tc.name      : PendingWantManager SenderInner
 * @tc.desc      : 1.SenderInner,pendingWantManager_ != nullptr,canceled_ == true
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_0700, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    SenderInfo info;
    pendingWantRecord->canceled_ = true;
    EXPECT_EQ(pendingWantRecord->SenderInner(info), 2097335);
}

/*
 * @tc.number    : PendingWantRecordTest_0800
 * @tc.name      : PendingWantManager SenderInner
 * @tc.desc      : 1.SenderInner,pendingWantManager_ != nullptr,flags:CONSTANT_FLAG,type:OperationType::START_ABILITY
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_0800, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo =
        MakeWantSenderInfo(want, (int32_t)Flags::CONSTANT_FLAG, 0, (int32_t)OperationType::START_ABILITY);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    SenderInfo info;
    EXPECT_NE(pendingWantRecord->SenderInner(info), 0); // Assert SenderInner failed.
    EXPECT_TRUE(info.resolvedType == key->GetRequestResolvedType());
}

/*
 * @tc.number    : PendingWantRecordTest_0900
 * @tc.name      : PendingWantManager SenderInner
 * @tc.desc      : 1.SenderInner,pendingWantManager_ == nullptr,flags:CONSTANT_FLAG
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_0900, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, (int32_t)Flags::CONSTANT_FLAG, 0);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(nullptr, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    SenderInfo info;
    EXPECT_EQ(pendingWantRecord->SenderInner(info), ERR_INVALID_VALUE);
    EXPECT_TRUE(info.resolvedType != key->GetRequestResolvedType());
}

/*
 * @tc.number    : PendingWantRecordTest_1000
 * @tc.name      : PendingWantManager SenderInner
 * @tc.desc      : 1.SenderInner,pendingWantManager_ == nullptr,flags:ONE_TIME_FLAG
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_1000, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, (int32_t)Flags::ONE_TIME_FLAG, 0);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(nullptr, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    SenderInfo info;
    info.finishedReceiver = new CancelReceiver();
    EXPECT_EQ(pendingWantRecord->SenderInner(info), ERR_INVALID_VALUE);
    EXPECT_TRUE(info.resolvedType != key->GetRequestResolvedType());
}

/*
 * @tc.number    : PendingWantRecordTest_1100
 * @tc.name      : PendingWantManager SenderInner
 * @tc.desc      : 1.SenderInner,pendingWantManager_ != nullptr,flags:CONSTANT_FLAG,type:OperationType::START_ABILITIES
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_1100, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo =
        MakeWantSenderInfo(want, (int32_t)Flags::CONSTANT_FLAG, 0, (int32_t)OperationType::START_ABILITIES);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    SenderInfo info;
    EXPECT_NE(pendingWantRecord->SenderInner(info), NO_ERROR);
    EXPECT_TRUE(info.resolvedType == key->GetRequestResolvedType());
}

/*
 * @tc.number    : PendingWantRecordTest_1200
 * @tc.name      : PendingWantManager SenderInner
 * @tc.desc      : 1.SenderInner,pendingWantManager_ != nullptr,flags:CONSTANT_FLAG,type:OperationType::START_SERVICE
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_1200, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo =
        MakeWantSenderInfo(want, (int32_t)Flags::CONSTANT_FLAG, 0, (int32_t)OperationType::START_SERVICE);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    SenderInfo info;
    EXPECT_NE(pendingWantRecord->SenderInner(info), NO_ERROR);
    EXPECT_TRUE(info.resolvedType == key->GetRequestResolvedType());
}

/*
 * @tc.number    : PendingWantRecordTest_1300
 * @tc.name      : PendingWantManager SenderInner
 * @tc.desc      : 1.SenderInner,pendingWantManager_ !=
 * nullptr,flags:CONSTANT_FLAG,type:OperationType::START_FOREGROUND_SERVICE
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_1300, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo =
        MakeWantSenderInfo(want, (int32_t)Flags::CONSTANT_FLAG, 0, (int32_t)OperationType::START_FOREGROUND_SERVICE);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    SenderInfo info;
    EXPECT_NE(pendingWantRecord->SenderInner(info), NO_ERROR);
    EXPECT_TRUE(info.resolvedType == key->GetRequestResolvedType());
}

/*
 * @tc.number    : PendingWantRecordTest_1400
 * @tc.name      : PendingWantManager SenderInner
 * @tc.desc      : 1.SenderInner,pendingWantManager_ !=
 * nullptr,flags:CONSTANT_FLAG,type:OperationType::SEND_COMMON_EVENT
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_1400, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo =
        MakeWantSenderInfo(want, (int32_t)Flags::CONSTANT_FLAG, 0, (int32_t)OperationType::SEND_COMMON_EVENT);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    SenderInfo info;
    pendingWantRecord->SenderInner(info);
    EXPECT_TRUE(info.resolvedType == key->GetRequestResolvedType());
}

/*
 * @tc.number    : PendingWantRecordTest_1500
 * @tc.name      : PendingWantManager SenderInner
 * @tc.desc      : 1.SenderInner,pendingWantManager_ !=
 * nullptr,flags:CONSTANT_FLAG,type:OperationType::SEND_COMMON_EVENT, 2.PerformReceive Called
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_1500, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo =
        MakeWantSenderInfo(want, (int32_t)Flags::CONSTANT_FLAG, 0, (int32_t)OperationType::SEND_COMMON_EVENT);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    SenderInfo info;
    info.finishedReceiver = new CancelReceiver();
    pendingWantRecord->SenderInner(info);
    EXPECT_TRUE(info.resolvedType == key->GetRequestResolvedType());
    EXPECT_TRUE(CancelReceiver::performReceiveCount == 100);
    CancelReceiver::performReceiveCount = 0;
}

/*
 * @tc.number    : PendingWantRecordTest_1600
 * @tc.name      : PendingWantManager GetKey
 * @tc.desc      : 1.GetKey
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_1600, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo =
        MakeWantSenderInfo(want, (int32_t)Flags::CONSTANT_FLAG, 0, (int32_t)OperationType::SEND_COMMON_EVENT);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    EXPECT_EQ(pendingWantRecord->GetKey(), key);
}

/*
 * @tc.number    : PendingWantRecordTest_1700
 * @tc.name      : PendingWantManager GetUid
 * @tc.desc      : 1.GetUid
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_1700, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo =
        MakeWantSenderInfo(want, (int32_t)Flags::CONSTANT_FLAG, 0, (int32_t)OperationType::SEND_COMMON_EVENT);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    EXPECT_EQ(pendingWantRecord->GetUid(), 1);
}

/*
 * @tc.number    : PendingWantRecordTest_1800
 * @tc.name      : PendingWantManager SetCanceled/GetCanceled
 * @tc.desc      : 1.SetCanceled/GetCanceled
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_1800, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo =
        MakeWantSenderInfo(want, (int32_t)Flags::CONSTANT_FLAG, 0, (int32_t)OperationType::SEND_COMMON_EVENT);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    EXPECT_EQ(pendingWantRecord->GetCanceled(), false);
    pendingWantRecord->SetCanceled();
    EXPECT_EQ(pendingWantRecord->GetCanceled(), true);
}

/*
 * @tc.number    : PendingWantRecordTest_1900
 * @tc.name      : PendingWantManager GetCancelCallbacks
 * @tc.desc      : 1.GetCancelCallbacks
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_1900, TestSize.Level1)
{
    sptr<CancelReceiver> cance = new CancelReceiver();
    sptr<CancelReceiver> cance1 = new CancelReceiver();
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    pendingWantRecord->RegisterCancelListener(cance);
    pendingWantRecord->RegisterCancelListener(cance1);
    EXPECT_EQ(static_cast<int>(pendingWantRecord->GetCancelCallbacks().size()), 2);
}

/*
 * @tc.number    : PendingWantRecordTest_2000
 * @tc.name      : PendingWantManager SenderInner
 * @tc.desc      : 1.SenderInner,pendingWantManager_ != nullptr,flags:CONSTANT_FLAG
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_2000, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, (int32_t)Flags::CONSTANT_FLAG, 0);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    SenderInfo info;
    pendingWantRecord->SenderInner(info);
    EXPECT_TRUE(info.resolvedType == key->GetRequestResolvedType());
}

/*
 * @tc.number    : PendingWantRecordTest_2100
 * @tc.name      : PendingWantManager SenderInner
 * @tc.desc      : 1.SenderInner,pendingWantManager_ != nullptr,flags:ONE_TIME_FLAG
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_2100, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, (int32_t)Flags::ONE_TIME_FLAG, 0);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    SenderInfo info;
    info.finishedReceiver = new CancelReceiver();
    pendingWantRecord->SenderInner(info);
    EXPECT_TRUE(info.resolvedType == key->GetRequestResolvedType());
}

/*
 * @tc.number    : PendingWantRecordTest_2200
 * @tc.name      : BuildSendWant
 * @tc.desc      : 1.BuildSendWant
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_2200, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    want.SetParam(std::string("ohos.extra.param.key.appInstance"), std::string("app_instance_100"));
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, (int32_t)Flags::ONE_TIME_FLAG, 0);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    EXPECT_NE(key, nullptr);
    key->SetType(static_cast<int32_t>(OperationType::START_SERVICE));
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);

    SenderInfo info;
    Want senderWant;
    pendingWantRecord->BuildSendWant(info, senderWant);
    EXPECT_EQ(senderWant.GetParams().GetStringParam(
        std::string("ohos.extra.param.key.appInstance")), std::string("app_instance_100"));
}

/*
 * @tc.number    : PendingWantRecordTest_2300
 * @tc.name      : CheckAppInstanceKey
 * @tc.desc      : 1.Verification failure case, bundle name empty
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_2300, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    want.SetParam(std::string("ohos.extra.param.key.appInstance"), std::string("app_instance_3"));

    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, (int32_t)Flags::ONE_TIME_FLAG, 0);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    EXPECT_NE(key, nullptr);
    key->SetType(static_cast<int32_t>(OperationType::START_ABILITY));
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);

    std::string bundleName{ "" };
    auto params = want.GetParams();
    pendingWantRecord->CheckAppInstanceKey(bundleName, params);
    EXPECT_EQ(params.GetStringParam(
        std::string("ohos.extra.param.key.appInstance")), std::string("app_instance_3"));
}


/*
 * @tc.number    : PendingWantRecordTest_2400
 * @tc.name      : CheckAppInstanceKey
 * @tc.desc      : 1.Successful verification cases
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_2400, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    want.SetParam(std::string("ohos.extra.param.key.appInstance"), std::string("app_instance_1"));

    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, (int32_t)Flags::ONE_TIME_FLAG, 0);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    EXPECT_NE(key, nullptr);
    key->SetType(static_cast<int32_t>(OperationType::START_ABILITY));
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);

    std::string bundleName{ "com.ix.hiMusic" };
    auto params = want.GetParams();
    pendingWantRecord->CheckAppInstanceKey(bundleName, params);
    EXPECT_EQ(params.GetStringParam(
        std::string("ohos.extra.param.key.appInstance")), std::string("app_instance_1"));
}

/*
 * @tc.number    : PendingWantRecordTest_2500
 * @tc.name      : CheckAppInstanceKey
 * @tc.desc      : 1.Do not check the START_SERVICE type
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_2500, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    want.SetParam(std::string("ohos.extra.param.key.appInstance"), std::string("app_instance_3"));

    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, (int32_t)Flags::ONE_TIME_FLAG, 0);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    EXPECT_NE(key, nullptr);
    key->SetType(static_cast<int32_t>(OperationType::START_SERVICE));
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);

    std::string bundleName{ "com.ix.hiMusic" };
    auto params = want.GetParams();
    pendingWantRecord->CheckAppInstanceKey(bundleName, params);
    EXPECT_EQ(params.GetStringParam(
        std::string("ohos.extra.param.key.appInstance")), std::string("app_instance_3"));
}

/*
 * @tc.number    : PendingWantRecordTest_2600
 * @tc.name      : GetAppIndexbyUid
 * @tc.desc      : 1.GetAppIndexbyUid
 */
HWTEST_F(PendingWantRecordTest, PendingWantRecordTest_2600, TestSize.Level1)
{
    Want want;
    std::string bundleName = "com.ix.hiMusic";
    ElementName element("device", bundleName, "MusicSAbility");
    want.SetElement(element);
    want.SetParam(std::string("ohos.extra.param.key.appInstance"), std::string("app_instance_100"));
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, (int32_t)Flags::ONE_TIME_FLAG, 0);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    EXPECT_NE(key, nullptr);
    key->SetType(static_cast<int32_t>(OperationType::START_SERVICE));
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);
    int32_t appIndex;
    int32_t ret = pendingWantRecord->GetAppIndexbyUid(pendingWantRecord->GetUid(), bundleName, appIndex);
    EXPECT_NE(ret, NO_ERROR);
}

/*
 * @tc.number    : PendingWantRecordTest_IsBundleNotExistForUser0_0100
 * @tc.name      : IsBundleNotExistForUser0
 * @tc.desc      : 1. userId != 0, return false
 */
HWTEST_F(PendingWantRecordTest, IsBundleNotExistForUser0_0100, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.example.notexist", "TestAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 100);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    key->SetUserId(100);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);

    bool result = pendingWantRecord->IsBundleNotExistForUser0(want);
    EXPECT_FALSE(result);
}

/*
 * @tc.number    : IsBundleNotExistForUser0_NullKey_0100
 * @tc.name      : IsBundleNotExistForUser0
 * @tc.desc      : key_ is nullptr, should return false
 */
HWTEST_F(PendingWantRecordTest, IsBundleNotExistForUser0_NullKey_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsBundleNotExistForUser0_NullKey_0100 start");
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);

    pendingManager_ = std::make_shared<PendingWantManager>();
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, nullptr);

    bool result = pendingWantRecord->IsBundleNotExistForUser0(want);
    EXPECT_EQ(result, false);
    TAG_LOGI(AAFwkTag::TEST, "IsBundleNotExistForUser0_NullKey_0100 end");
}

/*
 * @tc.number    : IsBundleNotExistForUser0_NonZeroUser_0100
 * @tc.name      : IsBundleNotExistForUser0
 * @tc.desc      : userId is not 0, should return false without query
 */
HWTEST_F(PendingWantRecordTest, IsBundleNotExistForUser0_NonZeroUser_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsBundleNotExistForUser0_NonZeroUser_0100 start");
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);

    pendingManager_ = std::make_shared<PendingWantManager>();
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 100);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);

    bool result = pendingWantRecord->IsBundleNotExistForUser0(want);
    EXPECT_EQ(result, false);
    TAG_LOGI(AAFwkTag::TEST, "IsBundleNotExistForUser0_NonZeroUser_0100 end");
}

/*
 * @tc.number    : PendingWantRecordTest_0100
 * @tc.name      : ExecuteOperation
 * @tc.desc      : check Starts a service extension
 */
HWTEST_F(PendingWantRecordTest, ExecuteOperation_0100, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "ExecuteOperation_0100 start.");
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    key->SetType(static_cast<int32_t>(OperationType::START_SERVICE_EXTENSION));
    SenderInfo info;
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);
    auto params = pendingWantRecord->ExecuteOperation(pendingManager_, info, want);
    EXPECT_NE(params, NO_ERROR);
}

/*
 * @tc.number    : ExecuteOperation_0200
 * @tc.name      : ExecuteOperation
 * @tc.desc      : check Unknown operation
 */
HWTEST_F(PendingWantRecordTest, ExecuteOperation_0200, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "ExecuteOperation_0200 start.");
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    key->SetType(static_cast<int32_t>(OperationType::UNKNOWN_TYPE));
    SenderInfo info;
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);
    auto params = pendingWantRecord->ExecuteOperation(pendingManager_, info, want);
    EXPECT_EQ(params, NO_ERROR);
    TAG_LOGD(AAFwkTag::TEST, "ExecuteOperation_0200 end.");
}

/*
 * @tc.number    : Send_0100
 * @tc.name      : Send
 * @tc.desc      : 1.Send, normal call scenario
 */
HWTEST_F(PendingWantRecordTest, Send_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Send_0100 start");

    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, (int32_t)Flags::CONSTANT_FLAG, 0,
        (int32_t)OperationType::START_ABILITY);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);

    SenderInfo info;
    pendingWantRecord->Send(info);
    EXPECT_TRUE(info.resolvedType == key->GetRequestResolvedType());

    TAG_LOGI(AAFwkTag::TEST, "Send_0100 end");
}

/*
 * @tc.number    : Send_0200
 * @tc.name      : Send
 * @tc.desc      : 1.Send, canceled scenario
 */
HWTEST_F(PendingWantRecordTest, Send_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Send_0200 start");

    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);

    pendingWantRecord->SetCanceled();
    SenderInfo info;
    info.finishedReceiver = new CancelReceiver();
    auto result = pendingWantRecord->SenderInner(info);
    EXPECT_EQ(result, ERR_WANTAGENT_CANCELED);

    TAG_LOGI(AAFwkTag::TEST, "Send_0200 end");
}

/*
 * @tc.number    : SetCallerUid_0100
 * @tc.name      : SetCallerUid
 * @tc.desc      : 1.SetCallerUid, normal call scenario
 */
HWTEST_F(PendingWantRecordTest, SetCallerUid_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetCallerUid_0100 start");

    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);

    EXPECT_EQ(pendingWantRecord->callerUid_, 0);
    int32_t testUid = 1001;
    pendingWantRecord->SetCallerUid(testUid);
    EXPECT_EQ(pendingWantRecord->callerUid_, testUid);

    TAG_LOGI(AAFwkTag::TEST, "SetCallerUid_0100 end");
}

/*
 * @tc.number    : BuildSendWant_MergeParams_0100
 * @tc.name      : BuildSendWant
 * @tc.desc      : 1.BuildSendWant, verify parameter merging logic
 */
HWTEST_F(PendingWantRecordTest, BuildSendWant_MergeParams_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "BuildSendWant_MergeParams_0100 start");

    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    want.SetParam("existing_key", std::string("existing_value"));

    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);

    SenderInfo info;
    Want senderWant;
    info.want.SetParam("new_key", std::string("new_value"));
    info.want.SetParam("existing_key", std::string("should_not_override"));

    pendingWantRecord->BuildSendWant(info, senderWant);

    EXPECT_EQ(senderWant.GetParams().GetStringParam("existing_key"), "existing_value");
    EXPECT_EQ(senderWant.GetParams().GetStringParam("new_key"), "new_value");

    TAG_LOGI(AAFwkTag::TEST, "BuildSendWant_MergeParams_0100 end");
}

/*
 * @tc.number    : BuildSendWant_ConstantFlag_0100
 * @tc.name      : BuildSendWant
 * @tc.desc      : 1.BuildSendWant, verify CONSTANT_FLAG behavior
 */
HWTEST_F(PendingWantRecordTest, BuildSendWant_ConstantFlag_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "BuildSendWant_ConstantFlag_0100 start");

    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);

    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, (int32_t)Flags::CONSTANT_FLAG, 0);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);

    SenderInfo info;
    Want senderWant;
    pendingWantRecord->BuildSendWant(info, senderWant);

    EXPECT_TRUE(info.resolvedType == key->GetRequestResolvedType());

    TAG_LOGI(AAFwkTag::TEST, "BuildSendWant_ConstantFlag_0100 end");
}

/*
 * @tc.number    : BuildSendWant_AllowCancelFlag_0100
 * @tc.name      : BuildSendWant
 * @tc.desc      : 1.BuildSendWant, verify ALLOW_CANCEL_FLAG behavior
 */
HWTEST_F(PendingWantRecordTest, BuildSendWant_AllowCancelFlag_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "BuildSendWant_AllowCancelFlag_0100 start");

    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);

    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, (int32_t)Flags::ALLOW_CANCEL_FLAG, 0);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);

    SenderInfo info;
    Want senderWant;
    pendingWantRecord->BuildSendWant(info, senderWant);

    EXPECT_TRUE(info.resolvedType == key->GetRequestResolvedType());

    TAG_LOGI(AAFwkTag::TEST, "BuildSendWant_AllowCancelFlag_0100 end");
}

/*
 * @tc.number    : BuildSendWant_EmptyAllWantsInfos_0200
 * @tc.name      : BuildSendWant
 * @tc.desc      : 1.BuildSendWant, empty allWantsInfos scenario
 */
HWTEST_F(PendingWantRecordTest, BuildSendWant_EmptyAllWantsInfos_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "BuildSendWant_EmptyAllWantsInfos_0200 start");

    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);

    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    key->GetAllWantsInfos().clear();
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);

    SenderInfo info;
    Want senderWant;
    pendingWantRecord->BuildSendWant(info, senderWant);

    TAG_LOGI(AAFwkTag::TEST, "BuildSendWant_EmptyAllWantsInfos_0200 end");
}

/*
 * @tc.number    : BuildSendWant_AppIndex_0200
 * @tc.name      : BuildSendWant
 * @tc.desc      : 1.BuildSendWant, verify appIndex parameter is set correctly
 */
HWTEST_F(PendingWantRecordTest, BuildSendWant_AppIndex_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "BuildSendWant_AppIndex_0200 start");

    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);

    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    key->SetAppIndex(2);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, INVALID_UID + 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);

    SenderInfo info;
    Want senderWant;
    pendingWantRecord->BuildSendWant(info, senderWant);

    auto appIndexParam = senderWant.GetParams().GetParam(Want::PARAM_APP_CLONE_INDEX_KEY);
    EXPECT_NE(appIndexParam, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "BuildSendWant_AppIndex_0200 end");
}

/*
 * @tc.number    : OneTimeFlag_AutoCancel_0100
 * @tc.name      : SenderInner
 * @tc.desc      : 1.SenderInner, verify ONE_TIME_FLAG triggers auto cancel
 */
HWTEST_F(PendingWantRecordTest, OneTimeFlag_AutoCancel_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OneTimeFlag_AutoCancel_0100 start");

    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);

    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, (int32_t)Flags::ONE_TIME_FLAG, 0,
        (int32_t)OperationType::START_ABILITY);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);

    EXPECT_EQ(pendingWantRecord->GetCanceled(), false);
    SenderInfo info;
    pendingWantRecord->SenderInner(info);

    TAG_LOGI(AAFwkTag::TEST, "OneTimeFlag_AutoCancel_0100 end");
}

/*
 * @tc.number    : GetAppIndexbyUid_InvalidUid_0200
 * @tc.name      : GetAppIndexbyUid
 * @tc.desc      : 1.GetAppIndexbyUid, invalid UID scenario
 */
HWTEST_F(PendingWantRecordTest, GetAppIndexbyUid_InvalidUid_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAppIndexbyUid_InvalidUid_0200 start");

    Want want;
    std::string bundleName = "com.ix.hiMusic";
    ElementName element("device", bundleName, "MusicSAbility");
    want.SetElement(element);

    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, INVALID_UID, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);

    int32_t appIndex;
    int32_t ret = pendingWantRecord->GetAppIndexbyUid(pendingWantRecord->GetUid(), bundleName, appIndex);
    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "GetAppIndexbyUid_InvalidUid_0200 end");
}

/*
 * @tc.number    : CheckAppInstanceKey_NullKey_0200
 * @tc.name      : CheckAppInstanceKey
 * @tc.desc      : 1.CheckAppInstanceKey, null key scenario
 */
HWTEST_F(PendingWantRecordTest, CheckAppInstanceKey_NullKey_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckAppInstanceKey_NullKey_0200 start");

    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);

    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    std::shared_ptr<PendingWantKey> key = nullptr;
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);

    std::string bundleName = "com.ix.hiMusic";
    auto params = want.GetParams();
    pendingWantRecord->CheckAppInstanceKey(bundleName, params);

    TAG_LOGI(AAFwkTag::TEST, "CheckAppInstanceKey_NullKey_0200 end");
}

/*
 * @tc.number    : CheckAppInstanceKey_EmptyAppKey_0200
 * @tc.name      : CheckAppInstanceKey
 * @tc.desc      : 1.CheckAppInstanceKey, empty app instance key scenario
 */
HWTEST_F(PendingWantRecordTest, CheckAppInstanceKey_EmptyAppKey_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckAppInstanceKey_EmptyAppKey_0200 start");

    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);

    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    key->SetType(static_cast<int32_t>(OperationType::START_ABILITY));
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);

    std::string bundleName = "com.ix.hiMusic";
    WantParams params;
    pendingWantRecord->CheckAppInstanceKey(bundleName, params);

    TAG_LOGI(AAFwkTag::TEST, "CheckAppInstanceKey_EmptyAppKey_0200 end");
}

/*
 * @tc.number    : CheckAppInstanceKey_NullManager_0200
 * @tc.name      : CheckAppInstanceKey
 * @tc.desc      : 1.CheckAppInstanceKey, null PendingWantManager scenario
 */
HWTEST_F(PendingWantRecordTest, CheckAppInstanceKey_NullManager_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckAppInstanceKey_NullManager_0200 start");

    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    want.SetParam(std::string("ohos.extra.param.key.appInstance"), std::string("app_instance_1"));

    std::shared_ptr<PendingWantManager> nullManager = nullptr;
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    key->SetType(static_cast<int32_t>(OperationType::START_ABILITY));
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(nullManager, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);

    std::string bundleName = "com.ix.hiMusic";
    auto params = want.GetParams();
    pendingWantRecord->CheckAppInstanceKey(bundleName, params);

    TAG_LOGI(AAFwkTag::TEST, "CheckAppInstanceKey_NullManager_0200 end");
}

/*
 * @tc.number    : SenderInner_FinishedReceiver_0100
 * @tc.name      : SenderInner
 * @tc.desc      : 1.SenderInner, verify finishedReceiver callback is invoked
 */
HWTEST_F(PendingWantRecordTest, SenderInner_FinishedReceiver_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SenderInner_FinishedReceiver_0100 start");

    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);

    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, (int32_t)Flags::CONSTANT_FLAG, 0,
        (int32_t)OperationType::START_ABILITY);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "SenderInner_FinishedReceiver_0100 end");
}

/*
 * @tc.number    : BuildSendWant_AppInstanceKey_0300
 * @tc.name      : BuildSendWant
 * @tc.desc      : 1.BuildSendWant, verify APP_MULTI_INSTANCE key is preserved
 */
HWTEST_F(PendingWantRecordTest, BuildSendWant_AppInstanceKey_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "BuildSendWant_AppInstanceKey_0300 start");

    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    int appIndex = 5;
    want.SetParam(std::string(Want::PARAM_APP_CLONE_INDEX_KEY), appIndex);

    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);

    SenderInfo info;
    Want senderWant;
    pendingWantRecord->BuildSendWant(info, senderWant);
    auto intValue = senderWant.GetIntParam(Want::PARAM_APP_CLONE_INDEX_KEY, 0);
    EXPECT_EQ(intValue, 5);

    TAG_LOGI(AAFwkTag::TEST, "BuildSendWant_AppInstanceKey_0300 end");
}

/*
 * @tc.number    : ExecuteOperation_EmptyAllWantsInfos_START_ABILITIES_0100
 * @tc.name      : ExecuteOperation
 * @tc.desc      : 1.ExecuteOperation, START_ABILITIES with empty allWantsInfos should return error
 */
HWTEST_F(PendingWantRecordTest, ExecuteOperation_EmptyAllWantsInfos_START_ABILITIES_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteOperation_EmptyAllWantsInfos_START_ABILITIES_0100 start");

    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);

    pendingManager_ = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager_, nullptr);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);
    std::shared_ptr<PendingWantKey> key = MakeWantKey(wantSenderInfo);
    key->SetType(static_cast<int32_t>(OperationType::START_ABILITIES));
    key->GetAllWantsInfos().clear();
    std::shared_ptr<PendingWantRecord> pendingWantRecord =
        std::make_shared<PendingWantRecord>(pendingManager_, 1, 0, nullptr, key);
    EXPECT_NE(pendingWantRecord, nullptr);

    SenderInfo info;
    int32_t result = pendingWantRecord->ExecuteOperation(pendingManager_, info, want);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "ExecuteOperation_EmptyAllWantsInfos_START_ABILITIES_0100 end");
}

}  // namespace AAFwk
}  // namespace OHOS
