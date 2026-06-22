/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include <cstdlib>
#include <regex>

#include "system_ability_definition.h"
#include "if_system_ability_manager.h"
#include "ability_delegator_registry.h"
#include "ability_runtime/js_ability.h"

#include "ability_runtime/js_ability_context.h"
#include "ability_start_setting.h"
#include "connection_manager.h"
#include "display_util.h"
#include "hilog_tag_wrapper.h"
#include "js_ability_lifecycle_callback.h"
#include "js_data_struct_converter.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi_common_configuration.h"
#ifdef SUPPORT_SCREEN
#include "js_window_stage.h"
#include "scene_board_judgement.h"
#endif
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "string_wrapper.h"
#include "time_util.h"
#include "context/context.h"
#include "context/application_context.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
#ifdef SUPPORT_SCREEN
const std::string METHOD_NAME = "WindowScene::GoForeground";
#endif

void* DetachNewJsAbilityContext(napi_env, void* nativeObject, void*)
{
    auto* origContext = static_cast<std::weak_ptr<AbilityContext> *>(nativeObject);
    if (origContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "origContext is null");
        return nullptr;
    }
    TAG_LOGD(AAFwkTag::APPKIT, "New detached base context");
    auto* detachNewContext = new (std::nothrow) std::weak_ptr<AbilityContext>(*origContext);
    return detachNewContext;
}

void DetachFinalizeJsAbilityContext(void* detachedObject, void*)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Finalizer detached base context");
    delete static_cast<std::weak_ptr<AbilityContext> *>(detachedObject);
}

napi_value PromiseCallback(napi_env env, napi_callback_info info)
{
    void *data = nullptr;
    NAPI_CALL_NO_THROW(napi_get_cb_info(env, info, nullptr, nullptr, nullptr, &data), nullptr);
    auto *callbackInfo = static_cast<AppExecFwk::AbilityTransactionCallbackInfo<> *>(data);
    callbackInfo->Call();
    AppExecFwk::AbilityTransactionCallbackInfo<>::Destroy(callbackInfo);
    data = nullptr;
    return nullptr;
}
}

napi_value AttachJsAbilityContext(napi_env env, void *value, void *)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (value == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITY, "null value");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<AbilityRuntime::AbilityContext>*>(value)->lock();
    if (ptr == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITY, "null ptr");
        return nullptr;
    }
    napi_value object = CreateJsAbilityContext(env, ptr);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.AbilityContext", &object, 1);
    if (systemModule == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITY, "null systemModule");
        return nullptr;
    }
    auto contextObj = systemModule->GetNapiValue();
    auto workContext = new (std::nothrow) std::weak_ptr<AbilityRuntime::AbilityContext>(ptr);
    auto coerceStatus = napi_coerce_to_native_binding_object(
        env, contextObj, DetachNewJsAbilityContext, AttachJsAbilityContext, workContext, nullptr);
    if (coerceStatus != napi_ok) {
        TAG_LOGE(AAFwkTag::ABILITY, "coerceStatus Failed: %{public}d", coerceStatus);
        delete workContext;
        return nullptr;
    }
    napi_add_detached_finalizer(env, contextObj, DetachFinalizeJsAbilityContext, nullptr);
    napi_status status = napi_wrap(env, contextObj, workContext,
        [](napi_env, void* data, void*) {
            TAG_LOGD(AAFwkTag::ABILITY, "finalizer for weak_ptr ability context is called");
            delete static_cast<std::weak_ptr<AbilityRuntime::AbilityContext> *>(data);
        },
        nullptr, nullptr);
    if (status != napi_ok && workContext != nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "napi_wrap Failed: %{public}d", status);
        delete workContext;
        return nullptr;
    }

    return contextObj;
}

Ability *JsAbility::Create(const std::unique_ptr<Runtime> &runtime)
{
    return new JsAbility(static_cast<JsRuntime &>(*runtime));
}

JsAbility::JsAbility(JsRuntime &jsRuntime) : jsRuntime_(jsRuntime)
{}
JsAbility::~JsAbility()
{
    TAG_LOGD(AAFwkTag::ABILITY, "destructor");
    auto context = GetAbilityContext();
    if (context) {
        context->Unbind();
    }

    jsRuntime_.FreeNativeReference(std::move(jsAbilityObj_));
    jsRuntime_.FreeNativeReference(std::move(shellContextRef_));
#ifdef SUPPORT_SCREEN
    jsRuntime_.FreeNativeReference(std::move(jsWindowStageObj_));
#endif
}

