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

napi_value MarkDisconnectCallbackInvoked(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, nullptr, nullptr, &data);
    if (data != nullptr) {
        *static_cast<bool*>(data) = true;
    }
    return CreateJsUndefined(env);
}
}  // namespace

class JsUiServiceExtensionContextSecondTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    napi_env env_ = nullptr;
    panda::ecmascript::EcmaVM* vm_ = nullptr;
};

void JsUiServiceExtensionContextSecondTest::SetUpTestCase()
{
}

void JsUiServiceExtensionContextSecondTest::TearDownTestCase()
{
}

void JsUiServiceExtensionContextSecondTest::SetUp()
{
    panda::RuntimeOption pandaOption;
    vm_ = panda::JSNApi::CreateJSVM(pandaOption);
    if (vm_ == nullptr) {
        TAG_LOGE(AAFwkTag::TEST, "Create vm failed.");
        return;
    }

    env_ = reinterpret_cast<napi_env>(new ArkNativeEngine(vm_, nullptr));
}

void JsUiServiceExtensionContextSecondTest::TearDown()
{
    {
        std::lock_guard guard(g_connectsMutex);
        for (auto &item : g_connects) {
            if (item.second != nullptr) {
                item.second->RemoveConnectionObject();
            }
        }
        g_connects.clear();
        g_serialNumber = 0;
    }
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
 * @tc.name: CheckConnectionParam_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiServiceExtensionContextSecondTest, CheckConnectionParam_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckConnectionParam_0100 start");
    napi_value value{nullptr};
    std::shared_ptr<UIServiceExtensionContext> uiServiceExtensionContext =
        std::make_shared<UIServiceExtensionContext>();
    JSUIServiceExtensionContext jsUIServiceExtensionContext(uiServiceExtensionContext);
    sptr<JSUIServiceExtensionConnection> connection =  new JSUIServiceExtensionConnection(env_);
    Want want;
    int32_t accountId = 10;
    auto result = jsUIServiceExtensionContext.CheckConnectionParam(env_, value, connection, want, accountId);
    EXPECT_EQ(result, false);
    TAG_LOGI(AAFwkTag::TEST, "CheckConnectionParam_0100 end");
}

