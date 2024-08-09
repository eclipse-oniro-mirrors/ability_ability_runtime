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

#ifndef OHOS_ABILITY_RUNTIME_WINDOW_MANAGER_SERVICE_HANDLER_STUB_H
#define OHOS_ABILITY_RUNTIME_WINDOW_MANAGER_SERVICE_HANDLER_STUB_H

#ifdef SUPPORT_SCREEN

#include <map>

#include "iremote_stub.h"
#include "message_parcel.h"
#include "nocopyable.h"
#include "window_manager_service_handler.h"

namespace OHOS {
namespace AAFwk {
class WindowManagerServiceHandlerStub : public IRemoteStub<IWindowManagerServiceHandler> {
public:
    WindowManagerServiceHandlerStub();
    virtual ~WindowManagerServiceHandlerStub();

    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    void Init();
    int NotifyWindowTransitionInner(MessageParcel &data, MessageParcel &reply);
    int GetFocusWindowInner(MessageParcel &data, MessageParcel &reply);
    int StartingWindowCold(MessageParcel &data, MessageParcel &reply);
    int StartingWindowHot(MessageParcel &data, MessageParcel &reply);
    int CancelStartingWindowInner(MessageParcel &data, MessageParcel &reply);
    int NotifyAnimationAbilityDiedInner(MessageParcel &data, MessageParcel &reply);
    int MoveMissionsToForegroundInner(MessageParcel &data, MessageParcel &reply);
    int MoveMissionsToBackgroundInner(MessageParcel &data, MessageParcel &reply);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif
#endif  // OHOS_ABILITY_RUNTIME_WINDOW_MANAGER_SERVICE_HANDLER_STUB_H
