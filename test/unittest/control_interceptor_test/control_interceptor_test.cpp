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
#include "appexecfwk_errors.h"
#include "hilog_tag_wrapper.h"
#define private public
#define protected public
#include "interceptor/control_interceptor.h"
#undef private
#undef protected
#include "mock_app_control_manager.h"
#include "mock_my_flag.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
namespace {
const std::string IS_FROM_PARENTCONTROL = "ohos.ability.isFromParentControl";
}

class ControlInterceptorTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void ControlInterceptorTest::SetUpTestCase()
{}

void ControlInterceptorTest::TearDownTestCase()
{}

void ControlInterceptorTest::SetUp()
{}

void ControlInterceptorTest::TearDown()
{}

/**
 * @tc.name: ControlInterceptorTest_CheckControl_001
 * @tc.desc: CheckControl
 * @tc.type: FUNC
 * @tc.require: CheckControl
 */
HWTEST_F(ControlInterceptorTest, CheckControl_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckControl_001 start");
    MyFlag::bundleMgrHelper_ = nullptr;
    ControlInterceptor interceptor;
    Want want;
    int32_t userId = 1001;
    AppExecFwk::AppRunningControlRuleResult controlRule;
    auto ret = interceptor.CheckControl(want, userId, controlRule);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "CheckControl_001 end");
}

/**
 * @tc.name: ControlInterceptorTest_CheckControl_002
 * @tc.desc: CheckControl
 * @tc.type: FUNC
 * @tc.require: CheckControl
 */
HWTEST_F(ControlInterceptorTest, CheckControl_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckControl_002 start");
    MyFlag::bundleMgrHelper_ = AppExecFwk::BundleMgrHelper::GetInstance();
    MyFlag::mockAppControlManager_ = nullptr;
    ControlInterceptor interceptor;
    Want want;
    int32_t userId = 1001;
    AppExecFwk::AppRunningControlRuleResult controlRule;
    auto ret = interceptor.CheckControl(want, userId, controlRule);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "CheckControl_002 end");
}

/**
 * @tc.name: ControlInterceptorTest_CheckControl_003
 * @tc.desc: CheckControl
 * @tc.type: FUNC
 * @tc.require: CheckControl
 */
HWTEST_F(ControlInterceptorTest, CheckControl_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckControl_003 start");
    MyFlag::bundleMgrHelper_ = AppExecFwk::BundleMgrHelper::GetInstance();
    auto appControlMgr = new (std::nothrow) AppControlProxy(nullptr);
    MyFlag::mockAppControlManager_ = appControlMgr;
    MyFlag::retGetAppRunningControlRule_ = ERR_BUNDLE_MANAGER_PERMISSION_DENIED;

    ControlInterceptor interceptor;
    Want want;
    int32_t userId = 1001;
    AppExecFwk::AppRunningControlRuleResult controlRule;
    auto ret = interceptor.CheckControl(want, userId, controlRule);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "CheckControl_003 end");
}

/**
 * @tc.name: ControlInterceptorTest_CheckControl_004
 * @tc.desc: CheckControl
 * @tc.type: FUNC
 * @tc.require: CheckControl
 */
HWTEST_F(ControlInterceptorTest, CheckControl_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckControl_004 start");
    MyFlag::bundleMgrHelper_ = AppExecFwk::BundleMgrHelper::GetInstance();
    auto appControlMgr = new (std::nothrow) AppControlProxy(nullptr);
    MyFlag::mockAppControlManager_ = appControlMgr;
    MyFlag::retGetAppRunningControlRule_ = ERR_OK;

    ControlInterceptor interceptor;
    Want want;
    int32_t userId = 1001;
    AppExecFwk::AppRunningControlRuleResult controlRule;
    auto ret = interceptor.CheckControl(want, userId, controlRule);
    EXPECT_TRUE(ret);
    TAG_LOGI(AAFwkTag::TEST, "CheckControl_004 end");
}

#ifdef SUPPORT_GRAPHICS
/**
 * @tc.name: ControlInterceptorTest_DoProcess_001
 * @tc.desc: DoProcess
 * @tc.type: FUNC
 * @tc.require: DoProcess
 */
