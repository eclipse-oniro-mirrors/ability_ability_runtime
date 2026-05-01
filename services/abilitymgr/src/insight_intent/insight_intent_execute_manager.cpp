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

#include "insight_intent_execute_manager.h"

#include <algorithm>
#include <unordered_map>

#include "ability_config.h"
#include "ability_util.h"
#include "ability_manager_errors.h"
#include "distributed_ability_runtime/distributed_client.h"
#include "extract_insight_intent_profile.h"
#include "hilog_tag_wrapper.h"
#include "insight_intent_execute_callback_interface.h"
#include "insight_intent_db_cache.h"
#include "insight_intent_utils.h"
#include "permission_verification.h"
#include "want_params_wrapper.h"
#include "time_util.h"
#include "res_sched_util.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace AAFwk {
using AppExecFwk::InsightIntentParam;
using AppExecFwk::ParamType;

namespace {
constexpr size_t INSIGHT_INTENT_EXECUTE_RECORDS_MAX_SIZE = 256;
constexpr char EXECUTE_INSIGHT_INTENT_PERMISSION[] = "ohos.permission.EXECUTE_INSIGHT_INTENT";
constexpr char PERMISSION_GET_BUNDLE_INFO_PRIVILEGED[] = "ohos.permission.GET_BUNDLE_INFO_PRIVILEGED";
constexpr int32_t OPERATION_DURATION = 10000;

ParamType StringToParamType(const std::string &typeStr)
{
    static const std::unordered_map<std::string, ParamType> typeMapping = {
        {"string", ParamType::STRING},
        {"number", ParamType::NUMBER},
        {"integer", ParamType::INTEGER},
        {"boolean", ParamType::BOOLEAN},
        {"object", ParamType::OBJECT},
        {"array", ParamType::ARRAY}
    };
    auto it = typeMapping.find(typeStr);
    return (it != typeMapping.end()) ? it->second : ParamType::UNKNOWN;
}

void GetMethodParamRequiredList(const nlohmann::json &jsonObj, std::vector<std::string> &requiredList)
{
    if (!jsonObj.contains("required") || !jsonObj["required"].is_array()) {
        return;
    }
    for (const auto &item : jsonObj["required"]) {
        if (item.is_string()) {
            requiredList.emplace_back(item.get<std::string>());
        }
    }
}

void GetMethodParamTypeMap(const nlohmann::json &jsonObj, std::unordered_map<std::string, ParamType> &typeMap)
{
    if (!jsonObj.contains("properties") || !jsonObj["properties"].is_object()) {
        return;
    }
    const auto &properties = jsonObj["properties"];
    for (auto it = properties.begin(); it != properties.end(); ++it) {
        if (!it.value().is_object() || !it.value().contains("type") || !it.value()["type"].is_string()) {
            typeMap[it.key()] = ParamType::UNKNOWN;
            continue;
        }
        typeMap[it.key()] = StringToParamType(it.value()["type"].get<std::string>());
    }
}

bool GetMethodParamNamesFromSchema(const std::vector<std::string> &methodParams, const std::string &parameters,
    std::vector<std::string> &encodedMethodParams)
{
    encodedMethodParams.clear();
    encodedMethodParams.reserve(methodParams.size());
    std::unordered_map<std::string, ParamType> typeMap;
    std::vector<std::string> requiredList;
    if (!parameters.empty()) {
        auto jsonObj = nlohmann::json::parse(parameters, nullptr, false);
        if (jsonObj.is_discarded() || !jsonObj.is_object()) {
            TAG_LOGW(AAFwkTag::INTENT, "parameters parse failed or not object");
        } else {
            GetMethodParamTypeMap(jsonObj, typeMap);
            GetMethodParamRequiredList(jsonObj, requiredList);
        }
    }
    for (const auto &methodParamName : methodParams) {
        InsightIntentParam methodParamInfo;
        methodParamInfo.paramName = methodParamName;
        auto typeIt = typeMap.find(methodParamName);
        if (typeIt != typeMap.end()) {
            methodParamInfo.type = typeIt->second;
        }
        methodParamInfo.isRequired = std::find(requiredList.begin(), requiredList.end(), methodParamName) !=
            requiredList.end();
        encodedMethodParams.emplace_back(EncodeMethodParam(methodParamInfo));
    }
    return true;
}
}
using namespace AppExecFwk;
using InsightIntentType = AbilityRuntime::InsightIntentType;
using ExtractInsightIntentInfo = AbilityRuntime::ExtractInsightIntentInfo;
using InsightIntentPageInfo = AbilityRuntime::InsightIntentPageInfo;
using InsightIntentFunctionInfo = AbilityRuntime::InsightIntentFunctionInfo;
using InsightIntentEntryInfo = AbilityRuntime::InsightIntentEntryInfo;
void InsightIntentExecuteRecipient::OnRemoteDied(const wptr<OHOS::IRemoteObject> &remote)
{
    TAG_LOGD(AAFwkTag::INTENT, "InsightIntentExecuteRecipient OnRemoteDied, %{public}" PRIu64, intentId_);
    auto object = remote.promote();
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null object");
        return;
    }
    DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->RemoteDied(intentId_);
}

InsightIntentExecuteManager::InsightIntentExecuteManager() = default;

InsightIntentExecuteManager::~InsightIntentExecuteManager() = default;

