/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_PERMISSION_VERIFICATION_H
#define OHOS_ABILITY_RUNTIME_PERMISSION_VERIFICATION_H

#include "singleton.h"

namespace OHOS {
namespace AAFwk {
class PermissionVerification : public DelayedSingleton<PermissionVerification> {
public:
    PermissionVerification() = default;
    ~PermissionVerification() = default;

    enum FLAG {
        IS_SA_CALL = 1,
    };

    static bool hasBlockAllAppStartPermission;
    static bool hasStartUIAbilityToHiddenPermission;
    static int flag;

    bool IsSACall();

    bool VerifyBlockAllAppStartPermission();

    bool VerifyStartUIAbilityToHiddenPermission();

private:
    DISALLOW_COPY_AND_MOVE(PermissionVerification);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_PERMISSION_VERIFICATION_H
