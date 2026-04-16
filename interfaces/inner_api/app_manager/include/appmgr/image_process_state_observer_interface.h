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

#ifndef OHOS_ABILITY_RUNTIME_IMAGE_PROCESS_STATE_OBSERVER_INTERFACE_H
#define OHOS_ABILITY_RUNTIME_IMAGE_PROCESS_STATE_OBSERVER_INTERFACE_H

#include "image_process_state_data.h"
#include "iremote_broker.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
class IImageProcessStateObserver : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.appexecfwk.IImageProcessStateObserver");

    virtual void OnImageProcessStateChanged(const ImageProcessStateData& imageProcessStateData) {}

    virtual void OnForkAllWorkProcessFailed(const ImageProcessStateData& imageProcessStateData,
        int32_t errCode) {}

    virtual void OnPreForkAllWorkProcess(const ImageProcessStateData& imageProcessStateData) {}

    enum class Message {
        ON_IMAGE_PROCESS_STATE_CHANGED = 0,
        ON_FORKALL_WORK_PROCESS_FAILED = 1,
        ON_PRE_FORK_ALL_WORK_PROCESS = 2
    };
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_IMAGE_PROCESS_STATE_OBSERVER_INTERFACE_H