void JsAbility::Init(const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::shared_ptr<OHOSApplication> application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    Ability::Init(abilityInfo, application, handler, token);

    if (!abilityInfo) {
        TAG_LOGE(AAFwkTag::ABILITY, "null abilityInfo");
        return;
    }
    auto srcPath = GenerateSrcPath(abilityInfo);
    if (srcPath.empty()) {
        return;
    }

    std::string moduleName(abilityInfo->moduleName);
    moduleName.append("::").append(abilityInfo->name);

    HandleScope handleScope(jsRuntime_);

    jsAbilityObj_ = jsRuntime_.LoadModule(
        moduleName, srcPath, abilityInfo->hapPath, abilityInfo->compileMode == AppExecFwk::CompileMode::ES_MODULE);
    if (jsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null jsAbilityObj_");
        return;
    }

    BindContext();
}

std::string JsAbility::GenerateSrcPath(std::shared_ptr<AbilityInfo> abilityInfo) const
{
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null abilityInfo");
        return "";
    }

    std::string srcPath(abilityInfo->package);
    if (!abilityInfo->isModuleJson) {
        /* temporary compatibility api8 + config.json */
        srcPath.append("/assets/js/");
        if (!abilityInfo->srcPath.empty()) {
            srcPath.append(abilityInfo->srcPath);
        }
        srcPath.append("/").append(abilityInfo->name).append(".abc");
    } else {
        if (abilityInfo->srcEntrance.empty()) {
            TAG_LOGE(AAFwkTag::ABILITY, "abilityInfo srcEntrance empty");
            return "";
        }
        srcPath.append("/");
        srcPath.append(abilityInfo->srcEntrance);
        srcPath.erase(srcPath.rfind("."));
        srcPath.append(".abc");
        TAG_LOGI(AAFwkTag::ABILITY, "%{public}s", srcPath.c_str());
    }
    return srcPath;
}

void JsAbility::BindContext()
{
    auto env = jsRuntime_.GetNapiEnv();
    AbilityRuntime::HandleScope handleScope(env);
    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::ABILITY, "check type failed");
        return;
    }

    auto context = GetAbilityContext();
    napi_value contextObj = CreateJsAbilityContext(env, context);
    shellContextRef_ = std::shared_ptr<NativeReference>(JsRuntime::LoadSystemModuleByEngine(
        env, "application.AbilityContext", &contextObj, 1).release());
    if (shellContextRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "load module failed");
        return;
    }
    contextObj = shellContextRef_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGE(AAFwkTag::ABILITY, "ability native object failed");
        return;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<AbilityRuntime::AbilityContext>(context);
    auto coerceStatus = napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachJsAbilityContext, workContext, nullptr);
    if (coerceStatus != napi_ok) {
        TAG_LOGE(AAFwkTag::ABILITY, "coerceStatus Failed: %{public}d", coerceStatus);
        delete workContext;
        return;
    }
    context->Bind(jsRuntime_, shellContextRef_.get());
    napi_set_named_property(env, obj, "context", contextObj);
    TAG_LOGD(AAFwkTag::ABILITY, "set ability context");

    napi_status status = napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGD(AAFwkTag::ABILITY, "finalizer for weak_ptr ability context is called");
            delete static_cast<std::weak_ptr<AbilityRuntime::AbilityContext> *>(data);
        },
        nullptr, nullptr);
    if (status != napi_ok && workContext != nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "napi_wrap Failed: %{public}d", status);
        delete workContext;
        return;
    }
}

void JsAbility::OnStart(const Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITY, "begin, ability: %{public}s", GetAbilityName().c_str());
    Ability::OnStart(want, sessionInfo);

    if (!jsAbilityObj_) {
        TAG_LOGW(AAFwkTag::ABILITY, "not found Ability.js");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::ABILITY, "Ability object failed");
        return;
    }

    napi_value jsWant = OHOS::AppExecFwk::WrapWant(env, want);
    if (jsWant == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null jsWant");
        return;
    }

    napi_set_named_property(env, obj, "launchWant", jsWant);
    napi_set_named_property(env, obj, "lastRequestWant", jsWant);

    napi_value argv[] = {
        jsWant,
        CreateJsLaunchParam(env, GetLaunchParam()),
    };
    std::string methodName = "OnStart";
    AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
    CallObjectMethod("onCreate", argv, ArraySize(argv));
    AddLifecycleEventAfterJSCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        TAG_LOGD(AAFwkTag::ABILITY, "call PostPerformStart");
        delegator->PostPerformStart(CreateADelegatorAbilityProperty());
    }
    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        JsAbilityLifecycleCallbackArgs ability(jsAbilityObj_);
        applicationContext->DispatchOnAbilityCreate(ability);
    }
    TAG_LOGD(AAFwkTag::ABILITY, "end, ability:%{public}s", GetAbilityName().c_str());
}

void JsAbility::AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState state, const std::string &methodName) const
{
    auto entry = std::string("JsAbility::") + methodName + " begin";
    FreezeUtil::GetInstance().AddLifecycleEvent(AbilityContext::token_, entry);
}

void JsAbility::AddLifecycleEventAfterJSCall(FreezeUtil::TimeoutState state, const std::string &methodName) const
{
    auto entry = std::string("JsAbility::") + methodName + " end";
    FreezeUtil::GetInstance().AddLifecycleEvent(AbilityContext::token_, entry);
}

