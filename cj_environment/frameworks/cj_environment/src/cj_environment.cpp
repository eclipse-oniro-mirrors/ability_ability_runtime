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

#include "cj_environment.h"

#include <unistd.h>
#include <string>
#include <sstream>
#include <mutex>
#include <charconv>
#include "cj_hilog.h"
#include "cj_invoker.h"
#include "cj_envsetup.h"
#ifdef __OHOS__
#include <dlfcn.h>
#endif
#ifdef __APPLE__
#include <cstdlib>
#include <filesystem>
#endif
#include "dynamic_loader.h"
#ifdef WITH_EVENT_HANDLER
#include "event_handler.h"
#endif

#ifdef APP_USE_ARM64
#define APP_LIB_NAME "arm64"
#elif defined(APP_USE_ARM)
#define APP_LIB_NAME "arm"
#elif defined(APP_USE_X86_64)
#define APP_LIB_NAME "x86_64"
#elif defined(NAPI_TARGET_ARM64)
#define APP_LIB_NAME "arm64"
#else
#error unsupported platform
#endif

namespace {
const std::string SANDBOX_LIB_PATH = "/data/storage/el1/bundle/libs/" APP_LIB_NAME;
const std::string CJ_COMPATIBILITY_PATH = SANDBOX_LIB_PATH + "/runtime";
const std::string CJ_MOCK_PATH = SANDBOX_LIB_PATH + "/ohos";
#ifdef APP_USE_ARM
const std::string CJ_CHIPSDK_PATH = "/system/lib/chipset-sdk-sp:/system/lib/chipset-sdk";
const std::string CJ_SDK_PATH = "/system/lib/platformsdk/cjsdk";
const std::string CJ_RUNTIME_PATH = "/system/lib/platformsdk/cjsdk/runtime";
#else
const std::string CJ_CHIPSDK_PATH = "/system/lib64/chipset-sdk-sp:/system/lib64/chipset-sdk";
const std::string CJ_SDK_PATH = "/system/lib64/platformsdk/cjsdk";
const std::string CJ_RUNTIME_PATH = "/system/lib64/platformsdk/cjsdk/runtime";
#endif
const std::string CJ_ASAN_PATH = SANDBOX_LIB_PATH + "/asan";
} // namespace

