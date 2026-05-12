/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "hilog_tag_wrapper.h"
#include "mock_my_flag.h"
#include "mock_skill_execute_callback.h"

#define private public
#define protected public
#include "skill_execute_manager.h"
#include "skill_execute_record.h"
#undef private
#undef protected

#include "ability_manager_errors.h"
#include "skill_execute_result.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
namespace {
const std::string TEST_BUNDLE_NAME = "com.test.bundle";
const std::string TEST_MODULE_NAME = "entry";
const std::string TEST_SKILL_NAME = "PlayMusic";
const std::string TEST_ABILITY_NAME = "MainAbility";
const std::string TEST_CALLER_BUNDLE = "com.test.caller";
const std::string TEST_REQUEST_CODE = "req_001";
} // namespace

int MyFlag::flag_ = 0;
bool MyFlag::isWithNative_ = false;

class SkillExecuteManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SkillExecuteManagerTest::SetUpTestCase()
{}

void SkillExecuteManagerTest::TearDownTestCase()
{}

void SkillExecuteManagerTest::SetUp()
{
    MyFlag::flag_ = 0;
}

void SkillExecuteManagerTest::TearDown()
{}

/**
 * @tc.name: CreateExecuteRecord_0100
 * @tc.desc: Test CreateExecuteRecord with external request code.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, CreateExecuteRecord_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto manager = std::make_shared<SkillExecuteManager>();
    sptr<MockSkillExecuteCallback> callback = new MockSkillExecuteCallback();

    auto requestCode = manager->CreateExecuteRecord(
        nullptr, TEST_BUNDLE_NAME, TEST_CALLER_BUNDLE, 0, callback, TEST_REQUEST_CODE);

    EXPECT_EQ(requestCode, TEST_REQUEST_CODE);
    EXPECT_EQ(manager->records_.size(), 1U);
    auto record = manager->records_[TEST_REQUEST_CODE];
    ASSERT_NE(record, nullptr);
    EXPECT_EQ(record->requestCode, TEST_REQUEST_CODE);
    EXPECT_EQ(record->targetBundleName, TEST_BUNDLE_NAME);
    EXPECT_EQ(record->callerBundleName, TEST_CALLER_BUNDLE);
    EXPECT_EQ(record->state, SkillExecuteState::EXECUTING);
    ASSERT_NE(record->callback, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: CreateExecuteRecord_0200
 * @tc.desc: Test CreateExecuteRecord without external request code generates auto code.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, CreateExecuteRecord_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto manager = std::make_shared<SkillExecuteManager>();
    sptr<MockSkillExecuteCallback> callback = new MockSkillExecuteCallback();

    auto requestCode = manager->CreateExecuteRecord(
        nullptr, TEST_BUNDLE_NAME, TEST_CALLER_BUNDLE, 0, callback, "");

    EXPECT_FALSE(requestCode.empty());
    EXPECT_EQ(manager->records_.size(), 1U);
    // Auto-generated code should be "1" (first seq)
    EXPECT_EQ(requestCode, "1");
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: CreateExecuteRecord_0300
 * @tc.desc: Test CreateExecuteRecord with null callback.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, CreateExecuteRecord_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto manager = std::make_shared<SkillExecuteManager>();

    auto requestCode = manager->CreateExecuteRecord(
        nullptr, TEST_BUNDLE_NAME, TEST_CALLER_BUNDLE, 0, nullptr, TEST_REQUEST_CODE);

    EXPECT_EQ(requestCode, TEST_REQUEST_CODE);
    EXPECT_EQ(manager->records_.size(), 1U);
    auto record = manager->records_[TEST_REQUEST_CODE];
    ASSERT_NE(record, nullptr);
    EXPECT_EQ(record->callback, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: CreateExecuteRecord_0400
 * @tc.desc: Test CreateExecuteRecord increments request code sequence.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, CreateExecuteRecord_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto manager = std::make_shared<SkillExecuteManager>();

    auto code1 = manager->CreateExecuteRecord(nullptr, TEST_BUNDLE_NAME, TEST_CALLER_BUNDLE, 0, nullptr, "");
    auto code2 = manager->CreateExecuteRecord(nullptr, TEST_BUNDLE_NAME, TEST_CALLER_BUNDLE, 0, nullptr, "");

    EXPECT_EQ(code1, "1");
    EXPECT_EQ(code2, "2");
    EXPECT_EQ(manager->records_.size(), 2U);
    EXPECT_EQ(manager->requestCodeSeq_, 2U);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: ExecuteSkillDone_0100
 * @tc.desc: Test ExecuteSkillDone with non-existent record.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, ExecuteSkillDone_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto manager = std::make_shared<SkillExecuteManager>();
    AppExecFwk::SkillExecuteResult result;

    int32_t ret = manager->ExecuteSkillDone("nonexistent", 0, result, TEST_BUNDLE_NAME);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: ExecuteSkillDone_0200
 * @tc.desc: Test ExecuteSkillDone with mismatched bundle name.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, ExecuteSkillDone_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto manager = std::make_shared<SkillExecuteManager>();
    sptr<MockSkillExecuteCallback> callback = new MockSkillExecuteCallback();

    manager->CreateExecuteRecord(
        nullptr, TEST_BUNDLE_NAME, TEST_CALLER_BUNDLE, 0, callback, TEST_REQUEST_CODE);

    AppExecFwk::SkillExecuteResult result;
    int32_t ret = manager->ExecuteSkillDone(TEST_REQUEST_CODE, 0, result, "wrong.bundle");
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: ExecuteSkillDone_0300
 * @tc.desc: Test ExecuteSkillDone with valid record.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, ExecuteSkillDone_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto manager = std::make_shared<SkillExecuteManager>();
    sptr<MockSkillExecuteCallback> callback = new MockSkillExecuteCallback();

    manager->CreateExecuteRecord(
        nullptr, TEST_BUNDLE_NAME, TEST_CALLER_BUNDLE, 0, callback, TEST_REQUEST_CODE);

    AppExecFwk::SkillExecuteResult result;
    result.code = 0;
    result.result = std::make_shared<AAFwk::WantParams>();

    EXPECT_CALL(*callback, OnExecuteDone(_, _, _)).Times(1);
    int32_t ret = manager->ExecuteSkillDone(TEST_REQUEST_CODE, 0, result, TEST_BUNDLE_NAME);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(manager->records_.size(), 0U);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: ExecuteSkillDone_0400
 * @tc.desc: Test ExecuteSkillDone with null callback.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, ExecuteSkillDone_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto manager = std::make_shared<SkillExecuteManager>();

    manager->CreateExecuteRecord(
        nullptr, TEST_BUNDLE_NAME, TEST_CALLER_BUNDLE, 0, nullptr, TEST_REQUEST_CODE);

    AppExecFwk::SkillExecuteResult result;
    int32_t ret = manager->ExecuteSkillDone(TEST_REQUEST_CODE, 0, result, TEST_BUNDLE_NAME);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(manager->records_.size(), 0U);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: ExecuteSkillDone_0500
 * @tc.desc: Test ExecuteSkillDone changes state to EXECUTE_DONE.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, ExecuteSkillDone_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto manager = std::make_shared<SkillExecuteManager>();
    sptr<MockSkillExecuteCallback> callback = new MockSkillExecuteCallback();

    manager->CreateExecuteRecord(
        nullptr, TEST_BUNDLE_NAME, TEST_CALLER_BUNDLE, 0, callback, TEST_REQUEST_CODE);

    // Verify state before
    EXPECT_EQ(manager->records_[TEST_REQUEST_CODE]->state, SkillExecuteState::EXECUTING);

    AppExecFwk::SkillExecuteResult result;
    EXPECT_CALL(*callback, OnExecuteDone(_, _, _)).Times(1);
    manager->ExecuteSkillDone(TEST_REQUEST_CODE, 0, result, TEST_BUNDLE_NAME);

    // Record should be removed after ExecuteSkillDone
    EXPECT_EQ(manager->records_.size(), 0U);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: ExecuteSkillDone_0600
 * @tc.desc: Test ExecuteSkillDone with record already done (invalid state).
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, ExecuteSkillDone_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto manager = std::make_shared<SkillExecuteManager>();
    sptr<MockSkillExecuteCallback> callback = new MockSkillExecuteCallback();

    manager->CreateExecuteRecord(
        nullptr, TEST_BUNDLE_NAME, TEST_CALLER_BUNDLE, 0, callback, TEST_REQUEST_CODE);

    // Manually set state to DONE
    manager->records_[TEST_REQUEST_CODE]->state = SkillExecuteState::EXECUTE_DONE;

    AppExecFwk::SkillExecuteResult result;
    int32_t ret = manager->ExecuteSkillDone(TEST_REQUEST_CODE, 0, result, TEST_BUNDLE_NAME);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: OnTimeout_0100
 * @tc.desc: Test OnTimeout with non-existent sequence.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, OnTimeout_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto manager = std::make_shared<SkillExecuteManager>();

    // Should not crash with non-existent seq
    manager->OnTimeout(999);
    EXPECT_EQ(manager->records_.size(), 0U);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: OnTimeout_0200
 * @tc.desc: Test OnTimeout with valid executing record.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, OnTimeout_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto manager = std::make_shared<SkillExecuteManager>();
    sptr<MockSkillExecuteCallback> callback = new MockSkillExecuteCallback();

    auto requestCode = manager->CreateExecuteRecord(
        nullptr, TEST_BUNDLE_NAME, TEST_CALLER_BUNDLE, 0, callback, "");

    // Get the seq from the record
    uint64_t seq = manager->records_[requestCode]->requestCodeSeq;
    // Manually add to seqToRequestCodeMap_ since PostSkillExecuteTimeout needs AMS
    manager->seqToRequestCodeMap_[seq] = requestCode;

    AppExecFwk::SkillExecuteResult emptyResult;
    EXPECT_CALL(*callback, OnExecuteDone(_, _, _)).Times(1);
    manager->OnTimeout(static_cast<int64_t>(seq));

    // Record should be removed after timeout
    EXPECT_EQ(manager->records_.size(), 0U);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: OnTimeout_0300
 * @tc.desc: Test OnTimeout with record not in EXECUTING state.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, OnTimeout_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto manager = std::make_shared<SkillExecuteManager>();
    sptr<MockSkillExecuteCallback> callback = new MockSkillExecuteCallback();

    auto requestCode = manager->CreateExecuteRecord(
        nullptr, TEST_BUNDLE_NAME, TEST_CALLER_BUNDLE, 0, callback, "");

    uint64_t seq = manager->records_[requestCode]->requestCodeSeq;
    manager->seqToRequestCodeMap_[seq] = requestCode;

    // Set state to DONE (not EXECUTING)
    manager->records_[requestCode]->state = SkillExecuteState::EXECUTE_DONE;

    // OnTimeout should not call callback
    EXPECT_CALL(*callback, OnExecuteDone(_, _, _)).Times(0);
    manager->OnTimeout(static_cast<int64_t>(seq));

    // Record should still exist since state was not EXECUTING
    EXPECT_EQ(manager->records_.size(), 1U);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: OnTimeout_0400
 * @tc.desc: Test OnTimeout with null callback in record.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, OnTimeout_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto manager = std::make_shared<SkillExecuteManager>();

    auto requestCode = manager->CreateExecuteRecord(
        nullptr, TEST_BUNDLE_NAME, TEST_CALLER_BUNDLE, 0, nullptr, "");

    uint64_t seq = manager->records_[requestCode]->requestCodeSeq;
    manager->seqToRequestCodeMap_[seq] = requestCode;

    // Should not crash with null callback
    manager->OnTimeout(static_cast<int64_t>(seq));

    EXPECT_EQ(manager->records_.size(), 0U);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: RemoveRecord_0100
 * @tc.desc: Test RemoveRecord with existing record.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, RemoveRecord_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto manager = std::make_shared<SkillExecuteManager>();

    manager->CreateExecuteRecord(
        nullptr, TEST_BUNDLE_NAME, TEST_CALLER_BUNDLE, 0, nullptr, TEST_REQUEST_CODE);

    EXPECT_EQ(manager->records_.size(), 1U);

    manager->RemoveRecord(TEST_REQUEST_CODE);
    EXPECT_EQ(manager->records_.size(), 0U);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: RemoveRecord_0200
 * @tc.desc: Test RemoveRecord with non-existent record.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, RemoveRecord_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto manager = std::make_shared<SkillExecuteManager>();

    // Should not crash
    manager->RemoveRecord("nonexistent");
    EXPECT_EQ(manager->records_.size(), 0U);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: OnCallerDied_0100
 * @tc.desc: Test OnCallerDied with existing record.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, OnCallerDied_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto manager = std::make_shared<SkillExecuteManager>();

    auto requestCode = manager->CreateExecuteRecord(
        nullptr, TEST_BUNDLE_NAME, TEST_CALLER_BUNDLE, 0, nullptr, TEST_REQUEST_CODE);

    // Set state to EXECUTING
    ASSERT_NE(manager->records_[TEST_REQUEST_CODE], nullptr);
    manager->records_[TEST_REQUEST_CODE]->state = SkillExecuteState::EXECUTING;

    manager->OnCallerDied(TEST_REQUEST_CODE);

    EXPECT_EQ(manager->records_.size(), 0U);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: OnCallerDied_0200
 * @tc.desc: Test OnCallerDied with non-existent record.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, OnCallerDied_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto manager = std::make_shared<SkillExecuteManager>();

    // Should not crash
    manager->OnCallerDied("nonexistent");
    EXPECT_EQ(manager->records_.size(), 0U);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: CheckSkillPermission_0100
 * @tc.desc: Test CheckSkillPermission with empty permissions list.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, CheckSkillPermission_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto manager = std::make_shared<SkillExecuteManager>();

    AppExecFwk::SkillInfo skillInfo;
    skillInfo.permissions = {};

    int32_t ret = manager->CheckSkillPermission(skillInfo);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: CheckSkillPermission_0200
 * @tc.desc: Test CheckSkillPermission with permissions and system API allowed.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, CheckSkillPermission_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto manager = std::make_shared<SkillExecuteManager>();
    MyFlag::flag_ = 1; // Permission check passes

    AppExecFwk::SkillInfo skillInfo;
    skillInfo.permissions = { "ohos.permission.TEST" };

    int32_t ret = manager->CheckSkillPermission(skillInfo);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: CheckSkillPermission_0300
 * @tc.desc: Test CheckSkillPermission with permissions denied.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, CheckSkillPermission_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto manager = std::make_shared<SkillExecuteManager>();
    MyFlag::flag_ = 0; // Permission check fails

    AppExecFwk::SkillInfo skillInfo;
    skillInfo.permissions = { "ohos.permission.TEST" };

    int32_t ret = manager->CheckSkillPermission(skillInfo);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GenerateSkillWant_0100
 * @tc.desc: Test GenerateSkillWant with abilityName specified in skillInfo.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, GenerateSkillWant_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto manager = std::make_shared<SkillExecuteManager>();

    AppExecFwk::SkillInfo skillInfo;
    skillInfo.bundleName = TEST_BUNDLE_NAME;
    skillInfo.moduleName = TEST_MODULE_NAME;
    skillInfo.skillName = TEST_SKILL_NAME;
    skillInfo.abilityName = TEST_ABILITY_NAME;
    skillInfo.srcEntries = { "./ets/PlayMusic.ts" };
    skillInfo.hapPath = "/data/app/test.hap";

    Want want;
    AppExecFwk::ExtensionAbilityType targetType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED;
    auto skillArgs = std::make_shared<AAFwk::WantParams>();

    int32_t ret = manager->GenerateSkillWant(skillInfo, want, 100, TEST_REQUEST_CODE,
        targetType, "", "", skillArgs);
    // May fail due to BundleMgr dependency for ResolveTargetType, but
    // abilityName is set so it won't call ResolveDefaultAbilityName
    // The function writes to want regardless of ResolveTargetType result
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_TRUE(SkillExecuteParam::IsSkillExecute(want));
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GenerateSkillWant_0200
 * @tc.desc: Test GenerateSkillWant with empty abilityName triggers resolve.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, GenerateSkillWant_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto manager = std::make_shared<SkillExecuteManager>();

    AppExecFwk::SkillInfo skillInfo;
    skillInfo.bundleName = TEST_BUNDLE_NAME;
    skillInfo.moduleName = TEST_MODULE_NAME;
    skillInfo.skillName = TEST_SKILL_NAME;
    skillInfo.abilityName = ""; // Empty, triggers resolve
    skillInfo.srcEntries = {};
    skillInfo.hapPath = "";

    Want want;
    AppExecFwk::ExtensionAbilityType targetType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED;

    // ResolveDefaultAbilityName will fail (no BundleMgr), returns ERR_INVALID_VALUE
    int32_t ret = manager->GenerateSkillWant(skillInfo, want, 100, TEST_REQUEST_CODE, targetType);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: SkillExecuteRecord_0100
 * @tc.desc: Test SkillExecuteRecord initial state.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, SkillExecuteRecord_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    SkillExecuteRecord record;
    EXPECT_EQ(record.callerToken, nullptr);
    EXPECT_EQ(record.deathRecipient, nullptr);
    EXPECT_EQ(record.callerTokenId, 0U);
    EXPECT_EQ(record.requestCodeSeq, 0U);
    EXPECT_EQ(record.state, SkillExecuteState::UNKNOWN);
    EXPECT_EQ(record.callback, nullptr);
    EXPECT_TRUE(record.requestCode.empty());
    EXPECT_TRUE(record.targetBundleName.empty());
    EXPECT_TRUE(record.callerBundleName.empty());
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: SkillExecuteState_0100
 * @tc.desc: Test SkillExecuteState enum values.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, SkillExecuteState_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    EXPECT_EQ(static_cast<int>(SkillExecuteState::UNKNOWN), 0);
    EXPECT_EQ(static_cast<int>(SkillExecuteState::EXECUTING), 1);
    EXPECT_EQ(static_cast<int>(SkillExecuteState::EXECUTE_DONE), 2);
    EXPECT_EQ(static_cast<int>(SkillExecuteState::REMOTE_DIED), 3);
    EXPECT_EQ(static_cast<int>(SkillExecuteState::TIMED_OUT), 4);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}
} // namespace AAFwk
} // namespace OHOS
