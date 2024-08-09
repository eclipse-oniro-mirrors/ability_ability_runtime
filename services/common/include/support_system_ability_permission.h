/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_SUPPORT_SYSTEM_ABILITY_PERMISSION_H
#define OHOS_ABILITY_SUPPORT_SYSTEM_ABILITY_PERMISSION_H

#include <string>
#include "hilog_tag_wrapper.h"
#include "parameters.h"

namespace OHOS {
namespace AAFwk {
namespace SupportSystemAbilityPermission {
const std::string SUPPORT_SYSTEM_ABILITY_PERMISSION = "abilitymanagerservice.support.sa.call";

inline bool IsSupportSaCallPermission()
{
    TAG_LOGD(AAFwkTag::DEFAULT, "call");
    std::string ret = OHOS::system::GetParameter(SUPPORT_SYSTEM_ABILITY_PERMISSION, "true");
    return ret == "true";
}
}  // namespace SupportSystemAbilityPermission
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_SUPPORT_SYSTEM_ABILITY_PERMISSION_H