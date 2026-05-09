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

#include "insight_intent_execute_param.h"

#include <charconv>
#include <limits>
#include <sstream>

#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "insight_intent_constant.h"
#include "int_wrapper.h"
#include "string_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
using WantParams = OHOS::AAFwk::WantParams;

namespace {
bool IsValidParamType(ParamType type)
{
    switch (type) {
        case ParamType::STRING:
        case ParamType::NUMBER:
        case ParamType::BOOLEAN:
        case ParamType::OBJECT:
        case ParamType::ARRAY:
        case ParamType::INTEGER:
        case ParamType::UNKNOWN:
            return true;
        default:
            return false;
    }
}

uint8_t GetParamTypeValue(ParamType type)
{
    if (IsValidParamType(type)) {
        return static_cast<uint8_t>(type);
    }
    TAG_LOGW(AAFwkTag::INTENT, "invalid param type enum, fallback to unknown");
    return static_cast<uint8_t>(ParamType::UNKNOWN);
}

bool ParseParamTypeValue(const std::string &typeStr, ParamType &type)
{
    uint32_t typeValue = 0;
    auto parseRet = std::from_chars(typeStr.data(), typeStr.data() + typeStr.size(), typeValue);
    if (parseRet.ec != std::errc() || parseRet.ptr != typeStr.data() + typeStr.size()) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid param type string: %{public}s", typeStr.c_str());
        return false;
    }
    if (typeValue > std::numeric_limits<uint8_t>::max()) {
        TAG_LOGE(AAFwkTag::INTENT, "param type out of range: %{public}u", typeValue);
        return false;
    }
    ParamType parsedType = static_cast<ParamType>(typeValue);
    if (!IsValidParamType(parsedType)) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid param type value: %{public}u", static_cast<uint8_t>(typeValue));
        return false;
    }
    type = parsedType;
    return true;
}

bool ParseRequiredValue(const std::string &requiredStr, bool &isRequired)
{
    if (requiredStr == "1") {
        isRequired = true;
        return true;
    }
    if (requiredStr == "0") {
        isRequired = false;
        return true;
    }
    TAG_LOGE(AAFwkTag::INTENT, "invalid param required string: %{public}s", requiredStr.c_str());
    return false;
}
} // namespace

std::string EncodeMethodParam(const InsightIntentParam &param)
{
    std::ostringstream builder;
    builder << param.paramName << METHOD_PARAM_SEPARATOR
        << static_cast<uint32_t>(GetParamTypeValue(param.type)) << METHOD_PARAM_SEPARATOR
        << (param.isRequired ? '1' : '0');
    return builder.str();
}

bool DecodeMethodParam(const std::string &encodedParam, InsightIntentParam &param)
{
    auto firstSeparator = encodedParam.find(METHOD_PARAM_SEPARATOR);
    if (firstSeparator == std::string::npos) {
        param.paramName = encodedParam;
        param.type = ParamType::UNKNOWN;
        param.isRequired = false;
        return !param.paramName.empty();
    }
    auto secondSeparator = encodedParam.find(METHOD_PARAM_SEPARATOR, firstSeparator + 1);
    if (secondSeparator == std::string::npos) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid encoded method param: %{public}s", encodedParam.c_str());
        return false;
    }
    param.paramName = encodedParam.substr(0, firstSeparator);
    if (param.paramName.empty()) {
        TAG_LOGE(AAFwkTag::INTENT, "empty method param name");
        return false;
    }
    auto typeStr = encodedParam.substr(firstSeparator + 1, secondSeparator - firstSeparator - 1);
    if (!ParseParamTypeValue(typeStr, param.type)) {
        return false;
    }
    auto requiredStr = encodedParam.substr(secondSeparator + 1);
    return ParseRequiredValue(requiredStr, param.isRequired);
}

std::string GetMethodParamName(const std::string &encodedMethodParam)
{
    auto separatorPos = encodedMethodParam.find(METHOD_PARAM_SEPARATOR);
    return separatorPos == std::string::npos ? encodedMethodParam : encodedMethodParam.substr(0, separatorPos);
}

