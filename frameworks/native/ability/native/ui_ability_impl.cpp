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

#include "ui_ability_impl.h"

#include "ability_handler.h"
#include "ability_manager_client.h"
#include "context/application_context.h"
#include "freeze_util.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "js_ui_ability.h"
#include "ohos_application.h"
#include "process_options.h"
#ifdef SUPPORT_SCREEN
#include "scene_board_judgement.h"
#endif
#include "time_util.h"

namespace OHOS {
namespace AbilityRuntime {
void UIAbilityImpl::Init(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &record, std::shared_ptr<UIAbility> &ability,
    std::shared_ptr<AppExecFwk::AbilityHandler> &handler, const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin.");
    if (token == nullptr || application == nullptr || handler == nullptr ||
        record == nullptr || ability == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Token or application or handler or record is nullptr.");
        return;
    }
    token_ = record->GetToken();
    ability_ = ability;
    handler_ = handler;
#ifdef SUPPORT_SCREEN
    ability_->SetSceneListener(sptr<WindowLifeCycleImpl>(
        new (std::nothrow) WindowLifeCycleImpl(token_, shared_from_this())));
#endif
    ability_->Init(record, application, handler, token);
    lifecycleState_ = AAFwk::ABILITY_STATE_INITIAL;
    abilityLifecycleCallbacks_ = application;
    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
}

void UIAbilityImpl::Start(const AAFwk::Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin.");
    if (ability_ == nullptr || ability_->GetAbilityInfo() == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "ability_ or abilityLifecycleCallbacks_ is nullptr.");
        return;
    }
#ifdef SUPPORT_GRAPHICS
    ability_->HandleCreateAsRecovery(want);
#endif
    ability_->OnStart(want, sessionInfo);

#ifdef SUPPORT_GRAPHICS
    lifecycleState_ = AAFwk::ABILITY_STATE_STARTED_NEW;
#else
    lifecycleState_ = AAFwk::ABILITY_STATE_INACTIVE;
#endif
    abilityLifecycleCallbacks_->OnAbilityStart(ability_);
    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
}

void UIAbilityImpl::Stop()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin.");
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "ability_ is nullptr.");
        return;
    }

    ability_->OnStop();
    StopCallback();
    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
}

void UIAbilityImpl::Stop(bool &isAsyncCallback)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin.");
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "ability_ is nullptr.");
        isAsyncCallback = false;
        return;
    }

    auto *callbackInfo = AppExecFwk::AbilityTransactionCallbackInfo<>::Create();
    if (callbackInfo == nullptr) {
        ability_->OnStop();
        StopCallback();
        isAsyncCallback = false;
        return;
    }
    std::weak_ptr<UIAbilityImpl> weakPtr = shared_from_this();
    auto asyncCallback = [abilityImplWeakPtr = weakPtr, state = AAFwk::ABILITY_STATE_INITIAL]() {
        auto abilityImpl = abilityImplWeakPtr.lock();
        if (abilityImpl == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "abilityImpl is nullptr.");
            return;
        }
        abilityImpl->StopCallback();
        abilityImpl->AbilityTransactionCallback(state);
    };
    callbackInfo->Push(asyncCallback);
    ability_->OnStop(callbackInfo, isAsyncCallback);
    if (!isAsyncCallback) {
        StopCallback();
        AppExecFwk::AbilityTransactionCallbackInfo<>::Destroy(callbackInfo);
    }
    // else: callbackInfo will be destroyed after the async callback
    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
}

void UIAbilityImpl::StopCallback()
{
    if (ability_ == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "ability_ or abilityLifecycleCallbacks_ is nullptr.");
        return;
    }
#ifdef SUPPORT_GRAPHICS
    lifecycleState_ = AAFwk::ABILITY_STATE_STOPED_NEW;
#else
    lifecycleState_ = AAFwk::ABILITY_STATE_INITIAL;
