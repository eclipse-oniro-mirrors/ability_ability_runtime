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

#include "connection_state_manager.h"

#include <fstream>

#include "app_mgr_interface.h"
#include "connection_observer_errors.h"
#include "global_constant.h"
#include "hilog_tag_wrapper.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AAFwk {
namespace {
static const int MAX_RETRY = 10;
static const int DELAY_TIME = 1000;
OHOS::sptr<OHOS::AppExecFwk::IAppMgr> GetAppMgr()
{
    OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        return nullptr;
    }
    OHOS::sptr<OHOS::IRemoteObject> object = systemAbilityManager->GetSystemAbility(OHOS::APP_MGR_SERVICE_ID);
    return OHOS::iface_cast<OHOS::AppExecFwk::IAppMgr>(object);
}
}
using namespace OHOS::AbilityRuntime;

ConnectionStateManager::ConnectionStateManager() {}

ConnectionStateManager::~ConnectionStateManager() {}

std::string ConnectionStateManager::GetProcessNameByPid(int32_t pid)
{
    return std::to_string(pid);
}

void ConnectionStateManager::Init(const std::shared_ptr<TaskHandlerWrap> &handler)
{
    if (!observerController_) {
        observerController_ = std::make_shared<ConnectionObserverController>();
    }
    handler_ = handler;
    if (!handler) {
        TAG_LOGW(AAFwkTag::CONNECTION, "invalid eventhandler");
        InitAppStateObserver();
        return;
    }
    auto initConnectionStateManagerTask = [weak = weak_from_this()]() {
        auto self = weak.lock();
        if (!self) {
            TAG_LOGW(AAFwkTag::CONNECTION, "invalid self pointer");
            return;
        }
        self->InitAppStateObserver();
    };
    handler->SubmitTask(initConnectionStateManagerTask, "InitConnectionStateManager");
}

int ConnectionStateManager::RegisterObserver(const sptr<AbilityRuntime::IConnectionObserver> &observer)
{
    if (!observerController_) {
        return ERR_SERVICE_NOT_INIT;
    }

    return observerController_->AddObserver(observer);
}

int ConnectionStateManager::UnregisterObserver(const sptr<AbilityRuntime::IConnectionObserver> &observer)
{
    if (!observerController_) {
        return ERR_SERVICE_NOT_INIT;
    }
    observerController_->RemoveObserver(observer);

    return 0;
}

void ConnectionStateManager::AddConnection(std::shared_ptr<ConnectionRecord> connectionRecord)
{
    std::shared_ptr<ConnectionObserverController> controller = observerController_;
    if (!controller) {
        return;
    }

    if (!connectionRecord) {
        TAG_LOGE(AAFwkTag::CONNECTION, "invalid connection record");
        return;
    }

    ConnectionData connectionData;
    if (!AddConnectionInner(connectionRecord, connectionData)) {
        TAG_LOGD(AAFwkTag::CONNECTION, "no need notify observers");
        return;
    }
    controller->NotifyExtensionConnected(connectionData);
}

void ConnectionStateManager::RemoveConnection(std::shared_ptr<ConnectionRecord> connectionRecord,
    bool isCallerDied)
{
    std::shared_ptr<ConnectionObserverController> controller = observerController_;
    if (!controller) {
        return;
    }

    if (!connectionRecord) {
        TAG_LOGE(AAFwkTag::CONNECTION, "invalid connection record");
        return;
    }

    // if caller died, notify at once.
    if (isCallerDied) {
        HandleCallerDied(connectionRecord->GetCallerPid());
        return;
    }

    ConnectionData connectionData;
    if (!RemoveConnectionInner(connectionRecord, connectionData)) {
        TAG_LOGD(AAFwkTag::CONNECTION, "no need notify observers");
        return;
    }
    controller->NotifyExtensionDisconnected(connectionData);
}

void ConnectionStateManager::AddDataAbilityConnection(const DataAbilityCaller &caller,
    const std::shared_ptr<DataAbilityRecord> &record)
{
    if (!CheckDataAbilityConnectionParams(caller, record)) {
        return;
    }

    ConnectionData connectionData;
    if (!AddDataAbilityConnectionInner(caller, record, connectionData)) {
        TAG_LOGW(AAFwkTag::CONNECTION, "no need notify observers");
        return;
    }
    observerController_->NotifyExtensionConnected(connectionData);
}