namespace OHOS {
namespace {
const char DEBUGGER_LIBNAME[] = "libcj_debugger.z.so";
const char DEBUGGER_SYMBOL_NAME[] = "StartDebuggerServer";
const char INIT_CJRUNTIME_SYMBOL_NAME[] = "InitCJRuntime";
const char INIT_UISCHEDULER_SYMBOL_NAME[] = "InitUIScheduler";
const char RUN_UISCHEDULER_SYMBOL_NAME[] = "RunUIScheduler";
const char FINI_CJRUNTIME_SYMBOL_NAME[] = "FiniCJRuntime";
const char INIT_CJLIBRARY_SYMBOL_NAME[] = "InitCJLibrary";
const char REGISTER_EVENTHANDLER_CALLBACKS_NAME[] = "RegisterEventHandlerCallbacks";
const char REGISTER_ARKVM_SYMBOL_NAME[] = "RegisterArkVMInRuntime";
const char REGISTER_STACKINFO_CALLBACKS_NAME[] = "RegisterStackInfoCallbacks";
const char DUMP_HEAP_SNAPSHOT_NAME[] = "CJ_MRT_DumpHeapSnapshot";
const char FORCE_FULL_GC_NAME[] = "CJ_MRT_ForceFullGC";
const char REGISTER_EVENT_HANDLER_NAME[] = "RegisterEventHandler";

using InitCJRuntimeType = int(*)(const struct RuntimeParam*);;
using InitUISchedulerType = void*(*)();
using RunUISchedulerType = int(*)(unsigned long long);
using FiniCJRuntimeType = int(*)();
using InitCJLibraryType = int(*)(const char*);
using RegisterEventHandlerType = void(*)(PostTaskType, HasHigherPriorityType);
using RegisterArkVMType = void(*)(unsigned long long);
using RegisterStackInfoType = void(*)(UpdateStackInfoFuncType);
using DumpHeapSnapshotType = void(*)(int);
using ForceFullGCType = void(*)();
using RegisterEventReportType = void(*)(const CJEventReportInfo& handle);

#ifdef __OHOS__
const char REGISTER_UNCAUGHT_EXCEPTION_NAME[] = "RegisterUncaughtExceptionHandler";
using RegisterUncaughtExceptionType = void (*)(const CJUncaughtExceptionInfo& handle);
#endif

#ifdef WITH_EVENT_HANDLER
static std::shared_ptr<AppExecFwk::EventHandler> g_handler = nullptr;
#endif

#ifdef __APPLE__
std::string getLibPath(const char* dlName) {
    if (!dlName) {
        LOGE("dlName is nullptr");
        return std::string("");
    }

    const char* value = getenv("CJENV_LIBRARY_PATH");
    if (!value) {
        return std::string(dlName);
    }
    std::string paths(value);

    size_t start = 0;
    size_t end = paths.size();
    do {
        end = paths.find(':', start);
        std::string str = paths.substr(start, end - start);
        start = end + 1;
        if (!str.empty()) {
            auto path = std::filesystem::path(str) / dlName;
            if (std::filesystem::exists(path)) {
                return path;
            }
        }
    }while (end != std::string::npos);

    return std::string(dlName);
}
#endif

bool LoadSymbolInitCJRuntime(void* handle, CJRuntimeAPI& apis)
{
    auto symbol = DynamicFindSymbol(handle, INIT_CJRUNTIME_SYMBOL_NAME);
    if (symbol == nullptr) {
        LOGE("runtime api not found: %{public}s", INIT_CJRUNTIME_SYMBOL_NAME);
        return false;
    }
    apis.InitCJRuntime = reinterpret_cast<InitCJRuntimeType>(symbol);
    return true;
}

bool LoadSymbolInitUIScheduler(void* handle, CJRuntimeAPI& apis)
{
    auto symbol = DynamicFindSymbol(handle, INIT_UISCHEDULER_SYMBOL_NAME);
    if (symbol == nullptr) {
        LOGE("runtime api not found: %{public}s", INIT_UISCHEDULER_SYMBOL_NAME);
        return false;
    }
    apis.InitUIScheduler = reinterpret_cast<InitUISchedulerType>(symbol);
    return true;
}

bool LoadSymbolRunUIScheduler(void* handle, CJRuntimeAPI& apis)
{
    auto symbol = DynamicFindSymbol(handle, RUN_UISCHEDULER_SYMBOL_NAME);
    if (symbol == nullptr) {
        LOGE("runtime api not found: %{public}s", RUN_UISCHEDULER_SYMBOL_NAME);
        return false;
    }
    apis.RunUIScheduler = reinterpret_cast<RunUISchedulerType>(symbol);
    return true;
}

bool LoadSymbolFiniCJRuntime(void* handle, CJRuntimeAPI& apis)
{
    auto symbol = DynamicFindSymbol(handle, FINI_CJRUNTIME_SYMBOL_NAME);
    if (symbol == nullptr) {
        LOGE("runtime api not found: %{public}s", FINI_CJRUNTIME_SYMBOL_NAME);
        return false;
    }
    apis.FiniCJRuntime = reinterpret_cast<FiniCJRuntimeType>(symbol);
    return true;
}

bool LoadSymbolInitCJLibrary(void* handle, CJRuntimeAPI& apis)
{
    auto symbol = DynamicFindSymbol(handle, INIT_CJLIBRARY_SYMBOL_NAME);
    if (symbol == nullptr) {
        LOGE("runtime api not found: %{public}s", INIT_CJLIBRARY_SYMBOL_NAME);
        return false;
    }
    apis.InitCJLibrary = reinterpret_cast<InitCJLibraryType>(symbol);
    return true;
}

bool LoadSymbolRegisterEventHandler(void* handle, CJRuntimeAPI& apis)
{
    auto symbol = DynamicFindSymbol(handle, REGISTER_EVENT_HANDLER_NAME);
    if (symbol == nullptr) {
        LOGE("runtime api not found: %{public}s", REGISTER_EVENT_HANDLER_NAME);
        return true;
    }
    apis.RegisterEventHandler = reinterpret_cast<RegisterEventReportType>(symbol);
    return true;
}

bool LoadSymbolRegisterEventHandlerCallbacks(void* handle, CJRuntimeAPI& apis)
{
    auto symbol = DynamicFindSymbol(handle, REGISTER_EVENTHANDLER_CALLBACKS_NAME);
    if (symbol == nullptr) {
        LOGE("runtime api not found: %{public}s", REGISTER_EVENTHANDLER_CALLBACKS_NAME);
        return false;
    }
    apis.RegisterEventHandlerCallbacks = reinterpret_cast<RegisterEventHandlerType>(symbol);
    return true;
}

bool LoadSymbolRegisterStackInfoCallbacks(void* handle, CJRuntimeAPI& apis)
{
    auto symbol = DynamicFindSymbol(handle, REGISTER_STACKINFO_CALLBACKS_NAME);
    if (symbol == nullptr) {
        LOGE("runtime api not found: %{public}s", REGISTER_STACKINFO_CALLBACKS_NAME);
        // return true for compatible.
        apis.RegisterStackInfoCallbacks = nullptr;
        return true;
    }
    apis.RegisterStackInfoCallbacks = reinterpret_cast<RegisterStackInfoType>(symbol);
    return true;
}

bool LoadSymbolRegisterArkVM(void* handle, CJRuntimeAPI& apis)
{
    auto symbol = DynamicFindSymbol(handle, REGISTER_ARKVM_SYMBOL_NAME);
    if (symbol == nullptr) {
        LOGE("runtime api not found: %{public}s", REGISTER_ARKVM_SYMBOL_NAME);
        // return true for compatible.
        apis.RegisterArkVMInRuntime = nullptr;
        return true;
    }
    apis.RegisterArkVMInRuntime = reinterpret_cast<RegisterArkVMType>(symbol);
    return true;
}

#ifdef __OHOS__
bool LoadSymbolRegisterCJUncaughtExceptionHandler(void* handle, CJRuntimeAPI& apis)
{
    auto symbol = DynamicFindSymbol(handle, REGISTER_UNCAUGHT_EXCEPTION_NAME);
    if (symbol == nullptr) {
        LOGE("runtime api not found: %{public}s", REGISTER_UNCAUGHT_EXCEPTION_NAME);
        return false;
    }
    apis.RegisterCJUncaughtExceptionHandler = reinterpret_cast<RegisterUncaughtExceptionType>(symbol);
    return true;
}
#endif

bool LoadSymbolDumpHeapSnapshot(void* handle, CJRuntimeAPI& apis)
{
    auto symbol = DynamicFindSymbol(handle, DUMP_HEAP_SNAPSHOT_NAME);
    if (symbol == nullptr) {
        LOGE("runtime api not found: %{public}s", DUMP_HEAP_SNAPSHOT_NAME);
        // return true for compatible.
        apis.DumpHeapSnapshot = nullptr;
        return true;
    }
    apis.DumpHeapSnapshot = reinterpret_cast<DumpHeapSnapshotType>(symbol);
    return true;
}

bool LoadSymbolForceFullGC(void* handle, CJRuntimeAPI& apis)
{
    auto symbol = DynamicFindSymbol(handle, FORCE_FULL_GC_NAME);
    if (symbol == nullptr) {
        LOGE("runtime api not found: %{public}s", FORCE_FULL_GC_NAME);
        // return true for compatible.
        apis.ForceFullGC = nullptr;
        return true;
    }
    apis.ForceFullGC = reinterpret_cast<ForceFullGCType>(symbol);
    return true;
}

bool PostTaskWrapper(void* func)
{
    return CJEnvironment::GetInstance()->PostTask(reinterpret_cast<TaskFuncType>(func));
}

bool HasHigherPriorityTaskWrapper()
{
    return CJEnvironment::GetInstance()->HasHigherPriorityTask();
}

CJEnvironment* instance_ = nullptr;
} // namespace

const char *CJEnvironment::cjChipSDKNSName = "cj_chipsdk";
const char *CJEnvironment::cjAppNSName = "moduleNs_default";
const char *CJEnvironment::cjRomSDKNSName = "cj_rom_sdk";
const char *CJEnvironment::cjSysNSName = "default";
const char *CJEnvironment::cjCompatibilitySDKNSName = "cj_compatibility_sdk";
const char *CJEnvironment::cjRuntimeNSName = "cj_runtime";
const char *CJEnvironment::cjMockNSName = "cj_mock_sdk";
const char *CJEnvironment::cjAppSDKNSName = "cj_app_sdk";
// use app sdk when version is less than 5.1.1.0
const std::string CJEnvironment::checkVersion = "5.1.1.0";
std::string CJEnvironment::appVersion = CJEnvironment::checkVersion;
SanitizerKind CJEnvironment::sanitizerKind = SanitizerKind::NONE;

#ifdef WITH_EVENT_HANDLER
static std::shared_ptr<AppExecFwk::EventHandler>GetGHandler()
{
    if (g_handler == nullptr) {
        g_handler = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());
    }
    return g_handler;
}
#endif

