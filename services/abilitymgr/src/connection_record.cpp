/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include "connection_record.h"

#include "ability_manager_service.h"
#include "ability_util.h"
#include "agent_extension_connection_constants.h"
#include "connection_state_manager.h"
#include "freeze_util.h"
#include "ui_service_extension_connection_constants.h"

namespace OHOS {
namespace AAFwk {
int64_t ConnectionRecord::connectRecordId = 0;
#ifdef SUPPORT_ASAN
const int DISCONNECT_TIMEOUT_MULTIPLE = 75;
#else
const int DISCONNECT_TIMEOUT_MULTIPLE = 1;
#endif

ConnectionRecord::ConnectionRecord(const sptr<IRemoteObject> &callerToken,
    const std::shared_ptr<BaseExtensionRecord> &targetService, const sptr<IAbilityConnection> &connCallback,
    std::shared_ptr<AbilityConnectManager> abilityConnectManager)
    : state_(ConnectionState::INIT),
      callerToken_(callerToken),
      targetService_(targetService),
      connCallback_(connCallback),
      abilityConnectManager_(abilityConnectManager)
{
    recordId_ = connectRecordId++;
}

ConnectionRecord::~ConnectionRecord()
{}

std::shared_ptr<ConnectionRecord> ConnectionRecord::CreateConnectionRecord(const sptr<IRemoteObject> &callerToken,
    const std::shared_ptr<BaseExtensionRecord> &targetService, const sptr<IAbilityConnection> &connCallback,
    std::shared_ptr<AbilityConnectManager> abilityConnectManager)
{
    auto connRecord = std::make_shared<ConnectionRecord>(
        callerToken, targetService, connCallback, abilityConnectManager);
    CHECK_POINTER_AND_RETURN(connRecord, nullptr);
    connRecord->SetConnectState(ConnectionState::INIT);
    return connRecord;
}

void ConnectionRecord::SetConnectState(const ConnectionState &state)
{
    state_ = state;
}

ConnectionState ConnectionRecord::GetConnectState() const
{
    return state_;
}

sptr<IRemoteObject> ConnectionRecord::GetToken() const
{
    return callerToken_;
}

std::shared_ptr<BaseExtensionRecord> ConnectionRecord::GetAbilityRecord() const
{
    return targetService_;
}

sptr<IAbilityConnection> ConnectionRecord::GetAbilityConnectCallback() const
{
    std::lock_guard lock(callbackMutex_);
    return connCallback_;
}

void ConnectionRecord::ClearConnCallBack()
{
    std::lock_guard lock(callbackMutex_);
    if (connCallback_) {
        connCallback_.clear();
    }
}

int ConnectionRecord::DisconnectAbility()
{
    if (state_ != ConnectionState::CONNECTED) {
        TAG_LOGE(AAFwkTag::CONNECTION, "connection not established, state: %{public}d",
            static_cast<int32_t>(state_));
        return INVALID_CONNECTION_STATE;
    }

    SetConnectState(ConnectionState::DISCONNECTING);
    CHECK_POINTER_AND_RETURN(targetService_, ERR_INVALID_VALUE);

    const auto &abilityInfo = targetService_->GetAbilityInfo();
    auto extAbilityType = abilityInfo.extensionAbilityType;
    bool isPerConnectionType = (extAbilityType == AppExecFwk::ExtensionAbilityType::UI_SERVICE ||
        extAbilityType == AppExecFwk::ExtensionAbilityType::AGENT);
    size_t connectNums = targetService_->GetConnectRecordList().size();
    if (connectNums == 1 || isPerConnectionType) {
        ScheduleDisconnectTimeout();
        if (isPerConnectionType) {
            const char *extName = (extAbilityType == AppExecFwk::ExtensionAbilityType::UI_SERVICE) ?
                "UIServiceExtension" : "AgentExtension";
            TAG_LOGI(AAFwkTag::CONNECTION, "Disconnect %{public}s ability, set correct want", extName);
            targetService_->DisconnectAbilityWithWant(GetConnectWant());
        } else {
            TAG_LOGI(AAFwkTag::CONNECTION, "DisconnectAbility %{public}s called", abilityInfo.name.c_str());
            targetService_->DisconnectAbility();
        }
    } else {
        TAG_LOGI(AAFwkTag::CONNECTION,
            "current connection count: %{public}zu, %{public}s:%{public}s no need disconnect, just remove",
            connectNums, abilityInfo.bundleName.c_str(), abilityInfo.name.c_str());
        targetService_->RemoveConnectRecordFromList(shared_from_this());
        SetConnectState(ConnectionState::DISCONNECTED);
    }

    return ERR_OK;
}

void ConnectionRecord::CompleteConnect()
{
    SetConnectState(ConnectionState::CONNECTED);
    CHECK_POINTER(targetService_);
    targetService_->SetAbilityState(AbilityState::ACTIVE);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Connect,%{public}s", targetService_->GetAbilityInfo().name.c_str());
    const AppExecFwk::AbilityInfo &abilityInfo = targetService_->GetAbilityInfo();
    AppExecFwk::ElementName element(targetService_->GetWant().GetDeviceId(), abilityInfo.bundleName,
        abilityInfo.name, abilityInfo.moduleName);
    auto remoteObject = targetService_->GetConnRemoteObject();
    auto callback = GetAbilityConnectCallback();
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    if (remoteObject == nullptr) {
        TAG_LOGW(AAFwkTag::CONNECTION, "null remoteObject: %{public}s/%{public}s",
            element.GetBundleName().c_str(), element.GetAbilityName().c_str());
        if (handler) {
            SetConnectState(ConnectionState::DISCONNECTING);
            handler->SubmitTask([service = targetService_]() {
                DelayedSingleton<AbilityManagerService>::GetInstance()->ScheduleDisconnectAbilityDone(
                    service->GetToken());
                });
        }
        return;
    }

    if (callback && handler) {
        handler->SubmitTask([callback, element, remoteObject, weakConnectionRecord = weak_from_this(),
            weakConnectManager = abilityConnectManager_] {
            TAG_LOGD(AAFwkTag::CONNECTION, "OnAbilityConnectDone");
            auto ret = ConnectionRecord::CallOnAbilityConnectDone(callback, element, remoteObject, ERR_OK);
            if (ret != ERR_OK) {
                TAG_LOGE(AAFwkTag::CONNECTION, "OnAbilityConnectDone failed");
                auto connectionRecord = weakConnectionRecord.lock();
                CHECK_POINTER(connectionRecord);
                auto abilityConnectManager = weakConnectManager.lock();
                CHECK_POINTER(abilityConnectManager);
                abilityConnectManager->HandleExtensionDisconnectTask(connectionRecord);
            }
        });
    }
    DelayedSingleton<ConnectionStateManager>::GetInstance()->AddConnection(shared_from_this());
    TAG_LOGI(AAFwkTag::CONNECTION, "connectState:%{public}d", state_);
}

void ConnectionRecord::CompleteConnectAndOnlyCallConnectDone()
{
    if (GetConnectState() != ConnectionState::CONNECTED) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Connect state is %{public}d, not connected state", GetConnectState());
        return;
    }
    CHECK_POINTER(targetService_);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CompleteConnect,%{public}s", targetService_->GetAbilityInfo().name.c_str());
    const AppExecFwk::AbilityInfo &abilityInfo = targetService_->GetAbilityInfo();
    AppExecFwk::ElementName element(targetService_->GetWant().GetDeviceId(), abilityInfo.bundleName,
        abilityInfo.name, abilityInfo.moduleName);
    auto remoteObject = targetService_->GetConnRemoteObject();
    auto callback = GetAbilityConnectCallback();
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    if (remoteObject == nullptr) {
        TAG_LOGW(AAFwkTag::CONNECTION, "Complete null remoteObject: %{public}s/%{public}s",
            element.GetBundleName().c_str(), element.GetAbilityName().c_str());
        return;
    }

