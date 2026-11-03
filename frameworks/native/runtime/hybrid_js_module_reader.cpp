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

#include "hybrid_js_module_reader.h"

#include <regex>
#include "bundle_info.h"
#include "bundle_mgr_helper.h"
#include "bundle_mgr_proxy.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "iservice_registry.h"
#include "js_runtime_utils.h"
#include "singleton.h"

using namespace OHOS::AbilityBase;

namespace OHOS {
namespace AbilityRuntime {

HybridJsModuleReader::HybridJsModuleReader(const std::string& bundleName, const std::string& hapPath, bool isFormRender)
    : JsModuleSearcher(bundleName), isFormRender_(isFormRender)
{
    if (!hapPath.empty() && hapPath.find(std::string(ABS_DATA_CODE_PATH)) != 0) {
        isSystemPath_ = true;
    } else {
        isSystemPath_ = false;
    }
}

std::shared_ptr<Extractor> HybridJsModuleReader::GetExtractor(
    const std::string& inputPath, std::string& errorMsg) const
{
    bool needFindPluginHsp = true;
    auto realHapPath = GetAppPath(inputPath, SHARED_FILE_SUFFIX, needFindPluginHsp);
    if (realHapPath.empty()) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "empty realHapPath");
        return nullptr;
    }
    if (needFindPluginHsp) {
        realHapPath = GetPluginHspPath(inputPath);
        if (realHapPath.empty()) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "empty realHapPath");
            return nullptr;
        }
        needFindPluginHsp = true;
    }
    bool newCreate = false;
    std::shared_ptr<Extractor> extractor = ExtractorUtil::GetExtractor(realHapPath, newCreate);
    if (extractor != nullptr) {
        return extractor;
    }

    realHapPath = GetAppPath(inputPath, ABILITY_FILE_SUFFIX, needFindPluginHsp);
    if (realHapPath.empty()) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "empty realHapPath");
        return nullptr;
    }
    extractor = ExtractorUtil::GetExtractor(realHapPath, newCreate);
    if (extractor == nullptr) {
        errorMsg = "hap path error: " + inputPath;
        TAG_LOGE(AAFwkTag::JSRUNTIME, "inputPath %{private}s GetExtractor failed", inputPath.c_str());
        return nullptr;
    }
    return extractor;
}

bool HybridJsModuleReader::operator()(const std::string& inputPath,
    uint8_t **buff, size_t *buffSize, std::string& errorMsg) const
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "called start: %{private}s", inputPath.c_str());
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (inputPath.empty() || buff == nullptr || buffSize == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Invalid param");
        return false;
    }

    std::shared_ptr<Extractor> extractor = GetExtractor(inputPath, errorMsg);
    if (extractor == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "failed to get extractor %{private}s", inputPath.c_str());
        return false;
    }

    auto data = extractor->GetSafeData(MERGE_ABC_PATH);
    if (!data) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null data");
        return false;
    }

    *buff = data->GetDataPtr();
    *buffSize = data->GetDataLen();
    return true;
}

std::string HybridJsModuleReader::GetPluginHspPath(const std::string& inputPath) const
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
                presetAppHapPath = ExtractorUtil::GetLoadFilePath(presetAppHapPath);
                TAG_LOGD(AAFwkTag::JSRUNTIME, "presetAppHapPath %{public}s", presetAppHapPath.c_str());
                return presetAppHapPath;
            }
        }
    }
    TAG_LOGE(AAFwkTag::JSRUNTIME, "GetPluginHspPath failed");
    return presetAppHapPath;
}

std::string HybridJsModuleReader::GetAppPath(const std::string& inputPath,
    const std::string& suffix, bool& needFindPluginHsp) const
{
    if (isFormRender_) {
        return GetFormAppPath(inputPath, suffix, needFindPluginHsp);
    }
    return GetCommonAppPath(inputPath, suffix, needFindPluginHsp);
}

std::string HybridJsModuleReader::GetFormAppPath(const std::string& inputPath,
    const std::string& suffix, bool& needFindPluginHsp) const
{
    std::string realHapPath;
    realHapPath.append("/data/bundles/")
        .append(bundleName_).append("/")
        .append(GetModuleName(inputPath))
        .append(SHARED_FILE_SUFFIX);

    TAG_LOGI(AAFwkTag::JSRUNTIME, "realHapPath: %{private}s", realHapPath.c_str());
    if (realHapPath.empty() ||
        realHapPath.length() < suffix.length() ||
        realHapPath.compare(realHapPath.length() - suffix.length(), suffix.length(), suffix) != 0) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "obtain realHapPath failed");
        return realHapPath;
    }
    needFindPluginHsp = false;
    return realHapPath;
}

std::string HybridJsModuleReader::GetModuleName(const std::string& inputPath) const
{
    return inputPath.substr(inputPath.find_last_of("/") + 1);
}

std::string HybridJsModuleReader::GetCommonAppPath(const std::string& inputPath,
    const std::string& suffix, bool& needFindPluginHsp) const
{
    std::string realHapPath = GetPresetAppHapPath(inputPath, bundleName_, needFindPluginHsp);
    if ((realHapPath.find(ABS_DATA_CODE_PATH) == 0) || (realHapPath == inputPath)) {
        realHapPath = std::string(ABS_CODE_PATH) + inputPath + suffix;
    }

    TAG_LOGD(AAFwkTag::JSRUNTIME, "realHapPath: %{private}s", realHapPath.c_str());
    if (realHapPath.empty() ||
        realHapPath.length() < suffix.length() ||
        realHapPath.compare(realHapPath.length() - suffix.length(), suffix.length(), suffix) != 0) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "obtain realHapPath failed");
        return realHapPath;
    }
    return realHapPath;
}

std::string HybridJsModuleReader::GetOtherHspPath(const std::string& bundleName, const std::string& moduleName,
    const std::string& inputPath, bool& needFindPluginHsp)
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
        TAG_LOGE(AAFwkTag::JSRUNTIME, "GetDependentBundleInfo failed");
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

std::string HybridJsModuleReader::GetPresetAppHapPath(const std::string& inputPath,
    const std::string& bundleName, bool& needFindPluginHsp)
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
        AppExecFwk::BundleInfo bundleInfo;
        auto getInfoResult = bundleMgrHelper->GetBundleInfoForSelf(static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::
            GET_BUNDLE_INFO_WITH_HAP_MODULE), bundleInfo);
        if (getInfoResult != 0 || bundleInfo.hapModuleInfos.empty()) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "GetBundleInfoForSelf failed");
            return presetAppHapPath;
        }
        for (auto hapModuleInfo : bundleInfo.hapModuleInfos) {
            if (hapModuleInfo.moduleName == moduleName) {
                presetAppHapPath = hapModuleInfo.hapPath;
                needFindPluginHsp = false;
                break;
            }
        }
    } else {
        presetAppHapPath = GetOtherHspPath(bundleName, moduleName, presetAppHapPath, needFindPluginHsp);
    }
    return presetAppHapPath;
}
} // namespace AbilityRuntime
} // namespace OHOS