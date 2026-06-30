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

#include "ets_ability_manager.h"

#include <regex>

#include "ability_business_error.h"
#include "ability_context.h"
#include "ability_manager_client.h"
#include "ability_manager_errors.h"
#include "ability_manager_interface.h"
#include "accesstoken_kit.h"
#include "acquire_share_data_callback_stub.h"
#include "ani_base_context.h"
#include "ani_common_ability_state_data.h"
#include "ani_common_configuration.h"
#include "ani_common_util.h"
#include "ani_common_want.h"
#include "ani_enum_convert.h"
#include "app_mgr_interface.h"
#include "app_utils.h"
#include "application_context.h"
#include "ets_ability_foreground_state_observer.h"
#include "ets_ability_manager_utils.h"
#include "ets_error_utils.h"
#include "ets_preload_ui_extension_callback_client.h"
#include "ets_query_erms_observer.h"
#include "hilog_tag_wrapper.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "permission_constants.h"
#include "preload_ui_extension_host_client.h"
#include "system_ability_definition.h"
#include "tokenid_kit.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char *ETS_ABILITY_MANAGER_NAMESPACE = "@ohos.app.ability.abilityManager.abilityManager";
constexpr const char *ETS_ABILITY_MANAGER_SIGNATURE_ARRAY = ":C{std.core.Array}";
constexpr const char *ETS_ABILITY_MANAGER_SIGNATURE_CALLBACK = "C{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char *ETS_ABILITY_MANAGER_SIGNATURE_VOID = ":";
constexpr const char *ON_OFF_TYPE_ABILITY_FOREGROUND_STATE = "abilityForegroundState";
constexpr const char *ON_OFF_PRELOAD_UI_EXTENSION_ABILITY_SIGNATURE = "C{std.core.Function1}:";
constexpr const char *PRELOAD_UI_EXTENSION_ABILITY_SIGNATURE =
    "C{@ohos.app.ability.Want.Want}C{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char *CLEAR_PRELOAD_UI_EXTENSION_ABILITY_SIGNATURE = "iC{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char *CLEAR_PRELOAD_UI_EXTENSION_ABILITIES_SIGNATURE = "C{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char *NOTIFY_SAVE_AS_RESULT_SIGNATURE =
    "C{ability.abilityResult.AbilityResult}iC{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char *NOTIFY_SAVE_AS_RESULT_CHECK_SIGNATURE = "C{ability.abilityResult.AbilityResult}:";
constexpr int32_t ERR_FAILURE = -1;
const std::string MAX_UINT64_VALUE = "18446744073709551615";
std::shared_ptr<AppExecFwk::EventHandler> mainHandler_ = nullptr;

sptr<AAFwk::AcquireShareDataCallbackStub> CreateShareDataCallbackStub(ani_env *env, ani_ref callbackRef)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return nullptr;
    }
    ani_vm *aniVM = nullptr;
    if (env->GetVM(&aniVM) != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "GetVM failed");
        return nullptr;
    }
    auto shareDataCallbackStub = new (std::nothrow) AAFwk::AcquireShareDataCallbackStub();
    if (shareDataCallbackStub == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null shareDataCallbackStub");
        return nullptr;
    }
    if (mainHandler_ == nullptr) {
        mainHandler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());
    }
    shareDataCallbackStub->SetHandler(mainHandler_);
    AAFwk::ShareRuntimeTask task =
        [aniVM, callbackRef](int32_t resultCode, const AAFwk::WantParams &wantParam) {
            ani_env *env = nullptr;
            ani_status status = aniVM->GetEnv(ANI_VERSION_1, &env);
            if (status != ANI_OK || env == nullptr) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
                return;
            }
            if (resultCode != 0) {
                AppExecFwk::AsyncCallback(env, static_cast<ani_object>(callbackRef),
                    EtsErrorUtil::CreateErrorByNativeErr(env, resultCode, "",
                        GetInnerErrorMsg(AbilityInnerErrorMsg::ACQUIRE_SHARE_DATA_FAILED)), nullptr);
                env->GlobalReference_Delete(callbackRef);
                return;
            }
            ani_ref wantParamRef = AppExecFwk::WrapWantParams(env, wantParam);
            if (wantParamRef == nullptr) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "null wantParamRef");
                AppExecFwk::AsyncCallback(env, static_cast<ani_object>(callbackRef),
                    EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
                        "", GetInnerErrorMsg(AbilityInnerErrorMsg::ACQUIRE_SHARE_DATA_FAILED)), nullptr);
                env->GlobalReference_Delete(callbackRef);
                return;
            }
            AppExecFwk::AsyncCallback(env, static_cast<ani_object>(callbackRef),
                EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), static_cast<ani_object>(wantParamRef));
            env->GlobalReference_Delete(callbackRef);
        };
    shareDataCallbackStub->SetShareRuntimeTask(task);
    return shareDataCallbackStub;
}
}

