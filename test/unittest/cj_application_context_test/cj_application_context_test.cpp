/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#define private public
#include "cj_ability_delegator.h"
#include "cj_ability_lifecycle_callback.h"
#include "cj_ability_runtime_error.h"
#include "cj_application_context.h"
#include "ability_delegator_registry.h"
#include "application_context.h"
#include "cj_utils_ffi.h"
#include "window_stage_impl.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::FFI;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace ApplicationContextCJ {

// Mock类
class MockCjAbilityLifecycleCallback : public CjAbilityLifecycleCallbackImpl {
public:
    MOCK_METHOD(void, OnAbilityCreate, (const int64_t &ability));
    MOCK_METHOD(void, OnWindowStageCreate, (const int64_t &ability, WindowStagePtr windowStage));
    MOCK_METHOD(void, OnWindowStageActive, (const int64_t &ability, WindowStagePtr windowStage));
    MOCK_METHOD(void, OnWindowStageInactive, (const int64_t &ability, WindowStagePtr windowStage));
    MOCK_METHOD(void, OnWindowStageDestroy, (const int64_t &ability, WindowStagePtr windowStage));
    MOCK_METHOD(void, OnAbilityDestroy, (const int64_t &ability));
    MOCK_METHOD(void, OnAbilityForeground, (const int64_t &ability));
    MOCK_METHOD(void, OnAbilityBackground, (const int64_t &ability));
    MOCK_METHOD(void, OnAbilityContinue, (const int64_t &ability));

    MOCK_METHOD(void, OnAbilityWillCreate, (const int64_t &ability));
    MOCK_METHOD(void, OnWindowStageWillCreate, (const int64_t &ability, WindowStagePtr windowStage));
    MOCK_METHOD(void, OnWindowStageWillDestroy, (const int64_t &ability, WindowStagePtr windowStage));
    MOCK_METHOD(void, OnAbilityWillDestroy, (const int64_t &ability));
    MOCK_METHOD(void, OnAbilityWillForeground, (const int64_t &ability));
    MOCK_METHOD(void, OnAbilityWillBackground, (const int64_t &ability));
    MOCK_METHOD(void, OnNewWant, (const int64_t &ability));
    MOCK_METHOD(void, OnWillNewWant, (const int64_t &ability));
    MOCK_METHOD(void, OnAbilityWillContinue, (const int64_t &ability));
    MOCK_METHOD(void, OnWindowStageWillRestore, (const int64_t &ability, WindowStagePtr windowStage));
    MOCK_METHOD(void, OnWindowStageRestore, (const int64_t &ability, WindowStagePtr windowStage));
    MOCK_METHOD(void, OnAbilityWillSaveState, (const int64_t &ability));
    MOCK_METHOD(void, OnAbilitySaveState, (const int64_t &ability));
};

