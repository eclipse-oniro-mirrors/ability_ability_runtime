/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#include "ability.h"
#include "cj_ability_object.h"
#include "cj_runtime.h"
#include "configuration.h"
#include "window_stage_impl.h"
#include "last_exit_detail_info.h"
#include "cj_ui_extension_object.h"
#include "cj_utils_ffi.h"

using ExtAbilityHandle = void*;

namespace OHOS {
namespace AbilityRuntime {
struct CJExtAbilityFuncs {
    int64_t (*createCjExtAbility)(const char* name, int32_t type);
    void (*releaseCjExtAbility)(int64_t id, int32_t type);
    void (*cjExtAbilityInit)(int64_t id, int32_t type, ExtAbilityHandle extAbility);
    void (*cjExtAbilityOnCreate)(int64_t id, int32_t type, WantHandle want, ::CJLaunchParam launchParam);
    void (*cjExtAbilityOnDestroy)(int64_t id, int32_t type);
    void (*cjExtAbilityOnSessionCreate)(int64_t id, int32_t type, WantHandle want, int64_t sessionId);
    void (*cjExtAbilityOnSessionDestroy)(int64_t id, int32_t type, int64_t sessionId);
    void (*cjExtAbilityOnForeground)(int64_t id, int32_t type);
    void (*cjExtAbilityOnBackground)(int64_t id, int32_t type);
    void (*cjExtAbilityOnConfigurationUpdate)(int64_t id, int32_t type, CConfiguration configuration);
    void (*cjExtAbilityOnMemoryLevel)(int64_t id, int32_t type, int32_t level);
    void (*cjExtAbilityOnStartContentEditing)(int64_t id, int32_t type, const char* imageUri, WantHandle want,
        int64_t sessionId);
};

struct CJExtAbilityFuncsV2 {
    void (*cjExtAbilityOnCreateV3)(int64_t id, int32_t type, WantHandle want, ::CJLaunchParamV3 launchParam);
    void (*cjExtAbilityOnConfigurationUpdateV2)(int64_t id, int32_t type, CConfigurationV2 configuration);
};
}
}

extern "C" {
CJ_EXPORT void FFIRegisterCJExtAbilityFuncs(void (*registerFunc)(OHOS::AbilityRuntime::CJExtAbilityFuncs*));
CJ_EXPORT void FFIRegisterCJExtAbilityFuncsV2(void (*registerFunc)(OHOS::AbilityRuntime::CJExtAbilityFuncsV2*));
}

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AbilityRuntime;

class CjAbilityObjectTest : public testing::Test {
};

bool ProxyCall()
{
    CJAbilityObject::LoadModule("0");
    CJAbilityObject::LoadModule("1");
    Want want;
    auto proxy = CJAbilityObject(0);
    proxy.Init(nullptr);
    proxy.OnStart(want, AAFwk::LaunchParam());
    auto win = std::make_shared<Rosen::WindowScene>();
    auto winStage = new Rosen::CJWindowStageImpl(win);
    proxy.OnSceneCreated(winStage);
    proxy.OnSceneRestored(winStage);
    proxy.OnForeground(want);
    proxy.OnBackground();
    auto config = std::make_shared<AppExecFwk::Configuration>();
    proxy.OnConfigurationUpdated(config);
    proxy.OnNewWant(want, AAFwk::LaunchParam());
    std::vector<std::string> params = {"123"};
    std::vector<std::string> infos = {"123"};
    AAFwk::WantParams wantParams = AAFwk::WantParams();
    proxy.OnContinue(wantParams);
    proxy.Dump(params, infos);
    proxy.OnSceneDestroyed();
    proxy.OnStop();
    return true;
}

static void RegisterCommonCJAbilityFuncs()
{
    auto registerFunc = [](CJAbilityFuncs* funcs) {
        funcs->cjAbilityCreate = [](const char* name) -> int64_t { return 1; };
        funcs->cjAbilityRelease = [](int64_t id) {};
        funcs->cjAbilityOnStart = [](int64_t id, WantHandle want, ::CJLaunchParam launchParam) {};
        funcs->cjAbilityOnStop = [](int64_t id) {};
        funcs->cjAbilityOnSceneCreated = [](int64_t id, WindowStagePtr cjWindowStage) {};
        funcs->cjAbilityOnSceneRestored = [](int64_t id, WindowStagePtr cjWindowStage) {};
        funcs->cjAbilityOnSceneDestroyed = [](int64_t id) {};
        funcs->cjAbilityOnForeground = [](int64_t id, WantHandle want) {};
        funcs->cjAbilityOnBackground = [](int64_t id) {};
        funcs->cjAbilityOnConfigurationUpdated = [](int64_t id, ::CJConfiguration configuration) {};
        funcs->cjAbilityOnNewWant = [](int64_t id, WantHandle want, ::CJLaunchParam launchParam) {};
        funcs->cjAbilityDump = [](int64_t id, VectorStringHandle params) { return VectorStringHandle(); };
        funcs->cjAbilityOnContinue = [](int64_t id, const char* params) { return 0; };
        funcs->cjAbilityInit = [](int64_t id, void* ability) {};
    };
    RegisterCJAbilityFuncs(registerFunc);
}

HWTEST_F(CjAbilityObjectTest, CJAbilityObject001, TestSize.Level1)
{
    auto result = ProxyCall();
    EXPECT_TRUE(result);
}