#endif
    abilityLifecycleCallbacks_->OnAbilityStop(ability_);
    ability_->DestroyInstance(); // Release window and ability.
}

int32_t UIAbilityImpl::Share(AAFwk::WantParams &wantParam)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin.");
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "ability_ is nullptr.");
        return ERR_INVALID_VALUE;
    }
    return ability_->OnShare(wantParam);
}

void UIAbilityImpl::DispatchSaveAbilityState()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    needSaveDate_ = true;
}

void UIAbilityImpl::DispatchRestoreAbilityState(const AppExecFwk::PacMap &inState)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    hasSaveData_ = true;
    restoreData_ = inState;
}

void UIAbilityImpl::HandleAbilityTransaction(
    const AAFwk::Want &want, const AAFwk::LifeCycleStateInfo &targetState, sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY,
        "Lifecycle: srcState:%{public}d; targetState: %{public}d; isNewWant: %{public}d, sceneFlag: %{public}d",
        lifecycleState_, targetState.state, targetState.isNewWant, targetState.sceneFlag);
    UpdateSilentForeground(targetState, sessionInfo);
#ifdef SUPPORT_SCREEN
    if (ability_ != nullptr) {
        ability_->sceneFlag_ = targetState.sceneFlag;
    }
    if ((lifecycleState_ == targetState.state) && !targetState.isNewWant) {
        if (ability_ != nullptr && targetState.state == AAFwk::ABILITY_STATE_FOREGROUND_NEW) {
            ability_->RequestFocus(want);
            AAFwk::AbilityManagerClient::GetInstance()->AbilityTransitionDone(
                token_, targetState.state, GetRestoreData());
        }
        TAG_LOGE(AAFwkTag::UIABILITY, "Org lifeCycleState equals to Dst lifeCycleState.");
        return;
    }
#endif
    SetLifeCycleStateInfo(targetState);
    if (ability_ != nullptr) {
        ability_->SetLaunchParam(targetState.launchParam);
        if (lifecycleState_ == AAFwk::ABILITY_STATE_INITIAL) {
            ability_->SetStartAbilitySetting(targetState.setting);
            Start(want, sessionInfo);
            CheckAndRestore();
        }
    }

    bool ret = false;
    ret = AbilityTransaction(want, targetState);
    if (ret) {
        AbilityTransactionCallback(targetState.state);
    }
}

void UIAbilityImpl::HandleShareData(int32_t uniqueId)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called sourceState: %{public}d.", lifecycleState_);
    WantParams wantParam;
    int32_t resultCode = Share(wantParam);
    TAG_LOGD(AAFwkTag::UIABILITY, "WantParam size: %{public}d.", wantParam.Size());
    AAFwk::AbilityManagerClient::GetInstance()->ShareDataDone(token_, resultCode, uniqueId, wantParam);
}

void UIAbilityImpl::AbilityTransactionCallback(const AAFwk::AbilityLifeCycleState &state)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Lifecycle: notify ability manager service.");
    FreezeUtil::LifecycleFlow flow = { token_, FreezeUtil::TimeoutState::FOREGROUND };
    std::string entry = std::to_string(TimeUtil::SystemTimeMillisecond()) +
        "; AbilityManagerClient::AbilityTransitionDone; the transaction start.";
    FreezeUtil::GetInstance().AddLifecycleEvent(flow, entry);
    if (state == AAFwk::ABILITY_STATE_FOREGROUND_NEW) {
        lifecycleState_ = AAFwk::ABILITY_STATE_FOREGROUND_NEW;
    }
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->AbilityTransitionDone(token_, state, GetRestoreData());
    if (ret == ERR_OK && state == AAFwk::ABILITY_STATE_FOREGROUND_NEW) {
        FreezeUtil::GetInstance().DeleteLifecycleEvent(flow);
    }
}

