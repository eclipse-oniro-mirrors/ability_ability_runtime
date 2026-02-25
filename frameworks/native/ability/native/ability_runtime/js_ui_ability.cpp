/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#include "js_ui_ability.h"

#include <cstdlib>
#include <regex>

#include "ability_business_error.h"
#include "ability_delegator_registry.h"
#include "ability_manager_client.h"
#include "ability_recovery.h"
#include "ability_start_setting.h"
#include "app_recovery.h"
#include "bool_wrapper.h"
#include "connection_manager.h"
#include "context/application_context.h"
#include "context/context.h"
#include "display_util.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "if_system_ability_manager.h"
#include "insight_intent_delay_result_callback_mgr.h"
#include "insight_intent_executor_info.h"
#include "insight_intent_executor_mgr.h"
#include "insight_intent_execute_param.h"
#include "interop_object_instance.h"
#include "js_ability_context.h"
#include "js_ability_lifecycle_callback.h"
#include "js_data_struct_converter.h"
#include "js_insight_intent_page.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "js_utils.h"
#ifdef SUPPORT_SCREEN
#include "js_window_stage.h"
#include "scene_board_judgement.h"
#endif
#include "ohos_application.h"
#include "napi_common_configuration.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "page_switch_log.h"
#include "string_wrapper.h"
#include "system_ability_definition.h"
#include "time_util.h"
#include "madvise/madvise_utils.h"
#ifdef ENABLE_ABILITY_MADVISE
#include "system_parameter.h"
#endif

namespace OHOS {
namespace AbilityRuntime {
namespace {
#ifdef SUPPORT_GRAPHICS
const std::string PAGE_STACK_PROPERTY_NAME = "pageStack";
const std::string SUPPORT_CONTINUE_PAGE_STACK_PROPERTY_NAME = "ohos.extra.param.key.supportContinuePageStack";
const std::string METHOD_NAME = "WindowScene::GoForeground";
#endif
// Numerical base (radix) that determines the valid characters and their interpretation.
#ifdef SUPPORT_SCREEN
const int32_t BASE_DISPLAY_ID_NUM (10);
#endif
constexpr const char* REUSING_WINDOW = "ohos.ability_runtime.reusing_window";
constexpr const int32_t API12 = 12;
constexpr const int32_t API_VERSION_MOD = 100;
constexpr const int32_t PROMISE_CALLBACK_PARAM_NUM = 2;
constexpr const int32_t CALL_BACK_ERROR = -1;
#ifdef SUPPORT_GRAPHICS
const int32_t CONTINUE_GET_CONTENT_FAILED = 29360200;
#endif

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

napi_value OnContinuePromiseCallback(napi_env env, napi_callback_info info)
{
    void *data = nullptr;
    size_t argc = 1;
    napi_value argv = {nullptr};
    NAPI_CALL_NO_THROW(napi_get_cb_info(env, info, &argc, &argv, nullptr, &data), nullptr);
    int32_t onContinueRes = 0;
    if (!ConvertFromJsValue(env, argv, onContinueRes)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "get value failed");
        onContinueRes = AppExecFwk::ContinuationManagerStage::OnContinueResult::ON_CONTINUE_ERR;
    }
    auto *callbackInfo = static_cast<AppExecFwk::AbilityTransactionCallbackInfo<int32_t> *>(data);
    callbackInfo->Call(onContinueRes);
    data = nullptr;

    return nullptr;
}

napi_value OnPrepareTerminatePromiseCallback(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::UIABILITY, "OnPrepareTerminatePromiseCallback begin");
    void *data = nullptr;
    size_t argc = ARGC_MAX_COUNT;
    napi_value argv[ARGC_MAX_COUNT] = {nullptr};
    NAPI_CALL_NO_THROW(napi_get_cb_info(env, info, &argc, argv, nullptr, &data), nullptr);
    auto *callbackInfo = static_cast<AppExecFwk::AbilityTransactionCallbackInfo<bool> *>(data);
    bool prepareTermination = false;
    if (callbackInfo == nullptr || (argc > 0 && !ConvertFromJsValue(env, argv[0], prepareTermination))) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null callbackInfo or unwrap prepareTermination result failed");
        return nullptr;
    }
    callbackInfo->Call(prepareTermination);
    AppExecFwk::AbilityTransactionCallbackInfo<bool>::Destroy(callbackInfo);
    data = nullptr;
    TAG_LOGI(AAFwkTag::UIABILITY, "OnPrepareTerminatePromiseCallback end");
    return nullptr;
}

napi_value OnSaveStateCallback(napi_env env, napi_callback_info info)
{
    void *data = nullptr;
    size_t argc = ARGC_MAX_COUNT;
    napi_value argv[ARGC_MAX_COUNT] = {nullptr};
    NAPI_CALL_NO_THROW(napi_get_cb_info(env, info, &argc, argv, nullptr, &data), nullptr);
    auto callInfo = static_cast<CallOnSaveStateInfo *>(data);
    if (callInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null info");
        return nullptr;
    }
    int32_t status = 0;
    if (callInfo->callbackInfo == nullptr || (argc > 0 && !ConvertFromJsValue(env, argv[0], status))) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null callbackInfo or unwrap onSaveState result failed");
        return nullptr;
    }
    AppExecFwk::OnSaveStateResult saveStateResult = {status, callInfo->wantParams, callInfo->reason};
    callInfo->callbackInfo->Call(saveStateResult);
    AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnSaveStateResult>::Destroy(callInfo->callbackInfo);
    data = nullptr;
    return nullptr;
}

napi_value AttachJsAbilityContext(napi_env env, void *value, void *)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (value == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "invalid params");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<AbilityRuntime::AbilityContext> *>(value)->lock();
    if (ptr == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ptr");
        return nullptr;
    }
    std::shared_ptr<NativeReference> systemModule = nullptr;
    int32_t screenMode = ptr->GetScreenMode();
    if (screenMode == AAFwk::IDLE_SCREEN_MODE) {
        auto uiAbiObject = CreateJsAbilityContext(env, ptr);
        CHECK_POINTER_AND_RETURN(uiAbiObject, nullptr);
        systemModule = std::shared_ptr<NativeReference>(JsRuntime::LoadSystemModuleByEngine(env,
            "application.AbilityContext", &uiAbiObject, 1).release());
    } else {
        auto emUIObject = JsEmbeddableUIAbilityContext::CreateJsEmbeddableUIAbilityContext(env,
            ptr, nullptr, screenMode);
        CHECK_POINTER_AND_RETURN(emUIObject, nullptr);
        systemModule = std::shared_ptr<NativeReference>(JsRuntime::LoadSystemModuleByEngine(env,
            "application.EmbeddableUIAbilityContext", &emUIObject, 1).release());
    }
    CHECK_POINTER_AND_RETURN(systemModule, nullptr);
    auto contextObj = systemModule->GetNapiValue();
    auto coerceStatus = napi_coerce_to_native_binding_object(env,
        contextObj, DetachCallbackFunc, AttachJsAbilityContext, value, nullptr);
    if (coerceStatus != napi_ok) {
        TAG_LOGE(AAFwkTag::UIABILITY, "coerceStatus Failed: %{public}d", coerceStatus);
        return nullptr;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<AbilityRuntime::AbilityContext>(ptr);
    if (workContext != nullptr) {
        napi_status status = napi_wrap(env, contextObj, workContext,
            [](napi_env, void* data, void*) {
              TAG_LOGD(AAFwkTag::UIABILITY, "finalizer for weak_ptr ability context is called");
              delete static_cast<std::weak_ptr<AbilityRuntime::AbilityContext> *>(data);
            },
            nullptr, nullptr);
        if (status != napi_ok && workContext != nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "napi_wrap Failed: %{public}d", status);
            delete workContext;
            return nullptr;
        }
    }
    return contextObj;
}

void BindContext(napi_env env, std::unique_ptr<NativeReference> contextRef, JsRuntime& jsRuntime,
    const std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext)
{
    CHECK_POINTER(contextRef);
    AbilityRuntime::HandleScope handleScope(env);
    napi_value contextObj = contextRef->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "get ability native object failed");
        return;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<AbilityRuntime::AbilityContext>(abilityContext);
    CHECK_POINTER(workContext);
    auto coerceStatus = napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachJsAbilityContext, workContext, nullptr);
    if (coerceStatus != napi_ok) {
        TAG_LOGE(AAFwkTag::UIABILITY, "coerceStatus Failed: %{public}d", coerceStatus);
        delete workContext;
        return;
    }
    abilityContext->Bind(jsRuntime, contextRef.release());
    napi_status wrapStatus = napi_wrap(
        env, contextObj, workContext,
        [](napi_env, void* data, void* hint) {
            TAG_LOGD(AAFwkTag::UIABILITY, "finalizer for weak_ptr ability context is called");
            delete static_cast<std::weak_ptr<AbilityRuntime::AbilityContext>*>(data);
        },
        nullptr, nullptr);
    if (wrapStatus != napi_ok) {
        TAG_LOGE(AAFwkTag::UIABILITY, "napi_wrap failed");
        delete workContext;
    }
}

NativeReference *GetBindingContext(const std::weak_ptr<AbilityRuntime::AbilityContext> &abilityContextWeakPtr)
{
    auto abilityContext = abilityContextWeakPtr.lock();
    if (abilityContext == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityContext");
        return nullptr;
    }
    auto& bindingObject = abilityContext->GetBindingObject();
    if (bindingObject == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null bindingObject");
        return nullptr;
    }
    auto* ptr = bindingObject->Get<NativeReference>();
    return ptr;
}

void UpdateAniContextConfig(napi_env env, const std::weak_ptr<AbilityRuntime::AbilityContext> &abilityContextWeakPtr,
    std::shared_ptr<AppExecFwk::Configuration> config)
{
    if (env == nullptr || config == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "env or config is null");
        return;
    }

    NativeReference *context = GetBindingContext(abilityContextWeakPtr);
    if (context == nullptr) {
        return;
    }

    JsAbilityContext::ConfigurationUpdated(env, context->GetNapiValue(), config);
}
} // namespace

#define DISPATCH_ABILITY_INTEROP(type, applicationContext, jsRuntime, ability)                      \
    do {                                                                                            \
        if (applicationContext && !applicationContext->IsInteropAbilityLifecycleCallbackEmpty()) {  \
            std::shared_ptr<InteropObject> interopObject =                                          \
                InteropObjectInstance::CreateInteropObject(jsRuntime, ability);                     \
            applicationContext->Dispatch##type(interopObject);                                      \
        }                                                                                           \
    } while(0)

#define DISPATCH_WINDOW_INTEROP(type, applicationContext, jsRuntime, ability, stage)                \
    do {                                                                                            \
        if (applicationContext && !applicationContext->IsInteropAbilityLifecycleCallbackEmpty()) {  \
            std::shared_ptr<InteropObject> interopAbility =                                         \
                InteropObjectInstance::CreateInteropObject(jsRuntime, ability);                     \
            std::shared_ptr<InteropObject> interopWindowStage =                                     \
                InteropObjectInstance::CreateInteropObject(jsRuntime, stage);                       \
            applicationContext->Dispatch##type(interopAbility, interopWindowStage);                 \
        }                                                                                           \
    } while(0)

