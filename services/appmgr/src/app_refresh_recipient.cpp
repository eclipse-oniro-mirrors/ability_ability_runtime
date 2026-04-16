/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "app_refresh_recipient.h"

#include "app_mgr_service_inner.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string TASK_ON_REMOTE_REFRESHED = "OnRemoteRefreshedTask";
} // namespace

void AppRefreshRecipient::OnRemoteRefreshed(const wptr<IRemoteObject> &remote)
{
    TAG_LOGI(AAFwkTag::APPMGR, "OnRemoteRefreshed");
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null remote");
        return;
    }

    auto handler = handler_.lock();
    if (!handler) {
        TAG_LOGE(AAFwkTag::APPMGR, "null handler");
        return;
    }

    auto onRemoteRefreshFunc = [appMgrServiceInner = appMgrServiceInner_, remote]() {
        auto serviceInner = appMgrServiceInner.lock();
        if (!serviceInner) {
            TAG_LOGE(AAFwkTag::APPMGR, "null serviceInner");
            return;
        }
        serviceInner->OnRemoteDied(remote);
        TAG_LOGD(AAFwkTag::APPMGR, "OnRemoteRefreshedTask end");
    };
    handler->SubmitTask(onRemoteRefreshFunc, AAFwk::TaskAttribute{
        .taskName_ = TASK_ON_REMOTE_REFRESHED,
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
    });
}

void AppRefreshRecipient::SetTaskHandler(const std::shared_ptr<AAFwk::TaskHandlerWrap> &handler)
{
    handler_ = handler;
}

void AppRefreshRecipient::SetAppMgrServiceInner(const std::shared_ptr<AppMgrServiceInner> &serviceInner)
{
    appMgrServiceInner_ = serviceInner;
}
}  // namespace AppExecFwk
}  // namespace OHOS