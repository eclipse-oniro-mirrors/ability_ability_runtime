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

#include "abilitydebugresponsestub_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>
#define private public
#define protected public
#include "ability_debug_response_stub.h"
#undef protected
#undef private

using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
    constexpr size_t U32_AT_SIZE = 4;
}

const std::u16string IA_APP_FOREGROUND_STATE_OBSERVER_TOKEN = u"ohos.appexecfwk.AbilityDebugResponse";

class AbilityDebugResponseStubFuzz : public AbilityDebugResponseStub {
public:
    explicit AbilityDebugResponseStubFuzz() {};
    virtual ~AbilityDebugResponseStubFuzz() {};

    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override{ return 0; };

    virtual void OnAbilitysDebugStarted(const std::vector<sptr<IRemoteObject>> &tokens) override{};
    virtual void OnAbilitysDebugStoped(const std::vector<sptr<IRemoteObject>> &tokens) override{};
    virtual void OnAbilitysAssertDebugChange(
        const std::vector<sptr<IRemoteObject>> &tokens, bool isAssertDebug) override{};
};

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    uint32_t code = fdp.ConsumeIntegralInRange<int32_t>(0, U32_AT_SIZE);
    MessageParcel parcel;
    parcel.WriteInterfaceToken(IA_APP_FOREGROUND_STATE_OBSERVER_TOKEN);
    parcel.WriteBuffer(data, size);
    parcel.RewindRead(0);
    MessageParcel reply;
    MessageOption option;
    std::shared_ptr<AbilityDebugResponseStub> stub = std::make_shared<AbilityDebugResponseStubFuzz>();
    stub->OnRemoteRequest(code, parcel, reply, option);
    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */

    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}