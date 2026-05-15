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

#ifndef OHOS_ABILITY_RUNTIME_USER_CONTROLLER_H
#define OHOS_ABILITY_RUNTIME_USER_CONTROLLER_H

#include <cstdint>
#include <unordered_map>
#include <unordered_set>

namespace OHOS {
namespace AAFwk {
class IUserCallback;
}
namespace AbilityRuntime {
class UserController final {
public:
    enum class UserLockStatus {
        USER_LOCKED = 0,
        USER_UNLOCKED = 1,
        USER_LOCK_STATUS_BUTT
    };

public:
    UserController() = default;
    ~UserController() = default;
    static UserController& GetInstance();

    UserLockStatus GetUserLockStatus(int32_t userId);

    // Mock function to set lock status for testing (applies to userId 0 and 100)
    void SetMockUserLockStatus(UserLockStatus status);

private:
    UserLockStatus mockUserLockStatus_ = UserLockStatus::USER_LOCK_STATUS_BUTT;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_USER_CONTROLLER_H
