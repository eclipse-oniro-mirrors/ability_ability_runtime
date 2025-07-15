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

#define private public
#define protected public
#include "extension_base.h"
#include "ui_extension_context.h"
#undef private
#undef protected

#include "hilog_tag_wrapper.h"
#include "want.h"
#include "mock_window.h"

using namespace testing::ext;
using namespace OHOS::Rosen;
using testing::Return;
using testing::_;

namespace OHOS {
namespace AbilityRuntime {
class UIExtensionContextTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void UIExtensionContextTest::SetUpTestCase(void)
{}

void UIExtensionContextTest::TearDownTestCase(void)
{}

void UIExtensionContextTest::SetUp()
{}

void UIExtensionContextTest::TearDown()
{}

/**
 * @tc.number: StartAbility_0100
 * @tc.name: StartAbility
 * @tc.desc: Start a new ability.
 */
HWTEST_F(UIExtensionContextTest, StartAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbility_0100 start");

    auto context = std::make_shared<UIExtensionContext>();
    AAFwk::Want want;
    ElementName element("device", "ohos.samples", "form_extension_context_test");
    want.SetElement(element);
    EXPECT_TRUE(context->StartAbility(want) != ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "StartAbility_0100 end");
}

/**
 * @tc.number: StartAbility_0200
 * @tc.name: StartAbility
 * @tc.desc: Start a new ability.
 */
HWTEST_F(UIExtensionContextTest, StartAbility_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbility_0200 start");

    auto context = std::make_shared<UIExtensionContext>();
    AAFwk::Want want;
    AAFwk::StartOptions startOptions;
    ElementName element("device", "ohos.samples", "form_extension_context_test");
    want.SetElement(element);
    EXPECT_TRUE(context->StartAbility(want, startOptions) != ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "StartAbility_0200 end");
}

/**
 * @tc.number: TerminateSelf_0100
 * @tc.name: TerminateSelf
 * @tc.desc: Terminate a ability.
 */
HWTEST_F(UIExtensionContextTest, TerminateSelf_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TerminateSelf_0100 start");

    auto context = std::make_shared<UIExtensionContext>();
    auto ret = context->TerminateSelf();
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "TerminateSelf_0100 end");
}

/**
 * @tc.number: ConnectAbility_0100
 * @tc.name: ConnectAbility
 * @tc.desc: Connect a ability.
 */
HWTEST_F(UIExtensionContextTest, ConnectAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ConnectAbility_0100 start");

    auto context = std::make_shared<UIExtensionContext>();
    AAFwk::Want want;
    sptr<AbilityConnectCallback> connectCallback;
    auto ret = context->ConnectAbility(want, connectCallback);
    EXPECT_NE(ret, 0);

    TAG_LOGI(AAFwkTag::TEST, "ConnectAbility_0100 end");
}

/**
 * @tc.number: DisconnectAbility_0100
 * @tc.name: DisconnectAbility
 * @tc.desc: Disconnect a ability.
 */
HWTEST_F(UIExtensionContextTest, DisconnectAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DisconnectAbility_0100 start");

    auto context = std::make_shared<UIExtensionContext>();
    AAFwk::Want want;
    sptr<AbilityConnectCallback> connectCallback;
    auto ret = context->DisconnectAbility(want, connectCallback);
    EXPECT_NE(ret, 0);

    TAG_LOGI(AAFwkTag::TEST, "DisconnectAbility_0100 end");
}

/**
 * @tc.number: StartAbilityForResult_0100
 * @tc.name: StartAbilityForResult
 * @tc.desc: Start a ability for result.
 */
HWTEST_F(UIExtensionContextTest, StartAbilityForResult_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityForResult_0100 start");

    auto context = std::make_shared<UIExtensionContext>();
    AAFwk::Want want;
    AAFwk::StartOptions startOptions;
    int32_t requestCode = 1;
    RuntimeTask task = [](const int32_t count, const Want& want, bool isInner)
    { GTEST_LOG_(INFO) << "StartAbilityForResult_0100 task called"; };
    auto ret = context->StartAbilityForResult(want, startOptions, requestCode, std::move(task));
    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "StartAbilityForResult_0100 end");
}

/**
 * @tc.number: StartAbilityForResult_0200
 * @tc.name: StartAbilityForResult
 * @tc.desc: Start a ability for result.
 */
