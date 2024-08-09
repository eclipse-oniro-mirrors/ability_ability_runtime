/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "ability_impl.h"

#include "ability_runtime/js_ability.h"
#include "ability_transaction_callback_info.h"
#include "data_ability_predicates.h"
#include "freeze_util.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "ohos_application.h"
#ifdef SUPPORT_SCREEN
#include "scene_board_judgement.h"
#endif // SUPPORT_SCREEN
#include "time_util.h"
#include "values_bucket.h"

namespace OHOS {
using AbilityRuntime::FreezeUtil;
namespace AppExecFwk {
namespace {
const std::string PERMISSION_KEY = "ohos.user.grant.permission";
const std::string GRANTED_RESULT_KEY = "ohos.user.grant.permission.result";
}

void AbilityImpl::Init(const std::shared_ptr<OHOSApplication> &application,
                       const std::shared_ptr<AbilityLocalRecord> &record,
                       std::shared_ptr<Ability> &ability,
                       std::shared_ptr<AbilityHandler> &handler,
                       const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if ((token == nullptr) || (application == nullptr) || (handler == nullptr) || (record == nullptr) ||
        ability == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null token or application or handler or record or ability");
        return;
    }

    token_ = record->GetToken();
    ability_ = ability;
    handler_ = handler;
    auto info = record->GetAbilityInfo();
    isStageBasedModel_ = info && info->isStageBasedModel;
#ifdef SUPPORT_SCREEN
    if (info && info->type == AbilityType::PAGE) {
        ability_->SetSceneListener(sptr<WindowLifeCycleImpl>(new WindowLifeCycleImpl(token_, shared_from_this())));
    }
#endif
    ability_->Init(record->GetAbilityInfo(), application, handler, token);
    lifecycleState_ = AAFwk::ABILITY_STATE_INITIAL;
    abilityLifecycleCallbacks_ = application;
    TAG_LOGD(AAFwkTag::ABILITY, "end");
}

void AbilityImpl::Start(const Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (ability_ == nullptr || ability_->GetAbilityInfo() == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null ability_ or abilityLifecycleCallbacks_");
        return;
    }
#ifdef SUPPORT_SCREEN
    if ((ability_->GetAbilityInfo()->type == AbilityType::PAGE) &&
        (!ability_->GetAbilityInfo()->isStageBasedModel)) {
        ability_->HandleCreateAsContinuation(want);
    }

    if ((ability_->GetAbilityInfo()->type == AbilityType::PAGE) &&
        (ability_->GetAbilityInfo()->isStageBasedModel)) {
        ability_->HandleCreateAsRecovery(want);
    }
#endif
    TAG_LOGD(AAFwkTag::ABILITY, "call onStart");
    ability_->OnStart(want, sessionInfo);
#ifdef SUPPORT_SCREEN
    if ((ability_->GetAbilityInfo()->type == AppExecFwk::AbilityType::PAGE) &&
        (ability_->GetAbilityInfo()->isStageBasedModel)) {
        lifecycleState_ = AAFwk::ABILITY_STATE_STARTED_NEW;
    } else {
#endif
        if (ability_->GetAbilityInfo()->type == AbilityType::DATA) {
            lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
        } else {
            lifecycleState_ = AAFwk::ABILITY_STATE_INACTIVE;
        }
#ifdef SUPPORT_SCREEN
    }
#endif

    TAG_LOGD(AAFwkTag::ABILITY, "end");
}

void AbilityImpl::Stop()
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (ability_ == nullptr || ability_->GetAbilityInfo() == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "AbilityImpl::Stop ability_ or abilityLifecycleCallbacks_ is nullptr");
        return;
    }

    ability_->OnStop();
    StopCallback();
    TAG_LOGD(AAFwkTag::ABILITY, "end");
}

