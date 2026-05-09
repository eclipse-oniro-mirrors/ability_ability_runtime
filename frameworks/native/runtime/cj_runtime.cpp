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

#include "cj_runtime.h"

#include <dlfcn.h>
#include <unistd.h>
#include <filesystem>
#include <regex>

#include "cj_envsetup.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "hdc_register.h"
#include "parameters.h"
#include "bundle_constants.h"
#include "connect_server_manager.h"
#include "faultloggerd_client.h"

using namespace OHOS::AbilityRuntime;

#define LIB_NAME "libcj_environment.z.so"
#define GET_ENV_INS_NAME "OHOS_GetCJEnvInstance"

namespace OHOS {
CJEnvMethods* CJEnv::LoadInstance()
{
    auto handle = dlopen(LIB_NAME, RTLD_NOW);
    if (!handle) {
        TAG_LOGE(AAFwkTag::CJRUNTIME, "dlopen failed %{public}s, %{public}s", LIB_NAME, dlerror());
        return nullptr;
    }
    auto symbol = dlsym(handle, GET_ENV_INS_NAME);
    if (!symbol) {
        TAG_LOGE(AAFwkTag::CJRUNTIME, "dlsym failed %{public}s, %{public}s", GET_ENV_INS_NAME, dlerror());
        dlclose(handle);
        return nullptr;
    }
    auto func = reinterpret_cast<CJEnvMethods* (*)()>(symbol);
    return func();
}
}
AppLibPathVec CJRuntime::appLibPaths_;

std::string CJRuntime::packageName_;

std::unique_ptr<CJRuntime> CJRuntime::Create(const Options& options)
{
    auto instance = std::make_unique<CJRuntime>();
    if (!instance || !instance->Initialize(options)) {
        return nullptr;
    }
    return instance;
}

void CJRuntime::SetAppLibPath(const AppLibPathMap& appLibPaths)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, "Initialize cangjie runtime and namespace");
    std::string appPath = "";
    for (const auto& kv : appLibPaths) {
        for (const auto& libPath : kv.second) {
            TAG_LOGD(AAFwkTag::CJRUNTIME, "SetCJAppLibPath: %{public}s.", libPath.c_str());
            CJRuntime::appLibPaths_.emplace_back(libPath);
            appPath += appPath.empty() ? libPath : ":" + libPath;
        }
    }
    auto cjEnv = OHOS::CJEnv::LoadInstance();
    if (cjEnv == nullptr) {
        TAG_LOGE(AAFwkTag::CJRUNTIME, "null cjEnv");
        return;
    }

    cjEnv->initCJAppNS(appPath);
}

bool CJRuntime::Initialize(const Options& options)
{
    if (options.lang != GetLanguage()) {
        TAG_LOGE(AAFwkTag::CJRUNTIME, "language mismatch");
        return false;
    }
    auto cjEnv = OHOS::CJEnv::LoadInstance();
    if (cjEnv == nullptr) {
        TAG_LOGE(AAFwkTag::CJRUNTIME, "null cjEnv");
        return false;
    }
    if (!cjEnv->startRuntime()) {
        TAG_LOGE(AAFwkTag::CJRUNTIME, "start cj runtime failed");
        return false;
    }
    if (!cjEnv->startUIScheduler()) {
        TAG_LOGE(AAFwkTag::CJRUNTIME, "start cj ui context failed");
        return false;
    }
    if (!LoadCJAppLibrary(CJRuntime::appLibPaths_)) {
        TAG_LOGE(AAFwkTag::CJRUNTIME, "load app library fail");
        return false;
    }
    bundleName_ = options.bundleName;
    instanceId_ = static_cast<uint32_t>(getproctid());
    return true;
}

void CJRuntime::RegisterUncaughtExceptionHandler(const CJUncaughtExceptionInfo& uncaughtExceptionInfo)
{
    auto cjEnv = OHOS::CJEnv::LoadInstance();
    if (cjEnv == nullptr) {
        TAG_LOGE(AAFwkTag::CJRUNTIME, "null cjEnv");
        return;
    }
    cjEnv->registerCJUncaughtExceptionHandler(uncaughtExceptionInfo);
}