HWTEST_F(UIExtensionContextTest, StartAbilityForResult_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityForResult_0200 start");

    auto context = std::make_shared<UIExtensionContext>();
    AAFwk::Want want;
    int32_t requestCode = 1;
    RuntimeTask task = [](const int32_t count, const Want& want, bool isInner)
    { GTEST_LOG_(INFO) << "StartAbilityForResult_0200 task called"; };
    auto ret = context->StartAbilityForResult(want, requestCode, std::move(task));
    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "StartAbilityForResult_0200 end");
}

/**
 * @tc.number: StartUIAbilities_0100
 * @tc.name: StartUIAbilities
 * @tc.desc: StartUIAbilities.
 */
HWTEST_F(UIExtensionContextTest, StartUIAbilities_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartUIAbilities_0100 start");

    auto context = std::make_shared<UIExtensionContext>();
    std::vector<AAFwk::Want> wantList(5);
    std::string requestKey = "12345";
    auto ret = context->StartUIAbilities(wantList, requestKey);
    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "StartUIAbilities_0100 end");
}

/**
 * @tc.number: OnAbilityResult_0100
 * @tc.name: OnAbilityResult
 * @tc.desc: On Ability Result.
 */
HWTEST_F(UIExtensionContextTest, OnAbilityResult_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnAbilityResult_0100 start");

    auto context = std::make_shared<UIExtensionContext>();
    int32_t code = 2;
    int32_t resultCode = 2;
    AAFwk::Want resultData;
    context->OnAbilityResult(code, resultCode, resultData);
    auto count = context->resultCallbacks_.size();
    EXPECT_EQ(count, 0);

    TAG_LOGI(AAFwkTag::TEST, "OnAbilityResult_0100 end");
}

/**
 * @tc.number: GenerateCurRequestCode_0100
 * @tc.name: GenerateCurRequestCode
 * @tc.desc: GenerateCurRequestCode.
 */
HWTEST_F(UIExtensionContextTest, GenerateCurRequestCode_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GenerateCurRequestCode_0100 start");

    auto context = std::make_shared<UIExtensionContext>();
    auto result = context->GenerateCurRequestCode();
    EXPECT_FALSE(result = 0);

    TAG_LOGI(AAFwkTag::TEST, "GenerateCurRequestCode_0100 end");
}

/**
 * @tc.number: GetWidow_0100
 * @tc.name: GetWidow
 * @tc.desc: GetWidow.
 */
HWTEST_F(UIExtensionContextTest, GetWidow_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetWidow_0100 start");

    auto context = std::make_shared<UIExtensionContext>();
    sptr<MockWindow> window(new (std::nothrow) MockWindow());
    context->SetWindow(window);
    EXPECT_TRUE(context->GetWindow() != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "GetWidow_0100 end");
}

/**
 * @tc.number: GetUIContent_0100
 * @tc.name: GetUIContent
 * @tc.desc: GetUIContent.
 */
HWTEST_F(UIExtensionContextTest, GetUIContent_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetUIContent_0100 start");

    auto context = std::make_shared<UIExtensionContext>();
    sptr<MockWindow> window(new (std::nothrow) MockWindow());
    context->SetWindow(window);
    Ace::UIContent* content = context->GetUIContent();
    EXPECT_TRUE(content == nullptr);

    TAG_LOGI(AAFwkTag::TEST, "GetUIContent_0100 end");
}

/**
 * @tc.number: StartAbilityForResultAsCaller_0100
 * @tc.name: StartAbilityForResultAsCaller
 * @tc.desc: StartAbilityForResultAsCaller.
 */
HWTEST_F(UIExtensionContextTest, StartAbilityForResultAsCaller_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityForResultAsCaller_0100 start");

    auto context = std::make_shared<UIExtensionContext>();
    sptr<MockWindow> window(new (std::nothrow) MockWindow());
    context->SetWindow(window);
    AAFwk::Want want;
    int requestCode = 0;
    RuntimeTask task = [](const int32_t count, const Want& want, bool isInner)
    { GTEST_LOG_(INFO) << "StartAbilityForResult_0100 task called"; };
    context->StartAbilityForResultAsCaller(want, requestCode, std::move(task));
    EXPECT_TRUE(context != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "StartAbilityForResultAsCaller_0100 end");
}

/**
 * @tc.number: StartAbilityForResultAsCaller_0200
 * @tc.name: StartAbilityForResultAsCaller
 * @tc.desc: StartAbilityForResultAsCaller.
 */
