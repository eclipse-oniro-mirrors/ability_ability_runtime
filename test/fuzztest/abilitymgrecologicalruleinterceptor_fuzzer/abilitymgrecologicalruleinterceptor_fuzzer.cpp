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

#include <fuzzer/FuzzedDataProvider.h>

#define private public
#include "ecological_rule_interceptor.h"
#undef private

#include "ability_record.h"
#include "ability_ecological_rule_mgr_service_param.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t FUZZ_DATA_MIN_SIZE = 4;
constexpr int32_t BUNDLE_TYPE_APP = 0;
constexpr int32_t BUNDLE_TYPE_ATOMIC_SERVICE = 1;
constexpr int32_t BUNDLE_TYPE_SHARED = 2;
constexpr int32_t BUNDLE_TYPE_APP_SERVICE_FWK = 3;
constexpr int32_t BUNDLE_TYPE_UNKNOWN = 99;
constexpr int32_t TEST_USER_ID = 100;
constexpr int32_t MAX_OPERATION_COUNT = 8;

// Named constants for operation dispatch
constexpr int32_t OP_GET_APP_TYPE = 0;
constexpr int32_t OP_DO_PROCESS_WANT = 2;
constexpr int32_t OP_GET_TARGET_INFO = 3;
constexpr int32_t OP_GET_CALLER_INFO = 4;
constexpr int32_t OP_INIT_CALLER_INFO = 5;
constexpr int32_t OP_QUERY_STARTUP_RULE = 6;
constexpr int32_t OP_NO_NEED_ERMS = 7;
constexpr int32_t MAX_OPERATION_TYPE = OP_NO_NEED_ERMS;

constexpr size_t STRING_MAX_LEN = 64;

const std::string TEST_BUNDLE = "com.example.fuzzTest";
const std::string TEST_ABILITY = "FuzzAbility";
const std::string CALLER_BUNDLE = "com.example.fuzzCaller";
} // namespace

sptr<Token> GetFuzzAbilityToken()
{
    sptr<Token> token = nullptr;
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = TEST_BUNDLE;
    abilityRequest.abilityInfo.name = TEST_ABILITY;
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    abilityRequest.abilityInfo.bundleName = TEST_BUNDLE;
    abilityRequest.abilityInfo.applicationInfo = abilityRequest.appInfo;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (abilityRecord != nullptr) {
        token = abilityRecord->GetToken();
    }
    return token;
}

std::shared_ptr<AbilityInfo> CreateFuzzAbilityInfo(FuzzedDataProvider &fdp)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    if (abilityInfo == nullptr) {
        return nullptr;
    }
    abilityInfo->bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    abilityInfo->name = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    abilityInfo->isStageBasedModel = fdp.ConsumeBool();
    int32_t bundleTypeVal = fdp.ConsumeIntegralInRange<int32_t>(
        BUNDLE_TYPE_APP, BUNDLE_TYPE_APP_SERVICE_FWK);
    abilityInfo->applicationInfo.bundleType =
        static_cast<BundleType>(bundleTypeVal);
    abilityInfo->applicationInfo.appDistributionType =
        fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    abilityInfo->applicationInfo.appProvisionType =
        fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    abilityInfo->applicationInfo.applicationReservedFlag =
        fdp.ConsumeIntegral<int32_t>();
    return abilityInfo;
}

Want BuildFuzzWant(FuzzedDataProvider &fdp)
{
    Want want;
    std::string callerBundle = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundle);
    std::string targetBundle = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    std::string targetAbility = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    ElementName element("", targetBundle, targetAbility);
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_CALLER_UID, fdp.ConsumeIntegral<int32_t>());
    want.SetParam(Want::PARAM_RESV_CALLER_PID, fdp.ConsumeIntegral<int32_t>());
    want.SetParam("send_to_erms_targetLinkFeature",
        fdp.ConsumeRandomLengthString(STRING_MAX_LEN));
    want.SetParam("send_to_erms_targetLinkType", fdp.ConsumeIntegral<int32_t>());
    want.SetParam("send_to_erms_embedded", fdp.ConsumeIntegral<int32_t>());
    return want;
}

