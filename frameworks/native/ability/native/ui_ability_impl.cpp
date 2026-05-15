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

#include "ui_ability_impl.h"

#include <cinttypes>

#include "ability_handler.h"
#include "ability_manager_client.h"
#include "context/application_context.h"
#include "freeze_util.h"
#include "global_constant.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "json_utils.h"
#include "ohos_application.h"
#include "page_config_manager.h"
#include "process_options.h"
#ifdef SUPPORT_SCREEN
#include "scene_board_judgement.h"
#endif
#include "time_util.h"

namespace OHOS {
namespace AbilityRuntime {
const std::string JSON_KEY_ERR_MSG = "errMsg";

void UIAbilityImpl::Init(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &record, std::shared_ptr<UIAbility> &ability,
    std::shared_ptr<AppExecFwk::AbilityHandler> &handler, const sptr<IRemoteObject> &token, bool &createObjSuc)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (token == nullptr || application == nullptr || handler == nullptr ||
        record == nullptr || ability == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null token or application or handler or record");
        return;
    }
    token_ = record->GetToken();
    ability_ = ability;
    handler_ = handler;
#ifdef SUPPORT_SCREEN
    ability_->SetSceneListener(sptr<WindowLifeCycleImpl>(
        new (std::nothrow) WindowLifeCycleImpl(token_, shared_from_this())));
#endif
    ability_->Init(record, application, handler, token, createObjSuc);
    ability_->BindHybridContext(application, record);
    lifecycleState_ = AAFwk::ABILITY_STATE_INITIAL;
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void UIAbilityImpl::Start(const AAFwk::Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (ability_ == nullptr || ability_->GetAbilityInfo() == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ability_");
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
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void UIAbilityImpl::Stop()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ability_");
        return;
    }

    ability_->OnStop();
    StopCallback();
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void UIAbilityImpl::Stop(bool &isAsyncCallback)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ability_");
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
    std::weak_ptr<UIAbilityImpl> weakPtr = weak_from_this();
    auto asyncCallback = [abilityImplWeakPtr = weakPtr, state = AAFwk::ABILITY_STATE_INITIAL]() {
        auto abilityImpl = abilityImplWeakPtr.lock();
        if (abilityImpl == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null abilityImpl");
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
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void UIAbilityImpl::StopCallback()
{
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ability_");
        return;
    }
#ifdef SUPPORT_GRAPHICS
    lifecycleState_ = AAFwk::ABILITY_STATE_STOPED_NEW;
#else
    lifecycleState_ = AAFwk::ABILITY_STATE_INITIAL;
#endif
    ability_->DestroyInstance(); // Release window and ability.
}

int32_t UIAbilityImpl::Share(AAFwk::WantParams &wantParam)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ability_");
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
        "srcState:%{public}d; targetState: %{public}d; isNewWant: %{public}d, sceneFlag: %{public}d",
        lifecycleState_, targetState.state, targetState.isNewWant, targetState.sceneFlag);
    const_cast<AAFwk::LifeCycleStateInfo&>(targetState).pageConfig =
        want.GetStringParam(GlobalConstant::PAGE_CONFIG);
    const_cast<AAFwk::Want&>(want).RemoveParam(GlobalConstant::PAGE_CONFIG);
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
        if (ability_ != nullptr && targetState.state == AAFwk::ABILITY_STATE_BACKGROUND_NEW) {
            TAG_LOGW(AAFwkTag::UIABILITY, "OnBackground is called when current state is already background");
            Background();
            AAFwk::AbilityManagerClient::GetInstance()->AbilityTransitionDone(
                token_, targetState.state, GetRestoreData());
        }
        TAG_LOGE(AAFwkTag::UIABILITY, "Org lifeCycleState equals to dst lifeCycleState");
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

    // Check if native module startup phase is PRE_WINDOW
    if (localNativeState_ == LocalNativeState::INIT_PRE_WINDOW) {
        TAG_LOGI(AAFwkTag::UIABILITY, "Native module startPhase is PRE_WINDOW, skip AbilityTransaction");
        localNativeState_ = LocalNativeState::HALF_FOREGROUND;
        AbilityTransactionCallback(targetState.state);
        return;
    }

    bool ret = false;
    ret = AbilityTransaction(want, targetState);
    if (ret) {
        AbilityTransactionCallback(targetState.state);
    }
}

void UIAbilityImpl::HandleShareData(int32_t uniqueId)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "sourceState: %{public}d", lifecycleState_);
    WantParams wantParam;
    int32_t resultCode = Share(wantParam);
    TAG_LOGD(AAFwkTag::UIABILITY, "wantParam size: %{public}d", wantParam.Size());
    AAFwk::AbilityManagerClient::GetInstance()->ShareDataDone(token_, resultCode, uniqueId, wantParam);
}

