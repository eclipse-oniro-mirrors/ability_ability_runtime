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

#ifndef OHOS_ABILITY_RUNTIME_ETS_INTERFACE_H
#define OHOS_ABILITY_RUNTIME_ETS_INTERFACE_H

#include <functional>
#include <map>
#include <string>
#include "ets_exception_callback.h"
#include "ets_native_reference.h"
#include "napi/native_api.h"

extern "C" {
struct ETSEnvFuncs {
    void (*InitETSSDKNS)(const std::string &path) = nullptr;
    void (*InitETSSysNS)(const std::string &path) = nullptr;

    bool (*Initialize)() = nullptr;
    void (*RegisterUncaughtExceptionHandler)(
        const OHOS::EtsEnv::ETSUncaughtExceptionInfo &uncaughtExceptionInfo) = nullptr;
    ani_env *(*GetAniEnv)() = nullptr;
    void (*HandleUncaughtError)() = nullptr;
    bool (*PreloadModule)(const std::string &modulePath) = nullptr;
    bool (*LoadModule)(const std::string &modulePath, const std::string &srcEntrance, void *&cls,
        void *&obj, void *&ref) = nullptr;
    void (*SetAppLibPath)(const std::map<std::string, std::string> &abcPathsToBundleModuleNameMap,
        std::function<bool(const std::string &bundleModuleName, std::string &namespaceName)> &cb) = nullptr;
    void (*FinishPreload)() = nullptr;
    void (*PostFork)(void *napiEnv, const std::string &aotPath) = nullptr;
    void (*PreloadSystemClass)(const char *className) = nullptr;
};
}
#endif // OHOS_ABILITY_RUNTIME_ETS_INTERFACE_H