HWTEST_F(UIExtensionContextTest, StartAbilityForResultAsCaller_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityForResultAsCaller_0200 start");

    auto context = std::make_shared<UIExtensionContext>();
    sptr<MockWindow> window(new (std::nothrow) MockWindow());
    context->SetWindow(window);
    AAFwk::Want want;
    AAFwk::StartOptions startOptions;
    int requestCode = 0;
    RuntimeTask task = [](const int32_t count, const Want& want, bool isInner)
    { GTEST_LOG_(INFO) << "StartAbilityForResult_0100 task called"; };
    context->StartAbilityForResultAsCaller(want, startOptions, requestCode, std::move(task));
    EXPECT_TRUE(context != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "StartAbilityForResultAsCaller_0200 end");
}

/**
 * @tc.number: ReportDrawnCompleted_0100
 * @tc.name: ReportDrawnCompleted
 * @tc.desc: ReportDrawnCompleted.
 */
HWTEST_F(UIExtensionContextTest, ReportDrawnCompleted_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ReportDrawnCompleted_0100 start");

    auto context = std::make_shared<UIExtensionContext>();
    sptr<MockWindow> window(new (std::nothrow) MockWindow());
    context->SetWindow(window);
    context->ReportDrawnCompleted();
    EXPECT_TRUE(context != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "ReportDrawnCompleted_0100 end");
}

/**
 * @tc.number: InsertResultCallbackTask_0100
 * @tc.name: InsertResultCallbackTask
 * @tc.desc: InsertResultCallbackTask.
 */
HWTEST_F(UIExtensionContextTest, InsertResultCallbackTask_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "InsertResultCallbackTask_0100 start");

    auto context = std::make_shared<UIExtensionContext>();
    sptr<MockWindow> window(new (std::nothrow) MockWindow());
    context->SetWindow(window);
    int requestCode = 0;
    RuntimeTask task = [](const int32_t count, const Want& want, bool isInner)
    { GTEST_LOG_(INFO) << "StartAbilityForResult_0100 task called"; };
    context->InsertResultCallbackTask(requestCode, std::move(task));
    EXPECT_TRUE(context != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "InsertResultCallbackTask_0100 end");
}

/**
 * @tc.number: OpenAtomicService_0100
 * @tc.name: OpenAtomicService
 * @tc.desc: OpenAtomicService.
 */
HWTEST_F(UIExtensionContextTest, OpenAtomicService_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OpenAtomicService_0100 start");

    auto context = std::make_shared<UIExtensionContext>();
    sptr<MockWindow> window(new (std::nothrow) MockWindow());
    context->SetWindow(window);
    AAFwk::Want want;
    AAFwk::StartOptions startOptions;
    int requestCode = 0;
    RuntimeTask task = [](const int32_t count, const Want& want, bool isInner)
    { GTEST_LOG_(INFO) << "StartAbilityForResult_0100 task called"; };
    context->OpenAtomicService(want, startOptions, requestCode, std::move(task));
    EXPECT_TRUE(context != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "OpenAtomicService_0100 end");
}

/**
 * @tc.number: ConvertTo_0100
 * @tc.name: ConvertTo
 * @tc.desc: ConvertTo.
 */
HWTEST_F(UIExtensionContextTest, ConvertTo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ConvertTo_0100 start");
    std::shared_ptr<Context> context = std::make_shared<UIExtensionContext>();
    auto uiHolderExtensionContext = Context::ConvertTo<UIHolderExtensionContext>(context);
    EXPECT_NE(uiHolderExtensionContext, nullptr);
    auto uiExtensionContext = Context::ConvertTo<UIExtensionContext>(context);
    EXPECT_NE(uiExtensionContext, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "ConvertTo_0100 end");
}

/**
 * @tc.number: OpenLink_0100
 * @tc.name: OpenLink
 * @tc.desc: OpenLink.
 */
HWTEST_F(UIExtensionContextTest, OpenLink_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OpenLink_0100 start");
    AAFwk::Want want;
    int requestCode = 0;
    auto context = std::make_shared<UIExtensionContext>();
    EXPECT_NE(context, nullptr);
    context->OpenLink(want, requestCode);
    EXPECT_TRUE(context != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "OpenLink_0100 end");
}

/**
 * @tc.number: RemoveResultCallbackTask_0100
 * @tc.name: RemoveResultCallbackTask
 * @tc.desc: RemoveResultCallbackTask.
 */
HWTEST_F(UIExtensionContextTest, RemoveResultCallbackTask_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoveResultCallbackTask_0100 start");
    auto context = std::make_shared<UIExtensionContext>();
    EXPECT_NE(context, nullptr);
    sptr<MockWindow> window(new (std::nothrow) MockWindow());
    context->SetWindow(window);
    int requestCode = 0;
    RuntimeTask task = [](const int32_t count, const Want &want, bool isInner) {
        GTEST_LOG_(INFO) << "RemoveResultCallbackTask_0100 task called";
    };
    context->InsertResultCallbackTask(requestCode, std::move(task));
    context->RemoveResultCallbackTask(requestCode);
    auto count = context->resultCallbacks_.size();
    EXPECT_EQ(count, 0);
    TAG_LOGI(AAFwkTag::TEST, "RemoveResultCallbackTask_0100 end");
}