bool InsightIntentExecuteParam::ReadFromParcel(Parcel &parcel)
{
    bundleName_ = Str16ToStr8(parcel.ReadString16());
    moduleName_ = Str16ToStr8(parcel.ReadString16());
    abilityName_ = Str16ToStr8(parcel.ReadString16());
    insightIntentName_ = Str16ToStr8(parcel.ReadString16());
    std::shared_ptr<WantParams> wantParams(parcel.ReadParcelable<WantParams>());
    if (wantParams == nullptr) {
        return false;
    }
    insightIntentParam_ = wantParams;
    executeMode_ = parcel.ReadInt32();
    insightIntentId_ = parcel.ReadUint64();
    displayId_ = parcel.ReadInt32();
    parcel.ReadStringVector(&uris_);
    flags_ = parcel.ReadInt32();
    userId_ = parcel.ReadInt32();
    decoratorType_ = parcel.ReadInt8();
    srcEntrance_ = Str16ToStr8(parcel.ReadString16());
    className_ = Str16ToStr8(parcel.ReadString16());
    methodName_ = Str16ToStr8(parcel.ReadString16());
    methodReturnType_ = Str16ToStr8(parcel.ReadString16());
    parcel.ReadStringVector(&methodParams_);
    pagePath_ = Str16ToStr8(parcel.ReadString16());
    navigationId_ = Str16ToStr8(parcel.ReadString16());
    navDestinationName_ = Str16ToStr8(parcel.ReadString16());
    isServiceMatch_ = parcel.ReadBool();
    deviceId_ = Str16ToStr8(parcel.ReadString16());
    return true;
}

InsightIntentExecuteParam *InsightIntentExecuteParam::Unmarshalling(Parcel &parcel)
{
    InsightIntentExecuteParam *param = new (std::nothrow) InsightIntentExecuteParam();
    if (param == nullptr) {
        return nullptr;
    }

    if (!param->ReadFromParcel(parcel)) {
        delete param;
        param = nullptr;
    }
    return param;
}

bool InsightIntentExecuteParam::Marshalling(Parcel &parcel) const
{
    parcel.WriteString16(Str8ToStr16(bundleName_));
    parcel.WriteString16(Str8ToStr16(moduleName_));
    parcel.WriteString16(Str8ToStr16(abilityName_));
    parcel.WriteString16(Str8ToStr16(insightIntentName_));
    parcel.WriteParcelable(insightIntentParam_.get());
    parcel.WriteInt32(executeMode_);
    parcel.WriteUint64(insightIntentId_);
    parcel.WriteInt32(displayId_);
    parcel.WriteStringVector(uris_);
    parcel.WriteInt32(flags_);
    parcel.WriteInt32(userId_);
    parcel.WriteInt8(decoratorType_);
    parcel.WriteString16(Str8ToStr16(srcEntrance_));
    parcel.WriteString16(Str8ToStr16(className_));
    parcel.WriteString16(Str8ToStr16(methodName_));
    parcel.WriteString16(Str8ToStr16(methodReturnType_));
    parcel.WriteStringVector(methodParams_);
    parcel.WriteString16(Str8ToStr16(pagePath_));
    parcel.WriteString16(Str8ToStr16(navigationId_));
    parcel.WriteString16(Str8ToStr16(navDestinationName_));
    parcel.WriteBool(isServiceMatch_);
    parcel.WriteString16(Str8ToStr16(deviceId_));
    return true;
}

bool InsightIntentExecuteParam::IsInsightIntentExecute(const AAFwk::Want &want)
{
    if (want.HasParameter(INSIGHT_INTENT_EXECUTE_PARAM_NAME)) {
        return true;
    }
    return false;
}

bool InsightIntentExecuteParam::IsInsightIntentPage(const AAFwk::Want &want)
{
    if (!want.HasParameter(INSIGHT_INTENT_EXECUTE_PARAM_NAME)) {
        return false;
    }

    const WantParams &wantParams = want.GetParams();
    auto type = wantParams.GetIntParam(INSIGHT_INTENT_DECORATOR_TYPE, -1);
    if (type == static_cast<int>(AbilityRuntime::InsightIntentType::DECOR_PAGE)) {
        return true;
    }

    return false;
}

