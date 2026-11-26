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

#include "js_mission_info_utils.h"

#include "hilog_tag_wrapper.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "bool_wrapper.h"
#include "byte_wrapper.h"
#include "double_wrapper.h"
#include "float_wrapper.h"
#include "int_wrapper.h"
#include "long_wrapper.h"
#include "short_wrapper.h"
#include "string_wrapper.h"
#include "zchar_wrapper.h"
#include "array_wrapper.h"
#include "want_params_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
napi_value CreateJsMissionInfo(napi_env env, const AAFwk::MissionInfo &missionInfo)
{
    HandleEscape handleEscape(env);
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);

    napi_set_named_property(env, objValue, "missionId", CreateJsValue(env, missionInfo.id));
    napi_set_named_property(env, objValue, "runningState", CreateJsValue(env, missionInfo.runningState));
    napi_set_named_property(env, objValue, "lockedState", CreateJsValue(env, missionInfo.lockedState));
    napi_set_named_property(env, objValue, "continuable", CreateJsValue(env, missionInfo.continuable));
    napi_set_named_property(env, objValue, "timestamp", CreateJsValue(env, missionInfo.time));
    napi_set_named_property(env, objValue, "want", CreateJsWant(env, missionInfo.want));
    napi_set_named_property(env, objValue, "label", CreateJsValue(env, missionInfo.label));
    napi_set_named_property(env, objValue, "iconPath", CreateJsValue(env, missionInfo.iconPath));
    return handleEscape.Escape(objValue);
}

napi_value CreateJsWant(napi_env env, const AAFwk::Want &want)
{
    HandleEscape handleEscape(env);
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);

    napi_set_named_property(env, objValue, "deviceId", CreateJsValue(env, want.GetElement().GetDeviceID()));
    napi_set_named_property(env, objValue, "bundleName", CreateJsValue(env, want.GetElement().GetBundleName()));
    napi_set_named_property(env, objValue, "abilityName", CreateJsValue(env, want.GetElement().GetAbilityName()));
    napi_set_named_property(env, objValue, "uri", CreateJsValue(env, want.GetUriString()));
    napi_set_named_property(env, objValue, "type", CreateJsValue(env, want.GetType()));
    napi_set_named_property(env, objValue, "flags", CreateJsValue(env, want.GetFlags()));
    napi_set_named_property(env, objValue, "action", CreateJsValue(env, want.GetAction()));
    napi_set_named_property(env, objValue, "parameters", CreateJsWantParams(env, want.GetParams()));
    napi_set_named_property(env, objValue, "entities", CreateNativeArray(env, want.GetEntities()));
    return handleEscape.Escape(objValue);
}

napi_value CreateJsWantParams(napi_env env, const AAFwk::WantParams &wantParams)
{
    HandleEscape handleEscape(env);
    napi_value object = nullptr;
    napi_create_object(env, &object);
    const std::map<std::string, sptr<AAFwk::IInterface>> paramList = wantParams.GetParams();
    for (auto iter = paramList.begin(); iter != paramList.end(); iter++) {
        if (AAFwk::IString::Query(iter->second) != nullptr) {
            InnerWrapJsWantParams<AAFwk::IString, AAFwk::String, std::string>(
                env, object, iter->first, wantParams);
        } else if (AAFwk::IBoolean::Query(iter->second) != nullptr) {
            InnerWrapJsWantParams<AAFwk::IBoolean, AAFwk::Boolean, bool>(
                env, object, iter->first, wantParams);
        } else if (AAFwk::IShort::Query(iter->second) != nullptr) {
            InnerWrapJsWantParams<AAFwk::IShort, AAFwk::Short, short>(
                env, object, iter->first, wantParams);
        } else if (AAFwk::IInteger::Query(iter->second) != nullptr) {
            InnerWrapJsWantParams<AAFwk::IInteger, AAFwk::Integer, int>(
                env, object, iter->first, wantParams);
        } else if (AAFwk::ILong::Query(iter->second) != nullptr) {
            InnerWrapJsWantParams<AAFwk::ILong, AAFwk::Long, int64_t>(
                env, object, iter->first, wantParams);
        } else if (AAFwk::IFloat::Query(iter->second) != nullptr) {
            InnerWrapJsWantParams<AAFwk::IFloat, AAFwk::Float, float>(
                env, object, iter->first, wantParams);
        } else if (AAFwk::IDouble::Query(iter->second) != nullptr) {
            InnerWrapJsWantParams<AAFwk::IDouble, AAFwk::Double, double>(
                env, object, iter->first, wantParams);
        } else if (AAFwk::IChar::Query(iter->second) != nullptr) {
            InnerWrapJsWantParams<AAFwk::IChar, AAFwk::Char, char>(
                env, object, iter->first, wantParams);
        } else if (AAFwk::IByte::Query(iter->second) != nullptr) {
            InnerWrapJsWantParams<AAFwk::IByte, AAFwk::Byte, int>(
                env, object, iter->first, wantParams);
        } else if (AAFwk::IArray::Query(iter->second) != nullptr) {
            AAFwk::IArray *ao = AAFwk::IArray::Query(iter->second);
            if (ao != nullptr) {
                sptr<AAFwk::IArray> array(ao);
                WrapJsWantParamsArray(env, object, iter->first, array);
            }
        } else if (AAFwk::IWantParams::Query(iter->second) != nullptr) {
            InnerWrapJsWantParamsWantParams(env, object, iter->first, wantParams);
        }
    }
    return handleEscape.Escape(object);
}

