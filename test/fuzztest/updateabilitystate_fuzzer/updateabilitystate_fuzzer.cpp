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

#include "updateabilitystate_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#include "ability_record.h"
#include "app_mgr_client.h"
#include "parcel.h"
#include "securec.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
sptr<Token> GetFuzzAbilityToken(FuzzedDataProvider *fdp)
{
    sptr<Token> token = nullptr;
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = static_cast<AbilityType>(fdp->ConsumeIntegral<int32_t>());
    abilityRequest.appInfo.bundleName = fdp->ConsumeRandomLengthString();
    abilityRequest.abilityInfo.name = abilityRequest.appInfo.bundleName;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (abilityRecord) {
        token = abilityRecord->GetToken();
    }
    return token;
}

bool DoSomethingInterestingWithMyAPI(FuzzedDataProvider *fdp)
{
    std::shared_ptr<AppMgrClient> appMgrClient = std::make_shared<AppMgrClient>();
    if (!appMgrClient) {
        return false;
    }

    sptr<IRemoteObject> token = GetFuzzAbilityToken(fdp);
    if (!token) {
        std::cout << "Get ability token failed." << std::endl;
        return false;
    }
    AppExecFwk::AbilityState state = AppExecFwk::AbilityState::ABILITY_STATE_CREATE;

    if (appMgrClient->UpdateAbilityState(token, state) != AppMgrResultCode::RESULT_OK) {
        return false;
    }
    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    FuzzedDataProvider fdp(data, size);
    OHOS::DoSomethingInterestingWithMyAPI(&fdp);
    return 0;
}
