/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "service_router_mgr_service.h"

#include <memory>
#include <string>

#include "ability_manager_client.h"
#include "appexecfwk_errors.h"
#include "bundle_constants.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "hilog_tag_wrapper.h"
#include "if_system_ability_manager.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "service_router_data_mgr.h"
#include "string_ex.h"
#include "sr_samgr_helper.h"
#include "system_ability_definition.h"
#include "want.h"
#include "accesstoken_kit.h"
#include "tokenid_kit.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
const std::string NAME_SERVICE_ROUTER_MGR_SERVICE = "ServiceRouterMgrService";
const std::string TASK_NAME = "ServiceRouterUnloadTask";
const int64_t UNLOAD_DELAY_TIME = 90000;
const int CYCLE_LIMIT = 1000;
}

const bool REGISTER_RESULT =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<ServiceRouterMgrService>::GetInstance().get());

ServiceRouterMgrService::ServiceRouterMgrService() : SystemAbility(SERVICE_ROUTER_MGR_SERVICE_ID, true)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "SRMS instance create");
}

ServiceRouterMgrService::~ServiceRouterMgrService()
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "SRMS instance destroy");
}

void ServiceRouterMgrService::OnStart()
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "SRMS starting...");
    Init();
    bool ret = Publish(this);
    if (!ret) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Publish SRMS failed");
        return;
    }
    DelayUnloadTask();
    TAG_LOGI(AAFwkTag::SER_ROUTER, "SRMS start success");
}

void ServiceRouterMgrService::OnStop()
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "Stop SRMS");
}

void ServiceRouterMgrService::Init()
{
    LoadAllBundleInfos();
    InitEventRunnerAndHandler();
    SubscribeCommonEvent();
}

void ServiceRouterMgrService::DelayUnloadTask()
{
    if (handler_ == nullptr) {
        TAG_LOGI(AAFwkTag::SER_ROUTER, "null handler_");
        return;
    }

    std::lock_guard<std::mutex> lock(delayTaskMutex_);
    handler_->RemoveTask(TASK_NAME);
    auto task = [this]() {
        TAG_LOGI(AAFwkTag::SER_ROUTER, "UnloadSA start");
        sptr<ISystemAbilityManager> saManager =
            OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (saManager == nullptr) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "null saManager");
            return;
        }
        int32_t result = saManager->UnloadSystemAbility(OHOS::SERVICE_ROUTER_MGR_SERVICE_ID);
        if (result != ERR_OK) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "UnloadSystemAbility ret: %{public}d", result);
            return;
        }
        TAG_LOGI(AAFwkTag::SER_ROUTER, "UnloadSA success");
    };
    handler_->PostTask(task, TASK_NAME, UNLOAD_DELAY_TIME);
}

bool ServiceRouterMgrService::LoadAllBundleInfos()
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "start");
    bool ret = ServiceRouterDataMgr::GetInstance().LoadAllBundleInfos();
    TAG_LOGD(AAFwkTag::SER_ROUTER, "end");
    return ret;
}

bool ServiceRouterMgrService::InitEventRunnerAndHandler()
{
    std::lock_guard<std::mutex> lock(mutex_);
    runner_ = EventRunner::Create(NAME_SERVICE_ROUTER_MGR_SERVICE);
    if (runner_ == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null runner_");
        return false;
    }
    handler_ = std::make_shared<EventHandler>(runner_);
    if (handler_ == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null handler_");
        return false;
    }
    return true;
}

bool ServiceRouterMgrService::ServiceRouterMgrService::SubscribeCommonEvent()
{
    if (eventSubscriber_ != nullptr) {
        TAG_LOGI(AAFwkTag::SER_ROUTER, "Already subscribed");
        return true;
    }
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_CHANGED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetThreadMode(EventFwk::CommonEventSubscribeInfo::COMMON);

    eventSubscriber_ = std::make_shared<SrCommonEventSubscriber>(subscribeInfo);
    eventSubscriber_->SetEventHandler(handler_);
    if (!EventFwk::CommonEventManager::SubscribeCommonEvent(eventSubscriber_)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Subscribed failed");
        return false;
    };
    TAG_LOGI(AAFwkTag::SER_ROUTER, "Subscribed success");
    return true;
}

