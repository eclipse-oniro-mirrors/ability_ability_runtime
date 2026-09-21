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
#include <gtest/hwext/gtest-multithread.h>
#define private public
#include "js_ui_service_extension_context.h"
#include "js_ui_service_extension_context.cpp"
#undef private
#include "ability_business_error.h"
#include "errors.h"
#include "hilog_wrapper.h"
#include "ability_record.h"
#include "mock_ability_token.h"
#include "runtime.h"
#include "hilog_tag_wrapper.h"
#include "native_engine/impl/ark/ark_native_engine.h"
#include "native_engine/native_engine.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AAFwk;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AbilityRuntime {
namespace {
const int64_t  COMMECTION_ID = 100;
}  // namespace

class JsUiServiceExtensionContextFirstTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    napi_env env_ = nullptr;
    panda::ecmascript::EcmaVM* vm_ = nullptr;
};

void JsUiServiceExtensionContextFirstTest::SetUpTestCase()
{
}

void JsUiServiceExtensionContextFirstTest::TearDownTestCase()
{
}

void JsUiServiceExtensionContextFirstTest::SetUp()
{
    panda::RuntimeOption pandaOption;
    vm_ = panda::JSNApi::CreateJSVM(pandaOption);
    if (vm_ == nullptr) {
        TAG_LOGE(AAFwkTag::TEST, "Create vm failed.");
        return;
    }

    env_ = reinterpret_cast<napi_env>(new ArkNativeEngine(vm_, nullptr));
}

void JsUiServiceExtensionContextFirstTest::TearDown()
{
    if (env_ != nullptr) {
        delete reinterpret_cast<NativeEngine*>(env_);
        env_ = nullptr;
    }

    if (vm_ != nullptr) {
        panda::JSNApi::DestroyJSVM(vm_);
        vm_ = nullptr;
    }
}

