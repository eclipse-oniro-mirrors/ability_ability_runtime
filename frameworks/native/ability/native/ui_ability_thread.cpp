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
#include "skill/skill_execute_param.h"
#include "time_util.h"

namespace OHOS {
namespace AbilityRuntime {
using AbilityManagerClient = OHOS::AAFwk::AbilityManagerClient;
namespace {
#ifdef SUPPORT_GRAPHICS
constexpr static char ABILITY_NAME[] = "UIAbility";
#endif
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
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityRecord");
        return abilityName;
    }

    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = abilityRecord->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityInfo");
        return abilityName;
    }

    if (abilityInfo->isNativeAbility) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityInfo name: %{public}s", abilityInfo->name.c_str());
        return abilityInfo->name;
    }
#ifdef SUPPORT_GRAPHICS
    abilityName = ABILITY_NAME;
#else
    abilityName = abilityInfo->name;
#endif
    TAG_LOGD(AAFwkTag::UIABILITY, "ability name: %{public}s", abilityName.c_str());
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
        TAG_LOGE(AAFwkTag::UIABILITY, "null application or abilityRecord or abilityObject");
        return contextDeal;
    }

    contextDeal = std::make_shared<AppExecFwk::ContextDeal>();
    if (contextDeal == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null contextDeal");
        return contextDeal;
    }

    auto abilityInfo = abilityRecord->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityInfo");
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
        TAG_LOGE(AAFwkTag::UIABILITY, "null application or abilityRecord or mainRunner");
        return;
    }

    // 1.new AbilityHandler
    std::string abilityName = CreateAbilityName(abilityRecord);
    if (abilityName.empty()) {
        TAG_LOGE(AAFwkTag::UIABILITY, "empty abilityName");
        return;
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "ability: %{public}s", abilityRecord->GetAbilityInfo()->name.c_str());
    abilityHandler_ = std::make_shared<AppExecFwk::AbilityHandler>(mainRunner);
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityHandler_");
        return;
    }

    // 2.new ability
    auto ability = AppExecFwk::AbilityLoader::GetInstance().GetUIAbilityByName(
        abilityName, abilityRecord->GetAbilityInfo()->arkTSMode);
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ability");
        return;
    }
    ability->SetAbilityRecordId(abilityRecord->GetAbilityRecordId());
    currentAbility_.reset(ability);
    token_ = abilityRecord->GetToken();
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

bool UIAbilityThread::InitNativeThread(const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo)
{
    auto appContext = AbilityRuntime::ApplicationContext::GetInstance();
    if (abilityInfo == nullptr || appContext == nullptr) {
        return false;
    }
    AAFwk::NativeAbilityMetaData metaData;
    AAFwk::NativeAbilityMetaData::InitData(*abilityInfo, metaData);
    if (!metaData.withNativeModule) {
        return true;
    }
    if (!appContext->CreateNativeThread(metaData, abilityInfo->bundleName, abilityInfo->moduleName)) {
        return false;
    }
    TAG_LOGI(AAFwkTag::UIABILITY, "Loading native module: %{public}s", metaData.nativeModuleSource.c_str());
    if (currentAbility_ != nullptr) {
        currentAbility_->SetWithNative(true);
    }
    if (abilityImpl_ != nullptr) {
        abilityImpl_->SetNativeModuleMetaData(metaData);
    }
    return true;
}

void UIAbilityThread::AttachInner(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
    const std::shared_ptr<Context> &stageContext)
{
    // new abilityImpl
    auto uiAbilityImpl = std::make_shared<UIAbilityImpl>();
    bool createObjSuc = true;
    uiAbilityImpl->Init(application, abilityRecord, currentAbility_, abilityHandler_, token_, createObjSuc);
    if (!createObjSuc) {
        TAG_LOGE(AAFwkTag::UIABILITY, "failed to create js obj");
        std::string entry = "AbilityThread::CreateObjError";
        FreezeUtil::GetInstance().AddLifecycleEvent(token_, entry);
        return;
    }
    abilityImpl_ = uiAbilityImpl;

    if (!InitNativeThread(abilityRecord->GetAbilityInfo())) {
        TAG_LOGI(AAFwkTag::UIABILITY, "InitNativeThread failed");
        // return to trigger load
        return;
    }

    // ability attach : ipc
    TAG_LOGI(AAFwkTag::UIABILITY, "Lifecycle:Attach");
    std::string entry = "AbilityThread::Attach";
    FreezeUtil::GetInstance().AddLifecycleEvent(token_, entry);
    ErrCode err = AbilityManagerClient::GetInstance()->AttachAbilityThread(this, token_);
    if (err != ERR_OK) {
        entry = std::string("AbilityThread::Attach; error ") + std::to_string(err);
        FreezeUtil::GetInstance().AddLifecycleEvent(token_, entry);
        TAG_LOGE(AAFwkTag::UIABILITY, "err: %{public}d", err);
        return;
    }
    FreezeUtil::GetInstance().DeleteLifecycleEvent(token_);
    FreezeUtil::GetInstance().DeleteAppLifecycleEvent(0);
}