    if (callback && handler) {
        handler->SubmitTask([callback, element, remoteObject] {
            TAG_LOGD(AAFwkTag::CONNECTION, "CallOnAbilityConnectDone");
            auto ret = ConnectionRecord::CallOnAbilityConnectDone(callback, element, remoteObject, ERR_OK);
            if (ret != ERR_OK) {
                TAG_LOGE(AAFwkTag::CONNECTION, "CallOnAbilityConnectDone failed");
            }
        });
    }
    TAG_LOGI(AAFwkTag::CONNECTION, "Complete connectState:%{public}d", state_);
}

int32_t ConnectionRecord::CallOnAbilityConnectDone(sptr<IAbilityConnection> callback,
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    CHECK_POINTER_AND_RETURN(callback, ERR_INVALID_VALUE);
    sptr<IRemoteObject> obj = callback->AsObject();
    CHECK_POINTER_AND_RETURN(obj, ERR_INVALID_VALUE);
    if (!obj->IsProxyObject()) {
        callback->OnAbilityConnectDone(element, remoteObject, ERR_OK);
        return ERR_OK;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!data.WriteInterfaceToken(IAbilityConnection::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "WriteInterfaceToken fail");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteParcelable(&element)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "element error");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteRemoteObject(remoteObject)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "remoteObject error");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteInt32(resultCode)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "resultCode error");
        return ERR_FLATTEN_OBJECT;
    }

    int32_t ret = obj->SendRequest(IAbilityConnection::ON_ABILITY_CONNECT_DONE, data, reply, option);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail. ret: %{public}d", ret);
    }
    return ret;
}

