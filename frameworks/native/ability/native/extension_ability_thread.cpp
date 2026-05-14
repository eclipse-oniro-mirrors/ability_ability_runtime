/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#include "extension_ability_thread.h"

#include "ability_context_impl.h"
#include "ability_handler.h"
#include "ability_loader.h"
#include "ability_manager_client.h"
#include "freeze_util.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "js_start_abilities_observer.h"
#include "ui_extension_wrapper.h"
#include "start_abilities_observer.h"
#include "skill/skill_execute_param.h"

namespace OHOS {
namespace AbilityRuntime {
using AbilityManagerClient = OHOS::AAFwk::AbilityManagerClient;
namespace {
#ifdef SUPPORT_GRAPHICS
constexpr static char FORM_EXTENSION[] = "FormExtension";
constexpr static char FORM_EDIT_EXTENSION[] = "FormEditExtensionAbility";
constexpr static char LIVE_FORM_EXTENSION[] = "LiveFormExtensionAbility";
constexpr static char UI_EXTENSION[] = "UIExtensionAbility";
constexpr static char CUSTOM_EXTENSION[] = "ExtensionAbility";
constexpr static char USER_AUTH_EXTENSION[] = "UserAuthExtensionAbility";
constexpr static char ACTION_EXTENSION[] = "ActionExtensionAbility";
constexpr static char SHARE_EXTENSION[] = "ShareExtensionAbility";
constexpr static char AGENT_UI_EXTENSION[] = "AgentUIExtensionAbility";
#ifdef SUPPORT_AUTO_FILL
constexpr static char AUTO_FILL_EXTENSION[] = "AutoFillExtensionAbility";
#endif  // SUPPORT_AUTO_FILL
constexpr static char EMBEDDED_UI_EXTENSION[] = "EmbeddedUIExtensionAbility";
constexpr static char PHOTO_EDITOR_EXTENSION[] = "PhotoEditorExtensionAbility";
constexpr static char VPN_EXTENSION[] = "VpnExtension";
#endif
constexpr static char BASE_SERVICE_EXTENSION[] = "ServiceExtension";
constexpr static char BASE_DRIVER_EXTENSION[] = "DriverExtension";
constexpr static char STATIC_SUBSCRIBER_EXTENSION[] = "StaticSubscriberExtension";
constexpr static char DATA_SHARE_EXT_ABILITY[] = "DataShareExtAbility";
constexpr static char WORK_SCHEDULER_EXTENSION[] = "WorkSchedulerExtension";
constexpr static char ACCESSIBILITY_EXTENSION[] = "AccessibilityExtension";
constexpr static char WALLPAPER_EXTENSION[] = "WallpaperExtension";
constexpr static char FILEACCESS_EXT_ABILITY[] = "FileAccessExtension";
constexpr static char ENTERPRISE_ADMIN_EXTENSION[] = "EnterpriseAdminExtension";
constexpr static char INPUTMETHOD_EXTENSION[] = "InputMethodExtensionAbility";
constexpr static char APP_ACCOUNT_AUTHORIZATION_EXTENSION[] = "AppAccountAuthorizationExtension";
constexpr static char FENCE_EXTENSION[] = "FenceExtension";
constexpr static char CALLER_INFO_QUERY_EXTENSION[] = "CallerInfoQueryExtension";
constexpr static char ASSET_ACCELERATION_EXTENSION[] = "AssetAccelerationExtension";
constexpr static char SELECTION_EXTENSION[] = "SelectionExtensionAbility";
constexpr static char CONTENT_EMBED_EXTENSION[] = "ContentEmbedExtension";
constexpr static char MODULAR_OBJECT_EXTENSION[] = "ModularObjectExtension";
}

const std::map<AppExecFwk::ExtensionAbilityType, std::string> UI_EXTENSION_NAME_MAP = {
#ifdef SUPPORT_GRAPHICS
    { AppExecFwk::ExtensionAbilityType::SHARE, SHARE_EXTENSION },
    { AppExecFwk::ExtensionAbilityType::ACTION, ACTION_EXTENSION },
    { AppExecFwk::ExtensionAbilityType::AGENT_UI, AGENT_UI_EXTENSION },
#ifdef SUPPORT_AUTO_FILL
    { AppExecFwk::ExtensionAbilityType::AUTO_FILL_PASSWORD, AUTO_FILL_EXTENSION },
    { AppExecFwk::ExtensionAbilityType::AUTO_FILL_SMART, AUTO_FILL_EXTENSION },
#endif  // SUPPORT_AUTO_FILL
    { AppExecFwk::ExtensionAbilityType::EMBEDDED_UI, EMBEDDED_UI_EXTENSION },
    { AppExecFwk::ExtensionAbilityType::PHOTO_EDITOR, PHOTO_EDITOR_EXTENSION },
    { AppExecFwk::ExtensionAbilityType::FORM_EDIT, FORM_EDIT_EXTENSION },
    { AppExecFwk::ExtensionAbilityType::LIVE_FORM, LIVE_FORM_EXTENSION }
#endif //SUPPORT_GRAPHICS
};

std::shared_ptr<AppExecFwk::EventRunner> ExtensionAbilityThread::contentEmbedEventRunner_ = nullptr;

ExtensionAbilityThread::ExtensionAbilityThread() : extensionImpl_(nullptr), currentExtension_(nullptr) {}

ExtensionAbilityThread::~ExtensionAbilityThread()
{
    if (currentExtension_ != nullptr) {
        currentExtension_.reset();
    }
}

std::string ExtensionAbilityThread::CreateAbilityName(
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
    const std::shared_ptr<AppExecFwk::OHOSApplication> &application)
{
    std::string abilityName;
    if (abilityRecord == nullptr || application == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "null AbilityRecord or application");
        return abilityName;
    }

    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = abilityRecord->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "null abilityInfo");
        return abilityName;
    }

    if (abilityInfo->isNativeAbility || abilityInfo->type != AppExecFwk::AbilityType::EXTENSION) {
        TAG_LOGD(AAFwkTag::EXT, "Ability info name:%{public}s", abilityInfo->name.c_str());
        return abilityInfo->name;
    }

    application->GetExtensionNameByType(static_cast<int32_t>(abilityInfo->extensionAbilityType), abilityName);
    if (!abilityName.empty()) {
        TAG_LOGD(AAFwkTag::EXT, "Get extension name: %{public}s success", abilityName.c_str());
        return abilityName;
    }

    abilityName = BASE_SERVICE_EXTENSION;
