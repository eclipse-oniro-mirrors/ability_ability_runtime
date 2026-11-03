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

#include "insightintentexecutioninfo_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#define private public
#include "insight_intent_execution_info.h"
#undef private

using namespace OHOS::AppExecFwk;

namespace OHOS {
bool DoSomethingInterestingWithMyAPI(FuzzedDataProvider *fdp)
{
    std::shared_ptr<InsightIntentExecutionInfo> info = std::make_shared<InsightIntentExecutionInfo>();
    Parcel parcel;
    parcel.WriteString(fdp->ConsumeRandomLengthString());
    parcel.WriteString(fdp->ConsumeRandomLengthString());
    parcel.WriteString(fdp->ConsumeRandomLengthString());
    parcel.WriteInt32(fdp->ConsumeIntegral<int32_t>());
    info->ReadFromParcel(parcel);
    info->Marshalling(parcel);
    InsightIntentExecutionInfo::Unmarshalling(parcel);
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    FuzzedDataProvider fdp(data, size);
    OHOS::DoSomethingInterestingWithMyAPI(&fdp);
    return 0;
}
