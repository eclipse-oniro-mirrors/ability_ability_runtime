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

#include "cj_ui_ability.h"

#include <cstdlib>
#include <dlfcn.h>
#include <regex>

#include "ability_business_error.h"
#include "ability_delegator_registry.h"
#include "ability_recovery.h"
#include "ability_start_setting.h"
#include "app_recovery.h"
#include "cj_ability_context.h"
#include "cj_ability_object.h"
#include "cj_insight_intent_executor_info.h"
#include "cj_insight_intent_executor_mgr.h"
#include "cj_runtime.h"
#include "connection_manager.h"
#include "context/application_context.h"
#include "context/context.h"
#include "display_util.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "if_system_ability_manager.h"
#include "insight_intent_execute_param.h"
#include "time_util.h"
#ifdef SUPPORT_SCREEN
#include "scene_board_judgement.h"
#endif
#include "string_wrapper.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
#ifdef SUPPORT_GRAPHICS
const std::string PAGE_STACK_PROPERTY_NAME = "pageStack";
const std::string METHOD_NAME = "WindowScene::GoForeground";
const std::string SUPPORT_CONTINUE_PAGE_STACK_PROPERTY_NAME = "ohos.extra.param.key.supportContinuePageStack";
#endif
#ifdef SUPPORT_SCREEN
// Numerical base (radix) that determines the valid characters and their interpretation.
const int32_t BASE_DISPLAY_ID_NUM (10);
#endif
const char* CJWINDOW_FFI_LIBNAME = "libcj_window_ffi.z.so";
const char* FUNC_CREATE_CJWINDOWSTAGE = "OHOS_CreateCJWindowStage";
constexpr const int32_t API12 = 12;
constexpr const int32_t API_VERSION_MOD = 100;
using CFFICreateCJWindowStage = int64_t (*)(std::shared_ptr<Rosen::WindowScene>&);

const char* CJ_ABILITY_LIBNAME = "libcj_ability_ffi.z.so";
const char* CJ_APP_CTX_FUNC = "OHOS_CjAppCtxFunc";
const char* CJ_APP_CTX_WINDOW_FUNC = "OHOS_CjAppCtxWindowFunc";

const char* CJ_IPC_LIBNAME = "libcj_ipc_ffi.z.so";
const char* FUNC_GET_NATIVE_REMOTEOBJECT = "OHOS_CallGetNativeRemoteObject";

const char* CJ_INSIGHT_LIBNAME = "libcj_insight_intent_executor.z.so";
const char* FUNC_TRIGGER_CALLBACK_INNER = "OHOS_CallTriggerCallbackInner";
const char* FUNC_EXECUTE_INSIGHT_INTENT = "OHOS_CallExecuteInsightIntent";

sptr<Rosen::CJWindowStageImpl> CreateCJWindowStage(std::shared_ptr<Rosen::WindowScene> windowScene)
{
    static void* handle = nullptr;
    if (handle == nullptr) {
        handle = dlopen(CJWINDOW_FFI_LIBNAME, RTLD_LAZY);
        if (handle == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null handle");
            return nullptr;
        }
    }
    // get function
    auto func = reinterpret_cast<CFFICreateCJWindowStage>(dlsym(handle, FUNC_CREATE_CJWINDOWSTAGE));
    if (func == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null func");
        dlclose(handle);
        handle = nullptr;
        return nullptr;
    }
    auto id = func(windowScene);
    return OHOS::FFI::FFIData::GetData<Rosen::CJWindowStageImpl>(id);
}
}

UIAbility *CJUIAbility::Create(const std::unique_ptr<Runtime> &runtime)
{
    return new (std::nothrow) CJUIAbility(static_cast<CJRuntime &>(*runtime));
}

CJUIAbility::CJUIAbility(CJRuntime &cjRuntime) : cjRuntime_(cjRuntime)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
}

CJUIAbility::~CJUIAbility()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (abilityContext_ != nullptr) {
        abilityContext_->Unbind();
    }
    if (cjAppHandle_) {
        dlclose(cjAppHandle_);
    }
}

void CJUIAbility::Init(std::shared_ptr<AppExecFwk::AbilityLocalRecord> record,
    const std::shared_ptr<OHOSApplication> application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token, bool &createObjSuc)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null record");
        return;
    }
    auto abilityInfo = record->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityInfo");
        return;
    }
    UIAbility::Init(record, application, handler, token, createObjSuc);

#ifdef SUPPORT_GRAPHICS
    if (abilityContext_ != nullptr) {
        AppExecFwk::AppRecovery::GetInstance().AddAbility(
            shared_from_this(), abilityContext_->GetAbilityInfo(), abilityContext_->GetToken());
    }
