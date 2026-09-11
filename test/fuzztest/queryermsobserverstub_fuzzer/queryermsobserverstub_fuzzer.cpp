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
#include "queryermsobserverstub_fuzzer.h"
#include "query_erms_observer_interface.h"
#include "query_erms_observer_stub.h"

using namespace OHOS::AbilityRuntime;
using namespace OHOS;

namespace OHOS {
class QueryErmsObserverStubFuzz : public QueryERMSObserverStub {
public:
    void OnQueryFinished(const std::string &appId, const std::string &startTime,
        const AtomicServiceStartupRule &rule, int resultCode) override {}
};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 1) {
        case 0:
            actualCode = static_cast<uint32_t>(IQueryERMSObserver::ON_QUERY_FINISHED);
            parcel.WriteString(OHOS::FuzzUtil::BuildSpecialCharString(fdp));
            parcel.WriteString(OHOS::FuzzUtil::GenStrcpyOverflowString(fdp, 256));
            parcel.WriteBool(fdp.ConsumeBool());
            parcel.WriteBool(fdp.ConsumeBool());
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(QueryErmsObserverStubFuzz, FuzzUtil::Tokens::QUERY_ERMS_OBSERVER)
} // namespace OHOS
