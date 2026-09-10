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

#ifndef OHOS_ABILITY_RUNTIME_ETS_UKEY_AUTH_UI_EXTENSION_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_ETS_UKEY_AUTH_UI_EXTENSION_CONTEXT_H

#include <memory>

#include "ani.h"
#include "ukey_auth_ui_extension_context.h"

namespace OHOS {
namespace AbilityRuntime {
ani_object CreateEtsUkeyAuthUIExtensionContext(ani_env *env,
    std::shared_ptr<UkeyAuthUIExtensionContext> context);

/**
 * @brief Ets wrapper for UkeyAuthUIExtensionContext, binding only the four public methods.
 */
class EtsUkeyAuthUIExtensionContext final {
public:
    explicit EtsUkeyAuthUIExtensionContext(const std::shared_ptr<UkeyAuthUIExtensionContext> &context)
        : context_(context) {}
    virtual ~EtsUkeyAuthUIExtensionContext() = default;

    static EtsUkeyAuthUIExtensionContext *GetEtsUkeyAuthUIExtensionContext(ani_env *env, ani_object obj);
    static void TerminateSelfSync(ani_env *env, ani_object obj, ani_object callback);
    static void TerminateSelfWithResultSync(
        ani_env *env, ani_object obj, ani_object abilityResult, ani_object callback);
    static void SetColorMode(ani_env *env, ani_object aniObj, ani_enum_item aniColorMode);
    static void ReportDrawnCompleted(ani_env *env, ani_object aniObj, ani_object callback);
    static bool BindNativePtrCleaner(ani_env *env);
    static void Clean(ani_env *env, ani_object object);

private:
    void OnTerminateSelf(ani_env *env, ani_object obj, ani_object callback);
    void OnTerminateSelfWithResult(ani_env *env, ani_object obj, ani_object abilityResult, ani_object callback);
    void OnSetColorMode(ani_env *env, ani_object aniObj, ani_enum_item aniColorMode);
    void OnReportDrawnCompleted(ani_env *env, ani_object aniObj, ani_object callback);

private:
    std::weak_ptr<UkeyAuthUIExtensionContext> context_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ETS_UKEY_AUTH_UI_EXTENSION_CONTEXT_H
