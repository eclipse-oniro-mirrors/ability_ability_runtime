/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

#include "js_application_context_utils.h"

#include <cstdint>
#include <map>

#include "ability_business_error.h"
#include "ability_manager_client.h"
#include "ability_manager_interface.h"
#include "ability_runtime_error_util.h"
#include "application_context.h"
#include "application_info.h"
#include "application_context_manager.h"
#include "hilog_tag_wrapper.h"
#include "interop_object_instance.h"
#include "ipc_skeleton.h"
#include "js_ability_auto_startup_callback.h"
#include "js_ability_auto_startup_manager_utils.h"
#include "js_app_process_state.h"
#include "js_context_utils.h"
#include "js_data_struct_converter.h"
#include "js_error_utils.h"
#include "js_resource_manager_utils.h"
#include "js_runtime_utils.h"
#include "js_system_configuration_updated_callback.h"
#include "napi_common_want.h"
#include "tokenid_kit.h"
#ifdef SUPPORT_SCREEN
#include "ui_ability.h"
#endif

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr char APPLICATION_CONTEXT_NAME[] = "__application_context_ptr__";
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;
constexpr size_t INDEX_ZERO = 0;
constexpr size_t INDEX_ONE = 1;
constexpr size_t INDEX_TWO = 2;
constexpr int32_t ERROR_CODE_ONE = 1;
constexpr double FOUNT_SIZE = 0.0;
const char* MD_NAME = "JsApplicationContextUtils";
constexpr const int32_t API26 = 26;
constexpr const int32_t API_VERSION_MOD = 100;
}  // namespace

napi_value JsApplicationContextUtils::CreateBundleContext(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnCreateBundleContext, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnCreateBundleContext(napi_env env, NapiCallbackInfo& info)
{
    if (!CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::APPKIT, "not system-app");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_NOT_SYSTEM_APP);
        return CreateJsUndefined(env);
    }
    if (info.argc == 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "Not enough arguments");
        ThrowInvalidParamError(env, "Not enough params.");
        return CreateJsUndefined(env);
    }
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        TAG_LOGW(AAFwkTag::APPKIT, "null applicationContext");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    std::string bundleName;
    if (!ConvertFromJsValue(env, info.argv[0], bundleName)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse bundleName failed");
        ThrowInvalidParamError(env, "Parse param bundleName failed, bundleName must be string.");
        return CreateJsUndefined(env);
    }
    auto bundleContext = applicationContext->CreateBundleContext(bundleName);
    if (!bundleContext) {
        TAG_LOGE(AAFwkTag::APPKIT, "null bundleContext");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    napi_value value = CreateJsBaseContext(env, bundleContext, true);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.Context", &value, 1);
    if (systemModule == nullptr) {
        TAG_LOGW(AAFwkTag::APPKIT, "null systemModule");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    napi_value contextObj = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGE(AAFwkTag::APPKIT, "get object failed");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    auto workContext = new (std::nothrow) std::weak_ptr<Context>(bundleContext);
    napi_coerce_to_native_binding_object(env, contextObj, DetachCallbackFunc, AttachBaseContext, workContext, nullptr);
    auto res = napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGD(AAFwkTag::APPKIT, "Finalizer for weak_ptr bundle context is called");
            delete static_cast<std::weak_ptr<Context> *>(data);
        },
        nullptr, nullptr);
    if (res != napi_ok && workContext != nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "napi_wrap failed:%{public}d", res);
        delete workContext;
        return CreateJsUndefined(env);
    }
    return contextObj;
}

napi_value JsApplicationContextUtils::SwitchArea(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnSwitchArea, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnSwitchArea(napi_env env, NapiCallbackInfo& info)
{
    if (info.argc == 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "Not enough params");
        return CreateJsUndefined(env);
    }

    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        TAG_LOGW(AAFwkTag::APPKIT, "null applicationContext");
        return CreateJsUndefined(env);
    }

    int mode = 0;
    if (!ConvertFromJsValue(env, info.argv[0], mode)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse mode failed");
        return CreateJsUndefined(env);
    }

    applicationContext->SwitchArea(mode);

    napi_value object = info.thisVar;
    if (!CheckTypeForNapiValue(env, object, napi_object)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Check type failed");
        return CreateJsUndefined(env);
    }
    BindNativeProperty(env, object, "cacheDir", GetCacheDir);
    BindNativeProperty(env, object, "tempDir", GetTempDir);
    BindNativeProperty(env, object, "resourceDir", GetResourceDir);
    BindNativeProperty(env, object, "filesDir", GetFilesDir);
    BindNativeProperty(env, object, "distributedFilesDir", GetDistributedFilesDir);
    BindNativeProperty(env, object, "databaseDir", GetDatabaseDir);
    BindNativeProperty(env, object, "preferencesDir", GetPreferencesDir);
    BindNativeProperty(env, object, "bundleCodeDir", GetBundleCodeDir);
    BindNativeProperty(env, object, "cloudFileDir", GetCloudFileDir);
    BindNativeProperty(env, object, "logFileDir", GetLogFileDir);
    return CreateJsUndefined(env);
}


napi_value JsApplicationContextUtils::CreateModuleContext(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnCreateModuleContext, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnCreateModuleContext(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        TAG_LOGW(AAFwkTag::APPKIT, "null applicationContext");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    std::string moduleName;
    std::shared_ptr<Context> moduleContext = nullptr;
    if (!ConvertFromJsValue(env, info.argv[1], moduleName)) {
        TAG_LOGD(AAFwkTag::APPKIT, "Parse inner module name.");
        if (!ConvertFromJsValue(env, info.argv[0], moduleName)) {
            TAG_LOGE(AAFwkTag::APPKIT, "Parse moduleName failed");
            ThrowInvalidParamError(env, "Parse param moduleName failed, moduleName must be string.");
            return CreateJsUndefined(env);
        }
        moduleContext = applicationContext->CreateModuleContext(moduleName);
    } else {
        std::string bundleName;
        if (!ConvertFromJsValue(env, info.argv[0], bundleName)) {
            TAG_LOGE(AAFwkTag::APPKIT, "Parse bundleName failed");
            ThrowInvalidParamError(env, "Parse param bundleName failed, bundleName must be string.");
            return CreateJsUndefined(env);
        }
        if (!CheckCallerIsSystemApp()) {
            TAG_LOGE(AAFwkTag::APPKIT, "not system-app");
            AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_NOT_SYSTEM_APP);
            return CreateJsUndefined(env);
        }
        TAG_LOGI(AAFwkTag::APPKIT, "parse outer module name");
        moduleContext = applicationContext->CreateModuleContext(bundleName, moduleName);
    }

    if (!moduleContext) {
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    return CreateJsModuleContext(env, moduleContext);
}

napi_value JsApplicationContextUtils::CreateJsModuleContext(napi_env env, const std::shared_ptr<Context>& moduleContext)
{
    HandleEscape handleEscape(env);
    napi_value value = CreateJsBaseContext(env, moduleContext, true);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.Context", &value, 1);
    if (systemModule == nullptr) {
        TAG_LOGW(AAFwkTag::APPKIT, "null systemModule");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    napi_value contextObj = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGE(AAFwkTag::APPKIT, "get object failed");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    auto workContext = new (std::nothrow) std::weak_ptr<Context>(moduleContext);
    napi_coerce_to_native_binding_object(env, contextObj, DetachCallbackFunc, AttachBaseContext, workContext, nullptr);
    auto res = napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGD(AAFwkTag::APPKIT, "Finalizer for weak_ptr module context is called");
            delete static_cast<std::weak_ptr<Context> *>(data);
        },
        nullptr, nullptr);
    if (res != napi_ok && workContext != nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "napi_wrap failed:%{public}d", res);
        delete workContext;
        return CreateJsUndefined(env);
    }
    return handleEscape.Escape(contextObj);
}

