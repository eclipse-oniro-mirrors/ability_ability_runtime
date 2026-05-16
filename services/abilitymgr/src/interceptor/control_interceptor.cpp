/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "interceptor/control_interceptor.h"

#include "ability_util.h"
#include "appexecfwk_errors.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr const char* INTERCEPT_PARAMETERS = "intercept_parammeters";
constexpr const char* INTERCEPT_BUNDLE_NAME = "intercept_bundleName";
constexpr const char* INTERCEPT_ABILITY_NAME = "intercept_abilityName";
constexpr const char* INTERCEPT_MODULE_NAME = "intercept_moduleName";
constexpr const char* IS_FROM_PARENTCONTROL = "ohos.ability.isFromParentControl";
}

ErrCode ControlInterceptor::DoProcess(AbilityInterceptorParam param)
{
    AppExecFwk::AppRunningControlRuleResult controlRule;
    if (CheckControl(param.want, param.userId, controlRule)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR,
            "app is intercepted %{public}s", controlRule.controlMessage.c_str());
#ifdef SUPPORT_GRAPHICS
        if (!param.isWithUI || controlRule.controlWant == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "no control want");
            return AbilityUtil::EdmErrorType(controlRule.isEdm);
        }
        if (controlRule.controlWant->GetBoolParam(IS_FROM_PARENTCONTROL, false)) {
            auto controlWant = controlRule.controlWant;
            auto controlParam = controlWant->GetParams();
            sptr<AAFwk::IWantParams> interceptParam = WantParamWrapper::Box(param.want.GetParams());
            if (interceptParam != nullptr) {
                controlParam.SetParam(INTERCEPT_PARAMETERS, interceptParam);
            }
            controlWant->SetParams(controlParam);
            auto wantEle = param.want.GetElement();
            controlWant->SetParam(INTERCEPT_BUNDLE_NAME, wantEle.GetBundleName());
            controlWant->SetParam(INTERCEPT_ABILITY_NAME, wantEle.GetAbilityName());
            controlWant->SetParam(INTERCEPT_MODULE_NAME, wantEle.GetModuleName());
            controlRule.controlWant = controlWant;
        }
        int ret = IN_PROCESS_CALL(AbilityManagerClient::GetInstance()->StartAbility(*controlRule.controlWant,
            param.requestCode, param.userId));
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "ControlInterceptor start failed:%{public}d", ret);
            return ret;
        }
#endif
        return AbilityUtil::EdmErrorType(controlRule.isEdm);
    }
    return ERR_OK;
}

bool ControlInterceptor::CheckControl(const Want &want, int32_t userId,
    AppExecFwk::AppRunningControlRuleResult &controlRule)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    // get bms
    auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null bundleMgrHelper");
        return false;
    }

    // get disposed status
    std::string bundleName = want.GetBundle();
    auto appControlMgr = bundleMgrHelper->GetAppControlProxy();
    if (appControlMgr == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null appControlMgr");
        return false;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "GetAppRunningControlRule");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    if (identity == "") {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "getId fail");
    }
    auto ret = appControlMgr->GetAppRunningControlRule(bundleName, userId, controlRule);
    if (ret == ERR_BUNDLE_MANAGER_PERMISSION_DENIED) {
        IPCSkeleton::SetCallingIdentity(identity, true);
    } else {
        IPCSkeleton::SetCallingIdentity(identity);
    }
    if (ret != ERR_OK) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Get No AppRunningControlRule.");
        return false;
    }
    return true;
}
} // namespace AAFwk
} // namespace OHOS