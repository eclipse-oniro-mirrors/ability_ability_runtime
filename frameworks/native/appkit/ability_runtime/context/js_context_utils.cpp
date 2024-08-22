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

#include "js_context_utils.h"

#include "ability_runtime_error_util.h"
#include "application_context.h"
#include "application_context_manager.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "js_application_context_utils.h"
#include "js_data_struct_converter.h"
#include "js_resource_manager_utils.h"
#include "js_runtime_utils.h"
#include "tokenid_kit.h"
#include "js_error_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr char BASE_CONTEXT_NAME[] = "__base_context_ptr__";

constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t INDEX_ONE = 1;

class JsBaseContext {
public:
    explicit JsBaseContext(std::weak_ptr<Context>&& context) : context_(std::move(context)) {}
    virtual ~JsBaseContext() = default;

    static void Finalizer(napi_env env, void* data, void* hint);
    static napi_value CreateBundleContext(napi_env env, napi_callback_info info);
    static napi_value GetApplicationContext(napi_env env, napi_callback_info info);
    static napi_value SwitchArea(napi_env env, napi_callback_info info);
    static napi_value GetArea(napi_env env, napi_callback_info info);
    static napi_value CreateModuleContext(napi_env env, napi_callback_info info);
    static napi_value CreateSystemHspModuleResourceManager(napi_env env, napi_callback_info info);
    static napi_value CreateModuleResourceManager(napi_env env, napi_callback_info info);

