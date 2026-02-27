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

#include "app_scheduler_proxy.h"

#include "error_msg_util.h"
#include "freeze_util.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "ipc_capacity_wrap.h"
#include "ipc_types.h"
#include "iremote_object.h"
#include "app_scheduler_const.h"

namespace OHOS {
namespace AppExecFwk {
AppSchedulerProxy::AppSchedulerProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IAppScheduler>(impl)
{}

bool AppSchedulerProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(AppSchedulerProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::APPMGR, "write interface token failed");
        return false;
    }
    return true;
}

bool AppSchedulerProxy::ScheduleForegroundApplication()
{
    TAG_LOGD(AAFwkTag::APPMGR, "AppSchedulerProxy::ScheduleForegroundApplication start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return false;
    }
    auto msgKey = AbilityRuntime::ErrorMgsUtil::BuildErrorKey(reinterpret_cast<uintptr_t>(this),
        "ScheduleForegroundRunning");
    AbilityRuntime::ErrorMgsUtil::GetInstance().UpdateErrorMsg(msgKey, "AppScheduler::ScheduleForegroundApplication");
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_FOREGROUND_APPLICATION_TRANSACTION),
            data,
            reply,
            option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
        AbilityRuntime::ErrorMgsUtil::GetInstance().UpdateErrorMsg(msgKey,
            std::string("ScheduleForegroundApplication ipc error ") + std::to_string(ret));
        return false;
    }
    return true;
}

void AppSchedulerProxy::ScheduleBackgroundApplication()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_BACKGROUND_APPLICATION_TRANSACTION),
            data,
            reply,
            option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is wrong, error code: %{public}d", ret);
    }
}

void AppSchedulerProxy::ScheduleTerminateApplication(bool isLastProcess)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteBool(isLastProcess)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write bool failed.");
        return;
    }
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_TERMINATE_APPLICATION_TRANSACTION), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is unsuccessful, error code: %{public}d", ret);
    }
}

void AppSchedulerProxy::ScheduleLowMemory()
{
    TAG_LOGD(AAFwkTag::APPMGR, "AppSchedulerProxy::ScheduleLowMemory begin");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_LOWMEMORY_APPLICATION_TRANSACTION), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
}

void AppSchedulerProxy::ScheduleMemoryLevel(int32_t level, bool isShellCall)
{
    uint32_t operation = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_MEMORYLEVEL_APPLICATION_TRANSACTION);
    ScheduleMemoryCommon(level, operation, isShellCall);
}

void AppSchedulerProxy::ScheduleHeapMemory(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo)
{
    TAG_LOGD(AAFwkTag::APPMGR, "AppSchedulerProxy::ScheduleHeapMemory start");
    uint32_t operation = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_HEAPMEMORY_APPLICATION_TRANSACTION);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppSchedulerProxy !WriteInterfaceToken.");
        return;
    }
    if (!data.WriteInt32(pid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write pid failed");
        return;
    }
    int32_t ret = SendTransactCmd(operation, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
        return;
    }

    std::unique_ptr<MallocInfo> info(reply.ReadParcelable<MallocInfo>());
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "MallocInfo ReadParcelable nullptr");
        return;
    }
    mallocInfo = *info;
}

void AppSchedulerProxy::ScheduleJsHeapMemory(OHOS::AppExecFwk::JsHeapDumpInfo &info)
{
    TAG_LOGD(AAFwkTag::APPMGR, "AppSchedulerProxy::ScheduleJsHeapMemory start");
    uint32_t operation = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_JSHEAP_MEMORY_APPLICATION_TRANSACTION);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppSchedulerProxy !WriteInterfaceToken.");
        return;
    }
    if (!data.WriteParcelable(&info)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write pid failed");
        return;
    }
    int32_t ret = SendTransactCmd(operation, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
        return;
    }
}

void AppSchedulerProxy::ScheduleCjHeapMemory(OHOS::AppExecFwk::CjHeapDumpInfo &info)
{
    TAG_LOGD(AAFwkTag::APPMGR, "AppSchedulerProxy::ScheduleCjHeapMemory start");
    uint32_t operation = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_CJHEAP_MEMORY_APPLICATION_TRANSACTION);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppSchedulerProxy !WriteInterfaceToken.");
        return;
    }
    if (!data.WriteParcelable(&info)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write pid failed");
        return;
    }
    int32_t ret = SendTransactCmd(operation, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
        return;
    }
}

