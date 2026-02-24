/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "app_lifecycle_deal.h"

#include "freeze_util.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "in_process_call_wrapper.h"
#include "time_util.h"
#include "app_mgr_service_const.h"
#include "app_mgr_service_dump_error_code.h"
#include "bundle_mgr_helper.h"

namespace OHOS {
using AbilityRuntime::FreezeUtil;
namespace AppExecFwk {
namespace {
constexpr const char* REUSING_WINDOW = "ohos.ability_runtime.reusing_window";
constexpr const char* IS_HOOK = "ohos.ability_runtime.is_hook";
}
AppLifeCycleDeal::AppLifeCycleDeal()
{}

AppLifeCycleDeal::~AppLifeCycleDeal()
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
}

void AppLifeCycleDeal::LaunchApplication(const AppLaunchData &launchData, const Configuration &config)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto appThread = GetApplicationClient();
    if (appThread) {
        appThread->ScheduleLaunchApplication(launchData, config);
    }
}

void AppLifeCycleDeal::UpdateApplicationInfoInstalled(const ApplicationInfo &appInfo, const std::string &moduleName)
{
    auto appThread = GetApplicationClient();
    if (!appThread) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appThread");
        return;
    }

    appThread->ScheduleUpdateApplicationInfoInstalled(appInfo, moduleName);
}

void AppLifeCycleDeal::AddAbilityStage(const HapModuleInfo &abilityStage)
{
    auto appThread = GetApplicationClient();
    if (!appThread) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appThread");
        return;
    }

    appThread->ScheduleAbilityStage(abilityStage);
}

void AppLifeCycleDeal::LaunchAbility(const std::shared_ptr<AbilityRunningRecord> &ability)
{
    auto appThread = GetApplicationClient();
    if (appThread && ability) {
        auto abilityInfo = ability->GetAbilityInfo();
        if (abilityInfo == nullptr) {
            TAG_LOGW(AAFwkTag::APPMGR, "null abilityInfo");
            return;
        }
        if (abilityInfo->type == AbilityType::PAGE) {
            std::string entry = "AppLifeCycleDeal::LaunchAbility";
            FreezeUtil::GetInstance().AddLifecycleEvent(ability->GetToken(), entry);
        }
        TAG_LOGD(AAFwkTag::APPMGR, "Launch");
        bool isHookAbility = ability->IsHook();
        if (isHookAbility) {
            auto bundleManagerHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
            HapModuleInfo hapModuleInfo;
            if (!bundleManagerHelper->GetHapModuleInfo(*abilityInfo, hapModuleInfo)) {
                TAG_LOGW(AAFwkTag::APPMGR, "GetHapModuleInfo fail");
                return;
            }
            int32_t BASE_USER_RANGE = 200000;
            auto userId = abilityInfo->uid / BASE_USER_RANGE;
            abilityInfo = std::make_shared<AbilityInfo>();
            Want want;
            want.SetElementName("", hapModuleInfo.bundleName, hapModuleInfo.abilitySrcEntryDelegator,
                hapModuleInfo.abilityStageSrcEntryDelegator);
            if (!IN_PROCESS_CALL(bundleManagerHelper->QueryAbilityInfo(
                want, AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION, userId, *abilityInfo))) {
                TAG_LOGW(AAFwkTag::APPMGR, "QueryAbilityInfo fail");
                return;
            }
        }
        appThread->ScheduleLaunchAbility(*abilityInfo, ability->GetToken(),
            ability->GetWant(), ability->GetAbilityRecordId());
        if (ability->GetWant() != nullptr) {
            ability->GetWant()->RemoveParam(REUSING_WINDOW);
            ability->GetWant()->RemoveParam(IS_HOOK);
        }
    } else {
        TAG_LOGW(AAFwkTag::APPMGR, "null appThread or ability");
    }
}

void AppLifeCycleDeal::ScheduleTerminate(bool isLastProcess)
{
    auto appThread = GetApplicationClient();
    if (!appThread) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appThread");
        return;
    }

    appThread->ScheduleTerminateApplication(isLastProcess);
}

bool AppLifeCycleDeal::ScheduleForegroundRunning()
{
    auto appThread = GetApplicationClient();
    if (!appThread) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appThread");
        return false;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return appThread->ScheduleForegroundApplication();
}

void AppLifeCycleDeal::ScheduleBackgroundRunning()
{
    auto appThread = GetApplicationClient();
    if (!appThread) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appThread");
        return;
    }

    appThread->ScheduleBackgroundApplication();
}

void AppLifeCycleDeal::ScheduleTrimMemory(int32_t timeLevel)
{
    auto appThread = GetApplicationClient();
    if (!appThread) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appThread");
        return;
    }

    appThread->ScheduleShrinkMemory(timeLevel);
}

