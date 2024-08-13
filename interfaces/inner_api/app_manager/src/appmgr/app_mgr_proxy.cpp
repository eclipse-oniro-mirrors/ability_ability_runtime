/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "app_mgr_proxy.h"

#include "appexecfwk_errors.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "ipc_types.h"
#include "iremote_object.h"
#include "parcel_util.h"

namespace OHOS {
namespace AppExecFwk {
constexpr int32_t CYCLE_LIMIT = 1000;
AppMgrProxy::AppMgrProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IAppMgr>(impl)
{}

bool AppMgrProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(AppMgrProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::APPMGR, "write interface token failed");
        return false;
    }
    return true;
}

void AppMgrProxy::AttachApplication(const sptr<IRemoteObject> &obj)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (obj == nullptr || obj.GetRefPtr() == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "app scheduler null");
    }
    PARCEL_UTIL_WRITE_NORET(data, RemoteObject, obj.GetRefPtr());

    PARCEL_UTIL_SENDREQ_NORET(AppMgrInterfaceCode::APP_ATTACH_APPLICATION, data, reply, option);
}

int32_t AppMgrProxy::PreloadApplication(const std::string &bundleName, int32_t userId,
    AppExecFwk::PreloadMode preloadMode, int32_t appIndex)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "PreloadApplication Write interface token failed.");
        return IPC_PROXY_ERR;
    }
    PARCEL_UTIL_WRITE_RET_INT(data, String16, Str8ToStr16(bundleName));
    PARCEL_UTIL_WRITE_RET_INT(data, Int32, userId);
    PARCEL_UTIL_WRITE_RET_INT(data, Int32, static_cast<int32_t>(preloadMode));
    PARCEL_UTIL_WRITE_RET_INT(data, Int32, appIndex);

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::PRELOAD_APPLICATION, data, reply, option);
    return reply.ReadInt32();
}

void AppMgrProxy::ApplicationForegrounded(const int32_t recordId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    PARCEL_UTIL_WRITE_NORET(data, Int32, recordId);

    PARCEL_UTIL_SENDREQ_NORET(AppMgrInterfaceCode::APP_APPLICATION_FOREGROUNDED, data, reply, option);
}

void AppMgrProxy::ApplicationBackgrounded(const int32_t recordId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    PARCEL_UTIL_WRITE_NORET(data, Int32, recordId);

    PARCEL_UTIL_SENDREQ_NORET(AppMgrInterfaceCode::APP_APPLICATION_BACKGROUNDED, data, reply, option);
}

void AppMgrProxy::ApplicationTerminated(const int32_t recordId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    PARCEL_UTIL_WRITE_NORET(data, Int32, recordId);

    PARCEL_UTIL_SENDREQ_NORET(AppMgrInterfaceCode::APP_APPLICATION_TERMINATED, data, reply, option);
}

void AppMgrProxy::AbilityCleaned(const sptr<IRemoteObject> &token)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    PARCEL_UTIL_WRITE_NORET(data, RemoteObject, token.GetRefPtr());

    PARCEL_UTIL_SENDREQ_NORET(AppMgrInterfaceCode::APP_ABILITY_CLEANED, data, reply, option);
}

sptr<IAmsMgr> AppMgrProxy::GetAmsMgr()
{
    MessageParcel data;
    MessageParcel reply;
    if (!WriteInterfaceToken(data)) {
        return nullptr;
    }
    if (!SendTransactCmd(AppMgrInterfaceCode::APP_GET_MGR_INSTANCE, data, reply)) {
        return nullptr;
    }
    sptr<IRemoteObject> object = reply.ReadRemoteObject();
    sptr<IAmsMgr> amsMgr = iface_cast<IAmsMgr>(object);
    if (!amsMgr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Ability manager service instance is nullptr. ");
        return nullptr;
    }
    return amsMgr;
}

int32_t AppMgrProxy::ClearUpApplicationData(const std::string &bundleName, int32_t appCloneIndex, const int32_t userId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_UTIL_WRITE_RET_INT(data, String, bundleName);
    PARCEL_UTIL_WRITE_RET_INT(data, Int32, appCloneIndex);
    PARCEL_UTIL_WRITE_RET_INT(data, Int32, userId);

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::APP_CLEAR_UP_APPLICATION_DATA, data, reply, option);
    return reply.ReadInt32();
}

int32_t AppMgrProxy::ClearUpApplicationDataBySelf(int32_t userId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_UTIL_WRITE_RET_INT(data, Int32, userId);

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::APP_CLEAR_UP_APPLICATION_DATA_BY_SELF, data, reply, option);
    return reply.ReadInt32();
}

int32_t AppMgrProxy::GetAllRunningProcesses(std::vector<RunningProcessInfo> &info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (!SendTransactCmd(AppMgrInterfaceCode::APP_GET_ALL_RUNNING_PROCESSES, data, reply)) {
        return ERR_NULL_OBJECT;
    }
    auto error = GetParcelableInfos<RunningProcessInfo>(reply, info);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetParcelableInfos fail, error: %{public}d", error);
        return error;
    }
    int result = reply.ReadInt32();
    return result;
}

int32_t AppMgrProxy::GetRunningMultiAppInfoByBundleName(const std::string &bundleName,
    RunningMultiAppInfo &info)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_UTIL_WRITE_RET_INT(data, String, bundleName);

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::GET_RUNNING_MULTIAPP_INFO_BY_BUNDLENAME, data, reply, option);
    std::unique_ptr<RunningMultiAppInfo> infoReply(reply.ReadParcelable<RunningMultiAppInfo>());
    if (infoReply == nullptr) {
        TAG_LOGW(AAFwkTag::APPMGR, "reply ReadParcelable is nullptr");
        return ERR_NULL_OBJECT;
    }
    info = *infoReply;
    int result = reply.ReadInt32();
    return result;
}

