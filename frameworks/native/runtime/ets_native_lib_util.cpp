/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#include "ets_native_lib_util.h"

#include "constants.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
constexpr char APP_ABC_LIB_PATH_KEY_PREFIX[] = "/data/storage/el1/bundle/";
constexpr char APP_ABC_LIB_PATH_KEY_SUFFIX[] = "/ets/modules_static.abc";

std::string GetEtsLibPath(const std::string &hapPath, bool isPreInstallApp)
{
    std::string libPath = AbilityBase::Constants::LOCAL_CODE_PATH;
    if (isPreInstallApp) {
        auto pos = hapPath.rfind("/");
        libPath = hapPath.substr(0, pos);
    }
    return libPath;
}

void GetEtsHapSoPath(const HapModuleInfo &hapInfo, AppLibPathMap &appLibPaths, bool isPreInstallApp,
    std::map<std::string, std::string> &abcPathsToBundleModuleNameMap)
{
    if (hapInfo.nativeLibraryPath.empty()) {
        TAG_LOGD(AAFwkTag::APPKIT, "Lib path of %{public}s is empty, lib isn't isolated or compressed",
            hapInfo.moduleName.c_str());
        return;
    }

    std::string appLibPathKey = hapInfo.bundleName + "/" + hapInfo.moduleName;
    std::string libPath = AbilityBase::Constants::LOCAL_CODE_PATH;
    if (!hapInfo.compressNativeLibs) {
        TAG_LOGD(AAFwkTag::APPKIT, "Lib of %{public}s will not be extracted from hap", hapInfo.moduleName.c_str());
        libPath = GetEtsLibPath(hapInfo.hapPath, isPreInstallApp);
    }

    libPath += (libPath.back() == '/') ? hapInfo.nativeLibraryPath : "/" + hapInfo.nativeLibraryPath;
    TAG_LOGD(
        AAFwkTag::APPKIT, "appLibPathKey: %{private}s, lib path: %{private}s", appLibPathKey.c_str(), libPath.c_str());
    appLibPaths[appLibPathKey].emplace_back(libPath);

    std::string appLibAbcPathKey = APP_ABC_LIB_PATH_KEY_PREFIX + hapInfo.moduleName + APP_ABC_LIB_PATH_KEY_SUFFIX;
    abcPathsToBundleModuleNameMap[appLibAbcPathKey] = appLibPathKey;
}

void GetEtsHspNativeLibPath(const BaseSharedBundleInfo &hspInfo, AppLibPathMap &appLibPaths, bool isPreInstallApp,
    const std::string &appBundleName, std::map<std::string, std::string> &abcPathsToBundleModuleNameMap)
{
    if (hspInfo.nativeLibraryPath.empty()) {
        return;
    }

    std::string appLibPathKey = hspInfo.bundleName + "/" + hspInfo.moduleName;
    std::string libPath = AbilityBase::Constants::LOCAL_CODE_PATH;
    if (!hspInfo.compressNativeLibs) {
        libPath = GetEtsLibPath(hspInfo.hapPath, isPreInstallApp);
        libPath = libPath.back() == '/' ? libPath : libPath + "/";
        if (isPreInstallApp) {
            libPath += hspInfo.nativeLibraryPath;
        } else {
            libPath += hspInfo.bundleName + "/" + hspInfo.moduleName + "/" + hspInfo.nativeLibraryPath;
        }
    } else {
        libPath = libPath.back() == '/' ? libPath : libPath + "/";
        libPath += hspInfo.bundleName + "/" + hspInfo.nativeLibraryPath;
    }

    TAG_LOGD(
        AAFwkTag::APPKIT, "appLibPathKey: %{private}s, libPath: %{private}s", appLibPathKey.c_str(), libPath.c_str());
    appLibPaths[appLibPathKey].emplace_back(libPath);

    if (!appBundleName.empty()) {
        const bool isInternalHsp = (hspInfo.bundleName == appBundleName);
        const std::string name = isInternalHsp ? hspInfo.moduleName : hspInfo.bundleName + "/" + hspInfo.moduleName;
        const std::string appLibAbcPathKey = APP_ABC_LIB_PATH_KEY_PREFIX + name + APP_ABC_LIB_PATH_KEY_SUFFIX;
        abcPathsToBundleModuleNameMap[appLibAbcPathKey] = appLibPathKey;
    }
}

