/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_SERVICE_ROUTER_FRAMEWORK_SERVICES_INCLUDE_SERVICE_ROUTER_MGR_SERVICE_H
#define OHOS_ABILITY_RUNTIME_SERVICE_ROUTER_FRAMEWORK_SERVICES_INCLUDE_SERVICE_ROUTER_MGR_SERVICE_H

#include "event_runner.h"
#include "event_handler.h"
#include "service_info.h"
#include "service_router_mgr_stub.h"
#include "singleton.h"
#include "sr_common_event_subscriber.h"
#include "system_ability.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;

/**
 * @class ServiceRouterMgrService
 * ServiceRouterMgrService provides a facility for managing ability life cycle.
 */
class ServiceRouterMgrService : public SystemAbility, public ServiceRouterMgrStub {
    DECLARE_DELAYED_SINGLETON(ServiceRouterMgrService)
    DECLEAR_SYSTEM_ABILITY(ServiceRouterMgrService)
public:
    /**
     * @brief Start the service router manager service.
     * @return
     */
    virtual void OnStart() override;
    /**
     * @brief Stop the service router service.
     * @return
     */
    virtual void OnStop() override;

    /**
     * @brief Check whether if the service router service is ready.
     * @return Returns true if the service router service is ready; returns false otherwise.
     */
    bool IsServiceReady() const;

    /**
     * @brief Query the business ability info of list by the given filter.
     * @param filter Indicates the filter containing the business ability info to be queried.
     * @param businessAbilityInfos Indicates the obtained business ability info objects
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual ErrCode QueryBusinessAbilityInfos(const BusinessAbilityFilter& filter,
        std::vector<BusinessAbilityInfo>& businessAbilityInfos, int32_t& funcResult) override;

    /**
     * @brief Query the PurposeInfo of list by the given Want.
     * @param want Indicates the information of the want.
     * @param purposeName Indicates the purposeName.
     * @param purposeInfos Indicates the obtained PurposeInfos object.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual ErrCode QueryPurposeInfos(const Want& want, const std::string& purposeName,
        std::vector<PurposeInfo>& purposeInfos, int32_t& funcResult) override;

    /**
     * Start ui extension ability with extension session info, send extension session info to ability manager service.
     *
     * @param sessionInfo the extension session info of the ability to start.
     * @param userId, Designation User ID.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual ErrCode StartUIExtensionAbility(const SessionInfo& sessionInfo,
        int32_t userId, int32_t& funcResult) override;

    /**
     * Connect ui extension ability with want, connect session with service ability.
     *
     * @param want, Special want for service type's ability.
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @param sessionInfo the extension session info of the ability to start.
     * @param userId, Designation User ID.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual ErrCode ConnectUIExtensionAbility(const Want& want, const sptr<IAbilityConnection>& connect,
        const SessionInfo& sessionInfo, int32_t userId, int32_t& funcResult) override;

private:
    void Init();
    bool InitEventRunnerAndHandler();
    bool LoadAllBundleInfos();
    bool SubscribeCommonEvent();
    bool SubscribeBundleEvent();
    void DelayUnloadTask();
    bool VerifyCallingPermission(const std::string &permissionName);
    bool VerifySystemApp();
    void QueryBusinessAbilityInfosInner(const BusinessAbilityFilter& filter,
        std::vector<BusinessAbilityInfo>& businessAbilityInfos, int32_t& funcResult);

    std::shared_ptr<EventRunner> runner_ = nullptr;
    std::shared_ptr<EventHandler> handler_ = nullptr;
    std::mutex bundleMgrMutex_;
    std::mutex mutex_;
    std::shared_ptr<SrCommonEventSubscriber> eventSubscriber_ = nullptr;
    std::mutex delayTaskMutex_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_SERVICE_ROUTER_FRAMEWORK_SERVICES_INCLUDE_SERVICE_ROUTER_MGR_SERVICE_H
