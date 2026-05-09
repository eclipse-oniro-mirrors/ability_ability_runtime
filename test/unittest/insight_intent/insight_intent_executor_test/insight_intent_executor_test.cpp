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
#include "insight_intent_executor_mgr.h"
#include "ets_insight_intent_executor_instance.h"
#undef private
#undef protected
#include "mock_runtime.h"
#include "want_params.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class MockInsightIntentExecutor : public InsightIntentExecutor {
public:
    MockInsightIntentExecutor() {}
    ~MockInsightIntentExecutor() {}
    bool Init(const InsightIntentExecutorInfo& intentInfo) override
    {
        return InsightIntentExecutor::Init(intentInfo);
    }

    bool HandleExecuteIntent(
        std::shared_ptr<InsightIntentExecuteParam> executeParam,
        const std::shared_ptr<NativeReference>& pageLoader,
        std::unique_ptr<InsightIntentExecutorAsyncCallback> callback,
        bool& isAsync) override
    {
        return false;
    }
};

class InsightIntentExecutorTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void InsightIntentExecutorTest::SetUpTestCase(void)
{}

void InsightIntentExecutorTest::TearDownTestCase(void)
{}

void InsightIntentExecutorTest::SetUp()
{}

void InsightIntentExecutorTest::TearDown()
{}

class InsightIntentExecutorMgrTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void InsightIntentExecutorMgrTest::SetUpTestCase(void)
{}

void InsightIntentExecutorMgrTest::TearDownTestCase(void)
{}

void InsightIntentExecutorMgrTest::SetUp()
{}

void InsightIntentExecutorMgrTest::TearDown()
{}

/*
* Feature: JsInsightIntentFunc
* Function: Create
* SubFunction: NA
*/
HWTEST_F(InsightIntentExecutorTest, JsInsightIntentPageCreate_001, TestSize.Level1)
{
    auto runtime = std::make_shared<MockRuntime>();
    auto res = InsightIntentExecutor::Create(*runtime, InsightIntentType::DECOR_NONE);
    EXPECT_NE(res, nullptr);
}

/*
* Feature: JsInsightIntentFunc
* Function: Create
* SubFunction: NA
*/
HWTEST_F(InsightIntentExecutorTest, JsInsightIntentPageCreate_002, TestSize.Level1)
{
    auto runtime = std::make_shared<MockRuntime>();
    auto res = InsightIntentExecutor::Create(*runtime, InsightIntentType::DECOR_ENTRY);
    EXPECT_NE(res, nullptr);
}

/*
* Feature: JsInsightIntentFunc
* Function: Create
* SubFunction: NA
*/
HWTEST_F(InsightIntentExecutorTest, JsInsightIntentPageCreate_003, TestSize.Level1)
{
    auto runtime = std::make_shared<MockRuntime>();
    auto res = InsightIntentExecutor::Create(*runtime, InsightIntentType::DECOR_FUNC);
    EXPECT_NE(res, nullptr);
}

/*
* Feature: JsInsightIntentFunc
* Function: Create
* SubFunction: NA
*/
HWTEST_F(InsightIntentExecutorTest, JsInsightIntentPageCreate_004, TestSize.Level1)
{
    auto runtime = std::make_shared<MockRuntime>();
    auto res = InsightIntentExecutor::Create(*runtime, InsightIntentType::DECOR_PAGE);
    EXPECT_NE(res, nullptr);
}

/*
* Feature: JsInsightIntentFunc
* Function: Create
* SubFunction: NA
*/
HWTEST_F(InsightIntentExecutorTest, JsInsightIntentPageCreate_005, TestSize.Level1)
{
    auto runtime = std::make_shared<MockRuntime>();
    auto res = InsightIntentExecutor::Create(*runtime, InsightIntentType::DECOR_LINK);
    EXPECT_EQ(res, nullptr);
}

/*
* Feature: JsInsightIntentFunc
* Function: Create
* SubFunction: NA
*/
HWTEST_F(InsightIntentExecutorTest, JsInsightIntentPageCreate_006, TestSize.Level1)
{
    auto runtime = std::make_shared<MockRuntime>();
    runtime->SetLanguage(Runtime::Language::CJ);
    auto res = InsightIntentExecutor::Create(*runtime, InsightIntentType::DECOR_LINK);
    EXPECT_EQ(res, nullptr);
}