void UIAbilityThread::Attach(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
    const std::shared_ptr<Context> &stageContext)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if ((application == nullptr) || (abilityRecord == nullptr)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null application or abilityRecord");
        return;
    }
    // 1.new AbilityHandler
    std::string abilityName = CreateAbilityName(abilityRecord);
    runner_ = AppExecFwk::EventRunner::Create(abilityName);
    if (runner_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null runner");
        return;
    }
    abilityHandler_ = std::make_shared<AppExecFwk::AbilityHandler>(runner_);
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityHandler_");
        return;
    }

    // 2.new ability
    auto ability = AppExecFwk::AbilityLoader::GetInstance().GetUIAbilityByName(
        abilityName, abilityRecord->GetAbilityInfo()->arkTSMode);
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ability");
        return;
    }
    ability->SetAbilityRecordId(abilityRecord->GetAbilityRecordId());
    currentAbility_.reset(ability);
    token_ = abilityRecord->GetToken();
    abilityRecord->SetAbilityThread(this);
    std::shared_ptr<AppExecFwk::AbilityContext> abilityObject = currentAbility_;
    std::shared_ptr<AppExecFwk::ContextDeal> contextDeal =
        CreateAndInitContextDeal(application, abilityRecord, abilityObject);
    ability->AttachBaseContext(contextDeal);

    // new hap requires
    ability->AttachAbilityContext(BuildAbilityContext(abilityRecord->GetAbilityInfo(),
        application, token_, stageContext, abilityRecord->GetAbilityRecordId()));

    AttachInner(application, abilityRecord, stageContext);
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void UIAbilityThread::HandleAbilityTransaction(
    const Want &want, const LifeCycleStateInfo &lifeCycleStateInfo, sptr<AAFwk::SessionInfo> sessionInfo)
{
    std::string connector = "##";
    std::string traceName = __PRETTY_FUNCTION__ + connector + want.GetElement().GetAbilityName();
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, traceName);
    TAG_LOGI(AAFwkTag::UIABILITY, "Lifecycle:name %{public}s", want.GetElement().GetAbilityName().c_str());
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityImpl_");
        return;
    }
    std::string methodName = "HandleAbilityTransaction";
    AddLifecycleEvent(lifeCycleStateInfo.state, methodName);

    abilityImpl_->SetCallingContext(lifeCycleStateInfo.caller.deviceId, lifeCycleStateInfo.caller.bundleName,
        lifeCycleStateInfo.caller.abilityName, lifeCycleStateInfo.caller.moduleName);
    abilityImpl_->HandleAbilityTransaction(want, lifeCycleStateInfo, sessionInfo);
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void UIAbilityThread::AddLifecycleEvent(uint32_t state, std::string &methodName) const
{
    if (state == AAFwk::ABILITY_STATE_FOREGROUND_NEW) {
        std::string entry = "AbilityThread::" + methodName;
        FreezeUtil::GetInstance().AddLifecycleEvent(token_, entry);
    }
    if (state == AAFwk::ABILITY_STATE_BACKGROUND_NEW) {
        std::string entry = "AbilityThread::" + methodName;
        FreezeUtil::GetInstance().AddLifecycleEvent(token_, entry);
    }
}

void UIAbilityThread::HandleShareData(const int32_t &uniqueId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityImpl_");
        return;
    }
    abilityImpl_->HandleShareData(uniqueId);
}

void UIAbilityThread::ScheduleSaveAbilityState()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityImpl_");
        return;
    }

    abilityImpl_->DispatchSaveAbilityState();
}

