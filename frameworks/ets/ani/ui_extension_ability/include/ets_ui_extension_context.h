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
#ifndef OHOS_ABILITY_RUNTIME_ETS_UI_EXTENSION_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_ETS_UI_EXTENSION_CONTEXT_H

#include <array>
#include <iostream>
#include <unistd.h>

#include "ani.h"
#include "ui_extension_context.h"
#include "ets_runtime.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "ohos_application.h"
#include "ui_extension_context.h"

ani_object CreateEtsUIExtensionContext(ani_env *env, std::shared_ptr<OHOS::AbilityRuntime::UIExtensionContext> context);

class EtsUIExtensionContext final {
public:
    explicit EtsUIExtensionContext(const std::shared_ptr<OHOS::AbilityRuntime::UIExtensionContext> &context)
        : context_(context) {}
    virtual ~EtsUIExtensionContext() = default;

    static void TerminateSelfSync(ani_env *env, ani_object obj, ani_object callback);
    static void TerminateSelfWithResultSync(ani_env *env, ani_object obj, ani_object abilityResult, ani_object callback);

    static void EtsCreatExtensionContext(ani_env *aniEnv, ani_class contextClass, ani_object contextObj,
        std::shared_ptr<OHOS::AbilityRuntime::ExtensionContext> context);
private:
    static void BindExtensionInfo(ani_env *aniEnv, ani_class contextClass, ani_object contextObj,
        std::shared_ptr<OHOS::AbilityRuntime::Context> context, std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo);

protected:
    std::weak_ptr<OHOS::AbilityRuntime::UIExtensionContext> context_;
};
#endif // OHOS_ABILITY_RUNTIME_ETS_UI_EXTENSION_CONTEXT_H