#endif
    SetAbilityContext(abilityInfo);
    cjAppHandle_ = dlopen(CJ_ABILITY_LIBNAME, RTLD_LAZY);
    if (cjAppHandle_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAppHandle");
        return;
    }
}

void CJUIAbility::SetAbilityContext(
    const std::shared_ptr<AbilityInfo> &abilityInfo)
{
    if (!abilityInfo) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityInfo");
        return;
    }
    cjAbilityObj_ = CJAbilityObject::LoadModule(abilityInfo->name);
    if (!cjAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAbilityObj_");
        return;
    }
    cjAbilityObj_->Init(this);
    if (abilityRecovery_ != nullptr) {
        abilityRecovery_->SetJsAbility(reinterpret_cast<uintptr_t>(abilityContext_.get()));
    }
}

void CJUIAbility::CallAppCtxFunc(CjAppCtxFuncType type, int64_t id)
{
    if (cjAppHandle_ != nullptr) {
        auto symbol = dlsym(cjAppHandle_, CJ_APP_CTX_FUNC);
        if (symbol == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "dlsym failed %{public}s, %{public}s", CJ_APP_CTX_FUNC, dlerror());
            return;
        }
        auto func = reinterpret_cast<void (*)(int32_t, int64_t)>(symbol);
        func(static_cast<int32_t>(type), id);
    }
}

void CJUIAbility::CallAppCtxWindowFunc(CjAppCtxFuncType type, int64_t id, sptr<Rosen::CJWindowStageImpl> window)
{
    if (cjAppHandle_ != nullptr) {
        WindowStagePtr windowStage = reinterpret_cast<WindowStagePtr>(window.GetRefPtr());
        auto symbol = dlsym(cjAppHandle_, CJ_APP_CTX_WINDOW_FUNC);
        if (symbol == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "dlsym failed %{public}s, %{public}s", CJ_APP_CTX_WINDOW_FUNC, dlerror());
            return;
        }
        auto func = reinterpret_cast<void (*)(int32_t, int64_t, void*)>(symbol);
        func(static_cast<int32_t>(type), id, windowStage);
    }
}

void CJUIAbility::OnStart(const Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "ability is %{public}s", GetAbilityName().c_str());
    UIAbility::OnStart(want, sessionInfo);

    if (!cjAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cJAbility");
        return;
    }
    CallAppCtxFunc(CjAppCtxFuncType::ON_ABILITY_WILL_CREATE, cjAbilityObj_->GetId());
    std::string methodName = "OnStart";
    AddLifecycleEventBeforeCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
    cjAbilityObj_->OnStart(want, GetLaunchParam());
    AddLifecycleEventAfterCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetCJAbilityDelegator();
    if (delegator) {
        TAG_LOGD(AAFwkTag::UIABILITY, "call PostPerformStart");
        delegator->PostPerformStart(CreateADelegatorAbilityProperty());
    }
    CallAppCtxFunc(CjAppCtxFuncType::ON_ABILITY_CREATE, cjAbilityObj_->GetId());
}

void CJUIAbility::AddLifecycleEventBeforeCall(FreezeUtil::TimeoutState state, const std::string &methodName) const
{
    auto entry = std::string("CJUIAbility::") + methodName + " begin";
    FreezeUtil::GetInstance().AddLifecycleEvent(AbilityContext::token_, entry);
}

void CJUIAbility::AddLifecycleEventAfterCall(FreezeUtil::TimeoutState state, const std::string &methodName) const
{
    auto entry = std::string("CJUIAbility::") + methodName + " end";
    FreezeUtil::GetInstance().AddLifecycleEvent(AbilityContext::token_, entry);
}

int32_t CJUIAbility::OnShare(WantParams &wantParams)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (!cjAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAbilityObj_");
        return ERR_INVALID_VALUE;
    }
    return cjAbilityObj_->OnShare(wantParams);
}

