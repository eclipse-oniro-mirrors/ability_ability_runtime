/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

#include "local_call_container.h"

#include "hilog_tag_wrapper.h"
#include "ability_manager_client.h"
#include "os_account_manager_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
constexpr int32_t REASON_CALLEE_TERMINATE = 1;
int LocalCallContainer::StartAbilityByCallInner(const Want& want, std::shared_ptr<CallerCallBack> callback,
    sptr<IRemoteObject> callerToken, int32_t accountId)
{
    AppExecFwk::ElementName element = want.GetElement();
    TAG_LOGD(AAFwkTag::LOCAL_CALL, "element:%{public}s/%{public}s/%{public}s", element.GetBundleName().c_str(),
        element.GetModuleName().c_str(), element.GetAbilityName().c_str());
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::LOCAL_CALL, "null callback");
        return ERR_INVALID_VALUE;
    }
    if (element.GetBundleName().empty() || element.GetAbilityName().empty()) {
        TAG_LOGE(AAFwkTag::LOCAL_CALL, "empty want");
        return ERR_INVALID_VALUE;
    }
    if (element.GetDeviceID().empty()) {
        TAG_LOGD(AAFwkTag::LOCAL_CALL, "element:DeviceID is empty");
    }

    int32_t oriValidUserId = GetValidUserId(accountId);
    std::shared_ptr<LocalCallRecord> localCallRecord;
    if (!GetCallLocalRecord(element, localCallRecord, oriValidUserId)) {
        localCallRecord = std::make_shared<LocalCallRecord>(element);
        localCallRecord->SetUserId(oriValidUserId);
        TAG_LOGD(
            AAFwkTag::LOCAL_CALL, "set user id[%{public}d] to record", oriValidUserId);
    }
    localCallRecord->AddCaller(callback);
    auto remote = localCallRecord->GetRemoteObject();
    // already finish call request.
    if (remote) {
        callback->InvokeCallBack(remote);
        if (!want.GetBoolParam(Want::PARAM_RESV_CALL_TO_FOREGROUND, false)) {
            return ERR_OK;
        }
    }
    sptr<CallerConnection> connect = sptr<CallerConnection>::MakeSptr();
    if (connect == nullptr) {
        TAG_LOGE(AAFwkTag::LOCAL_CALL, "null connect");
        return ERR_INVALID_VALUE;
    }
    {
        std::lock_guard lock(connectionsMutex_);
        connections_.emplace(connect);
        TAG_LOGD(AAFwkTag::LOCAL_CALL, "connections_.size is %{public}zu", connections_.size());
    }
    
    connect->SetRecordAndContainer(localCallRecord, shared_from_this());
    auto retval = AAFwk::AbilityManagerClient::GetInstance()->StartAbilityByCall(want, connect,
        callerToken, oriValidUserId);
    if (retval != ERR_OK) {
        ClearFailedCallConnection(callback);
    }
    return retval;
}

