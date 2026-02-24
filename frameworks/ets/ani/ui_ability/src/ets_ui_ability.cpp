/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "ets_ui_ability.h"

#include <regex>

#include "ability_delegator_infos.h"
#include "ability_delegator_registry.h"
#include "ani_common_configuration.h"
#include "ani_common_want.h"
#include "ani_enum_convert.h"
#include "remote_object_taihe_ani.h"
#ifdef SUPPORT_SCREEN
#include "ani_window_stage.h"
#include "js_window_stage.h"
#endif
#include "app_recovery.h"
#include "connection_manager.h"
#include "context/application_context.h"
#include "display_util.h"
#include "ets_ability_context.h"
#include "ets_ability_lifecycle_callback.h"
#include "ets_data_struct_converter.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "insight_intent_delay_result_callback_mgr.h"
#include "insight_intent_executor_info.h"
#include "insight_intent_executor_mgr.h"
#include "insight_intent_execute_param.h"
#include "interop_object_instance.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi_common_want.h"
#include "napi/native_api.h"
#include "ohos_application.h"
#include "page_switch_log.h"
#include "string_wrapper.h"

#ifdef WINDOWS_PLATFORM
#define ETS_EXPORT __declspec(dllexport)
#else
#define ETS_EXPORT __attribute__((visibility("default")))
#endif

namespace OHOS {
namespace AbilityRuntime {
std::once_flag EtsUIAbility::singletonFlag_;
namespace {
#ifdef SUPPORT_GRAPHICS
const std::string PAGE_STACK_PROPERTY_NAME = "pageStack";
const std::string SUPPORT_CONTINUE_PAGE_STACK_PROPERTY_NAME = "ohos.extra.param.key.supportContinuePageStack";
const std::string METHOD_NAME = "WindowScene::GoForeground";
#endif
#ifdef SUPPORT_SCREEN
constexpr int32_t BASE_DISPLAY_ID_NUM(10);
#endif
constexpr const char *UI_ABILITY_CLASS_NAME = "@ohos.app.ability.UIAbility.UIAbility";
constexpr const char *UI_ABILITY_SIGNATURE_VOID = ":";
constexpr const char *MEMORY_LEVEL_ENUM_NAME = "@ohos.app.ability.AbilityConstant.AbilityConstant.MemoryLevel";
constexpr const char *ON_SAVE_RESULT_ENUM_NAME = "@ohos.app.ability.AbilityConstant.AbilityConstant.OnSaveResult";
constexpr int32_t ON_SAVESTATE_INDEX_ONE = 1;
constexpr const int32_t CALL_BACK_ERROR = -1;
constexpr const char *ON_SHARE_SIGNATURE = "C{std.core.Record}:";
constexpr const char *ON_COLLABORATE =
    "C{std.core.Record}:C{@ohos.app.ability.AbilityConstant.AbilityConstant.CollaborateResult}";
constexpr const char *ON_SAVE_STATE_CALLBACK =
    "C{std.core.Record}C{@ohos.app.ability.AbilityConstant.AbilityConstant.OnSaveResult}i:";
constexpr const int32_t API12 = 12;
constexpr int32_t API_VERSION_MOD = 1000;

#define DISPATCH_ABILITY_INTEROP(type, applicationContext, etsRuntime, ability)                      \
    do {                                                                                            \
        if (applicationContext && !applicationContext->IsInteropAbilityLifecycleCallbackEmpty()) {  \
            std::shared_ptr<InteropObject> interopObject =                                          \
                InteropObjectInstance::CreateInteropObject(etsRuntime, ability);                    \
            applicationContext->Dispatch##type(interopObject);                                      \
        }                                                                                           \
    } while(0)

#define DISPATCH_WINDOW_INTEROP(type, applicationContext, etsRuntime, ability, stage)                \
    do {                                                                                            \
        if (applicationContext && !applicationContext->IsInteropAbilityLifecycleCallbackEmpty()) {  \
            std::shared_ptr<InteropObject> interopAbility =                                         \
                InteropObjectInstance::CreateInteropObject(etsRuntime, ability);                    \
            std::shared_ptr<InteropObject> interopWindowStage =                                     \
                InteropObjectInstance::CreateInteropObject(etsRuntime, stage);                      \
            applicationContext->Dispatch##type(interopAbility, interopWindowStage);                 \
        }                                                                                           \
    } while(0)

void OnDestroyPromiseCallback(ani_env *env, ani_object aniObj)
{
    TAG_LOGI(AAFwkTag::UIABILITY, "OnDestroyPromiseCallback called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
        return;
    }
    ani_long destroyCallbackPtr = 0;
    ani_status status = ANI_ERROR;
    if ((status = env->Object_GetFieldByName_Long(aniObj, "destroyCallbackPoint", &destroyCallbackPtr)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Object_GetFieldByName_Long status: %{public}d", status);
        return;
    }
    if (destroyCallbackPtr == 0) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null destroyCallbackPtr");
        return;
    }
    auto *callbackInfo = reinterpret_cast<AppExecFwk::AbilityTransactionCallbackInfo<> *>(destroyCallbackPtr);
    if (callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null callbackInfo");
        return;
    }
    callbackInfo->Call();
    AppExecFwk::AbilityTransactionCallbackInfo<>::Destroy(callbackInfo);
    if ((status = env->Object_SetFieldByName_Long(aniObj, "destroyCallbackPoint", 0)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Object_SetFieldByName_Long status: %{public}d", status);
        return;
    }
}

void OnPrepareTerminatePromiseCallback(ani_env *env, ani_object aniObj, ani_boolean aniPrepareTermination)
{
    TAG_LOGI(AAFwkTag::UIABILITY, "OnPrepareTerminatePromiseCallback begin");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
        return;
    }
    ani_long prepareToTerminateCallbackPointer = 0;
    ani_status status = env->Object_GetFieldByName_Long(
        aniObj, "prepareToTerminateCallbackPointer", &prepareToTerminateCallbackPointer);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "prepareToTerminateCallbackPointer GetField status: %{public}d", status);
        return;
    }
    auto *callbackInfo =
        reinterpret_cast<AppExecFwk::AbilityTransactionCallbackInfo<bool> *>(prepareToTerminateCallbackPointer);
    if (callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null callbackInfo");
        return;
    }
    bool prepareTermination = aniPrepareTermination;
    callbackInfo->Call(prepareTermination);
    AppExecFwk::AbilityTransactionCallbackInfo<bool>::Destroy(callbackInfo);
    if ((status = env->Object_SetFieldByName_Long(aniObj, "prepareToTerminateCallbackPointer", 0)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "prepareToTerminateCallbackPointer SetField status: %{public}d", status);
        return;
    }
    TAG_LOGI(AAFwkTag::UIABILITY, "OnPrepareTerminatePromiseCallback end");
}

void OnContinuePromiseCallback(ani_env *env, ani_object aniObj, ani_ref continueResult)
{
    TAG_LOGI(AAFwkTag::UIABILITY, "OnContinuePromiseCallback called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
        return;
    }
    ani_long continueCallbackPtr = 0;
    ani_status status = ANI_ERROR;
    if ((status = env->Object_GetFieldByName_Long(aniObj, "continueCallbackPoint", &continueCallbackPtr)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Object_GetFieldByName_Long status: %{public}d", status);
        return;
    }
    if (continueCallbackPtr == 0) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null continueCallbackPtr");
        return;
    }
    auto *callbackInfo = reinterpret_cast<AppExecFwk::AbilityTransactionCallbackInfo<int32_t> *>(continueCallbackPtr);
    if (callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null callbackInfo");
        return;
    }
    int32_t intResult = AppExecFwk::ContinuationManagerStage::ON_CONTINUE_ERR;
    if (!AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative(
        env, reinterpret_cast<ani_enum_item>(continueResult), intResult)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "EnumConvert_EtsToNative failed");
    }
    TAG_LOGI(AAFwkTag::UIABILITY, "OnContinuePromiseCallback result = %{public}d", intResult);
    callbackInfo->Call(intResult);
    AppExecFwk::AbilityTransactionCallbackInfo<int32_t>::Destroy(callbackInfo, env);
    if ((status = env->Object_SetFieldByName_Long(aniObj, "continueCallbackPoint", 0)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Object_SetFieldByName_Long status: %{public}d", status);
        return;
    }
}

void OnSaveStateCallback(
    ani_env *env, ani_object aniObj, ani_object aniWantParam, ani_object aniSaveResult, ani_int aniStateReason)
{
    TAG_LOGI(AAFwkTag::UIABILITY, "OnSaveStateCallback begin");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
        return;
    }
    ani_long saveStateCallbackPoint = 0;
    ani_status status = env->Object_GetFieldByName_Long(aniObj, "saveStateCallbackPoint", &saveStateCallbackPoint);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "saveStateCallbackPoint GetField status: %{public}d", status);
        return;
    }
    auto *callbackInfo = reinterpret_cast<AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnSaveStateResult> *>(
        saveStateCallbackPoint);
    if (callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null callbackInfo");
        return;
    }
    WantParams wantParams;
    if (!AppExecFwk::UnwrapWantParams(env, aniWantParam, wantParams)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "UnwrapWantParams failed");
        AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnSaveStateResult>::Destroy(callbackInfo);
        return;
    }
    int32_t saveResult = 0;
    if (!AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative(
        env, reinterpret_cast<ani_enum_item>(aniSaveResult), saveResult)) {
        AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnSaveStateResult>::Destroy(callbackInfo);
        TAG_LOGE(AAFwkTag::UIABILITY, "no result return from onSaveState");
        return;
    }
    AppExecFwk::OnSaveStateResult saveStateResult = { saveResult, wantParams,
        static_cast<AppExecFwk::StateReason>(aniStateReason) };
    callbackInfo->Call(saveStateResult);
    AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnSaveStateResult>::Destroy(callbackInfo);
    if ((status = env->Object_SetFieldByName_Long(aniObj, "saveStateCallbackPoint", 0)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "saveStateCallbackPoint SetField status: %{public}d", status);
        return;
    }
    TAG_LOGI(AAFwkTag::UIABILITY, "OnSaveStateCallback end");
}

