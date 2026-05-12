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

#include <chrono>
#include <deque>

#define private public
#define protected public
#include "ability_manager_service.h"
#include "extract_insight_intent_profile.h"
#include "session_info.h"
#include "sub_managers_helper.h"
#include "insight_intent_query_param.h"
#undef private
#undef protected
#include "ability_manager_errors.h"
#include "start_ability_utils.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
namespace {
    constexpr char INSIGHT_INTENT_EXECUTE_PARAM_NAME[] = "ohos.insightIntent.executeParam.name";
}
class AbilityManagerServiceMockTest : public Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};
    
void AbilityManagerServiceMockTest::SetUpTestCase() {}
    
void AbilityManagerServiceMockTest::TearDownTestCase() {}
    
void AbilityManagerServiceMockTest::SetUp() {}
    
void AbilityManagerServiceMockTest::TearDown() {}

/**
* @tc.name: ExecuteIntent_0400
* @tc.desc: Test  StartExtensionAbilityWithInsightIntent error
* @tc.type: FUNC
*/
HWTEST_F(AbilityManagerServiceMockTest, InsightIntentExecuteParam_0100, TestSize.Level1)
{
    Want want;
    auto ret = AppExecFwk::InsightIntentExecuteParam::IsInsightIntentExecute(want);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name: InsightIntentExecuteParam_0200
* @tc.desc: Test  StartExtensionAbilityWithInsightIntent error
* @tc.type: FUNC
*/
HWTEST_F(AbilityManagerServiceMockTest, InsightIntentExecuteParam_0200, TestSize.Level1)
{
    Want want;
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME, 1);
    auto ret = AppExecFwk::InsightIntentExecuteParam::IsInsightIntentExecute(want);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name: InsightIntentExecuteParam_0300
* @tc.desc: Test  StartExtensionAbilityWithInsightIntent error
* @tc.type: FUNC
*/
HWTEST_F(AbilityManagerServiceMockTest, InsightIntentExecuteParam_0300, TestSize.Level1)
{
    Want want;
    auto ret = AppExecFwk::InsightIntentExecuteParam::IsInsightIntentPage(want);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name: InsightIntentExecuteParam_0400
* @tc.desc: Test  StartExtensionAbilityWithInsightIntent error
* @tc.type: FUNC
*/
HWTEST_F(AbilityManagerServiceMockTest, InsightIntentExecuteParam_0400, TestSize.Level1)
{
    Want want;
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME, 1);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_DECORATOR_TYPE, 1);
    auto ret = AppExecFwk::InsightIntentExecuteParam::IsInsightIntentPage(want);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name: InsightIntentExecuteParam_0500
* @tc.desc: Test  StartExtensionAbilityWithInsightIntent error
* @tc.type: FUNC
*/
HWTEST_F(AbilityManagerServiceMockTest, InsightIntentExecuteParam_0500, TestSize.Level1)
{
    Want want;
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME, 1);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_DECORATOR_TYPE, 2);
    auto ret = AppExecFwk::InsightIntentExecuteParam::IsInsightIntentPage(want);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name: InsightIntentExecuteParam_0600
* @tc.desc: Test  StartExtensionAbilityWithInsightIntent error
* @tc.type: FUNC
*/
HWTEST_F(AbilityManagerServiceMockTest, InsightIntentExecuteParam_0600, TestSize.Level1)
{
    Want want;
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME, 1);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_DECORATOR_TYPE, 2);
    auto ret = AppExecFwk::InsightIntentExecuteParam::IsInsightIntentPage(want);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name: InsightIntentExecuteParam_0700
