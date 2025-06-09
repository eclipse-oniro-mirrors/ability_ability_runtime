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

#include "ability_debug_deal.h"

#include "ability_record.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "in_process_call_wrapper.h"

namespace OHOS {
namespace AAFwk {
void AbilityDebugDeal::RegisterAbilityDebugResponse()
{
    abilityDebugResponse_ = new (std::nothrow) AbilityDebugResponse(weak_from_this());
    if (abilityDebugResponse_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityDebugResponse null");
        return;
    }

    IN_PROCESS_CALL_WITHOUT_RET(
        DelayedSingleton<AppScheduler>::GetInstance()->RegisterAbilityDebugResponse(abilityDebugResponse_));
}

void AbilityDebugDeal::OnAbilitysDebugStarted(const std::vector<sptr<IRemoteObject>> &tokens)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    for (auto &token : tokens) {
        auto abilityRecord = Token::GetAbilityRecordByToken(token);
        if (abilityRecord == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityRecord null");
            continue;
        }
        abilityRecord->SetAttachDebug(true);
    }
}

void AbilityDebugDeal::OnAbilitysDebugStoped(const std::vector<sptr<IRemoteObject>> &tokens)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    for (auto &token : tokens) {
        auto abilityRecord = Token::GetAbilityRecordByToken(token);
        if (abilityRecord == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityRecord null");
            continue;
        }
        abilityRecord->SetAttachDebug(false);
    }
}

void AbilityDebugDeal::OnAbilitysAssertDebugChange(const std::vector<sptr<IRemoteObject>> &tokens, bool isAssertDebug)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Called. %{public}s", isAssertDebug ? "start assert debug" : "stop assert debug");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    for (const auto &token : tokens) {
        auto abilityRecord = Token::GetAbilityRecordByToken(token);
        if (abilityRecord == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityRecord null");
            continue;
        }
        abilityRecord->SetAssertDebug(isAssertDebug);
    }
}

ErrCode AbilityDebugResponse::OnAbilitysDebugStarted(const std::vector<sptr<IRemoteObject>> &tokens)
{
    if (tokens.empty()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "tokens empty");
        return ERR_INVALID_DATA;
    }

    auto deal = abilityDebugDeal_.lock();
    if (deal == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "deal null");
        return ERR_INVALID_DATA;
    }
    deal->OnAbilitysDebugStarted(tokens);
    return ERR_OK;
}

ErrCode AbilityDebugResponse::OnAbilitysDebugStoped(const std::vector<sptr<IRemoteObject>> &tokens)
{
    if (tokens.empty()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "tokens empty");
        return ERR_INVALID_DATA;
    }

    auto deal = abilityDebugDeal_.lock();
    if (deal == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "deal null");
        return ERR_INVALID_DATA;
    }
    deal->OnAbilitysDebugStoped(tokens);
    return ERR_OK;
}

ErrCode AbilityDebugResponse::OnAbilitysAssertDebugChange(
    const std::vector<sptr<IRemoteObject>> &tokens, bool isAssertDebug)
{
    if (tokens.empty()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "tokens empty");
        return ERR_INVALID_DATA;
    }

    auto deal = abilityDebugDeal_.lock();
    if (deal == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "deal null");
        return ERR_INVALID_DATA;
    }
    deal->OnAbilitysAssertDebugChange(tokens, isAssertDebug);
    return ERR_OK;
}
} // namespace AAFwk
} // namespace OHOS
