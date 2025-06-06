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

#ifndef OHOS_ABILITY_MANAGER_WINDOW_VISIBILITY_CHANGE_LISTENER_H
#define OHOS_ABILITY_MANAGER_WINDOW_VISIBILITY_CHANGE_LISTENER_H

#include "task_handler_wrap.h"
#include "window_manager.h"

namespace OHOS {
namespace AAFwk {
class AbilityManagerService;

class WindowVisibilityChangedListener : public OHOS::Rosen::IVisibilityChangedListener {
public:
    WindowVisibilityChangedListener(
        const std::weak_ptr<AbilityManagerService> &owner, const std::shared_ptr<AAFwk::TaskHandlerWrap> &handler);
    virtual ~WindowVisibilityChangedListener() = default;

    void OnWindowVisibilityChanged(
        const std::vector<sptr<OHOS::Rosen::WindowVisibilityInfo>> &windowVisibilityInfos) override;

private:
    std::weak_ptr<AbilityManagerService> owner_;
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_MANAGER_WINDOW_VISIBILITY_CHANGE_LISTENER_H
