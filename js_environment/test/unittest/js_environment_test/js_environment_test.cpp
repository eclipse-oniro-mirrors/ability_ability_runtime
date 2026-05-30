/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#define private public
#include "js_environment.h"
#undef private

#include <gtest/gtest.h>
#include <gtest/hwext/gtest-multithread.h>
#include <cstdarg>
#include <string>

#include "ecmascript/napi/include/jsnapi.h"
#include "ohos_js_env_logger.h"
#include "ohos_js_environment_impl.h"
#include "worker_info.h"

using namespace testing;
using namespace testing::ext;
using namespace testing::mt;

namespace {
bool callbackModuleFlag;
}

namespace OHOS {
namespace JsEnv {
class JsEnvironmentTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void JsEnvironmentTest::SetUpTestCase()
{
    AbilityRuntime::OHOSJsEnvLogger::RegisterJsEnvLogger();
}

void JsEnvironmentTest::TearDownTestCase()
{}

void JsEnvironmentTest::SetUp()
{}

void JsEnvironmentTest::TearDown()
{}

namespace {
void CallBackModuleFunc()
{
    callbackModuleFlag = true;
}
}

/**
 * @tc.name: JsEnvInitialize_0100
 * @tc.desc: Initialize js environment.
 * @tc.type: FUNC
 * @tc.require: issueI6KODF
 */
HWTEST_F(JsEnvironmentTest, JsEnvInitialize_0100, TestSize.Level0)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    ASSERT_NE(jsEnv, nullptr);
    ASSERT_EQ(jsEnv->GetVM(), nullptr);
    ASSERT_EQ(jsEnv->GetNativeEngine(), nullptr);

    panda::RuntimeOption pandaOption;
    auto ret = jsEnv->Initialize(pandaOption, static_cast<void*>(this));
    ASSERT_EQ(ret, true);

    auto vm = jsEnv->GetVM();
    EXPECT_NE(vm, nullptr);

    auto nativeEngine = jsEnv->GetNativeEngine();
    EXPECT_NE(nativeEngine, nullptr);
}

/**
 * @tc.name: JsEnvInitialize_0200
 * @tc.desc: Initialize js environment in multi thread.
 * @tc.type: FUNC
 * @tc.require: issueI6KODF
 */
HWTEST_F(JsEnvironmentTest, JsEnvInitialize_0200, TestSize.Level0)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    ASSERT_NE(jsEnv, nullptr);

    panda::RuntimeOption pandaOption;
    ASSERT_EQ(jsEnv->Initialize(pandaOption, nullptr), true);
    EXPECT_NE(jsEnv->GetVM(), nullptr);
    EXPECT_NE(jsEnv->GetNativeEngine(), nullptr);
}

/**
 * @tc.name: LoadScript_0100
 * @tc.desc: load script with invalid engine.
 * @tc.type: FUNC
 * @tc.require: issueI6KODF
 */
HWTEST_F(JsEnvironmentTest, LoadScript_0100, TestSize.Level2)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    ASSERT_NE(jsEnv, nullptr);

    EXPECT_EQ(jsEnv->LoadScript(""), false);
}

/**
 * @tc.name: LoadScript_0200
 * @tc.desc: load script with specify path.
 * @tc.type: FUNC
 * @tc.require: issueI6KODF
 */
HWTEST_F(JsEnvironmentTest, LoadScript_0200, TestSize.Level2)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    ASSERT_NE(jsEnv, nullptr);

    panda::RuntimeOption pandaOption;
    auto ret = jsEnv->Initialize(pandaOption, static_cast<void*>(this));
    ASSERT_EQ(ret, true);

    EXPECT_EQ(jsEnv->LoadScript("/system/etc/strip.native.min.abc"), true);
}

/**
 * @tc.name: JsEnvInitTimerModule_0100
 * @tc.desc: Initialize timer module.
 * @tc.type: FUNC
 * @tc.require: issueI6Z5M5
 */
HWTEST_F(JsEnvironmentTest, JsEnvInitTimerModule_0100, TestSize.Level2)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    ASSERT_NE(jsEnv, nullptr);

    // Init timer module when native engine is invalid.
    jsEnv->InitTimerModule();

    panda::RuntimeOption pandaOption;
    auto ret = jsEnv->Initialize(pandaOption, static_cast<void*>(this));
    ASSERT_EQ(ret, true);

    // Init timer module when native engine has created.
    jsEnv->InitTimerModule();
}

/**
 * @tc.name: PostTask_0100
 * @tc.desc: PostTask
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(JsEnvironmentTest, PostTask_0100, TestSize.Level2)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    ASSERT_NE(jsEnv, nullptr);

    // Init timer module when native engine is invalid.
    std::function<void()> task = CallBackModuleFunc;
    std::string name = "NAME";
    int64_t delayTime = 10;
    jsEnv->PostTask(task, name, delayTime);
}

/**
 * @tc.name: RemoveTask_0100
 * @tc.desc: RemoveTask
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(JsEnvironmentTest, RemoveTask_0100, TestSize.Level2)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    ASSERT_NE(jsEnv, nullptr);

    std::string name = "NAME";
    jsEnv->RemoveTask(name);
}

/**
 * @tc.name: InitSyscapModule_0100
 * @tc.desc: InitSyscapModule
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(JsEnvironmentTest, InitSyscapModule_0100, TestSize.Level2)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    ASSERT_NE(jsEnv, nullptr);

    jsEnv->InitSyscapModule();
}

/**
 * @tc.name: RegisterUncaughtExceptionHandler_0100
 * @tc.desc: RegisterUncaughtExceptionHandler
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(JsEnvironmentTest, RegisterUncaughtExceptionHandler_0100, TestSize.Level2)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    ASSERT_NE(jsEnv, nullptr);

    JsEnv::UncaughtExceptionInfo uncaughtExceptionInfo;
    jsEnv->RegisterUncaughtExceptionHandler(uncaughtExceptionInfo);
}

/**
 * @tc.name: StartDebugger_0100
 * @tc.desc: StartDebugger
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(JsEnvironmentTest, StartDebugger_0100, TestSize.Level2)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    ASSERT_NE(jsEnv, nullptr);

    std::string option = "ark:1234@Debugger";
    uint32_t socketFd = 10;
    bool isDebugApp = true;
    bool result = jsEnv->StartDebugger(option, socketFd, isDebugApp);
    ASSERT_EQ(result, false);
}

/**
 * @tc.name: StartDebugger_0200
 * @tc.desc: StartDebugger
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(JsEnvironmentTest, StartDebugger_0200, TestSize.Level2)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    ASSERT_NE(jsEnv, nullptr);
    panda::RuntimeOption pandaOption;
    auto ret = jsEnv->Initialize(pandaOption, static_cast<void*>(this));
    ASSERT_EQ(ret, true);

    std::string option = "ark:1234@Debugger";
    uint32_t socketFd = 10;
    bool isDebugApp = true;
    bool result = jsEnv->StartDebugger(option, socketFd, isDebugApp);
    ASSERT_EQ(result, false);
}

/**
 * @tc.name: StopDebugger_0100
 * @tc.desc: StopDebugger
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(JsEnvironmentTest, StopDebugger_0100, TestSize.Level2)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    ASSERT_NE(jsEnv, nullptr);

    jsEnv->StopDebugger();
}

/**
 * @tc.name: StopDebugger_0200
 * @tc.desc: StopDebugger
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(JsEnvironmentTest, StopDebugger_0200, TestSize.Level2)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());

    panda::RuntimeOption pandaOption;
    jsEnv->Initialize(pandaOption, static_cast<void*>(this));
    jsEnv->StopDebugger();
    ASSERT_NE(jsEnv, nullptr);
}

/**
 * @tc.name: StopDebugger_0300
 * @tc.desc: StopDebugger
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(JsEnvironmentTest, StopDebugger_0300, TestSize.Level2)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());

    panda::RuntimeOption pandaOption;
    jsEnv->Initialize(pandaOption, static_cast<void*>(this));
    std::string option = "ark:1234@Debugger";
    jsEnv->StopDebugger(option);
    ASSERT_NE(jsEnv, nullptr);
}

/**
 * @tc.name: InitConsoleModule_0100
 * @tc.desc: InitConsoleModule
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(JsEnvironmentTest, InitConsoleModule_0100, TestSize.Level2)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    ASSERT_NE(jsEnv, nullptr);

    jsEnv->InitConsoleModule();

    panda::RuntimeOption pandaOption;
    auto ret = jsEnv->Initialize(pandaOption, static_cast<void*>(this));
    ASSERT_EQ(ret, true);

    jsEnv->InitConsoleModule();
}

/**
 * @tc.name: StartProfiler_0100
 * @tc.desc: StartProfiler
 * @tc.type: FUNC
 */
