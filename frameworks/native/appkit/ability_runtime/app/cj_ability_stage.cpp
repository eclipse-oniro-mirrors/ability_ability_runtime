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

#include "cj_ability_stage.h"
#include "cj_runtime.h"
#include "context_impl.h"
#include "hilog_tag_wrapper.h"
#include "securec.h"
#include "ability_delegator_registry.h"
#include "js_ability_stage_context.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
char* CreateCStringFromString(const std::string& source)
{
    if (source.size() == 0) {
        return nullptr;
    }
    size_t length = source.size() + 1;
    auto res = static_cast<char*>(malloc(length));
    if (res == nullptr) {
        TAG_LOGE(AAFwkTag::DEFAULT, "null res");
        return nullptr;
    }
    if (strcpy_s(res, length, source.c_str()) != 0) {
        free(res);
        TAG_LOGE(AAFwkTag::DEFAULT, "Strcpy failed");
        return nullptr;
    }
    return res;
}
}

extern "C" {
CJ_EXPORT RetHapModuleInfoV2 FFICJGetHapModuleInfo(int64_t id)
{
    auto abilityStageContext = OHOS::FFI::FFIData::GetData<CJAbilityStageContext>(id);
    if (abilityStageContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "get abilityStageContext failed");
        return RetHapModuleInfoV2();
    }

    return abilityStageContext->GetRetHapModuleInfo();
}

CJ_EXPORT CurrentHapModuleInfo* FFICJCurrentHapModuleInfo(int64_t id)
{
    auto abilityStageContext = OHOS::FFI::FFIData::GetData<CJAbilityStageContext>(id);
    if (abilityStageContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null abilityStageContext");
        return nullptr;
    }

    auto hapInfo = abilityStageContext->GetHapModuleInfo();
    if (hapInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null hapInfo");
        return nullptr;
    }

    CurrentHapModuleInfo* buffer = static_cast<CurrentHapModuleInfo*>(malloc(sizeof(CurrentHapModuleInfo)));

    if (buffer == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Create CurrentHapModuleInfo failed, CurrentHapModuleInfo is nullptr.");
        return nullptr;
    }

    buffer->name = CreateCStringFromString(hapInfo->name);
    buffer->icon = CreateCStringFromString(hapInfo->iconPath);
    buffer->iconId = hapInfo->iconId;
    buffer->label = CreateCStringFromString(hapInfo->label);
    buffer->labelId = hapInfo->labelId;
    buffer->description = CreateCStringFromString(hapInfo->description);
    buffer->descriptionId = hapInfo->descriptionId;
    buffer->mainElementName = CreateCStringFromString(hapInfo->mainElementName);
    buffer->installationFree = hapInfo->installationFree;
    buffer->hashValue = CreateCStringFromString(hapInfo->hashValue);

    return buffer;
}

CJ_EXPORT CConfiguration FFICJGetConfiguration(int64_t id)
{
    auto abilityStageContext = OHOS::FFI::FFIData::GetData<CJAbilityStageContext>(id);
    if (abilityStageContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Get abilityStageContext failed. ");
        return CConfiguration();
    }

    return abilityStageContext->GetConfiguration();
}

CJ_EXPORT CConfigurationV2 FFICJGetConfigurationV2(int64_t id)
{
    auto abilityStageContext = OHOS::FFI::FFIData::GetData<CJAbilityStageContext>(id);
    if (abilityStageContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Get abilityStageContext failed. ");
        return CConfigurationV2();
    }

    return abilityStageContext->GetConfigurationV2();
}

CJ_EXPORT int64_t FFIAbilityGetAbilityStageContext(AbilityStageHandle abilityStageHandle)
{
    auto ability = static_cast<CJAbilityStage*>(abilityStageHandle);
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "GetAbilityStageContext failed, abilityStage is nullptr.");
        return ERR_INVALID_INSTANCE_CODE;
    }
    auto context = ability->GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        return ERR_INVALID_INSTANCE_CODE;
    }
    auto cjStageContext = OHOS::FFI::FFIData::Create<CJAbilityStageContext>(context);
    if (cjStageContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null cjStageContext");
        return ERR_INVALID_INSTANCE_CODE;
    }
    return cjStageContext->GetID();
}

CJ_EXPORT napi_value FfiConvertAbilityStageContext2Napi(napi_env env, int64_t id)
{
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);
    auto cjAbilityStageContext = OHOS::FFI::FFIData::GetData<CJAbilityStageContext>(id);
    if (cjAbilityStageContext == nullptr || cjAbilityStageContext->GetContext() == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "cj context null ptr");
        return undefined;
    }

    napi_value result = CreateJsAbilityStageContext(env, cjAbilityStageContext->GetContext());
    if (result == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null object");
        return undefined;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<OHOS::AbilityRuntime::Context>(
        cjAbilityStageContext->GetContext());
    napi_status status = napi_wrap(env, result, workContext,
        [](napi_env, void* data, void*) {
            TAG_LOGD(AAFwkTag::APPKIT, "Finalizer for weak_ptr ability stage context is called");
            delete static_cast<std::weak_ptr<OHOS::AbilityRuntime::Context>*>(data);
        },
        nullptr, nullptr);
    if (status != napi_ok && workContext != nullptr) {
        TAG_LOGD(AAFwkTag::APPKIT, "napi_wrap Failed: %{public}d", status);
        delete workContext;
        return undefined;
    }
    napi_value falseValue = nullptr;
    napi_get_boolean((napi_env)env, true, &falseValue);
    napi_set_named_property((napi_env)env, result, "stageMode", falseValue);
    return result;
}

