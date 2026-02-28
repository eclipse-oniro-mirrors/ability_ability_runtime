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

#include "ets_app_manager.h"

#include <cinttypes>

#include "ability_manager_client.h"
#include "ability_manager_interface.h"
#include "ani_common_util.h"
#include "ani_enum_convert.h"
#include "app_mgr_constants.h"
#include "app_mgr_interface.h"
#include "ets_ability_first_frame_state_observer.h"
#include "ets_app_foreground_state_observer.h"
#include "ets_app_manager_utils.h"
#include "ets_app_state_observer.h"
#include "ani_common_app_state_filter.h"
#include "ets_error_utils.h"
#include "hilog_tag_wrapper.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#ifdef SUPPORT_GRAPHICS
#ifdef SUPPORT_SCREEN
#include "tokenid_kit.h"
#endif
#endif

namespace OHOS {
namespace AppManagerEts {
namespace {
constexpr int32_t ERR_FAILURE = -1;
constexpr const char* APP_MANAGER_SPACE_NAME = "@ohos.app.ability.appManager.appManager";
constexpr const char* ON_OFF_TYPE = "applicationState";
constexpr const char* ON_OFF_TYPE_ABILITY_FIRST_FRAME_STATE = "abilityFirstFrameState";
constexpr const char* ON_OFF_TYPE_APP_FOREGROUND_STATE = "appForegroundState";

constexpr const char *APPLICATION_STATE_WITH_BUNDLELIST_ON_SIGNATURE =
    "C{std.core.String}C{application.ApplicationStateObserver.ApplicationStateObserver}C{std.core.Array}:i";
constexpr const char *APPLICATION_STATE_WITH_APP_STATE_FILTER_ON_SIGNATURE =
    "C{std.core.String}C{application.ApplicationStateObserver.ApplicationStateObserver}"
    "C{@ohos.app.ability.appManager.appManager.AppStateFilter}:i";
constexpr const char *APPLICATION_STATE_ON_SIGNATURE =
    "C{std.core.String}C{application.ApplicationStateObserver.ApplicationStateObserver}:i";
constexpr const char *APPLICATION_STATE_OFF_SIGNATURE =
    "C{std.core.String}iC{utils.AbilityUtils.AsyncCallbackWrapper}:";
static const char* ON_SIGNATURE_ABILITY_FIRST_FRAME_STATE =
    "C{std.core.String}C{application.AbilityFirstFrameStateObserver.AbilityFirstFrameStateObserver}C{std.core.String}:";
static const char* ON_SIGNATURE_APP_FOREGROUND_STATE
    = "C{std.core.String}C{application.AppForegroundStateObserver.AppForegroundStateObserver}:";
static const char *OFF_SIGNATURE_ABILITY_FIRST_FRAME_STATE
    = "C{std.core.String}C{application.AbilityFirstFrameStateObserver.AbilityFirstFrameStateObserver}:";
static const char *OFF_SIGNATURE_APP_FOREGROUND_STATE
    = "C{std.core.String}C{application.AppForegroundStateObserver.AppForegroundStateObserver}:";
constexpr const char *BUNDLE_NAME_CHECK_SIG = "C{std.core.String}:";
constexpr const char *CLEAR_UP_APPLICATION_DATA_SIG = "C{std.core.String}C{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char *GET_KEEP_ALIVE_APP_SERVICE_EXTENSION_SIG = "C{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char *SET_KEEP_ALIVE_FOR_APP_SERVICE_EXTENSION_SIG =
    "C{std.core.String}zC{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char *CLEAR_UP_APP_DATA_SIG = "C{utils.AbilityUtils.AsyncCallbackWrapper}"
    "C{std.core.String}C{std.core.Int}:";
constexpr const char *TERMINATION_SIG = "iC{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char *IS_APP_RUNNING_SIG = "C{std.core.String}C{utils.AbilityUtils.AsyncCallbackWrapper}:";
} // namespace

class EtsAppManager final {
public:
    static void PreloadApplication(ani_env *env, ani_object callback, ani_string aniBundleName, ani_int aniUserId,
        ani_enum_item aniMode, ani_object aniAppIndex);
    static void GetRunningProcessInformation(ani_env *env, ani_object callback);
    static void GetForegroundApplications(ani_env *env, ani_object callback);
    static void GetRunningMultiAppInfo(ani_env *env, ani_string aniBundleName, ani_object callback);
    static void GetRunningProcessInfoByBundleNameAndUserId(ani_env *env, ani_string aniBundleName,
        ani_int aniUserId, ani_object callback);
    static void GetRunningProcessInfoByBundleName(ani_env *env, ani_string aniBundleName, ani_object callback);
    static void GetAppMemorySize(ani_env *env, ani_object callback);
    static void IsRamConstrainedDevice(ani_env *env, ani_object callback);
    static void IsRunningInStabilityTest(ani_env *env, ani_object callback);
    static void NativeKillProcessesByBundleNameSync(ani_env *env, ani_string bundleName, ani_object callback);
    static void NativeKillProcessesByBundleName(
        ani_env *env, ani_object callback, ani_string bundleName, ani_boolean clearPageStack, ani_object stsAppIndex);
    static void NativeKillProcessWithAccountSync(ani_env *env, ani_string aniBundleName, ani_int aniAccountId,
        ani_object callback);
    static void NativeKillProcessWithAccount(ani_env *env, ani_object callback, ani_string aniBundleName,
        ani_int aniAccountId, ani_boolean clearPageStack, ani_object aniAppIndex);
    static void NativeGetProcessMemoryByPid(ani_env *env, ani_int aniPid, ani_object callback);
    static void GetRunningProcessInformationByBundleType(
        ani_env *env, ani_enum_item aniBundleType, ani_object callback);
    static void NativeIsSharedBundleRunning(ani_env *env, ani_string aniBundleName,
        ani_long aniVersionCode, ani_object callback);
    static void NativeGetSupportedProcessCachePids(ani_env *env, ani_string aniBundleName, ani_object callback);
    static void NativeKillProcessesInBatch(ani_env *env, ani_object pids, ani_object callback);
    static void NativeIsAppRunning(
        ani_env *env, ani_object callback, ani_string aniBundleName, ani_object aniAppCloneIndex);
    static void NativeSetKeepAliveForBundle(
        ani_env *env, ani_string aniBundleName, ani_int aniUserId, ani_boolean enable, ani_object callback);
    static void NativeGetKeepAliveBundles(ani_env *env, ani_object callback, ani_enum_item aniType,
        ani_object aniUserId);
    static ani_int OnOnApplicationStateWithBundleList(ani_env *env, ani_string type,
        ani_object observer, ani_object etsBundleNameList);
    static ani_int OnOnApplicationState(ani_env *env, ani_string type, ani_object observer);
    static ani_int OnOnApplicationStateWithAppStateFilter(ani_env *env,
        ani_string type, ani_object observer, ani_object etsAppStateFilter);
    static void OnOff(ani_env *env, ani_string type, ani_int etsObserverId, ani_object callback);
    static void OffApplicationStateCheck(ani_env *env, ani_int etsObserverId);
    static void OnOnAppForegroundState(ani_env *env, ani_string type, ani_object observer);
    static void OnOffAppForegroundState(ani_env *env, ani_string type, ani_object observer);
    static void OnOnAbilityFirstFrameState(
        ani_env *env, ani_string type, ani_object aniObserver, ani_object aniBundleName);
    static void OnOffAbilityFirstFrameState(ani_env *env, ani_string type, ani_object aniObserver);
    static void ClearUpApplicationDataCheck(ani_env *env, ani_string aniBundleName);
    static void ClearUpApplicationData(ani_env *env, ani_string aniBundleName, ani_object callback);
    static void GetKeepAliveAppServiceExtensions(ani_env *env, ani_object callback);
    static void SetKeepAliveForAppServiceExtension(ani_env *env, ani_string aniBundleName, ani_boolean enable,
        ani_object callback);
    static void ClearUpAppDataCheck(ani_env *env, ani_string aniBundleName);
    static void ClearUpAppData(ani_env *env, ani_object callback, ani_string aniBundleName, ani_object appCloneIndex);
    static void TerminateMission(ani_env *env, ani_int missionId, ani_object callback);
    static void IsApplicationRunning(ani_env *env, ani_string aniBundleName, ani_object callback);
    static void GetRunningMultiAppInfoCheck(ani_env *env, ani_string aniBundleName);
private:
    static sptr<AppExecFwk::IAppMgr> GetAppManagerInstance();
    static sptr<AAFwk::IAbilityManager> GetAbilityManagerInstance();
    static bool CheckOnOnApplicationStateInnerParam(ani_env *env, ani_string type, ani_object observer,
        ani_object etsBundleNameList, std::vector<std::string> &bundleNameList);
    static bool CheckOnOnApplicationStateWithAppStateFilterParam(ani_env *env, ani_string type,
        ani_object observer, const ani_object &etsAppStateFilter, OHOS::AppExecFwk::AppStateFilter &appStateFilter);
    static ani_int OnOnApplicationStateInner(
        ani_env *env, ani_string type, ani_object observer, ani_object aniBundleNameList);
    static void KillProcessesByBundleNameInner(ani_env *env, ani_object callback, ani_string etsBundleName,
        ani_boolean clearPageStack, ani_object etsAppIndex);
    static void KillProcessWithAccountInner(ani_env *env, ani_object callback, ani_string aniBundleName,
        ani_int aniAccountId, ani_boolean clearPageStack, ani_object aniAppIndex);
    static void OnOnAbilityFirstFrameStateInner(ani_env *env, ani_object aniObserver, const std::string &strBundleName);
    static void OnOffInner(ani_env *env, ani_int etsObserverId, ani_object callback);
    static int32_t GetObserverId();
    static int32_t serialNumber_;
    static sptr<AbilityRuntime::EtsAppStateObserver> appStateObserver_;
    static sptr<OHOS::AbilityRuntime::ETSAppForegroundStateObserver> observerForeground_;
    static std::mutex appStateObserverLock_;
};

int32_t EtsAppManager::serialNumber_ = 0;
sptr<AbilityRuntime::EtsAppStateObserver> EtsAppManager::appStateObserver_ = nullptr;
sptr<OHOS::AbilityRuntime::ETSAppForegroundStateObserver> EtsAppManager::observerForeground_ = nullptr;
std::mutex EtsAppManager::appStateObserverLock_;

sptr<AppExecFwk::IAppMgr> EtsAppManager::GetAppManagerInstance()
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> appObject = systemAbilityManager->GetSystemAbility(APP_MGR_SERVICE_ID);
    return iface_cast<AppExecFwk::IAppMgr>(appObject);
}

sptr<AAFwk::IAbilityManager> EtsAppManager::GetAbilityManagerInstance()
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> abilityObject =
        systemAbilityManager->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    return iface_cast<AAFwk::IAbilityManager>(abilityObject);
}

void EtsAppManager::PreloadApplication(ani_env *env, ani_object callback, ani_string aniBundleName,
    ani_int aniUserId, ani_enum_item aniMode, ani_object aniAppIndex)
{
    TAG_LOGD(AAFwkTag::APPMGR, "PreloadApplication");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env is null");
        return;
    }
    std::string bundleName;
    if (!AppExecFwk::GetStdString(env, aniBundleName, bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "param bundlename err");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateError(env,
            AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM), nullptr);
        return;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "PreloadApplication userId:%{public}d, bundleName %{public}s",
        aniUserId, bundleName.c_str());

    ani_int mode = 0;
    if (!AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative(env, aniMode, mode)) {
        TAG_LOGE(AAFwkTag::APPMGR, "param mode err");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateError(env,
            AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM), nullptr);
        return;
    }

    ani_status status = ANI_OK;
    int32_t appIndex = 0;
    ani_boolean isUndefined = false;
    if ((status = env->Reference_IsUndefined(aniAppIndex, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to check undefined status : %{public}d", status);
        return;
    }
    if (!isUndefined) {
        if ((status = env->Object_CallMethodByName_Int(aniAppIndex, "intValue", nullptr, &appIndex)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::APPMGR, "Object_CallMethodByName_Int status : %{public}d", status);
            return;
        }
        TAG_LOGD(AAFwkTag::APPMGR, "aniAppIndex: %{public}d", appIndex);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "PreloadApplication userId:%{public}d, mode:%{public}d, appIndex:%{public}d",
        aniUserId, mode, appIndex);
    sptr<OHOS::AppExecFwk::IAppMgr> appMgr = GetAppManagerInstance();
    if (appMgr == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appManager null ptr");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(
                env, static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER)), nullptr);
        return;
    }
    auto ret = appMgr->PreloadApplication(bundleName, aniUserId, static_cast<AppExecFwk::PreloadMode>(mode), appIndex);
    TAG_LOGD(AAFwkTag::APPMGR, "PreloadApplication ret %{public}d", ret);

    AppExecFwk::AsyncCallback(env, callback,
        AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret)), nullptr);
    TAG_LOGD(AAFwkTag::APPMGR, "PreloadApplication END");
}

