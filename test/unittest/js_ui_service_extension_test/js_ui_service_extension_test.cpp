/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include <gmock/gmock.h>

#define private public
#define protected public
#include "js_ui_service_extension.h"
#include "ui_service_extension_module_loader.h"
#include "js_ui_service_extension_context.cpp"
#undef private
#undef protected

#include "js_runtime_lite.h"
#include "mock_ability_token.h"
#include "ability_handler.h"
#include "ohos_application.h"
#include "runtime.h"
#include "ui_service_extension_connection_constants.h"
#include "ui_service_host_stub.h"

using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {

class ServiceHostStubImpl : public AAFwk::UIServiceHostStub {
public:
    virtual int32_t SendData(OHOS::AAFwk::WantParams &data)
    {
        return 0;
    }
};

class MockWindow : public Rosen::Window {
public:
    virtual Ace::UIContent* GetUIContent() const
    {
        return uiContent_.get();
    }

    std::unique_ptr<Ace::UIContent> uiContent_ = Ace::UIContent::Create(nullptr, nullptr);
};

class NativeReferenceMock : public NativeReference {
public:
    NativeReferenceMock() = default;
    virtual ~NativeReferenceMock() = default;
    MOCK_METHOD0(Ref, uint32_t());
    MOCK_METHOD0(Unref, uint32_t());
    MOCK_METHOD0(Get, napi_value());
    MOCK_METHOD0(GetData, void*());
    virtual operator napi_value() override
    {
        return reinterpret_cast<napi_value>(this);
    }
    MOCK_METHOD0(SetDeleteSelf, void());
    MOCK_METHOD0(GetRefCount, uint32_t());
    MOCK_METHOD0(GetFinalRun, bool());
    napi_value GetNapiValue() override
    {
        napi_env env{nullptr};
        napi_value object = AppExecFwk::CreateJSObject(env);
        return object;
    }
};

class JsUIServiceExtensionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::unique_ptr<Runtime> runtime;
    std::shared_ptr<JsUIServiceExtension> jsUIServiceExtension;
};

void JsUIServiceExtensionTest::SetUpTestCase()
{}

void JsUIServiceExtensionTest::TearDownTestCase()
{}

void JsUIServiceExtensionTest::SetUp()
{
    Runtime::Options options;
    runtime = Runtime::Create(options);
    jsUIServiceExtension.reset(JsUIServiceExtension::Create(runtime));

    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    info->name = "JsUIServiceExtensionTest";
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(info, nullptr, nullptr, 0);
    std::shared_ptr<AppExecFwk::OHOSApplication> application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityHandler> handler = std::make_shared<AppExecFwk::AbilityHandler>(nullptr);
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();

    AppExecFwk::Configuration configuration;
    std::shared_ptr<AbilityRuntime::ContextImpl> contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    contextImpl->SetConfiguration(std::make_shared<AppExecFwk::Configuration>(configuration));
    std::shared_ptr<AbilityRuntime::ApplicationContext> applicationContext =
        AbilityRuntime::ApplicationContext::GetInstance();
    applicationContext->AttachContextImpl(contextImpl);
    application->SetApplicationContext(applicationContext);

    jsUIServiceExtension->Init(record, application, handler, token);
}

void JsUIServiceExtensionTest::TearDown()
{}

/**
 * @tc.number: OnAddSystemAbility_0100
 * @tc.name: OnAddSystemAbility
 * @tc.desc: SystemAbilityStatusChangeListener OnAddSystemAbility
 */
HWTEST_F(JsUIServiceExtensionTest, OnAddSystemAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnAddSystemAbility_0100 start");

    constexpr int32_t WINDOW_MANAGER_SERVICE_ID = 4606;
    std::string deviceId{"deviceId"};
    JsUIServiceExtension::SystemAbilityStatusChangeListener systemAbilityStatusChangeListener{nullptr};
    systemAbilityStatusChangeListener.OnAddSystemAbility(WINDOW_MANAGER_SERVICE_ID, deviceId);

    constexpr int32_t UNAVAILABLE_SERVICE_ID = 0;
    systemAbilityStatusChangeListener.OnAddSystemAbility(UNAVAILABLE_SERVICE_ID, deviceId);
    EXPECT_TRUE(jsUIServiceExtension != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "OnAddSystemAbility_0100 end");
}

