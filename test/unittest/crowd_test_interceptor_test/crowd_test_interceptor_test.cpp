/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#include "interceptor/crowd_test_interceptor.h"
#undef private
#undef protected

#include "mock_my_status.h"
#include "start_ability_utils.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
class CrowdTestInterceptorTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void CrowdTestInterceptorTest::SetUpTestCase()
{}

void CrowdTestInterceptorTest::TearDownTestCase()
{}

void CrowdTestInterceptorTest::SetUp()
{}

void CrowdTestInterceptorTest::TearDown()
{}

/**
 * @tc.name: CrowdTestInterceptorTest_CheckCrowdtest_001
 * @tc.desc: CheckCrowdtest
 * @tc.type: FUNC
 * @tc.require: CheckCrowdtest
 */
HWTEST_F(CrowdTestInterceptorTest, CheckCrowdtest_001, TestSize.Level1)
{
    CrowdTestInterceptor crowdTestInterceptor;
    Want want;
    int32_t userId = 1001;
    StartAbilityUtils::retGetApplicationInfo = false;
    auto ret = crowdTestInterceptor.CheckCrowdtest(want, userId);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: CrowdTestInterceptorTest_CheckCrowdtest_002
 * @tc.desc: CheckCrowdtest
 * @tc.type: FUNC
 * @tc.require: CheckCrowdtest
 */
HWTEST_F(CrowdTestInterceptorTest, CheckCrowdtest_002, TestSize.Level1)
{
    CrowdTestInterceptor crowdTestInterceptor;
    Want want;
    int32_t userId = 1001;
    StartAbilityUtils::retGetApplicationInfo = true;
    StartAbilityUtils::applicationInfo.crowdtestDeadline = 0;
    auto ret = crowdTestInterceptor.CheckCrowdtest(want, userId);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: CrowdTestInterceptorTest_DoProcess_001
 * @tc.desc: DoProcess
 * @tc.type: FUNC
 * @tc.require: DoProcess
 */
HWTEST_F(CrowdTestInterceptorTest, DoProcess_001, TestSize.Level1)
{
    CrowdTestInterceptor crowdTestInterceptor;
    Want want;
    int requestCode = 0;
    int userId = 100;
    AbilityInterceptorParam param =
        InterceptorParamBuilder(want, requestCode, userId).WithUI(false).CallerToken(nullptr).Build();
    StartAbilityUtils::skipCrowTest = true;
    auto ret = crowdTestInterceptor.DoProcess(param);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: CrowdTestInterceptorTest_DoProcess_002
 * @tc.desc: DoProcess
 * @tc.type: FUNC
 * @tc.require: DoProcess
 */
HWTEST_F(CrowdTestInterceptorTest, DoProcess_002, TestSize.Level1)
{
    CrowdTestInterceptor crowdTestInterceptor;
    Want want;
    int requestCode = 0;
    int userId = 100;
    AbilityInterceptorParam param =
        InterceptorParamBuilder(want, requestCode, userId).WithUI(false).CallerToken(nullptr).Build();

    StartAbilityUtils::skipCrowTest = false;
    StartAbilityUtils::retGetApplicationInfo = false;
    auto ret = crowdTestInterceptor.DoProcess(param);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: CrowdTestInterceptorTest_DoProcess_003
 * @tc.desc: DoProcess
 * @tc.type: FUNC
 * @tc.require: DoProcess
 */
HWTEST_F(CrowdTestInterceptorTest, DoProcess_003, TestSize.Level1)
{
    CrowdTestInterceptor crowdTestInterceptor;
    Want want;
    int requestCode = 0;
    int userId = 100;
    bool isWithUI = true;
    AbilityInterceptorParam param =
        InterceptorParamBuilder(want, requestCode, userId).WithUI(isWithUI).CallerToken(nullptr).Build();

    StartAbilityUtils::skipCrowTest = false;
    StartAbilityUtils::retGetApplicationInfo = true;
    StartAbilityUtils::applicationInfo.crowdtestDeadline = 1;

    auto ret = crowdTestInterceptor.DoProcess(param);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: CrowdTestInterceptorTest_DoProcess_004
 * @tc.desc: Test the DoProcess function when GetBundleManagerHelper function returns a null pointer
 * @tc.type: FUNC
 * @tc.require: DoProcess
 */
HWTEST_F(CrowdTestInterceptorTest, DoProcess_004, TestSize.Level1)
{
    CrowdTestInterceptor crowdTestInterceptor;
    Want want;
    int requestCode = 0;
    int userId = 100;
    bool isWithUI = true;
    AbilityInterceptorParam param =
        InterceptorParamBuilder(want, requestCode, userId).WithUI(isWithUI).CallerToken(nullptr).Build();

    StartAbilityUtils::skipCrowTest = false;
    StartAbilityUtils::retGetApplicationInfo = true;
    StartAbilityUtils::applicationInfo.crowdtestDeadline = 1;

    AAFwk::MyStatus::GetInstance().isNullPtr = true;
    auto ret = crowdTestInterceptor.DoProcess(param);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: CrowdTestInterceptorTest_DoProcess_005
 * @tc.desc: Test the DoProcess function when GetBundleManagerHelper function
 * returns a non-null pointer and QueryAppGalleryBundleName function returns
 * true
 * @tc.type: FUNC
 * @tc.require: DoProcess
 */
HWTEST_F(CrowdTestInterceptorTest, DoProcess_005, TestSize.Level1)
{
    CrowdTestInterceptor crowdTestInterceptor;
    Want want;
    int requestCode = 0;
    int userId = 100;
    bool isWithUI = true;
    AbilityInterceptorParam param =
        InterceptorParamBuilder(want, requestCode, userId).WithUI(isWithUI).CallerToken(nullptr).Build();

    StartAbilityUtils::skipCrowTest = false;
    StartAbilityUtils::retGetApplicationInfo = true;
    StartAbilityUtils::applicationInfo.crowdtestDeadline = 1;

    AAFwk::MyStatus::GetInstance().isNullPtr = false;
    AAFwk::MyStatus::GetInstance().isFalse = true;
    auto ret = crowdTestInterceptor.DoProcess(param);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: CrowdTestInterceptorTest_DoProcess_006
 * @tc.desc: Test the DoProcess function when GetBundleManagerHelper function
 * returns a non-null pointer and QueryAppGalleryBundleName function returns
 * false
 * @tc.type: FUNC
 * @tc.require: DoProcess
 */
HWTEST_F(CrowdTestInterceptorTest, DoProcess_006, TestSize.Level1)
{
    CrowdTestInterceptor crowdTestInterceptor;
    Want want;
    int requestCode = 0;
    int userId = 100;
    bool isWithUI = true;
    AbilityInterceptorParam param =
        InterceptorParamBuilder(want, requestCode, userId).WithUI(isWithUI).CallerToken(nullptr).Build();

    StartAbilityUtils::skipCrowTest = false;
    StartAbilityUtils::retGetApplicationInfo = true;
    StartAbilityUtils::applicationInfo.crowdtestDeadline = 1;

    AAFwk::MyStatus::GetInstance().isNullPtr = false;
    AAFwk::MyStatus::GetInstance().isFalse = false;
    auto ret = crowdTestInterceptor.DoProcess(param);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: CrowdTestInterceptorTest_DoProcess_007
 * @tc.desc: Test the DoProcess function when CreateModalUIExtension function returns false
 * @tc.type: FUNC
 * @tc.require: DoProcess
 */
HWTEST_F(CrowdTestInterceptorTest, DoProcess_007, TestSize.Level1)
{
    CrowdTestInterceptor crowdTestInterceptor;
    Want want;
    int requestCode = 0;
    int userId = 100;
    bool isWithUI = true;
    AbilityInterceptorParam param =
        InterceptorParamBuilder(want, requestCode, userId).WithUI(isWithUI).CallerToken(nullptr).Build();

    StartAbilityUtils::skipCrowTest = false;
    StartAbilityUtils::retGetApplicationInfo = true;
    StartAbilityUtils::applicationInfo.crowdtestDeadline = 1;

    AAFwk::MyStatus::GetInstance().isNullPtr = false;
    AAFwk::MyStatus::GetInstance().isFalse = false;
    AAFwk::MyStatus::GetInstance().retCreateModalUIExtension = false;
    auto ret = crowdTestInterceptor.DoProcess(param);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: CrowdTestInterceptorTest_DoProcess_008
 * @tc.desc: Test the DoProcess function when the target UIExtension is not found.
 * @tc.type: FUNC
 * @tc.require: DoProcess
 */
HWTEST_F(CrowdTestInterceptorTest, DoProcess_008, TestSize.Level1)
{
    CrowdTestInterceptor crowdTestInterceptor;
    Want want;
    int requestCode = 0;
    int userId = 100;
    bool isWithUI = true;
    AbilityInterceptorParam param =
        InterceptorParamBuilder(want, requestCode, userId).WithUI(isWithUI).CallerToken(nullptr).Build();

    StartAbilityUtils::skipCrowTest = false;
    StartAbilityUtils::retGetApplicationInfo = true;
    StartAbilityUtils::applicationInfo.crowdtestDeadline = 1;

    AAFwk::MyStatus::GetInstance().isNullPtr = false;
    AAFwk::MyStatus::GetInstance().isFalse = false;
    AAFwk::MyStatus::GetInstance().retCreateModalUIExtension = true;
    AAFwk::MyStatus::GetInstance().queryExtensionAbilityInfos = false;
    auto ret = crowdTestInterceptor.DoProcess(param);
    EXPECT_NE(ret, ERR_OK);
}
} // namespace AAFwk
} // namespace OHOS
