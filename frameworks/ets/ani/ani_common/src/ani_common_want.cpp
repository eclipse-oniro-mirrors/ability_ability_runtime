/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ani_common_want.h"
#include "ani_common_cache_mgr.h"
#include "ani_common_json_util.h"
#include "ani_common_util.h"
#include "remote_object_taihe_ani.h"
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
namespace {
constexpr const char *ABILITY_WANT_CLASS_NAME = "@ohos.app.ability.Want.Want";
constexpr const char *INNER_CLASS_NAME = "ability.abilityResult.AbilityResultInner";
constexpr const char *ELEMENTNAME_CLASS_NAME = "bundleManager.ElementNameInner.ElementNameInner";
constexpr const char *RECORD_SET_NAME =
    "X{C{std.core.Numeric}C{std.core.String}C{std.core.BaseEnum}}Y:";
const int PROPERTIES_SIZE = 2;

bool InnerWrapWantParams(ani_env *env, ani_class wantCls, ani_object wantObject, const AAFwk::WantParams &wantParams)
{
    ani_ref wantParamRef = WrapWantParams(env, wantParams);
    if (wantParamRef == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "failed to WrapWantParams");
        return false;
    }
    return SetFieldRefByName(env, wantCls, wantObject, "parameters", wantParamRef);
}

bool InnerWrapWantParamsFD(ani_env *env, ani_class wantCls, ani_object wantObject, const AAFwk::WantParams &wantParams)
{
    ani_ref wantParamFDRef = WrapWantParamsFD(env, wantParams);
    if (wantParamFDRef == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "failed to WrapWantParamsFD");
        return false;
    }
    return SetFieldRefByName(env, wantCls, wantObject, "fds", wantParamFDRef);
}

bool InnerUnwrapWantParams(ani_env *env, ani_object wantObject, AAFwk::WantParams &wantParams)
{
    ani_ref wantParamRef = nullptr;
    if (!GetFieldRefByName(env, wantObject, "parameters", wantParamRef)) {
        TAG_LOGE(AAFwkTag::ANI, "failed to get want parameter");
        return false;
    }
    return UnwrapWantParams(env, wantParamRef, wantParams);
}

bool InnerCreateRecordObject(ani_env *env, ani_object &recordObject)
{
    ani_class recordCls = nullptr;
    ani_method recordCtorMethod = nullptr;
    AniCommonMethodCacheKey recordCtor = std::make_pair("<ctor>", ":");
    if (!AniCommonCacheMgr::GetCachedClassAndMethod(env, CLASSNAME_RECORD, recordCtor,
        recordCls, recordCtorMethod)) {
        TAG_LOGE(AAFwkTag::ANI, "failed to get cached class and method for record ctor");
        return false;
    }
    ani_status status = env->Object_New(recordCls, recordCtorMethod, &recordObject);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Object_New failed: %{public}d", status);
        return false;
    }
    return true;
}

bool InnerSetRecord(ani_env *env, ani_object recordObject, ani_string key, ani_object value)
{
    ani_class recordCls = nullptr;
    ani_method recordSetMethod = nullptr;
    AniCommonMethodCacheKey recordSet = std::make_pair("$_set", RECORD_SET_NAME);
    if (!AniCommonCacheMgr::GetCachedClassAndMethod(env, CLASSNAME_RECORD, recordSet,
        recordCls, recordSetMethod)) {
        TAG_LOGE(AAFwkTag::ANI, "failed to get cached class and method for record $_set");
        return false;
    }

    ani_status status = env->Object_CallMethod_Void(recordObject, recordSetMethod, key, value);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Object_CallMethod_Void failed status: %{public}d", status);
        return false;
    }
    return true;
}

bool InnerWrapWantParamsString(
    ani_env *env, ani_object recordObject, const std::string &key, const sptr<AAFwk::IInterface> &value)
{
    ani_string aniKey = GetAniString(env, key);
    if (aniKey == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "key GetAniString failed");
        return false;
    }

    AAFwk::IString *ao = AAFwk::IString::Query(value);
    if (ao == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null value");
        return false;
    }
    std::string natValue = AAFwk::String::Unbox(ao);
    ani_string aniValue = GetAniString(env, natValue);
    if (aniValue == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "value GetAniString failed");
        return false;
    }
    return InnerSetRecord(env, recordObject, aniKey, aniValue);
}

bool InnerCreateBooleanObject(ani_env *env, ani_boolean value, ani_object &object)
{
    ani_class cls = nullptr;
    ani_method ctorMethod = nullptr;
    AniCommonMethodCacheKey ctorKey = std::make_pair("<ctor>", "z:");
    if (!AniCommonCacheMgr::GetCachedClassAndMethod(env, CLASSNAME_BOOLEAN, ctorKey,
        cls, ctorMethod)) {
        return false;
    }
    ani_status status = env->Object_New(cls, ctorMethod, &object, value);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Object_New failed: %{public}d", status);
        return false;
    }
    return true;
}

bool InnerWrapWantParamsBool(
    ani_env *env, ani_object recordObject, const std::string &key, const sptr<AAFwk::IInterface> &value)
{
    ani_string aniKey = GetAniString(env, key);
    if (aniKey == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "key GetAniString failed");
        return false;
    }

    AAFwk::IBoolean *ao = AAFwk::IBoolean::Query(value);
    if (ao == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null value");
        return false;
    }
    ani_boolean natValue = AAFwk::Boolean::Unbox(ao);
    ani_object aniValue = nullptr;
    if (!InnerCreateBooleanObject(env, natValue, aniValue)) {
        TAG_LOGE(AAFwkTag::ANI, "failed to create object");
        return false;
    }
    return InnerSetRecord(env, recordObject, aniKey, aniValue);
}

