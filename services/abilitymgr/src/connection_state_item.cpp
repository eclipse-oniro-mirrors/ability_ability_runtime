/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include <algorithm>
#include "connection_state_item.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class ConnectedExtension
 * ConnectedExtension,This class is used to record a connected extension.
 */
class ConnectedExtension : public std::enable_shared_from_this<ConnectedExtension> {
public:
    static std::shared_ptr<ConnectedExtension> CreateConnectedExtension(std::shared_ptr<ConnectionRecord> record)
    {
        if (!record) {
            return nullptr;
        }

        auto targetExtension = record->GetAbilityRecord();
        if (!targetExtension) {
            return nullptr;
        }

        return std::make_shared<ConnectedExtension>(targetExtension);
    }

    ConnectedExtension()
    {
        extensionType_ = AppExecFwk::ExtensionAbilityType::UNSPECIFIED;
    }

    explicit ConnectedExtension(std::shared_ptr<BaseExtensionRecord> target)
    {
        if (!target) {
            return;
        }
        extensionPid_ = target->GetPid();
        extensionUid_ = target->GetUid();
        extensionBundleName_ = target->GetAbilityInfo().bundleName;
        extensionModuleName_ = target->GetAbilityInfo().moduleName;
        extensionName_ = target->GetAbilityInfo().name;
        extensionType_ = target->GetAbilityInfo().extensionAbilityType;
        if (target->GetAbilityInfo().type == AppExecFwk::AbilityType::SERVICE) {
            extensionType_ = AppExecFwk::ExtensionAbilityType::SERVICE;
        } else if (target->GetAbilityInfo().type == AppExecFwk::AbilityType::DATA) {
            extensionType_ = AppExecFwk::ExtensionAbilityType::DATASHARE;
        }
    }

    virtual ~ConnectedExtension() = default;

    bool AddConnection(sptr<IRemoteObject> connection, ConnectionEvent& event)
    {
        if (!connection) {
            return false;
        }

        std::lock_guard guard(connectionsMutex_);
        if (connections_.empty()) {
            event.connectedEvent = true;
            connections_.emplace(connection, false);
            return true;
        }

        bool needNotify = std::find_if(connections_.begin(), connections_.end(),
            [](const std::pair<sptr<IRemoteObject>, bool>& pair)->bool {return !pair.second;})
            == connections_.end();

        auto it = connections_.find(connection);
        if (it == connections_.end()) {
            connections_.emplace(connection, false);
        } else {
            (*it).second = false;
        }

        if (needNotify) {
            event.resumedEvent = true;
            return true;
        }
        return false;
    }

    bool RemoveConnection(sptr<IRemoteObject> connection, ConnectionEvent& event)
    {
        if (!connection) {
            return false;
        }
        std::lock_guard guard(connectionsMutex_);
        auto it = connections_.find(connection);
        if (it == connections_.end()) {
            return false;
        }

        bool isUnAvaliable = (*it).second;
        connections_.erase(connection);
        if (connections_.empty()) {
            event.disconnectedEvent = true;
            return true;
        }

        if (isUnAvaliable) {
            return false;
        }
        it = std::find_if(connections_.begin(), connections_.end(),
            [](const std::pair<sptr<IRemoteObject>, bool>& pair)->bool {return !pair.second;});
        if (it == connections_.end()) {
            event.suspendedEvent = true;
            return true;
        }
        return false;
    }

    bool SuspendConnection(sptr<IRemoteObject> connection)
    {
        if (!connection) {
            return false;
        }
        std::lock_guard guard(connectionsMutex_);
        auto it = connections_.find(connection);
        if (it == connections_.end()) {
            return false;
        }

        if ((*it).second) {
            return false;
        }
        (*it).second = true;
        it = std::find_if(connections_.begin(), connections_.end(),
            [](const std::pair<sptr<IRemoteObject>, bool>& pair)->bool {return !pair.second;});

        return it == connections_.end();
    }

