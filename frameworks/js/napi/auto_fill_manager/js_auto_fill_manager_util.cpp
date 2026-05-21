/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "js_auto_fill_manager_util.h"

#include "hilog_tag_wrapper.h"
#include "napi_common_util.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char* LEFT = "left";
constexpr const char* TOP = "top";
constexpr const char* WIDTH = "width";
constexpr const char* HEIGHT = "height";
constexpr const char* ID = "id";
constexpr const char* AUTO_FILL_TYPE = "autoFillType";
constexpr const char* VALUE = "value";
constexpr const char* PLACEHOLDER = "placeholder";
constexpr const char* RECT = "rect";
constexpr const char* IS_FOCUS = "isFocus";
constexpr const char* BUNDLE_NAME = "bundleName";
constexpr const char* PAGE_URL = "pageUrl";
constexpr const char* PAGE_NODE_INFOS = "pageNodeInfos";
constexpr const char* PAGE_RECT = "pageRect";
constexpr const char* ERR_CODE = "errCode";
constexpr const char* VIEW_DATA = "viewData";
constexpr const char* TYPE = "type";
constexpr const char* TRIGGER_TYPE = "triggerType";
}
using namespace AppExecFwk;

napi_value WrapAutoFillRect(napi_env env, const AbilityBase::Rect &rect)
{
    HandleEscape handleEscape(env);
    napi_value jsObject = nullptr;
    NAPI_CALL(env, napi_create_object(env, &jsObject));

    if (!SetPropertyValueByPropertyName(env, jsObject, LEFT, WrapDoubleToJS(env, rect.left))) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "set left failed");
        return CreateJsUndefined(env);
    }

    if (!SetPropertyValueByPropertyName(env, jsObject, TOP, WrapDoubleToJS(env, rect.top))) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "set top failed");
        return CreateJsUndefined(env);
    }

    if (!SetPropertyValueByPropertyName(env, jsObject, WIDTH, WrapDoubleToJS(env, rect.width))) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "set width failed");
        return CreateJsUndefined(env);
    }

    if (!SetPropertyValueByPropertyName(env, jsObject, HEIGHT, WrapDoubleToJS(env, rect.height))) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "set height failed");
        return CreateJsUndefined(env);
    }

    return handleEscape.Escape(jsObject);
}

napi_value WrapPageNodeInfo(napi_env env, const AbilityBase::PageNodeInfo &pageNodeInfo)
{
    HandleEscape handleEscape(env);
    napi_value jsObject = nullptr;
    NAPI_CALL(env, napi_create_object(env, &jsObject));

    if (!SetPropertyValueByPropertyName(env, jsObject, ID, WrapInt32ToJS(env, pageNodeInfo.id))) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "set id failed");
        return CreateJsUndefined(env);
    }

    if (!SetPropertyValueByPropertyName(env,
        jsObject, AUTO_FILL_TYPE, WrapInt32ToJS(env, static_cast<int32_t>(pageNodeInfo.autoFillType)))) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "set autoFillType failed");
        return CreateJsUndefined(env);
    }

    if (!SetPropertyValueByPropertyName(env, jsObject, VALUE, WrapStringToJS(env, pageNodeInfo.value))) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "set value failed");
        return CreateJsUndefined(env);
    }

    if (!SetPropertyValueByPropertyName(env, jsObject, PLACEHOLDER, WrapStringToJS(env, pageNodeInfo.placeholder))) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "set placeholder failed");
        return CreateJsUndefined(env);
    }

    if (!SetPropertyValueByPropertyName(env, jsObject, RECT, WrapAutoFillRect(env, pageNodeInfo.rect))) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "set rect failed");
        return CreateJsUndefined(env);
    }

    if (!SetPropertyValueByPropertyName(env, jsObject, IS_FOCUS, WrapBoolToJS(env, pageNodeInfo.isFocus))) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "set isFocus failed");
        return CreateJsUndefined(env);
    }

    return handleEscape.Escape(jsObject);
}

