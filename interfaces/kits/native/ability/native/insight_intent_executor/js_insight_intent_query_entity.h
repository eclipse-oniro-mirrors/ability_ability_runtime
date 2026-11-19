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

#ifndef OHOS_ABILITY_RUNTIME_JS_INSIGHT_INTENT_QUERY_ENTITY_H
#define OHOS_ABILITY_RUNTIME_JS_INSIGHT_INTENT_QUERY_ENTITY_H

#include "insight_intent_execute_result.h"
#include "insight_intent_executor.h"
#include "js_insight_intent_utils.h"
#include "js_runtime.h"
#include "native_reference.h"
#include "file_mapper.h"
#include "want_params.h"

namespace OHOS {
namespace AbilityRuntime {
using State = JsInsightIntentUtils::State;
class JsInsightIntentQueryEntityCallback {
public:
    std::string queryType_;
    std::shared_ptr<AAFwk::WantParams> parameters_;
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback_;
};
class JsInsightIntentQueryEntity final : public InsightIntentExecutor,
                                  public std::enable_shared_from_this<JsInsightIntentQueryEntity> {
public:
    explicit JsInsightIntentQueryEntity(JsRuntime& runtime);
    JsInsightIntentQueryEntity(const JsInsightIntentQueryEntity&) = delete;
    JsInsightIntentQueryEntity(const JsInsightIntentQueryEntity&&) = delete;
    JsInsightIntentQueryEntity& operator=(const JsInsightIntentQueryEntity&) = delete;
    JsInsightIntentQueryEntity& operator=(const JsInsightIntentQueryEntity&&) = delete;
    ~JsInsightIntentQueryEntity() override;

    static std::shared_ptr<JsInsightIntentQueryEntity> Create(JsRuntime& runtime);

    bool Init(const InsightIntentExecutorInfo& insightIntentInfo) override;

    bool HandleExecuteIntent(
        std::shared_ptr<InsightIntentExecuteParam> executeParam,
        const std::shared_ptr<NativeReference>& pageLoader,
        std::unique_ptr<InsightIntentExecutorAsyncCallback> callback,
        bool& isAsync) override;

private:
    bool LoadJsCode(const InsightIntentExecutorInfo& insightIntentInfo, JsRuntime& runtime);
    bool HandleResultReturnedFromJsFunc(napi_value resultJs);
    bool ExecuteQueryEntity(std::shared_ptr<InsightIntentExecuteParam> executeParam);
    bool ExecuteIntentCheckError();
    napi_value GetTargetMethod(napi_env env, napi_value constructor, const std::string &methodName);

    static napi_value HandleJsResultReturned(napi_env env, napi_callback_info info);

    JsRuntime& runtime_;
    State state_ = State::CREATED;
    std::unique_ptr<JsInsightIntentQueryEntityCallback> queryCallback_;
    std::unique_ptr<NativeReference> entityReference_ = nullptr;
    std::unique_ptr<AbilityBase::FileMapper> safeData_ = nullptr;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_INSIGHT_INTENT_QUERY_ENTITY_H