void UIAbilityImpl::AbilityTransactionCallback(const AAFwk::AbilityLifeCycleState &state)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (state == AAFwk::ABILITY_STATE_FOREGROUND_NEW) {
        lifecycleState_ = AAFwk::ABILITY_STATE_FOREGROUND_NEW;
        std::string entry = "AbilityManagerClient::AbilityTransitionDone";
        FreezeUtil::GetInstance().AddLifecycleEvent(token_, entry);
    }
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->AbilityTransitionDone(token_, state, GetRestoreData());
    if (ret == ERR_OK && state == AAFwk::ABILITY_STATE_FOREGROUND_NEW) {
        FreezeUtil::GetInstance().DeleteLifecycleEvent(token_);
        FreezeUtil::GetInstance().DeleteAppLifecycleEvent(0);
    }
}

void UIAbilityImpl::ExecuteInsightIntentDone(uint64_t intentId, const InsightIntentExecuteResult &result)
{
    TAG_LOGI(AAFwkTag::UIABILITY, "intentId %{public}" PRIu64"", intentId);
    if (result.isNeedDelayResult) {
        TAG_LOGD(AAFwkTag::UIABILITY, "intent execute result need delay");
        return;
    }
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->ExecuteInsightIntentDone(token_, intentId, result);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "notify execute done failed");
    }
}

bool UIAbilityImpl::HandleExecuteSkill(const AAFwk::Want &want, bool onlyExecuteSkill)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "handle execute skill");
    auto param = std::make_shared<SkillExecuteParam>();
    if (!SkillExecuteParam::GenerateFromWant(want, *param)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "generate skill param from want failed");
        if (!onlyExecuteSkill) {
            Background();
        }
        return true;
    }

    TAG_LOGD(AAFwkTag::UIABILITY,
        "skill bundle:%{public}s module:%{public}s name:%{public}s "
        "scriptPath:%{public}s func:%{public}s requestCode:%{public}s",
        param->bundleName_.c_str(), param->moduleName_.c_str(),
        param->skillName_.c_str(), param->scriptPath_.c_str(),
        param->functionName_.c_str(), param->requestCode_.c_str());
    ability_->ExecuteSkill(want, param);
    if (!onlyExecuteSkill) {
        Background();
    }
    return true;
}

#ifdef SUPPORT_SCREEN
bool UIAbilityImpl::PrepareTerminateAbility(std::function<void(bool)> callback, bool &isAsync)
{
    if (ability_ == nullptr || callback == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null ability_ or callback");
        return false;
    }
    auto *callbackInfo = AppExecFwk::AbilityTransactionCallbackInfo<bool>::Create();
    if (callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null callbackInfo");
        return false;
    }
    callbackInfo->Push(callback);
    ability_->OnPrepareTerminate(callbackInfo, isAsync);
    TAG_LOGD(AAFwkTag::ABILITY, "end, isAsync=%{public}d", isAsync);
    if (!isAsync) {
        AppExecFwk::AbilityTransactionCallbackInfo<bool>::Destroy(callbackInfo);
    }
    return true;
}
#endif
void UIAbilityImpl::SendResult(int requestCode, int resultCode, const AAFwk::Want &resultData)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ability_");
        return;
    }

    ability_->OnAbilityResult(requestCode, resultCode, resultData);
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void UIAbilityImpl::NewWant(const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ability_");
        return;
    }
    ability_->SetWant(want);
    ability_->OnNewWant(want);
#ifdef SUPPORT_SCREEN
    ability_->ContinuationRestore(want);
#endif
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void UIAbilityImpl::SetLifeCycleStateInfo(const AAFwk::LifeCycleStateInfo &info)
{
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ability_");
        return;
    }
    ability_->SetLifeCycleStateInfo(info);
}

