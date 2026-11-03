/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "app_running_record.h"
#include "render_record.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
RenderRecord::RenderRecord(pid_t hostPid, const std::string &renderParam,
                           FdGuard &&ipcFd, FdGuard &&sharedFd, FdGuard &&crashFd,
                           const std::shared_ptr<AppRunningRecord> &host)
    : hostPid_(hostPid),  ipcFd_(std::move(ipcFd)), sharedFd_(std::move(sharedFd)),
    crashFd_(std::move(crashFd)), host_(host), renderParam_(renderParam) {}

RenderRecord::~RenderRecord()
{}

std::shared_ptr<RenderRecord> RenderRecord::CreateRenderRecord(
    pid_t hostPid, const std::string &renderParam,
    FdGuard &&ipcFd, FdGuard &&sharedFd, FdGuard &&crashFd,
    const std::shared_ptr<AppRunningRecord> &host)
{
    if (hostPid <= 0 || renderParam.empty() || ipcFd.Get() <= 0 || sharedFd.Get() <= 0 ||
        crashFd.Get() <= 0 || !host) {
        return nullptr;
    }

    auto renderRecord = std::make_shared<RenderRecord>(
        hostPid, renderParam, std::move(ipcFd), std::move(sharedFd), std::move(crashFd), host);
    renderRecord->SetHostUid(host->GetUid());
    renderRecord->SetHostBundleName(host->GetBundleName());
    return renderRecord;
}

void RenderRecord::SetPid(pid_t pid)
{
    pid_ = pid;
}

pid_t RenderRecord::GetPid() const
{
    return pid_;
}

pid_t RenderRecord::GetHostPid() const
{
    return hostPid_;
}

void RenderRecord::SetUid(int32_t uid)
{
    uid_ = uid;
}

int32_t RenderRecord::GetUid() const
{
    return uid_;
}

void RenderRecord::SetHostUid(const int32_t hostUid)
{
    hostUid_ = hostUid;
}

int32_t RenderRecord::GetHostUid() const
{
    return hostUid_;
}

void RenderRecord::SetHostBundleName(const std::string &hostBundleName)
{
    hostBundleName_ = hostBundleName;
}

std::string RenderRecord::GetHostBundleName() const
{
    return hostBundleName_;
}

void RenderRecord::SetProcessName(const std::string &processName)
{
    processName_ = processName;
}

std::string RenderRecord::GetProcessName() const
{
    return processName_;
}

std::string RenderRecord::GetRenderParam() const
{
    return renderParam_;
}

int32_t RenderRecord::GetIpcFd() const
{
    return ipcFd_.Get();
}

int32_t RenderRecord::GetSharedFd() const
{
    return sharedFd_.Get();
}

int32_t RenderRecord::GetCrashFd() const
{
    return crashFd_.Get();
}

ProcessType RenderRecord::GetProcessType() const
{
    return processType_;
}

std::shared_ptr<AppRunningRecord> RenderRecord::GetHostRecord() const
{
    return host_.lock();
}

sptr<IRenderScheduler> RenderRecord::GetScheduler() const
{
    return renderScheduler_;
}

void RenderRecord::SetScheduler(const sptr<IRenderScheduler> &scheduler)
{
    renderScheduler_ = scheduler;
}

void RenderRecord::SetDeathRecipient(const sptr<AppDeathRecipient> recipient)
{
    deathRecipient_ = recipient;
}

void RenderRecord::RegisterDeathRecipient()
{
    if (renderScheduler_ && deathRecipient_) {
        auto obj = renderScheduler_->AsObject();
        if (!obj || !obj->AddDeathRecipient(deathRecipient_)) {
            TAG_LOGE(AAFwkTag::APPMGR, "AddDeathRecipient failed");
        }
    }
}

void RenderRecord::SetProcessType(ProcessType type)
{
    processType_ = type;
}

void RenderRecord::SetState(int32_t state)
{
    state_ = state;
}

int32_t RenderRecord::GetState() const
{
    return state_;
}

void RenderRecord::SetAttachTimeoutStartTime(const std::chrono::system_clock::time_point &time)
{
    attachTimeoutStartTime_ = time;
}

std::chrono::system_clock::time_point RenderRecord::GetAttachTimeoutStartTime() const
{
    return attachTimeoutStartTime_;
}
}  // namespace AppExecFwk
}  // namespace OHOS
