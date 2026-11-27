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

#include "consent_gate.h"

#include "errors.h"
#include "hilog_tag_wrapper.h"
#include "intent_preprocessor.h"

namespace OHOS {
namespace CliTool {

void PublicConsentGate::RequestConsent(InvokeSession &session, const ConsentContinuation &cont)
{
    // Inputs are copied by value: only the continuation (which holds the
    // owning executor) outlives the call — see ConsentGate for the contract.
    auto context = session.abilityContext;
    auto poster = session.mainPoster;
    std::string bundleName = session.param.functionNamespace;
    std::string intentName = session.param.functionName;
    // Defensive fail-closed guards: the NAPI layer already throws on a public
    // invocation without these; reaching here means the contract was bypassed.
    if (context == nullptr || poster == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "public invoke has no consent dialog channel");
        cont(false, 0);
        return;
    }
    // The preprocessor mounts the consent UI on the caller's UIContent, so it
    // must run on the JS/UI main thread; the decision settles from there (the
    // base chain re-posts execution to a worker thread on grant). A failed
    // post means the task never runs — fail-closed immediately.
    bool posted = poster([context, bundleName = std::move(bundleName), intentName = std::move(intentName),
        cont]() {
        ErrCode ret = AbilityRuntime::IntentPreprocessor::PreprocessIntentExecute(context, bundleName,
            intentName, [cont](bool granted, int32_t intentId) { cont(granted, intentId); });
        if (ret != ERR_OK) {
            // PreprocessIntentExecute never invokes its callback on failure:
            // the service-side pre-registration or the dialog mount failed —
            // fail-closed either way.
            TAG_LOGE(AAFwkTag::CLI_TOOL, "PreprocessIntentExecute failed: %{public}d", ret);
            cont(false, 0);
        }
    });
    if (!posted) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "public invoke consent post failed");
        cont(false, 0);
    }
}
} // namespace CliTool
} // namespace OHOS
