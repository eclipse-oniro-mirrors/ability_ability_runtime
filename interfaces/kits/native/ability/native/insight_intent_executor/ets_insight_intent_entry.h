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

#ifndef OHOS_ABILITY_RUNTIME_ETS_INSIGHT_INTENT_ENTRY_H
#define OHOS_ABILITY_RUNTIME_ETS_INSIGHT_INTENT_ENTRY_H

#include "ets_insight_intent_utils.h"
#include "ets_native_reference.h"
#include "ets_runtime.h"
#include "insight_intent_executor.h"

namespace OHOS::AbilityRuntime {
class EtsInsightIntentContext;

class EtsInsightIntentEntry final : public InsightIntentExecutor {
public:
    static InsightIntentExecutor *Create(Runtime &runtime);
    explicit EtsInsightIntentEntry(ETSRuntime &runtime);
    EtsInsightIntentEntry(const EtsInsightIntentEntry &) = delete;
    EtsInsightIntentEntry(const EtsInsightIntentEntry &&) = delete;
    EtsInsightIntentEntry &operator=(const EtsInsightIntentEntry &) = delete;
    EtsInsightIntentEntry &operator=(const EtsInsightIntentEntry &&) = delete;
    ~EtsInsightIntentEntry() override;

    bool Init(const InsightIntentExecutorInfo &insightIntentInfo) override;

    bool HandleExecuteIntent(std::shared_ptr<InsightIntentExecuteParam> executeParam,
        const std::shared_ptr<NativeReference> &pageLoader,
        std::unique_ptr<InsightIntentExecutorAsyncCallback> callback, bool &isAsync) override
    {
        return false;
    }

    bool HandleExecuteIntent(std::shared_ptr<InsightIntentExecuteParam> executeParam, void *pageLoader,
        std::unique_ptr<InsightIntentExecutorAsyncCallback> callback, bool &isAsync) override;

private:
    using State = EtsInsightIntentUtils::State;

    static std::unique_ptr<AppExecFwk::ETSNativeReference> LoadEtsCode(
        const InsightIntentExecutorInfo &insightIntentInfo, ETSRuntime &runtime);
    static void OnExecuteResult(ani_env *env, ani_object aniObj, ani_object result);
    static void OnExecuteError(ani_env *env, ani_object aniObj, ani_object error);

    void ReplyFailedInner(InsightIntentInnerErr innerErr = InsightIntentInnerErr::INSIGHT_INTENT_EXECUTE_REPLY_FAILED);
    void ReplySucceededInner(std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> resultCpp);
    bool PrepareExecuteEnvironment(ani_env *env, InsightIntentExecuteMode mode, void *pageLoader);
    bool ExecuteInsightIntent(ani_env *env, bool &isAsync);
    bool HandleResultReturnedFromEtsFunc(ani_env *env, ani_ref result, bool isAsync);
    bool ExecuteIntentCheckError();
    bool AssignObject(ani_env *env, const AAFwk::WantParams &wantParams);
    bool HasDeclaredProperty(ani_env *env, const std::string &name);

    ETSRuntime &runtime_;
    State state_ = State::CREATED;
    std::unique_ptr<AppExecFwk::ETSNativeReference> etsObj_ = nullptr;
    std::unique_ptr<AppExecFwk::ETSNativeReference> contextObj_ = nullptr;
    std::shared_ptr<EtsInsightIntentContext> contextCpp_ = nullptr;
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback_;
    bool isAsync_ = false;
};
} // namespace OHOS::AbilityRuntime

#endif // OHOS_ABILITY_RUNTIME_ETS_INSIGHT_INTENT_ENTRY_H
