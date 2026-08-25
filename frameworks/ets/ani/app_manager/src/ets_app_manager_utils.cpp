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

#include "ets_app_manager_utils.h"

#include <cstdint>
#include <vector>

#include "ani_common_util.h"
#include "ani_enum_convert.h"
#include "ets_error_utils.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppManagerEts {
namespace {
constexpr const char *DATA_CLASS_NAME = "application.AppStateData.AppStateData";
constexpr const char *CLASSNAME_ARRAY = "std.core.Array";
constexpr const char *INFO_INNER_CLASS_NAME = "application.RunningMultiAppInfo.RunningMultiAppInfoInner";
constexpr const char *INSTANCE_INNER_CLASS_NAME =
    "application.RunningMultiInstanceInfo.RunningMultiInstanceInfoInner";
constexpr const char *CLONE_INNER_CLASS_NAME = "application.RunningAppClone.RunningAppCloneInner";
constexpr const char *PROCESS_DATA_CLASS_NAME = "application.ProcessData.ProcessData";
constexpr const char *MULTI_APP_MODE_ENUM_NAME = "application.MultiAppMode.MultiAppMode";
constexpr const char *KEEP_ALIVE_APP_TYPE_ENUM_NAME = "@ohos.app.ability.appManager.appManager.KeepAliveAppType";
constexpr const char *KEEP_ALIVE_APP_SETTER_ENUM_NAME = "@ohos.app.ability.appManager.appManager.KeepAliveSetter";
constexpr const char *KEEP_ALIVE_BUNDLE_INFO_INNER_CLASS_NAME =
    "@ohos.app.ability.appManager.appManager.KeepAliveBundleInfoInner";
constexpr const char *ABILITY_FIRST_FRAME_STATE_DATA_CLASS_NAME =
    "application.AbilityFirstFrameStateData.AbilityFirstFrameStateDataInner";
}  // namespace

ani_object WrapAppStateData(ani_env *env, const AppExecFwk::AppStateData &appStateData)
{
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method method = nullptr;
    ani_object object = nullptr;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null env");
        return nullptr;
    }
    if ((status = env->FindClass(DATA_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status : %{public}d", status);
        return nullptr;
    }
    if (cls == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null cls");
        return nullptr;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status : %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_New(cls, method, &object)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status : %{public}d", status);
        return nullptr;
    }
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null object");
        return nullptr;
    }
    if (!SetAppStateData(env, object, appStateData)) {
        TAG_LOGE(AAFwkTag::APPMGR, "SetAppStateData failed");
        return nullptr;
    }
    return object;
}

bool SetAppStateData(ani_env *env, ani_object object, const AppExecFwk::AppStateData &appStateData)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null env");
        return false;
    }
    ani_status status = ANI_OK;
    status = env->Object_SetFieldByName_Ref(
        object, "bundleName", OHOS::AppExecFwk::GetAniString(env, appStateData.bundleName));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "failed status:%{public}d", status);
        return false;
    }
    status = env->Object_SetFieldByName_Int(object, "uid", appStateData.uid);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "failed status:%{public}d", status);
        return false;
    }
    status = env->Object_SetFieldByName_Int(object, "state", appStateData.state);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "failed status:%{public}d", status);
        return false;
    }
    status = env->Object_SetFieldByName_Boolean(object, "isSplitScreenMode", appStateData.isSplitScreenMode);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "failed status:%{public}d", status);
        return false;
    }
    status = env->Object_SetFieldByName_Boolean(object, "isFloatingWindowMode", appStateData.isFloatingWindowMode);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "failed status:%{public}d", status);
        return false;
    }
    return true;
}

ani_object CreateAppStateDataArray(ani_env *env, const std::vector<AppExecFwk::AppStateData> &data)
{
    ani_class arrayCls = nullptr;
    ani_status status = ANI_OK;
    status = env->FindClass(CLASSNAME_ARRAY, &arrayCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status : %{public}d", status);
        return nullptr;
    }

    ani_method arrayCtor = nullptr;
    status = env->Class_FindMethod(arrayCls, "<ctor>", "i:", &arrayCtor);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status : %{public}d", status);
        return nullptr;
    }

    ani_object arrayObj = nullptr;
    status = env->Object_New(arrayCls, arrayCtor, &arrayObj, data.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status : %{public}d", status);
        return arrayObj;
    }

    ani_size index = 0;
    for (auto &appStateData : data) {
        ani_ref aniData = WrapAppStateData(env, appStateData);
        if (aniData == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null aniData");
            break;
        }
        status = env->Object_CallMethodByName_Void(arrayObj, "$_set", "iY:", index, aniData);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::APPMGR, "status : %{public}d", status);
            break;
        }
        index++;
    }
    return arrayObj;
}