void CJUIAbility::OnStop()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (abilityContext_) {
        TAG_LOGD(AAFwkTag::UIABILITY, "set terminating true");
        abilityContext_->SetTerminating(true);
    }
    UIAbility::OnStop();
    if (!cjAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAbilityObj");
        return;
    }
    CallAppCtxFunc(CjAppCtxFuncType::ON_ABILITY_WILL_DESTROY, cjAbilityObj_->GetId());
    cjAbilityObj_->OnStop();
    CJUIAbility::OnStopCallback();
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void CJUIAbility::OnStop(AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback)
{
    if (callbackInfo == nullptr) {
        isAsyncCallback = false;
        OnStop();
        return;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin");
    if (abilityContext_) {
        TAG_LOGD(AAFwkTag::UIABILITY, "set terminating true");
        abilityContext_->SetTerminating(true);
    }

    UIAbility::OnStop();
    if (cjAbilityObj_ != nullptr) {
        CallAppCtxFunc(CjAppCtxFuncType::ON_ABILITY_WILL_DESTROY, cjAbilityObj_->GetId());
        cjAbilityObj_->OnStop();
    }
    OnStopCallback();
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void CJUIAbility::OnStopCallback()
{
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetCJAbilityDelegator();
    if (delegator) {
        TAG_LOGD(AAFwkTag::UIABILITY, "call PostPerformStop");
        delegator->PostPerformStop(CreateADelegatorAbilityProperty());
    }

    bool ret = ConnectionManager::GetInstance().DisconnectCaller(AbilityContext::token_);
    if (!ret) {
        TAG_LOGE(AAFwkTag::UIABILITY, "service connection disconnected");
    }
    ConnectionManager::GetInstance().ReportConnectionLeakEvent(getpid(), gettid());
    if (cjAbilityObj_ != nullptr) {
        CallAppCtxFunc(CjAppCtxFuncType::ON_ABILITY_DESTROY, cjAbilityObj_->GetId());
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

#ifdef SUPPORT_GRAPHICS
#ifdef SUPPORT_SCREEN
void CJUIAbility::OnSceneCreated()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "ability: %{public}s", GetAbilityName().c_str());
    UIAbility::OnSceneCreated();

    if (!cjAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAbilityObj");
        return;
    }
    cjWindowStage_ = CreateCJWindowStage(GetScene());
    if (!cjWindowStage_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjWindowStage");
        return;
    }
    CallAppCtxWindowFunc(CjAppCtxFuncType::ON_WINDOWSTAGE_WILL_CREATE, cjAbilityObj_->GetId(), cjWindowStage_);
    {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "onWindowStageCreate");
        std::string methodName = "OnSceneCreated";
        AddLifecycleEventBeforeCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
        cjAbilityObj_->OnSceneCreated(cjWindowStage_.GetRefPtr());
        AddLifecycleEventAfterCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
    }

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetCJAbilityDelegator();
    if (delegator) {
        TAG_LOGD(AAFwkTag::UIABILITY, "call PostPerformScenceCreated");
        delegator->PostPerformScenceCreated(CreateADelegatorAbilityProperty());
    }
    CallAppCtxWindowFunc(CjAppCtxFuncType::ON_WINDOWSTAGE_CREATE, cjAbilityObj_->GetId(), cjWindowStage_);
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void CJUIAbility::OnSceneRestored()
{
    UIAbility::OnSceneRestored();
    TAG_LOGD(AAFwkTag::UIABILITY, "called");

    if (!cjAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAbilityObj");
        return;
    }

    if (!cjWindowStage_) {
        cjWindowStage_ = CreateCJWindowStage(scene_);
        if (!cjWindowStage_) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null cjWindowStage");
            return;
        }
    }
    CallAppCtxWindowFunc(CjAppCtxFuncType::ON_WINDOWSTAGE_WILL_RESTORE, cjAbilityObj_->GetId(), cjWindowStage_);
    cjAbilityObj_->OnSceneRestored(cjWindowStage_.GetRefPtr());
    CallAppCtxWindowFunc(CjAppCtxFuncType::ON_WINDOWSTAGE_RESTORE, cjAbilityObj_->GetId(), cjWindowStage_);

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetCJAbilityDelegator();
    if (delegator) {
        TAG_LOGD(AAFwkTag::UIABILITY, "call PostPerformScenceRestored");
        delegator->PostPerformScenceRestored(CreateADelegatorAbilityProperty());
    }
}

void CJUIAbility::OnSceneWillDestroy()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "ability: %{public}s", GetAbilityName().c_str());
    if (!cjAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAbilityObj");
        return;
    }
    if (!cjWindowStage_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null CJWindowStage object");
        return;
    }
    cjAbilityObj_->OnSceneWillDestroy(cjWindowStage_.GetRefPtr());
}

void CJUIAbility::onSceneDestroyed()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "ability is %{public}s", GetAbilityName().c_str());
    UIAbility::onSceneDestroyed();

    if (!cjAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAbilityObj");
        return;
    }
    CallAppCtxWindowFunc(CjAppCtxFuncType::ON_WINDOWSTAGE_WILL_DESTROY, cjAbilityObj_->GetId(), cjWindowStage_);
    cjAbilityObj_->OnSceneDestroyed();

    if (scene_ != nullptr) {
        auto window = scene_->GetMainWindow();
        if (window != nullptr) {
            TAG_LOGD(AAFwkTag::UIABILITY, "window UnregisterDisplayMoveListener");
            window->UnregisterDisplayMoveListener(abilityDisplayMoveListener_);
        }
    }

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetCJAbilityDelegator();
    if (delegator) {
        TAG_LOGD(AAFwkTag::UIABILITY, "call PostPerformScenceDestroyed");
        delegator->PostPerformScenceDestroyed(CreateADelegatorAbilityProperty());
    }
    CallAppCtxWindowFunc(CjAppCtxFuncType::ON_WINDOWSTAGE_DESTROY, cjAbilityObj_->GetId(), cjWindowStage_);
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void CJUIAbility::OnForeground(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "ability: %{public}s", GetAbilityName().c_str());

    UIAbility::OnForeground(want);
    CallOnForegroundFunc(want);
}

