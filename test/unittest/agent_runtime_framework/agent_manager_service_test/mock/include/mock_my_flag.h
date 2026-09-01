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

#include <string>
#include <vector>
#include "ability_connect_callback_interface.h"
#include "agent_card.h"
#include "hap_module_info.h"
#include "want.h"
#include "iremote_object.h"
#include "running_process_info.h"
#include "extension_ability_info.h"

namespace OHOS {
namespace AgentRuntime {
class MyFlag {
public:
    static bool retAddSystemAbilityListener;
    static sptr<IRemoteObject> systemAbility;
    static bool retPublish;
    static bool retRegisterBundleEventCallback;
    static bool retGetApplicationInfo;
    static bool retGetBundleInfo;
    static bool retGetResConfigFile;
    static bool isRegisterBundleEventCallbackCalled;
    static bool isAddSystemAbilityListenerCalled;
    static int32_t backfillPreInstallCardsCallCount;
    static bool retVerifyCallingPermission;
    static bool retVerifyConnectAgentPermission;
    static bool retVerifyGetAgentCardPermission;
    static bool retJudgeCallerIsAllowedToUseSystemAPI;
    static bool retVerifyModifyAgentCardPermission;
    static bool retCheckSpecificSystemAbilityAccessPermission;
    static int32_t retRegisterAgentCard;
    static int32_t retUpdateAgentCard;
    static int32_t retDeleteAgentCard;
    static int32_t retConnectAbilityWithExtensionType;
    static int32_t retDisconnectAbility;
    static int32_t retGetAllAgentCards;
    static int32_t retGetAgentCardsByBundleName;
    static int32_t retGetAgentCardByAgentId;
    static std::vector<AgentCard> agentCardsByBundleName;
    static std::string agentCardAgentId;
    static std::string agentCardBundleName;
    static std::string agentCardModuleName;
    static std::string agentCardAbilityName;
    static bool shouldCreateAgentCardAppInfo;
    static int32_t agentCardType;
    static bool retQueryExtensionAbilityInfos;
    static bool shouldFillExtensionAbilityInfos;
    static AppExecFwk::ExtensionAbilityType extensionAbilityType;
    static bool mockApplicationInfoIsSystemApp;
    static std::vector<AppExecFwk::ExtensionAbilityInfo> mockExtensionInfos;
    static std::vector<AppExecFwk::HapModuleInfo> mockHapModuleInfos;
    static std::vector<std::string> mockProfileInfos;
    static int32_t extensionAbilityUid;
    static int32_t retGetProcessRunningInfoByPid;
    static AppExecFwk::AppProcessState processState;
    static int32_t retGetBundleNameByPid;
    static AAFwk::Want lastConnectAbilityWant;
    static sptr<AAFwk::IAbilityConnection> lastConnectAbilityConnection;
    static sptr<IRemoteObject> lastConnectAbilityCallerToken;
    static AppExecFwk::ExtensionAbilityType lastConnectAbilityExtensionType;
    static sptr<AAFwk::IAbilityConnection> lastDisconnectAbilityConnection;
    static int32_t connectAbilityWithExtensionTypeCallCount;
    static int32_t disconnectAbilityCallCount;
    // ForCli (CLI agent connect, issue-16055) test seams.
    static bool retGetBoolParameterCliEnabled;  // system::GetBoolParameter(CCM flag) -> this
    static bool retIsCliToolToken;              // AccessTokenKit::IsCliToolToken -> this
    static int32_t retGetHapTokenInfo;         // AccessTokenKit::GetHapTokenInfo return code (0 = ok)
    static int32_t hapTokenInfoUid;             // uid returned by the mocked GetHapTokenInfo
    // ForCli anti-spoof test seams (issue-16055 fix): IPCSkeleton::GetCallingUid() returns different
    // values before vs after SetCallingIdentity — cliToolUid (real process uid, captured before
    // SetCallingIdentity) vs identityUid (uid parsed from the callerIdentity string). When
    // overrideCallingUid is false GetCallingUid() falls back to getuid() (preserves prior behavior).
    static bool overrideCallingUid;             // when true, GetCallingUid() uses the seam values
    static bool setIdentityActive;              // toggled by SetCallingIdentity/ResetCallingIdentity
    static int32_t cliToolUid;                  // GetCallingUid() before SetCallingIdentity
    static int32_t identityUid;                 // GetCallingUid() after SetCallingIdentity
    static bool retSetCallingIdentity;          // IPCSkeleton::SetCallingIdentity -> this
};
}  // namespace AgentRuntime
}  // namespace OHOS
#endif // MOCK_AGENT_RUNTIME_MY_FLAG_H
