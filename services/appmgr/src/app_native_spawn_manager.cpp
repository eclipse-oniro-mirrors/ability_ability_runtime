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

#include "app_native_spawn_manager.h"

#include <nlohmann/json.hpp>
#include <sys/epoll.h>

#include "ability_manager_errors.h"
#include "appspawn.h"
#include "c/executor_task.h"
#include "ffrt.h"
#include "ffrt_inner.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "syspara/parameter.h"

namespace OHOS {
namespace AppExecFwk {

namespace {
//listen fd use
constexpr int32_t PIPE_MSG_READ_BUFFER = 1024;
constexpr const char* NATIVESPAWN_EXIT = "startup.service.ctl.nativespawn";
constexpr const char* NATIVESPAWN_EXIT_SIGNAL = "5";
}

AppNativeSpawnManager::~AppNativeSpawnManager() {}

AppNativeSpawnManager::AppNativeSpawnManager() {}

AppNativeSpawnManager &AppNativeSpawnManager::GetInstance()
{
    static AppNativeSpawnManager manager;
    return manager;
}

int32_t AppNativeSpawnManager::RegisterNativeChildExitNotify(const sptr<INativeChildNotify> &callback)
{
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "register null callback");
        return ERR_INVALID_VALUE;
    }
    int32_t callingPid = IPCSkeleton::GetCallingPid();
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is null");
        return ERR_INVALID_VALUE;
    }
    auto appRecord = appRunningManager_->GetAppRunningRecordByPid(callingPid);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no appRecord, parentPid:%{public}d", callingPid);
        return OHOS::AAFwk::ERR_CALLER_NOT_EXISTS;
    }
    std::lock_guard lock(nativeChildCallbackLock_);
    if (nativeChildCallbackMap_.find(callingPid) != nativeChildCallbackMap_.end()) {
        TAG_LOGE(AAFwkTag::APPMGR, "register native child exit:%{public}d fail", callingPid);
        return OHOS::AAFwk::ERR_INVALID_CALLER;
    }
    nativeChildCallbackMap_[callingPid] = callback;
    TAG_LOGI(AAFwkTag::APPMGR, "register native child exit:%{public}d success", callingPid);
    return ERR_OK;
}

int32_t AppNativeSpawnManager::UnregisterNativeChildExitNotify(const sptr<INativeChildNotify> &callback)
{
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "unregister null callback");
        return ERR_INVALID_VALUE;
    }
    int32_t callingPid = IPCSkeleton::GetCallingPid();
    std::lock_guard lock(nativeChildCallbackLock_);
    auto iter = nativeChildCallbackMap_.find(callingPid);
    if (iter == nativeChildCallbackMap_.end()) {
        TAG_LOGE(AAFwkTag::APPMGR, "unregister callback not exist:%{public}d", callingPid);
        return OHOS::AAFwk::ERR_INVALID_CALLER;
    }
    if (iter->second == nullptr || iter->second->AsObject() != callback->AsObject()) {
        TAG_LOGE(AAFwkTag::APPMGR, "unregister callback not same:%{public}d", callingPid);
        return OHOS::AAFwk::ERR_INVALID_CALLER;
    }

    nativeChildCallbackMap_.erase(callingPid);
    TAG_LOGI(AAFwkTag::APPMGR, "unregister native child exit:%{public}d success", callingPid);
    return ERR_OK;
}

static void AppNativeSpawnStartCallback(const char *key, const char *value, void *context)
{
    if (value == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "value nullptr");
        return;
    }
    TAG_LOGI(AAFwkTag::APPMGR, "value is: %{public}s", value);
    // set flag
    if (strcmp(value, NATIVESPAWN_EXIT_SIGNAL) == 0) {
        int ret = NativeSpawnListenCloseSet();
        if (ret != 0) {
            TAG_LOGE(AAFwkTag::APPMGR, "NativeSpawnListenCloseSet failed");
        }
    }
}

void AppNativeSpawnManager::NotifyChildProcessExitTask(int32_t pid, int32_t signal, const std::string &bundleName)
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "get appRunningManager fail");
        return;
    }

    int32_t parentPid = 0;
    auto appRecord = appRunningManager_->GetAppRunningRecordByChildProcessPid(pid);
    if (!appRecord) {
        parentPid = GetChildRelation(pid);
        if (parentPid <= 0) {
            TAG_LOGE(AAFwkTag::APPMGR, "not find parent, childPid:%{public}d", pid);
            return;
        }
        RemoveChildRelation(pid);
    } else {
        parentPid = appRecord->GetPid();
    }

    auto nativeChildCallbacks = GetNativeChildCallbackByPid(parentPid);
    if (!nativeChildCallbacks) {
        TAG_LOGW(AAFwkTag::APPMGR, "not found native child process callback");
        return;
    }
    auto ret = nativeChildCallbacks->OnNativeChildExit(pid, signal);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "OnNativeChildExit failed, pid: %{public}d", pid);
    }
}

