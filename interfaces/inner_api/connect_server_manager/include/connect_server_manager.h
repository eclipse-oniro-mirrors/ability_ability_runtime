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

#ifndef OHOS_ABILITY_RUNTIME_CONNECT_SERVER_MANAGER_H
#define OHOS_ABILITY_RUNTIME_CONNECT_SERVER_MANAGER_H

#include <mutex>
#include <unordered_map>
#include "jsnapi.h"
using DebuggerPostTask = std::function<void(std::function<void()>&&)>;
using DebuggerInfo = std::unordered_map<int, std::pair<void*, const DebuggerPostTask>>;
using InstanceMap = std::unordered_map<int32_t, std::string>;
using ServerConnectCallback = void(*)(void);
using AddInstanceCallBack = void(*)(int32_t);
using SendInstanceMessageCallBack = void(*)(int32_t);
#ifdef APP_USE_ARM
constexpr char ARK_DEBUGGER_LIB_PATH[] = "libark_inspector.z.so";
#elif defined(APP_USE_X86_64)
constexpr char ARK_DEBUGGER_LIB_PATH[] = "libark_inspector.z.so";
#else
constexpr char ARK_DEBUGGER_LIB_PATH[] = "libark_inspector.z.so";
#endif
namespace OHOS::AbilityRuntime {
class ConnectServerManager final {
public:
    static ConnectServerManager& Get();

    void StartConnectServer(const std::string& bundleName, int socketFd, bool isLocalAbstract);
    void StopConnectServer(bool isCloseSo = true);
    bool AddInstance(int32_t tid, int32_t instanceId, const std::string& instanceName = "PandaDebugger");
    void RemoveInstance(int32_t instanceId);
    void SendInspector(const std::string& jsonTreeStr, const std::string& jsonSnapshotStr);
    void SendMessage(const std::string &message);
    bool StoreInstanceMessage(
        int32_t tid, int32_t instanceId, const std::string& instanceName = "PandaDebugger");
    void StoreDebuggerInfo(int32_t tid, void* vm, const panda::JSNApi::DebugOption& debugOption,
        const DebuggerPostTask& debuggerPostTask, bool isDebugApp);
    void SetSwitchCallback(const std::function<void(int32_t)> &createLayoutInfo, int32_t instanceId);
    void SetProfilerCallBack(const std::function<void(bool)> &setStateProfilerStatus);
    void SetConnectedCallback();
    bool SendInstanceMessage(int32_t tid, int32_t instanceId, const std::string& instanceName);
    void SendDebuggerInfo(bool needBreakPoint, bool isDebugApp);
    void SendInstanceMessageAll(std::function<void(int32_t, const DebuggerPostTask&)> callback);
    void LoadConnectServerDebuggerSo();
    bool SetRecordCallback(const std::function<void(void)> &startRecordFunc,
        const std::function<void(void)> &stopRecordFunc);
    void SetRecordResults(const std::string &jsonArrayStr);
    void RegisterConnectServerCallback(const ServerConnectCallback &connectServerCallback);
    void RegisterSendInstanceMessageCallback(const SendInstanceMessageCallBack &sendInstanceMessageCallback);
    void RegisterAddInstanceCallback(const AddInstanceCallBack &addInstanceCallback);

private:
    ConnectServerManager() = default;
    ~ConnectServerManager();

    void* handlerConnectServerSo_ = nullptr;
    std::string bundleName_;

    std::mutex mutex_;
    static std::mutex instanceMutex_;
    static std::mutex connectServerCallbackMutex_;
    static std::mutex addInstanceCallbackMutex_;
    static std::mutex sendInstanceMessageCallbackMutex_;
    std::atomic<bool> isConnected_ = false;
    std::unordered_map<int32_t, std::pair<std::string, int32_t>> instanceMap_;
    std::unordered_map<int, std::pair<void*, const DebuggerPostTask>> g_debuggerInfo;
    std::vector<ServerConnectCallback> connectServerCallbacks_;

    std::vector<AddInstanceCallBack> addInstanceCallbacks_;
    std::vector<SendInstanceMessageCallBack> sendInstanceMessageCallbacks_;
    ConnectServerManager(const ConnectServerManager&) = delete;
    ConnectServerManager(ConnectServerManager&&) = delete;
    ConnectServerManager& operator=(const ConnectServerManager&) = delete;
    ConnectServerManager& operator=(ConnectServerManager&&) = delete;

    void SendInstanceMessageCallback(const int32_t instanceId);
    void AddInstanceCallback(const int32_t instanceId);
};
} // namespace OHOS::AbilityRuntime

#endif // OHOS_ABILITY_RUNTIME_CONNECT_SERVER_MANAGER_H