    bool ResumeConnection(sptr<IRemoteObject> connection)
    {
        if (!connection) {
            return false;
        }
        std::lock_guard guard(connectionsMutex_);
        auto it = connections_.find(connection);
        if (it == connections_.end()) {
            return false;
        }

        bool needNotify = std::find_if(connections_.begin(), connections_.end(),
            [](const std::pair<sptr<IRemoteObject>, bool>& pair)->bool {return !pair.second;})
            == connections_.end();

        (*it).second = false;
        return needNotify;
    }

    void GenerateExtensionInfo(AbilityRuntime::ConnectionData &data)
    {
        {
            std::lock_guard guard(connectionsMutex_);
            auto it = std::find_if(connections_.begin(), connections_.end(),
                [](const std::pair<sptr<IRemoteObject>, bool
                    >& pair) {return !pair.second;});
            if (it == connections_.end()) {
                data.isSuspended = true;
            }
        }
        data.extensionPid = extensionPid_;
        data.extensionUid = extensionUid_;
        data.extensionBundleName = extensionBundleName_;
        data.extensionModuleName = extensionModuleName_;
        data.extensionName = extensionName_;
        data.extensionType = extensionType_;
    }

private:
    int32_t extensionPid_ = 0;
    int32_t extensionUid_ = 0;
    std::string extensionBundleName_;
    std::string extensionModuleName_;
    std::string extensionName_;
    AppExecFwk::ExtensionAbilityType extensionType_;

    std::mutex connectionsMutex_;
    std::map<sptr<IRemoteObject>, bool> connections_; // remote object of IAbilityConnection
};

/**
 * @class ConnectedDataAbility
 * ConnectedDataAbility,This class is used to record a connected data ability.
 */
class ConnectedDataAbility : public std::enable_shared_from_this<ConnectedDataAbility> {
public:
    static std::shared_ptr<ConnectedDataAbility> CreateConnectedDataAbility(
        const std::shared_ptr<DataAbilityRecord> &record)
    {
        if (!record) {
            return nullptr;
        }

        auto targetAbility = record->GetAbilityRecord();
        if (!targetAbility) {
            return nullptr;
        }

        return std::make_shared<ConnectedDataAbility>(targetAbility);
    }

    ConnectedDataAbility() {}

    explicit ConnectedDataAbility(const std::shared_ptr<AbilityRecord> &target)
    {
        if (!target) {
            return;
        }

        dataAbilityPid_ = target->GetPid();
        dataAbilityUid_ = target->GetUid();
        bundleName_ = target->GetAbilityInfo().bundleName;
        moduleName_ = target->GetAbilityInfo().moduleName;
        abilityName_ = target->GetAbilityInfo().name;
    }

    virtual ~ConnectedDataAbility() = default;

    bool AddCaller(const DataAbilityCaller &caller)
    {
        if (!caller.isNotHap && !caller.callerToken) {
            return false;
        }

        bool needNotify = callers_.empty();
        auto it = find_if(callers_.begin(), callers_.end(), [&caller](const std::shared_ptr<CallerInfo> &info) {
            if (caller.isNotHap) {
                return info && info->IsNotHap() && info->GetCallerPid() == caller.callerPid;
            } else {
                return info && info->GetCallerToken() == caller.callerToken;
            }
        });
        if (it == callers_.end()) {
            callers_.emplace_back(std::make_shared<CallerInfo>(caller.isNotHap, caller.callerPid, caller.callerToken));
        }

        return needNotify;
    }

    bool RemoveCaller(const DataAbilityCaller &caller)
    {
        if (!caller.isNotHap && !caller.callerToken) {
            return false;
        }

        auto it = find_if(callers_.begin(), callers_.end(), [&caller](const std::shared_ptr<CallerInfo> &info) {
            if (caller.isNotHap) {
                return info && info->IsNotHap() && info->GetCallerPid() == caller.callerPid;
            } else {
                return info && info->GetCallerToken() == caller.callerToken;
            }
        });
        if (it != callers_.end()) {
            callers_.erase(it);
        }

        return callers_.empty();
    }