class EtsAbilityManager final {
public:
    static ani_object GetForegroundUIAbilities(ani_env *env);
    static void GetForegroundUIAbilitiesCallBack(ani_env *env, ani_object callbackObj);
    static void GetTopAbility(ani_env *env, ani_object callback);
    static void CheckSystemApp(ani_env *env);
    static void GetTopAbilityCheck(ani_env *env);
    static void GetAbilityRunningInfos(ani_env *env, ani_object callback);
    static ani_boolean IsEmbeddedUIExtensionSupported(ani_env *env);
    static void IsEmbeddedOpenAllowed(ani_env *env, ani_object contextObj, ani_string aniAppId, ani_object callbackObj);
    static void IsEmbeddedOpenAllowedCheck(ani_env *env, ani_object contextObj);
    static void NativeOn(ani_env *env, ani_string aniType, ani_object aniObserver);
    static void NativeOff(ani_env *env, ani_string aniType, ani_object aniObserver);
    static void NativeNotifyDebugAssertResult(ani_env *env, ani_string aniSessionId, ani_object userStatusObj,
        ani_object callbackObj);
    static void NativeNotifyDebugAssertResultCheck(ani_env *env, ani_string aniSessionId, ani_object userStatusObj);
    static void NativeSetResidentProcessEnabled(ani_env *env, ani_string aniBundleName, ani_boolean enabled,
        ani_object callbackObj);
    static void NativeSetResidentProcessEnabledCheck(ani_env *env, ani_string aniBundleName);
    static void NativeAcquireShareData(ani_env *env, ani_int aniMissionId, ani_object callbackObj);
    static void NativeUpdateConfiguration(ani_env *env, ani_object configObj, ani_object callbackObj);
    static void QueryAtomicServiceStartupRule(ani_env *env, ani_object contextObj,
        ani_string aniAppId, ani_object callbackObj);
    static void QueryAtomicServiceStartupRuleCheck(ani_env *env, ani_object contextObj);
    static void GetExtensionRunningInfos(ani_env *env, ani_int upperLimit, ani_object callback);
    static void NativeOnPreloadedUIExtensionAbilityLoaded(ani_env *env, ani_fn_object callback);
    static void NativeOffPreloadedUIExtensionAbilityLoaded(ani_env *env, ani_fn_object callback);
    static void NativeOnPreloadedUIExtensionAbilityDestroyed(ani_env *env, ani_fn_object callback);
    static void NativeOffPreloadedUIExtensionAbilityDestroyed(ani_env *env, ani_fn_object callback);
    static void NativePreloadUIExtensionAbility(ani_env *env, ani_object aniWant, ani_object callback);
    static void NativeClearPreloadedUIExtensionAbility(ani_env *env, ani_int preloadId, ani_object callback);
    static void NativeClearPreloadedUIExtensionAbilities(ani_env *env, ani_object callback);
    static void CheckPreloadUIExtensionAbilityPermission(ani_env *env);
    static void nativeNotifySaveAsResult(ani_env *env, ani_object aniParameter,
        ani_int requestCode, ani_object callback);
    static void nativeNotifySaveAsResultCheck(ani_env *env, ani_object aniParameter);

private:
    static sptr<AppExecFwk::IAbilityManager> GetAbilityManagerInstance();
    static sptr<AppExecFwk::IAppMgr> GetAppManagerInstance();
    static int AddQueryERMSObserver(ani_vm *vm, sptr<IRemoteObject> token, const std::string &appId,
        const std::string &startTime, ani_object callbackObj);
    static void QueryAtomicServiceStartupRuleInner(std::string appId, std::string startTime,
        sptr<IRemoteObject> token);
    static sptr<AbilityRuntime::ETSAbilityForegroundStateObserver> observerForeground_;
    static sptr<AbilityRuntime::EtsQueryERMSObserver> queryERMSObserver_;
    static std::vector<std::pair<ani_ref, int32_t>> loadedCallback_;
    static std::vector<std::pair<ani_ref, int32_t>> destroyCallback_;
    static std::mutex loadedCallbackMutex_;
    static std::mutex destroyedCallbackMutex_;
};

sptr<AbilityRuntime::ETSAbilityForegroundStateObserver> EtsAbilityManager::observerForeground_ = nullptr;
sptr<AbilityRuntime::EtsQueryERMSObserver> EtsAbilityManager::queryERMSObserver_ = nullptr;
std::vector<std::pair<ani_ref, int32_t>> EtsAbilityManager::loadedCallback_ = {};
std::vector<std::pair<ani_ref, int32_t>> EtsAbilityManager::destroyCallback_ = {};
std::mutex EtsAbilityManager::loadedCallbackMutex_;
std::mutex EtsAbilityManager::destroyedCallbackMutex_;

sptr<AppExecFwk::IAbilityManager> EtsAbilityManager::GetAbilityManagerInstance()
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> abilityManagerObj = systemAbilityManager->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    return iface_cast<AppExecFwk::IAbilityManager>(abilityManagerObj);
}

sptr<AppExecFwk::IAppMgr> EtsAbilityManager::GetAppManagerInstance()
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> appObject = systemAbilityManager->GetSystemAbility(APP_MGR_SERVICE_ID);
    return iface_cast<AppExecFwk::IAppMgr>(appObject);
}

ani_object EtsAbilityManager::GetForegroundUIAbilities(ani_env *env)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call GetForegroundUIAbilities");

    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return nullptr;
    }

    sptr<AppExecFwk::IAbilityManager> abilityManager = GetAbilityManagerInstance();
    if (abilityManager == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityManager is null");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        return nullptr;
    }
    std::vector<AppExecFwk::AbilityStateData> list;
    int32_t ret = abilityManager->GetForegroundUIAbilities(list);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed: ret=%{public}d", ret);
        AbilityRuntime::AbilityErrorCode code = AbilityRuntime::GetJsErrorCodeByNativeError(ret);
        AbilityRuntime::EtsErrorUtil::ThrowError(env, code);
        return nullptr;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "GetForegroundUIAbilities succeeds, list.size=%{public}zu", list.size());
    ani_object aniArray = AppExecFwk::CreateAniAbilityStateDataArray(env, list);
    if (aniArray == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null aniArray");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        return nullptr;
    }
    return aniArray;
}

void EtsAbilityManager::GetForegroundUIAbilitiesCallBack(ani_env *env, ani_object callbackObj)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call GetForegroundUIAbilitiesCallBack");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }

    sptr<AppExecFwk::IAbilityManager> abilityManager = GetAbilityManagerInstance();
    if (abilityManager == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityManager is null");
        AppExecFwk::AsyncCallback(env, callbackObj,
            EtsErrorUtil::CreateError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER), nullptr);
        return;
    }
    std::vector<AppExecFwk::AbilityStateData> list;
    int32_t ret = abilityManager->GetForegroundUIAbilities(list);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed: ret=%{public}d", ret);
        AbilityRuntime::AbilityErrorCode code = AbilityRuntime::GetJsErrorCodeByNativeError(ret);
        AppExecFwk::AsyncCallback(env, callbackObj, EtsErrorUtil::CreateError(env, code), nullptr);
        return;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "GetForegroundUIAbilities succeeds, list.size=%{public}zu", list.size());
    ani_object aniArray = AppExecFwk::CreateAniAbilityStateDataArray(env, list);
    if (aniArray == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null aniArray");
        AppExecFwk::AsyncCallback(env, callbackObj,
            EtsErrorUtil::CreateError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER), nullptr);
        return;
    }
    AppExecFwk::AsyncCallback(env, callbackObj, EtsErrorUtil::CreateErrorByNativeErr(env, ERR_OK), aniArray);
}

