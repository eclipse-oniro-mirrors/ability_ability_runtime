/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "start_specified_ability_response_stub.h"
#include "appexecfwk_errors.h"
#include "hilog_tag_wrapper.h"
#include "ipc_types.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
using namespace std::placeholders;
StartSpecifiedAbilityResponseStub::StartSpecifiedAbilityResponseStub()
{
    auto handleOnAcceptWantResponse = [this](OHOS::MessageParcel &arg1, OHOS::MessageParcel &arg2) {
        return HandleOnAcceptWantResponse(arg1, arg2);
    };

    responseFuncMap_.emplace(static_cast<uint32_t>(
        IStartSpecifiedAbilityResponse::Message::ON_ACCEPT_WANT_RESPONSE),
        std::move(handleOnAcceptWantResponse));

    auto handleOnTimeoutResponse = [this](OHOS::MessageParcel &arg1, OHOS::MessageParcel &arg2) {
        return  HandleOnTimeoutResponse(arg1, arg2);
    };

    responseFuncMap_.emplace(static_cast<uint32_t>(
        IStartSpecifiedAbilityResponse::Message::ON_TIMEOUT_RESPONSE),
        std::move(handleOnTimeoutResponse));

    auto handleOnNewProcessRequestResponse = [this](OHOS::MessageParcel &arg1, OHOS::MessageParcel &arg2) {
        return HandleOnNewProcessRequestResponse(arg1, arg2);
    };

    responseFuncMap_.emplace(static_cast<uint32_t>(
        IStartSpecifiedAbilityResponse::Message::ON_NEW_PROCESS_REQUEST_RESPONSE),
        std::move(handleOnNewProcessRequestResponse));

    auto handleOnNewProcessRequestTimeoutResponse = [this](OHOS::MessageParcel &arg1, OHOS::MessageParcel &arg2) {
        return HandleOnNewProcessRequestTimeoutResponse(arg1, arg2);
    };

    responseFuncMap_.emplace(static_cast<uint32_t>(
        IStartSpecifiedAbilityResponse::Message::ON_NEW_PROCESS_REQUEST_TIMEOUT_RESPONSE),
        std::move(handleOnNewProcessRequestTimeoutResponse));
}

StartSpecifiedAbilityResponseStub::~StartSpecifiedAbilityResponseStub()
{
    responseFuncMap_.clear();
}

int32_t StartSpecifiedAbilityResponseStub::HandleOnAcceptWantResponse(MessageParcel &data, MessageParcel &reply)
{
    AAFwk::Want *want = data.ReadParcelable<AAFwk::Want>();
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "want is nullptr");
        return ERR_INVALID_VALUE;
    }

    auto flag = Str16ToStr8(data.ReadString16());
    OnAcceptWantResponse(*want, flag, data.ReadInt32());
    delete want;
    return NO_ERROR;
}

int32_t StartSpecifiedAbilityResponseStub::HandleOnTimeoutResponse(MessageParcel &data, MessageParcel &reply)
{
    AAFwk::Want *want = data.ReadParcelable<AAFwk::Want>();
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "want is nullptr");
        return ERR_INVALID_VALUE;
    }

    OnTimeoutResponse(*want, data.ReadInt32());
    delete want;
    return NO_ERROR;
}

int32_t StartSpecifiedAbilityResponseStub::HandleOnNewProcessRequestResponse(MessageParcel &data, MessageParcel &reply)
{
    AAFwk::Want *want = data.ReadParcelable<AAFwk::Want>();
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "want is nullptr");
        return ERR_INVALID_VALUE;
    }

    auto flag = Str16ToStr8(data.ReadString16());
    OnNewProcessRequestResponse(*want, flag, data.ReadInt32());
    delete want;
    return NO_ERROR;
}

int32_t StartSpecifiedAbilityResponseStub::HandleOnNewProcessRequestTimeoutResponse(MessageParcel &data,
    MessageParcel &reply)
{
    AAFwk::Want *want = data.ReadParcelable<AAFwk::Want>();
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "want is nullptr");
        return ERR_INVALID_VALUE;
    }

    OnNewProcessRequestTimeoutResponse(*want, data.ReadInt32());
    delete want;
    return NO_ERROR;
}

int StartSpecifiedAbilityResponseStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    TAG_LOGI(AAFwkTag::APPMGR, "StartSpecifiedAbilityResponseStub::OnReceived, code = %{public}u, flags= %{public}d.",
        code, option.GetFlags());
    std::u16string descriptor = StartSpecifiedAbilityResponseStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGE(AAFwkTag::APPMGR, "local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    auto itFunc = responseFuncMap_.find(code);
    if (itFunc != responseFuncMap_.end()) {
        auto func = itFunc->second;
        if (func != nullptr) {
            return func(data, reply);
        }
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}
}  // namespace AppExecFwk
}  // namespace OHOS
