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
#include "app_utils.h"
#include "ffrt_inner.h"
#include "hilog_tag_wrapper.h"
#include "singleton.h"
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
}

std::shared_ptr<SubSpawnRecord> SubSpawnRecord::CreateSubSpawnRecord(pid_t appPid, int32_t uid,
    const std::string &bundleName)
{
    if (appPid <= 0 || bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Invalid appPid or bundleName");
        return nullptr;
    }
    return std::shared_ptr<SubSpawnRecord>(new (std::nothrow) SubSpawnRecord(appPid, uid, bundleName));
}

SubSpawnRecord::SubSpawnRecord(pid_t appPid, int32_t uid, const std::string &bundleName)
    : appPid_(appPid), uid_(uid), bundleName_(bundleName) {}

pid_t SubSpawnRecord::GetSubSpawnPid() const { return subSpawnPid_; }
void SubSpawnRecord::SetSubSpawnPid(pid_t pid) { subSpawnPid_ = pid; }
pid_t SubSpawnRecord::GetAppPid() const { return appPid_; }
int32_t SubSpawnRecord::GetUid() const { return uid_; }
std::string SubSpawnRecord::GetBundleName() const { return bundleName_; }
std::string SubSpawnRecord::GetSocketName() const { return socketName_; }
void SubSpawnRecord::SetSocketName(const std::string &name) { socketName_ = name; }
int32_t SubSpawnRecord::GetChildProcessCount() const { return childProcessCount_; }
void SubSpawnRecord::IncrementChildProcessCount() { childProcessCount_++; }
void SubSpawnRecord::DecrementChildProcessCount()
{
    childProcessCount_--;
    if (childProcessCount_ < 0) {
        childProcessCount_ = 0;
    }
}
bool SubSpawnRecord::IsIdleTimerActive() const { return idleTimerActive_; }
void SubSpawnRecord::SetIdleTimerActive(bool active) { idleTimerActive_ = active; }
bool SubSpawnRecord::IsTerminating() const { return terminating_; }
void SubSpawnRecord::SetTerminating(bool terminating) { terminating_ = terminating; }
bool SubSpawnRecord::IsCreating() const { return creating_; }
void SubSpawnRecord::SetCreating(bool creating) { creating_ = creating; }
bool SubSpawnRecord::IsReady() const { return subSpawnPid_ > 0 && !creating_ && !terminating_; }

SubSpawnManager::SubSpawnManager() {}

SubSpawnManager::~SubSpawnManager() {}

SubSpawnManager &SubSpawnManager::GetInstance()
{
    static SubSpawnManager instance;
    return instance;
}

bool SubSpawnManager::ShouldUseSubSpawn(const std::vector<std::string> &preloadSoList, bool isolationMode)
{
    if (!AppUtils::GetInstance().IsSupportSubSpawn()) {
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

void SubSpawnManager::SendSubSpawnShutdown(const std::string &bundleName, pid_t subSpawnPid)
{
    AppSpawnClientHandle handle = nullptr;
    ErrCode ret = AppSpawnClientInit(APPSPAWN_SERVER_NAME, &handle);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "SendSubSpawnShutdown: init fail, ret:%{public}d", ret);
        return;
    }
    AppSpawnReqMsgHandle reqHandle = nullptr;
    ret = AppSpawnReqMsgCreate(MSG_SUBSPAWN_SHUTDOWN, bundleName.c_str(), &reqHandle);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "SendSubSpawnShutdown: create msg fail, ret:%{public}d", ret);
        AppSpawnClientDestroy(handle);
        return;
    }
    ret = AppSpawnReqMsgAddExtInfo(reqHandle, "SubspawnPid",
        reinterpret_cast<const uint8_t *>(&subSpawnPid), sizeof(pid_t));
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "SendSubSpawnShutdown: add SubspawnPid fail, ret:%{public}d", ret);
    }
    AppSpawnResult result = {0};
    TAG_LOGD(AAFwkTag::APPMGR, "SendSubSpawnShutdown: sending MSG_SUBSPAWN_SHUTDOWN for bundle:%{public}s pid:%{public}d",
        bundleName.c_str(), subSpawnPid);
    AppSpawnClientSendMsg(handle, reqHandle, &result);
    AppSpawnClientDestroy(handle);
    TAG_LOGD(AAFwkTag::APPMGR, "SendSubSpawnShutdown: sent for bundle:%{public}s", bundleName.c_str());
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

