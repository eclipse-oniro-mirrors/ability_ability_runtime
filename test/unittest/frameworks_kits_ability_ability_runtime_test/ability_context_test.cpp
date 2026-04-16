/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "ability_context.h"

namespace OHOS {
namespace AbilityRuntime {
    const size_t Context::CONTEXT_TYPE_ID(std::hash<const char*> {} ("Context"));
}
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

class MockAbilityContext : public AbilityContext {
public:
    virtual void RegisterAbilityConfigUpdateCallback(AbilityConfigUpdateCallback abilityConfigUpdateCallback)
    {
        return;
    }
    virtual std::shared_ptr<AppExecFwk::Configuration> GetAbilityConfiguration() const
    {
        return nullptr;
    }
    virtual void SetAbilityConfiguration(const AppExecFwk::Configuration &config)
    {
        return;
    }
    virtual void SetAbilityColorMode(int32_t colorMode)
    {
        return;
    }
    virtual void SetAbilityResourceManager(std::shared_ptr<Global::Resource::ResourceManager> abilityResourceMgr)
    {
        return;
    }
    virtual ErrCode RevokeDelegator()
    {
        return ERR_OK;
    }
    virtual bool GetHookOff()
    {
        return false;
    }
    virtual void SetHookOff(bool hookOff)
    {
        return;
    }
    virtual bool IsHook()
    {
        return false;
    }
    virtual void SetHook(bool isHook)
    {
        return;
    }
    virtual std::string GetBundleName() const
    {
        return "MockAbilityContext for tdd";
    }
    virtual std::shared_ptr<Context> CreateBundleContext(const std::string &bundleName)
    {
        return nullptr;
    }
    virtual std::shared_ptr<AppExecFwk::ApplicationInfo> GetApplicationInfo() const
    {
        return nullptr;
    }
    virtual std::shared_ptr<Global::Resource::ResourceManager> GetResourceManager() const
    {
        return nullptr;
    }
    virtual std::string GetBundleCodePath() const
    {
        return "MockAbilityContext for tdd";
    }
    virtual std::shared_ptr<AppExecFwk::HapModuleInfo> GetHapModuleInfo() const
    {
        return nullptr;
    }
    virtual std::string GetBundleCodeDir()
    {
        return "MockAbilityContext for tdd";
    }
    virtual std::string GetCacheDir()
    {
        return "MockAbilityContext for tdd";
    }
    virtual std::string GetTempDir()
    {
        return "MockAbilityContext for tdd";
    }
    virtual std::string GetFilesDir()
    {
        return "MockAbilityContext for tdd";
    }
    virtual std::string GetResourceDir(const std::string &moduleName = "")
    {
        return "MockAbilityContext for tdd";
    }
    virtual bool IsUpdatingConfigurations()
    {
        return false;
    }
    virtual bool PrintDrawnCompleted()
    {
        return false;
    }
    virtual std::string GetDatabaseDir()
    {
        return "MockAbilityContext for tdd";
    }
    virtual int32_t GetSystemDatabaseDir(const std::string &groupId,
        bool checkExist, std::string &databaseDir)
    {
        return 0;
    }
    virtual std::string GetPreferencesDir()
    {
        return "MockAbilityContext for tdd";
    }
    virtual std::string GetGroupDir(std::string groupId)
    {
        return "MockAbilityContext for tdd";
    }
    virtual ErrCode StartAbilityWithAccount(const AAFwk::Want &want,
        int accountId, int requestCode)
    {
        return ERR_OK;
    }
    virtual ErrCode StartAbility(const AAFwk::Want &want,
        const AAFwk::StartOptions &startOptions, int requestCode)
    {
        return ERR_OK;
    }
    virtual ErrCode StartAbilityAsCaller(const AAFwk::Want &want, int requestCode)
    {
        return ERR_OK;
    }
    virtual ErrCode StartAbilityAsCaller(const AAFwk::Want &want,
        const AAFwk::StartOptions &startOptions, int requestCode)
    {
        return ERR_OK;
    }
    virtual ErrCode StartAbilityWithAccount(const AAFwk::Want &want,
        int accountId, const AAFwk::StartOptions &startOptions, int requestCode)
    {
        return ERR_OK;
    }
    virtual ErrCode StartAbilityForResult(const AAFwk::Want &Want, int requestCode, RuntimeTask &&task)
    {
        return ERR_OK;
    }
    virtual ErrCode StartAbilityForResultWithAccount(const AAFwk::Want &Want,
        int accountId, int requestCode, RuntimeTask &&task)
    {
        return ERR_OK;
    }
    virtual ErrCode StartAbilityForResult(const AAFwk::Want &Want,
        const AAFwk::StartOptions &startOptions, int requestCode, RuntimeTask &&task)
    {
        return ERR_OK;
    }
    virtual ErrCode StartAbilityForResultWithAccount(const AAFwk::Want &Want, int accountId,
        const AAFwk::StartOptions &startOptions, int requestCode, RuntimeTask &&task)
    {
        return ERR_OK;
    }
    virtual ErrCode StartServiceExtensionAbility(const AAFwk::Want &want, int32_t accountId = -1)
    {
        return ERR_OK;
    }
    virtual ErrCode StartUIServiceExtensionAbility(const AAFwk::Want &want, int32_t accountId = -1)
    {
        return ERR_OK;
    }
    virtual ErrCode StopServiceExtensionAbility(const AAFwk::Want& want, int32_t accountId = -1)
    {
        return ERR_OK;
    }
    virtual ErrCode TerminateAbilityWithResult(const AAFwk::Want &want, int resultCode)
    {
        return ERR_OK;
    }
    virtual ErrCode BackToCallerAbilityWithResult(const AAFwk::Want &want,
        int resultCode, int64_t requestCode)
    {
        return ERR_OK;
    }
    virtual ErrCode RestoreWindowStage(napi_env env, napi_value contentStorage)
    {
        return ERR_OK;
    }

