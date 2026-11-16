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

#include "uiabilitylifecyclemanagerb_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#include "ui_ability_lifecycle_manager.h"
#undef private

#include "ability_record.h"
#include "ability_state.h"

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
} // namespace

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

void DoSomethingInterestingWithMyAPI1(const char *data, size_t size)
{
    int32_t userId = static_cast<int32_t>(GetU32Data(data));
    auto uIAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>(userId);
    int32_t int32Param = static_cast<int32_t>(GetU32Data(data));
    bool boolParam = *data % ENABLE;
    uIAbilityLifecycleManager->CompleteFirstFrameDrawing(int32Param);
    uIAbilityLifecycleManager->GetAbilityStateByPersistentId(int32Param, boolParam);
    AbilityRequest abilityRequest1;
    uIAbilityLifecycleManager->GetPersistentIdByAbilityRequest(abilityRequest1, boolParam);
    abilityRequest1.collaboratorType = CollaboratorType::RESERVE_TYPE;
    if (abilityRequest1.collaboratorType == CollaboratorType::RESERVE_TYPE) {
        uIAbilityLifecycleManager->GetPersistentIdByAbilityRequest(abilityRequest1, boolParam);
    }

    abilityRequest1.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    if (abilityRequest1.abilityInfo.launchMode == AppExecFwk::LaunchMode::SPECIFIED) {
        uIAbilityLifecycleManager->GetPersistentIdByAbilityRequest(abilityRequest1, boolParam);
    }

    abilityRequest1.abilityInfo.launchMode = AppExecFwk::LaunchMode::STANDARD;
    if (abilityRequest1.abilityInfo.launchMode == AppExecFwk::LaunchMode::STANDARD) {
        uIAbilityLifecycleManager->GetPersistentIdByAbilityRequest(abilityRequest1, boolParam);
    }

    uIAbilityLifecycleManager->GetReusedSpecifiedPersistentId(abilityRequest1, boolParam);
    abilityRequest1.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    if (abilityRequest1.abilityInfo.launchMode == AppExecFwk::LaunchMode::SPECIFIED) {
        uIAbilityLifecycleManager->GetReusedSpecifiedPersistentId(abilityRequest1, boolParam);
    }
}
bool DoSomethingInterestingWithMyAPI(const char *data, size_t size)
{
    int32_t userId = static_cast<int32_t>(GetU32Data(data));
    auto uIAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>(userId);
    std::string strParam(data, size);
    int32_t int32Param = static_cast<int32_t>(GetU32Data(data));
    uIAbilityLifecycleManager->UninstallApp(strParam, int32Param);
    std::vector<AbilityRunningInfo> info;
    bool boolParam = *data % ENABLE;
    uIAbilityLifecycleManager->GetAbilityRunningInfos(info, boolParam);
    int intParam = static_cast<int>(GetU32Data(data));

    std::vector<std::string> info1;
    uIAbilityLifecycleManager->Dump(info1);
    uIAbilityLifecycleManager->DumpMissionList(info1, boolParam, strParam);
    std::vector<std::string> params;
    uIAbilityLifecycleManager->DumpMissionListByRecordId(info1, boolParam, int32Param, params);
    std::shared_ptr<StartOptions> startOptions;
    uIAbilityLifecycleManager->MoveMissionToFront(int32Param, startOptions);
    AbilityRequest abilityRequest;
    std::shared_ptr<AbilityRecord> targetRecord;
    uIAbilityLifecycleManager->IsAbilityStarted(abilityRequest, targetRecord);
    std::list<SessionInfo> sessionInfos;
    std::vector<int32_t> sessionIds;
    uIAbilityLifecycleManager->UpdateSessionInfoBySCB(sessionInfos, sessionIds);
    uIAbilityLifecycleManager->IsCallerInStatusBar("");
    std::vector<int32_t> pids;
    uIAbilityLifecycleManager->TryPrepareTerminateByPids(pids, false);
    sptr<IRemoteObject> token = GetFuzzAbilityToken();
    uIAbilityLifecycleManager->ChangeAbilityVisibility(token, boolParam, nullptr);
    sptr<SessionInfo> sessionInfo;
    uIAbilityLifecycleManager->ChangeUIAbilityVisibilityBySCB(sessionInfo, boolParam);
    AppExecFwk::ElementName element;
    uIAbilityLifecycleManager->GetAbilityRecordsByName(element, int32Param);
    uIAbilityLifecycleManager->GetAbilityRecordByToken(token);

    #ifdef SUPPORT_GRAPHICS
    uIAbilityLifecycleManager->CompleteFirstFrameDrawing(token);
    #endif
    DoSomethingInterestingWithMyAPI1(data, size);

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

    OHOS::DoSomethingInterestingWithMyAPI(ch, size);
    free(ch);
    ch = nullptr;
    return 0;
}