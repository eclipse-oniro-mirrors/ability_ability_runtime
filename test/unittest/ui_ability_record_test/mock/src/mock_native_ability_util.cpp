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

#include "native_ability_util.h"

namespace OHOS {
namespace AAFwk {
bool NativeAbilityMetaData::mockWithNativeModule_ = false;
StartupPhase NativeAbilityMetaData::mockStartupPhase_ = StartupPhase::PRE_WINDOW;

void NativeAbilityMetaData::InitData(
    const AppExecFwk::AbilityInfo &abilityInfo,
    NativeAbilityMetaData &data)
{
    data.withNativeModule = mockWithNativeModule_;
    data.startupPhase = mockStartupPhase_;
    data.nativeModuleSource = mockWithNativeModule_ ? "libmock.so" : "";
    data.nativeModuleFunc = mockWithNativeModule_ ? "MockMain" : "";
}

void NativeAbilityMetaData::SetMockInitData(bool withNativeModule, StartupPhase phase)
{
    mockWithNativeModule_ = withNativeModule;
    mockStartupPhase_ = phase;
}

void NativeAbilityMetaData::ResetMock()
{
    mockWithNativeModule_ = false;
    mockStartupPhase_ = StartupPhase::PRE_WINDOW;
}
}  // namespace AAFwk
}  // namespace OHOS