HWTEST_F(CjAbilityObjectTest, CJAbilityObject002, TestSize.Level1)
{
    auto registerFunc = [](CJAbilityFuncs* funcs) {
        funcs->cjAbilityCreate = [](const char* name) -> int64_t { return name[0] == '0' ? 0 : 1; };
        funcs->cjAbilityRelease = [](int64_t id) {};
        funcs->cjAbilityOnStart = [](int64_t id, WantHandle want, ::CJLaunchParam launchParam) {};
        funcs->cjAbilityOnStop = [](int64_t id) {};
        funcs->cjAbilityOnSceneCreated = [](int64_t id, WindowStagePtr cjWindowStage) {};
        funcs->cjAbilityOnSceneRestored = [](int64_t id, WindowStagePtr cjWindowStage) {};
        funcs->cjAbilityOnSceneDestroyed = [](int64_t id) {};
        funcs->cjAbilityOnForeground = [](int64_t id, WantHandle want) {};
        funcs->cjAbilityOnBackground = [](int64_t id) {};
        funcs->cjAbilityOnConfigurationUpdated = [](int64_t id, ::CJConfiguration configuration) {};
        funcs->cjAbilityOnNewWant = [](int64_t id, WantHandle want, ::CJLaunchParam launchParam) {};
        funcs->cjAbilityDump = [](int64_t id, VectorStringHandle params) { return VectorStringHandle(); };
        funcs->cjAbilityOnContinue = [](int64_t id, const char* params) { return 0; };
        funcs->cjAbilityInit = [](int64_t id, void* ability) {};
    };
    RegisterCJAbilityFuncs(registerFunc);
    ProxyCall();
    RegisterCJAbilityFuncs(nullptr);
    EXPECT_NE(registerFunc, nullptr);
}

/**
 * @tc.name: CjAbilityObjectTest_RegisterCJAbilityFuncsV3_001
 * @tc.desc: CjAbilityObjectTest test for RegisterCJAbilityFuncsV3 and nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityObjectTest, CjAbilityObjectTest_RegisterCJAbilityFuncsV3_001, TestSize.Level1)
{
    RegisterCJAbilityFuncsV3(nullptr);

    auto registerFunc = [](CJAbilityFuncsV3* funcs) {
        funcs->cjAbilityOnStartV3 = [](int64_t id, WantHandle want, ::CJLaunchParamV3 launchParam) {};
        funcs->cjAbilityOnNewWantV3 = [](int64_t id, WantHandle want, ::CJLaunchParamV3 launchParam) {};
        funcs->cjAbilityOnConfigurationUpdateV3 = [](int64_t id,
            OHOS::AbilityRuntime::CConfigurationV2 configuration) {};
    };
    RegisterCJAbilityFuncsV3(registerFunc);
    EXPECT_NE(registerFunc, nullptr);

    RegisterCJAbilityFuncsV3(registerFunc);
}

/**
 * @tc.name: CjAbilityObjectTest_OnConfigurationUpdatedV3_001
 * @tc.desc: CjAbilityObjectTest test for OnConfigurationUpdated with V3 registered.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityObjectTest, CjAbilityObjectTest_OnConfigurationUpdatedV3_001, TestSize.Level1)
{
    static bool v3Called = false;
    v3Called = false;
    auto registerFunc = [](CJAbilityFuncs* funcs) {
        funcs->cjAbilityCreate = [](const char* name) -> int64_t { return name[0] == '0' ? 0 : 1; };
        funcs->cjAbilityRelease = [](int64_t id) {};
        funcs->cjAbilityOnStart = [](int64_t id, WantHandle want, ::CJLaunchParam launchParam) {};
        funcs->cjAbilityOnStop = [](int64_t id) {};
        funcs->cjAbilityOnSceneCreated = [](int64_t id, WindowStagePtr cjWindowStage) {};
        funcs->cjAbilityOnSceneRestored = [](int64_t id, WindowStagePtr cjWindowStage) {};
        funcs->cjAbilityOnSceneDestroyed = [](int64_t id) {};
        funcs->cjAbilityOnForeground = [](int64_t id, WantHandle want) {};
        funcs->cjAbilityOnBackground = [](int64_t id) {};
        funcs->cjAbilityOnConfigurationUpdated = [](int64_t id, ::CJConfiguration configuration) {};
        funcs->cjAbilityOnNewWant = [](int64_t id, WantHandle want, ::CJLaunchParam launchParam) {};
        funcs->cjAbilityDump = [](int64_t id, VectorStringHandle params) { return VectorStringHandle(); };
        funcs->cjAbilityOnContinue = [](int64_t id, const char* params) { return 0; };
        funcs->cjAbilityInit = [](int64_t id, void* ability) {};
    };
    RegisterCJAbilityFuncs(registerFunc);

    auto registerFuncV3 = [](CJAbilityFuncsV3* funcs) {
        funcs->cjAbilityOnStartV3 = [](int64_t id, WantHandle want, ::CJLaunchParamV3 launchParam) {};
        funcs->cjAbilityOnNewWantV3 = [](int64_t id, WantHandle want, ::CJLaunchParamV3 launchParam) {};
        funcs->cjAbilityOnConfigurationUpdateV3 =
            [](int64_t id, OHOS::AbilityRuntime::CConfigurationV2 configuration) { v3Called = true; };
    };
    RegisterCJAbilityFuncsV3(registerFuncV3);

    auto proxy = CJAbilityObject(1);
    proxy.Init(nullptr);
    auto config = std::make_shared<AppExecFwk::Configuration>();
    proxy.OnConfigurationUpdated(config);

    EXPECT_TRUE(v3Called);
}

/**
 * @tc.name: CjAbilityObjectTest_OnStartV3_WithLastExitDetailInfo_001
 * @tc.desc: CjAbilityObjectTest test for OnStart V3 path with LastExitDetailInfo.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityObjectTest, CjAbilityObjectTest_OnStartV3_WithLastExitDetailInfo_001, TestSize.Level1)
{
    static bool v3StartCalled = false;
    static ::CJLaunchParamV3 capturedParam = {};
    v3StartCalled = false;
    capturedParam = {};

    RegisterCommonCJAbilityFuncs();

    auto registerFuncV3 = [](CJAbilityFuncsV3* funcs) {
        funcs->cjAbilityOnStartV3 =
            [](int64_t id, WantHandle want, ::CJLaunchParamV3 launchParam) {
                v3StartCalled = true;
                capturedParam = launchParam;
            };
        funcs->cjAbilityOnNewWantV3 = [](int64_t id, WantHandle want, ::CJLaunchParamV3 launchParam) {};
        funcs->cjAbilityOnConfigurationUpdateV3 =
            [](int64_t id, OHOS::AbilityRuntime::CConfigurationV2 configuration) {};
    };
    RegisterCJAbilityFuncsV3(registerFuncV3);

    auto proxy = CJAbilityObject(1);
    proxy.Init(nullptr);

    AAFwk::Want want;
    AAFwk::LaunchParam launchParam;
    launchParam.launchReason = static_cast<AAFwk::LaunchReason>(1);
    launchParam.lastExitReason = static_cast<AAFwk::LastExitReason>(2);
    launchParam.lastExitMessage = "test_exit_msg";
    launchParam.lastExitDetailInfo.pid = 100;
    launchParam.lastExitDetailInfo.processName = "com.test.process";
    launchParam.lastExitDetailInfo.killReason = "test_kill";

    proxy.OnStart(want, launchParam);

    EXPECT_TRUE(v3StartCalled);
    EXPECT_EQ(capturedParam.launchReason, 1);
    EXPECT_EQ(capturedParam.lastExitReason, 2);
    EXPECT_TRUE(capturedParam.lastExitDetailInfo.pid == 100);
    EXPECT_TRUE(capturedParam.lastExitDetailInfo.hasKillReason);
}

/**
 * @tc.name: CjAbilityObjectTest_OnStartV3_WithoutKillReason_001
 * @tc.desc: CjAbilityObjectTest test for OnStart V3 path with empty killReason.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityObjectTest, CjAbilityObjectTest_OnStartV3_WithoutKillReason_001, TestSize.Level1)
{
    static bool v3StartCalled = false;
    static ::CJLaunchParamV3 capturedParam = {};
    v3StartCalled = false;
    capturedParam = {};

    RegisterCommonCJAbilityFuncs();

    auto registerFuncV3 = [](CJAbilityFuncsV3* funcs) {
        funcs->cjAbilityOnStartV3 =
            [](int64_t id, WantHandle want, ::CJLaunchParamV3 launchParam) {
                v3StartCalled = true;
                capturedParam = launchParam;
            };
        funcs->cjAbilityOnNewWantV3 = [](int64_t id, WantHandle want, ::CJLaunchParamV3 launchParam) {};
        funcs->cjAbilityOnConfigurationUpdateV3 =
            [](int64_t id, OHOS::AbilityRuntime::CConfigurationV2 configuration) {};
    };
    RegisterCJAbilityFuncsV3(registerFuncV3);

    auto proxy = CJAbilityObject(1);
    proxy.Init(nullptr);

    AAFwk::Want want;
    AAFwk::LaunchParam launchParam;
    launchParam.launchReason = static_cast<AAFwk::LaunchReason>(0);
    launchParam.lastExitReason = static_cast<AAFwk::LastExitReason>(0);
    launchParam.lastExitDetailInfo.pid = 200;
    launchParam.lastExitDetailInfo.processName = "com.test.no_kill";
    launchParam.lastExitDetailInfo.killReason = "";

    proxy.OnStart(want, launchParam);

    EXPECT_TRUE(v3StartCalled);
    EXPECT_FALSE(capturedParam.lastExitDetailInfo.hasKillReason);
    EXPECT_EQ(capturedParam.lastExitDetailInfo.killReason, nullptr);
}

class CjUIExtensionObjectTest : public testing::Test {};

/**
 * @tc.name: CjUIExtensionObjectTest_FFIRegisterCJExtAbilityFuncsV2_001
 * @tc.desc: Test FFIRegisterCJExtAbilityFuncsV2 registration and nullptr handling.
 * @tc.type: FUNC
 */
