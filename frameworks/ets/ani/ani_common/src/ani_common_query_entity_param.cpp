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

#include "ani_common_query_entity_param.h"

#include "hilog_tag_wrapper.h"
#include "ani_common_util.h"
#include "ani_common_want.h"
#include "ani_enum_convert.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
namespace {
constexpr int32_t DEFAULT_INVAL_VALUE = -1;
constexpr const char *QUERY_TYPE_ENUM = "@ohos.app.ability.insightIntent.insightIntent.QueryType";
constexpr const char *QUERY_ENTITY_PARAM = "@ohos.app.ability.insightIntent.insightIntent.QueryEntityParamInner";

bool CreateObject(ani_env *env, ani_object &object, const std::string &className)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_status status = ANI_ERROR;
    ani_class cls = nullptr;
    if ((status = env->FindClass(className.c_str(), &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status : %{public}d", status);
        return false;
    }
    if (cls == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null cls");
        return false;
    }
    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status : %{public}d", status);
        return false;
    }
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null method");
        return false;
    }
    if ((status = env->Object_New(cls, method, &object)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status : %{public}d", status);
        return false;
    }
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null object");
        return false;
    }
    return true;
}
} // namespace

static bool UnwrapQueryEntityWantParamters(ani_env *env, ani_object param, InsightIntentQueryParam &queryParam)
{
    if (env == nullptr || param == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid params");
        return false;
    }

    ani_ref refQueryEntityParam = nullptr;
    if (!GetRefProperty(env, param, "queryEntityParam", refQueryEntityParam)) {
        TAG_LOGE(AAFwkTag::INTENT, "null queryEntityParam");
        return false;
    }

    auto objQueryEntityParam = reinterpret_cast<ani_object>(refQueryEntityParam);
    if (objQueryEntityParam == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null queryEntityParam");
        return false;
    }

    std::string queryType = "";
    ani_ref aniQueryType = nullptr;
    if (ANI_OK != env->Object_GetPropertyByName_Ref(objQueryEntityParam, "queryType", &aniQueryType)) {
        TAG_LOGE(AAFwkTag::INTENT, "Object_GetField_Ref queryType");
        return false;
    }
    if (!AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative(env,
        static_cast<ani_enum_item>(aniQueryType), queryType)) {
        TAG_LOGE(AAFwkTag::INTENT, "GetStdString failed queryType");
        return false;
    }
    if (queryType.compare("byProperty") != 0 && queryType.compare("all") != 0) {
        TAG_LOGE(AAFwkTag::INTENT, "queryType must be byProperty or all");
        return false;
    }
    queryParam.queryEntityParam_.queryType_ = queryType;

    if (IsExistsProperty(env, objQueryEntityParam, "parameters")) {
        ani_ref refParameters = nullptr;
        if (!GetRefProperty(env, objQueryEntityParam, "parameters", refParameters)) {
            TAG_LOGE(AAFwkTag::INTENT, "null parameters");
            return false;
        }
        auto wpParam = std::make_shared<WantParams>();
        if (!UnwrapWantParams(env, refParameters, *wpParam)) {
            TAG_LOGE(AAFwkTag::INTENT, "unwrap want fail");
            return false;
        }
        queryParam.queryEntityParam_.parameters_ = wpParam;
    }
    return true;
}

bool UnwrapQueryEntityParam(ani_env *env, ani_object param, InsightIntentQueryParam &queryParam)
{
    if (env == nullptr || param == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid params");
        return false;
    }

    std::string bundleName = "";
    if (!GetStringProperty(env, param, "bundleName", bundleName)) {
        TAG_LOGE(AAFwkTag::INTENT, "Wrong argument type bundleName");
        return false;
    }
    queryParam.bundleName_ = bundleName;

    std::string moduleName = "";
    if (!GetStringProperty(env, param, "moduleName", moduleName)) {
        TAG_LOGE(AAFwkTag::INTENT, "Wrong argument type moduleName");
        return false;
    }
    queryParam.moduleName_ = moduleName;

    std::string intentName = "";
    if (!GetStringProperty(env, param, "intentName", intentName)) {
        TAG_LOGE(AAFwkTag::INTENT, "Wrong argument type intentName");
        return false;
    }
    queryParam.intentName_ = intentName;

    std::string className = "";
    if (!GetStringProperty(env, param, "className", className)) {
        TAG_LOGE(AAFwkTag::INTENT, "Wrong argument type className");
        return false;
    }
    queryParam.className_ = className;

    if (IsExistsProperty(env, param, "userId")) {
        ani_int userId = DEFAULT_INVAL_VALUE;
        if (!GetIntPropertyObject(env, param, "userId", userId)) {
            TAG_LOGE(AAFwkTag::INTENT, "Wrong argument type userId");
        }
        queryParam.userId_ = static_cast<int32_t>(userId);
    }

    if (!UnwrapQueryEntityWantParamters(env, param, queryParam)) {
        TAG_LOGE(AAFwkTag::INTENT, "unwrap query entity want paramters failed");
        return false;
    }

    return true;
}

ani_ref WrapQueryEntityParam(ani_env *env, const std::string &queryType,
    const std::shared_ptr<AAFwk::WantParams> &queryParams)
{
    TAG_LOGD(AAFwkTag::ANI, "WrapQueryEntityParam called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return nullptr;
    }

    ani_object etsObject = nullptr;
    if (!CreateObject(env, etsObject, QUERY_ENTITY_PARAM)) {
        TAG_LOGE(AAFwkTag::ANI, "CreateObject failed for queryEntityParam");
        return nullptr;
    }

    ani_enum_item queryTypeEnumItem = nullptr;
    if (!AAFwk::AniEnumConvertUtil::EnumConvert_NativeToEts<std::string>(
        env, QUERY_TYPE_ENUM, queryType, queryTypeEnumItem)) {
        TAG_LOGE(AAFwkTag::ANI, "EnumConvert_NativeToEts failed for queryType: %{public}s", queryType.c_str());
        return nullptr;
    }

    ani_status status = ANI_ERROR;
    if ((status = env->Object_SetPropertyByName_Ref(etsObject, "queryType", queryTypeEnumItem)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "set type failed, status: %{public}d", status);
        return nullptr;
    }

    if (queryParams == nullptr) {
        return etsObject;
    }

    ani_ref wantParamsRef = WrapWantParams(env, *queryParams);
    if (wantParamsRef == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "WrapWantParams failed");
        return nullptr;
    }

    if ((status = env->Object_SetPropertyByName_Ref(etsObject, "parameters", wantParamsRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "set parameters failed, status: %{public}d", status);
        return nullptr;
    }

    return etsObject;
}

}  // namespace AbilityRuntime
}  // namespace OHOS