int LocalCallContainer::ReleaseCall(const std::shared_ptr<CallerCallBack>& callback)
{
    TAG_LOGD(AAFwkTag::LOCAL_CALL, "begin");
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::LOCAL_CALL, "null callback");
        return ERR_INVALID_VALUE;
    }
    auto abilityClient = AAFwk::AbilityManagerClient::GetInstance();
    if (abilityClient == nullptr) {
        TAG_LOGE(AAFwkTag::LOCAL_CALL, "null abilityClient");
        return ERR_INVALID_VALUE;
    }
    auto localCallRecord = callback->GetRecord();
    if (localCallRecord == nullptr) {
        TAG_LOGE(AAFwkTag::LOCAL_CALL, "null localCallRecord");
        return ERR_INVALID_VALUE;
    }
    localCallRecord->RemoveCaller(callback);
    if (localCallRecord->IsExistCallBack()) {
        // just release callback.
        TAG_LOGD(AAFwkTag::LOCAL_CALL,
            "ust release this callback");
        return ERR_OK;
    }
    auto connect = iface_cast<CallerConnection>(localCallRecord->GetConnection());
    if (connect == nullptr) {
        TAG_LOGE(AAFwkTag::LOCAL_CALL, "null connect");
        return ERR_INVALID_VALUE;
    }
    int32_t retval = ERR_OK;
    if (localCallRecord->IsSingletonRemote()) {
        retval = RemoveSingletonCallLocalRecord(localCallRecord);
    } else {
        retval = RemoveMultipleCallLocalRecord(localCallRecord);
    }

    if (retval != ERR_OK) {
        TAG_LOGE(AAFwkTag::LOCAL_CALL, "Remove call local record failed");
        return retval;
    }
    {
        std::lock_guard lock(connectionsMutex_);
        connections_.erase(connect);
    }
    connect->ClearCallRecord();
    localCallRecord->ClearData();
    if (abilityClient->ReleaseCall(connect, localCallRecord->GetElementName()) != ERR_OK) {
        TAG_LOGE(AAFwkTag::LOCAL_CALL, "ReleaseCall failed");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

void LocalCallContainer::ClearFailedCallConnection(const std::shared_ptr<CallerCallBack> &callback)
{
    TAG_LOGI(AAFwkTag::LOCAL_CALL, "called");
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::LOCAL_CALL, "null callback");
        return;
    }

    auto localCallRecord = callback->GetRecord();
    if (localCallRecord == nullptr) {
        TAG_LOGE(AAFwkTag::LOCAL_CALL, "null localCallRecord");
        return;
    }

    auto connect = iface_cast<CallerConnection>(localCallRecord->GetConnection());
    if (connect == nullptr) {
        TAG_LOGE(AAFwkTag::LOCAL_CALL, "null connect");
        return;
    }
    std::string deviceId = localCallRecord->GetElementName().GetDeviceID();
    if (!deviceId.empty()) {
        TAG_LOGI(AAFwkTag::LOCAL_CALL, "try releaseCall");
        AAFwk::AbilityManagerClient::GetInstance()->ReleaseCall(connect, localCallRecord->GetElementName());
    }
    std::lock_guard lock(connectionsMutex_);
    connections_.erase(connect);
}

int32_t LocalCallContainer::RemoveSingletonCallLocalRecord(const std::shared_ptr<LocalCallRecord> &record)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::LOCAL_CALL, "null record");
        return ERR_INVALID_VALUE;
    }

    auto iterRecord = callProxyRecords_.find(record->GetElementName().GetURI());
    if (iterRecord == callProxyRecords_.end()) {
        TAG_LOGE(AAFwkTag::LOCAL_CALL, "not found record");
        return ERR_INVALID_VALUE;
    }

    iterRecord->second.erase(record);
    if (iterRecord->second.empty()) {
        callProxyRecords_.erase(iterRecord);
    }

    return ERR_OK;
}

int32_t LocalCallContainer::RemoveMultipleCallLocalRecord(const std::shared_ptr<LocalCallRecord> &record)
{
    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::LOCAL_CALL, "null record");
        return ERR_INVALID_VALUE;
    }

    std::lock_guard<std::mutex> lock(multipleMutex_);
    auto iterRecord = multipleCallProxyRecords_.find(record->GetElementName().GetURI());
    if (iterRecord == multipleCallProxyRecords_.end()) {
        TAG_LOGE(AAFwkTag::LOCAL_CALL, "not found record");
        return ERR_INVALID_VALUE;
    }

    iterRecord->second.erase(record);
    if (iterRecord->second.empty()) {
        multipleCallProxyRecords_.erase(iterRecord);
    }

    return ERR_OK;
}

bool LocalCallContainer::IsCallBackCalled(const std::vector<std::shared_ptr<CallerCallBack>> &callers) const
{
    for (auto& callBack : callers) {
        if (callBack != nullptr && !callBack->IsCallBack()) {
            TAG_LOGE(AAFwkTag::LOCAL_CALL, "not called callback");
            return false;
        }
    }

    return true;
}

void LocalCallContainer::DumpCalls(std::vector<std::string>& info)
{
    TAG_LOGD(AAFwkTag::LOCAL_CALL, "called");
    info.emplace_back("          caller connections:");
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &item : callProxyRecords_) {
        for (auto &itemCall : item.second) {
            std::string tempstr = "            LocalCallRecord";
            tempstr += " ID #" + std::to_string(itemCall->GetRecordId()) + "\n";
            tempstr += "              callee";
            tempstr += " uri[" + item.first + "]" + "\n";
            tempstr += "              callers #" + std::to_string(itemCall->GetCallers().size());
            if (IsCallBackCalled(itemCall->GetCallers())) {
                TAG_LOGI(AAFwkTag::LOCAL_CALL, "state: REQUESTEND");
                tempstr += "  state #REQUESTEND";
            } else {
                TAG_LOGI(AAFwkTag::LOCAL_CALL, "state: REQUESTING");
                tempstr += "  state #REQUESTING";
            }
            info.emplace_back(tempstr);
        }
    }
    return;
}

