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

#include "permission_verification.h"
#include "insight_intent_utils.h"

namespace {
constexpr uint64_t ERRIGHT_INVALID_COMPONRNT = 2097269;
};
namespace OHOS {
namespace AAFwk {
using namespace AppExecFwk;
bool PermissionVerification::VerifyCallingPermission(
    const std::string &permissionName, const uint32_t specifyTokenId) const
{
    return true;
}
bool PermissionVerification::JudgeCallerIsAllowedToUseSystemAPI() const
{
    return true;
}
bool PermissionVerification::IsShellCall() const
{
    return false;
}
}

namespace AbilityRuntime {
uint32_t InsightIntentUtils::GetSrcEntry(const AppExecFwk::ElementName &elementName, const std::string &intentName,
    const AppExecFwk::ExecuteMode &executeMode, std::string &srcEntry, std::string *arkTSMode, int32_t userId,
    std::string *decoratorClass)
{
    if (executeMode == AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND) {
        if (intentName.empty()) {
            return ERRIGHT_INVALID_COMPONRNT;
        }
        srcEntry = "test.srcEntry";
        return ERR_OK;
    }
    return ERR_INVALID_VALUE;
}
}
} // namespace OHOS
