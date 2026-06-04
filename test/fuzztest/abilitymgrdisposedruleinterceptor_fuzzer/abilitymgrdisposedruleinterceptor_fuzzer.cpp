/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#include "abilitymgrdisposedruleinterceptor_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "ability_fuzz_util.h"
#include "ability_record.h"
#include "securec.h"

#define private public
#include "disposed_rule_interceptor.h"
#undef private

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

#define DISABLE_FUZZ
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
}

uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[INPUT_ZERO] << OFFSET_ZERO) | (ptr[INPUT_ONE] << OFFSET_ONE) | (ptr[INPUT_TWO] << OFFSET_TWO) |
        ptr[INPUT_THREE];
}

sptr<Token> GetFuzzAbilityToken()
{
    sptr<Token> token = nullptr;
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.fuzzTest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (abilityRecord) {
        token = abilityRecord->GetToken();
    }
    return token;
}

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size, const uint8_t* rawData)
{
    FuzzedDataProvider fdp(rawData, size);
    std::shared_ptr<DisposedRuleInterceptor> executer = std::make_shared<DisposedRuleInterceptor>();
    Want want;
    AppExecFwk::DisposedRule disposedRule;
    int requestCode = static_cast<int>(GetU32Data(data));
    int32_t userId = static_cast<int32_t>(GetU32Data(data));
    bool isWithUI = *data % ENABLE;
    sptr<IRemoteObject> token = GetFuzzAbilityToken();
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, isWithUI, token,
        shouldBlockFunc);
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo;
    int32_t bundleType = static_cast<int32_t>(GetU32Data(data));
    std::vector<AppExecFwk::DisposedRule> disposedRules = AbilityFuzzUtil::GetRandomDisposedRulesList(fdp);

    executer-> DoProcess(param);
    executer-> CheckControl(want, userId, disposedRule, 0);
    executer-> FindBlockDisposedRule(want, disposedRules, disposedRule);
    executer-> FindNonBlockDisposedRule(disposedRules, disposedRule);
    int32_t uid = static_cast<int32_t>(GetU32Data(data));
    abilityInfo->uid = uid;
    executer-> StartNonBlockRule(want, disposedRule, abilityInfo);
    executer-> UnregisterObserver(uid);
    executer-> CreateModalUIExtension(want, token);
    executer-> SetInterceptInfo(want, disposedRule);

    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
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

#ifndef DISABLE_FUZZ
    OHOS::DoSomethingInterestingWithMyAPI(ch, size, data);
#endif
    free(ch);
    ch = nullptr;
    return 0;
}

