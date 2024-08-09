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

#ifndef MOCK_RUNTIME_H
#define MOCK_RUNTIME_H

#include <gtest/gtest.h>

#include "cj_runtime.h"

namespace OHOS {
namespace AbilityRuntime {
class cjMockRuntime : public CJRuntime {
public:
    cjMockRuntime() = default;
    ~cjMockRuntime() = default;

    Language GetLanguage() const override
    {
        return Language::CJ;
    }

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
    bool RunScript(const std::string& path, const std::string& hapPath, bool useCommonChunk = false)
    {
        return true;
    }
    bool Initialize(const Options& options)
    {
        return true;
    }
    void Deinitialize() {}
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

    void DoCleanWorkAfterStageCleaned() override {}
public:
    Language language;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // MOCK_RUNTIME_H
