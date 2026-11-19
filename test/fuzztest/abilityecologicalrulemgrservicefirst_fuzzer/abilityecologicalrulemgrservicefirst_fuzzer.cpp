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

#include "abilityecologicalrulemgrservicefirst_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#define private public
#include "ability_ecological_rule_mgr_service.h"
#undef private

#include "ability_fuzz_util.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LENGTH = 128;
}
bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    auto client = std::make_shared<AbilityEcologicalRuleMgrServiceClient>();
    wptr<IRemoteObject> object;
    Want want;
    AbilityCallerInfo callerInfo;
    int32_t type;
    vector<AbilityInfo> abilityInfos;
    AbilityInfo info;
    AbilityExperienceRule rule;
    FuzzedDataProvider fdp(data, size);
    client->ConnectService();
    client->CheckConnectService();
    client->OnRemoteSaDied(object);
    type = fdp.ConsumeIntegral<int32_t>();
    size_t arraySize = fdp.ConsumeIntegralInRange<size_t>(0, STRING_MAX_LENGTH);
    for (size_t i = 0; i < arraySize; ++i) {
        AbilityFuzzUtil::GetRandomAbilityInfo(fdp, info);
        abilityInfos.emplace_back(info);
    }
    client->EvaluateResolveInfos(want, callerInfo, type, abilityInfos);
    AbilityFuzzUtil::GetRandomAbilityExperienceRule(fdp, rule);
    client->QueryStartExperience(want, callerInfo, rule);

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