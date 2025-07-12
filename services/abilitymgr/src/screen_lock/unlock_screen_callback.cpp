/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#ifdef SUPPORT_GRAPHICS
#ifdef ABILITY_RUNTIME_SCREENLOCK_ENABLE
#include "unlock_screen_callback.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
UnlockScreenCallback::~UnlockScreenCallback() {}

UnlockScreenCallback::UnlockScreenCallback(std::shared_ptr<std::promise<bool>> promise) : screenLockResult_(promise) {}

void UnlockScreenCallback::OnCallBack(const int32_t screenLockResult)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "result: %{public}d", screenLockResult);
    if (screenLockResult_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null");
        return;
    }
    screenLockResult_->set_value(screenLockResult == 0);
}
} // namespace AbilityRuntime
} // namespace OHOS
#endif // ABILITY_RUNTIME_SCREENLOCK_ENABLE
#endif // SUPPORT_GRAPHICS

