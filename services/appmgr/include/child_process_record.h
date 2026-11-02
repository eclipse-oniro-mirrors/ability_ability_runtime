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

#ifndef OHOS_ABILITY_RUNTIME_CHILD_PROCESS_RECORD_H
#define OHOS_ABILITY_RUNTIME_CHILD_PROCESS_RECORD_H

#include <memory>
#include <string>
#include <sys/types.h>

#include "app_death_recipient.h"
#include "app_mgr_constants.h"
#include "child_scheduler_interface.h"
#include "child_process_info.h"
#include "child_process_request.h"
#include "running_process_info.h"

namespace OHOS {
namespace AppExecFwk {
class AppRunningRecord;

class ChildProcessRecord {
public:
    ChildProcessRecord(pid_t hostPid, const ChildProcessRequest &request,
        const std::shared_ptr<AppRunningRecord> hostRecord);
    ChildProcessRecord(pid_t hostPid, const std::string &libName, const std::shared_ptr<AppRunningRecord> hostRecord,
        const sptr<IRemoteObject> &mainProcessCb, int32_t childProcessCount, bool isStartWithDebug,
        const std::string &customProcessName);
    virtual ~ChildProcessRecord();

    static std::shared_ptr<ChildProcessRecord> CreateChildProcessRecord(pid_t hostPid,
        const ChildProcessRequest &request, const std::shared_ptr<AppRunningRecord> hostRecord);
    static std::shared_ptr<ChildProcessRecord> CreateNativeChildProcessRecord(pid_t hostPid, const std::string &libName,
        const std::shared_ptr<AppRunningRecord> hostRecord, const sptr<IRemoteObject> &mainProcessCb,
        int32_t childProcessCount, bool isStartWithDebug, const std::string &customProcessName);

    void SetPid(pid_t pid);
    pid_t GetPid() const;
    pid_t GetHostPid() const;
    void SetUid(int32_t uid);
    int32_t GetUid() const;
    std::string GetProcessName() const;
    std::string GetSrcEntry() const;
    std::string GetEntryFunc() const;
    std::shared_ptr<AppRunningRecord> GetHostRecord() const;
    void SetScheduler(const sptr<IChildScheduler> &scheduler);
    sptr<IChildScheduler> GetScheduler() const;
    void SetDeathRecipient(const sptr<AppDeathRecipient> recipient);
    void RegisterDeathRecipient();
    void RemoveDeathRecipient();
    void ScheduleExitProcessSafely();
    bool isStartWithDebug();
    int32_t GetChildProcessType() const;
    sptr<IRemoteObject> GetMainProcessCallback() const;
    void ClearMainProcessCallback();
    void SetEntryParams(const std::string &entryParams);
    std::string GetEntryParams() const;
    ProcessType GetProcessType() const;
    bool IsNativeSpawnStarted() const;
    void SetAttachTimeoutStartTime(const std::chrono::system_clock::time_point &time);
    std::chrono::system_clock::time_point GetAttachTimeoutStartTime() const;
    bool IsStatic() const;
    AppProcessState GetState() const;
    void SetState(const AppProcessState state);

private:
    void MakeProcessName(const std::shared_ptr<AppRunningRecord> hostRecord, const std::string &customProcessName);

    bool isStartWithDebug_;
    bool isStaticChildProcess_ = false;
    pid_t hostPid_ = 0;
    int32_t childProcessCount_ = 0;
    int32_t childProcessType_ = CHILD_PROCESS_TYPE_JS;
    std::weak_ptr<AppRunningRecord> hostRecord_;
    sptr<IChildScheduler> scheduler_ = nullptr;
    sptr<AppDeathRecipient> deathRecipient_ = nullptr;
    sptr<IRemoteObject> mainProcessCb_ = nullptr;
    int32_t uid_ = 0;
    ProcessType processType_ = ProcessType::CHILD;
    pid_t pid_ = 0;
    std::string srcEntry_;
    std::string processName_;
    std::string entryFunc_;
    std::string entryParams_;
    std::chrono::system_clock::time_point attachTimeoutStartTime_;
    // Atomic Variables
    std::atomic<AppProcessState> curState_ = AppProcessState::APP_STATE_CREATE;  // current state of this process
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_CHILD_PROCESS_RECORD_H