bool InnerCreateByteObject(ani_env *env, ani_byte value, ani_object &object)
{
    ani_class cls = nullptr;
    ani_method ctorMethod = nullptr;
    ani_status status = env->FindClass("std.core.Byte", &cls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "FindClass failed status: %{public}d", status);
        return false;
    }
    status = env->Class_FindMethod(cls, "<ctor>", "b:", &ctorMethod);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Class_FindMethod constructor failed: %{public}d", status);
        return false;
    }
    status = env->Object_New(cls, ctorMethod, &object, value);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Object_New failed: %{public}d", status);
        return false;
    }
    return true;
}

bool InnerCreateShortObject(ani_env *env, ani_short value, ani_object &object)
{
    ani_class cls = nullptr;
    ani_method ctorMethod = nullptr;
    AniCommonMethodCacheKey ctorKey = std::make_pair("<ctor>", "s:");
    if (!AniCommonCacheMgr::GetCachedClassAndMethod(env, CLASSNAME_SHORT, ctorKey,
        cls, ctorMethod)) {
        return false;
    }
    ani_status status = env->Object_New(cls, ctorMethod, &object, value);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Object_New failed: %{public}d", status);
        return false;
    }
    return true;
}

bool InnerWrapWantParamsShort(
    ani_env *env, ani_object recordObject, const std::string &key, const sptr<AAFwk::IInterface> &value)
{
    ani_string aniKey = GetAniString(env, key);
    if (aniKey == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "key GetAniString failed");
        return false;
    }

    AAFwk::IShort *ao = AAFwk::IShort::Query(value);
    if (ao == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null value");
        return false;
    }
    ani_short natValue = AAFwk::Short::Unbox(ao);
    ani_object aniValue = nullptr;
    if (!InnerCreateShortObject(env, natValue, aniValue)) {
        TAG_LOGE(AAFwkTag::ANI, "failed to create object");
        return false;
    }
    return InnerSetRecord(env, recordObject, aniKey, aniValue);
}

bool InnerCreateIntObject(ani_env *env, ani_int value, ani_object &object)
{
    ani_class cls = nullptr;
    ani_method ctorMethod = nullptr;
    AniCommonMethodCacheKey ctorKey = std::make_pair("<ctor>", "i:");
    if (!AniCommonCacheMgr::GetCachedClassAndMethod(env, CLASSNAME_INT, ctorKey,
        cls, ctorMethod)) {
        return false;
    }
    ani_status status = env->Object_New(cls, ctorMethod, &object, value);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Object_New failed: %{public}d", status);
        return false;
    }
    return true;
}

bool InnerWrapWantParamsInt32(
    ani_env *env, ani_object recordObject, const std::string &key, const sptr<AAFwk::IInterface> &value)
{
    ani_string aniKey = GetAniString(env, key);
    if (aniKey == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "key GetAniString failed");
        return false;
    }

    AAFwk::IInteger *ao = AAFwk::IInteger::Query(value);
    if (ao == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null value");
        return false;
    }
    ani_int natValue = AAFwk::Integer::Unbox(ao);
    ani_object aniValue = nullptr;
    if (!InnerCreateIntObject(env, natValue, aniValue)) {
        TAG_LOGE(AAFwkTag::ANI, "failed to create object");
        return false;
    }
    return InnerSetRecord(env, recordObject, aniKey, aniValue);
}

bool InnerCreateLongObject(ani_env *env, ani_long value, ani_object &object)
{
    ani_class cls = nullptr;
    ani_method ctorMethod = nullptr;
    AniCommonMethodCacheKey ctorKey = std::make_pair("<ctor>", "l:");
    if (!AniCommonCacheMgr::GetCachedClassAndMethod(env, CLASSNAME_LONG, ctorKey,
        cls, ctorMethod)) {
        return false;
    }
    ani_status status = env->Object_New(cls, ctorMethod, &object, value);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Object_New failed: %{public}d", status);
        return false;
    }
    return true;
}

bool InnerWrapWantParamsInt64(
    ani_env *env, ani_object recordObject, const std::string &key, const sptr<AAFwk::IInterface> &value)
{
    ani_string aniKey = GetAniString(env, key);
    if (aniKey == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "key GetAniString failed");
        return false;
    }

    AAFwk::ILong *ao = AAFwk::ILong::Query(value);
    if (ao == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null value");
        return false;
    }
    ani_long natValue = AAFwk::Long::Unbox(ao);
    ani_object aniValue = nullptr;
    if (!InnerCreateLongObject(env, natValue, aniValue)) {
        TAG_LOGE(AAFwkTag::ANI, "failed to create object");
        return false;
    }
    return InnerSetRecord(env, recordObject, aniKey, aniValue);
}

bool InnerCreateFloatObject(ani_env *env, ani_float value, ani_object &object)
{
    ani_class cls = nullptr;
    ani_method ctorMethod = nullptr;
    AniCommonMethodCacheKey ctorKey = std::make_pair("<ctor>", "f:");
    if (!AniCommonCacheMgr::GetCachedClassAndMethod(env, CLASSNAME_FLOAT, ctorKey,
        cls, ctorMethod)) {
        return false;
    }
    ani_status status = env->Object_New(cls, ctorMethod, &object, value);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Object_New failed: %{public}d", status);
        return false;
    }
    return true;
}

bool InnerWrapWantParamsFloat(
    ani_env *env, ani_object recordObject, const std::string &key, const sptr<AAFwk::IInterface> &value)
{
    ani_string aniKey = GetAniString(env, key);
    if (aniKey == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "key GetAniString failed");
        return false;
    }

    AAFwk::IFloat *ao = AAFwk::IFloat::Query(value);
    if (ao == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null value");
        return false;
    }
    ani_float natValue = AAFwk::Float::Unbox(ao);
    ani_object aniValue = nullptr;
    if (!InnerCreateFloatObject(env, natValue, aniValue)) {
        TAG_LOGE(AAFwkTag::ANI, "failed to create object");
        return false;
    }
    return InnerSetRecord(env, recordObject, aniKey, aniValue);
}