int32_t InsightIntentExecuteManager::CheckAndUpdateParam(uint64_t key, const sptr<IRemoteObject> &callerToken,
    const std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> &param, std::string callerBundleName,
    const bool ignoreAbilityName, bool isDistributed, const std::string &srcDeviceId, uint64_t requestCode,
    uint64_t specifiedFullTokenId)
{
    int32_t result = CheckCallerPermission(specifiedFullTokenId);
    if (result != ERR_OK && (param == nullptr || !param->isServiceMatch_)) {
        return result;
    }
    if ((!isDistributed && callerToken == nullptr) && (param == nullptr || !param->isServiceMatch_)) {
        TAG_LOGE(AAFwkTag::INTENT, "null callerToken");
        return ERR_INVALID_VALUE;
    }
    if (param == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null param");
        return ERR_INVALID_VALUE;
    }

    if (param->bundleName_.empty() || param->moduleName_.empty() ||
        (!ignoreAbilityName && param->abilityName_.empty()) || param->insightIntentName_.empty()) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid param");
        return ERR_INVALID_VALUE;
    }
    uint64_t intentId = 0;
    result = AddRecord(key, callerToken, param->bundleName_,
        intentId, callerBundleName, isDistributed, srcDeviceId, requestCode);
    if (result != ERR_OK) {
        return result;
    }

    param->insightIntentId_ = intentId;
    return ERR_OK;
}

int32_t InsightIntentExecuteManager::CheckAndUpdateWant(Want &want, ExecuteMode executeMode, int32_t userId,
    std::string callerBundleName)
{
    auto uriVec = want.GetStringArrayParam(AbilityConfig::PARAMS_STREAM);
    auto uriVecTemp = want.GetStringArrayParam(INSIGHT_INTENT_EXECUTE_PARAM_URI);
    uriVec.insert(uriVec.begin(), uriVecTemp.begin(), uriVecTemp.end());
    want.SetParam(AbilityConfig::PARAMS_STREAM, uriVec);
    auto myflags = want.GetIntParam(INSIGHT_INTENT_EXECUTE_PARAM_FLAGS, 0);
    myflags |= want.GetFlags();
    want.SetFlags(myflags);

    int32_t result = IsValidCall(want);
    if (result != ERR_OK) {
        return result;
    }
    uint64_t intentId = 0;
    ElementName elementName = want.GetElement();
    result = AddRecord(0, nullptr, want.GetBundle(), intentId, callerBundleName);
    if (result != ERR_OK) {
        return result;
    }

    std::string srcEntry;
    std::string arkTSMode;
    std::string intentName = want.GetStringParam(INSIGHT_INTENT_EXECUTE_PARAM_NAME);
    std::string decoratorClass;
    auto ret = AbilityRuntime::InsightIntentUtils::GetSrcEntry(
        elementName, intentName, executeMode, srcEntry, &arkTSMode, DEFAULT_INVAL_VALUE, &decoratorClass);
    if (ret != ERR_OK || srcEntry.empty()) {
        TAG_LOGW(AAFwkTag::INTENT, "empty srcEntry");
        if (UpdateEntryDecoratorParams(want, executeMode, userId) != ERR_OK) {
            return ERR_INVALID_VALUE;
        }
    }

    want.SetParam(INSIGHT_INTENT_SRC_ENTRY, srcEntry);
    want.SetParam(INSIGHT_INTENT_ARKTS_MODE, arkTSMode);
    if (!decoratorClass.empty()) {
        want.SetParam("ohos.insightIntent.decoratorClass", decoratorClass);
    }
    want.SetParam(INSIGHT_INTENT_EXECUTE_PARAM_ID, std::to_string(intentId));
    want.SetParam(INSIGHT_INTENT_EXECUTE_PARAM_MODE, executeMode);
    TAG_LOGD(AAFwkTag::INTENT, "check done. insightIntentId: %{public}" PRIu64, intentId);
    return ERR_OK;
}

