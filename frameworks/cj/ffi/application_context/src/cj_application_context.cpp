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
 
#include "cj_application_context.h"

#include "ability_delegator_registry.h"
#include "ability_manager_errors.h"
#include "application_context.h"
#include "running_process_info.h"
#include "cj_utils_ffi.h"
#include "cj_lambda.h"
#include "cj_macro.h"
#include "hilog_tag_wrapper.h"
#include "cj_ability_runtime_error.h"

namespace OHOS {
namespace ApplicationContextCJ {
using namespace OHOS::FFI;
using namespace OHOS::AbilityRuntime;

std::vector<std::shared_ptr<CjAbilityLifecycleCallback>> CJApplicationContext::callbacks_;
CJApplicationContext* CJApplicationContext::cjApplicationContext_ = nullptr;
std::mutex CJApplicationContext::contexMutex_;

CJApplicationContext* CJApplicationContext::GetInstance()
{
    return GetCJApplicationContext(AbilityRuntime::ApplicationContext::GetInstance());
}

CJApplicationContext* CJApplicationContext::GetCJApplicationContext(
    std::weak_ptr<AbilityRuntime::ApplicationContext> &&applicationContext)
{
    if (cjApplicationContext_ == nullptr) {
        std::lock_guard<std::mutex> lock(contexMutex_);
        if (cjApplicationContext_ == nullptr) {
            cjApplicationContext_ = FFIData::Create<CJApplicationContext>(applicationContext);
        }
    }
    return cjApplicationContext_;
}

int CJApplicationContext::GetArea()
{
    auto context = applicationContext_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return ContextImpl::EL_DEFAULT;
    }
    return context->GetArea();
}

std::shared_ptr<AppExecFwk::ApplicationInfo> CJApplicationContext::GetApplicationInfo()
{
    auto context = applicationContext_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return nullptr;
    }
    return context->GetApplicationInfo();
}

bool CJApplicationContext::IsAbilityLifecycleCallbackEmpty()
{
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    return callbacks_.empty();
}

void CJApplicationContext::RegisterAbilityLifecycleCallback(
    const std::shared_ptr<CjAbilityLifecycleCallback> &abilityLifecycleCallback)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "called");
    if (abilityLifecycleCallback == nullptr) {
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    callbacks_.push_back(abilityLifecycleCallback);
}

void CJApplicationContext::UnregisterAbilityLifecycleCallback(
    const std::shared_ptr<CjAbilityLifecycleCallback> &abilityLifecycleCallback)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "called");
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    auto it = std::find(callbacks_.begin(), callbacks_.end(), abilityLifecycleCallback);
    if (it != callbacks_.end()) {
        callbacks_.erase(it);
    }
}

void CJApplicationContext::DispatchOnAbilityCreate(const int64_t &ability)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (!ability) {
        TAG_LOGE(AAFwkTag::CONTEXT, "ability is null");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityCreate(ability);
        }
    }
}

void CJApplicationContext::DispatchOnWindowStageCreate(const int64_t &ability, WindowStagePtr windowStage)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (!ability || !windowStage) {
        TAG_LOGE(AAFwkTag::CONTEXT, "ability or windowStage is nullptr");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnWindowStageCreate(ability, windowStage);
        }
    }
}

void CJApplicationContext::DispatchWindowStageFocus(const int64_t &ability, WindowStagePtr windowStage)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (!ability || !windowStage) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability or windowStage is null");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnWindowStageActive(ability, windowStage);
        }
    }
}

void CJApplicationContext::DispatchWindowStageUnfocus(const int64_t &ability, WindowStagePtr windowStage)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (!ability || !windowStage) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability or windowStage is nullptr");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnWindowStageInactive(ability, windowStage);
        }
    }
}

void CJApplicationContext::DispatchOnWindowStageDestroy(const int64_t &ability, WindowStagePtr windowStage)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (!ability || !windowStage) {
        TAG_LOGE(AAFwkTag::CONTEXT, "ability or windowStage is nullptr");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnWindowStageDestroy(ability, windowStage);
        }
    }
}

void CJApplicationContext::DispatchOnAbilityDestroy(const int64_t &ability)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability is null");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityDestroy(ability);
        }
    }
}

void CJApplicationContext::DispatchOnAbilityForeground(const int64_t &ability)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (!ability) {
        TAG_LOGE(AAFwkTag::CONTEXT, "ability is null");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityForeground(ability);
        }
    }
}

