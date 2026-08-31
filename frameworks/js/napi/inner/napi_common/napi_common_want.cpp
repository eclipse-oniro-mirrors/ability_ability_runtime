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

#include "napi_common_want.h"

#include "napi_common_util.h"
#include "array_wrapper.h"
#include "bool_wrapper.h"
#include "byte_wrapper.h"
#include "double_wrapper.h"
#include "float_wrapper.h"
#include "hilog_tag_wrapper.h"
#include "int_wrapper.h"
#include "ipc_skeleton.h"
#include "js_runtime_utils.h"
#include "long_wrapper.h"
#include "napi_remote_object.h"
#include "remote_object_wrapper.h"
#include "short_wrapper.h"
#include "string_wrapper.h"
#include "tokenid_kit.h"
#include "want_params_wrapper.h"
#include "zchar_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::AbilityRuntime;
const int PROPERTIES_SIZE = 2;
/**
 * @brief Init param of wantOptions.
 *
 * @param flagMap Indicates flag of list in Want .
 */
void InnerInitWantOptionsData(std::map<std::string, unsigned int> &flagMap)
{
    flagMap.emplace("authReadUriPermission", Want::FLAG_AUTH_READ_URI_PERMISSION);
    flagMap.emplace("authWriteUriPermission", Want::FLAG_AUTH_WRITE_URI_PERMISSION);
    flagMap.emplace("abilityForwardResult", Want::FLAG_ABILITY_FORWARD_RESULT);
    flagMap.emplace("abilityContinuation", Want::FLAG_ABILITY_CONTINUATION);
    flagMap.emplace("notOhosComponent", Want::FLAG_NOT_OHOS_COMPONENT);
    flagMap.emplace("abilityFormEnabled", Want::FLAG_ABILITY_FORM_ENABLED);
    flagMap.emplace("authPersistableUriPermission", Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION);
    flagMap.emplace("authPrefixUriPermission", Want::FLAG_AUTH_PREFIX_URI_PERMISSION);
    flagMap.emplace("abilitySliceMultiDevice", Want::FLAG_ABILITYSLICE_MULTI_DEVICE);
    flagMap.emplace("startForegroundAbility", Want::FLAG_START_FOREGROUND_ABILITY);
    flagMap.emplace("installOnDemand", Want::FLAG_INSTALL_ON_DEMAND);
    flagMap.emplace("abilitySliceForwardResult", Want::FLAG_ABILITYSLICE_FORWARD_RESULT);
    flagMap.emplace("installWithBackgroundMode", Want::FLAG_INSTALL_WITH_BACKGROUND_MODE);
    flagMap.emplace("abilityContinuationReversible", Want::FLAG_ABILITY_CONTINUATION_REVERSIBLE);
    flagMap.emplace("abilityClearMission", Want::FLAG_ABILITY_CLEAR_MISSION);
    flagMap.emplace("abilityNewMission", Want::FLAG_ABILITY_NEW_MISSION);
    flagMap.emplace("abilityMissionTop", Want::FLAG_ABILITY_MISSION_TOP);
    flagMap.emplace("abilityOnCollaborate", Want::FLAG_ABILITY_ON_COLLABORATE);
}

napi_value WrapElementName(napi_env env, const ElementName &elementName)
{
    HandleEscape handleEscape(env);
    napi_value jsObject = nullptr;
    NAPI_CALL(env, napi_create_object(env, &jsObject));

    napi_value jsValue = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, elementName.GetDeviceID().c_str(), NAPI_AUTO_LENGTH, &jsValue));
    NAPI_CALL(env, napi_set_named_property(env, jsObject, "deviceId", jsValue));

    jsValue = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, elementName.GetBundleName().c_str(), NAPI_AUTO_LENGTH, &jsValue));
    NAPI_CALL(env, napi_set_named_property(env, jsObject, "bundleName", jsValue));

    jsValue = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, elementName.GetAbilityName().c_str(), NAPI_AUTO_LENGTH, &jsValue));
    NAPI_CALL(env, napi_set_named_property(env, jsObject, "abilityName", jsValue));

    jsValue = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, elementName.GetModuleName().c_str(), NAPI_AUTO_LENGTH, &jsValue));
    NAPI_CALL(env, napi_set_named_property(env, jsObject, "moduleName", jsValue));

    return handleEscape.Escape(jsObject);
}

bool UnwrapElementName(napi_env env, napi_value param, ElementName &elementName)
{
    std::string natValue("");
    if (UnwrapStringByPropertyName(env, param, "deviceId", natValue)) {
        elementName.SetDeviceID(natValue);
    }

    natValue = "";
    if (UnwrapStringByPropertyName(env, param, "bundleName", natValue)) {
        elementName.SetBundleName(natValue);
    }

    natValue = "";
    if (UnwrapStringByPropertyName(env, param, "abilityName", natValue)) {
        elementName.SetAbilityName(natValue);
    }

    natValue = "";
    if (UnwrapStringByPropertyName(env, param, "moduleName", natValue)) {
        elementName.SetModuleName(natValue);
    }
    return true;
}

bool InnerWrapWantParamsChar(
    napi_env env, napi_value jsObject, const std::string &key, const AAFwk::WantParams &wantParams)
{
    auto value = wantParams.GetParam(key);
    AAFwk::IChar *ao = AAFwk::IChar::Query(value);
    if (ao == nullptr) {
        return false;
    }
    AbilityRuntime::HandleScope handleScope(env);
    std::string natValue(static_cast<Char *>(ao)->ToString());
    napi_value jsValue = WrapStringToJS(env, natValue);
    if (jsValue == nullptr) {
        return false;
    }

    NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
    return true;
}

bool InnerWrapWantParamsString(
    napi_env env, napi_value jsObject, const std::string &key, const AAFwk::WantParams &wantParams)
{
    auto value = wantParams.GetParam(key);
    AAFwk::IString *ao = AAFwk::IString::Query(value);
    if (ao == nullptr) {
        return false;
    }
    AbilityRuntime::HandleScope handleScope(env);
    std::string natValue = AAFwk::String::Unbox(ao);
    napi_value jsValue = WrapStringToJS(env, natValue);
    if (jsValue == nullptr) {
        return false;
    }

    NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
    return true;
}

bool InnerWrapWantParamsBool(
    napi_env env, napi_value jsObject, const std::string &key, const AAFwk::WantParams &wantParams)
{
    auto value = wantParams.GetParam(key);
    AAFwk::IBoolean *bo = AAFwk::IBoolean::Query(value);
    if (bo == nullptr) {
        return false;
    }
    AbilityRuntime::HandleScope handleScope(env);
    bool natValue = AAFwk::Boolean::Unbox(bo);
    napi_value jsValue = WrapBoolToJS(env, natValue);
    if (jsValue == nullptr) {
        return false;
    }

    NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
    return true;
}