    void GenerateExtensionInfo(AbilityRuntime::ConnectionData &data)
    {
        data.extensionPid = dataAbilityPid_;
        data.extensionUid = dataAbilityUid_;
        data.extensionBundleName = bundleName_;
        data.extensionModuleName = moduleName_;
        data.extensionName = abilityName_;
        data.extensionType = AppExecFwk::ExtensionAbilityType::DATASHARE;
    }

private:
    class CallerInfo : public std::enable_shared_from_this<CallerInfo> {
    public:
        CallerInfo(bool isNotHap, int32_t callerPid, const sptr<IRemoteObject> &callerToken)
            : isNotHap_(isNotHap), callerPid_(callerPid), callerToken_(callerToken) {}

        bool IsNotHap() const
        {
            return isNotHap_;
        }

        int32_t GetCallerPid() const
        {
            return callerPid_;
        }

        sptr<IRemoteObject> GetCallerToken() const
        {
            return callerToken_;
        }

    private:
        bool isNotHap_ = false;
        int32_t callerPid_ = 0;
        sptr<IRemoteObject> callerToken_ = nullptr;
    };

    int32_t dataAbilityPid_ = 0;
    int32_t dataAbilityUid_ = 0;
    std::string bundleName_;
    std::string moduleName_;
    std::string abilityName_;
    std::list<std::shared_ptr<CallerInfo>> callers_; // caller infos of this data ability.
};

ConnectionStateItem::ConnectionStateItem(int32_t callerUid, int32_t callerPid, const std::string &callerName)
    : callerUid_(callerUid), callerPid_(callerPid), callerName_(callerName)
{
}

ConnectionStateItem::~ConnectionStateItem()
{}

std::shared_ptr<ConnectionStateItem> ConnectionStateItem::CreateConnectionStateItem(
    const std::shared_ptr<ConnectionRecord> &record)
{
    if (!record) {
        return nullptr;
    }

    return std::make_shared<ConnectionStateItem>(record->GetCallerUid(),
        record->GetCallerPid(), record->GetCallerName());
}

std::shared_ptr<ConnectionStateItem> ConnectionStateItem::CreateConnectionStateItem(
    const DataAbilityCaller &dataCaller)
{
    return std::make_shared<ConnectionStateItem>(dataCaller.callerUid,
        dataCaller.callerPid, dataCaller.callerName);
}

bool ConnectionStateItem::AddConnection(std::shared_ptr<ConnectionRecord> record,
    AbilityRuntime::ConnectionData &data, ConnectionEvent &event)
{
    if (!record) {
        TAG_LOGE(AAFwkTag::CONNECTION, "invalid connection record");
        return false;
    }

    auto token = record->GetTargetToken();
    if (!token) {
        TAG_LOGE(AAFwkTag::CONNECTION, "invalid token");
        return false;
    }

    sptr<IRemoteObject> connectionObj = record->GetConnection();
    if (!connectionObj) {
        TAG_LOGE(AAFwkTag::CONNECTION, "no connection callback");
        return false;
    }

    std::shared_ptr<ConnectedExtension> connectedExtension = nullptr;
    auto it = connectionMap_.find(token);
    if (it == connectionMap_.end()) {
        connectedExtension = ConnectedExtension::CreateConnectedExtension(record);
        if (connectedExtension) {
            connectionMap_[token] = connectedExtension;
        }
    } else {
        connectedExtension = it->second;
    }

    if (!connectedExtension) {
        TAG_LOGE(AAFwkTag::CONNECTION, "invalid connectedExtension");
        return false;
    }

    bool needNotify = connectedExtension->AddConnection(connectionObj, event);
    if (needNotify) {
        GenerateConnectionData(connectedExtension, data);
    }

    return needNotify;
}