/**
 * @tc.number: AddFreeInstallObserver_0100
 * @tc.name: AddFreeInstallObserver
 * @tc.desc: AddFreeInstallObserver.
 */
HWTEST_F(UIExtensionContextTest, AddFreeInstallObserver_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AddFreeInstallObserver_0100 start");
    sptr<IFreeInstallObserver> observer;
    auto context = std::make_shared<UIExtensionContext>();
    EXPECT_NE(context, nullptr);
    context->AddFreeInstallObserver(observer);
    EXPECT_TRUE(context != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AddFreeInstallObserver_0100 end");
}

/**
 * @tc.number: StartUIServiceExtension_0100
 * @tc.name: StartUIServiceExtension
 * @tc.desc: StartUIServiceExtension.
 */
HWTEST_F(UIExtensionContextTest, StartUIServiceExtension_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartUIServiceExtension_0100 start");
    AAFwk::Want want;
    int32_t accountId = 1;
    auto context = std::make_shared<UIExtensionContext>();
    EXPECT_NE(context, nullptr);
    auto ans = context->StartUIServiceExtension(want, accountId);
    EXPECT_TRUE(ans != ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "StartUIServiceExtension_0100 end");
}

/**
 * @tc.number: SetAbilityResourceManager_0100
 * @tc.name: SetAbilityResourceManager
 * @tc.desc: SetAbilityResourceManager.
 */
HWTEST_F(UIExtensionContextTest, SetAbilityResourceManager_0100, TestSize.Level1)
{
    std::shared_ptr<Global::Resource::ResourceManager>
        abilityResourceMgr(Global::Resource::CreateResourceManager());
    auto context = std::make_shared<UIExtensionContext>();
    context->SetAbilityResourceManager(abilityResourceMgr);
    EXPECT_EQ(context->abilityResourceMgr_, abilityResourceMgr);
}

/**
 * @tc.number: RegisterAbilityConfigUpdateCallback_0100
 * @tc.name: RegisterAbilityConfigUpdateCallback
 * @tc.desc: RegisterAbilityConfigUpdateCallback.
 */
HWTEST_F(UIExtensionContextTest, RegisterAbilityConfigUpdateCallback_0100, TestSize.Level1)
{
    auto context = std::make_shared<UIExtensionContext>();
    auto abilityConfigCallback = [](const AppExecFwk::Configuration &config) {};
    context->RegisterAbilityConfigUpdateCallback(abilityConfigCallback);
    EXPECT_NE(context->abilityConfigUpdateCallback_, nullptr);
}

/**
 * @tc.number: GetAbilityConfiguration_0100
 * @tc.name: GetAbilityConfiguration
 * @tc.desc: GetAbilityConfiguration.
 */
HWTEST_F(UIExtensionContextTest, GetAbilityConfiguration_0100, TestSize.Level1)
{
    auto context = std::make_shared<UIExtensionContext>();
    context->abilityConfiguration_ = std::make_shared<AppExecFwk::Configuration>();
    auto test = context->GetAbilityConfiguration();
    EXPECT_NE(test, nullptr);
}

/**
 * @tc.number: SetAbilityConfiguration_0100
 * @tc.name: SetAbilityConfiguration
 * @tc.desc: SetAbilityConfiguration.
 */
HWTEST_F(UIExtensionContextTest, SetAbilityConfiguration_0100, TestSize.Level1)
{
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_TRUE(context->abilityConfiguration_ == nullptr);
    AppExecFwk::Configuration config;
    context->SetAbilityConfiguration(config);
    EXPECT_NE(context->abilityConfiguration_, nullptr);
}

/**
 * @tc.number: SetAbilityColorMode_0100
 * @tc.name: SetAbilityColorMode
 * @tc.desc: SetAbilityColorMode.
 */
HWTEST_F(UIExtensionContextTest, SetAbilityColorMode_0100, TestSize.Level1)
{
    auto context = std::make_shared<UIExtensionContext>();
    int32_t colorMode = -2;
    context->SetAbilityColorMode(colorMode);
    colorMode = 0;
    context->SetAbilityColorMode(colorMode);
    context->abilityConfigUpdateCallback_ = nullptr;
    auto abilityConfigCallback = [](const AppExecFwk::Configuration &config) {};
    context->abilityConfigUpdateCallback_ = abilityConfigCallback;
    context->SetAbilityColorMode(colorMode);
    EXPECT_NE(context, nullptr);
}

/**
 * @tc.number: SetScreenMode_0100
 * @tc.name: SetScreenMode
 * @tc.desc: SetScreenMode.
 */
HWTEST_F(UIExtensionContextTest, SetScreenMode_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetScreenMode_0100 start");

    auto context = std::make_shared<UIExtensionContext>();
    int32_t mode = 1;
    context->SetScreenMode(mode);
    EXPECT_EQ(context->screenMode_, mode);

    TAG_LOGI(AAFwkTag::TEST, "SetScreenMode_0100 end");
}

/**
 * @tc.number: GetScreenMode_0100
 * @tc.name: GetScreenMode
 * @tc.desc: GetScreenMode.
 */
HWTEST_F(UIExtensionContextTest, GetScreenMode_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetScreenMode_0100 start");

    auto context = std::make_shared<UIExtensionContext>();
    int32_t mode = 1;
    context->screenMode_ = mode;
    auto test = context->GetScreenMode();
    EXPECT_EQ(test, mode);

    TAG_LOGI(AAFwkTag::TEST, "GetScreenMode_0100 end");
}

/**
 * @tc.number: StartServiceExtensionAbility_0100
 * @tc.name: StartServiceExtensionAbility
 * @tc.desc: StartServiceExtensionAbility.
 */
HWTEST_F(UIExtensionContextTest, StartServiceExtensionAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartServiceExtensionAbility_0100 start");
    auto context = std::make_shared<UIExtensionContext>();
    EXPECT_NE(context, nullptr);
    const AAFwk::Want want;
    auto ret = context->StartServiceExtensionAbility(want, -1);
    EXPECT_NE(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "StartServiceExtensionAbility_0100 end");
}

/**
 * @tc.number: GetAbilityInfoType_0100
 * @tc.name: GetAbilityInfoType
 * @tc.desc: GetAbilityInfoType.
 */
HWTEST_F(UIExtensionContextTest, GetAbilityInfoType_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAbilityInfoType_0100 start");

    auto context = std::make_shared<UIExtensionContext>();
    EXPECT_NE(context, nullptr);
    auto result = context->GetAbilityInfoType();
    EXPECT_EQ(result, OHOS::AppExecFwk::AbilityType::UNKNOWN);
    context->abilityInfo_ = std::make_shared<OHOS::AppExecFwk::AbilityInfo>();
    context->abilityInfo_->type = OHOS::AppExecFwk::AbilityType::SERVICE;
    result = context->GetAbilityInfoType();
    EXPECT_EQ(result, OHOS::AppExecFwk::AbilityType::SERVICE);

    TAG_LOGI(AAFwkTag::TEST, "GetAbilityInfoType_0100 end");
}

/**
 * @tc.number: StartUIAbilities_1000
 * @tc.name: StartUIAbilities
 * @tc.desc: Start abilitys.
 */
HWTEST_F(UIExtensionContextTest, StartUIAbilities_1000, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartUIAbilities_1000 start");

    auto context = std::make_shared<UIExtensionContext>();
    std::vector<AAFwk::Want> wantList;
    std::string requestKey = "12345";
    EXPECT_TRUE(context->StartUIAbilities(wantList, requestKey) != ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "StartUIAbilities_1000 end");
}

/**
 * @tc.number: StartAbility_0300
 * @tc.name: StartAbility
 * @tc.desc: Start a new ability.
 */
HWTEST_F(UIExtensionContextTest, StartAbility_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbility_0200 start");

    auto context = std::make_shared<UIExtensionContext>();
    AAFwk::Want want;
    int requestCode = 1;
    EXPECT_TRUE(context->StartAbility(want, requestCode) != ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "StartAbility_0200 end");
}

/**
 * @tc.number: OnAbilityResult_0200
 * @tc.name: OnAbilityResult
 * @tc.desc: On Ability Result.
 */
HWTEST_F(UIExtensionContextTest, OnAbilityResult_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnAbilityResult_0200 start");

    auto context = std::make_shared<UIExtensionContext>();
    int32_t code = 2;
    int32_t resultCode = 2;
    AAFwk::Want resultData;
    bool dealed = false;
    auto runtimeTask = [&dealed](int, const AAFwk::Want &, bool) { dealed = true; };
    context->resultCallbacks_.insert(std::make_pair(code, runtimeTask));
    EXPECT_NE(context->resultCallbacks_.size(), 0);
    context->OnAbilityResult(code, resultCode, resultData);
    EXPECT_EQ(dealed, true);
    EXPECT_EQ(context->resultCallbacks_.size(), 0);

    TAG_LOGI(AAFwkTag::TEST, "OnAbilityResult_0200 start");
}

/**
 * @tc.number: SetAbilityConfiguration_0200
 * @tc.name: SetAbilityConfiguration
 * @tc.desc: SetAbilityConfiguration.
 */
HWTEST_F(UIExtensionContextTest, SetAbilityConfiguration_0200, TestSize.Level1)
{
    auto context = std::make_shared<UIExtensionContext>();
    context->abilityConfiguration_ = std::make_shared<AppExecFwk::Configuration>();
    EXPECT_NE(context->abilityConfiguration_, nullptr);
    AppExecFwk::Configuration config;
    context->SetAbilityConfiguration(config);

    std::string val{ "中文" };
    context->abilityConfiguration_->AddItem(1001, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, val);

    std::string English{ "中文" };
    config.AddItem(1002, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, English);
    context->SetAbilityConfiguration(config);
    auto result = context->abilityConfiguration_->GetItem(1002, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_EQ(result, English);
}

/**
 * @tc.number: GetResourceManager_0100
 * @tc.name: GetResourceManager
 * @tc.desc: GetResourceManager.
 */
HWTEST_F(UIExtensionContextTest, GetResourceManager_0100, TestSize.Level1)
{
    auto context = std::make_shared<UIExtensionContext>();
    std::shared_ptr<Global::Resource::ResourceManager> resourceMgr(Global::Resource::CreateResourceManager());
    context->resourceManager_ = resourceMgr;
    auto ref = context->GetResourceManager();
    EXPECT_NE(ref, nullptr);
}

/**
 * @tc.number: RequestComponentTerminate_0100
 * @tc.name: RequestComponentTerminate
 * @tc.desc: RequestComponentTerminate with all conditions required, transfer data ok.
 */
HWTEST_F(UIExtensionContextTest, RequestComponentTerminate_0100, TestSize.Level1)
{
    auto context = std::make_shared<UIExtensionContext>();
    sptr<MockWindow> window = sptr<MockWindow>::MakeSptr();
    EXPECT_CALL(*window, TransferExtensionData(_)).Times(1).WillOnce(Return(Rosen::WMError::WM_OK));
    context->SetWindow(window);
    context->RequestComponentTerminate();
}

/**
 * @tc.number: RequestComponentTerminate_0200
 * @tc.name: RequestComponentTerminate
 * @tc.desc: RequestComponentTerminate with all conditions required, transfer data not ok.
 */
HWTEST_F(UIExtensionContextTest, RequestComponentTerminate_0200, TestSize.Level1)
{
    auto context = std::make_shared<UIExtensionContext>();
    sptr<MockWindow> window = sptr<MockWindow>::MakeSptr();
    EXPECT_CALL(*window, TransferExtensionData(_)).Times(1).WillOnce(Return(Rosen::WMError::WM_DO_NOTHING));
    context->SetWindow(window);
    context->RequestComponentTerminate();
}

/**
 * @tc.number: StartAbilityByType_0100
 * @tc.name: UIExtensionContext StartAbilityByType
 * @tc.desc: UIExtensionContext StartAbilityByType.
 */
HWTEST_F(UIExtensionContextTest, StartAbilityByType_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityByType_0100 start");

    std::string type;
    AAFwk::WantParams wantParam;
    std::shared_ptr<JsUIExtensionCallback> uiExtensionCallbacks{nullptr};

    auto context = std::make_shared<UIExtensionContext>();
    auto result = context->StartAbilityByType(type, wantParam, uiExtensionCallbacks);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "StartAbilityByType_0100 end");
}
 
 /**
  * @tc.number: StartAbilityByType_0200
  * @tc.name: UIExtensionContext StartAbilityByType
  * @tc.desc: UIExtensionContext StartAbilityByType.
  */
HWTEST_F(UIExtensionContextTest, StartAbilityByType_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityByType_0200 start");

    std::string type;
    AAFwk::WantParams wantParam;
    napi_env env;
    std::shared_ptr<JsUIExtensionCallback> uiExtensionCallbacks = std::make_shared<JsUIExtensionCallback>(env);

    auto context = std::make_shared<UIExtensionContext>();
    context->SetWindow(nullptr);
    auto result = context->StartAbilityByType(type, wantParam, uiExtensionCallbacks);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "StartAbilityByType_0200 end");
}
 
 /**
  * @tc.number: StartAbilityByType_0300
  * @tc.name: UIExtensionContext StartAbilityByType
  * @tc.desc: UIExtensionContext StartAbilityByType.
  */
