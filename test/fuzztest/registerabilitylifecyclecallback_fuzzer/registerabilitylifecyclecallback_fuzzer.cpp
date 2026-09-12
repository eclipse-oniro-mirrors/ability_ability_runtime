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

#include "registerabilitylifecyclecallback_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "ability_lifecycle_callback.h"
#include "application_context.h"
#include "securec.h"
#include <fuzzer/FuzzedDataProvider.h>

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace {
constexpr size_t U32_AT_SIZE = 4;
} // namespace
class AbilityLifecycleCallbackFuzz : public AbilityLifecycleCallback {
public:
    explicit AbilityLifecycleCallbackFuzz(){};
    virtual ~AbilityLifecycleCallbackFuzz(){};
    void
    OnAbilityCreate(const AbilityLifecycleCallbackArgs &ability) override{};
    void OnWindowStageCreate(
        const AbilityLifecycleCallbackArgs &ability,
        const AbilityLifecycleCallbackArgs &windowStage) override{};
    void OnWindowStageDestroy(
        const AbilityLifecycleCallbackArgs &ability,
        const AbilityLifecycleCallbackArgs &windowStage) override{};
    void OnWindowStageActive(
        const AbilityLifecycleCallbackArgs &ability,
        const AbilityLifecycleCallbackArgs &windowStage) override{};
    void OnWindowStageInactive(
        const AbilityLifecycleCallbackArgs &ability,
        const AbilityLifecycleCallbackArgs &windowStage) override{};
    void
    OnAbilityDestroy(const AbilityLifecycleCallbackArgs &ability) override{};
    void OnAbilityForeground(
        const AbilityLifecycleCallbackArgs &ability) override{};
    void OnAbilityBackground(
        const AbilityLifecycleCallbackArgs &ability) override{};
    void
    OnAbilityContinue(const AbilityLifecycleCallbackArgs &ability) override{};
};
bool DoSomethingInterestingWithMyAPI(const char *data, size_t size)
{
    
    auto context = ApplicationContext::GetInstance();
    if (!context) {
        return false;
    }
    std::shared_ptr<AbilityLifecycleCallbackFuzz> callback;
    FuzzedDataProvider fdp(data, size);
    auto fuzzStr = fdp.ConsumeRandomLengthString(64);
    auto fuzzInt = fdp.ConsumeIntegral<int32_t>();
    auto fuzzBool = fdp.ConsumeBool();
    auto fuzzBuf = fdp.ConsumeRemainingBytesAsString();
    context->RegisterAbilityLifecycleCallback(callback);
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
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
    char *ch = static_cast<char*>(malloc(size + 1));
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