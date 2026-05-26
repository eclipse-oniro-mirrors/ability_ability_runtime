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

#include "extension_record_factory.h"

#include "ability_util.h"
#include "app_utils.h"
#include "base_extension_record.h"
#include "multi_instance_utils.h"
#include "startup_util.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
namespace {
const std::map<AppExecFwk::ExtensionAbilityType, ExtensionRecordConfig> EXTENSION_RECORD_CONFIG_MAP = {
    { AppExecFwk::ExtensionAbilityType::EMBEDDED_UI,
      { PROCESS_MODE_BUNDLE, PROCESS_MODE_SUPPORT_DEFAULT | PROCESS_MODE_HOST_SPECIFIED |
        PROCESS_MODE_HOST_INSTANCE | PROCESS_MODE_CUSTOM | PROCESS_MODE_PLUGIN,
        PRE_CHECK_FLAG_CALLED_WITHIN_THE_BUNDLE | PRE_CHECK_FLAG_MULTIPLE_PROCESSES }},
    { AppExecFwk::ExtensionAbilityType::STATUS_BAR_VIEW,
      { PROCESS_MODE_BUNDLE, PROCESS_MODE_SUPPORT_DEFAULT | PROCESS_MODE_RUN_WITH_MAIN_PROCESS,
        PRE_CHECK_FLAG_NONE }},
    { AppExecFwk::ExtensionAbilityType::AWC_WEBPAGE,
      { PROCESS_MODE_BUNDLE, PROCESS_MODE_SUPPORT_DEFAULT | PROCESS_MODE_RUN_WITH_MAIN_PROCESS,
        PRE_CHECK_FLAG_NONE }},
    { AppExecFwk::ExtensionAbilityType::AWC_NEWSFEED,
      { PROCESS_MODE_BUNDLE, PROCESS_MODE_SUPPORT_DEFAULT | PROCESS_MODE_RUN_WITH_MAIN_PROCESS,
        PRE_CHECK_FLAG_NONE }},
    { AppExecFwk::ExtensionAbilityType::LIVE_FORM, { PROCESS_MODE_BUNDLE, PROCESS_MODE_BUNDLE, PRE_CHECK_FLAG_NONE }},
    { AppExecFwk::ExtensionAbilityType::SYS_COMMON_UI,
      { PROCESS_MODE_BUNDLE, PROCESS_MODE_SUPPORT_DEFAULT | PROCESS_MODE_RUN_WITH_MAIN_PROCESS,
        PRE_CHECK_FLAG_NONE }},
    { AppExecFwk::ExtensionAbilityType::AGENT_UI,
      { PROCESS_MODE_AGENT_UI, PROCESS_MODE_AGENT_UI, PRE_CHECK_FLAG_NONE }},
    { AppExecFwk::ExtensionAbilityType::SYSPICKER_MEDIACONTROL,
      { PROCESS_MODE_BUNDLE, PROCESS_MODE_SUPPORT_DEFAULT | PROCESS_MODE_CALLER_INSTANCE,
        PRE_CHECK_FLAG_NONE }},
};

uint32_t GetPreCheckFlag(ExtensionAbilityType type)
{
    auto iter = EXTENSION_RECORD_CONFIG_MAP.find(type);
    if (iter == EXTENSION_RECORD_CONFIG_MAP.end()) {
        return PRE_CHECK_FLAG_NONE;
    }
    return iter->second.preCheckFlag;
}
}

ExtensionRecordFactory::ExtensionRecordFactory() = default;

ExtensionRecordFactory::~ExtensionRecordFactory() = default;

bool ExtensionRecordFactory::NeedReuse(const AAFwk::AbilityRequest &abilityRequest, int32_t &extensionRecordId)
{
    return false;
}

int32_t ExtensionRecordFactory::PreCheck(const AAFwk::AbilityRequest &abilityRequest, const std::string &hostBundleName)
{
    uint32_t preCheckFlag = GetPreCheckFlag(abilityRequest.extensionType);
    if (preCheckFlag == 0) {
        return ERR_OK;
    }

    if (preCheckFlag & PRE_CHECK_FLAG_MULTIPLE_PROCESSES) {
        if (!AppUtils::GetInstance().IsMultiProcessModel()) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "not multi process model");
            return ERR_INVALID_VALUE;
        }
    }
    return ERR_OK;
}

uint32_t ExtensionRecordFactory::GetExtensionProcessMode(
    const AAFwk::AbilityRequest &abilityRequest, bool &isHostSpecified)
{
    ExtensionRecordConfig config;
    auto iter = EXTENSION_RECORD_CONFIG_MAP.find(abilityRequest.extensionType);
    if (iter != EXTENSION_RECORD_CONFIG_MAP.end()) {
        config = iter->second;
    }

    if (config.processModeSupport & PROCESS_MODE_PLUGIN) {
        if (AbilityRuntime::StartupUtil::IsStartPlugin(abilityRequest.want)) {
            return PROCESS_MODE_PLUGIN;
        }
    }

    // check host specified
    isHostSpecified = false;
    if (config.processModeSupport & PROCESS_MODE_HOST_SPECIFIED) {
        if (abilityRequest.want.HasParameter(PROCESS_MODE_HOST_SPECIFIED_KEY)) {
            isHostSpecified = true;
            return PROCESS_MODE_HOST_SPECIFIED;
        }
    }

    if (config.processModeSupport & PROCESS_MODE_HOST_INSTANCE) {
        if (abilityRequest.want.HasParameter(PROCESS_MODE_HOST_INSTANCE_KEY)) {
            bool hostInstance = abilityRequest.want.GetBoolParam(PROCESS_MODE_HOST_INSTANCE_KEY, false);
            if (hostInstance) {
                return PROCESS_MODE_INSTANCE;
            }
        }
    }

    if (config.processModeSupport & PROCESS_MODE_CUSTOM) {
        if (!abilityRequest.customProcess.empty()) {
            return PROCESS_MODE_CUSTOM;
        }
    }

    if (abilityRequest.extensionProcessMode == ExtensionProcessMode::UNDEFINED) {
        return config.processModeDefault;
    }
    uint32_t inputProcessMode = 1 << static_cast<int32_t>(abilityRequest.extensionProcessMode);
    if (config.processModeSupport & inputProcessMode) {
        return inputProcessMode;
    }
    return config.processModeDefault;
}

int32_t ExtensionRecordFactory::CreateRecord(
    const AAFwk::AbilityRequest &abilityRequest, std::shared_ptr<ExtensionRecord> &extensionRecord)
{
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityRecord create failed");
        return ERR_NULL_OBJECT;
    }
    if (AAFwk::MultiInstanceUtils::IsMultiInstanceApp(abilityRequest.appInfo)) {
        abilityRecord->SetInstanceKey(AAFwk::MultiInstanceUtils::GetValidExtensionInstanceKey(abilityRequest));
    }
    extensionRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    extensionRecord->processMode_ = GetExtensionProcessMode(abilityRequest, extensionRecord->isHostSpecified_);
    return ERR_OK;
}
} // namespace AbilityRuntime
} // namespace OHOS