ani_object NewArrayClass(ani_env *env, const std::vector<std::string> &data)
{
    ani_class arrayCls = nullptr;
    ani_status status = ANI_OK;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null env");
        return nullptr;
    }
    status = env->FindClass(CLASSNAME_ARRAY, &arrayCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status : %{public}d", status);
        return nullptr;
    }

    ani_method arrayCtor = nullptr;
    status = env->Class_FindMethod(arrayCls, "<ctor>", "i:", &arrayCtor);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status : %{public}d", status);
        return nullptr;
    }

    ani_object arrayObj = nullptr;
    status = env->Object_New(arrayCls, arrayCtor, &arrayObj, data.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status : %{public}d", status);
        return arrayObj;
    }

    ani_size index = 0;
    for (auto &item : data) {
        ani_string aniString;
        status = env->String_NewUTF8(item.c_str(), item.size(), &aniString);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::APPMGR, "String_NewUTF8 failed status : %{public}d", status);
            break;
        }
        status = env->Object_CallMethodByName_Void(arrayObj, "$_set", "iY:", index, aniString);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::APPMGR, "Object_CallMethodByName_Void failed status : %{public}d", status);
            break;
        }
        index++;
    }
    return arrayObj;
}

ani_object CreateEmptyAniArray(ani_env *env)
{
    ani_class arrayCls = nullptr;
    ani_status status = ANI_OK;

    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null env");
        return nullptr;
    }

    status = env->FindClass(CLASSNAME_ARRAY, &arrayCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "FindClass failed status : %{public}d", status);
        return nullptr;
    }

    ani_method arrayCtor = nullptr;
    status = env->Class_FindMethod(arrayCls, "<ctor>", ":", &arrayCtor);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "find ctor failed status : %{public}d", status);
        return nullptr;
    }

    ani_object arrayObj = nullptr;
    status = env->Object_New(arrayCls, arrayCtor, &arrayObj);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Object_New array failed status : %{public}d", status);
        return arrayObj;
    }
    return arrayObj;
}

ani_object CreateEmptyMultiAppInfo(ani_env *env)
{
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method method = nullptr;
    ani_object object = nullptr;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null env");
        return nullptr;
    }
    if ((status = env->FindClass(INFO_INNER_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "FindClass failed status : %{public}d", status);
        return nullptr;
    }
    if (cls == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null cls");
        return nullptr;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "find ctor failed status : %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_New(cls, method, &object)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Object_New failed status : %{public}d", status);
        return nullptr;
    }
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null object");
        return nullptr;
    }
    return object;
}

ani_object CreateRunningMultiInstanceInfoArray(ani_env *env,
    const std::vector<AppExecFwk::RunningMultiInstanceInfo> &infos)
{
    ani_class arrayCls = nullptr;
    ani_status status = ANI_OK;

    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null env");
        return nullptr;
    }

    status = env->FindClass(CLASSNAME_ARRAY, &arrayCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status : %{public}d", status);
        return nullptr;
    }

    ani_method arrayCtor = nullptr;
    status = env->Class_FindMethod(arrayCls, "<ctor>", "i:", &arrayCtor);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status : %{public}d", status);
        return nullptr;
    }

    ani_object arrayObj = nullptr;
    status = env->Object_New(arrayCls, arrayCtor, &arrayObj, infos.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status : %{public}d", status);
        return arrayObj;
    }

    ani_size index = 0;
    for (auto &instanceInfo : infos) {
        ani_ref ani_info = WrapRunningMultiInstanceInfo(env, instanceInfo);
        if (ani_info == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null ani_info");
            break;
        }
        status = env->Object_CallMethodByName_Void(arrayObj, "$_set", "iY:", index, ani_info);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::APPMGR, "status : %{public}d", status);
            break;
        }
        index++;
    }
    return arrayObj;
}

