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

#include "ets_context_utils.h"

#include "ani_common_util.h"
#include "ani_enum_convert.h"
#include "ability_runtime_error_util.h"
#include "application_context.h"
#include "application_context_manager.h"
#include "common_fun_ani.h"
#include "ets_application_context_utils.h"
#include "ets_error_utils.h"
#include "ets_native_reference.h"
#include "event_hub.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "resourceManager.h"
#include "tokenid_kit.h"

namespace OHOS {
namespace AbilityRuntime {
namespace ContextUtil {
namespace {
static std::once_flag g_bindNativeMethodsFlag;
constexpr const char* CONTEXT_CLASS_NAME = "application.Context.Context";
constexpr const char* AREA_MODE_ENUM_NAME = "@ohos.app.ability.contextConstant.contextConstant.AreaMode";
constexpr const char* CLEANER_CLASS = "application.Context.Cleaner";
constexpr const char* ETS_EVENT_HUB_CLASS_NAME = "application.EventHub.EventHub";


void BindContextDirInner(ani_env *aniEnv, ani_object contextObj, std::shared_ptr<Context> context)
{
    ani_status status = ANI_ERROR;
    auto cloudFileDir = context->GetCloudFileDir();
    ani_string cloudFileDirString = nullptr;
    aniEnv->String_NewUTF8(cloudFileDir.c_str(), cloudFileDir.size(), &cloudFileDirString);
    if ((status = aniEnv->Object_SetFieldByName_Ref(contextObj, "cloudFileDir", cloudFileDirString)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "cloudFileDir SetField status: %{public}d", status);
        return;
    }

    auto distributedFilesDir = context->GetDistributedFilesDir();
    ani_string distributedFilesDirString = nullptr;
    aniEnv->String_NewUTF8(distributedFilesDir.c_str(), distributedFilesDir.size(), &distributedFilesDirString);
    if ((status = aniEnv->Object_SetFieldByName_Ref(contextObj, "distributedFilesDir",
        distributedFilesDirString)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "distributedFilesDir SetField status: %{public}d", status);
        return;
    }

    auto bundleCodeDir = context->GetBundleCodeDir();
    ani_string bundleCodeDirString = nullptr;
    aniEnv->String_NewUTF8(bundleCodeDir.c_str(), bundleCodeDir.size(), &bundleCodeDirString);
    if ((status = aniEnv->Object_SetFieldByName_Ref(contextObj, "bundleCodeDir", bundleCodeDirString)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "bundleCodeDir SetField status: %{public}d", status);
        return;
    }

    auto resourceDir = context->GetResourceDir();
    ani_string resourceDirString = nullptr;
    aniEnv->String_NewUTF8(resourceDir.c_str(), resourceDir.size(), &resourceDirString);
    if ((status = aniEnv->Object_SetFieldByName_Ref(contextObj, "resourceDir", resourceDirString)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "resourceDir SetField status: %{public}d", status);
        return;
    }
}

void BindContextDir(ani_env *aniEnv, ani_object contextObj, std::shared_ptr<Context> context)
{
    if (aniEnv == nullptr || context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "aniEnv or context is nullptr");
        return;
    }
    ani_status status = ANI_ERROR;
    auto preferencesDir = context->GetPreferencesDir();
    ani_string preferencesDirString = nullptr;
    aniEnv->String_NewUTF8(preferencesDir.c_str(), preferencesDir.size(), &preferencesDirString);
    if ((status = aniEnv->Object_SetFieldByName_Ref(contextObj, "preferencesDir", preferencesDirString)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "preferencesDir SetField status: %{public}d", status);
        return;
    }

    auto databaseDir = context->GetDatabaseDir();
    ani_string databaseDirString = nullptr;
    aniEnv->String_NewUTF8(databaseDir.c_str(), databaseDir.size(), &databaseDirString);
    if ((status = aniEnv->Object_SetFieldByName_Ref(contextObj, "databaseDir", databaseDirString)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "databaseDir SetField status: %{public}d", status);
        return;
    }

    auto cacheDir = context->GetCacheDir();
    ani_string cacheDirString = nullptr;
    aniEnv->String_NewUTF8(cacheDir.c_str(), cacheDir.size(), &cacheDirString);
    if ((status = aniEnv->Object_SetFieldByName_Ref(contextObj, "cacheDir", cacheDirString)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "cacheDir SetField status: %{public}d", status);
        return;
    }

