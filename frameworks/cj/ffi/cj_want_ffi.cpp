/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#include "cj_want_ffi.h"

#include <cstring>
#include <string>
#include <vector>

#include "cj_utils_ffi.h"
#include "int_wrapper.h"
#include "string_wrapper.h"
#include "want.h"
#include "want_params.h"
#include "want_params_wrapper.h"
#include "hilog_tag_wrapper.h"

using OHOS::AAFwk::Want;
using OHOS::AppExecFwk::ElementName;

static const char* FD = "FD";
static const char* TYPE_PROPERTY = "type";
static const char* VALUE_PROPERTY = "value";
static const int PROPERTIES_SIZE = 2;

static bool IsFdParam(const OHOS::AAFwk::WantParams &wp)
{
    auto valueMap = wp.GetParams();
    if (valueMap.size() != PROPERTIES_SIZE) {
        return false;
    }
    auto typeIt = valueMap.find(TYPE_PROPERTY);
    if (typeIt == valueMap.end()) {
        return false;
    }
    OHOS::AAFwk::IString *strValue = OHOS::AAFwk::IString::Query(typeIt->second);
    if (strValue == nullptr) {
        return false;
    }
    return OHOS::AAFwk::String::Unbox(strValue) == FD;
}

static bool ExtractFdValue(const OHOS::AAFwk::WantParams &wp, int32_t &fdValue)
{
    auto valueIt = wp.GetParams().find(VALUE_PROPERTY);
    if (valueIt == wp.GetParams().end()) {
        return false;
    }
    OHOS::AAFwk::IInteger *intValue = OHOS::AAFwk::IInteger::Query(valueIt->second);
    if (intValue == nullptr) {
        return false;
    }
    fdValue = OHOS::AAFwk::Integer::Unbox(intValue);
    return true;
}

static void InjectFdsArrayToWantParams(const CJArrFdParam &fdsArr, OHOS::AAFwk::WantParams &wantParams)
{
    if (fdsArr.head == nullptr || fdsArr.size <= 0) {
        return;
    }

    for (int64_t i = 0; i < fdsArr.size; i++) {
        CJFdParam* fdParam = fdsArr.head + i;
        if (fdParam == nullptr || fdParam->key == nullptr) {
            continue;
        }

        std::string key(fdParam->key);
        int32_t fdValue = fdParam->value;

        OHOS::AAFwk::WantParams fdParamObj;
        fdParamObj.SetParam(TYPE_PROPERTY, OHOS::AAFwk::String::Box(FD));
        fdParamObj.SetParam(VALUE_PROPERTY, OHOS::AAFwk::Integer::Box(fdValue));
        wantParams.SetParam(key, OHOS::AAFwk::WantParamWrapper::Box(fdParamObj));
    }
}

static CJArrFdParam ExtractFdsArrayFromWantParams(const OHOS::AAFwk::WantParams &wantParams)
{
    CJArrFdParam result = {nullptr, 0};
    auto paramList = wantParams.GetParams();
    std::vector<std::pair<std::string, int32_t>> fdsList;

    for (auto it = paramList.begin(); it != paramList.end(); it++) {
        auto value = wantParams.GetParam(it->first);
        OHOS::AAFwk::IWantParams *o = OHOS::AAFwk::IWantParams::Query(value);
        if (o == nullptr) {
            continue;
        }
        OHOS::AAFwk::WantParams wp = OHOS::AAFwk::WantParamWrapper::Unbox(o);
        if (!IsFdParam(wp)) {
            continue;
        }
        int32_t fdValue = 0;
        if (ExtractFdValue(wp, fdValue)) {
            fdsList.push_back(std::make_pair(it->first, fdValue));
        }
    }

    if (fdsList.empty()) {
        return result;
    }

    result.head = static_cast<CJFdParam*>(malloc(sizeof(CJFdParam) * fdsList.size()));
    if (result.head == nullptr) {
        return result;
    }

    result.size = static_cast<int64_t>(fdsList.size());
    for (size_t i = 0; i < fdsList.size(); i++) {
        result.head[i].key = CreateCStringFromString(fdsList[i].first);
        result.head[i].value = fdsList[i].second;
    }

    return result;
}

// Attention: The function does not handle entities.
WantHandle FFICJWantCreateWithWantInfo(CJWantParams params)
{
    Want* want = new (std::nothrow) Want();
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::DEFAULT, "null want");
        return nullptr;
    }

    auto element = reinterpret_cast<ElementName*>(params.elementName);
    want->SetElement(*element);
    want->SetFlags(params.flags);
    want->SetUri(params.uri);
    want->SetAction(params.action);
    want->SetType(params.wantType);
    want->SetParams(OHOS::AAFwk::WantParamWrapper::ParseWantParamsWithBrackets(params.parameters));

    return want;
}

WantHandle FFICJWantCreateWithWantInfoV2(CJWantParamsV2 params)
{
    Want* want = new (std::nothrow) Want();
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::DEFAULT, "null want");
        return nullptr;
    }

    auto element = reinterpret_cast<ElementName*>(params.elementName);
    want->SetElement(*element);
    want->SetFlags(params.flags);
    want->SetUri(params.uri);
    want->SetAction(params.action);
    want->SetType(params.wantType);

    auto wantParams = OHOS::AAFwk::WantParamWrapper::ParseWantParamsWithBrackets(params.parameters);

    // Inject fds to WantParams
    InjectFdsArrayToWantParams(params.fds, wantParams);

    want->SetParams(wantParams);

    return want;
}

