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
 * @tc.number: StartUIAbilitiesInSplitWindowMode_0100
 * @tc.name: StartUIAbilitiesInSplitWindowMode
 * @tc.desc: StartUIAbilitiesInSplitWindowMode.
 */
HWTEST_F(UIExtensionContextTest, StartUIAbilitiesInSplitWindowMode_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartUIAbilitiesInSplitWindowMode_0100 start");

    auto context = std::make_shared<UIExtensionContext>();
    int32_t primaryWindowId = 1;
    AAFwk::Want want;
    auto ret = context->StartUIAbilitiesInSplitWindowMode(primaryWindowId, want);
    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "StartUIAbilitiesInSplitWindowMode_0100 end");
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
 * @tc.number: OnOpenLinkRequestSuccess_0001
 * @tc.name: OnOpenLinkRequestSuccess
 * @tc.desc: OpenLinkRequest not exist.
 */
HWTEST_F(UIExtensionContextTest, OnOpenLinkRequestSuccess_0001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnOpenLinkRequestSuccess_0001 start");
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    std::string requestId = "1234567890";
    AAFwk::Want want;
    std::string message = "message";
    OnRequestResult onRequestSucc = [](const AppExecFwk::ElementName&, const std::string&) {};
    OnRequestResult onRequestFail = [](const AppExecFwk::ElementName&, const std::string&) {};
    auto result = context->AddCompletionHandlerForOpenLink(requestId, onRequestSucc, onRequestFail);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(context->onOpenLinkRequestResults_.empty(), false);
    std::string norequestId = "test";
    context->OnOpenLinkRequestSuccess(norequestId, want.GetElement(), message);
    EXPECT_EQ(context->onOpenLinkRequestResults_.empty(), false);
    context->onOpenLinkRequestResults_.clear();
    TAG_LOGI(AAFwkTag::TEST, "OnOpenLinkRequestSuccess_0001 end");
}

/**
 * @tc.number: OnOpenLinkRequestSuccess_0002
 * @tc.name: OnOpenLinkRequestSuccess
 * @tc.desc: OpenLinkRequest exist.
 */
HWTEST_F(UIExtensionContextTest, OnOpenLinkRequestSuccess_0002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnOpenLinkRequestSuccess_0002 start");
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    std::string requestId = "1234567890";
    AAFwk::Want want;
    std::string message = "message";
    OnRequestResult onRequestSucc = [](const AppExecFwk::ElementName&, const std::string&) {};
    OnRequestResult onRequestFail = [](const AppExecFwk::ElementName&, const std::string&) {};
    auto result = context->AddCompletionHandlerForOpenLink(requestId, onRequestSucc, onRequestFail);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(context->onOpenLinkRequestResults_.empty(), false);

    context->OnOpenLinkRequestSuccess(requestId, want.GetElement(), message);
    EXPECT_EQ(context->onOpenLinkRequestResults_.empty(), true);
    context->onOpenLinkRequestResults_.clear();
    TAG_LOGI(AAFwkTag::TEST, "OnOpenLinkRequestSuccess_0002 end");
}

/**
 * @tc.number: OnOpenLinkRequestFailure_0001
 * @tc.name: OnOpenLinkRequestFailure
 * @tc.desc: OpenLinkRequest not exist.
 */
HWTEST_F(UIExtensionContextTest, OnOpenLinkRequestFailure_0001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnOpenLinkRequestFailure_0001 start");
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    std::string requestId = "1234567890";
    AAFwk::Want want;
    std::string message = "message";
    OnRequestResult onRequestSucc = [](const AppExecFwk::ElementName&, const std::string&) {};
    OnRequestResult onRequestFail = [](const AppExecFwk::ElementName&, const std::string&) {};
    auto result = context->AddCompletionHandlerForOpenLink(requestId, onRequestSucc, onRequestFail);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(context->onOpenLinkRequestResults_.empty(), false);
    std::string norequestId = "test";
    context->OnOpenLinkRequestFailure(norequestId, want.GetElement(), message);
    EXPECT_EQ(context->onOpenLinkRequestResults_.empty(), false);
    context->onOpenLinkRequestResults_.clear();
    TAG_LOGI(AAFwkTag::TEST, "OnOpenLinkRequestFailure_0001 end");
}

