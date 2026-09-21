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

#include "extensioncontrolinterceptorfirst_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>

#define private public
#include "extension_control_interceptor.h"
#undef private

#include "ability_record.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
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

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    std::shared_ptr<ExtensionControlInterceptor> extensionControlInterceptor =
        std::make_shared<ExtensionControlInterceptor>();
    int intParam;
    int32_t int32Param;
    Want want;
    bool boolParam;
    sptr<IRemoteObject> token = GetFuzzAbilityToken();
    FuzzedDataProvider fdp(data, size);
    intParam = fdp.ConsumeIntegral<int>();
    int32Param = fdp.ConsumeIntegral<int32_t>();
    boolParam = fdp.ConsumeBool();
    AbilityInterceptorParam param =
        InterceptorParamBuilder(want, intParam, int32Param).WithUI(boolParam).CallerToken(token).Build();
    auto fuzzAbilityInfo = std::make_shared<AbilityInfo>();
    AbilityInterceptorParam param2 =
        InterceptorParamBuilder(want, intParam, int32Param).WithUI(boolParam).CallerToken(token)
        .AbilityInfo(fuzzAbilityInfo).Build();
    AbilityInfo targetAbilityInfo;
    AbilityInfo callerAbilityInfo;
    extensionControlInterceptor->DoProcess(param);
    extensionControlInterceptor->DoProcess(param2);
    extensionControlInterceptor->ProcessInterceptOld(param, targetAbilityInfo, callerAbilityInfo);
    extensionControlInterceptor->ProcessInterceptNew(param, targetAbilityInfo, callerAbilityInfo);
    extensionControlInterceptor->GetCallerAbilityInfo(param, callerAbilityInfo);
    extensionControlInterceptor->GetTargetAbilityInfo(param, targetAbilityInfo);
    extensionControlInterceptor->GetTargetAbilityInfo(param2, targetAbilityInfo);
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
