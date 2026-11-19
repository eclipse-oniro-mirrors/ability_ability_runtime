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
#include "native_extension/extension_ability_impl.h"
#include "mock_native_module_manager.h"
#include "module_manager/native_module_manager.h"
#include "native_runtime.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class NativeRuntimeTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void NativeRuntimeTest::SetUpTestCase() {}

void NativeRuntimeTest::TearDownTestCase() {}

void NativeRuntimeTest::SetUp()
{
    MockResetModuleManagerState();
}

void NativeRuntimeTest::TearDown()
{
    MockResetModuleManagerState();
}

/**
 * @tc.name: LoadModule_0100
 * @tc.desc: LoadModule_0100
 * @tc.type: FUNC
 */
HWTEST_F(NativeRuntimeTest, LoadModule_0100, TestSize.Level1)
{
    AbilityRuntime_ExtensionInstance instance;
    MockGetLdNamespaceName(false);
    MockDefaultNamespaceName(false);
    bool ret = NativeRuntime::LoadModule("testBundleName/moduleName", "test.so", "testAbilityName", instance);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: LoadModule_0200
 * @tc.desc: LoadModule_0200
 * @tc.type: FUNC
 */
HWTEST_F(NativeRuntimeTest, LoadModule_0200, TestSize.Level1)
{
    AbilityRuntime_ExtensionInstance instance;
    MockGetLdNamespaceName(true);
    MockDefaultNamespaceName(false);
    MockGetLdNamespaceNameStr("moduleNs_testBundleName/moduleName");
    bool ret = NativeRuntime::LoadModule("testBundleName/moduleName", "test.so", "testAbilityName", instance);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: LoadModule_0300
 * @tc.desc: LoadModule_0300
 * @tc.type: FUNC
 */
HWTEST_F(NativeRuntimeTest, LoadModule_0300, TestSize.Level1)
{
    AbilityRuntime_ExtensionInstance instance;
    MockGetLdNamespaceName(false);
    MockDefaultNamespaceName(true);
    MockGetLdNamespaceNameStr("moduleNs_default");
    bool ret = NativeRuntime::LoadModule("testBundleName/moduleName", "test.so", "testAbilityName", instance);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: StartDebugMode_0100
 * @tc.desc: StartDebugMode returns when local debug and developer mode are both false
 * @tc.type: FUNC
 */
HWTEST_F(NativeRuntimeTest, StartDebugMode_0100, TestSize.Level1)
{
    Runtime::DebugOption debugOption;
    debugOption.isDebugApp = false;
    debugOption.isDebugFromLocal = false;
    debugOption.isDeveloperMode = false;
    debugOption.isDebugApp = false;
    NativeRuntime::StartDebugMode(debugOption, "com.test.bundle");
    EXPECT_EQ(debugOption.isDebugApp, false);
}

/**
 * @tc.name: StartDebugMode_0200
 * @tc.desc: StartDebugMode is callable when developer mode is true
 * @tc.type: FUNC
 */
HWTEST_F(NativeRuntimeTest, StartDebugMode_0200, TestSize.Level1)
{
    Runtime::DebugOption debugOption;
    debugOption.isDebugFromLocal = false;
    debugOption.isDeveloperMode = true;
    debugOption.isDebugApp = false;
    NativeRuntime::StartDebugMode(debugOption, "com.test.bundle");
    EXPECT_EQ(debugOption.isDebugApp, false);
}

/**
 * @tc.name: StartDebugMode_0300
 * @tc.desc: StartDebugMode is callable when local debug is true
 * @tc.type: FUNC
 */
HWTEST_F(NativeRuntimeTest, StartDebugMode_0300, TestSize.Level1)
{
    Runtime::DebugOption debugOption;
    debugOption.isDebugFromLocal = true;
    debugOption.isDeveloperMode = false;
    debugOption.isDebugApp = false;
    NativeRuntime::StartDebugMode(debugOption, "com.test.bundle");
    EXPECT_EQ(debugOption.isDebugApp, false);
}

/**
 * @tc.name: StartDebugMode_0400
 * @tc.desc: StartDebugMode is callable for debug app with release provision type
 * @tc.type: FUNC
 */
HWTEST_F(NativeRuntimeTest, StartDebugMode_0400, TestSize.Level1)
{
    Runtime::DebugOption debugOption;
    debugOption.isDebugFromLocal = true;
    debugOption.isDeveloperMode = true;
    debugOption.isDebugApp = true;
    debugOption.appProvisionType = AppExecFwk::Constants::APP_PROVISION_TYPE_RELEASE;
    NativeRuntime::StartDebugMode(debugOption, "com.test.bundle");
    EXPECT_EQ(debugOption.isDebugApp, true);
}

/**
 * @tc.name: StartDebugMode_0500
 * @tc.desc: StartDebugMode is callable for debug app with non-release provision type
 * @tc.type: FUNC
 */
HWTEST_F(NativeRuntimeTest, StartDebugMode_0500, TestSize.Level1)
{
    Runtime::DebugOption debugOption;
    debugOption.isDebugFromLocal = true;
    debugOption.isDeveloperMode = true;
    debugOption.isDebugApp = true;
    debugOption.appProvisionType = "testProvisionType";
    NativeRuntime::StartDebugMode(debugOption, "com.test.bundle");
    NativeRuntime::StartDebugMode(debugOption, "com.test.bundle");
    NativeRuntime::StopDebugMode();
    EXPECT_EQ(debugOption.isDebugApp, true);
}
}  // namespace AbilityRuntime
}  // namespace OHOS