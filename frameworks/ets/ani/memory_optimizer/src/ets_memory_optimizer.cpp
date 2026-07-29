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

#include "ets_memory_optimizer.h"

#include "ability_business_error.h"
#include "ani_common_util.h"
#include "errors.h"
#include "ets_error_utils.h"
#include "hilog_tag_wrapper.h"
#include "madvise_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char *ETS_MEMORY_OPTIMIZER_NAMESPACE = "@ohos.app.ability.appMemoryOptimizer.appMemoryOptimizer";
constexpr const char *SIGNATURE_EVICT_FILE_PAGES =
    "C{std.core.Array}C{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char *SIGNATURE_EVICT_CHECK = "C{std.core.Array}:";
} // namespace

// Runs on the main thread (called before taskpool.execute in .ets). Validates
// system-app / params / extensions and throws synchronously, mirroring the JS
// napi binding where validation happens on the calling thread.
void EtsMemoryOptimizer::EvictFilePagesCheck(ani_env *env, ani_object fileNamesObj)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called EvictFilePagesCheck");
    if (env == nullptr) {
        return;
    }
    std::vector<std::string> fileNames;
    if (!AppExecFwk::UnwrapArrayString(env, fileNamesObj, fileNames)) {
        TAG_LOGE(AAFwkTag::ABILITY, "parse fileNames failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parameter error. Parse fileNames failed.");
        return;
    }
    if (fileNames.empty()) {
        TAG_LOGE(AAFwkTag::ABILITY, "empty fileNames array");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parameter error. Empty fileNames array.");
        return;
    }
    for (const auto &name : fileNames) {
        if (!MadviseUtil::IsValidEvictFileName(name)) {
            TAG_LOGE(AAFwkTag::ABILITY, "invalid file type: %{public}s", name.c_str());
            EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_FILE_TYPE_ERROR);
            return;
        }
    }
}

void EtsMemoryOptimizer::EvictFilePages(ani_env *env, ani_object fileNamesObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called EvictFilePages");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null env");
        return;
    }
    std::vector<std::string> fileNames;
    AppExecFwk::UnwrapArrayString(env, fileNamesObj, fileNames);
    MadviseUtil::EvictFilePages(fileNames);
    AppExecFwk::AsyncCallback(env, callback,
        EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), nullptr);
}

// Runs on the main thread. Validates system-app / params only (extension and
// config checks depend on reading the hap, so they stay in the work function).
void EtsMemoryOptimizer::EvictModuleFilePagesCheck(ani_env *env, ani_object moduleNamesObj)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called EvictModuleFilePagesCheck");
    if (env == nullptr) {
        return;
    }
    std::vector<std::string> moduleNames;
    if (!AppExecFwk::UnwrapArrayString(env, moduleNamesObj, moduleNames)) {
        TAG_LOGE(AAFwkTag::ABILITY, "parse moduleNames failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parameter error. Parse moduleNames failed.");
        return;
    }
    if (moduleNames.empty()) {
        TAG_LOGE(AAFwkTag::ABILITY, "empty moduleNames array");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parameter error. Empty moduleNames array.");
        return;
    }
}

void EtsMemoryOptimizer::EvictModuleFilePages(ani_env *env, ani_object moduleNamesObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called EvictModuleFilePages");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null env");
        return;
    }
    std::vector<std::string> moduleNames;
    AppExecFwk::UnwrapArrayString(env, moduleNamesObj, moduleNames);
    ErrCode ret = MadviseUtil::EvictModuleFilePages(moduleNames);
    AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateErrorByNativeErr(env, ret), nullptr);
}

void EtsMemoryOptimizerInit(ani_env *env)
{
    TAG_LOGD(AAFwkTag::ABILITY, "call EtsMemoryOptimizerInit");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null env");
        return;
    }
    ani_status status = ANI_ERROR;
    if (env->ResetError() != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "ResetError failed");
    }
    ani_namespace ns = nullptr;
    status = env->FindNamespace(ETS_MEMORY_OPTIMIZER_NAMESPACE, &ns);
    if (status != ANI_OK || ns == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "FindNamespace failed status: %{public}d or ns null", status);
        return;
    }
    std::array methods = {
        ani_native_function {"nativeEvictFilePagesCheck", SIGNATURE_EVICT_CHECK,
            reinterpret_cast<void *>(EtsMemoryOptimizer::EvictFilePagesCheck)},
        ani_native_function {"nativeEvictFilePagesSync", SIGNATURE_EVICT_FILE_PAGES,
            reinterpret_cast<void *>(EtsMemoryOptimizer::EvictFilePages)},
        ani_native_function {"nativeEvictModuleFilePagesCheck", SIGNATURE_EVICT_CHECK,
            reinterpret_cast<void *>(EtsMemoryOptimizer::EvictModuleFilePagesCheck)},
        ani_native_function {"nativeEvictModuleFilePagesSync", SIGNATURE_EVICT_FILE_PAGES,
            reinterpret_cast<void *>(EtsMemoryOptimizer::EvictModuleFilePages)},
    };
    status = env->Namespace_BindNativeFunctions(ns, methods.data(), methods.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "Namespace_BindNativeFunctions failed status: %{public}d", status);
        return;
    }
    if (env->ResetError() != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "ResetError failed");
    }
    TAG_LOGD(AAFwkTag::ABILITY, "EtsMemoryOptimizerInit success");
}

extern "C" {
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    TAG_LOGD(AAFwkTag::ABILITY, "in EtsMemoryOptimizer.ANI_Constructor");
    if (vm == nullptr || result == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null vm or result");
        return ANI_INVALID_ARGS;
    }

    ani_env *env = nullptr;
    ani_status status = vm->GetEnv(ANI_VERSION_1, &env);
    if (status != ANI_OK || env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "GetEnv failed, status=%{public}d or null env", status);
        return ANI_NOT_FOUND;
    }
    EtsMemoryOptimizerInit(env);
    *result = ANI_VERSION_1;
    TAG_LOGD(AAFwkTag::ABILITY, "EtsMemoryOptimizer.ANI_Constructor finished");
    return ANI_OK;
}
}
} // namespace AbilityRuntime
} // namespace OHOS
