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

#ifndef OHOS_ABILITY_RUNTIME_ETS_INSIGHT_INTENT_FUNC_H
#define OHOS_ABILITY_RUNTIME_ETS_INSIGHT_INTENT_FUNC_H

#include "ets_native_reference.h"
#include "ets_runtime.h"
#include "insight_intent_executor.h"
#include "ets_insight_intent_utils.h"

namespace OHOS::AbilityRuntime {
class EtsInsightIntentFunc final : public InsightIntentExecutor {
public:
    static InsightIntentExecutor *Create(Runtime &runtime);
    explicit EtsInsightIntentFunc(ETSRuntime &runtime);
    EtsInsightIntentFunc(const EtsInsightIntentFunc&) = delete;
    EtsInsightIntentFunc(const EtsInsightIntentFunc&&) = delete;
    EtsInsightIntentFunc &operator=(const EtsInsightIntentFunc&) = delete;
    EtsInsightIntentFunc &operator=(const EtsInsightIntentFunc&&) = delete;
    ~EtsInsightIntentFunc() override;

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
    bool CreatePromiseHelper();
    bool BindPromiseHelperMethods(ani_env *env, ani_class helperCls);
    bool InitPromiseHelperReference(ani_env *env, ani_class helperCls);
    bool GetDecodedMethodParam(const std::string &encodedMethodParam,
        AppExecFwk::InsightIntentParam &methodParamInfo) const;
    bool GetMethodArg(ani_env *env, ani_object wantParams, ani_method recordGetMethod,
        const std::string &encodedMethodParam, ani_ref &valueRef);
    bool BuildMethodArgs(ani_env *env, const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
        std::vector<ani_value> &args);
    bool IsVoidReturnType(const std::shared_ptr<InsightIntentExecuteParam> &executeParam) const;
    bool ExecuteInsightIntent(ani_env *env, const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
        bool &isAsync);
    bool HandleExecuteResult(ani_env *env, ani_ref result, bool &isAsync);
    bool HandleVoidExecuteResult();
    static void ResolvePromiseCbCpp(ani_env *env, ani_object aniObj, ani_ref result);
    static void RejectPromiseCbCpp(ani_env *env, ani_object aniObj, ani_ref error);

    void ReplyFailedInner(InsightIntentInnerErr innerErr = InsightIntentInnerErr::INSIGHT_INTENT_EXECUTE_REPLY_FAILED);
    void ReplySucceededInner(std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> resultCpp);
    bool HandleResultReturnedFromEtsFunc(ani_env *env, ani_ref result, bool isAsync);
    bool ExecuteIntentCheckError();

    ETSRuntime &runtime_;
    State state_ = State::CREATED;
    std::unique_ptr<AppExecFwk::ETSNativeReference> etsObj_ = nullptr;
    std::unique_ptr<AppExecFwk::ETSNativeReference> promiseHelperObj_ = nullptr;
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback_;
    bool isAsync_ = false;
};
} // namespace OHOS::AbilityRuntime

#endif // OHOS_ABILITY_RUNTIME_ETS_INSIGHT_INTENT_FUNC_H
