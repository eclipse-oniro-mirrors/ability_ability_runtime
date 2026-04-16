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

#ifndef OHOS_ABILITY_RUNTIME_ETS_FORM_EXTENSION_CONTEXT_MODULE_H
#define OHOS_ABILITY_RUNTIME_ETS_FORM_EXTENSION_CONTEXT_MODULE_H

#include <memory>

#include "ani.h"
#include "form_extension_context.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
class EtsFormExtensionContextModule {
public:
    EtsFormExtensionContextModule() = default;
    ~EtsFormExtensionContextModule() = default;

    EtsFormExtensionContextModule(const EtsFormExtensionContextModule&) = delete;
    EtsFormExtensionContextModule(EtsFormExtensionContextModule&&) = delete;
    EtsFormExtensionContextModule& operator=(const EtsFormExtensionContextModule&) = delete;
    EtsFormExtensionContextModule& operator=(EtsFormExtensionContextModule&&) = delete;
    static ani_object NativeTransferStatic(ani_env *env, ani_object, ani_object input, ani_object type);
    static ani_object NativeTransferDynamic(ani_env *env, ani_class aniCls, ani_object input);
    static napi_value GetOrCreateDynamicObject(napi_env napiEnv,
        std::shared_ptr<FormExtensionContext> formExtContext);
    static void RegisterContextObjectCreator();

private:
    static ani_object CreateStaticObject(ani_env *aniEnv, ani_object type,
        std::shared_ptr<FormExtensionContext> formExtContext);
    static std::unique_ptr<NativeReference> CreateNativeReference(napi_env napiEnv,
        std::shared_ptr<FormExtensionContext> formExtContext);
    static ani_object CreateDynamicObject(ani_env *aniEnv, ani_class aniCls,
        std::shared_ptr<FormExtensionContext> formExtContext);
    static bool IsInstanceOf(ani_env *aniEnv, ani_object aniObj);
    static napi_value AttachFormExtensionContext(napi_env env, void *value, void *hint);
};

void EtsFormExtensionContextModuleInit(ani_env *aniEnv);
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ETS_FORM_EXTENSION_CONTEXT_MODULE_H