int32_t AppMgrProxy::GetRunningProcessesByBundleType(const BundleType bundleType,
    std::vector<RunningProcessInfo> &info)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_UTIL_WRITE_RET_INT(data, Int32, static_cast<int32_t>(bundleType));

    if (!SendTransactCmd(AppMgrInterfaceCode::APP_GET_RUNNING_PROCESSES_BY_BUNDLE_TYPE, data, reply)) {
        return ERR_NULL_OBJECT;
    }
    auto error = GetParcelableInfos<RunningProcessInfo>(reply, info);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetParcelableInfos fail, error: %{public}d", error);
        return error;
    }
    int result = reply.ReadInt32();
    return result;
}

int32_t AppMgrProxy::GetAllRenderProcesses(std::vector<RenderProcessInfo> &info)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (!SendTransactCmd(AppMgrInterfaceCode::APP_GET_ALL_RENDER_PROCESSES, data, reply)) {
        return ERR_NULL_OBJECT;
    }
    auto error = GetParcelableInfos<RenderProcessInfo>(reply, info);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetParcelableInfos fail, error: %{public}d", error);
        return error;
    }
    int result = reply.ReadInt32();
    return result;
}

int32_t AppMgrProxy::JudgeSandboxByPid(pid_t pid, bool &isSandbox)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_UTIL_WRITE_RET_INT(data, Int32, pid);

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::JUDGE_SANDBOX_BY_PID, data, reply, option);
    isSandbox = reply.ReadBool();
    return reply.ReadInt32();
}

int32_t AppMgrProxy::GetProcessRunningInfosByUserId(std::vector<RunningProcessInfo> &info, int32_t userId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_UTIL_WRITE_RET_INT(data, Int32, userId);

    if (!SendTransactCmd(AppMgrInterfaceCode::APP_GET_RUNNING_PROCESSES_BY_USER_ID, data, reply)) {
        return ERR_NULL_OBJECT;
    }
    auto error = GetParcelableInfos<RunningProcessInfo>(reply, info);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetParcelableInfos fail, error: %{public}d", error);
        return error;
    }
    int result = reply.ReadInt32();
    return result;
}

int32_t AppMgrProxy::GetProcessRunningInformation(RunningProcessInfo &info)
{
    MessageParcel data;
    MessageParcel reply;

    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (!SendTransactCmd(AppMgrInterfaceCode::APP_GET_PROCESS_RUNNING_INFORMATION, data, reply)) {
        return ERR_NULL_OBJECT;
    }
    std::unique_ptr<RunningProcessInfo> infoReply(reply.ReadParcelable<RunningProcessInfo>());
    if (infoReply == nullptr) {
        TAG_LOGW(AAFwkTag::APPMGR, "reply ReadParcelable is nullptr");
        return ERR_NULL_OBJECT;
    }
    info = *infoReply;
    return reply.ReadInt32();
}

int32_t AppMgrProxy::NotifyMemoryLevel(int32_t level)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_UTIL_WRITE_RET_INT(data, Int32, level);

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::APP_NOTIFY_MEMORY_LEVEL, data, reply, option);
    return reply.ReadInt32();
}

int32_t AppMgrProxy::NotifyProcMemoryLevel(const std::map<pid_t, MemoryLevel> &procLevelMap)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    MemoryLevelInfo memoryLevelInfo(procLevelMap);
    PARCEL_UTIL_WRITE_RET_INT(data, Parcelable, &memoryLevelInfo);

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::APP_NOTIFY_PROC_MEMORY_LEVEL, data, reply, option);
    return reply.ReadInt32();
}

int32_t AppMgrProxy::DumpHeapMemory(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo)
{
    TAG_LOGD(AAFwkTag::APPMGR, "AppMgrProxy::DumpHeapMemory.");
    MessageParcel data;
    MessageParcel reply;
    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_UTIL_WRITE_RET_INT(data, Int32, pid);

    MessageOption option(MessageOption::TF_SYNC);
    int32_t ret = SendRequest(AppMgrInterfaceCode::DUMP_HEAP_MEMORY_PROCESS, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppMgrProxy SendRequest is failed, error code: %{public}d", ret);
        return ret;
    }

    std::unique_ptr<MallocInfo> info(reply.ReadParcelable<MallocInfo>());
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "MallocInfo ReadParcelable nullptr");
        return ERR_NULL_OBJECT;
    }
    mallocInfo = *info;
    return ret;
}

int32_t AppMgrProxy::DumpJsHeapMemory(OHOS::AppExecFwk::JsHeapDumpInfo &info)
{
    TAG_LOGD(AAFwkTag::APPMGR, "AppMgrProxy::DumpJsHeapMemory.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_UTIL_WRITE_RET_INT(data, Parcelable, &info);

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::DUMP_JSHEAP_MEMORY_PROCESS, data, reply, option);
    return reply.ReadInt32();
}

bool AppMgrProxy::SendTransactCmd(AppMgrInterfaceCode code, MessageParcel &data, MessageParcel &reply)
{
    MessageOption option(MessageOption::TF_SYNC);
    int32_t result = SendRequest(code, data, reply, option);
    if (result != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "receive error transact code %{public}d in transact cmd %{public}d", result, code);
        return false;
    }
    return true;
}

void AppMgrProxy::AddAbilityStageDone(const int32_t recordId)
{
    MessageParcel data;
    MessageParcel reply;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteInt32(recordId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "want write failed.");
        return;
    }

    if (!SendTransactCmd(AppMgrInterfaceCode::APP_ADD_ABILITY_STAGE_INFO_DONE, data, reply)) {
        TAG_LOGE(AAFwkTag::APPMGR, "SendTransactCmd failed");
        return;
    }
    return;
}

void AppMgrProxy::StartupResidentProcess(const std::vector<AppExecFwk::BundleInfo> &bundleInfos)
{
    MessageParcel data;
    MessageParcel reply;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteInt32(bundleInfos.size())) {
        TAG_LOGE(AAFwkTag::APPMGR, "write bundle info size failed.");
        return;
    }

    for (auto &bundleInfo : bundleInfos) {
        if (!data.WriteParcelable(&bundleInfo)) {
            TAG_LOGE(AAFwkTag::APPMGR, "write bundle info failed");
            return;
        }
    }

    if (!SendTransactCmd(AppMgrInterfaceCode::STARTUP_RESIDENT_PROCESS, data, reply)) {
        TAG_LOGE(AAFwkTag::APPMGR, "SendTransactCmd failed");
        return;
    }
    return;
}

