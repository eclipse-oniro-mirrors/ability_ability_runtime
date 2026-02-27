/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_DATAOBS_MGR_INNER_EXT_H
#define OHOS_ABILITY_RUNTIME_DATAOBS_MGR_INNER_EXT_H

#include <atomic>
#include <list>
#include <string>
#include <memory>
#include <map>
#include <set>
#include <mutex>
#include "cpp/mutex.h"

#include "data_ability_observer_interface.h"
#include "dataobs_mgr_errors.h"
#include "dataobs_mgr_inner_common.h"
#include "iremote_object.h"
#include "refbase.h"
#include "uri.h"

namespace OHOS {
namespace AAFwk {
class DataObsMgrInnerExt : public std::enable_shared_from_this<DataObsMgrInnerExt> {
public:

    DataObsMgrInnerExt();
    virtual ~DataObsMgrInnerExt();

    Status HandleRegisterObserver(Uri &uri, sptr<IDataAbilityObserver> dataObserver, ObserverInfo &info,
        bool isDescendants = false);
    Status HandleUnregisterObserver(Uri &uri, sptr<IDataAbilityObserver> dataObserver);
    Status HandleUnregisterObserver(sptr<IDataAbilityObserver> dataObserver);
    Status HandleNotifyChange(const ChangeInfo &changeInfo, int32_t userId,
        std::vector<NotifyInfo> &verifyResult);
    void OnCallBackDied(const wptr<IRemoteObject> &remote);

private:
    struct DeathRecipientRef {
        DeathRecipientRef(sptr<IRemoteObject::DeathRecipient> deathRec) : deathRecipient(deathRec), ref(1) {}
        sptr<IRemoteObject::DeathRecipient> deathRecipient;
        std::atomic<uint32_t> ref;
    };

    struct Entry {
        Entry(sptr<IDataAbilityObserver> obs, int32_t userId, uint32_t tokenId,
            std::shared_ptr<DeathRecipientRef> deathRef, bool isDes)
            : observer(obs), userId(userId), tokenId(tokenId), deathRecipientRef(deathRef), isDescendants(isDes)
        {
            // nextEntryId_ increased by 1
            entryId = nextEntryId_.fetch_add(1);
        }
        sptr<IDataAbilityObserver> observer;
        int32_t userId;
        uint32_t tokenId = 0;
        std::shared_ptr<DeathRecipientRef> deathRecipientRef;
        bool isDescendants;
        std::string permission;
        int32_t pid = 0;
        int64_t entryId = -1;
        static inline std::atomic<int64_t> nextEntryId_ = 1;
    };

    struct ObsNotifyInfo {
        ObsNotifyInfo()
        {
            uriList = std::list<NotifyInfo>();
        }
        uint32_t tokenId = 0;
        int32_t pid = 0;
        std::list<NotifyInfo> uriList;
    };

    using ObsMap = std::map<sptr<IDataAbilityObserver>, ObsNotifyInfo>;
    using EntryList = std::list<Entry>;

    class Node {
    public:
        Node(const std::string &name);
        void GetObs(const std::vector<std::string> &path, uint32_t index, NotifyInfo &info,
            int32_t userId, ObsMap &obsMap, std::set<DataObsMgrInnerExt::Node *> &nodeLogSet);
        bool AddObserver(const std::vector<std::string> &path, uint32_t index, const Entry &entry);
        bool RemoveObserver(const std::vector<std::string> &path, uint32_t index,
            sptr<IDataAbilityObserver> dataObserver);
        inline bool RemoveObserver(sptr<IDataAbilityObserver> dataObserver);
        bool RemoveObserver(sptr<IRemoteObject> dataObserver);
        bool IsLimit(const Entry &entry);

    private:
        std::string name_;
        EntryList entrys_;
        std::map<std::string, std::shared_ptr<Node>> childrens_;
    };

    std::shared_ptr<DeathRecipientRef> AddObsDeathRecipient(const sptr<IRemoteObject> &dataObserver);
    void RemoveObsDeathRecipient(const sptr<IRemoteObject> &dataObserver, bool isForce = false);
    void NotifyObserver(const ChangeInfo &changeInfo, sptr<IDataAbilityObserver> obs,
        ObsNotifyInfo &info);

    static constexpr uint32_t OBS_NUM_MAX = 50;
    static constexpr uint32_t OBS_ALL_NUM_MAX = OBS_NUM_MAX * OBS_NUM_MAX;
    static constexpr uint32_t MAX_URI_PATH_SIZE = 25;

    ffrt::mutex nodeMutex_;
    std::shared_ptr<Node> root_;
    std::map<sptr<IRemoteObject>, std::shared_ptr<DeathRecipientRef>> obsRecipientRefs;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_DATAOBS_MGR_INNER_H
