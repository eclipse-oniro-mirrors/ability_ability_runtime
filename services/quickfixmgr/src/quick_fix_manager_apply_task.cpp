/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "quick_fix_manager_apply_task.h"

#include "application_state_observer_stub.h"
#include "common_event_data.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "quick_fix_callback_stub.h"
#include "quick_fix_error_utils.h"
#include "quick_fix_manager_service.h"
#include "quick_fix/quick_fix_status_callback_host.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
namespace {
// same with quick_fix_result_info
constexpr const char *QUICK_FIX_BUNDLE_NAME = "bundleName";
constexpr const char *QUICK_FIX_BUNDLE_VERSION_CODE = "bundleVersionCode";
constexpr const char *QUICK_FIX_PATCH_VERSION_CODE = "patchVersionCode";
constexpr const char *QUICK_FIX_IS_SO_CONTAINED = "isSoContained";
constexpr const char *QUICK_FIX_TYPE = "type";
constexpr const char *QUICK_FIX_MODULE_NAME = "moduleNames";

// common event key
constexpr const char *APPLY_RESULT = "applyResult";
constexpr const char *APPLY_RESULT_INFO = "applyResultInfo";
constexpr const char *REVOKE_RESULT = "revokeResult";
constexpr const char *REVOKE_RESULT_INFO = "revokeResultInfo";
constexpr const char *BUNDLE_NAME = "bundleName";
constexpr const char *BUNDLE_VERSION = "bundleVersion";
constexpr const char *PATCH_VERSION = "patchVersion";

// timeout task
constexpr const char *TIMEOUT_TASK_NAME = "timeoutTask";
constexpr int64_t TIMEOUT_TASK_DELAY_TIME = 3 * 60 * 1000;
} // namespace

class QuickFixManagerStatusCallback : public AppExecFwk::QuickFixStatusCallbackHost {
public:
    explicit QuickFixManagerStatusCallback(std::shared_ptr<QuickFixManagerApplyTask> applyTask)
        : applyTask_(applyTask)
    {}

    virtual ~QuickFixManagerStatusCallback()
    {
        TAG_LOGD(AAFwkTag::QUICKFIX, "destroyed");
    }

    void OnPatchDeployed(const std::shared_ptr<AppExecFwk::QuickFixResult> &result) override
    {
        TAG_LOGD(AAFwkTag::QUICKFIX, "called");
        if (applyTask_ == nullptr) {
            TAG_LOGE(AAFwkTag::QUICKFIX, "null Apply task, result:%{public}s", result->ToString().c_str());
            return;
        }

        int32_t ret = QUICK_FIX_OK;
        do {
            if (result->GetResCode() != 0) {
                TAG_LOGE(AAFwkTag::QUICKFIX, "failed, result:%{public}s",
                    result->ToString().c_str());
                ret = QUICK_FIX_DEPLOY_FAILED;
                break;
            }

            if (!applyTask_->SetQuickFixInfo(result)) {
                TAG_LOGE(AAFwkTag::QUICKFIX, "set quickFixInfo failed");
                ret = QUICK_FIX_SET_INFO_FAILED;
                break;
            }

            applyTask_->HandlePatchDeployed();
        } while (0);

        if (ret != QUICK_FIX_OK) {
            applyTask_->NotifyApplyStatus(ret);
            applyTask_->RemoveSelf();
        }
        applyTask_->RemoveTimeoutTask();
    }

    void OnPatchSwitched(const std::shared_ptr<AppExecFwk::QuickFixResult> &result) override
    {
        TAG_LOGD(AAFwkTag::QUICKFIX, "called");
        if (applyTask_ == nullptr) {
            TAG_LOGE(AAFwkTag::QUICKFIX, "null apply task, result: %{public}s", result->ToString().c_str());
            return;
        }

        int32_t ret = QUICK_FIX_OK;
        do {
            if (result->GetResCode() != 0) {
                TAG_LOGE(AAFwkTag::QUICKFIX, "switch quickFix failed, result: %{public}s",
                    result->ToString().c_str());
                ret = QUICK_FIX_SWICH_FAILED;
                break;
            }

            if (applyTask_->GetTaskType() == QuickFixManagerApplyTask::TaskType::QUICK_FIX_APPLY) {
                applyTask_->HandlePatchSwitched();
                break;
            } else if (applyTask_->GetTaskType() == QuickFixManagerApplyTask::TaskType::QUICK_FIX_REVOKE) {
                applyTask_->HandleRevokePatchSwitched();
                break;
            }

            ret = QUICK_FIX_SWICH_FAILED;
            TAG_LOGE(AAFwkTag::QUICKFIX, "switch quickFix invalid task type");
        } while (0);

        if (ret != QUICK_FIX_OK) {
            applyTask_->NotifyApplyStatus(ret);
            applyTask_->RemoveSelf();
        }
        applyTask_->RemoveTimeoutTask();
    }

