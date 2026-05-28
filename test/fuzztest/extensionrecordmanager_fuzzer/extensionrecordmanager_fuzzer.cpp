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

#include "extensionrecordmanager_fuzzer.h"
#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#include "base_extension_record.h"
#include "extension_record_factory.h"
#define private public
#define inline
#include "extension_record.h"
#include "extension_record_manager.h"
#define inline
#undef private

using namespace OHOS::AAFwk;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LEN = 128;
constexpr int32_t API_GENERATE_RECORD_ID = 0;
constexpr int32_t API_ADD_REMOVE_RECORD = 1;
constexpr int32_t API_ACTIVE_UIEXT_LIST = 2;
constexpr int32_t API_HOST_PID_FOR_EXT_ID = 3;
constexpr int32_t API_AGENT_UI_METHODS = 4;
constexpr int32_t API_PRELOAD_METHODS = 5;
constexpr int32_t API_GET_EXTENSION_RECORD = 6;
constexpr int32_t API_TIMEOUT_METHODS = 7;
constexpr int32_t API_FOCUS_AND_TOKEN = 8;
constexpr int32_t API_SESSION_AND_CALLER = 9;
constexpr int32_t API_START_ABILITY = 10;
constexpr int32_t API_IS_BELONG_TO_MANAGER = 11;
constexpr int32_t API_GET_UIEXT_ROOT_HOST = 12;
constexpr int32_t API_TERMINATED_LIST = 13;
constexpr int32_t API_REGISTER_PRELOAD_CLIENT = 14;
constexpr int32_t MAX_API_CASE = API_REGISTER_PRELOAD_CLIENT;

AbilityRequest CreateTestAbilityRequest(FuzzedDataProvider &fdp)
{
    AbilityRequest request;
    request.abilityInfo.name =
        fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    request.abilityInfo.bundleName =
        fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    request.abilityInfo.moduleName =
        fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    request.abilityInfo.isStageBasedModel = fdp.ConsumeBool();
    request.appInfo.bundleName = request.abilityInfo.bundleName;
    request.appInfo.name =
        fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    return request;
}

ExtensionRecordManager::PreLoadUIExtensionMapKey MakePreLoadKey(
    FuzzedDataProvider &fdp)
{
    auto bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    auto moduleName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    auto abilityName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    pid_t hostPid = fdp.ConsumeIntegral<pid_t>();
    return std::make_tuple(bundleName, moduleName, abilityName, hostPid);
}
} // namespace

void FuzzGenerateRecordId(FuzzedDataProvider &fdp)
{
    auto manager = std::make_shared<ExtensionRecordManager>(0);
    // Generate with a fuzzed ID
    int32_t fuzzedId = fdp.ConsumeIntegral<int32_t>();
    manager->GenerateExtensionRecordId(fuzzedId);
    // Generate again with the same ID to exercise the collision path
    manager->GenerateExtensionRecordId(fuzzedId);
    // Generate with a different ID
    int32_t anotherId = fdp.ConsumeIntegral<int32_t>();
    manager->GenerateExtensionRecordId(anotherId);
    // Generate with zero (boundary value)
    manager->GenerateExtensionRecordId(0);
}

void FuzzAddRemoveRecord(FuzzedDataProvider &fdp)
{
    auto manager = std::make_shared<ExtensionRecordManager>(0);
    int32_t recordId = fdp.ConsumeIntegral<int32_t>();
    // Add a null record
    std::shared_ptr<ExtensionRecord> nullRecord = nullptr;
    manager->AddExtensionRecord(recordId, nullRecord);
    // Remove the record
    manager->RemoveExtensionRecord(recordId);
    // Add then remove with a second ID
    int32_t secondId = fdp.ConsumeIntegral<int32_t>();
    manager->AddExtensionRecord(secondId, nullRecord);
    manager->RemoveExtensionRecord(secondId);
    // Remove non-existent record
    int32_t missingId = fdp.ConsumeIntegral<int32_t>();
    manager->RemoveExtensionRecord(missingId);
}

void FuzzActiveUIExtensionList(FuzzedDataProvider &fdp)
{
    auto manager = std::make_shared<ExtensionRecordManager>(0);
    std::vector<std::string> extensionList;
    // Query by pid
    int32_t pid = fdp.ConsumeIntegral<int32_t>();
    manager->GetActiveUIExtensionList(pid, extensionList);
    extensionList.clear();
    // Query by bundleName
    std::string bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    manager->GetActiveUIExtensionList(bundleName, extensionList);
    extensionList.clear();
    // Query by uid
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    manager->GetActiveUIExtensionListByUid(uid, extensionList);
}