    virtual ErrCode RestoreWindowStage(void *contentStorage)
    {
        return ERR_OK;
    }

    void *GetEtsContentStorage()
    {
        return nullptr;
    }

    virtual void OnAbilityResult(int requestCode, int resultCode, const AAFwk::Want &resultData)
    {
        return;
    }
    virtual std::string GetDistributedFilesDir()
    {
        return "MockAbilityContext for tdd";
    }
    virtual std::string GetCloudFileDir()
    {
        return "MockAbilityContext for tdd";
    }
    virtual std::string GetLogFileDir()
    {
        return "MockAbilityContext for tdd";
    }
    virtual sptr<IRemoteObject> GetToken()
    {
        return nullptr;
    }
    virtual void SetToken(const sptr<IRemoteObject> &token)
    {
        return;
    }
    virtual void SwitchArea(int mode)
    {
        return;
    }
    virtual std::shared_ptr<Context> CreateModuleContext(const std::string &moduleName)
    {
        return nullptr;
    }
    virtual std::shared_ptr<Context> CreateModuleContext(const std::string &bundleName,
        const std::string &moduleName)
    {
        return nullptr;
    }
    virtual std::shared_ptr<Global::Resource::ResourceManager> CreateModuleResourceManager(
        const std::string &bundleName, const std::string &moduleName)
    {
        return nullptr;
    }
    virtual int32_t CreateSystemHspModuleResourceManager(const std::string &bundleName,
        const std::string &moduleName, std::shared_ptr<Global::Resource::ResourceManager> &resourceManager)
    {
        return 0;
    }
    virtual Global::Resource::DeviceType GetDeviceType() const
    {
        return Global::Resource::DeviceType::DEVICE_PHONE;
    }
    virtual ErrCode StartAbility(const AAFwk::Want &want, int requestCode)
    {
        return ERR_OK;
    }
    virtual int GetArea()
    {
        return 0;
    }
    virtual std::string GetProcessName()
    {
        return "MockAbilityContext for tdd";
    }
    virtual std::shared_ptr<AppExecFwk::Configuration> GetConfiguration() const
    {
        return nullptr;
    }
    virtual std::string GetBaseDir() const
    {
        return "MockAbilityContext for tdd";
    }
    virtual std::shared_ptr<Context> CreateAreaModeContext(int areaMode)
    {
        return nullptr;
    }