    void OnPatchDeleted(const std::shared_ptr<AppExecFwk::QuickFixResult> &result) override
    {
        TAG_LOGD(AAFwkTag::QUICKFIX, "called");
        if (applyTask_ == nullptr) {
            TAG_LOGE(AAFwkTag::QUICKFIX, "null apply task, result: %{public}s", result->ToString().c_str());
            return;
        }

        int32_t ret = QUICK_FIX_OK;
        do {
            if (result->GetResCode() != 0) {
                TAG_LOGE(AAFwkTag::QUICKFIX, "delete quickFix failed, result: %{public}s",
                    result->ToString().c_str());
                ret = QUICK_FIX_DELETE_FAILED;
                break;
            }

            if (applyTask_->GetTaskType() == QuickFixManagerApplyTask::TaskType::QUICK_FIX_APPLY) {
                applyTask_->HandlePatchDeleted();
                break;
            } else if (applyTask_->GetTaskType() == QuickFixManagerApplyTask::TaskType::QUICK_FIX_REVOKE) {
                applyTask_->HandleRevokePatchDeleted();
                break;
            }

            ret = QUICK_FIX_DELETE_FAILED;
            TAG_LOGE(AAFwkTag::QUICKFIX, "delete quickFix invalid task type");
        } while (0);

        if (ret != QUICK_FIX_OK) {
            applyTask_->NotifyApplyStatus(ret);
            applyTask_->RemoveSelf();
        }
        applyTask_->RemoveTimeoutTask();
    }

private:
    std::shared_ptr<QuickFixManagerApplyTask> applyTask_;
};

class RevokeQuickFixTaskCallback : public QuickFixManagerStatusCallback {
public:
    explicit RevokeQuickFixTaskCallback(std::shared_ptr<QuickFixManagerApplyTask> applyTask)
        : QuickFixManagerStatusCallback(applyTask)
    {}
    virtual ~RevokeQuickFixTaskCallback() = default;

    void OnPatchDeployed(const std::shared_ptr<AppExecFwk::QuickFixResult> &result) override
    {
        TAG_LOGD(AAFwkTag::QUICKFIX, "called");
    }
};

class QuickFixMgrAppStateObserver : public AppExecFwk::ApplicationStateObserverStub {
public:
    explicit QuickFixMgrAppStateObserver(std::shared_ptr<QuickFixManagerApplyTask> applyTask)
        : applyTask_(applyTask)
    {}

    virtual ~QuickFixMgrAppStateObserver()
    {
        TAG_LOGD(AAFwkTag::QUICKFIX, "destroyed");
    }

    void OnProcessDied(const AppExecFwk::ProcessData &processData) override
    {
        TAG_LOGI(AAFwkTag::QUICKFIX, "process died, bundle name: %{public}s", processData.bundleName.c_str());

        if (applyTask_ == nullptr) {
            TAG_LOGE(AAFwkTag::QUICKFIX, "null apply task, bundle name:%{public}s",
                processData.bundleName.c_str());
            return;
        }

        bool isRunning = applyTask_->GetRunningState();
        if (!isRunning) {
            if (applyTask_->GetTaskType() == QuickFixManagerApplyTask::TaskType::QUICK_FIX_APPLY) {
                applyTask_->HandlePatchDeployed();
            } else if (applyTask_->GetTaskType() == QuickFixManagerApplyTask::TaskType::QUICK_FIX_REVOKE) {
                applyTask_->PostRevokeQuickFixProcessDiedTask();
            } else {
                TAG_LOGW(AAFwkTag::QUICKFIX, "Invalid task type");
            }
        }

        applyTask_->UnregAppStateObserver();
    }

private:
    std::shared_ptr<QuickFixManagerApplyTask> applyTask_;
};

class QuickFixNotifyCallback : public AppExecFwk::QuickFixCallbackStub {
public:
    explicit QuickFixNotifyCallback(std::shared_ptr<QuickFixManagerApplyTask> applyTask)
        : applyTask_(applyTask)
    {}

    virtual ~QuickFixNotifyCallback()
    {
        TAG_LOGD(AAFwkTag::QUICKFIX, "destroyed");
    }

    void OnLoadPatchDone(int32_t resultCode, [[maybe_unused]] int32_t recordId) override
    {
        TAG_LOGD(AAFwkTag::QUICKFIX, "called");
        if (resultCode != 0) {
            TAG_LOGE(AAFwkTag::QUICKFIX, "notify patch failed: %{public}d", resultCode);
            applyTask_->NotifyApplyStatus(QUICK_FIX_NOTIFY_LOAD_PATCH_FAILED);
            applyTask_->RemoveSelf();
            return;
        }

        applyTask_->PostDeleteQuickFixTask();
    }

