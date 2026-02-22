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
#include "ui_extension_wrapper.h"

namespace OHOS {
namespace AAFwk {
namespace UIExtensionWrapper {

bool IsUIExtension(const AppExecFwk::ExtensionAbilityType type)
{
    return OHOS::AAFwk::UIExtensionUtils::IsUIExtension(type);
}

bool IsSystemUIExtension(const AppExecFwk::ExtensionAbilityType type)
{
    return OHOS::AAFwk::UIExtensionUtils::IsSystemUIExtension(type);
}

// In this case, extension which be starting needs that caller should be the system app, otherwise not supported.
bool IsSystemCallerNeeded(const AppExecFwk::ExtensionAbilityType type)
{
    return OHOS::AAFwk::UIExtensionUtils::IsSystemCallerNeeded(type);
}

// In this collection, extension can be embedded by public app, which requires vertical businesses to ensure security.
bool IsPublicForEmbedded(const AppExecFwk::ExtensionAbilityType type)
{
    return OHOS::AAFwk::UIExtensionUtils::IsPublicForEmbedded(type);
}

// In this collection, extension can be embedded by public app, which some UX effects are constrained
bool IsPublicForConstrainedEmbedded(const AppExecFwk::ExtensionAbilityType type)
{
    return OHOS::AAFwk::UIExtensionUtils::IsPublicForConstrainedEmbedded(type);
}

bool IsEnterpriseAdmin(const AppExecFwk::ExtensionAbilityType type)
{
    return OHOS::AAFwk::UIExtensionUtils::IsEnterpriseAdmin(type);
}

bool IsWindowExtension(const AppExecFwk::ExtensionAbilityType type)
{
    return OHOS::AAFwk::UIExtensionUtils::IsWindowExtension(type);
}

bool IsProcessUdkeyExtension(const AppExecFwk::ExtensionAbilityType type)
{
    return OHOS::AAFwk::UIExtensionUtils::IsProcessUdkeyExtension(type);
}

bool IsAgentUIExtension(const AppExecFwk::ExtensionAbilityType type)
{
    return OHOS::AAFwk::UIExtensionUtils::IsAgentUIExtension(type);
}

std::unordered_set<AppExecFwk::ExtensionAbilityType> GetUiExtensionSet()
{
    return OHOS::AAFwk::UIExtensionUtils::GetUiExtensionSet();
}
} // namespace UIExtensionWrapper
} // namespace AAFwk
} // namespace OHOS