UIAbility *JsUIAbility::Create(const std::unique_ptr<Runtime> &runtime)
{
    return new (std::nothrow) JsUIAbility(static_cast<JsRuntime &>(*runtime));
}

JsUIAbility::JsUIAbility(JsRuntime &jsRuntime) : jsRuntime_(jsRuntime)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
}

JsUIAbility::~JsUIAbility()
{
    // maintenance log
    TAG_LOGI(AAFwkTag::UIABILITY, "called");
    if (abilityContext_ != nullptr) {
        abilityContext_->Unbind();
    }

    jsRuntime_.FreeNativeReference(std::move(jsAbilityObj_));
    jsRuntime_.FreeNativeReference(std::move(shellContextRef_));
#ifdef SUPPORT_SCREEN
    jsRuntime_.FreeNativeReference(std::move(jsWindowStageObj_));
#endif
    InsightIntentDelayResultCallbackMgr::GetInstance().RemoveDelayResultCallback(intentId_);
}

void JsUIAbility::Init(std::shared_ptr<AppExecFwk::AbilityLocalRecord> record,
    const std::shared_ptr<OHOSApplication> application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token, bool &createObjSuc)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null localAbilityRecord");
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
            TAG_LOGE(AAFwkTag::UIABILITY, "empty srcEntrance");
            return;
        }
        srcPath.append("/");
        srcPath.append(abilityInfo->srcEntrance);
        srcPath.erase(srcPath.rfind("."));
        srcPath.append(".abc");
        TAG_LOGD(AAFwkTag::UIABILITY, "jsAbility srcPath: %{public}s", srcPath.c_str());
    }

    std::string moduleName(abilityInfo->moduleName);
    moduleName.append("::").append(abilityInfo->name);

    SetAbilityContext(abilityInfo, record->GetWant(), moduleName, srcPath, createObjSuc);
}

void JsUIAbility::UpdateAbilityObj(std::shared_ptr<AbilityInfo> abilityInfo,
    const std::string &moduleName, const std::string &srcPath)
{
    std::string key = moduleName + "::" + srcPath;
    std::unique_ptr<NativeReference> moduleObj = nullptr;
    jsAbilityObj_ = jsRuntime_.PopPreloadObj(key, moduleObj) ? std::move(moduleObj) : jsRuntime_.LoadModule(
        moduleName, srcPath, abilityInfo->hapPath, abilityInfo->compileMode == AppExecFwk::CompileMode::ES_MODULE,
        false, abilityInfo->srcEntrance);
}

void JsUIAbility::CreateAndBindContext(const std::shared_ptr<AbilityRuntime::AbilityContext> &abilityContext,
    const std::unique_ptr<Runtime>& runtime)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (abilityContext == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityContext");
        return;
    }
    if (runtime == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null runtime");
        return;
    }
    if (runtime->GetLanguage() != Runtime::Language::JS) {
        TAG_LOGE(AAFwkTag::UIABILITY, "wrong runtime language");
        return;
    }
    auto& jsRuntime = static_cast<JsRuntime&>(*runtime);
    HandleScope handleScope(jsRuntime);
    napi_env env = jsRuntime.GetNapiEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
        return;
    }
    int32_t screenMode = abilityContext->GetScreenMode();

    std::unique_ptr<NativeReference> contextRef;
    if (screenMode == AAFwk::IDLE_SCREEN_MODE) {
        napi_value contextObj = CreateJsAbilityContext(env, abilityContext);
        CHECK_POINTER(contextObj);
        contextRef = JsRuntime::LoadSystemModuleByEngine(env, "application.AbilityContext", &contextObj, 1);
    } else {
        napi_value contextObj =
            JsEmbeddableUIAbilityContext::CreateJsEmbeddableUIAbilityContext(env, abilityContext, nullptr, screenMode);
        CHECK_POINTER(contextObj);
        contextRef = JsRuntime::LoadSystemModuleByEngine(env, "application.EmbeddableUIAbilityContext", &contextObj, 1);
    }

    BindContext(env, std::move(contextRef), jsRuntime, abilityContext);
    std::weak_ptr abilityContextWeakPtr = abilityContext;
    abilityContext->RegisterBindingObjectConfigUpdateCallback(
        [env, abilityContextWeakPtr](std::shared_ptr<AppExecFwk::Configuration> config) {
            UpdateAniContextConfig(env, abilityContextWeakPtr, config);
        });
}

void JsUIAbility::SetAbilityContext(std::shared_ptr<AbilityInfo> abilityInfo,
    std::shared_ptr<AAFwk::Want> want, const std::string &moduleName, const std::string &srcPath, bool &createObjSuc)
{
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    UpdateAbilityObj(abilityInfo, moduleName, srcPath);
    if (jsAbilityObj_ == nullptr || abilityContext_ == nullptr || want == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null jsAbilityObj_ or abilityContext_ or want");
        createObjSuc = false;
        return;
    }
    reusingWindow_ = want->GetBoolParam(REUSING_WINDOW, false);
    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "check type failed");
        return;
    }
    napi_value contextObj = nullptr;
    int32_t screenMode = want->GetIntParam(AAFwk::SCREEN_MODE_KEY, AAFwk::ScreenMode::IDLE_SCREEN_MODE);
    abilityContext_->SetScreenMode(screenMode);
    CreateJSContext(env, contextObj, screenMode);
    CHECK_POINTER(shellContextRef_);
    contextObj = shellContextRef_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "get ability native object failed");
        return;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<AbilityRuntime::AbilityContext>(abilityContext_);
    CHECK_POINTER(workContext);

    auto coerceStatus = napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachJsAbilityContext, workContext, nullptr);
    if (coerceStatus != napi_ok) {
        TAG_LOGE(AAFwkTag::UIABILITY, "coerceStatus Failed: %{public}d", coerceStatus);
        delete workContext;
        return;
    }
    abilityContext_->Bind(jsRuntime_, shellContextRef_.get());
    napi_set_named_property(env, obj, "context", contextObj);
    if (abilityRecovery_ != nullptr) {
        abilityRecovery_->SetJsAbility(reinterpret_cast<uintptr_t>(workContext));
    }
    napi_status status = napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *hint) {
            delete static_cast<std::weak_ptr<AbilityRuntime::AbilityContext> *>(data);
        }, nullptr, nullptr);
    if (status != napi_ok && workContext != nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "napi_wrap Failed: %{public}d", status);
        delete workContext;
        return;
    }
}

void JsUIAbility::CreateJSContext(napi_env env, napi_value &contextObj, int32_t screenMode)
{
    AbilityRuntime::HandleScope handleScope(env);
    if (screenMode == AAFwk::IDLE_SCREEN_MODE) {
        contextObj = CreateJsAbilityContext(env, abilityContext_);
        CHECK_POINTER(contextObj);
        shellContextRef_ = std::shared_ptr<NativeReference>(JsRuntime::LoadSystemModuleByEngine(
            env, "application.AbilityContext", &contextObj, 1).release());
    } else {
        contextObj = JsEmbeddableUIAbilityContext::CreateJsEmbeddableUIAbilityContext(env,
            abilityContext_, nullptr, screenMode);
        CHECK_POINTER(contextObj);
        shellContextRef_ = std::shared_ptr<NativeReference>(JsRuntime::LoadSystemModuleByEngine(
            env, "application.EmbeddableUIAbilityContext", &contextObj, 1).release());
    }
}

void JsUIAbility::OnStart(const Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "ability: %{public}s", GetAbilityName().c_str());
    UIAbility::OnStart(want, sessionInfo);

    CHECK_POINTER(abilityInfo_);
    if (!jsAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "not found Ability.js");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "get ability object failed");
        return;
    }

    napi_value jsWant = OHOS::AppExecFwk::WrapWant(env, want);
    CHECK_POINTER(jsWant);

    napi_set_named_property(env, obj, "launchWant", jsWant);
    napi_set_named_property(env, obj, "lastRequestWant", jsWant);
    if (sessionInfo != nullptr && abilityInfo_->launchMode == AppExecFwk::LaunchMode::SPECIFIED) {
        napi_set_named_property(env, obj, "specifiedId", CreateJsValue(env, sessionInfo->specifiedFlag));
    }
    auto launchParam = GetLaunchParam();
    if (InsightIntentExecuteParam::IsInsightIntentExecute(want)) {
        launchParam.launchReason = AAFwk::LaunchReason::LAUNCHREASON_INSIGHT_INTENT;
    }
    napi_value argv[] = {
        jsWant,
        CreateJsLaunchParam(env, launchParam),
    };
    std::string methodName = "OnStart";

    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        applicationContext->SetLaunchParameter(want);
        JsAbilityLifecycleCallbackArgs ability(jsAbilityObj_);
        applicationContext->DispatchOnAbilityWillCreate(ability);
    }

    WriteLifecycleSwitchLog("onCreate");
    AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
    CallObjectMethod("onCreate", argv, ArraySize(argv));
    AddLifecycleEventAfterJSCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
    HandleAbilityDelegatorStart();
    applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        JsAbilityLifecycleCallbackArgs ability(jsAbilityObj_);
        applicationContext->DispatchOnAbilityCreate(ability);
        DISPATCH_ABILITY_INTEROP(OnAbilityCreate, applicationContext, jsRuntime_, ability);
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void JsUIAbility::HandleAbilityDelegatorStart()
{
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    auto property = std::make_shared<AppExecFwk::ADelegatorAbilityProperty>();
    if (delegator && CreateProperty(abilityContext_, property)) {
        TAG_LOGD(AAFwkTag::UIABILITY, "call PostPerformStart");
        property->object_ = jsAbilityObj_;
        delegator->PostPerformStart(property);
    }
}

void JsUIAbility::WriteLifecycleSwitchLog(const std::string lifecycleName)
{
    if (!OHOS::HiviewDFX::IsPageSwitchLoggable()) {
        return;
    }
    std::string msg = lifecycleName + ", ModuleName: " + GetModuleName() + ", AbilityName: " + GetAbilityName();
    auto ret = OHOS::HiviewDFX::WritePageSwitchStr(msg);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "write lifecycle switch log failed [%{public}d]", ret);
    }
}

void JsUIAbility::AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState state, const std::string &methodName) const
{
    auto entry = std::string("JsUIAbility::") + methodName + " begin";
    FreezeUtil::GetInstance().AddLifecycleEvent(AbilityContext::token_, entry);
}

void JsUIAbility::AddLifecycleEventAfterJSCall(FreezeUtil::TimeoutState state, const std::string &methodName) const
{
    auto entry = std::string("JsUIAbility::") + methodName + " end";
    FreezeUtil::GetInstance().AddLifecycleEvent(AbilityContext::token_, entry);
}

