/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "abilitymgrappexitreasonhelper_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#define private public
#include "app_exit_reason_helper.h"
#include "ability_manager_service.h"
#undef private

#include "ability_fuzz_util.h"
#include "ability_record.h"
#include "securec.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LENGTH = 128;
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    std::shared_ptr<SubManagersHelper> subManagersHelper;
    std::shared_ptr<AppExitReasonHelper> infos = std::make_shared<AppExitReasonHelper>(subManagersHelper);
    ExitReason exitReason;
    RunningProcessInfo processInfo;
    std::vector<std::string> abilities;
    std::vector<std::string> abilityLists;
    std::vector<std::string> abilityList;
    int32_t pid;
    int32_t callerPid;
    std::string bundleName;
    std::string abilityName;
    int32_t uid;
    int32_t appIndex;
    uint32_t accessTokenId;
    int32_t userId;
    bool withKillMsg;
    bool fromKillWithReason;
    FuzzedDataProvider fdp(data, size);
    pid = fdp.ConsumeIntegral<int32_t>();
    callerPid = fdp.ConsumeIntegral<int32_t>();
    fromKillWithReason = fdp.ConsumeBool();
    bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    abilityName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    uid = fdp.ConsumeIntegral<int32_t>();
    appIndex = fdp.ConsumeIntegral<int32_t>();
    accessTokenId = fdp.ConsumeIntegral<int32_t>();
    userId = fdp.ConsumeIntegral<int32_t>();
    withKillMsg = fdp.ConsumeBool();
    abilities = AbilityFuzzUtil::GenerateStringArray(fdp);
    abilityLists = AbilityFuzzUtil::GenerateStringArray(fdp);
    abilityList = AbilityFuzzUtil::GenerateStringArray(fdp);
    infos->RecordAppExitReason(exitReason);
    infos->RecordProcessExitReason(pid, exitReason, fromKillWithReason);
    infos->RecordAppExitReason(bundleName, uid, appIndex, exitReason);
    infos->RecordProcessExitReason(pid, uid, exitReason);
    infos->RecordProcessExitReason(pid, bundleName, uid, accessTokenId,
        exitReason, processInfo, fromKillWithReason);
    infos->RecordProcessExtensionExitReason(pid, bundleName, exitReason, processInfo, withKillMsg);
    infos->GetActiveAbilityList(uid, abilityLists, pid);
    infos->GetActiveAbilityListFromUIAbilityManager(uid, abilityLists, pid);
    infos->IsExitReasonValid(exitReason);
    infos->GetActiveAbilityListWithPid(uid, abilityList, pid);
    infos->RecordUIAbilityExitReason(pid, abilityName, exitReason);
    infos->RecordUIAbilityExitReason(pid, abilityName, exitReason, callerPid);
    infos->RecordKillCallerInfo(pid, callerPid);
    infos->GetRunningProcessInfo(pid, userId, bundleName, processInfo);
    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // Run your code on data.
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
