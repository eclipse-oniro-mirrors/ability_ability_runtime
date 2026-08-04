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

#include "uriutilsfirst_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>
#include <string>
#include <vector>

#define private public
#include "uri_utils.h"
#undef private

#include "uri.h"

using namespace OHOS::AAFwk;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LENGTH = 128;
constexpr size_t MIN_URI_COUNT = 1;
constexpr size_t MAX_URI_COUNT = 16;
constexpr int32_t DEFAULT_PERMISSION_TYPE = 0;
} // namespace

void ExercisePermissionedUriList(FuzzedDataProvider &fdp, UriUtils &utils,
    const std::vector<std::string> &uriVec, std::vector<CheckResult> &checkResults, Want &want)
{
    uint32_t callerTokenId = fdp.ConsumeIntegral<uint32_t>();
    std::string targetBundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    utils.GetPermissionedUriList(uriVec, checkResults, callerTokenId, targetBundleName, want);
    std::vector<CheckResult> mismatchResults = { CheckResult(true, DEFAULT_PERMISSION_TYPE) };
    utils.GetPermissionedUriList(uriVec, mismatchResults, callerTokenId, targetBundleName, want);

    uint32_t flag = fdp.ConsumeIntegral<uint32_t>();
    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    bool isNotify = fdp.ConsumeBool();
    utils.NotifyGrantUriPermissionStart(isNotify, uriVec, flag, userId);
    std::vector<bool> boolResults;
    for (const auto &result : checkResults) {
        boolResults.push_back(result.result);
    }
    utils.NotifyGrantUriPermissionEnd(isNotify, uriVec, flag, userId, boolResults);
    utils.MarkContentUriAuthorizedForBroker(uriVec, checkResults, fdp.ConsumeBool());
}

#ifdef SUPPORT_UPMS
void ExerciseGrantUriPermissionInner(FuzzedDataProvider &fdp, UriUtils &utils,
    const std::vector<std::string> &uriVec, Want &want)
{
    GrantUriPermissionInfo grantInfo;
    grantInfo.targetBundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    grantInfo.callerTokenId = fdp.ConsumeIntegral<uint32_t>();
    grantInfo.flag = fdp.ConsumeIntegral<uint32_t>();
    grantInfo.appIndex = fdp.ConsumeIntegral<int32_t>();
    grantInfo.userId = fdp.ConsumeIntegral<int32_t>();
    grantInfo.collaboratorType = fdp.ConsumeIntegral<int32_t>();
    grantInfo.isSandboxApp = fdp.ConsumeBool();
    grantInfo.isNotifyCollaborator = fdp.ConsumeBool();
    utils.GrantUriPermissionInner(uriVec, grantInfo, want, fdp.ConsumeBool());
}
#endif

bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    // Keep uriVec and checkResults non-empty: GetPermissionedUriList reads
    // checkResults[0] unconditionally after the size check (uri_utils.cpp),
    // so an empty vector would hit an out-of-bounds read in the target code.
    uint32_t uriCount = fdp.ConsumeIntegralInRange<uint32_t>(MIN_URI_COUNT, MAX_URI_COUNT);
    std::vector<std::string> uriVec;
    for (uint32_t i = 0; i < uriCount; i++) {
        uriVec.emplace_back(fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH));
    }
    std::vector<CheckResult> checkResults;
    for (uint32_t i = 0; i < uriCount; i++) {
        checkResults.emplace_back(fdp.ConsumeBool(), fdp.ConsumeIntegral<int32_t>());
    }

    Want want;
    want.SetUri(Uri(fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH)));

    UriUtils &utils = UriUtils::GetInstance();
    ExercisePermissionedUriList(fdp, utils, uriVec, checkResults, want);
#ifdef SUPPORT_UPMS
    ExerciseGrantUriPermissionInner(fdp, utils, uriVec, want);
#endif
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
