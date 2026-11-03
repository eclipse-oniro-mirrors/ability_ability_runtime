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

#include "statusbardelegatestub_fuzzer.h"
#include "attack_vectors.h"
#include "fuzz_util.h"

#include <cstddef>
#include <cstdint>

#include "message_parcel.h"
#include "securec.h"
#include "status_bar_delegate_stub.h"

using namespace OHOS::AbilityRuntime;
using namespace OHOS;

namespace OHOS {
namespace {
const std::u16string INTERFACE_TOKEN = u"ohos.ability.StatusBarDelegate";
}

class StatusBarDelegateStubFuzzTest : public StatusBarDelegateStub {
public:
    StatusBarDelegateStubFuzzTest() = default;
    virtual ~StatusBarDelegateStubFuzzTest()
    {}
    int32_t CheckIfStatusBarItemExists(uint32_t accessTokenId, const std::string &instanceKey, bool& isExist) override
    {
        return 0;
    }
    int32_t AttachPidToStatusBarItem(uint32_t accessTokenId, int32_t pid, const std::string &instanceKey) override
    {
        return 0;
    }
    int32_t DetachPidToStatusBarItem(uint32_t accessTokenId, int32_t pid, const std::string &instanceKey) override
    {
        return 0;
    }
};

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    FUZZ_EXTRACT_CODE(data, size);
    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;
    uint32_t actualCode =
        static_cast<uint32_t>(IStatusBarDelegate::StatusBarDelegateCmd::CHECK_IF_STATUS_BAR_ITEM_EXISTS);

    parcel.WriteInterfaceToken(INTERFACE_TOKEN);

    switch (code % 3) {
        case 0:
            actualCode =
                static_cast<uint32_t>(IStatusBarDelegate::StatusBarDelegateCmd::CHECK_IF_STATUS_BAR_ITEM_EXISTS);
            parcel.WriteUint32(static_cast<uint32_t>(OHOS::FuzzUtil::BuildIntegerOverflow(fdp)));
            parcel.WriteString(OHOS::FuzzUtil::BuildSpecialCharString(fdp));
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            break;
        case 1:
            actualCode = static_cast<uint32_t>(IStatusBarDelegate::StatusBarDelegateCmd::ATTACH_PID_TO_STATUS_BAR_ITEM);
            parcel.WriteUint32(static_cast<uint32_t>(OHOS::FuzzUtil::BuildIntegerOverflow(fdp)));
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteString(OHOS::FuzzUtil::BuildSandboxEscapePath(fdp));
            break;
        case 2:
            actualCode = static_cast<uint32_t>(IStatusBarDelegate::StatusBarDelegateCmd::DETACH_PID_TO_STATUS_BAR_ITEM);
            parcel.WriteUint32(static_cast<uint32_t>(OHOS::FuzzUtil::BuildIntegerOverflow(fdp)));
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteString(OHOS::FuzzUtil::BuildSandboxEscapePath(fdp));
            break;
        default:
            break;
    }

    parcel.RewindRead(0);
    std::shared_ptr<StatusBarDelegateStub> stub = std::make_shared<StatusBarDelegateStubFuzzTest>();
    stub->OnRemoteRequest(actualCode, parcel, reply, option);
    return true;
}
}

FUZZ_ENTRY_IMPL(OHOS::DoSomethingInterestingWithMyAPI)