bool UIAbilityImpl::CheckAndRestore()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (!hasSaveData_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "hasSaveData_: false");
        return false;
    }

    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ability_");
        return false;
    }
    ability_->OnRestoreAbilityState(restoreData_);
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
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
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ability_");
        return;
    }

    TAG_LOGD(AAFwkTag::UIABILITY, "ability name: [%{public}s]", ability_->GetAbilityName().c_str());
    ability_->OnConfigurationUpdatedNotify(config);
}

void UIAbilityImpl::ContinueAbility(const std::string &deviceId, uint32_t versionCode)
{
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ability_");
        return;
    }
    ability_->ContinueAbilityWithStack(deviceId, versionCode);
}

void UIAbilityImpl::NotifyContinuationResult(int32_t result)
{
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ability_");
        return;
    }
    ability_->OnCompleteContinuation(result);
}

void UIAbilityImpl::NotifyMemoryLevel(int32_t level)
{
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ability_");
        return;
    }
    ability_->OnMemoryLevel(level);
}

void UIAbilityImpl::UpdateSilentForeground(const AAFwk::LifeCycleStateInfo &targetState,
    sptr<AAFwk::SessionInfo> sessionInfo)
{
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ability_");
        return;
    }
    if (ability_->CheckIsSilentForeground() && targetState.state == AAFwk::ABILITY_STATE_FOREGROUND_NEW) {
        lifecycleState_ = AAFwk::ABILITY_STATE_STARTED_NEW;
    }
    if (lifecycleState_ == AAFwk::ABILITY_STATE_INITIAL &&
        sessionInfo && sessionInfo->processOptions &&
        (sessionInfo->processOptions->isRestartKeepAlive ||
        AAFwk::ProcessOptions::IsValidProcessMode(sessionInfo->processOptions->processMode) ||
        sessionInfo->processOptions->isStartFromNDK || sessionInfo->processOptions->isPreloadStart) &&
        sessionInfo->processOptions->startupVisibility == AAFwk::StartupVisibility::STARTUP_HIDE) {
        TAG_LOGI(AAFwkTag::UIABILITY, "set IsSilentForeground to true");
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
            TAG_LOGE(AAFwkTag::UIABILITY, "null impl");
            return;
        }

        if (!impl->ability_ || !impl->ability_->GetAbilityInfo()) {
            TAG_LOGE(AAFwkTag::UIABILITY, "%{public}s failed", focuseMode ? "AfterFocused" : "AfterUnFocused");
            return;
        }

        impl->ability_->OnAfterFocusedCommon(focuseMode);
    };

    if (handler_) {
        handler_->PostTask(task);
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void UIAbilityImpl::WindowLifeCycleImpl::AfterForeground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::UIABILITY, "wnd call, AfterForeground");
    auto owner = owner_.lock();
    if (owner == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null owner");
        return;
    }
    std::string entry = "UIAbilityImpl::WindowLifeCycleImpl::AfterForeground";
    FreezeUtil::GetInstance().AddLifecycleEvent(token_, entry);

    bool needNotifyAMS = false;
    {
        std::lock_guard<std::mutex> lock(owner->notifyForegroundLock_);
        if (owner->notifyForegroundByAbility_) {
            owner->notifyForegroundByAbility_ = false;
            needNotifyAMS = true;
        } else {
            TAG_LOGD(AAFwkTag::UIABILITY, "notify foreground by window, but client's foreground is running");
            owner->notifyForegroundByWindow_ = true;
        }
    }

    if (needNotifyAMS) {
        TAG_LOGI(AAFwkTag::UIABILITY, "wnd call, notify ability manager service");
        entry = "AbilityManagerClient::AbilityTransitionDone";
        FreezeUtil::GetInstance().AddLifecycleEvent(token_, entry);
        owner->lifecycleState_ = AAFwk::ABILITY_STATE_BACKGROUND_NEW;
        AppExecFwk::PacMap restoreData;
        auto ret = AAFwk::AbilityManagerClient::GetInstance()->AbilityTransitionDone(
            token_, AAFwk::AbilityLifeCycleState::ABILITY_STATE_FOREGROUND_NEW, restoreData);
        if (ret == ERR_OK) {
            FreezeUtil::GetInstance().DeleteLifecycleEvent(token_);
            FreezeUtil::GetInstance().DeleteAppLifecycleEvent(0);
        }
    }
}

