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
#include "ets_application_context_utils.h"

#include <mutex>

#include "ability_runtime_error_util.h"
#include "ani_enum_convert.h"
#include "application_context_manager.h"
#include "ets_context_utils.h"
#include "ets_error_utils.h"
#include "ets_interop_ability_lifecycle_callback.h"
#include "ets_native_reference.h"
#include "hilog_tag_wrapper.h"
#ifdef SUPPORT_SCREEN
#include "ui_ability.h"
#endif

namespace OHOS {
namespace AbilityRuntime {
namespace {
static std::once_flag g_bindNativeMethodsFlag;
constexpr const char* ETS_APPLICATION_CONTEXT_CLASS_NAME = "application.ApplicationContext.ApplicationContext";
constexpr const char* CLEANER_CLASS = "application.ApplicationContext.Cleaner";
constexpr const char* ETS_EVENT_HUB_CLASS_NAME = "application.EventHub.EventHub";
constexpr double FOUNT_SIZE = 0.0;
constexpr int32_t ERROR_ID = -1;
constexpr double ERROR_CODE_NULL_ENV = -1;
constexpr double ERROR_CODE_NULL_CALLBACK = -2;
constexpr double ERROR_CODE_NULL_CONTEXT = -3;
constexpr double ERROR_CODE_INVALID_PARAM = -4;
const std::string TYPE_ABILITY_LIFECYCLE = "abilityLifecycle";
#ifdef SUPPORT_SCREEN
constexpr const char* CLASSNAME_ARRAY = "std.core.Array";
#endif
}

std::mutex EtsApplicationContextUtils::abilityLifecycleCallbackLock_;
std::shared_ptr<EtsAbilityLifecycleCallback> EtsApplicationContextUtils::abilityLifecycleCallback_ = nullptr;


std::mutex g_interopAbilityLifecycleCallbackLock;
std::shared_ptr<EtsInteropAbilityLifecycleCallback> interopAbilityLifecycleCallback_ = nullptr;

void EtsApplicationContextUtils::Clean(ani_env *env, ani_object object)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Clean Call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    ani_long ptr = 0;
    ani_status status = env->Object_GetFieldByName_Long(object, "ptr", &ptr);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "ptr GetField status: %{public}d", status);
        return;
    }
    if (ptr != 0) {
        delete reinterpret_cast<EtsApplicationContextUtils *>(ptr);
    }
}

ani_int EtsApplicationContextUtils::OnNativeOnEnvironmentSync(ani_env *env, ani_object aniObj,
    ani_object envCallback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "env is nullptr");
        return ERROR_ID;
    }
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "nativeContext is null");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return ERROR_ID;
    }
    if (etsEnviromentCallback_ != nullptr) {
        return etsEnviromentCallback_->Register(envCallback);
    }
    ani_vm *aniVM = nullptr;
    if (env->GetVM(&aniVM) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "get aniVM failed");
        return ERROR_ID;
    }
    etsEnviromentCallback_ = std::make_shared<EtsEnviromentCallback>(aniVM);
    int32_t callbackId = etsEnviromentCallback_->Register(envCallback);
    applicationContext->RegisterEnvironmentCallback(etsEnviromentCallback_);

    return callbackId;
}

ani_object EtsApplicationContextUtils::OnGetUIAbilityByInstanceId(ani_env *env, ani_string instanceId)
{
    TAG_LOGD(AAFwkTag::APPKIT, "OnGetUIAbilityByInstanceId called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return nullptr;
    }
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationContext");
        EtsErrorUtil::ThrowRuntimeError(env, ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST);
        return nullptr;
    }

    std::string instanceIdStr;
    if (!AppExecFwk::GetStdString(env, instanceId, instanceIdStr)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse instanceId failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param instanceId failed, instanceId must be string.");
        return nullptr;
    }

    auto nativeAbility = applicationContext->GetNativeAbility(instanceIdStr);
    if (nativeAbility == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "NativeAbility not found for instanceId: %{public}s", instanceIdStr.c_str());
        EtsErrorUtil::ThrowRuntimeError(env, ERR_ABILITY_RUNTIME_EXTERNAL_NO_SUCH_ID);
        return nullptr;
    }

    if (nativeAbility->etsAbilityObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "etsAbilityObj is null for instanceId: %{public}s", instanceIdStr.c_str());
        EtsErrorUtil::ThrowRuntimeError(env, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR);
        return nullptr;
    }

    TAG_LOGD(AAFwkTag::APPKIT, "Get UIAbility for instanceId: %{public}s", instanceIdStr.c_str());
    return nativeAbility->etsAbilityObj;
}

void EtsApplicationContextUtils::OnNativeOffEnvironmentSync(ani_env *env, ani_object aniObj,
    ani_int callbackId, ani_object callback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "env is nullptr");
        return;
    }
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "nativeContext is null");
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM), nullptr);
        return;
    }

    if (etsEnviromentCallback_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "etsEnviromentCallback is null");
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env,
            (ani_int)AbilityErrorCode::ERROR_CODE_INVALID_PARAM, "env_callback is nullptr"), nullptr);
        return;
    }

    if (!etsEnviromentCallback_->UnRegister(callbackId)) {
        TAG_LOGE(AAFwkTag::APPKIT, "call UnRegister failed");
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env,
            (ani_int)AbilityErrorCode::ERROR_CODE_INVALID_PARAM, "call UnRegister failed!"), nullptr);
        return;
    }

    AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), nullptr);
}

void EtsApplicationContextUtils::OnNativeOnApplicationStateChangeSync(ani_env *env, ani_object aniObj,
    ani_object callback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "nativeContext is null");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return;
    }
    if (applicationStateCallback_ != nullptr) {
        applicationStateCallback_->Register(callback);
        return;
    }
    ani_vm *aniVM = nullptr;
    if (env->GetVM(&aniVM) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "get aniVM failed");
        return;
    }
    applicationStateCallback_ = std::make_shared<EtsApplicationStateChangeCallback>(aniVM);
    applicationStateCallback_->Register(callback);
    applicationContext->RegisterApplicationStateChangeCallback(applicationStateCallback_);
}

void EtsApplicationContextUtils::OnNativeSystemConfigurationUpdatedSync(
    ani_env *env, ani_object aniObj, ani_object callback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    ani_boolean isUndefined = true;
    env->Reference_IsUndefined(callback, &isUndefined);
    if (isUndefined) {
        TAG_LOGE(AAFwkTag::APPKIT, "callback undefine");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return;
    }
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "nativeContext is null");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return;
    }
    if (systemConfigurationUpdatedCallback_ != nullptr) {
        systemConfigurationUpdatedCallback_->Register(callback);
        return;
    }
    ani_vm *aniVM = nullptr;
    if (env->GetVM(&aniVM) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "get aniVM failed");
        return;
    }
    systemConfigurationUpdatedCallback_ = std::make_shared<EtsSystemConfigurationUpdatedCallback>(aniVM);
    systemConfigurationUpdatedCallback_->Register(callback);
    applicationContext->RegisterSystemConfigurationUpdatedCallback(systemConfigurationUpdatedCallback_);
}

