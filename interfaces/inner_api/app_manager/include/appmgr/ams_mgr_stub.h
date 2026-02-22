/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_AMS_MGR_STUB_H
#define OHOS_ABILITY_RUNTIME_AMS_MGR_STUB_H

#include <map>

#include "iremote_stub.h"
#include "nocopyable.h"
#include "string_ex.h"
#include "ams_mgr_interface.h"

namespace OHOS {
namespace AppExecFwk {
class AmsMgrStub : public IRemoteStub<IAmsMgr> {
public:
    AmsMgrStub();
    virtual ~AmsMgrStub();

    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

    /**
     * UpdateExtensionState, call UpdateExtensionState() through the proxy object, update the extension status.
     *
     * @param token, the unique identification to update the extension.
     * @param state, extension status that needs to be updated.
     * @return
     */
    virtual void UpdateExtensionState(const sptr<IRemoteObject> &token, const ExtensionState state) override;
private:
    void CreateMemberFuncMap();
    int32_t HandleLoadAbility(MessageParcel &data, MessageParcel &reply);
    int32_t HandleNotifyLoadAbilityFinished(MessageParcel &data, MessageParcel &reply);
    int32_t HandleTerminateAbility(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUpdateAbilityState(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUpdateExtensionState(MessageParcel &data, MessageParcel &reply);
    int32_t HandleRegisterAppStateCallback(MessageParcel &data, MessageParcel &reply);
    int32_t HandleReset(MessageParcel &data, MessageParcel &reply);
    int32_t HandleKillProcessByAbilityToken(MessageParcel &data, MessageParcel &reply);
    int32_t HandleKillProcessesByUserId(MessageParcel &data, MessageParcel &reply);
    int32_t HandleKillProcessesByPids(MessageParcel &data, MessageParcel &reply);
    int32_t HandleAttachPidToParent(MessageParcel &data, MessageParcel &reply);
    int32_t HandleKillProcessWithAccount(MessageParcel &data, MessageParcel &reply);
    int32_t HandleKillProcessesInBatch(MessageParcel &data, MessageParcel &reply);
    int32_t HandleKillApplication(MessageParcel &data, MessageParcel &reply);
    int32_t HandleForceKillApplication(MessageParcel &data, MessageParcel &reply);
    int32_t HandleKillProcessesByAccessTokenId(MessageParcel &data, MessageParcel &reply);
    int32_t HandleAbilityAttachTimeOut(MessageParcel &data, MessageParcel &reply);
    int32_t HandlePrepareTerminate(MessageParcel &data, MessageParcel &reply);
    int32_t HandleKillApplicationByUid(MessageParcel &data, MessageParcel &reply);
    int32_t HandleKillApplicationSelf(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetRunningProcessInfoByToken(MessageParcel &data, MessageParcel &reply);
    int32_t HandleSetAbilityForegroundingFlagToAppRecord(MessageParcel &data, MessageParcel &reply);
    int32_t HandlePrepareTerminateApp(MessageParcel &data, MessageParcel &reply);
    int32_t HandleStartSpecifiedAbility(MessageParcel &data, MessageParcel &reply);
    int32_t HandleRegisterStartSpecifiedAbilityResponse(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetApplicationInfoByProcessID(MessageParcel &data, MessageParcel &reply);
    int32_t HandleNotifyAppMgrRecordExitReason(MessageParcel &data, MessageParcel &reply);
    int32_t HandleNotifyAppMgrRecordExitReasonCompability(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUpdateApplicationInfoInstalled(MessageParcel &data, MessageParcel &reply);
    int32_t HandleSetEnableStartProcessFlagByUserId(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetBundleNameByPid(MessageParcel &data, MessageParcel &reply);
    int32_t HandleRegisterAppDebugListener(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUnregisterAppDebugListener(MessageParcel &data, MessageParcel &reply);
    int32_t HandleAttachAppDebug(MessageParcel &data, MessageParcel &reply);
    int32_t HandleDetachAppDebug(MessageParcel &data, MessageParcel &reply);
    int32_t HandleSetAppWaitingDebug(MessageParcel &data, MessageParcel &reply);
    int32_t HandleCancelAppWaitingDebug(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetWaitingDebugApp(MessageParcel &data, MessageParcel &reply);
    int32_t HandleIsWaitingDebugApp(MessageParcel &data, MessageParcel &reply);
    int32_t HandleClearNonPersistWaitingDebugFlag(MessageParcel &data, MessageParcel &reply);
    int32_t HandleRegisterAbilityDebugResponse(MessageParcel &data, MessageParcel &reply);
    int32_t HandleIsAttachDebug(MessageParcel &data, MessageParcel &reply);
    int32_t HandleClearProcessByToken(MessageParcel &data, MessageParcel &reply);
    int32_t HandleIsMemorySizeSufficient(MessageParcel &data, MessageParcel &reply);
    int32_t HandleIsNoRequireBigMemory(MessageParcel &data, MessageParcel &reply);
    int32_t HandleSetKeepAliveEnableState(MessageParcel &data, MessageParcel &reply);
    int32_t HandleSetKeepAliveDkv(MessageParcel &data, MessageParcel &reply);
    int32_t HandleSetKeepAliveAppService(MessageParcel &data, MessageParcel &reply);
    int32_t HandleAttachedToStatusBar(MessageParcel &data, MessageParcel &reply);
    int32_t OnRemoteRequestInner(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    int32_t OnRemoteRequestInnerFirst(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    int32_t OnRemoteRequestInnerSecond(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    int32_t OnRemoteRequestInnerThird(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    int32_t OnRemoteRequestInnerFourth(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    int32_t HandleBlockProcessCacheByPids(MessageParcel &data, MessageParcel &reply);
    int32_t HandleIsKilledForUpgradeWeb(MessageParcel &data, MessageParcel &reply);
    int32_t HandleCleanAbilityByUserRequest(MessageParcel &data, MessageParcel &reply);
    int32_t HandleIsProcessContainsOnlyUIAbility(MessageParcel &data, MessageParcel &reply);
    int32_t HandleIsProcessAttached(MessageParcel &data, MessageParcel &reply);
    int32_t HandleIsCallerKilling(MessageParcel &data, MessageParcel &reply);
    int32_t HandleNotifyUninstallOrUpgradeApp(MessageParcel &data, MessageParcel &reply);
    int32_t HandleNotifyUninstallOrUpgradeAppEnd(MessageParcel &data, MessageParcel &reply);
    int32_t HandlePreloadApplicationByPhase(MessageParcel &data, MessageParcel &reply);
    int32_t HandleNotifyPreloadAbilityStateChanged(MessageParcel &data, MessageParcel &reply);
    int32_t HandleCheckPreloadAppRecordExist(MessageParcel &data, MessageParcel &reply);
    int32_t HandleVerifyKillProcessPermission(MessageParcel &data, MessageParcel &reply);
    DISALLOW_COPY_AND_MOVE(AmsMgrStub);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_AMS_MGR_STUB_H