napi_value JsApplicationContextUtils::CreateSystemHspModuleResourceManager(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnCreateSystemHspModuleResourceManager, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnCreateSystemHspModuleResourceManager(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        TAG_LOGW(AAFwkTag::APPKIT, "null applicationContext");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    std::string bundleName = "";
    if (!ConvertFromJsValue(env, info.argv[0], bundleName)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse bundleName failed");
        ThrowInvalidParamError(env, "Parse param bundleName failed, bundleName must be string.");
        return CreateJsUndefined(env);
    }
    std::string moduleName = "";
    if (!ConvertFromJsValue(env, info.argv[1], moduleName)) {
        TAG_LOGD(AAFwkTag::APPKIT, "Parse module name failed.");
        ThrowInvalidParamError(env, "Parse param moduleName failed, moduleName must be string.");
        return CreateJsUndefined(env);
    }

    std::shared_ptr<Global::Resource::ResourceManager> resourceManager = nullptr;
    int32_t retCode = applicationContext->CreateSystemHspModuleResourceManager(bundleName, moduleName, resourceManager);
    if (resourceManager == nullptr && retCode == ERR_ABILITY_RUNTIME_EXTERNAL_NOT_SYSTEM_HSP) {
        TAG_LOGE(AAFwkTag::APPKIT, "null resourceManager");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_NOT_SYSTEM_HSP);
        return CreateJsUndefined(env);
    }
    if (resourceManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null resourceManager");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    return CreateJsResourceManager(env, resourceManager, nullptr);
}

napi_value JsApplicationContextUtils::CreateModuleResourceManager(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnCreateModuleResourceManager, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnCreateModuleResourceManager(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        TAG_LOGW(AAFwkTag::APPKIT, "null applicationContext");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    std::string bundleName;
    if (!ConvertFromJsValue(env, info.argv[0], bundleName)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse bundleName failed");
        ThrowInvalidParamError(env, "Parse param bundleName failed, bundleName must be string.");
        return CreateJsUndefined(env);
    }
    std::string moduleName;
    if (!ConvertFromJsValue(env, info.argv[1], moduleName)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse moduleName failed");
        ThrowInvalidParamError(env, "Parse param moduleName failed, moduleName must be string.");
        return CreateJsUndefined(env);
    }
    if (!CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::APPKIT, "not system-app");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_NOT_SYSTEM_APP);
        return CreateJsUndefined(env);
    }
    auto resourceManager = applicationContext->CreateModuleResourceManager(bundleName, moduleName);
    if (resourceManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null resourceManager");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    auto jsResourceManager = CreateJsResourceManager(env, resourceManager, nullptr);
    return jsResourceManager;
}

napi_value JsApplicationContextUtils::GetArea(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnGetArea, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetArea(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        TAG_LOGW(AAFwkTag::APPKIT, "null applicationContext");
        return CreateJsUndefined(env);
    }
    int area = applicationContext->GetArea();
    return CreateJsValue(env, area);
}

napi_value JsApplicationContextUtils::GetCacheDir(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnGetCacheDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetCacheDir(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        TAG_LOGW(AAFwkTag::APPKIT, "null applicationContext");
        return CreateJsUndefined(env);
    }
    std::string path = applicationContext->GetCacheDir();
    return CreateJsValue(env, path);
}

napi_value JsApplicationContextUtils::GetTempDir(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnGetTempDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetTempDir(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        TAG_LOGW(AAFwkTag::APPKIT, "null applicationContext");
        return CreateJsUndefined(env);
    }
    std::string path = applicationContext->GetTempDir();
    return CreateJsValue(env, path);
}

napi_value JsApplicationContextUtils::GetResourceDir(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnGetResourceDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetResourceDir(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        TAG_LOGW(AAFwkTag::APPKIT, "null applicationContext");
        return CreateJsUndefined(env);
    }
    std::string path = applicationContext->GetResourceDir();
    return CreateJsValue(env, path);
}

napi_value JsApplicationContextUtils::GetFilesDir(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnGetFilesDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetFilesDir(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        TAG_LOGW(AAFwkTag::APPKIT, "null applicationContext");
        return CreateJsUndefined(env);
    }
    std::string path = applicationContext->GetFilesDir();
    return CreateJsValue(env, path);
}

napi_value JsApplicationContextUtils::GetDistributedFilesDir(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnGetDistributedFilesDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetDistributedFilesDir(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        TAG_LOGW(AAFwkTag::APPKIT, "null applicationContext");
        return CreateJsUndefined(env);
    }
    std::string path = applicationContext->GetDistributedFilesDir();
    return CreateJsValue(env, path);
}

napi_value JsApplicationContextUtils::GetCloudFileDir(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnGetCloudFileDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetCloudFileDir(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        TAG_LOGW(AAFwkTag::APPKIT, "null applicationContext");
        return CreateJsUndefined(env);
    }
    std::string path = applicationContext->GetCloudFileDir();
    return CreateJsValue(env, path);
}

napi_value JsApplicationContextUtils::GetLogFileDir(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnGetLogFileDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetLogFileDir(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        TAG_LOGW(AAFwkTag::APPKIT, "null applicationContext");
        return CreateJsUndefined(env);
    }
    std::string path = applicationContext->GetLogFileDir();
    return CreateJsValue(env, path);
}

napi_value JsApplicationContextUtils::GetProcessName(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnGetProcessName, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetProcessName(napi_env env, NapiCallbackInfo &info)
{
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "applicationContext is already released");
        return CreateJsUndefined(env);
    }
    std::string name = applicationContext->GetProcessName();
    return CreateJsValue(env, name);
}

napi_value JsApplicationContextUtils::GetDatabaseDir(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnGetDatabaseDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetDatabaseDir(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        TAG_LOGW(AAFwkTag::APPKIT, "null applicationContext");
        return CreateJsUndefined(env);
    }
    std::string path = applicationContext->GetDatabaseDir();
    return CreateJsValue(env, path);
}

napi_value JsApplicationContextUtils::GetPreferencesDir(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(
        env, info, JsApplicationContextUtils, OnGetPreferencesDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::GetGroupDir(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnGetGroupDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetPreferencesDir(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        TAG_LOGW(AAFwkTag::APPKIT, "null applicationContext");
        return CreateJsUndefined(env);
    }
    std::string path = applicationContext->GetPreferencesDir();
    return CreateJsValue(env, path);
}

napi_value JsApplicationContextUtils::OnGetGroupDir(napi_env env, NapiCallbackInfo& info)
{
    if (info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
        TAG_LOGE(AAFwkTag::APPKIT, "Not enough params");
        ThrowInvalidParamError(env, "Not enough params.");
        return CreateJsUndefined(env);
    }

    std::string groupId;
    if (!ConvertFromJsValue(env, info.argv[0], groupId)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse groupId failed");
        ThrowInvalidParamError(env, "Parse param groupId failed, groupId must be string.");
        return CreateJsUndefined(env);
    }

    TAG_LOGD(AAFwkTag::APPKIT, "Get Group Dir");
    auto innerErrCode = std::make_shared<ErrCode>(ERR_OK);
    auto path = std::make_shared<std::string>("");
    NapiAsyncTask::ExecuteCallback execute =
        [applicationContext = applicationContext_, groupId, innerErrCode, path]() {
            auto context = applicationContext.lock();
            if (!context) {
                TAG_LOGE(AAFwkTag::APPKIT, "null Context");
                *innerErrCode = ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST;
                return;
            }
            *path = context->GetGroupDir(groupId);
        };
    auto complete = [innerErrCode, path](napi_env env, NapiAsyncTask& task, int32_t status) {
        HandleScope handleScope(env);
        if (*innerErrCode == ERR_OK) {
            task.ResolveWithNoError(env, CreateJsValue(env, *path));
        } else {
            task.Reject(env, CreateJsError(env, *innerErrCode,
                "applicationContext if already released"));
        }
    };
    napi_value lastParam = (info.argc == ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsApplicationContextUtils::OnGetGroupDir",
        env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsApplicationContextUtils::RestartApp(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnRestartApp, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnRestartApp(napi_env env, NapiCallbackInfo& info)
{
    // only support one params
    if (info.argc == ARGC_ZERO) {
        TAG_LOGE(AAFwkTag::APPKIT, "Not enough params");
        ThrowInvalidParamError(env, "Not enough params");
        return CreateJsUndefined(env);
    }
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        TAG_LOGW(AAFwkTag::APPKIT, "null applicationContext");
        return CreateJsUndefined(env);
    }
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse want failed");
        ThrowInvalidParamError(env, "Parse param want failed, want must be Want.");
        return CreateJsUndefined(env);
    }

    auto errCode = applicationContext->RestartApp(want);
    if (errCode == ERR_OK) {
        return CreateJsUndefined(env);
    }
    if (errCode == ERR_INVALID_VALUE) {
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    } else if (errCode == AAFwk::ERR_RESTART_APP_INCORRECT_ABILITY) {
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_RESTART_APP_INCORRECT_ABILITY);
    } else if (errCode == AAFwk::ERR_RESTART_APP_FREQUENT) {
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_RESTART_APP_FREQUENT);
    } else if (errCode == AAFwk::NOT_TOP_ABILITY) {
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_NOT_TOP_ABILITY);
    } else {
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR);
    }
    TAG_LOGE(AAFwkTag::APPKIT, "errCode:%{public}d.", errCode);
    return CreateJsUndefined(env);
}