template<typename T>
int AppMgrProxy::GetParcelableInfos(MessageParcel &reply, std::vector<T> &parcelableInfos)
{
    int32_t infoSize = reply.ReadInt32();
    if (infoSize > CYCLE_LIMIT) {
        TAG_LOGE(AAFwkTag::APPMGR, "infoSize is too large");
        return ERR_INVALID_VALUE;
    }
    for (int32_t i = 0; i < infoSize; i++) {
        std::unique_ptr<T> info(reply.ReadParcelable<T>());
        if (!info) {
            TAG_LOGE(AAFwkTag::APPMGR, "Read Parcelable infos failed");
            return ERR_INVALID_VALUE;
        }
        parcelableInfos.emplace_back(*info);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "get parcelable infos success");
    return NO_ERROR;
}

int AppMgrProxy::RegisterApplicationStateObserver(
    const sptr<IApplicationStateObserver> &observer, const std::vector<std::string> &bundleNameList)
{
    if (!observer) {
        TAG_LOGE(AAFwkTag::APPMGR, "observer null");
        return ERR_INVALID_VALUE;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "RegisterApplicationStateObserver start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteRemoteObject(observer->AsObject())) {
        TAG_LOGE(AAFwkTag::APPMGR, "observer write failed.");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteStringVector(bundleNameList)) {
        TAG_LOGE(AAFwkTag::APPMGR, "bundleNameList write failed.");
        return ERR_FLATTEN_OBJECT;
    }

    auto error = SendRequest(AppMgrInterfaceCode::REGISTER_APPLICATION_STATE_OBSERVER,
        data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AppMgrProxy::UnregisterApplicationStateObserver(
    const sptr<IApplicationStateObserver> &observer)
{
    if (!observer) {
        TAG_LOGE(AAFwkTag::APPMGR, "observer null");
        return ERR_INVALID_VALUE;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "UnregisterApplicationStateObserver start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_UTIL_WRITE_RET_INT(data, RemoteObject, observer->AsObject());

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::UNREGISTER_APPLICATION_STATE_OBSERVER, data, reply, option);
    return reply.ReadInt32();
}

int32_t AppMgrProxy::RegisterAbilityForegroundStateObserver(const sptr<IAbilityForegroundStateObserver> &observer)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (observer == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Observer is null.");
        return ERR_INVALID_VALUE;
    }

    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_UTIL_WRITE_RET_INT(data, RemoteObject, observer->AsObject());
    MessageParcel reply;
    MessageOption option;

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::REGISTER_ABILITY_FOREGROUND_STATE_OBSERVER, data, reply, option);
    return reply.ReadInt32();
}

int32_t AppMgrProxy::UnregisterAbilityForegroundStateObserver(const sptr<IAbilityForegroundStateObserver> &observer)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (observer == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Observer is null.");
        return ERR_INVALID_VALUE;
    }

    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_UTIL_WRITE_RET_INT(data, RemoteObject, observer->AsObject());
    MessageParcel reply;
    MessageOption option;
    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::UNREGISTER_ABILITY_FOREGROUND_STATE_OBSERVER, data, reply, option);
    return reply.ReadInt32();
}

int AppMgrProxy::GetForegroundApplications(std::vector<AppStateData> &list)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::GET_FOREGROUND_APPLICATIONS, data, reply, option);
    int32_t infoSize = reply.ReadInt32();
    if (infoSize > CYCLE_LIMIT) {
        TAG_LOGE(AAFwkTag::APPMGR, "infoSize is too large");
        return ERR_INVALID_VALUE;
    }
    for (int32_t i = 0; i < infoSize; i++) {
        std::unique_ptr<AppStateData> info(reply.ReadParcelable<AppStateData>());
        if (!info) {
            TAG_LOGE(AAFwkTag::APPMGR, "Read Parcelable infos failed.");
            return ERR_INVALID_VALUE;
        }
        list.emplace_back(*info);
    }
    return reply.ReadInt32();
}

int AppMgrProxy::StartUserTestProcess(
    const AAFwk::Want &want, const sptr<IRemoteObject> &observer, const BundleInfo &bundleInfo, int32_t userId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_UTIL_WRITE_RET_INT(data, Parcelable, &want);
    PARCEL_UTIL_WRITE_RET_INT(data, RemoteObject, observer);
    PARCEL_UTIL_WRITE_RET_INT(data, Parcelable, &bundleInfo);
    PARCEL_UTIL_WRITE_RET_INT(data, Int32, userId);

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::START_USER_TEST_PROCESS, data, reply, option);
    return reply.ReadInt32();
}

int AppMgrProxy::FinishUserTest(const std::string &msg, const int64_t &resultCode, const std::string &bundleName)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }

    PARCEL_UTIL_WRITE_RET_INT(data, String, msg);
    PARCEL_UTIL_WRITE_RET_INT(data, Int64, resultCode);
    PARCEL_UTIL_WRITE_RET_INT(data, String, bundleName);

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::FINISH_USER_TEST, data, reply, option);
    return reply.ReadInt32();
}

void AppMgrProxy::ScheduleAcceptWantDone(const int32_t recordId, const AAFwk::Want &want, const std::string &flag)
{
    MessageParcel data;
    MessageParcel reply;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteInt32(recordId) || !data.WriteParcelable(&want) || !data.WriteString(flag)) {
        TAG_LOGE(AAFwkTag::APPMGR, "want write failed.");
        return;
    }

    if (!SendTransactCmd(AppMgrInterfaceCode::SCHEDULE_ACCEPT_WANT_DONE, data, reply)) {
        TAG_LOGE(AAFwkTag::APPMGR, "SendTransactCmd failed");
        return;
    }
}

void AppMgrProxy::ScheduleNewProcessRequestDone(const int32_t recordId, const AAFwk::Want &want,
    const std::string &flag)
{
    MessageParcel data;
    MessageParcel reply;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteInt32(recordId) || !data.WriteParcelable(&want) || !data.WriteString(flag)) {
        TAG_LOGE(AAFwkTag::APPMGR, "want write failed.");
        return;
    }

    if (!SendTransactCmd(AppMgrInterfaceCode::SCHEDULE_NEW_PROCESS_REQUEST_DONE, data, reply)) {
        TAG_LOGE(AAFwkTag::APPMGR, "SendTransactCmd failed");
        return;
    }
}