bool InnerCreateDoubleObject(ani_env *env, ani_double value, ani_object &object)
{
    ani_class cls = nullptr;
    ani_method ctorMethod = nullptr;
    AniCommonMethodCacheKey ctorKey = std::make_pair("<ctor>", "d:");
    if (!AniCommonCacheMgr::GetCachedClassAndMethod(env, CLASSNAME_DOUBLE, ctorKey,
        cls, ctorMethod)) {
        return false;
    }
    ani_status status = env->Object_New(cls, ctorMethod, &object, value);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Object_New failed: %{public}d", status);
        return false;
    }
    return true;
}

bool InnerWrapWantParamsDouble(
    ani_env *env, ani_object recordObject, const std::string &key, const sptr<AAFwk::IInterface> &value)
{
    ani_string aniKey = GetAniString(env, key);
    if (aniKey == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "key GetAniString failed");
        return false;
    }

    AAFwk::IDouble *ao = AAFwk::IDouble::Query(value);
    if (ao == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null value");
        return false;
    }
    ani_double natValue = AAFwk::Double::Unbox(ao);
    ani_object aniValue = nullptr;
    if (!InnerCreateDoubleObject(env, natValue, aniValue)) {
        TAG_LOGE(AAFwkTag::ANI, "failed to create object");
        return false;
    }
    return InnerSetRecord(env, recordObject, aniKey, aniValue);
}

bool InnerWrapWantParamsChar(
    ani_env *env, ani_object recordObject, const std::string &key, const sptr<AAFwk::IInterface> &value)
{
    ani_string aniKey = GetAniString(env, key);
    if (aniKey == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "key GetAniString failed");
        return false;
    }

    AAFwk::IChar *ao = AAFwk::IChar::Query(value);
    if (ao == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null value");
        return false;
    }
    std::string natValue = static_cast<AAFwk::Char *>(ao)->ToString();
    ani_string aniValue = GetAniString(env, natValue);
    if (aniValue == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "value GetAniString failed");
        return false;
    }
    return InnerSetRecord(env, recordObject, aniKey, aniValue);
}

bool InnerWrapWantParamsByte(
    ani_env *env, ani_object recordObject, const std::string &key, const sptr<AAFwk::IInterface> &value)
{
    ani_string aniKey = GetAniString(env, key);
    if (aniKey == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "key GetAniString failed");
        return false;
    }

    AAFwk::IByte *ao = AAFwk::IByte::Query(value);
    if (ao == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null value");
        return false;
    }
    ani_int natValue = AAFwk::Byte::Unbox(ao);
    ani_object aniValue = nullptr;
    if (!InnerCreateIntObject(env, natValue, aniValue)) {
        TAG_LOGE(AAFwkTag::ANI, "failed to create object");
        return false;
    }
    return InnerSetRecord(env, recordObject, aniKey, aniValue);
}

bool InnerWrapWantParamsWantParams(
    ani_env *env, ani_object recordObject, const std::string &key, const sptr<AAFwk::IInterface> &value)
{
    ani_string aniKey = GetAniString(env, key);
    if (aniKey == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "key GetAniString failed");
        return false;
    }

    AAFwk::IWantParams *ao = AAFwk::IWantParams::Query(value);
    if (ao == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null value");
        return false;
    }
    AAFwk::WantParams natValue = AAFwk::WantParamWrapper::Unbox(ao);
    ani_ref aniValue = WrapWantParams(env, natValue);
    if (aniValue == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null aniValue");
        return false;
    }
    return InnerSetRecord(env, recordObject, aniKey, reinterpret_cast<ani_object>(aniValue));
}

bool InnerWrapWantParamsRemoteObject(
    ani_env *env, ani_object recordObject, const std::string &key, const sptr<AAFwk::IInterface> &value)
{
    ani_string aniKey = GetAniString(env, key);
    if (aniKey == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "key GetAniString failed");
        return false;
    }

    AAFwk::IRemoteObjectWrap *ao = AAFwk::IRemoteObjectWrap::Query(value);
    if (ao == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null value");
        return false;
    }
    auto remoteObject = AAFwk::RemoteObjectWrap::UnBox(ao);
    ani_object aniValue = ANI_ohos_rpc_CreateJsRemoteObject(env, remoteObject);
    if (aniValue == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null aniValue");
        return false;
    }
    return InnerSetRecord(env, recordObject, aniKey, aniValue);
}

bool InnerSetArrayString(ani_env *env, ani_object recordObject, ani_string aniKey,
    const std::vector<std::string> &natArray)
{
    ani_ref undefinedRef = nullptr;
    auto status = env->GetUndefined(&undefinedRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "GetUndefined failed, status: %{public}d", status);
        return false;
    }
    ani_array refArray = nullptr;
    status = env->Array_New(natArray.size(), undefinedRef, &refArray);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Array_New failed, status: %{public}d", status);
        return false;
    }

    for (size_t i = 0; i < natArray.size(); i++) {
        ani_string aniValue = GetAniString(env, natArray[i]);
        if (aniValue == nullptr) {
            TAG_LOGE(AAFwkTag::ANI, "value GetAniString failed");
            continue;
        }
        status = env->Array_Set(refArray, i, aniValue);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::ANI, "Array_Set failed, status: %{public}d", status);
        }
    }

    return InnerSetRecord(env, recordObject, aniKey, refArray);
}

bool InnerWrapWantParamsArrayString(ani_env *env, ani_object recordObject, const std::string &key,
    const sptr<AAFwk::IArray> &ao)
{
    ani_string aniKey = GetAniString(env, key);
    if (aniKey == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "key GetAniString failed");
        return false;
    }

    long size = 0;
    ErrCode code = ao->GetLength(size);
    if (code != ERR_OK) {
        TAG_LOGE(AAFwkTag::ANI, "GetLength failed, status: %{public}d", code);
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
    return InnerSetArrayString(env, recordObject, aniKey, natArray);
}