void UIAbilityImpl::ExecuteInsightIntentDone(uint64_t intentId, const InsightIntentExecuteResult &result)
{
    TAG_LOGI(AAFwkTag::UIABILITY, "Notify execute done, intentId %{public}" PRIu64"", intentId);
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->ExecuteInsightIntentDone(token_, intentId, result);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Notify execute done faild.");
    }
}
#ifdef SUPPORT_SCREEN
bool UIAbilityImpl::PrepareTerminateAbility()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin.");
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "ability_ is nullptr.");
        return false;
    }
    bool ret = ability_->OnPrepareTerminate();
    TAG_LOGD(AAFwkTag::UIABILITY, "End ret is %{public}d.", ret);
    return ret;
}
#endif
void UIAbilityImpl::SendResult(int requestCode, int resultCode, const AAFwk::Want &resultData)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin.");
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "ability_ is nullptr.");
        return;
    }

    ability_->OnAbilityResult(requestCode, resultCode, resultData);
    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
}

void UIAbilityImpl::NewWant(const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin.");
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "ability_ is nullptr.");
        return;
    }
    ability_->SetWant(want);
    ability_->OnNewWant(want);
#ifdef SUPPORT_SCREEN
    ability_->ContinuationRestore(want);
#endif
    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
}

void UIAbilityImpl::SetLifeCycleStateInfo(const AAFwk::LifeCycleStateInfo &info)
{
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "ability_ is nullptr.");
        return;
    }
    ability_->SetLifeCycleStateInfo(info);
}

bool UIAbilityImpl::CheckAndRestore()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin.");
    if (!hasSaveData_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "hasSaveData_ is false.");
        return false;
    }

    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "ability_ is nullptr.");
        return false;
    }
    ability_->OnRestoreAbilityState(restoreData_);
    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
    return true;
}

AppExecFwk::PacMap &UIAbilityImpl::GetRestoreData()
{
    return restoreData_;
}

void UIAbilityImpl::SetCallingContext(const std::string &deviceId, const std::string &bundleName,
    const std::string &abilityName, const std::string &moduleName)
{
    if (ability_ != nullptr) {
        ability_->SetCallingContext(deviceId, bundleName, abilityName, moduleName);
    }
}

void UIAbilityImpl::ScheduleUpdateConfiguration(const AppExecFwk::Configuration &config)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin.");
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "ability_ is nullptr.");
        return;
    }

    TAG_LOGD(AAFwkTag::UIABILITY, "Ability name: [%{public}s].", ability_->GetAbilityName().c_str());
    ability_->OnConfigurationUpdatedNotify(config);

    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
}

void UIAbilityImpl::ContinueAbility(const std::string &deviceId, uint32_t versionCode)
{
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "ability_ is nullptr.");
        return;
    }
    ability_->ContinueAbilityWithStack(deviceId, versionCode);
}

void UIAbilityImpl::NotifyContinuationResult(int32_t result)
{
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "ability_ is nullptr.");
        return;
    }
    ability_->OnCompleteContinuation(result);
}

void UIAbilityImpl::NotifyMemoryLevel(int32_t level)
{
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "ability_ is nullptr.");
        return;
    }
    ability_->OnMemoryLevel(level);
}

void UIAbilityImpl::UpdateSilentForeground(const AAFwk::LifeCycleStateInfo &targetState,
    sptr<AAFwk::SessionInfo> sessionInfo)
{
    if (ability_ == nullptr) {
        TAG_LOGD(AAFwkTag::UIABILITY, "ability_ is null");
        return;
    }
    if (ability_->CheckIsSilentForeground() && targetState.state == AAFwk::ABILITY_STATE_FOREGROUND_NEW) {
        lifecycleState_ = AAFwk::ABILITY_STATE_STARTED_NEW;
    }
    if (lifecycleState_ == AAFwk::ABILITY_STATE_INITIAL &&
        sessionInfo && sessionInfo->processOptions &&
        AAFwk::ProcessOptions::IsValidProcessMode(sessionInfo->processOptions->processMode) &&
        sessionInfo->processOptions->startupVisibility == AAFwk::StartupVisibility::STARTUP_HIDE) {
        TAG_LOGI(AAFwkTag::UIABILITY, "Set IsSilentForeground to true.");
        ability_->SetIsSilentForeground(true);
        return;
    }
    ability_->SetIsSilentForeground(false);
}

