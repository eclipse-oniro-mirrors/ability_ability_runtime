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

#include "ability_context_impl.h"
#include "ani_common_execute_param.h"
#include "context.h"
#include "insight_intent_execute_param.h"
#include "ipc_object_stub.h"
#include "message_parcel.h"
#include "mock_ani_env.h"
#include "string_wrapper.h"

using namespace testing::ext;

namespace OHOS::AbilityRuntime {
namespace {
constexpr const char *BUNDLE_NAME = "com.example.intent";
} // namespace

class EtsInsightIntentContextTest : public testing::Test {
protected:
    void SetUp() override
    {
        param_ = env_.NewObject();
        auto &properties = MockInsightIntentAniEnv::Get(param_).properties;
        properties["bundleName"] = env_.NewString(BUNDLE_NAME);
        properties["moduleName"] = env_.NewString("entry");
        properties["abilityName"] = env_.NewString("IntentAbility");
        properties["insightIntentName"] = env_.NewString("QueryWeather");
        auto mode = env_.NewObject();
        MockInsightIntentAniEnv::Get(mode).numbers["value"] = AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND;
        properties["executeMode"] = mode;
        auto record = env_.NewObject();
        MockInsightIntentAniEnv::Get(record).wantParams.SetParam("city", AAFwk::String::Box("Shanghai"));
        properties["insightIntentParam"] = record;
    }

    void TearDown() override
    {
        for (auto *weakContext : weakContexts_) {
            delete weakContext;
        }
        weakContexts_.clear();
        contexts_.clear();
    }

    void SetContext(ani_ref value)
    {
        MockInsightIntentAniEnv::Get(param_).properties["context"] = value;
    }

    ani_object CreateAbilityContextObject(const sptr<IRemoteObject> &callerToken)
    {
        auto abilityContext = std::make_shared<AbilityContextImpl>();
        abilityContext->SetToken(callerToken);
        contexts_.push_back(abilityContext);
        auto *weakContext = new std::weak_ptr<Context>(abilityContext);
        weakContexts_.push_back(weakContext);
        auto contextObj = env_.NewObject();
        MockInsightIntentAniEnv::Get(contextObj).numbers["nativeContext"] =
            reinterpret_cast<ani_long>(weakContext);
        return contextObj;
    }

    void ExpectExistingParams(const AppExecFwk::InsightIntentExecuteParam &param)
    {
        EXPECT_EQ(param.bundleName_, BUNDLE_NAME);
        EXPECT_EQ(param.moduleName_, "entry");
        EXPECT_EQ(param.abilityName_, "IntentAbility");
        EXPECT_EQ(param.insightIntentName_, "QueryWeather");
        EXPECT_EQ(param.executeMode_, AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND);
    }

