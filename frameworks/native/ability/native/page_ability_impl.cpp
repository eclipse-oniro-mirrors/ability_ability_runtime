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

#include "page_ability_impl.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace AppExecFwk {
using AbilityManagerClient = OHOS::AAFwk::AbilityManagerClient;
void PageAbilityImpl::HandleAbilityTransaction(const Want &want, const AAFwk::LifeCycleStateInfo &targetState,
    sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::ABILITY, "start, sourceState:%{public}d, targetState:%{public}d, "
             "isNewWant:%{public}d, sceneFlag:%{public}d",
        lifecycleState_, targetState.state, targetState.isNewWant, targetState.sceneFlag);
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null ability_");
        return;
    }

    auto abilityContext = ability_->GetAbilityContext();
    if (abilityContext != nullptr && abilityContext->IsTerminating()
        && targetState.state == AAFwk::ABILITY_STATE_INACTIVE) {
        TAG_LOGE(AAFwkTag::ABILITY, "invalid translate state");
        return;
    }

    ability_->sceneFlag_ = targetState.sceneFlag;
    if ((lifecycleState_ == targetState.state) && !targetState.isNewWant) {
        if (targetState.state == AAFwk::ABILITY_STATE_FOREGROUND_NEW) {
            ability_->RequestFocus(want);
            AbilityManagerClient::GetInstance()->AbilityTransitionDone(token_, targetState.state, GetRestoreData());
        }
        TAG_LOGE(AAFwkTag::ABILITY, "lifeCycleStat:org=Dst");
        return;
    }

    if (lifecycleState_ == AAFwk::ABILITY_STATE_BACKGROUND || lifecycleState_ == AAFwk::ABILITY_STATE_BACKGROUND_NEW) {
        if (targetState.state == AAFwk::ABILITY_STATE_ACTIVE || targetState.state == AAFwk::ABILITY_STATE_INACTIVE) {
            TAG_LOGE(AAFwkTag::ABILITY, "invalid state");
            return;
        }
    }

    SetLifeCycleStateInfo(targetState);

    if (lifecycleState_ == AAFwk::ABILITY_STATE_INITIAL) {
        ability_->SetStartAbilitySetting(targetState.setting);
        Start(want, sessionInfo);
        CheckAndRestore();
    }

    if (lifecycleState_ == AAFwk::ABILITY_STATE_ACTIVE &&
        targetState.state != AAFwk::ABILITY_STATE_FOREGROUND_NEW) {
        Inactive();
    }

    if (targetState.state == AAFwk::ABILITY_STATE_BACKGROUND_NEW ||
        targetState.state == AAFwk::ABILITY_STATE_BACKGROUND) {
        CheckAndSave();
    }

    if (AbilityTransaction(want, targetState)) {
        TAG_LOGI(AAFwkTag::ABILITY, "done, notify ams");
        AbilityManagerClient::GetInstance()->AbilityTransitionDone(token_, targetState.state, GetRestoreData());
    }
}

/**
 * @brief Handling the life cycle switching of PageAbility in switch.
 *
 * @param want Indicates the structure containing information about the ability.
 * @param targetState The life cycle state to switch to.
 *
 * @return return true if the lifecycle transaction successfully, otherwise return false.
 *
 */
