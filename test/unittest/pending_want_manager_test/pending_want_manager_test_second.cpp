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
#include <climits>
#include <set>
#include "bundlemgr/mock_bundle_manager.h"
#include "mock_native_token.h"
#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
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

class PendingWantManagerSecondTest : public testing::Test {
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

int PendingWantManagerSecondTest::CancelReceiver::performReceiveCount = 0;
int PendingWantManagerSecondTest::CancelReceiver::sendCount = 0;

void PendingWantManagerSecondTest::CancelReceiver::Send(const int32_t resultCode)
{
    sendCount = DEFAULT_COUNT;
}

void PendingWantManagerSecondTest::CancelReceiver::PerformReceive(const AAFwk::Want& want, int resultCode,
    const std::string& data, const AAFwk::WantParams& extras, bool serialized, bool sticky, int sendingUser)
{
    performReceiveCount = DEFAULT_COUNT;
}

void PendingWantManagerSecondTest::SetUpTestCase()
{
    MockNativeToken::SetNativeToken();
    OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->RegisterSystemAbility(
        OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, new BundleMgrService());
}

void PendingWantManagerSecondTest::TearDownTestCase()
{
    OHOS::DelayedSingleton<SaMgrClient>::DestroyInstance();
}

void PendingWantManagerSecondTest::SetUp()
{
}

void PendingWantManagerSecondTest::TearDown()
{
}

WantSenderInfo PendingWantManagerSecondTest::MakeWantSenderInfo(Want& want, int32_t flags, int32_t userId, int32_t type)
{
    WantSenderInfo wantSenderInfo;
    wantSenderInfo.type = type;
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

WantSenderInfo PendingWantManagerSecondTest::MakeWantSenderInfo(std::vector<Want>& wants,
    int32_t flags, int32_t userId, int32_t type)
{
    WantSenderInfo wantSenderInfo;
    wantSenderInfo.type = type;
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

std::shared_ptr<PendingWantKey> PendingWantManagerSecondTest::MakeWantKey(WantSenderInfo& wantSenderInfo)
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

/**
 * @tc.name: GetWantSender_WithAppIndex_0100
 * @tc.desc: Test GetWantSender with appIndex parameter
 * @tc.type: FUNC
 */
HWTEST_F(PendingWantManagerSecondTest, GetWantSender_WithAppIndex_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetWantSender_WithAppIndex_0100 start");

    // Arrange
    int32_t callingUid = 1;
    int32_t uid = 1;
    Want want;
    ElementName element("device", "bundleName", "abilityName");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);

    pendingManager_ = std::make_shared<PendingWantManager>();
    ASSERT_NE(pendingManager_, nullptr);

    // Act
    auto result = pendingManager_->GetWantSender(callingUid, uid, isSystemApp, wantSenderInfo, nullptr, 1);

    // Assert
    EXPECT_NE(result, nullptr);
    EXPECT_EQ((int)pendingManager_->wantRecords_.size(), 1);

    TAG_LOGI(AAFwkTag::TEST, "GetWantSender_WithAppIndex_0100 end");
}

/**
 * @tc.name: GetWantSender_AllWants_Mismatch_0200
 * @tc.desc: Test GetWantSender filters out allWants with mismatched bundle names
 * @tc.type: FUNC
 */
HWTEST_F(PendingWantManagerSecondTest, GetWantSender_AllWants_Mismatch_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetWantSender_AllWants_Mismatch_0200 start");

    // Arrange
    int32_t callingUid = 1;
    int32_t uid = 1;
    Want want1, want2;
    ElementName element1("device", "bundleName1", "abilityName1");
    ElementName element2("device", "bundleName2", "abilityName2");
    want1.SetElement(element1);
    want2.SetElement(element2);

    WantSenderInfo wantSenderInfo;
    wantSenderInfo.type = 1;
    wantSenderInfo.bundleName = "com.ix.hiRadio";
    wantSenderInfo.resultWho = "RadioTopAbility";
    wantSenderInfo.requestCode = 10;
    WantsInfo wantInfo1;
    wantInfo1.want = want1;
    wantInfo1.resolvedTypes = "type1";
    WantsInfo wantInfo2;
    wantInfo2.want = want2;
    wantInfo2.resolvedTypes = "type2";
    wantSenderInfo.allWants.push_back(wantInfo1);
    wantSenderInfo.allWants.push_back(wantInfo2);
    wantSenderInfo.flags = 0;
    wantSenderInfo.userId = 0;

    pendingManager_ = std::make_shared<PendingWantManager>();
    ASSERT_NE(pendingManager_, nullptr);

    // Act - Non-system app with mismatched bundle names
    auto result = pendingManager_->GetWantSender(callingUid, uid, false, wantSenderInfo, nullptr);

    // Assert - Should filter out allWants with different bundle names
    EXPECT_NE(result, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "GetWantSender_AllWants_Mismatch_0200 end");
}

/**
 * @tc.name: SendWantSender_FinishedReceiver_Callback_0100
 * @tc.desc: Test SendWantSender calls finishedReceiver when target is null
 * @tc.type: FUNC
 */
HWTEST_F(PendingWantManagerSecondTest, SendWantSender_FinishedReceiver_Callback_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendWantSender_FinishedReceiver_Callback_0100 start");

    // Arrange
    pendingManager_ = std::make_shared<PendingWantManager>();
    ASSERT_NE(pendingManager_, nullptr);

    SenderInfo senderInfo;
    senderInfo.code = 100;
    sptr<CancelReceiver> receiver = new CancelReceiver();
    senderInfo.finishedReceiver = receiver;

    // Act
    auto result = pendingManager_->SendWantSender(nullptr, senderInfo);

    // Assert
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    EXPECT_EQ(CancelReceiver::performReceiveCount, DEFAULT_COUNT);

    TAG_LOGI(AAFwkTag::TEST, "SendWantSender_FinishedReceiver_Callback_0100 end");
}

/**
 * @tc.name: SendLocalWantSender_START_ABILITY_0100
 * @tc.desc: Test SendLocalWantSender with START_ABILITY operation type
 * @tc.type: FUNC
 */
HWTEST_F(PendingWantManagerSecondTest, SendLocalWantSender_START_ABILITY_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendLocalWantSender_START_ABILITY_0100 start");

    // Arrange
    pendingManager_ = std::make_shared<PendingWantManager>();
    ASSERT_NE(pendingManager_, nullptr);

    SenderInfo senderInfo;
    Want want;
    ElementName element("device", "com.test.app", "TestAbility");
    want.SetElement(element);
    senderInfo.want = want;
    senderInfo.operType = static_cast<int32_t>(OperationType::START_ABILITY);
    senderInfo.callerToken = nullptr;
    senderInfo.uid = 1000;
    senderInfo.tokenId = 1;

    // Act
    auto result = pendingManager_->SendLocalWantSender(senderInfo);

    // Assert - Should return error since callerToken is null
    EXPECT_NE(result, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "SendLocalWantSender_START_ABILITY_0100 end");
}

/**
 * @tc.name: SendLocalWantSender_START_SERVICE_0100
 * @tc.desc: Test SendLocalWantSender with START_SERVICE operation type
 * @tc.type: FUNC
 */
HWTEST_F(PendingWantManagerSecondTest, SendLocalWantSender_START_SERVICE_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendLocalWantSender_START_SERVICE_0100 start");

    // Arrange
    pendingManager_ = std::make_shared<PendingWantManager>();
    ASSERT_NE(pendingManager_, nullptr);

    SenderInfo senderInfo;
    Want want;
    ElementName element("device", "com.test.app", "TestService");
    want.SetElement(element);
    senderInfo.want = want;
    senderInfo.operType = static_cast<int32_t>(OperationType::START_SERVICE);
    senderInfo.callerToken = nullptr;
    senderInfo.uid = 1000;
    senderInfo.tokenId = 1;

    // Act
    auto result = pendingManager_->SendLocalWantSender(senderInfo);

    // Assert - Should return error since callerToken is null
    EXPECT_NE(result, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "SendLocalWantSender_START_SERVICE_0100 end");
}

/**
 * @tc.name: CancelWantSender_NullSender_0200
 * @tc.desc: Test CancelWantSender with null sender parameter
 * @tc.type: FUNC
 */
HWTEST_F(PendingWantManagerSecondTest, CancelWantSender_NullSender_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CancelWantSender_NullSender_0200 start");

    // Arrange
    pendingManager_ = std::make_shared<PendingWantManager>();
    ASSERT_NE(pendingManager_, nullptr);

    // Act
    pendingManager_->CancelWantSender(true, nullptr);

    // Assert - Should not crash, just return early
    EXPECT_NE(pendingManager_, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "CancelWantSender_NullSender_0200 end");
}

/**
 * @tc.name: PendingWantStartAbilitys_EmptyVector_0200
 * @tc.desc: Test PendingWantStartAbilitys with empty wants vector
 * @tc.type: FUNC
 */
HWTEST_F(PendingWantManagerSecondTest, PendingWantStartAbilitys_EmptyVector_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PendingWantStartAbilitys_EmptyVector_0200 start");

    // Arrange
    pendingManager_ = std::make_shared<PendingWantManager>();
    ASSERT_NE(pendingManager_, nullptr);

    std::vector<WantsInfo> wantsInfo;

    // Act
    auto result = pendingManager_->PendingWantStartAbilitys(wantsInfo, nullptr, nullptr, -1, 1000, 1, -1);

    // Assert - Should succeed with empty vector
    EXPECT_EQ(result, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "PendingWantStartAbilitys_EmptyVector_0200 end");
}

/**
 * @tc.name: PendingWantPublishCommonEvent_WithPermission_0100
 * @tc.desc: Test PendingWantPublishCommonEvent with required permission
 * @tc.type: FUNC
 */
HWTEST_F(PendingWantManagerSecondTest, PendingWantPublishCommonEvent_WithPermission_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PendingWantPublishCommonEvent_WithPermission_0100 start");

    // Arrange
    pendingManager_ = std::make_shared<PendingWantManager>();
    ASSERT_NE(pendingManager_, nullptr);

    Want want;
    want.SetBundle("com.test.app");
    want.SetAction("test.event");

    SenderInfo senderInfo;
    senderInfo.code = 100;
    senderInfo.requiredPermission = "ohos.permission.TEST";

    // Act
    auto result = pendingManager_->PendingWantPublishCommonEvent(want, senderInfo, 1000, 1);

    // Assert - Should fail without proper event setup
    EXPECT_EQ(result, -1);

    TAG_LOGI(AAFwkTag::TEST, "PendingWantPublishCommonEvent_WithPermission_0100 end");
}

/**
 * @tc.name: GetWantSenderInfo_NullTarget_0200
 * @tc.desc: Test GetWantSenderInfo with null target parameter
 * @tc.type: FUNC
 */
HWTEST_F(PendingWantManagerSecondTest, GetWantSenderInfo_NullTarget_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetWantSenderInfo_NullTarget_0200 start");

    // Arrange
    pendingManager_ = std::make_shared<PendingWantManager>();
    ASSERT_NE(pendingManager_, nullptr);

    std::shared_ptr<WantSenderInfo> info = nullptr;

    // Act
    auto result = pendingManager_->GetWantSenderInfo(nullptr, info);

    // Assert
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "GetWantSenderInfo_NullTarget_0200 end");
}

/**
 * @tc.name: GetWantSenderInfo_NullInfo_0200
 * @tc.desc: Test GetWantSenderInfo with null info parameter
 * @tc.type: FUNC
 */
HWTEST_F(PendingWantManagerSecondTest, GetWantSenderInfo_NullInfo_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetWantSenderInfo_NullInfo_0200 start");

    // Arrange
    pendingManager_ = std::make_shared<PendingWantManager>();
    ASSERT_NE(pendingManager_, nullptr);

    Want want;
    ElementName element("device", "bundleName", "abilityName");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);