    MockInsightIntentAniEnv env_;
    ani_object param_ = nullptr;
    std::vector<std::shared_ptr<Context>> contexts_;
    std::vector<std::weak_ptr<Context> *> weakContexts_;
};

/**
 * @tc.name: UnwrapContextAbsent_0100
 * @tc.desc: A legacy input without the context property parses with a null token (BR-1).
 * @tc.type: FUNC
 */
HWTEST_F(EtsInsightIntentContextTest, UnwrapContextAbsent_0100, TestSize.Level1)
{
    AppExecFwk::InsightIntentExecuteParam param;
    ASSERT_TRUE(UnwrapExecuteParam(&env_, param_, param));
    EXPECT_EQ(param.callerAbilityToken_, nullptr);
    ExpectExistingParams(param);
}

/**
 * @tc.name: UnwrapContextUndefined_0200
 * @tc.desc: An explicit undefined context is treated as not provided (BR-1).
 * @tc.type: FUNC
 */
HWTEST_F(EtsInsightIntentContextTest, UnwrapContextUndefined_0200, TestSize.Level1)
{
    SetContext(env_.Undefined());
    AppExecFwk::InsightIntentExecuteParam param;
    ASSERT_TRUE(UnwrapExecuteParam(&env_, param_, param));
    EXPECT_EQ(param.callerAbilityToken_, nullptr);
    ExpectExistingParams(param);
}

/**
 * @tc.name: UnwrapContextNull_0300
 * @tc.desc: An explicit null context is treated as not provided (BR-1).
 * @tc.type: FUNC
 */
HWTEST_F(EtsInsightIntentContextTest, UnwrapContextNull_0300, TestSize.Level1)
{
    SetContext(env_.Null());
    AppExecFwk::InsightIntentExecuteParam param;
    ASSERT_TRUE(UnwrapExecuteParam(&env_, param_, param));
    EXPECT_EQ(param.callerAbilityToken_, nullptr);
    ExpectExistingParams(param);
}

/**
 * @tc.name: UnwrapContextNotAbilityContext_0400
 * @tc.desc: A plain object is not a framework-bound ability context: the parse succeeds
 *           and the token stays null (best-effort extraction, graceful degradation).
 * @tc.type: FUNC
 */
HWTEST_F(EtsInsightIntentContextTest, UnwrapContextNotAbilityContext_0400, TestSize.Level1)
{
    SetContext(env_.NewObject());
    AppExecFwk::InsightIntentExecuteParam param;
    ASSERT_TRUE(UnwrapExecuteParam(&env_, param_, param));
    EXPECT_EQ(param.callerAbilityToken_, nullptr);
}

/**
 * @tc.name: UnwrapContextString_0500
 * @tc.desc: A non-object context value is not a framework-bound ability context: the
 *           parse succeeds and the token stays null (best-effort extraction).
 * @tc.type: FUNC
 */
HWTEST_F(EtsInsightIntentContextTest, UnwrapContextString_0500, TestSize.Level1)
{
    SetContext(env_.NewString("not-a-context"));
    AppExecFwk::InsightIntentExecuteParam param;
    ASSERT_TRUE(UnwrapExecuteParam(&env_, param_, param));
    EXPECT_EQ(param.callerAbilityToken_, nullptr);
}

/**
 * @tc.name: UnwrapContextResolved_0600
 * @tc.desc: A valid UIAbilityContext yields its caller token on the param during
 *           parsing; the Context object itself is never kept (BR-4).
 * @tc.type: FUNC
 */
HWTEST_F(EtsInsightIntentContextTest, UnwrapContextResolved_0600, TestSize.Level1)
{
    sptr<IRemoteObject> callerToken = sptr<IPCObjectStub>(new (std::nothrow) IPCObjectStub(u"test.CallerToken"));
    ASSERT_NE(callerToken, nullptr);
    SetContext(CreateAbilityContextObject(callerToken));
    AppExecFwk::InsightIntentExecuteParam param;
    ASSERT_TRUE(UnwrapExecuteParam(&env_, param_, param));
    EXPECT_EQ(param.callerAbilityToken_, callerToken);
    ExpectExistingParams(param);
}

/**
 * @tc.name: UnwrapContextTokenUnavailable_0650
 * @tc.desc: A valid UIAbilityContext whose token is unavailable parses successfully and
 *           keeps a null token; the caller identity falls back to the pid-based
 *           adjudication (graceful degradation, no context object kept).
 * @tc.type: FUNC
 */
HWTEST_F(EtsInsightIntentContextTest, UnwrapContextTokenUnavailable_0650, TestSize.Level1)
{
    SetContext(CreateAbilityContextObject(nullptr));
    AppExecFwk::InsightIntentExecuteParam param;
    ASSERT_TRUE(UnwrapExecuteParam(&env_, param_, param));
    EXPECT_EQ(param.callerAbilityToken_, nullptr);
    ExpectExistingParams(param);
}

/**
 * @tc.name: ContextTokenMarshallingRoundTrip_0700
 * @tc.desc: The parcelable never serializes the caller token (BR-4): it stays null
 *           after a round trip even when set; the token is conveyed by the IPC proxy
 *           as a message-level tail instead.
 * @tc.type: FUNC
 */
HWTEST_F(EtsInsightIntentContextTest, ContextTokenMarshallingRoundTrip_0700, TestSize.Level1)
{
    sptr<IRemoteObject> callerToken = sptr<IPCObjectStub>(new (std::nothrow) IPCObjectStub(u"test.CallerToken"));
    ASSERT_NE(callerToken, nullptr);
    AppExecFwk::InsightIntentExecuteParam originalParam;
    originalParam.insightIntentParam_ = std::make_shared<AAFwk::WantParams>();
    originalParam.callerAbilityToken_ = callerToken;
    MessageParcel parcel;
    EXPECT_EQ(originalParam.Marshalling(parcel), true);

    AppExecFwk::InsightIntentExecuteParam unmarshalledParam;
    EXPECT_EQ(unmarshalledParam.ReadFromParcel(parcel), true);
    EXPECT_EQ(unmarshalledParam.callerAbilityToken_, nullptr);
}

/**
 * @tc.name: ContextTokenMarshallingAbsent_0800
 * @tc.desc: Without a context the token stays null across a round trip (BR-1).
 * @tc.type: FUNC
 */
HWTEST_F(EtsInsightIntentContextTest, ContextTokenMarshallingAbsent_0800, TestSize.Level1)
{
    AppExecFwk::InsightIntentExecuteParam originalParam;
    originalParam.bundleName_ = BUNDLE_NAME;
    originalParam.insightIntentParam_ = std::make_shared<AAFwk::WantParams>();
    MessageParcel parcel;
    EXPECT_EQ(originalParam.Marshalling(parcel), true);

    AppExecFwk::InsightIntentExecuteParam unmarshalledParam;
    EXPECT_EQ(unmarshalledParam.ReadFromParcel(parcel), true);
    EXPECT_EQ(unmarshalledParam.callerAbilityToken_, nullptr);
}
} // namespace OHOS::AbilityRuntime
