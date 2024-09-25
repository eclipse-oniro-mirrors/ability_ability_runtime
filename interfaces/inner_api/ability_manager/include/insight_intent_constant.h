/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_CONSTANT_H
#define OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_CONSTANT_H

#include "ability_manager_errors.h"

namespace OHOS::AbilityRuntime {
    enum class InsightIntentExecuteMode {
        UIABILITY_FOREGROUND,
        UIABILITY_BACKGROUND,
        UIEXTENSION_ABILITY,
        SERVICE_EXTENSION_ABILITY,
        // Use default enum value and keep `Count` as the last element
        Count
    };

    enum InsightIntentInnerErr {
        INSIGHT_INTENT_ERR_OK,
        INSIGHT_INTENT_INTERNAL_ERROR,
        INSIGHT_INTENT_EXECUTE_REPLY_FAILED = AAFwk::ERR_INSIGHT_INTENT_EXECUTE_REPLY_FAILED,
    };
} // namespace OHOS::AbilityRuntime

#endif // OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_CONSTANT_H
