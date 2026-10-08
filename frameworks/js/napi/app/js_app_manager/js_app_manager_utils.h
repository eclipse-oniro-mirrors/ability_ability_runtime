/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_JS_APP_MANAGER_UTILS_H
#define OHOS_ABILITY_RUNTIME_JS_APP_MANAGER_UTILS_H

#include "application_state_observer_stub.h"
#include "keep_alive_info.h"
#include "native_engine/native_engine.h"
#include "running_process_info.h"
#include "running_multi_info.h"
#ifdef SUPPORT_GRAPHICS
#include "ability_first_frame_state_data.h"
#endif

namespace OHOS {
namespace AbilityRuntime {
using OHOS::AppExecFwk::AppStateData;
using OHOS::AppExecFwk::AbilityStateData;
using OHOS::AppExecFwk::ProcessData;
using OHOS::AppExecFwk::RunningProcessInfo;
using OHOS::AppExecFwk::RunningMultiAppInfo;
using OHOS::AppExecFwk::RunningAppClone;
using OHOS::AppExecFwk::RunningMultiInstanceInfo;
#ifdef SUPPORT_GRAPHICS
using OHOS::AppExecFwk::AbilityFirstFrameStateData;
#endif
struct PreloadApplicationParam {
    std::string bundleName;
    int32_t userId;
    AppExecFwk::PreloadMode preloadMode;
    int32_t appIndex = -1;
};

napi_value CreateJsAppStateData(napi_env env, const AppStateData &appStateData);
napi_value CreateJsAbilityStateData(napi_env env, const AbilityStateData &abilityStateData);
#ifdef SUPPORT_GRAPHICS
napi_value CreateJsAbilityFirstFrameStateData(napi_env env,
    const AbilityFirstFrameStateData &abilityFirstFrameStateData);
#endif
napi_value CreateJsProcessData(napi_env env, const ProcessData &processData);
napi_value CreateJsAppStateDataArray(napi_env env, const std::vector<AppStateData> &appStateDatas);
napi_value CreateJsRunningProcessInfoArray(napi_env env, const std::vector<RunningProcessInfo> &infos);
napi_value CreateJsRunningProcessInfo(napi_env env, const RunningProcessInfo &info);
napi_value ApplicationStateInit(napi_env env);
napi_value ProcessStateInit(napi_env env);
napi_value PreloadModeInit(napi_env env);
napi_value KeepAliveAppTypeInit(napi_env env);
napi_value KeepAliveSetterInit(napi_env env);
bool ConvertPreloadApplicationParam(napi_env env, size_t argc, napi_value *argv, PreloadApplicationParam &param,
    std::string &errorMsg);
napi_value CreateJsRunningMultiAppInfo(napi_env env, const RunningMultiAppInfo &info);
napi_value CreateJsRunningAppCloneArray(napi_env env, const std::vector<RunningAppClone>& data);
napi_value CreateJsRunningAppClone(napi_env env, const RunningAppClone &info);
napi_value CreateJsRunningMultiInstanceInfosArray(napi_env env, const std::vector<RunningMultiInstanceInfo>& data);
napi_value CreateJsRunningMultiInstanceInfo(napi_env env, const RunningMultiInstanceInfo &info);
napi_value CreateJsKeepAliveBundleInfo(napi_env env, const KeepAliveInfo &info);
napi_value CreateJsKeepAliveBundleInfoArray(napi_env env, const std::vector<KeepAliveInfo>& data);
napi_value FilterBundleTypeInit(napi_env env);
napi_value FilterAppStateTypeInit(napi_env env);
napi_value FilterProcessStateTypeInit(napi_env env);
napi_value FilterAbilityStateTypeInit(napi_env env);
napi_value FilterCallbackInit(napi_env env);
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_APP_MANAGER_UTILS_H