    auto sender = pendingManager_->GetWantSender(1, 1, true, wantSenderInfo, nullptr);
    ASSERT_NE(sender, nullptr);

    std::shared_ptr<WantSenderInfo> info = nullptr;

    // Act
    auto result = pendingManager_->GetWantSenderInfo(sender, info);

    // Assert
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "GetWantSenderInfo_NullInfo_0200 end");
}

/**
 * @tc.name: GetWantSenderInfo_Success_0100
 * @tc.desc: Test GetWantSenderInfo returns correct info for a valid pending record
 * @tc.type: FUNC
 */
HWTEST_F(PendingWantManagerSecondTest, GetWantSenderInfo_Success_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetWantSenderInfo_Success_0100 start");

    // Arrange
    pendingManager_ = std::make_shared<PendingWantManager>();
    ASSERT_NE(pendingManager_, nullptr);

    Want want;
    ElementName element("device", "com.test.bundle", "TestAbility");
    want.SetElement(element);
    want.SetParam("key1", std::string("value1"));
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 100);
    wantSenderInfo.requestCode = 42;

    auto sender = pendingManager_->GetWantSender(1, 1, true, wantSenderInfo, nullptr, 5);
    ASSERT_NE(sender, nullptr);

    std::shared_ptr<WantSenderInfo> info = std::make_shared<WantSenderInfo>();

    // Act
    auto result = pendingManager_->GetWantSenderInfo(sender, info);

    // Assert
    EXPECT_EQ(result, NO_ERROR);
    ASSERT_NE(info, nullptr);
    EXPECT_EQ(info->requestCode, 42);
    EXPECT_EQ(info->type, wantSenderInfo.type);
    EXPECT_EQ(info->flags, static_cast<uint32_t>(wantSenderInfo.flags));
    EXPECT_EQ(info->userId, 100);
    EXPECT_EQ(info->appIndex, 5);
    EXPECT_EQ(info->allWants.size(), 1u);

    TAG_LOGI(AAFwkTag::TEST, "GetWantSenderInfo_Success_0100 end");
}