ani_object CreateRunningAppCloneArray(ani_env *env, const std::vector<AppExecFwk::RunningAppClone> &infos)
{
    ani_class arrayCls = nullptr;
    ani_status status = ANI_OK;

    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null env");
        return nullptr;
    }

    status = env->FindClass(CLASSNAME_ARRAY, &arrayCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status : %{public}d", status);
        return nullptr;
    }

    ani_method arrayCtor = nullptr;
    status = env->Class_FindMethod(arrayCls, "<ctor>", "i:", &arrayCtor);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status : %{public}d", status);
        return nullptr;
    }

    ani_object arrayObj = nullptr;
    status = env->Object_New(arrayCls, arrayCtor, &arrayObj, infos.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status : %{public}d", status);
        return arrayObj;
    }

    ani_size index = 0;
    for (auto &runningAppclone : infos) {
        ani_ref ani_info = WrapRunningAppClone(env, runningAppclone);
        if (ani_info == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null ani_info");
            break;
        }
        status = env->Object_CallMethodByName_Void(arrayObj, "$_set", "iY:", index, ani_info);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::APPMGR, "status : %{public}d", status);
            break;
        }
        index++;
    }
    return arrayObj;
}

bool SetRunningMultiAppInfo(ani_env *env, ani_object object,
    const AppExecFwk::RunningMultiAppInfo &runningMultiAppInfo)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null env");
        return false;
    }
    ani_status status = env->Object_SetPropertyByName_Ref(
        object, "bundleName", OHOS::AppExecFwk::GetAniString(env, runningMultiAppInfo.bundleName));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "bundleName failed status:%{public}d", status);
        return false;
    }
    ani_enum_item modeItem = nullptr;
    OHOS::AAFwk::AniEnumConvertUtil::EnumConvert_NativeToEts(
        env, MULTI_APP_MODE_ENUM_NAME, runningMultiAppInfo.mode, modeItem);
    status = env->Object_SetPropertyByName_Ref(object, "mode", modeItem);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "mode failed status:%{public}d", status);
        return false;
    }
    status = env->Object_SetPropertyByName_Ref(object,
        "runningMultiInstances",
        CreateRunningMultiInstanceInfoArray(env, runningMultiAppInfo.runningMultiIntanceInfos));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "runningMultiInstances failed status:%{public}d", status);
        return false;
    }
    status = env->Object_SetPropertyByName_Ref(
        object, "runningAppClones", CreateRunningAppCloneArray(env, runningMultiAppInfo.runningAppClones));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "runningAppClones failed status:%{public}d", status);
        return false;
    }
    return true;
}

ani_object WrapRunningMultiAppInfo(ani_env *env, const AppExecFwk::RunningMultiAppInfo &runningMultiAppInfo)
{
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method method = nullptr;
    ani_object object = nullptr;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null env");
        return nullptr;
    }
    if ((status = env->FindClass(INFO_INNER_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status : %{public}d", status);
        return nullptr;
    }
    if (cls == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null cls");
        return nullptr;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "find ctor failed status : %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_New(cls, method, &object)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Object_New failed status : %{public}d", status);
        return nullptr;
    }
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null object");
        return nullptr;
    }
    if (!SetRunningMultiAppInfo(env, object, runningMultiAppInfo)) {
        TAG_LOGE(AAFwkTag::APPMGR, "SetRunningMultiAppInfo failed");
        return nullptr;
    }
    return object;
}

ani_object WrapRunningMultiInstanceInfo(ani_env *env, const AppExecFwk::RunningMultiInstanceInfo &instanceInfo)
{
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method method = nullptr;
    ani_object object = nullptr;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null env");
        return nullptr;
    }
    if ((status = env->FindClass(INSTANCE_INNER_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status : %{public}d", status);
        return nullptr;
    }
    if (cls == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null cls");
        return nullptr;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status : %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_New(cls, method, &object)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status : %{public}d", status);
        return nullptr;
    }
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null object");
        return nullptr;
    }
    if (!SetRunningMultiInstanceInfo(env, object, instanceInfo)) {
        TAG_LOGE(AAFwkTag::APPMGR, "SetRunningMultiInstanceInfo failed");
        return nullptr;
    }
    return object;
}

