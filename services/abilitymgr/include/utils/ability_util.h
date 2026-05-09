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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_UTIL_H
#define OHOS_ABILITY_RUNTIME_ABILITY_UTIL_H

#include <string>
#include <unordered_set>

#include "ability_config.h"
#include "ability_manager_client.h"
#include "ability_manager_errors.h"
#include "app_jump_control_rule.h"
#include "bundle_mgr_helper.h"
#include "hilog_tag_wrapper.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "permission_verification.h"
#include "sa_mgr_client.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AAFwk {
namespace AbilityUtil {
constexpr const char* SYSTEM_BASIC = "system_basic";
constexpr const char* SYSTEM_CORE = "system_core";
constexpr const char* DEFAULT_DEVICE_ID = "";

#ifdef WITH_DLP
constexpr const char* DLP_BUNDLE_NAME = "com.ohos.dlpmanager";
constexpr const char* DLP_MODULE_NAME = "entry";
constexpr const char* DLP_ABILITY_NAME = "ViewAbility";
constexpr const char* DLP_PARAMS_SANDBOX = "ohos.dlp.params.sandbox";
constexpr const char* DLP_PARAMS_BUNDLE_NAME = "ohos.dlp.params.bundleName";
constexpr const char* DLP_PARAMS_MODULE_NAME = "ohos.dlp.params.moduleName";
constexpr const char* DLP_PARAMS_ABILITY_NAME = "ohos.dlp.params.abilityName";
#endif // WITH_DLP
constexpr const char* MARKET_BUNDLE_NAME = "com.hmsapp.appgallery";
constexpr const char* MARKET_CROWD_TEST_BUNDLE_PARAM = "crowd_test_bundle_name";
constexpr const char* BUNDLE_NAME_SELECTOR_DIALOG = "com.ohos.amsdialog";
constexpr const char* JUMP_INTERCEPTOR_DIALOG_CALLER_PKG = "interceptor_callerPkg";

#define CHECK_POINTER_CONTINUE(object)                         \
    if (!object) {                                             \
        TAG_LOGE(AAFwkTag::ABILITYMGR, "nullptr");        \
        continue;                                              \
    }

#define CHECK_POINTER_IS_NULLPTR(object)                       \
    if (object == nullptr) {                                   \
        TAG_LOGE(AAFwkTag::ABILITYMGR, "nullptr");        \
        return;                                                \
    }

#define CHECK_POINTER(object)                                  \
    if (!object) {                                             \
        TAG_LOGE(AAFwkTag::ABILITYMGR, "nullptr");        \
        return;                                                \
    }

#define CHECK_POINTER_LOG(object, log)                      \
    if (!object) {                                          \
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s:", log); \
        return;                                             \
    }

#define CHECK_POINTER_TAG_LOG(object, tag, log)             \
    if (!object) {                                          \
        TAG_LOGE(tag, "%{public}s:", log);                  \
        return;                                             \
    }

#define CHECK_POINTER_AND_RETURN(object, value)                \
    if (!object) {                                             \
        TAG_LOGE(AAFwkTag::ABILITYMGR, "nullptr"); \
        return value;                                          \
    }

#define CHECK_POINTER_AND_RETURN_LOG(object, value, log)    \
    if (!object) {                                          \
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s:", log); \
        return value;                                       \
    }

#define CHECK_POINTER_RETURN_BOOL(object)                      \
    if (!object) {                                             \
        TAG_LOGE(AAFwkTag::ABILITYMGR, "nullptr"); \
        return false;                                          \
    }

#define CHECK_RET_RETURN_RET(object, log)                                            \
    if (object != ERR_OK) {                                                          \
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s, ret : %{public}d", log, object); \
        return object;                                                               \
    }

#define CHECK_TRUE_RETURN_RET(object, value, log)          \
    if (object) {                                          \
        TAG_LOGW(AAFwkTag::ABILITYMGR, "%{public}s", log); \
        return value;                                      \
    }

[[maybe_unused]] static int64_t GetSysTimeNs()
{
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
}

[[maybe_unused]] static bool IsSystemDialogAbility(const std::string &bundleName, const std::string &abilityName)
{
    if (abilityName == AbilityConfig::SYSTEM_DIALOG_NAME && bundleName == AbilityConfig::SYSTEM_UI_BUNDLE_NAME) {
        return true;
    }

    if (abilityName == AbilityConfig::DEVICE_MANAGER_NAME && bundleName == AbilityConfig::DEVICE_MANAGER_BUNDLE_NAME) {
        return true;
    }

    return false;
}

[[maybe_unused]] static std::string ConvertBundleNameSingleton(const std::string &bundleName, const std::string &name,
    const std::string &moduleName, const int32_t appIndex = 0)
{
    std::string strName;
    if (appIndex == 0) {
        strName = AbilityConfig::MISSION_NAME_MARK_HEAD + bundleName +
            AbilityConfig::MISSION_NAME_SEPARATOR + moduleName +
            AbilityConfig::MISSION_NAME_SEPARATOR + name;
    } else {
        strName = AbilityConfig::MISSION_NAME_MARK_HEAD + bundleName +
            AbilityConfig::MISSION_NAME_SEPARATOR + std::to_string(appIndex) +
            AbilityConfig::MISSION_NAME_SEPARATOR + moduleName +
            AbilityConfig::MISSION_NAME_SEPARATOR + name;
    }

    return strName;
}

static constexpr int64_t NANOSECONDS = 1000000000;  // NANOSECONDS mean 10^9 nano second
static constexpr int64_t MICROSECONDS = 1000000;    // MICROSECONDS mean 10^6 millias second
[[maybe_unused]] static int64_t SystemTimeMillis()
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (int64_t)((t.tv_sec) * NANOSECONDS + t.tv_nsec) / MICROSECONDS;
}

[[maybe_unused]] static int64_t UTCTimeSeconds()
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 0;
    clock_gettime(CLOCK_REALTIME, &t);
    return (int64_t)(t.tv_sec);
}

[[maybe_unused]] static int64_t UTCTimeMillis()
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 0;
    clock_gettime(CLOCK_REALTIME, &t);
    return static_cast<int64_t>(t.tv_sec * 1000LL + t.tv_nsec / 1000000LL);
}

[[maybe_unused]] static bool IsStartFreeInstall(const Want &want)
{
    auto flags = want.GetFlags();
    if ((flags & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND) {
        return true;
    }
    return false;
}

[[maybe_unused]] static std::shared_ptr<AppExecFwk::BundleMgrHelper> GetBundleManagerHelper()
{
    return DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
}

[[maybe_unused]] static bool ParseJumpInterceptorWant(Want &targetWant, const std::string callerPkg)
{
    if (callerPkg.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "empty callerPkg");
        return false;
    }
    targetWant.SetParam(JUMP_INTERCEPTOR_DIALOG_CALLER_PKG, callerPkg);
    return true;
}

[[maybe_unused]] static bool CheckJumpInterceptorWant(const Want &targetWant, std::string &callerPkg,
    std::string &targetPkg)
{
    if (!targetWant.HasParameter(JUMP_INTERCEPTOR_DIALOG_CALLER_PKG)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid interceptor param");
        return false;
    }
    callerPkg = targetWant.GetStringParam(JUMP_INTERCEPTOR_DIALOG_CALLER_PKG);
    targetPkg = targetWant.GetElement().GetBundleName();
    return !callerPkg.empty() && !targetPkg.empty();
}

[[maybe_unused]] static bool AddAbilityJumpRuleToBms(const std::string &callerPkg, const std::string &targetPkg,
    int32_t userId)
{
    if (callerPkg.empty() || targetPkg.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid inputs");
        return false;
    }
    auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetBundleManagerHelper failed");
        return false;
    }
    auto appControlMgr = bundleMgrHelper->GetAppControlProxy();
    if (appControlMgr == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get appControlMgr failed");
        return false;
    }
    int ret = IN_PROCESS_CALL(appControlMgr->ConfirmAppJumpControlRule(callerPkg, targetPkg, userId));
    return ret == ERR_OK;
}

