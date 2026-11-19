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

#include "ability_handler.h"
#include "ability_local_record.h"
#include "configuration_convertor.h"
#define private public
#define protected public
#include "configuration_utils.h"
#undef private
#undef protected
#include "hilog_tag_wrapper.h"
#include "iremote_object.h"
#include "js_runtime.h"
#define private public
#define protected public
#include "js_service_extension.h"
#undef private
#undef protected
#include "js_service_extension_context.h"
#include "mock_ability_token.h"
#include "ohos_application.h"
#include "runtime.h"
#include "string_wrapper.h"
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
constexpr char JS_SERVICE_EXTENSION_TASK_RUNNER[] = "JsServiceExtension";
constexpr char DEFAULT_LANGUAGE[] = "zh_CN";
} // namespace

class JsServiceExtensionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    static std::shared_ptr<JsServiceExtension> jsServiceExtension_;
    static std::shared_ptr<ApplicationContext> applicationContext_;
    static std::unique_ptr<Runtime> jsRuntime_;

private:
    static void CreateJsServiceExtension();
};

std::shared_ptr<JsServiceExtension> JsServiceExtensionTest::jsServiceExtension_ = nullptr;
std::shared_ptr<ApplicationContext> JsServiceExtensionTest::applicationContext_ = nullptr;
std::unique_ptr<Runtime> JsServiceExtensionTest::jsRuntime_ = nullptr;

void JsServiceExtensionTest::SetUpTestCase()
{
    CreateJsServiceExtension();
}

void JsServiceExtensionTest::TearDownTestCase()
{}

void JsServiceExtensionTest::SetUp()
{}

void JsServiceExtensionTest::TearDown()
{}

void JsServiceExtensionTest::CreateJsServiceExtension()
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

    auto eventRunner = AppExecFwk::EventRunner::Create(JS_SERVICE_EXTENSION_TASK_RUNNER);
    Runtime::Options options;
    options.preload = true;
    options.eventRunner = eventRunner;
    jsRuntime_ = JsRuntime::Create(options);
    ASSERT_NE(jsRuntime_, nullptr);

    JsServiceExtension *extension = JsServiceExtension::Create(jsRuntime_);
    ASSERT_NE(extension, nullptr);
    jsServiceExtension_.reset(extension);

    jsServiceExtension_->Init(record, application, handler, token);
}

/**
 * @tc.name: Configuration_0100
 * @tc.desc: Js service extension init.
 * @tc.type: FUNC
 * @tc.require: issueI7HPHB
 */
HWTEST_F(JsServiceExtensionTest, Init_0100, TestSize.Level1)
{
    ASSERT_NE(jsServiceExtension_, nullptr);
    ASSERT_NE(applicationContext_, nullptr);
    auto context = jsServiceExtension_->GetContext();
    ASSERT_NE(context, nullptr);

    auto configuration = context->GetConfiguration();
    ASSERT_NE(configuration, nullptr);

    auto appConfig = applicationContext_->GetConfiguration();
    ASSERT_NE(appConfig, nullptr);

    // normally configuration is different, size is equal; cause LoadModule can't succeed, configuration is same.
    EXPECT_EQ(configuration.get(), appConfig.get());
    EXPECT_EQ(configuration->GetItemSize(), appConfig->GetItemSize());
    auto language = configuration->GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_EQ(language, DEFAULT_LANGUAGE);
    auto colorMode = configuration->GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
    EXPECT_EQ(colorMode, ConfigurationInner::COLOR_MODE_LIGHT);
}

/**
 * @tc.name: OnStart_0100
 * @tc.desc: Js service extension OnStart.
 * @tc.type: FUNC
 * @tc.require: issueI7HPHB
 */
