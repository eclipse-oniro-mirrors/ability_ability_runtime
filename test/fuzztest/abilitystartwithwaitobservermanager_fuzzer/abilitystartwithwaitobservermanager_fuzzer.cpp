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

#include "abilitystartwithwaitobservermanager_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#define private public
#include "ability_start_with_wait_observer_manager.h"
#undef private

#include "ability_record.h"

#include "ability_start_with_wait_observer_stub.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr int32_t API_REGISTER_OBSERVER = 0;
constexpr int32_t API_UNREGISTER_OBSERVER = 1;
constexpr int32_t API_NOTIFY_TERMINATE_BY_WANT = 2;
constexpr int32_t API_NOTIFY_TERMINATE_BY_RECORD = 3;
constexpr int32_t API_SET_COLD_START = 4;
constexpr int32_t API_GENERATE_DEATH_RECIPIENT = 5;
constexpr int32_t API_GET_INSTANCE = 6;
constexpr int32_t MAX_API_CASE = API_GET_INSTANCE;
constexpr int32_t TERMINATE_REASON_MIN = 0;
constexpr int32_t TERMINATE_REASON_MAX = 2;
constexpr int32_t INVALID_OBSERVER_ID = -1;
constexpr size_t STRING_MAX_LEN = 128;

class FuzzObserverStub : public AbilityStartWithWaitObserverStub {
public:
    FuzzObserverStub() = default;
    virtual ~FuzzObserverStub() = default;
    int32_t NotifyAATerminateWait(
        const AbilityStartWithWaitObserverData &data) override
    {
        return 0;
    }
};
} // namespace

void FuzzRegisterObserver(FuzzedDataProvider &fdp)
{
    auto &mgr = AbilityStartWithWaitObserverManager::GetInstance();
    Want want;
    std::string bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    std::string abilityName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    want.SetElementName(bundleName, abilityName);
    // null observer
    sptr<IAbilityStartWithWaitObserver> nullObs = nullptr;
    mgr.RegisterObserver(want, nullObs);
    // valid observer
    sptr<IAbilityStartWithWaitObserver> obs = new FuzzObserverStub();
    mgr.RegisterObserver(want, obs);
    // duplicate registration
    mgr.RegisterObserver(want, obs);
}

void FuzzUnregisterObserver(FuzzedDataProvider &fdp)
{
    auto &mgr = AbilityStartWithWaitObserverManager::GetInstance();
    // null
    mgr.UnregisterObserver(nullptr);
    // not registered
    sptr<IAbilityStartWithWaitObserver> obs2 = new FuzzObserverStub();
    mgr.UnregisterObserver(obs2);
    // register then unregister
    Want want;
    std::string bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    std::string abilityName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    want.SetElementName(bundleName, abilityName);
    sptr<IAbilityStartWithWaitObserver> obs3 = new FuzzObserverStub();
    mgr.RegisterObserver(want, obs3);
    mgr.UnregisterObserver(obs3);
}

void FuzzNotifyTerminateByWant(FuzzedDataProvider &fdp)
{
    auto &mgr = AbilityStartWithWaitObserverManager::GetInstance();
    auto reasonIdx = fdp.ConsumeIntegralInRange<int32_t>(
        TERMINATE_REASON_MIN, TERMINATE_REASON_MAX);
    auto reason = static_cast<TerminateReason>(reasonIdx);
    // invalid observer id
    Want wantNoId;
    mgr.NotifyAATerminateWait(wantNoId, reason);
    // register then notify with want containing observer id
    Want want;
    sptr<IAbilityStartWithWaitObserver> obs = new FuzzObserverStub();
    mgr.RegisterObserver(want, obs);
    mgr.NotifyAATerminateWait(want, reason);
}

void FuzzNotifyTerminateByRecord(FuzzedDataProvider &fdp)
{
    auto &mgr = AbilityStartWithWaitObserverManager::GetInstance();
    auto reasonIdx = fdp.ConsumeIntegralInRange<int32_t>(
        TERMINATE_REASON_MIN, TERMINATE_REASON_MAX);
    auto reason = static_cast<TerminateReason>(reasonIdx);
    // null record
    mgr.NotifyAATerminateWait(nullptr, reason);
    // record without observer id
    AbilityRequest request;
    request.appInfo.bundleName = "com.example.fuzzTest";
    request.abilityInfo.name = "MainAbility";
    auto record = AbilityRecord::CreateAbilityRecord(request);
    if (!record) {
        return;
    }
    mgr.NotifyAATerminateWait(record, reason);
    // register then notify with record containing valid observer id
    Want want;
    sptr<IAbilityStartWithWaitObserver> obs = new FuzzObserverStub();
    mgr.RegisterObserver(want, obs);
    int32_t obsId = want.GetIntParam(
        Want::START_ABILITY_WITH_WAIT_OBSERVER_ID_KEY,
        INVALID_OBSERVER_ID);
    // Set observer ID on request want before creating record
    AbilityRequest requestWithId;
    requestWithId.appInfo.bundleName = "com.example.fuzzTest";
    requestWithId.abilityInfo.name = "MainAbility";
    requestWithId.want.SetParam(
        Want::START_ABILITY_WITH_WAIT_OBSERVER_ID_KEY, obsId);
    auto recordWithId = AbilityRecord::CreateAbilityRecord(requestWithId);
    if (!recordWithId) {
        return;
    }
    mgr.NotifyAATerminateWait(recordWithId, reason);
}

void FuzzSetColdStart(FuzzedDataProvider &fdp)
{
    auto &mgr = AbilityStartWithWaitObserverManager::GetInstance();
    // null record
    mgr.SetColdStartForShellCall(nullptr);
    // record without observer id
    AbilityRequest request;
    request.appInfo.bundleName = "com.example.fuzzTest";
    request.abilityInfo.name = "MainAbility";
    auto record = AbilityRecord::CreateAbilityRecord(request);
    if (!record) {
        return;
    }
    mgr.SetColdStartForShellCall(record);
    // register then set cold start with record containing observer id
    Want want;
    std::string bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    std::string abilityName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    want.SetElementName(bundleName, abilityName);
    sptr<IAbilityStartWithWaitObserver> obs = new FuzzObserverStub();
    mgr.RegisterObserver(want, obs);
    int32_t obsId = want.GetIntParam(
        Want::START_ABILITY_WITH_WAIT_OBSERVER_ID_KEY,
        INVALID_OBSERVER_ID);
    // Set observer ID on request want before creating record
    AbilityRequest requestWithId;
    requestWithId.appInfo.bundleName = "com.example.fuzzTest";
    requestWithId.abilityInfo.name = "MainAbility";
    requestWithId.want.SetParam(
        Want::START_ABILITY_WITH_WAIT_OBSERVER_ID_KEY, obsId);
    auto recordWithId = AbilityRecord::CreateAbilityRecord(requestWithId);
    if (!recordWithId) {
        return;
    }
    mgr.SetColdStartForShellCall(recordWithId);
}

void FuzzGenerateDeathRecipient(FuzzedDataProvider &fdp)
{
    Want want;
    std::string bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    std::string abilityName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    want.SetElementName(bundleName, abilityName);
    auto &mgr = AbilityStartWithWaitObserverManager::GetInstance();
    // null observer
    mgr.GenerateDeathRecipient(nullptr);
    // valid observer
    sptr<IAbilityStartWithWaitObserver> obs = new FuzzObserverStub();
    mgr.GenerateDeathRecipient(obs);
    mgr.RegisterObserver(want, obs);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    auto apiCase = fdp.ConsumeIntegralInRange<int32_t>(0, MAX_API_CASE);
    switch (apiCase) {
        case API_REGISTER_OBSERVER:
            FuzzRegisterObserver(fdp);
            break;
        case API_UNREGISTER_OBSERVER:
            FuzzUnregisterObserver(fdp);
            break;
        case API_NOTIFY_TERMINATE_BY_WANT:
            FuzzNotifyTerminateByWant(fdp);
            break;
        case API_NOTIFY_TERMINATE_BY_RECORD:
            FuzzNotifyTerminateByRecord(fdp);
            break;
        case API_SET_COLD_START:
            FuzzSetColdStart(fdp);
            break;
        case API_GENERATE_DEATH_RECIPIENT:
            FuzzGenerateDeathRecipient(fdp);
            break;
        case API_GET_INSTANCE:
            AbilityStartWithWaitObserverManager::GetInstance();
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