bool LocalCallContainer::GetCallLocalRecord(
    const AppExecFwk::ElementName& elementName, std::shared_ptr<LocalCallRecord>& localCallRecord, int32_t accountId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto pair : callProxyRecords_) {
        AppExecFwk::ElementName callElement;
        if (!callElement.ParseURI(pair.first)) {
            TAG_LOGE(AAFwkTag::LOCAL_CALL,
                "failed elementName uri: %{private}s", pair.first.c_str());
            continue;
        }
        // elementName in callProxyRecords_ has moduleName (sometimes not empty),
        // but the moduleName of input param elementName is usually empty.
        callElement.SetModuleName("");
        if ((pair.first != elementName.GetURI() && callElement.GetURI() != elementName.GetURI())) {
            continue;
        }

        for (auto &itemCall : pair.second) {
            if (itemCall != nullptr && itemCall->GetUserId() == accountId) {
                localCallRecord = itemCall;
                return true;
            }
        }
    }
    return false;
}

void LocalCallContainer::OnCallStubDied(const wptr<IRemoteObject>& remote)
{
    auto diedRemote = remote.promote();
    auto isExist = [&diedRemote](auto& record) {
        return record->IsSameObject(diedRemote);
    };

    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto &item : callProxyRecords_) {
            auto iter = std::find_if(item.second.begin(), item.second.end(), isExist);
            if (iter == item.second.end()) {
                continue;
            }
            TAG_LOGD(AAFwkTag::LOCAL_CALL,
                "singleton key[%{public}s]. notify died event", item.first.c_str());
            (*iter)->OnCallStubDied();
            item.second.erase(iter);
            if (item.second.empty()) {
                TAG_LOGD(AAFwkTag::LOCAL_CALL,
                    "singleton key[%{public}s] empty", item.first.c_str());
                callProxyRecords_.erase(item.first);
                break;
            }
        }
    }

    std::lock_guard<std::mutex> lock(multipleMutex_);
    for (auto &item : multipleCallProxyRecords_) {
        TAG_LOGD(
            AAFwkTag::LOCAL_CALL, "multiple key[%{public}s].", item.first.c_str());
        auto iterMultiple = find_if(item.second.begin(), item.second.end(), isExist);
        if (iterMultiple == item.second.end()) {
            continue;
        }
        TAG_LOGD(AAFwkTag::LOCAL_CALL, "multiple key[%{public}s]. notify died event",
            item.first.c_str());
        (*iterMultiple)->OnCallStubDied();
        item.second.erase(iterMultiple);
        if (item.second.empty()) {
            TAG_LOGD(AAFwkTag::LOCAL_CALL,
                "multiple key[%{public}s] empty.", item.first.c_str());
            multipleCallProxyRecords_.erase(item.first);
            break;
        }
    }
}

void LocalCallContainer::RemoveCallerConnection(sptr<CallerConnection> connection)
{
    if (connection == nullptr) {
        TAG_LOGE(AAFwkTag::LOCAL_CALL, "null connection");
        return;
    }

    std::lock_guard lock(connectionsMutex_);
    connections_.erase(connection);
}

void LocalCallContainer::SetCallLocalRecord(std::shared_ptr<LocalCallRecord> localCallRecord)
{
    if (localCallRecord == nullptr) {
        TAG_LOGE(AAFwkTag::LOCAL_CALL, "localCallRecord null");
        return;
    }
    std::string strKey = localCallRecord->GetElementName().GetURI();
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = callProxyRecords_.find(strKey);
    if (iter == callProxyRecords_.end()) {
        std::set<std::shared_ptr<LocalCallRecord>> records = { localCallRecord };
        callProxyRecords_.emplace(strKey, records);
        return;
    }

    iter->second.emplace(localCallRecord);
}

void LocalCallContainer::SetMultipleCallLocalRecord(std::shared_ptr<LocalCallRecord> localCallRecord)
{
    if (localCallRecord == nullptr) {
        TAG_LOGE(AAFwkTag::LOCAL_CALL, "multiple localCallRecord null");
        return;
    }
    std::string strKey = localCallRecord->GetElementName().GetURI();
    std::lock_guard<std::mutex> lock(multipleMutex_);
    auto iter = multipleCallProxyRecords_.find(strKey);
    if (iter == multipleCallProxyRecords_.end()) {
        std::set<std::shared_ptr<LocalCallRecord>> records = { localCallRecord };
        multipleCallProxyRecords_.emplace(strKey, records);
        return;
    }

    iter->second.emplace(localCallRecord);
}

