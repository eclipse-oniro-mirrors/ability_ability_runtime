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

#include <atomic>
#include <functional>
#include <memory>
#include <string>

#include "ability_context.h"

namespace OHOS {
namespace AAFwk {
class Want;
} // namespace AAFwk

namespace AbilityRuntime {

/**
 * Encapsulates one lifecycle of the third-party public intent invoke
 * (invokePublicFunction) pre-processing: service-side pre-registration ->
 * authorization gate (automation-control shortcut or privacy-center consent
 * dialog) -> authorization decision.
 *
 * On decline/dismiss (or when the consent dialog cannot be mounted) the
 * pre-registered execute record is cleaned up internally; on grant the caller
 * reuses the returned execute record id to actually execute the intent.
 */
class IntentPreprocessor : public std::enable_shared_from_this<IntentPreprocessor> {
public:
    // Settled with the authorization decision. When granted=true, intentId is
    // the pre-registered execute record id the caller must reuse to execute;
    // when granted=false the record has already been cleaned up and intentId is 0.
    using DecideCallback = std::function<void(bool granted, uint64_t intentId)>;

    // Sole public entry. Returns synchronously whether the pre-processing
    // pipeline started (pre-registration / consent dialog mount succeeded).
    // On failure onDecided is never invoked; on success the decision is settled
    // through onDecided (synchronously for the automation-control shortcut,
    // asynchronously for the consent dialog).
    static ErrCode PreprocessIntentExecute(const std::shared_ptr<AbilityContext> &context,
        const std::string &targetBundleName, const std::string &targetIntentName,
        DecideCallback onDecided);

private:
    IntentPreprocessor(std::shared_ptr<AbilityContext> context, std::string targetBundleName,
        std::string targetIntentName, DecideCallback onDecided);

    ErrCode Mount();
    bool TryAutomationControlShortcut() const;
    ErrCode ShowConsentDialog();
    void OnResult(int32_t code, const AAFwk::Want &result);
    void OnError(int32_t code, const std::string &name, const std::string &message);
    void OnRelease(int32_t code);
    void Close();
    void Settle(bool granted);
    static bool ParseGrantResult(const AAFwk::Want &result);

    std::shared_ptr<AbilityContext> context_;
    std::string targetBundleName_;
    std::string targetIntentName_;
    uint64_t intentId_ = 0;
    int32_t sessionId_ = 0;
    std::atomic<bool> settled_{ false };
    DecideCallback onDecided_;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_INTENT_PREPROCESSOR_H