    napi_value OnGetCacheDir(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetTempDir(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetResourceDir(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetFilesDir(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetDistributedFilesDir(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetDatabaseDir(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetPreferencesDir(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetGroupDir(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetBundleCodeDir(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetCloudFileDir(napi_env env, NapiCallbackInfo& info);

    static napi_value GetCacheDir(napi_env env, napi_callback_info info);
    static napi_value GetTempDir(napi_env env, napi_callback_info info);
    static napi_value GetResourceDir(napi_env env, napi_callback_info info);
    static napi_value GetFilesDir(napi_env env, napi_callback_info info);
    static napi_value GetDistributedFilesDir(napi_env env, napi_callback_info info);
    static napi_value GetDatabaseDir(napi_env env, napi_callback_info info);
    static napi_value GetPreferencesDir(napi_env env, napi_callback_info info);
    static napi_value GetGroupDir(napi_env env, napi_callback_info info);
    static napi_value GetBundleCodeDir(napi_env env, napi_callback_info info);
    static napi_value GetCloudFileDir(napi_env env, napi_callback_info info);

protected:
    std::weak_ptr<Context> context_;

private:
    napi_value OnCreateBundleContext(napi_env env, NapiCallbackInfo& info);
    napi_value CreateJsBundleContext(napi_env env, const std::shared_ptr<Context>& bundleContext);
    napi_value OnGetApplicationContext(napi_env env, NapiCallbackInfo& info);
    napi_value CreateJSApplicationContext(napi_env env, const std::shared_ptr<ApplicationContext> applicationContext);
    napi_value OnSwitchArea(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetArea(napi_env env, NapiCallbackInfo& info);
    napi_value OnCreateModuleContext(napi_env env, NapiCallbackInfo& info);
    napi_value CreateJsModuleContext(napi_env env, const std::shared_ptr<Context>& moduleContext);
    napi_value OnCreateSystemHspModuleResourceManager(napi_env env, NapiCallbackInfo& info);
    napi_value OnCreateModuleResourceManager(napi_env env, NapiCallbackInfo& info);
    bool CheckCallerIsSystemApp();
};

void JsBaseContext::Finalizer(napi_env env, void* data, void* hint)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    std::unique_ptr<JsBaseContext>(static_cast<JsBaseContext*>(data));
}

napi_value JsBaseContext::CreateBundleContext(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsBaseContext, OnCreateBundleContext, BASE_CONTEXT_NAME);
}

napi_value JsBaseContext::GetApplicationContext(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsBaseContext, OnGetApplicationContext, BASE_CONTEXT_NAME);
}

napi_value JsBaseContext::SwitchArea(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsBaseContext, OnSwitchArea, BASE_CONTEXT_NAME);
}

napi_value JsBaseContext::OnSwitchArea(napi_env env, NapiCallbackInfo& info)
{
    if (info.argc == 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "Not enough params");
        return CreateJsUndefined(env);
    }

    auto context = context_.lock();
    if (!context) {
        TAG_LOGW(AAFwkTag::APPKIT, "context is already released");
        return CreateJsUndefined(env);
    }

    int mode = 0;
    if (!ConvertFromJsValue(env, info.argv[0], mode)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse mode failed");
        return CreateJsUndefined(env);
    }

    context->SwitchArea(mode);

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
    return CreateJsUndefined(env);
}

napi_value JsBaseContext::CreateModuleContext(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsBaseContext, OnCreateModuleContext, BASE_CONTEXT_NAME);
}

napi_value JsBaseContext::OnCreateModuleContext(napi_env env, NapiCallbackInfo& info)
{
    auto context = context_.lock();
    if (!context) {
        TAG_LOGW(AAFwkTag::APPKIT, "context is already released");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    std::shared_ptr<Context> moduleContext = nullptr;
    std::string moduleName;

    if (!ConvertFromJsValue(env, info.argv[1], moduleName)) {
        TAG_LOGD(AAFwkTag::APPKIT, "Parse inner module name");
        if (!ConvertFromJsValue(env, info.argv[0], moduleName)) {
            TAG_LOGE(AAFwkTag::APPKIT, "Parse moduleName failed");
            ThrowInvalidParamError(env, "Parse param moduleName failed, moduleName must be string.");
            return CreateJsUndefined(env);
        }
        moduleContext = context->CreateModuleContext(moduleName);
    } else {
        std::string bundleName;
        if (!ConvertFromJsValue(env, info.argv[0], bundleName)) {
            TAG_LOGE(AAFwkTag::APPKIT, "Parse bundleName failed");
            ThrowInvalidParamError(env, "Parse param bundleName failed, bundleName must be string.");
            return CreateJsUndefined(env);
        }
        if (!CheckCallerIsSystemApp()) {
            TAG_LOGE(AAFwkTag::APPKIT, "This application is not system-app, can not use system-api");
            AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_NOT_SYSTEM_APP);
            return CreateJsUndefined(env);
        }
        TAG_LOGD(AAFwkTag::APPKIT, "Parse outer module name");
        moduleContext = context->CreateModuleContext(bundleName, moduleName);
    }

    if (!moduleContext) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to create module context");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    return CreateJsModuleContext(env, moduleContext);
}

napi_value JsBaseContext::CreateJsModuleContext(napi_env env, const std::shared_ptr<Context>& moduleContext)
{
    napi_value value = CreateJsBaseContext(env, moduleContext, true);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.Context", &value, 1);
    if (systemModule == nullptr) {
        TAG_LOGW(AAFwkTag::APPKIT, "invalid systemModule");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    napi_value object = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(env, object, napi_object)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get object");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    auto workContext = new (std::nothrow) std::weak_ptr<Context>(moduleContext);
    napi_coerce_to_native_binding_object(env, object, DetachCallbackFunc, AttachBaseContext, workContext, nullptr);
    napi_wrap(env, object, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGD(AAFwkTag::APPKIT, "Finalizer for weak_ptr module context is called");
            delete static_cast<std::weak_ptr<Context> *>(data);
        },
        nullptr, nullptr);
    return object;
}

napi_value JsBaseContext::CreateSystemHspModuleResourceManager(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsBaseContext,
        OnCreateSystemHspModuleResourceManager, BASE_CONTEXT_NAME);
}

napi_value JsBaseContext::OnCreateSystemHspModuleResourceManager(napi_env env, NapiCallbackInfo& info)
{
    auto context = context_.lock();
    if (!context) {
        TAG_LOGW(AAFwkTag::APPKIT, "context is already released");
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
        TAG_LOGE(AAFwkTag::APPKIT, "Parse moduleName failed");
        ThrowInvalidParamError(env, "Parse param moduleName failed, moduleName must be string.");
        return CreateJsUndefined(env);
    }

    std::shared_ptr<Global::Resource::ResourceManager> resourceManager = nullptr;
    int32_t retCode = context->CreateSystemHspModuleResourceManager(bundleName, moduleName, resourceManager);
    if (resourceManager == nullptr && retCode == ERR_ABILITY_RUNTIME_EXTERNAL_NOT_SYSTEM_HSP) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to create resourceManager");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_NOT_SYSTEM_HSP);
        return CreateJsUndefined(env);
    }
    if (resourceManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to create resourceManager");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    return CreateJsResourceManager(env, resourceManager, nullptr);
}

napi_value JsBaseContext::CreateModuleResourceManager(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsBaseContext, OnCreateModuleResourceManager, BASE_CONTEXT_NAME);
}

napi_value JsBaseContext::OnCreateModuleResourceManager(napi_env env, NapiCallbackInfo& info)
{
    auto context = context_.lock();
    if (!context) {
        TAG_LOGW(AAFwkTag::APPKIT, "applicationContext is already released");
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
        TAG_LOGE(AAFwkTag::APPKIT, "This application is not system-app, can not use system-api");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_NOT_SYSTEM_APP);
        return CreateJsUndefined(env);
    }
    auto resourceManager = context->CreateModuleResourceManager(bundleName, moduleName);
    if (resourceManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to create resourceManager");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    auto jsResourceManager = CreateJsResourceManager(env, resourceManager, nullptr);
    return jsResourceManager;
}

napi_value JsBaseContext::GetArea(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsBaseContext, OnGetArea, BASE_CONTEXT_NAME);
}

napi_value JsBaseContext::OnGetArea(napi_env env, NapiCallbackInfo& info)
{
    auto context = context_.lock();
    if (!context) {
        TAG_LOGW(AAFwkTag::APPKIT, "context is already released");
        return CreateJsUndefined(env);
    }
    int area = context->GetArea();
    return CreateJsValue(env, area);
}

napi_value JsBaseContext::GetCacheDir(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsBaseContext, OnGetCacheDir, BASE_CONTEXT_NAME);
}