int32_t JsUIAbility::OnShare(WantParams &wantParam)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    if (jsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null jsAbilityObj_");
        return ERR_INVALID_VALUE;
    }
    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "ability napi value failed");
        return ERR_INVALID_VALUE;
    }

    napi_value jsWantParams = OHOS::AppExecFwk::WrapWantParams(env, wantParam);
    napi_value argv[] = {
        jsWantParams,
    };
    CallObjectMethod("onShare", argv, ArraySize(argv));
    OHOS::AppExecFwk::UnwrapWantParams(env, jsWantParams, wantParam);
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
    return ERR_OK;
}

void JsUIAbility::OnStop()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (abilityContext_) {
        TAG_LOGD(AAFwkTag::UIABILITY, "set terminating true");
        abilityContext_->SetTerminating(true);
    }
    UIAbility::OnStop();
    HandleScope handleScope(jsRuntime_);
    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        JsAbilityLifecycleCallbackArgs ability(jsAbilityObj_);
        applicationContext->DispatchOnAbilityWillDestroy(ability);
    }
    WriteLifecycleSwitchLog("onDestroy");
    CallObjectMethod("onDestroy");
    OnStopCallback();
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void JsUIAbility::OnStop(AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback)
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

    HandleScope handleScope(jsRuntime_);
    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        JsAbilityLifecycleCallbackArgs ability(jsAbilityObj_);
        applicationContext->DispatchOnAbilityWillDestroy(ability);
    }
    napi_value result = CallObjectMethod("onDestroy", nullptr, 0, true);
    if (!CheckPromise(result)) {
        OnStopCallback();
        isAsyncCallback = false;
        return;
    }

    std::weak_ptr<UIAbility> weakPtr = shared_from_this();
    auto asyncCallback = [abilityWeakPtr = weakPtr]() {
        auto ability = abilityWeakPtr.lock();
        if (ability == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null ability");
            return;
        }
        ability->OnStopCallback();
    };
    callbackInfo->Push(asyncCallback);
    isAsyncCallback = CallPromise(result, callbackInfo);
    if (!isAsyncCallback) {
        TAG_LOGE(AAFwkTag::UIABILITY, "call promise failed");
        OnStopCallback();
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void JsUIAbility::OnStopCallback()
{
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    auto property = std::make_shared<AppExecFwk::ADelegatorAbilityProperty>();
    if (delegator && CreateProperty(abilityContext_, property)) {
        TAG_LOGD(AAFwkTag::UIABILITY, "call PostPerformStop");
        property->object_ = jsAbilityObj_;
        delegator->PostPerformStop(property);
    }

    bool ret = ConnectionManager::GetInstance().DisconnectCaller(AbilityContext::token_);
    if (ret) {
        ConnectionManager::GetInstance().ReportConnectionLeakEvent(getpid(), gettid());
        TAG_LOGD(AAFwkTag::UIABILITY, "the service connection is not disconnected");
    }

    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr && jsAbilityObj_ != nullptr) {
        JsAbilityLifecycleCallbackArgs ability(jsAbilityObj_);
        applicationContext->DispatchOnAbilityDestroy(ability);
        DISPATCH_ABILITY_INTEROP(OnAbilityDestroy, applicationContext, jsRuntime_, ability);
    }
}

#ifdef SUPPORT_SCREEN
void JsUIAbility::OnSceneCreated()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "ability: %{public}s", GetAbilityName().c_str());
    UIAbility::OnSceneCreated();
    auto jsAppWindowStage = CreateAppWindowStage();
    if (jsAppWindowStage == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null jsAppWindowStage");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    UpdateJsWindowStage(jsAppWindowStage->GetNapiValue());
    napi_value argv[] = {jsAppWindowStage->GetNapiValue()};
    jsWindowStageObj_ = std::shared_ptr<NativeReference>(jsAppWindowStage.release());
    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        JsAbilityLifecycleCallbackArgs ability(jsAbilityObj_);
        JsAbilityLifecycleCallbackArgs stage(jsWindowStageObj_);
        applicationContext->DispatchOnWindowStageWillCreate(ability, stage);
    }
    {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "onWindowStageCreate");
        std::string methodName = "OnSceneCreated";
        AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
        WriteLifecycleSwitchLog("onWindowStageCreate");
        CallObjectMethod("onWindowStageCreate", argv, ArraySize(argv));
        AddLifecycleEventAfterJSCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
    }

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    auto property = std::make_shared<AppExecFwk::ADelegatorAbilityProperty>();
    if (delegator && CreateProperty(abilityContext_, property)) {
        TAG_LOGD(AAFwkTag::UIABILITY, "call PostPerformSceneCreated");
        property->object_ = jsAbilityObj_;
        delegator->PostPerformScenceCreated(property);
    }

    applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr && jsAbilityObj_ != nullptr && jsWindowStageObj_ != nullptr) {
        JsAbilityLifecycleCallbackArgs ability(jsAbilityObj_);
        JsAbilityLifecycleCallbackArgs stage(jsWindowStageObj_);
        applicationContext->DispatchOnWindowStageCreate(ability, stage);
        DISPATCH_WINDOW_INTEROP(OnWindowStageCreate, applicationContext, jsRuntime_, ability, stage);
    }

    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void JsUIAbility::OnSceneRestored()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    UIAbility::OnSceneRestored();
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    HandleScope handleScope(jsRuntime_);
    auto jsAppWindowStage = CreateAppWindowStage();
    if (jsAppWindowStage == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null jsAppWindowStage");
        return;
    }
    UpdateJsWindowStage(jsAppWindowStage->GetNapiValue());
    napi_value argv[] = {jsAppWindowStage->GetNapiValue()};
    jsWindowStageObj_ = std::shared_ptr<NativeReference>(jsAppWindowStage.release());
    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    JsAbilityLifecycleCallbackArgs ability(jsAbilityObj_);
    JsAbilityLifecycleCallbackArgs stage(jsWindowStageObj_);
    if (applicationContext != nullptr) {
        applicationContext->DispatchOnWindowStageWillRestore(ability, stage);
    }
    CallObjectMethod("onWindowStageRestore", argv, ArraySize(argv));
    if (applicationContext != nullptr) {
        applicationContext->DispatchOnWindowStageRestore(ability, stage);
    }

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    auto property = std::make_shared<AppExecFwk::ADelegatorAbilityProperty>();
    if (delegator && CreateProperty(abilityContext_, property)) {
        TAG_LOGD(AAFwkTag::UIABILITY, "call PostPerformScenceRestored");
        property->object_ = jsAbilityObj_;
        delegator->PostPerformScenceRestored(property);
    }
}

void JsUIAbility::OnSceneWillDestroy()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "ability: %{public}s", GetAbilityName().c_str());
    HandleScope handleScope(jsRuntime_);
    if (jsWindowStageObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null jsWindowStageObj_");
        return;
    }
    napi_value argv[] = {jsWindowStageObj_->GetNapiValue()};
    {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "onWindowStageWillDestroy");
        std::string methodName = "onWindowStageWillDestroy";
        CallObjectMethod("onWindowStageWillDestroy", argv, ArraySize(argv));
    }
}

void JsUIAbility::onSceneDestroyed()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "ability: %{public}s", GetAbilityName().c_str());
    UIAbility::onSceneDestroyed();

    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        JsAbilityLifecycleCallbackArgs ability(jsAbilityObj_);
        JsAbilityLifecycleCallbackArgs stage(jsWindowStageObj_);
        applicationContext->DispatchOnWindowStageWillDestroy(ability, stage);
    }
    HandleScope handleScope(jsRuntime_);
    UpdateJsWindowStage(nullptr);
    WriteLifecycleSwitchLog("onWindowStageDestroy");
    CallObjectMethod("onWindowStageDestroy");

    if (scene_ != nullptr) {
        auto window = scene_->GetMainWindow();
        if (window != nullptr) {
            TAG_LOGD(AAFwkTag::UIABILITY, "unRegisterDisplaymovelistener");
            window->UnregisterDisplayMoveListener(abilityDisplayMoveListener_);
        }
    }

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    auto property = std::make_shared<AppExecFwk::ADelegatorAbilityProperty>();
    if (delegator && CreateProperty(abilityContext_, property)) {
        TAG_LOGD(AAFwkTag::UIABILITY, "call PostPerformScenceDestroyed");
        property->object_ = jsAbilityObj_;
        delegator->PostPerformScenceDestroyed(property);
    }

    applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr && jsAbilityObj_ != nullptr && jsWindowStageObj_ != nullptr) {
        JsAbilityLifecycleCallbackArgs ability(jsAbilityObj_);
        JsAbilityLifecycleCallbackArgs stage(jsWindowStageObj_);
        applicationContext->DispatchOnWindowStageDestroy(ability, stage);
        DISPATCH_WINDOW_INTEROP(OnWindowStageDestroy, applicationContext, jsRuntime_, ability, stage);
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void JsUIAbility::OnForeground(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "ability: %{public}s", GetAbilityName().c_str());
    if (abilityInfo_) {
        jsRuntime_.UpdateModuleNameAndAssetPath(abilityInfo_->moduleName);
    }

    UIAbility::OnForeground(want);
    HandleCollaboration(want);

    if (CheckIsSilentForeground()) {
        TAG_LOGD(AAFwkTag::UIABILITY, "silent foreground, do not call 'onForeground'");
        return;
    }
    CallOnForegroundFunc(want);
}

void JsUIAbility::CallOnForegroundFunc(const Want &want)
{
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    if (jsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null jsAbilityObj_");
        return;
    }
    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "get Ability object failed");
        return;
    }

    napi_value jsWant = OHOS::AppExecFwk::WrapWant(env, want);
    if (jsWant == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null jsWant");
        return;
    }

    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        JsAbilityLifecycleCallbackArgs ability(jsAbilityObj_);
        applicationContext->DispatchOnAbilityWillForeground(ability);
    }

    WriteLifecycleSwitchLog("onForeground");
    napi_set_named_property(env, obj, "lastRequestWant", jsWant);
    std::string methodName = "OnForeground";
    AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
    CallObjectMethod("onForeground", &jsWant, 1);
    AddLifecycleEventAfterJSCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    auto property = std::make_shared<AppExecFwk::ADelegatorAbilityProperty>();
    if (delegator && CreateProperty(abilityContext_, property)) {
        TAG_LOGD(AAFwkTag::UIABILITY, "call PostPerformForeground");
        property->object_ = jsAbilityObj_;
        delegator->PostPerformForeground(property);
    }

    applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        JsAbilityLifecycleCallbackArgs ability(jsAbilityObj_);
        applicationContext->DispatchOnAbilityForeground(ability);
        DISPATCH_ABILITY_INTEROP(OnAbilityForeground, applicationContext, jsRuntime_, ability);
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void JsUIAbility::OnBackground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "ability: %{public}s", GetAbilityName().c_str());
    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        JsAbilityLifecycleCallbackArgs ability(jsAbilityObj_);
        applicationContext->DispatchOnAbilityWillBackground(ability);
    }
    std::string methodName = "OnBackground";
    HandleScope handleScope(jsRuntime_);
    AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState::BACKGROUND, methodName);
    WriteLifecycleSwitchLog("onBackground");
    CallObjectMethod("onBackground");
    AddLifecycleEventAfterJSCall(FreezeUtil::TimeoutState::BACKGROUND, methodName);

    UIAbility::OnBackground();

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    auto property = std::make_shared<AppExecFwk::ADelegatorAbilityProperty>();
    if (delegator && CreateProperty(abilityContext_, property)) {
        TAG_LOGD(AAFwkTag::UIABILITY, "call PostPerformBackground");
        property->object_ = jsAbilityObj_;
        delegator->PostPerformBackground(property);
    }

    applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr && jsAbilityObj_ != nullptr) {
        JsAbilityLifecycleCallbackArgs ability(jsAbilityObj_);
        applicationContext->DispatchOnAbilityBackground(ability);
        DISPATCH_ABILITY_INTEROP(OnAbilityBackground, applicationContext, jsRuntime_, ability);
    }
    auto want = GetWant();
    if (want != nullptr) {
        HandleCollaboration(*want);
    }
    // Apply madvise optimization based on config file (only once per process)
