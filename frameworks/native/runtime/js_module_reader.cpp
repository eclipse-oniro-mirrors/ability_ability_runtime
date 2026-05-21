/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include <regex>
#include "js_module_reader.h"

#include "bundle_info.h"
#include "bundle_mgr_helper.h"
#include "bundle_mgr_proxy.h"
#include "file_path_utils.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "iservice_registry.h"
#include "js_runtime_utils.h"
#include "singleton.h"
#include "system_ability_definition.h"

using namespace OHOS::AbilityBase;

namespace OHOS {
namespace AbilityRuntime {
using IBundleMgr = AppExecFwk::IBundleMgr;

JsModuleReader::JsModuleReader(const std::string& bundleName, const std::string& hapPath, bool isFormRender)
    : JsModuleSearcher(bundleName), isFormRender_(isFormRender)
{
    if (!hapPath.empty() && hapPath.find(std::string(ABS_DATA_CODE_PATH)) != 0) {
        isSystemPath_ = true;
    } else {
        isSystemPath_ = false;
    }
}

bool JsModuleReader::operator()(const std::string& inputPath, uint8_t **buff,
    size_t *buffSize, std::string& errorMsg) const
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "called start: %{private}s", inputPath.c_str());
    std::string connector = "##";
    std::string traceName = __PRETTY_FUNCTION__ + connector + inputPath;
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, traceName);
    if (inputPath.empty() || buff == nullptr || buffSize == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Invalid param");
        return false;
    }

    bool needFindPluginHsp = true;
    auto realHapPath = GetAppHspPath(inputPath, needFindPluginHsp, errorMsg);
    TAG_LOGD(AAFwkTag::JSRUNTIME, "GetAppHspPath: %{public}d, inputPath: %{private}s, appHspPath: %{private}s",
        !needFindPluginHsp, inputPath.c_str(), realHapPath.c_str());
    if (realHapPath.empty()) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "empty appHspPath");
        return false;
    }

    if (needFindPluginHsp) {
        realHapPath = GetPluginHspPath(inputPath);
        if (realHapPath.empty()) {
            if (errorMsg.empty()) {
                errorMsg = "get plugin hspPath failed, please check the module is correct";
            }
            TAG_LOGE(AAFwkTag::JSRUNTIME, "empty pluginHspPath");
            return false;
        }
    }

    bool newCreate = false;
    std::shared_ptr<Extractor> extractor = ExtractorUtil::GetExtractor(realHapPath, newCreate);
    if (extractor == nullptr) {
        errorMsg = "GetExtractor failed, please check " + realHapPath;
        TAG_LOGE(AAFwkTag::JSRUNTIME, "realHapPath %{private}s GetExtractor failed", realHapPath.c_str());
        return false;
    }

    auto data = extractor->GetSafeData(MERGE_ABC_PATH);
    if (!data) {
        errorMsg = "getData failed, please check " + realHapPath;
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null data");
        return false;
    }

    *buff = data->GetDataPtr();
    *buffSize = data->GetDataLen();
    return true;
}

