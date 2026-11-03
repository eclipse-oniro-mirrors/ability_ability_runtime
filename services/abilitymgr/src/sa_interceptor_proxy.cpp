/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "sa_interceptor_proxy.h"

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
int32_t SAInterceptorProxy::OnCheckStarting(const std::string &params, Rule &rule)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(SAInterceptorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SAInterceptorProxy Write interface token failed.");
        return AAFwk::ERR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteString(params)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "OnCheckStarting Write params failed");
        return AAFwk::ERR_SA_INTERCEPTOR_WRITE_PARAMS_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null remote object");
        return AAFwk::ERR_NULL_IPC_REMOTE;
    }
    int result = remote->SendRequest(static_cast<int32_t>(ISAInterceptor::SAInterceptorCmd::ON_DO_CHECK_STARTING),
        data, reply, option);
    if (result != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, error: %{public}d", result);
        return AAFwk::ERR_NULL_IPC_SEND_REQUST_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "read result failed");
        return AAFwk::ERR_SA_INTERCEPTOR_READ_PARAMS_FAILED;
    }

    std::unique_ptr<Rule> value(reply.ReadParcelable<Rule>());
    if (value == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "read rule failed");
        return AAFwk::ERR_SA_INTERCEPTOR_READ_PARAMS_FAILED;
    }
    rule = *value;
    return result;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
