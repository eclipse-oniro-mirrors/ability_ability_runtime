/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "os_account_manager_wrapper.h"

#include "mock_my_status.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr const int32_t USER_ID = 100;
}
ErrCode OsAccountManagerWrapper::QueryActiveOsAccountIds(std::vector<int32_t>& ids)
{
    return 0;
}

ErrCode OsAccountManagerWrapper::GetOsAccountLocalIdFromUid(const int32_t uid, int32_t& id)
{
    id = USER_ID;
    return AAFwk::MyStatus::GetInstance().getOsAccountRet_;
}

ErrCode OsAccountManagerWrapper::GetOsAccountLocalIdFromProcess(int& id)
{
    return 0;
}

ErrCode OsAccountManagerWrapper::IsOsAccountExists(const int id, bool& isOsAccountExists)
{
    return 0;
}

ErrCode OsAccountManagerWrapper::CreateOsAccount(const std::string& name, int32_t& osAccountUserId)
{
    return 0;
}

ErrCode OsAccountManagerWrapper::RemoveOsAccount(const int id)
{
    return 0;
}

int32_t OsAccountManagerWrapper::GetCurrentActiveAccountId()
{
    return 0;
}
} // namespace AppExecFwk
} // namespace OHOS
