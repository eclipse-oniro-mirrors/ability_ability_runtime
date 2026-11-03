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

#include "dialogsessionmanagerfirst_fuzzer.h"
#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>
#define private public
#include "dialog_session_manager.h"
#undef private
#include "ability_fuzz_util.h"
#include "ability_record.h"
#include "dialog_session_manager.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
    constexpr size_t CODE_TWO = 2;
} // namespace

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

bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    AbilityRequest info;
    SelectorType type;
    sptr<IRemoteObject> callerToken = GetFuzzAbilityToken();
    std::string dialogSessionId;
    FuzzedDataProvider fdp(data, size);
    bool isSCBCall = fdp.ConsumeBool();
    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    dialogSessionId = fdp.ConsumeRandomLengthString(256);
    bool needGrantUriPermission = fdp.ConsumeBool();
    type = static_cast<SelectorType>(fdp.ConsumeIntegralInRange<int32_t>(0, CODE_TWO));
    AbilityFuzzUtil::GetRandomAbilityRequestInfo(fdp, info);
    sptr<DialogSessionInfo> dilogSessionInfo(new DialogSessionInfo());
    std::shared_ptr<DialogCallerInfo> dialogCallerInfo = std::make_shared<DialogCallerInfo>();
    Want want;
    bool isAllowed = fdp.ConsumeBool();
    std::vector<DialogAbilityInfo> targetAbilityInfos;
    std::shared_ptr<DialogSessionManager> dialogSessionManager = std::make_shared<DialogSessionManager>();
    if (dialogSessionManager == nullptr) {
        return false;
    }
    dialogSessionManager->SetQueryERMSInfo(dialogSessionId, info);
    dialogSessionManager->UpdateExtensionWantWithDialogCallerInfo(info, callerToken, isSCBCall);
    dialogSessionManager->OnlySetDialogCallerInfo(info, userId, type, dialogSessionId, needGrantUriPermission);
    dialogSessionManager->SetDialogSessionInfo(dialogSessionId, dilogSessionInfo, dialogCallerInfo);
    dialogSessionManager->NotifyAbilityRequestFailure(dialogSessionId, want);
    dialogSessionManager->SendDialogResult(want, dialogSessionId, isAllowed);
    dialogSessionManager->GenerateJumpTargetAbilityInfos(info, targetAbilityInfos);

    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */

    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}