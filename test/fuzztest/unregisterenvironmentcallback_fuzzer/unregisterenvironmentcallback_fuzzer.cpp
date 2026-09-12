/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "unregisterenvironmentcallback_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "environment_callback.h"
#include "application_context.h"
#include "securec.h"
#include <fuzzer/FuzzedDataProvider.h>

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace {
constexpr size_t U32_AT_SIZE = 4;
}
class EnvironmentCallbackFuzz : public EnvironmentCallback {
public:
    explicit EnvironmentCallbackFuzz() {};
    void OnConfigurationUpdated(const AppExecFwk::Configuration& config) override {};
    void OnMemoryLevel(const int level) override {};
};
bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    
    auto context = ApplicationContext::GetInstance();
    if (!context) {
        return false;
    }

    std::shared_ptr<EnvironmentCallbackFuzz> callback;
    FuzzedDataProvider fdp(data, size);
    auto fuzzStr = fdp.ConsumeRandomLengthString(64);
    auto fuzzInt = fdp.ConsumeIntegral<int32_t>();
    auto fuzzBool = fdp.ConsumeBool();
    auto fuzzBuf = fdp.ConsumeRemainingBytesAsString();
    context->UnregisterEnvironmentCallback(callback);

    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        std::cout << "invalid data" << std::endl;
        return 0;
    }

    /* Validate the length of size */
    if (size < OHOS::U32_AT_SIZE) {
        return 0;
    }

    char* ch = static_cast<char*>(malloc(size + 1));
    if (ch == nullptr) {
        std::cout << "malloc failed." << std::endl;
        return 0;
    }

    (void)memset_s(ch, size + 1, 0x00, size + 1);
    if (memcpy_s(ch, size, data, size) != EOK) {
        std::cout << "copy failed." << std::endl;
        free(ch);
        ch = nullptr;
        return 0;
    }

    OHOS::DoSomethingInterestingWithMyAPI(ch, size);
    free(ch);
    ch = nullptr;
    return 0;
}