void FuzzHostPidForExtensionId(FuzzedDataProvider &fdp)
{
    auto manager = std::make_shared<ExtensionRecordManager>(0);
    int32_t recordId = fdp.ConsumeIntegral<int32_t>();
    pid_t hostPid = 0;
    manager->GetHostPidForExtensionId(recordId, hostPid);
    // Try with a record that was added
    int32_t addedId = fdp.ConsumeIntegral<int32_t>();
    std::shared_ptr<ExtensionRecord> record = nullptr;
    manager->AddExtensionRecord(addedId, record);
    manager->GetHostPidForExtensionId(addedId, hostPid);
}

void FuzzAgentUIMethods(FuzzedDataProvider &fdp)
{
    auto manager = std::make_shared<ExtensionRecordManager>(0);
    int32_t callerUid = fdp.ConsumeIntegral<int32_t>();
    std::string bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    int32_t extAbilityId = fdp.ConsumeIntegral<int32_t>();
    // Check limit
    manager->CheckAgentUILaunchLimit(callerUid, bundleName);
    // Add launch record
    manager->AddAgentUILaunchRecord(callerUid, bundleName, extAbilityId);
    // Check limit again after adding
    manager->CheckAgentUILaunchLimit(callerUid, bundleName);
    // Remove launch record
    manager->RemoveAgentUILaunchRecord(bundleName, extAbilityId);
    // Remove non-existent record
    std::string otherBundle = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    int32_t otherId = fdp.ConsumeIntegral<int32_t>();
    manager->RemoveAgentUILaunchRecord(otherBundle, otherId);
}

void FuzzPreloadMethods(FuzzedDataProvider &fdp)
{
    auto manager = std::make_shared<ExtensionRecordManager>(0);
    // AddPreloadUIExtensionRecord with null
    std::shared_ptr<BaseExtensionRecord> nullRecord = nullptr;
    manager->AddPreloadUIExtensionRecord(nullRecord);
    // Create a request and ability record for preload operations
    auto request = CreateTestAbilityRequest(fdp);
    auto abilityRecord =
        BaseExtensionRecord::CreateBaseExtensionRecord(request);
    if (abilityRecord) {
        manager->AddPreloadUIExtensionRecord(abilityRecord);
    }
    // QueryPreLoadUIExtensionRecord
    ElementName element(
        fdp.ConsumeRandomLengthString(STRING_MAX_LEN),
        fdp.ConsumeRandomLengthString(STRING_MAX_LEN),
        fdp.ConsumeRandomLengthString(STRING_MAX_LEN),
        fdp.ConsumeRandomLengthString(STRING_MAX_LEN));
    std::string moduleName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    int32_t hostPid = fdp.ConsumeIntegral<int32_t>();
    int32_t recordNum = 0;
    manager->QueryPreLoadUIExtensionRecord(
        element, moduleName, hostPid, recordNum);
    // IsPreloadExtensionRecord
    std::shared_ptr<ExtensionRecord> extRecord = nullptr;
    bool isLoaded = false;
    manager->IsPreloadExtensionRecord(request, hostPid, extRecord, isLoaded);
    // Remove preload records by key
    auto preLoadKey = MakePreLoadKey(fdp);
    manager->RemovePreloadUIExtensionRecord(preLoadKey);
    manager->RemoveAllPreloadUIExtensionRecord(preLoadKey);
    // Remove preload record by key and id
    int32_t recordId = fdp.ConsumeIntegral<int32_t>();
    manager->RemovePreloadUIExtensionRecordById(preLoadKey, recordId);
}

void FuzzGetExtensionRecord(FuzzedDataProvider &fdp)
{
    auto manager = std::make_shared<ExtensionRecordManager>(0);
    std::string hostBundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    std::shared_ptr<ExtensionRecord> extRecord = nullptr;
    bool isLoaded = false;
    // GetExtensionRecord with a fuzzed id
    int32_t recordId = fdp.ConsumeIntegral<int32_t>();
    manager->GetExtensionRecord(recordId, hostBundleName, extRecord, isLoaded);
    // GetExtensionRecordById (private, accessed via public methods)
    manager->GetExtensionRecordById(recordId);
    // Add a record then try to get it
    int32_t addedId = fdp.ConsumeIntegral<int32_t>();
    manager->AddExtensionRecord(addedId, nullptr);
    manager->GetExtensionRecordById(addedId);
    // GetOrCreateExtensionRecord
    auto request = CreateTestAbilityRequest(fdp);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = nullptr;
    isLoaded = false;
    manager->GetOrCreateExtensionRecord(
        request, hostBundleName, abilityRecord, isLoaded);
    // GetOrCreateExtensionRecordInner (private)
    extRecord = nullptr;
    isLoaded = false;
    manager->GetOrCreateExtensionRecordInner(
        request, hostBundleName, extRecord, isLoaded);
}