bool InsightIntentExecuteParam::GenerateFromWant(const AAFwk::Want &want,
    InsightIntentExecuteParam &executeParam)
{
    const WantParams &wantParams = want.GetParams();
    if (!wantParams.HasParam(INSIGHT_INTENT_EXECUTE_PARAM_NAME)) {
        TAG_LOGE(AAFwkTag::INTENT, "empty want");
        return false;
    }
    uint64_t insightIntentId = 0;
    std::string idStr = wantParams.GetStringParam(INSIGHT_INTENT_EXECUTE_PARAM_ID);
    auto res = std::from_chars(idStr.c_str(), idStr.c_str() + idStr.size(), insightIntentId);
    if (res.ec != std::errc()) {
        TAG_LOGE(AAFwkTag::ABILITY, "invalid insight intent ID");
        return false;
    }

    AppExecFwk::ElementName elementName = want.GetElement();
    executeParam.bundleName_ = elementName.GetBundleName();
    executeParam.moduleName_ = elementName.GetModuleName();
    executeParam.abilityName_ = elementName.GetAbilityName();
    executeParam.insightIntentName_ = wantParams.GetStringParam(INSIGHT_INTENT_EXECUTE_PARAM_NAME);
    executeParam.insightIntentId_ = insightIntentId;
    executeParam.executeMode_ = wantParams.GetIntParam(INSIGHT_INTENT_EXECUTE_PARAM_MODE, 0);

    auto insightIntentParam = wantParams.GetWantParams(INSIGHT_INTENT_EXECUTE_PARAM_PARAM);
    UpdateInsightIntentCallerInfo(wantParams, insightIntentParam);
    executeParam.insightIntentParam_ = std::make_shared<WantParams>(insightIntentParam);
    executeParam.uris_ = want.GetStringArrayParam(INSIGHT_INTENT_EXECUTE_PARAM_URI);
    executeParam.flags_ = wantParams.GetIntParam(INSIGHT_INTENT_EXECUTE_PARAM_FLAGS, 0);
    executeParam.decoratorType_ = wantParams.GetIntParam(INSIGHT_INTENT_DECORATOR_TYPE, 0);
    executeParam.srcEntrance_ = wantParams.GetStringParam(INSIGHT_INTENT_SRC_ENTRANCE);
    executeParam.className_ = wantParams.GetStringParam(INSIGHT_INTENT_FUNC_PARAM_CLASSNAME);
    executeParam.methodName_ = wantParams.GetStringParam(INSIGHT_INTENT_FUNC_PARAM_METHODNAME);
    executeParam.methodReturnType_ = wantParams.GetStringParam(INSIGHT_INTENT_FUNC_PARAM_RETURNTYPE);
    executeParam.methodParams_ = want.GetStringArrayParam(INSIGHT_INTENT_FUNC_PARAM_METHODPARAMS);
    executeParam.pagePath_ = wantParams.GetStringParam(INSIGHT_INTENT_PAGE_PARAM_PAGEPATH);
    executeParam.navigationId_ = wantParams.GetStringParam(INSIGHT_INTENT_PAGE_PARAM_NAVIGATIONID);
    executeParam.navDestinationName_ = wantParams.GetStringParam(INSIGHT_INTENT_PAGE_PARAM_NAVDESTINATIONNAME);
    executeParam.queryEntityClassName_ = wantParams.GetStringParam(INSIGHT_INTENT_QUERY_ENTITY_CLASS_NAME);
    executeParam.queryType_ = wantParams.GetStringParam(INSIGHT_INTENT_QUERY_TYPE);
    executeParam.userId_ = wantParams.GetIntParam(INSIGHT_INTENT_PARAM_USER_ID, -1);
    auto queryParams = wantParams.GetWantParams(INSIGHT_INTENT_QUERY_ENTITY_PARAM_PARAM);
    executeParam.queryParams_ = std::make_shared<WantParams>(queryParams);
    return true;
}