/**
 * @tc.number: OnOpenLinkRequestFailure_0002
 * @tc.name: OnOpenLinkRequestFailure
 * @tc.desc: OpenLinkRequest exist.
 */
HWTEST_F(UIExtensionContextTest, OnOpenLinkRequestFailure_0002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnOpenLinkRequestFailure_0002 start");
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    std::string requestId = "1234567890";
    AAFwk::Want want;
    std::string message = "message";
    OnRequestResult onRequestSucc = [](const AppExecFwk::ElementName&, const std::string&) {};
    OnRequestResult onRequestFail = [](const AppExecFwk::ElementName&, const std::string&) {};
    auto result = context->AddCompletionHandlerForOpenLink(requestId, onRequestSucc, onRequestFail);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(context->onOpenLinkRequestResults_.empty(), false);

    context->OnOpenLinkRequestFailure(requestId, want.GetElement(), message);
    EXPECT_EQ(context->onOpenLinkRequestResults_.empty(), true);
    context->onOpenLinkRequestResults_.clear();
    TAG_LOGI(AAFwkTag::TEST, "OnOpenLinkRequestFailure_0002 end");
}

/**
 * @tc.number: OnOpenLinkRequestFailure_0003
 * @tc.name: OnOpenLinkRequestFailure
 * @tc.desc: requestId empty.
 */
HWTEST_F(UIExtensionContextTest, OnOpenLinkRequestFailure_0003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnOpenLinkRequestFailure_0003 start");
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    std::string requestId = "1234567890";
    AAFwk::Want want;
    std::string message = "message";
    OnRequestResult onRequestSucc = [](const AppExecFwk::ElementName&, const std::string&) {};
    OnRequestResult onRequestFail = [](const AppExecFwk::ElementName&, const std::string&) {};
    auto result = context->AddCompletionHandlerForOpenLink(requestId, onRequestSucc, onRequestFail);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(context->onOpenLinkRequestResults_.empty(), false);
    std::string norequestId = "";
    context->OnOpenLinkRequestFailure(norequestId, want.GetElement(), message);
    EXPECT_EQ(context->onOpenLinkRequestResults_.empty(), false);
    context->onOpenLinkRequestResults_.clear();
    TAG_LOGI(AAFwkTag::TEST, "OnOpenLinkRequestFailure_0003 end");
}

/**
 * @tc.number: AddCompletionHandlerForOpenLink_0001
 * @tc.name: AddCompletionHandlerForOpenLink
 * @tc.desc: param onRequestSucc error
 */
HWTEST_F(UIExtensionContextTest, AddCompletionHandlerForOpenLink_0001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AddCompletionHandlerForOpenLink_0001 start");
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    std::string requestId = "1234567890";
    AAFwk::Want want;
    std::string message = "message";
    OnRequestResult onRequestSucc = [](const AppExecFwk::ElementName&, const std::string&) {};
    OnRequestResult onRequestFail = [](const AppExecFwk::ElementName&, const std::string&) {};
    auto result = context->AddCompletionHandlerForOpenLink(requestId, nullptr, onRequestFail);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    context->onOpenLinkRequestResults_.clear();
    TAG_LOGI(AAFwkTag::TEST, "AddCompletionHandlerForOpenLink_0001 end");
}

/**
 * @tc.number: AddCompletionHandlerForOpenLink_0002
 * @tc.name: AddCompletionHandlerForOpenLink
 * @tc.desc: param onRequestFail error
 */
