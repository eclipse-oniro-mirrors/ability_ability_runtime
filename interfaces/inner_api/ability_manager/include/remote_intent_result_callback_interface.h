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

#ifndef OHOS_ABILITY_RUNTIME_REMOTE_INTENT_RESULT_CALLBACK_INTERFACE_H
#define OHOS_ABILITY_RUNTIME_REMOTE_INTENT_RESULT_CALLBACK_INTERFACE_H

#include <string>
#include "iremote_broker.h"

namespace OHOS {
namespace AAFwk {

class IRemoteIntentResultCallback : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.distributedschedule.IRemoteIntentResultCallback");
    enum {
        ON_INTENT_RESULT = 1,
        ON_LINK_DISCONNECTED = 2,
    };

    virtual void OnIntentResult(uint64_t requestCode, int32_t resultCode, const std::string& resultMsg) = 0;
    virtual void OnLinkDisconnected(uint64_t requestCode, int32_t reason) = 0;
};

} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_REMOTE_INTENT_RESULT_CALLBACK_INTERFACE_H
