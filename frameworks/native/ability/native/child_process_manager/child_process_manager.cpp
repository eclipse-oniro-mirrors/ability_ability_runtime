/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "child_process_manager.h"

#include <csignal>
#include <filesystem>
#include <string>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "app_utils.h"
#include "application_info.h"
#include "app_mgr_interface.h"
#include "bundle_info.h"
#include "bundle_mgr_interface.h"
#include "child_process.h"
#include "native_args_child_process.h"
#include "native_child_ipc_process.h"
#include "child_process_manager_error_utils.h"
#include "child_process_request.h"
#include "child_process_start_info.h"
#include "constants.h"
#include "event_runner.h"
#include "errors.h"
#include "hap_module_info.h"
#include "hilog_tag_wrapper.h"
#include "ohos_application.h"
#include "parameters.h"
#include "runtime.h"
#include "sys_mgr_client.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
    bool g_jitEnabled = false;
    AbilityRuntime::Runtime::DebugOption g_debugOption;
    const std::string LARGE_SCREEN = "large_screen";
    constexpr char DEVELOPER_MODE_STATE[] = "const.security.developermode.state";
}
bool ChildProcessManager::signalRegistered_ = false;

ChildProcessManager &ChildProcessManager::GetInstance()
{
    static ChildProcessManager instance;
    return instance;
}

ChildProcessManager::ChildProcessManager()
{
    TAG_LOGD(AAFwkTag::PROCESSMGR, "called");
}

ChildProcessManager::~ChildProcessManager()
{
    TAG_LOGD(AAFwkTag::PROCESSMGR, "called");
}

ChildProcessManagerErrorCode ChildProcessManager::StartChildProcessBySelfFork(
    const std::string &srcEntry, pid_t &pid, bool isStaticChildProcess)
{
    TAG_LOGI(AAFwkTag::PROCESSMGR, "called");
    ChildProcessManagerErrorCode errorCode = PreCheckSelfFork();
    if (errorCode != ChildProcessManagerErrorCode::ERR_OK) {
        return errorCode;
    }

    AppExecFwk::BundleInfo bundleInfo;
    if (!GetBundleInfo(bundleInfo)) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "GetBundleInfo failed");
        return ChildProcessManagerErrorCode::ERR_GET_BUNDLE_INFO_FAILED;
    }

    if (!AAFwk::AppUtils::GetInstance().IsMultiProcessModel() && !IsMultiProcessFeatureApp(bundleInfo)) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Device features not support multi process.");
        return ChildProcessManagerErrorCode::ERR_MULTI_PROCESS_MODEL_DISABLED;
    }

    RegisterSignal();
    pid = fork();
    if (pid < 0) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Fork process failed");
        return ChildProcessManagerErrorCode::ERR_FORK_FAILED;
    }
    MakeProcessName(srcEntry); // set process name
    if (pid == 0) {
        const char *processName = g_debugOption.processName.c_str();
        if (prctl(PR_SET_NAME, processName) < 0) {
            TAG_LOGW(AAFwkTag::PROCESSMGR, "set process name failed %{public}d", errno);
        }
        HandleChildProcessBySelfFork(srcEntry, bundleInfo, isStaticChildProcess);
    }
    return ChildProcessManagerErrorCode::ERR_OK;
}

bool ChildProcessManager::IsMultiProcessFeatureApp(const AppExecFwk::BundleInfo &bundleInfo)
{
    for (const auto &info : bundleInfo.hapModuleInfos) {
        if (info.moduleType != AppExecFwk::ModuleType::ENTRY) {
            continue;
        }
        auto deviceType = OHOS::system::GetDeviceType();
        auto it = info.requiredDeviceFeatures.find(deviceType);
        if (it == info.requiredDeviceFeatures.end()) {
            return false;
        }
        auto &deviceFeatures = it->second;
        return std::find(deviceFeatures.begin(), deviceFeatures.end(), LARGE_SCREEN) != deviceFeatures.end();
    }
    return false;
}

ChildProcessManagerErrorCode ChildProcessManager::StartChildProcessByAppSpawnFork(
    const std::string &srcEntry, pid_t &pid, bool isStaticChildProcess)
{
    AppExecFwk::ChildProcessArgs args;
    AppExecFwk::ChildProcessOptions options;
    return StartChildProcessWithArgs(srcEntry, pid, AppExecFwk::CHILD_PROCESS_TYPE_JS, args, options,
        isStaticChildProcess);
}

