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

#ifndef OHOS_ABILITY_RUNTIME_NATIVE_ABILITY_UTIL_H
#define OHOS_ABILITY_RUNTIME_NATIVE_ABILITY_UTIL_H

#include "ability_info.h"

namespace OHOS {
namespace AAFwk {
enum class StartupPhase : uint8_t {
    PRE_WINDOW, // onCreate
    PRE_FOREGROUND, // onWindowStage create
    FOREGROUND, // onForeground
};

struct NativeAbilityMetaData {
public:
    static void InitData(const AppExecFwk::AbilityInfo &abilityInfo, NativeAbilityMetaData &data);
    static bool HideWindowOnStartup(const AppExecFwk::AbilityInfo &abilityInfo);
    static bool IsWithNative(const AppExecFwk::AbilityInfo &abilityInfo);
    bool withNativeModule = false;
    StartupPhase startupPhase = StartupPhase::PRE_WINDOW;
    std::string nativeModuleSource;
    std::string nativeModuleFunc;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_NATIVE_ABILITY_UTIL_H