void CJEnvironment::InitSpawnEnv()
{
#ifdef WITH_EVENT_HANDLER
    GetGHandler();
#endif
}

void CJEnvironment::SetAppPath(const std::string& appPath)
{
    static bool isInited = false;
    static std::mutex initMutex;
    std::lock_guard<std::mutex> lock(initMutex);
    if (isInited) {
        return;
    }
    auto mode = DetectAppNSMode();
    if (instance_) {
        if (instance_->nsMode_ == mode) {
            instance_->InitCJNS(appPath);
            return;
        }
        delete instance_;
    }
    instance_ = new CJEnvironment(mode);
    instance_->InitCJNS(appPath);
    isInited = true;
}

CJEnvironment::CJEnvironment(NSMode mode) : nsMode_(mode)
{
    InitRuntimeNS();
    LoadRuntimeApis();
}

CJEnvironment::~CJEnvironment()
{
    StopRuntime();
    UnLoadRuntimeApis();
}

CJEnvironment* CJEnvironment::GetInstance()
{
    return instance_;
}

#ifdef __OHOS__
bool CJEnvironment::RegisterCangjieCallback()
{
    constexpr char CANGJIE_DEBUGGER_LIB_PATH[] = "libark_connect_inspector.z.so";
    auto handlerConnectServerSo = LoadCJLibrary(CJEnvironment::SYSTEM, CANGJIE_DEBUGGER_LIB_PATH);
    if (handlerConnectServerSo == nullptr) {
        LOGE("null handlerConnectServerSo: %{public}s", dlerror());
        return false;
    }
    using SendMsgCB = const std::function<void(const std::string& message)>;
    using SetCangjieCallback = void(*)(const std::function<void(const std::string& message, SendMsgCB)>);
    using CangjieCallback = void(*)(const std::string& message, SendMsgCB);
    auto setCangjieCallback = reinterpret_cast<SetCangjieCallback>(
        DynamicFindSymbol(handlerConnectServerSo, "SetCangjieCallback"));
    if (setCangjieCallback == nullptr) {
        LOGE("null setCangjieCallback: %{public}s", dlerror());
        return false;
    }
    #define RTLIB_NAME "libcangjie-runtime.so"
    auto dso = LoadCJLibrary(CJEnvironment::SDK, RTLIB_NAME);
    if (!dso) {
        LOGE("load library failed: %{public}s", RTLIB_NAME);
        return false;
    }
    LOGE("load libcangjie-runtime.so success");
    #define PROFILERAGENT "ProfilerAgent"
    CangjieCallback cangjieCallback = reinterpret_cast<CangjieCallback>(DynamicFindSymbol(dso, PROFILERAGENT));
    if (cangjieCallback == nullptr) {
        dlclose(handlerConnectServerSo);
        handlerConnectServerSo = nullptr;
        LOGE("runtime api not found: %{public}s", PROFILERAGENT);
        return false;
    }
    LOGE("find runtime api success");
    setCangjieCallback(cangjieCallback);
    dlclose(handlerConnectServerSo);
    handlerConnectServerSo = nullptr;
    return true;
}

void* CJEnvironment::LoadRuntimeLib(const char* runtimeLibName) {
    Dl_namespace sdk;
    dlns_get(nsMode_ == NSMode::APP ? cjAppSDKNSName : cjCompatibilitySDKNSName, &sdk);
    auto dso = DynamicLoadLibrary(&sdk, runtimeLibName, 1);
    return dso;
}
#endif
bool CJEnvironment::LoadRuntimeApis()
{
    if (isRuntimeApiLoaded) {
        return true;
    }
    lazyApis_ = new CJRuntimeAPI();
#ifdef __WINDOWS__
#define RTLIB_NAME "libcangjie-runtime.dll"
#elif defined(__APPLE__)
#define RTLIB_NAME "libcangjie-runtime.dylib"
#else
#define RTLIB_NAME "libcangjie-runtime.so"
#endif

#ifdef __OHOS__
    auto dso = LoadRuntimeLib(RTLIB_NAME);
#else
#ifdef __APPLE__
    auto dso = DynamicLoadLibrary(getLibPath(RTLIB_NAME).c_str(), 1);
#else
    auto dso = DynamicLoadLibrary(RTLIB_NAME, 1);
#endif
#endif
    if (!dso) {
        LOGE("load library failed: %{public}s", RTLIB_NAME);
        return false;
    }
#undef RTLIB_NAME
    if (LoadSymbols(dso, *lazyApis_)) {
        LOGE("load symbol failed");
        DynamicFreeLibrary(dso);
        return false;
    }
#ifdef __OHOS__
    if (!LoadSymbolRegisterCJUncaughtExceptionHandler(dso, *lazyApis_)) {
        LOGE("load symbol RegisterCJUncaughtExceptionHandler failed");
        DynamicFreeLibrary(dso);
        return false;
    }
#endif
    isRuntimeApiLoaded = true;
    return true;
}

bool CJEnvironment::LoadSymbols(void* dso, CJRuntimeAPI& lazyApis_)
{
    return (!LoadSymbolInitCJRuntime(dso, lazyApis_) ||
            !LoadSymbolInitUIScheduler(dso, lazyApis_) ||
            !LoadSymbolRunUIScheduler(dso, lazyApis_) ||
            !LoadSymbolFiniCJRuntime(dso, lazyApis_) ||
            !LoadSymbolInitCJLibrary(dso, lazyApis_) ||
            !LoadSymbolRegisterEventHandlerCallbacks(dso, lazyApis_) ||
            !LoadSymbolRegisterStackInfoCallbacks(dso, lazyApis_) ||
            !LoadSymbolRegisterArkVM(dso, lazyApis_) ||
            !LoadSymbolDumpHeapSnapshot(dso, lazyApis_) ||
            !LoadSymbolForceFullGC(dso, lazyApis_) ||
            !LoadSymbolRegisterEventHandler(dso, lazyApis_));
}