#ifdef SUPPORT_SCREEN
void UIAbilityImpl::AfterUnFocused()
{
    AfterFocusedCommon(false);
}

void UIAbilityImpl::AfterFocused()
{
    AfterFocusedCommon(true);
}

void UIAbilityImpl::AfterFocusedCommon(bool isFocused)
{
    auto task = [abilityImpl = weak_from_this(), focuseMode = isFocused]() {
        auto impl = abilityImpl.lock();
        if (impl == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "impl is nullptr.");
            return;
        }

        if (!impl->ability_ || !impl->ability_->GetAbilityInfo()) {
            TAG_LOGE(AAFwkTag::UIABILITY, "%{public}s failed.", focuseMode ? "AfterFocused" : "AfterUnFocused");
            return;
        }

        auto abilityContext = impl->ability_->GetAbilityContext();
        if (abilityContext == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "abilityContext is nullptr.");
            return;
        }
        auto applicationContext = abilityContext->GetApplicationContext();
        if (applicationContext == nullptr || applicationContext->IsAbilityLifecycleCallbackEmpty()) {
            TAG_LOGE(AAFwkTag::UIABILITY, "applicationContext or lifecycleCallback is nullptr.");
            return;
        }
        auto &jsAbility = static_cast<JsUIAbility &>(*(impl->ability_));
        if (focuseMode) {
            applicationContext->DispatchWindowStageFocus(jsAbility.GetJsAbility(), jsAbility.GetJsWindowStage());
        } else {
            applicationContext->DispatchWindowStageUnfocus(jsAbility.GetJsAbility(), jsAbility.GetJsWindowStage());
        }
    };

    if (handler_) {
        handler_->PostTask(task);
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
}

void UIAbilityImpl::WindowLifeCycleImpl::AfterForeground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "Lifecycle: Call.");
    auto owner = owner_.lock();
    if (owner == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Owner is nullptr.");
        return;
    }
    FreezeUtil::LifecycleFlow flow = { token_, FreezeUtil::TimeoutState::FOREGROUND };
    std::string entry = std::to_string(TimeUtil::SystemTimeMillisecond()) +
        "; UIAbilityImpl::WindowLifeCycleImpl::AfterForeground; the foreground lifecycle.";
    FreezeUtil::GetInstance().AddLifecycleEvent(flow, entry);

    bool needNotifyAMS = false;
    {
        std::lock_guard<std::mutex> lock(owner->notifyForegroundLock_);
        if (owner->notifyForegroundByAbility_) {
            owner->notifyForegroundByAbility_ = false;
            needNotifyAMS = true;
        } else {
            TAG_LOGD(AAFwkTag::UIABILITY, "Notify foreground by window, but client's foreground is running.");
            owner->notifyForegroundByWindow_ = true;
        }
    }

    if (needNotifyAMS) {
        TAG_LOGI(AAFwkTag::UIABILITY, "Lifecycle: window notify ability manager service.");
        entry = std::to_string(TimeUtil::SystemTimeMillisecond()) +
            "; AbilityManagerClient::AbilityTransitionDone; the transaction start.";
        FreezeUtil::GetInstance().AddLifecycleEvent(flow, entry);
        owner->lifecycleState_ = AAFwk::ABILITY_STATE_BACKGROUND_NEW;
        AppExecFwk::PacMap restoreData;
        auto ret = AAFwk::AbilityManagerClient::GetInstance()->AbilityTransitionDone(
            token_, AAFwk::AbilityLifeCycleState::ABILITY_STATE_FOREGROUND_NEW, restoreData);
        if (ret == ERR_OK) {
            FreezeUtil::GetInstance().DeleteLifecycleEvent(flow);
        }
    }
}