bool InnerWrapWantParamsByte(
    napi_env env, napi_value jsObject, const std::string &key, const AAFwk::WantParams &wantParams)
{
    AbilityRuntime::HandleScope handleScope(env);
    auto value = wantParams.GetParam(key);
    AAFwk::IByte *bo = AAFwk::IByte::Query(value);
    if (bo == nullptr) {
        return false;
    }

    int intValue = (int)AAFwk::Byte::Unbox(bo);
    napi_value jsValue = WrapInt32ToJS(env, intValue);
    if (jsValue == nullptr) {
        return false;
    }

    NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
    return true;
}

bool InnerWrapWantParamsShort(
    napi_env env, napi_value jsObject, const std::string &key, const AAFwk::WantParams &wantParams)
{
    AbilityRuntime::HandleScope handleScope(env);
    auto value = wantParams.GetParam(key);
    AAFwk::IShort *ao = AAFwk::IShort::Query(value);
    if (ao == nullptr) {
        return false;
    }

    short natValue = AAFwk::Short::Unbox(ao);
    napi_value jsValue = WrapInt32ToJS(env, natValue);
    if (jsValue == nullptr) {
        return false;
    }

    NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
    return true;
}

bool InnerWrapWantParamsInt32(
    napi_env env, napi_value jsObject, const std::string &key, const AAFwk::WantParams &wantParams)
{
    AbilityRuntime::HandleScope handleScope(env);
    auto value = wantParams.GetParam(key);
    AAFwk::IInteger *ao = AAFwk::IInteger::Query(value);
    if (ao == nullptr) {
        return false;
    }

    int natValue = AAFwk::Integer::Unbox(ao);
    napi_value jsValue = WrapInt32ToJS(env, natValue);
    if (jsValue == nullptr) {
        return false;
    }

    NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
    return true;
}

bool InnerWrapWantParamsInt64(
    napi_env env, napi_value jsObject, const std::string &key, const AAFwk::WantParams &wantParams)
{
    AbilityRuntime::HandleScope handleScope(env);
    auto value = wantParams.GetParam(key);
    AAFwk::ILong *ao = AAFwk::ILong::Query(value);
    if (ao == nullptr) {
        return false;
    }

    int64_t natValue = AAFwk::Long::Unbox(ao);
    napi_value jsValue = WrapInt64ToJS(env, natValue);
    if (jsValue == nullptr) {
        return false;
    }

    NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
    return true;
}

bool InnerWrapWantParamsFloat(
    napi_env env, napi_value jsObject, const std::string &key, const AAFwk::WantParams &wantParams)
{
    AbilityRuntime::HandleScope handleScope(env);
    auto value = wantParams.GetParam(key);
    AAFwk::IFloat *ao = AAFwk::IFloat::Query(value);
    if (ao == nullptr) {
        return false;
    }

    float natValue = AAFwk::Float::Unbox(ao);
    napi_value jsValue = WrapDoubleToJS(env, natValue);
    if (jsValue == nullptr) {
        return false;
    }

    NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
    return true;
}

bool InnerWrapWantParamsDouble(
    napi_env env, napi_value jsObject, const std::string &key, const AAFwk::WantParams &wantParams)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    AbilityRuntime::HandleScope handleScope(env);
    auto value = wantParams.GetParam(key);
    AAFwk::IDouble *ao = AAFwk::IDouble::Query(value);
    if (ao == nullptr) {
        return false;
    }

    double natValue = AAFwk::Double::Unbox(ao);
    napi_value jsValue = WrapDoubleToJS(env, natValue);
    if (jsValue == nullptr) {
        return false;
    }

    NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
    return true;
}

bool InnerWrapWantParamsWantParams(
    napi_env env, napi_value jsObject, const std::string &key, const AAFwk::WantParams &wantParams)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "key=%{public}s", key.c_str());
    AbilityRuntime::HandleScope handleScope(env);
    auto value = wantParams.GetParam(key);
    AAFwk::IWantParams *o = AAFwk::IWantParams::Query(value);
    if (o == nullptr) {
        return false;
    }

    AAFwk::WantParams wp = AAFwk::WantParamWrapper::Unbox(o);
    napi_value jsValue = WrapWantParams(env, wp);
    if (jsValue == nullptr) {
        return false;
    }

    NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
    return true;
}

bool InnerWrapWantParamsRemoteObject(
    napi_env env, napi_value jsObject, const std::string &key, const AAFwk::WantParams &wantParams)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "key=%{public}s", key.c_str());
    auto value = wantParams.GetParam(key);
    AAFwk::IRemoteObjectWrap *remoteObjectIWrap = AAFwk::IRemoteObjectWrap::Query(value);
    if (remoteObjectIWrap == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null remoteObjectIWrap");
        return false;
    }
    auto remoteObject = AAFwk::RemoteObjectWrap::UnBox(remoteObjectIWrap);
    auto jsValue = NAPI_ohos_rpc_CreateJsRemoteObject(env, remoteObject);
    if (jsValue == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null jsValue");
        return false;
    }

    NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
    return true;
}

bool InnerWrapWantParamsArrayChar(napi_env env, napi_value jsObject, const std::string &key, sptr<AAFwk::IArray> &ao)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    long size = 0;
    if (ao->GetLength(size) != ERR_OK) {
        return false;
    }

    std::vector<std::string> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            AAFwk::IChar *iValue = AAFwk::IChar::Query(iface);
            if (iValue != nullptr) {
                std::string str(static_cast<Char *>(iValue)->ToString());
                natArray.push_back(str);
            }
        }
    }
    AbilityRuntime::HandleScope handleScope(env);
    napi_value jsValue = WrapArrayStringToJS(env, natArray);
    if (jsValue != nullptr) {
        NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
        return true;
    }
    return false;
}

bool InnerWrapWantParamsArrayString(napi_env env, napi_value jsObject, const std::string &key, sptr<AAFwk::IArray> &ao)
{
    long size = 0;
    if (ao->GetLength(size) != ERR_OK) {
        return false;
    }

    std::vector<std::string> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            AAFwk::IString *iValue = AAFwk::IString::Query(iface);
            if (iValue != nullptr) {
                natArray.push_back(AAFwk::String::Unbox(iValue));
            }
        }
    }
    AbilityRuntime::HandleScope handleScope(env);
    napi_value jsValue = WrapArrayStringToJS(env, natArray);
    if (jsValue != nullptr) {
        NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
        return true;
    }
    return false;
}

