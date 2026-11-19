/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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
#include <memory>
#include "appfreeze_state.h"

#define private public
#include "appfreeze_inner.h"
#undef private

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AbilityRuntime {
class AppFreezeStateTest : public testing::Test {
public:
    AppFreezeStateTest()
    {}
    ~AppFreezeStateTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AppFreezeStateTest::SetUpTestCase(void)
{}

void AppFreezeStateTest::TearDownTestCase(void)
{}

void AppFreezeStateTest::SetUp(void)
{}

void AppFreezeStateTest::TearDown(void)
{}

/**
 * @tc.number: AppfreezeStateTest_001
 * @tc.desc: Verify that function SetAppFreezeState and CancelAppFreezeState.
 * @tc.type: FUNC
 */
HWTEST_F(AppFreezeStateTest, AppfreezeStateTest_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "AppfreezeStateTest_001 start";
    uint32_t flag = 1;
    auto appFreezeState = std::make_shared<AbilityRuntime::AppFreezeState> ();
    auto inner = AppfreezeInner::GetInstance();
    appFreezeState->SetAppFreezeState(flag);
    EXPECT_FALSE(inner->IsHandleAppfreeze());

    flag = -1;
    appFreezeState->CancelAppFreezeState(flag);
    EXPECT_TRUE(inner->IsHandleAppfreeze());
    GTEST_LOG_(INFO) << "AppfreezeStateTest_001 end";
}
}  // namespace AbilityRuntime
}  // namespace OHOS
