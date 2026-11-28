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
#include "exit_resident_process_manager.h"
#include "mock_my_status.h"

namespace OHOS {
namespace AppExecFwk {

ExitResidentProcessManager::~ExitResidentProcessManager() {}

ExitResidentProcessManager::ExitResidentProcessManager() {}

ExitResidentProcessManager &ExitResidentProcessManager::GetInstance()
{
    static ExitResidentProcessManager instance;
    return instance;
}

bool ExitResidentProcessManager::IsMemorySizeSufficient() const
{
    return AAFwk::MyStatus::GetInstance().isMemorySizeSufficient_;
}

bool ExitResidentProcessManager::IsNoRequireBigMemory() const
{
    return false;
}

bool ExitResidentProcessManager::RecordExitResidentBundleName(const std::string &bundleName, int32_t uid)
{
    return false;
}

bool ExitResidentProcessManager::RecordExitResidentBundleNameOnRequireBigMemory(
    const std::string &bundleName, int32_t uid)
{
    return false;
}

void ExitResidentProcessManager::RecordExitResidentBundleDependedOnWeb(const std::string &bundleName, int32_t uid)
{
}

int32_t ExitResidentProcessManager::HandleMemorySizeInSufficent()
{
    return AAFwk::MyStatus::GetInstance().handleMemorySizeInSufficent_;
}

int32_t ExitResidentProcessManager::HandleRequireBigMemoryOptimization()
{
    return AAFwk::MyStatus::GetInstance().handleRequireBigMemoryOptimization_;
}

int32_t ExitResidentProcessManager::HandleMemorySizeSufficient(std::vector<ExitResidentProcessInfo>& processInfos)
{
    return ERR_OK;
}

void ExitResidentProcessManager::HandleExitResidentBundleDependedOnWeb(
    std::vector<ExitResidentProcessInfo> &bundleNames)
{
    // Set flag to indicate this method was called
    AAFwk::MyStatus::GetInstance().handleExitResidentBundleDependedOnWeb_called_ = true;
    bundleNames = AAFwk::MyStatus::GetInstance().mockBundleNames_;
    if (bundleNames.empty()) {
        AAFwk::MyStatus::GetInstance().restartResidentProcessDependedOnWeb_emptyBundleNames_called_ = true;
    }
}

int32_t ExitResidentProcessManager::HandleNoRequireBigMemoryOptimization (
    std::vector<ExitResidentProcessInfo> &processInfos)
{
    return ERR_OK;
}

void ExitResidentProcessManager::QueryExitBundleInfos(const std::vector<ExitResidentProcessInfo> &exitProcessInfos,
    std::vector<AppExecFwk::BundleInfo>& exitBundleInfos)
{
    AAFwk::MyStatus::GetInstance().queryExitBundleInfos_called_ = true;
}

bool ExitResidentProcessManager::IsKilledForUpgradeWeb(const std::string &bundleName) const
{
    return false;
}
}  // namespace AppExecFwk
}  // namespace OHOS