void EtsApplicationContextUtils::OffNativeSystemConfigurationUpdatedSync(
    ani_env *env, ani_object aniObj, ani_object callback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    if (systemConfigurationUpdatedCallback_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationStateCallback_");
        EtsErrorUtil::ThrowInvalidParamError(
            env, "invalid paramter.");
        return;
    }
    ani_boolean isUndefined = true;
    env->Reference_IsUndefined(callback, &isUndefined);
    if (isUndefined) {
        systemConfigurationUpdatedCallback_->UnRegister();
    } else if (!systemConfigurationUpdatedCallback_->UnRegister(callback)) {
        TAG_LOGE(AAFwkTag::APPKIT, "call UnRegister failed");
        EtsErrorUtil::ThrowInvalidParamError(
            env, "Parse param call UnRegister failed, invalid paramter.");
        return;
    }
    if (systemConfigurationUpdatedCallback_->IsEmpty()) {
        systemConfigurationUpdatedCallback_.reset();
    }
}

void EtsApplicationContextUtils::OnNativeOffApplicationStateChangeSync(ani_env *env, ani_object aniObj,
    ani_object callback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    if (applicationStateCallback_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationStateCallback_");
        EtsErrorUtil::ThrowInvalidParamError(
            env, "Parse applicationStateCallback failed, applicationStateCallback must be function.");
        return;
    }
    ani_boolean isUndefined = true;
    env->Reference_IsUndefined(callback, &isUndefined);
    if (isUndefined) {
        applicationStateCallback_->UnRegister();
    } else if (!applicationStateCallback_->UnRegister(callback)) {
        TAG_LOGE(AAFwkTag::APPKIT, "call UnRegister failed");
        EtsErrorUtil::ThrowInvalidParamError(env,
            "Parse param call UnRegister failed, call UnRegister must be function.");
        return;
    }
    if (applicationStateCallback_->IsEmpty()) {
        applicationStateCallback_.reset();
    }
}

ani_int EtsApplicationContextUtils::OnGetCurrentAppCloneIndex(ani_env *env, ani_object aniObj)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return ANI_ERROR;
    }
    auto context = applicationContext_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return ANI_ERROR;
    }
    if (context->GetCurrentAppMode() != static_cast<int32_t>(AppExecFwk::MultiAppModeType::APP_CLONE)) {
        TAG_LOGE(AAFwkTag::APPKIT, "not clone");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_NOT_APP_CLONE);
        return ANI_ERROR;
    }
    int32_t appIndex = context->GetCurrentAppCloneIndex();
    return appIndex;
}

ani_string EtsApplicationContextUtils::OnGetCurrentInstanceKey(ani_env *env, ani_object aniObj)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return nullptr;
    }
    auto context = applicationContext_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return nullptr;
    }
    if (context->GetCurrentAppMode() != static_cast<int32_t>(AppExecFwk::MultiAppModeType::MULTI_INSTANCE)) {
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_MULTI_INSTANCE_NOT_SUPPORTED);
        TAG_LOGE(AAFwkTag::APPKIT, "not support");
        return nullptr;
    }
    std::string instanceKey = context->GetCurrentInstanceKey();
    return AppExecFwk::GetAniString(env, instanceKey);
}

void EtsApplicationContextUtils::OnGetAllRunningInstanceKeys(ani_env *env, ani_object aniObj, ani_object callback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    ani_object emptyArray = AppExecFwk::CreateEmptyArray(env);
    std::vector<std::string> instanceKeys;
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env,
            AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT), emptyArray);
        return;
    }
    if (applicationContext->GetCurrentAppMode() != static_cast<int32_t>(AppExecFwk::MultiAppModeType::MULTI_INSTANCE)) {
        TAG_LOGE(AAFwkTag::APPKIT, "not supported");
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateErrorByNativeErr(env,
            AAFwk::ERR_MULTI_INSTANCE_NOT_SUPPORTED), emptyArray);
        return;
    }
    ErrCode innerErrCode = applicationContext->GetAllRunningInstanceKeys(instanceKeys);
    if (innerErrCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "innerErrCode=%{public}d", innerErrCode);
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateErrorByNativeErr(env, (int32_t)innerErrCode), emptyArray);
        return;
    }
    ani_object stringArray;
    AppExecFwk::WrapArrayString(env, stringArray, instanceKeys);
    AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), stringArray);
}

void EtsApplicationContextUtils::OnGetAllWindowStages(ani_env *env, ani_object aniObj, ani_object callback)
{
    ani_object emptyArray = AppExecFwk::CreateEmptyArray(env);
#ifdef SUPPORT_SCREEN
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    std::vector<std::shared_ptr<UIAbility>> uiAbility;
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationContext");
        AppExecFwk::AsyncCallback(
            env, callback, EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), emptyArray);
        return;
    }
    applicationContext->GetAllUIAbilities(uiAbility);
    ani_object windowStages = CreateWindowStageArray(env, uiAbility);
    if (windowStages == nullptr) {
        AppExecFwk::AsyncCallback(
            env, callback, EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), emptyArray);
        return;
    }
    AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), windowStages);
#else
    AppExecFwk::AsyncCallback(
        env, callback, EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), emptyArray);
#endif
}

void EtsApplicationContextUtils::GetAllWindowStages(ani_env *env, ani_object aniObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "GetAllWindowStages Call");
    auto etsContext = GeApplicationContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null etsContext");
        return;
    }
    etsContext->OnGetAllWindowStages(env, aniObj, callback);
}

#ifdef SUPPORT_SCREEN
ani_object EtsApplicationContextUtils::CreateWindowStageArray(
    ani_env *env, std::vector<std::shared_ptr<UIAbility>> uiAbility)
{
    ani_class arrayCls = nullptr;
    ani_status status = ANI_OK;

    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return nullptr;
    }

    status = env->FindClass(CLASSNAME_ARRAY, &arrayCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "FindClass failed status: %{public}d", status);
        return nullptr;
    }

    ani_method arrayCtor;
    status = env->Class_FindMethod(arrayCls, "<ctor>", "i:", &arrayCtor);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "find ctor failed status: %{public}d", status);
        return nullptr;
    }

    std::vector<ani_object> windowStageArray;
    for (auto &item : uiAbility) {
        if (item == nullptr) {
            continue;
        }
        auto windowStage = item->GetEtsWindowStage();
        if (windowStage == nullptr) {
            continue;
        }
        windowStageArray.emplace_back(windowStage);
    }
    ani_object arrayObj;
    status = env->Object_New(arrayCls, arrayCtor, &arrayObj, windowStageArray.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Object_New array status: %{public}d", status);
        return arrayObj;
    }
    ani_size index = 0;
    for (auto &windowStage : windowStageArray) {
        status = env->Object_CallMethodByName_Void(arrayObj, "$_set", "iY:", index, windowStage);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::APPKIT, "Object_CallMethodByName_Void failed status: %{public}d", status);
            continue;
        }
        index++;
    }
    return arrayObj;
}
#endif

