/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_JS_INSIGHT_INTENT_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_JS_INSIGHT_INTENT_CONTEXT_H

#include "native_engine/native_engine.h"
#include "insight_intent_context.h"
#include "insight_intent_constant.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
/**
 * @class JsInsightIntentContext
 * JsInsightIntentContext provides a context for insightintent to execute certain tasks.
 */
class JsInsightIntentContext final {
public:
    explicit JsInsightIntentContext(const std::shared_ptr<InsightIntentContext>& context) : context_(context) {}
    ~JsInsightIntentContext() = default;

    static void Finalizer(napi_env env, void* data, void* hint);

    /**
     * Starts a new ability. Only such ability in the same application with the caller
     * can be started.
     *
     * @param env, the napi environment.
     * @param info, the params passed from js caller.
     *
     * @return result of StartAbility.
     */
    static napi_value StartAbiity(napi_env env, napi_callback_info info);

    static napi_value SetReturnModeForUiAbilityForeground(napi_env env, napi_callback_info info);

    static napi_value SetReturnModeForUiExtensionAbility(napi_env env, napi_callback_info info);

    static napi_value GetInsightIntentExecutionInfo(napi_env env, napi_callback_info info);

    std::shared_ptr<InsightIntentContext> GetContext()
    {
        return context_.lock();
    }

private:
    napi_value OnStartAbility(napi_env env, NapiCallbackInfo& info);
    napi_value OnSetReturnModeForUiAbilityForeground(napi_env env, NapiCallbackInfo& info);
    napi_value OnSetReturnModeForUiExtensionAbility(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetInsightIntentExecutionInfo(napi_env env, NapiCallbackInfo& info);

    std::weak_ptr<InsightIntentContext> context_;
};

/**
 * Creates an js object for specific insight intent context.
 *
 * @param env, the napi environment.
 * @param context, the specific insight intent context object.
 *
 * @return result of StartAbility.
 */
napi_value CreateJsInsightIntentContext(napi_env env, const std::shared_ptr<InsightIntentContext>& context);

/**
 * Function of check startAbiliryParam parammeters.
 *
 * @param env, the napi environment.
 * @param info, Indicates the parameters from js.
 * @param want, the want of the ability to start.
 *
 * @return result of check startAbiliryParam parammeters.
 */
bool CheckStartAbilityParam(napi_env env, NapiCallbackInfo& info, AAFwk::Want& want);

}  // namespace AbilityRuntime
}  // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_INSIGHT_INTENT_CONTEXT_H