void CJRuntime::RegisterEventHandler(const CJEventReportInfo& reportInfo)
{
    auto cjEnv = OHOS::CJEnv::LoadInstance();
    if (cjEnv == nullptr) {
        TAG_LOGE(AAFwkTag::CJRUNTIME, "null cjEnv");
        return;
    }
    cjEnv->registerEventHandler(reportInfo);
}

bool CJRuntime::IsCJAbility(const std::string& info)
{
    // in cj application, the srcEntry format should be packageName.AbilityClassName.
    std::string pattern = "^([a-zA-Z0-9_]+\\.)+[a-zA-Z0-9_]+$";
    return std::regex_match(info, std::regex(pattern));
}

bool CJRuntime::LoadCJAppLibrary(const AppLibPathVec& appLibPaths)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    auto cjEnv = OHOS::CJEnv::LoadInstance();
    if (cjEnv == nullptr) {
        TAG_LOGE(AAFwkTag::CJRUNTIME, "null cjEnv");
        return false;
    }
    void* handle = nullptr;
    // According to the OHOS rule, the format of the SO name is as follows
    auto targetSoName = "lib" + packageName_ + ".so";

    for (const auto& libPath : appLibPaths) {
        for (auto& itor : std::filesystem::directory_iterator(libPath)) {
            // According to the convention, the names of cj generated products must contain the following keywords
            if (itor.path().string().find(targetSoName) == std::string::npos) {
                continue;
            }
            handle = cjEnv->loadCJLibrary(itor.path().c_str());
            if (handle == nullptr) {
                char* errMsg = dlerror();
                TAG_LOGE(AAFwkTag::CJRUNTIME,
                    "load %{public}s failed, reason: %{public}s", itor.path().c_str(), errMsg ? errMsg : "null");
                return false;
            }
        }
    }
    appLibLoaded_ = true;
    return true;
}

void CJRuntime::SetPackageName(std::string srcEntryName)
{
    // According to the srcEntry rule in the Cangjie application,
    // the last '.' The previous strings were all package names
    packageName_ = srcEntryName.substr(0, srcEntryName.find_last_of("."));
}

void CJRuntime::SetSanitizerVersion(SanitizerKind kind)
{
    auto cjEnv = OHOS::CJEnv::LoadInstance();
    if (cjEnv == nullptr) {
        TAG_LOGE(AAFwkTag::CJRUNTIME, "null cjEnv");
        return;
    }
    cjEnv->setSanitizerKindRuntimeVersion(kind);
}

bool CJRuntime::RegisterCangjieCallback()
{
    auto cjEnv = OHOS::CJEnv::LoadInstance();
    if (cjEnv == nullptr) {
        TAG_LOGE(AAFwkTag::CJRUNTIME, "null cjEnv");
        return false;
    }
    constexpr char CANGJIE_DEBUGGER_LIB_PATH[] = "libark_connect_inspector.z.so";
    #define LIBARARYKIND_SYS 0
    auto handlerConnectServerSo = cjEnv->loadLibrary(LIBARARYKIND_SYS, CANGJIE_DEBUGGER_LIB_PATH);
    if (handlerConnectServerSo == nullptr) {
        TAG_LOGE(AAFwkTag::CJRUNTIME, "null handlerConnectServerSo: %{public}s", dlerror());
        return false;
    }
    using SendMsgCB = const std::function<void(const std::string& message)>;
    using SetCangjieCallback = void(*)(const std::function<void(const std::string& message, SendMsgCB)>);
    using CangjieCallback = void(*)(const std::string& message, SendMsgCB);
    auto setCangjieCallback = reinterpret_cast<SetCangjieCallback>(
        cjEnv->getSymbol(handlerConnectServerSo, "SetCangjieCallback"));
    if (setCangjieCallback == nullptr) {
        TAG_LOGE(AAFwkTag::CJRUNTIME, "null setCangjieCallback: %{public}s", dlerror());
        return false;
    }
    #define RTLIB_NAME "libcangjie-runtime.so"
    #define LIBARARYKIND_SDK 1
    auto dso = cjEnv->loadLibrary(LIBARARYKIND_SDK, RTLIB_NAME);
    if (!dso) {
        TAG_LOGE(AAFwkTag::CJRUNTIME, "load library failed: %{public}s", RTLIB_NAME);
        return false;
    }
    TAG_LOGE(AAFwkTag::CJRUNTIME, "load libcangjie-runtime.so success");
    #define PROFILERAGENT "ProfilerAgent"
    CangjieCallback cangjieCallback = reinterpret_cast<CangjieCallback>(cjEnv->getSymbol(dso, PROFILERAGENT));
    if (cangjieCallback == nullptr) {
        TAG_LOGE(AAFwkTag::CJRUNTIME, "runtime api not found: %{public}s", PROFILERAGENT);
        dlclose(handlerConnectServerSo);
        handlerConnectServerSo = nullptr;
        return false;
    }
    TAG_LOGE(AAFwkTag::CJRUNTIME, "find runtime api success");
    setCangjieCallback(cangjieCallback);
    dlclose(handlerConnectServerSo);
    handlerConnectServerSo = nullptr;
    return true;
}

