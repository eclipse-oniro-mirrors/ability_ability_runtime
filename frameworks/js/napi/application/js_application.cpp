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

#include "js_application.h"

#include "ability_runtime_error_util.h"
#include "accesstoken_kit.h"
#include "app_mgr_client.h"
#include "context_impl.h"
#include "hilog_tag_wrapper.h"
#include "js_application_context_utils.h"
#include "js_error_utils.h"
#include "js_runtime_utils.h"
#include "js_context_utils.h"
#include "napi_base_context.h"
#include "singleton.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
    constexpr size_t ARGC_ZERO = 0;
    constexpr size_t ARGC_ONE = 1;
    constexpr size_t ARGC_TWO = 2;
    constexpr size_t ARGC_THREE = 3;
    constexpr const char* PERMISSION_GET_BUNDLE_INFO = "ohos.permission.GET_BUNDLE_INFO_PRIVILEGED";
}
void JsApplication::Finalizer(napi_env env, void *data, void *hint)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Called.");
    std::unique_ptr<JsApplication>(static_cast<JsApplication *>(data));
}

napi_value JsApplication::GetApplicationContext(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsApplication, OnGetApplicationContext);
}

napi_value JsApplication::OnGetApplicationContext(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Called.");
    napi_value value = JsApplicationContextUtils::CreateJsApplicationContext(env);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.ApplicationContext", &value, 1);
    if (systemModule == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid systemModule");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR);
        return CreateJsUndefined(env);
    }
    napi_value object = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(env, object, napi_object)) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to get context native object");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR);
        return CreateJsUndefined(env);
    }
    return object;
}

napi_value JsApplication::CreateModuleContext(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsApplication, OnCreateModuleContext);
}

napi_value JsApplication::CreateBundleContext(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsApplication, OnCreateBundleContext);
}

napi_value JsApplication::CreatePluginModuleContext(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsApplication, OnCreatePluginModuleContext);
}


napi_value JsApplication::OnCreatePluginModuleContext(napi_env env, NapiCallbackInfo &info)
{
    if (info.argc < ARGC_THREE) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    bool stageMode = false;
    napi_status status = OHOS::AbilityRuntime::IsStageContext(env, info.argv[ARGC_ZERO], stageMode);
    if (status != napi_ok || !stageMode) {
        ThrowInvalidParamError(env, "Parse param context failed, must be a context of stageMode.");
        return CreateJsUndefined(env);
    }

    auto context = OHOS::AbilityRuntime::GetStageModeContext(env, info.argv[ARGC_ZERO]);
    if (context == nullptr) {
        ThrowInvalidParamError(env, "Parse param context failed, must not be nullptr.");
        return CreateJsUndefined(env);
    }

    auto inputContextPtr = Context::ConvertTo<Context>(context);
    if (inputContextPtr == nullptr) {
        ThrowInvalidParamError(env, "Parse param context failed, must be a context.");
        return CreateJsUndefined(env);
    }

    std::shared_ptr<std::shared_ptr<Context>> moduleContext = std::make_shared<std::shared_ptr<Context>>();
    std::shared_ptr<ContextImpl> contextImpl = std::make_shared<ContextImpl>();
    if (contextImpl == nullptr) {
        ThrowInvalidParamError(env, "create context failed.");
        return CreateJsUndefined(env);
    }
    contextImpl->SetProcessName(context->GetProcessName());
    std::string moduleName = "";
    std::string pluginBundleName = "";

    if (!ConvertFromJsValue(env, info.argv[ARGC_TWO], moduleName)
        || !ConvertFromJsValue(env, info.argv[ARGC_ONE], pluginBundleName)) {
        ThrowInvalidParamError(env, "Parse param failed, moduleName and pluginBundleName must be string.");
        return CreateJsUndefined(env);
    }

    TAG_LOGD(AAFwkTag::APPKIT, "moduleName: %{public}s, pluginBundleName: %{public}s",
        moduleName.c_str(), pluginBundleName.c_str());
    NapiAsyncTask::ExecuteCallback execute = [moduleName, pluginBundleName, contextImpl,
        moduleContext, inputContextPtr]() {
        if (contextImpl != nullptr) {
            *moduleContext = contextImpl->CreatePluginContext(pluginBundleName, moduleName, inputContextPtr);
        }
    };

    NapiAsyncTask::CompleteCallback complete;
    SetCreateCompleteCallback(moduleContext, complete);

    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsApplication::OnCreatePluginModuleContext",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));

    return result;
}