void ConnectionStateManager::RemoveDataAbilityConnection(const DataAbilityCaller &caller,
    const std::shared_ptr<DataAbilityRecord> &record)
{
    if (!CheckDataAbilityConnectionParams(caller, record)) {
        return;
    }

    ConnectionData connectionData;
    if (!RemoveDataAbilityConnectionInner(caller, record, connectionData)) {
        TAG_LOGW(AAFwkTag::CONNECTION, "no need notify observers");
        return;
    }
    observerController_->NotifyExtensionDisconnected(connectionData);
}

bool ConnectionStateManager::CheckDataAbilityConnectionParams(const DataAbilityCaller &caller,
    const std::shared_ptr<DataAbilityRecord> &record) const
{
    if (!observerController_) {
        return false;
    }

    if (!record) {
        TAG_LOGE(AAFwkTag::CONNECTION, "invalid data ability record");
        return false;
    }

    if (caller.callerPid == 0) {
        TAG_LOGE(AAFwkTag::CONNECTION, "invalid callerPid");
        return false;
    }

    return true;
}

void ConnectionStateManager::HandleDataAbilityDied(const std::shared_ptr<DataAbilityRecord> &record)
{
    if (!record) {
        TAG_LOGE(AAFwkTag::CONNECTION, "invalid data ability");
        return;
    }

    auto token = record->GetToken();
    if (!token) {
        TAG_LOGE(AAFwkTag::CONNECTION, "invalid token");
        return;
    }

    std::vector<AbilityRuntime::ConnectionData> allData;
    HandleDataAbilityDiedInner(token, allData);
    if (allData.empty()) {
        TAG_LOGW(AAFwkTag::CONNECTION, "empty allData");
        return;
    }

    std::shared_ptr<ConnectionObserverController> controller = observerController_;
    if (!controller) {
        return;
    }

    for (auto& item : allData) {
        controller->NotifyExtensionDisconnected(item);
    }
}

void ConnectionStateManager::HandleDataAbilityCallerDied(int32_t callerPid)
{
    if (callerPid <= 0) {
        TAG_LOGW(AAFwkTag::CONNECTION, "invalid callerPid");
        return;
    }

    HandleCallerDied(callerPid);
}

#ifdef WITH_DLP
void ConnectionStateManager::AddDlpManager(const std::shared_ptr<AbilityRecord> &dlpManger)
{
    if (!dlpManger) {
        return;
    }

    auto userId = dlpManger->GetOwnerMissionUserId();
    std::lock_guard<ffrt::mutex> guard(dlpLock_);
    auto it = dlpItems_.find(userId);
    if (it == dlpItems_.end()) {
        dlpItems_[userId] = std::make_shared<DlpStateItem>(dlpManger->GetUid(), dlpManger->GetPid());
    }
}

void ConnectionStateManager::RemoveDlpManager(const std::shared_ptr<AbilityRecord> &dlpManger)
{
    if (!dlpManger) {
        return;
    }

    std::lock_guard<ffrt::mutex> guard(dlpLock_);
    dlpItems_.erase(dlpManger->GetOwnerMissionUserId());
}

void ConnectionStateManager::AddDlpAbility(const std::shared_ptr<AbilityRecord> &dlpAbility)
{
    std::shared_ptr<ConnectionObserverController> controller = observerController_;
    if (!controller) {
        return;
    }

    DlpStateData dlpData;
    if (!HandleDlpAbilityInner(dlpAbility, true, dlpData)) {
        TAG_LOGD(AAFwkTag::CONNECTION, "no need report dlp opened conn state");
        return;
    }
    controller->NotifyDlpAbilityOpened(dlpData);
}

void ConnectionStateManager::RemoveDlpAbility(const std::shared_ptr<AbilityRecord> &dlpAbility)
{
    std::shared_ptr<ConnectionObserverController> controller = observerController_;
    if (!controller) {
        return;
    }

    DlpStateData dlpData;
    if (!HandleDlpAbilityInner(dlpAbility, false, dlpData)) {
        TAG_LOGD(AAFwkTag::CONNECTION, "no need report dlp closed conn state");
        return;
    }
    controller->NotifyDlpAbilityClosed(dlpData);
}
#endif // WITH_DLP

void ConnectionStateManager::HandleAppDied(int32_t pid)
{
    HandleCallerDied(pid);
}

#ifdef WITH_DLP
void ConnectionStateManager::GetDlpConnectionInfos(std::vector<AbilityRuntime::DlpConnectionInfo> &infos)
{
    std::lock_guard<ffrt::mutex> guard(dlpLock_);
    for (auto it = dlpItems_.begin(); it != dlpItems_.end(); it++) {
        auto item = it->second;
        if (!item) {
            continue;
        }

        AbilityRuntime::DlpConnectionInfo info;
        info.dlpUid = item->GetDlpUid();
        info.openedAbilityCount = item->GetOpenedAbilitySize();
        infos.emplace_back(info);
    }
}
#endif // WITH_DLP

