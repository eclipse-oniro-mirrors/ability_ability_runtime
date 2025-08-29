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
#include "user_record_manager.h"

#include <mutex>
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t U0_USER_ID = 0;
}
UserRecordManager::~UserRecordManager() {}

UserRecordManager::UserRecordManager() {}

UserRecordManager &UserRecordManager::GetInstance()
{
    static UserRecordManager instance;
    return instance;
}

bool UserRecordManager::IsLogoutUser(int32_t userId)
{
    if (userId == U0_USER_ID) {
        return false;
    }
    std::lock_guard lock(logoutUserIdLock_);
    return logoutUserIdSet_.find(userId) != logoutUserIdSet_.end();
}

void UserRecordManager::SetEnableStartProcessFlagByUserId(int32_t userId, bool enableStartProcess)
{
    TAG_LOGI(AAFwkTag::APPMGR, "userId:%{public}d, enableStartProcess:%{public}d", userId, enableStartProcess);
    if (enableStartProcess) {
        std::lock_guard lock(logoutUserIdLock_);
        logoutUserIdSet_.erase(userId);
    } else {
        std::lock_guard lock(logoutUserIdLock_);
        logoutUserIdSet_.insert(userId);
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS