/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include <gtest/hwext/gtest-multithread.h>

#include "ark_native_engine.h"
#define private public
#define protected public
#include "js_environment.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "js_worker.h"
#undef private
#undef protected
#include "event_runner.h"
#include "mock_js_runtime.h"
#include "mock_jsnapi.h"
#include "hilog_tag_wrapper.h"
#include "js_runtime_lite.h"
#include "file_mapper.h"

using namespace testing;
using namespace testing::ext;
using namespace testing::mt;

namespace OHOS {
namespace AbilityRuntime {
namespace {
const std::string TEST_BUNDLE_NAME = "com.ohos.contactsdataability";
const std::string TEST_MODULE_NAME = ".ContactsDataAbility";
const std::string TEST_ABILITY_NAME = "ContactsDataAbility";
const std::string TEST_CODE_PATH = "/data/storage/el1/bundle";
const std::string TEST_HAP_PATH = "/system/app/com.ohos.contactsdataability/Contacts_DataAbility.hap";
const std::string TEST_LIB_PATH = "/data/storage/el1/bundle/lib/";
const std::string TEST_MODULE_PATH = "/data/storage/el1/bundle/curJsModulePath";
}  // namespace
class JsRuntimeTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    static std::unique_ptr<AbilityRuntime::JsRuntime> jsRuntimePtr;
};

std::unique_ptr<AbilityRuntime::JsRuntime> JsRuntimeTest::jsRuntimePtr = nullptr;
Runtime::Options options_;

void JsRuntimeTest::SetUpTestCase()
{
    options_.bundleName = TEST_BUNDLE_NAME;
    options_.codePath = TEST_CODE_PATH;
    options_.loadAce = false;
    options_.isBundle = true;
    options_.preload = false;
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner = AppExecFwk::EventRunner::Create(TEST_ABILITY_NAME);
    options_.eventRunner = eventRunner;
    jsRuntimePtr = AbilityRuntime::JsRuntime::Create(options_);
}

void JsRuntimeTest::TearDownTestCase()
{
    jsRuntimePtr.reset();
}

void JsRuntimeTest::SetUp()
{
}

void JsRuntimeTest::TearDown()
{
}

/**
 * @tc.name: JsperfProfilerCommandParse_100
 * @tc.desc: JsRuntime test for JsperfProfilerCommandParse.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsperfProfilerCommandParse_100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "JsperfProfilerCommandParse_100 start");
    auto jsRuntime = std::make_shared<AbilityRuntime::JsRuntime>();
    std::string command = "";
    constexpr int32_t defaultVal = 500;
    constexpr int32_t emptyVal = 0;
    ASSERT_EQ(jsRuntime->JsperfProfilerCommandParse(command, defaultVal), emptyVal);
    command = "jsperfabc";
    ASSERT_EQ(jsRuntime->JsperfProfilerCommandParse(command, defaultVal), defaultVal);
    command = "jsperf";
    ASSERT_EQ(jsRuntime->JsperfProfilerCommandParse(command, defaultVal), defaultVal);
    command = "jsperf ";
    ASSERT_EQ(jsRuntime->JsperfProfilerCommandParse(command, defaultVal), defaultVal);
    command = "jsperf 1000";
    ASSERT_NE(jsRuntime->JsperfProfilerCommandParse(command, defaultVal), defaultVal);
    command = " jsperf 1000";
    ASSERT_NE(jsRuntime->JsperfProfilerCommandParse(command, defaultVal), defaultVal);
    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "JsperfProfilerCommandParse_100 end");
}

/**
 * @tc.name: JsRuntimeTestCreate_0100
 * @tc.desc: JsRuntime Test for Create
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(JsRuntimeTest, JsRuntimeTestCreate_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "Create_0100 start");

    Runtime::Options options;
    options.preload = true;
    options.isStageModel = false;
    options.isTestFramework = false;
    JsRuntime jsRuntime;
    jsRuntime.preloaded_ = true;
    std::unique_ptr<JsRuntime> jsRuntimePtr1 = jsRuntime.Create(options);
    EXPECT_TRUE(jsRuntimePtr1 != nullptr);

    jsRuntimePtr1.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "Create_0100 end");
}

/**
 * @tc.name: JsRuntimeTestInit_0100
 * @tc.desc: JsRuntime Test for Init
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeTestInit_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "Init_0100 start");

    Runtime::Options options;
    options.isStageModel = false;
    options.isTestFramework = false;
    JsRuntime jsRuntime;
    bool isInit = jsRuntime.Init(options);
    EXPECT_EQ(isInit, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "Init_0100 end");
}

/**
 * @tc.name: JsRuntimeTestSetAppLibPath_0100
 * @tc.desc: JsRuntime Test for SetAppLibPath
 * @tc.type: FUNC
 * @tc.require: issueI581RO
 */
HWTEST_F(JsRuntimeTest, JsRuntimeTestSetAppLibPath_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "SetAppLibPath_0100 start");
    std::string appLibPathKey = TEST_BUNDLE_NAME + TEST_MODULE_NAME;
    std::string libPath = TEST_LIB_PATH;

    AppLibPathMap appLibPaths {};
    JsRuntime::SetAppLibPath(appLibPaths);

    appLibPaths[appLibPathKey].emplace_back(libPath);
    JsRuntime::SetAppLibPath(appLibPaths);
    EXPECT_NE(appLibPaths.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "SetAppLibPath_0100 end");
}

/**
 * @tc.name: JsRuntimeUtilsTest_0100
 * @tc.desc: JsRuntimeUtils Test
 * @tc.type: FUNC
 * @tc.require: issueI581RO
 */
HWTEST_F(JsRuntimeTest, JsRuntimeUtilsTest_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "UtilsTest_0100 start");

    auto env = (static_cast<AbilityRuntime::JsRuntime&>(*jsRuntimePtr)).GetNapiEnv();
    napi_ref callbackRef = nullptr;
    napi_create_reference(env, CreateJsUndefined(env), 1, &callbackRef);
    std::unique_ptr<NapiAsyncTask> task = std::make_unique<NapiAsyncTask>(callbackRef, nullptr, nullptr);
    task->ResolveWithNoError(env, CreateJsUndefined(env));
    EXPECT_TRUE(task->callbackRef_ == nullptr);

    napi_deferred nativeDeferred = nullptr;
    napi_value result;
    napi_create_promise(env, &nativeDeferred, &result);
    task = std::make_unique<NapiAsyncTask>(nativeDeferred, nullptr, nullptr);
    task->ResolveWithNoError(env, CreateJsUndefined(env));
    EXPECT_TRUE(task->deferred_ == nullptr);

    task->deferred_ = nullptr;
    task->callbackRef_ = nullptr;
    task->ResolveWithNoError(env, CreateJsUndefined(env));
    EXPECT_TRUE(task->deferred_ == nullptr);
    EXPECT_TRUE(task->callbackRef_ == nullptr);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "UtilsTest_0100 end");
}