int AppMgrProxy::GetAbilityRecordsByProcessID(const int pid, std::vector<sptr<IRemoteObject>> &tokens)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    data.WriteInt32(pid);
    if (!SendTransactCmd(AppMgrInterfaceCode::APP_GET_ABILITY_RECORDS_BY_PROCESS_ID, data, reply)) {
        return ERR_NULL_OBJECT;
    }
    int32_t infoSize = reply.ReadInt32();
    if (infoSize > CYCLE_LIMIT) {
        TAG_LOGE(AAFwkTag::APPMGR, "infoSize is too large");
        return ERR_INVALID_VALUE;
    }
    for (int32_t i = 0; i < infoSize; i++) {
        auto iRemote = reply.ReadRemoteObject();
        tokens.emplace_back(iRemote);
    }
    return reply.ReadInt32();
}

int AppMgrProxy::PreStartNWebSpawnProcess()
{
    TAG_LOGI(AAFwkTag::APPMGR, "PreStartNWebSpawnProcess");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "WriteInterfaceToken failed");
        return ERR_FLATTEN_OBJECT;
    }

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::PRE_START_NWEBSPAWN_PROCESS, data, reply, option);
    auto result = reply.ReadInt32();
    if (result != 0) {
        TAG_LOGW(AAFwkTag::APPMGR, "PreStartNWebSpawnProcess failed, result: %{public}d", result);
    }
    return result;
}

int AppMgrProxy::StartRenderProcess(const std::string &renderParam,
                                    int32_t ipcFd, int32_t sharedFd,
                                    int32_t crashFd, pid_t &renderPid, bool isGPU)
{
    if (renderParam.empty() || ipcFd <= 0 || sharedFd <= 0 || crashFd <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "Invalid params, renderParam:%{private}s, ipcFd:%{public}d, "
            "sharedFd:%{public}d, crashFd:%{public}d", renderParam.c_str(), ipcFd, sharedFd, crashFd);
        return -1;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "WriteInterfaceToken failed");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteString(renderParam)) {
        TAG_LOGE(AAFwkTag::APPMGR, "want paramSize failed.");
        return -1;
    }

    if (!data.WriteFileDescriptor(ipcFd) || !data.WriteFileDescriptor(sharedFd) ||
        !data.WriteFileDescriptor(crashFd)) {
        TAG_LOGE(AAFwkTag::APPMGR, "want fd failed, ipcFd:%{public}d, sharedFd:%{public}d, "
            "crashFd:%{public}d", ipcFd, sharedFd, crashFd);
        return -1;
    }

    if (!data.WriteBool(isGPU)) {
        TAG_LOGE(AAFwkTag::APPMGR, "want processType failed.");
        return -1;
    }

    int32_t ret = SendRequest(AppMgrInterfaceCode::START_RENDER_PROCESS, data,
        reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "StartRenderProcess SendRequest is failed, error code: %{public}d", ret);
        return ret;
    }

    auto result = reply.ReadInt32();
    renderPid = reply.ReadInt32();
    if (result != 0) {
        TAG_LOGW(AAFwkTag::APPMGR, "StartRenderProcess failed, result: %{public}d", result);
    }
    return result;
}

void AppMgrProxy::AttachRenderProcess(const sptr<IRemoteObject> &renderScheduler)
{
    if (!renderScheduler) {
        TAG_LOGE(AAFwkTag::APPMGR, "renderScheduler is null");
        return;
    }

    TAG_LOGD(AAFwkTag::APPMGR, "AttachRenderProcess start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteRemoteObject(renderScheduler)) {
        TAG_LOGE(AAFwkTag::APPMGR, "renderScheduler write failed.");
        return;
    }

    if (!SendTransactCmd(AppMgrInterfaceCode::ATTACH_RENDER_PROCESS, data, reply)) {
        TAG_LOGE(AAFwkTag::APPMGR, "SendTransactCmd ATTACH_RENDER_PROCESS failed");
        return;
    }
}

void AppMgrProxy::SaveBrowserChannel(sptr<IRemoteObject> browser)
{
    if (!browser) {
        TAG_LOGE(AAFwkTag::APPMGR, "browser is null");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteRemoteObject(browser)) {
        TAG_LOGE(AAFwkTag::APPMGR, "browser write failed.");
        return;
    }

    if (!SendTransactCmd(AppMgrInterfaceCode::SAVE_BROWSER_CHANNEL, data, reply)) {
        TAG_LOGE(AAFwkTag::APPMGR, "SendTransactCmd SAVE_BROWSER_CHANNEL failed");
        return;
    }
}

int AppMgrProxy::GetRenderProcessTerminationStatus(pid_t renderPid, int &status)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "WriteInterfaceToken failed");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteInt32(renderPid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write renderPid failed.");
        return -1;
    }

    int32_t ret = SendRequest(AppMgrInterfaceCode::GET_RENDER_PROCESS_TERMINATION_STATUS, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "GetRenderProcessTerminationStatus SendRequest is failed, error code: %{public}d",
            ret);
        return ret;
    }

    auto result = reply.ReadInt32();
    if (result != 0) {
        TAG_LOGW(AAFwkTag::APPMGR, "GetRenderProcessTerminationStatus failed, result: %{public}d", result);
        return result;
    }
    status = reply.ReadInt32();
    return 0;
}