pid_t SubSpawnManager::DoCreateSubSpawn(pid_t appPid, int32_t uid,
    const std::string &bundleName, const AppSpawnStartMsg &startMsg)
{
    TAG_LOGI(AAFwkTag::APPMGR, "DoCreateSubSpawn appPid:%{public}d, uid:%{public}d, bundle:%{public}s",
        appPid, uid, bundleName.c_str());

    AppSpawnClientHandle handle = nullptr;
    ErrCode ret = AppSpawnClientInit(APPSPAWN_SERVER_NAME, &handle);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppSpawnClientInit fail, ret:%{public}d", ret);
        return -1;
    }

    AppSpawnReqMsgHandle reqHandle = nullptr;
    ret = AppSpawnReqMsgCreate(MSG_SUBSPAWN_CREATE, bundleName.c_str(), &reqHandle);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppSpawnReqMsgCreate fail, ret:%{public}d", ret);
        AppSpawnClientDestroy(handle);
        return -1;
    }

    AppSpawnStartMsg subSpawnMsg = startMsg;
    subSpawnMsg.fds.clear();
    subSpawnMsg.procName = bundleName;
    AppSpawnClient spawnClient(APPSPAWN_SERVER_NAME);
    ret = spawnClient.AppspawnCreateDefaultMsg(subSpawnMsg, reqHandle);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppspawnCreateDefaultMsg fail, ret:%{public}d", ret);
        AppSpawnClientDestroy(handle);
        return -1;
    }

    AppSpawnResult result = {0};
    TAG_LOGD(AAFwkTag::APPMGR, "DoCreateSubSpawn: sending MSG_SUBSPAWN_CREATE to appspawn");
    ret = AppSpawnClientSendMsg(handle, reqHandle, &result);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppSpawnClientSendMsg fail, ret:%{public}d", ret);
        AppSpawnClientDestroy(handle);
        return -1;
    }

    AppSpawnClientDestroy(handle);
    if (result.pid <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "subspawn create fail, result:%{public}d", result.result);
        return -1;
    }

    TAG_LOGI(AAFwkTag::APPMGR, "subspawn created, pid:%{public}d", result.pid);
    return result.pid;
}

void SubSpawnManager::AsyncCreateSubSpawn(pid_t appPid, int32_t uid,
    const std::string &bundleName, const AppSpawnStartMsg &startMsg)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = subSpawnMap_.find(appPid);
        if (it != subSpawnMap_.end() && (it->second->IsReady() || it->second->IsCreating())) {
            TAG_LOGD(AAFwkTag::APPMGR, "subspawn already ready or creating, appPid:%{public}d", appPid);
            return;
        }
        auto record = SubSpawnRecord::CreateSubSpawnRecord(appPid, uid, bundleName);
        if (!record) {
            TAG_LOGE(AAFwkTag::APPMGR, "Failed to create SubSpawnRecord for appPid:%{public}d", appPid);
            return;
        }
        record->SetCreating(true);
        subSpawnMap_[appPid] = record;
    }

    auto startMsgCopy = std::make_shared<AppSpawnStartMsg>(startMsg);
    std::string bundleNameCopy = bundleName;
    ffrt::submit([=]() {
        pid_t subSpawnPid = GetInstance().DoCreateSubSpawn(appPid, uid, bundleNameCopy, *startMsgCopy);
        if (subSpawnPid <= 0) {
            TAG_LOGE(AAFwkTag::APPMGR, "async create subspawn failed for appPid:%{public}d", appPid);
            std::lock_guard<std::mutex> lock(GetInstance().mutex_);
            auto it = GetInstance().subSpawnMap_.find(appPid);
            if (it != GetInstance().subSpawnMap_.end() && it->second->IsCreating()) {
                GetInstance().subSpawnMap_.erase(it);
            }
            return;
        }

        std::string socketName = GetInstance().QuerySocketNameByUid(uid);

        {
            std::lock_guard<std::mutex> lock(GetInstance().mutex_);
            auto it = GetInstance().subSpawnMap_.find(appPid);
            if (it == GetInstance().subSpawnMap_.end() || it->second->IsTerminating()) {
                TAG_LOGW(AAFwkTag::APPMGR, "record removed or terminating during async create, pid:%{public}d",
                    subSpawnPid);
                kill(subSpawnPid, SIGKILL);
                GetInstance().SendSubSpawnShutdown(bundleNameCopy, subSpawnPid);
                return;
            }
            auto &record = it->second;
            record->SetSubSpawnPid(subSpawnPid);
            record->SetSocketName(socketName);
            record->SetCreating(false);
            GetInstance().pidToRecordMap_[subSpawnPid] = record;
        }

        AppPidFdManager::GetInstance().AddWatcher(subSpawnPid, PidFdType::CHILD);
        DelayedSingleton<AppStateObserverManager>::GetInstance()->OnSubSpawnProcessCreated(
            subSpawnPid, bundleNameCopy, uid, bundleNameCopy + SUBSPAWN_PROCESS_NAME_SUFFIX);

        TAG_LOGI(AAFwkTag::APPMGR, "subspawn ready, pid:%{public}d, appPid:%{public}d",
            subSpawnPid, appPid);
    });
}

