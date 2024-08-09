/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "ui_ability_thread.h"

#include <chrono>

#include "ability_context_impl.h"
#include "ability_handler.h"
#include "ability_loader.h"
#include "ability_manager_client.h"
#include "context_deal.h"
#include "freeze_util.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "time_util.h"

namespace OHOS {
namespace AbilityRuntime {
using AbilityManagerClient = OHOS::AAFwk::AbilityManagerClient;
namespace {
#ifdef SUPPORT_GRAPHICS
constexpr static char ABILITY_NAME[] = "UIAbility";
#endif
const int32_t PREPARE_TO_TERMINATE_TIMEOUT_MILLISECONDS = 3000;
}
UIAbilityThread::UIAbilityThread() : abilityImpl_(nullptr), currentAbility_(nullptr) {}

UIAbilityThread::~UIAbilityThread()
{
    if (currentAbility_) {
        currentAbility_->DetachBaseContext();
        currentAbility_.reset();
    }
}

std::string UIAbilityThread::CreateAbilityName(const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord)
{
    std::string abilityName;
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "AbilityRecord is nullptr.");
        return abilityName;
    }

    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = abilityRecord->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "AbilityInfo is nullptr.");
        return abilityName;
    }

    if (abilityInfo->isNativeAbility) {
        TAG_LOGD(AAFwkTag::UIABILITY, "AbilityInfo name is %{public}s.", abilityInfo->name.c_str());
        return abilityInfo->name;
    }
#ifdef SUPPORT_GRAPHICS
    abilityName = ABILITY_NAME;
#else
    abilityName = abilityInfo->name;
#endif
    TAG_LOGD(AAFwkTag::UIABILITY, "Ability name is %{public}s.", abilityName.c_str());
    return abilityName;
}

std::shared_ptr<AppExecFwk::ContextDeal> UIAbilityThread::CreateAndInitContextDeal(
    const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
    const std::shared_ptr<AppExecFwk::AbilityContext> &abilityObject)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    std::shared_ptr<AppExecFwk::ContextDeal> contextDeal = nullptr;
    if (application == nullptr || abilityRecord == nullptr || abilityObject == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Application or abilityRecord or abilityObject is nullptr.");
        return contextDeal;
    }

    contextDeal = std::make_shared<AppExecFwk::ContextDeal>();
    if (contextDeal == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "ContextDeal is nullptr.");
        return contextDeal;
    }

    auto abilityInfo = abilityRecord->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "ContextDeal is nullptr.");
        return nullptr;
    }

    contextDeal->SetAbilityInfo(abilityInfo);
    contextDeal->SetApplicationInfo(application->GetApplicationInfo());
    abilityObject->SetProcessInfo(application->GetProcessInfo());
    std::shared_ptr<AppExecFwk::Context> tmpContext = application->GetApplicationContext();
    contextDeal->SetApplicationContext(tmpContext);
    contextDeal->SetBundleCodePath(abilityInfo->codePath);
    contextDeal->SetContext(abilityObject);
    return contextDeal;
}

void UIAbilityThread::Attach(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
    const std::shared_ptr<AppExecFwk::EventRunner> &mainRunner,
    const std::shared_ptr<Context> &stageContext)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if ((application == nullptr) || (abilityRecord == nullptr) || (mainRunner == nullptr)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Application or abilityRecord or mainRunner is nullptr.");
        return;
    }

    // 1.new AbilityHandler
    std::string abilityName = CreateAbilityName(abilityRecord);
    if (abilityName.empty()) {
        TAG_LOGE(AAFwkTag::UIABILITY, "AbilityName is empty.");
        return;
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin ability: %{public}s.", abilityRecord->GetAbilityInfo()->name.c_str());
    abilityHandler_ = std::make_shared<AppExecFwk::AbilityHandler>(mainRunner);
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityHandler_ is nullptr.");
        return;
    }

    // 2.new ability
    auto ability = AppExecFwk::AbilityLoader::GetInstance().GetUIAbilityByName(abilityName);
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Ability is nullptr.");
        return;
    }
    ability->SetAbilityRecordId(abilityRecord->GetAbilityRecordId());
    currentAbility_.reset(ability);
    token_ = abilityRecord->GetToken();
    abilityRecord->SetEventHandler(abilityHandler_);
    abilityRecord->SetEventRunner(mainRunner);
    abilityRecord->SetAbilityThread(this);
    std::shared_ptr<AppExecFwk::AbilityContext> abilityObject = currentAbility_;
    std::shared_ptr<AppExecFwk::ContextDeal> contextDeal =
        CreateAndInitContextDeal(application, abilityRecord, abilityObject);
    ability->AttachBaseContext(contextDeal);

    // new hap requires
    ability->AttachAbilityContext(BuildAbilityContext(abilityRecord->GetAbilityInfo(),
        application, token_, stageContext, abilityRecord->GetAbilityRecordId()));

    AttachInner(application, abilityRecord, stageContext);
}

