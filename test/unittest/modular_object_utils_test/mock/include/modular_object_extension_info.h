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

#ifndef MOCK_MODULAR_OBJECT_EXTENSION_INFO_H
#define MOCK_MODULAR_OBJECT_EXTENSION_INFO_H

#include <string>
#include <cstdint>

namespace OHOS {
namespace AAFwk {

enum class MoeLaunchMode { IN_PROCESS = 0, CROSS_PROCESS = 1 };
enum class MoeThreadMode { BUNDLE = 0, TYPE = 1, INSTANCE = 2 };
enum class MoeProcessMode { BUNDLE = 0, TYPE = 1, INSTANCE = 2 };

struct ModularObjectExtensionInfo {
    std::string bundleName;
    std::string moduleName;
    std::string abilityName;
    int32_t appIndex = 0;
    MoeLaunchMode launchMode = MoeLaunchMode::IN_PROCESS;
    MoeProcessMode processMode = MoeProcessMode::BUNDLE;
    MoeThreadMode threadMode = MoeThreadMode::BUNDLE;
    bool isDisabled = false;
};

}  // namespace AAFwk
}  // namespace OHOS

#endif  // MOCK_MODULAR_OBJECT_EXTENSION_INFO_H
