/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "fa_ability_thread.h"

#include <chrono>
#include <functional>
#include <thread>

#include "ability_context_impl.h"
#include "ability_impl.h"
#include "ability_impl_factory.h"
#include "ability_loader.h"
#include "abs_shared_result_set.h"
#include "application_impl.h"
#include "context_deal.h"
#include "data_ability_predicates.h"
#include "dataobs_mgr_client.h"
#ifdef WITH_DLP
#include "dlp_file_kits.h"
#endif // WITH_DLP
#include "freeze_util.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "ohos_application.h"
#ifdef SUPPORT_GRAPHICS
#include "page_ability_impl.h"
#endif
#include "time_util.h"
#include "ui_extension_wrapper.h"
#include "values_bucket.h"

namespace OHOS {
using AbilityRuntime::FreezeUtil;
namespace AbilityRuntime {
using namespace std::chrono_literals;
using AbilityManagerClient = OHOS::AAFwk::AbilityManagerClient;
using DataObsMgrClient = OHOS::AAFwk::DataObsMgrClient;
namespace {
constexpr static char ACE_SERVICE_ABILITY_NAME[] = "AceServiceAbility";
constexpr static char ACE_DATA_ABILITY_NAME[] = "AceDataAbility";
#ifdef SUPPORT_GRAPHICS
constexpr static char ABILITY_NAME[] = "Ability";
constexpr static char ACE_ABILITY_NAME[] = "AceAbility";
constexpr static char ACE_FORM_ABILITY_NAME[] = "AceFormAbility";
constexpr static char FORM_EXTENSION[] = "FormExtension";
constexpr static char UI_EXTENSION[] = "UIExtensionAbility";
constexpr static char CUSTOM_EXTENSION[] = "ExtensionAbility";
constexpr static char MEDIA_CONTROL_EXTENSION[] = "MediaControlExtensionAbility";
constexpr static char USER_AUTH_EXTENSION[] = "UserAuthExtensionAbility";
constexpr static char ACTION_EXTENSION[] = "ActionExtensionAbility";
constexpr static char SHARE_EXTENSION[] = "ShareExtensionAbility";
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
constexpr static char CALLER_INFO_QUERY_EXTENSION[] = "CallerInfoQueryExtension";
#ifdef WITH_DLP
constexpr static char DLP_PARAMS_SANDBOX[] = "ohos.dlp.params.sandbox";
#endif // WITH_DLP
const int32_t PREPARE_TO_TERMINATE_TIMEOUT_MILLISECONDS = 3000;
}

FAAbilityThread::FAAbilityThread() : abilityImpl_(nullptr), currentAbility_(nullptr) {}

FAAbilityThread::~FAAbilityThread()
{
    if (isExtension_) {
        if (currentExtension_) {
            currentExtension_.reset();
        }
    } else {
        if (currentAbility_) {
            currentAbility_->DetachBaseContext();
            currentAbility_.reset();
        }
    }

    DelayedSingleton<AppExecFwk::AbilityImplFactory>::DestroyInstance();
}

std::string FAAbilityThread::CreateAbilityName(const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
    const std::shared_ptr<AppExecFwk::OHOSApplication> &application)
{
    std::string abilityName;
    if (abilityRecord == nullptr || application == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityRecord or application");
        return abilityName;
    }

    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = abilityRecord->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityInfo");
        return abilityName;
    }

    if (abilityInfo->isNativeAbility) {
        TAG_LOGD(AAFwkTag::FA, "AbilityInfo name is %{public}s", abilityInfo->name.c_str());
        return abilityInfo->name;
    }
#ifdef SUPPORT_GRAPHICS
    if (abilityInfo->type == AppExecFwk::AbilityType::PAGE) {
        if (abilityInfo->isStageBasedModel) {
            abilityName = ABILITY_NAME;
        } else {
            abilityName = ACE_ABILITY_NAME;
        }
    } else if (abilityInfo->type == AppExecFwk::AbilityType::SERVICE) {
#else
    if (abilityInfo->type == AppExecFwk::AbilityType::SERVICE) {
#endif
#ifdef SUPPORT_GRAPHICS
        if (abilityInfo->formEnabled == true) {
            abilityName = ACE_FORM_ABILITY_NAME;
        } else {
#endif
            abilityName = ACE_SERVICE_ABILITY_NAME;
#ifdef SUPPORT_GRAPHICS
        }
#endif
    } else if (abilityInfo->type == AppExecFwk::AbilityType::DATA) {
        abilityName = ACE_DATA_ABILITY_NAME;
    } else if (abilityInfo->type == AppExecFwk::AbilityType::EXTENSION) {
        CreateExtensionAbilityName(application, abilityInfo, abilityName);
    } else {
        abilityName = abilityInfo->name;
    }

    TAG_LOGD(AAFwkTag::FA, "ability name is %{public}s", abilityName.c_str());
    return abilityName;
}

void FAAbilityThread::CreateExtensionAbilityName(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo, std::string &abilityName)
{
    application->GetExtensionNameByType(static_cast<int32_t>(abilityInfo->extensionAbilityType), abilityName);
    if (abilityName.length() > 0) {
        TAG_LOGD(AAFwkTag::FA, "extension name: %{public}s", abilityName.c_str());
        return;
    }
    abilityName = BASE_SERVICE_EXTENSION;
    if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::STATICSUBSCRIBER) {
        abilityName = STATIC_SUBSCRIBER_EXTENSION;
    }
    if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::DRIVER) {
        abilityName = BASE_DRIVER_EXTENSION;
    }
    if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::DATASHARE) {
        abilityName = DATA_SHARE_EXT_ABILITY;
    }
    if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::WORK_SCHEDULER) {
        abilityName = WORK_SCHEDULER_EXTENSION;
    }
    if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::ACCESSIBILITY) {
        abilityName = ACCESSIBILITY_EXTENSION;
    }
    if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::WALLPAPER) {
        abilityName = WALLPAPER_EXTENSION;
    }
    if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::FILEACCESS_EXTENSION) {
        abilityName = FILEACCESS_EXT_ABILITY;
    }
    if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::ENTERPRISE_ADMIN) {
        abilityName = ENTERPRISE_ADMIN_EXTENSION;
    }
    if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::INPUTMETHOD) {
        abilityName = INPUTMETHOD_EXTENSION;
    }
    if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::APP_ACCOUNT_AUTHORIZATION) {
        abilityName = APP_ACCOUNT_AUTHORIZATION_EXTENSION;
    }
    if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::CALLER_INFO_QUERY) {
        abilityName = CALLER_INFO_QUERY_EXTENSION;
    }
    CreateExtensionAbilityNameSupportGraphics(abilityInfo, abilityName);
    TAG_LOGD(AAFwkTag::FA, "extension abilityName: %{public}s", abilityName.c_str());
}