/**
 * @tc.number: OnAddSystemAbility_0100
 * @tc.name: BindContext
 * @tc.desc: JsUIServiceExtension BindContext
 */
HWTEST_F(JsUIServiceExtensionTest, BindContext_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "BindContext_0100 start");

    napi_env env{nullptr};
    napi_value object{nullptr};

    jsUIServiceExtension->BindContext(env, object);
    EXPECT_TRUE(jsUIServiceExtension != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "BindContext_0100 end");
}

/**
 * @tc.number: OnStartAndStop_0100
 * @tc.name: OnStartAndStop
 * @tc.desc: JsUIServiceExtension OnStart and OnStop
 */
HWTEST_F(JsUIServiceExtensionTest, OnStartAndStop_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnStartAndStop_0100 start");

    AAFwk::Want want;

    jsUIServiceExtension->OnStart(want);
    jsUIServiceExtension->OnStop();
    EXPECT_TRUE(jsUIServiceExtension != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "OnStartAndStop_0100 end");
}

/**
 * @tc.number: OnCommand_0100
 * @tc.name: OnCommand
 * @tc.desc: JsUIServiceExtension OnCommand
 */
HWTEST_F(JsUIServiceExtensionTest, OnCommand_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnCommand_0100 start");

    AAFwk::Want want;
    bool restart{true};
    int startId{0};
    jsUIServiceExtension->firstRequest_ = true;
    jsUIServiceExtension->OnCommand(want, restart, startId);
    EXPECT_TRUE(jsUIServiceExtension != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "OnCommand_0100 end");
}

/**
 * @tc.number: CallObjectMethod_0100
 * @tc.name: CallObjectMethod
 * @tc.desc: JsUIServiceExtension CallObjectMethod
 */
HWTEST_F(JsUIServiceExtensionTest, CallObjectMethod_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CallObjectMethod_0100 start");

    napi_value object{nullptr};
    size_t argc{0};
    jsUIServiceExtension->jsObj_ = nullptr;
    auto result = jsUIServiceExtension->CallObjectMethod("Test", &object, argc);
    EXPECT_EQ(result, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "CallObjectMethod_0100 end");
}

/**
 * @tc.number: CallObjectMethod_0200
 * @tc.name: CallObjectMethod
 * @tc.desc: JsUIServiceExtension CallObjectMethod
 */
HWTEST_F(JsUIServiceExtensionTest, CallObjectMethod_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CallObjectMethod_0200 start");

    napi_value object{nullptr};
    size_t argc{0};

    jsUIServiceExtension->jsObj_ = std::make_unique<NativeReferenceMock>();
    auto result = jsUIServiceExtension->CallObjectMethod("Test", &object, argc);
    EXPECT_EQ(result, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "CallObjectMethod_0200 end");
}

/**
 * @tc.number: GetSrcPath_0100
 * @tc.name: GetSrcPath
 * @tc.desc: JsUIServiceExtension GetSrcPath
 */
HWTEST_F(JsUIServiceExtensionTest, GetSrcPath_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetSrcPath_0100 start");

    std::string srcPath{"Test.Test"};
    jsUIServiceExtension->abilityInfo_->srcEntrance = "Test";
    jsUIServiceExtension->GetSrcPath(srcPath);
    EXPECT_TRUE(jsUIServiceExtension != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "GetSrcPath_0100 end");
}

/**
 * @tc.number: GetSrcPath_0200
 * @tc.name: GetSrcPath
 * @tc.desc: JsUIServiceExtension GetSrcPath with normal srcEntrance that has file extension.
 */
HWTEST_F(JsUIServiceExtensionTest, GetSrcPath_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetSrcPath_0200 start");
    jsUIServiceExtension->abilityInfo_->moduleName = "entry";
    jsUIServiceExtension->abilityInfo_->srcEntrance = "ets/pages/Index.ts";
    std::string srcPath;
    jsUIServiceExtension->GetSrcPath(srcPath);
    EXPECT_EQ(srcPath, "entry/ets/pages/Index.abc");
    TAG_LOGI(AAFwkTag::TEST, "GetSrcPath_0200 end");
}

/**
 * @tc.number: GetSrcPath_0300
 * @tc.name: GetSrcPath
 * @tc.desc: JsUIServiceExtension GetSrcPath where folder name contains dot but filename has no extension.
 */