ChildProcessManagerErrorCode ChildProcessManager::StartChildProcessWithArgs(
    const std::string &srcEntry, pid_t &pid, int32_t childProcessType, const AppExecFwk::ChildProcessArgs &args,
    const AppExecFwk::ChildProcessOptions &options, bool isStaticChildProcess)
{
    TAG_LOGI(AAFwkTag::PROCESSMGR, "StartChildProcessWithArgs, childProcessType:%{public}d, startWitDebug: %{public}d,"
        "processName:%{public}s, native:%{public}d, entryParams size:%{public}zu, fdsSize:%{public}zu, "
        "options.isolationMode:%{public}d, options.isolationUid: %{public}d, options.customProcessName:%{public}s",
        childProcessType, g_debugOption.isStartWithDebug, g_debugOption.processName.c_str(),
        g_debugOption.isStartWithNative, args.entryParams.length(), args.fds.size(), options.isolationMode,
        options.isolationUid, options.customProcessName.c_str());
    ChildProcessManagerErrorCode errorCode = PreCheck(childProcessType);
    if (errorCode != ChildProcessManagerErrorCode::ERR_OK) {
        return errorCode;
    }
    sptr<AppExecFwk::IAppMgr> appMgr = GetAppMgr();
    if (appMgr == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null appMgr");
        return ChildProcessManagerErrorCode::ERR_GET_APP_MGR_FAILED;
    }
    AppExecFwk::ChildProcessRequest request;
    request.srcEntry = srcEntry;
    request.childProcessType = childProcessType;
    request.isStartWithDebug = g_debugOption.isStartWithDebug;
    request.isStaticChildProcess = isStaticChildProcess;
    request.args = args;
    request.options = options;
    request.childProcessCount = childProcessCount_.fetch_add(1);
    auto ret = appMgr->StartChildProcess(pid, request);
    TAG_LOGD(AAFwkTag::PROCESSMGR, "AppMgr StartChildProcess ret:%{public}d", ret);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "StartChildProcess error:%{public}d", ret);
        return GetErrorCodeCompat(ret, childProcessType);
    }
    return ChildProcessManagerErrorCode::ERR_OK;
}

ChildProcessManagerErrorCode ChildProcessManager::GetErrorCodeCompat(int32_t ret, int32_t childProcessType)
{
    if (ret == AAFwk::ERR_NOT_SUPPORT_CHILD_PROCESS && childProcessType == AppExecFwk::CHILD_PROCESS_TYPE_JS) {
        return ChildProcessManagerErrorCode::ERR_MULTI_PROCESS_MODEL_DISABLED;
    }
    return ChildProcessManagerErrorUtil::GetChildProcessManagerErrorCode(ret);
}

ChildProcessManagerErrorCode ChildProcessManager::CreateNativeChildProcessByAppSpawnFork(
    const std::string &libName, const sptr<IRemoteObject> &callbackStub, const std::string &customProcessName,
    const bool isolationMode, const bool isolationUid)
{
    TAG_LOGI(AAFwkTag::PROCESSMGR, "libName:%{private}s", libName.c_str());
    ChildProcessManagerErrorCode errorCode = PreCheck(AppExecFwk::CHILD_PROCESS_TYPE_NATIVE);
    if (errorCode != ChildProcessManagerErrorCode::ERR_OK) {
        return errorCode;
    }

    sptr<AppExecFwk::IAppMgr> appMgr = GetAppMgr();
    if (appMgr == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null appMgr");
        return ChildProcessManagerErrorCode::ERR_GET_APP_MGR_FAILED;
    }

    AppExecFwk::ChildProcessOptions options;
    options.isolationMode = isolationMode;
    options.customProcessName = customProcessName;
    options.isolationUid = isolationUid;
    AppExecFwk::ChildProcessRequest request;
    request.childProcessType = AppExecFwk::CHILD_PROCESS_TYPE_NATIVE;
    request.childProcessCount = childProcessCount_.fetch_add(1);
    request.options = options;
    auto ret = appMgr->CreateNativeChildProcess(libName, callbackStub, request);
    TAG_LOGD(AAFwkTag::PROCESSMGR, "CreateNativeChildProcess ret:%{public}d", ret);

    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "CreateNativeChildProcess error:%{public}d", ret);
        return ChildProcessManagerErrorUtil::GetChildProcessManagerErrorCode(ret);
    }

    return ChildProcessManagerErrorCode::ERR_OK;
}

