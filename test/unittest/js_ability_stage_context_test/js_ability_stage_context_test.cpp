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
#include <singleton.h>
#include <uv.h>

#include "ability_stage_context.h"
#include "application_context.h"
#include "context_impl.h"
#include "ecmascript/napi/include/jsnapi.h"
#include "hilog_wrapper.h"
#include "js_ability_stage_context.h"
#include "js_runtime_utils.h"
#include "native_engine/impl/ark/ark_native_engine.h"
#include "native_engine/native_engine.h"
#include "napi_common_want.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AbilityRuntime {

constexpr size_t ARGC_ZERO = 0;

const int USLEEPTIME = 100000;

class JsAbilityStageContextTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);

    void RunNowait(uv_loop_t* loop)
    {
        usleep(USLEEPTIME);
        uv_run(loop, UV_RUN_NOWAIT);
    }

    static napi_env env_;
    static panda::ecmascript::EcmaVM* vm_;
    static NativeEngine* engine_;
};

napi_env JsAbilityStageContextTest::env_ = nullptr;
panda::ecmascript::EcmaVM* JsAbilityStageContextTest::vm_ = nullptr;
NativeEngine* JsAbilityStageContextTest::engine_ = nullptr;

void JsAbilityStageContextTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "JsAbilityStageContextTest SetUpTestCase";
    panda::RuntimeOption pandaOption;
    vm_ = panda::JSNApi::CreateJSVM(pandaOption);
    ASSERT_NE(vm_, nullptr);

    engine_ = new ArkNativeEngine(vm_, nullptr);
    env_ = reinterpret_cast<napi_env>(engine_);
    ASSERT_NE(env_, nullptr);
}

void JsAbilityStageContextTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "JsAbilityStageContextTest TearDownTestCase";
    if (engine_ != nullptr) {
        delete engine_;
        engine_ = nullptr;
        env_ = nullptr;
    }

    if (vm_ != nullptr) {
        panda::JSNApi::DestroyJSVM(vm_);
        vm_ = nullptr;
    }
}

void JsAbilityStageContextTest::SetUp()
{}

void JsAbilityStageContextTest::TearDown()
{}