#ifdef SUPPORT_GRAPHICS
    if (abilityInfo->formEnabled || abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::FORM) {
        abilityName = FORM_EXTENSION;
    }

    if (AAFwk::UIExtensionWrapper::IsUIExtension(abilityInfo->extensionAbilityType)) {
        auto iter = UI_EXTENSION_NAME_MAP.find(abilityInfo->extensionAbilityType);
        if (iter != UI_EXTENSION_NAME_MAP.end()) {
            abilityName = iter->second;
        } else {
            abilityName = UI_EXTENSION;
        }
    }
#endif
    CreateExtensionAbilityName(abilityInfo, abilityName);
    TAG_LOGD(AAFwkTag::EXT, "Ability name: %{public}s", abilityName.c_str());
    return abilityName;
}

void ExtensionAbilityThread::CreateExtensionAbilityName(
    const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo, std::string &abilityName)
{
    if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::STATICSUBSCRIBER) {
        abilityName = STATIC_SUBSCRIBER_EXTENSION;
    } else if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::DRIVER) {
        abilityName = BASE_DRIVER_EXTENSION;
    } else if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::DATASHARE) {
        abilityName = DATA_SHARE_EXT_ABILITY;
    } else if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::WORK_SCHEDULER) {
        abilityName = WORK_SCHEDULER_EXTENSION;
    } else if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::ACCESSIBILITY) {
        abilityName = ACCESSIBILITY_EXTENSION;
    } else if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::WALLPAPER) {
        abilityName = WALLPAPER_EXTENSION;
    } else if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::FILEACCESS_EXTENSION) {
        abilityName = FILEACCESS_EXT_ABILITY;
    } else if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::ENTERPRISE_ADMIN) {
        abilityName = ENTERPRISE_ADMIN_EXTENSION;
    } else if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::INPUTMETHOD) {
        abilityName = INPUTMETHOD_EXTENSION;
    } else if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::APP_ACCOUNT_AUTHORIZATION) {
        abilityName = APP_ACCOUNT_AUTHORIZATION_EXTENSION;
    } else if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::FENCE) {
        abilityName = FENCE_EXTENSION;
    } else if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::CALLER_INFO_QUERY) {
        abilityName = CALLER_INFO_QUERY_EXTENSION;
    } else if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::ASSET_ACCELERATION) {
        abilityName = ASSET_ACCELERATION_EXTENSION;
    } else if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::SELECTION) {
        abilityName = SELECTION_EXTENSION;
    }