void CJRuntime::StartProfiler(const DebugOption dOption)
{
    if (!dOption.isDebugFromLocal && !dOption.isDeveloperMode) {
        TAG_LOGE(AAFwkTag::CJRUNTIME, "developer Mode false");
        return;
    }
    bool isStartWithDebug = dOption.isStartWithDebug;
    bool isDebugApp = dOption.isDebugApp;
    const std::string bundleName = bundleName_;
    int32_t instanceId = static_cast<int32_t>(instanceId_);
    std::string appProvisionType = dOption.appProvisionType;
    std::string inputProcessName = bundleName_ != dOption.processName ? dOption.processName : "";

    HdcRegister::Get().StartHdcRegister(bundleName_, inputProcessName, isDebugApp,
        HdcRegister::DebugRegisterMode::HDC_DEBUG_REG,
        [bundleName, isStartWithDebug, isDebugApp, instanceId, appProvisionType](int socketFd, std::string option) {
            TAG_LOGI(AAFwkTag::CJRUNTIME, "hdcRegister callback call, socket fd: %{public}d, option: %{public}s.",
                socketFd, option.c_str());
            bool isSystemDebuggable = system::GetBoolParameter("const.secure", true) == false &&
            system::GetBoolParameter("const.debuggable", false) == true;
            // Don't start any server if (system not in debuggable mode) and app is release version
            // Starting ConnectServer in release app on debuggable system
            // is only for debug mode, not for profiling mode.
            if ((!isSystemDebuggable) && appProvisionType == AppExecFwk::Constants::APP_PROVISION_TYPE_RELEASE) {
                TAG_LOGE(AAFwkTag::CJRUNTIME, "not support release app");
                return;
            }
            if (option.find(DEBUGGER) == std::string::npos) {
                ConnectServerManager::Get().StopConnectServer(false);
                TAG_LOGI(AAFwkTag::CJRUNTIME, "start SendInstanceMessage");
                ConnectServerManager::Get().SendInstanceMessage(instanceId, instanceId, bundleName);
                ConnectServerManager::Get().SendDebuggerInfo(isStartWithDebug, isDebugApp);
                ConnectServerManager::Get().StartConnectServer(bundleName, socketFd, false);
                CJRuntime::RegisterCangjieCallback();
            } else {
                TAG_LOGE(AAFwkTag::CJRUNTIME, "debugger service unexpected option: %{public}s", option.c_str());
            }
        });
}

