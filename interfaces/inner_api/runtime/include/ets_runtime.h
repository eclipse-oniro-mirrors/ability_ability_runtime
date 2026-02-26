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

#ifndef OHOS_ABILITY_RUNTIME_ETS_RUNTIME_H
#define OHOS_ABILITY_RUNTIME_ETS_RUNTIME_H

#include <unordered_map>
#include <map>
#include <string>
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

#include "runtime.h"
#include "js_runtime.h"

using AppLibPathMap = std::map<std::string, std::vector<std::string>>;
using AppLibPathVec = std::vector<std::string>;
struct __ani_env;
using ani_env = __ani_env;

namespace OHOS {
namespace EtsEnv {
class ETSEnvironment;
struct ETSUncaughtExceptionInfo;
}

namespace AppExecFwk {
struct ETSNativeReference;
}

namespace AbilityRuntime {
class ETSRuntime : public Runtime {
public:
    static std::unique_ptr<ETSRuntime> Create(const Options &options,
        std::unique_ptr<Runtime> &jsRuntime, bool isMove = true);
    static void SetAppLibPath(const AppLibPathMap& appLibPaths,
        const std::map<std::string, std::string>& abcPathsToBundleModuleNameMap, bool isSystemApp);
    ~ETSRuntime() override;
    Language GetLanguage() const override
    {
        return Language::ETS;
    }

    void StartDebugMode(const DebugOption debugOption) override;
    void DumpHeapSnapshot(bool isPrivate) override {}
    void NotifyApplicationState(bool isBackground) override {}
    bool SuspendVM(uint32_t tid) override { return false; }
    void ResumeVM(uint32_t tid) override {}
    void PreloadSystemModule(const std::string &moduleName) override;
    void PreloadMainAbility(const std::string &moduleName, const std::string &srcPath, const std::string &hapPath,
        bool isEsMode, const std::string &srcEntrance) override {}
    void PreloadModule(const std::string &moduleName, const std::string &srcPath, const std::string &hapPath,
        bool isEsMode, bool useCommonTrunk) override {}
    void PreloadModule(
        const std::string &moduleName, const std::string &hapPath, bool isEsMode, bool useCommonTrunk) override;
    void FinishPreload() override;
    bool LoadRepairPatch(const std::string &patchFile, const std::string &baseFile) override { return false; }
    bool NotifyHotReloadPage() override { return false; }
    bool UnLoadRepairPatch(const std::string &patchFile) override { return false; }
    void RegisterQuickFixQueryFunc(const std::map<std::string, std::string> &moduleAndPath) override {};
    void StartProfiler(const DebugOption debugOption) override {};
    void SetExtensionApiCheckCallback(
        std::function<bool(const std::string &className, const std::string &fileName)> &cb) override;
    void SetModuleLoadChecker(const std::shared_ptr<ModuleCheckerDelegate> moduleCheckerDelegate) const override;
    void SetDeviceDisconnectCallback(const std::function<bool()> &cb) override {};
    void DestroyHeapProfiler() override {};
    void ForceFullGC() override {};
    void ForceFullGC(uint32_t tid) override {};
    void DumpHeapSnapshot(uint32_t tid, bool isFullGC, bool isBinary = false) override {};
    void DumpCpuProfile() override {};
    void AllowCrossThreadExecution() override {};
    void GetHeapPrepare() override {};
    void RegisterUncaughtExceptionHandler(const EtsEnv::ETSUncaughtExceptionInfo &uncaughtExceptionInfo);
    ani_env *GetAniEnv() const;
    std::unique_ptr<AppExecFwk::ETSNativeReference> LoadModule(const std::string &moduleName,
        const std::string &modulePath, const std::string &hapPath, bool esmodule,
        bool useCommonChunk, const std::string &srcEntrance);
    bool HandleUncaughtError();
    const std::unique_ptr<AbilityRuntime::Runtime> &GetJsRuntime() const;
    std::unique_ptr<AbilityRuntime::Runtime> MoveJsRuntime();
    static std::unique_ptr<ETSRuntime> PreFork(const Options &options,
        std::unique_ptr<Runtime> &jsRuntime, bool isMove = true);
    bool PreloadSystemClass(const char *className) override;
    void DebuggerConnectionHandler(bool isDebugApp, bool isStartWithDebug);
    void StopDebugMode();
    uint32_t instanceId_ = 0;
    void SetJsRuntime(std::unique_ptr<AbilityRuntime::Runtime> &jsRuntime);

private:
    bool Initialize(const Options &options, std::unique_ptr<Runtime> &jsRuntime, bool isMove = true);
    void Deinitialize();
    bool CreateEtsEnv(const Options &options);
    std::unique_ptr<AppExecFwk::ETSNativeReference> LoadEtsModule(const std::string &moduleName,
        const std::string &fileName, const std::string &hapPath, const std::string &srcEntrance);
    bool PostFork(const Options &options, std::unique_ptr<Runtime> &jsRuntime, bool isMove = true);
    std::string GetAotPath(const Options &options);
    std::string HandleOhmUrlSrcEntry(const std::string &srcEntry);
    void HandleOhmUrlFileName(std::string &fileName);
    static void PreloadLibrary();
    int32_t apiTargetVersion_ = 0;
    std::string codePath_;
    std::string moduleName_;
    std::unique_ptr<AbilityRuntime::Runtime> jsRuntime_ = nullptr;
    bool debugMode_ = false;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ETS_RUNTIME_H
