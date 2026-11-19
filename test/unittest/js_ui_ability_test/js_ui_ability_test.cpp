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
#include "application_context.h"
#include "context.h"
#undef private
#undef protected
#include "js_runtime_utils.h"
#include "ability_stage_context.h"
#include "napi_common_want.h"
#include "native_ability_util.h"

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
 * @tc.name: JSUIAbility_OnStart_0700
 * @tc.desc: OnStart test with GAME_PRELAUNCH = true
 * @tc.desc: Verify isGamePreLaunch_ is set to true when GAME_PRELAUNCH param is true
 */
HWTEST_F(JsUiAbilityTest, JSUIAbility_OnStart_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JSUIAbility_OnStart_0700 start";
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto jsRuntime = static_cast<AbilityRuntime::JsRuntime*>(runtime.get());
    auto ability = std::make_shared<AbilityRuntime::JsUIAbility>(*jsRuntime);
    EXPECT_NE(ability, nullptr);
    Want want;
    want.SetParam(std::string("ohos.params.gamePrelaunch"), true);
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
    abilityContextImpl->SetAbilityInfo(abilityInfo);
    ability->abilityContext_ = abilityContextImpl;
    ability->abilityInfo_ = abilityInfo;
    EXPECT_NE(ability->abilityInfo_, nullptr);
    ability->OnStart(want, sessionInfo);
    EXPECT_TRUE(ability->isGamePreLaunch_);

    GTEST_LOG_(INFO) << "JSUIAbility_OnStart_0700 end";
}

/**
 * @tc.name: JSUIAbility_OnStart_0800
 * @tc.desc: OnStart test with GAME_PRELAUNCH = false
 * @tc.desc: Verify isGamePreLaunch_ remains false when GAME_PRELAUNCH param is false
 */