HWTEST_F(ControlInterceptorTest, DoProcess_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DoProcess_001 start");
    MyFlag::bundleMgrHelper_ = AppExecFwk::BundleMgrHelper::GetInstance();
    auto appControlMgr = new (std::nothrow) AppControlProxy(nullptr);
    MyFlag::mockAppControlManager_ = appControlMgr;
    MyFlag::retGetAppRunningControlRule_ = ERR_OK;
    MyFlag::edmCode_ = -1;

    ControlInterceptor interceptor;
    Want want;
    int requestCode = 0;
    int userId = 100;
    bool isWithUI = false;
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param(want, requestCode, userId, isWithUI, nullptr, shouldBlockFunc);
    auto ret = interceptor.DoProcess(param);
    EXPECT_EQ(ret, -1);
    TAG_LOGI(AAFwkTag::TEST, "DoProcess_001 end");
}

/**
 * @tc.name: ControlInterceptorTest_DoProcess_002
 * @tc.desc: DoProcess
 * @tc.type: FUNC
 * @tc.require: DoProcess
 */
HWTEST_F(ControlInterceptorTest, DoProcess_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DoProcess_002 start");
    MyFlag::bundleMgrHelper_ = AppExecFwk::BundleMgrHelper::GetInstance();
    auto appControlMgr = new (std::nothrow) AppControlProxy(nullptr);
    MyFlag::mockAppControlManager_ = appControlMgr;
    MyFlag::retGetAppRunningControlRule_ = ERR_OK;
    MyFlag::edmCode_ = -1;

    ControlInterceptor interceptor;
    Want want;
    int requestCode = 0;
    int userId = 100;
    bool isWithUI = true;
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param(want, requestCode, userId, isWithUI, nullptr, shouldBlockFunc);
    auto ret = interceptor.DoProcess(param);
    EXPECT_EQ(ret, -1);
    TAG_LOGI(AAFwkTag::TEST, "DoProcess_002 end");
}

/**
 * @tc.name: ControlInterceptorTest_DoProcess_003
 * @tc.desc: DoProcess
 * @tc.type: FUNC
 * @tc.require: DoProcess
 */
HWTEST_F(ControlInterceptorTest, DoProcess_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DoProcess_003 start");
    MyFlag::bundleMgrHelper_ = AppExecFwk::BundleMgrHelper::GetInstance();
    auto appControlMgr = new (std::nothrow) AppControlProxy(nullptr);
    MyFlag::mockAppControlManager_ = appControlMgr;
    MyFlag::retGetAppRunningControlRule_ = ERR_OK;
    MyFlag::appRunningControlRuleResult_.controlWant = std::make_shared<Want>();
    MyFlag::appRunningControlRuleResult_.controlWant->SetParam(IS_FROM_PARENTCONTROL, true);
    MyFlag::startAbilityRet_ = -2;

    ControlInterceptor interceptor;
    Want want;
    want.SetElementName("", "bundle", "ability", "module");
    int requestCode = 0;
    int userId = 100;
    bool isWithUI = true;
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param(want, requestCode, userId, isWithUI, nullptr, shouldBlockFunc);
    auto ret = interceptor.DoProcess(param);
    EXPECT_EQ(ret, -2);
    TAG_LOGI(AAFwkTag::TEST, "DoProcess_003 end");
}

/**
 * @tc.name: ControlInterceptorTest_DoProcess_004
 * @tc.desc: DoProcess
 * @tc.type: FUNC
 * @tc.require: DoProcess
 */
HWTEST_F(ControlInterceptorTest, DoProcess_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DoProcess_004 start");
    MyFlag::bundleMgrHelper_ = AppExecFwk::BundleMgrHelper::GetInstance();
    auto appControlMgr = new (std::nothrow) AppControlProxy(nullptr);
    MyFlag::mockAppControlManager_ = appControlMgr;
    MyFlag::retGetAppRunningControlRule_ = ERR_OK;
    MyFlag::appRunningControlRuleResult_.controlWant = std::make_shared<Want>();
    MyFlag::appRunningControlRuleResult_.controlWant->SetParam(IS_FROM_PARENTCONTROL, true);
    MyFlag::startAbilityRet_ = ERR_OK;
    MyFlag::edmCode_ = -3;

    ControlInterceptor interceptor;
    Want want;
    want.SetElementName("", "bundle", "ability", "module");
    int requestCode = 0;
    int userId = 100;
    bool isWithUI = true;
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param(want, requestCode, userId, isWithUI, nullptr, shouldBlockFunc);
    auto ret = interceptor.DoProcess(param);
    EXPECT_EQ(ret, -3);
    TAG_LOGI(AAFwkTag::TEST, "DoProcess_004 end");
}