/**
 * @tc.name: SetConnectionId_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiServiceExtensionContextFirstTest, SetConnectionId_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetConnectionId_0100 start");
    JSUIServiceExtensionConnection connection(env_);
    connection.SetConnectionId(COMMECTION_ID);
    EXPECT_EQ(connection.GetConnectionId(), COMMECTION_ID);
    TAG_LOGI(AAFwkTag::TEST, "SetConnectionId_0100 end");
}

/**
 * @tc.name: HandleOnAbilityConnectDone_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiServiceExtensionContextFirstTest, HandleOnAbilityConnectDone_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleOnAbilityConnectDone_0100 start");
    JSUIServiceExtensionConnection connection(env_);
    auto element = std::make_shared<AppExecFwk::ElementName>("bundlename", "appname", "abilityname");
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    int resultCode = 1;
    connection.jsConnectionObject_ = nullptr;
    connection.HandleOnAbilityConnectDone(*element, token, resultCode);
    EXPECT_EQ(connection.jsConnectionObject_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "HandleOnAbilityConnectDone_0100 end");
}

/**
 * @tc.name: HandleOnAbilityConnectDone_0200
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiServiceExtensionContextFirstTest, HandleOnAbilityConnectDone_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleOnAbilityConnectDone_0200 start");
    JSUIServiceExtensionConnection connection(env_);
    auto element = std::make_shared<AppExecFwk::ElementName>("bundlename", "appname", "abilityname");
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    int resultCode = 1;
    napi_value jsConnectionObject;
    napi_status status = napi_create_string_utf8(env_, "Hello, Service Extension!",
        NAPI_AUTO_LENGTH, &jsConnectionObject);
    EXPECT_EQ(status, napi_ok);
    connection.SetJsConnectionObject(jsConnectionObject);
    connection.HandleOnAbilityConnectDone(*element, token, resultCode);
    EXPECT_NE(connection.jsConnectionObject_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "HandleOnAbilityConnectDone_0200 end");
}

/**
 * @tc.name: HandleOnAbilityConnectDone_0300
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiServiceExtensionContextFirstTest, HandleOnAbilityConnectDone_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleOnAbilityConnectDone_0300 start");
    JSUIServiceExtensionConnection connection(env_);
    auto element = std::make_shared<AppExecFwk::ElementName>("bundlename", "appname", "abilityname");
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    int resultCode = 1;
    napi_value jsConnectionObject;
    napi_valuetype valueType = napi_undefined;
    napi_status status = napi_create_string_utf8(env_, "Hello, Service Extension!",
        NAPI_AUTO_LENGTH, &jsConnectionObject);
    status = napi_typeof(env_, jsConnectionObject, &valueType);
    EXPECT_EQ(status, napi_ok);
    connection.SetJsConnectionObject(jsConnectionObject);
    connection.HandleOnAbilityConnectDone(*element, token, resultCode);
    EXPECT_NE(connection.jsConnectionObject_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "HandleOnAbilityConnectDone_0300 end");
}


/**
 * @tc.name: HandleOnAbilityDisconnectDone_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiServiceExtensionContextFirstTest, HandleOnAbilityDisconnectDone_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleOnAbilityDisconnectDone_0100 start");
    JSUIServiceExtensionConnection connection(env_);
    auto element = std::make_shared<AppExecFwk::ElementName>("bundlename", "appname", "abilityname");
    int resultCode = 1;
    connection.HandleOnAbilityDisconnectDone(*element, resultCode);
    EXPECT_EQ(connection.jsConnectionObject_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "HandleOnAbilityDisconnectDone_0100 end");
}

/**
 * @tc.name: HandleOnAbilityDisconnectDone_0200
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiServiceExtensionContextFirstTest, HandleOnAbilityDisconnectDone_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleOnAbilityDisconnectDone_0200 start");
    JSUIServiceExtensionConnection connection(env_);
    auto element = std::make_shared<AppExecFwk::ElementName>("bundlename", "appname", "abilityname");
    int resultCode = 1;
    napi_value jsConnectionObject;
    napi_valuetype valueType = napi_undefined;
    napi_status status = napi_create_string_utf8(env_, "Hello, Service Extension!",
        NAPI_AUTO_LENGTH, &jsConnectionObject);
    EXPECT_EQ(status, napi_ok);
    connection.SetJsConnectionObject(jsConnectionObject);
    connection.HandleOnAbilityDisconnectDone(*element, resultCode);
    EXPECT_NE(connection.jsConnectionObject_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "HandleOnAbilityDisconnectDone_0200 start");
}

/**
 * @tc.name: HandleOnAbilityDisconnectDone_0300
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiServiceExtensionContextFirstTest, HandleOnAbilityDisconnectDone_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleOnAbilityDisconnectDone_0200 start");
    JSUIServiceExtensionConnection connection(env_);
    auto element = std::make_shared<AppExecFwk::ElementName>("bundlename", "appname", "abilityname");
    int resultCode = 1;
    napi_value jsConnectionObject;
    napi_valuetype valueType = napi_undefined;
    napi_status status = napi_create_string_utf8(env_, "Hello, Service Extension!",
        NAPI_AUTO_LENGTH, &jsConnectionObject);
    EXPECT_EQ(status, napi_ok);
    status = napi_typeof(env_, jsConnectionObject, &valueType);
    connection.SetJsConnectionObject(jsConnectionObject);
    connection.HandleOnAbilityDisconnectDone(*element, resultCode);
    EXPECT_NE(connection.jsConnectionObject_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "HandleOnAbilityDisconnectDone_0300 start");
}

/**
 * @tc.name: RemoveConnectionObject_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiServiceExtensionContextFirstTest, RemoveConnectionObject_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoveConnectionObject_0100 start");
    JSUIServiceExtensionConnection connection(env_);
    connection.RemoveConnectionObject();
    EXPECT_EQ(connection.jsConnectionObject_.get(), nullptr);
    TAG_LOGI(AAFwkTag::TEST, "RemoveConnectionObject_0100 end");
}

/**
 * @tc.name: CallJsFailed_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiServiceExtensionContextFirstTest, CallJsFailed_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CallJsFailed_0100 start");
    JSUIServiceExtensionConnection connection(env_);
    int32_t errorCode = 1;
    connection.CallJsFailed(errorCode);
    EXPECT_EQ(connection.jsConnectionObject_.get(), nullptr);
    TAG_LOGI(AAFwkTag::TEST, "CallJsFailed_0100 end");
}

/**
 * @tc.name: CallJsFailed_0200
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiServiceExtensionContextFirstTest, CallJsFailed_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CallJsFailed_0200 start");
    JSUIServiceExtensionConnection connection(env_);
    connection.RemoveConnectionObject();
    int32_t errorCode = 1;
    napi_value jsConnectionObject;
    napi_valuetype valueType = napi_undefined;
    napi_status status = napi_create_string_utf8(env_, "Hello, Service Extension!",
        NAPI_AUTO_LENGTH, &jsConnectionObject);
    EXPECT_EQ(status, napi_ok);
    status = napi_typeof(env_, jsConnectionObject, &valueType);
    connection.SetJsConnectionObject(jsConnectionObject);
    connection.CallJsFailed(errorCode);
    EXPECT_NE(connection.jsConnectionObject_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "CallJsFailed_0200 end");
}
/**
 * @tc.name: ScheduleStartAbilityByType_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiServiceExtensionContextFirstTest, ScheduleStartAbilityByType_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ScheduleStartAbilityByType_0100 start");
    ASSERT_NE(env_, nullptr);

    std::shared_ptr<UIServiceExtensionContext> context = nullptr;
    JSUIServiceExtensionContext jsContext(context);

    NapiCallbackInfo info;
    info.argc = 0;

    auto callback = std::make_shared<JsUIExtensionCallback>(env_);
    ASSERT_NE(callback, nullptr);
    auto innerErrCode = std::make_shared<ErrCode>(ERR_OK);
    ASSERT_NE(innerErrCode, nullptr);
    NapiAsyncTask::ExecuteCallback execute = []() {};

    napi_value result = jsContext.ScheduleStartAbilityByType(
        env_, info, callback, innerErrCode, std::move(execute));
    ASSERT_NE(result, nullptr);
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env_, result, &valueType);
    EXPECT_EQ(valueType, napi_object);

    TAG_LOGI(AAFwkTag::TEST, "ScheduleStartAbilityByType_0100 end");
}
}  // namespace AbilityRuntime
}  // namespace OHOS