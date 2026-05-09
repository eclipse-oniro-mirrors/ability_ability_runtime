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

#ifndef OHOS_ABILITY_RUNTIME_JS_INSIGHT_INTENT_FUNC_H
#define OHOS_ABILITY_RUNTIME_JS_INSIGHT_INTENT_FUNC_H

#include "file_mapper.h"
#include "insight_intent_execute_result.h"
#include "insight_intent_executor.h"
#include "js_insight_intent_utils.h"
#include "js_runtime.h"
#include "native_reference.h"

namespace OHOS {
namespace AbilityRuntime {
using State = JsInsightIntentUtils::State;

class JsInsightIntentFunc final : public InsightIntentExecutor,
                                  public std::enable_shared_from_this<JsInsightIntentFunc> {
public:
    explicit JsInsightIntentFunc(JsRuntime& runtime);
    JsInsightIntentFunc(const JsInsightIntentFunc&) = delete;
    JsInsightIntentFunc(const JsInsightIntentFunc&&) = delete;
    JsInsightIntentFunc& operator=(const JsInsightIntentFunc&) = delete;
    JsInsightIntentFunc& operator=(const JsInsightIntentFunc&&) = delete;
    ~JsInsightIntentFunc() override;

    /**
     * @brief Create insight intent executor, intent type is function.
     *
     * @param runtime The JsRuntime.
     */
    static std::shared_ptr<JsInsightIntentFunc> Create(JsRuntime& runtime);

    /**
     * @brief Init insight intent executor.
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
    bool LoadJsCode(const InsightIntentExecutorInfo& insightIntentInfo, JsRuntime& runtime);

    void ReplyFailedInner(InsightIntentInnerErr innerErr = InsightIntentInnerErr::INSIGHT_INTENT_EXECUTE_REPLY_FAILED);

    void ReplySucceededInner(std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> resultCpp);

    bool HandleResultReturnedFromJsFunc(napi_value resultJs);

    static std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> GetResultFromJs(napi_env env, napi_value resultJs);

    static napi_value ResolveCbCpp(napi_env env, napi_callback_info info);

    bool ExecuteIntentCheckError();

    bool ExecuteInsightIntent(std::shared_ptr<InsightIntentExecuteParam> executeParam);

    bool ParseParams(napi_env env, const AAFwk::WantParams& param,
        const std::unordered_map<std::string, size_t> &paramMap,
        size_t &argc, std::vector<napi_value> &argv);

    napi_value GetTargetMethod(napi_env env, napi_value constructor, const std::string &methodName);

    void PrepareMethodParams(std::shared_ptr<InsightIntentExecuteParam> executeParam,
        std::unordered_map<std::string, size_t> &paramMap);

    JsRuntime& runtime_;
    State state_ = State::CREATED;
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback_;
    std::unique_ptr<AbilityBase::FileMapper> safeData_ = nullptr;
    bool isAsync_ = false;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_INSIGHT_INTENT_FUNC_H