bool InnerWrapWantParamsArrayBool(napi_env env, napi_value jsObject, const std::string &key, sptr<AAFwk::IArray> &ao)
{
    AbilityRuntime::HandleScope handleScope(env);
    long size = 0;
    if (ao->GetLength(size) != ERR_OK) {
        return false;
    }

    std::vector<bool> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            AAFwk::IBoolean *iValue = AAFwk::IBoolean::Query(iface);
            if (iValue != nullptr) {
                natArray.push_back(AAFwk::Boolean::Unbox(iValue));
            }
        }
    }

    napi_value jsValue = WrapArrayBoolToJS(env, natArray);
    if (jsValue != nullptr) {
        NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
        return true;
    }
    return false;
}

bool InnerWrapWantParamsArrayShort(napi_env env, napi_value jsObject, const std::string &key, sptr<AAFwk::IArray> &ao)
{
    AbilityRuntime::HandleScope handleScope(env);
    long size = 0;
    if (ao->GetLength(size) != ERR_OK) {
        return false;
    }

    std::vector<int> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            AAFwk::IShort *iValue = AAFwk::IShort::Query(iface);
            if (iValue != nullptr) {
                natArray.push_back(AAFwk::Short::Unbox(iValue));
            }
        }
    }

    napi_value jsValue = WrapArrayInt32ToJS(env, natArray);
    if (jsValue != nullptr) {
        NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
        return true;
    }
    return false;
}
bool InnerWrapWantParamsArrayByte(napi_env env, napi_value jsObject, const std::string &key, sptr<AAFwk::IArray> &ao)
{
    AbilityRuntime::HandleScope handleScope(env);
    long size = 0;
    if (ao->GetLength(size) != ERR_OK) {
        return false;
    }

    std::vector<int> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            AAFwk::IByte *iValue = AAFwk::IByte::Query(iface);
            if (iValue != nullptr) {
                int intValue = (int)AAFwk::Byte::Unbox(iValue);
                natArray.push_back(intValue);
            }
        }
    }

    napi_value jsValue = WrapArrayInt32ToJS(env, natArray);
    if (jsValue != nullptr) {
        NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
        return true;
    }
    return false;
}

bool InnerWrapWantParamsArrayInt32(napi_env env, napi_value jsObject, const std::string &key, sptr<AAFwk::IArray> &ao)
{
    AbilityRuntime::HandleScope handleScope(env);
    long size = 0;
    if (ao->GetLength(size) != ERR_OK) {
        return false;
    }

    std::vector<int> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            AAFwk::IInteger *iValue = AAFwk::IInteger::Query(iface);
            if (iValue != nullptr) {
                natArray.push_back(AAFwk::Integer::Unbox(iValue));
            }
        }
    }

    napi_value jsValue = WrapArrayInt32ToJS(env, natArray);
    if (jsValue != nullptr) {
        NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
        return true;
    }
    return false;
}

bool InnerWrapWantParamsArrayInt64(napi_env env, napi_value jsObject, const std::string &key, sptr<AAFwk::IArray> &ao)
{
    AbilityRuntime::HandleScope handleScope(env);
    long size = 0;
    if (ao->GetLength(size) != ERR_OK) {
        return false;
    }

    std::vector<int64_t> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            AAFwk::ILong *iValue = AAFwk::ILong::Query(iface);
            if (iValue != nullptr) {
                natArray.push_back(AAFwk::Long::Unbox(iValue));
            }
        }
    }

    napi_value jsValue = WrapArrayInt64ToJS(env, natArray);
    if (jsValue != nullptr) {
        NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
        return true;
    }
    return false;
}

bool InnerWrapWantParamsArrayFloat(napi_env env, napi_value jsObject, const std::string &key, sptr<AAFwk::IArray> &ao)
{
    AbilityRuntime::HandleScope handleScope(env);
    long size = 0;
    if (ao->GetLength(size) != ERR_OK) {
        return false;
    }

    std::vector<double> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            AAFwk::IFloat *iValue = AAFwk::IFloat::Query(iface);
            if (iValue != nullptr) {
                natArray.push_back(AAFwk::Float::Unbox(iValue));
            }
        }
    }

    napi_value jsValue = WrapArrayDoubleToJS(env, natArray);
    if (jsValue != nullptr) {
        NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
        return true;
    }
    return false;
}

napi_value WrapArrayWantParamsToJS(napi_env env, const std::vector<WantParams> &value)
{
    HandleEscape handleEscape(env);
    napi_value jsArray = nullptr;
    napi_value jsValue = nullptr;
    uint32_t index = 0;

    NAPI_CALL(env, napi_create_array(env, &jsArray));
    for (uint32_t i = 0; i < value.size(); i++) {
        jsValue = WrapWantParams(env, value[i]);
        if (jsValue != nullptr) {
            if (napi_set_element(env, jsArray, index, jsValue) == napi_ok) {
                index++;
            }
        }
    }
    return handleEscape.Escape(jsArray);
}

bool InnerWrapWantParamsArrayDouble(napi_env env, napi_value jsObject, const std::string &key, sptr<AAFwk::IArray> &ao)
{
    AbilityRuntime::HandleScope handleScope(env);
    long size = 0;
    if (ao->GetLength(size) != ERR_OK) {
        return false;
    }

    std::vector<double> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            AAFwk::IDouble *iValue = AAFwk::IDouble::Query(iface);
            if (iValue != nullptr) {
                natArray.push_back(AAFwk::Double::Unbox(iValue));
            }
        }
    }

    napi_value jsValue = WrapArrayDoubleToJS(env, natArray);
    if (jsValue != nullptr) {
        NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
        return true;
    }
    return false;
}

bool InnerWrapWantParamsArrayWantParams(napi_env env, napi_value jsObject,
    const std::string &key, sptr<AAFwk::IArray> &ao)
{
    AbilityRuntime::HandleScope handleScope(env);
    long size = 0;
    if (ao->GetLength(size) != ERR_OK) {
        return false;
    }

    std::vector<WantParams> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            AAFwk::IWantParams *iValue = AAFwk::IWantParams::Query(iface);
            if (iValue != nullptr) {
                natArray.push_back(AAFwk::WantParamWrapper::Unbox(iValue));
            }
        }
    }

    napi_value jsValue = WrapArrayWantParamsToJS(env, natArray);
    if (jsValue != nullptr) {
        NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
        return true;
    }
    return false;
}