void ConnectionStateManager::GetConnectionData(std::vector<AbilityRuntime::ConnectionData> &connectionData)
{
    std::lock_guard guard(stateLock_);
    for (const auto &stateItem : connectionStates_) {
        if (!stateItem.second) {
            TAG_LOGW(AAFwkTag::CONNECTION, "Unexpected null");
            continue;
        }

        std::vector<AbilityRuntime::ConnectionData> allConnectionData;
        stateItem.second->GenerateAllConnectionData(allConnectionData);
        connectionData.insert(connectionData.end(), allConnectionData.begin(), allConnectionData.end());
    }
    TAG_LOGD(AAFwkTag::CONNECTION, "GetConnectionData: %{public}zu", connectionData.size());
}

bool ConnectionStateManager::AddConnectionInner(std::shared_ptr<ConnectionRecord> connectionRecord,
    AbilityRuntime::ConnectionData &data)
{
    std::shared_ptr<ConnectionStateItem> targetItem = nullptr;
    auto callerPid = connectionRecord->GetCallerPid();
    std::lock_guard<ffrt::mutex> guard(stateLock_);
    auto it = connectionStates_.find(callerPid);
    if (it == connectionStates_.end()) {
        targetItem = ConnectionStateItem::CreateConnectionStateItem(connectionRecord);
        if (targetItem) {
            connectionStates_[callerPid] = targetItem;
        }
    } else {
        targetItem = it->second;
    }

    if (!targetItem) {
        TAG_LOGE(AAFwkTag::CONNECTION, "find targetItem failed");
        return false;
    }

    return targetItem->AddConnection(connectionRecord, data);
}

bool ConnectionStateManager::RemoveConnectionInner(std::shared_ptr<ConnectionRecord> connectionRecord,
    AbilityRuntime::ConnectionData &data)
{
    auto callerPid = connectionRecord->GetCallerPid();
    std::lock_guard<ffrt::mutex> guard(stateLock_);
    auto it = connectionStates_.find(callerPid);
    if (it == connectionStates_.end()) {
        TAG_LOGW(AAFwkTag::CONNECTION, "find target failed, callerPid:%{public}d", callerPid);
        return false;
    }

    auto targetItem = it->second;
    if (!targetItem) {
        TAG_LOGE(AAFwkTag::CONNECTION, "find targetItem failed");
        return false;
    }

    bool result = targetItem->RemoveConnection(connectionRecord, data);
    if (result && targetItem->IsEmpty()) {
        connectionStates_.erase(it);
    }
    return result;
}

void ConnectionStateManager::HandleCallerDied(int32_t callerPid)
{
    auto connectionStateItem = RemoveDiedCaller(callerPid);
    if (!connectionStateItem) {
        TAG_LOGD(AAFwkTag::CONNECTION, "no connectionStateItem");
        return;
    }

    std::vector<AbilityRuntime::ConnectionData> allConnectionData;
    connectionStateItem->GenerateAllConnectionData(allConnectionData);
    if (allConnectionData.empty()) {
        TAG_LOGW(AAFwkTag::CONNECTION, "empty allConnectionData");
        return;
    }

    std::shared_ptr<ConnectionObserverController> controller = observerController_;
    if (!controller) {
        return;
    }

    for (auto& connectionData : allConnectionData) {
        controller->NotifyExtensionDisconnected(connectionData);
    }
}

std::shared_ptr<ConnectionStateItem> ConnectionStateManager::RemoveDiedCaller(int32_t callerPid)
{
    std::lock_guard<ffrt::mutex> guard(stateLock_);
    auto it = connectionStates_.find(callerPid);
    if (it == connectionStates_.end()) {
        TAG_LOGW(AAFwkTag::CONNECTION, "callerPid:%{public}d.", callerPid);
        return nullptr;
    }
    auto stateItem = it->second;
    (void)connectionStates_.erase(it);

    return stateItem;
}

bool ConnectionStateManager::AddDataAbilityConnectionInner(const DataAbilityCaller &caller,
    const std::shared_ptr<DataAbilityRecord> &record, ConnectionData &data)
{
    std::shared_ptr<ConnectionStateItem> targetItem = nullptr;
    std::lock_guard<ffrt::mutex> guard(stateLock_);
    auto it = connectionStates_.find(caller.callerPid);
    if (it == connectionStates_.end()) {
        targetItem = ConnectionStateItem::CreateConnectionStateItem(caller);
        if (targetItem) {
            connectionStates_[caller.callerPid] = targetItem;
        }
    } else {
        targetItem = it->second;
    }

    if (!targetItem) {
        TAG_LOGE(AAFwkTag::CONNECTION, "find targetItem failed");
        return false;
    }

    return targetItem->AddDataAbilityConnection(caller, record, data);
}