napi_value JsBaseContext::OnGetCacheDir(napi_env env, NapiCallbackInfo& info)
{
    auto context = context_.lock();
    if (!context) {
        TAG_LOGW(AAFwkTag::APPKIT, "context is already released");
        return CreateJsUndefined(env);
    }
    std::string path = context->GetCacheDir();
    return CreateJsValue(env, path);
}

napi_value JsBaseContext::GetTempDir(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsBaseContext, OnGetTempDir, BASE_CONTEXT_NAME);
}

napi_value JsBaseContext::OnGetTempDir(napi_env env, NapiCallbackInfo& info)
{
    auto context = context_.lock();
    if (!context) {
        TAG_LOGW(AAFwkTag::APPKIT, "context is already released");
        return CreateJsUndefined(env);
    }
    std::string path = context->GetTempDir();
    return CreateJsValue(env, path);
}

napi_value JsBaseContext::GetResourceDir(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsBaseContext, OnGetResourceDir, BASE_CONTEXT_NAME);
}

napi_value JsBaseContext::OnGetResourceDir(napi_env env, NapiCallbackInfo& info)
{
    auto context = context_.lock();
    if (!context) {
        TAG_LOGW(AAFwkTag::APPKIT, "context is already released");
        return CreateJsUndefined(env);
    }
    std::string path = context->GetResourceDir();
    return CreateJsValue(env, path);
}

napi_value JsBaseContext::GetFilesDir(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsBaseContext, OnGetFilesDir, BASE_CONTEXT_NAME);
}

napi_value JsBaseContext::OnGetFilesDir(napi_env env, NapiCallbackInfo& info)
{
    auto context = context_.lock();
    if (!context) {
        TAG_LOGW(AAFwkTag::APPKIT, "context is already released");
        return CreateJsUndefined(env);
    }
    std::string path = context->GetFilesDir();
    return CreateJsValue(env, path);
}

napi_value JsBaseContext::GetDistributedFilesDir(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsBaseContext, OnGetDistributedFilesDir, BASE_CONTEXT_NAME);
}

napi_value JsBaseContext::OnGetDistributedFilesDir(napi_env env, NapiCallbackInfo& info)
{
    auto context = context_.lock();
    if (!context) {
        TAG_LOGW(AAFwkTag::APPKIT, "context is already released");
        return CreateJsUndefined(env);
    }
    std::string path = context->GetDistributedFilesDir();
    return CreateJsValue(env, path);
}

