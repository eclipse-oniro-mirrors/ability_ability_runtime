/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#include "intent_client.h"

#include "errors.h"
#include "insight_intent_execute_result.h"

namespace OHOS {
namespace AAFwk {

IntentClient &IntentClient::GetInstance()
{
    static IntentClient instance;
    return instance;
}

int32_t IntentClient::ExecuteIntentByFunctionCall(const ExecuteIntentParam &param)
{
    if (mockStatus_ != ERR_OK) {
        return mockStatus_;  // simulate a synchronous execute failure (IPC/auth/etc.)
    }
    if (param.callback != nullptr) {
        AppExecFwk::InsightIntentExecuteResult result;
        result.code = 0;  // app-level success
        param.callback->ProcessInsightIntentExecute(0, result);
    }
    return ERR_OK;
}

} // namespace AAFwk
} // namespace OHOS
