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

#ifndef OHOS_ABILITY_RUNTIME_CJ_RUNTIME_H
#define OHOS_ABILITY_RUNTIME_CJ_RUNTIME_H

#include <functional>
#include <unordered_map>
#include <map>
#include <string>

#include "runtime.h"
#include "cj_envsetup.h"

using AppLibPathMap = std::map<std::string, std::vector<std::string>>;
using AppLibPathVec = std::vector<std::string>;

namespace OHOS {
struct CJUncaughtExceptionInfo;
namespace AbilityRuntime {

class CJRuntime : public Runtime {
public:
    static std::unique_ptr<CJRuntime> Create(const Options& options);
    static void SetAppLibPath(const AppLibPathMap& appLibPaths);
    static bool IsCJAbility(const std::string& info);
    static void SetAppVersion(std::string& version);
    static void SetSanitizerVersion(SanitizerKind kind);
    static bool RegisterCangjieCallback();
    static void SetPackageName(std::string srcEntryName);
    ~CJRuntime() override = default;

    Language GetLanguage() const override
    {
        return Language::CJ;
    }

    void StartDebugMode(const DebugOption debugOption) override;
    void DumpHeapSnapshot(bool isPrivate) override {}
    void NotifyApplicationState(bool isBackground) override {}
    bool SuspendVM(uint32_t tid) override { return false; }
    void ResumeVM(uint32_t tid) override {}
    void PreloadSystemModule(const std::string& moduleName) override {}
    void PreloadMainAbility(const std::string& moduleName, const std::string& srcPath,
        const std::string& hapPath,  bool isEsMode, const std::string& srcEntrance) override {}
    void PreloadModule(const std::string& moduleName, const std::string& srcPath,
        const std::string& hapPath, bool isEsMode, bool useCommonTrunk) override {}
    void PreloadModule(const std::string& moduleName, const std::string& hapPath,
        bool isEsMode, bool useCommonTrunk) override {}
    void FinishPreload() override {}
    bool LoadRepairPatch(const std::string& patchFile, const std::string& baseFile) override { return false; }
    bool NotifyHotReloadPage() override { return false; }
    bool UnLoadRepairPatch(const std::string& patchFile) override { return false; }
    void RegisterQuickFixQueryFunc(const std::map<std::string, std::string>& moduleAndPath) override {};
    void StartProfiler(const DebugOption dOption) override;
    void SetExtensionApiCheckCallback(
        std::function<bool(const std::string &className, const std::string &fileName)> &cb) override {}
    void SetModuleLoadChecker(const std::shared_ptr<ModuleCheckerDelegate> moduleCheckerDelegate) const override {}
    void SetDeviceDisconnectCallback(const std::function<bool()> &cb) override {};
    bool IsAppLibLoaded() const { return appLibLoaded_; }
    void UnLoadCJAppLibrary();
    void DestroyHeapProfiler() override {};
    void ForceFullGC() override {};
    void ForceFullGC(uint32_t tid) override;
    void DumpHeapSnapshot(uint32_t tid, bool isFullGC, bool isBinary = false) override;
    void DumpCpuProfile() override {};
    void AllowCrossThreadExecution() override {};
    void GetHeapPrepare() override {};
    void RegisterUncaughtExceptionHandler(const CJUncaughtExceptionInfo& uncaughtExceptionInfo);

private:
    bool StartDebugger();
    bool LoadCJAppLibrary(const AppLibPathVec& appLibPaths);
    bool Initialize(const Options& options);
    std::string cjAppLibPath_;
    bool appLibLoaded_ = false;
    bool debugModel_ = false;
    std::string bundleName_;
    uint32_t instanceId_ = 0;
    static AppLibPathVec appLibPaths_;
    static std::string packageName_;
};
} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CJ_RUNTIME_H