void EtsApplicationContextUtils::NativeOnInteropLifecycleCallbackSync(ani_env *env,
    ani_object aniObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "NativeOnInteropLifecycleCallbackSync Call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "env is nullptr");
        return;
    }
    auto etsContext = GeApplicationContext(env, aniObj);
    if (etsContext == nullptr) {
        EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return;
    }
    etsContext->RegisterInteropAbilityLifecycleCallback(env, callback);
}

void EtsApplicationContextUtils::RegisterInteropAbilityLifecycleCallback(ani_env *env, ani_object callback)
{
    TAG_LOGI(AAFwkTag::APPKIT, "call RegisterInteropAbilityLifecycleCallback");
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "applicationContext is null");
        EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return;
    }
    std::lock_guard<std::mutex> lock(g_interopAbilityLifecycleCallbackLock);
    if (interopAbilityLifecycleCallback_ != nullptr) {
        auto result = interopAbilityLifecycleCallback_->Register((void *)callback);
        if (result < 0) {
            TAG_LOGE(AAFwkTag::APPKIT, "register failed: %{public}d", result);
            EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        }
        return;
    }

    interopAbilityLifecycleCallback_ = std::make_shared<EtsInteropAbilityLifecycleCallback>(env);
    auto result = interopAbilityLifecycleCallback_->Register((void *)callback);
    if (result < 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "register failed: %{public}d", result);
        EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return;
    }
    applicationContext->RegisterInteropAbilityLifecycleCallback(interopAbilityLifecycleCallback_);
}

void EtsApplicationContextUtils::NativeOffInteropLifecycleCallbackSync(ani_env *env,
    ani_object aniObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "NativeOffInteropLifecycleCallbackSync Call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "env is nullptr");
        return;
    }
    auto etsContext = GeApplicationContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "applicationContext is null");
        EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return;
    }
    etsContext->UnregisterInteropAbilityLifecycleCallback(env, callback);
}

void EtsApplicationContextUtils::UnregisterInteropAbilityLifecycleCallback(ani_env *env, ani_object callback)
{
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "applicationContext is null");
        EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return;
    }

    std::lock_guard<std::mutex> lock(g_interopAbilityLifecycleCallbackLock);
    if (interopAbilityLifecycleCallback_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "interopAbilityLifecycleCallback_ is null");
        EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return;
    }

    ani_boolean isUndefined = true;
    env->Reference_IsUndefined(callback, &isUndefined);
    if (isUndefined) {
        interopAbilityLifecycleCallback_->Unregister();
    } else if (!interopAbilityLifecycleCallback_->Unregister((void *)callback)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Unregister failed");
        EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return;
    }

    applicationContext->UnregisterInteropAbilityLifecycleCallback(interopAbilityLifecycleCallback_);
    if (interopAbilityLifecycleCallback_->Empty()) {
        interopAbilityLifecycleCallback_.reset();
    }
}

void EtsApplicationContextUtils::OnRestartApp(ani_env *env, ani_object aniObj, ani_object wantObj)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationContext");
        return;
    }
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse want failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param want failed, want must be Want.");
        return;
    }
    auto errCode = applicationContext->RestartApp(want);
    switch (errCode) {
        case ERR_INVALID_VALUE:
            EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            break;
        case AAFwk::ERR_RESTART_APP_INCORRECT_ABILITY:
            EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_RESTART_APP_INCORRECT_ABILITY);
            break;
        case AAFwk::ERR_RESTART_APP_FREQUENT:
            EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_RESTART_APP_FREQUENT);
            break;
        case AAFwk::NOT_TOP_ABILITY:
            EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_TOP_ABILITY);
            break;
        default:
            EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
    }
    TAG_LOGD(AAFwkTag::APPKIT, "RestartApp errCode is %{public}d", errCode);
}

void EtsApplicationContextUtils::OnEnableDelayedProcessExit(ani_env *env, ani_object aniObj, ani_object callback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationContext");
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env,
            (ani_int)AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT,
            "applicationContext is already released."), nullptr);
        return;
    }
    auto errCode = applicationContext->EnableDelayedProcessExit();
    if (errCode == ERR_OK) {
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), nullptr);
    } else {
        TAG_LOGE(AAFwkTag::APPKIT, "EnableDelayedProcessExit failed %{public}d", errCode);
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateErrorByNativeErr(env, errCode), nullptr);
    }
}

void EtsApplicationContextUtils::OnDisableDelayedProcessExit(ani_env *env, ani_object aniObj, ani_object callback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationContext");
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env,
            (ani_int)AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT,
            "applicationContext is already released."), nullptr);
        return;
    }
    auto errCode = applicationContext->DisableDelayedProcessExit();
    if (errCode == ERR_OK) {
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), nullptr);
    } else {
        TAG_LOGE(AAFwkTag::APPKIT, "DisableDelayedProcessExit failed %{public}d", errCode);
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateErrorByNativeErr(env, errCode), nullptr);
    }
}

void EtsApplicationContextUtils::OnStartSelfUIAbility(ani_env *env,
    ani_object aniObj, ani_object wantObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "StartSelfUIAbility Call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    AAFwk::Want want;
    if (!OHOS::AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse want failed");
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateInvalidParamError(env,
            "Parse param want failed, want must be Want."), nullptr);
        return;
    }
    auto context = applicationContext_.lock();
    if (!context) {
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateErrorByNativeErr(env,
            (int32_t)AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT), nullptr);
        return;
    }
    auto errCode = context->StartSelfUIAbility(want);
    if (errCode == ERR_OK) {
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), nullptr);
    } else {
        TAG_LOGE(AAFwkTag::APPKIT, "StartSelfUIAbility failed %{public}d", errCode);
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateErrorByNativeErr(env, errCode), nullptr);
    }
}

void EtsApplicationContextUtils::OnSetFont(ani_env *env, ani_object aniObj, ani_string font)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationContext");
        return;
    }
    std::string stdFont = "";
    if (!AppExecFwk::GetStdString(env, font, stdFont) || stdFont.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse font failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param font failed, font must be string.");
        return;
    }
    TAG_LOGD(AAFwkTag::APPKIT, "SetFont font %{public}s", stdFont.c_str());
    applicationContext->SetFont(stdFont);
}

void EtsApplicationContextUtils::OnSetColorMode(ani_env *env, ani_object aniObj, ani_enum_item colorMode)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationContext");
        return;
    }
    ani_int mode = 0;
    if (!AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative(env, colorMode, mode)) {
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param colorMode failed, colorMode must be number.");
        TAG_LOGE(AAFwkTag::APPKIT, "Parse colorMode failed");
        return;
    }
    TAG_LOGD(AAFwkTag::APPKIT, "colorMode is %{public}d", mode);
    applicationContext->SetColorMode(static_cast<int32_t>(mode));
}

