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
#include <gmock/gmock.h>

#include "ability_transaction_callback_info.h"
#define private public
#define protected public
#include "js_insight_intent_entry.h"
#include "insight_intent_executor.h"
#include "js_insight_intent_utils.h"
#include "js_runtime.h"
#include "js_environment.h"
#undef private
#undef protected
#include "mock_my_flag.h"
#include "want_params.h"
#include "string_wrapper.h"
#include "napi_common_execute_result.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "native_reference.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class NativeReferenceMock : public NativeReference {
public:
    NativeReferenceMock() = default;
    virtual ~NativeReferenceMock() = default;
    MOCK_METHOD0(Ref, uint32_t());
    MOCK_METHOD0(Unref, uint32_t());
    MOCK_METHOD0(Get, napi_value());
    MOCK_METHOD0(GetData, void*());
    virtual operator napi_value() override
    {
        return reinterpret_cast<napi_value>(this);
    }
    MOCK_METHOD0(SetDeleteSelf, void());
    MOCK_METHOD0(GetRefCount, uint32_t());
    MOCK_METHOD0(GetFinalRun, bool());
    napi_value GetNapiValue() override
    {
        napi_env env{nullptr};
        napi_value object = AppExecFwk::CreateJSObject(env);
        return object;
    }
};

class JsInsightIntentEntrySecondTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void JsInsightIntentEntrySecondTest::SetUpTestCase(void)
{}

void JsInsightIntentEntrySecondTest::TearDownTestCase(void)
{}

void JsInsightIntentEntrySecondTest::SetUp()
{}

void JsInsightIntentEntrySecondTest::TearDown()
{}