    virtual std::shared_ptr<Context> CreateModuleOrPluginContext(const std::string &bundleName,
        const std::string &moduleName)
    {
        return nullptr;
    }
    virtual std::shared_ptr<Context> CreateDisplayContext(uint64_t displayId)
    {
        return nullptr;
    }
    virtual ErrCode RequestModalUIExtension(const AAFwk::Want& want)
    {
        return ERR_OK;
    }
    virtual ErrCode OpenLink(const AAFwk::Want& want, int requestCode, bool hideFailureTipDialog = false)
    {
        return ERR_OK;
    }
    virtual ErrCode OpenAtomicService(AAFwk::Want& want,
        const AAFwk::StartOptions &options, int requestCode, RuntimeTask &&task)
    {
        return ERR_OK;
    }
    virtual ErrCode AddFreeInstallObserver(const sptr<AbilityRuntime::IFreeInstallObserver> &observer)
    {
        return ERR_OK;
    }
    virtual ErrCode ConnectAbility(const AAFwk::Want &want, const sptr<AbilityConnectCallback> &connectCallback)
    {
        return ERR_OK;
    }
    virtual ErrCode ConnectAbilityWithAccount(const AAFwk::Want &want,
        int accountId, const sptr<AbilityConnectCallback> &connectCallback)
    {
        return ERR_OK;
    }
    virtual ErrCode ConnectUIServiceExtensionAbility(const AAFwk::Want& want,
        const sptr<AbilityConnectCallback>& connectCallback)
    {
        return ERR_OK;
    }
    virtual ErrCode StartExtensionAbilityWithExtensionType(const AAFwk::Want &want,
        AppExecFwk::ExtensionAbilityType extensionType)
    {
        return ERR_OK;
    }
    virtual ErrCode StopExtensionAbilityWithExtensionType(const AAFwk::Want& want,
        AppExecFwk::ExtensionAbilityType extensionType)
    {
        return ERR_OK;
    }
    virtual ErrCode ConnectExtensionAbilityWithExtensionType(const AAFwk::Want& want,
        const sptr<AbilityConnectCallback>& connectCallback, AppExecFwk::ExtensionAbilityType extensionType)
    {
        return ERR_OK;
    }
    virtual void DisconnectAbility(const AAFwk::Want &want,
        const sptr<AbilityConnectCallback> &connectCallback, int32_t accountId = -1)
    {
        return;
    }
    virtual std::shared_ptr<AppExecFwk::AbilityInfo> GetAbilityInfo() const
    {
        return nullptr;
    }
    virtual int32_t GetSystemPreferencesDir(const std::string &groupId,
        bool checkExist, std::string &preferencesDir)
    {
        return 0;
    }

