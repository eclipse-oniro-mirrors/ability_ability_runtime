/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "abilitystartwithwaitobserverstub_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#define private public
#define protected public
#include "ability_start_with_wait_observer_stub.h"
#undef protected

#include "ability_record.h"
#include "securec.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t U32_AT_SIZE = 4;
constexpr size_t STRING_MAX_LENGTH = 128;
const std::u16string AA_START_WAIT_OBSERVER_TOKEN = u"ohos.ability.IAbilityStartWithWaitObserver";
}

class AbilityStartWithWaitObserverStubFUZZ : public AbilityStartWithWaitObserverStub {
public:
    explicit AbilityStartWithWaitObserverStubFUZZ() {};
    virtual ~AbilityStartWithWaitObserverStubFUZZ() {};
    int32_t NotifyAATerminateWait(const AbilityStartWithWaitObserverData &abilityStartWithWaitData) override
    {
        return 0;
    };
};

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    FuzzedDataProvider fdp(reinterpret_cast<const uint8_t*>(data), size);
    std::shared_ptr<AbilityStartWithWaitObserverStub> infos = std::make_shared<AbilityStartWithWaitObserverStubFUZZ>();
    if (infos == nullptr) {
        return false;
    }
    AbilityStartWithWaitObserverData observerData;
    observerData.coldStart = fdp.ConsumeBool();
    observerData.reason = fdp.ConsumeIntegral<uint32_t>();
    observerData.startTime = fdp.ConsumeIntegral<int64_t>();
    observerData.foregroundTime = fdp.ConsumeIntegral<int64_t>();
    observerData.bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    observerData.abilityName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);

    uint32_t code = static_cast<uint32_t>(IAbilityStartWithWaitObserver::Message::NOTIFY_AA_TERMINATE_WAIT);
    MessageParcel parcel;
    parcel.WriteInterfaceToken(AA_START_WAIT_OBSERVER_TOKEN);
    parcel.WriteParcelable(&observerData);
    parcel.RewindRead(0);
    MessageParcel reply;
    MessageOption option;
    infos->OnRemoteRequest(code, parcel, reply, option);

    MessageParcel parcel2;
    parcel2.WriteParcelable(&observerData);
    parcel2.RewindRead(0);
    infos->OnNotifyAATerminateWithWait(parcel2, reply);

    wptr<IRemoteObject> remote;
    AbilityStartWithWaitObserverRecipient::RemoteDiedHandler handler;
    auto abilityStartWithWaitObserverRecipient =
        std::make_shared<AbilityStartWithWaitObserverRecipient>(handler);
    abilityStartWithWaitObserverRecipient->OnRemoteDied(remote);
    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        std::cout << "invalid data" << std::endl;
        return 0;
    }

    /* Validate the length of size */
    if (size < OHOS::U32_AT_SIZE) {
        return 0;
    }

    char* ch = static_cast<char*>(malloc(size + 1));
    if (ch == nullptr) {
        std::cout << "malloc failed." << std::endl;
        return 0;
    }

    (void)memset_s(ch, size + 1, 0x00, size + 1);
    if (memcpy_s(ch, size + 1, data, size) != EOK) {
        std::cout << "copy failed." << std::endl;
        free(ch);
        ch = nullptr;
        return 0;
    }

    OHOS::DoSomethingInterestingWithMyAPI(ch, size);
    free(ch);
    ch = nullptr;
    return 0;
}
