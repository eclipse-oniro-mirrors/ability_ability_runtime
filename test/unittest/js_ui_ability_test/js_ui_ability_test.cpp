/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#include "hilog_wrapper.h"
#define private public
#define protected public
#include "ability_context_impl.h"
#include "js_ui_ability.h"
#include "mock_scene_board_judgement.h"
#undef private
#undef protected
#include "js_runtime_utils.h"
#include "ability_stage_context.h"
#include "napi_common_want.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AAFwk;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AbilityRuntime {
class NativeReferenceMock : public NativeReference {
public:
    NativeReferenceMock() = default;
    ~NativeReferenceMock() override = default;

    MOCK_METHOD(uint32_t, Ref, (), (override));
    MOCK_METHOD(uint32_t, Unref, (), (override));
    MOCK_METHOD(napi_value, Get, (), (override));
    MOCK_METHOD(void*, GetData, (), (override));
    MOCK_METHOD(void, SetDeleteSelf, (), (override));
    MOCK_METHOD(uint32_t, GetRefCount, (), (override));
    MOCK_METHOD(bool, GetFinalRun, (), (override));
    explicit operator napi_value() override
    {
        return reinterpret_cast<napi_value>(this);
    }
    MOCK_METHOD(napi_value, GetNapiValue, (), (override));
};

class JsUiAbilityTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void JsUiAbilityTest::SetUpTestCase() {}

void JsUiAbilityTest::TearDownTestCase() {}

void JsUiAbilityTest::SetUp() {}

void JsUiAbilityTest::TearDown() {}

/**
 * @tc.name: JSUIAbility_OnWillForeground_0100
 * @tc.desc: OnWillForeground test
 * @tc.desc: Verify function OnWillForeground.
 */
HWTEST_F(JsUiAbilityTest, JSUIAbility_OnWillForeground_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JSUIAbility_OnWillForeground_0100 start";
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto ability = AbilityRuntime::JsUIAbility::Create(runtime);
    ASSERT_NE(ability, nullptr);
    ability->OnWillForeground();
    GTEST_LOG_(INFO) << "JSUIAbility_OnWillForeground_0100 end";
}
/**
 * @tc.name: JSUIAbility_OnDidForeground_0100
 * @tc.desc: OnDidForeground test
 * @tc.desc: Verify function OnDidForeground.
 */
HWTEST_F(JsUiAbilityTest, JSUIAbility_OnDidForeground_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JSUIAbility_OnDidForeground_0100 start";
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto ability = AbilityRuntime::JsUIAbility::Create(runtime);
    ASSERT_NE(ability, nullptr);
    ability->OnDidForeground();
    GTEST_LOG_(INFO) << "JSUIAbility_OnDidForeground_0100 end";
}

/**
 * @tc.name: JSUIAbility_OnWillBackground_0100
 * @tc.desc: OnWillBackground test
 * @tc.desc: Verify function OnWillBackground.
 */
HWTEST_F(JsUiAbilityTest, JSUIAbility_OnWillBackground_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JSUIAbility_OnWillBackground_0100 start";
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto ability = AbilityRuntime::JsUIAbility::Create(runtime);
    ASSERT_NE(ability, nullptr);
    ability->OnWillBackground();
    ability->scene_ = std::make_shared<Rosen::WindowScene>();
    ability->OnWillBackground();
    GTEST_LOG_(INFO) << "JSUIAbility_OnWillBackground_0100 end";
}

/**
 * @tc.name: JSUIAbility_OnDidBackground_0100
 * @tc.desc: OnDidBackground test
 * @tc.desc: Verify function OnDidBackground.
 */
HWTEST_F(JsUiAbilityTest, JSUIAbility_OnDidBackground_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JSUIAbility_OnDidBackground_0100 start";
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto ability = AbilityRuntime::JsUIAbility::Create(runtime);
    ASSERT_NE(ability, nullptr);
    ability->OnDidBackground();
    GTEST_LOG_(INFO) << "JSUIAbility_OnDidBackground_0100 end";
}

/**
 * @tc.name: JSUIAbility_OnAbilityRequestFailure_0100
 * @tc.desc: OnAbilityRequestFailure test
 * @tc.desc: Verify function OnAbilityRequestFailure.
 */