int32_t JsAbility::OnShare(WantParams &wantParam)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    if (jsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null jsAbilityObj_");
        return ERR_INVALID_VALUE;
    }
    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::ABILITY, "Ability object failed");
        return ERR_INVALID_VALUE;
    }

    napi_value jsWantParams = OHOS::AppExecFwk::WrapWantParams(env, wantParam);
    napi_value argv[] = {
        jsWantParams,
    };
    CallObjectMethod("onShare", argv, ArraySize(argv));
    OHOS::AppExecFwk::UnwrapWantParams(env, jsWantParams, wantParam);
    return ERR_OK;
}

void JsAbility::OnStop()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (abilityContext_) {
        TAG_LOGD(AAFwkTag::ABILITY, "set terminating true");
        abilityContext_->SetTerminating(true);
    }
    Ability::OnStop();
    HandleScope handleScope(jsRuntime_);
    CallObjectMethod("onDestroy");
    OnStopCallback();
}

void JsAbility::OnStop(AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback)
{
    if (callbackInfo == nullptr) {
        isAsyncCallback = false;
        OnStop();
        return;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (abilityContext_) {
        TAG_LOGD(AAFwkTag::ABILITY, "set terminating true");
        abilityContext_->SetTerminating(true);
    }

    Ability::OnStop();

    HandleScope handleScope(jsRuntime_);
    napi_value result = CallObjectMethod("onDestroy", nullptr, 0, true);
    if (!CheckPromise(result)) {
        OnStopCallback();
        isAsyncCallback = false;
        return;
    }

    std::weak_ptr<Ability> weakPtr = shared_from_this();
    auto asyncCallback = [abilityWeakPtr = weakPtr]() {
        auto ability = abilityWeakPtr.lock();
        if (ability == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITY, "null ability");
            return;
        }
        ability->OnStopCallback();
    };
    callbackInfo->Push(asyncCallback);
    isAsyncCallback = CallPromise(result, callbackInfo);
    if (!isAsyncCallback) {
        TAG_LOGE(AAFwkTag::ABILITY, "call promise");
        OnStopCallback();
    }
}

void JsAbility::OnStopCallback()
{
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        TAG_LOGD(AAFwkTag::ABILITY, "call PostPerformStop");
        delegator->PostPerformStop(CreateADelegatorAbilityProperty());
    }

    bool ret = ConnectionManager::GetInstance().DisconnectCaller(AbilityContext::token_);
    if (ret) {
        ConnectionManager::GetInstance().ReportConnectionLeakEvent(getpid(), gettid());
        TAG_LOGD(AAFwkTag::ABILITY, "service connection not disconnected");
    }

    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        JsAbilityLifecycleCallbackArgs ability(jsAbilityObj_);
        applicationContext->DispatchOnAbilityDestroy(ability);
    }
}

#ifdef SUPPORT_SCREEN
const std::string PAGE_STACK_PROPERTY_NAME = "pageStack";
const std::string SUPPORT_CONTINUE_PAGE_STACK_PROPERTY_NAME = "ohos.extra.param.key.supportContinuePageStack";

void JsAbility::OnSceneCreated()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITY, "ability: %{public}s", GetAbilityName().c_str());
    Ability::OnSceneCreated();
    auto jsAppWindowStage = CreateAppWindowStage();
    if (jsAppWindowStage == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null jsAppWindowStage");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    napi_value argv[] = {jsAppWindowStage->GetNapiValue()};
    {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "onWindowStageCreate");
        std::string methodName = "OnSceneCreated";
        AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
        CallObjectMethod("onWindowStageCreate", argv, ArraySize(argv));
        AddLifecycleEventAfterJSCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
    }

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        TAG_LOGD(AAFwkTag::ABILITY, "call PostPerformScenceCreated");
        delegator->PostPerformScenceCreated(CreateADelegatorAbilityProperty());
    }

    jsWindowStageObj_ = std::shared_ptr<NativeReference>(jsAppWindowStage.release());
    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        JsAbilityLifecycleCallbackArgs ability(jsAbilityObj_);
        JsAbilityLifecycleCallbackArgs stage(jsWindowStageObj_);
        applicationContext->DispatchOnWindowStageCreate(ability, stage);
    }

    TAG_LOGD(AAFwkTag::ABILITY, "end, ability:%{public}s", GetAbilityName().c_str());
}

void JsAbility::OnSceneRestored()
{
    Ability::OnSceneRestored();
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    HandleScope handleScope(jsRuntime_);
    auto jsAppWindowStage = CreateAppWindowStage();
    if (jsAppWindowStage == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null jsAppWindowStage");
        return;
    }
    napi_value argv[] = {jsAppWindowStage->GetNapiValue()};
    CallObjectMethod("onWindowStageRestore", argv, ArraySize(argv));

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        TAG_LOGD(AAFwkTag::ABILITY, "call PostPerformScenceRestored");
        delegator->PostPerformScenceRestored(CreateADelegatorAbilityProperty());
    }

    jsWindowStageObj_ = std::shared_ptr<NativeReference>(jsAppWindowStage.release());
}