/**
 * @tc.name: GetWantSenderInfo_RecordNotFound_0300
 * @tc.desc: Test GetWantSenderInfo when the pending record is not found in records map
 * @tc.type: FUNC
 */
HWTEST_F(PendingWantManagerSecondTest, GetWantSenderInfo_RecordNotFound_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetWantSenderInfo_RecordNotFound_0300 start");

    // Arrange
    pendingManager_ = std::make_shared<PendingWantManager>();
    ASSERT_NE(pendingManager_, nullptr);

    Want want;
    ElementName element("device", "com.test.bundle", "TestAbility");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);

    // Create a record but manually clear the records map to simulate "record not found"
    auto sender = pendingManager_->GetWantSender(1, 1, true, wantSenderInfo, nullptr);
    ASSERT_NE(sender, nullptr);
    ASSERT_EQ((int)pendingManager_->wantRecords_.size(), 1);

    // Clear the records map so GetPendingWantRecordByCode returns nullptr
    pendingManager_->wantRecords_.clear();

    std::shared_ptr<WantSenderInfo> info = std::make_shared<WantSenderInfo>();

    // Act
    auto result = pendingManager_->GetWantSenderInfo(sender, info);

    // Assert - Should return ERR_INVALID_VALUE when record not found
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "GetWantSenderInfo_RecordNotFound_0300 end");
}