void UIAbilityImpl::WindowLifeCycleImpl::AfterBackground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    FreezeUtil::LifecycleFlow flow = { token_, FreezeUtil::TimeoutState::BACKGROUND };
    std::string entry = std::to_string(TimeUtil::SystemTimeMillisecond()) +
        "; UIAbilityImpl::WindowLifeCycleImpl::AfterBackground; the background lifecycle.";
    FreezeUtil::GetInstance().AddLifecycleEvent(flow, entry);

    TAG_LOGD(AAFwkTag::UIABILITY, "Lifecycle: window after background.");
    AppExecFwk::PacMap restoreData;
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->AbilityTransitionDone(
        token_, AAFwk::AbilityLifeCycleState::ABILITY_STATE_BACKGROUND_NEW, restoreData);
    if (ret == ERR_OK) {
        FreezeUtil::GetInstance().DeleteLifecycleEvent(flow);
    }
}

void UIAbilityImpl::WindowLifeCycleImpl::AfterFocused()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin.");
    auto owner = owner_.lock();
    if (owner) {
        owner->AfterFocused();
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
}

void UIAbilityImpl::WindowLifeCycleImpl::AfterUnfocused()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin.");
    auto owner = owner_.lock();
    if (owner) {
        owner->AfterUnFocused();
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
}

void UIAbilityImpl::WindowLifeCycleImpl::ForegroundFailed(int32_t type)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin.");
    AppExecFwk::PacMap restoreData;
    switch (type) {
        case static_cast<int32_t>(OHOS::Rosen::WMError::WM_ERROR_INVALID_OPERATION): {
            TAG_LOGD(AAFwkTag::UIABILITY, "Window was freezed.");
            AAFwk::AbilityManagerClient::GetInstance()->AbilityTransitionDone(
                token_, AAFwk::AbilityLifeCycleState::ABILITY_STATE_WINDOW_FREEZED, restoreData);
            break;
        }
        case static_cast<int32_t>(OHOS::Rosen::WMError::WM_ERROR_INVALID_WINDOW_MODE_OR_SIZE): {
            TAG_LOGD(AAFwkTag::UIABILITY, "The ability is stage mode, schedule foreground invalid mode.");
            AAFwk::AbilityManagerClient::GetInstance()->AbilityTransitionDone(
                token_, AAFwk::AbilityLifeCycleState::ABILITY_STATE_INVALID_WINDOW_MODE, restoreData);
            break;
        }
        case static_cast<int32_t>(OHOS::Rosen::WMError::WM_DO_NOTHING): {
            AAFwk::AbilityManagerClient::GetInstance()->AbilityTransitionDone(
                token_, AAFwk::AbilityLifeCycleState::ABILITY_STATE_DO_NOTHING, restoreData);
            break;
        }
        default: {
            AAFwk::AbilityManagerClient::GetInstance()->AbilityTransitionDone(
                token_, AAFwk::AbilityLifeCycleState::ABILITY_STATE_FOREGROUND_FAILED, restoreData);
            break;
        }
    }
}

void UIAbilityImpl::Foreground(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin.");
    if (ability_ == nullptr || ability_->GetAbilityInfo() == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "ability_ or abilityLifecycleCallbacks_ is nullptr.");
        return;
    }

    TAG_LOGD(AAFwkTag::UIABILITY, "Call onForeground.");
    ability_->OnForeground(want);
    if (ability_->CheckIsSilentForeground()) {
        TAG_LOGI(AAFwkTag::UIABILITY, "Is silent foreground.");
        std::lock_guard<std::mutex> lock(notifyForegroundLock_);
        notifyForegroundByWindow_ = true;
        return;
    }
    {
        std::lock_guard<std::mutex> lock(notifyForegroundLock_);
        notifyForegroundByAbility_ = true;
    }
    abilityLifecycleCallbacks_->OnAbilityForeground(ability_);
    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
}

