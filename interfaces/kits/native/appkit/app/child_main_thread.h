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

#ifndef OHOS_ABILITY_RUNTIME_CHILD_MAIN_THREAD_H
#define OHOS_ABILITY_RUNTIME_CHILD_MAIN_THREAD_H

#include <string>
#include <vector>
#include <memory>

#include "app_mgr_interface.h"
#include "base_shared_bundle_info.h"
#include "bundle_info.h"
#include "bundle_mgr_interface.h"
#include "child_scheduler_interface.h"
#include "child_scheduler_stub.h"
#ifdef SUPPORT_CHILD_PROCESS
#include "child_process_info.h"
#endif // SUPPORT_CHILD_PROCESS
#include "event_handler.h"
#include "ipc_singleton.h"
#include "js_runtime.h"

namespace OHOS {
namespace AppExecFwk {
using HspList = std::vector<BaseSharedBundleInfo>;
class ChildMainThread : public ChildSchedulerStub {
    DECLARE_DELAYED_IPCSINGLETON(ChildMainThread);

public:
    static void Start(const std::map<std::string, int32_t> &fds);
    void SetFds(const std::map<std::string, int32_t> &fds);
    bool ScheduleLoadChild() override;
    bool ScheduleExitProcessSafely() override;
    bool ScheduleRunNativeProc(const sptr<IRemoteObject> &mainProcessCb) override;
    void OnLoadAbilityFinished(uint64_t callbackId, int32_t pid) override;

private:
    static int32_t GetChildProcessInfo(ChildProcessInfo &info);
    bool Init(const std::shared_ptr<EventRunner> &runner, const ChildProcessInfo &processInfo);
    bool Attach();
    void HandleLoadJs();
    void HandleLoadArkTs();
    void HandleLoadNative();
    void InitNativeLib(const BundleInfo &bundleInfo, const HspList hspList);
    void HandleExitProcessSafely();
    void ExitProcessSafely();
    void GetNativeLibPath(const BundleInfo &bundleInfo, const HspList &hspList, AppLibPathMap &appLibPaths);
    void HandleRunNativeProc(const sptr<IRemoteObject> &mainProcessCb);
    void UpdateNativeChildLibModuleName(const AppLibPathMap &appLibPaths, bool isSystemApp);
    std::string ParsePluginDefaultNamespaceLdDictionary();
    bool IsEtsAPP(const ChildProcessInfo &processInfo);

    sptr<IAppMgr> appMgr_ = nullptr;
    std::shared_ptr<EventHandler> mainHandler_ = nullptr;
    std::shared_ptr<BundleInfo> bundleInfo_ = nullptr;
    std::shared_ptr<ChildProcessInfo> processInfo_ = nullptr;
    std::unique_ptr<AbilityRuntime::Runtime> runtime_ = nullptr;
    std::string nativeLibModuleName_;
    std::string pluginDefaultNamespaceLdDictionary_ = "";
#ifdef SUPPORT_CHILD_PROCESS
    std::shared_ptr<ChildProcessArgs> processArgs_ = nullptr;
#endif // SUPPORT_CHILD_PROCESS

    DISALLOW_COPY_AND_MOVE(ChildMainThread);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_CHILD_MAIN_THREAD_H