void GetEtsPatchNativeLibPath(const HapModuleInfo &hapInfo, std::string &patchNativeLibraryPath,
    AppLibPathMap &appLibPaths, std::map<std::string, std::string> &abcPathsToBundleModuleNameMap)
{
    if (hapInfo.isLibIsolated) {
        patchNativeLibraryPath = hapInfo.hqfInfo.nativeLibraryPath;
    }

    if (patchNativeLibraryPath.empty()) {
        TAG_LOGD(AAFwkTag::APPKIT, "Patch lib path of %{public}s is empty", hapInfo.moduleName.c_str());
        return;
    }

    if (hapInfo.compressNativeLibs && !hapInfo.isLibIsolated) {
        TAG_LOGD(AAFwkTag::APPKIT, "Lib of %{public}s has compressed and isn't isolated, no need to set",
            hapInfo.moduleName.c_str());
        return;
    }

    std::string appLibPathKey = hapInfo.bundleName + "/" + hapInfo.moduleName;
    std::string patchLibPath = AbilityBase::Constants::LOCAL_CODE_PATH;
    patchLibPath += (patchLibPath.back() == '/') ? patchNativeLibraryPath : "/" + patchNativeLibraryPath;
    TAG_LOGD(AAFwkTag::APPKIT, "appLibPathKey: %{public}s, patch lib path: %{private}s", appLibPathKey.c_str(),
        patchLibPath.c_str());
    appLibPaths[appLibPathKey].emplace_back(patchLibPath);
    std::string appLibAbcPathKey = APP_ABC_LIB_PATH_KEY_PREFIX + hapInfo.moduleName + APP_ABC_LIB_PATH_KEY_SUFFIX;
    abcPathsToBundleModuleNameMap[appLibAbcPathKey] = appLibPathKey;
}

void GetEtsNativeLibPath(const BundleInfo &bundleInfo, const std::vector<BaseSharedBundleInfo> &hspList,
    AppLibPathMap &appLibPaths, std::map<std::string, std::string> &abcPathsToBundleModuleNameMap)
{
    std::string patchNativeLibraryPath = bundleInfo.applicationInfo.appQuickFix.deployedAppqfInfo.nativeLibraryPath;
    abcPathsToBundleModuleNameMap["default"] = "default";
    if (!patchNativeLibraryPath.empty()) {
        // libraries in patch lib path has a higher priority when loading.
        std::string patchLibPath = AbilityBase::Constants::LOCAL_CODE_PATH;
        patchLibPath += (patchLibPath.back() == '/') ? patchNativeLibraryPath : "/" + patchNativeLibraryPath;
        TAG_LOGD(AAFwkTag::APPKIT, "lib path = %{private}s", patchLibPath.c_str());
        appLibPaths["default"].emplace_back(patchLibPath);
    }

    std::string nativeLibraryPath = bundleInfo.applicationInfo.nativeLibraryPath;
    if (!nativeLibraryPath.empty()) {
        if (nativeLibraryPath.back() == '/') {
            nativeLibraryPath.pop_back();
        }
        std::string libPath = AbilityBase::Constants::LOCAL_CODE_PATH;
        libPath += (libPath.back() == '/') ? nativeLibraryPath : "/" + nativeLibraryPath;
        TAG_LOGD(AAFwkTag::APPKIT, "lib path = %{private}s", libPath.c_str());
        appLibPaths["default"].emplace_back(libPath);
    } else {
        TAG_LOGI(AAFwkTag::APPKIT, "nativeLibraryPath is empty");
    }

    for (auto &hapInfo : bundleInfo.hapModuleInfos) {
        TAG_LOGD(AAFwkTag::APPKIT,
            "moduleName: %{public}s, isLibIsolated: %{public}d, compressNativeLibs: %{public}d.",
            hapInfo.moduleName.c_str(), hapInfo.isLibIsolated, hapInfo.compressNativeLibs);
        GetEtsPatchNativeLibPath(hapInfo, patchNativeLibraryPath, appLibPaths, abcPathsToBundleModuleNameMap);
        GetEtsHapSoPath(hapInfo, appLibPaths, hapInfo.hapPath.find(AbilityBase::Constants::ABS_CODE_PATH),
            abcPathsToBundleModuleNameMap);
    }

    for (auto &hspInfo : hspList) {
        TAG_LOGD(AAFwkTag::APPKIT, "bundle:%s, module:%s, nativeLibraryPath:%s", hspInfo.bundleName.c_str(),
            hspInfo.moduleName.c_str(), hspInfo.nativeLibraryPath.c_str());
        GetEtsHspNativeLibPath(hspInfo, appLibPaths, hspInfo.hapPath.find(AbilityBase::Constants::ABS_CODE_PATH) != 0u,
            bundleInfo.applicationInfo.bundleName, abcPathsToBundleModuleNameMap);
    }
}
} // AppExecFwk
} // namespace OHOS