/*
* Feature: InsightIntentExecutor
* Function: Create
* SubFunction: Test ETS language with DECOR_NONE type
*/
HWTEST_F(InsightIntentExecutorTest, ETSInsightIntentExecutorCreate_001, TestSize.Level1)
{
    auto runtime = std::make_shared<MockRuntime>();
    runtime->SetLanguage(Runtime::Language::ETS);
    auto res = InsightIntentExecutor::Create(*runtime, InsightIntentType::DECOR_NONE);
    EXPECT_NE(res, nullptr);
}

/*
* Feature: InsightIntentExecutor
* Function: Create
* SubFunction: Test ETS language with DECOR_ENTRY type
*/
HWTEST_F(InsightIntentExecutorTest, ETSInsightIntentExecutorCreate_002, TestSize.Level1)
{
    auto runtime = std::make_shared<MockRuntime>();
    runtime->SetLanguage(Runtime::Language::ETS);
    auto res = InsightIntentExecutor::Create(*runtime, InsightIntentType::DECOR_ENTRY);
    EXPECT_NE(res, nullptr);
}

/*
* Feature: InsightIntentExecutor
* Function: Create
* SubFunction: Test ETS language with DECOR_FUNC type
*/
HWTEST_F(InsightIntentExecutorTest, ETSInsightIntentExecutorCreate_003, TestSize.Level1)
{
    auto runtime = std::make_shared<MockRuntime>();
    runtime->SetLanguage(Runtime::Language::ETS);
    auto res = InsightIntentExecutor::Create(*runtime, InsightIntentType::DECOR_FUNC);
    EXPECT_NE(res, nullptr);
}

/*
* Feature: InsightIntentExecutor
* Function: Create
* SubFunction: Test ETS language with DECOR_QUERY_ENTITY type
*/
HWTEST_F(InsightIntentExecutorTest, ETSInsightIntentExecutorCreate_004, TestSize.Level1)
{
    auto runtime = std::make_shared<MockRuntime>();
    runtime->SetLanguage(Runtime::Language::ETS);
    auto res = InsightIntentExecutor::Create(*runtime, InsightIntentType::DECOR_QUERY_ENTITY);
    EXPECT_NE(res, nullptr);
}

/*
* Feature: InsightIntentExecutor
* Function: Create
* SubFunction: Test ETS language with DECOR_PAGE type (returns nullptr)
*/
HWTEST_F(InsightIntentExecutorTest, ETSInsightIntentExecutorCreate_005, TestSize.Level1)
{
    auto runtime = std::make_shared<MockRuntime>();
    runtime->SetLanguage(Runtime::Language::ETS);
    auto res = InsightIntentExecutor::Create(*runtime, InsightIntentType::DECOR_PAGE);
    EXPECT_NE(res, nullptr);
}

/*
* Feature: InsightIntentExecutor
* Function: Create
* SubFunction: Test ETS language with DECOR_LINK type (returns nullptr)
*/
HWTEST_F(InsightIntentExecutorTest, ETSInsightIntentExecutorCreate_006, TestSize.Level1)
{
    auto runtime = std::make_shared<MockRuntime>();
    runtime->SetLanguage(Runtime::Language::ETS);
    auto res = InsightIntentExecutor::Create(*runtime, InsightIntentType::DECOR_LINK);
    EXPECT_EQ(res, nullptr);
}

/*
* Feature: InsightIntentExecutor
* Function: Create
* SubFunction: Test ETS language with DECOR_FORM type (returns nullptr)
*/
HWTEST_F(InsightIntentExecutorTest, ETSInsightIntentExecutorCreate_007, TestSize.Level1)
{
    auto runtime = std::make_shared<MockRuntime>();
    runtime->SetLanguage(Runtime::Language::ETS);
    auto res = InsightIntentExecutor::Create(*runtime, InsightIntentType::DECOR_FORM);
    EXPECT_EQ(res, nullptr);
}

/*
* Feature: InsightIntentExecutor
* Function: Create
* SubFunction: Test JS language with DECOR_QUERY_ENTITY type
*/
HWTEST_F(InsightIntentExecutorTest, JsInsightIntentExecutorCreate_001, TestSize.Level1)
{
    auto runtime = std::make_shared<MockRuntime>();
    auto res = InsightIntentExecutor::Create(*runtime, InsightIntentType::DECOR_QUERY_ENTITY);
    EXPECT_NE(res, nullptr);
}

