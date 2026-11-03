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
#include "js_insight_intent_page.h"
#include "js_insight_intent_utils.h"
#include "js_runtime.h"
#include "js_environment.h"
#undef private
#undef protected
#include "mock_my_flag.h"
#include "want_params.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class JsInsightIntentPageSecondTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void JsInsightIntentPageSecondTest::SetUpTestCase(void)
{}

void JsInsightIntentPageSecondTest::TearDownTestCase(void)
{}

void JsInsightIntentPageSecondTest::SetUp()
{}

void JsInsightIntentPageSecondTest::TearDown()
{}

/*
* Feature: JsInsightIntentFunc
* Function: Init
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentPageSecondTest, JsInsightIntentPageInit_001, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentPage = JsInsightIntentPage::Create(*jsRuntime);
    MyFlag::isGetNapiEnvNullptr_ = true;
    MyFlag::isExecuteSecureWithOhmUrl_ = true;
    InsightIntentExecutorInfo info;
    info.executeParam = std::make_shared<AppExecFwk::InsightIntentExecuteParam>();
    info.executeParam->moduleName_ = "entry";
    info.executeParam->srcEntrance_ = "ets/pages/Index";
    auto res = jsInsightIntentPage->Init(info);
    EXPECT_TRUE(res);
}

/*
* Feature: JsInsightIntentFunc
* Function: HandleExecuteIntent
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentPageSecondTest, JsInsightIntentPageHandleExecuteIntent_001, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentPage = JsInsightIntentPage::Create(*jsRuntime);
    jsInsightIntentPage->state_ = State::INITIALIZED;
    bool isAsync = false;
    auto res = jsInsightIntentPage->HandleExecuteIntent(nullptr, nullptr, nullptr, isAsync);
    EXPECT_EQ(jsInsightIntentPage->state_, State::INVALID);
    EXPECT_FALSE(res);
}

/*
* Feature: JsInsightIntentFunc
* Function: HandleExecuteIntent
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentPageSecondTest, JsInsightIntentPageHandleExecuteIntent_002, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentPage = JsInsightIntentPage::Create(*jsRuntime);
    jsInsightIntentPage->state_ = State::INITIALIZED;
    auto callback = std::make_unique<InsightIntentExecutorAsyncCallback>();
    auto asyncCallback = [](AppExecFwk::InsightIntentExecuteResult result) {};
    callback->Push(asyncCallback);
    bool isAsync = false;
    auto res = jsInsightIntentPage->HandleExecuteIntent(nullptr, nullptr, std::move(callback), isAsync);
    EXPECT_EQ(jsInsightIntentPage->state_, State::INVALID);
    EXPECT_FALSE(res);
}

/*
* Feature: JsInsightIntentFunc
* Function: HandleExecuteIntent
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentPageSecondTest, JsInsightIntentPageHandleExecuteIntent_003, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentPage = JsInsightIntentPage::Create(*jsRuntime);
    jsInsightIntentPage->state_ = State::INITIALIZED;
    auto executeParam = std::make_shared<AppExecFwk::InsightIntentExecuteParam>();
    executeParam->insightIntentParam_ = std::make_shared<AAFwk::WantParams>();
    auto callback = std::make_unique<InsightIntentExecutorAsyncCallback>();
    auto asyncCallback = [](AppExecFwk::InsightIntentExecuteResult result) {};
    callback->Push(asyncCallback);
    bool isAsync = true;
    auto res = jsInsightIntentPage->HandleExecuteIntent(executeParam, nullptr, std::move(callback), isAsync);
    EXPECT_FALSE(isAsync);
    EXPECT_TRUE(res);
}

/*
* Feature: JsInsightIntentFunc
* Function: LoadJsCode
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentPageSecondTest, JsInsightIntentPageLoadJsCode_001, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentPage = JsInsightIntentPage::Create(*jsRuntime);
    InsightIntentExecutorInfo info;
    info.executeParam = std::make_shared<AppExecFwk::InsightIntentExecuteParam>();
    info.executeParam->moduleName_ = "entry";
    info.executeParam->srcEntrance_ = "ets/pages/Index";
    EXPECT_NE(info.executeParam, nullptr);
    MyFlag::isExecuteSecureWithOhmUrl_ = false;
    auto res = jsInsightIntentPage->LoadJsCode(info, *jsRuntime);
    EXPECT_FALSE(res);
}

/*
* Feature: JsInsightIntentFunc
* Function: LoadJsCode
* SubFunction: invalid path
*/
HWTEST_F(JsInsightIntentPageSecondTest, JsInsightIntentPageLoadJsCode_InvalidPath_001, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentPage = JsInsightIntentPage::Create(*jsRuntime);
    InsightIntentExecutorInfo info;
    info.executeParam = std::make_shared<AppExecFwk::InsightIntentExecuteParam>();
    info.executeParam->moduleName_ = "entry";
    info.executeParam->srcEntrance_ = "../evil";
    auto res = jsInsightIntentPage->LoadJsCode(info, *jsRuntime);
    EXPECT_FALSE(res);
}

/*
* Feature: JsInsightIntentFunc
* Function: ReplyFailedInner
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentPageSecondTest, JsInsightIntentPageReplyFailedInner_001, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentPage = JsInsightIntentPage::Create(*jsRuntime);
    jsInsightIntentPage->state_ = State::INITIALIZED;
    jsInsightIntentPage->ReplyFailedInner(InsightIntentInnerErr::INSIGHT_INTENT_ERR_OK);
    EXPECT_EQ(jsInsightIntentPage->state_, State::INVALID);
}

/*
* Feature: JsInsightIntentFunc
* Function: ReplySucceededInner
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentPageSecondTest, JsInsightIntentPageReplySucceededInner_001, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentPage = JsInsightIntentPage::Create(*jsRuntime);
    jsInsightIntentPage->state_ = State::INITIALIZED;
    jsInsightIntentPage->ReplySucceededInner(nullptr);
    EXPECT_EQ(jsInsightIntentPage->state_, State::EXECUTATION_DONE);
}

/*
* Feature: JsInsightIntentFunc
* Function: ExecuteInsightIntent
* SubFunction: NA
*/
HWTEST_F(JsInsightIntentPageSecondTest, JsInsightIntentPageExecuteInsightIntent_001, TestSize.Level1)
{
    auto jsRuntime = std::make_shared<JsRuntime>();
    auto jsInsightIntentPage = JsInsightIntentPage::Create(*jsRuntime);
    AAFwk::WantParams wantParams;
    auto res = jsInsightIntentPage->ExecuteInsightIntent("", wantParams);
    EXPECT_TRUE(res);
}
} // namespace AbilityRuntime
} // namespace OHOS