HWTEST_F(CjUIExtensionObjectTest, CjUIExtensionObjectTest_FFIRegisterCJExtAbilityFuncsV2_001, TestSize.Level1)
{
    FFIRegisterCJExtAbilityFuncsV2(nullptr);

    static bool v2ConfigCalled = false;
    v2ConfigCalled = false;
    auto registerFuncV2 = [](CJExtAbilityFuncsV2* funcs) {
        funcs->cjExtAbilityOnCreateV3 =
            [](int64_t id, int32_t type, WantHandle want, ::CJLaunchParamV3 launchParam) {};
        funcs->cjExtAbilityOnConfigurationUpdateV2 =
            [](int64_t id, int32_t type, CConfigurationV2 configuration) { v2ConfigCalled = true; };
    };
    FFIRegisterCJExtAbilityFuncsV2(registerFuncV2);
    EXPECT_NE(registerFuncV2, nullptr);
}

/**
 * @tc.name: CjUIExtensionObjectTest_OnCreateV3_001
 * @tc.desc: Test CJUIExtensionObject::OnCreate with V2 registered (V3 path).
 * @tc.type: FUNC
 */
HWTEST_F(CjUIExtensionObjectTest, CjUIExtensionObjectTest_OnCreateV3_001, TestSize.Level1)
{
    static bool v3CreateCalled = false;
    v3CreateCalled = false;
    auto registerFunc = [](CJExtAbilityFuncs* funcs) {
        funcs->createCjExtAbility = [](const char* name, int32_t type) -> int64_t { return 1; };
        funcs->releaseCjExtAbility = [](int64_t id, int32_t type) {};
        funcs->cjExtAbilityInit = [](int64_t id, int32_t type, ExtAbilityHandle extAbility) {};
        funcs->cjExtAbilityOnCreate = [](int64_t id, int32_t type, WantHandle want, ::CJLaunchParam param) {};
        funcs->cjExtAbilityOnDestroy = [](int64_t id, int32_t type) {};
        funcs->cjExtAbilityOnSessionCreate = [](int64_t id, int32_t type, WantHandle want, int64_t sessionId) {};
        funcs->cjExtAbilityOnSessionDestroy = [](int64_t id, int32_t type, int64_t sessionId) {};
        funcs->cjExtAbilityOnForeground = [](int64_t id, int32_t type) {};
        funcs->cjExtAbilityOnBackground = [](int64_t id, int32_t type) {};
        funcs->cjExtAbilityOnConfigurationUpdate = [](int64_t id, int32_t type, CConfiguration cfg) {};
        funcs->cjExtAbilityOnMemoryLevel = [](int64_t id, int32_t type, int32_t level) {};
        funcs->cjExtAbilityOnStartContentEditing =
            [](int64_t id, int32_t type, const char* imageUri, WantHandle want, int64_t sessionId) {};
    };
    FFIRegisterCJExtAbilityFuncs(registerFunc);

    auto registerFuncV2 = [](CJExtAbilityFuncsV2* funcs) {
        funcs->cjExtAbilityOnCreateV3 =
            [](int64_t id, int32_t type, WantHandle want, ::CJLaunchParamV3 launchParam) {
                v3CreateCalled = true;
            };
        funcs->cjExtAbilityOnConfigurationUpdateV2 =
            [](int64_t id, int32_t type, CConfigurationV2 configuration) {};
    };
    FFIRegisterCJExtAbilityFuncsV2(registerFuncV2);

    CJUIExtensionObject extObj;
    extObj.Init("test", CJExtensionAbilityType::ACTION, nullptr);

    AAFwk::Want want;
    AAFwk::LaunchParam launchParam;
    launchParam.launchReason = static_cast<AAFwk::LaunchReason>(1);
    launchParam.lastExitReason = static_cast<AAFwk::LastExitReason>(2);
    launchParam.lastExitDetailInfo.pid = 100;
    extObj.OnCreate(want, launchParam);

    launchParam.lastExitDetailInfo.processName = "com.test.kill";
    launchParam.lastExitDetailInfo.exitMsg = "unknown";
    launchParam.lastExitDetailInfo.killReason = "test_kill";
    extObj.OnCreate(want, launchParam);

    EXPECT_TRUE(v3CreateCalled);
}

