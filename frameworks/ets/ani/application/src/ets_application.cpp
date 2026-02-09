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
#include "ets_application.h"

#include "ani_base_context.h"
#include "ani_common_util.h"
#include "application_context.h"
#include "app_mgr_client.h"
#include "application_context_manager.h"
#include "context_impl.h"
#include "ets_application_context_utils.h"
#include "ets_context_utils.h"
#include "ets_error_utils.h"
#include "ets_native_reference.h"
#include "hilog_tag_wrapper.h"
#include "permission_verification.h"
#include "application_env.h"
#include "ani_enum_convert.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char* PERMISSION_GET_BUNDLE_INFO = "ohos.permission.GET_BUNDLE_INFO_PRIVILEGED";
constexpr const char* CONTEXT_CLASS_NAME = "application.Context.Context";
constexpr const char* APPLICATION_SPACE_NAME = "@ohos.app.ability.application.application";
constexpr const char* APP_PRELOAD_TYPE_NAME = "@ohos.app.ability.application.application.AppPreloadType";
}

void EtsApplication::DemoteCurrentFromCandidateMasterProcess(ani_env *env, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "DemoteCurrentFromCandidateMasterProcess Call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    auto errCode = std::make_shared<int32_t>(ERR_OK);
    auto appMgrClient = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
    if (appMgrClient == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Null appMgrClient");
        *errCode = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
    } else {
        *errCode = appMgrClient->DemoteCurrentFromCandidateMasterProcess();
    }
    if (*errCode == ERR_OK) {
        TAG_LOGD(AAFwkTag::APPKIT, "demote to standby master process success");
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK),
        nullptr);
    } else {
        TAG_LOGE(AAFwkTag::APPKIT, "demote to standby master process failed, errCode: %{public}d", *errCode);
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateErrorByNativeErr(env, *errCode),
            nullptr);
    }
}

void EtsApplication::PromoteCurrentToCandidateMasterProcess(ani_env *env,
    ani_boolean isInsertToHead, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "PromoteCurrentToCandidateMasterProcess Call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    bool insertToHead = static_cast<bool>(isInsertToHead);
    auto errCode = std::make_shared<int32_t>(ERR_OK);
    auto appMgrClient = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
    if (appMgrClient == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Null appMgrClient");
        *errCode = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
    } else {
        *errCode = appMgrClient->PromoteCurrentToCandidateMasterProcess(insertToHead);
    }
    if (*errCode == ERR_OK) {
        TAG_LOGD(AAFwkTag::APPKIT, "promote to candidate master process success");
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK),
        nullptr);
    } else {
        TAG_LOGE(AAFwkTag::APPKIT, "promote to candidate master process failed, errCode: %{public}d", *errCode);
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateErrorByNativeErr(env, *errCode), nullptr);
    }
}

ani_object CreateEmptyContextObject(ani_env *env)
{
    ani_class cls = nullptr;
    ani_status status = env->FindClass(CONTEXT_CLASS_NAME, &cls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "find Context failed status: %{public}d", status);
        return nullptr;
    }
    ani_method method = nullptr;
    status = env->Class_FindMethod(cls, "<ctor>", ":", &method);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Class_FindMethod ctor failed status: %{public}d", status);
        return nullptr;
    }
    ani_object objValue = nullptr;
    if (env->Object_New(cls, method, &objValue) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Object_New failed status: %{public}d", status);
        return nullptr;
    }
    return objValue;
}

bool CheckIsSystemAppOrPermisson(ani_env *env, ani_object callback)
{
    auto emptyObject = CreateEmptyContextObject(env);
    if (!AAFwk::PermissionVerification::GetInstance()->IsSystemAppCall()) {
        TAG_LOGE(AAFwkTag::APPKIT, "no system app");
            AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env,
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP),
                "The application is not system-app, can not use system-api."), emptyObject);
        return false;
    }
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyGetBundleInfoPrivilegedPermission()) {
        TAG_LOGE(AAFwkTag::APPKIT, "no permission");
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateNoPermissionError(env, PERMISSION_GET_BUNDLE_INFO), emptyObject);
        return false;
    }
    return true;
}

