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

#ifndef OHOS_ABILITY_RUNTIME_ANI_COMMON_UTIL_H
#define OHOS_ABILITY_RUNTIME_ANI_COMMON_UTIL_H

#include <string>
#include <vector>
#include "ani.h"

namespace OHOS {
namespace AppExecFwk {
struct RunningProcessInfo;
bool GetFieldDoubleByName(ani_env *env, ani_object object, const char *name, double &value);
bool SetFieldDoubleByName(ani_env *env, ani_class cls, ani_object object, const char *name, double value);

bool GetFieldLongByName(ani_env *env, ani_object object, const char *name, int64_t &value);
bool SetFieldLongByName(ani_env *env, ani_class cls, ani_object object, const char *name, int64_t value);

bool GetFieldBoolByName(ani_env *env, ani_object object, const char *name, bool &value);
bool SetFieldBoolByName(ani_env *env, ani_class cls, ani_object object, const char *name, bool value);

bool GetFieldStringByName(ani_env *env, ani_object object, const char *name, std::string &value);
bool SetFieldStringByName(ani_env *env, ani_class cls, ani_object object, const char *name,
    const std::string &value);

bool GetFieldIntByName(ani_env *env, ani_object object, const char *name, int32_t &value);
bool SetFieldIntByName(ani_env *env, ani_class cls, ani_object object, const char *name, int32_t value);

bool GetFieldStringArrayByName(ani_env *env, ani_object object, const char *name, std::vector<std::string> &value);
bool SetFieldArrayStringByName(ani_env *env, ani_class cls, ani_object object, const char *name,
    const std::vector<std::string> &value);

bool GetFieldRefByName(ani_env *env, ani_object object, const char *name, ani_ref &ref);
bool SetFieldRefByName(ani_env *env, ani_class cls, ani_object object, const char *name, ani_ref value);

bool GetStdString(ani_env *env, ani_string str, std::string &value);
ani_string GetAniString(ani_env *env, const std::string &str);
bool GetAniStringArray(ani_env *env, const std::vector<std::string> &values, ani_array *value);

bool SetOptionalFieldInt(ani_env *env, ani_class cls, ani_object object, const std::string &fieldName, int value);

ani_object CreateDouble(ani_env *env, ani_double value);
ani_object CreateBoolean(ani_env *env, ani_boolean value);
ani_object CreateLong(ani_env *env, ani_long value);
ani_object CreateInt(ani_env *env, ani_int value);
ani_object CreateEtsNull(ani_env *env);

bool AsyncCallback(ani_env *env, ani_object call, ani_object error, ani_object result);
bool AsyncCallback(ani_env *env, const char *signature, ani_object call, ani_object error, ani_object result);
bool GetPropertyRef(ani_env *env, ani_object obj, const char *name, ani_ref &ref, ani_boolean &isUndefined);
bool IsExistsField(ani_env *env, ani_object param, const char *name);

bool WrapArrayString(ani_env *env, ani_object &arrayObj, const std::vector<std::string> &values);
bool UnwrapArrayString(ani_env *env, const ani_object &arrayObj, std::vector<std::string> &stringList);
ani_object CreateRunningProcessInfoArray(ani_env *env, std::vector<AppExecFwk::RunningProcessInfo> infos);
ani_object CreateEmptyArray(ani_env *env);

bool IsExistsProperty(ani_env *env, ani_object param, const char *name);
bool GetStringProperty(ani_env *env, ani_object param, const char *name, std::string &value);
bool GetStringArrayProperty(ani_env *env, ani_object param, const char *name, std::vector<std::string> &value);
bool GetDoublePropertyObject(ani_env *env, ani_object param, const char *name, double &value);
bool GetIntPropertyObject(ani_env *env, ani_object param, const char *name, ani_int &value);
bool GetLongPropertyObject(ani_env *env, ani_object param, const char *name, ani_long &value);
bool GetDoublePropertyValue(ani_env *env, ani_object param, const char *name, double &value);
bool GetIntPropertyValue(ani_env *env, ani_object param, const char *name, int32_t &value);
bool GetRefProperty(ani_env *env, ani_object param, const char *name, ani_ref &value);
bool GetBooleanPropertyObject(ani_env *env, ani_object param, const char *name, bool &value);

bool SetDoublePropertyObject(ani_env *env, ani_object param, const char *name, double value);
bool SetDoublePropertyValue(ani_env *env, ani_object param, const char *name, double value);
bool SetIntPropertyObject(ani_env *env, ani_object param, const char *name, int32_t value);
bool SetIntPropertyValue(ani_env *env, ani_object param, const char *name, int32_t value);
bool SetStringArrayProperty(ani_env *env, ani_object param, const char *name, const std::vector<std::string> &values);
bool SetRefProperty(ani_env *env, ani_object param, const char *name, ani_ref value);

bool GetStaticFieldString(ani_env *env, ani_class classObj, const char *fieldName, std::string &value);

ani_env *AttachAniEnv(ani_vm * etsVm, bool &isAttachThread);
void DetachAniEnv(ani_vm * etsVm, bool &isAttachThread);
bool IsValidProperty(ani_env *env, ani_ref param);
bool CheckCallerIsSystemApp();
ani_object WrapLocale(ani_env *env, const std::string &locale);
ani_object CreateIntAniArray(ani_env *env, const std::vector<int32_t> &dataArry);
} // namespace AppExecFwk
} // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ANI_COMMON_UTIL_H