#ifdef SUPPORT_GRAPHICS
    else if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::SYSDIALOG_USERAUTH) {
        abilityName = USER_AUTH_EXTENSION;
    } else if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::VPN) {
        abilityName = VPN_EXTENSION;
    } else if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::UNSPECIFIED &&
        abilityInfo->type == AppExecFwk::AbilityType::EXTENSION) {
        abilityName = abilityInfo->extensionTypeName + CUSTOM_EXTENSION;
    }
#endif // SUPPORT_GRAPHICS
    else if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::CONTENT_EMBED) {
        abilityName = CONTENT_EMBED_EXTENSION;
    } else if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::MODULAR_OBJECT) {
        abilityName = MODULAR_OBJECT_EXTENSION;
    }
}

void ExtensionAbilityThread::Attach(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
    const std::shared_ptr<AppExecFwk::EventRunner> &mainRunner,
    [[maybe_unused]] const std::shared_ptr<Context> &appContext)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::EXT, "Begin");
    if (application == nullptr || abilityRecord == nullptr || mainRunner == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "null application or abilityRecord or mainRunner");
        return;
    }
    HandleAttach(application, abilityRecord, mainRunner);
    TAG_LOGD(AAFwkTag::EXT, "End");
}

void ExtensionAbilityThread::Attach(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
    [[maybe_unused]] const std::shared_ptr<Context> &appContext)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::EXT, "Begin");
    if ((application == nullptr) || (abilityRecord == nullptr)) {
        TAG_LOGE(AAFwkTag::EXT, "null application or abilityRecord");
        return;
    }
    HandleAttach(application, abilityRecord, nullptr);
    TAG_LOGD(AAFwkTag::EXT, "End");
}

void ExtensionAbilityThread::HandleAttach(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
    const std::shared_ptr<AppExecFwk::EventRunner> &mainRunner)
{
    if (application == nullptr || abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "null Application or abilityRecord");
        return;
    }

    // 1.new AbilityHandler
    std::string abilityName = CreateAbilityName(abilityRecord, application);
    if (abilityName.empty()) {
        TAG_LOGE(AAFwkTag::EXT, "empty abilityName");
        FreezeUtil::GetInstance().AddLifecycleEvent(abilityRecord->GetToken(),
            "ExtensionAbilityThread::HandleAttach, empty abilityName.");
        return;
    }

    TAG_LOGD(AAFwkTag::EXT, "HandleAttach, extension: %{public}s", abilityName.c_str());
    FreezeUtil::GetInstance().AddLifecycleEvent(abilityRecord->GetToken(), "ExtensionAbilityThread::HandleAttach");
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = abilityRecord->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "null abilityInfo");
        return;
    }

    // 1.new AbilityHandler - original per-type logic
    if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::CONTENT_EMBED) {
        HandleNativeExtensionAttach(abilityRecord, abilityName);
    } else {
        HandleNormalExtensionAttach(abilityRecord, mainRunner, abilityName);
    }

    // 2.new ability
    auto extension = AppExecFwk::AbilityLoader::GetInstance().GetExtensionByName(abilityName,
        abilityInfo->arkTSMode);
    if (extension == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "null extension");
        return;
    }
    currentExtension_.reset(extension);

    // 3.Override with extension-provided handler if available (e.g. ModularObject)
    auto customHandler = currentExtension_->GetAbilityHandler(abilityInfo);
    if (customHandler != nullptr) {
        TAG_LOGD(AAFwkTag::EXT, "Using extension-provided abilityHandler");
        abilityHandler_ = customHandler;
    }

    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "Failed to create abilityHandler_");
        return;
    }

    token_ = abilityRecord->GetToken();
    abilityRecord->SetAbilityThread(this);
    HandleAttachInner(application, abilityRecord);
}