    virtual void MinimizeAbility(bool fromUser = false)
    {
        return;
    }
    virtual ErrCode OnBackPressedCallBack(bool &needMoveToBackground)
    {
        return ERR_OK;
    }
    virtual ErrCode MoveAbilityToBackground()
    {
        return ERR_OK;
    }
    virtual ErrCode MoveUIAbilityToBackground()
    {
        return ERR_OK;
    }
    virtual ErrCode TerminateSelf()
    {
        return ERR_OK;
    }
    virtual ErrCode CloseAbility()
    {
        return ERR_OK;
    }
    virtual std::unique_ptr<NativeReference>& GetContentStorage()
    {
        return mockContentStorage_;
    }
    virtual ErrCode StartAbilityByCall(const AAFwk::Want& want,
        const std::shared_ptr<CallerCallBack> &callback, int32_t accountId = DEFAULT_INVAL_VALUE)
    {
        return ERR_OK;
    }
    virtual ErrCode StartAbilityForPrelaunch(const Want &want, const int32_t frameNum)
    {
        return ERR_OK;
    }
    virtual ErrCode ReleaseCall(const std::shared_ptr<CallerCallBack> &callback)
    {
        return ERR_OK;
    }
    virtual void ClearFailedCallConnection(const std::shared_ptr<CallerCallBack> &callback)
    {
        return;
    }
    virtual std::shared_ptr<LocalCallContainer> GetLocalCallContainer()
    {
        return nullptr;
    }
    virtual void SetConfiguration(const std::shared_ptr<AppExecFwk::Configuration> &config)
    {
        return;
    }
    virtual void RegisterAbilityCallback(std::weak_ptr<AppExecFwk::IAbilityCallback> abilityCallback)
    {
        return;
    }
    virtual void SetWeakSessionToken(const wptr<IRemoteObject>& sessionToken)
    {
        return;
    }
    virtual void SetAbilityRecordId(int32_t abilityRecordId)
    {
        return;
    }
    virtual int32_t GetAbilityRecordId()
    {
        return 0;
    }
    virtual ErrCode RequestDialogService(napi_env env,
        AAFwk::Want &want, RequestDialogResultTask &&task)
    {
        return ERR_OK;
    }
    virtual ErrCode RequestDialogService(AAFwk::Want &want, RequestDialogResultTask &&task)
    {
        return ERR_OK;
    }
    virtual ErrCode ReportDrawnCompleted()
    {
        return ERR_OK;
    }
    virtual ErrCode GetMissionId(int32_t &missionId)
    {
        return ERR_OK;
    }
    virtual ErrCode SetMissionContinueState(const AAFwk::ContinueState &state)
    {
        return ERR_OK;
    }
    virtual void RegisterAbilityLifecycleObserver(
        const std::shared_ptr<AppExecFwk::ILifecycleObserver> &observer)
    {
        return;
    }
    virtual void UnregisterAbilityLifecycleObserver(
        const std::shared_ptr<AppExecFwk::ILifecycleObserver> &observer)
    {
        return;
    }
    virtual void SetRestoreEnabled(bool enabled)
    {
        return;
    }
    virtual bool GetRestoreEnabled()
    {
        return false;
    }
    virtual std::shared_ptr<AAFwk::Want> GetWant()
    {
        return nullptr;
    }
#ifdef SUPPORT_GRAPHICS
#ifdef SUPPORT_SCREEN
    virtual ErrCode SetMissionLabel(const std::string &label)
    {
        return ERR_OK;
    }
    virtual ErrCode SetMissionIcon(const std::shared_ptr<OHOS::Media::PixelMap> &icon)
    {
        return ERR_OK;
    }
    virtual ErrCode SetMissionWindowIcon(std::shared_ptr<OHOS::Media::PixelMap> windowIcon)
    {
        return ERR_OK;
    }
    virtual ErrCode SetAbilityInstanceInfo(const std::string& label,
        std::shared_ptr<OHOS::Media::PixelMap> icon)
    {
        return ERR_OK;
    }
    virtual int GetCurrentWindowMode()
    {
        return 0;
    }
    virtual void GetWindowRect(int32_t &left,
        int32_t &top, int32_t &width, int32_t &height)
    {
        return;
    }
    virtual Ace::UIContent* GetUIContent()
    {
        return nullptr;
    }
    virtual ErrCode StartAbilityByType(const std::string &type, AAFwk::WantParams &wantParam,
        std::shared_ptr<UIExtensionCallback> uiExtensionCallback)
    {
        return ERR_OK;
    }
    virtual ErrCode CreateModalUIExtensionWithApp(const AAFwk::Want &want)
    {
        return ERR_OK;
    }
    virtual void EraseUIExtension(int32_t sessionId)
    {
        return;
    }
#endif
#endif
    virtual bool IsTerminating()
    {
        return false;
    }
    virtual void SetTerminating(bool state)
    {
        return;
    }
    virtual void InsertResultCallbackTask(int requestCode, RuntimeTask&& task)
    {
        return;
    }
    virtual void RemoveResultCallbackTask(int requestCode)
    {
        return;
    }
    virtual ErrCode AddCompletionHandlerForAtomicService(const std::string &requestId,
        OnAtomicRequestSuccess onRequestSucc, OnAtomicRequestFailure onRequestFail, const std::string &appId)
    {
        return ERR_OK;
    }