HWTEST_F(JsUiAbilityTest, JSUIAbility_OnAbilityRequestFailure_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JSUIAbility_OnAbilityRequestFailure_0100 start";
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto ability = AbilityRuntime::JsUIAbility::Create(runtime);
    ASSERT_NE(ability, nullptr);
    std::string requestId = "1234567890";
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    std::string message = "failure";
    ability->OnAbilityRequestFailure(requestId, element, message);
    GTEST_LOG_(INFO) << "JSUIAbility_OnAbilityRequestFailure_0100 end";
}

/**
 * @tc.name: JSUIAbility_OnAbilityRequestSuccess_0100
 * @tc.desc: OnAbilityRequestSuccess test
 * @tc.desc: Verify function OnAbilityRequestSuccess.
 */
HWTEST_F(JsUiAbilityTest, JSUIAbility_OnAbilityRequestSuccess_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JSUIAbility_OnAbilityRequestSuccess_0100 start";
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto ability = AbilityRuntime::JsUIAbility::Create(runtime);
    ASSERT_NE(ability, nullptr);
    std::string requestId = "1234567890";
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    std::string message = "success";
    ability->OnAbilityRequestSuccess(requestId, element, message);
    GTEST_LOG_(INFO) << "JSUIAbility_OnAbilityRequestSuccess_0100 end";
}

/**
 * @tc.name: JSUIAbility_DoOnForegroundForSceneIsNull_0100
 * @tc.desc: DoOnForegroundForSceneIsNull test
 * @tc.desc: Verify function DoOnForegroundForSceneIsNull.
 */
HWTEST_F(JsUiAbilityTest, JSUIAbility_DoOnForegroundForSceneIsNull_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JSUIAbility_DoOnForegroundForSceneIsNull_0100 start";
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto jsRuntime = static_cast<AbilityRuntime::JsRuntime*>(runtime.get());
    auto ability = std::make_shared<AbilityRuntime::JsUIAbility>(*jsRuntime);

    wptr<IRemoteObject> token(new IPCObjectStub());
    ability->sessionToken_ = token;
    auto abilityContextImpl = std::make_shared<AbilityContextImpl>();

    auto stageContext = std::make_shared<AbilityRuntime::AbilityStageContext>();
    AppExecFwk::HapModuleInfo hapModuleInfo;
    stageContext->InitHapModuleInfo(hapModuleInfo);
    abilityContextImpl->SetStageContext(stageContext);
    abilityContextImpl->SetAbilityInfo(nullptr);
    
    ability->abilityContext_ = abilityContextImpl;
    Rosen::SceneBoardJudgement::flag_ = true;
    Want want;
    ability->DoOnForegroundForSceneIsNull(want);
    EXPECT_NE(abilityContextImpl->GetSessionToken(), nullptr);

    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityInfo->applicationInfo.bundleType = OHOS::AppExecFwk::BundleType::APP;
    abilityContextImpl->SetAbilityInfo(abilityInfo);
    ability->abilityContext_ = abilityContextImpl;
    ability->DoOnForegroundForSceneIsNull(want);
    EXPECT_NE(abilityContextImpl->GetSessionToken(), nullptr);

    std::string navDestinationInfo = "testNavDestinationInfo";
    want.SetParam(Want::ATOMIC_SERVICE_SHARE_ROUTER, navDestinationInfo);
    ability->DoOnForegroundForSceneIsNull(want);
    EXPECT_NE(abilityContextImpl->GetSessionToken(), nullptr);

    abilityInfo->applicationInfo.bundleType = OHOS::AppExecFwk::BundleType::ATOMIC_SERVICE;
    abilityContextImpl->SetAbilityInfo(abilityInfo);
    ability->abilityContext_ = abilityContextImpl;
    navDestinationInfo = "testNavDestinationInfo";
    want.SetParam(Want::ATOMIC_SERVICE_SHARE_ROUTER, navDestinationInfo);
    ability->DoOnForegroundForSceneIsNull(want);
    EXPECT_NE(abilityContextImpl->GetSessionToken(), nullptr);

    navDestinationInfo = "";
    want.SetParam(Want::ATOMIC_SERVICE_SHARE_ROUTER, navDestinationInfo);
    ability->sceneListener_ = new Rosen::IWindowLifeCycle();
    ability->DoOnForegroundForSceneIsNull(want);
    EXPECT_NE(abilityContextImpl->GetSessionToken(), nullptr);
    GTEST_LOG_(INFO) << "JSUIAbility_DoOnForegroundForSceneIsNull_0100 end";
}

