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
#ifndef OHOS_ABILITY_DIRECT_BOOT_TEST_SUPPORT_H
#define OHOS_ABILITY_DIRECT_BOOT_TEST_SUPPORT_H

#include <chrono>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "auto_startup_info.h"
#include "bundle_constants.h"
#include "bundle_info.h"
#include "mock_single_kv_store.h"

namespace OHOS::AbilityRuntime {
class AbilityAutoStartupService;
}

namespace OHOS::AAFwk {
constexpr int32_t DIRECT_BOOT_TEST_USER = 100;
constexpr int32_t DIRECT_BOOT_TEST_UID = 20000000;
constexpr uint32_t DIRECT_BOOT_TEST_CALLER_TOKEN = 9000;
constexpr uint32_t DIRECT_BOOT_TEST_TOKEN_BASE = 1000;

struct DirectBootTestTarget {
    uint32_t tokenId = DIRECT_BOOT_TEST_TOKEN_BASE;
    int32_t appIndex = 0;
    int32_t userId = DIRECT_BOOT_TEST_USER;
    bool allowDirectBoot = true;
    bool allowWindow = true;
};

struct DirectBootTestState {
    bool active = false;
    std::map<std::string, bool> parameters;
    std::map<std::string, DirectBootTestTarget> targets;
    bool ready = true;
    bool systemCaller = true;
    bool callerPermission = true;
    bool edmCallerPermission = false;
    bool bundleAvailable = true;
    std::string deviceType = "2in1";
    int32_t callingUid = DIRECT_BOOT_TEST_UID;
    int32_t accountCalls = 0;
    int32_t accountFailureCall = 0;
    int32_t accountSecondUser = DIRECT_BOOT_TEST_USER;
    bool overrideSecondUser = false;
    int32_t tokenQueries = 0;
    int32_t permissionQueries = 0;
    int32_t windowPermissionQueries = 0;
    int32_t queriedUser = -1;
    int32_t queriedClone = -1;
    std::string queriedBundle;
};

DirectBootTestState &GetDirectBootTestState();
bool GetDirectBootBundleInfo(const std::string &bundleName, AppExecFwk::BundleInfo &bundleInfo, int32_t userId);

class DirectBootTestStore : public MockSingleKvStore {
public:
    DistributedKv::Status GetEntries(const DistributedKv::Key &prefix,
        std::vector<DistributedKv::Entry> &entries) const override;
    DistributedKv::Status Put(const DistributedKv::Key &key, const DistributedKv::Value &value) override;
    DistributedKv::Status Delete(const DistributedKv::Key &key) override;

    std::vector<DistributedKv::Entry> entries;
    DistributedKv::Status readResult = DistributedKv::Status::SUCCESS;
    int32_t writes = 0;
};

class DirectBootServiceTestContext {
public:
    DirectBootServiceTestContext();
    ~DirectBootServiceTestContext();
    DirectBootServiceTestContext(const DirectBootServiceTestContext &) = delete;
    DirectBootServiceTestContext &operator=(const DirectBootServiceTestContext &) = delete;
    AbilityRuntime::AutoStartupInfo Target(const std::string &name, bool directBoot = true);
    void Register(const AbilityRuntime::AutoStartupInfo &info, bool enabled = true);

    std::shared_ptr<DirectBootTestStore> store_;
    std::shared_ptr<AbilityRuntime::AbilityAutoStartupService> service_;

private:
    std::shared_ptr<DistributedKv::SingleKvStore> oldStore_;
    std::chrono::steady_clock::time_point oldBackupTime_;
    bool oldBackupScheduled_ = false;
    bool oldSupportedLoaded_ = false;
    bool oldSupported_ = false;
    DirectBootTestState oldState_;
};
} // namespace OHOS::AAFwk
#endif
