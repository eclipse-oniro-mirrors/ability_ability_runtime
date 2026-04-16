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

#include "dataabilityrecordsecond_fuzzer.h"

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
class IRemoteObjectFuzz : public IRemoteObject {
public:
    IRemoteObjectFuzz() : IRemoteObject {u"IRemoteObjectFuzz"}
    {
    }

    ~IRemoteObjectFuzz()
    {
    }

    int32_t GetObjectRefCount()
    {
        return 0;
    }

    int SendRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
    {
        return 0;
    }

    bool IsProxyObject() const
    {
        return true;
    }

    bool CheckObjectLegality() const
    {
        return true;
    }

    bool AddDeathRecipient(const sptr<DeathRecipient>& recipient)
    {
        return true;
    }

    bool RemoveDeathRecipient(const sptr<DeathRecipient>& recipient)
    {
        return true;
    }

    bool AddRefreshRecipient(const sptr<RefreshRecipient> &recipient)
    {
        return true;
    }

    bool RemoveRefreshRecipient(const sptr<RefreshRecipient> &recipient)
    {
        return true;
    }

    sptr<IRemoteBroker> AsInterface()
    {
        return nullptr;
    }

    int Dump(int fd, const std::vector<std::u16string>& args)
    {
        return 0;
    }
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

    dataAbilityRecord->AddClient(client, tryBind, isNotHap);
    dataAbilityRecord->StartLoading();
    dataAbilityRecord->GetScheduler();
    dataAbilityRecord->AddClient(client, tryBind, isNotHap);
    dataAbilityRecord->GetClientCount(client);
    dataAbilityRecord->Dump();
    dataAbilityRecord->Dump(info);
    dataAbilityRecord->RemoveClient(client, isNotHap);
    dataAbilityRecord->RemoveClients(abilityRecordClient);
    dataAbilityRecord->KillBoundClientProcesses();
    client = new IRemoteObjectFuzz();
    dataAbilityRecord->AddClient(client, tryBind, isNotHap);
    dataAbilityRecord->GetClientCount(client);
    dataAbilityRecord->Dump();
    dataAbilityRecord->Dump(info);
    dataAbilityRecord->RemoveClient(client, isNotHap);
    dataAbilityRecord->RemoveClients(abilityRecordClient);
    dataAbilityRecord->KillBoundClientProcesses();
    DelayedSingleton<AppScheduler>::GetInstance();
    dataAbilityRecord->AddClient(client, tryBind, isNotHap);
    dataAbilityRecord->GetClientCount(client);
    dataAbilityRecord->Dump();
    dataAbilityRecord->Dump(info);
    dataAbilityRecord->KillBoundClientProcesses();
    dataAbilityRecord->GetRequest();
    dataAbilityRecord->GetAbilityRecord();
    dataAbilityRecord->GetToken();
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