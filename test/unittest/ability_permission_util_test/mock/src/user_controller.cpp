/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "user_controller.h"

namespace OHOS {
namespace AbilityRuntime {
UserController& UserController::GetInstance()
{
    static UserController instance;
    return instance;
}

UserController::UserLockStatus UserController::GetUserLockStatus(int32_t userId)
{
    return mockUserLockStatus_;
}

void UserController::SetMockUserLockStatus(UserLockStatus status)
{
    mockUserLockStatus_ = status;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
