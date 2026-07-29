/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_QUICK_FIX_MANAGER_APPLY_TASK_H
#define OHOS_ABILITY_RUNTIME_QUICK_FIX_MANAGER_APPLY_TASK_H

#include "app_mgr_interface.h"
#include "event_handler.h"
#include "quick_fix_result_info.h"
#include "quick_fix/quick_fix_manager_interface.h"

namespace OHOS {
namespace AAFwk {
class QuickFixManagerService;
class QuickFixManagerApplyTask : public std::enable_shared_from_this<QuickFixManagerApplyTask> {
public:
    QuickFixManagerApplyTask(sptr<AppExecFwk::IQuickFixManager> bundleQfMgr, sptr<AppExecFwk::IAppMgr> appMgr,
        std::shared_ptr<AppExecFwk::EventHandler> handler, wptr<QuickFixManagerService> service)
        : bundleQfMgr_(bundleQfMgr), appMgr_(appMgr), eventHandler_(handler), quickFixMgrService_(service)
    {}

    virtual ~QuickFixManagerApplyTask();

    enum TaskType {
        QUICK_FIX_APPLY,
        QUICK_FIX_REVOKE,
    };

    void Run(const std::vector<std::string> &quickFixFiles, bool isDebug = false, bool isReplace = false,
        bool isCheckDebugApp = false);
    void HandlePatchDeployed();
    void HandlePatchSwitched();
    void HandlePatchDeleted();

    bool SetQuickFixInfo(const std::shared_ptr<AppExecFwk::QuickFixResult> &result);
    bool ExtractQuickFixDataFromJson(nlohmann::json& resultJson);
    bool GetRunningState();

    void RemoveTimeoutTask();
    void NotifyApplyStatus(int32_t resultCode);
    void RemoveSelf();

    void PostSwitchQuickFixTask();
    void PostDeleteQuickFixTask();

    void UnregAppStateObserver();

    void RunRevoke();
    void InitRevokeTask(const std::string &bundleName, bool isSoContained);
    std::string GetBundleName();
    TaskType GetTaskType();
    void HandleRevokePatchDeleted();
    void HandleRevokePatchSwitched();
    void PostRevokeQuickFixDeleteTask();
    void PostRevokeQuickFixProcessDiedTask();
private:
    void PostDeployQuickFixTask(const std::vector<std::string> &quickFixFiles, bool isDebug = false,
        bool isReplace = false, bool isCheckDebugApp = false);
    void PostTimeOutTask();
    void PostNotifyLoadRepairPatchTask();
    void PostNotifyUnloadRepairPatchTask();
    void PostNotifyHotReloadPageTask();
    void RegAppStateObserver();
    void PostRevokeQuickFixTask();
    void HandleRevokeQuickFixAppRunning();
    void PostRevokeQuickFixNotifyUnloadPatchTask();
    void HandleRevokeQuickFixAppStop();

    sptr<AppExecFwk::IQuickFixManager> bundleQfMgr_ = nullptr;
    sptr<AppExecFwk::IAppMgr> appMgr_ = nullptr;
    sptr<AppExecFwk::IApplicationStateObserver> appStateCallback_ = nullptr;
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_ = nullptr;
    wptr<QuickFixManagerService> quickFixMgrService_ = nullptr;
    std::string bundleName_;
    std::vector<std::string> moduleNames_;
    AppExecFwk::QuickFixType type_ = AppExecFwk::QuickFixType::UNKNOWN;
    TaskType taskType_ = TaskType::QUICK_FIX_APPLY;
    int bundleVersionCode_ = 0;
    int patchVersionCode_ = 0;
    bool isRunning_ = false;
    bool isSoContained_ = false;
    int32_t callerUid_ = 0;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_QUICK_FIX_MANAGER_APPLY_TASK_H