HWTEST_F(JsEnvironmentTest, StartProfiler_0100, TestSize.Level1)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    ASSERT_NE(jsEnv, nullptr);

    const char* libraryPath = "LIBRARYPATH";
    jsEnv->StartProfiler(libraryPath, 0, JsEnvironment::PROFILERTYPE::PROFILERTYPE_CPU, 0, 0, true);
    ASSERT_EQ(jsEnv->GetVM(), nullptr);
}

/**
 * @tc.name: StartProfiler_0200
 * @tc.desc: StartProfiler
 * @tc.type: FUNC
 */
HWTEST_F(JsEnvironmentTest, StartProfiler_0200, TestSize.Level1)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    ASSERT_NE(jsEnv, nullptr);

    panda::RuntimeOption pandaOption;
    auto ret = jsEnv->Initialize(pandaOption, static_cast<void*>(this));
    ASSERT_EQ(ret, true);

    const char* libraryPath = "LIBRARYPATH";
    jsEnv->StartProfiler(libraryPath, 0, JsEnvironment::PROFILERTYPE::PROFILERTYPE_HEAP, 0, 0, true);
    ASSERT_NE(jsEnv->GetVM(), nullptr);
}

/**
 * @tc.name: PostSyncTask_0100
 * @tc.desc: Js environment post sync task.
 * @tc.type: FUNC
 * @tc.require: issueI7C87T
 */
HWTEST_F(JsEnvironmentTest, PostSyncTask_0100, TestSize.Level2)
{
    auto runner = AppExecFwk::EventRunner::Create("TASK_RUNNER");
    ASSERT_NE(runner, nullptr);
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>(runner));
    ASSERT_NE(jsEnv, nullptr);
    panda::RuntimeOption pandaOption;
    ASSERT_EQ(jsEnv->Initialize(pandaOption, static_cast<void*>(this)), true);
    ASSERT_EQ(jsEnv->InitLoop(), true);

    std::string taskName = "syncTask001";
    bool taskExecuted = false;
    auto task = [taskName, &taskExecuted]() {
        taskExecuted = true;
    };
    jsEnv->PostSyncTask(task, taskName);
    EXPECT_EQ(taskExecuted, true);
}

/**
 * @tc.name: SetRequestAotCallback_0100
 * @tc.desc: Js environment SetRequestAotCallback.
 * @tc.type: FUNC
 * @tc.require: issueI82L1A
 */
HWTEST_F(JsEnvironmentTest, SetRequestAotCallback_0100, TestSize.Level2)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    ASSERT_NE(jsEnv, nullptr);

    auto callback = [](const std::string& bundleName, const std::string& moduleName, int32_t triggerMode) -> int32_t {
        return 0;
    };
    jsEnv->SetRequestAotCallback(callback);
}

/**
 * @tc.name: ParseHdcRegisterOption_0100
 * @tc.desc: Js environment ParseHdcRegisterOption.
 * @tc.type: FUNC
 */