void ExtensionAbilityThread::HandleNativeExtensionAttach(
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
    const std::string &abilityName)
{
    TAG_LOGD(AAFwkTag::EXT, "HandleNativeExtensionAttach");
    if (contentEmbedEventRunner_ != nullptr) {
        runner_ = contentEmbedEventRunner_;
        abilityHandler_ = std::make_shared<AppExecFwk::AbilityHandler>(runner_);
        TAG_LOGD(AAFwkTag::EXT, "Reusing existing Content Embed EventRunner");
    } else {
        runner_ = AppExecFwk::EventRunner::Create(abilityName + "_CONTENT_EMBED");
        if (runner_ == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "Failed to create Content Embed EventRunner");
            return;
        }
        abilityHandler_ = std::make_shared<AppExecFwk::AbilityHandler>(runner_);
        if (abilityHandler_ == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "Failed to create Content Embed AbilityHandler");
            return;
        }
        contentEmbedEventRunner_ = runner_;
        TAG_LOGI(AAFwkTag::EXT, "Created new Content Embed EventRunner");
    }
}
 
void ExtensionAbilityThread::HandleNormalExtensionAttach(
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
    const std::shared_ptr<AppExecFwk::EventRunner> &mainRunner,
    const std::string &abilityName)
{
    if (mainRunner == nullptr) {
        runner_ = AppExecFwk::EventRunner::Create(abilityName);
        if (runner_ == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "null runner_");
            return;
        }
        abilityHandler_ = std::make_shared<AppExecFwk::AbilityHandler>(runner_);
    } else {
        abilityHandler_ = std::make_shared<AppExecFwk::AbilityHandler>(mainRunner);
    }
}

void ExtensionAbilityThread::HandleAttachInner(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord)
{
    extensionImpl_ = std::make_shared<ExtensionImpl>();
    if (extensionImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "null extensionImpl_");
        return;
    }

    FreezeUtil::GetInstance().AddLifecycleEvent(abilityRecord->GetToken(),
        "ExtensionAbilityThread::HandleAttachInner");
    // 3.new init
    extensionImpl_->Init(application, abilityRecord, currentExtension_, abilityHandler_, token_);
    // 4.ipc attach init
    ErrCode err = AbilityManagerClient::GetInstance()->AttachAbilityThread(this, token_);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::EXT, "Attach err: %{public}d", err);
        FreezeUtil::GetInstance().AddLifecycleEvent(abilityRecord->GetToken(),
            "ExtensionAbilityThread::HandleAttachInner fail, error is " + std::to_string(err));
        return;
    }
    FreezeUtil::GetInstance().DeleteAppLifecycleEvent(0);
    FreezeUtil::GetInstance().DeleteLifecycleEvent(abilityRecord->GetToken());
}

void ExtensionAbilityThread::HandleExtensionTransaction(
    const Want &want, const LifeCycleStateInfo &lifeCycleStateInfo, sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::EXT, "%{public}s", __func__);
    if (extensionImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "null extensionImpl_");
        return;
    }
    extensionImpl_->HandleExtensionTransaction(want, lifeCycleStateInfo, sessionInfo);
}

