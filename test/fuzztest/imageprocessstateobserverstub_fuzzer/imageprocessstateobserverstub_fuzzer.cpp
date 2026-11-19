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
#include "imageprocessstateobserverstub_fuzzer.h"
#include "image_process_state_observer_stub.h"
#include "image_process_state_observer_interface.h"

using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {
class ImageProcessStateObserverStubFuzz : public ImageProcessStateObserverStub {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 3) {
        case 0:
            actualCode = static_cast<uint32_t>(IImageProcessStateObserver::Message::ON_IMAGE_PROCESS_STATE_CHANGED);
            {
                ImageProcessStateData processData;
                parcel.WriteParcelable(&processData);
            }
            break;
        case 1:
            actualCode = static_cast<uint32_t>(IImageProcessStateObserver::Message::ON_FORKALL_WORK_PROCESS_FAILED);
            {
                ImageProcessStateData processData;
                parcel.WriteParcelable(&processData);
            }
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        case 2:
            actualCode = static_cast<uint32_t>(IImageProcessStateObserver::Message::ON_PRE_FORK_ALL_WORK_PROCESS);
            {
                ImageProcessStateData processData;
                parcel.WriteParcelable(&processData);
            }
            break;
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(ImageProcessStateObserverStubFuzz, FuzzUtil::Tokens::IMAGE_PROCESS_STATE_OBS)
} // namespace OHOS