std::string JsModuleReader::GetPluginHspPath(const std::string& inputPath) const
{
    std::string presetAppHapPath = "";
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null bundleMgrHelper");
        return presetAppHapPath;
    }
    std::string moduleName = inputPath.substr(inputPath.find_last_of("/") + 1);
    std::string tmpPath = inputPath.substr(inputPath.find_first_of("/") + 1);
    const std::string sharedBundleName = tmpPath.substr(0, tmpPath.find_first_of("/"));
    TAG_LOGI(AAFwkTag::JSRUNTIME, "moduleName: %{public}s, sharedBundleName: %{public}s",
        moduleName.c_str(), sharedBundleName.c_str());
    if (moduleName.empty() || sharedBundleName.empty()) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "empty moduleName");
        return presetAppHapPath;
    }

    std::vector<AppExecFwk::PluginBundleInfo> pluginBundleInfos;
    if (bundleMgrHelper->GetPluginInfosForSelf(pluginBundleInfos) != ERR_OK) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "GetPluginInfosForSelf failed");
        return presetAppHapPath;
    }

    for (auto &pluginBundleInfo : pluginBundleInfos) {
        for (auto &pluginModuleInfo : pluginBundleInfo.pluginModuleInfos) {
            if (moduleName == pluginModuleInfo.moduleName
                && sharedBundleName == pluginBundleInfo.pluginBundleName) {
                presetAppHapPath = pluginModuleInfo.hapPath;
                TAG_LOGD(AAFwkTag::JSRUNTIME, "presetAppHapPath %{public}s", presetAppHapPath.c_str());
                std::regex pattern(std::string(ABS_DATA_CODE_PATH) + bundleName_ + "/");
                presetAppHapPath = std::regex_replace(
                    presetAppHapPath, pattern, std::string(ABS_CODE_PATH) + std::string(BUNDLE));
                TAG_LOGD(AAFwkTag::JSRUNTIME, "presetAppHapPath %{public}s", presetAppHapPath.c_str());
                return presetAppHapPath;
            }
        }
    }
    TAG_LOGE(AAFwkTag::JSRUNTIME, "GetPluginHspPath failed");
    return presetAppHapPath;
}

std::string JsModuleReader::GetAppHspPath(const std::string& inputPath,
    bool& needFindPluginHsp, std::string& errorMsg) const
{
    if (isFormRender_) {
        return GetFormAppHspPath(inputPath, needFindPluginHsp, errorMsg);
    }
    return GetCommonAppHspPath(inputPath, needFindPluginHsp, errorMsg);
}

std::string JsModuleReader::GetFormAppHspPath(const std::string& inputPath,
    bool& needFindPluginHsp, std::string& errorMsg) const
{
    std::string realHapPath;
    std::string suffix = std::string(SHARED_FILE_SUFFIX);
    realHapPath.append("/data/bundles/")
        .append(bundleName_).append("/")
        .append(GetModuleName(inputPath))
        .append(SHARED_FILE_SUFFIX);

    TAG_LOGI(AAFwkTag::JSRUNTIME, "realHapPath: %{private}s", realHapPath.c_str());
    if (realHapPath.empty() ||
        realHapPath.length() < suffix.length() ||
        realHapPath.compare(realHapPath.length() - suffix.length(), suffix.length(), suffix) != 0) {
        errorMsg = "Get form HspPath failed: invalid path or suffix";
        TAG_LOGE(AAFwkTag::JSRUNTIME, "obtain realHapPath failed");
        return realHapPath;
    }
    needFindPluginHsp = false;
    return realHapPath;
}

std::string JsModuleReader::GetModuleName(const std::string& inputPath) const
{
    return inputPath.substr(inputPath.find_last_of("/") + 1);
}

std::string JsModuleReader::GetCommonAppHspPath(const std::string& inputPath,
    bool& needFindPluginHsp, std::string& errorMsg) const
{
    std::string suffix = std::string(SHARED_FILE_SUFFIX);
    std::string realHapPath = GetPresetAppHapPath(inputPath, bundleName_, needFindPluginHsp, errorMsg);
    if (needFindPluginHsp && errorMsg.empty()) {
        if (inputPath.find_first_of("/") == inputPath.find_last_of("/")) {
            errorMsg = "Get in-app HspPath failed, please check the module is correct";
        } else {
            errorMsg = "Get shared HspPath failed, please check the module is correct";
        }
    }
    if ((realHapPath.find(ABS_DATA_CODE_PATH) == 0) || (realHapPath == inputPath)) {
        realHapPath = std::string(ABS_CODE_PATH) + inputPath + suffix;
    }

    TAG_LOGD(AAFwkTag::JSRUNTIME, "realHapPath: %{private}s", realHapPath.c_str());
    if (realHapPath.empty() ||
        realHapPath.length() < suffix.length() ||
        realHapPath.compare(realHapPath.length() - suffix.length(), suffix.length(), suffix) != 0) {
        errorMsg = "Get HspPath failed: invalid path or suffix";
        TAG_LOGE(AAFwkTag::JSRUNTIME, "obtain realHapPath failed");
        return realHapPath;
    }
    return realHapPath;
}

