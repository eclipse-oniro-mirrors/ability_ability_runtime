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

#include "app_utils.h"
#include "ffrt_inner.h"
#include "hilog_tag_wrapper.h"
#include "appspawn.h"

namespace OHOS {
namespace AppExecFwk {

extern "C" __attribute__((weak)) char *GetSubSpawnSocketName(int uid);
extern "C" __attribute__((weak)) int AppSpawnClientInitSubSpawn(const char *socketName,
    AppSpawnClientHandle *handle);

namespace {
constexpr const char* SUBSPAWN_PROCESS_NAME_SUFFIX = ":subspawn";
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
    if (!AppUtils::GetInstance().IsSupportSubSpawn()) {
        return false;
    }
    if (preloadSoList.empty()) {
        return false;
    }
    if (!isolationMode) {
        return false;
    }
    return true;
}

std::string SubSpawnManager::QuerySocketNameByUid(int32_t uid)
{
    if (!::GetSubSpawnSocketName) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetSubSpawnSocketName not available");
        return "";
    }
    char *socketName = ::GetSubSpawnSocketName(uid);
    if (socketName == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetSubSpawnSocketName returned null for uid:%{public}d", uid);
        return "";
    }
    std::string result(socketName);
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

    std::string procName = bundleName + SUBSPAWN_PROCESS_NAME_SUFFIX;
    AppSpawnReqMsgHandle reqHandle = nullptr;
    ret = AppSpawnReqMsgCreate(static_cast<AppSpawnMsgType>(startMsg.code), procName.c_str(), &reqHandle);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppSpawnReqMsgCreate fail, ret:%{public}d", ret);
        AppSpawnClientDestroy(handle);
        return -1;
    }

    AppSpawnClient spawnClient(APPSPAWN_SERVER_NAME);
    ret = spawnClient.AppspawnCreateDefaultMsg(startMsg, reqHandle);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppspawnCreateDefaultMsg fail, ret:%{public}d", ret);
        AppSpawnClientDestroy(handle);
        return -1;
    }

    AppSpawnResult result = {0};
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
        if (it != subSpawnMap_.end() && it->second.subSpawnPid > 0) {
            return;
        }
    }

    auto startMsgCopy = std::make_shared<AppSpawnStartMsg>(startMsg);
    std::string bundleNameCopy = bundleName;
    ffrt::submit([=]() {
        pid_t subSpawnPid = GetInstance().DoCreateSubSpawn(appPid, uid, bundleNameCopy, *startMsgCopy);
        if (subSpawnPid <= 0) {
            TAG_LOGE(AAFwkTag::APPMGR, "async create subspawn failed for appPid:%{public}d", appPid);
            return;
        }

        std::string socketName = GetInstance().QuerySocketNameByUid(uid);

        {
            std::lock_guard<std::mutex> lock(GetInstance().mutex_);
            SubSpawnRecord record;
            record.subSpawnPid = subSpawnPid;
            record.appPid = appPid;
            record.bundleName = bundleNameCopy;
            record.uid = uid;
            record.socketName = socketName;
            GetInstance().subSpawnMap_[appPid] = record;
            GetInstance().pidToAppMap_[subSpawnPid] = appPid;
        }

        TAG_LOGI(AAFwkTag::APPMGR, "subspawn ready, pid:%{public}d, appPid:%{public}d",
            subSpawnPid, appPid);
    });
}

pid_t SubSpawnManager::StartChildViaSubSpawn(pid_t appPid, int32_t uid, const AppSpawnStartMsg &startMsg)
{
    std::string bundleName;
    bool needAsyncCreate = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = subSpawnMap_.find(appPid);
        if (it == subSpawnMap_.end() || it->second.subSpawnPid <= 0) {
            bundleName = it != subSpawnMap_.end() ? it->second.bundleName : "";
            needAsyncCreate = true;
        }
    }
    if (needAsyncCreate) {
        AsyncCreateSubSpawn(appPid, uid, bundleName, startMsg);
        return -1;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    auto it = subSpawnMap_.find(appPid);
    if (it == subSpawnMap_.end()) {
        return -1;
    }
    auto &record = it->second;
    if (record.socketName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "subspawn socketName empty, appPid:%{public}d", appPid);
        return -1;
    }

    if (!::AppSpawnClientInitSubSpawn) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppSpawnClientInitSubSpawn not available");
        return -1;
    }

    AppSpawnClientHandle handle = nullptr;
    int ret = ::AppSpawnClientInitSubSpawn(record.socketName.c_str(), &handle);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppSpawnClientInitSubSpawn fail, ret:%{public}d", ret);
        return -1;
    }

    AppSpawnReqMsgHandle reqHandle = nullptr;
    ret = AppSpawnReqMsgCreate(MSG_APP_SPAWN, startMsg.procName.c_str(), &reqHandle);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppSpawnReqMsgCreate fail, ret:%{public}d", ret);
        AppSpawnClientDestroy(handle);
        return -1;
    }

    AppSpawnClient spawnClient(APPSPAWN_SERVER_NAME);
    ret = spawnClient.AppspawnCreateDefaultMsg(startMsg, reqHandle);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppspawnCreateDefaultMsg fail, ret:%{public}d", ret);
        AppSpawnClientDestroy(handle);
        return -1;
    }

    AppSpawnResult result = {0};
    ret = AppSpawnClientSendMsg(handle, reqHandle, &result);
    if (ret != 0 || result.pid <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "subspawn fork child fail, ret:%{public}d, pid:%{public}d",
            ret, result.pid);
        AppSpawnClientDestroy(handle);
        return -1;
    }

    AppSpawnClientDestroy(handle);
    record.childProcessCount++;
    CancelIdleTimer(appPid);
    TAG_LOGI(AAFwkTag::APPMGR, "child created via subspawn, pid:%{public}d, appPid:%{public}d",
        result.pid, appPid);
    return result.pid;
}

