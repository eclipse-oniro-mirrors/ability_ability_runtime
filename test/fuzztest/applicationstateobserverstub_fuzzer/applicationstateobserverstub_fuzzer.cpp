/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include "applicationstateobserverstub_fuzzer.h"
#include "attack_vectors.h"
#include "fuzz_util.h"
#include "application_state_observer_stub.h"
#include "iapplication_state_observer.h"

using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {
class ApplicationStateObserverStubFuzz : public ApplicationStateObserverStub {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 20) {
        case 0:
            actualCode = static_cast<uint32_t>(
                IApplicationStateObserver::Message::TRANSACT_ON_FOREGROUND_APPLICATION_CHANGED);
            FuzzUtil::WriteMaliciousAppStateDataPhased(parcel, fdp,
                fdp.ConsumeBool() ? OHOS::FuzzUtil::PHASE_BEFORE_SERIALIZE
                                   : OHOS::FuzzUtil::PHASE_AFTER_DESERIALIZE);
            break;
        case 1:
            actualCode = static_cast<uint32_t>(
                IApplicationStateObserver::Message::TRANSACT_ON_ABILITY_STATE_CHANGED);
            FuzzUtil::WriteMaliciousAbilityStateData(parcel, fdp);
            break;
        case 2:
            actualCode = static_cast<uint32_t>(
                IApplicationStateObserver::Message::TRANSACT_ON_PROCESS_CREATED);
            FuzzUtil::WriteMaliciousProcessData(parcel, fdp);
            break;
        case 3:
            actualCode = static_cast<uint32_t>(
                IApplicationStateObserver::Message::TRANSACT_ON_PROCESS_DIED);
            FuzzUtil::WriteMaliciousProcessData(parcel, fdp);
            break;
        case 4:
            actualCode = static_cast<uint32_t>(
                IApplicationStateObserver::Message::TRANSACT_ON_PAGE_SHOW);
            FuzzUtil::WriteMaliciousPageStateData(parcel, fdp);
            break;
        case 5:
            actualCode = static_cast<uint32_t>(
                IApplicationStateObserver::Message::TRANSACT_ON_WINDOW_SHOW);
            FuzzUtil::WriteMaliciousProcessData(parcel, fdp);
            break;
        case 6:
            actualCode = static_cast<uint32_t>(
                IApplicationStateObserver::Message::TRANSACT_ON_WINDOW_HIDDEN);
            FuzzUtil::WriteMaliciousProcessData(parcel, fdp);
            break;
        case 7:
            actualCode = static_cast<uint32_t>(
                IApplicationStateObserver::Message::TRANSACT_ON_EXTENSION_STATE_CHANGED);
            FuzzUtil::WriteMaliciousAbilityStateData(parcel, fdp);
            break;
        case 8:
            actualCode = static_cast<uint32_t>(
                IApplicationStateObserver::Message::TRANSACT_ON_APPLICATION_STATE_CHANGED);
            FuzzUtil::WriteMaliciousAppStateData(parcel, fdp);
            break;
        case 9:
            actualCode = static_cast<uint32_t>(
                IApplicationStateObserver::Message::TRANSACT_ON_APP_STATE_CHANGED);
            FuzzUtil::WriteMaliciousAppStateData(parcel, fdp);
            break;
        case 10:
            actualCode = static_cast<uint32_t>(
                IApplicationStateObserver::Message::TRANSACT_ON_PROCESS_REUSED);
            FuzzUtil::WriteMaliciousProcessData(parcel, fdp);
            break;
        case 11:
            actualCode = static_cast<uint32_t>(
                IApplicationStateObserver::Message::TRANSACT_ON_APP_STARTED);
            FuzzUtil::WriteMaliciousAppStateData(parcel, fdp);
            break;
        case 12:
            actualCode = static_cast<uint32_t>(
                IApplicationStateObserver::Message::TRANSACT_ON_APP_STOPPED);
            FuzzUtil::WriteMaliciousAppStateData(parcel, fdp);
            break;
        case 13:
            actualCode = static_cast<uint32_t>(
                IApplicationStateObserver::Message::TRANSACT_ON_PROCESS_STATE_CHANGED);
            FuzzUtil::WriteMaliciousProcessData(parcel, fdp);
            break;
        case 14:
            actualCode = static_cast<uint32_t>(
                IApplicationStateObserver::Message::TRANSACT_ON_PAGE_HIDE);
            FuzzUtil::WriteMaliciousPageStateData(parcel, fdp);
            break;
        case 15:
            actualCode = static_cast<uint32_t>(
                IApplicationStateObserver::Message::TRANSACT_ON_APP_CACHE_STATE_CHANGED);
            FuzzUtil::WriteMaliciousAppStateData(parcel, fdp);
            break;
        case 16:
            actualCode = static_cast<uint32_t>(
                IApplicationStateObserver::Message::TRANSACT_ON_PROCESS_BINDINGRELATION_CHANGED);
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteBool(fdp.ConsumeBool());
            parcel.WriteInt32(FuzzUtil::BuildInvalidEnum(fdp, 20));
            parcel.WriteInt32(FuzzUtil::BuildInvalidEnum(fdp, 20));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
            break;
        case 17:
            actualCode = static_cast<uint32_t>(
                IApplicationStateObserver::Message::TRANSACT_ON_PROCESS_TYPE_CHANGED);
            FuzzUtil::WriteMaliciousProcessData(parcel, fdp);
            break;
        case 18:
            actualCode = static_cast<uint32_t>(
                IApplicationStateObserver::Message::TRANSACT_ON_KEEP_ALIVE_STATE_CHANGED);
            FuzzUtil::WriteMaliciousProcessData(parcel, fdp);
            break;
        case 19:
            actualCode = static_cast<uint32_t>(
                IApplicationStateObserver::Message::TRANSACT_ON_PRELOAD_PROCESS_STATE_CHANGED);
            parcel.WriteBool(fdp.ConsumeBool());
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            break;
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(ApplicationStateObserverStubFuzz, FuzzUtil::Tokens::APP_STATE_OBSERVER)
} // namespace OHOS
