/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <gtest/hwext/gtest-multithread.h>

#include "ability_manager_errors.h"
#include "ability_handler.h"
#include "ability_local_record.h"
#include "auto_fill_extension_context.h"
#include "configuration_convertor.h"
#include "ffrt.h"
#define private public
#define protected public
#include "ability_manager_client.h"
#include "configuration_utils.h"
#include "connection_manager.h"
#undef private
#undef protected
#include "ability_manager_proxy.h"
#include "hilog_tag_wrapper.h"
#include "iremote_object.h"
#define private public
#define protected public
#include "js_auto_fill_extension.h"
#include "js_auto_fill_extension_context.h"
#undef private
#undef protected
#include "js_auto_fill_extension_util.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "mock_ability_token.h"
#include "ohos_application.h"
#include "runtime.h"
#include "string_wrapper.h"
#include "token_setproc.h"
#include "want.h"
#ifdef SUPPORT_GRAPHICS
#include "locale_config.h"
#include "window_scene.h"
#endif

using namespace testing;
using namespace testing::ext;
using namespace testing::mt;

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr char JS_AUTO_FILL_EXTENSION_TASK_RUNNER[] = "JsAutoFillExtension";
constexpr char DEFAULT_LANGUAGE[] = "zh_CN";
constexpr uint64_t SYSTEM_APP_MASK = static_cast<uint64_t>(1) << 32;

napi_value CreateJsCallback(napi_env env, const char *name, napi_callback callback, void *data)
{
    napi_value result = nullptr;
    if (napi_create_function(env, name, NAPI_AUTO_LENGTH, callback, data, &result) != napi_ok) {
        return nullptr;
    }
    return result;
}

napi_status GetCallbackInfo(napi_env env, napi_callback_info info, size_t *argc, napi_value *argv, void **data)
{
    return napi_get_cb_info(env, info, argc, argv, nullptr, data);
}
} // namespace

class MockAutoFillExtensionContext : public AutoFillExtensionContext {
public:
    MOCK_METHOD(ErrCode, ConnectAbility,
        (const AAFwk::Want &want, const sptr<AbilityConnectCallback> &connectCallback), (const, override));
    MOCK_METHOD(ErrCode, DisconnectAbility,
        (const AAFwk::Want &want, const sptr<AbilityConnectCallback> &connectCallback), (const, override));
};

class MockAutoFillAbilityManager : public AAFwk::AbilityManagerProxy {
public:
    MockAutoFillAbilityManager() : AbilityManagerProxy(nullptr) {}
    MOCK_METHOD(int, DisconnectAbility, (sptr<AAFwk::IAbilityConnection> connect), (override));
};

class JsAutoFillExtensionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::shared_ptr<JsAutoFillExtension> jsAutoFillExtension_ = nullptr;
    std::shared_ptr<ApplicationContext> applicationContext_ = nullptr;
    std::unique_ptr<Runtime> jsRuntime_ = nullptr;
    sptr<AbilityConnectCallback> connection_;
    uint64_t selfTokenId_ = 0;
    int32_t errorCode_ = 0;
    bool promiseResolved_ = false;

private:
    void CreateJsAutoFillExtension();
};

void JsAutoFillExtensionTest::SetUpTestCase()
{
    ffrt::submit([]() {});
    ffrt::wait();
}

void JsAutoFillExtensionTest::TearDownTestCase()
{}

void JsAutoFillExtensionTest::SetUp()
{
    selfTokenId_ = GetSelfTokenID();
    ASSERT_EQ(SetSelfTokenID(selfTokenId_ | SYSTEM_APP_MASK), 0);
    CreateJsAutoFillExtension();
}

void JsAutoFillExtensionTest::TearDown()
{
    if (jsRuntime_ != nullptr) {
        auto env = static_cast<JsRuntime &>(*jsRuntime_).GetNapiEnv();
        if (env != nullptr) {
            // Finish pending callbacks while the fixture's callback data is still alive.
            reinterpret_cast<NativeEngine *>(env)->CleanupHandles();
        }
    }
    if (connection_ != nullptr) {
        auto connection = static_cast<JSAutoFillExtensionConnection *>(connection_.GetRefPtr());
        connection->HandleOnAbilityDisconnectDone(AppExecFwk::ElementName(), ERR_OK);
        connection_ = nullptr;
    }
    jsAutoFillExtension_ = nullptr;
    jsRuntime_.reset();
    EXPECT_EQ(SetSelfTokenID(selfTokenId_), 0);
}