int32_t InsightIntentExecuteManager::UpdateEntryDecoratorParams(Want &want, ExecuteMode executeMode, int32_t userId)
{
    std::string intentName = want.GetStringParam(INSIGHT_INTENT_EXECUTE_PARAM_NAME);
    ExtractInsightIntentInfo info;
    if (userId == DEFAULT_INVAL_VALUE) {
        userId = IPCSkeleton::GetCallingUid() / AppExecFwk::Constants::BASE_USER_RANGE;
    }
    DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance()->GetInsightIntentInfo(
        want.GetBundle(), want.GetModuleName(), intentName, userId, info);
    if (info.genericInfo.decoratorType != AbilityRuntime::INSIGHT_INTENTS_DECORATOR_TYPE_ENTRY) {
        TAG_LOGW(AAFwkTag::INTENT, "decorator %{public}s misMatch", info.genericInfo.decoratorType.c_str());
        return ERR_INVALID_VALUE;
    }
    InsightIntentType type = InsightIntentType::DECOR_ENTRY;
    want.SetParam(INSIGHT_INTENT_DECORATOR_TYPE, static_cast<int>(type));
    std::string srcEntrance = info.decoratorFile;
    want.SetParam(INSIGHT_INTENT_SRC_ENTRANCE, srcEntrance);
    std::vector<ExecuteMode> supportModes = info.genericInfo.get<InsightIntentEntryInfo>().executeMode;
    TAG_LOGD(AAFwkTag::INTENT, "support mode size %{public}zu", supportModes.size());
    bool found = std::find(supportModes.begin(), supportModes.end(), executeMode) != supportModes.end();
    if (!found) {
        TAG_LOGE(AAFwkTag::INTENT, "execute mode mismatch");
        return ERR_INVALID_VALUE;
    }
    if (want.GetElement().GetAbilityName() != info.genericInfo.get<InsightIntentEntryInfo>().abilityName) {
        TAG_LOGE(AAFwkTag::INTENT, "ability name mismatch");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

int32_t InsightIntentExecuteManager::AddRecord(uint64_t key, const sptr<IRemoteObject> &callerToken,
    const std::string &bundleName, uint64_t &intentId, const std::string &callerBundleName,
    bool isDistributed, const std::string &deviceId, uint64_t requestCode)
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    intentId = ++intentIdCount_;
    auto record = std::make_shared<InsightIntentExecuteRecord>();
    record->key = key;
    record->state = InsightIntentExecuteState::EXECUTING;
    record->callerToken = callerToken;
    record->bundleName = bundleName;
    record->callerBundleName = callerBundleName;
    record->isDistributed = isDistributed;
    record->deviceId = deviceId;
    record->requestCode = requestCode;
    if (callerToken != nullptr) {
        record->deathRecipient = sptr<InsightIntentExecuteRecipient>::MakeSptr(intentId);
        callerToken->AddDeathRecipient(record->deathRecipient);
    }

    // replace
    records_[intentId] = record;
    if (intentId > INSIGHT_INTENT_EXECUTE_RECORDS_MAX_SIZE) {
        // save the latest INSIGHT_INTENT_EXECUTE_RECORDS_MAX_SIZE records
        records_.erase(intentId - INSIGHT_INTENT_EXECUTE_RECORDS_MAX_SIZE);
    }
    return ERR_OK;
}

int32_t InsightIntentExecuteManager::RemoveExecuteIntent(uint64_t intentId)
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    records_.erase(intentId);
    TAG_LOGI(AAFwkTag::INTENT, "remove intentId: %{public}" PRIu64 ", records_ size: %{public}zu",
        intentId, records_.size());
    return ERR_OK;
}

int32_t InsightIntentExecuteManager::ExecuteIntentDone(uint64_t intentId, int32_t resultCode,
    const AppExecFwk::InsightIntentExecuteResult &result, int32_t callerUid, uint32_t accessToken)
{
    TAG_LOGI(AAFwkTag::INTENT, "execute start, intentId: %{public}" PRIu64 ", resultCode: %{public}d",
        intentId, resultCode);
    std::lock_guard<ffrt::mutex> lock(mutex_);
    EventInfo eventInfo;
    auto findResult = records_.find(intentId);
    if (findResult == records_.end()) {
        TAG_LOGE(AAFwkTag::INTENT, "intent not found, id: %{public}" PRIu64, intentId);
        eventInfo.errReason = "intent not found";
        SendIntentReport(eventInfo, INTENT_NOT_EXIST);
        return ERR_INVALID_VALUE;
    }

    std::shared_ptr<InsightIntentExecuteRecord> record = findResult->second;
    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null record, id: %{public}" PRIu64, intentId);
        return ERR_INVALID_VALUE;
    }

    TAG_LOGD(AAFwkTag::INTENT, "callback start, id:%{public}" PRIu64, intentId);
    if (record->state != InsightIntentExecuteState::EXECUTING) {
        TAG_LOGW(AAFwkTag::INTENT, "insight intent execute state is not EXECUTING, id:%{public}" PRIu64, intentId);
        eventInfo.errReason = "intent state error";
        SendIntentReport(eventInfo, INTENT_STATE_NOT_EXECUTING);
        return ERR_INVALID_OPERATION;
    }
    record->state = InsightIntentExecuteState::EXECUTE_DONE;
    
    if (record->isDistributed) {
        std::string msg = result.ToJsonString();
        Want want;
        want.SetElementName(record->deviceId, record->bundleName, "", "");
        IntentCallerInfo callerInfo;
        callerInfo.callerUid = callerUid;
        callerInfo.requestCode = record->requestCode;
        callerInfo.accessToken = accessToken;
        DistributedClient dmsClient;
        int32_t ret = dmsClient.SendIntentResult(want, callerInfo, msg);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "SendIntentResult failed: %{public}d", ret);
        }
    } else {
        sptr<IInsightIntentExecuteCallback> remoteCallback =
            iface_cast<IInsightIntentExecuteCallback>(record->callerToken);
        if (remoteCallback == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "intentExecuteCallback empty,"
                " intentId: %{public}" PRIu64 ", records_ size: %{public}zu", intentId, records_.size());
            return ERR_INVALID_VALUE;
        }
        remoteCallback->OnExecuteDone(record->key, resultCode, result);
    }
    
    if (record->callerToken != nullptr) {
        record->callerToken->RemoveDeathRecipient(record->deathRecipient);
        record->callerToken = nullptr;
    }
    TAG_LOGI(AAFwkTag::INTENT, "execute done, intentId: %{public}" PRIu64 ", records_ size: %{public}zu",
        intentId, records_.size());
    return ERR_OK;
}

int32_t InsightIntentExecuteManager::RemoteDied(uint64_t intentId)
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    auto result = records_.find(intentId);
    if (result == records_.end()) {
        TAG_LOGE(AAFwkTag::INTENT, "intent not found, id: %{public}" PRIu64, intentId);
        return ERR_INVALID_VALUE;
    }
    if (result->second == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null intent record , id: %{public}" PRIu64, intentId);
        return ERR_INVALID_VALUE;
    }
    result->second->callerToken = nullptr;
    result->second->state = InsightIntentExecuteState::REMOTE_DIED;
    return ERR_OK;
}

