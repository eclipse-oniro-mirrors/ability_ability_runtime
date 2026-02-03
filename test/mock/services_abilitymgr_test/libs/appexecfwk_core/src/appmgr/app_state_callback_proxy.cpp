/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "app_state_callback_proxy.h"
#include "bundle_info.h"

#include "ipc_types.h"


namespace OHOS {
namespace AppExecFwk {
AppStateCallbackProxy::AppStateCallbackProxy(const sptr<IRemoteObject>& impl) : IRemoteProxy<IAppStateCallback>(impl)
{}

bool AppStateCallbackProxy::WriteInterfaceToken(MessageParcel& data)
{
    return true;
}

void AppStateCallbackProxy::OnAbilityRequestDone(const sptr<IRemoteObject>& token, const AbilityState state)
{}

void AppStateCallbackProxy::OnAppStateChanged(const AppProcessData& appProcessData)
{}

void AppStateCallbackProxy::NotifyAppPreCache(int32_t pid, int32_t userId)
{}

void AppStateCallbackProxy::NotifyStartResidentProcess(std::vector<AppExecFwk::BundleInfo> &bundleInfos)
{}

void AppStateCallbackProxy::NotifyStartKeepAliveProcess(std::vector<AppExecFwk::BundleInfo> &bundleInfos,
    pid_t diedPid)
{}

void AppStateCallbackProxy::RecordAppExitSignalReason(int32_t pid, int32_t uid, int32_t signal,
    std::string &bundleName)
{}
}  // namespace AppExecFwk
}  // namespace OHOS
