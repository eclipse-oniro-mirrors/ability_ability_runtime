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
#ifndef OHOS_ABILITY_RUNTIME_ETS_APP_MANAGER_UTILS_H
#define OHOS_ABILITY_RUNTIME_ETS_APP_MANAGER_UTILS_H

#include <vector>

#include "ability_state_data.h"
#include "ani.h"
#include "app_state_data.h"
#include "ets_runtime.h"
#include "focused_application_info.h"
#include "keep_alive_info.h"
#include "process_data.h"
#include "running_multi_info.h"
#include "running_process_info.h"
#ifdef SUPPORT_GRAPHICS
#include "ability_first_frame_state_data.h"
#endif

namespace OHOS {
namespace AppManagerEts {
ani_object NewArrayClass(ani_env *env, const std::vector<std::string> &data);
ani_object WrapAppStateData(ani_env *env, const AppExecFwk::AppStateData &appStateData);
bool SetAppStateData(ani_env *env, ani_object object, const AppExecFwk::AppStateData &appStateData);
ani_object CreateAppStateDataArray(ani_env *env, const std::vector<AppExecFwk::AppStateData> &data);
ani_object CreateRunningMultiInstanceInfoArray(ani_env *env,
    const std::vector<AppExecFwk::RunningMultiInstanceInfo> &infos);
ani_object CreateRunningAppCloneArray(ani_env *env, const std::vector<AppExecFwk::RunningAppClone> &infos);
bool SetRunningMultiAppInfo(ani_env *env, ani_object object,
    const AppExecFwk::RunningMultiAppInfo &runningMultiAppInfo);
ani_object WrapRunningMultiAppInfo(ani_env *env, const AppExecFwk::RunningMultiAppInfo &runningMultiAppInfo);
ani_object WrapRunningMultiInstanceInfo(ani_env *env, const AppExecFwk::RunningMultiInstanceInfo &instanceInfo);
bool SetRunningMultiInstanceInfo(ani_env *env, ani_object object,
    const AppExecFwk::RunningMultiInstanceInfo &instanceInfo);
ani_object WrapRunningAppClone(ani_env *env, const AppExecFwk::RunningAppClone &runningAppClone);
bool SetRunningAppClone(ani_env *env, ani_object object, const AppExecFwk::RunningAppClone &runningAppClone);
ani_object WrapProcessData(ani_env *env, const AppExecFwk::ProcessData &processData);
bool SetProcessData(ani_env* env, ani_object object, const AppExecFwk::ProcessData &processData);
bool UnWrapArrayString(ani_env *env, ani_object arrayObj, std::vector<std::string> &stringList);
ani_object CreateEmptyAniArray(ani_env *env);
ani_object CreateEmptyMultiAppInfo(ani_env *env);
ani_object CreateIntAniArray(ani_env *env, const std::vector<int32_t> &dataArry);
bool UnWrapArrayInt(ani_env *env, ani_object arrayObj, std::vector<int32_t> &list);
ani_object CreateKeepAliveInfoArray(ani_env *env, const std::vector<AbilityRuntime::KeepAliveInfo> &infos);
ani_object WrapKeepAliveInfo(ani_env *env, const AbilityRuntime::KeepAliveInfo &keepAliveInfo);
bool SetKeepAliveInfo(ani_env *env, ani_object object, const AbilityRuntime::KeepAliveInfo &keepInfo);
#ifdef SUPPORT_GRAPHICS
ani_object WrapAbilityFirstFrameStateData(ani_env *env,
    const AppExecFwk::AbilityFirstFrameStateData &abilityFirstFrameStateData);
bool SetAbilityFirstFrameStateData(ani_env *env, ani_object object,
    const AppExecFwk::AbilityFirstFrameStateData &abilityFirstFrameStateData);
#endif
ani_object WrapFocusedApplicationInfo(ani_env *env,
    const AppExecFwk::FocusedApplicationInfo &focusedApplicationInfo);
bool AttachAniEnv(ani_vm *etsVm, ani_env *&env);
void DetachAniEnv(ani_vm *etsVm);
void ReleaseObjectReference(ani_vm *etsVm, ani_ref etsObjRef);

} // namespace AppManagerEts
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ETS_APP_MANAGER_UTILS_H
