/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_ISTART_SPECIFIED_ABILITY_RESPONSE_H
#define OHOS_ABILITY_RUNTIME_ISTART_SPECIFIED_ABILITY_RESPONSE_H

#include "iremote_broker.h"
#include "iremote_object.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
class IStartSpecifiedAbilityResponse : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.appexecfwk.startSpecifiedAbilityResponse");

    /**
     * @brief called when the module's onAcceptWant done to notify ability mgr to continue
     * @param want request param being accepted
     * @param flag specified flag return by application
     * @param requestId a number represents a request
     */
    virtual void OnAcceptWantResponse(const AAFwk::Want &want, const std::string &flag, int32_t requestId) = 0;

    /**
     * @brief called when the module's onAcceptWant happens time out
     * @param requestId a number represents a request
     */
    virtual void OnTimeoutResponse(int32_t requestId) = 0;

    virtual void OnNewProcessRequestResponse(const std::string &flag,
        int32_t requestId, const std::string &callerProcessName = "", int32_t recordId = 0) = 0;

    virtual void OnNewProcessRequestTimeoutResponse(int32_t requestId) = 0;

    virtual void OnStartSpecifiedFailed(int32_t requestId) {};

    enum Message {
        ON_ACCEPT_WANT_RESPONSE = 0,
        ON_TIMEOUT_RESPONSE,
        ON_NEW_PROCESS_REQUEST_RESPONSE,
        ON_NEW_PROCESS_REQUEST_TIMEOUT_RESPONSE,
        ON_START_SPECIFIED_FAILED
    };
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ISTART_SPECIFIED_ABILITY_RESPONSE_H
