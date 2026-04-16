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

#ifndef OHOS_ABILITY_RUNTIME_ETS_CONTEXT_MODULE_H
#define OHOS_ABILITY_RUNTIME_ETS_CONTEXT_MODULE_H

#include <memory>

#include "ability_stage_context.h"
#include "ani.h"
#include "application_context.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
class EtsContextModule {
public:
    EtsContextModule() = default;
    ~EtsContextModule() = default;

    EtsContextModule(const EtsContextModule&) = delete;
    EtsContextModule(EtsContextModule&&) = delete;
    EtsContextModule& operator=(const EtsContextModule&) = delete;
    EtsContextModule& operator=(EtsContextModule&&) = delete;

    static ani_object NativeTransferStatic(ani_env *env, ani_object aniObj, ani_object input, ani_object type);
    static ani_object NativeTransferDynamic(ani_env *env, ani_class aniCls, ani_object input);
    static ani_boolean ContextType(ani_env *env, ani_object self, ani_int contextType);
    static napi_value GetOrCreateDynamicObject(napi_env napiEnv, std::shared_ptr<Context> context);

private:
    static bool LoadTargetModule(ani_env *aniEnv, const std::string &className);
    static std::unique_ptr<NativeReference> CreateNativeReference(napi_env napiEnv, std::shared_ptr<Context> context);
    static ani_object GetOrCreateStaticObject(ani_env *aniEnv, ani_object input, ani_object type,
        std::shared_ptr<Context> context);
    static ani_object CreateDynamicObject(ani_env *aniEnv, ani_class aniCls, ani_object input,
        std::shared_ptr<Context> context);
    static void SaveDynamicBindingObject(ani_env *aniEnv, ani_object input, std::shared_ptr<Context> context);
    static void SaveStaticBindingObject(ani_env *aniEnv, ani_object input, std::shared_ptr<Context> context);
};

void EtsContextModuleInit(ani_env *aniEnv);
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ETS_CONTEXT_MODULE_H