HWTEST_F(JsServiceExtensionTest, OnStart_0100, TestSize.Level1)
{
    int displayId = Rosen::WindowScene::DEFAULT_DISPLAY_ID;
    float originDensity;
    std::string originDirection;
    auto configUtils = std::make_shared<AbilityRuntime::ConfigurationUtils>();
    auto ret = configUtils->GetDisplayConfig(displayId, originDensity, originDirection);
    EXPECT_EQ(ret, true);

    Want want;
    ASSERT_NE(jsServiceExtension_, nullptr);
    jsServiceExtension_->OnStart(want);

    auto context = jsServiceExtension_->GetContext();
    ASSERT_NE(context, nullptr);

    auto configuration = context->GetConfiguration();
    ASSERT_NE(configuration, nullptr);
    auto resourceManager = context->GetResourceManager();
    ASSERT_NE(resourceManager, nullptr);

    auto appConfig = applicationContext_->GetConfiguration();
    ASSERT_NE(appConfig, nullptr);

    // configuration is larger than original
    EXPECT_LE(configuration->GetItemSize(), appConfig->GetItemSize());

    // check configuration
    std::string displayIdStr = configuration->GetItem(ConfigurationInner::APPLICATION_DISPLAYID);
    EXPECT_EQ(displayIdStr, std::to_string(displayId));
    std::string densityStr = configuration->GetItem(displayId, ConfigurationInner::APPLICATION_DENSITYDPI);
    EXPECT_EQ(densityStr, GetDensityStr(originDensity));
    std::string directionStr = configuration->GetItem(displayId, ConfigurationInner::APPLICATION_DIRECTION);
    EXPECT_EQ(directionStr, originDirection);

    // check resource manager
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    resourceManager->GetResConfig(*resConfig);
    EXPECT_EQ(originDensity, resConfig->GetScreenDensity());
    EXPECT_EQ(ConvertDirection(originDirection), resConfig->GetDirection());
}

/**
 * @tc.name: OnConfigurationUpdated_0100
 * @tc.desc: Js service extension OnConfigurationUpdated.
 * @tc.type: FUNC
 * @tc.require: issueI7HPHB
 */
HWTEST_F(JsServiceExtensionTest, OnConfigurationUpdated_0100, TestSize.Level1)
{
    Configuration newConfig;
    newConfig.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, "en_US");
    newConfig.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, ConfigurationInner::COLOR_MODE_DARK);
    ASSERT_NE(jsServiceExtension_, nullptr);
    jsServiceExtension_->OnConfigurationUpdated(newConfig);

    auto context = jsServiceExtension_->GetContext();
    ASSERT_NE(context, nullptr);
    auto configuration = context->GetConfiguration();
    ASSERT_NE(configuration, nullptr);

    // check configuration
    auto language = configuration->GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_EQ(language, "en_US");
    auto colorMode = configuration->GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
    EXPECT_EQ(colorMode, ConfigurationInner::COLOR_MODE_DARK);
}

/**
 * @tc.name: OnCreate_0100
 * @tc.desc: Js service extension OnCreate.
 * @tc.type: FUNC
 * @tc.require: issueI7HPHB
 */
HWTEST_F(JsServiceExtensionTest, OnCreate_0100, TestSize.Level1)
{
    ASSERT_NE(jsServiceExtension_, nullptr);
    Rosen::DisplayId displayId = 1;
    jsServiceExtension_->OnCreate(displayId);
}

/**
 * @tc.name: OnDestroy_0100
 * @tc.desc: Js service extension OnDestroy.
 * @tc.type: FUNC
 * @tc.require: issueI7HPHB
 */
HWTEST_F(JsServiceExtensionTest, OnDestroy_0100, TestSize.Level1)
{
    ASSERT_NE(jsServiceExtension_, nullptr);
    Rosen::DisplayId displayId = 1;
    jsServiceExtension_->OnDestroy(displayId);
}

/**
 * @tc.name: OnChange_0100
 * @tc.desc: Js service extension OnChange.
 * @tc.type: FUNC
 * @tc.require: issueI7HPHB
 */
