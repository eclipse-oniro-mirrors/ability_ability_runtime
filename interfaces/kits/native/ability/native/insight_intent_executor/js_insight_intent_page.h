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

#ifndef OHOS_ABILITY_RUNTIME_JS_INSIGHT_INTENT_PAGE_H
#define OHOS_ABILITY_RUNTIME_JS_INSIGHT_INTENT_PAGE_H

#include "execute_ohmurl_operator.h"
#include "file_mapper.h"
#include "insight_intent_execute_result.h"
#include "insight_intent_executor.h"
#include "js_insight_intent_utils.h"
#include "js_runtime.h"
#include "native_reference.h"
#include "window.h"

namespace OHOS {
namespace AbilityRuntime {
using State = JsInsightIntentUtils::State;

class JsInsightIntentPage final : public InsightIntentExecutor,
                                  public std::enable_shared_from_this<JsInsightIntentPage> {
public:
    explicit JsInsightIntentPage(JsRuntime& runtime);
    JsInsightIntentPage(const JsInsightIntentPage&) = delete;
    JsInsightIntentPage(const JsInsightIntentPage&&) = delete;
    JsInsightIntentPage& operator=(const JsInsightIntentPage&) = delete;
    JsInsightIntentPage& operator=(const JsInsightIntentPage&&) = delete;
    ~JsInsightIntentPage() override;

    /**
     * @brief Create insight intent executor, intent type is page.
     *
     * @param runtime The JsRuntime.
     */
    static std::shared_ptr<JsInsightIntentPage> Create(JsRuntime& runtime);

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

    /**
     * @brief Set insight intent param to window, window will send to uicontent.
     *
     * @param runtime The JsRuntime.
     * @param hapPath The hap path.
     * @param want The want parameters when start ability.
     * @param window The window of current ability.
     * @param coldStart Indicates cold start or not.
     */
    static void SetInsightIntentParam(JsRuntime& runtime, const std::string &hapPath,
        const AAFwk::Want &want, wptr<Rosen::Window> window, bool coldStart);

private:
    bool LoadJsCode(const InsightIntentExecutorInfo& insightIntentInfo, JsRuntime& runtime);

    void ReplyFailedInner(InsightIntentInnerErr innerErr = InsightIntentInnerErr::INSIGHT_INTENT_EXECUTE_REPLY_FAILED);

    void ReplySucceededInner(std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> resultCpp);

    bool ExecuteInsightIntent(const std::string& name, const AAFwk::WantParams& param);

    JsRuntime& runtime_;
    State state_ = State::CREATED;
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback_;
    std::unique_ptr<AbilityBase::FileMapper> safeData_ = nullptr;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_INSIGHT_INTENT_PAGE_H