napi_value JsApplication::OnCreateModuleContext(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Called");
    if (info.argc < ARGC_TWO) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid argc");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    bool stageMode = false;
    napi_status status = OHOS::AbilityRuntime::IsStageContext(env, info.argv[ARGC_ZERO], stageMode);
    if (status != napi_ok || !stageMode) {
        TAG_LOGE(AAFwkTag::APPKIT, "not stageMode");
        ThrowInvalidParamError(env, "Parse param context failed, must be a context of stageMode.");
        return CreateJsUndefined(env);
    }

    auto context = OHOS::AbilityRuntime::GetStageModeContext(env, info.argv[ARGC_ZERO]);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        ThrowInvalidParamError(env, "Parse param context failed, must not be nullptr.");
        return CreateJsUndefined(env);
    }

    auto inputContextPtr = Context::ConvertTo<Context>(context);
    if (inputContextPtr == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Convert to context failed");
        ThrowInvalidParamError(env, "Parse param context failed, must be a context.");
        return CreateJsUndefined(env);
    }

    std::shared_ptr<std::shared_ptr<Context>> moduleContext = std::make_shared<std::shared_ptr<Context>>();
    std::shared_ptr<ContextImpl> contextImpl = std::make_shared<ContextImpl>();
    if (contextImpl == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        ThrowInvalidParamError(env, "create context failed.");
        return CreateJsUndefined(env);
    }
    contextImpl->SetProcessName(context->GetProcessName());
    std::string moduleName = "";
    std::string bundleName = "";
    if (info.argc == ARGC_TWO) {
        TAG_LOGD(AAFwkTag::APPKIT, "Called");
        if (!ConvertFromJsValue(env, info.argv[ARGC_ONE], moduleName)) {
            TAG_LOGE(AAFwkTag::APPKIT, "Parse failed");
            ThrowInvalidParamError(env, "Parse param moduleName failed, moduleName must be string.");
            return CreateJsUndefined(env);
        }
    } else {
        TAG_LOGD(AAFwkTag::APPKIT, "Called");
        if (!CheckCallerIsSystemApp()) {
            TAG_LOGE(AAFwkTag::APPKIT, "no system app");
            ThrowNotSystemAppError(env);
            return CreateJsUndefined(env);
        }

        if (!CheckCallerPermission(PERMISSION_GET_BUNDLE_INFO)) {
            TAG_LOGE(AAFwkTag::APPKIT, "no permission");
            ThrowNoPermissionError(env, PERMISSION_GET_BUNDLE_INFO);
            return CreateJsUndefined(env);
        }

        if (!ConvertFromJsValue(env, info.argv[ARGC_TWO], moduleName)
            || !ConvertFromJsValue(env, info.argv[ARGC_ONE], bundleName)) {
            TAG_LOGE(AAFwkTag::APPKIT, "Parse failed");
            ThrowInvalidParamError(env, "Parse param failed, moduleName and bundleName must be string.");
            return CreateJsUndefined(env);
        }
    }
    TAG_LOGD(AAFwkTag::APPKIT, "moduleName: %{public}s, bundlename: %{public}s",
        moduleName.c_str(), bundleName.c_str());
    NapiAsyncTask::ExecuteCallback execute = [moduleName, bundleName, contextImpl,
        moduleContext, inputContextPtr]() {
        if (bundleName.empty()) {
            *moduleContext = contextImpl->CreateModuleContext(moduleName, inputContextPtr);
        } else {
            *moduleContext = contextImpl->CreateModuleContext(bundleName, moduleName, inputContextPtr);
        }
    };

    NapiAsyncTask::CompleteCallback complete;
    SetCreateCompleteCallback(moduleContext, complete);

    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsApplication::OnCreateModuleContext",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));

    return result;
}

bool JsApplication::CheckCallerIsSystemApp()
{
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        return false;
    }
    return true;
}

bool JsApplication::CheckCallerPermission(const std::string &permission)
{
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    int ret = Security::AccessToken::AccessTokenKit::VerifyAccessToken(selfToken, permission);
    if (ret != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        return false;
    }
    return true;
}