void EtsAppManager::GetRunningProcessInformation(ani_env *env, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPMGR, "GetRunningProcessInformation called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env null ptr");
        return;
    }
    ani_object emptyArray = CreateEmptyAniArray(env);
    sptr<AppExecFwk::IAppMgr> appMgr = GetAppManagerInstance();
    if (appMgr == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appManager null ptr");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(
                env, static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER)), emptyArray);
        return;
    }
    std::vector<AppExecFwk::RunningProcessInfo> infos;
    auto ret = appMgr->GetAllRunningProcesses(infos);
    TAG_LOGD(AAFwkTag::APPMGR, "GetAllRunningProcesses ret:%{public}d, size:%{public}zu", ret, infos.size());
    if (ret != ERR_OK) {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret)), emptyArray);
        return;
    }
    ani_object aniInfosRef = CreateRunningProcessInfoArray(env, infos);
    if (aniInfosRef == nullptr) {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(
                env, static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER)), emptyArray);
    } else {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret)), aniInfosRef);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "GetRunningProcessInformation finished");
}

void EtsAppManager::GetForegroundApplications(ani_env *env, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPMGR, "GetForegroundApplications called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env null ptr");
        return;
    }
    ani_object emptyArray = CreateEmptyAniArray(env);
    auto appManager = GetAppManagerInstance();
    if (appManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appManager null ptr");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(
                env, static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER)), emptyArray);
        return;
    }
    std::vector<AppExecFwk::AppStateData> appStateData;
    int32_t ret = appManager->GetForegroundApplications(appStateData);
    TAG_LOGD(AAFwkTag::APPMGR, "GetForegroundApplications ret:%{public}d, size:%{public}zu", ret, appStateData.size());
    if (ret != ERR_OK) {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret)), emptyArray);
        return;
    }
    ani_object appStateDataObj = CreateAppStateDataArray(env, appStateData);
    if (appStateDataObj == nullptr) {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(
                env, static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER)), emptyArray);
    } else {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret)),
            appStateDataObj);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "GetForegroundApplications end");
}


void EtsAppManager::GetRunningMultiAppInfoCheck(ani_env *env, ani_string aniBundleName)
{
    TAG_LOGD(AAFwkTag::APPMGR, "GetRunningMultiAppInfoCheck called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env null");
        return;
    }
    std::string bundleName;
    if (!AppExecFwk::GetStdString(env, aniBundleName, bundleName) || bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetStdString Failed");
        AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env, "Parse param bundleName failed, must be a string.");
        return;
    }
#ifdef SUPPORT_SCREEN
    if (!AppExecFwk::CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Non-system app");
        AbilityRuntime::EtsErrorUtil::ThrowError(
            env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return;
    }
#endif
}

void EtsAppManager::GetRunningMultiAppInfo(ani_env *env, ani_string aniBundleName, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPMGR, "GetRunningMultiAppInfo called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env null ptr");
        return;
    }
    ani_object emptyMultiAppInfo = CreateEmptyMultiAppInfo(env);
#ifdef SUPPORT_SCREEN
    if (!AppExecFwk::CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Non-system app");
        return;
    }
#endif
    std::string bundleName;
    if (!AppExecFwk::GetStdString(env, aniBundleName, bundleName) || bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetStdString Failed");
        return;
    }
    auto appManager = GetAppManagerInstance();
    if (appManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appManager nullptr");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(
                env, static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER)), emptyMultiAppInfo);
        return;
    }
    AppExecFwk::RunningMultiAppInfo info;
    int32_t innerErrorCode = ERR_OK;
    innerErrorCode = appManager->GetRunningMultiAppInfoByBundleName(bundleName, info);
    TAG_LOGD(AAFwkTag::APPMGR, "GetRunningMultiAppInfoByBundleName ret: %{public}d", innerErrorCode);
    if (innerErrorCode != ERR_OK) {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, innerErrorCode), emptyMultiAppInfo);
        return;
    }
    ani_object appinfoObj = WrapRunningMultiAppInfo(env, info);
    if (appinfoObj == nullptr) {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(
                env, static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER)), emptyMultiAppInfo);
    } else {
        AppExecFwk::AsyncCallback(
            env, callback, AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, innerErrorCode), appinfoObj);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "GetRunningMultiAppInfo end");
}

void EtsAppManager::GetRunningProcessInfoByBundleNameAndUserId(
    ani_env *env, ani_string aniBundleName, ani_int aniUserId, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPMGR, "GetRunningProcessInfoByBundleNameAndUserId called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env null ptr");
        return;
    }
    ani_object emptyArray = CreateEmptyAniArray(env);
    if (aniBundleName == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "aniBundleName null ptr");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateError(env,
            AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM), emptyArray);
        return;
    }
    std::string bundleName;
    if (!AppExecFwk::GetStdString(env, aniBundleName, bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetStdString Failed");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateError(env,
            AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM), emptyArray);
        return;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "GetRunningProcessInfoByBundleNameAndUserId userid:%{public}d", aniUserId);
    auto appManager = GetAppManagerInstance();
    if (appManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appManager nullptr");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(
                env, static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER)), emptyArray);
        return;
    }
    std::vector<AppExecFwk::RunningProcessInfo> infos;
    int32_t ret = appManager->GetRunningProcessInformation(bundleName, aniUserId, infos);
    TAG_LOGD(AAFwkTag::APPMGR, "GetRunningProcessInformation ret: %{public}d, size:%{public}zu", ret, infos.size());
    if (ret != ERR_OK) {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret)), emptyArray);
        return;
    }
    ani_object aniInfos = CreateRunningProcessInfoArray(env, infos);
    if (aniInfos == nullptr) {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(
                env, static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER)), emptyArray);
    } else {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret)), aniInfos);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "GetRunningProcessInfoByBundleNameAndUserId finished");
}

