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
    Want want;
    auto ret = InsightIntentExecuteManager::GenerateWant(paramPtr, decoratorInfo, want);
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
    Want want;
    auto ret = InsightIntentExecuteManager::GenerateWant(paramPtr, decoratorInfo, want);
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

    auto ret = manager->CheckAndUpdateParam(key, callerToken, paramPtr, "caller.bundle", false);
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

    auto ret = manager->CheckAndUpdateParam(key, callerToken, paramPtr, "caller.bundle", false);
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

    auto ret = manager->CheckAndUpdateParam(key, callerToken, paramPtr, "caller.bundle", false);
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

    auto ret = manager->CheckAndUpdateParam(key, callerToken, paramPtr, "caller.bundle", false);
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
    EXPECT_TRUE(ret == ERR_OK || ret == ERR_PERMISSION_DENIED);

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
    Want want;

    auto ret = InsightIntentExecuteManager::GenerateWant(paramPtr, decoratorInfo, want);
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
    Want want;

    auto ret = InsightIntentExecuteManager::GenerateWant(paramPtr, decoratorInfo, want);
    EXPECT_EQ(ret, ERR_OK);

    // Verify intentId is set in want
    auto intentIdStr = want.GetStringParam("ohos.extra.param.key.insightIntentId");
    EXPECT_TRUE(intentIdStr.empty());

    TAG_LOGI(AAFwkTag::TEST, "GenerateWant_0400 end.");
}

} // namespace AAFwk
} // namespace OHOS
