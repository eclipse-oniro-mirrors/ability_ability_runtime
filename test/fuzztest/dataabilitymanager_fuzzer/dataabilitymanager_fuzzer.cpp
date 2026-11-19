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

#include "dataabilitymanager_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <cstring>
#include <string>
#include <vector>
#include <fuzzer/FuzzedDataProvider.h>

#define private public
#include "data_ability_manager.h"
#undef private

#include "ability_record.h"
#include "ability_scheduler_interface.h"
#include "iremote_stub.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LENGTH = 128;
constexpr int32_t API_ACQUIRE = 0;
constexpr int32_t API_RELEASE_AND_CONTAINS = 1;
constexpr int32_t API_ATTACH_AND_TRANSITION = 2;
constexpr int32_t API_DEATH_AND_STATE_CHANGE = 3;
constexpr int32_t API_QUERY_AND_DUMP = 4;
constexpr int32_t API_DUMP_STATE = 5;
constexpr int32_t API_OTHER = 6;
constexpr int32_t API_LOAD_LOCKED = 7;
constexpr int32_t API_RESTART = 8;
constexpr int32_t MAX_API_CASE = API_RESTART;

class FuzzAbilityScheduler : public IRemoteStub<IAbilityScheduler> {
public:
    bool ScheduleAbilityTransaction(const Want &want,
        const LifeCycleStateInfo &targetState,
        sptr<SessionInfo> sessionInfo = nullptr) override
    {
        return true;
    }
    void ScheduleShareData(const int32_t &uniqueId) override {}
    void SendResult(int requestCode, int resultCode,
        const Want &resultWant) override {}
    void ScheduleConnectAbility(const Want &want) override {}
    void ScheduleDisconnectAbility(const Want &want) override {}
    void ScheduleCommandAbility(const Want &want,
        bool restart, int startId) override {}
    void ScheduleCommandAbilityWindow(const Want &want,
        const sptr<SessionInfo> &sessionInfo,
        WindowCommand winCmd) override {}
    bool SchedulePrepareTerminateAbility() override { return false; }
    void ScheduleSaveAbilityState() override {}
    void ScheduleRestoreAbilityState(const PacMap &inState) override {}
    std::vector<std::string> GetFileTypes(const Uri &uri,
        const std::string &mimeTypeFilter) override { return {}; }
    int OpenFile(const Uri &uri,
        const std::string &mode) override { return 0; }
    int OpenRawFile(const Uri &uri,
        const std::string &mode) override { return 0; }
    int Insert(const Uri &uri,
        const NativeRdb::ValuesBucket &value) override { return 0; }
    int Update(const Uri &uri,
        const NativeRdb::ValuesBucket &value,
        const NativeRdb::DataAbilityPredicates &predicates) override
    {
        return 0;
    }
    int Delete(const Uri &uri,
        const NativeRdb::DataAbilityPredicates &predicates) override
    {
        return 0;
    }
    std::shared_ptr<PacMap> Call(const Uri &uri,
        const std::string &method, const std::string &arg,
        const PacMap &pacMap) override { return nullptr; }
    std::shared_ptr<NativeRdb::AbsSharedResultSet> Query(
        const Uri &uri, std::vector<std::string> &columns,
        const NativeRdb::DataAbilityPredicates &predicates) override
    {
        return nullptr;
    }
    std::string GetType(const Uri &uri) override { return ""; }
    bool Reload(const Uri &uri, const PacMap &extras) override
    {
        return false;
    }
    int BatchInsert(const Uri &uri,
        const std::vector<NativeRdb::ValuesBucket> &values) override
    {
        return 0;
    }
    bool ScheduleRegisterObserver(const Uri &uri,
        const sptr<IDataAbilityObserver> &dataObserver) override
    {
        return false;
    }
    bool ScheduleUnregisterObserver(const Uri &uri,
        const sptr<IDataAbilityObserver> &dataObserver) override
    {
        return false;
    }
    bool ScheduleNotifyChange(const Uri &uri) override { return false; }
    Uri NormalizeUri(const Uri &uri) override { return Uri(""); }
    Uri DenormalizeUri(const Uri &uri) override { return Uri(""); }
    std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>> ExecuteBatch(
        const std::vector<std::shared_ptr<
            AppExecFwk::DataAbilityOperation>> &operations) override
    {
        return {};
    }
    void ContinueAbility(const std::string &deviceId,
        uint32_t versionCode) override {}
    void NotifyContinuationResult(int32_t result) override {}
    void DumpAbilityInfo(const std::vector<std::string> &params,
        std::vector<std::string> &info) override {}
    int CreateModalUIExtension(const Want &want) override { return 0; }
    void OnExecuteIntent(const Want &want) override {}
    void CallRequest() override {}
    void UpdateSessionToken(
        sptr<IRemoteObject> sessionToken) override {}
    void ScheduleCollaborate(const Want &want) override {}
    void ScheduleAbilityRequestFailure(const std::string &requestId,
        const AppExecFwk::ElementName &element,
        const std::string &message, int32_t resultCode) override {}
    void ScheduleAbilityRequestSuccess(const std::string &requestId,
        const AppExecFwk::ElementName &element) override {}
    void ScheduleAbilitiesRequestDone(const std::string &requestKey,
        int32_t resultCode) override {}
};

