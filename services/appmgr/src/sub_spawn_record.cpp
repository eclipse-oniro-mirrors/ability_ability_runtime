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

#include "sub_spawn_record.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {

std::shared_ptr<SubSpawnRecord> SubSpawnRecord::CreateSubSpawnRecord(pid_t appPid, int32_t uid,
    const std::string &bundleName, const AppSpawnStartMsg &startMsg)
{
    if (appPid <= 0 || bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Invalid appPid or bundleName");
        return nullptr;
    }
    return std::shared_ptr<SubSpawnRecord>(new (std::nothrow) SubSpawnRecord(appPid, uid, bundleName, startMsg));
}

SubSpawnRecord::SubSpawnRecord(pid_t appPid, int32_t uid, const std::string &bundleName,
    const AppSpawnStartMsg &startMsg)
    : appPid_(appPid), uid_(uid), bundleName_(bundleName), startTime_(std::chrono::system_clock::now()),
      startMsg_(startMsg) {}

pid_t SubSpawnRecord::GetSubSpawnPid() const { return subSpawnPid_; }
void SubSpawnRecord::SetSubSpawnPid(pid_t pid) { subSpawnPid_ = pid; }
pid_t SubSpawnRecord::GetAppPid() const { return appPid_; }
int32_t SubSpawnRecord::GetUid() const { return uid_; }
std::string SubSpawnRecord::GetBundleName() const { return bundleName_; }

SubSpawnState SubSpawnRecord::GetState() const { return state_.load(); }
void SubSpawnRecord::SetState(SubSpawnState state) { state_.store(state); }
bool SubSpawnRecord::IsReady() const { return state_.load() == SubSpawnState::READY; }
bool SubSpawnRecord::IsCreating() const { return state_.load() == SubSpawnState::CREATING; }
bool SubSpawnRecord::IsTerminating() const { return state_.load() == SubSpawnState::TERMINATING; }

std::string SubSpawnRecord::GetSocketName() const { return socketName_; }
void SubSpawnRecord::SetSocketName(const std::string &name) { socketName_ = name; }

void SubSpawnRecord::AddChildPid(pid_t pid) { childPids_.insert(pid); }
void SubSpawnRecord::RemoveChildPid(pid_t pid) { childPids_.erase(pid); }
size_t SubSpawnRecord::GetChildProcessCount() const { return childPids_.size(); }
bool SubSpawnRecord::HasChildProcesses() const { return !childPids_.empty(); }

bool SubSpawnRecord::IsIdleTimerActive() const { return idleTimerActive_; }
void SubSpawnRecord::SetIdleTimerActive(bool active) { idleTimerActive_ = active; }

std::chrono::system_clock::time_point SubSpawnRecord::GetStartTime() const { return startTime_; }
const AppSpawnStartMsg &SubSpawnRecord::GetStartMsg() const { return startMsg_; }

void SubSpawnRecord::SetKillReason(const std::string &reason) { killReason_ = reason; }
std::string SubSpawnRecord::GetKillReason() const { return killReason_; }
void SubSpawnRecord::SetExitSignal(int32_t signal) { exitSignal_ = signal; }
int32_t SubSpawnRecord::GetExitSignal() const { return exitSignal_; }

} // namespace AppExecFwk
} // namespace OHOS
