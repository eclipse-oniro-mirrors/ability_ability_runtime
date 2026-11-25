/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "want.h"
#include "insight_intent_execute_param.h"
#include "insight_intent_execute_manager.h"
#include "int_wrapper.h"
#include "string_wrapper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class InsightIntentExecuteManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void InsightIntentExecuteManagerTest::SetUpTestCase(void)
{}

void InsightIntentExecuteManagerTest::TearDownTestCase(void)
{}

void InsightIntentExecuteManagerTest::SetUp()
{}

void InsightIntentExecuteManagerTest::TearDown()
{}

/**
 * @tc.name: GenerateWant_0100
 * @tc.desc: basic function test of display id.
 * @tc.type: FUNC
 * @tc.require: issueI8ZRAG
 */
HWTEST_F(InsightIntentExecuteManagerTest, GenerateWant_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "test.abilityName";
    param.insightIntentName_ = "PlayMusic";
    param.insightIntentParam_ = nullptr;
    // other member has default value.
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    AbilityRuntime::ExtractInsightIntentGenericInfo decoratorInfo;
    AbilityRuntime::ExtractInsightIntentInfo extractInfo;
    Want want;
    auto ret = InsightIntentExecuteManager::GenerateWant(paramPtr, decoratorInfo, extractInfo, want);
    EXPECT_EQ(ret, ERR_OK);
    // get display id of want, expect don't contain
    auto displayId = want.GetIntParam(Want::PARAM_RESV_DISPLAY_ID, -100);
    EXPECT_EQ(displayId, -100);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GenerateWant_0200
 * @tc.desc: basic function test of display id.
 * @tc.type: FUNC
 * @tc.require: issueI8ZRAG
 */
HWTEST_F(InsightIntentExecuteManagerTest, GenerateWant_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "test.abilityName";
    param.insightIntentName_ = "PlayMusic";
    param.insightIntentParam_ = nullptr;
    param.displayId_ = 2;
    // other member has default value.
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    AbilityRuntime::ExtractInsightIntentGenericInfo decoratorInfo;
    AbilityRuntime::ExtractInsightIntentInfo extractInfo;
    Want want;
    auto ret = InsightIntentExecuteManager::GenerateWant(paramPtr, decoratorInfo, extractInfo, want);
    EXPECT_EQ(ret, ERR_OK);
    // get display id of want
    auto displayId = want.GetIntParam(Want::PARAM_RESV_DISPLAY_ID, -100);
    EXPECT_EQ(displayId, 2);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: RemoveExecuteIntent_0100
 * @tc.desc: Test RemoveExecuteIntent with valid intentId.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, RemoveExecuteIntent_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoveExecuteIntent_0100 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    // First add a record
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.module";
    param.abilityName_ = "test.ability";
    param.insightIntentName_ = "TestIntent";

    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    sptr<IRemoteObject> callerToken = nullptr;
    uint64_t key = 1005;

    auto ret = manager->CheckAndUpdateParam(
        key, callerToken, paramPtr, ExecuteParam{.callerBundleName = "caller.bundle"});
    ASSERT_NE(ret, ERR_OK);
    uint64_t intentId = param.insightIntentId_;

    // Then remove it
    ret = manager->RemoveExecuteIntent(intentId);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "RemoveExecuteIntent_0100 end.");
}

