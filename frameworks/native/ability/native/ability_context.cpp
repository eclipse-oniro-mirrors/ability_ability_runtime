/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "ability_context.h"

#include "ability_manager_client.h"
#include "accesstoken_kit.h"
#include "authorization_result.h"
#include "bundle_constants.h"
#include "hilog_wrapper.h"
#include "iservice_registry.h"
#include "os_account_manager_wrapper.h"
#include "resource_manager.h"
#include "sys_mgr_client.h"
#include "system_ability_definition.h"
#include "hitrace_meter.h"
#include "remote_object_wrapper.h"
#include "scene_board_judgement.h"
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
    HILOG_DEBUG("AbilityContext::StartAbility called, requestCode = %{public}d", requestCode);
    AppExecFwk::AbilityType type = GetAbilityInfoType();
    if (type != AppExecFwk::AbilityType::PAGE && type != AppExecFwk::AbilityType::SERVICE) {
        HILOG_ERROR("AbilityContext::StartAbility AbilityType = %{public}d", type);
        return ERR_INVALID_VALUE;
    }
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, token_, requestCode);
    HILOG_DEBUG("%{public}s. End calling ams->StartAbility. ret=%{public}d", __func__, err);
    return err;
}

ErrCode AbilityContext::StartAbility(const Want &want, int requestCode, const AbilityStartSetting &abilityStartSetting)
{
    HILOG_DEBUG("AbilityContext::StartAbility with start setting called, requestCode = %{public}d", requestCode);
    AppExecFwk::AbilityType type = GetAbilityInfoType();
    if (type != AppExecFwk::AbilityType::PAGE && type != AppExecFwk::AbilityType::SERVICE) {
        HILOG_ERROR("AbilityContext::StartAbility AbilityType = %{public}d", type);
        return ERR_INVALID_VALUE;
    }
    ErrCode err =
        AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, abilityStartSetting, token_, requestCode);
    HILOG_DEBUG("%{public}s. End calling ams->StartAbility. ret=%{public}d", __func__, err);
    return err;
}

ErrCode AbilityContext::TerminateAbility(int requestCode)
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->TerminateAbility(token_, requestCode);
    if (err != ERR_OK) {
        HILOG_ERROR("AbilityContext::TerminateAbility is failed %{public}d", err);
    }
    return err;
}

ErrCode AbilityContext::TerminateAbility()
{
    std::shared_ptr<AbilityInfo> info = GetAbilityInfo();
    if (info == nullptr) {
        HILOG_ERROR("AbilityContext::TerminateAbility info == nullptr");
        return ERR_NULL_OBJECT;
    }

    ErrCode err = ERR_OK;
    switch (info->type) {
        case AppExecFwk::AbilityType::PAGE:
            HILOG_DEBUG("Terminate ability begin, type is page, ability is %{public}s.", info->name.c_str());
            if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
                auto sessionToken = sessionInfo_->sessionToken;
                if (sessionToken == nullptr) {
                    HILOG_ERROR("sessionToken is nullptr.");
                    return ERR_INVALID_VALUE;
                }
                sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
                sessionInfo->want = resultWant_;
                sessionInfo->resultCode = resultCode_;
                HILOG_INFO("FA TerminateAbility resultCode is %{public}d", sessionInfo->resultCode);
                auto ifaceSessionToken = iface_cast<Rosen::ISession>(sessionToken);
                auto err = ifaceSessionToken->TerminateSession(sessionInfo);
                HILOG_INFO("FA TerminateAbility. ret=%{public}d", err);
                return static_cast<int32_t>(err);
            } else {
                err = AAFwk::AbilityManagerClient::GetInstance()->TerminateAbility(token_, resultCode_, &resultWant_);
            }
            break;
        case AppExecFwk::AbilityType::SERVICE:
            HILOG_DEBUG("Terminate ability begin, type is service, ability is %{public}s.", info->name.c_str());
            err = AAFwk::AbilityManagerClient::GetInstance()->TerminateAbility(token_, -1, nullptr);
            break;
        default:
            HILOG_ERROR("AbilityContext::TerminateAbility info type error is %{public}d", info->type);
            break;
    }

    if (err != ERR_OK) {
        HILOG_ERROR("AbilityContext::TerminateAbility is failed %{public}d", err);
    }
    return err;
}

std::string AbilityContext::GetCallingBundle()
{
    return callingBundleName_;
}

std::shared_ptr<ElementName> AbilityContext::GetElementName()
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    std::shared_ptr<AbilityInfo> info = GetAbilityInfo();
    if (info == nullptr) {
        HILOG_ERROR("AbilityContext::GetElementName info == nullptr");
        return nullptr;
    }

    std::shared_ptr<ElementName> elementName = std::make_shared<ElementName>();
    elementName->SetAbilityName(info->name);
    elementName->SetBundleName(info->bundleName);
    elementName->SetDeviceID(info->deviceId);
    elementName->SetModuleName(info->moduleName);
    HILOG_DEBUG("%{public}s end.", __func__);
    return elementName;
}

std::shared_ptr<ElementName> AbilityContext::GetCallingAbility()
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    std::shared_ptr<ElementName> elementName = std::make_shared<ElementName>();
    elementName->SetAbilityName(callingAbilityName_);
    elementName->SetBundleName(callingBundleName_);
    elementName->SetDeviceID(callingDeviceId_);
    elementName->SetModuleName(callingModuleName_);
    HILOG_DEBUG("%{public}s end.", __func__);
    return elementName;
}

bool AbilityContext::ConnectAbility(const Want &want, const sptr<AAFwk::IAbilityConnection> &conn)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    AppExecFwk::AbilityType type = GetAbilityInfoType();

    std::shared_ptr<AbilityInfo> abilityInfo = GetAbilityInfo();
    if (abilityInfo == nullptr) {
        HILOG_ERROR("AbilityContext::ConnectAbility info == nullptr");
        return false;
    }

    HILOG_INFO("Connect ability begin, ability:%{public}s.", abilityInfo->name.c_str());

    if (type != AppExecFwk::AbilityType::PAGE && type != AppExecFwk::AbilityType::SERVICE) {
        HILOG_ERROR("AbilityContext::ConnectAbility AbilityType = %{public}d", type);
        return false;
    }

    ErrCode ret = AAFwk::AbilityManagerClient::GetInstance()->ConnectAbility(want, conn, token_);
    HILOG_DEBUG("%{public}s end ConnectAbility, ret=%{public}d", __func__, ret);
    bool value = ((ret == ERR_OK) ? true : false);
    if (!value) {
        HILOG_ERROR("AbilityContext::ConnectAbility ErrorCode = %{public}d", ret);
    }
    HILOG_DEBUG("%{public}s end.", __func__);
    return value;
}

ErrCode AbilityContext::DisconnectAbility(const sptr<AAFwk::IAbilityConnection> &conn)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::shared_ptr<AbilityInfo> info = GetAbilityInfo();
    HILOG_INFO("Disconnect ability begin, caller:%{public}s.", info == nullptr ? "" : info->name.c_str());

    AppExecFwk::AbilityType type = GetAbilityInfoType();
    if (type != AppExecFwk::AbilityType::PAGE && type != AppExecFwk::AbilityType::SERVICE) {
        HILOG_ERROR("AbilityContext::DisconnectAbility AbilityType = %{public}d", type);
        return ERR_INVALID_VALUE;
    }

    ErrCode ret = AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(conn);
    if (ret != ERR_OK) {
        HILOG_ERROR("AbilityContext::DisconnectAbility error, ret=%{public}d.", ret);
    }
    return ret;
}

bool AbilityContext::StopAbility(const AAFwk::Want &want)
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    AppExecFwk::AbilityType type = GetAbilityInfoType();
    if (type != AppExecFwk::AbilityType::PAGE && type != AppExecFwk::AbilityType::SERVICE) {
        HILOG_ERROR("AbilityContext::StopAbility AbilityType = %{public}d", type);
        return false;
    }

    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StopServiceAbility(want, token_);
    if (err != ERR_OK) {
        HILOG_ERROR("AbilityContext::StopAbility is failed %{public}d", err);
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
        HILOG_ERROR("AbilityContext::GetAbilityInfoType info == nullptr");
        return AppExecFwk::AbilityType::UNKNOWN;
    }

    return info->type;
}