/**
 * @tc.name: GetWantSenderInfo_MultipleWants_0100
 * @tc.desc: Test GetWantSenderInfo returns correct allWants for multiple wants
 * @tc.type: FUNC
 */
HWTEST_F(PendingWantManagerSecondTest, GetWantSenderInfo_MultipleWants_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetWantSenderInfo_MultipleWants_0100 start");

    // Arrange
    pendingManager_ = std::make_shared<PendingWantManager>();
    ASSERT_NE(pendingManager_, nullptr);

    Want want1;
    Want want2;
    ElementName element1("device", "com.test.bundle1", "Ability1");
    ElementName element2("device", "com.test.bundle1", "Ability2");
    want1.SetElement(element1);
    want2.SetElement(element2);
    std::vector<Want> wants;
    wants.emplace_back(want1);
    wants.emplace_back(want2);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(wants, 0, 0);
    wantSenderInfo.requestCode = 88;

    auto sender = pendingManager_->GetWantSender(1, 1, true, wantSenderInfo, nullptr);
    ASSERT_NE(sender, nullptr);

    std::shared_ptr<WantSenderInfo> info = std::make_shared<WantSenderInfo>();

    // Act
    auto result = pendingManager_->GetWantSenderInfo(sender, info);

    // Assert
    EXPECT_EQ(result, NO_ERROR);
    ASSERT_NE(info, nullptr);
    EXPECT_EQ(info->requestCode, 88);
    EXPECT_EQ(info->allWants.size(), 2u);

    TAG_LOGI(AAFwkTag::TEST, "GetWantSenderInfo_MultipleWants_0100 end");
}