napi_value JsApplication::OnCreateBundleContext(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Called");
    if (!CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::APPKIT, "no system app");
        ThrowNotSystemAppError(env);
        return CreateJsUndefined(env);
    }

    if (info.argc < ARGC_TWO) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid argc");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    if (!CheckCallerPermission(PERMISSION_GET_BUNDLE_INFO)) {
        TAG_LOGE(AAFwkTag::APPKIT, "no permission");
        ThrowNoPermissionError(env, PERMISSION_GET_BUNDLE_INFO);
        return CreateJsUndefined(env);
    }

    bool stageMode = false;
    napi_status status = OHOS::AbilityRuntime::IsStageContext(env, info.argv[ARGC_ZERO], stageMode);
    if (status != napi_ok || !stageMode) {
        TAG_LOGE(AAFwkTag::APPKIT, "not stageMode");
        ThrowInvalidParamError(env, "Parse param context failed, must be a context of stageMode.");
        return CreateJsUndefined(env);
    }

    auto context = OHOS::AbilityRuntime::GetStageModeContext(env, info.argv[ARGC_ZERO]);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        ThrowInvalidParamError(env, "Parse param context failed, must not be nullptr.");
        return CreateJsUndefined(env);
    }

    auto inputContextPtr = Context::ConvertTo<Context>(context);
    if (inputContextPtr == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Convert to context failed");
        ThrowInvalidParamError(env, "Parse param context failed, must be a context.");
        return CreateJsUndefined(env);
    }

    std::string bundleName;
    if (!ConvertFromJsValue(env, info.argv[ARGC_ONE], bundleName)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse bundleName failed");
        ThrowInvalidParamError(env, "Parse param bundleName failed, bundleName must be string.");
        return CreateJsUndefined(env);
    }

    auto bundleContext = std::make_shared<std::shared_ptr<Context>>();
    std::shared_ptr<ContextImpl> contextImpl = std::make_shared<ContextImpl>();
    contextImpl->SetProcessName(context->GetProcessName());

    NapiAsyncTask::ExecuteCallback execute = [bundleName, contextImpl,
        bundleContext, inputContextPtr]() {
        contextImpl->CreateBundleContext(*bundleContext, bundleName, inputContextPtr);
    };

    NapiAsyncTask::CompleteCallback complete;
    SetCreateCompleteCallback(bundleContext, complete);

    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsApplication::OnCreateBundleContext",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));

    return result;
}

void JsApplication::SetCreateCompleteCallback(std::shared_ptr<std::shared_ptr<Context>> contextPtr,
    NapiAsyncTask::CompleteCallback &complete)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Called");
    complete = [contextPtr](napi_env env, NapiAsyncTask &task, int32_t status) {
        auto context = *contextPtr;
        if (!context) {
            TAG_LOGE(AAFwkTag::APPKIT, "failed to create context");
            task.Reject(env, CreateJsError(env, 
                static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM), "invalid param."));
            return;
        }
        napi_value value = CreateJsBaseContext(env, context, true);
        auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.Context", &value, 1);
        if (systemModule == nullptr) {
            TAG_LOGW(AAFwkTag::APPKIT, "invalid systemModule");
            task.Reject(env, CreateJsError(env,
                static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM), "invalid param."));
            return;
        }

        napi_value object = systemModule->GetNapiValue();
        if (!CheckTypeForNapiValue(env, object, napi_object)) {
            TAG_LOGE(AAFwkTag::APPKIT, "Failed to get object");
            task.Reject(env, CreateJsError(env,
                static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM), "invalid param."));
            return;
        }

        auto workContext = new (std::nothrow) std::weak_ptr<Context>(context);
        napi_coerce_to_native_binding_object(env, object, DetachCallbackFunc, AttachBaseContext, workContext, nullptr);
        napi_status ret = napi_wrap(env, object, workContext,
            [](napi_env, void *data, void *) {
                TAG_LOGD(AAFwkTag::APPKIT, "Finalizer for weak_ptr module context is called");
                delete static_cast<std::weak_ptr<Context> *>(data);
            },
            nullptr, nullptr);
        if (ret != napi_ok && workContext != nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "napi_wrap Failed: %{public}d", ret);
            delete workContext;
            return;
        }
        task.ResolveWithNoError(env, object);
    };
}

napi_value JsApplication::CreateJsContext(napi_env env, const std::shared_ptr<Context> &context)
{
    napi_value value = CreateJsBaseContext(env, context, true);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.Context", &value, 1);
    if (systemModule == nullptr) {
        TAG_LOGW(AAFwkTag::APPKIT, "invalid systemModule");
        ThrowInvalidParamError(env, "invalid param.");
        return CreateJsUndefined(env);
    }
    napi_value object = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(env, object, napi_object)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get object");
        ThrowInvalidParamError(env, "invalid param.");
        return CreateJsUndefined(env);
    }

    auto workContext = new (std::nothrow) std::weak_ptr<Context>(context);
    napi_coerce_to_native_binding_object(env, object, DetachCallbackFunc, AttachBaseContext, workContext, nullptr);
    napi_status status = napi_wrap(env, object, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGD(AAFwkTag::APPKIT, "Finalizer for weak_ptr module context is called");
            delete static_cast<std::weak_ptr<Context> *>(data);
        },
        nullptr, nullptr);
    if (status != napi_ok && workContext != nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "napi_wrap Failed: %{public}d", status);
        delete workContext;
        ThrowInvalidParamError(env, "invalid param.");
        return CreateJsUndefined(env);
    }

    return object;
}

