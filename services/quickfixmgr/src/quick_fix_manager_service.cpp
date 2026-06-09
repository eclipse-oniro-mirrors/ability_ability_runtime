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

#include "quick_fix_manager_service.h"

#include "bundle_mgr_helper.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "parameters.h"
#include "permission_constants.h"
#include "permission_verification.h"
#include "quick_fix_error_utils.h"
#include "quick_fix_utils.h"

namespace OHOS {
namespace AAFwk {
constexpr const char* DEVELOPER_MODE_STATE = "const.security.developermode.state";

std::mutex QuickFixManagerService::mutex_;
sptr<QuickFixManagerService> QuickFixManagerService::instance_;

sptr<QuickFixManagerService> QuickFixManagerService::GetInstance()
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (instance_ == nullptr) {
        instance_ = new QuickFixManagerService();
    }
    return instance_;
}

bool QuickFixManagerService::Init()
{
    std::lock_guard<std::mutex> lock(eventMutex_);
    eventRunner_ = AppExecFwk::EventRunner::Create("QuickFixMgrSvrMain");
    if (eventRunner_ == nullptr) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "null eventRunner_");
        return false;
    }

    eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(eventRunner_);
    if (eventHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "null eventHandler_");
        return false;
    }

    return true;
}

int32_t QuickFixManagerService::ApplyQuickFix(const std::vector<std::string> &quickFixFiles, bool isDebug,
    bool isReplace)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::QUICKFIX, "called");
    bool isSACaller = AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI();
    bool isDeveloperMode = OHOS::system::GetBoolParameter(DEVELOPER_MODE_STATE, false);
    if (!isSACaller && (!isDeveloperMode || !AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_ALLOW_USE_BM))) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "caller not system-app,not use system-api");
        return QUICK_FIX_NOT_SYSTEM_APP;
    }
    bool isCheckDebugApp = false;
    if (!isSACaller || !AAFwk::PermissionVerification::GetInstance()->VerifyInstallBundlePermission()) {
        if (!isDeveloperMode ||
            !AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
                AAFwk::PermissionConstants::PERMISSION_ALLOW_USE_BM)) {
            return QUICK_FIX_VERIFY_PERMISSION_FAILED;
        }
        isCheckDebugApp = true;
    }

    auto bundleQfMgr = QuickFixUtil::GetBundleQuickFixMgrProxy();
    if (bundleQfMgr == nullptr) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "null bundleQfMgr");
        return QUICK_FIX_CONNECT_FAILED;
    }

    auto appMgr = QuickFixUtil::GetAppManagerProxy();
    if (appMgr == nullptr) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "null appMgr");
        return QUICK_FIX_CONNECT_FAILED;
    }
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr, appMgr, eventHandler_, this);
    AddApplyTask(applyTask);
    applyTask->Run(quickFixFiles, isDebug, isReplace, isCheckDebugApp);

    return QUICK_FIX_OK;
}

int32_t QuickFixManagerService::GetApplyedQuickFixInfo(const std::string &bundleName,
    ApplicationQuickFixInfo &quickFixInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::QUICKFIX, "called");
    bool isSACaller = AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI();
    bool isDeveloperMode = OHOS::system::GetBoolParameter(DEVELOPER_MODE_STATE, false);
    if (!isSACaller && (!isDeveloperMode || !AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_ALLOW_USE_BM))) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "caller not system-app,not use system-api");
        return QUICK_FIX_NOT_SYSTEM_APP;
    }
    bool isCheckDebugApp = false;
    if (!isSACaller || !AAFwk::PermissionVerification::GetInstance()->VerifyGetBundleInfoPrivilegedPermission()) {
        if (!isDeveloperMode ||
            !AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
                AAFwk::PermissionConstants::PERMISSION_ALLOW_USE_BM)) {
            return QUICK_FIX_VERIFY_PERMISSION_FAILED;
        }
        isCheckDebugApp = true;
    }

    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "null bundleMgrHelper");
        return QUICK_FIX_CONNECT_FAILED;
    }

    AppExecFwk::BundleInfo bundleInfo;
    if (!bundleMgrHelper->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo,
        AppExecFwk::Constants::ANY_USERID)) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "get bundleInfo failed");
        return QUICK_FIX_GET_BUNDLE_INFO_FAILED;
    }
    if (isCheckDebugApp &&
        bundleInfo.applicationInfo.appProvisionType != AppExecFwk::Constants::APP_PROVISION_TYPE_DEBUG) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "not debug version");
        return QUICK_FIX_VERIFY_PERMISSION_FAILED;
    }

    quickFixInfo.bundleName = bundleName;
    quickFixInfo.bundleVersionCode = bundleInfo.versionCode;
    quickFixInfo.bundleVersionName = bundleInfo.versionName;
    quickFixInfo.appqfInfo = bundleInfo.applicationInfo.appQuickFix.deployedAppqfInfo;

    return QUICK_FIX_OK;
}

