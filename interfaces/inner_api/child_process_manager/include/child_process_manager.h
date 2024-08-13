/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_CHILD_PROCESS_MANAGER_H
#define OHOS_ABILITY_RUNTIME_CHILD_PROCESS_MANAGER_H

#include <mutex>
#include <string>
#include <sys/types.h>

#include "app_mgr_interface.h"
#include "bundle_info.h"
#include "child_process_args.h"
#include "child_process_info.h"
#include "child_process_manager_error_utils.h"
#include "child_process_options.h"
#include "hap_module_info.h"
#include "runtime.h"
#include "iremote_object.h"

namespace OHOS {
namespace AbilityRuntime {
class ChildProcessManager {
public:
    static ChildProcessManager &GetInstance();
    ~ChildProcessManager();

    static void HandleSigChild(int32_t signo);
    bool IsChildProcess();
    ChildProcessManagerErrorCode StartChildProcessBySelfFork(const std::string &srcEntry, pid_t &pid);
    ChildProcessManagerErrorCode StartChildProcessByAppSpawnFork(const std::string &srcEntry, pid_t &pid);
    ChildProcessManagerErrorCode StartArkChildProcess(const std::string &srcEntry, pid_t &pid, int32_t childProcessType,
        const AppExecFwk::ChildProcessArgs &args, const AppExecFwk::ChildProcessOptions &options);
    ChildProcessManagerErrorCode StartNativeChildProcessByAppSpawnFork(
        const std::string &libName, const sptr<IRemoteObject> &callbackStub);
    bool GetBundleInfo(AppExecFwk::BundleInfo &bundleInfo);
    bool GetEntryHapModuleInfo(const AppExecFwk::BundleInfo &bundleInfo, AppExecFwk::HapModuleInfo &hapModuleInfo);
    bool GetHapModuleInfo(const AppExecFwk::BundleInfo &bundleInfo, const std::string &moduleName,
        AppExecFwk::HapModuleInfo &hapModuleInfo);
    std::unique_ptr<AbilityRuntime::Runtime> CreateRuntime(const AppExecFwk::BundleInfo &bundleInfo,
        const AppExecFwk::HapModuleInfo &hapModuleInfo, const bool fromAppSpawn, const bool jitEnabled);
    bool LoadJsFile(const std::string &srcEntry, const AppExecFwk::HapModuleInfo &hapModuleInfo,
        std::unique_ptr<AbilityRuntime::Runtime> &runtime,
        std::shared_ptr<AppExecFwk::ChildProcessArgs> args = nullptr);
    bool LoadNativeLib(const std::string &moduleName, const std::string &libPath,
        const sptr<IRemoteObject> &mainProcessCb);
    void SetForkProcessJITEnabled(bool jitEnabled);
    void SetForkProcessDebugOption(const std::string bundleName, const bool isStartWithDebug, const bool isDebugApp,
        const bool isStartWithNative);
    void SetAppSpawnForkDebugOption(Runtime::DebugOption &debugOption,
        std::shared_ptr<AppExecFwk::ChildProcessInfo> processInfo);
    std::string GetModuleNameFromSrcEntry(const std::string &srcEntry);

private:
    ChildProcessManager();

    ChildProcessManagerErrorCode PreCheck(bool useNewErrorCode = false);
    ChildProcessManagerErrorCode PreCheckNativeProcess();
    void RegisterSignal();
    void HandleChildProcessBySelfFork(const std::string &srcEntry, const AppExecFwk::BundleInfo &bundleInfo);
    bool HasChildProcessRecord();
    sptr<AppExecFwk::IAppMgr> GetAppMgr();
    void MakeProcessName(const std::string &srcEntry);

    static bool signalRegistered_;
    bool isChildProcessBySelfFork_ = false;
    int32_t childProcessCount_ = 0;
    std::mutex childProcessCountLock_;

    DISALLOW_COPY_AND_MOVE(ChildProcessManager);
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_CHILD_PROCESS_MANAGER_H
