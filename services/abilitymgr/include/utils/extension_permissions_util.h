/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_EXTENSION_PERMISSIONS_UTIL_H
#define OHOS_EXTENSION_PERMISSIONS_UTIL_H

#include "extension_ability_info.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class ExtensionPermissionsUtil
 * provides extension permission utilities.
 */
class ExtensionPermissionsUtil final {
public:
    /**
     * CheckSAPermission, check if it has SA permissions.
     *
     * @param extensionType The extension type.
    * @param specifyTokenId The specified token ID.
     * @return Whether it has SA permissions.
     */
    static bool CheckSAPermission(const AppExecFwk::ExtensionAbilityType &extensionType, uint32_t specifyTokenId = 0);

    /**
     * CheckSAPermissionMore, check if it has SA permissions or more.
     *
     * @param extensionType The extension type.
     * @param specifyTokenId The specified token ID.
     * @return Whether it has SA permissions or more.
     */
    static bool CheckSAPermissionMore(const AppExecFwk::ExtensionAbilityType &extensionType,
        uint32_t specifyTokenId = 0);

    /**
     * CheckCallerPermission, check the caller permission required to start the extension of
     * the given type. Types without a dedicated rule are bypassed.
     *
     * @param extensionType The extension type.
     * @param specifyTokenId The specified token ID.
     * @return Whether the caller is allowed to start the extension.
     */
    static bool CheckCallerPermission(const AppExecFwk::ExtensionAbilityType &extensionType,
        uint32_t specifyTokenId = 0);
};

} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_EXTENSION_PERMISSIONS_UTIL_H