void FuzzTimeoutMethods(FuzzedDataProvider &fdp)
{
    auto manager = std::make_shared<ExtensionRecordManager>(0);
    int32_t recordId = fdp.ConsumeIntegral<int32_t>();
    // Add record first so timeout can find it
    manager->AddExtensionRecord(recordId, nullptr);
    // Exercise all timeout methods
    manager->LoadTimeout(recordId);
    manager->ForegroundTimeout(recordId);
    manager->BackgroundTimeout(recordId);
    manager->TerminateTimeout(recordId);
    // Timeout on non-existent ID
    int32_t missingId = fdp.ConsumeIntegral<int32_t>();
    manager->LoadTimeout(missingId);
    manager->ForegroundTimeout(missingId);
    manager->BackgroundTimeout(missingId);
    manager->TerminateTimeout(missingId);
}

void FuzzFocusAndToken(FuzzedDataProvider &fdp)
{
    auto manager = std::make_shared<ExtensionRecordManager>(0);
    int32_t recordId = fdp.ConsumeIntegral<int32_t>();
    // IsFocused with null tokens
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> focusToken = nullptr;
    manager->IsFocused(recordId, token, focusToken);
    // SetCachedFocusedCallerToken and GetCachedFocusedCallerToken
    sptr<IRemoteObject> callerToken = nullptr;
    manager->SetCachedFocusedCallerToken(recordId, callerToken);
    manager->GetCachedFocusedCallerToken(recordId);
    // GetRootCallerTokenLocked with null ability record
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    manager->GetRootCallerTokenLocked(recordId, abilityRecord);
}

void FuzzSessionAndCaller(FuzzedDataProvider &fdp)
{
    int32_t recordId = fdp.ConsumeIntegral<int32_t>();
    auto manager = std::make_shared<ExtensionRecordManager>(0);
    // GetAbilityRecordBySessionInfo with null session
    sptr<SessionInfo> sessionInfo = nullptr;
    manager->GetAbilityRecordBySessionInfo(sessionInfo);
    // GetCallerTokenList with null ability record
    std::shared_ptr<AbilityRecord> nullRecord = nullptr;
    std::list<sptr<IRemoteObject>> callerList;
    manager->GetCallerTokenList(nullRecord, callerList);
    // GetUIExtensionSessionInfo with null token
    sptr<IRemoteObject> nullToken = nullptr;
    sptr<IRemoteObject> focusToken = nullptr;
    UIExtensionSessionInfo sessionInfoOut;
    manager->GetUIExtensionSessionInfo(nullToken, sessionInfoOut);
    manager->IsFocused(recordId, nullToken, focusToken);
}

void FuzzStartAbility(FuzzedDataProvider &fdp)
{
    auto manager = std::make_shared<ExtensionRecordManager>(0);
    auto request = CreateTestAbilityRequest(fdp);
    manager->StartAbility(request);
    // Call again with another random request
    auto request2 = CreateTestAbilityRequest(fdp);
    manager->StartAbility(request2);
}

void FuzzIsBelongToManager(FuzzedDataProvider &fdp)
{
    AbilityInfo abilityInfo;
    abilityInfo.name = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    abilityInfo.bundleName =
        fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    abilityInfo.extensionAbilityType =
        static_cast<ExtensionAbilityType>(fdp.ConsumeIntegral<uint16_t>());
    ExtensionRecordManager::IsBelongToManager(abilityInfo);
}

void FuzzGetUIExtRootHost(FuzzedDataProvider &fdp)
{
    auto manager = std::make_shared<ExtensionRecordManager>(0);
    // GetUIExtensionRootHostInfo with null token
    sptr<IRemoteObject> nullToken = nullptr;
    manager->GetUIExtensionRootHostInfo(nullToken);
    // GetUIExtensionRootHostToken with null token
    manager->GetUIExtensionRootHostToken(nullToken);
    // CreateExtensionRecord
    auto request = CreateTestAbilityRequest(fdp);
    std::string hostBundle = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    std::shared_ptr<ExtensionRecord> extRecord = nullptr;
    int32_t hostPid = fdp.ConsumeIntegral<int32_t>();
    manager->CreateExtensionRecord(request, hostBundle, extRecord, hostPid);
}