bool SetRunningMultiInstanceInfo(
    ani_env *env, ani_object object, const AppExecFwk::RunningMultiInstanceInfo &instanceInfo)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null env");
        return false;
    }
    ani_status status = env->Object_SetPropertyByName_Ref(
        object, "instanceKey", OHOS::AppExecFwk::GetAniString(env, instanceInfo.instanceKey));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "failed status:%{public}d", status);
        return false;
    }
    if ((status = env->Object_SetPropertyByName_Int(object, "uid", instanceInfo.uid)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "uid failed status:%{public}d", status);
        return false;
    }
    ani_class arrayCls = nullptr;
    status = env->FindClass(CLASSNAME_ARRAY, &arrayCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "find class failed status : %{public}d", status);
        return false;
    }
    ani_method arrayCtor;
    status = env->Class_FindMethod(arrayCls, "<ctor>", "i:", &arrayCtor);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "find ctor failed status : %{public}d", status);
        return false;
    }
    ani_object arrayObj;
    status = env->Object_New(arrayCls, arrayCtor, &arrayObj, instanceInfo.pids.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Object_New array failed status : %{public}d", status);
        return false;
    }
    ani_size index = 0;
    for (auto &pid : instanceInfo.pids) {
        ani_object aniPid = AppExecFwk::CreateInt(env, pid);
        status = env->Object_CallMethodByName_Void(arrayObj, "$_set", "iY:", index, aniPid);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::APPMGR, "set failed status : %{public}d", status);
            return false;
        }
        index++;
    }
    if ((status = env->Object_SetPropertyByName_Ref(object, "pids", arrayObj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "pids failed status:%{public}d", status);
        return false;
    }
    return true;
}

ani_object WrapRunningAppClone(ani_env *env, const AppExecFwk::RunningAppClone &runningAppClone)
{
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method method = nullptr;
    ani_object object = nullptr;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null env");
        return nullptr;
    }
    if ((status = env->FindClass(CLONE_INNER_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status : %{public}d", status);
        return nullptr;
    }
    if (cls == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null cls");
        return nullptr;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status : %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_New(cls, method, &object)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status : %{public}d", status);
        return nullptr;
    }
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null object");
        return nullptr;
    }
    if (!SetRunningAppClone(env, object, runningAppClone)) {
        TAG_LOGE(AAFwkTag::APPMGR, "SetRunningAppClone failed");
        return nullptr;
    }
    return object;
}

bool SetRunningAppClone(ani_env *env, ani_object object, const AppExecFwk::RunningAppClone &runningAppClone)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null env");
        return false;
    }
    ani_status status = env->Object_SetPropertyByName_Int(object, "appCloneIndex", runningAppClone.appCloneIndex);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "appCloneIndex failed status:%{public}d", status);
        return false;
    }
    status = env->Object_SetPropertyByName_Int(object, "uid", runningAppClone.uid);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "uid failed status:%{public}d", status);
        return false;
    }
    ani_class arrayCls = nullptr;
    status = env->FindClass(CLASSNAME_ARRAY, &arrayCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "find class failed status : %{public}d", status);
        return false;
    }
    ani_method arrayCtor;
    status = env->Class_FindMethod(arrayCls, "<ctor>", "i:", &arrayCtor);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "find ctor failed status : %{public}d", status);
        return false;
    }
    ani_object arrayObj;
    status = env->Object_New(arrayCls, arrayCtor, &arrayObj, runningAppClone.pids.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Object_New array failed status : %{public}d", status);
        return false;
    }
    ani_size index = 0;
    for (auto &pid : runningAppClone.pids) {
        ani_object aniPid = AppExecFwk::CreateInt(env, pid);
        status = env->Object_CallMethodByName_Void(arrayObj, "$_set", "iY:", index, aniPid);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::APPMGR, "set failed status : %{public}d", status);
            return false;
        }
        index++;
    }
    status = env->Object_SetPropertyByName_Ref(object, "pids", arrayObj);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "pids failed status:%{public}d", status);
        return false;
    }
    return true;
}

ani_object WrapProcessData(ani_env *env, const AppExecFwk::ProcessData &processData)
{
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method method = nullptr;
    ani_object object = nullptr;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null env");
        return nullptr;
    }
    if ((status = env->FindClass(PROCESS_DATA_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status : %{public}d", status);
        return nullptr;
    }
    if (cls == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null cls");
        return nullptr;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status : %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_New(cls, method, &object)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status : %{public}d", status);
        return nullptr;
    }
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null object");
        return nullptr;
    }
    if (!SetProcessData(env, object, processData)) {
        TAG_LOGE(AAFwkTag::APPMGR, "SetProcessData failed");
        return nullptr;
    }
    return object;
}

