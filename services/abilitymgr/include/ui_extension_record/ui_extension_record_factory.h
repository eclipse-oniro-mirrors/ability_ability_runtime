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

#ifndef OHOS_ABILITY_RUNTIME_UI_EXTENSION_RECORD_FACTORY_H
#define OHOS_ABILITY_RUNTIME_UI_EXTENSION_RECORD_FACTORY_H

#include "extension_record_factory.h"
#include "singleton.h"

namespace OHOS {
namespace AbilityRuntime {
class UIExtensionRecordFactory : public ExtensionRecordFactory {
DECLARE_DELAYED_SINGLETON(UIExtensionRecordFactory)

public:
    bool NeedReuse(const AAFwk::AbilityRequest &abilityRequest, int32_t &extensionRecordId) override;

    int32_t PreCheck(const AAFwk::AbilityRequest &abilityRequest, const std::string &hostBundleName) override;

    int32_t CreateRecord(
        const AAFwk::AbilityRequest &abilityRequest, std::shared_ptr<ExtensionRecord> &extensionRecord) override;
private:
    void CreateDebugRecord(
        const AAFwk::AbilityRequest &abilityRequest, std::shared_ptr<AAFwk::BaseExtensionRecord> abilityRecord);
    int32_t EmbeddedUIPreCheck(const AAFwk::AbilityRequest &abilityRequest, const std::string &hostBundleName);
    bool CheckAllowCrossUserEmbeddedUI(const AAFwk::AbilityRequest &abilityRequest, const std::string &hostBundleName);
    int32_t CheckHostBundleName(const AAFwk::AbilityRequest &abilityRequest, const std::string &hostBundleName);
    bool IsInAllowList(const std::vector<AppExecFwk::ExtensionAbilityInfo> &extensionInfos,
        const std::string &extensionName, const std::string &callerAppIdentifier);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_UI_EXTENSION_RECORD_FACTORY_H