void ChildProcessManager::RegisterSignal()
{
    if (!signalRegistered_) {
        signalRegistered_ = true;
        TAG_LOGD(AAFwkTag::PROCESSMGR, "Register signal");
        signal(SIGCHLD, ChildProcessManager::HandleSigChild);
    }
}

void ChildProcessManager::HandleSigChild(int32_t signo)
{
    while (waitpid(-1, NULL, WNOHANG) > 0) {
        continue;
    }
}

bool ChildProcessManager::AllowChildProcessOnDevice()
{
    return AAFwk::AppUtils::GetInstance().IsMultiProcessModel() ||
        AAFwk::AppUtils::GetInstance().AllowChildProcessInMultiProcessFeatureApp();
}

ChildProcessManagerErrorCode ChildProcessManager::PreCheckSelfFork()
{
    if (!AllowChildProcessOnDevice()) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Multi process model disabled");
        return ChildProcessManagerErrorCode::ERR_MULTI_PROCESS_MODEL_DISABLED;
    }
    if (IsChildProcess()) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Already in child process");
        return ChildProcessManagerErrorCode::ERR_ALREADY_IN_CHILD_PROCESS;
    }
    return ChildProcessManagerErrorCode::ERR_OK;
}

ChildProcessManagerErrorCode ChildProcessManager::PreCheck(int32_t childProcessType)
{
    if (!AllowChildProcessOnDevice() &&
        childProcessType != AppExecFwk::CHILD_PROCESS_TYPE_NATIVE_ARGS &&
        childProcessType != AppExecFwk::CHILD_PROCESS_TYPE_NATIVE) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Not support child process.");
        auto useNewErrorCode = childProcessType != AppExecFwk::CHILD_PROCESS_TYPE_JS;
        return useNewErrorCode ? ChildProcessManagerErrorCode::ERR_MULTI_PROCESS_MODEL_DISABLED_NEW :
            ChildProcessManagerErrorCode::ERR_MULTI_PROCESS_MODEL_DISABLED;
    }
    if (isChildProcessBySelfFork_) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Already in child process");
        return ChildProcessManagerErrorCode::ERR_ALREADY_IN_CHILD_PROCESS;
    }
    return ChildProcessManagerErrorCode::ERR_OK;
}

bool ChildProcessManager::IsChildProcess()
{
    return isChildProcessBySelfFork_ || HasChildProcessRecord();
}

bool ChildProcessManager::IsChildProcessBySelfFork()
{
    return isChildProcessBySelfFork_;
}

bool ChildProcessManager::LoadFromAppRuntime(const std::string &srcEntry,
    const AppExecFwk::HapModuleInfo &hapModuleInfo)
{
    std::shared_ptr<AppExecFwk::OHOSApplication> application;
    {
        std::lock_guard<std::mutex> lock(appMutex_);
        application = application_.lock();
    }
    if (application == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null application");
        return false;
    }
    auto &appRuntime = application->GetRuntime();
    if (appRuntime == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null appRuntime");
        return false;
    }
    if (appRuntime->GetLanguage() == AbilityRuntime::Runtime::Language::ETS) {
        TAG_LOGD(AAFwkTag::PROCESSMGR, "Reuse main process runtime");
        TAG_LOGD(AAFwkTag::PROCESSMGR, "StartDebugMode, isStartWithDebug: %{public}d, processName: %{public}s, "
            "isDebugApp: %{public}d, isStartWithNative: %{public}d", g_debugOption.isStartWithDebug,
            g_debugOption.processName.c_str(), g_debugOption.isDebugApp, g_debugOption.isStartWithNative);
        appRuntime->StartDebugMode(g_debugOption);
        LoadJsFile(srcEntry, hapModuleInfo, appRuntime);
        return true;
    }
    return false;
}

