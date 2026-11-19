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

#include "insightintentexecutemanagersecond_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#define private public
#include "insight_intent_execute_manager.h"
#include "insight_intent_execute_param.h"
#undef private

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LENGTH = 128;
}
bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    uint64_t intentId;
    std::string callerBundleName;
    int32_t uid;
    int32_t pid;
    FuzzedDataProvider fdp(data, size);
    intentId = fdp.ConsumeIntegral<uint64_t>();
    callerBundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    uid = fdp.ConsumeIntegral<int32_t>();
    pid = fdp.ConsumeIntegral<int32_t>();
    DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->GetCallerBundleName(intentId, callerBundleName);
    DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->SetIntentExemptionInfo(uid);
    DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->CheckIntentIsExemption(uid);
    DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->CheckGetInsightIntenInfoPermission();
    DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->GetAllIntentExemptionInfo();
    DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->OnInsightAppDied(callerBundleName, pid);

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