/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef MOCK_RUNTIME_H
#define MOCK_RUNTIME_H

#include <gtest/gtest.h>

#include "js_runtime.h"

namespace OHOS {
namespace AbilityRuntime {
class MockRuntime : public Runtime {
public:
    const int DEFAULT_LANGUAGE = 100;
    MockRuntime() = default;
    ~MockRuntime() = default;

    Language GetLanguage() const override
    {
        return static_cast<Runtime::Language>(DEFAULT_LANGUAGE);
    }

    void StartDebugMode(const DebugOption debugOption) override {}

    void FinishPreload() override {}
    bool LoadRepairPatch(const std::string& patchFile, const std::string& baseFile) override
    {
        return true;
    }
    bool NotifyHotReloadPage() override
    {
        return true;
    }
    bool SuspendVM(uint32_t tid) override
    {
        return true;
    }
    void ResumeVM(uint32_t tid) override {}
    bool UnLoadRepairPatch(const std::string& patchFile) override
    {
        return true;
    }
    void DumpHeapSnapshot(bool isPrivate) override
    {
        return;
    }
    void DumpCpuProfile() override
    {
        return;
    }
    void DestroyHeapProfiler() override
    {
        return;
    }
    void ForceFullGC() override
    {
        return;
    }
    void AllowCrossThreadExecution() override
    {
        return;
    }
    void GetHeapPrepare() override
    {
        return;
    }
    void NotifyApplicationState(bool isBackground) override
    {
        return;
    }
    void PreloadSystemModule(const std::string& moduleName) override
    {
        return;
    }
    void PreloadMainAbility(const std::string& moduleName, const std::string& srcPath,
        const std::string& hapPath,  bool isEsMode, const std::string& srcEntrance) override
    {
        return;
    }
    void PreloadModule(const std::string& moduleName, const std::string& srcPath,
        const std::string& hapPath, bool isEsMode, bool useCommonTrunk) override
    {
        return;
    }
    bool RunScript(const std::string& path, const std::string& hapPath, bool useCommonChunk = false)
    {
        return true;
    }
    bool Initialize(const Options& options)
    {
        return true;
    }
    void Deinitialize() {}
    napi_value LoadJsBundle(const std::string& path, const std::string& hapPath, bool useCommonChunk = false)
    {
        return nullptr;
    }
    napi_value LoadJsModule(const std::string& path, const std::string& hapPath)
    {
        return nullptr;
    }
    bool LoadScript(const std::string& path, std::vector<uint8_t>* buffer = nullptr, bool isBundle = false)
    {
        return true;
    }
    void RegisterQuickFixQueryFunc(const std::map<std::string, std::string>& moduleAndPath) override
    {
        return;
    }
    void SetDeviceDisconnectCallback(const std::function<bool()> &cb) override
    {
        return;
    }

    void StartProfiler(const DebugOption debugOption) override {}
    void SetExtensionApiCheckCallback(
        std::function<bool(const std::string &className, const std::string &fileName)> &cb) override {}
    void DumpHeapSnapshot(uint32_t tid, bool isFullGC, bool isBinary = false) override {}
    void DumpHeapSnapshot(uint32_t tid, const Runtime::JsHeapDumpParam &param) override {}
    void ForceFullGC(uint32_t tid) override {}
public:
    Language language;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // MOCK_RUNTIME_H