void EtsAppManager::GetRunningProcessInfoByBundleName(ani_env *env, ani_string aniBundleName, ani_object callback)
{
    int userId = IPCSkeleton::GetCallingUid() / AppExecFwk::Constants::BASE_USER_RANGE;
    GetRunningProcessInfoByBundleNameAndUserId(env, aniBundleName, userId, callback);
}

int32_t EtsAppManager::GetObserverId()
{
    int32_t observerId = serialNumber_;
    if (serialNumber_ < INT32_MAX) {
        serialNumber_++;
    } else {
        serialNumber_ = 0;
    }
    return observerId;
}

bool EtsAppManager::CheckOnOnApplicationStateInnerParam(ani_env *env, ani_string type, ani_object observer,
    ani_object etsBundleNameList, std::vector<std::string> &bundleNameList)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env null ptr");
        return false;
    }
    std::string strType;
    if (!AppExecFwk::GetStdString(env, type, strType) || strType != ON_OFF_TYPE) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetStdString failed");
        AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(
            env, "Parse param type failed, must be a string, value must be applicationState.");
        return false;
    }
    ani_boolean isUndefined = false;
    ani_status status = ANI_OK;
    if ((status = env->Reference_IsUndefined(etsBundleNameList, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to check undefined status : %{public}d", status);
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return false;
    }
    if (!isUndefined && !UnWrapArrayString(env, etsBundleNameList, bundleNameList)) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetStdString failed");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return false;
    }
    return true;
}

ani_int EtsAppManager::OnOnApplicationStateInner(ani_env *env, ani_string type, ani_object observer,
    ani_object etsBundleNameList)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OnOnApplicationStateInner called");
    std::vector<std::string> bundleNameList;
    if (!CheckOnOnApplicationStateInnerParam(env, type, observer, etsBundleNameList, bundleNameList)) {
        return ANI_ERROR;
    }
    auto appManager = GetAppManagerInstance();
    if (appManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appManager");
        AbilityRuntime::EtsErrorUtil::ThrowError(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER));
        return ANI_ERROR;
    }
    ani_vm *aniVM = nullptr;
    if (env->GetVM(&aniVM) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "get aniVM failed");
        AbilityRuntime::EtsErrorUtil::ThrowError(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER));
        return ANI_ERROR;
    }
    {
        std::lock_guard<std::mutex> lock(appStateObserverLock_);
        if (appStateObserver_ == nullptr) {
            appStateObserver_ = new (std::nothrow) AbilityRuntime::EtsAppStateObserver(aniVM);
            if (appStateObserver_ == nullptr) {
                TAG_LOGE(AAFwkTag::UI_EXT, "create appStateObserver_ failed");
                AbilityRuntime::EtsErrorUtil::ThrowError(env,
                    static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER));
                return ANI_ERROR;
            }
        }
    }
    if (appStateObserver_->GetEtsObserverMapSize() == 0) {
        int32_t ret = appManager->RegisterApplicationStateObserver(appStateObserver_, bundleNameList);
        TAG_LOGD(AAFwkTag::APPMGR, "err:%{public}d", ret);
        if (ret != ERR_OK) {
            AbilityRuntime::EtsErrorUtil::ThrowErrorByNativeErr(env, static_cast<int32_t>(ret));
            return ANI_ERROR;
        }
    }
    int32_t observerId = GetObserverId();
    appStateObserver_->AddEtsObserverObject(env, observerId, observer);
    TAG_LOGD(AAFwkTag::APPMGR, "OnOnApplicationStateInner end");
    return observerId;
}

ani_int EtsAppManager::OnOnApplicationStateWithBundleList(ani_env *env, ani_string type,
    ani_object observer, ani_object etsBundleNameList)
{
    return OnOnApplicationStateInner(env, type, observer, etsBundleNameList);
}

ani_int EtsAppManager::OnOnApplicationState(ani_env *env, ani_string type, ani_object observer)
{
    ani_ref undefined = nullptr;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env null ptr");
        return ANI_ERROR;
    }
    env->GetUndefined(&undefined);
    return OnOnApplicationStateInner(env, type, observer, static_cast<ani_object>(undefined));
}

bool EtsAppManager::CheckOnOnApplicationStateWithAppStateFilterParam(ani_env *env, ani_string type,
    ani_object observer, const ani_object &etsAppStateFilter, OHOS::AppExecFwk::AppStateFilter &appStateFilter)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env null ptr");
        return false;
    }
    std::string strType;
    if (!AppExecFwk::GetStdString(env, type, strType) || strType != ON_OFF_TYPE) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetStdString failed");
        AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(
            env, "Parse param type failed, must be a string, value must be applicationState.");
        return false;
    }
    ani_boolean isUndefined = false;
    ani_status status = ANI_OK;
    if ((status = env->Reference_IsUndefined(etsAppStateFilter, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to check undefined status : %{public}d", status);
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return false;
    }
    if (!isUndefined && !UnWrapAppStateFilter(env, etsAppStateFilter, appStateFilter)) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetAppStateFilter failed");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return false;
    }
    return true;
}

ani_int EtsAppManager::OnOnApplicationStateWithAppStateFilter(ani_env *env, ani_string type,
    ani_object observer, ani_object etsAppStateFilter)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OnOnApplicationStateWithAppStateFilter called");
    OHOS::AppExecFwk::AppStateFilter appStateFilter = OHOS::AppExecFwk::AppStateFilter();
    if (!CheckOnOnApplicationStateWithAppStateFilterParam(env, type, observer, etsAppStateFilter, appStateFilter)) {
        return ANI_ERROR;
    }
    auto appManager = GetAppManagerInstance();
    if (appManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appManager");
        AbilityRuntime::EtsErrorUtil::ThrowError(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER));
        return ANI_ERROR;
    }
    ani_vm *aniVM = nullptr;
    if (env->GetVM(&aniVM) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "get aniVM failed");
        AbilityRuntime::EtsErrorUtil::ThrowError(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER));
        return ANI_ERROR;
    }
    {
        std::lock_guard<std::mutex> lock(appStateObserverLock_);
        if (appStateObserver_ == nullptr) {
            appStateObserver_ = new (std::nothrow) AbilityRuntime::EtsAppStateObserver(aniVM);
            if (appStateObserver_ == nullptr) {
                TAG_LOGE(AAFwkTag::UI_EXT, "create appStateObserver_ failed");
                AbilityRuntime::EtsErrorUtil::ThrowError(env,
                    static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER));
                return ANI_ERROR;
            }
        }
    }
    std::vector<std::string> bundleNameList;
    int32_t ret = appManager->RegisterApplicationStateObserverWithFilter(
        appStateObserver_, bundleNameList, appStateFilter, true);
    TAG_LOGD(AAFwkTag::APPMGR, "err:%{public}d", ret);
    if (ret == ERR_OK) {
        int32_t observerId = GetObserverId();
        appStateObserver_->AddEtsObserverObject(env, observerId, observer);
        TAG_LOGD(AAFwkTag::APPMGR, "OnOnApplicationStateInner end");
        return observerId;
    }
    AbilityRuntime::EtsErrorUtil::ThrowErrorByNativeErr(env, static_cast<int32_t>(ret));
    return ANI_ERROR;
}

void EtsAppManager::OffApplicationStateCheck(ani_env *env, ani_int etsObserverId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OffApplicationStateCheck called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null env");
        return;
    }
    if (appStateObserver_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null observer");
        AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env, "observer is nullptr, please register first.");
        return;
    }
    int32_t observerId = static_cast<int32_t>(etsObserverId);
    if (!appStateObserver_->FindObserverByObserverId(observerId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "not find observer:%{public}d", static_cast<int32_t>(observerId));
        AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env, "not find observerId.");
        return;
    }
}

void EtsAppManager::OnOff(ani_env *env, ani_string type, ani_int etsObserverId, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OnOff called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env null ptr");
        return;
    }
    std::string strType;
    if (!AppExecFwk::GetStdString(env, type, strType) || strType != ON_OFF_TYPE) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetStdString failed");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateInvalidParamError(
                env, "Parse param type failed, must be a string, value must be applicationState."), nullptr);
        return;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "observerId:%{public}d", etsObserverId);
    if (appStateObserver_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null observer");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateInvalidParamError(
                env, "observer is nullptr, please register first."), nullptr);
        return;
    }
    OnOffInner(env, etsObserverId, callback);
}

