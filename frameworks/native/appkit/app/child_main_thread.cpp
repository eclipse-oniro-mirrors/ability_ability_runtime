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

#include "child_main_thread.h"

#include <unistd.h>

#include "bundle_mgr_helper.h"
#include "bundle_mgr_proxy.h"
#include "child_process_manager.h"
#include "constants.h"
#include "hilog_tag_wrapper.h"
#include "js_runtime.h"
#include "native_lib_util.h"
#include "sys_mgr_client.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::AbilityBase::Constants;
using OHOS::AbilityRuntime::ChildProcessManager;
ChildMainThread::ChildMainThread()
{
    processArgs_ = std::make_shared<ChildProcessArgs>();
}

ChildMainThread::~ChildMainThread()
{
    TAG_LOGD(AAFwkTag::APPKIT, "ChildMainThread deconstructor called");
}

void ChildMainThread::Start(const std::map<std::string, int32_t> &fds)
{
    TAG_LOGI(AAFwkTag::APPKIT, "ChildMainThread start");
    ChildProcessInfo processInfo;
    auto ret = GetChildProcessInfo(processInfo);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "GetChildProcessInfo failed, ret:%{public}d", ret);
        return;
    }

    sptr<ChildMainThread> thread = sptr<ChildMainThread>(new (std::nothrow) ChildMainThread());
    if (thread == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "New ChildMainThread failed");
        return;
    }
    thread->SetFds(fds);
    std::shared_ptr<EventRunner> runner = EventRunner::GetMainEventRunner();
    if (runner == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null runner");
        return;
    }
    if (!thread->Init(runner, processInfo)) {
        TAG_LOGE(AAFwkTag::APPKIT, "ChildMainThread Init failed");
        return;
    }
    if (!thread->Attach()) {
        TAG_LOGE(AAFwkTag::APPKIT, "ChildMainThread Attach failed");
        return;
    }

    ret = runner->Run();
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "ChildMainThread runner->Run failed ret = %{public}d", ret);
        return;
    }

    TAG_LOGD(AAFwkTag::APPKIT, "ChildMainThread end");
}

int32_t ChildMainThread::GetChildProcessInfo(ChildProcessInfo &info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    auto sysMgr = DelayedSingleton<SysMrgClient>::GetInstance();
    if (sysMgr == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "sys mgr invalid");
        return ERR_INVALID_VALUE;
    }

    auto object = sysMgr->GetSystemAbility(APP_MGR_SERVICE_ID);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "get app manager service failed");
        return ERR_INVALID_VALUE;
    }
    auto appMgr = iface_cast<IAppMgr>(object);
    if (appMgr == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "iface_cast object to appMgr failed");
        return ERR_INVALID_VALUE;
    }
    return appMgr->GetChildProcessInfoForSelf(info);
}

void ChildMainThread::SetFds(const std::map<std::string, int32_t> &fds)
{
    if (processArgs_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null processArgs_");
        return;
    }
    processArgs_->fds = fds;
}

bool ChildMainThread::Init(const std::shared_ptr<EventRunner> &runner, const ChildProcessInfo &processInfo)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (runner == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null runner");
        return false;
    }
    processInfo_ = std::make_shared<ChildProcessInfo>(processInfo);
    processArgs_->entryParams = processInfo.entryParams;
    mainHandler_ = std::make_shared<EventHandler>(runner);
    BundleInfo bundleInfo;
    if (!ChildProcessManager::GetInstance().GetBundleInfo(bundleInfo)) {
        TAG_LOGE(AAFwkTag::APPKIT, "GetBundleInfo failed");
        return false;
    }
    bundleInfo_ = std::make_shared<BundleInfo>(bundleInfo);
    InitNativeLib(bundleInfo);
    return true;
}

bool ChildMainThread::Attach()
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    auto sysMrgClient = DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance();
    if (sysMrgClient == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "get SysMrgClient failed");
        return false;
    }
    auto object = sysMrgClient->GetSystemAbility(APP_MGR_SERVICE_ID);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "GetAppMgr failed");
        return false;
    }
    appMgr_ = iface_cast<IAppMgr>(object);
    if (appMgr_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "iface_cast object to appMgr_ failed");
        return false;
    }
    appMgr_->AttachChildProcess(this);
    return true;
}

