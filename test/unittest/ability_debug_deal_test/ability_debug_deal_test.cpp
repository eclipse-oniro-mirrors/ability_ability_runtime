/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#define protected public
#define private public
#include "ability_debug_deal.h"
#include "ability_record.h"
#undef private
#undef protected
#include "want.h"

namespace OHOS {
namespace AAFwk {
using namespace testing;
using namespace testing::ext;
namespace {
    const std::string STRING_PROCESS_NAME = "process_name";
}
class AbilityDebugDealTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    std::shared_ptr<AbilityDebugDeal> deal_;
    std::vector<sptr<IRemoteObject>> tokens_;
};

void AbilityDebugDealTest::SetUpTestCase(void)
{}

void AbilityDebugDealTest::TearDownTestCase(void)
{}

void AbilityDebugDealTest::SetUp()
{
    deal_ = std::make_shared<AbilityDebugDeal>();
}

void AbilityDebugDealTest::TearDown()
{}

/**
 * @tc.name: AbilityDebugDealTest_RegisterAbilityDebugResponse_0100
 * @tc.desc: Verify register ability debug response calls normally.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityDebugDealTest, RegisterAbilityDebugResponse_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityDebugDealTest_RegisterAbilityDebugResponse_0100 start";
    EXPECT_EQ(deal_->abilityDebugResponse_, nullptr);
    deal_->RegisterAbilityDebugResponse();
    EXPECT_NE(deal_->abilityDebugResponse_, nullptr);
    GTEST_LOG_(INFO) << "AbilityDebugDealTest_RegisterAbilityDebugResponse_0100 end";
}

/**
 * @tc.name: OnAbilitysDebugStarted_0100
 * @tc.desc: Verify that OnAbilitysDebugStoped set isAttachDebug_ of AbilityRecord to true correctly.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityDebugDealTest, OnAbilitysDebugStarted_0100, TestSize.Level1)
{
    EXPECT_NE(deal_, nullptr);

    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.process = STRING_PROCESS_NAME;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    auto record = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    EXPECT_NE(record, nullptr);
    EXPECT_FALSE(record->isAttachDebug_);
    const sptr<IRemoteObject> token = new Token(record);
    tokens_.emplace_back(token);

    deal_->OnAbilitysDebugStarted(tokens_);
    auto ability_record = Token::GetAbilityRecordByToken(token);
    EXPECT_TRUE(ability_record->isAttachDebug_);
}

/**
 * @tc.name: OnAbilitysDebugStoped_0100
 * @tc.desc: Verify that OnAbilitysDebugStoped set isAttachDebug_ of AbilityRecord to false correctly.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityDebugDealTest, OnAbilitysDebugStoped_0100, TestSize.Level1)
{
    EXPECT_NE(deal_, nullptr);

    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.process = STRING_PROCESS_NAME;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    auto record = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    EXPECT_NE(record, nullptr);
    EXPECT_FALSE(record->isAttachDebug_);
    const sptr<IRemoteObject> token = new Token(record);
    tokens_.emplace_back(token);

    deal_->OnAbilitysDebugStarted(tokens_);
    auto ability_record = Token::GetAbilityRecordByToken(token);
    EXPECT_TRUE(ability_record->isAttachDebug_);
    deal_->OnAbilitysDebugStoped(tokens_);
    EXPECT_FALSE(ability_record->isAttachDebug_);
}

/**
 * @tc.name: OnAbilitysAssertDebugChange_0100
 * @tc.desc: Verify that OnAbilitysAssertDebugChange set assert debug flag to true correctly.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityDebugDealTest, OnAbilitysAssertDebugChange_0100, TestSize.Level1)
{
    EXPECT_NE(deal_, nullptr);

    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.process = STRING_PROCESS_NAME;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    auto record = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    EXPECT_NE(record, nullptr);
    EXPECT_FALSE(record->isAssertDebug_);
    const sptr<IRemoteObject> token = new Token(record);
    tokens_.emplace_back(token);

    deal_->OnAbilitysAssertDebugChange(tokens_, true);
    auto ability_record = Token::GetAbilityRecordByToken(token);
    EXPECT_TRUE(ability_record->isAssertDebug_);
}

/**
 * @tc.name: OnAbilitysAssertDebugChange_0200
 * @tc.desc: Verify that OnAbilitysAssertDebugChange set assert debug flag to false correctly.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityDebugDealTest, OnAbilitysAssertDebugChange_0200, TestSize.Level1)
{
    EXPECT_NE(deal_, nullptr);

    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.process = STRING_PROCESS_NAME;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    auto record = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    EXPECT_NE(record, nullptr);
    record->isAssertDebug_ = true;
    const sptr<IRemoteObject> token = new Token(record);
    tokens_.emplace_back(token);

    deal_->OnAbilitysAssertDebugChange(tokens_, false);
    auto ability_record = Token::GetAbilityRecordByToken(token);
    EXPECT_FALSE(ability_record->isAssertDebug_);
}

/**
 * @tc.name: OnAbilitysAssertDebugChange_0300
 * @tc.desc: Verify that OnAbilitysAssertDebugChange handles multiple tokens correctly.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityDebugDealTest, OnAbilitysAssertDebugChange_0300, TestSize.Level1)
{
    EXPECT_NE(deal_, nullptr);

    std::vector<std::shared_ptr<AbilityRecord>> records;
    std::vector<sptr<IRemoteObject>> tokens;
    
    for (int i = 0; i < 3; i++) {
        Want want;
        OHOS::AppExecFwk::AbilityInfo abilityInfo;
        abilityInfo.process = STRING_PROCESS_NAME + std::to_string(i);
        OHOS::AppExecFwk::ApplicationInfo applicationInfo;
        auto record = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
        EXPECT_NE(record, nullptr);
        EXPECT_FALSE(record->isAssertDebug_);
        const sptr<IRemoteObject> token = new Token(record);
        records.push_back(record);
        tokens.emplace_back(token);
    }

    deal_->OnAbilitysAssertDebugChange(tokens, true);

    for (size_t i = 0; i < tokens.size(); i++) {
        auto ability_record = Token::GetAbilityRecordByToken(tokens[i]);
        EXPECT_TRUE(ability_record->isAssertDebug_);
    }
}

/**
 * @tc.name: OnAbilitysAssertDebugChange_0400
 * @tc.desc: Verify that OnAbilitysAssertDebugChange handles null token gracefully.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityDebugDealTest, OnAbilitysAssertDebugChange_0400, TestSize.Level1)
{
    EXPECT_NE(deal_, nullptr);

    std::vector<sptr<IRemoteObject>> tokens;
    tokens.emplace_back(nullptr);
    
    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.process = STRING_PROCESS_NAME;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    auto record = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    EXPECT_NE(record, nullptr);
    const sptr<IRemoteObject> validToken = new Token(record);
    tokens.emplace_back(validToken);

    deal_->OnAbilitysAssertDebugChange(tokens, true);
    
    auto ability_record = Token::GetAbilityRecordByToken(validToken);
    EXPECT_TRUE(ability_record->isAssertDebug_);
}

/**
 * @tc.name: AbilityDebugResponse_OnAbilitysDebugStarted_0100
 * @tc.desc: Verify AbilityDebugResponse forwards call to AbilityDebugDeal correctly.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityDebugDealTest, AbilityDebugResponse_OnAbilitysDebugStarted_0100, TestSize.Level1)
{
    auto response = std::make_shared<AbilityDebugResponse>(deal_);
    EXPECT_NE(response, nullptr);

    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.process = STRING_PROCESS_NAME;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    auto record = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    EXPECT_NE(record, nullptr);
    EXPECT_FALSE(record->isAttachDebug_);
    const sptr<IRemoteObject> token = new Token(record);
    tokens_.emplace_back(token);

    response->OnAbilitysDebugStarted(tokens_);
    
    auto ability_record = Token::GetAbilityRecordByToken(token);
    EXPECT_TRUE(ability_record->isAttachDebug_);
}

/**
 * @tc.name: AbilityDebugResponse_OnAbilitysDebugStarted_EmptyTokens_0200
 * @tc.desc: Verify AbilityDebugResponse handles empty tokens gracefully.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityDebugDealTest, AbilityDebugResponse_OnAbilitysDebugStarted_EmptyTokens_0200, TestSize.Level1)
{
    auto response = std::make_shared<AbilityDebugResponse>(deal_);
    EXPECT_NE(response, nullptr);

    std::vector<sptr<IRemoteObject>> emptyTokens;
    
    response->OnAbilitysDebugStarted(emptyTokens);
    
    EXPECT_TRUE(true);
}

/**
 * @tc.name: AbilityDebugResponse_OnAbilitysDebugStoped_0100
 * @tc.desc: Verify AbilityDebugResponse forwards stop call to AbilityDebugDeal correctly.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityDebugDealTest, AbilityDebugResponse_OnAbilitysDebugStoped_0100, TestSize.Level1)
{
    auto response = std::make_shared<AbilityDebugResponse>(deal_);
    EXPECT_NE(response, nullptr);

    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.process = STRING_PROCESS_NAME;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    auto record = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    EXPECT_NE(record, nullptr);
    record->isAttachDebug_ = true;
    const sptr<IRemoteObject> token = new Token(record);
    tokens_.emplace_back(token);

    response->OnAbilitysDebugStoped(tokens_);
    
    auto ability_record = Token::GetAbilityRecordByToken(token);
    EXPECT_FALSE(ability_record->isAttachDebug_);
}

/**
 * @tc.name: AbilityDebugResponse_OnAbilitysAssertDebugChange_0100
 * @tc.desc: Verify AbilityDebugResponse forwards assert debug change call correctly.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityDebugDealTest, AbilityDebugResponse_OnAbilitysAssertDebugChange_0100, TestSize.Level1)
{
    auto response = std::make_shared<AbilityDebugResponse>(deal_);
    EXPECT_NE(response, nullptr);

    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.process = STRING_PROCESS_NAME;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    auto record = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    EXPECT_NE(record, nullptr);
    EXPECT_FALSE(record->isAssertDebug_);
    const sptr<IRemoteObject> token = new Token(record);
    tokens_.emplace_back(token);

    response->OnAbilitysAssertDebugChange(tokens_, true);
    
    auto ability_record = Token::GetAbilityRecordByToken(token);
    EXPECT_TRUE(ability_record->isAssertDebug_);
}

/**
 * @tc.name: AbilityDebugResponse_NullDeal_0100
 * @tc.desc: Verify AbilityDebugResponse handles null deal gracefully.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityDebugDealTest, AbilityDebugResponse_NullDeal_0100, TestSize.Level1)
{
    std::weak_ptr<AbilityDebugDeal> nullDeal;
    auto response = std::make_shared<AbilityDebugResponse>(nullDeal);
    EXPECT_NE(response, nullptr);

    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.process = STRING_PROCESS_NAME;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    auto record = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    EXPECT_NE(record, nullptr);
    const sptr<IRemoteObject> token = new Token(record);
    tokens_.emplace_back(token);

    response->OnAbilitysDebugStarted(tokens_);
    response->OnAbilitysDebugStoped(tokens_);
    response->OnAbilitysAssertDebugChange(tokens_, true);
    
    auto ability_record = Token::GetAbilityRecordByToken(token);
    EXPECT_FALSE(ability_record->isAttachDebug_);
    EXPECT_FALSE(ability_record->isAssertDebug_);
}

/**
 * @tc.name: MixedOperations_0100
 * @tc.desc: Verify mixed debug operations work correctly together.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityDebugDealTest, MixedOperations_0100, TestSize.Level1)
{
    EXPECT_NE(deal_, nullptr);

    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.process = STRING_PROCESS_NAME;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    auto record = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    EXPECT_NE(record, nullptr);
    const sptr<IRemoteObject> token = new Token(record);
    tokens_.emplace_back(token);

    deal_->OnAbilitysDebugStarted(tokens_);
    auto ability_record = Token::GetAbilityRecordByToken(token);
    EXPECT_TRUE(ability_record->isAttachDebug_);
    EXPECT_FALSE(ability_record->isAssertDebug_);

    deal_->OnAbilitysAssertDebugChange(tokens_, true);
    EXPECT_TRUE(ability_record->isAttachDebug_);
    EXPECT_TRUE(ability_record->isAssertDebug_);

    deal_->OnAbilitysAssertDebugChange(tokens_, false);
    EXPECT_TRUE(ability_record->isAttachDebug_);
    EXPECT_FALSE(ability_record->isAssertDebug_);

    deal_->OnAbilitysDebugStoped(tokens_);
    EXPECT_FALSE(ability_record->isAttachDebug_);
    EXPECT_FALSE(ability_record->isAssertDebug_);
}

/**
 * @tc.name: OnAbilitysDebugStarted_EmptyTokens_0200
 * @tc.desc: Verify OnAbilitysDebugStarted handles empty tokens gracefully.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityDebugDealTest, OnAbilitysDebugStarted_EmptyTokens_0200, TestSize.Level1)
{
    EXPECT_NE(deal_, nullptr);

    std::vector<sptr<IRemoteObject>> emptyTokens;
    
    deal_->OnAbilitysDebugStarted(emptyTokens);
    
    EXPECT_TRUE(true);
}

/**
 * @tc.name: OnAbilitysDebugStoped_EmptyTokens_0200
 * @tc.desc: Verify OnAbilitysDebugStoped handles empty tokens gracefully.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityDebugDealTest, OnAbilitysDebugStoped_EmptyTokens_0200, TestSize.Level1)
{
    EXPECT_NE(deal_, nullptr);

    std::vector<sptr<IRemoteObject>> emptyTokens;
    
    deal_->OnAbilitysDebugStoped(emptyTokens);
    
    EXPECT_TRUE(true);
}

/**
 * @tc.name: OnAbilitysAssertDebugChange_EmptyTokens_0500
 * @tc.desc: Verify OnAbilitysAssertDebugChange handles empty tokens gracefully.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityDebugDealTest, OnAbilitysAssertDebugChange_EmptyTokens_0500, TestSize.Level1)
{
    EXPECT_NE(deal_, nullptr);

    std::vector<sptr<IRemoteObject>> emptyTokens;
    
    deal_->OnAbilitysAssertDebugChange(emptyTokens, true);
    
    EXPECT_TRUE(true);
}
}  // namespace AppExecFwk
}  // namespace OHOS