void AbilityImpl::Stop(bool &isAsyncCallback)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (ability_ == nullptr || ability_->GetAbilityInfo() == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "AbilityImpl::Stop ability_ or abilityLifecycleCallbacks_ is nullptr");
        isAsyncCallback = false;
        return;
    }

    auto *callbackInfo = AbilityTransactionCallbackInfo<>::Create();
    if (callbackInfo == nullptr) {
        ability_->OnStop();
        StopCallback();
        isAsyncCallback = false;
        return;
    }
    std::weak_ptr<AbilityImpl> weakPtr = shared_from_this();
    auto asyncCallback = [abilityImplWeakPtr = weakPtr, state = ABILITY_STATE_INITIAL]() {
        auto abilityImpl = abilityImplWeakPtr.lock();
        if (abilityImpl == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITY, "abilityImpl is nullptr.");
            return;
        }
        abilityImpl->StopCallback();
        abilityImpl->AbilityTransactionCallback(state);
    };
    callbackInfo->Push(asyncCallback);

    ability_->OnStop(callbackInfo, isAsyncCallback);
    if (!isAsyncCallback) {
        StopCallback();
        AbilityTransactionCallbackInfo<>::Destroy(callbackInfo);
    }
    // else: callbackInfo will be destroyed after the async callback
    TAG_LOGD(AAFwkTag::ABILITY, "end");
}

void AbilityImpl::StopCallback()
{
    if (ability_ == nullptr || ability_->GetAbilityInfo() == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "AbilityImpl::Stop ability_ or abilityLifecycleCallbacks_ is nullptr");
        return;
    }
#ifdef SUPPORT_SCREEN
    if ((ability_->GetAbilityInfo()->type == AppExecFwk::AbilityType::PAGE) &&
        (ability_->GetAbilityInfo()->isStageBasedModel)) {
        lifecycleState_ = AAFwk::ABILITY_STATE_STOPED_NEW;
    } else {
#endif
        lifecycleState_ = AAFwk::ABILITY_STATE_INITIAL;
#ifdef SUPPORT_SCREEN
    }
#endif
    ability_->DestroyInstance(); // Release window and ability.
}

void AbilityImpl::Active()
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (ability_ == nullptr || ability_->GetAbilityInfo() == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "AbilityImpl::Active ability_ or abilityLifecycleCallbacks_ is nullptr");
        return;
    }

    ability_->OnActive();
#ifdef SUPPORT_SCREEN
    if ((lifecycleState_ == AAFwk::ABILITY_STATE_INACTIVE) && (ability_->GetAbilityInfo()->type == AbilityType::PAGE)) {
        ability_->OnTopActiveAbilityChanged(true);
        ability_->OnWindowFocusChanged(true);
    }
#endif
    lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
    TAG_LOGD(AAFwkTag::ABILITY, "end");
}

void AbilityImpl::Inactive()
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (ability_ == nullptr || ability_->GetAbilityInfo() == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "AbilityImpl::Inactive ability_ or abilityLifecycleCallbacks_ is nullptr");
        return;
    }

    ability_->OnInactive();
#ifdef SUPPORT_SCREEN
    if ((lifecycleState_ == AAFwk::ABILITY_STATE_ACTIVE) && (ability_->GetAbilityInfo()->type == AbilityType::PAGE)) {
        ability_->OnTopActiveAbilityChanged(false);
        ability_->OnWindowFocusChanged(false);
    }
#endif
    lifecycleState_ = AAFwk::ABILITY_STATE_INACTIVE;
    TAG_LOGD(AAFwkTag::ABILITY, "end");
}


int32_t AbilityImpl::Share(WantParams &wantParam)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "AbilityImpl::OnShare ability_ is nullptr ");
        return ERR_INVALID_VALUE;
    }
    return ability_->OnShare(wantParam);
}

bool AbilityImpl::IsStageBasedModel() const
{
    return isStageBasedModel_;
}

void AbilityImpl::DispatchSaveAbilityState()
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (ability_ == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY,
            "AbilityImpl::DispatchSaveAbilityState ability_ or abilityLifecycleCallbacks_ is nullptr");
        return;
    }

    needSaveDate_ = true;
    TAG_LOGD(AAFwkTag::ABILITY, "end");
}

void AbilityImpl::DispatchRestoreAbilityState(const PacMap &inState)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "AbilityImpl::DispatchRestoreAbilityState ability_ is nullptr");
        return;
    }

    hasSaveData_ = true;
    restoreData_ = inState;
    TAG_LOGD(AAFwkTag::ABILITY, "end");
}

