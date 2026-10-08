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

#include "context.h"
#include "context_impl.h"
#include "js_runtime_lite.h"
#include "js_ui_extension_callback.h"
#include "js_ui_extension_content_session.h"
#include "napi_common_want_agent.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "session_info.h"
#include "ui_extension_context.h"

using namespace testing;
using namespace testing::ext;
using namespace testing::mt;

namespace OHOS {
namespace AbilityRuntime {

class JsUIExtensionContentSessionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void JsUIExtensionContentSessionTest::SetUpTestCase() {}

void JsUIExtensionContentSessionTest::TearDownTestCase() {}

void JsUIExtensionContentSessionTest::SetUp() {}

void JsUIExtensionContentSessionTest::TearDown() {}

/**
 * @tc.number: ListenerManagementTestTest_0100
 * @tc.name: constructor function、AddListener and RemoveListener test
 * @tc.desc: constructor function、AddListener and RemoveListener test
 */
HWTEST_F(JsUIExtensionContentSessionTest, ListenerManagementTestTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListenerManagementTestTest_0100 start";

    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    sptr<Rosen::Window> uiWindow = nullptr;
    std::shared_ptr<AbilityRuntime::Context> context = nullptr;
    std::weak_ptr<AbilityRuntime::Context> contextWeak = context;
    std::shared_ptr<AbilityResultListeners> abilityResultListeners = nullptr;
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest1 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow, contextWeak, abilityResultListeners);
    ASSERT_NE(jsUIExtensionContentSessionTest1, nullptr);
    EXPECT_NE(jsUIExtensionContentSessionTest1->listener_, nullptr);

    abilityResultListeners = std::make_shared<AbilityResultListeners>();
    ASSERT_NE(abilityResultListeners, nullptr);

    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest2 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow, contextWeak, abilityResultListeners);
    ASSERT_NE(jsUIExtensionContentSessionTest2, nullptr);
    EXPECT_NE(jsUIExtensionContentSessionTest2->listener_, nullptr);

    sessionInfo = new (std::nothrow) AAFwk::SessionInfo();
    EXPECT_NE(sessionInfo, nullptr);

    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest3 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow, contextWeak, abilityResultListeners);
    ASSERT_NE(jsUIExtensionContentSessionTest3, nullptr);
    EXPECT_NE(jsUIExtensionContentSessionTest3->listener_, nullptr);

    int uiExtensionComponentId = 1;
    sessionInfo->uiExtensionComponentId = uiExtensionComponentId;
    EXPECT_EQ(abilityResultListeners->listeners_.size(), 0);

    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest4 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow, contextWeak, abilityResultListeners);
    ASSERT_NE(jsUIExtensionContentSessionTest4, nullptr);
    EXPECT_NE(jsUIExtensionContentSessionTest4->listener_, nullptr);
    EXPECT_EQ(abilityResultListeners->listeners_.size(), 1);

    abilityResultListeners->RemoveListener(uiExtensionComponentId);
    EXPECT_EQ(abilityResultListeners->listeners_.size(), 0);

    GTEST_LOG_(INFO) << "ListenerManagementTestTest_0100 end";
}

/**
 * @tc.number: ResultCallbacksTestTest_0100
 * @tc.name: OnAbilityResult and IsMatch OnAbilityResultInner SaveResultCallbacks test
 * @tc.desc: OnAbilityResult and IsMatch OnAbilityResultInner SaveResultCallbacks test
 */
HWTEST_F(JsUIExtensionContentSessionTest, ResultCallbacksTestTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ResultCallbacksTestTest_0100 start";

    std::shared_ptr<AbilityResultListeners> abilityResultListeners = std::make_shared<AbilityResultListeners>();
    ASSERT_NE(abilityResultListeners, nullptr);
    std::shared_ptr<UISessionAbilityResultListener> listener = std::make_shared<UISessionAbilityResultListener>();
    ASSERT_NE(listener, nullptr);

    int idTest1 = 1;
    RuntimeTask taskTest1;
    int idTest2 = 2;
    RuntimeTask taskTest2;
    listener->SaveResultCallbacks(idTest1, std::move(taskTest1));
    listener->SaveResultCallbacks(idTest2, std::move(taskTest2));
    EXPECT_EQ(listener->resultCallbacks_.size(), 2);

    abilityResultListeners->listeners_.emplace(0, nullptr);
    abilityResultListeners->listeners_.emplace(idTest1, listener);
    EXPECT_EQ(abilityResultListeners->listeners_.size(), 2);

    int requestCodeTest1 = 1;
    int resultCodeTest1 = 1;
    AAFwk::Want resultData;
    abilityResultListeners->OnAbilityResult(requestCodeTest1, resultCodeTest1, resultData);
    EXPECT_EQ(listener->resultCallbacks_.size(), 1);

    int requestCodeTest2 = 2;
    int resultCodeTest2 = 2;
    abilityResultListeners->OnAbilityResult(requestCodeTest1, resultCodeTest1, resultData);
    EXPECT_EQ(listener->resultCallbacks_.size(), 1);

    listener->OnAbilityResultInner(requestCodeTest2, resultCodeTest2, resultData);
    EXPECT_EQ(listener->resultCallbacks_.size(), 0);

    GTEST_LOG_(INFO) << "ResultCallbacksTestTest_0100 end";
}

/**
 * @tc.number: AbilityStartTestTest_0100
 * @tc.name: Ability Start test
 * @tc.desc: Ability Start test
 */
HWTEST_F(JsUIExtensionContentSessionTest, AbilityStartTestTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityStartTestTest_0100 start";

    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    sptr<Rosen::Window> uiWindow = nullptr;
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSession =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSession, nullptr);
    napi_env env = {};
    napi_callback_info info = {};
    EXPECT_EQ(jsUIExtensionContentSession->StartAbility(env, info), NULL);
    EXPECT_EQ(jsUIExtensionContentSession->StartAbilityAsCaller(env, info), NULL);
    EXPECT_EQ(jsUIExtensionContentSession->GetUIExtensionHostWindowProxy(env, info), NULL);
    EXPECT_EQ(jsUIExtensionContentSession->GetUIExtensionWindowProxy(env, info), NULL);
    EXPECT_EQ(jsUIExtensionContentSession->StartAbilityForResult(env, info), NULL);
    EXPECT_EQ(jsUIExtensionContentSession->TerminateSelf(env, info), NULL);
    EXPECT_EQ(jsUIExtensionContentSession->TerminateSelfWithResult(env, info), NULL);
    EXPECT_EQ(jsUIExtensionContentSession->SendData(env, info), NULL);
    EXPECT_EQ(jsUIExtensionContentSession->SetReceiveDataCallback(env, info), NULL);
    EXPECT_EQ(jsUIExtensionContentSession->SetReceiveDataForResultCallback(env, info), NULL);
    EXPECT_EQ(jsUIExtensionContentSession->LoadContent(env, info), NULL);
    EXPECT_EQ(jsUIExtensionContentSession->LoadContentByName(env, info), nullptr);
    EXPECT_EQ(jsUIExtensionContentSession->SetWindowBackgroundColor(env, info), NULL);
    EXPECT_EQ(jsUIExtensionContentSession->SetWindowPrivacyMode(env, info), NULL);
    EXPECT_EQ(jsUIExtensionContentSession->StartAbilityByType(env, info), NULL);

    GTEST_LOG_(INFO) << "AbilityStartTestTest_0100 end";
}

/**
 * @tc.number: OnStartAbilityTestTest_0100
 * @tc.name: OnStartAbility test
 * @tc.desc: OnStartAbility test
 */