int32_t AppMgrProxy::UpdateConfiguration(const Configuration &config)
{
    TAG_LOGI(AAFwkTag::APPMGR, "AppMgrProxy UpdateConfiguration");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_INVALID_DATA;
    }
    if (!data.WriteParcelable(&config)) {
        TAG_LOGE(AAFwkTag::APPMGR, "parcel config failed");
        return ERR_INVALID_DATA;
    }
    int32_t ret = SendRequest(AppMgrInterfaceCode::UPDATE_CONFIGURATION, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AppMgrProxy::UpdateConfigurationByBundleName(const Configuration &config, const std::string &name)
{
    TAG_LOGI(AAFwkTag::APPMGR, "AppMgrProxy UpdateConfigurationByBundleName");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return ERR_INVALID_DATA;
    }
    if (!data.WriteParcelable(&config)) {
        TAG_LOGE(AAFwkTag::APPMGR, "parcel config failed");
        return ERR_INVALID_DATA;
    }
    if (!data.WriteString(name)) {
        TAG_LOGE(AAFwkTag::APPMGR, "parcel name failed");
        return ERR_INVALID_DATA;
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    int32_t ret = SendRequest(AppMgrInterfaceCode::UPDATE_CONFIGURATION_BY_BUNDLE_NAME, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AppMgrProxy::GetConfiguration(Configuration &config)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "parcel data failed");
        return ERR_INVALID_DATA;
    }
    int32_t ret = SendRequest(AppMgrInterfaceCode::GET_CONFIGURATION, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
        return ret;
    }

    std::unique_ptr<Configuration> info(reply.ReadParcelable<Configuration>());
    if (!info) {
        TAG_LOGE(AAFwkTag::APPMGR, "read configuration failed.");
        return ERR_UNKNOWN_OBJECT;
    }
    config = *info;
    return reply.ReadInt32();
}

int32_t AppMgrProxy::RegisterConfigurationObserver(const sptr<IConfigurationObserver> &observer)
{
    if (!observer) {
        TAG_LOGE(AAFwkTag::APPMGR, "observer null");
        return ERR_INVALID_VALUE;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "RegisterConfigurationObserver start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteRemoteObject(observer->AsObject())) {
        TAG_LOGE(AAFwkTag::APPMGR, "observer write failed.");
        return ERR_FLATTEN_OBJECT;
    }

    auto error = SendRequest(AppMgrInterfaceCode::REGISTER_CONFIGURATION_OBSERVER,
        data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int32_t AppMgrProxy::UnregisterConfigurationObserver(const sptr<IConfigurationObserver> &observer)
{
    TAG_LOGD(AAFwkTag::APPMGR, "UnregisterConfigurationObserver start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteRemoteObject(observer->AsObject())) {
        TAG_LOGE(AAFwkTag::APPMGR, "observer write failed.");
        return ERR_FLATTEN_OBJECT;
    }

    auto error = SendRequest(AppMgrInterfaceCode::UNREGISTER_CONFIGURATION_OBSERVER,
        data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

#ifdef ABILITY_COMMAND_FOR_TEST
int AppMgrProxy::BlockAppService()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }

    int32_t ret = SendRequest(AppMgrInterfaceCode::BLOCK_APP_SERVICE, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}
#endif

bool AppMgrProxy::GetAppRunningStateByBundleName(const std::string &bundleName)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return false;
    }

    if (!data.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write bundle name failed.");
        return false;
    }

    MessageParcel reply;
    MessageOption option;
    auto ret = SendRequest(AppMgrInterfaceCode::GET_APP_RUNNING_STATE,
        data, reply, option);
    if (ret != 0) {
        TAG_LOGW(AAFwkTag::APPMGR, "Send request failed with error code %{public}d.", ret);
        return false;
    }

    return reply.ReadBool();
}

int32_t AppMgrProxy::NotifyLoadRepairPatch(const std::string &bundleName, const sptr<IQuickFixCallback> &callback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "NotifyLoadRepairPatch, Write interface token failed.");
        return ERR_INVALID_DATA;
    }

    if (!data.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "NotifyLoadRepairPatch, Write bundle name failed.");
        return ERR_INVALID_DATA;
    }

    if (callback == nullptr || !data.WriteRemoteObject(callback->AsObject())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write callback failed.");
        return ERR_INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option;
    auto ret = SendRequest(AppMgrInterfaceCode::NOTIFY_LOAD_REPAIR_PATCH,
        data, reply, option);
    if (ret != 0) {
        TAG_LOGW(AAFwkTag::APPMGR, "NotifyLoadRepairPatch, Send request failed with error code %{public}d.", ret);
        return ret;
    }

    return reply.ReadInt32();
}

int32_t AppMgrProxy::NotifyHotReloadPage(const std::string &bundleName, const sptr<IQuickFixCallback> &callback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return ERR_INVALID_DATA;
    }

    if (!data.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write bundle name failed.");
        return ERR_INVALID_DATA;
    }

    if (callback == nullptr || !data.WriteRemoteObject(callback->AsObject())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write callback failed.");
        return ERR_INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option;
    auto ret = SendRequest(AppMgrInterfaceCode::NOTIFY_HOT_RELOAD_PAGE,
        data, reply, option);
    if (ret != 0) {
        TAG_LOGW(AAFwkTag::APPMGR, "Send request failed with error code %{public}d.", ret);
        return ret;
    }

    return reply.ReadInt32();
}

#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
int32_t AppMgrProxy::SetContinuousTaskProcess(int32_t pid, bool isContinuousTask)
{
    TAG_LOGD(AAFwkTag::APPMGR, "SetContinuousTaskProcess start.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return ERR_INVALID_DATA;
    }

    if (!data.WriteInt32(pid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "uid write failed.");
        return ERR_INVALID_DATA;
    }

    if (!data.WriteBool(isContinuousTask)) {
        TAG_LOGE(AAFwkTag::APPMGR, "isContinuousTask write failed.");
        return ERR_INVALID_DATA;
    }

    auto ret = SendRequest(AppMgrInterfaceCode::SET_CONTINUOUSTASK_PROCESS,
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "Send request failed with error code %{public}d.", ret);
        return ret;
    }

    return reply.ReadInt32();
}
#endif

int32_t AppMgrProxy::NotifyUnLoadRepairPatch(const std::string &bundleName, const sptr<IQuickFixCallback> &callback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Notify unload patch, Write interface token failed.");
        return ERR_INVALID_DATA;
    }

    if (!data.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Notify unload patch, Write bundle name failed.");
        return ERR_INVALID_DATA;
    }

    if (callback == nullptr || !data.WriteRemoteObject(callback->AsObject())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write callback failed.");
        return ERR_INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option;

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::NOTIFY_UNLOAD_REPAIR_PATCH, data, reply, option);
    return reply.ReadInt32();
}

