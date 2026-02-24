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

#ifndef OHOS_ABILITY_RUNTIME_ETS_CONTEXT_UTILS_H
#define OHOS_ABILITY_RUNTIME_ETS_CONTEXT_UTILS_H

#include "ani.h"
#include "context.h"

namespace OHOS {
namespace AbilityRuntime {
namespace ContextUtil {
void BindApplicationInfo(ani_env *aniEnv, ani_class contextClass, ani_object contextObj,
    std::shared_ptr<Context> context);
void BindResourceManager(ani_env *aniEnv, ani_class contextClass, ani_object contextObj,
    std::shared_ptr<Context> context);
void CreateEtsBaseContext(ani_env *aniEnv, ani_class contextClass, ani_object contextObj,
    std::shared_ptr<Context> context);
ani_object GetApplicationContextSync(ani_env *env, ani_object aniObj);
std::shared_ptr<Context> GetBaseContext(ani_env *env, ani_object aniObj);
void SwitchArea(ani_env *env, ani_object obj, ani_enum_item areaModeItem);
ani_enum_item GetArea(ani_env *env, ani_object obj);
ani_string GetLogFileDir(ani_env *env, ani_object obj);
ani_object CreateModuleResourceManagerSync(ani_env *env, ani_object aniObj,
    ani_string bundleName, ani_string moduleName);
void Clean(ani_env *env, ani_object object);
bool SetNativeContextLong(ani_env *env, ani_object aniObj, ani_long nativeContextLong);
void NativeGetGroupDir([[maybe_unused]]ani_env *env, [[maybe_unused]]ani_object aniObj,
    ani_string dataGroupIdObj, ani_object callBackObj);
ani_object NativeCreateDisplayContext(ani_env *env, ani_object aniObj, ani_long displayId);
ani_object NativeCreateAreaModeContext(ani_env *env, ani_object aniObj, ani_object areaModeObj);
ani_object NativeCreateSystemHspModuleResourceManager(ani_env *env, ani_object aniObj,
    ani_string bundleNameObj, ani_string moduleNameObj);
ani_object CreateContextObject(ani_env* env, ani_class contextClass, std::shared_ptr<Context> nativeContext);
void SetEventHubContext(ani_env *aniEnv, ani_ref eventHubRef, ani_long nativeContextLong);

class EtsBaseContext final {
public:
    explicit EtsBaseContext(const std::shared_ptr<Context> &context)
        : context_(context) {}
    ~EtsBaseContext() = default;

    std::weak_ptr<Context> GetContext()
    {
        return context_;
    }
private:
    std::shared_ptr<Context> context_;
};
}
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ETS_CONTEXT_UTILS_H