void CJUIAbility::CallOnForegroundFunc(const Want &want)
{
    if (!cjAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAbilityObj");
        return;
    }
    CallAppCtxFunc(CjAppCtxFuncType::ON_ABILITY_WILL_FOREGROUND, cjAbilityObj_->GetId());
    std::string methodName = "OnForeground";
    AddLifecycleEventBeforeCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
    cjAbilityObj_->OnForeground(want);
    AddLifecycleEventAfterCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetCJAbilityDelegator();
    if (delegator) {
        TAG_LOGD(AAFwkTag::UIABILITY, "call PostPerformForeground");
        delegator->PostPerformForeground(CreateADelegatorAbilityProperty());
    }
    CallAppCtxFunc(CjAppCtxFuncType::ON_ABILITY_FOREGROUND, cjAbilityObj_->GetId());
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void CJUIAbility::OnBackground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "ability: %{public}s", GetAbilityName().c_str());

    if (cjAbilityObj_ != nullptr) {
        CallAppCtxFunc(CjAppCtxFuncType::ON_ABILITY_WILL_BACKGROUND, cjAbilityObj_->GetId());
    }

    UIAbility::OnBackground();

    if (!cjAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAbilityObj");
        return;
    }
    std::string methodName = "OnBackground";
    AddLifecycleEventBeforeCall(FreezeUtil::TimeoutState::BACKGROUND, methodName);
    cjAbilityObj_->OnBackground();
    AddLifecycleEventAfterCall(FreezeUtil::TimeoutState::BACKGROUND, methodName);

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetCJAbilityDelegator();
    if (delegator) {
        TAG_LOGD(AAFwkTag::UIABILITY, "call PostPerformBackground");
        delegator->PostPerformBackground(CreateADelegatorAbilityProperty());
    }

    CallAppCtxFunc(CjAppCtxFuncType::ON_ABILITY_BACKGROUND, cjAbilityObj_->GetId());
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void CJUIAbility::OnAfterFocusedCommon(bool isFocused)
{
    if (!cjAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAbilityObj");
        return;
    }

    if (isFocused) {
        CallAppCtxWindowFunc(CjAppCtxFuncType::WINDOWSTAGE_FOCUS, cjAbilityObj_->GetId(), cjWindowStage_);
    } else {
        CallAppCtxWindowFunc(CjAppCtxFuncType::WINDOWSTAGE_UNFOCUS, cjAbilityObj_->GetId(), cjWindowStage_);
    }
}

bool CJUIAbility::OnBackPress()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "ability: %{public}s", GetAbilityName().c_str());
    UIAbility::OnBackPress();
    bool defaultRet = BackPressDefaultValue();
    if (!cjAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAbilityObj");
        return defaultRet;
    }
    bool ret = cjAbilityObj_->OnBackPress(defaultRet);
    TAG_LOGD(AAFwkTag::UIABILITY, "end ret: %{public}d", ret);
    return ret;
}

bool CJUIAbility::OnPrepareTerminate()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "ability: %{public}s", GetAbilityName().c_str());
    UIAbility::OnPrepareTerminate();
    if (!cjAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAbilityObj");
        return false;
    }
    bool ret = cjAbilityObj_->OnPrepareTerminate();
    TAG_LOGD(AAFwkTag::UIABILITY, "end ret: %{public}d", ret);
    return ret;
}

void CJUIAbility::GetPageStackFromWant(const Want &want, std::string &pageStack)
{
    auto stringObj = AAFwk::IString::Query(want.GetParams().GetParam(PAGE_STACK_PROPERTY_NAME));
    if (stringObj != nullptr) {
        pageStack = AAFwk::String::Unbox(stringObj);
    }
}

bool CJUIAbility::IsRestorePageStack(const Want &want)
{
    return want.GetBoolParam(SUPPORT_CONTINUE_PAGE_STACK_PROPERTY_NAME, true);
}

void CJUIAbility::RestorePageStack(const Want &want)
{
    if (IsRestorePageStack(want)) {
        std::string pageStack;
        GetPageStackFromWant(want, pageStack);
    }
}

