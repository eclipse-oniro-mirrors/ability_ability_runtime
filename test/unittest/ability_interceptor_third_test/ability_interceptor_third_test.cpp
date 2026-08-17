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
    auto resMap = executer->GetInterceptorMapCopy();
    EXPECT_EQ(resMap.size(), 1);
    executer->RemoveInterceptor("CrowdTest");
    auto resMap2 = executer->GetInterceptorMapCopy();
    EXPECT_EQ(resMap2.size(), 0);
}

} // namespace AAFwk
} // namespace OHOS
