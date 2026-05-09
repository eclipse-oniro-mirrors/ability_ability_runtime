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

#include "hilog_tag_wrapper.h"
#include "insight_intent_execute_manager.h"
#include "insight_intent_execute_param.h"
#include "insight_intent_query_param.h"
#include "extract_insight_intent_profile.h"
#include "mock_ability_token.h"
#include "string_wrapper.h"
#include "want.h"

using namespace testing;
using namespace testing::ext;

namespace {
constexpr uint64_t NON_EXIST_ID = 12345;
};
namespace OHOS {
namespace AAFwk {
class InsightIntentExecuteManagerSecondTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void InsightIntentExecuteManagerSecondTest::SetUpTestCase(void)
{}

void InsightIntentExecuteManagerSecondTest::TearDownTestCase(void)
{}

void InsightIntentExecuteManagerSecondTest::SetUp()
{}

void InsightIntentExecuteManagerSecondTest::TearDown()
{}

/**
 * @tc.name: CheckAndUpdateParam_0100
 * @tc.desc: CheckAndUpdateParam_0100
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, CheckAndUpdateParam_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "test.abilityName";
    param.insightIntentName_ = "PlayMusic";
    param.insightIntentParam_ = std::make_shared<WantParams>();
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    sptr<IRemoteObject> callerToken = new AppExecFwk::MockAbilityToken();
    uint64_t key = 1;
    std::string callerBundleName = "com.bundlename.test";
    auto ret = DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->CheckAndUpdateParam(
        key, callerToken, paramPtr, callerBundleName);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: CheckAndUpdateParam_0200
 * @tc.desc: CheckAndUpdateParam_0200
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, CheckAndUpdateParam_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "test.abilityName";
    param.insightIntentName_ = "PlayMusic";
    param.insightIntentParam_ = std::make_shared<WantParams>();
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    sptr<IRemoteObject> callerToken;
    uint64_t key = 1;
    std::string callerBundleName = "com.bundlename.test";
    auto ret = DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->CheckAndUpdateParam(
        key, callerToken, paramPtr, callerBundleName);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: CheckAndUpdateParam_0300
 * @tc.desc: CheckAndUpdateParam_0300
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, CheckAndUpdateParam_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> paramPtr = nullptr;
    sptr<IRemoteObject> callerToken = new AppExecFwk::MockAbilityToken();
    uint64_t key = 1;
    std::string callerBundleName = "com.bundlename.test";
    auto ret = DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->CheckAndUpdateParam(
        key, callerToken, paramPtr, callerBundleName);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: CheckAndUpdateParam_0400
 * @tc.desc: CheckAndUpdateParam_0400
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, CheckAndUpdateParam_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AppExecFwk::InsightIntentExecuteParam param;
    param.moduleName_ = "test.entry";
    param.abilityName_ = "test.abilityName";
    param.insightIntentName_ = "PlayMusic";
    param.insightIntentParam_ = std::make_shared<WantParams>();
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    sptr<IRemoteObject> callerToken = new AppExecFwk::MockAbilityToken();
    uint64_t key = 1;
    std::string callerBundleName = "com.bundlename.test";
    auto ret = DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->CheckAndUpdateParam(
        key, callerToken, paramPtr, callerBundleName);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: CheckAndUpdateParam_0500
 * @tc.desc: Test CheckAndUpdateParam with ignoreAbilityName true
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, CheckAndUpdateParam_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = ""; // 空abilityName，但ignoreAbilityName为true
    param.insightIntentName_ = "PlayMusic";
    param.insightIntentParam_ = std::make_shared<WantParams>();
    param.isServiceMatch_ = true; // 跳过权限和callerToken检查
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    sptr<IRemoteObject> callerToken = nullptr; // 空callerToken但isServiceMatch_为true
    uint64_t key = 1;
    std::string callerBundleName = "com.bundlename.test";
    
    // 调用带ignoreAbilityName参数的重载版本
    std::shared_ptr<InsightIntentExecuteManager> manager = std::make_shared<InsightIntentExecuteManager>();
    auto ret = manager->CheckAndUpdateParam(key, callerToken, paramPtr, callerBundleName, true);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: CheckAndUpdateParam_0600
 * @tc.desc: Test CheckAndUpdateParam with permission denied but service match
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, CheckAndUpdateParam_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "test.abilityName";
    param.insightIntentName_ = "PlayMusic";
    param.insightIntentParam_ = std::make_shared<WantParams>();
    param.isServiceMatch_ = true; // 即使权限检查失败也能通过
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    sptr<IRemoteObject> callerToken = new AppExecFwk::MockAbilityToken();
    uint64_t key = 1;
    std::string callerBundleName = "com.bundlename.test";
    
    auto ret = DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->CheckAndUpdateParam(
        key, callerToken, paramPtr, callerBundleName);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: CheckAndUpdateWant_0100
 * @tc.desc: CheckAndUpdateWant_0100
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, CheckAndUpdateWant_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    Want want;
    auto executeMode = AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND;
    std::string callerBundleName = "test";
    int32_t userId = 100;
    auto ret = DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->CheckAndUpdateWant(
        want, executeMode, userId, callerBundleName);
    EXPECT_NE(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: CheckAndUpdateWant_0200
 * @tc.desc: CheckAndUpdateWant_0200
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, CheckAndUpdateWant_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    Want want;
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME, std::string("com.example.test"));
    auto executeMode = AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND;
    std::string callerBundleName = "test";
    int32_t userId = 100;
    auto ret = DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->CheckAndUpdateWant(
        want, executeMode, userId, callerBundleName);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: CheckAndUpdateWant_0300
 * @tc.desc: CheckAndUpdateWant_0300
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, CheckAndUpdateWant_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    Want want;
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME, std::string("com.example.test"));
    auto executeMode = AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND;
    std::string callerBundleName = "test";
    int32_t userId = 100;
    auto ret = DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->CheckAndUpdateWant(
        want, executeMode, userId, callerBundleName);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: ExecuteIntentDone_0100
 * @tc.desc: ExecuteIntentDone_0100
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, ExecuteIntentDone_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    uint64_t intentId = 0;
    int32_t resultCode = 0;
    AppExecFwk::InsightIntentExecuteResult result;
    auto ret = DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->ExecuteIntentDone(
        intentId, resultCode, result);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: ExecuteIntentDone_0200
 * @tc.desc: ExecuteIntentDone_0200
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, ExecuteIntentDone_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    uint64_t intentId = 0;
    int32_t resultCode = 0;
    AppExecFwk::InsightIntentExecuteResult result;
    std::string bundle = "com.bundle.test";
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "test.abilityName";
    param.insightIntentName_ = "PlayMusic";
    param.insightIntentParam_ = std::make_shared<WantParams>();
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    sptr<IRemoteObject> callerToken = new AppExecFwk::MockAbilityToken();
    uint64_t key = 1;
    std::string callerBundleName = "com.bundlename.test";
    DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->AddRecord(
        key, callerToken, param.bundleName_, intentId, callerBundleName);
    intentId = 1;
    auto ret = DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->ExecuteIntentDone(
        intentId, resultCode, result);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: ExecuteIntentDone_0300
 * @tc.desc: ExecuteIntentDone_0300
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, ExecuteIntentDone_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    uint64_t intentId = 0;
    int32_t resultCode = 0;
    AppExecFwk::InsightIntentExecuteResult result;
    DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->records_[intentId] = nullptr;
    auto ret = DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->ExecuteIntentDone(
        intentId, resultCode, result);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: ExecuteIntentDone_0400
 * @tc.desc: ExecuteIntentDone_0400
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, ExecuteIntentDone_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    uint64_t intentId = 0;
    int32_t resultCode = 0;
    AppExecFwk::InsightIntentExecuteResult result;
    auto record = std::make_shared<InsightIntentExecuteRecord>();
    record->state = InsightIntentExecuteState::UNKNOWN;
    DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->records_[intentId] = record;
    auto ret = DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->ExecuteIntentDone(
        intentId, resultCode, result);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GetCallerBundleName_0100
 * @tc.desc: GetCallerBundleName_0100
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, GetCallerBundleName_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    uint64_t intentId = 0;
    std::string callerBundleName = "com.bundlename.test";
    DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->records_.clear();
    auto ret = DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->GetCallerBundleName(
        intentId, callerBundleName);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GetCallerBundleName_0200
 * @tc.desc: GetCallerBundleName_0200
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, GetCallerBundleName_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    uint64_t intentId = 0;
    std::string callerBundleName = "com.bundlename.test";
    auto record = std::make_shared<InsightIntentExecuteRecord>();
    record->state = InsightIntentExecuteState::UNKNOWN;
    DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->records_[intentId] = record;
    auto ret = DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->GetCallerBundleName(
        intentId, callerBundleName);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GetCallerBundleName_0300
 * @tc.desc: GetCallerBundleName_0300
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, GetCallerBundleName_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    uint64_t intentId = 0;
    std::string callerBundleName = "com.bundlename.test";
    DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->records_[intentId] = nullptr;
    auto ret = DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->GetCallerBundleName(
        intentId, callerBundleName);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: AddWantUirsAndFlagsFromParam_001
 * @tc.desc: basic function test of display id.
 * @tc.type: FUNC
 * @tc.require: issueI8ZRAG
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, AddWantUirsAndFlagsFromParam_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    Want want;
    std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> paramPtr = nullptr;
    auto ret = InsightIntentExecuteManager::AddWantUirsAndFlagsFromParam(paramPtr, want);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: AddWantUirsAndFlagsFromParam_002
 * @tc.desc: basic function test of display id.
 * @tc.type: FUNC
 * @tc.require: issueI8ZRAG
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, AddWantUirsAndFlagsFromParam_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    Want want;
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "test.abilityName";
    param.insightIntentName_ = "PlayMusic";
    param.insightIntentParam_ = std::make_shared<WantParams>();
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    paramPtr->uris_.emplace_back("testUri1");
    paramPtr->uris_.emplace_back("testUri2");
    auto ret = InsightIntentExecuteManager::AddWantUirsAndFlagsFromParam(paramPtr, want);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: RemoteDied_0100
 * @tc.desc: RemoteDied_0100
 * @tc.type: FUNC
 * @tc.require: issueI8ZRAG
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, RemoteDied_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    std::shared_ptr<InsightIntentExecuteManager> manager = std::make_shared<InsightIntentExecuteManager>();
    EXPECT_NE(manager, nullptr);
    uint64_t nonExistentIntentId = NON_EXIST_ID;
    int32_t result = manager->RemoteDied(nonExistentIntentId);

    EXPECT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: RemoteDied_0200
 * @tc.desc: RemoteDied_0200
 * @tc.type: FUNC
 * @tc.require: issueI8ZRAG
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, RemoteDied_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    std::shared_ptr<InsightIntentExecuteManager> manager = std::make_shared<InsightIntentExecuteManager>();
    EXPECT_NE(manager, nullptr);
    uint64_t key = 1;
    sptr<IRemoteObject> callToken = new AppExecFwk::MockAbilityToken();
    EXPECT_NE(callToken, nullptr);
    std::string bundleName = "test.bundleName";
    std::string callerBundleName = "test.callerBundleName";
    uint64_t intentId = 1;
    manager->AddRecord(key, callToken, bundleName, intentId, callerBundleName);

    int32_t result = manager->RemoteDied(intentId);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GetBundleName_0100
 * @tc.desc: GetBundleName_0100
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, GetBundleName_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    std::shared_ptr<InsightIntentExecuteManager> manager = std::make_shared<InsightIntentExecuteManager>();
    EXPECT_NE(manager, nullptr);
    uint64_t nonExistentIntentId = NON_EXIST_ID;
    std::string bundleName;

    int32_t result = manager->GetBundleName(nonExistentIntentId, bundleName);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GetBundleName_0200
 * @tc.desc: GetBundleName_0200
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, GetBundleName_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    std::shared_ptr<InsightIntentExecuteManager> manager = std::make_shared<InsightIntentExecuteManager>();
    EXPECT_NE(manager, nullptr);
    uint64_t key = 1;
    sptr<IRemoteObject> callToken = new AppExecFwk::MockAbilityToken();
    EXPECT_NE(callToken, nullptr);
    std::string bundleName = "test.bundleName";
    std::string callerBundleName = "test.callerBundleName";
    uint64_t intentId = 1;
    manager->AddRecord(key, callToken, bundleName, intentId, callerBundleName);

    std::string retBundleName;
    int32_t result = manager->GetBundleName(intentId, retBundleName);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GenerateWant_0300
 * @tc.desc: GenerateWant_0300
 * @tc.type: FUNC
 * @tc.require: issueI8ZRAG
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, GenerateWant_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AbilityRuntime::ExtractInsightIntentGenericInfo decoratorInfo;
    Want want;
    auto ret = InsightIntentExecuteManager::GenerateWant(nullptr, decoratorInfo, want);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GenerateWant_0400
 * @tc.desc: GenerateWant_0400
 * @tc.type: FUNC
 * @tc.require: issueI8ZRAG
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, GenerateWant_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "test.abilityName";
    param.insightIntentName_ = "";
    param.insightIntentParam_ = nullptr;
    param.displayId_ = 2;
    param.executeMode_ = AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND;
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    Want want;
    std::string startTime = want.GetStringParam(Want::PARAM_RESV_START_TIME);
    EXPECT_TRUE(startTime.empty());
    AbilityRuntime::ExtractInsightIntentGenericInfo decoratorInfo;
    InsightIntentExecuteManager::GenerateWant(paramPtr, decoratorInfo, want);
    startTime = want.GetStringParam(Want::PARAM_RESV_START_TIME);
    EXPECT_FALSE(startTime.empty());
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GenerateWant_0500
 * @tc.desc: GenerateWant_0500
 * @tc.type: FUNC
 * @tc.require: issueI8ZRAG
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, GenerateWant_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "test.abilityName";
    param.insightIntentName_ = "test.IntentName";
    param.insightIntentParam_ = nullptr;
    param.displayId_ = 2;
    param.executeMode_ = AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND;
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    AbilityRuntime::ExtractInsightIntentGenericInfo decoratorInfo;
    Want want;
    int32_t result = InsightIntentExecuteManager::GenerateWant(paramPtr, decoratorInfo, want);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GenerateWant_0600
 * @tc.desc: GenerateWant_0600
 * @tc.type: FUNC
 * @tc.require: issueI8ZRAG
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, GenerateWant_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "test.abilityName";
    param.insightIntentName_ = "test.IntentName";
    param.insightIntentParam_ = nullptr;
    param.displayId_ = 2;
    param.executeMode_ = AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND;
    param.isServiceMatch_ = false;
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    AbilityRuntime::ExtractInsightIntentGenericInfo decoratorInfo;
    Want want;
    int32_t result = InsightIntentExecuteManager::GenerateWant(paramPtr, decoratorInfo, want);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GenerateWant_0700
 * @tc.desc: Test GenerateWant with displayId
 * @tc.type: FUNC
 * @tc.require: issueI8ZRAG
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, GenerateWant_0700, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "test.abilityName";
    param.insightIntentName_ = "test.IntentName";
    param.insightIntentParam_ = std::make_shared<WantParams>();
    param.displayId_ = 10; // 有效displayId
    param.executeMode_ = AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND;
    param.isServiceMatch_ = true;
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    AbilityRuntime::ExtractInsightIntentGenericInfo decoratorInfo;
    Want want;
    int32_t result = InsightIntentExecuteManager::GenerateWant(paramPtr, decoratorInfo, want);
    EXPECT_EQ(result, ERR_OK);
    
    // 验证displayId参数是否正确设置
    int32_t displayId = want.GetIntParam(Want::PARAM_RESV_DISPLAY_ID, -1);
    EXPECT_EQ(displayId, 10);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: CheckIntentIsExemption_0100
 * @tc.desc: CheckIntentIsExemption_0100
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, CheckIntentIsExemption_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    std::shared_ptr<InsightIntentExecuteManager> manager = std::make_shared<InsightIntentExecuteManager>();
    EXPECT_NE(manager, nullptr);
    int32_t uid = NON_EXIST_ID;

    bool result = manager->CheckIntentIsExemption(uid);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: CheckIntentIsExemption_0200
 * @tc.desc: CheckIntentIsExemption_0200
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, CheckIntentIsExemption_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    std::shared_ptr<InsightIntentExecuteManager> manager = std::make_shared<InsightIntentExecuteManager>();
    EXPECT_NE(manager, nullptr);
    int32_t uid = 1;
    manager->SetIntentExemptionInfo(uid);

    bool result = manager->CheckIntentIsExemption(uid);
    EXPECT_TRUE(result);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GetAllIntentExemptionInfo_0100
 * @tc.desc: GetAllIntentExemptionInfo_0100
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, GetAllIntentExemptionInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    std::shared_ptr<InsightIntentExecuteManager> manager = std::make_shared<InsightIntentExecuteManager>();
    EXPECT_NE(manager, nullptr);
    int32_t uid1 = 1;
    int32_t uid2 = 2;
    manager->SetIntentExemptionInfo(uid1);
    manager->SetIntentExemptionInfo(uid2);

    auto result = manager->GetAllIntentExemptionInfo();
    EXPECT_EQ(result.size(), 2);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: UpdateFuncDecoratorParams_0100
 * @tc.desc: UpdateFuncDecoratorParams_0100
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, UpdateFuncDecoratorParams_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "test.abilityName";
    param.insightIntentName_ = "PlayMusic";
    param.insightIntentParam_ = nullptr;
    param.executeMode_ = AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND;
    // other member has default value.
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    AbilityRuntime::ExtractInsightIntentInfo ententInfo;
    Want want;
    auto ret = InsightIntentExecuteManager::UpdateFuncDecoratorParams(paramPtr, ententInfo, want);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: UpdateFuncDecoratorParams_0200
 * @tc.desc: UpdateFuncDecoratorParams_0200
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, UpdateFuncDecoratorParams_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.insightIntentName_ = "PlayMusic";
    param.insightIntentParam_ = nullptr;
    param.executeMode_ = AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND;
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    AbilityRuntime::ExtractInsightIntentInfo ententInfo;;
    Want want;
    auto ret = InsightIntentExecuteManager::UpdateFuncDecoratorParams(paramPtr, ententInfo, want);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: UpdateFuncDecoratorParams_0300
 * @tc.desc: UpdateFuncDecoratorParams_0300
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, UpdateFuncDecoratorParams_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "test.abilityName";
    param.insightIntentName_ = "PlayMusic";
    param.insightIntentParam_ = nullptr;
    param.executeMode_ = AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND;
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    AbilityRuntime::ExtractInsightIntentInfo ententInfo;
    ententInfo.decoratorClass = "testClass";
    ententInfo.genericInfo.get<AbilityRuntime::InsightIntentFunctionInfo>().functionName = "";
    Want want;
    auto ret = InsightIntentExecuteManager::UpdateFuncDecoratorParams(paramPtr, ententInfo, want);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: UpdateFuncDecoratorParams_0400
 * @tc.desc: UpdateFuncDecoratorParams_0400
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, UpdateFuncDecoratorParams_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "test.abilityName";
    param.insightIntentName_ = "PlayMusic";
    param.insightIntentParam_ = nullptr;
    param.executeMode_ = AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND;
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    AbilityRuntime::ExtractInsightIntentInfo ententInfo;
    ententInfo.decoratorClass = "";
    ententInfo.genericInfo.get<AbilityRuntime::InsightIntentFunctionInfo>().functionName = "testFunctionName";
    Want want;
    auto ret = InsightIntentExecuteManager::UpdateFuncDecoratorParams(paramPtr, ententInfo, want);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: UpdateFuncDecoratorParams_0500
 * @tc.desc: UpdateFuncDecoratorParams_0500
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, UpdateFuncDecoratorParams_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "test.abilityName";
    param.insightIntentName_ = "PlayMusic";
    param.insightIntentParam_ = nullptr;
    param.executeMode_ = AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND;
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    AbilityRuntime::ExtractInsightIntentInfo ententInfo;
    ententInfo.decoratorClass = "testClass";
    ententInfo.genericInfo.get<AbilityRuntime::InsightIntentFunctionInfo>().functionName = "testFunctionName";
    Want want;
    auto ret = InsightIntentExecuteManager::UpdateFuncDecoratorParams(paramPtr, ententInfo, want);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GetMainElementName_0100
 * @tc.desc: GetMainElementName_0100
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, GetMainElementName_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "test.abilityName";
    param.insightIntentName_ = "";
    param.insightIntentParam_ = nullptr;
    param.displayId_ = 2;
    param.executeMode_ = AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND;
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    paramPtr->moduleName_ = "test.entry";
    std::string retString = InsightIntentExecuteManager::GetMainElementName(paramPtr->bundleName_,
        paramPtr->moduleName_);
    EXPECT_EQ(retString, "");
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: UpdatePageDecoratorParams_0100
 * @tc.desc: UpdatePageDecoratorParams_0100
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, UpdatePageDecoratorParams_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "test.abilityName";
    param.insightIntentName_ = "PlayMusic";
    param.insightIntentParam_ = nullptr;
    param.executeMode_ = AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND;
    // other member has default value.
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    AbilityRuntime::ExtractInsightIntentInfo intentInfo;
    Want want;
    auto ret = InsightIntentExecuteManager::UpdatePageDecoratorParams(paramPtr, intentInfo, want);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: UpdatePageDecoratorParams_0200
 * @tc.desc: UpdatePageDecoratorParams_0200
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, UpdatePageDecoratorParams_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "test.abilityName";
    param.insightIntentName_ = "PlayMusic";
    param.insightIntentParam_ = nullptr;
    param.executeMode_ = AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND;
    // other member has default value.
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    AbilityRuntime::ExtractInsightIntentInfo intentInfo;
    Want want;
    auto ret = InsightIntentExecuteManager::UpdatePageDecoratorParams(paramPtr, intentInfo, want);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: UpdatePageDecoratorParams_0300
 * @tc.desc: UpdatePageDecoratorParams_0300
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, UpdatePageDecoratorParams_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "test.abilityName";
    param.insightIntentName_ = "PlayMusic";
    param.insightIntentParam_ = nullptr;
    param.executeMode_ = AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND;
    // other member has default value.
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    AbilityRuntime::ExtractInsightIntentInfo intentInfo;
    intentInfo.genericInfo.get<AbilityRuntime::InsightIntentPageInfo>().pagePath = "testPagePath";
    Want want;
    auto ret = InsightIntentExecuteManager::UpdatePageDecoratorParams(paramPtr, intentInfo, want);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: UpdatePageDecoratorParams_0400
 * @tc.desc: UpdatePageDecoratorParams_0400
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, UpdatePageDecoratorParams_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "test.abilityName";
    param.insightIntentName_ = "PlayMusic";
    param.insightIntentParam_ = nullptr;
    param.executeMode_ = AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND;
    // other member has default value.
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    AbilityRuntime::ExtractInsightIntentInfo intentInfo;
    intentInfo.genericInfo.get<AbilityRuntime::InsightIntentPageInfo>().pagePath = "test.abilityName";
    intentInfo.genericInfo.get<AbilityRuntime::InsightIntentPageInfo>().uiAbility = "";
    Want want;
    auto ret = InsightIntentExecuteManager::UpdatePageDecoratorParams(paramPtr, intentInfo, want);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: UpdatePageDecoratorParams_0500
 * @tc.desc: UpdatePageDecoratorParams_0500
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, UpdatePageDecoratorParams_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "test.abilityName";
    param.insightIntentName_ = "PlayMusic";
    param.insightIntentParam_ = nullptr;
    param.executeMode_ = AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND;
    // other member has default value.
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    AbilityRuntime::ExtractInsightIntentInfo intentInfo;
    intentInfo.genericInfo.get<AbilityRuntime::InsightIntentPageInfo>().pagePath = "test.abilityName";
    intentInfo.genericInfo.get<AbilityRuntime::InsightIntentPageInfo>().uiAbility = "test.abilityName";
    Want want;
    auto ret = InsightIntentExecuteManager::UpdatePageDecoratorParams(paramPtr, intentInfo, want);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: UpdateEntryDecoratorParams_0100
 * @tc.desc: UpdateEntryDecoratorParams_0100
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, UpdateEntryDecoratorParams_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    Want want;
    auto mode = AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND;
    int32_t userId = 100;
    auto ret = InsightIntentExecuteManager::UpdateEntryDecoratorParams(want, mode, 100);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: UpdateFuncDecoratorParams_0600
 * @tc.desc: basic function test of display id.
 * @tc.type: FUNC
 * @tc.require: issueI8ZRAG
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, UpdateFuncDecoratorParams_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    Want want;
    AppExecFwk::InsightIntentExecuteParam param;
    param.executeMode_ = AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND;
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    AbilityRuntime::ExtractInsightIntentInfo info;
    auto ret = InsightIntentExecuteManager::UpdateFuncDecoratorParams(paramPtr, info, want);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: UpdateFuncDecoratorParams_0700
 * @tc.desc: basic function test of display id.
 * @tc.type: FUNC
 * @tc.require: issueI8ZRAG
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, UpdateFuncDecoratorParams_0700, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    Want want;
    AppExecFwk::InsightIntentExecuteParam param;
    param.executeMode_ = AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "test.abilityName";
    param.insightIntentName_ = "PlayMusic";
    param.insightIntentParam_ = std::make_shared<WantParams>();
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    AbilityRuntime::ExtractInsightIntentInfo info;
    info.decoratorClass = "";
    info.decoratorFile = "decorator.js";
    info.genericInfo.set<AbilityRuntime::InsightIntentFunctionInfo>();
    info.genericInfo.get<AbilityRuntime::InsightIntentFunctionInfo>().functionName = "testFunction";
    info.genericInfo.get<AbilityRuntime::InsightIntentFunctionInfo>().functionParams = {"param1", "param2"};
    auto ret = InsightIntentExecuteManager::UpdateFuncDecoratorParams(paramPtr, info, want);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: UpdateFuncDecoratorParams_0800
 * @tc.desc: basic function test of display id.
 * @tc.type: FUNC
 * @tc.require: issueI8ZRAG
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, UpdateFuncDecoratorParams_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    Want want;
    AppExecFwk::InsightIntentExecuteParam param;
    param.executeMode_ = AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "test.abilityName";
    param.insightIntentName_ = "PlayMusic";
    param.insightIntentParam_ = std::make_shared<WantParams>();
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    AbilityRuntime::ExtractInsightIntentInfo info;
    info.decoratorClass = "com.example.Decorator";
    info.decoratorFile = "decorator.js";
    info.genericInfo.set<AbilityRuntime::InsightIntentFunctionInfo>();
    info.genericInfo.get<AbilityRuntime::InsightIntentFunctionInfo>().functionName = "";
    auto ret = InsightIntentExecuteManager::UpdateFuncDecoratorParams(paramPtr, info, want);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: UpdateFuncDecoratorParams_0900
 * @tc.desc: basic function test of display id.
 * @tc.type: FUNC
 * @tc.require: issueI8ZRAG
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, UpdateFuncDecoratorParams_0900, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    Want want;
    AppExecFwk::InsightIntentExecuteParam param;
    param.executeMode_ = AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "test.abilityName";
    param.insightIntentName_ = "PlayMusic";
    param.insightIntentParam_ = std::make_shared<WantParams>();
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    AbilityRuntime::ExtractInsightIntentInfo info;
    info.decoratorClass = "com.example.Decorator";
    info.decoratorFile = "decorator.js";
    info.genericInfo.set<AbilityRuntime::InsightIntentFunctionInfo>();
    info.genericInfo.get<AbilityRuntime::InsightIntentFunctionInfo>().functionName = "testFunction";
    info.genericInfo.get<AbilityRuntime::InsightIntentFunctionInfo>().functionParams = {"param1", "param2"};
    auto ret = InsightIntentExecuteManager::UpdateFuncDecoratorParams(paramPtr, info, want);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: UpdateFuncDecoratorParams_1000
 * @tc.desc: basic function test of display id.
 * @tc.type: FUNC
 * @tc.require: issueI8ZRAG
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, UpdateFuncDecoratorParams_1000, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    Want want;
    AppExecFwk::InsightIntentExecuteParam param;
    param.executeMode_ = AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND;
    param.abilityName_ = "";
    param.bundleName_ = "";
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    AbilityRuntime::ExtractInsightIntentInfo info;
    auto ret = InsightIntentExecuteManager::UpdateFuncDecoratorParams(paramPtr, info, want);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: UpdatePageDecoratorParams_0600
 * @tc.desc: basic function test of display id.
 * @tc.type: FUNC
 * @tc.require: issueI8ZRAG
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, UpdatePageDecoratorParams_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    Want want;
    AppExecFwk::InsightIntentExecuteParam param;
    param.executeMode_ = AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND;
    param.abilityName_ = "test.abilityName";
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    AbilityRuntime::ExtractInsightIntentInfo info;
    info.decoratorFile = "decorator.js";
    info.decoratorClass = "com.example.Decorator";
    info.genericInfo.set<AbilityRuntime::InsightIntentPageInfo>();
    info.genericInfo.get<AbilityRuntime::InsightIntentPageInfo>().pagePath = "testPagePath";
    info.genericInfo.get<AbilityRuntime::InsightIntentPageInfo>().navigationId = "testNavigationId";
    info.genericInfo.get<AbilityRuntime::InsightIntentPageInfo>().navDestinationName = "testNavDestinationName";
    info.genericInfo.get<AbilityRuntime::InsightIntentPageInfo>().uiAbility = "testUIAbilityName";
    auto ret = InsightIntentExecuteManager::UpdatePageDecoratorParams(paramPtr, info, want);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: UpdatePageDecoratorParams_0700
 * @tc.desc: basic function test of display id.
 * @tc.type: FUNC
 * @tc.require: issueI8ZRAG
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, UpdatePageDecoratorParams_0700, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    Want want;
    AppExecFwk::InsightIntentExecuteParam param;
    param.executeMode_ = AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND;
    param.abilityName_ = "test.abilityName";
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    AbilityRuntime::ExtractInsightIntentInfo info;
    info.decoratorFile = "decorator.js";
    info.decoratorClass = "com.example.Decorator";
    info.genericInfo.set<AbilityRuntime::InsightIntentPageInfo>();
    info.genericInfo.get<AbilityRuntime::InsightIntentPageInfo>().pagePath = "";
    info.genericInfo.get<AbilityRuntime::InsightIntentPageInfo>().navigationId = "testNavigationId";
    info.genericInfo.get<AbilityRuntime::InsightIntentPageInfo>().navDestinationName = "testNavDestinationName";
    info.genericInfo.get<AbilityRuntime::InsightIntentPageInfo>().uiAbility = "test.abilityName";
    auto ret = InsightIntentExecuteManager::UpdatePageDecoratorParams(paramPtr, info, want);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: UpdatePageDecoratorParams_0800
 * @tc.desc: basic function test of display id.
 * @tc.type: FUNC
 * @tc.require: issueI8ZRAG
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, UpdatePageDecoratorParams_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    Want want;
    AppExecFwk::InsightIntentExecuteParam param;
    param.executeMode_ = AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND;
    param.abilityName_ = "test.abilityName";
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    AbilityRuntime::ExtractInsightIntentInfo info;
    info.decoratorFile = "decorator.js";
    info.decoratorClass = "com.example.Decorator";
    info.genericInfo.set<AbilityRuntime::InsightIntentPageInfo>();
    info.genericInfo.get<AbilityRuntime::InsightIntentPageInfo>().pagePath = "";
    info.genericInfo.get<AbilityRuntime::InsightIntentPageInfo>().navigationId = "testNavigationId";
    info.genericInfo.get<AbilityRuntime::InsightIntentPageInfo>().navDestinationName = "testNavDestinationName";
    info.genericInfo.get<AbilityRuntime::InsightIntentPageInfo>().uiAbility = "test.abilityName";
    auto ret = InsightIntentExecuteManager::UpdatePageDecoratorParams(paramPtr, info, want);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name:UpdateEntryDecoratorParams_0200
 * @tc.desc: basic function test of display id.
 * @tc.type: FUNC
 * @tc.require: issueI8ZRAG
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, UpdateEntryDecoratorParams_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    Want want;
    AppExecFwk::InsightIntentExecuteParam param;
    param.executeMode_ = -1;
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    AbilityRuntime::ExtractInsightIntentInfo info;
    auto ret = InsightIntentExecuteManager::UpdateEntryDecoratorParams(paramPtr, info, want);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: OnInsightAppDied_001
 * @tc.desc: OnInsightAppDied_001
 * @tc.type: FUNC
 * @tc.require: issueI8ZRAG
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, OnInsightAppDied_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentExecuteManagerSecondTest OnInsightAppDied_001 start");
    std::shared_ptr<InsightIntentExecuteManager> manager = std::make_shared<InsightIntentExecuteManager>();
    EXPECT_NE(manager, nullptr);
    uint64_t key = 1;
    sptr<IRemoteObject> callToken = new AppExecFwk::MockAbilityToken();
    EXPECT_NE(callToken, nullptr);
    std::string bundleName = "test.bundleName";
    std::string callerBundleName = "test.callerBundleName";
    uint64_t intentId = 1;
    manager->AddRecord(key, callToken, bundleName, intentId, callerBundleName);
    EXPECT_EQ(manager->records_.size(), 1);
    manager->OnInsightAppDied(bundleName);
    EXPECT_EQ(manager->records_.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentExecuteManagerSecondTest OnInsightAppDied_001 end");
}

/**
 * @tc.name: OnInsightAppDied_002
 * @tc.desc: Test OnInsightAppDied with multiple records
 * @tc.type: FUNC
 * @tc.require: issueI8ZRAG
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, OnInsightAppDied_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentExecuteManagerSecondTest OnInsightAppDied_002 start");
    std::shared_ptr<InsightIntentExecuteManager> manager = std::make_shared<InsightIntentExecuteManager>();
    EXPECT_NE(manager, nullptr);
    
    // 添加多条不同bundle的记录
    uint64_t key1 = 1;
    uint64_t key2 = 2;
    uint64_t key3 = 3;
    sptr<IRemoteObject> callToken = new AppExecFwk::MockAbilityToken();
    EXPECT_NE(callToken, nullptr);
    
    manager->AddRecord(key1, callToken, "test.bundle1", key1, "caller1");
    manager->AddRecord(key2, callToken, "test.bundle2", key2, "caller2");
    manager->AddRecord(key3, callToken, "test.bundle1", key3, "caller3");
    
    EXPECT_EQ(manager->records_.size(), 3);
    
    // 触发bundle1的死亡回调
    manager->OnInsightAppDied("test.bundle1");
    
    // 验证只有bundle1的记录被移除
    EXPECT_EQ(manager->records_.size(), 1);
    auto findResult = manager->records_.find(key2);
    EXPECT_NE(findResult, manager->records_.end());
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentExecuteManagerSecondTest OnInsightAppDied_002 end");
}

/**
 * @tc.name: IsValidCall_0100
 * @tc.desc: Test IsValidCall with empty name
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, IsValidCall_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    Want want;
    std::shared_ptr<InsightIntentExecuteManager> manager = std::make_shared<InsightIntentExecuteManager>();
    auto ret = manager->IsValidCall(want);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: RemoveExecuteIntent_0100
 * @tc.desc: Test RemoveExecuteIntent
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, RemoveExecuteIntent_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    uint64_t intentId = 999;
    auto record = std::make_shared<InsightIntentExecuteRecord>();
    std::shared_ptr<InsightIntentExecuteManager> manager = std::make_shared<InsightIntentExecuteManager>();
    manager->records_[intentId] = record;

    auto ret = manager->RemoveExecuteIntent(intentId);
    EXPECT_EQ(ret, ERR_OK);

    auto findResult = manager->records_.find(intentId);
    EXPECT_EQ(findResult, manager->records_.end());
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: CheckAndUpdateDecoratorParams_0100
 * @tc.desc: Test CheckAndUpdateDecoratorParams with different decorator types
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, CheckAndUpdateDecoratorParams_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "test.abilityName";
    param.insightIntentName_ = "test.IntentName";
    param.executeMode_ = AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND;
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    AbilityRuntime::ExtractInsightIntentGenericInfo decoratorInfo;
    decoratorInfo.decoratorType = AbilityRuntime::INSIGHT_INTENTS_DECORATOR_TYPE_LINK;
    Want want;
    
    std::shared_ptr<InsightIntentExecuteManager> manager = std::make_shared<InsightIntentExecuteManager>();
    auto ret = manager->CheckAndUpdateDecoratorParams(
        paramPtr, decoratorInfo, want);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: CheckAndUpdateQueryEntityParam_0100
 * @tc.desc: Test CheckAndUpdateQueryEntityParam with null callerToken
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, CheckAndUpdateQueryEntityParam_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckAndUpdateQueryEntityParam_0100 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    AppExecFwk::InsightIntentQueryParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.module";
    param.intentName_ = "test.intent";
    param.className_ = "test.className";
    param.queryEntityParam_.parameters_ = std::make_shared<WantParams>();
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentQueryParam>(param);

    sptr<IRemoteObject> callerToken = nullptr;
    uint64_t key = 1;
    std::string callerBundleName = "com.bundlename.test";

    auto ret = manager->CheckAndUpdateQueryEntityParam(key, callerToken, paramPtr, callerBundleName);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "CheckAndUpdateQueryEntityParam_0100 end.");
}

/**
 * @tc.name: CheckAndUpdateQueryEntityParam_0200
 * @tc.desc: Test CheckAndUpdateQueryEntityParam with null param
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, CheckAndUpdateQueryEntityParam_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckAndUpdateQueryEntityParam_0200 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    std::shared_ptr<AppExecFwk::InsightIntentQueryParam> paramPtr = nullptr;
    sptr<IRemoteObject> callerToken = new AppExecFwk::MockAbilityToken();
    uint64_t key = 1;
    std::string callerBundleName = "com.bundlename.test";

    auto ret = manager->CheckAndUpdateQueryEntityParam(key, callerToken, paramPtr, callerBundleName);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "CheckAndUpdateQueryEntityParam_0200 end.");
}

/**
 * @tc.name: CheckAndUpdateQueryEntityParam_0300
 * @tc.desc: Test CheckAndUpdateQueryEntityParam with empty bundleName
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, CheckAndUpdateQueryEntityParam_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckAndUpdateQueryEntityParam_0300 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    AppExecFwk::InsightIntentQueryParam param;
    param.bundleName_ = "";  // empty bundleName
    param.moduleName_ = "test.module";
    param.intentName_ = "test.intent";
    param.className_ = "test.className";
    param.queryEntityParam_.parameters_ = std::make_shared<WantParams>();
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentQueryParam>(param);

    sptr<IRemoteObject> callerToken = new AppExecFwk::MockAbilityToken();
    uint64_t key = 1;
    std::string callerBundleName = "com.bundlename.test";

    auto ret = manager->CheckAndUpdateQueryEntityParam(key, callerToken, paramPtr, callerBundleName);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "CheckAndUpdateQueryEntityParam_0300 end.");
}

/**
 * @tc.name: CheckAndUpdateQueryEntityParam_0400
 * @tc.desc: Test CheckAndUpdateQueryEntityParam with empty moduleName
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, CheckAndUpdateQueryEntityParam_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckAndUpdateQueryEntityParam_0400 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    AppExecFwk::InsightIntentQueryParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "";
    param.intentName_ = "test.intent";
    param.className_ = "test.className";
    param.queryEntityParam_.parameters_ = std::make_shared<WantParams>();
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentQueryParam>(param);

    sptr<IRemoteObject> callerToken = new AppExecFwk::MockAbilityToken();
    uint64_t key = 1;
    std::string callerBundleName = "com.bundlename.test";

    auto ret = manager->CheckAndUpdateQueryEntityParam(key, callerToken, paramPtr, callerBundleName);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "CheckAndUpdateQueryEntityParam_0400 end.");
}

/**
 * @tc.name: CheckAndUpdateQueryEntityParam_0500
 * @tc.desc: Test CheckAndUpdateQueryEntityParam with empty intentName
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, CheckAndUpdateQueryEntityParam_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckAndUpdateQueryEntityParam_0500 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    AppExecFwk::InsightIntentQueryParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.module";
    param.intentName_ = "";
    param.className_ = "test.className";
    param.queryEntityParam_.parameters_ = std::make_shared<WantParams>();
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentQueryParam>(param);

    sptr<IRemoteObject> callerToken = new AppExecFwk::MockAbilityToken();
    uint64_t key = 1;
    std::string callerBundleName = "com.bundlename.test";

    auto ret = manager->CheckAndUpdateQueryEntityParam(key, callerToken, paramPtr, callerBundleName);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "CheckAndUpdateQueryEntityParam_0500 end.");
}

/**
 * @tc.name: CheckAndUpdateQueryEntityParam_0600
 * @tc.desc: Test CheckAndUpdateQueryEntityParam with empty className
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, CheckAndUpdateQueryEntityParam_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckAndUpdateQueryEntityParam_0600 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    AppExecFwk::InsightIntentQueryParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.module";
    param.intentName_ = "test.intent";
    param.className_ = "";
    param.queryEntityParam_.parameters_ = std::make_shared<WantParams>();
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentQueryParam>(param);

    sptr<IRemoteObject> callerToken = new AppExecFwk::MockAbilityToken();
    uint64_t key = 1;
    std::string callerBundleName = "com.bundlename.test";

    auto ret = manager->CheckAndUpdateQueryEntityParam(key, callerToken, paramPtr, callerBundleName);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "CheckAndUpdateQueryEntityParam_0600 end.");
}

/**
 * @tc.name: CheckAndUpdateQueryEntityParam_0700
 * @tc.desc: Test CheckAndUpdateQueryEntityParam with valid params
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, CheckAndUpdateQueryEntityParam_0700, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckAndUpdateQueryEntityParam_0700 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    AppExecFwk::InsightIntentQueryParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.module";
    param.intentName_ = "test.intent";
    param.className_ = "test.className";
    param.userId_ = 100;
    param.queryEntityParam_.parameters_ = std::make_shared<WantParams>();
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentQueryParam>(param);

    sptr<IRemoteObject> callerToken = new AppExecFwk::MockAbilityToken();
    uint64_t key = 1001;
    std::string callerBundleName = "com.bundlename.test";

    auto ret = manager->CheckAndUpdateQueryEntityParam(key, callerToken, paramPtr, callerBundleName);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_NE(paramPtr->intentId_, 0);

    TAG_LOGI(AAFwkTag::TEST, "CheckAndUpdateQueryEntityParam_0700 end.");
}

/**
 * @tc.name: GenerateQueryEntityWant_0100
 * @tc.desc: Test GenerateQueryEntityWant with null param
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, GenerateQueryEntityWant_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GenerateQueryEntityWant_0100 begin.");

    std::shared_ptr<AppExecFwk::InsightIntentQueryParam> paramPtr = nullptr;
    Want want;

    auto ret = InsightIntentExecuteManager::GenerateQueryEntityWant(paramPtr, want);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "GenerateQueryEntityWant_0100 end.");
}

/**
 * @tc.name: CheckCanQueryEntity_0100
 * @tc.desc: Test CheckCanQueryEntity with empty className
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, CheckCanQueryEntity_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckCanQueryEntity_0100 begin.");

    AbilityRuntime::ExtractInsightIntentInfo intentInfo;
    std::string className = "";
    auto queryParams = std::make_shared<WantParams>();
    AppExecFwk::InsightIntentQueryEntityParam param;
    param.parameters_ = queryParams;
    auto ret = InsightIntentExecuteManager::CheckEntityQueryable(intentInfo, className, param);
    EXPECT_FALSE(ret);

    TAG_LOGI(AAFwkTag::TEST, "CheckCanQueryEntity_0100 end.");
}

/**
 * @tc.name: CheckCanQueryEntity_0200
 * @tc.desc: Test CheckCanQueryEntity with empty entities
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, CheckCanQueryEntity_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckCanQueryEntity_0200 begin.");

    AbilityRuntime::ExtractInsightIntentInfo intentInfo;
    intentInfo.entities.clear();
    std::string className = "test.className";
    auto queryParams = std::make_shared<WantParams>();
    AppExecFwk::InsightIntentQueryEntityParam param;
    param.parameters_ = queryParams;
    auto ret = InsightIntentExecuteManager::CheckEntityQueryable(intentInfo, className, param);
    EXPECT_FALSE(ret);

    TAG_LOGI(AAFwkTag::TEST, "CheckCanQueryEntity_0200 end.");
}

/**
 * @tc.name: CheckCanQueryEntity_0300
 * @tc.desc: Test CheckCanQueryEntity with entity not queryable
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, CheckCanQueryEntity_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckCanQueryEntity_0300 begin.");

    AbilityRuntime::ExtractInsightIntentInfo intentInfo;
    AbilityRuntime::InsightIntentEntityInfo entity;
    entity.className = "test.className";
    entity.parentClassName = "not.insightIntent.AppIntentEntity";
    intentInfo.entities.push_back(entity);

    std::string className = "test.className";
    auto queryParams = std::make_shared<WantParams>();
    AppExecFwk::InsightIntentQueryEntityParam param;
    param.parameters_ = queryParams;
    auto ret = InsightIntentExecuteManager::CheckEntityQueryable(intentInfo, className, param);
    EXPECT_FALSE(ret);

    TAG_LOGI(AAFwkTag::TEST, "CheckCanQueryEntity_0300 end.");
}

/**
 * @tc.name: CheckCanQueryEntity_0400
 * @tc.desc: Test CheckCanQueryEntity with className not match
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, CheckCanQueryEntity_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckCanQueryEntity_0400 begin.");

    AbilityRuntime::ExtractInsightIntentInfo intentInfo;
    AbilityRuntime::InsightIntentEntityInfo entity;
    entity.className = "other.className";
    entity.parentClassName = "insightIntent.AppIntentEntity";
    intentInfo.entities.push_back(entity);

    std::string className = "test.className";
    auto queryParams = std::make_shared<WantParams>();
    AppExecFwk::InsightIntentQueryEntityParam param;
    param.parameters_ = queryParams;
    auto ret = InsightIntentExecuteManager::CheckEntityQueryable(intentInfo, className, param);
    EXPECT_FALSE(ret);

    TAG_LOGI(AAFwkTag::TEST, "CheckCanQueryEntity_0400 end.");
}

/**
 * @tc.name: CheckCanQueryEntity_0500
 * @tc.desc: Test CheckCanQueryEntity with queryable entity
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, CheckCanQueryEntity_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckCanQueryEntity_0500 begin.");

    AbilityRuntime::ExtractInsightIntentInfo intentInfo;
    AbilityRuntime::InsightIntentEntityInfo entity;
    entity.className = "test.className";
    entity.parentClassName = "insightIntent.AppIntentEntity";
    entity.supportedQueryProperties = {"prop1", "prop2"};
    intentInfo.entities.push_back(entity);

    std::string className = "test.className";
    auto queryParams = std::make_shared<WantParams>();
    AppExecFwk::InsightIntentQueryEntityParam param;
    param.parameters_ = queryParams;
    auto ret = InsightIntentExecuteManager::CheckEntityQueryable(intentInfo, className, param);
    EXPECT_TRUE(ret);

    TAG_LOGI(AAFwkTag::TEST, "CheckCanQueryEntity_0500 end.");
}

/**
 * @tc.name: CheckCanQueryEntity_0600
 * @tc.desc: Test CheckCanQueryEntity with multiple entities
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, CheckCanQueryEntity_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckCanQueryEntity_0600 begin.");

    AbilityRuntime::ExtractInsightIntentInfo intentInfo;

    AbilityRuntime::InsightIntentEntityInfo entity1;
    entity1.className = "test.className";
    entity1.parentClassName = "not.queryable";
    intentInfo.entities.push_back(entity1);

    AbilityRuntime::InsightIntentEntityInfo entity2;
    entity2.className = "test.className";
    entity2.parentClassName = "insightIntent.AppIntentEntity";
    entity2.supportedQueryProperties = {"prop1"};
    intentInfo.entities.push_back(entity2);

    std::string className = "test.className";
    auto queryParams = std::make_shared<WantParams>();
    AppExecFwk::InsightIntentQueryEntityParam param;
    param.parameters_ = queryParams;
    auto ret = InsightIntentExecuteManager::CheckEntityQueryable(intentInfo, className, param);
    EXPECT_TRUE(ret);

    TAG_LOGI(AAFwkTag::TEST, "CheckCanQueryEntity_0600 end.");
}

/**
 * @tc.name: CheckCanQueryEntity_0700
 * @tc.desc: Test CheckCanQueryEntity with empty queryParams
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, CheckCanQueryEntity_0700, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckCanQueryEntity_0700 begin.");

    AbilityRuntime::ExtractInsightIntentInfo intentInfo;
    AbilityRuntime::InsightIntentEntityInfo entity;
    entity.className = "test.className";
    entity.parentClassName = "insightIntent.AppIntentEntity";
    entity.supportedQueryProperties = {"prop1", "prop2"};
    intentInfo.entities.push_back(entity);

    std::string className = "test.className";
    auto queryParams = std::make_shared<WantParams>();
    AppExecFwk::InsightIntentQueryEntityParam param;
    param.parameters_ = queryParams;
    auto ret = InsightIntentExecuteManager::CheckEntityQueryable(intentInfo, className, param);
    EXPECT_TRUE(ret);

    TAG_LOGI(AAFwkTag::TEST, "CheckCanQueryEntity_0700 end.");
}

/**
 * @tc.name: InsightIntentEntityInfo_IsQueryable_0100
 * @tc.desc: Test InsightIntentEntityInfo::isQueryable with matching parentClassName.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, InsightIntentEntityInfo_IsQueryable_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentEntityInfo_IsQueryable_0100 begin.");

    AbilityRuntime::InsightIntentEntityInfo entity;
    entity.className = "TestClass";
    entity.entityId = "testId";
    entity.parentClassName = "insightIntent.AppIntentEntity";
    EXPECT_TRUE(entity.isQueryable());

    TAG_LOGI(AAFwkTag::TEST, "InsightIntentEntityInfo_IsQueryable_0100 end.");
}

/**
 * @tc.name: InsightIntentEntityInfo_IsQueryable_0200
 * @tc.desc: Test InsightIntentEntityInfo::isQueryable with non-matching parentClassName.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, InsightIntentEntityInfo_IsQueryable_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentEntityInfo_IsQueryable_0200 begin.");

    AbilityRuntime::InsightIntentEntityInfo entity;
    entity.className = "TestClass";
    entity.entityId = "testId";
    entity.parentClassName = "other.ParentClass";
    EXPECT_FALSE(entity.isQueryable());

    TAG_LOGI(AAFwkTag::TEST, "InsightIntentEntityInfo_IsQueryable_0200 end.");
}

/**
 * @tc.name: InsightIntentEntityInfo_IsQueryable_0300
 * @tc.desc: Test InsightIntentEntityInfo::isQueryable with empty parentClassName.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, InsightIntentEntityInfo_IsQueryable_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentEntityInfo_IsQueryable_0300 begin.");

    AbilityRuntime::InsightIntentEntityInfo entity;
    entity.className = "TestClass";
    entity.entityId = "testId";
    entity.parentClassName = "";
    EXPECT_FALSE(entity.isQueryable());

    TAG_LOGI(AAFwkTag::TEST, "InsightIntentEntityInfo_IsQueryable_0300 end.");
}

/**
 * @tc.name: GenerateQueryEntityWant_0200
 * @tc.desc: Test GenerateQueryEntityWant with CheckCanQueryEntity returning false.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, GenerateQueryEntityWant_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GenerateQueryEntityWant_0200 begin.");

    AppExecFwk::InsightIntentQueryParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.module";
    param.intentName_ = "test.intent";
    param.className_ = "test.className";
    param.queryEntityParam_.queryType_ = "all";
    param.queryEntityParam_.parameters_ = std::make_shared<WantParams>();
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentQueryParam>(param);

    Want want;
    auto ret = InsightIntentExecuteManager::GenerateQueryEntityWant(paramPtr, want);
    EXPECT_EQ(ret, INTENT_NOT_EXIST);

    TAG_LOGI(AAFwkTag::TEST, "GenerateQueryEntityWant_0200 end.");
}

/**
 * @tc.name: CheckAndUpdateQueryEntityParam_0800
 * @tc.desc: Test CheckAndUpdateQueryEntityParam with userId set to default value.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, CheckAndUpdateQueryEntityParam_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckAndUpdateQueryEntityParam_0800 begin.");

    auto manager = std::make_shared<InsightIntentExecuteManager>();
    ASSERT_NE(manager, nullptr);

    AppExecFwk::InsightIntentQueryParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.module";
    param.intentName_ = "test.intent";
    param.className_ = "test.className";
    param.userId_ = -1;
    param.queryEntityParam_.parameters_ = std::make_shared<WantParams>();
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentQueryParam>(param);

    sptr<IRemoteObject> callerToken = new AppExecFwk::MockAbilityToken();
    uint64_t key = 2001;
    std::string callerBundleName = "com.bundlename.test";

    auto ret = manager->CheckAndUpdateQueryEntityParam(key, callerToken, paramPtr, callerBundleName);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_NE(paramPtr->userId_, -1);

    TAG_LOGI(AAFwkTag::TEST, "CheckAndUpdateQueryEntityParam_0800 end.");
}

/**
 * @tc.name: CheckCanQueryEntity_0800
 * @tc.desc: Test CheckCanQueryEntity with query key not in supported properties.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, CheckCanQueryEntity_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckCanQueryEntity_0800 begin.");

    AbilityRuntime::ExtractInsightIntentInfo intentInfo;
    AbilityRuntime::InsightIntentEntityInfo entity;
    entity.className = "test.className";
    entity.parentClassName = "insightIntent.AppIntentEntity";
    entity.supportedQueryProperties = {"prop1", "prop2"};
    intentInfo.entities.push_back(entity);

    std::string className = "test.className";
    auto queryParams = std::make_shared<WantParams>();
    queryParams->SetParam("unsupportedKey", AAFwk::String::Box("value"));
    AppExecFwk::InsightIntentQueryEntityParam param;
    param.queryType_ = "all";
    param.parameters_ = queryParams;
    auto ret = InsightIntentExecuteManager::CheckEntityQueryable(intentInfo, className, param);
    EXPECT_TRUE(ret);

    param.queryType_ = "byProperty";
    param.parameters_ = queryParams;
    ret = InsightIntentExecuteManager::CheckEntityQueryable(intentInfo, className, param);
    EXPECT_FALSE(ret);

    param.queryType_ = "byProperty";
    param.parameters_ = nullptr;
    ret = InsightIntentExecuteManager::CheckEntityQueryable(intentInfo, className, param);
    EXPECT_FALSE(ret);

    TAG_LOGI(AAFwkTag::TEST, "CheckCanQueryEntity_0800 end.");
}

/**
 * @tc.name: CheckCanQueryEntity_0900
 * @tc.desc: Test CheckCanQueryEntity with query key in supported properties.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteManagerSecondTest, CheckCanQueryEntity_0900, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckCanQueryEntity_0900 begin.");

    AbilityRuntime::ExtractInsightIntentInfo intentInfo;
    AbilityRuntime::InsightIntentEntityInfo entity;
    entity.className = "test.className";
    entity.parentClassName = "insightIntent.AppIntentEntity";
    entity.supportedQueryProperties = {"prop1", "prop2"};
    intentInfo.entities.push_back(entity);

    std::string className = "test.className";
    auto queryParams = std::make_shared<WantParams>();
    queryParams->SetParam("prop1", AAFwk::String::Box("value"));
    AppExecFwk::InsightIntentQueryEntityParam param;
    param.parameters_ = queryParams;
    auto ret = InsightIntentExecuteManager::CheckEntityQueryable(intentInfo, className, param);
    EXPECT_TRUE(ret);

    TAG_LOGI(AAFwkTag::TEST, "CheckCanQueryEntity_0900 end.");
}

} // namespace AAFwk
} // namespace OHOS
