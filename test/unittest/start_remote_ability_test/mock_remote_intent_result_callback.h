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

#ifndef OHOS_ABILITY_RUNTIME_MOCK_REMOTE_INTENT_RESULT_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_MOCK_REMOTE_INTENT_RESULT_CALLBACK_H

#include <string>
#include "iremote_stub.h"
#include "remote_intent_result_callback_interface.h"

namespace OHOS {
namespace AAFwk {
class RemoteIntentResultCallback : public IRemoteStub<IRemoteIntentResultCallback> {
public:
    RemoteIntentResultCallback() = default;
    virtual ~RemoteIntentResultCallback() = default;

    void OnIntentResult(int32_t requestCode, int32_t resultCode, const std::string& resultMsg) override;
    void OnLinkDisconnected(int32_t requestCode, int32_t reason) override;

    static int onIntentResultCount;
    static int onLinkDisconnectedCount;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif
