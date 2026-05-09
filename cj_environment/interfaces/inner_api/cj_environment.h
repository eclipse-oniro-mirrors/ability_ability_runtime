/*
* Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_CJ_ENVIRONMENT_H
#define OHOS_ABILITY_RUNTIME_CJ_ENVIRONMENT_H

#include "cj_envsetup.h"
#include "cj_invoker.h"

#include <string>
#include <functional>

#ifdef WINDOWS_PLATFORM
#define CJ_EXPORT __declspec(dllexport)
#else
#define CJ_EXPORT __attribute__((visibility("default")))
#endif

namespace OHOS {
struct CJRuntimeAPI;

using TaskFuncType = void(*)();

class CJ_EXPORT CJEnvironment final {
public:
    static CJEnvironment* GetInstance();
    static void InitSpawnEnv();
    static void SetAppPath(const std::string& paths);
    static CJEnvMethods* CreateEnvMethods();

    enum class NSMode {
        SINK,
        APP,
    };

    CJEnvironment(NSMode mode);
    ~CJEnvironment();

    bool IsRuntimeStarted()
    {
        return isRuntimeStarted_;
    }

    static void SetSanitizerKindRuntimeVersion(SanitizerKind kind);

    bool StartRuntime();
    void StopRuntime();
    void RegisterArkVMInRuntime(unsigned long long externalVM);
    void RegisterStackInfoCallbacks(UpdateStackInfoFuncType uFunc);
    void RegisterCJUncaughtExceptionHandler(const CJUncaughtExceptionInfo& handle);
    bool RegisterCangjieCallback();
    void RegisterEventHandlerCallbacks();
    void RegisterEventHandler(const CJEventReportInfo& handle);
    int InitCJRuntime();
    bool IsUISchedulerStarted()
    {
        return isUISchedulerStarted_;
    }
    bool StartUIScheduler();
    bool CheckLoadCJLibrary();
    void StopUIScheduler();
    enum LibraryKind {
        SYSTEM,
        SDK,
        APP,
    };
    void* LoadCJLibrary(const char* dlName);
    void* LoadCJLibrary(LibraryKind kind, const char* dlName);
    void UnLoadCJLibrary(void* handle);
    void* GetUIScheduler()
    {
        if (!isUISchedulerStarted_) {
            return nullptr;
        }
        return uiScheduler_;
    }
    void* GetSymbol(void* dso, const char* symbol);
    bool StartDebugger();
    bool PostTask(TaskFuncType task);
    bool HasHigherPriorityTask();
    void SetLazyApis(CJRuntimeAPI* apis) { lazyApis_ = apis; }

    void InitCJAppNS(const std::string& path);
    void InitCJAppSDKNS(const std::string& path);
    void InitCJRomSDKNS(const std::string& path);
    void InitCJCompatibilitySDKNS(const std::string& path);
    void InitCJChipSDKNS(const std::string& path);
    void InitCJRuntimeNS(const std::string& path);
    void InitCJMockNS(const std::string& path);
    void InitRuntimeNS();
    void* InitUIScheduler();
    int FiniCJRuntime();
    int InitCJLibrary(const char* dlName);
    void InitCJNS(const std::string& path);
    static NSMode DetectAppNSMode();
    static void SetAppVersion(std::string& version);
    void DumpHeapSnapshot(int fd);
    void ForceFullGC();
    void* LoadRuntimeLib(const char* runtimeLibName);
    void UnLoadRuntimeApis();
    static void DumpCjHeap(int fd);
    static void GC();
    bool LoadSymbols(void* handle, CJRuntimeAPI& apis);

    static const char *cjChipSDKNSName;
    static const char *cjAppNSName;
    static const char *cjRomSDKNSName;
    static const char *cjSysNSName;
    static const char *cjCompatibilitySDKNSName;
    static const char *cjRuntimeNSName;
    static const char *cjMockNSName;
    static const char *cjAppSDKNSName;
    static std::string appVersion;
    static const std::string checkVersion;
    static SanitizerKind sanitizerKind;

private:
    bool LoadRuntimeApis();
    bool isRuntimeApiLoaded {false};
    CJRuntimeAPI* lazyApis_ {nullptr};
    bool isRuntimeStarted_{false};
    bool isLoadCJLibrary_{false};
    bool isUISchedulerStarted_{false};
    void* uiScheduler_ {nullptr};
    NSMode nsMode_ {NSMode::SINK};
};

}

#endif //OHOS_ABILITY_RUNTIME_CJ_ENVIRONMENT_H