#ifdef ENABLE_ABILITY_MADVISE
    static bool madviseApplied = false;
    if (!madviseApplied && abilityInfo_ != nullptr) {
        bool enableMadvise = OHOS::system::GetBoolParameter("persist.sys.abilityms.enable_madvise", false);
        if (enableMadvise) {
            MadviseUtil::MadviseWithConfigFile(abilityInfo_->bundleName.c_str());
        }
        madviseApplied = true;
    }
#endif
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void JsUIAbility::OnWillForeground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "ability: %{public}s", GetAbilityName().c_str());
    UIAbility::OnWillForeground();

    std::string methodName = "OnWillForeground";
    HandleScope handleScope(jsRuntime_);
    AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
    CallObjectMethod("onWillForeground");
    AddLifecycleEventAfterJSCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);

    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void JsUIAbility::OnDidForeground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "ability: %{public}s", GetAbilityName().c_str());
    UIAbility::OnDidForeground();

    std::string methodName = "OnDidForeground";
    HandleScope handleScope(jsRuntime_);
    AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
    CallObjectMethod("onDidForeground");
    AddLifecycleEventAfterJSCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);

    if (scene_ != nullptr) {
        scene_->GoResume();
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void JsUIAbility::OnWillBackground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "ability: %{public}s", GetAbilityName().c_str());
    if (scene_ != nullptr) {
        scene_->GoPause();
    }
    UIAbility::OnWillBackground();

    std::string methodName = "OnWillBackground";
    HandleScope handleScope(jsRuntime_);
    AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState::BACKGROUND, methodName);
    CallObjectMethod("onWillBackground");
    AddLifecycleEventAfterJSCall(FreezeUtil::TimeoutState::BACKGROUND, methodName);

    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void JsUIAbility::OnDidBackground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "ability: %{public}s", GetAbilityName().c_str());
    UIAbility::OnDidBackground();

    std::string methodName = "OnDidBackground";
    HandleScope handleScope(jsRuntime_);
    AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState::BACKGROUND, methodName);
    CallObjectMethod("onDidBackground");
    AddLifecycleEventAfterJSCall(FreezeUtil::TimeoutState::BACKGROUND, methodName);

    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

bool JsUIAbility::OnBackPress()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "ability: %{public}s", GetAbilityName().c_str());
    UIAbility::OnBackPress();
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    napi_value jsValue = CallObjectMethod("onBackPressed", nullptr, 0, true, false);
    bool defaultRet = BackPressDefaultValue();
    if (jsValue == nullptr) {
        TAG_LOGD(AAFwkTag::UIABILITY, "null jsValue, return defaultRet %{public}d", defaultRet);
        return defaultRet;
    }
    bool ret = defaultRet;
    if (!ConvertFromJsValue(env, jsValue, ret)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "get js value failed");
        return defaultRet;
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "end ret: %{public}d", ret);
    return ret;
}

void JsUIAbility::OnPrepareTerminate(AppExecFwk::AbilityTransactionCallbackInfo<bool> *callbackInfo,
    bool &isAsync)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "ability: %{public}s", GetAbilityName().c_str());
    UIAbility::OnPrepareTerminate(callbackInfo, isAsync);
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    napi_value onPrepareToTerminateAsyncResult = nullptr;
    napi_value onPrepareToTerminateResult = nullptr;
    onPrepareToTerminateAsyncResult = CallObjectMethod("onPrepareToTerminateAsync", nullptr, 0, true);
    if (onPrepareToTerminateAsyncResult == nullptr) {
        TAG_LOGI(AAFwkTag::UIABILITY, "onPrepareToTerminateAsync not implemented, call onPrepareToTerminate");
        onPrepareToTerminateResult = CallObjectMethod("onPrepareToTerminate", nullptr, 0, true);
    }

    if (onPrepareToTerminateAsyncResult == nullptr && onPrepareToTerminateResult == nullptr) {
        TAG_LOGW(AAFwkTag::UIABILITY, "neither is implemented");
        return;
    }
    if (onPrepareToTerminateResult != nullptr) {
        TAG_LOGI(AAFwkTag::UIABILITY, "sync call");
        bool isTerminate = false;
        if (!ConvertFromJsValue(env, onPrepareToTerminateResult, isTerminate)) {
            TAG_LOGE(AAFwkTag::UIABILITY, "get js value failed");
        }
        callbackInfo->Call(isTerminate);
        return;
    }
    TAG_LOGI(AAFwkTag::UIABILITY, "async call");
    if (!CheckPromise(onPrepareToTerminateAsyncResult) ||
        !CallPromise(onPrepareToTerminateAsyncResult, callbackInfo)) {
        TAG_LOGE(AAFwkTag::APPKIT, "check or call promise error");
        return;
    }
    isAsync = true;
}

std::unique_ptr<NativeReference> JsUIAbility::CreateAppWindowStage()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    napi_value jsWindowStage = Rosen::CreateJsWindowStage(env, GetScene());
    if (jsWindowStage == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null jsWindowStage");
        return nullptr;
    }
    return JsRuntime::LoadSystemModuleByEngine(env, "application.WindowStage", &jsWindowStage, 1);
}

void JsUIAbility::GetPageStackFromWant(const Want &want, std::string &pageStack)
{
    auto stringObj = AAFwk::IString::Query(want.GetParams().GetParam(PAGE_STACK_PROPERTY_NAME));
    if (stringObj != nullptr) {
        pageStack = AAFwk::String::Unbox(stringObj);
    }
}

bool JsUIAbility::IsRestorePageStack(const Want &want)
{
    return want.GetBoolParam(SUPPORT_CONTINUE_PAGE_STACK_PROPERTY_NAME, true);
}

void JsUIAbility::RestorePageStack(const Want &want)
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
            TAG_LOGE(AAFwkTag::UIABILITY, "null content storage");
        }
    }
}

void JsUIAbility::AbilityContinuationOrRecover(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    // multi-instance ability continuation
    TAG_LOGD(AAFwkTag::UIABILITY, "launch reason: %{public}d, last exit reasion: %{public}d",
        launchParam_.launchReason, launchParam_.lastExitReason);
    if (IsRestoredInContinuation()) {
        RestorePageStack(want);
        OnSceneRestored();
        NotifyContinuationResult(want, true);
    } else if (ShouldRecoverState(want)) {
        std::string pageStack = abilityRecovery_->GetSavedPageStack(AppExecFwk::StateReason::DEVELOPER_REQUEST);
        HandleScope handleScope(jsRuntime_);
        auto env = jsRuntime_.GetNapiEnv();
        auto mainWindow = scene_->GetMainWindow();
        if (mainWindow != nullptr) {
            mainWindow->NapiSetUIContent(pageStack, env, abilityContext_->GetContentStorage()->GetNapiValue(),
                Rosen::BackupAndRestoreType::APP_RECOVERY);
        } else {
            TAG_LOGE(AAFwkTag::UIABILITY, "null mainWindow");
        }
        OnSceneRestored();
    } else {
        if (ShouldDefaultRecoverState(want) && abilityRecovery_ != nullptr && scene_ != nullptr) {
            TAG_LOGD(AAFwkTag::UIABILITY, "need restore");
            std::string pageStack = abilityRecovery_->GetSavedPageStack(AppExecFwk::StateReason::DEVELOPER_REQUEST);
            auto mainWindow = scene_->GetMainWindow();
            if (!pageStack.empty() && mainWindow != nullptr) {
                mainWindow->SetRestoredRouterStack(pageStack);
            }
        }
        OnSceneCreated();
    }
}

void JsUIAbility::DoOnForeground(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (scene_ == nullptr) {
        if ((abilityContext_ == nullptr) || (sceneListener_ == nullptr)) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null abilityContext or sceneListener_");
            return;
        }
        DoOnForegroundForSceneIsNull(want);
    } else {
        auto window = scene_->GetMainWindow();
        if (window != nullptr && want.HasParameter(Want::PARAM_RESV_WINDOW_MODE)) {
            auto windowMode = want.GetIntParam(
                Want::PARAM_RESV_WINDOW_MODE, AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED);
            if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
                window->SetWindowMode(static_cast<Rosen::WindowMode>(windowMode));
            }
            windowMode_ = windowMode;
            TAG_LOGD(AAFwkTag::UIABILITY, "set window mode: %{public}d", windowMode);
        }
        SetInsightIntentParam(want, false);
    }

    auto window = scene_->GetMainWindow();
    if (window != nullptr && securityFlag_) {
        window->SetSystemPrivacyMode(true);
    }

    if (CheckIsSilentForeground()) {
        TAG_LOGI(AAFwkTag::UIABILITY, "silent foreground, do not show window");
        return;
    }

    OnWillForeground();

    TAG_LOGD(AAFwkTag::UIABILITY, "move scene to foreground, sceneFlag_: %{public}d", UIAbility::sceneFlag_);
    AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState::FOREGROUND, METHOD_NAME);
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "scene_->GoForeground");
    scene_->GoForeground(UIAbility::sceneFlag_);
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void JsUIAbility::DoOnForegroundForSceneIsNull(const Want &want)
{
    scene_ = std::make_shared<Rosen::WindowScene>();
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
            TAG_LOGW(AAFwkTag::UIABILITY, "formatRegex: [%{public}s]", strDisplayId.c_str());
        }
    }
    auto option = GetWindowOption(want);
    Rosen::WMError ret = Rosen::WMError::WM_OK;
    auto sessionToken = GetSessionToken();
    auto identityToken = GetIdentityToken();
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "scene_->Init");
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled() && sessionToken != nullptr) {
        abilityContext_->SetWeakSessionToken(sessionToken);
        ret = scene_->Init(displayId, abilityContext_, sceneListener_, option, sessionToken, identityToken,
            reusingWindow_);
        RemoveShareRouterByBundleType(want);
        std::string navDestinationInfo = want.GetStringParam(Want::ATOMIC_SERVICE_SHARE_ROUTER);
        if (!navDestinationInfo.empty()) {
            TAG_LOGD(AAFwkTag::UIABILITY, "SetNavDestinationInfo :%{public}s", navDestinationInfo.c_str());
            scene_->SetNavDestinationInfo(navDestinationInfo);
        }
        if (abilityContext_->IsHook()) {
            TAG_LOGI(AAFwkTag::UIABILITY, "to set element");
            Rosen::WMError result = scene_->SetHookedWindowElementInfo(want.GetElement());
            if (result != Rosen::WMError::WM_OK) {
                TAG_LOGW(AAFwkTag::UIABILITY, "scene error:%{public}d", result);
            }
        }
    } else {
        ret = scene_->Init(displayId, abilityContext_, sceneListener_, option);
    }
    if (ret != Rosen::WMError::WM_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "init window scene failed");
        FreezeUtil::GetInstance().AppendLifecycleEvent(AbilityContext::token_,
            std::string("JsUIAbility::DoOnForegroundForSceneIsNull; error ") + std::to_string(static_cast<int>(ret)));
        return;
    }

    SetInsightIntentParam(want, true);
    AbilityContinuationOrRecover(want);
    auto window = scene_->GetMainWindow();
    if (window) {
        TAG_LOGD(AAFwkTag::UIABILITY, "registerDisplayMoveListener, windowId: %{public}d", window->GetWindowId());
        abilityDisplayMoveListener_ = new AbilityDisplayMoveListener(weak_from_this());
        if (abilityDisplayMoveListener_ == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null abilityDisplayMoveListener_");
            return;
        }
        window->RegisterDisplayMoveListener(abilityDisplayMoveListener_);
    }
}