bool InnerWrapWantParamsArray(napi_env env, napi_value jsObject, const std::string &key, sptr<AAFwk::IArray> &ao)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "key=%{public}s", key.c_str());
    if (AAFwk::Array::IsStringArray(ao)) {
        return InnerWrapWantParamsArrayString(env, jsObject, key, ao);
    } else if (AAFwk::Array::IsBooleanArray(ao)) {
        return InnerWrapWantParamsArrayBool(env, jsObject, key, ao);
    } else if (AAFwk::Array::IsShortArray(ao)) {
        return InnerWrapWantParamsArrayShort(env, jsObject, key, ao);
    } else if (AAFwk::Array::IsIntegerArray(ao)) {
        return InnerWrapWantParamsArrayInt32(env, jsObject, key, ao);
    } else if (AAFwk::Array::IsLongArray(ao)) {
        return InnerWrapWantParamsArrayInt64(env, jsObject, key, ao);
    } else if (AAFwk::Array::IsFloatArray(ao)) {
        return InnerWrapWantParamsArrayFloat(env, jsObject, key, ao);
    } else if (AAFwk::Array::IsByteArray(ao)) {
        return InnerWrapWantParamsArrayByte(env, jsObject, key, ao);
    } else if (AAFwk::Array::IsCharArray(ao)) {
        return InnerWrapWantParamsArrayChar(env, jsObject, key, ao);
    } else if (AAFwk::Array::IsDoubleArray(ao)) {
        return InnerWrapWantParamsArrayDouble(env, jsObject, key, ao);
    } else if (AAFwk::Array::IsWantParamsArray(ao)) {
        return InnerWrapWantParamsArrayWantParams(env, jsObject, key, ao);
    } else {
        return false;
    }
}

napi_value WrapWantParams(napi_env env, const AAFwk::WantParams &wantParams)
{
    AbilityRuntime::HandleEscape handleEscape(env);
    napi_value jsObject = nullptr;
    NAPI_CALL(env, napi_create_object(env, &jsObject));

    napi_value jsValue = nullptr;
    const std::map<std::string, sptr<AAFwk::IInterface>> paramList = wantParams.GetParams();
    for (auto iter = paramList.begin(); iter != paramList.end(); iter++) {
        jsValue = nullptr;
        if (AAFwk::IString::Query(iter->second) != nullptr) {
            InnerWrapWantParamsString(env, jsObject, iter->first, wantParams);
        } else if (AAFwk::IBoolean::Query(iter->second) != nullptr) {
            InnerWrapWantParamsBool(env, jsObject, iter->first, wantParams);
        } else if (AAFwk::IShort::Query(iter->second) != nullptr) {
            InnerWrapWantParamsShort(env, jsObject, iter->first, wantParams);
        } else if (AAFwk::IInteger::Query(iter->second) != nullptr) {
            InnerWrapWantParamsInt32(env, jsObject, iter->first, wantParams);
        } else if (AAFwk::ILong::Query(iter->second) != nullptr) {
            InnerWrapWantParamsInt64(env, jsObject, iter->first, wantParams);
        } else if (AAFwk::IFloat::Query(iter->second) != nullptr) {
            InnerWrapWantParamsFloat(env, jsObject, iter->first, wantParams);
        } else if (AAFwk::IDouble::Query(iter->second) != nullptr) {
            InnerWrapWantParamsDouble(env, jsObject, iter->first, wantParams);
        } else if (AAFwk::IChar::Query(iter->second) != nullptr) {
            InnerWrapWantParamsChar(env, jsObject, iter->first, wantParams);
        } else if (AAFwk::IByte::Query(iter->second) != nullptr) {
            InnerWrapWantParamsByte(env, jsObject, iter->first, wantParams);
        } else if (AAFwk::IArray::Query(iter->second) != nullptr) {
            AAFwk::IArray *ao = AAFwk::IArray::Query(iter->second);
            if (ao != nullptr) {
                sptr<AAFwk::IArray> array(ao);
                InnerWrapWantParamsArray(env, jsObject, iter->first, array);
            }
        } else if (AAFwk::IWantParams::Query(iter->second) != nullptr) {
            InnerWrapWantParamsWantParams(env, jsObject, iter->first, wantParams);
        } else if (AAFwk::IRemoteObjectWrap::Query(iter->second) != nullptr) {
            InnerWrapWantParamsRemoteObject(env, jsObject, iter->first, wantParams);
        }
    }
    return handleEscape.Escape(jsObject);
}

bool InnerSetWantParamsArrayObject(napi_env env, const std::string &key,
    const std::vector<napi_value> &value, AAFwk::WantParams &wantParams)
{
    size_t size = value.size();
    sptr<AAFwk::IArray> ao = new (std::nothrow) AAFwk::Array(size, AAFwk::g_IID_IWantParams);
    if (ao != nullptr) {
        for (size_t i = 0; i < size; i++) {
            AAFwk::WantParams wp;
            UnwrapWantParams(env, value[i], wp);
            ao->Set(i, AAFwk::WantParamWrapper::Box(wp));
        }
        wantParams.SetParam(key, ao);
        return true;
    } else {
        return false;
    }
}

bool InnerSetWantParamsArrayString(
    const std::string &key, const std::vector<std::string> &value, AAFwk::WantParams &wantParams)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    size_t size = value.size();
    sptr<AAFwk::IArray> ao = new (std::nothrow) AAFwk::Array(size, AAFwk::g_IID_IString);
    if (ao != nullptr) {
        for (size_t i = 0; i < size; i++) {
            ao->Set(i, AAFwk::String::Box(value[i]));
        }
        wantParams.SetParam(key, ao);
        return true;
    } else {
        return false;
    }
}

bool InnerSetWantParamsArrayInt(const std::string &key, const std::vector<int> &value, AAFwk::WantParams &wantParams)
{
    size_t size = value.size();
    sptr<AAFwk::IArray> ao = new (std::nothrow) AAFwk::Array(size, AAFwk::g_IID_IInteger);
    if (ao != nullptr) {
        for (size_t i = 0; i < size; i++) {
            ao->Set(i, AAFwk::Integer::Box(value[i]));
        }
        wantParams.SetParam(key, ao);
        return true;
    } else {
        return false;
    }
}

bool InnerSetWantParamsArrayLong(const std::string &key, const std::vector<long> &value, AAFwk::WantParams &wantParams)
{
    size_t size = value.size();
    sptr<AAFwk::IArray> ao = new (std::nothrow) AAFwk::Array(size, AAFwk::g_IID_ILong);
    if (ao != nullptr) {
        for (size_t i = 0; i < size; i++) {
            ao->Set(i, AAFwk::Long::Box(value[i]));
        }
        wantParams.SetParam(key, ao);
        return true;
    } else {
        return false;
    }
}

