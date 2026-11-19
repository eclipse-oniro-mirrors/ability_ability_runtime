/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#include "cj_ability_stage_context.h"
#include "ffi_remote_data.h"
#include "hap_module_info.h"
#include "ability_runtime/context/context.h"
#include "hilog_tag_wrapper.h"
#include <dlfcn.h>

namespace {
const char* CJ_ABILITY_LIBNAME = "libcj_ability_ffi.z.so";
const char* FUNC_CONVERT_CONFIGURATION = "OHOS_ConvertConfiguration";
const char* FUNC_CONVERT_CONFIGURATION_V2 = "OHOS_ConvertConfigurationV2";
const char* FUNC_FREE_CONFIGURATION_V2 = "OHOS_FreeConfigurationV2";
const char* CJ_BUNDLE_MGR_LIBNAME = "libcj_bundle_manager_ffi.z.so";
const char* FUNC_CONVERT_HAP_INFO = "OHOS_ConvertHapInfoV2";
}

namespace OHOS {
namespace AbilityRuntime {
CConfiguration CallConvertConfig(std::shared_ptr<AppExecFwk::Configuration> configuration)
{
    CConfiguration cCfg = {};
    void* handle = dlopen(CJ_ABILITY_LIBNAME, RTLD_LAZY);
    if (handle == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null handle");
        return cCfg;
    }
    using ConvertConfigFunc = CConfiguration (*)(void*);
    auto func = reinterpret_cast<ConvertConfigFunc>(dlsym(handle, FUNC_CONVERT_CONFIGURATION));
    if (func == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null func");
        dlclose(handle);
        return cCfg;
    }
    cCfg = func(configuration.get());
    dlclose(handle);
    return cCfg;
}

CConfigurationV2 CallConvertConfigV2(std::shared_ptr<AppExecFwk::Configuration> configuration)
{
    CConfigurationV2 cCfg = {};
    void* handle = dlopen(CJ_ABILITY_LIBNAME, RTLD_LAZY);
    if (handle == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null handle");
        return cCfg;
    }
    using ConvertConfigFuncV2 = CConfigurationV2 (*)(void*);
    auto func = reinterpret_cast<ConvertConfigFuncV2>(dlsym(handle, FUNC_CONVERT_CONFIGURATION_V2));
    if (func == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null func");
        dlclose(handle);
        return cCfg;
    }
    cCfg = func(configuration.get());
    dlclose(handle);
    return cCfg;
}

void CallFreeConfigV2(CConfigurationV2 cCfg)
{
    void* handle = dlopen(CJ_ABILITY_LIBNAME, RTLD_LAZY);
    if (handle == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null handle");
        return;
    }
    using FreeConfigFuncV2 = void (*)(CConfigurationV2*);
    auto func = reinterpret_cast<FreeConfigFuncV2>(dlsym(handle, FUNC_FREE_CONFIGURATION_V2));
    if (func == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null func");
        dlclose(handle);
        return;
    }
    func(&cCfg);
    dlclose(handle);
}

RetHapModuleInfoV2 CallConvertHapInfo(std::shared_ptr<AppExecFwk::HapModuleInfo> hapInfo)
{
    RetHapModuleInfoV2 retInfo = {};
    void* handle = dlopen(CJ_BUNDLE_MGR_LIBNAME, RTLD_LAZY);
    if (handle == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null handle");
        return retInfo;
    }
    using ConvertHapInfoFunc = RetHapModuleInfoV2 (*)(void*);
    auto func = reinterpret_cast<ConvertHapInfoFunc>(dlsym(handle, FUNC_CONVERT_HAP_INFO));
    if (func == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null func");
        dlclose(handle);
        return retInfo;
    }
    retInfo = func(hapInfo.get());
    dlclose(handle);
    return retInfo;
}

RetHapModuleInfoV2 CJAbilityStageContext::GetRetHapModuleInfo()
{
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "context is null, getHapModuleInfo failed. ");
        return RetHapModuleInfoV2();
    }

    auto hapInfo = context->GetHapModuleInfo();
    if (hapInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "CurrentHapMoudleInfo is nullptr.");
        return RetHapModuleInfoV2();
    }

    return CallConvertHapInfo(hapInfo);
}

std::shared_ptr<AppExecFwk::HapModuleInfo> CJAbilityStageContext::GetHapModuleInfo()
{
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        return nullptr;
    }
    return context->GetHapModuleInfo();
}

CConfiguration CJAbilityStageContext::GetConfiguration()
{
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "context is null, getConfiguration failed. ");
        return CConfiguration();
    }

    auto configuration = context->GetConfiguration();
    if (configuration == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "CurrentConfiguration is nullptr.");
        return CConfiguration();
    }

    return CallConvertConfig(configuration);
}

CConfigurationV2 CJAbilityStageContext::GetConfigurationV2()
{
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "context is null, getConfigurationV2 failed. ");
        return CConfigurationV2();
    }

    auto configuration = context->GetConfiguration();
    if (configuration == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "CurrentConfiguration is nullptr.");
        return CConfigurationV2();
    }

    return CallConvertConfigV2(configuration);
}

}
}