void EtsApplicationContextUtils::OnSetLanguage(ani_env *env, ani_object aniObj, ani_string language)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationContext");
        return;
    }
    std::string stdLanguage = "";
    if (!AppExecFwk::GetStdString(env, language, stdLanguage) || stdLanguage.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse language failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param language failed, language must be string.");
        return;
    }
    applicationContext->SetLanguage(stdLanguage);
    TAG_LOGD(AAFwkTag::APPKIT, "stdLanguage language %{public}s", stdLanguage.c_str());
}

void EtsApplicationContextUtils::OnSetFontSizeScale(ani_env *env, ani_object aniObj, ani_double fontSizeScale)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationContext");
        return;
    }

    double stdFontSizeScale = static_cast<double>(fontSizeScale);
    TAG_LOGD(AAFwkTag::APPKIT, "fontSizeScale: %{public}f", stdFontSizeScale);
    if (fontSizeScale < FOUNT_SIZE) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid size");
        EtsErrorUtil::ThrowInvalidParamError(env, "Invalid font size.");
        return;
    }
    applicationContext->SetFontSizeScale(stdFontSizeScale);
}

void EtsApplicationContextUtils::OnClearUpApplicationData(ani_env *env, ani_object aniObj, ani_object callback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "nativeContext is null");
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env,
            (ani_int)AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT,
            "applicationContext if already released."), nullptr);
        return;
    }
    AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), nullptr);
    applicationContext->ClearUpApplicationData();
}

void EtsApplicationContextUtils::OnGetRunningProcessInformation(ani_env *env, ani_object aniObj, ani_object callback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    ani_object emptyArray = AppExecFwk::CreateEmptyArray(env);
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "nativeContext is null");
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env,
            (ani_int)AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT,
            "applicationContext if already released."), emptyArray);
        return;
    }
    std::vector<AppExecFwk::RunningProcessInfo> infos;
    AppExecFwk::RunningProcessInfo processInfo;
    ErrCode innerErrCode = applicationContext->GetProcessRunningInformation(processInfo);
    if (innerErrCode == ERR_OK) {
        infos.emplace_back(processInfo);
        ani_object aniInfosRef = AppExecFwk::CreateRunningProcessInfoArray(env, infos);
        if (aniInfosRef == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null array");
            AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env,
                (ani_int)AbilityErrorCode::ERROR_CODE_INNER, "Initiate array failed."), emptyArray);
        } else {
            AppExecFwk::AsyncCallback(env, callback,
                EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), aniInfosRef);
        }
    } else {
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env,
            (ani_int)AbilityErrorCode::ERROR_CODE_INNER, "Get process infos failed."), emptyArray);
    }
}

void EtsApplicationContextUtils::OnkillAllProcesses(ani_env *env, ani_object aniObj,
    ani_boolean clearPageStack, ani_object callback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "env is nullptr");
        return;
    }
    ani_object aniObject = EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK);
    auto context = applicationContext_.lock();
    if (!context) {
        TAG_LOGE(AAFwkTag::APPKIT, "nativeContextLong is nullptr");
        aniObject = EtsErrorUtil::CreateError(env, (ani_int)AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT,
            "applicationContext is already released.");
        AppExecFwk::AsyncCallback(env, callback, aniObject, nullptr);
        return;
    }
    AppExecFwk::AsyncCallback(env, callback, aniObject, nullptr);
    context->KillProcessBySelf(clearPageStack);
}

void EtsApplicationContextUtils::OnPreloadUIExtensionAbility(ani_env *env,
    ani_object aniObj, ani_object wantObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "PreloadUIExtensionAbility Call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    AAFwk::Want want;
    if (!OHOS::AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse want failed");
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateInvalidParamError(env,
            "Parse param want failed, want must be Want."), nullptr);
        return;
    }
    auto context = applicationContext_.lock();
    if (!context) {
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateErrorByNativeErr(env,
            (int32_t)AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT), nullptr);
        return;
    }
    auto hostBundleName = context->GetBundleName();
    TAG_LOGD(AAFwkTag::APPKIT, "HostBundleName is %{public}s", hostBundleName.c_str());
    auto innerErrCode = AAFwk::AbilityManagerClient::GetInstance()->PreloadUIExtensionAbility(want, hostBundleName);
    if (innerErrCode == ERR_OK) {
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env,
            AbilityErrorCode::ERROR_OK), nullptr);
    } else {
        TAG_LOGE(AAFwkTag::APPKIT, "OnPreloadUIExtensionAbility failed %{public}d", innerErrCode);
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateErrorByNativeErr(env, innerErrCode), nullptr);
    }
}

void EtsApplicationContextUtils::OnSetSupportedProcessCacheSync(ani_env *env, ani_object aniObj, ani_boolean value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationContext");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return;
    }
    int32_t errCode = applicationContext->SetSupportedProcessCacheSelf(value);
    if (errCode == AAFwk::ERR_CAPABILITY_NOT_SUPPORT) {
        EtsErrorUtil::ThrowError(env,
            AbilityErrorCode::ERROR_CODE_CAPABILITY_NOT_SUPPORT);
    } else if (errCode != ERR_OK) {
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
    }
}

void EtsApplicationContextUtils::RestartApp(ani_env *env, ani_object aniObj, ani_object wantObj)
{
    TAG_LOGD(AAFwkTag::APPKIT, "RestartApp Call");
    auto etsContext = GeApplicationContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null etsContext");
        return;
    }
    etsContext->OnRestartApp(env, aniObj, wantObj);
}

void EtsApplicationContextUtils::EnableDelayedProcessExit(ani_env *env, ani_object aniObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "EnableDelayedProcessExit Call");
    auto etsContext = GeApplicationContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null etsContext");
        return;
    }
    etsContext->OnEnableDelayedProcessExit(env, aniObj, callback);
}

void EtsApplicationContextUtils::DisableDelayedProcessExit(ani_env *env, ani_object aniObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "DisableDelayedProcessExit Call");
    auto etsContext = GeApplicationContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null etsContext");
        return;
    }
    etsContext->OnDisableDelayedProcessExit(env, aniObj, callback);
}

void EtsApplicationContextUtils::StartSelfUIAbility(ani_env *env, ani_object aniObj,
    ani_object wantObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "StartSelfUIAbility Call");
    auto etsContext = GeApplicationContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null etsContext");
        return;
    }
    etsContext->OnStartSelfUIAbility(env, aniObj, wantObj, callback);
}

void EtsApplicationContextUtils::SetFont(ani_env *env, ani_object aniObj, ani_string font)
{
    TAG_LOGD(AAFwkTag::APPKIT, "SetFont Call");
    auto etsContext = GeApplicationContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null etsContext");
        return;
    }
    etsContext->OnSetFont(env, aniObj, font);
}