void CJUIAbility::AbilityContinuationOrRecover(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    // multi-instance ability continuation
    TAG_LOGD(AAFwkTag::UIABILITY, "launch reason: %{public}d", launchParam_.launchReason);
    if (IsRestoredInContinuation()) {
        RestorePageStack(want);
        OnSceneRestored();
        NotifyContinuationResult(want, true);
    } else if (ShouldRecoverState(want)) {
        std::string pageStack = abilityRecovery_->GetSavedPageStack(AppExecFwk::StateReason::DEVELOPER_REQUEST);
        OnSceneRestored();
    } else {
        OnSceneCreated();
    }
}

void CJUIAbility::DoOnForeground(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (scene_ == nullptr) {
        if ((abilityContext_ == nullptr) || (sceneListener_ == nullptr)) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null abilityContext or sceneListener");
            return;
        }
        scene_ = std::make_shared<Rosen::WindowScene>();
        InitSceneDoOnForeground(scene_, want);
    } else {
        auto window = scene_->GetMainWindow();
        if (window  == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null window");
            return;
        }
        if (want.HasParameter(Want::PARAM_RESV_WINDOW_MODE)) {
            TAG_LOGI(AAFwkTag::UIABILITY, "recv window mode");
            auto windowMode = want.GetIntParam(
                Want::PARAM_RESV_WINDOW_MODE, AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED);
            window->SetWindowMode(static_cast<Rosen::WindowMode>(windowMode));
            windowMode_ = windowMode;
            TAG_LOGD(AAFwkTag::UIABILITY, "set window mode: %{public}d", windowMode);
        }
    }

    auto window = scene_->GetMainWindow();
    if (window  == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null window");
        return;
    }
    if (securityFlag_) {
        window->SetSystemPrivacyMode(true);
    }

    TAG_LOGD(AAFwkTag::UIABILITY, "move scene to foreground, sceneFlag_: %{public}d", UIAbility::sceneFlag_);
    AddLifecycleEventBeforeCall(FreezeUtil::TimeoutState::FOREGROUND, METHOD_NAME);
    scene_->GoForeground(UIAbility::sceneFlag_);
}

void CJUIAbility::InitSceneDoOnForeground(std::shared_ptr<Rosen::WindowScene> scene, const Want &want)
{
    int32_t displayId = AAFwk::DisplayUtil::GetDefaultDisplayId();
    if (setting_ != nullptr) {
        std::string strDisplayId = setting_->GetProperty(OHOS::AppExecFwk::AbilityStartSetting::WINDOW_DISPLAY_ID_KEY);
        std::regex formatRegex("[0-9]{0,9}$");
        std::smatch sm;
        bool flag = std::regex_match(strDisplayId, sm, formatRegex);
        if (flag && !strDisplayId.empty()) {
            displayId = strtol(strDisplayId.c_str(), nullptr, BASE_DISPLAY_ID_NUM);
            TAG_LOGD(AAFwkTag::UIABILITY, "displayId: %{public}d", displayId);
        } else {
            TAG_LOGE(AAFwkTag::UIABILITY, "formatRegex: [%{public}s] failed", strDisplayId.c_str());
        }
    }
    Rosen::WMError ret = Rosen::WMError::WM_OK;
    auto option = GetWindowOption(want);
    auto sessionToken = GetSessionToken();
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled() && sessionToken != nullptr) {
        abilityContext_->SetWeakSessionToken(sessionToken);
        ret = scene_->Init(displayId, abilityContext_, sceneListener_, option, sessionToken);
    } else {
        ret = scene_->Init(displayId, abilityContext_, sceneListener_, option);
    }
    if (ret != Rosen::WMError::WM_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "init window scene failed");
        return;
    }

    AbilityContinuationOrRecover(want);
    auto window = scene_->GetMainWindow();
    if (window) {
        TAG_LOGD(AAFwkTag::UIABILITY,
            "call RegisterDisplayMoveListener, windowId: %{public}d", window->GetWindowId());
        abilityDisplayMoveListener_ = new AbilityDisplayMoveListener(weak_from_this());
        if (abilityDisplayMoveListener_ == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null abilityDisplayMoveListener_");
            return;
        }
        window->RegisterDisplayMoveListener(abilityDisplayMoveListener_);
    }
}