AbilityRequest CreateDataAbilityRequest()
{
    AbilityRequest request;
    request.appInfo.bundleName = "com.example.fuzzTest";
    request.abilityInfo.name = "DataAbility";
    request.abilityInfo.type = AbilityType::DATA;
    return request;
}

void SetupLoadedRecord(DataAbilityManager &mgr,
    const std::string &name, sptr<IAbilityScheduler> scheduler)
{
    auto request = CreateDataAbilityRequest();
    auto record = std::make_shared<DataAbilityRecord>(request);
    auto ability = AbilityRecord::CreateAbilityRecord(request);
    if (!ability || !record) {
        return;
    }
    ability->SetAbilityState(ACTIVE);
    record->ability_ = ability;
    record->scheduler_ = scheduler;
    mgr.dataAbilityRecordsLoaded_[name] = record;
}

void SetupLoadingRecord(DataAbilityManager &mgr,
    const std::string &name, sptr<IAbilityScheduler> scheduler)
{
    auto request = CreateDataAbilityRequest();
    request.abilityInfo.name = "LoadingAbility";
    auto record = std::make_shared<DataAbilityRecord>(request);
    auto ability = AbilityRecord::CreateAbilityRecord(request);
    if (!ability || !record) {
        return;
    }
    record->ability_ = ability;
    record->scheduler_ = scheduler;
    mgr.dataAbilityRecordsLoading_[name] = record;
}
} // namespace

void FuzzAcquireApis(FuzzedDataProvider &fdp, DataAbilityManager &mgr)
{
    // wrong type
    AbilityRequest wrongReq;
    wrongReq.abilityInfo.type = AbilityType::PAGE;
    mgr.Acquire(wrongReq, false, nullptr, false);
    // empty name
    AbilityRequest emptyReq;
    emptyReq.abilityInfo.type = AbilityType::DATA;
    mgr.Acquire(emptyReq, false, nullptr, false);
    // valid request without client
    auto validReq = CreateDataAbilityRequest();
    bool tryBind = fdp.ConsumeBool();
    bool isNotHap = fdp.ConsumeBool();
    mgr.Acquire(validReq, tryBind, nullptr, isNotHap);
    // valid request with null client token
    sptr<IRemoteObject> client;
    mgr.Acquire(validReq, tryBind, client, isNotHap);
}

void FuzzReleaseAndContains(DataAbilityManager &mgr,
    sptr<IAbilityScheduler> scheduler)
{
    // null params
    mgr.Release(nullptr, nullptr, false);
    // scheduler not in map
    sptr<IRemoteObject> client;
    mgr.Release(scheduler, client, false);
    // ContainsDataAbility: null
    mgr.ContainsDataAbility(nullptr);
    // ContainsDataAbility: found
    mgr.ContainsDataAbility(scheduler);
    // ContainsDataAbility: not found
    sptr<IAbilityScheduler> otherScheduler;
    mgr.ContainsDataAbility(otherScheduler);
}

void FuzzAttachAndTransition(DataAbilityManager &mgr,
    sptr<IAbilityScheduler> scheduler)
{
    const std::string loadingName = "com.example.fuzzTest.LoadingAbility";
    // null params
    mgr.AttachAbilityThread(nullptr, nullptr);
    // valid attach with loading record
    auto loadingIt = mgr.dataAbilityRecordsLoading_.find(loadingName);
    if (loadingIt != mgr.dataAbilityRecordsLoading_.end()) {
        auto token = loadingIt->second->GetToken();
        mgr.AttachAbilityThread(scheduler, token);
    }
    // null token for transition
    mgr.AbilityTransitionDone(nullptr, 0);
    // valid transition with loading record
    if (loadingIt != mgr.dataAbilityRecordsLoading_.end()) {
        auto token = loadingIt->second->GetToken();
        mgr.AbilityTransitionDone(
            token, AbilityLifeCycleState::ABILITY_STATE_ACTIVE);
    }
}

