/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "native_lib_path_util.h"

#include "constants.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {

std::string GetDefaultAppLibPath(const std::string &nativeLibraryPath)
{
    if (nativeLibraryPath.empty()) {
        return "";
    }
    std::string path = nativeLibraryPath;
    if (path.back() == '/') {
        path.pop_back();
    }
    std::string libPath = AbilityBase::Constants::LOCAL_CODE_PATH;
    libPath += (libPath.back() == '/') ? path : "/" + path;
    TAG_LOGD(AAFwkTag::APPMGR, "default app lib path: %{private}s", libPath.c_str());
    return libPath;
}

void GetDefaultNamespacePaths(const BundleInfo &bundleInfo, LibPathMap &appLibPaths)
{
    std::string patchNativeLibraryPath =
        bundleInfo.applicationInfo.appQuickFix.deployedAppqfInfo.nativeLibraryPath;
    if (!patchNativeLibraryPath.empty()) {
        std::string patchLibPath = AbilityBase::Constants::LOCAL_CODE_PATH;
        patchLibPath += (patchLibPath.back() == '/') ?
            patchNativeLibraryPath : "/" + patchNativeLibraryPath;
        TAG_LOGD(AAFwkTag::APPMGR, "default patch lib path: %{private}s", patchLibPath.c_str());
        appLibPaths["default"].emplace_back(patchLibPath);
    }

    std::string nativeLibraryPath = bundleInfo.applicationInfo.nativeLibraryPath;
    if (!nativeLibraryPath.empty()) {
        std::string libPath = GetDefaultAppLibPath(nativeLibraryPath);
        if (!libPath.empty()) {
            appLibPaths["default"].emplace_back(libPath);
        }
    }
    TAG_LOGD(AAFwkTag::APPMGR, "default namespace paths count: %{public}zu",
        appLibPaths["default"].size());
}

void GetModuleSoPaths(const BundleInfo &bundleInfo,
    const std::vector<BaseSharedBundleInfo> &hspList,
    LibPathMap &appLibPaths)
{
    std::string patchNativeLibraryPath =
        bundleInfo.applicationInfo.appQuickFix.deployedAppqfInfo.nativeLibraryPath;

    for (auto &hapInfo : bundleInfo.hapModuleInfos) {
        TAG_LOGD(AAFwkTag::APPMGR,
            "moduleName: %{public}s, isLibIsolated: %{public}d, compressNativeLibs: %{public}d",
            hapInfo.moduleName.c_str(), hapInfo.isLibIsolated, hapInfo.compressNativeLibs);
        std::string appLibPathKey = hapInfo.bundleName + "/" + hapInfo.moduleName;
        if (!hapInfo.nativeLibraryPath.empty()) {
            std::string libPath = AbilityBase::Constants::LOCAL_CODE_PATH;
            if (!hapInfo.compressNativeLibs) {
                auto pos = hapInfo.hapPath.rfind("/");
                if (pos != std::string::npos) {
                    libPath = hapInfo.hapPath.substr(0, pos);
                }
            }
            libPath += (libPath.back() == '/') ? hapInfo.nativeLibraryPath :
                "/" + hapInfo.nativeLibraryPath;
            appLibPaths[appLibPathKey].emplace_back(libPath);
        }
        if (!patchNativeLibraryPath.empty() && hapInfo.compressNativeLibs && !hapInfo.isLibIsolated) {
            continue;
        }
        if (!hapInfo.hqfInfo.nativeLibraryPath.empty() && hapInfo.compressNativeLibs) {
            std::string patchLibPath = AbilityBase::Constants::LOCAL_CODE_PATH;
            patchLibPath += (patchLibPath.back() == '/') ?
                hapInfo.hqfInfo.nativeLibraryPath : "/" + hapInfo.hqfInfo.nativeLibraryPath;
            appLibPaths[appLibPathKey].emplace_back(patchLibPath);
        }
    }

    for (auto &hspInfo : hspList) {
        TAG_LOGD(AAFwkTag::APPMGR, "bundle:%s, module:%s, nativeLibraryPath:%s",
            hspInfo.bundleName.c_str(), hspInfo.moduleName.c_str(), hspInfo.nativeLibraryPath.c_str());
        if (hspInfo.nativeLibraryPath.empty()) {
            continue;
        }
        std::string appLibPathKey = hspInfo.bundleName + "/" + hspInfo.moduleName;
        std::string libPath = AbilityBase::Constants::LOCAL_CODE_PATH;
        if (!hspInfo.compressNativeLibs) {
            auto pos = hspInfo.hapPath.rfind("/");
            if (pos != std::string::npos) {
                libPath = hspInfo.hapPath.substr(0, pos);
            }
            libPath = libPath.back() == '/' ? libPath : libPath + "/";
            libPath += hspInfo.nativeLibraryPath;
        } else {
            libPath = libPath.back() == '/' ? libPath : libPath + "/";
            libPath += hspInfo.bundleName + "/" + hspInfo.nativeLibraryPath;
        }
        appLibPaths[appLibPathKey].emplace_back(libPath);
    }
}

} // namespace AppExecFwk
} // namespace OHOS