Want BuildSameBundleWant(FuzzedDataProvider &fdp)
{
    Want want;
    std::string sameBundle = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, sameBundle);
    ElementName element("", sameBundle, fdp.ConsumeRandomLengthString(STRING_MAX_LEN));
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_CALLER_UID, fdp.ConsumeIntegral<int32_t>());
    want.SetParam(Want::PARAM_RESV_CALLER_PID, fdp.ConsumeIntegral<int32_t>());
    return want;
}

Want BuildDifferentBundleWant(FuzzedDataProvider &fdp)
{
    Want want;
    std::string callerBundle = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    std::string targetBundle = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    while (callerBundle == targetBundle && callerBundle.size() > 0) {
        targetBundle = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    }
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundle);
    ElementName element("", targetBundle,
        fdp.ConsumeRandomLengthString(STRING_MAX_LEN));
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_CALLER_UID, fdp.ConsumeIntegral<int32_t>());
    want.SetParam(Want::PARAM_RESV_CALLER_PID, fdp.ConsumeIntegral<int32_t>());
    return want;
}

void TestGetAppTypeByBundleType(
    const std::shared_ptr<EcologicalRuleInterceptor> &interceptor)
{
    interceptor->GetAppTypeByBundleType(BUNDLE_TYPE_ATOMIC_SERVICE);
    interceptor->GetAppTypeByBundleType(BUNDLE_TYPE_APP);
    interceptor->GetAppTypeByBundleType(BUNDLE_TYPE_APP_SERVICE_FWK);
    interceptor->GetAppTypeByBundleType(BUNDLE_TYPE_UNKNOWN);
    interceptor->GetAppTypeByBundleType(BUNDLE_TYPE_SHARED);
}

void TestDoProcessWantUserId(
    const std::shared_ptr<EcologicalRuleInterceptor> &interceptor,
    FuzzedDataProvider &fdp)
{
    Want sameBundleWant = BuildSameBundleWant(fdp);
    int32_t userId = fdp.ConsumeIntegralInRange<int32_t>(0, TEST_USER_ID);
    interceptor->DoProcess(sameBundleWant, userId);
    Want diffBundleWant = BuildDifferentBundleWant(fdp);
    interceptor->DoProcess(diffBundleWant, userId);
}

void TestGetEcologicalTargetInfo(
    const std::shared_ptr<EcologicalRuleInterceptor> &interceptor,
    FuzzedDataProvider &fdp)
{
    Want want = BuildFuzzWant(fdp);
    ErmsCallerInfo callerInfo;
    auto abilityInfo = CreateFuzzAbilityInfo(fdp);
    interceptor->GetEcologicalTargetInfo(want, abilityInfo, callerInfo);
    auto nullAbilityInfo = std::shared_ptr<AbilityInfo>(nullptr);
    interceptor->GetEcologicalTargetInfo(want, nullAbilityInfo, callerInfo);
}

void TestGetEcologicalCallerInfo(
    const std::shared_ptr<EcologicalRuleInterceptor> &interceptor,
    FuzzedDataProvider &fdp)
{
    Want want = BuildFuzzWant(fdp);
    ErmsCallerInfo callerInfo;
    callerInfo.uid = fdp.ConsumeIntegral<int32_t>();
    int32_t userId = fdp.ConsumeIntegralInRange<int32_t>(0, TEST_USER_ID);
    sptr<IRemoteObject> token = GetFuzzAbilityToken();
    interceptor->GetEcologicalCallerInfo(want, callerInfo, userId, token);
    interceptor->GetEcologicalCallerInfo(want, callerInfo, userId, nullptr);
}