napi_value CreateJsMissionInfoArray(napi_env env, const std::vector<AAFwk::MissionInfo> &missionInfos)
{
    HandleEscape handleEscape(env);
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, missionInfos.size(), &arrayValue);
    uint32_t index = 0;
    for (const auto &missionInfo : missionInfos) {
        napi_set_element(env, arrayValue, index++, CreateJsMissionInfo(env, missionInfo));
    }
    TAG_LOGD(AAFwkTag::MISSION, "end");
    return handleEscape.Escape(arrayValue);
}

bool InnerWrapJsWantParamsWantParams(
    napi_env env, napi_value object, const std::string &key, const AAFwk::WantParams &wantParams)
{
    TAG_LOGD(AAFwkTag::MISSION, "enter");
    auto value = wantParams.GetParam(key);
    AAFwk::IWantParams *o = AAFwk::IWantParams::Query(value);
    if (o != nullptr) {
        AAFwk::WantParams wp = AAFwk::WantParamWrapper::Unbox(o);
        napi_set_named_property(env, object, key.c_str(), CreateJsWantParams(env, wp));
        return true;
    }
    TAG_LOGD(AAFwkTag::MISSION, "end");
    return false;
}

bool WrapJsWantParamsArray(napi_env env, napi_value object, const std::string &key, sptr<AAFwk::IArray> &ao)
{
    TAG_LOGI(AAFwkTag::MISSION, "key=%{public}s", key.c_str());
    if (AAFwk::Array::IsStringArray(ao)) {
        return InnerWrapWantParamsArray<AAFwk::IString, AAFwk::String, std::string>(
            env, object, key, ao);
    } else if (AAFwk::Array::IsBooleanArray(ao)) {
        return InnerWrapWantParamsArray<AAFwk::IBoolean, AAFwk::Boolean, bool>(
            env, object, key, ao);
    } else if (AAFwk::Array::IsShortArray(ao)) {
        return InnerWrapWantParamsArray<AAFwk::IShort, AAFwk::Short, short>(
            env, object, key, ao);
    } else if (AAFwk::Array::IsIntegerArray(ao)) {
        return InnerWrapWantParamsArray<AAFwk::IInteger, AAFwk::Integer, int>(
            env, object, key, ao);
    } else if (AAFwk::Array::IsLongArray(ao)) {
        return InnerWrapWantParamsArray<AAFwk::ILong, AAFwk::Long, int64_t>(
            env, object, key, ao);
    } else if (AAFwk::Array::IsFloatArray(ao)) {
        return InnerWrapWantParamsArray<AAFwk::IFloat, AAFwk::Float, float>(
            env, object, key, ao);
    } else if (AAFwk::Array::IsByteArray(ao)) {
        return InnerWrapWantParamsArray<AAFwk::IByte, AAFwk::Byte, int>(
            env, object, key, ao);
    } else if (AAFwk::Array::IsCharArray(ao)) {
        return InnerWrapWantParamsArray<AAFwk::IChar, AAFwk::Char, char>(
            env, object, key, ao);
    } else if (AAFwk::Array::IsDoubleArray(ao)) {
        return InnerWrapWantParamsArray<AAFwk::IDouble, AAFwk::Double, double>(
            env, object, key, ao);
    } else {
        return false;
    }
}
}  // namespace AbilityRuntime
}  // namespace OHOS