/**
 * @tc.name: JSUIAbility_OnNewWant_0100
 * @tc.desc: OnNewWant test
 * @tc.desc: Verify function OnNewWant.
 */
HWTEST_F(JsUiAbilityTest, JSUIAbility_OnNewWant_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JSUIAbility_OnNewWant_0100 start";
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto jsRuntime = static_cast<AbilityRuntime::JsRuntime*>(runtime.get());
    auto ability = std::make_shared<AbilityRuntime::JsUIAbility>(*jsRuntime);
    Want want;

    ability->scene_ = std::make_shared<Rosen::WindowScene>();
    auto abilityContextImpl = std::make_shared<AbilityContextImpl>();
    abilityContextImpl->SetAbilityInfo(nullptr);
    ability->abilityContext_ = abilityContextImpl;
    ability->OnNewWant(want);
    
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityInfo->applicationInfo.bundleType = OHOS::AppExecFwk::BundleType::ATOMIC_SERVICE;
    abilityContextImpl->SetAbilityInfo(abilityInfo);
    ability->abilityContext_ = abilityContextImpl;
    ability->OnNewWant(want);

    abilityInfo->applicationInfo.bundleType = OHOS::AppExecFwk::BundleType::APP;
    abilityContextImpl->SetAbilityInfo(abilityInfo);
    ability->abilityContext_ = abilityContextImpl;

    std::string navDestinationInfo = "testNavDestinationInfo";
    want.SetParam(Want::ATOMIC_SERVICE_SHARE_ROUTER, navDestinationInfo);
    ability->OnNewWant(want);
    EXPECT_EQ(want.GetStringParam(Want::ATOMIC_SERVICE_SHARE_ROUTER), navDestinationInfo);
    GTEST_LOG_(INFO) << "JSUIAbility_OnNewWant_0100 end";
}

/**
 * @tc.name: JSUIAbility_OnStart_0100
 * @tc.desc: OnStart test
 * @tc.desc: abilityInfo_ == nullptr
 */
HWTEST_F(JsUiAbilityTest, JSUIAbility_OnStart_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JSUIAbility_OnStart_0100 start";
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto jsRuntime = static_cast<AbilityRuntime::JsRuntime*>(runtime.get());
    auto ability = std::make_shared<AbilityRuntime::JsUIAbility>(*jsRuntime);
    EXPECT_NE(ability, nullptr);
    Want want;
    napi_ref ref = nullptr;
    auto env = jsRuntime->GetNapiEnv();
    napi_value value = OHOS::AppExecFwk::WrapWant(env, want);
    napi_create_reference(env, value, 1, &ref);
    ability->jsAbilityObj_ = std::unique_ptr<NativeReference>(
        reinterpret_cast<NativeReference *>(ref));
    EXPECT_NE(ability->jsAbilityObj_, nullptr);

    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    EXPECT_NE(sessionInfo, nullptr);
    ability->scene_ = std::make_shared<Rosen::WindowScene>();
    EXPECT_NE(ability->scene_, nullptr);
    auto abilityContextImpl = std::make_shared<AbilityContextImpl>();
    EXPECT_NE(abilityContextImpl, nullptr);
    abilityContextImpl->SetAbilityInfo(nullptr);
    ability->abilityContext_ = abilityContextImpl;
    ability->abilityInfo_ = nullptr;
    ability->OnStart(want, sessionInfo);
    EXPECT_NE(ability->jsAbilityObj_, nullptr);
    
    GTEST_LOG_(INFO) << "JSUIAbility_OnStart_0100 end";
}

/**
 * @tc.name: JSUIAbility_OnStart_0200
 * @tc.desc: OnStart test
 * @tc.desc: jsAbilityObj_ == nullptr
 */
