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
#include <singleton.h>
#include <thread_ex.h>
#include <unordered_map>
#include "cpp/mutex.h"

#include "dataobs_mgr_inner.h"
#include "dataobs_mgr_inner_ext.h"
#include "dataobs_mgr_inner_pref.h"
#include "dataobs_mgr_stub.h"
#include "iremote_object.h"
#include "system_ability.h"
#include "task_handler_wrap.h"
#include "uri.h"

namespace OHOS {
namespace AAFwk {
enum class DataObsServiceRunningState { STATE_NOT_START, STATE_RUNNING };
constexpr char SHARE_PREFERENCES[] = "sharepreferences";
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

    virtual int RegisterObserver(const Uri &uri, sptr<IDataAbilityObserver> dataObserver) override;
    virtual int UnregisterObserver(const Uri &uri, sptr<IDataAbilityObserver> dataObserver) override;
    virtual int NotifyChange(const Uri &uri) override;
    virtual Status RegisterObserverExt(const Uri &uri, sptr<IDataAbilityObserver> dataObserver,
        bool isDescendants) override;
    virtual Status UnregisterObserverExt(const Uri &uri, sptr<IDataAbilityObserver> dataObserver) override;
    virtual Status UnregisterObserverExt(sptr<IDataAbilityObserver> dataObserver) override;
    virtual Status NotifyChangeExt(const ChangeInfo &changeInfo) override;

    /**
     * @brief DataObs hidumper.
     * @param fd Indicates the fd.
     * @param args Indicates the params.
     * @return Returns the dump result.
     */
    int Dump(int fd, const std::vector<std::u16string>& args) override;

private:
    bool Init();
    void Dump(const std::vector<std::u16string>& args, std::string& result) const;
    void ShowHelp(std::string& result) const;
    Status DeepCopyChangeInfo(const ChangeInfo &src, ChangeInfo &dst) const;
private:
    static constexpr std::uint32_t TASK_COUNT_MAX = 50;
    ffrt::mutex taskCountMutex_;
    std::uint32_t taskCount_ = 0;
    std::shared_ptr<TaskHandlerWrap> handler_;

    DataObsServiceRunningState state_;

    std::shared_ptr<DataObsMgrInner> dataObsMgrInner_;
    std::shared_ptr<DataObsMgrInnerExt> dataObsMgrInnerExt_;
    std::shared_ptr<DataObsMgrInnerPref> dataObsMgrInnerPref_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_DATAOBS_MGR_SERVICE_H
