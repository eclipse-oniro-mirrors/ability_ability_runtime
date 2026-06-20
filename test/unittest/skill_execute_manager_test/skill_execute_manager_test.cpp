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

#include <memory>

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "mock_skill_execute_callback_stub.h"
#include "singleton.h"
#include "skill/skill_execute_manager.h"
#include "skill/skill_execute_record.h"
#include "skill_execute_result.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AAFwk {

namespace {
constexpr const char *TARGET_BUNDLE = "com.example.target";
constexpr const char *CALLER_BUNDLE = "com.example.caller";
} // namespace

class SkillExecuteManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() override;
    void TearDown() override;

protected:
    void SeedRecord(const std::string &requestCode, uint64_t seq,
        SkillExecuteState state = SkillExecuteState::EXECUTING);

    std::shared_ptr<SkillExecuteManager> mgr_;
    sptr<MockSkillExecuteCallbackStub> callback_;
};

void SkillExecuteManagerTest::SetUp()
{
    mgr_ = DelayedSingleton<SkillExecuteManager>::GetInstance();
    ASSERT_NE(mgr_, nullptr);
    callback_ = new MockSkillExecuteCallbackStub();
    ASSERT_NE(callback_, nullptr);
}

void SkillExecuteManagerTest::TearDown()
{
    {
        std::lock_guard<ffrt::mutex> lock(mgr_->mutex_);
        mgr_->records_.clear();
        mgr_->seqToRequestCodeMap_.clear();
    }
    callback_ = nullptr;
    mgr_ = nullptr;
}

void SkillExecuteManagerTest::SeedRecord(const std::string &requestCode, uint64_t seq,
    SkillExecuteState state)
{
    auto record = std::make_shared<SkillExecuteRecord>();
    record->requestCode = requestCode;
    record->targetBundleName = TARGET_BUNDLE;
    record->callerBundleName = CALLER_BUNDLE;
    record->requestCodeSeq = seq;
    record->state = state;
    record->callback = callback_;

    std::lock_guard<ffrt::mutex> lock(mgr_->mutex_);
    mgr_->records_[requestCode] = record;
    mgr_->seqToRequestCodeMap_[seq] = requestCode;
}

/**
 * @tc.name: ExecuteSkillDone_0100
 * @tc.desc: ExecuteSkillDone happy path: callback invoked, record removed.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, ExecuteSkillDone_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    SeedRecord("req001", 1);
    EXPECT_CALL(*callback_, OnExecuteDone(Eq("req001"), Eq(0), _)).Times(1);

    AppExecFwk::SkillExecuteResult result;
    auto ret = mgr_->ExecuteSkillDone("req001", 0, result, TARGET_BUNDLE);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: ExecuteSkillDone_0200
 * @tc.desc: ExecuteSkillDone on unknown requestCode returns ERR_CODE_INVALID_ID.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, ExecuteSkillDone_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    EXPECT_CALL(*callback_, OnExecuteDone(_, _, _)).Times(0);

    AppExecFwk::SkillExecuteResult result;
    auto ret = mgr_->ExecuteSkillDone("nonexistent", 0, result, TARGET_BUNDLE);
    EXPECT_EQ(ret, ERR_CODE_INVALID_ID);
    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: ExecuteSkillDone_0300
 * @tc.desc: ExecuteSkillDone with bundle mismatch returns ERR_INVALID_VALUE.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, ExecuteSkillDone_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    SeedRecord("req003", 3);
    EXPECT_CALL(*callback_, OnExecuteDone(_, _, _)).Times(0);

    AppExecFwk::SkillExecuteResult result;
    auto ret = mgr_->ExecuteSkillDone("req003", 0, result, "com.example.wrong");
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: ExecuteSkillDone_0400
 * @tc.desc: ExecuteSkillDone twice: second call fails because record was removed.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, ExecuteSkillDone_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    SeedRecord("req004", 4);
    EXPECT_CALL(*callback_, OnExecuteDone(_, _, _)).Times(1);

    AppExecFwk::SkillExecuteResult result;
    auto ret1 = mgr_->ExecuteSkillDone("req004", 0, result, TARGET_BUNDLE);
    EXPECT_EQ(ret1, ERR_OK);

    auto ret2 = mgr_->ExecuteSkillDone("req004", 0, result, TARGET_BUNDLE);
    EXPECT_EQ(ret2, ERR_CODE_INVALID_ID);
    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: ExecuteSkillDone_0500
 * @tc.desc: ExecuteSkillDone reentrant from callback must not deadlock.
 *          Pre-fix: callback ran under mutex_; re-entering deadlocked.
 *          Post-fix: callback runs outside mutex_; re-entry is safe.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, ExecuteSkillDone_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    SeedRecord("req005", 5);

    auto mgr = mgr_;
    EXPECT_CALL(*callback_, OnExecuteDone(Eq("req005"), _, _))
        .Times(1)
        .WillOnce(Invoke([mgr](const std::string &reqCode, int32_t,
            const AppExecFwk::SkillExecuteResult &) {
            AppExecFwk::SkillExecuteResult inner;
            auto ret = mgr->ExecuteSkillDone(reqCode, 0, inner, TARGET_BUNDLE);
            EXPECT_EQ(ret, ERR_CODE_INVALID_ID);
        }));

    AppExecFwk::SkillExecuteResult result;
    auto ret = mgr_->ExecuteSkillDone("req005", 0, result, TARGET_BUNDLE);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: OnTimeout_0100
 * @tc.desc: OnTimeout on EXECUTING record flips state, fires callback once.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, OnTimeout_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    SeedRecord("req101", 101);
    EXPECT_CALL(*callback_, OnExecuteDone(Eq("req101"), Eq(ERR_TIMED_OUT), _)).Times(1);

    mgr_->OnTimeout(101);
    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: OnTimeout_0200
 * @tc.desc: OnTimeout with unknown seq does nothing.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, OnTimeout_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    EXPECT_CALL(*callback_, OnExecuteDone(_, _, _)).Times(0);

    mgr_->OnTimeout(999);
    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: OnTimeout_0300
 * @tc.desc: OnTimeout twice on same seq: second call is a no-op.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, OnTimeout_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    SeedRecord("req103", 103);
    EXPECT_CALL(*callback_, OnExecuteDone(_, _, _)).Times(1);

    mgr_->OnTimeout(103);
    mgr_->OnTimeout(103);
    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: OnTimeout_0400
 * @tc.desc: OnTimeout after ExecuteSkillDone: record gone, no callback.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, OnTimeout_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    SeedRecord("req104", 104);
    EXPECT_CALL(*callback_, OnExecuteDone(_, _, _)).Times(1);

    AppExecFwk::SkillExecuteResult result;
    mgr_->ExecuteSkillDone("req104", 0, result, TARGET_BUNDLE);
    mgr_->OnTimeout(104);
    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: OnTimeout_0500
 * @tc.desc: OnTimeout reentrant from callback must not deadlock.
 *          Pre-fix: callback ran under mutex_; re-entering deadlocked.
 *          Post-fix: callback runs outside mutex_; re-entry is safe.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, OnTimeout_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    SeedRecord("req105", 105);

    auto mgr = mgr_;
    EXPECT_CALL(*callback_, OnExecuteDone(Eq("req105"), _, _))
        .Times(1)
        .WillOnce(Invoke([mgr](const std::string &, int32_t,
            const AppExecFwk::SkillExecuteResult &) {
            mgr->OnTimeout(105); // seq already erased, no-op
        }));

    mgr_->OnTimeout(105);
    TAG_LOGI(AAFwkTag::TEST, "end");
}

} // namespace AAFwk
} // namespace OHOS
