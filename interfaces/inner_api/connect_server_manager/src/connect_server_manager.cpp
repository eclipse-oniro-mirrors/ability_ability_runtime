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

#include "connect_server_manager.h"

#include <dlfcn.h>
#include <unistd.h>

#include "hilog_tag_wrapper.h"

namespace OHOS::AbilityRuntime {
namespace {
std::string GetInstanceMapMessage(
    const std::string& messageType, int32_t instanceId, const std::string& instanceName, int32_t tid)
{
    std::string message;
    message.append("{\"type\":\"");
    message.append(messageType);
    message.append("\",\"instanceId\":");
    message.append(std::to_string(instanceId));
    message.append(",\"name\":\"");
    message.append(instanceName);
    message.append("\",\"tid\":");
    message.append(std::to_string(tid));
    message.append(",\"apiType\":\"");
    message.append("stageMode\"");
    message.append(",\"language\":\"");
    message.append("ets\"");
    message.append("}");
    return message;
}
}

using StartServer = void (*)(const std::string&);
using StartServerForSocketPair = bool (*)(int);
using SendMessage = void (*)(const std::string&);
using StopServer = void (*)(const std::string&);
using StoreMessage = void (*)(int32_t, const std::string&);
using SetProfilerCallback = void (*)(const std::function<void(bool)> &setStateProfilerStatus);
using SetSwitchCallBack = void (*)(const std::function<void(int32_t)> &createLayoutInfo, int32_t instanceId);
using SetConnectCallback = void (*)(const std::function<void(bool)>);
using RemoveMessage = void (*)(int32_t);
using WaitForConnection = bool (*)();
using SetRecordCallBack = void (*)(const std::function<void(void)> &startRecordFunc,
    const std::function<void(void)> &stopRecordFunc);

std::mutex g_debuggerMutex;
std::mutex g_loadsoMutex;
std::mutex ConnectServerManager::instanceMutex_;
std::mutex ConnectServerManager::connectServerCallbackMutex_;
std::mutex ConnectServerManager::addInstanceCallbackMutex_;
std::mutex ConnectServerManager::sendInstanceMessageCallbackMutex_;

ConnectServerManager::~ConnectServerManager()
{
    StopConnectServer();
}

ConnectServerManager& ConnectServerManager::Get()
{
    std::lock_guard<std::mutex> lock(instanceMutex_);
    static ConnectServerManager connectServerManager;
    return connectServerManager;
}

void ConnectServerManager::LoadConnectServerDebuggerSo()
{
    std::lock_guard<std::mutex> lock(g_loadsoMutex);
    if (handlerConnectServerSo_ == nullptr) {
        handlerConnectServerSo_ = dlopen("libark_connect_inspector.z.so", RTLD_LAZY);
        if (handlerConnectServerSo_ == nullptr) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "null handlerConnectServerSo_");
            return;
        }
    }
}

void ConnectServerManager::StartConnectServer(const std::string& bundleName, int socketFd, bool isLocalAbstract)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "called");

    LoadConnectServerDebuggerSo();
    bundleName_ = bundleName;
    if (isLocalAbstract) {
        auto startServer = reinterpret_cast<StartServer>(dlsym(handlerConnectServerSo_, "StartServer"));
        if (startServer == nullptr) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "null startServer");
            return;
        }
        startServer(bundleName_);
        return;
    }
    auto startServerForSocketPair =
        reinterpret_cast<StartServerForSocketPair>(dlsym(handlerConnectServerSo_, "StartServerForSocketPair"));
    if (startServerForSocketPair == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null startServerForSocketPair");
        return;
    }
    startServerForSocketPair(socketFd);

    std::lock_guard<std::mutex> lock(connectServerCallbackMutex_);
    for (const auto &callback : connectServerCallbacks_) {
        if (callback != nullptr) {
            callback();
        }
    }
}

void ConnectServerManager::StopConnectServer(bool isCloseSo)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "called");
    if (handlerConnectServerSo_ == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null handlerConnectServerSo_");
        return;
    }
    auto stopServer = reinterpret_cast<StopServer>(dlsym(handlerConnectServerSo_, "StopServer"));
    if (stopServer != nullptr) {
        stopServer(bundleName_);
    } else {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null StopServer");
    }
    if (isCloseSo) {
        dlclose(handlerConnectServerSo_);
        handlerConnectServerSo_ = nullptr;
    }
}

