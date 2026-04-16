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

#include "abilitymgrecologicalruleinterceptorfirst_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#define private public
#include "ecological_rule_interceptor.h"
#undef private

#include "ability_ecological_rule_mgr_service_param.h"
#include "ability_record.h"
#include "securec.h"
#include "want_params.h"
#include "ipc_skeleton.h"

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
constexpr size_t MAX_PARAM_NUM = 10;
constexpr const char* TEST_BUNDLE_NAME = "com.example.fuzzTest";
constexpr const char* TEST_ABILITY_NAME = "MainAbility";

uint32_t GetU32Data(const char* ptr, size_t size, size_t offset = 0)
{
    if (ptr == nullptr || (offset + sizeof(uint32_t)) > size) {
        return 0;
    }
    uint32_t value = 0;
    (void)memcpy_s(&value, sizeof(value), ptr + offset, sizeof(value));
    return value;
}

std::string GetStringFromFuzz(const char* ptr, size_t size, size_t& offset)
{
    if (ptr == nullptr || offset >= size) {
        return "";
    }
    size_t strLen = static_cast<uint8_t>(ptr[offset++]) % MAX_STR_LEN;
    if (strLen == 0 || (offset + strLen) > size) {
        return "";
    }
    std::string str(ptr + offset, strLen);
    offset += strLen;
    return str;
}

void FillWantErmsParams(Want& want, const char* data, size_t size, size_t& offset)
{
    std::string targetLinkFeature = GetStringFromFuzz(data, size, offset);
    want.SetParam("send_to_erms_targetLinkFeature", targetLinkFeature);
    int targetLinkType = static_cast<int>(GetU32Data(data, size, offset));
    want.SetParam("send_to_erms_targetLinkType", targetLinkType);
    int embedded = static_cast<int>(GetU32Data(data, size, offset));
    want.SetParam("send_to_erms_embedded", embedded);
}

void FillWantCallerParams(Want& want, const char* data, size_t size, size_t& offset)
{
    int callerUid = static_cast<int>(GetU32Data(data, size, offset));
    want.SetParam(Want::PARAM_RESV_CALLER_UID, callerUid);
    int callerPid = static_cast<int>(GetU32Data(data, size, offset));
    want.SetParam(Want::PARAM_RESV_CALLER_PID, callerPid);
}

void FillWantExtraParams(Want& want, const char* data, size_t size, size_t& offset)
{
    size_t paramCount = GetU32Data(data, size, offset) % MAX_PARAM_NUM;
    for (size_t i = 0; i < paramCount && offset < size; i++) {
        std::string key = GetStringFromFuzz(data, size, offset);
        std::string value = GetStringFromFuzz(data, size, offset);
        want.SetParam(key, value);
    }
}

Want BuildFuzzWant(const char* data, size_t size)
{
    Want want;
    size_t offset = U32_AT_SIZE;

    std::string callerBundle = GetStringFromFuzz(data, size, offset);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundle);
    
    ElementName element;
    element.SetBundleName(GetStringFromFuzz(data, size, offset));
    element.SetAbilityName(GetStringFromFuzz(data, size, offset));
    element.SetModuleName("entry");
    want.SetElement(element);

    FillWantErmsParams(want, data, size, offset);
    FillWantCallerParams(want, data, size, offset);
    FillWantExtraParams(want, data, size, offset);

    want.SetAction(Want::ACTION_HOME);
    want.AddEntity(Want::ENTITY_HOME);
    want.SetFlags(Want::FLAG_START_FOREGROUND_ABILITY);
    want.SetUri("dataability:///com.example.erms.test");

    return want;
}

void FillAbilityInfoBasic(
    std::shared_ptr<AbilityInfo> abilityInfo, const char* data, size_t size, size_t& offset)
{
    abilityInfo->bundleName = GetStringFromFuzz(data, size, offset);
    abilityInfo->name = GetStringFromFuzz(data, size, offset);
    abilityInfo->type = static_cast<AbilityType>(
        GetU32Data(data, size, offset) % static_cast<uint32_t>(AbilityType::EXTENSION));
    abilityInfo->extensionAbilityType = static_cast<ExtensionAbilityType>(
        GetU32Data(data, size, offset) % static_cast<uint32_t>(ExtensionAbilityType::UNSPECIFIED));
}

void FillAbilityInfoAppInfo(
    std::shared_ptr<AbilityInfo> abilityInfo, const char* data, size_t size, size_t& offset)
{
    abilityInfo->applicationInfo.bundleName = GetStringFromFuzz(data, size, offset);
    abilityInfo->applicationInfo.name = GetStringFromFuzz(data, size, offset);
    abilityInfo->applicationInfo.bundleType = static_cast<BundleType>(
        GetU32Data(data, size, offset) % static_cast<uint32_t>(BundleType::SHARED));
    abilityInfo->applicationInfo.appDistributionType = static_cast<AppDistributionTypeEnum>(
        GetU32Data(data, size, offset) % static_cast<uint32_t>(AppDistributionTypeEnum::
        APP_DISTRIBUTION_TYPE_APP_GALLERY));
    abilityInfo->applicationInfo.applicationReservedFlag =
        static_cast<uint32_t>(GetU32Data(data, size, offset));
    abilityInfo->isStageBasedModel = true;
}