/**
 * @tc.name: JsRuntimeGetLanguageTest_0100
 * @tc.desc: JsRuntime Test
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeGetLanguageTest_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "GetLanguageTest_0100 start");

    std::shared_ptr<JsRuntime> jsRuntime = std::make_shared<AbilityRuntime::JsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);
    Runtime::Language language = jsRuntime->GetLanguage();
    EXPECT_TRUE(language ==JsRuntime::Language::JS);
    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "GetLanguageTest_0100 end");
}

/**
 * @tc.name: JsRuntimeNotifyApplicationStateTest_0100
 * @tc.desc: JsRuntime test
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeNotifyApplicationStateTest_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyApplicationState start");

    bool isBackground = false;
    jsRuntimePtr->NotifyApplicationState(isBackground);
    ASSERT_TRUE(jsRuntimePtr != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "NotifyApplicationState end");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

/**
 * @tc.name: JsRuntimeNotifyApplicationStateTest_0200
 * @tc.desc: JsRuntime test for NotifyApplicationState when nativeEngine is not nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeNotifyApplicationStateTest_0200, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyApplicationState start");

    std::unique_ptr<JsRuntime> jsRuntime = std::make_unique<AbilityRuntime::JsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);
    bool isBackground = true;
    jsRuntime->NotifyApplicationState(isBackground);
    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    TAG_LOGI(AAFwkTag::TEST, "NotifyApplicationState end");
}

/**
 * @tc.name: JsRuntimeDumpHeapSnapshotTest_0100
 * @tc.desc: JsRuntime test for DumpHeapSnapshot.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeDumpHeapSnapshotTest_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "DumpHeapSnapshot start");
    std::unique_ptr<JsRuntime> jsRuntime = std::make_unique<AbilityRuntime::JsRuntime>();
    jsRuntime->DumpHeapSnapshot(false);
    EXPECT_TRUE(jsRuntime != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "DumpHeapSnapshot end");
}

/**
 * @tc.name: JsRuntimeGetHeapTotalSizeTest_0100
 * @tc.desc: JsRuntime test for GetHeapTotalSize.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeGetHeapTotalSizeTest_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "GetHeapTotalSize start");
    std::unique_ptr<JsRuntime> jsRuntime = std::make_unique<AbilityRuntime::JsRuntime>();
    size_t ret = jsRuntime->GetHeapTotalSize();
    EXPECT_TRUE(ret >= 0);
    TAG_LOGI(AAFwkTag::TEST, "GetHeapTotalSize end");
}

/**
 * @tc.name: JsRuntimeGetHeapObjectSizeTest_0100
 * @tc.desc: JsRuntime test for GetHeapObjectSize.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeGetHeapObjectSizeTest_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "GetHeapObjectSize start");
    std::unique_ptr<JsRuntime> jsRuntime = std::make_unique<AbilityRuntime::JsRuntime>();
    size_t ret = jsRuntime->GetHeapObjectSize();
    EXPECT_TRUE(ret >= 0);
    TAG_LOGI(AAFwkTag::TEST, "GetHeapObjectSize end");
}

/**
 * @tc.name: JsRuntimePreloadSystemModuleTest_0100
 * @tc.desc: JsRuntime test for PreloadSystemModule.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimePreloadSystemModuleTest_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadSystemModule start");

    std::unique_ptr<JsRuntime> jsRuntime = std::make_unique<AbilityRuntime::JsRuntime>();
    std::string moduleName = "PreloadSystemModuleTest";
    jsRuntime->PreloadSystemModule(moduleName);
    EXPECT_TRUE(jsRuntime != nullptr);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    TAG_LOGI(AAFwkTag::TEST, "PreloadSystemModule end");
}

/**
 * @tc.name: JsRuntimeRunSandboxScriptTest_0100
 * @tc.desc: JsRuntime test for RunSandboxScript.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeRunSandboxScriptTest_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "RunSandboxScript start");

    auto jsRuntime = std::make_unique<AbilityRuntime::JsRuntime>();
    std::string path = "";
    std::string hapPath = "";
    auto test = jsRuntime->RunSandboxScript(path, hapPath);
    EXPECT_EQ(test, false);
    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "RunSandboxScript end");
}

/**
 * @tc.name: JsRuntimeLoadSystemModuleByEngineTest_0100
 * @tc.desc: JsRuntime test for LoadSystemModuleByEngine.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeLoadSystemModuleByEngineTest_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "LoadSystemModuleByEngine start");

    std::string moduleName = "";
    std::unique_ptr<JsRuntime> jsRuntime = std::make_unique<AbilityRuntime::JsRuntime>();

    auto ref = jsRuntime->LoadSystemModuleByEngine(nullptr, moduleName, nullptr, 0);
    EXPECT_EQ(ref, nullptr);
    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "LoadSystemModuleByEngine end");
}

/**
 * @tc.name: JsRuntimeFinishPreloadTest_0100
 * @tc.desc: JsRuntime test for FinishPreload.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeFinishPreloadTest_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "FinishPreload start");

    auto jsRuntime = std::make_unique<JsRuntime>();

    jsRuntime->FinishPreload();
    EXPECT_TRUE(jsRuntime != nullptr);

    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "FinishPreload end");
}

/**
 * @tc.name: JsRuntimePostPreloadTest_0100
 * @tc.desc: JsRuntime test for FinishPreload.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimePostPreloadTest_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "PostPreload start");

    auto jsRuntime = std::make_unique<JsRuntime>();

    jsRuntime->PostPreload(options_);
    EXPECT_TRUE(jsRuntime != nullptr);

    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "PostPreload end");
}

/**
 * @tc.name: JsRuntimeLoadAotFileTest_0100
 * @tc.desc: JsRuntime test for LoadAotFile.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeLoadAotFileTest_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "LoadAotFile start");

    auto jsRuntime = std::make_unique<JsRuntime>();

    jsRuntime->LoadAotFile(options_);
    EXPECT_TRUE(jsRuntime != nullptr);

    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "LoadAotFile end");
}

/**
 * @tc.name: JsRuntimeLoadModuleTest_0100
 * @tc.desc: JsRuntime test for LoadModule.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeLoadModuleTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "LoadModule start");

    std::unique_ptr<JsRuntime> jsRuntime = std::make_unique<AbilityRuntime::JsRuntime>();

    std::string moduleName = TEST_MODULE_NAME;
    std::string modulePath = TEST_MODULE_PATH;
    std::string hapPath = TEST_HAP_PATH;
    auto ref = jsRuntimePtr->LoadModule(moduleName, modulePath, hapPath);
    EXPECT_EQ(ref, nullptr);
    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "LoadModule end");
}

/**
 * @tc.name: JsRuntimeLoadSystemModuleTest_0100
 * @tc.desc: JsRuntime test for LoadSystemModule (invoke the overwrite interface).
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeLoadSystemModuleTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "LoadSystemModule start");

    std::unique_ptr<JsRuntime> jsRuntime = std::make_unique<AbilityRuntime::JsRuntime>();
    std::string moduleName = TEST_MODULE_NAME;
    auto ref = jsRuntime->LoadSystemModule(moduleName, nullptr, 0);
    EXPECT_EQ(ref, nullptr);
    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "LoadSystemModule end");
}

/**
 * @tc.name: RuntimeSavePreloadedTest_0100
 * @tc.desc: Runtime test for SavePreloaded.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, RuntimeSavePreloadedTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SavePreloaded start");

    Runtime::SavePreloaded(nullptr);
    auto result = Runtime::GetPreloaded(Runtime::Language::JS);
    EXPECT_EQ(result, nullptr);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "SavePreloaded end");
}

/**
 * @tc.name: RuntimeSetModuleLoadCheckerTest_0100
 * @tc.desc: Runtime test for SetModuleLoadChecker.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, RuntimeSetModuleLoadCheckerTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetModuleLoadChecker start");

    std::unique_ptr<JsRuntime> jsRuntime = std::make_unique<AbilityRuntime::JsRuntime>();
    jsRuntime->SetModuleLoadChecker(nullptr);
    EXPECT_TRUE(jsRuntime != nullptr);
    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "SetModuleLoadChecker end");
}

/**
 * @tc.name: JsRuntimeSuspendVMTest_0100
 * @tc.desc: JsRuntime test for SuspendVM.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeSuspendVMTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SuspendVM start");

    auto jsRuntime = std::make_unique<AbilityRuntime::JsRuntime>();
    auto result = jsRuntime->SuspendVM(gettid());
    EXPECT_EQ(result, false);

    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "SuspendVM end");
}

/**
 * @tc.name: JsRuntimeResumeVMTest_0100
 * @tc.desc: JsRuntime test for ResumeVM.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeResumeVMTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ResumeVM start");

    auto jsRuntime = std::make_unique<AbilityRuntime::JsRuntime>();
    jsRuntime->ResumeVM(gettid());
    EXPECT_TRUE(jsRuntime != nullptr);

    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "ResumeVM end");
}

/**
 * @tc.name: JsRuntimeSetDeviceDisconnectCallbackTest_0100
 * @tc.desc: JsRuntime test for SetDeviceDisconnectCallback.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeSetDeviceDisconnectCallbackTest_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "SetDeviceDisconnectCallback start");

    auto jsRuntime = std::make_unique<AbilityRuntime::JsRuntime>();
    std::function<bool()> task = [&]() {
        return true;
    };
    jsRuntime->SetDeviceDisconnectCallback(task);
    EXPECT_TRUE(jsRuntime != nullptr);

    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "SetDeviceDisconnectCallback end");
}

/**
 * @tc.name: JsRuntimeDetachCallbackFuncTest_0100
 * @tc.desc: JsRuntime test for PostTask.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeDetachCallbackFuncTest_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "DetachCallbackFunc start");

    int32_t value = 1;
    int32_t number = 1;
    auto result = AbilityRuntime::DetachCallbackFunc(nullptr, &value, &number);
    EXPECT_EQ(result, &value);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "DetachCallbackFunc end");
}

/**
 * @tc.name: JsRuntimeLoadSystemModulesTest_0100
 * @tc.desc: JsRuntime test for LoadSystemModule.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeLoadSystemModulesTest_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "LoadSystemModule start");

    auto jsRuntime = std::make_unique<AbilityRuntime::JsRuntime>();
    std::string moduleName = "PreloadSystemModuleTest";
    napi_value object = nullptr;
    std::unique_ptr<NativeReference> ref = jsRuntime->LoadSystemModule(moduleName, &object, 0);
    EXPECT_EQ(ref, nullptr);

    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "LoadSystemModule end");
}

/**
 * @tc.name: JsRuntimeStartDebugModeTest_0100
 * @tc.desc: JsRuntime test for StartDebugMode.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeStartDebugModeTest_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "JsRuntimeStartDebugModeTest_0100 start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    AbilityRuntime::Runtime::DebugOption debugOption;
    debugOption.isStartWithDebug = true;
    debugOption.processName = "test";
    debugOption.isDebugApp = true;
    debugOption.isStartWithNative = false;
    debugOption.isDeveloperMode = true;
    jsRuntime->StartDebugMode(debugOption);
    EXPECT_TRUE(jsRuntime != nullptr);

    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "JsRuntimeStartDebugModeTest_0100 end");
}

/**
 * @tc.name: JsRuntimeStartDebugModeTest_0200
 * @tc.desc: JsRuntime test for StartDebugMode.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeStartDebugModeTest_0200, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "JsRuntimeStartDebugModeTest_0200 start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    AbilityRuntime::Runtime::DebugOption debugOption;
    debugOption.isStartWithDebug = true;
    debugOption.processName = "test";
    debugOption.isDebugApp = true;
    debugOption.isStartWithNative = false;
    debugOption.isDeveloperMode = false;
    debugOption.isDebugFromLocal = true;
    jsRuntime->StartDebugMode(debugOption);
    EXPECT_TRUE(jsRuntime != nullptr);

    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "JsRuntimeStartDebugModeTest_0200 end");
}

/**
 * @tc.name: JsRuntimeStopDebugModeTest_0100
 * @tc.desc: JsRuntime test for StopDebugMode.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeStopDebugModeTest_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StopDebugMode start");

    auto jsRuntime = std::make_unique<JsRuntime>();

    jsRuntime->StopDebugMode();
    EXPECT_TRUE(jsRuntime != nullptr);

    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "StopDebugMode end");
}

/**
 * @tc.name: JsRuntimeInitConsoleModuleTest_0100
 * @tc.desc: JsRuntime test for InitConsoleModule.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeInitConsoleModuleTest_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "InitConsoleModule start");

    auto jsRuntime = std::make_unique<JsRuntime>();

    jsRuntime->InitConsoleModule();
    EXPECT_TRUE(jsRuntime != nullptr);

    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "InitConsoleModule end");
}

/**
 * @tc.name: JsRuntimeLoadRepairPatchTest_0100
 * @tc.desc: JsRuntime test for LoadRepairPatch.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeLoadRepairPatchTest_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "LoadRepairPatch start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);

    std::string hqfFile = "<hqfFile>";
    std::string hapPath = "<hapPath>";
    bool lrp = jsRuntime->LoadRepairPatch(hqfFile, hapPath);
    EXPECT_EQ(lrp, false);

    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "LoadRepairPatch end");
}

/**
 * @tc.name: JsRuntimeUnLoadRepairPatchTest_0100
 * @tc.desc: JsRuntime test for UnLoadRepairPatch.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeUnLoadRepairPatchTest_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "UnLoadRepairPatch start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);

    std::string hqfFile = "<hqfFile>";
    bool lrp = jsRuntime->UnLoadRepairPatch(hqfFile);
    EXPECT_EQ(lrp, false);

    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "UnLoadRepairPatch end");
}

/**
 * @tc.name: JsRuntimeNotifyHotReloadPageTest_0100
 * @tc.desc: JsRuntime test for NotifyHotReloadPage.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeNotifyHotReloadPageTest_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyHotReloadPage start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);

    bool lrp = jsRuntime->NotifyHotReloadPage();
    EXPECT_EQ(lrp, true);

    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "NotifyHotReloadPage end");
}

/**
 * @tc.name: JsRuntimeUpdateModuleNameAndAssetPathTest_0100
 * @tc.desc: JsRuntime test for UpdateModuleNameAndAssetPath.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeUpdateModuleNameAndAssetPathTest_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateModuleNameAndAssetPath start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);

    std::string moduleName = "moduleName";
    jsRuntime->UpdateModuleNameAndAssetPath(moduleName);

    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "UpdateModuleNameAndAssetPath end");
}

/**
 * @tc.name: JsRuntimeUpdateModuleNameAndAssetPathTest_0200
 * @tc.desc: JsRuntime test for UpdateModuleNameAndAssetPath.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeUpdateModuleNameAndAssetPathTest_0200, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "JsRuntimeUpdateModuleNameAndAssetPathTest_0200 start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);

    jsRuntime->isBundle_ = false;
    std::string moduleName = "moduleName";
    jsRuntime->UpdateModuleNameAndAssetPath(moduleName);

    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "JsRuntimeUpdateModuleNameAndAssetPathTest_0200 end");
}

/**
 * @tc.name: JsRuntimeUpdateModuleNameAndAssetPathTest_0300
 * @tc.desc: JsRuntime test for UpdateModuleNameAndAssetPath.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeUpdateModuleNameAndAssetPathTest_0300, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "JsRuntimeUpdateModuleNameAndAssetPathTest_0300 start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);

    jsRuntime->isBundle_ = false;
    std::string moduleName = "";
    jsRuntime->UpdateModuleNameAndAssetPath(moduleName);

    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "JsRuntimeUpdateModuleNameAndAssetPathTest_0300 end");
}

/**
 * @tc.name: JsRuntimeInitialize_0100
 * @tc.desc: Initialize js runtime in multi thread.
 * @tc.type: FUNC
 * @tc.require: issueI6KODF
 */