bool InsightIntentExecuteParam::RemoveInsightIntent(AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (want.HasParameter(INSIGHT_INTENT_EXECUTE_PARAM_NAME)) {
        want.RemoveParam(INSIGHT_INTENT_EXECUTE_PARAM_NAME);
    }
    if (want.HasParameter(INSIGHT_INTENT_EXECUTE_PARAM_ID)) {
        want.RemoveParam(INSIGHT_INTENT_EXECUTE_PARAM_ID);
    }
    if (want.HasParameter(INSIGHT_INTENT_EXECUTE_PARAM_MODE)) {
        want.RemoveParam(INSIGHT_INTENT_EXECUTE_PARAM_MODE);
    }
    if (want.HasParameter(INSIGHT_INTENT_EXECUTE_PARAM_PARAM)) {
        want.RemoveParam(INSIGHT_INTENT_EXECUTE_PARAM_PARAM);
    }
    if (want.HasParameter(INSIGHT_INTENT_SRC_ENTRY)) {
        want.RemoveParam(INSIGHT_INTENT_SRC_ENTRY);
    }
    if (want.HasParameter(INSIGHT_INTENT_ARKTS_MODE)) {
        want.RemoveParam(INSIGHT_INTENT_ARKTS_MODE);
    }
    if (want.HasParameter(INSIGHT_INTENT_EXECUTE_PARAM_URI)) {
        want.RemoveParam(INSIGHT_INTENT_EXECUTE_PARAM_URI);
    }
    if (want.HasParameter(INSIGHT_INTENT_EXECUTE_PARAM_FLAGS)) {
        want.RemoveParam(INSIGHT_INTENT_EXECUTE_PARAM_FLAGS);
    }
    if (want.HasParameter(INSIGHT_INTENT_EXECUTE_OPENLINK_FLAG)) {
        want.RemoveParam(INSIGHT_INTENT_EXECUTE_OPENLINK_FLAG);
    }
    if (want.HasParameter(INSIGHT_INTENT_DECORATOR_TYPE)) {
	    want.RemoveParam(INSIGHT_INTENT_DECORATOR_TYPE);
    }
    if (want.HasParameter(INSIGHT_INTENT_SRC_ENTRANCE)) {
        want.RemoveParam(INSIGHT_INTENT_SRC_ENTRANCE);
    }
    if (want.HasParameter(INSIGHT_INTENT_FUNC_PARAM_CLASSNAME)) {
        want.RemoveParam(INSIGHT_INTENT_FUNC_PARAM_CLASSNAME);
    }
    if (want.HasParameter(INSIGHT_INTENT_FUNC_PARAM_METHODNAME)) {
        want.RemoveParam(INSIGHT_INTENT_FUNC_PARAM_METHODNAME);
    }
    if (want.HasParameter(INSIGHT_INTENT_FUNC_PARAM_RETURNTYPE)) {
        want.RemoveParam(INSIGHT_INTENT_FUNC_PARAM_RETURNTYPE);
    }
    if (want.HasParameter(INSIGHT_INTENT_FUNC_PARAM_METHODPARAMS)) {
        want.RemoveParam(INSIGHT_INTENT_FUNC_PARAM_METHODPARAMS);
    }
    if (want.HasParameter(INSIGHT_INTENT_PAGE_PARAM_PAGEPATH)) {
        want.RemoveParam(INSIGHT_INTENT_PAGE_PARAM_PAGEPATH);
    }
    if (want.HasParameter(INSIGHT_INTENT_PAGE_PARAM_NAVIGATIONID)) {
        want.RemoveParam(INSIGHT_INTENT_PAGE_PARAM_NAVIGATIONID);
    }
    if (want.HasParameter(INSIGHT_INTENT_PAGE_PARAM_NAVDESTINATIONNAME)) {
        want.RemoveParam(INSIGHT_INTENT_PAGE_PARAM_NAVDESTINATIONNAME);
    }
    if (want.HasParameter(INSIGHT_INTENT_QUERY_ENTITY_CLASS_NAME)) {
        want.RemoveParam(INSIGHT_INTENT_QUERY_ENTITY_CLASS_NAME);
    }
    return true;
}

void InsightIntentExecuteParam::UpdateInsightIntentCallerInfo(const WantParams &wantParams,
    WantParams &insightIntentParam)
{
    insightIntentParam.Remove(AAFwk::Want::PARAM_RESV_CALLER_TOKEN);
    insightIntentParam.SetParam(AAFwk::Want::PARAM_RESV_CALLER_TOKEN,
        AAFwk::Integer::Box(wantParams.GetIntParam(AAFwk::Want::PARAM_RESV_CALLER_TOKEN, 0)));

    insightIntentParam.Remove(AAFwk::Want::PARAM_RESV_CALLER_UID);
    insightIntentParam.SetParam(AAFwk::Want::PARAM_RESV_CALLER_UID,
        AAFwk::Integer::Box(wantParams.GetIntParam(AAFwk::Want::PARAM_RESV_CALLER_UID, 0)));

    insightIntentParam.Remove(AAFwk::Want::PARAM_RESV_CALLER_PID);
    insightIntentParam.SetParam(AAFwk::Want::PARAM_RESV_CALLER_PID,
        AAFwk::Integer::Box(wantParams.GetIntParam(AAFwk::Want::PARAM_RESV_CALLER_PID, 0)));

    insightIntentParam.Remove(AAFwk::Want::PARAM_RESV_CALLER_BUNDLE_NAME);
    insightIntentParam.SetParam(AAFwk::Want::PARAM_RESV_CALLER_BUNDLE_NAME,
        AAFwk::String::Box(wantParams.GetStringParam(AAFwk::Want::PARAM_RESV_CALLER_BUNDLE_NAME)));
}
} // namespace AppExecFwk
} // namespace OHOS