void EtsAbilityManager::CheckSystemApp(ani_env *env)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "not system app");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return;
    }
}

void EtsAbilityManager::CheckPreloadUIExtensionAbilityPermission(ani_env *env)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    int result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(
        selfToken, AAFwk::PermissionConstants::PERMISSION_PRELOAD_UI_EXTENSION_ABILITY);
    if (result != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "VerifyAccessToken fail");
        EtsErrorUtil::ThrowNoPermissionError(env, AAFwk::PermissionConstants::PERMISSION_PRELOAD_UI_EXTENSION_ABILITY);
    }
}

void EtsAbilityManager::GetTopAbility(ani_env *env, ani_object callback)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call GetTopAbility");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "not system app");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return;
    }
    AppExecFwk::ElementName elementName = AAFwk::AbilityManagerClient::GetInstance()->GetTopAbility();
    int resultCode = 0;
    ani_object elementNameobj = AppExecFwk::WrapElementName(env, elementName);
    if (elementNameobj == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null elementNameobj");
        resultCode = ERR_FAILURE;
    }
    AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateErrorByNativeErr(env, resultCode, "",
        GetInnerErrorMsg(AbilityInnerErrorMsg::GET_TOP_ABILITY_FAILED)), elementNameobj);
    return;
}

void EtsAbilityManager::GetTopAbilityCheck(ani_env *env)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "GetTopAbilityCheck");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "not system app");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return;
    }
}

void EtsAbilityManager::GetAbilityRunningInfos(ani_env *env, ani_object callback)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "GetAbilityRunningInfos");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    std::vector<AAFwk::AbilityRunningInfo> infos;
    auto errcode = AAFwk::AbilityManagerClient::GetInstance()->GetAbilityRunningInfos(infos);
    ani_object retObject = nullptr;
    AbilityManagerEts::WrapAbilityRunningInfoArray(env, retObject, infos);
    AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateErrorByNativeErr(env, errcode, "",
        GetInnerErrorMsg(AbilityInnerErrorMsg::GET_ABILITY_RUNNING_INFOS_FAILED)), retObject);
}

void EtsAbilityManager::IsEmbeddedOpenAllowedCheck(ani_env *env, ani_object contextObj)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "IsEmbeddedOpenAllowedCheck");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    auto context = OHOS::AbilityRuntime::GetStageModeContext(env, contextObj);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null context");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param context failed, must not be nullptr.");
        return;
    }
    auto uiAbilityContext = OHOS::AbilityRuntime::Context::ConvertTo<OHOS::AbilityRuntime::AbilityContext>(context);
    if (uiAbilityContext == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null UIAbilityContext");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param context failed, must be UIAbilityContext.");
    }
}

ani_boolean EtsAbilityManager::IsEmbeddedUIExtensionSupported(ani_env *env)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "IsEmbeddedUIExtensionSupported");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return ANI_FALSE;
    }
    bool result = AAFwk::AppUtils::GetInstance().IsMultiProcessModel();
    return result ? ANI_TRUE : ANI_FALSE;
}

void EtsAbilityManager::IsEmbeddedOpenAllowed(ani_env *env, ani_object contextObj,
    ani_string aniAppId, ani_object callbackObj)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "IsEmbeddedOpenAllowed");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    auto context = OHOS::AbilityRuntime::GetStageModeContext(env, contextObj);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null context");
        return;
    }
    auto uiAbilityContext = OHOS::AbilityRuntime::Context::ConvertTo<OHOS::AbilityRuntime::AbilityContext>(context);
    if (uiAbilityContext == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null UIAbilityContext");
        return;
    }
    std::string appId;
    if (!AppExecFwk::GetStdString(env, aniAppId, appId)) {
        return;
    }
    auto token = uiAbilityContext->GetToken();
    ani_boolean ret = AAFwk::AbilityManagerClient::GetInstance()->IsEmbeddedOpenAllowed(token, appId);
    AppExecFwk::AsyncCallback(env, callbackObj,
        EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK),  AppExecFwk::CreateBoolean(env, ret));
}

int EtsAbilityManager::AddQueryERMSObserver(ani_vm *vm, sptr<IRemoteObject> token, const std::string &appId,
    const std::string &startTime, ani_object callbackObj)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "AddQueryERMSObserver");
    if (queryERMSObserver_ == nullptr) {
        queryERMSObserver_ = sptr<EtsQueryERMSObserver>::MakeSptr(vm);
    }
    queryERMSObserver_->AddEtsObserverObject(appId, startTime, callbackObj);
    int32_t ret = AAFwk::AbilityManagerClient::GetInstance()->AddQueryERMSObserver(token, queryERMSObserver_);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "addQueryERMSObserver error");
        AtomicServiceStartupRule rule;
        queryERMSObserver_->OnQueryFinished(appId, startTime, rule, AAFwk::INNER_ERR);
        return ret;
    }
    return ERR_OK;
}

void EtsAbilityManager::QueryAtomicServiceStartupRuleCheck(ani_env *env, ani_object contextObj)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "QueryAtomicServiceStartupRule");
    auto context = OHOS::AbilityRuntime::GetStageModeContext(env, contextObj);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null context");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param context failed, must not be nullptr.");
        return;
    }
    auto uiAbilityContext = OHOS::AbilityRuntime::Context::ConvertTo<OHOS::AbilityRuntime::AbilityContext>(context);
    if (uiAbilityContext == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null UIAbilityContext");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param context failed, must be UIAbilityContext.");
        return;
    }
}

