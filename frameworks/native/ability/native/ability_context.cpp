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

#include "fa_ability_context.h"

#include "ability_manager_client.h"
#include "accesstoken_kit.h"
#include "authorization_result.h"
#include "bundle_constants.h"
#include "hilog_tag_wrapper.h"
#include "iservice_registry.h"
#include "os_account_manager_wrapper.h"
#include "resource_manager.h"
#include "sys_mgr_client.h"
#include "system_ability_definition.h"
#include "hitrace_meter.h"
#include "remote_object_wrapper.h"
#ifdef SUPPORT_SCREEN
#include "scene_board_judgement.h"
#endif // SUPPORT_SCREEN
#include "session/host/include/zidl/session_interface.h"
#include "session_info.h"
#include "string_wrapper.h"
#include "want_params_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
int AbilityContext::ABILITY_CONTEXT_DEFAULT_REQUEST_CODE(0);
namespace {
const std::string GRANT_ABILITY_BUNDLE_NAME = "com.ohos.permissionmanager";
const std::string GRANT_ABILITY_ABILITY_NAME = "com.ohos.permissionmanager.GrantAbility";
const std::string PERMISSION_KEY = "ohos.user.grant.permission";
const std::string STATE_KEY = "ohos.user.grant.permission.state";
const std::string TOKEN_KEY = "ohos.ability.params.token";
const std::string CALLBACK_KEY = "ohos.ability.params.callback";
}

ErrCode AbilityContext::StartAbility(const AAFwk::Want &want, int requestCode)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "requestCode = %{public}d", requestCode);
    AppExecFwk::AbilityType type = GetAbilityInfoType();
    if (type != AppExecFwk::AbilityType::PAGE && type != AppExecFwk::AbilityType::SERVICE) {
        TAG_LOGE(AAFwkTag::CONTEXT, "abilityType: %{public}d", type);
        return ERR_INVALID_VALUE;
    }
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, token_, requestCode);
    TAG_LOGD(AAFwkTag::CONTEXT, "ret=%{public}d", err);
    return err;
}

ErrCode AbilityContext::StartAbility(const Want &want, int requestCode, const AbilityStartSetting &abilityStartSetting)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "requestCode: %{public}d",
        requestCode);
    AppExecFwk::AbilityType type = GetAbilityInfoType();
    if (type != AppExecFwk::AbilityType::PAGE && type != AppExecFwk::AbilityType::SERVICE) {
        TAG_LOGE(AAFwkTag::CONTEXT, "abilityType: %{public}d", type);
        return ERR_INVALID_VALUE;
    }
    ErrCode err =
        AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, abilityStartSetting, token_, requestCode);
    TAG_LOGD(AAFwkTag::CONTEXT, "ret=%{public}d", err);
    return err;
}

ErrCode AbilityContext::TerminateAbility()
{
    std::shared_ptr<AbilityInfo> info = GetAbilityInfo();
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null info");
        return ERR_NULL_OBJECT;
    }

    ErrCode err = ERR_OK;
    switch (info->type) {
        case AppExecFwk::AbilityType::PAGE:
            TAG_LOGD(AAFwkTag::CONTEXT, "type:page, ability: %{public}s", info->name.c_str());
#ifdef SUPPORT_SCREEN
            if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
                auto sessionToken = GetSessionToken();
                if (sessionToken == nullptr) {
                    TAG_LOGE(AAFwkTag::CONTEXT, "null sessionToken");
                    return ERR_INVALID_VALUE;
                }
                sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
                sessionInfo->want = resultWant_;
                sessionInfo->resultCode = resultCode_;
                TAG_LOGI(AAFwkTag::CONTEXT, "resultCode: %{public}d", sessionInfo->resultCode);
                auto ifaceSessionToken = iface_cast<Rosen::ISession>(sessionToken);
                if (ifaceSessionToken == nullptr) {
                    TAG_LOGE(AAFwkTag::CONTEXT, "null sessionToken");
                    return ERR_INVALID_VALUE;
                }
                auto err = ifaceSessionToken->TerminateSession(sessionInfo);
                TAG_LOGI(AAFwkTag::CONTEXT, "ret: %{public}d", err);
                return static_cast<int32_t>(err);
            } else {
                err = AAFwk::AbilityManagerClient::GetInstance()->TerminateAbility(token_, resultCode_, &resultWant_);
            }