void AppSchedulerProxy::ScheduleShrinkMemory(const int32_t level)
{
    uint32_t operation = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_SHRINK_MEMORY_APPLICATION_TRANSACTION);
    ScheduleMemoryCommon(level, operation);
}

void AppSchedulerProxy::ScheduleMemoryCommon(const int32_t level, const uint32_t operation, bool isShellCall)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!isShellCall) {
        option.SetFlags(MessageOption::TF_ASYNC_WAKEUP_LATER);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "option: %{public}d", option.GetFlags());
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteInt32(level)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write pid failed");
        return;
    }
    int32_t ret = SendTransactCmd(operation, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
}

void AppSchedulerProxy::ScheduleLaunchAbility(const AbilityInfo &info, const sptr<IRemoteObject> &token,
    const std::shared_ptr<AAFwk::Want> &want, int32_t abilityRecordId)
{
    MessageParcel data;
    MessageParcel reply;
    AAFwk::ExtendMaxIpcCapacityForInnerWant(data);
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteParcelable(&info)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write pid failed");
        return;
    }
    if (token) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(token.GetRefPtr())) {
            TAG_LOGE(AAFwkTag::APPMGR, "Failed to write flag and token");
            return;
        }
    } else {
        if (!data.WriteBool(false)) {
            TAG_LOGE(AAFwkTag::APPMGR, "Failed to write flag");
            return;
        }
    }

    if (!data.WriteParcelable(want.get())) {
        TAG_LOGE(AAFwkTag::APPMGR, "write want fail.");
        AbilityRuntime::FreezeUtil::GetInstance().AppendLifecycleEvent(token,
            "AppLifeCycleDeal::LaunchAbility; write want fail");
        return;
    }
    if (!data.WriteInt32(abilityRecordId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write ability record id fail.");
        return;
    }
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_LAUNCH_ABILITY_TRANSACTION), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
        AbilityRuntime::FreezeUtil::GetInstance().AppendLifecycleEvent(token,
            "AppLifeCycleDeal::LaunchAbility; ipc error " + std::to_string(ret));
    }
}

void AppSchedulerProxy::ScheduleCleanAbility(const sptr<IRemoteObject> &token, bool isCacheProcess)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteRemoteObject(token.GetRefPtr())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write token");
        return;
    }
    if (!data.WriteBool(isCacheProcess)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write bool");
        return;
    }
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_CLEAN_ABILITY_TRANSACTION), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
}

void AppSchedulerProxy::ScheduleLaunchApplication(const AppLaunchData &launchData, const Configuration &config)
{
    TAG_LOGD(AAFwkTag::APPMGR, "AppSchedulerProxy ScheduleLaunchApplication start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    auto msgKey = AbilityRuntime::ErrorMgsUtil::BuildErrorKey(reinterpret_cast<uintptr_t>(this),
        "LaunchApplication");
    AbilityRuntime::ErrorMgsUtil::GetInstance().UpdateErrorMsg(msgKey, "AppScheduler::ScheduleLaunchApplication");
    if (!data.WriteParcelable(&launchData)) {
        TAG_LOGE(AAFwkTag::APPMGR, "WriteParcelable launchData failed");
        AbilityRuntime::ErrorMgsUtil::GetInstance().UpdateErrorMsg(msgKey, "write launchData fail");
        return;
    }

    if (!data.WriteParcelable(&config)) {
        TAG_LOGE(AAFwkTag::APPMGR, "WriteParcelable config failed");
        AbilityRuntime::ErrorMgsUtil::GetInstance().UpdateErrorMsg(msgKey, "write config fail");
        return ;
    }

    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_LAUNCH_APPLICATION_TRANSACTION), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
        AbilityRuntime::ErrorMgsUtil::GetInstance().UpdateErrorMsg(msgKey,
            std::string("ScheduleLaunchApplication ipc error ") + std::to_string(ret));
    }
}