bool InnerSetWantParamsArrayBool(const std::string &key, const std::vector<bool> &value, AAFwk::WantParams &wantParams)
{
    size_t size = value.size();
    sptr<AAFwk::IArray> ao = new (std::nothrow) AAFwk::Array(size, AAFwk::g_IID_IBoolean);
    if (ao != nullptr) {
        for (size_t i = 0; i < size; i++) {
            ao->Set(i, AAFwk::Boolean::Box(value[i]));
        }
        wantParams.SetParam(key, ao);
        return true;
    } else {
        return false;
    }
}

bool InnerSetWantParamsArrayDouble(
    const std::string &key, const std::vector<double> &value, AAFwk::WantParams &wantParams)
{
    size_t size = value.size();
    sptr<AAFwk::IArray> ao = new (std::nothrow) AAFwk::Array(size, AAFwk::g_IID_IDouble);
    if (ao != nullptr) {
        for (size_t i = 0; i < size; i++) {
            ao->Set(i, AAFwk::Double::Box(value[i]));
        }
        wantParams.SetParam(key, ao);
        return true;
    } else {
        return false;
    }
}

bool InnerUnwrapWantParamsArray(napi_env env, const std::string &key, napi_value param, AAFwk::WantParams &wantParams)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    ComplexArrayData natArrayValue;
    if (!UnwrapArrayComplexFromJS(env, param, natArrayValue)) {
        return false;
    }
    if (natArrayValue.objectList.size() > 0) {
        return InnerSetWantParamsArrayObject(env, key, natArrayValue.objectList, wantParams);
    }
    if (natArrayValue.stringList.size() > 0) {
        return InnerSetWantParamsArrayString(key, natArrayValue.stringList, wantParams);
    }
    if (natArrayValue.intList.size() > 0) {
        return InnerSetWantParamsArrayInt(key, natArrayValue.intList, wantParams);
    }
    if (natArrayValue.longList.size() > 0) {
        return InnerSetWantParamsArrayLong(key, natArrayValue.longList, wantParams);
    }
    if (natArrayValue.boolList.size() > 0) {
        return InnerSetWantParamsArrayBool(key, natArrayValue.boolList, wantParams);
    }
    if (natArrayValue.doubleList.size() > 0) {
        return InnerSetWantParamsArrayDouble(key, natArrayValue.doubleList, wantParams);
    }

    return false;
}

bool InnerUnwrapWantParams(napi_env env, const std::string &key, napi_value param, AAFwk::WantParams &wantParams)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    AAFwk::WantParams wp;

    if (UnwrapWantParams(env, param, wp)) {
        sptr<AAFwk::IWantParams> pWantParams = AAFwk::WantParamWrapper::Box(wp);
        if (pWantParams != nullptr) {
            wantParams.SetParam(key, pWantParams);
            return true;
        }
    }
    return false;
}

void InnerUnwrapWantParamsNumber(napi_env env, const std::string &key, napi_value param, AAFwk::WantParams &wantParams)
{
    int32_t natValue32 = 0;
    double natValueDouble = 0.0;
    bool isReadValue32 = false;
    bool isReadDouble = false;
    if (napi_get_value_int32(env, param, &natValue32) == napi_ok) {
        isReadValue32 = true;
    }

    if (napi_get_value_double(env, param, &natValueDouble) == napi_ok) {
        isReadDouble = true;
    }

    if (isReadValue32 && isReadDouble) {
        if (abs(natValueDouble - natValue32 * 1.0) > 0.0) {
            wantParams.SetParam(key, AAFwk::Double::Box(natValueDouble));
        } else {
            wantParams.SetParam(key, AAFwk::Integer::Box(natValue32));
        }
    } else if (isReadValue32) {
        wantParams.SetParam(key, AAFwk::Integer::Box(natValue32));
    } else if (isReadDouble) {
        wantParams.SetParam(key, AAFwk::Double::Box(natValueDouble));
    }
}

bool BlackListFilter(const std::string &strProName, const std::string &proNameNotFilter)
{
    if (strProName == proNameNotFilter) {
        return false;
    }
    if (strProName == Want::PARAM_RESV_WINDOW_MODE) {
        return true;
    }
    if (strProName == Want::PARAM_RESV_DISPLAY_ID) {
        return true;
    }
    return false;
}

bool UnwrapWantParams(napi_env env, napi_value param, AAFwk::WantParams &wantParams)
{
    return UnwrapWantParams(env, param, wantParams, "");
}

bool UnwrapWantParams(napi_env env, napi_value param, AAFwk::WantParams &wantParams,
    const std::string &proNameNotFilter)
{
    if (!IsTypeForNapiValue(env, param, napi_object)) {
        return false;
    }

    napi_valuetype jsValueType = napi_undefined;
    napi_value jsProNameList = nullptr;
    uint32_t jsProCount = 0;

    NAPI_CALL_BASE(env, napi_get_property_names(env, param, &jsProNameList), false);
    NAPI_CALL_BASE(env, napi_get_array_length(env, jsProNameList, &jsProCount), false);

    napi_value jsProName = nullptr;
    napi_value jsProValue = nullptr;
    for (uint32_t index = 0; index < jsProCount; index++) {
        NAPI_CALL_BASE(env, napi_get_element(env, jsProNameList, index, &jsProName), false);

        std::string strProName = UnwrapStringFromJS(env, jsProName);
        /* skip reserved param */
        if (BlackListFilter(strProName, proNameNotFilter)) {
            TAG_LOGD(AAFwkTag::JSNAPI, "%{public}s is filtered.", strProName.c_str());
            continue;
        }
        TAG_LOGD(AAFwkTag::JSNAPI, "property name=%{public}s", strProName.c_str());
        NAPI_CALL_BASE(env, napi_get_named_property(env, param, strProName.c_str(), &jsProValue), false);
        NAPI_CALL_BASE(env, napi_typeof(env, jsProValue, &jsValueType), false);

        switch (jsValueType) {
            case napi_string: {
                std::string natValue = UnwrapStringFromJS(env, jsProValue);
                wantParams.SetParam(strProName, AAFwk::String::Box(natValue));
                break;
            }
            case napi_boolean: {
                bool natValue = false;
                NAPI_CALL_BASE(env, napi_get_value_bool(env, jsProValue, &natValue), false);
                wantParams.SetParam(strProName, AAFwk::Boolean::Box(natValue));
                break;
            }
            case napi_number: {
                InnerUnwrapWantParamsNumber(env, strProName, jsProValue, wantParams);
                break;
            }
            case napi_object: {
                HandleNapiObject(env, param, jsProValue, strProName, wantParams);
                break;
            }
            default:
                break;
        }
    }

    return true;
}