    auto filesDir = context->GetFilesDir();
    ani_string filesDirString = nullptr;
    aniEnv->String_NewUTF8(filesDir.c_str(), filesDir.size(), &filesDirString);
    if ((status = aniEnv->Object_SetFieldByName_Ref(contextObj, "filesDir", filesDirString)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "filesDir SetField status: %{public}d", status);
        return;
    }

    auto tempDir = context->GetTempDir();
    ani_string tempDirString = nullptr;
    aniEnv->String_NewUTF8(tempDir.c_str(), tempDir.size(), &tempDirString);
    if ((status = aniEnv->Object_SetFieldByName_Ref(contextObj, "tempDir", tempDirString)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "tempDir SetField status: %{public}d", status);
        return;
    }

    BindContextDirInner(aniEnv, contextObj, context);
}
} // namespace

void Clean(ani_env *env, ani_object object)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Clean called");
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
        delete reinterpret_cast<std::weak_ptr<Context> *>(ptr);
    }
    ani_long nativePtr = 0;
    status = env->Object_GetFieldByName_Long(object, "nativePtr", &nativePtr);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "nativePtr GetField status: %{public}d", status);
        return;
    }
    if (nativePtr != 0) {
        delete reinterpret_cast<EtsBaseContext *>(nativePtr);
    }
}

bool SetNativeContextLong(ani_env *env, ani_object aniObj, ani_long nativeContextLong)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return false;
    }
    auto status = env->Object_SetFieldByName_Long(aniObj, "nativeContext", nativeContextLong);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "nativeContextLong SetField status: %{public}d", status);
        return false;
    }
    ani_class contextCls = nullptr;
    if (env->FindClass(CONTEXT_CLASS_NAME, &contextCls) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "FindClass Context failed");
        return false;
    }
    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(contextCls, "<ctor>", ":", &method)) != ANI_OK ||
        method == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "ctor FindMethod status: %{public}d, or null method", status);
        return false;
    }
    ani_object contextObj = nullptr;
    if ((status = env->Object_New(contextCls, method, &contextObj)) != ANI_OK || contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Object_New status: %{public}d, or null contextObj", status);
        return false;
    }
    if ((status = env->Class_FindMethod(contextCls, "setEtsContextPtr", "l:", &method)) != ANI_OK ||
        method == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "setEtsContextPtr FindMethod status: %{public}d, or null method", status);
        return false;
    }
    if ((status = env->Object_CallMethod_Void(contextObj, method, nativeContextLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "call contextObj method failed, status: %{public}d", status);
        return false;
    }
    return true;
}