HWTEST_F(JsEnvironmentTest, ParseHdcRegisterOption_0100, TestSize.Level2)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    ASSERT_NE(jsEnv, nullptr);
    std::string option1 = "";
    int result1 = jsEnv->ParseHdcRegisterOption(option1);
    ASSERT_EQ(result1, -1);
    std::string option2 = "@";
    int result2 = jsEnv->ParseHdcRegisterOption(option2);
    ASSERT_EQ(result2, -1);
    std::string option3 = ":";
    int result3 = jsEnv->ParseHdcRegisterOption(option3);
    ASSERT_EQ(result3, -1);
    std::string option4 = "ark:123@Debugger";
    int result4 = jsEnv->ParseHdcRegisterOption(option4);
    ASSERT_EQ(result4, 123);
    std::string option5 = "ark:123@456@Debugger";
    int result5 = jsEnv->ParseHdcRegisterOption(option5);
    ASSERT_EQ(result5, 456);
}

/**
 * @tc.name: SetDeviceDisconnectCallback_0100
 * @tc.desc: Js environment SetDeviceDisconnectCallback.
 * @tc.type: FUNC
 * @tc.require: issueI82L1A
 */
HWTEST_F(JsEnvironmentTest, SetDeviceDisconnectCallback_0100, TestSize.Level2)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    ASSERT_NE(jsEnv, nullptr);
    panda::RuntimeOption pandaOption;
    auto ret = jsEnv->Initialize(pandaOption, static_cast<void*>(this));
    ASSERT_EQ(ret, true);

    bool taskExecuted = false;
    auto task = [&taskExecuted]() {
        return true;
    };
    jsEnv->SetDeviceDisconnectCallback(task);
    ASSERT_EQ(taskExecuted, false);
}

/**
 * @tc.name: DestroyHeapProfiler_0100
 * @tc.desc: Js environment DestroyHeapProfiler.
 * @tc.type: FUNC
 */
HWTEST_F(JsEnvironmentTest, DestroyHeapProfiler_0100, TestSize.Level2)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    jsEnv->DestroyHeapProfiler();
    ASSERT_NE(jsEnv, nullptr);
}

/**
 * @tc.name: NotifyDebugMode_0100
 * @tc.desc: Js environment NotifyDebugMode.
 * @tc.type: FUNC
 */
HWTEST_F(JsEnvironmentTest, NotifyDebugMode_0100, TestSize.Level2)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    int tid = 1;
    char* libraryPath;
    uint32_t instanceId = 1;
    bool debug = true;
    bool debugMode = true;
    jsEnv->NotifyDebugMode(tid, libraryPath, instanceId, debug, debugMode);
    ASSERT_NE(jsEnv, nullptr);
}

/**
 * @tc.name: GetDebuggerPostTask_0100
 * @tc.desc: Js environment GetDebuggerPostTask.
 * @tc.type: FUNC
 */
HWTEST_F(JsEnvironmentTest, GetDebuggerPostTask_0100, TestSize.Level2)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    jsEnv->GetDebuggerPostTask();
    ASSERT_NE(jsEnv, nullptr);
}

/**
 * @tc.name: GetDebuggerPostTask_0200
 * @tc.desc: Js environment GetDebuggerPostTask.
 * @tc.type: FUNC
 */
HWTEST_F(JsEnvironmentTest, GetDebuggerPostTask_0200, TestSize.Level2)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    auto poster = jsEnv->GetDebuggerPostTask();
    ASSERT_NE(jsEnv, nullptr);
    poster([]() {
        std::string temp;
    });
}

/**
 * @tc.name: GetHeapPrepare_0100
 * @tc.desc: Js environment GetHeapPrepare.
 * @tc.type: FUNC
 */
HWTEST_F(JsEnvironmentTest, GetHeapPrepare_0100, TestSize.Level2)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    jsEnv->GetHeapPrepare();
    ASSERT_NE(jsEnv, nullptr);
}

/**
 * @tc.name: GetHeapPrepare_0200
 * @tc.desc: Js environment GetHeapPrepare.
 * @tc.type: FUNC
 */
HWTEST_F(JsEnvironmentTest, GetHeapPrepare_0200, TestSize.Level2)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    panda::RuntimeOption pandaOption;
    jsEnv->Initialize(pandaOption, static_cast<void*>(this));
    jsEnv->GetHeapPrepare();
    ASSERT_NE(jsEnv, nullptr);
}