void AppSchedulerProxy::ScheduleUpdateApplicationInfoInstalled(const ApplicationInfo &appInfo,
    const std::string& moduleName)
{
    TAG_LOGD(AAFwkTag::APPMGR, "AppSchedulerProxy ScheduleUpdateApplicationInfoInstalled begin");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteParcelable(&appInfo)) {
        return ;
    }
    if (!data.WriteString(moduleName)) {
        return;
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_UPDATE_APPLICATION_INFO_INSTALLED), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "AppSchedulerProxy ScheduleUpdateApplicationInfoInstalled end");
}

void AppSchedulerProxy::ScheduleAbilityStage(const HapModuleInfo &abilityStage)
{
    TAG_LOGD(AAFwkTag::APPMGR, "AppSchedulerProxy ScheduleAbilityStage start");
    MessageParcel data;
    constexpr int32_t max = 10000;
    constexpr int32_t large = 60;
    constexpr int32_t mid = 20;
    auto abilityInfoSize = static_cast<int32_t>(abilityStage.abilityInfos.size());
    auto extensionInfoSize = static_cast<int32_t>(abilityStage.extensionInfos.size());
    if (abilityInfoSize > max || extensionInfoSize > max) {
        TAG_LOGE(AAFwkTag::APPMGR, "size exceeds max");
        return;
    }
    auto componentSize = abilityInfoSize + extensionInfoSize;
    if (componentSize > large) {
        constexpr int32_t size = 2 * 1024 * 1024; // 1.6 M
        data.SetDataCapacity(size);
    } else if (componentSize > mid) {
        constexpr int32_t size = 800 * 1024; // 800 kb
        data.SetDataCapacity(size);
    }
    if (!WriteInterfaceToken(data)) {
        return;
    }

    if (!data.WriteParcelable(&abilityStage)) {
        return ;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_ABILITY_STAGE_INFO), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "AppSchedulerProxy ScheduleAbilityStage end");
}

void AppSchedulerProxy::ScheduleProfileChanged(const Profile &profile)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteParcelable(&profile)) {
        TAG_LOGD(AAFwkTag::APPMGR, "write profile failed");
        return;
    }
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_PROFILE_CHANGED_TRANSACTION), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
}

void AppSchedulerProxy::ScheduleConfigurationUpdated(const Configuration &config, ConfigUpdateReason reason)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteParcelable(&config)) {
        TAG_LOGD(AAFwkTag::APPMGR, "write profile failed");
        return;
    }
    if (!data.WriteUint8(static_cast<uint8_t>(reason))) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write flag failed.");
        return;
    }
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_CONFIGURATION_UPDATED), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
}

void AppSchedulerProxy::ScheduleProcessSecurityExit()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_PROCESS_SECURITY_EXIT_TRANSACTION), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
}

void AppSchedulerProxy::ScheduleClearPageStack()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_CLEAR_PAGE_STACK), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
}

void AppSchedulerProxy::ScheduleAcceptWant(const AAFwk::Want &want, const std::string &moduleName)
{
    MessageParcel data;
    MessageParcel reply;
    AAFwk::ExtendMaxIpcCapacityForInnerWant(data);
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    if (!data.WriteParcelable(&want) || !data.WriteString(moduleName)) {
        return;
    }
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_ACCEPT_WANT), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
}

void AppSchedulerProxy::SchedulePrepareTerminate(const std::string &moduleName)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "token write error");
        return;
    }
    if (!data.WriteString(moduleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write SchedulePrepareTerminate moduleName failed.");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Remote() is NULL");
        return;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_PREPARE_TERMINATE), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest SchedulePrepareTerminate err: %{public}d", ret);
        return;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "Get SchedulePrepareTerminate reply success");
}

void AppSchedulerProxy::ScheduleNewProcessRequest(const AAFwk::Want &want, const std::string &moduleName)
{
    MessageParcel data;
    MessageParcel reply;
    AAFwk::ExtendMaxIpcCapacityForInnerWant(data);
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    if (!data.WriteParcelable(&want) || !data.WriteString(moduleName)) {
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Remote() is NULL");
        return;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_NEW_PROCESS_REQUEST), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
}

