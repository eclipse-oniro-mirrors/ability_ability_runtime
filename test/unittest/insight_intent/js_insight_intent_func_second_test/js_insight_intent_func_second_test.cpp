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

#include "ability_transaction_callback_info.h"

#define private public
#define protected public
#include "insight_intent_executor.h"
#include "js_insight_intent_func.h"
#include "js_insight_intent_utils.h"
#include "js_runtime.h"
#include "js_environment.h"
#undef private
#undef protected
#include "want_params.h"
#include "string_wrapper.h"
#include "mock_my_flag.h"
#include "napi_common_execute_result.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "native_reference.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
napi_value NapiTestFunc(napi_env env, napi_callback_info info)
{
    return nullptr;
}

class JsInsightIntentFuncSecondTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void JsInsightIntentFuncSecondTest::SetUpTestCase(void)
{}

void JsInsightIntentFuncSecondTest::TearDownTestCase(void)
{}

void JsInsightIntentFuncSecondTest::SetUp()
{}

void JsInsightIntentFuncSecondTest::TearDown()
{}

/*
* Feature: JsInsightIntentFunc
* Function: Init
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentFuncSecondTest, JsInsightIntentFuncInit_001, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentFunc = JsInsightIntentFunc::Create(*jsRuntime);
    jsInsightIntentFunc->state_ = State::CREATED;
    InsightIntentExecutorInfo info;
    info.executeParam = std::make_shared<AppExecFwk::InsightIntentExecuteParam>();
    info.executeParam->moduleName_ = "entry";
    info.executeParam->srcEntrance_ = "ets/pages/Index";
    MyFlag::isExecuteSecureWithOhmUrl_ = true;
    MyFlag::isGetNapiEnvNullptr_ = true;
    auto res = jsInsightIntentFunc->Init(info);
    EXPECT_EQ(jsInsightIntentFunc->state_, State::INITIALIZED);
    EXPECT_TRUE(res);
}

/*
* Feature: JsInsightIntentFunc
* Function: HandleExecuteIntent
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentFuncSecondTest, JsInsightIntentFuncHandleExecuteIntent_001, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentFunc = JsInsightIntentFunc::Create(*jsRuntime);
    bool isAsync = true;
    jsInsightIntentFunc->state_ = State::INITIALIZED;
    jsInsightIntentFunc->isAsync_ = false;
    auto ret = jsInsightIntentFunc->HandleExecuteIntent(nullptr, nullptr, nullptr, isAsync);
    EXPECT_EQ(jsInsightIntentFunc->state_, State::INVALID);
    EXPECT_FALSE(jsInsightIntentFunc->isAsync_);
    EXPECT_FALSE(ret);
}

/*
* Feature: JsInsightIntentFunc
* Function: HandleExecuteIntent
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentFuncSecondTest, JsInsightIntentFuncHandleExecuteIntent_002, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentFunc = JsInsightIntentFunc::Create(*jsRuntime);
    bool isAsync = true;
    jsInsightIntentFunc->state_ = State::INITIALIZED;
    jsInsightIntentFunc->isAsync_ = false;
    auto callback = std::make_unique<InsightIntentExecutorAsyncCallback>();
    EXPECT_NE(callback, nullptr);
    auto asyncCallback = [](AppExecFwk::InsightIntentExecuteResult result) {};
    callback->Push(asyncCallback);
    auto ret = jsInsightIntentFunc->HandleExecuteIntent(nullptr, nullptr, std::move(callback), isAsync);
    EXPECT_EQ(jsInsightIntentFunc->state_, State::INVALID);
    EXPECT_FALSE(isAsync);
    EXPECT_FALSE(ret);
}

/*
* Feature: JsInsightIntentFunc
* Function: LoadJsCode
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentFuncSecondTest, JsInsightIntentFuncLoadJsCode_001, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentFunc = JsInsightIntentFunc::Create(*jsRuntime);
    InsightIntentExecutorInfo info;
    info.executeParam = std::make_shared<AppExecFwk::InsightIntentExecuteParam>();
    info.executeParam->moduleName_ = "entry";
    info.executeParam->srcEntrance_ = "ets/pages/Index";
    MyFlag::isExecuteSecureWithOhmUrl_ = true;
    auto runTime = std::make_shared<JsRuntime>();
    auto ret = jsInsightIntentFunc->LoadJsCode(info, *runTime);
    EXPECT_TRUE(ret);
}

/*
* Feature: JsInsightIntentFunc
* Function: LoadJsCode
* SubFunction: invalid moduleName
*/
HWTEST_F(JsInsightIntentFuncSecondTest, JsInsightIntentFuncLoadJsCode_InvalidModuleName_001, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentFunc = JsInsightIntentFunc::Create(*jsRuntime);
    InsightIntentExecutorInfo info;
    info.executeParam = std::make_shared<AppExecFwk::InsightIntentExecuteParam>();
    info.executeParam->moduleName_ = "../evil";
    info.executeParam->srcEntrance_ = "ets/pages/Index";
    auto runTime = std::make_shared<JsRuntime>();
    auto ret = jsInsightIntentFunc->LoadJsCode(info, *runTime);
    EXPECT_FALSE(ret);
}