ani_object GetBindingContext(const std::weak_ptr<AbilityRuntime::AbilityContext> &abilityContextWeakPtr)
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
    auto* ptr = bindingObject->Get<ani_ref>();
    if (ptr == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "no binding context");
        return nullptr;
    }

    return *reinterpret_cast<ani_object *>(ptr);
}

void UpdateAniContextConfig(ani_vm *aniVM, const std::weak_ptr<AbilityRuntime::AbilityContext> &abilityContextWeakPtr,
    std::shared_ptr<AppExecFwk::Configuration> config)
{
    if (aniVM == nullptr || config == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "etsVm or config is null");
        return;
    }

    ani_object context = GetBindingContext(abilityContextWeakPtr);
    if (context == nullptr) {
        return;
    }

    bool isAttachedThread = false;
    ani_env *env = AppExecFwk::AttachAniEnv(aniVM, isAttachedThread);
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "AttachAniEnv failed");
        return;
    }

    EtsAbilityContext::ConfigurationUpdated(env, context, config);
    AppExecFwk::DetachAniEnv(aniVM, isAttachedThread);
}
} // namespace

UIAbility *EtsUIAbility::Create(const std::unique_ptr<Runtime> &runtime)
{
    return new (std::nothrow) EtsUIAbility(static_cast<ETSRuntime &>(*runtime));
}

EtsUIAbility::EtsUIAbility(ETSRuntime &etsRuntime) : etsRuntime_(etsRuntime)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "EtsUIAbility called");
}

EtsUIAbility::~EtsUIAbility()
{
    TAG_LOGI(AAFwkTag::UIABILITY, "~EtsUIAbility called");
    if (abilityContext_ != nullptr) {
        abilityContext_->Unbind<ani_ref>();
    }
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
        return;
    }
    if (shellContextRef_ && shellContextRef_->aniRef) {
        env->GlobalReference_Delete(shellContextRef_->aniRef);
    }
    if (etsWindowStageObj_ && etsWindowStageObj_->aniRef) {
        env->GlobalReference_Delete(etsWindowStageObj_->aniRef);
    }
    InsightIntentDelayResultCallbackMgr::GetInstance().RemoveDelayResultCallback(intentId_);
}

void EtsUIAbility::Init(std::shared_ptr<AppExecFwk::AbilityLocalRecord> record,
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
        auto pos = srcPath.rfind(".");
        if (pos != std::string::npos) {
            srcPath.erase(pos);
            srcPath.append(".abc");
        }
        TAG_LOGD(AAFwkTag::UIABILITY, "etsAbility srcPath: %{public}s", srcPath.c_str());
    }

    std::string moduleName(abilityInfo->moduleName);
    moduleName.append("::").append(abilityInfo->name);
    SetAbilityContext(abilityInfo, record->GetWant(), moduleName, srcPath);
}

bool EtsUIAbility::BindNativeMethods()
{
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
        return false;
    }
    ani_class cls = nullptr;
    ani_status status = env->FindClass(UI_ABILITY_CLASS_NAME, &cls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "FindClass failed status: %{public}d", status);
        return false;
    }
    std::call_once(singletonFlag_, [&status, env, cls]() {
        std::array functions = {
            ani_native_function { "nativeOnDestroyCallback", ":", reinterpret_cast<void *>(OnDestroyPromiseCallback) },
            ani_native_function { "nativeOnPrepareToTerminateCallback", "z:",
                reinterpret_cast<void *>(OnPrepareTerminatePromiseCallback) },
            ani_native_function { "nativeOnContinueCallback", nullptr,
                reinterpret_cast<void*>(OnContinuePromiseCallback) },
            ani_native_function { "nativeOnSaveStateCallback", ON_SAVE_STATE_CALLBACK,
                reinterpret_cast<void *>(OnSaveStateCallback) },
        };
        status = env->Class_BindNativeMethods(cls, functions.data(), functions.size());
    });
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Class_BindNativeMethods failed status: %{public}d", status);
        return false;
    }
    return true;
}

void EtsUIAbility::UpdateAbilityObj(
    std::shared_ptr<AbilityInfo> abilityInfo, const std::string &moduleName, const std::string &srcPath)
{
    std::string key = moduleName + "::" + srcPath;
    std::unique_ptr<NativeReference> moduleObj = nullptr;
    etsAbilityObj_ = etsRuntime_.LoadModule(moduleName, srcPath, abilityInfo->hapPath,
        abilityInfo->compileMode == AppExecFwk::CompileMode::ES_MODULE, false, abilityInfo->srcEntrance);
    if (!BindNativeMethods()) {
        TAG_LOGE(AAFwkTag::UIABILITY, "BindNativeMethods failed");
        return;
    }
}

void EtsUIAbility::CreateAndBindContext(const std::shared_ptr<AbilityRuntime::AbilityContext> &abilityContext,
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
    if (runtime->GetLanguage() != Runtime::Language::ETS) {
        TAG_LOGE(AAFwkTag::UIABILITY, "wrong runtime language");
        return;
    }
    auto& etsRuntime = static_cast<ETSRuntime&>(*runtime);
    auto env = etsRuntime.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
        return;
    }

    int32_t screenMode = abilityContext->GetScreenMode();
    if (screenMode == AAFwk::IDLE_SCREEN_MODE) {
        ani_ref contextObj = CreateEtsAbilityContext(env, abilityContext);
        ani_ref* contextGlobalRef = new ani_ref;
        ani_status status = ANI_ERROR;
        if ((status = env->GlobalReference_Create(contextObj, contextGlobalRef)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::UIABILITY, "status : %{public}d", status);
            return;
        }
        abilityContext->Bind(etsRuntime, contextGlobalRef);
        std::weak_ptr abilityContextWeakPtr = abilityContext;
        ani_vm *aniVM = nullptr;
        status = env->GetVM(&aniVM);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::UIABILITY, "get vm failed: %{public}d", status);
            return;
        }
        abilityContext->RegisterBindingObjectConfigUpdateCallback(
            [aniVM, abilityContextWeakPtr](std::shared_ptr<AppExecFwk::Configuration> config) {
                UpdateAniContextConfig(aniVM, abilityContextWeakPtr, config);
            });
    }
    // no CreateAniEmbeddableUIAbilityContext
}

void EtsUIAbility::SetAbilityContext(std::shared_ptr<AbilityInfo> abilityInfo, std::shared_ptr<Want> want,
    const std::string &moduleName, const std::string &srcPath)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "SetAbilityContext called");
    UpdateAbilityObj(abilityInfo, moduleName, srcPath);
    if (etsAbilityObj_ == nullptr || abilityContext_ == nullptr || want == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null etsAbilityObj_ or abilityContext_ or want");
        return;
    }
    int32_t screenMode = want->GetIntParam(AAFwk::SCREEN_MODE_KEY, AAFwk::ScreenMode::IDLE_SCREEN_MODE);
    abilityContext_->SetScreenMode(screenMode);
    CreateEtsContext(screenMode);
}

void EtsUIAbility::CreateEtsContext(int32_t screenMode)
{
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
        return;
    }
    if (screenMode == AAFwk::IDLE_SCREEN_MODE) {
        ani_object contextObj = CreateEtsAbilityContext(env, abilityContext_);
        if (contextObj == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null contextObj");
            return;
        }
        ani_ref contextGlobalRef = nullptr;
        env->GlobalReference_Create(contextObj, &contextGlobalRef);
        ani_status status = env->Object_SetFieldByName_Ref(etsAbilityObj_->aniObj, "context", contextGlobalRef);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::UIABILITY, "Object_SetFieldByName_Ref status: %{public}d", status);
            return;
        }
        shellContextRef_ = std::make_shared<AppExecFwk::ETSNativeReference>();
        shellContextRef_->aniObj = contextObj;
        shellContextRef_->aniRef = contextGlobalRef;
        abilityContext_->Bind(etsRuntime_, &(shellContextRef_->aniRef));
    }
    // to be done: CreateAniEmbeddableUIAbilityContext
}

void EtsUIAbility::WriteLifecycleSwitchLog(const std::string lifecycleName)
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

void EtsUIAbility::OnStart(const Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "ability: %{public}s", GetAbilityName().c_str());
    UIAbility::OnStart(want, sessionInfo);

    if (!etsAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "not found Ability.js");
        return;
    }
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
        return;
    }
    OnStartInner(env, want, sessionInfo);
    TAG_LOGD(AAFwkTag::UIABILITY, "OnStart end");
}

void EtsUIAbility::OnStartInner(ani_env *env, const Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
{
    ani_object wantObj = AppExecFwk::WrapWant(env, want);
    if (wantObj == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null wantObj");
        return;
    }
    ani_status status = ANI_ERROR;
    if ((status = env->Object_SetFieldByName_Ref(etsAbilityObj_->aniObj, "launchWant", wantObj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "launchWant Object_SetFieldByName_Ref status: %{public}d", status);
        return;
    }
    if ((status = env->Object_SetFieldByName_Ref(etsAbilityObj_->aniObj, "lastRequestWant", wantObj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "lastRequestWant Object_SetFieldByName_Ref status: %{public}d", status);
        return;
    }
    SetSelfSpecifiedId(env, sessionInfo);
    auto launchParam = GetLaunchParam();
    if (InsightIntentExecuteParam::IsInsightIntentExecute(want)) {
        launchParam.launchReason = AAFwk::LaunchReason::LAUNCHREASON_INSIGHT_INTENT;
    }
    ani_object launchParamObj = nullptr;
    if (!WrapLaunchParam(env, launchParam, launchParamObj)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "WrapLaunchParam failed");
        return;
    }
    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        EtsAbilityLifecycleCallbackArgs ability(etsAbilityObj_);
        applicationContext->DispatchOnAbilityWillCreate(ability);
    }
    WriteLifecycleSwitchLog("onCreate");
    CallObjectMethod(false, "onCreate", nullptr, wantObj, launchParamObj);
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator(
        AbilityRuntime::Runtime::Language::ETS);
    auto property = std::make_shared<AppExecFwk::EtsDelegatorAbilityProperty>();
    if (delegator && CreateProperty(abilityContext_, property)) {
        TAG_LOGD(AAFwkTag::UIABILITY, "call PostPerformStart");
        property->object_ = etsAbilityObj_;
        delegator->PostPerformStart(property);
    }
    applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        TAG_LOGD(AAFwkTag::UIABILITY, "call DispatchOnAbilityCreate");
        EtsAbilityLifecycleCallbackArgs ability(etsAbilityObj_);
        applicationContext->DispatchOnAbilityCreate(ability);
        DISPATCH_ABILITY_INTEROP(OnAbilityCreate, applicationContext, etsRuntime_, ability);
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "OnStart end");
}