void EtsApplicationContextUtils::SetColorMode(ani_env *env, ani_object aniObj, ani_enum_item colorMode)
{
    TAG_LOGD(AAFwkTag::APPKIT, "SetColorMode Call");
    auto etsContext = GeApplicationContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null etsContext");
        return;
    }
    etsContext->OnSetColorMode(env, aniObj, colorMode);
}

void EtsApplicationContextUtils::SetLanguage(ani_env *env, ani_object aniObj, ani_string language)
{
    TAG_LOGD(AAFwkTag::APPKIT, "SetLanguage Call");
    auto etsContext = GeApplicationContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null etsContext");
        return;
    }
    etsContext->OnSetLanguage(env, aniObj, language);
}

void EtsApplicationContextUtils::SetFontSizeScale(ani_env *env, ani_object aniObj, ani_double fontSizeScale)
{
    TAG_LOGD(AAFwkTag::APPKIT, "SetFontSizeScale Call");
    auto etsContext = GeApplicationContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null etsContext");
        return;
    }
    etsContext->OnSetFontSizeScale(env, aniObj, fontSizeScale);
}

void EtsApplicationContextUtils::ClearUpApplicationData(ani_env *env, ani_object aniObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "ClearUpApplicationData Call");
    auto etsContext = GeApplicationContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null etsContext");
        return;
    }
    etsContext->OnClearUpApplicationData(env, aniObj, callback);
}

void EtsApplicationContextUtils::GetRunningProcessInformation(ani_env *env, ani_object aniObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "GetRunningProcessInformation Call");
    auto etsContext = GeApplicationContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null etsContext");
        return;
    }
    etsContext->OnGetRunningProcessInformation(env, aniObj, callback);
}

ani_int EtsApplicationContextUtils::NativeOnLifecycleCallbackSync(ani_env *env,
    ani_object aniObj, ani_string type, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "NativeOnLifecycleCallbackSync Call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "env is nullptr");
        return ani_int(ERROR_CODE_NULL_ENV);
    }
    auto etsContext = GeApplicationContext(env, aniObj);
    if (etsContext == nullptr) {
        EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return ani_int(ERROR_CODE_NULL_CONTEXT);
    }
    std::string stdType;
    if (!AppExecFwk::GetStdString(env, type, stdType)) {
        TAG_LOGE(AAFwkTag::APPKIT, "parse type failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Failed to parse param type. Type must be a string.");
        return ani_int(ERROR_CODE_INVALID_PARAM);
    }
    TAG_LOGD(AAFwkTag::APPKIT, "type=%{public}s", stdType.c_str());
    if (stdType == TYPE_ABILITY_LIFECYCLE) {
        return etsContext->RegisterAbilityLifecycleCallback(env, callback);
    }
    EtsErrorUtil::ThrowInvalidParamError(env, "Unknown type.");
    return ani_int(ERROR_CODE_INVALID_PARAM);
}

ani_int EtsApplicationContextUtils::RegisterAbilityLifecycleCallback(ani_env *env, ani_object callback)
{
    TAG_LOGI(AAFwkTag::APPKIT, "call RegisterAbilityLifecycleCallback");
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "applicationContext is null");
        EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return ani_int(ERROR_CODE_NULL_CONTEXT);
    }
    std::lock_guard<std::mutex> lock(abilityLifecycleCallbackLock_);
    if (abilityLifecycleCallback_ != nullptr) {
        auto result = abilityLifecycleCallback_->Register(callback);
        if (result < 0) {
            TAG_LOGE(AAFwkTag::APPKIT, "register failed: %{public}d", result);
            EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return ani_int(ERROR_CODE_INVALID_PARAM);
        }
        return ani_int(result);
    }

    abilityLifecycleCallback_ = std::make_shared<EtsAbilityLifecycleCallback>(env);
    int32_t callbackId = abilityLifecycleCallback_->Register(callback);
    if (callbackId >= 0) {
        applicationContext->RegisterAbilityLifecycleCallback(abilityLifecycleCallback_);
        return ani_int(callbackId);
    }
    if (callbackId == static_cast<int32_t>(ERROR_CODE_NULL_ENV) ||
        callbackId == static_cast<int32_t>(ERROR_CODE_NULL_CALLBACK)) {
        EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return ani_int(ERROR_CODE_INVALID_PARAM);
    }

    EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
    return ani_int(ERROR_CODE_INVALID_PARAM);
}

void EtsApplicationContextUtils::NativeOffAbilityLifecycleCheck(ani_env *env, ani_object aniObj)
{
    TAG_LOGD(AAFwkTag::APPKIT, "NativeOffAbilityLifecycleCheck Call");
    if (env == nullptr || aniObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env or aniObj");
        return;
    }
    auto etsContext = GeApplicationContext(env, aniObj);
    if (etsContext == nullptr) {
        EtsErrorUtil::ThrowRuntimeError(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return;
    }
    auto applicationContext = etsContext->applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "nativeContext is null");
        EtsErrorUtil::ThrowRuntimeError(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }
}

void EtsApplicationContextUtils::NativeOffLifecycleCallbackSync(ani_env *env,
    ani_object aniObj, ani_string type, ani_int callbackId, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "NativeOffLifecycleCallbackSync Call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "env is nullptr");
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env,
            (ani_int)AbilityErrorCode::ERROR_CODE_INVALID_PARAM, "env is nullptr"), nullptr);
        return;
    }
    auto etsContext = GeApplicationContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "etsContext is null");
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env,
            (ani_int)AbilityErrorCode::ERROR_CODE_INVALID_PARAM, "etsContext is null"), nullptr);
        return;
    }
    std::string stdType;
    if (!AppExecFwk::GetStdString(env, type, stdType)) {
        TAG_LOGE(AAFwkTag::APPKIT, "parse type failed");
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env,
            (ani_int)AbilityErrorCode::ERROR_CODE_INVALID_PARAM, "Failed to parse param type. Type must be a string."),
            nullptr);
        return;
    }
    TAG_LOGD(AAFwkTag::APPKIT, "type=%{public}s", stdType.c_str());
    if (stdType == TYPE_ABILITY_LIFECYCLE) {
        etsContext->UnregisterAbilityLifecycleCallback(env, callbackId, callback);
        return;
    }
    AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env,
        (ani_int)AbilityErrorCode::ERROR_CODE_INVALID_PARAM, "Unknown type."), nullptr);
}