void HandleNapiObject(napi_env env, napi_value param, napi_value jsProValue, std::string &strProName,
    AAFwk::WantParams &wantParams)
{
    if (IsSpecialObject(env, param, strProName, FD, napi_number)) {
        HandleFdObject(env, param, strProName, wantParams);
    } else if (IsSpecialObject(env, param, strProName, REMOTE_OBJECT, napi_object)) {
        auto selfToken = IPCSkeleton::GetSelfTokenID();
        if (Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
            HandleRemoteObject(env, param, strProName, wantParams);
        } else {
            TAG_LOGW(AAFwkTag::JSNAPI, "not system app");
        }
    } else {
        bool isArray = false;
        if (napi_is_array(env, jsProValue, &isArray) == napi_ok) {
            if (isArray) {
                InnerUnwrapWantParamsArray(env, strProName, jsProValue, wantParams);
            } else {
                InnerUnwrapWantParams(env, strProName, jsProValue, wantParams);
            }
        }
    }
}

bool IsSpecialObject(napi_env env, napi_value param, std::string &strProName, std::string type,
    napi_valuetype jsValueProType)
{
    napi_value jsWantParamProValue = nullptr;
    NAPI_CALL_BASE(env, napi_get_named_property(env, param, strProName.c_str(), &jsWantParamProValue), false);

    napi_valuetype jsValueType = napi_undefined;
    napi_value jsProValue = nullptr;
    NAPI_CALL_BASE(env, napi_get_named_property(env, jsWantParamProValue, TYPE_PROPERTY, &jsProValue), false);
    NAPI_CALL_BASE(env, napi_typeof(env, jsProValue, &jsValueType), false);
    if (jsValueType != napi_string) {
        return false;
    }
    std::string natValue = UnwrapStringFromJS(env, jsProValue);
    if (natValue != type) {
        return false;
    }
    napi_value jsProNameList = nullptr;
    uint32_t jsProCount = 0;

    NAPI_CALL_BASE(env, napi_get_property_names(env, jsWantParamProValue, &jsProNameList), false);
    NAPI_CALL_BASE(env, napi_get_array_length(env, jsProNameList, &jsProCount), false);

    if (jsProCount != PROPERTIES_SIZE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "invalid size, not fd object");
        return false;
    }

    jsValueType = napi_undefined;
    jsProValue = nullptr;
    NAPI_CALL_BASE(env, napi_get_named_property(env, jsWantParamProValue, VALUE_PROPERTY, &jsProValue),
        false);
    NAPI_CALL_BASE(env, napi_typeof(env, jsProValue, &jsValueType), false);
    if (jsValueType != jsValueProType) {
        TAG_LOGE(AAFwkTag::JSNAPI, "invalid value property, not fd object");
        return false;
    }

    return true;
}

bool HandleFdObject(napi_env env, napi_value param, std::string &strProName, AAFwk::WantParams &wantParams)
{
    napi_value jsWantParamProValue = nullptr;
    NAPI_CALL_BASE(env, napi_get_named_property(env, param, strProName.c_str(), &jsWantParamProValue), false);
    napi_value jsProValue = nullptr;
    NAPI_CALL_BASE(env, napi_get_named_property(env, jsWantParamProValue, VALUE_PROPERTY, &jsProValue),
        false);

    int32_t natValue32 = 0;
    napi_get_value_int32(env, jsProValue, &natValue32);
    TAG_LOGI(AAFwkTag::JSNAPI, "fd:%{public}d", natValue32);
    WantParams wp;
    wp.SetParam(TYPE_PROPERTY, String::Box(FD));
    wp.SetParam(VALUE_PROPERTY, Integer::Box(natValue32));
    sptr<AAFwk::IWantParams> pWantParams = AAFwk::WantParamWrapper::Box(wp);
    wantParams.SetParam(strProName, pWantParams);
    return true;
}

bool HandleRemoteObject(napi_env env, napi_value param, std::string &strProName, AAFwk::WantParams &wantParams)
{
    napi_value jsWantParamProValue = nullptr;
    NAPI_CALL_BASE(env, napi_get_named_property(env, param, strProName.c_str(), &jsWantParamProValue), false);
    napi_value jsProValue = nullptr;
    NAPI_CALL_BASE(env, napi_get_named_property(env, jsWantParamProValue, VALUE_PROPERTY, &jsProValue),
        false);

    sptr<IRemoteObject> remoteObject = NAPI_ohos_rpc_getNativeRemoteObject(env, jsProValue);
    if (remoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null remoteObject");
        return false;
    }

    WantParams wp;
    wp.SetParam(TYPE_PROPERTY, String::Box(REMOTE_OBJECT));
    wp.SetParam(VALUE_PROPERTY, AAFwk::RemoteObjectWrap::Box(remoteObject));
    sptr<AAFwk::IWantParams> pWantParams = AAFwk::WantParamWrapper::Box(wp);
    wantParams.SetParam(strProName, pWantParams);
    return true;
}

napi_value InnerWrapWantOptions(napi_env env, const Want &want)
{
    HandleEscape handleEscape(env);
    napi_value jsObject = nullptr;
    std::map<std::string, unsigned int> flagMap;
    InnerInitWantOptionsData(flagMap);
    unsigned int flags = want.GetFlags();
    bool natValue = false;
    napi_value jsValue = nullptr;

    NAPI_CALL(env, napi_create_object(env, &jsObject));
    for (auto iter = flagMap.begin(); iter != flagMap.end(); iter++) {
        jsValue = nullptr;
        natValue = ((flags & iter->second) == iter->second);
        if (napi_get_boolean(env, natValue, &jsValue) == napi_ok) {
            SetPropertyValueByPropertyName(env, jsObject, iter->first.c_str(), jsValue);
        }
    }

    return handleEscape.Escape(jsObject);
}

bool InnerUnwrapWantOptions(napi_env env, napi_value param, const char *propertyName, Want &want)
{
    AbilityRuntime::HandleScope handleScope(env);
    napi_value jsValue = GetPropertyValueByPropertyName(env, param, propertyName, napi_object);
    if (jsValue == nullptr) {
        return false;
    }

    bool natValue = false;
    unsigned int flags = 0;
    std::map<std::string, unsigned int> flagMap;
    InnerInitWantOptionsData(flagMap);
    for (auto iter = flagMap.begin(); iter != flagMap.end(); iter++) {
        natValue = false;
        if (UnwrapBooleanByPropertyName(env, jsValue, iter->first.c_str(), natValue)) {
            if (natValue) {
                flags |= iter->second;
            }
        }
    }

    want.SetFlags(flags);
    return true;
}

