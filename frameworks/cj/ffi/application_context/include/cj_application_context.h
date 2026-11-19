/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_CJ_APPLICATION_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_CJ_APPLICATION_CONTEXT_H

#include <cstdint>
#include <shared_mutex>

#include "application_context.h"
#include "cj_ability_lifecycle_callback_impl.h"
#include "cj_application_state_change_callback.h"
#include "cj_environment_callback.h"
#include "cj_context.h"
#include "ffi_remote_data.h"
#include "running_process_info.h"
#include "want.h"
#include "want_params.h"

namespace OHOS {
namespace AbilityRuntime {
enum class CjAppCtxFuncType : int32_t {
    ON_ABILITY_WILL_CREATE = 0,
    ON_ABILITY_CREATE,
    ON_ABILITY_WILL_DESTROY,
    ON_ABILITY_DESTROY,
    ON_WINDOWSTAGE_WILL_CREATE,
    ON_WINDOWSTAGE_CREATE,
    ON_WINDOWSTAGE_WILL_RESTORE,
    ON_WINDOWSTAGE_RESTORE,
    ON_WINDOWSTAGE_WILL_DESTROY,
    ON_WINDOWSTAGE_DESTROY,
    ON_ABILITY_WILL_FOREGROUND,
    ON_ABILITY_FOREGROUND,
    ON_ABILITY_WILL_BACKGROUND,
    ON_ABILITY_BACKGROUND,
    WINDOWSTAGE_FOCUS,
    WINDOWSTAGE_UNFOCUS,
    ON_ABILITY_WILL_CONTINUE,
    ON_ABILITY_CONTINUE,
    ON_ABILITY_WILL_SAVE_STATE,
    ON_ABILITY_SAVE_STATE,
    ON_WILL_NEW_WANT,
    ON_NEW_WANT,
};
}

namespace ApplicationContextCJ {
using namespace OHOS::AbilityRuntime;

enum CjAppProcessState {
    STATE_CREATE,
    STATE_FOREGROUND,
    STATE_ACTIVE,
    STATE_BACKGROUND,
    STATE_DESTROY
};

class CJApplicationContext : public FfiContext::CJContext {
public:
    explicit CJApplicationContext(std::weak_ptr<AbilityRuntime::ApplicationContext> &&applicationContext)
        : FfiContext::CJContext(applicationContext.lock()), applicationContext_(std::move(applicationContext)) {};

    int GetArea();
    std::shared_ptr<AppExecFwk::ApplicationInfo> GetApplicationInfo();
    void RegisterAbilityLifecycleCallback(const std::shared_ptr<CjAbilityLifecycleCallback> &abilityLifecycleCallback);
    void UnregisterAbilityLifecycleCallback(
        const std::shared_ptr<CjAbilityLifecycleCallback> &abilityLifecycleCallback);
    bool IsAbilityLifecycleCallbackEmpty();
    void DispatchOnAbilityCreate(const int64_t &ability);
    void DispatchOnWindowStageCreate(const int64_t &ability, WindowStagePtr windowStage);
    void DispatchWindowStageFocus(const int64_t &ability, WindowStagePtr windowStage);
    void DispatchWindowStageUnfocus(const int64_t &ability, WindowStagePtr windowStage);
    void DispatchOnWindowStageDestroy(const int64_t &ability, WindowStagePtr windowStage);
    void DispatchOnAbilityDestroy(const int64_t &ability);
    void DispatchOnAbilityForeground(const int64_t &ability);
    void DispatchOnAbilityBackground(const int64_t &ability);
    void DispatchOnAbilityContinue(const int64_t &ability);
    // optional callbacks
    void DispatchOnAbilityWillCreate(const int64_t &ability);
    void DispatchOnWindowStageWillCreate(const int64_t &ability, WindowStagePtr windowStage);
    void DispatchOnWindowStageWillDestroy(const int64_t &ability, WindowStagePtr windowStage);
    void DispatchOnAbilityWillDestroy(const int64_t &ability);
    void DispatchOnAbilityWillForeground(const int64_t &ability);
    void DispatchOnAbilityWillBackground(const int64_t &ability);
    void DispatchOnNewWant(const int64_t &ability);
    void DispatchOnWillNewWant(const int64_t &ability);
    void DispatchOnAbilityWillContinue(const int64_t &ability);
    void DispatchOnWindowStageWillRestore(const int64_t &ability, WindowStagePtr windowStage);
    void DispatchOnWindowStageRestore(const int64_t &ability, WindowStagePtr windowStage);
    void DispatchOnAbilityWillSaveState(const int64_t &ability);
    void DispatchOnAbilitySaveState(const int64_t &ability);

    void OnSetFont(std::string font);
    void OnSetLanguage(std::string font);
    void OnSetColorMode(int32_t colorMode);
    std::shared_ptr<AppExecFwk::RunningProcessInfo> OnGetRunningProcessInformation(int32_t &errCode);
    void OnKillProcessBySelf(bool clearPageStack, int32_t &errCode);
    int32_t OnGetCurrentAppCloneIndex(int32_t &errCode);
    void OnRestartApp(AAFwk::Want want, int32_t &errCode);
    void OnClearUpApplicationData(int32_t &errCode);
    void OnSetSupportedProcessCacheSelf(bool isSupported, int32_t &errCode);
    int32_t OnSetFontSizeScale(double fontSizeScale);

    int32_t OnOnEnvironment(void (*cfgCallback)(AbilityRuntime::CConfiguration),
        void (*memCallback)(int32_t), bool isSync, int32_t &errCode);
    int32_t OnOnEnvironmentV2(void (*cfgCallback)(AbilityRuntime::CConfigurationV2),
        void (*memCallback)(int32_t), bool isSync, int32_t &errCode);
    int32_t OnOnAbilityLifecycle(std::vector<int64_t> &cFuncIds, bool isSync, int32_t &errCode);
    int32_t OnOnApplicationStateChange(void (*foregroundCallback)(void),
        void (*backgroundCallback)(void), int32_t &errCode);
    void OnOffEnvironment(int32_t callbackId, int32_t &errCode);
    void OnOffAbilityLifecycle(int32_t callbackId, int32_t &errCode);
    void OnOffApplicationStateChange(int32_t callbackId, int32_t &errCode);
    static CJApplicationContext* GetInstance();
    static CJApplicationContext* GetCJApplicationContext(
        std::weak_ptr<AbilityRuntime::ApplicationContext> &&applicationContext);
    std::shared_ptr<AbilityRuntime::ApplicationContext> GetApplicationContext()
    {
        return applicationContext_.lock();
    }
private:
    std::weak_ptr<AbilityRuntime::ApplicationContext> applicationContext_;
    std::shared_ptr<AbilityRuntime::CjAbilityLifecycleCallbackImpl> callback_;
    std::shared_ptr<AbilityRuntime::CjEnvironmentCallback> envCallback_;
    std::shared_ptr<CjApplicationStateChangeCallback> applicationStateCallback_;
    std::mutex applicationStateCallbackLock_;
    std::recursive_mutex callbackLock_;
    static std::vector<std::shared_ptr<CjAbilityLifecycleCallback>> callbacks_;
    static CJApplicationContext* cjApplicationContext_;
    static std::mutex contexMutex_;
};

}
}
#endif // OHOS_ABILITY_RUNTIME_CJ_APPLICATION_CONTEXT_H