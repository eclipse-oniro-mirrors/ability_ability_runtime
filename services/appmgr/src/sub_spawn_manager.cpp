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

#include "sub_spawn_manager.h"

#include <csignal>
#include <unistd.h>

#include "app_pidfd_manager.h"
#include "app_state_observer_manager.h"
#include "ffrt_inner.h"
#include "hilog_tag_wrapper.h"
#include "singleton.h"
#include "task_handler_wrap.h"
#include "appspawn.h"

namespace OHOS {
namespace AppExecFwk {

extern "C" {
char *GetSubSpawnSocketName(int uid);
int AppSpawnClientInitSubspawn(const char *socketName, AppSpawnClientHandle *handle);
bool IsSupportSubSpawn(void);
}

namespace {
constexpr const char* SUBSPAWN_PROCESS_NAME_SUFFIX = ":subspawn";
constexpr const char* IDLE_TIMER_PREFIX = "SubSpawnIdle_";
}

SubSpawnManager::SubSpawnManager() {}

SubSpawnManager::~SubSpawnManager() {}

SubSpawnManager &SubSpawnManager::GetInstance()
{
    static SubSpawnManager instance;
    return instance;
}

bool SubSpawnManager::ShouldUseSubSpawn(const std::vector<std::string> &preloadSoList, bool isolationMode)
{
    if (!IsSupportSubSpawn()) {
        TAG_LOGD(AAFwkTag::APPMGR, "ShouldUseSubSpawn: device not support");
        return false;
    }
    if (preloadSoList.empty()) {
        TAG_LOGD(AAFwkTag::APPMGR, "ShouldUseSubSpawn: preloadSoList empty");
        return false;
    }
    if (!isolationMode) {
        TAG_LOGD(AAFwkTag::APPMGR, "ShouldUseSubSpawn: not isolated mode");
        return false;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "ShouldUseSubSpawn: true, soList size:%{public}zu", preloadSoList.size());
    return true;
}

std::string SubSpawnManager::QuerySocketNameByUid(int32_t uid)
{
    TAG_LOGD(AAFwkTag::APPMGR, "QuerySocketNameByUid uid:%{public}d", uid);
    char *socketName = GetSubSpawnSocketName(uid);
    if (socketName == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetSubSpawnSocketName returned null for uid:%{public}d", uid);
        return "";
    }
    std::string result(socketName);
    free(socketName);
    TAG_LOGD(AAFwkTag::APPMGR, "socketName: %{public}s", result.c_str());
    return result;
}

pid_t SubSpawnManager::DoCreateSubSpawn(int32_t uid,
    const std::string &bundleName, const AppSpawnStartMsg &startMsg)
{
    auto startTime = std::chrono::system_clock::now();
    TAG_LOGI(AAFwkTag::APPMGR, "DoCreateSubSpawn uid:%{public}d, bundle:%{public}s",
        uid, bundleName.c_str());

    AppSpawnClientHandle handle = nullptr;
    ErrCode ret = AppSpawnClientInit(APPSPAWN_SERVER_NAME, &handle);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppSpawnClientInit fail, errCode:%{public}d, ret:%{public}d",
            static_cast<int32_t>(SubSpawnErrorCode::ERR_SOCKET_CONNECT_FAILED), ret);
        return -1;
    }

    AppSpawnReqMsgHandle reqHandle = nullptr;
    ret = AppSpawnReqMsgCreate(MSG_SUBSPAWN_CREATE, bundleName.c_str(), &reqHandle);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppSpawnReqMsgCreate fail, errCode:%{public}d, ret:%{public}d",
            static_cast<int32_t>(SubSpawnErrorCode::ERR_MSG_CREATE_FAILED), ret);
        return -1;
    }

    AppSpawnStartMsg subSpawnMsg = startMsg;
    subSpawnMsg.fds.clear();
    subSpawnMsg.uid = uid;
    subSpawnMsg.gid = uid;
#ifdef SUPPORT_CHILD_PROCESS
    subSpawnMsg.childProcessType = CHILD_PROCESS_TYPE_NOT_CHILD;