std::shared_ptr<AbilityInfo> BuildFuzzAbilityInfo(const char* data, size_t size, size_t& offset)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    if (abilityInfo == nullptr) {
        return nullptr;
    }

    FillAbilityInfoBasic(abilityInfo, data, size, offset);
    FillAbilityInfoAppInfo(abilityInfo, data, size, offset);

    return abilityInfo;
}

sptr<Token> GetFuzzAbilityToken()
{
    sptr<Token> token = nullptr;
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = TEST_BUNDLE_NAME;
    abilityRequest.abilityInfo.name = TEST_ABILITY_NAME;
    abilityRequest.abilityInfo.type = static_cast<AbilityType>(
        GetU32Data(TEST_BUNDLE_NAME, strlen(TEST_BUNDLE_NAME)) %
        static_cast<uint32_t>(AbilityType::EXTENSION));
    
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (abilityRecord) {
        token = abilityRecord->GetToken();
    }
    return token;
}

AbilityInterceptorParam BuildFuzzInterceptorParam(const Want& want, const char* data, size_t size, size_t& offset)
{
    int requestCode = static_cast<int>(GetU32Data(data, size, offset));
    int32_t userId = static_cast<int32_t>(GetU32Data(data, size, offset));
    bool isWithUI = (GetU32Data(data, size, offset) % 2 == 1);
    sptr<Token> callerToken = GetFuzzAbilityToken();
    auto abilityInfo = BuildFuzzAbilityInfo(data, size, offset);
    bool isStartAsCaller = (GetU32Data(data, size, offset) % 2 == 1);
    int32_t appIndex = static_cast<int32_t>(GetU32Data(data, size, offset));

    AbilityInterceptorParam param(
        want,
        requestCode,
        userId,
        isWithUI,
        callerToken,
        abilityInfo,
        isStartAsCaller,
        appIndex
    );

    return param;
}

void CallEcologicalRuleFunctions(
    std::shared_ptr<EcologicalRuleInterceptor> executer,
    const Want& want,
    const AbilityInterceptorParam& param,
    std::shared_ptr<AbilityInfo> abilityInfo,
    int32_t userId,
    sptr<Token> token,
    const char* data,
    size_t size,
    size_t& paramOffset)
{
    ErmsCallerInfo callerInfo;
    AtomicServiceStartupRule rule;
    sptr<Want> replaceWant = new Want(want);

    (void)executer->GetEcologicalTargetInfo(want, abilityInfo, callerInfo);
    
    (void)executer->GetEcologicalCallerInfo(want, callerInfo, userId, token);
    
    (void)executer->InitErmsCallerInfo(want, abilityInfo, callerInfo, userId, token);
    
    Want atomicWant = want;
    std::string callerBundle = GetStringFromFuzz(data, size, paramOffset);
    atomicWant.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundle);
    (void)executer->QueryAtomicServiceStartupRule(
        atomicWant, token, userId, rule, replaceWant);
}

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    if (data == nullptr || size < U32_AT_SIZE) {
        return false;
    }

    std::shared_ptr<EcologicalRuleInterceptor> executer = std::make_shared<EcologicalRuleInterceptor>();
    if (executer == nullptr) {
        return false;
    }

    size_t offset = 0;
    Want want = BuildFuzzWant(data, size);
    size_t paramOffset = U32_AT_SIZE * 2;
    AbilityInterceptorParam param = BuildFuzzInterceptorParam(want, data, size, paramOffset);
    auto abilityInfo = BuildFuzzAbilityInfo(data, size, paramOffset);
    int32_t userId = static_cast<int32_t>(GetU32Data(data, size, paramOffset));
    sptr<Token> token = GetFuzzAbilityToken();
    
    CallEcologicalRuleFunctions(
        executer, want, param, abilityInfo, userId, token, data, size, paramOffset);

    return true;
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        return 0;
    }

    if (size < OHOS::U32_AT_SIZE) {
        return 0;
    }

    char* ch = static_cast<char*>(malloc(size + 1));
    if (ch == nullptr) {
        return 0;
    }
    (void)memset_s(ch, size + 1, 0x00, size + 1);
    if (memcpy_s(ch, size + 1, data, size) != EOK) {
        free(ch);
        ch = nullptr;
        return 0;
    }

    (void)OHOS::DoSomethingInterestingWithMyAPI(ch, size);

    free(ch);
    ch = nullptr;
    return 0;
}
}