HWTEST_F(JsServiceExtensionTest, OnChange_0100, TestSize.Level1)
{
    int displayId = Rosen::WindowScene::DEFAULT_DISPLAY_ID;
    float originDensity;
    std::string originDirection;
    auto configUtils = std::make_shared<AbilityRuntime::ConfigurationUtils>();
    auto ret = configUtils->GetDisplayConfig(displayId, originDensity, originDirection);
    EXPECT_EQ(ret, true);

    ASSERT_NE(jsServiceExtension_, nullptr);
    auto context = jsServiceExtension_->GetContext();
    ASSERT_NE(context, nullptr);
    auto configuration = context->GetConfiguration();
    ASSERT_NE(configuration, nullptr);
    auto resourceManager = context->GetResourceManager();
    ASSERT_NE(resourceManager, nullptr);

    configuration->RemoveItem(displayId, ConfigurationInner::APPLICATION_DENSITYDPI);

    ASSERT_NE(jsServiceExtension_, nullptr);
    jsServiceExtension_->OnChange(displayId);

    // check configuration
    std::string displayIdStr = configuration->GetItem(ConfigurationInner::APPLICATION_DISPLAYID);
    EXPECT_EQ(displayIdStr, std::to_string(displayId));
    std::string densityStr = configuration->GetItem(displayId, ConfigurationInner::APPLICATION_DENSITYDPI);
    EXPECT_EQ(densityStr, GetDensityStr(originDensity));
    std::string directionStr = configuration->GetItem(displayId, ConfigurationInner::APPLICATION_DIRECTION);
    EXPECT_EQ(directionStr, originDirection);

    // check resource manager
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    resourceManager->GetResConfig(*resConfig);
    EXPECT_EQ(originDensity, resConfig->GetScreenDensity());
    EXPECT_EQ(ConvertDirection(originDirection), resConfig->GetDirection());
}

/**
 * @tc.name: HandleInsightIntent_0100
 * @tc.desc: Js service extension HandleInsightIntent.
 * @tc.type: FUNC
 * @tc.require: issueI7HPHB
 */
HWTEST_F(JsServiceExtensionTest, HandleInsightIntent_0100, TestSize.Level1)
{
    ASSERT_NE(jsServiceExtension_, nullptr);
    Want want;
    WantParams wantParams;
    std::string paramName(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME);
    std::string insightIntentId(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_ID);
    wantParams.SetParam(paramName, String::Box("playMusic"));
    wantParams.SetParam(insightIntentId, String::Box("1"));

    want.SetParams(wantParams);
    AppExecFwk::ElementName element;
    element.SetBundleName("com.ohos.test");
    want.SetElement(element);

    auto ret = jsServiceExtension_->HandleInsightIntent(want);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: GetSrcPath_0100
 * @tc.desc: Js service extension GetSrcPath with normal srcEntrance that has file extension.
 * @tc.type: FUNC
 */
HWTEST_F(JsServiceExtensionTest, GetSrcPath_0100, TestSize.Level1)
{
    ASSERT_NE(jsServiceExtension_, nullptr);
    jsServiceExtension_->abilityInfo_->isModuleJson = true;
    jsServiceExtension_->abilityInfo_->moduleName = "entry";
    jsServiceExtension_->abilityInfo_->srcEntrance = "ets/pages/Index.ts";
    std::string srcPath;
    jsServiceExtension_->GetSrcPath(srcPath);
    EXPECT_EQ(srcPath, "entry/ets/pages/Index.abc");
}

/**
 * @tc.name: GetSrcPath_0200
 * @tc.desc: Js service extension GetSrcPath where folder name contains dot but filename has no extension.
 * @tc.type: FUNC
 */
HWTEST_F(JsServiceExtensionTest, GetSrcPath_0200, TestSize.Level1)
{
    ASSERT_NE(jsServiceExtension_, nullptr);
    jsServiceExtension_->abilityInfo_->isModuleJson = true;
    jsServiceExtension_->abilityInfo_->moduleName = "entry";
    jsServiceExtension_->abilityInfo_->srcEntrance = "v1.0/pages/Index";
    std::string srcPath;
    jsServiceExtension_->GetSrcPath(srcPath);
    EXPECT_EQ(srcPath, "entry/v1.0/pages/Index.abc");
}

/**
 * @tc.name: GetSrcPath_0300
 * @tc.desc: Js service extension GetSrcPath where path has no dot, should not erase and not go out of bounds.
 * @tc.type: FUNC
 */
HWTEST_F(JsServiceExtensionTest, GetSrcPath_0300, TestSize.Level1)
{
    ASSERT_NE(jsServiceExtension_, nullptr);
    jsServiceExtension_->abilityInfo_->isModuleJson = true;
    jsServiceExtension_->abilityInfo_->moduleName = "entry";
    jsServiceExtension_->abilityInfo_->srcEntrance = "ets/pages/Index";
    std::string srcPath;
    jsServiceExtension_->GetSrcPath(srcPath);
    EXPECT_EQ(srcPath, "entry/ets/pages/Index.abc");
}
} // namespace AbilityRuntime
} // namespace OHOS