    void OnUnloadPatchDone(int32_t resultCode, [[maybe_unused]] int32_t recordId) override
    {
        TAG_LOGD(AAFwkTag::QUICKFIX, "called");
        if (resultCode != 0) {
            TAG_LOGE(AAFwkTag::QUICKFIX, "notify patch failed: %{public}d", resultCode);
            applyTask_->NotifyApplyStatus(QUICK_FIX_NOTIFY_UNLOAD_PATCH_FAILED);
            applyTask_->RemoveSelf();
            return;
        }

        if (applyTask_->GetTaskType() == QuickFixManagerApplyTask::TaskType::QUICK_FIX_APPLY) {
            applyTask_->PostSwitchQuickFixTask();
            return;
        } else if (applyTask_->GetTaskType() == QuickFixManagerApplyTask::TaskType::QUICK_FIX_REVOKE) {
            applyTask_->PostRevokeQuickFixDeleteTask();
            return;
        }

        TAG_LOGW(AAFwkTag::QUICKFIX, "Invalid task type");
    }

    void OnReloadPageDone(int32_t resultCode, [[maybe_unused]] int32_t recordId) override
    {
        TAG_LOGD(AAFwkTag::QUICKFIX, "called");
        if (resultCode != 0) {
            TAG_LOGE(AAFwkTag::QUICKFIX, "notify app load patch failed: %{public}d", resultCode);
            applyTask_->NotifyApplyStatus(QUICK_FIX_NOTIFY_RELOAD_PAGE_FAILED);
            applyTask_->RemoveSelf();
            return;
        }

        applyTask_->NotifyApplyStatus(QUICK_FIX_OK);
        applyTask_->RemoveSelf();
    }

private:
    std::shared_ptr<QuickFixManagerApplyTask> applyTask_;
};

class RevokeQuickFixNotifyCallback : public QuickFixNotifyCallback {
public:
    explicit RevokeQuickFixNotifyCallback(std::shared_ptr<QuickFixManagerApplyTask> applyTask)
        : QuickFixNotifyCallback(applyTask)
    {}

    virtual ~RevokeQuickFixNotifyCallback() = default;

    void OnLoadPatchDone(int32_t resultCode, [[maybe_unused]] int32_t recordId) override
    {}

    void OnReloadPageDone(int32_t resultCode, [[maybe_unused]] int32_t recordId) override
    {}
};

QuickFixManagerApplyTask::~QuickFixManagerApplyTask()
{
    TAG_LOGD(AAFwkTag::QUICKFIX, "destroyed");
}

void QuickFixManagerApplyTask::Run(const std::vector<std::string> &quickFixFiles, bool isDebug, bool isReplace,
    bool isCheckDebugApp)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::QUICKFIX, "Run apply task");
    taskType_ = TaskType::QUICK_FIX_APPLY;
    PostDeployQuickFixTask(quickFixFiles, isDebug, isReplace, isCheckDebugApp);
}

void QuickFixManagerApplyTask::RunRevoke()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::QUICKFIX, "Run apply revoke task");
    taskType_ = TaskType::QUICK_FIX_REVOKE;
    PostRevokeQuickFixTask();
}

void QuickFixManagerApplyTask::InitRevokeTask(const std::string &bundleName, bool isSoContained)
{
    isSoContained_ = isSoContained;
    bundleName_ = bundleName;
    TAG_LOGI(AAFwkTag::QUICKFIX, "call func:%{public}s, isSoContained:%{public}s", bundleName_.c_str(),
        isSoContained_ ? "true" : "false");
}

void QuickFixManagerApplyTask::HandlePatchDeployed()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::QUICKFIX, "called");

    isRunning_ = GetRunningState();
    if (isRunning_ && isSoContained_) {
        return RegAppStateObserver();
    } else if (isRunning_ && !isSoContained_) {
        ApplicationQuickFixInfo quickFixInfo;
        auto service = quickFixMgrService_.promote();
        if (service == nullptr) {
            TAG_LOGE(AAFwkTag::QUICKFIX, "null quick fix service");
            NotifyApplyStatus(QUICK_FIX_INVALID_PARAM);
            RemoveSelf();
            return;
        }

        auto ret = service->GetApplyedQuickFixInfo(bundleName_, quickFixInfo);
        if (ret == QUICK_FIX_OK && !quickFixInfo.appqfInfo.hqfInfos.empty()) {
            // if there exist old version hqfInfo, need to unload.
            TAG_LOGD(AAFwkTag::QUICKFIX, "Need unload patch firstly");
            return PostNotifyUnloadRepairPatchTask();
        }
    }

    PostSwitchQuickFixTask();
}