void UIAbilityThread::ScheduleRestoreAbilityState(const AppExecFwk::PacMap &state)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityImpl_");
        return;
    }
    abilityImpl_->DispatchRestoreAbilityState(state);
}

void UIAbilityThread::ScheduleUpdateConfiguration(const AppExecFwk::Configuration &config)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    HandleUpdateConfiguration(config);
}

void UIAbilityThread::HandleUpdateConfiguration(const AppExecFwk::Configuration &config)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityImpl_");
        return;
    }

    abilityImpl_->ScheduleUpdateConfiguration(config);
}

bool UIAbilityThread::ScheduleAbilityTransaction(
    const Want &want, const LifeCycleStateInfo &lifeCycleStateInfo, sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::UIABILITY, "name:%{public}s,targeState:%{public}d,isNewWant:%{public}d",
        want.GetElement().GetAbilityName().c_str(),
        lifeCycleStateInfo.state,
        lifeCycleStateInfo.isNewWant);
    std::string methodName = "ScheduleAbilityTransaction";
    AddLifecycleEvent(lifeCycleStateInfo.state, methodName);

    if (token_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null token_");
        return false;
    }
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityHandler_");
        return false;
    }
    wptr<UIAbilityThread> weak = this;
    auto task = [weak, want, lifeCycleStateInfo, sessionInfo]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null abilityThread");
            return;
        }

        abilityThread->HandleAbilityTransaction(want, lifeCycleStateInfo, sessionInfo);
    };
    bool ret = abilityHandler_->PostTask(task, "UIAbilityThread:AbilityTransaction");
    if (!ret) {
        TAG_LOGE(AAFwkTag::UIABILITY, "postTask error");
        return false;
    }
    return true;
}

void UIAbilityThread::ScheduleShareData(const int32_t &uniqueId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (token_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null token_");
        return;
    }
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityHandler_");
        return;
    }
    wptr<UIAbilityThread> weak = this;
    auto task = [weak, uniqueId]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null abilityThread");
            return;
        }
        abilityThread->HandleShareData(uniqueId);
    };
    bool ret = abilityHandler_->PostTask(task, "UIAbilityThread:ShareData");
    if (!ret) {
        TAG_LOGE(AAFwkTag::UIABILITY, "postTask error");
    }
}

bool UIAbilityThread::SchedulePrepareTerminateAbility()
{
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityHandler_");
        return false;
    }
    if (getpid() == gettid()) {
        TAG_LOGE(AAFwkTag::UIABILITY, "in app main thread");
        return HandlePrepareTerminateAbility();
    }
    wptr<UIAbilityThread> weak = this;
    auto task = [weak]() {
        TAG_LOGI(AAFwkTag::UIABILITY, "prepare terminate task");
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null abilityThread");
            return;
        }
        abilityThread->HandlePrepareTerminateAbility();
    };
    bool ret = abilityHandler_->PostTask(task, "UIAbilityThread:PrepareTerminateAbility");
    if (!ret) {
        TAG_LOGE(AAFwkTag::UIABILITY, "postTask error");
        return false;
    }
    return true;
}

bool UIAbilityThread::HandlePrepareTerminateAbility()
{
    wptr<UIAbilityThread> weak = this;
    auto callback = [weak, token = token_] (bool prepareTermination) {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null AbilityThread");
            return;
        }
        TAG_LOGI(AAFwkTag::UIABILITY, "in callback, prepareTermination=%{public}d", prepareTermination);
        AbilityManagerClient::GetInstance()->PrepareTerminateAbilityDone(token, prepareTermination);
    };
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityImpl_");
        return true;
    }
    bool isAsync = false;
    bool isSuccess = abilityImpl_->PrepareTerminateAbility(callback, isAsync);
    if (!isSuccess) {
        TAG_LOGI(AAFwkTag::UIABILITY, "PrepareTerminateAbility failed");
        callback(false);
        return false;
    }
    if (!isAsync) {
        TAG_LOGI(AAFwkTag::UIABILITY, "sync call");
    }
    return true;
}