void FAAbilityThread::CreateExtensionAbilityNameSupportGraphics(
    const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo,
    std::string &abilityName)
{
#ifdef SUPPORT_GRAPHICS
    if (abilityInfo->formEnabled || abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::FORM) {
        abilityName = FORM_EXTENSION;
    }

    if (AAFwk::UIExtensionWrapper::IsUIExtension(abilityInfo->extensionAbilityType)) {
        if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::SHARE) {
            abilityName = SHARE_EXTENSION;
        } else if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::ACTION) {
            abilityName = ACTION_EXTENSION;
        } else {
            abilityName = UI_EXTENSION;
        }
    }
    if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::SYSDIALOG_USERAUTH) {
        abilityName = USER_AUTH_EXTENSION;
    }
    if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::SYSPICKER_MEDIACONTROL) {
        abilityName = MEDIA_CONTROL_EXTENSION;
    }
    if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::UNSPECIFIED &&
        abilityInfo->type == AppExecFwk::AbilityType::EXTENSION) {
        abilityName = abilityInfo->extensionTypeName + CUSTOM_EXTENSION;
    }
#endif
}

std::shared_ptr<AppExecFwk::ContextDeal> FAAbilityThread::CreateAndInitContextDeal(
    const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
    const std::shared_ptr<AppExecFwk::AbilityContext> &abilityObject)
{
    TAG_LOGD(AAFwkTag::FA, "begin");
    std::shared_ptr<AppExecFwk::ContextDeal> contextDeal = nullptr;
    if ((application == nullptr) || (abilityRecord == nullptr) || (abilityObject == nullptr)) {
        TAG_LOGE(AAFwkTag::FA, "null application or abilityRecord or abilityObject");
        return contextDeal;
    }

    contextDeal = std::make_shared<AppExecFwk::ContextDeal>();
    contextDeal->SetAbilityInfo(abilityRecord->GetAbilityInfo());
    contextDeal->SetApplicationInfo(application->GetApplicationInfo());
    abilityObject->SetProcessInfo(application->GetProcessInfo());

    std::shared_ptr<AppExecFwk::Context> tmpContext = application->GetApplicationContext();
    contextDeal->SetApplicationContext(tmpContext);

    contextDeal->SetBundleCodePath(abilityRecord->GetAbilityInfo()->codePath);
    contextDeal->SetContext(abilityObject);
    return contextDeal;
}

void FAAbilityThread::Attach(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
    const std::shared_ptr<AppExecFwk::EventRunner> &mainRunner, const std::shared_ptr<Context> &stageContext)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if ((application == nullptr) || (abilityRecord == nullptr) || (mainRunner == nullptr)) {
        TAG_LOGE(AAFwkTag::FA, "null application or abilityRecord or mainRunner");
        return;
    }
    InitExtensionFlag(abilityRecord);
    if (isExtension_) {
        AttachExtension(application, abilityRecord, mainRunner);
        TAG_LOGD(AAFwkTag::FA, "Execute AttachExtension");
        return;
    }

    // 1.new AbilityHandler
    std::string abilityName = CreateAbilityName(abilityRecord, application);
    if (abilityName.empty()) {
        TAG_LOGE(AAFwkTag::FA, "abilityName empty");
        return;
    }
    TAG_LOGD(AAFwkTag::FA, "begin ability: %{public}s", abilityRecord->GetAbilityInfo()->name.c_str());
    abilityHandler_ = std::make_shared<AppExecFwk::AbilityHandler>(mainRunner);
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityHandler_");
        return;
    }

    // 2.new ability
    auto ability = AppExecFwk::AbilityLoader::GetInstance().GetAbilityByName(abilityName);
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null ability");
        return;
    }
    currentAbility_.reset(ability);
    token_ = abilityRecord->GetToken();
    abilityRecord->SetAbilityThread(this);
    std::shared_ptr<AppExecFwk::AbilityContext> abilityObject = currentAbility_;
    std::shared_ptr<AppExecFwk::ContextDeal> contextDeal =
        CreateAndInitContextDeal(application, abilityRecord, abilityObject);
    ability->AttachBaseContext(contextDeal);
    // new hap requires
    ability->AttachAbilityContext(
        BuildAbilityContext(abilityRecord->GetAbilityInfo(), application, token_, stageContext));

    AttachInner(application, abilityRecord, stageContext);
}

