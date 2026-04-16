/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_REMOTE_ON_LISTENER_INTERFACE_H
#define OHOS_ABILITY_RUNTIME_REMOTE_ON_LISTENER_INTERFACE_H

#include <vector>
#include "iremote_broker.h"

namespace OHOS {
namespace AAFwk {
/**
 * @struct OnCallbackInfo
 * Structure to hold the callback information for remote on listener.
 */
struct OnCallbackInfo {
    uint32_t continueState = 0;
    std::string srcDeviceId;
    std::string bundleName;
    std::string continueType;
    std::string srcBundleName;
    std::vector<std::string> appIdentifiers;
};

/**
 * @class IRemoteOnListener
 * IRemoteOnListener is used to notify caller that remote device mission is changed.
 */
class IRemoteOnListener : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.aafwk.RemoteOnListener");

    /**
     * @brief When the remote device mission changed, AbilityMs notify the listener.
     *
     * @param info, callback information containing continue state, device info, etc.
     */
    virtual void OnCallback(const OnCallbackInfo &info) = 0;

    enum {
        // ipc id for OnCallback
        ON_CALLBACK = 0,
    };
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_REMOTE_MISSION_LISTENER_INTERFACE_H