void EtsAbilityManager::QueryAtomicServiceStartupRule(ani_env *env, ani_object contextObj,
    ani_string aniAppId, ani_object callbackObj)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "QueryAtomicServiceStartupRule");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    auto context = OHOS::AbilityRuntime::GetStageModeContext(env, contextObj);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null context");
        return;
    }
    auto uiAbilityContext = OHOS::AbilityRuntime::Context::ConvertTo<OHOS::AbilityRuntime::AbilityContext>(context);
    if (uiAbilityContext == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null UIAbilityContext");
        return;
    }
    std::string appId;
    if (!AppExecFwk::GetStdString(env, aniAppId, appId)) {
        return;
    }
    ani_vm *aniVM = nullptr;
    if (env->GetVM(&aniVM) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "get aniVM failed");
        return;
    }
    auto token = uiAbilityContext->GetToken();
    std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
        system_clock::now().time_since_epoch()).count());
    auto ret = AddQueryERMSObserver(aniVM, token, appId, startTime, callbackObj);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "AddQueryERMSObserver failed, ret=%{public}d", ret);
        return;
    }
    QueryAtomicServiceStartupRuleInner(appId, startTime, token);
}

void EtsAbilityManager::QueryAtomicServiceStartupRuleInner(std::string appId, std::string startTime,
    sptr<IRemoteObject> token)
{
    auto rule = std::make_shared<AtomicServiceStartupRule>();
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->QueryAtomicServiceStartupRule(token,
        appId, startTime, *rule);
    if (ret== AAFwk::ERR_ECOLOGICAL_CONTROL_STATUS) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "openning dialog to confirm");
        return;
    }
    if (queryERMSObserver_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "queryERMSObserver_ null");
        return;
    }
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "query failed: %{public}d", ret);
        queryERMSObserver_->OnQueryFinished(appId, startTime, *rule, ret);
        return;
    }
    queryERMSObserver_->OnQueryFinished(appId, startTime, *rule, ERR_OK);
}

void EtsAbilityManager::NativeOn(ani_env *env, ani_string aniType, ani_object aniObserver)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "nativeOn called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    std::string strType;
    if (!AppExecFwk::GetStdString(env, aniType, strType) || strType != ON_OFF_TYPE_ABILITY_FOREGROUND_STATE) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetStdString failed");
        EtsErrorUtil::ThrowInvalidParamError(env,
            "Parse param observer failed, must be a AbilityForegroundStateObserver.");
        return;
    }
    ani_vm *aniVM = nullptr;
    if (env->GetVM(&aniVM) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "get aniVM failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Get aniVm failed.");
        return;
    }
    if (observerForeground_ == nullptr) {
        observerForeground_ = sptr<ETSAbilityForegroundStateObserver>::MakeSptr(aniVM);
        if (observerForeground_ == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null observerForeground_");
            EtsErrorUtil::ThrowError(env, static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER),
                GetInnerErrorMsg(AbilityInnerErrorMsg::OBSERVER_CREATE_FAILED));
            return;
        }
    }
    if (observerForeground_->IsEmpty()) {
        auto appManager = GetAppManagerInstance();
        if (appManager == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "appManager null ptr");
            EtsErrorUtil::ThrowError(env, static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER),
                GetInnerErrorMsg(AbilityInnerErrorMsg::SERVICE_UNAVAILABLE));
            return;
        }
        int32_t ret = appManager->RegisterAbilityForegroundStateObserver(observerForeground_);
        TAG_LOGD(AAFwkTag::ABILITYMGR, "ret: %{public}d", ret);
        if (ret != NO_ERROR) {
            EtsErrorUtil::ThrowErrorByNativeErr(env, static_cast<int32_t>(ret));
            return;
        }
    }
    observerForeground_->AddEtsObserverObject(env, aniObserver);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "nativeOn end");
}

void EtsAbilityManager::NativeOff(ani_env *env, ani_string aniType, ani_object aniObserver)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "nativeOff called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    std::string strType;
    if (!AppExecFwk::GetStdString(env, aniType, strType)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetStdString failed");
        EtsErrorUtil::ThrowInvalidParamError(env,
            "Parse param observer failed, must be a AbilityForegroundStateObserver.");
        return;
    }
    if (observerForeground_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null observer");
        EtsErrorUtil::ThrowError(env, static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER),
            GetInnerErrorMsg(AbilityInnerErrorMsg::OBSERVER_NOT_EXIST));
        return;
    }
    ani_status status = ANI_OK;
    ani_boolean isUndefined = false;
    if ((status = env->Reference_IsUndefined(aniObserver, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to check undefined status : %{public}d", status);
        return;
    }
    if (!isUndefined) {
        observerForeground_->RemoveEtsObserverObject(aniObserver);
    } else {
        observerForeground_->RemoveAllEtsObserverObject();
    }
    if (observerForeground_->IsEmpty()) {
        auto appManager = GetAppManagerInstance();
        if (appManager == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "appManager null ptr");
            EtsErrorUtil::ThrowError(env, static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER),
                GetInnerErrorMsg(AbilityInnerErrorMsg::SERVICE_UNAVAILABLE));
            return;
        }
        int32_t ret = appManager->UnregisterAbilityForegroundStateObserver(observerForeground_);
        TAG_LOGD(AAFwkTag::ABILITYMGR, "ret: %{public}d", ret);
        if (ret != NO_ERROR) {
            EtsErrorUtil::ThrowErrorByNativeErr(env, static_cast<int32_t>(ret));
        }
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "nativeOff end");
}

bool CheckIsNumString(const std::string &numStr)
{
    const std::regex regexJsperf(R"(^\d*)");
    std::match_results<std::string::const_iterator> matchResults;
    if (numStr.empty() || !std::regex_match(numStr, matchResults, regexJsperf)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "parse failed: %{public}s", numStr.c_str());
        return false;
    }
    if (MAX_UINT64_VALUE.length() < numStr.length() ||
        (MAX_UINT64_VALUE.length() == numStr.length() && MAX_UINT64_VALUE.compare(numStr) < 0)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "parse failed: %{public}s", numStr.c_str());
        return false;
    }
    return true;
}

