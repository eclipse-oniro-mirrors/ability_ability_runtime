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

#ifndef OHOS_ABILITY_RUNTIME_CHILD_SCHEDULER_PROXY_H
#define OHOS_ABILITY_RUNTIME_CHILD_SCHEDULER_PROXY_H

#include "child_scheduler_interface.h"

#include "iremote_proxy.h"

namespace OHOS {
namespace AppExecFwk {
class ChildSchedulerProxy : public IRemoteProxy<IChildScheduler> {
public:
    explicit ChildSchedulerProxy(const sptr<IRemoteObject> &impl);
    virtual ~ChildSchedulerProxy() = default;

    bool ScheduleLoadChild() override;
    bool ScheduleExitProcessSafely() override;
    bool ScheduleRunNativeProc(const sptr<IRemoteObject> &mainProcessCb) override;

private:
    bool WriteInterfaceToken(MessageParcel &data);
    static inline BrokerDelegator<ChildSchedulerProxy> delegator_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_CHILD_SCHEDULER_PROXY_H