void JsAbility::onSceneDestroyed()
{
    TAG_LOGD(AAFwkTag::ABILITY, "begin, ability:%{public}s", GetAbilityName().c_str());
    Ability::onSceneDestroyed();
    HandleScope handleScope(jsRuntime_);
    CallObjectMethod("onWindowStageDestroy");

    if (scene_ != nullptr) {
        auto window = scene_->GetMainWindow();
        if (window != nullptr) {
            TAG_LOGD(AAFwkTag::ABILITY, "call UnregisterDisplayMoveListener");
            window->UnregisterDisplayMoveListener(abilityDisplayMoveListener_);
        }
    }

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        TAG_LOGD(AAFwkTag::ABILITY, "call PostPerformScenceDestroyed");
        delegator->PostPerformScenceDestroyed(CreateADelegatorAbilityProperty());
    }

    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        JsAbilityLifecycleCallbackArgs ability(jsAbilityObj_);
        JsAbilityLifecycleCallbackArgs stage(jsWindowStageObj_);
        applicationContext->DispatchOnWindowStageDestroy(ability, stage);
    }
    TAG_LOGD(AAFwkTag::ABILITY, "end, ability:%{public}s", GetAbilityName().c_str());
}

void JsAbility::OnForeground(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITY, "begin, ability:%{public}s", GetAbilityName().c_str());
    if (abilityInfo_) {
        jsRuntime_.UpdateModuleNameAndAssetPath(abilityInfo_->moduleName);
    }

    Ability::OnForeground(want);

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    if (jsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null jsAbilityObj_");
        return;
    }
    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::ABILITY, "Ability object failed");
        return;
    }

    napi_value jsWant = OHOS::AppExecFwk::WrapWant(env, want);
    if (jsWant == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null jsWant");
        return;
    }

    napi_set_named_property(env, obj, "lastRequestWant", jsWant);
    std::string methodName = "OnForeground";
    AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
    CallObjectMethod("onForeground", &jsWant, 1);
    AddLifecycleEventAfterJSCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        TAG_LOGD(AAFwkTag::ABILITY, "call PostPerformForeground");
        delegator->PostPerformForeground(CreateADelegatorAbilityProperty());
    }

    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        JsAbilityLifecycleCallbackArgs ability(jsAbilityObj_);
        applicationContext->DispatchOnAbilityForeground(ability);
    }
    TAG_LOGD(AAFwkTag::ABILITY, "end, ability:%{public}s", GetAbilityName().c_str());
}

void JsAbility::OnBackground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITY, "begin, ability:%{public}s", GetAbilityName().c_str());
    std::string methodName = "OnBackground";
    HandleScope handleScope(jsRuntime_);
    AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState::BACKGROUND, methodName);
    CallObjectMethod("onBackground");
    AddLifecycleEventAfterJSCall(FreezeUtil::TimeoutState::BACKGROUND, methodName);

    Ability::OnBackground();

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        TAG_LOGD(AAFwkTag::ABILITY, "call PostPerformBackground");
        delegator->PostPerformBackground(CreateADelegatorAbilityProperty());
    }

    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        JsAbilityLifecycleCallbackArgs ability(jsAbilityObj_);
        applicationContext->DispatchOnAbilityBackground(ability);
    }
    TAG_LOGD(AAFwkTag::ABILITY, "end, ability:%{public}s", GetAbilityName().c_str());
}

bool JsAbility::OnBackPress()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITY, "call, ability:%{public}s", GetAbilityName().c_str());
    Ability::OnBackPress();
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    napi_value jsValue = CallObjectMethod("onBackPressed", nullptr, 0, true);
    bool ret = false;
    if (!ConvertFromJsValue(env, jsValue, ret)) {
        TAG_LOGW(AAFwkTag::ABILITY, "get js value failed");
        return false;
    }
    TAG_LOGD(AAFwkTag::ABILITY, "end, ret:%{public}d", ret);
    return ret;
}

bool JsAbility::OnPrepareTerminate()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::ABILITY, "prepare terminate call, ability: %{public}s", GetAbilityName().c_str());
    Ability::OnPrepareTerminate();
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    napi_value jsValue = CallObjectMethod("onPrepareToTerminate", nullptr, 0, true);
    bool ret = false;
    if (!ConvertFromJsValue(env, jsValue, ret)) {
        TAG_LOGW(AAFwkTag::ABILITY, "get js value failed");
        return false;
    }
    return ret;
}

std::unique_ptr<NativeReference> JsAbility::CreateAppWindowStage()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    napi_value jsWindowStage = Rosen::CreateJsWindowStage(env, GetScene());
    if (jsWindowStage == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null jsWindowStage");
        return nullptr;
    }
    return JsRuntime::LoadSystemModuleByEngine(env, "application.WindowStage", &jsWindowStage, 1);
}