void JsUIAbility::RequestFocus(const Want &want)
{
    TAG_LOGI(AAFwkTag::UIABILITY, "called");
    if (scene_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null scene_");
        return;
    }
    auto window = scene_->GetMainWindow();
    if (window != nullptr && want.HasParameter(Want::PARAM_RESV_WINDOW_MODE) &&
        !Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto windowMode = want.GetIntParam(
            Want::PARAM_RESV_WINDOW_MODE, AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED);
        window->SetWindowMode(static_cast<Rosen::WindowMode>(windowMode));
        TAG_LOGD(AAFwkTag::UIABILITY, "set window mode: %{public}d", windowMode);
    }
    SetInsightIntentParam(want, false);
    AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState::FOREGROUND, METHOD_NAME);
    scene_->GoForeground(UIAbility::sceneFlag_);
    TAG_LOGI(AAFwkTag::UIABILITY, "end");
}

void JsUIAbility::ContinuationRestore(const Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (!IsRestoredInContinuation()) {
        TAG_LOGE(AAFwkTag::UIABILITY, "not in continuation");
        return;
    }
    if (scene_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null scene_");
        return;
    }
    RestorePageStack(want);
    OnSceneRestored();
    NotifyContinuationResult(want, true);
}

std::shared_ptr<NativeReference> JsUIAbility::GetJsWindowStage()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (jsWindowStageObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null jsWindowStageObj_");
    }
    return jsWindowStageObj_;
}

napi_value JsUIAbility::GetWindowStage()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (shellContextRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null shellContextRef_");
        return nullptr;
    }

    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value contextObj = shellContextRef_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "contextObj is not a valid JS object");
        return nullptr;
    }

    napi_value windowStage = nullptr;
    napi_status status = napi_get_named_property(env, contextObj, "windowStage", &windowStage);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Failed to get 'windowStage' property, status: %{public}d", status);
        return nullptr;
    }
    napi_valuetype type;
    napi_typeof(env, windowStage, &type);
    if (type == napi_undefined || type == napi_null) {
        return nullptr;
    }
    return windowStage;
}

const JsRuntime &JsUIAbility::GetJsRuntime()
{
    return jsRuntime_;
}

void JsUIAbility::ExecuteInsightIntentRepeateForeground(const Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (executeParam == nullptr) {
        TAG_LOGW(AAFwkTag::UIABILITY, "null executeParam");
        RequestFocus(want);
        InsightIntentExecutorMgr::TriggerCallbackInner(std::move(callback), ERR_OK);
        return;
    }

    auto asyncCallback = [weak = weak_from_this(), want](InsightIntentExecuteResult result) {
        TAG_LOGD(AAFwkTag::UIABILITY, "request focus");
        auto ability = weak.lock();
        if (ability == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null ability");
            return;
        }
        std::static_pointer_cast<JsUIAbility>(ability)->AddLifecycleEventAfterJSCall(
            FreezeUtil::TimeoutState::FOREGROUND, "IntentRepeat");
        ability->RequestFocus(want);
    };
    callback->Push(asyncCallback);

    InsightIntentExecutorInfo executeInfo;
    auto ret = GetInsightIntentExecutorInfo(want, executeParam, executeInfo);
    if (!ret) {
        TAG_LOGE(AAFwkTag::UIABILITY, "get intentExecutor failed");
        InsightIntentExecutorMgr::TriggerCallbackInner(std::move(callback),
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
        return;
    }
    AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState::FOREGROUND, "IntentRepeat");
    ret = DelayedSingleton<InsightIntentExecutorMgr>::GetInstance()->ExecuteInsightIntent(
        jsRuntime_, executeInfo, std::move(callback));
    if (!ret) {
        // callback has removed, release in insight intent executor.
        TAG_LOGE(AAFwkTag::UIABILITY, "execute insightIntent failed");
    }
}

void JsUIAbility::ExecuteInsightIntentMoveToForeground(const Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (executeParam == nullptr) {
        TAG_LOGW(AAFwkTag::UIABILITY, "null executeParam");
        OnForeground(want);
        InsightIntentExecutorMgr::TriggerCallbackInner(std::move(callback), ERR_OK);
        return;
    }

    if (abilityInfo_) {
        jsRuntime_.UpdateModuleNameAndAssetPath(abilityInfo_->moduleName);
    }
    UIAbility::OnForeground(want);

    auto asyncCallback = [weak = weak_from_this(), want](InsightIntentExecuteResult result) {
        TAG_LOGD(AAFwkTag::UIABILITY, "begin call onForeground");
        auto ability = weak.lock();
        if (ability == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null ability");
            return;
        }
        std::static_pointer_cast<JsUIAbility>(ability)->AddLifecycleEventAfterJSCall(
            FreezeUtil::TimeoutState::FOREGROUND, "IntentForeground");
        ability->CallOnForegroundFunc(want);
    };
    callback->Push(asyncCallback);

    InsightIntentExecutorInfo executeInfo;
    auto ret = GetInsightIntentExecutorInfo(want, executeParam, executeInfo);
    if (!ret) {
        TAG_LOGE(AAFwkTag::UIABILITY, "get intentExecutor failed");
        InsightIntentExecutorMgr::TriggerCallbackInner(std::move(callback),
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
        return;
    }
    RegisterDelayResultCallback(executeParam);
    AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState::FOREGROUND, "IntentForeground");
    ret = DelayedSingleton<InsightIntentExecutorMgr>::GetInstance()->ExecuteInsightIntent(
        jsRuntime_, executeInfo, std::move(callback));
    if (!ret) {
        // callback has removed, release in insight intent executor.
        TAG_LOGE(AAFwkTag::UIABILITY, "execute insightIntent failed");
        InsightIntentDelayResultCallbackMgr::GetInstance().RemoveDelayResultCallback(
            executeParam->insightIntentId_);
        return;
    }
    intentId_ = executeParam->insightIntentId_;
}

void JsUIAbility::ExecuteInsightIntentPage(const Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback)
{
    TAG_LOGD(AAFwkTag::INTENT, "execute intent page");
    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid ability info");
        InsightIntentExecutorMgr::TriggerCallbackInner(std::move(callback),
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
        return;
    }

    InsightIntentExecutorInfo executeInfo;
    executeInfo.hapPath = abilityInfo_->hapPath;
    executeInfo.executeParam = executeParam;
    auto ret = DelayedSingleton<InsightIntentExecutorMgr>::GetInstance()->ExecuteInsightIntent(
        jsRuntime_, executeInfo, std::move(callback));
    if (!ret) {
        // callback has removed, release in insight intent executor.
        TAG_LOGE(AAFwkTag::INTENT, "execute insightIntent failed");
    }
}

void JsUIAbility::SetInsightIntentParam(const Want &want, bool coldStart)
{
    if (scene_ == nullptr) {
        TAG_LOGW(AAFwkTag::INTENT, "scene invalid");
        return;
    }

    auto window = scene_->GetMainWindow();
    if (window == nullptr) {
        TAG_LOGW(AAFwkTag::INTENT, "window invalid");
        return;
    }

    if (abilityInfo_ == nullptr) {
        TAG_LOGW(AAFwkTag::INTENT, "abilityInfo invalid");
        return;
    }

    if (AppExecFwk::InsightIntentExecuteParam::IsInsightIntentPage(want)) {
        JsInsightIntentPage::SetInsightIntentParam(jsRuntime_, abilityInfo_->hapPath, want, window, coldStart);
    }
}

void JsUIAbility::ExecuteInsightIntentBackground(const Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback)
{
    TAG_LOGI(AAFwkTag::UIABILITY, "executeInsightIntentBackground");
    if (executeParam == nullptr) {
        TAG_LOGW(AAFwkTag::UIABILITY, "null executeParam");
        InsightIntentExecutorMgr::TriggerCallbackInner(std::move(callback), ERR_OK);
        return;
    }

    if (abilityInfo_) {
        jsRuntime_.UpdateModuleNameAndAssetPath(abilityInfo_->moduleName);
    }

    InsightIntentExecutorInfo executeInfo;
    auto ret = GetInsightIntentExecutorInfo(want, executeParam, executeInfo);
    if (!ret) {
        TAG_LOGE(AAFwkTag::UIABILITY, "get intentExecutor failed");
        InsightIntentExecutorMgr::TriggerCallbackInner(std::move(callback),
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
        return;
    }

    ret = DelayedSingleton<InsightIntentExecutorMgr>::GetInstance()->ExecuteInsightIntent(
        jsRuntime_, executeInfo, std::move(callback));
    if (!ret) {
        // callback has removed, release in insight intent executor.
        TAG_LOGE(AAFwkTag::UIABILITY, "execute insightIntent failed");
    }
}

bool JsUIAbility::GetInsightIntentExecutorInfo(const Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    InsightIntentExecutorInfo& executeInfo)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");

    auto context = GetAbilityContext();
    if (executeParam == nullptr || context == nullptr || abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "param invalid");
        return false;
    }

    if (executeParam->executeMode_ == AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND
        && jsWindowStageObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "param invalid");
        return false;
    }

    const WantParams &wantParams = want.GetParams();
    executeInfo.srcEntry = wantParams.GetStringParam("ohos.insightIntent.srcEntry");
    executeInfo.hapPath = abilityInfo_->hapPath;
    executeInfo.esmodule = abilityInfo_->compileMode == AppExecFwk::CompileMode::ES_MODULE;
    executeInfo.windowMode = windowMode_;
    executeInfo.token = context->GetToken();
    if (jsWindowStageObj_ != nullptr) {
        executeInfo.pageLoader = jsWindowStageObj_;
    }
    executeInfo.executeParam = executeParam;
    return true;
}

