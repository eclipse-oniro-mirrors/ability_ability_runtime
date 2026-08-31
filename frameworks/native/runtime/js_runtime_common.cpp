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

#include "js_runtime_common.h"

#include "connect_server_manager.h"
#include "hilog_tag_wrapper.h"
#include "js_environment.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr char ARK_DEBUGGER_LIB_PATH[] = "libark_inspector.z.so";
}
JsRuntimeCommon& JsRuntimeCommon::GetInstance()
{
    static JsRuntimeCommon JsRuntimeCommon;
    return JsRuntimeCommon;
}

JsRuntimeCommon::JsRuntimeCommon() {}

JsRuntimeCommon::~JsRuntimeCommon() {}

bool JsRuntimeCommon::IsDebugMode()
{
    return debugMode_.load();
}

bool JsRuntimeCommon::IsDebugApp()
{
    return debugApp_.load();
}

bool JsRuntimeCommon::IsNativeStart()
{
    return nativeStart_.load();
}

void JsRuntimeCommon::SetDebugMode(bool isDebugMode)
{
    debugMode_.store(isDebugMode);
}

void JsRuntimeCommon::SetDebugApp(bool isDebugApp)
{
    debugApp_.store(isDebugApp);
}

void JsRuntimeCommon::SetNativeStart(bool isNativeStart)
{
    nativeStart_.store(isNativeStart);
}

void JsRuntimeCommon::StartDebuggerModule(bool isDebugApp, bool isNativeStart)
{
    debugMode_.store(true);
    debugApp_.store(isDebugApp);
    nativeStart_.store(isNativeStart);
}

napi_status JsRuntimeCommon::StartDebugMode(NativeEngine* nativeEngine, const std::string& threadName)
{
    if (nativeEngine == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null nativeEngine");
        return napi_status::napi_invalid_arg;
    }
    TAG_LOGI(AAFwkTag::JSRUNTIME, "debug mode is %{public}d, debug app is %{public}d", IsDebugMode(), IsDebugApp());
    auto arkNativeEngine = static_cast<NativeEngine*>(nativeEngine);
    auto instanceId = panda::DFXJSNApi::GetCurrentThreadId();
    TAG_LOGI(AAFwkTag::JSRUNTIME, "Create instanceId is %{public}d", instanceId);
    std::string instanceName = threadName + "_" + std::to_string(instanceId);
    bool isAddInstance = ConnectServerManager::Get().AddInstance(instanceId, instanceId, instanceName);
    if (IsNativeStart()) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "native: true, set isAddInstance: false");
        isAddInstance = false;
    }
    auto postTask = [nativeEngine](std::function<void()>&& callback) {
        nativeEngine->CallDebuggerPostTaskFunc(std::move(callback));
    };
    panda::JSNApi::DebugOption debugOption = {ARK_DEBUGGER_LIB_PATH, isAddInstance};
    auto vm = const_cast<EcmaVM*>(arkNativeEngine->GetEcmaVm());
    ConnectServerManager::Get().StoreDebuggerInfo(
        instanceId, reinterpret_cast<void*>(vm), debugOption, postTask, IsDebugApp());
    panda::JSNApi::NotifyDebugMode(instanceId, vm, debugOption, instanceId, postTask, IsDebugApp());
    return napi_status::napi_ok;
}

napi_status JsRuntimeCommon::StopDebugMode(NativeEngine* nativeEngine)
{
    if (nativeEngine == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null nativeEngine");
        return napi_status::napi_invalid_arg;
    }
    auto instanceId = panda::DFXJSNApi::GetCurrentThreadId();
    TAG_LOGI(AAFwkTag::JSRUNTIME, "destroy instanceId is %{public}d", instanceId);
    ConnectServerManager::Get().RemoveInstance(instanceId);
    auto arkNativeEngine = static_cast<NativeEngine*>(nativeEngine);
    auto vm = const_cast<EcmaVM*>(arkNativeEngine->GetEcmaVm());
    panda::JSNApi::StopDebugger(vm);
    return napi_status::napi_ok;
}
} // namespace AbilityRuntime
} // namespace OHOS