void CJUIAbility::RequestFocus(const Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (scene_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null scene_");
        return;
    }
    auto window = scene_->GetMainWindow();
    if (window != nullptr && want.HasParameter(Want::PARAM_RESV_WINDOW_MODE)) {
        auto windowMode = want.GetIntParam(
            Want::PARAM_RESV_WINDOW_MODE, AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED);
        window->SetWindowMode(static_cast<Rosen::WindowMode>(windowMode));
        TAG_LOGD(AAFwkTag::UIABILITY, "set window mode: %{public}d", windowMode);
    }
    AddLifecycleEventBeforeCall(FreezeUtil::TimeoutState::FOREGROUND, METHOD_NAME);
    scene_->GoForeground(UIAbility::sceneFlag_);
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void CJUIAbility::ContinuationRestore(const Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (!IsRestoredInContinuation() || scene_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "is not in continuation or null scene_");
        return;
    }
    RestorePageStack(want);
    OnSceneRestored();
    NotifyContinuationResult(want, true);
}

const CJRuntime &CJUIAbility::GetCJRuntime()
{
    return cjRuntime_;
}

void CallTriggerCallbackInner(std::unique_ptr<InsightIntentExecutorAsyncCallback> callback, int32_t errCode)
{
    void* handle = dlopen(CJ_INSIGHT_LIBNAME, RTLD_LAZY);
    if (handle == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null handle");
        return;
    }
    using TriggerCallbackInnerFunc = void (*)(void*, int32_t);
    auto func = reinterpret_cast<TriggerCallbackInnerFunc>(dlsym(handle, FUNC_TRIGGER_CALLBACK_INNER));
    if (func == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null func");
        dlclose(handle);
        return;
    }
    func(&callback, errCode);
    dlclose(handle);
}

bool CallExecuteInsightIntent(Runtime& runtime, CJInsightIntentExecutorInfo& executeInfo,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback)
{
    void* handle = dlopen(CJ_INSIGHT_LIBNAME, RTLD_LAZY);
    if (handle == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null handle");
        return false;
    }
    using ExecuteInsightIntentFunc = bool (*)(void*, void*, void*);
    auto func = reinterpret_cast<ExecuteInsightIntentFunc>(dlsym(handle, FUNC_EXECUTE_INSIGHT_INTENT));
    if (func == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null func");
        dlclose(handle);
        return false;
    }
    auto ret = func(&runtime, &executeInfo, &callback);
    dlclose(handle);
    return ret;
}

