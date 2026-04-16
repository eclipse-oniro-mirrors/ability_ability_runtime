/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "ani.h"

#include <cstdint>
#include <unistd.h>
#include <memory>
#include <mutex>
#include "ability_business_error.h"
#include "application_data_manager.h"
#include "event_runner.h"
#include "hilog_tag_wrapper.h"
#include "ierror_observer.h"
#include "event_runner.h"
#include "ani_common_util.h"
#include "ets_error_utils.h"
#include "error_manager_ani_util.h"
#include "app_recovery.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
struct ObserverItem {
    ani_ref ref;
    ani_vm* vm;
};
static ObserverItem g_freezeObserver;
static ObserverItem g_defaultHandler;
static std::mutex g_defaultHandlerMtx;
static std::mutex g_freezeMtx;
static bool g_freezeCallbackRegistered = false;
static ObserverItem g_defaultFreezeObserver;
static std::mutex g_defaultFreezeMtx;
static std::set<ani_ref> g_unhandledRejectionObservers;
static std::mutex g_unhandledRejectionMtx;
static int64_t g_lastWatchTimeReport = 0;
static int64_t ONE_MINUTES_DELAY_TIMER = 60 * 1000;
} // namespace

class ErrorManagerAni final {
public:
    ErrorManagerAni() {}
    ~ErrorManagerAni() = default;

    static void Finalizer(ani_env *env, void* data, void* hint)
    {
        TAG_LOGI(AAFwkTag::JSNAPI, "finalizer called");
        std::unique_ptr<ErrorManagerAni>(static_cast<ErrorManagerAni*>(data));
        ClearReference(env);
    }

    static void ClearReference(ani_env *env)
    {
        std::lock_guard<std::mutex> lock(g_unhandledRejectionMtx);
        for (auto& iter : g_unhandledRejectionObservers) {
            env->GlobalReference_Delete(iter);
        }
        g_unhandledRejectionObservers.clear();
    }