void CJEnvironment::RegisterArkVMInRuntime(unsigned long long externalVM)
{
    if (!lazyApis_ || !lazyApis_->RegisterArkVMInRuntime) {
        return;
    }
    lazyApis_->RegisterArkVMInRuntime(externalVM);
}

void CJEnvironment::RegisterStackInfoCallbacks(UpdateStackInfoFuncType uFunc)
{
    if (!lazyApis_ || !lazyApis_->RegisterStackInfoCallbacks) {
        return;
    }
    lazyApis_->RegisterStackInfoCallbacks(uFunc);
}

void CJEnvironment::RegisterCJUncaughtExceptionHandler(const CJUncaughtExceptionInfo& handle)
{
    if (!lazyApis_ || !lazyApis_->RegisterCJUncaughtExceptionHandler) {
        return;
    }
    lazyApis_->RegisterCJUncaughtExceptionHandler(handle);
}

void CJEnvironment::RegisterEventHandlerCallbacks()
{
    if (!lazyApis_ || !lazyApis_->RegisterEventHandlerCallbacks) {
        return;
    }
    lazyApis_->RegisterEventHandlerCallbacks(PostTaskWrapper, HasHigherPriorityTaskWrapper);
}

void CJEnvironment::RegisterEventHandler(const CJEventReportInfo& handle)
{
    if (!lazyApis_ || !lazyApis_->RegisterEventHandler) {
        LOGE("api RegisterEventHandler is null.");
        return;
    }
    lazyApis_->RegisterEventHandler(handle);
}

void CJEnvironment::DumpCjHeap(int fd)
{
    auto instance = CJEnvironment::GetInstance();
    if (instance == nullptr) {
        LOGE("cjEnv instance is null.");
        return;
    }
    instance->DumpHeapSnapshot(fd);
}

void CJEnvironment::GC()
{
    auto instance = CJEnvironment::GetInstance();
    if (instance == nullptr) {
        LOGE("cjEnv instance is null.");
        return;
    }
    instance->ForceFullGC();
}

int CJEnvironment::InitCJRuntime()
{
    if (!lazyApis_ || !lazyApis_->InitCJRuntime) {
        return E_FAILED;
    }
    RuntimeParam rtParams {
        .heapParam = {
            .regionSize = 64,
            .heapSize = 256 * 1024,
            .exemptionThreshold= 0.8,
            .heapUtilization = 0.8,
            .heapGrowth = 0.15,
            .allocationRate = 0,
            .allocationWaitTime = 0,
        },
        .gcParam = {
            .gcThreshold = 0,
            .garbageThreshold = 0,
            .gcInterval = 0,
            .backupGCInterval = 0,
            .gcThreads = 0,
        },
        .logParam = {
            .logLevel = RTLOG_ERROR,
        },
        .coParam = {
            .thStackSize = 2 * 1024,
            .coStackSize = 2 * 1024,
            .processorNum = 8,
        }
    };
    return lazyApis_->InitCJRuntime(&rtParams);
}

int CJEnvironment::InitCJLibrary(const char* dlName)
{
    if (!lazyApis_ || !lazyApis_->InitCJLibrary || !dlName) {
        return E_FAILED;
    }
    return lazyApis_->InitCJLibrary(dlName);
}

int CJEnvironment::FiniCJRuntime()
{
    if (!lazyApis_ || !lazyApis_->FiniCJRuntime) {
        return E_FAILED;
    }
    return lazyApis_->FiniCJRuntime();
}

void* CJEnvironment::InitUIScheduler()
{
    if (!lazyApis_ || !lazyApis_->InitUIScheduler) {
        return nullptr;
    }
    return lazyApis_->InitUIScheduler();
}

void CJEnvironment::DumpHeapSnapshot(int fd)
{
    if (!lazyApis_ || !lazyApis_->DumpHeapSnapshot) {
        return;
    }
    lazyApis_->DumpHeapSnapshot(fd);
}

void CJEnvironment::ForceFullGC()
{
    if (!lazyApis_ || !lazyApis_->ForceFullGC) {
        return;
    }
    lazyApis_->ForceFullGC();
}

bool CJEnvironment::PostTask(TaskFuncType task)
{
#ifdef WITH_EVENT_HANDLER
    if (task == nullptr) {
        LOGE("null task could not be posted");
        return false;
    }

    bool postDone = GetGHandler()->PostTask(task, "spawn-main-task-from-cj", 0, AppExecFwk::EventQueue::Priority::HIGH);
    if (!postDone) {
        LOGE("event handler support cj ui scheduler");
        return false;
    }
    return true;
#endif
    return true;
}

bool CJEnvironment::HasHigherPriorityTask()
{
#ifdef WITH_EVENT_HANDLER
    return GetGHandler()->HasPreferEvent(static_cast<int>(AppExecFwk::EventQueue::Priority::HIGH));
#endif
    return false;
}