void EtsAbilityManager::NativeNotifyDebugAssertResultCheck(ani_env *env, ani_string aniSessionId,
    ani_object userStatusObj)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "NativeNotifyDebugAssertResultCheck called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    std::string sessionId;
    if (!AppExecFwk::GetStdString(env, aniSessionId, sessionId) || !CheckIsNumString(sessionId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "convert sessionId failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param sessionId failed, must be a string.");
        return;
    }
    uint64_t assertSessionId = 0;
    try {
        assertSessionId = std::stoull(sessionId);
    } catch (...) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Parse param sessionId failed.");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param sessionId failed.");
        return;
    }
    if (assertSessionId == 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "convert sessionId failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param sessionId failed, value must not be equal to zero.");
        return;
    }
    int32_t userStatus;
    if (!AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative(env, userStatusObj, userStatus)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "convert status failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param status failed, must be a UserStatus.");
        return;
    }
}

void EtsAbilityManager::NativeNotifyDebugAssertResult(ani_env *env, ani_string aniSessionId,
    ani_object userStatusObj, ani_object callbackObj)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "NativeNotifyDebugAssertResult called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    std::string sessionId;
    if (!AppExecFwk::GetStdString(env, aniSessionId, sessionId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "convert sessionId failed");
        return;
    }

    uint64_t assertSessionId = 0;
    try {
        assertSessionId = std::stoull(sessionId);
    } catch (...) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Parse param sessionId failed.");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param sessionId failed.");
        return;
    }
    if (assertSessionId == 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "convert sessionId failed");
        return;
    }
    int32_t userStatus;
    if (!AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative(env, userStatusObj, userStatus)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "convert status failed");
        return;
    }
    auto amsClient = AAFwk::AbilityManagerClient::GetInstance();
    if (amsClient == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null amsClient");
        AppExecFwk::AsyncCallback(env, callbackObj,
            EtsErrorUtil::CreateErrorByNativeErr(env, AAFwk::INNER_ERR), nullptr);
        return;
    }
    auto ret = amsClient->NotifyDebugAssertResult(assertSessionId, static_cast<AAFwk::UserStatus>(userStatus));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed %{public}d", ret);
    }
    AppExecFwk::AsyncCallback(env, callbackObj, EtsErrorUtil::CreateErrorByNativeErr(env, ret), nullptr);
}

void EtsAbilityManager::NativeSetResidentProcessEnabledCheck(ani_env *env, ani_string aniBundleName)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "NativeSetResidentProcessEnabledCheck called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }

    std::string bundleName;
    if (!AppExecFwk::GetStdString(env, aniBundleName, bundleName) || bundleName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "parse bundleName failed, not string");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param bundleName failed, must be a string.");
        return;
    }
}

void EtsAbilityManager::NativeSetResidentProcessEnabled(ani_env *env, ani_string aniBundleName,
    ani_boolean enabled, ani_object callbackObj)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "NativeSetResidentProcessEnabled called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }

    std::string bundleName;
    if (!AppExecFwk::GetStdString(env, aniBundleName, bundleName) || bundleName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "parse bundleName failed, not string");
        return;
    }
    bool enableState = (enabled != 0);
    auto amsClient = AAFwk::AbilityManagerClient::GetInstance();
    int32_t ret = ERR_OK;
    if (amsClient == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null amsClient");
        ret = static_cast<int32_t>(AAFwk::INNER_ERR);
    } else {
        ret = amsClient->SetResidentProcessEnabled(bundleName, enableState);
    }

    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "error: %{public}d", ret);
        AppExecFwk::AsyncCallback(env, callbackObj,
            EtsErrorUtil::CreateErrorByNativeErr(env, ret), nullptr);
        return;
    }
    AppExecFwk::AsyncCallback(env, callbackObj,
        EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), nullptr);
}

void EtsAbilityManager::NativeAcquireShareData(ani_env *env, ani_int aniMissionId, ani_object callbackObj)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "NativeAcquireShareData called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    int32_t missionId = aniMissionId;
    ani_ref callbackRef = nullptr;
    env->GlobalReference_Create(callbackObj, &callbackRef);

    auto shareDataCallbackStub = CreateShareDataCallbackStub(env, callbackRef);
    if (shareDataCallbackStub == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null shareDataCallbackStub");
        AppExecFwk::AsyncCallback(env, callbackObj,
            EtsErrorUtil::CreateErrorByNativeErr(env, AAFwk::INNER_ERR, "",
                GetInnerErrorMsg(AbilityInnerErrorMsg::ACQUIRE_SHARE_DATA_FAILED)), nullptr);
        env->GlobalReference_Delete(callbackRef);
        return;
    }

    auto amsClient = AAFwk::AbilityManagerClient::GetInstance();
    int32_t err = ERR_OK;
    if (amsClient == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null amsClient");
        err = static_cast<int32_t>(AAFwk::INNER_ERR);
    } else {
        err = amsClient->AcquireShareData(missionId, shareDataCallbackStub);
    }
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "error: %{public}d", err);
        AppExecFwk::AsyncCallback(env, callbackObj,
            EtsErrorUtil::CreateErrorByNativeErr(env, err, "",
                GetInnerErrorMsg(AbilityInnerErrorMsg::ACQUIRE_SHARE_DATA_FAILED)), nullptr);
        env->GlobalReference_Delete(callbackRef);
    }
}

void EtsAbilityManager::NativeUpdateConfiguration(ani_env *env, ani_object configObj, ani_object callbackObj)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "NativeUpdateConfiguration called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }

    AppExecFwk::Configuration changeConfig;
    if (!AppExecFwk::UnwrapConfiguration(env, configObj, changeConfig)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "UnwrapConfiguration failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param config failed, must be a Configuration.");
        return;
    }

    auto appManager = GetAppManagerInstance();
    int32_t errcode = ERR_OK;
    if (appManager == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "appManager is null");
        errcode = static_cast<int32_t>(AAFwk::INNER_ERR);
    } else {
        errcode = appManager->UpdateConfiguration(changeConfig);
    }

    if (errcode != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "UpdateConfiguration failed: %{public}d", errcode);
        AppExecFwk::AsyncCallback(env, callbackObj,
            EtsErrorUtil::CreateErrorByNativeErr(env, errcode), nullptr);
        return;
    }
    AppExecFwk::AsyncCallback(env, callbackObj,
        EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), nullptr);
}