void QuickFixManagerApplyTask::HandlePatchSwitched()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::QUICKFIX, "called");

    if (isRunning_ && !isSoContained_) {
        return PostNotifyLoadRepairPatchTask();
    }

    PostDeleteQuickFixTask();
}

void QuickFixManagerApplyTask::HandlePatchDeleted()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::QUICKFIX, "called");

    if (isRunning_ && !isSoContained_ && type_ == AppExecFwk::QuickFixType::HOT_RELOAD) {
        return PostNotifyHotReloadPageTask();
    }

    NotifyApplyStatus(QUICK_FIX_OK);
    RemoveSelf();
}

void QuickFixManagerApplyTask::PostDeployQuickFixTask(const std::vector<std::string> &quickFixFiles, bool isDebug,
    bool isReplace, bool isCheckDebugApp)
{
    auto callback = sptr<QuickFixManagerStatusCallback>::MakeSptr(shared_from_this());
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "Create deploy callback failed");
        NotifyApplyStatus(QUICK_FIX_DEPLOY_FAILED);
        RemoveSelf();
        return;
    }

    std::weak_ptr<QuickFixManagerApplyTask> thisWeakPtr(weak_from_this());
    auto deployTask = [thisWeakPtr, quickFixFiles, callback, isDebug, isReplace, isCheckDebugApp]() {
        auto applyTask = thisWeakPtr.lock();
        if (applyTask == nullptr) {
            TAG_LOGE(AAFwkTag::QUICKFIX, "null apply task");
            return;
        }

        if (applyTask->bundleQfMgr_ == nullptr) {
            TAG_LOGE(AAFwkTag::QUICKFIX, "null bundleQfMgr_");
            applyTask->NotifyApplyStatus(QUICK_FIX_BUNDLEMGR_INVALID);
            applyTask->RemoveSelf();
            return;
        }

        TAG_LOGD(AAFwkTag::QUICKFIX, "isDebug is %{public}d isReplace is %{public}d isCheckDebugApp is %{public}d",
            isDebug, isReplace, isCheckDebugApp);
        auto ret =
            applyTask->bundleQfMgr_->DeployQuickFix(quickFixFiles, callback, isDebug, "", isReplace, isCheckDebugApp);
        if (ret != 0) {
            TAG_LOGE(AAFwkTag::QUICKFIX, "failed: %{public}d", ret);
            applyTask->NotifyApplyStatus(QUICK_FIX_DEPLOY_FAILED);
            applyTask->RemoveSelf();
            return;
        }
    };
    if (eventHandler_ == nullptr || !eventHandler_->PostTask(deployTask, "QuickFixManager:deployTask")) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "post deploy task failed");
    }
    PostTimeOutTask();
}

void QuickFixManagerApplyTask::PostSwitchQuickFixTask()
{
    auto callback = sptr<QuickFixManagerStatusCallback>::MakeSptr(shared_from_this());
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "null callback");
        NotifyApplyStatus(QUICK_FIX_SWICH_FAILED);
        RemoveSelf();
        return;
    }

    std::weak_ptr<QuickFixManagerApplyTask> thisWeakPtr(weak_from_this());
    auto switchTask = [thisWeakPtr, callback]() {
        auto applyTask = thisWeakPtr.lock();
        if (applyTask == nullptr) {
            TAG_LOGE(AAFwkTag::QUICKFIX, "null apply task");
            return;
        }

        if (applyTask->bundleQfMgr_ == nullptr) {
            TAG_LOGE(AAFwkTag::QUICKFIX, "null bundleQfMgr_");
            applyTask->NotifyApplyStatus(QUICK_FIX_BUNDLEMGR_INVALID);
            applyTask->RemoveSelf();
            return;
        }

        auto ret = applyTask->bundleQfMgr_->SwitchQuickFix(applyTask->bundleName_, true, callback);
        if (ret != 0) {
            TAG_LOGE(AAFwkTag::QUICKFIX, "failed: %{public}d", ret);
            applyTask->NotifyApplyStatus(QUICK_FIX_SWICH_FAILED);
            applyTask->RemoveSelf();
            return;
        }
    };
    if (eventHandler_ == nullptr || !eventHandler_->PostTask(switchTask, "QuickFixManager:switchTask")) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "Post switch task failed");
    }
    PostTimeOutTask();
}