void AbilityImpl::HandleAbilityTransaction(const Want &want, const AAFwk::LifeCycleStateInfo &targetState,
    sptr<AAFwk::SessionInfo> sessionInfo)
{}

void AbilityImpl::HandleShareData(const int32_t &requestCode)
{}

void AbilityImpl::AbilityTransactionCallback(const AAFwk::AbilityLifeCycleState &state)
{}

sptr<IRemoteObject> AbilityImpl::ConnectAbility(const Want &want)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "AbilityImpl::ConnectAbility ability_ is nullptr");
        return nullptr;
    }
    sptr<IRemoteObject> object = ability_->OnConnect(want);
    lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
    TAG_LOGD(AAFwkTag::ABILITY, "end");

    return object;
}

void AbilityImpl::DisconnectAbility(const Want &want)
{
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "Disconnect ability error, ability_ is nullptr.");
        return;
    }
    TAG_LOGD(AAFwkTag::ABILITY, "Disconnect ability begin, ability:%{public}s.", ability_->GetAbilityName().c_str());
    ability_->OnDisconnect(want);
}

void AbilityImpl::CommandAbility(const Want &want, bool restart, int startId)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "AbilityImpl::CommandAbility ability_ is nullptr");
        return;
    }
    ability_->OnCommand(want, restart, startId);
    lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
    TAG_LOGD(AAFwkTag::ABILITY, "end");
}
#ifdef SUPPORT_SCREEN
bool AbilityImpl::PrepareTerminateAbility()
{
    TAG_LOGD(AAFwkTag::ABILITY, "call");
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "ability_ is nullptr.");
        return false;
    }
    bool ret = ability_->OnPrepareTerminate();
    TAG_LOGD(AAFwkTag::ABILITY, "end, ret = %{public}d", ret);
    return ret;
}
#endif
int AbilityImpl::GetCurrentState()
{
    return lifecycleState_;
}

void AbilityImpl::SendResult(int requestCode, int resultCode, const Want &resultData)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "AbilityImpl::SendResult ability_ is nullptr");
        return;
    }

    ability_->OnAbilityResult(requestCode, resultCode, resultData);
    // for api5 FeatureAbility::startAbilityForResult
    ability_->OnFeatureAbilityResult(requestCode, resultCode, resultData);
    TAG_LOGD(AAFwkTag::ABILITY, "end");
}

void AbilityImpl::NewWant(const Want &want)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "AbilityImpl::NewWant ability_ is nullptr");
        return;
    }
    ability_->SetWant(want);
    ability_->OnNewWant(want);
#ifdef SUPPORT_SCREEN
    ability_->ContinuationRestore(want);
#endif
    TAG_LOGD(AAFwkTag::ABILITY, "end");
}

std::vector<std::string> AbilityImpl::GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter)
{
    TAG_LOGD(AAFwkTag::ABILITY, "AbilityImpl::GetFileTypes");
    std::vector<std::string> types;
    return types;
}

int AbilityImpl::OpenFile(const Uri &uri, const std::string &mode)
{
    TAG_LOGD(AAFwkTag::ABILITY, "AbilityImpl::OpenFile");
    return -1;
}

int AbilityImpl::OpenRawFile(const Uri &uri, const std::string &mode)
{
    TAG_LOGD(AAFwkTag::ABILITY, "AbilityImpl::OpenRawFile");
    return -1;
}

int AbilityImpl::Insert(const Uri &uri, const NativeRdb::ValuesBucket &value)
{
    TAG_LOGD(AAFwkTag::ABILITY, "AbilityImpl::Insert");
    return -1;
}

std::shared_ptr<AppExecFwk::PacMap> AbilityImpl::Call(
    const Uri &uri, const std::string &method, const std::string &arg, const AppExecFwk::PacMap &pacMap)
{
    TAG_LOGD(AAFwkTag::ABILITY, "AbilityImpl::Call");
    return nullptr;
}