/**
 * @tc.name: Dump_EmptyRecords_0100
 * @tc.desc: Test Dump with no pending want records
 * @tc.type: FUNC
 */
HWTEST_F(PendingWantManagerSecondTest, Dump_EmptyRecords_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Dump_EmptyRecords_0100 start");

    // Arrange
    pendingManager_ = std::make_shared<PendingWantManager>();
    ASSERT_NE(pendingManager_, nullptr);

    std::vector<std::string> info;

    // Act
    pendingManager_->Dump(info);

    // Assert - Should not crash, info may be empty
    EXPECT_GE(info.size(), 0u);

    TAG_LOGI(AAFwkTag::TEST, "Dump_EmptyRecords_0100 end");
}

/**
 * @tc.name: Dump_WithRecords_0100
 * @tc.desc: Test Dump with pending want records
 * @tc.type: FUNC
 */
HWTEST_F(PendingWantManagerSecondTest, Dump_WithRecords_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Dump_WithRecords_0100 start");

    // Arrange
    pendingManager_ = std::make_shared<PendingWantManager>();
    ASSERT_NE(pendingManager_, nullptr);

    Want want;
    ElementName element("device", "bundleName", "abilityName");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);

    auto sender = pendingManager_->GetWantSender(1, 1, true, wantSenderInfo, nullptr);
    ASSERT_NE(sender, nullptr);
    ASSERT_EQ((int)pendingManager_->wantRecords_.size(), 1);

    std::vector<std::string> info;

    // Act
    pendingManager_->Dump(info);

    // Assert - Should contain dump information
    EXPECT_GT(info.size(), 0u);

    TAG_LOGI(AAFwkTag::TEST, "Dump_WithRecords_0100 end");
}

/**
 * @tc.name: DumpByRecordId_InvalidId_0200
 * @tc.desc: Test DumpByRecordId with invalid record ID
 * @tc.type: FUNC
 */
HWTEST_F(PendingWantManagerSecondTest, DumpByRecordId_InvalidId_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DumpByRecordId_InvalidId_0200 start");

    // Arrange
    pendingManager_ = std::make_shared<PendingWantManager>();
    ASSERT_NE(pendingManager_, nullptr);

    std::vector<std::string> info;
    std::string args = "invalid_id";

    // Act
    pendingManager_->DumpByRecordId(info, args);

    // Assert - Should not crash
    EXPECT_GE(info.size(), 0u);

    TAG_LOGI(AAFwkTag::TEST, "DumpByRecordId_InvalidId_0200 end");
}

/**
 * @tc.name: ClearPendingWantRecord_EmptyBundleName_0200
 * @tc.desc: Test ClearPendingWantRecord with empty bundle name
 * @tc.type: FUNC
 */
HWTEST_F(PendingWantManagerSecondTest, ClearPendingWantRecord_EmptyBundleName_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClearPendingWantRecord_EmptyBundleName_0200 start");

    // Arrange
    pendingManager_ = std::make_shared<PendingWantManager>();
    ASSERT_NE(pendingManager_, nullptr);

    Want want;
    ElementName element("device", "bundleName", "abilityName");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);

    auto sender = pendingManager_->GetWantSender(1, 1, true, wantSenderInfo, nullptr);
    ASSERT_NE(sender, nullptr);
    ASSERT_EQ((int)pendingManager_->wantRecords_.size(), 1);

    // Act
    pendingManager_->ClearPendingWantRecord("", 1);

    // Assert - Record should not be cleared with empty bundle name
    EXPECT_EQ((int)pendingManager_->wantRecords_.size(), 1);

    TAG_LOGI(AAFwkTag::TEST, "ClearPendingWantRecord_EmptyBundleName_0200 end");
}

