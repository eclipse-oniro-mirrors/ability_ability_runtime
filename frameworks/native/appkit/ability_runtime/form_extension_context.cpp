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

#include "form_extension_context.h"

#include "ability_manager_client.h"
#include "connection_manager.h"
#include "appexecfwk_errors.h"
#include "form_mgr.h"
#include "form_mgr_errors.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace AbilityRuntime {
const size_t FormExtensionContext::CONTEXT_TYPE_ID(std::hash<const char*> {} ("FormExtensionContext"));

int FormExtensionContext::UpdateForm(const int64_t formId, const AppExecFwk::FormProviderData &formProviderData)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Update form, formId: %{public}" PRId64 ".", formId);
    if (formId <= 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "formId not be negative or zero");
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    // check fms recover status
    if (AppExecFwk::FormMgr::GetRecoverStatus() == AppExecFwk::Constants::IN_RECOVERING) {
        TAG_LOGE(AAFwkTag::APPKIT, "update failed");
        return ERR_APPEXECFWK_FORM_IN_RECOVER;
    }

    // check formProviderData
    if (formProviderData.GetDataString().empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "form data empty");
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    // update form request to fms
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "AppExecFwk::FormMgr::GetInstance().UpdateForm");
    return AppExecFwk::FormMgr::GetInstance().UpdateForm(formId, formProviderData);
}

int FormExtensionContext::UpdateForm(const int64_t formId, const AppExecFwk::FormProviderInfo &formProviderInfo)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Update form, formId: %{public}" PRId64 ".", formId);
    if (formId <= 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "formId not be negative or zero");
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }
    if (AppExecFwk::FormMgr::GetRecoverStatus() == AppExecFwk::Constants::IN_RECOVERING) {
        TAG_LOGE(AAFwkTag::APPKIT, "update failed");
        return ERR_APPEXECFWK_FORM_IN_RECOVER;
    }
    if (formProviderInfo.GetFormData().GetDataString().empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "form data empty");
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "AppExecFwk::FormMgr::GetInstance().UpdateForm");
    return AppExecFwk::FormMgr::GetInstance().UpdateForm(formId, formProviderInfo);
}

ErrCode FormExtensionContext::StartAbility(const AAFwk::Want &want) const
{
    TAG_LOGD(AAFwkTag::APPKIT, "Start ability");
    // route to FMS
    ErrCode err = AppExecFwk::FormMgr::GetInstance().StartAbility(want, token_);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed %{public}d", err);
    }
    return err;
}

AppExecFwk::AbilityType FormExtensionContext::GetAbilityInfoType() const
{
    std::shared_ptr<AppExecFwk::AbilityInfo> info = GetAbilityInfo();
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null info");
        return AppExecFwk::AbilityType::UNKNOWN;
    }

    return info->type;
}

std::shared_ptr<AppExecFwk::AbilityInfo> FormExtensionContext::GetAbilityInfo() const
{
    return abilityInfo_;
}

void FormExtensionContext::SetAbilityInfo(const std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> &abilityInfo)
{
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null abilityInfo");
        return;
    }
    abilityInfo_ = abilityInfo;
}

ErrCode FormExtensionContext::ConnectAbility(
    const AAFwk::Want &want, const sptr<AbilityConnectCallback> &connectCallback) const
{
    TAG_LOGI(AAFwkTag::APPKIT, "connect ability: %{public}s", want.GetElement().GetAbilityName().c_str());
    ErrCode ret =
        ConnectionManager::GetInstance().ConnectAbility(token_, want, connectCallback);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "errorCode = %{public}d", ret);
    }
    return ret;
}

ErrCode FormExtensionContext::DisconnectAbility(
    const AAFwk::Want &want, const sptr<AbilityConnectCallback> &connectCallback) const
{
    TAG_LOGI(AAFwkTag::APPKIT, "Call");
    ErrCode ret =
        ConnectionManager::GetInstance().DisconnectAbility(token_, want, connectCallback);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "ret=%{public}d", ret);
    }
    return ret;
}
} // namespace AbilityRuntime
} // namespace OHOS
