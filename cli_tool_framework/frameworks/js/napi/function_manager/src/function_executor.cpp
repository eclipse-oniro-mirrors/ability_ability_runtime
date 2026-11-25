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

#include "function_executor.h"

#include "cli_error_code.h"
#include "consent_gate.h"
#include "errors.h"
#include "ffrt.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {

FunctionExecutor::FunctionExecutor(InvokeSession session, FunctionInfo functionInfo,
    std::unique_ptr<ConsentGate> consentGate)
    : session_(std::move(session)), functionInfo_(std::move(functionInfo)),
      consentGate_(std::move(consentGate))
{
}

FunctionExecutor::~FunctionExecutor() = default;

void FunctionExecutor::Execute()
{
    auto self = shared_from_this();
    RequestConsent([self](bool granted, int32_t intentRecordId) {
        if (granted) {
            // The pre-registered record id the consent flow returned: the
            // intent executor reuses it when assembling the execution param.
            self->session_.intentRecordId = intentRecordId;
            self->DispatchInvoke();
            return;
        }
        TAG_LOGW(AAFwkTag::CLI_TOOL, "consent not granted");
        // The common OHOS code, not CliTool's 35700007.
        self->Fail(OHOS::ERR_PERMISSION_DENIED);
    });
}

void FunctionExecutor::RequestConsent(const ConsentContinuation &cont)
{
    if (consentGate_ == nullptr) {
        cont(true, 0);
        return;
    }
    consentGate_->RequestConsent(session_, cont);
}

void FunctionExecutor::DispatchInvoke()
{
    // The consent decision may arrive from the JS/UI main thread (dialog);
    // execution (IPC) always runs on a worker thread.
    PostToWorker([self = shared_from_this()]() {
        // Guard against dispatch-after-settle: the invocation may already be
        // settled (30s timeout, or a late consent decision racing a decline) —
        // server-side execution whose outcome the caller can no longer receive
        // must not be dispatched.
        if (self->session_.Settled()) {
            TAG_LOGW(AAFwkTag::CLI_TOOL, "invoke skipped: invocation already settled");
            return;
        }
        ErrCode err = self->Invoke();
        if (err != ERR_OK) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "invoke failed: %{public}d", err);
            self->Fail(err);
        }
    });
}

void FunctionExecutor::Fail(int32_t errorCode)
{
    session_.Fail(errorCode);
}

void FunctionExecutor::PostToWorker(std::function<void()> task)
{
    ffrt::submit(std::move(task));
}

bool FunctionExecutor::PostToMain(std::function<void()> task)
{
    if (task == nullptr) {
        return false;
    }
    if (session_.mainPoster == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "no main-thread channel, task not run");
        return false;
    }
    if (!session_.mainPoster(std::move(task))) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "main-thread post failed, task not run");
        return false;
    }
    return true;
}
} // namespace CliTool
} // namespace OHOS