void EtsAbilityManager::GetExtensionRunningInfos(ani_env *env, ani_int upperLimit, ani_object callback)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call GetExtensionRunningInfos");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    std::vector<AAFwk::ExtensionRunningInfo> infos;
    auto errcode = AAFwk::AbilityManagerClient::GetInstance()->GetExtensionRunningInfos(upperLimit, infos);
    if (errcode != ERR_OK) {
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateErrorByNativeErr(env, errcode), nullptr);
        return;
    }

    ani_object extensionArray = nullptr;
    AbilityManagerEts::WrapExtensionRunningInfoArray(env, extensionArray, infos);
    if (extensionArray == nullptr) {
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateErrorByNativeErr(env,
                static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER)), nullptr);
        return;
    }
    AppExecFwk::AsyncCallback(env, callback,
        EtsErrorUtil::CreateErrorByNativeErr(env, errcode), extensionArray);
}

void EtsAbilityManager::NativeOnPreloadedUIExtensionAbilityLoaded(ani_env *env, ani_fn_object callback)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call NativeOnPreloadedUIExtensionAbilityLoaded");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    {
        std::lock_guard<std::mutex> lock(loadedCallbackMutex_);
        for (const auto &cb : loadedCallback_) {
            ani_boolean isEquals = ANI_FALSE;
            env->Reference_StrictEquals(callback, cb.first, &isEquals);
            if (isEquals) {
                TAG_LOGW(AAFwkTag::ABILITYMGR, "callback already exists");
                return;
            }
        }
    }
    ani_ref callbackRef = nullptr;
    ani_status status = env->GlobalReference_Create(callback, &callbackRef);
    if (status != ANI_OK || callbackRef == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GlobalReference_Create failed or null callbackRef, status: %{public}d", status);
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }
    ani_vm *vm = nullptr;
    if (env->GetVM(&vm) != ANI_OK || vm == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR,
            "ani_env::GetVM failed or returned null ETS Virtual Machine instance");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }
    auto client = std::make_shared<EtsPreloadUIExtensionCallbackClient>(vm, callbackRef);
    int32_t key = PreloadUIExtensionHostClient::GetInstance()->AddLoadedCallback(client);
    std::lock_guard<std::mutex> lock(loadedCallbackMutex_);
    loadedCallback_.push_back({ callbackRef, key });
}

void EtsAbilityManager::NativeOffPreloadedUIExtensionAbilityLoaded(ani_env *env, ani_fn_object callback)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call NativeOffPreloadedUIExtensionAbilityLoaded");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    ani_boolean isUndefined = true;
    env->Reference_IsUndefined(callback, &isUndefined);
    std::lock_guard<std::mutex> lock(loadedCallbackMutex_);
    if (isUndefined) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "callback is undefined");
        if (loadedCallback_.empty()) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "null callback");
            return;
        }
        PreloadUIExtensionHostClient::GetInstance()->RemoveAllLoadedCallback();
        loadedCallback_.clear();
        return;
    }
    auto it = std::find_if(loadedCallback_.begin(), loadedCallback_.end(), [env, callback](const auto &cb) {
        ani_boolean isEquals = ANI_FALSE;
        env->Reference_StrictEquals(callback, cb.first, &isEquals);
        return isEquals;
    });
    if (it == loadedCallback_.end()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "callback not found");
        return;
    }
    PreloadUIExtensionHostClient::GetInstance()->RemoveLoadedCallback(it->second);
    loadedCallback_.erase(it);
}

void EtsAbilityManager::NativeOnPreloadedUIExtensionAbilityDestroyed(ani_env *env, ani_fn_object callback)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call NativeOnPreloadedUIExtensionAbilityDestroyed");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    {
        std::lock_guard<std::mutex> lock(destroyedCallbackMutex_);
        for (const auto &cb : destroyCallback_) {
            ani_boolean isEquals = ANI_FALSE;
            env->Reference_StrictEquals(callback, cb.first, &isEquals);
            if (isEquals) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "callback already exists");
                return;
            }
        }
    }
    ani_ref callbackRef = nullptr;
    ani_status status = env->GlobalReference_Create(callback, &callbackRef);
    if (status != ANI_OK || callbackRef == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GlobalReference_Create failed or null callbackRef, status: %{public}d", status);
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }
    ani_vm *vm = nullptr;
    if (env->GetVM(&vm) != ANI_OK || vm == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR,
            "ani_env::GetVM failed or returned null ETS Virtual Machine instance");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }
    auto client = std::make_shared<EtsPreloadUIExtensionCallbackClient>(vm, callbackRef);
    int32_t key = PreloadUIExtensionHostClient::GetInstance()->AddDestroyCallback(client);
    std::lock_guard<std::mutex> lock(destroyedCallbackMutex_);
    destroyCallback_.push_back({ callbackRef, key });
}

void EtsAbilityManager::NativeOffPreloadedUIExtensionAbilityDestroyed(ani_env *env, ani_fn_object callback)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call NativeOffPreloadedUIExtensionAbilityDestroyed");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    ani_boolean isUndefined = true;
    env->Reference_IsUndefined(callback, &isUndefined);
    std::lock_guard<std::mutex> lock(destroyedCallbackMutex_);
    if (isUndefined) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "callback is undefined");
        if (destroyCallback_.empty()) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "null callback");
            return;
        }
        PreloadUIExtensionHostClient::GetInstance()->RemoveAllDestroyCallback();
        destroyCallback_.clear();
        return;
    }
    auto it = std::find_if(destroyCallback_.begin(), destroyCallback_.end(), [env, callback](const auto &cb) {
        ani_boolean isEquals = ANI_FALSE;
        env->Reference_StrictEquals(callback, cb.first, &isEquals);
        return isEquals;
    });
    if (it == destroyCallback_.end()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "callback not found");
        return;
    }
    PreloadUIExtensionHostClient::GetInstance()->RemoveDestroyCallback(it->second);
    destroyCallback_.erase(it);
}