bool ChildMainThread::ScheduleLoadChild()
{
    TAG_LOGI(AAFwkTag::APPKIT, "ScheduleLoadChild called.");
    if (mainHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null mainHandler_");
        return false;
    }
    if (processInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null processInfo");
        return false;
    }
    auto childProcessType = processInfo_->childProcessType;
    wptr<ChildMainThread> weak = this;
    auto task = [weak, childProcessType]() {
        auto childMainThread = weak.promote();
        if (childMainThread == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null childMainThread, ScheduleLoadChild failed.");
            return;
        }
        if (childProcessType == CHILD_PROCESS_TYPE_ARK) {
            childMainThread->HandleLoadArkTs();
        } else if (childProcessType == CHILD_PROCESS_TYPE_NATIVE_ARGS) {
            childMainThread->HandleLoadNative();
        } else {
            childMainThread->HandleLoadJs();
        }
    };
    if (!mainHandler_->PostTask(task, "ChildMainThread::HandleLoadJs")) {
        TAG_LOGE(AAFwkTag::APPKIT, "ChildMainThread::ScheduleLoadChild PostTask task failed.");
        return false;
    }
    return true;
}

void ChildMainThread::HandleLoadJs()
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (processInfo_ == nullptr || bundleInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null processInfo or bundleInfo_");
        return;
    }
    ChildProcessManager &childProcessManager = ChildProcessManager::GetInstance();
    HapModuleInfo hapModuleInfo;
    BundleInfo bundleInfoCopy = *bundleInfo_;
    if (!childProcessManager.GetEntryHapModuleInfo(bundleInfoCopy, hapModuleInfo)) {
        TAG_LOGE(AAFwkTag::APPKIT, "GetEntryHapModuleInfo failed");
        return;
    }

    runtime_ = childProcessManager.CreateRuntime(bundleInfoCopy, hapModuleInfo, true, processInfo_->jitEnabled);
    if (!runtime_) {
        TAG_LOGE(AAFwkTag::APPKIT, "create child process runtime failed");
        return;
    }
    AbilityRuntime::Runtime::DebugOption debugOption;
    childProcessManager.SetAppSpawnForkDebugOption(debugOption, processInfo_);
    TAG_LOGD(AAFwkTag::APPKIT, "StartDebugMode, isStartWithDebug is %{public}d, processName is %{public}s, "
        "isDebugApp is %{public}d, isStartWithNative is %{public}d", processInfo_->isStartWithDebug,
        processInfo_->processName.c_str(), processInfo_->isDebugApp, processInfo_->isStartWithNative);
    runtime_->StartDebugMode(debugOption);
    std::string srcPath;
    srcPath.append(hapModuleInfo.moduleName).append("/").append(processInfo_->srcEntry);
    childProcessManager.LoadJsFile(srcPath, hapModuleInfo, runtime_);
    ExitProcessSafely();
}

void ChildMainThread::HandleLoadArkTs()
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (!processInfo_ || !bundleInfo_) {
        TAG_LOGE(AAFwkTag::APPKIT, "null processInfo or bundleInfo_");
        return;
    }
    if (!processArgs_) {
        TAG_LOGE(AAFwkTag::APPKIT, "null processArgs_");
        return;
    }
    auto &srcEntry = processInfo_->srcEntry;
    ChildProcessManager &childProcessManager = ChildProcessManager::GetInstance();
    std::string moduleName = childProcessManager.GetModuleNameFromSrcEntry(srcEntry);
    if (moduleName.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "not find module name from srcEntry, srcEntry: %{private}s", srcEntry.c_str());
        return;
    }
    HapModuleInfo hapModuleInfo;
    if (!childProcessManager.GetHapModuleInfo(*bundleInfo_, moduleName, hapModuleInfo)) {
        TAG_LOGE(AAFwkTag::APPKIT, "GetHapModuleInfo failed, not find module:%{public}s", moduleName.c_str());
        return;
    }

    runtime_ = childProcessManager.CreateRuntime(*bundleInfo_, hapModuleInfo, true, processInfo_->jitEnabled);
    if (!runtime_) {
        TAG_LOGE(AAFwkTag::APPKIT, "create child process runtime failed");
        return;
    }
    AbilityRuntime::Runtime::DebugOption debugOption;
    childProcessManager.SetAppSpawnForkDebugOption(debugOption, processInfo_);
    TAG_LOGD(AAFwkTag::APPKIT, "StartDebugMode, isStartWithDebug is %{public}d, processName is %{public}s, "
        "isDebugApp is %{public}d, isStartWithNative is %{public}d", processInfo_->isStartWithDebug,
        processInfo_->processName.c_str(), processInfo_->isDebugApp, processInfo_->isStartWithNative);
    runtime_->StartDebugMode(debugOption);

    childProcessManager.LoadJsFile(srcEntry, hapModuleInfo, runtime_, processArgs_);
}