void ConnectionRecord::CompleteDisconnect(int resultCode, bool isCallerDied, bool isTargetDied)
{
    if (resultCode == ERR_OK) {
        SetConnectState(ConnectionState::DISCONNECTED);
    }
    CHECK_POINTER(targetService_);
    const AppExecFwk::AbilityInfo &abilityInfo = targetService_->GetAbilityInfo();
    AppExecFwk::ElementName element(targetService_->GetWant().GetDeviceId(), abilityInfo.bundleName,
        abilityInfo.name, abilityInfo.moduleName);
    auto code = isTargetDied ? (resultCode - 1) : resultCode;
    auto onDisconnectDoneTask = [connCallback = GetAbilityConnectCallback(), element, code]() {
        TAG_LOGD(AAFwkTag::CONNECTION, "OnAbilityDisconnectDone");
        if (!connCallback) {
            TAG_LOGD(AAFwkTag::CONNECTION, "null connCallback");
            return;
        }
        connCallback->OnAbilityDisconnectDone(element, code);
    };
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    if (handler == nullptr) {
        TAG_LOGE(AAFwkTag::CONNECTION, "null handler");
        return;
    }
    handler->SubmitTask(onDisconnectDoneTask);
    DelayedSingleton<ConnectionStateManager>::GetInstance()->RemoveConnection(shared_from_this(), isCallerDied);
    TAG_LOGD(AAFwkTag::CONNECTION, "result: %{public}d, connectState:%{public}d", resultCode, state_);
}

void ConnectionRecord::ScheduleDisconnectAbilityDone()
{
    if (state_ != ConnectionState::DISCONNECTING) {
        TAG_LOGE(AAFwkTag::CONNECTION, "failed, current state not disconnecting");
        return;
    }

    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    if (handler == nullptr) {
        TAG_LOGE(AAFwkTag::CONNECTION, "fail to get AbilityTaskHandler");
    } else {
        std::string taskName = std::string("DisconnectTimeout_") + std::to_string(recordId_);
        handler->CancelTask(taskName);
    }

    CompleteDisconnect(ERR_OK, GetAbilityConnectCallback() == nullptr);
}

void ConnectionRecord::ScheduleConnectAbilityDone()
{
    if (state_ != ConnectionState::CONNECTING) {
        TAG_LOGE(AAFwkTag::CONNECTION, "failed, current state not connecting");
        return;
    }

    sptr<IRemoteObject> hostproxy = nullptr;
    if (connectWant_.HasParameter(UISERVICEHOSTPROXY_KEY)) {
        hostproxy = connectWant_.GetRemoteObject(UISERVICEHOSTPROXY_KEY);
    }
    sptr<IRemoteObject> connectorProxy = nullptr;
    if (connectWant_.HasParameter(AgentRuntime::AGENTEXTENSIONHOSTPROXY_KEY)) {
        connectorProxy = connectWant_.GetRemoteObject(AgentRuntime::AGENTEXTENSIONHOSTPROXY_KEY);
    }
    auto element = connectWant_.GetElement();
    Want::ClearWant(&connectWant_);
    connectWant_.SetElement(element);
    if (hostproxy != nullptr) {
        connectWant_.SetParam(UISERVICEHOSTPROXY_KEY, hostproxy);
    }
    if (connectorProxy != nullptr) {
        connectWant_.SetParam(AgentRuntime::AGENTEXTENSIONHOSTPROXY_KEY, connectorProxy);
    }

    CancelConnectTimeoutTask();

    CompleteConnect();
}