HWTEST_F(JsUIExtensionContentSessionTest, OnStartAbilityTestTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnStartAbilityTestTest_0100 start";

    OHOS::AbilityRuntime::Runtime::Options options;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_NE(jsEnv, nullptr);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    EXPECT_NE(env, nullptr);

    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    sptr<Rosen::Window> uiWindow = nullptr;
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest1 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest1, nullptr);
    NapiCallbackInfo info;
    info.argc = 0;
    EXPECT_NE(jsUIExtensionContentSessionTest1->OnStartAbility(env, info), NULL);

    info.argc = 1;
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest2 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest2, nullptr);
    EXPECT_NE(jsUIExtensionContentSessionTest2->OnStartAbility(env, info), NULL);

    AAFwk::Want want;
    AbilityRuntime::Runtime::Options optionsWant;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnvWant = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(optionsWant, jsEnvWant);
    ASSERT_NE(jsEnvWant, nullptr);
    EXPECT_NE(jsEnvWant->GetNativeEngine(), nullptr);
    napi_env envWant = reinterpret_cast<napi_env>(jsEnvWant->GetNativeEngine());
    info.argv[0] = AppExecFwk::WrapWant(envWant, want);

    EXPECT_NE(info.argv[0], NULL);
    unsigned int flags = 0;
    want.SetFlags(flags);
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest3 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest3, nullptr);
    std::shared_ptr<AbilityRuntime::Context> context = nullptr;
    jsUIExtensionContentSessionTest3->context_ = context;
    EXPECT_NE(jsUIExtensionContentSessionTest3->OnStartAbility(env, info), NULL);

    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest4 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest4, nullptr);
    flags = 2048;
    want.SetFlags(flags);
    info.argc = 3;
    info.argv[1] = AppExecFwk::WrapWant(envWant, want);
    EXPECT_NE(info.argv[1], NULL);
    EXPECT_NE(jsUIExtensionContentSessionTest4->OnStartAbility(env, info), NULL);
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnvWant->GetNativeEngine()));

    GTEST_LOG_(INFO) << "OnStartAbilityTestTest_0100 end";
}

/**
 * @tc.number: OnGetUIExtensionHostWindowProxyTestTest_0100
 * @tc.name: OnGetUIExtensionHostWindowProxy test
 * @tc.desc: OnGetUIExtensionHostWindowProxy test
 */
HWTEST_F(JsUIExtensionContentSessionTest, OnGetUIExtensionHostWindowProxyTestTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnGetUIExtensionHostWindowProxyTestTest_0100 start";

    AbilityRuntime::Runtime::Options options;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_NE(jsEnv, nullptr);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    EXPECT_NE(env, nullptr);

    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    sptr<Rosen::Window> uiWindow = nullptr;
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest1 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest1, nullptr);
    NapiCallbackInfo info;
    EXPECT_NE(jsUIExtensionContentSessionTest1->OnGetUIExtensionHostWindowProxy(env, info), NULL);

    sessionInfo = new (std::nothrow) AAFwk::SessionInfo();
    EXPECT_NE(sessionInfo, nullptr);
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest2 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest2, nullptr);
    EXPECT_NE(jsUIExtensionContentSessionTest2->OnGetUIExtensionHostWindowProxy(env, info), NULL);

    uiWindow = new Rosen::Window();
    EXPECT_NE(uiWindow, nullptr);
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest3 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest3, nullptr);
    EXPECT_NE(jsUIExtensionContentSessionTest3->OnGetUIExtensionHostWindowProxy(env, info), NULL);
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));

    GTEST_LOG_(INFO) << "OnGetUIExtensionHostWindowProxyTestTest_0100 end";
}

/**
 * @tc.number: OnGetUIExtensionWindowProxyTestTest_0100
 * @tc.name: OnGetUIExtensionWindowProxy test
 * @tc.desc: OnGetUIExtensionWindowProxy test
 */
HWTEST_F(JsUIExtensionContentSessionTest, OnGetUIExtensionWindowProxyTestTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnGetUIExtensionWindowProxyTestTest_0100 start";

    AbilityRuntime::Runtime::Options options;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_NE(jsEnv, nullptr);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    EXPECT_NE(env, nullptr);

    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    sptr<Rosen::Window> uiWindow = nullptr;
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest1 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest1, nullptr);
    NapiCallbackInfo info;
    EXPECT_NE(jsUIExtensionContentSessionTest1->OnGetUIExtensionWindowProxy(env, info), NULL);

    sessionInfo = new (std::nothrow) AAFwk::SessionInfo();
    EXPECT_NE(sessionInfo, nullptr);
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest2 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest2, nullptr);
    EXPECT_NE(jsUIExtensionContentSessionTest2->OnGetUIExtensionWindowProxy(env, info), NULL);

    uiWindow = new Rosen::Window();
    EXPECT_NE(uiWindow, nullptr);
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest3 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest3, nullptr);
    EXPECT_NE(jsUIExtensionContentSessionTest3->OnGetUIExtensionWindowProxy(env, info), NULL);
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));

    GTEST_LOG_(INFO) << "OnGetUIExtensionWindowProxyTestTest_0100 end";
}

/**
 * @tc.number: OnStartAbilityAsCallerTestTest_0100
 * @tc.name: OnStartAbilityAsCaller test
 * @tc.desc: OnStartAbilityAsCaller test
 */
HWTEST_F(JsUIExtensionContentSessionTest, OnStartAbilityAsCallerTestTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnStartAbilityAsCallerTestTest_0100 start";

    AbilityRuntime::Runtime::Options options;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_NE(jsEnv, nullptr);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    EXPECT_NE(env, nullptr);

    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    sptr<Rosen::Window> uiWindow = nullptr;
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest1 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest1, nullptr);
    NapiCallbackInfo info;
    info.argc = 0;
    EXPECT_NE(jsUIExtensionContentSessionTest1->OnStartAbilityAsCaller(env, info), NULL);

    info.argc = 1;
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest2 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest2, nullptr);
    EXPECT_NE(jsUIExtensionContentSessionTest2->OnStartAbilityAsCaller(env, info), NULL);
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));

    GTEST_LOG_(INFO) << "OnStartAbilityAsCallerTestTest_0100 end";
}

/**
 * @tc.number: OnStartAbilityAsCallerTestTest_0200
 * @tc.name: OnStartAbilityAsCaller test
 * @tc.desc: OnStartAbilityAsCaller test
 */
HWTEST_F(JsUIExtensionContentSessionTest, OnStartAbilityAsCallerTestTest_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnStartAbilityAsCallerTestTest_0200 start";

    AbilityRuntime::Runtime::Options options;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_NE(jsEnv, nullptr);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    EXPECT_NE(env, nullptr);

    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    sptr<Rosen::Window> uiWindow = nullptr;
    NapiCallbackInfo info;
    info.argc = 2;
    AAFwk::Want want;
    AbilityRuntime::Runtime::Options optionsWant;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnvWant = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(optionsWant, jsEnvWant);
    ASSERT_NE(jsEnvWant, nullptr);
    EXPECT_NE(jsEnvWant->GetNativeEngine(), nullptr);
    napi_env envWant = reinterpret_cast<napi_env>(jsEnvWant->GetNativeEngine());
    info.argv[0] = AppExecFwk::WrapWant(envWant, want);
    EXPECT_NE(info.argv[0], NULL);
    info.argv[1] = AppExecFwk::WrapWant(envWant, want);
    EXPECT_NE(info.argv[1], NULL);
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest3 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest3, nullptr);
    std::shared_ptr<Context> context = nullptr;
    jsUIExtensionContentSessionTest3->context_ = context;
    EXPECT_NE(jsUIExtensionContentSessionTest3->OnStartAbilityAsCaller(env, info), NULL);
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnvWant->GetNativeEngine()));

    GTEST_LOG_(INFO) << "OnStartAbilityAsCallerTestTest_0200 end";
}

/**
 * @tc.number: OnStartAbilityAsCallerTestTest_0300
 * @tc.name: OnStartAbilityAsCaller test
 * @tc.desc: OnStartAbilityAsCaller test
 */
HWTEST_F(JsUIExtensionContentSessionTest, OnStartAbilityAsCallerTestTest_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnStartAbilityAsCallerTestTest_0300 start";

    AbilityRuntime::Runtime::Options options;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_NE(jsEnv, nullptr);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    EXPECT_NE(env, nullptr);

    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    sptr<Rosen::Window> uiWindow = nullptr;
    NapiCallbackInfo info;
    info.argc = 2;
    AAFwk::Want want;
    AbilityRuntime::Runtime::Options optionsWant;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnvWant = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(optionsWant, jsEnvWant);
    ASSERT_NE(jsEnvWant, nullptr);
    EXPECT_NE(jsEnvWant->GetNativeEngine(), nullptr);
    napi_env envWant = reinterpret_cast<napi_env>(jsEnvWant->GetNativeEngine());
    info.argv[0] = AppExecFwk::WrapWant(envWant, want);
    EXPECT_NE(info.argv[0], NULL);
    info.argv[1] = AppExecFwk::WrapWant(envWant, want);
    EXPECT_NE(info.argv[1], NULL);

    std::shared_ptr<Context> context = std::make_shared<ContextImpl>();
    EXPECT_NE(context, nullptr);
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest4 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest4, nullptr);
    jsUIExtensionContentSessionTest4->context_ = context;
    EXPECT_NE(jsUIExtensionContentSessionTest4->OnStartAbilityAsCaller(env, info), NULL);

    sessionInfo = new (std::nothrow) AAFwk::SessionInfo();
    EXPECT_NE(sessionInfo, nullptr);
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest5 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest5, nullptr);
    EXPECT_NE(jsUIExtensionContentSessionTest5->OnStartAbilityAsCaller(env, info), NULL);

    info.argc = 3;
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest6 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest6, nullptr);
    EXPECT_NE(jsUIExtensionContentSessionTest6->OnStartAbilityAsCaller(env, info), NULL);
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnvWant->GetNativeEngine()));

    GTEST_LOG_(INFO) << "OnStartAbilityAsCallerTestTest_0300 end";
}