int32_t AppSchedulerProxy::ScheduleNotifyLoadRepairPatch(const std::string &bundleName,
    const sptr<IQuickFixCallback> &callback, const int32_t recordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "ScheduleNotifyLoadRepairPatch, Write interface token failed.");
        return ERR_INVALID_DATA;
    }

    if (!data.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "ScheduleNotifyLoadRepairPatch, Write bundle name failed.");
        return ERR_INVALID_DATA;
    }

    if (callback == nullptr || !data.WriteRemoteObject(callback->AsObject())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write callback failed.");
        return ERR_INVALID_DATA;
    }

    if (!data.WriteInt32(recordId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write record id failed.");
        return ERR_INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = SendTransactCmd(static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_NOTIFY_LOAD_REPAIR_PATCH),
        data, reply, option);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "ScheduleNotifyLoadRepairPatch, Send request failed with errno %{public}d.", ret);
        return ret;
    }

    return reply.ReadInt32();
}

int32_t AppSchedulerProxy::ScheduleNotifyHotReloadPage(const sptr<IQuickFixCallback> &callback, const int32_t recordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed!");
        return ERR_INVALID_DATA;
    }

    if (callback == nullptr || !data.WriteRemoteObject(callback->AsObject())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write callback failed.");
        return ERR_INVALID_DATA;
    }

    if (!data.WriteInt32(recordId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write record id failed.");
        return ERR_INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option;
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_NOTIFY_HOT_RELOAD_PAGE),
        data, reply, option);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request failed with errno %{public}d.", ret);
        return ret;
    }

    return reply.ReadInt32();
}

int32_t AppSchedulerProxy::ScheduleNotifyUnLoadRepairPatch(const std::string &bundleName,
    const sptr<IQuickFixCallback> &callback, const int32_t recordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Schedule notify unload patch, Write interface token failed.");
        return ERR_INVALID_DATA;
    }

    if (!data.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Schedule notify unload patch, Write bundle name failed.");
        return ERR_INVALID_DATA;
    }

    if (callback == nullptr || !data.WriteRemoteObject(callback->AsObject())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write callback failed.");
        return ERR_INVALID_DATA;
    }

    if (!data.WriteInt32(recordId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write record id failed.");
        return ERR_INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_NOTIFY_UNLOAD_REPAIR_PATCH), data, reply, option);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "Schedule notify unload patch, Send request failed with errno %{public}d.", ret);
        return ret;
    }

    return reply.ReadInt32();
}

int32_t AppSchedulerProxy::ScheduleNotifyAppFault(const FaultData &faultData)
{
    MessageParcel data;

    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return ERR_INVALID_DATA;
    }

    if (!data.WriteParcelable(&faultData)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write FaultData error.");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_NOTIFY_FAULT),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request failed with error code %{public}d.", ret);
        return ret;
    }

    return reply.ReadInt32();
}

int32_t AppSchedulerProxy::ScheduleChangeAppGcState(int32_t state, uint64_t tid)
{
    MessageParcel data;

    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed");
        return ERR_INVALID_DATA;
    }

    data.WriteInt32(state);
    data.WriteUint64(tid);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAppScheduler::Message::APP_GC_STATE_CHANGE),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request failed with error code %{public}d.", ret);
        return ret;
    }

    return NO_ERROR;
}

void AppSchedulerProxy::AttachAppDebug(bool isDebugFromLocal)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return;
    }
    if (!data.WriteBool(isDebugFromLocal)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write AttachAppDebug isDebugFromLocal failed");
        return;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    auto ret = SendTransactCmd(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_ATTACH_APP_DEBUG), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to send request with error code: %{public}d", ret);
    }
}

void AppSchedulerProxy::DetachAppDebug()
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    auto ret = SendTransactCmd(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_DETACH_APP_DEBUG), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request failed with error code: %{public}d", ret);
    }
}