void EtsAbilityManager::NativePreloadUIExtensionAbility(ani_env *env, ani_object aniWant, ani_object callback)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call NativePreloadUIExtensionAbility");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, aniWant, want)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to parse want");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param want failed, must be a Want");
        return;
    }
    ani_vm *etsVm = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->GetVM(&etsVm)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "status: %{public}d", status);
        return;
    }
    ani_ref callbackRef = nullptr;
    env->GlobalReference_Create(callback, &callbackRef);
    PreloadTask task = [etsVm, callbackRef](int32_t preloadId, int32_t innerErrCode) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "start async callback");
        bool isAttachThread = false;
        ani_env *env = AppExecFwk::AttachAniEnv(etsVm, isAttachThread);
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
            AppExecFwk::AsyncCallback(env, reinterpret_cast<ani_object>(callbackRef),
                EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER)),
                nullptr);
            return;
        }
        if (innerErrCode != ERR_OK) {
            AppExecFwk::AsyncCallback(env, reinterpret_cast<ani_object>(callbackRef),
                EtsErrorUtil::CreateErrorByNativeErr(env, innerErrCode), nullptr);
        } else {
            AppExecFwk::AsyncCallback(
                env, reinterpret_cast<ani_object>(callbackRef), nullptr, AppExecFwk::CreateInt(env, preloadId));
        }
        AppExecFwk::DetachAniEnv(etsVm, isAttachThread);
    };
    auto context = OHOS::AbilityRuntime::Context::GetApplicationContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null context");
        EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }
    std::string bundleName = context->GetBundleName();
    PreloadUIExtensionHostClient::GetInstance()->PreloadUIExtensionAbility(want, bundleName, std::move(task));
}

void EtsAbilityManager::NativeClearPreloadedUIExtensionAbility(ani_env *env, ani_int preloadId, ani_object callback)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call NativeClearPreloadedUIExtensionAbility");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    ErrCode result = AAFwk::AbilityManagerClient::GetInstance()->ClearPreloadedUIExtensionAbility(preloadId);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ClearPreloadedUIExtensionAbility failed, result: %{public}d", result);
        AppExecFwk::AsyncCallback(
            env, callback, EtsErrorUtil::CreateErrorByNativeErr(env, result), nullptr);
    }
}

void EtsAbilityManager::NativeClearPreloadedUIExtensionAbilities(ani_env *env, ani_object callback)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call NativeClearPreloadedUIExtensionAbilities");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    ErrCode result = AAFwk::AbilityManagerClient::GetInstance()->ClearPreloadedUIExtensionAbilities();
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ClearPreloadedUIExtensionAbilities failed, result: %{public}d", result);
        AppExecFwk::AsyncCallback(
            env, callback, EtsErrorUtil::CreateErrorByNativeErr(env, result), nullptr);
    }
}

void EtsAbilityManager::nativeNotifySaveAsResultCheck(ani_env *env, ani_object aniParameter)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call NotifySaveAsResultCheck");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    AppExecFwk::Want want;
    int resultCode = ERR_OK;
    if (!AppExecFwk::UnWrapAbilityResult(env, aniParameter, resultCode, want)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "unwrap abilityResult failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param parameter failed, must be a AbilityResult.");
        return;
    }
}

void EtsAbilityManager::nativeNotifySaveAsResult(
    ani_env *env, ani_object aniParameter, ani_int requestCode, ani_object callback)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call NotifySaveAsResult");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    AppExecFwk::Want want;
    int resultCode = ERR_OK;
    if (!AppExecFwk::UnWrapAbilityResult(env, aniParameter, resultCode, want)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "unwrap abilityResult failed");
        return;
    }
    ErrCode errorCode = AAFwk::AbilityManagerClient::GetInstance()->
        NotifySaveAsResult(want, resultCode, static_cast<int32_t>(requestCode));
    if (errorCode != ERR_OK) {
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(errorCode)), nullptr);
        return;
    }
    AppExecFwk::AsyncCallback(env, callback, nullptr, nullptr);
}