void ExtensionAbilityThread::HandleConnectExtension(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::EXT, "Begin");
    FreezeUtil::GetInstance().AddLifecycleEvent(token_,
        "ExtensionAbilityThread::HandleConnectExtension");
    if (extensionImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "null extensionImpl_");
        return;
    }

    bool isAsyncCallback = false;
    sptr<IRemoteObject> service = extensionImpl_->ConnectExtension(want, isAsyncCallback);
    if (!isAsyncCallback) {
        extensionImpl_->ConnectExtensionCallback(service);
    }
    TAG_LOGD(AAFwkTag::EXT, "End");
}

void ExtensionAbilityThread::HandleDisconnectExtension(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::EXT, "Begin");
    if (extensionImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "null extensionImpl_");
        return;
    }

    bool isAsyncCallback = false;
    extensionImpl_->DisconnectExtension(want, isAsyncCallback);
    if (!isAsyncCallback) {
        extensionImpl_->DisconnectExtensionCallback();
    }
    TAG_LOGD(AAFwkTag::EXT, "End");
}

void ExtensionAbilityThread::HandleCommandExtension(const Want &want, bool restart, int32_t startId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::EXT, "Begin");
    if (extensionImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "null extensionImpl_");
        return;
    }
    extensionImpl_->CommandExtension(want, restart, startId);
    ErrCode err = AbilityManagerClient::GetInstance()->ScheduleCommandAbilityDone(token_);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::EXT, "err: %{public}d", err);
    }
    TAG_LOGD(AAFwkTag::EXT, "End");
}

void ExtensionAbilityThread::HandleInsightIntent(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::EXT, "Begin");
    if (extensionImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "null extensionImpl_");
        return;
    }
    auto ret = extensionImpl_->HandleInsightIntent(want);
    if (!ret) {
        TAG_LOGE(AAFwkTag::EXT, "HandleInsightIntent failed");
        return;
    }
    TAG_LOGD(AAFwkTag::EXT, "End");
}

void ExtensionAbilityThread::HandleExecuteSkill(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::EXT, "Begin");
    if (extensionImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "null extensionImpl_");
        return;
    }
    auto ret = extensionImpl_->HandleExecuteSkill(want);
    if (!ret) {
        TAG_LOGE(AAFwkTag::EXT, "HandleExecuteSkill failed");
        return;
    }
    TAG_LOGD(AAFwkTag::EXT, "End");
}

void ExtensionAbilityThread::HandleCommandExtensionWindow(
    const Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::EXT, "Begin");
    if (extensionImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "null extensionImpl_");
        return;
    }
    extensionImpl_->CommandExtensionWindow(want, sessionInfo, winCmd);
    TAG_LOGD(AAFwkTag::EXT, "End");
}

void ExtensionAbilityThread::ScheduleUpdateConfiguration(const AppExecFwk::Configuration &config)
{
    TAG_LOGD(AAFwkTag::EXT, "Begin");
    HandleExtensionUpdateConfiguration(config);
    TAG_LOGD(AAFwkTag::EXT, "End");
}

void ExtensionAbilityThread::HandleExtensionUpdateConfiguration(const AppExecFwk::Configuration &config)
{
    TAG_LOGD(AAFwkTag::EXT, "Begin");
    if (extensionImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "null extensionImpl_");
        return;
    }
    extensionImpl_->ScheduleUpdateConfiguration(config);
    TAG_LOGD(AAFwkTag::EXT, "End");
}

