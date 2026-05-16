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

#ifndef OHOS_ABILITY_RUNTIME_REMOTE_INTENT_RESULT_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_REMOTE_INTENT_RESULT_CALLBACK_H

#include <iremote_object.h>
#include <iremote_stub.h>

#include "remote_intent_result_callback_stub.h"

namespace OHOS {
namespace AAFwk {
class RemoteIntentResultCallback : public RemoteIntentResultCallbackStub {
public:
    RemoteIntentResultCallback() = default;
    virtual ~RemoteIntentResultCallback() = default;

    void OnIntentResult(uint64_t requestCode, int32_t resultCode, const std::string& resultMsg) override;

    void OnLinkDisconnected(uint64_t requestCode, int32_t reason) override;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_REMOTE_INTENT_RESULT_CALLBACK_H
