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
bool GetFieldDoubleByName(ani_env *env, ani_object object, const char *name, double &value);
bool SetFieldDoubleByName(ani_env *env, ani_class cls, ani_object object, const char *name, double value);

bool GetFieldBoolByName(ani_env *env, ani_object object, const char *name, bool &value);
bool SetFieldBoolByName(ani_env *env, ani_class cls, ani_object object, const char *name, bool value);

bool GetFieldStringByName(ani_env *env, ani_object object, const char *name, std::string &value);
bool SetFieldStringByName(ani_env *env, ani_class cls, ani_object object, const char *name,
    const std::string &value);

bool GetFieldIntByName(ani_env *env, ani_object object, const char *name, int &value);
bool SetFieldIntByName(ani_env *env, ani_class cls, ani_object object, const char *name, int value);

bool GetFieldStringArrayByName(ani_env *env, ani_object object, const char *name, std::vector<std::string> &value);
bool SetFieldArrayStringByName(ani_env *env, ani_class cls, ani_object object, const char *name,
    const std::vector<std::string> &value);

bool GetFieldRefByName(ani_env *env, ani_object object, const char *name, ani_ref &ref);
bool SetFieldRefByName(ani_env *env, ani_class cls, ani_object object, const char *name, ani_ref value);

bool GetStdString(ani_env *env, ani_string str, std::string &value);
ani_string GetAniString(ani_env *env, const std::string &str);
bool GetAniStringArray(ani_env *env, const std::vector<std::string> &values, ani_array_ref *value);
ani_object CreateInt(ani_env *env, ani_int value);
bool SetOptionalFieldInt(ani_env *env, ani_class cls, ani_object object, const std::string &fieldName, int value);

ani_object CreateDouble(ani_env *env, ani_double value);
ani_object CreateBoolean(ani_env *env, ani_boolean value);

bool AsyncCallback(ani_env *env, ani_object call, ani_object error, ani_object result);
} // namespace AppExecFwk
} // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ANI_COMMON_UTIL_H