int32_t InsightIntentExecuteManager::GetBundleName(uint64_t intentId, std::string &bundleName) const
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    auto result = records_.find(intentId);
    if (result == records_.end()) {
        TAG_LOGE(AAFwkTag::INTENT, "intent not found, id: %{public}" PRIu64, intentId);
        return ERR_INVALID_VALUE;
    }
    if (result->second == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null intent record,id: %{public}" PRIu64, intentId);
        return ERR_INVALID_VALUE;
    }
    bundleName = result->second->bundleName;
    return ERR_OK;
}

int32_t InsightIntentExecuteManager::GetCallerBundleName(uint64_t intentId, std::string &callerBundleName) const
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    auto result = records_.find(intentId);
    if (result == records_.end()) {
        TAG_LOGE(AAFwkTag::INTENT, "intent not found, id: %{public}" PRIu64, intentId);
        return ERR_INVALID_VALUE;
    }
    if (result->second == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null intent record,id: %{public}" PRIu64, intentId);
        return ERR_INVALID_VALUE;
    }
    callerBundleName = result->second->callerBundleName;
    return ERR_OK;
}

int32_t InsightIntentExecuteManager::AddWantUirsAndFlagsFromParam(
    const std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> &param, Want &want)
{
    if (param == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null param");
        return ERR_INVALID_VALUE;
    }
    if (param->uris_.size() > 0) {
        auto uriVec = want.GetStringArrayParam(AbilityConfig::PARAMS_STREAM);
        for (auto &uri : param->uris_) {
            if (!uri.empty()) {
                uriVec.insert(uriVec.begin(), uri);
            }
        }
        want.SetParam(AbilityConfig::PARAMS_STREAM, uriVec);
        auto flags = want.GetFlags();
        flags |= param->flags_;
        want.SetFlags(flags);
    }
    return ERR_OK;
}

int32_t InsightIntentExecuteManager::UpdateFuncDecoratorParams(
    const std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> &param,
    ExtractInsightIntentInfo &info, Want &want)
{
    if (param->executeMode_ != AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid execute mode %{public}d", param->executeMode_);
        return ERR_INVALID_VALUE;
    }

    if (param->abilityName_.empty()) {
        param->abilityName_ = GetMainElementName(param->bundleName_, param->moduleName_, param->userId_);
    }
    if (param->abilityName_.empty()) {
        TAG_LOGE(AAFwkTag::INTENT, "ability name empty");
        return ERR_INVALID_VALUE;
    }
    want.SetElementName("", param->bundleName_, param->abilityName_, param->moduleName_);

    std::string srcEntrance = info.decoratorFile;
    want.SetParam(INSIGHT_INTENT_SRC_ENTRANCE, srcEntrance);

    std::string className = info.decoratorClass;
    std::string methodName = info.genericInfo.get<InsightIntentFunctionInfo>().functionName;
    std::string methodReturnType = info.genericInfo.get<InsightIntentFunctionInfo>().functionReturnType;
    std::vector<std::string> methodParams = info.genericInfo.get<InsightIntentFunctionInfo>().functionParams;
    const auto &parameters = info.genericInfo.get<InsightIntentFunctionInfo>().parameters;
    if (className.empty() || methodName.empty()) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid func param");
        return ERR_INVALID_VALUE;
    }
    if (!parameters.empty()) {
        std::vector<std::string> encodedMethodParams;
        if (!GetMethodParamNamesFromSchema(methodParams, parameters, encodedMethodParams)) {
            TAG_LOGE(AAFwkTag::INTENT, "encode method params from schema failed");
            return ERR_INVALID_VALUE;
        }
        methodParams = std::move(encodedMethodParams);
    }
    want.SetParam(INSIGHT_INTENT_FUNC_PARAM_CLASSNAME, className);
    want.SetParam(INSIGHT_INTENT_FUNC_PARAM_METHODNAME, methodName);
    want.SetParam(INSIGHT_INTENT_FUNC_PARAM_RETURNTYPE, methodReturnType);
    want.SetParam(INSIGHT_INTENT_FUNC_PARAM_METHODPARAMS, methodParams);
    want.SetParam(INSIGHT_INTENT_ARKTS_MODE, info.arkTSMode);
    return ERR_OK;
}

std::string InsightIntentExecuteManager::GetMainElementName(const std::string &bundleName,
    const std::string &moduleName, int32_t userId)
{
    auto bms = AbilityUtil::GetBundleManagerHelper();
    if (bms == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "get bms failed");
        return "";
    }

    if (userId == DEFAULT_INVAL_VALUE) {
        userId = IPCSkeleton::GetCallingUid() / AppExecFwk::Constants::BASE_USER_RANGE;
    }
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;
    if (IN_PROCESS_CALL(bms->GetLauncherAbilityInfoSync(bundleName, userId, abilityInfos)) != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "get launcher ability info failed");
        return "";
    }

    for (auto &info: abilityInfos) {
        TAG_LOGD(AAFwkTag::INTENT, "moduleName %{public}s", moduleName.c_str());
        if (info.moduleName == moduleName) {
            TAG_LOGI(AAFwkTag::INTENT, "ability matched %{public}s", info.name.c_str());
            return info.name;
        }
    }

    return "";
}