void UIAbilityThread::AttachInner(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
    const std::shared_ptr<Context> &stageContext)
{
    // new abilityImpl
    abilityImpl_ = std::make_shared<UIAbilityImpl>();
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityImpl_ is nullptr.");
        return;
    }
    abilityImpl_->Init(application, abilityRecord, currentAbility_, abilityHandler_, token_);

    // ability attach : ipc
    TAG_LOGI(AAFwkTag::UIABILITY, "LoadLifecycle: Attach uiability.");
    FreezeUtil::LifecycleFlow flow = { token_, FreezeUtil::TimeoutState::LOAD };
    std::string entry = std::to_string(TimeUtil::SystemTimeMillisecond()) +
        "; AbilityThread::Attach start; the load lifecycle.";
    FreezeUtil::GetInstance().AddLifecycleEvent(flow, entry);
    ErrCode err = AbilityManagerClient::GetInstance()->AttachAbilityThread(this, token_);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Err is %{public}d.", err);
        return;
    }
    FreezeUtil::GetInstance().DeleteLifecycleEvent(flow);
}

void UIAbilityThread::Attach(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
    const std::shared_ptr<Context> &stageContext)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin.");
    if ((application == nullptr) || (abilityRecord == nullptr)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Application or abilityRecord is nullptr.");
        return;
    }
    // 1.new AbilityHandler
    std::string abilityName = CreateAbilityName(abilityRecord);
    runner_ = AppExecFwk::EventRunner::Create(abilityName);
    if (runner_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Runner is nullptr.");
        return;
    }
    abilityHandler_ = std::make_shared<AppExecFwk::AbilityHandler>(runner_);
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityHandler_ is nullptr.");
        return;
    }

    // 2.new ability
    auto ability = AppExecFwk::AbilityLoader::GetInstance().GetUIAbilityByName(abilityName);
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Ability is nullptr.");
        return;
    }
    ability->SetAbilityRecordId(abilityRecord->GetAbilityRecordId());
    currentAbility_.reset(ability);
    token_ = abilityRecord->GetToken();
    abilityRecord->SetEventHandler(abilityHandler_);
    abilityRecord->SetEventRunner(runner_);
    abilityRecord->SetAbilityThread(this);
    std::shared_ptr<AppExecFwk::AbilityContext> abilityObject = currentAbility_;
    std::shared_ptr<AppExecFwk::ContextDeal> contextDeal =
        CreateAndInitContextDeal(application, abilityRecord, abilityObject);
    ability->AttachBaseContext(contextDeal);

    // new hap requires
    ability->AttachAbilityContext(BuildAbilityContext(abilityRecord->GetAbilityInfo(),
        application, token_, stageContext, abilityRecord->GetAbilityRecordId()));

    AttachInner(application, abilityRecord, stageContext);
    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
}

void UIAbilityThread::HandleAbilityTransaction(
    const Want &want, const LifeCycleStateInfo &lifeCycleStateInfo, sptr<AAFwk::SessionInfo> sessionInfo)
{
    std::string connector = "##";
    std::string traceName = __PRETTY_FUNCTION__ + connector + want.GetElement().GetAbilityName();
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, traceName);
    TAG_LOGD(AAFwkTag::UIABILITY, "Lifecycle: name is %{public}s.", want.GetElement().GetAbilityName().c_str());
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityImpl_ is nullptr.");
        return;
    }
    std::string methodName = "HandleAbilityTransaction";
    AddLifecycleEvent(lifeCycleStateInfo.state, methodName);

    abilityImpl_->SetCallingContext(lifeCycleStateInfo.caller.deviceId, lifeCycleStateInfo.caller.bundleName,
        lifeCycleStateInfo.caller.abilityName, lifeCycleStateInfo.caller.moduleName);
    abilityImpl_->HandleAbilityTransaction(want, lifeCycleStateInfo, sessionInfo);
    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
}

