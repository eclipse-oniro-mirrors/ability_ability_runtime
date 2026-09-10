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

#include "ets_ukey_auth_ui_extension_base.h"

#include "application_configuration_manager.h"
#include "ets_runtime.h"
#include "ets_ukey_auth_ui_extension_context.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
EtsUkeyAuthUIExtensionBase::EtsUkeyAuthUIExtensionBase(const std::unique_ptr<Runtime> &runtime)
    : EtsUIExtensionBase(runtime) {}

void EtsUkeyAuthUIExtensionBase::BindContext()
{
    EtsUIExtensionBase::BindContext();
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "env is null");
        return;
    }
    if (etsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsObj_");
        return;
    }
    if (ukeyContext_ == nullptr) {
        ukeyContext_ = std::make_shared<UkeyAuthUIExtensionContext>();
        ukeyContext_->SetToken(context_ == nullptr ? nullptr : context_->GetToken());
        ukeyContext_->SetAbilityInfo(abilityInfo_);
        RegisterUkeyContextConfigUpdateCallback();
    }
    ani_object contextObj = CreateEtsUkeyAuthUIExtensionContext(env, ukeyContext_);
    if (contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null contextObj");
        return;
    }
    ani_field contextField = nullptr;
    auto status = env->Class_FindField(etsObj_->aniCls, "context", &contextField);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return;
    }
    ani_ref contextRef = nullptr;
    if ((status = env->GlobalReference_Create(contextObj, &contextRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return;
    }
    if ((status = env->Object_SetField_Ref(etsObj_->aniObj, contextField, contextRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
    }
    env->GlobalReference_Delete(contextRef);
}

void EtsUkeyAuthUIExtensionBase::OnCommandWindow(const AAFwk::Want &want,
    const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd)
{
    EtsUIExtensionBase::OnCommandWindow(want, sessionInfo, winCmd);
    if (winCmd != AAFwk::WIN_CMD_FOREGROUND || sessionInfo == nullptr || ukeyContext_ == nullptr) {
        return;
    }
    auto it = uiWindowMap_.find(sessionInfo->uiExtensionComponentId);
    if (it != uiWindowMap_.end() && it->second != nullptr) {
        ukeyContext_->SetWindow(it->second);
        ukeyContext_->SetSessionInfo(sessionInfo);
    }
}

void EtsUkeyAuthUIExtensionBase::OnForeground(const AAFwk::Want &want,
    sptr<AAFwk::SessionInfo> sessionInfo)
{
    EtsUIExtensionBase::OnForeground(want, sessionInfo);
    if (sessionInfo == nullptr || ukeyContext_ == nullptr) {
        return;
    }
    auto it = uiWindowMap_.find(sessionInfo->uiExtensionComponentId);
    if (it != uiWindowMap_.end() && it->second != nullptr) {
        ukeyContext_->SetWindow(it->second);
        ukeyContext_->SetSessionInfo(sessionInfo);
        TAG_LOGI(AAFwkTag::UI_EXT, "ukey ets OnForeground: window and session injected");
    }
}

void EtsUkeyAuthUIExtensionBase::RegisterUkeyContextConfigUpdateCallback()
{
    if (ukeyContext_ == nullptr || abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null ukeyContext_ or abilityInfo_");
        return;
    }
    std::weak_ptr<UkeyAuthUIExtensionContext> ukeyContextWptr = ukeyContext_;
    ukeyContext_->RegisterAbilityConfigUpdateCallback(
        [ukeyContextWptr](AppExecFwk::Configuration &config) {
        auto ukeyContext = ukeyContextWptr.lock();
        if (ukeyContext == nullptr || ukeyContext->GetAbilityInfo() == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null ukeyContext or null GetAbilityInfo");
            return;
        }
        if (ukeyContext->GetAbilityConfiguration() == nullptr) {
            auto abilityModuleContext = ukeyContext->CreateModuleContext(
                ukeyContext->GetAbilityInfo()->moduleName);
            if (abilityModuleContext == nullptr) {
                TAG_LOGE(AAFwkTag::UI_EXT, "null abilityModuleContext");
                return;
            }
            auto abilityResourceMgr = abilityModuleContext->GetResourceManager();
            ukeyContext->SetAbilityResourceManager(abilityResourceMgr);
            AbilityRuntime::ApplicationConfigurationManager::GetInstance().
                AddIgnoreContext(ukeyContext, abilityResourceMgr);
        }
        ukeyContext->SetAbilityConfiguration(config);
    });
}
} // namespace AbilityRuntime
} // namespace OHOS
