/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "appmgr_test_service.h"

#include <string>
#include <unistd.h>

#include "ability_scheduler.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
AppMgrEventHandler::AppMgrEventHandler(
    const std::shared_ptr<AppExecFwk::EventRunner>& runner, const std::shared_ptr<AbilityManagerService>& server)
    : AppExecFwk::EventHandler(runner), server_(server)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityEventHandler::AbilityEventHandler::instance created.");
}

void AppMgrEventHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    if (event == nullptr) {
        TAG_LOGE(AAFwkTag::TEST, "AMSEventHandler::ProcessEvent::parameter error");
        return;
    }
    TAG_LOGD(AAFwkTag::TEST, "AMSEventHandler::ProcessEvent::inner event id obtained: %u.",
             event->GetInnerEventId());
    switch (event->GetInnerEventId()) {
        case AppManagerTestService::LOAD_ABILITY_MSG: {
            TAG_LOGD(AAFwkTag::TEST, "Load Ability msg.");
            ProcessLoadAbility(event);
            break;
        }
        case AppManagerTestService::SCHEDULE_ABILITY_MSG: {
            TAG_LOGD(AAFwkTag::TEST, "scheduler ability msg.");
            ScheduleAbilityTransaction(event);
            break;
        }
        case AppManagerTestService::SCHEDULE_CONNECT_MSG: {
            TAG_LOGD(AAFwkTag::TEST, "scheduler connect msg.");
            ScheduleConnectAbilityTransaction(event);
            break;
        }
        case AppManagerTestService::SCHEDULE_DISCONNECT_MSG: {
            TAG_LOGD(AAFwkTag::TEST, "scheduler disconnect msg.");
            ScheduleDisconnectAbilityTransaction(event);
            break;
        }
        default: {
            TAG_LOGD(AAFwkTag::TEST, "unknown message.");
            break;
        }
    }
}

void AppMgrEventHandler::ProcessLoadAbility(const AppExecFwk::InnerEvent::Pointer& event)
{
    TAG_LOGD(AAFwkTag::TEST, "process load ability.");
    auto tokenPtr = event->GetUniqueObject<sptr<IRemoteObject>>().get();
    if (tokenPtr == nullptr) {
        TAG_LOGE(AAFwkTag::TEST, "abilityToken unavailable.");
    } else {
        OHOS::sptr<IRemoteObject> token = *tokenPtr;
        std::shared_ptr<AbilityRecord> ability = Token::GetAbilityRecordByToken(token);
        if (ability == nullptr) {
            TAG_LOGE(AAFwkTag::TEST, "ability unavailable.");
            return;
        }
        TAG_LOGD(AAFwkTag::TEST, "attach ability %s thread.", ability->GetAbilityInfo().name.c_str());
        sptr<IAbilityScheduler> abilitySched = new AbilityScheduler();
        tokenMap_[abilitySched] = token;
        server_->AttachAbilityThread(abilitySched, token);
    }
}

void AppMgrEventHandler::ScheduleAbilityTransaction(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto object = event->GetUniqueObject<std::pair<sptr<AbilityScheduler>, int>>();
    auto abilitySched = object.get()->first;
    auto targetState = object.get()->second;
    auto abilityToken = tokenMap_[abilitySched];

    if (abilityToken == nullptr) {
        TAG_LOGE(AAFwkTag::TEST, "abilityToken unavailable.");
        return;
    }

    std::shared_ptr<AbilityRecord> ability = Token::GetAbilityRecordByToken(abilityToken);
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::TEST, "ability unavailable.");
        return;
    }
    AppExecFwk::AbilityInfo abilityinfo = ability->GetAbilityInfo();

    if (targetState == ACTIVE) {
        TAG_LOGD(AAFwkTag::TEST, "process schedule ability %s active.", abilityinfo.name.c_str());
        if (std::string::npos != abilityinfo.name.find("BlockActive")) {
            usleep(BLOCK_TEST_TIME);
        }
    } else if (targetState == INACTIVE) {
        TAG_LOGD(AAFwkTag::TEST, "process schedule ability %s inactive.", abilityinfo.name.c_str());
        if (std::string::npos != abilityinfo.name.find("BlockInActive")) {
            usleep(BLOCK_TEST_TIME);
        }
    }
    PacMap saveData;
    server_->AbilityTransitionDone(ability->GetToken(), targetState, saveData);
    return;
}

void AppMgrEventHandler::ScheduleConnectAbilityTransaction(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto object = event->GetUniqueObject<std::pair<sptr<AbilityScheduler>, Want>>();
    auto abilitySched = object.get()->first;
    auto abilityToken = tokenMap_[abilitySched];
    server_->ScheduleConnectAbilityDone(abilityToken, abilityToken);
}

void AppMgrEventHandler::ScheduleDisconnectAbilityTransaction(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto object = event->GetUniqueObject<sptr<AbilityScheduler>>();
    auto abilitySchedPtr = object.get();
    sptr<IAbilityScheduler> abilitySched = *abilitySchedPtr;
    auto abilityToken = tokenMap_[abilitySched];

    if (abilityToken == nullptr) {
        TAG_LOGE(AAFwkTag::TEST, "abilityToken unavailable.");
        return;
    }

    std::shared_ptr<AbilityRecord> ability = Token::GetAbilityRecordByToken(abilityToken);
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::TEST, "ability unavailable.");
        return;
    }
    AppExecFwk::AbilityInfo abilityinfo = ability->GetAbilityInfo();
    if (std::string::npos == abilityinfo.name.find("Block")) {
        TAG_LOGI(AAFwkTag::TEST, "no block, so call disconnect done");
        server_->ScheduleDisconnectAbilityDone(abilityToken);
    }
    return;
}

AppManagerTestService::AppManagerTestService() : eventLoop_(nullptr), handler_(nullptr)
{}

AppManagerTestService::~AppManagerTestService()
{}

void AppManagerTestService::Start()
{
    eventLoop_ = AppExecFwk::EventRunner::Create("AppManagerTestService");
    if (eventLoop_.get() == nullptr) {
        TAG_LOGE(AAFwkTag::TEST, "failed to create EventRunner");
        return;
    }
    handler_ = std::make_shared<AppMgrEventHandler>(eventLoop_, DelayedSingleton<AbilityManagerService>::GetInstance());
    eventLoop_->Run();
    return;
}
}  // namespace AAFwk
}  // namespace OHOS
