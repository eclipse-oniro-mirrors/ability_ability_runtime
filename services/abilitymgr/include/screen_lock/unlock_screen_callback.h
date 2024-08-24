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

#ifndef OHOS_ABILITY_RUNTIME_UNLOCK_SCREEN_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_UNLOCK_SCREEN_CALLBACK_H

#ifdef SUPPORT_GRAPHICS
#include "screenlock_manager.h"
#include "screenlock_callback_stub.h"

namespace OHOS {
namespace AbilityRuntime {
class UnlockScreenCallback : public ScreenLock::ScreenLockCallbackStub {
public:
    explicit UnlockScreenCallback();
    ~UnlockScreenCallback() override;
    void OnCallBack(const int32_t screenLockResult) override;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // SUPPORT_GRAPHICS
#endif // OHOS_ABILITY_RUNTIME_UNLOCK_SCREEN_CALLBACK_H
