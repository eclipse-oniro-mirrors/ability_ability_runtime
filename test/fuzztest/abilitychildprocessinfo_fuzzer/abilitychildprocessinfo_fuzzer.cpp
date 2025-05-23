/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "abilitychildprocessinfo_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#define private public
#define protected public
#include "child_process_info.h"
#undef protected
#undef private
#include "parcel.h"
#include <iostream>
#include "securec.h"
#include "configuration.h"
using namespace OHOS::AppExecFwk;
namespace OHOS {
bool DoSomethingInterestingWithMyAPI(FuzzedDataProvider *fdp)
{
    std::shared_ptr<ChildProcessInfo>childProcessInfo = std::make_shared<ChildProcessInfo>();
    if (childProcessInfo == nullptr) {
        return false;
    }
    Parcel parcel;
    parcel.WriteString(fdp->ConsumeRandomLengthString());
    childProcessInfo->ReadFromParcel(parcel);
    childProcessInfo->Marshalling(parcel);
    ChildProcessInfo::Unmarshalling(parcel);
    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    FuzzedDataProvider fdp(data, size);
    OHOS::DoSomethingInterestingWithMyAPI(&fdp);
    return 0;
}