void TestInitErmsCallerInfo(
    const std::shared_ptr<EcologicalRuleInterceptor> &interceptor,
    FuzzedDataProvider &fdp)
{
    Want want = BuildFuzzWant(fdp);
    auto abilityInfo = CreateFuzzAbilityInfo(fdp);
    ErmsCallerInfo callerInfo;
    int32_t userId = fdp.ConsumeIntegralInRange<int32_t>(0, TEST_USER_ID);
    sptr<IRemoteObject> token = GetFuzzAbilityToken();
    bool skipCallerInfo = fdp.ConsumeBool();
    interceptor->InitErmsCallerInfo(
        want, abilityInfo, callerInfo, userId, token, skipCallerInfo);
    interceptor->InitErmsCallerInfo(
        want, abilityInfo, callerInfo, userId, nullptr, skipCallerInfo);
}

void TestQueryAtomicServiceStartupRule(
    const std::shared_ptr<EcologicalRuleInterceptor> &interceptor,
    FuzzedDataProvider &fdp)
{
    Want want = BuildFuzzWant(fdp);
    int32_t userId = fdp.ConsumeIntegralInRange<int32_t>(0, TEST_USER_ID);
    sptr<IRemoteObject> token = GetFuzzAbilityToken();
    AtomicServiceStartupRule rule;
    sptr<Want> replaceWant = new (std::nothrow) Want();
    interceptor->QueryAtomicServiceStartupRule(
        want, token, userId, rule, replaceWant);
    Want sameBundleWant = BuildSameBundleWant(fdp);
    interceptor->QueryAtomicServiceStartupRule(
        sameBundleWant, token, userId, rule, replaceWant);
}

void TestNoNeedErms(
    const std::shared_ptr<EcologicalRuleInterceptor> &interceptor,
    FuzzedDataProvider &fdp)
{
    auto blockFunc = []() { return false; };
    Want sameBundleWant = BuildSameBundleWant(fdp);
    int32_t userId = fdp.ConsumeIntegralInRange<int32_t>(0, TEST_USER_ID);
    AbilityInterceptorParam sameParam(
        sameBundleWant, 0, userId, false, nullptr, blockFunc);
    sameParam.isTargetPlugin = false;
    interceptor->NoNeedErms(sameParam);
    Want diffBundleWant = BuildDifferentBundleWant(fdp);
    AbilityInterceptorParam pluginParam(
        diffBundleWant, 0, userId, false, nullptr, blockFunc);
    pluginParam.isTargetPlugin = true;
    interceptor->NoNeedErms(pluginParam);
}

void DoFuzzOperations(
    const std::shared_ptr<EcologicalRuleInterceptor> &interceptor,
    FuzzedDataProvider &fdp)
{
    int32_t operationCount = fdp.ConsumeIntegralInRange<int32_t>(
        1, MAX_OPERATION_COUNT);
    for (int32_t i = 0; i < operationCount && fdp.remaining_bytes() > 0; ++i) {
        int32_t operation = fdp.ConsumeIntegralInRange<int32_t>(
            0, MAX_OPERATION_TYPE);
        switch (operation) {
            case OP_GET_APP_TYPE:
                TestGetAppTypeByBundleType(interceptor);
                break;
            case OP_DO_PROCESS_WANT:
                TestDoProcessWantUserId(interceptor, fdp);
                break;
            case OP_GET_TARGET_INFO:
                TestGetEcologicalTargetInfo(interceptor, fdp);
                break;
            case OP_GET_CALLER_INFO:
                TestGetEcologicalCallerInfo(interceptor, fdp);
                break;
            case OP_INIT_CALLER_INFO:
                TestInitErmsCallerInfo(interceptor, fdp);
                break;
            case OP_QUERY_STARTUP_RULE:
                TestQueryAtomicServiceStartupRule(interceptor, fdp);
                break;
            case OP_NO_NEED_ERMS:
                TestNoNeedErms(interceptor, fdp);
                break;
            default:
                break;
        }
    }
}

bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    if (data == nullptr || size < FUZZ_DATA_MIN_SIZE) {
        return false;
    }
    FuzzedDataProvider fdp(data, size);
    auto interceptor = std::make_shared<EcologicalRuleInterceptor>();
    if (interceptor == nullptr) {
        return false;
    }
    DoFuzzOperations(interceptor, fdp);
    return true;
}
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return 0;
    }
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
