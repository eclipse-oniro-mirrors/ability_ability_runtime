/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_CONNECTION_RECORD_H
#define OHOS_ABILITY_RUNTIME_CONNECTION_RECORD_H

#include <mutex>
#include "ability_connect_callback_interface.h"
#include "base_extension_record.h"
#include "caller_info.h"
#include "extension_config.h"
#include "nocopyable.h"

namespace OHOS {
namespace AAFwk {
class AbilityConnectManager;
/**
 * @enum ConnectionState
 * ConnectionState defines the state of connect ability.
 */
enum class ConnectionState { INIT, CONNECTING, CONNECTED, DISCONNECTING, DISCONNECTED };
/**
 * @class ConnectionRecord
 * ConnectionRecord,This class is used to record information about a connection.
 */
class ConnectionRecord : public std::enable_shared_from_this<ConnectionRecord> {
public:
    ConnectionRecord(const sptr<IRemoteObject> &callerToken, const std::shared_ptr<BaseExtensionRecord> &targetService,
        const sptr<IAbilityConnection> &connCallback, std::shared_ptr<AbilityConnectManager> abilityConnectManager);
    virtual ~ConnectionRecord();

    /**
     * create a connection record by caller token , service ability and call back ipc object.
     *
     * @param callerToken, the token of caller ability.
     * @param targetService, target service ability.
     * @param callback, call back (ipc object).
     * @return Return the connect record.
     */
    static std::shared_ptr<ConnectionRecord> CreateConnectionRecord(const sptr<IRemoteObject> &callerToken,
        const std::shared_ptr<BaseExtensionRecord> &targetService, const sptr<IAbilityConnection> &connCallback,
        std::shared_ptr<AbilityConnectManager> abilityConnectManager);

    static int32_t CallOnAbilityConnectDone(sptr<IAbilityConnection> callback,
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode);
    /**
     * set the connect state.
     *
     * @param state, target connection state.
     */
    void SetConnectState(const ConnectionState &state);

    /**
     * get the connect state.
     *
     * @return state, target connection state.
     */
    ConnectionState GetConnectState() const;

    /**
     * get the token of the ability.
     *
     * @return token.
     */
    sptr<IRemoteObject> GetToken() const;

    /**
     * get the ability record from connection record.
     *
     * @return BaseExtensionRecord.
     */
    std::shared_ptr<BaseExtensionRecord> GetAbilityRecord() const;

    sptr<IAbilityConnection> GetAbilityConnectCallback() const;

    /**
     * disconnect the service ability.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    int DisconnectAbility();

    /**
     * Suspend the service ability.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t SuspendExtensionAbility();
    
    /**
     * Resume the service ability.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t ResumeExtensionAbility();

    /**
     * force to disconnect time out event.
     *
     */
    void DisconnectTimeout();

    /**
     * complete connect ability and invoke callback.
     *
     */
    void CompleteConnect();

    /**
     * complete connect ability and just try to invoke callback.
     *
     */
    void CompleteConnectAndOnlyCallConnectDone();

    /**
     * complete disconnect ability and invoke callback.
     *
     */
    void CompleteDisconnect(int resultCode, bool isCallerDied, bool isTargetDied = false);

    /**
     * scheduler target service disconnect done.
     *
     */
    void ScheduleDisconnectAbilityDone();

    /**
     * scheduler target service Connect done.
     *
     */
    void ScheduleConnectAbilityDone();

    /**
     * cancel connect timeout task.
     *
     */
    void CancelConnectTimeoutTask();

    /**
     * get connection record id.
     *
     */
    inline int GetRecordId() const
    {
        return recordId_;
    }

    void ClearConnCallBack();

    std::string ConvertConnectionState(const ConnectionState &state) const;

    void Dump(std::vector<std::string> &info) const;

    void AttachCallerInfo(std::shared_ptr<IndirectCallerInfo> indirectCallerInfo = nullptr);
    int32_t GetCallerUid() const;
    int32_t GetCallerPid() const;
    uint32_t GetCallerTokenId() const;
    std::string GetCallerName() const;
    int32_t GetDirectCallerUid() const;
    int32_t GetDirectCallerPid() const;
    uint32_t GetDirectCallerTokenId() const;
    std::string GetDirectCallerName() const;
    sptr<IRemoteObject> GetTargetToken() const;
    sptr<IRemoteObject> GetConnection() const;

    void SetConnectWant(const Want &want);
    Want GetConnectWant() const;
private:
    void ScheduleDisconnectTimeout();
    void SetDirectCallerInfo();

    static int64_t connectRecordId;
    Want connectWant_;
    ConnectionState state_;                            // service connection state
    sptr<IRemoteObject> callerToken_ = nullptr;              // from:caller token
    std::shared_ptr<BaseExtensionRecord> targetService_ = nullptr; // target:service need to be connected
    mutable std::mutex callbackMutex_;
    sptr<IAbilityConnection> connCallback_ = nullptr;        // service connect callback
    std::weak_ptr<AbilityConnectManager> abilityConnectManager_;
    std::string callerName_;                           // caller bundleName or processName
    std::atomic<int32_t> recordId_ = 0;                // record id
    int32_t callerUid_ = 0;                            // caller uid
    int32_t callerPid_ = 0;                            // caller pid
    uint32_t callerTokenId_ = 0;                       // caller token id
    int32_t directCallerUid_ = 0;                      // direct caller uid
    int32_t directCallerPid_ = 0;                      // direct caller pid
    uint32_t directCallerTokenId_ = 0;                 // direct caller token id
    std::string directCallerName_;                     // direct caller bundleName or processName

    DISALLOW_COPY_AND_MOVE(ConnectionRecord);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_CONNECTION_RECORD_H
