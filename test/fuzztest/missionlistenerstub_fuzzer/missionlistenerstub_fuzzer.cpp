/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "missionlistenerstub_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <iostream>

#include "mission_listener_stub.h"
#include "message_parcel.h"
#include "securec.h"

using namespace OHOS::AAFwk;

namespace OHOS {
namespace {
constexpr size_t U32_AT_SIZE = 4;
const std::u16string ABILITYMGR_INTERFACE_TOKEN = u"ohos.aafwk.MissionListener";
}

class MissionListenerStubFuzzTest : public MissionListenerStub {
public:
    MissionListenerStubFuzzTest() = default;
    virtual ~MissionListenerStubFuzzTest()
    {};
    void OnMissionCreated(int32_t missionId) override
    {}
    void OnMissionDestroyed(int32_t missionId) override
    {}
    void OnMissionSnapshotChanged(int32_t missionId) override
    {}
    void OnMissionMovedToFront(int32_t missionId) override
    {}
    void OnMissionMovedToBackground(int32_t missionId) override
    {}
    void OnMissionIconUpdated(int32_t missionId, const std::shared_ptr<OHOS::Media::PixelMap>& icon) override
    {}
    void OnMissionClosed(int32_t missionId) override
    {}
    void OnMissionLabelUpdated(int32_t missionId) override
    {}
};

uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];
}

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    uint32_t code = GetU32Data(data);

    MessageParcel parcel;
    parcel.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    parcel.WriteBuffer(data, size);
    parcel.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    std::shared_ptr<MissionListenerStub> missionlistenerstub = std::make_shared<MissionListenerStubFuzzTest>();

    missionlistenerstub->OnRemoteRequest(code, parcel, reply, option);

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

    char* ch = (char*)malloc(size + 1);
    if (ch == nullptr) {
        std::cout << "malloc failed." << std::endl;
        return 0;
    }

    (void)memset_s(ch, size + 1, 0x00, size + 1);
    if (memcpy_s(ch, size, data, size) != EOK) {
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