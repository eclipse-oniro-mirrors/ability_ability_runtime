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

#include "modular_object_ability_connection.h"

#include <unistd.h>

#include "connection_manager.h"
#include "hilog_tag_wrapper.h"
#include "modular_object_connection_manager.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t DIED = -1;
} // namespace

void ModularObjectAbilityConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    TAG_LOGI(AAFwkTag::EXT,
        "OnAbilityConnectDone, bundleName:%{public}s, abilityName:%{public}s, resultCode:%{public}d",
        element.GetBundleName().c_str(), element.GetAbilityName().c_str(), resultCode);
    std::vector<sptr<AbilityConnectCallback>> callbacks;
    {
        std::lock_guard<std::mutex> lock(modularMutex_);
        callbacks = GetCallbackList();
        if (callbacks.empty()) {
            TAG_LOGW(AAFwkTag::EXT, "empty callbackList");
            return;
        }

        SetRemoteObject(remoteObject);
        SetResultCode(resultCode);
        SetConnectionState(CONNECTION_STATE_CONNECTED);
    }
    sptr<ModularObjectAbilityConnection> connection(this);
    if (ModularObjectConnectionManager::GetInstance().DisconnectNonexistentService(element, connection)) {
        TAG_LOGW(AAFwkTag::EXT, "No need onConnect callback");
        return;
    }

    auto item = callbacks.begin();
    while (item != callbacks.end()) {
        (*item)->OnAbilityConnectDone(element, remoteObject, resultCode);
        item++;
    }
}

void ModularObjectAbilityConnection::OnAbilityDisconnectDone(
    const AppExecFwk::ElementName &element, int resultCode)
{
    TAG_LOGI(AAFwkTag::EXT,
        "OnAbilityDisconnectDone, bundleName:%{public}s, abilityName:%{public}s, resultCode:%{public}d",
        element.GetBundleName().c_str(), element.GetAbilityName().c_str(), resultCode);
    std::vector<sptr<AbilityConnectCallback>> callbacks;
    {
        std::lock_guard<std::mutex> lock(modularMutex_);
        SetConnectionState(CONNECTION_STATE_DISCONNECTED);
        callbacks = GetCallbackList();
        if (callbacks.empty()) {
            TAG_LOGE(AAFwkTag::EXT, "empty callbackList");
            return;
        }
    }

    // if resultCode < 0 that means the service is dead
    if (resultCode == DIED) {
        sptr<ModularObjectAbilityConnection> connection(this);
        ModularObjectConnectionManager::GetInstance().RemoveConnection(connection);
        resultCode = DIED + 1;
    }

    auto item = callbacks.begin();
    while (item != callbacks.end()) {
        (*item)->OnAbilityDisconnectDone(element, resultCode);
        item++;
    }
    SetRemoteObject(nullptr);
}

} // namespace AbilityRuntime
} // namespace OHOS