HWTEST_F(UIExtensionContextTest, StartAbilityByType_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityByType_0300 start");

    std::string type;
    AAFwk::WantParams wantParam;
    napi_env env;
    std::shared_ptr<JsUIExtensionCallback> uiExtensionCallbacks = std::make_shared<JsUIExtensionCallback>(env);
    sptr<Rosen::Window> window = new MockWindow();

    auto context = std::make_shared<UIExtensionContext>();
    context->SetWindow(window);
    auto result = context->StartAbilityByType(type, wantParam, uiExtensionCallbacks);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "StartAbilityByType_0300 end");
}
 
 /**
  * @tc.number: StartAbilityByType_0400
  * @tc.name: UIExtensionContext StartAbilityByType
  * @tc.desc: UIExtensionContext StartAbilityByType.
  */
HWTEST_F(UIExtensionContextTest, StartAbilityByType_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityByType_0400 start");

    std::string type;
    AAFwk::WantParams wantParam;
    const std::string FLAG_AUTH_READ_URI_PERMISSION = "ability.want.params.uriPermissionFlag";
    wantParam.SetParam(FLAG_AUTH_READ_URI_PERMISSION, 0);
    napi_env env;
    std::shared_ptr<JsUIExtensionCallback> uiExtensionCallbacks = std::make_shared<JsUIExtensionCallback>(env);
    sptr<Rosen::Window> window = new MockWindow();

    auto context = std::make_shared<UIExtensionContext>();
    context->SetWindow(window);
    auto result = context->StartAbilityByType(type, wantParam, uiExtensionCallbacks);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "StartAbilityByType_0400 end");
}

 /**
  * @tc.number: IsTerminating_0100
  * @tc.name: UIExtensionContext IsTerminating
  * @tc.desc: UIExtensionContext IsTerminating.
  */
