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

#ifndef OHOS_ABILITY_RUNTIME_ETS_APPLICATION_CONTEXT_UTILS_H
#define OHOS_ABILITY_RUNTIME_ETS_APPLICATION_CONTEXT_UTILS_H

#include "ani.h"
#include "ability_manager_client.h"
#include "ability_native_thread.h"
#include "ani_common_util.h"
#include "ani_common_want.h"
#include "application_context.h"
#include "ets_ability_lifecycle_callback.h"
#include "ets_application_state_change_callback.h"
#include "ets_context_utils.h"
#include "ets_enviroment_callback.h"
#include "ets_system_configuration_updated_callback.h"

namespace OHOS {
namespace AbilityRuntime {
class EtsApplicationContextUtils {
public:
    explicit EtsApplicationContextUtils(std::weak_ptr<ApplicationContext> &&applicationContext)
        : applicationContext_(std::move(applicationContext))
    {
    }
    virtual ~EtsApplicationContextUtils() = default;
    static void NativeOnInteropLifecycleCallbackSync(ani_env *env, ani_object aniObj, ani_object callback);
    static void NativeOffInteropLifecycleCallbackSync(ani_env *env, ani_object aniObj, ani_object callback);
    static void RestartApp(ani_env *env, ani_object aniObj, ani_object wantObj);
    static void SetFont(ani_env *env, ani_object aniObj, ani_string font);
    static void SetColorMode(ani_env *env, ani_object aniObj, ani_enum_item colorMode);
    static void SetLanguage(ani_env *env, ani_object aniObj, ani_string language);
    static void SetFontSizeScale(ani_env *env, ani_object aniObj, ani_double fontSizeScale);
    static void ClearUpApplicationData(ani_env *env, ani_object aniObj, ani_object callback);
    static void GetRunningProcessInformation(ani_env *env, ani_object aniObj, ani_object callback);
    static void killAllProcesses(ani_env *env, ani_object aniObj, ani_boolean clearPageStack, ani_object callback);
    static void PreloadUIExtensionAbility(ani_env *env, ani_object aniObj, ani_object wantObj, ani_object callback);
    static void SetSupportedProcessCacheSync(ani_env *env, ani_object aniObj, ani_boolean value);
    static void Clean(ani_env *env, ani_object object);
    static EtsApplicationContextUtils* GeApplicationContext(ani_env *env, ani_object aniObj);
    static ani_object SetApplicationContext(ani_env *aniEnv,
        const std::shared_ptr<ApplicationContext> &applicationContext);
    static ani_object CreateEtsApplicationContext(ani_env *aniEnv,
        const std::shared_ptr<ApplicationContext> &applicationContext);
    static void BindApplicationContextFunc(ani_env *aniEnv);
    static ani_int GetCurrentAppCloneIndex(ani_env *env, ani_object aniObj);
    static ani_string GetCurrentInstanceKey(ani_env *env, ani_object aniObj);
    static void GetAllRunningInstanceKeys(ani_env *env, ani_object aniObj, ani_object callback);
    static void GetAllWindowStages(ani_env *env, ani_object aniObj, ani_object callback);
    static ani_int NativeOnLifecycleCallbackSync(ani_env *env, ani_object aniObj, ani_string type,
        ani_object callback);
    static void NativeOffLifecycleCallbackSync(ani_env *env, ani_object aniObj, ani_string type,
        ani_int callbackId, ani_object callback);
    static void NativeOffAbilityLifecycleCheck(ani_env *env, ani_object aniObj);
    static void NativeOffApplicationStateChangeSync(ani_env *env, ani_object aniObj, ani_object callback);
    static void NativeOnApplicationStateChangeSync(ani_env *env, ani_object aniObj, ani_object callback);