void JsAbility::GetPageStackFromWant(const Want &want, std::string &pageStack)
{
    auto stringObj = AAFwk::IString::Query(want.GetParams().GetParam(PAGE_STACK_PROPERTY_NAME));
    if (stringObj != nullptr) {
        pageStack = AAFwk::String::Unbox(stringObj);
    }
}

bool JsAbility::IsRestorePageStack(const Want &want)
{
    return want.GetBoolParam(SUPPORT_CONTINUE_PAGE_STACK_PROPERTY_NAME, true);
}

void JsAbility::RestorePageStack(const Want &want)
{
    if (IsRestorePageStack(want)) {
        std::string pageStack;
        GetPageStackFromWant(want, pageStack);
        HandleScope handleScope(jsRuntime_);
        auto env = jsRuntime_.GetNapiEnv();
        if (abilityContext_->GetContentStorage()) {
            scene_->GetMainWindow()->NapiSetUIContent(pageStack, env,
                abilityContext_->GetContentStorage()->GetNapiValue(), Rosen::BackupAndRestoreType::CONTINUATION);
        } else {
            TAG_LOGE(AAFwkTag::ABILITY, "null contentStorage");
        }
    }
}

void JsAbility::AbilityContinuationOrRecover(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    // multi-instance ability continuation
    TAG_LOGD(AAFwkTag::ABILITY, "launch reason = %{public}d", launchParam_.launchReason);
    if (IsRestoredInContinuation()) {
        RestorePageStack(want);
        OnSceneRestored();
        NotifyContinuationResult(want, true);
    } else if (ShouldRecoverState(want)) {
        OnSceneRestored();
    } else {
        OnSceneCreated();
    }
}

void JsAbility::DoOnForeground(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (scene_ == nullptr) {
        if (!InitWindowScene(want)) {
            return;
        }
        AbilityContinuationOrRecover(want);
        auto window = scene_->GetMainWindow();
        if (window) {
            TAG_LOGD(AAFwkTag::ABILITY, "call RegisterDisplayMoveListener, windowId: %{public}d",
                window->GetWindowId());
            abilityDisplayMoveListener_ = new AbilityDisplayMoveListener(weak_from_this());
            window->RegisterDisplayMoveListener(abilityDisplayMoveListener_);
        }
    } else {
        auto window = scene_->GetMainWindow();
        if (window != nullptr && want.HasParameter(Want::PARAM_RESV_WINDOW_MODE)) {
            auto windowMode = want.GetIntParam(Want::PARAM_RESV_WINDOW_MODE,
                AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED);
            window->SetWindowMode(static_cast<Rosen::WindowMode>(windowMode));
            TAG_LOGD(AAFwkTag::ABILITY, "set window mode:%{public}d", windowMode);
        }
    }

    auto window = scene_->GetMainWindow();
    if (window != nullptr && securityFlag_) {
        window->SetSystemPrivacyMode(true);
    }

    TAG_LOGI(AAFwkTag::ABILITY, "move scene to foreground, sceneFlag_:%{public}d", Ability::sceneFlag_);
    AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState::FOREGROUND, METHOD_NAME);
    scene_->GoForeground(Ability::sceneFlag_);
    TAG_LOGD(AAFwkTag::ABILITY, "end");
}

bool JsAbility::InitWindowScene(const Want &want)
{
    if ((abilityContext_ == nullptr) || (sceneListener_ == nullptr)) {
        TAG_LOGE(AAFwkTag::ABILITY, "null abilityContext_ or sceneListener_");
        return false;
    }
    scene_ = std::make_shared<Rosen::WindowScene>();
    int32_t displayId = AAFwk::DisplayUtil::GetDefaultDisplayId();
    if (setting_ != nullptr) {
        std::string strDisplayId =
            setting_->GetProperty(OHOS::AppExecFwk::AbilityStartSetting::WINDOW_DISPLAY_ID_KEY);
        std::regex formatRegex("[0-9]{0,9}$");
        std::smatch sm;
        bool flag = std::regex_match(strDisplayId, sm, formatRegex);
        if (flag && !strDisplayId.empty()) {
            int base = 10; // Numerical base (radix) that determines the valid characters and their interpretation.
            displayId = strtol(strDisplayId.c_str(), nullptr, base);
            TAG_LOGD(AAFwkTag::ABILITY, "displayId:%{public}d", displayId);
        } else {
            TAG_LOGW(AAFwkTag::ABILITY, "formatRegex:[%{public}s]", strDisplayId.c_str());
        }
    }
    auto option = GetWindowOption(want);
    Rosen::WMError ret = Rosen::WMError::WM_OK;
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto sessionToken = GetSessionToken();
        if (sessionToken == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITY, "null sessionToken");
            return false;
        }
        abilityContext_->SetWeakSessionToken(sessionToken);
        ret = scene_->Init(displayId, abilityContext_, sceneListener_, option, sessionToken);
    } else {
        ret = scene_->Init(displayId, abilityContext_, sceneListener_, option);
    }
    if (ret != Rosen::WMError::WM_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "init window scene");
        return false;
    }
    return true;
}