std::shared_ptr<Global::Resource::ResourceManager> AbilityContext::GetResourceManager() const
{
    std::shared_ptr<Context> appContext = GetApplicationContext();
    if (appContext == nullptr) {
        HILOG_ERROR("AbilityContext::GetResourceManager appContext is nullptr");
        return nullptr;
    }

    HILOG_DEBUG("%{public}s begin appContext->GetResourceManager.", __func__);
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager = appContext->GetResourceManager();
    HILOG_DEBUG("%{public}s end appContext->GetResourceManager.", __func__);
    if (resourceManager == nullptr) {
        HILOG_ERROR("AbilityContext::GetResourceManager resourceManager is nullptr");
        return nullptr;
    }
    return resourceManager;
}

int AbilityContext::VerifyPermission(const std::string &permission, int pid, int uid)
{
    HILOG_INFO("%{public}s begin. permission=%{public}s, pid=%{public}d, uid=%{public}d",
        __func__,
        permission.c_str(),
        pid,
        uid);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->VerifyPermission(permission, pid, uid);
    HILOG_DEBUG("End calling VerifyPermission. ret=%{public}d", err);
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
        HILOG_DEBUG("GetPermissionDes %{public}s: RET_SUCCESS", permissionName.c_str());
        des = permissionDef.description;
    }
    HILOG_DEBUG("%{public}s end GetPermissionDef.", __func__);
}

void AbilityContext::RequestPermissionsFromUser(std::vector<std::string> &permissions,
    std::vector<int> &permissionsState, PermissionRequestTask &&task)
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    if (permissions.size() == 0) {
        HILOG_ERROR("AbilityContext::RequestPermissionsFromUser permissions is empty");
        return;
    }

    AAFwk::Want want;
    want.SetElementName(GRANT_ABILITY_BUNDLE_NAME, GRANT_ABILITY_ABILITY_NAME);
    want.SetParam(PERMISSION_KEY, permissions);
    want.SetParam(STATE_KEY, permissionsState);
    want.SetParam(TOKEN_KEY, token_);
    sptr<IRemoteObject> remoteObject = new AbilityRuntime::AuthorizationResult(std::move(task));
    want.SetParam(CALLBACK_KEY, remoteObject);
    StartAbility(want, -1);
    HILOG_DEBUG("%{public}s end.", __func__);
}

void AbilityContext::SetCallingContext(const std::string &deviceId, const std::string &bundleName,
    const std::string &abilityName, const std::string &moduleName)
{
    callingDeviceId_ = deviceId;
    callingBundleName_ = bundleName;
    callingAbilityName_ = abilityName;
    callingModuleName_ = moduleName;
}

bool AbilityContext::TerminateAbilityResult(int startId)
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    auto abilityClient = AAFwk::AbilityManagerClient::GetInstance();
    if (abilityClient == nullptr) {
        HILOG_ERROR("AbilityContext::TerminateAbilityResult abilityClient is nullptr");
        return false;
    }

    HILOG_DEBUG("%{public}s begin ams->TerminateAbilityResult, startId=%{public}d.", __func__, startId);
    ErrCode errval = abilityClient->TerminateAbilityResult(token_, startId);
    HILOG_DEBUG("%{public}s end ams->TerminateAbilityResult, ret=%{public}d.", __func__, errval);
    if (errval != ERR_OK) {
        HILOG_ERROR("AbilityContext::TerminateAbilityResult TerminateAbilityResult retval is %d", errval);
    }

    HILOG_DEBUG("%{public}s end.", __func__);
    return (errval == ERR_OK) ? true : false;
}

void AbilityContext::StartAbilities(const std::vector<AAFwk::Want> &wants)
{
    for (auto want : wants) {
        StartAbility(want, ABILITY_CONTEXT_DEFAULT_REQUEST_CODE);
    }
    HILOG_DEBUG("%{public}s end.", __func__);
}
}  // namespace AppExecFwk
}  // namespace OHOS