HWTEST_F(UIExtensionContextTest, IsTerminating_0100, TestSize.Level1)
{
    auto context = std::make_shared<UIExtensionContext>();
    context->SetTerminating(true);
    EXPECT_TRUE(context->IsTerminating());
}

/**
 * @tc.number: AddCompletionHandler_0100
 * @tc.name: AddCompletionHandler
 * @tc.desc: Verify that function AddCompletionHandler.
 */
HWTEST_F(UIExtensionContextTest, AddCompletionHandler_100, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    OnRequestResult onRequestSucc = nullptr;
    OnRequestResult onRequestFail = nullptr;
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    auto result = context->AddCompletionHandler(requestId, onRequestSucc, onRequestFail);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    EXPECT_EQ(context->onRequestResults_.empty(), true);
}

/**
 * @tc.number: AddCompletionHandler_0200
 * @tc.name: AddCompletionHandler
 * @tc.desc: Verify that function AddCompletionHandler.
 */
HWTEST_F(UIExtensionContextTest, AddCompletionHandler_0200, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    OnRequestResult onRequestSucc = [](const AppExecFwk::ElementName&, const std::string&) {};
    OnRequestResult onRequestFail = nullptr;
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    auto result = context->AddCompletionHandler(requestId, onRequestSucc, onRequestFail);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    EXPECT_EQ(context->onRequestResults_.empty(), true);
}

