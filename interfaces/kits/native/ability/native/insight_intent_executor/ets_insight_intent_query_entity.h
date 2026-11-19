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

#ifndef OHOS_ABILITY_RUNTIME_ETS_INSIGHT_INTENT_QUERY_ENTITY_H
#define OHOS_ABILITY_RUNTIME_ETS_INSIGHT_INTENT_QUERY_ENTITY_H

#include "ets_native_reference.h"
#include "ets_runtime.h"
#include "insight_intent_executor.h"
#include "ets_insight_intent_utils.h"

namespace OHOS::AbilityRuntime {
class EtsInsightIntentQueryEntityCallback {
public:
    std::string queryType_;
    std::shared_ptr<AAFwk::WantParams> paramters_;
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback_;
};

class EtsInsightIntentQueryEntity final : public InsightIntentExecutor {
public:
    static InsightIntentExecutor *Create(Runtime &runtime);
    explicit EtsInsightIntentQueryEntity(ETSRuntime &runtime);
    EtsInsightIntentQueryEntity(const EtsInsightIntentQueryEntity&) = delete;
    EtsInsightIntentQueryEntity(const EtsInsightIntentQueryEntity&&) = delete;
    EtsInsightIntentQueryEntity &operator=(const EtsInsightIntentQueryEntity&) = delete;
    EtsInsightIntentQueryEntity &operator=(const EtsInsightIntentQueryEntity&&) = delete;
    ~EtsInsightIntentQueryEntity() override;

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

private:
    using State = EtsInsightIntentUtils::State;

    static std::unique_ptr<AppExecFwk::ETSNativeReference> LoadEtsCode(
        const InsightIntentExecutorInfo &insightIntentInfo, ETSRuntime &runtime);

    static void OnQueryEntityResult(ani_env *env, ani_object aniObj, ani_object result);
    static void OnQueryEntityError(ani_env *env, ani_object aniObj, ani_object err);

    void ReplyFailedInner(InsightIntentInnerErr innerErr = InsightIntentInnerErr::INSIGHT_INTENT_EXECUTE_REPLY_FAILED);
    void ReplySucceededInner(std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> resultCpp);
    bool HandleEtsResultReturned(ani_env *env, ani_ref result);
    bool ExecuteIntentCheckError();
    bool CallPromise(ani_env *env, ani_ref promise);

    ETSRuntime &runtime_;
    State state_ = State::CREATED;
    std::unique_ptr<AppExecFwk::ETSNativeReference> etsObj_ = nullptr;
    std::unique_ptr<EtsInsightIntentQueryEntityCallback> queryCallback_;
};
} // namespace OHOS::AbilityRuntime

#endif // OHOS_ABILITY_RUNTIME_ETS_INSIGHT_INTENT_QUERY_ENTITY_H