napi_value JsBaseContext::GetDatabaseDir(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsBaseContext, OnGetDatabaseDir, BASE_CONTEXT_NAME);
}

napi_value JsBaseContext::OnGetDatabaseDir(napi_env env, NapiCallbackInfo& info)
{
    auto context = context_.lock();
    if (!context) {
        TAG_LOGW(AAFwkTag::APPKIT, "context is already released");
        return CreateJsUndefined(env);
    }
    std::string path = context->GetDatabaseDir();
    return CreateJsValue(env, path);
}

napi_value JsBaseContext::GetPreferencesDir(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsBaseContext, OnGetPreferencesDir, BASE_CONTEXT_NAME);
}

napi_value JsBaseContext::OnGetPreferencesDir(napi_env env, NapiCallbackInfo& info)
{
    auto context = context_.lock();
    if (!context) {
        TAG_LOGW(AAFwkTag::APPKIT, "context is already released");
        return CreateJsUndefined(env);
    }
    std::string path = context->GetPreferencesDir();
    return CreateJsValue(env, path);
}

napi_value JsBaseContext::GetGroupDir(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsBaseContext, OnGetGroupDir, BASE_CONTEXT_NAME);
}

napi_value JsBaseContext::OnGetGroupDir(napi_env env, NapiCallbackInfo& info)
{
    if (info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
        TAG_LOGE(AAFwkTag::APPKIT, "Not enough params");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    std::string groupId;
    if (!ConvertFromJsValue(env, info.argv[0], groupId)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse groupId failed");
        ThrowInvalidParamError(env, "Parse param groupId failed, groupId must be string.");
        return CreateJsUndefined(env);
    }

    auto complete = [context = context_, groupId]
        (napi_env env, NapiAsyncTask& task, int32_t status) {
        auto completeContext = context.lock();
        if (!completeContext) {
            task.Reject(env, CreateJsError(env, ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST,
                "completeContext if already released."));
            return;
        }
        std::string path = completeContext->GetGroupDir(groupId);
        task.ResolveWithNoError(env, CreateJsValue(env, path));
    };

    napi_value lastParam = (info.argc == ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsBaseContext::OnGetGroupDir",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsBaseContext::GetBundleCodeDir(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsBaseContext, OnGetBundleCodeDir, BASE_CONTEXT_NAME);
}

napi_value JsBaseContext::OnGetBundleCodeDir(napi_env env, NapiCallbackInfo& info)
{
    auto context = context_.lock();
    if (!context) {
        TAG_LOGW(AAFwkTag::APPKIT, "context is already released");
        return CreateJsUndefined(env);
    }
    std::string path = context->GetBundleCodeDir();
    return CreateJsValue(env, path);
}

napi_value JsBaseContext::GetCloudFileDir(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsBaseContext, OnGetCloudFileDir, BASE_CONTEXT_NAME);
}

napi_value JsBaseContext::OnGetCloudFileDir(napi_env env, NapiCallbackInfo& info)
{
    auto context = context_.lock();
    if (!context) {
        TAG_LOGW(AAFwkTag::APPKIT, "context is already released");
        return CreateJsUndefined(env);
    }
    std::string path = context->GetCloudFileDir();
    return CreateJsValue(env, path);
}

napi_value JsBaseContext::OnCreateBundleContext(napi_env env, NapiCallbackInfo& info)
{
    if (!CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::APPKIT, "This application is not system-app, can not use system-api");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_NOT_SYSTEM_APP);
        return CreateJsUndefined(env);
    }

    if (info.argc == 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "Not enough params");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    auto context = context_.lock();
    if (!context) {
        TAG_LOGW(AAFwkTag::APPKIT, "context is already released");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    std::string bundleName;
    if (!ConvertFromJsValue(env, info.argv[0], bundleName)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse bundleName failed");
        ThrowInvalidParamError(env, "Parse param bundleName failed, bundleName must be string.");
        return CreateJsUndefined(env);
    }

    auto bundleContext = context->CreateBundleContext(bundleName);
    if (!bundleContext) {
        TAG_LOGE(AAFwkTag::APPKIT, "bundleContext is nullptr");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    return CreateJsBundleContext(env, bundleContext);
}

napi_value JsBaseContext::CreateJsBundleContext(napi_env env, const std::shared_ptr<Context>& bundleContext)
{
    napi_value value = CreateJsBaseContext(env, bundleContext, true);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.Context", &value, 1);
    if (systemModule == nullptr) {
        TAG_LOGW(AAFwkTag::APPKIT, "OnCreateBundleContext, invalid systemModule");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    napi_value object = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(env, object, napi_object)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get object");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    auto workContext = new (std::nothrow) std::weak_ptr<Context>(bundleContext);
    napi_coerce_to_native_binding_object(env, object, DetachCallbackFunc, AttachBaseContext, workContext, nullptr);
    napi_wrap(env, object, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGD(AAFwkTag::APPKIT, "Finalizer for weak_ptr bundle context is called");
            delete static_cast<std::weak_ptr<Context> *>(data);
        },
        nullptr, nullptr);
    return object;
}

napi_value JsBaseContext::OnGetApplicationContext(napi_env env, NapiCallbackInfo& info)
{
    auto context = context_.lock();
    if (!context) {
        TAG_LOGW(AAFwkTag::APPKIT, "context is already released");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    auto applicationContext = Context::GetApplicationContext();
    if (applicationContext == nullptr) {
        TAG_LOGW(AAFwkTag::APPKIT, "applicationContext is nullptr");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    if (!applicationContext->GetApplicationInfoUpdateFlag()) {
        std::shared_ptr<NativeReference> applicationContextObj =
            ApplicationContextManager::GetApplicationContextManager().GetGlobalObject(env);
        if (applicationContextObj != nullptr) {
            napi_value objValue = applicationContextObj->GetNapiValue();
            return objValue;
        }
    }
    return CreateJSApplicationContext(env, applicationContext);
}

napi_value JsBaseContext::CreateJSApplicationContext(napi_env env,
    const std::shared_ptr<ApplicationContext> applicationContext)
{
    napi_value value = JsApplicationContextUtils::CreateJsApplicationContext(env);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.ApplicationContext", &value, 1);
    if (systemModule == nullptr) {
        TAG_LOGW(AAFwkTag::APPKIT, "OnGetApplicationContext, invalid systemModule");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    napi_value object = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(env, object, napi_object)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get object");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    auto workContext = new (std::nothrow) std::weak_ptr<ApplicationContext>(applicationContext);
    napi_coerce_to_native_binding_object(
        env, object, DetachCallbackFunc, AttachApplicationContext, workContext, nullptr);
    napi_wrap(env, object, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGD(AAFwkTag::APPKIT, "Finalizer for weak_ptr application context is called");
            delete static_cast<std::weak_ptr<ApplicationContext> *>(data);
            data = nullptr;
        },
        nullptr, nullptr);
    napi_ref ref = nullptr;
    napi_create_reference(env, object, 1, &ref);
    ApplicationContextManager::GetApplicationContextManager()
        .AddGlobalObject(env, std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref)));
    applicationContext->SetApplicationInfoUpdateFlag(false);
    return object;
}

bool JsBaseContext::CheckCallerIsSystemApp()
{
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        return false;
    }
    return true;
}
} // namespace