void BindApplicationInfo(ani_env *aniEnv, ani_class contextClass, ani_object contextObj,
    std::shared_ptr<Context> context)
{
    if (aniEnv == nullptr || context ==  nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null aniEnv or null context");
    }
    ani_field applicationInfoField;
    if (ANI_OK != aniEnv->Class_FindField(contextClass, "applicationInfo", &applicationInfoField)) {
        TAG_LOGE(AAFwkTag::APPKIT, "find applicationInfo failed");
        return;
    }
    auto appInfo = context->GetApplicationInfo();
    ani_object appInfoObj = AppExecFwk::CommonFunAni::ConvertApplicationInfo(aniEnv, *appInfo);
    if (aniEnv->Object_SetField_Ref(contextObj, applicationInfoField,
        reinterpret_cast<ani_ref>(appInfoObj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Object_SetField_Ref failed");
        return;
    }
}

void BindResourceManager(ani_env *aniEnv, ani_class contextClass, ani_object contextObj,
    std::shared_ptr<Context> context)
{
    if (aniEnv == nullptr || context ==  nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null aniEnv or null context");
    }
    ani_field resourceManagerField;
    if (ANI_OK != aniEnv->Class_FindField(contextClass, "resourceManager", &resourceManagerField)) {
        TAG_LOGE(AAFwkTag::APPKIT, "find resourceManager failed");
        return;
    }
    auto resourceManager = context->GetResourceManager();
    ani_object resourceMgrObj = Global::Resource::ResMgrAddon::CreateResMgr(aniEnv, "", resourceManager, context);
    if (aniEnv->Object_SetField_Ref(contextObj, resourceManagerField,
        reinterpret_cast<ani_ref>(resourceMgrObj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Object_SetField_Ref failed");
        return;
    }
}

void BindParentProperty(ani_env *aniEnv, ani_class contextClass, ani_object contextObj,
    std::shared_ptr<Context> context)
{
    if (aniEnv == nullptr || context ==  nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null aniEnv or null context");
    }
    BindApplicationInfo(aniEnv, contextClass, contextObj, context);
    BindResourceManager(aniEnv, contextClass, contextObj, context);
    BindContextDir(aniEnv, contextObj, context);
    ani_status status = ANI_ERROR;
    ani_field processNameField;
    if ((status = aniEnv->Class_FindField(contextClass, "processName", &processNameField)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "find processName failed status: %{public}d", status);
        return;
    }
    auto processName = context->GetProcessName();
    ani_string processNameString = nullptr;
    aniEnv->String_NewUTF8(processName.c_str(), processName.size(), &processNameString);
    if ((status = aniEnv->Object_SetField_Ref(contextObj, processNameField,
        reinterpret_cast<ani_ref>(processNameString))) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Object_SetField_Ref failed, status: %{public}d", status);
        return;
    }
}

void BindNativeFunction(ani_env *aniEnv)
{
    if (aniEnv == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "env is null");
        return;
    }
    ani_class contextCls = nullptr;
    if (aniEnv->FindClass(CONTEXT_CLASS_NAME, &contextCls) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "FindClass Context failed");
        return;
    }
    ani_status status = ANI_ERROR;
    std::call_once(g_bindNativeMethodsFlag, [&status, aniEnv, contextCls]() {
        std::array contextFunctions = {
            ani_native_function {"getApplicationContextSync", ":C{application.ApplicationContext.ApplicationContext}",
                reinterpret_cast<void *>(ContextUtil::GetApplicationContextSync)},
            ani_native_function {"switchArea", nullptr,
                reinterpret_cast<void *>(ContextUtil::SwitchArea)},
            ani_native_function {"getArea", nullptr,
                reinterpret_cast<void *>(ContextUtil::GetArea)},
            ani_native_function {"getLogFileDir", nullptr,
                reinterpret_cast<void *>(ContextUtil::GetLogFileDir)},
            ani_native_function {"createModuleResourceManagerSync", "C{std.core.String}C{std.core.String}"
                ":C{@ohos.resourceManager.resourceManager.ResourceManager}",
                reinterpret_cast<void *>(ContextUtil::CreateModuleResourceManagerSync)},
            ani_native_function {"nativeGetGroupDir", nullptr,
                reinterpret_cast<void *>(ContextUtil::NativeGetGroupDir)},
            ani_native_function {"nativeCreateDisplayContext", "l:C{application.Context.Context}",
                reinterpret_cast<void *>(ContextUtil::NativeCreateDisplayContext)},
            ani_native_function {"nativeCreateAreaModeContext",
                "C{@ohos.app.ability.contextConstant.contextConstant.AreaMode}:C{application.Context.Context}",
                reinterpret_cast<void *>(ContextUtil::NativeCreateAreaModeContext)},
            ani_native_function {"nativeCreateSystemHspModuleResourceManager", "C{std.core.String}C{std.core.String}"
                ":C{@ohos.resourceManager.resourceManager.ResourceManager}",
                reinterpret_cast<void *>(ContextUtil::NativeCreateSystemHspModuleResourceManager)},
            ani_native_function {"nativeContextType",
                "C{@ohos.app.ability.contextConstant.contextConstant.ContextType}:z",
                reinterpret_cast<void *>(ContextUtil::ContextType)},
        };
        status = aniEnv->Class_BindNativeMethods(contextCls, contextFunctions.data(),
            contextFunctions.size());
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::APPKIT, "Class_BindNativeMethods failed status: %{public}d", status);
            return;
        }
        ani_class cleanerCls = nullptr;
        if ((status = aniEnv->FindClass(CLEANER_CLASS, &cleanerCls)) != ANI_OK || cleanerCls == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "Cleaner FindClass failed status: %{public}d, or null cleanerCls", status);
            return;
        }
        std::array cleanerMethods = {
            ani_native_function {"clean", nullptr, reinterpret_cast<void *>(Clean) },
        };
        if ((status = aniEnv->Class_BindNativeMethods(cleanerCls,
            cleanerMethods.data(), cleanerMethods.size())) != ANI_OK) {
            TAG_LOGE(AAFwkTag::APPKIT, "Class_BindNativeMethods failed status: %{public}d", status);
            return;
        }
    });
}

