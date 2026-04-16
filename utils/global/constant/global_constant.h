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

#ifndef OHOS_ABILITY_RUNTIME_GLOBAL_CONSTANT_H
#define OHOS_ABILITY_RUNTIME_GLOBAL_CONSTANT_H

namespace OHOS::AbilityRuntime {
namespace GlobalConstant {
constexpr int32_t MAX_APP_CLONE_INDEX = 1000;

constexpr int32_t TIMEOUT_UNIT_TIME = 1000;
constexpr int32_t TIMEOUT_UNIT_TIME_MICRO = 1000 * 1000;

constexpr int32_t PREPARE_TERMINATE_TIMEOUT_TIME = 10000;
constexpr int32_t DEFAULT_FFRT_TASK_TIMEOUT = 60 * 1000 * 1000; // 60s = 60 000 000us
constexpr int32_t FFRT_TASK_TIMEOUT = 5 * 1000 * 1000;  // 5s
constexpr const char* LOW_MEMORY_KILL = "LowMemoryKill";
constexpr const char* PAGE_CONFIG = "ohos.abilityRuntime.pageConfig";
constexpr const char* GAME_PRELAUNCH = "ohos.params.gamePrelaunch";
constexpr int32_t GAME_SA_UID = 7800;

#ifdef SUPPORT_ASAN
constexpr int32_t COLDSTART_TIMEOUT_MULTIPLE = 15000;
constexpr int32_t LOAD_TIMEOUT_MULTIPLE = 15000;
constexpr int32_t FOREGROUND_TIMEOUT_MULTIPLE = 7500;
constexpr int32_t BACKGROUND_TIMEOUT_MULTIPLE = 4500;
constexpr int32_t INSIGHT_INTENT_TIMEOUT_MULTIPLE = 15000;
constexpr int32_t ACTIVE_TIMEOUT_MULTIPLE = 7500;
constexpr int32_t TERMINATE_TIMEOUT_MULTIPLE = 15000;
constexpr int32_t INACTIVE_TIMEOUT_MULTIPLE = 800;
constexpr int32_t INACTIVE_TIMEOUT_MULTIPLE_NEW = 800;
constexpr int32_t DUMP_TIMEOUT_MULTIPLE = 1500;
constexpr int32_t SHAREDATA_TIMEOUT_MULTIPLE = 7500;
constexpr int32_t CONCURRENT_START_TIMEOUT = 10;
#else
constexpr int32_t COLDSTART_TIMEOUT_MULTIPLE = 10;
constexpr int32_t LOAD_TIMEOUT_MULTIPLE = 10;
constexpr int32_t FOREGROUND_TIMEOUT_MULTIPLE = 5;
constexpr int32_t BACKGROUND_TIMEOUT_MULTIPLE = 3;
constexpr int32_t INSIGHT_INTENT_TIMEOUT_MULTIPLE = 10;
constexpr int32_t ACTIVE_TIMEOUT_MULTIPLE = 5;
constexpr int32_t TERMINATE_TIMEOUT_MULTIPLE = 10;
constexpr int32_t INACTIVE_TIMEOUT_MULTIPLE = 1;
constexpr int32_t INACTIVE_TIMEOUT_MULTIPLE_NEW = 21;
constexpr int32_t DUMP_TIMEOUT_MULTIPLE = 1000;
constexpr int32_t SHAREDATA_TIMEOUT_MULTIPLE = 5;
constexpr int32_t CONCURRENT_START_TIMEOUT = 1;
constexpr int32_t TYPE_RESERVE = 1;
constexpr int32_t TYPE_OTHERS = 2;
#endif

constexpr int32_t MIGRATE_CLIENT_TIMEOUT_MULTIPLE = 3;

constexpr int32_t GetLoadTimeOutBase()
{
    return TIMEOUT_UNIT_TIME * LOAD_TIMEOUT_MULTIPLE;
}

constexpr int32_t GetLoadAndInactiveTimeout()
{
    return TIMEOUT_UNIT_TIME * (LOAD_TIMEOUT_MULTIPLE + INACTIVE_TIMEOUT_MULTIPLE);
}

}  // namespace GlobalConstant
}  // namespace OHOS::AbilityRuntime
#endif  // OHOS_ABILITY_RUNTIME_GLOBAL_CONSTANT_H