bool PageAbilityImpl::AbilityTransaction(const Want &want, const AAFwk::LifeCycleStateInfo &targetState)
{
    TAG_LOGD(AAFwkTag::ABILITY, "begin");
    bool ret = true;
    switch (targetState.state) {
        case AAFwk::ABILITY_STATE_INITIAL: {
            if (lifecycleState_ == AAFwk::ABILITY_STATE_INACTIVE) {
                Background();
            }
            Stop();
            break;
        }
        case AAFwk::ABILITY_STATE_INACTIVE: {
            if (lifecycleState_ == AAFwk::ABILITY_STATE_ACTIVE) {
                Inactive();
            }
            ret = false;
            break;
        }
        case AAFwk::ABILITY_STATE_FOREGROUND_NEW: {
            AbilityTransactionForeground(want, targetState);
            break;
        }
        case AAFwk::ABILITY_STATE_ACTIVE: {
            if (lifecycleState_ == AAFwk::ABILITY_STATE_BACKGROUND) {
                Foreground(want);
            }
            Active();
            ret = false;
            break;
        }
        case AAFwk::ABILITY_STATE_BACKGROUND_NEW: {
            if (lifecycleState_ != AAFwk::ABILITY_STATE_INACTIVE) {
                Inactive();
            }
            Background();
            break;
        }
        default: {
            ret = false;
            TAG_LOGE(AAFwkTag::ABILITY, "error");
            break;
        }
    }
    TAG_LOGD(AAFwkTag::ABILITY, "end: ret = %{public}d", static_cast<int>(ret));
    return ret;
}

void PageAbilityImpl::AbilityTransactionForeground(const Want &want, const AAFwk::LifeCycleStateInfo &targetState)
{
    if (targetState.isNewWant) {
        NewWant(want);
    }
    SetUriString(targetState.caller.deviceId + "/" + targetState.caller.bundleName + "/" +
                 targetState.caller.abilityName);
    if (lifecycleState_ == AAFwk::ABILITY_STATE_BACKGROUND_NEW ||
        lifecycleState_ == AAFwk::ABILITY_STATE_BACKGROUND) {
        Foreground(want);
    } else {
        if (ability_) {
            ability_->RequestFocus(want);
        }
    }
}

/**
 * @brief Execution the KeyDown callback of the ability
 * @param keyEvent Indicates the key-down event.
 *
 * @return Returns true if this event is handled and will not be passed further; returns false if this event is
 * not handled and should be passed to other handlers.
 *
 */
void PageAbilityImpl::DoKeyDown(const std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    TAG_LOGI(AAFwkTag::ABILITY, "begin");
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null ability_");
        return;
    }
    auto abilityInfo = ability_->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null abilityInfo");
        return;
    }
    TAG_LOGI(AAFwkTag::ABILITY, "called %{public}s,Focus:%{public}s",
        abilityInfo->name.c_str(),
        ability_->HasWindowFocus() ? "true" : "false");

    ability_->OnKeyDown(keyEvent);
}

/**
 * @brief Execution the KeyUp callback of the ability
 * @param keyEvent Indicates the key-up event.
 *
 * @return Returns true if this event is handled and will not be passed further; returns false if this event is
 * not handled and should be passed to other handlers.
 *
 */
void PageAbilityImpl::DoKeyUp(const std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    TAG_LOGI(AAFwkTag::ABILITY, "begin");
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null ability_");
        return;
    }
    auto abilityInfo = ability_->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null abilityInfo");
        return;
    }
    TAG_LOGI(AAFwkTag::ABILITY, "called %{public}s,Focus:%{public}s",
        abilityInfo->name.c_str(),
        ability_->HasWindowFocus() ? "true" : "false");

    ability_->OnKeyUp(keyEvent);
}

/**
 * @brief Called when a touch event is dispatched to this ability. The default implementation of this callback
 * does nothing and returns false.
 * @param touchEvent Indicates information about the touch event.
 *
 * @return Returns true if the event is handled; returns false otherwise.
 *
 */
void PageAbilityImpl::DoPointerEvent(std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    TAG_LOGI(AAFwkTag::ABILITY, "begin");
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null ability_");
        return;
    }
    auto abilityInfo = ability_->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null abilityInfo");
        return;
    }
    TAG_LOGI(AAFwkTag::ABILITY, "called %{public}s,Focus:%{public}s",
        abilityInfo->name.c_str(),
        ability_->HasWindowFocus() ? "true" : "false");

    ability_->OnPointerEvent(pointerEvent);
}
}  // namespace AppExecFwk
}  // namespace OHOS
