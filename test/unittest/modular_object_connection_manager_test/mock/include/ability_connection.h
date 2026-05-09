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

#ifndef MOCK_ABILITY_CONNECTION_H
#define MOCK_ABILITY_CONNECTION_H

#include <algorithm>
#include <mutex>
#include <vector>

#include "ability_connect_callback.h"
#include "refbase.h"

namespace OHOS {
namespace AbilityRuntime {

enum {
    CONNECTION_STATE_DISCONNECTED = -1,
    CONNECTION_STATE_CONNECTED = 0,
    CONNECTION_STATE_CONNECTING = 1
};

class AbilityConnection : public RefBase {
public:
    AbilityConnection() = default;
    virtual ~AbilityConnection() = default;

    virtual void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) {}

    virtual void OnAbilityDisconnectDone(
        const AppExecFwk::ElementName &element, int resultCode) {}

    void AddConnectCallback(const sptr<AbilityConnectCallback> &cb)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        callbackList_.push_back(cb);
    }

    void RemoveConnectCallback(const sptr<AbilityConnectCallback> &cb)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = std::find(callbackList_.begin(), callbackList_.end(), cb);
        if (it != callbackList_.end()) {
            callbackList_.erase(it);
        }
    }

    void SetRemoteObject(const sptr<IRemoteObject> &obj) { remoteObject_ = obj; }
    void SetResultCode(int code) { resultCode_ = code; }
    void SetConnectionState(int state) { connectionState_ = state; }
    sptr<IRemoteObject> GetRemoteObject() const { return remoteObject_; }
    int GetResultCode() const { return resultCode_; }
    int GetConnectionState() const { return connectionState_; }
    std::vector<sptr<AbilityConnectCallback>> GetCallbackList() { return callbackList_; }

private:
    std::vector<sptr<AbilityConnectCallback>> callbackList_;
    sptr<IRemoteObject> remoteObject_;
    int resultCode_ = -1;
    int connectionState_ = CONNECTION_STATE_DISCONNECTED;
    std::mutex mutex_;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // MOCK_ABILITY_CONNECTION_H
