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

#include "intent_preprocessor.h"

namespace OHOS {
namespace AbilityRuntime {

ErrCode IntentPreprocessor::mockStatus_ = ERR_OK;
bool IntentPreprocessor::mockGranted_ = false;
int32_t IntentPreprocessor::mockIntentId_ = 0;
bool IntentPreprocessor::preprocessorCalled_ = false;
std::string IntentPreprocessor::lastBundleName_;
std::string IntentPreprocessor::lastIntentName_;

ErrCode IntentPreprocessor::PreprocessIntentExecute(const std::shared_ptr<AbilityContext> &context,
    const std::string &targetBundleName, const std::string &targetIntentName, DecideCallback onDecided)
{
    preprocessorCalled_ = true;
    lastBundleName_ = targetBundleName;
    lastIntentName_ = targetIntentName;
    if (context == nullptr) {
        // No dialog host: fail synchronously without invoking the callback,
        // matching the real pipeline's mount-failure contract.
        return ERR_INVALID_VALUE;
    }
    if (mockStatus_ != ERR_OK) {
        return mockStatus_;
    }
    if (onDecided != nullptr) {
        onDecided(mockGranted_, mockIntentId_);
    }
    return ERR_OK;
}

} // namespace AbilityRuntime
} // namespace OHOS