int32_t EtsUIAbility::OnShare(WantParams &wantParam)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "OnShare called");
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
        return ERR_INVALID_VALUE;
    }
    ani_ref wantParamRef = AppExecFwk::WrapWantParams(env, wantParam);
    if (wantParamRef == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null wantParamRef");
        return ERR_INVALID_VALUE;
    }
    CallObjectMethod(false, "onShare", ON_SHARE_SIGNATURE, wantParamRef);
    AppExecFwk::UnwrapWantParams(env, wantParamRef, wantParam);
    return ERR_OK;
}

void EtsUIAbility::SetSelfSpecifiedId(ani_env *env, sptr<AAFwk::SessionInfo> sessionInfo)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "SetSelfSpecifiedId called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
        return;
    }
    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityInfo_");
        return;
    }
    if (etsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null etsAbilityObj_");
        return;
    }
    ani_status status = ANI_ERROR;
    if (sessionInfo != nullptr && abilityInfo_->launchMode == AppExecFwk::LaunchMode::SPECIFIED) {
        if ((status = env->Object_SetFieldByName_Ref(etsAbilityObj_->aniObj,
            "specifiedId", AppExecFwk::GetAniString(env, sessionInfo->specifiedFlag))) != ANI_OK) {
            TAG_LOGE(AAFwkTag::UIABILITY, "specifiedId Object_SetFieldByName_Ref status: %{public}d", status);
            return;
        }
    }
}

void EtsUIAbility::OnStop()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "OnStop called");
    if (abilityContext_) {
        TAG_LOGD(AAFwkTag::UIABILITY, "set terminating true");
        abilityContext_->SetTerminating(true);
    }
    UIAbility::OnStop();
    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        EtsAbilityLifecycleCallbackArgs ability(etsAbilityObj_);
        applicationContext->DispatchOnAbilityWillDestroy(ability);
    }
    WriteLifecycleSwitchLog("onDestroy");
    CallObjectMethod(false, "onDestroy", nullptr);
    OnStopCallback();
    TAG_LOGD(AAFwkTag::UIABILITY, "OnStop end");
}

void EtsUIAbility::OnStop(AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback)
{
    if (callbackInfo == nullptr) {
        isAsyncCallback = false;
        OnStop();
        return;
    }
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "OnStop Begin");
    if (abilityContext_) {
        TAG_LOGD(AAFwkTag::UIABILITY, "set terminating true");
        abilityContext_->SetTerminating(true);
    }
    UIAbility::OnStop();
    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        EtsAbilityLifecycleCallbackArgs ability(etsAbilityObj_);
        applicationContext->DispatchOnAbilityWillDestroy(ability);
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

    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr || etsAbilityObj_ == nullptr) {
        isAsyncCallback = false;
        OnStop();
        return;
    }

    ani_long destroyCallbackPoint = reinterpret_cast<ani_long>(callbackInfo);
    ani_status status =
        env->Object_SetFieldByName_Long(etsAbilityObj_->aniObj, "destroyCallbackPoint", destroyCallbackPoint);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Object_SetFieldByName_Long status: %{public}d", status);
        return;
    }
    isAsyncCallback = CallObjectMethod(true, "callOnDestroy", ":z");
    TAG_LOGD(AAFwkTag::UIABILITY, "callOnDestroy isAsyncCallback: %{public}d", isAsyncCallback);
    if (!isAsyncCallback) {
        OnStopCallback();
        return;
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "OnStop end");
}

void EtsUIAbility::OnStopCallback()
{
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator(
        AbilityRuntime::Runtime::Language::ETS);
    auto property = std::make_shared<AppExecFwk::EtsDelegatorAbilityProperty>();
    if (delegator && CreateProperty(abilityContext_, property)) {
        TAG_LOGD(AAFwkTag::UIABILITY, "call PostPerformStop");
        property->object_ = etsAbilityObj_;
        delegator->PostPerformStop(property);
    }
    bool ret = ConnectionManager::GetInstance().DisconnectCaller(AbilityContext::token_);
    if (ret) {
        ConnectionManager::GetInstance().ReportConnectionLeakEvent(getpid(), gettid());
        TAG_LOGD(AAFwkTag::UIABILITY, "the service connection is not disconnected");
    }

    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr && etsAbilityObj_ != nullptr) {
        TAG_LOGD(AAFwkTag::UIABILITY, "call DispatchOnAbilityDestroy");
        EtsAbilityLifecycleCallbackArgs ability(etsAbilityObj_);
        applicationContext->DispatchOnAbilityDestroy(ability);
        DISPATCH_ABILITY_INTEROP(OnAbilityDestroy, applicationContext, etsRuntime_, ability);
    }
}

#ifdef SUPPORT_SCREEN
void EtsUIAbility::OnSceneCreated()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "ability: %{public}s", GetAbilityName().c_str());
    UIAbility::OnSceneCreated();
    auto etsAppWindowStage = CreateAppWindowStage();
    if (etsAppWindowStage == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null etsAppWindowStage");
        return;
    }
    UpdateEtsWindowStage(reinterpret_cast<ani_ref>(etsAppWindowStage));
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
        return;
    }
    etsWindowStageObj_ = std::make_shared<AppExecFwk::ETSNativeReference>();
    etsWindowStageObj_->aniObj = etsAppWindowStage;
    ani_ref entryObjectRef = nullptr;
    env->GlobalReference_Create(etsAppWindowStage, &entryObjectRef);
    etsWindowStageObj_->aniRef = entryObjectRef;
    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        EtsAbilityLifecycleCallbackArgs ability(etsAbilityObj_);
        EtsAbilityLifecycleCallbackArgs stage(etsWindowStageObj_);
        applicationContext->DispatchOnWindowStageWillCreate(ability, stage);
    }
    WriteLifecycleSwitchLog("onWindowStageCreate");
    CallObjectMethod(false, "onWindowStageCreate", nullptr, etsAppWindowStage);
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator(
        AbilityRuntime::Runtime::Language::ETS);
    auto property = std::make_shared<AppExecFwk::EtsDelegatorAbilityProperty>();
    if (delegator && CreateProperty(abilityContext_, property)) {
        TAG_LOGD(AAFwkTag::UIABILITY, "call PostPerformScenceCreated");
        property->object_ = etsAbilityObj_;
        delegator->PostPerformScenceCreated(property);
    }

    applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr && etsAbilityObj_ != nullptr && etsWindowStageObj_ != nullptr) {
        TAG_LOGD(AAFwkTag::UIABILITY, "call DispatchOnWindowStageCreate");
        EtsAbilityLifecycleCallbackArgs ability(etsAbilityObj_);
        EtsAbilityLifecycleCallbackArgs stage(etsWindowStageObj_);
        applicationContext->DispatchOnWindowStageCreate(ability, stage);
        DISPATCH_WINDOW_INTEROP(OnWindowStageCreate, applicationContext, etsRuntime_, ability, stage);
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "OnSceneCreated end");
}

ani_object EtsUIAbility::GetEtsWindowStage()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "GetEtsWindowStage called");

    if (shellContextRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null shellContextRef_");
        return nullptr;
    }

    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
        return nullptr;
    }

    ani_ref windowStageRef = nullptr;
    ani_status status = env->Object_GetFieldByName_Ref(shellContextRef_->aniObj, "windowStage", &windowStageRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Object_GetFieldByName_Ref failed, status: %{public}d", status);
        return nullptr;
    }

    ani_boolean isUndefined = true;
    if ((status = env->Reference_IsUndefined(windowStageRef, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Reference_IsUndefined status: %{public}d", status);
        return nullptr;
    }
    ani_boolean isNull = true;
    if ((status = env->Reference_IsNullishValue(windowStageRef, &isNull)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Reference_IsNullishValue status: %{public}d", status);
        return nullptr;
    }
    if (isUndefined || isNull) {
        TAG_LOGE(AAFwkTag::UIABILITY, "monitorObj is undefined or null");
        return nullptr;
    }

    return reinterpret_cast<ani_object>(windowStageRef);
}

void EtsUIAbility::OnSceneRestored()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    UIAbility::OnSceneRestored();
    TAG_LOGD(AAFwkTag::UIABILITY, "OnSceneRestored called");

    auto etsAppWindowStage = CreateAppWindowStage();
    if (etsAppWindowStage == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null etsAppWindowStage");
        return;
    }
    UpdateEtsWindowStage(reinterpret_cast<ani_ref>(etsAppWindowStage));
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
        return;
    }
    if (etsWindowStageObj_ && etsWindowStageObj_->aniRef) {
        env->GlobalReference_Delete(etsWindowStageObj_->aniRef);
    }
    etsWindowStageObj_ = std::make_shared<AppExecFwk::ETSNativeReference>();
    etsWindowStageObj_->aniObj = etsAppWindowStage;
    ani_ref entryObjectRef = nullptr;
    env->GlobalReference_Create(etsAppWindowStage, &entryObjectRef);
    etsWindowStageObj_->aniRef = entryObjectRef;
    EtsAbilityLifecycleCallbackArgs ability(etsAbilityObj_);
    EtsAbilityLifecycleCallbackArgs stage(etsWindowStageObj_);
    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        applicationContext->DispatchOnWindowStageWillRestore(ability, stage);
    }
    CallObjectMethod(false, "onWindowStageRestore", nullptr, etsAppWindowStage);
    applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        applicationContext->DispatchOnWindowStageRestore(ability, stage);
    }
}