void CJApplicationContext::DispatchOnAbilityBackground(const int64_t &ability)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (!ability) {
        TAG_LOGE(AAFwkTag::CONTEXT, "ability is null");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityBackground(ability);
        }
    }
}

void CJApplicationContext::DispatchOnAbilityContinue(const int64_t &ability)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability is null");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityContinue(ability);
        }
    }
}

void CJApplicationContext::DispatchOnAbilityWillCreate(const int64_t &ability)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability is null");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityWillCreate(ability);
        }
    }
}

void CJApplicationContext::DispatchOnWindowStageWillCreate(const int64_t &ability, WindowStagePtr windowStage)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (!ability || !windowStage) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability or windowStage is null");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnWindowStageWillCreate(ability, windowStage);
        }
    }
}

void CJApplicationContext::DispatchOnWindowStageWillDestroy(const int64_t &ability, WindowStagePtr windowStage)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (!ability || !windowStage) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability or windowStage is null");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnWindowStageWillDestroy(ability, windowStage);
        }
    }
}

void CJApplicationContext::DispatchOnAbilityWillDestroy(const int64_t &ability)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability is null");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityWillDestroy(ability);
        }
    }
}

void CJApplicationContext::DispatchOnAbilityWillForeground(const int64_t &ability)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability is null");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityWillForeground(ability);
        }
    }
}

void CJApplicationContext::DispatchOnAbilityWillBackground(const int64_t &ability)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability is null");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityWillBackground(ability);
        }
    }
}

void CJApplicationContext::DispatchOnNewWant(const int64_t &ability)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability is null");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnNewWant(ability);
        }
    }
}

void CJApplicationContext::DispatchOnWillNewWant(const int64_t &ability)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability is null");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnWillNewWant(ability);
        }
    }
}

void CJApplicationContext::DispatchOnAbilityWillContinue(const int64_t &ability)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Dispatch onAbilityWillContinue");
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability is null");
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityWillContinue(ability);
        }
    }
}

void CJApplicationContext::DispatchOnWindowStageWillRestore(const int64_t &ability, WindowStagePtr windowStage)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Dispatch onWindowStageWillRestore");
    if (!ability || windowStage == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability or windowStage is null");
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnWindowStageWillRestore(ability, windowStage);
        }
    }
}

void CJApplicationContext::DispatchOnWindowStageRestore(const int64_t &ability, WindowStagePtr windowStage)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Dispatch onWindowStageRestore");
    if (!ability || windowStage == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability or windowStage is null");
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnWindowStageRestore(ability, windowStage);
        }
    }
}

void CJApplicationContext::DispatchOnAbilityWillSaveState(const int64_t &ability)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Dispatch onAbilityWillSaveState");
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability is null");
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityWillSaveState(ability);
        }
    }
}

void CJApplicationContext::DispatchOnAbilitySaveState(const int64_t &ability)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability is null");
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnAbilitySaveState(ability);
        }
    }
}

int32_t CJApplicationContext::OnOnEnvironment(void (*cfgCallback)(CConfiguration),
    void (*memCallback)(int32_t), bool isSync, int32_t &errCode)
{
    auto context = applicationContext_.lock();
    if (context == nullptr || cfgCallback == nullptr || memCallback == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        errCode = ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
        return -1;
    }
    if (envCallback_ != nullptr) {
        TAG_LOGD(AAFwkTag::CONTEXT, "envCallback_ is not nullptr.");
        return envCallback_->Register(CJLambda::Create(cfgCallback), CJLambda::Create(memCallback), isSync);
    }
    envCallback_ = std::make_shared<CjEnvironmentCallback>();
    int32_t callbackId = envCallback_->Register(CJLambda::Create(cfgCallback), CJLambda::Create(memCallback), isSync);
    context->RegisterEnvironmentCallback(envCallback_);
    TAG_LOGD(AAFwkTag::CONTEXT, "OnOnEnvironment is end");
    return callbackId;
}