void UIAbilityImpl::WindowLifeCycleImpl::AfterBackground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::UIABILITY, "wnd call, AfterBackground");
    std::string entry = "UIAbilityImpl::WindowLifeCycleImpl::AfterBackground";
    FreezeUtil::GetInstance().AddLifecycleEvent(token_, entry);

    AppExecFwk::PacMap restoreData;
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->AbilityTransitionDone(
        token_, AAFwk::AbilityLifeCycleState::ABILITY_STATE_BACKGROUND_NEW, restoreData);
    if (ret == ERR_OK) {
        FreezeUtil::GetInstance().DeleteLifecycleEvent(token_);
        FreezeUtil::GetInstance().DeleteAppLifecycleEvent(0);
    }
}

void UIAbilityImpl::WindowLifeCycleImpl::AfterDidForeground()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "wnd call, AfterDidForeground");
    auto owner = owner_.lock();
    if (owner == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null owner");
        return;
    }
    if (owner->ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ability_");
        return;
    }
    owner->ability_->OnDidForeground();
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void UIAbilityImpl::WindowLifeCycleImpl::AfterDidBackground()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "wnd call, AfterDidBackground");
    auto owner = owner_.lock();
    if (owner == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null owner");
        return;
    }
    if (owner->ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ability_");
        return;
    }
    owner->ability_->OnDidBackground();
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void UIAbilityImpl::WindowLifeCycleImpl::AfterFocused()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    auto owner = owner_.lock();
    if (owner) {
        owner->AfterFocused();
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void UIAbilityImpl::WindowLifeCycleImpl::AfterUnfocused()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    auto owner = owner_.lock();
    if (owner) {
        owner->AfterUnFocused();
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void UIAbilityImpl::WindowLifeCycleImpl::ForegroundFailed(int32_t type)
{
    TAG_LOGE(AAFwkTag::UIABILITY, "wnd call, ForegroundFailed");
    std::string entry = "UIAbilityImpl::WindowLifeCycleImpl::ForegroundFailed; GoForeground failed";
    FreezeUtil::GetInstance().AppendLifecycleEvent(token_, entry);
    AppExecFwk::PacMap restoreData;
    switch (type) {
        case static_cast<int32_t>(OHOS::Rosen::WMError::WM_ERROR_INVALID_OPERATION): {
            TAG_LOGD(AAFwkTag::UIABILITY, "window is freezed");
            AAFwk::AbilityManagerClient::GetInstance()->AbilityTransitionDone(
                token_, AAFwk::AbilityLifeCycleState::ABILITY_STATE_WINDOW_FREEZED, restoreData);
            break;
        }
        case static_cast<int32_t>(OHOS::Rosen::WMError::WM_ERROR_INVALID_WINDOW_MODE_OR_SIZE): {
            TAG_LOGD(AAFwkTag::UIABILITY, "invalid stage mode");
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
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (ability_ == nullptr || ability_->GetAbilityInfo() == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ability_");
        return;
    }

    // Check if native module startup phase is PRE_FOREGROUND
    if (localNativeState_ == LocalNativeState::INIT_PRE_FOREGROUND) {
        TAG_LOGI(AAFwkTag::UIABILITY, "Native module startPhase is PRE_FOREGROUND, skip OnForeground");
        localNativeState_ = LocalNativeState::HALF_FOREGROUND;
        auto oldSilent = ability_->CheckIsSilentForeground();
        ability_->SetIsSilentForeground(true);
        ability_->OnForeground(want);
        ability_->SetIsSilentForeground(oldSilent);
        std::lock_guard<std::mutex> lock(notifyForegroundLock_);
        notifyForegroundByWindow_ = true;
        return;
    }

    ability_->OnForeground(want);
    localNativeState_ = LocalNativeState::NONE;
    if (ability_->CheckIsSilentForeground()) {
        TAG_LOGI(AAFwkTag::UIABILITY, "is silent foreground");
        std::lock_guard<std::mutex> lock(notifyForegroundLock_);
        notifyForegroundByWindow_ = true;
        return;
    }
    {
        std::lock_guard<std::mutex> lock(notifyForegroundLock_);
        notifyForegroundByAbility_ = true;
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void UIAbilityImpl::WindowLifeCycleImpl::BackgroundFailed(int32_t type)
{
    TAG_LOGE(AAFwkTag::UIABILITY, "wnd call, BackgroundFailed");
    if (type == static_cast<int32_t>(OHOS::Rosen::WMError::WM_DO_NOTHING)) {
        AppExecFwk::PacMap restoreData;
        AAFwk::AbilityManagerClient::GetInstance()->AbilityTransitionDone(
            token_, AAFwk::AbilityLifeCycleState::ABILITY_STATE_BACKGROUND_FAILED, restoreData);
    }
}

void UIAbilityImpl::Background()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (ability_ == nullptr || ability_->GetAbilityInfo() == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ability_");
        return;
    }
    ability_->OnLeaveForeground();
    ability_->OnBackground();
    lifecycleState_ = AAFwk::ABILITY_STATE_BACKGROUND_NEW;
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}
#endif

void UIAbilityImpl::OnWillBackground()
{
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ability_");
        return;
    }
    ability_->OnWillBackground();
}

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
            if (targetState.isNewWant && localNativeState_ == LocalNativeState::NONE) {
                NewWant(want);
            }
#ifdef SUPPORT_SCREEN
            if (!InsightIntentExecuteParam::IsInsightIntentExecute(want)) {
                HandleForegroundNewState(want, ret);
            } else {
                HandleExecuteInsightIntentForeground(want, ret);
            }
            if (ability_ != nullptr) {
                TAG_LOGD(AAFwkTag::UIABILITY, "pageConfigSize:%{public}zu", targetState.pageConfig.size());
                PageConfigManager::GetInstance().Initialize(targetState.pageConfig, ability_->GetWindow());
            }
#endif
            break;
        }
        case AAFwk::ABILITY_STATE_BACKGROUND_NEW: {
            if (lifecycleState_ != AAFwk::ABILITY_STATE_STARTED_NEW) {
                ret = false;
            }
            OnWillBackground();
#ifdef SUPPORT_GRAPHICS
            if (InsightIntentExecuteParam::IsInsightIntentExecute(want)) {
                TAG_LOGD(AAFwkTag::UIABILITY, "handleExecuteInsightIntentBackground");
                ret = HandleExecuteInsightIntentBackground(want);
            } else if (SkillExecuteParam::IsSkillExecute(want)) {
                TAG_LOGD(AAFwkTag::UIABILITY, "handleExecuteSkill");
                ret = HandleExecuteSkill(want);
            } else {
                Background();
            }
#endif
            break;
        }
        default: {
            ret = false;
            TAG_LOGE(AAFwkTag::UIABILITY, "state error");
            break;
        }
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "end retVal: %{public}d", static_cast<int>(ret));
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
    if (lifecycleState_ == AAFwk::ABILITY_STATE_FOREGROUND_NEW &&
        localNativeState_ == LocalNativeState::NONE) {
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
    std::string entry = "UIAbilityImpl::HandleExecuteInsightIntentForeground";
    FreezeUtil::GetInstance().AddLifecycleEvent(token_, entry);
    TAG_LOGI(AAFwkTag::INTENT, "handle execute insight intent foreground");
    auto executeParam = std::make_shared<InsightIntentExecuteParam>();
    auto ret = InsightIntentExecuteParam::GenerateFromWant(want, *executeParam);
    if (!ret) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid params");
        HandleForegroundNewState(want, bflag);
        return;
    }

    TAG_LOGD(AAFwkTag::INTENT,
        "insightIntent bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s"
        "insightIntentName: %{public}s, executeMode: %{public}d, intentId: %{public}" PRIu64"",
        executeParam->bundleName_.c_str(), executeParam->moduleName_.c_str(), executeParam->abilityName_.c_str(),
        executeParam->insightIntentName_.c_str(), executeParam->executeMode_, executeParam->insightIntentId_);
    auto intentCb = std::make_unique<InsightIntentExecutorAsyncCallback>();
    intentCb.reset(InsightIntentExecutorAsyncCallback::Create());
    if (intentCb == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null intentCb");
        HandleForegroundNewState(want, bflag);
        return;
    }

    if (AppExecFwk::InsightIntentExecuteParam::IsInsightIntentPage(want)) {
        ExecuteInsightIntentPage(want, executeParam, std::move(intentCb));
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
    TAG_LOGD(AAFwkTag::INTENT, "called");
    auto asyncCallback =
        [weak = weak_from_this(), intentId = executeParam->insightIntentId_](InsightIntentExecuteResult result) {
            TAG_LOGD(AAFwkTag::INTENT, "execute insightIntent finshed, intentId %{public}" PRIu64"", intentId);
            auto abilityImpl = weak.lock();
            if (abilityImpl == nullptr) {
                TAG_LOGE(AAFwkTag::INTENT, "null ability impl");
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
    TAG_LOGD(AAFwkTag::INTENT, "called");
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "ability null");
        return;
    }

    {
        std::lock_guard<std::mutex> lock(notifyForegroundLock_);
        notifyForegroundByWindow_ = false;
    }

    auto oriHideValue = ability_->CheckIsSilentForeground();
    if (localNativeState_ == LocalNativeState::INIT_PRE_FOREGROUND) {
        TAG_LOGI(AAFwkTag::UIABILITY, "Native module startPhase is PRE_FOREGROUND, skip OnForeground");
        localNativeState_ = LocalNativeState::HALF_FOREGROUND;
        ability_->SetIsSilentForeground(true);
        std::lock_guard<std::mutex> lock(notifyForegroundLock_);
        notifyForegroundByWindow_ = true;
    }

    auto asyncCallback =
        [weak = weak_from_this(), intentId = executeParam->insightIntentId_](InsightIntentExecuteResult result) {
            TAG_LOGD(AAFwkTag::INTENT, "end, intentId %{public}" PRIu64"", intentId);
            auto abilityImpl = weak.lock();
            if (abilityImpl == nullptr) {
                TAG_LOGE(AAFwkTag::INTENT, "null ability impl");
                return;
            }
            abilityImpl->ExecuteInsightIntentDone(intentId, result);
            abilityImpl->PostForegroundInsightIntent();
        };
    callback->Push(asyncCallback);

    // private function, no need check ability_ validity.
    ability_->ExecuteInsightIntentMoveToForeground(want, executeParam, std::move(callback));
    ability_->SetIsSilentForeground(oriHideValue);
}

void UIAbilityImpl::ExecuteInsightIntentPage(const Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback)
{
    // cause intent page no need execute onExecute callback, it just need set some params
    TAG_LOGI(AAFwkTag::INTENT, "execute page intent");
    auto asyncCallback =
        [weak = weak_from_this(), intentId = executeParam->insightIntentId_, want](InsightIntentExecuteResult result) {
            TAG_LOGD(AAFwkTag::INTENT, "execute insightIntent finished, intentId %{public}" PRIu64"", intentId);
            auto abilityImpl = weak.lock();
            if (abilityImpl == nullptr) {
                TAG_LOGE(AAFwkTag::INTENT, "null ability impl");
                return;
            }
            bool bflag = true;
            abilityImpl->HandleForegroundNewState(want, bflag);
            abilityImpl->ExecuteInsightIntentDone(intentId, result);
            abilityImpl->AbilityTransactionCallback(AAFwk::ABILITY_STATE_FOREGROUND_NEW);
        };
    callback->Push(asyncCallback);

    // private function, no need check ability_ validity.
    ability_->ExecuteInsightIntentPage(want, executeParam, std::move(callback));
}

void UIAbilityImpl::PostForegroundInsightIntent()
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null ability_");
        return;
    }

    lifecycleState_ = AAFwk::ABILITY_STATE_FOREGROUND_NEW;

    {
        std::lock_guard<std::mutex> lock(notifyForegroundLock_);
        notifyForegroundByAbility_ = true;
    }

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

bool UIAbilityImpl::HandleExecuteInsightIntentBackground(const AAFwk::Want &want, bool onlyExecuteIntent)
{
    auto executeParam = std::make_shared<InsightIntentExecuteParam>();
    auto ret = InsightIntentExecuteParam::GenerateFromWant(want, *executeParam);
    if (!ret && !onlyExecuteIntent) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid params");
        Background();
        return true;
    }

    TAG_LOGI(AAFwkTag::INTENT,
        "insightIntent bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s"
        "insightIntentName: %{public}s, executeMode: %{public}d, intentId: %{public}" PRIu64"",
        executeParam->bundleName_.c_str(), executeParam->moduleName_.c_str(), executeParam->abilityName_.c_str(),
        executeParam->insightIntentName_.c_str(), executeParam->executeMode_, executeParam->insightIntentId_);

    auto intentCb = std::make_unique<InsightIntentExecutorAsyncCallback>();
    intentCb.reset(InsightIntentExecutorAsyncCallback::Create());
    if (intentCb == nullptr && !onlyExecuteIntent) {
        TAG_LOGE(AAFwkTag::INTENT, "null intentCb");
        Background();
        return true;
    }
    TAG_LOGD(AAFwkTag::INTENT, "lifecycleState_: %{public}d", lifecycleState_);
    if (lifecycleState_ == AAFwk::ABILITY_STATE_INITIAL
        || lifecycleState_ == AAFwk::ABILITY_STATE_STARTED_NEW) {
        ExecuteInsightIntentBackgroundByColdBoot(want, executeParam, std::move(intentCb));
        return false;
    } else {
        ExecuteInsightIntentBackgroundAlreadyStart(want, executeParam, std::move(intentCb));
        return true;
    }
}

