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

#include "ani_common_json_util.h"

#include "ani_common_cache_mgr.h"
#include "ani_common_util.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr const char *CLASSNAME_ARRAY = "std.core.Array";
constexpr const char *RECORD_SET_NAME =
    "X{C{std.core.Numeric}C{std.core.String}C{std.core.BaseEnum}}Y:";

bool SetRecordValue(ani_env *env, ani_object recordObject, ani_string key, ani_object value)
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

bool CreateAniValueFromJson(ani_env *env, const nlohmann::json &jsonValue, ani_object &aniValue)
{
    if (jsonValue.is_object()) {
        return CreateAniRecordFromJson(env, jsonValue, aniValue);
    }
    if (jsonValue.is_array()) {
        return CreateAniArrayFromJson(env, jsonValue, aniValue);
    }
    if (jsonValue.is_string()) {
        aniValue = GetAniString(env, jsonValue.get<std::string>());
        return aniValue != nullptr;
    }
    if (jsonValue.is_boolean()) {
        aniValue = CreateBoolean(env, jsonValue.get<bool>());
        return aniValue != nullptr;
    }
    if (jsonValue.is_number()) {
        aniValue = CreateDouble(env, jsonValue.get<double>());
        return aniValue != nullptr;
    }
    if (jsonValue.is_null()) {
        return true;
    }

    TAG_LOGW(AAFwkTag::ANI, "unsupported json value type");
    return false;
}
} // namespace

bool CreateEmptyAniRecord(ani_env *env, ani_object &recordObject)
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

bool CreateAniArrayFromJson(ani_env *env, const nlohmann::json &jsonArray, ani_object &arrayObject)
{
    if (!jsonArray.is_array()) {
        TAG_LOGE(AAFwkTag::ANI, "json is not array");
        return false;
    }

    ani_class arrayCls = nullptr;
    ani_status status = env->FindClass(CLASSNAME_ARRAY, &arrayCls);
    if (status != ANI_OK || arrayCls == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "failed to find array class, status: %{public}d", status);
        return false;
    }

    ani_method arrayCtor = nullptr;
    status = env->Class_FindMethod(arrayCls, "<ctor>", "i:", &arrayCtor);
    if (status != ANI_OK || arrayCtor == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "failed to find array constructor, status: %{public}d", status);
        return false;
    }

    status = env->Object_New(arrayCls, arrayCtor, &arrayObject, jsonArray.size());
    if (status != ANI_OK || arrayObject == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "failed to create array object, status: %{public}d", status);
        return false;
    }

    ani_size index = 0;
    for (const auto &item : jsonArray) {
        ani_object elementValue = nullptr;
        if (!CreateAniValueFromJson(env, item, elementValue)) {
            TAG_LOGE(AAFwkTag::ANI, "failed to create array element");
            index++;
            continue;
        }
        if (elementValue != nullptr) {
            status = env->Object_CallMethodByName_Void(arrayObject, "$_set", "iY:", index, elementValue);
            if (status != ANI_OK) {
                TAG_LOGE(AAFwkTag::ANI, "failed to set array element, status: %{public}d", status);
                return false;
            }
        }
        index++;
    }
    return true;
}

bool CreateAniRecordFromJson(ani_env *env, const nlohmann::json &jsonObject, ani_object &recordObject)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    if (!jsonObject.is_object()) {
        TAG_LOGE(AAFwkTag::ANI, "json is not object");
        return false;
    }
    if (!CreateEmptyAniRecord(env, recordObject)) {
        TAG_LOGE(AAFwkTag::ANI, "failed to create record object");
        return false;
    }

    for (const auto &item : jsonObject.items()) {
        const std::string &key = item.key();
        const nlohmann::json &value = item.value();
        ani_string aniKey = GetAniString(env, key);
        if (aniKey == nullptr) {
            TAG_LOGE(AAFwkTag::ANI, "failed to create key string: %{public}s", key.c_str());
            continue;
        }

        ani_object aniValue = nullptr;
        if (!CreateAniValueFromJson(env, value, aniValue)) {
            TAG_LOGE(AAFwkTag::ANI, "failed to create value for key: %{public}s", key.c_str());
            continue;
        }
        if (aniValue == nullptr) {
            continue;
        }
        if (!SetRecordValue(env, recordObject, aniKey, aniValue)) {
            TAG_LOGE(AAFwkTag::ANI, "failed to set record for key: %{public}s", key.c_str());
            return false;
        }
    }
    return true;
}
} // namespace AppExecFwk
} // namespace OHOS