napi_value WrapWant(napi_env env, const Want &want)
{
    HandleEscape handleEscape(env);
    napi_value jsObject = nullptr;
    napi_value jsValue = nullptr;

    // PARAM_SET_URI_WITH_ORIGIN_STRING is an internal system parameter used to
    // pass through an origin uri that bypasses scheme validation. It must not be
    // exposed to third-party apps, so it is stripped here for non-system apps.
    Want wrapWant = want;
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        wrapWant.RemoveParam(Want::PARAM_SET_URI_WITH_ORIGIN_STRING);
    }

    NAPI_CALL(env, napi_create_object(env, &jsObject));

    napi_value jsElementName = WrapElementName(env, wrapWant.GetElement());
    if (jsElementName == nullptr) {
        TAG_LOGI(AAFwkTag::JSNAPI, "null jsElementName");
        return nullptr;
    }

    jsValue = GetPropertyValueByPropertyName(env, jsElementName, "deviceId", napi_string);
    SetPropertyValueByPropertyName(env, jsObject, "deviceId", jsValue);

    jsValue = nullptr;
    jsValue = GetPropertyValueByPropertyName(env, jsElementName, "bundleName", napi_string);
    SetPropertyValueByPropertyName(env, jsObject, "bundleName", jsValue);

    jsValue = nullptr;
    jsValue = GetPropertyValueByPropertyName(env, jsElementName, "abilityName", napi_string);
    SetPropertyValueByPropertyName(env, jsObject, "abilityName", jsValue);

    jsValue = nullptr;
    jsValue = GetPropertyValueByPropertyName(env, jsElementName, "moduleName", napi_string);
    SetPropertyValueByPropertyName(env, jsObject, "moduleName", jsValue);

    jsValue = nullptr;
    jsValue = WrapStringToJS(env, wrapWant.GetUriString());
    SetPropertyValueByPropertyName(env, jsObject, "uri", jsValue);

    jsValue = nullptr;
    jsValue = WrapStringToJS(env, wrapWant.GetType());
    SetPropertyValueByPropertyName(env, jsObject, "type", jsValue);

    jsValue = nullptr;
    jsValue = WrapInt32ToJS(env, wrapWant.GetFlags());
    SetPropertyValueByPropertyName(env, jsObject, "flags", jsValue);

    jsValue = nullptr;
    jsValue = WrapStringToJS(env, wrapWant.GetAction());
    SetPropertyValueByPropertyName(env, jsObject, "action", jsValue);

    jsValue = nullptr;
    jsValue = WrapWantParams(env, wrapWant.GetParams());
    SetPropertyValueByPropertyName(env, jsObject, "parameters", jsValue);

    jsValue = nullptr;
    jsValue = WrapWantParamsFD(env, wrapWant.GetParams());
    SetPropertyValueByPropertyName(env, jsObject, "fds", jsValue);

    jsValue = nullptr;
    jsValue = WrapArrayStringToJS(env, wrapWant.GetEntities());
    SetPropertyValueByPropertyName(env, jsObject, "entities", jsValue);

    return handleEscape.Escape(jsObject);
}

napi_value WrapWantParamsFD(napi_env env, const AAFwk::WantParams &wantParams)
{
    AbilityRuntime::HandleEscape handleEscape(env);
    napi_value jsObject = nullptr;
    NAPI_CALL(env, napi_create_object(env, &jsObject));
    auto paramList = wantParams.GetParams();
    WantParams fds;
    for (auto it = paramList.begin(); it != paramList.end(); it++) {
        if (AAFwk::IWantParams::Query(it->second) == nullptr) {
            TAG_LOGD(AAFwkTag::JSNAPI, "not wantpram");
            continue;
        }
        auto value = wantParams.GetParam(it->first);
        AAFwk::IWantParams *o = AAFwk::IWantParams::Query(value);
        if (o == nullptr) {
            return handleEscape.Escape(jsObject);
        }
        AAFwk::WantParams wp = AAFwk::WantParamWrapper::Unbox(o);
        auto valueMap = wp.GetParams();
        if (valueMap.size() != PROPERTIES_SIZE) {
            TAG_LOGD(AAFwkTag::JSNAPI, "not fd");
            return handleEscape.Escape(jsObject);
        }

        //type
        auto typeIt = valueMap.find(TYPE_PROPERTY);
        if (typeIt == valueMap.end()) {
            return handleEscape.Escape(jsObject);
        }
        AAFwk::IString *strValue = AAFwk::IString::Query(typeIt->second);
        if (strValue == nullptr) {
            return handleEscape.Escape(jsObject);
        }
        std::string typeString = AAFwk::String::Unbox(strValue);
        if (typeString != FD) {
            TAG_LOGD(AAFwkTag::JSNAPI, "not fd");
            return handleEscape.Escape(jsObject);
        }

        // value
        auto valueIt = valueMap.find(VALUE_PROPERTY);
        if (valueIt == valueMap.end()) {
            return handleEscape.Escape(jsObject);
        }
        AAFwk::IInteger *intValue = AAFwk::IInteger::Query(valueIt->second);
        if (intValue == nullptr) {
            return handleEscape.Escape(jsObject);
        }
        fds.SetParam(it->first, intValue);
    }
    return handleEscape.Escape(WrapWantParams(env, fds));
}

bool UnwrapWant(napi_env env, napi_value param, Want &want)
{
    return UnwrapWant(env, param, want, "");
}

void SetUriWithPassThroughFlag(const std::string &uriString, Want &want)
{
    if (!want.GetBoolParam(Want::PARAM_SET_URI_WITH_ORIGIN_STRING, false)) {
        want.SetUri(uriString);
        return;
    }
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    if (Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        // Restore the raw uri when the pass-through flag is set, so that the
        // non-standard scheme uri survives re-marshalling.
        Uri uri(uriString);
        uri.SetUriWithOriginString(uriString);
        want.SetUri(uri);
    } else {
        want.SetUri(uriString);
    }
}

