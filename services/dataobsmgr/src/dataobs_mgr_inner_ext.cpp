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
#include "dataobs_mgr_inner_ext.h"

#include "data_ability_observer_stub.h"
#include "data_share_permission.h"
#include "datashare_errno.h"
#include "datashare_log.h"
#include "dataobs_mgr_errors.h"
#include "hilog_tag_wrapper.h"
#include "common_utils.h"

namespace OHOS {
namespace AAFwk {
using namespace DataShare;
DataObsMgrInnerExt::DataObsMgrInnerExt() : root_(std::make_shared<Node>("root")) {}

DataObsMgrInnerExt::~DataObsMgrInnerExt() {}

Status DataObsMgrInnerExt::HandleRegisterObserver(Uri &uri, sptr<IDataAbilityObserver> dataObserver,
    ObserverInfo &info, bool isDescendants)
{
    if (dataObserver->AsObject() == nullptr) {
        return DATA_OBSERVER_IS_NULL;
    }
    std::lock_guard<ffrt::mutex> lock(nodeMutex_);
    auto deathRecipientRef = AddObsDeathRecipient(dataObserver->AsObject());
    if (deathRecipientRef == nullptr) {
        return DATAOBS_SERVICE_OBS_LIMMIT;
    }

    std::vector<std::string> path = { uri.GetScheme(), uri.GetAuthority() };
    uri.GetPathSegments(path);
    if (path.size() >= MAX_URI_PATH_SIZE) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "path size:%{public}zu invalid", path.size());
        return DATAOBS_INVALID_URI;
    }
    Entry entry = Entry(dataObserver, info.userId, info.tokenId, deathRecipientRef, isDescendants);
    entry.pid = info.pid;
    if (root_ != nullptr && !root_->AddObserver(path, 0, entry)) {
        TAG_LOGE(AAFwkTag::DBOBSMGR,
            "subscribers:%{public}s num maxed",
            CommonUtils::Anonymous(uri.ToString()).c_str());
        RemoveObsDeathRecipient(dataObserver->AsObject());
        return DATAOBS_SERVICE_OBS_LIMMIT;
    }
    TAG_LOGI(AAFwkTag::DBOBSMGR, "subscribers:%{public}s p:%{public}d Id:%{public}" PRId64,
        CommonUtils::Anonymous(uri.ToString()).c_str(), entry.pid, entry.entryId);
    return SUCCESS;
}

Status DataObsMgrInnerExt::HandleUnregisterObserver(Uri &uri, sptr<IDataAbilityObserver> dataObserver)
{
    if (dataObserver->AsObject() == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "null dataObserver, uri:%{public}s num maxed",
            CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATA_OBSERVER_IS_NULL;
    }
    std::lock_guard<ffrt::mutex> lock(nodeMutex_);
    std::vector<std::string> path = { uri.GetScheme(), uri.GetAuthority() };
    uri.GetPathSegments(path);
    if (path.size() >= MAX_URI_PATH_SIZE) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "path size:%{public}zu invalid", path.size());
        return DATAOBS_INVALID_URI;
    }
    if (root_ != nullptr) {
        root_->RemoveObserver(path, 0, dataObserver);
    }
    RemoveObsDeathRecipient(dataObserver->AsObject());
    return SUCCESS;
}

Status DataObsMgrInnerExt::HandleUnregisterObserver(sptr<IDataAbilityObserver> dataObserver)
{
    if (dataObserver->AsObject() == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "null dataObserver");
        return DATA_OBSERVER_IS_NULL;
    }
    std::lock_guard<ffrt::mutex> lock(nodeMutex_);
    if (root_ != nullptr) {
        root_->RemoveObserver(dataObserver);
    }
    RemoveObsDeathRecipient(dataObserver->AsObject(), true);
    return SUCCESS;
}