void JsAbility::RequestFocus(const Want &want)
{
    TAG_LOGI(AAFwkTag::ABILITY, "begin");
    if (scene_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null scene_");
        return;
    }
    auto window = scene_->GetMainWindow();
    if (window != nullptr && want.HasParameter(Want::PARAM_RESV_WINDOW_MODE)) {
        auto windowMode = want.GetIntParam(Want::PARAM_RESV_WINDOW_MODE,
            AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED);
        window->SetWindowMode(static_cast<Rosen::WindowMode>(windowMode));
        TAG_LOGD(AAFwkTag::ABILITY, "set window mode = %{public}d", windowMode);
    }
    AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState::FOREGROUND, METHOD_NAME);
    scene_->GoForeground(Ability::sceneFlag_);
}

void JsAbility::ContinuationRestore(const Want &want)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (!IsRestoredInContinuation() || scene_ == nullptr) {
        return;
    }
    RestorePageStack(want);
    OnSceneRestored();
    NotifyContinuationResult(want, true);
}

std::shared_ptr<NativeReference> JsAbility::GetJsWindowStage()
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (jsWindowStageObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null jsWindowStage");
    }
    return jsWindowStageObj_;
}

const JsRuntime& JsAbility::GetJsRuntime()
{
    return jsRuntime_;
}

#endif

int32_t JsAbility::OnContinue(WantParams &wantParams)
{
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    if (jsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null jsAbilityObj_");
        return AppExecFwk::ContinuationManager::OnContinueResult::ON_CONTINUE_ERR;
    }
    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::ABILITY, "Ability object");
        return AppExecFwk::ContinuationManager::OnContinueResult::ON_CONTINUE_ERR;
    }

    napi_value methodOnCreate = nullptr;
    napi_get_named_property(env, obj, "onContinue", &methodOnCreate);
    if (methodOnCreate == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null methodOnCreate");
        return AppExecFwk::ContinuationManager::OnContinueResult::ON_CONTINUE_ERR;
    }

    napi_value jsWantParams = OHOS::AppExecFwk::WrapWantParams(env, wantParams);
    napi_value result = nullptr;
    napi_call_function(env, obj, methodOnCreate, 1, &jsWantParams, &result);

    OHOS::AppExecFwk::UnwrapWantParams(env, jsWantParams, wantParams);

    int32_t numberResult = 0;
    if (!ConvertFromJsValue(env, result, numberResult)) {
        TAG_LOGE(AAFwkTag::ABILITY, "'onContinue' not implemented");
        return AppExecFwk::ContinuationManager::OnContinueResult::ON_CONTINUE_ERR;
    }

    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        JsAbilityLifecycleCallbackArgs ability(jsAbilityObj_);
        applicationContext->DispatchOnAbilityContinue(ability);
    }

    return numberResult;
}

int32_t JsAbility::OnSaveState(int32_t reason, WantParams &wantParams)
{
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    if (jsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null jsAbilityObj_");
        return -1;
    }
    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::ABILITY, "appRecovery get Ability object");
        return -1;
    }

    napi_value methodOnSaveState = nullptr;
    napi_get_named_property(env, obj, "onSaveState", &methodOnSaveState);
    if (methodOnSaveState == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null methodOnSaveState");
        return -1;
    }

    napi_value jsWantParams = OHOS::AppExecFwk::WrapWantParams(env, wantParams);
    napi_value jsReason = CreateJsValue(env, reason);
    napi_value args[] = { jsReason, jsWantParams };
    napi_value result = nullptr;
    napi_call_function(env, obj, methodOnSaveState, 2, args, &result); // 2:args size
    OHOS::AppExecFwk::UnwrapWantParams(env, jsWantParams, wantParams);

    int32_t numberResult = 0;
    if (!ConvertFromJsValue(env, result, numberResult)) {
        TAG_LOGE(AAFwkTag::ABILITY, "appRecovery no result");
        return -1;
    }
    return numberResult;
}

void JsAbility::OnConfigurationUpdated(const Configuration &configuration)
{
    Ability::OnConfigurationUpdated(configuration);
    TAG_LOGD(AAFwkTag::ABILITY, "called");

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    auto fullConfig = GetAbilityContext()->GetConfiguration();
    if (!fullConfig) {
        TAG_LOGE(AAFwkTag::ABILITY, "configuration");
        return;
    }

    napi_value napiConfiguration = OHOS::AppExecFwk::WrapConfiguration(env, configuration);
    CallObjectMethod("onConfigurationUpdated", &napiConfiguration, 1);
    CallObjectMethod("onConfigurationUpdate", &napiConfiguration, 1);
    if (shellContextRef_ != nullptr) {
        JsAbilityContext::ConfigurationUpdated(env, shellContextRef_->GetNapiValue(), fullConfig);
    }
}

