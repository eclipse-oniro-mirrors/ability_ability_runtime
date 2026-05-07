/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "extension.h"

#include "ability_local_record.h"
#include "configuration.h"
#include "extension_context.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace AbilityRuntime {
void Extension::Init(const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &record,
    const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    std::shared_ptr<AppExecFwk::AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    TAG_LOGD(AAFwkTag::EXT, "called");
    if ((record == nullptr) || (application == nullptr) || (handler == nullptr) || (token == nullptr)) {
        TAG_LOGE(AAFwkTag::EXT, "init failed, some obj null");
        return;
    }
    abilityInfo_ = record->GetAbilityInfo();
    handler_ = handler;
    application_ = application;
}

void Extension::OnStart(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::EXT, "extension:%{public}s", abilityInfo_->name.c_str());
    SetLaunchWant(want);
    SetLastRequestWant(want);
}

void Extension::OnStart(const AAFwk::Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::EXT, "extension:%{public}s", abilityInfo_->name.c_str());
    SetLaunchWant(want);
    SetLastRequestWant(want);
}

void Extension::OnStop()
{
    TAG_LOGD(AAFwkTag::EXT, "extension:%{public}s", abilityInfo_->name.c_str());
}

void Extension::OnStop(AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback)
{
    isAsyncCallback = false;
    OnStop();
}

void Extension::OnStopCallBack()
{
}

sptr<IRemoteObject> Extension::OnConnect(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::EXT, "extension:%{public}s", abilityInfo_->name.c_str());
    return nullptr;
}

sptr<IRemoteObject> Extension::OnConnect(const AAFwk::Want &want,
    AppExecFwk::AbilityTransactionCallbackInfo<sptr<IRemoteObject>> *callbackInfo, bool &isAsyncCallback)
{
    isAsyncCallback = false;
    return OnConnect(want);
}

void Extension::OnDisconnect(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::EXT, "extension:%{public}s", abilityInfo_->name.c_str());
}

void Extension::OnDisconnect(const AAFwk::Want &want, AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo,
    bool &isAsyncCallback)
{
    isAsyncCallback = false;
    OnDisconnect(want);
}

void Extension::OnCommand(const AAFwk::Want &want, bool restart, int startId)
{
    TAG_LOGD(AAFwkTag::EXT, "restart:%{public}s,startId:%{public}d",
        restart ? "true" : "false",
        startId);
    SetLastRequestWant(want);
}

void Extension::OnCommandWindow(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo,
    AAFwk::WindowCommand winCmd)
{
    TAG_LOGD(AAFwkTag::EXT, "call");
}

void Extension::OnForeground(const AAFwk::Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::EXT, "extension:%{public}s", abilityInfo_->name.c_str());
}

void Extension::OnBackground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::EXT, "extension:%{public}s", abilityInfo_->name.c_str());
}

void Extension::SetLaunchWant(const AAFwk::Want &want)
{
    launchWant_ = std::make_shared<AAFwk::Want>(want);
}

std::shared_ptr<AAFwk::Want> Extension::GetLaunchWant()
{
    return launchWant_;
}

void Extension::SetLastRequestWant(const AAFwk::Want &want)
{
    lastRequestWant_ = std::make_shared<AAFwk::Want>(want);
}

void Extension::SetCallingInfo(const CallingInfo &callingInfo)
{
    callingInfo_ = std::make_shared<CallingInfo>(callingInfo);
}

std::shared_ptr<CallingInfo> Extension::GetCallingInfo()
{
    return callingInfo_;
}

void Extension::OnConfigurationUpdated(const AppExecFwk::Configuration &configuration)
{
    TAG_LOGD(AAFwkTag::EXT, "call");
}

void Extension::OnMemoryLevel(int level)
{
    TAG_LOGD(AAFwkTag::EXT, "call");
}

void Extension::Dump(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    TAG_LOGD(AAFwkTag::EXT, "call");
}

void Extension::SetExtensionWindowLifeCycleListener(const sptr<Rosen::IWindowLifeCycle> &listener)
{
    extensionWindowLifeCycleListener_ = listener;
}

void Extension::OnAbilityResult(int requestCode, int resultCode, const Want &want)
{
    TAG_LOGD(AAFwkTag::EXT, "call");
}

void Extension::OnCommandWindowDone(const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd)
{
    TAG_LOGD(AAFwkTag::EXT, "call");
}

void Extension::OnInsightIntentExecuteDone(const sptr<AAFwk::SessionInfo> &sessionInfo,
    const AppExecFwk::InsightIntentExecuteResult &result)
{
    TAG_LOGD(AAFwkTag::EXT, "call");
}

void Extension::SetLaunchParam(const AAFwk::LaunchParam &launchParam)
{
    launchParam_ = launchParam;
}

const AAFwk::LaunchParam &Extension::GetLaunchParam() const
{
    return launchParam_;
}

bool Extension::HandleInsightIntent(const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::EXT, "call");
    return true;
}

bool Extension::HandleExecuteSkill(const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::EXT, "call");
    return true;
}

bool Extension::OnInsightIntentExecuteDone(uint64_t intentId, const AppExecFwk::InsightIntentExecuteResult &result)
{
    return true;
}

void Extension::OnExtensionAbilityRequestFailure(const std::string &requestId, const AppExecFwk::ElementName &element,
    const std::string &message, int32_t resultCode)
{
}

void Extension::OnExtensionAbilityRequestSuccess(const std::string &requestId, const AppExecFwk::ElementName &element,
    const std::string &message)
{
}
}
}