void CJUIAbility::ExecuteInsightIntentRepeateForeground(const Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (executeParam == nullptr) {
        TAG_LOGW(AAFwkTag::UIABILITY, "invalid param");
        RequestFocus(want);
        CallTriggerCallbackInner(std::move(callback), ERR_OK);
        return;
    }

    auto asyncCallback = [weak = weak_from_this(), want](InsightIntentExecuteResult result) {
        TAG_LOGD(AAFwkTag::UIABILITY, "begin request focus");
        auto ability = weak.lock();
        if (ability == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null ability");
            return;
        }
        ability->RequestFocus(want);
    };
    callback->Push(asyncCallback);

    CJInsightIntentExecutorInfo executeInfo;
    auto ret = GetInsightIntentExecutorInfo(want, executeParam, executeInfo);
    if (!ret) {
        TAG_LOGE(AAFwkTag::UIABILITY, "get intention executor failed");
        CallTriggerCallbackInner(std::move(callback), static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
        return;
    }
    ret = CallExecuteInsightIntent(cjRuntime_, executeInfo, std::move(callback));
    if (!ret) {
        // callback has removed, release in insight intent executor.
        TAG_LOGE(AAFwkTag::UIABILITY, "execute insightIntent failed");
    }
}

void CJUIAbility::ExecuteInsightIntentMoveToForeground(const Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (executeParam == nullptr) {
        TAG_LOGW(AAFwkTag::UIABILITY, "param invalid");
        OnForeground(want);
        CallTriggerCallbackInner(std::move(callback), ERR_OK);
        return;
    }

    UIAbility::OnForeground(want);

    auto asyncCallback = [weak = weak_from_this(), want](InsightIntentExecuteResult result) {
        TAG_LOGD(AAFwkTag::UIABILITY, "begin call onForeground");
        auto ability = weak.lock();
        if (ability == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null ability");
            return;
        }
        ability->CallOnForegroundFunc(want);
    };
    callback->Push(asyncCallback);

    CJInsightIntentExecutorInfo executeInfo;
    auto ret = GetInsightIntentExecutorInfo(want, executeParam, executeInfo);
    if (!ret) {
        TAG_LOGE(AAFwkTag::UIABILITY, "get Intention executor failed");
        CallTriggerCallbackInner(std::move(callback), static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
        return;
    }
    ret = CallExecuteInsightIntent(cjRuntime_, executeInfo, std::move(callback));
    if (!ret) {
        // callback has removed, release in insight intent executor.
        TAG_LOGE(AAFwkTag::UIABILITY, "execute insightIntent failed");
    }
}

void CJUIAbility::ExecuteInsightIntentBackground(const Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback)
{
    TAG_LOGI(AAFwkTag::UIABILITY, "executeInsightIntentBackground");
    if (executeParam == nullptr) {
        TAG_LOGW(AAFwkTag::UIABILITY, "null executePara");
        CallTriggerCallbackInner(std::move(callback), ERR_OK);
        return;
    }

    CJInsightIntentExecutorInfo executeInfo;
    auto ret = GetInsightIntentExecutorInfo(want, executeParam, executeInfo);
    if (!ret) {
        TAG_LOGE(AAFwkTag::UIABILITY, "get intentExecutor failed");
        CallTriggerCallbackInner(std::move(callback), static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
        return;
    }

    ret = CallExecuteInsightIntent(cjRuntime_, executeInfo, std::move(callback));
    if (!ret) {
        // callback has removed, release in insight intent executor.
        TAG_LOGE(AAFwkTag::UIABILITY, "execute insightIntent failed");
    }
}

bool CJUIAbility::GetInsightIntentExecutorInfo(const Want& want,
    const std::shared_ptr<InsightIntentExecuteParam>& executeParam, CJInsightIntentExecutorInfo& executeInfo)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    auto context = GetAbilityContext();
    if (executeParam == nullptr || context == nullptr || abilityInfo_ == nullptr || cjWindowStage_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "invalid param");
        return false;
    }

    const WantParams &wantParams = want.GetParams();
    executeInfo.srcEntry = wantParams.GetStringParam("ohos.insightIntent.srcEntry");
    executeInfo.hapPath = abilityInfo_->hapPath;
    executeInfo.esmodule = abilityInfo_->compileMode == AppExecFwk::CompileMode::ES_MODULE;
    executeInfo.windowMode = windowMode_;
    executeInfo.token = context->GetToken();
    if (cjWindowStage_ != nullptr) {
        executeInfo.pageLoader.windowPageLoader = cjWindowStage_.GetRefPtr();
    }
    executeInfo.executeParam = executeParam;
    return true;
}
#endif
#endif
int32_t CJUIAbility::OnContinue(WantParams &wantParams, bool &isAsyncOnContinue,
    const AppExecFwk::AbilityInfo &abilityInfo)
{
    if (!cjAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAbilityObj_");
        return AppExecFwk::ContinuationManagerStage::OnContinueResult::ON_CONTINUE_ERR;
    }

    CallAppCtxFunc(CjAppCtxFuncType::ON_ABILITY_WILL_CONTINUE, cjAbilityObj_->GetId());
    auto res = cjAbilityObj_->OnContinue(wantParams);
    TAG_LOGD(AAFwkTag::UIABILITY, "end, value: %{public}d", res);

    CallAppCtxFunc(CjAppCtxFuncType::ON_ABILITY_CONTINUE, cjAbilityObj_->GetId());
    return res;
}

int32_t CJUIAbility::OnSaveState(int32_t reason, WantParams &wantParams)
{
    if (!cjAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAbilityObj_");
        return -1;
    }

    CallAppCtxFunc(CjAppCtxFuncType::ON_ABILITY_WILL_SAVE_STATE, cjAbilityObj_->GetId());
    int32_t numberResult = cjAbilityObj_->OnSaveState(reason, wantParams);

    CallAppCtxFunc(CjAppCtxFuncType::ON_ABILITY_SAVE_STATE, cjAbilityObj_->GetId());
    return numberResult;
}

void CJUIAbility::OnConfigurationUpdated(const Configuration &configuration)
{
    UIAbility::OnConfigurationUpdated(configuration);
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    auto fullConfig = GetAbilityContext()->GetConfiguration();
    if (!fullConfig) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null fullConfig");
        return;
    }

    if (!cjAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAbilityObj_");
        return;
    }

    cjAbilityObj_->OnConfigurationUpdated(fullConfig);
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void CJUIAbility::OnMemoryLevel(int level)
{
    UIAbility::OnMemoryLevel(level);
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (!cjAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAbilityObj_");
        return;
    }
    auto memLevel = static_cast<int32_t>(level);
    cjAbilityObj_->OnMemoryLevel(memLevel);
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void CJUIAbility::UpdateContextConfiguration()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
}

void CJUIAbility::OnNewWant(const Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    UIAbility::OnNewWant(want);

#ifdef SUPPORT_GRAPHICS
#ifdef SUPPORT_SCREEN
    if (scene_) {
        scene_->OnNewWant(want);
    }
#endif
#endif
    if (!cjAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAbilityObj_");
        return;
    }

    CallAppCtxFunc(CjAppCtxFuncType::ON_WILL_NEW_WANT, cjAbilityObj_->GetId());
    std::string methodName = "OnNewWant";
    AddLifecycleEventBeforeCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
    cjAbilityObj_->OnNewWant(want, GetLaunchParam());
    AddLifecycleEventAfterCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);

    CallAppCtxFunc(CjAppCtxFuncType::ON_NEW_WANT, cjAbilityObj_->GetId());
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void CJUIAbility::OnAbilityResult(int requestCode, int resultCode, const Want &resultData)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    UIAbility::OnAbilityResult(requestCode, resultCode, resultData);
    if (abilityContext_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityContext_");
        return;
    }
    abilityContext_->OnAbilityResult(requestCode, resultCode, resultData);
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

sptr<IRemoteObject> CJUIAbility::CallRequest()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (cjAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityContext_");
        return nullptr;
    }
    if (remoteCallee_ != nullptr) {
        TAG_LOGD(AAFwkTag::UIABILITY, "some remoteCallee_");
        return remoteCallee_;
    }
    int64_t ret = cjAbilityObj_->OnCallRequest();
    if (ret <= 0) {
        TAG_LOGE(AAFwkTag::UIABILITY, "invalid callee");
        return nullptr;
    }
    remoteCallee_ = SetNewRuleFlagToCallee(ret);
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
    return remoteCallee_;
}