bool SetNativeContextLong(ani_env *env, std::shared_ptr<Context> context, ani_class& cls, ani_object& contextObj)
{
    if (env == nullptr || context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "env or context is null");
        return false;
    }
    ani_status status = ANI_ERROR;
    ani_method method {};
    if ((status = env->Class_FindMethod(cls, "<ctor>", "l:", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "status: %{public}d", status);
        return false;
    }

    std::unique_ptr<ContextUtil::EtsBaseContext> eteBaseContext =
        std::make_unique<ContextUtil::EtsBaseContext>(context);
    auto workContext = new (std::nothrow) std::weak_ptr<Context>(eteBaseContext->GetContext());
    if (workContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "workContext nullptr");
        return false;
    }
    if ((status = env->Object_New(cls, method, &contextObj, (ani_long)(eteBaseContext.release()))) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "status: %{public}d", status);
        delete workContext;
        return false;
    }
    ani_long nativeContextLong = reinterpret_cast<ani_long>(workContext);
    if (!ContextUtil::SetNativeContextLong(env, contextObj, nativeContextLong)) {
        TAG_LOGE(AAFwkTag::APPKIT, "SetNativeContextLong failed");
        delete workContext;
        workContext = nullptr;
        return false;
    }
    return true;
}

void SetCreateCompleteCallback(ani_env *env, std::shared_ptr<std::shared_ptr<Context>> contextPtr, ani_object callback)
{
    if (env == nullptr || contextPtr == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "env or contextPtr is nullptr");
        return;
    }
    auto context = *contextPtr;
    if (!context) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to create context");
        auto emptyObject = CreateEmptyContextObject(env);
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env,
            AbilityErrorCode::ERROR_CODE_INVALID_PARAM), emptyObject);
        return;
    }
    ani_class cls {};
    ani_status status = ANI_ERROR;
    if ((status = env->FindClass(CONTEXT_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "status: %{public}d", status);
        return;
    }
    ani_object contextObj = nullptr;
    if (!SetNativeContextLong(env, context, cls, contextObj)) {
        TAG_LOGE(AAFwkTag::APPKIT, "set nativeContextLong failed");
        return;
    }
    ContextUtil::CreateEtsBaseContext(env, cls, contextObj, context);
    AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), contextObj);
}

std::shared_ptr<Context> GetContextByStageMode(ani_env *env, ani_object &contextObj,
    ani_object callback, ani_object emptyObject)
{
    ani_boolean stageMode = false;
    ani_status status = IsStageContext(env, contextObj, stageMode);
    if (status != ANI_OK || !stageMode) {
        TAG_LOGE(AAFwkTag::APPKIT, "not stageMode");
        return nullptr;
    }
    auto context = GetStageModeContext(env, contextObj);
    if (context == nullptr) {
        return nullptr;
    }
    return context;
}