pid_t SubSpawnManager::StartChildViaSubSpawn(pid_t appPid, int32_t uid, const AppSpawnStartMsg &startMsg)
{
    std::string socketName;
    bool needAsyncCreate = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = subSpawnMap_.find(appPid);
        if (it == subSpawnMap_.end()) {
            needAsyncCreate = true;
        } else if (it->second->IsCreating()) {
            TAG_LOGD(AAFwkTag::APPMGR, "subspawn creating, appPid:%{public}d", appPid);
            return -1;
        } else if (it->second->IsTerminating()) {
            TAG_LOGD(AAFwkTag::APPMGR, "subspawn terminating, appPid:%{public}d", appPid);
            return -1;
        } else if (it->second->GetSocketName().empty()) {
            TAG_LOGE(AAFwkTag::APPMGR, "subspawn socketName empty, appPid:%{public}d", appPid);
            return -1;
        } else {
            socketName = it->second->GetSocketName();
        }
    }
    if (needAsyncCreate) {
        AsyncCreateSubSpawn(appPid, uid, startMsg.bundleName, startMsg);
        return -1;
    }

    AppSpawnClientHandle handle = nullptr;
    TAG_LOGD(AAFwkTag::APPMGR, "StartChildViaSubSpawn: connecting to subspawn socket: %{public}s",
        socketName.c_str());
    int ret = AppSpawnClientInitSubspawn(socketName.c_str(), &handle);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppSpawnClientInitSubspawn fail, ret:%{public}d", ret);
        return -1;
    }

    AppSpawnReqMsgHandle reqHandle = nullptr;
    ret = AppSpawnReqMsgCreate(MSG_APP_SPAWN, startMsg.procName.c_str(), &reqHandle);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppSpawnReqMsgCreate fail, ret:%{public}d", ret);
        AppSpawnClientDestroy(handle);
        return -1;
    }

    AppSpawnStartMsg childMsg = startMsg;
    childMsg.preloadSoList.clear();
    childMsg.appLibPath.clear();
    AppSpawnClient spawnClient(APPSPAWN_SERVER_NAME);
    ret = spawnClient.AppspawnCreateDefaultMsg(childMsg, reqHandle);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppspawnCreateDefaultMsg fail, ret:%{public}d", ret);
        AppSpawnClientDestroy(handle);
        return -1;
    }

    AppSpawnResult result = {0};
    TAG_LOGD(AAFwkTag::APPMGR, "StartChildViaSubSpawn: sending MSG_APP_SPAWN to subspawn");
    ret = AppSpawnClientSendMsg(handle, reqHandle, &result);
    AppSpawnClientDestroy(handle);
    if (ret != 0 || result.pid <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "subspawn fork child fail, ret:%{public}d, pid:%{public}d",
            ret, result.pid);
        return -1;
    }

    TAG_LOGI(AAFwkTag::APPMGR, "child created via subspawn, pid:%{public}d, appPid:%{public}d",
        result.pid, appPid);
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
        subSpawnMap_.erase(record->GetAppPid());
    }
    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnSubSpawnProcessDied(
        subSpawnPid, record->GetBundleName(), record->GetUid(),
        record->GetBundleName() + SUBSPAWN_PROCESS_NAME_SUFFIX);
    TAG_LOGI(AAFwkTag::APPMGR, "subspawn exited, pid:%{public}d, appPid:%{public}d",
        subSpawnPid, record->GetAppPid());
}