void FAAbilityThread::AttachInner(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
    const std::shared_ptr<Context> &stageContext)
{
    // 3.new abilityImpl
    abilityImpl_ = DelayedSingleton<AppExecFwk::AbilityImplFactory>::GetInstance()->MakeAbilityImplObject(
        abilityRecord->GetAbilityInfo());
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityImpl_");
        return;
    }
    abilityImpl_->Init(application, abilityRecord, currentAbility_, abilityHandler_, token_);
    // 4. ability attach : ipc
    TAG_LOGD(AAFwkTag::FA, "attach ability");
    std::string entry = "AbilityThread::Attach";
    FreezeUtil::GetInstance().AddLifecycleEvent(token_, entry);
    ErrCode err = AbilityManagerClient::GetInstance()->AttachAbilityThread(this, token_);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::FA, "err = %{public}d", err);
        return;
    }
    FreezeUtil::GetInstance().DeleteLifecycleEvent(token_);
    FreezeUtil::GetInstance().DeleteAppLifecycleEvent(0);
}

void FAAbilityThread::AttachExtension(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
    const std::shared_ptr<AppExecFwk::EventRunner> &mainRunner)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if ((application == nullptr) || (abilityRecord == nullptr) || (mainRunner == nullptr)) {
        TAG_LOGE(AAFwkTag::FA, "null application or abilityRecord or mainRunner");
        return;
    }

    // 1.new AbilityHandler
    std::string abilityName = CreateAbilityName(abilityRecord, application);
    if (abilityName.empty()) {
        TAG_LOGE(AAFwkTag::FA, "empty abilityName");
        return;
    }
    TAG_LOGD(AAFwkTag::FA, "extension: %{public}s", abilityRecord->GetAbilityInfo()->name.c_str());
    abilityHandler_ = std::make_shared<AppExecFwk::AbilityHandler>(mainRunner);
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityHandler_");
        return;
    }

    // 2.new ability
    auto extension = AppExecFwk::AbilityLoader::GetInstance().GetExtensionByName(abilityName);
    if (extension == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null extension");
        return;
    }

    currentExtension_.reset(extension);
    token_ = abilityRecord->GetToken();
    abilityRecord->SetAbilityThread(this);
    extensionImpl_ = std::make_shared<ExtensionImpl>();
    if (extensionImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null extensionImpl_");
        return;
    }
    // 3.new init
    extensionImpl_->Init(application, abilityRecord, currentExtension_, abilityHandler_, token_);
    // 4.ipc attach init
    ErrCode err = AbilityManagerClient::GetInstance()->AttachAbilityThread(this, token_);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::FA, "err = %{public}d", err);
    }
}

void FAAbilityThread::AttachExtension(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::FA, "begin");
    if ((application == nullptr) || (abilityRecord == nullptr)) {
        TAG_LOGE(AAFwkTag::FA, "null application or abilityRecord");
        return;
    }

    // 1.new AbilityHandler
    std::string abilityName = CreateAbilityName(abilityRecord, application);
    runner_ = AppExecFwk::EventRunner::Create(abilityName);
    if (runner_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null runner");
        return;
    }
    abilityHandler_ = std::make_shared<AppExecFwk::AbilityHandler>(runner_);
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityHandler_");
        return;
    }

    // 2.new ability
    auto extension = AppExecFwk::AbilityLoader::GetInstance().GetExtensionByName(abilityName);
    if (extension == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null extension");
        return;
    }

    currentExtension_.reset(extension);
    token_ = abilityRecord->GetToken();
    abilityRecord->SetAbilityThread(this);
    extensionImpl_ = std::make_shared<ExtensionImpl>();
    if (extensionImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null extensionImpl_");
        return;
    }
    // 3.new init
    extensionImpl_->Init(application, abilityRecord, currentExtension_, abilityHandler_, token_);
    // 4.ipc attach init
    ErrCode err = AbilityManagerClient::GetInstance()->AttachAbilityThread(this, token_);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::FA, "failed err = %{public}d", err);
        return;
    }
}

void FAAbilityThread::Attach(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord, const std::shared_ptr<Context> &stageContext)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::FA, "begin");
    if ((application == nullptr) || (abilityRecord == nullptr)) {
        TAG_LOGE(AAFwkTag::FA, "null application or abilityRecord");
        return;
    }
    InitExtensionFlag(abilityRecord);
    if (isExtension_) {
        AttachExtension(application, abilityRecord);
        TAG_LOGD(AAFwkTag::FA, "Execute AttachExtension");
        return;
    }

    // 1.new AbilityHandler
    std::string abilityName = CreateAbilityName(abilityRecord, application);
    runner_ = AppExecFwk::EventRunner::Create(abilityName);
    if (runner_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null runner_");
        return;
    }
    abilityHandler_ = std::make_shared<AppExecFwk::AbilityHandler>(runner_);
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityHandler_");
        return;
    }

    // 2.new ability
    auto ability = AppExecFwk::AbilityLoader::GetInstance().GetAbilityByName(abilityName);
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null ability");
        return;
    }
    currentAbility_.reset(ability);
    token_ = abilityRecord->GetToken();
    abilityRecord->SetAbilityThread(this);
    std::shared_ptr<AppExecFwk::AbilityContext> abilityObject = currentAbility_;
    std::shared_ptr<AppExecFwk::ContextDeal> contextDeal =
        CreateAndInitContextDeal(application, abilityRecord, abilityObject);
    ability->AttachBaseContext(contextDeal);
    // new hap requires
    ability->AttachAbilityContext(
        BuildAbilityContext(abilityRecord->GetAbilityInfo(), application, token_, stageContext));

    AttachInner(application, abilityRecord, stageContext);
}

void FAAbilityThread::HandleAbilityTransaction(
    const Want &want, const LifeCycleStateInfo &lifeCycleStateInfo, sptr<AAFwk::SessionInfo> sessionInfo)
{
    std::string connector = "##";
    std::string traceName = __PRETTY_FUNCTION__ + connector + want.GetElement().GetAbilityName();
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, traceName);
    TAG_LOGD(AAFwkTag::FA, "Lifecycle: name is %{public}s.", want.GetElement().GetAbilityName().c_str());
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityImpl_");
        return;
    }
    std::string methodName = "HandleAbilityTransaction";
    AddLifecycleEvent(lifeCycleStateInfo.state, methodName);

    abilityImpl_->SetCallingContext(lifeCycleStateInfo.caller.deviceId, lifeCycleStateInfo.caller.bundleName,
        lifeCycleStateInfo.caller.abilityName, lifeCycleStateInfo.caller.moduleName);
    abilityImpl_->HandleAbilityTransaction(want, lifeCycleStateInfo, sessionInfo);
}