void ConnectionRecord::CancelConnectTimeoutTask()
{
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    CHECK_POINTER(handler);
    std::string taskName = std::to_string(recordId_);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CancelConnectTimeoutTask:%{public}s", taskName.c_str());
    handler->RemoveEvent(AbilityManagerService::CONNECT_TIMEOUT_MSG, taskName);
    handler->RemoveEvent(AbilityManagerService::CONNECT_HALF_TIMEOUT_MSG, taskName);
    auto targetToken = GetTargetToken();
    CHECK_POINTER(targetToken);
    AbilityRuntime::FreezeUtil::GetInstance().DeleteLifecycleEvent(targetToken);
}

void ConnectionRecord::DisconnectTimeout()
{
    CHECK_POINTER(targetService_);
    /* force to disconnect */
    /* so scheduler target service disconnect done */
    DelayedSingleton<AbilityManagerService>::GetInstance()->ScheduleDisconnectAbilityDone(targetService_->GetToken());
}

void ConnectionRecord::ScheduleDisconnectTimeout()
{
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    if (handler == nullptr) {
        TAG_LOGE(AAFwkTag::CONNECTION, "null handler");
        return;
    }

    auto disconnectTask = [weakPtr = weak_from_this()]() {
        auto connectionRecord = weakPtr.lock();
        if (connectionRecord == nullptr) {
            TAG_LOGE(AAFwkTag::CONNECTION, "null connectionRecord");
            return;
        }
        TAG_LOGE(AAFwkTag::CONNECTION, "Disconnect timeout");
        connectionRecord->DisconnectTimeout();
    };

    std::string taskName = "DisconnectTimeout_" + std::to_string(recordId_);
    int disconnectTimeout =
        AmsConfigurationParameter::GetInstance().GetAppStartTimeoutTime() * DISCONNECT_TIMEOUT_MULTIPLE;
    handler->SubmitTask(disconnectTask, taskName, disconnectTimeout);
}

std::string ConnectionRecord::ConvertConnectionState(const ConnectionState &state) const
{
    switch (state) {
        case ConnectionState::INIT:
            return "INIT";
        case ConnectionState::CONNECTING:
            return "CONNECTING";
        case ConnectionState::CONNECTED:
            return "CONNECTED";
        case ConnectionState::DISCONNECTING:
            return "DISCONNECTING";
        case ConnectionState::DISCONNECTED:
            return "DISCONNECTED";
        default:
            return "INVALIDSTATE";
    }
}

void ConnectionRecord::Dump(std::vector<std::string> &info) const
{
    auto abilityRecord = GetAbilityRecord();
    CHECK_POINTER(abilityRecord);
    info.emplace_back("       > " + abilityRecord->GetAbilityInfo().bundleName + "/" +
                      abilityRecord->GetAbilityInfo().name + "   connectionState #" +
                      ConvertConnectionState(GetConnectState()));
}

void ConnectionRecord::AttachCallerInfo(std::shared_ptr<IndirectCallerInfo> indirectCallerInfo)
{
    SetDirectCallerInfo();

    if (indirectCallerInfo != nullptr) {
        callerTokenId_ = indirectCallerInfo->tokenId;
        callerUid_ = indirectCallerInfo->callerUid;
        callerPid_ = indirectCallerInfo->callerPid;
        callerName_ = ConnectionStateManager::GetProcessNameByPid(callerPid_);
        TAG_LOGD(AAFwkTag::CONNECTION, "callerTokenId_:%{public}u, callerUid_:%{public}d, callerPid_:%{public}d, "
            "callerName_:%{public}s", callerTokenId_, callerUid_, callerPid_, callerName_.c_str());
        return;
    }
    
    callerTokenId_ = GetDirectCallerTokenId();
    callerUid_ = GetDirectCallerUid();
    callerPid_ = GetDirectCallerPid();
    callerName_ = GetDirectCallerName();
    TAG_LOGD(AAFwkTag::CONNECTION, "callerTokenId_:%{public}u, callerUid_:%{public}d, callerPid_:%{public}d, "
        "callerName_:%{public}s", callerTokenId_, callerUid_, callerPid_, callerName_.c_str());
}