void EtsUIAbility::OnSceneWillDestroy()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "ability: %{public}s", GetAbilityName().c_str());
    if (etsWindowStageObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null etsWindowStageObj_");
        return;
    }
    CallObjectMethod(false, "onWindowStageWillDestroy", nullptr, etsWindowStageObj_->aniRef);
}

void EtsUIAbility::onSceneDestroyed()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "ability: %{public}s", GetAbilityName().c_str());
    UIAbility::onSceneDestroyed();
    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        EtsAbilityLifecycleCallbackArgs ability(etsAbilityObj_);
        EtsAbilityLifecycleCallbackArgs stage(etsWindowStageObj_);
        applicationContext->DispatchOnWindowStageWillDestroy(ability, stage);
    }
    UpdateEtsWindowStage(nullptr);
    WriteLifecycleSwitchLog("onWindowStageDestroy");
    CallObjectMethod(false, "onWindowStageDestroy", nullptr);
    if (scene_ != nullptr) {
        auto window = scene_->GetMainWindow();
        if (window != nullptr) {
            TAG_LOGD(AAFwkTag::UIABILITY, "unRegisterDisplaymovelistener");
            window->UnregisterDisplayMoveListener(abilityDisplayMoveListener_);
        }
    }
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator(
        AbilityRuntime::Runtime::Language::ETS);
    auto property = std::make_shared<AppExecFwk::EtsDelegatorAbilityProperty>();
    if (delegator && CreateProperty(abilityContext_, property)) {
        TAG_LOGD(AAFwkTag::UIABILITY, "call PostPerformScenceDestroyed");
        property->object_ = etsAbilityObj_;
        delegator->PostPerformScenceDestroyed(property);
    }

    applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr && etsAbilityObj_ != nullptr && etsWindowStageObj_ != nullptr) {
        TAG_LOGD(AAFwkTag::UIABILITY, "call DispatchOnWindowStageDestroy");
        EtsAbilityLifecycleCallbackArgs ability(etsAbilityObj_);
        EtsAbilityLifecycleCallbackArgs stage(etsWindowStageObj_);
        applicationContext->DispatchOnWindowStageDestroy(ability, stage);
        DISPATCH_WINDOW_INTEROP(OnWindowStageDestroy, applicationContext, etsRuntime_, ability, stage);
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "onSceneDestroyed end");
}

void EtsUIAbility::OnForeground(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "ability: %{public}s", GetAbilityName().c_str());
    UIAbility::OnForeground(want);
    HandleCollaboration(want);
    if (CheckIsSilentForeground()) {
        TAG_LOGD(AAFwkTag::UIABILITY, "silent foreground, do not call 'onForeground'");
        return;
    }
    CallOnForegroundFunc(want);
}

void EtsUIAbility::CallOnForegroundFunc(const Want &want)
{
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
        return;
    }
    if (etsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null etsAbilityObj_");
        return;
    }
    ani_status status = ANI_ERROR;
    ani_ref wantRef = AppExecFwk::WrapWant(env, want);
    if (wantRef == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null wantObj");
        return;
    }
    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        EtsAbilityLifecycleCallbackArgs ability(etsAbilityObj_);
        applicationContext->DispatchOnAbilityWillForeground(ability);
    }
    if ((status = env->Object_SetFieldByName_Ref(etsAbilityObj_->aniObj, "lastRequestWant", wantRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "lastRequestWant Object_SetFieldByName_Ref status: %{public}d", status);
        return;
    }
    WriteLifecycleSwitchLog("onForeground");
    CallObjectMethod(false, "onForeground", nullptr, wantRef);
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator(
        AbilityRuntime::Runtime::Language::ETS);
    auto property = std::make_shared<AppExecFwk::EtsDelegatorAbilityProperty>();
    if (delegator && CreateProperty(abilityContext_, property)) {
        TAG_LOGD(AAFwkTag::UIABILITY, "call PostPerformForeground");
        property->object_ = etsAbilityObj_;
        delegator->PostPerformForeground(property);
    }

    applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        TAG_LOGD(AAFwkTag::UIABILITY, "call DispatchOnAbilityForeground");
        EtsAbilityLifecycleCallbackArgs ability(etsAbilityObj_);
        applicationContext->DispatchOnAbilityForeground(ability);
        DISPATCH_ABILITY_INTEROP(OnAbilityForeground, applicationContext, etsRuntime_, ability);
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "CallOnForegroundFunc end");
}

void EtsUIAbility::OnBackground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "ability: %{public}s", GetAbilityName().c_str());
    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        EtsAbilityLifecycleCallbackArgs ability(etsAbilityObj_);
        applicationContext->DispatchOnAbilityWillBackground(ability);
    }
    WriteLifecycleSwitchLog("onBackground");
    CallObjectMethod(false, "onBackground", nullptr);
    UIAbility::OnBackground();
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator(
        AbilityRuntime::Runtime::Language::ETS);
    auto property = std::make_shared<AppExecFwk::EtsDelegatorAbilityProperty>();
    if (delegator && CreateProperty(abilityContext_, property)) {
        TAG_LOGD(AAFwkTag::UIABILITY, "call PostPerformBackground");
        property->object_ = etsAbilityObj_;
        delegator->PostPerformBackground(property);
    }

    applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        auto env = etsRuntime_.GetAniEnv();
        if (env == nullptr || etsAbilityObj_ == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null env or ability");
            return;
        }
        TAG_LOGD(AAFwkTag::UIABILITY, "call DispatchOnAbilityBackground");
        EtsAbilityLifecycleCallbackArgs ability(etsAbilityObj_);
        applicationContext->DispatchOnAbilityBackground(ability);
        DISPATCH_ABILITY_INTEROP(OnAbilityBackground, applicationContext, etsRuntime_, ability);
    }
    auto want = GetWant();
    if (want != nullptr) {
        HandleCollaboration(*want);
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "OnBackground end");
}

bool EtsUIAbility::OnBackPress()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "OnBackPress ability: %{public}s", GetAbilityName().c_str());
    UIAbility::OnBackPress();
    bool ret = CallObjectMethod(true, "onBackPressed", nullptr);
    TAG_LOGD(AAFwkTag::UIABILITY, "ret: %{public}d", ret);
    bool defaultRet = BackPressDefaultValue();
    if (!ret) {
        TAG_LOGD(AAFwkTag::UIABILITY, "null etsValue, return defaultRet %{public}d", defaultRet);
        return defaultRet;
    }
    return ret;
}

void EtsUIAbility::OnPrepareTerminate(AppExecFwk::AbilityTransactionCallbackInfo<bool> *callbackInfo, bool &isAsync)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "OnPrepareTerminate ability: %{public}s", GetAbilityName().c_str());
    UIAbility::OnPrepareTerminate(callbackInfo, isAsync);
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr || etsAbilityObj_ == nullptr || callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env or etsAbilityObj_ or callbackInfo");
        return;
    }
    ani_long prepareToTerminateCallbackPointer = (ani_long)callbackInfo;
    ani_status status = env->Object_SetFieldByName_Long(
        etsAbilityObj_->aniObj, "prepareToTerminateCallbackPointer", prepareToTerminateCallbackPointer);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "prepareToTerminateCallbackPointer SetField status: %{public}d", status);
        return;
    }
    if (CallObjectMethod(true, "callOnPrepareToTerminateAsync", nullptr)) {
        TAG_LOGI(AAFwkTag::UIABILITY, "async call");
        isAsync = true;
        return;
    }
    TAG_LOGI(AAFwkTag::UIABILITY, "onPrepareToTerminateAsync not implemented, call onPrepareToTerminate");
    ani_method method = nullptr;
    if (etsAbilityObj_->aniCls == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null uiability ani class");
        return;
    }
    if ((status = env->Class_FindMethod(etsAbilityObj_->aniCls, "onPrepareToTerminate", nullptr, &method)) != ANI_OK ||
        method == nullptr) {
        if (status != ANI_PENDING_ERROR) {
            TAG_LOGE(AAFwkTag::UIABILITY, "onPrepareToTerminate FindMethod status: %{public}d, or null method", status);
        }
        TAG_LOGW(AAFwkTag::UIABILITY, "neither is implemented");
        env->ResetError();
        return;
    }
    ani_boolean isTerminateAni = false;
    if ((status = env->Object_CallMethod_Boolean(etsAbilityObj_->aniObj, method, &isTerminateAni)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "onPrepareToTerminate CallMethod status: %{public}d", status);
        return;
    }
    bool isTerminate = isTerminateAni;
    callbackInfo->Call(isTerminate);
}

ani_object EtsUIAbility::CreateAppWindowStage()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
        return nullptr;
    }
    auto scene = GetScene();
    if (scene == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "scene not found");
        return nullptr;
    }
    ani_object etsWindowStage = CreateAniWindowStage(env, scene);
    if (etsWindowStage == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null etsWindowStage");
        return nullptr;
    }
    return etsWindowStage;
}

void EtsUIAbility::GetPageStackFromWant(const Want &want, std::string &pageStack)
{
    auto stringObj = AAFwk::IString::Query(want.GetParams().GetParam(PAGE_STACK_PROPERTY_NAME));
    if (stringObj != nullptr) {
        pageStack = AAFwk::String::Unbox(stringObj);
    }
}

bool EtsUIAbility::IsRestorePageStack(const Want &want)
{
    return want.GetBoolParam(SUPPORT_CONTINUE_PAGE_STACK_PROPERTY_NAME, true);
}

void EtsUIAbility::RestorePageStack(const Want &want)
{
    if (!IsRestorePageStack(want)) {
        return;
    }
    std::string pageStack;
    GetPageStackFromWant(want, pageStack);
    if (scene_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null scene_");
        return;
    }
    auto window = scene_->GetMainWindow();
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null window");
        return;
    }
    if (abilityContext_->GetEtsContentStorage() == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null etsContentStorage");
        return;
    }
    auto localStorageRef = reinterpret_cast<ani_ref>(abilityContext_->GetEtsContentStorage());
    if (localStorageRef == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null localStorageRef");
        return;
    }
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
        return;
    }
    window->AniSetUIContent(pageStack, env, reinterpret_cast<ani_object>(localStorageRef),
        Rosen::BackupAndRestoreType::CONTINUATION);
}