    static void DoErrorCallback(const AppExecFwk::ErrorObject &errorObj)
    {
        std::lock_guard<std::mutex> lock(g_defaultHandlerMtx);
        if (g_defaultHandler.vm == nullptr) {
            TAG_LOGE(AAFwkTag::JSNAPI, "null vm or defaultHandler ref.");
            return;
        }
        ani_env *env = nullptr;
        bool isAttachThread = false;
        env = AppExecFwk::AttachAniEnv(g_defaultHandler.vm, isAttachThread);
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::JSNAPI, "null env");
            return;
        }
        if (g_defaultHandler.ref == nullptr || IsRefUndefined(env, g_defaultHandler.ref) ||
            IsNull(env, g_defaultHandler.ref)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "invalid defaultHandler ref.");
            return;
        }
        ani_object error = CreateErrorObject(env, errorObj.name, errorObj.message, errorObj.stack);
        if (error == nullptr) {
            TAG_LOGE(AAFwkTag::JSNAPI, "null error param");
            return;
        }
        std::vector<ani_ref> args = {error};
        ani_ref result = nullptr;
        ani_status status = env->FunctionalObject_Call(
            reinterpret_cast<ani_fn_object>(g_defaultHandler.ref), args.size(), args.data(), &result);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::JSNAPI, "failed to call default handler function, status: %{public}d", status);
        }
        AppExecFwk::DetachAniEnv(g_defaultHandler.vm, isAttachThread);
        TAG_LOGD(AAFwkTag::JSNAPI, "doErrorCallback end.");
    }

    static void NotifyUnhandledRejectionHandler(ani_env *env, ani_object reason, ani_object promise)
    {
        std::lock_guard<std::mutex> lock(g_unhandledRejectionMtx);
        for (auto& iter : g_unhandledRejectionObservers) {
            ani_object callback = static_cast<ani_object>(iter);
            if (!ValidateFunction(env, callback) || !ValidateFunction(env, promise) ||
                !ValidateFunction(env, reason)) {
                TAG_LOGE(AAFwkTag::JSNAPI, "UnhandledRejection callback, promise or reason invalid.");
                return;
            }
            std::vector<ani_ref> args = {reason, promise};
            ani_ref result = nullptr;
            ani_status status = env->FunctionalObject_Call(
                reinterpret_cast<ani_fn_object>(callback), args.size(), args.data(), &result);
            if (status != ANI_OK) {
                TAG_LOGE(AAFwkTag::JSNAPI, "failed to call unhandled function, status: %{public}d", status);
            }
        }
    }

    static ani_object SetDefaultErrorHandler(ani_env *env, ani_object function)
    {
        ani_object result = nullptr;
        if (!AppExecFwk::EventRunner::IsAppMainThread()) {
            TAG_LOGE(AAFwkTag::JSNAPI, "not mainThread");
            EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_MAIN_THREAD);
            return result;
        }
        if (IsRefUndefined(env, function)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "invalid func");
            EtsErrorUtil::ThrowInvalidNumParametersError(env);
            return result;
        }
        if (IsNull(env, function)) {
            function = nullptr;
        }
        std::lock_guard<std::mutex> lock(g_defaultHandlerMtx);
        if (g_defaultHandler.ref) {
            ani_wref weakRef;
            auto status = env->WeakReference_Create(g_defaultHandler.ref, &weakRef);
            if (status != ANI_OK) {
                TAG_LOGE(AAFwkTag::JSNAPI, "create weakref failed, status: %{public}d", status);
                return result;
            }
            ani_boolean released = ANI_FALSE;
            ani_ref weakResult = nullptr;
            status = env->WeakReference_GetReference(weakRef, &released, &weakResult);
            if (status != ANI_OK) {
                TAG_LOGE(AAFwkTag::JSNAPI, "create weakref failed, "
                    "status: %{public}d, released: %{public}d", status, released);
                return result;
            }
            result = static_cast<ani_object>(weakResult);
        }
        ani_ref objectRef = nullptr;
        if (function) {
            auto status = env->GlobalReference_Create(function, &objectRef);
            if (status != ANI_OK) {
                TAG_LOGE(AAFwkTag::JSNAPI, "create defaultHandler function failed.");
                return result;
            }
        }
        if (g_defaultHandler.ref) {
            env->GlobalReference_Delete(g_defaultHandler.ref);
            g_defaultHandler.ref = nullptr;
        }
        g_defaultHandler.ref = objectRef;
        g_defaultHandler.vm = GetAniVm(env);
        return result;
    }

    static ani_object SetDefaultFreezeObserver(ani_env *env, ani_object function)
    {
        ani_object result = nullptr;
        if (!CheckDefaultFreezeError(env, function)) {
            return result;
        }
        std::lock_guard<std::mutex> lock(g_defaultFreezeMtx);
        if (g_defaultFreezeObserver.ref) {
            ani_wref weakRef;
            auto status = env->WeakReference_Create(g_defaultFreezeObserver.ref, &weakRef);
            if (status != ANI_OK) {
                TAG_LOGE(AAFwkTag::JSNAPI, "create weakref failed, status: %{public}d", status);
                return result;
            }
            ani_boolean released = ANI_FALSE;
            ani_ref weakResult = nullptr;
            status = env->WeakReference_GetReference(weakRef, &released, &weakResult);
            if (status != ANI_OK) {
                TAG_LOGE(AAFwkTag::JSNAPI, "create weakref failed, "
                    "status: %{public}d, released: %{public}d", status, released);
                return result;
            }
            result = static_cast<ani_object>(weakResult);
        }
        ani_ref objectRef = nullptr;
        if (function) {
            auto status = env->GlobalReference_Create(function, &objectRef);
            if (status != ANI_OK) {
                TAG_LOGE(AAFwkTag::JSNAPI, "create defaultHandler function failed.");
                return result;
            }
        }
        if (g_defaultFreezeObserver.ref) {
            env->GlobalReference_Delete(g_defaultFreezeObserver.ref);
            g_defaultFreezeObserver.ref = nullptr;
        }
        g_defaultFreezeObserver.ref = objectRef;
        g_defaultFreezeObserver.vm = GetAniVm(env);
        if (!g_freezeCallbackRegistered) {
            AppExecFwk::AppRecovery::GetInstance().SetFreezeCallback(FreezeCallback);
            g_freezeCallbackRegistered = true;
            TAG_LOGI(AAFwkTag::JSNAPI, "Freeze callback registered to AppRecovery successfully");
        }
        return result;
    }

    static bool CheckDefaultFreezeError(ani_env *env, ani_object function)
    {
        if (!AppExecFwk::EventRunner::IsAppMainThread()) {
            TAG_LOGE(AAFwkTag::JSNAPI, "not mainThread");
            EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_MAIN_THREAD);
            return false;
        }
        if (IsNull(env, function)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "func is null.");
            EtsErrorUtil::ThrowInvalidNumParametersError(env);
            return false;
        }
        if (IsRefUndefined(env, function)) {
            TAG_LOGI(AAFwkTag::JSNAPI, "func is undefined.");
        }
        return true;
    }

    static bool CheckReportDuration()
    {
        int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
            steady_clock::now().time_since_epoch()).count();
        if ((now - g_lastWatchTimeReport > ONE_MINUTES_DELAY_TIMER) || (now - g_lastWatchTimeReport < 0)) {
            g_lastWatchTimeReport = now;
            return true;
        } else {
            TAG_LOGW(AAFwkTag::JSNAPI, "reporting once per minute.");
            return false;
        }
    }

    static void OnFreezeCallback()
    {
        TAG_LOGD(AAFwkTag::JSNAPI, "FreezeCallback begin");
        std::lock_guard<std::mutex> lock(g_freezeMtx);
        ani_env *env = nullptr;
        bool isAttachThread = false;
        env = AppExecFwk::AttachAniEnv(g_freezeObserver.vm, isAttachThread);
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::JSNAPI, "null env");
            return;
        }
        if (g_freezeObserver.ref == nullptr) {
            TAG_LOGE(AAFwkTag::JSNAPI, "null freezeObserver ref");
            return;
        }
        std::vector<ani_ref> args = {};
        ani_ref result{};
        ani_status status = env->FunctionalObject_Call(
            reinterpret_cast<ani_fn_object>(g_freezeObserver.ref), 0, args.data(), &result);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::JSNAPI, "failed to call function, status: %{public}d", status);
        }
        AppExecFwk::DetachAniEnv(g_freezeObserver.vm, isAttachThread);
        TAG_LOGD(AAFwkTag::JSNAPI, "FreezeCallback end");
    }

    static void DefaultFreezeCallback()
    {
        TAG_LOGD(AAFwkTag::JSNAPI, "DefaultFreezeCallback begin");
        std::lock_guard<std::mutex> lock(g_defaultFreezeMtx);
        ani_env *env = nullptr;
        bool isAttachThread = false;
        env = AppExecFwk::AttachAniEnv(g_defaultFreezeObserver.vm, isAttachThread);
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::JSNAPI, "null env");
            return;
        }
        if (g_defaultFreezeObserver.ref == nullptr) {
            TAG_LOGE(AAFwkTag::JSNAPI, "null freezeObserver ref");
            return;
        }
        std::vector<ani_ref> args = {};
        ani_ref result{};
        ani_status status = env->FunctionalObject_Call(
            reinterpret_cast<ani_fn_object>(g_defaultFreezeObserver.ref), 0, args.data(), &result);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::JSNAPI, "failed to call function, status: %{public}d", status);
        }
        AppExecFwk::DetachAniEnv(g_defaultFreezeObserver.vm, isAttachThread);
        TAG_LOGD(AAFwkTag::JSNAPI, "DefaultFreezeCallback end");
    }

    static void FreezeCallback()
    {
        if (!CheckReportDuration()) {
            return;
        }
        OnFreezeCallback();
        DefaultFreezeCallback();
    }

    static ani_object OnFreeze(ani_env *env, ani_object function)
    {
        ani_object result{};
        if (!AppExecFwk::EventRunner::IsAppMainThread()) {
            TAG_LOGE(AAFwkTag::JSNAPI, "not mainThread");
            EtsErrorUtil::ThrowInvalidCallerError(env);
            return result;
        }
        if (!ValidateFunction(env, function)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "invalid argc");
            return result;
        }
        std::lock_guard<std::mutex> lock(g_freezeMtx);
        if (g_freezeObserver.ref) {
            env->GlobalReference_Delete(g_freezeObserver.ref);
            g_freezeObserver.ref = nullptr;
        }
        if (function) {
            auto status = env->GlobalReference_Create(function, &g_freezeObserver.ref);
            if (status != ANI_OK) {
                TAG_LOGE(AAFwkTag::JSNAPI, "create freeze function failed.");
                return result;
            }
        }
        g_freezeObserver.vm = GetAniVm(env);
        if (!g_freezeCallbackRegistered) {
            AppExecFwk::AppRecovery::GetInstance().SetFreezeCallback(FreezeCallback);
            g_freezeCallbackRegistered = true;
            TAG_LOGI(AAFwkTag::JSNAPI, "Freeze callback registered to AppRecovery successfully");
        }
        return result;
    }

    static ani_object OffFreeze(ani_env *env, ani_object function)
    {
        ani_object result{};
        if (!AppExecFwk::EventRunner::IsAppMainThread()) {
            TAG_LOGE(AAFwkTag::JSNAPI, "not mainThread");
            EtsErrorUtil::ThrowInvalidCallerError(env);
            return result;
        }
        std::lock_guard<std::mutex> lock(g_freezeMtx);
        if (g_freezeObserver.ref == nullptr) {
            TAG_LOGE(AAFwkTag::JSNAPI, "null freezeObserver");
            return result;
        }

        if (function == nullptr) {
            env->GlobalReference_Delete(g_freezeObserver.ref);
            g_freezeObserver.ref = nullptr;
            g_freezeObserver = {};
            if (g_freezeCallbackRegistered) {
                AppExecFwk::AppRecovery::GetInstance().SetFreezeCallback(nullptr);
                g_freezeCallbackRegistered = false;
                TAG_LOGI(AAFwkTag::JSNAPI, "Freeze callback unregistered from AppRecovery successfully");
            }
            return result;
        }
        if (!ValidateFunction(env, function)) {
            return result;
        }
        ani_object observer = static_cast<ani_object>(g_freezeObserver.ref);
        ani_boolean equals = false;
        env->Reference_StrictEquals(observer, function, &equals);
        if (equals) {
            env->GlobalReference_Delete(g_freezeObserver.ref);
            g_freezeObserver.ref = nullptr;
            g_freezeObserver = {};
            if (g_freezeCallbackRegistered) {
                AppExecFwk::AppRecovery::GetInstance().SetFreezeCallback(nullptr);
                g_freezeCallbackRegistered = false;
                TAG_LOGI(AAFwkTag::JSNAPI, "Freeze callback unregistered from AppRecovery successfully");
            }
            return result;
        }
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_OBSERVER_NOT_FOUND);
        return result;
    }

    static ani_object OnUnhandledRejectionInner(ani_env *env, ani_object function)
    {
        ani_object result{};
        if (!ValidateFunction(env, function)) {
            return result;
        }
        std::lock_guard<std::mutex> lock(g_unhandledRejectionMtx);
        for (auto& iter : g_unhandledRejectionObservers) {
            ani_object observer = static_cast<ani_object>(iter);
            ani_boolean equals = false;
            env->Reference_StrictEquals(observer, function, &equals);
            if (equals) {
                env->GlobalReference_Delete(iter);
                g_unhandledRejectionObservers.erase(iter);
                break;
            }
        }
        ani_ref ref = nullptr;
        if (function) {
            auto status = env->GlobalReference_Create(function, &ref);
            if (status != ANI_OK) {
                TAG_LOGE(AAFwkTag::JSNAPI, "create unhandledRejection function failed.");
                return result;
            }
        }
        g_unhandledRejectionObservers.insert(ref);
        return result;
    }

    static ani_object OffUnhandledRejection(ani_env *env, ani_object function)
    {
        ani_object result{};
        std::lock_guard<std::mutex> lock(g_unhandledRejectionMtx);
        if (function == nullptr) {
            for (auto& iter : g_unhandledRejectionObservers) {
                env->GlobalReference_Delete(iter);
            }
            g_unhandledRejectionObservers.clear();
            return result;
        }

        if (!ValidateFunction(env, function)) {
            return result;
        }

        for (auto& iter : g_unhandledRejectionObservers) {
            ani_object observer = static_cast<ani_object>(iter);
            ani_boolean equals = false;
            env->Reference_StrictEquals(observer, function, &equals);
            if (equals) {
                env->GlobalReference_Delete(iter);
                g_unhandledRejectionObservers.erase(iter);
                return result;
            }
        }
        TAG_LOGE(AAFwkTag::JSNAPI, "remove unhandle observer failed");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_OBSERVER_NOT_FOUND);
        return result;
    }

    static bool ValidateFunction(ani_env *env, ani_object function)
    {
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::JSNAPI, "null env");
            return false;
        }
        if (function == nullptr || IsRefUndefined(env, function) || IsNull(env, function)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "invalid func");
            EtsErrorUtil::ThrowInvalidNumParametersError(env);
            return false;
        }
        return true;
    }

    static bool IsErrorObserverListNotEmpty()
    {
        std::lock_guard<std::mutex> lock(g_defaultHandlerMtx);
        return g_defaultHandler.ref == nullptr ? false : true;
    }
};

