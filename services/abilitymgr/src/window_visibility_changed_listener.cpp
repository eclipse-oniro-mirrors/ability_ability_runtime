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

#include "window_visibility_changed_listener.h"

#include "ability_manager_service.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
using namespace OHOS::Rosen;
WindowVisibilityChangedListener::WindowVisibilityChangedListener(
    const std::weak_ptr<AbilityManagerService> &owner, const std::shared_ptr<AAFwk::TaskHandlerWrap> &handler)
    : owner_(owner), taskHandler_(handler)
{}

void WindowVisibilityChangedListener::OnWindowVisibilityChanged(
    const std::vector<sptr<WindowVisibilityInfo>> &windowVisibilityInfos)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    if (windowVisibilityInfos.empty()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "windowVisibilityInfo is empty");
        return;
    }

    if (taskHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null taskHandler");
        return;
    }

    auto task = [inner = owner_, windowVisibilityInfos] {
        auto owner = inner.lock();
        if (owner == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "get fail");
            return;
        }
        owner->HandleWindowVisibilityChanged(windowVisibilityInfos);
    };
    taskHandler_->SubmitTask(task);
}
} // namespace AAFwk
} // namespace OHOS