int AbilityImpl::Update(
    const Uri &uri, const NativeRdb::ValuesBucket &value, const NativeRdb::DataAbilityPredicates &predicates)
{
    TAG_LOGD(AAFwkTag::ABILITY, "AbilityImpl::Update");
    return -1;
}

int AbilityImpl::Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
{
    TAG_LOGD(AAFwkTag::ABILITY, "AbilityImpl::Delete");
    return -1;
}

std::shared_ptr<NativeRdb::AbsSharedResultSet> AbilityImpl::Query(
    const Uri &uri, std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
{
    TAG_LOGD(AAFwkTag::ABILITY, "AbilityImpl::Query");
    return nullptr;
}

std::string AbilityImpl::GetType(const Uri &uri)
{
    TAG_LOGD(AAFwkTag::ABILITY, "AbilityImpl::GetType");
    return "";
}

bool AbilityImpl::Reload(const Uri &uri, const PacMap &extras)
{
    return false;
}

int AbilityImpl::BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values)
{
    TAG_LOGD(AAFwkTag::ABILITY, "AbilityImpl::BatchInsert");
    return -1;
}

void AbilityImpl::SetUriString(const std::string &uri)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "AbilityImpl::SetUriString ability_ is nullptr");
        return;
    }
    ability_->SetUriString(uri);
    TAG_LOGD(AAFwkTag::ABILITY, "end");
}

void AbilityImpl::SetLifeCycleStateInfo(const AAFwk::LifeCycleStateInfo &info)
{
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "AbilityImpl::SetLifeCycleStateInfo ability_ is nullptr");
        return;
    }
    ability_->SetLifeCycleStateInfo(info);
}

bool AbilityImpl::CheckAndRestore()
{
    TAG_LOGD(AAFwkTag::ABILITY, "AbilityImpl::CheckAndRestore called start");
    if (!hasSaveData_) {
        TAG_LOGE(AAFwkTag::ABILITY, "AbilityImpl::CheckAndRestore hasSaveData_ is false");
        return false;
    }

    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "AbilityImpl::CheckAndRestore ability_ is nullptr");
        return false;
    }
    ability_->OnRestoreAbilityState(restoreData_);

    TAG_LOGD(AAFwkTag::ABILITY, "AbilityImpl::CheckAndRestore called end");
    return true;
}

bool AbilityImpl::CheckAndSave()
{
    TAG_LOGD(AAFwkTag::ABILITY, "AbilityImpl::CheckAndSave called start");
    if (!needSaveDate_) {
        TAG_LOGE(AAFwkTag::ABILITY, "AbilityImpl::CheckAndSave needSaveDate_ is false");
        return false;
    }

    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "AbilityImpl::CheckAndSave ability_ is nullptr");
        return false;
    }

    ability_->OnSaveAbilityState(restoreData_);

    needSaveDate_ = false;

    TAG_LOGD(AAFwkTag::ABILITY, "AbilityImpl::CheckAndSave called end");
    return true;
}

PacMap &AbilityImpl::GetRestoreData()
{
    return restoreData_;
}

void AbilityImpl::SetCallingContext(const std::string &deviceId, const std::string &bundleName,
    const std::string &abilityName, const std::string &moduleName)
{
    if (ability_ != nullptr) {
        ability_->SetCallingContext(deviceId, bundleName, abilityName, moduleName);
    }
}

Uri AbilityImpl::NormalizeUri(const Uri &uri)
{
    TAG_LOGD(AAFwkTag::ABILITY, "AbilityImpl::NormalizeUri");
    return uri;
}

Uri AbilityImpl::DenormalizeUri(const Uri &uri)
{
    TAG_LOGD(AAFwkTag::ABILITY, "AbilityImpl::DenormalizeUri");
    return uri;
}

void AbilityImpl::ScheduleUpdateConfiguration(const Configuration &config)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "AbilityImpl::ScheduleUpdateConfiguration ability_ is nullptr");
        return;
    }

    if (lifecycleState_ != AAFwk::ABILITY_STATE_INITIAL) {
        TAG_LOGI(AAFwkTag::ABILITY, "ability name: [%{public}s]", ability_->GetAbilityName().c_str());
        ability_->OnConfigurationUpdatedNotify(config);
    }

    TAG_LOGD(AAFwkTag::ABILITY, "end");
}