void SubSpawnManager::OnChildProcessCreated(pid_t appPid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = subSpawnMap_.find(appPid);
    if (it != subSpawnMap_.end() && it->second->IsReady()) {
        it->second->IncrementChildProcessCount();
        CancelIdleTimer(it->second);
        TAG_LOGD(AAFwkTag::APPMGR, "OnChildProcessCreated: appPid:%{public}d, count:%{public}d",
            appPid, it->second->GetChildProcessCount());
    }
}

void SubSpawnManager::OnChildProcessExited(pid_t appPid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = subSpawnMap_.find(appPid);
    if (it != subSpawnMap_.end() && it->second->IsReady()) {
        it->second->DecrementChildProcessCount();
        TAG_LOGD(AAFwkTag::APPMGR, "OnChildProcessExited: appPid:%{public}d, count:%{public}d",
            appPid, it->second->GetChildProcessCount());
        if (it->second->GetChildProcessCount() == 0) {
            StartIdleTimer(it->second);
            TAG_LOGD(AAFwkTag::APPMGR, "OnChildProcessExited: start idle timer, appPid:%{public}d", appPid);
        }
    }
}

void SubSpawnManager::OnAppProcessDied(pid_t appPid)
{
    std::shared_ptr<SubSpawnRecord> record;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = subSpawnMap_.find(appPid);
        if (it == subSpawnMap_.end()) {
            return;
        }
        record = it->second;
        if (record->IsTerminating()) {
            TAG_LOGD(AAFwkTag::APPMGR, "subspawn already terminating, appPid:%{public}d", appPid);
            return;
        }
        record->SetTerminating(true);
    }

    pid_t subSpawnPid = record->GetSubSpawnPid();
    if (subSpawnPid > 0) {
        kill(subSpawnPid, SIGKILL);
        TAG_LOGI(AAFwkTag::APPMGR, "killed subspawn pid:%{public}d for appPid:%{public}d",
            subSpawnPid, appPid);
        SendSubSpawnShutdown(record->GetBundleName(), subSpawnPid);
    }
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
    pid_t appPid = record->GetAppPid();

    ffrt::submit([=]() {
        ffrt::this_task::sleep_for(std::chrono::milliseconds(IDLE_TIMEOUT_MS));
        GetInstance().HandleIdleTimeout(appPid);
    });
}

void SubSpawnManager::CancelIdleTimer(const std::shared_ptr<SubSpawnRecord> &record)
{
    if (record) {
        record->SetIdleTimerActive(false);
    }
}

void SubSpawnManager::HandleIdleTimeout(pid_t appPid)
{
    std::shared_ptr<SubSpawnRecord> record;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = subSpawnMap_.find(appPid);
        if (it == subSpawnMap_.end()) {
            TAG_LOGD(AAFwkTag::APPMGR, "HandleIdleTimeout: no record, appPid:%{public}d", appPid);
            return;
        }
        record = it->second;
        if (record->GetChildProcessCount() > 0 || !record->IsIdleTimerActive()) {
            TAG_LOGD(AAFwkTag::APPMGR, "HandleIdleTimeout: skip, count:%{public}d, active:%{public}d",
                record->GetChildProcessCount(), record->IsIdleTimerActive());
            return;
        }
        if (record->IsTerminating()) {
            TAG_LOGD(AAFwkTag::APPMGR, "HandleIdleTimeout: already terminating, appPid:%{public}d", appPid);
            return;
        }
        record->SetTerminating(true);
    }

    pid_t subSpawnPid = record->GetSubSpawnPid();
    if (subSpawnPid > 0) {
        kill(subSpawnPid, SIGKILL);
        TAG_LOGI(AAFwkTag::APPMGR, "idle timeout, killed subspawn pid:%{public}d for appPid:%{public}d",
            subSpawnPid, appPid);
        SendSubSpawnShutdown(record->GetBundleName(), subSpawnPid);
    }
}

} // namespace AppExecFwk
} // namespace OHOS