bool ConnectionStateItem::RemoveConnection(std::shared_ptr<ConnectionRecord> record,
    AbilityRuntime::ConnectionData &data, ConnectionEvent &event)
{
    if (!record) {
        TAG_LOGE(AAFwkTag::CONNECTION, "invalid connection record");
        return false;
    }

    auto token = record->GetTargetToken();
    if (!token) {
        TAG_LOGE(AAFwkTag::CONNECTION, "invalid token");
        return false;
    }

    sptr<IRemoteObject> connectionObj = record->GetConnection();
    if (!connectionObj) {
        TAG_LOGE(AAFwkTag::CONNECTION, "no connection callback");
        return false;
    }

    auto it = connectionMap_.find(token);
    if (it == connectionMap_.end()) {
        TAG_LOGE(AAFwkTag::CONNECTION, "no such connectedExtension");
        return false;
    }

    auto connectedExtension = it->second;
    if (!connectedExtension) {
        TAG_LOGE(AAFwkTag::CONNECTION, "no such connectedExtension");
        return false;
    }

    bool needNotify = connectedExtension->RemoveConnection(connectionObj, event);
    if (needNotify) {
        connectionMap_.erase(it);
        GenerateConnectionData(connectedExtension, data);
    }

    return needNotify;
}

bool ConnectionStateItem::SuspendConnection(std::shared_ptr<ConnectionRecord> record,
    AbilityRuntime::ConnectionData &data)
{
    if (!record) {
        TAG_LOGE(AAFwkTag::CONNECTION, "invalid connection record");
        return false;
    }

    auto token = record->GetTargetToken();
    if (!token) {
        TAG_LOGE(AAFwkTag::CONNECTION, "invalid token");
        return false;
    }

    sptr<IRemoteObject> connectionObj = record->GetConnection();
    if (!connectionObj) {
        TAG_LOGE(AAFwkTag::CONNECTION, "no connection callback");
        return false;
    }

    auto it = connectionMap_.find(token);
    if (it == connectionMap_.end()) {
        TAG_LOGE(AAFwkTag::CONNECTION, "no such connectedExtension");
        return false;
    }

    auto connectedExtension = it->second;
    if (!connectedExtension) {
        TAG_LOGE(AAFwkTag::CONNECTION, "no such connectedExtension");
        return false;
    }

    bool needNotify = connectedExtension->SuspendConnection(connectionObj);
    if (needNotify) {
        GenerateConnectionData(connectedExtension, data);
    }

    return needNotify;
}

bool ConnectionStateItem::ResumeConnection(std::shared_ptr<ConnectionRecord> record,
    AbilityRuntime::ConnectionData &data)
{
    if (!record) {
        TAG_LOGE(AAFwkTag::CONNECTION, "invalid connection record");
        return false;
    }

    auto token = record->GetTargetToken();
    if (!token) {
        TAG_LOGE(AAFwkTag::CONNECTION, "invalid token");
        return false;
    }

    sptr<IRemoteObject> connectionObj = record->GetConnection();
    if (!connectionObj) {
        TAG_LOGE(AAFwkTag::CONNECTION, "no connection callback");
        return false;
    }

    auto it = connectionMap_.find(token);
    if (it == connectionMap_.end()) {
        TAG_LOGE(AAFwkTag::CONNECTION, "no such connectedExtension");
        return false;
    }

    auto connectedExtension = it->second;
    if (!connectedExtension) {
        TAG_LOGE(AAFwkTag::CONNECTION, "no such connectedExtension");
        return false;
    }

    bool needNotify = connectedExtension->ResumeConnection(connectionObj);
    if (needNotify) {
        GenerateConnectionData(connectedExtension, data);
    }

    return needNotify;
}

bool ConnectionStateItem::AddDataAbilityConnection(const DataAbilityCaller &caller,
    const std::shared_ptr<DataAbilityRecord> &dataAbility, AbilityRuntime::ConnectionData &data)
{
    if (!dataAbility) {
        TAG_LOGE(AAFwkTag::CONNECTION, "invalid dataAbility");
        return false;
    }

    auto token = dataAbility->GetToken();
    if (!token) {
        TAG_LOGE(AAFwkTag::CONNECTION, "invalid dataAbility token");
        return false;
    }

    std::shared_ptr<ConnectedDataAbility> connectedAbility = nullptr;
    auto it = dataAbilityMap_.find(token);
    if (it == dataAbilityMap_.end()) {
        connectedAbility = ConnectedDataAbility::CreateConnectedDataAbility(dataAbility);
        if (connectedAbility) {
            dataAbilityMap_[token] = connectedAbility;
        }
    } else {
        connectedAbility = it->second;
    }

    if (!connectedAbility) {
        TAG_LOGE(AAFwkTag::CONNECTION, "invalid connectedAbility");
        return false;
    }

    bool needNotify = connectedAbility->AddCaller(caller);
    if (needNotify) {
        GenerateConnectionData(connectedAbility, data);
    }

    return needNotify;
}

