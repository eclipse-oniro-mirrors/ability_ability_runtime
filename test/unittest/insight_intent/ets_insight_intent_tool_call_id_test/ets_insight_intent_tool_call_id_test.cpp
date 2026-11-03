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
#include <functional>

#include "ability_transaction_callback_info.h"
#include "ani_common_execute_param.h"
#include "ets_insight_intent_context.h"
#include "insight_intent_execute_result.h"
#include "insight_intent_executor.h"
#include "mock_ani_env.h"
#include "string_wrapper.h"

using namespace testing::ext;

namespace OHOS::AbilityRuntime {
namespace {
constexpr ani_int INTENT_ID = 42;
constexpr const char *BUNDLE_NAME = "com.example.intent";
constexpr const char *TOOL_CALL_ID = "tc-ets-001";

class TestInsightIntentExecutor : public InsightIntentExecutor {
public:
    bool Init(const InsightIntentExecutorInfo &info) override
    {
        return InsightIntentExecutor::Init(info);
    }

    bool HandleExecuteIntent(std::shared_ptr<InsightIntentExecuteParam>, const std::shared_ptr<NativeReference> &,
        std::unique_ptr<InsightIntentExecutorAsyncCallback>, bool &) override
    {
        return false;
    }
};
} // namespace

class EtsInsightIntentToolCallIdTest : public testing::Test {
protected:
    void SetUp() override
    {
        param_ = env_.NewObject();
        auto &properties = MockInsightIntentAniEnv::Get(param_).properties;
        properties["bundleName"] = env_.NewString(BUNDLE_NAME);
        properties["moduleName"] = env_.NewString("entry");
        properties["abilityName"] = env_.NewString("IntentAbility");
        properties["insightIntentName"] = env_.NewString("QueryWeather");
        properties["deviceId"] = env_.NewString("device-001");
        auto mode = env_.NewObject();
        MockInsightIntentAniEnv::Get(mode).numbers["value"] = AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND;
        properties["executeMode"] = mode;
        auto record = env_.NewObject();
        MockInsightIntentAniEnv::Get(record).wantParams.SetParam("city", AAFwk::String::Box("Shanghai"));
        MockInsightIntentAniEnv::Get(record).wantParams.SetParam("toolCallId", AAFwk::String::Box("business-value"));
        properties["insightIntentParam"] = record;
    }

    void SetToolCallId(ani_ref value)
    {
        MockInsightIntentAniEnv::Get(param_).properties["toolCallId"] = value;
    }

    void ExpectExistingParams(const AppExecFwk::InsightIntentExecuteParam &param)
    {
        EXPECT_EQ(param.bundleName_, BUNDLE_NAME);
        EXPECT_EQ(param.moduleName_, "entry");
        EXPECT_EQ(param.abilityName_, "IntentAbility");
        EXPECT_EQ(param.insightIntentName_, "QueryWeather");
        EXPECT_EQ(param.deviceId_, "device-001");
        EXPECT_EQ(param.executeMode_, AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND);
        ASSERT_NE(param.insightIntentParam_, nullptr);
        EXPECT_EQ(param.insightIntentParam_->GetStringParam("city"), "Shanghai");
        EXPECT_EQ(param.insightIntentParam_->GetStringParam("toolCallId"), "business-value");
        EXPECT_FALSE(param.insightIntentParam_->HasParam(AppExecFwk::INSIGHT_INTENT_TOOL_CALL_ID));
    }

    std::unique_ptr<AppExecFwk::ETSNativeReference> CreateContext(const std::string &toolCallId)
    {
        nativeContext_ = std::make_shared<InsightIntentContext>(nullptr, BUNDLE_NAME, 0, INTENT_ID, toolCallId);
        etsContext_ = std::make_unique<EtsInsightIntentContext>(nativeContext_);
        return CreateEtsInsightIntentContext(&env_, etsContext_.get());
    }

