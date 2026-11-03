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
#include "imageerrorhandlerstub_fuzzer.h"
#include "image_error_handler_stub.h"
#include "image_error_handler_interface.h"

using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {
class ImageErrorHandlerStubFuzz : public ImageErrorHandlerStub {
public:
    void OnError(int32_t errorCode) override {}
};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 1) {
        case 0:
            actualCode = static_cast<uint32_t>(IImageErrorHandler::Message::ON_ERROR);
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(ImageErrorHandlerStubFuzz, FuzzUtil::Tokens::IMAGE_ERROR_HANDLER)
} // namespace OHOS