/**
 * @tc.name: ControlInterceptorTest_DoProcess_005
 * @tc.desc: DoProcess
 * @tc.type: FUNC
 * @tc.require: DoProcess
 */
HWTEST_F(ControlInterceptorTest, DoProcess_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DoProcess_005 start");
    MyFlag::bundleMgrHelper_ = AppExecFwk::BundleMgrHelper::GetInstance();
    auto appControlMgr = new (std::nothrow) AppControlProxy(nullptr);
    MyFlag::mockAppControlManager_ = appControlMgr;
    MyFlag::retGetAppRunningControlRule_ = ERR_OK;
    MyFlag::appRunningControlRuleResult_.controlWant = std::make_shared<Want>();
    MyFlag::appRunningControlRuleResult_.controlWant->SetParam(IS_FROM_PARENTCONTROL, true);
    MyFlag::startAbilityRet_ = ERR_OK;
    MyFlag::edmCode_ = ERR_OK;

    ControlInterceptor interceptor;
    Want want;
    want.SetElementName("", "bundle", "ability", "module");
    int requestCode = 0;
    int userId = 100;
    bool isWithUI = true;
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param(want, requestCode, userId, isWithUI, nullptr, shouldBlockFunc);
    auto ret = interceptor.DoProcess(param);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "DoProcess_005 end");
}

/**
 * @tc.name: ControlInterceptorTest_DoProcess_006
 * @tc.desc: DoProcess
 * @tc.type: FUNC
 * @tc.require: DoProcess
 */
HWTEST_F(ControlInterceptorTest, DoProcess_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DoProcess_006 start");
    MyFlag::bundleMgrHelper_ = nullptr;

    ControlInterceptor interceptor;
    Want want;
    int requestCode = 0;
    int userId = 100;
    bool isWithUI = true;
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param(want, requestCode, userId, isWithUI, nullptr, shouldBlockFunc);
    auto ret = interceptor.DoProcess(param);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "DoProcess_006 end");
}

/**
 * @tc.name: ControlInterceptorTest_DoProcess_007
 * @tc.desc: DoProcess
 * @tc.type: FUNC
 * @tc.require: DoProcess
 */
HWTEST_F(ControlInterceptorTest, DoProcess_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DoProcess_007 start");
    MyFlag::bundleMgrHelper_ = AppExecFwk::BundleMgrHelper::GetInstance();
    MyFlag::mockAppControlManager_ = nullptr;

    ControlInterceptor interceptor;
    Want want;
    int requestCode = 0;
    int userId = 100;
    bool isWithUI = true;
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param(want, requestCode, userId, isWithUI, nullptr, shouldBlockFunc);
    auto ret = interceptor.DoProcess(param);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "DoProcess_007 end");
}

/**
 * @tc.name: ControlInterceptorTest_DoProcess_008
 * @tc.desc: DoProcess
 * @tc.type: FUNC
 * @tc.require: DoProcess
 */
HWTEST_F(ControlInterceptorTest, DoProcess_008, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DoProcess_008 start");
    MyFlag::bundleMgrHelper_ = AppExecFwk::BundleMgrHelper::GetInstance();
    auto appControlMgr = new (std::nothrow) AppControlProxy(nullptr);
    MyFlag::mockAppControlManager_ = appControlMgr;
    MyFlag::retGetAppRunningControlRule_ = ERR_BUNDLE_MANAGER_PERMISSION_DENIED;

    ControlInterceptor interceptor;
    Want want;
    int requestCode = 0;
    int userId = 100;
    bool isWithUI = true;
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param(want, requestCode, userId, isWithUI, nullptr, shouldBlockFunc);
    auto ret = interceptor.DoProcess(param);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "DoProcess_008 end");
}
#endif
} // namespace AAFwk
} // namespace OHOS