void EtsApplication::CreateModuleContextCheck(ani_env *env,
    ani_object contextObj, ani_string moduleName, ani_object bundleName)
{
    TAG_LOGD(AAFwkTag::APPKIT, "CreateModuleContextCheck Call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    ani_boolean stageMode = false;
    ani_status status = IsStageContext(env, contextObj, stageMode);
    if (status != ANI_OK || !stageMode) {
        TAG_LOGE(AAFwkTag::APPKIT, "not stageMode");
        EtsErrorUtil::ThrowInvalidParamError(env,
            "Parse param context failed, must be a context of stageMode.");
        return;
    }
    auto context = GetStageModeContext(env, contextObj);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        EtsErrorUtil::ThrowInvalidParamError(env,
            "Parse param context failed, must not be nullptr.");
        return;
    }
    auto inputContextPtr = Context::ConvertTo<Context>(context);
    if (inputContextPtr == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Convert to context failed");
        EtsErrorUtil::ThrowInvalidParamError(env,
            "Parse param context failed, must be a context.");
        return;
    }
    ani_boolean isUndefined = false;
    if ((status = env->Reference_IsUndefined(bundleName, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to check undefined status : %{public}d", status);
        return;
    }
    if (!isUndefined) {
        if (!AAFwk::PermissionVerification::GetInstance()->IsSystemAppCall()) {
            TAG_LOGE(AAFwkTag::APPKIT, "no system app");
            EtsErrorUtil::ThrowNotSystemAppError(env);
            return;
        }
        if (!AAFwk::PermissionVerification::GetInstance()->VerifyGetBundleInfoPrivilegedPermission()) {
            TAG_LOGE(AAFwkTag::APPKIT, "no permission");
            EtsErrorUtil::ThrowNoPermissionError(env, PERMISSION_GET_BUNDLE_INFO);
            return;
        }
    }
}

void EtsApplication::CreateModuleContext(ani_env *env,
    ani_object contextObj, ani_string bundleName, ani_string moduleName, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "CreateModuleContext Call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    ani_object emptyObject = CreateEmptyContextObject(env);
    std::string stdBundleName = "";
    std::string stdModuleName = "";
    AppExecFwk::GetStdString(env, bundleName, stdBundleName);
    AppExecFwk::GetStdString(env, moduleName, stdModuleName);
    auto context = GetContextByStageMode(env, contextObj, callback, emptyObject);
    if (context == nullptr) {
        return;
    }
    auto inputContextPtr = Context::ConvertTo<Context>(context);
    if (inputContextPtr == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Convert to context failed");
        return;
    }
    std::shared_ptr<std::shared_ptr<Context>> moduleContext = std::make_shared<std::shared_ptr<Context>>();
    std::shared_ptr<ContextImpl> contextImpl = std::make_shared<ContextImpl>();
    if (contextImpl == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateInvalidParamError(env,
            "create context failed."), emptyObject);
        return;
    }
    contextImpl->SetProcessName(context->GetProcessName());
    if (stdBundleName.empty()) {
        *moduleContext = contextImpl->CreateModuleContext(stdModuleName, inputContextPtr);
    } else {
        if (!CheckIsSystemAppOrPermisson(env, callback)) {
            TAG_LOGE(AAFwkTag::APPKIT, "CheckCaller failed");
        }
        *moduleContext = contextImpl->CreateModuleContext(stdBundleName, stdModuleName, inputContextPtr);
    }
    SetCreateCompleteCallback(env, moduleContext, callback);
}

void EtsApplication::CreateBundleContextCheck(ani_env *env,
    ani_object contextObj, ani_string bundleName)
{
    if (!AAFwk::PermissionVerification::GetInstance()->IsSystemAppCall()) {
        TAG_LOGE(AAFwkTag::APPKIT, "no system app");
        EtsErrorUtil::ThrowNotSystemAppError(env);
        return;
    }
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyGetBundleInfoPrivilegedPermission()) {
        TAG_LOGE(AAFwkTag::APPKIT, "no permission");
        EtsErrorUtil::ThrowNoPermissionError(env, PERMISSION_GET_BUNDLE_INFO);
        return;
    }
    ani_boolean stageMode = false;
    ani_status status = IsStageContext(env, contextObj, stageMode);
    if (status != ANI_OK || !stageMode) {
        TAG_LOGE(AAFwkTag::APPKIT, "not stageMode");
        EtsErrorUtil::ThrowInvalidParamError(env,
            "Parse param context failed, must be a context of stageMode.");
        return;
    }
    auto context = GetStageModeContext(env, contextObj);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        EtsErrorUtil::ThrowInvalidParamError(env,
            "Parse param context failed, must not be nullptr.");
        return;
    }
    auto inputContextPtr = Context::ConvertTo<Context>(context);
    if (inputContextPtr == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Convert to context failed");
        EtsErrorUtil::ThrowInvalidParamError(env,
            "Parse param context failed, must be a context.");
        return;
    }
}

void EtsApplication::CreateBundleContext(ani_env *env,
    ani_object contextObj, ani_string bundleName, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "CreateBundleContext Call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    ani_object emptyObject = CreateEmptyContextObject(env);
    if (!CheckIsSystemAppOrPermisson(env, callback)) {
        TAG_LOGE(AAFwkTag::APPKIT, "CheckCaller failed");
        return;
    }
    std::string stdBundleName = "";
    AppExecFwk::GetStdString(env, bundleName, stdBundleName);
    auto context = GetContextByStageMode(env, contextObj, callback, emptyObject);
    if (context == nullptr) {
        return;
    }
    auto inputContextPtr = Context::ConvertTo<Context>(context);
    if (inputContextPtr == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Convert to context failed");
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateInvalidParamError(env,
            "Parse param context failed, must be a context."), emptyObject);
        return;
    }
    auto bundleContext = std::make_shared<std::shared_ptr<Context>>();
    std::shared_ptr<ContextImpl> contextImpl = std::make_shared<ContextImpl>();
    if (contextImpl == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateInvalidParamError(env,
            "create context failed."), emptyObject);
        return;
    }
    contextImpl->SetProcessName(context->GetProcessName());
    contextImpl->CreateBundleContext(*bundleContext, stdBundleName, inputContextPtr);
    SetCreateCompleteCallback(env, bundleContext, callback);
}