void ChildMainThread::HandleLoadNative()
{
    TAG_LOGD(AAFwkTag::APPKIT, "HandleLoadNative called.");
    if (!processInfo_) {
        TAG_LOGE(AAFwkTag::APPKIT, "null processInfo_");
        return;
    }
    if (!processArgs_) {
        TAG_LOGE(AAFwkTag::APPKIT, "null processArgs_");
        return;
    }
    ChildProcessManager &childProcessMgr = ChildProcessManager::GetInstance();
    childProcessMgr.LoadNativeLibWithArgs(nativeLibModuleName_, processInfo_->srcEntry, processInfo_->entryFunc,
        processArgs_);
    TAG_LOGD(AAFwkTag::APPKIT, "HandleLoadNative end.");
    ExitProcessSafely();
}

void ChildMainThread::InitNativeLib(const BundleInfo &bundleInfo)
{
    HspList hspList;
    ErrCode ret = DelayedSingleton<BundleMgrHelper>::GetInstance()->GetBaseSharedBundleInfos(bundleInfo.name, hspList,
        AppExecFwk::GetDependentBundleInfoFlag::GET_ALL_DEPENDENT_BUNDLE_INFO);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Get base shared bundle infos failed: %{public}d", ret);
    }

    AppLibPathMap appLibPaths {};
    GetNativeLibPath(bundleInfo, hspList, appLibPaths);
    bool isSystemApp = bundleInfo.applicationInfo.isSystemApp;
    TAG_LOGD(AAFwkTag::APPKIT, "the application isSystemApp: %{public}d", isSystemApp);

    if (processInfo_->childProcessType != CHILD_PROCESS_TYPE_NATIVE &&
        processInfo_->childProcessType != CHILD_PROCESS_TYPE_NATIVE_ARGS) {
        AbilityRuntime::JsRuntime::SetAppLibPath(appLibPaths, isSystemApp);
    } else {
        UpdateNativeChildLibModuleName(appLibPaths, isSystemApp);
    }
}

void ChildMainThread::ExitProcessSafely()
{
    if (appMgr_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null appMgr_, use exit(0) instead");
        exit(0);
        return;
    }
    appMgr_->ExitChildProcessSafely();
}

bool ChildMainThread::ScheduleExitProcessSafely()
{
    TAG_LOGD(AAFwkTag::APPKIT, "ScheduleExitProcessSafely");
    if (mainHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null mainHandler_");
        return false;
    }
    wptr<ChildMainThread> weak = this;
    auto task = [weak]() {
        auto childMainThread = weak.promote();
        if (childMainThread == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null childMainThread, ScheduleExitProcessSafely failed");
            return;
        }
        childMainThread->HandleExitProcessSafely();
    };
    if (!mainHandler_->PostTask(task, "ChildMainThread::HandleExitProcessSafely")) {
        TAG_LOGE(AAFwkTag::APPKIT, "ScheduleExitProcessSafely PostTask task failed");
        return false;
    }
    return true;
}

void ChildMainThread::HandleExitProcessSafely()
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    std::shared_ptr<EventRunner> runner = mainHandler_->GetEventRunner();
    if (runner == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "HandleExitProcessSafely get runner error");
        return;
    }
    int ret = runner->Stop();
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "HandleExitProcessSafely failed. runner->Run failed ret = %{public}d", ret);
    }
}