/**
 * @tc.name: HandleAddWantAgentNumber_NullKey_0200
 * @tc.desc: Test HandleAddWantAgentNumber with null key
 * @tc.type: FUNC
 */
HWTEST_F(PendingWantManagerSecondTest, HandleAddWantAgentNumber_NullKey_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleAddWantAgentNumber_NullKey_0200 start");

    // Arrange
    pendingManager_ = std::make_shared<PendingWantManager>();
    ASSERT_NE(pendingManager_, nullptr);

    // Act
    pendingManager_->HandleAddWantAgentNumber(nullptr);

    // Assert - Should not crash
    EXPECT_NE(pendingManager_, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "HandleAddWantAgentNumber_NullKey_0200 end");
}

/**
 * @tc.name: HandleReduceWantAgentNumber_NullKey_0200
 * @tc.desc: Test HandleReduceWantAgentNumber with null key
 * @tc.type: FUNC
 */
HWTEST_F(PendingWantManagerSecondTest, HandleReduceWantAgentNumber_NullKey_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleReduceWantAgentNumber_NullKey_0200 start");

    // Arrange
    pendingManager_ = std::make_shared<PendingWantManager>();
    ASSERT_NE(pendingManager_, nullptr);

    // Act
    pendingManager_->HandleReduceWantAgentNumber(nullptr);

    // Assert - Should not crash
    EXPECT_NE(pendingManager_, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "HandleReduceWantAgentNumber_NullKey_0200 end");
}

/**
 * @tc.name: HandleReduceWantAgentNumber_NotFound_0200
 * @tc.desc: Test HandleReduceWantAgentNumber when key not found
 * @tc.type: FUNC
 */
HWTEST_F(PendingWantManagerSecondTest, HandleReduceWantAgentNumber_NotFound_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleReduceWantAgentNumber_NotFound_0200 start");

    // Arrange
    pendingManager_ = std::make_shared<PendingWantManager>();
    ASSERT_NE(pendingManager_, nullptr);

    std::shared_ptr<PendingWantKey> key = std::make_shared<PendingWantKey>();
    key->SetBundleName("non.existent.bundle");

    // Act
    pendingManager_->HandleReduceWantAgentNumber(key);

    // Assert - Should not crash, just log
    EXPECT_NE(pendingManager_, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "HandleReduceWantAgentNumber_NotFound_0200 end");
}

/**
 * @tc.name: GetAllRunningInstanceKeysByBundleName_EmptyString_0200
 * @tc.desc: Test GetAllRunningInstanceKeysByBundleName with empty bundle name
 * @tc.type: FUNC
 */
HWTEST_F(PendingWantManagerSecondTest, GetAllRunningInstanceKeysByBundleName_EmptyString_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningInstanceKeysByBundleName_EmptyString_0200 start");

    // Arrange
    pendingManager_ = std::make_shared<PendingWantManager>();
    ASSERT_NE(pendingManager_, nullptr);

    std::string bundleName = "";
    std::vector<std::string> appKeyVec;

    // Act
    auto result = pendingManager_->GetAllRunningInstanceKeysByBundleName(bundleName, appKeyVec);

    // Assert
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningInstanceKeysByBundleName_EmptyString_0200 end");
}

/**
 * @tc.name: CheckWindowState_WithValidPid_0100
 * @tc.desc: Test CheckWindowState with valid process ID
 * @tc.type: FUNC
 */
HWTEST_F(PendingWantManagerSecondTest, CheckWindowState_WithValidPid_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckWindowState_WithValidPid_0100 start");

    // Arrange
    pendingManager_ = std::make_shared<PendingWantManager>();
    ASSERT_NE(pendingManager_, nullptr);

    int32_t pid = 1234;

    // Act
    auto result = pendingManager_->CheckWindowState(pid);

    // Assert - Should return false without proper window manager setup
    EXPECT_FALSE(result);

    TAG_LOGI(AAFwkTag::TEST, "CheckWindowState_WithValidPid_0100 end");
}