void UIAbilityThread::AddLifecycleEvent(uint32_t state, std::string &methodName) const
{
    if (state == AAFwk::ABILITY_STATE_FOREGROUND_NEW) {
        FreezeUtil::LifecycleFlow flow = { token_, FreezeUtil::TimeoutState::FOREGROUND };
        std::string entry = std::to_string(TimeUtil::SystemTimeMillisecond()) +
            "; AbilityThread::" + methodName + "; the foreground lifecycle.";
        FreezeUtil::GetInstance().AddLifecycleEvent(flow, entry);
    }
    if (state == AAFwk::ABILITY_STATE_BACKGROUND_NEW) {
        FreezeUtil::LifecycleFlow flow = { token_, FreezeUtil::TimeoutState::BACKGROUND };
        std::string entry = std::to_string(TimeUtil::SystemTimeMillisecond()) +
            "; AbilityThread::" + methodName + "; the background lifecycle.";
        FreezeUtil::GetInstance().AddLifecycleEvent(flow, entry);
    }
}

void UIAbilityThread::HandleShareData(const int32_t &uniqueId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityImpl_ is nullptr.");
        return;
    }
    abilityImpl_->HandleShareData(uniqueId);
    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
}

void UIAbilityThread::ScheduleSaveAbilityState()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin.");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityImpl_ is nullptr.");
        return;
    }

    abilityImpl_->DispatchSaveAbilityState();
    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
}

void UIAbilityThread::ScheduleRestoreAbilityState(const AppExecFwk::PacMap &state)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin.");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityImpl_ is nullptr.");
        return;
    }
    abilityImpl_->DispatchRestoreAbilityState(state);
    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
}

void UIAbilityThread::ScheduleUpdateConfiguration(const AppExecFwk::Configuration &config)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin.");
    HandleUpdateConfiguration(config);
    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
}

void UIAbilityThread::HandleUpdateConfiguration(const AppExecFwk::Configuration &config)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin.");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityImpl_ is nullptr.");
        return;
    }

    abilityImpl_->ScheduleUpdateConfiguration(config);
    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
}

void UIAbilityThread::ScheduleAbilityTransaction(
    const Want &want, const LifeCycleStateInfo &lifeCycleStateInfo, sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::UIABILITY, "Lifecycle: name:%{public}s,targeState:%{public}d,isNewWant:%{public}d",
        want.GetElement().GetAbilityName().c_str(),
        lifeCycleStateInfo.state,
        lifeCycleStateInfo.isNewWant);
    std::string methodName = "ScheduleAbilityTransaction";
    AddLifecycleEvent(lifeCycleStateInfo.state, methodName);

    if (token_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "token_ is nullptr.");
        return;
    }
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityHandler_ is nullptr.");
        return;
    }
    wptr<UIAbilityThread> weak = this;
    auto task = [weak, want, lifeCycleStateInfo, sessionInfo]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "AbilityThread is nullptr.");
            return;
        }

        abilityThread->HandleAbilityTransaction(want, lifeCycleStateInfo, sessionInfo);
    };
    bool ret = abilityHandler_->PostTask(task, "UIAbilityThread:AbilityTransaction");
    if (!ret) {
        TAG_LOGE(AAFwkTag::UIABILITY, "PostTask error.");
    }
}

void UIAbilityThread::ScheduleShareData(const int32_t &uniqueId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (token_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "token_ is nullptr.");
        return;
    }
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityHandler_ is nullptr.");
        return;
    }
    wptr<UIAbilityThread> weak = this;
    auto task = [weak, uniqueId]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "AbilityThread is nullptr.");
            return;
        }
        abilityThread->HandleShareData(uniqueId);
    };
    bool ret = abilityHandler_->PostTask(task, "UIAbilityThread:ShareData");
    if (!ret) {
        TAG_LOGE(AAFwkTag::UIABILITY, "postTask error.");
    }
}