#endif
    AppSpawnClient spawnClient(APPSPAWN_SERVER_NAME);
    ret = spawnClient.AppspawnCreateDefaultMsg(subSpawnMsg, reqHandle);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppspawnCreateDefaultMsg fail, errCode:%{public}d, ret:%{public}d",
            static_cast<int32_t>(SubSpawnErrorCode::ERR_MSG_FILL_FAILED), ret);
        return -1;
    }

    AppSpawnResult result = {0};
    TAG_LOGD(AAFwkTag::APPMGR, "DoCreateSubSpawn: sending MSG_SUBSPAWN_CREATE to appspawn");
    ret = AppSpawnClientSendMsg(handle, reqHandle, &result);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppSpawnClientSendMsg fail, errCode:%{public}d, ret:%{public}d",
            static_cast<int32_t>(SubSpawnErrorCode::ERR_SEND_FAILED), ret);
        return -1;
    }

    if (result.pid <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "subspawn create fail, errCode:%{public}d, result:%{public}d",
            static_cast<int32_t>(SubSpawnErrorCode::ERR_CREATE_FAILED), result.result);
        return -1;
    }

    auto costMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now() - startTime).count();
    TAG_LOGI(AAFwkTag::APPMGR, "subspawn created, pid:%{public}d, cost:%{public}lld ms",
        result.pid, costMs);
    return result.pid;
}

void SubSpawnManager::AsyncCreateSubSpawn(int32_t uid,
    const std::string &bundleName, const AppSpawnStartMsg &startMsg)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = subSpawnMap_.find(uid);
        if (it != subSpawnMap_.end() && (it->second->IsReady() || it->second->IsCreating())) {
            TAG_LOGD(AAFwkTag::APPMGR, "subspawn already ready or creating, uid:%{public}d", uid);
            return;
        }
        auto record = SubSpawnRecord::CreateSubSpawnRecord(uid, bundleName, startMsg);
        if (!record) {
            TAG_LOGE(AAFwkTag::APPMGR, "Failed to create SubSpawnRecord for uid:%{public}d", uid);
            return;
        }
        subSpawnMap_[uid] = record;
    }

    auto startMsgCopy = std::make_shared<AppSpawnStartMsg>(startMsg);
    std::string bundleNameCopy = bundleName;
    ffrt::submit([=]() {
        pid_t subSpawnPid = GetInstance().DoCreateSubSpawn(uid, bundleNameCopy, *startMsgCopy);
        if (subSpawnPid <= 0) {
            TAG_LOGE(AAFwkTag::APPMGR, "async create subspawn failed, errCode:%{public}d, uid:%{public}d",
                static_cast<int32_t>(SubSpawnErrorCode::ERR_CREATE_FAILED), uid);
            std::lock_guard<std::mutex> lock(GetInstance().mutex_);
            auto it = GetInstance().subSpawnMap_.find(uid);
            if (it != GetInstance().subSpawnMap_.end() && it->second->IsCreating()) {
                it->second->SetKillReason("create_failed");
                GetInstance().subSpawnMap_.erase(it);
            }
            return;
        }

        std::string socketName = GetInstance().QuerySocketNameByUid(uid);
        if (socketName.empty()) {
            TAG_LOGE(AAFwkTag::APPMGR, "socketName empty after create, killing subspawn pid:%{public}d",
                subSpawnPid);
            AppPidFdManager::GetInstance().KillWatchedProcess(subSpawnPid);
            std::lock_guard<std::mutex> lock(GetInstance().mutex_);
            auto it = GetInstance().subSpawnMap_.find(uid);
            if (it != GetInstance().subSpawnMap_.end()) {
                it->second->SetKillReason("socket_name_empty");
                GetInstance().subSpawnMap_.erase(it);
            }
            return;
        }

        {
            std::lock_guard<std::mutex> lock(GetInstance().mutex_);
            auto it = GetInstance().subSpawnMap_.find(uid);
            if (it == GetInstance().subSpawnMap_.end() || it->second->IsTerminating()) {
                TAG_LOGW(AAFwkTag::APPMGR, "record removed or terminating during async create, pid:%{public}d",
                    subSpawnPid);
                AppPidFdManager::GetInstance().KillWatchedProcess(subSpawnPid);
                if (it != GetInstance().subSpawnMap_.end()) {
                    it->second->SetKillReason("terminated_during_create");
                    GetInstance().pidToRecordMap_.erase(subSpawnPid);
                    GetInstance().subSpawnMap_.erase(it);
                }
                return;
            }
            auto &record = it->second;
            record->SetSubSpawnPid(subSpawnPid);
            record->SetSocketName(socketName);
            record->SetState(SubSpawnState::READY);
            GetInstance().pidToRecordMap_[subSpawnPid] = record;
        }

        if (!AppPidFdManager::GetInstance().AddWatcher(subSpawnPid, PidFdType::SUBSPAWN)) {
            TAG_LOGE(AAFwkTag::APPMGR, "pidfd register failed, killing subspawn pid:%{public}d", subSpawnPid);
            AppPidFdManager::GetInstance().KillWatchedProcess(subSpawnPid);
            std::lock_guard<std::mutex> lock(GetInstance().mutex_);
            auto it = GetInstance().subSpawnMap_.find(uid);
            if (it != GetInstance().subSpawnMap_.end()) {
                it->second->SetKillReason("pidfd_failed");
                GetInstance().pidToRecordMap_.erase(subSpawnPid);
                GetInstance().subSpawnMap_.erase(it);
            }
            return;
        }

        {
            std::lock_guard<std::mutex> lock(GetInstance().mutex_);
            auto it = GetInstance().subSpawnMap_.find(uid);
            if (it == GetInstance().subSpawnMap_.end()) {
                TAG_LOGW(AAFwkTag::APPMGR, "record removed before created notify, skip notify, pid:%{public}d",
                    subSpawnPid);
                return;
            }
            it->second->SetCreatedNotified(true);
        }
        DelayedSingleton<AppStateObserverManager>::GetInstance()->OnSubSpawnProcessCreated(
            subSpawnPid, bundleNameCopy, uid, bundleNameCopy + SUBSPAWN_PROCESS_NAME_SUFFIX);

        TAG_LOGI(AAFwkTag::APPMGR, "subspawn ready, pid:%{public}d, uid:%{public}d",
            subSpawnPid, uid);
    });
}