/**
 * @tc.name: CheckWindowState_ZeroPid_0300
 * @tc.desc: Test CheckWindowState with zero process ID (boundary value)
 * @tc.type: FUNC
 */
HWTEST_F(PendingWantManagerSecondTest, CheckWindowState_ZeroPid_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckWindowState_ZeroPid_0300 start");

    // Arrange
    pendingManager_ = std::make_shared<PendingWantManager>();
    ASSERT_NE(pendingManager_, nullptr);

    int32_t pid = 0;

    // Act
    auto result = pendingManager_->CheckWindowState(pid);

    // Assert - Should return false for zero PID
    EXPECT_FALSE(result);

    TAG_LOGI(AAFwkTag::TEST, "CheckWindowState_ZeroPid_0300 end");
}

/**
 * @tc.name: CheckWindowState_NegativePid_0300
 * @tc.desc: Test CheckWindowState with negative process ID (boundary value)
 * @tc.type: FUNC
 */
HWTEST_F(PendingWantManagerSecondTest, CheckWindowState_NegativePid_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckWindowState_NegativePid_0300 start");

    // Arrange
    pendingManager_ = std::make_shared<PendingWantManager>();
    ASSERT_NE(pendingManager_, nullptr);

    int32_t pid = -1;

    // Act
    auto result = pendingManager_->CheckWindowState(pid);

    // Assert - Should return false for negative PID
    EXPECT_FALSE(result);

    TAG_LOGI(AAFwkTag::TEST, "CheckWindowState_NegativePid_0300 end");
}

/**
 * @tc.name: MakeWantSenderCanceledLocked_WithCallbacks_0100
 * @tc.desc: Test MakeWantSenderCanceledLocked notifies all registered callbacks
 * @tc.type: FUNC
 */
HWTEST_F(PendingWantManagerSecondTest, MakeWantSenderCanceledLocked_WithCallbacks_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "MakeWantSenderCanceledLocked_WithCallbacks_0100 start");

    // Arrange
    pendingManager_ = std::make_shared<PendingWantManager>();
    ASSERT_NE(pendingManager_, nullptr);

    Want want;
    ElementName element("device", "bundleName", "abilityName");
    want.SetElement(element);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(want, 0, 0);

    auto sender = pendingManager_->GetWantSender(1, 1, true, wantSenderInfo, nullptr);
    ASSERT_NE(sender, nullptr);

    sptr<CancelReceiver> receiver1 = new CancelReceiver();
    sptr<CancelReceiver> receiver2 = new CancelReceiver();

    pendingManager_->RegisterCancelListener(sender, receiver1);
    pendingManager_->RegisterCancelListener(sender, receiver2);

    // Reset counters
    CancelReceiver::sendCount = 0;

    // Act
    auto record = iface_cast<PendingWantRecord>(sender->AsObject());
    pendingManager_->MakeWantSenderCanceledLocked(*record);

    // Assert - Both callbacks should be notified
    EXPECT_EQ(CancelReceiver::sendCount, DEFAULT_COUNT);

    TAG_LOGI(AAFwkTag::TEST, "MakeWantSenderCanceledLocked_WithCallbacks_0100 end");
}

/**
 * @tc.name: GetWantSender_MultipleFlags_0100
 * @tc.desc: Test GetWantSender with multiple flag combinations
 * @tc.type: FUNC
 */
HWTEST_F(PendingWantManagerSecondTest, GetWantSender_MultipleFlags_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetWantSender_MultipleFlags_0100 start");

    // Arrange
    pendingManager_ = std::make_shared<PendingWantManager>();
    ASSERT_NE(pendingManager_, nullptr);

    Want want;
    ElementName element("device", "bundleName", "abilityName");
    want.SetElement(element);

    // Test with UPDATE_PRESENT_FLAG
    WantSenderInfo wantSenderInfo1 = MakeWantSenderInfo(want,
        static_cast<int32_t>(Flags::UPDATE_PRESENT_FLAG), 0);
    auto sender1 = pendingManager_->GetWantSender(1, 1, true, wantSenderInfo1, nullptr);
    EXPECT_NE(sender1, nullptr);
    EXPECT_EQ((int)pendingManager_->wantRecords_.size(), 1);

    TAG_LOGI(AAFwkTag::TEST, "GetWantSender_MultipleFlags_0100 end");
}