bool SetHapModuleInfo(
    ani_env *env, ani_class cls, ani_object contextObj, const std::shared_ptr<OHOS::AbilityRuntime::Context> &context)
{
    if (env == nullptr || context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env or context");
        return false;
    }
    auto hapModuleInfo = context->GetHapModuleInfo();
    if (hapModuleInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "hapModuleInfo is nullptr");
        return false;
    }
    ani_ref hapModuleInfoRef = AppExecFwk::CommonFunAni::ConvertHapModuleInfo(env, *hapModuleInfo);
    if (hapModuleInfoRef == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "hapModuleInfoRef is nullptr");
        return false;
    }
    ani_status status = ANI_OK;
    status = env->Object_SetPropertyByName_Ref(contextObj, "currentHapModuleInfo", hapModuleInfoRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Object_SetPropertyByName_Ref failed, status: %{public}d", status);
        return false;
    }
    return true;
}

void CreateEtsBaseContext(ani_env *aniEnv, ani_class contextClass, ani_object contextObj,
    std::shared_ptr<Context> context)
{
    if (aniEnv == nullptr || context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null aniEnv or null context");
        return;
    }
    if (!SetHapModuleInfo(aniEnv, contextClass, contextObj, context)) {
        TAG_LOGE(AAFwkTag::APPKIT, "SetHapModuleInfo fail");
    }
    BindParentProperty(aniEnv, contextClass, contextObj, context);
    BindNativeFunction(aniEnv);
    // set eventHub context
    ani_ref eventHubRef = nullptr;
    ani_status status = ANI_OK;
    if ((status = aniEnv->Object_GetFieldByName_Ref(contextObj, "eventHub", &eventHubRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Object_GetFieldByName_Ref failed status: %{public}d", status);
        return;
    }

    auto workContext = new (std::nothrow) std::weak_ptr<AbilityRuntime::Context>(context);
    if (workContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "workContext null");
        return;
    }
    ani_long nativeContextLong = (ani_long)workContext;
    SetEventHubContext(aniEnv, eventHubRef, nativeContextLong);
}

void SetEventHubContext(ani_env *aniEnv, ani_ref eventHubRef, ani_long nativeContextLong)
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
    if ((status = aniEnv->Class_FindField(contextCls, "nativeContext", &contextField)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Class_FindField failed status: %{public}d", status);
        return;
    }
    if ((status = aniEnv->Object_SetField_Long(static_cast<ani_object>(eventHubRef), contextField,
        nativeContextLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Object_SetField_Long failed status: %{public}d", status);
        return;
    }
}

std::shared_ptr<Context> GetBaseContext(ani_env *env, ani_object aniObj)
{
    ani_status status = ANI_ERROR;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return nullptr;
    }
    ani_class cls = nullptr;
    if ((status = env->FindClass(CONTEXT_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "status: %{public}d", status);
        return nullptr;
    }
    ani_field contextField = nullptr;
    if ((status = env->Class_FindField(cls, "nativeContext", &contextField)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "status: %{public}d", status);
        return nullptr;
    }
    ani_long nativeContextLong;
    if ((status = env->Object_GetField_Long(aniObj, contextField, &nativeContextLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "status: %{public}d", status);
        return nullptr;
    }
    auto weakContext = reinterpret_cast<std::weak_ptr<Context>*>(nativeContextLong);
    return weakContext != nullptr ? weakContext->lock() : nullptr;
}

ani_object CreateModuleResourceManagerSync(ani_env *env, ani_object aniObj,
    ani_string bundleName, ani_string moduleName)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return nullptr;
    }
    std::string stdBundleName = "";
    AppExecFwk::GetStdString(env, bundleName, stdBundleName);
    std::string stdModuleName = "";
    AppExecFwk::GetStdString(env, moduleName, stdModuleName);
    auto context = GetBaseContext(env, aniObj);
    if (!context) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return nullptr;
    }
    if (!AppExecFwk::CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::APPKIT, "not system-app");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return nullptr;
    }
    auto resourceManager = context->CreateModuleResourceManager(stdBundleName, stdModuleName);
    if (resourceManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null resourceManager");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return nullptr;
    }
    return Global::Resource::ResMgrAddon::CreateResMgr(env, "", resourceManager, context);
}

