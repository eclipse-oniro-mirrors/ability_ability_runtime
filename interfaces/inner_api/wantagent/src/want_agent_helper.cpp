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

#include "want_agent_helper.h"

#include "ability_runtime_error_util.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "want_params_wrapper.h"
#include "pending_want.h"
#include "want_agent_client.h"
#include "want_agent_log_wrapper.h"
#include "want_sender_info.h"
#include "want_sender_interface.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime;
namespace OHOS::AbilityRuntime::WantAgent {
WantAgentHelper::WantAgentHelper()
{}

unsigned int WantAgentHelper::FlagsTransformer(const std::vector<WantAgentConstant::Flags> &flags)
{
    unsigned int wantFlags = 0;
    if (flags.empty()) {
        wantFlags |= static_cast<unsigned int>(FLAG_UPDATE_CURRENT);
        return wantFlags;
    }

    for (auto flag : flags) {
        switch (flag) {
            case WantAgentConstant::Flags::ONE_TIME_FLAG:
                wantFlags |= static_cast<unsigned int>(FLAG_ONE_SHOT);
                break;
            case WantAgentConstant::Flags::NO_BUILD_FLAG:
                wantFlags |= static_cast<unsigned int>(FLAG_NO_CREATE);
                break;
            case WantAgentConstant::Flags::CANCEL_PRESENT_FLAG:
                wantFlags |= static_cast<unsigned int>(FLAG_CANCEL_CURRENT);
                break;
            case WantAgentConstant::Flags::UPDATE_PRESENT_FLAG:
                wantFlags |= static_cast<unsigned int>(FLAG_UPDATE_CURRENT);
                break;
            case WantAgentConstant::Flags::CONSTANT_FLAG:
                wantFlags |= static_cast<unsigned int>(FLAG_IMMUTABLE);
                break;
            case WantAgentConstant::Flags::REPLACE_ELEMENT:
                wantFlags |= static_cast<unsigned int>(FLAG_UPDATE_CURRENT);
                TAG_LOGE(AAFwkTag::WANTAGENT, "Invalid flag:REPLACE_ELEMENT");
                break;
            case WantAgentConstant::Flags::REPLACE_ACTION:
                wantFlags |= static_cast<unsigned int>(FLAG_UPDATE_CURRENT);
                TAG_LOGE(AAFwkTag::WANTAGENT, "Invalid flag:REPLACE_ACTION");
                break;
            case WantAgentConstant::Flags::REPLACE_URI:
                wantFlags |= static_cast<unsigned int>(FLAG_UPDATE_CURRENT);
                TAG_LOGE(AAFwkTag::WANTAGENT, "Invalid flag:REPLACE_URI");
                break;
            case WantAgentConstant::Flags::REPLACE_ENTITIES:
                wantFlags |= static_cast<unsigned int>(FLAG_UPDATE_CURRENT);
                TAG_LOGE(AAFwkTag::WANTAGENT, "Invalid flag:REPLACE_ENTITIES");
                break;
            case WantAgentConstant::Flags::REPLACE_BUNDLE:
                wantFlags |= static_cast<unsigned int>(FLAG_UPDATE_CURRENT);
                TAG_LOGE(AAFwkTag::WANTAGENT, "Invalid flag:REPLACE_BUNDLE");
                break;
            default:
                TAG_LOGE(AAFwkTag::WANTAGENT, "flags is error");
                break;
        }
    }
    return wantFlags;
}

ErrCode WantAgentHelper::GetWantAgent(
    const std::shared_ptr<OHOS::AbilityRuntime::ApplicationContext> &context,
    const WantAgentInfo &paramsInfo, std::shared_ptr<WantAgent> &wantAgent)
{
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "invalid input param");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }

    std::vector<std::shared_ptr<Want>> wants = paramsInfo.GetWants();
    if (wants.empty()) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "invalid input param");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }

    unsigned int flags = FlagsTransformer(paramsInfo.GetFlags());
    if (flags == 0) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "flags invalid");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }

    std::shared_ptr<WantParams> extraInfo = paramsInfo.GetExtraInfo();
    std::shared_ptr<PendingWant> pendingWant = nullptr;
    int requestCode = paramsInfo.GetRequestCode();
    WantAgentConstant::OperationType operationType = paramsInfo.GetOperationType();
    ErrCode result;
    switch (operationType) {
        case WantAgentConstant::OperationType::START_ABILITY:
            result = PendingWant::GetAbility(context, requestCode, wants[0], flags, extraInfo, pendingWant);
            break;
        case WantAgentConstant::OperationType::START_ABILITIES:
            result = PendingWant::GetAbilities(context, requestCode, wants, flags, extraInfo, pendingWant);
            break;
        case WantAgentConstant::OperationType::START_SERVICE:
            result = PendingWant::GetService(context, requestCode, wants[0], flags, pendingWant);
            break;
        case WantAgentConstant::OperationType::START_FOREGROUND_SERVICE:
            result = PendingWant::GetForegroundService(context, requestCode, wants[0], flags, pendingWant);
            break;
        case WantAgentConstant::OperationType::SEND_COMMON_EVENT:
            result = PendingWant::GetCommonEvent(context, requestCode, wants[0], flags, pendingWant);
            break;
        case WantAgentConstant::OperationType::START_SERVICE_EXTENSION:
            result = PendingWant::GetServiceExtension(context, requestCode, wants[0], flags, pendingWant);
            break;
        default:
            TAG_LOGE(AAFwkTag::WANTAGENT, "operation type is error");
            result = ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
            break;
    }

    if (pendingWant == nullptr) {
        return result;
    }
    wantAgent = std::make_shared<WantAgent>(pendingWant);
    return ERR_OK;
}