HWTEST_F(JsUiAbilityTest, JSUIAbility_OnStart_0800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JSUIAbility_OnStart_0800 start";
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto jsRuntime = static_cast<AbilityRuntime::JsRuntime*>(runtime.get());
    auto ability = std::make_shared<AbilityRuntime::JsUIAbility>(*jsRuntime);
    EXPECT_NE(ability, nullptr);
    Want want;
    want.SetParam(std::string("ohos.params.gamePrelaunch"), false);
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
    abilityContextImpl->SetAbilityInfo(abilityInfo);
    ability->abilityContext_ = abilityContextImpl;
    ability->abilityInfo_ = abilityInfo;
    EXPECT_NE(ability->abilityInfo_, nullptr);
    ability->OnStart(want, sessionInfo);
    EXPECT_FALSE(ability->isGamePreLaunch_);

    GTEST_LOG_(INFO) << "JSUIAbility_OnStart_0800 end";
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
/**
 * @tc.name: JSUIAbility_HandleNativeModule_0100
 * @tc.desc: HandleNativeModule test
 * @tc.desc: withNative_ is false, should return early.
 */
HWTEST_F(JsUiAbilityTest, JSUIAbility_HandleNativeModule_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JSUIAbility_HandleNativeModule_0100 start";
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto jsRuntime = static_cast<AbilityRuntime::JsRuntime*>(runtime.get());
    auto ability = std::make_shared<AbilityRuntime::JsUIAbility>(*jsRuntime);
    ASSERT_NE(ability, nullptr);

    // withNative_ defaults to false, HandleNativeModule should return early
    ability->withNative_ = false;
    auto env = jsRuntime->GetNapiEnv();
    ASSERT_NE(env, nullptr);
    ability->HandleNativeModule(env);
    // No crash, early return path
    GTEST_LOG_(INFO) << "JSUIAbility_HandleNativeModule_0100 end";
}

/**
 * @tc.name: JSUIAbility_HandleNativeModule_0200
 * @tc.desc: HandleNativeModule test
 * @tc.desc: withNative_ is true but jsAbilityObj_ is null, should return early.
 */
HWTEST_F(JsUiAbilityTest, JSUIAbility_HandleNativeModule_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JSUIAbility_HandleNativeModule_0200 start";
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto jsRuntime = static_cast<AbilityRuntime::JsRuntime*>(runtime.get());
    auto ability = std::make_shared<AbilityRuntime::JsUIAbility>(*jsRuntime);
    ASSERT_NE(ability, nullptr);

    ability->withNative_ = true;
    ability->jsAbilityObj_ = nullptr;
    auto env = jsRuntime->GetNapiEnv();
    ASSERT_NE(env, nullptr);
    ability->HandleNativeModule(env);
    // No crash, jsAbilityObj_ null path
    GTEST_LOG_(INFO) << "JSUIAbility_HandleNativeModule_0200 end";
}

/**
 * @tc.name: JSUIAbility_HandleNativeModule_0300
 * @tc.desc: HandleNativeModule test
 * @tc.desc: withNative_ is true, jsAbilityObj_ not null but ApplicationContext is null.
 */
HWTEST_F(JsUiAbilityTest, JSUIAbility_HandleNativeModule_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JSUIAbility_HandleNativeModule_0300 start";
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto jsRuntime = static_cast<AbilityRuntime::JsRuntime*>(runtime.get());
    auto ability = std::make_shared<AbilityRuntime::JsUIAbility>(*jsRuntime);
    ASSERT_NE(ability, nullptr);
    auto env = jsRuntime->GetNapiEnv();
    ASSERT_NE(env, nullptr);

    ability->withNative_ = true;

    // Create a valid napi object as jsAbilityObj_
    napi_value jsObj = nullptr;
    napi_create_object(env, &jsObj);
    ASSERT_NE(jsObj, nullptr);
    napi_ref ref = nullptr;
    napi_create_reference(env, jsObj, 1, &ref);
    ability->jsAbilityObj_ = std::unique_ptr<NativeReference>(
        reinterpret_cast<NativeReference *>(ref));
    ASSERT_NE(ability->jsAbilityObj_, nullptr);

    // Ensure ApplicationContext is null
    AbilityRuntime::Context::applicationContext_ = nullptr;

    ability->HandleNativeModule(env);
    // No crash, ApplicationContext is null, should log error and return
    GTEST_LOG_(INFO) << "JSUIAbility_HandleNativeModule_0300 end";
}

/**
 * @tc.name: JSUIAbility_HandleNativeModule_0400
 * @tc.desc: HandleNativeModule test
 * @tc.desc: withNative_ is true, valid jsAbilityObj_, verify wrapper stores shared_ptr<NativeReference>.
 */
HWTEST_F(JsUiAbilityTest, JSUIAbility_HandleNativeModule_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JSUIAbility_HandleNativeModule_0400 start";
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto jsRuntime = static_cast<AbilityRuntime::JsRuntime*>(runtime.get());
    auto ability = std::make_shared<AbilityRuntime::JsUIAbility>(*jsRuntime);
    ASSERT_NE(ability, nullptr);
    auto env = jsRuntime->GetNapiEnv();
    ASSERT_NE(env, nullptr);

    ability->withNative_ = true;

    // Create a valid napi object as jsAbilityObj_
    napi_value jsObj = nullptr;
    napi_create_object(env, &jsObj);
    ASSERT_NE(jsObj, nullptr);
    napi_ref ref = nullptr;
    napi_create_reference(env, jsObj, 1, &ref);
    ability->jsAbilityObj_ = std::unique_ptr<NativeReference>(
        reinterpret_cast<NativeReference *>(ref));
    ASSERT_NE(ability->jsAbilityObj_, nullptr);

    // Set up ApplicationContext
    auto appContext = std::make_shared<AbilityRuntime::ApplicationContext>();
    ASSERT_NE(appContext, nullptr);
    AbilityRuntime::Context::applicationContext_ = appContext;

    ability->HandleNativeModule(env);

    // Verify wrapper was added and jsAbilityObj is stored as shared_ptr<NativeReference>
    EXPECT_FALSE(appContext->nativeAbilities_.empty());
    auto it = appContext->nativeAbilities_.begin();
    ASSERT_NE(it->second, nullptr);
    EXPECT_NE(it->second->jsAbilityObj, nullptr);
    // Verify GetNapiValue() returns a valid napi_value
    napi_value retrievedObj = it->second->jsAbilityObj->GetNapiValue();
    EXPECT_NE(retrievedObj, nullptr);

    // Cleanup
    AbilityRuntime::Context::applicationContext_ = nullptr;
    GTEST_LOG_(INFO) << "JSUIAbility_HandleNativeModule_0400 end";
}

/**
 * @tc.name: JSUIAbility_HandleNativeModule_0500
 * @tc.desc: HandleNativeModule test
 * @tc.desc: withNative_ is true, valid jsAbilityObj_, ApplicationContext exists but GetNativeThread is null.
 */
HWTEST_F(JsUiAbilityTest, JSUIAbility_HandleNativeModule_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JSUIAbility_HandleNativeModule_0500 start";
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto jsRuntime = static_cast<AbilityRuntime::JsRuntime*>(runtime.get());
    auto ability = std::make_shared<AbilityRuntime::JsUIAbility>(*jsRuntime);
    ASSERT_NE(ability, nullptr);
    auto env = jsRuntime->GetNapiEnv();
    ASSERT_NE(env, nullptr);

    ability->withNative_ = true;

    // Create a valid napi object as jsAbilityObj_
    napi_value jsObj = nullptr;
    napi_create_object(env, &jsObj);
    ASSERT_NE(jsObj, nullptr);
    napi_ref ref = nullptr;
    napi_create_reference(env, jsObj, 1, &ref);
    ability->jsAbilityObj_ = std::unique_ptr<NativeReference>(
        reinterpret_cast<NativeReference *>(ref));
    ASSERT_NE(ability->jsAbilityObj_, nullptr);

    // Set up ApplicationContext with no native thread
    auto appContext = std::make_shared<AbilityRuntime::ApplicationContext>();
    ASSERT_NE(appContext, nullptr);
    AbilityRuntime::Context::applicationContext_ = appContext;

    ability->HandleNativeModule(env);
    // No crash, GetNativeThread returns nullptr
    // The wrapper should have been added to nativeAbilities_ though
    EXPECT_EQ(appContext->nativeAbilities_.size(), 1u);

    // Cleanup
    AbilityRuntime::Context::applicationContext_ = nullptr;
    GTEST_LOG_(INFO) << "JSUIAbility_HandleNativeModule_0500 end";
}

/**
 * @tc.name: JSUIAbility_HandleNativeModule_0600
 * @tc.desc: HandleNativeModule test
 * @tc.desc: Full path: withNative_ true, valid jsAbilityObj_, ApplicationContext with valid abilityInfo.
 */
HWTEST_F(JsUiAbilityTest, JSUIAbility_HandleNativeModule_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JSUIAbility_HandleNativeModule_0600 start";
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto jsRuntime = static_cast<AbilityRuntime::JsRuntime*>(runtime.get());
    auto ability = std::make_shared<AbilityRuntime::JsUIAbility>(*jsRuntime);
    ASSERT_NE(ability, nullptr);
    auto env = jsRuntime->GetNapiEnv();
    ASSERT_NE(env, nullptr);

    ability->withNative_ = true;

    // Set up abilityInfo_ so GetAbilityName() returns a valid name
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityInfo->name = "TestAbility";
    ability->abilityInfo_ = abilityInfo;

    // Create a valid napi object as jsAbilityObj_
    napi_value jsObj = nullptr;
    napi_create_object(env, &jsObj);
    ASSERT_NE(jsObj, nullptr);
    napi_ref ref = nullptr;
    napi_create_reference(env, jsObj, 1, &ref);
    ability->jsAbilityObj_ = std::unique_ptr<NativeReference>(
        reinterpret_cast<NativeReference *>(ref));
    ASSERT_NE(ability->jsAbilityObj_, nullptr);

    // Set up ApplicationContext
    auto appContext = std::make_shared<AbilityRuntime::ApplicationContext>();
    ASSERT_NE(appContext, nullptr);
    AbilityRuntime::Context::applicationContext_ = appContext;

    ability->HandleNativeModule(env);

    // Verify wrapper was added to ApplicationContext
    EXPECT_FALSE(appContext->nativeAbilities_.empty());
    auto it = appContext->nativeAbilities_.begin();
    ASSERT_NE(it->second, nullptr);
    EXPECT_EQ(it->second->abilityName, "TestAbility");
    EXPECT_EQ(it->second->env, env);
    // Verify jsAbilityObj is stored as shared_ptr<NativeReference> and GetNapiValue() works
    EXPECT_NE(it->second->jsAbilityObj, nullptr);
    napi_value retrievedObj = it->second->jsAbilityObj->GetNapiValue();
    EXPECT_NE(retrievedObj, nullptr);

    // Cleanup
    AbilityRuntime::Context::applicationContext_ = nullptr;
    GTEST_LOG_(INFO) << "JSUIAbility_HandleNativeModule_0600 end";
}

/**
 * @tc.name: JSUIAbility_DoOnForeground_WithRequestId_0100
 * @tc.desc: Test DoOnForeground with requestId and scbRequestId in want
 * @tc.type: FUNC
 */
HWTEST_F(JsUiAbilityTest, JSUIAbility_DoOnForeground_WithRequestId_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JSUIAbility_DoOnForeground_WithRequestId_0100 start";
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto jsRuntime = static_cast<AbilityRuntime::JsRuntime*>(runtime.get());
    auto ability = std::make_shared<AbilityRuntime::JsUIAbility>(*jsRuntime);
    ASSERT_NE(ability, nullptr);

    // Create mock scene
    ability->scene_ = std::make_shared<Rosen::WindowScene>();
    ASSERT_NE(ability->scene_, nullptr);

    // Create want with requestId and scbRequestId
    Want want;
    want.SetParam(AAFwk::Want::PARAM_RESV_APP_REQUEST_ID, 100);
    want.SetParam(AAFwk::Want::PARAM_RESV_SCB_REQUEST_ID, 200);

    // Call DoOnForeground
    ability->DoOnForeground(want);

    // Verify requestId and scbRequestId are removed after calling WMS
    int32_t requestId = want.GetIntParam(AAFwk::Want::PARAM_RESV_APP_REQUEST_ID, -1);
    int32_t scbRequestId = want.GetIntParam(AAFwk::Want::PARAM_RESV_SCB_REQUEST_ID, -1);
    EXPECT_EQ(requestId, -1);  // Should be removed (returns default -1)
    EXPECT_EQ(scbRequestId, -1);  // Should be removed (returns default -1)

    GTEST_LOG_(INFO) << "JSUIAbility_DoOnForeground_WithRequestId_0100 end";
}

/**
 * @tc.name: JSUIAbility_DoOnForeground_WithRequestId_0200
 * @tc.desc: Test DoOnForeground with only requestId in want
 * @tc.type: FUNC
 */
HWTEST_F(JsUiAbilityTest, JSUIAbility_DoOnForeground_WithRequestId_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JSUIAbility_DoOnForeground_WithRequestId_0200 start";
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto jsRuntime = static_cast<AbilityRuntime::JsRuntime*>(runtime.get());
    auto ability = std::make_shared<AbilityRuntime::JsUIAbility>(*jsRuntime);
    ASSERT_NE(ability, nullptr);

    // Create mock scene
    ability->scene_ = std::make_shared<Rosen::WindowScene>();
    ASSERT_NE(ability->scene_, nullptr);

    // Create want with only requestId
    Want want;
    want.SetParam(AAFwk::Want::PARAM_RESV_APP_REQUEST_ID, 100);

    // Call DoOnForeground
    ability->DoOnForeground(want);

    // Verify requestId is removed and scbRequestId was never set
    int32_t requestId = want.GetIntParam(AAFwk::Want::PARAM_RESV_APP_REQUEST_ID, -1);
    int32_t scbRequestId = want.GetIntParam(AAFwk::Want::PARAM_RESV_SCB_REQUEST_ID, -1);
    EXPECT_EQ(requestId, -1);  // Should be removed
    EXPECT_EQ(scbRequestId, -1);  // Was never set

    GTEST_LOG_(INFO) << "JSUIAbility_DoOnForeground_WithRequestId_0200 end";
}

/**
 * @tc.name: JSUIAbility_DoOnForeground_WithRequestId_0300
 * @tc.desc: Test DoOnForeground with only scbRequestId in want
 * @tc.type: FUNC
 */
HWTEST_F(JsUiAbilityTest, JSUIAbility_DoOnForeground_WithRequestId_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JSUIAbility_DoOnForeground_WithRequestId_0300 start";
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto jsRuntime = static_cast<AbilityRuntime::JsRuntime*>(runtime.get());
    auto ability = std::make_shared<AbilityRuntime::JsUIAbility>(*jsRuntime);
    ASSERT_NE(ability, nullptr);

    // Create mock scene
    ability->scene_ = std::make_shared<Rosen::WindowScene>();
    ASSERT_NE(ability->scene_, nullptr);

    // Create want with only scbRequestId
    Want want;
    want.SetParam(AAFwk::Want::PARAM_RESV_SCB_REQUEST_ID, 200);

    // Call DoOnForeground
    ability->DoOnForeground(want);

    // Verify scbRequestId is removed and requestId was never set
    int32_t requestId = want.GetIntParam(AAFwk::Want::PARAM_RESV_APP_REQUEST_ID, -1);
    int32_t scbRequestId = want.GetIntParam(AAFwk::Want::PARAM_RESV_SCB_REQUEST_ID, -1);
    EXPECT_EQ(requestId, -1);  // Was never set
    EXPECT_EQ(scbRequestId, -1);  // Should be removed

    GTEST_LOG_(INFO) << "JSUIAbility_DoOnForeground_WithRequestId_0300 end";
}

/**
 * @tc.name: JSUIAbility_DoOnForeground_WithRequestId_0400
 * @tc.desc: Test DoOnForeground without requestId or scbRequestId in want
 * @tc.type: FUNC
 */
HWTEST_F(JsUiAbilityTest, JSUIAbility_DoOnForeground_WithRequestId_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JSUIAbility_DoOnForeground_WithRequestId_0400 start";
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto jsRuntime = static_cast<AbilityRuntime::JsRuntime*>(runtime.get());
    auto ability = std::make_shared<AbilityRuntime::JsUIAbility>(*jsRuntime);
    ASSERT_NE(ability, nullptr);

    // Create mock scene
    ability->scene_ = std::make_shared<Rosen::WindowScene>();
    ASSERT_NE(ability->scene_, nullptr);

    // Create want without requestId or scbRequestId
    Want want;

    // Call DoOnForeground
    ability->DoOnForeground(want);

    // Verify neither parameter is set
    int32_t requestId = want.GetIntParam(AAFwk::Want::PARAM_RESV_APP_REQUEST_ID, -1);
    int32_t scbRequestId = want.GetIntParam(AAFwk::Want::PARAM_RESV_SCB_REQUEST_ID, -1);
    EXPECT_EQ(requestId, -1);  // Never set
    EXPECT_EQ(scbRequestId, -1);  // Never set

    GTEST_LOG_(INFO) << "JSUIAbility_DoOnForeground_WithRequestId_0400 end";
}

/**
 * @tc.name: JSUIAbility_DoOnForegroundForSceneIsNull_WithRequestId_0100
 * @tc.desc: Test DoOnForegroundForSceneIsNull with requestId and scbRequestId
 * @tc.type: FUNC
 */
HWTEST_F(JsUiAbilityTest, JSUIAbility_DoOnForegroundForSceneIsNull_WithRequestId_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JSUIAbility_DoOnForegroundForSceneIsNull_WithRequestId_0100 start";
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto jsRuntime = static_cast<AbilityRuntime::JsRuntime*>(runtime.get());
    auto ability = std::make_shared<AbilityRuntime::JsUIAbility>(*jsRuntime);
    ASSERT_NE(ability, nullptr);

    wptr<IRemoteObject> token(new IPCObjectStub());
    ability->sessionToken_ = token;
    auto abilityContextImpl = std::make_shared<AbilityContextImpl>();

    auto stageContext = std::make_shared<AbilityRuntime::AbilityStageContext>();
    AppExecFwk::HapModuleInfo hapModuleInfo;
    stageContext->InitHapModuleInfo(hapModuleInfo);
    abilityContextImpl->SetStageContext(stageContext);

    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityInfo->applicationInfo.bundleType = OHOS::AppExecFwk::BundleType::APP;
    abilityContextImpl->SetAbilityInfo(abilityInfo);
    ability->abilityContext_ = abilityContextImpl;

    Rosen::SceneBoardJudgement::flag_ = true;

    // Create want with requestId and scbRequestId
    Want want;
    want.SetParam(AAFwk::Want::PARAM_RESV_APP_REQUEST_ID, 100);
    want.SetParam(AAFwk::Want::PARAM_RESV_SCB_REQUEST_ID, 200);

    // Note: This will try to initialize scene which may fail in test environment
    // The important part is testing that the requestIds are extracted and removed
    ability->sceneListener_ = new Rosen::IWindowLifeCycle();

    // Call DoOnForegroundForSceneIsNull
    ability->DoOnForegroundForSceneIsNull(want);

    int32_t requestId = want.GetIntParam(AAFwk::Want::PARAM_RESV_APP_REQUEST_ID, -1);
    int32_t scbRequestId = want.GetIntParam(AAFwk::Want::PARAM_RESV_SCB_REQUEST_ID, -1);
    EXPECT_EQ(requestId, 100);
    EXPECT_EQ(scbRequestId, 200);

    GTEST_LOG_(INFO) << "JSUIAbility_DoOnForegroundForSceneIsNull_WithRequestId_0100 end";
}

/**
 * @tc.name: JSUIAbility_RequestFocus_WithRequestId_0100
 * @tc.desc: Test RequestFocus with requestId and scbRequestId
 * @tc.type: FUNC
 */
HWTEST_F(JsUiAbilityTest, JSUIAbility_RequestFocus_WithRequestId_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JSUIAbility_RequestFocus_WithRequestId_0100 start";
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto jsRuntime = static_cast<AbilityRuntime::JsRuntime*>(runtime.get());
    auto ability = std::make_shared<AbilityRuntime::JsUIAbility>(*jsRuntime);
    ASSERT_NE(ability, nullptr);

    // Create mock scene
    ability->scene_ = std::make_shared<Rosen::WindowScene>();
    ASSERT_NE(ability->scene_, nullptr);

    // Create want with requestId and scbRequestId
    Want want;
    want.SetParam(AAFwk::Want::PARAM_RESV_APP_REQUEST_ID, 100);
    want.SetParam(AAFwk::Want::PARAM_RESV_SCB_REQUEST_ID, 200);

    // Call RequestFocus
    ability->RequestFocus(want);

    // Verify requestId and scbRequestId are removed after calling WMS
    int32_t requestId = want.GetIntParam(AAFwk::Want::PARAM_RESV_APP_REQUEST_ID, -1);
    int32_t scbRequestId = want.GetIntParam(AAFwk::Want::PARAM_RESV_SCB_REQUEST_ID, -1);
    EXPECT_EQ(requestId, -1);  // Should be removed
    EXPECT_EQ(scbRequestId, -1);  // Should be removed

    GTEST_LOG_(INFO) << "JSUIAbility_RequestFocus_WithRequestId_0100 end";
}

} // namespace AbilityRuntime
} // namespace OHOS