/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "ability_context_impl.h"
#include "mock_context.h"
#include "mock_my_flag.h"
#include "ability_manager_errors.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AAFwk;
namespace {
const std::string TEST_BUNDLE_NAME = "com.test.bundle";
const std::string TEST_ABILITY_NAME = "TestAbility";
const std::string TEST_MODULE_NAME = "entry";
const int32_t TEST_REQUEST_CODE = 100;
const int32_t USER_CANCEL = -7;
}

class AbilityContextImplThirdTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
public:
    std::shared_ptr<AbilityContextImpl> context_ = nullptr;
    std::shared_ptr<MockContext> mock_ = nullptr;
};

void AbilityContextImplThirdTest::SetUpTestCase(void) {}

void AbilityContextImplThirdTest::TearDownTestCase(void) {}

void AbilityContextImplThirdTest::SetUp(void)
{
    context_ = std::make_shared<AbilityContextImpl>();
    mock_ = std::make_shared<MockContext>();
}

void AbilityContextImplThirdTest::TearDown(void) {}

/**
 * @tc.number: AbilityContextImplThirdTest_SetToken_0100
 * @tc.name: SetToken and GetToken
 * @tc.desc: Test SetToken and GetToken with valid token.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_SetToken_0100, Function | MediumTest | Level1)
{
    EXPECT_EQ(context_->GetToken(), nullptr);
    sptr<IRemoteObject> token = new (std::nothrow) IPCObjectStub();
    ASSERT_NE(token, nullptr);
    context_->SetToken(token);
    EXPECT_EQ(context_->GetToken(), token);
}

/**
 * @tc.number: AbilityContextImplThirdTest_SetToken_0200
 * @tc.name: SetToken and GetToken
 * @tc.desc: Test SetToken with nullptr.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_SetToken_0200, Function | MediumTest | Level1)
{
    sptr<IRemoteObject> token = new (std::nothrow) IPCObjectStub();
    context_->SetToken(token);
    EXPECT_EQ(context_->GetToken(), token);
    context_->SetToken(nullptr);
    EXPECT_EQ(context_->GetToken(), nullptr);
}

/**
 * @tc.number: AbilityContextImplThirdTest_SetAbilityRecordId_0100
 * @tc.name: SetAbilityRecordId and GetAbilityRecordId
 * @tc.desc: Test SetAbilityRecordId and GetAbilityRecordId with valid id.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_SetAbilityRecordId_0100,
    Function | MediumTest | Level1)
{
    EXPECT_EQ(context_->GetAbilityRecordId(), 0);
    int32_t recordId = 12345;
    context_->SetAbilityRecordId(recordId);
    EXPECT_EQ(context_->GetAbilityRecordId(), recordId);
}

/**
 * @tc.number: AbilityContextImplThirdTest_SetAbilityRecordId_0200
 * @tc.name: SetAbilityRecordId and GetAbilityRecordId
 * @tc.desc: Test SetAbilityRecordId with negative value.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_SetAbilityRecordId_0200,
    Function | MediumTest | Level1)
{
    int32_t recordId = -1;
    context_->SetAbilityRecordId(recordId);
    EXPECT_EQ(context_->GetAbilityRecordId(), recordId);
}

/**
 * @tc.number: AbilityContextImplThirdTest_RestoreWindowStage_0100
 * @tc.name: RestoreWindowStage(void*)
 * @tc.desc: Test RestoreWindowStage with void* contentStorage.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_RestoreWindowStage_0100,
    Function | MediumTest | Level1)
{
    int testData = 42;
    void *contentStorage = &testData;
    ErrCode ret = context_->RestoreWindowStage(contentStorage);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(context_->GetEtsContentStorage(), contentStorage);
}

/**
 * @tc.number: AbilityContextImplThirdTest_RestoreWindowStage_0200
 * @tc.name: RestoreWindowStage(void*)
 * @tc.desc: Test RestoreWindowStage with nullptr.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_RestoreWindowStage_0200,
    Function | MediumTest | Level1)
{
    ErrCode ret = context_->RestoreWindowStage(nullptr);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(context_->GetEtsContentStorage(), nullptr);
}

/**
 * @tc.number: AbilityContextImplThirdTest_RestoreWindowStage_0300
 * @tc.name: RestoreWindowStage(void*)
 * @tc.desc: Test RestoreWindowStage returns ERR_NOT_SUPPORTED when IsHook is true.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_RestoreWindowStage_0300,
    Function | MediumTest | Level1)
{
    context_->SetHook(true);
    int testData = 42;
    void *contentStorage = &testData;
    ErrCode ret = context_->RestoreWindowStage(contentStorage);
    EXPECT_EQ(ret, -2);
}

/**
 * @tc.number: AbilityContextImplThirdTest_StartSelf_0100
 * @tc.name: StartSelf
 * @tc.desc: Test StartSelf calls AbilityManagerClient, verifies context is not destroyed.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_StartSelf_0100, Function | MediumTest | Level1)
{
    sptr<IRemoteObject> token = new (std::nothrow) IPCObjectStub();
    context_->SetToken(token);
    ErrCode ret = context_->StartSelf();
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: AbilityContextImplThirdTest_SetHook_0100
 * @tc.name: SetHook and IsHook
 * @tc.desc: Test SetHook and IsHook default and modified values.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_SetHook_0100, Function | MediumTest | Level1)
{
    EXPECT_EQ(context_->IsHook(), false);
    context_->SetHook(true);
    EXPECT_EQ(context_->IsHook(), true);
    context_->SetHook(false);
    EXPECT_EQ(context_->IsHook(), false);
}

/**
 * @tc.number: AbilityContextImplThirdTest_SetHookOff_0100
 * @tc.name: SetHookOff and GetHookOff
 * @tc.desc: Test SetHookOff and GetHookOff default and modified values.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_SetHookOff_0100, Function | MediumTest | Level1)
{
    EXPECT_EQ(context_->GetHookOff(), false);
    context_->SetHookOff(true);
    EXPECT_EQ(context_->GetHookOff(), true);
    context_->SetHookOff(false);
    EXPECT_EQ(context_->GetHookOff(), false);
}

/**
 * @tc.number: AbilityContextImplThirdTest_IsTerminating_0100
 * @tc.name: IsTerminating and SetTerminating
 * @tc.desc: Test IsTerminating initial state and after setting.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_IsTerminating_0100,
    Function | MediumTest | Level1)
{
    EXPECT_EQ(context_->IsTerminating(), false);
    context_->SetTerminating(true);
    EXPECT_EQ(context_->IsTerminating(), true);
    context_->SetTerminating(false);
    EXPECT_EQ(context_->IsTerminating(), false);
}

/**
 * @tc.number: AbilityContextImplThirdTest_SetStageContext_0100
 * @tc.name: SetStageContext
 * @tc.desc: Test SetStageContext with valid context and verify getters work.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_SetStageContext_0100,
    Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    EXPECT_EQ(context_->GetBundleName(), "com.test.bundleName");
    EXPECT_NE(context_->GetApplicationInfo(), nullptr);
    EXPECT_EQ(context_->GetBundleCodePath(), "codePath");
}

/**
 * @tc.number: AbilityContextImplThirdTest_SetStageContext_0200
 * @tc.name: SetStageContext with nullptr
 * @tc.desc: Test SetStageContext with nullptr and verify getters return defaults.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_SetStageContext_0200,
    Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    EXPECT_EQ(context_->GetBundleName(), "");
    EXPECT_EQ(context_->GetApplicationInfo(), nullptr);
    EXPECT_EQ(context_->GetBundleCodePath(), "");
}

/**
 * @tc.number: AbilityContextImplThirdTest_SetAbilityInfo_0100
 * @tc.name: SetAbilityInfo and GetAbilityInfo
 * @tc.desc: Test SetAbilityInfo with valid info.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_SetAbilityInfo_0100,
    Function | MediumTest | Level1)
{
    EXPECT_EQ(context_->GetAbilityInfo(), nullptr);
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityInfo->name = TEST_ABILITY_NAME;
    abilityInfo->bundleName = TEST_BUNDLE_NAME;
    context_->SetAbilityInfo(abilityInfo);
    EXPECT_EQ(context_->GetAbilityInfo(), abilityInfo);
    EXPECT_EQ(context_->GetAbilityInfo()->name, TEST_ABILITY_NAME);
    EXPECT_EQ(context_->GetAbilityInfo()->bundleName, TEST_BUNDLE_NAME);
}

/**
 * @tc.number: AbilityContextImplThirdTest_SetConfiguration_0100
 * @tc.name: SetConfiguration and GetConfiguration
 * @tc.desc: Test SetConfiguration and GetConfiguration.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_SetConfiguration_0100,
    Function | MediumTest | Level1)
{
    EXPECT_EQ(context_->GetConfiguration(), nullptr);
    auto config = std::make_shared<AppExecFwk::Configuration>();
    config->AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, "zh_CN");
    context_->SetConfiguration(config);
    EXPECT_EQ(context_->GetConfiguration(), config);
}

/**
 * @tc.number: AbilityContextImplThirdTest_GetWant_0100
 * @tc.name: GetWant
 * @tc.desc: Test GetWant when abilityCallback_ is nullptr.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_GetWant_0100, Function | MediumTest | Level1)
{
    std::weak_ptr<AppExecFwk::IAbilityCallback> emptyCallback;
    context_->RegisterAbilityCallback(emptyCallback);
    auto want = context_->GetWant();
    EXPECT_EQ(want, nullptr);
}

/**
 * @tc.number: AbilityContextImplThirdTest_InsertRemoveResultCallbackTask_0100
 * @tc.name: InsertResultCallbackTask and RemoveResultCallbackTask
 * @tc.desc: Test insert and remove result callback task via OnAbilityResult behavior.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_InsertRemoveResultCallbackTask_0100,
    Function | MediumTest | Level1)
{
    // Test insert: verify callback is invoked by OnAbilityResult
    bool callbackInvoked = false;
    RuntimeTask task = [&callbackInvoked](int32_t code, const AAFwk::Want& want, bool isInner) {
        callbackInvoked = true;
    };
    context_->InsertResultCallbackTask(TEST_REQUEST_CODE, std::move(task));
    AAFwk::Want resultData;
    context_->OnAbilityResult(TEST_REQUEST_CODE, 0, resultData);
    EXPECT_EQ(callbackInvoked, true);

    // Test remove: verify removed callback is not invoked by OnAbilityResult
    bool callbackInvoked2 = false;
    RuntimeTask task2 = [&callbackInvoked2](int32_t code, const AAFwk::Want& want, bool isInner) {
        callbackInvoked2 = true;
    };
    int requestCode2 = TEST_REQUEST_CODE + 1;
    context_->InsertResultCallbackTask(requestCode2, std::move(task2));
    context_->RemoveResultCallbackTask(requestCode2);
    context_->OnAbilityResult(requestCode2, 0, resultData);
    EXPECT_EQ(callbackInvoked2, false);
}

/**
 * @tc.number: AbilityContextImplThirdTest_RemoveResultCallbackTask_0100
 * @tc.name: RemoveResultCallbackTask
 * @tc.desc: Test remove non-existent result callback task does not affect other callbacks.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_RemoveResultCallbackTask_0100,
    Function | MediumTest | Level1)
{
    bool callbackInvoked = false;
    RuntimeTask task = [&callbackInvoked](int32_t code, const AAFwk::Want& want, bool isInner) {
        callbackInvoked = true;
    };
    context_->InsertResultCallbackTask(TEST_REQUEST_CODE, std::move(task));
    context_->RemoveResultCallbackTask(999);
    AAFwk::Want resultData;
    context_->OnAbilityResult(TEST_REQUEST_CODE, 0, resultData);
    EXPECT_EQ(callbackInvoked, true);
}

/**
 * @tc.number: AbilityContextImplThirdTest_SetAbilityResourceManager_0100
 * @tc.name: SetAbilityResourceManager and GetResourceManager
 * @tc.desc: Test SetAbilityResourceManager overrides stageContext resource manager.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_SetAbilityResourceManager_0100,
    Function | MediumTest | Level1)
{
    EXPECT_EQ(context_->GetResourceManager(), nullptr);
    auto resourceMgr = std::shared_ptr<Global::Resource::ResourceManager>(Global::Resource::CreateResourceManager());
    ASSERT_NE(resourceMgr, nullptr);
    context_->SetAbilityResourceManager(resourceMgr);
    auto retrieved = context_->GetResourceManager();
    EXPECT_EQ(retrieved, resourceMgr);
}

/**
 * @tc.number: AbilityContextImplThirdTest_RegisterAbilityConfigUpdateCallback_0100
 * @tc.name: RegisterAbilityConfigUpdateCallback
 * @tc.desc: Test RegisterAbilityConfigUpdateCallback registers callback that is invoked.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_RegisterAbilityConfigUpdateCallback_0100,
    Function | MediumTest | Level1)
{
    bool called = false;
    auto callback = [&called](AppExecFwk::Configuration& config) {
        called = true;
    };
    context_->RegisterAbilityConfigUpdateCallback(callback);
    context_->SetAbilityColorMode(0);
    EXPECT_EQ(called, true);
}

/**
 * @tc.number: AbilityContextImplThirdTest_GetAbilityConfiguration_0100
 * @tc.name: GetAbilityConfiguration
 * @tc.desc: Test GetAbilityConfiguration default and after setting.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_GetAbilityConfiguration_0100,
    Function | MediumTest | Level1)
{
    EXPECT_EQ(context_->GetAbilityConfiguration(), nullptr);
    AppExecFwk::Configuration config;
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, "dark");
    context_->SetAbilityConfiguration(config);
    auto abilityConfig = context_->GetAbilityConfiguration();
    ASSERT_NE(abilityConfig, nullptr);
}

/**
 * @tc.number: AbilityContextImplThirdTest_SetAbilityColorMode_0100
 * @tc.name: SetAbilityColorMode
 * @tc.desc: Test SetAbilityColorMode with invalid color mode values does not invoke callback.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_SetAbilityColorMode_0100,
    Function | MediumTest | Level1)
{
    bool called = false;
    auto callback = [&called](AppExecFwk::Configuration& config) {
        called = true;
    };
    context_->RegisterAbilityConfigUpdateCallback(callback);
    context_->SetAbilityColorMode(-2);
    EXPECT_EQ(called, false);
    context_->SetAbilityColorMode(2);
    EXPECT_EQ(called, false);
}

/**
 * @tc.number: AbilityContextImplThirdTest_SetAbilityColorMode_0200
 * @tc.name: SetAbilityColorMode
 * @tc.desc: Test SetAbilityColorMode with valid color mode and callback.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_SetAbilityColorMode_0200,
    Function | MediumTest | Level1)
{
    bool called = false;
    auto callback = [&called](AppExecFwk::Configuration& config) {
        called = true;
    };
    context_->RegisterAbilityConfigUpdateCallback(callback);
    context_->SetAbilityColorMode(0);
    EXPECT_EQ(called, true);
}

/**
 * @tc.number: AbilityContextImplThirdTest_SetAbilityColorMode_0300
 * @tc.name: SetAbilityColorMode
 * @tc.desc: Test SetAbilityColorMode with empty callback does not crash.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_SetAbilityColorMode_0300,
    Function | MediumTest | Level1)
{
    context_->RegisterAbilityConfigUpdateCallback(AbilityConfigUpdateCallback());
    context_->SetAbilityColorMode(0);
    EXPECT_EQ(context_->GetAbilityConfiguration(), nullptr);
}

/**
 * @tc.number: AbilityContextImplThirdTest_AddCompletionHandler_0100
 * @tc.name: AddCompletionHandler
 * @tc.desc: Test AddCompletionHandler with duplicate requestId returns ERR_OK and does not add duplicate.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_AddCompletionHandler_0100,
    Function | MediumTest | Level1)
{
    int successCount = 0;
    std::string requestId = "dup_request_id";
    OnRequestResult onRequestSucc = [&successCount](const AppExecFwk::ElementName&, const std::string&) {
        successCount++;
    };
    OnRequestResult onRequestFail = [](const AppExecFwk::ElementName&, const std::string&) {};
    auto ret = context_->AddCompletionHandler(requestId, onRequestSucc, onRequestFail);
    EXPECT_EQ(ret, ERR_OK);
    ret = context_->AddCompletionHandler(requestId, onRequestSucc, onRequestFail);
    EXPECT_EQ(ret, ERR_OK);
    // Verify only one handler exists by calling OnRequestSuccess twice
    AppExecFwk::ElementName element;
    context_->OnRequestSuccess(requestId, element, "test");
    EXPECT_EQ(successCount, 1);
    context_->OnRequestSuccess(requestId, element, "test");
    EXPECT_EQ(successCount, 1);
}

/**
 * @tc.number: AbilityContextImplThirdTest_SetRestoreEnabled_0100
 * @tc.name: SetRestoreEnabled and GetRestoreEnabled
 * @tc.desc: Test SetRestoreEnabled and GetRestoreEnabled with normal context.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_SetRestoreEnabled_0100,
    Function | MediumTest | Level1)
{
    EXPECT_EQ(context_->GetRestoreEnabled(), false);
    context_->SetRestoreEnabled(true);
    EXPECT_EQ(context_->GetRestoreEnabled(), true);
    context_->SetRestoreEnabled(false);
    EXPECT_EQ(context_->GetRestoreEnabled(), false);
}

/**
 * @tc.number: AbilityContextImplThirdTest_SetRestoreEnabled_0200
 * @tc.name: SetRestoreEnabled when isHook
 * @tc.desc: Test SetRestoreEnabled is skipped when context is hook module.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_SetRestoreEnabled_0200,
    Function | MediumTest | Level1)
{
    context_->SetHook(true);
    context_->SetRestoreEnabled(false);
    context_->SetRestoreEnabled(true);
    EXPECT_EQ(context_->GetRestoreEnabled(), false);
}

/**
 * @tc.number: AbilityContextImplThirdTest_OnAbilityResult_0100
 * @tc.name: OnAbilityResult
 * @tc.desc: Test OnAbilityResult with registered callback, verifying isInner is false.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_OnAbilityResult_0100,
    Function | MediumTest | Level1)
{
    int requestCode = TEST_REQUEST_CODE;
    bool innerReceived = true;
    RuntimeTask task = [&innerReceived](int32_t code, const AAFwk::Want& want, bool isInner) {
        innerReceived = isInner;
    };
    context_->InsertResultCallbackTask(requestCode, std::move(task));
    AAFwk::Want resultData;
    context_->OnAbilityResult(requestCode, 0, resultData);
    EXPECT_EQ(innerReceived, false);
}

/**
 * @tc.number: AbilityContextImplThirdTest_OnAbilityResult_0200
 * @tc.name: OnAbilityResult
 * @tc.desc: Test OnAbilityResult with no registered callback does not crash.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_OnAbilityResult_0200,
    Function | MediumTest | Level1)
{
    int requestCode = TEST_REQUEST_CODE;
    AAFwk::Want resultData;
    context_->OnAbilityResult(requestCode, 0, resultData);
    // No callback registered, no crash expected
    EXPECT_EQ(context_->IsTerminating(), false);
}

/**
 * @tc.number: AbilityContextImplThirdTest_SetWeakSessionToken_0100
 * @tc.name: SetWeakSessionToken
 * @tc.desc: Test SetWeakSessionToken with valid token does not crash.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_SetWeakSessionToken_0100,
    Function | MediumTest | Level1)
{
    sptr<IRemoteObject> token = new (std::nothrow) IPCObjectStub();
    ASSERT_NE(token, nullptr);
    context_->SetWeakSessionToken(token);
    // SetWeakSessionToken stores weak pointer internally, no crash expected
    EXPECT_NE(token, nullptr);
}

/**
 * @tc.number: AbilityContextImplThirdTest_SetWeakSessionToken_0200
 * @tc.name: SetWeakSessionToken
 * @tc.desc: Test SetWeakSessionToken with nullptr does not crash.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_SetWeakSessionToken_0200,
    Function | MediumTest | Level1)
{
    context_->SetWeakSessionToken(nullptr);
    // SetWeakSessionToken with nullptr, no crash expected
    EXPECT_EQ(context_->GetToken(), nullptr);
}

/**
 * @tc.number: AbilityContextImplThirdTest_SetOnNewWantSkipScenarios_0100
 * @tc.name: SetOnNewWantSkipScenarios
 * @tc.desc: Test SetOnNewWantSkipScenarios calls AbilityManagerClient without crash and returns ErrCode.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_SetOnNewWantSkipScenarios_0100,
    Function | MediumTest | Level1)
{
    sptr<IRemoteObject> token = new (std::nothrow) IPCObjectStub();
    context_->SetToken(token);
    auto ret = context_->SetOnNewWantSkipScenarios(0);
    EXPECT_EQ(ret, ERR_OK);
    ret = context_->SetOnNewWantSkipScenarios(1);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: AbilityContextImplThirdTest_NotifyCancelGamePreLaunch_0100
 * @tc.name: NotifyCancelGamePreLaunch
 * @tc.desc: Test NotifyCancelGamePreLaunch calls AbilityManagerClient and returns ERR_OK.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_NotifyCancelGamePreLaunch_0100,
    Function | MediumTest | Level1)
{
    sptr<IRemoteObject> token = new (std::nothrow) IPCObjectStub();
    context_->SetToken(token);
    auto ret = context_->NotifyCancelGamePreLaunch();
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: AbilityContextImplThirdTest_NotifyCompleteGamePreLaunch_0100
 * @tc.name: NotifyCompleteGamePreLaunch
 * @tc.desc: Test NotifyCompleteGamePreLaunch calls AbilityManagerClient and returns ERR_OK.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_NotifyCompleteGamePreLaunch_0100,
    Function | MediumTest | Level1)
{
    sptr<IRemoteObject> token = new (std::nothrow) IPCObjectStub();
    context_->SetToken(token);
    auto ret = context_->NotifyCompleteGamePreLaunch();
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: AbilityContextImplThirdTest_CreateModuleContext_0100
 * @tc.name: CreateModuleContext
 * @tc.desc: Test CreateModuleContext with bundle name and module name when stageContext_ is null.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_CreateModuleContext_0100,
    Function | MediumTest | Level1)
{
    auto result = context_->CreateModuleContext(TEST_BUNDLE_NAME, TEST_MODULE_NAME);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.number: AbilityContextImplThirdTest_CreateModuleContext_0200
 * @tc.name: CreateModuleContext
 * @tc.desc: Test CreateModuleContext with only module name when stageContext_ is null.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_CreateModuleContext_0200,
    Function | MediumTest | Level1)
{
    auto result = context_->CreateModuleContext(TEST_MODULE_NAME);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.number: AbilityContextImplThirdTest_CreateBundleContext_0100
 * @tc.name: CreateBundleContext
 * @tc.desc: Test CreateBundleContext when stageContext_ is null.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_CreateBundleContext_0100,
    Function | MediumTest | Level1)
{
    auto result = context_->CreateBundleContext(TEST_BUNDLE_NAME);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.number: AbilityContextImplThirdTest_CreateModuleResourceManager_0100
 * @tc.name: CreateModuleResourceManager
 * @tc.desc: Test CreateModuleResourceManager when stageContext_ is null.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_CreateModuleResourceManager_0100,
    Function | MediumTest | Level1)
{
    auto result = context_->CreateModuleResourceManager(TEST_BUNDLE_NAME, TEST_MODULE_NAME);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.number: AbilityContextImplThirdTest_CreateSystemHspModuleResourceManager_0100
 * @tc.name: CreateSystemHspModuleResourceManager
 * @tc.desc: Test CreateSystemHspModuleResourceManager when stageContext_ is null.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_CreateSystemHspModuleResourceManager_0100,
    Function | MediumTest | Level1)
{
    std::shared_ptr<Global::Resource::ResourceManager> resourceMgr = nullptr;
    auto ret = context_->CreateSystemHspModuleResourceManager(TEST_BUNDLE_NAME, TEST_MODULE_NAME, resourceMgr);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.number: AbilityContextImplThirdTest_CreateAreaModeContext_0100
 * @tc.name: CreateAreaModeContext
 * @tc.desc: Test CreateAreaModeContext when stageContext_ is null.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_CreateAreaModeContext_0100,
    Function | MediumTest | Level1)
{
    auto result = context_->CreateAreaModeContext(0);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.number: AbilityContextImplThirdTest_GetArea_0100
 * @tc.name: GetArea
 * @tc.desc: Test GetArea when stageContext_ is null returns EL_DEFAULT.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_GetArea_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    int area = context_->GetArea();
    EXPECT_EQ(area, 1);
}

/**
 * @tc.number: AbilityContextImplThirdTest_GetArea_0200
 * @tc.name: GetArea
 * @tc.desc: Test GetArea when stageContext_ is set.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_GetArea_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    int mode = 2;
    context_->SwitchArea(mode);
    int area = context_->GetArea();
    EXPECT_EQ(area, mode);
}

/**
 * @tc.number: AbilityContextImplThirdTest_StartExtensionAbilityWithExtensionType_0100
 * @tc.name: StartExtensionAbilityWithExtensionType
 * @tc.desc: Test StartExtensionAbilityWithExtensionType with SERVICE type.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_StartExtensionAbilityWithExtensionType_0100,
    Function | MediumTest | Level1)
{
    AAFwk::Want want;
    want.SetElementName(TEST_BUNDLE_NAME, TEST_ABILITY_NAME);
    auto result = context_->StartExtensionAbilityWithExtensionType(
        want, AppExecFwk::ExtensionAbilityType::SERVICE);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.number: AbilityContextImplThirdTest_StopExtensionAbilityWithExtensionType_0100
 * @tc.name: StopExtensionAbilityWithExtensionType
 * @tc.desc: Test StopExtensionAbilityWithExtensionType with SERVICE type.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_StopExtensionAbilityWithExtensionType_0100,
    Function | MediumTest | Level1)
{
    AAFwk::Want want;
    want.SetElementName(TEST_BUNDLE_NAME, TEST_ABILITY_NAME);
    auto result = context_->StopExtensionAbilityWithExtensionType(
        want, AppExecFwk::ExtensionAbilityType::SERVICE);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.number: AbilityContextImplThirdTest_NotifyBindingObjectConfigUpdate_0100
 * @tc.name: NotifyBindingObjectConfigUpdate
 * @tc.desc: Test NotifyBindingObjectConfigUpdate when config is null, callback should not be invoked.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_NotifyBindingObjectConfigUpdate_0100,
    Function | MediumTest | Level1)
{
    bool callbackInvoked = false;
    context_->RegisterBindingObjectConfigUpdateCallback(
        [&callbackInvoked](std::shared_ptr<AppExecFwk::Configuration> config) {
            callbackInvoked = true;
        });
    // config is null by default (no SetConfiguration called)
    context_->NotifyBindingObjectConfigUpdate();
    EXPECT_EQ(callbackInvoked, false);
}

/**
 * @tc.number: AbilityContextImplThirdTest_NotifyBindingObjectConfigUpdate_0200
 * @tc.name: NotifyBindingObjectConfigUpdate
 * @tc.desc: Test NotifyBindingObjectConfigUpdate with config and valid callback, callback is invoked.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_NotifyBindingObjectConfigUpdate_0200,
    Function | MediumTest | Level1)
{
    bool callbackInvoked = false;
    std::shared_ptr<AppExecFwk::Configuration> receivedConfig;
    auto testConfig = std::make_shared<AppExecFwk::Configuration>();
    testConfig->AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, "en");
    context_->SetConfiguration(testConfig);
    context_->RegisterBindingObjectConfigUpdateCallback(
        [&callbackInvoked, &receivedConfig](std::shared_ptr<AppExecFwk::Configuration> config) {
            callbackInvoked = true;
            receivedConfig = config;
        });
    context_->NotifyBindingObjectConfigUpdate();
    EXPECT_EQ(callbackInvoked, true);
    EXPECT_EQ(receivedConfig, testConfig);
}

/**
 * @tc.number: AbilityContextImplThirdTest_StartSelfUIAbilityInCurrentProcess_0100
 * @tc.name: StartSelfUIAbilityInCurrentProcess
 * @tc.desc: Test StartSelfUIAbilityInCurrentProcess with hasOptions false.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_StartSelfUIAbilityInCurrentProcess_0100,
    Function | MediumTest | Level1)
{
    AAFwk::Want want;
    want.SetElementName(TEST_BUNDLE_NAME, TEST_ABILITY_NAME);
    std::string specifiedFlag = "testFlag";
    AAFwk::StartOptions startOptions;
    auto ret = context_->StartSelfUIAbilityInCurrentProcess(want, specifiedFlag, startOptions, false);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: AbilityContextImplThirdTest_StartSelfUIAbilityInCurrentProcess_0200
 * @tc.name: StartSelfUIAbilityInCurrentProcess
 * @tc.desc: Test StartSelfUIAbilityInCurrentProcess with hasOptions true.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_StartSelfUIAbilityInCurrentProcess_0200,
    Function | MediumTest | Level1)
{
    AAFwk::Want want;
    want.SetElementName(TEST_BUNDLE_NAME, TEST_ABILITY_NAME);
    std::string specifiedFlag;
    AAFwk::StartOptions startOptions;
    auto ret = context_->StartSelfUIAbilityInCurrentProcess(want, specifiedFlag, startOptions, true);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: AbilityContextImplThirdTest_GetProcessName_0100
 * @tc.name: GetProcessName
 * @tc.desc: Test GetProcessName with stageContext_ set returns correct value.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_GetProcessName_0100,
    Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    EXPECT_EQ(context_->GetProcessName(), "processName");
}

/**
 * @tc.number: AbilityContextImplThirdTest_GetProcessName_0200
 * @tc.name: GetProcessName
 * @tc.desc: Test GetProcessName with stageContext_ null returns empty string.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_GetProcessName_0200,
    Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    EXPECT_EQ(context_->GetProcessName(), "");
}

/**
 * @tc.number: AbilityContextImplThirdTest_GetHapModuleInfo_0100
 * @tc.name: GetHapModuleInfo
 * @tc.desc: Test GetHapModuleInfo returns nullptr when stageContext_ is null.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_GetHapModuleInfo_0100,
    Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->GetHapModuleInfo();
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.number: AbilityContextImplThirdTest_EraseUIExtension_0100
 * @tc.name: EraseUIExtension
 * @tc.desc: Test EraseUIExtension with non-existent sessionId does not crash.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_EraseUIExtension_0100,
    Function | MediumTest | Level1)
{
    context_->EraseUIExtension(999);
    context_->EraseUIExtension(1);
    AAFwk::Want want;
    EXPECT_EQ(context_->IsUIExtensionExist(want), false);
}

/**
 * @tc.number: AbilityContextImplThirdTest_IsUIExtensionExist_0100
 * @tc.name: IsUIExtensionExist
 * @tc.desc: Test IsUIExtensionExist with empty map returns false.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_IsUIExtensionExist_0100,
    Function | MediumTest | Level1)
{
    AAFwk::Want matchWant;
    matchWant.SetElementName(TEST_BUNDLE_NAME, TEST_MODULE_NAME, TEST_ABILITY_NAME);
    EXPECT_EQ(context_->IsUIExtensionExist(matchWant), false);
    AAFwk::Want noMatchWant;
    noMatchWant.SetElementName("other.bundle", "other.module", "OtherAbility");
    EXPECT_EQ(context_->IsUIExtensionExist(noMatchWant), false);
}

/**
 * @tc.number: AbilityContextImplThirdTest_OnRequestFailure_0100
 * @tc.name: OnRequestFailure with USER_CANCEL resultCode
 * @tc.desc: Test failure callback receives USER_CANCEL info when resultCode is USER_CANCEL.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_OnRequestFailure_0100,
    Function | MediumTest | Level1)
{
    int32_t failureCode = -1;
    std::string failureMessage;
    std::string requestId = "test_cancel";
    OnAtomicRequestSuccess onSuccess = [](const std::string&) {};
    OnAtomicRequestFailure onFailure =
        [&failureCode, &failureMessage](const std::string& appId, int32_t code, const std::string& msg) {
            failureCode = code;
            failureMessage = msg;
        };
    context_->AddCompletionHandlerForAtomicService(requestId, onSuccess, onFailure, "com.test");
    AppExecFwk::ElementName element;
    context_->OnRequestFailure(requestId, element, "test message", USER_CANCEL);
    EXPECT_EQ(failureCode, 1);
    EXPECT_EQ(failureMessage, "The user canceled this startup");
}

/**
 * @tc.number: AbilityContextImplThirdTest_OnRequestFailure_0200
 * @tc.name: OnRequestFailure with user refused message
 * @tc.desc: Test failure callback receives user refuse info when message contains refusal.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_OnRequestFailure_0200,
    Function | MediumTest | Level1)
{
    int32_t failureCode = -1;
    std::string failureMessage;
    std::string requestId = "test_refuse";
    OnAtomicRequestSuccess onSuccess = [](const std::string&) {};
    OnAtomicRequestFailure onFailure =
        [&failureCode, &failureMessage](const std::string& appId, int32_t code, const std::string& msg) {
            failureCode = code;
            failureMessage = msg;
        };
    context_->AddCompletionHandlerForAtomicService(requestId, onSuccess, onFailure, "com.test");
    AppExecFwk::ElementName element;
    context_->OnRequestFailure(requestId, element, "User refused redirection to app", 0);
    EXPECT_EQ(failureCode, 2);
    EXPECT_EQ(failureMessage, "User refused redirection");
}

/**
 * @tc.number: AbilityContextImplThirdTest_OnRequestFailure_0300
 * @tc.name: OnRequestFailure with generic error
 * @tc.desc: Test failure callback receives system error info for unknown error message.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_OnRequestFailure_0300,
    Function | MediumTest | Level1)
{
    int32_t failureCode = -1;
    std::string failureMessage;
    std::string requestId = "test_error";
    OnAtomicRequestSuccess onSuccess = [](const std::string&) {};
    OnAtomicRequestFailure onFailure =
        [&failureCode, &failureMessage](const std::string& appId, int32_t code, const std::string& msg) {
            failureCode = code;
            failureMessage = msg;
        };
    context_->AddCompletionHandlerForAtomicService(requestId, onSuccess, onFailure, "com.test");
    AppExecFwk::ElementName element;
    context_->OnRequestFailure(requestId, element, "Some unknown error", 0);
    EXPECT_EQ(failureCode, 0);
    EXPECT_EQ(failureMessage, "A system error occurred");
}
} // namespace AppExecFwk
} // namespace OHOS