/*
* Feature: JsInsightIntentFunc
* Function: LoadJsCode
* SubFunction: invalid srcEntrance
*/
HWTEST_F(JsInsightIntentFuncSecondTest, JsInsightIntentFuncLoadJsCode_InvalidSrcEntrance_001, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentFunc = JsInsightIntentFunc::Create(*jsRuntime);
    InsightIntentExecutorInfo info;
    info.executeParam = std::make_shared<AppExecFwk::InsightIntentExecuteParam>();
    info.executeParam->moduleName_ = "entry";
    info.executeParam->srcEntrance_ = "../evil";
    auto runTime = std::make_shared<JsRuntime>();
    auto ret = jsInsightIntentFunc->LoadJsCode(info, *runTime);
    EXPECT_FALSE(ret);
}

/*
* Feature: JsInsightIntentFunc
* Function: LoadJsCode
* SubFunction: invalid hapPath
*/
HWTEST_F(JsInsightIntentFuncSecondTest, JsInsightIntentFuncLoadJsCode_InvalidHapPath_001, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentFunc = JsInsightIntentFunc::Create(*jsRuntime);
    InsightIntentExecutorInfo info;
    info.executeParam = std::make_shared<AppExecFwk::InsightIntentExecuteParam>();
    info.executeParam->moduleName_ = "entry";
    info.executeParam->srcEntrance_ = "ets/pages/Index";
    info.hapPath = "../evil.hap";
    auto runTime = std::make_shared<JsRuntime>();
    auto ret = jsInsightIntentFunc->LoadJsCode(info, *runTime);
    EXPECT_FALSE(ret);
}