HWTEST_F(JsUIServiceExtensionTest, GetSrcPath_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetSrcPath_0300 start");
    jsUIServiceExtension->abilityInfo_->moduleName = "entry";
    jsUIServiceExtension->abilityInfo_->srcEntrance = "v1.0/pages/Index";
    std::string srcPath;
    jsUIServiceExtension->GetSrcPath(srcPath);
    EXPECT_EQ(srcPath, "entry/v1.0/pages/Index.abc");
    TAG_LOGI(AAFwkTag::TEST, "GetSrcPath_0300 end");
}

/**
 * @tc.number: GetSrcPath_0400
 * @tc.name: GetSrcPath
 * @tc.desc: JsUIServiceExtension GetSrcPath where path has no dot, should not erase and not go out of bounds.
 */
HWTEST_F(JsUIServiceExtensionTest, GetSrcPath_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetSrcPath_0400 start");
    jsUIServiceExtension->abilityInfo_->moduleName = "entry";
    jsUIServiceExtension->abilityInfo_->srcEntrance = "ets/pages/Index";
    std::string srcPath;
    jsUIServiceExtension->GetSrcPath(srcPath);
    EXPECT_EQ(srcPath, "entry/ets/pages/Index.abc");
    TAG_LOGI(AAFwkTag::TEST, "GetSrcPath_0400 end");
}

/**
 * @tc.number: OnConfigurationUpdated_0100
 * @tc.name: OnConfigurationUpdated
 * @tc.desc: JsUIServiceExtension OnConfigurationUpdated
 */
HWTEST_F(JsUIServiceExtensionTest, OnConfigurationUpdated_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnConfigurationUpdated_0100 start");

    AppExecFwk::Configuration configuration;
    jsUIServiceExtension->OnConfigurationUpdated(configuration);
    EXPECT_TRUE(jsUIServiceExtension != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "OnConfigurationUpdated_0100 end");
}

/**
 * @tc.number: OnConfigurationUpdated_0200
 * @tc.name: OnConfigurationUpdated
 * @tc.desc: JsUIServiceExtension OnConfigurationUpdated
 */
HWTEST_F(JsUIServiceExtensionTest, OnConfigurationUpdated_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnConfigurationUpdated_0200 start");

    AppExecFwk::Configuration configuration;
    jsUIServiceExtension->context_ = nullptr;
    jsUIServiceExtension->OnConfigurationUpdated(configuration);
    EXPECT_TRUE(jsUIServiceExtension != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "OnConfigurationUpdated_0200 end");
}

/**
 * @tc.number: OnConfigurationUpdated_0300
 * @tc.name: OnConfigurationUpdated
 * @tc.desc: JsUIServiceExtension OnConfigurationUpdated
 */
HWTEST_F(JsUIServiceExtensionTest, OnConfigurationUpdated_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnConfigurationUpdated_0300 start");

    AppExecFwk::Configuration configuration;
    std::shared_ptr<AppExecFwk::Configuration> config{nullptr};
    jsUIServiceExtension->context_->SetConfiguration(config);
    jsUIServiceExtension->OnConfigurationUpdated(configuration);
    EXPECT_TRUE(jsUIServiceExtension != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "OnConfigurationUpdated_0300 end");
}

/**
 * @tc.number: ConfigurationUpdated_0100
 * @tc.name: ConfigurationUpdated
 * @tc.desc: JsUIServiceExtension ConfigurationUpdated
 */
HWTEST_F(JsUIServiceExtensionTest, ConfigurationUpdated_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ConfigurationUpdated_0100 start");

    jsUIServiceExtension->ConfigurationUpdated();
    EXPECT_TRUE(jsUIServiceExtension != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "ConfigurationUpdated_0100 end");
}

/**
 * @tc.number: OnCreateAndOnChangeAndOnDestroy_0100
 * @tc.name: OnCreateAndOnChangeAndOnDestroy
 * @tc.desc: JsUIServiceExtension OnCreate And OnChange And OnDestroy
 */