class CjApplicationContextTest : public testing::Test {
public:
    CjApplicationContextTest()
    {}
    ~CjApplicationContextTest()
    {}
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void CjApplicationContextTest::SetUpTestCase() {}

void CjApplicationContextTest::TearDownTestCase() {}

void CjApplicationContextTest::SetUp()
{
    // Create an CJApplicationContext object
    CJApplicationContext::GetInstance()->OnSetFont("");
    CJApplicationContext::GetInstance()->OnSetLanguage("ZH-CN");
    CJApplicationContext::GetInstance()->OnSetColorMode(0);
}

void CjApplicationContextTest::TearDown() {}

/**
 * @tc.name: CJApplicationContextTestGetArea_001
 * @tc.desc: CjApplicationContextTest test for GetArea.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestGetArea_001, TestSize.Level1)
{
    // 测试 GetArea 函数
    int area = CJApplicationContext::GetInstance()->GetArea();
    EXPECT_EQ(area, 1);
}

/**
 * @tc.name: CJApplicationContextTestGetApplicationInfo_001
 * @tc.desc: CjApplicationContextTest test for GetApplicationInfo.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestGetApplicationInfo_001, TestSize.Level1)
{
    EXPECT_TRUE(CJApplicationContext::GetInstance()->GetApplicationInfo() == nullptr);
    EXPECT_TRUE(CJApplicationContext::GetInstance()->GetApplicationContext() != nullptr);
}

/**
 * @tc.name: CJApplicationContextTestIsA
 * bilityLifecycleCallbackEmpty_001
 * @tc.desc: CjApplicationContextTest test for IsAbilityLifecycleCallbackEmpty.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestIsAbilityLifecycleCallbackEmpty_001, TestSize.Level1)
{
    bool result = CJApplicationContext::GetInstance()->IsAbilityLifecycleCallbackEmpty();
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: CJApplicationContextTestOnOnEnvironment_001
 * @tc.desc: CjApplicationContextTest test for OnOnEnvironment.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestOnOnEnvironment_001, TestSize.Level1)
{
    auto cfgCallback = [](AbilityRuntime::CConfiguration) {};
    auto memCallback = [](int32_t) {};
    auto ret =
        CJApplicationContext::GetInstance()->OnOnEnvironment(cfgCallback, memCallback, true, nullptr);
    EXPECT_EQ(ret, -1);

    CJApplicationContext::GetInstance()->envCallback_ = nullptr;
    ret = CJApplicationContext::GetInstance()->OnOnEnvironment(cfgCallback, memCallback, true, nullptr);
    EXPECT_EQ(ret, -1);
}

/**
 * @tc.name: CJApplicationContextTestOnOnAbilityLifecycle_001
 * @tc.desc: CjApplicationContextTest test for OnOnAbilityLifecycle.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestOnOnAbilityLifecycle_001, TestSize.Level1)
{
    CArrI64 cFuncIds;
    bool isSync = true;
    int32_t err = 0;
    int32_t *errCode = &err;
    CJApplicationContext::GetInstance()->callback_ =
        std::make_shared<AbilityRuntime::CjAbilityLifecycleCallbackImpl>();
    EXPECT_EQ(CJApplicationContext::GetInstance()->OnOnAbilityLifecycle(cFuncIds, isSync, errCode), -1);
}

/**
 * @tc.name: CJApplicationContextTestOnOnApplicationStateChange_001
 * @tc.desc: CjApplicationContextTest test for OnOnApplicationStateChange.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestOnOnApplicationStateChange_001, TestSize.Level1)
{
    auto foregroundCallback = []() {};
    auto backgroundCallback = []() {};
    int32_t err = 0;
    int32_t *errCode = &err;
    auto ret = CJApplicationContext::GetInstance()->OnOnApplicationStateChange(foregroundCallback,
        backgroundCallback, errCode);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: CJApplicationContextTestOnOffEnvironment_001
 * @tc.desc: CjApplicationContextTest test for OnOffEnvironment.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestOnOffEnvironment_001, TestSize.Level1)
{
    int32_t callbackId = 10;
    int32_t err = 0;
    int32_t *errCode = &err;
    CJApplicationContext::GetInstance()->envCallback_ = std::make_shared<CjEnvironmentCallback>();
    CJApplicationContext::GetInstance()->OnOffEnvironment(callbackId, errCode);
    EXPECT_EQ(*errCode, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
}

/**
 * @tc.name: CJApplicationContextTestOnOffAbilityLifecycle_001
 * @tc.desc: CjApplicationContextTest test for OnOffAbilityLifecycle.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestOnOffAbilityLifecycle_001, TestSize.Level1)
{
    int32_t callbackId = 10;
    int32_t err = 0;
    int32_t *errCode = &err;
    CJApplicationContext::GetInstance()->callback_ =
        std::make_shared<AbilityRuntime::CjAbilityLifecycleCallbackImpl>();
    CJApplicationContext::GetInstance()->OnOffAbilityLifecycle(callbackId, errCode);
    EXPECT_EQ(*errCode, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);

    CJApplicationContext::GetInstance()->applicationContext_ =
        std::make_shared<AbilityRuntime::ApplicationContext>();
    CJApplicationContext::GetInstance()->OnOffAbilityLifecycle(callbackId, errCode);
    EXPECT_EQ(*errCode, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);

    CJApplicationContext::GetInstance()->callback_ = nullptr;
    CJApplicationContext::GetInstance()->OnOffAbilityLifecycle(callbackId, errCode);
    EXPECT_EQ(*errCode, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
}

/**
 * @tc.name: CJApplicationContextTestOnOffApplicationStateChange_001
 * @tc.desc: CjApplicationContextTest test for OnOffApplicationStateChange.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestOnOffApplicationStateChange_001, TestSize.Level1)
{
    int32_t callbackId = 10;
    int32_t err = 0;
    int32_t *errCode = &err;
    CJApplicationContext::GetInstance()->OnOffApplicationStateChange(callbackId, errCode);
    EXPECT_EQ(*errCode, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);

    CJApplicationContext::GetInstance()->applicationContext_ =
        std::make_shared<AbilityRuntime::ApplicationContext>();
    CJApplicationContext::GetInstance()->applicationStateCallback_ =
        std::make_shared<CjApplicationStateChangeCallback>();
    CJApplicationContext::GetInstance()->OnOffApplicationStateChange(callbackId, errCode);
    EXPECT_EQ(*errCode, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);

    CJApplicationContext::GetInstance()->applicationStateCallback_ = nullptr;
    CJApplicationContext::GetInstance()->OnOffApplicationStateChange(callbackId, errCode);
    EXPECT_EQ(*errCode, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
}

/**
 * @tc.name: CJApplicationContextTestDispatchOnAbilityCreate_001
 * @tc.desc: CjApplicationContextTest test for DispatchOnAbilityCreate.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestDispatchOnAbilityCreate_001, TestSize.Level1)
{
    std::shared_ptr<CjAbilityLifecycleCallbackImpl> mock;
    CJApplicationContext::GetInstance()->RegisterAbilityLifecycleCallback(mock);
    CJApplicationContext::GetInstance()->DispatchOnAbilityCreate(10);
    CJApplicationContext::GetInstance()->UnregisterAbilityLifecycleCallback(mock);
    EXPECT_TRUE(CJApplicationContext::callbacks_.empty());
}

/**
 * @tc.name: CJApplicationContextTestDispatchOnWindowStageCreate_001
 * @tc.desc: CjApplicationContextTest test for DispatchOnWindowStageCreate.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestDispatchOnWindowStageCreate_001, TestSize.Level1)
{
    std::shared_ptr<CjAbilityLifecycleCallbackImpl> mock;
    CJApplicationContext::GetInstance()->RegisterAbilityLifecycleCallback(mock);
    auto win = std::make_shared<Rosen::WindowScene>();
    auto winStage = new Rosen::CJWindowStageImpl(win);
    WindowStagePtr windowStage = reinterpret_cast<WindowStagePtr>(winStage);
    CJApplicationContext::GetInstance()->DispatchOnWindowStageCreate(10, windowStage);
    CJApplicationContext::GetInstance()->UnregisterAbilityLifecycleCallback(mock);
    EXPECT_TRUE(CJApplicationContext::callbacks_.empty());
    delete winStage;
}

/**
 * @tc.name: CJApplicationContextTestDispatchWindowStageFocus_001
 * @tc.desc: CjApplicationContextTest test for DispatchWindowStageFocus.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestDispatchWindowStageFocus_001, TestSize.Level1)
{
    std::shared_ptr<CjAbilityLifecycleCallbackImpl> mock;
    CJApplicationContext::GetInstance()->RegisterAbilityLifecycleCallback(mock);
    auto win = std::make_shared<Rosen::WindowScene>();
    auto winStage = new Rosen::CJWindowStageImpl(win);
    WindowStagePtr windowStage = reinterpret_cast<WindowStagePtr>(winStage);
    CJApplicationContext::GetInstance()->DispatchWindowStageFocus(10, windowStage);
    CJApplicationContext::GetInstance()->DispatchWindowStageUnfocus(10, windowStage);
    CJApplicationContext::GetInstance()->DispatchOnWindowStageDestroy(10, windowStage);
    CJApplicationContext::GetInstance()->UnregisterAbilityLifecycleCallback(mock);
    delete winStage;
    EXPECT_TRUE(CJApplicationContext::callbacks_.empty());
}

/**
 * @tc.name: CJApplicationContextTestDispatchOnWindowStageDestroy_001
 * @tc.desc: CjApplicationContextTest test for DispatchOnWindowStageDestroy.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTest_Ability_001, TestSize.Level1)
{
    std::shared_ptr<CjAbilityLifecycleCallbackImpl> mock;
    CJApplicationContext::GetInstance()->RegisterAbilityLifecycleCallback(mock);
    CJApplicationContext::GetInstance()->DispatchOnAbilityDestroy(10);
    CJApplicationContext::GetInstance()->DispatchOnAbilityForeground(10);
    CJApplicationContext::GetInstance()->DispatchOnAbilityBackground(10);
    CJApplicationContext::GetInstance()->DispatchOnAbilityContinue(10);
    CJApplicationContext::GetInstance()->DispatchOnAbilityWillCreate(10);
    CJApplicationContext::GetInstance()->UnregisterAbilityLifecycleCallback(mock);
    EXPECT_TRUE(CJApplicationContext::callbacks_.empty());
}

/**
 * @tc.name: CJApplicationContextTest_Ability_002
 * @tc.desc: CjApplicationContextTest test for Ability.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTest_Ability_002, TestSize.Level1)
{
    std::shared_ptr<CjAbilityLifecycleCallbackImpl> mock;
    CJApplicationContext::GetInstance()->RegisterAbilityLifecycleCallback(mock);
    auto win = std::make_shared<Rosen::WindowScene>();
    auto winStage = new Rosen::CJWindowStageImpl(win);
    WindowStagePtr windowStage = reinterpret_cast<WindowStagePtr>(winStage);
    CJApplicationContext::GetInstance()->DispatchOnWindowStageWillCreate(10, windowStage);
    CJApplicationContext::GetInstance()->DispatchOnWindowStageWillDestroy(10, windowStage);
    CJApplicationContext::GetInstance()->DispatchOnAbilityWillDestroy(10);
    CJApplicationContext::GetInstance()->DispatchOnAbilityWillForeground(10);
    CJApplicationContext::GetInstance()->UnregisterAbilityLifecycleCallback(mock);
    delete winStage;
    EXPECT_TRUE(CJApplicationContext::callbacks_.empty());
}

/**
 * @tc.name: CJApplicationContextTest_Ability_003
 * @tc.desc: CjApplicationContextTest test for ability.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTest_Ability_003, TestSize.Level1)
{
    std::shared_ptr<CjAbilityLifecycleCallbackImpl> mock;
    CJApplicationContext::GetInstance()->RegisterAbilityLifecycleCallback(mock);
    auto win = std::make_shared<Rosen::WindowScene>();
    auto winStage = new Rosen::CJWindowStageImpl(win);
    WindowStagePtr windowStage = reinterpret_cast<WindowStagePtr>(winStage);
    CJApplicationContext::GetInstance()->DispatchOnAbilityWillBackground(10);
    CJApplicationContext::GetInstance()->DispatchOnNewWant(10);
    CJApplicationContext::GetInstance()->DispatchOnWillNewWant(10);
    CJApplicationContext::GetInstance()->DispatchOnAbilityWillContinue(10);
    CJApplicationContext::GetInstance()->DispatchOnWindowStageWillRestore(10, windowStage);
    CJApplicationContext::GetInstance()->DispatchOnWindowStageRestore(10, windowStage);
    CJApplicationContext::GetInstance()->DispatchOnAbilityWillSaveState(10);
    CJApplicationContext::GetInstance()->DispatchOnAbilitySaveState(10);
    CJApplicationContext::GetInstance()->UnregisterAbilityLifecycleCallback(mock);
    delete winStage;
    EXPECT_TRUE(CJApplicationContext::callbacks_.empty());
}

/**
 * @tc.name: CJApplicationContextTestOnGetRunningProcessInformation_001
 * @tc.desc: CjApplicationContextTest test for OnGetRunningProcessInformation.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestOnGetRunningProcessInformation_001, TestSize.Level1)
{
    int err = 0;
    int* errorCode = &err;
    auto info = CJApplicationContext::GetInstance()->OnGetRunningProcessInformation(errorCode);
    EXPECT_EQ(*errorCode, ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST);
    EXPECT_EQ(info, nullptr);
}

/**
 * @tc.name: CJApplicationContextTestOnOnKillProcessBySelf_001
 * @tc.desc: CjApplicationContextTest test for OnKillProcessBySelf.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestOnOnKillProcessBySelf_001, TestSize.Level1)
{
    int err = 0;
    int* errorCode = &err;
    CJApplicationContext::GetInstance()->OnKillProcessBySelf(true, errorCode);
    EXPECT_EQ(*errorCode, ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST);
}

/**
 * @tc.name: CJApplicationContextTestOnGetCurrentAppCloneIndex_001
 * @tc.desc: CjApplicationContextTest test for OnGetCurrentAppCloneIndex.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestOnGetCurrentAppCloneIndex_001, TestSize.Level1)
{
    int err = 0;
    int* errorCode = &err;
    auto index = CJApplicationContext::GetInstance()->OnGetCurrentAppCloneIndex(errorCode);
    EXPECT_EQ(*errorCode, ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST);
}

/**
 * @tc.name: CJApplicationContextTestOnRestartApp_001
 * @tc.desc: CjApplicationContextTest test for OnRestartApp.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestOnRestartApp_001, TestSize.Level1)
{
    int err = 0;
    int* errorCode = &err;
    CJApplicationContext::GetInstance()->OnRestartApp(AAFwk::Want(), errorCode);
    EXPECT_EQ(*errorCode, ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST);
}

/**
 * @tc.name: CJApplicationContextTestOnClearUpApplicationData_001
 * @tc.desc: CjApplicationContextTest test for OnClearUpApplicationData.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestOnClearUpApplicationData_001, TestSize.Level1)
{
    int err = 0;
    int* errorCode = &err;
    CJApplicationContext::GetInstance()->OnClearUpApplicationData(errorCode);
    EXPECT_EQ(*errorCode, ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST);
}

/**
 * @tc.name: CJApplicationContextTestOnSetSupportedProcessCacheSelf_001
 * @tc.desc: CjApplicationContextTest test for OnSetSupportedProcessCacheSelf.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestOnSetSupportedProcessCacheSelf_001, TestSize.Level1)
{
    int err = 0;
    int* errorCode = &err;
    CJApplicationContext::GetInstance()->OnSetSupportedProcessCacheSelf(true, errorCode);
    EXPECT_EQ(*errorCode, ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST);
}
}  // namespace AbilityRuntime
}  // namespace OHOS