void NativeGetGroupDir([[maybe_unused]]ani_env *env, [[maybe_unused]]ani_object aniObj,
    ani_string dataGroupIdObj, ani_object callBackObj)
{
    TAG_LOGD(AAFwkTag::APPKIT, "NativeGetGroupDir");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "env is nullptr");
        return;
    }
    std::string dataGroupId = "";
    if (!AppExecFwk::GetStdString(env, dataGroupIdObj, dataGroupId)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse groupId failed");
        AppExecFwk::AsyncCallback(env, callBackObj, EtsErrorUtil::CreateError(env,
            AbilityErrorCode::ERROR_CODE_INVALID_PARAM), nullptr);
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param groupId failed, groupId must be string.");
        return;
    }
    auto context = GetBaseContext(env, aniObj);
    if (!context) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        AppExecFwk::AsyncCallback(env, callBackObj, EtsErrorUtil::CreateError(env,
            AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT), nullptr);
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return;
    }
    ErrCode ret = ERR_OK;
    std::string path = context->GetGroupDir(dataGroupId);
    ani_object errorObject = EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(ret));
    ani_string aniPath = AppExecFwk::GetAniString(env, path);
    AppExecFwk::AsyncCallback(env, callBackObj, errorObject, aniPath);
}

ani_object NativeCreateDisplayContext(ani_env *env, ani_object aniObj, ani_long displayId)
{
    TAG_LOGD(AAFwkTag::APPKIT, "NativeCreateDisplayContext");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return nullptr;
    }
    ani_ref undefRef = nullptr;
    ani_status status = env->GetUndefined(&undefRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "GetUndefined failed %{public}d", status);
        return nullptr;
    }
#ifdef SUPPORT_GRAPHICS
    auto context = GetBaseContext(env, aniObj);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        EtsErrorUtil::ThrowRuntimeError(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return reinterpret_cast<ani_object>(undefRef);
    }
    if (displayId < 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "displayId is invalid, less than 0");
        EtsErrorUtil::ThrowRuntimeError(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return reinterpret_cast<ani_object>(undefRef);
    }
    uint64_t validDisplayId = static_cast<uint64_t>(displayId);
    auto displayContext = context->CreateDisplayContext(validDisplayId);
    if (displayContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to create displayContext");
        return reinterpret_cast<ani_object>(undefRef);
    }
    ani_class contextClass = nullptr;
    status = env->FindClass(CONTEXT_CLASS_NAME, &contextClass);
    if (status != ANI_OK || contextClass == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "FindClass Context failed, status: %{public}d", status);
        EtsErrorUtil::ThrowInvalidParamError(env, "FindClass failed");
        return reinterpret_cast<ani_object>(undefRef);
    }
    ani_object displayContextObj = CreateContextObject(env, contextClass, displayContext);
    if (displayContextObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextObj");
        return reinterpret_cast<ani_object>(undefRef);
    }
    return displayContextObj;
#else
    return reinterpret_cast<ani_object>(undefRef);
#endif
}

ani_object NativeCreateAreaModeContext(ani_env *env, ani_object aniObj, ani_object areaModeObj)
{
    TAG_LOGD(AAFwkTag::APPKIT, "NativeCreateAreaModeContext");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return nullptr;
    }
    ani_ref undefRef = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->GetUndefined(&undefRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "GetUndefined failed %{public}d", status);
        return nullptr;
    }
    auto context = GetBaseContext(env, aniObj);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return reinterpret_cast<ani_object>(undefRef);
    }
    int areaMode = 0;
    if (!AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative(env, areaModeObj, areaMode)) {
        TAG_LOGE(AAFwkTag::APPKIT, "param areaMode err");
        EtsErrorUtil::ThrowInvalidNumParametersError(env);
        return reinterpret_cast<ani_object>(undefRef);
    }
    auto areaContext = context->CreateAreaModeContext(areaMode);
    if (areaContext == nullptr) {
        EtsErrorUtil::ThrowError(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to create areaContext");
        return reinterpret_cast<ani_object>(undefRef);
    }
    ani_class contextClass = nullptr;
    if ((status = env->FindClass(CONTEXT_CLASS_NAME, &contextClass)) != ANI_OK || contextClass == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "FindClass Context failed status: %{public}d", status);
        EtsErrorUtil::ThrowInvalidParamError(env, "FindClass failed");
        return reinterpret_cast<ani_object>(undefRef);
    }
    ani_object areaContextObj = CreateContextObject(env, contextClass, areaContext);
    if (areaContextObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null areaContextObj");
        return reinterpret_cast<ani_object>(undefRef);
    }
    return areaContextObj;
}