* @tc.desc: Test  StartExtensionAbilityWithInsightIntent error
* @tc.type: FUNC
*/
HWTEST_F(AbilityManagerServiceMockTest, InsightIntentExecuteParam_0700, TestSize.Level1)
{
    Want want;
    AppExecFwk::InsightIntentExecuteParam param;
    auto ret = AppExecFwk::InsightIntentExecuteParam::GenerateFromWant(want, param);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name: InsightIntentExecuteParam_0800
* @tc.desc: Test  StartExtensionAbilityWithInsightIntent error
* @tc.type: FUNC
*/
HWTEST_F(AbilityManagerServiceMockTest, InsightIntentExecuteParam_0800, TestSize.Level1)
{
    Want want;
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME, 1);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_ID, std::string("error"));
    AppExecFwk::InsightIntentExecuteParam param;
    auto ret = AppExecFwk::InsightIntentExecuteParam::GenerateFromWant(want, param);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name: InsightIntentExecuteParam_0900
* @tc.desc: Test  StartExtensionAbilityWithInsightIntent error
* @tc.type: FUNC
*/
HWTEST_F(AbilityManagerServiceMockTest, InsightIntentExecuteParam_0900, TestSize.Level1)
{
    Want want;
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME, 1);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_ID, std::string("error"));
    AppExecFwk::InsightIntentExecuteParam param;
    auto ret = AppExecFwk::InsightIntentExecuteParam::GenerateFromWant(want, param);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name: InsightIntentExecuteParam_1000
* @tc.desc: Test  StartExtensionAbilityWithInsightIntent error
* @tc.type: FUNC
*/
HWTEST_F(AbilityManagerServiceMockTest, InsightIntentExecuteParam_1000, TestSize.Level1)
{
    Want want;
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME, 1);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_ID, 1);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_MODE, 1);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_PARAM, 1);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_SRC_ENTRY, 1);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_URI, 1);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_FLAGS, 1);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_OPENLINK_FLAG, 1);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_DECORATOR_TYPE, 1);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_SRC_ENTRANCE, 1);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_FUNC_PARAM_CLASSNAME, 1);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_FUNC_PARAM_METHODNAME, 1);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_FUNC_PARAM_METHODPARAMS, 1);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_PAGE_PARAM_PAGEPATH, 1);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_PAGE_PARAM_NAVIGATIONID, 1);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_PAGE_PARAM_NAVDESTINATIONNAME, 1);

    auto ret = AppExecFwk::InsightIntentExecuteParam::RemoveInsightIntent(want);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: ExecuteIntent_0100
 * @tc.desc: Test  StartAbilityByCallWithInsightIntent error
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, ExecuteIntent_0100, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "ability1";
    param.insightIntentName_ = "test1";
    param.insightIntentParam_ = std::make_shared<WantParams>();
    param.executeMode_ = AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND;
    std::shared_ptr<AbilityRecord> ability = nullptr;
    sptr<IRemoteObject> callerToken = new OHOS::AAFwk::Token(ability);
    uint64_t key = 1;
    auto ret = abilityMs->ExecuteIntent(key, callerToken, param);
    EXPECT_EQ(ret, RESOLVE_ABILITY_ERR);
}