void CJEnvironment::InitCJChipSDKNS(const std::string& path)
{
#ifdef __OHOS__
    LOGI("InitCJChipSDKNS: %{public}s", path.c_str());
    Dl_namespace chip_sdk;
    DynamicInitNamespace(&chip_sdk, path.c_str(), CJEnvironment::cjChipSDKNSName);
#endif
}

// Init app namespace
void CJEnvironment::InitCJAppNS(const std::string& path)
{
#ifdef __OHOS__
    LOGI("InitCJAppNS: %{public}s", path.c_str());
    Dl_namespace ns;
    DynamicInitNamespace(&ns, path.c_str(), CJEnvironment::cjAppNSName);
    if (nsMode_ == NSMode::APP) {
        DynamicInherit(&ns, CJEnvironment::cjChipSDKNSName, "libssl_openssl.z.so");
        DynamicInherit(&ns, CJEnvironment::cjAppSDKNSName, "allow_all_shared_libs");
    } else {
        DynamicInherit(&ns, CJEnvironment::cjCompatibilitySDKNSName, "allow_all_shared_libs");
        DynamicInherit(&ns, CJEnvironment::cjMockNSName, "allow_all_shared_libs");
        DynamicInherit(&ns, CJEnvironment::cjRomSDKNSName, "allow_all_shared_libs");
        DynamicInherit(&ns, CJEnvironment::cjRuntimeNSName, "allow_all_shared_libs");
    }
#endif
}

void CJEnvironment::InitCJAppSDKNS(const std::string& path)
{
#ifdef __OHOS__
    LOGI("InitCJAppSDKNS: %{public}s", path.c_str());
    Dl_namespace ns;
    DynamicInitNamespace(&ns, path.c_str(), CJEnvironment::cjAppSDKNSName);
#endif
}

void CJEnvironment::InitCJRomSDKNS(const std::string& path)
{
#ifdef __OHOS__
    LOGI("InitCJRomSDKNS: %{public}s", path.c_str());
    Dl_namespace ns;
    DynamicInitNamespace(&ns, path.c_str(), CJEnvironment::cjRomSDKNSName);
    DynamicInherit(&ns, CJEnvironment::cjRuntimeNSName, "allow_all_shared_libs");
#endif
}

void CJEnvironment::InitCJCompatibilitySDKNS(const std::string& path)
{
#ifdef __OHOS__
    LOGI("InitCJCompatibilitySDKNS: %{public}s", path.c_str());
    Dl_namespace ns;
    DynamicInitNamespace(&ns, path.c_str(), CJEnvironment::cjCompatibilitySDKNSName);
    DynamicInherit(&ns, CJEnvironment::cjMockNSName, "allow_all_shared_libs");
    DynamicInherit(&ns, CJEnvironment::cjRomSDKNSName, "allow_all_shared_libs");
    DynamicInherit(&ns, CJEnvironment::cjRuntimeNSName, "allow_all_shared_libs");
    DynamicInheritByName(CJEnvironment::cjMockNSName,
                         CJEnvironment::cjCompatibilitySDKNSName, "allow_all_shared_libs");
#endif
}

void CJEnvironment::InitCJRuntimeNS(const std::string& path)
{
#ifdef __OHOS__
    LOGI("InitCJRuntimeNS: %{public}s", path.c_str());
    Dl_namespace ns;
    DynamicInitNamespace(&ns, path.c_str(), CJEnvironment::cjRuntimeNSName);
#endif
}

void CJEnvironment::InitCJMockNS(const std::string& path)
{
#ifdef __OHOS__
    LOGI("InitCJMockNS: %{public}s", path.c_str());
    Dl_namespace ns;
    DynamicInitNamespace(&ns, path.c_str(), CJEnvironment::cjMockNSName);
#endif
}

void CJEnvironment::UnLoadRuntimeApis()
{
    if (lazyApis_ != nullptr) {
        delete lazyApis_;
        lazyApis_ = nullptr;
    }
}

bool CJEnvironment::StartRuntime()
{
    if (isRuntimeStarted_) {
        return true;
    }
    if (!LoadRuntimeApis()) {
        LOGE("LoadRuntimeApis failed");
        UnLoadRuntimeApis();
        return false;
    }
    auto status = InitCJRuntime();
    if (status != E_OK) {
        LOGE("init cj runtime failed: %{public}d", status);
        UnLoadRuntimeApis();
        return false;
    }
    RegisterEventHandlerCallbacks();
    isRuntimeStarted_ = true;
    return true;
}

void CJEnvironment::StopRuntime()
{
    if (!isRuntimeStarted_) {
        return;
    }

    if (isUISchedulerStarted_) {
        StopUIScheduler();
    }

    auto code = FiniCJRuntime();
    if (code != E_OK) {
        LOGE("Failed to fini cj runtime.");
        return;
    }
    isRuntimeStarted_ = false;
}