pid_t SubSpawnManager::StartChildViaSubSpawn(int32_t uid, const AppSpawnStartMsg &startMsg)
{
    std::string socketName;
    bool needAsyncCreate = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = subSpawnMap_.find(uid);
        if (it == subSpawnMap_.end()) {
            needAsyncCreate = true;
        } else if (it->second->IsCreating()) {
            TAG_LOGD(AAFwkTag::APPMGR, "subspawn creating, errCode:%{public}d, uid:%{public}d",
                static_cast<int32_t>(SubSpawnErrorCode::ERR_CREATING), uid);
            return -1;
        } else if (it->second->IsTerminating()) {
            TAG_LOGD(AAFwkTag::APPMGR, "subspawn terminating, errCode:%{public}d, uid:%{public}d",
                static_cast<int32_t>(SubSpawnErrorCode::ERR_TERMINATING), uid);
            return -1;
        } else if (it->second->GetSocketName().empty()) {
            TAG_LOGE(AAFwkTag::APPMGR, "subspawn socketName empty, errCode:%{public}d, uid:%{public}d",
                static_cast<int32_t>(SubSpawnErrorCode::ERR_SOCKET_NAME_EMPTY), uid);
            return -1;
        } else {
            socketName = it->second->GetSocketName();
        }
    }
    if (needAsyncCreate) {
        AsyncCreateSubSpawn(uid, startMsg.bundleName, startMsg);
        return -1;
    }

    auto forkStartTime = std::chrono::system_clock::now();
    AppSpawnClientHandle handle = nullptr;
    TAG_LOGD(AAFwkTag::APPMGR, "StartChildViaSubSpawn: connecting to subspawn socket: %{public}s",
        socketName.c_str());
    int ret = AppSpawnClientInitSubspawn(socketName.c_str(), &handle);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppSpawnClientInitSubspawn fail, errCode:%{public}d, ret:%{public}d",
            static_cast<int32_t>(SubSpawnErrorCode::ERR_SOCKET_CONNECT_FAILED), ret);
        return -1;
    }

    AppSpawnReqMsgHandle reqHandle = nullptr;
    ret = AppSpawnReqMsgCreate(MSG_APP_SPAWN, startMsg.procName.c_str(), &reqHandle);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppSpawnReqMsgCreate fail, errCode:%{public}d, ret:%{public}d",
            static_cast<int32_t>(SubSpawnErrorCode::ERR_MSG_CREATE_FAILED), ret);
        return -1;
    }

    AppSpawnStartMsg childMsg = startMsg;
    childMsg.preloadSoList.clear();
    childMsg.appLibPath.clear();
    AppSpawnClient spawnClient(APPSPAWN_SERVER_NAME);
    ret = spawnClient.AppspawnCreateDefaultMsg(childMsg, reqHandle);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppspawnCreateDefaultMsg fail, errCode:%{public}d, ret:%{public}d",
            static_cast<int32_t>(SubSpawnErrorCode::ERR_MSG_FILL_FAILED), ret);
        return -1;
    }

    AppSpawnResult result = {0};
    TAG_LOGD(AAFwkTag::APPMGR, "StartChildViaSubSpawn: sending MSG_APP_SPAWN to subspawn");
    ret = AppSpawnClientSendMsg(handle, reqHandle, &result);
    if (ret != 0 || result.pid <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "subspawn fork child fail, errCode:%{public}d, ret:%{public}d, pid:%{public}d",
            static_cast<int32_t>(SubSpawnErrorCode::ERR_FORK_FAILED), ret, result.pid);
        return -1;
    }

    auto costMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now() - forkStartTime).count();
    TAG_LOGI(AAFwkTag::APPMGR, "child created via subspawn, pid:%{public}d, uid:%{public}d, cost:%{public}lld ms",
        result.pid, uid, costMs);
    return result.pid;
}