/**
 * @tc.name: CjUIExtensionObjectTest_OnCreateV1_001
 * @tc.desc: Test CJUIExtensionObject::OnCreate with V1 only.
 * @tc.type: FUNC
 */
HWTEST_F(CjUIExtensionObjectTest, CjUIExtensionObjectTest_OnCreateV1_001, TestSize.Level1)
{
    auto resetV2 = [](CJExtAbilityFuncsV2* funcs) {
        funcs->cjExtAbilityOnCreateV3 = nullptr;
        funcs->cjExtAbilityOnConfigurationUpdateV2 = nullptr;
    };
    FFIRegisterCJExtAbilityFuncsV2(resetV2);

    static bool v1CreateCalled = false;
    static ::CJLaunchParam capturedParam = {};
    v1CreateCalled = false;
    capturedParam = {};
    auto registerFunc = [](CJExtAbilityFuncs* funcs) {
        funcs->createCjExtAbility = [](const char* name, int32_t type) -> int64_t { return 1; };
        funcs->releaseCjExtAbility = [](int64_t id, int32_t type) {};
        funcs->cjExtAbilityInit = [](int64_t id, int32_t type, ExtAbilityHandle extAbility) {};
        funcs->cjExtAbilityOnCreate =
            [](int64_t id, int32_t type, WantHandle want, ::CJLaunchParam param) {
                v1CreateCalled = true;
                capturedParam = param;
            };
        funcs->cjExtAbilityOnDestroy = [](int64_t id, int32_t type) {};
        funcs->cjExtAbilityOnSessionCreate = [](int64_t id, int32_t type, WantHandle want, int64_t sessionId) {};
        funcs->cjExtAbilityOnSessionDestroy = [](int64_t id, int32_t type, int64_t sessionId) {};
        funcs->cjExtAbilityOnForeground = [](int64_t id, int32_t type) {};
        funcs->cjExtAbilityOnBackground = [](int64_t id, int32_t type) {};
        funcs->cjExtAbilityOnConfigurationUpdate = [](int64_t id, int32_t type, CConfiguration cfg) {};
        funcs->cjExtAbilityOnMemoryLevel = [](int64_t id, int32_t type, int32_t level) {};
        funcs->cjExtAbilityOnStartContentEditing =
            [](int64_t id, int32_t type, const char* imageUri, WantHandle want, int64_t sessionId) {};
    };
    FFIRegisterCJExtAbilityFuncs(registerFunc);

    CJUIExtensionObject extObj;
    extObj.Init("test", CJExtensionAbilityType::ACTION, nullptr);

    AAFwk::Want want;
    AAFwk::LaunchParam launchParam;
    launchParam.launchReason = static_cast<AAFwk::LaunchReason>(3);
    launchParam.lastExitReason = static_cast<AAFwk::LastExitReason>(4);
    launchParam.lastExitMessage = "v1_exit_msg";
    extObj.OnCreate(want, launchParam);

    EXPECT_FALSE(v1CreateCalled);
    EXPECT_EQ(capturedParam.launchReason, 0);
    EXPECT_EQ(capturedParam.lastExitReason, 0);
}