napi_value JsApplicationContextUtils::GetBundleCodeDir(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(
        env, info, JsApplicationContextUtils, OnGetBundleCodeDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetBundleCodeDir(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        TAG_LOGW(AAFwkTag::APPKIT, "null applicationContext");
        return CreateJsUndefined(env);
    }
    std::string path = applicationContext->GetBundleCodeDir();
    return CreateJsValue(env, path);
}

napi_value JsApplicationContextUtils::KillProcessBySelf(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnKillProcessBySelf, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnKillProcessBySelf(napi_env env, NapiCallbackInfo& info)
{
    // only support 0 or 1 or 2 params
    if (info.argc != ARGC_ZERO && info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
        TAG_LOGE(AAFwkTag::APPKIT, "Not enough params");
        ThrowInvalidParamError(env, "Not enough params.");
        return CreateJsUndefined(env);
    }

    bool clearPageStack = true;
    bool hasClearPageStack = false;
    if (info.argc > ARGC_ZERO && ConvertFromJsValue(env, info.argv[0], clearPageStack)) {
        hasClearPageStack = true;
    }

    TAG_LOGD(AAFwkTag::APPKIT, "kill self process");
    auto innerErrCode = std::make_shared<ErrCode>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute =
        [applicationContext = applicationContext_, clearPageStack, innerErrCode]() {
        auto context = applicationContext.lock();
        if (!context) {
            TAG_LOGE(AAFwkTag::APPKIT, "null context");
            *innerErrCode = ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST;
            return;
        }
        context->KillProcessBySelf(clearPageStack);
    };
    NapiAsyncTask::CompleteCallback complete = [innerErrCode](napi_env env, NapiAsyncTask& task, int32_t status) {
        HandleScope handleScope(env);
        if (*innerErrCode != ERR_OK) {
            task.Reject(env, CreateJsError(env, *innerErrCode, "applicationContext is already released."));
            return;
        }
        task.ResolveWithNoError(env, CreateJsUndefined(env));
    };
    napi_value lastParam = (info.argc == ARGC_ONE && !hasClearPageStack) ? info.argv[INDEX_ZERO] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsApplicationContextUtils::OnkillProcessBySelf",
        env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsApplicationContextUtils::SetColorMode(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnSetColorMode, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnSetColorMode(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    // only support one params
    if (info.argc == ARGC_ZERO) {
        TAG_LOGE(AAFwkTag::APPKIT, "Not enough params");
        ThrowInvalidParamError(env, "Not enough params.");
        return CreateJsUndefined(env);
    }
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGW(AAFwkTag::APPKIT, "null applicationContext");
        return CreateJsUndefined(env);
    }

    int32_t colorMode = 0;
    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], colorMode)) {
        ThrowInvalidParamError(env, "Parse param colorMode failed, colorMode must be number.");
        TAG_LOGE(AAFwkTag::APPKIT, "Parse colorMode failed");
        return CreateJsUndefined(env);
    }
    applicationContext->SetColorMode(colorMode);
    return CreateJsUndefined(env);
}

napi_value JsApplicationContextUtils::SetLanguage(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnSetLanguage, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnSetLanguage(napi_env env, NapiCallbackInfo& info)
{
    // only support one params
    if (info.argc == ARGC_ZERO) {
        TAG_LOGE(AAFwkTag::APPKIT, "Not enough params");
        ThrowInvalidParamError(env, "Not enough params.");
        return CreateJsUndefined(env);
    }
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        TAG_LOGW(AAFwkTag::APPKIT, "null applicationContext");
        return CreateJsUndefined(env);
    }
    std::string language;
    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], language)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse language failed");
        ThrowInvalidParamError(env, "Parse param language failed, language must be string.");
        return CreateJsUndefined(env);
    }
    applicationContext->SetLanguage(language);
    return CreateJsUndefined(env);
}

napi_value JsApplicationContextUtils::SetFontSizeScale(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnSetFontSizeScale, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnSetFontSizeScale(napi_env env, NapiCallbackInfo& info)
{
    if (info.argc == ARGC_ZERO) {
        TAG_LOGE(AAFwkTag::APPKIT, "Not enough params");
        ThrowInvalidParamError(env, "Not enough params.");
        return CreateJsUndefined(env);
    }

    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationContext");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return CreateJsUndefined(env);
    }

    double fontSizeScale = 1;
    if (!ConvertFromJsNumber(env, info.argv[INDEX_ZERO], fontSizeScale)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse fontSizeScale failed");
        ThrowInvalidParamError(env, "Parse fontSizeScale failed, fontSizeScale must be number.");
        return CreateJsUndefined(env);
    }
    TAG_LOGD(AAFwkTag::APPKIT, "fontSizeScale: %{public}f", fontSizeScale);
    if (fontSizeScale < FOUNT_SIZE) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid size");
        ThrowInvalidParamError(env, "Invalid font size.");
        return CreateJsUndefined(env);
    }

    applicationContext->SetFontSizeScale(fontSizeScale);
    return CreateJsUndefined(env);
}

napi_value JsApplicationContextUtils::SetFont(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnSetFont, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnSetFont(napi_env env, NapiCallbackInfo& info)
{
    // only support one params
    if (info.argc == ARGC_ZERO) {
        TAG_LOGE(AAFwkTag::APPKIT, "Not enough params");
        ThrowInvalidParamError(env, "Not enough params.");
        return CreateJsUndefined(env);
    }
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        TAG_LOGW(AAFwkTag::APPKIT, "null applicationContext");
        return CreateJsUndefined(env);
    }
    std::string font;
    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], font)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse font failed");
        ThrowInvalidParamError(env, "Parse param font failed, font must be string.");
        return CreateJsUndefined(env);
    }
    applicationContext->SetFont(font);
    return CreateJsUndefined(env);
}