void QuickFixManagerApplyTask::PostDeleteQuickFixTask()
{
    auto callback = sptr<QuickFixManagerStatusCallback>::MakeSptr(shared_from_this());
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "null callback");
        NotifyApplyStatus(QUICK_FIX_DELETE_FAILED);
        RemoveSelf();
        return;
    }

    std::weak_ptr<QuickFixManagerApplyTask> thisWeakPtr(weak_from_this());
    auto deleteTask = [thisWeakPtr, callback]() {
        auto applyTask = thisWeakPtr.lock();
        if (applyTask == nullptr) {
            TAG_LOGE(AAFwkTag::QUICKFIX, "null apply task");
            return;
        }

        if (applyTask->bundleQfMgr_ == nullptr) {
            TAG_LOGE(AAFwkTag::QUICKFIX, "null bundleQfMgr_");
            applyTask->NotifyApplyStatus(QUICK_FIX_BUNDLEMGR_INVALID);
            applyTask->RemoveSelf();
            return;
        }

        auto ret = applyTask->bundleQfMgr_->DeleteQuickFix(applyTask->bundleName_, callback);
        if (ret != 0) {
            TAG_LOGE(AAFwkTag::QUICKFIX, "delete quick fix failed: %{public}d", ret);
            applyTask->NotifyApplyStatus(QUICK_FIX_DELETE_FAILED);
            applyTask->RemoveSelf();
            return;
        }
    };
    if (eventHandler_ == nullptr || !eventHandler_->PostTask(deleteTask, "QuickFixManager:deleteTask")) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "Post delete task failed");
    }
    PostTimeOutTask();
}

void QuickFixManagerApplyTask::PostTimeOutTask()
{
    std::weak_ptr<QuickFixManagerApplyTask> thisWeakPtr(weak_from_this());
    auto timeoutTask = [thisWeakPtr]() {
        auto applyTask = thisWeakPtr.lock();
        if (applyTask == nullptr) {
            TAG_LOGE(AAFwkTag::QUICKFIX, "null apply task");
            return;
        }

        applyTask->NotifyApplyStatus(QUICK_FIX_PROCESS_TIMEOUT);
        applyTask->RemoveSelf();
    };
    if (eventHandler_ == nullptr || !eventHandler_->PostTask(timeoutTask, TIMEOUT_TASK_NAME, TIMEOUT_TASK_DELAY_TIME)) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "Post delete task failed");
    }
}

void QuickFixManagerApplyTask::RemoveTimeoutTask()
{
    if (eventHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "null event handler");
        return;
    }
    eventHandler_->RemoveTask(TIMEOUT_TASK_NAME);
}

bool QuickFixManagerApplyTask::ExtractQuickFixDataFromJson(nlohmann::json& resultJson)
{
    if (!resultJson.contains(QUICK_FIX_BUNDLE_NAME) || !resultJson.at(QUICK_FIX_BUNDLE_NAME).is_string()) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "Invalid bundleName");
        return false;
    }
    bundleName_ = resultJson.at(QUICK_FIX_BUNDLE_NAME).get<std::string>();

    if (!resultJson.contains(QUICK_FIX_BUNDLE_VERSION_CODE) ||
        !resultJson.at(QUICK_FIX_BUNDLE_VERSION_CODE).is_number()) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "Invalid bundle version code");
        return false;
    }
    bundleVersionCode_ = resultJson.at(QUICK_FIX_BUNDLE_VERSION_CODE).get<int32_t>();

    if (!resultJson.contains(QUICK_FIX_PATCH_VERSION_CODE) ||
        !resultJson.at(QUICK_FIX_PATCH_VERSION_CODE).is_number()) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "Invalid patch version code");
        return false;
    }
    patchVersionCode_ = resultJson.at(QUICK_FIX_PATCH_VERSION_CODE).get<int32_t>();

    if (!resultJson.contains(QUICK_FIX_IS_SO_CONTAINED) || !resultJson.at(QUICK_FIX_IS_SO_CONTAINED).is_boolean()) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "Invalid so status");
        return false;
    }
    isSoContained_ = resultJson.at(QUICK_FIX_IS_SO_CONTAINED).get<bool>();

    if (!resultJson.contains(QUICK_FIX_TYPE) || !resultJson.at(QUICK_FIX_TYPE).is_number()) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "Invalid quickfix type");
        return false;
    }
    type_ = static_cast<AppExecFwk::QuickFixType>(resultJson.at(QUICK_FIX_TYPE).get<int32_t>());
    return true;
}

