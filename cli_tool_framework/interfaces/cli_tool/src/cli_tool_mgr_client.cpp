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

#include "cli_tool_mgr_client.h"

#include "cli_error_code.h"
#include "cli_event_reply_manager.h"
#include "cli_mgr_load_callback.h"
#include "cli_session_subscription_manager.h"
#include "cli_tool_mgr_scheduler_recipient.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace CliTool {

namespace {
constexpr int32_t ERR_OK = 0;
} // namespace

CliToolMGRClient& CliToolMGRClient::GetInstance()
{
    static CliToolMGRClient instance;
    return instance;
}

ErrCode CliToolMGRClient::ExecTool(const ExecToolParam &param, const ExecToolReplyCallback &callback)
{
    ErrCode ret = EnsureSchedulerStubCreated();
    if (ret != ERR_OK) {
        return ret;
    }

    auto proxy = GetCliToolMgrProxy();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "connect failed");
        return GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }

    std::string eventId = CliEventReplyManager::GetInstance().AddEventReplyCallback(param.toolName,
        [cb = std::move(callback)](const CliEventReplyResult &result) {
            if (cb) {
                CliSessionInfo sessionInfo;
                if (result.sessionInfo.has_value()) {
                    sessionInfo = result.sessionInfo.value();
                }
                cb(result.code, sessionInfo);
            }
        });
    ret = proxy->ExecTool(param, eventId, schedulerStub_);
    if (ret != ERR_OK) {
        CliEventReplyManager::GetInstance().RemoveEventReplyCallback(eventId);
    } else {
        CliEventReplyManager::GetInstance().ActivateEventReplyCallback(eventId);
    }
    return ret;
}

ErrCode CliToolMGRClient::ExecCmd(const ExecCmdParam &param,
    const ExecToolReplyCallback &callback, const std::shared_ptr<SessionEventCallback> &sessionEventCallback)
{
    ErrCode ret = EnsureSchedulerStubCreated();
    if (ret != ERR_OK) {
        return ret;
    }

    auto proxy = GetCliToolMgrProxy();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "connect failed");
        return GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }

    std::string eventId = CliEventReplyManager::GetInstance().AddEventReplyCallback("shell",
        [cb = std::move(callback)](const CliEventReplyResult &result) {
            if (cb) {
                CliSessionInfo sessionInfo;
                if (result.sessionInfo.has_value()) {
                    sessionInfo = result.sessionInfo.value();
                }
                cb(result.code, sessionInfo);
            }
        });
    
    std::string subscriptionId = CliSessionSubscriptionManager::GetInstance().AddProvisionalSubscription("shell",
        [sessionEventCallback](const std::string &sessionId,
            const std::string &subscriptionId, const CliToolEvent &event) {
            if (sessionEventCallback) {
                sessionEventCallback->OnToolEvent(sessionId, subscriptionId, event);
            }
        });
    ret = proxy->ExecCmd(param, eventId, schedulerStub_, subscriptionId);
    if (ret != ERR_OK) {
        CliEventReplyManager::GetInstance().RemoveEventReplyCallback(eventId);
        CliSessionSubscriptionManager::GetInstance().RemoveSubscription(subscriptionId);
    } else {
        CliEventReplyManager::GetInstance().ActivateEventReplyCallback(eventId);
        CliSessionSubscriptionManager::GetInstance().ActivateSubscription(subscriptionId);
    }
    return ret;
}

ErrCode CliToolMGRClient::EnsureSchedulerStubCreated()
{
    std::lock_guard<std::mutex> lock(schedulerMutex_);
    if (schedulerStub_ != nullptr) {
        return ERR_OK;
    }
    schedulerStub_ = sptr<CliToolManagerSchedulerRecipient>::MakeSptr();
    if (schedulerStub_ == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "EnsureSchedulerStubCreated failed");
        return ERR_NO_INIT;
    }
    return ERR_OK;
}

ErrCode CliToolMGRClient::GetAllToolSummaries(std::vector<ToolSummary> &summaries)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto proxy = GetCliToolMgrProxy();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "proxy is null");
        return GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    return proxy->GetAllToolSummaries(summaries);
}

ErrCode CliToolMGRClient::GetToolInfoByName(const std::string &name, ToolInfo &tool)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto proxy = GetCliToolMgrProxy();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "proxy is null");
        return GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    return proxy->GetToolInfoByName(name, tool);
}

ErrCode CliToolMGRClient::GetAllToolInfos(std::vector<ToolInfo> &tools)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto proxy = GetCliToolMgrProxy();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "proxy is null");
        return GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    ToolsRawData rawData;
    auto ret = proxy->GetAllToolInfos(rawData);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetAllToolInfos failed: %{public}d", ret);
        return ret;
    }
    return ToolsRawData::ToToolInfoVec(rawData, tools);
}

