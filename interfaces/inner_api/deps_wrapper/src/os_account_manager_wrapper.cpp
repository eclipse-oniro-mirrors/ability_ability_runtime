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

#include "os_account_manager_wrapper.h"

#include "hilog_tag_wrapper.h"
#ifdef OS_ACCOUNT_PART_ENABLED
#include "os_account_manager.h"
#endif // OS_ACCOUNT_PART_ENABLED

namespace OHOS {
namespace AppExecFwk {
#ifndef OS_ACCOUNT_PART_ENABLED
namespace {
const int32_t DEFAULT_OS_ACCOUNT_ID = 0; // default id when there is no os_account part
const int32_t USER_ID_U100 = 100;
const int32_t UID_TRANSFORM_DIVISOR = 200000;
}
#endif // OS_ACCOUNT_PART_ENABLED

ErrCode OsAccountManagerWrapper::QueryActiveOsAccountIds(std::vector<int32_t>& ids)
{
#ifndef OS_ACCOUNT_PART_ENABLED
    TAG_LOGD(AAFwkTag::DEFAULT, "Without os account subsystem");
    ids.emplace_back(DEFAULT_OS_ACCOUNT_ID);
    return ERR_OK;
#else
    TAG_LOGD(AAFwkTag::DEFAULT, "os account subsystem");
    return AccountSA::OsAccountManager::QueryActiveOsAccountIds(ids);
#endif // OS_ACCOUNT_PART_ENABLED
}

ErrCode OsAccountManagerWrapper::GetOsAccountLocalIdFromUid(const int32_t uid, int32_t &id)
{
#ifndef OS_ACCOUNT_PART_ENABLED
    TAG_LOGD(AAFwkTag::DEFAULT, "Without os account subsystem");
    id = uid / UID_TRANSFORM_DIVISOR;
    return ERR_OK;
#else
    TAG_LOGD(AAFwkTag::DEFAULT, "os account subsystem");
    return AccountSA::OsAccountManager::GetOsAccountLocalIdFromUid(uid, id);
#endif // OS_ACCOUNT_PART_ENABLED
}

ErrCode OsAccountManagerWrapper::GetOsAccountLocalIdFromProcess(int &id)
{
#ifndef OS_ACCOUNT_PART_ENABLED
    TAG_LOGD(AAFwkTag::DEFAULT, "Without os account subsystem");
    id = DEFAULT_OS_ACCOUNT_ID;
    return ERR_OK;
#else
    TAG_LOGD(AAFwkTag::DEFAULT, "os account subsystem");
    return AccountSA::OsAccountManager::GetOsAccountLocalIdFromProcess(id);
#endif // OS_ACCOUNT_PART_ENABLED
}

ErrCode OsAccountManagerWrapper::IsOsAccountExists(const int id, bool &isOsAccountExists)
{
#ifndef OS_ACCOUNT_PART_ENABLED
    TAG_LOGD(AAFwkTag::DEFAULT, "Without os account subsystem");
    isOsAccountExists = (id == DEFAULT_OS_ACCOUNT_ID);
    return ERR_OK;
#else // OS_ACCOUNT_PART_ENABLED
    TAG_LOGD(AAFwkTag::DEFAULT, "os account subsystem");
    return AccountSA::OsAccountManager::IsOsAccountExists(id, isOsAccountExists);
#endif // OS_ACCOUNT_PART_ENABLED
}

ErrCode OsAccountManagerWrapper::CreateOsAccount(const std::string &name, int32_t &osAccountUserId)
{
#ifndef OS_ACCOUNT_PART_ENABLED
    TAG_LOGD(AAFwkTag::DEFAULT, "Without os account subsystem");
    osAccountUserId = USER_ID_U100;
    return ERR_OK;
#else // OS_ACCOUNT_PART_ENABLED
    TAG_LOGD(AAFwkTag::DEFAULT, "os account subsystem");
    AccountSA::OsAccountInfo osAccountInfo;
    ErrCode errCode = AccountSA::OsAccountManager::CreateOsAccount(name,
        AccountSA::OsAccountType::NORMAL, osAccountInfo);
    osAccountUserId = osAccountInfo.GetLocalId();
    return errCode;
#endif // OS_ACCOUNT_PART_ENABLED
}

ErrCode OsAccountManagerWrapper::RemoveOsAccount(const int id)
{
#ifndef OS_ACCOUNT_PART_ENABLED
    TAG_LOGD(AAFwkTag::DEFAULT, "Without os account subsystem");
    return ERR_OK;
#else // OS_ACCOUNT_PART_ENABLED
    TAG_LOGD(AAFwkTag::DEFAULT, "os account subsystem");
    return AccountSA::OsAccountManager::RemoveOsAccount(id);
#endif // OS_ACCOUNT_PART_ENABLED
}

int32_t OsAccountManagerWrapper::GetCurrentActiveAccountId()
{
    std::vector<int32_t> accountIds;
    ErrCode ret = QueryActiveOsAccountIds(accountIds);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::DEFAULT, "Query active id failed. ret: %{public}d", ret);
        return 0;
    }

    if (accountIds.empty()) {
        TAG_LOGE(AAFwkTag::DEFAULT, "account empty");
        return 0;
    }
    TAG_LOGD(AAFwkTag::DEFAULT, "accountId: %{public}d", accountIds[0]);
    return accountIds[0];
}
}  // namespace AppExecFwk
}  // namespace OHOS