bool SetProcessData(ani_env *env, ani_object object, const AppExecFwk::ProcessData &processData)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null env");
        return false;
    }
    ani_status status = env->Object_SetFieldByName_Ref(
        object, "bundleName", OHOS::AppExecFwk::GetAniString(env, processData.bundleName));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "bundleName failed status:%{public}d", status);
        return false;
    }
    status = env->Object_SetFieldByName_Int(object, "pid", processData.pid);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "pid failed status:%{public}d", status);
        return false;
    }
    status = env->Object_SetFieldByName_Int(object, "uid", processData.uid);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "uid failed status:%{public}d", status);
        return false;
    }
    status = env->Object_SetFieldByName_Int(object, "state", static_cast<ani_int>(processData.state));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "state failed status:%{public}d", status);
        return false;
    }
    status = env->Object_SetFieldByName_Boolean(object, "isContinuousTask", processData.isContinuousTask);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "isContinuousTask failed status:%{public}d", status);
        return false;
    }
    status = env->Object_SetFieldByName_Boolean(object, "isKeepAlive", processData.isKeepAlive);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "isKeepAlive failed status:%{public}d", status);
        return false;
    }
    return true;
}

bool UnWrapArrayString(ani_env *env, ani_object arrayObj, std::vector<std::string> &stringList)
{
    if (env == nullptr || arrayObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env null or arrayObj null");
        return false;
    }
    stringList.clear();
    ani_size size = 0;
    ani_status status = ANI_ERROR;
    if ((status = env->Array_GetLength(reinterpret_cast<ani_array>(arrayObj), &size)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status : %{public}d", status);
        return false;
    }
    ani_ref ref = nullptr;
    ani_size idx = 0;
    for (idx = 0; idx < size; idx++) {
        if ((status = env->Array_Get(reinterpret_cast<ani_array>(arrayObj), idx, &ref)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::APPMGR, "status : %{public}d, index: %{public}zu", status, idx);
            return false;
        }
        std::string str = "";
        if (!OHOS::AppExecFwk::GetStdString(env, reinterpret_cast<ani_string>(ref), str)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "GetStdString failed, index: %{public}zu", idx);
            return false;
        }
        stringList.push_back(str);
    }
    return true;
}

ani_object CreateIntAniArray(ani_env * env, const std::vector<int32_t> &dataArry)
{
    ani_class arrayCls = nullptr;
    ani_status status = ANI_OK;

    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null env");
        return nullptr;
    }

    status = env->FindClass(CLASSNAME_ARRAY, &arrayCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status : %{public}d", status);
        return nullptr;
    }

    ani_method arrayCtor = nullptr;
    status = env->Class_FindMethod(arrayCls, "<ctor>", "i:", &arrayCtor);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status : %{public}d", status);
        return nullptr;
    }

    ani_object arrayObj = nullptr;
    status = env->Object_New(arrayCls, arrayCtor, &arrayObj, dataArry.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status : %{public}d", status);
        return arrayObj;
    }

    for (size_t i = 0; i < dataArry.size(); i++) {
        ani_object intObj = AppExecFwk::CreateInt(env, dataArry[i]);
        if (intObj == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "intObj nullptr");
            return nullptr;
        }
        ani_status status = env->Object_CallMethodByName_Void(arrayObj, "$_set", "iY:", i, intObj);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::APPMGR, "status : %{public}d", status);
            return nullptr;
        }
    }
    return arrayObj;
}

