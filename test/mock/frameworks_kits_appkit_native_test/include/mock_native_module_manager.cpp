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

#include "mock_native_module_manager.h"

namespace {
    NativeModuleManager* g_instance = nullptr;
    NativeModule* g_module = nullptr;
}

void MockSetNativeModuleManager(NativeModuleManager* instance)
{
    g_instance = instance;
}

void MockSetNativeModule(NativeModule* module)
{
    g_module = module;
}

NativeModuleManager* NativeModuleManager::GetInstance()
{
    static NativeModuleManager realMockInstance;
    if (g_instance == reinterpret_cast<NativeModuleManager*>(1)) {
        return &realMockInstance;
    }
    return g_instance;
}

NativeModule* NativeModuleManager::LoadNativeModule(const char* moduleName, const char* path, bool isAppModule,
    std::string& errInfo, bool internal, const char* relativePath, std::string* loadErrInfo)
{
    if (g_module == nullptr) {
        errInfo = "mock load failed";
    }
    return g_module;
}