int32_t JsUIAbility::OnCollaborate(WantParams &wantParam)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "OnCollaborate: %{public}s", GetAbilityName().c_str());
    int32_t ret = CollaborateResult::ON_COLLABORATE_ERR;
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    if (jsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null jsAbilityObj_");
        return ret;
    }
    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "get ability object fail");
        return ret;
    }

    napi_value jsWantParams = OHOS::AppExecFwk::WrapWantParams(env, wantParam);
    napi_value argv[] = {
        jsWantParams,
    };
    auto result = CallObjectMethod("onCollaborate", argv, ArraySize(argv), true);
    if (result == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "onCollaborate not implemented");
        return CollaborateResult::ON_COLLABORATE_NOT_IMPLEMENTED;
    }
    OHOS::AppExecFwk::UnwrapWantParams(env, jsWantParams, wantParam);

    if (!ConvertFromJsValue(env, result, ret)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "get js value failed");
        return ret;
    }
    ret = (ret == CollaborateResult::ACCEPT) ? CollaborateResult::ACCEPT : CollaborateResult::REJECT;
    return ret;
}
#endif

void JsUIAbility::OnAbilityRequestFailure(const std::string &requestId, const AppExecFwk::ElementName &element,
    const std::string &message, int32_t resultCode)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "OnAbilityRequestFailure called");
    HandleScope handle(jsRuntime_);
    UIAbility::OnAbilityRequestFailure(requestId, element, message, resultCode);
    auto abilityContext = GetAbilityContext();
    if (abilityContext == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityContext");
        return;
    }
    abilityContext->OnRequestFailure(requestId, element, message, resultCode);
}

void JsUIAbility::OnAbilityRequestSuccess(const std::string &requestId, const AppExecFwk::ElementName &element,
    const std::string &message)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "OnAbilityRequestSuccess called");
    HandleScope handle(jsRuntime_);
    UIAbility::OnAbilityRequestSuccess(requestId, element, message);
    auto abilityContext = GetAbilityContext();
    if (abilityContext == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityContext");
        return;
    }
    abilityContext->OnRequestSuccess(requestId, element, message);
}

int32_t JsUIAbility::OnContinue(WantParams &wantParams, bool &isAsyncOnContinue,
    const AppExecFwk::AbilityInfo &abilityInfo)
{
    TAG_LOGI(AAFwkTag::UIABILITY, "called");
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    if (jsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null jsAbilityObj_");
        return AppExecFwk::ContinuationManagerStage::OnContinueResult::ON_CONTINUE_ERR;
    }
    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "failed get ability");
        return AppExecFwk::ContinuationManagerStage::OnContinueResult::ON_CONTINUE_ERR;
    }
    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        JsAbilityLifecycleCallbackArgs ability(jsAbilityObj_);
        applicationContext->DispatchOnAbilityWillContinue(ability);
    }
    napi_value jsWantParams = OHOS::AppExecFwk::WrapWantParams(env, wantParams);
    napi_value result = CallObjectMethod("onContinue", &jsWantParams, 1, true);
    int32_t onContinueRes = 0;
    if (!CheckPromise(result)) {
        return OnContinueSyncCB(result, wantParams, jsWantParams);
    }
    auto *callbackInfo = AppExecFwk::AbilityTransactionCallbackInfo<int32_t>::Create();
    if (callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null callbackInfo");
        return OnContinueSyncCB(result, wantParams, jsWantParams);
    }
    std::weak_ptr<UIAbility> weakPtr = shared_from_this();
    napi_ref jsWantParamsRef;
    napi_status createStatus = napi_create_reference(env, jsWantParams, 1, &jsWantParamsRef);
    if (createStatus != napi_ok) {
        TAG_LOGE(AAFwkTag::UIABILITY, "napi_create_reference failed, %{public}d", createStatus);
        AppExecFwk::AbilityTransactionCallbackInfo<int32_t>::Destroy(callbackInfo);
        callbackInfo = nullptr;
        return AppExecFwk::ContinuationManagerStage::OnContinueResult::ON_CONTINUE_ERR;
    }
    ReleaseOnContinueAsset(env, result, jsWantParamsRef, callbackInfo);
    auto asyncCallback = [jsWantParamsRef, abilityWeakPtr = weakPtr, abilityInfo](int32_t status) {
        auto ability = abilityWeakPtr.lock();
        if (ability == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null ability");
            return;
        }
        ability->OnContinueAsyncCB(jsWantParamsRef, status, abilityInfo);
    };
    callbackInfo->Push(asyncCallback);
    if (!CallPromise(result, callbackInfo)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "call promise failed");
        AppExecFwk::AbilityTransactionCallbackInfo<int32_t>::Destroy(callbackInfo);
        callbackInfo = nullptr;
        return OnContinueSyncCB(result, wantParams, jsWantParams);
    }
    isAsyncOnContinue = true;
    TAG_LOGI(AAFwkTag::UIABILITY, "end");
    return onContinueRes;
}

void JsUIAbility::ReleaseOnContinueAsset(const napi_env env, napi_value &promise,
    napi_ref &jsWantParamsRef, AppExecFwk::AbilityTransactionCallbackInfo<int32_t> *callbackInfo)
{
    napi_add_finalizer(env, promise, jsWantParamsRef, [](napi_env env, void *context, void *) {
        TAG_LOGI(AAFwkTag::UIABILITY, "Release jsWantParamsRef");
        auto contextRef = reinterpret_cast<napi_ref>(context);
        if (contextRef != nullptr) {
            napi_delete_reference(env, contextRef);
            contextRef = nullptr;
        }
    }, nullptr, nullptr);
    napi_add_finalizer(env, promise, callbackInfo, [](napi_env env, void *context, void *) {
        TAG_LOGI(AAFwkTag::UIABILITY, "Release callbackInfo");
        auto contextRef = reinterpret_cast<AppExecFwk::AbilityTransactionCallbackInfo<int32_t> *>(context);
        if (contextRef != nullptr) {
            AppExecFwk::AbilityTransactionCallbackInfo<int32_t>::Destroy(contextRef);
            contextRef = nullptr;
        }
    }, nullptr, nullptr);
}

int32_t JsUIAbility::OnContinueAsyncCB(napi_ref jsWantParamsRef, int32_t status,
    const AppExecFwk::AbilityInfo &abilityInfo)
{
    TAG_LOGI(AAFwkTag::UIABILITY, "call");
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    WantParams wantParams;
    napi_value jsWantParams;
    napi_get_reference_value(env, jsWantParamsRef, &jsWantParams);
    OHOS::AppExecFwk::UnwrapWantParams(env, jsWantParams, wantParams);
    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        JsAbilityLifecycleCallbackArgs ability(jsAbilityObj_);
        applicationContext->DispatchOnAbilityContinue(ability);
    }
#ifdef SUPPORT_GRAPHICS
    auto value = wantParams.GetParam(SUPPORT_CONTINUE_PAGE_STACK_PROPERTY_NAME);
    AAFwk::IBoolean *ao = AAFwk::IBoolean::Query(value);
    if (ao == nullptr || AAFwk::Boolean::Unbox(ao)) {
        std::string pageStack = GetContentInfo();
        if (pageStack.empty()) {
            TAG_LOGE(AAFwkTag::CONTINUATION, "GetContentInfo failed");
            status = CONTINUE_GET_CONTENT_FAILED;
        } else {
            TAG_LOGI(AAFwkTag::CONTINUATION, "pageStack: %{public}s", pageStack.c_str());
            wantParams.SetParam(PAGE_STACK_PROPERTY_NAME, AAFwk::String::Box(pageStack));
        }
    }
#endif
    Want want;
    want.SetParams(wantParams);
    want.AddFlags(want.FLAG_ABILITY_CONTINUATION);
    want.SetElementName(abilityInfo.deviceId, abilityInfo.bundleName, abilityInfo.name,
        abilityInfo.moduleName);
    int result = AAFwk::AbilityManagerClient::GetInstance()->StartContinuation(want, token_, status);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "StartContinuation failed: %{public}d", result);
    }
    TAG_LOGI(AAFwkTag::UIABILITY, "end");
    return result;
}

int32_t JsUIAbility::OnContinueSyncCB(napi_value result, WantParams &wantParams, napi_value jsWantParams)
{
    TAG_LOGI(AAFwkTag::UIABILITY, "call");
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    int32_t onContinueRes = 0;
    if (!ConvertFromJsValue(env, result, onContinueRes)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "'onContinue' is not implemented");
        return AppExecFwk::ContinuationManagerStage::OnContinueResult::ON_CONTINUE_ERR;
    }
    OHOS::AppExecFwk::UnwrapWantParams(env, jsWantParams, wantParams);
    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        JsAbilityLifecycleCallbackArgs ability(jsAbilityObj_);
        applicationContext->DispatchOnAbilityContinue(ability);
    }
    TAG_LOGI(AAFwkTag::UIABILITY, "end");
    return onContinueRes;
}

int32_t JsUIAbility::OnSaveState(int32_t reason, WantParams &wantParams,
    AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnSaveStateResult> *callbackInfo,
    bool &isAsync, AppExecFwk::StateReason stateReason)
{
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    CHECK_POINTER_AND_RETURN(jsAbilityObj_, CALL_BACK_ERROR);
    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "get ability object failed");
        return CALL_BACK_ERROR;
    }
    JsAbilityLifecycleCallbackArgs ability(jsAbilityObj_);
    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        applicationContext->DispatchOnAbilityWillSaveState(ability);
    }

    napi_value jsWantParams = OHOS::AppExecFwk::WrapWantParams(env, wantParams);
    napi_value jsReason = CreateJsValue(env, reason);
    napi_value args[] = {jsReason, jsWantParams};
    bool hasCaughtException = false;
    CallObjectMethodParams callObjectMethodParams;
    callObjectMethodParams.withResult = true;
    napi_value onSaveStateAsyncResult = CallObjectMethod(
        "onSaveStateAsync", hasCaughtException, callObjectMethodParams, args, PROMISE_CALLBACK_PARAM_NUM);
    if (hasCaughtException) {
        return CALL_BACK_ERROR;
    }
    if (onSaveStateAsyncResult != nullptr) {
        OHOS::AppExecFwk::UnwrapWantParams(env, jsWantParams, wantParams);
        CallOnSaveStateInfo info = { callbackInfo, wantParams, stateReason };
        int32_t status = CallSaveStatePromise(onSaveStateAsyncResult, info);
        if (status == ERR_OK && applicationContext != nullptr) {
            applicationContext->DispatchOnAbilitySaveState(ability);
        }
        isAsync = true;
        return status;
    }
    napi_value methodOnSaveState = nullptr;
    napi_get_named_property(env, obj, "onSaveState", &methodOnSaveState);
    CHECK_POINTER_AND_RETURN(methodOnSaveState, CALL_BACK_ERROR);

    napi_value result = nullptr;
    napi_call_function(env, obj, methodOnSaveState, PROMISE_CALLBACK_PARAM_NUM, args, &result);
    OHOS::AppExecFwk::UnwrapWantParams(env, jsWantParams, wantParams);

    int32_t numberResult = 0;
    if (!ConvertFromJsValue(env, result, numberResult)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "no result return from onSaveState");
        return CALL_BACK_ERROR;
    }

    if (applicationContext != nullptr) {
        applicationContext->DispatchOnAbilitySaveState(ability);
    }
    AppExecFwk::OnSaveStateResult saveStateResult = {numberResult, wantParams, stateReason};
    callbackInfo->Call(saveStateResult);
    AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnSaveStateResult>::Destroy(callbackInfo);
    return numberResult;
}