void JsAutoFillExtensionTest::CreateJsAutoFillExtension()
{
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token(new (std::nothrow) MockAbilityToken());
    auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);

    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityRuntime::ContextImpl> contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();

    Configuration config;
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, DEFAULT_LANGUAGE);
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, ConfigurationInner::COLOR_MODE_LIGHT);
    contextImpl->SetConfiguration(std::make_shared<Configuration>(config));

    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
#ifdef SUPPORT_GRAPHICS
    UErrorCode status = U_ZERO_ERROR;
    icu::Locale locale = icu::Locale::forLanguageTag("zh", status);
    TAG_LOGI(AAFwkTag::TEST, "language: %{public}s, script: %{public}s, region: %{public}s", locale.getLanguage(),
             locale.getScript(), locale.getCountry());
    resConfig->SetLocaleInfo(locale);
#endif
    Global::Resource::RState updateRet = resourceManager->UpdateResConfig(*resConfig);
    if (updateRet != Global::Resource::RState::SUCCESS) {
        TAG_LOGE(AAFwkTag::TEST, "Init locale failed.");
    }
    contextImpl->SetResourceManager(resourceManager);

    std::shared_ptr<AbilityRuntime::ApplicationContext> applicationContext =
        AbilityRuntime::ApplicationContext::GetInstance();
    applicationContext->AttachContextImpl(contextImpl);
    application->SetApplicationContext(applicationContext);
    applicationContext_ = applicationContext;

    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(nullptr);

    auto eventRunner = AppExecFwk::EventRunner::Create(JS_AUTO_FILL_EXTENSION_TASK_RUNNER);
    Runtime::Options options;
    options.preload = true;
    options.eventRunner = eventRunner;
    jsRuntime_ = JsRuntime::Create(options);
    ASSERT_NE(jsRuntime_, nullptr);

    JsAutoFillExtension *extension = JsAutoFillExtension::Create(jsRuntime_);
    ASSERT_NE(extension, nullptr);
    jsAutoFillExtension_.reset(extension);

    jsAutoFillExtension_->Init(record, application, handler, token);
}

/**
 * @tc.name: Configuration_0100
 * @tc.desc: Js auto fill extension init.
 * @tc.type: FUNC
 */
HWTEST_F(JsAutoFillExtensionTest, Init_0100, TestSize.Level1)
{
    ASSERT_NE(jsAutoFillExtension_, nullptr);
    ASSERT_NE(applicationContext_, nullptr);

    auto context = jsAutoFillExtension_->GetContext();
    ASSERT_NE(context, nullptr);
    auto configuration = context->GetConfiguration();
    EXPECT_NE(configuration, nullptr);
    auto appConfig = applicationContext_->GetConfiguration();
    EXPECT_NE(appConfig, nullptr);
}

/**
 * @tc.name: OnStart_0100
 * @tc.desc: Js auto fill extension OnStart.
 * @tc.type: FUNC
 */
HWTEST_F(JsAutoFillExtensionTest, OnStart_0100, TestSize.Level1)
{
    Want want;
    ASSERT_NE(jsAutoFillExtension_, nullptr);
    sptr<AAFwk::SessionInfo> sessionInfo = new (std::nothrow) AAFwk::SessionInfo();
    ASSERT_NE(sessionInfo, nullptr);
    jsAutoFillExtension_->OnStart(want, sessionInfo);

    auto context = jsAutoFillExtension_->GetContext();
    ASSERT_NE(context, nullptr);
    auto configuration = context->GetConfiguration();
    EXPECT_NE(configuration, nullptr);
    auto resourceManager = context->GetResourceManager();
    EXPECT_NE(resourceManager, nullptr);
    auto appConfig = applicationContext_->GetConfiguration();
    EXPECT_NE(appConfig, nullptr);
}

/**
 * @tc.name: OnReloadInModal_0100
 * @tc.desc: Js auto fill extension OnReloadInModal.
 * @tc.type: FUNC
 */
HWTEST_F(JsAutoFillExtensionTest, OnReloadInModal_0100, TestSize.Level1)
{
    CustomData customData;
    ASSERT_NE(jsAutoFillExtension_, nullptr);
    jsAutoFillExtension_->isPopup_ = true;
    auto ret = jsAutoFillExtension_->OnReloadInModal(nullptr, customData);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);

    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    ret = jsAutoFillExtension_->OnReloadInModal(sessionInfo, customData);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
}

