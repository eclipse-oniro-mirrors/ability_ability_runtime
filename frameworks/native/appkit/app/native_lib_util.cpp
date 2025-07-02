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

#include "native_lib_util.h"

#include "constants.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
constexpr char APP_ABC_LIB_PATH_KEY_PREFIX[] = "/data/storage/el1/bundle/";
constexpr char APP_ABC_LIB_PATH_KEY_SUFFIX[] = "/ets/modules_static.abc";
std::string GetLibPath(const std::string &hapPath, bool isPreInstallApp)
{
    std::string libPath = AbilityBase::Constants::LOCAL_CODE_PATH;
    if (isPreInstallApp) {
        auto pos = hapPath.rfind("/");
        libPath = hapPath.substr(0, pos);
    }
    return libPath;
}

void GetHapSoPath(const HapModuleInfo &hapInfo, AppLibPathMap &appLibPaths, bool isPreInstallApp,
    AppLibPathMap &appAbcLibPaths)
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
        libPath = GetLibPath(hapInfo.hapPath, isPreInstallApp);
    }

    libPath += (libPath.back() == '/') ? hapInfo.nativeLibraryPath : "/" + hapInfo.nativeLibraryPath;
    TAG_LOGD(
        AAFwkTag::APPKIT, "appLibPathKey: %{private}s, lib path: %{private}s", appLibPathKey.c_str(), libPath.c_str());
    appLibPaths[appLibPathKey].emplace_back(libPath);

    std::string appLibAbcPathKey = APP_ABC_LIB_PATH_KEY_PREFIX + hapInfo.moduleName + APP_ABC_LIB_PATH_KEY_SUFFIX;
    appAbcLibPaths[appLibAbcPathKey].emplace_back(libPath);
}

void GetHspNativeLibPath(const BaseSharedBundleInfo &hspInfo, AppLibPathMap &appLibPaths, bool isPreInstallApp,
    const std::string &appBundleName, AppLibPathMap &appAbcLibPaths)
{
    if (hspInfo.nativeLibraryPath.empty()) {
        return;
    }

    std::string appLibPathKey = hspInfo.bundleName + "/" + hspInfo.moduleName;
    std::string libPath = AbilityBase::Constants::LOCAL_CODE_PATH;
    if (!hspInfo.compressNativeLibs) {
        libPath = GetLibPath(hspInfo.hapPath, isPreInstallApp);
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
        const bool isInternalHsp = (hspInfo.moduleName == appBundleName);
        const std::string name = isInternalHsp ? hspInfo.moduleName : hspInfo.bundleName + "/" + hspInfo.moduleName;
        const std::string appLibAbcPathKey = APP_ABC_LIB_PATH_KEY_PREFIX + name + APP_ABC_LIB_PATH_KEY_SUFFIX;
        appAbcLibPaths[appLibAbcPathKey].emplace_back(libPath);
    }
}

void GetPatchNativeLibPath(const HapModuleInfo &hapInfo, std::string &patchNativeLibraryPath,
    AppLibPathMap &appLibPaths, AppLibPathMap &appAbcLibPaths)
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
    appAbcLibPaths[appLibAbcPathKey].emplace_back(patchLibPath);
}
} // AppExecFwk
} // namespace OHOS