/**
 * @tc.number: AddCompletionHandler_0300
 * @tc.name: AddCompletionHandler
 * @tc.desc: Verify that function AddCompletionHandler.
 */
HWTEST_F(UIExtensionContextTest, AddCompletionHandler_0300, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    OnRequestResult onRequestSucc = [](const AppExecFwk::ElementName&, const std::string&) {};
    OnRequestResult onRequestFail = [](const AppExecFwk::ElementName&, const std::string&) {};
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    auto result = context->AddCompletionHandler(requestId, onRequestSucc, onRequestFail);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(context->onRequestResults_.empty(), false);
    context->onRequestResults_.clear();
}

/**
 * @tc.number: AddCompletionHandler_0400
 * @tc.name: AddCompletionHandler
 * @tc.desc: Verify that function AddCompletionHandler.
 */
HWTEST_F(UIExtensionContextTest, AddCompletionHandler_0400, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    OnRequestResult onRequestSucc = [](const AppExecFwk::ElementName&, const std::string&) {};
    OnRequestResult onRequestFail = [](const AppExecFwk::ElementName&, const std::string&) {};
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    context->onRequestResults_.emplace_back(requestId, onRequestSucc, onRequestFail);
    auto result = context->AddCompletionHandler(requestId, onRequestSucc, onRequestFail);
    EXPECT_EQ(result, ERR_OK);
    std::string norequestId = "test";
    result = context->AddCompletionHandler(norequestId, onRequestSucc, onRequestFail);
    EXPECT_EQ(context->onRequestResults_.size(), 2);
    context->onRequestResults_.clear();
}