bool InnerWrapWantParamsArrayBool(ani_env *env, ani_object recordObject, const std::string &key,
    const sptr<AAFwk::IArray> &ao)
{
    ani_string aniKey = GetAniString(env, key);
    if (aniKey == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "key GetAniString failed");
        return false;
    }

    long size = 0;
    ErrCode code = ao->GetLength(size);
    if (code != ERR_OK) {
        TAG_LOGE(AAFwkTag::ANI, "GetLength failed, status: %{public}d", code);
        return false;
    }

    std::vector<ani_boolean> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            AAFwk::IBoolean *iValue = AAFwk::IBoolean::Query(iface);
            if (iValue != nullptr) {
                natArray.push_back(AAFwk::Boolean::Unbox(iValue));
            }
        }
    }
    ani_ref undefinedRef = nullptr;
    auto status = env->GetUndefined(&undefinedRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "GetUndefined failed, status: %{public}d", status);
        return false;
    }
    ani_array aniArray = nullptr;
    status = env->Array_New(natArray.size(), undefinedRef, &aniArray);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Array_New failed, status: %{public}d", status);
        return false;
    }
    auto *aniArrayBuf = reinterpret_cast<ani_boolean *>(natArray.data());

    for (size_t i = 0; i < natArray.size(); ++i) {
        ani_object booleanObj {};
        if (!InnerCreateBooleanObject(env, aniArrayBuf[i], booleanObj)) {
            return false;
        }
        status = env->Array_Set(aniArray, i, booleanObj);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::ANI, "Array_Set failed, status: %{public}d", status);
            return false;
        }
    }

    return InnerSetRecord(env, recordObject, aniKey, aniArray);
}

bool InnerWrapWantParamsArrayShort(ani_env *env, ani_object recordObject, const std::string &key,
    const sptr<AAFwk::IArray> &ao)
{
    ani_string aniKey = GetAniString(env, key);
    if (aniKey == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "key GetAniString failed");
        return false;
    }

    long size = 0;
    ErrCode code = ao->GetLength(size);
    if (code != ERR_OK) {
        TAG_LOGE(AAFwkTag::ANI, "GetLength failed, status: %{public}d", code);
        return false;
    }

    std::vector<ani_short> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            AAFwk::IShort *iValue = AAFwk::IShort::Query(iface);
            if (iValue != nullptr) {
                natArray.push_back(AAFwk::Short::Unbox(iValue));
            }
        }
    }

    ani_array aniArray = nullptr;
    ani_ref undefinedRef = nullptr;
    auto status = env->GetUndefined(&undefinedRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "GetUndefined failed, status: %{public}d", status);
        return false;
    }
    status = env->Array_New(natArray.size(), undefinedRef, &aniArray);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Array_New failed, status: %{public}d", status);
        return false;
    }

    auto *aniArrayBuf = reinterpret_cast<ani_short *>(natArray.data());

    for (size_t i = 0; i < natArray.size(); ++i) {
        ani_object shortObj {};
        if (!InnerCreateShortObject(env, aniArrayBuf[i], shortObj)) {
            return false;
        }
        status = env->Array_Set(aniArray, i, shortObj);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::ANI, "Array_Set failed, status: %{public}d", status);
            return false;
        }
    }

    return InnerSetRecord(env, recordObject, aniKey, aniArray);
}

bool InnerWrapWantParamsArrayInt32(ani_env *env, ani_object recordObject, const std::string &key,
    const sptr<AAFwk::IArray> &ao)
{
    ani_string aniKey = GetAniString(env, key);
    if (aniKey == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "key GetAniString failed");
        return false;
    }

    long size = 0;
    ErrCode code = ao->GetLength(size);
    if (code != ERR_OK) {
        TAG_LOGE(AAFwkTag::ANI, "GetLength failed, status: %{public}d", code);
        return false;
    }

    std::vector<int32_t> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            AAFwk::IInteger *iValue = AAFwk::IInteger::Query(iface);
            if (iValue != nullptr) {
                natArray.push_back(AAFwk::Integer::Unbox(iValue));
            }
        }
    }

    ani_array aniArray = nullptr;
    ani_ref undefinedRef = nullptr;
    auto status = env->GetUndefined(&undefinedRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "GetUndefined failed, status: %{public}d", status);
        return false;
    }
    status = env->Array_New(natArray.size(), undefinedRef, &aniArray);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Array_New failed, status: %{public}d", status);
        return false;
    }

    auto *aniArrayBuf = reinterpret_cast<ani_int *>(natArray.data());

    for (size_t i = 0; i < natArray.size(); ++i) {
        ani_object intObj {};
        if (!InnerCreateIntObject(env, aniArrayBuf[i], intObj)) {
            return false;
        }
        status = env->Array_Set(aniArray, i, intObj);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::ANI, "Array_Set failed, status: %{public}d", status);
            return false;
        }
    }

    return InnerSetRecord(env, recordObject, aniKey, aniArray);
}

bool InnerWrapWantParamsArrayInt64(ani_env *env, ani_object recordObject, const std::string &key,
    const sptr<AAFwk::IArray> &ao)
{
    ani_string aniKey = GetAniString(env, key);
    if (aniKey == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "key GetAniString failed");
        return false;
    }

    long size = 0;
    ErrCode code = ao->GetLength(size);
    if (code != ERR_OK) {
        TAG_LOGE(AAFwkTag::ANI, "GetLength failed, status: %{public}d", code);
        return false;
    }

    std::vector<ani_long> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            AAFwk::ILong *iValue = AAFwk::ILong::Query(iface);
            if (iValue != nullptr) {
                natArray.push_back(AAFwk::Long::Unbox(iValue));
            }
        }
    }

    ani_array aniArray = nullptr;
    ani_ref undefinedRef = nullptr;
    auto status = env->GetUndefined(&undefinedRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "GetUndefined failed, status: %{public}d", status);
        return false;
    }
    status = env->Array_New(natArray.size(), undefinedRef, &aniArray);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Array_New failed, status: %{public}d", status);
        return false;
    }

    auto *aniArrayBuf = reinterpret_cast<ani_long *>(natArray.data());

    for (size_t i = 0; i < natArray.size(); ++i) {
        ani_object longObj {};
        if (!InnerCreateLongObject(env, aniArrayBuf[i], longObj)) {
            return false;
        }
        status = env->Array_Set(aniArray, i, longObj);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::ANI, "Array_Set failed, status: %{public}d", status);
            return false;
        }
    }

    return InnerSetRecord(env, recordObject, aniKey, aniArray);
}

bool InnerWrapWantParamsArrayFloat(ani_env *env, ani_object recordObject, const std::string &key,
    const sptr<AAFwk::IArray> &ao)
{
    ani_string aniKey = GetAniString(env, key);
    if (aniKey == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "key GetAniString failed");
        return false;
    }

    long size = 0;
    ErrCode code = ao->GetLength(size);
    if (code != ERR_OK) {
        TAG_LOGE(AAFwkTag::ANI, "GetLength failed, status: %{public}d", code);
        return false;
    }

    std::vector<ani_float> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            AAFwk::IFloat *iValue = AAFwk::IFloat::Query(iface);
            if (iValue != nullptr) {
                natArray.push_back(AAFwk::Float::Unbox(iValue));
            }
        }
    }

    ani_array aniArray = nullptr;
    ani_ref undefinedRef = nullptr;
    auto status = env->GetUndefined(&undefinedRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "GetUndefined failed, status: %{public}d", status);
        return false;
    }
    status = env->Array_New(natArray.size(), undefinedRef, &aniArray);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Array_New failed, status: %{public}d", status);
        return false;
    }

    auto *aniArrayBuf = reinterpret_cast<ani_float *>(natArray.data());

    for (size_t i = 0; i < natArray.size(); ++i) {
        ani_object floatObj {};
        if (!InnerCreateFloatObject(env, aniArrayBuf[i], floatObj)) {
            return false;
        }
        status = env->Array_Set(aniArray, i, floatObj);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::ANI, "Array_Set failed, status: %{public}d", status);
            return false;
        }
    }

    return InnerSetRecord(env, recordObject, aniKey, aniArray);
}

bool InnerWrapWantParamsArrayByte(ani_env *env, ani_object recordObject, const std::string &key,
    const sptr<AAFwk::IArray> &ao)
{
    ani_string aniKey = GetAniString(env, key);
    if (aniKey == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "key GetAniString failed");
        return false;
    }

    long size = 0;
    ErrCode code = ao->GetLength(size);
    if (code != ERR_OK) {
        TAG_LOGE(AAFwkTag::ANI, "GetLength failed, status: %{public}d", code);
        return false;
    }

    std::vector<ani_byte> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            AAFwk::IByte *iValue = AAFwk::IByte::Query(iface);
            if (iValue != nullptr) {
                natArray.push_back(AAFwk::Byte::Unbox(iValue));
            }
        }
    }

    ani_array aniArray = nullptr;
    ani_ref undefinedRef = nullptr;
    auto status = env->GetUndefined(&undefinedRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "GetUndefined failed, status: %{public}d", status);
        return false;
    }
    status = env->Array_New(natArray.size(), undefinedRef, &aniArray);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Array_New failed, status: %{public}d", status);
        return false;
    }

    auto *aniArrayBuf = reinterpret_cast<ani_byte *>(natArray.data());

    for (size_t i = 0; i < natArray.size(); ++i) {
        ani_object byteObj {};
        if (!InnerCreateByteObject(env, aniArrayBuf[i], byteObj)) {
            return false;
        }
        status = env->Array_Set(aniArray, i, byteObj);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::ANI, "Array_Set failed, status: %{public}d", status);
            return false;
        }
    }

    return InnerSetRecord(env, recordObject, aniKey, aniArray);
}

bool InnerWrapWantParamsArrayChar(ani_env *env, ani_object recordObject, const std::string &key,
    const sptr<AAFwk::IArray> &ao)
{
    ani_string aniKey = GetAniString(env, key);
    if (aniKey == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "key GetAniString failed");
        return false;
    }

    long size = 0;
    ErrCode code = ao->GetLength(size);
    if (code != ERR_OK) {
        TAG_LOGE(AAFwkTag::ANI, "GetLength failed, status: %{public}d", code);
        return false;
    }

    std::vector<std::string> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            AAFwk::IChar *iValue = AAFwk::IChar::Query(iface);
            if (iValue != nullptr) {
                std::string str(static_cast<AAFwk::Char *>(iValue)->ToString());
                natArray.push_back(str);
            }
        }
    }

    return InnerSetArrayString(env, recordObject, aniKey, natArray);
}

bool InnerWrapWantParamsArrayDouble(ani_env *env, ani_object recordObject, const std::string &key,
    const sptr<AAFwk::IArray> &ao)
{
    ani_string aniKey = GetAniString(env, key);
    if (aniKey == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "key GetAniString failed");
        return false;
    }

    long size = 0;
    ErrCode code = ao->GetLength(size);
    if (code != ERR_OK) {
        TAG_LOGE(AAFwkTag::ANI, "GetLength failed, status: %{public}d", code);
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

    ani_array aniArray = nullptr;
    ani_ref undefinedRef = nullptr;
    auto status = env->GetUndefined(&undefinedRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "GetUndefined failed, status: %{public}d", status);
        return false;
    }
    status = env->Array_New(natArray.size(), undefinedRef, &aniArray);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Array_New failed, status: %{public}d", status);
        return false;
    }

    auto *aniArrayBuf = reinterpret_cast<ani_double *>(natArray.data());

    for (size_t i = 0; i < natArray.size(); ++i) {
        ani_object doubleObj {};
        if (!InnerCreateDoubleObject(env, aniArrayBuf[i], doubleObj)) {
            return false;
        }
        status = env->Array_Set(aniArray, i, doubleObj);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::ANI, "Array_Set failed, status: %{public}d", status);
            return false;
        }
    }

    return InnerSetRecord(env, recordObject, aniKey, aniArray);
}