HWTEST_F(JsUIServiceExtensionTest, OnCreateAndOnChangeAndOnDestroy_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnCreate_0100 start");

    Rosen::DisplayId displayId{0};

    jsUIServiceExtension->OnCreate(displayId);
    jsUIServiceExtension->OnChange(displayId);
    jsUIServiceExtension->OnDestroy(displayId);
    EXPECT_TRUE(jsUIServiceExtension != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "OnCreate_0100 end");
}

/**
 * @tc.number: OnCreateAndOnChangeAndOnDestroy_0200
 * @tc.name: OnCreateAndOnChangeAndOnDestroy
 * @tc.desc: JsUIServiceExtension OnCreate And OnChange And OnDestroy
 */
HWTEST_F(JsUIServiceExtensionTest, OnCreateAndOnChangeAndOnDestroy_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnCreate_0200 start");

    Rosen::DisplayId displayId{0};

    jsUIServiceExtension->context_ = nullptr;
    jsUIServiceExtension->OnCreate(displayId);
    jsUIServiceExtension->OnChange(displayId);
    jsUIServiceExtension->OnDestroy(displayId);
    EXPECT_TRUE(jsUIServiceExtension != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "OnCreate_0200 end");
}

/**
 * @tc.number: OnCreateAndOnChangeAndOnDestroy_0300
 * @tc.name: OnCreateAndOnChangeAndOnDestroy
 * @tc.desc: JsUIServiceExtension OnCreate And OnChange And OnDestroy
 */
HWTEST_F(JsUIServiceExtensionTest, OnCreateAndOnChangeAndOnDestroy_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnCreate_0300 start");

    Rosen::DisplayId displayId{0};

    std::shared_ptr<AppExecFwk::Configuration> config{nullptr};
    jsUIServiceExtension->context_->SetConfiguration(config);
    jsUIServiceExtension->OnCreate(displayId);
    jsUIServiceExtension->OnChange(displayId);
    jsUIServiceExtension->OnDestroy(displayId);
    EXPECT_TRUE(jsUIServiceExtension != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "OnCreate_0300 end");
}

/**
 * @tc.number: OnSceneWillCreated_0100
 * @tc.name: OnSceneWillCreated
 * @tc.desc: JsUIServiceExtension OnSceneWillCreated
 */
HWTEST_F(JsUIServiceExtensionTest, OnSceneWillCreated_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnSceneWillCreated_0100 start");

    std::shared_ptr<Rosen::ExtensionWindowConfig> extensionWindowConfig
        = std::make_shared<Rosen::ExtensionWindowConfig>();
    jsUIServiceExtension->OnSceneWillCreated(extensionWindowConfig);
    EXPECT_TRUE(jsUIServiceExtension != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "OnSceneWillCreated_0100 end");
}

/**
 * @tc.number: OnSceneDidCreated_0100
 * @tc.name: OnSceneDidCreated
 * @tc.desc: JsUIServiceExtension OnSceneDidCreated
 */
HWTEST_F(JsUIServiceExtensionTest, OnSceneDidCreated_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnSceneDidCreated_0100 start");

    sptr<Rosen::Window> window = new Rosen::Window();

    jsUIServiceExtension->OnSceneDidCreated(window);
    EXPECT_TRUE(jsUIServiceExtension != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "OnSceneDidCreated_0100 end");
}

/**
 * @tc.number: OnConnect_0100
 * @tc.name: OnConnect
 * @tc.desc: JsUIServiceExtension OnConnect
 */
HWTEST_F(JsUIServiceExtensionTest, OnConnect_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnConnect_0100 start");
    sptr<ServiceHostStubImpl> stub = sptr<ServiceHostStubImpl>::MakeSptr();
    AAFwk::Want want;
    want.SetParam(UISERVICEHOSTPROXY_KEY, stub->AsObject());
    bool isAsyncCallback = false;
    auto result = jsUIServiceExtension->OnConnect(want, nullptr, isAsyncCallback);
    EXPECT_EQ(result, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "OnConnect_0100 end");
}

/**
 * @tc.number: OnConnect_0200
 * @tc.name: OnConnect
 * @tc.desc: JsUIServiceExtension OnConnect
 */
HWTEST_F(JsUIServiceExtensionTest, OnConnect_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnConnect_0200 start");
    sptr<ServiceHostStubImpl> stub = sptr<ServiceHostStubImpl>::MakeSptr();
    AAFwk::Want want;
    bool isAsyncCallback = false;
    auto result = jsUIServiceExtension->OnConnect(want, nullptr, isAsyncCallback);
    EXPECT_EQ(result, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "OnConnect_0200 end");
}