/**
 * @tc.name: ConnectServiceExtensionAbility_0100
 * @tc.desc: An expired context reports 16000011 asynchronously through onFailed.
 * @tc.type: FUNC
 */
HWTEST_F(JsAutoFillExtensionTest, ConnectServiceExtensionAbility_0100, TestSize.Level1)
{
    ASSERT_NE(jsRuntime_, nullptr);
    auto env = static_cast<JsRuntime &>(*jsRuntime_).GetNapiEnv();
    ASSERT_NE(env, nullptr);
    HandleScope handleScope(env);
    auto context = std::make_shared<AutoFillExtensionContext>();
    JsAutoFillExtensionContext jsContext(context);
    context.reset();
    napi_callback onFailed = [](napi_env env, napi_callback_info info) -> napi_value {
        size_t argc = 1;
        napi_value argv[1] = {nullptr};
        void *data = nullptr;
        GetCallbackInfo(env, info, &argc, argv, &data);
        napi_get_value_int32(env, argv[0], static_cast<int32_t *>(data));
        return CreateJsUndefined(env);
    };
    auto callback = CreateJsCallback(env, "onFailed", onFailed, &errorCode_);
    ASSERT_NE(callback, nullptr);
    NapiCallbackInfo info{2};
    ASSERT_EQ(napi_create_object(env, &info.argv[0]), napi_ok);
    ASSERT_EQ(napi_create_object(env, &info.argv[1]), napi_ok);
    ASSERT_EQ(napi_set_named_property(env, info.argv[1], "onFailed", callback), napi_ok);

    auto result = jsContext.OnConnectAbility(env, info);
    ASSERT_NE(result, nullptr);
    int64_t connectId = -1;
    EXPECT_EQ(napi_get_value_int64(env, result, &connectId), napi_ok);
    EXPECT_GE(connectId, 0);
    EXPECT_EQ(errorCode_, 0);
    for (int32_t i = 0; i < 50 && errorCode_ == 0; ++i) {
        usleep(100000);
        reinterpret_cast<NativeEngine *>(env)->Loop(LOOP_NOWAIT);
    }
    EXPECT_EQ(errorCode_, 16000011);
}

/**
 * @tc.name: ConnectServiceExtensionAbility_0200
 * @tc.desc: An unresolved ability reports 16000001 through onFailed.
 * @tc.type: FUNC
 */
HWTEST_F(JsAutoFillExtensionTest, ConnectServiceExtensionAbility_0200, TestSize.Level1)
{
    ASSERT_NE(jsRuntime_, nullptr);
    auto env = static_cast<JsRuntime &>(*jsRuntime_).GetNapiEnv();
    ASSERT_NE(env, nullptr);
    HandleScope handleScope(env);
    auto context = std::make_shared<MockAutoFillExtensionContext>();
    EXPECT_CALL(*context, ConnectAbility(_, _)).WillOnce(Return(AAFwk::RESOLVE_ABILITY_ERR));
    JsAutoFillExtensionContext jsContext(context);
    napi_callback onFailed = [](napi_env env, napi_callback_info info) -> napi_value {
        size_t argc = 1;
        napi_value argv[1] = {nullptr};
        void *data = nullptr;
        GetCallbackInfo(env, info, &argc, argv, &data);
        napi_get_value_int32(env, argv[0], static_cast<int32_t *>(data));
        return CreateJsUndefined(env);
    };
    auto callback = CreateJsCallback(env, "onFailed", onFailed, &errorCode_);
    ASSERT_NE(callback, nullptr);
    NapiCallbackInfo info{2};
    ASSERT_EQ(napi_create_object(env, &info.argv[0]), napi_ok);
    ASSERT_EQ(napi_create_object(env, &info.argv[1]), napi_ok);
    ASSERT_EQ(napi_set_named_property(env, info.argv[1], "onFailed", callback), napi_ok);

    auto result = jsContext.OnConnectAbility(env, info);
    ASSERT_NE(result, nullptr);
    for (int32_t i = 0; i < 50 && errorCode_ == 0; ++i) {
        usleep(100000);
        reinterpret_cast<NativeEngine *>(env)->Loop(LOOP_NOWAIT);
    }
    EXPECT_EQ(errorCode_, 16000001);
}

/**
 * @tc.name: ConnectServiceExtensionAbility_0300
 * @tc.desc: A non-system app throws 202 before connecting to a service.
 * @tc.type: FUNC
 */
