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

#include "ets_auto_fill_manager_util.h"

#include "ani_common_util.h"
#include "ani_enum_convert.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AutoFillManagerEts {
namespace {
constexpr const char* AUTO_FILL_RECT_IMPL_CLASS_NAME = "application.AutoFillRect.AutoFillRectImpl";
constexpr const char* PAGE_NODE_INFO_IMPL_CLASS_NAME = "application.PageNodeInfo.PageNodeInfoImpl";
constexpr const char* VIEW_DATA_IMPL_CLASS_NAME = "application.ViewData.ViewDataImpl";
constexpr const char* FILL_FAILURE_RESULT_INNER_CLASS_NAME = "application.AutoFillRequest.FillFailureResultInner";
constexpr const char* AUTO_FILL_TYPE_ENUM_NAME = "application.AutoFillType.AutoFillType";
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

ani_object WrapAutoFillRect(ani_env *env, const AbilityBase::Rect &rect)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "env null");
        return nullptr;
    }

    ani_object object = nullptr;
    if (!CreateObjectByClassName(env, AUTO_FILL_RECT_IMPL_CLASS_NAME, object)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "fail to create AutoFillRect object");
        return nullptr;
    }

    if (!SetDoublePropertyValue(env, object, LEFT, rect.left)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "set left failed");
        return nullptr;
    }

    if (!SetDoublePropertyValue(env, object, TOP, rect.top)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "set top failed");
        return nullptr;
    }

    if (!SetDoublePropertyValue(env, object, WIDTH, rect.width)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "set width failed");
        return nullptr;
    }

    if (!SetDoublePropertyValue(env, object, HEIGHT, rect.height)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "set height failed");
        return nullptr;
    }
    return object;
}

ani_object WrapPageNodeInfo(ani_env *env, const AbilityBase::PageNodeInfo &pageNodeInfo)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "env null");
        return nullptr;
    }

    ani_object object = nullptr;
    if (!CreateObjectByClassName(env, PAGE_NODE_INFO_IMPL_CLASS_NAME, object)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "fail to create PageNodeInfo object");
        return nullptr;
    }

    if (!SetIntPropertyValue(env, object, ID, pageNodeInfo.id)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "set id failed");
        return nullptr;
    }

    ani_enum_item aniAutoFillType = nullptr;
    if (!AAFwk::AniEnumConvertUtil::EnumConvert_NativeToEts(env,
        AUTO_FILL_TYPE_ENUM_NAME, pageNodeInfo.autoFillType, aniAutoFillType) ||
        !SetRefProperty(env, object, AUTO_FILL_TYPE, aniAutoFillType)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "set autoFillType failed");
        return nullptr;
    }

    if (!SetRefProperty(env, object, VALUE, GetAniString(env, pageNodeInfo.value))) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "set value failed");
        return nullptr;
    }

    if (!SetRefProperty(env, object, PLACEHOLDER, GetAniString(env, pageNodeInfo.placeholder))) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "set placeholder failed");
        return nullptr;
    }

    if (!SetRefProperty(env, object, RECT, WrapAutoFillRect(env, pageNodeInfo.rect))) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "set rect failed");
        return nullptr;
    }

    ani_status status = ANI_ERROR;
    if ((status = env->Object_SetPropertyByName_Boolean(object, IS_FOCUS, pageNodeInfo.isFocus)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Object_SetPropertyByName_Boolean failed: status: %{public}d", status);
        return nullptr;
    }
    return object;
}

ani_object WrapViewData(ani_env *env, const AbilityBase::ViewData &viewData)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "env null");
        return nullptr;
    }

    ani_object object = nullptr;
    if (!CreateObjectByClassName(env, VIEW_DATA_IMPL_CLASS_NAME, object)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "fail to create ViewData object");
        return nullptr;
    }

    if (!SetRefProperty(env, object, BUNDLE_NAME, GetAniString(env, viewData.bundleName))) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "set bundleName failed");
        return nullptr;
    }

    if (!SetRefProperty(env, object, PAGE_URL, GetAniString(env, viewData.pageUrl))) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "set pageUrl failed");
        return nullptr;
    }

    ani_object aniNodes = nullptr;
    if (!CreateArrayObject(env, aniNodes, viewData.nodes.size())) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "fail to create array object");
        return nullptr;
    }
    ani_size index = 0;
    ani_status status = ANI_ERROR;
    for (const auto &item : viewData.nodes) {
        status = env->Object_CallMethodByName_Void(aniNodes, "$_set", "iY:", index, WrapPageNodeInfo(env, item));
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Object_CallMethodByName_Void failed: %{public}d", status);
            return nullptr;
        }
        ++index;
    }

    if (!SetRefProperty(env, object, PAGE_NODE_INFOS, aniNodes)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "set pageNodeInfos failed");
        return nullptr;
    }

    if (!SetRefProperty(env, object, PAGE_RECT, WrapAutoFillRect(env, viewData.pageRect))) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "set pageRect failed");
        return nullptr;
    }
    return object;
}