/**
 * @tc.name: ExecuteIntent_0200
 * @tc.desc: Test  StartAbilityWithInsightIntent error
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, ExecuteIntent_0200, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "ability1";
    param.insightIntentName_ = "test1";
    param.insightIntentParam_ = std::make_shared<WantParams>();
    param.executeMode_ = AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND;
    std::shared_ptr<AbilityRecord> ability = nullptr;
    sptr<IRemoteObject> callerToken = new OHOS::AAFwk::Token(ability);
    uint64_t key = 1;
    auto ret = abilityMs->ExecuteIntent(key, callerToken, param);
    EXPECT_EQ(ret, ERR_NULL_INTERCEPTOR_EXECUTER);
}

/**
 * @tc.name: ExecuteIntent_0300
 * @tc.desc: Test  StartExtensionAbilityWithInsightIntent error
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, ExecuteIntent_0300, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "ability1";
    param.insightIntentName_ = "test1";
    param.insightIntentParam_ = std::make_shared<WantParams>();
    param.executeMode_ = AppExecFwk::ExecuteMode::SERVICE_EXTENSION_ABILITY;
    std::shared_ptr<AbilityRecord> ability = nullptr;
    sptr<IRemoteObject> callerToken = new OHOS::AAFwk::Token(ability);
    uint64_t key = 1;
    auto ret = abilityMs->ExecuteIntent(key, callerToken, param);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: ExecuteIntent_0400
 * @tc.desc: Test ExecuteIntent distributed branch permission denied
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, ExecuteIntent_0400, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);

    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "ability1";
    param.insightIntentName_ = "test1";
    param.insightIntentParam_ = std::make_shared<WantParams>();
    param.executeMode_ = AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND;
    param.deviceId_ = "remoteDevice";

    std::shared_ptr<AbilityRecord> ability = nullptr;
    sptr<IRemoteObject> callerToken = new OHOS::AAFwk::Token(ability);
    uint64_t key = 1;

    auto ret = abilityMs->ExecuteIntent(key, callerToken, param);
    EXPECT_EQ(ret, ERR_INTENT_CONNECTION_FAILED);
}

/**
 * @tc.name: ExecuteIntent_0500
 * @tc.desc: Test ExecuteIntent distributed branch with flood attack
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, ExecuteIntent_0500, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);

    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "ability1";
    param.insightIntentName_ = "test1";
    param.insightIntentParam_ = std::make_shared<WantParams>();
    param.executeMode_ = AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND;
    param.deviceId_ = "remoteDevice";

    int32_t callerUid = IPCSkeleton::GetCallingUid();
    int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    abilityMs->floodAttackStatistics_[callerUid] = std::deque<int64_t>(10, now);

    std::shared_ptr<AbilityRecord> ability = nullptr;
    sptr<IRemoteObject> callerToken = new OHOS::AAFwk::Token(ability);
    uint64_t key = 1;
    auto ret = abilityMs->ExecuteIntent(key, callerToken, param);
    EXPECT_EQ(ret, INNER_ERR);
}

/**
 * @tc.name: ExecuteIntentForDistributed_0100
 * @tc.desc: Test ExecuteIntentForDistributed with invalid want
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, ExecuteIntentForDistributed_0100, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);

    Want want;
    uint64_t requestCode = 1;
    uint64_t specifiedFullTokenId = 0;
    auto ret = abilityMs->ExecuteIntentForDistributed(want, "deviceA", requestCode, specifiedFullTokenId);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: ExecuteIntentForDistributed_0200
 * @tc.desc: Test ExecuteIntentForDistributed enters common path and returns invalid for incomplete element
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, ExecuteIntentForDistributed_0200, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);

    Want want;
    want.SetElementName("", "com.example.bundle", "MainAbility");
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME, std::string("intent.test"));
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_ID, std::string("1"));

    uint64_t requestCode = 1;
    uint64_t specifiedFullTokenId = 0;
    auto ret = abilityMs->ExecuteIntentForDistributed(want, "deviceA", requestCode, specifiedFullTokenId);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: ExecuteIntentCommon_0100
 * @tc.desc: Test ExecuteIntentCommon with nullptr param
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, ExecuteIntentCommon_0100, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);

    AbilityRuntime::ExtractInsightIntentGenericInfo infos;
    AbilityRuntime::ExecuteIntentCommonOptions options(true, infos, 1);
    auto ret = abilityMs->ExecuteIntentCommon(nullptr, nullptr, "", options);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: ExecuteIntentCommon_0200
 * @tc.desc: Test ExecuteIntentCommon with invalid required fields
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, ExecuteIntentCommon_0200, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);

    auto param = std::make_shared<AppExecFwk::InsightIntentExecuteParam>();
    param->isServiceMatch_ = true;
    param->bundleName_ = "";
    param->moduleName_ = "test.entry";
    param->abilityName_ = "MainAbility";
    param->insightIntentName_ = "intent.test";

    AbilityRuntime::ExtractInsightIntentGenericInfo infos;
    AbilityRuntime::ExecuteIntentCommonOptions options(false, infos, 1);
    auto ret = abilityMs->ExecuteIntentCommon(nullptr, param, "", options);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: ExecuteIntentCommon_0300
 * @tc.desc: Test ExecuteIntentCommon switch UI_ABILITY_FOREGROUND branch
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, ExecuteIntentCommon_0300, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);

    auto param = std::make_shared<AppExecFwk::InsightIntentExecuteParam>();
    param->isServiceMatch_ = true;
    param->bundleName_ = "test.bundleName";
    param->moduleName_ = "test.entry";
    param->abilityName_ = "ability1";
    param->insightIntentName_ = "test1";
    param->insightIntentParam_ = std::make_shared<WantParams>();
    param->executeMode_ = AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND;

    AbilityRuntime::ExtractInsightIntentGenericInfo infos;
    AbilityRuntime::ExecuteIntentCommonOptions options(false, infos, 1);
    auto ret = abilityMs->ExecuteIntentCommon(nullptr, param, "", options);
    EXPECT_NE(ret, ERR_OK);

    param->executeMode_ = AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND;
    ret = abilityMs->ExecuteIntentCommon(nullptr, param, "", options);
    EXPECT_NE(ret, ERR_OK);

    param->executeMode_ = AppExecFwk::ExecuteMode::UI_EXTENSION_ABILITY;
    ret = abilityMs->ExecuteIntentCommon(nullptr, param, "", options);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);

    param->executeMode_ = AppExecFwk::ExecuteMode::SERVICE_EXTENSION_ABILITY;
    ret = abilityMs->ExecuteIntentCommon(nullptr, param, "", options);
    EXPECT_NE(ret, ERR_OK);

    param->executeMode_ = 999;
    ret = abilityMs->ExecuteIntentCommon(nullptr, param, "", options);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: PreloadUIExtensionAbilityInner_0100
 * @tc.desc: Test  GetConnectManagerByUserId error
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, PreloadUIExtensionAbilityInner_0100, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    std::string bundleName = "testBundleName";
    std::string abilityName_ = "testAbility";
    Want want;
    want.SetElementName(bundleName, abilityName_);
    StartAbilityUtils::startAbilityInfo = std::make_shared<StartAbilityInfo>();
    AppExecFwk::ApplicationInfo applicationInfo;
    applicationInfo.bundleName = bundleName;
    applicationInfo.name = "test";
    StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo = applicationInfo;
    StartAbilityUtils::startAbilityInfo->status = ERR_OK;
    StartAbilityUtils::startAbilityInfo->abilityInfo.visible = true;
    abilityMs->subManagersHelper_ = nullptr;
    int32_t preloadId = 1;
    int32_t userId = 1;
    auto ret = abilityMs->PreloadUIExtensionAbilityInner(want, bundleName, preloadId, userId);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: PreloadUIExtensionAbilityInner_0200
 * @tc.desc: Test PreloadUIExtensionAbilityInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, PreloadUIExtensionAbilityInner_0200, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    std::string bundleName = "com.ohos.sceneboard";
    std::string abilityName_ = "MainAbility";
    Want want;
    want.SetElementName(bundleName, abilityName_);
    StartAbilityUtils::startAbilityInfo = std::make_shared<StartAbilityInfo>();
    AppExecFwk::ApplicationInfo applicationInfo;
    applicationInfo.bundleName = bundleName;
    applicationInfo.name = "test";
    StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo = applicationInfo;
    StartAbilityUtils::startAbilityInfo->status = ERR_OK;
    StartAbilityUtils::startAbilityInfo->abilityInfo.visible = true;
    abilityMs->subManagersHelper_ = nullptr;
    int32_t preloadId = 1;
    int32_t userId = 1;
    auto ret = abilityMs->PreloadUIExtensionAbilityInner(want, bundleName, preloadId, userId);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: StartUIExtensionAbility_0100
 * @tc.desc: Test  getabilityInfos error
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, StartUIExtensionAbility_0100, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    Want want;
    want.SetParam(AAFwk::SCREEN_MODE_KEY, AAFwk::EMBEDDED_FULL_SCREEN_MODE);
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->want = want;
    auto ret = abilityMs->StartUIExtensionAbility(sessionInfo, 1);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: StartUIExtensionAbility_0200
 * @tc.desc: Test  CheckAndUpdateWant error
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, StartUIExtensionAbility_0200, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    Want want;
    want.SetParam(INSIGHT_INTENT_EXECUTE_PARAM_NAME, true);
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->want = want;
    auto ret = abilityMs->StartUIExtensionAbility(sessionInfo, 1);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: StartUIExtensionAbility_0300
 * @tc.desc: Test getabilityInfos error
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, StartUIExtensionAbility_0300, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    Want want;
    want.SetParam(AAFwk::SCREEN_MODE_KEY, AAFwk::EMBEDDED_HALF_SCREEN_MODE);
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->want = want;
    auto ret = abilityMs->StartUIExtensionAbility(sessionInfo, 1);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: QueryEntityInfo_0100
 * @tc.desc: Test QueryEntityInfo with executeManager nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, QueryEntityInfo_0100, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);

    uint64_t key = 123;
    sptr<IRemoteObject> callerToken = nullptr;
    AppExecFwk::InsightIntentQueryParam param;
    param.bundleName_ = "test.bundle";
    param.moduleName_ = "test.module";
    param.intentName_ = "test.intent";
    param.className_ = "test.class";
    param.userId_ = 100;

    auto ret = abilityMs->QueryEntityInfo(key, callerToken, param);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: QueryEntityInfo_0200
 * @tc.desc: Test QueryEntityInfo with empty bundleName
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, QueryEntityInfo_0200, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);

    uint64_t key = 123;
    sptr<IRemoteObject> callerToken = nullptr;
    AppExecFwk::InsightIntentQueryParam param;
    param.bundleName_ = "";
    param.moduleName_ = "test.module";
    param.intentName_ = "test.intent";
    param.className_ = "test.class";
    param.userId_ = 100;

    auto ret = abilityMs->QueryEntityInfo(key, callerToken, param);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: QueryEntityInfo_0300
 * @tc.desc: Test QueryEntityInfo with empty moduleName
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, QueryEntityInfo_0300, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);

    uint64_t key = 123;
    sptr<IRemoteObject> callerToken = nullptr;
    AppExecFwk::InsightIntentQueryParam param;
    param.bundleName_ = "test.bundle";
    param.moduleName_ = "";
    param.intentName_ = "test.intent";
    param.className_ = "test.class";
    param.userId_ = 100;

    auto ret = abilityMs->QueryEntityInfo(key, callerToken, param);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: QueryEntityInfo_0400
 * @tc.desc: Test QueryEntityInfo with empty intentName
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, QueryEntityInfo_0400, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);

    uint64_t key = 123;
    sptr<IRemoteObject> callerToken = nullptr;
    AppExecFwk::InsightIntentQueryParam param;
    param.bundleName_ = "test.bundle";
    param.moduleName_ = "test.module";
    param.intentName_ = "";
    param.className_ = "test.class";
    param.userId_ = 100;

    auto ret = abilityMs->QueryEntityInfo(key, callerToken, param);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: QueryEntityInfo_0500
 * @tc.desc: Test QueryEntityInfo with empty className
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, QueryEntityInfo_0500, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);

    uint64_t key = 123;
    sptr<IRemoteObject> callerToken = nullptr;
    AppExecFwk::InsightIntentQueryParam param;
    param.bundleName_ = "test.bundle";
    param.moduleName_ = "test.module";
    param.intentName_ = "test.intent";
    param.className_ = "";
    param.userId_ = 100;

    auto ret = abilityMs->QueryEntityInfo(key, callerToken, param);
    EXPECT_NE(ret, ERR_OK);
}
}
}