void FAAbilityThread::AddLifecycleEvent(uint32_t state, std::string &methodName) const
{
    if (!isUIAbility_) {
        return;
    }
    if (state == AAFwk::ABILITY_STATE_FOREGROUND_NEW) {
        std::string entry = "AbilityThread::" + methodName;
        FreezeUtil::GetInstance().AddLifecycleEvent(token_, entry);
    }
    if (state == AAFwk::ABILITY_STATE_BACKGROUND_NEW) {
        std::string entry = "AbilityThread::" + methodName;
        FreezeUtil::GetInstance().AddLifecycleEvent(token_, entry);
    }
}

void FAAbilityThread::HandleShareData(const int32_t &uniqueId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityImpl_");
        return;
    }
    abilityImpl_->HandleShareData(uniqueId);
}

void FAAbilityThread::HandleExtensionTransaction(
    const Want &want, const LifeCycleStateInfo &lifeCycleStateInfo, sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::FA, "begin");
    if (extensionImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null extensionImpl_");
        return;
    }
    extensionImpl_->HandleExtensionTransaction(want, lifeCycleStateInfo, sessionInfo);
}

void FAAbilityThread::HandleConnectAbility(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::FA, "begin");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityImpl_");
        return;
    }

    sptr<IRemoteObject> service = abilityImpl_->ConnectAbility(want);
    ErrCode err = AbilityManagerClient::GetInstance()->ScheduleConnectAbilityDone(token_, service);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::FA, "failed err = %{public}d", err);
    }
}

void FAAbilityThread::HandleDisconnectAbility(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::FA, "begin");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityImpl_");
        return;
    }

    abilityImpl_->DisconnectAbility(want);
    ErrCode err = AbilityManagerClient::GetInstance()->ScheduleDisconnectAbilityDone(token_);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::FA, "err = %{public}d", err);
    }
}

void FAAbilityThread::HandleCommandAbility(const Want &want, bool restart, int32_t startId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::FA, "begin");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityImpl_");
        return;
    }
    abilityImpl_->CommandAbility(want, restart, startId);
    ErrCode err = AbilityManagerClient::GetInstance()->ScheduleCommandAbilityDone(token_);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::FA, "failed err = %{public}d", err);
    }
}

void FAAbilityThread::HandleConnectExtension(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::FA, "begin");
    if (extensionImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null extensionImpl_");
        return;
    }
    bool isAsyncCallback = false;
    sptr<IRemoteObject> service = extensionImpl_->ConnectExtension(want, isAsyncCallback);
    if (!isAsyncCallback) {
        extensionImpl_->ConnectExtensionCallback(service);
    }
}

void FAAbilityThread::HandleDisconnectExtension(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::FA, "begin");
    if (extensionImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null extensionImpl_");
        return;
    }

    bool isAsyncCallback = false;
    extensionImpl_->DisconnectExtension(want, isAsyncCallback);
    if (!isAsyncCallback) {
        extensionImpl_->DisconnectExtensionCallback();
    }
}

void FAAbilityThread::HandleCommandExtension(const Want &want, bool restart, int32_t startId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::FA, "begin");
    if (extensionImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null extensionImpl_");
        return;
    }
    extensionImpl_->CommandExtension(want, restart, startId);
    ErrCode err = AbilityManagerClient::GetInstance()->ScheduleCommandAbilityDone(token_);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::FA, "failed err = %{public}d", err);
    }
}

void FAAbilityThread::HandleCommandExtensionWindow(
    const Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::FA, "begin");
    if (extensionImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null extensionImpl_");
        return;
    }
    extensionImpl_->CommandExtensionWindow(want, sessionInfo, winCmd);
}

void FAAbilityThread::HandleRestoreAbilityState(const AppExecFwk::PacMap &state)
{
    TAG_LOGD(AAFwkTag::FA, "begin");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityImpl_");
        return;
    }

    abilityImpl_->DispatchRestoreAbilityState(state);
}

void FAAbilityThread::ScheduleSaveAbilityState()
{
    TAG_LOGD(AAFwkTag::FA, "begin");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityImpl_");
        return;
    }

    abilityImpl_->DispatchSaveAbilityState();
}

void FAAbilityThread::ScheduleRestoreAbilityState(const AppExecFwk::PacMap &state)
{
    TAG_LOGD(AAFwkTag::FA, "begin");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityImpl_");
        return;
    }
    abilityImpl_->DispatchRestoreAbilityState(state);
}

void FAAbilityThread::ScheduleUpdateConfiguration(const AppExecFwk::Configuration &config)
{
    TAG_LOGD(AAFwkTag::FA, "begin");
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityHandler_");
        return;
    }
    wptr<FAAbilityThread> weak = this;
    auto task = [weak, config]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::FA, "null abilityThread");
            return;
        }

        if (abilityThread->isExtension_) {
            abilityThread->HandleExtensionUpdateConfiguration(config);
        } else {
            abilityThread->HandleUpdateConfiguration(config);
        }
    };
    bool ret = abilityHandler_->PostTask(task, "FAAbilityThread:UpdateConfiguration");
    if (!ret) {
        TAG_LOGE(AAFwkTag::FA, "PostTask error");
    }
}

void FAAbilityThread::HandleUpdateConfiguration(const AppExecFwk::Configuration &config)
{
    TAG_LOGD(AAFwkTag::FA, "begin");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityImpl_");
        return;
    }

    abilityImpl_->ScheduleUpdateConfiguration(config);
}

