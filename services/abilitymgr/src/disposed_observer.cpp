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

#include "disposed_observer.h"

#include "interceptor/disposed_rule_interceptor.h"
#include "ability_record.h"
#include "modal_system_ui_extension.h"
#include "want_params.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr const char* UIEXTENSION_MODAL_TYPE = "ability.want.params.modalType";
constexpr const char* INTERCEPT_MISSION_ID = "intercept_missionId";
constexpr const char* IS_EMBEDDABLE_SERVICE = "ohos.param.isCallerEmbeddableUIExtension";

bool IsEmbeddableStart(int32_t screenMode)
{
    return screenMode == AAFwk::EMBEDDED_FULL_SCREEN_MODE ||
        screenMode == AAFwk::EMBEDDED_HALF_SCREEN_MODE;
}
}

DisposedObserver::DisposedObserver(const AppExecFwk::DisposedRule &disposedRule,
    const std::shared_ptr<DisposedRuleInterceptor> &interceptor, int32_t uid)
    : interceptor_(interceptor), disposedRule_(disposedRule), uid_(uid)
{}

void DisposedObserver::OnAbilityStateChanged(const AppExecFwk::AbilityStateData &abilityStateData)
{
    std::lock_guard<ffrt::mutex> guard(observerLock_);
    if (abilityStateData.abilityState != static_cast<int32_t>(AppExecFwk::AbilityState::ABILITY_STATE_FOREGROUND)) {
        return;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call");
    token_ = abilityStateData.token;
    auto abilityRecord = Token::GetAbilityRecordByToken(token_);
    if (abilityRecord && !abilityRecord->GetAbilityInfo().isStageBasedModel) {
        auto systemUIExtension = std::make_shared<OHOS::Rosen::ModalSystemUiExtension>();
        Want want = *disposedRule_.want;
        want.SetParam(UIEXTENSION_MODAL_TYPE, 1);
        auto sessionInfo = abilityRecord->GetSessionInfo();
        if (sessionInfo != nullptr) {
            want.SetParam(INTERCEPT_MISSION_ID, sessionInfo->persistentId);
        } else {
            want.SetParam(INTERCEPT_MISSION_ID, abilityRecord->GetMissionId());
        }
        TAG_LOGD(AAFwkTag::ABILITYMGR, "FA modal system");
        bool ret = IN_PROCESS_CALL(systemUIExtension->CreateModalUIExtension(want));
        if (!ret) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "call failed");
        }
        interceptor_->UnregisterObserver(abilityStateData.uid);
    }
}

void DisposedObserver::OnPageShow(const AppExecFwk::PageStateData &pageStateData)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "recv onPageShow");
    if (pageStateData.uid != uid_) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "currentUid:%{public}d, paramUid:%{public}d", uid_, pageStateData.uid);
        return;
    }
    if (disposedRule_.componentType == AppExecFwk::ComponentType::UI_ABILITY) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Call");
        int ret = IN_PROCESS_CALL(AbilityManagerClient::GetInstance()->StartAbility(*disposedRule_.want));
        if (ret != ERR_OK) {
            interceptor_->UnregisterObserver(pageStateData.uid);
            TAG_LOGE(AAFwkTag::ABILITYMGR, "call failed");
            return;
        }
    }
    if (disposedRule_.componentType == AppExecFwk::ComponentType::UI_EXTENSION) {
        int ret = ExecuteUIExtension(pageStateData);
        if (ret != ERR_OK) {
            interceptor_->UnregisterObserver(pageStateData.uid);
            TAG_LOGE(AAFwkTag::ABILITYMGR, "call failed");
            return;
        }
    }
    interceptor_->UnregisterObserver(pageStateData.uid);
}

ErrCode DisposedObserver::ExecuteUIExtension(const AppExecFwk::PageStateData &pageStateData)
{
    auto abilityRecord = Token::GetAbilityRecordByToken(token_);
    Want want = *disposedRule_.want;

    bool isEmbeddable = false;
    if (abilityRecord != nullptr) {
        const auto& abilityWant = abilityRecord->GetWant();
        if (abilityWant.HasParameter(AAFwk::SCREEN_MODE_KEY)) {
            int32_t screenMode = abilityWant.GetIntParam(AAFwk::SCREEN_MODE_KEY, AAFwk::IDLE_SCREEN_MODE);
            isEmbeddable = IsEmbeddableStart(screenMode);
        }
    }

    if (isEmbeddable) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Handle embeddable UIExtension");
        want.SetParam(IS_EMBEDDABLE_SERVICE, true);
        int ret = abilityRecord->CreateModalUIExtension(want);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Handle embeddable UIExtension failed");
            return ret;
        }
        return ERR_OK;
    }

    if (abilityRecord == nullptr || abilityRecord->GetAbilityInfo().type != AppExecFwk::AbilityType::PAGE) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Handle non-PAGE UIExtension (modal system path)");
        auto systemUIExtension = std::make_shared<OHOS::Rosen::ModalSystemUiExtension>();
        want.SetParam(UIEXTENSION_MODAL_TYPE, 1);
        bool ret = IN_PROCESS_CALL(systemUIExtension->CreateModalUIExtension(want));
        if (!ret) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Modal system UIExtension creation failed");
            return ret;
        }
        return ERR_OK;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "Handle PAGE type UIExtension (modal app path)");
    auto sessionInfo = abilityRecord->GetSessionInfo();
    if (sessionInfo != nullptr) {
        want.SetParam(INTERCEPT_MISSION_ID, sessionInfo->persistentId);
    } else {
        want.SetParam(INTERCEPT_MISSION_ID, abilityRecord->GetMissionId());
    }
    int ret = abilityRecord->CreateModalUIExtension(want);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Modal app UIExtension creation failed");
        return ret;
    }
    return ERR_OK;
}
} // namespace AAFwk
} // namespace OHOS