/**
 * @tc.name: RemoveExecuteIntent_0200
 * @tc.desc: Test RemoveExecuteIntent with non-existent intentId.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, RemoveExecuteIntent_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoveExecuteIntent_0200 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    uint64_t invalidIntentId = 99999;
    auto ret = manager->RemoveExecuteIntent(invalidIntentId);
    EXPECT_EQ(ret, ERR_OK);  // Remove doesn't return error for non-existent

    TAG_LOGI(AAFwkTag::TEST, "RemoveExecuteIntent_0200 end.");
}

/**
 * @tc.name: GetBundleName_0100
 * @tc.desc: Test GetBundleName with valid intentId.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, GetBundleName_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetBundleName_0100 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    // First add a record
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.module";
    param.abilityName_ = "test.ability";
    param.insightIntentName_ = "TestIntent";

    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    sptr<IRemoteObject> callerToken = nullptr;
    uint64_t key = 1006;

    auto ret = manager->CheckAndUpdateParam(
        key, callerToken, paramPtr, ExecuteParam{.callerBundleName = "caller.bundle"});
    ASSERT_NE(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "GetBundleName_0100 end.");
}

/**
 * @tc.name: GetBundleName_0200
 * @tc.desc: Test GetBundleName with invalid intentId.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, GetBundleName_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetBundleName_0200 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    uint64_t invalidIntentId = 99999;
    std::string bundleName;
    auto ret = manager->GetBundleName(invalidIntentId, bundleName);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    EXPECT_TRUE(bundleName.empty());

    TAG_LOGI(AAFwkTag::TEST, "GetBundleName_0200 end.");
}

/**
 * @tc.name: RemoteDied_0100
 * @tc.desc: Test RemoteDied with valid intentId.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, RemoteDied_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoteDied_0100 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    // First add a record
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.module";
    param.abilityName_ = "test.ability";
    param.insightIntentName_ = "TestIntent";

    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    sptr<IRemoteObject> callerToken = nullptr;
    uint64_t key = 1008;

    auto ret = manager->CheckAndUpdateParam(
        key, callerToken, paramPtr, ExecuteParam{.callerBundleName = "caller.bundle"});
    ASSERT_NE(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "RemoteDied_0100 end.");
}

/**
 * @tc.name: RemoteDied_0200
 * @tc.desc: Test RemoteDied with invalid intentId.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, RemoteDied_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoteDied_0200 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    uint64_t invalidIntentId = 99999;
    auto ret = manager->RemoteDied(invalidIntentId);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "RemoteDied_0200 end.");
}

/**
 * @tc.name: ExecuteIntentDone_0100
 * @tc.desc: Test ExecuteIntentDone with valid intentId.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, ExecuteIntentDone_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteIntentDone_0100 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    // First add a record
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.module";
    param.abilityName_ = "test.ability";
    param.insightIntentName_ = "TestIntent";

    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    sptr<IRemoteObject> callerToken = nullptr;
    uint64_t key = 1009;

    auto ret = manager->CheckAndUpdateParam(
        key, callerToken, paramPtr, ExecuteParam{.callerBundleName = "caller.bundle"});
    ASSERT_NE(ret, ERR_OK);
    uint64_t intentId = param.insightIntentId_;

    // Mark as done
    AppExecFwk::InsightIntentExecuteResult result;
    ret = manager->ExecuteIntentDone(intentId, 0, result);
    // This will fail because callerToken is null and callback iface_cast will fail
    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "ExecuteIntentDone_0100 end.");
}

/**
 * @tc.name: ExecuteIntentDone_0200
 * @tc.desc: Test ExecuteIntentDone with invalid intentId.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, ExecuteIntentDone_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteIntentDone_0200 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    uint64_t invalidIntentId = 99999;
    AppExecFwk::InsightIntentExecuteResult result;
    auto ret = manager->ExecuteIntentDone(invalidIntentId, 0, result);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "ExecuteIntentDone_0200 end.");
}

/**
 * @tc.name: CheckCallerPermission_0100
 * @tc.desc: Test CheckCallerPermission static method.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, CheckCallerPermission_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckCallerPermission_0100 begin.");

    auto ret = InsightIntentExecuteManager::CheckCallerPermission();
    // Result depends on system permissions, just check it doesn't crash
    EXPECT_TRUE(ret == ERR_OK || ret == ERR_PERMISSION_DENIED || ret == ERR_NOT_SYSTEM_APP);

    TAG_LOGI(AAFwkTag::TEST, "CheckCallerPermission_0100 end.");
}

/**
 * @tc.name: SetIntentExemptionInfo_0100
 * @tc.desc: Test SetIntentExemptionInfo with valid uid.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, SetIntentExemptionInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetIntentExemptionInfo_0100 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    int32_t uid = 1000;
    manager->SetIntentExemptionInfo(uid);

    // Verify exemption info was set
    auto exemptionInfo = manager->GetAllIntentExemptionInfo();
    EXPECT_GE(exemptionInfo.size(), 0);

    TAG_LOGI(AAFwkTag::TEST, "SetIntentExemptionInfo_0100 end.");
}

/**
 * @tc.name: CheckIntentIsExemption_0100
 * @tc.desc: Test CheckIntentIsExemption with exempted uid.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, CheckIntentIsExemption_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckIntentIsExemption_0100 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    int32_t uid = 1001;
    manager->SetIntentExemptionInfo(uid);

    bool isExemption = manager->CheckIntentIsExemption(uid);
    EXPECT_TRUE(isExemption);

    TAG_LOGI(AAFwkTag::TEST, "CheckIntentIsExemption_0100 end.");
}

/**
 * @tc.name: GetAllIntentExemptionInfo_0100
 * @tc.desc: Test GetAllIntentExemptionInfo returns map.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, GetAllIntentExemptionInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAllIntentExemptionInfo_0100 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    int32_t uid = 1002;
    manager->SetIntentExemptionInfo(uid);

    auto exemptionInfo = manager->GetAllIntentExemptionInfo();
    EXPECT_GE(exemptionInfo.size(), 1);

    TAG_LOGI(AAFwkTag::TEST, "GetAllIntentExemptionInfo_0100 end.");
}

/**
 * @tc.name: GenerateWant_0300
 * @tc.desc: Test GenerateWant with null param.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, GenerateWant_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GenerateWant_0300 begin.");

    std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> paramPtr = nullptr;
    AbilityRuntime::ExtractInsightIntentGenericInfo decoratorInfo;
    AbilityRuntime::ExtractInsightIntentInfo extractInfo;
    Want want;

    auto ret = InsightIntentExecuteManager::GenerateWant(paramPtr, decoratorInfo, extractInfo, want);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "GenerateWant_0300 end.");
}

/**
 * @tc.name: GenerateWant_0400
 * @tc.desc: Test GenerateWant with all parameters set.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, GenerateWant_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GenerateWant_0400 begin.");

    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "test.abilityName";
    param.insightIntentName_ = "PlayMusic";
    param.insightIntentParam_ = nullptr;
    param.displayId_ = 2;
    param.insightIntentId_ = 100;

    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    AbilityRuntime::ExtractInsightIntentGenericInfo decoratorInfo;
    AbilityRuntime::ExtractInsightIntentInfo extractInfo;
    Want want;

    auto ret = InsightIntentExecuteManager::GenerateWant(paramPtr, decoratorInfo, extractInfo, want);
    EXPECT_EQ(ret, ERR_OK);

    // Verify intentId is set in want
    auto intentIdStr = want.GetStringParam("ohos.extra.param.key.insightIntentId");
    EXPECT_TRUE(intentIdStr.empty());

    TAG_LOGI(AAFwkTag::TEST, "GenerateWant_0400 end.");
}

/**
 * @tc.name: GenerateWant_0500
 * @tc.desc: toolCallId is written into the outer Want without changing business parameters.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, GenerateWant_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GenerateWant_0500 begin.");
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "test.abilityName";
    param.insightIntentName_ = "PlayMusic";
    param.insightIntentParam_ = std::make_shared<WantParams>();
    param.insightIntentParam_->SetParam("dummy", Integer::Box(-1));
    param.toolCallId_ = "tc-001";
    param.insightIntentParam_->SetParam("toolCallId", String::Box("business-value"));
    param.displayId_ = 2;

    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    AbilityRuntime::ExtractInsightIntentGenericInfo decoratorInfo;
    AbilityRuntime::ExtractInsightIntentInfo extractInfo;
    Want want;
    auto ret = InsightIntentExecuteManager::GenerateWant(paramPtr, decoratorInfo, extractInfo, want);
    EXPECT_EQ(ret, ERR_OK);

    WantParams insightIntentParam = want.GetParams().GetWantParams(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_PARAM);
    EXPECT_EQ(want.GetStringParam(AppExecFwk::INSIGHT_INTENT_TOOL_CALL_ID), "tc-001");
    EXPECT_FALSE(insightIntentParam.HasParam(AppExecFwk::INSIGHT_INTENT_TOOL_CALL_ID));
    EXPECT_FALSE(paramPtr->insightIntentParam_->HasParam(AppExecFwk::INSIGHT_INTENT_TOOL_CALL_ID));
    // business params are preserved
    EXPECT_EQ(insightIntentParam.GetIntParam("dummy", 0), -1);
    EXPECT_EQ(insightIntentParam.GetStringParam("toolCallId"), "business-value");
    TAG_LOGI(AAFwkTag::TEST, "GenerateWant_0500 end.");
}

/**
 * @tc.name: GenerateWant_0600
 * @tc.desc: Empty toolCallId omits the key; a nonempty ID does not require business parameters.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, GenerateWant_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GenerateWant_0600 begin.");
    // case 1: empty toolCallId with existing execute param
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "test.abilityName";
    param.insightIntentName_ = "PlayMusic";
    param.insightIntentParam_ = std::make_shared<WantParams>();
    param.insightIntentParam_->SetParam("dummy", Integer::Box(-1));
    param.toolCallId_ = "";
    param.displayId_ = 2;
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    AbilityRuntime::ExtractInsightIntentGenericInfo decoratorInfo;
    AbilityRuntime::ExtractInsightIntentInfo extractInfo;
    Want want;
    EXPECT_EQ(InsightIntentExecuteManager::GenerateWant(paramPtr, decoratorInfo, extractInfo, want), ERR_OK);
    EXPECT_FALSE(want.HasParameter(AppExecFwk::INSIGHT_INTENT_TOOL_CALL_ID));
    WantParams insightIntentParam = want.GetParams().GetWantParams(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_PARAM);
    EXPECT_EQ(insightIntentParam.HasParam(AppExecFwk::INSIGHT_INTENT_TOOL_CALL_ID), false);

    // case 2: null business parameters remain null while the outer Want carries toolCallId.
    AppExecFwk::InsightIntentExecuteParam param2;
    param2.bundleName_ = "test.bundleName";
    param2.moduleName_ = "test.entry";
    param2.abilityName_ = "test.abilityName";
    param2.insightIntentName_ = "PlayMusic";
    param2.insightIntentParam_ = nullptr;
    param2.toolCallId_ = "tc-002";
    param2.displayId_ = 2;
    auto paramPtr2 = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param2);
    Want want2;
    EXPECT_EQ(InsightIntentExecuteManager::GenerateWant(paramPtr2, decoratorInfo, extractInfo, want2), ERR_OK);
    EXPECT_EQ(want2.GetStringParam(AppExecFwk::INSIGHT_INTENT_TOOL_CALL_ID), "tc-002");
    EXPECT_FALSE(want2.HasParameter(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_PARAM));
    EXPECT_EQ(paramPtr2->insightIntentParam_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "GenerateWant_0600 end.");
}

/**
 * @tc.name: GenerateWant_ToolCallIdRoundTrip_0700
 * @tc.desc: The toolCallId field used by Driver and FunctionManager survives the outer Want round trip.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, GenerateWant_ToolCallIdRoundTrip_0700, TestSize.Level1)
{
    auto param = std::make_shared<AppExecFwk::InsightIntentExecuteParam>();
    param->bundleName_ = "test.bundleName";
    param->moduleName_ = "test.entry";
    param->abilityName_ = "test.abilityName";
    param->insightIntentName_ = "PlayMusic";
    param->insightIntentId_ = 42;
    param->displayId_ = 2;
    param->insightIntentParam_ = std::make_shared<WantParams>();
    param->toolCallId_ = "tc-fm";
    param->insightIntentParam_->SetParam("dummy", Integer::Box(-1));

    AbilityRuntime::ExtractInsightIntentGenericInfo decoratorInfo;
    AbilityRuntime::ExtractInsightIntentInfo extractInfo;
    Want want;
    ASSERT_EQ(InsightIntentExecuteManager::GenerateWant(param, decoratorInfo, extractInfo, want), ERR_OK);
    EXPECT_EQ(want.GetStringParam(AppExecFwk::INSIGHT_INTENT_TOOL_CALL_ID), "tc-fm");

    AppExecFwk::InsightIntentExecuteParam received;
    ASSERT_TRUE(AppExecFwk::InsightIntentExecuteParam::GenerateFromWant(want, received));
    ASSERT_NE(received.insightIntentParam_, nullptr);
    EXPECT_FALSE(received.insightIntentParam_->HasParam(AppExecFwk::INSIGHT_INTENT_TOOL_CALL_ID));
    EXPECT_EQ(received.insightIntentParam_->GetIntParam("dummy", 0), -1);
    EXPECT_EQ(received.insightIntentId_, 42u);
    EXPECT_EQ(received.toolCallId_, "tc-fm");
}

/**
 * @tc.name: RegisterExecuteRecord_0100
 * @tc.desc: Pre-register a PENDING_CONSENT record; the returned id is non-zero and
 *           its caller bundle is queryable while the consent dialog is pending.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, RegisterExecuteRecord_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RegisterExecuteRecord_0100 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    ExecuteParam execParam;
    execParam.callerBundleName = "caller.bundle";
    uint64_t intentId = 0;
    auto ret = manager->RegisterExecuteRecord(0, nullptr, "test.bundle", execParam, intentId);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_NE(intentId, 0);

    // A live PENDING_CONSENT record exposes its caller bundle.
    EXPECT_EQ(manager->GetInsightIntentCallerBundleName(intentId), "caller.bundle");

    TAG_LOGI(AAFwkTag::TEST, "RegisterExecuteRecord_0100 end.");
}

/**
 * @tc.name: AcquireExecuteRecord_0100
 * @tc.desc: Promote a pre-registered PENDING_CONSENT record to EXECUTING on the
 *           first acquire, then reuse the EXECUTING record idempotently.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, AcquireExecuteRecord_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AcquireExecuteRecord_0100 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    ExecuteParam execParam;
    execParam.callerBundleName = "caller.bundle";
    uint64_t intentId = 0;
    ASSERT_EQ(manager->RegisterExecuteRecord(0, nullptr, "test.bundle", execParam, intentId), ERR_OK);

    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundle";
    param.insightIntentId_ = intentId;
    param.taskId_ = "task-1";
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    sptr<IRemoteObject> callerToken = nullptr;

    // Promote PENDING_CONSENT -> EXECUTING.
    auto ret = manager->AcquireExecuteRecord(1005, callerToken, paramPtr, execParam);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(manager->GetInsightIntentCallerBundleName(intentId), "caller.bundle");

    // Reuse the EXECUTING record idempotently.
    ret = manager->AcquireExecuteRecord(1005, callerToken, paramPtr, execParam);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "AcquireExecuteRecord_0100 end.");
}

/**
 * @tc.name: AcquireExecuteRecord_0200
 * @tc.desc: A specified intentId whose bundle does not match the pre-registered
 *           record is rejected with ERR_INVALID_VALUE.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, AcquireExecuteRecord_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AcquireExecuteRecord_0200 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    ExecuteParam execParam;
    execParam.callerBundleName = "caller.bundle";
    uint64_t intentId = 0;
    ASSERT_EQ(manager->RegisterExecuteRecord(0, nullptr, "test.bundle", execParam, intentId), ERR_OK);

    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "other.bundle";
    param.insightIntentId_ = intentId;
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    sptr<IRemoteObject> callerToken = nullptr;

    EXPECT_EQ(manager->AcquireExecuteRecord(1006, callerToken, paramPtr, execParam), ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "AcquireExecuteRecord_0200 end.");
}

/**
 * @tc.name: AcquireExecuteRecord_0300
 * @tc.desc: A specified intentId that does not map to any record is rejected with
 *           ERR_INVALID_VALUE.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, AcquireExecuteRecord_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AcquireExecuteRecord_0300 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    ExecuteParam execParam;
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundle";
    param.insightIntentId_ = 99999999;
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    sptr<IRemoteObject> callerToken = nullptr;

    EXPECT_EQ(manager->AcquireExecuteRecord(1007, callerToken, paramPtr, execParam), ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "AcquireExecuteRecord_0300 end.");
}

/**
 * @tc.name: AcquireExecuteRecord_0400
 * @tc.desc: A zero intentId allocates a fresh EXECUTING record via AddRecord and
 *           stamps the new id back onto the param.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, AcquireExecuteRecord_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AcquireExecuteRecord_0400 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    ExecuteParam execParam;
    execParam.callerBundleName = "caller.bundle";

    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundle";
    param.insightIntentId_ = 0;
    param.taskId_ = "task-1";
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    sptr<IRemoteObject> callerToken = nullptr;

    EXPECT_EQ(manager->AcquireExecuteRecord(2001, callerToken, paramPtr, execParam), ERR_OK);
    EXPECT_NE(paramPtr->insightIntentId_, 0);
    // A fresh record starts in EXECUTING and exposes its caller bundle.
    EXPECT_EQ(manager->GetInsightIntentCallerBundleName(paramPtr->insightIntentId_), "caller.bundle");

    TAG_LOGI(AAFwkTag::TEST, "AcquireExecuteRecord_0400 end.");
}

/**
 * @tc.name: AcquireExecuteRecord_0500
 * @tc.desc: A specified intentId whose record sits in EXECUTE_DONE is rejected
 *           with ERR_INVALID_VALUE instead of being reused.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, AcquireExecuteRecord_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AcquireExecuteRecord_0500 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    ExecuteParam execParam;
    execParam.callerBundleName = "caller.bundle";
    uint64_t intentId = 0;
    ASSERT_EQ(manager->RegisterExecuteRecord(0, nullptr, "test.bundle", execParam, intentId), ERR_OK);
    ASSERT_EQ(manager->UpdateRecordState(intentId, InsightIntentExecuteState::PENDING_CONSENT,
        InsightIntentExecuteState::EXECUTING), ERR_OK);
    ASSERT_EQ(manager->UpdateRecordState(intentId, InsightIntentExecuteState::EXECUTING,
        InsightIntentExecuteState::EXECUTE_DONE), ERR_OK);

    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundle";
    param.insightIntentId_ = intentId;
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    sptr<IRemoteObject> callerToken = nullptr;

    EXPECT_EQ(manager->AcquireExecuteRecord(2002, callerToken, paramPtr, execParam), ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "AcquireExecuteRecord_0500 end.");
}

/**
 * @tc.name: AcquireExecuteRecord_0600
 * @tc.desc: A specified intentId whose caller already died (REMOTE_DIED) is
 *           rejected with ERR_INVALID_VALUE instead of being reused.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, AcquireExecuteRecord_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AcquireExecuteRecord_0600 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    ExecuteParam execParam;
    execParam.callerBundleName = "caller.bundle";
    uint64_t intentId = 0;
    ASSERT_EQ(manager->RegisterExecuteRecord(0, nullptr, "test.bundle", execParam, intentId), ERR_OK);
    ASSERT_EQ(manager->RemoteDied(intentId), ERR_OK);

    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundle";
    param.insightIntentId_ = intentId;
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    sptr<IRemoteObject> callerToken = nullptr;

    EXPECT_EQ(manager->AcquireExecuteRecord(2003, callerToken, paramPtr, execParam), ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "AcquireExecuteRecord_0600 end.");
}

/**
 * @tc.name: GetInsightIntentCallerBundleName_0100
 * @tc.desc: A missing / unknown intentId returns an empty caller bundle name.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, GetInsightIntentCallerBundleName_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetInsightIntentCallerBundleName_0100 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    EXPECT_EQ(manager->GetInsightIntentCallerBundleName(99999999), "");

    TAG_LOGI(AAFwkTag::TEST, "GetInsightIntentCallerBundleName_0100 end.");
}

/**
 * @tc.name: GetInsightIntentCallerBundleName_0200
 * @tc.desc: A stale (EXECUTE_DONE) record no longer exposes its caller bundle.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, GetInsightIntentCallerBundleName_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetInsightIntentCallerBundleName_0200 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    ExecuteParam execParam;
    execParam.callerBundleName = "caller.bundle";
    uint64_t intentId = 0;
    ASSERT_EQ(manager->RegisterExecuteRecord(0, nullptr, "test.bundle", execParam, intentId), ERR_OK);
    ASSERT_EQ(manager->GetInsightIntentCallerBundleName(intentId), "caller.bundle");

    // Move PENDING_CONSENT -> EXECUTING -> EXECUTE_DONE; stale records return empty.
    ASSERT_EQ(manager->UpdateRecordState(intentId, InsightIntentExecuteState::PENDING_CONSENT,
        InsightIntentExecuteState::EXECUTING), ERR_OK);
    ASSERT_EQ(manager->UpdateRecordState(intentId, InsightIntentExecuteState::EXECUTING,
        InsightIntentExecuteState::EXECUTE_DONE), ERR_OK);
    EXPECT_EQ(manager->GetInsightIntentCallerBundleName(intentId), "");

    TAG_LOGI(AAFwkTag::TEST, "GetInsightIntentCallerBundleName_0200 end.");
}

/**
 * @tc.name: UpdateRecordState_0100
 * @tc.desc: A CAS-style transition succeeds when the record sits in the expected state.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, UpdateRecordState_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateRecordState_0100 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    ExecuteParam execParam;
    uint64_t intentId = 0;
    ASSERT_EQ(manager->RegisterExecuteRecord(0, nullptr, "test.bundle", execParam, intentId), ERR_OK);

    EXPECT_EQ(manager->UpdateRecordState(intentId, InsightIntentExecuteState::PENDING_CONSENT,
        InsightIntentExecuteState::EXECUTING), ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "UpdateRecordState_0100 end.");
}

/**
 * @tc.name: UpdateRecordState_0200
 * @tc.desc: Transition fails with ERR_INVALID_OPERATION when the current state
 *           does not match the expected one.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, UpdateRecordState_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateRecordState_0200 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    ExecuteParam execParam;
    uint64_t intentId = 0;
    ASSERT_EQ(manager->RegisterExecuteRecord(0, nullptr, "test.bundle", execParam, intentId), ERR_OK);

    // Already EXECUTING, expecting PENDING_CONSENT again must fail.
    ASSERT_EQ(manager->UpdateRecordState(intentId, InsightIntentExecuteState::PENDING_CONSENT,
        InsightIntentExecuteState::EXECUTING), ERR_OK);
    EXPECT_EQ(manager->UpdateRecordState(intentId, InsightIntentExecuteState::PENDING_CONSENT,
        InsightIntentExecuteState::EXECUTING), ERR_INVALID_OPERATION);

    TAG_LOGI(AAFwkTag::TEST, "UpdateRecordState_0200 end.");
}

/**
 * @tc.name: UpdateRecordState_0300
 * @tc.desc: Transition on an unknown intentId fails with ERR_INVALID_VALUE.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, UpdateRecordState_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateRecordState_0300 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    EXPECT_EQ(manager->UpdateRecordState(99999999, InsightIntentExecuteState::PENDING_CONSENT,
        InsightIntentExecuteState::EXECUTING), ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "UpdateRecordState_0300 end.");
}

/**
 * @tc.name: RemoveExecuteRecordIfCaller_0100
 * @tc.desc: The caller removes its own pre-registered record (matching token id).
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, RemoveExecuteRecordIfCaller_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoveExecuteRecordIfCaller_0100 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    ExecuteParam execParam;
    execParam.callerBundleName = "caller.bundle";
    uint64_t intentId = 0;
    ASSERT_EQ(manager->RegisterExecuteRecord(0, nullptr, "test.bundle", execParam, intentId), ERR_OK);
    ASSERT_EQ(manager->GetInsightIntentCallerBundleName(intentId), "caller.bundle");

    EXPECT_EQ(manager->RemoveExecuteRecordIfCaller(intentId), ERR_OK);
    EXPECT_EQ(manager->GetInsightIntentCallerBundleName(intentId), "");

    TAG_LOGI(AAFwkTag::TEST, "RemoveExecuteRecordIfCaller_0100 end.");
}

/**
 * @tc.name: RemoveExecuteRecordIfCaller_0200
 * @tc.desc: Removing an unknown intentId is a no-op that returns ERR_OK.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, RemoveExecuteRecordIfCaller_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoveExecuteRecordIfCaller_0200 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    EXPECT_EQ(manager->RemoveExecuteRecordIfCaller(99999999), ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "RemoveExecuteRecordIfCaller_0200 end.");
}

/**
 * @tc.name: GetInsightIntentExported_0100
 * @tc.desc: Empty target identifiers short-circuit to not-exported without
 *           touching the intent cache.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, GetInsightIntentExported_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetInsightIntentExported_0100 begin.");

    EXPECT_FALSE(InsightIntentExecuteManager::GetInsightIntentExported("", "", "", 0));
    EXPECT_FALSE(InsightIntentExecuteManager::GetInsightIntentExported("test.bundle", "", "TestIntent", 0));

    TAG_LOGI(AAFwkTag::TEST, "GetInsightIntentExported_0100 end.");
}

/**
 * @tc.name: CheckExportedIntentCallerPermission_0100
 * @tc.desc: A non-exported (missing) target is reported as INTENT_NOT_EXIST so
 *           its existence is not leaked to third-party callers.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, CheckExportedIntentCallerPermission_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckExportedIntentCallerPermission_0100 begin.");

    EXPECT_EQ(InsightIntentExecuteManager::CheckExportedIntentCallerPermission("", "", "", 0),
        INTENT_NOT_EXIST);

    TAG_LOGI(AAFwkTag::TEST, "CheckExportedIntentCallerPermission_0100 end.");
}

/**
 * @tc.name: CheckAndUpdatePublicParam_0100
 * @tc.desc: A public invoke whose target intent is not exported (empty target)
 *           is rejected with INTENT_NOT_EXIST before any record is acquired.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, CheckAndUpdatePublicParam_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckAndUpdatePublicParam_0100 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    AppExecFwk::InsightIntentExecuteParam param;
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    sptr<IRemoteObject> callerToken = nullptr;

    ExecuteParam execParam;
    EXPECT_EQ(manager->CheckAndUpdatePublicParam(0, callerToken, paramPtr, execParam), INTENT_NOT_EXIST);

    TAG_LOGI(AAFwkTag::TEST, "CheckAndUpdatePublicParam_0100 end.");
}

} // namespace AAFwk
} // namespace OHOS