void FAAbilityThread::HandleExtensionUpdateConfiguration(const AppExecFwk::Configuration &config)
{
    TAG_LOGD(AAFwkTag::FA, "begin");
    if (extensionImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null extensionImpl_");
        return;
    }

    extensionImpl_->ScheduleUpdateConfiguration(config);
}

bool FAAbilityThread::ScheduleAbilityTransaction(
    const Want &want, const LifeCycleStateInfo &lifeCycleStateInfo, sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::FA, "Lifecycle: name:%{public}s,targeState:%{public}d,isNewWant:%{public}d",
        want.GetElement().GetAbilityName().c_str(),
        lifeCycleStateInfo.state,
        lifeCycleStateInfo.isNewWant);
    std::string methodName = "ScheduleAbilityTransaction";
    AddLifecycleEvent(lifeCycleStateInfo.state, methodName);

    if (token_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null token_");
        return false;
    }
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityHandler_");
        return false;
    }
    wptr<FAAbilityThread> weak = this;
    auto task = [weak, want, lifeCycleStateInfo, sessionInfo]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::FA, "null abilityThread");
            return;
        }
        if (abilityThread->isExtension_) {
            abilityThread->HandleExtensionTransaction(want, lifeCycleStateInfo, sessionInfo);
            Want newWant(want);
            newWant.CloseAllFd();
        } else {
            abilityThread->HandleAbilityTransaction(want, lifeCycleStateInfo, sessionInfo);
        }
    };
    bool ret = abilityHandler_->PostTask(task, "FAAbilityThread:AbilityTransaction");
    if (!ret) {
        TAG_LOGE(AAFwkTag::FA, "PostTask error");
        return false;
    }
    return true;
}

void FAAbilityThread::ScheduleShareData(const int32_t &uniqueId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (token_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null token_");
        return;
    }
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityHandler_");
        return;
    }
    wptr<FAAbilityThread> weak = this;
    auto task = [weak, uniqueId]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::FA, "null abilityThread");
            return;
        }
        abilityThread->HandleShareData(uniqueId);
    };
    bool ret = abilityHandler_->PostTask(task, "FAAbilityThread:ShareData");
    if (!ret) {
        TAG_LOGE(AAFwkTag::FA, "postTask error");
    }
}

void FAAbilityThread::ScheduleConnectAbility(const Want &want)
{
    TAG_LOGD(AAFwkTag::FA, "begin, isExtension_: %{public}d", isExtension_);
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityHandler_");
        return;
    }
    wptr<FAAbilityThread> weak = this;
    auto task = [weak, want]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::FA, "null abilityThread");
            return;
        }
        if (abilityThread->isExtension_) {
            abilityThread->HandleConnectExtension(want);
        } else {
            abilityThread->HandleConnectAbility(want);
        }
    };
    bool ret = abilityHandler_->PostTask(task, "FAAbilityThread:ConnectAbility");
    if (!ret) {
        TAG_LOGE(AAFwkTag::FA, "PostTask error");
    }
}

void FAAbilityThread::ScheduleDisconnectAbility(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::FA, "begin, isExtension: %{public}d", isExtension_);
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityHandler_");
        return;
    }
    wptr<FAAbilityThread> weak = this;
    auto task = [weak, want]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::FA, "null abilityThread");
            return;
        }
        if (abilityThread->isExtension_) {
            abilityThread->HandleDisconnectExtension(want);
        } else {
            abilityThread->HandleDisconnectAbility(want);
        }
    };
    bool ret = abilityHandler_->PostTask(task, "FAAbilityThread:DisconnectAbility");
    if (!ret) {
        TAG_LOGE(AAFwkTag::FA, "PostTask error");
    }
}

void FAAbilityThread::ScheduleCommandAbility(const Want &want, bool restart, int startId)
{
    TAG_LOGD(AAFwkTag::FA, "begin. startId: %{public}d", startId);
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityHandler_");
        return;
    }
    wptr<FAAbilityThread> weak = this;
    auto task = [weak, want, restart, startId]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::FA, "null abilityThread");
            return;
        }
        if (abilityThread->isExtension_) {
            Want newWant(want);
#ifdef WITH_DLP
            bool sandboxFlag = Security::DlpPermission::DlpFileKits::GetSandboxFlag(newWant);
            newWant.SetParam(DLP_PARAMS_SANDBOX, sandboxFlag);
            if (sandboxFlag) {
                newWant.CloseAllFd();
            }
#endif // WITH_DLP
            abilityThread->HandleCommandExtension(newWant, restart, startId);
            newWant.CloseAllFd();
        } else {
            abilityThread->HandleCommandAbility(want, restart, startId);
        }
    };
    bool ret = abilityHandler_->PostTask(task, "FAAbilityThread:CommandAbility");
    if (!ret) {
        TAG_LOGE(AAFwkTag::FA, "PostTask error");
    }
}

bool FAAbilityThread::SchedulePrepareTerminateAbility()
{
    TAG_LOGD(AAFwkTag::FA, "begin");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityImpl_");
        return false;
    }
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityHandler_");
        return false;
    }
    if (getpid() == gettid()) {
        bool ret = abilityImpl_->PrepareTerminateAbility();
        TAG_LOGD(AAFwkTag::FA, "end, ret = %{public}d", ret);
        return ret;
    }
    wptr<FAAbilityThread> weak = this;
    auto task = [weak]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::FA, "null abilityThread");
            return;
        }
        abilityThread->HandlePrepareTerminateAbility();
    };
    bool ret = abilityHandler_->PostTask(task, "FAAbilityThread:PrepareTerminateAbility");
    if (!ret) {
        TAG_LOGE(AAFwkTag::FA, "PostTask error");
        return false;
    }

    std::unique_lock<std::mutex> lock(mutex_);
    auto condition = [weak] {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::FA, "null abilityThread");
            return false;
        }
        return abilityThread->isPrepareTerminateAbilityDone_.load();
    };
    if (!cv_.wait_for(lock, std::chrono::milliseconds(PREPARE_TO_TERMINATE_TIMEOUT_MILLISECONDS), condition)) {
        TAG_LOGW(AAFwkTag::FA, "Wait timeout");
    }
    TAG_LOGD(AAFwkTag::FA, "end, ret = %{public}d", isPrepareTerminate_);
    return isPrepareTerminate_;
}