void EtsApplication::CreatePluginModuleContextCheck(ani_env *env,
    ani_object contextObj, ani_string pluginBundleName, ani_string pluginModuleName)
{
    TAG_LOGD(AAFwkTag::APPKIT, "CreatePluginModuleContextCheck Call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    ani_boolean stageMode = false;
    ani_status status = OHOS::AbilityRuntime::IsStageContext(env, contextObj, stageMode);
    if (status != ANI_OK || !stageMode) {
        EtsErrorUtil::ThrowInvalidParamError(
            env, "Parse param context failed, must be a context of stageMode.");
        return;
    }
    auto context = OHOS::AbilityRuntime::GetStageModeContext(env, contextObj);
    if (context == nullptr) {
        EtsErrorUtil::ThrowInvalidParamError(
            env, "Parse param context failed, must not be nullptr.");
        return;
    }
    auto inputContextPtr = Context::ConvertTo<Context>(context);
    if (inputContextPtr == nullptr) {
        EtsErrorUtil::ThrowInvalidParamError(
            env, "Parse param context failed, must be a context.");
        return;
    }
}

void EtsApplication::CreatePluginModuleContext(ani_env *env,
    ani_object contextObj, ani_string pluginBundleName, ani_string pluginModuleName, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "CreatePluginModuleContext Call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    ani_boolean stageMode = false;
    ani_status status = OHOS::AbilityRuntime::IsStageContext(env, contextObj, stageMode);
    if (status != ANI_OK || !stageMode) {
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateInvalidParamError(env, "Parse param context failed, must be a context of stageMode."),
            nullptr);
        return;
    }
    auto context = OHOS::AbilityRuntime::GetStageModeContext(env, contextObj);
    if (context == nullptr) {
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateInvalidParamError(env, "Parse param context failed, must not be nullptr."), nullptr);
        return;
    }
    auto inputContextPtr = Context::ConvertTo<Context>(context);
    if (inputContextPtr == nullptr) {
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateInvalidParamError(env, "Parse param context failed, must be a context."), nullptr);
        return;
    }
    std::string stdPluginBundleName = "";
    std::string stdModuleName = "";
    AppExecFwk::GetStdString(env, pluginBundleName, stdPluginBundleName);
    AppExecFwk::GetStdString(env, pluginModuleName, stdModuleName);
    TAG_LOGD(AAFwkTag::APPKIT, "pluginModuleName: %{public}s, pluginBundleName: %{public}s",
        stdModuleName.c_str(), stdPluginBundleName.c_str());
    if (stdPluginBundleName.empty() || stdModuleName.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "Empty pluginBundleName or moduleName");
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateInvalidParamError(env, "Empty pluginBundleName or moduleName"), nullptr);
        return;
    }
    auto moduleContext = std::make_shared<std::shared_ptr<Context>>();
    auto contextImpl = std::make_shared<ContextImpl>();
    if (contextImpl == nullptr) {
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateInvalidParamError(env, "create context failed."), nullptr);
        return;
    }
    contextImpl->SetProcessName(context->GetProcessName());
    *moduleContext = contextImpl->CreatePluginContext(stdPluginBundleName, stdModuleName, inputContextPtr);
    SetCreateCompleteCallback(env, moduleContext, callback);
}

ani_object EtsApplication::GetApplicationContextInstance(ani_env *env)
{
    TAG_LOGD(AAFwkTag::APPKIT, "GetApplicationContextInstance Call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return nullptr;
    }
    auto etsReference =
        AbilityRuntime::ApplicationContextManager::GetApplicationContextManager().GetEtsGlobalObject();
    if (etsReference == nullptr || etsReference->aniRef == nullptr) {
        auto applicationContext = ApplicationContext::GetInstance();
        ani_object applicationContextObject =
            EtsApplicationContextUtils::CreateEtsApplicationContext(env, applicationContext);
        if (applicationContextObject == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null applicationContextObject");
            AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
            ani_ref result = nullptr;
            env->GetNull(&result);
            return static_cast<ani_object>(result);
        }
        return applicationContextObject;
    }
    return reinterpret_cast<ani_object>(etsReference->aniRef);
}

ani_object EtsApplication::GetApplicationContext(ani_env *env)
{
    TAG_LOGD(AAFwkTag::APPKIT, "GetApplicationContext Call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return nullptr;
    }

    auto applicationContext = ApplicationContext::GetInstance();
    ani_object applicationContextObject =
        EtsApplicationContextUtils::CreateEtsApplicationContext(env, applicationContext);
    if (applicationContextObject == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationContextObject");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        ani_ref result = nullptr;
        env->GetNull(&result);
        return static_cast<ani_object>(result);
    }
    return applicationContextObject;
}