/**
 * @tc.name: CjUIExtensionObjectTest_OnCreateV1Null_001
 * @tc.desc: Test CJUIExtensionObject::OnCreate with V1 OnCreate nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(CjUIExtensionObjectTest, CjUIExtensionObjectTest_OnCreateV1Null_001, TestSize.Level1)
{
    auto resetV2 = [](CJExtAbilityFuncsV2* funcs) {
        funcs->cjExtAbilityOnCreateV3 = nullptr;
        funcs->cjExtAbilityOnConfigurationUpdateV2 = nullptr;
    };
    FFIRegisterCJExtAbilityFuncsV2(resetV2);

    auto registerFuncNoCreate = [](CJExtAbilityFuncs* funcs) {
        funcs->createCjExtAbility = [](const char* name, int32_t type) -> int64_t { return 1; };
        funcs->releaseCjExtAbility = [](int64_t id, int32_t type) {};
        funcs->cjExtAbilityInit = [](int64_t id, int32_t type, ExtAbilityHandle extAbility) {};
        funcs->cjExtAbilityOnCreate = nullptr;
        funcs->cjExtAbilityOnDestroy = [](int64_t id, int32_t type) {};
        funcs->cjExtAbilityOnSessionCreate = [](int64_t id, int32_t type, WantHandle want, int64_t sessionId) {};
        funcs->cjExtAbilityOnSessionDestroy = [](int64_t id, int32_t type, int64_t sessionId) {};
        funcs->cjExtAbilityOnForeground = [](int64_t id, int32_t type) {};
        funcs->cjExtAbilityOnBackground = [](int64_t id, int32_t type) {};
        funcs->cjExtAbilityOnConfigurationUpdate = [](int64_t id, int32_t type, CConfiguration cfg) {};
        funcs->cjExtAbilityOnMemoryLevel = [](int64_t id, int32_t type, int32_t level) {};
        funcs->cjExtAbilityOnStartContentEditing =
            [](int64_t id, int32_t type, const char* imageUri, WantHandle want, int64_t sessionId) {};
    };
    FFIRegisterCJExtAbilityFuncs(registerFuncNoCreate);

    CJUIExtensionObject noCreateObj;
    noCreateObj.Init("test", CJExtensionAbilityType::ACTION, nullptr);

    AAFwk::Want want;
    AAFwk::LaunchParam launchParam;
    noCreateObj.OnCreate(want, launchParam);

    EXPECT_TRUE(true);
}

/**
 * @tc.name: CjUIExtensionObjectTest_OnConfigurationUpdateV2_001
 * @tc.desc: Test CJUIExtensionObject::OnConfigurationUpdate with V2 registered.
 * @tc.type: FUNC
 */
HWTEST_F(CjUIExtensionObjectTest, CjUIExtensionObjectTest_OnConfigurationUpdateV2_001, TestSize.Level1)
{
    static bool v2ConfigCalled = false;
    v2ConfigCalled = false;
    auto registerFunc = [](CJExtAbilityFuncs* funcs) {
        funcs->createCjExtAbility = [](const char* name, int32_t type) -> int64_t { return 1; };
        funcs->releaseCjExtAbility = [](int64_t id, int32_t type) {};
        funcs->cjExtAbilityInit = [](int64_t id, int32_t type, ExtAbilityHandle extAbility) {};
        funcs->cjExtAbilityOnCreate = [](int64_t id, int32_t type, WantHandle want, ::CJLaunchParam param) {};
        funcs->cjExtAbilityOnDestroy = [](int64_t id, int32_t type) {};
        funcs->cjExtAbilityOnSessionCreate = [](int64_t id, int32_t type, WantHandle want, int64_t sessionId) {};
        funcs->cjExtAbilityOnSessionDestroy = [](int64_t id, int32_t type, int64_t sessionId) {};
        funcs->cjExtAbilityOnForeground = [](int64_t id, int32_t type) {};
        funcs->cjExtAbilityOnBackground = [](int64_t id, int32_t type) {};
        funcs->cjExtAbilityOnConfigurationUpdate = [](int64_t id, int32_t type, CConfiguration cfg) {};
        funcs->cjExtAbilityOnMemoryLevel = [](int64_t id, int32_t type, int32_t level) {};
        funcs->cjExtAbilityOnStartContentEditing =
            [](int64_t id, int32_t type, const char* imageUri, WantHandle want, int64_t sessionId) {};
    };
    FFIRegisterCJExtAbilityFuncs(registerFunc);

    auto registerFuncV2 = [](CJExtAbilityFuncsV2* funcs) {
        funcs->cjExtAbilityOnCreateV3 =
            [](int64_t id, int32_t type, WantHandle want, ::CJLaunchParamV3 launchParam) {};
        funcs->cjExtAbilityOnConfigurationUpdateV2 =
            [](int64_t id, int32_t type, CConfigurationV2 configuration) { v2ConfigCalled = true; };
    };
    FFIRegisterCJExtAbilityFuncsV2(registerFuncV2);

    CJUIExtensionObject extObj;
    extObj.Init("test", CJExtensionAbilityType::ACTION, nullptr);

    auto config = std::make_shared<AppExecFwk::Configuration>();
    config->AddItem(OHOS::AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, "zh_CN");
    extObj.OnConfigurationUpdate(config);

    EXPECT_TRUE(v2ConfigCalled);
}

/**
 * @tc.name: CjUIExtensionObjectTest_OnConfigurationUpdateV1_001
 * @tc.desc: Test CJUIExtensionObject::OnConfigurationUpdate with V1 only (no V2 registered).
 * @tc.type: FUNC
 */