bool ConnectServerManager::StoreInstanceMessage(int32_t tid, int32_t instanceId, const std::string& instanceName)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto result = instanceMap_.try_emplace(instanceId, std::make_pair(instanceName, tid));
        if (!result.second) {
            TAG_LOGW(AAFwkTag::JSRUNTIME, "Instance %{public}d added", instanceId);
            return false;
        }
    }
    return true;
}

void ConnectServerManager::StoreDebuggerInfo(int32_t tid, void* vm, const panda::JSNApi::DebugOption& debugOption,
    const DebuggerPostTask& debuggerPostTask, bool isDebugApp)
{
    std::lock_guard<std::mutex> lock(g_debuggerMutex);
    if (g_debuggerInfo.find(tid) == g_debuggerInfo.end()) {
        g_debuggerInfo.emplace(tid, std::make_pair(vm, debuggerPostTask));
    }

    if (!isConnected_) {
        TAG_LOGW(AAFwkTag::JSRUNTIME, "not Connected");
        return;
    }

    panda::JSNApi::StoreDebugInfo(tid, reinterpret_cast<panda::EcmaVM*>(vm), debugOption, debuggerPostTask, isDebugApp);
}

void ConnectServerManager::SendDebuggerInfo(bool needBreakPoint, bool isDebugApp)
{
    ConnectServerManager::Get().SetConnectedCallback();
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& instance : instanceMap_) {
        auto instanceId = instance.first;
        auto instanceName = instance.second.first;
        auto tid = instance.second.second;

        panda::EcmaVM* vm = reinterpret_cast<panda::EcmaVM*>(g_debuggerInfo[tid].first);
        std::lock_guard<std::mutex> lock(g_debuggerMutex);
        const auto &debuggerPostTask = g_debuggerInfo[tid].second;
        if (!debuggerPostTask) {
            continue;
        }
        ConnectServerManager::Get().SendInstanceMessage(tid, instanceId, instanceName);
        panda::JSNApi::DebugOption debugOption = {ARK_DEBUGGER_LIB_PATH, isDebugApp ? needBreakPoint : false};
        panda::JSNApi::StoreDebugInfo(tid, vm, debugOption, debuggerPostTask, isDebugApp);
    }
}

void ConnectServerManager::SetConnectedCallback()
{
    LoadConnectServerDebuggerSo();

    auto setConnectCallBack = reinterpret_cast<SetConnectCallback>(
        dlsym(handlerConnectServerSo_, "SetConnectCallback"));
    if (setConnectCallBack == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null setConnectCallBack");
        return;
    }

    setConnectCallBack([](bool isConnected) {
        ConnectServerManager::Get().isConnected_ = isConnected;
    });
}

void ConnectServerManager::SetSwitchCallback(const std::function<void(int32_t)> &createLayoutInfo, int32_t instanceId)
{
    LoadConnectServerDebuggerSo();
    auto setSwitchCallBack = reinterpret_cast<SetSwitchCallBack>(
        dlsym(handlerConnectServerSo_, "SetSwitchCallBack"));
    if (setSwitchCallBack == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null setSwitchCallBack");
        return;
    }
    setSwitchCallBack(createLayoutInfo, instanceId);
}

void ConnectServerManager::SetProfilerCallBack(const std::function<void(bool)> &setStateProfilerStatus)
{
    LoadConnectServerDebuggerSo();
    auto setProfilerCallback = reinterpret_cast<SetProfilerCallback>(
        dlsym(handlerConnectServerSo_, "SetProfilerCallback"));
    if (setProfilerCallback == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null setProfilerCallback");
        return;
    }
    setProfilerCallback(setStateProfilerStatus);
}

bool ConnectServerManager::SendInstanceMessage(int32_t tid, int32_t instanceId, const std::string& instanceName)
{
    TAG_LOGI(AAFwkTag::JSRUNTIME, "called");
    ConnectServerManager::Get().SendInstanceMessageCallback(instanceId);
    std::string message = GetInstanceMapMessage("addInstance", instanceId, instanceName, tid);
    LoadConnectServerDebuggerSo();
    auto storeMessage = reinterpret_cast<StoreMessage>(dlsym(handlerConnectServerSo_, "StoreMessage"));
    if (storeMessage == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null storeMessage");
        return false;
    }
    storeMessage(instanceId, message);
    return true;
}