napi_value JsApplicationContextUtils::PreloadUIExtensionAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(
        env, info, JsApplicationContextUtils, OnPreloadUIExtensionAbility, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnPreloadUIExtensionAbility(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (info.argc < ARGC_ONE) {
        TAG_LOGW(AAFwkTag::APPKIT, "params error");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want)) {
        TAG_LOGW(AAFwkTag::APPKIT, "Parse want failed");
        ThrowInvalidParamError(env,
            "Parse param want failed, want must be Want.");
        return CreateJsUndefined(env);
    }

    auto innerErrCode = std::make_shared<ErrCode>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute = [applicationContext = applicationContext_, want, innerErrCode]() {
        auto context = applicationContext.lock();
        if (!context) {
            TAG_LOGE(AAFwkTag::APPKIT, "null context");
            *innerErrCode = static_cast<int>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
            return;
        }
        auto hostBundleName = context->GetBundleName();
        TAG_LOGD(AAFwkTag::APPKIT, "HostBundleName is %{public}s", hostBundleName.c_str());
        *innerErrCode = AAFwk::AbilityManagerClient::GetInstance()->PreloadUIExtensionAbility(want, hostBundleName);
    };
    NapiAsyncTask::CompleteCallback complete = [innerErrCode](napi_env env, NapiAsyncTask& task, int32_t status) {
        HandleScope handleScope(env);
        if (*innerErrCode == ERR_OK) {
            task.Resolve(env, CreateJsUndefined(env));
        } else {
            TAG_LOGE(AAFwkTag::APPKIT, "OnPreloadUIExtensionAbility failed %{public}d", *innerErrCode);
            task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrCode));
        }
    };
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsApplicationContextUtils::OnPreloadUIExtensionAbility",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsApplicationContextUtils::ClearUpApplicationData(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(
        env, info, JsApplicationContextUtils, OnClearUpApplicationData, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnClearUpApplicationData(napi_env env, NapiCallbackInfo &info)
{
    // only support 0 or 1 params
    if (info.argc != ARGC_ZERO && info.argc != ARGC_ONE) {
        TAG_LOGE(AAFwkTag::APPKIT, "Not enough params");
        ThrowInvalidParamError(env, "Not enough params.");
        return CreateJsUndefined(env);
    }
    auto innerErrCode = std::make_shared<ErrCode>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute = [applicationContext = applicationContext_, innerErrCode]() {
        auto context = applicationContext.lock();
        if (!context) {
            TAG_LOGE(AAFwkTag::APPKIT, "null context");
            *innerErrCode = ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST;
            return;
        }
        context->ClearUpApplicationData();
    };
    NapiAsyncTask::CompleteCallback complete =
        [applicationContext = applicationContext_, innerErrCode](napi_env env, NapiAsyncTask& task, int32_t status) {
            HandleScope handleScope(env);
            if (*innerErrCode == ERR_OK) {
                task.ResolveWithNoError(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsError(env, *innerErrCode, "applicationContext if already released."));
            }
    };
    napi_value lastParam = (info.argc == ARGC_ONE) ? info.argv[INDEX_ZERO] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsApplicationContextUtils::OnClearUpApplicationData",
        env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsApplicationContextUtils::GetRunningProcessInformation(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnGetRunningProcessInformation, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetRunningProcessInformation(napi_env env, NapiCallbackInfo& info)
{
    // only support 0 or 1 params
    if (info.argc != ARGC_ZERO && info.argc != ARGC_ONE) {
        TAG_LOGE(AAFwkTag::APPKIT, "Not enough params");
        ThrowInvalidParamError(env, "Not enough params.");
        return CreateJsUndefined(env);
    }
    TAG_LOGD(AAFwkTag::APPKIT, "Get Process Info");
    auto innerErrCode = std::make_shared<ErrCode>(ERR_OK);
    auto processInfo = std::make_shared<AppExecFwk::RunningProcessInfo>();
    NapiAsyncTask::ExecuteCallback execute = [applicationContext = applicationContext_, innerErrCode, processInfo]() {
        auto context = applicationContext.lock();
        if (!context) {
            TAG_LOGE(AAFwkTag::APPKIT, "null context");
            *innerErrCode = ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST;
            return;
        }
        *innerErrCode = context->GetProcessRunningInformation(*processInfo);
    };
    auto complete = [innerErrCode, processInfo](napi_env env, NapiAsyncTask& task, int32_t status) {
        HandleScope handleScope(env);
        if (*innerErrCode == ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST) {
            task.Reject(env, CreateJsError(env, *innerErrCode,
                "applicationContext if already released."));
            return;
        }
        if (*innerErrCode == ERR_OK) {
            napi_value object = nullptr;
            napi_create_object(env, &object);
            napi_set_named_property(env, object, "processName", CreateJsValue(env, processInfo->processName_));
            napi_set_named_property(env, object, "pid", CreateJsValue(env, processInfo->pid_));
            napi_set_named_property(env, object, "uid", CreateJsValue(env, processInfo->uid_));
            napi_set_named_property(env, object, "bundleNames", CreateNativeArray(env, processInfo->bundleNames));
            napi_set_named_property(env, object,
                "state", CreateJsValue(env, ConvertToJsAppProcessState(processInfo->state_, processInfo->isFocused)));
            napi_set_named_property(env, object, "bundleType", CreateJsValue(env, processInfo->bundleType));
            if (processInfo->appCloneIndex != -1) {
                napi_set_named_property(env, object, "appCloneIndex", CreateJsValue(env, processInfo->appCloneIndex));
            }
            napi_set_named_property(env, object, "isPreload", CreateJsValue(env, processInfo->isPreload));
            napi_value array = nullptr;
            napi_create_array_with_length(env, 1, &array);
            if (array == nullptr) {
                TAG_LOGE(AAFwkTag::APPKIT, "null array");
                task.Reject(env, CreateJsError(env, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR,
                    "Initiate array failed."));
            } else {
                napi_set_element(env, array, 0, object);
                task.ResolveWithNoError(env, array);
            }
        } else {
            task.Reject(env, CreateJsError(env, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR,
                "Get process infos failed."));
        }
    };
    napi_value lastParam = (info.argc == ARGC_ONE) ? info.argv[INDEX_ZERO] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsApplicationContextUtils::OnGetRunningProcessInformation",
        env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsApplicationContextUtils::GetAllWindowStages(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnGetAllWindowStages, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetAllWindowStages(napi_env env, NapiCallbackInfo& info)
{
#ifdef SUPPORT_SCREEN
    auto uiAbility = std::make_shared<std::vector<std::shared_ptr<UIAbility>>>();

    NapiAsyncTask::ExecuteCallback execute = [applicationContext = applicationContext_, uiAbility]() {
        auto context = applicationContext.lock();
        if (context == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null context");
            return;
        }
        context->GetAllUIAbilities(*uiAbility);
    };
    auto complete = [uiAbility](napi_env env, NapiAsyncTask& task, int32_t status) {
        HandleScope handleScope(env);
        napi_value array = nullptr;
        napi_create_array(env, &array);
        if (array == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null array");
            return;
        }
        uint32_t index = 0;
        for (const auto &item : *uiAbility) {
            if (item == nullptr) {
                continue;
            }
            napi_value windowStage = item->GetWindowStage();
            if (windowStage == nullptr) {
                continue;
            }
            if (napi_set_element(env, array, index, windowStage) != napi_ok) {
                TAG_LOGE(AAFwkTag::APPKIT, "Failed to set element at index %{public}d", index);
                continue;
            }
            index++;
        }
        task.ResolveWithNoError(env, array);
    };
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsApplicationContextUtils::OnGetAllWindowStages", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
#else
    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    napi_create_promise(env, &deferred, &promise);
    napi_value emptyArray = nullptr;
    napi_create_array(env, &emptyArray);
    napi_resolve_deferred(env, deferred, emptyArray);
    return promise;
#endif
}

napi_value JsApplicationContextUtils::GetCurrentAppCloneIndex(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnGetCurrentAppCloneIndex, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetCurrentAppCloneIndex(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Get App Index");
    auto context = applicationContext_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return CreateJsUndefined(env);
    }
    if (context->GetCurrentAppMode() != static_cast<int32_t>(AppExecFwk::MultiAppModeType::APP_CLONE)) {
        ThrowError(env, AbilityErrorCode::ERROR_NOT_APP_CLONE);
        return CreateJsUndefined(env);
    }
    int32_t appIndex = context->GetCurrentAppCloneIndex();
    return CreateJsValue(env, appIndex);
}

napi_value JsApplicationContextUtils::GetCurrentInstanceKey(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnGetCurrentInstanceKey, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetCurrentInstanceKey(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Get current instance key");
    auto context = applicationContext_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return CreateJsUndefined(env);
    }
    if (context->GetCurrentAppMode() != static_cast<int32_t>(AppExecFwk::MultiAppModeType::MULTI_INSTANCE)) {
        ThrowError(env, AbilityErrorCode::ERROR_MULTI_INSTANCE_NOT_SUPPORTED);
        return CreateJsUndefined(env);
    }
    std::string instanceKey = context->GetCurrentInstanceKey();
    return CreateJsValue(env, instanceKey);
}

napi_value JsApplicationContextUtils::GetAllRunningInstanceKeys(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnGetAllRunningInstanceKeys, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetAllRunningInstanceKeys(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Get all running instance keys");
    auto innerErrCode = std::make_shared<ErrCode>(ERR_OK);
    std::shared_ptr<std::vector<std::string>> instanceKeys = std::make_shared<std::vector<std::string>>();
    NapiAsyncTask::ExecuteCallback execute =
        [applicationContext = applicationContext_, innerErrCode, instanceKeys]() {
        auto context = applicationContext.lock();
        if (!context) {
            TAG_LOGE(AAFwkTag::APPKIT, "null context");
            *innerErrCode = ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST;
            return;
        }
        if (context->GetCurrentAppMode() != static_cast<int32_t>(AppExecFwk::MultiAppModeType::MULTI_INSTANCE)) {
            TAG_LOGE(AAFwkTag::APPKIT, "not supported");
            *innerErrCode = static_cast<int>(AbilityErrorCode::ERROR_MULTI_INSTANCE_NOT_SUPPORTED);
            return;
        }
        *innerErrCode = context->GetAllRunningInstanceKeys(*instanceKeys);
    };
    auto complete = [applicationContext = applicationContext_, innerErrCode, instanceKeys](
        napi_env env, NapiAsyncTask& task, int32_t status) {
        HandleScope handleScope(env);
        if (*innerErrCode == ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST ||
            *innerErrCode == static_cast<int>(AbilityErrorCode::ERROR_MULTI_INSTANCE_NOT_SUPPORTED)) {
            TAG_LOGE(AAFwkTag::APPKIT, "innerErrCode=%{public}d", *innerErrCode);
            task.Reject(env, CreateJsError(env, *innerErrCode, GetErrorMsg(AbilityErrorCode(*innerErrCode))));
            return;
        }
        if (*innerErrCode != ERR_OK) {
            TAG_LOGE(AAFwkTag::APPKIT, "innerErrCode=%{public}d", *innerErrCode);
            task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrCode));
            return;
        }
        task.ResolveWithNoError(env, CreateNativeArray(env, *instanceKeys));
    };

    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsApplicationContextUtils::OnGetAllRunningInstanceKeys",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsApplicationContextUtils::CreateAreaModeContext(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnCreateAreaModeContext, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnCreateAreaModeContext(napi_env env, NapiCallbackInfo &info)
{
    if (info.argc == ARGC_ZERO) {
        TAG_LOGE(AAFwkTag::APPKIT, "not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        TAG_LOGE(AAFwkTag::APPKIT, "applicationContext is already released");
        return CreateJsUndefined(env);
    }

    int areaMode = 0;
    if (!ConvertFromJsValue(env, info.argv[0], areaMode)) {
        TAG_LOGE(AAFwkTag::APPKIT, "parse areaMode failed");
        ThrowInvalidParamError(env, "parse param areaMode failed, areaMode must be number.");
        return CreateJsUndefined(env);
    }

    auto areaContext = applicationContext->CreateAreaModeContext(areaMode);
    if (areaContext == nullptr) {
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        TAG_LOGE(AAFwkTag::APPKIT, "failed to create areaContext");
        return CreateJsUndefined(env);
    }

    return CreateJsContext(env, areaContext);
}

napi_value JsApplicationContextUtils::CreateDisplayContext(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnCreateDisplayContext, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnCreateDisplayContext(napi_env env, NapiCallbackInfo &info)
{
#ifdef SUPPORT_GRAPHICS
    if (info.argc == ARGC_ZERO) {
        TAG_LOGE(AAFwkTag::APPKIT, "not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        TAG_LOGE(AAFwkTag::APPKIT, "applicationContext is already released");
        return CreateJsUndefined(env);
    }

    int64_t displayId = -1;
    if (!ConvertFromJsValue(env, info.argv[0], displayId)) {
        TAG_LOGE(AAFwkTag::APPKIT, "parse displayId failed");
        ThrowInvalidParamError(env, "parse param displayId failed, displayId must be number.");
        return CreateJsUndefined(env);
    }
    if (displayId < 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "displayId is invalid, less than 0");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    uint64_t validDisplayId = static_cast<uint64_t>(displayId);

    auto context = applicationContext->CreateDisplayContext(validDisplayId);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to create context");
        return CreateJsUndefined(env);
    }
    return CreateJsContext(env, context);
#else
    return CreateJsUndefined(env);
#endif
}

napi_value JsApplicationContextUtils::CreateJsContext(napi_env env, const std::shared_ptr<Context> &context)
{
    HandleEscape handleEscape(env);
    napi_value value = CreateJsBaseContext(env, context, true);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.Context", &value, 1);
    if (systemModule == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid systemModule");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    napi_value contextObj = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to get context native object");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    auto workContext = new (std::nothrow) std::weak_ptr<Context>(context);
    auto status = napi_coerce_to_native_binding_object(env, contextObj, DetachCallbackFunc, AttachBaseContext,
        workContext, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::APPKIT, "coerce context failed: %{public}d", status);
        delete workContext;
        return CreateJsUndefined(env);
    }
    auto res = napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGD(AAFwkTag::APPKIT, "Finalizer for weak_ptr module context is called");
            delete static_cast<std::weak_ptr<Context> *>(data);
        },
        nullptr, nullptr);
    if (res != napi_ok && workContext != nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "napi_wrap failed:%{public}d", res);
        delete workContext;
        return CreateJsUndefined(env);
    }
    return handleEscape.Escape(contextObj);
}

void JsApplicationContextUtils::Finalizer(napi_env env, void *data, void *hint)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    std::unique_ptr<JsApplicationContextUtils>(static_cast<JsApplicationContextUtils *>(data));
}

napi_value JsApplicationContextUtils::RegisterAbilityLifecycleCallback(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnRegisterAbilityLifecycleCallback, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::UnregisterAbilityLifecycleCallback(
    napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnUnregisterAbilityLifecycleCallback, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnRegisterAbilityLifecycleCallback(
    napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    // only support one params
    if (info.argc != ARGC_ONE) {
        TAG_LOGE(AAFwkTag::APPKIT, "Not enough params");
        return CreateJsUndefined(env);
    }

    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationContext");
        return CreateJsUndefined(env);
    }
    if (applicationContext->GetApplicationInfo()->apiTargetVersion % API_VERSION_MOD >= API26) {
        TAG_LOGE(AAFwkTag::APPKIT, "reject registerAbilityLifecycleCallback");
        return CreateJsUndefined(env);
    }
    if (callback_ != nullptr) {
        TAG_LOGD(AAFwkTag::APPKIT, "callback_ is not nullptr");
        return CreateJsValue(env, callback_->Register(info.argv[0]));
    }
    callback_ = std::make_shared<JsAbilityLifecycleCallback>(env);
    int32_t callbackId = callback_->Register(info.argv[INDEX_ZERO]);
    applicationContext->RegisterAbilityLifecycleCallback(callback_);
    return CreateJsValue(env, callbackId);
}

napi_value JsApplicationContextUtils::OnUnregisterAbilityLifecycleCallback(
    napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    int32_t errCode = 0;
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ApplicationContext");
        errCode = ERROR_CODE_ONE;
    }
    if (applicationContext->GetApplicationInfo()->apiTargetVersion % API_VERSION_MOD >= API26) {
        TAG_LOGE(AAFwkTag::APPKIT, "reject unregisterAbilityLifecycleCallback");
        return CreateJsUndefined(env);
    }
    int32_t callbackId = -1;
    if (info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
        TAG_LOGE(AAFwkTag::APPKIT, "not enough params");
        errCode = ERROR_CODE_ONE;
    } else {
        napi_get_value_int32(env, info.argv[INDEX_ZERO], &callbackId);
        TAG_LOGD(AAFwkTag::APPKIT, "callbackId is %{public}d.", callbackId);
    }
    std::weak_ptr<JsAbilityLifecycleCallback> callbackWeak(callback_);

    NapiAsyncTask::CompleteCallback complete = [callbackWeak, callbackId, errCode](
            napi_env env, NapiAsyncTask &task, int32_t status) {
            HandleScope handleScope(env);
            if (errCode != 0) {
                task.Reject(env, CreateJsError(env, errCode, "Invalidate params."));
                return;
            }
            auto callback = callbackWeak.lock();
            if (callback == nullptr) {
                TAG_LOGE(AAFwkTag::APPKIT, "null callback");
                task.Reject(env, CreateJsError(env, ERROR_CODE_ONE, "callback is nullptr"));
                return;
            }

            TAG_LOGD(AAFwkTag::APPKIT, "OnUnregisterAbilityLifecycleCallback begin");
            if (!callback->UnRegister(callbackId)) {
                TAG_LOGE(AAFwkTag::APPKIT, "call UnRegister failed");
                task.Reject(env, CreateJsError(env, ERROR_CODE_ONE, "call UnRegister failed!"));
                return;
            }

            task.Resolve(env, CreateJsUndefined(env));
        };
    napi_value lastParam = (info.argc <= ARGC_ONE) ? nullptr : info.argv[INDEX_ONE];
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsApplicationContextUtils::OnUnregisterAbilityLifecycleCallback", env,
        CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsApplicationContextUtils::RegisterEnvironmentCallback(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnRegisterEnvironmentCallback, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::UnregisterEnvironmentCallback(
    napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnUnregisterEnvironmentCallback, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnRegisterEnvironmentCallback(
    napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    // only support one params
    if (info.argc != ARGC_ONE) {
        TAG_LOGE(AAFwkTag::APPKIT, "Not enough params");
        return CreateJsUndefined(env);
    }

    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationContext");
        return CreateJsUndefined(env);
    }
    if (applicationContext->GetApplicationInfo()->apiTargetVersion % API_VERSION_MOD >= API26) {
        TAG_LOGE(AAFwkTag::APPKIT, "reject registerEnvironmentCallback");
        return CreateJsUndefined(env);
    }
    if (envCallback_ != nullptr) {
        TAG_LOGD(AAFwkTag::APPKIT, "envCallback_ is not nullptr");
        return CreateJsValue(env, envCallback_->Register(info.argv[0]));
    }
    envCallback_ = std::make_shared<JsEnvironmentCallback>(env);
    int32_t callbackId = envCallback_->Register(info.argv[INDEX_ZERO]);
    applicationContext->RegisterEnvironmentCallback(envCallback_);
    return CreateJsValue(env, callbackId);
}

napi_value JsApplicationContextUtils::OnUnregisterEnvironmentCallback(
    napi_env env, NapiCallbackInfo& info)
{
    int32_t errCode = 0;
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationContext");
        errCode = ERROR_CODE_ONE;
    }
    if (applicationContext->GetApplicationInfo()->apiTargetVersion % API_VERSION_MOD >= API26) {
        TAG_LOGE(AAFwkTag::APPKIT, "reject unregisterEnvironmentCallback");
        return CreateJsUndefined(env);
    }
    int32_t callbackId = -1;
    if (info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
        TAG_LOGE(AAFwkTag::APPKIT, "Not enough params");
        errCode = ERROR_CODE_ONE;
    } else {
        napi_get_value_int32(env, info.argv[INDEX_ZERO], &callbackId);
        TAG_LOGD(AAFwkTag::APPKIT, "callbackId is %{public}d", callbackId);
    }
    std::weak_ptr<JsEnvironmentCallback> envCallbackWeak(envCallback_);
    NapiAsyncTask::CompleteCallback complete = [envCallbackWeak, callbackId, errCode](
            napi_env env, NapiAsyncTask &task, int32_t status) {
            HandleScope handleScope(env);
            if (errCode != 0) {
                task.Reject(env, CreateJsError(env, errCode, "Invalidate params."));
                return;
            }
            auto env_callback = envCallbackWeak.lock();
            if (env_callback == nullptr) {
                TAG_LOGE(AAFwkTag::APPKIT, "null env_callback");
                task.Reject(env, CreateJsError(env, ERROR_CODE_ONE, "env_callback is nullptr"));
                return;
            }

            if (!env_callback->UnRegister(callbackId)) {
                TAG_LOGE(AAFwkTag::APPKIT, "call UnRegister failed");
                task.Reject(env, CreateJsError(env, ERROR_CODE_ONE, "call UnRegister failed!"));
                return;
            }

            task.Resolve(env, CreateJsUndefined(env));
        };
    napi_value lastParam = (info.argc <= ARGC_ONE) ? nullptr : info.argv[INDEX_ONE];
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsApplicationContextUtils::OnUnregisterEnvironmentCallback", env,
        CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsApplicationContextUtils::On(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnOn, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::Off(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnOff, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::RegisterSystemConfigurationUpdated(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnSystemConfigurationUpdated,
        APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::UnRegisterSystemConfigurationUpdated(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OffSystemConfigurationUpdated,
        APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnOn(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");

    if (info.argc != ARGC_TWO) {
        TAG_LOGE(AAFwkTag::APPKIT, "Not enough params");
        ThrowInvalidParamError(env, "Not enough params.");
        return CreateJsUndefined(env);
    }

    if (!CheckTypeForNapiValue(env, info.argv[0], napi_string)) {
        TAG_LOGE(AAFwkTag::APPKIT, "param invalid");
        ThrowInvalidParamError(env, "Parse param type failed, type must be string.");
        return CreateJsUndefined(env);
    }
    std::string type;
    if (!ConvertFromJsValue(env, info.argv[0], type)) {
        TAG_LOGE(AAFwkTag::APPKIT, "convert type failed");
        ThrowInvalidParamError(env,
            "Parse param type failed, type must be string.");
        return CreateJsUndefined(env);
    }

    if (type == "abilityLifecycle") {
        return OnOnAbilityLifecycle(env, info, false);
    }
    if (type == "abilityLifecycleEvent") {
        return OnOnAbilityLifecycle(env, info, true);
    }
    if (type == "environment") {
        return OnOnEnvironment(env, info, false);
    }
    if (type == "environmentEvent") {
        return OnOnEnvironment(env, info, true);
    }
    if (type == "applicationStateChange") {
        return OnOnApplicationStateChange(env, info);
    }
    if (type == "interopAbilityLifecycle") {
        return OnOnInteropAbilityLifecycle(env, info);
    }
    TAG_LOGE(AAFwkTag::APPKIT, "on function type not match");
    ThrowInvalidParamError(env, "Parse param callback failed, callback must be function.");
    return CreateJsUndefined(env);
}

napi_value JsApplicationContextUtils::OnSystemConfigurationUpdated(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::APPKIT, "OnSystemConfigurationUpdated called");

    if (info.argc != ARGC_ONE) {
        TAG_LOGE(AAFwkTag::APPKIT, "Not enough params");
        ThrowInvalidParamError(env, "Not enough params.");
        return CreateJsUndefined(env);
    }

    if (!CheckTypeForNapiValue(env, info.argv[0], napi_object)) {
        TAG_LOGE(AAFwkTag::APPKIT, "param invalid");
        ThrowInvalidParamError(env, "Parse param type failed, type must be object.");
        return CreateJsUndefined(env);
    }

    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationContext");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    std::lock_guard<std::mutex> lock(systemConfigurationUpdatedCallbackLock_);
    if (systemConfigurationUpdatedCallback_ != nullptr) {
        systemConfigurationUpdatedCallback_->Register(info.argv[INDEX_ZERO]);
        return CreateJsUndefined(env);
    }

    systemConfigurationUpdatedCallback_ = std::make_shared<JsSystemConfigurationUpdatedCallback>(env);
    systemConfigurationUpdatedCallback_->Register(info.argv[INDEX_ZERO]);
    applicationContext->RegisterSystemConfigurationUpdatedCallback(systemConfigurationUpdatedCallback_);
    return CreateJsUndefined(env);
}

napi_value JsApplicationContextUtils::OffSystemConfigurationUpdated(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");

    if (info.argc != ARGC_ONE && info.argc != ARGC_ZERO) {
        TAG_LOGE(AAFwkTag::APPKIT, "Not enough params");
        ThrowInvalidParamError(env, "Not enough params.");
        return CreateJsUndefined(env);
    }

    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationContext");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    std::lock_guard<std::mutex> lock(systemConfigurationUpdatedCallbackLock_);
    if (systemConfigurationUpdatedCallback_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null systemConfigurationUpdatedCallback_");
        ThrowInvalidParamError(env, "parameter invalid.");
        return CreateJsUndefined(env);
    }

    if (info.argc == ARGC_ONE && !CheckTypeForNapiValue(env, info.argv[ARGC_ZERO], napi_object)) {
        systemConfigurationUpdatedCallback_->UnRegister();
    } else if (!systemConfigurationUpdatedCallback_->UnRegister(info.argv[ARGC_ZERO])) {
        TAG_LOGE(AAFwkTag::APPKIT, "UnRegister failed");
        ThrowInvalidParamError(env, "parameter invalid");
        return CreateJsUndefined(env);
    }

    if (systemConfigurationUpdatedCallback_->IsEmpty()) {
        systemConfigurationUpdatedCallback_.reset();
    }
    return CreateJsUndefined(env);
}

napi_value JsApplicationContextUtils::OnOff(napi_env env, NapiCallbackInfo& info)
{
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::APPKIT, "Not enough params");
        ThrowInvalidParamError(env, "Not enough params.");
        return CreateJsUndefined(env);
    }

    if (!CheckTypeForNapiValue(env, info.argv[0], napi_string)) {
        TAG_LOGE(AAFwkTag::APPKIT, "param invalid");
        ThrowInvalidParamError(env, "Parse param type failed, type must be string.");
        return CreateJsUndefined(env);
    }
    std::string type;
    if (!ConvertFromJsValue(env, info.argv[0], type)) {
        TAG_LOGE(AAFwkTag::APPKIT, "convert type failed");
        ThrowInvalidParamError(env,
            "Parse param type failed, type must be string.");
        return CreateJsUndefined(env);
    }

    if (type == "applicationStateChange") {
        return OnOffApplicationStateChange(env, info);
    }

    if (info.argc != ARGC_TWO && info.argc != ARGC_THREE) {
        TAG_LOGE(AAFwkTag::APPKIT, "Not enough params");
        ThrowInvalidParamError(env, "Not enough params.");
        return CreateJsUndefined(env);
    }

    int32_t callbackId = -1;
    if (CheckTypeForNapiValue(env, info.argv[1], napi_number)) {
        napi_get_value_int32(env, info.argv[1], &callbackId);
        TAG_LOGD(AAFwkTag::APPKIT, "callbackId is %{public}d.", callbackId);
    }

    if (type == "abilityLifecycle") {
        return OnOffAbilityLifecycle(env, info, callbackId);
    }
    if (type == "abilityLifecycleEvent") {
        return OnOffAbilityLifecycleEventSync(env, info, callbackId);
    }
    if (type == "environment") {
        return OnOffEnvironment(env, info, callbackId);
    }
    if (type == "environmentEvent") {
        return OnOffEnvironmentEventSync(env, info, callbackId);
    }
    if (type == "interopAbilityLifecycle") {
        return OnOffInteropAbilityLifecycle(env, info);
    }
    TAG_LOGE(AAFwkTag::APPKIT, "off function type not match");
    ThrowInvalidParamError(env, "Parse param callback failed, callback must be function.");
    return CreateJsUndefined(env);
}

napi_value JsApplicationContextUtils::OnOnAbilityLifecycle(
    napi_env env, NapiCallbackInfo& info, bool isSync)
{
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationContext");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    if (callback_ != nullptr) {
        TAG_LOGD(AAFwkTag::APPKIT, "callback_ is not nullptr");
        return CreateJsValue(env, callback_->Register(info.argv[1], isSync));
    }
    callback_ = std::make_shared<JsAbilityLifecycleCallback>(env);
    int32_t callbackId = callback_->Register(info.argv[1], isSync);
    applicationContext->RegisterAbilityLifecycleCallback(callback_);
    return CreateJsValue(env, callbackId);
}

napi_value JsApplicationContextUtils::OnOffAbilityLifecycle(
    napi_env env, NapiCallbackInfo& info, int32_t callbackId)
{
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationContext");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    std::weak_ptr<JsAbilityLifecycleCallback> callbackWeak(callback_);
    NapiAsyncTask::CompleteCallback complete = [callbackWeak, callbackId](
            napi_env env, NapiAsyncTask &task, int32_t status) {
            HandleScope handleScope(env);
            auto callback = callbackWeak.lock();
            if (callback == nullptr) {
                TAG_LOGE(AAFwkTag::APPKIT, "null callback");
                task.Reject(env, CreateJsError(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER,
                    "callback is nullptr"));
                return;
            }

            if (!callback->UnRegister(callbackId, false)) {
                TAG_LOGE(AAFwkTag::APPKIT, "call UnRegister failed");
                task.Reject(env, CreateJsError(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER,
                    "call UnRegister failed!"));
                return;
            }

            task.ResolveWithNoError(env, CreateJsUndefined(env));
        };
    napi_value lastParam = (info.argc <= ARGC_TWO) ? nullptr : info.argv[INDEX_TWO];
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsApplicationContextUtils::OnOffAbilityLifecycle", env,
        CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsApplicationContextUtils::OnOnInteropAbilityLifecycle(
    napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationContext");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    if (interopCallback_ != nullptr) {
        TAG_LOGD(AAFwkTag::APPKIT, "interopCallback_ is not nullptr");
        auto result = interopCallback_->Register((void *)(info.argv[INDEX_ONE]));
        if (result != ERR_OK) {
            TAG_LOGE(AAFwkTag::APPKIT, "failed to register:%{public}d", result);
            AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        }
        return CreateJsUndefined(env);
    }
    interopCallback_ = InteropObjectInstance::CreateJsInteropAbilityLifecycleCallback((void *)env);
    auto result = interopCallback_->Register((void *)(info.argv[INDEX_ONE]));
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to register:%{public}d", result);
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    applicationContext->RegisterInteropAbilityLifecycleCallback(interopCallback_);
    return CreateJsUndefined(env);
}

napi_value JsApplicationContextUtils::OnOffInteropAbilityLifecycle(
    napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationContext");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    if (interopCallback_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null interop callback");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    if (info.argc == ARGC_ONE || !CheckTypeForNapiValue(env, info.argv[INDEX_ONE], napi_object)) {
        interopCallback_->Unregister();
    } else if (!interopCallback_->Unregister((void *)(info.argv[INDEX_ONE]))) {
        TAG_LOGE(AAFwkTag::APPKIT, "call Unregister failed");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    if (interopCallback_->Empty()) {
        interopCallback_.reset();
    }
    return CreateJsUndefined(env);
}

napi_value JsApplicationContextUtils::OnOffAbilityLifecycleEventSync(
    napi_env env, NapiCallbackInfo& info, int32_t callbackId)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");

    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationContext");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR);
        return CreateJsUndefined(env);
    }
    if (callback_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null callback");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR);
        return CreateJsUndefined(env);
    }
    if (!callback_->UnRegister(callbackId, true)) {
        TAG_LOGE(AAFwkTag::APPKIT, "call UnRegister failed");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR);
        return CreateJsUndefined(env);
    }
    return CreateJsUndefined(env);
}

