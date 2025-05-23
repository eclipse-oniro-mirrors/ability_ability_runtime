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

#include "abilitymgrinsightintentexecutemanager_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#include "insight_intent_execute_manager.h"
#include "insight_intent_execute_param.h"
#undef private

#include "securec.h"
#include "ability_record.h"

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
}
uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[INPUT_ZERO] << OFFSET_ZERO) | (ptr[INPUT_ONE] << OFFSET_ONE) | (ptr[INPUT_TWO] << OFFSET_TWO) |
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

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    std::shared_ptr<InsightIntentExecuteConnection> infos = std::make_shared<InsightIntentExecuteConnection>();
    AppExecFwk::ElementName element;
    sptr<IRemoteObject> remoteObject;
    int resultCod = static_cast<int>(GetU32Data(data));
    infos->OnAbilityConnectDone(element, remoteObject, resultCod);
    infos->OnAbilityDisconnectDone(element, resultCod);
    uint64_t intentId = static_cast<uint64_t>(GetU32Data(data));

    InsightIntentExecuteRecipient Rinfos(intentId);
    wptr<IRemoteObject> remote;
    Rinfos.OnRemoteDied(remote);
    sptr<IRemoteObject> callerToken = GetFuzzAbilityToken();

    std::shared_ptr<InsightIntentExecuteManager> Minfos = std::make_shared<InsightIntentExecuteManager>();
    std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> paramPtr;
    uint64_t key = static_cast<uint64_t>(GetU32Data(data));
    Minfos->CheckAndUpdateParam(key, callerToken, paramPtr);
    Want want;
    ExecuteMode executeMode = UI_ABILITY_FOREGROUND;
    Minfos->CheckAndUpdateWant(want, executeMode);
    executeMode = UI_ABILITY_BACKGROUND;
    Minfos->CheckAndUpdateWant(want, executeMode);
    executeMode = UI_EXTENSION_ABILITY;
    Minfos->CheckAndUpdateWant(want, executeMode);
    executeMode = SERVICE_EXTENSION_ABILITY;
    Minfos->CheckAndUpdateWant(want, executeMode);

    Minfos->RemoveExecuteIntent(intentId);
    int32_t resultCode = static_cast<int32_t>(GetU32Data(data));
    InsightIntentExecuteResult result;
    Minfos->ExecuteIntentDone(intentId, resultCode, result);
    Minfos->RemoteDied(intentId);
    std::string jsonStr(data, size);
    Minfos->GetBundleName(intentId, jsonStr);
    std::string callerBundleName = "com.example.fuzzTest";
    Minfos->AddRecord(key, callerToken, jsonStr, intentId, callerBundleName);
    AbilityRuntime::ExtractInsightIntentGenericInfo decoratorInfo;
    Minfos->GenerateWant(paramPtr, decoratorInfo, want);
    Minfos->IsValidCall(want);
    Minfos->CheckCallerPermission();
    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
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

