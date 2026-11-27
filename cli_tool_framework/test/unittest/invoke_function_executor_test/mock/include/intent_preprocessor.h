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

#ifndef OHOS_ABILITY_RUNTIME_INTENT_PREPROCESSOR_H
#define OHOS_ABILITY_RUNTIME_INTENT_PREPROCESSOR_H

#include <functional>
#include <memory>
#include <string>

#include "errors.h"

namespace OHOS {
namespace AbilityRuntime {

// Test stub of the caller-side UIAbilityContext: only addressable identity
// is needed, none of the real context surface.
class AbilityContext {
public:
    virtual ~AbilityContext() = default;
};

/**
 * @class IntentPreprocessor
 * @brief Trimmed test mock of the shared consent pipeline. Shadowed over the
 *        real header by the mock include path; records the call and settles
 *        the decision from the knobs below (synchronously, matching the
 *        automation-shortcut timing of the real pipeline).
 */
class IntentPreprocessor {
public:
    using DecideCallback = std::function<void(bool granted, int32_t intentId)>;

    static ErrCode PreprocessIntentExecute(const std::shared_ptr<AbilityContext> &context,
        const std::string &targetBundleName, const std::string &targetIntentName,
        DecideCallback onDecided);

    // Synchronous return of PreprocessIntentExecute; non-ERR_OK skips the
    // decide callback (fail-closed, same contract as the real pipeline).
    static ErrCode mockStatus_;
    // Decision handed to the decide callback when mockStatus_ is ERR_OK.
    static bool mockGranted_;
    static int32_t mockIntentId_;
    // Call capture (reset by the test fixture).
    static bool preprocessorCalled_;
    static std::string lastBundleName_;
    static std::string lastIntentName_;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_INTENT_PREPROCESSOR_H
