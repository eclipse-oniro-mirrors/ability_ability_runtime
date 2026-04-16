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

#include "window_focus_changed_listener.h"

#include "app_mgr_service_inner.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
#ifdef SUPPORT_SCREEN
using namespace OHOS::Rosen;

WindowFocusChangedListener::WindowFocusChangedListener(const std::shared_ptr<AppMgrServiceInner> &owner,
    const std::shared_ptr<AAFwk::TaskHandlerWrap>& handler) : owner_(owner), taskHandler_(handler) {}

WindowFocusChangedListener::~WindowFocusChangedListener() {}

void WindowFocusChangedListener::OnFocused(const sptr<FocusChangeInfo> &focusChangeInfo)
{
    if (!focusChangeInfo) {
        TAG_LOGW(AAFwkTag::APPMGR, "null focusChangeInfo");
        return;
    }

    if (taskHandler_) {
        auto task = [inner = owner_, focusChangeInfo] {
            auto owner = inner.lock();
            if (!owner) {
                TAG_LOGW(AAFwkTag::APPMGR, "get app mgr service inner fail");
                return;
            }
            owner->HandleFocused(focusChangeInfo);
        };
        taskHandler_->SubmitTask(task, "WindowFocusChangedListener::OnFocused");
    }
}

void WindowFocusChangedListener::OnUnfocused(const sptr<FocusChangeInfo> &focusChangeInfo)
{
    if (!focusChangeInfo) {
        TAG_LOGW(AAFwkTag::APPMGR, "null focusChangeInfo");
        return;
    }

    if (taskHandler_) {
        auto task = [inner = owner_, focusChangeInfo] {
            auto owner = inner.lock();
            if (!owner) {
                TAG_LOGW(AAFwkTag::APPMGR, "get fail");
                return;
            }
            owner->HandleUnfocused(focusChangeInfo);
        };
        AAFwk::TaskAttribute attr = {
            .taskName_ = "WindowFocusChangedListener::OnUnfocused",
            .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
        };
        taskHandler_->SubmitTask(task, attr);
    }
}
#endif // SUPPORT_SCREEN
}  // namespace AppExecFwk
}  // namespace OHOS