bool QuickFixManagerApplyTask::SetQuickFixInfo(const std::shared_ptr<AppExecFwk::QuickFixResult> &result)
{
    auto resultJson = nlohmann::json::parse(result->ToString(), nullptr, false);
    if (resultJson.is_discarded()) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "failed to parse json sting");
        return false;
    }
    if (ExtractQuickFixDataFromJson(resultJson) != true) {
        return false;
    }
    if (type_ != AppExecFwk::QuickFixType::PATCH && type_ != AppExecFwk::QuickFixType::HOT_RELOAD) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "quick fix type invalid");
        return false;
    }

    if (!resultJson.contains(QUICK_FIX_MODULE_NAME) || !resultJson.at(QUICK_FIX_MODULE_NAME).is_array()) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "Invalid moduleName");
        return false;
    }
    moduleNames_.clear();
    auto size = resultJson[QUICK_FIX_MODULE_NAME].size();
    for (size_t i = 0; i < size; i++) {
        if (resultJson[QUICK_FIX_MODULE_NAME][i].is_string()) {
            moduleNames_.emplace_back(resultJson[QUICK_FIX_MODULE_NAME][i]);
        }
    }

    TAG_LOGI(AAFwkTag::QUICKFIX, "bundleName: %{public}s, bundleVersion: %{public}d, patchVersion: %{public}d,"
                "soContained: %{public}d, ""type: %{public}d", bundleName_.c_str(), bundleVersionCode_,
        patchVersionCode_, isSoContained_, static_cast<int32_t>(type_));
    return true;
}

bool QuickFixManagerApplyTask::GetRunningState()
{
    if (appMgr_ == nullptr) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "null appMgr_");
        return false;
    }

    auto ret = appMgr_->GetAppRunningStateByBundleName(bundleName_);
    TAG_LOGI(AAFwkTag::QUICKFIX, "process running state of [%{public}s]: %{public}d", bundleName_.c_str(), ret);
    return ret;
}

void QuickFixManagerApplyTask::NotifyApplyStatus(int32_t resultCode)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::QUICKFIX, "called");

    Want want;
    if (GetTaskType() == TaskType::QUICK_FIX_APPLY) {
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_QUICK_FIX_APPLY_RESULT);
        want.SetParam(APPLY_RESULT, QuickFixErrorUtil::GetErrorCode(resultCode));
        want.SetParam(APPLY_RESULT_INFO, QuickFixErrorUtil::GetErrorMessage(resultCode));
        want.SetParam(BUNDLE_VERSION, bundleVersionCode_);
        want.SetParam(PATCH_VERSION, patchVersionCode_);

        std::string moduleName = std::accumulate(moduleNames_.begin(), moduleNames_.end(), std::string(""),
            [moduleName = moduleNames_](const std::string &name, const std::string &str) {
                return (str == moduleName.front()) ? (name + str) : (name + "," + str);
            });
        want.SetModuleName(moduleName);
    } else if (GetTaskType() == TaskType::QUICK_FIX_REVOKE) {
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_QUICK_FIX_REVOKE_RESULT);
        want.SetParam(REVOKE_RESULT, QuickFixErrorUtil::GetErrorCode(resultCode));
        want.SetParam(REVOKE_RESULT_INFO, QuickFixErrorUtil::GetErrorMessage(resultCode));
    } else {
        TAG_LOGW(AAFwkTag::QUICKFIX, "invalid task type, not publish common event");
        return;
    }

    want.SetParam(BUNDLE_NAME, bundleName_);

    EventFwk::CommonEventData commonData {want};
    EventFwk::CommonEventManager::PublishCommonEvent(commonData);
}

void QuickFixManagerApplyTask::PostNotifyLoadRepairPatchTask()
{
    auto callback = sptr<QuickFixNotifyCallback>::MakeSptr(shared_from_this());
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "null callback");
        NotifyApplyStatus(QUICK_FIX_NOTIFY_LOAD_PATCH_FAILED);
        RemoveSelf();
        return;
    }

    std::weak_ptr<QuickFixManagerApplyTask> thisWeakPtr(weak_from_this());
    auto loadPatchTask = [thisWeakPtr, callback]() {
        auto applyTask = thisWeakPtr.lock();
        if (applyTask == nullptr) {
            TAG_LOGE(AAFwkTag::QUICKFIX, "null apply task");
            return;
        }

        if (applyTask->appMgr_ == nullptr) {
            TAG_LOGE(AAFwkTag::QUICKFIX, "null appMgr");
            applyTask->NotifyApplyStatus(QUICK_FIX_APPMGR_INVALID);
            applyTask->RemoveSelf();
            return;
        }

        auto ret = applyTask->appMgr_->NotifyLoadRepairPatch(applyTask->bundleName_, callback);
        if (ret != 0) {
            TAG_LOGE(AAFwkTag::QUICKFIX, "Notify app load patch failed");
            applyTask->NotifyApplyStatus(QUICK_FIX_NOTIFY_LOAD_PATCH_FAILED);
            applyTask->RemoveSelf();
        }
    };
    if (eventHandler_ == nullptr || !eventHandler_->PostTask(loadPatchTask, "QuickFixManager:loadPatchTask")) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "post delete failed");
    }
    PostTimeOutTask();
}