/**
 * @tc.number: OnConnect_0300
 * @tc.name: OnConnect
 * @tc.desc: JsUIServiceExtension OnConnect
 */
HWTEST_F(JsUIServiceExtensionTest, OnConnect_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnConnect_0300 start");
    sptr<ServiceHostStubImpl> stub = sptr<ServiceHostStubImpl>::MakeSptr();
    AAFwk::Want want;
    want.SetParam(UISERVICEHOSTPROXY_KEY, stub->AsObject());
    bool isAsyncCallback = false;
    auto result = jsUIServiceExtension->OnConnect(want, nullptr, isAsyncCallback);
    EXPECT_EQ(result, nullptr);
    result = jsUIServiceExtension->OnConnect(want, nullptr, isAsyncCallback);
    EXPECT_NE(result, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "OnConnect_0300 end");
}

/**
 * @tc.number: OnDisconnect_0100
 * @tc.name: OnDisconnect
 * @tc.desc: JsUIServiceExtension OnDisconnect
 */
HWTEST_F(JsUIServiceExtensionTest, OnDisconnect_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnDisconnect_0100 start");
    AAFwk::Want want;
    bool isAsyncCallback = false;

    jsUIServiceExtension->OnDisconnect(want, nullptr, isAsyncCallback);
    EXPECT_TRUE(jsUIServiceExtension != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "OnDisconnect_0100 end");
}

/**
 * @tc.number: OnDisconnect_0200
 * @tc.name: OnDisconnect
 * @tc.desc: JsUIServiceExtension OnDisconnect
 */
HWTEST_F(JsUIServiceExtensionTest, OnDisconnect_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnDisconnect_0200 start");
    sptr<ServiceHostStubImpl> stub = sptr<ServiceHostStubImpl>::MakeSptr();
    AAFwk::Want want;
    want.SetParam(UISERVICEHOSTPROXY_KEY, stub->AsObject());
    bool isAsyncCallback = false;

    jsUIServiceExtension->OnDisconnect(want, nullptr, isAsyncCallback);

    auto result = jsUIServiceExtension->OnConnect(want, nullptr, isAsyncCallback);
    EXPECT_EQ(result, nullptr);

    jsUIServiceExtension->OnDisconnect(want, nullptr, isAsyncCallback);

    TAG_LOGI(AAFwkTag::TEST, "OnDisconnect_0200 end");
}

/**
 * @tc.number: HandleSendData_0100
 * @tc.name: HandleSendData
 * @tc.desc: JsUIServiceExtension HandleSendData
 */
HWTEST_F(JsUIServiceExtensionTest, HandleSendData_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleSendData_0100 start");
    sptr<ServiceHostStubImpl> stub = sptr<ServiceHostStubImpl>::MakeSptr();
    AAFwk::Want want;
    want.SetParam(UISERVICEHOSTPROXY_KEY, stub->AsObject());
    bool isAsyncCallback = false;

    jsUIServiceExtension->OnConnect(want, nullptr, isAsyncCallback);
    EXPECT_TRUE(jsUIServiceExtension != nullptr);

    AAFwk::WantParams params;
    jsUIServiceExtension->HandleSendData(stub->AsObject(), params);

    TAG_LOGI(AAFwkTag::TEST, "HandleSendData_0100 end");
}

/**
 * @tc.number: HandleSendData_0200
 * @tc.name: HandleSendData
 * @tc.desc: JsUIServiceExtension HandleSendData
 */
HWTEST_F(JsUIServiceExtensionTest, HandleSendData_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleSendData_0200 start");

    AAFwk::WantParams params;
    jsUIServiceExtension->HandleSendData(nullptr, params);
    EXPECT_TRUE(jsUIServiceExtension != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "HandleSendData_0200 end");
}

/**
 * @tc.number: HandleSendData_0300
 * @tc.name: HandleSendData
 * @tc.desc: JsUIServiceExtension HandleSendData
 */