int32_t CJApplicationContext::OnOnEnvironmentV2(void (*cfgCallback)(CConfigurationV2),
    void (*memCallback)(int32_t), bool isSync, int32_t &errCode)
{
    auto context = applicationContext_.lock();
    if (context == nullptr || cfgCallback == nullptr || memCallback == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        errCode = ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
        return -1;
    }
    if (envCallback_ != nullptr) {
        TAG_LOGD(AAFwkTag::CONTEXT, "envCallback_ is not nullptr.");
        return envCallback_->RegisterV2(CJLambda::Create(cfgCallback), CJLambda::Create(memCallback), isSync);
    }
    envCallback_ = std::make_shared<CjEnvironmentCallback>();
    int32_t callbackId = envCallback_->RegisterV2(CJLambda::Create(cfgCallback), CJLambda::Create(memCallback), isSync);
    context->RegisterEnvironmentCallback(envCallback_);
    TAG_LOGD(AAFwkTag::CONTEXT, "OnOnEnvironmentV2 is end");
    return callbackId;
}

const size_t FUNC_NUM = 22;

int32_t CJApplicationContext::OnOnAbilityLifecycle(std::vector<int64_t> &cFuncIds, bool isSync, int32_t &errCode)
{
    auto context = applicationContext_.lock();
    if (context == nullptr || cFuncIds.size() != FUNC_NUM) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        errCode = ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
        return -1;
    }
    if (callback_ != nullptr) {
        TAG_LOGD(AAFwkTag::CONTEXT, "callback_ is not nullptr.");
        return callback_->Register(cFuncIds, isSync);
    }
    callback_ = std::make_shared<CjAbilityLifecycleCallbackImpl>();
    int32_t callbackId = callback_->Register(cFuncIds, isSync);
    RegisterAbilityLifecycleCallback(callback_);
    return callbackId;
}

int32_t CJApplicationContext::OnOnApplicationStateChange(void (*foregroundCallback)(void),
    void (*backgroundCallback)(void), int32_t &errCode)
{
    auto context = applicationContext_.lock();
    if (context == nullptr || foregroundCallback == nullptr || backgroundCallback == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        errCode = ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
        return -1;
    }
    std::lock_guard<std::mutex> lock(applicationStateCallbackLock_);
    if (applicationStateCallback_ != nullptr) {
        return applicationStateCallback_->Register(CJLambda::Create(foregroundCallback),
            CJLambda::Create(backgroundCallback));
    }
    applicationStateCallback_ = std::make_shared<CjApplicationStateChangeCallback>();
    int32_t callbackId = applicationStateCallback_->Register(CJLambda::Create(foregroundCallback),
        CJLambda::Create(backgroundCallback));
    context->RegisterApplicationStateChangeCallback(applicationStateCallback_);
    return callbackId;
}

void CJApplicationContext::OnOffEnvironment(int32_t callbackId, int32_t &errCode)
{
    auto context = applicationContext_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        errCode = ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
        return;
    }
    std::weak_ptr<CjEnvironmentCallback> envCallbackWeak(envCallback_);
    auto env_callback = envCallbackWeak.lock();
    if (env_callback == nullptr) {
        TAG_LOGD(AAFwkTag::CONTEXT, "env_callback is not nullptr.");
        errCode = ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
        return;
    }
    TAG_LOGD(AAFwkTag::CONTEXT, "OnOffEnvironment begin");
    if (!env_callback->UnRegister(callbackId, false)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "call UnRegister failed");
        errCode = ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
        return;
    }
}

void CJApplicationContext::OnOffAbilityLifecycle(int32_t callbackId, int32_t &errCode)
{
    auto context = applicationContext_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        errCode = ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
        return;
    }
    std::weak_ptr<CjAbilityLifecycleCallbackImpl> callbackWeak(callback_);
    auto lifecycle_callback = callbackWeak.lock();
    if (lifecycle_callback == nullptr) {
        TAG_LOGD(AAFwkTag::CONTEXT, "env_callback is not nullptr.");
        errCode = ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
        return;
    }
    TAG_LOGD(AAFwkTag::CONTEXT, "OnOffAbilityLifecycle begin");
    if (!lifecycle_callback->UnRegister(callbackId, false)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "call UnRegister failed");
        errCode = ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
        return;
    }
}

void CJApplicationContext::OnOffApplicationStateChange(int32_t callbackId, int32_t &errCode)
{
    auto context = applicationContext_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        errCode = ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
        return;
    }
    std::lock_guard<std::mutex> lock(applicationStateCallbackLock_);
    if (applicationStateCallback_ == nullptr) {
        TAG_LOGD(AAFwkTag::CONTEXT, "env_callback is not nullptr.");
        errCode = ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
        return;
    }
    TAG_LOGD(AAFwkTag::CONTEXT, "OnOffApplicationStateChange begin");
    if (!applicationStateCallback_->UnRegister(callbackId)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "call UnRegister failed");
        errCode = ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
        return;
    }
    if (applicationStateCallback_->IsEmpty()) {
        applicationStateCallback_.reset();
    }
}