void ChildProcessManager::HandleChildProcessBySelfFork(const std::string &srcEntry,
    const AppExecFwk::BundleInfo &bundleInfo, bool isStaticChildProcess)
{
    TAG_LOGD(AAFwkTag::PROCESSMGR, "start");
    isChildProcessBySelfFork_ = true;
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner = AppExecFwk::EventRunner::GetMainEventRunner();
    if (eventRunner == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null eventRunner");
        return;
    }
    eventRunner->Stop();
    
    AppExecFwk::HapModuleInfo hapModuleInfo;
    if (!GetEntryHapModuleInfo(bundleInfo, hapModuleInfo)) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "GetHapModuleInfo failed");
        return;
    }

    if (isStaticChildProcess) {
        auto pos = srcEntry.find('/');
        std::string prefix = (pos != std::string::npos) ? srcEntry.substr(0, pos) : srcEntry;
        if (prefix != "entry") {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "srcEntry prefix is not entry, prefix: %{public}s", prefix.c_str());
            exit(0);
        }
    }

    if (isStaticChildProcess && LoadFromAppRuntime(srcEntry, hapModuleInfo)) {
        TAG_LOGD(AAFwkTag::PROCESSMGR, "end");
        exit(0);
    }

    auto runtime = CreateRuntime(bundleInfo, hapModuleInfo, false, g_jitEnabled, isStaticChildProcess);
    if (!runtime) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Create runtime failed");
        return;
    }
    std::string srcPath = isStaticChildProcess ? srcEntry : hapModuleInfo.moduleName + "/" + srcEntry;
    TAG_LOGD(AAFwkTag::PROCESSMGR, "StartDebugMode, isStartWithDebug: %{public}d, processName: %{public}s, "
        "isDebugApp: %{public}d, isStartWithNative: %{public}d", g_debugOption.isStartWithDebug,
        g_debugOption.processName.c_str(), g_debugOption.isDebugApp, g_debugOption.isStartWithNative);
    runtime->StartDebugMode(g_debugOption);
    LoadJsFile(srcPath, hapModuleInfo, runtime);
    TAG_LOGD(AAFwkTag::PROCESSMGR, "end");
    exit(0);
}

bool ChildProcessManager::LoadJsFile(const std::string &srcEntry, const AppExecFwk::HapModuleInfo &hapModuleInfo,
    const std::unique_ptr<AbilityRuntime::Runtime> &runtime, std::shared_ptr<AppExecFwk::ChildProcessArgs> args)
{
    std::shared_ptr<ChildProcessStartInfo> processStartInfo = std::make_shared<ChildProcessStartInfo>();
    std::string filename = std::filesystem::path(srcEntry).stem();
    processStartInfo->name = filename;
    processStartInfo->moduleName = hapModuleInfo.moduleName;
    processStartInfo->hapPath = hapModuleInfo.hapPath;
    processStartInfo->srcEntry = srcEntry;
    processStartInfo->isEsModule = (hapModuleInfo.compileMode == AppExecFwk::CompileMode::ES_MODULE);

    auto process = ChildProcess::Create(runtime);
    if (process == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null process");
        return false;
    }
    bool ret = process->Init(processStartInfo);
    if (!ret) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "init failed");
        return false;
    }
    if (args) {
        process->OnStart(args);
    } else {
        process->OnStart();
    }
    TAG_LOGD(AAFwkTag::PROCESSMGR, "end");
    return true;
}

bool ChildProcessManager::LoadNativeLib(const std::string &moduleName,
    const std::string &libPath, const sptr<IRemoteObject> &mainProcessCb)
{
    auto childProcess = NativeChildIpcProcess::Create();
    if (childProcess == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null childIpcProcess");
        return false;
    }
    
    std::shared_ptr<ChildProcessStartInfo> processStartInfo = std::make_shared<ChildProcessStartInfo>();
    processStartInfo->moduleName = moduleName;
    processStartInfo->name = std::filesystem::path(libPath).stem();
    processStartInfo->srcEntry = libPath;
    processStartInfo->ipcObj = mainProcessCb;
    if (!childProcess->Init(processStartInfo)) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "NativeChildIpcProcess init failed");
        return false;
    }
    
    childProcess->OnStart();
    TAG_LOGD(AAFwkTag::PROCESSMGR, "end");
    return true;
}