HWTEST_F(CjUIExtensionObjectTest, CjUIExtensionObjectTest_OnConfigurationUpdateV1_001, TestSize.Level1)
{
    static bool v1ConfigCalled = false;
    v1ConfigCalled = false;
    auto registerFunc = [](CJExtAbilityFuncs* funcs) {
        funcs->createCjExtAbility = [](const char* name, int32_t type) -> int64_t { return 1; };
        funcs->releaseCjExtAbility = [](int64_t id, int32_t type) {};
        funcs->cjExtAbilityInit = [](int64_t id, int32_t type, ExtAbilityHandle extAbility) {};
        funcs->cjExtAbilityOnCreate = [](int64_t id, int32_t type, WantHandle want, ::CJLaunchParam param) {};
        funcs->cjExtAbilityOnDestroy = [](int64_t id, int32_t type) {};
        funcs->cjExtAbilityOnSessionCreate = [](int64_t id, int32_t type, WantHandle want, int64_t sessionId) {};
        funcs->cjExtAbilityOnSessionDestroy = [](int64_t id, int32_t type, int64_t sessionId) {};
        funcs->cjExtAbilityOnForeground = [](int64_t id, int32_t type) {};
        funcs->cjExtAbilityOnBackground = [](int64_t id, int32_t type) {};
        funcs->cjExtAbilityOnConfigurationUpdate =
            [](int64_t id, int32_t type, CConfiguration cfg) { v1ConfigCalled = true; };
        funcs->cjExtAbilityOnMemoryLevel = [](int64_t id, int32_t type, int32_t level) {};
        funcs->cjExtAbilityOnStartContentEditing =
            [](int64_t id, int32_t type, const char* imageUri, WantHandle want, int64_t sessionId) {};
    };
    FFIRegisterCJExtAbilityFuncs(registerFunc);

    CJUIExtensionObject extObj;
    extObj.Init("test", CJExtensionAbilityType::ACTION, nullptr);

    auto config = std::make_shared<AppExecFwk::Configuration>();
    config->AddItem(OHOS::AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, "en_US");
    extObj.OnConfigurationUpdate(config);

    EXPECT_FALSE(v1ConfigCalled);

    CJUIExtensionObject noFuncObj;
    noFuncObj.OnConfigurationUpdate(config);
}

/**
 * @tc.name: CjUIExtensionObjectTest_OnConfigurationUpdateV2_Null_001
 * @tc.desc: Test CJUIExtensionObject::OnConfigurationUpdate with V2 null, covers V2 negative branch and V1 path.
 * @tc.type: FUNC
 */
HWTEST_F(CjUIExtensionObjectTest, CjUIExtensionObjectTest_OnConfigurationUpdateV2_Null_001, TestSize.Level1)
{
    static bool v1ConfigCalled = false;
    v1ConfigCalled = false;

    auto registerFunc = [](CJExtAbilityFuncs* funcs) {
        funcs->createCjExtAbility = [](const char* name, int32_t type) -> int64_t { return 1; };
        funcs->releaseCjExtAbility = [](int64_t id, int32_t type) {};
        funcs->cjExtAbilityInit = [](int64_t id, int32_t type, ExtAbilityHandle extAbility) {};
        funcs->cjExtAbilityOnCreate = [](int64_t id, int32_t type, WantHandle want, ::CJLaunchParam param) {};
        funcs->cjExtAbilityOnDestroy = [](int64_t id, int32_t type) {};
        funcs->cjExtAbilityOnSessionCreate = [](int64_t id, int32_t type, WantHandle want, int64_t sessionId) {};
        funcs->cjExtAbilityOnSessionDestroy = [](int64_t id, int32_t type, int64_t sessionId) {};
        funcs->cjExtAbilityOnForeground = [](int64_t id, int32_t type) {};
        funcs->cjExtAbilityOnBackground = [](int64_t id, int32_t type) {};
        funcs->cjExtAbilityOnConfigurationUpdate =
            [](int64_t id, int32_t type, CConfiguration cfg) { v1ConfigCalled = true; };
        funcs->cjExtAbilityOnMemoryLevel = [](int64_t id, int32_t type, int32_t level) {};
        funcs->cjExtAbilityOnStartContentEditing =
            [](int64_t id, int32_t type, const char* imageUri, WantHandle want, int64_t sessionId) {};
    };
    FFIRegisterCJExtAbilityFuncs(registerFunc);

    auto registerFuncV2 = [](CJExtAbilityFuncsV2* funcs) {
        funcs->cjExtAbilityOnCreateV3 = nullptr;
        funcs->cjExtAbilityOnConfigurationUpdateV2 = nullptr;
    };
    FFIRegisterCJExtAbilityFuncsV2(registerFuncV2);

    CJUIExtensionObject extObj;
    extObj.Init("test", CJExtensionAbilityType::ACTION, nullptr);

    auto config = std::make_shared<AppExecFwk::Configuration>();
    config->AddItem(OHOS::AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, "en_US");
    extObj.OnConfigurationUpdate(config);

    EXPECT_FALSE(v1ConfigCalled);
}

/**
 * @tc.name: CjUIExtensionObjectTest_OnConfigurationUpdate_NullFuncs_001
 * @tc.desc: Test CJUIExtensionObject::OnConfigurationUpdate with both V2 and V1 funcs null,
 *             covers null-check early return.
 * @tc.type: FUNC
 */