int32_t AppSchedulerProxy::ScheduleDumpIpcStart(std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "AppSchedulerProxy::ScheduleDumpIpcStart start");
    uint32_t operation = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_DUMP_IPC_START);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        result.append(MSG_DUMP_IPC_START_STAT, strlen(MSG_DUMP_IPC_START_STAT))
            .append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INTERNAL, strlen(MSG_DUMP_FAIL_REASON_INTERNAL));
        TAG_LOGE(AAFwkTag::APPMGR, "AppSchedulerProxy !WriteInterfaceToken.");
        return DumpErrorCode::ERR_INTERNAL_ERROR;
    }
    int32_t ret = SendTransactCmd(operation, data, reply, option);
    if (ret != NO_ERROR) {
        result.append(MSG_DUMP_IPC_START_STAT, strlen(MSG_DUMP_IPC_START_STAT))
            .append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INTERNAL, strlen(MSG_DUMP_FAIL_REASON_INTERNAL));
        TAG_LOGE(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
        return DumpErrorCode::ERR_INTERNAL_ERROR;
    }
    if (!reply.ReadString(result)) {
        result.append(MSG_DUMP_IPC_START_STAT, strlen(MSG_DUMP_IPC_START_STAT))
            .append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INTERNAL, strlen(MSG_DUMP_FAIL_REASON_INTERNAL));
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to read string of ScheduleDumpIpcStart result");
        return DumpErrorCode::ERR_INTERNAL_ERROR;
    }
    return DumpErrorCode::ERR_OK;
}

int32_t AppSchedulerProxy::ScheduleDumpIpcStop(std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "AppSchedulerProxy::ScheduleDumpIpcStop start");
    uint32_t operation = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_DUMP_IPC_STOP);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        result.append(MSG_DUMP_IPC_STOP_STAT, strlen(MSG_DUMP_IPC_STOP_STAT))
            .append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INTERNAL, strlen(MSG_DUMP_FAIL_REASON_INTERNAL));
        TAG_LOGE(AAFwkTag::APPMGR, "AppSchedulerProxy !WriteInterfaceToken.");
        return DumpErrorCode::ERR_INTERNAL_ERROR;
    }
    int32_t ret = SendTransactCmd(operation, data, reply, option);
    if (ret != NO_ERROR) {
        result.append(MSG_DUMP_IPC_STOP_STAT, strlen(MSG_DUMP_IPC_STOP_STAT))
            .append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INTERNAL, strlen(MSG_DUMP_FAIL_REASON_INTERNAL));
        TAG_LOGE(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
        return DumpErrorCode::ERR_INTERNAL_ERROR;
    }
    if (!reply.ReadString(result)) {
        result.append(MSG_DUMP_IPC_STOP_STAT, strlen(MSG_DUMP_IPC_STOP_STAT))
            .append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INTERNAL, strlen(MSG_DUMP_FAIL_REASON_INTERNAL));
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to read string of ScheduleDumpIpcStop result");
        return DumpErrorCode::ERR_INTERNAL_ERROR;
    }
    return DumpErrorCode::ERR_OK;
}

int32_t AppSchedulerProxy::ScheduleDumpIpcStat(std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "AppSchedulerProxy::ScheduleDumpIpcStat start");
    uint32_t operation = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_DUMP_IPC_STAT);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        result.append(MSG_DUMP_IPC_STAT, strlen(MSG_DUMP_IPC_STAT))
            .append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INTERNAL, strlen(MSG_DUMP_FAIL_REASON_INTERNAL));
        TAG_LOGE(AAFwkTag::APPMGR, "AppSchedulerProxy !WriteInterfaceToken.");
        return DumpErrorCode::ERR_INTERNAL_ERROR;
    }
    int32_t ret = SendTransactCmd(operation, data, reply, option);
    if (ret != NO_ERROR) {
        result.append(MSG_DUMP_IPC_STAT, strlen(MSG_DUMP_IPC_STAT))
            .append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INTERNAL, strlen(MSG_DUMP_FAIL_REASON_INTERNAL));
        TAG_LOGE(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
        return DumpErrorCode::ERR_INTERNAL_ERROR;
    }
    if (!reply.ReadString(result)) {
        result.append(MSG_DUMP_IPC_STAT, strlen(MSG_DUMP_IPC_STAT))
            .append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INTERNAL, strlen(MSG_DUMP_FAIL_REASON_INTERNAL));
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to read string of ScheduleDumpIpcStat result");
        return DumpErrorCode::ERR_INTERNAL_ERROR;
    }
    return DumpErrorCode::ERR_OK;
}

