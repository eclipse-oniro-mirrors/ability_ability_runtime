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

#include "accesstoken_kit.h"
#include "mock_my_flag.h"

namespace OHOS {
namespace AgentRuntime {
// ForCli test seams (issue-16055): defaults mirror the happy path (CLI tool token, valid HAP).
bool MyFlag::retIsCliToolToken = true;
int32_t MyFlag::retGetHapTokenInfo = 0;
int32_t MyFlag::hapTokenInfoUid = 0;
// ForCli anti-spoof seams (issue-16055 fix): default overrideCallingUid=false keeps GetCallingUid()
// on the real getuid() for all non-ForCli tests; cliToolUid/identityUid default to the calling uid.
bool MyFlag::overrideCallingUid = false;
bool MyFlag::setIdentityActive = false;
bool MyFlag::retSetCallingIdentity = true;
int32_t MyFlag::cliToolUid = 0;
int32_t MyFlag::identityUid = 0;
}  // namespace AgentRuntime

namespace Security {
namespace AccessToken {

// Overrides for the two AccessTokenKit entry points the ForCli path (ConnectAgentExtensionAbilityForCli
// / DisconnectAgentExtensionAbilityForCli) depends on. Linked ahead of access_token:libaccesstoken_sdk
// (shared), so these definitions take precedence; the SDK still provides AccessTokenID / HapTokenInfo
// types and any non-overridden entry points.
bool AccessTokenKit::IsCliToolToken(FullTokenID tokenID)
{
    return OHOS::AgentRuntime::MyFlag::retIsCliToolToken;
}

int AccessTokenKit::GetHapTokenInfo(AccessTokenID tokenID, HapTokenInfo &hapInfo)
{
    hapInfo.uid = OHOS::AgentRuntime::MyFlag::hapTokenInfoUid;
    return OHOS::AgentRuntime::MyFlag::retGetHapTokenInfo;
}

}  // namespace AccessToken
}  // namespace Security
}  // namespace OHOS