bool InnerSetArrayObject(ani_env *env, ani_object recordObject, ani_string aniKey,
    const std::vector<AAFwk::WantParams> &natArray)
{
    ani_ref undefinedRef = nullptr;
    auto status = env->GetUndefined(&undefinedRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "GetUndefined failed, status: %{public}d", status);
        return false;
    }
    ani_array refArray = nullptr;
    status = env->Array_New(natArray.size(), undefinedRef, &refArray);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Array_New failed, status: %{public}d", status);
        return false;
    }

    for (size_t i = 0; i < natArray.size(); i++) {
        ani_ref aniValue = WrapWantParams(env, natArray[i]);
        if (aniValue == nullptr) {
            TAG_LOGE(AAFwkTag::ANI, "value GetAniString failed");
            continue;
        }
        status = env->Array_Set(refArray, i, aniValue);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::ANI, "Array_Set failed, status: %{public}d", status);
        }
    }

    return InnerSetRecord(env, recordObject, aniKey, refArray);
}

bool InnerWrapWantParamsArrayWantParams(ani_env *env, ani_object recordObject, const std::string &key,
    const sptr<AAFwk::IArray> &ao)
{
    ani_string aniKey = GetAniString(env, key);
    if (aniKey == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "key GetAniString failed");
        return false;
    }

    long size = 0;
    ErrCode code = ao->GetLength(size);
    if (code != ERR_OK) {
        TAG_LOGE(AAFwkTag::ANI, "GetLength failed, status: %{public}d", code);
        return false;
    }

    std::vector<AAFwk::WantParams> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            AAFwk::IWantParams *iValue = AAFwk::IWantParams::Query(iface);
            if (iValue != nullptr) {
                natArray.push_back(AAFwk::WantParamWrapper::Unbox(iValue));
            }
        }
    }

    return InnerSetArrayObject(env, recordObject, aniKey, natArray);
}

bool InnerWrapWantParamsArray(
    ani_env *env, ani_object recordObject, const std::string &key, const sptr<AAFwk::IInterface> &value)
{
    AAFwk::IArray *ao = AAFwk::IArray::Query(value);
    if (ao == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null value");
        return false;
    }
    sptr array(ao);
    if (array == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null array");
        return false;
    }

    if (AAFwk::Array::IsStringArray(ao)) {
        return InnerWrapWantParamsArrayString(env, recordObject, key, array);
    } else if (AAFwk::Array::IsBooleanArray(ao)) {
        return InnerWrapWantParamsArrayBool(env, recordObject, key, array);
    } else if (AAFwk::Array::IsShortArray(ao)) {
        return InnerWrapWantParamsArrayShort(env, recordObject, key, array);
    } else if (AAFwk::Array::IsIntegerArray(ao)) {
        return InnerWrapWantParamsArrayInt32(env, recordObject, key, array);
    } else if (AAFwk::Array::IsLongArray(ao)) {
        return InnerWrapWantParamsArrayInt64(env, recordObject, key, array);
    } else if (AAFwk::Array::IsFloatArray(ao)) {
        return InnerWrapWantParamsArrayFloat(env, recordObject, key, array);
    } else if (AAFwk::Array::IsByteArray(ao)) {
        return InnerWrapWantParamsArrayByte(env, recordObject, key, array);
    } else if (AAFwk::Array::IsCharArray(ao)) {
        return InnerWrapWantParamsArrayChar(env, recordObject, key, array);
    } else if (AAFwk::Array::IsDoubleArray(ao)) {
        return InnerWrapWantParamsArrayDouble(env, recordObject, key, array);
    } else if (AAFwk::Array::IsWantParamsArray(ao)) {
        return InnerWrapWantParamsArrayWantParams(env, recordObject, key, array);
    } else {
        TAG_LOGE(AAFwkTag::ANI, "unsupported array type for key: %{public}s", key.c_str());
        return false;
    }
}

}

