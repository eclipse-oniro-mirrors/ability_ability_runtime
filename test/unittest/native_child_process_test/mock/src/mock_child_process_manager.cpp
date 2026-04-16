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

#include "mock_child_process_manager.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
    bool g_jitEnabled = false;
    AbilityRuntime::Runtime::DebugOption g_debugOption;
    const std::string LARGE_SCREEN = "large_screen";
}
bool ChildProcessManager::signalRegistered_ = false;

ChildProcessManager &ChildProcessManager::GetInstance()
{
    static ChildProcessManager instance;
    return instance;
}

ChildProcessManager::ChildProcessManager()
{
}

ChildProcessManager::~ChildProcessManager()
{
}

ChildProcessManagerErrorCode ChildProcessManager::StartChildProcessBySelfFork(
    const std::string &srcEntry, pid_t &pid, bool isStaticChildProcess)
{
    return ChildProcessManagerErrorCode::ERR_OK;
}

bool ChildProcessManager::IsMultiProcessFeatureApp(const AppExecFwk::BundleInfo &bundleInfo)
{
    return false;
}

ChildProcessManagerErrorCode ChildProcessManager::StartChildProcessByAppSpawnFork(
    const std::string &srcEntry, pid_t &pid, bool isStaticChildProcess)
{
    return ChildProcessManagerErrorCode::ERR_ALREADY_IN_CHILD_PROCESS;
}

ChildProcessManagerErrorCode ChildProcessManager::StartChildProcessWithArgs(
    const std::string &srcEntry, pid_t &pid, int32_t childProcessType, const AppExecFwk::ChildProcessArgs &args,
    const AppExecFwk::ChildProcessOptions &options, bool isStaticChildProcess)
{
    return startErrorCode_;
}

ChildProcessManagerErrorCode ChildProcessManager::CreateNativeChildProcessByAppSpawnFork(
    const std::string &libName, const sptr<IRemoteObject> &callbackStub, const std::string &customProcessName,
    const bool isolationMode, const bool isIsolationUid)
{
    return ChildProcessManagerErrorCode::ERR_OK;
}

void ChildProcessManager::RegisterSignal()
{
}

void ChildProcessManager::HandleSigChild(int32_t signo)
{
}

bool ChildProcessManager::AllowChildProcessOnDevice()
{
    return false;
}

ChildProcessManagerErrorCode ChildProcessManager::PreCheckSelfFork()
{
    return ChildProcessManagerErrorCode::ERR_OK;
}

ChildProcessManagerErrorCode ChildProcessManager::PreCheck(int32_t childProcessType)
{
    return ChildProcessManagerErrorCode::ERR_OK;
}

bool ChildProcessManager::IsChildProcess()
{
    return false;
}

bool ChildProcessManager::IsChildProcessBySelfFork()
{
    return isChildProcessBySelfFork_;
}

void ChildProcessManager::HandleChildProcessBySelfFork(const std::string &srcEntry,
    const AppExecFwk::BundleInfo &bundleInfo, bool isStaticChildProcess)
{
}

bool ChildProcessManager::LoadJsFile(const std::string &srcEntry, const AppExecFwk::HapModuleInfo &hapModuleInfo,
    const std::unique_ptr<AbilityRuntime::Runtime> &runtime, std::shared_ptr<AppExecFwk::ChildProcessArgs> args)
{
    return true;
}

bool ChildProcessManager::LoadNativeLib(const std::string &moduleName,
    const std::string &libPath, const sptr<IRemoteObject> &mainProcessCb)
{
    return true;
}

bool ChildProcessManager::LoadNativeLibWithArgs(const std::string &moduleName, const std::string &srcEntry,
    const std::string &entryFunc, std::shared_ptr<AppExecFwk::ChildProcessArgs> args)
{
    return true;
}

std::unique_ptr<AbilityRuntime::Runtime> ChildProcessManager::CreateRuntime(const AppExecFwk::BundleInfo &bundleInfo,
    const AppExecFwk::HapModuleInfo &hapModuleInfo, const bool fromAppSpawn, const bool jitEnabled,
    bool isStaticChildProcess)
{
    return nullptr;
}

bool ChildProcessManager::GetBundleInfo(AppExecFwk::BundleInfo &bundleInfo)
{
    return false;
}

bool ChildProcessManager::GetEntryHapModuleInfo(const AppExecFwk::BundleInfo &bundleInfo,
    AppExecFwk::HapModuleInfo &hapModuleInfo)
{
    return false;
}

bool ChildProcessManager::GetHapModuleInfo(const AppExecFwk::BundleInfo &bundleInfo, const std::string &moduleName,
    AppExecFwk::HapModuleInfo &hapModuleInfo)
{
    return false;
}

bool ChildProcessManager::HasChildProcessRecord()
{
    return false;
}

sptr<AppExecFwk::IAppMgr> ChildProcessManager::GetAppMgr()
{
    return nullptr;
}

void ChildProcessManager::SetForkProcessJITEnabled(bool jitEnabled)
{
    g_jitEnabled = jitEnabled;
}

void ChildProcessManager::SetForkProcessDebugOption(const std::string bundleName, const bool isStartWithDebug,
    const bool isDebugApp, const bool isStartWithNative)
{
}

void ChildProcessManager::SetAppSpawnForkDebugOption(Runtime::DebugOption &debugOption,
    std::shared_ptr<AppExecFwk::ChildProcessInfo> processInfo)
{
}

void ChildProcessManager::MakeProcessName(const std::string &srcEntry)
{
}

std::string ChildProcessManager::GetModuleNameFromSrcEntry(const std::string &srcEntry)
{
    return "";
}
}  // namespace AbilityRuntime
}  // namespace OHOS