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

#ifndef OHOS_ABILITY_RUNTIME_NATIVE_LIB_UTIL_H
#define OHOS_ABILITY_RUNTIME_NATIVE_LIB_UTIL_H

#include <string>

#include "base_shared_bundle_info.h"
#include "hap_module_info.h"
#include "js_runtime.h"

namespace OHOS {
namespace AppExecFwk {
std::string GetLibPath(const std::string &hapPath, bool isPreInstallApp);

void GetHapSoPath(const HapModuleInfo &hapInfo, AppLibPathMap &appLibPaths, bool isPreInstallApp);

void GetHspNativeLibPath(const BaseSharedBundleInfo &hspInfo, AppLibPathMap &appLibPaths, bool isPreInstallApp);

void GetPatchNativeLibPath(const HapModuleInfo &hapInfo, std::string &patchNativeLibraryPath,
    AppLibPathMap &appLibPaths);

void GetLibrarySupportDirectory(
    const std::vector<HapModuleInfo> &hapModuleInfos,
    const std::string &nativeLibraryPath,
    AppLibPathMap &appLibPaths);

void TraverseLibrarySupportDirectory(
    const std::vector<std::string> &librarySupportDirectory,
    const std::string &prefixPath,
    const std::string &appLibPathKey,
    AppLibPathMap &appLibPaths);
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_NATIVE_LIB_UTIL_H