    /**
     * @brief Add CompletioHandler.
     *
     * @param requestId, the requestId.
     * @param onRequestSucc, the callback ot be called upon request success.
     * @param onRequestFail, the callback ot be called upon request failure.
     * @return ERR_OK on success, otherwise failure.
     */
    virtual ErrCode AddCompletionHandler(const std::string &requestId, OnRequestResult onRequestSucc,
        OnRequestResult onRequestFail)
    {
        return 0;
    }

    virtual ErrCode AddCompletionHandlerForOpenLink(const std::string &requestId,
        OnRequestResult onRequestSucc, OnRequestResult onRequestFail)
    {
        return 0;
    }

    /**
     * @brief Callback on request success.
     *
     * @param requestId, the requestId.
     * @param element, the want element of startAbility.
     * @param message, the message returned to the callback.
     */
    virtual void OnRequestSuccess(const std::string &requestId, const AppExecFwk::ElementName &element,
        const std::string &message)
    {
        return;
    }

    /**
     * @brief Callback on request failure.
     *
     * @param requestId, the requestId.
     * @param element, the want element of startAbility.
     * @param message, the message returned to the callback.
     */
    virtual void OnRequestFailure(const std::string &requestId, const AppExecFwk::ElementName &element,
        const std::string &message, int32_t resultCode)
    {
        return;
    }

    virtual void OnOpenLinkRequestSuccess(const std::string &requestId, const AppExecFwk::ElementName &element,
        const std::string &message)
    {
        return;
    }

    virtual void OnOpenLinkRequestFailure(const std::string &requestId, const AppExecFwk::ElementName &element,
        const std::string &message)
    {
        return;
    }

    virtual ErrCode SetOnNewWantSkipScenarios(int32_t scenarios)
    {
        return 0;
    }

    virtual ErrCode StartSelfUIAbilityInCurrentProcess(const AAFwk::Want &want, const std::string &specifiedFlag,
        const AAFwk::StartOptions &startOptions, bool isHaveOptions)
    {
        return ERR_OK;
    }

    virtual ErrCode NotifyCompleteGamePreLaunch()
    {
        return ERR_OK;
    }

    virtual ErrCode NotifyCancelGamePreLaunch()
    {
        return ERR_OK;
    }

    virtual ErrCode RestartAppWithWindow(const AAFwk::Want &want)
    {
        return 0;
    }

    void RegisterBindingObjectConfigUpdateCallback(BindingObjectConfigUpdateCallback callback) override
    {
    }

    void NotifyBindingObjectConfigUpdate() override
    {
    }
private:
    std::unique_ptr<NativeReference> mockContentStorage_ = nullptr;
};

class AbilityContextTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AbilityContextTest::SetUpTestCase(void)
{
}

void AbilityContextTest::TearDownTestCase(void)
{
}

void AbilityContextTest::SetUp(void)
{
}

void AbilityContextTest::TearDown(void)
{
}

/**
 * @tc.name: AbilityContextTest_ChangeAbilityVisibility_0100
 * @tc.desc: ChangeAbilityVisibility.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityContextTest, AbilityContext_ChangeAbilityVisibility_0100, Function | MediumTest | Level1)
{
    MockAbilityContext  mockAbilityContext;
    EXPECT_EQ(mockAbilityContext.ChangeAbilityVisibility(true), 0);
}
} // namespace AppExecFwk
} // namespace OHOS