napi_value JsApplicationContextUtils::OnOnEnvironment(
    napi_env env, NapiCallbackInfo& info, bool isSync)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationContext");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR);
        return CreateJsUndefined(env);
    }

    if (envCallback_ != nullptr) {
        TAG_LOGD(AAFwkTag::APPKIT, "envCallback_ is not nullptr.");
        return CreateJsValue(env, envCallback_->Register(info.argv[1], isSync));
    }
    envCallback_ = std::make_shared<JsEnvironmentCallback>(env);
    int32_t callbackId = envCallback_->Register(info.argv[1], isSync);
    applicationContext->RegisterEnvironmentCallback(envCallback_);
    TAG_LOGD(AAFwkTag::APPKIT, "OnOnEnvironment is end");
    return CreateJsValue(env, callbackId);
}

napi_value JsApplicationContextUtils::OnOffEnvironment(
    napi_env env, NapiCallbackInfo& info, int32_t callbackId)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");

    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationContext");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    std::weak_ptr<JsEnvironmentCallback> envCallbackWeak(envCallback_);
    NapiAsyncTask::CompleteCallback complete = [envCallbackWeak, callbackId](
            napi_env env, NapiAsyncTask &task, int32_t status) {
            HandleScope handleScope(env);
            auto env_callback = envCallbackWeak.lock();
            if (env_callback == nullptr) {
                TAG_LOGE(AAFwkTag::APPKIT, "null env_callback");
                task.Reject(env,
                    CreateJsError(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER,
                        "env_callback is nullptr"));
                return;
            }

            TAG_LOGD(AAFwkTag::APPKIT, "OnOffEnvironment begin");
            if (!env_callback->UnRegister(callbackId, false)) {
                TAG_LOGE(AAFwkTag::APPKIT, "call UnRegister failed");
                task.Reject(env, CreateJsError(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER,
                    "call UnRegister failed!"));
                return;
            }

            task.ResolveWithNoError(env, CreateJsUndefined(env));
        };
    napi_value lastParam = (info.argc <= ARGC_TWO) ? nullptr : info.argv[INDEX_TWO];
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsApplicationContextUtils::OnOffEnvironment", env,
        CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsApplicationContextUtils::OnOffEnvironmentEventSync(
    napi_env env, NapiCallbackInfo& info, int32_t callbackId)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");

    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationContext");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR);
        return CreateJsUndefined(env);
    }
    if (envCallback_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null envCallback");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR);
        return CreateJsUndefined(env);
    }
    if (!envCallback_->UnRegister(callbackId, true)) {
        TAG_LOGE(AAFwkTag::APPKIT, "call UnRegister failed");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR);
        return CreateJsUndefined(env);
    }
    return CreateJsUndefined(env);
}

