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

#ifndef OHOS_ABILITY_RUNTIME_SA_INTERCEPTOR_STUB_H
#define OHOS_ABILITY_RUNTIME_SA_INTERCEPTOR_STUB_H

#include "iremote_stub.h"
#include "sa_interceptor_interface.h"

namespace OHOS {
namespace AbilityRuntime {
class SAInterceptorStub : public IRemoteStub<ISAInterceptor> {
public:
    SAInterceptorStub();
    virtual ~SAInterceptorStub();

    virtual int32_t OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    int32_t HandleOnCheckStarting(MessageParcel &data, MessageParcel &reply);

    DISALLOW_COPY_AND_MOVE(SAInterceptorStub);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_SA_INTERCEPTOR_STUB_H
