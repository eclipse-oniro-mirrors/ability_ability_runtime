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

#include "extensionconfig_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#include "ability_record.h"
#define private public
#include "extension_config.h"
#define private public

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
constexpr size_t STRING_MAX_LENGTH = 128;
} // namespace

uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[INPUT_ZERO] << OFFSET_ZERO) | (ptr[INPUT_ONE] << OFFSET_ONE) | (ptr[INPUT_TWO] << OFFSET_TWO) |
        ptr[INPUT_THREE];
}

bool DoSomethingInterestingWithMyAPI(const char *ch, size_t size, const uint8_t *data)
{
    std::string strParam(ch, size);
    auto extensionConfig = std::make_shared<ExtensionConfig>();
    extensionConfig->LoadExtensionConfiguration();
    extensionConfig->GetExtensionAutoDisconnectTime(strParam);
    extensionConfig->IsExtensionStartThirdPartyAppEnable(strParam);
    ElementName targetElement;
    targetElement.SetBundleName(strParam);
    targetElement.SetAbilityName(strParam);
    extensionConfig->IsExtensionStartServiceEnable(strParam, targetElement);
    extensionConfig->HasAbilityAccess(strParam);
    extensionConfig->HasThridPartyAppAccessFlag(strParam);
    extensionConfig->HasServiceAccessFlag(strParam);
    extensionConfig->HasDefaultAccessFlag(strParam);
    extensionConfig->IsExtensionStartServiceEnableNew(strParam, targetElement);
    extensionConfig->IsExtensionStartThirdPartyAppEnableNew(strParam, targetElement);
    extensionConfig->IsExtensionStartDefaultEnable(strParam, targetElement);
    nlohmann::json object;
    extensionConfig->LoadExtensionConfig(object);
    FuzzedDataProvider fdp(data, size);
    std::string extensionTypeName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    bool isSystemApp = fdp.ConsumeBool();
    std::string bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    extensionConfig->IsScreenUnlockIntercept(extensionTypeName, isSystemApp, bundleName);
    std::string abilityName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    extensionConfig->IsScreenUnlockAllowAbility(extensionTypeName, bundleName, abilityName);
    extensionConfig->ReadFileInfoJson(strParam, object);
    extensionConfig->GetExtensionConfigPath();
    extensionConfig->LoadExtensionAutoDisconnectTime(object, strParam);
    extensionConfig->LoadExtensionThirdPartyAppBlockedList(object, strParam);
    extensionConfig->LoadExtensionServiceBlockedList(object, strParam);
    extensionConfig->LoadExtensionAbilityAccess(object, strParam);
    extensionConfig->CheckExtensionUriValid(strParam);
    extensionConfig->LoadExtensionNetworkEnable(object, strParam);
    extensionConfig->LoadExtensionSAEnable(object, strParam);
    extensionConfig->IsExtensionNetworkEnable(strParam);
    extensionConfig->IsExtensionSAEnable(strParam);
    ElementName targetElementName;
    std::unordered_set<std::string> list;
    std::string extensionTypeName1 = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    bool isSystemApp1 = fdp.ConsumeBool();
    std::string bundleName1 = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    std::string abilityName1 = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    extensionConfig->FindTargetUriInList(targetElementName, list);
    extensionConfig->IsScreenUnlockIntercept(extensionTypeName1, isSystemApp1, bundleName1);
    extensionConfig->IsScreenUnlockAllowAbility(extensionTypeName1, bundleName1, abilityName1);
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        std::cout << "invalid data" << std::endl;
        return 0;
    }

    /* Validate the length of size */
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

    OHOS::DoSomethingInterestingWithMyAPI(ch, size, data);
    free(ch);
    ch = nullptr;
    return 0;
}