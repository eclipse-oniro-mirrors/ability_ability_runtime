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
#ifdef APP_USE_ARM
constexpr char ARK_DEBUGGER_LIB_PATH[] = "/system/lib/platformsdk/libark_debugger.z.so";
#elif defined(APP_USE_X86_64)
constexpr char ARK_DEBUGGER_LIB_PATH[] = "/system/lib64/platformsdk/libark_debugger.z.so";
#else
constexpr char ARK_DEBUGGER_LIB_PATH[] = "/system/lib64/platformsdk/libark_debugger.z.so";
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
    void SendArkUIStateProfilerMessage(const std::string &message);
    void SetLayoutInspectorCallback(
        const std::function<void(int32_t)> &createLayoutInfo, const std::function<void(bool)> &setStatus);
    void SetStateProfilerCallback(const std::function<void(bool)> &setArkUIStateProfilerStatus);
    std::function<void(int32_t)> GetLayoutInspectorCallback();
    bool StoreInstanceMessage(
        int32_t tid, int32_t instanceId, const std::string& instanceName = "PandaDebugger");
    void StoreDebuggerInfo(int32_t tid, void* vm, const panda::JSNApi::DebugOption& debugOption,
        const DebuggerPostTask& debuggerPostTask, bool isDebugApp);
    void SetConnectedCallback();
    bool SendInstanceMessage(int32_t tid, int32_t instanceId, const std::string& instanceName);
    void SendDebuggerInfo(bool needBreakPoint, bool isDebugApp);
    void LoadConnectServerDebuggerSo();
    DebuggerPostTask GetDebuggerPostTask(int32_t tid);
    void SetSwitchCallback(int32_t instanceId);
    void SetProfilerCallBack();
    bool SetRecordCallback(const std::function<void(void)> &startRecordFunc,
        const std::function<void(void)> &stopRecordFunc);
    void SetRecordResults(const std::string &jsonArrayStr);

private:
    ConnectServerManager() = default;
    ~ConnectServerManager();

    void* handlerConnectServerSo_ = nullptr;
    std::string bundleName_;

    std::mutex mutex_;
    static std::mutex instanceMutex_;
    std::atomic<bool> isConnected_ = false;
    std::unordered_map<int32_t, std::pair<std::string, int32_t>> instanceMap_;
    std::function<void(int32_t)> createLayoutInfo_;
    std::function<void(int32_t)> setStatus_;
    std::function<void(int32_t)> setArkUIStateProfilerStatus_;
    ConnectServerManager(const ConnectServerManager&) = delete;
    ConnectServerManager(ConnectServerManager&&) = delete;
    ConnectServerManager& operator=(const ConnectServerManager&) = delete;
    ConnectServerManager& operator=(ConnectServerManager&&) = delete;
};
} // namespace OHOS::AbilityRuntime

#endif // OHOS_ABILITY_RUNTIME_CONNECT_SERVER_MANAGER_H