void EtsApplicationContextUtils::UnregisterAbilityLifecycleCallback(ani_env *env, int32_t callbackId,
    ani_object callback)
{
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "applicationContext is null");
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env,
            (ani_int)AbilityErrorCode::ERROR_CODE_INVALID_PARAM, "applicationContext is null"), nullptr);
        return;
    }
    std::lock_guard<std::mutex> lock(abilityLifecycleCallbackLock_);
    if (abilityLifecycleCallback_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "abilityLifecycleCallback_ is null");
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env,
            (ani_int)AbilityErrorCode::ERROR_CODE_INVALID_PARAM, "callback_ is null"), nullptr);
        return;
    }

    if (abilityLifecycleCallback_->Unregister(callbackId)) {
        applicationContext->UnregisterAbilityLifecycleCallback(abilityLifecycleCallback_);
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), nullptr);
        return;
    }
    TAG_LOGE(AAFwkTag::APPKIT, "failed to unregister");
    AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env,
        (ani_int)AbilityErrorCode::ERROR_CODE_INVALID_PARAM, "failed to unregister"), nullptr);
}

void EtsApplicationContextUtils::killAllProcesses(ani_env *env, ani_object aniObj,
    ani_boolean clearPageStack, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "killAllProcesses Call");
    auto etsContext = GeApplicationContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null etsContext");
        return;
    }
    etsContext->OnkillAllProcesses(env, aniObj, clearPageStack, callback);
}

void EtsApplicationContextUtils::PreloadUIExtensionAbility(ani_env *env,
    ani_object aniObj, ani_object wantObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "PreloadUIExtensionAbility Call");
    auto etsContext = GeApplicationContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null etsContext");
        return;
    }
    etsContext->OnPreloadUIExtensionAbility(env, aniObj, wantObj, callback);
}

void EtsApplicationContextUtils::SetSupportedProcessCacheSync(ani_env *env, ani_object aniObj, ani_boolean value)
{
    TAG_LOGD(AAFwkTag::APPKIT, "SetSupportedProcessCacheSync Call");
    auto etsContext = GeApplicationContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null etsContext");
        return;
    }
    etsContext->OnSetSupportedProcessCacheSync(env, aniObj, value);
}

ani_int EtsApplicationContextUtils::NativeOnEnvironmentSync(ani_env *env, ani_object aniObj, ani_object envCallback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "NativeOnEnvironmentSync Call");
    auto etsContext = GeApplicationContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null etsContext");
        return ANI_ERROR;
    }
    return etsContext->OnNativeOnEnvironmentSync(env, aniObj, envCallback);
}

ani_object EtsApplicationContextUtils::GetUIAbilityByInstanceId(
    ani_env *env, ani_object aniObj, ani_string instanceId)
{
    TAG_LOGD(AAFwkTag::APPKIT, "GetUIAbilityByInstanceId called");
    auto etsContext = GeApplicationContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null etsContext");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return nullptr;
    }
    return etsContext->OnGetUIAbilityByInstanceId(env, instanceId);
}

void EtsApplicationContextUtils::NativeOffEnvironmentCheck(ani_env *env, ani_object aniObj)
{
    TAG_LOGD(AAFwkTag::APPKIT, "NativeOffEnvironmentCheck Call");
    if (env == nullptr || aniObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env or aniObj");
        return;
    }
    auto etsContext = GeApplicationContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null etsContext");
        EtsErrorUtil::ThrowRuntimeError(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return;
    }
    auto applicationContext = etsContext->applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "nativeContext is null");
        EtsErrorUtil::ThrowRuntimeError(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }
}

void EtsApplicationContextUtils::NativeOffEnvironmentSync(ani_env *env, ani_object aniObj,
    ani_int callbackId, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "NativeOffEnvironmentSync Call");
    auto etsContext = GeApplicationContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null etsContext");
        return;
    }
    etsContext->OnNativeOffEnvironmentSync(env, aniObj, callbackId, callback);
}

void EtsApplicationContextUtils::NativeOnApplicationStateChangeSync(ani_env *env, ani_object aniObj,
    ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "NativeOnApplicationStateChangeSync Call");
    auto etsContext = GeApplicationContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null etsContext");
        return;
    }
    etsContext->OnNativeOnApplicationStateChangeSync(env, aniObj, callback);
}

void EtsApplicationContextUtils::NativeOnSystemConfigurationUpdatedSync(
    ani_env *env, ani_object aniObj, ani_object callback)
{
    TAG_LOGI(AAFwkTag::APPKIT, "wlh NativeOnSystemConfigurationUpdatedSync Call");
    auto etsContext = GeApplicationContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null etsContext");
        return;
    }
    etsContext->OnNativeSystemConfigurationUpdatedSync(env, aniObj, callback);
}

void EtsApplicationContextUtils::NativeOffSystemConfigurationUpdatedSync(
    ani_env *env, ani_object aniObj, ani_object callback)
{
    TAG_LOGI(AAFwkTag::APPKIT, "wlh NativeOffSystemConfigurationUpdatedSync Call");
    auto etsContext = GeApplicationContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null etsContext");
        return;
    }
    etsContext->OffNativeSystemConfigurationUpdatedSync(env, aniObj, callback);
}

void EtsApplicationContextUtils::NativeOffApplicationStateChangeSync(ani_env *env, ani_object aniObj,
    ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "NativeOffApplicationStateChangeSync Call");
    auto etsContext = GeApplicationContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null etsContext");
        return;
    }
    etsContext->OnNativeOffApplicationStateChangeSync(env, aniObj, callback);
}

void EtsApplicationContextUtils::GetAllRunningInstanceKeys(ani_env *env, ani_object aniObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "GetAllRunningInstanceKeys Call");
    auto etsContext = GeApplicationContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null etsContext");
        return;
    }
    etsContext->OnGetAllRunningInstanceKeys(env, aniObj, callback);
}

ani_string EtsApplicationContextUtils::GetCurrentInstanceKey(ani_env *env, ani_object aniObj)
{
    TAG_LOGD(AAFwkTag::APPKIT, "GetCurrentInstanceKey Call");
    auto etsContext = GeApplicationContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null etsContext");
        return nullptr;
    }
    return etsContext->OnGetCurrentInstanceKey(env, aniObj);
}

ani_int EtsApplicationContextUtils::GetCurrentAppCloneIndex(ani_env *env, ani_object aniObj)
{
    TAG_LOGD(AAFwkTag::APPKIT, "GetCurrentAppCloneIndex Call");
    auto etsContext = GeApplicationContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null etsContext");
        return ANI_ERROR;
    }
    return etsContext->OnGetCurrentAppCloneIndex(env, aniObj);
}

EtsApplicationContextUtils* EtsApplicationContextUtils::GeApplicationContext(ani_env *env, ani_object aniObj)
{
    if (env == nullptr || aniObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env or aniObj");
        return nullptr;
    }
    ani_long etsApplicationContextPtr = 0;
    ani_status status = env->Object_GetFieldByName_Long(aniObj, "etsApplicationContextPtr", &etsApplicationContextPtr);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "etsApplicationContextPtr GetField status: %{public}d", status);
        return nullptr;
    }
    auto etsContext = reinterpret_cast<EtsApplicationContextUtils *>(etsApplicationContextPtr);
    return etsContext;
}

