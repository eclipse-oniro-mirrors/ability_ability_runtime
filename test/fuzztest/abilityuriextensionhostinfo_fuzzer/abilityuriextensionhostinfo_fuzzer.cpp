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
#include "abilityuriextensionhostinfo_fuzzer.h"

#include "ui_extension_host_info.h"

#include <fuzzer/FuzzedDataProvider.h>
#include <iostream>
#include "securec.h"

namespace OHOS {
using namespace OHOS::AbilityRuntime;

constexpr size_t STRING_MAX_LENGTH = 128;
bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    std::shared_ptr<UIExtensionHostInfo> extensionHostInfo = std::make_shared<UIExtensionHostInfo>();
    if (extensionHostInfo == nullptr) {
        return false;
    }
    Parcel parcel;
    parcel.WriteString(fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH));
    extensionHostInfo->ReadFromParcel(parcel);
    extensionHostInfo->Marshalling(parcel);
    extensionHostInfo.reset(extensionHostInfo->Unmarshalling(parcel));
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