void FAAbilityThread::ScheduleCommandAbilityWindow(
    const Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd)
{
    TAG_LOGD(AAFwkTag::FA, "begin");
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityHandler_");
        return;
    }
    wptr<FAAbilityThread> weak = this;
    auto task = [weak, want, sessionInfo, winCmd]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::FA, "null abilityThread");
            return;
        }
        abilityThread->HandleCommandExtensionWindow(want, sessionInfo, winCmd);
    };
    bool ret = abilityHandler_->PostTask(task, "FAAbilityThread:CommandAbilityWindow");
    if (!ret) {
        TAG_LOGE(AAFwkTag::FA, "PostTask error");
    }
}

void FAAbilityThread::SendResult(int requestCode, int resultCode, const Want &want)
{
    TAG_LOGD(AAFwkTag::FA, "begin");
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityHandler_");
        return;
    }
    wptr<FAAbilityThread> weak = this;
    auto task = [weak, requestCode, resultCode, want]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::FA, "null abilityThread");
            return;
        }
        if (requestCode == -1) {
            TAG_LOGE(AAFwkTag::FA, "requestCode is -1");
            return;
        }
        if (abilityThread->isExtension_ && abilityThread->extensionImpl_ != nullptr) {
            abilityThread->extensionImpl_->SendResult(requestCode, resultCode, want);
            return;
        }
        if (!abilityThread->isExtension_ && abilityThread->abilityImpl_ != nullptr) {
            abilityThread->abilityImpl_->SendResult(requestCode, resultCode, want);
            return;
        }
        TAG_LOGE(AAFwkTag::FA, "null %{public}s impl", abilityThread->isExtension_ ? "extension" : "ability");
    };
    bool ret = abilityHandler_->PostTask(task, "FAAbilityThread:SendResult");
    if (!ret) {
        TAG_LOGE(AAFwkTag::FA, "PostTask error");
    }
}

std::vector<std::string> FAAbilityThread::GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter)
{
    TAG_LOGD(AAFwkTag::FA, "begin");
    std::vector<std::string> types;
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityImpl_");
        return types;
    }

    types = abilityImpl_->GetFileTypes(uri, mimeTypeFilter);
    return types;
}

int FAAbilityThread::OpenFile(const Uri &uri, const std::string &mode)
{
    TAG_LOGD(AAFwkTag::FA, "called");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityImpl_");
        return -1;
    }
    return abilityImpl_->OpenFile(uri, mode);
}

int FAAbilityThread::OpenRawFile(const Uri &uri, const std::string &mode)
{
    TAG_LOGD(AAFwkTag::FA, "called");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityImpl_");
        return -1;
    }

    return abilityImpl_->OpenRawFile(uri, mode);
}

int FAAbilityThread::Insert(const Uri &uri, const NativeRdb::ValuesBucket &value)
{
    TAG_LOGD(AAFwkTag::FA, "called");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityImpl_");
        return -1;
    }

    return abilityImpl_->Insert(uri, value);
}

std::shared_ptr<AppExecFwk::PacMap> FAAbilityThread::Call(
    const Uri &uri, const std::string &method, const std::string &arg, const AppExecFwk::PacMap &pacMap)
{
    TAG_LOGD(AAFwkTag::FA, "called");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityImpl_");
        return nullptr;
    }

    return abilityImpl_->Call(uri, method, arg, pacMap);
}

int FAAbilityThread::Update(
    const Uri &uri, const NativeRdb::ValuesBucket &value, const NativeRdb::DataAbilityPredicates &predicates)
{
    TAG_LOGD(AAFwkTag::FA, "called");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityImpl_");
        return -1;
    }

    return abilityImpl_->Update(uri, value, predicates);
}

int FAAbilityThread::Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
{
    TAG_LOGD(AAFwkTag::FA, "called");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityImpl_");
        return -1;
    }
    return abilityImpl_->Delete(uri, predicates);
}

std::shared_ptr<NativeRdb::AbsSharedResultSet> FAAbilityThread::Query(
    const Uri &uri, std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
{
    TAG_LOGD(AAFwkTag::FA, "called");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityImpl_");
        return nullptr;
    }

    return abilityImpl_->Query(uri, columns, predicates);
}

std::string FAAbilityThread::GetType(const Uri &uri)
{
    TAG_LOGD(AAFwkTag::FA, "called");
    std::string type;
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityImpl_");
        return "";
    }

    return abilityImpl_->GetType(uri);
}

bool FAAbilityThread::Reload(const Uri &uri, const AppExecFwk::PacMap &extras)
{
    TAG_LOGD(AAFwkTag::FA, "called");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityImpl_");
        return false;
    }
    return abilityImpl_->Reload(uri, extras);
}

int FAAbilityThread::BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values)
{
    TAG_LOGD(AAFwkTag::FA, "called");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityImpl_");
        return -1;
    }

    return abilityImpl_->BatchInsert(uri, values);
}

void FAAbilityThread::ContinueAbility(const std::string &deviceId, uint32_t versionCode)
{
    TAG_LOGD(AAFwkTag::FA, "begin");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityImpl_");
        return;
    }
    abilityImpl_->ContinueAbility(deviceId, versionCode);
}

