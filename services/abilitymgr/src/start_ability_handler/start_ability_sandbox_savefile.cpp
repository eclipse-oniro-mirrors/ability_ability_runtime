/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "start_ability_sandbox_savefile.h"

#include "hitrace_meter.h"
#include "ability_util.h"
#include "ability_manager_service.h"
#include "display_manager.h"
#include "display_util.h"

namespace OHOS {
namespace AAFwk {
namespace {
#ifdef WITH_DLP
const std::string DLP_BUNDLE_NAME = "com.ohos.dlpmanager";
const std::string DLP_ABILITY_NAME = "SaveAsAbility";
#endif // WITH_DLP

class EmptyConnection : public IRemoteStub<IAbilityConnection> {
public:
    void OnAbilityConnectDone(const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject, int resultCode) override
    {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "OnAbilityConnectDone");
    }
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override
    {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "OnAbilityDisconnectDone");
    }
};
}

StartAbilitySandboxSavefile &StartAbilitySandboxSavefile::GetInstance()
{
    static StartAbilitySandboxSavefile instance;
    return instance;
}

bool StartAbilitySandboxSavefile::MatchStartRequest(StartAbilityParams &params)
{
    if (params.IsCallerSandboxApp() && params.want.GetActionRef() == "ohos.want.action.CREATE_FILE" &&
        params.want.GetStringParam("startMode") == "save") {
        return true;
    }

    auto element = params.want.GetElement();
#ifdef WITH_DLP
    if (element.GetBundleName() == DLP_BUNDLE_NAME && element.GetAbilityName() == DLP_ABILITY_NAME &&
        !ContainRecord(params.requestCode)) {
        return true;
    }
#endif // WITH_DLP
    return false;
}

int StartAbilitySandboxSavefile::HandleStartRequest(StartAbilityParams &params)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    auto callerRecord = params.GetCallerRecord();
    if (!callerRecord) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null caller shouldn't happen");
        return CHECK_PERMISSION_FAILED;
    }

#ifdef WITH_DLP
    if (!params.SandboxExternalAuth()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "sandbox external auth failed");
        return CHECK_PERMISSION_FAILED;
    }
#endif // WITH_DLP

    auto reqCode = PushRecord(params.requestCode, callerRecord);
    auto &want = params.want;
#ifdef WITH_DLP
    want.SetElementName(DLP_BUNDLE_NAME, DLP_ABILITY_NAME);
#endif // WITH_DLP
    want.SetParam("requestCode", reqCode);
    want.SetParam("startMode", std::string("save_redirect"));

    return StartAbility(params, reqCode);
}

int StartAbilitySandboxSavefile::StartAbility(StartAbilityParams &params, int requestCode)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    AbilityRequest abilityRequest;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    abilityRequest.callerUid = IPCSkeleton::GetCallingUid();
    abilityRequest.callerToken = params.callerToken;
    abilityRequest.startSetting = nullptr;
    abilityRequest.want = params.want;
    abilityRequest.connect = sptr<IAbilityConnection>(new EmptyConnection());

    auto abilityMs = DelayedSingleton<AbilityManagerService>::GetInstance();
    auto ret = abilityMs->GenerateAbilityRequest(params.want, requestCode,
        abilityRequest, params.callerToken, params.GetValidUserId());
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "generate ability request error");
        return ret;
    }

    if (params.startOptions) {
        if (params.startOptions->GetDisplayID() == 0) {
            abilityRequest.want.SetParam(Want::PARAM_RESV_DISPLAY_ID,
                DisplayUtil::GetDefaultDisplayId());
        } else {
            abilityRequest.want.SetParam(Want::PARAM_RESV_DISPLAY_ID, params.startOptions->GetDisplayID());
        }
        abilityRequest.want.SetParam(Want::PARAM_RESV_WINDOW_MODE, params.startOptions->GetWindowMode());
    }

    return abilityMs->StartAbilityJust(abilityRequest, params.GetValidUserId());
}

int StartAbilitySandboxSavefile::PushRecord(int reqCode, const std::shared_ptr<AbilityRecord> &caller)
{
    std::lock_guard guard(recordsMutex_);
    requestCode_++;
    if (requestCode_ >= INT_MAX) {
        requestCode_ = 0;
    }

    auto it = fileSavingRecords_.find(requestCode_);
    if (it != fileSavingRecords_.end()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "repeated request code");
        fileSavingRecords_.erase(it);
    }

    SaveFileRecord record{reqCode, caller};
    fileSavingRecords_.emplace(requestCode_, record);
    return requestCode_;
}

bool StartAbilitySandboxSavefile::ContainRecord(int reqCode)
{
    std::lock_guard guard(recordsMutex_);
    return fileSavingRecords_.count(reqCode) > 0;
}

void StartAbilitySandboxSavefile::HandleResult(const Want &want, int resultCode, int requestCode)
{
    std::shared_ptr<AbilityRecord> callerRecord;
    int originReqCode = -1;
    {
        std::lock_guard guard(recordsMutex_);
        auto it = fileSavingRecords_.find(requestCode);
        if (it != fileSavingRecords_.end()) {
            callerRecord = it->second.caller.lock();
            originReqCode = it->second.originReqCode;
            fileSavingRecords_.erase(it);
        }
    }
    if (!callerRecord) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "request code not found: %{public}d.", requestCode);
        return;
    }
    callerRecord->SendSandboxSavefileResult(want, resultCode, originReqCode);
}
} // namespace AAFwk
} // namespace OHOS