void EtsAbilityManagerRegistryInit(ani_env *env)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call EtsAbilityManagerRegistryInit");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    ani_status status = ANI_ERROR;
    if (env->ResetError() != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ResetError failed");
    }
    ani_namespace ns = nullptr;
    status = env->FindNamespace(ETS_ABILITY_MANAGER_NAMESPACE, &ns);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "FindNamespace abilityManager failed status : %{public}d", status);
        return;
    }
    std::array methods = {
        ani_native_function {
            "nativeGetForegroundUIAbilities", ETS_ABILITY_MANAGER_SIGNATURE_ARRAY,
            reinterpret_cast<void *>(EtsAbilityManager::GetForegroundUIAbilities)
        },
        ani_native_function {
            "getForegroundUIAbilitiesCallback", "C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsAbilityManager::GetForegroundUIAbilitiesCallBack)
        },
        ani_native_function { "nativeGetTopAbility", ETS_ABILITY_MANAGER_SIGNATURE_CALLBACK,
            reinterpret_cast<void *>(EtsAbilityManager::GetTopAbility) },
        ani_native_function { "nativeCheckSystemApp", ETS_ABILITY_MANAGER_SIGNATURE_VOID,
            reinterpret_cast<void *>(EtsAbilityManager::CheckSystemApp) },
        ani_native_function { "nativeGetTopAbilityCheck", ":",
            reinterpret_cast<void *>(EtsAbilityManager::GetTopAbilityCheck) },
        ani_native_function { "nativeGetAbilityRunningInfos", "C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsAbilityManager::GetAbilityRunningInfos) },
        ani_native_function { "nativeGetExtensionRunningInfos", "iC{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsAbilityManager::GetExtensionRunningInfos) },
        ani_native_function { "nativeIsEmbeddedUIExtensionSupported", ":z",
            reinterpret_cast<void *>(EtsAbilityManager::IsEmbeddedUIExtensionSupported) },
        ani_native_function { "nativeIsEmbeddedOpenAllowed",
            "C{application.Context.Context}C{std.core.String}C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsAbilityManager::IsEmbeddedOpenAllowed) },
        ani_native_function { "nativeIsEmbeddedOpenAllowedCheck", "C{application.Context.Context}:",
            reinterpret_cast<void *>(EtsAbilityManager::IsEmbeddedOpenAllowedCheck) },
        ani_native_function { "nativeQueryAtomicServiceStartupRule",
            "C{application.Context.Context}C{std.core.String}C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsAbilityManager::QueryAtomicServiceStartupRule) },
        ani_native_function { "nativeQueryAtomicServiceStartupRuleCheck", "C{application.Context.Context}:",
            reinterpret_cast<void *>(EtsAbilityManager::QueryAtomicServiceStartupRuleCheck) },
        ani_native_function { "nativeOn", nullptr, reinterpret_cast<void *>(EtsAbilityManager::NativeOn) },
        ani_native_function { "nativeOff", nullptr, reinterpret_cast<void *>(EtsAbilityManager::NativeOff) },
        ani_native_function { "nativeNotifyDebugAssertResult",
            nullptr, reinterpret_cast<void *>(EtsAbilityManager::NativeNotifyDebugAssertResult) },
        ani_native_function { "nativeNotifyDebugAssertResultCheck",
            nullptr, reinterpret_cast<void *>(EtsAbilityManager::NativeNotifyDebugAssertResultCheck) },
        ani_native_function { "nativeSetResidentProcessEnabled",
            nullptr, reinterpret_cast<void *>(EtsAbilityManager::NativeSetResidentProcessEnabled) },
        ani_native_function { "nativeSetResidentProcessEnabledCheck",
            nullptr, reinterpret_cast<void *>(EtsAbilityManager::NativeSetResidentProcessEnabledCheck) },
        ani_native_function { "nativeAcquireShareData",
            nullptr, reinterpret_cast<void *>(EtsAbilityManager::NativeAcquireShareData) },
        ani_native_function { "nativeUpdateConfiguration",
            nullptr, reinterpret_cast<void *>(EtsAbilityManager::NativeUpdateConfiguration) },
        ani_native_function { "nativeOnPreloadedUIExtensionAbilityLoaded",
            ON_OFF_PRELOAD_UI_EXTENSION_ABILITY_SIGNATURE,
            reinterpret_cast<void *>(EtsAbilityManager::NativeOnPreloadedUIExtensionAbilityLoaded) },
        ani_native_function { "nativeOffPreloadedUIExtensionAbilityLoaded",
            ON_OFF_PRELOAD_UI_EXTENSION_ABILITY_SIGNATURE,
            reinterpret_cast<void *>(EtsAbilityManager::NativeOffPreloadedUIExtensionAbilityLoaded) },
        ani_native_function { "nativeOnPreloadedUIExtensionAbilityDestroyed",
            ON_OFF_PRELOAD_UI_EXTENSION_ABILITY_SIGNATURE,
            reinterpret_cast<void *>(EtsAbilityManager::NativeOnPreloadedUIExtensionAbilityDestroyed) },
        ani_native_function { "nativeOffPreloadedUIExtensionAbilityDestroyed",
            ON_OFF_PRELOAD_UI_EXTENSION_ABILITY_SIGNATURE,
            reinterpret_cast<void *>(EtsAbilityManager::NativeOffPreloadedUIExtensionAbilityDestroyed) },
        ani_native_function { "nativePreloadUIExtensionAbility", PRELOAD_UI_EXTENSION_ABILITY_SIGNATURE,
            reinterpret_cast<void *>(EtsAbilityManager::NativePreloadUIExtensionAbility) },
        ani_native_function { "nativeClearPreloadedUIExtensionAbility", CLEAR_PRELOAD_UI_EXTENSION_ABILITY_SIGNATURE,
            reinterpret_cast<void *>(EtsAbilityManager::NativeClearPreloadedUIExtensionAbility) },
        ani_native_function { "nativeClearPreloadedUIExtensionAbilities",
            CLEAR_PRELOAD_UI_EXTENSION_ABILITIES_SIGNATURE,
            reinterpret_cast<void *>(EtsAbilityManager::NativeClearPreloadedUIExtensionAbilities) },
        ani_native_function { "nativeCheckPreloadUIExtensionAbilityPermission", ETS_ABILITY_MANAGER_SIGNATURE_VOID,
            reinterpret_cast<void *>(EtsAbilityManager::CheckPreloadUIExtensionAbilityPermission) },
        ani_native_function { "nativeNotifySaveAsResult", NOTIFY_SAVE_AS_RESULT_SIGNATURE,
            reinterpret_cast<void *>(EtsAbilityManager::nativeNotifySaveAsResult) },
        ani_native_function { "nativeNotifySaveAsResultCheck", NOTIFY_SAVE_AS_RESULT_CHECK_SIGNATURE,
            reinterpret_cast<void *>(EtsAbilityManager::nativeNotifySaveAsResultCheck) },
    };
    status = env->Namespace_BindNativeFunctions(ns, methods.data(), methods.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Namespace_BindNativeFunctions failed status : %{public}d", status);
    }
    if (env->ResetError() != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ResetError failed");
    }
}

extern "C" {
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "in AbilityManagerEts.ANI_Constructor");
    if (vm == nullptr || result == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null vm or result");
        return ANI_INVALID_ARGS;
    }

    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    status = vm->GetEnv(ANI_VERSION_1, &env);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetEnv failed, status=%{public}d", status);
        return ANI_NOT_FOUND;
    }
    EtsAbilityManagerRegistryInit(env);
    *result = ANI_VERSION_1;
    TAG_LOGD(AAFwkTag::ABILITYMGR, "AbilityManagerEts.ANI_Constructor finished");
    return ANI_OK;
}
}
} // namespace AbilityManagerEts
} // namespace OHOS