ani_object WrapFillFailureResult(ani_env *env, int32_t errCode)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "env null");
        return nullptr;
    }

    ani_object object = nullptr;
    if (!CreateObjectByClassName(env, FILL_FAILURE_RESULT_INNER_CLASS_NAME, object)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "fail to create FillFailureResult object");
        return nullptr;
    }

    if (!SetIntPropertyValue(env, object, ERR_CODE, errCode)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "set errCode failed");
        return nullptr;
    }

    return object;
}

bool UnwrapAutoFillRect(ani_env *env, ani_object object, AbilityBase::Rect &rect, std::string &errorMsg)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "env null");
        return false;
    }

    double doubleValue;
    if (!GetDoublePropertyValue(env, object, LEFT, doubleValue)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "parameter error");
        errorMsg = "Parameter error. The type of rect.left must be double";
        return false;
    }
    rect.left = static_cast<float>(doubleValue);

    if (!GetDoublePropertyValue(env, object, TOP, doubleValue)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "parameter error");
        errorMsg = "Parameter error. The type of rect.top must be double";
        return false;
    }
    rect.top = static_cast<float>(doubleValue);

    if (!GetDoublePropertyValue(env, object, WIDTH, doubleValue)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "parameter error");
        errorMsg = "Parameter error. The type of rect.width must be double";
        return false;
    }
    rect.width = static_cast<float>(doubleValue);

    if (!GetDoublePropertyValue(env, object, HEIGHT, doubleValue)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "parameter error");
        errorMsg = "Parameter error. The type of rect.height must be double";
        return false;
    }
    rect.height = static_cast<float>(doubleValue);
    return true;
}

bool UnwrapPageNodeInfo(ani_env *env, ani_object object, AbilityBase::PageNodeInfo &pageNodeInfo,
    std::string &errorMsg)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "env null");
        return false;
    }

    if (!GetIntPropertyValue(env, object, ID, pageNodeInfo.id)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "parameter error");
        errorMsg = "Parameter error. The type of pageNodeInfo.id must be int";
        return false;
    }

    ani_ref aniAutoFillType = nullptr;
    if (!GetRefProperty(env, object, AUTO_FILL_TYPE, aniAutoFillType) || aniAutoFillType == nullptr ||
        !AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative(env,
            reinterpret_cast<ani_enum_item>(aniAutoFillType), pageNodeInfo.autoFillType)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "parameter error");
        errorMsg = "Parameter error. The type of pageNodeInfo.autoFillType must be AutoFillType";
        return false;
    }

    if (!GetStringProperty(env, object, VALUE, pageNodeInfo.value)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "parameter error");
        errorMsg = "Parameter error. The type of pageNodeInfo.value must be string";
        return false;
    }

    if (IsExistsProperty(env, object, PLACEHOLDER) &&
        !GetStringProperty(env, object, PLACEHOLDER, pageNodeInfo.placeholder)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "parameter error");
        errorMsg = "Parameter error. The type of pageNodeInfo.placeholder must be string";
        return false;
    }

    ani_ref aniRect = nullptr;
    if (!GetRefProperty(env, object, RECT, aniRect) || aniRect == nullptr ||
        !UnwrapAutoFillRect(env, reinterpret_cast<ani_object>(aniRect), pageNodeInfo.rect, errorMsg)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "UnwrapAutoFillRect failed");
        return false;
    }

    ani_boolean aniIsFocus = ANI_FALSE;
    if (env->Object_GetPropertyByName_Boolean(object, IS_FOCUS, &aniIsFocus) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "parameter error");
        errorMsg = "Parameter error. The type of pageNodeInfo.isFocus must be boolean";
        return false;
    }
    pageNodeInfo.isFocus = static_cast<bool>(aniIsFocus);
    return true;
}

