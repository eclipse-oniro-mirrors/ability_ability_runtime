/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "configuration.h"
#include "environment_callback.h"
#include "global_configuration_key.h"
#include "hilog_tag_wrapper.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "native_engine/native_engine.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AbilityRuntime;
using namespace AAFwk;

namespace {
constexpr int32_t TEST_MEMORY_LEVEL = 1;
}

class JsEnvironmentCallbackTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override;
    void TearDown() override;

protected:
    std::unique_ptr<JsRuntime> jsRuntime_ = nullptr;
    napi_env env_ = nullptr;
    std::shared_ptr<JsEnvironmentCallback> jsEnvCallback_ = nullptr;
};

void JsEnvironmentCallbackTest::SetUp()
{
    JsRuntime::Options options;
    jsRuntime_ = JsRuntime::Create(options);
    ASSERT_NE(jsRuntime_, nullptr);
    env_ = jsRuntime_->GetNapiEnv();
    ASSERT_NE(env_, nullptr);
    jsEnvCallback_ = std::make_shared<JsEnvironmentCallback>(env_);
    ASSERT_NE(jsEnvCallback_, nullptr);
}

void JsEnvironmentCallbackTest::TearDown()
{
    jsEnvCallback_ = nullptr;
    jsRuntime_.reset();
}

napi_value TestCallback(napi_env env, napi_callback_info info)
{
    return nullptr;
}

/**
 * @tc.name: JsEnvironmentCallback_OnConfigurationUpdated_001
 * @tc.desc: Test OnConfigurationUpdated with valid config and registered callback
 * @tc.type: FUNC
 */
HWTEST_F(JsEnvironmentCallbackTest, JsEnvironmentCallback_OnConfigurationUpdated_001, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::APPKIT, "JsEnvironmentCallback_OnConfigurationUpdated_001 start");
    napi_value callback = nullptr;
    napi_create_function(env_, "testCallback", NAPI_AUTO_LENGTH, TestCallback, nullptr, &callback);
    ASSERT_NE(callback, nullptr);

    int32_t callbackId = jsEnvCallback_->Register(callback, false);
    EXPECT_GE(callbackId, 0);

    AppExecFwk::Configuration config;
    config.AddItem(0, GlobalConfigurationKey::SYSTEM_LANGUAGE, "zh-CN");
    jsEnvCallback_->OnConfigurationUpdated(config);

    EXPECT_FALSE(jsEnvCallback_->IsEmpty());
    TAG_LOGI(AAFwkTag::APPKIT, "JsEnvironmentCallback_OnConfigurationUpdated_001 end");
}

/**
 * @tc.name: JsEnvironmentCallback_OnConfigurationUpdated_002
 * @tc.desc: Test OnConfigurationUpdated with sync callback
 * @tc.type: FUNC
 */
HWTEST_F(JsEnvironmentCallbackTest, JsEnvironmentCallback_OnConfigurationUpdated_002, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::APPKIT, "JsEnvironmentCallback_OnConfigurationUpdated_002 start");
    napi_value callback = nullptr;
    napi_create_function(env_, "testCallback", NAPI_AUTO_LENGTH, TestCallback, nullptr, &callback);
    ASSERT_NE(callback, nullptr);

    int32_t callbackId = jsEnvCallback_->Register(callback, true);
    EXPECT_GE(callbackId, 0);

    AppExecFwk::Configuration config;
    config.AddItem(0, GlobalConfigurationKey::SYSTEM_LANGUAGE, "en-US");
    jsEnvCallback_->OnConfigurationUpdated(config);

    EXPECT_FALSE(jsEnvCallback_->IsEmpty());
    TAG_LOGI(AAFwkTag::APPKIT, "JsEnvironmentCallback_OnConfigurationUpdated_002 end");
}

/**
 * @tc.name: JsEnvironmentCallback_OnConfigurationUpdated_003
 * @tc.desc: Test OnConfigurationUpdated with both sync and async callbacks
 * @tc.type: FUNC
 */
HWTEST_F(JsEnvironmentCallbackTest, JsEnvironmentCallback_OnConfigurationUpdated_003, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::APPKIT, "JsEnvironmentCallback_OnConfigurationUpdated_003 start");
    napi_value callback = nullptr;
    napi_create_function(env_, "testCallback", NAPI_AUTO_LENGTH, TestCallback, nullptr, &callback);
    ASSERT_NE(callback, nullptr);

    int32_t asyncCallbackId = jsEnvCallback_->Register(callback, false);
    EXPECT_GE(asyncCallbackId, 0);

    int32_t syncCallbackId = jsEnvCallback_->Register(callback, true);
    EXPECT_GE(syncCallbackId, 0);

    AppExecFwk::Configuration config;
    config.AddItem(0, "ohos.application.direction", "1");
    jsEnvCallback_->OnConfigurationUpdated(config);

    EXPECT_FALSE(jsEnvCallback_->IsEmpty());
    TAG_LOGI(AAFwkTag::APPKIT, "JsEnvironmentCallback_OnConfigurationUpdated_003 end");
}

/**
 * @tc.name: JsEnvironmentCallback_OnConfigurationUpdated_004
 * @tc.desc: Test OnConfigurationUpdated when callback is destroyed (weak_ptr lock fails)
 * @tc.type: FUNC
 */
HWTEST_F(JsEnvironmentCallbackTest, JsEnvironmentCallback_OnConfigurationUpdated_004, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::APPKIT, "JsEnvironmentCallback_OnConfigurationUpdated_004 start");
    auto testCallback = std::make_shared<JsEnvironmentCallback>(env_);
    ASSERT_NE(testCallback, nullptr);

    napi_value callback = nullptr;
    napi_create_function(env_, "testCallback", NAPI_AUTO_LENGTH, TestCallback, nullptr, &callback);
    ASSERT_NE(callback, nullptr);

    int32_t callbackId = testCallback->Register(callback, false);
    EXPECT_GE(callbackId, 0);

    AppExecFwk::Configuration config;
    testCallback->OnConfigurationUpdated(config);

    testCallback = nullptr;
    TAG_LOGI(AAFwkTag::APPKIT, "JsEnvironmentCallback_OnConfigurationUpdated_004 end");
}