HWTEST_F(JsAutoFillExtensionTest, ConnectServiceExtensionAbility_0300, TestSize.Level1)
{
    ASSERT_NE(jsRuntime_, nullptr);
    auto env = static_cast<JsRuntime &>(*jsRuntime_).GetNapiEnv();
    ASSERT_NE(env, nullptr);
    HandleScope handleScope(env);
    auto context = std::make_shared<MockAutoFillExtensionContext>();
    EXPECT_CALL(*context, ConnectAbility(_, _)).Times(0);
    auto object = JsAutoFillExtensionContext::CreateJsAutoFillExtensionContext(env, context);
    ASSERT_NE(object, nullptr);
    napi_value method = nullptr;
    ASSERT_EQ(napi_get_named_property(env, object, "connectServiceExtensionAbility", &method), napi_ok);
    napi_value argv[2] = {nullptr};
    ASSERT_EQ(napi_create_object(env, &argv[0]), napi_ok);
    ASSERT_EQ(napi_create_object(env, &argv[1]), napi_ok);
    ASSERT_EQ(SetSelfTokenID(selfTokenId_ & ~SYSTEM_APP_MASK), 0);

    auto engine = reinterpret_cast<NativeEngine *>(env);
    EXPECT_EQ(engine->CallFunction(object, method, argv, 2), nullptr);
    napi_value error = nullptr;
    ASSERT_EQ(napi_get_and_clear_last_exception(env, &error), napi_ok);
    ASSERT_NE(error, nullptr);
    napi_value code = nullptr;
    ASSERT_EQ(napi_get_named_property(env, error, "code", &code), napi_ok);
    int32_t errorCode = 0;
    ASSERT_EQ(napi_get_value_int32(env, code, &errorCode), napi_ok);
    EXPECT_EQ(errorCode, 202);
}

/**
 * @tc.name: DisconnectServiceExtensionAbility_0100
 * @tc.desc: A missing context rejects the Promise with 16000011.
 * @tc.type: FUNC
 */
HWTEST_F(JsAutoFillExtensionTest, DisconnectServiceExtensionAbility_0100, TestSize.Level1)
{
    ASSERT_NE(jsRuntime_, nullptr);
    auto env = static_cast<JsRuntime &>(*jsRuntime_).GetNapiEnv();
    ASSERT_NE(env, nullptr);
    HandleScope handleScope(env);
    JsAutoFillExtensionContext jsContext(nullptr);
    NapiCallbackInfo info{1};
    ASSERT_EQ(napi_create_int64(env, 0, &info.argv[0]), napi_ok);
    auto result = jsContext.OnDisconnectAbility(env, info);
    ASSERT_NE(result, nullptr);
    bool isPromise = false;
    ASSERT_EQ(napi_is_promise(env, result, &isPromise), napi_ok);
    ASSERT_TRUE(isPromise);

    napi_callback onRejected = [](napi_env env, napi_callback_info info) -> napi_value {
        size_t argc = 1;
        napi_value argv[1] = {nullptr};
        void *data = nullptr;
        GetCallbackInfo(env, info, &argc, argv, &data);
        napi_value code = nullptr;
        napi_get_named_property(env, argv[0], "code", &code);
        napi_get_value_int32(env, code, static_cast<int32_t *>(data));
        return CreateJsUndefined(env);
    };
    auto rejectCallback = CreateJsCallback(env, "onRejected", onRejected, &errorCode_);
    ASSERT_NE(rejectCallback, nullptr);
    napi_value catchMethod = nullptr;
    ASSERT_EQ(napi_get_named_property(env, result, "catch", &catchMethod), napi_ok);
    ASSERT_NE(reinterpret_cast<NativeEngine *>(env)->CallFunction(result, catchMethod, &rejectCallback, 1), nullptr);
    for (int32_t i = 0; i < 50 && errorCode_ == 0; ++i) {
        usleep(100000);
        reinterpret_cast<NativeEngine *>(env)->Loop(LOOP_NOWAIT);
    }
    EXPECT_EQ(errorCode_, 16000011);
}

/**
 * @tc.name: DisconnectServiceExtensionAbility_0200
 * @tc.desc: An unknown connection rejects the Promise with 16000050.
 * @tc.type: FUNC
 */