HWTEST_F(JsRuntimeTest, JsRuntimeInitialize_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "Running in multi-thread, using default thread number.");

    auto jsRuntime = std::make_unique<AbilityRuntime::JsRuntime>();
    jsRuntime->preloaded_ = true;
    AbilityRuntime::Runtime::Options options;
    options.isStageModel = false;
    options.isTestFramework = false;
    options.preload = true;
    options.versionCode = 1000;
    bool result = jsRuntime->Initialize(options);
    EXPECT_EQ(result, true);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "Initialize_0100 end");
}

/**
 * @tc.name: RegisterQuickFixQueryFunc_0100
 * @tc.desc: JsRuntime test for RegisterQuickFixQueryFunc.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, RegisterQuickFixQueryFunc_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "RegisterQuickFixQueryFunc start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);
    std::string moudel = "<moudelName>";
    std::string hqfFile = "<hqfFile>";
    std::map<std::string, std::string> moduleAndPath;
    moduleAndPath.insert(std::make_pair(moudel, hqfFile));
    jsRuntime->RegisterQuickFixQueryFunc(moduleAndPath);
    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "RegisterQuickFixQueryFunc end");
}

/**
 * @tc.name: RegisterUncaughtExceptionHandler_0100
 * @tc.desc: JsRuntime test for RegisterUncaughtExceptionHandler.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, RegisterUncaughtExceptionHandler_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "RegisterUncaughtExceptionHandler start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    ASSERT_NE(jsRuntime, nullptr);
    JsEnv::UncaughtExceptionInfo uncaughtExceptionInfo;
    jsRuntime->RegisterUncaughtExceptionHandler(uncaughtExceptionInfo);
    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "RegisterUncaughtExceptionHandler end");
}

/**
 * @tc.name: ReadSourceMapData_0100
 * @tc.desc: JsRuntime test for ReadSourceMapData.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, ReadSourceMapData_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ReadSourceMapData start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    ASSERT_NE(jsRuntime, nullptr);
    std::string hapPath = "";
    std::string sourceMapPath = "";
    std::string content = "";
    auto result = jsRuntime->ReadSourceMapData(hapPath, sourceMapPath, content);
    ASSERT_FALSE(result);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "ReadSourceMapData end");
}

/**
 * @tc.name: StopDebugger_0100
 * @tc.desc: JsRuntime test for StopDebugger.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, StopDebugger_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StopDebugger_0100 start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    ASSERT_NE(jsRuntime, nullptr);
    jsRuntime->StopDebugger();
    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "StopDebugger_0100 end");
}

/**
 * @tc.name: GetFileBuffer_0100
 * @tc.desc: JsRuntime test for GetFileBuffer.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, GetFileBuffer_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "GetFileBuffer_0100 start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    ASSERT_NE(jsRuntime, nullptr);
    std::string filePath = "";
    std::string fileFullName = "";
    std::vector<uint8_t> buffer;
    bool result = jsRuntime->GetFileBuffer(filePath, fileFullName, buffer);
    EXPECT_EQ(result, false);
    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "GetFileBuffer_0100 end");
}

/**
 * @tc.name: JsRuntimeRunScriptTest_0100
 * @tc.desc: JsRuntime test for RunScript.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeRunScriptTest_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "RunScript_0100 start");

    auto jsRuntime = std::make_unique<AbilityRuntime::JsRuntime>();
    std::string srcPath = TEST_MODULE_PATH;
    std::string hapPath = TEST_HAP_PATH;
    auto result = jsRuntime->RunScript(srcPath, hapPath);
    EXPECT_EQ(result, false);
    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "RunScript_0100 end");
}

/**
 * @tc.name: JsRuntimeLoadScriptTest_0100
 * @tc.desc: JsRuntime test for LoadScript.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeLoadScriptTest_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "LoadScriptTest_0100 start");
    auto jsRuntime = std::make_unique<AbilityRuntime::JsRuntime>();
    std::string path = "/system/etc/strip.native.min.abc";
    auto result = jsRuntime->LoadScript(path);
    EXPECT_EQ(result, false);
    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "LoadScriptTest_0100 end");
}

/**
 * @tc.name: JsRuntimeStopDebuggerTest_0100
 * @tc.desc: JsRuntime test for StopDebugger.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeStopDebuggerTest_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StopDebuggerTest_0100 start");
    auto jsRuntime = std::make_unique<AbilityRuntime::JsRuntime>();
    ASSERT_NE(jsRuntime, nullptr);

    jsRuntime->StopDebugger();
    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "StopDebuggerTest_0100 end");
}

/**
 * @tc.name: PostSyncTask_0100
 * @tc.desc: Js runtime post sync task.
 * @tc.type: FUNC
 * @tc.require: issueI7C87T
 */