bool ChildMainThread::ScheduleRunNativeProc(const sptr<IRemoteObject> &mainProcessCb)
{
    TAG_LOGD(AAFwkTag::APPKIT, "ScheduleRunNativeProc");
    if (mainProcessCb == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null Main process callback");
        return false;
    }

    if (mainHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null mainHandler_");
        return false;
    }

    auto task = [weak = wptr<ChildMainThread>(this), callback = sptr<IRemoteObject>(mainProcessCb)]() {
        auto childMainThread = weak.promote();
        if (childMainThread == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null childMainThread, ScheduleRunNativeProc failed");
            return;
        }
        childMainThread->HandleRunNativeProc(callback);
    };
    if (!mainHandler_->PostTask(task, "ChildMainThread::HandleRunNativeProc")) {
        TAG_LOGE(AAFwkTag::APPKIT, "HandleRunNativeProc PostTask task failed");
        return false;
    }
    return true;
}

void ChildMainThread::HandleRunNativeProc(const sptr<IRemoteObject> &mainProcessCb)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (!processInfo_) {
        TAG_LOGE(AAFwkTag::APPKIT, "null processInfo");
        return;
    }

    ChildProcessManager &childProcessMgr = ChildProcessManager::GetInstance();
    childProcessMgr.LoadNativeLib(nativeLibModuleName_, processInfo_->srcEntry, mainProcessCb);
    ExitProcessSafely();
}

void ChildMainThread::UpdateNativeChildLibModuleName(const AppLibPathMap &appLibPaths, bool isSystemApp)
{
    nativeLibModuleName_.clear();
    NativeModuleManager *nativeModuleMgr = NativeModuleManager::GetInstance();
    if (nativeModuleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Get native module manager for native child failed");
        return;
    }

    std::string nativeLibPath;
    for (const auto &libPathPair : appLibPaths) {
        for (const auto &libDir : libPathPair.second) {
            nativeLibPath = libDir;
            if (!nativeLibPath.empty() && nativeLibPath.back() != '/') {
                nativeLibPath += '/';
            }
            nativeLibPath += processInfo_->srcEntry;
            if (access(nativeLibPath.c_str(), F_OK) == 0) {
                nativeLibModuleName_ = libPathPair.first;
                nativeModuleMgr->SetAppLibPath(libPathPair.first, libPathPair.second, isSystemApp);
                TAG_LOGI(AAFwkTag::APPKIT, "Find native lib in app module: %{public}s", libPathPair.first.c_str());
                return;
            }
        }
    }

    TAG_LOGE(AAFwkTag::APPKIT, "not find native lib(%{private}s) in any app module",
        processInfo_->srcEntry.c_str());
}

void ChildMainThread::GetNativeLibPath(const BundleInfo &bundleInfo, const HspList &hspList,
    AppLibPathMap &appLibPaths)
{
    std::string nativeLibraryPath = bundleInfo.applicationInfo.nativeLibraryPath;
    if (!nativeLibraryPath.empty()) {
        if (nativeLibraryPath.back() == '/') {
            nativeLibraryPath.pop_back();
        }
        std::string libPath = LOCAL_CODE_PATH;
        libPath += (libPath.back() == '/') ? nativeLibraryPath : "/" + nativeLibraryPath;
        TAG_LOGI(AAFwkTag::APPKIT, "napi lib path = %{private}s", libPath.c_str());
        appLibPaths["default"].emplace_back(libPath);
    }

    for (auto &hapInfo : bundleInfo.hapModuleInfos) {
        TAG_LOGD(AAFwkTag::APPKIT,
            "moduleName: %{public}s, isLibIsolated: %{public}d, compressNativeLibs: %{public}d",
            hapInfo.moduleName.c_str(), hapInfo.isLibIsolated, hapInfo.compressNativeLibs);
        GetHapSoPath(hapInfo, appLibPaths, hapInfo.hapPath.find(ABS_CODE_PATH));
    }

    for (auto &hspInfo : hspList) {
        TAG_LOGD(AAFwkTag::APPKIT, "bundle:%s, module:%s, nativeLibraryPath:%s", hspInfo.bundleName.c_str(),
            hspInfo.moduleName.c_str(), hspInfo.nativeLibraryPath.c_str());
        GetHspNativeLibPath(hspInfo, appLibPaths, hspInfo.hapPath.find(ABS_CODE_PATH) != 0u);
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