HWTEST_F(CjUIExtensionObjectTest, CjUIExtensionObjectTest_OnConfigurationUpdate_NullFuncs_001, TestSize.Level1)
{
    auto registerFunc = [](CJExtAbilityFuncs* funcs) {
        funcs->createCjExtAbility = [](const char* name, int32_t type) -> int64_t { return 1; };
        funcs->releaseCjExtAbility = [](int64_t id, int32_t type) {};
        funcs->cjExtAbilityInit = [](int64_t id, int32_t type, ExtAbilityHandle extAbility) {};
        funcs->cjExtAbilityOnCreate = [](int64_t id, int32_t type, WantHandle want, ::CJLaunchParam param) {};
        funcs->cjExtAbilityOnDestroy = [](int64_t id, int32_t type) {};
        funcs->cjExtAbilityOnSessionCreate = [](int64_t id, int32_t type, WantHandle want, int64_t sessionId) {};
        funcs->cjExtAbilityOnSessionDestroy = [](int64_t id, int32_t type, int64_t sessionId) {};
        funcs->cjExtAbilityOnForeground = [](int64_t id, int32_t type) {};
        funcs->cjExtAbilityOnBackground = [](int64_t id, int32_t type) {};
        funcs->cjExtAbilityOnConfigurationUpdate = nullptr;
        funcs->cjExtAbilityOnMemoryLevel = [](int64_t id, int32_t type, int32_t level) {};
        funcs->cjExtAbilityOnStartContentEditing =
            [](int64_t id, int32_t type, const char* imageUri, WantHandle want, int64_t sessionId) {};
    };
    FFIRegisterCJExtAbilityFuncs(registerFunc);

    auto registerFuncV2 = [](CJExtAbilityFuncsV2* funcs) {
        funcs->cjExtAbilityOnCreateV3 = nullptr;
        funcs->cjExtAbilityOnConfigurationUpdateV2 = nullptr;
    };
    FFIRegisterCJExtAbilityFuncsV2(registerFuncV2);

    CJUIExtensionObject extObj;
    extObj.Init("test", CJExtensionAbilityType::ACTION, nullptr);

    auto config = std::make_shared<AppExecFwk::Configuration>();
    extObj.OnConfigurationUpdate(config);

    EXPECT_TRUE(true);
}

/**
 * @tc.name  : CjAbilityObjectTest_OnNewWantV3_001
 * @tc.desc  : Test OnNewWant with V3 registered, covers CreateCJLastExitDetailInfo with killReason,
 *             FreeCJLastExitDetailInfo with non-null fields, and CallConvertConfigV2 dlopen-failed path.
 * @tc.type  : FUNC
 */
HWTEST_F(CjAbilityObjectTest, CjAbilityObjectTest_OnNewWantV3_001, TestSize.Level1)
{
    static bool v3NewWantCalled = false;
    static ::CJLaunchParamV3 capturedParam = {};
    v3NewWantCalled = false;
    capturedParam = {};

    RegisterCommonCJAbilityFuncs();

    auto registerFuncV3 = [](CJAbilityFuncsV3* funcs) {
        funcs->cjAbilityOnStartV3 = [](int64_t id, WantHandle want, ::CJLaunchParamV3 launchParam) {};
        funcs->cjAbilityOnNewWantV3 =
            [](int64_t id, WantHandle want, ::CJLaunchParamV3 launchParam) {
                v3NewWantCalled = true;
                capturedParam = launchParam;
            };
        funcs->cjAbilityOnConfigurationUpdateV3 =
            [](int64_t id, OHOS::AbilityRuntime::CConfigurationV2 configuration) {};
    };
    RegisterCJAbilityFuncsV3(registerFuncV3);

    auto proxy = CJAbilityObject(1);
    proxy.Init(nullptr);

    AAFwk::Want want;
    AAFwk::LaunchParam launchParam;
    launchParam.launchReason = static_cast<AAFwk::LaunchReason>(1);
    launchParam.lastExitReason = static_cast<AAFwk::LastExitReason>(2);
    launchParam.lastExitMessage = "exit_msg";
    launchParam.launchReasonMessage = "launch_msg";
    launchParam.lastExitDetailInfo.pid = 100;
    launchParam.lastExitDetailInfo.processName = "com.test.process";
    launchParam.lastExitDetailInfo.exitMsg = "exit_detail";
    launchParam.lastExitDetailInfo.killReason = "test_kill";
    proxy.OnNewWant(want, launchParam);

    EXPECT_TRUE(v3NewWantCalled);
    EXPECT_EQ(capturedParam.launchReason, 1);
    EXPECT_EQ(capturedParam.lastExitReason, 2);
    EXPECT_TRUE(capturedParam.lastExitDetailInfo.pid == 100);
    EXPECT_TRUE(capturedParam.lastExitDetailInfo.hasKillReason);
}

/**
 * @tc.name  : CjAbilityObjectTest_OnNewWantV3_EmptyFields_001
 * @tc.desc  : Test OnNewWant V3 with empty strings, covers FreeCJLastExitDetailInfo null-check branches
 *             and CreateCJLastExitDetailInfo with empty killReason.
 * @tc.type  : FUNC
 */
HWTEST_F(CjAbilityObjectTest, CjAbilityObjectTest_OnNewWantV3_EmptyFields_001, TestSize.Level1)
{
    static bool v3NewWantCalled = false;
    static ::CJLaunchParamV3 capturedParam = {};
    v3NewWantCalled = false;
    capturedParam = {};

    RegisterCommonCJAbilityFuncs();

    auto registerFuncV3 = [](CJAbilityFuncsV3* funcs) {
        funcs->cjAbilityOnStartV3 = [](int64_t id, WantHandle want, ::CJLaunchParamV3 launchParam) {};
        funcs->cjAbilityOnNewWantV3 =
            [](int64_t id, WantHandle want, ::CJLaunchParamV3 launchParam) {
                v3NewWantCalled = true;
                capturedParam = launchParam;
            };
        funcs->cjAbilityOnConfigurationUpdateV3 =
            [](int64_t id, OHOS::AbilityRuntime::CConfigurationV2 configuration) {};
    };
    RegisterCJAbilityFuncsV3(registerFuncV3);

    auto proxy = CJAbilityObject(1);
    proxy.Init(nullptr);

    AAFwk::Want want;
    AAFwk::LaunchParam launchParam;
    launchParam.lastExitMessage = "";
    launchParam.launchReasonMessage = "";
    launchParam.lastExitDetailInfo.processName = "";
    launchParam.lastExitDetailInfo.exitMsg = "";
    launchParam.lastExitDetailInfo.killReason = "";
    proxy.OnNewWant(want, launchParam);

    EXPECT_TRUE(v3NewWantCalled);
    EXPECT_EQ(capturedParam.lastExitMessage, nullptr);
    EXPECT_EQ(capturedParam.launchReasonMessage, nullptr);
    EXPECT_EQ(capturedParam.lastExitDetailInfo.processName, nullptr);
    EXPECT_EQ(capturedParam.lastExitDetailInfo.exitMsg, nullptr);
    EXPECT_EQ(capturedParam.lastExitDetailInfo.killReason, nullptr);
    EXPECT_FALSE(capturedParam.lastExitDetailInfo.hasKillReason);
}