ErrCode ServiceRouterMgrService::QueryBusinessAbilityInfos(const BusinessAbilityFilter& filter,
    std::vector<BusinessAbilityInfo>& businessAbilityInfos, int32_t& funcResult)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "CheckPermission is supported");
    if (!VerifySystemApp()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "verify system app failed");
        funcResult = ERR_BUNDLE_MANAGER_SYSTEM_API_DENIED;
        return funcResult;
    }
    if (!VerifyCallingPermission(Constants::PERMISSION_GET_BUNDLE_INFO_PRIVILEGED)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "verify GET_BUNDLE_INFO_PRIVILEGED failed");
        funcResult = ERR_BUNDLE_MANAGER_PERMISSION_DENIED;
        return funcResult;
    }
    if (funcResult > CYCLE_LIMIT) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "funcResult size too large");
        funcResult = ERR_APPEXECFWK_PARCEL_ERROR;
        return funcResult;
    }
    QueryBusinessAbilityInfosInner(filter, businessAbilityInfos, funcResult);
    return ERR_OK;
}

void ServiceRouterMgrService::QueryBusinessAbilityInfosInner(const BusinessAbilityFilter& filter,
    std::vector<BusinessAbilityInfo>& businessAbilityInfos, int32_t& funcResult)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "coldStart");
    DelayUnloadTask();
    funcResult = ServiceRouterDataMgr::GetInstance().QueryBusinessAbilityInfos(filter, businessAbilityInfos);
}

ErrCode ServiceRouterMgrService::QueryPurposeInfos(const Want& want, const std::string& purposeName,
    std::vector<PurposeInfo>& purposeInfos, int32_t& funcResult)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "coldStart");
    if (!VerifySystemApp()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "verify system app failed");
        funcResult = ERR_BUNDLE_MANAGER_SYSTEM_API_DENIED;
        return funcResult;
    }
    DelayUnloadTask();
    funcResult = ServiceRouterDataMgr::GetInstance().QueryPurposeInfos(want, purposeName, purposeInfos);
    return ERR_OK;
}

ErrCode ServiceRouterMgrService::StartUIExtensionAbility(const SessionInfo& sessionInfo, int32_t userId,
    int32_t& funcResult)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "Called");
    DelayUnloadTask();
    auto shard_sessionInfo = sptr<SessionInfo>::MakeSptr(sessionInfo);
    funcResult = IN_PROCESS_CALL(AbilityManagerClient::GetInstance()->StartUIExtensionAbility(shard_sessionInfo,
        userId));
    return ERR_OK;
}

ErrCode ServiceRouterMgrService::ConnectUIExtensionAbility(const Want& want, const sptr<IAbilityConnection>& connect,
    const SessionInfo& sessionInfo, int32_t userId, int32_t& funcResult)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "Called");
    DelayUnloadTask();
    auto shard_sessionInfo = sptr<SessionInfo>::MakeSptr(sessionInfo);
    funcResult = IN_PROCESS_CALL(AbilityManagerClient::GetInstance()->
        ConnectUIExtensionAbility(want, connect, shard_sessionInfo, userId));
    return ERR_OK;
}

bool ServiceRouterMgrService::VerifySystemApp()
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "Called");
    Security::AccessToken::AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::ATokenTypeEnum tokenType =
        Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(callerToken);
    if (tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE
        || IPCSkeleton::GetCallingUid() == Constants::ROOT_UID) {
        return true;
    }
    uint64_t accessTokenIdEx = IPCSkeleton::GetCallingFullTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(accessTokenIdEx)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "non-system app calling system api");
        return false;
    }
    return true;
}

bool ServiceRouterMgrService::VerifyCallingPermission(const std::string &permissionName)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "Verify: %{public}s", permissionName.c_str());
    OHOS::Security::AccessToken::AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();
    OHOS::Security::AccessToken::ATokenTypeEnum tokenType =
        OHOS::Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(callerToken);
    if (tokenType == OHOS::Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE) {
        return true;
    }
    int32_t ret = OHOS::Security::AccessToken::AccessTokenKit::VerifyAccessToken(callerToken, permissionName);
    if (ret == OHOS::Security::AccessToken::PermissionState::PERMISSION_DENIED) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "PERMISSION_DENIED: %{public}s", permissionName.c_str());
        return false;
    }
    return true;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