HWTEST_F(JsUiAbilityTest, JSUIAbility_OnStart_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JSUIAbility_OnStart_0200 start";
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto jsRuntime = static_cast<AbilityRuntime::JsRuntime*>(runtime.get());
    auto ability = std::make_shared<AbilityRuntime::JsUIAbility>(*jsRuntime);
    EXPECT_NE(ability, nullptr);
    ability->jsAbilityObj_ = nullptr;
    Want want;

    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    EXPECT_NE(sessionInfo, nullptr);
    ability->scene_ = std::make_shared<Rosen::WindowScene>();
    EXPECT_NE(ability->scene_, nullptr);
    auto abilityContextImpl = std::make_shared<AbilityContextImpl>();
    EXPECT_NE(abilityContextImpl, nullptr);
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    EXPECT_NE(abilityInfo, nullptr);
    abilityContextImpl->SetAbilityInfo(abilityInfo);
    ability->abilityContext_ = abilityContextImpl;
    ability->abilityInfo_ = abilityInfo;
    ability->jsAbilityObj_ = nullptr;
    ability->OnStart(want, sessionInfo);
    EXPECT_NE(ability->abilityInfo_, nullptr);

    GTEST_LOG_(INFO) << "JSUIAbility_OnStart_0200 end";
}

/**
 * @tc.name: JSUIAbility_OnStart_0300
 * @tc.desc: OnStart test
 * @tc.desc: sessionInfo nullptr; launchMode false
 */
HWTEST_F(JsUiAbilityTest, JSUIAbility_OnStart_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JSUIAbility_OnStart_0300 start";
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto jsRuntime = static_cast<AbilityRuntime::JsRuntime*>(runtime.get());
    auto ability = std::make_shared<AbilityRuntime::JsUIAbility>(*jsRuntime);
    EXPECT_NE(ability, nullptr);
    Want want;
    napi_ref ref = nullptr;
    auto env = jsRuntime->GetNapiEnv();
    napi_value value = OHOS::AppExecFwk::WrapWant(env, want);
    napi_create_reference(env, value, 1, &ref);
    ability->jsAbilityObj_ = std::unique_ptr<NativeReference>(
        reinterpret_cast<NativeReference *>(ref));
    EXPECT_NE(ability->jsAbilityObj_, nullptr);

    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    ability->scene_ = std::make_shared<Rosen::WindowScene>();
    EXPECT_NE(ability->scene_, nullptr);
    auto abilityContextImpl = std::make_shared<AbilityContextImpl>();
    EXPECT_NE(abilityContextImpl, nullptr);
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    EXPECT_NE(abilityInfo, nullptr);
    abilityInfo->launchMode = AppExecFwk::LaunchMode::SINGLETON;
    abilityContextImpl->SetAbilityInfo(abilityInfo);
    ability->abilityContext_ = abilityContextImpl;
    ability->abilityInfo_ = abilityInfo;
    EXPECT_NE(ability->abilityInfo_, nullptr);
    ability->OnStart(want, sessionInfo);
    EXPECT_NE(ability->abilityInfo_->launchMode, AppExecFwk::LaunchMode::SPECIFIED);

    GTEST_LOG_(INFO) << "JSUIAbility_OnStart_0300 end";
}

/**
 * @tc.name: JSUIAbility_OnStart_0400
 * @tc.desc: OnStart test
 * @tc.desc: sessionInfo true; launchMode false
 */
HWTEST_F(JsUiAbilityTest, JSUIAbility_OnStart_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JSUIAbility_OnStart_0400 start";
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto jsRuntime = static_cast<AbilityRuntime::JsRuntime*>(runtime.get());
    auto ability = std::make_shared<AbilityRuntime::JsUIAbility>(*jsRuntime);
    EXPECT_NE(ability, nullptr);
    Want want;
    napi_ref ref = nullptr;
    auto env = jsRuntime->GetNapiEnv();
    napi_value value = OHOS::AppExecFwk::WrapWant(env, want);
    napi_create_reference(env, value, 1, &ref);
    ability->jsAbilityObj_ = std::unique_ptr<NativeReference>(
        reinterpret_cast<NativeReference *>(ref));
    EXPECT_NE(ability->jsAbilityObj_, nullptr);

    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    EXPECT_NE(sessionInfo, nullptr);
    ability->scene_ = std::make_shared<Rosen::WindowScene>();
    EXPECT_NE(ability->scene_, nullptr);
    auto abilityContextImpl = std::make_shared<AbilityContextImpl>();
    EXPECT_NE(abilityContextImpl, nullptr);
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    EXPECT_NE(abilityInfo, nullptr);
    abilityInfo->launchMode = AppExecFwk::LaunchMode::SINGLETON;
    abilityContextImpl->SetAbilityInfo(abilityInfo);
    ability->abilityContext_ = abilityContextImpl;
    ability->abilityInfo_ = abilityInfo;
    EXPECT_NE(ability->abilityInfo_, nullptr);
    ability->OnStart(want, sessionInfo);
    EXPECT_NE(ability->abilityInfo_->launchMode, AppExecFwk::LaunchMode::SPECIFIED);

    GTEST_LOG_(INFO) << "JSUIAbility_OnStart_0400 end";
}