ani_object WrapWant(ani_env *env, const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::ANI, "WrapWant called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method method = nullptr;
    ani_object object = nullptr;
    if ((status = env->FindClass(ABILITY_WANT_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return nullptr;
    }
    if (cls == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null wantCls");
        return nullptr;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_New(cls, method, &object)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return nullptr;
    }
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null object");
        return nullptr;
    }

    auto elementName = want.GetElement();
    SetFieldStringByName(env, cls, object, "deviceId", elementName.GetDeviceID());
    SetFieldStringByName(env, cls, object, "bundleName", elementName.GetBundleName());
    SetFieldStringByName(env, cls, object, "abilityName", elementName.GetAbilityName());
    SetFieldStringByName(env, cls, object, "moduleName", elementName.GetModuleName());
    SetFieldStringByName(env, cls, object, "uri", want.GetUriString());
    SetFieldStringByName(env, cls, object, "type", want.GetType());
    SetFieldIntByName(env, cls, object, "flags", want.GetFlags());
    SetFieldStringByName(env, cls, object, "action", want.GetAction());
    InnerWrapWantParams(env, cls, object, want.GetParams());
    InnerWrapWantParamsFD(env, cls, object, want.GetParams());
    SetFieldArrayStringByName(env, cls, object, "entities", want.GetEntities());

    return object;
}

ani_ref WrapWantParamsFD(ani_env *env, const AAFwk::WantParams &wantParams)
{
    auto paramList = wantParams.GetParams();
    AAFwk::WantParams fds;
    for (auto it = paramList.begin(); it != paramList.end(); it++) {
        if (AAFwk::IWantParams::Query(it->second) == nullptr) {
            TAG_LOGW(AAFwkTag::ANI, "not wantParam");
            continue;
        }
        auto value = wantParams.GetParam(it->first);
        AAFwk::IWantParams *o = AAFwk::IWantParams::Query(value);
        if (o == nullptr) {
            return nullptr;
        }
        AAFwk::WantParams wp = AAFwk::WantParamWrapper::Unbox(o);
        auto valueMap = wp.GetParams();
        if (valueMap.size() != PROPERTIES_SIZE) {
            TAG_LOGD(AAFwkTag::ANI, "not fd");
            return nullptr;
        }
        auto typeIt = valueMap.find(AAFwk::TYPE_PROPERTY);
        if (typeIt == valueMap.end()) {
            return nullptr;
        }
        AAFwk::IString *strValue = AAFwk::IString::Query(typeIt->second);
        if (strValue == nullptr) {
            return nullptr;
        }
        std::string typeString = AAFwk::String::Unbox(strValue);
        if (typeString != AAFwk::FD) {
            TAG_LOGD(AAFwkTag::ANI, "not fd");
            return nullptr;
        }
        auto valueIt = valueMap.find(AAFwk::VALUE_PROPERTY);
        if (valueIt == valueMap.end()) {
            return nullptr;
        }
        AAFwk::IInteger *intValue = AAFwk::IInteger::Query(valueIt->second);
        if (intValue == nullptr) {
            return nullptr;
        }
        fds.SetParam(it->first, intValue);
    }
    return WrapWantParams(env, fds);
}

ani_ref WrapWantParams(ani_env *env, const AAFwk::WantParams &wantParams)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return nullptr;
    }

    ani_object wantParamsRecord = nullptr;
    if (!InnerCreateRecordObject(env, wantParamsRecord)) {
        TAG_LOGE(AAFwkTag::ANI, "failed to create record object");
        return nullptr;
    }

    const std::map<std::string, sptr<AAFwk::IInterface>> &paramList = wantParams.GetParams();
    for (const auto &[first, second] : paramList) {
        if (second == nullptr) {
            continue;
        }
        if (AAFwk::IString::Query(second) != nullptr) {
            InnerWrapWantParamsString(env, wantParamsRecord, first, second);
        } else if (AAFwk::IBoolean::Query(second) != nullptr) {
            InnerWrapWantParamsBool(env, wantParamsRecord, first, second);
        } else if (AAFwk::IShort::Query(second) != nullptr) {
            InnerWrapWantParamsShort(env, wantParamsRecord, first, second);
        } else if (AAFwk::IInteger::Query(second) != nullptr) {
            InnerWrapWantParamsInt32(env, wantParamsRecord, first, second);
        } else if (AAFwk::ILong::Query(second) != nullptr) {
            InnerWrapWantParamsInt64(env, wantParamsRecord, first, second);
        } else if (AAFwk::IFloat::Query(second) != nullptr) {
            InnerWrapWantParamsFloat(env, wantParamsRecord, first, second);
        } else if (AAFwk::IDouble::Query(second) != nullptr) {
            InnerWrapWantParamsDouble(env, wantParamsRecord, first, second);
        } else if (AAFwk::IChar::Query(second) != nullptr) {
            InnerWrapWantParamsChar(env, wantParamsRecord, first, second);
        } else if (AAFwk::IByte::Query(second) != nullptr) {
            InnerWrapWantParamsByte(env, wantParamsRecord, first, second);
        } else if (AAFwk::IArray::Query(second) != nullptr) {
            InnerWrapWantParamsArray(env, wantParamsRecord, first, second);
        } else if (AAFwk::IWantParams::Query(second) != nullptr) {
            InnerWrapWantParamsWantParams(env, wantParamsRecord, first, second);
        } else if (AAFwk::IRemoteObjectWrap::Query(second) != nullptr) {
            InnerWrapWantParamsRemoteObject(env, wantParamsRecord, first, second);
        }
    }
    return wantParamsRecord;
}

bool InnerWrapWantParamsString(
    ani_env *env, ani_object object, const std::string &key, const AAFwk::WantParams &wantParams)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    auto value = wantParams.GetParam(key);
    AAFwk::IString *ao = AAFwk::IString::Query(value);
    return ao != nullptr;
}

bool UnwrapElementName(ani_env *env, ani_object param, ElementName &elementName)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    std::string deviceId;
    if (GetFieldStringByName(env, param, "deviceId", deviceId)) {
        elementName.SetDeviceID(deviceId);
    }

    std::string bundleName;
    if (GetFieldStringByName(env, param, "bundleName", bundleName)) {
        elementName.SetBundleName(bundleName);
    }

    std::string abilityName;
    if (GetFieldStringByName(env, param, "abilityName", abilityName)) {
        elementName.SetAbilityName(abilityName);
    }

    std::string moduleName;
    if (GetFieldStringByName(env, param, "moduleName", moduleName)) {
        elementName.SetModuleName(moduleName);
    }
    return true;
}

bool UnwrapWant(ani_env *env, ani_object param, AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::ANI, "UnwrapWant called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    AAFwk::WantParams wantParams;
    if (InnerUnwrapWantParams(env, param, wantParams)) {
        want.SetParams(wantParams);
    }
    std::string action;
    if (GetFieldStringByName(env, param, "action", action)) {
        TAG_LOGD(AAFwkTag::ANI, "action %{public}s", action.c_str());
        want.SetAction(action);
    }

    std::string uri = "";
    if (GetFieldStringByName(env, param, "uri", uri)) {
        TAG_LOGD(AAFwkTag::ANI, "uri %{public}s", uri.c_str());
        want.SetUri(uri);
    }

    int32_t flags = 0;
    if (GetFieldIntByName(env, param, "flags", flags)) {
        TAG_LOGD(AAFwkTag::ANI, "flags %{public}d", flags);
        want.SetFlags(flags);
    }

    std::string type = "";
    if (GetFieldStringByName(env, param, "type", type)) {
        TAG_LOGD(AAFwkTag::ANI, "type %{public}s", type.c_str());
        want.SetType(type);
    }

    ElementName natElementName;
    UnwrapElementName(env, param, natElementName);
    want.SetElementName(natElementName.GetDeviceID(), natElementName.GetBundleName(), natElementName.GetAbilityName(),
        natElementName.GetModuleName());

    std::vector<std::string> valueStringList;
    if (GetFieldStringArrayByName(env, param, "entities", valueStringList)) {
        for (size_t i = 0; i < valueStringList.size(); i++) {
            want.AddEntity(valueStringList[i]);
        }
    }
    return true;
}

