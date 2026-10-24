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

#ifndef OHOS_ABILITY_RUNTIME_ETS_INSIGHT_INTENT_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_ETS_INSIGHT_INTENT_CONTEXT_H

#include "ani.h"
#include "ets_native_reference.h"
#include "insight_intent_context.h"
namespace OHOS {
namespace AbilityRuntime {
/**
 * @class EtsInsightIntentContext
 * EtsInsightIntentContext provides a context for insightintent to execute certain tasks.
 */
class EtsInsightIntentContext final {
public:

    explicit EtsInsightIntentContext(const std::shared_ptr<InsightIntentContext> &context) : context_(context) {}
    ~EtsInsightIntentContext() = default;

    static void Finalizer(ani_env *env, void *data, void *hint);

    /**
     * Starts a new ability. Only such ability in the same application with the
     * caller can be started.
     *
     * @param env, the ani environment.
     * @param info, the params passed from ets caller.
     *
     * @return result of StartAbility.
     */
    static ani_object StartAbilitySync(ani_env *env, ani_object aniObj, ani_object wantObj);
    static void StartAbilitySyncCheck(ani_env *env, ani_object aniObj);
    static void SetReturnModeForUIAbilityForeground(ani_env *env, ani_object aniObj, ani_enum_item aniMode);
    static void SetReturnModeForUIExtensionAbility(ani_env *env, ani_object aniObj, ani_enum_item aniMode);
    static ani_object GetInsightIntentExecutionInfo(ani_env *env, ani_object aniObj);
    static EtsInsightIntentContext *GetContext(ani_env *env, ani_object aniObj);

    std::shared_ptr<InsightIntentContext> GetNativeContext()
    {
        return context_.lock();
    }

private:
    ani_object StartAbilityInner(ani_env *env, AAFwk::Want &want);
    std::weak_ptr<InsightIntentContext> context_;
};

std::unique_ptr<AppExecFwk::ETSNativeReference> CreateEtsInsightIntentContext(ani_env *env,
    EtsInsightIntentContext *context);
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ETS_INSIGHT_INTENT_CONTEXT_H