void DataObsMgrInnerExt::NotifyObserver(const ChangeInfo &changeInfo, sptr<IDataAbilityObserver> obs,
    ObsNotifyInfo &info)
{
    std::list sendUriList = std::list<Uri>();
    for (NotifyInfo &verifyInfo : info.uriList) {
        bool success = DataShare::DataSharePermission::VerifyPermission(verifyInfo.uri, info.tokenId,
            verifyInfo.readPermission, verifyInfo.isSilentUri);
        if (!success) {
            TAG_LOGE(AAFwkTag::DBOBSMGR, "VerifyPermission fail, uri %{public}s permission %{public}s pid %{public}d"
                " token %{public}d", verifyInfo.uri.ToString().c_str(), verifyInfo.readPermission.c_str(),
                info.pid, info.tokenId);
            continue;
        }
        sendUriList.push_back(verifyInfo.uri);
    }
    if (sendUriList.empty()) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "NotifyObserver all denied, uri %{public}s permission %{public}s",
            info.uriList.front().uri.ToString().c_str(), info.uriList.front().readPermission.c_str());
        return;
    }
    obs->OnChangeExt(
        { changeInfo.changeType_, move(sendUriList),
        changeInfo.data_, changeInfo.size_, changeInfo.valueBuckets_ });
}

Status DataObsMgrInnerExt::HandleNotifyChange(const ChangeInfo &changeInfo, int32_t userId,
    std::vector<NotifyInfo> &notifyInfo)
{
    if (changeInfo.uris_.size() != notifyInfo.size()) {
        LOG_ERROR("size invalid uris %{public}zu info %{public}zu", changeInfo.uris_.size(), notifyInfo.size());
        return INVALID_PARAM;
    }
    ObsMap changeRes;
    std::vector<std::string> path;
    {
        std::lock_guard<ffrt::mutex> lock(nodeMutex_);
        std::set<DataObsMgrInnerExt::Node *> nodeLogSet;
        int32_t count = 0;
        for (auto &uri : changeInfo.uris_) {
            path.clear();
            path.emplace_back(uri.GetScheme());
            path.emplace_back(uri.GetAuthority());
            uri.GetPathSegments(path);
            notifyInfo[count].uri = uri;
            root_->GetObs(path, 0, notifyInfo[count], userId, changeRes, nodeLogSet);
            count++;
        }
    }
    if (changeRes.empty()) {
        TAG_LOGD(AAFwkTag::DBOBSMGR,
            "uris no obs, changeType:%{public}ud, uris num:%{public}zu, null data:%{public}d, size:%{public}ud",
            changeInfo.changeType_, changeInfo.uris_.size(), changeInfo.data_ == nullptr, changeInfo.size_);
        return NO_OBS_FOR_URI;
    }
    for (auto &[obs, value] : changeRes) {
        if (obs == nullptr || value.uriList.empty()) {
            continue;
        }
        NotifyObserver(changeInfo, obs, value);
    }

    return SUCCESS;
}

std::shared_ptr<DataObsMgrInnerExt::DeathRecipientRef> DataObsMgrInnerExt::AddObsDeathRecipient(
    const sptr<IRemoteObject> &dataObserver)
{
    auto it = obsRecipientRefs.find(dataObserver);
    if (it != obsRecipientRefs.end()) {
        if (std::numeric_limits<uint32_t>::max() - 1 < it->second->ref) {
            TAG_LOGE(AAFwkTag::DBOBSMGR, "observer num maxed");
            return nullptr;
        }
    } else {
        std::weak_ptr<DataObsMgrInnerExt> thisWeakPtr(shared_from_this());
        sptr<IRemoteObject::DeathRecipient> deathRecipient =
            new DataObsCallbackRecipient([thisWeakPtr](const wptr<IRemoteObject> &remote) {
                auto DataObsMgrInnerExt = thisWeakPtr.lock();
                if (DataObsMgrInnerExt) {
                    DataObsMgrInnerExt->OnCallBackDied(remote);
                }
            });
        dataObserver->AddDeathRecipient(deathRecipient);
        it = obsRecipientRefs.emplace(dataObserver, std::make_shared<DeathRecipientRef>(deathRecipient)).first;
    }
    TAG_LOGD(AAFwkTag::DBOBSMGR, "add observer, sum:%{public}ud",
        it->second->ref.load(std::memory_order_relaxed));
    return it->second;
}