void UIAbilityImpl::ExecuteInsightIntentBackgroundByColdBoot(const Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback)
{
    TAG_LOGI(AAFwkTag::INTENT, "execute insight intent background by cold boot");
    auto asyncCallback =
        [weak = weak_from_this(), intentId = executeParam->insightIntentId_](InsightIntentExecuteResult result) {
            TAG_LOGD(AAFwkTag::INTENT, "end, intentId %{public}" PRIu64"", intentId);
            auto abilityImpl = weak.lock();
            if (abilityImpl == nullptr) {
                TAG_LOGE(AAFwkTag::INTENT, "null ability impl");
                return;
            }
            abilityImpl->Background();
            abilityImpl->ExecuteInsightIntentDone(intentId, result);
            abilityImpl->AbilityTransactionCallback(AAFwk::ABILITY_STATE_BACKGROUND_NEW);
        };
    callback->Push(asyncCallback);

    // private function, no need check ability_ validity.
    ability_->ExecuteInsightIntentBackground(want, executeParam, std::move(callback));
}

void UIAbilityImpl::ExecuteInsightIntentBackgroundAlreadyStart(const Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback)
{
    TAG_LOGI(AAFwkTag::INTENT, "execute insight intent background already start");

    auto asyncCallback =
        [weak = weak_from_this(), intentId = executeParam->insightIntentId_](InsightIntentExecuteResult result) {
            TAG_LOGD(AAFwkTag::INTENT, "end, intentId %{public}" PRIu64"", intentId);
            auto abilityImpl = weak.lock();
            if (abilityImpl == nullptr) {
                TAG_LOGE(AAFwkTag::INTENT, "null ability impl");
                return;
            }
            abilityImpl->ExecuteInsightIntentDone(intentId, result);
        };
    callback->Push(asyncCallback);

    // private function, no need check ability_ validity.
    ability_->ExecuteInsightIntentBackground(want, executeParam, std::move(callback));
}
#endif