int32_t QuickFixManagerService::RevokeQuickFix(const std::string &bundleName)
{
    TAG_LOGD(AAFwkTag::QUICKFIX, "called");
    if (!AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "caller not system-app, not use system-api");
        return QUICK_FIX_NOT_SYSTEM_APP;
    }

    if (!AAFwk::PermissionVerification::GetInstance()->VerifyGetBundleInfoPrivilegedPermission() ||
        !AAFwk::PermissionVerification::GetInstance()->VerifyInstallBundlePermission()) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "Permission verification failed");
        return QUICK_FIX_VERIFY_PERMISSION_FAILED;
    }

    if (CheckTaskRunningState(bundleName)) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "apply quick fix task");
        return QUICK_FIX_DEPLOYING_TASK;
    }

    auto patchExists = false;
    auto isSoContained = false;
    auto ret = GetQuickFixInfo(bundleName, patchExists, isSoContained);
    if (ret != QUICK_FIX_OK || !patchExists) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "get bundle info failed/patch not exist");
        return QUICK_FIX_GET_BUNDLE_INFO_FAILED;
    }

    auto appMgr = QuickFixUtil::GetAppManagerProxy();
    if (appMgr == nullptr) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "null appMgr");
        return QUICK_FIX_CONNECT_FAILED;
    }

    auto bundleQfMgr = QuickFixUtil::GetBundleQuickFixMgrProxy();
    if (bundleQfMgr == nullptr) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "null bundleQfMgr");
        return QUICK_FIX_CONNECT_FAILED;
    }

    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr, appMgr, eventHandler_, this);
    if (applyTask == nullptr) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "null applyTask");
        return QUICK_FIX_CONNECT_FAILED;
    }

    applyTask->InitRevokeTask(bundleName, isSoContained);
    AddApplyTask(applyTask);
    applyTask->RunRevoke();
    return QUICK_FIX_OK;
}

void QuickFixManagerService::AddApplyTask(std::shared_ptr<QuickFixManagerApplyTask> applyTask)
{
    std::lock_guard<std::mutex> lock(taskMutex_);
    applyTasks_.emplace_back(applyTask);
}

void QuickFixManagerService::RemoveApplyTask(std::shared_ptr<QuickFixManagerApplyTask> applyTask)
{
    std::lock_guard<std::mutex> lock(taskMutex_);
    for (auto it = applyTasks_.begin(); it != applyTasks_.end();) {
        if (*it == applyTask) {
            it = applyTasks_.erase(it);
        } else {
            it++;
        }
    }
}

bool QuickFixManagerService::CheckTaskRunningState(const std::string &bundleName)
{
    std::lock_guard<std::mutex> lock(taskMutex_);
    for (auto &item : applyTasks_) {
        if (item != nullptr && item->GetBundleName() == bundleName) {
            return true;
        }
    }

    TAG_LOGD(AAFwkTag::QUICKFIX, "bundleName %{public}s not found in tasks", bundleName.c_str());
    return false;
}

int32_t QuickFixManagerService::GetQuickFixInfo(const std::string &bundleName, bool &patchExists, bool &isSoContained)
{
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "null bundleMgrHelper");
        return QUICK_FIX_CONNECT_FAILED;
    }

    AppExecFwk::BundleInfo bundleInfo;
    if (!bundleMgrHelper->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo,
        AppExecFwk::Constants::ANY_USERID)) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "Get bundle info failed");
        return QUICK_FIX_GET_BUNDLE_INFO_FAILED;
    }

    for (auto &item : bundleInfo.hapModuleInfos) {
        if (!item.hqfInfo.moduleName.empty()) {
            patchExists = true;
            break;
        }
    }

    isSoContained = !bundleInfo.applicationInfo.appQuickFix.deployedAppqfInfo.nativeLibraryPath.empty();
    return QUICK_FIX_OK;
}
}  // namespace AAFwk
}  // namespace OHOS