HWTEST_F(JsUIServiceExtensionTest, HandleSendData_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleSendData_0300 start");

    sptr<ServiceHostStubImpl> stub = sptr<ServiceHostStubImpl>::MakeSptr();

    AAFwk::WantParams params;
    jsUIServiceExtension->HandleSendData(stub->AsObject(), params);
    EXPECT_TRUE(jsUIServiceExtension != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "HandleSendData_0300 end");
}

/**
 * @tc.number: Finalizer_0100
 * @tc.name: Finalizer
 * @tc.desc: JSUIServiceExtensionContext Finalizer
 */
HWTEST_F(JsUIServiceExtensionTest, Finalizer_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Finalizer_0100 start");

    napi_env env{nullptr};

    std::shared_ptr<UIServiceExtensionContext> uiServiceExtensionContext =
        std::make_shared<UIServiceExtensionContext>();
    JSUIServiceExtensionContext jsUIServiceExtensionContext(uiServiceExtensionContext);

    jsUIServiceExtensionContext.Finalizer(env, nullptr, nullptr);
    EXPECT_TRUE(jsUIServiceExtension != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "Finalizer_0100 end");
}

/**
 * @tc.number: TerminateSelf_0100
 * @tc.name: TerminateSelf
 * @tc.desc: JSUIServiceExtensionContext TerminateSelf
 */
HWTEST_F(JsUIServiceExtensionTest, TerminateSelf_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TerminateSelf_0100 start");

    napi_env env{nullptr};
    napi_callback_info info{nullptr};

    std::shared_ptr<UIServiceExtensionContext> uiServiceExtensionContext =
        std::make_shared<UIServiceExtensionContext>();
    JSUIServiceExtensionContext jsUIServiceExtensionContext(uiServiceExtensionContext);

    jsUIServiceExtensionContext.TerminateSelf(env, info);
    EXPECT_TRUE(jsUIServiceExtension != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "TerminateSelf_0100 end");
}

/**
 * @tc.number: StartAbilityByType_0100
 * @tc.name: StartAbilityByType
 * @tc.desc: JSUIServiceExtensionContext StartAbilityByType
 */
HWTEST_F(JsUIServiceExtensionTest, StartAbilityByType_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityByType_0100 start");

    napi_env env{nullptr};
    napi_callback_info info{nullptr};

    std::shared_ptr<UIServiceExtensionContext> uiServiceExtensionContext =
        std::make_shared<UIServiceExtensionContext>();
    JSUIServiceExtensionContext jsUIServiceExtensionContext(uiServiceExtensionContext);

    jsUIServiceExtensionContext.StartAbilityByType(env, info);
    EXPECT_TRUE(jsUIServiceExtension != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "StartAbilityByType_0100 end");
}

/**
 * @tc.number: OnStartAbility_0100
 * @tc.name: OnStartAbility
 * @tc.desc: JSUIServiceExtensionContext OnStartAbility
 */
HWTEST_F(JsUIServiceExtensionTest, OnStartAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnStartAbility_0100 start");

    napi_env env{nullptr};
    NapiCallbackInfo info{0};

    std::shared_ptr<UIServiceExtensionContext> uiServiceExtensionContext =
        std::make_shared<UIServiceExtensionContext>();
    JSUIServiceExtensionContext jsUIServiceExtensionContext(uiServiceExtensionContext);

    jsUIServiceExtensionContext.OnStartAbility(env, info);
    EXPECT_TRUE(jsUIServiceExtension != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "OnStartAbility_0100 end");
}

/**
 * @tc.number: OnStartAbility_0200
 * @tc.name: OnStartAbility
 * @tc.desc: JSUIServiceExtensionContext OnStartAbility
 */
HWTEST_F(JsUIServiceExtensionTest, OnStartAbility_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnStartAbility_0200 start");

    napi_env env{nullptr};
    NapiCallbackInfo info{2};

    std::shared_ptr<UIServiceExtensionContext> uiServiceExtensionContext =
        std::make_shared<UIServiceExtensionContext>();
    JSUIServiceExtensionContext jsUIServiceExtensionContext(uiServiceExtensionContext);

    jsUIServiceExtensionContext.OnStartAbility(env, info);
    EXPECT_TRUE(jsUIServiceExtension != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "OnStartAbility_0200 end");
}

/**
 * @tc.number: CheckStartAbilityInputParam_0100
 * @tc.name: CheckStartAbilityInputParam
 * @tc.desc: JSUIServiceExtensionContext CheckStartAbilityInputParam
 */