ErrCode CliToolMGRClient::RegisterTool(const ToolInfo &tool)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto proxy = GetCliToolMgrProxy();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "proxy is null");
        return GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    return proxy->RegisterTool(tool);
}

ErrCode CliToolMGRClient::RegisterFunction(const FunctionInfo &function)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "RegisterFunction: %{public}s/%{public}s",
        function.functionNamespace.c_str(), function.functionName.c_str());
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto proxy = GetCliToolMgrProxy();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "proxy is null");
        return GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    return proxy->RegisterFunction(function);
}

ErrCode CliToolMGRClient::BatchRegisterFunctions(const std::vector<FunctionInfo> &functions,
    int32_t &successCount)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "BatchRegisterFunctions: %{public}zu functions", functions.size());
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto proxy = GetCliToolMgrProxy();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "proxy is null");
        return GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    return proxy->BatchRegisterFunctions(functions, successCount);
}

ErrCode CliToolMGRClient::GetFunctionInfo(const std::string &functionNamespace, const std::string &functionName,
    FunctionInfo &function)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "GetFunctionInfo: %{public}s/%{public}s",
        functionNamespace.c_str(), functionName.c_str());
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto proxy = GetCliToolMgrProxy();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "proxy is null");
        return GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    return proxy->GetFunctionInfo(functionNamespace, functionName, function);
}

ErrCode CliToolMGRClient::UnregisterFunction(const std::string &functionNamespace, const std::string &functionName)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "UnregisterFunction: %{public}s/%{public}s",
        functionNamespace.c_str(), functionName.c_str());
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto proxy = GetCliToolMgrProxy();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "proxy is null");
        return GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    return proxy->UnregisterFunction(functionNamespace, functionName);
}

ErrCode CliToolMGRClient::UnregisterIntentFunctionsByNamespace(const std::string &functionNamespace)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "UnregisterIntentFunctionsByNamespace: %{public}s", functionNamespace.c_str());
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto proxy = GetCliToolMgrProxy();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "proxy is null");
        return GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    return proxy->UnregisterIntentFunctionsByNamespace(functionNamespace);
}

ErrCode CliToolMGRClient::GetAllFunctions(std::vector<FunctionInfo> &functions)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "GetAllFunctions called");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto proxy = GetCliToolMgrProxy();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "proxy is null");
        return GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    FunctionsRawData rawData;
    int32_t ret = proxy->GetAllFunctions(rawData);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetAllFunctions failed: %{public}d", ret);
        return ret;
    }
    return FunctionsRawData::ToFunctionInfoVec(rawData, functions);
}

int32_t CliToolMGRClient::BatchQueryPermissionBySubCommand(
    const std::vector<Command> &cmds,
    std::vector<CommandPermission> &cmdPermissions)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto proxy = GetCliToolMgrProxy();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "proxy is null");
        return GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    return proxy->BatchQueryPermissionBySubCommand(cmds, cmdPermissions);
}

void CliToolMGRClient::OnLoadSystemAbilitySuccess(const sptr<IRemoteObject> &remoteObject)
{
    SetCliToolMgr(remoteObject);
    std::unique_lock<std::mutex> lock(loadSaMutex_);
    loadSaFinished_ = true;
    loadSaCondation_.notify_one();
}

void CliToolMGRClient::OnLoadSystemAbilityFail()
{
    SetCliToolMgr(nullptr);
    std::unique_lock<std::mutex> lock(loadSaMutex_);
    loadSaFinished_ = true;
    loadSaCondation_.notify_one();
}

sptr<ICliToolManager> CliToolMGRClient::GetCliToolMgrProxy()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::CLI_TOOL, "called");
    auto cliToolMgr = GetCliToolMgr();
    if (cliToolMgr != nullptr) {
        TAG_LOGD(AAFwkTag::CLI_TOOL, "Cli tool manager has been started");
        return cliToolMgr;
    }

    if (!LoadCliToolMgrService()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Load cli tool manager service failed");
        return nullptr;
    }

    cliToolMgr = GetCliToolMgr();
    if (cliToolMgr == nullptr || cliToolMgr->AsObject() == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to get cli tool manager");
        return nullptr;
    }

    const auto &onClearProxyCallback = [](const wptr<IRemoteObject> &remote) {
        auto &instance = GetInstance();
        auto cliToolMgr = instance.GetCliToolMgr();
        if (cliToolMgr != nullptr && cliToolMgr->AsObject() == remote) {
            instance.ClearProxy();
        }
    };

    sptr<CliMgrDeathRecipient> recipient(new (std::nothrow) CliMgrDeathRecipient(onClearProxyCallback));
    cliToolMgr->AsObject()->AddDeathRecipient(recipient);

    return cliToolMgr;
}