bool UIAbilityThread::SchedulePrepareTerminateAbility()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin.");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityImpl_ is nullptr.");
        return true;
    }
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityHandler_ is nullptr.");
        return false;
    }
    if (getpid() == gettid()) {
        bool ret = abilityImpl_->PrepareTerminateAbility();
        TAG_LOGD(AAFwkTag::UIABILITY, "End ret is %{public}d.", ret);
        return ret;
    }
    wptr<UIAbilityThread> weak = this;
    auto task = [weak]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "AbilityThread is nullptr.");
            return;
        }
        abilityThread->HandlePrepareTermianteAbility();
    };
    bool ret = abilityHandler_->PostTask(task, "UIAbilityThread:PrepareTerminateAbility");
    if (!ret) {
        TAG_LOGE(AAFwkTag::UIABILITY, "PostTask error.");
        return false;
    }

    std::unique_lock<std::mutex> lock(mutex_);
    auto condition = [weak] {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "AbilityThread is nullptr.");
            return false;
        }
        return abilityThread->isPrepareTerminateAbilityDone_.load();
    };
    if (!cv_.wait_for(lock, std::chrono::milliseconds(PREPARE_TO_TERMINATE_TIMEOUT_MILLISECONDS), condition)) {
        TAG_LOGW(AAFwkTag::UIABILITY, "Wait timeout.");
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "End, ret is %{public}d.", isPrepareTerminate_);
    return isPrepareTerminate_;
}

void UIAbilityThread::SendResult(int requestCode, int resultCode, const Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin.");
    if (abilityHandler_ == nullptr || requestCode == -1) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityHandler_ is nullptr or requestCode is -1.");
        return;
    }

    wptr<UIAbilityThread> weak = this;
    auto task = [weak, requestCode, resultCode, want]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "AbilityThread is nullptr.");
            return;
        }

        if (abilityThread->abilityImpl_ != nullptr) {
            abilityThread->abilityImpl_->SendResult(requestCode, resultCode, want);
            return;
        }
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityImpl_ is nullptr");
    };
    bool ret = abilityHandler_->PostTask(task, "UIAbilityThread:SendResult");
    if (!ret) {
        TAG_LOGE(AAFwkTag::UIABILITY, "PostTask error");
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void UIAbilityThread::ContinueAbility(const std::string &deviceId, uint32_t versionCode)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin.");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityImpl_ is nullptr.");
        return;
    }
    abilityImpl_->ContinueAbility(deviceId, versionCode);
    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
}

void UIAbilityThread::NotifyContinuationResult(int32_t result)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Result: %{public}d.", result);
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityImpl_ is nullptr.");
        return;
    }
    abilityImpl_->NotifyContinuationResult(result);
}

void UIAbilityThread::NotifyMemoryLevel(int32_t level)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Result: %{public}d.", level);
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityImpl_ is nullptr.");
        return;
    }
    abilityImpl_->NotifyMemoryLevel(level);
}

std::shared_ptr<AbilityContext> UIAbilityThread::BuildAbilityContext(
    const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo,
    const std::shared_ptr<AppExecFwk::OHOSApplication> &application, const sptr<IRemoteObject> &token,
    const std::shared_ptr<Context> &stageContext, int32_t abilityRecordId)
{
    auto abilityContextImpl = std::make_shared<AbilityContextImpl>();
    if (abilityContextImpl == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityContextImpl is nullptr.");
        return abilityContextImpl;
    }
    abilityContextImpl->SetStageContext(stageContext);
    abilityContextImpl->SetToken(token);
    abilityContextImpl->SetAbilityInfo(abilityInfo);
    abilityContextImpl->SetConfiguration(application->GetConfiguration());
    abilityContextImpl->SetAbilityRecordId(abilityRecordId);
    return abilityContextImpl;
}

void UIAbilityThread::DumpAbilityInfo(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (token_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "token_ is nullptr.");
        return;
    }
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityHandler_ is nullptr.");
        return;
    }
    wptr<UIAbilityThread> weak = this;
    auto task = [weak, params, token = token_]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "AbilityThread is nullptr.");
            return;
        }
        std::vector<std::string> dumpInfo;
        abilityThread->DumpAbilityInfoInner(params, dumpInfo);
        ErrCode err = AbilityManagerClient::GetInstance()->DumpAbilityInfoDone(dumpInfo, token);
        if (err != ERR_OK) {
            TAG_LOGE(AAFwkTag::UIABILITY, "Failed err is %{public}d.", err);
        }
    };
    abilityHandler_->PostTask(task, "UIAbilityThread:DumpAbilityInfo");
}