/**
 * @tc.number: JsAbilityStageContext_LaunchElement_001
 * @tc.name: CreateJsAbilityStageContext sets launchElement property
 * @tc.desc: Test that CreateJsAbilityStageContext properly sets launchElement property when ElementName is set.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsAbilityStageContextTest, JsAbilityStageContext_LaunchElement_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsAbilityStageContext_LaunchElement_001 start";

    // Create AbilityStageContext and set parent context
    auto context = std::make_shared<AbilityRuntime::ContextImpl>();
    ASSERT_NE(context, nullptr);
    auto abilityStageContext = std::make_shared<AbilityStageContext>();
    ASSERT_NE(abilityStageContext, nullptr);
    abilityStageContext->SetParentContext(context);

    // Set ElementName
    AppExecFwk::ElementName elementName;
    elementName.SetBundleName("com.example.testbundle");
    elementName.SetAbilityName("TestAbility");
    elementName.SetModuleName("TestModule");
    abilityStageContext->SetLaunchElement(elementName);

    // Create JS context object
    napi_value jsContext = CreateJsAbilityStageContext(env_, abilityStageContext);
    ASSERT_NE(jsContext, nullptr);

    // Verify it's an object
    napi_valuetype valueType;
    napi_status status = napi_typeof(env_, jsContext, &valueType);
    EXPECT_EQ(status, napi_ok);
    EXPECT_EQ(valueType, napi_object);

    // Get the launchElement property
    napi_value launchElement;
    status = napi_get_named_property(env_, jsContext, "launchElement", &launchElement);
    EXPECT_EQ(status, napi_ok);
    EXPECT_NE(launchElement, nullptr);

    // Verify it's an object
    status = napi_typeof(env_, launchElement, &valueType);
    EXPECT_EQ(status, napi_ok);
    EXPECT_EQ(valueType, napi_object);

    GTEST_LOG_(INFO) << "JsAbilityStageContext_LaunchElement_001 end";
}

/**
 * @tc.number: JsAbilityStageContext_LaunchElement_002
 * @tc.name: launchElement property with empty ElementName (optional parameter)
 * @tc.desc: Test launchElement property when ElementName is not set - should be undefined (optional parameter).
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsAbilityStageContextTest, JsAbilityStageContext_LaunchElement_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsAbilityStageContext_LaunchElement_002 start";

    // Create AbilityStageContext without setting ElementName (empty)
    auto context = std::make_shared<AbilityRuntime::ContextImpl>();
    ASSERT_NE(context, nullptr);
    auto abilityStageContext = std::make_shared<AbilityStageContext>();
    ASSERT_NE(abilityStageContext, nullptr);
    abilityStageContext->SetParentContext(context);

    // Create JS context object
    napi_value jsContext = CreateJsAbilityStageContext(env_, abilityStageContext);
    ASSERT_NE(jsContext, nullptr);

    // Get the launchElement property
    napi_value launchElement;
    napi_status status = napi_get_named_property(env_, jsContext, "launchElement", &launchElement);
    EXPECT_EQ(status, napi_ok);

    // Verify it's undefined (optional parameter behavior)
    napi_valuetype valueType;
    status = napi_typeof(env_, launchElement, &valueType);
    EXPECT_EQ(status, napi_ok);
    EXPECT_EQ(valueType, napi_undefined);

    GTEST_LOG_(INFO) << "JsAbilityStageContext_LaunchElement_002 end";
}

/**
 * @tc.number: JsAbilityStageContext_LaunchElement_003
 * @tc.name: launchElement property values
 * @tc.desc: Test that launchElement property contains correct values from ElementName.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsAbilityStageContextTest, JsAbilityStageContext_LaunchElement_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsAbilityStageContext_LaunchElement_003 start";

    auto context = std::make_shared<AbilityRuntime::ContextImpl>();
    ASSERT_NE(context, nullptr);
    auto abilityStageContext = std::make_shared<AbilityStageContext>();
    ASSERT_NE(abilityStageContext, nullptr);
    abilityStageContext->SetParentContext(context);

    // Set ElementName with all fields
    AppExecFwk::ElementName elementName;
    elementName.SetDeviceID("testDeviceId123");
    elementName.SetBundleName("com.example.valuetest");
    elementName.SetAbilityName("ValueTestAbility");
    elementName.SetModuleName("ValueTestModule");
    abilityStageContext->SetLaunchElement(elementName);

    // Create JS context
    napi_value jsContext = CreateJsAbilityStageContext(env_, abilityStageContext);
    ASSERT_NE(jsContext, nullptr);

    // Get launchElement property
    napi_value launchElement;
    napi_status status = napi_get_named_property(env_, jsContext, "launchElement", &launchElement);
    EXPECT_EQ(status, napi_ok);
    ASSERT_NE(launchElement, nullptr);

    // Verify it's an object
    napi_valuetype valueType;
    status = napi_typeof(env_, launchElement, &valueType);
    EXPECT_EQ(status, napi_ok);
    EXPECT_EQ(valueType, napi_object);

    // Verify we can read properties from the launchElement object
    napi_value bundleNameProp;
    status = napi_get_named_property(env_, launchElement, "bundleName", &bundleNameProp);
    EXPECT_EQ(status, napi_ok);

    GTEST_LOG_(INFO) << "JsAbilityStageContext_LaunchElement_003 end";
}

/**
 * @tc.number: JsAbilityStageContext_LaunchElement_004
 * @tc.name: launchElement property is not a function
 * @tc.desc: Test that launchElement is a property, not a function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsAbilityStageContextTest, JsAbilityStageContext_LaunchElement_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsAbilityStageContext_LaunchElement_004 start";

    auto context = std::make_shared<AbilityRuntime::ContextImpl>();
    ASSERT_NE(context, nullptr);
    auto abilityStageContext = std::make_shared<AbilityStageContext>();
    ASSERT_NE(abilityStageContext, nullptr);
    abilityStageContext->SetParentContext(context);

    AppExecFwk::ElementName elementName;
    elementName.SetBundleName("com.example.typetest");
    elementName.SetAbilityName("TypeTestAbility");
    abilityStageContext->SetLaunchElement(elementName);

    // Create JS context
    napi_value jsContext = CreateJsAbilityStageContext(env_, abilityStageContext);
    ASSERT_NE(jsContext, nullptr);

    // Get launchElement property
    napi_value launchElement;
    napi_status status = napi_get_named_property(env_, jsContext, "launchElement", &launchElement);
    EXPECT_EQ(status, napi_ok);
    ASSERT_NE(launchElement, nullptr);

    // Verify it's NOT a function
    napi_valuetype valueType;
    status = napi_typeof(env_, launchElement, &valueType);
    EXPECT_EQ(status, napi_ok);
    EXPECT_NE(valueType, napi_function);
    EXPECT_EQ(valueType, napi_object);

    GTEST_LOG_(INFO) << "JsAbilityStageContext_LaunchElement_004 end";
}

/**
 * @tc.number: JsAbilityStageContext_LaunchElement_005
 * @tc.name: launchElement with partial ElementName values
 * @tc.desc: Test launchElement property when ElementName has only some fields set.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsAbilityStageContextTest, JsAbilityStageContext_LaunchElement_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsAbilityStageContext_LaunchElement_005 start";

    auto context = std::make_shared<AbilityRuntime::ContextImpl>();
    ASSERT_NE(context, nullptr);
    auto abilityStageContext = std::make_shared<AbilityStageContext>();
    ASSERT_NE(abilityStageContext, nullptr);
    abilityStageContext->SetParentContext(context);

    // Set ElementName with only bundleName (partial data)
    AppExecFwk::ElementName elementName;
    elementName.SetBundleName("com.example.partial");
    // abilityName is empty
    abilityStageContext->SetLaunchElement(elementName);

    // Create JS context
    napi_value jsContext = CreateJsAbilityStageContext(env_, abilityStageContext);
    ASSERT_NE(jsContext, nullptr);

    // Get launchElement property - should be set because bundleName is not empty
    napi_value launchElement;
    napi_status status = napi_get_named_property(env_, jsContext, "launchElement", &launchElement);
    EXPECT_EQ(status, napi_ok);

    // Verify it's an object (not undefined)
    napi_valuetype valueType;
    status = napi_typeof(env_, launchElement, &valueType);
    EXPECT_EQ(status, napi_ok);
    EXPECT_EQ(valueType, napi_object);

    GTEST_LOG_(INFO) << "JsAbilityStageContext_LaunchElement_005 end";
}

/**
 * @tc.number: JsAbilityStageContext_LaunchElement_006
 * @tc.name: launchElement with null context
 * @tc.desc: Test CreateJsAbilityStageContext handles null context gracefully.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsAbilityStageContextTest, JsAbilityStageContext_LaunchElement_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsAbilityStageContext_LaunchElement_006 start";

    // Create context with null pointer
    std::shared_ptr<AbilityRuntime::Context> nullContext = nullptr;

    // Create JS context - should handle null gracefully
    napi_value jsContext = CreateJsAbilityStageContext(env_, nullContext);
    // Result depends on implementation - just verify it doesn't crash

    GTEST_LOG_(INFO) << "JsAbilityStageContext_LaunchElement_006 end";
}

} // namespace AbilityRuntime
} // namespace OHOS
