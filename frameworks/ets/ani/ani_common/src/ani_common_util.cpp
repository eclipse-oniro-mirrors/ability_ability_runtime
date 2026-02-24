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
#include <cstring>

#include "ani_common_util.h"

#include "ani_enum_convert.h"
#include "running_process_info.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "securec.h"
#include "tokenid_kit.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr const char* CLASSNAME_DOUBLE = "std.core.Double";
constexpr const char* CLASSNAME_BOOL = "std.core.Boolean";
constexpr const char* CLASSNAME_ARRAY = "std.core.Array";
constexpr const char* CLASSNAME_LONG = "std.core.Long";
constexpr const char* CLASSNAME_INT = "std.core.Int";
constexpr const char* CLASSNAME_ASYNC_CALLBACK_WRAPPER = "utils.AbilityUtils.AsyncCallbackWrapper";
constexpr const char* SET_OBJECT_VOID_SIGNATURE = "iY:";
constexpr const char* CLASSNAME_INNER = "application.ProcessInformation.ProcessInformationInner";
constexpr const char* ENUMNAME_PROCESS = "@ohos.app.ability.appManager.appManager.ProcessState";
constexpr const char* ENUMNAME_BUNDLE = "@ohos.bundle.bundleManager.bundleManager.BundleType";
}