std::shared_ptr<WantAgent> WantAgentHelper::GetWantAgent(const WantAgentInfo &paramsInfo, int32_t userId)
{
    std::vector<std::shared_ptr<Want>> wants = paramsInfo.GetWants();
    if (wants.empty()) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "invalid input param");
        return nullptr;
    }

    std::shared_ptr<Want> want = wants[0];
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "invalid input param");
        return nullptr;
    }

    WantsInfo wantsInfo;
    wantsInfo.want = *want;
    wantsInfo.resolvedTypes = want->GetType();
    if (paramsInfo.GetExtraInfo() != nullptr && !paramsInfo.GetExtraInfo()->IsEmpty()) {
        wantsInfo.want.SetParams(*paramsInfo.GetExtraInfo());
    }

    WantSenderInfo wantSenderInfo;
    wantSenderInfo.allWants.push_back(wantsInfo);
    wantSenderInfo.bundleName = want->GetOperation().GetBundleName();
    wantSenderInfo.flags = FlagsTransformer(paramsInfo.GetFlags());
    wantSenderInfo.type = static_cast<int32_t>(paramsInfo.GetOperationType());
    wantSenderInfo.userId = userId;
    sptr<IWantSender> target = nullptr;
    WantAgentClient::GetInstance().GetWantSender(wantSenderInfo, nullptr, target);
    if (target == nullptr) {
        return nullptr;
    }
    std::shared_ptr<WantAgent> agent = std::make_shared<WantAgent>(std::make_shared<PendingWant>(target));

    return agent;
}

WantAgentConstant::OperationType WantAgentHelper::GetType(std::shared_ptr<WantAgent> agent)
{
    if ((agent == nullptr) || (agent->GetPendingWant() == nullptr)) {
        return WantAgentConstant::OperationType::UNKNOWN_TYPE;
    }

    return agent->GetPendingWant()->GetType(agent->GetPendingWant()->GetTarget());
}

ErrCode WantAgentHelper::TriggerWantAgent(std::shared_ptr<WantAgent> agent,
    const std::shared_ptr<CompletedCallback> &callback, const TriggerInfo &paramsInfo)
{
    TAG_LOGI(AAFwkTag::WANTAGENT, "call");
    if (agent == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "invalid input param");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }
    std::shared_ptr<PendingWant> pendingWant = agent->GetPendingWant();
    WantAgentConstant::OperationType type = GetType(agent);
    sptr<CompletedDispatcher> dispatcher = nullptr;
    if (callback != nullptr) {
        dispatcher = new (std::nothrow) CompletedDispatcher(pendingWant, callback, nullptr);
    }
    return Send(pendingWant, type, dispatcher, paramsInfo);
}

ErrCode WantAgentHelper::Send(const std::shared_ptr<PendingWant> &pendingWant,
    WantAgentConstant::OperationType type, const sptr<CompletedDispatcher> &callBack, const TriggerInfo &paramsInfo)
{
    TAG_LOGI(AAFwkTag::WANTAGENT, "call");
    if (pendingWant == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "invalid input param");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }

    return pendingWant->Send(paramsInfo.GetResultCode(),
        paramsInfo.GetWant(),
        callBack,
        paramsInfo.GetPermission(),
        paramsInfo.GetExtraInfo(),
        paramsInfo.GetStartOptions(),
        pendingWant->GetTarget());
}