ani_enum_item EtsApplication::GetAppPreloadType(ani_env *env)
{
    TAG_LOGD(AAFwkTag::APPKIT, "GetAppPreloadType Call");
    ani_enum_item appPreloadTypeItem = nullptr;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return appPreloadTypeItem;
    }
    auto appPreload = GetAppPreload();
    OHOS::AAFwk::AniEnumConvertUtil::EnumConvert_NativeToEts(
        env, APP_PRELOAD_TYPE_NAME, appPreload, appPreloadTypeItem);
    return appPreloadTypeItem;
}

void EtsApplication::ExitMasterProcessRole(ani_env *env, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "ExitMasterProcessRole Call");
    int32_t errCode = ERR_OK;
    auto appMgrClient = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
    if (appMgrClient == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Null appMgrClient");
        errCode = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
    } else {
        errCode = appMgrClient->ExitMasterProcessRole();
    }
    if (errCode == ERR_OK) {
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), nullptr);
    } else {
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateErrorByNativeErr(env, errCode), nullptr);
    }
}

void EtsApplication::CreatePluginModuleContextForHostBundleCheck(ani_env *env, ani_object contextObj,
    ani_string pluginBundleName, ani_string pluginModuleName, ani_string hostBundleName, ani_object callback)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "ConnectUIServiceExtensionCheck called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env or aniObj");
        return;
    }

    if (!AAFwk::PermissionVerification::GetInstance()->IsSystemAppCall()) {
        TAG_LOGE(AAFwkTag::APPKIT, "no system app");
        EtsErrorUtil::ThrowNotSystemAppError(env);
        return;
    }

    if (!AAFwk::PermissionVerification::GetInstance()->VerifyGetBundleInfoPrivilegedPermission()) {
        TAG_LOGE(AAFwkTag::APPKIT, "no permission");
        EtsErrorUtil::ThrowNoPermissionError(env, PERMISSION_GET_BUNDLE_INFO);
        return;
    }

    std::string stdPluginBundleName = "";
    std::string stdPluginModuleName = "";
    std::string stdHostBundleName = "";
    if (!AppExecFwk::GetStdString(env, pluginBundleName, stdPluginBundleName)
        || !AppExecFwk::GetStdString(env, pluginModuleName, stdPluginModuleName)
        || !AppExecFwk::GetStdString(env, hostBundleName, stdHostBundleName)) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid params");
        EtsErrorUtil::ThrowInvalidParamError(env,
            "Parse param failed, moduleName and pluginBundleName must be string.");
        return;
    }
    
    ani_boolean stageMode = false;
    ani_status status = IsStageContext(env, contextObj, stageMode);
    if (status != ANI_OK || !stageMode) {
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param context failed, must be a context of stageMode.");
        return;
    }

    auto context = GetStageModeContext(env, contextObj);
    if (context == nullptr) {
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param context failed, must not be nullptr.");
        return;
    }

    auto inputContextPtr = Context::ConvertTo<Context>(context);
    if (inputContextPtr == nullptr) {
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param context failed, must be a context.");
        return;
    }
}

void EtsApplication::CreatePluginModuleContextForHostBundle(ani_env *env, ani_object contextObj,
    ani_string pluginBundleName, ani_string pluginModuleName, ani_string hostBundleName, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "CreatePluginModuleContextForHostBundle Call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }

    std::string stdPluginBundleName = "";
    std::string stdPluginModuleName = "";
    std::string stdHostBundleName = "";
    if (!AppExecFwk::GetStdString(env, pluginBundleName, stdPluginBundleName)
        || !AppExecFwk::GetStdString(env, pluginModuleName, stdPluginModuleName)
        || !AppExecFwk::GetStdString(env, hostBundleName, stdHostBundleName)) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid params");
        return;
    }
    
    ani_boolean stageMode = false;
    ani_status status = IsStageContext(env, contextObj, stageMode);
    if (status != ANI_OK || !stageMode) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid IsStageContext");
        return;
    }

    auto context = GetStageModeContext(env, contextObj);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid context");
        return;
    }

    auto inputContextPtr = Context::ConvertTo<Context>(context);
    if (inputContextPtr == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid inputContextPtr");
        return;
    }

    auto moduleContext = std::make_shared<std::shared_ptr<Context>>();
    std::shared_ptr<ContextImpl> contextImpl = std::make_shared<ContextImpl>();
    if (contextImpl == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid contextImpl");
        return;
    }

    contextImpl->SetProcessName(context->GetProcessName());
    *moduleContext = contextImpl->CreateTargetPluginContext(stdHostBundleName, stdPluginBundleName,
        stdPluginModuleName, inputContextPtr);

    SetCreateCompleteCallback(env, moduleContext, callback);
}