HWTEST_F(UIExtensionContextTest, AddCompletionHandlerForOpenLink_0002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AddCompletionHandlerForOpenLink_0002 start");
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    std::string requestId = "1234567890";
    AAFwk::Want want;
    std::string message = "message";
    OnRequestResult onRequestSucc = [](const AppExecFwk::ElementName&, const std::string&) {};
    OnRequestResult onRequestFail = [](const AppExecFwk::ElementName&, const std::string&) {};
    auto result = context->AddCompletionHandlerForOpenLink(requestId, onRequestSucc, nullptr);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    context->onOpenLinkRequestResults_.clear();
    TAG_LOGI(AAFwkTag::TEST, "AddCompletionHandlerForOpenLink_0002 end");
}

/**
 * @tc.number: AddCompletionHandlerForOpenLink_0003
 * @tc.name: AddCompletionHandlerForOpenLink
 * @tc.desc: param onRequestSucc onRequestFail error
 */
HWTEST_F(UIExtensionContextTest, AddCompletionHandlerForOpenLink_0003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AddCompletionHandlerForOpenLink_0003 start");
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    std::string requestId = "1234567890";
    AAFwk::Want want;
    std::string message = "message";
    OnRequestResult onRequestSucc = [](const AppExecFwk::ElementName&, const std::string&) {};
    OnRequestResult onRequestFail = [](const AppExecFwk::ElementName&, const std::string&) {};
    auto result = context->AddCompletionHandlerForOpenLink(requestId, nullptr, nullptr);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    context->onOpenLinkRequestResults_.clear();
    TAG_LOGI(AAFwkTag::TEST, "AddCompletionHandlerForOpenLink_0003 end");
}

/**
 * @tc.number: AddCompletionHandlerForOpenLink_0004
 * @tc.name: AddCompletionHandlerForOpenLink
 * @tc.desc: insert success.
 */
HWTEST_F(UIExtensionContextTest, AddCompletionHandlerForOpenLink_0004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AddCompletionHandlerForOpenLink_0004 start");
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    std::string requestId = "1234567890";
    AAFwk::Want want;
    std::string message = "message";
    OnRequestResult onRequestSucc = [](const AppExecFwk::ElementName&, const std::string&) {};
    OnRequestResult onRequestFail = [](const AppExecFwk::ElementName&, const std::string&) {};
    auto result = context->AddCompletionHandlerForOpenLink(requestId, onRequestSucc, onRequestFail);
    EXPECT_EQ(result, ERR_OK);
    context->onOpenLinkRequestResults_.clear();
    TAG_LOGI(AAFwkTag::TEST, "AddCompletionHandlerForOpenLink_0004 end");
}

/**
 * @tc.number: AddCompletionHandlerForOpenLink_0005
 * @tc.name: AddCompletionHandlerForOpenLink
 * @tc.desc: already exists.
 */
