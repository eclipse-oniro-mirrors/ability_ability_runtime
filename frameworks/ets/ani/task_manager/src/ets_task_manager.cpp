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

#include "ets_task_manager.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

#include "ability_business_error.h"
#include "ability_manager_errors.h"
#include "ani_common_util.h"
#include "ani_enum_convert.h"
#include "errors.h"
#include "ets_error_utils.h"
#include "hilog_tag_wrapper.h"
#include "task_manager_client.h"

namespace OHOS {
namespace TaskManagerEts {
namespace {
constexpr const char *TASK_MANAGER_SPACE_NAME = "@ohos.app.agent.taskManager.taskManager";

// Aligned with the TaskPriority enum of @ohos.app.agent.taskManager and the limits checked by
// the NAPI binding and the service side.
constexpr int32_t TASK_PRIORITY_LOW = 100;
constexpr int32_t TASK_PRIORITY_MEDIUM = 200;
constexpr int32_t TASK_PRIORITY_HIGH = 300;
constexpr size_t TASK_ID_MIN_LEN = 1;
constexpr size_t TASK_ID_MAX_LEN = 256;

bool ConvertTaskId(ani_env *env, ani_string aniTaskId, std::string &taskId)
{
    if (!AppExecFwk::GetStdString(env, aniTaskId, taskId)) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "convert taskId failed");
        return false;
    }
    if (taskId.size() < TASK_ID_MIN_LEN || taskId.size() > TASK_ID_MAX_LEN) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "invalid taskId length: %{public}zu", taskId.size());
        return false;
    }
    return true;
}

bool ConvertPriority(ani_env *env, ani_enum_item aniPriority, int32_t &priority)
{
    if (!AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative<int32_t>(env, aniPriority, priority)) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "convert priority failed");
        return false;
    }
    if (priority != TASK_PRIORITY_LOW && priority != TASK_PRIORITY_MEDIUM && priority != TASK_PRIORITY_HIGH) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "invalid priority: %{public}d", priority);
        return false;
    }
    return true;
}

void CallbackInvalidParam(ani_env *env, ani_object callback)
{
    AppExecFwk::AsyncCallback(env, callback,
        AbilityRuntime::EtsErrorUtil::CreateError(env,
            AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM), nullptr);
}
} // namespace

class EtsTaskManager final {
public:
    EtsTaskManager() = delete;
    ~EtsTaskManager() = delete;

    static void NativeStartTask(ani_env *env, ani_string aniTaskId, ani_enum_item aniPriority,
        ani_object callback);

    static void NativeFinishTask(ani_env *env, ani_string aniTaskId, ani_object callback);
};

void EtsTaskManager::NativeStartTask(ani_env *env, ani_string aniTaskId, ani_enum_item aniPriority,
    ani_object callback)
{
    TAG_LOGD(AAFwkTag::ETSRUNTIME, "called");
    if (env == nullptr || callback == nullptr) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "env or callback is null");
        return;
    }
    std::string taskId;
    int32_t priority = TASK_PRIORITY_MEDIUM;
    if (!ConvertTaskId(env, aniTaskId, taskId) || !ConvertPriority(env, aniPriority, priority)) {
        CallbackInvalidParam(env, callback);
        return;
    }
    int32_t ret = AppExecFwk::TaskManagerClient::GetInstance().StartTask(taskId, priority);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "startTask failed: %{public}d", ret);
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, ret), nullptr);
        return;
    }
    AppExecFwk::AsyncCallback(env, callback, AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, ERR_OK),
        nullptr);
}

void EtsTaskManager::NativeFinishTask(ani_env *env, ani_string aniTaskId, ani_object callback)
{
    TAG_LOGD(AAFwkTag::ETSRUNTIME, "called");
    if (env == nullptr || callback == nullptr) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "env or callback is null");
        return;
    }
    std::string taskId;
    if (!ConvertTaskId(env, aniTaskId, taskId)) {
        CallbackInvalidParam(env, callback);
        return;
    }
    int32_t ret = AppExecFwk::TaskManagerClient::GetInstance().FinishTask(taskId);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "finishTask failed: %{public}d", ret);
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, ret), nullptr);
        return;
    }
    AppExecFwk::AsyncCallback(env, callback, AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, ERR_OK),
        nullptr);
}

void EtsTaskManagerRegistryInit(ani_env *env)
{
    TAG_LOGD(AAFwkTag::ETSRUNTIME, "call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "env is null");
        return;
    }
    ani_status status = env->ResetError();
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "ResetError failed status : %{public}d", status);
    }
    ani_namespace ns;
    status = env->FindNamespace(TASK_MANAGER_SPACE_NAME, &ns);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "FindNamespace taskManager failed status : %{public}d", status);
        return;
    }
    std::array kitFunctions = {
        ani_native_function{ "nativeStartTask", nullptr,
            reinterpret_cast<void *>(EtsTaskManager::NativeStartTask) },
        ani_native_function{ "nativeFinishTask", nullptr,
            reinterpret_cast<void *>(EtsTaskManager::NativeFinishTask) },
    };
    status = env->Namespace_BindNativeFunctions(ns, kitFunctions.data(), kitFunctions.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "Namespace_BindNativeFunctions failed status : %{public}d", status);
    }
    if (env->ResetError() != ANI_OK) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "ResetError failed");
    }
    TAG_LOGD(AAFwkTag::ETSRUNTIME, "end");
}
} // namespace TaskManagerEts
} // namespace OHOS

extern "C" {
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    if (vm == nullptr || result == nullptr) {
        return ANI_INVALID_ARGS;
    }
    ani_env *env = nullptr;
    ani_status status = vm->GetEnv(ANI_VERSION_1, &env);
    if (status != ANI_OK) {
        return ANI_NOT_FOUND;
    }
    OHOS::TaskManagerEts::EtsTaskManagerRegistryInit(env);
    *result = ANI_VERSION_1;
    return ANI_OK;
}
} // extern "C"