void ApplicationInit(ani_env *env)
{
    TAG_LOGD(AAFwkTag::APPKIT, "ApplicationInit Call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    ani_status status = ANI_ERROR;
    ani_namespace ns;
    status = env->FindNamespace(APPLICATION_SPACE_NAME, &ns);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "FindNamespace application failed status: %{public}d", status);
        return;
    }
    std::array methods = {
        ani_native_function {
            "nativeCreateModuleContext",
            "C{application.Context.Context}C{std.core.String}C{std.core.String}"
            "C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsApplication::CreateModuleContext)
        },
        ani_native_function {
            "nativeCreateModuleContextCheck",
            "C{application.Context.Context}C{std.core.String}C{std.core.String}:",
            reinterpret_cast<void *>(EtsApplication::CreateModuleContextCheck)
        },
        ani_native_function {
            "nativeCreateBundleContext",
            "C{application.Context.Context}C{std.core.String}"
            "C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsApplication::CreateBundleContext)
        },
        ani_native_function {
            "nativeCreateBundleContextCheck",
            "C{application.Context.Context}C{std.core.String}:",
            reinterpret_cast<void *>(EtsApplication::CreateBundleContextCheck)
        },
        ani_native_function {
            "nativeCreatePluginModuleContext",
            "C{application.Context.Context}C{std.core.String}C{std.core.String}"
            "C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsApplication::CreatePluginModuleContext)
        },
        ani_native_function {
            "nativeCreatePluginModuleContextCheck",
            "C{application.Context.Context}C{std.core.String}C{std.core.String}:",
            reinterpret_cast<void *>(EtsApplication::CreatePluginModuleContextCheck)
        },
        ani_native_function {
            "nativeGetApplicationContext",
            ":C{application.ApplicationContext.ApplicationContext}",
            reinterpret_cast<void *>(EtsApplication::GetApplicationContext)
        },
        ani_native_function {
            "nativeDemoteCurrentFromCandidateMasterProcess",
            "C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsApplication::DemoteCurrentFromCandidateMasterProcess)
        },
        ani_native_function {
            "nativePromoteCurrentToCandidateMasterProcess",
            "zC{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsApplication::PromoteCurrentToCandidateMasterProcess)
        },
        ani_native_function {
            "nativeGetApplicationContextInstance",
            ":C{application.ApplicationContext.ApplicationContext}",
            reinterpret_cast<void *>(EtsApplication::GetApplicationContextInstance)
        },
        ani_native_function {
            "nativeGetAppPreloadType",
            ":C{@ohos.app.ability.application.application.AppPreloadType}",
            reinterpret_cast<void *>(EtsApplication::GetAppPreloadType)
        },
        ani_native_function {
            "nativeExitMasterProcessRole",
            "C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsApplication::ExitMasterProcessRole)
        },
        ani_native_function {
            "nativeCreatePluginModuleContextForHostBundle",
            "C{application.Context.Context}C{std.core.String}"
            "C{std.core.String}C{std.core.String}"
            "C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsApplication::CreatePluginModuleContextForHostBundle)
        },
        ani_native_function {
            "nativeCreatePluginModuleContextForHostBundleCheck",
            "C{application.Context.Context}C{std.core.String}"
            "C{std.core.String}C{std.core.String}:",
            reinterpret_cast<void *>(EtsApplication::CreatePluginModuleContextForHostBundleCheck)
        },
    };
    status = env->Namespace_BindNativeFunctions(ns, methods.data(), methods.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Namespace_BindNativeFunctions failed status: %{public}d", status);
    }
}

extern "C" {
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    TAG_LOGD(AAFwkTag::APPKIT, "in ApplicationETS.ANI_Constructor");
    if (vm == nullptr || result == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null vm or result");
        return ANI_INVALID_ARGS;
    }
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    status = vm->GetEnv(ANI_VERSION_1, &env);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "GetEnv failed, status: %{public}d", status);
        return ANI_NOT_FOUND;
    }
    ApplicationInit(env);
    *result = ANI_VERSION_1;
    TAG_LOGD(AAFwkTag::APPKIT, "AbilityManagerEts.ANI_Constructor finished");
    return ANI_OK;
}
}
} // namespace AbilityRuntime
} // namespace OHOS