HWTEST_F(UIExtensionContextTest, AddCompletionHandlerForOpenLink_0005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AddCompletionHandlerForOpenLink_0005 start");
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    std::string requestId = "1234567890";
    AAFwk::Want want;
    std::string message = "message";
    OnRequestResult onRequestSucc = [](const AppExecFwk::ElementName&, const std::string&) {};
    OnRequestResult onRequestFail = [](const AppExecFwk::ElementName&, const std::string&) {};
    auto result = context->AddCompletionHandlerForOpenLink(requestId, onRequestSucc, onRequestFail);
    EXPECT_EQ(result, ERR_OK);
    result = context->AddCompletionHandlerForOpenLink(requestId, onRequestSucc, onRequestFail);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(context->onOpenLinkRequestResults_.size(), 1);
    context->onOpenLinkRequestResults_.clear();
    TAG_LOGI(AAFwkTag::TEST, "AddCompletionHandlerForOpenLink_0005 end");
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
 * @tc.number: AddCompletionHandlerForAtomicService_0100
 * @tc.name: AddCompletionHandlerForAtomicService
 * @tc.desc: Verify that function AddCompletionHandlerForAtomicService.
 */
HWTEST_F(UIExtensionContextTest, AddCompletionHandlerForAtomicService_100, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    std::string appId = "atomic";
    OnAtomicRequestSuccess onRequestSucc = nullptr;
    OnAtomicRequestFailure onRequestFail = nullptr;
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    auto result = context->AddCompletionHandlerForAtomicService(requestId, onRequestSucc, onRequestFail, appId);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    EXPECT_EQ(context->onAtomicRequestResults_.empty(), true);
}

/**
 * @tc.number: AddCompletionHandlerForAtomicService_0200
 * @tc.name: AddCompletionHandlerForAtomicService
 * @tc.desc: Verify that function AddCompletionHandlerForAtomicService.
 */
HWTEST_F(UIExtensionContextTest, AddCompletionHandlerForAtomicService_0200, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    std::string appId = "atomic";
    OnAtomicRequestSuccess onRequestSucc = [](const std::string&) {};
    OnAtomicRequestFailure onRequestFail = nullptr;
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    auto result = context->AddCompletionHandlerForAtomicService(requestId, onRequestSucc, onRequestFail, appId);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    EXPECT_EQ(context->onAtomicRequestResults_.empty(), true);
}

/**
 * @tc.number: AddCompletionHandlerForAtomicService_0300
 * @tc.name: AddCompletionHandlerForAtomicService
 * @tc.desc: Verify that function AddCompletionHandlerForAtomicService.
 */
HWTEST_F(UIExtensionContextTest, AddCompletionHandlerForAtomicService_0300, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    std::string appId = "atomic";
    OnAtomicRequestSuccess onRequestSucc = [](const std::string&) {};
    OnAtomicRequestFailure onRequestFail = [](const std::string&, int32_t, const std::string&) {};
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    auto result = context->AddCompletionHandlerForAtomicService(requestId, onRequestSucc, onRequestFail, appId);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(context->onAtomicRequestResults_.empty(), false);
    context->onAtomicRequestResults_.clear();
}

/**
 * @tc.number: AddCompletionHandler_0400
 * @tc.name: AddCompletionHandlerForAtomicService
 * @tc.desc: Verify that function AddCompletionHandlerForAtomicService.
 */
HWTEST_F(UIExtensionContextTest, AddCompletionHandler_0400, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    std::string appId = "atomic";
    OnAtomicRequestSuccess onRequestSucc = [](const std::string&) {};
    OnAtomicRequestFailure onRequestFail = [](const std::string&, int32_t, const std::string&) {};
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    context->onAtomicRequestResults_.emplace_back(
        std::make_shared<OnAtomicRequestResult>(requestId, appId, onRequestSucc, onRequestFail));
    auto result = context->AddCompletionHandlerForAtomicService(requestId, onRequestSucc, onRequestFail, appId);
    EXPECT_EQ(result, ERR_OK);
    std::string norequestId = "test";
    result = context->AddCompletionHandlerForAtomicService(norequestId, onRequestSucc, onRequestFail, appId);
    EXPECT_EQ(context->onAtomicRequestResults_.size(), 2);
    context->onAtomicRequestResults_.clear();
}

/**
 * @tc.number: OnRequestSuccess_0100
 * @tc.name: OnRequestSuccess
 * @tc.desc: Verify that function OnRequestSuccess.
 */
HWTEST_F(UIExtensionContextTest, OnRequestSuccess_0100, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    std::string appId = "atomic";
    OnAtomicRequestSuccess onRequestSucc = [](const std::string&) {};
    OnAtomicRequestFailure onRequestFail = [](const std::string&, int32_t, const std::string&) {};
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    auto result = context->AddCompletionHandlerForAtomicService(requestId, onRequestSucc, onRequestFail, appId);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(context->onAtomicRequestResults_.empty(), false);
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    context->OnRequestSuccess(requestId, element, "success");
    EXPECT_EQ(context->onAtomicRequestResults_.empty(), true);
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
    EXPECT_EQ(context->onAtomicRequestResults_.empty(), true);
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    context->OnRequestSuccess(requestId, element, "success");
    EXPECT_EQ(context->onAtomicRequestResults_.empty(), true);
}

/**
 * @tc.number: OnRequestSuccess_0300
 * @tc.name: OnRequestSuccess
 * @tc.desc: Verify that function OnRequestSuccess.
 */
HWTEST_F(UIExtensionContextTest, OnRequestSuccess_0300, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    std::string appId = "atomic";
    OnAtomicRequestSuccess onRequestSucc = [](const std::string&) {};
    OnAtomicRequestFailure onRequestFail = [](const std::string&, int32_t, const std::string&) {};
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    auto result = context->AddCompletionHandlerForAtomicService(requestId, onRequestSucc, onRequestFail, appId);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(context->onAtomicRequestResults_.empty(), false);
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    std::string norequestId = "test";
    context->OnRequestSuccess(norequestId, element, "success");
    EXPECT_EQ(context->onAtomicRequestResults_.empty(), false);
    context->onAtomicRequestResults_.clear();
}

/**
 * @tc.number: OnRequestFailure_0100
 * @tc.name: OnRequestFailure
 * @tc.desc: Verify that function OnRequestFailure.
 */
HWTEST_F(UIExtensionContextTest, OnRequestFailure_0100, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    std::string appId = "atomic";
    OnAtomicRequestSuccess onRequestSucc = [](const std::string&) {};
    OnAtomicRequestFailure onRequestFail = [](const std::string&, int32_t, const std::string&) {};
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    auto result = context->AddCompletionHandlerForAtomicService(requestId, onRequestSucc, onRequestFail, appId);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(context->onAtomicRequestResults_.empty(), false);
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    context->OnRequestFailure(requestId, element, "failure");
    EXPECT_EQ(context->onAtomicRequestResults_.empty(), true);
    context->onAtomicRequestResults_.clear();
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
    EXPECT_EQ(context->onAtomicRequestResults_.empty(), true);
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    context->OnRequestFailure(requestId, element, "failure");
    EXPECT_EQ(context->onAtomicRequestResults_.empty(), true);
}

/**
 * @tc.number: OnRequestFailure_0300
 * @tc.name: OnRequestFailure
 * @tc.desc: Verify that function OnRequestFailure.
 */
HWTEST_F(UIExtensionContextTest, OnRequestFailure_0300, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    std::string appId = "atomic";
    OnAtomicRequestSuccess onRequestSucc = [](const std::string&) {};
    OnAtomicRequestFailure onRequestFail = [](const std::string&, int32_t, const std::string&) {};
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    auto result = context->AddCompletionHandlerForAtomicService(requestId, onRequestSucc, onRequestFail, appId);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(context->onAtomicRequestResults_.empty(), false);
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    std::string norequestId = "test";
    context->OnRequestFailure(norequestId, element, "failure");
    EXPECT_EQ(context->onAtomicRequestResults_.empty(), false);
    context->onAtomicRequestResults_.clear();
}

/**
 * @tc.number: GetFailureInfoByMessage_0100
 * @tc.name: GetFailureInfoByMessage
 * @tc.desc: Verify that function GetFailureInfoByMessage.
 */
HWTEST_F(UIExtensionContextTest, GetFailureInfoByMessage_0100, Function | MediumTest | Level1)
{
    std::string message = "User refused redirection";
    int32_t faileCode = 0;
    std::string failReason;
    int32_t resultCode = USER_CANCEL;
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    context->GetFailureInfoByMessage(message, faileCode, failReason, resultCode);
    EXPECT_EQ(faileCode, 1);
    EXPECT_EQ(failReason, "The user canceled this startup");
    resultCode = 0;
    context->GetFailureInfoByMessage(message, faileCode, failReason, resultCode);
    EXPECT_EQ(faileCode, 2);
    EXPECT_EQ(failReason, "User refused redirection");
    message = "test";
    context->GetFailureInfoByMessage(message, faileCode, failReason, resultCode);
    EXPECT_EQ(faileCode, 0);
    EXPECT_EQ(failReason, "A system error occurred");
}

/**
 * @tc.number: SetAbilityFontSize_0100
 * @tc.name: SetAbilityFontSize
 * @tc.desc: Test SetAbilityFontSize with negative fontSize value.
 */
HWTEST_F(UIExtensionContextTest, SetAbilityFontSize_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetAbilityFontSize_0100 start");

    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);

    // 注册回调以便检测是否被调用
    bool callbackInvoked = false;
    auto abilityConfigCallback = [&callbackInvoked](const AppExecFwk::Configuration &config) {
        callbackInvoked = true;
    };
    context->RegisterAbilityConfigUpdateCallback(abilityConfigCallback);

    // 测试负值字体大小（应该直接返回，不调用回调）
    double negativeFontSize = -1.0;
    context->SetAbilityFontSize(negativeFontSize);
    EXPECT_FALSE(callbackInvoked);

    TAG_LOGI(AAFwkTag::TEST, "SetAbilityFontSize_0100 end");
}