bool UnwrapWant(napi_env env, napi_value param, Want &want, const std::string &proNameNotFilter)
{
    AbilityRuntime::HandleScope handleScope(env);
    if (!IsTypeForNapiValue(env, param, napi_object)) {
        TAG_LOGI(AAFwkTag::JSNAPI, "not napi_object");
        return false;
    }

    napi_value jsValue = GetPropertyValueByPropertyName(env, param, "parameters", napi_object);
    if (jsValue != nullptr) {
        AAFwk::WantParams wantParams;
        if (UnwrapWantParams(env, jsValue, wantParams, proNameNotFilter)) {
            want.SetParams(wantParams);
        }
    }

    std::string natValueString("");
    if (UnwrapStringByPropertyName(env, param, "action", natValueString)) {
        want.SetAction(natValueString);
    }

    std::vector<std::string> natValueStringList;
    if (UnwrapStringArrayByPropertyName(env, param, "entities", natValueStringList)) {
        for (size_t i = 0; i < natValueStringList.size(); i++) {
            want.AddEntity(natValueStringList[i]);
        }
    }

    natValueString = "";
    if (UnwrapStringByPropertyName(env, param, "uri", natValueString)) {
        SetUriWithPassThroughFlag(natValueString, want);
    }

    int32_t flags = 0;
    if (UnwrapInt32ByPropertyName(env, param, "flags", flags)) {
        want.SetFlags(flags);
    }

    ElementName natElementName;
    UnwrapElementName(env, param, natElementName);
    want.SetElementName(natElementName.GetDeviceID(), natElementName.GetBundleName(),
        natElementName.GetAbilityName(), natElementName.GetModuleName());

    natValueString = "";
    if (UnwrapStringByPropertyName(env, param, "type", natValueString)) {
        want.SetType(natValueString);
    }

    return true;
}

napi_value WrapAbilityResult(napi_env env, const int &resultCode, const AAFwk::Want &want)
{
    HandleEscape handleEscape(env);
    napi_value jsObject = nullptr;
    napi_value jsValue = nullptr;

    NAPI_CALL(env, napi_create_object(env, &jsObject));

    jsValue = WrapInt32ToJS(env, resultCode);
    SetPropertyValueByPropertyName(env, jsObject, "resultCode", jsValue);

    jsValue = nullptr;
    jsValue = WrapWant(env, want);
    SetPropertyValueByPropertyName(env, jsObject, "want", jsValue);

    return handleEscape.Escape(jsObject);
}

bool UnWrapAbilityResult(napi_env env, napi_value param, int &resultCode, AAFwk::Want &want)
{
    if (!IsTypeForNapiValue(env, param, napi_object)) {
        return false;
    }

    if (!UnwrapInt32ByPropertyName(env, param, "resultCode", resultCode)) {
        return false;
    }

    if (IsExistsByPropertyName(env, param, "want")) {
        napi_value jsWant = nullptr;
        if (napi_get_named_property(env, param, "want", &jsWant) != napi_ok) {
            return false;
        }
        if (IsTypeForNapiValue(env, jsWant, napi_undefined)) {
            return true;
        }
        if (!UnwrapWant(env, jsWant, want)) {
            return false;
        }
    }
    return true;
}

napi_value CreateJsWant(napi_env env, const Want &want)
{
    HandleEscape handleEscape(env);
    napi_value object = nullptr;
    napi_create_object(env, &object);

    napi_set_named_property(env, object, "deviceId", CreateJsValue(env, want.GetElement().GetDeviceID()));
    napi_set_named_property(env, object, "bundleName", CreateJsValue(env, want.GetElement().GetBundleName()));
    napi_set_named_property(env, object, "abilityName", CreateJsValue(env, want.GetElement().GetAbilityName()));
    napi_set_named_property(env, object, "moduleName", CreateJsValue(env, want.GetElement().GetModuleName()));
    napi_set_named_property(env, object, "uri", CreateJsValue(env, want.GetUriString()));
    napi_set_named_property(env, object, "type", CreateJsValue(env, want.GetType()));
    napi_set_named_property(env, object, "flags", CreateJsValue(env, static_cast<int32_t>(want.GetFlags())));
    napi_set_named_property(env, object, "action", CreateJsValue(env, want.GetAction()));
    napi_set_named_property(env, object, "parameters", CreateJsWantParams(env, want.GetParams()));
    napi_set_named_property(env, object, "entities", CreateNativeArray(env, want.GetEntities()));
    return handleEscape.Escape(object);
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

bool InnerWrapJsWantParamsWantParams(
    napi_env env, napi_value object, const std::string &key, const AAFwk::WantParams &wantParams)
{
    AbilityRuntime::HandleScope handleScope(env);
    auto value = wantParams.GetParam(key);
    AAFwk::IWantParams *o = AAFwk::IWantParams::Query(value);
    if (o != nullptr) {
        AAFwk::WantParams wp = AAFwk::WantParamWrapper::Unbox(o);
        napi_value propertyValue = CreateJsWantParams(env, wp);
        napi_set_named_property(env, object, key.c_str(), propertyValue);
        return true;
    }
    return false;
}

bool WrapJsWantParamsArray(napi_env env, napi_value object, const std::string &key, sptr<AAFwk::IArray> &ao)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "key=%{public}s", key.c_str());
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
    } else if (AAFwk::Array::IsWantParamsArray(ao)) {
        TAG_LOGD(AAFwkTag::JSNAPI, "Array type is WantParams");
        return InnerWrapWantParamsArrayWantParams(env, object, key, ao);
    } else {
        TAG_LOGE(AAFwkTag::JSNAPI, "Array type unknown");
        return false;
    }
}

template<class TBase, class T, class NativeT>
bool InnerWrapWantParamsArray(napi_env env, napi_value object, const std::string &key, sptr<AAFwk::IArray> &ao)
{
    if (ao == nullptr) {
        return false;
    }
    long size = 0;
    if (ao->GetLength(size) != ERR_OK) {
        return false;
    }
    std::vector<NativeT> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            TBase *iValue = TBase::Query(iface);
            if (iValue != nullptr) {
                natArray.push_back(T::Unbox(iValue));
            }
        }
    }
    napi_set_named_property(env, object, key.c_str(), OHOS::AbilityRuntime::CreateNativeArray(env, natArray));
    return true;
}

template<class TBase, class T, class NativeT>
bool InnerWrapJsWantParams(napi_env env, napi_value object, const std::string &key, const AAFwk::WantParams &wantParams)
{
    AbilityRuntime::HandleScope handleScope(env);
    auto value = wantParams.GetParam(key);
    TBase *ao = TBase::Query(value);
    if (ao != nullptr) {
        NativeT natValue = T::Unbox(ao);
        napi_value propertyValue = OHOS::AbilityRuntime::CreateJsValue(env, natValue);
        napi_set_named_property(env, object, key.c_str(), propertyValue);
        return true;
    }
    return false;
}
}  // namespace AppExecFwk
}  // namespace OHOS