void EtsUIAbility::AbilityContinuationOrRecover(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "launch reason: %{public}d, last exit reasion: %{public}d", launchParam_.launchReason,
        launchParam_.lastExitReason);
    if (IsRestoredInContinuation()) {
        RestorePageStack(want);
        OnSceneRestored();
        NotifyContinuationResult(want, true);
        return;
    }
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

void EtsUIAbility::DoOnForeground(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "DoOnForeground begin");
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
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "scene_->GoForeground");
    scene_->GoForeground(UIAbility::sceneFlag_);
    TAG_LOGD(AAFwkTag::UIABILITY, "DoOnForeground end");
}

void EtsUIAbility::OnWillForeground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "OnWillForeground ability: %{public}s", GetAbilityName().c_str());
    UIAbility::OnWillForeground();
    CallObjectMethod(false, "onWillForeground", UI_ABILITY_SIGNATURE_VOID);
}

void EtsUIAbility::OnDidForeground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "OnDidForeground ability: %{public}s", GetAbilityName().c_str());
    UIAbility::OnDidForeground();
    CallObjectMethod(false, "onDidForeground", UI_ABILITY_SIGNATURE_VOID);
    if (scene_ != nullptr) {
        scene_->GoResume();
    }
}

void EtsUIAbility::OnWillBackground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "OnWillBackground ability: %{public}s", GetAbilityName().c_str());
    UIAbility::OnWillBackground();
    CallObjectMethod(false, "onWillBackground", UI_ABILITY_SIGNATURE_VOID);
}

void EtsUIAbility::OnDidBackground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "OnDidBackground ability: %{public}s", GetAbilityName().c_str());
    UIAbility::OnDidBackground();
    CallObjectMethod(false, "onDidBackground", UI_ABILITY_SIGNATURE_VOID);
}

void EtsUIAbility::DoOnForegroundForSceneIsNull(const Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "DoOnForegroundForSceneIsNull begin");
    scene_ = std::make_shared<Rosen::WindowScene>();
    int32_t displayId = AAFwk::DisplayUtil::GetDefaultDisplayId();
    if (setting_ != nullptr) {
        std::string strDisplayId = setting_->GetProperty(AppExecFwk::AbilityStartSetting::WINDOW_DISPLAY_ID_KEY);
        std::regex formatRegex("[0-9]{0,9}$");
        std::smatch sm;
        bool flag = std::regex_match(strDisplayId, sm, formatRegex);
        if (flag && !strDisplayId.empty()) {
            displayId = strtol(strDisplayId.c_str(), nullptr, BASE_DISPLAY_ID_NUM);
            TAG_LOGD(AAFwkTag::UIABILITY, "displayId: %{public}d", displayId);
        } else {
            TAG_LOGW(AAFwkTag::UIABILITY, "formatRegex: [%{public}s] failed", strDisplayId.c_str());
        }
    }
    auto option = GetWindowOption(want);
    Rosen::WMError ret = Rosen::WMError::WM_OK;
    auto sessionToken = GetSessionToken();
    auto identityToken = GetIdentityToken();
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "scene_->Init");
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled() && sessionToken != nullptr) {
        abilityContext_->SetWeakSessionToken(sessionToken);
        RemoveShareRouterByBundleType(want);
        std::string navDestinationInfo = want.GetStringParam(Want::ATOMIC_SERVICE_SHARE_ROUTER);
        if (!navDestinationInfo.empty()) {
            TAG_LOGD(AAFwkTag::UIABILITY, "SetNavDestinationInfo :%{public}s", navDestinationInfo.c_str());
            scene_->SetNavDestinationInfo(navDestinationInfo);
        }
        ret = scene_->Init(displayId, abilityContext_, sceneListener_, option, sessionToken, identityToken);
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
        TAG_LOGD(AAFwkTag::UIABILITY, "registerDisplayMoveListener, windowId: %{public}d", window->GetWindowId());
        abilityDisplayMoveListener_ = new AbilityDisplayMoveListener(weak_from_this());
        if (abilityDisplayMoveListener_ == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null abilityDisplayMoveListener_");
            return;
        }
        window->RegisterDisplayMoveListener(abilityDisplayMoveListener_);
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "DoOnForegroundForSceneIsNull end");
}

void EtsUIAbility::RemoveShareRouterByBundleType(const Want &want)
{
    auto abilityInfo = abilityContext_ ? abilityContext_->GetAbilityInfo() : nullptr;
    if (abilityInfo == nullptr ||
        abilityInfo->applicationInfo.bundleType != OHOS::AppExecFwk::BundleType::ATOMIC_SERVICE) {
        TAG_LOGD(AAFwkTag::UIABILITY, "not atomicService");
        const_cast<Want &>(want).RemoveParam(Want::ATOMIC_SERVICE_SHARE_ROUTER);
    }
    return;
}
void EtsUIAbility::ContinuationRestore(const Want &want)
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

void EtsUIAbility::UpdateEtsWindowStage(ani_ref windowStage)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "UpdateEtsWindowStage called");
    if (shellContextRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null shellContextRef_");
        return;
    }
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
        return;
    }
    ani_status status = ANI_ERROR;
    if (windowStage == nullptr) {
        ani_ref nullRef = nullptr;
        env->GetNull(&nullRef);
        if ((status = env->Object_SetFieldByName_Ref(shellContextRef_->aniObj, "windowStage", nullRef)) !=
            ANI_OK) {
            TAG_LOGE(AAFwkTag::UIABILITY, "Object_SetFieldByName_Ref status: %{public}d", status);
            return;
        }
        return;
    }
    if ((status = env->Object_SetFieldByName_Ref(shellContextRef_->aniObj, "windowStage", windowStage)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Object_SetFieldByName_Ref status: %{public}d", status);
        return;
    }
}

std::unique_ptr<NativeReference> EtsUIAbility::CreateJsAppWindowStage()
{
    auto& jsRuntime = etsRuntime_.GetJsRuntime();
    auto &jsRuntimePoint = (static_cast<AbilityRuntime::JsRuntime &>(*jsRuntime));
    auto env = jsRuntimePoint.GetNapiEnv();
    napi_value jsWindowStage = Rosen::CreateJsWindowStage(env, GetScene());
    if (jsWindowStage == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null jsWindowStage");
        return nullptr;
    }
    return JsRuntime::LoadSystemModuleByEngine(env, "application.WindowStage", &jsWindowStage, 1);
}