void UIAbilityImpl::ScheduleCollaborate(const Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ability_");
        return;
    }
    ability_->HandleCollaboration(want);
}

void UIAbilityImpl::ScheduleAbilityRequestFailure(const std::string &requestId, const AppExecFwk::ElementName &element,
    const std::string &message, int32_t resultCode)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "ScheduleAbilityRequestFailure called");
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ability_");
        return;
    }
    ability_->OnAbilityRequestFailure(requestId, element, message, resultCode);
}

void UIAbilityImpl::ScheduleAbilityRequestSuccess(const std::string &requestId, const AppExecFwk::ElementName &element)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "ScheduleAbilityRequestSuccess called");
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ability_");
        return;
    }
    nlohmann::json jsonObject = nlohmann::json {
        { JSON_KEY_ERR_MSG, "Succeeded" },
    };
    ability_->OnAbilityRequestSuccess(requestId, element, jsonObject.dump());
}

void UIAbilityImpl::SetNativeModuleMetaData(const AAFwk::NativeAbilityMetaData& metaData)
{
    nativeModuleMetaData_ = metaData;
    if (metaData.withNativeModule) {
        if (metaData.startupPhase == AAFwk::StartupPhase::PRE_WINDOW) {
            localNativeState_ = LocalNativeState::INIT_PRE_WINDOW;
        } else if (metaData.startupPhase == AAFwk::StartupPhase::PRE_FOREGROUND) {
            localNativeState_ = LocalNativeState::INIT_PRE_FOREGROUND;
        }
    }
}

#ifdef SUPPORT_SCREEN
std::shared_ptr<UIAbility> UIAbilityImpl::GetUIAbility()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ability_");
        return nullptr;
    }
    return ability_;
}
#endif
} // namespace AbilityRuntime
} // namespace OHOS