void FAAbilityThread::NotifyContinuationResult(int32_t result)
{
    TAG_LOGD(AAFwkTag::FA, "begin, result: %{public}d", result);
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityImpl_");
        return;
    }
    abilityImpl_->NotifyContinuationResult(result);
}

void FAAbilityThread::NotifyMemoryLevel(int32_t level)
{
    TAG_LOGD(AAFwkTag::FA, "begin, result: %{public}d", level);
    if (isExtension_) {
        if (extensionImpl_ == nullptr) {
            TAG_LOGE(AAFwkTag::FA, "null extensionImpl_");
            return;
        }
        extensionImpl_->NotifyMemoryLevel(level);
        return;
    }
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityImpl_");
        return;
    }
    abilityImpl_->NotifyMemoryLevel(level);
}

void FAAbilityThread::InitExtensionFlag(const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord)
{
    TAG_LOGD(AAFwkTag::FA, "begin");
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityRecord");
        return;
    }
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = abilityRecord->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null ability");
        return;
    }
    if (abilityInfo->type == AppExecFwk::AbilityType::EXTENSION) {
        TAG_LOGD(AAFwkTag::FA, "InitExtensionFlag is true");
        isExtension_ = true;
    } else {
        isExtension_ = false;
    }
    if (abilityInfo->type == AppExecFwk::AbilityType::PAGE) {
        TAG_LOGD(AAFwkTag::FA, "isUIAbility_ is assigned true");
        isUIAbility_ = true;
    }
}

Uri FAAbilityThread::NormalizeUri(const Uri &uri)
{
    TAG_LOGD(AAFwkTag::FA, "begin");
    Uri urivalue("");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityImpl_");
        return urivalue;
    }

    urivalue = abilityImpl_->NormalizeUri(uri);
    return urivalue;
}

Uri FAAbilityThread::DenormalizeUri(const Uri &uri)
{
    TAG_LOGD(AAFwkTag::FA, "begin");
    Uri urivalue("");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityImpl");
        return urivalue;
    }

    urivalue = abilityImpl_->DenormalizeUri(uri);
    return urivalue;
}

bool FAAbilityThread::HandleRegisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    auto obsMgrClient = DataObsMgrClient::GetInstance();
    if (obsMgrClient == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null obsMgrClient");
        return false;
    }

    ErrCode ret = obsMgrClient->RegisterObserver(uri, dataObserver);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::FA, "error %{public}d", ret);
        return false;
    }
    return true;
}

bool FAAbilityThread::HandleUnregisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    auto obsMgrClient = DataObsMgrClient::GetInstance();
    if (obsMgrClient == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null obsMgrClient");
        return false;
    }

    ErrCode ret = obsMgrClient->UnregisterObserver(uri, dataObserver);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::FA, "error %{public}d", ret);
        return false;
    }
    return true;
}

bool FAAbilityThread::HandleNotifyChange(const Uri &uri)
{
    auto obsMgrClient = DataObsMgrClient::GetInstance();
    if (obsMgrClient == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null obsMgrClient");
        return false;
    }

    ErrCode ret = obsMgrClient->NotifyChange(uri);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::FA, "error %{public}d", ret);
        return false;
    }
    return true;
}

bool FAAbilityThread::ScheduleRegisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    TAG_LOGD(AAFwkTag::FA, "called");
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityHandler_");
        return false;
    }
    wptr<FAAbilityThread> weak = this;
    auto task = [weak, uri, dataObserver]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::FA, "null abilityThread");
            return;
        }
        abilityThread->HandleRegisterObserver(uri, dataObserver);
    };
    bool ret = abilityHandler_->PostTask(task, "FAAbilityThread:RegisterObserver");
    if (!ret) {
        TAG_LOGE(AAFwkTag::FA, "PostTask error");
    }
    return ret;
}

bool FAAbilityThread::ScheduleUnregisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    TAG_LOGD(AAFwkTag::FA, "called");
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityHandler_");
        return false;
    }
    wptr<FAAbilityThread> weak = this;
    auto task = [weak, uri, dataObserver]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::FA, "null abilityThread");
            return;
        }
        abilityThread->HandleUnregisterObserver(uri, dataObserver);
    };
    bool ret = abilityHandler_->PostSyncTask(task, "FAAbilityThread:UnregisterObserver");
    if (!ret) {
        TAG_LOGE(AAFwkTag::FA, "PostTask error");
    }
    return ret;
}

bool FAAbilityThread::ScheduleNotifyChange(const Uri &uri)
{
    TAG_LOGD(AAFwkTag::FA, "called");
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityHandler_");
        return false;
    }
    wptr<FAAbilityThread> weak = this;
    auto task = [weak, uri]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::FA, "null abilityThread");
            return;
        }
        abilityThread->HandleNotifyChange(uri);
    };
    bool ret = abilityHandler_->PostTask(task, "FAAbilityThread:NotifyChange");
    if (!ret) {
        TAG_LOGE(AAFwkTag::FA, "PostTask error");
    }
    return ret;
}

std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>> FAAbilityThread::ExecuteBatch(
    const std::vector<std::shared_ptr<AppExecFwk::DataAbilityOperation>> &operations)
{
    TAG_LOGD(AAFwkTag::FA, "begin");
    std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>> results;
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityImpl_");
        results.clear();
        return results;
    }
    results = abilityImpl_->ExecuteBatch(operations);
    return results;
}

std::shared_ptr<AbilityContext> FAAbilityThread::BuildAbilityContext(
    const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo,
    const std::shared_ptr<AppExecFwk::OHOSApplication> &application, const sptr<IRemoteObject> &token,
    const std::shared_ptr<Context> &stageContext)
{
    auto abilityContextImpl = std::make_shared<AbilityContextImpl>();
    if (abilityContextImpl == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityContextImpl");
        return abilityContextImpl;
    }
    abilityContextImpl->SetStageContext(stageContext);
    abilityContextImpl->SetToken(token);
    abilityContextImpl->SetAbilityInfo(abilityInfo);
    abilityContextImpl->SetConfiguration(application->GetConfiguration());
    return abilityContextImpl;
}

