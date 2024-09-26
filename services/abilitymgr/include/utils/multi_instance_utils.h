/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_MULTI_INSTANCE_UTILS_H
#define OHOS_ABILITY_RUNTIME_MULTI_INSTANCE_UTILS_H

#include <string>

#include "ability_record.h"
#include "application_info.h"
#include "extension_ability_info.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
class MultiInstanceUtils {
public:
    static std::string GetInstanceKey(const Want& want);
    static std::string GetValidExtensionInstanceKey(const AbilityRequest &abilityRequest);
    static std::string GetSelfCallerInstanceKey(const AbilityRequest &abilityRequest);
    static bool IsDefaultInstanceKey(const std::string& key);
    static bool IsMultiInstanceApp(AppExecFwk::ApplicationInfo appInfo);
    static bool IsSupportedExtensionType(AppExecFwk::ExtensionAbilityType type);
    static bool IsInstanceKeyExist(const std::string& bundleName, const std::string& key);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_MULTI_INSTANCE_UTILS_H
