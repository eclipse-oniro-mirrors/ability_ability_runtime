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

#ifndef OHOS_ABILITY_RUNTIME_START_SPECIFIED_ABILITY_RESPONSE_STUB_H
#define OHOS_ABILITY_RUNTIME_START_SPECIFIED_ABILITY_RESPONSE_STUB_H

#include "iremote_stub.h"
#include "nocopyable.h"
#include "string_ex.h"
#include "istart_specified_ability_response.h"

namespace OHOS {
namespace AppExecFwk {
class StartSpecifiedAbilityResponseStub : public IRemoteStub<IStartSpecifiedAbilityResponse> {
public:
    StartSpecifiedAbilityResponseStub() = default;
    virtual ~StartSpecifiedAbilityResponseStub() = default;

    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    int32_t HandleOnAcceptWantResponse(MessageParcel &data, MessageParcel &reply);
    int32_t HandleOnTimeoutResponse(MessageParcel &data, MessageParcel &reply);
    int32_t HandleOnNewProcessRequestResponse(MessageParcel &data, MessageParcel &reply);
    int32_t HandleOnNewProcessRequestTimeoutResponse(MessageParcel &data, MessageParcel &reply);
    int32_t HandleOnStartSpecifiedFailed(MessageParcel &data, MessageParcel &reply);

    DISALLOW_COPY_AND_MOVE(StartSpecifiedAbilityResponseStub);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_START_SPECIFIED_ABILITY_RESPONSE_STUB_H