void EtsAppManager::OnOffInner(ani_env *env, ani_int etsObserverId, ani_object callback)
{
    int32_t observerId = static_cast<int32_t>(etsObserverId);
    if (!appStateObserver_->FindObserverByObserverId(observerId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "not find observer:%{public}d", static_cast<int32_t>(observerId));
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateInvalidParamError(
                env, "not find observerId."), nullptr);
        return;
    }
    if (!appStateObserver_->RemoveEtsObserverObject(observerId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "OnOff RemoveEtsObserverObject err:");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateError(env,
                static_cast<AbilityRuntime::AbilityErrorCode>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER)),
            nullptr);
        return;
    }
    int32_t ret = 0;
    if (appStateObserver_->GetEtsObserverMapSize() == 0) {
        sptr<AppExecFwk::IAppMgr> appMgr = GetAppManagerInstance();
        if (appMgr == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "appManager null ptr");
            AppExecFwk::AsyncCallback(env, callback,
                AbilityRuntime::EtsErrorUtil::CreateError(env,
                    static_cast<AbilityRuntime::AbilityErrorCode>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER)),
                nullptr);
            return;
        }
        ret = appMgr->UnregisterApplicationStateObserver(appStateObserver_);
        if (ret != 0) {
            TAG_LOGE(AAFwkTag::APPMGR, "OnOff err:%{public}d", ret);
        }
    }
    AppExecFwk::AsyncCallback(env, callback,
        AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret)), nullptr);
    TAG_LOGD(AAFwkTag::APPMGR, "OnOff end");
}

void EtsAppManager::GetAppMemorySize(ani_env *env, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPMGR, "GetAppMemorySize called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env null ptr");
        return;
    }
    auto abilityManager = GetAbilityManagerInstance();
    if (abilityManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "abilityManager null ptr");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER)),
            AppExecFwk::CreateInt(env, ERR_FAILURE));
        return;
    }
    int32_t memorySize = abilityManager->GetAppMemorySize();
    TAG_LOGD(AAFwkTag::APPMGR, "memorySize:%{public}d", memorySize);

    AppExecFwk::AsyncCallback(env, callback,
        AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ERR_OK)),
        AppExecFwk::CreateInt(env, memorySize));
    TAG_LOGD(AAFwkTag::APPMGR, "GetAppMemorySize end");
}

void EtsAppManager::IsRamConstrainedDevice(ani_env *env, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPMGR, "IsRamConstrainedDevice called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env null ptr");
        return;
    }
    auto abilityManager = GetAbilityManagerInstance();
    if (abilityManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "abilityManager null ptr");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER)),
            AppExecFwk::CreateBoolean(env, false));
        return;
    }
    bool ret = abilityManager->IsRamConstrainedDevice();
    TAG_LOGD(AAFwkTag::APPMGR, "IsRamConstrainedDevice:%{public}d", ret);

    AppExecFwk::AsyncCallback(env, callback,
        AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ERR_OK)),
        AppExecFwk::CreateBoolean(env, static_cast<ani_boolean>(ret)));
    TAG_LOGD(AAFwkTag::APPMGR, "IsRamConstrainedDevice end");
}

void EtsAppManager::IsRunningInStabilityTest(ani_env *env, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPMGR, "IsRunningInStabilityTest called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env null ptr");
        return;
    }
    auto abilityManager = GetAbilityManagerInstance();
    if (abilityManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "abilityManager null ptr");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER)),
            AppExecFwk::CreateBoolean(env, false));
        return;
    }
    bool ret = abilityManager->IsRunningInStabilityTest();
    TAG_LOGD(AAFwkTag::APPMGR, "IsRunningInStabilityTest:%{public}d", ret);

    AppExecFwk::AsyncCallback(env, callback,
        AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ERR_OK)),
        AppExecFwk::CreateBoolean(env, static_cast<ani_boolean>(ret)));
    TAG_LOGD(AAFwkTag::APPMGR, "IsRunningInStabilityTest end");
}

void EtsAppManager::NativeKillProcessesByBundleNameSync(ani_env *env, ani_string bundleName, ani_object callback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env null ptr");
        return;
    }
    ani_ref undefined = nullptr;
    env->GetUndefined(&undefined);
    KillProcessesByBundleNameInner(env, callback, bundleName, false, static_cast<ani_object>(undefined));
}

void EtsAppManager::NativeKillProcessesByBundleName(ani_env *env, ani_object callback, ani_string bundleName,
    ani_boolean clearPageStack, ani_object etsAppIndex)
{
    KillProcessesByBundleNameInner(env, callback, bundleName, clearPageStack, etsAppIndex);
}

void EtsAppManager::KillProcessesByBundleNameInner(ani_env *env, ani_object callback, ani_string etsBundleName,
    ani_boolean clearPageStack, ani_object etsAppIndex)
{
    TAG_LOGD(AAFwkTag::APPMGR, "KillProcessesByBundleNameInner called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env null ptr");
        return;
    }
    auto abilityManager = GetAbilityManagerInstance();
    if (abilityManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "abilityManager null ptr");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER)), nullptr);
        return;
    }
    ani_status status = ANI_OK;
    std::string bundleName;
    if (!AppExecFwk::GetStdString(env, etsBundleName, bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetStdString Failed");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateInvalidParamError(
                env, "Parse param bundleName failed, must be a string."), nullptr);
        return;
    }
    int32_t appIndex = 0;
    ani_boolean isUndefined = false;
    if ((status = env->Reference_IsUndefined(etsAppIndex, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to check undefined status : %{public}d", status);
        return;
    }
    if (!isUndefined) {
        if ((status = env->Object_CallMethodByName_Int(etsAppIndex,
            "toInt", nullptr, &appIndex)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::APPMGR, "Object_CallMethodByName_Int status : %{public}d", status);
            return;
        }
        TAG_LOGD(AAFwkTag::APPMGR, "etsAppIndex: %{public}d", appIndex);
    }
    auto ret = abilityManager->KillProcess(bundleName, clearPageStack, appIndex);
    TAG_LOGD(AAFwkTag::APPMGR, "KillProcess ret: %{public}d", ret);
    AppExecFwk::AsyncCallback(env, callback,
        AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret)), nullptr);
    TAG_LOGD(AAFwkTag::APPMGR, "KillProcessesByBundleNameInner end");
}

void EtsAppManager::NativeKillProcessWithAccountSync(ani_env *env, ani_string aniBundleName, ani_int aniAccountId,
    ani_object callback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env null ptr");
        return;
    }
    ani_ref undefined = nullptr;
    env->GetUndefined(&undefined);
    KillProcessWithAccountInner(env, callback, aniBundleName, aniAccountId,
        false, static_cast<ani_object>(undefined));
}

void EtsAppManager::NativeKillProcessWithAccount(ani_env *env, ani_object callback, ani_string aniBundleName,
    ani_int aniAccountId, ani_boolean clearPageStack, ani_object aniAppIndex)
{
    KillProcessWithAccountInner(env, callback, aniBundleName, aniAccountId,
        clearPageStack, aniAppIndex);
}

void EtsAppManager::KillProcessWithAccountInner(ani_env *env, ani_object callback, ani_string aniBundleName,
    ani_int aniAccountId, ani_boolean clearPageStack, ani_object aniAppIndex)
{
    TAG_LOGD(AAFwkTag::APPMGR, "KillProcessWithAccountInner called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env null ptr");
        return;
    }
    sptr<AppExecFwk::IAppMgr> appMgr = GetAppManagerInstance();
    if (appMgr == nullptr || appMgr->GetAmsMgr() == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appManager null ptr");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER)), nullptr);
        return;
    }
    std::string bundleName;
    if (!AppExecFwk::GetStdString(env, aniBundleName, bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetStdString Failed");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateInvalidParamError(
                env, "Parse param bundleName failed, must be a string."), nullptr);
        return;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "KillProcessWithAccount accountId:%{public}d", aniAccountId);
    int32_t appIndex = 0;
    ani_status status = ANI_OK;
    ani_boolean isUndefined = false;
    if ((status = env->Reference_IsUndefined(aniAppIndex, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to check undefined status : %{public}d", status);
        return;
    }
    if (!isUndefined) {
        if ((status = env->Object_CallMethodByName_Int(aniAppIndex,
            "toInt", nullptr, &appIndex)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::APPMGR, "Object_CallMethodByName_Int status : %{public}d", status);
            return;
        }
        TAG_LOGD(AAFwkTag::APPMGR, "stsAppIndex: %{public}d", appIndex);
    }
    auto ret = appMgr->GetAmsMgr()->KillProcessWithAccount(bundleName, aniAccountId, clearPageStack, appIndex);
    TAG_LOGD(AAFwkTag::APPMGR, "KillProcessWithAccount ret: %{public}d", ret);
    AppExecFwk::AsyncCallback(env, callback,
        AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret)), nullptr);
    TAG_LOGD(AAFwkTag::APPMGR, "KillProcessWithAccount end");
}

