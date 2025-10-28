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
#include "insight_intent_executor_info.h"
#include "insight_intent_executor_mgr.h"
#include "insight_intent_execute_param.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi_common_want.h"
#include "napi/native_api.h"
#include "ohos_application.h"
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
constexpr const char *UI_ABILITY_CLASS_NAME = "L@ohos/app/ability/UIAbility/UIAbility;";
constexpr const char *UI_ABILITY_SIGNATURE_VOID = ":V";
constexpr const char *MEMORY_LEVEL_ENUM_NAME = "L@ohos/app/ability/AbilityConstant/AbilityConstant/MemoryLevel;";
constexpr const char *ON_SAVE_RESULT_ENUM_NAME = "@ohos.app.ability.AbilityConstant.AbilityConstant.OnSaveResult";
constexpr int32_t ON_SAVESTATE_INDEX = 2;
constexpr int32_t ON_SAVESTATE_INDEX_ONE = 1;
constexpr const int32_t CALL_BACK_ERROR = -1;

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
}

void EtsUIAbility::Init(std::shared_ptr<AppExecFwk::AbilityLocalRecord> record,
    const std::shared_ptr<OHOSApplication> application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
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
    UIAbility::Init(record, application, handler, token);
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
            ani_native_function { "nativeOnDestroyCallback", ":V", reinterpret_cast<void *>(OnDestroyPromiseCallback) },
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
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "OnStart end");
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
    isAsyncCallback = CallObjectMethod(true, "callOnDestroy", ":Z");
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
    if (applicationContext != nullptr) {
        TAG_LOGD(AAFwkTag::UIABILITY, "call DispatchOnAbilityDestroy");
        EtsAbilityLifecycleCallbackArgs ability(etsAbilityObj_);
        applicationContext->DispatchOnAbilityDestroy(ability);
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
    if (applicationContext != nullptr) {
        TAG_LOGD(AAFwkTag::UIABILITY, "call DispatchOnWindowStageCreate");
        EtsAbilityLifecycleCallbackArgs ability(etsAbilityObj_);
        EtsAbilityLifecycleCallbackArgs stage(etsWindowStageObj_);
        applicationContext->DispatchOnWindowStageCreate(ability, stage);
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "OnSceneCreated end");
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
    if (applicationContext != nullptr) {
        TAG_LOGD(AAFwkTag::UIABILITY, "call DispatchOnWindowStageDestroy");
        EtsAbilityLifecycleCallbackArgs ability(etsAbilityObj_);
        EtsAbilityLifecycleCallbackArgs stage(etsWindowStageObj_);
        applicationContext->DispatchOnWindowStageDestroy(ability, stage);
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "onSceneDestroyed end");
}

void EtsUIAbility::OnForeground(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "ability: %{public}s", GetAbilityName().c_str());
    UIAbility::OnForeground(want);
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
    return ret;
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
    if (IsRestorePageStack(want)) {
        std::string pageStack;
        GetPageStackFromWant(want, pageStack);
        // to be done: AniSetUIContent
    }
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
#endif

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
    EtsAbilityContext::ConfigurationUpdated(env, shellContextRef_, realConfigPtr);
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
    EtsAbilityContext::ConfigurationUpdated(env, shellContextRef_, abilityContext_->GetConfiguration());
}

void EtsUIAbility::OnNewWant(const Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "OnNewWant called");
    UIAbility::OnNewWant(want);

#ifdef SUPPORT_SCREEN
    if (scene_) {
        scene_->OnNewWant(want);
    }
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
    std::string methodName = "OnNewWant";
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
        return false;
    }
    va_end(args);
    TAG_LOGI(AAFwkTag::UIABILITY, "CallObjectMethod end, name: %{public}s", name);
    return false;
}

int32_t EtsUIAbility::OnSaveState(int32_t reason, WantParams &wantParams,
    AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnSaveStateResult> *callbackInfo,
    bool &isAsync, AppExecFwk::StateReason stateReason)
{
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr || etsAbilityObj_  == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env or etsAbilityObj_");
        return CALL_BACK_ERROR;
    }

    EtsAbilityLifecycleCallbackArgs ability(etsAbilityObj_);
    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        applicationContext->DispatchOnAbilityWillSaveState(ability);
    }

    ani_method method = nullptr;
    ani_status status = env->Class_FindMethod(etsAbilityObj_->aniCls, "onSaveState", nullptr, &method);
    if (status != ANI_OK || method == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "onSaveState FindMethod status: %{public}d, or null method", status);
        return CALL_BACK_ERROR;
    }

    ani_enum_item reasonEnum = nullptr;
    if (!AAFwk::AniEnumConvertUtil::EnumConvert_NativeToEts(env, ON_SAVE_RESULT_ENUM_NAME, reason, reasonEnum) ||
        reasonEnum == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "reasonEnum failed, or null reasonEnum");
        return CALL_BACK_ERROR;
    }

    ani_ref wantParamsRef = AppExecFwk::WrapWantParams(env, wantParams);
    if (wantParamsRef == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null wantParamsRef");
        return CALL_BACK_ERROR;
    }

    ani_value args[ON_SAVESTATE_INDEX] = {};
    args[0].r = reasonEnum;
    args[ON_SAVESTATE_INDEX_ONE].r = wantParamsRef;
    ani_ref result = nullptr;
    if ((status = env->Object_CallMethod_Ref_A(etsAbilityObj_->aniObj, method, &result, args)) != ANI_OK ||
        result == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Object_CallMethod_Ref_A status: %{public}d, or null result", status);
        return CALL_BACK_ERROR;
    }
    AppExecFwk::UnwrapWantParams(env, wantParamsRef, wantParams);

    int32_t numberResult = 0;
    if (!AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative(
        env, reinterpret_cast<ani_enum_item>(result), numberResult)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "no result return from onSaveState");
        return CALL_BACK_ERROR;
    }

    if (applicationContext != nullptr) {
        applicationContext->DispatchOnAbilitySaveState(ability);
    }
    AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnSaveStateResult>::Destroy(callbackInfo);
    return numberResult;
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