bool UnWrapArrayInt(ani_env *env, ani_object arrayObj, std::vector<int32_t> &list)
{
    if (env == nullptr || arrayObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env null or arrayObj null");
        return false;
    }
    list.clear();
    ani_size size = 0;
    ani_status status = ANI_ERROR;
    if ((status = env->Array_GetLength(reinterpret_cast<ani_array>(arrayObj), &size)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status : %{public}d", status);
        return false;
    }
    ani_ref ref;
    ani_size idx;
    for (idx = 0; idx < size; idx++) {
        if ((status = env->Array_Get(reinterpret_cast<ani_array>(arrayObj), idx, &ref)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::APPMGR, "status : %{public}d, index: %{public}zu", status, idx);
            return false;
        }
        ani_int ival = 0;
        if ((status = env->Object_CallMethodByName_Int(static_cast<ani_object>(ref),
            "toInt", nullptr, &ival)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::APPMGR, "Object_CallMethodByName_Int status : %{public}d", status);
            return false;
        }
        list.push_back(ival);
    }
    return true;
}

ani_object WrapKeepAliveInfo(ani_env *env, const AbilityRuntime::KeepAliveInfo &keepAliveInfo)
{
    ani_class cls {};
    ani_status status = ANI_ERROR;
    ani_method method {};
    ani_object object = nullptr;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null env");
        return nullptr;
    }
    if ((status = env->FindClass(KEEP_ALIVE_BUNDLE_INFO_INNER_CLASS_NAME, &cls))
        != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "find class failed status : %{public}d", status);
        return nullptr;
    }
    if (cls == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null cls");
        return nullptr;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "find ctor failed status : %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_New(cls, method, &object)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Object_New failed status : %{public}d", status);
        return nullptr;
    }
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null object");
        return nullptr;
    }
    if (!SetKeepAliveInfo(env, object, keepAliveInfo)) {
        TAG_LOGE(AAFwkTag::APPMGR, "SetKeepAliveInfo failed");
        return nullptr;
    }
    return object;
}

bool SetKeepAliveInfo(ani_env *env, ani_object object, const AbilityRuntime::KeepAliveInfo &keepInfo)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null env");
        return false;
    }
    ani_status status = ANI_OK;
    if ((status = env->Object_SetPropertyByName_Ref(object, "bundleName",
        OHOS::AppExecFwk::GetAniString(env, keepInfo.bundleName))) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "bundleName failed status:%{public}d", status);
        return false;
    }
    ani_enum_item typeItem {};
    OHOS::AAFwk::AniEnumConvertUtil::EnumConvert_NativeToEts(
        env, KEEP_ALIVE_APP_TYPE_ENUM_NAME, keepInfo.appType, typeItem);
    if ((status = env->Object_SetPropertyByName_Ref(object, "type", typeItem)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "state failed status:%{public}d", status);
        return false;
    }
    ani_enum_item setterItem {};
    OHOS::AAFwk::AniEnumConvertUtil::EnumConvert_NativeToEts(
        env, KEEP_ALIVE_APP_SETTER_ENUM_NAME, keepInfo.setter, setterItem);
    if ((status = env->Object_SetPropertyByName_Ref(object, "setter", setterItem)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "state failed status:%{public}d", status);
        return false;
    }
    if ((status = env->Object_SetPropertyByName_Ref(object, "setterUserId",
        AppExecFwk::CreateInt(env, keepInfo.setterId))) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "setterUserId failed status:%{public}d", status);
        return false;
    }
    if (keepInfo.policy != AbilityRuntime::KeepAlivePolicy::UNSPECIFIED) {
        if ((status = env->Object_SetPropertyByName_Boolean(object, "allowUserToCancel",
            keepInfo.policy == AbilityRuntime::KeepAlivePolicy::ALLOW_CANCEL)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::APPMGR, "allowUserToCancel failed status:%{public}d", status);
            return false;
        }
    }
    return true;
}