bool ExtensionAbilityThread::ScheduleAbilityTransaction(
    const Want &want, const LifeCycleStateInfo &lifeCycleStateInfo, sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::EXT, "Name: %{public}s, targeState: %{public}d, isNewWant: %{public}d",
        want.GetElement().GetAbilityName().c_str(), lifeCycleStateInfo.state, lifeCycleStateInfo.isNewWant);
    if (token_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "null token_");
        return false;
    }
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "null abilityHandler_");
        return false;
    }
    wptr<ExtensionAbilityThread> weak = this;
    auto task = [weak, want, lifeCycleStateInfo, sessionInfo]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "null abilityThread");
            return;
        }
        abilityThread->HandleExtensionTransaction(want, lifeCycleStateInfo, sessionInfo);
    };
    bool ret = abilityHandler_->PostTask(task, AppExecFwk::EventQueue::Priority::HIGH);
    if (!ret) {
        TAG_LOGE(AAFwkTag::EXT, "PostTask error");
        return false;
    }
    return true;
}

void ExtensionAbilityThread::ScheduleConnectAbility(const Want &want)
{
    TAG_LOGD(AAFwkTag::EXT, "called");
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "null abilityHandler_");
        return;
    }
    FreezeUtil::GetInstance().AddLifecycleEvent(token_,
        "ExtensionAbilityThread::ScheduleConnectAbility");
    wptr<ExtensionAbilityThread> weak = this;
    auto task = [weak, want]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "null abilityThread");
            return;
        }
        abilityThread->HandleConnectExtension(want);
    };
    bool ret = abilityHandler_->PostTask(task, AppExecFwk::EventQueue::Priority::HIGH);
    if (!ret) {
        TAG_LOGE(AAFwkTag::EXT, "PostTask error");
    }
}

void ExtensionAbilityThread::ScheduleDisconnectAbility(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::EXT, "called");
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "null abilityHandler_");
        return;
    }
    wptr<ExtensionAbilityThread> weak = this;
    auto task = [weak, want]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "null abilityThread");
            return;
        }
        abilityThread->HandleDisconnectExtension(want);
    };
    bool ret = abilityHandler_->PostTask(task, AppExecFwk::EventQueue::Priority::HIGH);
    if (!ret) {
        TAG_LOGE(AAFwkTag::EXT, "PostTask error");
    }
}

void ExtensionAbilityThread::ScheduleCommandAbility(const Want &want, bool restart, int32_t startId)
{
    TAG_LOGD(AAFwkTag::EXT, "Begin startId: %{public}d", startId);
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "null abilityHandler_");
        return;
    }
    ScheduleCommandAbilityInner(want, restart, startId);
    if (AppExecFwk::InsightIntentExecuteParam::IsInsightIntentExecute(want)) {
        ScheduleInsightIntentInner(want);
    } else if (AppExecFwk::SkillExecuteParam::IsSkillExecute(want)) {
        ScheduleSkillExecuteInner(want);
    }
    TAG_LOGD(AAFwkTag::EXT, "End");
}

void ExtensionAbilityThread::ScheduleCommandAbilityInner(const Want &want, bool restart, int32_t startId)
{
    wptr<ExtensionAbilityThread> weak = this;
    auto task = [weak, want, restart, startId]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "null AbilityThread");
            return;
        }
        abilityThread->HandleCommandExtension(want, restart, startId);
    };
    bool ret = abilityHandler_->PostTask(task, AppExecFwk::EventQueue::Priority::HIGH);
    if (!ret) {
        TAG_LOGE(AAFwkTag::EXT, "PostTask error");
    }
}

void ExtensionAbilityThread::ScheduleInsightIntentInner(const Want &want)
{
    wptr<ExtensionAbilityThread> weak = this;
    auto task = [weak, want]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "null AbilityThread");
            return;
        }
        abilityThread->HandleInsightIntent(want);
    };
    bool ret = abilityHandler_->PostTask(task, AppExecFwk::EventQueue::Priority::HIGH);
    if (!ret) {
        TAG_LOGE(AAFwkTag::EXT, "PostTask error");
    }
}