int32_t InsightIntentExecuteManager::UpdatePageDecoratorParams(
    const std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> &param,
    ExtractInsightIntentInfo &info, Want &want)
{
    if (param->executeMode_ != AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid execute mode %{public}d", param->executeMode_);
        return ERR_INVALID_VALUE;
    }

    std::string srcEntrance = info.decoratorFile;
    want.SetParam(INSIGHT_INTENT_SRC_ENTRANCE, srcEntrance);

    std::string pagePath = info.genericInfo.get<InsightIntentPageInfo>().pagePath;
    std::string navigationId = info.genericInfo.get<InsightIntentPageInfo>().navigationId;
    std::string navDestinationName = info.genericInfo.get<InsightIntentPageInfo>().navDestinationName;
    std::string uiAbilityName = info.genericInfo.get<InsightIntentPageInfo>().uiAbility;
    if (pagePath.empty() || uiAbilityName != param->abilityName_) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid page param, pagePath %{public}s, uiability %{public}s, %{public}s",
            pagePath.c_str(), uiAbilityName.c_str(), param->abilityName_.c_str());
        return ERR_INVALID_VALUE;
    }
    if (uiAbilityName.empty()) {
        uiAbilityName = GetMainElementName(param->bundleName_, param->moduleName_);
    }
    if (uiAbilityName.empty()) {
        TAG_LOGE(AAFwkTag::INTENT, "ability name empty");
        return ERR_INVALID_VALUE;
    }
    param->abilityName_ = uiAbilityName;
    want.SetElementName("", param->bundleName_, param->abilityName_, param->moduleName_);
    want.SetParam(INSIGHT_INTENT_PAGE_PARAM_PAGEPATH, pagePath);
    want.SetParam(INSIGHT_INTENT_PAGE_PARAM_NAVIGATIONID, navigationId);
    want.SetParam(INSIGHT_INTENT_PAGE_PARAM_NAVDESTINATIONNAME, navDestinationName);
    return ERR_OK;
}