/**
 * @tc.name: GetSourceMapOperator_0100
 * @tc.desc: Js environment GetSourceMapOperator.
 * @tc.type: FUNC
 */
HWTEST_F(JsEnvironmentTest, GetSourceMapOperator_0100, TestSize.Level2)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    jsEnv->GetSourceMapOperator();
    ASSERT_NE(jsEnv, nullptr);
}

/**
 * @tc.name: initworkermodule_0100
 * @tc.desc: Js environment initworkermodule.
 * @tc.type: FUNC
 */
HWTEST_F(JsEnvironmentTest, initworkermodule_0100, TestSize.Level2)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    std::shared_ptr<WorkerInfo> workerInfo = std::make_shared<WorkerInfo>();
    jsEnv->InitWorkerModule(workerInfo);
    ASSERT_NE(jsEnv, nullptr);
}

/**
 * @tc.name: InitSourceMap_0100
 * @tc.desc: Js environment InitSourceMap.
 * @tc.type: FUNC
 */
HWTEST_F(JsEnvironmentTest, InitSourceMap_0100, TestSize.Level2)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    std::shared_ptr<JsEnv::SourceMapOperator> operatorObj = nullptr;
    jsEnv->InitSourceMap(operatorObj);
    ASSERT_NE(jsEnv, nullptr);
}

/**
 * @tc.name: DeInitLoop_0100
 * @tc.desc: Js environment DeInitLoop.
 * @tc.type: FUNC
 */
HWTEST_F(JsEnvironmentTest, DeInitLoop_0100, TestSize.Level2)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    jsEnv->DeInitLoop();
    ASSERT_NE(jsEnv, nullptr);
}

/**
 * @tc.name: SetModuleLoadChecker_0100
 * @tc.desc: Js environment SetModuleLoadChecker.
 * @tc.type: FUNC
 */
HWTEST_F(JsEnvironmentTest, SetModuleLoadChecker_0100, TestSize.Level2)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    std::shared_ptr<ModuleCheckerDelegate> moduleCheckerDelegate = nullptr;
    jsEnv->SetModuleLoadChecker(moduleCheckerDelegate);
    ASSERT_NE(jsEnv, nullptr);
}

/**
 * @tc.name: ReInitJsEnvImpl_0100
 * @tc.desc: Js environment ReInitJsEnvImpl.
 * @tc.type: FUNC
 */
HWTEST_F(JsEnvironmentTest, ReInitJsEnvImpl_0100, TestSize.Level2)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    jsEnv->ReInitJsEnvImpl(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    ASSERT_NE(jsEnv->impl_, nullptr);
}

/**
 * @tc.name: GetDebugMode_0100
 * @tc.desc: Js environment GetDebugMode.
 * @tc.type: FUNC
 */
HWTEST_F(JsEnvironmentTest, GetDebugMode_0100, TestSize.Level2)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    auto result = jsEnv->GetDebugMode();
    ASSERT_EQ(result, false);
}

/**
 * @tc.name: RegisterUncatchableExceptionHandler_0100
 * @tc.desc: Js environment RegisterUncatchableExceptionHandler.
 * @tc.type: FUNC
 */
HWTEST_F(JsEnvironmentTest, RegisterUncatchableExceptionHandler_0100, TestSize.Level1)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    jsEnv->RegisterUncatchableExceptionHandler(nullptr);
    EXPECT_NE(jsEnv, nullptr);
    UncatchableTask task;
    jsEnv->RegisterUncatchableExceptionHandler(task);
    EXPECT_NE(jsEnv, nullptr);
}

