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

#ifndef MOCK_AGENT_RUNTIME_MY_FLAG_H
#define MOCK_AGENT_RUNTIME_MY_FLAG_H

#include "iremote_object.h"

namespace OHOS {
namespace AgentRuntime {
class MyFlag {
public:
    static bool retAddSystemAbilityListener;
    static sptr<IRemoteObject> systemAbility;
    static bool retPublish;
    static bool retRegisterBundleEventCallback;
    static bool retGetApplicationInfo;
    static bool isRegisterBundleEventCallbackCalled;
    static bool isAddSystemAbilityListenerCalled;
    static bool retVerifyCallingPermission;
    static int32_t retConnectAbilityWithExtensionType;
    static int32_t retDisconnectAbility;
    static int32_t retGetAllAgentCards;
    static int32_t retGetAgentCardsByBundleName;
    static int32_t retGetAgentCardByAgentId;
};
}  // namespace AgentRuntime
}  // namespace OHOS
#endif // MOCK_AGENT_RUNTIME_MY_FLAG_H