/**
 * @tc.number: StartAbilityExecuteCallbackTestTest_0100
 * @tc.name: StartAbilityExecuteCallback test
 * @tc.desc: StartAbilityExecuteCallback test
 */
HWTEST_F(JsUIExtensionContentSessionTest, StartAbilityExecuteCallbackTestTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartAbilityExecuteCallbackTestTest_0100 start";

    AbilityRuntime::Runtime::Options options;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_NE(jsEnv, nullptr);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    EXPECT_NE(env, nullptr);

    AAFwk::Want want;
    unsigned int flags = 2048;
    want.SetFlags(flags);
    AbilityRuntime::Runtime::Options optionsWant;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnvWant = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(optionsWant, jsEnvWant);
    ASSERT_NE(jsEnvWant, nullptr);
    EXPECT_NE(jsEnvWant->GetNativeEngine(), nullptr);
    napi_env envWant = reinterpret_cast<napi_env>(jsEnvWant->GetNativeEngine());
    NapiCallbackInfo info;
    info.argc = 2;
    info.argv[0] = AppExecFwk::WrapWant(envWant, want);
    EXPECT_NE(info.argv[0], NULL);
    info.argv[1] = AppExecFwk::WrapWant(envWant, want);
    EXPECT_NE(info.argv[1], NULL);
    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    sptr<Rosen::Window> uiWindow = nullptr;

    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest1 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest1, nullptr);
    std::shared_ptr<Context> context = nullptr;
    jsUIExtensionContentSessionTest1->context_ = context;
    size_t unwrapArgc = 0;
    std::shared_ptr<int> innerErrorCode = std::make_shared<int>(ERR_OK);
    NapiAsyncTask::ExecuteCallback executeTest1 =
        jsUIExtensionContentSessionTest1->StartAbilityExecuteCallback(want, unwrapArgc, env, info, innerErrorCode);
    EXPECT_EQ(unwrapArgc, 1);
    executeTest1();
    EXPECT_NE(*innerErrorCode, static_cast<int>(ERR_OK));
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnvWant->GetNativeEngine()));

    GTEST_LOG_(INFO) << "StartAbilityExecuteCallbackTestTest_0100 end";
}

/**
 * @tc.number: StartAbilityExecuteCallbackTest_0200
 * @tc.name: StartAbilityExecuteCallback test
 * @tc.desc: StartAbilityExecuteCallback test
 */
HWTEST_F(JsUIExtensionContentSessionTest, StartAbilityExecuteCallbackTest_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartAbilityExecuteCallbackTest_0200 start";

    AbilityRuntime::Runtime::Options options;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_NE(jsEnv, nullptr);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    EXPECT_NE(env, nullptr);

    AAFwk::Want want;
    unsigned int flags = 2048;
    want.SetFlags(flags);
    NapiCallbackInfo info;
    info.argc = 2;
    info.argv[0] = AppExecFwk::WrapWant(env, want);
    EXPECT_NE(info.argv[0], NULL);
    info.argv[1] = AppExecFwk::WrapWant(env, want);
    EXPECT_NE(info.argv[1], NULL);
    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    sptr<Rosen::Window> uiWindow = nullptr;

    size_t unwrapArgc = 0;
    std::shared_ptr<Context> context = std::make_shared<ContextImpl>();
    EXPECT_NE(context, nullptr);
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    jsUIExtensionContentSessionTest->context_ = context;
    std::shared_ptr<int> innerErrorCode = std::make_shared<int>(ERR_OK);
    NapiAsyncTask::ExecuteCallback executeTest =
        jsUIExtensionContentSessionTest->StartAbilityExecuteCallback(want, unwrapArgc, env, info, innerErrorCode);
    EXPECT_EQ(unwrapArgc, 1);
    executeTest();
    EXPECT_NE(*innerErrorCode, static_cast<int>(ERR_OK));
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));

    GTEST_LOG_(INFO) << "StartAbilityExecuteCallbackTest_0200 end";
}

/**
 * @tc.number: StartAbilityExecuteCallbackTest_0300
 * @tc.name: StartAbilityExecuteCallback test
 * @tc.desc: StartAbilityExecuteCallback test
 */
HWTEST_F(JsUIExtensionContentSessionTest, StartAbilityExecuteCallbackTest_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartAbilityExecuteCallbackTest_0300 start";

    AbilityRuntime::Runtime::Options options;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_NE(jsEnv, nullptr);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    EXPECT_NE(env, nullptr);

    AAFwk::Want want;
    unsigned int flags = 2048;
    want.SetFlags(flags);
    NapiCallbackInfo info;
    info.argc = 2;
    info.argv[0] = AppExecFwk::WrapWant(env, want);
    EXPECT_NE(info.argv[0], NULL);
    info.argv[1] = AppExecFwk::WrapWant(env, want);
    EXPECT_NE(info.argv[1], NULL);
    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    sptr<Rosen::Window> uiWindow = nullptr;

    size_t unwrapArgc = 0;
    std::shared_ptr<Context> context = std::make_shared<ContextImpl>();
    EXPECT_NE(context, nullptr);
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest, nullptr);
    jsUIExtensionContentSessionTest->context_ = context;
    sptr<JsFreeInstallObserver> freeInstallObserver = new (std::nothrow) JsFreeInstallObserver(env);
    EXPECT_NE(freeInstallObserver, nullptr);
    jsUIExtensionContentSessionTest->freeInstallObserver_ = freeInstallObserver;
    std::shared_ptr<int> innerErrorCode = std::make_shared<int>(ERR_OK);
    NapiAsyncTask::ExecuteCallback executeTest =
        jsUIExtensionContentSessionTest->StartAbilityExecuteCallback(want, unwrapArgc, env, info, innerErrorCode);
    EXPECT_EQ(unwrapArgc, 1);
    executeTest();
    EXPECT_NE(*innerErrorCode, static_cast<int>(ERR_OK));
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));

    GTEST_LOG_(INFO) << "StartAbilityExecuteCallbackTest_0300 end";
}

/**
 * @tc.number: OnStartAbilityForResultTestTest_0100
 * @tc.name: OnStartAbilityForResult test
 * @tc.desc: OnStartAbilityForResult test
 */
HWTEST_F(JsUIExtensionContentSessionTest, OnStartAbilityForResultTestTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnStartAbilityForResultTestTest_0100 start";

    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    sptr<Rosen::Window> uiWindow = nullptr;
    AbilityRuntime::Runtime::Options options;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_NE(jsEnv, nullptr);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    EXPECT_NE(env, nullptr);
    NapiCallbackInfo info;

    info.argc = 0;
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest1 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    EXPECT_NE(jsUIExtensionContentSessionTest1, nullptr);
    EXPECT_NE(jsUIExtensionContentSessionTest1->OnStartAbilityForResult(env, info), NULL);

    info.argc = 1;
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest2 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    EXPECT_NE(jsUIExtensionContentSessionTest2, nullptr);
    EXPECT_NE(jsUIExtensionContentSessionTest2->OnStartAbilityForResult(env, info), NULL);
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));

    GTEST_LOG_(INFO) << "OnStartAbilityForResultTestTest_0100 end";
}

/**
 * @tc.number: OnStartAbilityForResultTestTest_0200
 * @tc.name: OnStartAbilityForResult test
 * @tc.desc: OnStartAbilityForResult test
 */