/*
* Feature: InsightIntentExecutor
* Function: Create
* SubFunction: Test JS language with DECOR_FORM type (returns nullptr)
*/
HWTEST_F(InsightIntentExecutorTest, JsInsightIntentExecutorCreate_002, TestSize.Level1)
{
    auto runtime = std::make_shared<MockRuntime>();
    auto res = InsightIntentExecutor::Create(*runtime, InsightIntentType::DECOR_FORM);
    EXPECT_EQ(res, nullptr);
}

/*
* Feature: InsightIntentExecutor
* Function: Create
* SubFunction: Test default language (returns nullptr)
*/
HWTEST_F(InsightIntentExecutorTest, DefaultLanguageCreate_001, TestSize.Level1)
{
    auto runtime = std::make_shared<MockRuntime>();
    runtime->SetLanguage(Runtime::Language::CJ);  // CJ is not supported, returns nullptr
    auto res = InsightIntentExecutor::Create(*runtime, InsightIntentType::DECOR_NONE);
    EXPECT_EQ(res, nullptr);
}

/*
* Feature: JsInsightIntentFunc
* Function: Init
* SubFunction: NA
*/
HWTEST_F(InsightIntentExecutorTest, JsInsightIntentPageInit_001, TestSize.Level1)
{
    auto insightIntentExecutor = std::make_shared<MockInsightIntentExecutor>();
    InsightIntentExecutorInfo info;
    info.executeParam = std::make_shared<AppExecFwk::InsightIntentExecuteParam>();
    auto res = insightIntentExecutor->Init(info);
    EXPECT_TRUE(res);
}

/*
* Feature: JsInsightIntentFunc
* Function: CreateETSInsightIntentExecutor
* SubFunction: NA
*/
HWTEST_F(InsightIntentExecutorTest, JsInsightIntentPageCreateETSInsightIntentExecutor_001, TestSize.Level1)
{
    auto runtime = std::make_shared<MockRuntime>();
    auto res = CreateETSInsightIntentExecutor(*runtime);
    EXPECT_NE(res, nullptr);
}

/*
* Feature: JsInsightIntentFunc
* Function: ExecuteInsightIntent
* SubFunction: NA
*/
HWTEST_F(InsightIntentExecutorTest, JsInsightIntentPageExecuteInsightIntent_001, TestSize.Level1)
{
    auto runtime = std::make_shared<MockRuntime>();
    auto callback = std::make_unique<InsightIntentExecutorAsyncCallback>();
    EXPECT_NE(callback, nullptr);
    auto mgr = std::make_shared<InsightIntentExecutorMgr>();
    InsightIntentExecutorInfo info;
    info.executeParam = nullptr;
    auto res = mgr->ExecuteInsightIntent(*runtime, info, std::move(callback));
    EXPECT_FALSE(res);
}

/*
* Feature: JsInsightIntentFunc
* Function: AddInsightIntentExecutor
* SubFunction: NA
*/
HWTEST_F(InsightIntentExecutorTest, JsInsightIntentPageAddInsightIntentExecutor_001, TestSize.Level1)
{
    uint64_t intentId = 0;
    auto mgr = std::make_shared<InsightIntentExecutorMgr>();
    auto executor = std::make_shared<MockInsightIntentExecutor>();
    mgr->AddInsightIntentExecutor(intentId, executor);
    EXPECT_FALSE(mgr->insightIntentExecutors_.empty());
}

/*
* Feature: JsInsightIntentFunc
* Function: RemoveInsightIntentExecutor
* SubFunction: NA
*/
HWTEST_F(InsightIntentExecutorTest, JsInsightIntentPageRemoveInsightIntentExecutor_001, TestSize.Level1)
{
    uint64_t intentId = 0;
    auto mgr = std::make_shared<InsightIntentExecutorMgr>();
    mgr->RemoveInsightIntentExecutor(intentId);
    EXPECT_TRUE(mgr->insightIntentExecutors_.empty());
}

/*
* Feature: JsInsightIntentFunc
* Function: TriggerCallbackInner
* SubFunction: NA
*/
HWTEST_F(InsightIntentExecutorTest, JsInsightIntentPageTriggerCallbackInner_001, TestSize.Level1)
{
    auto callback = std::make_unique<InsightIntentExecutorAsyncCallback>();
    EXPECT_NE(callback, nullptr);
    int32_t errCode = 0;
    auto mgr = std::make_shared<InsightIntentExecutorMgr>();
    mgr->TriggerCallbackInner(std::move(callback), errCode);
    EXPECT_EQ(callback, nullptr);
}
} // namespace AbilityRuntime
} // namespace OHOS
