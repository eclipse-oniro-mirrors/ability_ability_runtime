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
#include "exit_resident_process_manager.h"

#include <mutex>

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "in_process_call_wrapper.h"
#include "remote_client_manager.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t U0_USER_ID = 0;
constexpr int32_t BASE_USER_RANGE = 200000;
}
ExitResidentProcessManager::~ExitResidentProcessManager() {}

ExitResidentProcessManager::ExitResidentProcessManager() {}

ExitResidentProcessManager &ExitResidentProcessManager::GetInstance()
{
    static ExitResidentProcessManager instance;
    return instance;
}

bool ExitResidentProcessManager::IsMemorySizeSufficient() const
{
    std::lock_guard<ffrt::mutex> lock(mutexLock_);
    return currentMemorySizeState_ == MemorySizeState::MEMORY_SIZE_SUFFICIENT;
}

bool ExitResidentProcessManager::RecordExitResidentBundleName(const std::string &bundleName, int32_t uid)
{
    std::lock_guard<ffrt::mutex> lock(mutexLock_);
    if (currentMemorySizeState_ == MemorySizeState::MEMORY_SIZE_SUFFICIENT) {
        return false;
    }
    exitResidentInfos_.emplace_back(bundleName, uid);
    return true;
}

void ExitResidentProcessManager::RecordExitResidentBundleDependedOnWeb(const std::string &bundleName, int32_t uid)
{
    std::lock_guard<ffrt::mutex> lock(webMutexLock_);
    TAG_LOGE(AAFwkTag::APPMGR, "call");
    exitResidentBundlesDependedOnWeb_.emplace_back(bundleName, uid);
}

int32_t ExitResidentProcessManager::HandleMemorySizeInSufficent()
{
    std::lock_guard<ffrt::mutex> lock(mutexLock_);
    if (currentMemorySizeState_ != MemorySizeState::MEMORY_SIZE_SUFFICIENT) {
        TAG_LOGE(AAFwkTag::APPMGR, "memory size is insufficient");
        return AAFwk::ERR_NATIVE_MEMORY_SIZE_STATE_UNCHANGED;
    }
    currentMemorySizeState_ = MemorySizeState::MEMORY_SIZE_INSUFFICIENT;
    return ERR_OK;
}

int32_t ExitResidentProcessManager::HandleMemorySizeSufficient(std::vector<ExitResidentProcessInfo>& processInfos)
{
    std::lock_guard<ffrt::mutex> lock(mutexLock_);
    if (currentMemorySizeState_ == MemorySizeState::MEMORY_SIZE_SUFFICIENT) {
        TAG_LOGE(AAFwkTag::APPMGR, "memory size is sufficient");
        return AAFwk::ERR_NATIVE_MEMORY_SIZE_STATE_UNCHANGED;
    }
    currentMemorySizeState_ = MemorySizeState::MEMORY_SIZE_SUFFICIENT;
    processInfos = std::move(exitResidentInfos_);
    return ERR_OK;
}

void ExitResidentProcessManager::HandleExitResidentBundleDependedOnWeb(
    std::vector<ExitResidentProcessInfo> &bundleNames)
{
    std::lock_guard<ffrt::mutex> lock(webMutexLock_);
    TAG_LOGE(AAFwkTag::APPMGR, "call");
    bundleNames = std::move(exitResidentBundlesDependedOnWeb_);
}

void ExitResidentProcessManager::QueryExitBundleInfos(const std::vector<ExitResidentProcessInfo> &exitProcessInfos,
    std::vector<AppExecFwk::BundleInfo>& exitBundleInfos)
{
    AppExecFwk::BundleInfo bundleInfo;
    std::shared_ptr<RemoteClientManager> remoteClientManager = std::make_shared<RemoteClientManager>();
    if (remoteClientManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null remoteClientManager");
        return;
    }
    auto bundleMgrHelper = remoteClientManager->GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null bundleMgrHelper");
        return;
    }
    for (const auto &item: exitProcessInfos) {
        std::string bundleName;
        int32_t appIndex;
        if (IN_PROCESS_CALL(bundleMgrHelper->GetNameAndIndexForUid(item.uid, bundleName, appIndex))) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to get appIndex for %{public}s", item.bundleName.c_str());
            continue;
        }
        auto flags = static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION)
            | static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE)
            | static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_ABILITY)
            | static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_EXTENSION_ABILITY);
        if (IN_PROCESS_CALL(bundleMgrHelper->GetCloneBundleInfo(item.bundleName, flags, appIndex, bundleInfo,
            item.uid / BASE_USER_RANGE)) != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "fail from %{public}s", item.bundleName.c_str());
            continue;
        }
        if (!bundleInfo.isKeepAlive) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "not a resident application");
            continue;
        }
        exitBundleInfos.emplace_back(bundleInfo);
    }
}

bool ExitResidentProcessManager::IsKilledForUpgradeWeb(const std::string &bundleName) const
{
    TAG_LOGE(AAFwkTag::APPMGR, "call");
    std::vector<ExitResidentProcessInfo> bundleNames;
    {
        std::lock_guard<ffrt::mutex> lock(webMutexLock_);
        bundleNames = exitResidentBundlesDependedOnWeb_;
    }
    for (const auto &innerBundleName : bundleNames) {
        if (innerBundleName.bundleName == bundleName) {
            TAG_LOGD(AAFwkTag::APPMGR, "Is killed for upgrade web.");
            return true;
        }
    }
    TAG_LOGD(AAFwkTag::APPMGR, "Not killed for upgrade web.");
    return false;
}
}  // namespace AppExecFwk
}  // namespace OHOS
