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

#ifndef OHOS_ABILITY_RUNTIME_INTENT_CLIENT_H
#define OHOS_ABILITY_RUNTIME_INTENT_CLIENT_H

#include <memory>
#include <string>

#include "insight_intent_callback_interface.h"
#include "want_params.h"

namespace OHOS {
namespace AAFwk {

struct ExecuteIntentParam {
    std::string bundleName;
    std::string intentName;
    WantParams wantParam;
    std::shared_ptr<AbilityRuntime::InsightIntentExecuteCallbackInterface> callback;
};

class IntentClient {
public:
    static IntentClient &GetInstance();

    /**
     * @brief Execute intent by function call with simplified parameters.
     * @param param The execute intent parameters.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t ExecuteIntentByFunctionCall(const ExecuteIntentParam &param);

    int32_t mockStatus_ = 0;
};

} // namespace AAFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_INTENT_CLIENT_H