ErrCode WantAgentHelper::Cancel(const std::shared_ptr<WantAgent> &agent)
{
    if (agent == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "invalid input param");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }

    std::shared_ptr<PendingWant> pendingWant = agent->GetPendingWant();
    if (pendingWant == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "invalid input param");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }

    return pendingWant->Cancel(pendingWant->GetTarget());
}

ErrCode WantAgentHelper::IsEquals(
    const std::shared_ptr<WantAgent> &agent, const std::shared_ptr<WantAgent> &otherAgent)
{
    if ((agent == nullptr) && (otherAgent == nullptr)) {
        return ERR_OK;
    }

    if ((agent == nullptr) || (otherAgent == nullptr)) {
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }
    return PendingWant::IsEquals(agent->GetPendingWant(), otherAgent->GetPendingWant());
}

ErrCode WantAgentHelper::GetBundleName(const std::shared_ptr<WantAgent> &agent, std::string &bundleName)
{
    TAG_LOGD(AAFwkTag::WANTAGENT, "called");
    if (agent == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "invalid input param");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }

    std::shared_ptr<PendingWant> pendingWant = agent->GetPendingWant();
    if (pendingWant == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "invalid input param");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }

    return pendingWant->GetBundleName(pendingWant->GetTarget(), bundleName);
}

ErrCode WantAgentHelper::GetUid(const std::shared_ptr<WantAgent> &agent, int32_t &uid)
{
    if (agent == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "invalid input param");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }

    std::shared_ptr<PendingWant> pendingWant = agent->GetPendingWant();
    if (pendingWant == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "invalid input param");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }

    return pendingWant->GetUid(pendingWant->GetTarget(), uid);
}

std::shared_ptr<Want> WantAgentHelper::GetWant(const std::shared_ptr<WantAgent> &agent)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (agent == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "invalid input param");
        return nullptr;
    }

    std::shared_ptr<PendingWant> pendingWant = agent->GetPendingWant();
    if (pendingWant == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "invalid input param");
        return nullptr;
    }

    return pendingWant->GetWant(pendingWant->GetTarget());
}

void WantAgentHelper::RegisterCancelListener(
    const std::shared_ptr<CancelListener> &cancelListener, const std::shared_ptr<WantAgent> &agent)
{
    if (agent == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "invalid input param");
        return;
    }

    std::shared_ptr<PendingWant> pendingWant = agent->GetPendingWant();
    if (pendingWant == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "invalid input param");
        return;
    }

    pendingWant->RegisterCancelListener(cancelListener, pendingWant->GetTarget());
}

void WantAgentHelper::UnregisterCancelListener(
    const std::shared_ptr<CancelListener> &cancelListener, const std::shared_ptr<WantAgent> &agent)
{
    if (agent == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "invalid input param");
        return;
    }

    std::shared_ptr<PendingWant> pendingWant = agent->GetPendingWant();
    if (pendingWant == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "invalid input param");
        return;
    }

    pendingWant->UnregisterCancelListener(cancelListener, pendingWant->GetTarget());
}

std::string WantAgentHelper::ToString(const std::shared_ptr<WantAgent> &agent)
{
    if (agent == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "invalid input param");
        return "";
    }

    std::shared_ptr<PendingWant> pendingWant = agent->GetPendingWant();
    if (pendingWant == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "invalid input param");
        return "";
    }

    std::shared_ptr<WantSenderInfo> info = pendingWant->GetWantSenderInfo(pendingWant->GetTarget());
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "invalid input param");
        return "";
    }
    nlohmann::json jsonObject;
    jsonObject["requestCode"] = (*info.get()).requestCode;
    jsonObject["operationType"] = (*info.get()).type;
    jsonObject["flags"] = (*info.get()).flags;

    nlohmann::json wants = nlohmann::json::array();
    for (auto &wantInfo : (*info.get()).allWants) {
        wants.emplace_back(wantInfo.want.ToString());
    }
    jsonObject["wants"] = wants;

    if ((*info.get()).allWants.size() > 0) {
        nlohmann::json paramsObj;
        AAFwk::WantParamWrapper wWrapper((*info.get()).allWants[0].want.GetParams());
        paramsObj["extraInfoValue"] = wWrapper.ToString();
        jsonObject["extraInfo"] = paramsObj;
    }

    return jsonObject.dump();
}