void EtsUIAbility::ExecuteInsightIntentRepeateForeground(const Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "ExecuteInsightIntentRepeateForeground called");
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
        ability->RequestFocus(want);
    };
    callback->Push(asyncCallback);
    const WantParams &wantParams = want.GetParams();
    std::string arkTSMode = wantParams.GetStringParam(AppExecFwk::INSIGHT_INTENT_ARKTS_MODE);
    InsightIntentExecutorInfo executeInfo;
    if (arkTSMode == AbilityRuntime::CODE_LANGUAGE_ARKTS_1_2) {
        auto ret = GetInsightIntentExecutorInfo(want, executeParam, executeInfo, arkTSMode);
        if (!ret) {
            TAG_LOGE(AAFwkTag::UIABILITY, "get intentExecutor failed");
            InsightIntentExecutorMgr::TriggerCallbackInner(
                std::move(callback), static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
            return;
        }
        ret = DelayedSingleton<InsightIntentExecutorMgr>::GetInstance()->ExecuteInsightIntent(
            etsRuntime_, executeInfo, std::move(callback));
        if (!ret) {
            // callback has removed, release in insight intent executor.
            TAG_LOGE(AAFwkTag::UIABILITY, "execute insightIntent failed");
        }
    } else {
        auto jsAppWindowStage = CreateJsAppWindowStage();
        if (jsAppWindowStage == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null jsAppWindowStage");
            return;
        }
        jsWindowStageObj_ = std::shared_ptr<NativeReference>(jsAppWindowStage.release());
        auto& jsRuntime = etsRuntime_.GetJsRuntime();
        if (jsRuntime == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null jsRuntime");
            return;
        }
        if (abilityInfo_) {
            auto &jsRuntimePoint = (static_cast<AbilityRuntime::JsRuntime &>(*jsRuntime));
            jsRuntimePoint.UpdateModuleNameAndAssetPath(abilityInfo_->moduleName);
        }
        auto ret = GetInsightIntentExecutorInfo(want, executeParam, executeInfo, arkTSMode);
        if (!ret) {
            TAG_LOGE(AAFwkTag::UIABILITY, "get intentExecutor failed");
            InsightIntentExecutorMgr::TriggerCallbackInner(
                std::move(callback), static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
            return;
        }
        ret = DelayedSingleton<InsightIntentExecutorMgr>::GetInstance()->ExecuteInsightIntent(
            *jsRuntime, executeInfo, std::move(callback));
        if (!ret) {
            // callback has removed, release in insight intent executor.
            TAG_LOGE(AAFwkTag::UIABILITY, "execute insightIntent failed");
        }
    }
}

void EtsUIAbility::ExecuteInsightIntentMoveToForeground(const Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "ExecuteInsightIntentMoveToForeground called");
    if (executeParam == nullptr) {
        TAG_LOGW(AAFwkTag::UIABILITY, "null executeParam");
        OnForeground(want);
        InsightIntentExecutorMgr::TriggerCallbackInner(std::move(callback), ERR_OK);
        return;
    }

    if (abilityInfo_) {
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
    const WantParams &wantParams = want.GetParams();
    std::string arkTSMode = wantParams.GetStringParam(AppExecFwk::INSIGHT_INTENT_ARKTS_MODE);
    InsightIntentExecutorInfo executeInfo;
    auto delayResultCallback = [intentId = executeParam->insightIntentId_, token = token_]
        (AppExecFwk::InsightIntentExecuteResult result) -> int32_t {
        TAG_LOGD(AAFwkTag::UIABILITY, "UiAbility intent delayResultCallback");
        auto ret = AAFwk::AbilityManagerClient::GetInstance()->ExecuteInsightIntentDone(token, intentId, result);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::UIABILITY, "ExecuteInsightIntentDone ret : %{public}d", ret);
        }
        return ret;
    };
    InsightIntentDelayResultCallbackMgr::GetInstance().RemoveDelayResultCallback(intentId_);
    bool isDecorator = executeParam->decoratorType_ != static_cast<int8_t>(InsightIntentType::DECOR_NONE);
    if (arkTSMode == AbilityRuntime::CODE_LANGUAGE_ARKTS_1_2) {
        auto ret = GetInsightIntentExecutorInfo(want, executeParam, executeInfo, arkTSMode);
        if (!ret) {
            TAG_LOGE(AAFwkTag::UIABILITY, "get intentExecutor failed");
            InsightIntentExecutorMgr::TriggerCallbackInner(
                std::move(callback), static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
            return;
        }
        InsightIntentDelayResultCallbackMgr::GetInstance().AddDelayResultCallback(executeParam->insightIntentId_,
            {delayResultCallback, isDecorator});
        ret = DelayedSingleton<InsightIntentExecutorMgr>::GetInstance()->ExecuteInsightIntent(
            etsRuntime_, executeInfo, std::move(callback));
        if (!ret) {
            // callback has removed, release in insight intent executor.
            TAG_LOGE(AAFwkTag::UIABILITY, "execute insightIntent failed");
            InsightIntentDelayResultCallbackMgr::GetInstance().RemoveDelayResultCallback(
                executeParam->insightIntentId_);
        }
    } else {
        auto jsAppWindowStage = CreateJsAppWindowStage();
        if (jsAppWindowStage == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null jsAppWindowStage");
            return;
        }
        jsWindowStageObj_ = std::shared_ptr<NativeReference>(jsAppWindowStage.release());
        auto& jsRuntime = etsRuntime_.GetJsRuntime();
        if (jsRuntime == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null jsRuntime");
            return;
        }
        if (abilityInfo_) {
            auto &jsRuntimePoint = (static_cast<AbilityRuntime::JsRuntime &>(*jsRuntime));
            jsRuntimePoint.UpdateModuleNameAndAssetPath(abilityInfo_->moduleName);
        }
        auto ret = GetInsightIntentExecutorInfo(want, executeParam, executeInfo, arkTSMode);
        if (!ret) {
            TAG_LOGE(AAFwkTag::UIABILITY, "get intentExecutor failed");
            InsightIntentExecutorMgr::TriggerCallbackInner(
                std::move(callback), static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
            return;
        }
        InsightIntentDelayResultCallbackMgr::GetInstance().AddDelayResultCallback(executeParam->insightIntentId_,
            {delayResultCallback, isDecorator});
        ret = DelayedSingleton<InsightIntentExecutorMgr>::GetInstance()->ExecuteInsightIntent(
            *jsRuntime, executeInfo, std::move(callback));
        if (!ret) {
            // callback has removed, release in insight intent executor.
            TAG_LOGE(AAFwkTag::UIABILITY, "execute insightIntent failed");
            InsightIntentDelayResultCallbackMgr::GetInstance().RemoveDelayResultCallback(
                executeParam->insightIntentId_);
            return;
        }
    }
    intentId_ = executeInfo.executeParam->insightIntentId_;
}

void EtsUIAbility::ExecuteInsightIntentBackground(const Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "ExecuteInsightIntentBackground called");
    if (executeParam == nullptr) {
        TAG_LOGW(AAFwkTag::UIABILITY, "null executeParam");
        InsightIntentExecutorMgr::TriggerCallbackInner(std::move(callback), ERR_OK);
        return;
    }

    if (abilityInfo_) {
    }

    const WantParams &wantParams = want.GetParams();
    std::string arkTSMode = wantParams.GetStringParam(AppExecFwk::INSIGHT_INTENT_ARKTS_MODE);
    InsightIntentExecutorInfo executeInfo;
    if (arkTSMode == AbilityRuntime::CODE_LANGUAGE_ARKTS_1_2) {
        auto ret = GetInsightIntentExecutorInfo(want, executeParam, executeInfo, arkTSMode);
        if (!ret) {
            TAG_LOGE(AAFwkTag::UIABILITY, "get intentExecutor failed");
            InsightIntentExecutorMgr::TriggerCallbackInner(
                std::move(callback), static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
            return;
        }

        ret = DelayedSingleton<InsightIntentExecutorMgr>::GetInstance()->ExecuteInsightIntent(
            etsRuntime_, executeInfo, std::move(callback));
        if (!ret) {
            // callback has removed, release in insight intent executor.
            TAG_LOGE(AAFwkTag::UIABILITY, "execute insightIntent failed");
        }
    } else {
        auto& jsRuntime = etsRuntime_.GetJsRuntime();
        if (jsRuntime == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null jsRuntime");
            return;
        }
        if (abilityInfo_) {
            auto &jsRuntimePoint = (static_cast<AbilityRuntime::JsRuntime &>(*jsRuntime));
            jsRuntimePoint.UpdateModuleNameAndAssetPath(abilityInfo_->moduleName);
        }
        auto ret = GetInsightIntentExecutorInfo(want, executeParam, executeInfo, arkTSMode);
        if (!ret) {
            TAG_LOGE(AAFwkTag::UIABILITY, "get intentExecutor failed");
            InsightIntentExecutorMgr::TriggerCallbackInner(
                std::move(callback), static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
            return;
        }

        ret = DelayedSingleton<InsightIntentExecutorMgr>::GetInstance()->ExecuteInsightIntent(
            *jsRuntime, executeInfo, std::move(callback));
        if (!ret) {
            // callback has removed, release in insight intent executor.
            TAG_LOGE(AAFwkTag::UIABILITY, "execute insightIntent failed");
        }
    }
}

bool EtsUIAbility::GetInsightIntentExecutorInfo(const Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam, InsightIntentExecutorInfo &executeInfo,
    std::string arkTSMode)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "GetInsightIntentExecutorInfo called");

    auto context = GetAbilityContext();
    if (executeParam == nullptr || context == nullptr || abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "param invalid");
        return false;
    }

    if (arkTSMode == AbilityRuntime::CODE_LANGUAGE_ARKTS_1_2) {
        if (executeParam->executeMode_ == AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND &&
            etsWindowStageObj_ == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "param invalid");
            return false;
        }
    } else {
        if (executeParam->executeMode_ == AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND
            && jsWindowStageObj_ == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "param invalid");
            return false;
        }
    }

    const WantParams &wantParams = want.GetParams();
    executeInfo.srcEntry = wantParams.GetStringParam("ohos.insightIntent.srcEntry");
    executeInfo.hapPath = abilityInfo_->hapPath;
    executeInfo.esmodule = abilityInfo_->compileMode == AppExecFwk::CompileMode::ES_MODULE;
    executeInfo.windowMode = windowMode_;
    executeInfo.token = context->GetToken();
    if (arkTSMode == AbilityRuntime::CODE_LANGUAGE_ARKTS_1_2) {
        if (etsWindowStageObj_ != nullptr) {
            executeInfo.etsPageLoader = reinterpret_cast<void *>(etsWindowStageObj_->aniRef);
        }
    } else {
        if (jsWindowStageObj_ != nullptr) {
            executeInfo.pageLoader = jsWindowStageObj_;
        }
    }
    executeInfo.executeParam = executeParam;
    return true;
}

int32_t EtsUIAbility::OnCollaborate(WantParams &wantParam)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "OnCollaborate: %{public}s", GetAbilityName().c_str());
    int32_t ret = CollaborateResult::REJECT;
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
        return ret;
    }
    if (etsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null etsAbilityObj_");
        return ret;
    }
    ani_method method = nullptr;
    ani_status status = env->Class_FindMethod(etsAbilityObj_->aniCls, "onCollaborate", ON_COLLABORATE, &method);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "onCollaborate FindMethod status: %{public}d, or null method", status);
        return ret;
    }
    ani_ref wantParamsRef = AppExecFwk::WrapWantParams(env, wantParam);
    if (wantParamsRef == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null wantParamsRef");
        return ret;
    }
    ani_value args[] = { { .r = wantParamsRef } };
    ani_ref result = nullptr;
    if ((status = env->Object_CallMethod_Ref_A(etsAbilityObj_->aniObj, method, &result, args)) != ANI_OK ||
        result == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "CallMethod status: %{public}d, or null result", status);
        return ret;
    }
    AppExecFwk::UnwrapWantParams(env, wantParamsRef, wantParam);
    if (!AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative(env, reinterpret_cast<ani_enum_item>(result), ret)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "EnumConvert_EtsToNative failed");
        return ret;
    }
    ret = (ret == CollaborateResult::ACCEPT) ? CollaborateResult::ACCEPT : CollaborateResult::REJECT;
    return ret;
}
#endif

void EtsUIAbility::OnAbilityRequestFailure(const std::string &requestId, const AppExecFwk::ElementName &element,
    const std::string &message, int32_t resultCode)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "OnAbilityRequestFailure called");
    UIAbility::OnAbilityRequestFailure(requestId, element, message);
    auto abilityContext = GetAbilityContext();
    if (abilityContext == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityContext");
        return;
    }
    abilityContext->OnRequestFailure(requestId, element, message, resultCode);
}

void EtsUIAbility::OnAbilityRequestSuccess(const std::string &requestId, const AppExecFwk::ElementName &element,
    const std::string &message)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "OnAbilityRequestSuccess called");
    UIAbility::OnAbilityRequestSuccess(requestId, element, message);
    auto abilityContext = GetAbilityContext();
    if (abilityContext == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityContext");
        return;
    }
    abilityContext->OnRequestSuccess(requestId, element, message);
}

void EtsUIAbility::OnConfigurationUpdated(const Configuration &configuration)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    UIAbility::OnConfigurationUpdated(configuration);
    TAG_LOGD(AAFwkTag::UIABILITY, "OnConfigurationUpdated called");
    if (abilityContext_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityContext");
        return;
    }
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "env null");
        return;
    }
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
    ani_object aniConfiguration = AppExecFwk::WrapConfiguration(env, realConfig);
    CallObjectMethod(false, "onConfigurationUpdated", nullptr, aniConfiguration);
    CallObjectMethod(false, "onConfigurationUpdate", nullptr, aniConfiguration);
    auto realConfigPtr = std::make_shared<Configuration>(realConfig);
    if (shellContextRef_ != nullptr) {
        EtsAbilityContext::ConfigurationUpdated(env, shellContextRef_->aniObj, realConfigPtr);
    }
    abilityContext_->NotifyBindingObjectConfigUpdate();
}

