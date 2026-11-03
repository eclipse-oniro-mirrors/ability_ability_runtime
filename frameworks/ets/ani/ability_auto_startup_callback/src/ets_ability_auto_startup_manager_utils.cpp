/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"),
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

#include "ets_ability_auto_startup_manager_utils.h"

#include "ani_common_util.h"
#include "global_constant.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char *CLASSNAME_ARRAY = "std.core.Array";
constexpr const char *SIGNATURE_AUTO_STARTUP_INFO = "application.AutoStartupInfo.AutoStartupInfoInner";
}
bool UnwrapAutoStartupInfo(ani_env *env, ani_object param, AutoStartupInfo &info)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called UnwrapAutoStartupInfo");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null env");
        return false;
    }
    if (!AppExecFwk::GetStringProperty(env, param, "bundleName", info.bundleName)) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Failed to get bundleName");
        return false;
    }
    if (!AppExecFwk::GetStringProperty(env, param, "abilityName", info.abilityName)) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Failed to get abilityName");
        return false;
    }
    if (AppExecFwk::IsExistsProperty(env, param, "appCloneIndex")) {
        ani_int appCloneIndex = 0;
        if (!AppExecFwk::GetIntPropertyObject(env, param, "appCloneIndex", appCloneIndex)) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Failed to get appCloneIndex");
            return false;
        }
        info.appCloneIndex = appCloneIndex;
    }
    if (!AppExecFwk::GetStringProperty(env, param, "moduleName", info.moduleName)) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Failed to get moduleName");
        return false;
    }
    if (AppExecFwk::IsExistsProperty(env, param, "isDirectBoot") &&
        !AppExecFwk::GetBooleanPropertyObject(env, param, "isDirectBoot", info.isDirectBoot)) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Failed to get isDirectBoot");
        return false;
    }
    return true;
}

static bool SetAutoStartupInfo(ani_env *env, ani_object object, const AutoStartupInfo &info)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null env");
        return false;
    }
    if (!AppExecFwk::SetRefProperty(env, object, "bundleName", AppExecFwk::GetAniString(env, info.bundleName))) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Failed to set bundleName");
        return false;
    }
    if (!AppExecFwk::SetRefProperty(env, object, "moduleName", AppExecFwk::GetAniString(env, info.moduleName))) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Failed to set moduleName");
        return false;
    }
    if (!AppExecFwk::SetRefProperty(env, object, "abilityName", AppExecFwk::GetAniString(env, info.abilityName))) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Failed to set abilityName");
        return false;
    }
    if (!AppExecFwk::SetRefProperty(
        env, object, "abilityTypeName", AppExecFwk::GetAniString(env, info.abilityTypeName))) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Failed to set abilityTypeName");
        return false;
    }
    if (info.appCloneIndex >= 0 && info.appCloneIndex < GlobalConstant::MAX_APP_CLONE_INDEX) {
        if (!AppExecFwk::SetIntPropertyObject(env, object, "appCloneIndex", info.appCloneIndex)) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Failed to set appCloneIndex");
            return false;
        }
    }
    if (!AppExecFwk::SetIntPropertyObject(env, object, "userId", info.userId)) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Failed to set userId");
        return false;
    }
    if (!AppExecFwk::SetIntPropertyObject(env, object, "setterUserId", info.setterUserId)) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Failed to set setterUserId");
        return false;
    }
    if (!AppExecFwk::SetRefProperty(
        env, object, "canUserModify", AppExecFwk::CreateBoolean(env, info.canUserModify))) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Failed to set canUserModify");
        return false;
    }
    if (!AppExecFwk::SetRefProperty(
        env, object, "isDirectBoot", AppExecFwk::CreateBoolean(env, info.isDirectBoot))) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Failed to set isDirectBoot");
        return false;
    }
    return true;
}

ani_object ConvertAutoStartupInfos(ani_env *env, const std::vector<AutoStartupInfo> &infos)
{
    ani_class arrayCls = nullptr;
    ani_status status = ANI_OK;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null env");
        return nullptr;
    }
    status = env->FindClass(CLASSNAME_ARRAY, &arrayCls);
    if (status != ANI_OK || arrayCls == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "FindClass failed status : %{public}d or null arrayCls", status);
        return nullptr;
    }
    ani_method arrayCtor = nullptr;
    status = env->Class_FindMethod(arrayCls, "<ctor>", "i:", &arrayCtor);
    if (status != ANI_OK || arrayCtor == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "find ctor failed status : %{public}d or null arrayCtor", status);
        return nullptr;
    }
    ani_object arrayObj = nullptr;
    status = env->Object_New(arrayCls, arrayCtor, &arrayObj, infos.size());
    if (status != ANI_OK || arrayObj == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Object_New array status : %{public}d or null arrayObj", status);
        return arrayObj;
    }
    ani_size index = 0;
    for (auto &info : infos) {
        ani_object ani_info = CreateAniAutoStartupInfo(env, info);
        if (ani_info == nullptr) {
            TAG_LOGW(AAFwkTag::AUTO_STARTUP, "null ani_info");
            return nullptr;
        }
        status = env->Object_CallMethodByName_Void(arrayObj, "$_set", "iY:", index, ani_info);
        if (status != ANI_OK) {
            TAG_LOGW(AAFwkTag::AUTO_STARTUP, "Object_CallMethodByName_Void failed status : %{public}d", status);
            return nullptr;
        }
        index++;
    }
    return arrayObj;
}

ani_object CreateAniAutoStartupInfo(ani_env *env, const AutoStartupInfo &info)
{
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method method = nullptr;
    ani_object object = nullptr;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null env");
        return nullptr;
    }
    if ((status = env->FindClass(SIGNATURE_AUTO_STARTUP_INFO, &cls)) != ANI_OK || cls == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "find class failed status : %{public}d or null cls", status);
        return nullptr;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":", &method)) != ANI_OK || method == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "find ctor failed status : %{public}d or null method", status);
        return nullptr;
    }
    if ((status = env->Object_New(cls, method, &object)) != ANI_OK || object == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Object_New failed status : %{public}d or null object", status);
        return nullptr;
    }
    if (!SetAutoStartupInfo(env, object, info)) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "SetBusinessAbilityInfo failed");
        return nullptr;
    }
    return object;
}
} // namespace AbilityRuntime
} // namespace OHOS