void ExtensionAbilityThread::ScheduleSkillExecuteInner(const Want &want)
{
    wptr<ExtensionAbilityThread> weak = this;
    auto task = [weak, want]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "null AbilityThread");
            return;
        }
        abilityThread->HandleExecuteSkill(want);
    };
    bool ret = abilityHandler_->PostTask(task, AppExecFwk::EventQueue::Priority::HIGH);
    if (!ret) {
        TAG_LOGE(AAFwkTag::EXT, "PostTask error");
    }
}

void ExtensionAbilityThread::ExecuteSkill(const Want &want)
{
    TAG_LOGD(AAFwkTag::EXT, "ExecuteSkill called");
    ScheduleSkillExecuteInner(want);
}

void ExtensionAbilityThread::ScheduleCommandAbilityWindow(
    const Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd)
{
    TAG_LOGD(AAFwkTag::EXT, "Begin, winCmd: %{public}d", winCmd);
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "null abilityHandler_");
        return;
    }
    wptr<ExtensionAbilityThread> weak = this;
    auto task = [weak, want, sessionInfo, winCmd]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "null abilityThread");
            return;
        }
        abilityThread->HandleCommandExtensionWindow(want, sessionInfo, winCmd);
    };
    bool ret = abilityHandler_->PostTask(task, AppExecFwk::EventQueue::Priority::HIGH);
    if (!ret) {
        TAG_LOGE(AAFwkTag::EXT, "PostTask error");
    }
    TAG_LOGD(AAFwkTag::EXT, "End");
}

void ExtensionAbilityThread::SendResult(int requestCode, int resultCode, const Want &want)
{
    TAG_LOGD(AAFwkTag::EXT, "Begin");
    if (abilityHandler_ == nullptr || requestCode == -1) {
        TAG_LOGE(AAFwkTag::EXT, "null abilityHandler_ or requestCode -1");
        return;
    }

    wptr<ExtensionAbilityThread> weak = this;
    auto task = [weak, requestCode, resultCode, want]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "null abilityThread");
            return;
        }

        if (abilityThread->extensionImpl_ == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "null extensionImpl_");
            return;
        }
        abilityThread->extensionImpl_->SendResult(requestCode, resultCode, want);
    };
    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        TAG_LOGE(AAFwkTag::EXT, "PostTask error");
    }
    TAG_LOGD(AAFwkTag::EXT, "End");
}

void ExtensionAbilityThread::ScheduleAbilitiesRequestDone(const std::string &requestKey,
    const int32_t resultCode)
{
    TAG_LOGD(AAFwkTag::EXT, "ScheduleAbilitiesRequestDone");
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "null abilityHandler_");
        return;
    }

    wptr<ExtensionAbilityThread> weak = this;
    auto task = [weak, requestKey, resultCode]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "null abilityThread");
            return;
        }

        JsStartAbilitiesObserver::GetInstance().HandleFinished(requestKey, resultCode);
        StartAbilitiesObserver::HandleFinished(requestKey, resultCode);
    };
    abilityHandler_->PostTask(task);
}

void ExtensionAbilityThread::NotifyMemoryLevel(int32_t level)
{
    TAG_LOGD(AAFwkTag::EXT, "result: %{public}d", level);
    if (extensionImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "null extensionImpl_");
        return;
    }
    extensionImpl_->NotifyMemoryLevel(level);
}

void ExtensionAbilityThread::DumpAbilityInfo(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    TAG_LOGD(AAFwkTag::EXT, "Begin");
    if (token_ == nullptr || abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "null token_ or abilityHandler_");
        return;
    }
    wptr<ExtensionAbilityThread> weak = this;
    auto task = [weak, params, token = token_]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "null abilityThread");
            return;
        }
        std::vector<std::string> dumpInfo;
        abilityThread->DumpAbilityInfoInner(params, dumpInfo);
        ErrCode err = AbilityManagerClient::GetInstance()->DumpAbilityInfoDone(dumpInfo, token);
        if (err != ERR_OK) {
            TAG_LOGE(AAFwkTag::EXT, "Dump ability info err: %{public}d", err);
        }
    };
    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        TAG_LOGE(AAFwkTag::EXT, "PostTask error");
    }
    TAG_LOGD(AAFwkTag::EXT, "End");
}

