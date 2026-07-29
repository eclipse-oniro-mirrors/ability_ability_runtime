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

#include "ability_keep_alive_service.h"

#include "ability_keep_alive_data_manager.h"
#include "ability_manager_service.h"
#include "hilog_tag_wrapper.h"
#include "user_controller/user_controller.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AAFwk;

AbilityKeepAliveService &AbilityKeepAliveService::GetInstance()
{
    static AbilityKeepAliveService instance;
    return instance;
}

AbilityKeepAliveService::AbilityKeepAliveService() {}

AbilityKeepAliveService::~AbilityKeepAliveService() {}

int32_t AbilityKeepAliveService::SetApplicationKeepAlive(KeepAliveInfo &info, bool flag)
{
    TAG_LOGD(AAFwkTag::KEEP_ALIVE, "SetApplicationKeepAlive is called,"
        " bundleName: %{public}s, userId: %{public}d, flag: %{public}d",
        info.bundleName.c_str(), info.userId, static_cast<int>(flag));

    GetValidUserId(info.userId);

    if (flag) {
        return SetKeepAliveTrue(info);
    }
    return CancelKeepAlive(info);
}

int32_t AbilityKeepAliveService::SetKeepAliveTrue(const KeepAliveInfo &info)
{
    KeepAliveStatus status = AbilityKeepAliveDataManager::GetInstance().QueryKeepAliveData(info);
    if (status.code != ERR_OK && status.code != ERR_NAME_NOT_FOUND) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "QueryKeepAliveData fail");
        return status.code;
    }

    if (status.code == ERR_NAME_NOT_FOUND) {
        return AbilityKeepAliveDataManager::GetInstance().InsertKeepAliveData(info);
    }

    // check if the app is already set by a higher priority setter
    if (static_cast<int32_t>(status.setter) <= static_cast<int32_t>(info.setter)) {
        TAG_LOGI(AAFwkTag::KEEP_ALIVE, "app is already set");
        return ERR_OK;
    }

    (void)AbilityKeepAliveDataManager::GetInstance().DeleteKeepAliveData(info);
    return AbilityKeepAliveDataManager::GetInstance().InsertKeepAliveData(info);
}

int32_t AbilityKeepAliveService::CancelKeepAlive(const KeepAliveInfo &info)
{
    KeepAliveStatus status = AbilityKeepAliveDataManager::GetInstance().QueryKeepAliveData(info);
    if (status.code != ERR_OK && status.code != ERR_NAME_NOT_FOUND) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "QueryKeepAliveData fail");
        return status.code;
    }

    if (status.code == ERR_NAME_NOT_FOUND) {
        return ERR_TARGET_BUNDLE_NOT_EXIST;
    }

    if (static_cast<int32_t>(status.setter) < static_cast<int32_t>(info.setter)) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "app is set keep-alive by system, cannot unset by user");
        return CHECK_PERMISSION_FAILED;
    }

    return AbilityKeepAliveDataManager::GetInstance().DeleteKeepAliveData(info);
}

int32_t AbilityKeepAliveService::QueryKeepAliveApplications(int32_t userId,
    int32_t appType, std::vector<KeepAliveInfo> &infoList)
{
    GetValidUserId(userId);
    KeepAliveInfo queryParam;
    queryParam.userId = userId;
    queryParam.appType = KeepAliveAppType(appType);
    return AbilityKeepAliveDataManager::GetInstance().QueryKeepAliveApplications(
        queryParam, infoList);
}

void AbilityKeepAliveService::GetValidUserId(int32_t &userId)
{
    if (userId >= 0) {
        return;
    }
    userId = AbilityRuntime::UserController::GetInstance().GetCallerUserId();
}

bool AbilityKeepAliveService::IsKeepAliveApp(const std::string &bundleName, int32_t userId)
{
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "bundle name empty");
        return false;
    }

    GetValidUserId(userId);
    KeepAliveInfo info;
    info.bundleName = bundleName;
    info.userId = userId;
    KeepAliveStatus status = AbilityKeepAliveDataManager::GetInstance().QueryKeepAliveData(info);
    return (status.code == ERR_OK);
}

int32_t AbilityKeepAliveService::GetKeepAliveApplications(int32_t userId, std::vector<KeepAliveInfo> &infoList)
{
    KeepAliveInfo queryParam;
    queryParam.userId = userId;
    return AbilityKeepAliveDataManager::GetInstance().QueryKeepAliveApplications(
        queryParam, infoList);
}

int32_t AbilityKeepAliveService::SetAppServiceExtensionKeepAlive(KeepAliveInfo &info, bool flag)
{
    TAG_LOGD(AAFwkTag::KEEP_ALIVE, "SetAppServiceExtensionKeepAlive is called,"
        " bundleName: %{public}s, userId: %{public}d, flag: %{public}d",
        info.bundleName.c_str(), info.userId, static_cast<int>(flag));

    KeepAliveStatus status = AbilityKeepAliveDataManager::GetInstance().QueryKeepAliveData(info);
    if (status.code != ERR_OK && status.code != ERR_NAME_NOT_FOUND) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "QueryKeepAliveData fail");
        return status.code;
    }

    if (status.code == ERR_NAME_NOT_FOUND) {
        return flag ? AbilityKeepAliveDataManager::GetInstance().InsertKeepAliveData(info) :
            ERR_GET_TARGET_BUNDLE_INFO_FAILED;
    }

    if (!flag) {
        if (info.setterId != status.setterId && info.setter == status.setter) {
            TAG_LOGE(AAFwkTag::KEEP_ALIVE, "setter id is different, cannot unset");
            return ERR_CHANGE_KEEP_ALIVE;
        } else if (info.setter == KeepAliveSetter::USER && status.policy == KeepAlivePolicy::NOT_ALLOW_CANCEL) {
            TAG_LOGE(AAFwkTag::KEEP_ALIVE, "app service extension is set keep-alive by system, cannot unset by user");
            return ERR_CHANGE_KEEP_ALIVE;
        }
        return AbilityKeepAliveDataManager::GetInstance().DeleteKeepAliveData(info);
    }

    if (info.setter == KeepAliveSetter::USER ||
        (info.setter == KeepAliveSetter::SYSTEM && info.policy == status.policy)) {
        TAG_LOGI(AAFwkTag::KEEP_ALIVE, "app service extension is already set");
        return ERR_OK;
    }

    AbilityKeepAliveDataManager::GetInstance().DeleteKeepAliveData(info);
    return AbilityKeepAliveDataManager::GetInstance().InsertKeepAliveData(info);
}

int32_t AbilityKeepAliveService::QueryKeepAliveAppServiceExtensions(std::vector<KeepAliveInfo> &infoList)
{
    KeepAliveInfo queryParam;
    queryParam.userId = U1_USER_ID;
    return AbilityKeepAliveDataManager::GetInstance().QueryKeepAliveApplications(
        queryParam, infoList);
}

int32_t AbilityKeepAliveService::ClearKeepAliveAppServiceExtension(const KeepAliveInfo &info)
{
    TAG_LOGD(AAFwkTag::KEEP_ALIVE, "ClearKeepAliveAppServiceExtension is called,"
        "setterId: %{public}d", info.setterId);
    return AbilityKeepAliveDataManager::GetInstance().DeleteKeepAliveDataWithSetterId(info);
}
} // namespace AbilityRuntime
} // namespace OHOS