void UIAbilityImpl::WindowLifeCycleImpl::BackgroundFailed(int32_t type)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (type == static_cast<int32_t>(OHOS::Rosen::WMError::WM_DO_NOTHING)) {
        AppExecFwk::PacMap restoreData;
        AAFwk::AbilityManagerClient::GetInstance()->AbilityTransitionDone(
            token_, AAFwk::AbilityLifeCycleState::ABILITY_STATE_BACKGROUND_FAILED, restoreData);
    }
}

void UIAbilityImpl::Background()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin.");
    if (ability_ == nullptr || ability_->GetAbilityInfo() == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "ability_ or abilityLifecycleCallbacks_ is nullptr.");
        return;
    }
    ability_->OnLeaveForeground();
    ability_->OnBackground();
    lifecycleState_ = AAFwk::ABILITY_STATE_BACKGROUND_NEW;
    abilityLifecycleCallbacks_->OnAbilityBackground(ability_);
    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
}
#endif

bool UIAbilityImpl::AbilityTransaction(const AAFwk::Want &want, const AAFwk::LifeCycleStateInfo &targetState)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "begin");
    bool ret = true;
    switch (targetState.state) {
        case AAFwk::ABILITY_STATE_INITIAL: {
            HandleInitialState(ret);
            break;
        }
        case AAFwk::ABILITY_STATE_FOREGROUND_NEW: {
            if (targetState.isNewWant) {
                NewWant(want);
            }
#ifdef SUPPORT_SCREEN
            if (!InsightIntentExecuteParam::IsInsightIntentExecute(want)) {
                HandleForegroundNewState(want, ret);
            } else {
                HandleExecuteInsightIntentForeground(want, ret);
            }
#endif
            break;
        }
        case AAFwk::ABILITY_STATE_BACKGROUND_NEW: {
            if (lifecycleState_ != AAFwk::ABILITY_STATE_STARTED_NEW) {
                ret = false;
            }
#ifdef SUPPORT_GRAPHICS
            if (!InsightIntentExecuteParam::IsInsightIntentExecute(want)) {
                Background();
            } else {
                TAG_LOGD(AAFwkTag::UIABILITY, "HandleExecuteInsightIntentBackground");
                HandleExecuteInsightIntentBackground(want);
            }
#endif
            break;
        }
        default: {
            ret = false;
            TAG_LOGE(AAFwkTag::UIABILITY, "State error.");
            break;
        }
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "End retVal is %{public}d.", static_cast<int>(ret));
    return ret;
}

void UIAbilityImpl::HandleInitialState(bool &ret)
{
#ifdef SUPPORT_SCREEN
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled() &&
        lifecycleState_ == AAFwk::ABILITY_STATE_FOREGROUND_NEW) {
        Background();
    }
#endif
    bool isAsyncCallback = false;
    Stop(isAsyncCallback);
    if (isAsyncCallback) {
        // AbilityManagerService will be notified after async callback
        ret = false;
    }
}

#ifdef SUPPORT_SCREEN
void UIAbilityImpl::HandleForegroundNewState(const AAFwk::Want &want, bool &bflag)
{
    if (lifecycleState_ == AAFwk::ABILITY_STATE_FOREGROUND_NEW) {
        if (ability_) {
            ability_->RequestFocus(want);
        }
    } else {
        {
            std::lock_guard<std::mutex> lock(notifyForegroundLock_);
            notifyForegroundByWindow_ = false;
        }
        Foreground(want);
        std::lock_guard<std::mutex> lock(notifyForegroundLock_);
        bflag = notifyForegroundByWindow_;
        if (bflag) {
            notifyForegroundByWindow_ = false;
            notifyForegroundByAbility_ = false;
        }
    }
}