HWTEST_F(JsRuntimeTest, PostSyncTask_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PostSyncTask_0100 start");

    auto jsRuntime = std::make_unique<AbilityRuntime::JsRuntime>();
    ASSERT_NE(jsRuntime, nullptr);
    std::string taskName = "syncTask001";
    bool taskExecuted = false;
    auto task = [taskName, &taskExecuted]() {
        TAG_LOGI(AAFwkTag::TEST, "%{public}s called.", taskName.c_str());
        taskExecuted = true;
    };
    jsRuntime->PostSyncTask(task, taskName);
    EXPECT_NE(taskExecuted, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "PostSyncTask_0100 end");
}

/**
 * @tc.name: ReInitJsEnvImpl_0100
 * @tc.desc: Js runtime reinit js env impl.
 * @tc.type: FUNC
 * @tc.require: issueI7C87T
 */
HWTEST_F(JsRuntimeTest, ReInitJsEnvImpl_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ReInitJsEnvImpl_0100 start");
    auto jsRuntime = std::make_unique<JsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);

    jsRuntime->ReInitJsEnvImpl(options_);
    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "ReInitJsEnvImpl_0100 end");
}

/**
 * @tc.name: PostTask_0100
 * @tc.desc: Js runtime post task.
 * @tc.type: FUNC
 * @tc.require: issueI7C87T
 */
HWTEST_F(JsRuntimeTest, PostTask_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PostTask_0100 start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    std::string taskName = "postTask001";
    bool taskExecuted = false;
    auto task = [taskName, &taskExecuted]() {
        TAG_LOGI(AAFwkTag::TEST, "%{public}s called.", taskName.c_str());
        taskExecuted = true;
    };
    int64_t delayTime = 10;
    jsRuntime->PostTask(task, taskName, delayTime);
    EXPECT_NE(taskExecuted, true);
    TAG_LOGI(AAFwkTag::TEST, "PostTask_0100 end");
}

/**
 * @tc.name: RemoveTask_0100
 * @tc.desc: Js runtime remove task.
 * @tc.type: FUNC
 * @tc.require: issueI7C87T
 */
HWTEST_F(JsRuntimeTest, RemoveTask_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoveTask_0100 start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    ASSERT_TRUE(jsRuntime != nullptr);

    std::string taskName = "removeTask001";
    bool taskExecuted = false;
    auto task = [taskName, &taskExecuted]() {
        TAG_LOGI(AAFwkTag::TEST, "%{public}s called.", taskName.c_str());
        taskExecuted = true;
    };
    int64_t delayTime = 10;
    jsRuntime->PostTask(task, taskName, delayTime);
    jsRuntime->RemoveTask(taskName);
    EXPECT_NE(taskExecuted, true);
    TAG_LOGI(AAFwkTag::TEST, "RemoveTask_0100 end");
}