HWTEST_F(JsUIExtensionContentSessionTest, OnStartAbilityForResultTestTest_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnStartAbilityForResultTestTest_0200 start";

    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    sptr<Rosen::Window> uiWindow = nullptr;
    AbilityRuntime::Runtime::Options options;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_NE(jsEnv, nullptr);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    EXPECT_NE(env, nullptr);
    NapiCallbackInfo info;

    info.argc = 2;
    AAFwk::Want want;
    unsigned int flags = 2048;
    want.SetFlags(flags);
    AbilityRuntime::Runtime::Options optionsWant;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnvWant = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(optionsWant, jsEnvWant);
    ASSERT_NE(jsEnvWant, nullptr);
    EXPECT_NE(jsEnvWant->GetNativeEngine(), nullptr);
    napi_env envWant = reinterpret_cast<napi_env>(jsEnvWant->GetNativeEngine());

    info.argv[0] = AppExecFwk::WrapWant(envWant, want);
    EXPECT_NE(info.argv[0], NULL);
    info.argv[1] = AppExecFwk::WrapWant(envWant, want);
    EXPECT_NE(info.argv[1], NULL);
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest3 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    EXPECT_NE(jsUIExtensionContentSessionTest3, nullptr);
    jsUIExtensionContentSessionTest3->listener_ = nullptr;
    EXPECT_NE(jsUIExtensionContentSessionTest3->OnStartAbilityForResult(env, info), NULL);

    info.argc = 3;
    flags = 0;
    want.SetFlags(flags);
    std::shared_ptr<UISessionAbilityResultListener> listener = std::make_shared<UISessionAbilityResultListener>();
    EXPECT_NE(listener, nullptr);
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest4 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    jsUIExtensionContentSessionTest4->listener_ = listener;
    EXPECT_NE(jsUIExtensionContentSessionTest4, nullptr);
    info.argv[0] = AppExecFwk::WrapWant(envWant, want);
    EXPECT_NE(jsUIExtensionContentSessionTest4->OnStartAbilityForResult(env, info), NULL);
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnvWant->GetNativeEngine()));

    GTEST_LOG_(INFO) << "OnStartAbilityForResultTestTest_0200 end";
}
/**
 * @tc.number: OnStartAbilityForResultTestTest_0300
 * @tc.name: OnStartAbilityForResult test
 * @tc.desc: OnStartAbilityForResult test
 */
HWTEST_F(JsUIExtensionContentSessionTest, OnStartAbilityForResultTestTest_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnStartAbilityForResultTestTest_0300 start";

    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    sptr<Rosen::Window> uiWindow = nullptr;
    AbilityRuntime::Runtime::Options options;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_NE(jsEnv, nullptr);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    EXPECT_NE(env, nullptr);
    NapiCallbackInfo info;

    info.argc = 2;
    AAFwk::Want want;
    unsigned int flags = 2048;
    want.SetFlags(flags);
    AbilityRuntime::Runtime::Options optionsWant;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnvWant = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(optionsWant, jsEnvWant);
    ASSERT_NE(jsEnvWant, nullptr);
    EXPECT_NE(jsEnvWant->GetNativeEngine(), nullptr);
    napi_env envWant = reinterpret_cast<napi_env>(jsEnvWant->GetNativeEngine());

    info.argc = 3;
    info.argv[0] = AppExecFwk::WrapWant(envWant, want);
    EXPECT_NE(info.argv[0], NULL);
    info.argv[1] = AppExecFwk::WrapWant(envWant, want);
    EXPECT_NE(info.argv[1], NULL);

    flags = 0;
    want.SetFlags(flags);
    std::shared_ptr<UISessionAbilityResultListener> listener = std::make_shared<UISessionAbilityResultListener>();
    EXPECT_NE(listener, nullptr);
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest4 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    jsUIExtensionContentSessionTest4->listener_ = listener;
    EXPECT_NE(jsUIExtensionContentSessionTest4, nullptr);
    info.argv[0] = AppExecFwk::WrapWant(envWant, want);
    EXPECT_NE(jsUIExtensionContentSessionTest4->OnStartAbilityForResult(env, info), NULL);
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnvWant->GetNativeEngine()));

    GTEST_LOG_(INFO) << "OnStartAbilityForResultTestTest_0300 end";
}

/**
 * @tc.number: StartAbilityForResultRuntimeTaskTestTest_0100
 * @tc.name: StartAbilityForResultRuntimeTask test
 * @tc.desc: StartAbilityForResultRuntimeTask test
 */
HWTEST_F(JsUIExtensionContentSessionTest, StartAbilityForResultRuntimeTaskTestTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartAbilityForResultRuntimeTaskTestTest_0100 start";

    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    sptr<Rosen::Window> uiWindow = nullptr;
    AbilityRuntime::Runtime::Options options;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_NE(jsEnv, nullptr);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    EXPECT_NE(env, nullptr);
    NapiCallbackInfo info;
    size_t unwrapArgc = 0;

    info.argc = 0;
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest1 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest1, nullptr);

    AAFwk::Want want;
    unsigned int flags = 2048;
    want.SetFlags(flags);
    AAFwk::StartOptions startOptions;
    jsUIExtensionContentSessionTest1->StartAbilityForResultRuntimeTask(env, want, nullptr, unwrapArgc, startOptions);
    EXPECT_EQ(want.parameters_.params_.size(), 0);

    std::shared_ptr<Context> context = nullptr;
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest2 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest2, nullptr);
    jsUIExtensionContentSessionTest2->context_ = context;
    napi_deferred deferred;
    std::shared_ptr<NapiAsyncTask> asyncTask = std::make_shared<NapiAsyncTask>(deferred, nullptr, nullptr);
    EXPECT_NE(asyncTask, nullptr);
    jsUIExtensionContentSessionTest2->StartAbilityForResultRuntimeTask(env, want, asyncTask, unwrapArgc, startOptions);
    EXPECT_EQ(want.parameters_.params_.size(), 0);

    std::shared_ptr<UIExtensionContext> extensionContext = std::make_shared<UIExtensionContext>();
    EXPECT_NE(extensionContext, nullptr);
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest3 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest3, nullptr);
    jsUIExtensionContentSessionTest3->context_ = extensionContext;
    jsUIExtensionContentSessionTest3->listener_ = nullptr;
    jsUIExtensionContentSessionTest3->StartAbilityForResultRuntimeTask(env, want, asyncTask, unwrapArgc, startOptions);
    EXPECT_EQ(want.parameters_.params_.size(), 1);
    EXPECT_EQ(extensionContext->curRequestCode_, 1);
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));

    GTEST_LOG_(INFO) << "StartAbilityForResultRuntimeTaskTestTest_0100 end";
}

/**
 * @tc.number: StartAbilityForResultRuntimeTaskTestTest_0200
 * @tc.name: StartAbilityForResultRuntimeTask test
 * @tc.desc: StartAbilityForResultRuntimeTask test
 */
HWTEST_F(JsUIExtensionContentSessionTest, StartAbilityForResultRuntimeTaskTestTest_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartAbilityForResultRuntimeTaskTestTest_0200 start";

    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    sptr<Rosen::Window> uiWindow = nullptr;
    AbilityRuntime::Runtime::Options options;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_NE(jsEnv, nullptr);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    EXPECT_NE(env, nullptr);
    NapiCallbackInfo info;
    size_t unwrapArgc = 0;

    AAFwk::Want want;
    unsigned int flags = 2048;
    want.SetFlags(flags);
    AAFwk::StartOptions startOptions;
    napi_deferred deferred;
    std::shared_ptr<NapiAsyncTask> asyncTask = std::make_shared<NapiAsyncTask>(deferred, nullptr, nullptr);
    EXPECT_NE(asyncTask, nullptr);

    std::shared_ptr<UIExtensionContext> extensionContext = std::make_shared<UIExtensionContext>();
    EXPECT_NE(extensionContext, nullptr);
    extensionContext->curRequestCode_ = 0;
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest4 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest4, nullptr);

    jsUIExtensionContentSessionTest4->context_ = extensionContext;
    std::shared_ptr<UISessionAbilityResultListener> listener = std::make_shared<UISessionAbilityResultListener>();
    EXPECT_NE(listener, nullptr);
    jsUIExtensionContentSessionTest4->listener_ = listener;
    sptr<JsFreeInstallObserver> freeInstallObserver = new (std::nothrow) JsFreeInstallObserver(env);
    EXPECT_NE(freeInstallObserver, nullptr);
    jsUIExtensionContentSessionTest4->freeInstallObserver_ = freeInstallObserver;
    jsUIExtensionContentSessionTest4->StartAbilityForResultRuntimeTask(env, want, asyncTask, unwrapArgc, startOptions);
    EXPECT_EQ(want.parameters_.params_.size(), 2);
    EXPECT_EQ(extensionContext->curRequestCode_, 1);

    unwrapArgc = 1;
    extensionContext->curRequestCode_ = 0;
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest5 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest5, nullptr);
    jsUIExtensionContentSessionTest5->context_ = extensionContext;
    jsUIExtensionContentSessionTest5->listener_ = listener;
    jsUIExtensionContentSessionTest5->freeInstallObserver_ = freeInstallObserver;
    jsUIExtensionContentSessionTest5->StartAbilityForResultRuntimeTask(env, want, asyncTask, unwrapArgc, startOptions);
    EXPECT_EQ(extensionContext->curRequestCode_, 1);
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));

    GTEST_LOG_(INFO) << "StartAbilityForResultRuntimeTaskTestTest_0200 end";
}

/**
 * @tc.number: OnTerminateSelfTest_0100
 * @tc.name: OnTerminateSelf test
 * @tc.desc: OnTerminateSelf test
 */
HWTEST_F(JsUIExtensionContentSessionTest, OnTerminateSelfTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnTerminateSelfTest_0100 start";

    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    sptr<Rosen::Window> uiWindow = nullptr;
    AbilityRuntime::Runtime::Options options;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_NE(jsEnv, nullptr);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    EXPECT_NE(env, nullptr);
    NapiCallbackInfo info;
    info.argc = 0;

    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest1 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest1, nullptr);
    EXPECT_NE(jsUIExtensionContentSessionTest1->OnTerminateSelf(env, info), NULL);

    info.argc = 1;
    sessionInfo = new (std::nothrow) AAFwk::SessionInfo();
    EXPECT_NE(sessionInfo, nullptr);
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest2 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    EXPECT_NE(jsUIExtensionContentSessionTest2->OnTerminateSelf(env, info), NULL);
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));

    GTEST_LOG_(INFO) << "OnTerminateSelfTest_0100 end";
}

/**
 * @tc.number: OnTerminateSelfWithResultTest_0100
 * @tc.name: OnTerminateSelfWithResult test
 * @tc.desc: OnTerminateSelfWithResult test
 */
HWTEST_F(JsUIExtensionContentSessionTest, OnTerminateSelfWithResultTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnTerminateSelfWithResultTest_0100 end";

    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    sptr<Rosen::Window> uiWindow = nullptr;
    AbilityRuntime::Runtime::Options options;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_NE(jsEnv, nullptr);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    EXPECT_NE(env, nullptr);
    NapiCallbackInfo info;
    AAFwk::Want want;
    info.argc = 0;

    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest1 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest1, nullptr);
    EXPECT_NE(jsUIExtensionContentSessionTest1->OnTerminateSelfWithResult(env, info), NULL);

    info.argc = 1;
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest2 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest2, nullptr);
    EXPECT_NE(jsUIExtensionContentSessionTest2->OnTerminateSelfWithResult(env, info), NULL);

    AbilityRuntime::Runtime::Options optionsWant;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnvWant = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(optionsWant, jsEnvWant);
    ASSERT_NE(jsEnvWant, nullptr);
    EXPECT_NE(jsEnvWant->GetNativeEngine(), nullptr);
    napi_env envWant = reinterpret_cast<napi_env>(jsEnvWant->GetNativeEngine());
    info.argv[0] = AppExecFwk::WrapWant(envWant, want);
    EXPECT_NE(info.argv[0], NULL);
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest3 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest3, nullptr);
    EXPECT_NE(jsUIExtensionContentSessionTest3->OnTerminateSelfWithResult(env, info), NULL);

    info.argc = 2;
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest4 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest4, nullptr);
    EXPECT_NE(jsUIExtensionContentSessionTest4->OnTerminateSelfWithResult(env, info), NULL);
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnvWant->GetNativeEngine()));

    GTEST_LOG_(INFO) << "OnTerminateSelfWithResultTest_0100 end";
}

/**
 * @tc.number: OnSendDataTest_0100
 * @tc.name: OnSendData test
 * @tc.desc: OnSendData test
 */
HWTEST_F(JsUIExtensionContentSessionTest, OnSendDataTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnSendDataTest_0100 start";

    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    sptr<Rosen::Window> uiWindow = nullptr;
    AbilityRuntime::Runtime::Options options;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_NE(jsEnv, nullptr);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    EXPECT_NE(env, nullptr);
    NapiCallbackInfo info;
    info.argc = 0;

    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest1 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest1, nullptr);
    EXPECT_NE(jsUIExtensionContentSessionTest1->OnSendData(env, info), NULL);

    info.argc = 1;
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest2 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    EXPECT_NE(jsUIExtensionContentSessionTest2->OnSendData(env, info), NULL);

    AAFwk::WantParams wantParams;
    AbilityRuntime::Runtime::Options optionsWantParams;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnvWantParams = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(optionsWantParams, jsEnvWantParams);
    ASSERT_NE(jsEnvWantParams, nullptr);
    EXPECT_NE(jsEnvWantParams->GetNativeEngine(), nullptr);
    napi_env envWantParams = reinterpret_cast<napi_env>(jsEnvWantParams->GetNativeEngine());
    napi_value jsWantParams = OHOS::AppExecFwk::WrapWantParams(envWantParams, wantParams);
    info.argv[0] = jsWantParams;

    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest3 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest3, nullptr);
    EXPECT_NE(jsUIExtensionContentSessionTest3->OnSendData(env, info), NULL);

    uiWindow = new Rosen::Window();
    EXPECT_NE(uiWindow, nullptr);
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest4 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest4, nullptr);
    EXPECT_NE(jsUIExtensionContentSessionTest4->OnSendData(env, info), NULL);
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnvWantParams->GetNativeEngine()));

    GTEST_LOG_(INFO) << "OnSendDataTest_0100 end";
}

/**
 * @tc.number: OnSetReceiveDataCallbackTest_0100
 * @tc.name: OnSetReceiveDataCallback test
 * @tc.desc: OnSetReceiveDataCallback test
 */
HWTEST_F(JsUIExtensionContentSessionTest, OnSetReceiveDataCallbackTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnSetReceiveDataCallbackTest_0100 start";
    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    sptr<Rosen::Window> uiWindow = nullptr;
    AbilityRuntime::Runtime::Options options;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_NE(jsEnv, nullptr);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    EXPECT_NE(env, nullptr);
    NapiCallbackInfo info;
    info.argc = 0;

    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest1 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest1, nullptr);
    EXPECT_NE(jsUIExtensionContentSessionTest1->OnSetReceiveDataCallback(env, info), NULL);

    info.argc = 2;
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest2 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    EXPECT_NE(jsUIExtensionContentSessionTest2->OnSetReceiveDataCallback(env, info), NULL);

    AbilityRuntime::Runtime::Options optionsFunction;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnvFunction = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(optionsFunction, jsEnvFunction);
    ASSERT_NE(jsEnvFunction, nullptr);
    EXPECT_NE(jsEnvFunction->GetNativeEngine(), nullptr);
    napi_env envFunction = reinterpret_cast<napi_env>(jsEnvFunction->GetNativeEngine());
    auto callback = [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; };
    napi_value myFunction;
    napi_create_function(envFunction, nullptr, 1, callback, nullptr, &myFunction);
    info.argv[0] = myFunction;

    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest3 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest3, nullptr);
    jsUIExtensionContentSessionTest3->isRegistered = true;
    EXPECT_NE(jsUIExtensionContentSessionTest3->OnSetReceiveDataCallback(env, info), NULL);

    uiWindow = new Rosen::Window();
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest4 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest4, nullptr);
    jsUIExtensionContentSessionTest4->isRegistered = true;
    EXPECT_NE(jsUIExtensionContentSessionTest4->OnSetReceiveDataCallback(env, info), NULL);
    EXPECT_TRUE(jsUIExtensionContentSessionTest4->isRegistered);
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnvFunction->GetNativeEngine()));

    GTEST_LOG_(INFO) << "OnSetReceiveDataCallbackTest_0100 end";
}

