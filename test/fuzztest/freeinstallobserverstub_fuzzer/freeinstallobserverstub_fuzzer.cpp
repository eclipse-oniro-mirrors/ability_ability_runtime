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
#include "freeinstallobserverstub_fuzzer.h"
#include "fuzz_util.h"
#include "free_install_observer_stub.h"
#include "free_install_observer_interface.h"

using namespace OHOS::AbilityRuntime;
using namespace OHOS;

namespace OHOS {
class FreeInstallObserverStubFuzz : public FreeInstallObserverStub {
public:
    void OnInstallFinished(const std::string &bundleName, const std::string &abilityName,
        const std::string &startTime, int32_t resultCode) override {}
    void OnInstallFinishedByUrl(const std::string &startTime, const std::string &url,
        int32_t resultCode) override {}
};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 2) {
        case 0:
            actualCode = static_cast<uint32_t>(IFreeInstallObserver::ON_INSTALL_FINISHED);
            parcel.WriteString(OHOS::FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteString(OHOS::FuzzUtil::BuildSpecialCharString(fdp));
            parcel.WriteString(OHOS::FuzzUtil::GenStrcpyOverflowString(fdp, 256));
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        case 1:
            actualCode = static_cast<uint32_t>(IFreeInstallObserver::ON_INSTALL_FINISHED_BY_URL);
            parcel.WriteString(OHOS::FuzzUtil::GenStrcpyOverflowString(fdp, 256));
            parcel.WriteString(OHOS::FuzzUtil::BuildSandboxEscapePath(fdp));
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(FreeInstallObserverStubFuzz, FuzzUtil::Tokens::FREE_INSTALL_OBSERVER)
} // namespace OHOS
