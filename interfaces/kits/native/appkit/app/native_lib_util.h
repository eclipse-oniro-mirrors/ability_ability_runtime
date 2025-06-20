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

void GetHapSoPath(const HapModuleInfo &hapInfo, AppLibPathMap &appLibPaths, bool isPreInstallApp,
    AppLibPathMap &appAbcLibPaths);

<<<<<<< HEAD
<<<<<<< HEAD
void GetHspNativeLibPath(const BaseSharedBundleInfo &hspInfo, AppLibPathMap &appLibPaths, bool isPreInstallApp);
=======
void GetHspNativeLibPath(const BaseSharedBundleInfo &hspInfo, AppLibPathMap &appLibPaths, bool isPreInstallApp,
    const std::string &appBundleName, AppLibPathMap &appAbcLibPaths);
>>>>>>> fe783cf77a (feature SetAppAbcLibPath-2)
=======
void GetHspNativeLibPath(const BaseSharedBundleInfo &hspInfo, AppLibPathMap &appLibPaths, bool isPreInstallApp,
    AppLibPathMap &appAbcLibPaths);
>>>>>>> 8c5b847421 (feature SetAppAbcLibPath)

void GetPatchNativeLibPath(const HapModuleInfo &hapInfo, std::string &patchNativeLibraryPath,
    AppLibPathMap &appLibPaths, AppLibPathMap &appAbcLibPaths);
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_NATIVE_LIB_UTIL_H