ani_object EtsApplicationContextUtils::SetApplicationContext(ani_env *aniEnv,
    const std::shared_ptr<ApplicationContext> &applicationContext)
{
    TAG_LOGD(AAFwkTag::APPKIT, "SetApplicationContext Call");
    if (aniEnv == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null aniEnv");
        return nullptr;
    }
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        return nullptr;
    }
    ani_status status = ANI_ERROR;
    ani_class cls {};
    if ((status = aniEnv->FindClass(ETS_APPLICATION_CONTEXT_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "status: %{public}d", status);
        return nullptr;
    }
    ani_method method {};
    if ((status = aniEnv->Class_FindMethod(cls, "<ctor>", "l:", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "status: %{public}d", status);
        return nullptr;
    }
    auto etsContext = new (std::nothrow) EtsApplicationContextUtils(applicationContext);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "workContext nullptr");
        return nullptr;
    }
    ani_object contextObj = nullptr;
    if ((status = aniEnv->Object_New(cls, method, &contextObj, reinterpret_cast<ani_long>(etsContext))) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "status: %{public}d", status);
        delete etsContext;
        etsContext = nullptr;
        return nullptr;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<ApplicationContext>(etsContext->applicationContext_);
    if (workContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null workContext");
        return nullptr;
    }
    ani_long nativeContextLong = reinterpret_cast<ani_long>(workContext);
    if (!ContextUtil::SetNativeContextLong(aniEnv, contextObj, nativeContextLong)) {
        TAG_LOGE(AAFwkTag::APPKIT, "SetNativeContextLong failed");
        delete workContext;
        workContext = nullptr;
        return nullptr;
    }
    return contextObj;
}

void EtsApplicationContextUtils::BindApplicationContextFunc(ani_env *aniEnv)
{
    if (aniEnv == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null aniEnv");
        return;
    }
    ani_class contextClass = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = aniEnv->FindClass(ETS_APPLICATION_CONTEXT_CLASS_NAME, &contextClass)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "FindClass ApplicationContext failed status: %{public}d", status);
        return;
    }
    std::call_once(g_bindNativeMethodsFlag, [&status, aniEnv, contextClass]() {
        std::array applicationContextFunctions = {
            ani_native_function {"setSupportedProcessCacheSync", "z:",
                reinterpret_cast<void *>(EtsApplicationContextUtils::SetSupportedProcessCacheSync)},
            ani_native_function {"nativekillAllProcessesSync", "zC{utils.AbilityUtils.AsyncCallbackWrapper}:",
                reinterpret_cast<void *>(EtsApplicationContextUtils::killAllProcesses)},
            ani_native_function {"nativepreloadUIExtensionAbilitySync",
                "C{@ohos.app.ability.Want.Want}C{utils.AbilityUtils.AsyncCallbackWrapper}:",
                reinterpret_cast<void *>(EtsApplicationContextUtils::PreloadUIExtensionAbility)},
            ani_native_function {"nativeOnLifecycleCallbackSync",
                "C{std.core.String}C{@ohos.app.ability.AbilityLifecycleCallback.AbilityLifecycleCallback}:i",
                reinterpret_cast<void *>(EtsApplicationContextUtils::NativeOnLifecycleCallbackSync)},
            ani_native_function {"nativeOffLifecycleCallbackSync",
                "C{std.core.String}iC{utils.AbilityUtils.AsyncCallbackWrapper}:",
                reinterpret_cast<void *>(EtsApplicationContextUtils::NativeOffLifecycleCallbackSync)},
            ani_native_function {"nativeOffAbilityLifecycleCheck", ":",
                reinterpret_cast<void *>(EtsApplicationContextUtils::NativeOffAbilityLifecycleCheck)},
            ani_native_function {"nativegetRunningProcessInformation",
                "C{utils.AbilityUtils.AsyncCallbackWrapper}:",
                reinterpret_cast<void *>(EtsApplicationContextUtils::GetRunningProcessInformation)},
            ani_native_function {"nativeclearUpApplicationData",
                "C{utils.AbilityUtils.AsyncCallbackWrapper}:",
                reinterpret_cast<void *>(EtsApplicationContextUtils::ClearUpApplicationData)},
            ani_native_function {"nativesetLanguage", "C{std.core.String}:",
                reinterpret_cast<void *>(EtsApplicationContextUtils::SetLanguage)},
            ani_native_function {"nativesetFontSizeScale", "d:",
                reinterpret_cast<void *>(EtsApplicationContextUtils::SetFontSizeScale)},
            ani_native_function {"nativesetColorMode",
                "C{@ohos.app.ability.ConfigurationConstant.ConfigurationConstant.ColorMode}:",
                reinterpret_cast<void *>(EtsApplicationContextUtils::SetColorMode)},
            ani_native_function {"nativesetFont", "C{std.core.String}:",
                reinterpret_cast<void *>(EtsApplicationContextUtils::SetFont)},
            ani_native_function {"nativerestartApp", "C{@ohos.app.ability.Want.Want}:",
                reinterpret_cast<void *>(EtsApplicationContextUtils::RestartApp)},
            ani_native_function {"nativeEnableDelayedProcessExitSync",
                "C{utils.AbilityUtils.AsyncCallbackWrapper}:",
                reinterpret_cast<void *>(EtsApplicationContextUtils::EnableDelayedProcessExit)},
            ani_native_function {"nativeDisableDelayedProcessExitSync",
                "C{utils.AbilityUtils.AsyncCallbackWrapper}:",
                reinterpret_cast<void *>(EtsApplicationContextUtils::DisableDelayedProcessExit)},
            ani_native_function {"nativeStartSelfUIAbilitySync",
                "C{@ohos.app.ability.Want.Want}C{utils.AbilityUtils.AsyncCallbackWrapper}:",
                reinterpret_cast<void *>(EtsApplicationContextUtils::StartSelfUIAbility)},
            ani_native_function {"nativeOnEnvironmentSync",
                "C{@ohos.app.ability.EnvironmentCallback.EnvironmentCallback}:i",
                reinterpret_cast<void *>(EtsApplicationContextUtils::NativeOnEnvironmentSync)},
            ani_native_function {"nativeOffEnvironmentSync", "iC{utils.AbilityUtils.AsyncCallbackWrapper}:",
                reinterpret_cast<void *>(EtsApplicationContextUtils::NativeOffEnvironmentSync)},
            ani_native_function {"nativeOffEnvironmentCheck", ":",
                reinterpret_cast<void *>(EtsApplicationContextUtils::NativeOffEnvironmentCheck)},
            ani_native_function {"nativeOnApplicationStateChangeSync",
                "C{@ohos.app.ability.ApplicationStateChangeCallback.ApplicationStateChangeCallback}:",
                reinterpret_cast<void*>(EtsApplicationContextUtils::NativeOnApplicationStateChangeSync)},
            ani_native_function {"nativeOffApplicationStateChangeSync",
                "C{@ohos.app.ability.ApplicationStateChangeCallback.ApplicationStateChangeCallback}:",
                reinterpret_cast<void*>(EtsApplicationContextUtils::NativeOffApplicationStateChangeSync)},
            ani_native_function {"nativeGetAllRunningInstanceKeys", "C{utils.AbilityUtils.AsyncCallbackWrapper}:",
                reinterpret_cast<void *>(EtsApplicationContextUtils::GetAllRunningInstanceKeys)},
            ani_native_function {"nativeGetAllWindowStages", "C{utils.AbilityUtils.AsyncCallbackWrapper}:",
                reinterpret_cast<void *>(EtsApplicationContextUtils::GetAllWindowStages)},
            ani_native_function{"nativegetCurrentInstanceKey", ":C{std.core.String}",
                reinterpret_cast<void *>(EtsApplicationContextUtils::GetCurrentInstanceKey)},
            ani_native_function {"nativegetCurrentAppCloneIndex", ":i",
                reinterpret_cast<void *>(EtsApplicationContextUtils::GetCurrentAppCloneIndex)},
            ani_native_function {"nativeOnInteropLifecycleCallbackSync",
                "C{@ohos.app.ability.InteropAbilityLifecycleCallback.InteropAbilityLifecycleCallback}:",
                reinterpret_cast<void *>(EtsApplicationContextUtils::NativeOnInteropLifecycleCallbackSync)},
            ani_native_function {"nativeOffInteropLifecycleCallbackSync",
                "C{@ohos.app.ability.InteropAbilityLifecycleCallback.InteropAbilityLifecycleCallback}:",
                reinterpret_cast<void *>(EtsApplicationContextUtils::NativeOffInteropLifecycleCallbackSync)},
            ani_native_function {"nativeOnSystemConfigurationUpdatedSync",
                "C{@ohos.app.ability.systemConfiguration.systemConfiguration.UpdatedCallback}:",
                reinterpret_cast<void*>(EtsApplicationContextUtils::NativeOnSystemConfigurationUpdatedSync)},
            ani_native_function {"nativeOffSystemConfigurationUpdatedSync",
                "C{@ohos.app.ability.systemConfiguration.systemConfiguration.UpdatedCallback}:",
                reinterpret_cast<void*>(EtsApplicationContextUtils::NativeOffSystemConfigurationUpdatedSync)},
            ani_native_function {"nativeGetUIAbilityByInstanceId",
                "C{std.core.String}:C{@ohos.app.ability.UIAbility.UIAbility}",
                reinterpret_cast<void*>(EtsApplicationContextUtils::GetUIAbilityByInstanceId)},
        };
        if ((status = aniEnv->Class_BindNativeMethods(contextClass, applicationContextFunctions.data(),
            applicationContextFunctions.size())) != ANI_OK) {
            TAG_LOGE(AAFwkTag::APPKIT, "Class_BindNativeMethods failed status: %{public}d", status);
            return;
        }
        ani_class cleanerCls = nullptr;
        if ((status = aniEnv->FindClass(CLEANER_CLASS, &cleanerCls)) != ANI_OK || cleanerCls == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "Cleaner FindClass failed status: %{public}d, or null cleanerCls", status);
            return;
        }
        std::array cleanerMethods = {
            ani_native_function {"clean", nullptr, reinterpret_cast<void *>(EtsApplicationContextUtils::Clean) },
        };
        if ((status = aniEnv->Class_BindNativeMethods(cleanerCls, cleanerMethods.data(),
            cleanerMethods.size())) != ANI_OK) {
            TAG_LOGE(AAFwkTag::APPKIT, "Class_BindNativeMethods failed status: %{public}d", status);
            return;
        }
    });
}