int32_t InsightIntentExecuteManager::UpdateEntryDecoratorParams(
    const std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> &param,
    ExtractInsightIntentInfo &info, Want &want)
{
    TAG_LOGD(AAFwkTag::INTENT, "update entry params");
    std::string srcEntrance = info.decoratorFile;
    want.SetParam(INSIGHT_INTENT_SRC_ENTRANCE, srcEntrance);
    auto executeMode = param->executeMode_;
    std::vector<ExecuteMode> supportModes = info.genericInfo.get<InsightIntentEntryInfo>().executeMode;
    TAG_LOGD(AAFwkTag::INTENT, "support mode size %{public}zu", supportModes.size());
    bool found = std::find(supportModes.begin(), supportModes.end(), executeMode) != supportModes.end();
    if (!found) {
        TAG_LOGE(AAFwkTag::INTENT, "execute mode %{public}d mismatch", executeMode);
        return ERR_INVALID_VALUE;
    }
    if (param->abilityName_ != info.genericInfo.get<InsightIntentEntryInfo>().abilityName) {
        TAG_LOGE(AAFwkTag::INTENT, "ability name mismatch");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

int32_t InsightIntentExecuteManager::CheckAndUpdateDecoratorParams(
    const std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> &param,
    const AbilityRuntime::ExtractInsightIntentGenericInfo &decoratorInfo,
    Want &want)
{
    // ExtractInsightIntentGenericInfo don't satisfy for now
    ExtractInsightIntentInfo info;
    int32_t userId = param->userId_;
    if (userId == DEFAULT_INVAL_VALUE) {
        userId = IPCSkeleton::GetCallingUid() / AppExecFwk::Constants::BASE_USER_RANGE;
    }
    DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance()->GetInsightIntentInfo(
        param->bundleName_, param->moduleName_, param->insightIntentName_, userId, info);

    InsightIntentType type = InsightIntentType::DECOR_NONE;
    std::string decoratorType = info.genericInfo.decoratorType;
    TAG_LOGD(AAFwkTag::INTENT, "intentName %{public}s, decoratorType %{public}s", param->insightIntentName_.c_str(),
        decoratorType.c_str());
    static const std::unordered_map<std::string, InsightIntentType> mapping = {
        {AbilityRuntime::INSIGHT_INTENTS_DECORATOR_TYPE_LINK, InsightIntentType::DECOR_LINK},
        {AbilityRuntime::INSIGHT_INTENTS_DECORATOR_TYPE_PAGE, InsightIntentType::DECOR_PAGE},
        {AbilityRuntime::INSIGHT_INTENTS_DECORATOR_TYPE_ENTRY, InsightIntentType::DECOR_ENTRY},
        {AbilityRuntime::INSIGHT_INTENTS_DECORATOR_TYPE_FUNCTION, InsightIntentType::DECOR_FUNC},
        {AbilityRuntime::INSIGHT_INTENTS_DECORATOR_TYPE_FORM, InsightIntentType::DECOR_FORM}
    };
    auto it = mapping.find(decoratorType);
    if (it != mapping.end()) {
        type = it->second;
    }

    want.SetParam(INSIGHT_INTENT_DECORATOR_TYPE, static_cast<int>(type));
    TAG_LOGI(AAFwkTag::INTENT, "intentName %{public}s, type %{public}d", param->insightIntentName_.c_str(),
        static_cast<int8_t>(type));
    switch (type) {
        case InsightIntentType::DECOR_FUNC: {
            return UpdateFuncDecoratorParams(param, info, want);
        }
        case InsightIntentType::DECOR_PAGE: {
            return UpdatePageDecoratorParams(param, info, want);
        }
        case InsightIntentType::DECOR_ENTRY: {
            return UpdateEntryDecoratorParams(param, info, want);
        }
        case InsightIntentType::DECOR_NONE:
        case InsightIntentType::DECOR_LINK:
        case InsightIntentType::DECOR_FORM:
        default:
            break;
    }
    return ERR_OK;
}

int32_t InsightIntentExecuteManager::GenerateWant(
    const std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> &param,
    const AbilityRuntime::ExtractInsightIntentGenericInfo &decoratorInfo,
    Want &want)
{
    if (param == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null param");
        return ERR_INVALID_VALUE;
    }
    want.SetElementName("", param->bundleName_, param->abilityName_, param->moduleName_);

    if (param->insightIntentParam_ != nullptr) {
        sptr<AAFwk::IWantParams> pExecuteParams = WantParamWrapper::Box(*param->insightIntentParam_);
        if (pExecuteParams != nullptr) {
            WantParams wantParams;
            wantParams.SetParam(INSIGHT_INTENT_EXECUTE_PARAM_PARAM, pExecuteParams);
            want.SetParams(wantParams);
        }
    }

    std::string srcEntry;
    std::string arkTSMode;
    std::string decoratorClass;
    auto ret = AbilityRuntime::InsightIntentUtils::GetSrcEntry(want.GetElement(), param->insightIntentName_,
        static_cast<AppExecFwk::ExecuteMode>(param->executeMode_), srcEntry, &arkTSMode, param->userId_,
        &decoratorClass);
    if (!arkTSMode.empty()) {
        want.SetParam(INSIGHT_INTENT_ARKTS_MODE, arkTSMode);
    }
    if (!decoratorClass.empty()) {
        want.SetParam(INSIGHT_INTENT_DECORATOR_CLASS, decoratorClass);
    }
    if (!srcEntry.empty()) {
        want.SetParam(INSIGHT_INTENT_SRC_ENTRY, srcEntry);
    } else if (decoratorInfo.decoratorType == "" && ret == ERR_INSIGHT_INTENT_GET_PROFILE_FAILED &&
        param->executeMode_ == AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND && !param->isServiceMatch_) {
        TAG_LOGI(AAFwkTag::INTENT, "insight intent srcEntry invalid, try free install ondemand");
        std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
        want.SetParam(Want::PARAM_RESV_START_TIME, startTime);
        want.AddFlags(Want::FLAG_INSTALL_ON_DEMAND);
    } else if (decoratorInfo.decoratorType == "" && !param->isServiceMatch_ && param->deviceId_.empty()) {
        // decoratorType is empty indicate no decorator
        TAG_LOGE(AAFwkTag::INTENT, "insight intent srcEntry invalid");
        return ERR_INVALID_VALUE;
    }

    want.SetParam(INSIGHT_INTENT_EXECUTE_PARAM_NAME, param->insightIntentName_);
    want.SetParam(INSIGHT_INTENT_EXECUTE_PARAM_MODE, param->executeMode_);
    want.SetParam(INSIGHT_INTENT_EXECUTE_PARAM_ID, std::to_string(param->insightIntentId_));
    want.SetParam(INSIGHT_INTENT_PARAM_USER_ID, param->userId_);
    if (param->displayId_ != INVALID_DISPLAY_ID) {
        want.SetParam(Want::PARAM_RESV_DISPLAY_ID, param->displayId_);
        TAG_LOGD(AAFwkTag::INTENT, "Generate want with displayId: %{public}d", param->displayId_);
    }

    if (!param->deviceId_.empty()) {
        want.SetDeviceId(param->deviceId_);
    }
    if (!param->uris_.empty()) {
        want.SetParam(INSIGHT_INTENT_EXECUTE_PARAM_URI, param->uris_);
    }

    auto intRet = AddWantUirsAndFlagsFromParam(param, want);
    if (intRet != ERR_OK) {
        return intRet;
    }

    intRet = CheckAndUpdateDecoratorParams(param, decoratorInfo, want);
    if (intRet != ERR_OK) {
        // log has print in sub method
        return intRet;
    }

    return ERR_OK;
}

int32_t InsightIntentExecuteManager::IsValidCall(const Want &want)
{
    std::string insightIntentName = want.GetStringParam(INSIGHT_INTENT_EXECUTE_PARAM_NAME);
    if (insightIntentName.empty()) {
        TAG_LOGE(AAFwkTag::INTENT, "empty insightIntentName");
        return ERR_INVALID_VALUE;
    }
    TAG_LOGD(AAFwkTag::INTENT, "insightIntentName: %{public}s", insightIntentName.c_str());

    int32_t ret = CheckCallerPermission();
    if (ret != ERR_OK) {
        return ret;
    }
    return ERR_OK;
}

int32_t InsightIntentExecuteManager::CheckCallerPermission(uint64_t specifiedFullTokenId)
{
    TAG_LOGI(AAFwkTag::INTENT, "specifiedFullTokenId: %{public}" PRIu64, specifiedFullTokenId);
    bool isSystemAppCall = false;
    if (specifiedFullTokenId != 0) {
        isSystemAppCall = PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPIByTokenId(
            specifiedFullTokenId);
    } else {
        isSystemAppCall = PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI();
    }
    if (!isSystemAppCall) {
        TAG_LOGE(AAFwkTag::INTENT, "system-api cannot use");
        return ERR_NOT_SYSTEM_APP;
    }

    bool isCallingPerm = PermissionVerification::GetInstance()->VerifyCallingPermission(
        EXECUTE_INSIGHT_INTENT_PERMISSION, specifiedFullTokenId);
    if (!isCallingPerm) {
        TAG_LOGE(AAFwkTag::INTENT, "permission %{public}s verification failed", EXECUTE_INSIGHT_INTENT_PERMISSION);
        return ERR_PERMISSION_DENIED;
    }
    return ERR_OK;
}

int32_t InsightIntentExecuteManager::CheckGetInsightIntenInfoPermission()
{
    bool isSystemAppCall = PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI();
    if (!isSystemAppCall) {
        TAG_LOGE(AAFwkTag::INTENT, "system-api cannot use");
        return ERR_NOT_SYSTEM_APP;
    }

    bool isCallingPerm = PermissionVerification::GetInstance()->VerifyGetBundleInfoPrivilegedPermission();
    if (!isCallingPerm) {
        TAG_LOGE(AAFwkTag::INTENT, "permission %{public}s verification failed", PERMISSION_GET_BUNDLE_INFO_PRIVILEGED);
        return ERR_PERMISSION_DENIED;
    }
    return ERR_OK;
}

void InsightIntentExecuteManager::SetIntentExemptionInfo(int32_t uid)
{
    std::lock_guard<ffrt::mutex> guard(intentExemptionLock_);
    std::map<int32_t, int64_t>::iterator iter = intentExemptionDeadlineTime_.find(uid);
    intentExemptionDeadlineTime_[uid] = AbilityRuntime::TimeUtil::CurrentTimeMillis();
}

bool InsightIntentExecuteManager::CheckIntentIsExemption(int32_t uid)
{
    std::lock_guard<ffrt::mutex> guard(intentExemptionLock_);
    if (intentExemptionDeadlineTime_.find(uid) != intentExemptionDeadlineTime_.end()) {
        if (AbilityRuntime::TimeUtil::CurrentTimeMillis() - INTENT_EXEMPTION_DURATION <=
            intentExemptionDeadlineTime_[uid]) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "exemption check uid:%{public}d", uid);
            return true;
        } else {
            intentExemptionDeadlineTime_.erase(uid);
            return false;
        }
    }
    return false;
}

