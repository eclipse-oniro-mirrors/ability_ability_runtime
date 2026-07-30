/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_DATAOBS_MGR_PROXY_H
#define OHOS_ABILITY_RUNTIME_DATAOBS_MGR_PROXY_H

#include "dataobs_mgr_interface.h"
#include "iremote_proxy.h"
#include "dataobs_mgr_errors.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class DataObsManagerProxy
 * DataObsManagerProxy.
 */
class DataObsManagerProxy : public IRemoteProxy<IDataObsMgr> {
public:
    explicit DataObsManagerProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IDataObsMgr>(impl)
    {}

    virtual ~DataObsManagerProxy()
    {}

    /**
     * Registers an observer to DataObsMgr specified by the given Uri.
     *
     * @param uri, Indicates the path of the data to operate.
     * @param dataObserver, Indicates the IDataAbilityObserver object.
     *
     * @return Returns ERR_OK on success, others on failure.
     */

    virtual int RegisterObserver(const Uri &uri, sptr<IDataAbilityObserver> dataObserver,
        int32_t userId = DATAOBS_DEFAULT_CURRENT_USER, DataObsOption opt = DataObsOption()) override;

    virtual int RegisterObserverFromExtension(const Uri &uri, sptr<IDataAbilityObserver> dataObserver,
        int32_t userId = DATAOBS_DEFAULT_CURRENT_USER, DataObsOption opt = DataObsOption()) override;

    /**
     * Deregisters an observer used for DataObsMgr specified by the given Uri.
     *
     * @param uri, Indicates the path of the data to operate.
     * @param dataObserver, Indicates the IDataAbilityObserver object.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int UnregisterObserver(const Uri &uri, sptr<IDataAbilityObserver> dataObserver,
        int32_t userId = DATAOBS_DEFAULT_CURRENT_USER, DataObsOption opt = DataObsOption()) override;

    /**
     * Notifies the registered observers of a change to the data resource specified by Uri.
     *
     * @param uri, Indicates the path of the data to operate.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int NotifyChange(const Uri &uri, int32_t userId = DATAOBS_DEFAULT_CURRENT_USER,
        DataObsOption opt = DataObsOption()) override;

    /**
     * Notifies the registered observers of a change to the data resource specified by Uri.
     *
     * @param uri, Indicates the path of the data to operate.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int NotifyChangeFromExtension(const Uri &uri, int32_t userId = DATAOBS_DEFAULT_CURRENT_USER,
        DataObsOption opt = DataObsOption()) override;

    /**
     * Registers an observer to DataObsMgr specified by the given Uri.
     *
     * @param uri, Indicates the path of the data to operate.
     * @param dataObserver, Indicates the IDataAbilityObserver object.
     * @param isDescendants, Indicates the Whether to note the change of descendants.
     *
     * @return Returns SUCCESS on success, others on failure.
     */
    virtual Status RegisterObserverExt(const Uri &uri, sptr<IDataAbilityObserver> dataObserver,
        bool isDescendants, DataObsOption opt = DataObsOption()) override;

    /**
     * Deregisters an observer used for DataObsMgr specified by the given Uri.
     *
     * @param uri, Indicates the path of the data to operate.
     * @param dataObserver, Indicates the IDataAbilityObserver object.
     *
     * @return Returns SUCCESS on success, others on failure.
     */
    virtual Status UnregisterObserverExt(const Uri &uri, sptr<IDataAbilityObserver> dataObserver,
        DataObsOption opt = DataObsOption()) override;

    /**
     * Deregisters dataObserver used for DataObsMgr specified
     *
     * @param dataObserver, Indicates the IDataAbilityObserver object.
     *
     * @return Returns SUCCESS on success, others on failure.
     */
    virtual Status UnregisterObserverExt(sptr<IDataAbilityObserver> dataObserver,
        DataObsOption opt = DataObsOption()) override;

    /**
     * Notifies the registered observers of a change to the data resource specified by Uris.
     *
     * @param changeInfo Indicates the info of the data to operate.
     *
     * @return Returns SUCCESS on success, others on failure.
     */
    virtual Status NotifyChangeExt(const ChangeInfo &changeInfo, DataObsOption opt = DataObsOption()) override;

    /**
     * Notifies the process observer with the given progress key and cancel observer.
     *
     * @param key Identifies the progress of a specific task.

     * @param observer Observer for monitoring the ongoing process.
     *
     * @return Returns SUCCESS on success, others on failure.
     */
    virtual Status NotifyProcessObserver(const std::string &key, const sptr<IRemoteObject> observer) override;

private:
    bool WriteInterfaceToken(MessageParcel &data);
    bool WriteParam(MessageParcel &data, const Uri &uri, sptr<IDataAbilityObserver> dataObserver);
    bool WriteObsOpt(MessageParcel &data, DataObsOption opt);
    int SendTransactCmd(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);

private:
    static inline BrokerDelegator<DataObsManagerProxy> delegator_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_DATAOBS_MGR_PROXY_H
