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

#ifndef OHOS_ABILITY_RUNTIME_CLI_SA_CLIENT_H
#define OHOS_ABILITY_RUNTIME_CLI_SA_CLIENT_H

#include <condition_variable>
#include <functional>
#include <mutex>

#include "cli_session_info.h"
#include "cli_tool_event.h"
#include "exec_cmd_param.h"
#include "exec_options.h"
#include "icli_tool_manager.h"
#include "iremote_object.h"

namespace OHOS {
namespace CliTool {

using ClearProxyCallback = std::function<void(const wptr<IRemoteObject>&)>;

class SessionEventCallback {
public:
    SessionEventCallback() = default;
    virtual ~SessionEventCallback() = default;

    virtual void OnToolEvent(const std::string &sessionId,
                             const std::string &subscriptionId,
                             const CliToolEvent &event) {}
};

/**
 * @class CliToolMGRClient
 * CliToolMGRClient provides client access to the CliSaService.
 * This is a singleton class that manages connection to the service.
 */
class CliToolMGRClient {
public:
    using ExecToolReplyCallback = std::function<void(int32_t, const CliSessionInfo &)>;

    using EventReplyCallback = std::function<void(int32_t)>;

    /**
     * @brief Get the singleton instance of CliToolMGRClient.
     * @return Reference to the CliToolMGRClient instance.
     */
    static CliToolMGRClient& GetInstance();

    /**
     * @brief Destructor.
     */
    ~CliToolMGRClient() = default;

    /**
     * @brief Get all tool summaries (lightweight for listing)
     * @param summaries Output vector of ToolSummary
     * @return ErrCode ERR_OK on success
     */
    ErrCode GetAllToolSummaries(std::vector<ToolSummary> &summaries);

    /**
     * @brief Get tool information by name
     * @param name Tool name
     * @param tool Output ToolInfo
     * @return ErrCode ERR_OK on success
     */
    ErrCode GetToolInfoByName(const std::string &name, ToolInfo &tool);

    /**
     * @brief Get all tool infos
     * @param tools Output vector of ToolInfo
     * @return ErrCode ERR_OK on success
     */
    ErrCode GetAllToolInfos(std::vector<ToolInfo> &tools);

    /**
     * @brief Register a CLI tool
     * @param tool ToolInfo to register
     * @return ErrCode ERR_OK on success
     */
    ErrCode RegisterTool(const ToolInfo &tool);

    /**
     * @brief Register a function
     * @param function FunctionInfo to register
     * @return ErrCode ERR_OK on success
     */
    ErrCode RegisterFunction(const FunctionInfo &function);

    /**
     * @brief Batch register functions
     * @param functions Vector of FunctionInfo to register
     * @param successCount Output count of successfully registered functions
     * @return ErrCode ERR_OK on success
     */
    ErrCode BatchRegisterFunctions(const std::vector<FunctionInfo> &functions, int32_t &successCount);

    /**
     * @brief Get function information by bundleName and functionName
     * @param bundleName Bundle name
     * @param functionName Function name
     * @param function Output FunctionInfo
     * @return ErrCode ERR_OK on success
     */
    ErrCode GetFunctionInfo(const std::string &functionNamespace, const std::string &functionName,
        FunctionInfo &function);

    /**
     * @brief Unregister a function
     * @param functionNamespace Namespace
     * @param functionName Function name
     * @return ErrCode ERR_OK on success
     */
    ErrCode UnregisterFunction(const std::string &functionNamespace, const std::string &functionName);

    /**
     * @brief Batch unregister intentFunctions by namespace
     * @param functionNamespace Namespace to delete all functions from
     * @return ErrCode ERR_OK on success
     */
    ErrCode UnregisterIntentFunctionsByNamespace(const std::string &functionNamespace);

    /**
     * @brief Get all functions
     * @param functions Output vector of FunctionInfo
     * @return ErrCode ERR_OK on success
     */
    ErrCode GetAllFunctions(std::vector<FunctionInfo> &functions);

    /**
     * @brief Execute a CLI tool with key-value pairs (convenience method).
     * @param param The CLI tool param.
     * @param callback reply callback.
     * @return Returns ERR_OK on success, error code otherwise.
     */
    ErrCode ExecTool(const ExecToolParam &param, const ExecToolReplyCallback &callback);

    ErrCode ExecCmd(const ExecCmdParam &param,
        const ExecToolReplyCallback &callback, const std::shared_ptr<SessionEventCallback> &sessionEventCallback);

    ErrCode SubscribeSession(const std::string &sessionId,
                             const std::shared_ptr<SessionEventCallback> &callback,
                             std::string &subscriptionId);

    ErrCode UnsubscribeSession(const std::string &sessionId, const std::string &subscriptionId);

    ErrCode ClearSession(const std::string &sessionId);

    ErrCode QuerySession(const std::string &sessionId, CliSessionInfo &session);

    ErrCode SendMessage(const std::string &sessionId,
                        const std::string &inputText,
                        const EventReplyCallback &callback);

    /**
     * @brief Batch query command permissions (Inner API, for SA only).
     * @param cmds Command list to query.
     * @param cmdPermissions Query result list.
     * @return ERR_OK Success
     *         ERR_NOT_SA_CALLER Caller is not SA
     *         ERR_PERMISSION_DENIED Missing ohos.permission.QUERY_CLI_TOOL permission
     */
    int32_t BatchQueryPermissionBySubCommand(const std::vector<Command> &cmds,
        std::vector<CommandPermission> &cmdPermissions);

    /**
     * @brief On load system ability success.
     * @param remoteObject The remote object of system ability.
     */
    void OnLoadSystemAbilitySuccess(const sptr<IRemoteObject> &remoteObject);

    /**
     * @brief On load system ability fail.
     */
    void OnLoadSystemAbilityFail();

private:
    CliToolMGRClient() = default;
    DISALLOW_COPY_AND_MOVE(CliToolMGRClient);

    class CliMgrDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit CliMgrDeathRecipient(const ClearProxyCallback &callback) : callback_(callback) {}
        ~CliMgrDeathRecipient() override = default;
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override;

    private:
        ClearProxyCallback callback_;
    };

    sptr<ICliToolManager> GetCliToolMgrProxy();
    bool LoadCliToolMgrService();
    void ClearProxy();
    void SetCliToolMgr(const sptr<IRemoteObject> &remoteObject);
    sptr<ICliToolManager> GetCliToolMgr();
    ErrCode EnsureSchedulerStubCreated();

    std::condition_variable loadSaCondation_;
    std::mutex loadSaMutex_;
    bool loadSaFinished_;
    std::mutex proxyMutex_;
    sptr<ICliToolManager> cliToolMgr_ = nullptr;
    std::mutex schedulerMutex_;
    sptr<ICliToolManagerScheduler> schedulerStub_ = nullptr;
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_SA_CLIENT_H
