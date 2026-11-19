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

#include "cj_extension_context.h"

#include "cj_macro.h"
#include "cj_common_ffi.h"
#include "cj_utils_ffi.h"
#include "bundle_manager_convert.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace CJSystemapi::BundleManager;

class CJExtensionContextImpl {
public:
    CJExtensionContextImpl(const std::shared_ptr<ExtensionContext> &context,
        std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo)
        : context_(context), abilityInfo_(abilityInfo) {}
    int32_t GetConfiguration(CConfiguration* cConfig);
    int32_t GetConfigurationV2(CConfigurationV2* cConfig);
    int32_t GetCurrentHapModuleInfo(RetHapModuleInfoV2* hapInfo);
    int32_t GetExtAbilityInfo(RetExtensionAbilityInfoV2* retInfo);
    std::weak_ptr<ExtensionContext> context_;
    std::weak_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo_;
};

CJExtensionContext::CJExtensionContext(const std::shared_ptr<ExtensionContext> &context,
    std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo)
    : FfiContext::CJContext(context)
{
    impl_ = std::make_shared<CJExtensionContextImpl>(context, abilityInfo);
}

int32_t CJExtensionContextImpl::GetConfiguration(CConfiguration* cConfig)
{
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "context is nullptr");
        return ERR_INVALID_INSTANCE_CODE;
    }
    auto configuration = context->GetConfiguration();
    if (configuration == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "GetConfiguration return nullptr");
        return ERR_INVALID_INSTANCE_CODE;
    }
    *cConfig = CreateCConfiguration(*configuration);
    return SUCCESS_CODE;
}

int32_t CJExtensionContextImpl::GetConfigurationV2(CConfigurationV2* cConfig)
{
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "context is nullptr");
        return ERR_INVALID_INSTANCE_CODE;
    }
    auto configuration = context->GetConfiguration();
    if (configuration == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "GetConfiguration return nullptr");
        return ERR_INVALID_INSTANCE_CODE;
    }
    *cConfig = CreateCConfigurationV2(*configuration);
    return SUCCESS_CODE;
}

int32_t CJExtensionContextImpl::GetExtAbilityInfo(RetExtensionAbilityInfoV2* retInfo)
{
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "context is nullptr");
        return ERR_INVALID_INSTANCE_CODE;
    }
    auto abilityInfo = abilityInfo_.lock();
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "GetAbilityInfo return nullptr");
        return ERR_INVALID_INSTANCE_CODE;
    }
    auto hapModuleInfo = context->GetHapModuleInfo();
    if (hapModuleInfo == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "GetCurrentHapModuleInfo return nullptr");
        return ERR_INVALID_INSTANCE_CODE;
    }
    auto isExist = [&abilityInfo](const AppExecFwk::ExtensionAbilityInfo& info) {
        TAG_LOGD(AAFwkTag::CONTEXT, "%{public}s, %{public}s", info.bundleName.c_str(), info.name.c_str());
        return info.bundleName == abilityInfo->bundleName && info.name == abilityInfo->name;
    };
    auto infoIter = std::find_if(
        hapModuleInfo->extensionInfos.begin(), hapModuleInfo->extensionInfos.end(), isExist);
    if (infoIter == hapModuleInfo->extensionInfos.end()) {
        TAG_LOGE(AAFwkTag::CONTEXT, "get extensionAbilityInfo fail");
        return ERR_INVALID_INSTANCE_CODE;
    }
    *retInfo = Convert::ConvertExtensionAbilityInfoV2(*infoIter);
    return SUCCESS_CODE;
}

int32_t CJExtensionContextImpl::GetCurrentHapModuleInfo(RetHapModuleInfoV2* hapInfo)
{
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "context is nullptr");
        return ERR_INVALID_INSTANCE_CODE;
    }
    auto hapModuleInfo = context->GetHapModuleInfo();
    if (hapModuleInfo == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "GetCurrentHapModuleInfo return nullptr");
        return ERR_INVALID_INSTANCE_CODE;
    }
    *hapInfo = Convert::ConvertHapModuleInfoV2(*hapModuleInfo);
    return SUCCESS_CODE;
}

extern "C" {
CJ_EXPORT int32_t FFICJExtCtxGetConfig(int64_t id, void* paramConfig)
{
    if (paramConfig == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "input param paramConfig is nullptr");
        return ERR_INVALID_INSTANCE_CODE;
    }
    auto cjContext = OHOS::FFI::FFIData::GetData<CJExtensionContext>(id);
    if (cjContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "GetCJExtensionContext failed, context is nullptr");
        return ERR_INVALID_INSTANCE_CODE;
    }
    return cjContext->impl_->GetConfiguration(static_cast<CConfiguration*>(paramConfig));
}

CJ_EXPORT int32_t FFICJExtCtxGetConfigV2(int64_t id, void* paramConfig)
{
    if (paramConfig == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "input param paramConfig is nullptr");
        return ERR_INVALID_INSTANCE_CODE;
    }
    auto cjContext = OHOS::FFI::FFIData::GetData<CJExtensionContext>(id);
    if (cjContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "GetCJExtensionContext failed, context is nullptr");
        return ERR_INVALID_INSTANCE_CODE;
    }
    return cjContext->impl_->GetConfigurationV2(static_cast<CConfigurationV2*>(paramConfig));
}

CJ_EXPORT int32_t FFICJExtCtxGetExtAbilityInfo(int64_t id, void* retInfo)
{
    if (retInfo == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "input param retInfo is nullptr");
        return ERR_INVALID_INSTANCE_CODE;
    }
    auto cjContext = OHOS::FFI::FFIData::GetData<CJExtensionContext>(id);
    if (cjContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "GetCJExtensionContext failed, context is nullptr");
        return ERR_INVALID_INSTANCE_CODE;
    }
    return cjContext->impl_->GetExtAbilityInfo(static_cast<RetExtensionAbilityInfoV2*>(retInfo));
}

CJ_EXPORT int32_t FFICJExtCtxGetCurrentHapModuleInfo(int64_t id, void* hapInfo)
{
    if (hapInfo == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "input param hapInfo is nullptr");
        return ERR_INVALID_INSTANCE_CODE;
    }
    auto cjContext = OHOS::FFI::FFIData::GetData<CJExtensionContext>(id);
    if (cjContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "GetCJExtensionContext failed, context is nullptr");
        return ERR_INVALID_INSTANCE_CODE;
    }
    return cjContext->impl_->GetCurrentHapModuleInfo(static_cast<RetHapModuleInfoV2*>(hapInfo));
}
}
} // namespace AbilityRuntime
} // namespace OHOS