void AppLifeCycleDeal::ScheduleMemoryLevel(int32_t Level, bool isShellCall)
{
    auto appThread = GetApplicationClient();
    if (!appThread) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appThread");
        return;
    }

    appThread->ScheduleMemoryLevel(Level, isShellCall);
}

void AppLifeCycleDeal::ScheduleHeapMemory(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo)
{
    auto appThread = GetApplicationClient();
    if (!appThread) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appThread");
        return;
    }

    appThread->ScheduleHeapMemory(pid, mallocInfo);
}

void AppLifeCycleDeal::ScheduleJsHeapMemory(OHOS::AppExecFwk::JsHeapDumpInfo &info)
{
    auto appThread = GetApplicationClient();
    if (!appThread) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appThread");
        return;
    }

    appThread->ScheduleJsHeapMemory(info);
}

void AppLifeCycleDeal::ScheduleCjHeapMemory(OHOS::AppExecFwk::CjHeapDumpInfo &info)
{
    auto appThread = GetApplicationClient();
    if (!appThread) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appThread");
        return;
    }

    appThread->ScheduleCjHeapMemory(info);
}

void AppLifeCycleDeal::LowMemoryWarning()
{
    auto appThread = GetApplicationClient();
    if (!appThread) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appThread");
        return;
    }

    appThread->ScheduleLowMemory();
}

void AppLifeCycleDeal::ScheduleCleanAbility(const sptr<IRemoteObject> &token, bool isCacheProcess)
{
    auto appThread = GetApplicationClient();
    if (!appThread) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appThread");
        return;
    }
    appThread->ScheduleCleanAbility(token, isCacheProcess);
}

void AppLifeCycleDeal::ScheduleProcessSecurityExit()
{
    auto appThread = GetApplicationClient();
    if (!appThread) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appThread");
        return;
    }

    appThread->ScheduleProcessSecurityExit();
}

void AppLifeCycleDeal::ScheduleClearPageStack()
{
    auto appThread = GetApplicationClient();
    if (!appThread) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appThread");
        return;
    }

    appThread->ScheduleClearPageStack();
}

void AppLifeCycleDeal::SetApplicationClient(const sptr<IAppScheduler> &thread)
{
    std::lock_guard guard(schedulerMutex_);
    appThread_ = thread;
}

sptr<IAppScheduler> AppLifeCycleDeal::GetApplicationClient() const
{
    std::lock_guard guard(schedulerMutex_);
    return appThread_;
}

void AppLifeCycleDeal::ScheduleAcceptWant(const AAFwk::Want &want, const std::string &moduleName)
{
    auto appThread = GetApplicationClient();
    if (!appThread) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appThread");
        return;
    }

    appThread->ScheduleAcceptWant(want, moduleName);
}

void AppLifeCycleDeal::SchedulePrepareTerminate(const std::string &moduleName)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    auto appThread = GetApplicationClient();
    if (!appThread) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appThread");
        return;
    }
    appThread->SchedulePrepareTerminate(moduleName);
}

void AppLifeCycleDeal::ScheduleNewProcessRequest(const AAFwk::Want &want, const std::string &moduleName)
{
    auto appThread = GetApplicationClient();
    if (!appThread) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appThread");
        return;
    }

    appThread->ScheduleNewProcessRequest(want, moduleName);
}

int32_t AppLifeCycleDeal::UpdateConfiguration(const Configuration &config, ConfigUpdateReason reason)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "call");
    auto appThread = GetApplicationClient();
    if (!appThread) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appThread");
        return ERR_INVALID_VALUE;
    }
    appThread->ScheduleConfigurationUpdated(config, reason);
    return ERR_OK;
}

int32_t AppLifeCycleDeal::NotifyLoadRepairPatch(const std::string &bundleName, const sptr<IQuickFixCallback> &callback,
    const int32_t recordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "call");
    auto appThread = GetApplicationClient();
    if (appThread == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appThread");
        return ERR_INVALID_VALUE;
    }
    return appThread->ScheduleNotifyLoadRepairPatch(bundleName, callback, recordId);
}

int32_t AppLifeCycleDeal::NotifyHotReloadPage(const sptr<IQuickFixCallback> &callback, const int32_t recordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "call");
    auto appThread = GetApplicationClient();
    if (appThread == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appThread");
        return ERR_INVALID_VALUE;
    }
    return appThread->ScheduleNotifyHotReloadPage(callback, recordId);
}