void ExtensionAbilityThread::ScheduleAbilityRequestFailure(const std::string &requestId,
    const AppExecFwk::ElementName &element, const std::string &message, int32_t resultCode)
{
    TAG_LOGD(AAFwkTag::EXT, "ExtensionAbilityThread::ScheduleAbilityRequestFailure called");
    if (extensionImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "null extensionImpl_");
        return;
    }
    wptr<ExtensionAbilityThread> weak = this;
    auto task = [weak, requestId, element, message, resultCode]() {
        auto extensionAbilityThread = weak.promote();
        if (extensionAbilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "null extensionAbilityThread");
            return;
        }
        if (extensionAbilityThread->extensionImpl_ != nullptr) {
            extensionAbilityThread->extensionImpl_->ScheduleAbilityRequestFailure(
                requestId, element, message, resultCode);
            return;
        }
    };
    bool ret = abilityHandler_->PostTask(task, "ExtensionAbilityThread:ScheduleAbilityRequestFailure");
    if (!ret) {
        TAG_LOGE(AAFwkTag::EXT, "postTask error");
    }
}

void ExtensionAbilityThread::ScheduleAbilityRequestSuccess(const std::string &requestId,
    const AppExecFwk::ElementName &element)
{
    TAG_LOGD(AAFwkTag::EXT, "ExtensionAbilityThread::ScheduleAbilityRequestSuccess called");
    if (extensionImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "null extensionImpl_");
        return;
    }
    wptr<ExtensionAbilityThread> weak = this;
    auto task = [weak, requestId, element]() {
        auto extensionAbilityThread = weak.promote();
        if (extensionAbilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "null extensionAbilityThread");
            return;
        }
        if (extensionAbilityThread->extensionImpl_ != nullptr) {
            extensionAbilityThread->extensionImpl_->ScheduleAbilityRequestSuccess(requestId, element);
        }
    };
    bool ret = abilityHandler_->PostTask(task, "ExtensionAbilityThread:ScheduleAbilityRequestSuccess");
    if (!ret) {
        TAG_LOGE(AAFwkTag::EXT, "postTask error");
    }
}

void ExtensionAbilityThread::DumpAbilityInfoInner(
    const std::vector<std::string> &params, std::vector<std::string> &info)
{
    TAG_LOGD(AAFwkTag::EXT, "Begin");
    if (currentExtension_ == nullptr) {
        TAG_LOGD(AAFwkTag::EXT, "null currentExtension_");
        return;
    }
    currentExtension_->Dump(params, info);

#ifdef SUPPORT_GRAPHICS
    if (params.empty()) {
        DumpOtherInfo(info);
        return;
    }
#else
    DumpOtherInfo(info);
#endif
    TAG_LOGD(AAFwkTag::EXT, "End");
}

void ExtensionAbilityThread::DumpOtherInfo(std::vector<std::string> &info)
{
    std::string dumpInfo = "        event:";
    info.push_back(dumpInfo);
    if (abilityHandler_ == nullptr) {
        TAG_LOGD(AAFwkTag::EXT, "null abilityHandler_");
        return;
    }
    auto runner = abilityHandler_->GetEventRunner();
    if (runner == nullptr) {
        TAG_LOGD(AAFwkTag::EXT, "null runner_");
        return;
    }
    dumpInfo = "";
    runner->DumpRunnerInfo(dumpInfo);
    info.push_back(dumpInfo);
}

int ExtensionAbilityThread::CreateModalUIExtension(const Want &want)
{
    if (extensionImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "null extensionImpl_");
        return ERR_INVALID_VALUE;
    }

    return extensionImpl_->CreateModalUIExtension(want);
}
} // namespace AbilityRuntime
} // namespace OHOS