/**
 * @tc.number: OnSetReceiveDataForResultCallbackTest_0100
 * @tc.name: OnSetReceiveDataForResultCallback test
 * @tc.desc: OnSetReceiveDataForResultCallback test
 */
HWTEST_F(JsUIExtensionContentSessionTest, OnSetReceiveDataForResultCallbackTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnSetReceiveDataForResultCallbackTest_0100 start";

    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    sptr<Rosen::Window> uiWindow = nullptr;
    AbilityRuntime::Runtime::Options options;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_NE(jsEnv, nullptr);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    EXPECT_NE(env, nullptr);
    NapiCallbackInfo info;
    info.argc = 0;

    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest1 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest1, nullptr);
    EXPECT_NE(jsUIExtensionContentSessionTest1->OnSetReceiveDataForResultCallback(env, info), NULL);

    info.argc = 2;
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest2 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    EXPECT_NE(jsUIExtensionContentSessionTest2->OnSetReceiveDataForResultCallback(env, info), NULL);

    auto callback = [](napi_env env, napi_callback_info object) -> napi_value { return nullptr; };
    napi_value myFunction;
    napi_create_function(env, nullptr, 1, callback, nullptr, &myFunction);
    info.argv[0] = myFunction;

    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest3 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest3, nullptr);
    jsUIExtensionContentSessionTest3->isRegistered = true;
    EXPECT_NE(jsUIExtensionContentSessionTest3->OnSetReceiveDataForResultCallback(env, info), NULL);

    uiWindow = new Rosen::Window();
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest4 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest4, nullptr);
    jsUIExtensionContentSessionTest4->isRegistered = true;
    EXPECT_NE(jsUIExtensionContentSessionTest4->OnSetReceiveDataForResultCallback(env, info), NULL);
    EXPECT_TRUE(jsUIExtensionContentSessionTest4->isRegistered);
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));

    GTEST_LOG_(INFO) << "OnSetReceiveDataForResultCallbackTest_0100 end";
}

/**
 * @tc.number: OnLoadContentTest_0100
 * @tc.name: OnLoadContent test
 * @tc.desc: OnLoadContent test
 */
HWTEST_F(JsUIExtensionContentSessionTest, OnLoadContentTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnLoadContentTest_0100 start";

    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    sptr<Rosen::Window> uiWindow = nullptr;
    AbilityRuntime::Runtime::Options options;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_NE(jsEnv, nullptr);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    EXPECT_NE(env, nullptr);
    NapiCallbackInfo info;
    info.argc = 0;

    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest1 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest1, nullptr);
    EXPECT_NE(jsUIExtensionContentSessionTest1->OnLoadContent(env, info), NULL);

    info.argc = 2;
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest2 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest2, nullptr);
    EXPECT_NE(jsUIExtensionContentSessionTest2->OnLoadContent(env, info), NULL);

    AbilityRuntime::Runtime::Options optionsObject;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnvObject = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(optionsObject, jsEnvObject);
    ASSERT_NE(jsEnvObject, nullptr);
    EXPECT_NE(jsEnvObject->GetNativeEngine(), nullptr);
    napi_env jsObject = reinterpret_cast<napi_env>(jsEnvObject->GetNativeEngine());
    std::string contextPath = "pages/Extension";
    info.argv[0] = AppExecFwk::WrapStringToJS(jsObject, contextPath);
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest3 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    EXPECT_NE(jsUIExtensionContentSessionTest3->OnLoadContent(env, info), NULL);
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnvObject->GetNativeEngine()));

    GTEST_LOG_(INFO) << "OnLoadContentTest_0100 end";
}

/**
 * @tc.number: OnLoadContentTest_0200
 * @tc.name: OnLoadContent test
 * @tc.desc: OnLoadContent test
 */
HWTEST_F(JsUIExtensionContentSessionTest, OnLoadContentTest_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnLoadContentTest_0200 start";

    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    sptr<Rosen::Window> uiWindow = nullptr;
    AbilityRuntime::Runtime::Options options;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_NE(jsEnv, nullptr);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    EXPECT_NE(env, nullptr);
    NapiCallbackInfo info;

    info.argc = 2;
    AbilityRuntime::Runtime::Options optionsObject;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnvObject = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(optionsObject, jsEnvObject);
    ASSERT_NE(jsEnvObject, nullptr);
    EXPECT_NE(jsEnvObject->GetNativeEngine(), nullptr);
    napi_env jsObject = reinterpret_cast<napi_env>(jsEnvObject->GetNativeEngine());
    std::string contextPath = "pages/Extension";
    info.argv[0] = AppExecFwk::WrapStringToJS(jsObject, contextPath);
    info.argv[1] = AppExecFwk::CreateJSObject(jsObject);
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest4 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest4, nullptr);
    EXPECT_NE(jsUIExtensionContentSessionTest4->OnLoadContent(env, info), NULL);

    uiWindow = new Rosen::Window();
    EXPECT_NE(uiWindow, nullptr);
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest5 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest5, nullptr);
    EXPECT_NE(jsUIExtensionContentSessionTest5->OnLoadContent(env, info), NULL);

    sessionInfo = new (std::nothrow) AAFwk::SessionInfo();
    EXPECT_NE(sessionInfo, nullptr);
    sessionInfo->isAsyncModalBinding = true;
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest6 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest6, nullptr);
    jsUIExtensionContentSessionTest6->isFirstTriggerBindModal_ = true;
    EXPECT_NE(jsUIExtensionContentSessionTest6->OnLoadContent(env, info), NULL);
    EXPECT_FALSE(jsUIExtensionContentSessionTest6->isFirstTriggerBindModal_);
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnvObject->GetNativeEngine()));

    GTEST_LOG_(INFO) << "OnLoadContentTest_0200 end";
}

/**
 * @tc.number: OnLoadContentByNameTest_0100
 * @tc.name: OnLoadContentByName test
 * @tc.desc: OnLoadContentByName test
 */
HWTEST_F(JsUIExtensionContentSessionTest, OnLoadContentByNameTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnLoadContentByNameTest_0100 start";

    // construct sessionInfo and uiwindow
    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    sptr<Rosen::Window> uiWindow = nullptr;
    AbilityRuntime::Runtime::Options options;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_NE(jsEnv, nullptr);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    EXPECT_NE(env, nullptr);

    // case1: input param invalid
    NapiCallbackInfo info;
    info.argc = 0;
    auto contentSession1 = std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(contentSession1, nullptr);
    EXPECT_NE(contentSession1->OnLoadContentByName(env, info), nullptr);

    // case2: has one param, but the first not a string
    info.argc = 1;
    auto contentSession2 = std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(contentSession2, nullptr);
    EXPECT_NE(contentSession2->OnLoadContentByName(env, info), nullptr);

    // case3: has one param, but the sessioninfo and window is invalid
    std::string name = "ExtensionName";
    info.argv[0] = AppExecFwk::WrapStringToJS(env, name);
    auto contentSession3 = std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(contentSession3, nullptr);
    EXPECT_NE(contentSession3->OnLoadContentByName(env, info), nullptr);

    // case4: has two param, but the second not a object
    info.argc = 2;
    auto contentSession4 = std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(contentSession4, nullptr);
    EXPECT_NE(contentSession4->OnLoadContentByName(env, info), nullptr);

    // after testcase finished
    JsRuntimeLite::GetInstance().RemoveJsEnv(env);

    GTEST_LOG_(INFO) << "OnLoadContentByNameTest_0100 end";
}

/**
 * @tc.number: OnLoadContentByNameTest_0200
 * @tc.name: OnLoadContentByName test
 * @tc.desc: OnLoadContentByName test
 */
HWTEST_F(JsUIExtensionContentSessionTest, OnLoadContentByNameTest_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnLoadContentByNameTest_0100 start";

    // construct sessionInfo and uiwindow
    auto uiWindow = sptr<Rosen::Window>::MakeSptr();
    EXPECT_NE(uiWindow, nullptr);
    auto sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    EXPECT_NE(sessionInfo, nullptr);
    sessionInfo->isAsyncModalBinding = true;
    AbilityRuntime::Runtime::Options options;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_NE(jsEnv, nullptr);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    EXPECT_NE(env, nullptr);

    NapiCallbackInfo info;
    std::string name = "ExtensionName";
    info.argc = 1;
    info.argv[0] = AppExecFwk::WrapStringToJS(env, name);
    auto contentSession = std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(contentSession, nullptr);
    EXPECT_NE(contentSession->OnLoadContentByName(env, info), nullptr);

    // after testcase finished
    JsRuntimeLite::GetInstance().RemoveJsEnv(env);

    GTEST_LOG_(INFO) << "OnLoadContentByNameTest_0200 end";
}