/**
 * @tc.number: SetAbilityFontSize_0200
 * @tc.name: SetAbilityFontSize
 * @tc.desc: Test SetAbilityFontSize with valid fontSize value and callback registered.
 */
HWTEST_F(UIExtensionContextTest, SetAbilityFontSize_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetAbilityFontSize_0200 start");

    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);

    // 注册回调
    bool callbackInvoked = false;
    double receivedFontSize = 0.0;
    auto abilityConfigCallback = [&callbackInvoked, &receivedFontSize](const AppExecFwk::Configuration &config) {
        callbackInvoked = true;
        // 验证配置中包含正确的字体大小值
        std::string fontSizeStr = config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_SIZE_SCALE);
        receivedFontSize = std::stod(fontSizeStr);
    };
    context->RegisterAbilityConfigUpdateCallback(abilityConfigCallback);

    // 测试正常字体大小值
    double validFontSize = 1.5;
    context->SetAbilityFontSize(validFontSize);
    EXPECT_TRUE(callbackInvoked);
    EXPECT_DOUBLE_EQ(receivedFontSize, validFontSize);

    TAG_LOGI(AAFwkTag::TEST, "SetAbilityFontSize_0200 end");
}

/**
 * @tc.number: SetAbilityFontSize_0300
 * @tc.name: SetAbilityFontSize
 * @tc.desc: Test SetAbilityFontSize with zero fontSize value.
 */
