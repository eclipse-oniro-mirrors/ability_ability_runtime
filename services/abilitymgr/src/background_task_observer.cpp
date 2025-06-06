/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
#include "background_task_observer.h"

#include "hilog_tag_wrapper.h"
#include "sa_mgr_client.h"
#include "system_ability_definition.h"
#include "resource_type.h"

namespace OHOS {
namespace AAFwk {
BackgroundTaskObserver::BackgroundTaskObserver()
{}

BackgroundTaskObserver::~BackgroundTaskObserver()
{}

void BackgroundTaskObserver::OnContinuousTaskStart(const std::shared_ptr<BackgroundTaskMgr::ContinuousTaskCallbackInfo>
    &continuousTaskCallbackInfo)
{
    TAG_LOGD(
        AAFwkTag::ABILITYMGR, "OnContinuousTaskStart, uid:%{public}d", continuousTaskCallbackInfo->GetCreatorUid());
    std::lock_guard<std::mutex> lock(bgTaskMutex_);
    bgTaskUids_.push_front(continuousTaskCallbackInfo->GetCreatorUid());
    if (appManager_ == nullptr) {
        GetAppManager();
    }
    if (appManager_ != nullptr) {
        appManager_->SetContinuousTaskProcess(continuousTaskCallbackInfo->GetCreatorPid(), true);
    }
}

void BackgroundTaskObserver::OnContinuousTaskStop(const std::shared_ptr<BackgroundTaskMgr::ContinuousTaskCallbackInfo>
    &continuousTaskCallbackInfo)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "OnContinuousTaskStop, uid:%{public}d", continuousTaskCallbackInfo->GetCreatorUid());
    std::lock_guard<std::mutex> lock(bgTaskMutex_);
    bgTaskUids_.remove(continuousTaskCallbackInfo->GetCreatorUid());
    if (appManager_ == nullptr) {
        GetAppManager();
    }
    if (appManager_ != nullptr) {
        appManager_->SetContinuousTaskProcess(continuousTaskCallbackInfo->GetCreatorPid(), false);
    }
}

void BackgroundTaskObserver::OnProcEfficiencyResourcesApply(
    const std::shared_ptr<BackgroundTaskMgr::ResourceCallbackInfo> &resourceInfo)
{
    if (!resourceInfo || (resourceInfo->GetResourceNumber() & BackgroundTaskMgr::ResourceType::WORK_SCHEDULER) == 0) {
        return;
    }
    std::lock_guard<std::mutex> lock(efficiencyMutex_);
    efficiencyUids_.push_back(resourceInfo->GetUid());
}

void BackgroundTaskObserver::OnProcEfficiencyResourcesReset(
    const std::shared_ptr<BackgroundTaskMgr::ResourceCallbackInfo> &resourceInfo)
{
    if (!resourceInfo || (resourceInfo->GetResourceNumber() & BackgroundTaskMgr::ResourceType::WORK_SCHEDULER) == 0) {
        return;
    }
    std::lock_guard<std::mutex> lock(efficiencyMutex_);
    int32_t uid = resourceInfo->GetUid();
    auto iter = std::find(efficiencyUids_.begin(), efficiencyUids_.end(), uid);
    if (iter != efficiencyUids_.end()) {
        efficiencyUids_.erase(iter);
    }
}

void BackgroundTaskObserver::OnAppEfficiencyResourcesApply(
    const std::shared_ptr<BackgroundTaskMgr::ResourceCallbackInfo> &resourceInfo)
{
    OnProcEfficiencyResourcesApply(resourceInfo);
}

void BackgroundTaskObserver::OnAppEfficiencyResourcesReset(
    const std::shared_ptr<BackgroundTaskMgr::ResourceCallbackInfo> &resourceInfo)
{
    OnProcEfficiencyResourcesReset(resourceInfo);
}

void BackgroundTaskObserver::GetContinuousTaskApps()
{
    std::vector<std::shared_ptr<BackgroundTaskMgr::ContinuousTaskCallbackInfo>> continuousTasks;
    ErrCode result = BackgroundTaskMgr::BackgroundTaskMgrHelper::GetContinuousTaskApps(continuousTasks);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to GetContinuousTaskApps, err: %{public}d", result);
        return;
    }
    std::lock_guard<std::mutex> lock(bgTaskMutex_);
    bgTaskUids_.clear();
    for (size_t index = 0; index < continuousTasks.size(); index++) {
        bgTaskUids_.push_front(continuousTasks[index]->GetCreatorUid());
    }
}

void BackgroundTaskObserver::GetEfficiencyResourcesTaskApps()
{
    std::vector<std::shared_ptr<BackgroundTaskMgr::ResourceCallbackInfo>> appList;
    std::vector<std::shared_ptr<BackgroundTaskMgr::ResourceCallbackInfo>> procList;
    ErrCode result = BackgroundTaskMgr::BackgroundTaskMgrHelper::GetEfficiencyResourcesInfos(appList, procList);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to GetEfficiencyResourcesInfos, err: %{public}d", result);
        return;
    }
    std::lock_guard<std::mutex> lock(efficiencyMutex_);
    efficiencyUids_.clear();
    for (auto& info : appList) {
        if (info == nullptr) {
            continue;
        }
        if ((info->GetResourceNumber() & BackgroundTaskMgr::ResourceType::WORK_SCHEDULER) != 0) {
            efficiencyUids_.push_back(info->GetUid());
        }
    }
    for (auto& info : procList) {
        if (info == nullptr) {
            continue;
        }
        if ((info->GetResourceNumber() & BackgroundTaskMgr::ResourceType::WORK_SCHEDULER) != 0) {
            efficiencyUids_.push_back(info->GetUid());
        }
    }
}

bool BackgroundTaskObserver::IsBackgroundTaskUid(const int uid)
{
    std::lock_guard<std::mutex> lock(bgTaskMutex_);
    auto iter = find(bgTaskUids_.begin(), bgTaskUids_.end(), uid);
    if (iter != bgTaskUids_.end()) {
        return true;
    }
    return false;
}

bool BackgroundTaskObserver::IsEfficiencyResourcesTaskUid(const int uid)
{
    std::lock_guard<std::mutex> lock(efficiencyMutex_);
    auto iter = std::find(efficiencyUids_.begin(), efficiencyUids_.end(), uid);
    if (iter != efficiencyUids_.end()) {
        return true;
    }
    return false;
}

sptr<AppExecFwk::IAppMgr> BackgroundTaskObserver::GetAppManager()
{
    if (appManager_ == nullptr) {
        auto appObj =
            OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->GetSystemAbility(APP_MGR_SERVICE_ID);
        if (appObj == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null appObj");
            return nullptr;
        }
        appManager_ = iface_cast<AppExecFwk::IAppMgr>(appObj);
    }
    return appManager_;
}
}  // namespace AAFwk
}  // namespace OHOS
#endif
