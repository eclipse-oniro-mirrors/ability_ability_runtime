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

#include "js_application.h"

#include "context_impl.h"
#include "hilog_tag_wrapper.h"
#include "js_context_utils.h"
#include "js_error_utils.h"
#include "js_runtime.h"
#include "napi_base_context.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;
constexpr const char *PERMISSION_GET_BUNDLE_INFO = "ohos.permission.GET_BUNDLE_INFO_PRIVILEGED";
} // namespace

void JsApplication::Finalizer(napi_env env, void *data, void *hint)
{
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "Called.");
    std::unique_ptr<JsApplication>(static_cast<JsApplication *>(data));
}

napi_value JsApplication::GetApplicationContext(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value JsApplication::CreateBundleContext(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value JsApplication::CreateModuleContext(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsApplication, OnCreateModuleContext);
}

napi_value JsApplication::OnCreateModuleContext(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "Called");
    if (info.argc < ARGC_TWO) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "invalid argc");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    bool stageMode = false;
    napi_status status = OHOS::AbilityRuntime::IsStageContext(env, info.argv[ARGC_ZERO], stageMode);
    if (status != napi_ok || !stageMode) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "not stageMode");
        ThrowInvalidParamError(env, "Parse param context failed, must be a context of stageMode.");
        return CreateJsUndefined(env);
    }

    auto context = OHOS::AbilityRuntime::GetStageModeContext(env, info.argv[ARGC_ZERO]);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "null context");
        ThrowInvalidParamError(env, "Parse param context failed, must not be nullptr.");
        return CreateJsUndefined(env);
    }

    auto inputContextPtr = Context::ConvertTo<Context>(context);
    if (inputContextPtr == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "Convert to context failed");
        ThrowInvalidParamError(env, "Parse param context failed, must be a context.");
        return CreateJsUndefined(env);
    }

    std::shared_ptr<std::shared_ptr<Context>> moduleContext = std::make_shared<std::shared_ptr<Context>>();
    std::shared_ptr<ContextImpl> contextImpl = std::make_shared<ContextImpl>();
    if (contextImpl == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "null contextImpl");
        ThrowInvalidParamError(env, "create context failed.");
        return CreateJsUndefined(env);
    }
    std::string moduleName = "";
    std::string bundleName = "";
    if (info.argc == ARGC_TWO) {
        TAG_LOGD(AAFwkTag::ABILITY_SIM, "Called");
        if (!ConvertFromJsValue(env, info.argv[ARGC_ONE], moduleName)) {
            TAG_LOGE(AAFwkTag::ABILITY_SIM, "Parse failed");
            ThrowInvalidParamError(env, "Parse param moduleName failed, moduleName must be string.");
            return CreateJsUndefined(env);
        }
    } else {
        TAG_LOGD(AAFwkTag::ABILITY_SIM, "Called");
        if (!CheckCallerIsSystemApp()) {
            TAG_LOGE(AAFwkTag::ABILITY_SIM, "no system app");
            ThrowNotSystemAppError(env);
            return CreateJsUndefined(env);
        }

        if (!CheckCallerPermission(PERMISSION_GET_BUNDLE_INFO)) {
            TAG_LOGE(AAFwkTag::ABILITY_SIM, "no permission");
            ThrowNoPermissionError(env, PERMISSION_GET_BUNDLE_INFO);
            return CreateJsUndefined(env);
        }

        if (!ConvertFromJsValue(env, info.argv[ARGC_TWO], moduleName)
            || !ConvertFromJsValue(env, info.argv[ARGC_ONE], bundleName)) {
            TAG_LOGE(AAFwkTag::ABILITY_SIM, "Parse failed");
            ThrowInvalidParamError(env, "Parse param failed, moduleName and bundleName must be string.");
            return CreateJsUndefined(env);
        }
    }
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "moduleName: %{public}s, bundlename: %{public}s",
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

napi_value JsApplication::CreatePluginModuleContext(napi_env env, napi_callback_info info)
{
    return nullptr;
}


bool JsApplication::CheckCallerIsSystemApp()
{
    return true;
}

bool JsApplication::CheckCallerPermission(const std::string &permission)
{
    return true;
}

void JsApplication::SetCreateCompleteCallback(std::shared_ptr<std::shared_ptr<Context>> contextPtr,
    NapiAsyncTask::CompleteCallback &complete)
{
    complete = [contextPtr](napi_env env, NapiAsyncTask &task, int32_t status) {
        auto context = *contextPtr;
        if (context == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITY_SIM, "failed to create context");
            task.Reject(env, CreateJsError(env,
                static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM), "invalid param."));
            return;
        }
        napi_value value = CreateJsBaseContext(env, context, true);
        auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.Context", &value, 1);
        if (systemModule == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITY_SIM, "invalid systemModule");
            task.Reject(env, CreateJsError(env,
                static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM), "invalid param."));
            return;
        }

        napi_value object = systemModule->GetNapiValue();
        if (!CheckTypeForNapiValue(env, object, napi_object)) {
            TAG_LOGE(AAFwkTag::ABILITY_SIM, "Failed to get object");
            task.Reject(env, CreateJsError(env,
                static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM), "invalid param."));
            return;
        }

        auto workContext = new (std::nothrow) std::weak_ptr<Context>(context);
        napi_coerce_to_native_binding_object(env, object, DetachCallbackFunc, AttachBaseContext, workContext, nullptr);
        napi_status ret = napi_wrap(env, object, workContext,
            [](napi_env, void *data, void *) {
                TAG_LOGD(AAFwkTag::ABILITY_SIM, "Finalizer for weak_ptr module context is called");
                delete static_cast<std::weak_ptr<Context> *>(data);
            },
            nullptr, nullptr);
        if (ret != napi_ok && workContext != nullptr) {
            TAG_LOGE(AAFwkTag::ABILITY_SIM, "napi_wrap Failed: %{public}d", ret);
            delete workContext;
            return;
        }
        task.ResolveWithNoError(env, object);
    };
}

napi_value JsApplication::OnCreateBundleContext(napi_env env, NapiCallbackInfo &info)
{
    return nullptr;
}

napi_value JsApplication::OnCreatePluginModuleContext(napi_env env, NapiCallbackInfo &info)
{
    return nullptr;
}

napi_value JsApplication::CreateJsContext(napi_env env, const std::shared_ptr<Context> &context)
{
    return nullptr;
}

napi_value JsApplication::OnGetApplicationContext(napi_env env, NapiCallbackInfo &info)
{
    return nullptr;
}

napi_value ApplicationInit(napi_env env, napi_value exportObj)
{
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "Called");
    if (env == nullptr || exportObj == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "null env or exportObj");
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
    return CreateJsUndefined(env);
}
} // namespace AbilityRuntime
} // namespace OHOS