/*
* Feature: JsInsightIntentEntry
* Function: Init
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentEntrySecondTest, JsInsightIntentEntryInit_001, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    EXPECT_NE(jsRuntime->jsEnv_, nullptr);

    auto jsInsightIntentEntry = JsInsightIntentEntry::Create(*jsRuntime);
    jsInsightIntentEntry->jsObj_ = std::make_unique<NativeReferenceMock>();
    jsInsightIntentEntry->state_ = State::CREATED;
    InsightIntentExecutorInfo info;
    MyFlag::isGetNapiEnvNullptr_ = true;

    auto ret = jsInsightIntentEntry->Init(info);
    EXPECT_EQ(jsInsightIntentEntry->state_, State::INVALID);
    EXPECT_FALSE(ret);
}

/*
* Feature: JsInsightIntentEntry
* Function: Init
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentEntrySecondTest, JsInsightIntentEntryInit_002, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentEntry = JsInsightIntentEntry::Create(*jsRuntime);
    jsInsightIntentEntry->jsObj_ = std::make_unique<NativeReferenceMock>();
    jsInsightIntentEntry->state_ = State::CREATED;
    jsInsightIntentEntry->context_ = nullptr;

    panda::RuntimeOption pandaOption;
    jsRuntime->jsEnv_->Initialize(pandaOption, static_cast<void*>(this));
    MyFlag::isGetNapiEnvNullptr_ = false;

    InsightIntentExecutorInfo info;
    info.executeParam = nullptr;
    auto ret = jsInsightIntentEntry->Init(info);
    EXPECT_EQ(jsInsightIntentEntry->state_, State::INVALID);
    EXPECT_FALSE(ret);
}

/*
* Feature: JsInsightIntentEntry
* Function: Init
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentEntrySecondTest, JsInsightIntentEntryInit_003, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentEntry = JsInsightIntentEntry::Create(*jsRuntime);
    jsInsightIntentEntry->jsObj_ = std::make_unique<NativeReferenceMock>();
    jsInsightIntentEntry->state_ = State::CREATED;
    InsightIntentExecutorInfo info;
    info.executeParam = std::make_shared<AppExecFwk::InsightIntentExecuteParam>();
    
    panda::RuntimeOption pandaOption;
    jsRuntime->jsEnv_->Initialize(pandaOption, static_cast<void*>(this));
    MyFlag::isGetNapiEnvNullptr_ = false;
    MyFlag::isLoadSystemModuleByEngine_ = false;

    auto ret = jsInsightIntentEntry->Init(info);
    EXPECT_EQ(jsInsightIntentEntry->state_, State::INVALID);
    EXPECT_FALSE(ret);
}

/*
* Feature: JsInsightIntentEntry
* Function: Init
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentEntrySecondTest, JsInsightIntentEntryInit_004, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentEntry = JsInsightIntentEntry::Create(*jsRuntime);
    jsInsightIntentEntry->jsObj_ = std::make_unique<NativeReferenceMock>();
    jsInsightIntentEntry->state_ = State::CREATED;
    InsightIntentExecutorInfo info;
    info.executeParam = std::make_shared<AppExecFwk::InsightIntentExecuteParam>();

    panda::RuntimeOption pandaOption;
    jsRuntime->jsEnv_->Initialize(pandaOption, static_cast<void*>(this));
    MyFlag::isGetNapiEnvNullptr_ = false;
    MyFlag::isLoadSystemModuleByEngine_ = true;
    
    auto ret = jsInsightIntentEntry->Init(info);
    EXPECT_EQ(jsInsightIntentEntry->state_, State::INVALID);
    EXPECT_FALSE(ret);
}

/*
* Feature: JsInsightIntentEntry
* Function: HandleExecuteIntent
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentEntrySecondTest, JsInsightIntentEntryHandleExecuteIntent_001, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentEntry = JsInsightIntentEntry::Create(*jsRuntime);
    jsInsightIntentEntry->state_ = State::INITIALIZED;
    MyFlag::isGetNapiEnvNullptr_ = true;
    bool isAsync = false;
    auto ret = jsInsightIntentEntry->HandleExecuteIntent(nullptr, nullptr, nullptr, isAsync);
    EXPECT_EQ(jsInsightIntentEntry->state_, State::INVALID);
    EXPECT_FALSE(ret);
}

/*
* Feature: JsInsightIntentEntry
* Function: HandleExecuteIntent
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentEntrySecondTest, JsInsightIntentEntryHandleExecuteIntent_002, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentEntry = JsInsightIntentEntry::Create(*jsRuntime);
    jsInsightIntentEntry->state_ = State::INITIALIZED;
    auto callback = std::make_unique<InsightIntentExecutorAsyncCallback>();
    auto asyncCallback = [](AppExecFwk::InsightIntentExecuteResult result) {};
    callback->Push(asyncCallback);
    MyFlag::isGetNapiEnvNullptr_ = true;
    bool isAsync = false;
    auto ret = jsInsightIntentEntry->HandleExecuteIntent(nullptr, nullptr, std::move(callback), isAsync);
    EXPECT_EQ(jsInsightIntentEntry->state_, State::INVALID);
    EXPECT_FALSE(ret);
}

/*
* Feature: JsInsightIntentEntry
* Function: HandleExecuteIntent
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentEntrySecondTest, JsInsightIntentEntryHandleExecuteIntent_003, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentEntry = JsInsightIntentEntry::Create(*jsRuntime);
    jsInsightIntentEntry->state_ = State::INITIALIZED;
    auto executeParam = std::make_shared<AppExecFwk::InsightIntentExecuteParam>();
    executeParam->insightIntentParam_ = std::make_shared<AAFwk::WantParams>();
    executeParam->executeMode_ = -1;
    MyFlag::isGetNapiEnvNullptr_ = true;
    auto callback = std::make_unique<InsightIntentExecutorAsyncCallback>();
    auto asyncCallback = [](AppExecFwk::InsightIntentExecuteResult result) {};
    callback->Push(asyncCallback);
    bool isAsync = false;
    auto ret = jsInsightIntentEntry->HandleExecuteIntent(executeParam, nullptr, std::move(callback), isAsync);
    EXPECT_EQ(jsInsightIntentEntry->state_, State::INVALID);
    EXPECT_FALSE(ret);
}

/*
* Feature: JsInsightIntentEntry
* Function: LoadJsCode
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentEntrySecondTest, JsInsightIntentEntryLoadJsCode_001, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentEntry = JsInsightIntentEntry::Create(*jsRuntime);
    MyFlag::isGetNapiEnvNullptr_ = true;
    InsightIntentExecutorInfo info;
    info.executeParam = std::make_shared<AppExecFwk::InsightIntentExecuteParam>();
    EXPECT_NE(info.executeParam, nullptr);
    MyFlag::isExecuteSecureWithOhmUrl_ = false;
    auto ret = jsInsightIntentEntry->LoadJsCode(info, *jsRuntime);
    EXPECT_EQ(ret, nullptr);
}

/*
* Feature: JsInsightIntentEntry
* Function: LoadJsCode
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentEntrySecondTest, JsInsightIntentEntryLoadJsCode_002, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentEntry = JsInsightIntentEntry::Create(*jsRuntime);
    MyFlag::isGetNapiEnvNullptr_ = true;
    InsightIntentExecutorInfo info;
    info.executeParam = std::make_shared<AppExecFwk::InsightIntentExecuteParam>();
    EXPECT_NE(info.executeParam, nullptr);
    MyFlag::isExecuteSecureWithOhmUrl_ = true;
    auto ret = jsInsightIntentEntry->LoadJsCode(info, *jsRuntime);
    EXPECT_EQ(ret, nullptr);
}

/*
* Feature: JsInsightIntentEntry
* Function: CallJsFunctionWithResultInner
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentEntrySecondTest, JsInsightIntentEntryCallJsFunctionWithResultInner_001, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentEntry = JsInsightIntentEntry::Create(*jsRuntime);
    jsInsightIntentEntry->jsObj_ = std::make_unique<NativeReferenceMock>();
    MyFlag::isGetNapiEnvNullptr_ = true;
    napi_value argv;
    napi_value result;
    auto ret = jsInsightIntentEntry->CallJsFunctionWithResultInner("test", 1, &argv);
    EXPECT_EQ(ret, nullptr);
}

/*
* Feature: JsInsightIntentEntry
* Function: ReplyFailedInner
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentEntrySecondTest, JsInsightIntentEntryReplyFailedInner_001, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentEntry = JsInsightIntentEntry::Create(*jsRuntime);
    jsInsightIntentEntry->state_ = State::CREATED;
    MyFlag::isGetNapiEnvNullptr_ = true;
    jsInsightIntentEntry->ReplyFailedInner(InsightIntentInnerErr::INSIGHT_INTENT_ERR_OK);
    EXPECT_EQ(jsInsightIntentEntry->state_, State::INVALID);
}

/*
* Feature: JsInsightIntentEntry
* Function: ReplySucceededInner
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentEntrySecondTest, JsInsightIntentEntryReplySucceededInner_001, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentEntry = JsInsightIntentEntry::Create(*jsRuntime);
    jsInsightIntentEntry->state_ = State::CREATED;
    jsInsightIntentEntry->ReplySucceededInner(nullptr);
    EXPECT_EQ(jsInsightIntentEntry->state_, State::EXECUTATION_DONE);
}

/*
* Feature: JsInsightIntentEntry
* Function: HandleResultReturnedFromJsFunc
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentEntrySecondTest, JsInsightIntentEntryHandleResultReturnedFromJsFunc_001, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentEntry = JsInsightIntentEntry::Create(*jsRuntime);
    jsInsightIntentEntry->state_ = State::CREATED;
    MyFlag::isGetNapiEnvNullptr_ = true;
    auto ret = jsInsightIntentEntry->HandleResultReturnedFromJsFunc(nullptr);
    EXPECT_EQ(jsInsightIntentEntry->state_, State::INVALID);
    EXPECT_FALSE(ret);
}

/*
* Feature: JsInsightIntentEntry
* Function: HandleResultReturnedFromJsFunc
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentEntrySecondTest, JsInsightIntentEntryHandleResultReturnedFromJsFunc_002, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentEntry = JsInsightIntentEntry::Create(*jsRuntime);
    jsInsightIntentEntry->state_ = State::CREATED;

    panda::RuntimeOption pandaOption;
    jsRuntime->jsEnv_->Initialize(pandaOption, static_cast<void*>(this));
    MyFlag::isGetNapiEnvNullptr_ = false;
    napi_value result;
    napi_env env = jsRuntime->GetNapiEnv();
    napi_create_int32(env, 1, &result);

    auto ret = jsInsightIntentEntry->HandleResultReturnedFromJsFunc(result);
    EXPECT_EQ(jsInsightIntentEntry->state_, State::INVALID);
    EXPECT_FALSE(ret);
}

/*
* Feature: JsInsightIntentEntry
* Function: HandleResultReturnedFromJsFunc
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentEntrySecondTest, JsInsightIntentEntryHandleResultReturnedFromJsFunc_003, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentEntry = JsInsightIntentEntry::Create(*jsRuntime);
    jsInsightIntentEntry->state_ = State::CREATED;

    InsightIntentExecutorInfo info;
    info.executeParam = std::make_shared<AppExecFwk::InsightIntentExecuteParam>();
    jsInsightIntentEntry->context_ = std::make_shared<InsightIntentContext>(info.token, info.executeParam->bundleName_,
        info.windowMode, info.executeParam->insightIntentId_);
    panda::RuntimeOption pandaOption;
    jsRuntime->jsEnv_->Initialize(pandaOption, static_cast<void*>(this));
    MyFlag::isGetNapiEnvNullptr_ = false;

    napi_value promise;
    napi_deferred deferred;
    napi_env env = jsRuntime->GetNapiEnv();
    napi_create_promise(env, &deferred, &promise);

    auto ret = jsInsightIntentEntry->HandleResultReturnedFromJsFunc(promise);
    EXPECT_TRUE(ret);
}

/*
* Feature: JsInsightIntentEntry
* Function: ExecuteIntentCheckError
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentEntrySecondTest, JsInsightIntentEntryExecuteIntentCheckError_001, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentEntry = JsInsightIntentEntry::Create(*jsRuntime);
    MyFlag::isGetNapiEnvNullptr_ = true;
    jsInsightIntentEntry->state_ = State::CREATED;
    auto ret = jsInsightIntentEntry->ExecuteIntentCheckError();
    EXPECT_EQ(jsInsightIntentEntry->state_, State::INVALID);
    EXPECT_FALSE(ret);
}

/*
* Feature: JsInsightIntentEntry
* Function: ExecuteInsightIntent
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentEntrySecondTest, JsInsightIntentEntryExecuteInsightIntent_001, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentEntry = JsInsightIntentEntry::Create(*jsRuntime);
    jsInsightIntentEntry->state_ = State::CREATED;
    MyFlag::isGetNapiEnvNullptr_ = true;
    std::string name = "name";
    WantParams params;
    auto ret = jsInsightIntentEntry->ExecuteInsightIntent(name, params, nullptr);
    EXPECT_EQ(jsInsightIntentEntry->state_, State::INVALID);
    EXPECT_FALSE(ret);
}

/*
* Feature: JsInsightIntentEntry
* Function: ExecuteInsightIntent
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentEntrySecondTest, JsInsightIntentEntryExecuteInsightIntent_002, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    panda::RuntimeOption pandaOption;
    jsRuntime->jsEnv_->Initialize(pandaOption, static_cast<void*>(this));

    auto jsInsightIntentEntry = JsInsightIntentEntry::Create(*jsRuntime);
    jsInsightIntentEntry->jsObj_ = std::make_unique<NativeReferenceMock>();
    MyFlag::isGetNapiEnvNullptr_ = false;

    std::string name = "name";
    WantParams params;
    auto ret = jsInsightIntentEntry->ExecuteInsightIntent(name, params, nullptr);
    EXPECT_FALSE(ret);
}

/*
* Feature: JsInsightIntentEntry
* Function: PrepareParameters
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentEntrySecondTest, JsInsightIntentEntryPrepareParameters_001, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentEntry = JsInsightIntentEntry::Create(*jsRuntime);
    panda::RuntimeOption pandaOption;
    jsRuntime->jsEnv_->Initialize(pandaOption, static_cast<void*>(this));
    MyFlag::isGetNapiEnvNullptr_ = false;
    auto ret = jsInsightIntentEntry->PrepareParameters(InsightIntentExecuteMode::Count, nullptr);
    EXPECT_FALSE(ret);
}

/*
* Feature: JsInsightIntentEntry
* Function: PrepareParameters
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentEntrySecondTest, JsInsightIntentEntryPrepareParameters_002, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentEntry = JsInsightIntentEntry::Create(*jsRuntime);
    panda::RuntimeOption pandaOption;
    jsRuntime->jsEnv_->Initialize(pandaOption, static_cast<void*>(this));
    MyFlag::isGetNapiEnvNullptr_ = false;
    auto ret = jsInsightIntentEntry->PrepareParameters(InsightIntentExecuteMode::UIABILITY_FOREGROUND, nullptr);
    EXPECT_FALSE(ret);
}

/*
* Feature: JsInsightIntentEntry
* Function: PrepareParameters
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentEntrySecondTest, JsInsightIntentEntryPrepareParameters_003, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentEntry = JsInsightIntentEntry::Create(*jsRuntime);
    jsInsightIntentEntry->jsObj_ = nullptr;
    panda::RuntimeOption pandaOption;
    jsRuntime->jsEnv_->Initialize(pandaOption, static_cast<void*>(this));
    MyFlag::isGetNapiEnvNullptr_ = false;
    auto ret = jsInsightIntentEntry->PrepareParameters(InsightIntentExecuteMode::UIABILITY_BACKGROUND, nullptr);
    EXPECT_FALSE(ret);
}

/*
* Feature: JsInsightIntentEntry
* Function: PrepareParameters
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentEntrySecondTest, JsInsightIntentEntryPrepareParameters_004, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentEntry = JsInsightIntentEntry::Create(*jsRuntime);
    panda::RuntimeOption pandaOption;
    jsRuntime->jsEnv_->Initialize(pandaOption, static_cast<void*>(this));
    MyFlag::isGetNapiEnvNullptr_ = false;
    auto ret = jsInsightIntentEntry->PrepareParameters(InsightIntentExecuteMode::UIEXTENSION_ABILITY, nullptr);
    EXPECT_FALSE(ret);
}

/*
* Feature: JsInsightIntentEntry
* Function: PrepareParameters
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentEntrySecondTest, JsInsightIntentEntryPrepareParameters_005, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentEntry = JsInsightIntentEntry::Create(*jsRuntime);
    jsInsightIntentEntry->jsObj_ = nullptr;
    panda::RuntimeOption pandaOption;
    jsRuntime->jsEnv_->Initialize(pandaOption, static_cast<void*>(this));
    MyFlag::isGetNapiEnvNullptr_ = false;
    auto ret = jsInsightIntentEntry->PrepareParameters(InsightIntentExecuteMode::SERVICE_EXTENSION_ABILITY, nullptr);
    EXPECT_FALSE(ret);
}

/*
* Feature: JsInsightIntentEntry
* Function: PrepareParameters
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentEntrySecondTest, JsInsightIntentEntryPrepareParameters_006, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentEntry = JsInsightIntentEntry::Create(*jsRuntime);
    panda::RuntimeOption pandaOption;
    jsRuntime->jsEnv_->Initialize(pandaOption, static_cast<void*>(this));
    MyFlag::isGetNapiEnvNullptr_ = false;
    auto ret = jsInsightIntentEntry->PrepareParameters(InsightIntentExecuteMode::Count, nullptr);
    EXPECT_FALSE(ret);
}

/*
* Feature: JsInsightIntentEntry
* Function: PrepareParametersUIAbilityForeground
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentEntrySecondTest, JsInsightIntentEntryPrepareParametersUIAbilityForeground_001, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentEntry = JsInsightIntentEntry::Create(*jsRuntime);
    MyFlag::isGetNapiEnvNullptr_ = true;
    auto ret = jsInsightIntentEntry->PrepareParametersUIAbilityForeground(nullptr, nullptr);
    EXPECT_FALSE(ret);
}

/*
* Feature: JsInsightIntentEntry
* Function: PrepareParametersUIAbilityForeground
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentEntrySecondTest, JsInsightIntentEntryPrepareParametersUIAbilityForeground_002, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentEntry = JsInsightIntentEntry::Create(*jsRuntime);
    auto pageLoader = std::shared_ptr<NativeReference>();
    jsInsightIntentEntry->jsObj_ = std::make_unique<NativeReferenceMock>();
    auto ret = jsInsightIntentEntry->PrepareParametersUIAbilityForeground(nullptr, pageLoader);
    EXPECT_FALSE(ret);
}

/*
* Feature: JsInsightIntentEntry
* Function: PrepareParametersUIAbilityBackground
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentEntrySecondTest, JsInsightIntentEntryPrepareParametersUIAbilityBackground_001, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentEntry = JsInsightIntentEntry::Create(*jsRuntime);
    jsInsightIntentEntry->jsObj_ = nullptr;
    auto ret = jsInsightIntentEntry->PrepareParametersUIAbilityBackground(nullptr);
    EXPECT_FALSE(ret);
}

/*
* Feature: JsInsightIntentEntry
* Function: PrepareParametersUIAbilityBackground
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentEntrySecondTest, JsInsightIntentEntryPrepareParametersUIAbilityBackground_002, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentEntry = JsInsightIntentEntry::Create(*jsRuntime);
    jsInsightIntentEntry->jsObj_ = std::make_unique<NativeReferenceMock>();
    auto ret = jsInsightIntentEntry->PrepareParametersUIAbilityBackground(nullptr);
    EXPECT_FALSE(ret);
}

/*
* Feature: JsInsightIntentEntry
* Function: PrepareParametersUIExtension
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentEntrySecondTest, JsInsightIntentEntryPrepareParametersUIExtension_001, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentEntry = JsInsightIntentEntry::Create(*jsRuntime);
    jsInsightIntentEntry->jsObj_ = std::make_unique<NativeReferenceMock>();
    auto ret = jsInsightIntentEntry->PrepareParametersUIExtension(nullptr, nullptr);
    EXPECT_FALSE(ret);
}

/*
* Feature: JsInsightIntentEntry
* Function: PrepareParametersUIExtension
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentEntrySecondTest, JsInsightIntentEntryPrepareParametersUIExtension_002, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentEntry = JsInsightIntentEntry::Create(*jsRuntime);
    jsInsightIntentEntry->jsObj_ = std::make_unique<NativeReferenceMock>();
    auto pageLoader = std::make_shared<NativeReferenceMock>();
    auto ret = jsInsightIntentEntry->PrepareParametersUIExtension(nullptr, pageLoader);
    EXPECT_FALSE(ret);
}

/*
* Feature: JsInsightIntentEntry
* Function: PrepareParametersServiceExtension
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentEntrySecondTest, JsInsightIntentEntryPrepareParametersServiceExtension_001, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentEntry = JsInsightIntentEntry::Create(*jsRuntime);
    jsInsightIntentEntry->jsObj_ = nullptr;
    auto ret = jsInsightIntentEntry->PrepareParametersServiceExtension(nullptr);
    EXPECT_FALSE(ret);
}

/*
* Feature: JsInsightIntentEntry
* Function: PrepareParametersServiceExtension
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentEntrySecondTest, JsInsightIntentEntryPrepareParametersServiceExtension_002, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentEntry = JsInsightIntentEntry::Create(*jsRuntime);
    jsInsightIntentEntry->jsObj_ = std::make_unique<NativeReferenceMock>();
    auto ret = jsInsightIntentEntry->PrepareParametersServiceExtension(nullptr);
    EXPECT_FALSE(ret);
}

/*
* Feature: JsInsightIntentEntry
* Function: AssignObject
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentEntrySecondTest, JsInsightIntentEntryAssignObject_001, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentEntry = JsInsightIntentEntry::Create(*jsRuntime);
    jsInsightIntentEntry->jsObj_ = std::make_unique<NativeReferenceMock>();
    WantParams want;
    auto ret = jsInsightIntentEntry->AssignObject(nullptr, want);
    EXPECT_FALSE(ret);
}
} // namespace AbilityRuntime
} // namespace OHOS