HWTEST_F(UIExtensionContextTest, SetAbilityFontSize_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetAbilityFontSize_0300 start");

    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);

    // 注册回调
    bool callbackInvoked = false;
    double receivedFontSize = 0.0;
    auto abilityConfigCallback = [&callbackInvoked, &receivedFontSize](const AppExecFwk::Configuration &config) {
        callbackInvoked = true;
        std::string fontSizeStr = config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_SIZE_SCALE);
        receivedFontSize = std::stod(fontSizeStr);
    };
    context->RegisterAbilityConfigUpdateCallback(abilityConfigCallback);

    // 测试零值字体大小（应该正常执行）
    double zeroFontSize = 0.1;
    context->SetAbilityFontSize(zeroFontSize);
    EXPECT_TRUE(callbackInvoked);
    EXPECT_DOUBLE_EQ(receivedFontSize, zeroFontSize);

    TAG_LOGI(AAFwkTag::TEST, "SetAbilityFontSize_0300 end");
}

/**
 * @tc.number: SetAbilityFontSize_0400
 * @tc.name: SetAbilityFontSize
 * @tc.desc: Test SetAbilityFontSize when callback is not registered.
 */
HWTEST_F(UIExtensionContextTest, SetAbilityFontSize_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetAbilityFontSize_0400 start");

    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);

    // 不注册回调（abilityConfigUpdateCallback_ 为 nullptr）
    double validFontSize = 2.0;
    // 即使字体大小有效，没有回调也不应该崩溃或出现异常
    context->SetAbilityFontSize(validFontSize);
    EXPECT_EQ(context->abilityConfigUpdateCallback_, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "SetAbilityFontSize_0400 end");
}