void JsAbility::OnMemoryLevel(int level)
{
    Ability::OnMemoryLevel(level);
    TAG_LOGD(AAFwkTag::ABILITY, "called");

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    if (jsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null jsAbilityObj_");
        return;
    }
    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::ABILITY, "Ability object failed");
        return;
    }

    napi_value jslevel = CreateJsValue(env, level);
    napi_value argv[] = {
        jslevel,
    };
    CallObjectMethod("onMemoryLevel", argv, ArraySize(argv));
}

void JsAbility::UpdateContextConfiguration()
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    if (shellContextRef_ != nullptr) {
        JsAbilityContext::ConfigurationUpdated(env, shellContextRef_->GetNapiValue(),
            GetAbilityContext()->GetConfiguration());
    }
}

void JsAbility::OnNewWant(const Want &want)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    Ability::OnNewWant(want);

#ifdef SUPPORT_SCREEN
    if (scene_) {
        scene_->OnNewWant(want);
    }
#endif

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    if (jsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null jsAbilityObj_");
        return;
    }
    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::ABILITY, "Ability object failed");
        return;
    }

    napi_value jsWant = OHOS::AppExecFwk::WrapWant(env, want);
    if (jsWant == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null want");
        return;
    }

    napi_set_named_property(env, obj, "lastRequestWant", jsWant);

    napi_value argv[] = {
        jsWant,
        CreateJsLaunchParam(env, GetLaunchParam()),
    };
    std::string methodName = "OnNewWant";
    AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
    CallObjectMethod("onNewWant", argv, ArraySize(argv));
    AddLifecycleEventAfterJSCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);

    TAG_LOGD(AAFwkTag::ABILITY, "end");
}

void JsAbility::OnAbilityResult(int requestCode, int resultCode, const Want &resultData)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    Ability::OnAbilityResult(requestCode, resultCode, resultData);
    std::shared_ptr<AbilityRuntime::AbilityContext> context = GetAbilityContext();
    if (context == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITY, "null context");
        return;
    }
    context->OnAbilityResult(requestCode, resultCode, resultData);
    TAG_LOGD(AAFwkTag::ABILITY, "end");
}

sptr<IRemoteObject> JsAbility::CallRequest()
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (jsAbilityObj_ == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITY, "null Obj");
        return nullptr;
    }

    if (remoteCallee_ != nullptr) {
        TAG_LOGD(AAFwkTag::ABILITY, "get Callee remoteObj");
        return remoteCallee_;
    }

    HandleScope handleScope(jsRuntime_);
    TAG_LOGD(AAFwkTag::ABILITY, "set runtime scope");
    auto env = jsRuntime_.GetNapiEnv();
    auto obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::ABILITY, "object failed");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, "onCallRequest", &method);
    bool isCallable = false;
    napi_is_callable(env, method, &isCallable);
    if (!isCallable) {
        TAG_LOGE(AAFwkTag::ABILITY, "method:%{public}s",
            method == nullptr ? "nullptr" : "not func");
        return nullptr;
    }

    napi_value remoteJsObj = nullptr;
    napi_call_function(env, obj, method, 0, nullptr, &remoteJsObj);
    if (remoteJsObj == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null remoteJsObj");
        return nullptr;
    }

    remoteCallee_ = SetNewRuleFlagToCallee(env, remoteJsObj);
    TAG_LOGD(AAFwkTag::ABILITY, "end");
    return remoteCallee_;
}

napi_value JsAbility::CallObjectMethod(const char *name, napi_value const *argv, size_t argc, bool withResult)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::ABILITY, "begin:%{public}s", name);

    if (!jsAbilityObj_) {
        TAG_LOGW(AAFwkTag::ABILITY, "not found Ability.js");
        return nullptr;
    }

    HandleEscape handleEscape(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::ABILITY, "Ability object failed");
        return nullptr;
    }

    napi_value methodOnCreate = nullptr;
    napi_get_named_property(env, obj, name, &methodOnCreate);
    if (methodOnCreate == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null methodOnCreate");
        return nullptr;
    }
    if (withResult) {
        napi_value result = nullptr;
        napi_call_function(env, obj, methodOnCreate, argc, argv, &result);
        return handleEscape.Escape(result);
    }
    napi_call_function(env, obj, methodOnCreate, argc, argv, nullptr);
    TAG_LOGI(AAFwkTag::ABILITY, "end:%{public}s", name);
    return nullptr;
}

bool JsAbility::CheckPromise(napi_value result)
{
    if (result == nullptr) {
        TAG_LOGD(AAFwkTag::ABILITY, "null result");
        return false;
    }
    auto env = jsRuntime_.GetNapiEnv();
    bool isPromise = false;
    napi_is_promise(env, result, &isPromise);
    if (!isPromise) {
        TAG_LOGD(AAFwkTag::ABILITY, "result not promise");
        return false;
    }
    return true;
}

