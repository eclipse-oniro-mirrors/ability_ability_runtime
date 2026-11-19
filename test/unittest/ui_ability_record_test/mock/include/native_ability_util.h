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

#ifndef OHOS_ABILITY_RUNTIME_MOCK_NATIVE_ABILITY_UTIL_H
#define OHOS_ABILITY_RUNTIME_MOCK_NATIVE_ABILITY_UTIL_H

#include "ability_info.h"

namespace OHOS {
namespace AAFwk {
enum class StartupPhase : uint8_t {
    PRE_WINDOW,
    PRE_FOREGROUND,
    FOREGROUND,
};

struct NativeAbilityMetaData {
public:
    static void InitData(const AppExecFwk::AbilityInfo &abilityInfo, NativeAbilityMetaData &data);

    static void SetMockInitData(bool withNativeModule, StartupPhase phase = StartupPhase::PRE_WINDOW);
    static void ResetMock();

    bool withNativeModule = false;
    StartupPhase startupPhase = StartupPhase::PRE_WINDOW;
    std::string nativeModuleSource;
    std::string nativeModuleFunc;

private:
    static bool mockWithNativeModule_;
    static StartupPhase mockStartupPhase_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_MOCK_NATIVE_ABILITY_UTIL_H
