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
#ifndef OHOS_ABILITY_RUNTIME_DATA_ABILITY_OBSERVER_PROXY_H
#define OHOS_ABILITY_RUNTIME_DATA_ABILITY_OBSERVER_PROXY_H

#include "data_ability_observer_interface.h"

#include <iremote_proxy.h>

namespace OHOS {
namespace AAFwk {
class DataAbilityObserverProxy : public IRemoteProxy<IDataAbilityObserver> {
public:
    explicit DataAbilityObserverProxy(const sptr<IRemoteObject> &remote);
    virtual ~DataAbilityObserverProxy();

    /**
     * @brief Called back to notify that the data being observed has changed.
     *
     * @param uri Indicates the path of the data to operate.
     */
    void OnChange() override;

    /**
     * @brief Called back to notify that the data being observed has changed.
     *
     * @param changeInfo Indicates the info of the data to operate.
     */
    void OnChangeExt(const ChangeInfo &changeInfo) override;

    /**
     * @brief Called back to notify that the data being observed has changed.
     *
     * @param uri Indicates the path of the data to operate.
     */
    void OnChangePreferences(const std::string &key) override;

private:
    static inline BrokerDelegator<DataAbilityObserverProxy> delegator_;
    int32_t SendTransactCmd(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void SetMessageOption(MessageOption &option);
    uint32_t messageCount_ = 0;
    std::mutex countMutex_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_DATA_ABILITY_OBSERVER_PROXY_H