bool UnwrapWantParams(ani_env *env, ani_ref param, AAFwk::WantParams &wantParams)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_class cls = nullptr;
    ani_static_method unwrapRecordMethod = nullptr;
    AniCommonMethodCacheKey unwrapKey = std::make_pair("unwrapRecordNoThrow", nullptr);
    if (!AniCommonCacheMgr::GetCachedClassAndStaticMethod(env, TOOL_CLASS_NAME, unwrapKey,
        cls, unwrapRecordMethod)) {
        TAG_LOGE(AAFwkTag::ANI, "failed to get cached class and static method");
        return false;
    }
    ani_boolean isSuccess;
    ani_long wantParamsLong = reinterpret_cast<ani_long>(&wantParams);
    ani_status status = env->Class_CallStaticMethod_Boolean(cls, unwrapRecordMethod, &isSuccess, param, wantParamsLong);
    if (status != ANI_OK || isSuccess != ANI_TRUE) {
        TAG_LOGE(AAFwkTag::ANI, "failed to call unwrapRecordNoThrow method, status: %{public}d", status);
        return false;
    }
    return true;
}

bool GetAbilityResultClass(ani_env *env, ani_class &cls)
{
    ani_status status = env->FindClass(INNER_CLASS_NAME, &cls);
    if (status != ANI_OK || cls == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    return true;
}

bool GetResultCode(ani_env *env, ani_object param, ani_class cls, int &resultCode)
{
    ani_method method = nullptr;
    ani_status status = env->Class_FindMethod(cls, "<get>resultCode", nullptr, &method);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    ani_int iResultCode = 0;
    status = env->Object_CallMethod_Int(param, method, &iResultCode);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    resultCode = static_cast<int>(iResultCode);
    return true;
}

bool GetWantReference(ani_env *env, ani_object param, ani_class cls, ani_ref &wantRef)
{
    ani_method method {};
    ani_status status = env->Class_FindMethod(cls, "<get>want", nullptr, &method);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    status = env->Object_CallMethod_Ref(param, method, &wantRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    ani_boolean isUndefined = ANI_TRUE;
    status = env->Reference_IsUndefined(wantRef, &isUndefined);
    if (status != ANI_OK || isUndefined) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    return true;
}

bool UnWrapAbilityResult(ani_env *env, ani_object param, int &resultCode, AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::ANI, "called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_class cls = nullptr;
    if (!GetAbilityResultClass(env, cls)) {
        return false;
    }
    if (!GetResultCode(env, param, cls, resultCode)) {
        return false;
    }
    ani_ref wantRef = nullptr;
    if (!GetWantReference(env, param, cls, wantRef)) {
        return false;
    }
    return UnwrapWant(env, reinterpret_cast<ani_object>(wantRef), want);
}

ani_object WrapElementName(ani_env *env, const AppExecFwk::ElementName &elementNameParam)
{
    TAG_LOGD(AAFwkTag::ANI, "WrapElementName");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return nullptr;
    }
    ani_class elementNameObj = nullptr;
    ani_status status = ANI_ERROR;
    ani_method method = nullptr;
    ani_object object = nullptr;
    if ((status = env->FindClass(ELEMENTNAME_CLASS_NAME, &elementNameObj)) != ANI_OK || elementNameObj == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "FindClass status: %{public}d or null elementNameObj", status);
        return nullptr;
    }
    if ((status = env->Class_FindMethod(elementNameObj, "<ctor>", ":", &method)) != ANI_OK || method == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "Class_FindMethod status: %{public}d or null method", status);
        return nullptr;
    }
    if ((status = env->Object_New(elementNameObj, method, &object)) != ANI_OK || object == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "Object_New status: %{public}d or null object", status);
        return nullptr;
    }
    return WrapElementNameInner(env, elementNameObj, object, elementNameParam);
}

ani_object WrapElementNameInner(ani_env *env, ani_class elementNameObj, ani_object object,
    const AppExecFwk::ElementName &elementNameParam)
{
    TAG_LOGD(AAFwkTag::ANI, "WrapElementNameInner");
    if (env == nullptr || elementNameObj == nullptr || object == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "invalid args");
        return nullptr;
    }
    if (!SetFieldStringByName(env, elementNameObj, object, "bundleName", elementNameParam.GetBundleName())) {
        TAG_LOGE(AAFwkTag::ANI, "set bundleName failed");
        return nullptr;
    }
    if (!SetFieldStringByName(env, elementNameObj, object, "abilityName", elementNameParam.GetAbilityName())) {
        TAG_LOGE(AAFwkTag::ANI, "set abilityName failed");
        return nullptr;
    }
    if (!SetFieldStringByName(env, elementNameObj, object, "deviceId", elementNameParam.GetDeviceID())) {
        TAG_LOGE(AAFwkTag::ANI, "set deviceId failed");
    }
    if (!SetFieldStringByName(env, elementNameObj, object, "moduleName", elementNameParam.GetModuleName())) {
        TAG_LOGE(AAFwkTag::ANI, "set moduleName failed");
    }
    return object;
}

ani_object CreateAniWant(ani_env *env, const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::ANI, "CreateAniWant called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method method = nullptr;
    ani_object object = nullptr;
    if ((status = env->FindClass(ABILITY_WANT_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return nullptr;
    }
    if (cls == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null wantCls");
        return nullptr;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_New(cls, method, &object)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return nullptr;
    }
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null object");
        return nullptr;
    }

    auto elementName = want.GetElement();
    SetFieldStringByName(env, cls, object, "deviceId", elementName.GetDeviceID());
    SetFieldStringByName(env, cls, object, "bundleName", elementName.GetBundleName());
    SetFieldStringByName(env, cls, object, "abilityName", elementName.GetAbilityName());
    SetFieldStringByName(env, cls, object, "moduleName", elementName.GetModuleName());
    SetFieldStringByName(env, cls, object, "uri", want.GetUriString());
    SetFieldStringByName(env, cls, object, "type", want.GetType());
    SetFieldIntByName(env, cls, object, "flags", want.GetFlags());
    SetFieldStringByName(env, cls, object, "action", want.GetAction());
    InnerWrapWantParams(env, cls, object, want.GetParams());
    SetFieldArrayStringByName(env, cls, object, "entities", want.GetEntities());

    return object;
}

} // namespace AppExecFwk
} // namespace OHOS