bool AppMgrProxy::IsSharedBundleRunning(const std::string &bundleName, uint32_t versionCode)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return false;
    }
    if (!data.WriteString(bundleName) || !data.WriteUint32(versionCode)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write bundle name or version code failed.");
        return false;
    }

    MessageParcel reply;
    MessageOption option;
    auto ret = SendRequest(AppMgrInterfaceCode::IS_SHARED_BUNDLE_RUNNING,
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
        return false;
    }

    return reply.ReadBool();
}

int32_t AppMgrProxy::StartNativeProcessForDebugger(const AAFwk::Want &want)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteParcelable(&want)) {
        TAG_LOGE(AAFwkTag::APPMGR, "want write failed.");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::START_NATIVE_PROCESS_FOR_DEBUGGER, data, reply, option);
    return reply.ReadInt32();
}

int32_t AppMgrProxy::GetBundleNameByPid(const int pid, std::string &bundleName, int32_t &uid)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return ERR_INVALID_DATA;
    }

    if (!data.WriteInt32(pid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "pid write failed.");
        return ERR_INVALID_DATA;
    }

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::GET_BUNDLE_NAME_BY_PID, data, reply, option);
    bundleName = reply.ReadString();
    uid = reply.ReadInt32();
    return ERR_NONE;
}

int32_t AppMgrProxy::GetRunningProcessInfoByPid(const pid_t pid, OHOS::AppExecFwk::RunningProcessInfo &info)
{
    TAG_LOGD(AAFwkTag::APPMGR, "start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return ERR_INVALID_DATA;
    }

    if (!data.WriteInt32(static_cast<int32_t>(pid))) {
        TAG_LOGE(AAFwkTag::APPMGR, "parcel WriteInt32 failed.");
        return ERR_INVALID_DATA;
    }

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::GET_RUNNING_PROCESS_INFO_BY_PID, data, reply, option);

    std::unique_ptr<AppExecFwk::RunningProcessInfo> processInfo(reply.ReadParcelable<AppExecFwk::RunningProcessInfo>());
    if (processInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "recv process info failded");
        return ERR_INVALID_DATA;
    }
    info = *processInfo;
    return reply.ReadInt32();
}

int32_t AppMgrProxy::NotifyAppFault(const FaultData &faultData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;

    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteParcelable(&faultData)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write FaultData error.");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::NOTIFY_APP_FAULT, data, reply, option);
    return reply.ReadInt32();
}

int32_t AppMgrProxy::NotifyAppFaultBySA(const AppFaultDataBySA &faultData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;

    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteParcelable(&faultData)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write FaultDataBySA error.");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::NOTIFY_APP_FAULT_BY_SA, data, reply, option);
    return reply.ReadInt32();
}

bool AppMgrProxy::SetAppFreezeFilter(int32_t pid)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return false;
    }
    if (!data.WriteInt32(pid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write pid failed.");
        return false;
    }
    auto ret = SendRequest(AppMgrInterfaceCode::SET_APPFREEZE_FILTER,
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request failed with error code %{public}d.", ret);
        return false;
    }
    return reply.ReadBool();
}

int32_t AppMgrProxy::GetProcessMemoryByPid(const int32_t pid, int32_t &memorySize)
{
    TAG_LOGD(AAFwkTag::APPMGR, "GetProcessMemoryByPid start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteInt32(pid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write pid failed.");
        return ERR_INVALID_DATA;
    }

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::GET_PROCESS_MEMORY_BY_PID, data, reply, option);
    memorySize = reply.ReadInt32();
    auto result = reply.ReadInt32();
    return result;
}

int32_t AppMgrProxy::GetRunningProcessInformation(
    const std::string &bundleName, int32_t userId, std::vector<RunningProcessInfo> &info)
{
    TAG_LOGD(AAFwkTag::APPMGR, "GetRunningProcessInformation start");
    MessageParcel data;
    MessageParcel reply;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write bundleName failed.");
        return ERR_INVALID_DATA;
    }

    if (!data.WriteInt32(userId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write userId failed.");
        return ERR_INVALID_DATA;
    }

    MessageOption option(MessageOption::TF_SYNC);
    auto ret = SendRequest(AppMgrInterfaceCode::GET_PIDS_BY_BUNDLENAME,
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request failed with error code %{public}d.", ret);
        return ret;
    }

    auto error = GetParcelableInfos<RunningProcessInfo>(reply, info);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetParcelableInfos fail, error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int32_t AppMgrProxy::ChangeAppGcState(pid_t pid, int32_t state)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;
    MessageParcel reply;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return ERR_FLATTEN_OBJECT;
    }
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInt32(pid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Pid write failed.");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(state)) {
        TAG_LOGE(AAFwkTag::APPMGR, "State write failed.");
        return ERR_FLATTEN_OBJECT;
    }

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::CHANGE_APP_GC_STATE, data, reply, option);
    return NO_ERROR;
}

int32_t AppMgrProxy::NotifyPageShow(const sptr<IRemoteObject> &token, const PageStateData &pageStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "call");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteRemoteObject(token)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write token");
        return ERR_INVALID_DATA;
    }
    if (!data.WriteParcelable(&pageStateData)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write PageStateData error.");
        return ERR_FLATTEN_OBJECT;
    }

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::NOTIFY_PAGE_SHOW, data, reply, option);
    return NO_ERROR;
}

int32_t AppMgrProxy::NotifyPageHide(const sptr<IRemoteObject> &token, const PageStateData &pageStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteRemoteObject(token)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write token");
        return ERR_INVALID_DATA;
    }
    if (!data.WriteParcelable(&pageStateData)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write PageStateData error.");
        return ERR_FLATTEN_OBJECT;
    }

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::NOTIFY_PAGE_HIDE, data, reply, option);
    return NO_ERROR;
}

int32_t AppMgrProxy::SendRequest(AppMgrInterfaceCode code, MessageParcel &data, MessageParcel &reply,
    MessageOption& option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Remote() is NULL");
        return ERR_NULL_OBJECT;
    }

    return remote->SendRequest(static_cast<uint32_t>(code), data, reply, option);
}