void FAAbilityThread::DumpAbilityInfo(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    TAG_LOGD(AAFwkTag::FA, "begin");
    if (token_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null token_");
        return;
    }
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityHandler_");
        return;
    }
    wptr<FAAbilityThread> weak = this;
    auto task = [weak, params, token = token_]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::FA, "null abilityThread");
            return;
        }
        std::vector<std::string> dumpInfo;
        abilityThread->DumpAbilityInfoInner(params, dumpInfo);
        ErrCode err = AbilityManagerClient::GetInstance()->DumpAbilityInfoDone(dumpInfo, token);
        if (err != ERR_OK) {
            TAG_LOGE(AAFwkTag::FA, "failed = %{public}d", err);
        }
    };
    abilityHandler_->PostTask(task, "FAAbilityThread:DumpAbilityInfo");
}

#ifdef SUPPORT_SCREEN
void FAAbilityThread::DumpAbilityInfoInner(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    TAG_LOGD(AAFwkTag::FA, "begin");
    if (currentAbility_ == nullptr && currentExtension_ == nullptr) {
        TAG_LOGD(AAFwkTag::FA, "currentAbility and currentExtension_ is nullptr");
        return;
    }
    if (currentAbility_ != nullptr) {
        if (abilityImpl_->IsStageBasedModel()) {
            auto scene = currentAbility_->GetScene();
            if (scene == nullptr) {
                TAG_LOGE(AAFwkTag::FA, "null scene");
                return;
            }
            auto window = scene->GetMainWindow();
            if (window == nullptr) {
                TAG_LOGE(AAFwkTag::FA, "null window");
                return;
            }
            window->DumpInfo(params, info);
        }
        currentAbility_->Dump(params, info);
    }
    if (currentExtension_ != nullptr) {
        currentExtension_->Dump(params, info);
    }
    if (params.empty()) {
        DumpOtherInfo(info);
        return;
    }
}
#else
void FAAbilityThread::DumpAbilityInfoInner(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    TAG_LOGD(AAFwkTag::FA, "begin");
    if (currentAbility_ != nullptr) {
        currentAbility_->Dump(params, info);
    }

    if (currentExtension_ != nullptr) {
        currentExtension_->Dump(params, info);
    }
    DumpOtherInfo(info);
}
#endif

void FAAbilityThread::DumpOtherInfo(std::vector<std::string> &info)
{
    std::string dumpInfo = "        event:";
    info.push_back(dumpInfo);
    if (abilityHandler_ == nullptr) {
        TAG_LOGD(AAFwkTag::FA, "null abilityHandler_");
        return;
    }
    auto runner = abilityHandler_->GetEventRunner();
    if (runner == nullptr) {
        TAG_LOGD(AAFwkTag::FA, "null runner_");
        return;
    }
    dumpInfo = "";
    runner->DumpRunnerInfo(dumpInfo);
    info.push_back(dumpInfo);
    if (currentAbility_ != nullptr) {
        const auto ablityContext = currentAbility_->GetAbilityContext();
        if (ablityContext == nullptr) {
            TAG_LOGD(AAFwkTag::FA, "abilitycontext is nullptr");
            return;
        }
        const auto localCallContainer = ablityContext->GetLocalCallContainer();
        if (localCallContainer == nullptr) {
            TAG_LOGD(AAFwkTag::FA, "localCallContainer is nullptr");
            return;
        }
        localCallContainer->DumpCalls(info);
    }
}

void FAAbilityThread::CallRequest()
{
    TAG_LOGD(AAFwkTag::FA, "begin");
    if (currentAbility_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null ability");
        return;
    }
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityHandler_");
        return;
    }

    sptr<IRemoteObject> retval = nullptr;
    std::weak_ptr<OHOS::AppExecFwk::Ability> weakAbility = currentAbility_;
    auto syncTask = [ability = weakAbility, &retval]() {
        auto currentAbility = ability.lock();
        if (currentAbility == nullptr) {
            TAG_LOGE(AAFwkTag::FA, "null ability");
            return;
        }

        retval = currentAbility->CallRequest();
    };
    abilityHandler_->PostSyncTask(syncTask, "FAAbilityThread:CallRequest");
    AbilityManagerClient::GetInstance()->CallRequestDone(token_, retval);
}

void FAAbilityThread::ExecuteSkill(const Want &want)
{
    TAG_LOGW(AAFwkTag::FA, "FA model does not support ExecuteSkill");
}

void FAAbilityThread::HandlePrepareTerminateAbility()
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null abilityImpl_");
        return;
    }
    isPrepareTerminate_ = abilityImpl_->PrepareTerminateAbility();
    TAG_LOGD(AAFwkTag::FA, "end, ret = %{public}d", isPrepareTerminate_);
    isPrepareTerminateAbilityDone_.store(true);
    cv_.notify_all();
}
#ifdef SUPPORT_SCREEN
int FAAbilityThread::CreateModalUIExtension(const Want &want)
{
    TAG_LOGD(AAFwkTag::FA, "Call");
    if (currentAbility_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null current ability");
        return ERR_INVALID_VALUE;
    }
    return currentAbility_->CreateModalUIExtension(want);
}
#endif //SUPPORT_SCREEN
void FAAbilityThread::UpdateSessionToken(sptr<IRemoteObject> sessionToken)
{
    if (currentAbility_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null current ability");
        return;
    }
#ifdef SUPPORT_SCREEN
    currentAbility_->UpdateSessionToken(sessionToken);
#endif //SUPPORT_SCREEN
}
} // namespace AbilityRuntime
} // namespace OHOS