std::shared_ptr<WantAgent> WantAgentHelper::FromString(const std::string &jsonString)
{
    if (jsonString.empty()) {
        return nullptr;
    }
    nlohmann::json jsonObject = nlohmann::json::parse(jsonString);
    if (jsonObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Failed to parse json string");
        return nullptr;
    }
    int requestCode = -1;
    if (jsonObject.contains("requestCode") && jsonObject["requestCode"].is_number_integer()) {
        requestCode = jsonObject.at("requestCode").get<int>();
    }

    WantAgentConstant::OperationType operationType = WantAgentConstant::OperationType::UNKNOWN_TYPE;
    if (jsonObject.contains("operationType") && jsonObject["operationType"].is_number_integer()) {
        operationType = static_cast<WantAgentConstant::OperationType>(jsonObject.at("operationType").get<int>());
    }

    std::vector<WantAgentConstant::Flags> flagsVec = ParseFlags(jsonObject);

    std::vector<std::shared_ptr<AAFwk::Want>> wants = {};
    if (jsonObject.contains("wants") && jsonObject["wants"].is_array()) {
        for (auto &wantObj : jsonObject.at("wants")) {
            if (wantObj.is_string()) {
                auto wantString = wantObj.get<std::string>();
                wants.emplace_back(std::make_shared<AAFwk::Want>(*Want::FromString(wantString)));
            }
        }
    }

    std::shared_ptr<AAFwk::WantParams> extraInfo = nullptr;
    if (jsonObject.contains("extraInfo") && jsonObject["extraInfo"].is_object()) {
        auto extraInfoObj = jsonObject.at("extraInfo");
        if (extraInfoObj.contains("extraInfoValue") && extraInfoObj["extraInfoValue"].is_string()) {
            auto pwWrapper = AAFwk::WantParamWrapper::Parse(extraInfoObj.at("extraInfoValue").get<std::string>());
            AAFwk::WantParams params;
            if (pwWrapper->GetValue(params) == ERR_OK) {
                extraInfo = std::make_shared<AAFwk::WantParams>(params);
            }
        }
    }
    WantAgentInfo info(requestCode, operationType, flagsVec, wants, extraInfo);

    return GetWantAgent(info);
}

std::vector<WantAgentConstant::Flags> WantAgentHelper::ParseFlags(nlohmann::json jsonObject)
{
    int flags = -1;
    std::vector<WantAgentConstant::Flags> flagsVec = {};
    if (jsonObject.contains("flags") && jsonObject.at("flags").is_number_integer()) {
        flags = jsonObject.at("flags").get<int>();
    }

    if (flags < 0) {
        return flagsVec;
    }

    if (static_cast<uint32_t>(flags) & static_cast<uint32_t>(FLAG_ONE_SHOT)) {
        flagsVec.emplace_back(WantAgentConstant::Flags::ONE_TIME_FLAG);
    }
    if (static_cast<uint32_t>(flags) & static_cast<uint32_t>(FLAG_NO_CREATE)) {
        flagsVec.emplace_back(WantAgentConstant::Flags::NO_BUILD_FLAG);
    }
    if (static_cast<uint32_t>(flags) & static_cast<uint32_t>(FLAG_CANCEL_CURRENT)) {
        flagsVec.emplace_back(WantAgentConstant::Flags::CANCEL_PRESENT_FLAG);
    }
    if (static_cast<uint32_t>(flags) & static_cast<uint32_t>(FLAG_UPDATE_CURRENT)) {
        flagsVec.emplace_back(WantAgentConstant::Flags::UPDATE_PRESENT_FLAG);
    }
    if (static_cast<uint32_t>(flags) & static_cast<uint32_t>(FLAG_IMMUTABLE)) {
        flagsVec.emplace_back(WantAgentConstant::Flags::CONSTANT_FLAG);
    }

    return flagsVec;
}

ErrCode WantAgentHelper::GetType(const std::shared_ptr<WantAgent> &agent, int32_t &operType)
{
    if ((agent == nullptr) || (agent->GetPendingWant() == nullptr)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "invalid input param");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_WANTAGENT;
    }

    return agent->GetPendingWant()->GetType(agent->GetPendingWant()->GetTarget(), operType);
}

ErrCode WantAgentHelper::GetWant(const std::shared_ptr<WantAgent> &agent, std::shared_ptr<AAFwk::Want> &want)
{
    if ((agent == nullptr) || (agent->GetPendingWant() == nullptr)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "WantAgent or PendingWant invalid input param.");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_WANTAGENT;
    }

    return agent->GetPendingWant()->GetWant(agent->GetPendingWant()->GetTarget(), want);
}
}  // namespace OHOS::AbilityRuntime::WantAgent