int32_t EtsUIAbility::OnContinue(AAFwk::WantParams &wantParams, bool &isAsyncOnContinue,
    const AppExecFwk::AbilityInfo &abilityInfo)
{
    using namespace AppExecFwk;
    auto aniEnv = etsRuntime_.GetAniEnv();
    if (aniEnv == nullptr || etsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "env null or etsAbilityObj_ null");
        return ContinuationManagerStage::ON_CONTINUE_ERR;
    }
    ani_method method = nullptr;
    ani_status status = aniEnv->Class_FindMethod(etsAbilityObj_->aniCls, "callOnContinue", nullptr, &method);
    if (status != ANI_OK || method == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "FindMethod callOnContinue failed %{public}d, or null method", status);
        return ContinuationManagerStage::ON_CONTINUE_ERR;
    }
    ani_ref wantParamRef = AppExecFwk::WrapWantParams(aniEnv, wantParams);
    if (wantParamRef == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null wantParamRef");
        return ContinuationManagerStage::ON_CONTINUE_ERR;
    }
    EtsAbilityLifecycleCallbackArgs ability(etsAbilityObj_);
    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        applicationContext->DispatchOnAbilityWillContinue(ability);
    }
    int32_t continueResult = ContinuationManagerStage::ON_CONTINUE_ERR;
    bool isPromise = false;
    bool callRet = CallObjectOnContinueMethod(method, wantParamRef, continueResult, isPromise);
    if (!callRet) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Failed to CallObjectOnContinueMethod");
        return ContinuationManagerStage::ON_CONTINUE_ERR;
    }
    if (isPromise) {
        if (true == OnContinueReturnPromise(aniEnv, wantParamRef, abilityInfo)) {
            isAsyncOnContinue = true;
            return ContinuationManagerStage::AGREE;
        }
        return ContinuationManagerStage::ON_CONTINUE_ERR;
    }
    TAG_LOGI(AAFwkTag::UIABILITY, "CallObjectOnContinueMethod, continueResult = %{public}d", continueResult);
    if (!AppExecFwk::UnwrapWantParams(aniEnv, wantParamRef, wantParams)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Failed to UnwrapWantParams");
        return ContinuationManagerStage::ON_CONTINUE_ERR;
    }
    if (applicationContext != nullptr) {
        applicationContext->DispatchOnAbilityContinue(ability);
    }
    return (continueResult == ContinuationManagerStage::AGREE) ?
        ContinuationManagerStage::AGREE : ContinuationManagerStage::REJECT;
}

bool EtsUIAbility::OnContinueReturnPromise(ani_env* aniEnv,
    ani_ref aniWantParamRef, const AppExecFwk::AbilityInfo &abilityInfo)
{
    TAG_LOGI(AAFwkTag::UIABILITY, "OnContinueReturnPromise called");
    if (aniEnv == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "env null");
        return false;
    }
    auto *callbackInfo = AppExecFwk::AbilityTransactionCallbackInfo<int32_t>::Create();
    if (callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null callbackInfo");
        return false;
    }
    ani_ref globalWantRef = nullptr;
    if (ANI_OK != aniEnv->GlobalReference_Create(aniWantParamRef, &globalWantRef) || globalWantRef == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "GlobalReference_Create failed");
        AppExecFwk::AbilityTransactionCallbackInfo<int32_t>::Destroy(callbackInfo);
        return false;
    }
    callbackInfo->SetFinalizeCallback([globalWantRef] (void* data) {
        ani_env* env = reinterpret_cast<ani_env*>(data);
        if (env != nullptr && globalWantRef != nullptr) {
            env->GlobalReference_Delete(globalWantRef);
        }
    });
    ani_long continueCallbackInfo = reinterpret_cast<ani_long>(callbackInfo);
    ani_status status = aniEnv->Object_SetFieldByName_Long(
        etsAbilityObj_->aniObj, "continueCallbackPoint", continueCallbackInfo);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Object_SetFieldByName_Long status: %{public}d", status);
        AppExecFwk::AbilityTransactionCallbackInfo<int32_t>::Destroy(callbackInfo, aniEnv);
        return false;
    }
    auto weakThis = weak_from_this();
    auto asyncCallback =
        [globalWantRef, weakThis, capturedInfo = abilityInfo] (int32_t result) {
            auto ability = weakThis.lock();
            if (ability == nullptr) {
                TAG_LOGE(AAFwkTag::UIABILITY, "null ability");
                return;
            }
            ability->OnContinueAsyncCB(globalWantRef, result, capturedInfo);
        };
    callbackInfo->Push(asyncCallback);
    return true;
}

int32_t EtsUIAbility::OnContinueAsyncCB(ani_ref aniWantParamsRef, int32_t status,
    const AppExecFwk::AbilityInfo &abilityInfo)
{
    TAG_LOGI(AAFwkTag::UIABILITY, "called");
    auto aniEnv = etsRuntime_.GetAniEnv();
    if (aniEnv == nullptr || etsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "env null or etsAbilityObj_ null");
        return ERR_INVALID_VALUE;
    }
    AAFwk::WantParams wantParams;
    if (!AppExecFwk::UnwrapWantParams(aniEnv, aniWantParamsRef, wantParams)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Failed to UnwrapWantParams");
        return ERR_INVALID_VALUE;
    }
    EtsAbilityLifecycleCallbackArgs ability(etsAbilityObj_);
    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        applicationContext->DispatchOnAbilityContinue(ability);
    }

    Want want;
    want.SetParams(wantParams);
    want.AddFlags(want.FLAG_ABILITY_CONTINUATION);
    want.SetElementName(abilityInfo.deviceId, abilityInfo.bundleName, abilityInfo.name, abilityInfo.moduleName);
    int result = AAFwk::AbilityManagerClient::GetInstance()->StartContinuation(want, token_, status);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "StartContinuation failed: %{public}d", result);
    }
    TAG_LOGI(AAFwkTag::UIABILITY, "end");
    return result;
}

bool EtsUIAbility::CallObjectOnContinueMethod(ani_method method, ani_ref wantParamRef,
    int32_t& onContinueResult, bool& isPromise)
{
    TAG_LOGE(AAFwkTag::UIABILITY, "CallObjectOnContinueMethod");
    auto aniEnv = etsRuntime_.GetAniEnv();
    if (aniEnv == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "env null");
        return false;
    }
    if (etsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null etsAbilityObj_");
        return false;
    }
    isPromise = false;
    ani_value args[] = { { .r = wantParamRef } };
    ani_ref result = nullptr;
    ani_status status = ANI_OK;
    if ((status = aniEnv->Object_CallMethod_Ref_A(etsAbilityObj_->aniObj, method, &result, args)) != ANI_OK ||
        result == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "CallMethod status: %{public}d, or null result", status);
        ani_boolean unhandleException = false;
        if (ANI_OK == aniEnv->ExistUnhandledError(&unhandleException) && unhandleException) {
            TAG_LOGE(AAFwkTag::UIABILITY, "CallObjectOnContinueMethod, unhandleException, reset");
            aniEnv->ResetError();
        }
        return false;
    }
    ani_boolean isUndefined = ANI_FALSE;
    aniEnv->Reference_IsUndefined(result, &isUndefined);
    if (isUndefined) {
        TAG_LOGI(AAFwkTag::UIABILITY, "wait promise back");
        isPromise = true;
        return true;
    }
    if (!AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative(aniEnv,
        reinterpret_cast<ani_enum_item>(result), onContinueResult)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "EnumConvert_EtsToNative failed");
        return false;
    }
    return true;
}

void EtsUIAbility::OnMemoryLevel(int level)
{
    UIAbility::OnMemoryLevel(level);
    TAG_LOGD(AAFwkTag::UIABILITY, "OnMemoryLevel called");
    if (etsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null etsAbilityObj_");
        return;
    }
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
        return;
    }
    ani_enum_item levelEnum {};
    if (!AAFwk::AniEnumConvertUtil::EnumConvert_NativeToEts(env, MEMORY_LEVEL_ENUM_NAME, level, levelEnum)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "levelEnum NativeToEts failed");
        return;
    }
    CallObjectMethod(false, "onMemoryLevel", nullptr, levelEnum);
}

void EtsUIAbility::UpdateContextConfiguration()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "UpdateContextConfiguration called");
    if (abilityContext_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityContext_");
        return;
    }
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
        return;
    }
    if (shellContextRef_ != nullptr) {
        EtsAbilityContext::ConfigurationUpdated(env, shellContextRef_->aniObj, abilityContext_->GetConfiguration());
    }
    abilityContext_->NotifyBindingObjectConfigUpdate();
}

void EtsUIAbility::OnNewWant(const Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "OnNewWant called");
    UIAbility::OnNewWant(want);

#ifdef SUPPORT_SCREEN
    if (scene_) {
        scene_->OnNewWant(want);
    }
    HandleCollaboration(want);
#endif
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
        return;
    }
    if (etsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null etsAbilityObj_");
        return;
    }
    ani_object wantObj = AppExecFwk::WrapWant(env, want);
    if (wantObj == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null wantObj");
        return;
    }
    EtsAbilityLifecycleCallbackArgs ability(etsAbilityObj_);
    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        applicationContext->DispatchOnWillNewWant(ability);
    }
    ani_status status = env->Object_SetFieldByName_Ref(etsAbilityObj_->aniObj, "lastRequestWant", wantObj);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "lastRequestWant Object_SetFieldByName_Ref status: %{public}d", status);
        return;
    }
    auto launchParam = GetLaunchParam();
    if (InsightIntentExecuteParam::IsInsightIntentExecute(want)) {
        launchParam.launchReason = AAFwk::LaunchReason::LAUNCHREASON_INSIGHT_INTENT;
    }
    ani_object launchParamObj = nullptr;
    if (!WrapLaunchParam(env, launchParam, launchParamObj)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "WrapLaunchParam failed");
        return;
    }
    WriteLifecycleSwitchLog("onNewWant");
    CallObjectMethod(false, "onNewWant", nullptr, wantObj, launchParamObj);
    applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        applicationContext->DispatchOnNewWant(ability);
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "OnNewWant end");
}