bool CJEnvironment::StartUIScheduler()
{
    if (isUISchedulerStarted_) {
        return true;
    }

    uiScheduler_ = InitUIScheduler();
    if (!uiScheduler_) {
        LOGE("init cj ui scheduler failed");
        return false;
    }

    isUISchedulerStarted_ = true;
    return true;
}

void CJEnvironment::StopUIScheduler()
{
    isUISchedulerStarted_ = false;
}

void* CJEnvironment::LoadCJLibrary(const char* dlName)
{
    if (!StartRuntime()) {
        LOGE("StartRuntime failed");
        return nullptr;
    }
    auto handle = LoadCJLibrary(APP, dlName);
    if (!handle) {
        LOGE("load cj library failed: %{public}s", DynamicGetError());
        return nullptr;
    }
    LOGI("LoadCJLibrary InitCJLibrary: %{public}s", dlName);
    auto status = InitCJLibrary(dlName);
    if (status != E_OK) {
        LOGE("InitCJLibrary failed: %{public}s", dlName);
        UnLoadCJLibrary(handle);
        return nullptr;
    }
#ifdef __OHOS__
    CJEnvironment::RegisterCangjieCallback();
#endif
    isLoadCJLibrary_ = true;
    return handle;
}

void* CJEnvironment::LoadCJLibrary(OHOS::CJEnvironment::LibraryKind kind, const char* dlName)
{
#ifdef __OHOS__
    Dl_namespace ns;
    switch (kind) {
        case APP:
            dlns_get(CJEnvironment::cjAppNSName, &ns);
            break;
        case SYSTEM:
            dlns_get(CJEnvironment::cjSysNSName, &ns);
            break;
        case SDK:
            dlns_get(nsMode_ == NSMode::APP ? CJEnvironment::cjAppSDKNSName : CJEnvironment::cjCompatibilitySDKNSName,
                     &ns);
            break;
    }
    auto handle = DynamicLoadLibrary(&ns, dlName, 0);
#else
#ifdef __APPLE__
    auto handle = DynamicLoadLibrary(getLibPath(dlName).c_str(), 1);
#else
    auto handle = DynamicLoadLibrary(dlName, 1);
#endif
#endif
    if (!handle) {
        LOGE("load cj library failed: %{public}s", DynamicGetError());
        return nullptr;
    }
    isLoadCJLibrary_ = true;
    return handle;
}

bool CJEnvironment::CheckLoadCJLibrary()
{
    return isLoadCJLibrary_;
}

void CJEnvironment::UnLoadCJLibrary(void* handle)
{
    DynamicFreeLibrary(handle);
}

void* CJEnvironment::GetSymbol(void* dso, const char* symbol)
{
    return DynamicFindSymbol(dso, symbol);
}

bool CJEnvironment::StartDebugger()
{
#ifdef __OHOS__
    Dl_namespace ns;
    dlns_get(CJEnvironment::cjSysNSName, &ns);
    auto handle = DynamicLoadLibrary(&ns, DEBUGGER_LIBNAME, 0);
#else
#ifdef __APPLE__
    auto handle = DynamicLoadLibrary(getLibPath(DEBUGGER_LIBNAME).c_str(), 0);
#else
    auto handle = DynamicLoadLibrary(DEBUGGER_LIBNAME, 0);
#endif
#endif
    if (!handle) {
        LOGE("failed to load library: %{public}s", DEBUGGER_LIBNAME);
        return false;
    }
    auto symbol = DynamicFindSymbol(handle, DEBUGGER_SYMBOL_NAME);
    if (!symbol) {
        LOGE("failed to find symbol: %{public}s", DEBUGGER_SYMBOL_NAME);
        DynamicFreeLibrary(handle);
        return false;
    }
    auto func = reinterpret_cast<bool (*)(int, const std::string&)>(symbol);
    std::string name = "PandaDebugger";
    func(0, name);
    return true;
}

std::vector<uint32_t> SplitVersion(const std::string& version, char separator)
{
    std::vector<uint32_t> result;
    std::stringstream ss(version);
    std::string item;
    uint32_t num;
    while (std::getline(ss, item, separator)) {
        auto res = std::from_chars(item.data(), item.data() + item.size(), num);
        if (res.ec == std::errc()) {
            result.push_back(num);
        } else {
            LOGE("Incorrect version");
            return result;
        }
    }
    return result;
}

CJEnvironment::NSMode CJEnvironment::DetectAppNSMode()
{
    LOGI("App compileSDKVersion is %{public}s", CJEnvironment::appVersion.c_str());
    std::vector<uint32_t> tokens = SplitVersion(CJEnvironment::appVersion, '.');
    std::vector<uint32_t> checkTokens = SplitVersion(CJEnvironment::checkVersion, '.');
    if (tokens.size() != checkTokens.size()) {
        return NSMode::SINK;
    }
    for (size_t i = 0; i < checkTokens.size(); i++) {
        if (tokens[i] > checkTokens[i]) {
            return NSMode::SINK;
        }
        if (tokens[i] < checkTokens[i]) {
            return NSMode::APP;
        }
    }
    return NSMode::SINK;
}