#endif // SUPPORT_SCREEN
            break;
        case AppExecFwk::AbilityType::SERVICE:
            TAG_LOGD(AAFwkTag::CONTEXT, "service type, ability: %{public}s", info->name.c_str());
            err = AAFwk::AbilityManagerClient::GetInstance()->TerminateAbility(token_, -1, nullptr);
            break;
        default:
            TAG_LOGE(AAFwkTag::CONTEXT, "error type: %{public}d", info->type);
            break;
    }

    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "failed %{public}d", err);
    }
    return err;
}

std::string AbilityContext::GetCallingBundle()
{
    return callingBundleName_;
}

std::shared_ptr<AppExecFwk::ElementName> AbilityContext::GetElementName()
{
    TAG_LOGD(AAFwkTag::CONTEXT, "called");
    std::shared_ptr<AbilityInfo> info = GetAbilityInfo();
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null info");
        return nullptr;
    }

    std::shared_ptr<ElementName> elementName =
        std::make_shared<ElementName>(info->deviceId, info->bundleName, info->name, info->moduleName);
    TAG_LOGD(AAFwkTag::CONTEXT, "end");
    return elementName;
}

std::shared_ptr<ElementName> AbilityContext::GetCallingAbility()
{
    TAG_LOGD(AAFwkTag::CONTEXT, "called");
    std::shared_ptr<ElementName> elementName = std::make_shared<ElementName>(
            callingDeviceId_, callingBundleName_, callingAbilityName_, callingModuleName_);
    TAG_LOGD(AAFwkTag::CONTEXT, "end");
    return elementName;
}

bool AbilityContext::ConnectAbility(const Want &want, const sptr<AAFwk::IAbilityConnection> &conn)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    AppExecFwk::AbilityType type = GetAbilityInfoType();

    std::shared_ptr<AbilityInfo> abilityInfo = GetAbilityInfo();
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null info");
        return false;
    }

    TAG_LOGD(AAFwkTag::CONTEXT, "ability:%{public}s", abilityInfo->name.c_str());

    if (type != AppExecFwk::AbilityType::PAGE && type != AppExecFwk::AbilityType::SERVICE) {
        TAG_LOGE(AAFwkTag::CONTEXT, "abilityType: %{public}d", type);
        return false;
    }

    ErrCode ret = AAFwk::AbilityManagerClient::GetInstance()->ConnectAbility(want, conn, token_);
    TAG_LOGD(AAFwkTag::CONTEXT, "ret=%{public}d", ret);
    bool value = ((ret == ERR_OK) ? true : false);
    if (!value) {
        TAG_LOGE(AAFwkTag::CONTEXT, "errorCode: %{public}d", ret);
    }
    TAG_LOGD(AAFwkTag::CONTEXT, "end");
    return value;
}

ErrCode AbilityContext::DisconnectAbility(const sptr<AAFwk::IAbilityConnection> &conn)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::shared_ptr<AbilityInfo> info = GetAbilityInfo();
    TAG_LOGI(AAFwkTag::CONTEXT, "caller:%{public}s",
        info == nullptr ? "" : info->name.c_str());

    AppExecFwk::AbilityType type = GetAbilityInfoType();
    if (type != AppExecFwk::AbilityType::PAGE && type != AppExecFwk::AbilityType::SERVICE) {
        TAG_LOGE(AAFwkTag::CONTEXT, "abilityType:%{public}d", type);
        return ERR_INVALID_VALUE;
    }

    ErrCode ret = AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(conn);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "error, ret:%{public}d", ret);
    }
    return ret;
}

bool AbilityContext::StopAbility(const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "called");
    AppExecFwk::AbilityType type = GetAbilityInfoType();
    if (type != AppExecFwk::AbilityType::PAGE && type != AppExecFwk::AbilityType::SERVICE) {
        TAG_LOGE(AAFwkTag::CONTEXT, "abilityType: %{public}d", type);
        return false;
    }

    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StopServiceAbility(want, token_);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "failed %{public}d", err);
        return false;
    }

    return true;
}

sptr<IRemoteObject> AbilityContext::GetToken()
{
    return token_;
}

AppExecFwk::AbilityType AbilityContext::GetAbilityInfoType()
{
    std::shared_ptr<AbilityInfo> info = GetAbilityInfo();
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null info");
        return AppExecFwk::AbilityType::UNKNOWN;
    }

    return info->type;
}

