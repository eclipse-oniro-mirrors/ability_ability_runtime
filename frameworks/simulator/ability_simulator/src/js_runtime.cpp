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

#include "js_runtime.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
JsRuntime::JsRuntime()
{
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "called");
}

JsRuntime::~JsRuntime()
{
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "called");
}

void JsRuntime::DumpHeapSnapshot(bool isPrivate)
{}

void JsRuntime::DumpCpuProfile()
{}

void JsRuntime::DestroyHeapProfiler()
{}

void JsRuntime::ForceFullGC()
{}

void JsRuntime::ForceFullGC(uint32_t tid)
{}

void JsRuntime::DumpHeapSnapshot(uint32_t tid, bool isFullGC, bool isBinary)
{}

void JsRuntime::DumpHeapSnapshot(uint32_t tid, bool isFullGC, bool isBinary, bool isClearNodeIdCache)
{}

void JsRuntime::AllowCrossThreadExecution()
{}

void JsRuntime::GetHeapPrepare()
{}

void JsRuntime::NotifyApplicationState(bool isBackground)
{}

bool JsRuntime::SuspendVM(uint32_t tid)
{
    return false;
}

void JsRuntime::ResumeVM(uint32_t tid)
{}

void JsRuntime::PreloadSystemModule(const std::string &moduleName)
{}

void JsRuntime::PreloadMainAbility(const std::string &moduleName, const std::string &srcPath,
    const std::string &hapPath, bool isEsMode, const std::string &srcEntrance)
{}

void JsRuntime::PreloadModule(const std::string &moduleName, const std::string &srcPath, const std::string &hapPath,
    bool isEsMode, bool useCommonTrunk)
{}

void JsRuntime::StartDebugMode(const DebugOption debugOption)
{}

void JsRuntime::SetDebugOption(const DebugOption debugOption)
{}

void JsRuntime::StartLocalDebugMode(bool isDebugFromLocal)
{}

bool JsRuntime::LoadRepairPatch(const std::string &hqfFile, const std::string &hapPath)
{
    return false;
}

bool JsRuntime::UnLoadRepairPatch(const std::string &hqfFile)
{
    return false;
}

bool JsRuntime::NotifyHotReloadPage()
{
    return false;
}

void JsRuntime::RegisterQuickFixQueryFunc(const std::map<std::string, std::string> &moduleAndPath)
{}

void JsRuntime::StartProfiler(const DebugOption debugOption)
{}

void JsRuntime::SetModuleLoadChecker(const std::shared_ptr<ModuleCheckerDelegate> moduleCheckerDelegate) const
{}

void JsRuntime::SetDeviceDisconnectCallback(const std::function<bool()> &cb)
{}

void JsRuntime::SetStopPreloadSoCallback(const std::function<void()> &callback)
{}

void JsRuntime::FinishPreload()
{}

std::unique_ptr<JsRuntime> JsRuntime::Create(const Options &options)
{
    return std::unique_ptr<JsRuntime>();
}

std::unique_ptr<NativeReference> JsRuntime::LoadSystemModuleByEngine(
    napi_env env, const std::string &moduleName, const napi_value *argv, size_t argc)
{
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "LoadSystemModule(%{public}s)", moduleName.c_str());
    if (env == nullptr) {
        TAG_LOGD(AAFwkTag::ABILITY_SIM, "invalid env");
        return std::unique_ptr<NativeReference>();
    }

    napi_value globalObj = nullptr;
    napi_get_global(env, &globalObj);
    std::unique_ptr<NativeReference> methodRequireNapiRef_;
    napi_value ref = nullptr;
    napi_get_named_property(env, globalObj, "requireNapi", &ref);
    napi_ref methodRequireNapiRef = nullptr;
    napi_create_reference(env, ref, 1, &methodRequireNapiRef);
    methodRequireNapiRef_.reset(reinterpret_cast<NativeReference *>(methodRequireNapiRef));
    if (!methodRequireNapiRef_) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "create reference failed");
        return nullptr;
    }
    napi_value className = nullptr;
    napi_create_string_utf8(env, moduleName.c_str(), moduleName.length(), &className);
    napi_value classValue = nullptr;
    napi_call_function(env, globalObj, methodRequireNapiRef_->GetNapiValue(), 1, &className, &classValue);
    napi_value instanceValue = nullptr;
    napi_new_instance(env, classValue, argc, argv, &instanceValue);
    if (instanceValue == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "create object instance failed");
        return std::unique_ptr<NativeReference>();
    }

    napi_ref result = nullptr;
    napi_create_reference(env, instanceValue, 1, &result);
    return std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference *>(result));
}
} // namespace AbilityRuntime
} // namespace OHOS