std::shared_ptr<AbilityPostEventTimeout> AbilityImpl::CreatePostEventTimeouter(std::string taskstr)
{
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "AbilityImpl::CreatePostEventTimeouter ability_ is nullptr");
        return nullptr;
    }

    return ability_->CreatePostEventTimeouter(taskstr);
}

std::vector<std::shared_ptr<DataAbilityResult>> AbilityImpl::ExecuteBatch(
    const std::vector<std::shared_ptr<DataAbilityOperation>> &operations)
{
    TAG_LOGD(AAFwkTag::ABILITY, "AbilityImpl::ExecuteBatch");
    std::vector<std::shared_ptr<DataAbilityResult>> results;
    return results;
}

void AbilityImpl::ContinueAbility(const std::string& deviceId, uint32_t versionCode)
{
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "AbilityImpl::ContinueAbility ability_ is nullptr");
        return;
    }
    ability_->ContinueAbilityWithStack(deviceId, versionCode);
}

void AbilityImpl::NotifyContinuationResult(int32_t result)
{
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "AbilityImpl::NotifyContinuationResult ability_ is nullptr");
        return;
    }
    ability_->OnCompleteContinuation(result);
}

void AbilityImpl::NotifyMemoryLevel(int32_t level)
{
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "AbilityImpl::NotifyMemoryLevel ability_ is nullptr");
        return;
    }
    ability_->OnMemoryLevel(level);
}

#ifdef SUPPORT_SCREEN
void AbilityImpl::AfterUnFocused()
{
    AfterFocusedCommon(false);
}

void AbilityImpl::AfterFocused()
{
    AfterFocusedCommon(true);
}

void AbilityImpl::AfterFocusedCommon(bool isFocused)
{
    auto task = [abilityImpl = weak_from_this(), focuseMode = isFocused]() {
        auto impl = abilityImpl.lock();
        if (impl == nullptr) {
            return;
        }

        if (!impl->ability_ || !impl->ability_->GetAbilityInfo()) {
            TAG_LOGW(AAFwkTag::ABILITY,
                "AbilityImpl::%{public}s failed", focuseMode ? "AfterFocused" : "AfterUnFocused");
            return;
        }
        TAG_LOGI(AAFwkTag::ABILITY, "isStageBasedModel: %{public}d",
            impl->ability_->GetAbilityInfo()->isStageBasedModel);

        if (impl->ability_->GetAbilityInfo()->isStageBasedModel) {
            auto abilityContext = impl->ability_->GetAbilityContext();
            if (abilityContext == nullptr) {
                return;
            }
            auto applicationContext = abilityContext->GetApplicationContext();
            if (applicationContext == nullptr || applicationContext->IsAbilityLifecycleCallbackEmpty()) {
                return;
            }
            auto& jsAbility = static_cast<AbilityRuntime::JsAbility&>(*(impl->ability_));
            if (focuseMode) {
                applicationContext->DispatchWindowStageFocus(jsAbility.GetJsAbility(),  jsAbility.GetJsWindowStage());
            } else {
                applicationContext->DispatchWindowStageUnfocus(jsAbility.GetJsAbility(), jsAbility.GetJsWindowStage());
            }
            return;
        }

        if (impl->ability_->GetWant() == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITY, "want is nullptr.");
            return;
        }
        auto info = impl->ability_->GetLifeCycleStateInfo();
        if (focuseMode) {
            info.state = AbilityLifeCycleState::ABILITY_STATE_ACTIVE;
        } else {
            info.state = AbilityLifeCycleState::ABILITY_STATE_INACTIVE;
        }
        info.isNewWant = false;
        impl->HandleAbilityTransaction(*(impl->ability_->GetWant()), info);
    };

    if (handler_) {
        handler_->PostTask(task, "AbilityImpl:AfterFocusedCommon");
    }
    TAG_LOGD(AAFwkTag::ABILITY, "end.");
}