/**
 * @tc.name: StartDebugger_0100
 * @tc.desc: JsRuntime test for StartDebugger.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, StartDebugger_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartDebugger_0100 start");

    auto jsRuntime = std::make_unique<AbilityRuntime::JsRuntime>();
    ASSERT_NE(jsRuntime, nullptr);

    bool needBreakPoint = false;
    uint32_t instanceId = 1;

    auto result = jsRuntime->StartDebugger(needBreakPoint, instanceId);
    EXPECT_EQ(result, true);
    // debug mode is global option, maybe has started by other testcase, not check here.
    TAG_LOGI(AAFwkTag::TEST, "StartDebugger_0100 end");
}

/**
 * @tc.name: ReloadFormComponent_0100
 * @tc.desc: JsRuntime test for ReloadFormComponent.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, ReloadFormComponent_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ReloadFormComponent_0100 start");

    auto jsRuntime = std::make_unique<AbilityRuntime::JsRuntime>();
    ASSERT_NE(jsRuntime, nullptr);

    jsRuntime->ReloadFormComponent();
    jsRuntime.reset();
    TAG_LOGI(AAFwkTag::TEST, "ReloadFormComponent_0100 end");
}

/**
 * @tc.name: SetRequestAotCallback_0100
 * @tc.desc: JsRuntime test for SetRequestAotCallback.
 * @tc.type: FUNC
 * @tc.require: issueI82L1A
 */
HWTEST_F(JsRuntimeTest, SetRequestAotCallback_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "start");

    auto jsRuntime = std::make_unique<AbilityRuntime::JsRuntime>();
    ASSERT_NE(jsRuntime, nullptr);

    jsRuntime->SetRequestAotCallback();
    auto ret = panda::MockJSNApi::GetInstance()->RequestAot("bundleName", "moduleName", 0);
    EXPECT_NE(ret, -1);
    jsRuntime.reset();
    TAG_LOGI(AAFwkTag::TEST, "finish");
}

/**
 * @tc.name: DestroyHeapProfiler_0100
 * @tc.desc: JsRuntime test for DestroyHeapProfiler.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, DestroyHeapProfiler_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "DestroyHeapProfiler_0100 start");

    auto jsRuntime = std::make_unique<AbilityRuntime::JsRuntime>();

    jsRuntime->DestroyHeapProfiler();
    ASSERT_NE(jsRuntime, nullptr);
    jsRuntime.reset();
    TAG_LOGI(AAFwkTag::TEST, "DestroyHeapProfiler_0100 end");
}

/**
 * @tc.name: ForceFullGC_0100
 * @tc.desc: JsRuntime test for ForceFullGC.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, ForceFullGC_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ForceFullGC_0100 start");

    auto jsRuntime = std::make_unique<AbilityRuntime::JsRuntime>();

    jsRuntime->ForceFullGC();
    ASSERT_NE(jsRuntime, nullptr);
    jsRuntime.reset();
    TAG_LOGI(AAFwkTag::TEST, "ForceFullGC_0100 end");
}

/**
 * @tc.name: AllowCrossThreadExecution_0100
 * @tc.desc: JsRuntime test for AllowCrossThreadExecution.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, AllowCrossThreadExecution_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AllowCrossThreadExecution_0100 start");

    auto jsRuntime = std::make_unique<AbilityRuntime::JsRuntime>();

    jsRuntime->AllowCrossThreadExecution();
    ASSERT_NE(jsRuntime, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AllowCrossThreadExecution_0100 end");
}

/**
 * @tc.name: GetHeapPrepare_0100
 * @tc.desc: JsRuntime test for GetHeapPrepare.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, GetHeapPrepare_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "GetHeapPrepare_0100 start");

    auto jsRuntime = std::make_unique<AbilityRuntime::JsRuntime>();

    jsRuntime->GetHeapPrepare();
    ASSERT_NE(jsRuntime, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "GetHeapPrepare_0100 end");
}

/**
 * @tc.name: InitLoop_0100
 * @tc.desc: JsRuntime test for InitLoop.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, InitLoop_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "InitLoop_0100 start");

    auto jsRuntime = std::make_unique<AbilityRuntime::JsRuntime>();

    auto result = jsRuntime->InitLoop();
    EXPECT_EQ(result, false);
    jsRuntime.reset();
    TAG_LOGI(AAFwkTag::TEST, "InitLoop_0100 end");
}

/**
 * @tc.name: InitSourceMap_0100
 * @tc.desc: JsRuntime test for InitSourceMap.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, InitSourceMap_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "InitSourceMap_0100 start");

    auto jsRuntime = std::make_unique<AbilityRuntime::JsRuntime>();
    auto operatorObj = std::make_shared<JsEnv::SourceMapOperator>("");
    jsRuntime->InitSourceMap(operatorObj);
    ASSERT_NE(jsRuntime, nullptr);
    jsRuntime.reset();
    TAG_LOGI(AAFwkTag::TEST, "InitSourceMap_0100 end");
}

HWTEST_F(JsRuntimeTest, InitSourceMap_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "InitSourceMap_0200 start");

    auto mapObj = std::make_shared<JsEnv::SourceMap>();
    bool hasFile = false;
    mapObj->Init(hasFile, TEST_HAP_PATH);
    EXPECT_FALSE(hasFile);
    TAG_LOGI(AAFwkTag::TEST, "InitSourceMap_0200 end");
}

/**
 * @tc.name: Deinitialize_0100
 * @tc.desc: JsRuntime test for Deinitialize.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, Deinitialize_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "Deinitialize_0100 start");

    auto jsRuntime = std::make_unique<AbilityRuntime::JsRuntime>();

    jsRuntime->Deinitialize();
    ASSERT_NE(jsRuntime, nullptr);
    jsRuntime.reset();
    TAG_LOGI(AAFwkTag::TEST, "Deinitialize_0100 end");
}

/**
 * @tc.name: GetPkgContextInfoListMap_0100
 * @tc.desc: JsRuntime test for GetPkgContextInfoListMap.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, GetPkgContextInfoListMap_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "GetPkgContextInfoListMap_0100 start");

    std::map<std::string, std::string> modulePkgContentMap;
    std::string pkgContentJsonString = R"({"library": {"packageName": "library", "bundleName": "com.xxx.xxxx",
        "moduleName": "library", "version": "1.0.0", "entryPath": "", "isSO": false}})";
    modulePkgContentMap["entry"] = pkgContentJsonString;

    AbilityRuntime::Runtime::Options options;
    options.preload = true;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);
    std::map<std::string, std::vector<std::vector<std::string>>> ret;
    std::map<std::string, std::string> pkgAliasMap;
    JsRuntimeLite::GetInstance().GetPkgContextInfoListMap(modulePkgContentMap, ret, pkgAliasMap);
    std::string expectString = "library:packageName:library:bundleName:";
    expectString += "com.xxx.xxxx:moduleName:library:version:1.0.0:entryPath::isSO:false:";
    auto it = ret.find("entry");
    ASSERT_EQ(it, ret.end());
    jsRuntime.reset();
    TAG_LOGI(AAFwkTag::TEST, "GetPkgContextInfoListMap_0100 end");
}

/**
 * @tc.name: GetPkgContextInfoListMap_0200
 * @tc.desc: JsRuntime test for GetPkgContextInfoListMap.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, GetPkgContextInfoListMap_0200, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "GetPkgContextInfoListMap_0200 start");

    std::map<std::string, std::string> modulePkgContentMap;
    std::string pkgContentJsonString = R"({"library": {"packageName": "library", "bundleName":
        "com.xxx.xxxx", "moduleName": "library", "version": "1.0.0", "entryPath": "", "isSO": false}})";
    modulePkgContentMap["entry"] = pkgContentJsonString;

    std::string libraryString = R"({"library": {"packageName": "library","bundleName": "com.xxx.xxxx", "moduleName":
        "library", "version": "1.0.0", "entryPath": "", "isSO": false}})";
    modulePkgContentMap["library"] = libraryString;

    AbilityRuntime::Runtime::Options options;
    options.preload = true;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);
    std::map<std::string, std::vector<std::vector<std::string>>> ret;
    std::map<std::string, std::string> pkgAliasMap;
    JsRuntimeLite::GetInstance().GetPkgContextInfoListMap(modulePkgContentMap, ret, pkgAliasMap);
    std::string expectString = "library:packageName:library:bundleName:";
    expectString += "com.xxx.xxxx:moduleName:library:version:1.0.0:entryPath::isSO:false:";
    auto it = ret.find("entry");
    ASSERT_EQ(it, ret.end());
    auto libraryIt = ret.find("library");
    ASSERT_EQ(libraryIt, ret.end());
    jsRuntime.reset();
    TAG_LOGI(AAFwkTag::TEST, "GetPkgContextInfoListMap_0200 end");
}

/**
 * @tc.name: CreateJsEnv_0100
 * @tc.desc: JsRuntime test for CreateJsEnv.
 * @tc.type: FUNC
 * @tc.require: issueI9CHSB
 */