std::shared_ptr<Global::Resource::ResourceManager> AbilityContext::GetResourceManager() const
{
    std::shared_ptr<Context> appContext = GetApplicationContext();
    if (appContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null appContext");
        return nullptr;
    }

    TAG_LOGD(AAFwkTag::CONTEXT, "before getResourceManager");
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager = appContext->GetResourceManager();
    TAG_LOGD(AAFwkTag::CONTEXT, "after getResourceManager");
    if (resourceManager == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null resourceManager");
        return nullptr;
    }
    return resourceManager;
}

int AbilityContext::VerifyPermission(const std::string &permission, int pid, int uid)
{
    TAG_LOGI(AAFwkTag::CONTEXT, "permission=%{public}s, pid=%{public}d, uid=%{public}d",
        permission.c_str(),
        pid,
        uid);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->VerifyPermission(permission, pid, uid);
    TAG_LOGD(AAFwkTag::CONTEXT, "ret=%{public}d", err);
    if (err != ERR_OK) {
        return AppExecFwk::Constants::PERMISSION_NOT_GRANTED;
    }
    return 0;
}

void AbilityContext::GetPermissionDes(const std::string &permissionName, std::string &des)
{
    Security::AccessToken::PermissionDef permissionDef;
    int32_t ret = Security::AccessToken::AccessTokenKit::GetDefPermission(permissionName, permissionDef);
    if (ret == Security::AccessToken::AccessTokenKitRet::RET_SUCCESS) {
        TAG_LOGD(AAFwkTag::CONTEXT, "%{public}s: RET_SUCCESS", permissionName.c_str());
        des = permissionDef.description;
    }
    TAG_LOGD(AAFwkTag::CONTEXT, "end");
}

void AbilityContext::RequestPermissionsFromUser(std::vector<std::string> &permissions,
    std::vector<int> &permissionsState, PermissionRequestTask &&task)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "called");
    if (permissions.size() == 0) {
        TAG_LOGE(AAFwkTag::CONTEXT, "empty permissions");
        return;
    }

    Security::AccessToken::PermissionGrantInfo info;
    Security::AccessToken::AccessTokenKit::GetPermissionManagerInfo(info);

    AAFwk::Want want;
    if (info.grantBundleName.empty() || info.grantServiceAbilityName.empty()) {
        TAG_LOGD(AAFwkTag::CONTEXT, "default");
        want.SetElementName(GRANT_ABILITY_BUNDLE_NAME, GRANT_ABILITY_ABILITY_NAME);
    } else {
        want.SetElementName(info.grantBundleName, info.grantServiceAbilityName);
    }

    want.SetParam(PERMISSION_KEY, permissions);
    want.SetParam(STATE_KEY, permissionsState);
    want.SetParam(TOKEN_KEY, token_);
    sptr<IRemoteObject> remoteObject = sptr<AbilityRuntime::AuthorizationResult>::MakeSptr(std::move(task));
    want.SetParam(CALLBACK_KEY, remoteObject);
    StartAbility(want, -1);
    TAG_LOGD(AAFwkTag::CONTEXT, "end");
}

void AbilityContext::SetCallingContext(const std::string &deviceId, const std::string &bundleName,
    const std::string &abilityName, const std::string &moduleName)
{
    callingDeviceId_ = deviceId;
    callingBundleName_ = bundleName;
    callingAbilityName_ = abilityName;
    callingModuleName_ = moduleName;
}

void AbilityContext::StartAbilities(const std::vector<AAFwk::Want> &wants)
{
    for (auto want : wants) {
        StartAbility(want, ABILITY_CONTEXT_DEFAULT_REQUEST_CODE);
    }
    TAG_LOGD(AAFwkTag::CONTEXT, "end");
}

sptr<IRemoteObject> AbilityContext::GetSessionToken()
{
    std::lock_guard lock(sessionTokenMutex_);
    return sessionToken_;
}

int32_t AbilityContext::AddFreeInstallObserver(const sptr<AbilityRuntime::IFreeInstallObserver> &observer)
{
    ErrCode ret = AAFwk::AbilityManagerClient::GetInstance()->AddFreeInstallObserver(token_, observer);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "add observer failed, ret: %{public}d", ret);
    }
    return ret;
}
}  // namespace AppExecFwk
}  // namespace OHOS
