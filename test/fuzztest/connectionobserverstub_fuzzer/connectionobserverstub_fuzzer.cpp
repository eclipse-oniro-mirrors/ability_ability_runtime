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
#include "connectionobserverstub_fuzzer.h"
#include "fuzz_util.h"
#include "parcelable_constructors.h"
#include "connection_observer_stub.h"
#include "iconnection_observer.h"

using namespace OHOS::AbilityRuntime;
using namespace OHOS;

namespace OHOS {
class ConnectionObserverStubFuzz : public ConnectionObserverStub {
public:
    void OnExtensionConnected(const ConnectionData &data) override {}
    void OnExtensionDisconnected(const ConnectionData &data) override {}
    void OnExtensionSuspended(const ConnectionData &data) override {}
    void OnExtensionResumed(const ConnectionData &data) override {}
#ifdef WITH_DLP
    void OnDlpAbilityOpened(const DlpStateData &data) override {}
    void OnDlpAbilityClosed(const DlpStateData &data) override {}
#endif
};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 4) {
        case 0:
            actualCode = static_cast<uint32_t>(IConnectionObserver::ON_EXTENSION_CONNECTED);
            FuzzUtil::WriteMaliciousConnectionData(parcel, fdp);
            break;
        case 1:
            actualCode = static_cast<uint32_t>(IConnectionObserver::ON_EXTENSION_DISCONNECTED);
            FuzzUtil::WriteMaliciousConnectionData(parcel, fdp);
            break;
        case 2:
            actualCode = static_cast<uint32_t>(IConnectionObserver::ON_EXTENSION_SUSPENDED);
            FuzzUtil::WriteMaliciousConnectionData(parcel, fdp);
            break;
        case 3:
            actualCode = static_cast<uint32_t>(IConnectionObserver::ON_EXTENSION_RESUMED);
            FuzzUtil::WriteMaliciousConnectionData(parcel, fdp);
            break;
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(ConnectionObserverStubFuzz, FuzzUtil::Tokens::CONNECTION_OBSERVER)
} // namespace OHOS