void AbilityImpl::WindowLifeCycleImpl::AfterForeground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::ABILITY, "Lifecycle: Call.");
    auto owner = owner_.lock();
    if (owner == nullptr || !owner->IsStageBasedModel()) {
        TAG_LOGE(AAFwkTag::ABILITY, "Not stage mode ability or abilityImpl is nullptr.");
        return;
    }
    FreezeUtil::LifecycleFlow flow = { token_, FreezeUtil::TimeoutState::FOREGROUND };
    std::string entry = std::to_string(AbilityRuntime::TimeUtil::SystemTimeMillisecond()) +
        "; AbilityImpl::WindowLifeCycleImpl::AfterForeground; the foreground lifecycle.";
    FreezeUtil::GetInstance().AddLifecycleEvent(flow, entry);

    bool needNotifyAMS = false;
    {
        std::lock_guard<std::mutex> lock(owner->notifyForegroundLock_);
        if (owner->notifyForegroundByAbility_) {
            owner->notifyForegroundByAbility_ = false;
            needNotifyAMS = true;
        } else {
            TAG_LOGD(AAFwkTag::ABILITY, "Notify foreground by window, but client's foreground is running.");
            owner->notifyForegroundByWindow_ = true;
        }
    }

    if (needNotifyAMS) {
        TAG_LOGI(AAFwkTag::ABILITY, "Lifecycle: window notify ability manager service.");
        PacMap restoreData;
        auto ret = AbilityManagerClient::GetInstance()->AbilityTransitionDone(token_,
            AbilityLifeCycleState::ABILITY_STATE_FOREGROUND_NEW, restoreData);
        if (ret == ERR_OK) {
            FreezeUtil::GetInstance().DeleteLifecycleEvent(flow);
        }
    }
}

void AbilityImpl::WindowLifeCycleImpl::AfterBackground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto owner = owner_.lock();
    if (owner && !owner->IsStageBasedModel()) {
        TAG_LOGW(AAFwkTag::ABILITY, "Not stage.");
        return;
    }
    FreezeUtil::LifecycleFlow flow = { token_, FreezeUtil::TimeoutState::BACKGROUND };
    std::string entry = std::to_string(AbilityRuntime::TimeUtil::SystemTimeMillisecond()) +
        "; AbilityImpl::WindowLifeCycleImpl::AfterBackground; the background lifecycle.";
    FreezeUtil::GetInstance().AddLifecycleEvent(flow, entry);

    TAG_LOGI(AAFwkTag::ABILITY, "Lifecycle: window after background.");
    PacMap restoreData;
    auto ret = AbilityManagerClient::GetInstance()->AbilityTransitionDone(token_,
        AbilityLifeCycleState::ABILITY_STATE_BACKGROUND_NEW, restoreData);
    if (ret == ERR_OK) {
        FreezeUtil::GetInstance().DeleteLifecycleEvent(flow);
    }
}

void AbilityImpl::WindowLifeCycleImpl::AfterFocused()
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    auto owner = owner_.lock();
    if (owner) {
        owner->AfterFocused();
    }
    TAG_LOGD(AAFwkTag::ABILITY, "end");
}

void AbilityImpl::WindowLifeCycleImpl::AfterUnfocused()
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    auto owner = owner_.lock();
    if (owner) {
        owner->AfterUnFocused();
    }
    TAG_LOGD(AAFwkTag::ABILITY, "end");
}

