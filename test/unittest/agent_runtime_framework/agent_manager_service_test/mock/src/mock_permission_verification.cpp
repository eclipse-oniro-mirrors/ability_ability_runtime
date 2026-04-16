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

#include "mock_my_flag.h"
#include "permission_constants.h"
#include "permission_verification.h"

namespace OHOS {
namespace AgentRuntime {
bool MyFlag::retVerifyCallingPermission = true;
bool MyFlag::retVerifyConnectAgentPermission = true;
bool MyFlag::retVerifyGetAgentCardPermission = true;
bool MyFlag::retJudgeCallerIsAllowedToUseSystemAPI = true;
bool MyFlag::retVerifyModifyAgentCardPermission = true;
}

namespace AAFwk {
bool PermissionVerification::VerifyCallingPermission(const std::string &permissionName,
    const uint32_t specifyTokenId) const
{
    if (permissionName == PermissionConstants::PERMISSION_CONNECT_AGENT) {
        return AgentRuntime::MyFlag::retVerifyConnectAgentPermission;
    }
    if (permissionName == PermissionConstants::PERMISSION_GET_AGENT_CARD) {
        return AgentRuntime::MyFlag::retVerifyGetAgentCardPermission;
    }
    if (permissionName == PermissionConstants::PERMISSION_MODIFY_AGENT_CARD) {
        return AgentRuntime::MyFlag::retVerifyModifyAgentCardPermission;
    }
    return AgentRuntime::MyFlag::retVerifyCallingPermission;
}

bool PermissionVerification::JudgeCallerIsAllowedToUseSystemAPI() const
{
    return AgentRuntime::MyFlag::retJudgeCallerIsAllowedToUseSystemAPI;
}
}  // namespace AAFwk
}  // namespace OHOS
