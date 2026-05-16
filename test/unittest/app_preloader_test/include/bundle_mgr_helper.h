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

#ifndef OHOS_ABILITY_RUNTIME_BUNDLE_MGR_HELPER_H
#define OHOS_ABILITY_RUNTIME_BUNDLE_MGR_HELPER_H

#include "bundle_mgr_interface.h"
#include "want.h"
#include "app_mgr_service_dump_error_code.h"

namespace OHOS {
namespace AppExecFwk {
    enum UidCheckCode {
        UID_CHECK_INVALID_NUM = -2,
        UID_CHECK_FALSE = -3,
        UID_CHECK_PRELOAD_SERVICE = -5,
        UID_CHECK_PRELOAD_PAGE = -6,
        UID_CHECK_PRELOAD_CONDITIONS = -7,
        UID_CHECK_BUNDLE_INFO = -9,
        UID_CHECK_BUNDLE_INFO_FAILED = -10,
        UID_CHECK_NOT_ENTRY_HAP_MODULE_INFO = -11,
    };

using Want = OHOS::AAFwk::Want;

class BundleMgrHelper : public std::enable_shared_from_this<BundleMgrHelper> {
public:
    BundleMgrHelper() = default;;
    ~BundleMgrHelper() = default;

    ErrCode GetLaunchWantForBundle(const std::string &bundleName, Want &want, int32_t userId)
    {
        if (userId == UID_CHECK_INVALID_NUM) {
            return ERR_INVALID_NUM_ARGS_ERROR;
        }
            
        return ERR_OK;
    }

    bool QueryAbilityInfo(const Want &want, int32_t flags, int32_t userId, AbilityInfo &abilityInfo)
    {
        switch (userId) {
            case UID_CHECK_FALSE:
                return false;
                break;
            case UID_CHECK_PRELOAD_SERVICE:
                abilityInfo.type = AppExecFwk::AbilityType::SERVICE;
                abilityInfo.isStageBasedModel = true;
                abilityInfo.name = "MainAbility";
                abilityInfo.applicationName = "com.acts.preloadtest";
                abilityInfo.applicationInfo.name = abilityInfo.applicationName;
                break;
            case UID_CHECK_PRELOAD_PAGE:
                abilityInfo.type = AppExecFwk::AbilityType::PAGE;
                abilityInfo.isStageBasedModel = true;
                abilityInfo.name = "";
                abilityInfo.applicationName = "com.acts.preloadtest";
                abilityInfo.applicationInfo.name = abilityInfo.applicationName;
                break;
            case UID_CHECK_PRELOAD_CONDITIONS:
                abilityInfo.type = AppExecFwk::AbilityType::PAGE;
                abilityInfo.isStageBasedModel = true;
                abilityInfo.name = "MainAbility";
                abilityInfo.applicationName = "com.acts.preloadtest";
                abilityInfo.applicationInfo.name = "com.acts.preloadtest2";
                break;
            default:
                abilityInfo.type = AppExecFwk::AbilityType::PAGE;
                abilityInfo.isStageBasedModel = true;
                abilityInfo.name = "MainAbility";
                abilityInfo.applicationName = "com.acts.preloadtest";
                abilityInfo.applicationInfo.name = abilityInfo.applicationName;
        }
  
        return true;
    }

    bool GetBundleInfo(const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo, int32_t userId)
    {
        if (userId == UID_CHECK_BUNDLE_INFO) {
            return false;
        }
        
        return true;
    }

    bool GetHapModuleInfo(const AbilityInfo &abilityInfo, int32_t userId, HapModuleInfo &hapModuleInfo)
    {
        switch (userId) {
            case UID_CHECK_BUNDLE_INFO_FAILED:
                hapModuleInfo.moduleType = AppExecFwk::ModuleType::ENTRY;
                return false;
            case UID_CHECK_NOT_ENTRY_HAP_MODULE_INFO:
                hapModuleInfo.moduleType = AppExecFwk::ModuleType::FEATURE;
                break;
            default:
                hapModuleInfo.moduleType = AppExecFwk::ModuleType::ENTRY;
                break;
        }
        return true;
    }

    bool QueryExtensionAbilityInfos(const Want &want, const int32_t &flag, const int32_t &userId,
        std::vector<ExtensionAbilityInfo> &extensionInfos)
    {
        ExtensionAbilityInfo extensionInfo;
        extensionInfos.emplace_back(extensionInfo);
        return true;
    }

    ErrCode GetSandboxExtAbilityInfos(const Want &want, int32_t appIndex, int32_t flags,
        int32_t userId, std::vector<ExtensionAbilityInfo> &extensionInfos)
    {
        ExtensionAbilityInfo extensionInfo;
        extensionInfos.emplace_back(extensionInfo);
        return ERR_OK;
    }
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_BUNDLE_MGR_HELPER_H