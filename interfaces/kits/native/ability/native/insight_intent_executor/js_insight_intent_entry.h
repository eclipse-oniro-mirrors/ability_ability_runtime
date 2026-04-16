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

#ifndef OHOS_ABILITY_RUNTIME_JS_INSIGHT_INTENT_ENTRY_H
#define OHOS_ABILITY_RUNTIME_JS_INSIGHT_INTENT_ENTRY_H

#include "insight_intent_executor.h"

#include "file_mapper.h"
#include "insight_intent_execute_result.h"
#include "js_insight_intent_utils.h"
#include "js_runtime.h"
#include "native_reference.h"

namespace OHOS {
namespace AbilityRuntime {
using State = JsInsightIntentUtils::State;

class JsInsightIntentEntry final : public InsightIntentExecutor,
                                   public std::enable_shared_from_this<JsInsightIntentEntry> {
public:
    explicit JsInsightIntentEntry(JsRuntime& runtime);
    JsInsightIntentEntry(const JsInsightIntentEntry&) = delete;
    JsInsightIntentEntry(const JsInsightIntentEntry&&) = delete;
    JsInsightIntentEntry& operator=(const JsInsightIntentEntry&) = delete;
    JsInsightIntentEntry& operator=(const JsInsightIntentEntry&&) = delete;
    ~JsInsightIntentEntry() override;

    /**
     * @brief Create insight intent executor, intent type is entry.
     *
     * @param runtime The JsRuntime.
     */
    static std::shared_ptr<JsInsightIntentEntry> Create(JsRuntime& runtime);

    /**
     * @brief Init insight intent executor and insight intent context.
     *
     * @param insightIntentInfo The insight intent executor information.
     */
    bool Init(const InsightIntentExecutorInfo& insightIntentInfo) override;

    /**
     * @brief Handling the insight intent execute.
     *
     * @param executeParam The execute params.
     * @param pageLoader The page loader.
     * @param callback The async callback.
     * @param isAsync Indicates the target function is promise or not.
     */
    bool HandleExecuteIntent(
        std::shared_ptr<InsightIntentExecuteParam> executeParam,
        const std::shared_ptr<NativeReference>& pageLoader,
        std::unique_ptr<InsightIntentExecutorAsyncCallback> callback,
        bool& isAsync) override;

private:
    std::unique_ptr<NativeReference> LoadJsCode(
        const InsightIntentExecutorInfo& insightIntentInfo,
        JsRuntime& runtime);

    napi_value CallJsFunctionWithResultInner(
        const char* funcName,
        size_t argc,
        const napi_value* argv
    );

    void ReplyFailedInner(InsightIntentInnerErr innerErr = InsightIntentInnerErr::INSIGHT_INTENT_EXECUTE_REPLY_FAILED);

    void ReplySucceededInner(std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> resultCpp);

    bool HandleResultReturnedFromJsFunc(napi_value resultJs);

    bool ExecuteIntentCheckError();

    bool ExecuteInsightIntent(
        const std::string& name,
        const AAFwk::WantParams& param,
        const std::shared_ptr<NativeReference>& pageLoader);

    bool PrepareParameters(InsightIntentExecuteMode mode, const std::shared_ptr<NativeReference>& pageLoader);
    bool PrepareParametersUIAbilityForeground(napi_env env, const std::shared_ptr<NativeReference>& pageLoader);
    bool PrepareParametersUIAbilityBackground(napi_env env);
    bool PrepareParametersUIExtension(napi_env env, const std::shared_ptr<NativeReference>& pageLoader);
    bool PrepareParametersServiceExtension(napi_env env);

    bool AssignObject(napi_env env, const AAFwk::WantParams &wantParams);

    JsRuntime& runtime_;
    State state_ = State::CREATED;
    std::unique_ptr<NativeReference> jsObj_ = nullptr;
    std::unique_ptr<NativeReference> contextObj_ = nullptr;
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback_;
    bool isAsync_ = false;
    std::unique_ptr<AbilityBase::FileMapper> safeData_ = nullptr;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_INSIGHT_INTENT_ENTRY_H