bool CliToolMGRClient::LoadCliToolMgrService()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    const int32_t LOAD_SA_TIMEOUT_MS = 4 * 1000;
    {
        std::unique_lock<std::mutex> lock(loadSaMutex_);
        loadSaFinished_ = false;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "GetSystemAbilityManager");
    auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityMgr == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to get SystemAbilityManager");
        return false;
    }

    sptr<CliMgrLoadCallback> loadCallback = sptr<CliMgrLoadCallback>::MakeSptr();
    if (loadCallback == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Create load callback failed");
        return false;
    }

    auto ret = systemAbilityMgr->LoadSystemAbility(CLI_TOOL_MGR_SERVICE_ID, loadCallback);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Load system ability %{public}d failed with %{public}d", CLI_TOOL_MGR_SERVICE_ID,
            ret);
        return false;
    }

    {
        std::unique_lock<std::mutex> lock(loadSaMutex_);
        auto waitStatus = loadSaCondation_.wait_for(lock, std::chrono::milliseconds(LOAD_SA_TIMEOUT_MS),
            [this]() {
                return loadSaFinished_;
            });
        if (!waitStatus) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Wait for load sa timeout");
            return false;
        }
    }

    return true;
}

void CliToolMGRClient::SetCliToolMgr(const sptr<IRemoteObject> &remoteObject)
{
    std::lock_guard<std::mutex> lock(proxyMutex_);
    cliToolMgr_ = iface_cast<ICliToolManager>(remoteObject);
}

sptr<ICliToolManager> CliToolMGRClient::GetCliToolMgr()
{
    std::lock_guard<std::mutex> lock(proxyMutex_);
    return cliToolMgr_;
}

void CliToolMGRClient::ClearProxy()
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "called");
    std::lock_guard<std::mutex> lock(proxyMutex_);
    cliToolMgr_ = nullptr;
    CliEventReplyManager::GetInstance().ClearAllEvent();
    CliSessionSubscriptionManager::GetInstance().ClearAllSubscriptions();
}

void CliToolMGRClient::CliMgrDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    if (callback_ != nullptr) {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "cli tool manager service died");
        callback_(remote);
    }
}

ErrCode CliToolMGRClient::SubscribeSession(const std::string &sessionId,
                                           const std::shared_ptr<SessionEventCallback> &callback,
                                           std::string &subscriptionId)
{
    ErrCode ret = EnsureSchedulerStubCreated();
    if (ret != ERR_OK) {
        return ret;
    }

    auto proxy = GetCliToolMgrProxy();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "connect failed");
        return GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }

    subscriptionId = CliSessionSubscriptionManager::GetInstance().AddProvisionalSubscription(sessionId,
        [callback](const std::string &sessionId,
            const std::string &subscriptionId, const CliToolEvent &event) {
            if (callback) {
                callback->OnToolEvent(sessionId, subscriptionId, event);
            }
        });
    ret = proxy->SubscribeSession(sessionId, subscriptionId, schedulerStub_);
    if (ret != ERR_OK) {
        CliSessionSubscriptionManager::GetInstance().RemoveSubscription(subscriptionId);
        subscriptionId = "";
    } else {
        CliSessionSubscriptionManager::GetInstance().ActivateSubscription(subscriptionId);
    }
    return ret;
}

ErrCode CliToolMGRClient::UnsubscribeSession(const std::string &sessionId, const std::string &subscriptionId)
{
    auto proxy = GetCliToolMgrProxy();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "connect failed");
        return GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    CliSessionSubscriptionManager::GetInstance().RemoveSubscription(subscriptionId);

    return proxy->UnsubscribeSession(sessionId, subscriptionId);
}

ErrCode CliToolMGRClient::ClearSession(const std::string &sessionId)
{
    auto proxy = GetCliToolMgrProxy();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "connect failed");
        return GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    return proxy->ClearSession(sessionId);
}

ErrCode CliToolMGRClient::QuerySession(const std::string &sessionId, CliSessionInfo &session)
{
    auto proxy = GetCliToolMgrProxy();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "connect failed");
        return GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    return proxy->QuerySession(sessionId, session);
}

ErrCode CliToolMGRClient::SendMessage(const std::string &sessionId, const std::string &inputText,
    const EventReplyCallback &callback)
{
    ErrCode ret = EnsureSchedulerStubCreated();
    if (ret != ERR_OK) {
        return ret;
    }

    auto proxy = GetCliToolMgrProxy();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "connect failed");
        return GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }

    std::string eventId = CliEventReplyManager::GetInstance().AddEventReplyCallback(sessionId,
        [cb = std::move(callback)](const CliEventReplyResult &result) {
            if (cb) {
                cb(result.code);
            }
        });
    ret = proxy->SendMessage(sessionId, inputText, eventId, schedulerStub_);
    if (ret != ERR_OK) {
        CliEventReplyManager::GetInstance().RemoveEventReplyCallback(eventId);
    } else {
        CliEventReplyManager::GetInstance().ActivateEventReplyCallback(eventId);
    }
    return ret;
}
} // namespace CliTool
} // namespace OHOS
