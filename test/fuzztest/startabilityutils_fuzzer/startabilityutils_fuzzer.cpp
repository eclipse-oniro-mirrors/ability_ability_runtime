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

#include "startabilityutils_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#define protected public
#include "start_ability_utils.h"
#undef protected
#undef private

#include "ability_record.h"
#include <fuzzer/FuzzedDataProvider.h>

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace {
constexpr int INPUT_ZERO = 0;
constexpr int INPUT_ONE = 1;
constexpr int INPUT_THREE = 3;
constexpr size_t U32_AT_SIZE = 4;
constexpr uint8_t ENABLE = 2;
constexpr size_t OFFSET_ZERO = 24;
constexpr size_t OFFSET_ONE = 16;
constexpr size_t OFFSET_TWO = 8;
}

uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[INPUT_ZERO] << OFFSET_ZERO) | (ptr[INPUT_ONE] << OFFSET_ONE) | (ptr[ENABLE] << OFFSET_TWO) |
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

void StartAbilityUtilsFuzztest1(bool boolParam, std::string &stringParam, int32_t int32Param, int32_t userId)
{
    Want want;
    sptr<Token> callerToken = GetFuzzAbilityToken();
    StartAbilityUtils::GetAppIndex(want, callerToken, int32Param);
    AppExecFwk::ApplicationInfo appInfo;
    StartAbilityUtils::GetApplicationInfo(stringParam, int32Param, appInfo);
    AppExecFwk::AbilityInfo abilityInfo;
    StartAbilityUtils::GetCallerAbilityInfo(nullptr, abilityInfo);
    StartAbilityUtils::GetCallerAbilityInfo(callerToken, abilityInfo);
    StartAbilityUtils::CheckAppProvisionMode(want, int32Param, callerToken);
    std::shared_ptr<StartAbilityInfoWrap>  startAbilityInfoWrap =
        std::make_shared<StartAbilityInfoWrap>(want, int32Param, int32Param, callerToken, boolParam);
    StartAbilityInfo::CreateStartAbilityInfo(want, int32Param, int32Param, nullptr);
    StartAbilityInfo::CreateCallerAbilityInfo(nullptr);
    StartAbilityInfo::CreateCallerAbilityInfo(callerToken);
    StartAbilityUtils::GetCloneAppIndexes(stringParam, int32Param);
    StartAbilityUtils::CheckAppProvisionMode(want, userId, callerToken);
}

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    bool boolParam = *data % ENABLE;
    std::string stringParam(data, size);
    int32_t int32Param = static_cast<int32_t>(GetU32Data(data));
    int32_t userId = static_cast<int32_t>(GetU32Data(data));
    StartAbilityUtilsFuzztest1(boolParam, stringParam, int32Param, userId);
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