bool GetFieldDoubleByName(ani_env *env, ani_object object, const char *name, double &value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_status status = ANI_ERROR;
    ani_ref field = nullptr;
    if ((status = env->Object_GetFieldByName_Ref(object, name, &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    ani_boolean isUndefined = ANI_TRUE;
    if ((status = env->Reference_IsUndefined(field, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    if (isUndefined) {
        TAG_LOGE(AAFwkTag::ANI, "%{public}s: undefined", name);
        return false;
    }
    ani_double aniValue = 0.0;
    if ((status = env->Object_CallMethodByName_Double(
        reinterpret_cast<ani_object>(field), "toDouble", nullptr, &aniValue)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    value = static_cast<double>(aniValue);
    return true;
}

bool SetFieldDoubleByName(ani_env *env, ani_class cls, ani_object object, const char *name, double value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_status status = ANI_ERROR;
    ani_field field = nullptr;
    if ((status = env->Class_FindField(cls, name, &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    ani_object obj = CreateDouble(env, value);
    if (obj == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "CreateDouble failed");
        return false;
    }
    if ((status = env->Object_SetField_Ref(object, field, reinterpret_cast<ani_ref>(obj))) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    return true;
}

bool GetFieldBoolByName(ani_env *env, ani_object object, const char *name, bool &value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_status status = ANI_ERROR;
    ani_ref field = nullptr;
    if ((status = env->Object_GetFieldByName_Ref(object, name, &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    ani_boolean isUndefined = ANI_TRUE;
    if ((status = env->Reference_IsUndefined(field, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    if (isUndefined) {
        TAG_LOGE(AAFwkTag::ANI, "%{public}s: undefined", name);
        return false;
    }
    ani_boolean aniValue = ANI_FALSE;
    if ((status = env->Object_CallMethodByName_Boolean(
        reinterpret_cast<ani_object>(field), "toBoolean", nullptr, &aniValue)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    value = static_cast<bool>(aniValue);
    return true;
}

bool SetFieldBoolByName(ani_env *env, ani_class cls, ani_object object, const char *name, bool value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_status status = ANI_ERROR;
    ani_field field = nullptr;
    if ((status = env->Class_FindField(cls, name, &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    if ((status = env->Object_SetField_Boolean(object, field, value)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    return true;
}

bool GetFieldStringByName(ani_env *env, ani_object object, const char *name, std::string &value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_status status = ANI_ERROR;
    ani_ref field = nullptr;
    if ((status = env->Object_GetFieldByName_Ref(object, name, &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    ani_boolean isUndefined = ANI_TRUE;
    if ((status = env->Reference_IsUndefined(field, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    if (isUndefined) {
        TAG_LOGE(AAFwkTag::ANI, "%{public}s: undefined", name);
        return false;
    }
    if (!GetStdString(env, reinterpret_cast<ani_string>(field), value)) {
        TAG_LOGE(AAFwkTag::ANI, "GetStdString failed");
        return false;
    }
    return true;
}

bool SetFieldStringByName(ani_env *env, ani_class cls, ani_object object, const char *name,
    const std::string &value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_status status = ANI_ERROR;
    ani_field field = nullptr;
    if ((status = env->Class_FindField(cls, name, &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    ani_string aniStr = nullptr;
    if ((status = env->String_NewUTF8(value.c_str(), value.size(), &aniStr)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    if ((status = env->Object_SetField_Ref(object, field, aniStr)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    return true;
}

bool GetFieldIntByName(ani_env *env, ani_object object, const char *name, int32_t &value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_status status = ANI_ERROR;
    ani_ref field = nullptr;
    if ((status = env->Object_GetFieldByName_Ref(object, name, &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    ani_boolean isUndefined = ANI_TRUE;
    if ((status = env->Reference_IsUndefined(field, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    if (isUndefined) {
        TAG_LOGE(AAFwkTag::ANI, "%{public}s: undefined", name);
        return false;
    }
    ani_int aniInt = 0;
    if ((status = env->Object_CallMethodByName_Int(
        reinterpret_cast<ani_object>(field), "toInt", nullptr, &aniInt)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    value = static_cast<int>(aniInt);
    return true;
}

bool SetFieldIntByName(ani_env *env, ani_class cls, ani_object object, const char *name, int32_t value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_status status = ANI_ERROR;
    ani_field field = nullptr;
    if ((status = env->Class_FindField(cls, name, &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    ani_object obj = CreateInt(env, value);
    if (obj == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "CreateInt failed");
        return false;
    }
    if ((status = env->Object_SetField_Ref(object, field, reinterpret_cast<ani_ref>(obj))) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    return true;
}

bool GetFieldLongByName(ani_env *env, ani_object object, const char *name, int64_t &value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_status status = ANI_ERROR;
    ani_ref field = nullptr;
    if ((status = env->Object_GetFieldByName_Ref(object, name, &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    ani_boolean isUndefined = ANI_TRUE;
    if ((status = env->Reference_IsUndefined(field, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    if (isUndefined) {
        TAG_LOGE(AAFwkTag::ANI, "%{public}s: undefined", name);
        return false;
    }
    ani_long aniLong = 0;
    if ((status = env->Object_CallMethodByName_Long(
        reinterpret_cast<ani_object>(field), "toLong", nullptr, &aniLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    value = static_cast<int64_t>(aniLong);
    return true;
}

bool SetFieldLongByName(ani_env *env, ani_class cls, ani_object object, const char *name, int64_t value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_status status = ANI_ERROR;
    ani_field field = nullptr;
    if ((status = env->Class_FindField(cls, name, &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    ani_object obj = CreateLong(env, value);
    if (obj == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "CreateLong failed");
        return false;
    }
    if ((status = env->Object_SetField_Ref(object, field, reinterpret_cast<ani_ref>(obj))) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    return true;
}

bool GetFieldStringArrayByName(ani_env *env, ani_object object, const char *name, std::vector<std::string> &value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_status status = ANI_ERROR;
    ani_ref arrayObj = nullptr;
    if ((status = env->Object_GetFieldByName_Ref(object, name, &arrayObj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    ani_boolean isUndefined = ANI_TRUE;
    if ((status = env->Reference_IsUndefined(arrayObj, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    if (isUndefined) {
        TAG_LOGE(AAFwkTag::ANI, "%{public}s: undefined", name);
        return false;
    }
    ani_int length = 0;
    if ((status = env->Object_GetPropertyByName_Int(reinterpret_cast<ani_object>(arrayObj),
        "length", &length)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    for (int i = 0; i < static_cast<int>(length); i++) {
        ani_ref stringEntryRef;
        status = env->Object_CallMethodByName_Ref(reinterpret_cast<ani_object>(arrayObj),
            "$_get", "i:Y", &stringEntryRef, (ani_int)i);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::ANI, "status: %{public}d, index: %{public}d", status, i);
            return false;
        }
        std::string str = "";
        if (!GetStdString(env, reinterpret_cast<ani_string>(stringEntryRef), str)) {
            TAG_LOGE(AAFwkTag::ANI, "GetStdString failed, index: %{public}d", i);
            return false;
        }
        value.emplace_back(str);
        TAG_LOGD(AAFwkTag::ANI, "GetStdString index: %{public}d %{public}s", i, str.c_str());
    }
    return true;
}

bool SetFieldArrayStringByName(ani_env *env, ani_class cls, ani_object object, const char *name,
    const std::vector<std::string> &value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_status status = ANI_ERROR;
    ani_field field = nullptr;
    if ((status = env->Class_FindField(cls, name, &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    ani_class arrayCls = nullptr;
    if ((status = env->FindClass(CLASSNAME_ARRAY, &arrayCls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    ani_method arrayCtor = nullptr;
    if ((status = env->Class_FindMethod(arrayCls, "<ctor>", "i:", &arrayCtor)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    ani_object arrayObj = nullptr;
    if ((status = env->Object_New(arrayCls, arrayCtor, &arrayObj, value.size())) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    for (size_t i = 0; i < value.size(); i++) {
        ani_string str = nullptr;
        if ((status = env->String_NewUTF8(value[i].c_str(), value[i].size(), &str)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
            return false;
        }
        if ((status = env->Object_CallMethodByName_Void(arrayObj, "$_set", "iY:",
            i, str)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
            return false;
        }
    }
    if ((status = env->Object_SetField_Ref(object, field, arrayObj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    return true;
}

bool GetFieldRefByName(ani_env *env, ani_object object, const char *name, ani_ref &ref)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_status status = ANI_ERROR;
    if ((status = env->Object_GetFieldByName_Ref(object, name, &ref)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    ani_boolean isUndefined = ANI_TRUE;
    if ((status = env->Reference_IsUndefined(ref, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    if (isUndefined) {
        TAG_LOGE(AAFwkTag::ANI, "%{public}s is undefined", name);
        return false;
    }
    return true;
}

bool SetFieldRefByName(ani_env *env, ani_class cls, ani_object object, const char *name, ani_ref value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_status status = ANI_ERROR;
    ani_field field = nullptr;
    if ((status = env->Class_FindField(cls, name, &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    if ((status = env->Object_SetField_Ref(object, field, value)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    return true;
}

bool GetStdString(ani_env *env, ani_string str, std::string &value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_status status = ANI_ERROR;
    ani_size sz = 0;
    if ((status = env->String_GetUTF8Size(str, &sz)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    value.resize(sz + 1);
    if ((status = env->String_GetUTF8SubString(str, 0, sz, value.data(), value.size(), &sz)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    value.resize(sz);
    return true;
}

ani_string GetAniString(ani_env *env, const std::string &str)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return nullptr;
    }
    ani_string aniStr = nullptr;
    ani_status status = env->String_NewUTF8(str.c_str(), str.size(), &aniStr);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return nullptr;
    }
    return aniStr;
}

ani_object CreateDouble(ani_env *env, ani_double value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return nullptr;
    }
    ani_status status = ANI_ERROR;
    ani_class cls = nullptr;
    if ((status = env->FindClass(CLASSNAME_DOUBLE, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return nullptr;
    }
    ani_method ctor = nullptr;
    if ((status = env->Class_FindMethod(cls, "<ctor>", "d:", &ctor)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return nullptr;
    }
    ani_object obj = nullptr;
    if ((status = env->Object_New(cls, ctor, &obj, value)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return nullptr;
    }
    return obj;
}

ani_object CreateBoolean(ani_env *env, ani_boolean value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return nullptr;
    }
    ani_status status = ANI_ERROR;
    ani_class cls = nullptr;
    if ((status = env->FindClass(CLASSNAME_BOOL, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return nullptr;
    }
    ani_method ctor = nullptr;
    if ((status = env->Class_FindMethod(cls, "<ctor>", "z:", &ctor)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return nullptr;
    }
    ani_object obj = nullptr;
    if ((status = env->Object_New(cls, ctor, &obj, value)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return nullptr;
    }
    return obj;
}

ani_object CreateEtsNull(ani_env *env)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return nullptr;
    }
    ani_ref ref = nullptr;
    env->GetNull(&ref);
    return reinterpret_cast<ani_object>(ref);
}

ani_object CreateLong(ani_env *env, ani_long value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_status status = env->FindClass(CLASSNAME_LONG, &cls);
    if (status != ANI_OK || cls == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "FindClass status: %{public}d, or null class", status);
        return nullptr;
    }
    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(cls, "<ctor>", "l:", &method)) != ANI_OK || method == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "Class_FindMethod status: %{public}d, or null method", status);
        return nullptr;
    }
    ani_object object = nullptr;
    if ((status = env->Object_New(cls, method, &object, value)) != ANI_OK || object == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "Object_New status: %{public}d, or null object", status);
        return nullptr;
    }
    return object;
}

bool AsyncCallback(ani_env *env, ani_object call, ani_object error, ani_object result)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_class clsCall = nullptr;
    ani_status status = env->FindClass(CLASSNAME_ASYNC_CALLBACK_WRAPPER, &clsCall);
    if (status!= ANI_OK || clsCall == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "FindClass status: %{public}d, or null clsCall", status);
        return false;
    }
    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(clsCall, "invoke", nullptr, &method)) != ANI_OK || method == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "Class_FindMethod status: %{public}d, or null method", status);
        return false;
    }
    if (error == nullptr) {
        ani_ref nullRef = nullptr;
        env->GetNull(&nullRef);
        error = reinterpret_cast<ani_object>(nullRef);
    }
    if (result == nullptr) {
        ani_ref undefinedRef = nullptr;
        env->GetUndefined(&undefinedRef);
        result = reinterpret_cast<ani_object>(undefinedRef);
    }
    if ((status = env->Object_CallMethod_Void(call, method, error, result)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Object_CallMethod_Void status: %{public}d", status);
        return false;
    }
    return true;
}

bool AsyncCallback(ani_env *env, const char *signature, ani_object call, ani_object error, ani_object result)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_class clsCall = nullptr;
    ani_status status = env->FindClass(signature, &clsCall);
    if (status!= ANI_OK || clsCall == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "FindClass status: %{public}d, or null clsCall", status);
        return false;
    }
    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(clsCall, "invoke", nullptr, &method)) != ANI_OK || method == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "Class_FindMethod status: %{public}d, or null method", status);
        return false;
    }
    if (error == nullptr) {
        ani_ref nullRef = nullptr;
        env->GetNull(&nullRef);
        error = reinterpret_cast<ani_object>(nullRef);
    }
    if (result == nullptr) {
        ani_ref undefinedRef = nullptr;
        env->GetUndefined(&undefinedRef);
        result = reinterpret_cast<ani_object>(undefinedRef);
    }
    if ((status = env->Object_CallMethod_Void(call, method, error, result)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Object_CallMethod_Void status: %{public}d", status);
        return false;
    }
    return true;
}

bool GetDoubleOrUndefined(ani_env *env, ani_object param, const char *name, ani_double &value)
{
    ani_ref obj = nullptr;
    ani_boolean isUndefined = true;
    ani_status status = ANI_ERROR;

    if  (env == nullptr || param == nullptr || name == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env or param or name");
        return false;
    }

    if ((status = env->Object_GetFieldByName_Ref(param, name, &obj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status : %{public}d", status);
        return false;
    }
    if ((status = env->Reference_IsUndefined(obj, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status : %{public}d", status);
        return false;
    }
    if (isUndefined) {
        TAG_LOGE(AAFwkTag::ANI, "%{public}s : undefined", name);
        return false;
    }
    if ((status = env->Object_CallMethodByName_Double(
        reinterpret_cast<ani_object>(obj), "toDouble", nullptr, &value)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status : %{public}d", status);
        return false;
    }
    return true;
}

bool GetStringOrUndefined(ani_env *env, ani_object param, const char *name, std::string &res)
{
    ani_ref obj = nullptr;
    ani_boolean isUndefined = true;
    ani_status status = ANI_ERROR;

    if (env == nullptr || param == nullptr || name == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env or param or name");
        return false;
    }

    if ((status = env->Object_GetFieldByName_Ref(param, name, &obj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status : %{public}d", status);
        return false;
    }
    if ((status = env->Reference_IsUndefined(obj, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status : %{public}d", status);
        return false;
    }
    if (isUndefined) {
        TAG_LOGE(AAFwkTag::ANI, "%{public}s : undefined", name);
        return false;
    }
    if (!GetStdString(env, reinterpret_cast<ani_string>(obj), res)) {
        TAG_LOGE(AAFwkTag::ANI, "GetStdString failed");
        return false;
    }
    return true;
}

bool GetPropertyRef(ani_env *env, ani_object obj, const char *name, ani_ref &ref, ani_boolean &isUndefined)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_status status = env->Object_GetPropertyByName_Ref(obj, name, &ref);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Failed to get property '%{public}s', status: %{public}d", name, status);
        return false;
    }
    status = env->Reference_IsUndefined(ref, &isUndefined);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Failed to check undefined for '%{public}s', status: %{public}d", name, status);
        return false;
    }
    return true;
}

ani_object CreateInt(ani_env *env, ani_int value)
{
    ani_class cls;
    ani_status status = ANI_ERROR;
    if ((status = env->FindClass(CLASSNAME_INT, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "FindClass status : %{public}d", status);
        return nullptr;
    }
    ani_method ctor;
    if ((status = env->Class_FindMethod(cls, "<ctor>", "i:", &ctor)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Class_FindMethod status : %{public}d", status);
        return nullptr;
    }
    ani_object object;
    if ((status = env->Object_New(cls, ctor, &object, value)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Object_New status : %{public}d", status);
        return nullptr;
    }
    return object;
}

bool SetOptionalFieldInt(ani_env *env, ani_class cls, ani_object object, const std::string &fieldName, int value)
{
    ani_field field = nullptr;
    ani_status status = env->Class_FindField(cls, fieldName.c_str(), &field);
    if (status != ANI_OK || field == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "Class_FindField failed or null field, status=%{public}d, fieldName=%{public}s",
            status, fieldName.c_str());
        return false;
    }
    ani_object intObj = CreateInt(env, value);
    if (intObj == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null intObj");
        return false;
    }
    status = env->Object_SetField_Ref(object, field, intObj);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Object_SetField_Ref failed, status=%{public}d, fieldName=%{public}s",
            status, fieldName.c_str());
        return false;
    }
    return true;
}

bool IsExistsField(ani_env *env, ani_object param, const char *name)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }

    ani_ref resRef = nullptr;
    ani_status status;
    ani_boolean isUndefined = true;

    if ((status = env->Object_GetFieldByName_Ref(param, name, &resRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    if ((status = env->Reference_IsUndefined(resRef, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    if (isUndefined) {
        return false;
    }
    return true;
}

bool WrapArrayString(ani_env *env, ani_object &arrayObj, const std::vector<std::string> &values)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "env null or arrayObj null");
        return false;
    }
    ani_class arrayCls = nullptr;
    ani_method arrayCtor;
    ani_string aniStr = nullptr;
    ani_status status = ANI_ERROR;
    status = env->FindClass(CLASSNAME_ARRAY, &arrayCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    status = env->Class_FindMethod(arrayCls, "<ctor>", "i:", &arrayCtor);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    status = env->Object_New(arrayCls, arrayCtor, &arrayObj, values.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }

    for (size_t i = 0; i < values.size(); i++) {
        aniStr = nullptr;
        status = env->String_NewUTF8(values[i].c_str(), values[i].size(), &aniStr);
        if (aniStr == nullptr) {
            TAG_LOGE(AAFwkTag::ANI, "null aniStr");
            return false;
        }
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
            return false;
        }
        status = env->Object_CallMethodByName_Void(arrayObj, "$_set", SET_OBJECT_VOID_SIGNATURE, i, aniStr);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
            return false;
        }
    }
    return true;
}

bool UnwrapArrayString(ani_env *env, const ani_object &arrayObj, std::vector<std::string> &stringList)
{
    if (env == nullptr || arrayObj == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "env null or arrayObj null");
        return false;
    }
    stringList.clear();
    ani_size size = 0;
    ani_status status = ANI_ERROR;
    if ((status = env->Array_GetLength(reinterpret_cast<ani_array>(arrayObj), &size)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    ani_ref ref;
    ani_size idx;
    for (idx = 0; idx < size; idx++) {
        if ((status = env->Array_Get(reinterpret_cast<ani_array>(arrayObj), idx, &ref)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::ANI, "status: %{public}d, index: %{public}zu", status, idx);
            return false;
        }
        if (ref == nullptr) {
            TAG_LOGE(AAFwkTag::ANI, "null ref");
            return false;
        }
        std::string str = "";
        if (!OHOS::AppExecFwk::GetStdString(env, reinterpret_cast<ani_string>(ref), str)) {
            TAG_LOGE(AAFwkTag::ANI, "GetStdString failed, index: %{public}zu", idx);
            return false;
        }
        stringList.push_back(str);
    }
    return true;
}

bool SetProcessInformation(ani_env *env, ani_object object, const AppExecFwk::RunningProcessInfo &processInfo)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_status status = ANI_OK;
    if ((status = env->Object_SetPropertyByName_Int(object, "pid", processInfo.pid_)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "pid failed status:%{public}d", status);
        return false;
    }
    if ((status = env->Object_SetPropertyByName_Int(object, "uid", processInfo.uid_)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "uid failed status:%{public}d", status);
        return false;
    }
    status = env->Object_SetPropertyByName_Ref(object, "processName",
        AppExecFwk::GetAniString(env, processInfo.processName_));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "processName failed status:%{public}d", status);
        return false;
    }
    ani_object arrayObj = nullptr;
    WrapArrayString(env, arrayObj, processInfo.bundleNames);
    status = env->Object_SetPropertyByName_Ref(object, "bundleNames", arrayObj);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "bundleNames failed status:%{public}d", status);
        return false;
    }
    ani_enum_item stateItem {};
    AAFwk::AniEnumConvertUtil::EnumConvert_NativeToEts(env,
        ENUMNAME_PROCESS, processInfo.state_, stateItem);
    if ((status = env->Object_SetPropertyByName_Ref(object, "state", stateItem)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "state failed status:%{public}d", status);
        return false;
    }
    ani_enum_item bundleTypeItem {};
    AAFwk::AniEnumConvertUtil::EnumConvert_NativeToEts(env,
        ENUMNAME_BUNDLE,
        processInfo.bundleType, bundleTypeItem);
    if ((status = env->Object_SetPropertyByName_Ref(object, "bundleType", bundleTypeItem)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "bundleType failed status:%{public}d", status);
        return false;
    }
    if (processInfo.appCloneIndex != -1 &&
        (status = env->Object_SetPropertyByName_Ref(
            object, "appCloneIndex", CreateInt(env, processInfo.appCloneIndex))) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "appCloneIndex failed status:%{public}d", status);
        return false;
    }
    return true;
}

ani_object WrapProcessInformation(ani_env *env, const AppExecFwk::RunningProcessInfo &processInfo)
{
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method method {};
    ani_object object = nullptr;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return nullptr;
    }
    if ((status = env->FindClass(CLASSNAME_INNER, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "FindClass failed status: %{public}d", status);
        return nullptr;
    }
    if (cls == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null cls");
        return nullptr;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "find ctor failed status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_New(cls, method, &object)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Object_New ProcessInformationInner failed status: %{public}d", status);
        return nullptr;
    }
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null object");
        return nullptr;
    }
    if (!SetProcessInformation(env, object, processInfo)) {
        TAG_LOGE(AAFwkTag::ANI, "SetProcessInformation failed");
        return nullptr;
    }
    return object;
}

ani_object CreateRunningProcessInfoArray(ani_env *env, std::vector<AppExecFwk::RunningProcessInfo> infos)
{
    ani_class arrayCls = nullptr;
    ani_status status = ANI_OK;

    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return nullptr;
    }

    status = env->FindClass(CLASSNAME_ARRAY, &arrayCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "FindClass failed status: %{public}d", status);
        return nullptr;
    }

    ani_method arrayCtor;
    status = env->Class_FindMethod(arrayCls, "<ctor>", "i:", &arrayCtor);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "find ctor failed status: %{public}d", status);
        return nullptr;
    }

    ani_object arrayObj;
    status = env->Object_New(arrayCls, arrayCtor, &arrayObj, infos.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Object_New array status: %{public}d", status);
        return arrayObj;
    }
    ani_size index = 0;
    for (auto &processInfo : infos) {
        ani_object aniInfo = WrapProcessInformation(env, processInfo);
        if (aniInfo == nullptr) {
            TAG_LOGW(AAFwkTag::ANI, "null aniInfo");
            break;
        }
        status = env->Object_CallMethodByName_Void(arrayObj, "$_set", "iY:", index, aniInfo);
        if (status != ANI_OK) {
            TAG_LOGW(AAFwkTag::ANI, "Object_CallMethodByName_Void failed status: %{public}d", status);
            break;
        }
        index++;
    }
    return arrayObj;
}

ani_object CreateEmptyArray(ani_env *env)
{
    ani_status status = ANI_OK;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return nullptr;
    }
    ani_class arrayCls = nullptr;
    status = env->FindClass(CLASSNAME_ARRAY, &arrayCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "FindClass failed status: %{public}d", status);
        return nullptr;
    }
    ani_method arrayCtor;
    status = env->Class_FindMethod(arrayCls, "<ctor>", ":", &arrayCtor);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "find ctor failed status: %{public}d", status);
        return nullptr;
    }
    ani_object arrayObj;
    status = env->Object_New(arrayCls, arrayCtor, &arrayObj);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Object_New array failed status: %{public}d", status);
        return nullptr;
    }
    return arrayObj;
}

bool IsExistsProperty(ani_env *env, ani_object param, const char *name)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }

    ani_ref resRef = nullptr;
    ani_status status = ANI_ERROR;
    ani_boolean isUndefined = true;

    if ((status = env->Object_GetPropertyByName_Ref(param, name, &resRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    if ((status = env->Reference_IsUndefined(resRef, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    return !isUndefined;
}

bool GetStringProperty(ani_env *env, ani_object param, const char *name, std::string &value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }

    ani_ref obj = nullptr;
    ani_boolean isUndefined = true;
    ani_status status = ANI_ERROR;

    if ((status = env->Object_GetPropertyByName_Ref(param, name, &obj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    if ((status = env->Reference_IsUndefined(obj, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    if (isUndefined) {
        TAG_LOGE(AAFwkTag::ANI, "%{public}s : undefined", name);
        return false;
    }
    if (!GetStdString(env, reinterpret_cast<ani_string>(obj), value)) {
        TAG_LOGE(AAFwkTag::ANI, "GetStdString failed");
        return false;
    }
    return true;
}

bool GetStringArrayProperty(ani_env *env, ani_object param, const char *name, std::vector<std::string> &value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }

    ani_ref arrayObj = nullptr;
    ani_boolean isUndefined = true;
    ani_status status = ANI_ERROR;
    ani_double length = 0.0;
    std::string str;

    if ((status = env->Object_GetPropertyByName_Ref(param, name, &arrayObj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    if ((status = env->Reference_IsUndefined(arrayObj, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    if (isUndefined) {
        TAG_LOGE(AAFwkTag::ANI, "%{public}s : undefined", name);
        return false;
    }

    status = env->Object_GetPropertyByName_Double(reinterpret_cast<ani_object>(arrayObj), "length", &length);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }

    for (int i = 0; i < static_cast<int>(length); i++) {
        ani_ref stringEntryRef;
        status = env->Object_CallMethodByName_Ref(reinterpret_cast<ani_object>(arrayObj),
            "$_get", "i:Y", &stringEntryRef, (ani_int)i);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::ANI, "status: %{public}d, index: %{public}d", status, i);
            return false;
        }

        str = "";
        if (!GetStdString(env, reinterpret_cast<ani_string>(stringEntryRef), str)) {
            TAG_LOGE(AAFwkTag::ANI, "GetStdString failed, index: %{public}d", i);
            return false;
        }

        value.push_back(str);
        TAG_LOGI(AAFwkTag::ANI, "GetStdString index: %{public}d %{public}s", i, str.c_str());
    }

    return true;
}

bool GetDoublePropertyObject(ani_env *env, ani_object param, const char *name, double &value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }

    ani_ref obj = nullptr;
    ani_status status = ANI_ERROR;
    if (!GetRefProperty(env, param, name, obj)) {
        TAG_LOGW(AAFwkTag::ANI, "%{public}s : undefined", name);
        return false;
    }
    if ((status = env->Object_CallMethodByName_Double(
        reinterpret_cast<ani_object>(obj), "toDouble", nullptr, &value)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    return true;
}

bool GetLongPropertyObject(ani_env *env, ani_object param, const char *name, ani_long &value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }

    ani_ref obj = nullptr;
    ani_status status = ANI_ERROR;
    if (!GetRefProperty(env, param, name, obj)) {
        TAG_LOGW(AAFwkTag::ANI, "%{public}s : undefined", name);
        return false;
    }
    if ((status = env->Object_CallMethodByName_Long(
        reinterpret_cast<ani_object>(obj), "toLong", nullptr, &value)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    return true;
}

bool GetIntPropertyObject(ani_env *env, ani_object param, const char *name, ani_int &value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }

    ani_ref obj = nullptr;
    ani_status status = ANI_ERROR;
    if (!GetRefProperty(env, param, name, obj)) {
        TAG_LOGW(AAFwkTag::ANI, "%{public}s : undefined", name);
        return false;
    }
    if ((status = env->Object_CallMethodByName_Int(
        reinterpret_cast<ani_object>(obj), "toInt", nullptr, &value)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    return true;
}

bool GetDoublePropertyValue(ani_env *env, ani_object param, const char *name, double &value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }

    ani_status status = ANI_ERROR;
    ani_double res = 0.0;
    if ((status = env->Object_GetPropertyByName_Double(param, name, &res)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    value = static_cast<double>(res);
    return true;
}

bool GetIntPropertyValue(ani_env *env, ani_object param, const char *name, int32_t &value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }

    ani_status status = ANI_ERROR;
    ani_int res = 0;
    if ((status = env->Object_GetPropertyByName_Int(param, name, &res)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    value = res;
    return true;
}

bool GetRefProperty(ani_env *env, ani_object param, const char *name, ani_ref &value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }

    ani_status status = ANI_ERROR;
    ani_boolean isUndefined = true;

    if ((status = env->Object_GetPropertyByName_Ref(param, name, &value)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    if ((status = env->Reference_IsUndefined(value, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    return !isUndefined;
}

bool GetBooleanPropertyObject(ani_env *env, ani_object param, const char *name, bool &value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }

    ani_ref obj = nullptr;
    ani_status status = ANI_ERROR;
    if (!GetRefProperty(env, param, name, obj)) {
        TAG_LOGW(AAFwkTag::ANI, "%{public}s : undefined", name);
        return false;
    }
    ani_boolean aniValue = ANI_FALSE;
    if ((status = env->Object_CallMethodByName_Boolean(
        reinterpret_cast<ani_object>(obj), "toBoolean", nullptr, &aniValue)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    value = aniValue;
    return true;
}

bool SetDoublePropertyObject(ani_env *env, ani_object param, const char *name, double value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }

    ani_object obj = CreateDouble(env, static_cast<ani_double>(value));
    if (obj == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null obj");
        return false;
    }

    ani_status status = ANI_ERROR;
    if ((status = env->Object_SetPropertyByName_Ref(param, name, obj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    return true;
}

bool SetDoublePropertyValue(ani_env *env, ani_object param, const char *name, double value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }

    ani_status status = ANI_ERROR;
    if ((status = env->Object_SetPropertyByName_Double(param, name, value)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    return true;
}

bool SetIntPropertyObject(ani_env *env, ani_object param, const char *name, int32_t value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }

    ani_object obj = CreateInt(env, value);
    if (obj == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null obj");
        return false;
    }

    ani_status status = ANI_ERROR;
    if ((status = env->Object_SetPropertyByName_Ref(param, name, obj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    return true;
}

bool SetIntPropertyValue(ani_env *env, ani_object param, const char *name, int32_t value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }

    ani_status status = ANI_ERROR;
    if ((status = env->Object_SetPropertyByName_Int(param, name, value)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    return true;
}

bool SetStringArrayProperty(ani_env *env, ani_object param, const char *name, const std::vector<std::string> &values)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }

    ani_class arrayCls = nullptr;
    ani_method arrayCtor = nullptr;
    ani_object arrayObj = nullptr;
    ani_string string = nullptr;

    ani_status status = env->FindClass(CLASSNAME_ARRAY, &arrayCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }

    status = env->Class_FindMethod(arrayCls, "<ctor>", "i:", &arrayCtor);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }

    status = env->Object_New(arrayCls, arrayCtor, &arrayObj, values.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }

    for (size_t i = 0; i < values.size(); i++) {
        string = nullptr;
        status = env->String_NewUTF8(values[i].c_str(), values[i].size(), &string);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
            return false;
        }

        status = env->Object_CallMethodByName_Void(arrayObj, "$_set", "iY:", i, string);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
            return false;
        }
    }
    status = env->Object_SetPropertyByName_Ref(param, name, arrayObj);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }

    return true;
}

bool SetRefProperty(ani_env *env, ani_object param, const char *name, ani_ref value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }

    ani_status status = ANI_ERROR;
    if ((status = env->Object_SetPropertyByName_Ref(param, name, value)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    return true;
}

bool GetStaticFieldString(ani_env *env, ani_class classObj, const char *fieldName, std::string &value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null env");
        return false;
    }

    ani_status status = ANI_ERROR;
    ani_static_field field {};
    if ((status = env->Class_FindStaticField(classObj, fieldName, &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::JSNAPI, "Class_FindStaticField status: %{public}d", status);
        return false;
    }

    ani_ref obj = nullptr;
    if ((status = env->Class_GetStaticField_Ref(classObj, field, &obj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::JSNAPI, "Class_GetStaticField_Ref status: %{public}d", status);
        return false;
    }

    ani_boolean isUndefined = true;
    if ((status = env->Reference_IsUndefined(obj, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::JSNAPI, "status: %{public}d", status);
        return false;
    }
    if (isUndefined) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s : undefined", fieldName);
        return false;
    }

    if (!AppExecFwk::GetStdString(env, reinterpret_cast<ani_string>(obj), value)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "GetStdString failed");
        return false;
    }
    return true;
}

ani_env *AttachAniEnv(ani_vm * etsVm, bool &isAttachThread)
{
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    if (etsVm == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "etsVm is null");
        return nullptr;
    }
    if ((status = etsVm->GetEnv(ANI_VERSION_1, &env)) == ANI_OK) {
        return env;
    }
    ani_option interopEnabled { "--interop=disable", nullptr };
    ani_options aniArgs { 1, &interopEnabled };
    if ((status = etsVm->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &env)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "status: %{public}d", status);
        return nullptr;
    }
    isAttachThread = true;
    return env;
}

void DetachAniEnv(ani_vm * etsVm, bool &isAttachThread)
{
    if (isAttachThread) {
        if (etsVm == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "etsVm is null");
            return;
        }
        etsVm->DetachCurrentThread();
        isAttachThread = false;
    }
}

bool IsValidProperty(ani_env *env, ani_ref param)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_status status = ANI_ERROR;
    ani_boolean isNull = false;
    if ((status = env->Reference_IsNullishValue(param, &isNull)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    ani_boolean isUndefined = false;
    if ((status = env->Reference_IsUndefined(param, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    if (isUndefined || isNull) {
        return false;
    }
    return true;
}

bool CheckCallerIsSystemApp()
{
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        TAG_LOGE(AAFwkTag::ANI, "Non-system app forbidden to call");
        return false;
    }
    return true;
}

ani_object WrapLocale(ani_env *env, const std::string &locale)
{
    TAG_LOGD(AAFwkTag::ANI, "WrapLocale called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "env is nullptr");
        return nullptr;
    }
    if (locale.empty()) {
        TAG_LOGE(AAFwkTag::ANI, "Locale string is empty");
        return nullptr;
    }
    ani_class localClass = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->FindClass("std.core.Intl.Locale", &localClass)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Find class failed, status: %{public}d", status);
        return nullptr;
    }
    ani_method localCtor = nullptr;
    if ((status = env->Class_FindMethod(localClass, "<ctor>",
        "X{C{std.core.Intl.Locale}C{std.core.String}}C{std.core.Intl.LocaleOptions}:", &localCtor)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Find Locale constructor failed, status: %{public}d", status);
        return nullptr;
    }
    ani_ref undefinedRef;
    if ((status = env->GetUndefined(&undefinedRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "GetUndefined failed, status: %{public}d", status);
        return nullptr;
    }
    ani_object localStrObj = GetAniString(env, locale);
    if (localStrObj == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "Get ani string failed");
        return nullptr;
    }
    ani_object localeObj = nullptr;
    if ((status = env->Object_New(localClass, localCtor, &localeObj, localStrObj, undefinedRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Object_New failed, status: %{public}d", status);
        return nullptr;
    }
    return localeObj;
}

ani_object CreateIntAniArray(ani_env *env, const std::vector<int32_t> &dataArry)
{
    ani_class arrayCls = nullptr;
    ani_status status = ANI_OK;

    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return nullptr;
    }

    status = env->FindClass("std.core.Array", &arrayCls);
    if (status != ANI_OK || arrayCls == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "FindClass failed, status : %{public}d", status);
        return nullptr;
    }

    ani_method arrayCtor = nullptr;
    status = env->Class_FindMethod(arrayCls, "<ctor>", "i:", &arrayCtor);
    if (status != ANI_OK || arrayCtor == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "Class_FindMethod failed, status : %{public}d", status);
        return nullptr;
    }

    ani_object arrayObj = nullptr;
    status = env->Object_New(arrayCls, arrayCtor, &arrayObj, dataArry.size());
    if (status != ANI_OK || arrayObj == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "Object_New failed, status : %{public}d", status);
        return arrayObj;
    }

    for (size_t i = 0; i < dataArry.size(); i++) {
        ani_object intObj = AppExecFwk::CreateInt(env, dataArry[i]);
        if (intObj == nullptr) {
            TAG_LOGE(AAFwkTag::ANI, "intObj nullptr");
            return nullptr;
        }
        ani_status status =
            env->Object_CallMethodByName_Void(arrayObj, "$_set", "iY:", i, intObj);
        if (status != ANI_OK) {
            TAG_LOGE(
                AAFwkTag::ANI, "Object_CallMethodByName_Void failed, status : %{public}d", status);
            return nullptr;
        }
    }
    return arrayObj;
}
} // namespace AppExecFwk
} // namespace OHOS
