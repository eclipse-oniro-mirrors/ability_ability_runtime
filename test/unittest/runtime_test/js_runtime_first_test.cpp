/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#define private public
#define protected public
#include "js_environment.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "js_worker.h"
#undef private
#undef protected
#include "event_runner.h"
#include "hilog_tag_wrapper.h"
#include "mock_js_runtime.h"
#include "mock_jsnapi.h"

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
const std::string TEST_HAP_PATH =
    "/system/app/com.ohos.contactsdataability/Contacts_DataAbility.hap";
const std::string TEST_LIB_PATH = "/data/storage/el1/bundle/lib/";
const std::string TEST_MODULE_PATH = "/data/storage/el1/bundle/curJsModulePath";
}  // namespace
class JsRuntimeTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    Runtime::Options options_;
};

void JsRuntimeTest::SetUpTestCase() {}

void JsRuntimeTest::TearDownTestCase() {}

void JsRuntimeTest::SetUp()
{
    Runtime::Options newOptions;
    options_ = newOptions;
    options_.bundleName = TEST_BUNDLE_NAME;
    options_.codePath = TEST_CODE_PATH;
    options_.loadAce = false;
    options_.isBundle = true;
    options_.preload = false;
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner =
        AppExecFwk::EventRunner::Create(TEST_ABILITY_NAME);
    options_.eventRunner = eventRunner;
}

void JsRuntimeTest::TearDown() {}

/**
 * @tc.name: DebuggerConnectionHandler_0100
 * @tc.desc: JsRuntime test for DebuggerConnectionHandler.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, UpdatePkgContextInfoJson_0100, TestSize.Level1)
{
    AbilityRuntime::Runtime::Options options;
    options.preload = true;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);
    ASSERT_NE(jsRuntime, nullptr);
    bool isDebugApp = true;
    bool isStartWithDebug = false;
    jsRuntime->jsEnv_->vm_ = nullptr;
    jsRuntime->DebuggerConnectionHandler(isDebugApp, isStartWithDebug);
    jsRuntime.reset();
}

/**
 * @tc.name: DebuggerConnectionHandler_0200
 * @tc.desc: JsRuntime test for DebuggerConnectionHandler.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, UpdatePkgContextInfoJson_0200, TestSize.Level1)
{
    AbilityRuntime::Runtime::Options options;
    options.preload = true;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);
    ASSERT_NE(jsRuntime, nullptr);
    bool isDebugApp = true;
    bool isStartWithDebug = false;
    jsRuntime->jsEnv_->vm_ = nullptr;
    jsRuntime->DebuggerConnectionHandler(isDebugApp, isStartWithDebug);
    jsRuntime.reset();
}

/**
 * @tc.name: GetSafeData_0100
 * @tc.desc: JsRuntime test for GetSafeData.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, GetSafeData_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetSafeData_0100 start");
    AbilityRuntime::Runtime::Options options;
    options.preload = true;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);
    ASSERT_NE(jsRuntime, nullptr);
    std::string Path = "";
    std::string fileFullName = "";
    jsRuntime->jsEnv_->vm_ = nullptr;
    jsRuntime->GetSafeData(Path, fileFullName);
    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "GetSafeData_0100 end");
}

/**
 * @tc.name: DebuggerConnectionManager_0100
 * @tc.desc: JsRuntime test for DebuggerConnectionManager.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, DebuggerConnectionManager_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DebuggerConnectionManager_0100 start");
    AbilityRuntime::Runtime::Options options;
    options.preload = true;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);
    ASSERT_NE(jsRuntime, nullptr);
    bool isDebugApp = true;
    bool isStartWithDebug = true;
    AbilityRuntime::Runtime::DebugOption dOption;
    dOption.perfCmd = "profile jsperf 100";
    dOption.isStartWithDebug = false;
    dOption.processName = "test";
    dOption.isDebugApp = true;
    dOption.isStartWithNative = false;
    jsRuntime->jsEnv_->vm_ = nullptr;
    jsRuntime->DebuggerConnectionManager(isDebugApp, isStartWithDebug, dOption);
    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "DebuggerConnectionManager_0100 end");
}

/**
 * @tc.name: DebuggerConnectionManager_0200
 * @tc.desc: JsRuntime test for DebuggerConnectionManager.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, DebuggerConnectionManager_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DebuggerConnectionManager_0200 start");
    AbilityRuntime::Runtime::Options options;
    options.preload = true;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);
    ASSERT_NE(jsRuntime, nullptr);
    bool isDebugApp = false;
    bool isStartWithDebug = true;
    AbilityRuntime::Runtime::DebugOption dOption;
    dOption.perfCmd = "profile jsperf 100";
    dOption.isStartWithDebug = false;
    dOption.processName = "test";
    dOption.isDebugApp = true;
    dOption.isStartWithNative = false;
    jsRuntime->jsEnv_->vm_ = nullptr;
    jsRuntime->DebuggerConnectionManager(isDebugApp, isStartWithDebug, dOption);
    jsRuntime.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    TAG_LOGI(AAFwkTag::TEST, "DebuggerConnectionManager_0200 end");
}
}  // namespace AbilityRuntime
}  // namespace OHOS