/**
 * @tc.number: OnSetWindowBackgroundColorTest_0100
 * @tc.name: OnSetWindowBackgroundColor test
 * @tc.desc: OnSetWindowBackgroundColor test
 */
HWTEST_F(JsUIExtensionContentSessionTest, OnSetWindowBackgroundColorTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnSetWindowBackgroundColorTest_0100 start";

    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    sptr<Rosen::Window> uiWindow = nullptr;
    AbilityRuntime::Runtime::Options options;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_NE(jsEnv, nullptr);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    EXPECT_NE(env, nullptr);
    NapiCallbackInfo info;
    info.argc = 0;

    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest1 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest1, nullptr);
    EXPECT_NE(jsUIExtensionContentSessionTest1->OnSetWindowBackgroundColor(env, info), NULL);

    info.argc = 2;
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest2 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    EXPECT_NE(jsUIExtensionContentSessionTest2->OnSetWindowBackgroundColor(env, info), NULL);

    AbilityRuntime::Runtime::Options optionsObject;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnvObject = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(optionsObject, jsEnvObject);
    ASSERT_NE(jsEnvObject, nullptr);
    EXPECT_NE(jsEnvObject->GetNativeEngine(), nullptr);
    napi_env jsObject = reinterpret_cast<napi_env>(jsEnvObject->GetNativeEngine());
    std::string color = "#00FF00";
    info.argv[0] = AppExecFwk::WrapStringToJS(jsObject, color);
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest3 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    EXPECT_NE(jsUIExtensionContentSessionTest3->OnSetWindowBackgroundColor(env, info), NULL);

    uiWindow = new Rosen::Window();
    EXPECT_NE(uiWindow, nullptr);
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest4 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest4, nullptr);
    EXPECT_NE(jsUIExtensionContentSessionTest4->OnSetWindowBackgroundColor(env, info), NULL);
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnvObject->GetNativeEngine()));
    GTEST_LOG_(INFO) << "OnSetWindowBackgroundColorTest_0100 end";
}

/**
 * @tc.number: OnSetWindowPrivacyModeTest_0100
 * @tc.name: OnSetWindowPrivacyMode test
 * @tc.desc: OnSetWindowPrivacyMode test
 */
HWTEST_F(JsUIExtensionContentSessionTest, OnSetWindowPrivacyModeTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnSetWindowPrivacyModeTest_0100 start";

    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    sptr<Rosen::Window> uiWindow = nullptr;
    AbilityRuntime::Runtime::Options options;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_NE(jsEnv, nullptr);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    EXPECT_NE(env, nullptr);
    NapiCallbackInfo info;
    info.argc = 0;

    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest1 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest1, nullptr);
    EXPECT_NE(jsUIExtensionContentSessionTest1->OnSetWindowPrivacyMode(env, info), NULL);

    info.argc = 2;
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest2 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest2, nullptr);
    EXPECT_NE(jsUIExtensionContentSessionTest2->OnSetWindowPrivacyMode(env, info), NULL);

    AbilityRuntime::Runtime::Options optionsBool;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnvTest = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(optionsBool, jsEnvTest);
    ASSERT_NE(jsEnvTest, nullptr);
    EXPECT_NE(jsEnvTest->GetNativeEngine(), nullptr);
    napi_env jsBool = reinterpret_cast<napi_env>(jsEnvTest->GetNativeEngine());
    bool isPrivacyMode = true;
    info.argv[0] = AppExecFwk::WrapBoolToJS(jsBool, isPrivacyMode);
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest3 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    EXPECT_NE(jsUIExtensionContentSessionTest3->OnSetWindowPrivacyMode(env, info), NULL);
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnvTest->GetNativeEngine()));
    GTEST_LOG_(INFO) << "OnSetWindowPrivacyModeTest_0100 end";
}

/**
 * @tc.number: OnStartAbilityByTypeTest_0100
 * @tc.name: OnStartAbilityByType test
 * @tc.desc: OnStartAbilityByType test
 */
HWTEST_F(JsUIExtensionContentSessionTest, OnStartAbilityByTypeTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnStartAbilityByTypeTest_0100 start";

    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    sptr<Rosen::Window> uiWindow = nullptr;
    AbilityRuntime::Runtime::Options options;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_NE(jsEnv, nullptr);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    EXPECT_NE(env, nullptr);
    NapiCallbackInfo info;
    info.argc = 0;

    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest1 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest1, nullptr);
    EXPECT_NE(jsUIExtensionContentSessionTest1->OnStartAbilityByType(env, info), NULL);

    info.argc = 2;
    AbilityRuntime::Runtime::Options optionsParameter;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnvParameter = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(optionsParameter, jsEnvParameter);
    ASSERT_NE(jsEnvParameter, nullptr);
    EXPECT_NE(jsEnvParameter->GetNativeEngine(), nullptr);
    napi_env jsParameter = reinterpret_cast<napi_env>(jsEnvParameter->GetNativeEngine());
    std::string str = "OnStartAbilityByTypeTest";
    info.argv[0] = AppExecFwk::WrapStringToJS(jsParameter, str);
    AAFwk::WantParams wantParams;
    wantParams.SetParam("ability.want.params.uriPermissionFlag", nullptr);
    info.argv[1] = AppExecFwk::WrapWantParams(jsParameter, wantParams);
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest2 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest2, nullptr);
    EXPECT_NE(jsUIExtensionContentSessionTest2->OnStartAbilityByType(env, info), NULL);

    info.argc = 4;
    uiWindow = new Rosen::Window();
    EXPECT_NE(uiWindow, nullptr);
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest3 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest3, nullptr);
    EXPECT_NE(jsUIExtensionContentSessionTest3->OnStartAbilityByType(env, info), NULL);
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnvParameter->GetNativeEngine()));

    GTEST_LOG_(INFO) << "OnStartAbilityByTypeTest_0100 end";
}

/**
 * @tc.number: CheckStartAbilityByTypeParamTest_0100
 * @tc.name: CheckStartAbilityByTypeParam test
 * @tc.desc: CheckStartAbilityByTypeParam test
 */
HWTEST_F(JsUIExtensionContentSessionTest, CheckStartAbilityByTypeParamTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckStartAbilityByTypeParamTest_0100 start";

    AbilityRuntime::Runtime::Options options;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_NE(jsEnv, nullptr);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    EXPECT_NE(env, nullptr);

    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest =
        std::make_shared<JsUIExtensionContentSession>(nullptr, nullptr);
    ASSERT_NE(jsUIExtensionContentSessionTest, nullptr);
    NapiCallbackInfo info;
    std::string type = "";
    AAFwk::WantParams wantParams;
    bool res = jsUIExtensionContentSessionTest->CheckStartAbilityByTypeParam(env, info, type, wantParams);
    EXPECT_EQ(res, false);

    info.argc = 3;
    res = jsUIExtensionContentSessionTest->CheckStartAbilityByTypeParam(env, info, type, wantParams);
    EXPECT_EQ(res, false);

    info.argv[0] = AppExecFwk::WrapStringToJS(env, "OnStartAbilityByTypeTest");
    res = jsUIExtensionContentSessionTest->CheckStartAbilityByTypeParam(env, info, type, wantParams);
    EXPECT_EQ(res, false);
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));

    GTEST_LOG_(INFO) << "CheckStartAbilityByTypeParamTest_0100 end";
}

/**
 * @tc.number: CreateJsUIExtensionContentSessionTest_0100
 * @tc.name: CreateJsUIExtensionContentSession test
 * @tc.desc: CreateJsUIExtensionContentSession test
 */