/**
 * @tc.name: AppendSoLoadFailure_0100
 * @tc.desc: Test AppendSoLoadFailure when nativeModuleErrorInfo is absent.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(JsEnvironmentTest, AppendSoLoadFailure_0100, TestSize.Level2)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    ASSERT_NE(jsEnv, nullptr);
    panda::RuntimeOption pandaOption;
    ASSERT_EQ(jsEnv->Initialize(pandaOption, static_cast<void*>(this)), true);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    ASSERT_NE(env, nullptr);

    // Provide minimal valid error properties so CallbackTask reaches AppendSoLoadFailure.
    napi_value object = nullptr;
    ASSERT_EQ(napi_create_object(env, &object), napi_ok);
    ASSERT_NE(object, nullptr);
    std::string errorMsg = "load native module failed";
    std::string errorName = "BusinessError";
    std::string errorStack = "BusinessError: load native module failed\n    at test.js:1:1";
    napi_value nativeErrorMsg = nullptr;
    napi_value nativeErrorName = nullptr;
    napi_value nativeErrorStack = nullptr;
    napi_create_string_utf8(env, errorMsg.c_str(), errorMsg.length(), &nativeErrorMsg);
    napi_create_string_utf8(env, errorName.c_str(), errorName.length(), &nativeErrorName);
    napi_create_string_utf8(env, errorStack.c_str(), errorStack.length(), &nativeErrorStack);
    napi_set_named_property(env, object, "message", nativeErrorMsg);
    napi_set_named_property(env, object, "name", nativeErrorName);
    napi_set_named_property(env, object, "stack", nativeErrorStack);
    // nativeModuleErrorInfo is intentionally not set.

    std::string capturedSummary;
    auto task = [&capturedSummary](std::string summary, const JsEnv::ErrorObject errorObj,
        napi_env env, napi_value exception) { capturedSummary = summary; };
    NapiUncaughtExceptionCallback callback(task, nullptr, env);
    callback(object);

    // The callback ran and produced a summary, but no NativeModuleErrorInfo section is appended.
    EXPECT_FALSE(capturedSummary.empty());
    EXPECT_EQ(capturedSummary.find("NativeModuleErrorInfo:"), std::string::npos);
}

/**
 * @tc.name: AppendSoLoadFailure_0200
 * @tc.desc: Test AppendSoLoadFailure with a single line of nativeModuleErrorInfo.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(JsEnvironmentTest, AppendSoLoadFailure_0200, TestSize.Level2)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    ASSERT_NE(jsEnv, nullptr);
    panda::RuntimeOption pandaOption;
    ASSERT_EQ(jsEnv->Initialize(pandaOption, static_cast<void*>(this)), true);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    ASSERT_NE(env, nullptr);

    napi_value object = nullptr;
    ASSERT_EQ(napi_create_object(env, &object), napi_ok);
    ASSERT_NE(object, nullptr);
    std::string errorStack = "BusinessError: load native module failed\n    at test.js:1:1";
    std::string soLoadFailureInfo = "Failed to dlopen libexample.so: cannot find symbol";
    napi_value nativeErrorStack = nullptr;
    napi_value nativeSoLoadFailure = nullptr;
    napi_create_string_utf8(env, errorStack.c_str(), errorStack.length(), &nativeErrorStack);
    napi_create_string_utf8(env, soLoadFailureInfo.c_str(), soLoadFailureInfo.length(), &nativeSoLoadFailure);
    napi_set_named_property(env, object, "stack", nativeErrorStack);
    napi_set_named_property(env, object, "nativeModuleErrorInfo", nativeSoLoadFailure);

    std::string capturedSummary;
    auto task = [&capturedSummary](std::string summary, const JsEnv::ErrorObject errorObj,
        napi_env env, napi_value exception) { capturedSummary = summary; };
    NapiUncaughtExceptionCallback callback(task, nullptr, env);
    callback(object);

    // Only one line: printed as-is with no numbering.
    EXPECT_NE(capturedSummary.find("NativeModuleErrorInfo:\n" + soLoadFailureInfo + "\n"), std::string::npos);
    EXPECT_EQ(capturedSummary.find("#1 "), std::string::npos);
}

/**
 * @tc.name: AppendSoLoadFailure_0300
 * @tc.desc: Test AppendSoLoadFailure with multiple lines of nativeModuleErrorInfo.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(JsEnvironmentTest, AppendSoLoadFailure_0300, TestSize.Level2)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    ASSERT_NE(jsEnv, nullptr);
    panda::RuntimeOption pandaOption;
    ASSERT_EQ(jsEnv->Initialize(pandaOption, static_cast<void*>(this)), true);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    ASSERT_NE(env, nullptr);

    napi_value object = nullptr;
    ASSERT_EQ(napi_create_object(env, &object), napi_ok);
    ASSERT_NE(object, nullptr);
    std::string errorStack = "BusinessError: load native module failed\n    at test.js:1:1";
    // The first line is the header; the remaining lines must be numbered starting from #1.
    std::string firstLine = "NativeModule load failure summary";
    std::string secondLine = "libfoo.so: dlopen failed";
    std::string thirdLine = "libbar.so: symbol not found";
    std::string soLoadFailureInfo = firstLine + "\n" + secondLine + "\n" + thirdLine;
    napi_value nativeErrorStack = nullptr;
    napi_value nativeSoLoadFailure = nullptr;
    napi_create_string_utf8(env, errorStack.c_str(), errorStack.length(), &nativeErrorStack);
    napi_create_string_utf8(env, soLoadFailureInfo.c_str(), soLoadFailureInfo.length(), &nativeSoLoadFailure);
    napi_set_named_property(env, object, "stack", nativeErrorStack);
    napi_set_named_property(env, object, "nativeModuleErrorInfo", nativeSoLoadFailure);

    std::string capturedSummary;
    auto task = [&capturedSummary](std::string summary, const JsEnv::ErrorObject errorObj,
        napi_env env, napi_value exception) { capturedSummary = summary; };
    NapiUncaughtExceptionCallback callback(task, nullptr, env);
    callback(object);

    EXPECT_NE(capturedSummary.find("NativeModuleErrorInfo:\n" + firstLine + "\n"), std::string::npos);
    EXPECT_NE(capturedSummary.find("#1 " + secondLine + "\n"), std::string::npos);
    EXPECT_NE(capturedSummary.find("#2 " + thirdLine + "\n"), std::string::npos);
}

/**
 * @tc.name: AppendSoLoadFailure_0400
 * @tc.desc: Test AppendSoLoadFailure skips empty lines and keeps numbering continuous.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(JsEnvironmentTest, AppendSoLoadFailure_0400, TestSize.Level2)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    ASSERT_NE(jsEnv, nullptr);
    panda::RuntimeOption pandaOption;
    ASSERT_EQ(jsEnv->Initialize(pandaOption, static_cast<void*>(this)), true);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    ASSERT_NE(env, nullptr);

    napi_value object = nullptr;
    ASSERT_EQ(napi_create_object(env, &object), napi_ok);
    ASSERT_NE(object, nullptr);
    std::string errorStack = "BusinessError: load native module failed\n    at test.js:1:1";
    // An empty line is embedded between the header and a detail line.
    std::string firstLine = "NativeModule load failure summary";
    std::string detailLine = "libfoo.so: dlopen failed";
    std::string soLoadFailureInfo = firstLine + "\n" + "\n" + detailLine;
    napi_value nativeErrorStack = nullptr;
    napi_value nativeSoLoadFailure = nullptr;
    napi_create_string_utf8(env, errorStack.c_str(), errorStack.length(), &nativeErrorStack);
    napi_create_string_utf8(env, soLoadFailureInfo.c_str(), soLoadFailureInfo.length(), &nativeSoLoadFailure);
    napi_set_named_property(env, object, "stack", nativeErrorStack);
    napi_set_named_property(env, object, "nativeModuleErrorInfo", nativeSoLoadFailure);

    std::string capturedSummary;
    auto task = [&capturedSummary](std::string summary, const JsEnv::ErrorObject errorObj,
        napi_env env, napi_value exception) { capturedSummary = summary; };
    NapiUncaughtExceptionCallback callback(task, nullptr, env);
    callback(object);

    // The empty line is dropped and the detail line becomes the first numbered entry (#1).
    EXPECT_NE(capturedSummary.find("NativeModuleErrorInfo:\n" + firstLine + "\n"), std::string::npos);
    EXPECT_NE(capturedSummary.find("#1 " + detailLine + "\n"), std::string::npos);
    EXPECT_EQ(capturedSummary.find("#2 "), std::string::npos);
}
} // namespace JsEnv
} // namespace OHOS