napi_value JsApplication::PromoteCurrentToCandidateMasterProcess(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    GET_NAPI_INFO_AND_CALL(env, info, JsApplication, OnPromoteCurrentToCandidateMasterProcess);
}

napi_value JsApplication::OnPromoteCurrentToCandidateMasterProcess(napi_env env, NapiCallbackInfo& info)
{
    // only support one params
    if (info.argc == ARGC_ZERO) {
        TAG_LOGE(AAFwkTag::APPKIT, "Not enough params");
        ThrowInvalidParamError(env, "Not enough params");
        return CreateJsUndefined(env);
    }
    
    bool isInsertToHead = false;
    if (!ConvertFromJsValue(env, info.argv[ARGC_ZERO], isInsertToHead)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse isInsertToHead failed");
        ThrowInvalidParamError(env,
            "Parse param isSupport failed, isInsertToHead must be boolean.");
        return CreateJsUndefined(env);
    }

    auto errCode = std::make_shared<int32_t>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute = [isInsertToHead, errCode]() {
        auto appMgrClient = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
        if (appMgrClient == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "Null appMgrClient");
            *errCode = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
        }
        *errCode = appMgrClient->PromoteCurrentToCandidateMasterProcess(isInsertToHead);
    };
    NapiAsyncTask::CompleteCallback complete = [errCode](napi_env env, NapiAsyncTask& task, int32_t status) {
        if (*errCode == ERR_OK) {
            TAG_LOGD(AAFwkTag::APPKIT, "promote to standby master process success");
            task.ResolveWithNoError(env, CreateJsUndefined(env));
            return ;
        }
        task.Reject(env, CreateJsErrorByNativeErr(env, *errCode));
    };
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsApplication::OnPromoteCurrentToCandidateMasterProcess",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsApplication::DemoteCurrentFromCandidateMasterProcess(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    GET_NAPI_INFO_AND_CALL(env, info, JsApplication, OnDemoteCurrentFromCandidateMasterProcess);
}

napi_value JsApplication::OnDemoteCurrentFromCandidateMasterProcess(napi_env env, NapiCallbackInfo& info)
{
    auto errCode = std::make_shared<int32_t>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute = [errCode]() {
    auto appMgrClient = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
        if (appMgrClient == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "Null appMgrClient");
            *errCode = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
        }
        *errCode = appMgrClient->DemoteCurrentFromCandidateMasterProcess();
    };
    NapiAsyncTask::CompleteCallback complete = [errCode](napi_env env, NapiAsyncTask& task, int32_t status) {
        if (*errCode == ERR_OK) {
            TAG_LOGD(AAFwkTag::APPKIT, "demote to standby master process success");
            task.ResolveWithNoError(env, CreateJsUndefined(env));
            return ;
        }
        task.Reject(env, CreateJsErrorByNativeErr(env, *errCode));
    };
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsApplication::OnDemoteCurrentFromCandidateMasterProcess",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value ApplicationInit(napi_env env, napi_value exportObj)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Called");
    if (env == nullptr || exportObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env or exportObj");
        return nullptr;
    }

    auto jsApplication = std::make_unique<JsApplication>();
    napi_wrap(env, exportObj, jsApplication.release(), JsApplication::Finalizer, nullptr, nullptr);

    const char *moduleName = "application";
    BindNativeFunction(env, exportObj, "getApplicationContext", moduleName,
        JsApplication::GetApplicationContext);

    BindNativeFunction(env, exportObj, "createModuleContext", moduleName,
        JsApplication::CreateModuleContext);

    BindNativeFunction(env, exportObj, "createBundleContext", moduleName,
        JsApplication::CreateBundleContext);

    BindNativeFunction(env, exportObj, "createPluginModuleContext", moduleName,
        JsApplication::CreatePluginModuleContext);

    BindNativeFunction(env, exportObj, "promoteCurrentToCandidateMasterProcess", moduleName,
        JsApplication::PromoteCurrentToCandidateMasterProcess);
    
    BindNativeFunction(env, exportObj, "demoteCurrentFromCandidateMasterProcess", moduleName,
        JsApplication::DemoteCurrentFromCandidateMasterProcess);

    return CreateJsUndefined(env);
}
} // namespace AbilityRuntime
} // namespace OHOS