HWTEST_F(JsRuntimeTest, CreateJsEnv_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateJsEnv_0100 start");
    auto jsRuntime = std::make_unique<JsRuntime>();
    auto ret = jsRuntime->CreateJsEnv(options_);
    EXPECT_EQ(ret, true);
    jsRuntime.reset();
    TAG_LOGI(AAFwkTag::TEST, "CreateJsEnv_0100 start");
}

/**
 * @tc.name: DumpCpuProfile_0100
 * @tc.desc: JsRuntime test for DumpCpuProfile.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, DumpCpuProfile_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DumpCpuProfile_0100 start");
    auto jsRuntime = std::make_unique<JsRuntime>();
    bool isPrivate = true;
    jsRuntime->DumpCpuProfile();
    EXPECT_TRUE(jsRuntime != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "DumpCpuProfile_0100 end");
}

/**
 * @tc.name: DumpHeapSnapshot_0100
 * @tc.desc: JsRuntime test for DumpHeapSnapshot.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, DumpHeapSnapshot_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DumpHeapSnapshot_0100 start");
    auto jsRuntime = std::make_unique<JsRuntime>();
    bool isPrivate = true;
    jsRuntime->DumpHeapSnapshot(isPrivate);
    EXPECT_TRUE(jsRuntime != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "DumpHeapSnapshot end");
}

/**
 * @tc.name: DumpHeapSnapshot_0200
 * @tc.desc: JsRuntime test for DumpHeapSnapshot.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, DumpHeapSnapshot_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DumpHeapSnapshot_0100 start");
    auto jsRuntime = std::make_unique<JsRuntime>();
    uint32_t tid = 1;
    bool isFullGC = true;
    jsRuntime->DumpHeapSnapshot(tid, isFullGC);
    EXPECT_TRUE(jsRuntime != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "DumpHeapSnapshot end");
}

/**
 * @tc.name: DumpHeapSnapshot_0400
 * @tc.desc: JsRuntime test for DumpHeapSnapshot with JsHeapDumpParam.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, DumpHeapSnapshot_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DumpHeapSnapshot_0400 start");
    auto jsRuntime = std::make_unique<JsRuntime>();
    uint32_t tid = 1;
    OHOS::AbilityRuntime::Runtime::JsHeapDumpParam param;
    param.isFullGC = false;
    param.isBinary = false;
    param.isClearNodeIdCache = true;
    param.isProcDump = false;
    jsRuntime->DumpHeapSnapshot(tid, param);
    EXPECT_TRUE(jsRuntime != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "DumpHeapSnapshot end");
}

/**
 * @tc.name: DumpHeapSnapshot_0500
 * @tc.desc: JsRuntime test for DumpHeapSnapshot with JsHeapDumpParam isMergeBinary true.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, DumpHeapSnapshot_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DumpHeapSnapshot_0500 start");
    auto jsRuntime = std::make_unique<JsRuntime>();
    uint32_t tid = 1;
    OHOS::AbilityRuntime::Runtime::JsHeapDumpParam param;
    param.isFullGC = true;
    param.isBinary = true;
    param.isClearNodeIdCache = true;
    param.isProcDump = true;
    jsRuntime->DumpHeapSnapshot(tid, param);
    EXPECT_TRUE(jsRuntime != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "DumpHeapSnapshot end");
}

/**
 * @tc.name: DumpHeapSnapshot_0600
 * @tc.desc: JsRuntime test for DumpHeapSnapshot with default JsHeapDumpParam.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, DumpHeapSnapshot_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DumpHeapSnapshot_0600 start");
    auto jsRuntime = std::make_unique<JsRuntime>();
    uint32_t tid = 1;
    OHOS::AbilityRuntime::Runtime::JsHeapDumpParam param;
    jsRuntime->DumpHeapSnapshot(tid, param);
    EXPECT_TRUE(jsRuntime != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "DumpHeapSnapshot end");
}

/**
 * @tc.name: AllowCrossThreadExecution_0200
 * @tc.desc: JsRuntime test for AllowCrossThreadExecution.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, AllowCrossThreadExecution_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AllowCrossThreadExecution_0200 start");
    auto jsRuntime = std::make_unique<JsRuntime>();
    jsRuntime->AllowCrossThreadExecution();
    EXPECT_TRUE(jsRuntime != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AllowCrossThreadExecution_0200 end");
}

/**
 * @tc.name: GetHeapPrepare_0200
 * @tc.desc: JsRuntime test for GetHeapPrepare.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, GetHeapPrepare_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetHeapPrepare_0200 start");
    auto jsRuntime = std::make_unique<JsRuntime>();
    jsRuntime->GetHeapPrepare();
    EXPECT_TRUE(jsRuntime != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "GetHeapPrepare_0200 end");
}

/**
 * @tc.name: RegisterQuickFixQueryFunc_0200
 * @tc.desc: JsRuntime test for RegisterQuickFixQueryFunc.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, RegisterQuickFixQueryFunc_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RegisterQuickFixQueryFunc_0200 start");
    auto jsRuntime = std::make_unique<JsRuntime>();
    std::map<std::string, std::string> moduleAndPath;
    jsRuntime->RegisterQuickFixQueryFunc(moduleAndPath);
    EXPECT_TRUE(jsRuntime != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "RegisterQuickFixQueryFunc_0200 end");
}

/**
 * @tc.name: UpdatePkgContextInfoJson_0100
 * @tc.desc: JsRuntime test for UpdatePkgContextInfoJson.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, UpdatePkgContextInfoJson_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdatePkgContextInfoJson_0100 start");
    auto jsRuntime = std::make_unique<JsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);
    std::string moduleName = "moduleName";
    std::string hapPath = TEST_HAP_PATH;
    std::string packageName = "packageName";
    jsRuntime->UpdatePkgContextInfoJson(moduleName, hapPath, packageName);
    TAG_LOGI(AAFwkTag::TEST, "UpdatePkgContextInfoJson_0100 end");
}

HWTEST_F(JsRuntimeTest, SetPkgContextInfoJson_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetPkgContextInfoJson_0100 start");
    auto jsRuntime = std::make_unique<JsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);
    std::string moduleName = "moduleName";
    std::string hapPath = TEST_HAP_PATH;
    std::string packageName = "packageName";
    jsRuntime->SetPkgContextInfoJson(moduleName, hapPath, packageName);
    EXPECT_FALSE(jsRuntime->pkgContextInfoJsonStringMap_.empty());
    TAG_LOGI(AAFwkTag::TEST, "SetPkgContextInfoJson_0100 end");
}

/**
 * @tc.name: JsRuntimePreloadModuleandDoCleanWorkAfterStageCleaned_0100
 * @tc.desc: JsRuntime test for JsRuntimePreloadModule and DoCleanWorkAfterStageCleaned.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimePreloadModuleandDoCleanWorkAfterStageCleaned_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadModule_0100 start");
    auto jsRuntime = std::make_unique<JsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);
    jsRuntime->DoCleanWorkAfterStageCleaned();
    std::string moduleName = TEST_MODULE_NAME;
    std::string srcPath = TEST_LIB_PATH;
    std::string hapPath = TEST_HAP_PATH;
    bool isEsMode = true;
    bool useCommonTrunk = true;
    jsRuntime->PreloadModule(moduleName, srcPath, hapPath, isEsMode, useCommonTrunk);
    jsRuntime->PreloadModule(moduleName, hapPath, isEsMode, useCommonTrunk);
    EXPECT_EQ(jsRuntime->preloadList_.size(), 1);
    jsRuntime.reset();
    TAG_LOGI(AAFwkTag::TEST, "PreloadModule_0100 end");
}

/**
 * @tc.name: JsRuntimePreloadMainAbility_0100
 * @tc.desc: JsRuntime test for JsRuntimePreloadMainAbility.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimePreloadMainAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadMainAbility_0100 start");
    auto jsRuntime = std::make_unique<JsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);
    std::string moduleName = TEST_MODULE_NAME;
    std::string srcPath = TEST_MODULE_PATH;
    std::string hapPath = TEST_HAP_PATH;
    std::string srcEntrance = TEST_LIB_PATH;
    bool isEsMode = true;
    jsRuntime->PreloadMainAbility(moduleName, srcPath, hapPath, isEsMode, srcEntrance);
    EXPECT_EQ(jsRuntime->preloadList_.size(), 1);
    jsRuntime.reset();
    TAG_LOGI(AAFwkTag::TEST, "PreloadMainAbility_0100 end");
}

/**
 * @tc.name: JsRuntimeSetStopPreloadSoCallback_0100
 * @tc.desc: JsRuntime test for JsRuntimeSetStopPreloadSoCallback.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeSetStopPreloadSoCallback_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetStopPreloadSoCallback_0100 start");
    auto callBack = []() {};
    jsRuntimePtr->SetStopPreloadSoCallback(callBack);
    ASSERT_TRUE(jsRuntimePtr != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "SetStopPreloadSoCallback_0100 start");
}

/**
 * @tc.name: SetDebugOption_0100
 * @tc.desc: JsRuntime test for SetDebugOption.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, SetDebugOption_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetDebugOption_0100 start");
    Runtime::DebugOption debugOption;
    std::string bundleName = "com.ohos.example.bundleName";
    debugOption.bundleName = bundleName;
    debugOption.processName = "com.ohos.example.bundleName";
    auto jsRuntime = std::make_unique<JsRuntime>();
    jsRuntime->SetDebugOption(debugOption);
    EXPECT_EQ(jsRuntime->debugOption_.bundleName, bundleName);
    TAG_LOGI(AAFwkTag::TEST, "SetDebugOption_0100 end");
}

/**
 * @tc.name: StartLocalDebugMode_0100
 * @tc.desc: JsRuntime test for StartLocalDebugMode.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, StartLocalDebugMode_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartLocalDebugMode_0100 start");
    bool isDebugFromLocal = false;
    auto jsRuntime = std::make_unique<JsRuntime>();
    Runtime::DebugOption debugOption;
    jsRuntime->SetDebugOption(debugOption);
    jsRuntime->debugOption_.isDeveloperMode = false;
    jsRuntime->StartLocalDebugMode(isDebugFromLocal);
    EXPECT_EQ(jsRuntime->debugOption_.isDebugFromLocal, false);

    isDebugFromLocal = true;
    jsRuntime->StartLocalDebugMode(isDebugFromLocal);
    EXPECT_EQ(jsRuntime->debugOption_.isDebugFromLocal, true);
    TAG_LOGI(AAFwkTag::TEST, "StartLocalDebugMode_0100 end");
}

/**
 * @tc.name: StartProfiler_0100
 * @tc.desc: JsRuntime test for StartProfiler.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, StartProfiler_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartProfiler_0100 start");
    Runtime::DebugOption debugOption;
    debugOption.isDebugFromLocal = true;
    debugOption.isDeveloperMode = true;
    auto jsRuntime = std::make_unique<JsRuntime>();
    EXPECT_NE(jsRuntime, nullptr);
    jsRuntime->StartProfiler(debugOption);
    EXPECT_EQ(jsRuntime->jsEnv_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "StartProfiler_0100 end");
}

/**
 * @tc.name: LoadScript_0200
 * @tc.desc: LoadScript
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, LoadScript_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "LoadScript_0200 start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    jsRuntime->jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    jsRuntime->jsEnv_->engine_ = nullptr;

    auto ret = jsRuntime->LoadScript("MyPath", nullptr, false);
    EXPECT_EQ(ret, false);

    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "LoadScript_0200 end");
}

/**
 * @tc.name: LoadScript_0300
 * @tc.desc: LoadScript
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, LoadScript_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "LoadScript_0300 start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    jsRuntime->jsEnv_ = nullptr;

    auto ret = jsRuntime->LoadScript("MyPath", nullptr, 0, false, "");
    EXPECT_EQ(ret, false);

    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "LoadScript_0300 end");
}

/**
 * @tc.name: LoadScript_0400
 * @tc.desc: LoadScript
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, LoadScript_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "LoadScript_0400 start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    jsRuntime->jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    jsRuntime->isOhmUrl_ = true;
    jsRuntime->moduleName_ = "moduleName";

    auto ret = jsRuntime->LoadScript("MyPath", nullptr, 0, false, "");
    EXPECT_EQ(ret, false);

    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "LoadScript_0400 end");
}

/**
 * @tc.name: LoadScript_0500
 * @tc.desc: LoadScript
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, LoadScript_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "LoadScript_0500 start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    jsRuntime->jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    jsRuntime->isOhmUrl_ = false;
    jsRuntime->moduleName_ = "moduleName";

    auto ret = jsRuntime->LoadScript("MyPath", nullptr, 0, false, "");
    EXPECT_EQ(ret, false);

    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "LoadScript_0500 end");
}

/**
 * @tc.name: LoadSystemModuleByEngine_0200
 * @tc.desc: LoadSystemModuleByEngine
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, LoadSystemModuleByEngine_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "LoadSystemModuleByEngine_0200 start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    jsRuntime->jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    napi_env env = {};
    jsRuntime->jsEnv_->engine_ = reinterpret_cast<NativeEngine*>(env);

    auto ret = jsRuntime->LoadSystemModuleByEngine(env, "", nullptr, 0);
    EXPECT_EQ(ret, nullptr);

    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "LoadSystemModuleByEngine_0200 end");
}

/**
 * @tc.name: Deinitialize_0200
 * @tc.desc: Deinitialize
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, Deinitialize_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Deinitialize_0200 start");

    auto jsRuntime = std::make_unique<AbilityRuntime::JsRuntime>();
    std::string moduleName = "PreloadSystemModuleTest";
    napi_value object = nullptr;
    std::unique_ptr<NativeReference> nativeRef = jsRuntime->LoadSystemModule(moduleName, &object, 0);
    jsRuntime->modules_.emplace("moduleName", nativeRef.get());
    jsRuntime->jsEnv_ = nullptr;

    jsRuntime->Deinitialize();
    EXPECT_EQ(jsRuntime->modules_.size(), 0);

    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "Deinitialize_0200 end");
}

/**
 * @tc.name: RunScript_0200
 * @tc.desc: RunScript
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, RunScript_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RunScript_0200 start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    jsRuntime->jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    panda::RuntimeOption pandaOption;
    jsRuntime->jsEnv_->vm_ = panda::JSNApi::CreateJSVM(pandaOption);

    auto ret = jsRuntime->RunScript("", "", false);
    EXPECT_EQ(ret, false);

    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "RunScript_0200 end");
}

/**
 * @tc.name: RunScript_0300
 * @tc.desc: RunScript
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, RunScript_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RunScript_0300 start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    jsRuntime->jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    panda::RuntimeOption pandaOption;
    jsRuntime->jsEnv_->vm_ = panda::JSNApi::CreateJSVM(pandaOption);

    auto ret = jsRuntime->RunScript("MockPath", "", true);
    EXPECT_EQ(ret, false);

    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "RunScript_0300 end");
}

/**
 * @tc.name: RunSandboxScript_0300
 * @tc.desc: RunSandboxScript
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, RunSandboxScript_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RunSandboxScript_0300 start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    std::string bundleName(PATH_MAX, 'a');
    const std::string& codePath = bundleName;
    const std::string& modulePath = "";
    std::string fileName = "";
    jsRuntime->codePath_ = codePath;

    auto ret = jsRuntime->RunSandboxScript(modulePath, fileName);
    EXPECT_EQ(ret, false);

    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "RunSandboxScript_0300 end");
}

/**
 * @tc.name: RunSandboxScript_0300
 * @tc.desc: RunSandboxScript
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, RunSandboxScript_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RunSandboxScript_0400 start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    std::string bundleName(PATH_MAX, 'a');
    const std::string& codePath = bundleName;
    const std::string& modulePath = "";
    std::string fileName = "HelloWorld";
    jsRuntime->codePath_ = codePath;

    auto ret = jsRuntime->RunSandboxScript(modulePath, fileName);
    EXPECT_EQ(ret, false);

    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "RunSandboxScript_0400 end");
}

/**
 * @tc.name: RemoveTask_0300
 * @tc.desc: RemoveTask
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, RemoveTask_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoveTask_0300 start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    jsRuntime->jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    std::string taskName = "removeTask003";
    bool taskExecuted = false;
    auto task = [taskName, &taskExecuted]() {
        TAG_LOGI(AAFwkTag::TEST, "%{public}s called.", taskName.c_str());
        taskExecuted = true;
    };
    int64_t delayTime = 20;
    jsRuntime->PostTask(task, taskName, delayTime);
    jsRuntime->RemoveTask(taskName);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    EXPECT_EQ(taskExecuted, false);

    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "RemoveTask_0300 end");
}

/**
 * @tc.name: SuspendVM_0200
 * @tc.desc: SuspendVM
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, SuspendVM_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SuspendVM_0200 start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    jsRuntime->jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    napi_env env = {};
    jsRuntime->jsEnv_->engine_ = reinterpret_cast<NativeEngine*>(env);
    auto ret = jsRuntime->SuspendVM(3);
    EXPECT_EQ(ret, false);

    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "SuspendVM_0200 end");
}

/**
 * @tc.name: PopPreloadObj_0100
 * @tc.desc: PopPreloadObj
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, PopPreloadObj_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PopPreloadObj_0100 start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    napi_ref ref = nullptr;
    napi_value value = {};
    napi_env env_ = {};
    napi_create_reference(env_, value, 1, &ref);
    auto navRef = std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference *>(ref));
    std::map<std::string, std::unique_ptr<NativeReference>> preloadObjMap;
    preloadObjMap.emplace("PreloadObj", nullptr);
    preloadObjMap.emplace("PreloadObj1", std::move(navRef));
    jsRuntime->preloadList_ = std::move(preloadObjMap);

    std::unique_ptr<NativeReference> obj = nullptr;
    auto ret = jsRuntime->PopPreloadObj("PreloadObj", obj);
    EXPECT_EQ(ret, false);
    EXPECT_EQ(obj, nullptr);
    EXPECT_EQ(jsRuntime->preloadList_.size(), 1);

    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "PopPreloadObj_0100 end");
}

/**
 * @tc.name: UpdateModuleNameAndAssetPath_0400
 * @tc.desc: UpdateModuleNameAndAssetPath
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, UpdateModuleNameAndAssetPath_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateModuleNameAndAssetPath_0400 start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    jsRuntime->jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    jsRuntime->isBundle_ = false;
    panda::RuntimeOption pandaOption;
    jsRuntime->jsEnv_->vm_ = panda::JSNApi::CreateJSVM(pandaOption);
    jsRuntime->moduleName_ = "";

    jsRuntime->UpdateModuleNameAndAssetPath("moduleName");
    EXPECT_EQ(jsRuntime->moduleName_, "moduleName");

    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "UpdateModuleNameAndAssetPath_0400 end");
}

/**
 * @tc.name: ExecuteSecureWithOhmUrl_0100
 * @tc.desc: basic function test of ExecuteSecureWithOhmUrl and GetExportObjectFromOhmUrl.
 * @tc.type: FUNC
 * @tc.require: issueIC77WI
 */
