/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "mock_bundle_mgr_helper.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const int32_t TOKENID_LENGTH = 4;
constexpr int32_t DEFAULT_APPLICATION_UID = 100;
constexpr int32_t U1_USER_APPLICATION_UID = 1;
bool IsDigitString(std::string name)
{
    for (int i = 0; i < name.size(); i++) {
        if (!isdigit(name[i])) {
            return false;
        }
    }
    return true;
}

bool IsExtensionCase(const std::string& bundleName)
{
    return bundleName.find("extensionInfo") != std::string::npos ||
        bundleName.find("extensionInfos") != std::string::npos;
}

void ProcessExtension(
    BundleInfo& bundleInfo, const std::string& bundleName, const std::string& moduleName, int32_t accessTokenId)
{
    ExtensionAbilityInfo info;
    info.bundleName = bundleName;
    info.applicationInfo.accessTokenId = accessTokenId;
    info.name = "nameTest";
    info.moduleName = moduleName;
    if ("extensionInfoTypeAppService" == bundleName) {
        info.type = AppExecFwk::ExtensionAbilityType::APP_SERVICE;
        info.visible = true;
        info.applicationInfo.uid = DEFAULT_APPLICATION_UID;
    } else if ("extensionInfoTypeAppServiceU1" == bundleName) {
        info.type = AppExecFwk::ExtensionAbilityType::APP_SERVICE;
        info.visible = true;
        info.applicationInfo.uid = U1_USER_APPLICATION_UID;
    }
    bundleInfo.extensionInfos.emplace_back(info);
}
} // namespace

BundleMgrHelper::BundleMgrHelper() {}

BundleMgrHelper::~BundleMgrHelper() {}

void BundleMgrHelper::OnDeath() {}

ErrCode BundleMgrHelper::GetSandboxBundleInfo(
    const std::string& bundleName, int32_t appIndex, int32_t userId, BundleInfo& info)
{
    if (bundleName.size() < TOKENID_LENGTH) {
        return -1;
    }
    auto tokenIdStr = bundleName.substr(bundleName.size() - TOKENID_LENGTH, TOKENID_LENGTH);
    if (!IsDigitString(tokenIdStr)) {
        return -1;
    }
    uint32_t tokenId = std::stoi(tokenIdStr);
    info.applicationInfo.accessTokenId = tokenId;
    return ERR_OK;
}

bool BundleMgrHelper::GetBundleInfo(
    const std::string& bundleName, const BundleFlag flags, BundleInfo& bundleInfo, int32_t userId)
{
    if (!bundleName.empty()) {
        int32_t accessTokenId = 1;
        if ("infoListIs0" == bundleName) {
            accessTokenId++;
        }
        std::string moduleName = "moduleName";
        if (IsExtensionCase(bundleName)) {
            moduleName = "moduleNameTest";
            ProcessExtension(bundleInfo, bundleName, moduleName, accessTokenId);
        } else {
            AbilityInfo abilityInfo;
            bundleInfo.applicationInfo.accessTokenId = accessTokenId;
            abilityInfo.bundleName = "bundleNameTest";
            abilityInfo.name = "nameTest";
            abilityInfo.moduleName = moduleName;
            bundleInfo.abilityInfos.emplace_back(abilityInfo);
            if ("isFoundIsTrue" == bundleName) {
                moduleName = "moduleNameTest";
            } else if ("moduleNameIsempty" == bundleName) {
                moduleName = "";
            }
            if ("hapModuleInfosModuleNameIsempty" == bundleName || "hapModuleInfosModuleNameNotempty" == bundleName) {
                abilityInfo.bundleName = bundleName;
                abilityInfo.moduleName = "moduleNameTest";
                HapModuleInfo hapModuleInfo;
                hapModuleInfo.abilityInfos.emplace_back(abilityInfo);
                bundleInfo.hapModuleInfos.emplace_back(hapModuleInfo);
            }
            if ("hapAbilityInfoVisible" == bundleName) {
                abilityInfo.bundleName = bundleName;
                abilityInfo.moduleName = "moduleNameTest";
                abilityInfo.visible = true;
                HapModuleInfo hapModuleInfo;
                hapModuleInfo.abilityInfos.emplace_back(abilityInfo);
                bundleInfo.hapModuleInfos.emplace_back(hapModuleInfo);
            }
        }
    }
    return true;
}

bool BundleMgrHelper::GetApplicationInfo(
    const std::string& appName, int32_t flags, int32_t userId, ApplicationInfo& appInfo)
{
    return true;
}

std::string BundleMgrHelper::GetAppIdByBundleName(const std::string& bundleName, const int32_t userId)
{
    auto appId = bundleName + "_appId";
    return appId;
}

ErrCode BundleMgrHelper::GetCloneBundleInfo(
    const std::string& bundleName, int32_t flags, int32_t appCloneIndex, BundleInfo& bundleInfo, int32_t userId)
{
    if (bundleName.size() < TOKENID_LENGTH) {
        return -1;
    }
    auto tokenIdStr = bundleName.substr(bundleName.size() - TOKENID_LENGTH, TOKENID_LENGTH);
    if (!IsDigitString(tokenIdStr)) {
        return -1;
    }
    uint32_t tokenId = std::stoi(tokenIdStr);
    bundleInfo.applicationInfo.accessTokenId = tokenId;
    return ERR_OK;
}
} // namespace AppExecFwk
} // namespace OHOS