void UIAbilityImpl::HandleExecuteInsightIntentForeground(const AAFwk::Want &want, bool &bflag)
{
    TAG_LOGI(AAFwkTag::UIABILITY, "Execute insight intent in foreground mode.");
    auto executeParam = std::make_shared<InsightIntentExecuteParam>();
    auto ret = InsightIntentExecuteParam::GenerateFromWant(want, *executeParam);
    if (!ret) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Generate execute param failed.");
        HandleForegroundNewState(want, bflag);
        return;
    }

    TAG_LOGD(AAFwkTag::UIABILITY,
        "Insight intent bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s"
        "insightIntentName: %{public}s, executeMode: %{public}d, intentId: %{public}" PRIu64"",
        executeParam->bundleName_.c_str(), executeParam->moduleName_.c_str(), executeParam->abilityName_.c_str(),
        executeParam->insightIntentName_.c_str(), executeParam->executeMode_, executeParam->insightIntentId_);
    auto intentCb = std::make_unique<InsightIntentExecutorAsyncCallback>();
    intentCb.reset(InsightIntentExecutorAsyncCallback::Create());
    if (intentCb == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Create async callback failed.");
        HandleForegroundNewState(want, bflag);
        return;
    }

    if (lifecycleState_ == AAFwk::ABILITY_STATE_FOREGROUND_NEW) {
        ExecuteInsightIntentRepeateForeground(want, executeParam, std::move(intentCb));
    } else {
        ExecuteInsightIntentMoveToForeground(want, executeParam, std::move(intentCb));
    }

    bflag = false;
}

void UIAbilityImpl::ExecuteInsightIntentRepeateForeground(const Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    auto asyncCallback =
        [weak = weak_from_this(), intentId = executeParam->insightIntentId_](InsightIntentExecuteResult result) {
            TAG_LOGD(AAFwkTag::UIABILITY, "Execute insight intent finshed, intentId %{public}" PRIu64"", intentId);
            auto abilityImpl = weak.lock();
            if (abilityImpl == nullptr) {
                TAG_LOGE(AAFwkTag::UIABILITY, "Ability impl is nullptr.");
                return;
            }
            abilityImpl->ExecuteInsightIntentDone(intentId, result);
            abilityImpl->AbilityTransactionCallback(AAFwk::ABILITY_STATE_FOREGROUND_NEW);
        };
    callback->Push(asyncCallback);

    // private function, no need check ability_ validity.
    ability_->ExecuteInsightIntentRepeateForeground(want, executeParam, std::move(callback));
}

void UIAbilityImpl::ExecuteInsightIntentMoveToForeground(const Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");

    {
        std::lock_guard<std::mutex> lock(notifyForegroundLock_);
        notifyForegroundByWindow_ = false;
    }

    auto asyncCallback =
        [weak = weak_from_this(), intentId = executeParam->insightIntentId_](InsightIntentExecuteResult result) {
            TAG_LOGD(AAFwkTag::UIABILITY, "Execute insight intent finshed, intentId %{public}" PRIu64"", intentId);
            auto abilityImpl = weak.lock();
            if (abilityImpl == nullptr) {
                TAG_LOGE(AAFwkTag::UIABILITY, "Ability impl is nullptr.");
                return;
            }
            abilityImpl->ExecuteInsightIntentDone(intentId, result);
            abilityImpl->PostForegroundInsightIntent();
        };
    callback->Push(asyncCallback);

    // private function, no need check ability_ validity.
    ability_->ExecuteInsightIntentMoveToForeground(want, executeParam, std::move(callback));
}

void UIAbilityImpl::PostForegroundInsightIntent()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (ability_ == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Ability params invalid.");
        return;
    }

    lifecycleState_ = AAFwk::ABILITY_STATE_FOREGROUND_NEW;

    {
        std::lock_guard<std::mutex> lock(notifyForegroundLock_);
        notifyForegroundByAbility_ = true;
    }

    abilityLifecycleCallbacks_->OnAbilityForeground(ability_);

    bool flag = true;
    {
        std::lock_guard<std::mutex> lock(notifyForegroundLock_);
        flag = notifyForegroundByWindow_;
        if (flag) {
            notifyForegroundByWindow_ = false;
            notifyForegroundByAbility_ = false;
        }
    }

    if (flag) {
        AbilityTransactionCallback(AAFwk::ABILITY_STATE_FOREGROUND_NEW);
    }
}