void QuickFixManagerApplyTask::PostNotifyUnloadRepairPatchTask()
{
    auto callback = sptr<QuickFixNotifyCallback>::MakeSptr(shared_from_this());
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "null callback");
        NotifyApplyStatus(QUICK_FIX_NOTIFY_UNLOAD_PATCH_FAILED);
        RemoveSelf();
        return;
    }

    std::weak_ptr<QuickFixManagerApplyTask> thisWeakPtr(weak_from_this());
    auto unloadPatchTask = [thisWeakPtr, callback]() {
        auto applyTask = thisWeakPtr.lock();
        if (applyTask == nullptr) {
            TAG_LOGE(AAFwkTag::QUICKFIX, "null apply task");
            return;
        }

        if (applyTask->appMgr_ == nullptr) {
            TAG_LOGE(AAFwkTag::QUICKFIX, "null appMgr");
            applyTask->NotifyApplyStatus(QUICK_FIX_APPMGR_INVALID);
            applyTask->RemoveSelf();
            return;
        }

        auto ret = applyTask->appMgr_->NotifyUnLoadRepairPatch(applyTask->bundleName_, callback);
        if (ret != 0) {
            TAG_LOGE(AAFwkTag::QUICKFIX, "Notify app unload patch failed");
            applyTask->NotifyApplyStatus(QUICK_FIX_NOTIFY_UNLOAD_PATCH_FAILED);
            applyTask->RemoveSelf();
        }
    };
    if (eventHandler_ == nullptr || !eventHandler_->PostTask(unloadPatchTask, "QuickFixManager:unloadPatchTask")) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "Post delete task failed");
    }
    PostTimeOutTask();
}

void QuickFixManagerApplyTask::PostNotifyHotReloadPageTask()
{
    auto callback = sptr<QuickFixNotifyCallback>::MakeSptr(shared_from_this());
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "null callback");
        NotifyApplyStatus(QUICK_FIX_NOTIFY_RELOAD_PAGE_FAILED);
        RemoveSelf();
        return;
    }

    std::weak_ptr<QuickFixManagerApplyTask> thisWeakPtr(weak_from_this());
    auto reloadPageTask = [thisWeakPtr, callback]() {
        auto applyTask = thisWeakPtr.lock();
        if (applyTask == nullptr) {
            TAG_LOGE(AAFwkTag::QUICKFIX, "null apply task");
            return;
        }

        if (applyTask->appMgr_ == nullptr) {
            TAG_LOGE(AAFwkTag::QUICKFIX, "null appMgr");
            applyTask->NotifyApplyStatus(QUICK_FIX_APPMGR_INVALID);
            applyTask->RemoveSelf();
            return;
        }

        auto ret = applyTask->appMgr_->NotifyHotReloadPage(applyTask->bundleName_, callback);
        if (ret != 0) {
            TAG_LOGE(AAFwkTag::QUICKFIX, "Notify app reload page failed");
            applyTask->NotifyApplyStatus(QUICK_FIX_NOTIFY_RELOAD_PAGE_FAILED);
            applyTask->RemoveSelf();
        }
    };
    if (eventHandler_ == nullptr || !eventHandler_->PostTask(reloadPageTask, "QuickFixManager:reloadPageTask")) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "Post delete task failed");
    }
    PostTimeOutTask();
}

void QuickFixManagerApplyTask::RegAppStateObserver()
{
    TAG_LOGD(AAFwkTag::QUICKFIX, "Register application state observer");
    if (appMgr_ == nullptr) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "null appMgr");
        NotifyApplyStatus(QUICK_FIX_APPMGR_INVALID);
        RemoveSelf();
        return;
    }

    std::vector<std::string> bundleNameList;
    bundleNameList.push_back(bundleName_);
    auto callback = sptr<QuickFixMgrAppStateObserver>::MakeSptr(shared_from_this());
    // The validity of callback will be checked below.
    auto ret = appMgr_->RegisterApplicationStateObserver(callback, bundleNameList);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "register app state observer failed");
        NotifyApplyStatus(QUICK_FIX_REGISTER_OBSERVER_FAILED);
        RemoveSelf();
        return;
    }

    appStateCallback_ = callback;
    TAG_LOGD(AAFwkTag::QUICKFIX, "Register application state observer succeed");
}

void QuickFixManagerApplyTask::UnregAppStateObserver()
{
    TAG_LOGD(AAFwkTag::QUICKFIX, "Unregister application state observer");
    if (appMgr_ == nullptr || appStateCallback_ == nullptr) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "null appMgr/callback");
        return;
    }

    auto ret = appMgr_->UnregisterApplicationStateObserver(appStateCallback_);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "unregister app state observer failed");
        return;
    }

    TAG_LOGD(AAFwkTag::QUICKFIX, "Unregister application state observer succeed");
}