void EtsAppManager::NativeGetProcessMemoryByPid(ani_env *env, ani_int aniPid, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPMGR, "NativeGetProcessMemoryByPid called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env null ptr");
        return;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "NativeGetProcessMemoryByPid pid:%{public}d", aniPid);

    sptr<AppExecFwk::IAppMgr> appMgr = GetAppManagerInstance();
    if (appMgr == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appManager null ptr");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER)),
            AppExecFwk::CreateInt(env, ERR_FAILURE));
        return;
    }
    int32_t memSize = 0;
    int32_t ret = appMgr->GetProcessMemoryByPid(aniPid, memSize);
    TAG_LOGD(AAFwkTag::APPMGR, "NativeGetProcessMemoryByPid memSize: %{public}d, ret:%{public}d", memSize, ret);
    AppExecFwk::AsyncCallback(env, callback,
        AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret)),
        AppExecFwk::CreateInt(env, memSize));
    TAG_LOGD(AAFwkTag::APPMGR, "NativeGetProcessMemoryByPid end");
}

void EtsAppManager::GetRunningProcessInformationByBundleType(
    ani_env *env, ani_enum_item aniBundleType, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPMGR, "GetRunningProcessInformationByBundleType called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env null ptr");
        return;
    }
    ani_object emptyArray = CreateEmptyAniArray(env);
    sptr<AppExecFwk::IAppMgr> appMgr = GetAppManagerInstance();
    if (appMgr == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appManager null ptr");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER)), emptyArray);
        return;
    }
    ani_int bundleType;
    if (!AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative(env, aniBundleType, bundleType)) {
        TAG_LOGE(AAFwkTag::APPMGR, "param aniBundleType err");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateInvalidParamError(
                env, "Parse param bundleType failed, must be a BundleType."), emptyArray);
        return;
    }
    std::vector<AppExecFwk::RunningProcessInfo> infos;
    auto ret = appMgr->GetRunningProcessesByBundleType(static_cast<AppExecFwk::BundleType>(bundleType), infos);
    TAG_LOGD(AAFwkTag::APPMGR, "GetRunningProcessInformationByBundleType ret:%{public}d, size:%{public}zu",
        ret, infos.size());
    if (ret != ERR_OK) {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret)), emptyArray);
        return;
    }
    ani_object aniInfosRef = CreateRunningProcessInfoArray(env, infos);
    if (aniInfosRef == nullptr) {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER)), emptyArray);
    } else {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret)), aniInfosRef);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "GetRunningProcessInformationByBundleType end");
}

void EtsAppManager::NativeIsSharedBundleRunning(ani_env *env, ani_string aniBundleName,
    ani_long aniVersionCode, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPMGR, "NativeIsSharedBundleRunning called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env null ptr");
        return;
    }
    sptr<AppExecFwk::IAppMgr> appMgr = GetAppManagerInstance();
    if (appMgr == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appManager null ptr");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER)),
            AppExecFwk::CreateBoolean(env, false));
        return;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "NativeGetProcessMemoryByPid pid:%{public}" PRId64,
        static_cast<int64_t>(aniVersionCode));
    uint32_t versionCode = static_cast<uint32_t>(aniVersionCode);

    std::string bundleName;
    if (!AppExecFwk::GetStdString(env, aniBundleName, bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetStdString Failed");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateInvalidParamError(
                env, "Parse param bundleName failed, must be a string."), AppExecFwk::CreateBoolean(env, false));
        return;
    }
    bool ret = appMgr->IsSharedBundleRunning(bundleName, versionCode);
    TAG_LOGD(AAFwkTag::APPMGR, "NativeIsSharedBundleRunning ret :%{public}d", ret);
    AppExecFwk::AsyncCallback(env, callback,
        AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ERR_OK)),
        AppExecFwk::CreateBoolean(env, static_cast<ani_boolean>(ret)));
    TAG_LOGD(AAFwkTag::APPMGR, "NativeIsSharedBundleRunning end");
}

void EtsAppManager::NativeGetSupportedProcessCachePids(ani_env *env, ani_string aniBundleName, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPMGR, "NativeGetSupportedProcessCachePids called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env null ptr");
        return;
    }
    ani_object emptyArray = CreateEmptyAniArray(env);
    sptr<AppExecFwk::IAppMgr> appMgr = GetAppManagerInstance();
    if (appMgr == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appManager null ptr");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER)), emptyArray);
        return;
    }
    std::string bundleName;
    if (!AppExecFwk::GetStdString(env, aniBundleName, bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetStdString Failed");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateInvalidParamError(
                env, "Parse param bundleName failed, must be a string."), emptyArray);
        return;
    }
    std::vector<int32_t> list;
    int32_t ret = appMgr->GetSupportedProcessCachePids(bundleName, list);
    TAG_LOGD(AAFwkTag::APPMGR, "GetSupportedProcessCachePids ret:%{public}d, size:%{public}zu", ret, list.size());
    if (ret != ERR_OK) {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret)), emptyArray);
        return;
    }
    ani_object arrayObj = CreateIntAniArray(env, list);
    if (arrayObj == nullptr) {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER)), emptyArray);
    } else {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret)), arrayObj);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "NativeGetSupportedProcessCachePids end");
}

void EtsAppManager::NativeKillProcessesInBatch(ani_env *env, ani_object pids, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPMGR, "NativeKillProcessesInBatch called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env null ptr");
        return;
    }
    sptr<AppExecFwk::IAppMgr> appMgr = GetAppManagerInstance();
    if (appMgr == nullptr || appMgr->GetAmsMgr() == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appManager null ptr");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER)), nullptr);
        return;
    }
    std::vector<int32_t> pidList;
    if (!UnWrapArrayInt(env, pids, pidList)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Parse pids failed");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateInvalidParamError(
                env, "Parse param pids failed, must be array of numbers."), nullptr);
        return;
    }
    int32_t innerErrorCode = appMgr->GetAmsMgr()->KillProcessesInBatch(pidList);
    TAG_LOGD(AAFwkTag::APPMGR, "NativeKillProcessesInBatch ret:%{public}d", innerErrorCode);
    AppExecFwk::AsyncCallback(env, callback,
        AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(innerErrorCode)), nullptr);
    TAG_LOGD(AAFwkTag::APPMGR, "NativeKillProcessesInBatch end");
}

void EtsAppManager::NativeIsAppRunning(ani_env *env, ani_object callback, ani_string aniBundleName,
    ani_object aniAppCloneIndex)
{
    TAG_LOGD(AAFwkTag::APPMGR, "NativeIsAppRunning called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env null ptr");
        return;
    }
    sptr<AppExecFwk::IAppMgr> appMgr = GetAppManagerInstance();
    if (appMgr == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appManager null ptr");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER)),
            AppExecFwk::CreateBoolean(env, false));
        return;
    }
    std::string bundleName;
    if (!AppExecFwk::GetStdString(env, aniBundleName, bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetStdString Failed");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateInvalidParamError(
                env, "Parse param bundleName failed, must be a string."), AppExecFwk::CreateBoolean(env, false));
        return;
    }
    ani_status status = ANI_OK;
    int32_t appCloneIndex = 0;
    ani_boolean isUndefined = false;
    if ((status = env->Reference_IsUndefined(aniAppCloneIndex, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to check undefined status : %{public}d", status);
        return;
    }
    if (!isUndefined) {
        if ((status = env->Object_CallMethodByName_Int(aniAppCloneIndex,
            "toInt", nullptr, &appCloneIndex)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::APPMGR, "Object_CallMethodByName_Int status : %{public}d", status);
            return;
        }
        TAG_LOGD(AAFwkTag::APPMGR, "aniAppCloneIndex: %{public}d", appCloneIndex);
    }
    bool isRunnig = false;
    int32_t innerErrorCode = appMgr->IsAppRunning(bundleName, appCloneIndex, isRunnig);
    TAG_LOGD(AAFwkTag::APPMGR, "innerErrorCode:%{public}d, isRunning:%{public}d", innerErrorCode, isRunnig);
    if (innerErrorCode == ERR_OK) {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(innerErrorCode)),
            AppExecFwk::CreateBoolean(env, isRunnig));
    } else {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(innerErrorCode)),
            AppExecFwk::CreateBoolean(env, false));
    }
    TAG_LOGD(AAFwkTag::APPMGR, "NativeIsAppRunning end");
}

void EtsAppManager::NativeSetKeepAliveForBundle(ani_env *env, ani_string aniBundleName,
    ani_int aniUserId, ani_boolean enable, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPMGR, "NativeSetKeepAliveForBundle called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env null ptr");
        return;
    }
    auto abilityManager = GetAbilityManagerInstance();
    if (abilityManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "abilityManager null ptr");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateError(
                env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER), nullptr);
        return;
    }
    std::string bundleName;
    if (!AppExecFwk::GetStdString(env, aniBundleName, bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetStdString Failed");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateInvalidParamError(
                env, "Parse param bundleName failed, must be a string."), nullptr);
        return;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "KillProcessWithAccount aniUserId:%{public}d", aniUserId);
    int32_t innerErrCode = abilityManager->SetApplicationKeepAlive(bundleName, aniUserId, enable);
    TAG_LOGD(AAFwkTag::APPMGR, "innerErrCode:%{public}d", innerErrCode);
    AppExecFwk::AsyncCallback(env, callback,
        AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(innerErrCode)), nullptr);
    TAG_LOGD(AAFwkTag::APPMGR, "NativeSetKeepAliveForBundle end");
}

