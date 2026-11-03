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

#include "ability_util.h"
#define private public
#define protected public
#include "interceptor/kiosk_interceptor.h"
#include "kiosk_manager.h"
#undef private
#undef protected
#include "system_ability_definition.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using OHOS::AppExecFwk::AbilityType;
constexpr char KIOSK_WHITE_LIST[] = "KioskWhitelist";

namespace OHOS {
namespace AAFwk {
class KioskInterceptorTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void KioskInterceptorTest::SetUpTestCase() {}

void KioskInterceptorTest::TearDownTestCase() {}

void KioskInterceptorTest::SetUp() {}

void KioskInterceptorTest::TearDown() {}

/*
 * Feature: KioskInterceptor
 * Function: IsInKioskMode
 * SubFunction: NA
 * FunctionPoints: KioskInterceptor IsInKioskMode
 */
HWTEST_F(KioskInterceptorTest, IsInKioskMode_001, TestSize.Level1) {
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = false;
    bool result = KioskManager::GetInstance().IsInKioskModeInner();
    EXPECT_EQ(result, false);

    result = KioskManager::GetInstance().IsInKioskMode();
    EXPECT_EQ(result, false);
}

/*
 * Feature: KioskInterceptor
 * Function: IsInKioskMode
 * SubFunction: NA
 * FunctionPoints: KioskInterceptor IsInKioskMode
 */
HWTEST_F(KioskInterceptorTest, IsInKioskMode_002, TestSize.Level1) {
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = true;
    bool result = KioskManager::GetInstance().IsInKioskModeInner();
    EXPECT_EQ(result, true);

    result = KioskManager::GetInstance().IsInKioskMode();
    EXPECT_EQ(result, true);
}

/*
 * Feature: KioskInterceptor
 * Function: IsInWhiteList
 * SubFunction: NA
 * FunctionPoints: KioskInterceptor IsInWhiteList
 */
HWTEST_F(KioskInterceptorTest, IsInWhiteList_001, TestSize.Level1) {
    std::string bundleName = "com.test.example";
    KioskManager::GetInstance().whitelist_.clear();
    bool result = KioskManager::GetInstance().IsInWhiteListInner(bundleName);
    EXPECT_EQ(result, false);

    result = KioskManager::GetInstance().IsInWhiteList(bundleName);
    EXPECT_EQ(result, false);
}

/*
 * Feature: KioskInterceptor
 * Function: IsInWhiteList
 * SubFunction: NA
 * FunctionPoints: KioskInterceptor IsInWhiteList
 */
HWTEST_F(KioskInterceptorTest, IsInWhiteList_002, TestSize.Level1) {
    std::string bundleName = "com.test.example";
    KioskManager::GetInstance().whitelist_.emplace(bundleName);
    bool result = KioskManager::GetInstance().IsInWhiteListInner(bundleName);
    EXPECT_EQ(result, true);

    result = KioskManager::GetInstance().IsInWhiteList(bundleName);
    EXPECT_EQ(result, true);
}

#ifdef SUPPORT_GRAPHICS
/*
 * Feature: KioskInterceptor
 * Function: KioskInterceptor
 * SubFunction: NA
 * FunctionPoints: KioskInterceptor DoProcess
 */
HWTEST_F(KioskInterceptorTest, KioskInterceptor_001, TestSize.Level1)
{
    auto kioskInterceptor = std::make_shared<KioskInterceptor>();
    Want want;
    want.SetElementName("com.example.test", "MainAbility");
    want.SetParam(AAFwk::SCREEN_MODE_KEY, ScreenMode::IDLE_SCREEN_MODE);
    want.SetAction("com.example.myapplication");
    AbilityInterceptorParam param = InterceptorParamBuilder(want, 0, 0).WithUI(false).CallerToken(nullptr).Build();
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = false;
    int32_t result = kioskInterceptor->DoProcess(param);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: KioskInterceptor
 * Function: KioskInterceptor
 * SubFunction: NA
 * FunctionPoints: KioskInterceptor DoProcess
 */
HWTEST_F(KioskInterceptorTest, KioskInterceptor_002, TestSize.Level1)
{
    auto kioskInterceptor = std::make_shared<KioskInterceptor>();
    Want want;
    want.SetElementName("com.example.test", "MainAbility");
    want.SetParam(AAFwk::SCREEN_MODE_KEY, ScreenMode::IDLE_SCREEN_MODE);
    want.SetAction("com.example.myapplication");
    AbilityInterceptorParam param = InterceptorParamBuilder(want, 0, 0).WithUI(false).CallerToken(nullptr).Build();
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = true;
    KioskManager::GetInstance().whitelist_.clear();
    int32_t result = kioskInterceptor->DoProcess(param);
    EXPECT_EQ(result, ERR_KIOSK_MODE_NOT_IN_WHITELIST);
}

/*
 * Feature: KioskInterceptor
 * Function: KioskInterceptor
 * SubFunction: NA
 * FunctionPoints: KioskInterceptor DoProcess
 */
HWTEST_F(KioskInterceptorTest, KioskInterceptor_003, TestSize.Level1)
{
    auto kioskInterceptor = std::make_shared<KioskInterceptor>();
    Want want;
    std::string bundleName = "com.test.example";
    want.SetElementName(bundleName, "MainAbility");
    want.SetParam(AAFwk::SCREEN_MODE_KEY, ScreenMode::IDLE_SCREEN_MODE);
    want.SetAction("com.example.myapplication");
    AbilityInterceptorParam param = InterceptorParamBuilder(want, 0, 0).WithUI(false).CallerToken(nullptr).Build();
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = true;
    KioskManager::GetInstance().whitelist_.emplace(bundleName);
    int32_t result = kioskInterceptor->DoProcess(param);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: KioskInterceptor
 * Function: KioskInterceptor
 * SubFunction: NA
 * FunctionPoints: KioskInterceptor DoProcess
 */
HWTEST_F(KioskInterceptorTest, KioskInterceptor_004, TestSize.Level1)
{
    auto kioskInterceptor = std::make_shared<KioskInterceptor>();
    Want want;
    std::string bundleName = "";
    want.SetElementName(bundleName, "MainAbility");
    want.SetParam(AAFwk::SCREEN_MODE_KEY, ScreenMode::IDLE_SCREEN_MODE);
    want.SetAction("com.example.myapplication");
    AbilityInterceptorParam param = InterceptorParamBuilder(want, 0, 0).WithUI(false).CallerToken(nullptr).Build();
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = true;
    int32_t result = kioskInterceptor->DoProcess(param);
    EXPECT_EQ(result, ERR_KIOSK_MODE_NOT_IN_WHITELIST);
}

/*
 * Feature: KioskInterceptor
 * Function: KioskInterceptor
 * SubFunction: NA
 * FunctionPoints: KioskInterceptor DoProcess
 */
HWTEST_F(KioskInterceptorTest, KioskInterceptor_005, TestSize.Level1)
{
    auto kioskInterceptor = std::make_shared<KioskInterceptor>();
    Want want;
    std::string bundleName = "com.test.example";
    AbilityInterceptorParam param = InterceptorParamBuilder(want, 0, 0).WithUI(false).CallerToken(nullptr).Build();
    int32_t result = kioskInterceptor->DoProcess(param);
    EXPECT_EQ(result, ERR_OK);
}
#endif
} // namespace AAFwk
} // namespace OHOS