void DataObsMgrInnerExt::RemoveObsDeathRecipient(const sptr<IRemoteObject> &dataObserver, bool isForce)
{
    auto it = obsRecipientRefs.find(dataObserver);
    if (it == obsRecipientRefs.end()) {
        return;
    }

    if (isForce || it->second->ref <= 1) {
        TAG_LOGD(AAFwkTag::DBOBSMGR, "remove deathRecipient, sum:%{public}ud",
            it->second->ref.load(std::memory_order_relaxed));
        dataObserver->RemoveDeathRecipient(it->second->deathRecipient);
        obsRecipientRefs.erase(it);
        return;
    }
}

void DataObsMgrInnerExt::OnCallBackDied(const wptr<IRemoteObject> &remote)
{
    TAG_LOGD(AAFwkTag::DBOBSMGR, "observer died");
    auto dataObserver = remote.promote();
    if (dataObserver == nullptr) {
        return;
    }
    std::lock_guard<ffrt::mutex> lock(nodeMutex_);
    if (root_ != nullptr) {
        root_->RemoveObserver(dataObserver);
    }
    RemoveObsDeathRecipient(dataObserver, true);
}

DataObsMgrInnerExt::Node::Node(const std::string &name) : name_(name) {}

void DataObsMgrInnerExt::Node::GetObs(const std::vector<std::string> &path, uint32_t index,
    NotifyInfo &info, int32_t userId, ObsMap &obsRes, std::set<DataObsMgrInnerExt::Node *> &nodeLogSet)
{
    std::string obsStr = "";
    bool logFlag = false;
    auto nodeLogPair = nodeLogSet.insert(this);
    if (path.size() == index) {
        for (auto entry : entrys_) {
            if (entry.userId != userId && entry.userId != 0 && userId != 0) {
                TAG_LOGW(AAFwkTag::DBOBSMGR, "Not allow across user notify, uri:%{public}s, from %{public}d to"
                    "%{public}d", CommonUtils::Anonymous(info.uri.ToString()).c_str(), userId, entry.userId);
                continue;
            }
            ObsNotifyInfo &notifyInfo = obsRes.try_emplace(entry.observer, ObsNotifyInfo()).first->second;
            notifyInfo.uriList.push_back(info);
            notifyInfo.tokenId = entry.tokenId;
            notifyInfo.pid = entry.pid;
            obsStr += "p:" + std::to_string(entry.pid) + "Id:" + std::to_string(entry.entryId) + ",";
            logFlag = nodeLogPair.second;
        }
        if (logFlag) {
            TAG_LOGI(AAFwkTag::DBOBSMGR, "uri:%{public}s entrys_:%{public}s",
                CommonUtils::Anonymous(info.uri.ToString()).c_str(), obsStr.c_str());
        }
        return;
    }

    for (const auto &entry : entrys_) {
        if (entry.isDescendants) {
            if (entry.userId != userId && entry.userId != 0 && userId != 0) {
                TAG_LOGW(AAFwkTag::DBOBSMGR, "Not allow across user notify, uri:%{public}s, from %{public}d to"
                    "%{public}d", CommonUtils::Anonymous(info.uri.ToString()).c_str(), userId, entry.userId);
                continue;
            }
            ObsNotifyInfo &notifyInfo = obsRes.try_emplace(entry.observer, ObsNotifyInfo()).first->second;
            notifyInfo.uriList.push_back(info);
            notifyInfo.tokenId = entry.tokenId;
            notifyInfo.pid = entry.pid;
            obsStr += "p:" + std::to_string(entry.pid) + "Id:" + std::to_string(entry.entryId) + ",";
            logFlag = nodeLogPair.second;
        }
    }
    if (logFlag) {
        TAG_LOGI(AAFwkTag::DBOBSMGR, "uri:%{public}s entrys_:%{public}s",
            CommonUtils::Anonymous(info.uri.ToString()).c_str(), obsStr.c_str());
    }

    auto it = childrens_.find(path[index]);
    if (it == childrens_.end()) {
        return;
    }
    it->second->GetObs(path, ++index, info, userId, obsRes, nodeLogSet);
}