/*
* Feature: JsInsightIntentFunc
* Function: ReplyFailedInner
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentFuncSecondTest, JsInsightIntentFuncReplyFailedInner_001, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentFunc = JsInsightIntentFunc::Create(*jsRuntime);

    jsInsightIntentFunc->state_ = State::EXECUTING;

    jsInsightIntentFunc->ReplyFailedInner(InsightIntentInnerErr::INSIGHT_INTENT_ERR_OK);
    EXPECT_EQ(jsInsightIntentFunc->state_, State::INVALID);
}

/*
* Feature: JsInsightIntentFunc
* Function: ReplySucceededInner
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentFuncSecondTest, JsInsightIntentFuncReplySucceededInner_001, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentFunc = JsInsightIntentFunc::Create(*jsRuntime);

    jsInsightIntentFunc->state_ = State::EXECUTING;

    jsInsightIntentFunc->ReplySucceededInner(nullptr);
    EXPECT_EQ(jsInsightIntentFunc->state_, State::EXECUTATION_DONE);
}

/*
* Feature: JsInsightIntentFunc
* Function: HandleResultReturnedFromJsFunc
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentFuncSecondTest, JsInsightIntentFuncHandleResultReturnedFromJsFunc_001, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentFunc = JsInsightIntentFunc::Create(*jsRuntime);

    jsInsightIntentFunc->state_ = State::EXECUTING;

    auto ret = jsInsightIntentFunc->HandleResultReturnedFromJsFunc(nullptr);
    EXPECT_EQ(jsInsightIntentFunc->state_, State::INVALID);
    EXPECT_FALSE(ret);
}

/*
* Feature: JsInsightIntentFunc
* Function: HandleResultReturnedFromJsFunc
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentFuncSecondTest, JsInsightIntentFuncHandleResultReturnedFromJsFunc_002, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    panda::RuntimeOption pandaOption;
    jsRuntime->jsEnv_->Initialize(pandaOption, static_cast<void*>(this));
    auto jsInsightIntentFunc = JsInsightIntentFunc::Create(*jsRuntime);

    jsInsightIntentFunc->state_ = State::EXECUTING;
    jsInsightIntentFunc->isAsync_ = false;
    MyFlag::isGetNapiEnvNullptr_ = false;
    napi_value promise;
    napi_deferred deferred;
    napi_env env = jsRuntime->GetNapiEnv();
    napi_create_promise(env, &deferred, &promise);

    auto ret = jsInsightIntentFunc->HandleResultReturnedFromJsFunc(promise);
    EXPECT_TRUE(jsInsightIntentFunc->isAsync_);
    EXPECT_TRUE(ret);
}

/*
* Feature: JsInsightIntentFunc
* Function: HandleResultReturnedFromJsFunc
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentFuncSecondTest, JsInsightIntentFuncHandleResultReturnedFromJsFunc_003, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    panda::RuntimeOption pandaOption;
    jsRuntime->jsEnv_->Initialize(pandaOption, static_cast<void*>(this));
    auto jsInsightIntentFunc = JsInsightIntentFunc::Create(*jsRuntime);

    jsInsightIntentFunc->state_ = State::EXECUTING;
    jsInsightIntentFunc->isAsync_ = true;
    MyFlag::isGetNapiEnvNullptr_ = false;
    napi_value result;
    napi_deferred deferred;
    napi_env env = jsRuntime->GetNapiEnv();
    napi_create_int32(env, 1, &result);

    auto ret = jsInsightIntentFunc->HandleResultReturnedFromJsFunc(result);
    EXPECT_FALSE(jsInsightIntentFunc->isAsync_);
    EXPECT_TRUE(ret);
}

/*
* Feature: JsInsightIntentFunc
* Function: GetResultFromJs
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentFuncSecondTest, JsInsightIntentFuncGetResultFromJs_001, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentFunc = JsInsightIntentFunc::Create(*jsRuntime);
    MyFlag::isGetNapiEnvNullptr_ = true;
    auto ret = jsInsightIntentFunc->GetResultFromJs(nullptr, nullptr);
    EXPECT_NE(ret, nullptr);
    EXPECT_EQ(ret->code, InsightIntentInnerErr::INSIGHT_INTENT_ERR_OK);
}

/*
* Feature: JsInsightIntentFunc
* Function: ResolveCbCpp
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentFuncSecondTest, JsInsightIntentFuncResolveCbCpp_001, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentFunc = JsInsightIntentFunc::Create(*jsRuntime);
    MyFlag::isGetNapiEnvNullptr_ = true;
    auto ret = jsInsightIntentFunc->ResolveCbCpp(nullptr, nullptr);
    EXPECT_EQ(ret, nullptr);
}

/*
* Feature: JsInsightIntentFunc
* Function: ExecuteIntentCheckError
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentFuncSecondTest, JsInsightIntentFuncExecuteIntentCheckError_001, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentFunc = JsInsightIntentFunc::Create(*jsRuntime);
    jsInsightIntentFunc->state_ = State::CREATED;
    auto ret = jsInsightIntentFunc->ExecuteIntentCheckError();
    EXPECT_EQ(jsInsightIntentFunc->state_, State::INVALID);
    EXPECT_FALSE(ret);
}

/*
* Feature: JsInsightIntentFunc
* Function: ExecuteInsightIntent
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentFuncSecondTest, JsInsightIntentFuncExecuteInsightIntent_001, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentFunc = JsInsightIntentFunc::Create(*jsRuntime);
    jsInsightIntentFunc->state_ = State::CREATED;
    auto ret = jsInsightIntentFunc->ExecuteInsightIntent(nullptr);
    EXPECT_EQ(jsInsightIntentFunc->state_, State::INVALID);
    EXPECT_FALSE(ret);
}

/*
* Feature: JsInsightIntentFunc
* Function: ExecuteInsightIntent
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentFuncSecondTest, JsInsightIntentFuncExecuteInsightIntent_002, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentFunc = JsInsightIntentFunc::Create(*jsRuntime);
    jsInsightIntentFunc->state_ = State::CREATED;
    auto executeParam = std::make_shared<InsightIntentExecuteParam>();
    executeParam->insightIntentParam_ = std::make_shared<WantParams>();
    MyFlag::isGetNapiEnvNullptr_ = true;
    EXPECT_EQ(jsRuntime->GetNapiEnv(), nullptr);
    auto ret = jsInsightIntentFunc->ExecuteInsightIntent(executeParam);
    EXPECT_EQ(jsInsightIntentFunc->state_, State::INVALID);
    EXPECT_FALSE(ret);
}

/*
* Feature: JsInsightIntentFunc
* Function: ExecuteInsightIntent
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentFuncSecondTest, JsInsightIntentFuncExecuteInsightIntent_003, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    panda::RuntimeOption pandaOption;
    jsRuntime->jsEnv_->Initialize(pandaOption, static_cast<void*>(this));
    auto jsInsightIntentFunc = JsInsightIntentFunc::Create(*jsRuntime);
    jsInsightIntentFunc->state_ = State::CREATED;
    auto executeParam = std::make_shared<InsightIntentExecuteParam>();
    executeParam->insightIntentParam_ = std::make_shared<WantParams>();
    MyFlag::isGetNapiEnvNullptr_ = false;
    EXPECT_NE(jsRuntime->GetNapiEnv(), nullptr);
    MyFlag::isGetExportObjectFromOhmUrlNullptr_ = true;
    auto ret = jsInsightIntentFunc->ExecuteInsightIntent(executeParam);
    EXPECT_FALSE(ret);
}

/*
* Feature: JsInsightIntentFunc
* Function: ExecuteInsightIntent
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentFuncSecondTest, JsInsightIntentFuncExecuteInsightIntent_004, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    panda::RuntimeOption pandaOption;
    jsRuntime->jsEnv_->Initialize(pandaOption, static_cast<void*>(this));
    auto jsInsightIntentFunc = JsInsightIntentFunc::Create(*jsRuntime);
    jsInsightIntentFunc->state_ = State::CREATED;
    auto executeParam = std::make_shared<InsightIntentExecuteParam>();
    executeParam->insightIntentParam_ = std::make_shared<WantParams>();
    MyFlag::isGetNapiEnvNullptr_ = false;
    MyFlag::isGetExportObjectFromOhmUrlNullptr_ = false;
    auto ret = jsInsightIntentFunc->ExecuteInsightIntent(executeParam);
    EXPECT_FALSE(ret);
}

/*
* Feature: JsInsightIntentFunc
* Function: ParseParams
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentFuncSecondTest, JsInsightIntentFuncParseParams_001, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    panda::RuntimeOption pandaOption;
    jsRuntime->jsEnv_->Initialize(pandaOption, static_cast<void*>(this));
    auto jsInsightIntentFunc = JsInsightIntentFunc::Create(*jsRuntime);
    MyFlag::isGetNapiEnvNullptr_ = false;
    WantParams params;
    size_t argc = 0;
    std::vector<napi_value> argv = {};
    auto ret = jsInsightIntentFunc->ParseParams(nullptr, params, {}, argc, argv);
    EXPECT_FALSE(ret);
}

/*
* Feature: JsInsightIntentFunc
* Function: GetTargetMethod
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentFuncSecondTest, JsInsightIntentFuncGetTargetMethod_001, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentFunc = JsInsightIntentFunc::Create(*jsRuntime);
    MyFlag::isGetNapiEnvNullptr_ = true;
    auto ret = jsInsightIntentFunc->GetTargetMethod(nullptr, nullptr, "");
    EXPECT_EQ(ret, nullptr);
}

/*
* Feature: JsInsightIntentFunc
* Function: GetTargetMethod
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentFuncSecondTest, JsInsightIntentFuncGetTargetMethod_002, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    panda::RuntimeOption pandaOption;
    jsRuntime->jsEnv_->Initialize(pandaOption, static_cast<void*>(this));
    auto jsInsightIntentFunc = JsInsightIntentFunc::Create(*jsRuntime);
    MyFlag::isGetNapiEnvNullptr_ = false;
    napi_value fn;
    napi_create_function(jsRuntime->GetNapiEnv(), "test", 0, NapiTestFunc, NULL, &fn);
    auto ret = jsInsightIntentFunc->GetTargetMethod(jsRuntime->GetNapiEnv(), fn, "test");
    EXPECT_EQ(ret, nullptr);
}
} // namespace AbilityRuntime
} // namespace OHOS