bool ConnectionStateManager::RemoveDataAbilityConnectionInner(const DataAbilityCaller &caller,
    const std::shared_ptr<DataAbilityRecord> &record, AbilityRuntime::ConnectionData &data)
{
    std::lock_guard<ffrt::mutex> guard(stateLock_);
    auto it = connectionStates_.find(caller.callerPid);
    if (it == connectionStates_.end()) {
        TAG_LOGW(AAFwkTag::CONNECTION, "find target item failed, callerPid:%{public}d", caller.callerPid);
        return false;
    }

    auto targetItem = it->second;
    if (!targetItem) {
        TAG_LOGE(AAFwkTag::CONNECTION, "find targetItem failed");
        return false;
    }

    bool result = targetItem->RemoveDataAbilityConnection(caller, record, data);
    if (result && targetItem->IsEmpty()) {
        connectionStates_.erase(it);
    }
    return result;
}

void ConnectionStateManager::HandleDataAbilityDiedInner(const sptr<IRemoteObject> &abilityToken,
    std::vector<AbilityRuntime::ConnectionData> &allData)
{
    std::lock_guard<ffrt::mutex> guard(stateLock_);
    for (auto it = connectionStates_.begin(); it != connectionStates_.end();) {
        auto item = it->second;
        if (!item) {
            connectionStates_.erase(it++);
            continue;
        }

        AbilityRuntime::ConnectionData data;
        if (item->HandleDataAbilityDied(abilityToken, data)) {
            allData.emplace_back(data);
        }

        if (item->IsEmpty()) {
            connectionStates_.erase(it++);
        } else {
            it++;
        }
    }
}

#ifdef WITH_DLP
bool ConnectionStateManager::HandleDlpAbilityInner(const std::shared_ptr<AbilityRecord> &dlpAbility,
    bool isAdd, AbilityRuntime::DlpStateData &dlpData)
{
    if (!dlpAbility) {
        TAG_LOGD(AAFwkTag::CONNECTION, "invalid dlp ability");
        return false;
    }

    if (dlpAbility->GetAppIndex() <= AbilityRuntime::GlobalConstant::MAX_APP_CLONE_INDEX) {
        TAG_LOGD(AAFwkTag::CONNECTION, " not dlp ability, do not report connection stat");
        return false;
    }

    std::lock_guard<ffrt::mutex> guard(dlpLock_);
    auto it = dlpItems_.find(dlpAbility->GetOwnerMissionUserId());
    if (it == dlpItems_.end()) {
        TAG_LOGW(AAFwkTag::CONNECTION, "invalid state");
        return false;
    }

    auto dlpItem = it->second;
    if (!dlpItem) {
        TAG_LOGW(AAFwkTag::CONNECTION, "invalid dlpItem");
        return false;
    }

    if (isAdd) {
        return dlpItem->AddDlpConnectionState(dlpAbility, dlpData);
    }

    return dlpItem->RemoveDlpConnectionState(dlpAbility, dlpData);
}
#endif // WITH_DLP

void ConnectionStateManager::InitAppStateObserver()
{
    if (appStateObserver_) {
        return;
    }

    sptr<OHOS::AppExecFwk::IAppMgr> appManager = GetAppMgr();
    if (!appManager) {
        TAG_LOGW(AAFwkTag::CONNECTION, "null appManager, retry:%{public}d", retry_);
        if (retry_ < MAX_RETRY && handler_) {
            auto initConnectionStateManagerTask = [weak = weak_from_this()]() {
                auto self = weak.lock();
                if (!self) {
                    TAG_LOGW(AAFwkTag::CONNECTION, "invalid self pointer");
                    return;
                }
                self->InitAppStateObserver();
            };
            handler_->SubmitTask(initConnectionStateManagerTask, "InitConnectionStateManager", DELAY_TIME);
            retry_++;
        }
        return;
    }

    appStateObserver_ = new (std::nothrow)InnerAppStateObserver([](int32_t pid) {
        DelayedSingleton<ConnectionStateManager>::GetInstance()->HandleAppDied(pid);
    });
    int32_t err = appManager->RegisterApplicationStateObserver(appStateObserver_);
    if (err != 0) {
        TAG_LOGE(AAFwkTag::CONNECTION, "register to appmanager err:%{public}d", err);
        appStateObserver_ = nullptr;
        return;
    }
}
} // namespace AAFwk
} // namespace OHOS