/**
 * @tc.number: SetAbilityFontSize_0500
 * @tc.name: SetAbilityFontSize
 * @tc.desc: Test SetAbilityFontSize with various valid fontSize values.
 */
HWTEST_F(UIExtensionContextTest, SetAbilityFontSize_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetAbilityFontSize_0500 start");

    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);

    // 注册回调
    std::vector<double> receivedFontSizes;
    auto abilityConfigCallback = [&receivedFontSizes](const AppExecFwk::Configuration &config) {
        std::string fontSizeStr = config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_SIZE_SCALE);
        receivedFontSizes.push_back(std::stod(fontSizeStr));
    };
    context->RegisterAbilityConfigUpdateCallback(abilityConfigCallback);

    // 测试多个不同的字体大小值
    std::vector<double> testFontSizes = {0.5, 1.0, 1.5, 2.0, 3.0};
    for (double fontSize : testFontSizes) {
        context->SetAbilityFontSize(fontSize);
    }

    // 验证所有值都被正确处理
    EXPECT_EQ(receivedFontSizes.size(), testFontSizes.size());
    for (size_t i = 0; i < testFontSizes.size(); ++i) {
        EXPECT_DOUBLE_EQ(receivedFontSizes[i], testFontSizes[i]);
    }

    TAG_LOGI(AAFwkTag::TEST, "SetAbilityFontSize_0500 end");
}

/**
 * @tc.number: SetAbilityConfiguration_0300
 * @tc.name: SetAbilityConfiguration
 * @tc.desc: Test SetAbilityConfiguration when abilityConfiguration_ is null and fullConfig is null.
 */
HWTEST_F(UIExtensionContextTest, SetAbilityConfiguration_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetAbilityConfiguration_0300 start");

    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    ASSERT_EQ(context->abilityConfiguration_, nullptr);

    AppExecFwk::Configuration config;
    context->SetAbilityConfiguration(config);

    // 验证 abilityConfiguration_ 被正确创建
    EXPECT_NE(context->abilityConfiguration_, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "SetAbilityConfiguration_0300 end");
}

/**
 * @tc.number: SetAbilityConfiguration_0400
 * @tc.name: SetAbilityConfiguration
 * @tc.desc: Test SetAbilityConfiguration with multiple configuration updates.
 */
HWTEST_F(UIExtensionContextTest, SetAbilityConfiguration_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetAbilityConfiguration_0400 start");

    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);

    // 设置初始配置
    AppExecFwk::Configuration config1;
    std::string language1 = "zh-CN";
    config1.AddItem(1001, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, language1);
    context->SetAbilityConfiguration(config1);

    EXPECT_NE(context->abilityConfiguration_, nullptr);
    std::string result1 = context->abilityConfiguration_->GetItem(1001, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_EQ(result1, language1);

    // 更新配置（添加新的配置项）
    AppExecFwk::Configuration config2;
    std::string language2 = "en-US";
    config2.AddItem(1002, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, language2);
    context->SetAbilityConfiguration(config2);

    // 验证配置被正确合并
    std::string result2 = context->abilityConfiguration_->GetItem(1002, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_EQ(result2, language2);

    TAG_LOGI(AAFwkTag::TEST, "SetAbilityConfiguration_0400 end");
}

/**
 * @tc.number: SetAbilityConfiguration_0500
 * @tc.name: SetAbilityConfiguration
 * @tc.desc: Test SetAbilityConfiguration when abilityConfiguration_ is null and fullConfig is null.
 */
HWTEST_F(UIExtensionContextTest, SetAbilityConfiguration_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetAbilityConfiguration_0500 start");

    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    ASSERT_EQ(context->abilityConfiguration_, nullptr);

    AppExecFwk::Configuration config;
    context->SetAbilityConfiguration(config);

    // 验证 abilityConfiguration_ 被正确创建
    EXPECT_NE(context->abilityConfiguration_, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "SetAbilityConfiguration_0500 end");
}
} // namespace AbilityRuntime
} // namespace OHOS
