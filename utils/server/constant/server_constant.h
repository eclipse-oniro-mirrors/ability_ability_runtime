/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_SERVER_CONSTANT_H
#define OHOS_ABILITY_RUNTIME_SERVER_CONSTANT_H

namespace OHOS::AbilityRuntime {
namespace ServerConstant {
constexpr const char* DLP_INDEX = "ohos.dlp.params.index";
constexpr const char* IS_CALL_BY_SCB = "isCallBySCB";
constexpr int32_t SCENARIO_MOVE_MISSION_TO_FRONT = 0x00000001;
constexpr int32_t SCENARIO_SHOW_ABILITY = 0x00000002;
constexpr int32_t SCENARIO_BACK_TO_CALLER_ABILITY_WITH_RESULT = 0x00000004;
constexpr int32_t SCENARIO_BACK_TO_CALLER_APP_WITH_RESULT = 0x00000008;
constexpr uint64_t DEFAULT_DISPLAY_ID = 0;
}  // namespace ServerConstant
}  // namespace OHOS::AbilityRuntime
#endif  // OHOS_ABILITY_RUNTIME_SERVER_CONSTANT_H