std::shared_ptr<AppExecFwk::ACJDelegatorAbilityProperty> CJUIAbility::CreateADelegatorAbilityProperty()
{
    if (abilityContext_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityContext_");
        return nullptr;
    }
    auto property = std::make_shared<AppExecFwk::ACJDelegatorAbilityProperty>();
    property->token_ = abilityContext_->GetToken();
    property->name_ = GetAbilityName();
    property->moduleName_ = GetModuleName();
    if (GetApplicationInfo() == nullptr || GetApplicationInfo()->bundleName.empty()) {
        property->fullName_ = GetAbilityName();
    } else {
        std::string::size_type pos = GetAbilityName().find(GetApplicationInfo()->bundleName);
        if (pos == std::string::npos || pos != 0) {
            property->fullName_ = GetApplicationInfo()->bundleName + "." + GetAbilityName();
        } else {
            property->fullName_ = GetAbilityName();
        }
    }
    property->lifecycleState_ = GetState();
    property->cjObject_ = cjAbilityObj_->GetId();
    return property;
}

void CJUIAbility::Dump(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    UIAbility::Dump(params, info);
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (!cjAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAbilityObj_");
        return;
    }
    cjAbilityObj_->Dump(params, info);
    TAG_LOGD(AAFwkTag::UIABILITY, "dump, size: %{public}zu", info.size());
}

std::shared_ptr<CJAbilityObject> CJUIAbility::GetCJAbility()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (cjAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAbilityObj_");
    }
    return cjAbilityObj_;
}

sptr<IRemoteObject> CallGetNativeRemoteObject(int64_t remoteCjObjId)
{
    void* handle = dlopen(CJ_IPC_LIBNAME, RTLD_LAZY);
    if (handle == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null handle");
        return nullptr;
    }
    using GetNativeRemoteObjectFunc = void (*)(int64_t, void*);
    auto func = reinterpret_cast<GetNativeRemoteObjectFunc>(dlsym(handle, FUNC_GET_NATIVE_REMOTEOBJECT));
    if (func == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null func");
        dlclose(handle);
        return nullptr;
    }
    sptr<IRemoteObject> remoteObject = nullptr;
    func(remoteCjObjId, &remoteObject);
    dlclose(handle);
    return remoteObject;
}

sptr<IRemoteObject> CJUIAbility::SetNewRuleFlagToCallee(int64_t remoteCjObjId)
{
    if (remoteCjObjId <= 0) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null callee");
        return nullptr;
    }
    auto flag = IsUseNewStartUpRule();
    cjAbilityObj_->OnSetCalleeFlag(flag);
    auto remoteObj = CallGetNativeRemoteObject(remoteCjObjId);
    if (remoteObj == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null remoteObj");
    }
    return remoteObj;
}

bool CJUIAbility::CheckSatisfyTargetAPIVersion(int32_t version)
{
    auto applicationInfo = GetApplicationInfo();
    if (!applicationInfo) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null targetAPIVersion");
        return false;
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "targetAPIVersion: %{public}d", applicationInfo->apiTargetVersion);
    return applicationInfo->apiTargetVersion % API_VERSION_MOD >= version;
}

bool CJUIAbility::BackPressDefaultValue()
{
    return CheckSatisfyTargetAPIVersion(API12) ? true : false;
}
} // namespace AbilityRuntime
} // namespace OHOS