#ifdef SUPPORT_SCREEN
void UIAbilityThread::DumpAbilityInfoInner(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin.");
    if (currentAbility_ == nullptr) {
        TAG_LOGD(AAFwkTag::UIABILITY, "currentAbility_ is nullptr.");
        return;
    }
    auto scene = currentAbility_->GetScene();
    if (scene == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "scene is nullptr.");
        return;
    }

    auto window = scene->GetMainWindow();
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Window is nullptr.");
        return;
    }
    window->DumpInfo(params, info);
    currentAbility_->Dump(params, info);
    if (params.empty()) {
        DumpOtherInfo(info);
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
}
#else
void UIAbilityThread::DumpAbilityInfoInner(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (currentAbility_ != nullptr) {
        currentAbility_->Dump(params, info);
    }

    DumpOtherInfo(info);
}
#endif

void UIAbilityThread::DumpOtherInfo(std::vector<std::string> &info)
{
    std::string dumpInfo = "        event:";
    info.push_back(dumpInfo);
    if (abilityHandler_ == nullptr) {
        TAG_LOGD(AAFwkTag::UIABILITY, "abilityHandler_ is nullptr.");
        return;
    }
    auto runner = abilityHandler_->GetEventRunner();
    if (runner == nullptr) {
        TAG_LOGD(AAFwkTag::UIABILITY, "Runner is nullptr.");
        return;
    }
    dumpInfo = "";
    runner->DumpRunnerInfo(dumpInfo);
    info.push_back(dumpInfo);
    if (currentAbility_ != nullptr) {
        const auto ablityContext = currentAbility_->GetAbilityContext();
        if (ablityContext == nullptr) {
            TAG_LOGD(AAFwkTag::UIABILITY, "Ablitycontext is nullptr.");
            return;
        }
        const auto localCallContainer = ablityContext->GetLocalCallContainer();
        if (localCallContainer == nullptr) {
            TAG_LOGD(AAFwkTag::UIABILITY, "LocalCallContainer is nullptr.");
            return;
        }
        localCallContainer->DumpCalls(info);
    }
}

void UIAbilityThread::CallRequest()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin.");
    if (currentAbility_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "ability is nullptr.");
        return;
    }
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityHandler_ is nullptr.");
        return;
    }

    sptr<IRemoteObject> retval = nullptr;
    std::weak_ptr<UIAbility> weakAbility = currentAbility_;
    auto syncTask = [ability = weakAbility, &retval]() {
        auto currentAbility = ability.lock();
        if (currentAbility == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "Ability is nullptr.");
            return;
        }

        retval = currentAbility->CallRequest();
    };
    abilityHandler_->PostSyncTask(syncTask, "UIAbilityThread:CallRequest");
    AbilityManagerClient::GetInstance()->CallRequestDone(token_, retval);
    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
}

void UIAbilityThread::OnExecuteIntent(const Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin.");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityImpl_ is nullptr.");
        return;
    }

    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityHandler_ is nullptr.");
        return;
    }

    wptr<UIAbilityThread> weak = this;
    auto task = [weak, want]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "AbilityThread is nullptr.");
            return;
        }
        if (abilityThread->abilityImpl_ != nullptr) {
            abilityThread->abilityImpl_->HandleExecuteInsightIntentBackground(want, true);
            return;
        }
    };
    abilityHandler_->PostTask(task, "UIAbilityThread:OnExecuteIntent");
}

void UIAbilityThread::HandlePrepareTermianteAbility()
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityImpl_ is nullptr.");
        return;
    }
    isPrepareTerminate_ = abilityImpl_->PrepareTerminateAbility();
    TAG_LOGD(AAFwkTag::UIABILITY, "End ret is %{public}d.", isPrepareTerminate_);
    isPrepareTerminateAbilityDone_.store(true);
    cv_.notify_all();
}
#ifdef SUPPORT_SCREEN
int UIAbilityThread::CreateModalUIExtension(const Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Call");
    if (currentAbility_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "current ability is nullptr");
        return ERR_INVALID_VALUE;
    }
    return currentAbility_->CreateModalUIExtension(want);
}
#endif //SUPPORT_SCREEN
void UIAbilityThread::UpdateSessionToken(sptr<IRemoteObject> sessionToken)
{
    if (currentAbility_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "current ability is nullptr");
        return;
    }
#ifdef SUPPORT_SCREEN
    currentAbility_->UpdateSessionToken(sessionToken);
#endif //SUPPORT_SCREEN
}
} // namespace AbilityRuntime
} // namespace OHOS