ani_object NativeCreateSystemHspModuleResourceManager(ani_env *env, ani_object aniObj,
    ani_string bundleNameObj, ani_string moduleNameObj)
{
    TAG_LOGE(AAFwkTag::APPKIT, "NativeCreateSystemHspModuleResourceManager");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "env is nullptr");
        return nullptr;
    }
    ani_ref undefRef = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->GetUndefined(&undefRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "GetUndefined failed %{public}d", status);
        return nullptr;
    }
    auto context = GetBaseContext(env, aniObj);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return reinterpret_cast<ani_object>(undefRef);
    }
    std::string bundleName = "";
    if (!AppExecFwk::GetStdString(env, bundleNameObj, bundleName)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse bundleName failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param bundleName failed, bundleName must be string.");
        return reinterpret_cast<ani_object>(undefRef);
    }
    std::string moduleName = "";
    if (!AppExecFwk::GetStdString(env, moduleNameObj, moduleName)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse moduleName failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param moduleName failed, moduleName must be string.");
        return reinterpret_cast<ani_object>(undefRef);
    }
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager = nullptr;
    int32_t retCode = context->CreateSystemHspModuleResourceManager(bundleName, moduleName, resourceManager);
    if (resourceManager == nullptr && retCode == ERR_ABILITY_RUNTIME_EXTERNAL_NOT_SYSTEM_HSP) {
        TAG_LOGE(AAFwkTag::APPKIT, "null resourceManager");
        EtsErrorUtil::ThrowRuntimeError(env, ERR_ABILITY_RUNTIME_EXTERNAL_NOT_SYSTEM_HSP);
        return reinterpret_cast<ani_object>(undefRef);
    }
    if (resourceManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null resourceManager, errorCode:%{public}d", retCode);
        EtsErrorUtil::ThrowRuntimeError(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return reinterpret_cast<ani_object>(undefRef);
    }
    return Global::Resource::ResMgrAddon::CreateResMgr(env, "", resourceManager, nullptr);
}

ani_object CreateContextObject(ani_env* env, ani_class contextClass, std::shared_ptr<Context> nativeContext)
{
    ani_object contextObj = nullptr;
    ani_method ctorMethod = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->Class_FindMethod(contextClass, "<ctor>", "l:", &ctorMethod)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Find ctor method failed, status: %{public}d", status);
        return nullptr;
    }
    if (ctorMethod == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ctorMethod");
        return nullptr;
    }
    std::unique_ptr<EtsBaseContext> etsContext = std::make_unique<EtsBaseContext>(nativeContext);
    auto workContext = new (std::nothrow) std::weak_ptr<Context>(etsContext->GetContext());
    if (workContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "workContext nullptr");
        return nullptr;
    }
    
    if ((status = env->Object_New(contextClass, ctorMethod, &contextObj, (ani_long)(etsContext.release())))
        != ANI_OK || contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Object_New failed, status: %{public}d", status);
        delete workContext;
        return nullptr;
    }
    CreateEtsBaseContext(env, contextClass, contextObj, nativeContext);
    ani_long nativeContextLong = (ani_long)workContext;
    if (!ContextUtil::SetNativeContextLong(env, contextObj, nativeContextLong)) {
        TAG_LOGE(AAFwkTag::APPKIT, "SetNativeContextLong failed");
        delete workContext;
        workContext = nullptr;
        return nullptr;
    }
    ani_ref *contextGlobalRef = new (std::nothrow) ani_ref;
    if (contextGlobalRef == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextGlobalRef");
        delete workContext;
        return nullptr;
    }
    if ((status = env->GlobalReference_Create(contextObj, contextGlobalRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "GlobalReference_Create failed status: %{public}d", status);
        delete contextGlobalRef;
        delete workContext;
        return nullptr;
    }
    nativeContext->Bind(contextGlobalRef);
    return contextObj;
}