void QuickFixManagerApplyTask::RemoveSelf()
{
    auto service = quickFixMgrService_.promote();
    if (service) {
        service->RemoveApplyTask(shared_from_this());
    }
}

std::string QuickFixManagerApplyTask::GetBundleName()
{
    return bundleName_;
}

QuickFixManagerApplyTask::TaskType QuickFixManagerApplyTask::GetTaskType()
{
    return taskType_;
}

void QuickFixManagerApplyTask::PostRevokeQuickFixTask()
{
    std::weak_ptr<QuickFixManagerApplyTask> thisWeakPtr(weak_from_this());
    auto revokeTask = [thisWeakPtr] () {
        auto applyTask = thisWeakPtr.lock();
        if (applyTask == nullptr) {
            TAG_LOGE(AAFwkTag::QUICKFIX, "null revoke task");
            return;
        }
        if (applyTask->GetRunningState()) {
            applyTask->HandleRevokeQuickFixAppRunning();
            return;
        }
        applyTask->HandleRevokeQuickFixAppStop();
    };
    if (eventHandler_ == nullptr || !eventHandler_->PostTask(revokeTask, "QuickFixManager:revokeTask")) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "post revoke failed");
    }
    PostTimeOutTask();
}

void QuickFixManagerApplyTask::HandleRevokeQuickFixAppRunning()
{
    // process run
    // so contained, reg app died
    if (isSoContained_) {
        RegAppStateObserver();
        RemoveTimeoutTask();
        return;
    }

    // so not contained, call bms to switch
    HandleRevokeQuickFixAppStop();
}

void QuickFixManagerApplyTask::HandleRevokePatchSwitched()
{
    TAG_LOGD(AAFwkTag::QUICKFIX, "called");
    // process is run, notify app unload patch
    if (GetRunningState()) {
        PostRevokeQuickFixNotifyUnloadPatchTask();
        return;
    }

    // call bms to delete patch
    PostRevokeQuickFixDeleteTask();
}

void QuickFixManagerApplyTask::PostRevokeQuickFixNotifyUnloadPatchTask()
{
    // notify app process unload patch
    if (appMgr_ == nullptr) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "null appMgr");
        NotifyApplyStatus(QUICK_FIX_APPMGR_INVALID);
        RemoveSelf();
        return;
    }

    // app process run and wait callback
    auto callback = sptr<RevokeQuickFixNotifyCallback>::MakeSptr(shared_from_this());
    // The validity of callback will be checked below.
    auto ret = appMgr_->NotifyUnLoadRepairPatch(bundleName_, callback);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "Notify app unload patch failed");
        NotifyApplyStatus(QUICK_FIX_NOTIFY_UNLOAD_PATCH_FAILED);
        RemoveSelf();
    }

    TAG_LOGD(AAFwkTag::QUICKFIX, "Function end");
}

void QuickFixManagerApplyTask::PostRevokeQuickFixDeleteTask()
{
    auto callback = sptr<RevokeQuickFixTaskCallback>::MakeSptr(shared_from_this());
    if (callback == nullptr || bundleQfMgr_ == nullptr) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "Param invalid");
        NotifyApplyStatus(QUICK_FIX_BUNDLEMGR_INVALID);
        RemoveSelf();
        return;
    }

    // call delete patch to bms
    auto ret = bundleQfMgr_->DeleteQuickFix(bundleName_, callback);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "failed: %{public}d", ret);
        NotifyApplyStatus(QUICK_FIX_DELETE_FAILED);
        RemoveSelf();
        return;
    }
}

void QuickFixManagerApplyTask::PostRevokeQuickFixProcessDiedTask()
{
    TAG_LOGD(AAFwkTag::QUICKFIX, "called");
    // app process died
    HandleRevokeQuickFixAppStop();
    PostTimeOutTask();
}

void QuickFixManagerApplyTask::HandleRevokeQuickFixAppStop()
{
    auto callback = sptr<RevokeQuickFixTaskCallback>::MakeSptr(shared_from_this());
    if (callback == nullptr || bundleQfMgr_ == nullptr) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "Param invalid");
        NotifyApplyStatus(QUICK_FIX_BUNDLEMGR_INVALID);
        RemoveSelf();
        return;
    }

    auto ret = bundleQfMgr_->SwitchQuickFix(bundleName_, false, callback);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "failed: %{public}d", ret);
        NotifyApplyStatus(QUICK_FIX_SWICH_FAILED);
        RemoveSelf();
        return;
    }
}

void QuickFixManagerApplyTask::HandleRevokePatchDeleted()
{
    NotifyApplyStatus(QUICK_FIX_OK);
    RemoveSelf();
}
} // namespace AAFwk
} // namespace OHOS