void SubSpawnManager::OnSubSpawnExited(pid_t subSpawnPid)
{
    std::shared_ptr<SubSpawnRecord> record;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto pidIt = pidToRecordMap_.find(subSpawnPid);
        if (pidIt == pidToRecordMap_.end()) {
            TAG_LOGW(AAFwkTag::APPMGR, "unknown subspawn pid:%{public}d", subSpawnPid);
            return;
        }
        record = pidIt->second;
        pidToRecordMap_.erase(pidIt);
        subSpawnMap_.erase(record->GetUid());
    }
    if (record->IsCreatedNotified()) {
        DelayedSingleton<AppStateObserverManager>::GetInstance()->OnSubSpawnProcessDied(
            subSpawnPid, record->GetBundleName(), record->GetUid(),
            record->GetBundleName() + SUBSPAWN_PROCESS_NAME_SUFFIX);
    }
    TAG_LOGI(AAFwkTag::APPMGR, "subspawn exited, pid:%{public}d, uid:%{public}d, killReason:%{public}s",
        subSpawnPid, record->GetUid(), record->GetKillReason().c_str());
}

void SubSpawnManager::OnChildProcessCreated(int32_t uid, pid_t childPid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = subSpawnMap_.find(uid);
    if (it != subSpawnMap_.end() && it->second->IsReady()) {
        it->second->AddChildPid(childPid);
        CancelIdleTimer(it->second);
        TAG_LOGD(AAFwkTag::APPMGR, "OnChildProcessCreated: uid:%{public}d, childPid:%{public}d, count:%{public}zu",
            uid, childPid, it->second->GetChildProcessCount());
    }
}

void SubSpawnManager::OnChildProcessExited(int32_t uid, pid_t childPid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = subSpawnMap_.find(uid);
    if (it != subSpawnMap_.end() && it->second->IsReady()) {
        it->second->RemoveChildPid(childPid);
        TAG_LOGD(AAFwkTag::APPMGR, "OnChildProcessExited: uid:%{public}d, childPid:%{public}d, count:%{public}zu",
            uid, childPid, it->second->GetChildProcessCount());
        if (!it->second->HasChildProcesses()) {
            StartIdleTimer(it->second);
            TAG_LOGD(AAFwkTag::APPMGR, "OnChildProcessExited: start idle timer, uid:%{public}d", uid);
        }
    }
}