[[maybe_unused]] static bool HandleDlpApp(Want &want)
{
#ifdef WITH_DLP
    const std::unordered_set<std::string> whiteListDlpSet = { BUNDLE_NAME_SELECTOR_DIALOG };
    if (whiteListDlpSet.find(want.GetBundle()) != whiteListDlpSet.end()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "enter special app");
        return false;
    }

    AppExecFwk::ElementName element = want.GetElement();
    if (want.GetBoolParam(DLP_PARAMS_SANDBOX, false) && !element.GetBundleName().empty() &&
        !element.GetAbilityName().empty()) {
        want.SetElementName(DEFAULT_DEVICE_ID, DLP_BUNDLE_NAME, DLP_ABILITY_NAME, DLP_MODULE_NAME);
        want.SetParam(DLP_PARAMS_BUNDLE_NAME, element.GetBundleName());
        want.SetParam(DLP_PARAMS_MODULE_NAME, element.GetModuleName());
        want.SetParam(DLP_PARAMS_ABILITY_NAME, element.GetAbilityName());
        want.RemoveParam(DLP_PARAMS_SANDBOX);
        return true;
    }
#endif // WITH_DLP
    return false;
}

[[maybe_unused]] static bool IsStartIncludeAtomicService(const Want &want, const int32_t userId)
{
    auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetBundleManagerHelper failed");
        return false;
    }

    std::string targetBundleName = want.GetBundle();
    AppExecFwk::ApplicationInfo targetAppInfo;
    bool getTargetResult = IN_PROCESS_CALL(bundleMgrHelper->GetApplicationInfo(targetBundleName,
        AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, targetAppInfo));
    if (!getTargetResult) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get targetAppInfo failed");
        return false;
    }
    if (targetAppInfo.bundleType == AppExecFwk::BundleType::ATOMIC_SERVICE) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "target is atomic service");
        return true;
    }

    int callerUid = IPCSkeleton::GetCallingUid();
    std::string callerBundleName;
    ErrCode err = IN_PROCESS_CALL(bundleMgrHelper->GetNameForUid(callerUid, callerBundleName));
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get bms failed");
        return false;
    }
    AppExecFwk::ApplicationInfo callerAppInfo;
    bool getCallerResult = IN_PROCESS_CALL(bundleMgrHelper->GetApplicationInfo(callerBundleName,
        AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, callerAppInfo));
    if (!getCallerResult) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get callerAppInfo failed");
        return false;
    }
    if (callerAppInfo.bundleType == AppExecFwk::BundleType::ATOMIC_SERVICE) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "caller is atomic service");
        return true;
    }
    return false;
}

