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

#include "js_ukey_auth_ui_extension_base.h"

#include "ability_info.h"
#include "application_configuration_manager.h"
#include "hilog_tag_wrapper.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "js_ukey_auth_ui_extension_context.h"
#include "napi/native_api.h"
#include "native_engine/impl/ark/ark_native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ONE = 1;
}
JsUkeyAuthUIExtensionBase::JsUkeyAuthUIExtensionBase(const std::unique_ptr<Runtime> &runtime)
    : JsUIExtensionBase(runtime) {}

void JsUkeyAuthUIExtensionBase::BindContext()
{
    JsUIExtensionBase::BindContext();
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    if (jsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null jsObj_");
        return;
    }
    if (ukeyContext_ == nullptr) {
        ukeyContext_ = std::make_shared<UkeyAuthUIExtensionContext>();
        ukeyContext_->SetToken(context_ == nullptr ? nullptr : context_->GetToken());
        ukeyContext_->SetAbilityInfo(abilityInfo_);
        RegisterUkeyContextConfigUpdateCallback();
    }
    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "not object");
        return;
    }
    napi_value contextObj = JsUkeyAuthUIExtensionContext::CreateJsUkeyAuthUIExtensionContext(env, ukeyContext_);
    if (contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null contextObj");
        return;
    }
    auto ukeyContextRef = JsRuntime::LoadSystemModuleByEngine(
        env, "application.UkeyAuthUIExtensionContext", &contextObj, ARGC_ONE);
    if (ukeyContextRef == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "get LoadSystemModuleByEngine failed");
        return;
    }
    contextObj = ukeyContextRef->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "get object failed");
        return;
    }
    napi_set_named_property(env, obj, "context", contextObj);
}

void JsUkeyAuthUIExtensionBase::OnCommandWindow(const AAFwk::Want &want,
    const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd)
{
    JsUIExtensionBase::OnCommandWindow(want, sessionInfo, winCmd);
    if (winCmd != AAFwk::WIN_CMD_FOREGROUND || sessionInfo == nullptr || ukeyContext_ == nullptr) {
        return;
    }
    auto it = uiWindowMap_.find(sessionInfo->uiExtensionComponentId);
    if (it != uiWindowMap_.end() && it->second != nullptr) {
        ukeyContext_->SetWindow(it->second);
        ukeyContext_->SetSessionInfo(sessionInfo);
    } else {
        TAG_LOGE(AAFwkTag::UI_EXT, "ukey OnCommandWindow: window not found, componentId=%{public}llu,"
            " mapSize=%{public}zu", static_cast<unsigned long long>(sessionInfo->uiExtensionComponentId),
            uiWindowMap_.size());
    }
}

void JsUkeyAuthUIExtensionBase::OnForeground(const AAFwk::Want &want,
    sptr<AAFwk::SessionInfo> sessionInfo)
{
    JsUIExtensionBase::OnForeground(want, sessionInfo);
    if (sessionInfo == nullptr || ukeyContext_ == nullptr) {
        return;
    }
    auto it = uiWindowMap_.find(sessionInfo->uiExtensionComponentId);
    if (it != uiWindowMap_.end() && it->second != nullptr) {
        ukeyContext_->SetWindow(it->second);
        ukeyContext_->SetSessionInfo(sessionInfo);
        TAG_LOGI(AAFwkTag::UI_EXT, "ukey OnForeground: window and session injected, componentId=%{public}llu",
            static_cast<unsigned long long>(sessionInfo->uiExtensionComponentId));
    } else {
        TAG_LOGE(AAFwkTag::UI_EXT, "ukey OnForeground: window not found, componentId=%{public}llu,"
            " mapSize=%{public}zu", static_cast<unsigned long long>(sessionInfo->uiExtensionComponentId),
            uiWindowMap_.size());
    }
}

void JsUkeyAuthUIExtensionBase::RegisterUkeyContextConfigUpdateCallback()
{
    if (ukeyContext_ == nullptr || abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null ukeyContext_ or abilityInfo_");
        return;
    }
    auto ukeyExtensionAbility = std::static_pointer_cast<JsUkeyAuthUIExtensionBase>(shared_from_this());
    std::weak_ptr<JsUkeyAuthUIExtensionBase> abilityWptr = ukeyExtensionAbility;
    std::weak_ptr<UkeyAuthUIExtensionContext> ukeyContextWptr = ukeyContext_;
    ukeyContext_->RegisterAbilityConfigUpdateCallback(
        [abilityWptr, ukeyContextWptr](AppExecFwk::Configuration &config) {
        std::shared_ptr<JsUkeyAuthUIExtensionBase> abilitySptr = abilityWptr.lock();
        if (abilitySptr == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null abilitySptr");
            return;
        }
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
            TAG_LOGD(AAFwkTag::UI_EXT, "%{public}zu",
                AbilityRuntime::ApplicationConfigurationManager::GetInstance().GetIgnoreContext().size());
        }
        ukeyContext->SetAbilityConfiguration(config);
        if (config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE).
            compare(AppExecFwk::ConfigurationInner::COLOR_MODE_AUTO) == 0) {
            config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE,
                ApplicationConfigurationManager::GetInstance().GetColorMode());

            if (AbilityRuntime::ApplicationConfigurationManager::GetInstance().
                GetColorModeSetLevel() > AbilityRuntime::SetLevel::System) {
                config.AddItem(AAFwk::GlobalConfigurationKey::COLORMODE_IS_SET_BY_APP,
                    AppExecFwk::ConfigurationInner::IS_SET_BY_APP);
            }
            ukeyContext->GetAbilityConfiguration()->
                RemoveItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
            ukeyContext->GetAbilityConfiguration()->
                RemoveItem(AAFwk::GlobalConfigurationKey::COLORMODE_IS_SET_BY_APP);
        }

        abilitySptr->OnAbilityConfigurationUpdated(config);
    });
}
} // namespace AbilityRuntime
} // namespace OHOS