HWTEST_F(JsUIServiceExtensionTest, CheckStartAbilityInputParam_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckStartAbilityInputParam_0100 start");

    napi_env env{nullptr};
    NapiCallbackInfo info{0};
    AAFwk::Want want;
    AAFwk::StartOptions startOptions;
    size_t unwrapArgc{0};

    std::shared_ptr<UIServiceExtensionContext> uiServiceExtensionContext =
        std::make_shared<UIServiceExtensionContext>();
    JSUIServiceExtensionContext jsUIServiceExtensionContext(uiServiceExtensionContext);

    auto ret = jsUIServiceExtensionContext.CheckStartAbilityInputParam(env, info, want, startOptions, unwrapArgc);
    EXPECT_EQ(ret, false);

    TAG_LOGI(AAFwkTag::TEST, "CheckStartAbilityInputParam_0100 end");
}

/**
 * @tc.number: CheckStartAbilityInputParam_0200
 * @tc.name: CheckStartAbilityInputParam
 * @tc.desc: JSUIServiceExtensionContext CheckStartAbilityInputParam
 */
HWTEST_F(JsUIServiceExtensionTest, CheckStartAbilityInputParam_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckStartAbilityInputParam_0200 start");

    napi_env env{nullptr};
    NapiCallbackInfo info{1};
    info.argv[0] = AppExecFwk::CreateJSObject(env);
    AAFwk::Want want;
    AAFwk::StartOptions startOptions;
    size_t unwrapArgc{0};

    std::shared_ptr<UIServiceExtensionContext> uiServiceExtensionContext =
        std::make_shared<UIServiceExtensionContext>();
    JSUIServiceExtensionContext jsUIServiceExtensionContext(uiServiceExtensionContext);

    auto ret = jsUIServiceExtensionContext.CheckStartAbilityInputParam(env, info, want, startOptions, unwrapArgc);
    EXPECT_EQ(ret, false);

    TAG_LOGI(AAFwkTag::TEST, "CheckStartAbilityInputParam_0200 end");
}

/**
 * @tc.number: OnTerminateSelf_0100
 * @tc.name: OnTerminateSelf
 * @tc.desc: JSUIServiceExtensionContext OnTerminateSelf
 */
HWTEST_F(JsUIServiceExtensionTest, OnTerminateSelf_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnTerminateSelf_0100 start");

    napi_env env{nullptr};
    NapiCallbackInfo info{0};

    std::shared_ptr<UIServiceExtensionContext> uiServiceExtensionContext =
        std::make_shared<UIServiceExtensionContext>();
    JSUIServiceExtensionContext jsUIServiceExtensionContext(uiServiceExtensionContext);

    jsUIServiceExtensionContext.OnTerminateSelf(env, info);
    EXPECT_TRUE(jsUIServiceExtension != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "OnTerminateSelf_0100 end");
}

/**
 * @tc.number: OnStartAbilityByType_0100
 * @tc.name: OnStartAbilityByType
 * @tc.desc: JSUIServiceExtensionContext OnStartAbilityByType
 */
HWTEST_F(JsUIServiceExtensionTest, OnStartAbilityByType_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnStartAbilityByType_0100 start");

    napi_env env{nullptr};
    NapiCallbackInfo info{0};

    std::shared_ptr<UIServiceExtensionContext> uiServiceExtensionContext =
        std::make_shared<UIServiceExtensionContext>();
    JSUIServiceExtensionContext jsUIServiceExtensionContext(uiServiceExtensionContext);

    jsUIServiceExtensionContext.OnStartAbilityByType(env, info);
    EXPECT_TRUE(jsUIServiceExtension != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "OnStartAbilityByType_0100 end");
}

/**
 * @tc.number: OnStartAbilityByType_0200
 * @tc.name: OnStartAbilityByType
 * @tc.desc: JSUIServiceExtensionContext OnStartAbilityByType
 */
