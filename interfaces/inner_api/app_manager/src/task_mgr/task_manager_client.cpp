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

#include "task_mgr/task_manager_client.h"

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::AAFwk;

TaskManagerClient &TaskManagerClient::GetInstance()
{
    static TaskManagerClient instance;
    return instance;
}

void TaskManagerClient::TaskManagerDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    TaskManagerClient::GetInstance().ResetProxy(remote);
}

int32_t TaskManagerClient::StartTask(const std::string &taskId, int32_t priority)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto proxy = GetTaskManagerProxy();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "task manager proxy is null");
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    auto ret = proxy->StartTask(taskId, priority);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "start task failed: %{public}d", ret);
    }
    return ret;
}

int32_t TaskManagerClient::FinishTask(const std::string &taskId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto proxy = GetTaskManagerProxy();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "task manager proxy is null");
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    auto ret = proxy->FinishTask(taskId);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "finish task failed: %{public}d", ret);
    }
    return ret;
}

int32_t TaskManagerClient::RegisterManagedProcess(const std::string &taskId, int32_t pid, int32_t toolType)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto proxy = GetTaskManagerProxy();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "task manager proxy is null");
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    auto ret = proxy->RegisterManagedProcess(taskId, pid, toolType);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "register managed process failed: %{public}d", ret);
    }
    return ret;
}

int32_t TaskManagerClient::UnregisterManagedProcess(const std::string &taskId, int32_t pid)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto proxy = GetTaskManagerProxy();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "task manager proxy is null");
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    auto ret = proxy->UnregisterManagedProcess(taskId, pid);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "unregister managed process failed: %{public}d", ret);
    }
    return ret;
}

sptr<ITaskManager> TaskManagerClient::GetTaskManagerProxy()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (taskManagerProxy_ != nullptr) {
        return taskManagerProxy_;
    }
    auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityMgr == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "failed to get SystemAbilityManager");
        return nullptr;
    }
    auto remoteObject = systemAbilityMgr->GetSystemAbility(APP_MGR_SERVICE_ID);
    if (remoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "get system ability %{public}d failed", APP_MGR_SERVICE_ID);
        return nullptr;
    }
    if (deathRecipient_ == nullptr) {
        deathRecipient_ = sptr(new (std::nothrow) TaskManagerDeathRecipient());
        if (deathRecipient_ == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "failed to create death recipient");
            return nullptr;
        }
    }
    remoteObject->AddDeathRecipient(deathRecipient_);
    taskManagerProxy_ = iface_cast<ITaskManager>(remoteObject);
    TAG_LOGI(AAFwkTag::APPMGR, "get task manager proxy %{public}s",
        taskManagerProxy_ != nullptr ? "success" : "failed");
    return taskManagerProxy_;
}

void TaskManagerClient::ResetProxy(const wptr<IRemoteObject> &remote)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (taskManagerProxy_ == nullptr) {
        return;
    }
    auto remoteObject = remote.promote();
    if (remoteObject != nullptr && remoteObject == taskManagerProxy_->AsObject()) {
        taskManagerProxy_ = nullptr;
        deathRecipient_ = nullptr;
        TAG_LOGW(AAFwkTag::APPMGR, "app mgr service died, reset task manager proxy");
    }
}
} // namespace AppExecFwk
} // namespace OHOS