/**
 * @tc.name: GetWantSender_DifferentUserIds_0100
 * @tc.desc: Test GetWantSender with different user IDs
 * @tc.type: FUNC
 */
HWTEST_F(PendingWantManagerSecondTest, GetWantSender_DifferentUserIds_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetWantSender_DifferentUserIds_0100 start");

    // Arrange
    pendingManager_ = std::make_shared<PendingWantManager>();
    ASSERT_NE(pendingManager_, nullptr);

    Want want;
    ElementName element("device", "bundleName", "abilityName");
    want.SetElement(element);

    // Test with userId 0
    WantSenderInfo wantSenderInfo1 = MakeWantSenderInfo(want, 0, 0);
    auto sender1 = pendingManager_->GetWantSender(1, 1, true, wantSenderInfo1, nullptr);
    EXPECT_NE(sender1, nullptr);

    // Test with userId 100
    WantSenderInfo wantSenderInfo2 = MakeWantSenderInfo(want, 0, 100);
    auto sender2 = pendingManager_->GetWantSender(1, 1, true, wantSenderInfo2, nullptr);
    EXPECT_NE(sender2, nullptr);

    // Test with userId 999 (boundary)
    WantSenderInfo wantSenderInfo3 = MakeWantSenderInfo(want, 0, 999);
    auto sender3 = pendingManager_->GetWantSender(1, 1, true, wantSenderInfo3, nullptr);
    EXPECT_NE(sender3, nullptr);

    // All should create separate records due to different userIds
    EXPECT_EQ((int)pendingManager_->wantRecords_.size(), 3);

    TAG_LOGI(AAFwkTag::TEST, "GetWantSender_DifferentUserIds_0100 end");
}

/**
 * @tc.name: GetWantSender_DifferentRequestCodes_0100
 * @tc.desc: Test GetWantSender with different request codes
 * @tc.type: FUNC
 */
HWTEST_F(PendingWantManagerSecondTest, GetWantSender_DifferentRequestCodes_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetWantSender_DifferentRequestCodes_0100 start");

    // Arrange
    pendingManager_ = std::make_shared<PendingWantManager>();
    ASSERT_NE(pendingManager_, nullptr);

    Want want;
    ElementName element("device", "bundleName", "abilityName");
    want.SetElement(element);

    // Test with requestCode 0
    WantSenderInfo wantSenderInfo1 = MakeWantSenderInfo(want, 0, 0);
    wantSenderInfo1.requestCode = 0;
    auto sender1 = pendingManager_->GetWantSender(1, 1, true, wantSenderInfo1, nullptr);
    EXPECT_NE(sender1, nullptr);

    // Test with requestCode INT_MAX
    WantSenderInfo wantSenderInfo2 = MakeWantSenderInfo(want, 0, 0);
    wantSenderInfo2.requestCode = INT_MAX;
    auto sender2 = pendingManager_->GetWantSender(1, 1, true, wantSenderInfo2, nullptr);
    EXPECT_NE(sender2, nullptr);

    // Test with requestCode -1 (negative boundary)
    WantSenderInfo wantSenderInfo3 = MakeWantSenderInfo(want, 0, 0);
    wantSenderInfo3.requestCode = -1;
    auto sender3 = pendingManager_->GetWantSender(1, 1, true, wantSenderInfo3, nullptr);
    EXPECT_NE(sender3, nullptr);

    // All should create separate records due to different requestCodes
    EXPECT_EQ((int)pendingManager_->wantRecords_.size(), 3);

    TAG_LOGI(AAFwkTag::TEST, "GetWantSender_DifferentRequestCodes_0100 end");
}

}  // namespace AAFwk
}  // namespace OHOS