napi_value JsApplicationContextUtils::OnOnApplicationStateChange(
    napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationContext");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    std::lock_guard<std::mutex> lock(applicationStateCallbackLock_);
    if (applicationStateCallback_ != nullptr) {
        applicationStateCallback_->Register(info.argv[INDEX_ONE]);
        return CreateJsUndefined(env);
    }

    applicationStateCallback_ = std::make_shared<JsApplicationStateChangeCallback>(env);
    applicationStateCallback_->Register(info.argv[INDEX_ONE]);
    applicationContext->RegisterApplicationStateChangeCallback(applicationStateCallback_);
    return CreateJsUndefined(env);
}

napi_value JsApplicationContextUtils::OnOffApplicationStateChange(
    napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationContext");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    std::lock_guard<std::mutex> lock(applicationStateCallbackLock_);
    if (applicationStateCallback_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationStateCallback_");
        ThrowInvalidParamError(env,
            "Parse applicationStateCallback failed, applicationStateCallback must be function.");
        return CreateJsUndefined(env);
    }

    if (info.argc == ARGC_ONE || !CheckTypeForNapiValue(env, info.argv[INDEX_ONE], napi_object)) {
        applicationStateCallback_->UnRegister();
    } else if (!applicationStateCallback_->UnRegister(info.argv[INDEX_ONE])) {
        TAG_LOGE(AAFwkTag::APPKIT, "call UnRegister failed");
        ThrowInvalidParamError(env, "Parse param call UnRegister failed, call UnRegister must be function.");
        return CreateJsUndefined(env);
    }

    if (applicationStateCallback_->IsEmpty()) {
        applicationStateCallback_.reset();
    }
    return CreateJsUndefined(env);
}