napi_value WrapViewData(const napi_env env, const AbilityBase::ViewData &viewData)
{
    HandleEscape handleEscape(env);
    napi_value jsObject = nullptr;
    NAPI_CALL(env, napi_create_object(env, &jsObject));

    if (!SetPropertyValueByPropertyName(env, jsObject, BUNDLE_NAME, WrapStringToJS(env, viewData.bundleName))) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "set bundleName failed");
        return CreateJsUndefined(env);
    }

    if (!SetPropertyValueByPropertyName(env, jsObject, PAGE_URL, WrapStringToJS(env, viewData.pageUrl))) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "set pageUrl failed");
        return CreateJsUndefined(env);
    }

    napi_value jsArray = nullptr;
    NAPI_CALL(env, napi_create_array(env, &jsArray));
    uint32_t index = 0;
    for (const auto &element : viewData.nodes) {
        napi_value jsSubValue = WrapPageNodeInfo(env, element);
        if (jsSubValue != nullptr && napi_set_element(env, jsArray, index, jsSubValue) == napi_ok) {
            ++index;
        } else {
            TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Set element fail");
        }
    }
    if (!SetPropertyValueByPropertyName(env, jsObject, PAGE_NODE_INFOS, jsArray)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "set pageNodeInfos failed");
        return CreateJsUndefined(env);
    }

    if (!SetPropertyValueByPropertyName(env, jsObject, PAGE_RECT, WrapAutoFillRect(env, viewData.pageRect))) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "set pageRect failed");
        return CreateJsUndefined(env);
    }
    return handleEscape.Escape(jsObject);
}

napi_value WrapFillFailureResult(napi_env env, int32_t errCode)
{
    HandleEscape handleEscape(env);
    napi_value jsObject = nullptr;
    NAPI_CALL(env, napi_create_object(env, &jsObject));

    if (!SetPropertyValueByPropertyName(env, jsObject, ERR_CODE, WrapInt32ToJS(env, errCode))) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "set errCode failed");
        return CreateJsUndefined(env);
    }
    return handleEscape.Escape(jsObject);
}

bool UnwrapAutoFillRect(napi_env env, napi_value jsValue, AbilityBase::Rect &rect, std::string &errorMsg)
{
    if (!IsTypeForNapiValue(env, jsValue, napi_object)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "parameter error");
        errorMsg = "Parameter error. The type of jsValue must be AutoFillRect";
        return false;
    }

    double doubleValue = 0;
    if (!UnwrapDoubleByPropertyName(env, jsValue, LEFT, doubleValue)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "parameter error");
        errorMsg = "Parameter error. The type of rect.left must be number";
        return false;
    }
    rect.left = static_cast<float>(doubleValue);

    if (!UnwrapDoubleByPropertyName(env, jsValue, TOP, doubleValue)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "parameter error");
        errorMsg = "Parameter error. The type of rect.top must be number";
        return false;
    }
    rect.top = static_cast<float>(doubleValue);

    if (!UnwrapDoubleByPropertyName(env, jsValue, WIDTH, doubleValue)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "parameter error");
        errorMsg = "Parameter error. The type of rect.width must be number";
        return false;
    }
    rect.width = static_cast<float>(doubleValue);

    if (!UnwrapDoubleByPropertyName(env, jsValue, HEIGHT, doubleValue)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "parameter error");
        errorMsg = "Parameter error. The type of rect.height must be number";
        return false;
    }
    rect.height = static_cast<float>(doubleValue);
    return true;
}

bool UnwrapPageNodeInfo(napi_env env, napi_value jsValue, AbilityBase::PageNodeInfo &pageNodeInfo,
    std::string &errorMsg)
{
    if (!IsTypeForNapiValue(env, jsValue, napi_object)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "parameter error");
        errorMsg = "Parameter error. The type of jsValue must be PageNodeInfo";
        return false;
    }

    if (!UnwrapInt32ByPropertyName(env, jsValue, ID, pageNodeInfo.id)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "parameter error");
        errorMsg = "Parameter error. The type of pageNodeInfo.id must be number";
        return false;
    }

    int32_t int32Value = 0;
    if (!UnwrapInt32ByPropertyName(env, jsValue, AUTO_FILL_TYPE, int32Value)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "parameter error");
        errorMsg = "Parameter error. The type of pageNodeInfo.autoFillType must be AutoFillType";
        return false;
    }
    pageNodeInfo.autoFillType = static_cast<AbilityBase::AutoFillType>(int32Value);

    if (!UnwrapStringByPropertyName(env, jsValue, VALUE, pageNodeInfo.value)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "parameter error");
        errorMsg = "Parameter error. The type of pageNodeInfo.value must be string";
        return false;
    }

    if (IsExistsByPropertyName(env, jsValue, PLACEHOLDER) &&
        !UnwrapStringByPropertyName(env, jsValue, PLACEHOLDER, pageNodeInfo.placeholder)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "parameter error");
        errorMsg = "Parameter error. The type of pageNodeInfo.placeholder must be string";
        return false;
    }

    napi_value jsRect = GetPropertyValueByPropertyName(env, jsValue, RECT, napi_object);
    if (jsRect != nullptr && !UnwrapAutoFillRect(env, jsRect, pageNodeInfo.rect, errorMsg)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "UnwrapAutoFillRect error");
        return false;
    }

    if (!UnwrapBooleanByPropertyName(env, jsValue, IS_FOCUS, pageNodeInfo.isFocus)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "parameter error");
        errorMsg = "Parameter error. The type of pageNodeInfo.isFocus must be boolean";
        return false;
    }
    return true;
}