/**
 * @tc.name: JSUIAbility_OnStart_0500
 * @tc.desc: OnStart test
 * @tc.desc: sessionInfo nullptr; launchMode true
 */
HWTEST_F(JsUiAbilityTest, JSUIAbility_OnStart_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JSUIAbility_OnStart_0500 start";
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto jsRuntime = static_cast<AbilityRuntime::JsRuntime*>(runtime.get());
    auto ability = std::make_shared<AbilityRuntime::JsUIAbility>(*jsRuntime);
    EXPECT_NE(ability, nullptr);
    Want want;
    napi_ref ref = nullptr;
    auto env = jsRuntime->GetNapiEnv();
    napi_value value = OHOS::AppExecFwk::WrapWant(env, want);
    napi_create_reference(env, value, 1, &ref);
    ability->jsAbilityObj_ = std::unique_ptr<NativeReference>(
        reinterpret_cast<NativeReference *>(ref));
    EXPECT_NE(ability->jsAbilityObj_, nullptr);

    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    ability->scene_ = std::make_shared<Rosen::WindowScene>();
    EXPECT_NE(ability->scene_, nullptr);
    auto abilityContextImpl = std::make_shared<AbilityContextImpl>();
    EXPECT_NE(abilityContextImpl, nullptr);
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    EXPECT_NE(abilityInfo, nullptr);
    abilityInfo->launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    abilityContextImpl->SetAbilityInfo(abilityInfo);
    ability->abilityContext_ = abilityContextImpl;
    ability->abilityInfo_ = abilityInfo;
    EXPECT_NE(ability->abilityInfo_, nullptr);
    ability->OnStart(want, sessionInfo);
    EXPECT_EQ(ability->abilityInfo_->launchMode, AppExecFwk::LaunchMode::SPECIFIED);

    GTEST_LOG_(INFO) << "JSUIAbility_OnStart_0500 end";
}

/**
 * @tc.name: JSUIAbility_OnStart_0600
 * @tc.desc: OnStart test
 * @tc.desc: sessionInfo true; launchMode true
 */
HWTEST_F(JsUiAbilityTest, JSUIAbility_OnStart_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JSUIAbility_OnStart_0600 start";
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto jsRuntime = static_cast<AbilityRuntime::JsRuntime*>(runtime.get());
    auto ability = std::make_shared<AbilityRuntime::JsUIAbility>(*jsRuntime);
    EXPECT_NE(ability, nullptr);
    Want want;
    napi_ref ref = nullptr;
    auto env = jsRuntime->GetNapiEnv();
    napi_value value = OHOS::AppExecFwk::WrapWant(env, want);
    napi_create_reference(env, value, 1, &ref);
    ability->jsAbilityObj_ = std::unique_ptr<NativeReference>(
        reinterpret_cast<NativeReference *>(ref));
    EXPECT_NE(ability->jsAbilityObj_, nullptr);

    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    EXPECT_NE(sessionInfo, nullptr);
    ability->scene_ = std::make_shared<Rosen::WindowScene>();
    EXPECT_NE(ability->scene_, nullptr);
    auto abilityContextImpl = std::make_shared<AbilityContextImpl>();
    EXPECT_NE(abilityContextImpl, nullptr);
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    EXPECT_NE(abilityInfo, nullptr);
    abilityInfo->launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    abilityContextImpl->SetAbilityInfo(abilityInfo);
    ability->abilityContext_ = abilityContextImpl;
    ability->abilityInfo_ = abilityInfo;
    EXPECT_NE(ability->abilityInfo_, nullptr);
    ability->OnStart(want, sessionInfo);
    EXPECT_EQ(ability->abilityInfo_->launchMode, AppExecFwk::LaunchMode::SPECIFIED);

    GTEST_LOG_(INFO) << "JSUIAbility_OnStart_0600 end";
}