HWTEST_F(JsRuntimeTest, ExecuteSecureWithOhmUrl_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteSecureWithOhmUrl_0100 start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    jsRuntime->jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    jsRuntime->isBundle_ = false;
    panda::RuntimeOption pandaOption;
    jsRuntime->jsEnv_->vm_ = panda::JSNApi::CreateJSVM(pandaOption);
    jsRuntime->moduleName_ = "";

    std::string moduleName = "moduleName";
    std::string hapPath = "hapPath";
    std::string srcEntrance = "srcEntrance";
    auto ret = jsRuntime->ExecuteSecureWithOhmUrl(moduleName, hapPath, srcEntrance);
    EXPECT_EQ(ret, nullptr);

    std::string key = "key";
    auto retVal = jsRuntime->GetExportObjectFromOhmUrl(srcEntrance, key);
    EXPECT_EQ(retVal, nullptr);

    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "ExecuteSecureWithOhmUrl_0100 end");
}

/**
 * @tc.name: RegisterUncatchableExceptionHandler_0100
 * @tc.desc: JsRuntime test for RegisterUncatchableExceptionHandler.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsRuntimeTest, RegisterUncatchableExceptionHandler_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RegisterUncatchableExceptionHandler_0100 start");
    auto jsRuntime = std::make_unique<JsRuntime>();
    EXPECT_NE(jsRuntime, nullptr);
    auto ret = jsRuntime->CreateJsEnv(options_);
    jsRuntime->RegisterUncatchableExceptionHandler(nullptr);
    EXPECT_NE(jsRuntime, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "RegisterUncatchableExceptionHandler_0100 end");
}

/**
 * @tc.name: JsRuntimeTestSetOrUpdateLibPath_0100
 * @tc.desc: JsRuntime Test for SetOrUpdateLibPath
 * @tc.type: FUNC
 * @tc.require: issueI581RO
 */
HWTEST_F(JsRuntimeTest, JsRuntimeTestSetOrUpdateLibPath_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "SetOrUpdateLibPath_0100 start");
    std::string appLibPathKey = TEST_BUNDLE_NAME + TEST_MODULE_NAME;
    std::string libPath = TEST_LIB_PATH;

    AppLibPathMap appLibPaths {};
    JsRuntime::SetOrUpdateLibPath(appLibPaths);

    appLibPaths[appLibPathKey].emplace_back(libPath);
    JsRuntime::SetOrUpdateLibPath(appLibPaths);
    EXPECT_NE(appLibPaths.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "SetOrUpdateLibPath_0100 end");
}
} // namespace AbilityRuntime
} // namespace OHOS