CJ_EXPORT int64_t FfiCreateAbilityStageContextFromNapi(napi_env env, napi_value stageContext)
{
    if (env == nullptr || stageContext == nullptr) {
        return ERR_INVALID_INSTANCE_CODE;
    }

    napi_valuetype type;
    if (napi_typeof(env, stageContext, &type) || type != napi_object) {
        return ERR_INVALID_INSTANCE_CODE;
    }

    std::weak_ptr<Context>* context = nullptr;
    napi_status status = napi_unwrap(env, stageContext, reinterpret_cast<void**>(&context));
    if (status != napi_ok) {
        return ERR_INVALID_INSTANCE_CODE;
    }

    if (context == nullptr || (*context).lock() == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return ERR_INVALID_INSTANCE_CODE;
    }
    auto cjContext = OHOS::FFI::FFIData::Create<CJAbilityStageContext>((*context).lock());
    if (cjContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null cjContext");
        return ERR_INVALID_INSTANCE_CODE;
    }
    return cjContext->GetID();
}
}

std::shared_ptr<CJAbilityStage> CJAbilityStage::Create(
    const std::unique_ptr<Runtime>& runtime, const AppExecFwk::HapModuleInfo& hapModuleInfo)
{
    if (!runtime) {
        TAG_LOGE(AAFwkTag::APPKIT, "null runtime");
        return nullptr;
    }
    auto& cjRuntime = static_cast<CJRuntime&>(*runtime);
    // Load cj app library.
    if (!cjRuntime.IsAppLibLoaded()) {
        TAG_LOGE(AAFwkTag::APPKIT, "appLib not loaded");
        return nullptr;
    }

    auto cjAbilityStageObject = CJAbilityStageObject::LoadModule(hapModuleInfo.moduleName);
    if (cjAbilityStageObject == nullptr) {
        cjRuntime.UnLoadCJAppLibrary();
        TAG_LOGE(AAFwkTag::APPKIT, "null cjAbilityStage");
        return nullptr;
    }

    return std::make_shared<CJAbilityStage>(cjAbilityStageObject);
}

void CJAbilityStage::Init(const std::shared_ptr<Context> &context,
    const std::weak_ptr<AppExecFwk::OHOSApplication> application)
{
    AbilityStage::Init(context, application);
    if (!cjAbilityStageObject_) {
        TAG_LOGE(AAFwkTag::APPKIT, "null cjAbilityStage");
        return;
    }
    cjAbilityStageObject_->Init(this);
}

void CJAbilityStage::OnCreate(const AAFwk::Want& want) const
{
    AbilityStage::OnCreate(want);
    if (!cjAbilityStageObject_) {
        TAG_LOGE(AAFwkTag::APPKIT, "null cjAbilityStage");
        return;
    }
    TAG_LOGD(AAFwkTag::APPKIT, "CJAbilityStage::OnCreate");
    cjAbilityStageObject_->OnCreate();

    auto delegator = OHOS::AppExecFwk::AbilityDelegatorRegistry::GetCJAbilityDelegator();
    if (delegator) {
        delegator->PostPerformStageStart(CreateStageProperty());
    }
}

std::string CJAbilityStage::OnAcceptWant(const AAFwk::Want& want)
{
    AbilityStage::OnAcceptWant(want);
    if (!cjAbilityStageObject_) {
        TAG_LOGE(AAFwkTag::APPKIT, "null cjAbilityStage");
        return "";
    }
    return cjAbilityStageObject_->OnAcceptWant(want);
}

std::string CJAbilityStage::OnNewProcessRequest(const AAFwk::Want& want)
{
    AbilityStage::OnNewProcessRequest(want);
    if (!cjAbilityStageObject_) {
        TAG_LOGE(AAFwkTag::APPKIT, "CJAbilityStage is not loaded.");
        return "";
    }
    return cjAbilityStageObject_->OnNewProcessRequest(want);
}

void CJAbilityStage::OnConfigurationUpdated(const AppExecFwk::Configuration& configuration)
{
    AbilityStage::OnConfigurationUpdated(configuration);
    auto fullConfig = GetContext()->GetConfiguration();
    if (!fullConfig) {
        TAG_LOGE(AAFwkTag::APPKIT, "null fullConfig");
        return;
    }

    if (!cjAbilityStageObject_) {
        TAG_LOGE(AAFwkTag::APPKIT, "null cjAbilityStage");
        return;
    }
    cjAbilityStageObject_->OnConfigurationUpdated(fullConfig);
}

void CJAbilityStage::OnMemoryLevel(int level)
{
    AbilityStage::OnMemoryLevel(level);
    if (!cjAbilityStageObject_) {
        TAG_LOGE(AAFwkTag::APPKIT, "null cjAbilityStage");
        return;
    }
    cjAbilityStageObject_->OnMemoryLevel(level);
}

void CJAbilityStage::OnDestroy() const
{
    AbilityStage::OnDestroy();
    if (!cjAbilityStageObject_) {
        TAG_LOGE(AAFwkTag::APPKIT, "CJAbilityStage is not loaded.");
        return;
    }
    cjAbilityStageObject_->OnDestroy();
}

std::shared_ptr<OHOS::AppExecFwk::CJDelegatorAbilityStageProperty> CJAbilityStage::CreateStageProperty() const
{
    auto property = std::make_shared<OHOS::AppExecFwk::CJDelegatorAbilityStageProperty>();
    auto context = GetContext();
    if (!context) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get context");
        return nullptr;
    }
    auto hapModuleInfo = context->GetHapModuleInfo();
    if (!hapModuleInfo) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get hapModuleInfo");
        return nullptr;
    }
    property->moduleName_ = hapModuleInfo->name;
    property->srcEntrance_ = hapModuleInfo->srcEntrance;
    property->cjStageObject_ = cjAbilityStageObject_->GetId();
    return property;
}
}
}
