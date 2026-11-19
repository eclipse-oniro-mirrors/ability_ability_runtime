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

#ifndef OHOS_ABILITY_RUNTIME_ETS_INSIGHT_INTENT_UTILS_H
#define OHOS_ABILITY_RUNTIME_ETS_INSIGHT_INTENT_UTILS_H

#include <cstdint>
#include <memory>

#include "ets_native_reference.h"
#include "ets_runtime.h"
#include "insight_intent_executor.h"

namespace OHOS::AbilityRuntime {
class EtsInsightIntentUtils final {
public:
    enum class State : uint8_t { INVALID, CREATED, INITIALIZED, EXECUTING, EXECUTATION_DONE, DESTROYED };

    static void DeleteReference(ETSRuntime &runtime, const std::unique_ptr<AppExecFwk::ETSNativeReference> &ref);

    static ani_ref CallObjectMethod(ETSRuntime &runtime, const std::unique_ptr<AppExecFwk::ETSNativeReference> &etsObj,
        bool withResult, const char *name, const char *signature, ...);

    static std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> GetResultFromEts(
        ani_env *env, ani_ref result, bool isDecorator = false);

    static void ReplyFailed(InsightIntentExecutorAsyncCallback *callback,
        InsightIntentInnerErr innerErr = InsightIntentInnerErr::INSIGHT_INTENT_EXECUTE_REPLY_FAILED);

    static void ReplySucceeded(InsightIntentExecutorAsyncCallback *callback,
        std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> resultCpp);
};
} // namespace OHOS::AbilityRuntime

#endif // OHOS_ABILITY_RUNTIME_ETS_INSIGHT_INTENT_UTILS_H
