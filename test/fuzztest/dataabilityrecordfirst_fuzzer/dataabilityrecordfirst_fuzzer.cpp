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

#include "dataabilityrecordfirst_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#define private public
#include "data_ability_record.h"
#undef private

#include "ability_fuzz_util.h"

using namespace std::chrono;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LENGTH = 128;
}
class AbilitySchedulerFuzzTest : public IRemoteStub<IAbilityScheduler> {
public:
    AbilitySchedulerFuzzTest() = default;
    virtual ~AbilitySchedulerFuzzTest()
    {};
    bool ScheduleAbilityTransaction(const Want& want, const LifeCycleStateInfo& targetState,
        sptr<SessionInfo> sessionInfo = nullptr) override
    {
        return true;
    }
    void ScheduleShareData(const int32_t &uniqueId) override
    {}
    void SendResult(int requestCode, int resultCode, const Want& resultWant) override
    {}
    void ScheduleConnectAbility(const Want& want) override
    {}
    void ScheduleDisconnectAbility(const Want& want) override
    {}
    void ScheduleCommandAbility(const Want& want, bool restart, int startId) override
    {}
    void ScheduleCommandAbilityWindow(const Want &want, const sptr<SessionInfo> &sessionInfo,
        WindowCommand winCmd) override
    {}
    bool SchedulePrepareTerminateAbility() override
    {
        return false;
    }
    void ScheduleSaveAbilityState() override
    {}
    void ScheduleRestoreAbilityState(const PacMap& inState) override
    {}
    std::vector<std::string> GetFileTypes(const Uri& uri, const std::string& mimeTypeFilter) override
    {
        return {};
    }
    int OpenFile(const Uri& uri, const std::string& mode) override
    {
        return 0;
    }
    int OpenRawFile(const Uri& uri, const std::string& mode) override
    {
        return 0;
    }
    int Insert(const Uri& uri, const NativeRdb::ValuesBucket& value) override
    {
        return 0;
    }
    int Update(const Uri& uri, const NativeRdb::ValuesBucket& value,
        const NativeRdb::DataAbilityPredicates& predicates) override
    {
        return 0;
    }
    int Delete(const Uri& uri, const NativeRdb::DataAbilityPredicates& predicates) override
    {
        return 0;
    }
    std::shared_ptr<AppExecFwk::PacMap> Call(
        const Uri& uri, const std::string& method, const std::string& arg, const AppExecFwk::PacMap& pacMap) override
    {
        return {};
    }
    std::shared_ptr<NativeRdb::AbsSharedResultSet> Query(const Uri& uri,
        std::vector<std::string>& columns, const NativeRdb::DataAbilityPredicates& predicates) override
    {
        return {};
    }
    std::string GetType(const Uri& uri) override
    {
        return {};
    }
    bool Reload(const Uri& uri, const PacMap& extras) override
    {
        return true;
    }
    int BatchInsert(const Uri& uri, const std::vector<NativeRdb::ValuesBucket>& values) override
    {
        return 0;
    }
    bool ScheduleRegisterObserver(const Uri& uri, const sptr<IDataAbilityObserver>& dataObserver) override
    {
        return true;
    }
    bool ScheduleUnregisterObserver(const Uri& uri, const sptr<IDataAbilityObserver>& dataObserver) override
    {
        return true;
    }
    bool ScheduleNotifyChange(const Uri& uri) override
    {
        return true;
    }
    Uri NormalizeUri(const Uri& uri) override
    {
        return Uri{ "abilityschedulerstub" };
    }

    Uri DenormalizeUri(const Uri& uri) override
    {
        return Uri{ "abilityschedulerstub" };
    }
    std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>> ExecuteBatch(
        const std::vector<std::shared_ptr<AppExecFwk::DataAbilityOperation>>& operations) override
    {
        return {};
    }
    void ContinueAbility(const std::string& deviceId, uint32_t versionCode) override
    {}
    void NotifyContinuationResult(int32_t result) override
    {}
    void DumpAbilityInfo(const std::vector<std::string>& params, std::vector<std::string>& info) override
    {}
    void UpdateSessionToken(sptr<IRemoteObject> sessionToken) override
    {}
    void OnExecuteIntent(const Want &want) override
    {}
    int CreateModalUIExtension(const Want &want) override
    {
        return 0;
    }
    void CallRequest() override
    {
        return;
    }
    void ScheduleCollaborate(const Want &want) override
    {}

    void ScheduleAbilityRequestFailure(const std::string &requestId, const AppExecFwk::ElementName &element,
        const std::string &message, int32_t resultCode) override
    {}

    void ScheduleAbilityRequestSuccess(const std::string &requestId, const AppExecFwk::ElementName &element) override
    {}

    void ScheduleAbilitiesRequestDone(const std::string &requestKey, int32_t resultCode) override
    {}
};

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.fuzzTest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    ffrt::mutex mutex;
    system_clock::duration timeout = 800ms;
    sptr<IAbilityScheduler> scheduler;
    int state;
    sptr<IRemoteObject> client;
    bool tryBind;
    bool isNotHap;
    std::shared_ptr<AbilityRecord> abilityRecordClient;
    std::vector<std::string> info;
    wptr<IRemoteObject> remote;
    sptr<IRemoteObject> callerRemote;
    FuzzedDataProvider fdp(data, size);
    state = fdp.ConsumeIntegral<int>();
    tryBind = fdp.ConsumeBool();
    isNotHap = fdp.ConsumeBool();
    info = AbilityFuzzUtil::GenerateStringArray(fdp);

    dataAbilityRecord->StartLoading();
    dataAbilityRecord->GetScheduler();
    mutex.lock();
    dataAbilityRecord->WaitForLoaded(mutex, timeout);
    mutex.unlock();
    dataAbilityRecord->Attach(scheduler);
    scheduler = new AbilitySchedulerFuzzTest();
    dataAbilityRecord->Attach(scheduler);
    dataAbilityRecord->OnTransitionDone(state);
    dataAbilityRecord->AddClient(client, tryBind, isNotHap);
    dataAbilityRecord->GetClientCount(client);
    dataAbilityRecord->KillBoundClientProcesses();
    dataAbilityRecord->RemoveClient(client, isNotHap);
    dataAbilityRecord->RemoveClients(abilityRecordClient);
    dataAbilityRecord->GetRequest();
    dataAbilityRecord->GetAbilityRecord();
    dataAbilityRecord->GetToken();
    dataAbilityRecord->Dump();
    dataAbilityRecord->Dump(info);
    dataAbilityRecord->GetDiedCallerPid(callerRemote);
    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // Run your code on data.
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}