void FuzzDeathAndStateChange(DataAbilityManager &mgr)
{
    auto request = CreateDataAbilityRequest();
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(request);
    if (abilityRecord) {
        mgr.OnAbilityDied(abilityRecord);
    }
    // OnAppStateChanged with empty info
    AppInfo appInfo;
    mgr.OnAppStateChanged(appInfo);
    // OnAppStateChanged with matching info
    const std::string loadedName = "com.example.fuzzTest.DataAbility";
    auto it = mgr.dataAbilityRecordsLoaded_.find(loadedName);
    if (it != mgr.dataAbilityRecordsLoaded_.end()) {
        auto rec = it->second->GetAbilityRecord();
        if (rec) {
            appInfo.bundleName = rec->GetApplicationInfo().bundleName;
            appInfo.appIndex = rec->GetAppIndex();
            appInfo.instanceKey = rec->GetInstanceKey();
            mgr.OnAppStateChanged(appInfo);
        }
    }
}

void FuzzQueryAndDump(DataAbilityManager &mgr,
    sptr<IAbilityScheduler> scheduler, const std::string &args)
{
    // GetAbilityRecordById
    mgr.GetAbilityRecordById(0);
    // GetAbilityRecordByToken: null
    mgr.GetAbilityRecordByToken(nullptr);
    // GetAbilityRecordByToken: valid
    const std::string loadedName = "com.example.fuzzTest.DataAbility";
    auto it = mgr.dataAbilityRecordsLoaded_.find(loadedName);
    if (it != mgr.dataAbilityRecordsLoaded_.end()) {
        auto token = it->second->GetToken();
        mgr.GetAbilityRecordByToken(token);
    }
    // GetAbilityRecordByScheduler: null
    mgr.GetAbilityRecordByScheduler(nullptr);
    // GetAbilityRecordByScheduler: valid
    mgr.GetAbilityRecordByScheduler(scheduler);
    // Dump
    auto func = std::make_unique<char[]>(args.length() + 1);
    if (memcpy_s(func.get(), args.length() + 1,
        args.data(), args.length()) == EOK) {
        func[args.length()] = '\0';
        mgr.Dump(static_cast<const char *>(func.get()), 0);
    }
}

void FuzzDumpStateApis(DataAbilityManager &mgr,
    const std::string &args)
{
    std::vector<std::string> info;
    // DumpState with empty args
    mgr.DumpState(info, "");
    // DumpState with args
    mgr.DumpState(info, args);
    // DumpSysState with empty args
    mgr.DumpSysState(info, true, "");
    // DumpSysState with args
    mgr.DumpSysState(info, false, args);
    // DumpClientInfo with null record
    std::shared_ptr<DataAbilityRecord> nullRecord;
    mgr.DumpClientInfo(info, true, nullRecord);
    // DumpClientInfo with valid record
    const std::string loadedName = "com.example.fuzzTest.DataAbility";
    auto it = mgr.dataAbilityRecordsLoaded_.find(loadedName);
    if (it != mgr.dataAbilityRecordsLoaded_.end()) {
        mgr.DumpClientInfo(info, true, it->second);
    }
}

void FuzzOtherApis(DataAbilityManager &mgr)
{
    // OnAbilityRequestDone
    mgr.OnAbilityRequestDone(nullptr, 0);
    // GetAbilityRunningInfos
    std::vector<AbilityRunningInfo> runningInfos;
    mgr.GetAbilityRunningInfos(runningInfos, true);
    mgr.GetAbilityRunningInfos(runningInfos, false);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    auto mgr = std::make_shared<DataAbilityManager>();
    auto scheduler = sptr<IAbilityScheduler>(new FuzzAbilityScheduler());
    const std::string loadedName = "com.example.fuzzTest.DataAbility";
    const std::string loadingName = "com.example.fuzzTest.LoadingAbility";
    SetupLoadedRecord(*mgr, loadedName, scheduler);
    SetupLoadingRecord(*mgr, loadingName, scheduler);
    auto stringParam = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    auto apiCase = fdp.ConsumeIntegralInRange<int32_t>(0, MAX_API_CASE);
    switch (apiCase) {
        case API_ACQUIRE:
            FuzzAcquireApis(fdp, *mgr);
            break;
        case API_RELEASE_AND_CONTAINS:
            FuzzReleaseAndContains(*mgr, scheduler);
            break;
        case API_ATTACH_AND_TRANSITION:
            FuzzAttachAndTransition(*mgr, scheduler);
            break;
        case API_DEATH_AND_STATE_CHANGE:
            FuzzDeathAndStateChange(*mgr);
            break;
        case API_QUERY_AND_DUMP:
            FuzzQueryAndDump(*mgr, scheduler, stringParam);
            break;
        case API_DUMP_STATE:
            FuzzDumpStateApis(*mgr, stringParam);
            break;
        case API_OTHER:
            FuzzOtherApis(*mgr);
            break;
        case API_LOAD_LOCKED:
            mgr->LoadLocked(loadedName, CreateDataAbilityRequest());
            break;
        case API_RESTART:
            mgr->RestartDataAbility(nullptr);
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
