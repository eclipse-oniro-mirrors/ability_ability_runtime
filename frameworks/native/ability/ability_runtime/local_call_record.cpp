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
#include "local_call_record.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t FOREGROUND = 2;
constexpr int32_t BACKGROUND = 4;
}
int64_t LocalCallRecord::callRecordId = 0;
LocalCallRecord::LocalCallRecord(const AppExecFwk::ElementName& elementName)
{
    recordId_ = callRecordId++;
    elementName_ = elementName;
}

LocalCallRecord::~LocalCallRecord()
{
    ClearData();
}

void LocalCallRecord::ClearData()
{
    sptr<IRemoteObject> remote;
    sptr<IRemoteObject::DeathRecipient> recipient;
    {
        std::lock_guard lock(remoteMutex_);
        if (remoteObject_ == nullptr) {
            return;
        }
        remote = remoteObject_;
        recipient = callRecipient_;
        remoteObject_ = nullptr;
        callRecipient_ = nullptr;
    }

    if (recipient != nullptr) {
        remote->RemoveDeathRecipient(recipient);
    }

    {
        std::lock_guard lock(callersMutex_);
        callers_.clear();
    }
}

void LocalCallRecord::SetRemoteObject(const sptr<IRemoteObject>& call,
    sptr<IRemoteObject::DeathRecipient> callRecipient)
{
    if (call == nullptr) {
        TAG_LOGE(AAFwkTag::LOCAL_CALL, "null call");
        return;
    }

    {
        std::lock_guard lock(remoteMutex_);
        if (remoteObject_ != nullptr) {
            TAG_LOGE(AAFwkTag::LOCAL_CALL, "remoteObject already set");
            return;
        }
        remoteObject_ = call;
        callRecipient_ = callRecipient;
    }

    if (callRecipient != nullptr) {
        call->AddDeathRecipient(callRecipient);
    }
}

void LocalCallRecord::AddCaller(const std::shared_ptr<CallerCallBack>& callback)
{
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::LOCAL_CALL, "null callback");
        return;
    }

    callback->SetRecord(weak_from_this());
    {
        std::lock_guard lock(callersMutex_);
        callers_.emplace_back(callback);
    }
}

bool LocalCallRecord::RemoveCaller(const std::shared_ptr<CallerCallBack>& callback)
{
    bool found = false;
    {
        std::lock_guard lock(callersMutex_);
        if (callers_.empty()) {
            TAG_LOGE(AAFwkTag::LOCAL_CALL, "empty callers_");
            return false;
        }

        auto iter = std::find(callers_.begin(), callers_.end(), callback);
        if (iter != callers_.end()) {
            callers_.erase(iter);
            found = true;
        } else {
            TAG_LOGE(AAFwkTag::LOCAL_CALL, "callback not find");
        }
    }

    if (found && callback != nullptr) {
        callback->InvokeOnRelease(ON_RELEASE);
    }
    return found;
}

void LocalCallRecord::OnCallStubDied()
{
    TAG_LOGI(AAFwkTag::LOCAL_CALL, "OnCallStubDied");
    std::vector<std::shared_ptr<CallerCallBack>> callersCopy;
    {
        std::lock_guard lock(callersMutex_);
        callersCopy = callers_;
    }
    for (const auto& callBack : callersCopy) {
        if (callBack != nullptr) {
            TAG_LOGI(AAFwkTag::LOCAL_CALL, "Notify caller released");
            callBack->InvokeOnRelease(ON_DIED);
        }
    }
}

void LocalCallRecord::InvokeCallBack() const
{
    auto remoteObject = GetRemoteObject();
    if (remoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::LOCAL_CALL, "null object");
        return;
    }

    std::vector<std::shared_ptr<CallerCallBack>> callersCopy;
    {
        std::lock_guard lock(callersMutex_);
        callersCopy = callers_;
    }
    for (const auto& callBack : callersCopy) {
        if (callBack != nullptr && !callBack->IsCallBack()) {
            callBack->InvokeCallBack(remoteObject);
        }
    }
}

void LocalCallRecord::NotifyRemoteStateChanged(int32_t abilityState)
{
    std::string state = "";
    if (abilityState == FOREGROUND) {
        state = "foreground";
    } else if (abilityState == BACKGROUND) {
        state = "background";
    }

    std::vector<std::shared_ptr<CallerCallBack>> callersCopy;
    {
        std::lock_guard lock(callersMutex_);
        callersCopy = callers_;
    }
    for (const auto& callBack : callersCopy) {
        if (callBack != nullptr && callBack->IsCallBack()) {
            TAG_LOGI(AAFwkTag::LOCAL_CALL, "not null callback and is callback ");
            callBack->InvokeOnNotify(state);
        }
    }
}

sptr<IRemoteObject> LocalCallRecord::GetRemoteObject() const
{
    std::lock_guard lock(remoteMutex_);
    return remoteObject_;
}

AppExecFwk::ElementName LocalCallRecord::GetElementName() const
{
    return elementName_;
}

bool LocalCallRecord::IsExistCallBack() const
{
    std::lock_guard lock(callersMutex_);
    return !callers_.empty();
}

int LocalCallRecord::GetRecordId() const
{
    return recordId_;
}

std::vector<std::shared_ptr<CallerCallBack>> LocalCallRecord::GetCallers() const
{
    std::lock_guard lock(callersMutex_);
    return callers_;
}

bool LocalCallRecord::IsSameObject(const sptr<IRemoteObject>& remote) const
{
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::LOCAL_CALL, "null remote");
        return false;
    }

    std::lock_guard lock(remoteMutex_);
    bool retVal = (remoteObject_ == remote);
    TAG_LOGD(AAFwkTag::LOCAL_CALL, "remoteObject_ matches remote: %{public}s", retVal ? "true" : "false");
    return retVal;
}

void LocalCallRecord::SetIsSingleton(bool flag)
{
    isSingleton_ = flag;
}

bool LocalCallRecord::IsSingletonRemote()
{
    return isSingleton_;
}

void LocalCallRecord::SetConnection(const sptr<IRemoteObject> &connect)
{
    connection_ = connect;
}

sptr<IRemoteObject> LocalCallRecord::GetConnection()
{
    return connection_.promote();
}

void LocalCallRecord::SetUserId(int32_t userId)
{
    userId_ = userId;
}

int32_t LocalCallRecord::GetUserId() const
{
    return userId_;
}
} // namespace AbilityRuntime
} // namespace OHOS
