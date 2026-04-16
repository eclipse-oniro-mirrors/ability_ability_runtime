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

#ifndef OHOS_ABILITY_RUNTIME_IMAGE_PROCESS_STATE_OBSERVER_STUB_H
#define OHOS_ABILITY_RUNTIME_IMAGE_PROCESS_STATE_OBSERVER_STUB_H

#include <map>
#include <mutex>

#include "image_process_state_observer_interface.h"
#include "iremote_stub.h"

namespace OHOS {
namespace AppExecFwk {
class ImageProcessStateObserverStub : public IRemoteStub<IImageProcessStateObserver> {
public:
    ImageProcessStateObserverStub();
    virtual ~ImageProcessStateObserverStub();

    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    int32_t HandleOnImageProcessStateChanged(MessageParcel &data, MessageParcel &reply);
    int32_t HandleOnForkAllWorkProcessFailed(MessageParcel &data, MessageParcel &reply);
    int32_t HandleOnPreForkAllWorkProcess(MessageParcel &data, MessageParcel &reply);

    DISALLOW_COPY_AND_MOVE(ImageProcessStateObserverStub);
};

class ImageProcessStateObserverRecipient : public IRemoteObject::DeathRecipient {
public:
    using RemoteDiedHandler = std::function<void(const wptr<IRemoteObject> &)>;
    explicit ImageProcessStateObserverRecipient(RemoteDiedHandler handler);
    virtual ~ImageProcessStateObserverRecipient() = default;
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;

private:
    RemoteDiedHandler handler_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_IMAGE_PROCESS_STATE_OBSERVER_STUB_H