bool ConnectServerManager::AddInstance(int32_t tid, int32_t instanceId, const std::string& instanceName)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto result = instanceMap_.try_emplace(instanceId, std::make_pair(instanceName, tid));
        if (!result.second) {
            TAG_LOGW(AAFwkTag::JSRUNTIME, "instance %{public}d added", instanceId);
            return false;
        }
    }

    if (!isConnected_) {
        TAG_LOGW(AAFwkTag::JSRUNTIME, "not Connected");
        return false;
    }

    TAG_LOGD(AAFwkTag::JSRUNTIME, "called");

    ConnectServerManager::Get().AddInstanceCallback(instanceId);
    LoadConnectServerDebuggerSo();
    // Get the message including information of new instance, which will be send to IDE.
    std::string message = GetInstanceMapMessage("addInstance", instanceId, instanceName, tid);

    auto storeMessage = reinterpret_cast<StoreMessage>(dlsym(handlerConnectServerSo_, "StoreMessage"));
    if (storeMessage == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null StoreMessage");
        return false;
    }
    storeMessage(instanceId, message);

    // WaitForConnection() means the connection state of the connect server
    auto sendMessage =
        reinterpret_cast<OHOS::AbilityRuntime::SendMessage>(dlsym(handlerConnectServerSo_, "SendMessage"));
    if (sendMessage == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null SendMessage");
        return false;
    }
    // if connected, message will be sent immediately.
    sendMessage(message);
    return true;
}

void ConnectServerManager::RemoveInstance(int32_t instanceId)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "called");
    std::string instanceName;
    int32_t tid;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = instanceMap_.find(instanceId);
        if (it == instanceMap_.end()) {
            TAG_LOGW(AAFwkTag::JSRUNTIME, "Instance %{public}d not found", instanceId);
            return;
        }

        instanceName = std::move(it->second.first);
        tid = std::move(it->second.second);
        instanceMap_.erase(it);
    }
    {
        std::lock_guard<std::mutex> lock(g_debuggerMutex);
        g_debuggerInfo.erase(tid);
    }
    
    if (!isConnected_) {
        TAG_LOGW(AAFwkTag::JSRUNTIME, "not Connected");
        return;
    }

    LoadConnectServerDebuggerSo();
    auto waitForConnection = reinterpret_cast<WaitForConnection>(dlsym(handlerConnectServerSo_, "WaitForConnection"));
    if (waitForConnection == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null WaitForConnection");
        return;
    }

    // Get the message including information of deleted instance, which will be send to IDE.
    std::string message = GetInstanceMapMessage("destroyInstance", instanceId, instanceName, tid);

    auto removeMessage = reinterpret_cast<RemoveMessage>(dlsym(handlerConnectServerSo_, "RemoveMessage"));
    if (removeMessage == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null RemoveMessage");
        return;
    }
    removeMessage(instanceId);

    if (waitForConnection()) {
        return;
    }

    auto sendMessage =
        reinterpret_cast<OHOS::AbilityRuntime::SendMessage>(dlsym(handlerConnectServerSo_, "SendMessage"));
    if (sendMessage == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null sendMessage");
        return;
    }
    sendMessage(message);
}

void ConnectServerManager::SendInspector(const std::string& jsonTreeStr, const std::string& jsonSnapshotStr)
{
    TAG_LOGI(AAFwkTag::JSRUNTIME, "called");
    auto sendMessage =
        reinterpret_cast<OHOS::AbilityRuntime::SendMessage>(dlsym(handlerConnectServerSo_, "SendMessage"));
    if (sendMessage == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null sendMessage");
        return;
    }
    sendMessage(jsonTreeStr);
    sendMessage(jsonSnapshotStr);
}

void ConnectServerManager::SendMessage(const std::string &message)
{
    TAG_LOGI(AAFwkTag::JSRUNTIME, "called");
    auto sendMessage =
        reinterpret_cast<OHOS::AbilityRuntime::SendMessage>(dlsym(handlerConnectServerSo_, "SendMessage"));
    if (sendMessage == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null sendMessage");
        return;
    }

    sendMessage(message);
}

