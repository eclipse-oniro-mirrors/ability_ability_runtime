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

#ifndef OHOS_ABILITY_RUNTIME_CHILD_SCHEDULER_INTERFACE_H
#define OHOS_ABILITY_RUNTIME_CHILD_SCHEDULER_INTERFACE_H

#include "iremote_broker.h"

namespace OHOS {
namespace AppExecFwk {
class IChildScheduler : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.appexecfwk.ChildScheduler");

    /**
     * Notify chile process to load js file.
     */
    virtual bool ScheduleLoadChild() = 0;

    /**
     * Notify chile process to exit safely.
     */
    virtual bool ScheduleExitProcessSafely() = 0;

    /**
     * Notify child process run main proc from shared lib.
     * @param mainProcessCb Main process callback ipc object
     */
    virtual bool ScheduleRunNativeProc(const sptr<IRemoteObject> &mainProcessCb) = 0;

    enum class Message {
        SCHEDULE_LOAD_JS = 0,
        SCHEDULE_EXIT_PROCESS_SAFELY = 1,
        SCHEDULE_RUN_NATIVE_PROC = 2,
    };
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_CHILD_SCHEDULER_INTERFACE_H
