/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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
#define private public
#define protected public
#include "ability_interceptor_executer.h"
#undef private
#undef protected

#include "interceptor/crowd_test_interceptor.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {

// Test interceptor used to validate the non-const-ref read-back contract:
// DoProcess mutates param.want in place; the caller must observe the change.
class WantMutatingInterceptor : public IAbilityInterceptor {
public:
    ErrCode DoProcess(AbilityInterceptorParam &param) override
    {
        param.want.SetParam("readBack", std::string("ok"));
        return ERR_OK;
    }
};

class AbilityInterceptorThirdTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

public:
};

void AbilityInterceptorThirdTest::SetUpTestCase()
{}

void AbilityInterceptorThirdTest::TearDownTestCase()
{}

void AbilityInterceptorThirdTest::SetUp()
{}

void AbilityInterceptorThirdTest::TearDown()
{}

/**
 * @tc.name: AbilityInterceptorThirdTest_RemoveInterceptor_001
 * @tc.desc: CrowdTestInterceptor
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(AbilityInterceptorThirdTest, RemoveInterceptor_001, TestSize.Level1)
{
    std::shared_ptr<AbilityInterceptorExecuter> executer = std::make_shared<AbilityInterceptorExecuter>();
    executer->AddInterceptor("CrowdTest", std::make_shared<CrowdTestInterceptor>());
    auto resMap = executer->GetInterceptorListCopy();
    EXPECT_EQ(resMap.size(), 1);
    executer->RemoveInterceptor("CrowdTest");
    auto resMap2 = executer->GetInterceptorListCopy();
    EXPECT_EQ(resMap2.size(), 0);
}

/**
 * @tc.name: AbilityInterceptorThirdTest_InterceptorListOrder_001
 * @tc.desc: The merged executer preserves interceptor insertion order (push_back).
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(AbilityInterceptorThirdTest, InterceptorListOrder_001, TestSize.Level1)
{
    auto executer = std::make_shared<AbilityInterceptorExecuter>();
    executer->AddInterceptor("First", std::make_shared<CrowdTestInterceptor>());
    executer->AddInterceptor("Second", std::make_shared<CrowdTestInterceptor>());
    executer->AddInterceptor("Third", std::make_shared<CrowdTestInterceptor>());
    auto list = executer->GetInterceptorListCopy();
    ASSERT_EQ(list.size(), 3u);
    EXPECT_EQ(list[0].first, "First");
    EXPECT_EQ(list[1].first, "Second");
    EXPECT_EQ(list[2].first, "Third");
}

/**
 * @tc.name: AbilityInterceptorThirdTest_Builder_IndependentFields_001
 * @tc.desc: isWithUI and isVisible are independent fields (different semantics).
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(AbilityInterceptorThirdTest, Builder_IndependentFields_001, TestSize.Level1)
{
    Want want;
    auto param = InterceptorParamBuilder(want, 0, 100).WithUI(true).Visible(false).Build();
    EXPECT_TRUE(param.isWithUI);
    EXPECT_FALSE(param.isVisible);
    auto param2 = InterceptorParamBuilder(want, 0, 100).WithUI(false).Visible(true).Build();
    EXPECT_FALSE(param2.isWithUI);
    EXPECT_TRUE(param2.isVisible);
}

/**
 * @tc.name: AbilityInterceptorThirdTest_DoProcess_ReadBackContract_001
 * @tc.desc: A registered interceptor mutates param.want in place; the executer
 *           caller observes the mutation (non-const-ref read-back is load-bearing).
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(AbilityInterceptorThirdTest, DoProcess_ReadBackContract_001, TestSize.Level1)
{
    auto executer = std::make_shared<AbilityInterceptorExecuter>();
    executer->AddInterceptor("Mutator", std::make_shared<WantMutatingInterceptor>());
    Want want;
    AbilityInterceptorParam param = InterceptorParamBuilder(want, 0, 100).Build();
    EXPECT_EQ(executer->DoProcess(param), ERR_OK);
    EXPECT_EQ(param.want.GetStringParam("readBack"), "ok");
}

} // namespace AAFwk
} // namespace OHOS