bool DataObsMgrInnerExt::Node::IsLimit(const Entry &entry)
{
    if (entrys_.size() >= OBS_ALL_NUM_MAX) {
        return true;
    }
    uint32_t count = 0;
    for (Entry& existEntry : entrys_) {
        if (existEntry.tokenId == entry.tokenId) {
            count++;
            if (count > OBS_NUM_MAX) {
                return true;
            }
        }
    }
    TAG_LOGE(AAFwkTag::DBOBSMGR, "subscribers num :%{public}d", count);
    return false;
}

bool DataObsMgrInnerExt::Node::AddObserver(const std::vector<std::string> &path, uint32_t index, const Entry &entry)
{
    if (path.size() == index) {
        if (IsLimit(entry)) {
            TAG_LOGE(AAFwkTag::DBOBSMGR, "subscribers num maxed, token:%{public}d", entry.tokenId);
            return false;
        }
        entry.deathRecipientRef->ref++;
        entrys_.emplace_back(entry);
        TAG_LOGI(AAFwkTag::DBOBSMGR, "p:%{public}d Id:%{public}" PRId64, entry.pid, entry.entryId);
        return true;
    }
    auto it = childrens_.try_emplace(path[index], std::make_shared<Node>(path[index])).first;
    return it->second->AddObserver(path, ++index, entry);
}

bool DataObsMgrInnerExt::Node::RemoveObserver(const std::vector<std::string> &path, uint32_t index,
    sptr<IDataAbilityObserver> dataObserver)
{
    if (index == path.size()) {
        entrys_.remove_if([dataObserver](const Entry &entry) {
            if (entry.observer->AsObject() != dataObserver->AsObject()) {
                return false;
            }
            entry.deathRecipientRef->ref--;
            TAG_LOGI(AAFwkTag::DBOBSMGR, "p:%{public}d Id:%{public}" PRId64, entry.pid, entry.entryId);
            return true;
        });
        return entrys_.empty() && childrens_.empty();
    }
    auto child = childrens_.find(path[index]);
    if (child != childrens_.end() && child->second->RemoveObserver(path, ++index, dataObserver)) {
        childrens_.erase(child);
    }
    return entrys_.empty() && childrens_.empty();
}

// remove observer of all users
bool DataObsMgrInnerExt::Node::RemoveObserver(sptr<IRemoteObject> dataObserver)
{
    for (auto child = childrens_.begin(); child != childrens_.end();) {
        if (child->second->RemoveObserver(dataObserver)) {
            child = childrens_.erase(child);
        } else {
            child++;
        }
    }
    entrys_.remove_if([dataObserver](const Entry &entry) {
        if (entry.observer->AsObject() != dataObserver) {
            return false;
        }
        entry.deathRecipientRef->ref--;
        TAG_LOGI(AAFwkTag::DBOBSMGR, "p:%{public}d Id:%{public}" PRId64, entry.pid, entry.entryId);
        return true;
    });
    return entrys_.empty() && childrens_.empty();
}

inline bool DataObsMgrInnerExt::Node::RemoveObserver(sptr<IDataAbilityObserver> dataObserver)
{
    auto obs = dataObserver->AsObject();
    return obs != nullptr && RemoveObserver(obs);
}

} // namespace AAFwk
} // namespace OHOS