ani_object EtsApplicationContextUtils::CreateEtsApplicationContext(ani_env *aniEnv,
    const std::shared_ptr<ApplicationContext> &applicationContext)
{
    TAG_LOGD(AAFwkTag::APPKIT, "CreateEtsApplicationContext Call");
    if (applicationContext == nullptr || aniEnv == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationContext or aniEnv");
        return nullptr;
    }
    ani_object applicationContextObject = SetApplicationContext(aniEnv, applicationContext);
    ani_status status = ANI_ERROR;
    ani_ref applicationContextObjectRef = nullptr;
    if ((status = aniEnv->GlobalReference_Create(applicationContextObject, &applicationContextObjectRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "GlobalReference_Create failed status: %{public}d", status);
        return nullptr;
    }
    auto etsReference = std::make_shared<AppExecFwk::ETSNativeReference>();
    etsReference->aniObj = applicationContextObject;
    etsReference->aniRef = applicationContextObjectRef;
    ApplicationContextManager::GetApplicationContextManager().SetEtsGlobalObject(etsReference);
    BindApplicationContextFunc(aniEnv);
    ani_class applicationContextClass = nullptr;
    if ((status = aniEnv->FindClass(ETS_APPLICATION_CONTEXT_CLASS_NAME, &applicationContextClass)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "FindClass ApplicationContext failed status: %{public}d", status);
        return nullptr;
    }
    ContextUtil::CreateEtsBaseContext(aniEnv, applicationContextClass, applicationContextObject, applicationContext);
    ani_ref *contextGlobalRef = new (std::nothrow) ani_ref;
    if (contextGlobalRef == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null contextGlobalRef");
        return nullptr;
    }
    if ((status = aniEnv->GlobalReference_Create(applicationContextObject, contextGlobalRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "GlobalReference_Create failed status: %{public}d", status);
        delete contextGlobalRef;
        return nullptr;
    }
    applicationContext->Bind(contextGlobalRef);
    ani_ref eventHubRef = nullptr;
    if ((status = aniEnv->Object_GetFieldByName_Ref(applicationContextObject, "eventHub", &eventHubRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Object_GetFieldByName_Ref failed status: %{public}d", status);
        return nullptr;
    }
    SetEventHubContextIsApplicationContext(aniEnv, eventHubRef);
    return applicationContextObject;
}

void EtsApplicationContextUtils::SetEventHubContextIsApplicationContext(ani_env *aniEnv, ani_ref eventHubRef)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (aniEnv == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    ani_status status = ANI_ERROR;
    ani_class contextCls = nullptr;
    if ((status = aniEnv->FindClass(ETS_EVENT_HUB_CLASS_NAME, &contextCls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "FindClass Context failed status: %{public}d", status);
        return;
    }
    ani_field contextField;
    if ((status = aniEnv->Class_FindField(contextCls, "isApplicationContext", &contextField)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Class_FindField failed status: %{public}d", status);
        return;
    }
    if ((status = aniEnv->Object_SetField_Boolean(static_cast<ani_object>(eventHubRef), contextField,
        true)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Object_SetField_Boolean failed status: %{public}d", status);
        return;
    }
}
} // namespace AbilityRuntime
} // namespace OHOS