void EtsAppManager::NativeGetKeepAliveBundles(ani_env *env, ani_object callback, ani_enum_item aniType,
    ani_object aniUserId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "NativeGetKeepAliveBundles called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env null ptr");
        return;
    }
    ani_object emptyArray = CreateEmptyAniArray(env);
    auto abilityManager = GetAbilityManagerInstance();
    if (abilityManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "abilityManager null ptr");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateError(
                env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER), nullptr);
        return;
    }
    ani_int appType = 0;
    if (!AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative(env, aniType, appType)) {
        TAG_LOGE(AAFwkTag::APPMGR, "param mode err");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateInvalidParamError(
                env, "Parse param appType failed, must be a number."), nullptr);
        return;
    }
    ani_status status = ANI_OK;
    int32_t userId = -1;
    ani_boolean isUndefined = false;
    if ((status = env->Reference_IsUndefined(aniUserId, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to check undefined status : %{public}d", status);
        return;
    }
    if (!isUndefined) {
        if ((status = env->Object_CallMethodByName_Int(aniUserId,
            "toInt", nullptr, &userId)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::APPMGR, "Object_CallMethodByName_Int status : %{public}d", status);
            return;
        }
        TAG_LOGD(AAFwkTag::APPMGR, "stsAppIndex: %{public}d", userId);
    }
    std::vector<AbilityRuntime::KeepAliveInfo> infoList;
    int32_t innerErrCode = abilityManager->QueryKeepAliveApplications(appType, userId, infoList);
    TAG_LOGD(AAFwkTag::APPMGR, "GetSupportedProcessCachePids innerErrCode:%{public}d, size:%{public}zu",
        innerErrCode, infoList.size());
    if (innerErrCode != ERR_OK) {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(innerErrCode)), emptyArray);
        return;
    }
    ani_object arrayObj = CreateKeepAliveInfoArray(env, infoList);
    if (arrayObj == nullptr) {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER)), emptyArray);
    } else {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(innerErrCode)), arrayObj);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "NativeGetKeepAliveBundles end");
}

void EtsAppManager::OnOnAbilityFirstFrameStateInner(
    ani_env *env, ani_object aniObserver, const std::string &strBundleName)
{
#ifdef SUPPORT_SCREEN
    ani_vm *aniVM = nullptr;
    if (env->GetVM(&aniVM) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "get aniVM failed");
        AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env, "Get aniVm failed.");
        return;
    }
    auto abilityManager = GetAbilityManagerInstance();
    if (abilityManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "abilityManager null ptr");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }
    sptr<AbilityRuntime::ETSAbilityFirstFrameStateObserver> observer =
        new (std::nothrow) AbilityRuntime::ETSAbilityFirstFrameStateObserver(aniVM);
    if (observer == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null abilityMgr_ or observer");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }
    if (AbilityRuntime::ETSAbilityFirstFrameStateObserverManager::GetInstance()->IsObserverObjectExist(aniObserver)) {
        TAG_LOGE(AAFwkTag::APPMGR, "observer exist");
        return;
    }
    int32_t ret = abilityManager->RegisterAbilityFirstFrameStateObserver(observer, strBundleName);
    TAG_LOGD(AAFwkTag::APPMGR, "ret: %{public}d", ret);
    if (ret != NO_ERROR) {
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }
    observer->SetEtsObserverObject(aniObserver);
    AbilityRuntime::ETSAbilityFirstFrameStateObserverManager::GetInstance()->AddEtsAbilityFirstFrameStateObserver(
        observer);
#endif
}

void EtsAppManager::OnOnAbilityFirstFrameState(
    ani_env *env, ani_string type, ani_object aniObserver, ani_object aniBundleName)
{
#ifdef SUPPORT_SCREEN
    TAG_LOGD(AAFwkTag::APPMGR, "OnOnAbilityFirstFrameState called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env null ptr");
        return;
    }
    if (!AppExecFwk::CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Non-system app");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return;
    }
    std::string strType;
    if (!OHOS::AppExecFwk::GetStdString(env, type, strType)
        && strType != ON_OFF_TYPE_ABILITY_FIRST_FRAME_STATE) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetStdString failed");
        AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env,
            "Parse param observer failed, must be a AbilityFirstFrameStateObserver.");
        return;
    }
    ani_status status = ANI_OK;
    std::string strBundleName;
    ani_boolean isUndefined;
    if ((status = env->Reference_IsUndefined(aniBundleName, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to check undefined status : %{public}d", status);
        return;
    }
    if (!isUndefined && !OHOS::AppExecFwk::GetStdString(env,
        reinterpret_cast<ani_string>(aniBundleName), strBundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetStdString failed");
        AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env, "Parse param bundleName failed, must be a string.");
        return;
    }
    OnOnAbilityFirstFrameStateInner(env, aniObserver, strBundleName);
    TAG_LOGD(AAFwkTag::APPMGR, "OnOnAbilityFirstFrameState end");
#endif
}

void EtsAppManager::OnOnAppForegroundState(ani_env *env, ani_string type, ani_object observer)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OnOnAppForegroundState called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env null ptr");
        return;
    }
    ani_vm *aniVM = nullptr;
    if (env->GetVM(&aniVM) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "get aniVM failed");
        AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env, "Get aniVm failed.");
        return;
    }
    std::string strType;
    if (!OHOS::AppExecFwk::GetStdString(env, type, strType)
        && strType != ON_OFF_TYPE_APP_FOREGROUND_STATE) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetStdString failed");
        AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env,
            "Parse param observer failed, must be a AppForegroundStateObserver.");
        return;
    }

    sptr<OHOS::AppExecFwk::IAppMgr> appMgr = GetAppManagerInstance();
    if (appMgr == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appManager null ptr");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }
    //Create Observer
    if (observerForeground_ == nullptr) {
        observerForeground_ = new (std::nothrow) AbilityRuntime::ETSAppForegroundStateObserver(aniVM);
    }
    if (observerForeground_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appMgr or observer");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }
    if (observerForeground_->IsEmpty()) {
        int32_t ret = appMgr->RegisterAppForegroundStateObserver(observerForeground_);
        TAG_LOGD(AAFwkTag::APPMGR, "RegisterAppForegroundStateObserver ret: %{public}d", ret);
        if (ret != NO_ERROR) {
            AbilityRuntime::EtsErrorUtil::ThrowErrorByNativeErr(env, static_cast<int32_t>(ret));
            return;
        }
    }
    observerForeground_->AddEtsObserverObject(observer);
    TAG_LOGD(AAFwkTag::APPMGR, "OnOnAppForegroundState end");
}

void EtsAppManager::OnOffAbilityFirstFrameState(ani_env *env, ani_string type, ani_object aniObserver)
{
#ifdef SUPPORT_SCREEN
    TAG_LOGD(AAFwkTag::APPMGR, "OnOffAbilityFirstFrameState called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env null ptr");
        return;
    }
    if (!AppExecFwk::CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Non-system app");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return;
    }
    ani_vm *aniVM = nullptr;
    if (env->GetVM(&aniVM) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "get aniVM failed");
        AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env, "Get aniVm failed.");
        return;
    }
    std::string strType;
    if (!OHOS::AppExecFwk::GetStdString(env, type, strType)
        && strType != ON_OFF_TYPE_ABILITY_FIRST_FRAME_STATE) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetStdString failed");
        AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env,
            "Parse param observer failed, must be a AbilityFirstFrameStateObserver.");
        return;
    }
    ani_status status = ANI_OK;
    ani_boolean isUndefined = false;
    if ((status = env->Reference_IsUndefined(aniObserver, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to check undefined status : %{public}d", status);
        return;
    }
    auto abilityManager = GetAbilityManagerInstance();
    if (abilityManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "abilityManager null ptr");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }
    if (isUndefined) {
        AbilityRuntime::ETSAbilityFirstFrameStateObserverManager::GetInstance()->RemoveAllEtsObserverObjects(
            abilityManager);
    } else {
        AbilityRuntime::ETSAbilityFirstFrameStateObserverManager::GetInstance()->RemoveEtsObserverObject(
            abilityManager, aniObserver);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "OnOffAbilityFirstFrameState end");
#endif
}

