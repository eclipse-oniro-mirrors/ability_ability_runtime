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

#ifndef OHOS_ABILITY_RUNTIME_DATAOBS_MGR_SERVICE_H
#define OHOS_ABILITY_RUNTIME_DATAOBS_MGR_SERVICE_H

#include <memory>
#include <list>
#include <shared_mutex>
#include <singleton.h>
#include <thread_ex.h>
#include "cpp/mutex.h"

#include "bundle_mgr_interface.h"
#include "data_share_permission.h"
#include "dataobs_mgr_inner.h"
#include "dataobs_mgr_inner_common.h"
#include "dataobs_mgr_inner_ext.h"
#include "dataobs_mgr_inner_pref.h"
#include "dataobs_mgr_stub.h"
#include "iremote_object.h"
#include "system_ability.h"
#include "task_handler_wrap.h"
#include "uri.h"

namespace OHOS {
namespace AAFwk {
using namespace AppExecFwk;
enum class DataObsServiceRunningState { STATE_NOT_START, STATE_RUNNING };
constexpr char SHARE_PREFERENCES[] = "sharepreferences";
constexpr char RELATIONAL_STORE[] = "rdb";
/**
 * @class DataObsMgrService
 * DataObsMgrService provides a facility for dataobserver.
 */
class DataObsMgrService : public SystemAbility,
                          public DataObsManagerStub,
                          public std::enable_shared_from_this<DataObsMgrService> {
    DECLARE_DELAYED_SINGLETON(DataObsMgrService)
    DECLEAR_SYSTEM_ABILITY(DataObsMgrService)
public:
    void OnStart() override;
    void OnStop() override;
    DataObsServiceRunningState QueryServiceState() const;

    std::pair<bool, struct ObserverNode> ConstructObserverNode(sptr<IDataAbilityObserver> dataObserver,
        int32_t userId, uint32_t tokenId, int32_t pid);
    virtual int RegisterObserver(const Uri &uri,
        sptr<IDataAbilityObserver> dataObserver, int32_t userId = DATAOBS_DEFAULT_CURRENT_USER,
        DataObsOption opt = DataObsOption()) override;
    virtual int RegisterObserverFromExtension(const Uri &uri,
        sptr<IDataAbilityObserver> dataObserver, int32_t userId = DATAOBS_DEFAULT_CURRENT_USER,
        DataObsOption opt = DataObsOption()) override;
    virtual int UnregisterObserver(const Uri &uri,
        sptr<IDataAbilityObserver> dataObserver, int32_t userId = DATAOBS_DEFAULT_CURRENT_USER,
        DataObsOption opt = DataObsOption()) override;
    virtual int NotifyChange(const Uri &uri, int32_t userId = DATAOBS_DEFAULT_CURRENT_USER,
        DataObsOption opt = DataObsOption()) override;
    virtual int NotifyChangeFromExtension(const Uri &uri, int32_t userId = DATAOBS_DEFAULT_CURRENT_USER,
        DataObsOption opt = DataObsOption()) override;
    virtual Status RegisterObserverExt(const Uri &uri, sptr<IDataAbilityObserver> dataObserver,
        bool isDescendants, DataObsOption opt = DataObsOption()) override;
    virtual Status UnregisterObserverExt(const Uri &uri, sptr<IDataAbilityObserver> dataObserver,
        DataObsOption opt = DataObsOption()) override;
    virtual Status UnregisterObserverExt(sptr<IDataAbilityObserver> dataObserver,
        DataObsOption opt = DataObsOption()) override;
    virtual Status NotifyChangeExt(const ChangeInfo &changeInfo, DataObsOption opt = DataObsOption()) override;
    virtual Status NotifyProcessObserver(const std::string &key, const sptr<IRemoteObject> &observer,
        DataObsOption opt = DataObsOption()) override;

    /**
     * @brief DataObs hidumper.
     * @param fd Indicates the fd.
     * @param args Indicates the params.
     * @return Returns the dump result.
     */
    int Dump(int fd, const std::vector<std::u16string>& args) override;

private:
    struct FocusedAppInfo {
        int32_t left = 0;
        int32_t top = 0;
        uint32_t width = 0;
        uint32_t height = 0;
        sptr<IRemoteObject> abilityToken = nullptr;
    };
    bool Init();
    void Dump(const std::vector<std::u16string>& args, std::string& result) const;
    void ShowHelp(std::string& result) const;
    Status DeepCopyChangeInfo(const ChangeInfo &src, ChangeInfo &dst) const;
    FocusedAppInfo GetFocusedWindowInfo() const;
    sptr<IRemoteObject> GetAbilityManagerService() const;
    static int32_t GetCallingUserId(uint32_t tokenId);
    static bool IsSystemApp(uint32_t tokenId, uint64_t fullTokenId);
    static bool IsCallingPermissionValid(DataObsOption &opt, int32_t userId, int32_t callingUserId);
    static bool IsCallingPermissionValid(DataObsOption &opt);
    static bool IsDataMgrService(uint32_t tokenId, int32_t uid);
    static bool IsSystemAppWithAllowedProvider(uint32_t callerTokenId, uint64_t fullTokenId,
        const std::string &callingFunc, const Uri &uri, bool isExtension);
    int32_t RegisterObserverInner(const Uri &uri, sptr<IDataAbilityObserver> dataObserver, int32_t userId,
        DataObsOption opt, bool isExtension);
    std::pair<Status, std::string> GetUriPermission(Uri &uri, bool isRead, ObserverInfo &info);
    int32_t VerifyDataShareExtension(Uri &uri, ObserverInfo &info);
    int32_t VerifyDataSharePermission(Uri &uri, bool isRead, ObserverInfo &info);
    int32_t ConstructRegisterObserver(const Uri &uri, sptr<IDataAbilityObserver> dataObserver,
        uint32_t token, int32_t userId, int32_t pid);
    Status VerifyDataSharePermissionInner(Uri &uri, bool isRead, ObserverInfo &info);
    int32_t NotifyChangeInner(Uri &uri, int32_t userId,
        DataObsOption opt, bool isExtension);
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
    bool IsTaskOverLimit();
    std::pair<Status, std::vector<NotifyInfo>> MakeNotifyInfos(ChangeInfo &changes, DataObsOption opt,
        uint32_t tokenId, int32_t userId);
    void SubmitNotifyChangeTask(Uri &uri, int32_t userId, std::string readPermission,
        ObserverInfo &info);
private:
    static constexpr std::uint32_t TASK_COUNT_MAX = 50;
    ffrt::mutex taskCountMutex_;
    std::uint32_t taskCount_ = 0;
    std::shared_ptr<TaskHandlerWrap> handler_;
    std::shared_ptr<DataShare::DataSharePermission> permission_;
    DataObsServiceRunningState state_;

    std::shared_ptr<DataObsMgrInner> dataObsMgrInner_;
    std::shared_ptr<DataObsMgrInnerExt> dataObsMgrInnerExt_;
    std::shared_ptr<DataObsMgrInnerPref> dataObsMgrInnerPref_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_DATAOBS_MGR_SERVICE_H
