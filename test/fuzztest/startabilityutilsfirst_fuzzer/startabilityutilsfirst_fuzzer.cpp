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

#include "startabilityutilsfirst_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include <iostream>

#define private public
#define protected public
#include "start_ability_utils.h"
#undef protected
#undef private

#include "ability_record.h"
#include "securec.h"
#include "want_params.h"
#include "ability_util.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace {
constexpr int INPUT_ZERO = 0;
constexpr int INPUT_ONE = 1;
constexpr int INPUT_TWO = 2;
constexpr int INPUT_THREE = 3;
constexpr size_t U32_AT_SIZE = 4;
constexpr uint8_t ENABLE = 2;
constexpr size_t OFFSET_ZERO = 24;
constexpr size_t OFFSET_ONE = 16;
constexpr size_t OFFSET_TWO = 8;
constexpr int32_t MAX_APP_CLONE_INDEX = 10;
constexpr int32_t APP_PROVISION_TYPE_DEBUG = 0;
constexpr size_t MAX_STR_LEN = 256;
constexpr size_t TEST_APPINDEX = 1;
constexpr size_t Max_Status = 3;
constexpr size_t MAX_FUZZ_INPUT_SIZE = 4096;
}

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
    
    ElementName element(bundleName, abilityName, "entry");
    want.SetElement(element);
    want.SetBundle(bundleName);
    int32_t appIndex = static_cast<int32_t>(GetU32Data(data) % (MAX_APP_CLONE_INDEX));
    want.SetParam(Want::PARAM_APP_CLONE_INDEX_KEY, appIndex);
    want.SetParam("isPlugin", true);
    return want;
}

std::shared_ptr<StartAbilityInfo> BuildFuzzStartAbilityInfo(const char* data, size_t size)
{
    auto info = std::make_shared<StartAbilityInfo>();
    if (data == nullptr || size == 0 || info == nullptr) {
        return info;
    }
    info->abilityInfo.bundleName = std::string(data, size % MAX_STR_LEN);
    info->abilityInfo.name = std::string(data + INPUT_ONE, (size - INPUT_ONE) % MAX_STR_LEN);
    info->abilityInfo.applicationInfo.appProvisionType = static_cast<int32_t>(GetU32Data(data));
    info->abilityInfo.applicationInfo.appIndex = static_cast<int32_t>(GetU32Data(data) % MAX_APP_CLONE_INDEX);
    info->customProcess = std::string(data + INPUT_ONE, (size - INPUT_ONE) % MAX_STR_LEN);
    return info;
}

AbilityInfo BuildFuzzAbilityInfo(const char* data, size_t size)
{
    AbilityInfo info;
    if (data == nullptr || size == 0) {
        return info;
    }
    info.bundleName = std::string(data, size % MAX_STR_LEN);
    info.name = std::string(data + INPUT_THREE, size % MAX_STR_LEN);
    info.allowSelfRedirect = true;
    info.linkType = static_cast<LinkType>(GetU32Data(data));
    info.applicationInfo.uid = static_cast<int32_t>(GetU32Data(data));
    return info;
}

sptr<Token> GetFuzzAbilityToken()
{
    sptr<Token> token = nullptr;
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.fuzzTest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    abilityRequest.abilityInfo.applicationInfo.appIndex = TEST_APPINDEX;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (abilityRecord) {
        token = abilityRecord->GetToken();
    }
    return token;
}

void StartAbilityUtilsFuzztest1(bool boolParam, std::string &stringParam, int32_t int32Param, int32_t userId)
{
    Want want = BuildFuzzWant(stringParam.c_str(), stringParam.size());
    sptr<Token> callerToken = GetFuzzAbilityToken();
    sptr<Token> nullToken = nullptr;
    
    int32_t appIndex = 0;
    StartAbilityUtils::GetAppIndex(want, callerToken, appIndex);
    StartAbilityUtils::GetAppIndex(want, nullToken, appIndex);
    
    AppExecFwk::ApplicationInfo appInfo;

    AppExecFwk::AbilityInfo abilityInfo;
    StartAbilityUtils::callerAbilityInfo = BuildFuzzStartAbilityInfo(stringParam.c_str(), stringParam.size());
    StartAbilityUtils::GetCallerAbilityInfo(callerToken, abilityInfo);
    StartAbilityUtils::GetCallerAbilityInfo(nullToken, abilityInfo);
    StartAbilityUtils::callerAbilityInfo.reset();
    StartAbilityUtils::GetCallerAbilityInfo(callerToken, abilityInfo);
    StartAbilityUtils::GetCallerAbilityInfo(nullToken, abilityInfo);
    
    StartAbilityUtils::CheckAppProvisionMode(want, int32Param, callerToken);
    StartAbilityUtils::CheckAppProvisionMode(want, int32Param, nullToken);
    
    std::shared_ptr<StartAbilityInfoWrap> wrap1 =
        std::make_shared<StartAbilityInfoWrap>(want, int32Param, int32Param, callerToken, boolParam);
    std::shared_ptr<StartAbilityInfoWrap> wrap2 = std::make_shared<StartAbilityInfoWrap>();
    wrap2->SetStartAbilityInfo(abilityInfo);

    StartAbilityUtils::SetTargetCloneIndexInSameBundle(want, callerToken);
    StartAbilityUtils::SetTargetCloneIndexInSameBundle(want, nullToken);
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

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        return 0;
    }

    if (size < OHOS::U32_AT_SIZE || size > OHOS::MAX_FUZZ_INPUT_SIZE) {
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