int32_t AppMgrProxy::RegisterAppRunningStatusListener(const sptr<IRemoteObject> &listener)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return ERR_FLATTEN_OBJECT;
    }
    if (listener == nullptr || !data.WriteRemoteObject(listener)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write listener failed.");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::REGISTER_APP_RUNNING_STATUS_LISTENER, data, reply, option);
    return reply.ReadInt32();
}

int32_t AppMgrProxy::UnregisterAppRunningStatusListener(const sptr<IRemoteObject> &listener)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return ERR_FLATTEN_OBJECT;
    }
    if (listener == nullptr || !data.WriteRemoteObject(listener)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write listener failed.");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::UNREGISTER_APP_RUNNING_STATUS_LISTENER, data, reply, option);
    return reply.ReadInt32();
}

int32_t AppMgrProxy::RegisterAppForegroundStateObserver(const sptr<IAppForegroundStateObserver> &observer)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (observer == nullptr || !data.WriteRemoteObject(observer->AsObject())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Observer is null or Write Remote failed.");
        return ERR_FLATTEN_OBJECT;
    }
    MessageParcel reply;
    MessageOption option;

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::REGISTER_APP_FOREGROUND_STATE_OBSERVER, data, reply, option);
    return reply.ReadInt32();
}

int32_t AppMgrProxy::UnregisterAppForegroundStateObserver(const sptr<IAppForegroundStateObserver> &observer)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (observer == nullptr || !data.WriteRemoteObject(observer->AsObject())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Observer is null or Write Remote failed.");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::UNREGISTER_APP_FOREGROUND_STATE_OBSERVER, data, reply, option);
    return reply.ReadInt32();
}

int32_t AppMgrProxy::IsApplicationRunning(const std::string &bundleName, bool &isRunning)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    isRunning = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return ERR_INVALID_DATA;
    }
    PARCEL_UTIL_WRITE_RET_INT(data, String, bundleName);

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::IS_APPLICATION_RUNNING, data, reply, option);
    isRunning = reply.ReadBool();
    return reply.ReadInt32();
}

int32_t AppMgrProxy::IsAppRunning(const std::string &bundleName, int32_t appCloneIndex, bool &isRunning)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return ERR_INVALID_DATA;
    }
    PARCEL_UTIL_WRITE_RET_INT(data, String, bundleName);
    PARCEL_UTIL_WRITE_RET_INT(data, Int32, appCloneIndex);

    MessageParcel reply;
    MessageOption option;

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::IS_APP_RUNNING, data, reply, option);
    isRunning = reply.ReadBool();
    return reply.ReadInt32();
}

int32_t AppMgrProxy::StartChildProcess(pid_t &childPid, const ChildProcessRequest &request)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (request.srcEntry.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Invalid params, srcEntry:%{private}s", request.srcEntry.c_str());
        return ERR_INVALID_VALUE;
    }
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "WriteInterfaceToken failed");
        return IPC_PROXY_ERR;
    }
    if (!data.WriteParcelable(&request)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write param request failed.");
        return IPC_PROXY_ERR;
    }

    MessageParcel reply;
    MessageOption option;

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::START_CHILD_PROCESS, data, reply, option);
    auto result = reply.ReadInt32();
    if (result == ERR_OK) {
        childPid = reply.ReadInt32();
    }
    return result;
}

int32_t AppMgrProxy::GetChildProcessInfoForSelf(ChildProcessInfo &info)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "WriteInterfaceToken failed");
        return ERR_FLATTEN_OBJECT;
    }

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::GET_CHILD_PROCCESS_INFO_FOR_SELF, data, reply, option);
    auto result = reply.ReadInt32();
    if (result == ERR_OK) {
        std::unique_ptr<ChildProcessInfo> infoReply(reply.ReadParcelable<ChildProcessInfo>());
        info = *infoReply;
    }
    return result;
}

void AppMgrProxy::AttachChildProcess(const sptr<IRemoteObject> &childScheduler)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!childScheduler) {
        TAG_LOGE(AAFwkTag::APPMGR, "childScheduler is null");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "WriteInterfaceToken failed");
        return;
    }
    PARCEL_UTIL_WRITE_NORET(data, RemoteObject, childScheduler.GetRefPtr());

    PARCEL_UTIL_SENDREQ_NORET(AppMgrInterfaceCode::ATTACH_CHILD_PROCESS, data, reply, option);
}

void AppMgrProxy::ExitChildProcessSafely()
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "WriteInterfaceToken failed");
        return;
    }

    PARCEL_UTIL_SENDREQ_NORET(AppMgrInterfaceCode::EXIT_CHILD_PROCESS_SAFELY, data, reply, option);
}

bool AppMgrProxy::IsFinalAppProcess()
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return ERR_INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option;
    auto ret = SendRequest(AppMgrInterfaceCode::IS_FINAL_APP_PROCESS,
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request is failed, error code: %{public}d", ret);
        return false;
    }

    return reply.ReadBool();
}

int32_t AppMgrProxy::RegisterRenderStateObserver(const sptr<IRenderStateObserver> &observer)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return ERR_INVALID_DATA;
    }
    if (observer == nullptr || !data.WriteRemoteObject(observer->AsObject())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Observer is null or Write Remote failed.");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::REGISTER_RENDER_STATUS_OBSERVER, data, reply, option);
    return reply.ReadInt32();
}

int32_t AppMgrProxy::UnregisterRenderStateObserver(const sptr<IRenderStateObserver> &observer)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return ERR_INVALID_DATA;
    }
    if (observer == nullptr || !data.WriteRemoteObject(observer->AsObject())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Observer is null or Write Remote failed.");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::UNREGISTER_RENDER_STATUS_OBSERVER, data, reply, option);
    return reply.ReadInt32();
}

int32_t AppMgrProxy::UpdateRenderState(pid_t renderPid, int32_t state)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return ERR_INVALID_DATA;
    }
    PARCEL_UTIL_WRITE_RET_INT(data, Int32, renderPid);
    PARCEL_UTIL_WRITE_RET_INT(data, Int32, state);

    MessageParcel reply;
    MessageOption option;

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::UPDATE_RENDER_STATUS, data, reply, option);
    return reply.ReadInt32();
}

