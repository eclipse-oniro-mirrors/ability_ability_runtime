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
#include <mutex>
#include <string>

#include "insight_intent_callback_interface.h"
#include "iremote_object.h"
#include "refbase.h"
#include "want_params.h"
#include "function_call_execute_param.h"

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

    ~IntentClient() = default;

    /**
     * @brief Execute intent by function call with simplified parameters.
     * @param param The execute intent parameters.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t ExecuteIntentByFunctionCall(const ExecuteIntentParam &param);

    /**
     * @brief Pre-register the insight intent execute record for the
     *        invokePublicFunction caller (third-party). The AMS resolves the
     *        target intent module by {bundleName, intentName}, checks the
     *        exported visibility flag and ACCESS_PUBLIC_FUNCTION, then
     *        pre-registers a PENDING_CONSENT record and returns its id for the
     *        consent dialog to query.
     * @param bundleName The target function namespace (bundle name).
     * @param intentName The target insight intent (function) name.
     * @param intentId Output execute record id pre-registered by AMS; non-zero
     *                 only when ERR_OK is returned.
     * @return Returns ERR_OK on success, INTENT_NOT_EXIST when the target is
     *         missing or not open to third-party callers, ERR_PERMISSION_DENIED
     *         when ACCESS_PUBLIC_FUNCTION is absent.
     */
    int32_t PreRegisterIntentRecord(const std::string &bundleName, const std::string &intentName,
        int32_t &intentId);

    /**
     * @brief Remove the pre-registered insight intent execute record identified
     *        by intentId. Called when the consent dialog is declined or dismissed
     *        (no grant) so the PENDING_CONSENT record does not linger.
     * @param intentId The execute record id returned by PreRegisterIntentRecord.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t RemoveIntentRecord(int32_t intentId);

private:
    IntentClient() = default;

    sptr<IRemoteObject> GetAbilityManagerRemote();

    void ResetRemote(wptr<IRemoteObject> remote);

    int32_t SendExecuteRequest(uint32_t code, uint64_t key, const sptr<IRemoteObject> &callerToken,
        const AppExecFwk::FunctionCallExecuteParam &param);

    class DeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
    };

    std::mutex mutex_;
    sptr<IRemoteObject> remoteObj_;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_;
};

} // namespace AAFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_INTENT_CLIENT_H