void UIAbilityImpl::HandleExecuteInsightIntentBackground(const AAFwk::Want &want, bool onlyExecuteIntent)
{
    TAG_LOGI(AAFwkTag::UIABILITY, "Execute insight intent in background mode.");
    auto executeParam = std::make_shared<InsightIntentExecuteParam>();
    auto ret = InsightIntentExecuteParam::GenerateFromWant(want, *executeParam);
    if (!ret && !onlyExecuteIntent) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Generate execute param failed.");
        Background();
        return;
    }

    TAG_LOGD(AAFwkTag::UIABILITY,
        "Insight intent bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s"
        "insightIntentName: %{public}s, executeMode: %{public}d, intentId: %{public}" PRIu64"",
        executeParam->bundleName_.c_str(), executeParam->moduleName_.c_str(), executeParam->abilityName_.c_str(),
        executeParam->insightIntentName_.c_str(), executeParam->executeMode_, executeParam->insightIntentId_);

    auto intentCb = std::make_unique<InsightIntentExecutorAsyncCallback>();
    intentCb.reset(InsightIntentExecutorAsyncCallback::Create());
    if (intentCb == nullptr && !onlyExecuteIntent) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Create async callback failed.");
        Background();
        return;
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "lifecycleState_: %{public}d", lifecycleState_);
    if (lifecycleState_ == AAFwk::ABILITY_STATE_INITIAL
        || lifecycleState_ == AAFwk::ABILITY_STATE_STARTED_NEW) {
        ExecuteInsightIntentBackgroundByColdBoot(want, executeParam, std::move(intentCb));
    } else {
        ExecuteInsightIntentBackgroundAlreadyStart(want, executeParam, std::move(intentCb));
    }
}

void UIAbilityImpl::ExecuteInsightIntentBackgroundByColdBoot(const Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    auto asyncCallback =
        [weak = weak_from_this(), intentId = executeParam->insightIntentId_](InsightIntentExecuteResult result) {
            TAG_LOGD(AAFwkTag::UIABILITY, "Execute insight intent finshed, intentId %{public}" PRIu64"", intentId);
            auto abilityImpl = weak.lock();
            if (abilityImpl == nullptr) {
                TAG_LOGE(AAFwkTag::UIABILITY, "Ability impl is nullptr.");
                return;
            }
            abilityImpl->Background();
            abilityImpl->ExecuteInsightIntentDone(intentId, result);
        };
    callback->Push(asyncCallback);

    // private function, no need check ability_ validity.
    ability_->ExecuteInsightIntentBackground(want, executeParam, std::move(callback));
}

void UIAbilityImpl::ExecuteInsightIntentBackgroundAlreadyStart(const Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");

    auto asyncCallback =
        [weak = weak_from_this(), intentId = executeParam->insightIntentId_](InsightIntentExecuteResult result) {
            TAG_LOGD(AAFwkTag::UIABILITY, "Execute insight intent finshed, intentId %{public}" PRIu64"", intentId);
            auto abilityImpl = weak.lock();
            if (abilityImpl == nullptr) {
                TAG_LOGE(AAFwkTag::UIABILITY, "Ability impl is nullptr.");
                return;
            }
            abilityImpl->ExecuteInsightIntentDone(intentId, result);
        };
    callback->Push(asyncCallback);

    // private function, no need check ability_ validity.
    ability_->ExecuteInsightIntentBackground(want, executeParam, std::move(callback));
}
#endif
} // namespace AbilityRuntime
} // namespace OHOS