void UIAbilityThread::SendResult(int requestCode, int resultCode, const Want &want)
{
    TAG_LOGI(AAFwkTag::UIABILITY, "SendResult");
    if (abilityHandler_ == nullptr || requestCode == -1) {
        TAG_LOGW(AAFwkTag::UIABILITY, "null abilityHandler_ or requestCode is -1");
        return;
    }

    wptr<UIAbilityThread> weak = this;
    auto task = [weak, requestCode, resultCode, want]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null abilityThread");
            return;
        }

        if (abilityThread->abilityImpl_ != nullptr) {
            abilityThread->abilityImpl_->SendResult(requestCode, resultCode, want);
            return;
        }
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityImpl_");
    };
    bool ret = abilityHandler_->PostTask(task, "UIAbilityThread:SendResult");
    if (!ret) {
        TAG_LOGE(AAFwkTag::UIABILITY, "postTask error");
    }
}

void UIAbilityThread::ContinueAbility(const std::string &deviceId, uint32_t versionCode)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityImpl_");
        return;
    }
    abilityImpl_->ContinueAbility(deviceId, versionCode);
}

void UIAbilityThread::NotifyContinuationResult(int32_t result)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "result: %{public}d", result);
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityImpl_");
        return;
    }
    abilityImpl_->NotifyContinuationResult(result);
}

void UIAbilityThread::NotifyMemoryLevel(int32_t level)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "result: %{public}d", level);
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityImpl_");
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
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityContextImpl");
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
        TAG_LOGE(AAFwkTag::UIABILITY, "null token_");
        return;
    }
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityHandler_");
        return;
    }
    wptr<UIAbilityThread> weak = this;
    auto task = [weak, params, token = token_]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null abilityThread");
            return;
        }
        std::vector<std::string> dumpInfo;
        abilityThread->DumpAbilityInfoInner(params, dumpInfo);
        ErrCode err = AbilityManagerClient::GetInstance()->DumpAbilityInfoDone(dumpInfo, token);
        if (err != ERR_OK) {
            TAG_LOGE(AAFwkTag::UIABILITY, "failed err: %{public}d", err);
        }
    };
    abilityHandler_->PostTask(task, "UIAbilityThread:DumpAbilityInfo");
}

#ifdef SUPPORT_SCREEN
void UIAbilityThread::DumpAbilityInfoInner(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (currentAbility_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null currentAbility_");
        return;
    }
    auto scene = currentAbility_->GetScene();
    if (scene == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null scene");
        return;
    }

    auto window = scene->GetMainWindow();
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null Window");
        return;
    }
    window->DumpInfo(params, info);
    currentAbility_->Dump(params, info);
    if (params.empty()) {
        DumpOtherInfo(info);
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
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
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityHandler_");
        return;
    }
    auto runner = abilityHandler_->GetEventRunner();
    if (runner == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null runner");
        return;
    }
    dumpInfo = "";
    runner->DumpRunnerInfo(dumpInfo);
    info.push_back(dumpInfo);
    if (currentAbility_ != nullptr) {
        const auto ablityContext = currentAbility_->GetAbilityContext();
        if (ablityContext == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null abilityContext");
            return;
        }
        const auto localCallContainer = ablityContext->GetLocalCallContainer();
        if (localCallContainer == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null localCallContainer");
            return;
        }
        localCallContainer->DumpCalls(info);
    }
}

void UIAbilityThread::CallRequest()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (currentAbility_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ability");
        return;
    }
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityHandler_");
        return;
    }

    sptr<IRemoteObject> retval = nullptr;
    std::weak_ptr<UIAbility> weakAbility = currentAbility_;
    auto syncTask = [ability = weakAbility, &retval]() {
        auto currentAbility = ability.lock();
        if (currentAbility == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null ability");
            return;
        }

        retval = currentAbility->CallRequest();
    };
    std::string entry = "UIAbilityThread::CallRequest Begin";
    FreezeUtil::GetInstance().AddLifecycleEvent(token_, entry);
    abilityHandler_->PostSyncTask(syncTask, "UIAbilityThread:CallRequest");
    std::string end = "UIAbilityThread::CallRequest End";
    FreezeUtil::GetInstance().AddLifecycleEvent(token_, end);
    AbilityManagerClient::GetInstance()->CallRequestDone(token_, retval);
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void UIAbilityThread::OnExecuteIntent(const Want &want)
{
    TAG_LOGI(AAFwkTag::UIABILITY, "on execute intent");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityImpl_");
        return;
    }

    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityHandler_");
        return;
    }

    wptr<UIAbilityThread> weak = this;
    auto task = [weak, want]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null AbilityThread");
            return;
        }
        if (abilityThread->abilityImpl_ != nullptr) {
            abilityThread->abilityImpl_->HandleExecuteInsightIntentBackground(want, true);
        }
    };
    abilityHandler_->PostTask(task, "UIAbilityThread:OnExecuteIntent");
}

