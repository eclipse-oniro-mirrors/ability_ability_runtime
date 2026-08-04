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

#include "abilitymgrappexitreasonhelpersecond_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>
#include <string>
#include <vector>

#define private public
#include "app_exit_reason_helper.h"
#include "ability_manager_service.h"
#undef private

#include "ability_record.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LENGTH = 128;
// Covers valid Reason values (0..REASON_MAX=11) and invalid values up to 20,
// so both the valid and invalid branches of IsExitReasonValid are exercised.
constexpr int32_t REASON_VALUE_MAX = 20;

struct ExitReasonFuzzParam {
    int32_t pid = 0;
    int32_t uid = 0;
    int32_t userId = 0;
    int32_t appIndex = 0;
    uint32_t accessTokenId = 0;
    std::string bundleName;
    std::string abilityName;
};
} // namespace

ExitReasonCompability MakeExitReason(FuzzedDataProvider &fdp)
{
    ExitReasonCompability exitReason;
    exitReason.reason = static_cast<Reason>(fdp.ConsumeIntegralInRange<int32_t>(0, REASON_VALUE_MAX));
    exitReason.subReason = fdp.ConsumeIntegral<int32_t>();
    exitReason.killId = fdp.ConsumeIntegral<int32_t>();
    exitReason.exitMsg = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    exitReason.killMsg = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    exitReason.innerMsg = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    return exitReason;
}

void ExerciseRecordPaths(FuzzedDataProvider &fdp, const std::shared_ptr<AppExitReasonHelper> &helper,
    const ExitReasonFuzzParam &param, const ExitReasonCompability &exitReason,
    const AppExecFwk::RunningProcessInfo &processInfo)
{
    std::vector<AppExecFwk::RunningProcessInfo> processInfoList;
    processInfoList.emplace_back(processInfo);
    AppExecFwk::RunningProcessInfo invalidInfo;
    processInfoList.emplace_back(invalidInfo);

    helper->RecordAppWithReason(param.pid, param.uid, exitReason);
    helper->RecordAppsWithReasonByProcessInfoList(exitReason, processInfoList);
    helper->RecordInvalidKillId(param.pid, exitReason, param.bundleName, param.userId);
    helper->RecordInvalidKillId(param.pid, exitReason);

    std::vector<std::string> abilityList;
    abilityList.emplace_back(param.abilityName);
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.bundleName = param.bundleName;
    abilityInfo.name = param.abilityName;
    ExitReason exitReasonValue(static_cast<Reason>(fdp.ConsumeIntegralInRange<int32_t>(0, REASON_VALUE_MAX)),
        fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH));
    helper->RecordProcessExitReasonForTimeout(abilityInfo, exitReasonValue, abilityList, processInfo);
}

void ExerciseAddPaths(FuzzedDataProvider &fdp, const std::shared_ptr<AppExitReasonHelper> &helper,
    const ExitReasonFuzzParam &param, const ExitReasonCompability &exitReason,
    const AppExecFwk::RunningProcessInfo &processInfo)
{
    RecordExitReasonParams params;
    params.pid = param.pid;
    params.bundleName = param.bundleName;
    params.uid = param.uid;
    params.accessTokenId = param.accessTokenId;
    params.exitReason = exitReason;
    params.processInfo = processInfo;
    params.fromKillWithReason = fdp.ConsumeBool();
    helper->AddProcessExitReason(params);

    helper->AddAppExitReason(param.bundleName, param.pid, param.uid, param.appIndex, exitReason);
    helper->AddBundleExitReason(param.bundleName, param.userId, param.appIndex, exitReason);

    std::vector<AppExecFwk::RunningProcessInfo> infos = helper->GetRunningProcessInfos(param.userId,
        param.bundleName);
    (void)infos;
    helper->RecordAppWithReasonInner(exitReason, processInfo);
    AppExecFwk::RunningProcessInfo emptyInfo;
    helper->RecordAppWithReasonInner(exitReason, emptyInfo);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    std::shared_ptr<SubManagersHelper> subManagersHelper;
    std::shared_ptr<AppExitReasonHelper> helper = std::make_shared<AppExitReasonHelper>(subManagersHelper);
    ExitReasonFuzzParam param;
    param.pid = fdp.ConsumeIntegral<int32_t>();
    param.uid = fdp.ConsumeIntegral<int32_t>();
    param.userId = fdp.ConsumeIntegral<int32_t>();
    param.appIndex = fdp.ConsumeIntegral<int32_t>();
    param.accessTokenId = fdp.ConsumeIntegral<uint32_t>();
    param.bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    param.abilityName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);

    ExitReasonCompability exitReason = MakeExitReason(fdp);

    AppExecFwk::RunningProcessInfo processInfo;
    processInfo.pid_ = param.pid;
    processInfo.uid_ = param.uid;
    processInfo.accessTokenId_ = param.accessTokenId;
    processInfo.bundleNames.emplace_back(param.bundleName);

    ExerciseRecordPaths(fdp, helper, param, exitReason, processInfo);
    ExerciseAddPaths(fdp, helper, param, exitReason, processInfo);
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