void CJApplicationContext::OnSetFont(std::string font)
{
    auto context = applicationContext_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "applicationContext is already released");
        return;
    }
    context->SetFont(font);
}

void CJApplicationContext::OnSetLanguage(std::string language)
{
    auto context = applicationContext_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "applicationContext is already released");
        return;
    }
    context->SetLanguage(language);
}

void CJApplicationContext::OnSetColorMode(int32_t colorMode)
{
    auto context = applicationContext_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "applicationContext is already released");
        return;
    }
    context->SetColorMode(colorMode);
}

std::shared_ptr<AppExecFwk::RunningProcessInfo> CJApplicationContext::OnGetRunningProcessInformation(
    int32_t &errCode)
{
    auto context = applicationContext_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "applicationContext is already released");
        errCode = ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST;
        return nullptr;
    }
    auto processInfo = std::make_shared<AppExecFwk::RunningProcessInfo>();
    errCode = context->GetProcessRunningInformation(*processInfo);
    return processInfo;
}

void CJApplicationContext::OnKillProcessBySelf(bool clearPageStack, int32_t &errCode)
{
    auto context = applicationContext_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "applicationContext is already released");
        errCode = ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST;
        return;
    }
    context->KillProcessBySelf(clearPageStack);
}

int32_t CJApplicationContext::OnGetCurrentAppCloneIndex(int32_t &errCode)
{
    auto context = applicationContext_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "applicationContext is already released");
        errCode = ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST;
        return -1;
    }
    if (context->GetCurrentAppMode() != static_cast<int32_t>(AppExecFwk::MultiAppModeType::APP_CLONE)) {
        errCode = ERR_ABILITY_RUNTIME_EXTERNAL_NOT_APP_CLONE;
        return -1;
    }
    return context->GetCurrentAppCloneIndex();
}

void CJApplicationContext::OnRestartApp(AAFwk::Want want, int32_t &errCode)
{
    auto context = applicationContext_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "applicationContext is already released");
        errCode = ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST;
        return;
    }
    auto code = context->RestartApp(want);
    if (code == ERR_OK) {
        return;
    }
    if (code == ERR_INVALID_VALUE) {
        errCode = ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    } else if (code == AAFwk::ERR_RESTART_APP_INCORRECT_ABILITY) {
        errCode = ERR_ABILITY_RUNTIME_RESTART_APP_INCORRECT_ABILITY;
    } else if (code == AAFwk::ERR_RESTART_APP_FREQUENT) {
        errCode = ERR_ABILITY_RUNTIME_RESTART_APP_FREQUENT;
    } else if (code == AAFwk::NOT_TOP_ABILITY) {
        errCode = ERR_ABILITY_RUNTIME_EXTERNAL_NOT_TOP_ABILITY;
    } else {
        errCode = ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR;
    }
}

void CJApplicationContext::OnClearUpApplicationData(int32_t &errCode)
{
    auto context = applicationContext_.lock();
    if (!context) {
        TAG_LOGE(AAFwkTag::APPKIT, "applicationContext is released");
        errCode = ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST;
        return;
    }
    context->ClearUpApplicationData();
}

void CJApplicationContext::OnSetSupportedProcessCacheSelf(bool isSupported, int32_t &errCode)
{
    auto context = applicationContext_.lock();
    if (!context) {
        TAG_LOGE(AAFwkTag::APPKIT, "applicationContext is released");
        errCode = ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST;
        return;
    }
    int32_t code = context->SetSupportedProcessCacheSelf(isSupported);
    if (code == AAFwk::ERR_CAPABILITY_NOT_SUPPORT) {
        errCode = ERR_ABILITY_RUNTIME_EXTERNAL_NO_SUCH_SYSCAP;
    } else if (code != ERR_OK) {
        errCode = ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR;
    }
}

int32_t CJApplicationContext::OnSetFontSizeScale(double fontSizeScale)
{
    auto context = applicationContext_.lock();
    if (!context) {
        TAG_LOGE(AAFwkTag::APPKIT, "applicationContext is released");
        return ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST;
    }
    if (context->SetFontSizeScale(fontSizeScale)) {
        return ERR_OK;
    } else {
        return ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR;
    }
}