/**
 * @tc.name: JSUIAbility_GetWindowStage_0100
 * @tc.desc: GetWindowStage test
 * @tc.desc: Verify function GetWindowStage.
 */
HWTEST_F(JsUiAbilityTest, JSUIAbility_GetWindowStage_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JSUIAbility_GetWindowStage_0100 start";
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto ability = AbilityRuntime::JsUIAbility::Create(runtime);
    ASSERT_NE(ability, nullptr);
    auto windowStage  = ability->GetWindowStage();
    ASSERT_EQ(windowStage, nullptr);
    GTEST_LOG_(INFO) << "JSUIAbility_GetWindowStage_0100 end";
}

/**
 * @tc.name: JSUIAbility_GetWindowStage_0200
 * @tc.desc: GetWindowStage test
 * @tc.desc: Verify function GetWindowStage.
 */
HWTEST_F(JsUiAbilityTest, JSUIAbility_GetWindowStage_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JSUIAbility_GetWindowStage_0200 start";
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto jsRuntime = static_cast<AbilityRuntime::JsRuntime*>(runtime.get());
    ASSERT_NE(jsRuntime, nullptr);
    auto ability = std::make_shared<AbilityRuntime::JsUIAbility>(*jsRuntime);
    ASSERT_NE(ability, nullptr);
    napi_env env = jsRuntime->GetNapiEnv();
    ASSERT_NE(env, nullptr);
    napi_value jsContextObj = nullptr;
    ASSERT_EQ(napi_create_object(env, &jsContextObj), napi_ok);
    ASSERT_NE(jsContextObj, nullptr);
    auto mockRef = std::make_shared<NativeReferenceMock>();
    ON_CALL(*mockRef, GetNapiValue()).WillByDefault(Return(jsContextObj));
    ability->shellContextRef_ = mockRef;
    ASSERT_NE(ability->shellContextRef_, nullptr);
    auto windowStage = ability->GetWindowStage();
    ASSERT_EQ(windowStage, nullptr);
    GTEST_LOG_(INFO) << "JSUIAbility_GetWindowStage_0200 end";
}

/**
 * @tc.name: JSUIAbility_GetWindowStage_0300
 * @tc.desc: GetWindowStage test
 * @tc.desc: Verify function GetWindowStage.
 */
HWTEST_F(JsUiAbilityTest, JSUIAbility_GetWindowStage_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JSUIAbility_GetWindowStage_0300 start";
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto jsRuntime = static_cast<AbilityRuntime::JsRuntime*>(runtime.get());
    ASSERT_NE(jsRuntime, nullptr);
    auto ability = std::make_shared<AbilityRuntime::JsUIAbility>(*jsRuntime);
    ASSERT_NE(ability, nullptr);
    napi_env env = jsRuntime->GetNapiEnv();
    ASSERT_NE(env, nullptr);
    napi_value jsContextObj = nullptr;
    ASSERT_EQ(napi_create_object(env, &jsContextObj), napi_ok);
    ASSERT_NE(jsContextObj, nullptr);
    auto mockRef = std::make_shared<NativeReferenceMock>();
    napi_value windowStageObj = nullptr;
    napi_create_object(env, &windowStageObj);
    ASSERT_EQ(napi_set_named_property(env, jsContextObj, "windowStage", windowStageObj), napi_ok);
    ON_CALL(*mockRef, GetNapiValue()).WillByDefault(Return(jsContextObj));
    ability->shellContextRef_ = mockRef;
    ASSERT_NE(ability->shellContextRef_, nullptr);
    auto windowStage = ability->GetWindowStage();
    ASSERT_NE(windowStage, nullptr);
    GTEST_LOG_(INFO) << "JSUIAbility_GetWindowStage_0300 end";
}

/**
 * @tc.name: JSUIAbility_GetWindowStage_0400
 * @tc.desc: GetWindowStage test
 * @tc.desc: Verify function GetWindowStage.
 */