HWTEST_F(JsAutoFillExtensionTest, DisconnectServiceExtensionAbility_0200, TestSize.Level1)
{
    ASSERT_NE(jsRuntime_, nullptr);
    auto env = static_cast<JsRuntime &>(*jsRuntime_).GetNapiEnv();
    ASSERT_NE(env, nullptr);
    HandleScope handleScope(env);
    auto context = std::make_shared<MockAutoFillExtensionContext>();
    EXPECT_CALL(*context, DisconnectAbility(_, _)).Times(0);
    JsAutoFillExtensionContext jsContext(context);
    NapiCallbackInfo info{1};
    ASSERT_EQ(napi_create_int64(env, -1, &info.argv[0]), napi_ok);

    auto result = jsContext.OnDisconnectAbility(env, info);
    ASSERT_NE(result, nullptr);
    bool isPromise = false;
    ASSERT_EQ(napi_is_promise(env, result, &isPromise), napi_ok);
    ASSERT_TRUE(isPromise);
    napi_callback onRejected = [](napi_env env, napi_callback_info info) -> napi_value {
        size_t argc = 1;
        napi_value argv[1] = {nullptr};
        void *data = nullptr;
        GetCallbackInfo(env, info, &argc, argv, &data);
        napi_value code = nullptr;
        napi_get_named_property(env, argv[0], "code", &code);
        napi_get_value_int32(env, code, static_cast<int32_t *>(data));
        return CreateJsUndefined(env);
    };
    auto rejectCallback = CreateJsCallback(env, "onRejected", onRejected, &errorCode_);
    ASSERT_NE(rejectCallback, nullptr);
    napi_value catchMethod = nullptr;
    ASSERT_EQ(napi_get_named_property(env, result, "catch", &catchMethod), napi_ok);
    ASSERT_NE(reinterpret_cast<NativeEngine *>(env)->CallFunction(result, catchMethod, &rejectCallback, 1), nullptr);
    for (int32_t i = 0; i < 50 && errorCode_ == 0; ++i) {
        usleep(100000);
        reinterpret_cast<NativeEngine *>(env)->Loop(LOOP_NOWAIT);
    }
    EXPECT_EQ(errorCode_, 16000050);
}

/**
 * @tc.name: DisconnectServiceExtensionAbility_0300
 * @tc.desc: A non-system app throws 202 before disconnecting from a service.
 * @tc.type: FUNC
 */
HWTEST_F(JsAutoFillExtensionTest, DisconnectServiceExtensionAbility_0300, TestSize.Level1)
{
    ASSERT_NE(jsRuntime_, nullptr);
    auto env = static_cast<JsRuntime &>(*jsRuntime_).GetNapiEnv();
    ASSERT_NE(env, nullptr);
    HandleScope handleScope(env);
    auto context = std::make_shared<MockAutoFillExtensionContext>();
    EXPECT_CALL(*context, DisconnectAbility(_, _)).Times(0);
    auto object = JsAutoFillExtensionContext::CreateJsAutoFillExtensionContext(env, context);
    ASSERT_NE(object, nullptr);
    napi_value method = nullptr;
    ASSERT_EQ(napi_get_named_property(env, object, "disconnectServiceExtensionAbility", &method), napi_ok);
    napi_value connectionId = nullptr;
    ASSERT_EQ(napi_create_int64(env, 1, &connectionId), napi_ok);
    ASSERT_EQ(SetSelfTokenID(selfTokenId_ & ~SYSTEM_APP_MASK), 0);

    auto engine = reinterpret_cast<NativeEngine *>(env);
    EXPECT_EQ(engine->CallFunction(object, method, &connectionId, 1), nullptr);
    napi_value error = nullptr;
    ASSERT_EQ(napi_get_and_clear_last_exception(env, &error), napi_ok);
    ASSERT_NE(error, nullptr);
    napi_value code = nullptr;
    ASSERT_EQ(napi_get_named_property(env, error, "code", &code), napi_ok);
    int32_t errorCode = 0;
    ASSERT_EQ(napi_get_value_int32(env, code, &errorCode), napi_ok);
    EXPECT_EQ(errorCode, 202);
}

/**
 * @tc.name: DisconnectServiceExtensionAbility_0400
 * @tc.desc: Successful disconnection resolves the Promise and releases the connection.
 * @tc.type: FUNC
 */
