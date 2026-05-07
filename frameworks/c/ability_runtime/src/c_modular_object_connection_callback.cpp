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

#include "c_modular_object_connection_callback.h"

#include <algorithm>
#include <cinttypes>

#include "c_modular_object_utils.h"
#include "hilog_tag_wrapper.h"
#include "ipc_inner_object.h"
#include "modular_object_connection_manager.h"
#include "modular_object_extension_types.h"
#include "want_manager.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
static std::map<ModularObjectConnectionKey, sptr<CModularObjectConnectionCallback>,
    ModularObjectConnectionKeyCompare> g_connectCallbacks;
static std::recursive_mutex g_connectCallbacksLock;
static int64_t g_serialNumber = 0;
} // namespace

namespace CModularObjectConnectionUtils {
int64_t InsertConnection(sptr<CModularObjectConnectionCallback> callback)
{
    std::lock_guard<std::recursive_mutex> lock(g_connectCallbacksLock);
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "null callback");
        return -1;
    }
    int64_t connectId = g_serialNumber;
    ModularObjectConnectionKey key;
    key.id = g_serialNumber;
    callback->SetConnectionId(connectId);
    g_connectCallbacks.emplace(key, callback);
    if (g_serialNumber < INT64_MAX) {
        g_serialNumber++;
    } else {
        g_serialNumber = 0;
    }
    TAG_LOGD(AAFwkTag::EXT, "Connection inserted, id: %{public}" PRId64, connectId);
    return connectId;
}

void RemoveConnectionCallback(int64_t connectionId)
{
    sptr<CModularObjectConnectionCallback> callback;
    std::lock_guard<std::recursive_mutex> lock(g_connectCallbacksLock);
    auto item = std::find_if(g_connectCallbacks.begin(), g_connectCallbacks.end(),
        [&connectionId](const auto &obj) { return connectionId == obj.first.id; });
    if (item != g_connectCallbacks.end()) {
        callback = item->second;
        g_connectCallbacks.erase(item);
    } else {
        TAG_LOGW(AAFwkTag::EXT, "Connection not found, id: %{public}" PRId64, connectionId);
    }
}

void FindConnection(int64_t connectionId, sptr<CModularObjectConnectionCallback> &callback)
{
    std::lock_guard<std::recursive_mutex> lock(g_connectCallbacksLock);
    auto item = std::find_if(g_connectCallbacks.begin(), g_connectCallbacks.end(),
        [&connectionId](const auto &obj) { return connectionId == obj.first.id; });
    if (item != g_connectCallbacks.end()) {
        callback = item->second;
    }
}
} // namespace CModularObjectConnectionUtils

CModularObjectConnectionCallback::CModularObjectConnectionCallback(
    const std::shared_ptr<OH_AbilityRuntime_ConnectOptionsState> &state)
    : state_(state)
{}

void CModularObjectConnectionCallback::OnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    TAG_LOGD(AAFwkTag::EXT, "ConnectDone:%{public}s, %{public}d", element.GetAbilityName().c_str(), resultCode);
    if (remoteObject == nullptr) {
        return;
    }
    auto state = state_.lock();
    if (state == nullptr) {
        return;
    }

    OH_AbilityRuntime_ConnectOptions_OnConnectCallback callback = nullptr;
    OH_AbilityRuntime_ConnectOptions *owner = nullptr;
    {
        std::lock_guard<std::mutex> guard(state->mutex);
        if (!state->alive) {
            return;
        }
        callback = state->onConnectCallback;
        owner = state->owner;
    }
    if (callback == nullptr) {
        return;
    }

    AbilityBase_Element cElement;
    if (!CModularObjectUtils::BuildElement(element, cElement)) {
        CModularObjectUtils::NotifyFailed(state, ABILITY_RUNTIME_ERROR_CODE_INTERNAL);
        return;
    }
    sptr<IRemoteObject> remoteObjectCopy = remoteObject;
    OHIPCRemoteProxy *proxy = CreateIPCRemoteProxy(remoteObjectCopy);
    if (proxy == nullptr) {
        CModularObjectUtils::DestroyElement(cElement);
        CModularObjectUtils::NotifyFailed(state, ABILITY_RUNTIME_ERROR_CODE_INTERNAL);
        return;
    }
    callback(owner, &cElement, proxy);
    CModularObjectUtils::DestroyElement(cElement);
}

void CModularObjectConnectionCallback::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element,
    int resultCode)
{
    TAG_LOGD(AAFwkTag::EXT, "DisconnectDone:%{public}s, %{public}d", element.GetAbilityName().c_str(), resultCode);
    auto state = state_.lock();
    if (state == nullptr) {
        TAG_LOGW(AAFwkTag::EXT, "state null");
        CModularObjectConnectionUtils::RemoveConnectionCallback(connectionId_);
        return;
    }
    OH_AbilityRuntime_ConnectOptions_OnDisconnectCallback callback = nullptr;
    OH_AbilityRuntime_ConnectOptions *owner = nullptr;
    {
        std::lock_guard<std::mutex> guard(state->mutex);
        if (state->alive) {
            callback = state->onDisconnectCallback;
            owner = state->owner;
        } else {
            TAG_LOGW(AAFwkTag::EXT, "state not alive");
        }
    }

    if (callback == nullptr) {
        TAG_LOGW(AAFwkTag::EXT, "callback null");
        return;
    }
    AbilityBase_Element cElement;
    if (CModularObjectUtils::BuildElement(element, cElement)) {
        callback(owner, &cElement);
        CModularObjectUtils::DestroyElement(cElement);
    } else {
        CModularObjectUtils::NotifyFailed(state, ABILITY_RUNTIME_ERROR_CODE_INTERNAL);
    }
}
} // namespace AbilityRuntime
} // namespace OHOS