void CJEnvironment::InitRuntimeNS()
{
#ifdef __OHOS__
    if (nsMode_ == NSMode::APP) {
        InitCJChipSDKNS(CJ_CHIPSDK_PATH);
        InitCJAppSDKNS(CJ_COMPATIBILITY_PATH + ":" + CJ_MOCK_PATH);
    } else {
        switch (CJEnvironment::sanitizerKind) {
            case SanitizerKind::ASAN:
                InitCJRuntimeNS(CJ_ASAN_PATH);
                break;
            default:
                InitCJRuntimeNS(CJ_RUNTIME_PATH);
        }
        InitCJMockNS(CJ_MOCK_PATH);
        InitCJRomSDKNS(CJ_SDK_PATH);
        InitCJCompatibilitySDKNS(CJ_COMPATIBILITY_PATH);
    }
#endif
}

void CJEnvironment::InitCJNS(const std::string& appPath)
{
#ifdef __OHOS__
    InitCJAppNS(appPath.empty() ? SANDBOX_LIB_PATH : appPath);
#endif
    if (!StartRuntime()) {
        LOGE("Failed to start cj runtime.");
        return;
    }
    StartUIScheduler();
}

void CJEnvironment::SetAppVersion(std::string& version)
{
    CJEnvironment::appVersion = version;
}

void CJEnvironment::SetSanitizerKindRuntimeVersion(SanitizerKind kind)
{
    LOGI("Set sanitizer for cj.");
    CJEnvironment::sanitizerKind = kind;
}

CJEnvMethods* CJEnvironment::CreateEnvMethods()
{
    static CJEnvMethods gCJEnvMethods {
        .initCJAppNS = [](const std::string& path) {
            // to keep compatibility with older version
            CJEnvironment::SetAppPath(path);
        },
        .initCJSDKNS = [](const std::string& path) {
            // @deprecated
        },
        .initCJSysNS = [](const std::string& path) {
            // @deprecated
        },
        .initCJChipSDKNS = [](const std::string& path) {
            // @deprecated
        },
        .startRuntime = [] {
            return CJEnvironment::GetInstance()->StartRuntime();
        },
        .startUIScheduler = [] {
            return CJEnvironment::GetInstance()->StartUIScheduler();
        },
        .loadCJModule = [](const char* dllName) {
            return CJEnvironment::GetInstance()->LoadCJLibrary(dllName);
        },
        .loadLibrary = [](uint32_t kind, const char* dllName) {
            return CJEnvironment::GetInstance()->LoadCJLibrary(static_cast<CJEnvironment::LibraryKind>(kind), dllName);
        },
        .getSymbol = [](void* handle, const char* dllName) {
            return CJEnvironment::GetInstance()->GetSymbol(handle, dllName);
        },
        .loadCJLibrary = [](const char* dllName) {
            return CJEnvironment::GetInstance()->LoadCJLibrary(dllName);
        },
        .startDebugger = []() {
            return CJEnvironment::GetInstance()->StartDebugger();
        },
        .registerCJUncaughtExceptionHandler = [](const CJUncaughtExceptionInfo& handle) {
            return CJEnvironment::GetInstance()->RegisterCJUncaughtExceptionHandler(handle);
        },
        .setSanitizerKindRuntimeVersion = [](SanitizerKind kind) {
            CJEnvironment::SetSanitizerKindRuntimeVersion(kind);
        },
        .checkLoadCJLibrary = []() {
            return CJEnvironment::GetInstance()->CheckLoadCJLibrary();
        },
        .registerArkVMInRuntime = [](unsigned long long arkVM) {
            CJEnvironment::GetInstance()->RegisterArkVMInRuntime(arkVM);
        },
        .registerStackInfoCallbacks = [](UpdateStackInfoFuncType uFunc) {
            CJEnvironment::GetInstance()->RegisterStackInfoCallbacks(uFunc);
        },
        .setAppVersion = [](std::string& version) {
            CJEnvironment::SetAppVersion(version);
        },
        .dumpHeapSnapshot = [](int fd) {
            CJEnvironment::DumpCjHeap(fd);
        },
        .forceFullGC = []() {
            CJEnvironment::GC();
        },
        .registerEventHandler = [](const CJEventReportInfo& handle) {
            return CJEnvironment::GetInstance()->RegisterEventHandler(handle);
        },
    };
    return &gCJEnvMethods;
}

CJ_EXPORT extern "C" void OHOS_InitSpawnEnv()
{
    CJEnvironment::InitSpawnEnv();
}

CJ_EXPORT extern "C" CJEnvMethods* OHOS_GetCJEnvInstance()
{
    return CJEnvironment::CreateEnvMethods();
}
}
