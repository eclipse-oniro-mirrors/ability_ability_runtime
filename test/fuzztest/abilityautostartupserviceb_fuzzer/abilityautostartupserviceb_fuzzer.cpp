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

#include "abilityautostartupserviceb_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#define protected public
#include "ability_auto_startup_service.h"
#undef protected
#undef private

#include "ability_record.h"
#include "auto_startup_callback_proxy.h"

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

void AbilityStartupServiceFuzztest1(bool boolParam, std::string &stringParam, int32_t int32Param)
{
    std::shared_ptr<AbilityAutoStartupService> service = std::make_shared<AbilityAutoStartupService>();
    service->SetDeathRecipient(nullptr, nullptr);
    sptr<Token> token1 = GetFuzzAbilityToken();
    service->SetDeathRecipient(token1, nullptr); // add deathrecipient
    sptr<IRemoteObject::DeathRecipient> client =
        new (std::nothrow) AbilityAutoStartupService::ClientDeathRecipient(std::weak_ptr(service));
    service->SetDeathRecipient(token1, client); // add deathrecipient
    service->SetDeathRecipient(token1, client); // duplicate add deathrecipient
    sptr<Token> token2 = GetFuzzAbilityToken();
    service->RegisterAutoStartupSystemCallback(token1);
    service->CleanResource(nullptr); // branch nullptr token
    service->CleanResource(token1); // branch clean exists token
    service->CleanResource(token2);  // branch clean not exists token

    client->OnRemoteDied(nullptr); // branch
    client->OnRemoteDied(token1); // branch

    service->GetSelfApplicationBundleName();
    service->CheckSelfApplication(stringParam);
    AppExecFwk::BundleInfo bundleInfo;
    service->GetBundleInfo(stringParam, int32Param, int32Param, bundleInfo); // branch
    AutoStartupInfo info;
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.bundleName = stringParam;
    abilityInfo.name = stringParam;
    abilityInfo.moduleName = stringParam;
    service->GetAbilityTypeName(abilityInfo); // branch
    abilityInfo.type == AppExecFwk::AbilityType::PAGE;
    service->GetAbilityTypeName(abilityInfo); // branch
    service->GetBundleMgrClient();
    service->CheckPermissionForSystem();
    service->SetApplicationAutoStartupByEDM(info, boolParam);
    service->CancelApplicationAutoStartupByEDM(info, boolParam);
    service->InnerApplicationAutoStartupByEDM(info, boolParam, boolParam);
    service->CheckPermissionForEDM();
    service->AddHandledAutoStartupUsers(int32Param);
    service->RemoveHandledAutoStartupUsers(int32Param);
    service->FindHandledAutoStartupUsers(int32Param);
}

void AbilityStartupServiceFuzztest2(bool boolParam, std::string &stringParam, int32_t int32Param)
{
    std::shared_ptr<AbilityAutoStartupService> service = std::make_shared<AbilityAutoStartupService>();
    AutoStartupAbilityData abilityData;
    abilityData.isVisible = boolParam;
    abilityData.abilityTypeName = stringParam;
    abilityData.accessTokenId = stringParam;
    abilityData.setterUserId = int32Param;
    AutoStartupInfo info;
    service->GetAbilityData(info, abilityData); // branch
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.bundleName = stringParam;
    abilityInfo.name = stringParam;
    abilityInfo.moduleName = stringParam;
    service->IsTargetAbility(info, abilityInfo);
    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.bundleName = stringParam;
    extensionInfo.name = stringParam;
    extensionInfo.moduleName = stringParam;
    service->IsTargetExtension(info, extensionInfo);
    service->GetExtensionTypeName(extensionInfo);
    extensionInfo.type == AppExecFwk::ExtensionAbilityType::SERVICE;
    service->GetExtensionTypeName(extensionInfo);
    extensionInfo.type == AppExecFwk::ExtensionAbilityType::APP_SERVICE;
    service->GetExtensionTypeName(extensionInfo);
    service->GetAbilityInfo(info, abilityData);
}

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    bool boolParam = *data % ENABLE;
    std::string stringParam(data, size);
    int32_t int32Param = static_cast<int32_t>(GetU32Data(data));
    AbilityStartupServiceFuzztest1(boolParam, stringParam, int32Param);
    AbilityStartupServiceFuzztest2(boolParam, stringParam, int32Param);
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