void JsUIAbility::OnConfigurationUpdated(const Configuration &configuration)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    UIAbility::OnConfigurationUpdated(configuration);
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (abilityContext_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityContext");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    auto abilityConfig = abilityContext_->GetAbilityConfiguration();
    auto fullConfig = abilityContext_->GetConfiguration();
    if (fullConfig == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null fullConfig");
        return;
    }

    auto realConfig = AppExecFwk::Configuration(*fullConfig);

    if (abilityConfig != nullptr) {
        std::vector<std::string> changeKeyV;
        realConfig.CompareDifferent(changeKeyV, *abilityConfig);
        if (!changeKeyV.empty()) {
            realConfig.Merge(changeKeyV, *abilityConfig);
        }
    }

    TAG_LOGD(AAFwkTag::UIABILITY, "realConfig: %{public}s", realConfig.GetName().c_str());
    napi_value napiConfiguration = OHOS::AppExecFwk::WrapConfiguration(env, realConfig);

    CallObjectMethod("onConfigurationUpdated", &napiConfiguration, 1);
    CallObjectMethod("onConfigurationUpdate", &napiConfiguration, 1);
    auto realConfigPtr = std::make_shared<Configuration>(realConfig);
    if (shellContextRef_ != nullptr) {
        JsAbilityContext::ConfigurationUpdated(env, shellContextRef_->GetNapiValue(), realConfigPtr);
    }
    abilityContext_->NotifyBindingObjectConfigUpdate();
}

void JsUIAbility::OnMemoryLevel(int level)
{
    UIAbility::OnMemoryLevel(level);
    TAG_LOGD(AAFwkTag::UIABILITY, "called");

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    if (jsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null jsAbilityObj_");
        return;
    }
    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "get ability object failed");
        return;
    }

    napi_value jslevel = CreateJsValue(env, level);
    napi_value argv[] = { jslevel };
    CallObjectMethod("onMemoryLevel", argv, ArraySize(argv));
}

void JsUIAbility::UpdateContextConfiguration()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (abilityContext_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityContext_");
        return;
    }
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    if (shellContextRef_ != nullptr) {
        JsAbilityContext::ConfigurationUpdated(env, shellContextRef_->GetNapiValue(),
            abilityContext_->GetConfiguration());
    }
    abilityContext_->NotifyBindingObjectConfigUpdate();
}

void JsUIAbility::RemoveShareRouterByBundleType(const Want &want)
{
    auto abilityInfo = abilityContext_ ? abilityContext_->GetAbilityInfo() : nullptr;
    if (abilityInfo == nullptr ||
        abilityInfo->applicationInfo.bundleType != OHOS::AppExecFwk::BundleType::ATOMIC_SERVICE) {
        TAG_LOGD(AAFwkTag::UIABILITY, "not atomicService");
        const_cast<Want &>(want).RemoveParam(Want::ATOMIC_SERVICE_SHARE_ROUTER);
    }
    return;
}

void JsUIAbility::OnNewWant(const Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    UIAbility::OnNewWant(want);

#ifdef SUPPORT_SCREEN
    if (scene_) {
        RemoveShareRouterByBundleType(want);
        scene_->OnNewWant(want);
    }
    HandleCollaboration(want);
#endif

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    if (jsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null jsAbilityObj_");
        return;
    }
    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "ability object failed");
        return;
    }

    napi_value jsWant = OHOS::AppExecFwk::WrapWant(env, want);
    if (jsWant == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null want");
        return;
    }

    JsAbilityLifecycleCallbackArgs ability(jsAbilityObj_);
    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        applicationContext->SetLatestParameter(want);
        applicationContext->DispatchOnWillNewWant(ability);
    }

    napi_set_named_property(env, obj, "lastRequestWant", jsWant);
    auto launchParam = GetLaunchParam();
    if (InsightIntentExecuteParam::IsInsightIntentExecute(want)) {
        launchParam.launchReason = AAFwk::LaunchReason::LAUNCHREASON_INSIGHT_INTENT;
    }
    napi_value argv[] = {
        jsWant,
        CreateJsLaunchParam(env, launchParam),
    };
    std::string methodName = "OnNewWant";
    AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
    WriteLifecycleSwitchLog("onNewWant");
    CallObjectMethod("onNewWant", argv, ArraySize(argv));
    AddLifecycleEventAfterJSCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);

    applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        applicationContext->DispatchOnNewWant(ability);
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void JsUIAbility::OnAbilityResult(int requestCode, int resultCode, const Want &resultData)
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

sptr<IRemoteObject> JsUIAbility::CallRequest()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (jsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityContext_");
        return nullptr;
    }

    FreezeUtil::GetInstance().AddLifecycleEvent(AbilityContext::token_, "JsUIAbility::CallRequest");
    if (remoteCallee_ != nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "remoteCallee_ is exist");
        return remoteCallee_;
    }

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    auto obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null value");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, "onCallRequest", &method);
    bool isCallable = false;
    napi_is_callable(env, method, &isCallable);
    if (!isCallable) {
        TAG_LOGE(AAFwkTag::UIABILITY, "method: %{public}s", method == nullptr ? "nullptr" : "not func");
        return nullptr;
    }

    napi_value remoteJsObj = nullptr;
    std::string methodName = "onCallRequest";
    AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState::BY_CALL, methodName);
    napi_call_function(env, obj, method, 0, nullptr, &remoteJsObj);
    AddLifecycleEventAfterJSCall(FreezeUtil::TimeoutState::BY_CALL, methodName);
    if (remoteJsObj == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null remoteJsObj");
        return nullptr;
    }

    remoteCallee_ = SetNewRuleFlagToCallee(env, remoteJsObj);
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
    return remoteCallee_;
}

napi_value JsUIAbility::CallObjectMethod(const char *name, napi_value const *argv, size_t argc, bool withResult,
    bool showMethodNotFoundLog)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, std::string("CallObjectMethod:") + name);
    TAG_LOGI(AAFwkTag::UIABILITY, "JsUIAbility call js, name: %{public}s", name);
    if (jsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null jsAbilityObj");
        return nullptr;
    }

    HandleEscape handleEscape(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "get ability object failed");
        return nullptr;
    }

    napi_value methodOnCreate = nullptr;
    napi_get_named_property(env, obj, name, &methodOnCreate);
    if (methodOnCreate == nullptr) {
        if (showMethodNotFoundLog) {
            TAG_LOGE(AAFwkTag::UIABILITY, "get '%{public}s' from ability object failed", name);
        }
        return nullptr;
    }
    TryCatch tryCatch(env);
    if (withResult) {
        napi_value result = nullptr;
        napi_status withResultStatus = napi_call_function(env, obj, methodOnCreate, argc, argv, &result);
        if (withResultStatus != napi_ok) {
            TAG_LOGE(AAFwkTag::UIABILITY, "JsUIAbility call js, withResult failed: %{public}d", withResultStatus);
        }
        if (tryCatch.HasCaught()) {
            reinterpret_cast<NativeEngine*>(env)->HandleUncaughtException();
        }
        return handleEscape.Escape(result);
    }
    int64_t timeStart = AbilityRuntime::TimeUtil::SystemTimeMillisecond();
    napi_status status = napi_call_function(env, obj, methodOnCreate, argc, argv, nullptr);
    if (status != napi_ok && status != napi_function_expected) {
        TAG_LOGE(AAFwkTag::UIABILITY, "napi err: %{public}d", status);
    }
    int64_t timeEnd = AbilityRuntime::TimeUtil::SystemTimeMillisecond();
    if (tryCatch.HasCaught()) {
        reinterpret_cast<NativeEngine*>(env)->HandleUncaughtException();
    }
    TAG_LOGI(AAFwkTag::UIABILITY, "end, name: %{public}s, time: %{public}s",
        name, std::to_string(timeEnd - timeStart).c_str());
    return nullptr;
}

napi_value JsUIAbility::CallObjectMethod(const char *name, bool &hasCaughtException,
    const CallObjectMethodParams &callObjectMethodParams, napi_value const *argv, size_t argc)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, std::string("CallObjectMethod:") + name);
    TAG_LOGI(AAFwkTag::UIABILITY, "JsUIAbility call js, name: %{public}s", name);
    if (jsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null jsAbilityObj");
        return nullptr;
    }

    HandleEscape handleEscape(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "get ability object failed");
        return nullptr;
    }

    napi_value methodOnCreate = nullptr;
    napi_get_named_property(env, obj, name, &methodOnCreate);
    if (methodOnCreate == nullptr) {
        if (callObjectMethodParams.showMethodNotFoundLog) {
            TAG_LOGE(AAFwkTag::UIABILITY, "get '%{public}s' from ability object failed", name);
        }
        return nullptr;
    }
    TryCatch tryCatch(env);
    if (callObjectMethodParams.withResult) {
        napi_value result = nullptr;
        napi_status withResultStatus = napi_call_function(env, obj, methodOnCreate, argc, argv, &result);
        if (withResultStatus != napi_ok) {
            TAG_LOGE(AAFwkTag::UIABILITY, "JsUIAbility call js, withResult failed: %{public}d", withResultStatus);
        }
        if (tryCatch.HasCaught()) {
            TAG_LOGE(AAFwkTag::APPKIT, "%{public}s exception occurred", name);
            reinterpret_cast<NativeEngine*>(env)->HandleUncaughtException();
            hasCaughtException = true;
        }
        return handleEscape.Escape(result);
    }
    int64_t timeStart = AbilityRuntime::TimeUtil::SystemTimeMillisecond();
    napi_status status = napi_call_function(env, obj, methodOnCreate, argc, argv, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::UIABILITY, "JsUIAbility call js, failed: %{public}d", status);
    }
    int64_t timeEnd = AbilityRuntime::TimeUtil::SystemTimeMillisecond();
    if (tryCatch.HasCaught()) {
        reinterpret_cast<NativeEngine*>(env)->HandleUncaughtException();
        hasCaughtException = true;
    }
    TAG_LOGI(AAFwkTag::UIABILITY, "end, name: %{public}s, time: %{public}s",
        name, std::to_string(timeEnd - timeStart).c_str());
    return nullptr;
}

bool JsUIAbility::CheckPromise(napi_value result)
{
    if (result == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null result");
        return false;
    }
    auto env = jsRuntime_.GetNapiEnv();
    bool isPromise = false;
    napi_is_promise(env, result, &isPromise);
    if (!isPromise) {
        TAG_LOGD(AAFwkTag::UIABILITY, "result is not promise");
        return false;
    }
    return true;
}

