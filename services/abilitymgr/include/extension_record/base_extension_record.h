/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_EXTENSION_RECORD_BASE_H
#define OHOS_ABILITY_RUNTIME_EXTENSION_RECORD_BASE_H

#include "ability_record.h"

namespace OHOS {
namespace AAFwk {
class ConnectionRecord;

class BaseExtensionRecord : public AbilityRecord {
public:
    BaseExtensionRecord(const Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
        const AppExecFwk::ApplicationInfo &applicationInfo, int requestCode = -1);

    static std::shared_ptr<BaseExtensionRecord> CreateBaseExtensionRecord(const AbilityRequest &abilityRequest);

    static std::shared_ptr<BaseExtensionRecord> TransferToExtensionRecordBase(
        const std::shared_ptr<AbilityRecord> &abilityRecord);

    AbilityRecordType GetAbilityRecordType() override;

    /**
     * add connect record to the list.
     *
     */
    void AddConnectRecordToList(const std::shared_ptr<ConnectionRecord> &connRecord);

    /**
     * get the list of connect record.
     *
     */
    std::list<std::shared_ptr<ConnectionRecord>> GetConnectRecordList() const;

    /**
     * get the list of connect record.
     *
     */
    std::list<std::shared_ptr<ConnectionRecord>> GetConnectingRecordList();

    /**
     * remove the connect record from list.
     *
     */
    void RemoveConnectRecordFromList(const std::shared_ptr<ConnectionRecord> &connRecord);

    void PostUIExtensionAbilityTimeoutTask(uint32_t messageId);

    /**
     * get connecting record from list.
     *
     */
    std::shared_ptr<ConnectionRecord> GetConnectingRecord() const;

    /**
     * get disconnecting record from list.
     *
     */
    std::shared_ptr<ConnectionRecord> GetDisconnectingRecord() const;

    size_t GetConnectedListSize();

    /**
     * check whether connect list is empty.
     *
     */
    bool IsConnectListEmpty();

    bool NeedConnectAfterCommand();

    size_t GetConnectingListSize();

    /**
     * get the count of In Progress record.
     *
     */
    uint32_t GetInProgressRecordCount();

    /**
     * disconnect the ability.
     *
     */
    void DisconnectAbility();

    /**
     * disconnect the ability with want
     *
     */
    void DisconnectAbilityWithWant(const Want &want);

    /**
     * connect the ability.
     *
     */
    void ConnectAbility();

    /**
     * connect the ability with want.
     *
     */
    void ConnectAbilityWithWant(const Want &want);
    /**
     * dump service info.
     *
     */
    void DumpService(std::vector<std::string> &info, bool isClient = false) const;

    /**
     * dump service info.
     *
     */
    void DumpService(std::vector<std::string> &info, std::vector<std::string> &params,
        bool isClient = false) const;

    /**
     * set connect remote object.
     *
     */
    void SetConnRemoteObject(const sptr<IRemoteObject> &remoteObject);

    /**
     * get connect remote object.
     *
     */
    sptr<IRemoteObject> GetConnRemoteObject() const;

    void SetClientPid(pid_t clientPid);

    pid_t GetClientPid() const;

private:
    void DumpUIExtensionRootHostInfo(std::vector<std::string> &info) const;

    void DumpUIExtensionPid(std::vector<std::string> &info, bool isUIExtension) const;

    pid_t clientPid_ = -1;
    // service(ability) can be connected by multi-pages(abilities), so need to store this service's connections
    mutable ffrt::mutex connRecordListMutex_;
    std::list<std::shared_ptr<ConnectionRecord>> connRecordList_ = {};
    // service(ability) onConnect() return proxy of service ability
    sptr<IRemoteObject> connRemoteObject_ = {};
    bool isConnected = false;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_EXTENSION_RECORD_BASE_H