/**
 * @tc.number: OnRequestSuccess_0100
 * @tc.name: OnRequestSuccess
 * @tc.desc: Verify that function OnRequestSuccess.
 */
HWTEST_F(UIExtensionContextTest, OnRequestSuccess_0100, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    OnRequestResult onRequestSucc = [](const AppExecFwk::ElementName&, const std::string&) {};
    OnRequestResult onRequestFail = [](const AppExecFwk::ElementName&, const std::string&) {};
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    auto result = context->AddCompletionHandler(requestId, onRequestSucc, onRequestFail);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(context->onRequestResults_.empty(), false);
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    context->OnRequestSuccess(requestId, element, "success");
    EXPECT_EQ(context->onRequestResults_.empty(), true);
}

/**
 * @tc.number: OnRequestSuccess_0200
 * @tc.name: OnRequestSuccess
 * @tc.desc: Verify that function OnRequestSuccess.
 */
HWTEST_F(UIExtensionContextTest, OnRequestSuccess_0200, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    EXPECT_EQ(context->onRequestResults_.empty(), true);
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    context->OnRequestSuccess(requestId, element, "success");
    EXPECT_EQ(context->onRequestResults_.empty(), true);
}

/**
 * @tc.number: OnRequestSuccess_0300
 * @tc.name: OnRequestSuccess
 * @tc.desc: Verify that function OnRequestSuccess.
 */
HWTEST_F(UIExtensionContextTest, OnRequestSuccess_0300, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    OnRequestResult onRequestSucc = [](const AppExecFwk::ElementName&, const std::string&) {};
    OnRequestResult onRequestFail = [](const AppExecFwk::ElementName&, const std::string&) {};
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    auto result = context->AddCompletionHandler(requestId, onRequestSucc, onRequestFail);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(context->onRequestResults_.empty(), false);
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    std::string norequestId = "test";
    context->OnRequestSuccess(norequestId, element, "success");
    EXPECT_EQ(context->onRequestResults_.empty(), false);
    context->onRequestResults_.clear();
}

/**
 * @tc.number: OnRequestFailure_0100
 * @tc.name: OnRequestFailure
 * @tc.desc: Verify that function OnRequestFailure.
 */
HWTEST_F(UIExtensionContextTest, OnRequestFailure_0100, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    OnRequestResult onRequestSucc = [](const AppExecFwk::ElementName&, const std::string&) {};
    OnRequestResult onRequestFail = [](const AppExecFwk::ElementName&, const std::string&) {};
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    auto result = context->AddCompletionHandler(requestId, onRequestSucc, onRequestFail);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(context->onRequestResults_.empty(), false);
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    context->OnRequestFailure(requestId, element, "failure");
    EXPECT_EQ(context->onRequestResults_.empty(), true);
    context->onRequestResults_.clear();
}

/**
 * @tc.number: OnRequestFailure_0200
 * @tc.name: OnRequestFailure
 * @tc.desc: Verify that function OnRequestFailure.
 */
HWTEST_F(UIExtensionContextTest, OnRequestFailure_0200, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    EXPECT_EQ(context->onRequestResults_.empty(), true);
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    context->OnRequestFailure(requestId, element, "failure");
    EXPECT_EQ(context->onRequestResults_.empty(), true);
}

/**
 * @tc.number: OnRequestFailure_0300
 * @tc.name: OnRequestFailure
 * @tc.desc: Verify that function OnRequestFailure.
 */
HWTEST_F(UIExtensionContextTest, OnRequestFailure_0300, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    OnRequestResult onRequestSucc = [](const AppExecFwk::ElementName&, const std::string&) {};
    OnRequestResult onRequestFail = [](const AppExecFwk::ElementName&, const std::string&) {};
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    auto result = context->AddCompletionHandler(requestId, onRequestSucc, onRequestFail);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(context->onRequestResults_.empty(), false);
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    std::string norequestId = "test";
    context->OnRequestFailure(norequestId, element, "failure");
    EXPECT_EQ(context->onRequestResults_.empty(), false);
    context->onRequestResults_.clear();
}
} // namespace AbilityRuntime
} // namespace OHOS