void SubSpawnManager::OnSubSpawnExited(pid_t subSpawnPid)
{
    pid_t appPid = 0;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto pidIt = pidToAppMap_.find(subSpawnPid);
        if (pidIt == pidToAppMap_.end()) {
            TAG_LOGW(AAFwkTag::APPMGR, "unknown subspawn pid:%{public}d", subSpawnPid);
            return;
        }
        appPid = pidIt->second;
        pidToAppMap_.erase(pidIt);
        subSpawnMap_.erase(appPid);
    }
    TAG_LOGI(AAFwkTag::APPMGR, "subspawn exited, pid:%{public}d, appPid:%{public}d",
        subSpawnPid, appPid);
}

void SubSpawnManager::OnChildProcessCreated(pid_t appPid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = subSpawnMap_.find(appPid);
    if (it != subSpawnMap_.end()) {
        it->second.childProcessCount++;
        CancelIdleTimer(appPid);
    }
}

void SubSpawnManager::OnChildProcessExited(pid_t appPid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = subSpawnMap_.find(appPid);
    if (it != subSpawnMap_.end()) {
        it->second.childProcessCount--;
        if (it->second.childProcessCount <= 0) {
            it->second.childProcessCount = 0;
            StartIdleTimer(appPid);
        }
    }
}

void SubSpawnManager::OnAppProcessDied(pid_t appPid)
{
    pid_t subSpawnPid = 0;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = subSpawnMap_.find(appPid);
        if (it == subSpawnMap_.end()) {
            return;
        }
        subSpawnPid = it->second.subSpawnPid;
        pidToAppMap_.erase(subSpawnPid);
        subSpawnMap_.erase(it);
    }

    if (subSpawnPid > 0) {
        kill(subSpawnPid, SIGKILL);
        TAG_LOGI(AAFwkTag::APPMGR, "killed subspawn pid:%{public}d for appPid:%{public}d",
            subSpawnPid, appPid);
    }
}

bool SubSpawnManager::IsSubSpawn(pid_t pid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    return pidToAppMap_.find(pid) != pidToAppMap_.end();
}

void SubSpawnManager::StartIdleTimer(pid_t appPid)
{
    auto it = subSpawnMap_.find(appPid);
    if (it == subSpawnMap_.end() || it->second.idleTimerActive) {
        return;
    }
    it->second.idleTimerActive = true;

    ffrt::submit([=]() {
        ffrt::this_task::sleep_for(std::chrono::milliseconds(IDLE_TIMEOUT_MS));
        GetInstance().HandleIdleTimeout(appPid);
    });
}

void SubSpawnManager::CancelIdleTimer(pid_t appPid)
{
    auto it = subSpawnMap_.find(appPid);
    if (it != subSpawnMap_.end()) {
        it->second.idleTimerActive = false;
    }
}

void SubSpawnManager::HandleIdleTimeout(pid_t appPid)
{
    pid_t subSpawnPid = 0;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = subSpawnMap_.find(appPid);
        if (it == subSpawnMap_.end()) {
            return;
        }
        if (it->second.childProcessCount > 0 || !it->second.idleTimerActive) {
            return;
        }
        subSpawnPid = it->second.subSpawnPid;
        pidToAppMap_.erase(subSpawnPid);
        subSpawnMap_.erase(it);
    }

    if (subSpawnPid > 0) {
        kill(subSpawnPid, SIGKILL);
        TAG_LOGI(AAFwkTag::APPMGR, "idle timeout, killed subspawn pid:%{public}d for appPid:%{public}d",
            subSpawnPid, appPid);
    }
}

} // namespace AppExecFwk
} // namespace OHOS
