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

#include "app_state_callback_host.h"
#include "appexecfwk_errors.h"
#include "hilog_tag_wrapper.h"
#include "ipc_types.h"
#include "iremote_object.h"
#include "app_state_callback_proxy.h"

namespace OHOS {
namespace AppExecFwk {
AppStateCallbackHost::AppStateCallbackHost()
{}

AppStateCallbackHost::~AppStateCallbackHost()
{}

int AppStateCallbackHost::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    return 0;
}

void AppStateCallbackHost::OnAbilityRequestDone(const sptr<IRemoteObject>&, const AbilityState)
{
    TAG_LOGD(AAFwkTag::TEST, "OnAbilityRequestDone called");
}

void AppStateCallbackHost::OnAppStateChanged(const AppProcessData&)
{
    TAG_LOGD(AAFwkTag::TEST, "OnAppStateChanged called");
}

void AppStateCallbackHost::NotifyStartResidentProcess(std::vector<AppExecFwk::BundleInfo> &bundleInfos)
{
    TAG_LOGD(AAFwkTag::TEST, "NotifyStartResidentProcess called");
}

void AppStateCallbackHost::NotifyStartKeepAliveProcess(std::vector<AppExecFwk::BundleInfo> &bundleInfos)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
}

void AppStateCallbackHost::NotifyAppPreCache(int32_t pid, int32_t userId)
{
    TAG_LOGD(AAFwkTag::TEST, "NotifyAppPreCache called");
}

int32_t AppStateCallbackHost::HandleOnAppStateChanged(MessageParcel& data, MessageParcel& reply)
{
    return NO_ERROR;
}

int32_t AppStateCallbackHost::HandleOnAbilityRequestDone(MessageParcel& data, MessageParcel& reply)
{
    return NO_ERROR;
}

int32_t AppStateCallbackHost::HandleNotifyStartResidentProcess(MessageParcel &data, MessageParcel &reply)
{
    return NO_ERROR;
}

int32_t AppStateCallbackHost::HandleNotifyStartKeepAliveProcess(MessageParcel &data, MessageParcel &reply)
{
    return NO_ERROR;
}

int32_t AppStateCallbackHost::HandleNotifyAppPreCache(MessageParcel &data, MessageParcel &reply)
{
    return NO_ERROR;
}
}  // namespace AppExecFwk
}  // namespace OHOS