static void EtsErrorManagerInit(ani_env *env)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "ErrorManager ets called.");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null env");
        return;
    }
    ani_status status = ANI_ERROR;
    if (env->ResetError() != ANI_OK) {
        TAG_LOGE(AAFwkTag::JSNAPI, "ResetError failed");
    }

    ani_namespace ns;
    status = env->FindNamespace("@ohos.app.ability.errorManager.errorManager", &ns);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::JSNAPI, "FindNamespace errorManager failed status: %{public}d", status);
        return;
    }

    std::array kitFunctions = {
        ani_native_function {"setDefaultErrorHandler", nullptr, reinterpret_cast<void *>(
            ErrorManagerAni::SetDefaultErrorHandler)},
        ani_native_function {"onFreeze", nullptr, reinterpret_cast<void *>(ErrorManagerAni::OnFreeze)},
        ani_native_function {"offFreeze", nullptr, reinterpret_cast<void *>(ErrorManagerAni::OffFreeze)},
        ani_native_function {"notifyUnhandledRejectionHandler", nullptr, reinterpret_cast<void *>(
            ErrorManagerAni::NotifyUnhandledRejectionHandler)},
        ani_native_function {"onUnhandledRejectionInner", nullptr, reinterpret_cast<void *>(
            ErrorManagerAni::OnUnhandledRejectionInner)},
        ani_native_function {"offUnhandledRejection", nullptr, reinterpret_cast<void *>(
            ErrorManagerAni::OffUnhandledRejection)},
        ani_native_function {"setDefaultFreezeObserver", nullptr, reinterpret_cast<void *>(
            ErrorManagerAni::SetDefaultFreezeObserver)},
    };

    status = env->Namespace_BindNativeFunctions(ns, kitFunctions.data(), kitFunctions.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::JSNAPI, "Namespace_BindNativeFunctions failed status: %{public}d", status);
    }

    if (env->ResetError() != ANI_OK) {
        TAG_LOGE(AAFwkTag::JSNAPI, "ResetError failed");
    }
    TAG_LOGD(AAFwkTag::JSNAPI, "ErrorManager ets called end");
}


}  // namespace AbilityRuntime
}  // namespace OHOS

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "ANI_Constructor start.");
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    if (vm == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null vm");
        return ANI_ERROR;
    }
    status = vm->GetEnv(ANI_VERSION_1, &env);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::JSNAPI, "GetEnv failed status: %{public}d", status);
        return ANI_NOT_FOUND;
    }

    OHOS::AbilityRuntime::EtsErrorManagerInit(env);
    *result = ANI_VERSION_1;
    OHOS::AppExecFwk::ApplicationDataManager::GetInstance().SetErrorHandlerCallback(
        OHOS::AbilityRuntime::ErrorManagerAni::DoErrorCallback);
    OHOS::AppExecFwk::ApplicationDataManager::GetInstance().RegisterHasOnErrorCallback(
        OHOS::AbilityRuntime::ErrorManagerAni::IsErrorObserverListNotEmpty);

    TAG_LOGD(AAFwkTag::JSNAPI, "ANI_Constructor finish");
    return ANI_OK;
}