void UIAbilityThread::ExecuteSkill(const Want &want)
{
    TAG_LOGI(AAFwkTag::UIABILITY, "execute skill");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityImpl_");
        return;
    }
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityHandler_");
        return;
    }
    wptr<UIAbilityThread> weak = this;
    auto task = [weak, want]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null AbilityThread");
            return;
        }
        if (abilityThread->abilityImpl_ != nullptr) {
            abilityThread->abilityImpl_->HandleExecuteSkill(want, true);
        }
    };
    abilityHandler_->PostTask(task, "UIAbilityThread:ExecuteSkill");
}

#ifdef SUPPORT_SCREEN
int UIAbilityThread::CreateModalUIExtension(const Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Call");
    if (currentAbility_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null current ability");
        return ERR_INVALID_VALUE;
    }
    return currentAbility_->CreateModalUIExtension(want);
}
#endif //SUPPORT_SCREEN
void UIAbilityThread::UpdateSessionToken(sptr<IRemoteObject> sessionToken)
{
    if (currentAbility_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null current ability");
        return;
    }
#ifdef SUPPORT_SCREEN
    currentAbility_->UpdateSessionToken(sessionToken);
#endif //SUPPORT_SCREEN
}

void UIAbilityThread::ScheduleCollaborate(const Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityImpl_");
        return;
    }
    wptr<UIAbilityThread> weak = this;
    auto task = [weak, want]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null abilityThread");
            return;
        }
        if (abilityThread->abilityImpl_ != nullptr) {
            abilityThread->abilityImpl_->ScheduleCollaborate(want);
            return;
        }
    };
    bool ret = abilityHandler_->PostTask(task, "UIAbilityThread:ScheduleCollaborate");
    if (!ret) {
        TAG_LOGE(AAFwkTag::UIABILITY, "postTask error");
        return;
    }
}

void UIAbilityThread::ScheduleAbilityRequestFailure(const std::string &requestId,
    const AppExecFwk::ElementName &element, const std::string &message, int32_t resultCode)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "ScheduleAbilityRequestFailure called");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityImpl_");
        return;
    }
    wptr<UIAbilityThread> weak = this;
    auto task = [weak, requestId, element, message, resultCode]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null abilityThread");
            return;
        }
        if (abilityThread->abilityImpl_ != nullptr) {
            abilityThread->abilityImpl_->ScheduleAbilityRequestFailure(requestId, element, message, resultCode);
            return;
        }
    };
    bool ret = abilityHandler_->PostTask(task, "UIAbilityThread:ScheduleAbilityRequestFailure");
    if (!ret) {
        TAG_LOGE(AAFwkTag::UIABILITY, "postTask error");
        return;
    }
}

void UIAbilityThread::ScheduleAbilityRequestSuccess(const std::string &requestId,
    const AppExecFwk::ElementName &element)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "ScheduleAbilityRequestSuccess called");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityImpl_");
        return;
    }
    wptr<UIAbilityThread> weak = this;
    auto task = [weak, requestId, element]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null abilityThread");
            return;
        }
        if (abilityThread->abilityImpl_ != nullptr) {
            abilityThread->abilityImpl_->ScheduleAbilityRequestSuccess(requestId, element);
            return;
        }
    };
    bool ret = abilityHandler_->PostTask(task, "UIAbilityThread:ScheduleAbilityRequestSuccess");
    if (!ret) {
        TAG_LOGE(AAFwkTag::UIABILITY, "postTask error");
        return;
    }
}

#ifdef SUPPORT_SCREEN
std::shared_ptr<UIAbility> UIAbilityThread::GetUIAbility()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityImpl_");
        return nullptr;
    }
    return abilityImpl_->GetUIAbility();
}
#endif
} // namespace AbilityRuntime
} // namespace OHOS
