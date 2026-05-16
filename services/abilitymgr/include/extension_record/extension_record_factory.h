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

#ifndef OHOS_ABILITY_RUNTIME_EXTENSION_RECORD_FACTORY_H
#define OHOS_ABILITY_RUNTIME_EXTENSION_RECORD_FACTORY_H

#include "ability_record.h"
#include "extension_record.h"

namespace OHOS {
namespace AbilityRuntime {
constexpr const char *PROCESS_MODE_HOST_SPECIFIED_KEY = "ohos.extension.processMode.hostSpecified";
constexpr const char *PROCESS_MODE_HOST_INSTANCE_KEY = "ohos.extension.processMode.hostInstance";
constexpr uint32_t PROCESS_MODE_INSTANCE = 1 << static_cast<uint32_t>(AppExecFwk::ExtensionProcessMode::INSTANCE);
constexpr uint32_t PROCESS_MODE_TYPE = 1 << static_cast<uint32_t>(AppExecFwk::ExtensionProcessMode::TYPE);
constexpr uint32_t PROCESS_MODE_BUNDLE = 1 << static_cast<uint32_t>(AppExecFwk::ExtensionProcessMode::BUNDLE);
constexpr uint32_t PROCESS_MODE_RUN_WITH_MAIN_PROCESS =
    1 << static_cast<uint32_t>(AppExecFwk::ExtensionProcessMode::RUN_WITH_MAIN_PROCESS);
constexpr uint32_t PROCESS_MODE_CALLER_INSTANCE =
    1 << static_cast<uint32_t>(AppExecFwk::ExtensionProcessMode::CALLER_INSTANCE);
constexpr uint32_t PROCESS_INNER_MODE_OFFSET = 16;
constexpr uint32_t PROCESS_MODE_HOST_SPECIFIED = 1 << (PROCESS_INNER_MODE_OFFSET + 0);
constexpr uint32_t PROCESS_MODE_HOST_INSTANCE = 1 << (PROCESS_INNER_MODE_OFFSET + 1);
constexpr uint32_t PROCESS_MODE_CUSTOM = 1 << (PROCESS_INNER_MODE_OFFSET + 2);
constexpr uint32_t PROCESS_MODE_PLUGIN = 1 << (PROCESS_INNER_MODE_OFFSET + 3);
constexpr uint32_t PROCESS_MODE_AGENT_UI = 1 << (PROCESS_INNER_MODE_OFFSET + 4);
constexpr uint32_t PROCESS_MODE_SUPPORT_DEFAULT = PROCESS_MODE_BUNDLE | PROCESS_MODE_TYPE | PROCESS_MODE_INSTANCE;
constexpr uint32_t PRE_CHECK_FLAG_NONE = 0;
constexpr uint32_t PRE_CHECK_FLAG_CALLED_WITHIN_THE_BUNDLE = 1 << 0;
constexpr uint32_t PRE_CHECK_FLAG_MULTIPLE_PROCESSES = 1 << 1;
struct ExtensionRecordConfig {
    uint32_t processModeDefault = PROCESS_MODE_BUNDLE;
    uint32_t processModeSupport = PROCESS_MODE_SUPPORT_DEFAULT;
    uint32_t preCheckFlag = PRE_CHECK_FLAG_NONE;
};

class ExtensionRecordFactory : public std::enable_shared_from_this<ExtensionRecordFactory> {
public:
    ExtensionRecordFactory();

    virtual ~ExtensionRecordFactory();

    /**
     * @brief Check whether the existing extensionRecord needs to be reused.
     *
     * @param abilityRequest Indicates the request of the extension ability to start.
     * @param extensionRecordId Indicates the ID of the reused extension record.
     * @return bool Returns true if the extension record need to be reused.
     */
    virtual bool NeedReuse(const AAFwk::AbilityRequest &abilityRequest, int32_t &extensionRecordId);

    /**
     * @brief Check the request of the extension ability to start.
     *
     * @param abilityRequest Indicates the request of the extension ability to start.
     * @param hostBundleName Indicates the bundle name of the host.
     * @return int32_t Returns ERR_OK on success, others on failure.
     */
    virtual int32_t PreCheck(const AAFwk::AbilityRequest &abilityRequest, const std::string &hostBundleName);

    /**
     * @brief Create extension record based on the abilityRequest.
     *
     * @param abilityRequest Indicates the request of the extension ability to start.
     * @param extensionRecord Indicates the created extension record.
     * @return int32_t Returns ERR_OK on success, others on failure.
     */
    virtual int32_t CreateRecord(
        const AAFwk::AbilityRequest &abilityRequest, std::shared_ptr<ExtensionRecord> &extensionRecord);

protected:
    uint32_t GetExtensionProcessMode(const AAFwk::AbilityRequest &abilityRequest, bool &isHostSpecified);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_EXTENSION_RECORD_FACTORY_H