HWTEST_F(JsUIExtensionContentSessionTest, CreateJsUIExtensionContentSessionTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckStartAbilityByTypeParamTest_0100 start";
    AbilityRuntime::Runtime::Options options;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_NE(jsEnv, nullptr);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    EXPECT_NE(env, nullptr);

    std::shared_ptr<AbilityRuntime::ContextImpl> context = std::make_shared<AbilityRuntime::ContextImpl>();
    ASSERT_NE(context, nullptr);
    std::shared_ptr<AbilityResultListeners> abilityResultListeners = std::make_shared<AbilityResultListeners>();
    ASSERT_NE(abilityResultListeners, nullptr);
    auto res = JsUIExtensionContentSession::CreateJsUIExtensionContentSession(
        env, nullptr, nullptr, context, abilityResultListeners);
    EXPECT_NE(res, nullptr);

    res = JsUIExtensionContentSession::CreateJsUIExtensionContentSession(env, nullptr, nullptr);
    EXPECT_NE(res, nullptr);
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));

    GTEST_LOG_(INFO) << "CheckStartAbilityByTypeParamTest_0100 end";
}

/**
 * @tc.number: AddFreeInstallObserverTest_0100
 * @tc.name: AddFreeInstallObserver test
 * @tc.desc: AddFreeInstallObserver test
 */
HWTEST_F(JsUIExtensionContentSessionTest, AddFreeInstallObserverTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddFreeInstallObserverTest_0100 start";

    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    sptr<Rosen::Window> uiWindow = nullptr;
    OHOS::AbilityRuntime::Runtime::Options options;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_NE(jsEnv, nullptr);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    EXPECT_NE(env, nullptr);

    OHOS::AbilityRuntime::Runtime::Options optionsParameter;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnvParameter = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(optionsParameter, jsEnvParameter);
    ASSERT_NE(jsEnvParameter, nullptr);
    EXPECT_NE(jsEnvParameter->GetNativeEngine(), nullptr);
    napi_env envParameter = reinterpret_cast<napi_env>(jsEnvParameter->GetNativeEngine());

    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest1 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest1, nullptr);
    AAFwk::Want want;
    auto callback = [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; };
    napi_value myFunction;
    napi_create_function(envParameter, nullptr, 1, callback, nullptr, &myFunction);
    jsUIExtensionContentSessionTest1->AddFreeInstallObserver(env, want, myFunction, nullptr, false);
    EXPECT_NE(jsUIExtensionContentSessionTest1->freeInstallObserver_, nullptr);

    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest2 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest2, nullptr);
    std::shared_ptr<ContextImpl> context = std::make_shared<ContextImpl>();
    jsUIExtensionContentSessionTest2->context_ = context;
    jsUIExtensionContentSessionTest2->AddFreeInstallObserver(env, want, myFunction, nullptr, false);
    EXPECT_NE(jsUIExtensionContentSessionTest2->freeInstallObserver_, nullptr);
    EXPECT_EQ(jsUIExtensionContentSessionTest2->freeInstallObserver_->jsObserverObjectList_.size(), 0);

    sptr<JsFreeInstallObserver> freeInstallObserver = new (std::nothrow) JsFreeInstallObserver(envParameter);
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest3 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest3, nullptr);
    jsUIExtensionContentSessionTest3->context_ = context;
    jsUIExtensionContentSessionTest3->freeInstallObserver_ = freeInstallObserver;
    jsUIExtensionContentSessionTest3->AddFreeInstallObserver(env, want, myFunction, nullptr, false);
    EXPECT_EQ(jsUIExtensionContentSessionTest3->freeInstallObserver_->jsObserverObjectList_.size(), 1);
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnvParameter->GetNativeEngine()));

    GTEST_LOG_(INFO) << "AddFreeInstallObserverTest_0100 end";
}

/**
 * @tc.number: SetCallbackForTerminateWithResultTest_0100
 * @tc.name: SetCallbackForTerminateWithResult test
 * @tc.desc: SetCallbackForTerminateWithResult test
 */
HWTEST_F(JsUIExtensionContentSessionTest, SetCallbackForTerminateWithResultTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetCallbackForTerminateWithResultTest_0100 start";

    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    sptr<Rosen::Window> uiWindow = nullptr;
    OHOS::AbilityRuntime::Runtime::Options options;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_NE(jsEnv, nullptr);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    EXPECT_NE(env, nullptr);

    napi_ref callbackRef = nullptr;
    NapiAsyncTask task(callbackRef, nullptr, nullptr);
    int32_t status = 0;
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest1 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest1, nullptr);
    AAFwk::Want want;
    int32_t resultCode = 1;
    NapiAsyncTask::CompleteCallback completeTest1 = nullptr;
    jsUIExtensionContentSessionTest1->SetCallbackForTerminateWithResult(resultCode, want, completeTest1);
    EXPECT_NE(completeTest1, nullptr);
    completeTest1(env, task, status);

    uiWindow = new Rosen::Window();
    EXPECT_NE(uiWindow, nullptr);
    NapiAsyncTask::CompleteCallback completeTest2 = nullptr;
    std::shared_ptr<JsUIExtensionContentSession> jsUIExtensionContentSessionTest2 =
        std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(jsUIExtensionContentSessionTest2, nullptr);
    std::shared_ptr<ContextImpl> context = std::make_shared<ContextImpl>();
    jsUIExtensionContentSessionTest2->context_ = context;
    jsUIExtensionContentSessionTest2->SetCallbackForTerminateWithResult(resultCode, want, completeTest2);
    EXPECT_NE(completeTest2, nullptr);
    completeTest1(env, task, status);
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));

    GTEST_LOG_(INFO) << "SetCallbackForTerminateWithResultTest_0100 end";
}

/**
 * @tc.number: ProcessStartAbilityByTypeComplete_0100
 * @tc.name: ProcessStartAbilityByTypeComplete with null uiWindow
 * @tc.desc: ProcessStartAbilityByTypeComplete rejects when uiWindow is null
 */
HWTEST_F(JsUIExtensionContentSessionTest, ProcessStartAbilityByTypeComplete_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcessStartAbilityByTypeComplete_0100 start";

    AbilityRuntime::Runtime::Options options;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_NE(jsEnv, nullptr);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    EXPECT_NE(env, nullptr);

    bool callbackInvoked = false;
    napi_value callbackFunc = nullptr;
    napi_create_function(env, "completionCallback", NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            void* data = nullptr;
            napi_get_cb_info(env, info, nullptr, nullptr, nullptr, &data);
            if (data != nullptr) {
                *static_cast<bool*>(data) = true;
            }
            napi_value result = nullptr;
            napi_get_undefined(env, &result);
            return result;
        }, &callbackInvoked, &callbackFunc);
    EXPECT_NE(callbackFunc, nullptr);

    auto uiExtensionCallback = std::make_shared<JsUIExtensionCallback>(env);
    ASSERT_NE(uiExtensionCallback, nullptr);
    EXPECT_TRUE(uiExtensionCallback->SetCompletionCallback(env, callbackFunc));

    AAFwk::Want want;
    sptr<Rosen::Window> nullWindow = nullptr;
    JsUIExtensionContentSession::ProcessStartAbilityByTypeComplete(env, nullWindow, want, uiExtensionCallback);
    EXPECT_TRUE(callbackInvoked);
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));

    GTEST_LOG_(INFO) << "ProcessStartAbilityByTypeComplete_0100 end";
}

/**
 * @tc.number: DispatchStartAbilityByTypeResult_0100
 * @tc.name: DispatchStartAbilityByTypeResult with null lastParam
 * @tc.desc: DispatchStartAbilityByTypeResult returns a promise when lastParam is null
 */
HWTEST_F(JsUIExtensionContentSessionTest, DispatchStartAbilityByTypeResult_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DispatchStartAbilityByTypeResult_0100 start";

    AbilityRuntime::Runtime::Options options;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_NE(jsEnv, nullptr);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    EXPECT_NE(env, nullptr);

    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    sptr<Rosen::Window> uiWindow = nullptr;
    auto session = std::make_shared<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    ASSERT_NE(session, nullptr);

    napi_value lastParam = nullptr;
    HandleEscape handleEscape(env);
    auto uiExtensionCallback = std::make_shared<JsUIExtensionCallback>(env);
    NapiAsyncTask::CompleteCallback complete =
        [](napi_env env, NapiAsyncTask& task, int32_t status) {};
    auto result = session->DispatchStartAbilityByTypeResult(
        env, lastParam, handleEscape, uiExtensionCallback, complete);
    ASSERT_NE(result, nullptr);
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, result, &valueType);
    EXPECT_EQ(valueType, napi_object);
    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));

    GTEST_LOG_(INFO) << "DispatchStartAbilityByTypeResult_0100 end";
}
} // namespace AbilityRuntime
} // namespace OHOS