int32_t AppSchedulerProxy::ScheduleDumpFfrt(std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "AppSchedulerProxy::ScheduleDumpFfrt start");
    uint32_t operation = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_DUMP_FFRT);
    MessageParcel data;
    MessageParcel reply;
    reply.SetMaxCapacity(MAX_CAPACITY);
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        result.append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INTERNAL, strlen(MSG_DUMP_FAIL_REASON_INTERNAL));
        TAG_LOGE(AAFwkTag::APPMGR, "AppSchedulerProxy !WriteInterfaceToken.");
        return DumpErrorCode::ERR_INTERNAL_ERROR;
    }
    int32_t ret = SendTransactCmd(operation, data, reply, option);
    if (ret != NO_ERROR) {
        result.append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INTERNAL, strlen(MSG_DUMP_FAIL_REASON_INTERNAL));
        TAG_LOGE(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
        return DumpErrorCode::ERR_INTERNAL_ERROR;
    }
    if (!reply.ReadString(result)) {
        result.append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INTERNAL, strlen(MSG_DUMP_FAIL_REASON_INTERNAL));
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to read string of ScheduleDumpFfrt result");
        return DumpErrorCode::ERR_INTERNAL_ERROR;
    }
    return DumpErrorCode::ERR_OK;
}

int32_t AppSchedulerProxy::ScheduleDumpArkWeb(const std::string &customArgs, std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "ScheduleDumpArkWeb start");
    uint32_t operation = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_DUMP_ARKWEB);
    MessageParcel data;
    MessageParcel reply;
    reply.SetMaxCapacity(MAX_CAPACITY);
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        result.append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INTERNAL, strlen(MSG_DUMP_FAIL_REASON_INTERNAL));
        TAG_LOGE(AAFwkTag::APPMGR, "WriteInterfaceToken failed");
        return DumpErrorCode::ERR_INTERNAL_ERROR;
    }
    if (!data.WriteString(customArgs)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write customArgs failed");
        return DumpErrorCode::ERR_INTERNAL_ERROR;
    }
    int32_t ret = SendTransactCmd(operation, data, reply, option);
    if (ret != NO_ERROR) {
        result.append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INTERNAL, strlen(MSG_DUMP_FAIL_REASON_INTERNAL));
        TAG_LOGE(AAFwkTag::APPMGR, "SendRequest failed, error code: %{public}d", ret);
        return DumpErrorCode::ERR_INTERNAL_ERROR;
    }
    if (!reply.ReadString(result)) {
        result.append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INTERNAL, strlen(MSG_DUMP_FAIL_REASON_INTERNAL));
        TAG_LOGE(AAFwkTag::APPMGR, "read ScheduleDumpArkWeb result failed");
        return DumpErrorCode::ERR_INTERNAL_ERROR;
    }
    return DumpErrorCode::ERR_OK;
}

void AppSchedulerProxy::SetWatchdogBackgroundStatus(bool status)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteBool(status)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write bool failed.");
        return;
    }
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAppScheduler::Message::WATCHDOG_BACKGROUND_STATUS_TRANSACTION),
            data,
            reply,
            option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is wrong, error code: %{public}d", ret);
    }
}

int32_t AppSchedulerProxy::SendTransactCmd(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Remote is nullptr.");
        return ERR_NULL_OBJECT;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "remote->SendRequest");
    auto ret = remote->SendRequest(code, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request failed with error code: %{public}d", ret);
        return ret;
    }
    return ret;
}

void AppSchedulerProxy::ScheduleCacheProcess()
{
    uint32_t operation = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_CACHE_PROCESS);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    int32_t ret = SendTransactCmd(operation, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
}

void AppSchedulerProxy::OnLoadAbilityFinished(uint64_t callbackId, int32_t pid)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteUint64(callbackId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write callbackId failed.");
        return;
    }
    if (!data.WriteInt32(pid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write pid failed.");
        return;
    }
    int32_t ret = SendTransactCmd(static_cast<uint32_t>(IAppScheduler::Message::ON_LOAD_ABILITY_FINISHED),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is wrong, error code: %{public}d", ret);
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