bool JsAbility::CallPromise(napi_value result, AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo)
{
    auto env = jsRuntime_.GetNapiEnv();
    if (!CheckTypeForNapiValue(env, result, napi_object)) {
        TAG_LOGE(AAFwkTag::ABILITY, "convert failed");
        return false;
    }
    napi_value then = nullptr;
    napi_get_named_property(env, result, "then", &then);
    if (then == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null then");
        return false;
    }
    bool isCallable = false;
    napi_is_callable(env, then, &isCallable);
    if (!isCallable) {
        TAG_LOGE(AAFwkTag::ABILITY, "property not callable");
        return false;
    }
    HandleScope handleScope(jsRuntime_);
    napi_value promiseCallback = nullptr;
    napi_create_function(env, "promiseCallback", strlen("promiseCallback"), PromiseCallback,
        callbackInfo, &promiseCallback);
    napi_value argv[1] = { promiseCallback };
    napi_call_function(env, result, then, 1, argv, nullptr);
    TAG_LOGD(AAFwkTag::ABILITY, "callPromise complete");
    return true;
}

std::shared_ptr<AppExecFwk::ADelegatorAbilityProperty> JsAbility::CreateADelegatorAbilityProperty()
{
    auto property = std::make_shared<AppExecFwk::ADelegatorAbilityProperty>();
    property->token_          = GetAbilityContext()->GetToken();
    property->name_           = GetAbilityName();
    property->moduleName_     = GetModuleName();
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
    property->object_         = jsAbilityObj_;

    return property;
}

void JsAbility::Dump(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    Ability::Dump(params, info);
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    HandleScope handleScope(jsRuntime_);

    if (!jsAbilityObj_) {
        TAG_LOGW(AAFwkTag::ABILITY, "not found .js");
        return;
    }

    auto env = jsRuntime_.GetNapiEnv();
    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::ABILITY, "get object failed");
        return;
    }

    if (!AddDumpInfo(env, obj, params, info, "dump")) {
        return;
    }
    if (!AddDumpInfo(env, obj, params, info, "onDump")) {
        return;
    }

    TAG_LOGD(AAFwkTag::ABILITY, "dump info size:%{public}zu", info.size());
}

bool JsAbility::AddDumpInfo(napi_env env, napi_value obj, const std::vector<std::string> &params,
    std::vector<std::string> &info, const std::string &methodName) const
{
    // create js array object of params
    napi_value argv[] = { CreateNativeArray(env, params) };

    napi_value method = nullptr;
    napi_get_named_property(env, obj, methodName.c_str(), &method);

    napi_value dumpInfo = nullptr;
    if (method != nullptr) {
        napi_call_function(env, obj, method, 1, argv, &dumpInfo);
    }

    if (dumpInfo == nullptr) {
        uint32_t len = 0;
        napi_get_array_length(env, dumpInfo, &len);
        for (uint32_t i = 0; i < len; i++) {
            std::string dumpInfoStr;
            napi_value element = nullptr;
            napi_get_element(env, dumpInfo, i, &element);
            if (!ConvertFromJsValue(env, element, dumpInfoStr)) {
                TAG_LOGE(AAFwkTag::ABILITY, "parse dumpInfoStr failed");
                return false;
            }
            info.push_back(dumpInfoStr);
        }
    }
    return true;
}

std::shared_ptr<NativeReference> JsAbility::GetJsAbility()
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (jsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null jsAbilityObj_");
    }
    return jsAbilityObj_;
}

sptr<IRemoteObject> JsAbility::SetNewRuleFlagToCallee(napi_env env, napi_value remoteJsObj)
{
    if (!CheckTypeForNapiValue(env, remoteJsObj, napi_object)) {
        TAG_LOGE(AAFwkTag::ABILITY, "null calleeObj");
        return nullptr;
    }
    napi_value setFlagMethod = nullptr;
    napi_get_named_property(env, remoteJsObj, "setNewRuleFlag", &setFlagMethod);
    bool isCallable = false;
    napi_is_callable(env, setFlagMethod, &isCallable);
    if (!isCallable) {
        TAG_LOGE(AAFwkTag::ABILITY, "setFlagMethod:%{public}s",
            setFlagMethod == nullptr ? "nullptr" : "not func");
        return nullptr;
    }
    auto flag = CreateJsValue(env, IsUseNewStartUpRule());
    napi_value argv[1] = { flag };
    napi_call_function(env, remoteJsObj, setFlagMethod, 1, argv, nullptr);

    auto remoteObj = NAPI_ohos_rpc_getNativeRemoteObject(env, remoteJsObj);
    if (remoteObj == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null remoteObj");
        return nullptr;
    }
    return remoteObj;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