/**
 * @tc.name: CheckConnectionParam_0200
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiServiceExtensionContextSecondTest, CheckConnectionParam_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckConnectionParam_0200 start");
    napi_value value;
    napi_valuetype valueType = napi_undefined;
    napi_status status = napi_create_string_utf8(env_, "Hello, Service Extension!",
        NAPI_AUTO_LENGTH, &value);
    EXPECT_EQ(status, napi_ok);
    status = napi_typeof(env_, value, &valueType);
    std::shared_ptr<UIServiceExtensionContext> uiServiceExtensionContext =
        std::make_shared<UIServiceExtensionContext>();
    JSUIServiceExtensionContext jsUIServiceExtensionContext(uiServiceExtensionContext);
    sptr<JSUIServiceExtensionConnection> connection =  new JSUIServiceExtensionConnection(env_);
    Want want;
    int32_t accountId = 10;
    auto result = jsUIServiceExtensionContext.CheckConnectionParam(env_, value, connection, want, accountId);
    EXPECT_EQ(result, false);
    TAG_LOGI(AAFwkTag::TEST, "CheckConnectionParam_0200 end");
}

/**
 * @tc.name: GetConnectAbilityExecFunc_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiServiceExtensionContextSecondTest, GetConnectAbilityExecFunc_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetConnectAbilityExecFunc_0100 start");
    std::shared_ptr<UIServiceExtensionContext> uiServiceExtensionContext =
        std::make_shared<UIServiceExtensionContext>();
    JSUIServiceExtensionContext jsUIServiceExtensionContext(uiServiceExtensionContext);
    sptr<JSUIServiceExtensionConnection> connection =  new JSUIServiceExtensionConnection(env_);
    Want want;
    int64_t connectId = 10;
    std::shared_ptr<int> innerErrorCode = std::make_shared<int>(10);
    jsUIServiceExtensionContext.GetConnectAbilityExecFunc(want, connection, connectId, innerErrorCode);
    EXPECT_NE(innerErrorCode, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "GetConnectAbilityExecFunc_0100 end");
}

/**
 * @tc.name: FindConnection_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiServiceExtensionContextSecondTest, FindConnection_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FindConnection_0100 start");
    std::shared_ptr<UIServiceExtensionContext> uiServiceExtensionContext =
        std::make_shared<UIServiceExtensionContext>();
    JSUIServiceExtensionContext jsUIServiceExtensionContext(uiServiceExtensionContext);
    sptr<JSUIServiceExtensionConnection> connection =  new JSUIServiceExtensionConnection(env_);
    Want want;
    int64_t connectId = 10;
    int32_t accountId = 10;
    jsUIServiceExtensionContext.FindConnection(want, connection, connectId, accountId);
    EXPECT_NE(connection, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "FindConnection_0100 end");
}

/**
 * @tc.name: HandleOnAbilityDisconnectDone_0400
 * @tc.desc: A matched connection releases its JS reference and is removed after disconnect.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiServiceExtensionContextSecondTest, HandleOnAbilityDisconnectDone_0400, TestSize.Level1)
{
    bool callbackInvoked = false;
    napi_value connectionObject = nullptr;
    ASSERT_EQ(napi_create_object(env_, &connectionObject), napi_ok);
    napi_value onDisconnect = nullptr;
    ASSERT_EQ(napi_create_function(env_, "onDisconnect", NAPI_AUTO_LENGTH,
        MarkDisconnectCallbackInvoked, &callbackInvoked, &onDisconnect), napi_ok);
    ASSERT_EQ(napi_set_named_property(env_, connectionObject, "onDisconnect", onDisconnect), napi_ok);

    sptr<JSUIServiceExtensionConnection> connection = new JSUIServiceExtensionConnection(env_);
    connection->SetJsConnectionObject(connectionObject);
    connection->SetConnectionId(COMMECTION_ID);
    AppExecFwk::ElementName element("device", "com.example.uiservice", "UIServiceExtensionAbility");
    Want want;
    want.SetElement(element);
    ConnectionKey key;
    key.want = want;
    key.id = COMMECTION_ID;
    key.accountId = -1;
    {
        std::lock_guard guard(g_connectsMutex);
        g_connects.emplace(key, connection);
    }

    connection->HandleOnAbilityDisconnectDone(element, ERR_OK);

    EXPECT_TRUE(callbackInvoked);
    EXPECT_EQ(connection->jsConnectionObject_, nullptr);
    std::lock_guard guard(g_connectsMutex);
    EXPECT_TRUE(g_connects.empty());
}

/**
 * @tc.name: OnConnectServiceExtensionAbility_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiServiceExtensionContextSecondTest, OnConnectServiceExtensionAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnConnectServiceExtensionAbility_0100 start");
    NapiCallbackInfo info;
    std::shared_ptr<UIServiceExtensionContext> uiServiceExtensionContext =
        std::make_shared<UIServiceExtensionContext>();
    JSUIServiceExtensionContext jsUIServiceExtensionContext(uiServiceExtensionContext);
    auto result = jsUIServiceExtensionContext.OnConnectServiceExtensionAbility(env_, info);
    EXPECT_NE(info.argc, 0);
    TAG_LOGI(AAFwkTag::TEST, "OnConnectServiceExtensionAbility_0100 end");
}

/**
 * @tc.name: OnConnectServiceExtensionAbility_0200
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiServiceExtensionContextSecondTest, OnConnectServiceExtensionAbility_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnConnectServiceExtensionAbility_0200 start");
    NapiCallbackInfo info;
    napi_value value;
    napi_valuetype valueType = napi_undefined;
    napi_status status = napi_create_string_utf8(env_, "Hello, Service Extension!",
        NAPI_AUTO_LENGTH, &value);
    EXPECT_EQ(status, napi_ok);
    status = napi_typeof(env_, value, &valueType);
    std::shared_ptr<UIServiceExtensionContext> uiServiceExtensionContext =
        std::make_shared<UIServiceExtensionContext>();
    JSUIServiceExtensionContext jsUIServiceExtensionContext(uiServiceExtensionContext);
    auto result = jsUIServiceExtensionContext.OnConnectServiceExtensionAbility(env_, info);
    EXPECT_NE(result, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "OnConnectServiceExtensionAbility_0200 end");
}

/**
 * @tc.name: OnDisConnectServiceExtensionAbility_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiServiceExtensionContextSecondTest, OnDisConnectServiceExtensionAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnDisConnectServiceExtensionAbility_0100 start");
    NapiCallbackInfo info;
    napi_value value;
    napi_valuetype valueType = napi_undefined;
    napi_status status = napi_create_string_utf8(env_, "Hello, Service Extension!",
        NAPI_AUTO_LENGTH, &value);
    EXPECT_EQ(status, napi_ok);
    status = napi_typeof(env_, value, &valueType);
    std::shared_ptr<UIServiceExtensionContext> uiServiceExtensionContext =
        std::make_shared<UIServiceExtensionContext>();
    JSUIServiceExtensionContext jsUIServiceExtensionContext(uiServiceExtensionContext);
    auto result = jsUIServiceExtensionContext.OnDisConnectServiceExtensionAbility(env_, info);
    EXPECT_NE(result, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "OnDisConnectServiceExtensionAbility_0100 end");
}

/**
 * @tc.name: OnDisConnectServiceExtensionAbility_0200
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiServiceExtensionContextSecondTest, OnDisConnectServiceExtensionAbility_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnDisConnectServiceExtensionAbility_0200 start");
    NapiCallbackInfo info;
    std::shared_ptr<UIServiceExtensionContext> uiServiceExtensionContext =
        std::make_shared<UIServiceExtensionContext>();
    JSUIServiceExtensionContext jsUIServiceExtensionContext(uiServiceExtensionContext);
    auto result = jsUIServiceExtensionContext.OnDisConnectServiceExtensionAbility(env_, info);
    EXPECT_NE(result, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "OnDisConnectServiceExtensionAbility_0200 end");
}
}  // namespace AbilityRuntime
}  // namespace OHOS