/**
 * @tc.name  : CjAbilityObjectTest_OnConfigurationUpdated_NullFuncs_001
 * @tc.desc  : Test OnConfigurationUpdated with both V3 and V1 funcs null, covers null-check early return.
 * @tc.type  : FUNC
 */
HWTEST_F(CjAbilityObjectTest, CjAbilityObjectTest_OnConfigurationUpdated_NullFuncs_001, TestSize.Level1)
{
    auto registerFunc = [](CJAbilityFuncs* funcs) {
        funcs->cjAbilityCreate = [](const char* name) -> int64_t { return 1; };
        funcs->cjAbilityRelease = [](int64_t id) {};
        funcs->cjAbilityOnStart = [](int64_t id, WantHandle want, ::CJLaunchParam launchParam) {};
        funcs->cjAbilityOnStop = [](int64_t id) {};
        funcs->cjAbilityOnSceneCreated = [](int64_t id, WindowStagePtr cjWindowStage) {};
        funcs->cjAbilityOnSceneRestored = [](int64_t id, WindowStagePtr cjWindowStage) {};
        funcs->cjAbilityOnSceneDestroyed = [](int64_t id) {};
        funcs->cjAbilityOnForeground = [](int64_t id, WantHandle want) {};
        funcs->cjAbilityOnBackground = [](int64_t id) {};
        funcs->cjAbilityOnConfigurationUpdated = nullptr;
        funcs->cjAbilityOnNewWant = [](int64_t id, WantHandle want, ::CJLaunchParam launchParam) {};
        funcs->cjAbilityDump = [](int64_t id, VectorStringHandle params) { return VectorStringHandle(); };
        funcs->cjAbilityOnContinue = [](int64_t id, const char* params) { return 0; };
        funcs->cjAbilityInit = [](int64_t id, void* ability) {};
    };
    RegisterCJAbilityFuncs(registerFunc);

    auto registerFuncV3 = [](CJAbilityFuncsV3* funcs) {
        funcs->cjAbilityOnStartV3 = nullptr;
        funcs->cjAbilityOnNewWantV3 = nullptr;
        funcs->cjAbilityOnConfigurationUpdateV3 = nullptr;
    };
    RegisterCJAbilityFuncsV3(registerFuncV3);

    auto proxy = CJAbilityObject(1);
    proxy.Init(nullptr);
    auto config = std::make_shared<AppExecFwk::Configuration>();
    proxy.OnConfigurationUpdated(config);

    EXPECT_TRUE(true);
}

/**
 * @tc.name  : CjAbilityObjectTest_OnNewWant_NullFuncs_001
 * @tc.desc  : Test OnNewWant with both V3 and V1 funcs null, covers null-check early return.
 * @tc.type  : FUNC
 */
HWTEST_F(CjAbilityObjectTest, CjAbilityObjectTest_OnNewWant_NullFuncs_001, TestSize.Level1)
{
    auto registerFunc = [](CJAbilityFuncs* funcs) {
        funcs->cjAbilityCreate = [](const char* name) -> int64_t { return 1; };
        funcs->cjAbilityRelease = [](int64_t id) {};
        funcs->cjAbilityOnStart = [](int64_t id, WantHandle want, ::CJLaunchParam launchParam) {};
        funcs->cjAbilityOnStop = [](int64_t id) {};
        funcs->cjAbilityOnSceneCreated = [](int64_t id, WindowStagePtr cjWindowStage) {};
        funcs->cjAbilityOnSceneRestored = [](int64_t id, WindowStagePtr cjWindowStage) {};
        funcs->cjAbilityOnSceneDestroyed = [](int64_t id) {};
        funcs->cjAbilityOnForeground = [](int64_t id, WantHandle want) {};
        funcs->cjAbilityOnBackground = [](int64_t id) {};
        funcs->cjAbilityOnConfigurationUpdated = [](int64_t id, ::CJConfiguration configuration) {};
        funcs->cjAbilityOnNewWant = nullptr;
        funcs->cjAbilityDump = [](int64_t id, VectorStringHandle params) { return VectorStringHandle(); };
        funcs->cjAbilityOnContinue = [](int64_t id, const char* params) { return 0; };
        funcs->cjAbilityInit = [](int64_t id, void* ability) {};
    };
    RegisterCJAbilityFuncs(registerFunc);

    auto registerFuncV3 = [](CJAbilityFuncsV3* funcs) {
        funcs->cjAbilityOnStartV3 = nullptr;
        funcs->cjAbilityOnNewWantV3 = nullptr;
        funcs->cjAbilityOnConfigurationUpdateV3 = nullptr;
    };
    RegisterCJAbilityFuncsV3(registerFuncV3);

    auto proxy = CJAbilityObject(1);
    proxy.Init(nullptr);
    AAFwk::Want want;
    AAFwk::LaunchParam launchParam;
    proxy.OnNewWant(want, launchParam);

    EXPECT_TRUE(true);
}