bool JsUIAbility::CallPromise(napi_value result, AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo)
{
    auto env = jsRuntime_.GetNapiEnv();
    if (!CheckTypeForNapiValue(env, result, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "convert failed");
        return false;
    }
    napi_value then = nullptr;
    napi_get_named_property(env, result, "then", &then);
    if (then == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null then");
        return false;
    }
    bool isCallable = false;
    napi_is_callable(env, then, &isCallable);
    if (!isCallable) {
        TAG_LOGE(AAFwkTag::UIABILITY, "not callable");
        return false;
    }
    HandleScope handleScope(jsRuntime_);
    napi_value promiseCallback = nullptr;
    napi_create_function(env, "promiseCallback", strlen("promiseCallback"), PromiseCallback,
        callbackInfo, &promiseCallback);
    napi_value argv[1] = { promiseCallback };
    napi_call_function(env, result, then, 1, argv, nullptr);
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
    return true;
}

bool JsUIAbility::CallPromise(napi_value result, AppExecFwk::AbilityTransactionCallbackInfo<int32_t> *callbackInfo)
{
    TAG_LOGI(AAFwkTag::UIABILITY, "called");
    auto env = jsRuntime_.GetNapiEnv();
    if (!CheckTypeForNapiValue(env, result, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "convert error");
        return false;
    }
    napi_value then = nullptr;
    napi_get_named_property(env, result, "then", &then);
    if (then == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null then");
        return false;
    }
    bool isCallable = false;
    napi_is_callable(env, then, &isCallable);
    if (!isCallable) {
        TAG_LOGE(AAFwkTag::UIABILITY, "not callable");
        return false;
    }
    HandleScope handleScope(jsRuntime_);
    napi_value promiseCallback = nullptr;
    napi_create_function(env, nullptr, NAPI_AUTO_LENGTH, OnContinuePromiseCallback,
        callbackInfo, &promiseCallback);
    napi_value argv[2] = { promiseCallback, promiseCallback };
    napi_call_function(env, result, then, PROMISE_CALLBACK_PARAM_NUM, argv, nullptr);
    TAG_LOGI(AAFwkTag::UIABILITY, "end");
    return true;
}

bool JsUIAbility::CallPromise(napi_value result, AppExecFwk::AbilityTransactionCallbackInfo<bool> *callbackInfo)
{
    TAG_LOGI(AAFwkTag::UIABILITY, "called");
    auto env = jsRuntime_.GetNapiEnv();
    if (!CheckTypeForNapiValue(env, result, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "convert error");
        return false;
    }
    napi_value then = nullptr;
    napi_get_named_property(env, result, "then", &then);
    if (then == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null then");
        return false;
    }
    bool isCallable = false;
    napi_is_callable(env, then, &isCallable);
    if (!isCallable) {
        TAG_LOGE(AAFwkTag::UIABILITY, "not callable");
        return false;
    }
    HandleScope handleScope(jsRuntime_);
    napi_value promiseCallback = nullptr;
    napi_create_function(env, "promiseCallback", strlen("promiseCallback"),
        OnPrepareTerminatePromiseCallback, callbackInfo, &promiseCallback);
    napi_value argv[1] = { promiseCallback };
    napi_call_function(env, result, then, 1, argv, nullptr);
    TAG_LOGI(AAFwkTag::UIABILITY, "end");
    return true;
}

int32_t JsUIAbility::CallSaveStatePromise(napi_value result, CallOnSaveStateInfo info)
{
    auto env = jsRuntime_.GetNapiEnv();
    if (!CheckPromise(result)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "check orpromise error");
        return CALL_BACK_ERROR;
    }
    if (!CheckTypeForNapiValue(env, result, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "check type error");
        return CALL_BACK_ERROR;
    }
    napi_value then = nullptr;
    napi_get_named_property(env, result, "then", &then);
    if (then == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null then");
        return CALL_BACK_ERROR;
    }
    bool isCallable = false;
    napi_is_callable(env, then, &isCallable);
    if (!isCallable) {
        TAG_LOGE(AAFwkTag::UIABILITY, "not callable");
        return CALL_BACK_ERROR;
    }
    napi_value promiseCallback = nullptr;
    napi_create_function(env, "promiseCallback", strlen("promiseCallback"),
        OnSaveStateCallback, &info, &promiseCallback);
    napi_value argv[1] = { promiseCallback };
    napi_call_function(env, result, then, 1, argv, nullptr);
    return ERR_OK;
}

void JsUIAbility::Dump(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    UIAbility::Dump(params, info);
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    // create js array object of params
    napi_value argv[] = { CreateNativeArray(env, params) };
    napi_value dumpInfo = CallObjectMethod("dump", argv, ArraySize(argv), true);
    napi_value onDumpInfo = CallObjectMethod("onDump", argv, ArraySize(argv), true);

    GetDumpInfo(env, dumpInfo, onDumpInfo, info);
    TAG_LOGD(AAFwkTag::UIABILITY, "dump info size: %{public}zu", info.size());
}

void JsUIAbility::GetDumpInfo(
    napi_env env, napi_value dumpInfo, napi_value onDumpInfo, std::vector<std::string> &info)
{
    if (dumpInfo != nullptr) {
        uint32_t len = 0;
        napi_get_array_length(env, dumpInfo, &len);
        for (uint32_t i = 0; i < len; i++) {
            std::string dumpInfoStr;
            napi_value element = nullptr;
            napi_get_element(env, dumpInfo, i, &element);
            if (!ConvertFromJsValue(env, element, dumpInfoStr)) {
                TAG_LOGE(AAFwkTag::UIABILITY, "parse dumpInfoStr failed");
                return;
            }
            info.push_back(dumpInfoStr);
        }
    }

    if (onDumpInfo != nullptr) {
        uint32_t len = 0;
        napi_get_array_length(env, onDumpInfo, &len);
        for (uint32_t i = 0; i < len; i++) {
            std::string dumpInfoStr;
            napi_value element = nullptr;
            napi_get_element(env, onDumpInfo, i, &element);
            if (!ConvertFromJsValue(env, element, dumpInfoStr)) {
                TAG_LOGE(AAFwkTag::UIABILITY, "invalid dumpInfoStr from onDumpInfoNative");
                return;
            }
            info.push_back(dumpInfoStr);
        }
    }
}

std::shared_ptr<NativeReference> JsUIAbility::GetJsAbility()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (jsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null jsAbilityObj_");
    }
    return jsAbilityObj_;
}

sptr<IRemoteObject> JsUIAbility::SetNewRuleFlagToCallee(napi_env env, napi_value remoteJsObj)
{
    if (!CheckTypeForNapiValue(env, remoteJsObj, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null callee");
        return nullptr;
    }
    napi_value setFlagMethod = nullptr;
    napi_get_named_property(env, remoteJsObj, "setNewRuleFlag", &setFlagMethod);
    bool isCallable = false;
    napi_is_callable(env, setFlagMethod, &isCallable);
    if (!isCallable) {
        TAG_LOGE(AAFwkTag::UIABILITY, "setFlagMethod: %{public}s", setFlagMethod == nullptr ? "nullptr" : "not func");
        return nullptr;
    }
    auto flag = CreateJsValue(env, IsUseNewStartUpRule());
    napi_value argv[1] = { flag };
    napi_call_function(env, remoteJsObj, setFlagMethod, 1, argv, nullptr);

    auto remoteObj = NAPI_ohos_rpc_getNativeRemoteObject(env, remoteJsObj);
    if (remoteObj == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null remoteObj");
        return nullptr;
    }
    return remoteObj;
}
#ifdef SUPPORT_SCREEN
void JsUIAbility::UpdateJsWindowStage(napi_value windowStage)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (shellContextRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null shellContextRef_");
        return;
    }
    napi_value contextObj = shellContextRef_->GetNapiValue();
    napi_env env = jsRuntime_.GetNapiEnv();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "get native context obj failed");
        return;
    }
    if (windowStage == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null windowStage");
        napi_set_named_property(env, contextObj, "windowStage", CreateJsUndefined(env));
        return;
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "set context windowStage");
    napi_set_named_property(env, contextObj, "windowStage", windowStage);
}
#endif
bool JsUIAbility::CheckSatisfyTargetAPIVersion(int32_t version)
{
    auto applicationInfo = GetApplicationInfo();
    if (!applicationInfo) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null targetAPIVersion");
        return false;
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "targetAPIVersion: %{public}d", applicationInfo->apiTargetVersion);
    return applicationInfo->apiTargetVersion % API_VERSION_MOD >= version;
}

bool JsUIAbility::BackPressDefaultValue()
{
    return CheckSatisfyTargetAPIVersion(API12) ? true : false;
}

void JsUIAbility::OnAfterFocusedCommon(bool isFocused)
{
    auto abilityContext = GetAbilityContext();
    if (abilityContext == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityContext");
        return;
    }
    auto applicationContext = abilityContext->GetApplicationContext();
    if (applicationContext == nullptr || applicationContext->IsAbilityLifecycleCallbackEmpty()) {
        TAG_LOGD(AAFwkTag::UIABILITY, "null applicationContext or lifecycleCallback");
        return;
    }
    JsAbilityLifecycleCallbackArgs ability(GetJsAbility());
    JsAbilityLifecycleCallbackArgs stage(GetJsWindowStage());
    if (isFocused) {
        applicationContext->DispatchWindowStageFocus(ability, stage);
    } else {
        applicationContext->DispatchWindowStageUnfocus(ability, stage);
    }
}

void JsUIAbility::SetContinueState(int32_t state)
{
    if (scene_ == nullptr) {
        TAG_LOGW(AAFwkTag::UIABILITY, "windowScene is nullptr.");
        return;
    }
    auto window = scene_->GetMainWindow();
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "window is nullptr.");
        return;
    }
    window->SetContinueState(state);
    TAG_LOGI(AAFwkTag::UIABILITY, "window SetContinueState, state: %{public}d.", state);
}

void JsUIAbility::NotifyWindowDestroy()
{
    if (scene_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "windowScene is nullptr.");
        return;
    }
    TAG_LOGI(AAFwkTag::UIABILITY, "Notify scene to destroy Window.");
    Rosen::WMError ret = scene_->GoDestroyHookWindow();
    if (ret != Rosen::WMError::WM_OK) {
        TAG_LOGW(AAFwkTag::UIABILITY, "scene return error.");
    }
}

void JsUIAbility::RegisterDelayResultCallback(const std::shared_ptr<InsightIntentExecuteParam> &executeParam)
{
    auto delayResultCallback = [intentId = executeParam->insightIntentId_, token = token_]
        (AppExecFwk::InsightIntentExecuteResult result) -> int32_t {
        TAG_LOGE(AAFwkTag::UIABILITY, "delayResultCallback start");
        auto ret = AAFwk::AbilityManagerClient::GetInstance()->ExecuteInsightIntentDone(token, intentId, result);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::UIABILITY, "ExecuteInsightIntentDone ret : %{public}d", ret);
        }
        return ret;
    };
    
    InsightIntentDelayResultCallbackMgr::GetInstance().RemoveDelayResultCallback(intentId_);
    bool isDecorator = executeParam->decoratorType_ != static_cast<int8_t>(InsightIntentType::DECOR_NONE);
    InsightIntentDelayResultCallbackMgr::GetInstance().AddDelayResultCallback(
        executeParam->insightIntentId_, {delayResultCallback, isDecorator});
}
} // namespace AbilityRuntime
} // namespace OHOS