void EtsUIAbility::OnAbilityResult(int requestCode, int resultCode, const Want &resultData)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "OnAbilityResult called");
    UIAbility::OnAbilityResult(requestCode, resultCode, resultData);
    if (abilityContext_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityContext_");
        return;
    }
    abilityContext_->OnAbilityResult(requestCode, resultCode, resultData);
    TAG_LOGD(AAFwkTag::UIABILITY, "OnAbilityResult end");
}

void EtsUIAbility::Dump(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    UIAbility::Dump(params, info);
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr || etsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env or etsAbilityObj");
        return;
    }
    ani_object arrayObj = nullptr;
    if (!AppExecFwk::WrapArrayString(env, arrayObj, params)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "WrapArrayString failed");
        return;
    }
    if (!etsAbilityObj_->aniObj || !etsAbilityObj_->aniCls) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null aniObj or aniCls");
        return;
    }
    ani_status status = ANI_ERROR;
    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(etsAbilityObj_->aniCls, "onDump", nullptr, &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Class_FindMethod FAILED: %{public}d", status);
        return;
    }
    if (!method) {
        TAG_LOGE(AAFwkTag::UIABILITY, "find method onDump failed");
        return;
    }
    ani_ref strArrayRef;
    if ((status = env->Object_CallMethod_Ref(etsAbilityObj_->aniObj, method, &strArrayRef, arrayObj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Object_CallMethod_Ref FAILED: %{public}d", status);
        return;
    }
    if (!strArrayRef) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null strArrayRef");
        return;
    }
    std::vector<std::string> dumpInfoStrArray;
    if (!AppExecFwk::UnwrapArrayString(env, reinterpret_cast<ani_object>(strArrayRef), dumpInfoStrArray)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "UnwrapArrayString failed");
        return;
    }
    for (auto dumpInfoStr:dumpInfoStrArray) {
        info.push_back(dumpInfoStr);
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "dump info size: %{public}zu", info.size());
}

sptr<IRemoteObject> EtsUIAbility::CallRequest()
{
    TAG_LOGI(AAFwkTag::UIABILITY, "CallRequest");
    if (etsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityContext_");
        return nullptr;
    }

    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
        return nullptr;
    }
    auto obj = etsAbilityObj_->aniObj;
    ani_status status = ANI_ERROR;
    ani_ref calleeRef = nullptr;
    status = env->Object_GetFieldByName_Ref(obj, "callee", &calleeRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "get callee: %{public}d", status);
        return nullptr;
    }
    env->Object_CallMethodByName_Void(reinterpret_cast<ani_object>(calleeRef), "setNewRuleFlag",
        "z:", ani_boolean(IsUseNewStartUpRule()));
    ani_long nativePtr = 0;
    env->Object_CallMethodByName_Long(reinterpret_cast<ani_object>(calleeRef), "getNativePtr",
        ":l", &nativePtr);
    sptr<IRemoteObject> remoteObj(reinterpret_cast<IRemoteObject*>(nativePtr));
    if (remoteObj == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "AniGetNativeRemoteObject null");
    }
    return remoteObj;
}

void EtsUIAbility::OnAfterFocusedCommon(bool isFocused)
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
    EtsAbilityLifecycleCallbackArgs ability(etsAbilityObj_);
    EtsAbilityLifecycleCallbackArgs stage(etsWindowStageObj_);
    if (isFocused) {
        applicationContext->DispatchWindowStageFocus(ability, stage);
    } else {
        applicationContext->DispatchWindowStageUnfocus(ability, stage);
    }
}

bool EtsUIAbility::CallObjectMethod(bool withResult, const char *name, const char *signature, ...)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, std::string("CallObjectMethod:") + name);
    TAG_LOGI(AAFwkTag::UIABILITY, "EtsUIAbility call ets, name: %{public}s", name);
    if (etsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null etsAbilityObj");
        return false;
    }
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
        return false;
    }
    auto obj = etsAbilityObj_->aniObj;
    auto cls = etsAbilityObj_->aniCls;

    ani_method method = nullptr;
    ani_status status = env->Class_FindMethod(cls, name, signature, &method);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Class_FindMethod status: %{public}d", status);
        env->ResetError();
        return false;
    }
    if (withResult) {
        ani_boolean res = ANI_FALSE;
        va_list args;
        va_start(args, signature);
        if ((status = env->Object_CallMethod_Boolean_V(obj, method, &res, args)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::UIABILITY, "Object_CallMethod_Boolean_V status: %{public}d", status);
            etsRuntime_.HandleUncaughtError();
            va_end(args);
            return false;
        }
        va_end(args);
        return res;
    }
    va_list args;
    va_start(args, signature);
    if ((status = env->Object_CallMethod_Void_V(obj, method, args)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Object_CallMethod_Void_V status: %{public}d", status);
        etsRuntime_.HandleUncaughtError();
        va_end(args);
        return false;
    }
    va_end(args);
    TAG_LOGI(AAFwkTag::UIABILITY, "CallObjectMethod end, name: %{public}s", name);
    return false;
}

int32_t EtsUIAbility::OnSaveState(int32_t reason, WantParams &wantParams,
    AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnSaveStateResult> *callbackInfo, bool &isAsync,
    AppExecFwk::StateReason stateReason)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "OnSaveState called");
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr || etsAbilityObj_ == nullptr || callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env or etsAbilityObj_ or callbackInfo");
        return CALL_BACK_ERROR;
    }
    ani_long saveStateCallbackPoint = reinterpret_cast<ani_long>(callbackInfo);
    ani_status status =
        env->Object_SetFieldByName_Long(etsAbilityObj_->aniObj, "saveStateCallbackPoint", saveStateCallbackPoint);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "SetField saveStateCallbackPoint failed, status: %{public}d", status);
        return CALL_BACK_ERROR;
    }
    ani_enum_item reasonEnum = nullptr;
    if (!AAFwk::AniEnumConvertUtil::EnumConvert_NativeToEts(env, ON_SAVE_RESULT_ENUM_NAME, reason, reasonEnum) ||
        reasonEnum == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "EnumConvert_NativeToEts failed, or null reasonEnum");
        return CALL_BACK_ERROR;
    }
    ani_ref wantParamsRef = AppExecFwk::WrapWantParams(env, wantParams);
    if (wantParamsRef == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "WrapWantParams failed, null wantParamsRef");
        return CALL_BACK_ERROR;
    }
    if (CallObjectMethod(true, "callOnSaveState", nullptr, reasonEnum, wantParamsRef,
        static_cast<ani_int>(stateReason))) {
        TAG_LOGD(AAFwkTag::UIABILITY, "async call");
        isAsync = true;
        return ERR_OK;
    }
    ani_value args[] = { { .r = reasonEnum }, { .r = wantParamsRef } };
    return CallSaveState(args, wantParams, stateReason, callbackInfo);
}

int32_t EtsUIAbility::CallSaveState(ani_value args[], WantParams &wantParams, AppExecFwk::StateReason stateReason,
    AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnSaveStateResult> *callbackInfo)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "CallSaveState called");
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr || etsAbilityObj_ == nullptr || callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env or etsAbilityObj_ or callbackInfo");
        return CALL_BACK_ERROR;
    }
    ani_method method = nullptr;
    ani_status status = env->Class_FindMethod(etsAbilityObj_->aniCls, "onSaveState", nullptr, &method);
    if (status != ANI_OK || method == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "FindMethod onSaveState failed, status: %{public}d", status);
        return CALL_BACK_ERROR;
    }
    ani_ref result = nullptr;
    status = env->Object_CallMethod_Ref_A(etsAbilityObj_->aniObj, method, &result, args);
    if (status != ANI_OK || result == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "onSaveState CallMethod failed, status: %{public}d", status);
        return CALL_BACK_ERROR;
    }
    ani_ref wantParamsRef = args[ON_SAVESTATE_INDEX_ONE].r;
    if (!AppExecFwk::UnwrapWantParams(env, wantParamsRef, wantParams)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "UnwrapWantParams failed");
        AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnSaveStateResult>::Destroy(callbackInfo);
        return CALL_BACK_ERROR;
    }
    int32_t numberResult = 0;
    if (!AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative(
        env, reinterpret_cast<ani_enum_item>(result), numberResult)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "EnumConvert_EtsToNative failed, no result from onSaveState");
        return CALL_BACK_ERROR;
    }
    AppExecFwk::OnSaveStateResult saveStateResult = { numberResult, wantParams, stateReason };
    callbackInfo->Call(saveStateResult);
    AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnSaveStateResult>::Destroy(callbackInfo);
    return numberResult;
}

bool EtsUIAbility::CheckSatisfyTargetAPIVersion(int32_t version)
{
    auto applicationInfo = GetApplicationInfo();
    if (!applicationInfo) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null targetAPIVersion");
        return false;
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "targetAPIVersion: %{public}d", applicationInfo->apiTargetVersion);
    return applicationInfo->apiTargetVersion % API_VERSION_MOD >= version;
}

bool EtsUIAbility::BackPressDefaultValue()
{
    return CheckSatisfyTargetAPIVersion(API12) ? true : false;
}
} // namespace AbilityRuntime
} // namespace OHOS

ETS_EXPORT extern "C" OHOS::AbilityRuntime::UIAbility *OHOS_ETS_Ability_Create(
    const std::unique_ptr<OHOS::AbilityRuntime::Runtime> &runtime)
{
    return OHOS::AbilityRuntime::EtsUIAbility::Create(runtime);
}

ETS_EXPORT extern "C" void OHOS_CreateAndBindETSUIAbilityContext(
    const std::shared_ptr<OHOS::AbilityRuntime::AbilityContext> &abilityContext,
    const std::unique_ptr<OHOS::AbilityRuntime::Runtime> &runtime)
{
    OHOS::AbilityRuntime::EtsUIAbility::CreateAndBindContext(abilityContext, runtime);
}
