/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include <thread>
#include "dataobs_mgr_client.h"

#include "common_utils.h"
#include "dataobs_mgr_interface.h"
#include "datashare_log.h"
#include "hilog_tag_wrapper.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "system_ability_status_change_stub.h"

namespace OHOS {
namespace AAFwk {
std::mutex DataObsMgrClient::mutex_;
constexpr const int32_t RETRY_MAX_TIMES = 100;
constexpr const int64_t RETRY_SLEEP_TIME_MILLISECOND = 1; // 1ms
using namespace DataShare;

class DataObsMgrClient::SystemAbilityStatusChangeListener
    : public SystemAbilityStatusChangeStub {
public:
    SystemAbilityStatusChangeListener()
    {
    }
    ~SystemAbilityStatusChangeListener() = default;
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override
    {
    }
};

void DataObsMgrClient::SystemAbilityStatusChangeListener::OnAddSystemAbility(
    int32_t systemAbilityId, const std::string &deviceId)
{
    TAG_LOGI(AAFwkTag::DBOBSMGR, "called");
    if (systemAbilityId != DATAOBS_MGR_SERVICE_SA_ID) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "OnAddSystemAbility! systemAbilityId:%{public}d", systemAbilityId);
        return;
    }
    GetInstance()->ResetService();
    GetInstance()->ReRegister();
}

std::shared_ptr<DataObsMgrClient> DataObsMgrClient::GetInstance()
{
    static std::shared_ptr<DataObsMgrClient> proxy = std::make_shared<DataObsMgrClient>();
    return proxy;
}

DataObsMgrClient::DataObsMgrClient()
{
    callback_ = new SystemAbilityStatusChangeListener();
}

DataObsMgrClient::~DataObsMgrClient()
{
    if (dataObsManger_ != nullptr) {
        dataObsManger_->AsObject()->RemoveDeathRecipient(deathRecipient_);
    }
    sptr<ISystemAbilityManager> systemManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemManager == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "null systemmgr");
        return;
    }
    auto res = systemManager->UnSubscribeSystemAbility(DATAOBS_MGR_SERVICE_SA_ID, callback_);
    if (res != 0) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "UnSubscribeSystemAbility failed, errCode is %{public}d.", res);
        systemManager->UnSubscribeSystemAbility(DATAOBS_MGR_SERVICE_SA_ID, callback_);
    }

    int32_t retryTimes = 0;
    while ((callback_ != nullptr) && (callback_->GetSptrRefCount() > 1) && (retryTimes < RETRY_MAX_TIMES)) {
        retryTimes++;
        std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_SLEEP_TIME_MILLISECOND));
    }

    if ((callback_ != nullptr) && (callback_->GetSptrRefCount() > 1)) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "Callback has other in use: %{public}d.", callback_->GetSptrRefCount());
    }
}

