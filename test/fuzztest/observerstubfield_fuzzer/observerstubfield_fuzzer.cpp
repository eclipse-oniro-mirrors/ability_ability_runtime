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
#include "attack_vectors.h"
#include "fuzz_util.h"
#include "appmgr/application_state_observer_stub.h"
#include "observerstubfield_fuzzer.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {
class ApplicationStateObserverStubFieldFuzz : public ApplicationStateObserverStub {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 4) {
        case 0:
            actualCode = static_cast<uint32_t>(
                IApplicationStateObserver::Message::TRANSACT_ON_FOREGROUND_APPLICATION_CHANGED);
            parcel.WriteBool(true);
            FuzzUtil::WriteMaliciousAppStateData(parcel, fdp);
            break;
        case 1:
            actualCode = static_cast<uint32_t>(
                IApplicationStateObserver::Message::TRANSACT_ON_ABILITY_STATE_CHANGED);
            parcel.WriteBool(true);
            FuzzUtil::WriteMaliciousAbilityStateData(parcel, fdp);
            break;
        case 2:
            actualCode = static_cast<uint32_t>(
                IApplicationStateObserver::Message::TRANSACT_ON_PROCESS_CREATED);
            parcel.WriteBool(true);
            FuzzUtil::WriteMaliciousProcessData(parcel, fdp);
            break;
        case 3:
            actualCode = static_cast<uint32_t>(
                IApplicationStateObserver::Message::TRANSACT_ON_PAGE_SHOW);
            parcel.WriteBool(true);
            FuzzUtil::WriteMaliciousPageStateData(parcel, fdp);
            break;
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(ApplicationStateObserverStubFieldFuzz, FuzzUtil::Tokens::APP_STATE_OBSERVER)
} // namespace OHOS