napi_value JsApplicationContextUtils::GetApplicationContext(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnGetApplicationContext, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetApplicationContext(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        TAG_LOGW(AAFwkTag::APPKIT, "null applicationContext");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    if (!applicationContext->GetApplicationInfoUpdateFlag()) {
        auto appContextObj = ApplicationContextManager::GetApplicationContextManager().GetGlobalObject(env);
        if (appContextObj != nullptr) {
            return appContextObj->GetNapiValue();
        }
    }

    napi_value value = CreateJsApplicationContext(env);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.ApplicationContext", &value, 1);
    if (systemModule == nullptr) {
        TAG_LOGW(AAFwkTag::APPKIT, "null systemModule");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    napi_value contextObj = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGE(AAFwkTag::APPKIT, "get object failed");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    auto workContext = new (std::nothrow) std::weak_ptr<ApplicationContext>(applicationContext);
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachApplicationContext, workContext, nullptr);
    if (workContext != nullptr) {
        auto res = napi_wrap(env, contextObj, workContext,
            [](napi_env, void *data, void *) {
              TAG_LOGD(AAFwkTag::APPKIT, "Finalizer for weak_ptr application context is called");
              delete static_cast<std::weak_ptr<ApplicationContext> *>(data);
              data = nullptr;
            },
            nullptr, nullptr);
        if (res != napi_ok && workContext != nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "napi_wrap failed:%{public}d", res);
            delete workContext;
            return CreateJsUndefined(env);
        }
    }
    return contextObj;
}

bool JsApplicationContextUtils::CheckCallerIsSystemApp()
{
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        return false;
    }
    return true;
}