ani_object GetApplicationContext(ani_env *env, const std::shared_ptr<ApplicationContext> applicationContext)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return {};
    }
    ani_object applicationContextObject =
        EtsApplicationContextUtils::CreateEtsApplicationContext(env, applicationContext);
    if (applicationContextObject == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationContextObject");
        return {};
    }
    applicationContext->SetApplicationInfoUpdateFlag(false);
    return applicationContextObject;
}

ani_object GetApplicationContextSync(ani_env *env, ani_object aniObj)
{
    auto applicationContext = Context::GetApplicationContext();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationContext");
        EtsErrorUtil::ThrowError(env, (int32_t)AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return {};
    }
    if (!applicationContext->GetApplicationInfoUpdateFlag()) {
        auto appContextObj = ApplicationContextManager::GetApplicationContextManager().GetEtsGlobalObject();
        if (appContextObj != nullptr && appContextObj->aniRef != nullptr) {
            TAG_LOGD(AAFwkTag::APPKIT, "appContextObj is not nullptr");
            return reinterpret_cast<ani_object>(appContextObj->aniRef);
        }
    }
    return GetApplicationContext(env, applicationContext);
}

void SwitchArea(ani_env *env, ani_object obj, ani_enum_item areaModeItem)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
    }
    int32_t areaMode = 0;
    if (!AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative(env, areaModeItem, areaMode)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse area failed");
        return;
    }
    auto context = GetBaseContext(env, obj);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        return;
    }
    context->SwitchArea(areaMode);
    BindContextDir(env, obj, context);
}

ani_enum_item GetArea(ani_env *env, ani_object obj)
{
    ani_enum_item areaModeItem = nullptr;
    auto context = GetBaseContext(env, obj);
    if (env == nullptr || context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "env or context is null");
        return areaModeItem;
    }
    int32_t areaMode = static_cast<int32_t>(context->GetArea());
    OHOS::AAFwk::AniEnumConvertUtil::EnumConvert_NativeToEts(env, AREA_MODE_ENUM_NAME, areaMode, areaModeItem);
    return areaModeItem;
}

ani_string GetLogFileDir(ani_env *env, ani_object obj)
{
    auto context = GetBaseContext(env, obj);
    if (env == nullptr || context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "env or context is null");
        return AppExecFwk::GetAniString(env, "");
    }
    auto logFileDir = context->GetLogFileDir();
    return AppExecFwk::GetAniString(env, logFileDir);
}

ani_boolean ContextType(ani_env *aniEnv, ani_object self, ani_object contextTypeObj)
{
    TAG_LOGD(AAFwkTag::APPKIT, "ContextType called");
    if (aniEnv == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null aniEnv");
        return ANI_FALSE;
    }

    int contextTypeValue = 0;
    if (!AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative(aniEnv, contextTypeObj, contextTypeValue)) {
        TAG_LOGE(AAFwkTag::APPKIT, "param contextType err");
        return ANI_FALSE;
    }
    TAG_LOGD(AAFwkTag::APPKIT, "ContextType contextType: %{public}d", contextTypeValue);

    static const std::unordered_map<int, std::string> contextTypeMap = {
        {0, "ApplicationContext"},
        {1, "AbilityStageContext"},
        {2, "UIAbilityContext"},
        {3, "FormExtensionContext"},
        {4, "AppServiceExtensionContext"},
        {5, "ServiceExtensionContext"},
        {6, "UIServiceExtensionContext"},
        {7, "AutoFillExtensionContext"}
    };

    auto it = contextTypeMap.find(contextTypeValue);
    if (it == contextTypeMap.end()) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid contextType: %{public}d", contextTypeValue);
        return ANI_FALSE;
    }
    const std::string &expectedType = it->second;

    ani_type type = nullptr;
    ani_status status = aniEnv->Object_GetType(self, &type);
    if (status != ANI_OK || type == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Object_GetType failed, status: %{public}d", status);
        return ANI_FALSE;
    }

    std::string actualType;
    if (!AppExecFwk::GetStaticFieldString(aniEnv, static_cast<ani_class>(type), "contextType", actualType)) {
        TAG_LOGE(AAFwkTag::APPKIT, "get context type failed");
        return ANI_FALSE;
    }

    return (actualType == expectedType) ? ANI_TRUE : ANI_FALSE;
}
}
} // namespace AbilityRuntime
} // namespace OHOS