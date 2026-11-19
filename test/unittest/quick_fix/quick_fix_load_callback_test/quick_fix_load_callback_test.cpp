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
#include <climits>

#include "hilog_tag_wrapper.h"
#include "mock_quick_fix_manager_stub.h"
#define private public
#include "quick_fix_load_callback.h"
#include "quick_fix_manager_client.h"
#undef private
#include "system_ability_definition.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class QuickFixLoadCallbackTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<MockQuickFixManagerStub> mockService_ = nullptr;
    std::shared_ptr<QuickFixLoadCallback> loadCallback_ = nullptr;
};

void QuickFixLoadCallbackTest::SetUpTestCase(void) {}

void QuickFixLoadCallbackTest::TearDownTestCase(void) {}

void QuickFixLoadCallbackTest::SetUp()
{
    mockService_ = new (std::nothrow) MockQuickFixManagerStub();
    ASSERT_NE(mockService_, nullptr);
    loadCallback_ = std::make_shared<QuickFixLoadCallback>();
    ASSERT_NE(loadCallback_, nullptr);
    auto client = QuickFixManagerClient::GetInstance();
    ASSERT_NE(client, nullptr);
    client->quickFixMgr_ = nullptr;
}

void QuickFixLoadCallbackTest::TearDown() {}

/**
 * @tc.name: OnLoadSystemAbilitySuccess_0100
 * @tc.desc: valid SA id with non-null object should forward to client and set the proxy.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixLoadCallbackTest, OnLoadSystemAbilitySuccess_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto client = QuickFixManagerClient::GetInstance();
    ASSERT_NE(client, nullptr);
    EXPECT_EQ(client->quickFixMgr_, nullptr);

    loadCallback_->OnLoadSystemAbilitySuccess(QUICK_FIX_MGR_SERVICE_ID, mockService_);
    EXPECT_NE(client->quickFixMgr_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: OnLoadSystemAbilitySuccess_0200
 * @tc.desc: mismatched SA id should early-return without touching the client proxy.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixLoadCallbackTest, OnLoadSystemAbilitySuccess_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto client = QuickFixManagerClient::GetInstance();
    ASSERT_NE(client, nullptr);
    client->quickFixMgr_ = nullptr;

    loadCallback_->OnLoadSystemAbilitySuccess(QUICK_FIX_MGR_SERVICE_ID - 1, mockService_);
    EXPECT_EQ(client->quickFixMgr_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: OnLoadSystemAbilitySuccess_0300
 * @tc.desc: valid SA id but null remote object should early-return without setting proxy.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixLoadCallbackTest, OnLoadSystemAbilitySuccess_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto client = QuickFixManagerClient::GetInstance();
    ASSERT_NE(client, nullptr);
    client->quickFixMgr_ = nullptr;

    loadCallback_->OnLoadSystemAbilitySuccess(QUICK_FIX_MGR_SERVICE_ID, nullptr);
    EXPECT_EQ(client->quickFixMgr_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: OnLoadSystemAbilitySuccess_0400
 * @tc.desc: both mismatched SA id and null object should early-return safely.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixLoadCallbackTest, OnLoadSystemAbilitySuccess_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto client = QuickFixManagerClient::GetInstance();
    ASSERT_NE(client, nullptr);
    client->quickFixMgr_ = nullptr;

    loadCallback_->OnLoadSystemAbilitySuccess(QUICK_FIX_MGR_SERVICE_ID + 1, nullptr);
    EXPECT_EQ(client->quickFixMgr_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: OnLoadSystemAbilitySuccess_0500
 * @tc.desc: SA id one less than QUICK_FIX_MGR_SERVICE_ID is a mismatch and should early-return.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixLoadCallbackTest, OnLoadSystemAbilitySuccess_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto client = QuickFixManagerClient::GetInstance();
    ASSERT_NE(client, nullptr);
    client->quickFixMgr_ = nullptr;

    loadCallback_->OnLoadSystemAbilitySuccess(QUICK_FIX_MGR_SERVICE_ID - 1, mockService_);
    EXPECT_EQ(client->quickFixMgr_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: OnLoadSystemAbilitySuccess_0600
 * @tc.desc: SA id one more than QUICK_FIX_MGR_SERVICE_ID is a mismatch and should early-return.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixLoadCallbackTest, OnLoadSystemAbilitySuccess_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto client = QuickFixManagerClient::GetInstance();
    ASSERT_NE(client, nullptr);
    client->quickFixMgr_ = nullptr;

    loadCallback_->OnLoadSystemAbilitySuccess(QUICK_FIX_MGR_SERVICE_ID + 1, mockService_);
    EXPECT_EQ(client->quickFixMgr_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: OnLoadSystemAbilitySuccess_0700
 * @tc.desc: negative SA id is a mismatch and should early-return safely.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixLoadCallbackTest, OnLoadSystemAbilitySuccess_0700, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto client = QuickFixManagerClient::GetInstance();
    ASSERT_NE(client, nullptr);
    client->quickFixMgr_ = nullptr;

    loadCallback_->OnLoadSystemAbilitySuccess(-1, mockService_);
    EXPECT_EQ(client->quickFixMgr_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: OnLoadSystemAbilityFail_0100
 * @tc.desc: valid SA id on fail should forward to client OnLoadSystemAbilityFail without crash.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixLoadCallbackTest, OnLoadSystemAbilityFail_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto client = QuickFixManagerClient::GetInstance();
    ASSERT_NE(client, nullptr);
    client->loadSaFinished_ = false;

    loadCallback_->OnLoadSystemAbilityFail(QUICK_FIX_MGR_SERVICE_ID);
    EXPECT_TRUE(client->loadSaFinished_);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: OnLoadSystemAbilityFail_0200
 * @tc.desc: mismatched SA id on fail should early-return without touching client state.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixLoadCallbackTest, OnLoadSystemAbilityFail_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto client = QuickFixManagerClient::GetInstance();
    ASSERT_NE(client, nullptr);
    client->loadSaFinished_ = false;

    loadCallback_->OnLoadSystemAbilityFail(QUICK_FIX_MGR_SERVICE_ID - 1);
    EXPECT_FALSE(client->loadSaFinished_);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: OnLoadSystemAbilityFail_0300
 * @tc.desc: SA id one less than target is a mismatch on fail path.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixLoadCallbackTest, OnLoadSystemAbilityFail_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto client = QuickFixManagerClient::GetInstance();
    ASSERT_NE(client, nullptr);
    client->loadSaFinished_ = false;

    loadCallback_->OnLoadSystemAbilityFail(QUICK_FIX_MGR_SERVICE_ID - 1);
    EXPECT_FALSE(client->loadSaFinished_);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: OnLoadSystemAbilityFail_0400
 * @tc.desc: SA id one more than target is a mismatch on fail path.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixLoadCallbackTest, OnLoadSystemAbilityFail_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto client = QuickFixManagerClient::GetInstance();
    ASSERT_NE(client, nullptr);
    client->loadSaFinished_ = false;

    loadCallback_->OnLoadSystemAbilityFail(QUICK_FIX_MGR_SERVICE_ID + 1);
    EXPECT_FALSE(client->loadSaFinished_);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: OnLoadSystemAbilityFail_0500
 * @tc.desc: negative SA id is a mismatch on fail path and should early-return.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixLoadCallbackTest, OnLoadSystemAbilityFail_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto client = QuickFixManagerClient::GetInstance();
    ASSERT_NE(client, nullptr);
    client->loadSaFinished_ = false;

    loadCallback_->OnLoadSystemAbilityFail(-1);
    EXPECT_FALSE(client->loadSaFinished_);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: OnLoadSystemAbilityFail_0600
 * @tc.desc: SA id of 0 is a mismatch on fail path and should early-return.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixLoadCallbackTest, OnLoadSystemAbilityFail_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto client = QuickFixManagerClient::GetInstance();
    ASSERT_NE(client, nullptr);
    client->loadSaFinished_ = false;

    loadCallback_->OnLoadSystemAbilityFail(0);
    EXPECT_FALSE(client->loadSaFinished_);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: OnLoadSystemAbilityFail_0700
 * @tc.desc: SA id of INT32_MAX is a mismatch on fail path and should early-return.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixLoadCallbackTest, OnLoadSystemAbilityFail_0700, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto client = QuickFixManagerClient::GetInstance();
    ASSERT_NE(client, nullptr);
    client->loadSaFinished_ = false;

    loadCallback_->OnLoadSystemAbilityFail(INT32_MAX);
    EXPECT_FALSE(client->loadSaFinished_);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}
} // namespace AAFwk
} // namespace OHOS