int32_t ConnectionRecord::GetCallerUid() const
{
    return callerUid_;
}

int32_t ConnectionRecord::GetCallerPid() const
{
    return callerPid_;
}

uint32_t ConnectionRecord::GetCallerTokenId() const
{
    return callerTokenId_;
}

std::string ConnectionRecord::GetCallerName() const
{
    return callerName_;
}

int32_t ConnectionRecord::GetDirectCallerUid() const
{
    return directCallerUid_;
}

int32_t ConnectionRecord::GetDirectCallerPid() const
{
    return directCallerPid_;
}

uint32_t ConnectionRecord::GetDirectCallerTokenId() const
{
    return directCallerTokenId_;
}

std::string ConnectionRecord::GetDirectCallerName() const
{
    return directCallerName_;
}

void ConnectionRecord::SetDirectCallerInfo()
{
    directCallerTokenId_ = IPCSkeleton::GetCallingTokenID(); // tokenId identifies the real caller
    auto targetRecord = Token::GetAbilityRecordByToken(callerToken_);
    if (targetRecord) {
        directCallerUid_ = targetRecord->GetUid();
        directCallerPid_ = targetRecord->GetPid();
        directCallerName_ = targetRecord->GetAbilityInfo().bundleName;
        return;
    }

    directCallerUid_ = static_cast<int32_t>(IPCSkeleton::GetCallingUid());
    directCallerPid_ = static_cast<int32_t>(IPCSkeleton::GetCallingPid());
    directCallerName_ = ConnectionStateManager::GetProcessNameByPid(directCallerPid_);

    TAG_LOGD(AAFwkTag::CONNECTION, "directCallerTokenId_:%{public}u, directCallerUid_:%{public}d, "
        "directCallerPid_:%{public}d, directCallerName_:%{public}s", directCallerTokenId_, directCallerUid_,
        directCallerPid_, directCallerName_.c_str());
}

sptr<IRemoteObject> ConnectionRecord::GetTargetToken() const
{
    auto targetService = targetService_;
    if (!targetService) {
        return nullptr;
    }

    auto token = targetService->GetToken();
    if (!token) {
        return nullptr;
    }

    return token->AsObject();
}

sptr<IRemoteObject> ConnectionRecord::GetConnection() const
{
    auto callback = GetAbilityConnectCallback();
    if (!callback) {
        return nullptr;
    }

    return callback->AsObject();
}

void ConnectionRecord::SetConnectWant(const Want &want)
{
    connectWant_ = want;
}

Want ConnectionRecord::GetConnectWant() const
{
    return connectWant_;
}

int32_t ConnectionRecord::SuspendExtensionAbility()
{
    if (state_ != ConnectionState::CONNECTED) {
        TAG_LOGE(AAFwkTag::CONNECTION, "connection not established, state: %{public}d",
            static_cast<int32_t>(state_));
        return INVALID_CONNECTION_STATE;
    }

    DelayedSingleton<ConnectionStateManager>::GetInstance()->SuspendConnection(shared_from_this());
    return ERR_OK;
}

int32_t ConnectionRecord::ResumeExtensionAbility()
{
    if (state_ != ConnectionState::CONNECTED) {
        TAG_LOGE(AAFwkTag::CONNECTION, "connection not established, state: %{public}d",
            static_cast<int32_t>(state_));
        return INVALID_CONNECTION_STATE;
    }

    DelayedSingleton<ConnectionStateManager>::GetInstance()->ResumeConnection(shared_from_this());
    return ERR_OK;
}
}  // namespace AAFwk
}  // namespace OHOS