bool ChildProcessManager::LoadNativeLibWithArgs(const std::string &moduleName, const std::string &srcEntry,
    const std::string &entryFunc, std::shared_ptr<AppExecFwk::ChildProcessArgs> args)
{
    TAG_LOGI(AAFwkTag::PROCESSMGR, "moduleName:%{public}s, srcEntry:%{public}s, entryFunc:%{public}s",
        moduleName.c_str(), srcEntry.c_str(), entryFunc.c_str());
    auto childProcess = NativeArgsChildProcess::Create();
    if (childProcess == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null childProcess");
        return false;
    }

    std::shared_ptr<ChildProcessStartInfo> processStartInfo = std::make_shared<ChildProcessStartInfo>();
    processStartInfo->moduleName = moduleName;
    processStartInfo->srcEntry = srcEntry;
    processStartInfo->entryFunc = entryFunc;
    if (!childProcess->Init(processStartInfo)) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "nativeArgsChildProcess init failed");
        return false;
    }

    childProcess->OnStart(args);
    TAG_LOGD(AAFwkTag::PROCESSMGR, "LoadNativeLibWithArgs end.");
    return true;
}

std::unique_ptr<AbilityRuntime::Runtime> ChildProcessManager::CreateRuntime(
    const AppExecFwk::BundleInfo &bundleInfo, const AppExecFwk::HapModuleInfo &hapModuleInfo,
    const bool fromAppSpawn, const bool jitEnabled, bool isStaticChildProcess)
{
    AppExecFwk::ApplicationInfo applicationInfo = bundleInfo.applicationInfo;
    AbilityRuntime::Runtime::Options options;
    options.codePath = AbilityBase::Constants::LOCAL_CODE_PATH;
    options.bundleName = hapModuleInfo.bundleName;
    options.hapPath = hapModuleInfo.hapPath;
    options.moduleName = hapModuleInfo.moduleName;
    options.isBundle = (hapModuleInfo.compileMode != AppExecFwk::CompileMode::ES_MODULE);
    options.uid = applicationInfo.uid;
    options.isDebugVersion = applicationInfo.debug;
    options.arkNativeFilePath = applicationInfo.arkNativeFilePath;
    options.apiTargetVersion = applicationInfo.apiTargetVersion;
    options.loadAce = true;
    options.jitEnabled = jitEnabled;
    options.lang = isStaticChildProcess ? AbilityRuntime::Runtime::Language::ETS :
        AbilityRuntime::Runtime::Language::JS;

    for (auto &moduleItem : bundleInfo.hapModuleInfos) {
        options.pkgContextInfoJsonStringMap[moduleItem.moduleName] = moduleItem.hapPath;
        options.packageNameList[moduleItem.moduleName] = moduleItem.packageName;
    }

    std::shared_ptr<AppExecFwk::EventRunner> eventRunner =
        fromAppSpawn ? AppExecFwk::EventRunner::GetMainEventRunner() : AppExecFwk::EventRunner::Create();
    options.eventRunner = eventRunner;

    return AbilityRuntime::Runtime::Create(options);
}

bool ChildProcessManager::GetBundleInfo(AppExecFwk::BundleInfo &bundleInfo)
{
    auto sysMrgClient = DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance();
    if (sysMrgClient == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null sysMrgClient");
        return false;
    }
    auto bundleObj = sysMrgClient->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (bundleObj == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null bundleObj");
        return false;
    }
    auto bundleMgr = iface_cast<AppExecFwk::IBundleMgr>(bundleObj);
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null bundleMgr");
        return false;
    }
    return (bundleMgr->GetBundleInfoForSelf(
        (static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE) +
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION) +
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_METADATA)), bundleInfo) == ERR_OK);
}

bool ChildProcessManager::GetEntryHapModuleInfo(const AppExecFwk::BundleInfo &bundleInfo,
    AppExecFwk::HapModuleInfo &hapModuleInfo)
{
    if (bundleInfo.hapModuleInfos.empty()) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "hapModuleInfos empty");
        return false;
    }
    TAG_LOGD(AAFwkTag::PROCESSMGR, "hapModueInfos size: %{public}zu", bundleInfo.hapModuleInfos.size());
    bool result = false;
    for (const auto &info : bundleInfo.hapModuleInfos) {
        if (info.moduleType == AppExecFwk::ModuleType::ENTRY) {
            result = true;
            hapModuleInfo = info;
            break;
        }
    }
    return result;
}

bool ChildProcessManager::GetHapModuleInfo(const AppExecFwk::BundleInfo &bundleInfo, const std::string &moduleName,
    AppExecFwk::HapModuleInfo &hapModuleInfo)
{
    if (bundleInfo.hapModuleInfos.empty()) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "hapModuleInfos empty");
        return false;
    }
    TAG_LOGD(AAFwkTag::PROCESSMGR, "hapModueInfos size: %{public}zu", bundleInfo.hapModuleInfos.size());
    bool result = false;
    for (const auto &info : bundleInfo.hapModuleInfos) {
        if (info.name == moduleName) {
            result = true;
            hapModuleInfo = info;
            break;
        }
    }
    return result;
}