int32_t AppMgrProxy::SignRestartAppFlag(const std::string &bundleName)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return IPC_PROXY_ERR;
    }
    PARCEL_UTIL_WRITE_RET_INT(data, String, bundleName);

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::SIGN_RESTART_APP_FLAG, data, reply, option);
    return reply.ReadInt32();
}

int32_t AppMgrProxy::GetAppRunningUniqueIdByPid(pid_t pid, std::string &appRunningUniqueId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return IPC_PROXY_ERR;
    }
    PARCEL_UTIL_WRITE_RET_INT(data, Int32, pid);

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::GET_APP_RUNNING_UNIQUE_ID_BY_PID, data, reply, option);
    auto result = reply.ReadInt32();
    if (result == ERR_OK) {
        appRunningUniqueId = reply.ReadString();
        TAG_LOGD(AAFwkTag::APPMGR, "appRunningUniqueId = %{public}s", appRunningUniqueId.c_str());
    }
    return result;
}

int32_t AppMgrProxy::GetAllUIExtensionRootHostPid(pid_t pid, std::vector<pid_t> &hostPids)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write remote object failed.");
        return ERR_INVALID_DATA;
    }

    PARCEL_UTIL_WRITE_RET_INT(data, Int32, pid);

    MessageParcel reply;
    MessageOption option;

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::GET_ALL_UI_EXTENSION_ROOT_HOST_PID, data, reply, option);

    int32_t size = reply.ReadInt32();
    if (size > CYCLE_LIMIT) {
        TAG_LOGE(AAFwkTag::APPMGR, "Vector is too large.");
        return ERR_INVALID_VALUE;
    }

    for (int32_t i = 0; i < size; i++) {
        pid_t temp = reply.ReadInt32();
        hostPids.emplace_back(temp);
    }

    return reply.ReadInt32();
}

int32_t AppMgrProxy::GetAllUIExtensionProviderPid(pid_t hostPid, std::vector<pid_t> &providerPids)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write remote object failed.");
        return ERR_INVALID_DATA;
    }
    PARCEL_UTIL_WRITE_RET_INT(data, Int32, hostPid);

    MessageParcel reply;
    MessageOption option;

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::GET_ALL_UI_EXTENSION_PROVIDER_PID, data, reply, option);

    int32_t size = reply.ReadInt32();
    if (size > CYCLE_LIMIT) {
        TAG_LOGE(AAFwkTag::APPMGR, "Vector is too large.");
        return ERR_INVALID_VALUE;
    }

    for (int32_t i = 0; i < size; i++) {
        pid_t temp = reply.ReadInt32();
        providerPids.emplace_back(temp);
    }

    return reply.ReadInt32();
}

int32_t AppMgrProxy::NotifyMemorySizeStateChanged(bool isMemorySizeSufficent)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return ERR_INVALID_DATA;
    }
    PARCEL_UTIL_WRITE_RET_INT(data, Bool, isMemorySizeSufficent);

    MessageParcel reply;
    MessageOption option;

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::NOTIFY_MEMORY_SIZE_STATE_CHANGED, data, reply, option);
    return reply.ReadInt32();
}

int32_t AppMgrProxy::SetSupportedProcessCacheSelf(bool isSupport)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return ERR_INVALID_DATA;
    }
    PARCEL_UTIL_WRITE_RET_INT(data, Bool, isSupport);

    MessageParcel reply;
    MessageOption option;

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::SET_SUPPORTED_PROCESS_CACHE_SELF, data, reply, option);
    return reply.ReadInt32();
}

void AppMgrProxy::SetAppAssertionPauseState(bool flag)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return;
    }
    PARCEL_UTIL_WRITE_NORET(data, Bool, flag);

    PARCEL_UTIL_SENDREQ_NORET(AppMgrInterfaceCode::SET_APP_ASSERT_PAUSE_STATE_SELF, data, reply, option);
}

int32_t AppMgrProxy::StartNativeChildProcess(const std::string &libName, int32_t childProcessCount,
    const sptr<IRemoteObject> &callback)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (libName.empty() || !callback) {
        TAG_LOGE(AAFwkTag::APPMGR, "Invalid params, libName:%{private}s", libName.c_str());
        return ERR_INVALID_VALUE;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return IPC_PROXY_ERR;
    }
    PARCEL_UTIL_WRITE_RET_INT(data, String, libName);
    PARCEL_UTIL_WRITE_RET_INT(data, Int32, childProcessCount);
    PARCEL_UTIL_WRITE_RET_INT(data, RemoteObject, callback);

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::START_NATIVE_CHILD_PROCESS, data, reply, option);
    return reply.ReadInt32();
}

int32_t AppMgrProxy::CheckCallingIsUserTestMode(const pid_t pid, bool &isUserTest)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_UTIL_WRITE_RET_INT(data, Int32, pid);
    int32_t ret = SendRequest(AppMgrInterfaceCode::CHECK_CALLING_IS_USER_TEST_MODE, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
        isUserTest = false;
        return ret;
    }
    isUserTest = reply.ReadBool();
    return reply.ReadInt32();
}

int32_t AppMgrProxy::NotifyProcessDependedOnWeb()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return IPC_PROXY_ERR;
    }

    PARCEL_UTIL_SENDREQ_RET_INT(AppMgrInterfaceCode::NOTIFY_PROCESS_DEPENDED_ON_WEB, data, reply, option);
    return reply.ReadInt32();
}

void AppMgrProxy::KillProcessDependedOnWeb()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return;
    }

    PARCEL_UTIL_SENDREQ_NORET(AppMgrInterfaceCode::KILL_PROCESS_DEPENDED_ON_WEB, data, reply, option);
}

void AppMgrProxy::RestartResidentProcessDependedOnWeb()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return;
    }

    PARCEL_UTIL_SENDREQ_NORET(AppMgrInterfaceCode::RESTART_RESIDENT_PROCESS_DEPENDED_ON_WEB, data, reply, option);
}
}  // namespace AppExecFwk
}  // namespace OHOS