std::string JsModuleReader::GetOtherHspPath(const std::string& bundleName, const std::string& moduleName,
    const std::string& inputPath, bool& needFindPluginHsp, std::string& errorMsg)
{
    std::string presetAppHapPath = inputPath;

    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null bundleMgrHelper");
        return presetAppHapPath;
    }

    std::vector<AppExecFwk::BaseSharedBundleInfo> baseSharedBundleInfos;
    if (bundleMgrHelper->GetBaseSharedBundleInfos(bundleName, baseSharedBundleInfos) != 0) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "GetBaseSharedBundleInfos failed");
        return presetAppHapPath;
    }
    std::string tmpPath = inputPath.substr(inputPath.find_first_of("/") + 1);
    const std::string sharedBundleName = tmpPath.substr(0, tmpPath.find_first_of("/"));
    for (const auto &info : baseSharedBundleInfos) {
        if ((info.bundleName == sharedBundleName) && (info.moduleName == moduleName)) {
            presetAppHapPath = info.hapPath;
            needFindPluginHsp = false;
            break;
        }
    }
    AppExecFwk::BundleInfo bundleInfo;
    int32_t ret = bundleMgrHelper->GetDependentBundleInfo(sharedBundleName, bundleInfo,
        AppExecFwk::GetDependentBundleInfoFlag::GET_APP_SERVICE_HSP_BUNDLE_INFO);
    if (ret != ERR_OK) {
        TAG_LOGW(AAFwkTag::JSRUNTIME, "GetDependentBundleInfo failed");
        return presetAppHapPath;
    }
    for (const auto &info : bundleInfo.hapModuleInfos) {
        if (info.moduleName == moduleName) {
            presetAppHapPath = info.hapPath;
            needFindPluginHsp = false;
            break;
        }
    }
    return presetAppHapPath;
}

std::string JsModuleReader::GetPresetAppHapPath(const std::string& inputPath, const std::string& bundleName,
    bool& needFindPluginHsp, std::string& errorMsg)
{
    std::string presetAppHapPath = inputPath;
    std::string moduleName = inputPath.substr(inputPath.find_last_of("/") + 1);
    if (moduleName.empty()) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "empty moduleName");
        return presetAppHapPath;
    }
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null bundleMgrHelper");
        return presetAppHapPath;
    }
    if (inputPath.find_first_of("/") == inputPath.find_last_of("/")) {
        AppExecFwk::AbilityInfo abilityInfo;
        abilityInfo.bundleName = bundleName;
        abilityInfo.package = moduleName;
        AppExecFwk::HapModuleInfo hapModuleInfo;
        if (!bundleMgrHelper->GetHapModuleInfo(abilityInfo, hapModuleInfo)) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "GetHapModuleInfo failed");
            return presetAppHapPath;
        }
        presetAppHapPath = hapModuleInfo.hapPath;
        needFindPluginHsp = false;
    } else {
        presetAppHapPath = GetOtherHspPath(bundleName, moduleName, presetAppHapPath, needFindPluginHsp, errorMsg);
    }
    return presetAppHapPath;
}

void JsModuleReader::GetHapPathList(const std::string &bundleName, std::vector<std::string> &hapList)
{
    auto systemAbilityManagerClient = OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManagerClient) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null systemAbilityManagerClient");
        return;
    }
    auto remoteObject = systemAbilityManagerClient->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (!remoteObject) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null remoteObject");
        return;
    }
    auto bundleMgrProxy = iface_cast<IBundleMgr>(remoteObject);
    AppExecFwk::BundleInfo bundleInfo;
    auto getInfoResult = bundleMgrProxy->GetBundleInfoForSelf(static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::
        GET_BUNDLE_INFO_WITH_HAP_MODULE), bundleInfo);
    if (getInfoResult != 0 || bundleInfo.hapModuleInfos.empty()) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "GetBundleInfoForSelf failed");
        return;
    }
    for (auto hapModuleInfo : bundleInfo.hapModuleInfos) {
        hapList.emplace_back(hapModuleInfo.hapPath);
    }
}
} // namespace AbilityRuntime
} // namespace OHOS