bool ChildProcessManager::HasChildProcessRecord()
{
    sptr<AppExecFwk::IAppMgr> appMgr = GetAppMgr();
    if (appMgr == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null appMgr");
        return false;
    }
    AppExecFwk::ChildProcessInfo info;
    return appMgr->GetChildProcessInfoForSelf(info) == ERR_OK;
}

sptr<AppExecFwk::IAppMgr> ChildProcessManager::GetAppMgr()
{
    auto sysMrgClient = DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance();
    if (sysMrgClient == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null sysMrgClient");
        return nullptr;
    }
    auto object = sysMrgClient->GetSystemAbility(APP_MGR_SERVICE_ID);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null object");
        return nullptr;
    }
    return iface_cast<AppExecFwk::IAppMgr>(object);
}

void ChildProcessManager::SetForkProcessJITEnabled(bool jitEnabled)
{
    g_jitEnabled = jitEnabled;
}

void ChildProcessManager::SetForkProcessDebugOption(const std::string bundleName, const bool isStartWithDebug,
    const bool isDebugApp, const bool isStartWithNative)
{
    g_debugOption.bundleName = bundleName;
    g_debugOption.isStartWithDebug = isStartWithDebug;
    g_debugOption.isDebugApp = isDebugApp;
    g_debugOption.isStartWithNative = isStartWithNative;
}

void ChildProcessManager::SetAppSpawnForkDebugOption(Runtime::DebugOption &debugOption,
    std::shared_ptr<AppExecFwk::ChildProcessInfo> processInfo)
{
    if (!processInfo) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null processInfo");
        return;
    }
    debugOption.processName = processInfo->processName;
    debugOption.isStartWithDebug = processInfo->isStartWithDebug;
    debugOption.isDebugApp = processInfo->isDebugApp;
    debugOption.isStartWithNative = processInfo->isStartWithNative;
    debugOption.isDeveloperMode = system::GetBoolParameter(DEVELOPER_MODE_STATE, false);
}

void ChildProcessManager::MakeProcessName(const std::string &srcEntry)
{
    std::string processName = g_debugOption.bundleName;
    if (srcEntry.empty()) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "srcEntry empty");
    } else {
        TAG_LOGW(AAFwkTag::PROCESSMGR, "srcEntry not empty");
        std::string filename = std::filesystem::path(srcEntry).stem();
        if (!filename.empty()) {
            processName.append(":");
            processName.append(filename);
        }
    }
    processName.append(std::to_string(childProcessCount_.fetch_add(1)));
    TAG_LOGD(AAFwkTag::PROCESSMGR, "processName: %{public}s", processName.c_str());
    g_debugOption.processName = processName;
}

std::string ChildProcessManager::GetModuleNameFromSrcEntry(const std::string &srcEntry)
{
    std::string::size_type pos = srcEntry.find_first_of('/');
    if (pos == std::string::npos) {
        return "";
    }
    std::string moduleName = srcEntry.substr(0, pos);
    if (moduleName == ".") {
        return "";
    }
    return moduleName;
}

void ChildProcessManager::SetApplication(std::shared_ptr<AppExecFwk::OHOSApplication> application)
{
    application_ = application;
}

ChildProcessManagerErrorCode ChildProcessManager::KillChildProcessByPid(int32_t pid)
{
    TAG_LOGD(AAFwkTag::PROCESSMGR, "pid:%{public}d", pid);
    sptr<AppExecFwk::IAppMgr> appMgr = GetAppMgr();
    if (appMgr == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null appMgr");
        return ChildProcessManagerErrorCode::ERR_GET_APP_MGR_FAILED;
    }
    auto ret = appMgr->KillChildProcessByPid(pid);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "KillChildProcessByPid error:%{public}d, pid:%{public}d", ret, pid);
        return ChildProcessManagerErrorUtil::GetChildProcessManagerErrorCode(ret);
    }
    return ChildProcessManagerErrorCode::ERR_OK;
}
}  // namespace AbilityRuntime
}  // namespace OHOS