void CJRuntime::StartDebugMode(const DebugOption dOption)
{
    if (debugModel_) {
        TAG_LOGI(AAFwkTag::CJRUNTIME, "already debug mode");
        return;
    }
    if (!dOption.isDebugFromLocal && !dOption.isDeveloperMode) {
        TAG_LOGE(AAFwkTag::CJRUNTIME, "developer Mode false");
        return;
    }

    bool isStartWithDebug = dOption.isStartWithDebug;
    bool isDebugApp = dOption.isDebugApp;
    const std::string bundleName = bundleName_;
    int32_t instanceId = static_cast<int32_t>(instanceId_);
    std::string appProvisionType = dOption.appProvisionType;
    std::string inputProcessName = bundleName_ != dOption.processName ? dOption.processName : "";

    TAG_LOGI(AAFwkTag::CJRUNTIME, "StartDebugMode %{public}s", bundleName_.c_str());

    HdcRegister::Get().StartHdcRegister(bundleName_, inputProcessName, isDebugApp,
        HdcRegister::DebugRegisterMode::HDC_DEBUG_REG,
        [bundleName, isStartWithDebug, isDebugApp, instanceId, appProvisionType](int socketFd, std::string option) {
            TAG_LOGI(AAFwkTag::CJRUNTIME, "hdcRegister callback call, socket fd: %{public}d, option: %{public}s.",
                socketFd, option.c_str());
                    // system is debuggable when const.secure is false and const.debuggable is true
            bool isSystemDebuggable = system::GetBoolParameter("const.secure", true) == false &&
            system::GetBoolParameter("const.debuggable", false) == true;
            // Don't start any server if (system not in debuggable mode) and app is release version
            // Starting ConnectServer in release app on debuggable system
            // is only for debug mode, not for profiling mode.
            if ((!isSystemDebuggable) && appProvisionType == AppExecFwk::Constants::APP_PROVISION_TYPE_RELEASE) {
                TAG_LOGE(AAFwkTag::CJRUNTIME, "not support release app");
                return;
            }
            if (option.find(DEBUGGER) == std::string::npos) {
                ConnectServerManager::Get().StopConnectServer(false);
                TAG_LOGI(AAFwkTag::CJRUNTIME, "start SendInstanceMessage");
                ConnectServerManager::Get().SendInstanceMessage(instanceId, instanceId, bundleName);
                ConnectServerManager::Get().SendDebuggerInfo(isStartWithDebug, isDebugApp);
                ConnectServerManager::Get().StartConnectServer(bundleName, socketFd, false);
                CJRuntime::RegisterCangjieCallback();
            } else {
                TAG_LOGE(AAFwkTag::CJRUNTIME, "debugger service unexpected option: %{public}s", option.c_str());
            }
        });
    if (isDebugApp) {
        ConnectServerManager::Get().StartConnectServer(bundleName_, -1, true);
    }
    ConnectServerManager::Get().AddInstance(instanceId_, instanceId_);

    debugModel_ = StartDebugger();
}

bool CJRuntime::StartDebugger()
{
    auto cjEnv = OHOS::CJEnv::LoadInstance();
    if (cjEnv == nullptr) {
        TAG_LOGE(AAFwkTag::CJRUNTIME, "null cjEnv");
        return false;
    }
    return cjEnv->startDebugger();
}

void CJRuntime::UnLoadCJAppLibrary()
{
    TAG_LOGI(AAFwkTag::CJRUNTIME, "UnLoadCJAppLibrary not support yet");
}

void CJRuntime::SetAppVersion(std::string& version)
{
    auto cjEnv = OHOS::CJEnv::LoadInstance();
    if (cjEnv == nullptr) {
        TAG_LOGE(AAFwkTag::CJRUNTIME, "null cjEnv");
        return;
    }
    return cjEnv->setAppVersion(version);
}

void CJRuntime::DumpHeapSnapshot(uint32_t tid, bool isFullGC, bool isBinary)
{
    auto cjEnv = OHOS::CJEnv::LoadInstance();
    if (cjEnv == nullptr) {
        TAG_LOGE(AAFwkTag::CJRUNTIME, "null cjEnv");
        return;
    }
    int32_t fd = RequestFileDescriptor(static_cast<int32_t>(FaultLoggerType::CJ_HEAP_SNAPSHOT));
    if (fd < 0) {
        TAG_LOGE(AAFwkTag::CJRUNTIME, "fd:%{public}d.\n", fd);
        return;
    }
    cjEnv->dumpHeapSnapshot(fd);
    close(fd);
}

void CJRuntime::ForceFullGC(uint32_t tid)
{
    auto cjEnv = OHOS::CJEnv::LoadInstance();
    if (cjEnv == nullptr) {
        TAG_LOGE(AAFwkTag::CJRUNTIME, "null cjEnv");
        return;
    }
    cjEnv->forceFullGC();
}