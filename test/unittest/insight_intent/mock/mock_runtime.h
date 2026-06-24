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

#ifndef MOCK_RUNTIME_H
#define MOCK_RUNTIME_H

#include "mock_my_flag.h"
#include "runtime.h"

namespace OHOS {
namespace AbilityRuntime {
class MockRuntime : public Runtime {
public:
    MockRuntime() {}
    ~MockRuntime() {}

    Language GetLanguage() const override
    {
        return language_;
    }

    void StartDebugMode(const DebugOption debugOption) override {}
    void SetDebugOption(const DebugOption debugOption) override {}
    void StartLocalDebugMode(bool isDebugFromLocal) override {}
    void DumpHeapSnapshot(bool isPrivate) override {}
    void DumpCpuProfile() override {}
    void DestroyHeapProfiler() override {}
    void ForceFullGC() override {}
    void ForceFullGC(uint32_t tid) override {}
    void DumpHeapSnapshot(uint32_t tid, bool isFullGC, bool isBinary = false) override {}
    void DumpHeapSnapshot(uint32_t tid, const Runtime::JsHeapDumpParam &param) override {}
    void AllowCrossThreadExecution() override {}
    void GetHeapPrepare() override {}
    void NotifyApplicationState(bool isBackground) override {}
    bool SuspendVM(uint32_t tid) override { return false; }
    void ResumeVM(uint32_t tid) override {}
    void PreloadSystemModule(const std::string& moduleName) override {}
    void PreloadMainAbility(const std::string& moduleName, const std::string& srcPath,
        const std::string& hapPath,  bool isEsMode, const std::string& srcEntrance) override {}
    void PreloadModule(const std::string& moduleName, const std::string& srcPath,
        const std::string& hapPath, bool isEsMode, bool useCommonTrunk) override {}
    void FinishPreload() override {}
    bool LoadRepairPatch(const std::string& patchFile, const std::string& baseFile) override { return false; }
    bool NotifyHotReloadPage() override { return false; }
    bool UnLoadRepairPatch(const std::string& patchFile) override { return false; }
    void RegisterQuickFixQueryFunc(const std::map<std::string, std::string>& moduleAndPath) override {}
    void StartProfiler(const DebugOption debugOption) override {}
    void SetExtensionApiCheckCallback(
        std::function<bool(const std::string &className, const std::string &fileName)> &cb) override {}
    void SetDeviceDisconnectCallback(const std::function<bool()> &cb) override {}
    void SetLanguage(const Runtime::Language& language) { language_ = language; }
private:
    Runtime::Language language_ = Runtime::Language::JS;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif // MOCK_RUNTIME_H