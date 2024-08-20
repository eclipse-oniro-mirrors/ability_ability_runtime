/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "ability_manager_errors.h"
#include "ability_manager_service.h"
#include "ability_util.h"
#include "connection_state_manager.h"
#include "hilog_tag_wrapper.h"
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
    const std::shared_ptr<AbilityRecord> &targetService, const sptr<IAbilityConnection> &connCallback)
    : state_(ConnectionState::INIT),
      callerToken_(callerToken),
      targetService_(targetService),
      connCallback_(connCallback)
{
    recordId_ = connectRecordId++;
}

ConnectionRecord::~ConnectionRecord()
{}

std::shared_ptr<ConnectionRecord> ConnectionRecord::CreateConnectionRecord(const sptr<IRemoteObject> &callerToken,
    const std::shared_ptr<AbilityRecord> &targetService, const sptr<IAbilityConnection> &connCallback)
{
    auto connRecord = std::make_shared<ConnectionRecord>(callerToken, targetService, connCallback);
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

std::shared_ptr<AbilityRecord> ConnectionRecord::GetAbilityRecord() const
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

    /* set state to Disconnecting */
    SetConnectState(ConnectionState::DISCONNECTING);
    CHECK_POINTER_AND_RETURN(targetService_, ERR_INVALID_VALUE);
    std::size_t connectNums = targetService_->GetConnectRecordList().size();
    AppExecFwk::ExtensionAbilityType extAbilityType = targetService_->GetAbilityInfo().extensionAbilityType;
    bool isAbilityUIServiceExt = (extAbilityType == AppExecFwk::ExtensionAbilityType::UI_SERVICE);
    if (connectNums == 1 || isAbilityUIServiceExt) {
        /* post timeout task to taskhandler */
        auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
        if (handler == nullptr) {
            TAG_LOGE(AAFwkTag::CONNECTION, "fail to get TaskHandler");
        } else {
            std::string taskName("DisconnectTimeout_");
            taskName += std::to_string(recordId_);
            auto disconnectTask = [connectionRecord = shared_from_this()]() {
                TAG_LOGE(AAFwkTag::CONNECTION, "Disconnect timeout");
                connectionRecord->DisconnectTimeout();
            };
            int disconnectTimeout =
                AmsConfigurationParameter::GetInstance().GetAppStartTimeoutTime() * DISCONNECT_TIMEOUT_MULTIPLE;
            handler->SubmitTask(disconnectTask, taskName, disconnectTimeout);
        }
        /* schedule disconnect to target ability */
        if (isAbilityUIServiceExt) {
            TAG_LOGI(AAFwkTag::CONNECTION, "Disconnect UIServiceExtension ability, set correct want");
            targetService_->DisconnectUIServiceExtAbility(GetConnectWant());
        } else {
            targetService_->DisconnectAbility();
        }
    } else {
        TAG_LOGD(AAFwkTag::CONNECTION,
            "current connection count: %{public}zu, no need disconnect, just remove", connectNums);
        targetService_->RemoveConnectRecordFromList(shared_from_this());
        SetConnectState(ConnectionState::DISCONNECTED);
    }

    return ERR_OK;
}

void ConnectionRecord::CompleteConnect(int resultCode)
{
    CHECK_POINTER(targetService_);
    if (resultCode == ERR_OK) {
        SetConnectState(ConnectionState::CONNECTED);
        targetService_->SetAbilityState(AbilityState::ACTIVE);
    }
    const AppExecFwk::AbilityInfo &abilityInfo = targetService_->GetAbilityInfo();
    AppExecFwk::ElementName element(abilityInfo.deviceId, abilityInfo.bundleName,
        abilityInfo.name, abilityInfo.moduleName);
    auto remoteObject = targetService_->GetConnRemoteObject();
    auto callback = GetAbilityConnectCallback();
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    if (remoteObject == nullptr) {
        TAG_LOGW(AAFwkTag::CONNECTION, "extension returned null: %{public}s", element.GetURI().c_str());
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
        handler->SubmitTask([callback, element, remoteObject, resultCode] {
            TAG_LOGD(AAFwkTag::CONNECTION, "OnAbilityConnectDone");
            callback->OnAbilityConnectDone(element, remoteObject, resultCode);
            });
    }
    DelayedSingleton<ConnectionStateManager>::GetInstance()->AddConnection(shared_from_this());
    TAG_LOGI(AAFwkTag::CONNECTION, "result: %{public}d, connectState:%{public}d", resultCode, state_);
}

void ConnectionRecord::CompleteDisconnect(int resultCode, bool isCallerDied, bool isTargetDied)
{
    if (resultCode == ERR_OK) {
        SetConnectState(ConnectionState::DISCONNECTED);
    }
    CHECK_POINTER(targetService_);
    const AppExecFwk::AbilityInfo &abilityInfo = targetService_->GetAbilityInfo();
    AppExecFwk::ElementName element(abilityInfo.deviceId, abilityInfo.bundleName,
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
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    if (handler == nullptr) {
        TAG_LOGE(AAFwkTag::CONNECTION, "fail to get AbilityTaskHandler");
    } else {
        std::string taskName = std::string("ConnectTimeout_") + std::to_string(recordId_);
        handler->CancelTask(taskName);
    }

    CompleteConnect(ERR_OK);
}

void ConnectionRecord::DisconnectTimeout()
{
    CHECK_POINTER(targetService_);
    /* force to disconnect */
    /* so scheduler target service disconnect done */
    DelayedSingleton<AbilityManagerService>::GetInstance()->ScheduleDisconnectAbilityDone(targetService_->GetToken());
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
    info.emplace_back("       > " + GetAbilityRecord()->GetAbilityInfo().bundleName + "/" +
                      GetAbilityRecord()->GetAbilityInfo().name + "   connectionState #" +
                      ConvertConnectionState(GetConnectState()));
}

void ConnectionRecord::AttachCallerInfo()
{
    callerTokenId_ = IPCSkeleton::GetCallingTokenID(); // tokenId identifies the real caller
    auto targetRecord = Token::GetAbilityRecordByToken(callerToken_);
    if (targetRecord) {
        callerUid_ = targetRecord->GetUid();
        callerPid_ = targetRecord->GetPid();
        callerName_ = targetRecord->GetAbilityInfo().bundleName;
        return;
    }

    callerUid_ = static_cast<int32_t>(IPCSkeleton::GetCallingUid());
    callerPid_ = static_cast<int32_t>(IPCSkeleton::GetCallingPid());
    callerName_ = ConnectionStateManager::GetProcessNameByPid(callerPid_);
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

Want ConnectionRecord::GetConnectWant()
{
    return connectWant_;
}
}  // namespace AAFwk
}  // namespace OHOS