bool UnwrapViewData(napi_env env, napi_value jsValue, AbilityBase::ViewData &viewData, std::string &errorMsg)
{
    if (!IsTypeForNapiValue(env, jsValue, napi_object)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "parameter error");
        errorMsg = "Parameter error. The type of jsValue must be ViewData";
        return false;
    }

    if (!UnwrapStringByPropertyName(env, jsValue, BUNDLE_NAME, viewData.bundleName)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "parameter error");
        errorMsg = "Parameter error. The type of viewData.bundleName must be string";
        return false;
    }

    if (!UnwrapStringByPropertyName(env, jsValue, PAGE_URL, viewData.pageUrl)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "parameter error");
        errorMsg = "Parameter error. The type of viewData.pageUrl must be string";
        return false;
    }

    napi_value jsPageNodeInfos = GetPropertyValueByPropertyName(env, jsValue, PAGE_NODE_INFOS, napi_object);
    uint32_t arraySize = 0;
    if (jsPageNodeInfos == nullptr || !IsArrayForNapiValue(env, jsPageNodeInfos, arraySize)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "parameter error");
        errorMsg = "Parameter error. The type of viewData.pageNodeInfos must be array";
        return false;
    }
    viewData.nodes.clear();
    for (uint32_t i = 0; i < arraySize; ++i) {
        napi_value jsPageNodeInfo = nullptr;
        if (napi_get_element(env, jsPageNodeInfos, i, &jsPageNodeInfo) != napi_ok) {
            TAG_LOGE(AAFwkTag::AUTOFILLMGR, "napi_get_element failed");
            return false;
        }
        AbilityBase::PageNodeInfo pageNodeInfo;
        if (!UnwrapPageNodeInfo(env, jsPageNodeInfo, pageNodeInfo, errorMsg)) {
            TAG_LOGE(AAFwkTag::AUTOFILLMGR, "UnwrapPageNodeInfo failed");
            return false;
        }
        viewData.nodes.emplace_back(pageNodeInfo);
    }

    napi_value jsPageRect = GetPropertyValueByPropertyName(env, jsValue, PAGE_RECT, napi_object);
    if (jsPageRect != nullptr && !UnwrapAutoFillRect(env, jsPageRect, viewData.pageRect, errorMsg)) {
        return false;
    }
    return true;
}

bool UnwrapSaveRequest(napi_env env, napi_value jsValue, AutoFill::AutoFillRequest &request, std::string &errorMsg)
{
    if (!IsTypeForNapiValue(env, jsValue, napi_object)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "parameter error");
        errorMsg = "Parameter error. The type of jsValue must be SaveRequest";
        return false;
    }

    napi_value jsViewData = GetPropertyValueByPropertyName(env, jsValue, VIEW_DATA, napi_object);
    if (jsViewData != nullptr && !UnwrapViewData(env, jsViewData, request.viewData, errorMsg)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "UnwrapViewData failed");
        return false;
    }
    return true;
}

bool UnwrapFillRequest(napi_env env, napi_value jsValue, AutoFill::AutoFillRequest &request, std::string &errorMsg)
{
    if (!IsTypeForNapiValue(env, jsValue, napi_object)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "parameter error");
        errorMsg = "Parameter error. The type of jsValue must be FillRequest";
        return false;
    }

    int32_t int32Value;
    if (!UnwrapInt32ByPropertyName(env, jsValue, TYPE, int32Value)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "parameter error");
        errorMsg = "Parameter error. The type of request.autoFillType must be AutoFillType";
        return false;
    }
    request.autoFillType = static_cast<AbilityBase::AutoFillType>(int32Value);

    napi_value jsViewData = GetPropertyValueByPropertyName(env, jsValue, VIEW_DATA, napi_object);
    if (jsViewData != nullptr && !UnwrapViewData(env, jsViewData, request.viewData, errorMsg)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "UnwrapViewData failed");
        return false;
    }

    if (IsExistsByPropertyName(env, jsValue, TRIGGER_TYPE)) {
        if (!UnwrapInt32ByPropertyName(env, jsValue, TRIGGER_TYPE, int32Value)) {
            TAG_LOGE(AAFwkTag::AUTOFILLMGR, "parameter error");
            errorMsg = "Parameter error. The type of request.autoFillTriggerType must be AutoFillTriggerType";
            return false;
        }
        request.autoFillTriggerType = static_cast<AutoFill::AutoFillTriggerType>(int32Value);
    }
    return true;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