/**
 * Registers an observer to DataObsMgr specified by the given Uri.
 *
 * @param uri, Indicates the path of the data to operate.
 * @param dataObserver, Indicates the IDataAbilityObserver object.
 *
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode DataObsMgrClient::RegisterObserver(const Uri &uri, sptr<IDataAbilityObserver> dataObserver, int userId,
    DataObsOption opt)
{
    auto [errCode, dataObsManger] = GetObsMgr();
    if (errCode != SUCCESS) {
        LOG_ERROR("Failed to get ObsMgr, errCode: %{public}d.", errCode);
        return DATAOBS_SERVICE_NOT_CONNECTED;
    }
    auto status = dataObsManger->RegisterObserver(uri, dataObserver, userId, opt);
    if (status != NO_ERROR) {
        return status;
    }
    observers_.Compute(dataObserver, [&uri, userId](const auto &key, auto &value) {
        value.emplace_back(uri, userId);
        return true;
    });
    return status;
}

ErrCode DataObsMgrClient::RegisterObserverFromExtension(const Uri &uri, sptr<IDataAbilityObserver> dataObserver,
    int userId, DataObsOption opt)
{
    auto [errCode, dataObsManger] = GetObsMgr();
    if (errCode != SUCCESS) {
        LOG_ERROR("Failed to get ObsMgr, errCode: %{public}d.", errCode);
        return DATAOBS_SERVICE_NOT_CONNECTED;
    }
    auto status = dataObsManger->RegisterObserverFromExtension(uri, dataObserver, userId, opt);
    if (status != NO_ERROR) {
        return status;
    }
    uint32_t firstCallerTokenID = opt.FirstCallerTokenID();
    observers_.Compute(dataObserver, [&uri, userId, firstCallerTokenID](const auto &key, auto &value) {
        ObserverInfo info(uri, userId);
        info.isExtension = true;
        info.firstCallerTokenID = firstCallerTokenID;
        value.emplace_back(info);
        return true;
    });
    return status;
}

/**
 * Deregisters an observer used for DataObsMgr specified by the given Uri.
 *
 * @param uri, Indicates the path of the data to operate.
 * @param dataObserver, Indicates the IDataAbilityObserver object.
 *
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode DataObsMgrClient::UnregisterObserver(const Uri &uri, sptr<IDataAbilityObserver> dataObserver, int userId,
    DataObsOption opt)
{
    auto [errCode, dataObsManger] = GetObsMgr();
    if (errCode != SUCCESS) {
        return DATAOBS_SERVICE_NOT_CONNECTED;
    }
    auto status = dataObsManger->UnregisterObserver(uri, dataObserver, userId, opt);
    if (status != NO_ERROR) {
        return status;
    }
    observers_.Compute(dataObserver, [&uri, userId](const auto &key, auto &value) {
        value.remove_if([&uri, userId](const auto &val) {
            return uri == val.uri && userId == val.userId;
        });
        return !value.empty();
    });
    return status;
}

/**
 * Notifies the registered observers of a change to the data resource specified by Uri.
 *
 * @param uri, Indicates the path of the data to operate.
 *
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode DataObsMgrClient::NotifyChange(const Uri &uri, int userId, DataObsOption opt)
{
    auto [errCode, dataObsManger] = GetObsMgr();
    if (errCode != SUCCESS) {
        return DATAOBS_SERVICE_NOT_CONNECTED;
    }
    return dataObsManger->NotifyChange(uri, userId, opt);
}

/**
 * Notifies the registered observers of a change to the data resource specified by Uri.
 *
 * @param uri, Indicates the path of the data to operate.
 *
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode DataObsMgrClient::NotifyChangeFromExtension(const Uri &uri, int userId, DataObsOption opt)
{
    auto [errCode, dataObsManger] = GetObsMgr();
    if (errCode != SUCCESS) {
        return DATAOBS_SERVICE_NOT_CONNECTED;
    }
    return dataObsManger->NotifyChangeFromExtension(uri, userId, opt);
}

/**
 * Connect dataobs manager service.
 *
 * @return Returns SUCCESS on success, others on failure.
 */
__attribute__ ((no_sanitize("cfi"))) std::pair<Status, sptr<IDataObsMgr>> DataObsMgrClient::GetObsMgr()
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (dataObsManger_ != nullptr) {
        return std::make_pair(SUCCESS, dataObsManger_);
    }

    sptr<ISystemAbilityManager> systemManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemManager == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "registry failed");
        return std::make_pair(GET_DATAOBS_SERVICE_FAILED, nullptr);
    }

    auto remoteObject = systemManager->CheckSystemAbility(DATAOBS_MGR_SERVICE_SA_ID);
    if (remoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "systemAbility failed");
        return std::make_pair(GET_DATAOBS_SERVICE_FAILED, nullptr);
    }

    dataObsManger_ = iface_cast<IDataObsMgr>(remoteObject);
    if (dataObsManger_ == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "iDataObsMgr failed");
        return std::make_pair(GET_DATAOBS_SERVICE_FAILED, nullptr);
    }
    if (deathRecipient_ == nullptr) {
        deathRecipient_ = new (std::nothrow) ServiceDeathRecipient(GetInstance());
    }
    dataObsManger_->AsObject()->AddDeathRecipient(deathRecipient_);
    return std::make_pair(SUCCESS, dataObsManger_);
}

Status DataObsMgrClient::RegisterObserverExt(const Uri &uri, sptr<IDataAbilityObserver> dataObserver,
    bool isDescendants, DataObsOption opt)
{
    auto [errCode, dataObsManger] = GetObsMgr();
    if (errCode != SUCCESS) {
        return DATAOBS_SERVICE_NOT_CONNECTED;
    }
    auto status = dataObsManger->RegisterObserverExt(uri, dataObserver, isDescendants, opt);
    if (status != SUCCESS) {
        return status;
    }
    observerExts_.Compute(dataObserver, [&uri, isDescendants](const auto &key, auto &value) {
        value.emplace_back(uri, isDescendants);
        return true;
    });
    return status;
}