HWTEST_F(JsUiAbilityTest, JSUIAbility_GetWindowStage_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JSUIAbility_GetWindowStage_0400 start";
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto jsRuntime = static_cast<AbilityRuntime::JsRuntime*>(runtime.get());
    ASSERT_NE(jsRuntime, nullptr);
    auto ability = std::make_shared<AbilityRuntime::JsUIAbility>(*jsRuntime);
    ASSERT_NE(ability, nullptr);
    napi_env env = jsRuntime->GetNapiEnv();
    ASSERT_NE(env, nullptr);
    napi_value notAnObject = nullptr;
    ASSERT_EQ(napi_create_double(env, 42.0, &notAnObject), napi_ok);
    ASSERT_NE(notAnObject, nullptr);
    auto mockRef = std::make_shared<NativeReferenceMock>();
    ON_CALL(*mockRef, GetNapiValue()).WillByDefault(Return(notAnObject));
    ability->shellContextRef_ = mockRef;
    ASSERT_NE(ability->shellContextRef_, nullptr);
    auto windowStage = ability->GetWindowStage();
    ASSERT_EQ(windowStage, nullptr);
    GTEST_LOG_(INFO) << "JSUIAbility_GetWindowStage_0400 end";
}

/**
 * @tc.name: JSUIAbility_GetWindowStage_0500
 * @tc.desc: GetWindowStage test
 * @tc.desc: Verify function GetWindowStage.
 */
HWTEST_F(JsUiAbilityTest, JSUIAbility_GetWindowStage_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JSUIAbility_GetWindowStage_0500 start";
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto jsRuntime = static_cast<AbilityRuntime::JsRuntime*>(runtime.get());
    ASSERT_NE(jsRuntime, nullptr);
    auto ability = std::make_shared<AbilityRuntime::JsUIAbility>(*jsRuntime);
    ASSERT_NE(ability, nullptr);
    napi_env env = jsRuntime->GetNapiEnv();
    ASSERT_NE(env, nullptr);
    napi_value undefinedVal = nullptr;
    napi_status status = napi_get_undefined(env, &undefinedVal);
    ASSERT_EQ(status, napi_ok);
    napi_value jsContextObj = nullptr;
    status = napi_create_object(env, &jsContextObj);
    ASSERT_EQ(status, napi_ok);
    ASSERT_NE(jsContextObj, nullptr);
    ASSERT_EQ(napi_set_named_property(env, jsContextObj, "windowStage", undefinedVal), napi_ok);
    auto mockRef = std::make_shared<NativeReferenceMock>();
    ON_CALL(*mockRef, GetNapiValue()).WillByDefault(Return(jsContextObj));
    ability->shellContextRef_ = mockRef;
    ASSERT_NE(ability->shellContextRef_, nullptr);
    auto windowStage = ability->GetWindowStage();
    ASSERT_EQ(windowStage, nullptr);
    GTEST_LOG_(INFO) << "JSUIAbility_GetWindowStage_0500 end";
}

/**
 * @tc.name: JSUIAbility_GetWindowStage_0600
 * @tc.desc: GetWindowStage test
 * @tc.desc: Verify function GetWindowStage.
 */
HWTEST_F(JsUiAbilityTest, JSUIAbility_GetWindowStage_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JSUIAbility_GetWindowStage_0600 start";
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto jsRuntime = static_cast<AbilityRuntime::JsRuntime*>(runtime.get());
    ASSERT_NE(jsRuntime, nullptr);
    auto ability = std::make_shared<AbilityRuntime::JsUIAbility>(*jsRuntime);
    ASSERT_NE(ability, nullptr);
    napi_env env = jsRuntime->GetNapiEnv();
    ASSERT_NE(env, nullptr);
    napi_value nullVal = nullptr;
    napi_status status = napi_get_null(env, &nullVal);
    ASSERT_EQ(status, napi_ok);
    napi_value jsContextObj = nullptr;
    status = napi_create_object(env, &jsContextObj);
    ASSERT_EQ(status, napi_ok);
    ASSERT_NE(jsContextObj, nullptr);
    ASSERT_EQ(napi_set_named_property(env, jsContextObj, "windowStage", nullVal), napi_ok);
    auto mockRef = std::make_shared<NativeReferenceMock>();
    ON_CALL(*mockRef, GetNapiValue()).WillByDefault(Return(jsContextObj));
    ability->shellContextRef_ = mockRef;
    ASSERT_NE(ability->shellContextRef_, nullptr);
    auto windowStage = ability->GetWindowStage();
    ASSERT_EQ(windowStage, nullptr);
    GTEST_LOG_(INFO) << "JSUIAbility_GetWindowStage_0600 end";
}
} // namespace AbilityRuntime
} // namespace OHOS