[[maybe_unused]] static void RemoveShowModeKey(Want &want)
{
    if (want.HasParameter(AAFwk::SCREEN_MODE_KEY)) {
        want.RemoveParam(AAFwk::SCREEN_MODE_KEY);
    }
}

[[maybe_unused]] static bool IsSceneBoard(const std::string &bundleName, const std::string &AbilityName)
{
    return AbilityName == AbilityConfig::SCENEBOARD_ABILITY_NAME &&
        bundleName == AbilityConfig::SCENEBOARD_BUNDLE_NAME;
}

[[maybe_unused]] static void RemoveWindowModeKey(Want &want)
{
    if (want.HasParameter(Want::PARAM_RESV_WINDOW_MODE)) {
        want.RemoveParam(Want::PARAM_RESV_WINDOW_MODE);
    }
}

[[maybe_unused]] static void RemoveInstanceKey(Want &want)
{
    want.RemoveParam(Want::APP_INSTANCE_KEY);
    want.RemoveParam(Want::CREATE_APP_INSTANCE_KEY);
}

[[maybe_unused]] static void RemoveWantKey(Want &want)
{
    RemoveShowModeKey(want);
    RemoveWindowModeKey(want);
}

[[maybe_unused]] static int32_t CheckInstanceKey(const Want &want)
{
    auto instanceKey = want.GetStringParam(Want::APP_INSTANCE_KEY);
    auto isCreating = want.GetBoolParam(Want::CREATE_APP_INSTANCE_KEY, false);
    if (!instanceKey.empty() || isCreating) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Not support multi-instance");
        return ERR_MULTI_INSTANCE_NOT_SUPPORTED;
    }
    return ERR_OK;
}

[[maybe_unused]] static void WantSetParameterWindowMode(Want &want, int32_t windowMode)
{
    want.SetParam(Want::PARAM_RESV_WINDOW_MODE, windowMode);
}

[[maybe_unused]] static void ProcessWindowMode(Want &want, uint32_t accessTokenId, int32_t windowMode)
{
    if (PermissionVerification::GetInstance()->IsSystemAppCall()) {
        want.SetParam(Want::PARAM_RESV_WINDOW_MODE, windowMode);
        return;
    }
    if (IPCSkeleton::GetCallingTokenID() == accessTokenId && (
        windowMode == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY ||
        windowMode == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY ||
        windowMode == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SPLIT)) {
        want.SetParam(Want::PARAM_RESV_WINDOW_MODE, windowMode);
        TAG_LOGI(AAFwkTag::ABILITYMGR, "set windowMode for inner application split-screen mode");
    } else if (windowMode == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FULLSCREEN) {
        want.SetParam(Want::PARAM_RESV_WINDOW_MODE, windowMode);
        TAG_LOGI(AAFwkTag::ABILITYMGR, "set windowMode for full screen mode");
    } else {
        RemoveWindowModeKey(want);
    }
}

[[maybe_unused]] static int StartAppgallery(const std::string &bundleName, const int requestCode, const int32_t userId,
    const std::string &action)
{
    std::string appGalleryBundleName;
    auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr || !bundleMgrHelper->QueryAppGalleryBundleName(appGalleryBundleName)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetBundleManagerHelper or QueryAppGalleryBundleName failed");
        appGalleryBundleName = MARKET_BUNDLE_NAME;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "appGalleryBundleName:%{public}s", appGalleryBundleName.c_str());

    Want want;
    want.SetElementName(appGalleryBundleName, "");
    want.SetAction(action);
    want.SetParam(MARKET_CROWD_TEST_BUNDLE_PARAM, bundleName);
    return AbilityManagerClient::GetInstance()->StartAbility(want, requestCode, userId);
}

[[maybe_unused]] static std::time_t GetTimeStamp()
{
    std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp =
        std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    std::time_t timestamp = tp.time_since_epoch().count();
    return timestamp;
}

inline ErrCode EdmErrorType(bool isEdm)
{
    if (isEdm) {
        return ERR_EDM_APP_CONTROLLED;
    }
    return ERR_APP_CONTROLLED;
}
}  // namespace AbilityUtil
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_ABILITY_UTIL_H
