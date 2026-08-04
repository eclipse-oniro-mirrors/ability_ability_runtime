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

#include "keepaliveprocessmanagertwentysecond_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>
#include <string>
#include <vector>

#define private public
#include "keep_alive_process_manager.h"
#undef private

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LENGTH = 128;
// A second uid to drive both the found and not-found paths of the
// restart-after-upgrade lists.
constexpr int32_t SECOND_UID_OFFSET = 1;
} // namespace

void ExerciseRestartAfterUpgrade(FuzzedDataProvider &fdp, KeepAliveProcessManager &mgr)
{
    std::string bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    int32_t uid = fdp.ConsumeIntegral<int32_t>();

    mgr.keepAliveRestartAfterUpgradeList_.insert(uid);
    mgr.keepAliveRestartAfterUpgradeList_.insert(uid + SECOND_UID_OFFSET);

    mgr.KeepAliveIsRestartAfterUpdate(uid);
    mgr.KeepAliveCheckNeedRestartAfterUpgrade(uid);
    mgr.KeepAliveCheckNeedRestartAfterUpgrade(uid);

    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    mgr.StartKeepAliveAfterAppUpgrade(bundleInfos, uid + SECOND_UID_OFFSET);
    mgr.StartKeepAliveAfterAppUpgrade(bundleInfos, uid);

    mgr.SaveKeepAliveAppRestartAfterUpgrade(bundleName, uid);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    KeepAliveProcessManager &mgr = KeepAliveProcessManager::GetInstance();
    ExerciseRestartAfterUpgrade(fdp, mgr);
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return 0;
    }
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