std::map<int32_t, int64_t> InsightIntentExecuteManager::GetAllIntentExemptionInfo() const
{
    std::lock_guard<ffrt::mutex> guard(intentExemptionLock_);
    return intentExemptionDeadlineTime_;
}

void InsightIntentExecuteManager::SendIntentReport(EventInfo &eventInfo, int32_t errCode)
{
    eventInfo.errCode = errCode;
    EventReport::SendExecuteIntentEvent(EventName::EXECUTE_INSIGHT_INTENT_ERROR, HISYSEVENT_FAULT, eventInfo);
}

void InsightIntentExecuteManager::OnInsightAppDied(const std::string &bundleName)
{
    TAG_LOGI(AAFwkTag::INTENT, "app died: %{public}s", bundleName.c_str());
    std::lock_guard<ffrt::mutex> lock(mutex_);
    AppExecFwk::InsightIntentExecuteResult errorResult{};
    errorResult.innerErr = AbilityRuntime::InsightIntentInnerErr::INSIGHT_INTENT_EXECUTE_REPLY_FAILED;

    std::vector<uint64_t> toRemove;
    for (const auto &[intentId, record] : records_) {
        if (record == nullptr || bundleName != record->bundleName) {
            continue;
        }
        toRemove.push_back(intentId);
        sptr<IInsightIntentExecuteCallback> remoteCallback =
            iface_cast<IInsightIntentExecuteCallback>(record->callerToken);
        if (remoteCallback != nullptr) {
            TAG_LOGD(AAFwkTag::INTENT, "notify caller, intentId: %{public}" PRIu64, intentId);
            remoteCallback->OnExecuteDone(record->key, errorResult.innerErr, errorResult);
        }

        if (record->callerToken != nullptr && record->deathRecipient != nullptr) {
            record->callerToken->RemoveDeathRecipient(record->deathRecipient);
        }
    }

    for (uint64_t intentId : toRemove) {
        records_.erase(intentId);
    }
    TAG_LOGI(AAFwkTag::INTENT, "removed %{public}zu records, remaining: %{public}zu",
        toRemove.size(), records_.size());
}

int32_t InsightIntentExecuteManager::CheckAndUpdateQueryEntityParam(uint64_t key,
    const sptr<IRemoteObject> &callerToken, std::shared_ptr<AppExecFwk::InsightIntentQueryParam> &param,
    const std::string& callerBundleName)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    int32_t result = CheckCallerPermission();
    if (result != ERR_OK) {
        return result;
    }

    if (callerToken == nullptr || param == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null callerToken or param");
        return ERR_INVALID_VALUE;
    }

    if (param->userId_ == DEFAULT_INVAL_VALUE) {
        param->userId_ = IPCSkeleton::GetCallingUid() / AppExecFwk::Constants::BASE_USER_RANGE;
    }

    uint64_t intentId = 0;
    result = AddRecord(key, callerToken, param->bundleName_, intentId, callerBundleName);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "add record failed, result:%{public}d", result);
        return result;
    }
    param->intentId_ = intentId;
    return ERR_OK;
}