bool ConnectionStateItem::RemoveDataAbilityConnection(const DataAbilityCaller &caller,
    const std::shared_ptr<DataAbilityRecord> &dataAbility, AbilityRuntime::ConnectionData &data)
{
    if (!dataAbility) {
        TAG_LOGE(AAFwkTag::CONNECTION, "invalid data ability record");
        return false;
    }

    auto token = dataAbility->GetToken();
    if (!token) {
        TAG_LOGE(AAFwkTag::CONNECTION, "invalid data ability token");
        return false;
    }

    auto it = dataAbilityMap_.find(token);
    if (it == dataAbilityMap_.end()) {
        TAG_LOGE(AAFwkTag::CONNECTION, "no such connected data ability");
        return false;
    }

    auto connectedDataAbility = it->second;
    if (!connectedDataAbility) {
        TAG_LOGE(AAFwkTag::CONNECTION, "no such connectedDataAbility");
        return false;
    }

    bool needNotify = connectedDataAbility->RemoveCaller(caller);
    if (needNotify) {
        dataAbilityMap_.erase(it);
        GenerateConnectionData(connectedDataAbility, data);
    }

    return needNotify;
}

bool ConnectionStateItem::HandleDataAbilityDied(const sptr<IRemoteObject> &token,
    AbilityRuntime::ConnectionData &data)
{
    if (!token) {
        return false;
    }

    auto it = dataAbilityMap_.find(token);
    if (it == dataAbilityMap_.end()) {
        TAG_LOGE(AAFwkTag::CONNECTION, "no such data ability");
        return false;
    }

    auto connectedDataAbility = it->second;
    if (!connectedDataAbility) {
        TAG_LOGE(AAFwkTag::CONNECTION, "no connectedDataAbility");
        return false;
    }

    dataAbilityMap_.erase(it);
    GenerateConnectionData(connectedDataAbility, data);
    return true;
}

bool ConnectionStateItem::IsEmpty() const
{
    return connectionMap_.empty() && dataAbilityMap_.empty();
}

void ConnectionStateItem::GenerateAllConnectionData(std::vector<AbilityRuntime::ConnectionData> &datas)
{
    AbilityRuntime::ConnectionData data;
    for (auto it = connectionMap_.begin(); it != connectionMap_.end(); ++it) {
        GenerateConnectionData(it->second, data);
        datas.emplace_back(data);
    }

    for (auto it = dataAbilityMap_.begin(); it != dataAbilityMap_.end(); ++it) {
        GenerateConnectionData(it->second, data);
        datas.emplace_back(data);
    }
}

void ConnectionStateItem::GenerateConnectionData(
    const std::shared_ptr<ConnectedExtension> &connectedExtension, AbilityRuntime::ConnectionData &data)
{
    if (connectedExtension) {
        connectedExtension->GenerateExtensionInfo(data);
    }
    data.callerUid = callerUid_;
    data.callerPid = callerPid_;
    data.callerName = callerName_;
}

void ConnectionStateItem::GenerateConnectionData(const std::shared_ptr<ConnectedDataAbility> &connectedDataAbility,
    AbilityRuntime::ConnectionData &data)
{
    if (connectedDataAbility) {
        connectedDataAbility->GenerateExtensionInfo(data);
    }
    data.callerUid = callerUid_;
    data.callerPid = callerPid_;
    data.callerName = callerName_;
}
}  // namespace AAFwk
}  // namespace OHOS
