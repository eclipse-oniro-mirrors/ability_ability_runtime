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

#include "preloaduiextensionexecutecallbackstub_fuzzer.h"
#include "attack_vectors.h"
#include "fuzz_util.h"

#include <cstddef>
#include <cstdint>
#include <iostream>

#include "parcel.h"
#include "preload_ui_extension_execute_callback_stub.h"
#include "securec.h"

using namespace OHOS::AAFwk;
using namespace OHOS;

namespace OHOS {
namespace {
const std::u16string ABILITYMGR_INTERFACE_TOKEN = u"ohos.AAFwk.PreloadUIExtensionCallback";
}

class PreloadUIExtensionExecuteCallbackStubFuzz : public PreloadUIExtensionExecuteCallbackStub {
public:
    PreloadUIExtensionExecuteCallbackStubFuzz() = default;
    ~PreloadUIExtensionExecuteCallbackStubFuzz() = default;

    void OnLoadedDone(int32_t extensionAbilityId) override {}
    void OnDestroyDone(int32_t extensionAbilityId) override {}
    void OnPreloadSuccess(int32_t requestCode, int32_t extensionAbilityId, int32_t innerErrCode) override {}
};

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    FUZZ_EXTRACT_CODE(data, size);
    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;
    uint32_t actualCode = static_cast<uint32_t>(
        IPreloadUIExtensionExecuteCallback::ON_PRELOAD_UI_EXTENSION_ABILITY_LOADED_DONE);

    parcel.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);

    switch (code % 3) {
        case 0:
            actualCode = static_cast<uint32_t>(
                IPreloadUIExtensionExecuteCallback::ON_PRELOAD_UI_EXTENSION_ABILITY_LOADED_DONE);
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        case 1:
            actualCode = static_cast<uint32_t>(
                IPreloadUIExtensionExecuteCallback::ON_PRELOAD_UI_EXTENSION_ABILITY_DESTROY_DONE);
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        case 2:
            actualCode = static_cast<uint32_t>(
                IPreloadUIExtensionExecuteCallback::ON_PRELOAD_UI_EXTENSION_ABILITY_SUCCESS);
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        default:
            break;
    }

    parcel.RewindRead(0);
    std::shared_ptr<PreloadUIExtensionExecuteCallbackStub> stub =
        std::make_shared<PreloadUIExtensionExecuteCallbackStubFuzz>();
    stub->OnRemoteRequest(actualCode, parcel, reply, option);
    return true;
}
}

FUZZ_ENTRY_IMPL(OHOS::DoSomethingInterestingWithMyAPI)
