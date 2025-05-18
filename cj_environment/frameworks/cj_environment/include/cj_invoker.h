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

#ifndef OHOS_ABILITY_RUNTIME_CJ_INVOKER_H
#define OHOS_ABILITY_RUNTIME_CJ_INVOKER_H

#include "cj_interface.h"

#include <cstddef>
#include <cstdint>
#include <csignal>

namespace OHOS {
using PostTaskType = bool(*)(void*);
using HasHigherPriorityType = bool(*)();
using UpdateStackInfoFuncType = void(*)(unsigned long long, void*, unsigned int);

struct CJUncaughtExceptionInfo;

struct CJRuntimeAPI {
    int (*InitCJRuntime)(const struct RuntimeParam*);
    void* (*InitUIScheduler)();
    int (*RunUIScheduler)(unsigned long long);
    int (*FiniCJRuntime)();
    int (*InitCJLibrary)(const char*);
    void (*RegisterEventHandlerCallbacks)(PostTaskType, HasHigherPriorityType);
    void (*RegisterCJUncaughtExceptionHandler)(const CJUncaughtExceptionInfo& handle);
    void (*RegisterArkVMInRuntime)(unsigned long long);
    void (*RegisterStackInfoCallbacks)(UpdateStackInfoFuncType);
    void (*DumpHeapSnapshot)(int fd);
    void (*ForceFullGC)();
};
}

#endif //OHOS_ABILITY_RUNTIME_CJ_INVOKER_H