HWTEST_F(JsUIServiceExtensionTest, OnStartAbilityByType_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnStartAbilityByType_0200 start");

    OHOS::AbilityRuntime::Runtime::Options options;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    auto err = JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    EXPECT_EQ(err, napi_status::napi_ok);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());

    NapiCallbackInfo info{3};
    string test{"TEST"};
    info.argv[0] = AppExecFwk::WrapStringToJS(env, test);
    info.argv[1] = AppExecFwk::WrapStringToJS(env, test);
    info.argv[2] = AppExecFwk::CreateJSObject(env);

    std::shared_ptr<UIServiceExtensionContext> uiServiceExtensionContext =
        std::make_shared<UIServiceExtensionContext>();
    JSUIServiceExtensionContext jsUIServiceExtensionContext(uiServiceExtensionContext);

    jsUIServiceExtensionContext.OnStartAbilityByType(env, info);

    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));

    TAG_LOGI(AAFwkTag::TEST, "OnStartAbilityByType_0200 end");
}

/**
 * @tc.number: OnStartAbilityByType_0300
 * @tc.name: OnStartAbilityByType
 * @tc.desc: JSUIServiceExtensionContext OnStartAbilityByType
 */
HWTEST_F(JsUIServiceExtensionTest, OnStartAbilityByType_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnStartAbilityByType_0300 start");

    OHOS::AbilityRuntime::Runtime::Options options;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    auto err = JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    EXPECT_EQ(err, napi_status::napi_ok);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());

    NapiCallbackInfo info{3};
    string test{"TEST"};
    info.argv[0] = AppExecFwk::WrapStringToJS(env, test);
    info.argv[1] = AppExecFwk::CreateJSObject(env);
    info.argv[2] = AppExecFwk::CreateJSObject(env);

    std::shared_ptr<UIServiceExtensionContext> uiServiceExtensionContext =
        std::make_shared<UIServiceExtensionContext>();
    JSUIServiceExtensionContext jsUIServiceExtensionContext(uiServiceExtensionContext);

    jsUIServiceExtensionContext.OnStartAbilityByType(env, info);

    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));

    TAG_LOGI(AAFwkTag::TEST, "OnStartAbilityByType_0300 end");
}

/**
 * @tc.number: CreateJsUIServiceExtensionContext_0100
 * @tc.name: CreateJsUIServiceExtensionContext
 * @tc.desc: CreateJsUIServiceExtensionContext
 */
HWTEST_F(JsUIServiceExtensionTest, CreateJsUIServiceExtensionContext_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateJsUIServiceExtensionContext_0100 start");

    napi_env env{nullptr};
    CreateJsUIServiceExtensionContext(env, nullptr);

    std::shared_ptr<UIServiceExtensionContext> uiServiceExtensionContext =
        std::make_shared<UIServiceExtensionContext>();
    CreateJsUIServiceExtensionContext(env, uiServiceExtensionContext);
    EXPECT_TRUE(uiServiceExtensionContext != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "CreateJsUIServiceExtensionContext_0100 end");
}

/**
 * @tc.number: ListenWMS_0100
 * @tc.name: ListenWMS
 * @tc.desc: ListenWMS
 */
HWTEST_F(JsUIServiceExtensionTest, ListenWMS_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ListenWMS_0100 start");

    jsUIServiceExtension->ListenWMS();

    EXPECT_NE(jsUIServiceExtension->displayListener_, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "ListenWMS_0100 end");
}

/**
 * @tc.number: AbilityWindowConfigTransition_0100
 * @tc.name: AbilityWindowConfigTransition
 * @tc.desc: AbilityWindowConfigTransition
 */
HWTEST_F(JsUIServiceExtensionTest, AbilityWindowConfigTransition_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityWindowConfigTransition_0100 start");

    sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
    uint32_t windowId = 1;

    jsUIServiceExtension->AbilityWindowConfigTransition(option, windowId);

    EXPECT_NE(jsUIServiceExtension->context_, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "AbilityWindowConfigTransition_0100 end");
}

/**
 * @tc.number: OnSendData_0100
 * @tc.name: OnSendData
 * @tc.desc: OnSendData
 */
HWTEST_F(JsUIServiceExtensionTest, OnSendData_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnSendData_0100 start");

    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    AAFwk::WantParams params;
    int32_t ret = jsUIServiceExtension->
        OnSendData(token, params);
    EXPECT_EQ(ret, 0);

    TAG_LOGI(AAFwkTag::TEST, "OnSendData_0100 end");
}
} // namespace AbilityRuntime
} // namespace OHOS