void CallerConnection::ClearCallRecord()
{
    localCallRecord_.reset();
}

void CallerConnection::SetRecordAndContainer(const std::shared_ptr<LocalCallRecord> &localCallRecord,
    const std::weak_ptr<LocalCallContainer> &container)
{
    if (localCallRecord == nullptr) {
        TAG_LOGD(AAFwkTag::LOCAL_CALL, "input param is nullptr");
        return;
    }
    localCallRecord_ = localCallRecord;
    container_ = container;
    localCallRecord_->SetConnection(this->AsObject());
}

void CallerConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int code)
{
    TAG_LOGD(AAFwkTag::LOCAL_CALL, "start: %{public}s/%{public}s/%{public}s", element.GetBundleName().c_str(),
        element.GetModuleName().c_str(), element.GetAbilityName().c_str());
    auto container = container_.lock();
    if (container == nullptr || localCallRecord_ == nullptr) {
        TAG_LOGE(AAFwkTag::LOCAL_CALL, "null container or record");
        return;
    }

    const bool isSingleton = (code == static_cast<int32_t>(AppExecFwk::LaunchMode::SINGLETON));
    localCallRecord_->SetIsSingleton(isSingleton);

    auto callRecipient = new (std::nothrow) CallRecipient([container](const wptr<IRemoteObject> &arg) {
        container->OnCallStubDied(arg);
    });
    localCallRecord_->SetRemoteObject(remoteObject, callRecipient);

    if (isSingleton) {
        container->SetCallLocalRecord(localCallRecord_);
    } else {
        container->SetMultipleCallLocalRecord(localCallRecord_);
    }

    localCallRecord_->InvokeCallBack();
    return;
}

void CallerConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int32_t code)
{
    TAG_LOGI(AAFwkTag::LOCAL_CALL, "OnAbilityDisconnectDone: %{public}s, code: %{public}d",
        element.GetAbilityName().c_str(), code);

    if (localCallRecord_ == nullptr) {
        TAG_LOGE(AAFwkTag::LOCAL_CALL, "local call record is nullptr");
        return;
    }

    // Handle callee ability termination (code == 1)
    // This is called when callee ability exits but process is still alive
    if (code == REASON_CALLEE_TERMINATE) {
        TAG_LOGI(AAFwkTag::LOCAL_CALL, "Callee ability terminated, notify all callers");

        // Notify all callers that callee is terminating with proper release reason
        localCallRecord_->OnCallStubDied();

        // Remove record from container
        auto container = container_.lock();
        if (container == nullptr) {
            TAG_LOGE(AAFwkTag::LOCAL_CALL, "null container");
            return;
        }

        // Remove the local call record from container
        if (localCallRecord_->IsSingletonRemote()) {
            container->RemoveSingletonCallLocalRecord(localCallRecord_);
        } else {
            container->RemoveMultipleCallLocalRecord(localCallRecord_);
        }

        // Remove this caller connection from container
        container->RemoveCallerConnection(this);

        // Clear local call record and connection
        localCallRecord_->ClearData();
        localCallRecord_.reset();
    }
}

void CallerConnection::OnRemoteStateChanged(const AppExecFwk::ElementName &element, int32_t abilityState)
{
    if (localCallRecord_ == nullptr) {
        TAG_LOGD(AAFwkTag::LOCAL_CALL, "local call record is nullptr.");
        return;
    }

    localCallRecord_->NotifyRemoteStateChanged(abilityState);

    return;
}

int32_t LocalCallContainer::GetCurrentUserId()
{
    if (currentUserId_ == DEFAULT_INVAL_VALUE) {
        AppExecFwk::OsAccountManagerWrapper::GetOsAccountLocalIdFromProcess(currentUserId_);
    }
    TAG_LOGD(AAFwkTag::LOCAL_CALL, "currentUserId %{public}d", currentUserId_);
    return currentUserId_;
}

int32_t LocalCallContainer::GetValidUserId(int32_t accountId)
{
    TAG_LOGD(AAFwkTag::LOCAL_CALL, "accountId is %{public}d", accountId);
    if (accountId > 0 && accountId != GetCurrentUserId()) {
        return accountId;
    }

    return DEFAULT_INVAL_VALUE;
}
} // namespace AbilityRuntime
} // namespace OHOS
