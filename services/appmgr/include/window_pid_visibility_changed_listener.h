/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_APP_MANAGER_WINDOW_PID_VISIBILITY_CHANGE_LISTENER_H
#define OHOS_APP_MANAGER_WINDOW_PID_VISIBILITY_CHANGE_LISTENER_H

#include "task_handler_wrap.h"
#ifdef SUPPORT_SCREEN
#include "window_manager.h"
#endif // SUPPORT_SCREEN
namespace OHOS {
namespace AppExecFwk {
class AppMgrServiceInner;
#ifdef SUPPORT_SCREEN
class WindowPidVisibilityChangedListener : public OHOS::Rosen::IWindowPidVisibilityChangedListener {
public:
    WindowPidVisibilityChangedListener(
        const std::weak_ptr<AppMgrServiceInner> &appInner, const std::shared_ptr<AAFwk::TaskHandlerWrap> &handler);
    virtual ~WindowPidVisibilityChangedListener() {}

    void NotifyWindowPidVisibilityChanged(
        const sptr<OHOS::Rosen::WindowPidVisibilityInfo>& windowPidVisibilityInfo) override;

private:
    std::weak_ptr<AppMgrServiceInner> appServiceInner_;
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_;
};
#endif // SUPPORT_SCREEN
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_APP_MANAGER_WINDOW_PID_VISIBILITY_CHANGE_LISTENER_H