napi_value AttachBaseContext(napi_env env, void* value, void* hint)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (value == nullptr || env == nullptr) {
        TAG_LOGW(AAFwkTag::APPKIT, "invalid parameter");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<Context>*>(value)->lock();
    if (ptr == nullptr) {
        TAG_LOGW(AAFwkTag::APPKIT, "invalid context");
        return nullptr;
    }
    napi_value object = CreateJsBaseContext(env, ptr, true);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.Context", &object, 1);
    if (systemModule == nullptr) {
        TAG_LOGW(AAFwkTag::APPKIT, "AttachBaseContext, invalid systemModule");
        return nullptr;
    }

    napi_value contextObj = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get object");
        return nullptr;
    }
    napi_coerce_to_native_binding_object(env, contextObj, DetachCallbackFunc, AttachBaseContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<Context>(ptr);
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGD(AAFwkTag::APPKIT, "Finalizer for weak_ptr base context is called");
            delete static_cast<std::weak_ptr<Context> *>(data);
        },
        nullptr, nullptr);
    return contextObj;
}

napi_value AttachApplicationContext(napi_env env, void* value, void* hint)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (value == nullptr || env == nullptr) {
        TAG_LOGW(AAFwkTag::APPKIT, "invalid parameter");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<ApplicationContext>*>(value)->lock();
    if (ptr == nullptr) {
        TAG_LOGW(AAFwkTag::APPKIT, "invalid context");
        return nullptr;
    }
    napi_value object = JsApplicationContextUtils::CreateJsApplicationContext(env);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.ApplicationContext", &object, 1);
    if (systemModule == nullptr) {
        TAG_LOGW(AAFwkTag::APPKIT, "invalid systemModule");
        return nullptr;
    }
    auto contextObj = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get object");
        return nullptr;
    }
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachApplicationContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<ApplicationContext>(ptr);
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGD(AAFwkTag::APPKIT, "Finalizer for weak_ptr application context is called");
            delete static_cast<std::weak_ptr<ApplicationContext> *>(data);
            data = nullptr;
        },
        nullptr, nullptr);
    return contextObj;
}