void FFICJWantDelete(WantHandle want)
{
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::DEFAULT, "null want");
        return;
    }
    auto actualWant = reinterpret_cast<Want*>(want);
    delete actualWant;
    actualWant = nullptr;
}

CJWantParams* FFICJWantGetWantInfo(WantHandle want)
{
    CJWantParams* buffer = static_cast<CJWantParams*>(malloc(sizeof(CJWantParams)));
    if (buffer == nullptr) {
        TAG_LOGE(AAFwkTag::DEFAULT, "null buffer");
        return nullptr;
    }

    auto actualWant = reinterpret_cast<Want*>(want);
    auto element = actualWant->GetElement();
    ElementNameHandle elementName = new ElementName(
        element.GetDeviceID(), element.GetBundleName(), element.GetAbilityName(), element.GetModuleName());
    if (elementName == nullptr) {
        free(buffer);
        TAG_LOGE(AAFwkTag::DEFAULT, "element name null");
        return nullptr;
    }
    buffer->elementName = elementName;
    buffer->flags = actualWant->GetFlags();
    buffer->uri = CreateCStringFromString(actualWant->GetUriString());
    buffer->action = CreateCStringFromString(actualWant->GetAction());
    buffer->wantType = CreateCStringFromString(actualWant->GetType());
    buffer->entities = const_cast<std::vector<std::string>*>(&(actualWant->GetEntities())); // reference vector<String>
    buffer->parameters = CreateCStringFromString(OHOS::AAFwk::WantParamWrapper(actualWant->GetParams()).ToString());
    return buffer;
}

void FFICJWantParamsDelete(CJWantParams* params)
{
    if (params == nullptr) {
        TAG_LOGE(AAFwkTag::DEFAULT, "argc null");
        return;
    }
    auto actualElementName = reinterpret_cast<ElementName*>(params->elementName);
    delete actualElementName;
    actualElementName = nullptr;

    free(static_cast<void*>(params->uri));
    free(static_cast<void*>(params->action));
    free(static_cast<void*>(params->wantType));
    free(static_cast<void*>(params->parameters));
    // Entities are reference, do not free.
    free(static_cast<void*>(params));
}

void FFICJWantAddEntity(WantHandle want, const char* entity)
{
    if (want == nullptr || entity == nullptr) {
        TAG_LOGE(AAFwkTag::DEFAULT, "Want or entity null");
        return;
    }
    auto actualWant = reinterpret_cast<Want*>(want);
    actualWant->AddEntity(entity);
}

WantHandle FFICJWantParseUri(const char* uri)
{
    if (uri == nullptr) {
        TAG_LOGE(AAFwkTag::DEFAULT, "Uri null");
        return nullptr;
    }
    return Want::ParseUri(uri);
}

CJWantParamsV2* FFICJWantGetWantInfoV2(WantHandle want)
{
    CJWantParamsV2* buffer = static_cast<CJWantParamsV2*>(malloc(sizeof(CJWantParamsV2)));
    if (buffer == nullptr) {
        TAG_LOGE(AAFwkTag::DEFAULT, "null buffer");
        return nullptr;
    }

    auto actualWant = reinterpret_cast<Want*>(want);
    auto element = actualWant->GetElement();
    ElementNameHandle elementName = new ElementName(
        element.GetDeviceID(), element.GetBundleName(), element.GetAbilityName(), element.GetModuleName());
    if (elementName == nullptr) {
        free(buffer);
        TAG_LOGE(AAFwkTag::DEFAULT, "element name null");
        return nullptr;
    }
    buffer->elementName = elementName;
    buffer->flags = actualWant->GetFlags();
    buffer->uri = CreateCStringFromString(actualWant->GetUriString());
    buffer->action = CreateCStringFromString(actualWant->GetAction());
    buffer->wantType = CreateCStringFromString(actualWant->GetType());
    buffer->entities = const_cast<std::vector<std::string>*>(&(actualWant->GetEntities()));
    buffer->parameters = CreateCStringFromString(OHOS::AAFwk::WantParamWrapper(actualWant->GetParams()).ToString());
    buffer->fds = ExtractFdsArrayFromWantParams(actualWant->GetParams());
    return buffer;
}

void FFICJWantParamsDeleteV2(CJWantParamsV2* params)
{
    if (params == nullptr) {
        TAG_LOGE(AAFwkTag::DEFAULT, "argc null");
        return;
    }
    auto actualElementName = reinterpret_cast<ElementName*>(params->elementName);
    delete actualElementName;
    actualElementName = nullptr;

    free(static_cast<void*>(params->uri));
    free(static_cast<void*>(params->action));
    free(static_cast<void*>(params->wantType));
    free(static_cast<void*>(params->parameters));
    
    // Free fds array
    if (params->fds.head != nullptr) {
        for (int64_t i = 0; i < params->fds.size; i++) {
            free(static_cast<void*>(params->fds.head[i].key));
        }
        free(static_cast<void*>(params->fds.head));
    }
    
    free(static_cast<void*>(params));
}
