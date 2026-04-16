/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_WANT_AGENT_HELPER_H
#define OHOS_ABILITY_RUNTIME_WANT_AGENT_HELPER_H

#include <string>
#include <memory>
#include "context/application_context.h"
#include "completed_callback.h"
#include "completed_dispatcher.h"
#include "event_handler.h"
#include "local_want_agent_info.h"
#include "nlohmann/json.hpp"
#include "trigger_info.h"
#include "want.h"
#include "want_agent.h"
#include "want_agent_info.h"
#include "want_params.h"

namespace OHOS::AbilityRuntime::WantAgent {
/**
 * A helper class used to obtain, trigger, cancel, and compare WantAgent objects and to obtain
 * the bundle name, UID, and hash code value of an WantAgent object.
 *
 */

static const int FLAG_ONE_SHOT = 1 << 30;
static const int FLAG_NO_CREATE = 1 << 29;
static const int FLAG_CANCEL_CURRENT = 1 << 28;
static const int FLAG_UPDATE_CURRENT = 1 << 27;
static const int FLAG_IMMUTABLE = 1 << 26;
static const int FLAG_ALLOW_CANCEL = 1 << 20;
static const int FLAG_INVLID = 0;

static const int INVLID_WANT_AGENT_USER_ID = -1;

class WantAgentHelper final : public std::enable_shared_from_this<WantAgentHelper> {
public:
    /**
     * Obtains an WantAgent object.
     * The WantAgent class does not have any constructor, and you can only use this method to create an
     * WantAgent object.
     *
     * @param context Indicates the context of the caller. This parameter cannot be null.
     * @param paramsInfo Indicates the WantAgentInfo object that contains parameters of the
     * WantAgent object to create.
     * @return Returns ERR_OK If get wantaget correctly.
     */
    static ErrCode GetWantAgent(
        const std::shared_ptr<OHOS::AbilityRuntime::ApplicationContext> &context,
        const WantAgentInfo &paramsInfo, std::shared_ptr<WantAgent> &wantAgent);

    /**
     * Obtains an WantAgent object.
     *
     * The WantAgent class does not have any constructor, and you can only use this method to create an
     * WantAgent object.
     *
     * @param paramsInfo Indicates the WantAgentInfo object that contains parameters of the
     * WantAgent object to create.
     * @param userId Indicates the user id for this wantagent info, default is INVLID_WANT_AGENT_USER_ID(-1).
     * @return Returns the created WantAgent object.
     */
    static std::shared_ptr<WantAgent> GetWantAgent(const WantAgentInfo &paramsInfo,
        int32_t userId = INVLID_WANT_AGENT_USER_ID, int32_t uid = -1);

    /**
     * Creates a LocalWantAgent object.
     *
     * @param context Indicates the context of the caller. This parameter cannot be null.
     * @param paramsInfo Indicates the LocalWantAgentInfo object that contains parameters of the
     * WantAgent object to create.
     * @return Returns ERR_OK If create local wantagent correctly.
     */
    static ErrCode CreateLocalWantAgent(
        const std::shared_ptr<OHOS::AbilityRuntime::ApplicationContext> &context,
        const LocalWantAgentInfo &paramsInfo, std::shared_ptr<WantAgent> &wantAgent);

    /**
     * Obtains an WantAgent object operation type.
     *
     * @param agent Indicates the WantAgent to trigger.
     * @return Returns the created WantAgent object.
     */
    static WantAgentConstant::OperationType GetType(std::shared_ptr<WantAgent> agent);
    static ErrCode GetType(const std::shared_ptr<WantAgent> &agent, int32_t &operType);

    /**
     * Triggers an WantAgent.
     *
     * After this method is called, events associated with the specified WantAgent will be executed,
     * such as starting an ability or sending a common event.
     *
     * @param context Indicates the context of the caller. This parameter cannot be null.
     * @param agent Indicates the WantAgent to trigger.
     * @param onCompleted Indicates the callback method to be called after the WantAgent is triggered.
     * This parameter can be null.
     * @param handler Indicates the thread for executing the callback indicated by OnCompleted.
     * If this parameter is null, the callback method will be executed in a thread in the thread pool of
     * the current process.
     * @param paramsInfo Indicates the TriggerInfo object that contains triggering parameters.
     */
    static ErrCode TriggerWantAgent(std::shared_ptr<WantAgent> agent,
        const std::shared_ptr<CompletedCallback> &callback,
        const TriggerInfo &paramsInfo, sptr<CompletedDispatcher> &data, sptr<IRemoteObject> callerToken);

