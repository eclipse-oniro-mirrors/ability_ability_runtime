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

#ifndef UNITTEST_OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_COLLABORATOR_STUB_MOCK_H
#define UNITTEST_OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_COLLABORATOR_STUB_MOCK_H
#include <gmock/gmock.h>
#include "iability_manager_collaborator.h"
#include "iremote_object.h"
#include "iremote_stub.h"

namespace OHOS {
namespace AAFwk {
class AbilityManagerCollaboratorStubMock : public IRemoteStub<IAbilityManagerCollaborator> {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"IAbilityManagerCollaboratorMock");

    AbilityManagerCollaboratorStubMock() : code_(0)
    {}
    virtual ~AbilityManagerCollaboratorStubMock()
    {}

    int32_t NotifyStartAbility(const AppExecFwk::AbilityInfo &abilityInfo,
        int32_t userId, Want &want, uint64_t accessTokenIDEx, int32_t windowMode) override
    {
        return 0;
    }

    int32_t NotifyMissionCreated(int32_t missionId, const Want &want) override
    {
        return 0;
    }

    int32_t NotifyMissionCreated(const sptr<SessionInfo> &sessionInfo) override
    {
        return 0;
    }

    int32_t NotifyLoadAbility(
        const AppExecFwk::AbilityInfo &abilityInfo, int32_t missionId, const Want &want)
    {
        return 0;
    }

    int32_t NotifyLoadAbility(
        const AppExecFwk::AbilityInfo &abilityInfo, const sptr<SessionInfo> &sessionInfo)
    {
        return 0;
    }

    int32_t NotifyMoveMissionToBackground(int32_t missionId, int32_t userId, bool isFromScreenOffBackground)
    {
        return 0;
    }

    int32_t NotifyMoveMissionToForeground(int32_t missionId, int32_t userId)
    {
        return 0;
    }

    int32_t NotifyTerminateMission(int32_t missionId, int32_t userId)
    {
        return 0;
    }

    int32_t NotifyClearMission(int32_t missionId, int32_t userId)
    {
        return 0;
    }

    int32_t NotifyRemoveShellProcess(int32_t pid, int32_t type, const std::string &reason)
    {
        return 0;
    }

    void UpdateMissionInfo(sptr<SessionInfo> &sessionInfo)
    {}
    MOCK_METHOD4(SendRequest, int(uint32_t, MessageParcel&, MessageParcel&, MessageOption&));
    MOCK_METHOD3(OpenFile, int(const Uri& uri, uint32_t flag, uint32_t tokenId));
    MOCK_METHOD3(NotifyMissionBindPid, void(int32_t missionId, int32_t pid, int32_t userId));
    MOCK_METHOD2(CheckStaticCfgPermission, int32_t(const Want &want, bool isImplicit));
    MOCK_METHOD2(NotifyKillProcesses, int32_t(const std::string &, int32_t));

    int InvokeSendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
    {
        code_ = code;
        return 0;
    }

    int GetCode()
    {
        return code_;
    }

private:
    int code_ = 0;
};
} // namepace AAFwk
} // namespace OHOS
#endif // UNITTEST_OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_COLLABORATOR_STUB_MOCK_H