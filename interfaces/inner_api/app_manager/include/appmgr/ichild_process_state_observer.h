/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_ICHILD_PROCESS_STATE_OBSERVER_H
#define OHOS_ABILITY_RUNTIME_ICHILD_PROCESS_STATE_OBSERVER_H

#include "iremote_broker.h"
#include "child_process_state_data.h"

namespace OHOS {
namespace AppExecFwk {
class IChildProcessStateObserver : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.appexecfwk.IChildProcessStateObserver");

    /**
     * Called when one child process's state changes.
     *
     * @param childProcessStateData retrieved state data.
     */
    virtual void OnChildProcessStateChanged(const ChildProcessStateData &childProcessStateData) = 0;

    enum {
        ON_CHILD_PROCESS_STATE_CHANGED = 0,
    };
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ICHILD_PROCESS_STATE_OBSERVER_H