void EtsAppManager::OnOffAppForegroundState(ani_env *env, ani_string type, ani_object observer)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OnOffAppForegroundState called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env null ptr");
        return;
    }
    std::string strType;
    if (!OHOS::AppExecFwk::GetStdString(env, type, strType)
        && strType != ON_OFF_TYPE_APP_FOREGROUND_STATE) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetStdString failed");
        AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env,
            "Parse param observer failed, must be a AppForegroundStateObserver.");
        return;
    }
    sptr<OHOS::AppExecFwk::IAppMgr> appMgr = GetAppManagerInstance();
    if (appMgr == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appManager null ptr");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }
    if (observerForeground_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null observer or appMgr");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }
    ani_status status = ANI_OK;
    ani_boolean isUndefined = false;
    if ((status = env->Reference_IsUndefined(observer, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to check undefined status : %{public}d", status);
        return;
    }
    if (isUndefined) {
        observerForeground_->RemoveAllEtsObserverObjects();
    } else {
        observerForeground_->RemoveEtsObserverObject(observer);
    }
    if (observerForeground_->IsEmpty()) {
        int32_t ret = appMgr->UnregisterAppForegroundStateObserver(observerForeground_);
        TAG_LOGD(AAFwkTag::APPMGR, "ret: %{public}d.", ret);
        if (ret != NO_ERROR) {
            AbilityRuntime::EtsErrorUtil::ThrowErrorByNativeErr(env, static_cast<int32_t>(ret));
        }
    }
    TAG_LOGD(AAFwkTag::APPMGR, "OnOffAppForegroundState end");
}

void EtsAppManager::ClearUpApplicationDataCheck(ani_env *env, ani_string aniBundleName)
{
    TAG_LOGD(AAFwkTag::APPMGR, "ClearUpApplicationDataCheck called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env null ptr");
        return;
    }
    std::string bundleName;
    if (!AppExecFwk::GetStdString(env, aniBundleName, bundleName) || bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "get bundleName failed");
        AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env, "Parse param bundleName failed, must be a string.");
        return;
    }
}

void EtsAppManager::ClearUpApplicationData(ani_env *env, ani_string aniBundleName, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPMGR, "ClearUpApplicationData called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env null ptr");
        return;
    }
    std::string bundleName;
    if (!AppExecFwk::GetStdString(env, aniBundleName, bundleName) || bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "get bundleName failed");
        return;
    }
    sptr<AppExecFwk::IAppMgr> appMgr = GetAppManagerInstance();
    if (appMgr == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appMgr");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER)), nullptr);
        return;
    }
    int32_t ret = appMgr->ClearUpApplicationData(bundleName, 0);
    TAG_LOGD(AAFwkTag::APPMGR, "ClearUpApplicationData ret %{public}d", ret);
    if (ret == 0) {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, ret), nullptr);
    } else if (ret == AAFwk::CHECK_PERMISSION_FAILED || ret == AAFwk::ERR_NOT_SYSTEM_APP) {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, ret, "clear up application failed."), nullptr);
    } else {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env,
                static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER),
                "clear up application failed."), nullptr);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "ClearUpApplicationData end");
}

void EtsAppManager::GetKeepAliveAppServiceExtensions(ani_env *env, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPMGR, "GetKeepAliveAppServiceExtensions called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env null ptr");
        return;
    }
    ani_object emptyArray = CreateEmptyAniArray(env);
    int32_t innerErrCode = static_cast<int32_t>(ERR_OK);
    auto infoList = std::make_shared<std::vector<AbilityRuntime::KeepAliveInfo>>();
    if (infoList == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "infoList or inner code null");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER)), emptyArray);
        return;
    }
    auto abilityManager = GetAbilityManagerInstance();
    if (abilityManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null abilityManager");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER)), emptyArray);
        return;
    }
    innerErrCode = abilityManager->QueryKeepAliveAppServiceExtensions(*infoList);
    ani_object arrayObj = CreateKeepAliveInfoArray(env, *infoList);
    if (arrayObj == nullptr) {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER)), emptyArray);
        return;
    }
    if (innerErrCode != ERR_OK) {
        TAG_LOGD(AAFwkTag::APPMGR, "QueryKeepAliveAppServiceExtensions failed:%{public}d", innerErrCode);
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, innerErrCode), emptyArray);
    } else {
        TAG_LOGD(AAFwkTag::APPMGR, "QueryKeepAliveAppServiceExtensions succeeded.");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, innerErrCode), arrayObj);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "GetKeepAliveAppServiceExtensions end");
}

void EtsAppManager::SetKeepAliveForAppServiceExtension(ani_env *env, ani_string aniBundleName, ani_boolean enable,
    ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPMGR, "SetKeepAliveForAppServiceExtension called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env null ptr");
        return;
    }
    std::string bundleName;
    if (!AppExecFwk::GetStdString(env, aniBundleName, bundleName) || bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "get bundleName failed");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateInvalidParamError(
                env, "Parse param bundleName failed, must be a string."), nullptr);
        return;
    }
    auto abilityManager = GetAbilityManagerInstance();
    if (abilityManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null abilityManager");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER)), nullptr);
        return;
    }
    int32_t innerErrCode = abilityManager->SetAppServiceExtensionKeepAlive(bundleName, enable);
    TAG_LOGD(AAFwkTag::APPMGR, "SetAppServiceExtensionKeepAlive innerErrCode:%{public}d", innerErrCode);
    AppExecFwk::AsyncCallback(env, callback,
        AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, innerErrCode), nullptr);
    TAG_LOGD(AAFwkTag::APPMGR, "SetKeepAliveForAppServiceExtension end");
}

void EtsAppManager::ClearUpAppDataCheck(ani_env *env, ani_string aniBundleName)
{
    TAG_LOGD(AAFwkTag::APPMGR, "ClearUpAppDataCheck called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env null ptr");
        return;
    }
    std::string bundleName;
    if (!AppExecFwk::GetStdString(env, aniBundleName, bundleName) || bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "get bundleName failed");
        AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env, "Parse param bundleName failed, must be a string");
        return;
    }
}

void EtsAppManager::ClearUpAppData(ani_env *env, ani_object callback, ani_string aniBundleName,
    ani_object aniAppCloneIndex)
{
    TAG_LOGD(AAFwkTag::APPMGR, "ClearUpAppData called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env null ptr");
        return;
    }
    std::string bundleName;
    if (!AppExecFwk::GetStdString(env, aniBundleName, bundleName) || bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "get bundleName failed");
        return;
    }
    int32_t appCloneIndex = 0;
    ani_boolean isUndefined = false;
    ani_status status = ANI_ERROR;
    if ((status = env->Reference_IsUndefined(aniAppCloneIndex, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to check undefined status : %{public}d", status);
        return;
    }
    if (!isUndefined) {
        if ((status = env->Object_CallMethodByName_Int(aniAppCloneIndex,
            "toInt", nullptr, &appCloneIndex)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::APPMGR, "Object_CallMethodByName_Int status : %{public}d", status);
            return;
        }
        TAG_LOGD(AAFwkTag::APPMGR, "aniAppCloneIndex: %{public}d", appCloneIndex);
    }
    sptr<OHOS::AppExecFwk::IAppMgr> appMgr = GetAppManagerInstance();
    if (appMgr == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appManager null ptr");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env,
                static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER)), nullptr);
        return;
    }
    int32_t ret = appMgr->ClearUpApplicationData(bundleName, appCloneIndex);
    TAG_LOGD(AAFwkTag::APPMGR, "ClearUpApplicationData ret:%{public}d", ret);
    if (ret == 0) {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, ret), nullptr);
    } else {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, ret,  "clear up application failed."), nullptr);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "ClearUpAppData end");
}

void EtsAppManager::TerminateMission(ani_env *env, ani_int missionId, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPMGR, "TerminateMission call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env null ptr");
        return;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "missionId:%{public}d", missionId);
    auto amsClient = AAFwk::AbilityManagerClient::GetInstance();
    if (amsClient == nullptr) {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env,
                static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER)), nullptr);
        return;
    }
    int32_t ret = amsClient->TerminateMission(missionId);
    TAG_LOGD(AAFwkTag::APPMGR, "TerminateMission ret:%{public}d", ret);
    if (ret == ERR_OK) {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, ret), nullptr);
    } else {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, ret,  "Terminate mission failed."), nullptr);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "TerminateMission end");
}

void EtsAppManager::IsApplicationRunning(ani_env *env, ani_string aniBundleName, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPMGR, "IsApplicationRunning called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env null ptr");
        return;
    }
    std::string bundleName;
    if (!AppExecFwk::GetStdString(env, aniBundleName, bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "get bundleName failed");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateInvalidParamError(
                env, "Parse param bundleName failed, must be a string."), AppExecFwk::CreateBoolean(env, false));
        return;
    }
    sptr<OHOS::AppExecFwk::IAppMgr> appMgr = GetAppManagerInstance();
    if (appMgr == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appManager null ptr");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env,
                static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER)),
            AppExecFwk::CreateBoolean(env, false));
        return;
    }
    bool isRunning = false;
    int32_t innerErrorCode = appMgr->IsApplicationRunning(bundleName, isRunning);
    TAG_LOGD(AAFwkTag::APPMGR, "IsApplicationRunning isRunning:%{public}d", isRunning);
    if (innerErrorCode == ERR_OK) {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, innerErrorCode),
            AppExecFwk::CreateBoolean(env, isRunning));
    } else {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, innerErrorCode),
            AppExecFwk::CreateBoolean(env, false));
    }
    TAG_LOGD(AAFwkTag::APPMGR, "IsApplicationRunning end");
}

