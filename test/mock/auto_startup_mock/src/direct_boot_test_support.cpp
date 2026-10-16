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
#include "direct_boot_test_support.h"

#include <unistd.h>

#include <algorithm>

#include "ability_auto_startup_data_manager.h"
#include "ability_auto_startup_service.h"
#include "app_utils.h"
#include "ipc_skeleton.h"
#include "os_account_manager_wrapper.h"
#include "parameters.h"
#include "token_setproc.h"

namespace OHOS::AAFwk {
DirectBootTestState &GetDirectBootTestState()
{
    static DirectBootTestState state;
    return state;
}

DistributedKv::Status DirectBootTestStore::GetEntries(const DistributedKv::Key &prefix,
    std::vector<DistributedKv::Entry> &result) const
{
    result = entries;
    return readResult;
}

DistributedKv::Status DirectBootTestStore::Put(const DistributedKv::Key &key, const DistributedKv::Value &value)
{
    ++writes;
    for (auto &entry : entries) {
        if (entry.key == key) {
            entry.value = value;
            return DistributedKv::Status::SUCCESS;
        }
    }
    entries.push_back({key, value});
    return DistributedKv::Status::SUCCESS;
}

DistributedKv::Status DirectBootTestStore::Delete(const DistributedKv::Key &key)
{
    entries.erase(std::remove_if(entries.begin(), entries.end(),
        [&key](const auto &entry) { return entry.key == key; }), entries.end());
    return DistributedKv::Status::SUCCESS;
}

} // namespace OHOS::AAFwk

// Test-only dependency replacements. The scoped context restores all configurable state.
namespace OHOS {
pid_t IPCSkeleton::GetCallingUid()
{
    const auto &state = AAFwk::GetDirectBootTestState();
    return state.active ? state.callingUid : getuid();
}

uint32_t IPCSkeleton::GetCallingTokenID()
{
    return AAFwk::GetDirectBootTestState().active ? AAFwk::DIRECT_BOOT_TEST_CALLER_TOKEN : GetSelfTokenID();
}
} // namespace OHOS

namespace OHOS::system {
std::string GetDeviceType()
{
    const auto &state = AAFwk::GetDirectBootTestState();
    return state.active ? state.deviceType : GetParameter("const.product.devicetype", "default");
}
} // namespace OHOS::system

namespace OHOS::AppExecFwk {
namespace {
constexpr int32_t SECOND_ACCOUNT_QUERY = 2;
}

ErrCode OsAccountManagerWrapper::GetOsAccountLocalIdFromUid(const int32_t uid, int32_t &id)
{
    auto &state = AAFwk::GetDirectBootTestState();
    if (!state.active) {
        id = uid / Constants::BASE_USER_RANGE;
        return ERR_OK;
    }
    ++state.accountCalls;
    if (state.accountCalls == state.accountFailureCall) {
        return -1;
    }
    id = state.overrideSecondUser && state.accountCalls == SECOND_ACCOUNT_QUERY ?
        state.accountSecondUser : uid / Constants::BASE_USER_RANGE;
    return ERR_OK;
}

} // namespace OHOS::AppExecFwk

namespace OHOS::AAFwk {
using namespace AbilityRuntime;

DirectBootServiceTestContext::DirectBootServiceTestContext()
{
    oldState_ = GetDirectBootTestState();
    GetDirectBootTestState() = {};
    GetDirectBootTestState().active = true;
    GetDirectBootTestState().parameters["const.abilityms.enable_auto_startup"] = true;
    oldSupportedLoaded_ = AppUtils::GetInstance().isAutoStartupSupported_.isLoaded;
    oldSupported_ = AppUtils::GetInstance().isAutoStartupSupported_.value;
    AppUtils::GetInstance().isAutoStartupSupported_.isLoaded = false;
    AppUtils::GetInstance().isAutoStartupSupported_.value = true;
    auto manager = DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance();
    oldStore_ = manager->kvStorePtr_;
    oldBackupTime_ = manager->lastBackupTime_;
    oldBackupScheduled_ = manager->backupFlushScheduled_;
    // Keep unrelated database backup work out of this scoped test context.
    manager->lastBackupTime_ = std::chrono::steady_clock::now();
    manager->backupFlushScheduled_ = true;
    store_ = std::make_shared<DirectBootTestStore>();
    manager->kvStorePtr_ = store_;
    service_ = std::make_shared<AbilityAutoStartupService>();
}

DirectBootServiceTestContext::~DirectBootServiceTestContext()
{
    service_.reset();
    auto manager = DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance();
    manager->kvStorePtr_ = oldStore_;
    manager->lastBackupTime_ = oldBackupTime_;
    manager->backupFlushScheduled_ = oldBackupScheduled_;
    AppUtils::GetInstance().isAutoStartupSupported_.isLoaded = oldSupportedLoaded_;
    AppUtils::GetInstance().isAutoStartupSupported_.value = oldSupported_;
    GetDirectBootTestState() = oldState_;
}

AutoStartupInfo DirectBootServiceTestContext::Target(const std::string &name, bool directBoot)
{
    AutoStartupInfo info;
    info.bundleName = name;
    info.moduleName = "entry";
    info.abilityName = "EntryAbility";
    info.abilityTypeName = ABILITY_TYPE_UI_ABILITY;
    info.userId = DIRECT_BOOT_TEST_USER;
    info.setterUserId = DIRECT_BOOT_TEST_USER;
    info.appCloneIndex = 0;
    info.isDirectBoot = directBoot;
    auto &targets = GetDirectBootTestState().targets;
    auto [it, inserted] = targets.try_emplace(name);
    if (inserted) {
        it->second.tokenId = DIRECT_BOOT_TEST_TOKEN_BASE + targets.size();
    }
    info.accessTokenId = std::to_string(it->second.tokenId);
    return info;
}

void DirectBootServiceTestContext::Register(const AutoStartupInfo &info, bool enabled)
{
    auto manager = DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance();
    store_->Put(manager->ConvertAutoStartupDataToKey(info),
        manager->ConvertAutoStartupStatusToValue(info, enabled, false));
}

} // namespace OHOS::AAFwk
