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

#ifndef OHOS_ABILITY_RUNTIME_ETS_APPLICATION_H
#define OHOS_ABILITY_RUNTIME_ETS_APPLICATION_H

#include "ani.h"

namespace OHOS {
namespace AbilityRuntime {
class EtsApplication {
public:
    static void CreateModuleContext(ani_env *env,
        ani_object contextObj, ani_string bundleName, ani_string moduleName, ani_object callback);
    static void CreateModuleContextCheck(ani_env *env,
        ani_object contextObj, ani_string moduleName, ani_object bundleName);
    static ani_object CreateModuleContextSync(ani_env *env,
        ani_object contextObj, ani_string moduleName);
    static void CreateBundleContext(ani_env *env,
        ani_object contextObj, ani_string bundleName, ani_object callback);
    static void CreateBundleContextCheck(ani_env *env,
        ani_object contextObj, ani_string bundleName);
    static void CreatePluginModuleContext(ani_env *env,
        ani_object contextObj, ani_string pluginBundleName, ani_string pluginModuleName, ani_object callback);
    static void CreatePluginModuleContextCheck(ani_env *env,
        ani_object contextObj, ani_string pluginBundleName, ani_string pluginModuleName);
    static ani_object GetApplicationContext(ani_env *env);
    static ani_object GetApplicationContextInstance(ani_env *env);
    static ani_enum_item GetAppPreloadType(ani_env *env);
    static void ExitMasterProcessRole(ani_env *env, ani_object callback);
    static void CreatePluginModuleContextForHostBundle(ani_env *env, ani_object contextObj,
        ani_string pluginBundleName, ani_string pluginModuleName, ani_string hostBundleName, ani_object callback);
    static void CreatePluginModuleContextForHostBundleCheck(ani_env *env, ani_object contextObj,
        ani_string pluginBundleName, ani_string pluginModuleName, ani_string hostBundleName, ani_object callback);
    static void DemoteCurrentFromCandidateMasterProcess(ani_env *env, ani_object callback);
    static void PromoteCurrentToCandidateMasterProcess(ani_env *env,
        ani_boolean isInsertToHead, ani_object callback);
};
void ApplicationInit(ani_env *env);
} // namespace AbilityRuntime
} // namespace OHOS
#endif //OHOS_ABILITY_RUNTIME_ETS_APPLICATION_H