napi_value CreateJsBaseContext(napi_env env, std::shared_ptr<Context> context, bool keepContext)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        TAG_LOGW(AAFwkTag::APPKIT, "invalid object");
        return nullptr;
    }
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "context object");
        return nullptr;
    }
    auto jsContext = std::make_unique<JsBaseContext>(context);
    SetNamedNativePointer(env, object, BASE_CONTEXT_NAME, jsContext.release(), JsBaseContext::Finalizer);

    auto appInfo = context->GetApplicationInfo();
    if (appInfo != nullptr) {
        napi_set_named_property(env, object, "applicationInfo", CreateJsApplicationInfo(env, *appInfo));
    }
    auto hapModuleInfo = context->GetHapModuleInfo();
    if (hapModuleInfo != nullptr) {
        napi_set_named_property(env, object, "currentHapModuleInfo", CreateJsHapModuleInfo(env, *hapModuleInfo));
    }
    auto resourceManager = context->GetResourceManager();
    if (resourceManager != nullptr) {
        auto jsResourceManager = CreateJsResourceManager(env, resourceManager, context);
        if (jsResourceManager != nullptr) {
            napi_set_named_property(env, object, "resourceManager", jsResourceManager);
        } else {
            TAG_LOGE(AAFwkTag::APPKIT, "jsResourceManager is nullptr");
        }
    }

    BindNativeProperty(env, object, "cacheDir", JsBaseContext::GetCacheDir);
    BindNativeProperty(env, object, "tempDir", JsBaseContext::GetTempDir);
    BindNativeProperty(env, object, "resourceDir", JsBaseContext::GetResourceDir);
    BindNativeProperty(env, object, "filesDir", JsBaseContext::GetFilesDir);
    BindNativeProperty(env, object, "distributedFilesDir", JsBaseContext::GetDistributedFilesDir);
    BindNativeProperty(env, object, "databaseDir", JsBaseContext::GetDatabaseDir);
    BindNativeProperty(env, object, "preferencesDir", JsBaseContext::GetPreferencesDir);
    BindNativeProperty(env, object, "bundleCodeDir", JsBaseContext::GetBundleCodeDir);
    BindNativeProperty(env, object, "cloudFileDir", JsBaseContext::GetCloudFileDir);
    BindNativeProperty(env, object, "area", JsBaseContext::GetArea);
    const char *moduleName = "JsBaseContext";
    BindNativeFunction(env, object, "createBundleContext", moduleName, JsBaseContext::CreateBundleContext);
    BindNativeFunction(env, object, "getApplicationContext", moduleName, JsBaseContext::GetApplicationContext);
    BindNativeFunction(env, object, "switchArea", moduleName, JsBaseContext::SwitchArea);
    BindNativeFunction(env, object, "getArea", moduleName, JsBaseContext::GetArea);
    BindNativeFunction(env, object, "createModuleContext", moduleName, JsBaseContext::CreateModuleContext);
    BindNativeFunction(env, object, "createSystemHspModuleResourceManager", moduleName,JsBaseContext::CreateSystemHspModuleResourceManager);
    BindNativeFunction(env, object, "createModuleResourceManager", moduleName,JsBaseContext::CreateModuleResourceManager);
    BindNativeFunction(env, object, "getGroupDir", moduleName, JsBaseContext::GetGroupDir);
    return object;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