HWTEST_F(JsAutoFillExtensionTest, DisconnectServiceExtensionAbility_0400, TestSize.Level1)
{
    ASSERT_NE(jsRuntime_, nullptr);
    auto env = static_cast<JsRuntime &>(*jsRuntime_).GetNapiEnv();
    ASSERT_NE(env, nullptr);
    HandleScope handleScope(env);
    auto engine = reinterpret_cast<NativeEngine *>(env);
    auto context = std::make_shared<MockAutoFillExtensionContext>();
    EXPECT_CALL(*context, ConnectAbility(_, _)).WillOnce(DoAll(SaveArg<1>(&connection_), Return(ERR_OK)));
    EXPECT_CALL(*context, DisconnectAbility(_, _)).WillOnce(Return(ERR_OK));
    JsAutoFillExtensionContext jsContext(context);
    NapiCallbackInfo connectInfo{2};
    ASSERT_EQ(napi_create_object(env, &connectInfo.argv[0]), napi_ok);
    ASSERT_EQ(napi_create_object(env, &connectInfo.argv[1]), napi_ok);

    NapiCallbackInfo info{1};
    info.argv[0] = jsContext.OnConnectAbility(env, connectInfo);
    ASSERT_NE(info.argv[0], nullptr);
    engine->CleanupHandles();
    ASSERT_NE(connection_, nullptr);
    auto result = jsContext.OnDisconnectAbility(env, info);
    ASSERT_NE(result, nullptr);
    napi_callback onResolved = [](napi_env env, napi_callback_info info) -> napi_value {
        void *data = nullptr;
        GetCallbackInfo(env, info, nullptr, nullptr, &data);
        *static_cast<bool *>(data) = true;
        return CreateJsUndefined(env);
    };
    auto resolveCallback = CreateJsCallback(env, "onResolved", onResolved, &promiseResolved_);
    ASSERT_NE(resolveCallback, nullptr);
    napi_value thenMethod = nullptr;
    ASSERT_EQ(napi_get_named_property(env, result, "then", &thenMethod), napi_ok);
    ASSERT_NE(engine->CallFunction(result, thenMethod, &resolveCallback, 1), nullptr);
    engine->CleanupHandles();
    engine->Loop(LOOP_NOWAIT);
    ASSERT_TRUE(promiseResolved_);

    auto connection = static_cast<JSAutoFillExtensionConnection *>(connection_.GetRefPtr());
    connection->HandleOnAbilityDisconnectDone(AppExecFwk::ElementName(), ERR_OK);
    EXPECT_EQ(connection->GetJsConnectionObject(), nullptr);
    connection_ = nullptr;
}

/**
 * @tc.name: OnStop_0100
 * @tc.desc: Stopping the extension disconnects and removes its service connections.
 * @tc.type: FUNC
 */
HWTEST_F(JsAutoFillExtensionTest, OnStop_0100, TestSize.Level1)
{
    ASSERT_NE(jsAutoFillExtension_, nullptr);
    auto context = jsAutoFillExtension_->GetContext();
    ASSERT_NE(context, nullptr);
    auto token = context->GetToken();
    ASSERT_NE(token, nullptr);
    auto firstConnection = sptr<AbilityConnection>::MakeSptr();
    auto secondConnection = sptr<AbilityConnection>::MakeSptr();
    Want want;
    want.SetElementName("com.example.autofill", "FirstService");
    ConnectionInfo firstInfo(token, want.GetOperation(), firstConnection);
    want.SetElementName("com.example.autofill", "SecondService");
    ConnectionInfo secondInfo(token, want.GetOperation(), secondConnection);
    auto &connections = ConnectionManager::GetInstance().abilityConnections_;
    connections.emplace(firstInfo, std::vector<sptr<AbilityConnectCallback>>{});
    connections.emplace(secondInfo, std::vector<sptr<AbilityConnectCallback>>{});
    auto abilityManager = sptr<MockAutoFillAbilityManager>::MakeSptr();
    EXPECT_CALL(*abilityManager, DisconnectAbility(sptr<AAFwk::IAbilityConnection>(firstConnection)))
        .WillOnce(Return(ERR_OK));
    EXPECT_CALL(*abilityManager, DisconnectAbility(sptr<AAFwk::IAbilityConnection>(secondConnection)))
        .WillOnce(Return(ERR_OK));
    auto client = AAFwk::AbilityManagerClient::GetInstance();
    auto originalProxy = client->proxy_;
    client->proxy_ = abilityManager;

    jsAutoFillExtension_->OnStop();

    client->proxy_ = originalProxy;
    EXPECT_EQ(connections.count(firstInfo), 0);
    EXPECT_EQ(connections.count(secondInfo), 0);
    connections.erase(firstInfo);
    connections.erase(secondInfo);
}

} // namespace AbilityRuntime
} // namespace OHOS
