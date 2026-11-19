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

#include "modular_object_connection_manager.h"

#include "ability_manager_client.h"
#include "hilog_tag_wrapper.h"
#include "modular_object_ability_connection.h"

namespace OHOS {
namespace AbilityRuntime {
ModularObjectConnectionManager &ModularObjectConnectionManager::GetInstance()
{
    static ModularObjectConnectionManager instance;
    return instance;
}

ErrCode ModularObjectConnectionManager::ConnectModularObjectExtension(const AAFwk::Want &want,
    const sptr<AbilityConnectCallback> &callback)
{
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "Invalid callback");
        return ERR_INVALID_VALUE;
    }
    sptr<ModularObjectAbilityConnection> abilityConnection = sptr<ModularObjectAbilityConnection>::MakeSptr();
    if (abilityConnection == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "Failed to create connection");
        return ERR_INVALID_VALUE;
    }
    abilityConnection->AddConnectCallback(callback);
    abilityConnection->SetConnectionState(CONNECTION_STATE_CONNECTING);
    ModularObjectConnectionInfo info(abilityConnection, want.GetOperation());
    {
        std::lock_guard<std::mutex> guard(connectionMutex_);
        auto &callbacks = connectionRecords_[info];
        callbacks.push_back(callback);
    }
    ErrCode ret = AAFwk::AbilityManagerClient::GetInstance()->ConnectAbilityWithExtensionType(
        want, abilityConnection, nullptr, AAFwk::DEFAULT_INVAL_VALUE,
        AppExecFwk::ExtensionAbilityType::MODULAR_OBJECT);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::EXT, "Connect failed: %{public}d", ret);
        std::lock_guard<std::mutex> guard(connectionMutex_);
        connectionRecords_.erase(info);
        return ret;
    }
    return ERR_OK;
}

ErrCode ModularObjectConnectionManager::DisconnectModularObjectExtension(
    const sptr<AbilityConnectCallback> &callback)
{
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "Invalid callback");
        return ERR_INVALID_VALUE;
    }

    sptr<AbilityConnection> abilityConnection;
    {
        std::lock_guard<std::mutex> guard(connectionMutex_);
        for (auto iter = connectionRecords_.begin(); iter != connectionRecords_.end(); ++iter) {
            auto &callbacks = iter->second;
            auto cbIter = std::find(callbacks.begin(), callbacks.end(), callback);
            if (cbIter != callbacks.end()) {
                abilityConnection = iter->first.abilityConnection;
                callbacks.erase(cbIter);
                if (callbacks.empty()) {
                    connectionRecords_.erase(iter);
                }
                break;
            }
        }
    }

    if (abilityConnection == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "Connection not found");
        return AAFwk::CONNECTION_NOT_EXIST;
    }

    ErrCode ret = AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(abilityConnection);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::EXT, "Disconnect failed: %{public}d", ret);
    }
    return ret;
}

bool ModularObjectConnectionManager::RemoveConnection(
    const sptr<ModularObjectAbilityConnection> &connection)
{
    std::lock_guard<std::mutex> lock(connectionMutex_);
    TAG_LOGD(AAFwkTag::EXT, "connectionRecordsSize: %{public}zu", connectionRecords_.size());

    bool isDisconnect = false;
    auto iter = connectionRecords_.begin();
    while (iter != connectionRecords_.end()) {
        ModularObjectConnectionInfo connectionInfo = iter->first;
        if (connectionInfo.abilityConnection == connection) {
            TAG_LOGD(AAFwkTag::EXT, "Remove connection");
            iter = connectionRecords_.erase(iter);
            isDisconnect = true;
        } else {
            ++iter;
        }
    }
    return isDisconnect;
}

bool ModularObjectConnectionManager::DisconnectNonexistentService(
    const AppExecFwk::ElementName &element,
    const sptr<ModularObjectAbilityConnection> &connection)
{
    bool exist = false;
    std::map<ModularObjectConnectionInfo, std::vector<sptr<AbilityConnectCallback>>> connectionRecords;
    {
        std::lock_guard<std::mutex> lock(connectionMutex_);
        connectionRecords = connectionRecords_;
    }
    TAG_LOGD(AAFwkTag::EXT, "connectionRecordsSize: %{public}zu", connectionRecords.size());

    for (auto &&record : connectionRecords) {
        ModularObjectConnectionInfo connectionInfo = record.first;
        if (connectionInfo.abilityConnection == connection &&
            connectionInfo.connectReceiver.GetBundleName() == element.GetBundleName()) {
            TAG_LOGD(AAFwkTag::EXT, "find connection");
            exist = true;
            break;
        }
    }
    if (!exist) {
        TAG_LOGE(AAFwkTag::EXT, "ext need disconnect");
        AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(connection);
        return true;
    }
    return false;
}
} // namespace AbilityRuntime
} // namespace OHOS