Status DataObsMgrClient::UnregisterObserverExt(const Uri &uri, sptr<IDataAbilityObserver> dataObserver,
    DataObsOption opt)
{
    auto [errCode, dataObsManger] = GetObsMgr();
    if (errCode != SUCCESS) {
        return DATAOBS_SERVICE_NOT_CONNECTED;
    }
    auto status = dataObsManger->UnregisterObserverExt(uri, dataObserver);
    if (status != SUCCESS) {
        return status;
    }
    observerExts_.Compute(dataObserver, [&uri](const auto &key, auto &value) {
        value.remove_if([&uri](const auto &param) {
            return uri == param.uri;
        });
        return !value.empty();
    });
    return status;
}

Status DataObsMgrClient::UnregisterObserverExt(sptr<IDataAbilityObserver> dataObserver, DataObsOption opt)
{
    auto [errCode, dataObsManger] = GetObsMgr();
    if (errCode != SUCCESS) {
        return DATAOBS_SERVICE_NOT_CONNECTED;
    }
    auto status = dataObsManger->UnregisterObserverExt(dataObserver, opt);
    if (status != SUCCESS) {
        return status;
    }
    observerExts_.Erase(dataObserver);
    return status;
}

Status DataObsMgrClient::NotifyChangeExt(const ChangeInfo &changeInfo, DataObsOption opt)
{
    auto [errCode, dataObsManger] = GetObsMgr();
    if (errCode != SUCCESS) {
        return DATAOBS_SERVICE_NOT_CONNECTED;
    }
    return dataObsManger->NotifyChangeExt(changeInfo, opt);
}

Status DataObsMgrClient::NotifyProcessObserver(const std::string &key, const sptr<IRemoteObject> observer)
{
    if (key.empty() || observer == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "Null observer, key:%{public}s", key.c_str());
        return INVALID_PARAM;
    }
    auto [errCode, dataObsManger] = GetObsMgr();
    if (errCode != SUCCESS) {
        return DATAOBS_SERVICE_NOT_CONNECTED;
    }
    return dataObsManger->NotifyProcessObserver(key, observer);
}

void DataObsMgrClient::ResetService()
{
    std::lock_guard<std::mutex> lock(mutex_);
    dataObsManger_ = nullptr;
}

void DataObsMgrClient::OnRemoteDied()
{
    sptr<ISystemAbilityManager> systemManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemManager == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "null systemmgr");
        return;
    }
    systemManager->SubscribeSystemAbility(DATAOBS_MGR_SERVICE_SA_ID, callback_);
}

int32_t DataObsMgrClient::TryRegisterObserver(const Uri &uri, sptr<IDataAbilityObserver> key, int userId,
    bool isExtension, DataObsOption opt)
{
    int32_t ret;
    int retryCount = 0;
    const int maxRetries = 3;
    do {
        if (isExtension) {
            ret = RegisterObserverFromExtension(uri, key, userId, opt);
        } else {
            ret = RegisterObserver(uri, key, userId);
        }
        if (ret != SUCCESS) {
            TAG_LOGE(AAFwkTag::DBOBSMGR,
                "RegisterObserver failed, uri:%{public}s, ret:%{public}d, isExtension %{public}d, attempt:%{public}d",
                CommonUtils::Anonymous(uri.ToString()).c_str(), ret, isExtension, retryCount + 1
                );
        }
        ++retryCount;
    } while (ret != SUCCESS && retryCount < maxRetries);
    return ret;
}

void DataObsMgrClient::ReRegister()
{
    decltype(observers_) observers(std::move(observers_));
    observers_.Clear();
    observers.ForEach([this](const auto &key, const auto &value) {
        for (const auto &val : value) {
            int32_t ret = (val.isExtension)
                ? TryRegisterObserver(val.uri, key, val.userId, true, DataObsOption{val.firstCallerTokenID})
                : TryRegisterObserver(val.uri, key, val.userId, false);

            if (ret != SUCCESS) {
                TAG_LOGE(AAFwkTag::DBOBSMGR, "RegisterObserver failed after 3 attempts, uri:%{public}s, ret:%{public}d",
                    CommonUtils::Anonymous(val.uri.ToString()).c_str(), ret);
            }
        }
        return false;
    });
    decltype(observerExts_) observerExts(std::move(observerExts_));
    observerExts_.Clear();
    observerExts.ForEach([this](const auto &key, const auto &value) {
        for (const auto &param : value) {
            int32_t ret = RegisterObserverExt(param.uri, key, param.isDescendants);
            if (ret != SUCCESS) {
                TAG_LOGE(AAFwkTag::DBOBSMGR,
                    "RegisterObserverExt failed, param.uri:%{public}s, ret:%{public}d, param.isDescendants:%{public}d",
                    CommonUtils::Anonymous(param.uri.ToString()).c_str(), ret, param.isDescendants
                );
            }
        }
        return false;
    });
}

}  // namespace AAFwk
}  // namespace OHOS