std::shared_ptr<AbilityRuntime::InsightIntentEntityInfo> InsightIntentExecuteManager::CheckEntityQueryable(
    const ExtractInsightIntentInfo& intentInfo, const std::string& className,
    const AppExecFwk::InsightIntentQueryEntityParam& queryEntityParam)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    if (className.empty()) {
        TAG_LOGW(AAFwkTag::INTENT, "className is empty");
        return nullptr;
    }

    if (queryEntityParam.queryType_.compare("byProperty") == 0 && queryEntityParam.parameters_ == nullptr) {
        TAG_LOGW(AAFwkTag::INTENT, "parameters is nullptr");
        return nullptr;
    }

    if (intentInfo.entities.size() <= 0) {
        return nullptr;
    }

    for (size_t i = 0; i < intentInfo.entities.size(); i++) {
        const auto& entity = intentInfo.entities[i];
        TAG_LOGD(AAFwkTag::INTENT, "queryClassName:%{public}s, entityid:%{public}s, className:%{public}s,"
            "parentClassName:%{public}s, isQueryable:%{public}d, supportedProperty count:%{public}zu",
            className.c_str(), entity.entityId.c_str(), entity.className.c_str(), entity.parentClassName.c_str(),
            entity.isQueryable(), entity.supportedQueryProperties.size());

        if (!entity.isQueryable() || entity.className.compare(className) != 0) {
            continue;
        }

        if (queryEntityParam.queryType_.compare("all") == 0) {
            return std::make_shared<AbilityRuntime::InsightIntentEntityInfo>(entity);
        }

        bool found = true;
        auto queryKeys = queryEntityParam.parameters_->KeySet();
        for (const std::string& key : queryKeys) {
            TAG_LOGD(AAFwkTag::INTENT, "start check query key %{public}s", key.c_str());
            auto it = std::find(entity.supportedQueryProperties.begin(), entity.supportedQueryProperties.end(), key);
            if (it == entity.supportedQueryProperties.end()) {
                TAG_LOGW(AAFwkTag::INTENT, "query key %{public}s not found in supportedQueryProperties", key.c_str());
                found = false;
                break;
            }
        }
        if (found) {
            return std::make_shared<AbilityRuntime::InsightIntentEntityInfo>(entity);
        }
    }
    return nullptr;
}

int32_t InsightIntentExecuteManager::GenerateQueryEntityWant(
    const std::shared_ptr<AppExecFwk::InsightIntentQueryParam> &param, Want &want)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    if (param == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null param");
        return ERR_INVALID_VALUE;
    }

    ExtractInsightIntentInfo intentInfo;
    DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance()->GetInsightIntentInfo(
        param->bundleName_, param->moduleName_, param->intentName_, param->userId_, intentInfo);

    auto entity = CheckEntityQueryable(intentInfo, param->className_, param->queryEntityParam_);
    if (entity == nullptr) {
        TAG_LOGW(AAFwkTag::INTENT, "target intent is not queryable, bundle:%{public}s, module:%{public}s,"
            "intent:%{public}s", param->bundleName_.c_str(), param->moduleName_.c_str(), param->intentName_.c_str());
        return INTENT_NOT_EXIST;
    }

    auto abilityName = InsightIntentExecuteManager::GetMainElementName(param->bundleName_, param->moduleName_);
    if (abilityName.empty()) {
        TAG_LOGE(AAFwkTag::INTENT, "ability name empty");
        return ERR_INVALID_VALUE;
    }
    want.SetElementName("", param->bundleName_, abilityName, param->moduleName_);

    WantParams wantParams;
    sptr<AAFwk::IWantParams> pExecuteParams = WantParamWrapper::Box(wantParams);
    if (pExecuteParams != nullptr) {
        wantParams.SetParam(INSIGHT_INTENT_EXECUTE_PARAM_PARAM, pExecuteParams);
    }
    if (param->queryEntityParam_.parameters_ != nullptr) {
        sptr<AAFwk::IWantParams> pQueryEntityParams = WantParamWrapper::Box(*param->queryEntityParam_.parameters_);
        wantParams.SetParam(INSIGHT_INTENT_QUERY_ENTITY_PARAM_PARAM, pQueryEntityParams);
    }
    want.SetParams(wantParams);

    intentInfo.decoratorFile = entity->decoratorFile;
    TAG_LOGD(AAFwkTag::INTENT, "intentId %{public}" PRIu64 ", arkTSMode is %{public}s, srcEntry is %{public}s,"
        "intentName is %{public}s, className is %{public}s, queryType is %{public}s", param->intentId_,
        intentInfo.arkTSMode.c_str(), intentInfo.decoratorFile.c_str(), param->intentName_.c_str(),
        param->className_.c_str(), param->queryEntityParam_.queryType_.c_str());

    want.SetParam(INSIGHT_INTENT_ARKTS_MODE, intentInfo.arkTSMode);
    want.SetParam(INSIGHT_INTENT_SRC_ENTRANCE, intentInfo.decoratorFile);
    want.SetParam(INSIGHT_INTENT_EXECUTE_PARAM_NAME, param->intentName_);
    want.SetParam(INSIGHT_INTENT_EXECUTE_PARAM_MODE,
        static_cast<int32_t>(AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND));
    want.SetParam(INSIGHT_INTENT_EXECUTE_PARAM_ID, std::to_string(param->intentId_));
    want.SetParam(INSIGHT_INTENT_DECORATOR_TYPE, static_cast<int>(InsightIntentType::DECOR_QUERY_ENTITY));

    want.SetParam(INSIGHT_INTENT_QUERY_ENTITY_CLASS_NAME, param->className_);
    want.SetParam(INSIGHT_INTENT_QUERY_TYPE, param->queryEntityParam_.queryType_);
    return ERR_OK;
}
} // namespace AAFwk
} // namespace OHOS