void SubSpawnManager::OnAppProcessDied(int32_t uid)
{
    std::shared_ptr<SubSpawnRecord> record;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = subSpawnMap_.find(uid);
        if (it == subSpawnMap_.end()) {
            return;
        }
        record = it->second;
        if (record->IsTerminating()) {
            TAG_LOGD(AAFwkTag::APPMGR, "subspawn already terminating, uid:%{public}d", uid);
            return;
        }
        record->SetState(SubSpawnState::TERMINATING);
        record->SetKillReason("app_died");
        pidToRecordMap_.erase(record->GetSubSpawnPid());
        subSpawnMap_.erase(it);
    }

    CancelIdleTimer(record);

    pid_t subSpawnPid = record->GetSubSpawnPid();
    if (subSpawnPid > 0) {
        AppPidFdManager::GetInstance().KillWatchedProcess(subSpawnPid);
        TAG_LOGI(AAFwkTag::APPMGR, "killed subspawn pid:%{public}d for uid:%{public}d",
            subSpawnPid, uid);
    }

    if (record->IsCreatedNotified()) {
        DelayedSingleton<AppStateObserverManager>::GetInstance()->OnSubSpawnProcessDied(
            subSpawnPid, record->GetBundleName(), record->GetUid(),
            record->GetBundleName() + SUBSPAWN_PROCESS_NAME_SUFFIX);
    }
    TAG_LOGI(AAFwkTag::APPMGR, "subspawn cleanup done for uid:%{public}d, killReason:%{public}s",
        uid, record->GetKillReason().c_str());
}

bool SubSpawnManager::IsSubSpawn(pid_t pid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    return pidToRecordMap_.find(pid) != pidToRecordMap_.end();
}

void SubSpawnManager::StartIdleTimer(const std::shared_ptr<SubSpawnRecord> &record)
{
    if (!record || record->IsIdleTimerActive()) {
        return;
    }
    record->SetIdleTimerActive(true);
    int32_t uid = record->GetUid();
    std::string taskName = std::string(IDLE_TIMER_PREFIX) + std::to_string(uid);
    AAFwk::TaskHandlerWrap::GetFfrtHandler()->SubmitTask([uid]() {
        GetInstance().HandleIdleTimeout(uid);
    }, taskName, IDLE_TIMEOUT_MS);
}

void SubSpawnManager::CancelIdleTimer(const std::shared_ptr<SubSpawnRecord> &record)
{
    if (record) {
        record->SetIdleTimerActive(false);
        std::string taskName = std::string(IDLE_TIMER_PREFIX) + std::to_string(record->GetUid());
        AAFwk::TaskHandlerWrap::GetFfrtHandler()->CancelTask(taskName);
    }
}

void SubSpawnManager::HandleIdleTimeout(int32_t uid)
{
    std::shared_ptr<SubSpawnRecord> record;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = subSpawnMap_.find(uid);
        if (it == subSpawnMap_.end()) {
            TAG_LOGD(AAFwkTag::APPMGR, "HandleIdleTimeout: no record, uid:%{public}d", uid);
            return;
        }
        record = it->second;
        if (record->HasChildProcesses() || !record->IsIdleTimerActive()) {
            TAG_LOGD(AAFwkTag::APPMGR, "HandleIdleTimeout: skip, count:%{public}zu, active:%{public}d",
                record->GetChildProcessCount(), record->IsIdleTimerActive());
            return;
        }
        if (record->IsTerminating()) {
            TAG_LOGD(AAFwkTag::APPMGR, "HandleIdleTimeout: already terminating, uid:%{public}d", uid);
            return;
        }
        record->SetState(SubSpawnState::TERMINATING);
        record->SetKillReason("idle_timeout");
        record->SetIdleTimerActive(false);
        pidToRecordMap_.erase(record->GetSubSpawnPid());
        subSpawnMap_.erase(it);
    }

    pid_t subSpawnPid = record->GetSubSpawnPid();
    if (subSpawnPid > 0) {
        AppPidFdManager::GetInstance().KillWatchedProcess(subSpawnPid);
        TAG_LOGI(AAFwkTag::APPMGR, "idle timeout, killed subspawn pid:%{public}d for uid:%{public}d",
            subSpawnPid, uid);
    }

    if (record->IsCreatedNotified()) {
        DelayedSingleton<AppStateObserverManager>::GetInstance()->OnSubSpawnProcessDied(
            subSpawnPid, record->GetBundleName(), record->GetUid(),
            record->GetBundleName() + SUBSPAWN_PROCESS_NAME_SUFFIX);
    }
    TAG_LOGI(AAFwkTag::APPMGR, "subspawn idle timeout cleanup done, uid:%{public}d", uid);
}

} // namespace AppExecFwk
} // namespace OHOS