void EtsAppManagerRegistryInit(ani_env *env)
{
    TAG_LOGD(AAFwkTag::APPMGR, "EtsAppManagerRegistryInit call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env null ptr");
        return;
    }
    ani_status status = ANI_ERROR;
    if (env->ResetError() != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "ResetError failed");
    }
    ani_namespace ns;
    status = env->FindNamespace(APP_MANAGER_SPACE_NAME, &ns);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "FindNamespace appManager failed status : %{public}d", status);
        return;
    }
    std::array kitFunctions = {
        ani_native_function{
            "nativePreloadApplication", nullptr, reinterpret_cast<void *>(EtsAppManager::PreloadApplication)},
        ani_native_function{"nativeGetRunningProcessInformation", nullptr,
            reinterpret_cast<void *>(EtsAppManager::GetRunningProcessInformation)},
        ani_native_function{"nativeGetForegroundApplications", nullptr,
            reinterpret_cast<void *>(EtsAppManager::GetForegroundApplications)},
        ani_native_function{
            "nativeGetRunningMultiAppInfo", nullptr, reinterpret_cast<void *>(EtsAppManager::GetRunningMultiAppInfo)},
        ani_native_function{"nativeGetRunningProcessInfoByBundleName", nullptr,
            reinterpret_cast<void *>(EtsAppManager::GetRunningProcessInfoByBundleName)},
        ani_native_function{"nativeGetRunningMultiAppInfoCheck", nullptr,
            reinterpret_cast<void *>(EtsAppManager::GetRunningMultiAppInfoCheck)},
        ani_native_function{"nativeGetRunningProcessInfoByBundleNameAndUserId", nullptr,
            reinterpret_cast<void *>(EtsAppManager::GetRunningProcessInfoByBundleNameAndUserId)},
        ani_native_function {"nativeOn", APPLICATION_STATE_WITH_BUNDLELIST_ON_SIGNATURE,
            reinterpret_cast<void *>(EtsAppManager::OnOnApplicationStateWithBundleList)},
        ani_native_function {"nativeOn", APPLICATION_STATE_ON_SIGNATURE,
            reinterpret_cast<void *>(EtsAppManager::OnOnApplicationState)},
        ani_native_function {"nativeOn", APPLICATION_STATE_WITH_APP_STATE_FILTER_ON_SIGNATURE,
            reinterpret_cast<void *>(EtsAppManager::OnOnApplicationStateWithAppStateFilter)},
        ani_native_function {"nativeOff", APPLICATION_STATE_OFF_SIGNATURE,
            reinterpret_cast<void *>(EtsAppManager::OnOff)},
        ani_native_function {"nativeOffApplicationStateCheck", "i:",
            reinterpret_cast<void *>(EtsAppManager::OffApplicationStateCheck)},
        ani_native_function {"nativeGetAppMemorySize", nullptr,
            reinterpret_cast<void *>(EtsAppManager::GetAppMemorySize)},
        ani_native_function {"nativeIsRamConstrainedDevice", nullptr,
            reinterpret_cast<void *>(EtsAppManager::IsRamConstrainedDevice)},
        ani_native_function {"nativeIsRunningInStabilityTest", nullptr,
            reinterpret_cast<void *>(EtsAppManager::IsRunningInStabilityTest)},
        ani_native_function {"nativeKillProcessesByBundleNameSync", nullptr,
            reinterpret_cast<void *>(EtsAppManager::NativeKillProcessesByBundleNameSync)},
        ani_native_function {"nativeKillProcessesByBundleName", nullptr,
            reinterpret_cast<void *>(EtsAppManager::NativeKillProcessesByBundleName)},
        ani_native_function {"nativeKillProcessWithAccountSync", nullptr,
            reinterpret_cast<void *>(EtsAppManager::NativeKillProcessWithAccountSync)},
        ani_native_function {"nativeKillProcessWithAccount", nullptr,
            reinterpret_cast<void *>(EtsAppManager::NativeKillProcessWithAccount)},
        ani_native_function {"nativeGetProcessMemoryByPid", nullptr,
            reinterpret_cast<void *>(EtsAppManager::NativeGetProcessMemoryByPid)},
        ani_native_function {"nativeGetRunningProcessInformationByBundleType", nullptr,
            reinterpret_cast<void *>(EtsAppManager::GetRunningProcessInformationByBundleType)},
        ani_native_function {"nativeIsSharedBundleRunning", nullptr,
            reinterpret_cast<void *>(EtsAppManager::NativeIsSharedBundleRunning)},
        ani_native_function {"nativeGetSupportedProcessCachePids", nullptr,
            reinterpret_cast<void *>(EtsAppManager::NativeGetSupportedProcessCachePids)},
        ani_native_function {"nativeKillProcessesInBatch", nullptr,
            reinterpret_cast<void *>(EtsAppManager::NativeKillProcessesInBatch)},
        ani_native_function {"nativeIsAppRunning", nullptr,
            reinterpret_cast<void *>(EtsAppManager::NativeIsAppRunning)},
        ani_native_function {"nativeSetKeepAliveForBundle", nullptr,
            reinterpret_cast<void *>(EtsAppManager::NativeSetKeepAliveForBundle)},
        ani_native_function {"nativeGetKeepAliveBundles", nullptr,
            reinterpret_cast<void *>(EtsAppManager::NativeGetKeepAliveBundles)},
        ani_native_function {"nativeOnAppForeGroundState", ON_SIGNATURE_APP_FOREGROUND_STATE,
            reinterpret_cast<void *>(EtsAppManager::OnOnAppForegroundState)},
        ani_native_function {"nativeOffAppForeGroundState", OFF_SIGNATURE_APP_FOREGROUND_STATE,
            reinterpret_cast<void *>(EtsAppManager::OnOffAppForegroundState)},
        ani_native_function {"nativeOnAbilityFirstFrameState", ON_SIGNATURE_ABILITY_FIRST_FRAME_STATE,
            reinterpret_cast<void *>(EtsAppManager::OnOnAbilityFirstFrameState)},
        ani_native_function {"nativeOffAbilityFirstFrameState", OFF_SIGNATURE_ABILITY_FIRST_FRAME_STATE,
            reinterpret_cast<void *>(EtsAppManager::OnOffAbilityFirstFrameState)},
        ani_native_function {"nativeClearUpApplicationDataCheck", BUNDLE_NAME_CHECK_SIG,
            reinterpret_cast<void *>(EtsAppManager::ClearUpApplicationDataCheck)},
        ani_native_function {"nativeClearUpApplicationData", CLEAR_UP_APPLICATION_DATA_SIG,
            reinterpret_cast<void *>(EtsAppManager::ClearUpApplicationData)},
        ani_native_function {"nativeGetKeepAliveAppServiceExtensions", GET_KEEP_ALIVE_APP_SERVICE_EXTENSION_SIG,
            reinterpret_cast<void *>(EtsAppManager::GetKeepAliveAppServiceExtensions)},
        ani_native_function {"nativeSetKeepAliveForAppServiceExtension", SET_KEEP_ALIVE_FOR_APP_SERVICE_EXTENSION_SIG,
            reinterpret_cast<void *>(EtsAppManager::SetKeepAliveForAppServiceExtension)},
        ani_native_function {"nativeClearUpAppDataCheck", BUNDLE_NAME_CHECK_SIG,
            reinterpret_cast<void *>(EtsAppManager::ClearUpAppDataCheck)},
        ani_native_function {"nativeClearUpAppData", CLEAR_UP_APP_DATA_SIG,
            reinterpret_cast<void *>(EtsAppManager::ClearUpAppData)},
        ani_native_function {"nativeTerminateMission", TERMINATION_SIG,
            reinterpret_cast<void *>(EtsAppManager::TerminateMission)},
        ani_native_function {"nativeIsApplicationRunning", IS_APP_RUNNING_SIG,
            reinterpret_cast<void *>(EtsAppManager::IsApplicationRunning)},
	};
    status = env->Namespace_BindNativeFunctions(ns, kitFunctions.data(), kitFunctions.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Namespace_BindNativeFunctions failed status : %{public}d", status);
    }
    if (env->ResetError() != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "ResetError failed");
    }
    TAG_LOGD(AAFwkTag::APPMGR, "EtsAppManagerRegistryInit end");
}

extern "C" {
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "in AppManagerEts.ANI_Constructor");
    if (vm == nullptr || result == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null vm or result");
        return ANI_INVALID_ARGS;
    }

    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    status = vm->GetEnv(ANI_VERSION_1, &env);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetEnv failed, status=%{public}d", status);
        return ANI_NOT_FOUND;
    }
    EtsAppManagerRegistryInit(env);
    *result = ANI_VERSION_1;
    TAG_LOGD(AAFwkTag::APPMGR, "AppManagerEts.ANI_Constructor finished");
    return ANI_OK;
}
}  // extern "C"
}  // namespace AppManagerEts
}  // namespace OHOS