    MockInsightIntentAniEnv env_;
    ani_object param_ = nullptr;
    std::shared_ptr<InsightIntentContext> nativeContext_;
    std::unique_ptr<EtsInsightIntentContext> etsContext_;
};

/**
 * @tc.name: UnwrapToolCallId_0100
 * @tc.desc: The ETS Driver input reaches the native field without changing existing parameters.
 * @tc.type: FUNC
 */
HWTEST_F(EtsInsightIntentToolCallIdTest, UnwrapToolCallId_0100, TestSize.Level1)
{
    SetToolCallId(env_.NewString(TOOL_CALL_ID));
    AppExecFwk::InsightIntentExecuteParam param;
    ASSERT_TRUE(UnwrapExecuteParam(&env_, param_, param));
    EXPECT_EQ(param.toolCallId_, TOOL_CALL_ID);
    ExpectExistingParams(param);
}

/**
 * @tc.name: UnwrapEmptyToolCallId_0200
 * @tc.desc: An explicit empty string leaves the native identifier empty.
 * @tc.type: FUNC
 */
HWTEST_F(EtsInsightIntentToolCallIdTest, UnwrapEmptyToolCallId_0200, TestSize.Level1)
{
    SetToolCallId(env_.NewString(""));
    AppExecFwk::InsightIntentExecuteParam param;
    ASSERT_TRUE(UnwrapExecuteParam(&env_, param_, param));
    EXPECT_TRUE(param.toolCallId_.empty());
    ExpectExistingParams(param);
}

/**
 * @tc.name: UnwrapMissingToolCallId_0300
 * @tc.desc: Legacy input without the new property still parses successfully.
 * @tc.type: FUNC
 */
HWTEST_F(EtsInsightIntentToolCallIdTest, UnwrapMissingToolCallId_0300, TestSize.Level1)
{
    AppExecFwk::InsightIntentExecuteParam param;
    ASSERT_TRUE(UnwrapExecuteParam(&env_, param_, param));
    EXPECT_TRUE(param.toolCallId_.empty());
    ExpectExistingParams(param);
}

/**
 * @tc.name: UnwrapUndefinedToolCallId_0400
 * @tc.desc: An optional ETS property with value undefined is treated as unspecified.
 * @tc.type: FUNC
 */
HWTEST_F(EtsInsightIntentToolCallIdTest, UnwrapUndefinedToolCallId_0400, TestSize.Level1)
{
    SetToolCallId(env_.Undefined());
    AppExecFwk::InsightIntentExecuteParam param;
    ASSERT_TRUE(UnwrapExecuteParam(&env_, param_, param));
    EXPECT_TRUE(param.toolCallId_.empty());
    ExpectExistingParams(param);
}

/**
 * @tc.name: UnwrapNonStringToolCallId_0500
 * @tc.desc: A non-string identifier is ignored, matching the JS entry's optional-field behavior.
 * @tc.type: FUNC
 */
HWTEST_F(EtsInsightIntentToolCallIdTest, UnwrapNonStringToolCallId_0500, TestSize.Level1)
{
    SetToolCallId(env_.NewObject());
    AppExecFwk::InsightIntentExecuteParam param;
    ASSERT_TRUE(UnwrapExecuteParam(&env_, param_, param));
    EXPECT_TRUE(param.toolCallId_.empty());
    ExpectExistingParams(param);
}

/**
 * @tc.name: UnwrapToolCallIdReadFailure_0600
 * @tc.desc: A failed optional string conversion does not reject otherwise valid input.
 * @tc.type: FUNC
 */
HWTEST_F(EtsInsightIntentToolCallIdTest, UnwrapToolCallIdReadFailure_0600, TestSize.Level1)
{
    env_.unreadableString = env_.NewString(TOOL_CALL_ID);
    SetToolCallId(env_.unreadableString);
    AppExecFwk::InsightIntentExecuteParam param;
    ASSERT_TRUE(UnwrapExecuteParam(&env_, param_, param));
    EXPECT_TRUE(param.toolCallId_.empty());
    ExpectExistingParams(param);
}

/**
 * @tc.name: UnwrapRequiredFieldStillChecked_0700
 * @tc.desc: Adding toolCallId does not relax validation of existing required fields.
 * @tc.type: FUNC
 */
HWTEST_F(EtsInsightIntentToolCallIdTest, UnwrapRequiredFieldStillChecked_0700, TestSize.Level1)
{
    SetToolCallId(env_.NewString(TOOL_CALL_ID));
    MockInsightIntentAniEnv::Get(param_).properties.erase("bundleName");
    AppExecFwk::InsightIntentExecuteParam param;
    EXPECT_FALSE(UnwrapExecuteParam(&env_, param_, param));
}

/**
 * @tc.name: ContextToolCallId_0800
 * @tc.desc: The shared ETS bridge exposes the identifier while retaining instanceId and nativeContext.
 * @tc.type: FUNC
 */
HWTEST_F(EtsInsightIntentToolCallIdTest, ContextToolCallId_0800, TestSize.Level1)
{
    auto context = CreateContext(TOOL_CALL_ID);
    ASSERT_NE(context, nullptr);
    const auto &value = MockInsightIntentAniEnv::Get(context->aniObj);
    EXPECT_EQ(MockInsightIntentAniEnv::Get(value.properties.at("toolCallId")).text, TOOL_CALL_ID);
    EXPECT_EQ(value.numbers.at("instanceId"), INTENT_ID);
    EXPECT_EQ(value.numbers.at("nativeContext"), reinterpret_cast<ani_long>(etsContext_.get()));
    EXPECT_EQ(env_.toolCallIdSetCalls, 1);
    EXPECT_EQ(env_.contextGlobalRefCalls, 1);
}

/**
 * @tc.name: ContextEmptyToolCallId_0900
 * @tc.desc: Empty identifiers leave the ETS property undefined without allocating or setting a string.
 * @tc.type: FUNC
 */
HWTEST_F(EtsInsightIntentToolCallIdTest, ContextEmptyToolCallId_0900, TestSize.Level1)
{
    auto context = CreateContext("");
    ASSERT_NE(context, nullptr);
    EXPECT_EQ(MockInsightIntentAniEnv::Get(context->aniObj).properties.at("toolCallId"), env_.Undefined());
    EXPECT_EQ(env_.stringNewCalls, 0);
    EXPECT_EQ(env_.toolCallIdSetCalls, 0);
    EXPECT_EQ(env_.contextGlobalRefCalls, 1);
}

/**
 * @tc.name: ContextLegacyConstructor_1000
 * @tc.desc: Existing native callers using the original constructor keep an undefined ETS identifier.
 * @tc.type: FUNC
 */
HWTEST_F(EtsInsightIntentToolCallIdTest, ContextLegacyConstructor_1000, TestSize.Level1)
{
    auto nativeContext = std::make_shared<InsightIntentContext>(nullptr, BUNDLE_NAME, 0, INTENT_ID);
    EtsInsightIntentContext etsContext(nativeContext);
    auto context = CreateEtsInsightIntentContext(&env_, &etsContext);
    ASSERT_NE(context, nullptr);
    EXPECT_EQ(MockInsightIntentAniEnv::Get(context->aniObj).properties.at("toolCallId"), env_.Undefined());
    EXPECT_EQ(env_.toolCallIdSetCalls, 0);
}

/**
 * @tc.name: ContextToolCallIdIsolation_1100
 * @tc.desc: A later call without an identifier does not inherit a previous call's identifier.
 * @tc.type: FUNC
 */
HWTEST_F(EtsInsightIntentToolCallIdTest, ContextToolCallIdIsolation_1100, TestSize.Level1)
{
    auto first = CreateContext(TOOL_CALL_ID);
    auto second = CreateContext("");
    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    const auto firstToolCallId = MockInsightIntentAniEnv::Get(first->aniObj).properties.at("toolCallId");
    EXPECT_EQ(MockInsightIntentAniEnv::Get(firstToolCallId).text, TOOL_CALL_ID);
    EXPECT_EQ(MockInsightIntentAniEnv::Get(second->aniObj).properties.at("toolCallId"), env_.Undefined());
}

/**
 * @tc.name: ContextStringCreationFailure_1200
 * @tc.desc: Failed ANI string creation returns failure before publishing a global Context reference.
 * @tc.type: FUNC
 */
HWTEST_F(EtsInsightIntentToolCallIdTest, ContextStringCreationFailure_1200, TestSize.Level1)
{
    env_.stringNewStatus = ANI_OUT_OF_MEMORY;
    EXPECT_EQ(CreateContext(TOOL_CALL_ID), nullptr);
    EXPECT_EQ(env_.toolCallIdSetCalls, 0);
    EXPECT_EQ(env_.contextGlobalRefCalls, 0);
}

/**
 * @tc.name: ContextPropertyWriteFailure_1300
 * @tc.desc: Failed ANI field assignment does not publish a partially initialized Context reference.
 * @tc.type: FUNC
 */
HWTEST_F(EtsInsightIntentToolCallIdTest, ContextPropertyWriteFailure_1300, TestSize.Level1)
{
    env_.setToolCallIdStatus = ANI_NOT_FOUND;
    EXPECT_EQ(CreateContext(TOOL_CALL_ID), nullptr);
    EXPECT_EQ(env_.toolCallIdSetCalls, 1);
    EXPECT_EQ(env_.contextGlobalRefCalls, 0);
}

/**
 * @tc.name: ExecuteParamToEtsContext_1400
 * @tc.desc: The shared executor bridges the toolCallId field to ETS without changing business parameters.
 * @tc.type: FUNC
 */
HWTEST_F(EtsInsightIntentToolCallIdTest, ExecuteParamToEtsContext_1400, TestSize.Level1)
{
    InsightIntentExecutorInfo info;
    info.executeParam = std::make_shared<AppExecFwk::InsightIntentExecuteParam>();
    info.executeParam->bundleName_ = BUNDLE_NAME;
    info.executeParam->insightIntentId_ = INTENT_ID;
    info.executeParam->toolCallId_ = TOOL_CALL_ID;
    info.executeParam->insightIntentParam_ = std::make_shared<AAFwk::WantParams>();
    info.executeParam->insightIntentParam_->SetParam(
        AppExecFwk::INSIGHT_INTENT_TOOL_CALL_ID, AAFwk::String::Box("nested-value"));
    info.executeParam->insightIntentParam_->SetParam("toolCallId", AAFwk::String::Box("business-value"));
    TestInsightIntentExecutor executor;
    ASSERT_TRUE(executor.Init(info));
    EtsInsightIntentContext etsContext(executor.GetContext());
    auto context = CreateEtsInsightIntentContext(&env_, &etsContext);
    ASSERT_NE(context, nullptr);
    const auto toolCallId = MockInsightIntentAniEnv::Get(context->aniObj).properties.at("toolCallId");
    EXPECT_EQ(MockInsightIntentAniEnv::Get(toolCallId).text, TOOL_CALL_ID);
    EXPECT_EQ(info.executeParam->insightIntentParam_->GetStringParam("toolCallId"), "business-value");
    EXPECT_EQ(info.executeParam->insightIntentParam_->GetStringParam(AppExecFwk::INSIGHT_INTENT_TOOL_CALL_ID),
        "nested-value");
}
} // namespace OHOS::AbilityRuntime
