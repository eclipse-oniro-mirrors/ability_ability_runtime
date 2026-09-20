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

#include "extensioncontrolinterceptorsecond_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <functional>
#include <iostream>

#define private public
#include "extension_control_interceptor.h"
#undef private

#include "ability_record.h"
#include "securec.h"
#include "want_params.h"

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
constexpr size_t MAX_STR_LEN = 256;
constexpr const char* STRICT_MODE = "strict_mode";
} // namespace

uint32_t GetU32Data(const char* ptr)
{
    if (ptr == nullptr) {
        return 0;
    }
    return (ptr[INPUT_ZERO] << OFFSET_ZERO) | (ptr[INPUT_ONE] << OFFSET_ONE) |
           (ptr[INPUT_TWO] << OFFSET_TWO) | ptr[INPUT_THREE];
}

Want BuildFuzzWant(const char* data, size_t size)
{
    Want want;
    if (data == nullptr || size == 0) {
        return want;
    }
    std::string bundleName(data, size > MAX_STR_LEN ? MAX_STR_LEN : size);
    std::string abilityName(data + INPUT_THREE, size > INPUT_THREE + MAX_STR_LEN ?
        MAX_STR_LEN : size - INPUT_THREE);
    std::string uri(data + INPUT_TWO, size > INPUT_TWO + MAX_STR_LEN ?
        MAX_STR_LEN : size - INPUT_TWO);
    
    ElementName element(bundleName, abilityName, "");
    want.SetElement(element);
    want.SetUri(uri);
    want.SetParam(STRICT_MODE, true);
    return want;
}

std::shared_ptr<AbilityInfo> BuildFuzzAbilityInfo(const char* data, size_t size)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    if (data == nullptr || size == 0 || abilityInfo == nullptr) {
        return abilityInfo;
    }
    uint32_t abilityTypeMax = static_cast<uint32_t>(AbilityType::EXTENSION);
    abilityTypeMax = (abilityTypeMax == 0) ? 1 : abilityTypeMax;
    abilityInfo->type = static_cast<AbilityType>(GetU32Data(data) % abilityTypeMax);

    uint32_t extTypeMax = static_cast<uint32_t>(ExtensionAbilityType::SERVICE);
    extTypeMax = (extTypeMax == 0) ? 1 : extTypeMax;
    abilityInfo->extensionAbilityType = static_cast<ExtensionAbilityType>(
        GetU32Data(data) % extTypeMax);
    abilityInfo->bundleName = "com.example.bundleName";
    abilityInfo->applicationInfo.isSystemApp = true;
    abilityInfo->extensionTypeName = "extensionTypeName";
    return abilityInfo;
}

sptr<Token> GetFuzzAbilityToken(const char* data, size_t size)
{
    sptr<Token> token = nullptr;
    if (data == nullptr || size == 0) {
        return token;
    }
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.fuzz";
    abilityRequest.abilityInfo.name = "abilityInfoName";
    uint32_t abilityTypeMax = static_cast<uint32_t>(AbilityType::EXTENSION);
    abilityTypeMax = (abilityTypeMax == 0) ? 1 : abilityTypeMax;
    abilityRequest.abilityInfo.type = static_cast<AbilityType>(GetU32Data(data) % abilityTypeMax);
    
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (abilityRecord) {
        token = abilityRecord->GetToken();
    }
    return token;
}

bool DoSomethingInterestingWithMyAPI(const char *data, size_t size)
{
    std::shared_ptr<ExtensionControlInterceptor> interceptor =
        std::make_shared<ExtensionControlInterceptor>();
    if (interceptor == nullptr) {
        return false;
    }
    int intParam = static_cast<int>(GetU32Data(data));
    int32_t int32Param = static_cast<int32_t>(GetU32Data(data));
    Want want = BuildFuzzWant(data, size);
    bool boolParam = (GetU32Data(data) % ENABLE) == 1;
    sptr<IRemoteObject> token = GetFuzzAbilityToken(data, size);
    static auto shouldBlockFunc = []() { return false; };
    auto abilityInfo = BuildFuzzAbilityInfo(data, size);
    bool isStartAsCaller = (GetU32Data(data) % 2) == 1;
    int32_t appIndex = static_cast<int32_t>(GetU32Data(data) % 100);

    AbilityInterceptorParam param1 =
        InterceptorParamBuilder(want, intParam, int32Param).WithUI(boolParam).CallerToken(token)
        .Context<AbilityInterceptorParam::EcologicalCtx>({false, true, ""}).Build();
    AbilityInterceptorParam param2 = InterceptorParamBuilder(want, intParam, int32Param).WithUI(boolParam)
        .CallerToken(token).AbilityInfo(abilityInfo)
        .Context<AbilityInterceptorParam::EcologicalCtx>({isStartAsCaller, false, ""})
        .Context<AbilityInterceptorParam::DisposedCtx>({appIndex, nullptr}).Build();
    
    (void)interceptor->DoProcess(param1);
    (void)interceptor->DoProcess(param2);
    
    AbilityInfo callerInfo = *BuildFuzzAbilityInfo(data, size);
    AbilityInfo targetInfo = *BuildFuzzAbilityInfo(data, size);
    (void)interceptor->ProcessInterceptOld(param1, targetInfo, callerInfo);
    (void)interceptor->ProcessInterceptNew(param1, targetInfo, callerInfo);
    
    AbilityInfo tempInfo;
    (void)interceptor->GetCallerAbilityInfo(param1, tempInfo);
    (void)interceptor->GetTargetAbilityInfo(param1, tempInfo);
    
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        std::cout << "invalid data" << std::endl;
        return 0;
    }

    if (size < OHOS::U32_AT_SIZE) {
        return 0;
    }

    char *ch = static_cast<char*>(malloc(size + 1));
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
