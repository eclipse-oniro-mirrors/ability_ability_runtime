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

#include "abilitymgrecologicalruleinterceptor_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#include "ecological_rule_interceptor.h"
#undef private

#include "ability_ecological_rule_mgr_service_param.h"
#include "ability_record.h"
#include "securec.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr int INPUT_ZERO = 0;
constexpr int INPUT_ONE = 1;
constexpr int INPUT_TWO = 2;
constexpr int INPUT_THREE = 3;
constexpr size_t U32_AT_SIZE = 4;
constexpr size_t OFFSET_ZERO = 24;
constexpr size_t OFFSET_ONE = 16;
constexpr size_t OFFSET_TWO = 8;
constexpr uint8_t ENABLE = 2;
}  // namespace

uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[INPUT_ZERO] << OFFSET_ZERO) | (ptr[INPUT_ONE] << OFFSET_ONE) |
           (ptr[INPUT_TWO] << OFFSET_TWO) | ptr[INPUT_THREE];
}

sptr<Token> GetFuzzAbilityToken()
{
    sptr<Token> token = nullptr;
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.fuzzTest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord =
        AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (abilityRecord) {
        token = abilityRecord->GetToken();
    }
    return token;
}

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    std::shared_ptr<EcologicalRuleInterceptor> executer =
        std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    int requestCode = static_cast<int>(GetU32Data(data));
    int32_t userId = static_cast<int32_t>(GetU32Data(data));
    bool isWithUI = *data % ENABLE;
    sptr<IRemoteObject> token = GetFuzzAbilityToken();
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param =
        AbilityInterceptorParam(want, requestCode, userId, isWithUI, token, shouldBlockFunc);
    const std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo;
    AbilityCallerInfo callerInfo;
    AtomicServiceStartupRule rule;
    sptr<Want> replaceWant;
    int32_t bundleType = static_cast<int32_t>(GetU32Data(data));
    executer->DoProcess(param);
    executer->DoProcess(want, userId);
    executer->GetEcologicalTargetInfo(want, abilityInfo, callerInfo);
    executer->GetEcologicalCallerInfo(want, callerInfo, userId, token);
    executer->InitErmsCallerInfo(want, abilityInfo, callerInfo, userId, token);
    executer->GetAppTypeByBundleType(bundleType);
    executer->QueryAtomicServiceStartupRule(want, token, userId, rule, replaceWant);
    return true;
}
}  // namespace OHOS

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
    if (memcpy_s(ch, size + 1, data, size) != EOK) {
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