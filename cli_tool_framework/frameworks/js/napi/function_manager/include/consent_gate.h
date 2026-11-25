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

#ifndef OHOS_CLI_TOOL_CONSENT_GATE_H
#define OHOS_CLI_TOOL_CONSENT_GATE_H

#include "invoke_session.h"

namespace OHOS {
namespace CliTool {

/**
 * @brief Authorization strategy of a public invocation, selected by the
 *        FunctionExecutorFactory from the entry AND the function type.
 *
 * The consent stage applies to the public entry (invokePublicFunction) only:
 * the system entry gets NO gate at all — its authorization happens on the
 * service side inside ExecuteIntentByFunctionCall — and the executor's
 * default consent implementation passes straight through on a null gate. The
 * public gate may vary by functionType, so the factory is the single
 * composition point that maps (entry, type) to the concrete gate.
 *
 * Threading contract: RequestConsent starts on the worker thread and hops to
 * the JS/UI main thread (via session.mainPoster) when user-facing UI is
 * required.
 */
class ConsentGate {
public:
    virtual ~ConsentGate() = default;

    /**
     * @brief Obtain the user's consent for the invocation. The default
     *        implementation grants immediately (no user-facing authorization
     *        on this path). Implementations must not capture the session (or
     *        anything session-owned) beyond the call: the continuation alone
     *        carries the ownership of the owning executor.
     * @param session The invocation session (inputs only).
     * @param cont Invoked exactly once with the decision and, on grant, the
     *             pre-registered execute record id; may be called from the
     *             main thread.
     */
    virtual void RequestConsent(InvokeSession &, const ConsentContinuation &cont)
    {
        cont(true, 0);
    }
};

/**
 * @brief Public entry (invokePublicFunction): the consent-gated path.
 *
 * The gate reuses the shared IntentPreprocessor pipeline: service-side
 * pre-registration, the automation-control shortcut (an explicit pre-grant
 * skips the dialog) and the privacy-center consent dialog mounted on the
 * caller's UIContent. A decline, dialog failure or dismissal cleans up the
 * pre-registered record inside the preprocessor before the decision settles;
 * on grant the decision rides the continuation (with the record id), and the
 * executor chain stores the id into the session for the intent executor.
 *
 * The public entry must carry a UIAbilityContext (the consent dialog host):
 * the NAPI layer throws on a context-less public invocation up front, and the
 * defensive guards here settle fail-closed if the contract is ever bypassed.
 */
class PublicConsentGate final : public ConsentGate {
public:
    void RequestConsent(InvokeSession &session, const ConsentContinuation &cont) override;
};
} // namespace CliTool
} // namespace OHOS

#endif // OHOS_CLI_TOOL_CONSENT_GATE_H
