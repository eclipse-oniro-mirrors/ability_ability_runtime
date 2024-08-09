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

#include "child_scheduler_proxy.h"

#include "hilog_tag_wrapper.h"
#include "ipc_types.h"

namespace OHOS {
namespace AppExecFwk {
ChildSchedulerProxy::ChildSchedulerProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IChildScheduler>(impl)
{}

bool ChildSchedulerProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(ChildSchedulerProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::APPMGR, "write interface token failed");
        return false;
    }
    return true;
}

bool ChildSchedulerProxy::ScheduleLoadJs()
{
    TAG_LOGD(AAFwkTag::APPMGR, "ScheduleLoadJs start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return false;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Remote() is NULL");
        return false;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IChildScheduler::Message::SCHEDULE_LOAD_JS), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
        return false;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "ScheduleLoadJs end");
    return true;
}

bool ChildSchedulerProxy::ScheduleExitProcessSafely()
{
    TAG_LOGD(AAFwkTag::APPMGR, "ScheduleExitProcessSafely start.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return false;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Remote() is NULL.");
        return false;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IChildScheduler::Message::SCHEDULE_EXIT_PROCESS_SAFELY), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d.", ret);
        return false;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "ScheduleExitProcessSafely end.");
    return true;
}

bool ChildSchedulerProxy::ScheduleRunNativeProc(const sptr<IRemoteObject> &mainProcessCb)
{
    TAG_LOGD(AAFwkTag::APPMGR, "ScheduleRunNativeProc start.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return false;
    }

    if (!data.WriteRemoteObject(mainProcessCb)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write main process callback ipc object failed.");
        return false;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Remote() is null.");
        return false;
    }

    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IChildScheduler::Message::SCHEDULE_RUN_NATIVE_PROC), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d.", ret);
        return false;
    }

    TAG_LOGD(AAFwkTag::APPMGR, "ScheduleRunNativeProc end.");
    return true;
}

}  // namespace AppExecFwk
}  // namespace OHOS