bool UnwrapViewData(ani_env *env, ani_object object, AbilityBase::ViewData &viewData, std::string &errorMsg)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "env null");
        return false;
    }

    if (!GetStringProperty(env, object, BUNDLE_NAME, viewData.bundleName)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "parameter error");
        errorMsg = "Parameter error. The type of viewData.bundleName must be string";
        return false;
    }

    if (!GetStringProperty(env, object, PAGE_URL, viewData.pageUrl)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "parameter error");
        errorMsg = "Parameter error. The type of viewData.pageUrl must be string";
        return false;
    }

    ani_ref aniNodes = nullptr;
    if (!GetRefProperty(env, object, PAGE_NODE_INFOS, aniNodes) || aniNodes == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "parameter error");
        errorMsg = "Parameter error. The type of viewData.pageNodeInfos must be array";
        return false;
    }
    ani_int length = 0;
    ani_status status = env->Object_GetPropertyByName_Int(reinterpret_cast<ani_object>(aniNodes), "length", &length);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Object_GetPropertyByName_Int failed: status: %{public}d", status);
        return false;
    }
    viewData.nodes.clear();
    for (int i = 0; i < length; ++i) {
        ani_ref aniPageNodeInfo = nullptr;
        if ((status = env->Object_CallMethodByName_Ref(reinterpret_cast<ani_object>(aniNodes),
            "$_get", "i:Y", &aniPageNodeInfo, (ani_int)i)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Object_CallMethodByName_Ref failed: status: %{public}d", status);
            return false;
        }
        AbilityBase::PageNodeInfo pageNodeInfo;
        if (!UnwrapPageNodeInfo(env, reinterpret_cast<ani_object>(aniPageNodeInfo), pageNodeInfo, errorMsg)) {
            TAG_LOGE(AAFwkTag::AUTOFILLMGR, "UnwrapPageNodeInfo failed");
            return false;
        }
        viewData.nodes.emplace_back(pageNodeInfo);
    }

    ani_ref aniPageRect = nullptr;
    if (!GetRefProperty(env, object, PAGE_RECT, aniPageRect) || aniPageRect == nullptr ||
        !UnwrapAutoFillRect(env, reinterpret_cast<ani_object>(aniPageRect), viewData.pageRect, errorMsg)) {
        return false;
    }
    return true;
}

bool UnwrapSaveRequest(ani_env *env,
    ani_object object, AbilityRuntime::AutoFill::AutoFillRequest &request, std::string &errorMsg)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "env null");
        return false;
    }

    ani_ref aniViewData = nullptr;
    if (!GetRefProperty(env, object, VIEW_DATA, aniViewData) || aniViewData == nullptr ||
        !UnwrapViewData(env, reinterpret_cast<ani_object>(aniViewData), request.viewData, errorMsg)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "UnwrapViewData failed");
        return false;
    }
    return true;
}

bool UnwrapFillRequest(ani_env *env,
    ani_object object, AbilityRuntime::AutoFill::AutoFillRequest &request, std::string &errorMsg)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "env null");
        return false;
    }

    ani_ref aniAutoFillType = nullptr;
    if (!GetRefProperty(env, object, TYPE, aniAutoFillType) || aniAutoFillType == nullptr ||
        !AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative(env,
            reinterpret_cast<ani_enum_item>(aniAutoFillType), request.autoFillType)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "parameter error");
        errorMsg = "Parameter error. The type of request.autoFillType must be AutoFillType";
        return false;
    }

    ani_ref aniViewData = nullptr;
    if (!GetRefProperty(env, object, VIEW_DATA, aniViewData) || aniViewData == nullptr ||
        !UnwrapViewData(env, reinterpret_cast<ani_object>(aniViewData), request.viewData, errorMsg)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "UnwrapViewData failed");
        return false;
    }

    ani_ref aniAutoFillTriggerType = nullptr;
    if (IsExistsProperty(env, object, TRIGGER_TYPE) &&
        (!GetRefProperty(env, object, TRIGGER_TYPE, aniAutoFillTriggerType) || aniAutoFillTriggerType == nullptr ||
        !AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative(env,
            reinterpret_cast<ani_enum_item>(aniAutoFillTriggerType), request.autoFillTriggerType))) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "parameter error");
        errorMsg = "Parameter error. The type of request.autoFillTriggerType must be AutoFillTriggerType";
        return false;
    }
    return true;
}
}  // namespace AutoFillManagerEts
}  // namespace OHOS
