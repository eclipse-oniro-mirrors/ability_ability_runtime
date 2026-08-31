/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "kia_interceptor_stub.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
KiaInterceptorStub::KiaInterceptorStub() {}

KiaInterceptorStub::~KiaInterceptorStub() {}

int KiaInterceptorStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    TAG_LOGD(AAFwkTag::APPMGR, "cmd=%{public}d,flags=%{public}d", code, option.GetFlags());
    std::u16string descriptor = KiaInterceptorStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGI(AAFwkTag::APPMGR, "local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    if (code == KIA_INTERCEPTOR_ON_INTERCEPT) {
        return OnInterceptInner(data, reply);
    }
    TAG_LOGW(AAFwkTag::APPMGR, "KiaInterceptorStub::OnRemoteRequest, default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int KiaInterceptorStub::OnInterceptInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<AAFwk::Want> want = data.ReadParcelable<AAFwk::Want>();
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null want.");
        return ERR_INVALID_VALUE;
    }
    int resultCode = OnIntercept(*want);
    if (!reply.WriteInt32(resultCode)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write resultCode failed.");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteParcelable(want)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write want failed.");
        return ERR_INVALID_VALUE;
    }

    return NO_ERROR;
}
}  // namespace AppExecFwk
}  // namespace OHOS
