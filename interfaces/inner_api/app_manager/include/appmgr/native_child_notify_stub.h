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

#ifndef OHOS_ABILITY_RUNTIME_NATIVE_CHILD_NOTIFY_STUB_H
#define OHOS_ABILITY_RUNTIME_NATIVE_CHILD_NOTIFY_STUB_H

#include "native_child_notify_interface.h"
#include "iremote_stub.h"

namespace OHOS {
namespace AppExecFwk {

class NativeChildNotifyStub : public IRemoteStub<INativeChildNotify> {
public:
    NativeChildNotifyStub() = default;
    virtual ~NativeChildNotifyStub() = default;

    int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    int32_t HandleOnNativeChildStarted(MessageParcel &data, MessageParcel &reply);
    int32_t HandleOnError(MessageParcel &data, MessageParcel &reply);
    int32_t HandleOnNativeChildExit(MessageParcel &data, MessageParcel &reply);
};

} // OHOS
} // AppExecFwk

#endif // OHOS_ABILITY_RUNTIME_NATIVE_CHILD_NOTIFY_STUB_H