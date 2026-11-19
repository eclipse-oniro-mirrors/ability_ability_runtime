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

#include "extensionrecordmanagerc_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>
#include <memory>
#include <vector>
#include <list>
#include <string>

#include "nlohmann/json.hpp"
#define private public
#define protected public
#define inline
#include "extension_record_manager.h"
#undef inline
#undef protected
#undef private

#include "ability_fuzz_util.h"
#include "ability_record.h"
#include "base_extension_record.h"
#include "extension_record.h"
#include "extension_record_factory.h"
#include "ipc_skeleton.h"
#include "ui_extension_record.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LENGTH = 128;
constexpr int32_t INVALID_EXTENSION_RECORD_ID = -1;
constexpr int32_t DEFAULT_USER_ID = 0;
constexpr int32_t CODE_THREE = 3;
constexpr int32_t CODE_SEVEN = 7;
constexpr pid_t INVALID_PID = -1;
const std::string PROCESS_MODE_HOST_SPECIFIED_KEY = "ability.want.params.host_specified_process";
const std::string IS_PRELOAD_UIEXTENSION_ABILITY = "ability.want.params.is_preload_uiextension_ability";
const std::string UIEXTENSION_TYPE_KEY = "ability.want.params.uiExtensionType";
const std::string UIEXTENSION_TYPE_VALUE = "embeddedUI";
} // namespace

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < sizeof(int32_t)) {
        return true;
    }

    std::shared_ptr<ExtensionRecordManager> extensionRecordManager =
        std::make_shared<ExtensionRecordManager>(DEFAULT_USER_ID);
    FuzzedDataProvider fdp(data, size);

    int32_t extensionRecordId = fdp.ConsumeIntegral<int32_t>();
    int32_t validExtId = extensionRecordManager->GenerateExtensionRecordId(INVALID_EXTENSION_RECORD_ID);
    int32_t pid = fdp.ConsumeIntegral<int32_t>();
    int32_t hostPid = fdp.ConsumeIntegral<int32_t>();
    int32_t recordNum = fdp.ConsumeIntegral<int32_t>();
    bool isLoaded = fdp.ConsumeBool();
    bool preloadFlag = fdp.ConsumeBool();
    bool isolationProcess = fdp.ConsumeBool();

    std::string hostBundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    std::string bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    std::string process = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    std::string moduleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    std::string customProcess = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    std::string emptyStr = "";

    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<BaseExtensionRecord> abilityRecord =
        std::make_shared<BaseExtensionRecord>(want, abilityInfo, applicationInfo);
    std::shared_ptr<ExtensionRecord> emptyRecord = nullptr;
    std::shared_ptr<ExtensionRecord> validRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    std::shared_ptr<BaseExtensionRecord> emptyAbilityRecord = nullptr;
    std::shared_ptr<BaseExtensionRecord> validAbilityRecord =
        std::make_shared<BaseExtensionRecord>(want, abilityInfo, applicationInfo);
    std::list<sptr<IRemoteObject>> callerList;
    std::vector<std::string> extensionList;
    std::vector<std::shared_ptr<ExtensionRecord>> recordsToUnload;
    AbilityFuzzUtil::GetRandomAbilityInfo(fdp, abilityInfo);
    abilityInfo.isolationProcess = isolationProcess;
    abilityInfo.extensionAbilityType =
        static_cast<ExtensionAbilityType>(fdp.ConsumeIntegralInRange<int32_t>(0, CODE_THREE));

    AbilityRequest abilityRequest;
    AbilityFuzzUtil::GetRandomAbilityRequestInfo(fdp, abilityRequest);
    abilityRequest.extensionProcessMode =
        static_cast<ExtensionProcessMode>(fdp.ConsumeIntegralInRange<int32_t>(0, CODE_SEVEN));
    abilityRequest.customProcess = customProcess;
    abilityRequest.abilityInfo = abilityInfo;
    abilityRequest.want.SetParam(PROCESS_MODE_HOST_SPECIFIED_KEY, process);
    abilityRequest.want.SetParam(IS_PRELOAD_UIEXTENSION_ABILITY, preloadFlag);
    abilityRequest.want.SetParam(UIEXTENSION_TYPE_KEY, UIEXTENSION_TYPE_VALUE);
    abilityRequest.sessionInfo = new (std::nothrow) SessionInfo();
    abilityRequest.sessionInfo->uiExtensionComponentId = fdp.ConsumeIntegral<uint64_t>();

    ElementName element;
    AbilityFuzzUtil::GenerateElementName(fdp, element);

    UIExtensionSessionInfo uiExtensionSessionInfo;
    sptr<IRemoteObject> token;
    sptr<IRemoteObject> nullToken = nullptr;
    sptr<IRemoteObject::DeathRecipient> deathRecipient = nullptr;
    ExtensionRecordManager::PreLoadUIExtensionMapKey preLoadUIExtensionInfo =
        std::make_tuple(element.GetAbilityName(), element.GetBundleName(), moduleName, hostPid);
    std::tuple<std::string, std::string, std::string, int32_t> extensionRecordMapKey = preLoadUIExtensionInfo;

    extensionRecordManager->AddExtensionRecord(validExtId, validRecord);
    validRecord->extensionRecordId_ = validExtId;
    validRecord->hostBundleName_ = hostBundleName;
    validRecord->hostPid_ = IPCSkeleton::GetCallingPid();
    validRecord->abilityRecord_ = validAbilityRecord;
    int32_t outExtId = INVALID_EXTENSION_RECORD_ID;
    
    extensionRecordManager->CreateExtensionRecord(abilityRequest, hostBundleName, validRecord, outExtId, hostPid);
    extensionRecordManager->CreateExtensionRecord(abilityRequest, emptyStr, emptyRecord, outExtId, INVALID_PID);
    extensionRecordManager->GetUIExtensionRootHostInfo(token);
    extensionRecordManager->GetUIExtensionRootHostInfo(nullToken);
    extensionRecordManager->GetUIExtensionSessionInfo(token, uiExtensionSessionInfo);
    extensionRecordManager->GetUIExtensionSessionInfo(nullToken, uiExtensionSessionInfo);
    extensionRecordManager->GetExtensionRecordById(extensionRecordId);
    extensionRecordManager->GetExtensionRecordById(validExtId);
    extensionRecordManager->GetExtensionRecordById(INVALID_EXTENSION_RECORD_ID);
    extensionRecordManager->LoadTimeout(extensionRecordId);
    extensionRecordManager->LoadTimeout(validExtId);
    extensionRecordManager->ForegroundTimeout(extensionRecordId);
    extensionRecordManager->BackgroundTimeout(extensionRecordId);
    extensionRecordManager->TerminateTimeout(extensionRecordId);
    extensionRecordManager->GetCallerTokenList(std::static_pointer_cast<AbilityRecord>(validAbilityRecord), callerList);
    extensionRecordManager->GetCallerTokenList(nullptr, callerList);
    extensionRecordManager->IsFocused(extensionRecordId, token, token);
    extensionRecordManager->IsFocused(INVALID_EXTENSION_RECORD_ID, nullToken, nullToken);
    extensionRecordManager->QueryPreLoadUIExtensionRecord(element, moduleName, hostPid, recordNum);
    extensionRecordManager->QueryPreLoadUIExtensionRecord(element, emptyStr, INVALID_PID, recordNum);
    extensionRecordManager->ClearPreloadedUIExtensionAbility(extensionRecordId);
    extensionRecordManager->ClearPreloadedUIExtensionAbility(validExtId);
    extensionRecordManager->ClearAllPreloadUIExtensionRecordForHost();
    extensionRecordManager->RegisterPreloadUIExtensionHostClient(token);
    extensionRecordManager->RegisterPreloadUIExtensionHostClient(nullToken);
    extensionRecordManager->UnRegisterPreloadUIExtensionHostClient(pid, deathRecipient);
    extensionRecordManager->ConvertToUnloadExtensionRecords(recordsToUnload, recordsToUnload);
    extensionRecordManager->GetRemoteCallback(validRecord);
    extensionRecordManager->GetRemoteCallback(emptyRecord);
    extensionRecordManager->HandlePreloadUIExtensionLoaded(validRecord);
    extensionRecordManager->HandlePreloadUIExtensionDestroyed(validRecord);
    extensionRecordManager->HandlePreloadUIExtensionSuccess(validExtId, preloadFlag);

    return true;
}
} // namespace OHOS

/* Fuzzer entry code on data */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}