extern "C" {
CJ_EXPORT void OHOS_CjAppCtxFunc(int32_t type, int64_t id)
{
    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationContext");
        return;
    }
    auto appContext = ApplicationContextCJ::CJApplicationContext::GetCJApplicationContext(applicationContext);
    if (appContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null appContext");
        return;
    }
    switch (CjAppCtxFuncType(type)) {
        case CjAppCtxFuncType::ON_ABILITY_WILL_CREATE:
            return appContext->DispatchOnAbilityWillCreate(id);
        case CjAppCtxFuncType::ON_ABILITY_CREATE:
            return appContext->DispatchOnAbilityCreate(id);
        case CjAppCtxFuncType::ON_ABILITY_WILL_DESTROY:
            return appContext->DispatchOnAbilityWillDestroy(id);
        case CjAppCtxFuncType::ON_ABILITY_DESTROY:
            return appContext->DispatchOnAbilityDestroy(id);
        case CjAppCtxFuncType::ON_ABILITY_WILL_FOREGROUND:
            return appContext->DispatchOnAbilityWillForeground(id);
        case CjAppCtxFuncType::ON_ABILITY_FOREGROUND:
            return appContext->DispatchOnAbilityForeground(id);
        case CjAppCtxFuncType::ON_ABILITY_WILL_BACKGROUND:
            return appContext->DispatchOnAbilityWillBackground(id);
        case CjAppCtxFuncType::ON_ABILITY_BACKGROUND:
            return appContext->DispatchOnAbilityBackground(id);
        case CjAppCtxFuncType::ON_ABILITY_WILL_CONTINUE:
            return appContext->DispatchOnAbilityWillContinue(id);
        case CjAppCtxFuncType::ON_ABILITY_CONTINUE:
            return appContext->DispatchOnAbilityContinue(id);
        case CjAppCtxFuncType::ON_ABILITY_WILL_SAVE_STATE:
            return appContext->DispatchOnAbilityWillSaveState(id);
        case CjAppCtxFuncType::ON_ABILITY_SAVE_STATE:
            return appContext->DispatchOnAbilitySaveState(id);
        case CjAppCtxFuncType::ON_WILL_NEW_WANT:
            return appContext->DispatchOnWillNewWant(id);
        case CjAppCtxFuncType::ON_NEW_WANT:
            return appContext->DispatchOnNewWant(id);
        default:
            TAG_LOGE(AAFwkTag::APPKIT, "invalid type: %{public}d", type);
            return;
    }
}

CJ_EXPORT void OHOS_CjAppCtxWindowFunc(int32_t type, int64_t id, void* window)
{
    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationContext");
        return;
    }
    auto appContext = ApplicationContextCJ::CJApplicationContext::GetCJApplicationContext(applicationContext);
    if (appContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null appContext");
        return;
    }
    switch (CjAppCtxFuncType(type)) {
        case CjAppCtxFuncType::ON_WINDOWSTAGE_WILL_CREATE:
            return appContext->DispatchOnWindowStageWillCreate(id, window);
        case CjAppCtxFuncType::ON_WINDOWSTAGE_CREATE:
            return appContext->DispatchOnWindowStageCreate(id, window);
        case CjAppCtxFuncType::ON_WINDOWSTAGE_WILL_RESTORE:
            return appContext->DispatchOnWindowStageWillRestore(id, window);
        case CjAppCtxFuncType::ON_WINDOWSTAGE_RESTORE:
            return appContext->DispatchOnWindowStageRestore(id, window);
        case CjAppCtxFuncType::ON_WINDOWSTAGE_WILL_DESTROY:
            return appContext->DispatchOnWindowStageWillDestroy(id, window);
        case CjAppCtxFuncType::ON_WINDOWSTAGE_DESTROY:
            return appContext->DispatchOnWindowStageDestroy(id, window);
        case CjAppCtxFuncType::WINDOWSTAGE_FOCUS:
            return appContext->DispatchWindowStageFocus(id, window);
        case CjAppCtxFuncType::WINDOWSTAGE_UNFOCUS:
            return appContext->DispatchWindowStageUnfocus(id, window);
        default:
            TAG_LOGE(AAFwkTag::APPKIT, "invalid type: %{public}d", type);
            return;
    }
}
}
}
}