void AbilityImpl::WindowLifeCycleImpl::ForegroundFailed(int32_t type)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    PacMap restoreData;
    switch (type) {
        case static_cast<int32_t>(OHOS::Rosen::WMError::WM_ERROR_INVALID_OPERATION): {
            TAG_LOGD(AAFwkTag::ABILITY, "window was freezed.");
            AbilityManagerClient::GetInstance()->AbilityTransitionDone(token_,
                AbilityLifeCycleState::ABILITY_STATE_WINDOW_FREEZED, restoreData);
            break;
        }
        case static_cast<int32_t>(OHOS::Rosen::WMError::WM_ERROR_INVALID_WINDOW_MODE_OR_SIZE): {
            auto owner = owner_.lock();
            if (owner == nullptr || !owner->IsStageBasedModel()) {
                TAG_LOGE(AAFwkTag::ABILITY, "Not stage mode ability or abilityImpl is nullptr.");
                return;
            }

            TAG_LOGD(AAFwkTag::ABILITY, "The ability is stage mode, schedule foreground invalid mode.");
            AbilityManagerClient::GetInstance()->AbilityTransitionDone(token_,
                AbilityLifeCycleState::ABILITY_STATE_INVALID_WINDOW_MODE, restoreData);
            break;
        }
        case static_cast<int32_t>(OHOS::Rosen::WMError::WM_DO_NOTHING): {
            AbilityManagerClient::GetInstance()->AbilityTransitionDone(token_,
                AbilityLifeCycleState::ABILITY_STATE_DO_NOTHING, restoreData);
            break;
        }
        default: {
            AbilityManagerClient::GetInstance()->AbilityTransitionDone(token_,
                AbilityLifeCycleState::ABILITY_STATE_FOREGROUND_FAILED, restoreData);
        }
    }
}

void AbilityImpl::Foreground(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (ability_ == nullptr || ability_->GetAbilityInfo() == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "AbilityImpl::Foreground ability_ or abilityLifecycleCallbacks_ is nullptr");
        return;
    }

    TAG_LOGD(AAFwkTag::ABILITY, "AbilityImpl::Foreground");
    ability_->OnForeground(want);
    if ((ability_->GetAbilityInfo()->type == AppExecFwk::AbilityType::PAGE) &&
        (ability_->GetAbilityInfo()->isStageBasedModel)) {
        lifecycleState_ = AAFwk::ABILITY_STATE_FOREGROUND_NEW;
    } else {
        lifecycleState_ = AAFwk::ABILITY_STATE_INACTIVE;
    }
    {
        std::lock_guard<std::mutex> lock(notifyForegroundLock_);
        notifyForegroundByAbility_ = true;
    }
    TAG_LOGI(AAFwkTag::ABILITY, "end");
}

void AbilityImpl::WindowLifeCycleImpl::BackgroundFailed(int32_t type)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (type == static_cast<int32_t>(OHOS::Rosen::WMError::WM_DO_NOTHING)) {
        PacMap restoreData;
        AbilityManagerClient::GetInstance()->AbilityTransitionDone(token_,
            AbilityLifeCycleState::ABILITY_STATE_BACKGROUND_FAILED, restoreData);
    }
}

void AbilityImpl::Background()
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (ability_ == nullptr || ability_->GetAbilityInfo() == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null ability_ or abilityLifecycleCallbacks_");
        return;
    }
    ability_->OnLeaveForeground();
    ability_->OnBackground();
    if ((ability_->GetAbilityInfo()->type == AppExecFwk::AbilityType::PAGE) &&
        (ability_->GetAbilityInfo()->isStageBasedModel)) {
        lifecycleState_ = AAFwk::ABILITY_STATE_BACKGROUND_NEW;
    } else {
        lifecycleState_ = AAFwk::ABILITY_STATE_BACKGROUND;
    }
    TAG_LOGD(AAFwkTag::ABILITY, "end");
}

void AbilityImpl::DoKeyDown(const std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
}

void AbilityImpl::DoKeyUp(const std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
}

void AbilityImpl::DoPointerEvent(std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
}

void AbilityImpl::InputEventConsumerImpl::OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const
{
    int32_t code = keyEvent->GetKeyAction();
    if (code == MMI::KeyEvent::KEY_ACTION_DOWN) {
        abilityImpl_->DoKeyDown(keyEvent);
        TAG_LOGD(AAFwkTag::ABILITY, "keyAction: %{public}d", code);
    } else if (code == MMI::KeyEvent::KEY_ACTION_UP) {
        abilityImpl_->DoKeyUp(keyEvent);
        TAG_LOGD(AAFwkTag::ABILITY, "keyAction: %{public}d", code);
    }
}

void AbilityImpl::InputEventConsumerImpl::OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    abilityImpl_->DoPointerEvent(pointerEvent);
}
#endif
}  // namespace AppExecFwk
}  // namespace OHOS