ani_object CreateKeepAliveInfoArray(ani_env *env, const std::vector<AbilityRuntime::KeepAliveInfo> &infos)
{
    ani_class arrayCls = nullptr;
    ani_status status = ANI_OK;

    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null env");
        return nullptr;
    }

    status = env->FindClass(CLASSNAME_ARRAY, &arrayCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "FindClass failed status : %{public}d", status);
        return nullptr;
    }

    ani_method arrayCtor;
    status = env->Class_FindMethod(arrayCls, "<ctor>", "i:", &arrayCtor);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "find ctor failed status : %{public}d", status);
        return nullptr;
    }

    ani_object arrayObj = nullptr;
    status = env->Object_New(arrayCls, arrayCtor, &arrayObj, infos.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Object_New array status : %{public}d", status);
        return nullptr;
    }
    ani_size index = 0;
    for (auto &info : infos) {
        ani_object ani_info = WrapKeepAliveInfo(env, info);
        if (ani_info == nullptr) {
            TAG_LOGW(AAFwkTag::APPMGR, "null ani_info");
            break;
        }
        status = env->Object_CallMethodByName_Void(arrayObj, "$_set", "iY:", index, ani_info);
        if (status != ANI_OK) {
            TAG_LOGW(AAFwkTag::APPMGR, "Object_CallMethodByName_Void failed status : %{public}d", status);
            break;
        }
        index++;
    }
    return arrayObj;
}
#ifdef SUPPORT_GRAPHICS
bool SetAbilityFirstFrameStateData(ani_env *env, ani_object object,
    const AppExecFwk::AbilityFirstFrameStateData &abilityFirstFrameStateData)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null env");
        return false;
    }
    ani_status status = ANI_OK;
    const std::string &strBundleName = abilityFirstFrameStateData.bundleName;
    if ((status = env->Object_SetPropertyByName_Ref(object, "bundleName",
        OHOS::AppExecFwk::GetAniString(env, strBundleName))) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "bundleName failed status:%{public}d", status);
        return false;
    }
    const std::string &strModuleName = abilityFirstFrameStateData.moduleName;
    if ((status = env->Object_SetPropertyByName_Ref(object, "moduleName",
        OHOS::AppExecFwk::GetAniString(env, strModuleName))) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "moduleName failed status:%{public}d", status);
        return false;
    }
    const std::string &strAbilityName = abilityFirstFrameStateData.abilityName;
    if ((status = env->Object_SetPropertyByName_Ref(object, "abilityName",
        OHOS::AppExecFwk::GetAniString(env, strAbilityName))) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "abilityName failed status:%{public}d", status);
        return false;
    }
    if ((status = env->Object_SetPropertyByName_Int(object, "appIndex",
        abilityFirstFrameStateData.appIndex)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "appIndex failed status:%{public}d", status);
        return false;
    }
    if ((status = env->Object_SetPropertyByName_Boolean(object, "isColdStart",
        abilityFirstFrameStateData.coldStart)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "isColdStart failed status:%{public}d", status);
        return false;
    }
    return true;
}

ani_object WrapAbilityFirstFrameStateData(ani_env *env,
    const AppExecFwk::AbilityFirstFrameStateData &abilityFirstFrameStateData)
{
    ani_class cls {};
    ani_status status = ANI_ERROR;
    ani_method method {};
    ani_object object = nullptr;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null env");
        return nullptr;
    }
    if ((status = env->FindClass(ABILITY_FIRST_FRAME_STATE_DATA_CLASS_NAME, &cls)
        ) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "find class failed status : %{public}d", status);
        return nullptr;
    }
    if (cls == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null cls");
        return nullptr;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "find ctor failed status : %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_New(cls, method, &object)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Object_New AbilityFirstFrameStateData failed status : %{public}d", status);
        return nullptr;
    }
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null object");
        return nullptr;
    }
    if (!SetAbilityFirstFrameStateData(env, object, abilityFirstFrameStateData)) {
        TAG_LOGE(AAFwkTag::APPMGR, "SetAbilityFirstFrameStateData failed");
        return nullptr;
    }
    return object;
}
#endif

bool AttachAniEnv(ani_vm *etsVm, ani_env *&env)
{
    if (etsVm == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "etsVm nullptr");
        return false;
    }
    ani_status status = ANI_ERROR;
    if ((status = etsVm->GetEnv(ANI_VERSION_1, &env)) == ANI_OK) {
        return env != nullptr;
    }
    ani_option interopEnabled { "--interop=disable", nullptr };
    ani_options aniArgs { 1, &interopEnabled };
    if ((status = (etsVm->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &env))) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status: %{public}d", status);
        return false;
    }
    return env != nullptr;
}

void DetachAniEnv(ani_vm *etsVm)
{
    if (etsVm == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "etsVm nullptr");
        return;
    }
    ani_status status = ANI_ERROR;
    if ((status = etsVm->DetachCurrentThread()) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status: %{public}d", status);
    }
}

void ReleaseObjectReference(ani_vm *etsVm, ani_ref etsObjRef)
{
    if (etsVm == nullptr || etsObjRef == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "etsVm null or etsObjRef null");
        return;
    }
    ani_status status = ANI_ERROR;
    ani_env *env = nullptr;
    if ((status = etsVm->GetEnv(ANI_VERSION_1, &env)) != ANI_OK || env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetEnv failed status:%{public}d", status);
        return;
    }
    if ((status = env->GlobalReference_Delete(etsObjRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "GlobalReference_Delete status: %{public}d", status);
    }
}

} // namespace AppManagerEts
} // namespace OHOS
