/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_MAIN_ELEMENT_UTILS_H
#define OHOS_ABILITY_RUNTIME_MAIN_ELEMENT_UTILS_H

#include "bundle_info.h"
#include "param.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class MainElementUtils
 * provides main element utilities.
 */
class MainElementUtils final {
public:
    /**
     * CheckMainElement, check main element.
     *
     * @param hapModuleInfo The hap module info.
     * @param processName The process name.
     * @param mainElement The returned main element.
     * @param isDataAbility The returned flag indicates whether the module contains data ability.
     * @param uri Returned URI of the data ability.
     * @param userId User id.
     * @return Whether or not the hap module has the main element.
     */
    static bool CheckMainElement(const AppExecFwk::HapModuleInfo &hapModuleInfo,
        const std::string &processName, std::string &mainElement, bool &isDataAbility,
        std::string &uriStr, int32_t userId = 0);
    
    /**
     * UpdateMainElement, update main element.
     *
     * @param bundleName The bundle name.
     * @param moduleName The modle name.
     * @param mainElement The returned main element.
     * @param updateEnable Flag indicated whether update is enabled.
     * @param userId User id.
     */
    static void UpdateMainElement(const std::string &bundleName, const std::string &moduleName,
        const std::string &mainElement, bool updateEnable, int32_t userId);

    /**
     * IsMainUIAbility, verify whether or not the ability is main UIAbility.
     *
     * @param bundleName The bundle name.
     * @param abilityName The ability name.
     * @param userId User id.
     * @return Whether or not the ability is main UIAbility.
     */
    static bool IsMainUIAbility(const std::string &bundleName, const std::string &abilityName, int32_t userId);

    static void SetMainUIAbilityKeepAliveFlag(bool isMainUIAbility,
        const std::string &bundleName, AbilityRuntime::LoadParam &loadParam);

    /**
     * CheckMainUIAbility, check if bundle has main UIAbility.
     *
     * @param bundleInfo The bundle info.
     * @param mainElementName The returned main element name.
     * @return Whether or not the bundle has the main element.
     */
    static bool CheckMainUIAbility(const AppExecFwk::BundleInfo &bundleInfo, std::string& mainElementName);

    /**
     * CheckStatusBarAbility, check if bundle has status bar ability.
     *
     * @param bundleInfo The bundle info.
     * @return Whether or not the bundle has a status bar ability.
     */
    static bool CheckStatusBarAbility(const AppExecFwk::BundleInfo &bundleInfo);

    /**
     * CheckAppServiceExtension, check if bundle has app service extension.
     *
     * @param bundleInfo The bundle info.
     * @return Whether or not the bundle has app service extension.
     */
    static bool CheckAppServiceExtension(const AppExecFwk::BundleInfo &bundleInfo, std::string& mainElementName);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_MAIN_ELEMENT_UTILS_H