napi_value JsApplicationContextUtils::CreateJsApplicationContext(napi_env env)
{
    TAG_LOGD(AAFwkTag::APPKIT, "start");
    HandleEscape handleEscape(env);
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        return nullptr;
    }

    std::shared_ptr<ApplicationContext> applicationContext = ApplicationContext::GetInstance();
    if (applicationContext == nullptr) {
        return object;
    }

    auto jsApplicationContextUtils = std::make_unique<JsApplicationContextUtils>(applicationContext);
    SetNamedNativePointer(env, object, APPLICATION_CONTEXT_NAME, jsApplicationContextUtils.release(),
        JsApplicationContextUtils::Finalizer);

    auto appInfo = applicationContext->GetApplicationInfo();
    if (appInfo != nullptr) {
        napi_set_named_property(env, object, "applicationInfo", CreateJsApplicationInfo(env, *appInfo));
    }
    auto resourceManager = applicationContext->GetResourceManager();
    std::shared_ptr<Context> context = std::dynamic_pointer_cast<Context>(applicationContext);
    if (resourceManager != nullptr) {
        napi_set_named_property(env, object, "resourceManager", CreateJsResourceManager(env, resourceManager, context));
    }
    napi_value applicationContextValue = nullptr;
    Context *contextPtr = context.get();
    int64_t applicationContextAddress = reinterpret_cast<int64_t>(contextPtr);
    auto status = napi_create_int64(env, applicationContextAddress, &applicationContextValue);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::APPKIT, "get context index fial");
        return nullptr;
    }
    napi_set_named_property(env, object, "index", applicationContextValue);

    std::string type = "ApplicationContext";
    napi_set_named_property(env, object, "contextType", CreateJsValue(env, type));

    BindNativeApplicationContextOne(env, object);
    BindNativeApplicationContextTwo(env, object);
    return handleEscape.Escape(object);
}

napi_value JsApplicationContextUtils::SetSupportedProcessCacheSelf(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnSetSupportedProcessCacheSelf, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnSetSupportedProcessCacheSelf(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");

    // only support one params
    if (info.argc == ARGC_ZERO) {
        TAG_LOGE(AAFwkTag::APPKIT, "Not enough params");
        ThrowInvalidParamError(env, "Not enough params.");
        return CreateJsUndefined(env);
    }
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGW(AAFwkTag::APPKIT, "null applicationContext");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST);
        return CreateJsUndefined(env);
    }

    bool isSupport = false;
    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], isSupport)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse isSupport failed");
        ThrowInvalidParamError(env,
            "Parse param isSupport failed, isSupport must be boolean.");
        return CreateJsUndefined(env);
    }

    int32_t errCode = applicationContext->SetSupportedProcessCacheSelf(isSupport);
    if (errCode == AAFwk::ERR_CAPABILITY_NOT_SUPPORT) {
        TAG_LOGE(AAFwkTag::APPKIT, "process cache feature disabled");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_NO_SUCH_SYSCAP);
    } else if (errCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "set failed");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR);
    }
    return CreateJsUndefined(env);
}

void JsApplicationContextUtils::BindNativeApplicationContextOne(napi_env env, napi_value object)
{
    BindNativeProperty(env, object, "cacheDir", JsApplicationContextUtils::GetCacheDir);
    BindNativeProperty(env, object, "tempDir", JsApplicationContextUtils::GetTempDir);
    BindNativeProperty(env, object, "resourceDir", JsApplicationContextUtils::GetResourceDir);
    BindNativeProperty(env, object, "filesDir", JsApplicationContextUtils::GetFilesDir);
    BindNativeProperty(env, object, "distributedFilesDir", JsApplicationContextUtils::GetDistributedFilesDir);
    BindNativeProperty(env, object, "databaseDir", JsApplicationContextUtils::GetDatabaseDir);
    BindNativeProperty(env, object, "preferencesDir", JsApplicationContextUtils::GetPreferencesDir);
    BindNativeProperty(env, object, "bundleCodeDir", JsApplicationContextUtils::GetBundleCodeDir);
    BindNativeProperty(env, object, "cloudFileDir", JsApplicationContextUtils::GetCloudFileDir);
    BindNativeProperty(env, object, "logFileDir", JsApplicationContextUtils::GetLogFileDir);
    BindNativeProperty(env, object, "processName", JsApplicationContextUtils::GetProcessName);
    BindNativeFunction(env, object, "registerAbilityLifecycleCallback", MD_NAME,
        JsApplicationContextUtils::RegisterAbilityLifecycleCallback);
    BindNativeFunction(env, object, "unregisterAbilityLifecycleCallback", MD_NAME,
        JsApplicationContextUtils::UnregisterAbilityLifecycleCallback);
    BindNativeFunction(env, object, "registerEnvironmentCallback", MD_NAME,
        JsApplicationContextUtils::RegisterEnvironmentCallback);
    BindNativeFunction(env, object, "unregisterEnvironmentCallback", MD_NAME,
        JsApplicationContextUtils::UnregisterEnvironmentCallback);
    BindNativeFunction(env, object, "createBundleContext", MD_NAME, JsApplicationContextUtils::CreateBundleContext);
    BindNativeFunction(env, object, "switchArea", MD_NAME, JsApplicationContextUtils::SwitchArea);
    BindNativeFunction(env, object, "getArea", MD_NAME, JsApplicationContextUtils::GetArea);
    BindNativeFunction(env, object, "createModuleContext", MD_NAME, JsApplicationContextUtils::CreateModuleContext);
    BindNativeFunction(env, object, "createSystemHspModuleResourceManager", MD_NAME,
        JsApplicationContextUtils::CreateSystemHspModuleResourceManager);
    BindNativeFunction(env, object, "createModuleResourceManager", MD_NAME,
        JsApplicationContextUtils::CreateModuleResourceManager);
    BindNativeFunction(env, object, "on", MD_NAME, JsApplicationContextUtils::On);
    BindNativeFunction(env, object, "off", MD_NAME, JsApplicationContextUtils::Off);
    BindNativeFunction(env, object, "getApplicationContext", MD_NAME,
        JsApplicationContextUtils::GetApplicationContext);
    BindNativeFunction(env, object, "killAllProcesses", MD_NAME, JsApplicationContextUtils::KillProcessBySelf);
    BindNativeFunction(env, object, "setColorMode", MD_NAME, JsApplicationContextUtils::SetColorMode);
    BindNativeFunction(env, object, "onSystemConfigurationUpdated", MD_NAME,
        JsApplicationContextUtils::RegisterSystemConfigurationUpdated);
    BindNativeFunction(env, object, "offSystemConfigurationUpdated", MD_NAME,
        JsApplicationContextUtils::UnRegisterSystemConfigurationUpdated);
    BindNativeFunction(env, object, "setLanguage", MD_NAME, JsApplicationContextUtils::SetLanguage);
    BindNativeFunction(env, object, "setFont", MD_NAME, JsApplicationContextUtils::SetFont);
    BindNativeFunction(env, object, "clearUpApplicationData", MD_NAME,
        JsApplicationContextUtils::ClearUpApplicationData);
    BindNativeFunction(env, object, "createAreaModeContext", MD_NAME, JsApplicationContextUtils::CreateAreaModeContext);
    BindNativeFunction(env, object, "createDisplayContext", MD_NAME,
        JsApplicationContextUtils::CreateDisplayContext);
}

void JsApplicationContextUtils::BindNativeApplicationContextTwo(napi_env env, napi_value object)
{
    BindNativeFunction(env, object, "preloadUIExtensionAbility", MD_NAME,
        JsApplicationContextUtils::PreloadUIExtensionAbility);
    BindNativeFunction(env, object, "getProcessRunningInformation", MD_NAME,
        JsApplicationContextUtils::GetRunningProcessInformation);
    BindNativeFunction(env, object, "getRunningProcessInformation", MD_NAME,
        JsApplicationContextUtils::GetRunningProcessInformation);
    BindNativeFunction(env, object, "getCurrentAppCloneIndex", MD_NAME,
        JsApplicationContextUtils::GetCurrentAppCloneIndex);
    BindNativeFunction(env, object, "getCurrentInstanceKey", MD_NAME,
        JsApplicationContextUtils::GetCurrentInstanceKey);
    BindNativeFunction(env, object, "getAllRunningInstanceKeys", MD_NAME,
        JsApplicationContextUtils::GetAllRunningInstanceKeys);
    BindNativeFunction(env, object, "getGroupDir", MD_NAME, JsApplicationContextUtils::GetGroupDir);
    BindNativeFunction(env, object, "restartApp", MD_NAME, JsApplicationContextUtils::RestartApp);
    BindNativeFunction(env, object, "setSupportedProcessCache", MD_NAME,
        JsApplicationContextUtils::SetSupportedProcessCacheSelf);
    BindNativeFunction(env, object, "setFontSizeScale", MD_NAME,
        JsApplicationContextUtils::SetFontSizeScale);
    BindNativeFunction(env, object, "getAllWindowStages", MD_NAME, JsApplicationContextUtils::GetAllWindowStages);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