int32_t AppLifeCycleDeal::NotifyUnLoadRepairPatch(const std::string &bundleName,
    const sptr<IQuickFixCallback> &callback, const int32_t recordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto appThread = GetApplicationClient();
    if (appThread == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appThread");
        return ERR_INVALID_VALUE;
    }
    return appThread->ScheduleNotifyUnLoadRepairPatch(bundleName, callback, recordId);
}

int32_t AppLifeCycleDeal::NotifyAppFault(const FaultData &faultData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto appThread = GetApplicationClient();
    if (appThread == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appThread");
        return ERR_INVALID_VALUE;
    }
    return appThread->ScheduleNotifyAppFault(faultData);
}

int32_t AppLifeCycleDeal::ChangeAppGcState(int32_t state, uint64_t tid)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto appThread = GetApplicationClient();
    if (appThread == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appThread");
        return ERR_INVALID_VALUE;
    }
    return appThread->ScheduleChangeAppGcState(state, tid);
}

int32_t AppLifeCycleDeal::AttachAppDebug(bool isDebugFromLocal)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto appThread = GetApplicationClient();
    if (appThread == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appThread");
        return ERR_INVALID_VALUE;
    }
    appThread->AttachAppDebug(isDebugFromLocal);
    return ERR_OK;
}

int32_t AppLifeCycleDeal::DetachAppDebug()
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto appThread = GetApplicationClient();
    if (appThread == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appThread");
        return ERR_INVALID_VALUE;
    }
    appThread->DetachAppDebug();
    return ERR_OK;
}

int AppLifeCycleDeal::DumpIpcStart(std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto appThread = GetApplicationClient();
    if (appThread == nullptr) {
        result.append(MSG_DUMP_IPC_START_STAT, strlen(MSG_DUMP_IPC_START_STAT))
            .append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INTERNAL, strlen(MSG_DUMP_FAIL_REASON_INTERNAL));
        TAG_LOGE(AAFwkTag::APPMGR, "null appThread");
        return DumpErrorCode::ERR_INTERNAL_ERROR;
    }
    return appThread->ScheduleDumpIpcStart(result);
}

int AppLifeCycleDeal::DumpIpcStop(std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto appThread = GetApplicationClient();
    if (appThread == nullptr) {
        result.append(MSG_DUMP_IPC_STOP_STAT, strlen(MSG_DUMP_IPC_STOP_STAT))
            .append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INTERNAL, strlen(MSG_DUMP_FAIL_REASON_INTERNAL));
        TAG_LOGE(AAFwkTag::APPMGR, "null appThread");
        return DumpErrorCode::ERR_INTERNAL_ERROR;
    }
    return appThread->ScheduleDumpIpcStop(result);
}

int AppLifeCycleDeal::DumpIpcStat(std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto appThread = GetApplicationClient();
    if (appThread == nullptr) {
        result.append(MSG_DUMP_IPC_STAT, strlen(MSG_DUMP_IPC_STAT))
            .append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INTERNAL, strlen(MSG_DUMP_FAIL_REASON_INTERNAL));
        TAG_LOGE(AAFwkTag::APPMGR, "null appThread");
        return DumpErrorCode::ERR_INTERNAL_ERROR;
    }
    return appThread->ScheduleDumpIpcStat(result);
}

void AppLifeCycleDeal::ScheduleCacheProcess()
{
    auto appThread = GetApplicationClient();
    if (!appThread) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appThread");
        return;
    }

    appThread->ScheduleCacheProcess();
}

int AppLifeCycleDeal::DumpFfrt(std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto appThread = GetApplicationClient();
    if (appThread == nullptr) {
        result.append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INTERNAL, strlen(MSG_DUMP_FAIL_REASON_INTERNAL));
        TAG_LOGE(AAFwkTag::APPMGR, "null appThread");
        return DumpErrorCode::ERR_INTERNAL_ERROR;
    }
    return appThread->ScheduleDumpFfrt(result);
}

int32_t AppLifeCycleDeal::DumpArkWeb(const std::string &customArgs, std::string &result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "DumpArkWeb called");
    auto appThread = GetApplicationClient();
    if (appThread == nullptr) {
        result.append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INTERNAL, strlen(MSG_DUMP_FAIL_REASON_INTERNAL));
        TAG_LOGE(AAFwkTag::APPMGR, "null appThread");
        return DumpErrorCode::ERR_INTERNAL_ERROR;
    }
    return appThread->ScheduleDumpArkWeb(customArgs, result);
}

void AppLifeCycleDeal::SetWatchdogBackgroundStatusRunning(bool status)
{
    auto appThread = GetApplicationClient();
    if (!appThread) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appThread");
        return;
    }
    appThread->SetWatchdogBackgroundStatus(status);
}
}  // namespace AppExecFwk
}  // namespace OHOS
