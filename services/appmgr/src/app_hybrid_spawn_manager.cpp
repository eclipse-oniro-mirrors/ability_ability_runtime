/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "app_hybrid_spawn_manager.h"

#include <sys/epoll.h>

#include "c/executor_task.h"
#include "syspara/parameter.h"

namespace OHOS {
namespace AppExecFwk {

namespace {
//listen fd use
constexpr int32_t PIPE_MSG_READ_BUFFER = 1024;
constexpr const char* HYBRIDSPAWN_EXIT = "startup.service.ctl.hybridspawn";
constexpr const char* HYBRIDSPAWN_EXIT_SIGNAL = "5";
}

AppHybridSpawnManager::~AppHybridSpawnManager() {}

AppHybridSpawnManager::AppHybridSpawnManager() {}

AppHybridSpawnManager &AppHybridSpawnManager::GetInstance()
{
    static AppHybridSpawnManager manager;
    return manager;
}

static void ProcessSignalData(void *token, uint32_t event)
{
    int rFd = AppHybridSpawnManager::GetInstance().GetHRfd();
    if (rFd <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "rFd is invalid, %{public}d", rFd);
        return;
    }
    // read data from appspawn
    char buffer[PIPE_MSG_READ_BUFFER] = {0};
    std::string readResult = "";
    int count = read(rFd, buffer, PIPE_MSG_READ_BUFFER - 1);
    if (count == -1) {
        TAG_LOGE(AAFwkTag::APPMGR, "read pipe failed");
    } else if (count == 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "write end closed");
        fdsan_close_with_tag(rFd, static_cast<uint32_t>(AAFwkTag::APPMGR));
    } else {
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
            TAG_LOGE(AAFwkTag::APPMGR, "type check failed for one or more fields");
            return;
        }
        pid = jsonObject["pid"];
        signal = jsonObject["signal"];
        uid = jsonObject["uid"];
        bundleName = jsonObject["bundleName"];
        if (signal == 0) {
            TAG_LOGD(AAFwkTag::APPMGR, "ignore signal 0, pid: %{public}d", pid);
            return;
        }
        TAG_LOGD(AAFwkTag::APPMGR, "To update reason detail info because of SIGNAL");
        AppHybridSpawnManager::GetInstance().RecordAppExitSignalReason(pid, uid, signal, bundleName);
    }
}

static void HybridSpawnExitCallback(const char *key, const char *value, void *context)
{
    if (value == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "value nullptr");
        return;
    }
    TAG_LOGI(AAFwkTag::APPMGR, "value is: %{public}s", value);
    // set flag
    if (strcmp(value, HYBRIDSPAWN_EXIT_SIGNAL) == 0) {
        int ret = HybridSpawnListenCloseSet();
        if (ret != 0) {
            TAG_LOGE(AAFwkTag::APPMGR, "HybridSpawnListenCloseSet failed");
        }
    }
}

void AppHybridSpawnManager::InitHybridSpawnMsgPipe(std::weak_ptr<AppMgrServiceInner> appMgrServiceInner)
{
    appMgrServiceInner_ = appMgrServiceInner;
    if (appMgrServiceInner_.lock() == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgrServiceInner_ is null");
        return;
    }

    int pipeFd[2];
    if (pipe(pipeFd) != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "create pipe failed");
        return;
    }
    hrFd_ = pipeFd[0];
    hwFd_ = pipeFd[1];
    fdsan_exchange_owner_tag(hrFd_, 0, static_cast<uint32_t>(AAFwkTag::APPMGR));
    fdsan_exchange_owner_tag(hwFd_, 0, static_cast<uint32_t>(AAFwkTag::APPMGR));
    TAG_LOGI(AAFwkTag::APPMGR, "hrFd is: %{public}d, hwFd is: %{public}d", hrFd_, hwFd_);

    // send fd
    int ret = HybridSpawnListenFdSet(hwFd_);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "send fd to hybridspawn failed, ret: %{public}d", ret);
        fdsan_close_with_tag(hrFd_, static_cast<uint32_t>(AAFwkTag::APPMGR));
        fdsan_close_with_tag(hwFd_, static_cast<uint32_t>(AAFwkTag::APPMGR));
        hrFd_ = -1;
        hwFd_ = -1;
        return;
    }
    ret = WatchParameter(HYBRIDSPAWN_EXIT, HybridSpawnExitCallback, nullptr);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "watch parameter ret: %{public}d", ret);
        fdsan_close_with_tag(hrFd_, static_cast<uint32_t>(AAFwkTag::APPMGR));
        fdsan_close_with_tag(hwFd_, static_cast<uint32_t>(AAFwkTag::APPMGR));
        hrFd_ = -1;
        hwFd_ = -1;
        return;
    }
    ffrt_qos_t taskQos = 0;
    ret = ffrt_epoll_ctl(taskQos, EPOLL_CTL_ADD, hrFd_, EPOLLIN, nullptr, ProcessSignalData);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "ffrt_epoll_ctl failed, ret: %{public}d", ret);
        int32_t unwatchRet = RemoveParameterWatcher(HYBRIDSPAWN_EXIT, HybridSpawnExitCallback, nullptr);
        if (unwatchRet != 0) {
            TAG_LOGW(AAFwkTag::APPMGR, "remove parameter watcher failed, ret: %{public}d", unwatchRet);
        }
        fdsan_close_with_tag(hrFd_, static_cast<uint32_t>(AAFwkTag::APPMGR));
        fdsan_close_with_tag(hwFd_, static_cast<uint32_t>(AAFwkTag::APPMGR));
        hrFd_ = -1;
        hwFd_ = -1;
        return;
    }
    TAG_LOGI(AAFwkTag::APPMGR, "Listen signal msg ...");
}

void AppHybridSpawnManager::RecordAppExitSignalReason(int32_t pid, int32_t uid, int32_t signal, std::string &bundleName)
{
    auto appMgrServiceInner = appMgrServiceInner_.lock();
    if (appMgrServiceInner) {
        appMgrServiceInner->RecordAppExitSignalReason(pid, uid, signal, bundleName);
    }
}
} // end AppExecFwk
} // end OHOS