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
#include "autostartupcallbackstub_fuzzer.h"
#include "fuzz_util.h"
#include "auto_startup_callback_stub.h"
#include "auto_startup_interface.h"
#include "ability_manager_ipc_interface_code.h"

using namespace OHOS::AbilityRuntime;
using namespace OHOS::AAFwk;
using namespace OHOS;

namespace OHOS {
class AutoStartupCallBackStubFuzz : public AutoStartupCallBackStub {
public:
    void OnAutoStartupOn(const AutoStartupInfo &info) override {}
    void OnAutoStartupOff(const AutoStartupInfo &info) override {}
};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 2) {
        case 0:
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::ON_AUTO_STARTUP_ON);
            {
                AutoStartupInfo info;
                parcel.WriteParcelable(&info);
            }
            break;
        case 1:
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::ON_AUTO_STARTUP_OFF);
            {
                AutoStartupInfo info;
                parcel.WriteParcelable(&info);
            }
            break;
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(AutoStartupCallBackStubFuzz, FuzzUtil::Tokens::AUTO_STARTUP_CB)
} // namespace OHOS
