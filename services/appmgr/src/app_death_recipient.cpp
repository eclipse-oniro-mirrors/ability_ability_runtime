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

#include "app_death_recipient.h"
#include "app_mgr_service_inner.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string TASK_ON_REMOTE_DIED = "OnRemoteDiedTask";
} // namespace

void AppDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null remote");
        return;
    }

    auto handler = handler_.lock();
    if (!handler) {
        TAG_LOGE(AAFwkTag::APPMGR, "null handler");
        return;
    }

    auto onRemoteDiedFunc = [appMgrServiceInner = appMgrServiceInner_, remote,
        isRenderProcess = isRenderProcess_,
        isChildProcess = isChildProcess_,
        isImageProcess = isImageProcess_]() {
        auto serviceInner = appMgrServiceInner.lock();
        if (!serviceInner) {
            TAG_LOGE(AAFwkTag::APPMGR, "null serviceInner");
            return;
        }
        serviceInner->OnRemoteDied(remote, isRenderProcess, isChildProcess, isImageProcess);
        TAG_LOGD(AAFwkTag::APPMGR, "OnRemoteDiedTask end");
    };
    handler->SubmitTask(onRemoteDiedFunc, AAFwk::TaskAttribute{
        .taskName_ = TASK_ON_REMOTE_DIED,
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
    });
}

void AppDeathRecipient::SetTaskHandler(const std::shared_ptr<AAFwk::TaskHandlerWrap> &handler)
{
    handler_ = handler;
}

void AppDeathRecipient::SetAppMgrServiceInner(const std::shared_ptr<AppMgrServiceInner> &serviceInner)
{
    appMgrServiceInner_ = serviceInner;
}

void AppDeathRecipient::SetIsRenderProcess(bool isRenderProcess)
{
    isRenderProcess_ = isRenderProcess;
}

void AppDeathRecipient::SetIsChildProcess(bool isChildProcess)
{
    isChildProcess_ = isChildProcess;
}

void AppDeathRecipient::SetIsImageProcess(bool isImageProcess)
{
    isImageProcess_ = isImageProcess;
}

AppStateCallbackDeathRecipient::AppStateCallbackDeathRecipient(std::weak_ptr<AppMgrServiceInner> appMgrServiceInner)
    : appMgrServiceInner_(appMgrServiceInner) {}

void AppStateCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    auto appMgrInner = appMgrServiceInner_.lock();
    if (appMgrInner) {
        appMgrInner->RemoveDeadAppStateCallback(remote);
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