    static void NativeOnSystemConfigurationUpdatedSync(ani_env *env, ani_object aniObj, ani_object callback);
    static void NativeOffSystemConfigurationUpdatedSync(ani_env *env, ani_object aniObj, ani_object callback);
    static void NativeOffEnvironmentSync(ani_env *env, ani_object aniObj, ani_int callbackId, ani_object callback);
    static void NativeOffEnvironmentCheck(ani_env *env, ani_object aniObj);
    static ani_int NativeOnEnvironmentSync(ani_env *env, ani_object aniObj, ani_object envCallback);
    static ani_object GetUIAbilityByInstanceId(ani_env *env, ani_object aniObj, ani_string instanceId);
protected:
    std::weak_ptr<ApplicationContext> applicationContext_;
private:
    ani_int RegisterAbilityLifecycleCallback(ani_env *env, ani_object callback);
    void UnregisterAbilityLifecycleCallback(ani_env *env, int32_t callbackId, ani_object callback);
    void RegisterInteropAbilityLifecycleCallback(ani_env *env, ani_object callback);
    void UnregisterInteropAbilityLifecycleCallback(ani_env *env, ani_object callback);
    void OnRestartApp(ani_env *env, ani_object aniObj, ani_object wantObj);
    void OnSetFont(ani_env *env, ani_object aniObj, ani_string font);
    void OnSetColorMode(ani_env *env, ani_object aniObj, ani_enum_item colorMode);
    void OnSetLanguage(ani_env *env, ani_object aniObj, ani_string language);
    void OnSetFontSizeScale(ani_env *env, ani_object aniObj, ani_double fontSizeScale);
    void OnClearUpApplicationData(ani_env *env, ani_object aniObj, ani_object callback);
    void OnGetRunningProcessInformation(ani_env *env, ani_object aniObj, ani_object callback);
    void OnkillAllProcesses(ani_env *env, ani_object aniObj, ani_boolean clearPageStack, ani_object callback);
    void OnPreloadUIExtensionAbility(ani_env *env, ani_object aniObj, ani_object wantObj, ani_object callback);
    void OnSetSupportedProcessCacheSync(ani_env *env, ani_object aniObj, ani_boolean value);
    ani_int OnGetCurrentAppCloneIndex(ani_env *env, ani_object aniObj);
    ani_string OnGetCurrentInstanceKey(ani_env *env, ani_object aniObj);
    void OnGetAllRunningInstanceKeys(ani_env *env, ani_object aniObj, ani_object callback);
    void OnGetAllWindowStages(ani_env *env, ani_object aniObj, ani_object callback);
    void OnNativeOffApplicationStateChangeSync(ani_env *env, ani_object aniObj, ani_object callback);
    void OnNativeOnApplicationStateChangeSync(ani_env *env, ani_object aniObj, ani_object callback);
    void OnNativeOffEnvironmentSync(ani_env *env, ani_object aniObj, ani_int callbackId, ani_object callback);
    void OnNativeSystemConfigurationUpdatedSync(ani_env *env, ani_object aniObj, ani_object callback);
    void OffNativeSystemConfigurationUpdatedSync(ani_env *env, ani_object aniObj, ani_object callback);
    ani_int OnNativeOnEnvironmentSync(ani_env *env, ani_object aniObj, ani_object envCallback);
    ani_object OnGetUIAbilityByInstanceId(ani_env *env, ani_string instanceId);
    static void SetEventHubContextIsApplicationContext(ani_env *aniEnv, ani_ref eventHubRef);
    ani_object CreateWindowStageArray(ani_env *env, std::vector<std::shared_ptr<UIAbility>> uiAbility);
    std::shared_ptr<EtsEnviromentCallback> etsEnviromentCallback_;
    std::shared_ptr<EtsApplicationStateChangeCallback> applicationStateCallback_;
    std::shared_ptr<EtsSystemConfigurationUpdatedCallback> systemConfigurationUpdatedCallback_;
    static std::mutex abilityLifecycleCallbackLock_;
    static std::shared_ptr<EtsAbilityLifecycleCallback> abilityLifecycleCallback_ ;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ETS_APPLICATION_CONTEXT_UTILS_H