static void ProcessSignalData(void *token, uint32_t event)
{
    int rFd = AppNativeSpawnManager::GetInstance().GetNRfd();
    if (rFd <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "rFd is invalid, %{public}d", rFd);
        return;
    }

    // read data from nativespawn
    char buffer[PIPE_MSG_READ_BUFFER] = {0};
    std::string readResult = "";
    int count = read(rFd, buffer, PIPE_MSG_READ_BUFFER - 1);
    if (count <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "read pipe failed");
        return;
    }

    int32_t pid = -1;
    int32_t signal = -1;
    int32_t uid = 0;
    std::string bundleName = "";
    std::string bufferStr = buffer;
    TAG_LOGD(AAFwkTag::APPMGR, "buffer read: %{public}s", bufferStr.c_str());
    nlohmann::json jsonObject = nlohmann::json::parse(bufferStr, nullptr, false);
    if (jsonObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::APPMGR, "parse json string failed");
        return;
    }
    if (!jsonObject.contains("pid") || !jsonObject.contains("signal") || !jsonObject.contains("uid")
        || !jsonObject.contains("bundleName")) {
        TAG_LOGE(AAFwkTag::APPMGR, "info lost!");
        return;
    }
    if (!jsonObject["pid"].is_number_integer() || !jsonObject["signal"].is_number_integer() ||
        !jsonObject["uid"].is_number_integer() || !jsonObject["bundleName"].is_string()) {
        TAG_LOGE(AAFwkTag::APPMGR, "info type err!");
        return;
    }

    pid = jsonObject["pid"];
    signal = jsonObject["signal"];
    uid = jsonObject["uid"];
    bundleName = jsonObject["bundleName"];
    TAG_LOGI(AAFwkTag::APPMGR, "pid:%{public}d, signal:%{public}d, uid:%{public}d, bundleName:%{public}s",
        pid, signal, uid, bundleName.c_str());
    AppNativeSpawnManager::GetInstance().NotifyChildProcessExitTask(pid, signal, bundleName);
}

sptr<INativeChildNotify> AppNativeSpawnManager::GetNativeChildCallbackByPid(int32_t pid)
{
    std::lock_guard lock(nativeChildCallbackLock_);
    auto it = nativeChildCallbackMap_.find(pid);
    return it != nativeChildCallbackMap_.end() ? it->second : nullptr;
}

void AppNativeSpawnManager::RemoveNativeChildCallbackByPid(int32_t pid)
{
    TAG_LOGI(AAFwkTag::APPMGR, "remove native child callback, pid:%{public}d", pid);
    std::lock_guard lock(nativeChildCallbackLock_);
    nativeChildCallbackMap_.erase(pid);
}

void AppNativeSpawnManager::InitNativeSpawnMsgPipe(std::shared_ptr<AppRunningManager> appRunningManager)
{
    appRunningManager_ = appRunningManager;
    int pipeFd[2];
    if (pipe(pipeFd) != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "create native pipe failed");
        return;
    }
    nrFd_ = pipeFd[0];
    nwFd_ = pipeFd[1];
    fdsan_exchange_owner_tag(nrFd_, 0, static_cast<uint32_t>(AAFwkTag::APPMGR));
    fdsan_exchange_owner_tag(nwFd_, 0, static_cast<uint32_t>(AAFwkTag::APPMGR));
    TAG_LOGI(AAFwkTag::APPMGR, "nrFd is: %{public}d, nwFd is: %{public}d", nrFd_, nwFd_);
    // send fd
    int ret = NativeSpawnListenFdSet(nwFd_);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "send fd to native spawn failed, ret: %{public}d", ret);
        fdsan_close_with_tag(nrFd_, static_cast<uint32_t>(AAFwkTag::APPMGR));
        fdsan_close_with_tag(nwFd_, static_cast<uint32_t>(AAFwkTag::APPMGR));
        return;
    }
    ret = WatchParameter(NATIVESPAWN_EXIT, AppNativeSpawnStartCallback, nullptr);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "watch native parameter, ret :%{public}d", ret);
        fdsan_close_with_tag(nrFd_, static_cast<uint32_t>(AAFwkTag::APPMGR));
        fdsan_close_with_tag(nwFd_, static_cast<uint32_t>(AAFwkTag::APPMGR));
        return;
    }
    ffrt_qos_t taskQos = 0;
    ret = ffrt_epoll_ctl(taskQos, EPOLL_CTL_ADD, nrFd_, EPOLLIN, nullptr, ProcessSignalData);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "ffrt_epoll_ctl failed, ret :%{public}d", ret);
        fdsan_close_with_tag(nrFd_, static_cast<uint32_t>(AAFwkTag::APPMGR));
        fdsan_close_with_tag(nwFd_, static_cast<uint32_t>(AAFwkTag::APPMGR));
        return;
    }
    TAG_LOGI(AAFwkTag::APPMGR, "Listen native signal msg ...");
}

int32_t AppNativeSpawnManager::GetChildRelation(int32_t childPid)
{
    std::lock_guard lock(childRelationLock_);
    auto iter = childRelationMap_.find(childPid);
    if (iter != childRelationMap_.end()) {
        return iter->second;
    }
    return 0;
}

void AppNativeSpawnManager::AddChildRelation(int32_t childPid, int32_t parentPid)
{
    std::lock_guard lock(childRelationLock_);
    childRelationMap_[childPid] = parentPid;
}

void AppNativeSpawnManager::RemoveChildRelation(int32_t childPid)
{
    std::lock_guard lock(childRelationLock_);
    childRelationMap_.erase(childPid);
}
} // end AppExecFwk
} // end OHOS