    /**
     * Cancels an WantAgent.
     *
     * if flags not equal FLAG_INVLID, cancel only when flags match wantAgent flags.
     *
     * @param agent Indicates the WantAgent to cancel.
     * @param flags Indicates the flags to cancel, default is FLAG_INVLID(0).
     */
    static ErrCode Cancel(const std::shared_ptr<WantAgent> agent, uint32_t flags = FLAG_INVLID);

    /**
     * Checks whether two WantAgent objects are the same.
     *
     * @param agent Indicates one of the WantAgent object to compare.
     * @param otherAgent Indicates the other WantAgent object to compare.
     * @return Returns ERR_OK If the two objects are the same.
     */
    static ErrCode IsEquals(const std::shared_ptr<WantAgent> &agent, const std::shared_ptr<WantAgent> &otherAgent);

    /**
     * @brief Get bundle name by want agent.
     *
     * @param agent The WantAgent.
     * @param bundleName BundleName obtained.
     * @return Returns ERR_OK if get bundle name succeed.
     */
    static ErrCode GetBundleName(const std::shared_ptr<WantAgent> &agent, std::string &bundleName);

    /**
     * @brief Get uid by want agent.
     *
     * @param agent The WantAgent.
     * @param uid Uid obtained.
     * @return Returns ERR_OK if get bundle name succeed.
     */
    static ErrCode GetUid(const std::shared_ptr<WantAgent> &agent, int32_t &uid);

    /**
     * Obtains the Want WantAgent.
     *
     * @param agent Indicates the WantAgent whose Want is to be obtained.
     * @return Returns the Want of the WantAgent.
     */
    static std::shared_ptr<AAFwk::Want> GetWant(const std::shared_ptr<WantAgent> &agent);
    static ErrCode GetWant(const std::shared_ptr<WantAgent> &agent, std::shared_ptr<AAFwk::Want> &want);

    /**
     * Obtains the Want WantAgent from proxy.
     *
     * @param agent Indicates the WantAgent whose Want is to be obtained.
     * @return Returns the Want of the WantAgent.
     */
    static std::shared_ptr<AAFwk::Want> GetWantFromProxy(const std::shared_ptr<WantAgent> &agent);

    /**
     * Register Cancel function Listener.
     *
     * @param cancelListener Register listener object.
     * @param agent Indicates the WantAgent whose bundle name is to be obtained.
     */
    static void RegisterCancelListener(
        const std::shared_ptr<CancelListener> &cancelListener, const std::shared_ptr<WantAgent> &agent);

    /**
     * Unregister Cancel function Listener.
     *
     * @param cancelListener Register listener object.
     * @param agent Indicates the WantAgent whose bundle name is to be obtained.
     */
    static void UnregisterCancelListener(
        const std::shared_ptr<CancelListener> &cancelListener, const std::shared_ptr<WantAgent> &agent);

    /**
     * Convert WantAgentInfo object to json string.
     *
     * @param jsonObject Json object.
     * @return WantAgentInfo object's json string.
     */
    static std::string ToString(const std::shared_ptr<WantAgent> &agent);

    /**
     * Convert json string to WantAgentInfo object.
     *
     * @param jsonString Json string.
     * @return WantAgentInfo object.
     */
    static std::shared_ptr<WantAgent> FromString(const std::string &jsonString, int32_t uid = -1);

private:
    WantAgentHelper();
    virtual ~WantAgentHelper() = default;

private:
    static ErrCode Send(const std::shared_ptr<PendingWant> &pendingWant,
        WantAgentConstant::OperationType type,
        sptr<CompletedDispatcher> &callBack,
        const TriggerInfo &paramsInfo,
        sptr<IRemoteObject> callerToken);

    static ErrCode Send(const std::shared_ptr<LocalPendingWant> &localPendingWant,
        const sptr<CompletedDispatcher> &callBack,
        const TriggerInfo &paramsInfo,
        sptr<IRemoteObject> callerToken);

    static unsigned int FlagsTransformer(const std::vector<WantAgentConstant::Flags> &flags);

    static std::vector<WantAgentConstant::Flags> ParseFlags(nlohmann::json jsonObject);
};
}  // namespace OHOS::AbilityRuntime::WantAgent
#endif  // OHOS_ABILITY_RUNTIME_WANT_AGENT_HELPER_H