void FuzzTerminatedList(FuzzedDataProvider &fdp)
{
    auto manager = std::make_shared<ExtensionRecordManager>(0);
    int32_t recordId = fdp.ConsumeIntegral<int32_t>();
    // Add to terminated list
    manager->AddExtensionRecordToTerminatedList(recordId);
    // Add again (duplicate path)
    manager->AddExtensionRecordToTerminatedList(recordId);
    // GetExtensionRecordById should also check terminateRecords_
    manager->GetExtensionRecordById(recordId);
    // Add a different ID to terminated list
    int32_t secondId = fdp.ConsumeIntegral<int32_t>();
    manager->AddExtensionRecordToTerminatedList(secondId);
    manager->GetExtensionRecordById(secondId);
}

void FuzzRegisterPreloadClient(FuzzedDataProvider &fdp)
{
    auto manager = std::make_shared<ExtensionRecordManager>(0);
    // RegisterPreloadUIExtensionHostClient with null token
    sptr<IRemoteObject> nullToken = nullptr;
    manager->RegisterPreloadUIExtensionHostClient(nullToken);
    // HandlePreloadUIExtensionLoaded with null
    std::shared_ptr<ExtensionRecord> nullExtRecord = nullptr;
    manager->HandlePreloadUIExtensionLoaded(nullExtRecord);
    // HandlePreloadUIExtensionDestroyed with null
    manager->HandlePreloadUIExtensionDestroyed(nullExtRecord);
    // HandlePreloadUIExtensionSuccess
    int32_t recordId = fdp.ConsumeIntegral<int32_t>();
    bool isSuccess = fdp.ConsumeBool();
    manager->HandlePreloadUIExtensionSuccess(recordId, isSuccess);
    // ClearPreloadedUIExtensionAbility
    manager->ClearPreloadedUIExtensionAbility(recordId);
    // ClearAllPreloadUIExtensionRecordForHost
    manager->ClearAllPreloadUIExtensionRecordForHost();
}

void DispatchApiCaseSecond(FuzzedDataProvider &fdp, int32_t apiCase);

void DispatchApiCase(FuzzedDataProvider &fdp, int32_t apiCase)
{
    switch (apiCase) {
        case API_GENERATE_RECORD_ID:
            FuzzGenerateRecordId(fdp);
            break;
        case API_ADD_REMOVE_RECORD:
            FuzzAddRemoveRecord(fdp);
            break;
        case API_ACTIVE_UIEXT_LIST:
            FuzzActiveUIExtensionList(fdp);
            break;
        case API_HOST_PID_FOR_EXT_ID:
            FuzzHostPidForExtensionId(fdp);
            break;
        case API_AGENT_UI_METHODS:
            FuzzAgentUIMethods(fdp);
            break;
        case API_PRELOAD_METHODS:
            FuzzPreloadMethods(fdp);
            break;
        case API_GET_EXTENSION_RECORD:
            FuzzGetExtensionRecord(fdp);
            break;
        default:
            DispatchApiCaseSecond(fdp, apiCase);
            break;
    }
}

void DispatchApiCaseSecond(FuzzedDataProvider &fdp, int32_t apiCase)
{
    switch (apiCase) {
        case API_TIMEOUT_METHODS:
            FuzzTimeoutMethods(fdp);
            break;
        case API_FOCUS_AND_TOKEN:
            FuzzFocusAndToken(fdp);
            break;
        case API_SESSION_AND_CALLER:
            FuzzSessionAndCaller(fdp);
            break;
        case API_START_ABILITY:
            FuzzStartAbility(fdp);
            break;
        case API_IS_BELONG_TO_MANAGER:
            FuzzIsBelongToManager(fdp);
            break;
        case API_GET_UIEXT_ROOT_HOST:
            FuzzGetUIExtRootHost(fdp);
            break;
        case API_TERMINATED_LIST:
            FuzzTerminatedList(fdp);
            break;
        case API_REGISTER_PRELOAD_CLIENT:
            FuzzRegisterPreloadClient(fdp);
            break;
        default:
            break;
    }
}

bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    auto apiCase = fdp.ConsumeIntegralInRange<int32_t>(0, MAX_API_CASE);
    DispatchApiCase(fdp, apiCase);
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
