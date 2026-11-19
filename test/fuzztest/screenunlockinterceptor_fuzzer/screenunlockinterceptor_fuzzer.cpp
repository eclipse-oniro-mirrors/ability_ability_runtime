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

#include "screenunlockinterceptor_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#include "bundle_mgr_helper.h"
#define private public
#include "screen_unlock_interceptor.h"
#undef private

#include "ability_record.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr int32_t API_DO_PROCESS = 0;
constexpr int32_t API_PROCESS_SYSTEM_APP = 1;
constexpr int32_t API_PROCESS_NON_SYSTEM_APP = 2;
constexpr int32_t API_CHECK_EXT_INTERCEPTION = 3;
constexpr int32_t API_CHECK_SYS_EXT_INTERCEPTION = 4;
constexpr int32_t API_CHECK_THIRD_EXT_INTERCEPTION = 5;
constexpr int32_t API_CHECK_INTERCEPTION_BY_CONFIG = 6;
constexpr int32_t API_GET_APP_IDENTIFIER = 7;
constexpr int32_t API_REPORT_SYS_UI_ABILITY_EVENT = 8;
constexpr int32_t MAX_API_CASE = API_REPORT_SYS_UI_ABILITY_EVENT;
constexpr size_t STRING_MAX_LEN = 128;
constexpr int32_t USER_ID_DEFAULT = 100;
constexpr int32_t REQUEST_CODE_DEFAULT = 0;
} // namespace

sptr<Token> CreateFuzzAbilityToken()
{
    AbilityRequest request;
    request.appInfo.bundleName = "com.example.fuzzTest";
    request.abilityInfo.name = "MainAbility";
    request.abilityInfo.type = AbilityType::DATA;
    auto record = AbilityRecord::CreateAbilityRecord(request);
    if (record != nullptr) {
        return record->GetToken();
    }
    return nullptr;
}

AbilityInterceptorParam BuildInterceptorParam(Want &want, sptr<IRemoteObject> &token,
    const std::shared_ptr<AbilityInfo> &abilityInfo)
{
    return InterceptorParamBuilder(want, REQUEST_CODE_DEFAULT, USER_ID_DEFAULT)
        .WithUI(false).CallerToken(token).AbilityInfo(abilityInfo).Build();
}

void FuzzDoProcess(FuzzedDataProvider &fdp)
{
    auto interceptor = std::make_shared<ScreenUnlockInterceptor>();
    sptr<IRemoteObject> token = CreateFuzzAbilityToken();
    Want want;
    std::string bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    std::string abilityName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    want.SetElementName(bundleName, abilityName);
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = fdp.ConsumeBool() ? AbilityType::EXTENSION : AbilityType::PAGE;
    abilityInfo->extensionTypeName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    abilityInfo->applicationInfo.bundleName = bundleName;
    abilityInfo->applicationInfo.name = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    abilityInfo->applicationInfo.isSystemApp = fdp.ConsumeBool();
    abilityInfo->applicationInfo.allowAppRunWhenDeviceFirstLocked = fdp.ConsumeBool();
    AbilityInterceptorParam param = BuildInterceptorParam(want, token, abilityInfo);
    interceptor->DoProcess(param);
}

void FuzzProcessSystemApp(FuzzedDataProvider &fdp)
{
    auto interceptor = std::make_shared<ScreenUnlockInterceptor>();
    AbilityInfo info;
    bool allowRun = fdp.ConsumeBool();
    info.applicationInfo.allowAppRunWhenDeviceFirstLocked = allowRun;
    info.applicationInfo.bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    info.applicationInfo.name = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    info.applicationInfo.isSystemApp = true;
    bool isExtension = fdp.ConsumeBool();
    if (isExtension) {
        info.type = AbilityType::EXTENSION;
    }
    info.extensionTypeName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    info.name = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    interceptor->ProcessSystemApp(info);
}

void FuzzProcessNonSystemApp(FuzzedDataProvider &fdp)
{
    auto interceptor = std::make_shared<ScreenUnlockInterceptor>();
    AbilityInfo info;
    info.applicationInfo.bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    info.applicationInfo.name = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    info.applicationInfo.isSystemApp = false;
    bool isExtension = fdp.ConsumeBool();
    if (isExtension) {
        info.type = AbilityType::EXTENSION;
    }
    info.extensionTypeName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    info.name = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    interceptor->ProcessNonSystemApp(info);
}

void FuzzCheckExtensionInterception(FuzzedDataProvider &fdp)
{
    auto interceptor = std::make_shared<ScreenUnlockInterceptor>();
    std::string extTypeName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    std::string bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    bool isSystemApp = fdp.ConsumeBool();
    interceptor->CheckExtensionInterception(extTypeName, bundleName, isSystemApp);
}

void FuzzCheckSystemAppExtensionInterception(FuzzedDataProvider &fdp)
{
    auto interceptor = std::make_shared<ScreenUnlockInterceptor>();
    std::string extTypeName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    std::string bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    interceptor->CheckSystemAppExtensionInterception(extTypeName, bundleName);
}

void FuzzCheckThirdPartyExtensionInterception(FuzzedDataProvider &fdp)
{
    auto interceptor = std::make_shared<ScreenUnlockInterceptor>();
    std::string extTypeName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    std::string bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    interceptor->CheckThirdPartyExtensionInterception(extTypeName, bundleName);
}

void FuzzCheckInterceptionByConfig(FuzzedDataProvider &fdp)
{
    auto interceptor = std::make_shared<ScreenUnlockInterceptor>();
    std::string extTypeName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    std::string appIdentifier = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    bool interception = fdp.ConsumeBool();
    bool isSystemApp = fdp.ConsumeBool();
    interceptor->CheckInterceptionByConfig(
        extTypeName, appIdentifier, interception, isSystemApp);
}

void FuzzGetAppIdentifier(FuzzedDataProvider &fdp)
{
    auto interceptor = std::make_shared<ScreenUnlockInterceptor>();
    std::string bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    interceptor->GetAppIdentifier(bundleName);
    interceptor->GetAppIdentifier("");
}

void FuzzReportSystemAppUIAbilityEvent(FuzzedDataProvider &fdp)
{
    auto interceptor = std::make_shared<ScreenUnlockInterceptor>();
    AbilityInfo info;
    info.applicationInfo.bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    info.name = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    interceptor->ReportSystemAppUIAbilityEvent(info);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    auto apiCase = fdp.ConsumeIntegralInRange<int32_t>(0, MAX_API_CASE);
    switch (apiCase) {
        case API_DO_PROCESS:
            FuzzDoProcess(fdp);
            break;
        case API_PROCESS_SYSTEM_APP:
            FuzzProcessSystemApp(fdp);
            break;
        case API_PROCESS_NON_SYSTEM_APP:
            FuzzProcessNonSystemApp(fdp);
            break;
        case API_CHECK_EXT_INTERCEPTION:
            FuzzCheckExtensionInterception(fdp);
            break;
        case API_CHECK_SYS_EXT_INTERCEPTION:
            FuzzCheckSystemAppExtensionInterception(fdp);
            break;
        case API_CHECK_THIRD_EXT_INTERCEPTION:
            FuzzCheckThirdPartyExtensionInterception(fdp);
            break;
        case API_CHECK_INTERCEPTION_BY_CONFIG:
            FuzzCheckInterceptionByConfig(fdp);
            break;
        case API_GET_APP_IDENTIFIER:
            FuzzGetAppIdentifier(fdp);
            break;
        case API_REPORT_SYS_UI_ABILITY_EVENT:
            FuzzReportSystemAppUIAbilityEvent(fdp);
            break;
        default:
            break;
    }
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
