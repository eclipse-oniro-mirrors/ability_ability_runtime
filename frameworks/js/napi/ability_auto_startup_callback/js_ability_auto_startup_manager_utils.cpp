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

#include "js_ability_auto_startup_manager_utils.h"

#include "global_constant.h"
#include "hilog_tag_wrapper.h"
#include "napi_common_util.h"

namespace OHOS {
namespace AbilityRuntime {
bool UnwrapAutoStartupInfo(napi_env env, napi_value param, AutoStartupInfo &info)
{
    if (!IsNormalObject(env, param)) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "invalid param");
        return false;
    }

    if (!AppExecFwk::UnwrapStringByPropertyName(env, param, "bundleName", info.bundleName)) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "convert bundleName failed");
        return false;
    }

    if (!AppExecFwk::UnwrapStringByPropertyName(env, param, "abilityName", info.abilityName)) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "convert abilityName failed");
        return false;
    }

    if (AppExecFwk::IsExistsByPropertyName(env, param, "appCloneIndex")) {
        if (!AppExecFwk::UnwrapInt32ByPropertyName(env, param, "appCloneIndex", info.appCloneIndex)) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "convert appCloneIndex failed");
            return false;
        }
    }
    
    AppExecFwk::UnwrapStringByPropertyName(env, param, "moduleName", info.moduleName);
    return true;
}

bool IsNormalObject(napi_env env, napi_value value)
{
    if (value == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null value");
        return false;
    }
    napi_valuetype type;
    napi_typeof(env, value, &type);
    if (type == napi_undefined || type == napi_null) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "invalid type");
        return false;
    }
    if (type != napi_object) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "invalid type");
        return false;
    }
    return true;
}

napi_value CreateJsAutoStartupInfoArray(napi_env env, const std::vector<AutoStartupInfo> &infoList)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called");
    napi_value arrayObj = nullptr;
    napi_create_array(env, &arrayObj);
    for (size_t i = 0; i < infoList.size(); ++i) {
        auto object = CreateJsAutoStartupInfo(env, infoList.at(i));
        if (object == nullptr) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null obj");
            return nullptr;
        }

        if (napi_set_element(env, arrayObj, i, object) != napi_ok) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "insert object failed");
            return nullptr;
        }
    }

    return arrayObj;
}

napi_value CreateJsAutoStartupInfo(napi_env env, const AutoStartupInfo &info)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called");
    napi_value object = AppExecFwk::CreateJSObject(env);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null object");
        return nullptr;
    }

    napi_value bundleName = AppExecFwk::WrapStringToJS(env, info.bundleName);
    if (bundleName == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null bundleName");
        return nullptr;
    }

    napi_value abilityName = AppExecFwk::WrapStringToJS(env, info.abilityName);
    if (abilityName == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null abilityName");
        return nullptr;
    }

    napi_value moduleName = AppExecFwk::WrapStringToJS(env, info.moduleName);
    if (moduleName == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null moduleName");
        return nullptr;
    }

    napi_value abilityTypeName = AppExecFwk::WrapStringToJS(env, info.abilityTypeName);
    if (abilityTypeName == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null abilityTypeName");
        return nullptr;
    }

    if (!(AppExecFwk::SetPropertyValueByPropertyName(env, object, "bundleName", bundleName) &&
        AppExecFwk::SetPropertyValueByPropertyName(env, object, "abilityName", abilityName) &&
        AppExecFwk::SetPropertyValueByPropertyName(env, object, "moduleName", moduleName) &&
        AppExecFwk::SetPropertyValueByPropertyName(env, object, "abilityTypeName", abilityTypeName))) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "create js AutoStartupInfo failed");
        return nullptr;
    }
    if (info.appCloneIndex >= 0 && info.appCloneIndex < GlobalConstant::MAX_APP_CLONE_INDEX) {
        napi_value appCloneIndex = AppExecFwk::WrapInt32ToJS(env, info.appCloneIndex);
        if (appCloneIndex == nullptr) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null appCloneIndex");
            return nullptr;
        }
        if (!AppExecFwk::SetPropertyValueByPropertyName(env, object, "appCloneIndex", appCloneIndex)) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "create js AutoStartupInfo failed");
            return nullptr;
        }
    }
    return object;
}
} // namespace AbilityRuntime
} // namespace OHOS