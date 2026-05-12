/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "native_runtime.h"

#include <unistd.h>

#include "hilog_tag_wrapper.h"
#include "module_manager/native_module_manager.h"
#include "connect_server_manager.h"
#include "hdc_register.h"
#include "parameters.h"
#include "constants.h"
#include "runtime.h"
#include "bundle_constants.h"

namespace OHOS {
namespace AbilityRuntime {
const std::string DEFAULT_NAMESPACE = "default";
const char *OH_ABILITY_RUNTIME_ON_NATIVE_EXTENSION_CREATE = "OH_AbilityRuntime_OnNativeExtensionCreate";
using CreateFuncType = void(*)(AbilityRuntime_ExtensionInstanceHandle, const char*);

void DebuggerConnectionHandler(const std::string &bundleName)
{
    int32_t instanceId = static_cast<int32_t>(getproctid());
    int32_t tid = instanceId;
    if (!ConnectServerManager::Get().StoreInstanceMessage(tid, instanceId, bundleName)) {
        ConnectServerManager::Get().RemoveInstance(instanceId);
        ConnectServerManager::Get().StoreInstanceMessage(tid, instanceId, bundleName);
    }
    ConnectServerManager::Get().SendInstanceMessage(tid, instanceId, bundleName);
}

bool NativeRuntime::LoadModule(const std::string& bundleModuleName, const std::string& fileName,
    const std::string& abilityName, AbilityRuntime_ExtensionInstance& instance)
{
    auto moduleManager = NativeModuleManager::GetInstance();
    if (moduleManager == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null moduleManager");
        return false;
    }
    std::string namespaceName;
    if (!moduleManager->GetLdNamespaceName(bundleModuleName, namespaceName)) {
        if (!moduleManager->GetLdNamespaceName(DEFAULT_NAMESPACE, namespaceName)) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "GetLdNamespaceName failed");
            return false;
        }
    }
    Dl_namespace ns;
    if (dlns_get(namespaceName.data(), &ns) != 0) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "namespaceName not found: %{public}s", namespaceName.c_str());
        return false;
    }
    LIBHANDLE nativeHandle = nullptr;
    nativeHandle = dlopen_ns(&ns, fileName.c_str(), RTLD_LAZY);
    if (nativeHandle == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "dlopen failed: %{public}s", fileName.c_str());
        return false;
    }
    auto symbol = LIBSYM(nativeHandle, OH_ABILITY_RUNTIME_ON_NATIVE_EXTENSION_CREATE);
    if (!symbol) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "dlsym failed");
        LIBFREE(nativeHandle);
        return false;
    }
    auto func = reinterpret_cast<CreateFuncType>(symbol);
    AbilityRuntime_ExtensionInstanceHandle handle = &instance;
    func(handle, abilityName.c_str());
    return true;
}

void NativeRuntime::StartDebugMode(const Runtime::DebugOption &dOption, const std::string &bundleName)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "localDebug %{public}d, isDebugApp %{public}d, bundleName %{public}s",
        dOption.isDebugFromLocal, dOption.isDebugApp, bundleName.c_str());
    if (!dOption.isDebugFromLocal && !dOption.isDeveloperMode) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "developer Mode false");
        return;
    }

    bool isStartWithDebug = dOption.isStartWithDebug;
    TAG_LOGD(AAFwkTag::JSRUNTIME, "Native is starting debug mode [%{public}s]", isStartWithDebug ? "break" : "normal");
    bool isDebugApp = dOption.isDebugApp;
    std::string appProvisionType = dOption.appProvisionType;
    std::string inputProcessName = bundleName != dOption.processName ? dOption.processName : "";
    HdcRegister::DebugRegisterMode debugMode = HdcRegister::DebugRegisterMode::HDC_DEBUG_REG;
    if (dOption.isDebugFromLocal && dOption.isDeveloperMode) {
        debugMode = HdcRegister::DebugRegisterMode::BOTH_REG;
    } else if (dOption.isDebugFromLocal) {
        debugMode = HdcRegister::DebugRegisterMode::LOCAL_DEBUG_REG;
    }

    TAG_LOGD(AAFwkTag::JSRUNTIME, "inputProcessName %{public}s, debugMode:%{public}d, appProvisionType:%{public}s",
        inputProcessName.c_str(), static_cast<int>(debugMode), appProvisionType.c_str());
    HdcRegister::Get().StartHdcRegister(bundleName, inputProcessName, isDebugApp, debugMode,
        [bundleName, isStartWithDebug, isDebugApp, appProvisionType](int socketFd, std::string option) {
        TAG_LOGI(AAFwkTag::JSRUNTIME,
            "HdcRegister msg, fd %{public}d, option %{public}s, isStartWithDebug %{public}d, isDebugApp %{public}d",
            socketFd, option.c_str(), isStartWithDebug, isDebugApp);
        // system is unlocked when const.boot.oemmode is rd
        std::string oemmode = OHOS::system::GetParameter("const.boot.oemmode", "");
        bool unlocked = "rd" == oemmode;
        TAG_LOGI(AAFwkTag::JSRUNTIME, "unlocked= %{public}d, oemmode= %{public}s", unlocked, oemmode.c_str());
        // Don't start any server if (system is locked) and app is release version
        // Starting ConnectServer in release app on debuggable system is only for debug mode, not for profiling mode.
        if ((!unlocked) && appProvisionType == AppExecFwk::Constants::APP_PROVISION_TYPE_RELEASE) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "not support release app");
            return;
        }
        if (option.find(DEBUGGER) == std::string::npos) {
            TAG_LOGD(AAFwkTag::JSRUNTIME, "stop old connect server");
            // if has old connect server, stop it
            ConnectServerManager::Get().StopConnectServer(false);
            ConnectServerManager::Get().SendDebuggerInfo(isStartWithDebug, isDebugApp);
            ConnectServerManager::Get().StartConnectServer(bundleName, socketFd, false);
        } else {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "debugger service unexpected option: %{public}s", option.c_str());
        }
    });

    if (isDebugApp && appProvisionType != AppExecFwk::Constants::APP_PROVISION_TYPE_RELEASE) {
        TAG_LOGD(AAFwkTag::JSRUNTIME, "start connect server");
        ConnectServerManager::Get().StartConnectServer(bundleName, -1, true);
    }
    DebuggerConnectionHandler(bundleName);
    TAG_LOGD(AAFwkTag::JSRUNTIME, "StartDebugMode end");
}

void NativeRuntime::StopDebugMode()
{
    int32_t instanceId = static_cast<int32_t>(getproctid());
    ConnectServerManager::Get().RemoveInstance(instanceId);
    TAG_LOGD(AAFwkTag::JSRUNTIME, "StopDebugMode end, instanceId=%{public}d", instanceId);
}
} // namespace AbilityRuntime
} // namespace OHOS