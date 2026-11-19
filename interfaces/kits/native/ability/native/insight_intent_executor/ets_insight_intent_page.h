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

#ifndef OHOS_ABILITY_RUNTIME_ETS_INSIGHT_INTENT_PAGE_H
#define OHOS_ABILITY_RUNTIME_ETS_INSIGHT_INTENT_PAGE_H

#include "ets_runtime.h"
#include "insight_intent_execute_result.h"
#include "insight_intent_executor.h"
#include "ets_insight_intent_utils.h"
#include "window.h"

namespace OHOS::AbilityRuntime {
class EtsInsightIntentPage final : public InsightIntentExecutor {
public:
    static InsightIntentExecutor *Create(Runtime &runtime);
    explicit EtsInsightIntentPage(ETSRuntime &runtime);
    EtsInsightIntentPage(const EtsInsightIntentPage&) = delete;
    EtsInsightIntentPage(const EtsInsightIntentPage&&) = delete;
    EtsInsightIntentPage &operator=(const EtsInsightIntentPage&) = delete;
    EtsInsightIntentPage &operator=(const EtsInsightIntentPage&&) = delete;
    ~EtsInsightIntentPage() override;

    bool Init(const InsightIntentExecutorInfo &insightIntentInfo) override;

    bool HandleExecuteIntent(
        std::shared_ptr<InsightIntentExecuteParam> executeParam,
        const std::shared_ptr<NativeReference> &pageLoader,
        std::unique_ptr<InsightIntentExecutorAsyncCallback> callback,
        bool &isAsync) override { return false; }

    bool HandleExecuteIntent(
        std::shared_ptr<InsightIntentExecuteParam> executeParam,
        void *pageLoader,
        std::unique_ptr<InsightIntentExecutorAsyncCallback> callback,
        bool &isAsync) override;

    static void SetInsightIntentParam(const std::string &hapPath,
        const AAFwk::Want &want, wptr<Rosen::Window> window, bool coldStart);

private:
    using State = EtsInsightIntentUtils::State;

    void ReplyFailedInner(InsightIntentInnerErr innerErr = InsightIntentInnerErr::INSIGHT_INTENT_EXECUTE_REPLY_FAILED);
    void ReplySucceededInner(std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> resultCpp);

    [[maybe_unused]] ETSRuntime &runtime_;
    State state_ = State::CREATED;
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback_;
};
} // namespace OHOS::AbilityRuntime

#endif // OHOS_ABILITY_RUNTIME_ETS_INSIGHT_INTENT_PAGE_H