bool ConnectServerManager::SetRecordCallback(const std::function<void(void)> &startRecordFunc,
    const std::function<void(void)> &stopRecordFunc)
{
    if (handlerConnectServerSo_ == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "No connected server");
        return false;
    }
    auto setRecordCallback = reinterpret_cast<SetRecordCallBack>(dlsym(handlerConnectServerSo_, "SetRecordCallback"));
    if (setRecordCallback == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null setRecordCallback");
        return false;
    }
    setRecordCallback(startRecordFunc, stopRecordFunc);
    return true;
}

void ConnectServerManager::SetRecordResults(const std::string &jsonArrayStr)
{
    if (handlerConnectServerSo_ == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "No connected server");
        return;
    }
    auto sendMessage =
        reinterpret_cast<OHOS::AbilityRuntime::SendMessage>(dlsym(handlerConnectServerSo_, "SendMessage"));
    if (sendMessage == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null sendMessage");
        return;
    }
    sendMessage(jsonArrayStr);
}

void ConnectServerManager::RegisterConnectServerCallback(const ServerConnectCallback &connectServerCallback)
{
    if (connectServerCallback == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null callback");
        return;
    }
    std::lock_guard<std::mutex> lock(connectServerCallbackMutex_);
    for (const auto &callback : connectServerCallbacks_) {
        if (callback == connectServerCallback) {
            TAG_LOGW(AAFwkTag::JSRUNTIME, "callback exist");
            return;
        }
    }
    connectServerCallbacks_.emplace_back(connectServerCallback);
    TAG_LOGD(AAFwkTag::JSRUNTIME, "register connectServerCallback succeed");
}

void ConnectServerManager::RegisterSendInstanceMessageCallback(
    const SendInstanceMessageCallBack &sendInstanceMessageCallback)
{
    if (sendInstanceMessageCallback == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null callback");
        return;
    }
    std::lock_guard<std::mutex> lock(sendInstanceMessageCallbackMutex_);
    for (const auto &callback : sendInstanceMessageCallbacks_) {
        if (callback == sendInstanceMessageCallback) {
            TAG_LOGW(AAFwkTag::JSRUNTIME, "callback exist");
            return;
        }
    }
    sendInstanceMessageCallbacks_.emplace_back(sendInstanceMessageCallback);
    TAG_LOGD(AAFwkTag::JSRUNTIME, "register sendInstanceMessageCallback succeed");
}

void ConnectServerManager::SendInstanceMessageCallback(const int32_t instanceId)
{
    std::lock_guard<std::mutex> lock(sendInstanceMessageCallbackMutex_);
    for (const auto &callback : sendInstanceMessageCallbacks_) {
        if (callback != nullptr) {
            callback(instanceId);
        }
    }
}

void ConnectServerManager::RegisterAddInstanceCallback(const AddInstanceCallBack &addInstanceCallback)
{
    if (addInstanceCallback == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null callback");
        return;
    }
    std::lock_guard<std::mutex> lock(addInstanceCallbackMutex_);
    for (const auto &callback : addInstanceCallbacks_) {
        if (callback == addInstanceCallback) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "callback exist");
            return;
        }
    }
    addInstanceCallbacks_.emplace_back(addInstanceCallback);
    TAG_LOGD(AAFwkTag::JSRUNTIME, "register addInstanceCallback succeed");
}

void ConnectServerManager::AddInstanceCallback(const int32_t instanceId)
{
    std::lock_guard<std::mutex> lock(addInstanceCallbackMutex_);
    for (const auto &callback : addInstanceCallbacks_) {
        if (callback != nullptr) {
            callback(instanceId);
        }
    }
}

void ConnectServerManager::SendInstanceMessageAll(std::function<void(int32_t, const DebuggerPostTask&)> callback)
{
    ConnectServerManager::Get().SetConnectedCallback();
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& instance : instanceMap_) {
        auto instanceId = instance.first;
        auto instanceName = instance.second.first;
        auto tid = instance.second.second;
        std::lock_guard<std::mutex> lock(g_debuggerMutex);
        ConnectServerManager::Get().SendInstanceMessage(tid, instanceId, instanceName);
        const auto &debuggerPostTask = g_debuggerInfo[tid].second;
        if (!debuggerPostTask) {
            continue;
        }
        if (callback != nullptr) {
            callback(tid, debuggerPostTask);
        }
    }
}
} // namespace OHOS::AbilityRuntime