/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

#include "hilog_tag_wrapper.h"
#include "mock_ability_delegator_stub.h"

namespace OHOS {
namespace AAFwk {
namespace {
const std::string STRING_BUNDLENAME_EX = "com.example.myapplication1";
}

bool MockAbilityDelegatorStub::finishFlag_ = false;

MockAbilityDelegatorStub::MockAbilityDelegatorStub()
{}

int MockAbilityDelegatorStub::StartAbility(
    const Want& want, int32_t userId, int requestCode, uint64_t specifiedFullTokenId)
{
    return 0;
}

int MockAbilityDelegatorStub::CloseAbility(const sptr<IRemoteObject>& token, int resultCode, const Want* resultWant)
{
    return 0;
}

int MockAbilityDelegatorStub::StartAbility(const Want& want, const StartOptions& startOptions,
    const sptr<IRemoteObject>& callerToken, int requestCode, int32_t userId)
{
    return 0;
}

int MockAbilityDelegatorStub::StartAbilityAsCaller(const Want& want, const StartOptions& startOptions,
    const sptr<IRemoteObject>& callerToken, sptr<IRemoteObject> asCallerSourceToken,
    int requestCode, int32_t userId)
{
    return 0;
}

int MockAbilityDelegatorStub::MinimizeAbility(const sptr<IRemoteObject>& token, bool fromUser)
{
    return 0;
}

void MockAbilityDelegatorStub::DumpState(const std::string& args, std::vector<std::string>& state)
{
    return;
}

int MockAbilityDelegatorStub::StartUser(int userId, uint64_t displayId, sptr<IUserCallback> callback,
    bool isAppRecovery)
{
    return 0;
}

int MockAbilityDelegatorStub::StopUser(int userId, const sptr<IUserCallback>& callback)
{
    return 0;
}

int MockAbilityDelegatorStub::LogoutUser(int32_t userId, sptr<IUserCallback> callback)
{
    return 0;
}

int MockAbilityDelegatorStub::StartSyncRemoteMissions(const std::string& devId, bool fixConflict, int64_t tag)
{
    return 0;
}

int MockAbilityDelegatorStub::StopSyncRemoteMissions(const std::string& devId)
{
    return 0;
}

int MockAbilityDelegatorStub::RegisterMissionListener(const std::string& deviceId,
    const sptr<IRemoteMissionListener>& listener)
{
    return 0;
}

int MockAbilityDelegatorStub::UnRegisterMissionListener(const std::string& deviceId,
    const sptr<IRemoteMissionListener>& listener)
{
    return 0;
}

int MockAbilityDelegatorStub::StartAbilityByCall(const Want& want, const sptr<IAbilityConnection>& connect,
    const sptr<IRemoteObject>& callerToken, int32_t accountId,
    bool isSilent, bool promotePriority, bool isVisible, uint64_t specifiedFullTokenId)
{
    return 0;
}

int MockAbilityDelegatorStub::StartAbilityForPrelaunch(const Want& want, const int32_t frameNum)
{
    return 0;
}

void MockAbilityDelegatorStub::CallRequestDone(
    const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &callStub)
{
    return;
}

int MockAbilityDelegatorStub::ReleaseCall(const sptr<IAbilityConnection>& connect,
    const AppExecFwk::ElementName& element)
{
    return 0;
}

int MockAbilityDelegatorStub::GetMissionSnapshot(const std::string& deviceId, int32_t missionId,
    MissionSnapshot& snapshot, bool isLowResolution)
{
    return 0;
}

int MockAbilityDelegatorStub::RegisterSnapshotHandler(const sptr<ISnapshotHandler>& handler)
{
    return 0;
}

int32_t MockAbilityDelegatorStub::ReportDrawnCompleted(const sptr<IRemoteObject>& callerToken)
{
    return 0;
}

int MockAbilityDelegatorStub::SetAbilityController(const sptr<AppExecFwk::IAbilityController>& abilityController,
    bool imAStabilityTest)
{
    return 0;
}

bool MockAbilityDelegatorStub::IsRunningInStabilityTest()
{
    return true;
}

void MockAbilityDelegatorStub::DumpSysState(
    const std::string& args, std::vector<std::string>& info, bool isClient, bool isUserID, int UserID)
{
    return;
}

int MockAbilityDelegatorStub::StartUserTest(const Want& want, const sptr<IRemoteObject>& observer)
{
    if (want.GetStringParam("-p") == STRING_BUNDLENAME_EX) {
        return ERR_INVALID_VALUE;
    }
    return OHOS::ERR_OK;
}

int MockAbilityDelegatorStub::StopServiceAbility(const Want& want, int32_t userId, const sptr<IRemoteObject> &token)
{
    return 0;
}

int MockAbilityDelegatorStub::GetTopAbility(sptr<IRemoteObject>& token)
{
    TAG_LOGI(AAFwkTag::TEST, "MockAbilityDelegatorStub::GetTopAbility is called");
    token = sptr<IRemoteObject>(new MockAbilityDelegatorStub);
    return OHOS::ERR_OK;
}

int MockAbilityDelegatorStub::DelegatorDoAbilityForeground(const sptr<IRemoteObject>& token)
{
    TAG_LOGI(AAFwkTag::TEST, "MockAbilityDelegatorStub::DelegatorDoAbilityForeground is called");
    return OHOS::ERR_OK;
}

int MockAbilityDelegatorStub::DelegatorDoAbilityBackground(const sptr<IRemoteObject>& token)
{
    TAG_LOGI(AAFwkTag::TEST, "MockAbilityDelegatorStub::DelegatorDoAbilityBackground is called");
    return OHOS::ERR_OK;
}

int MockAbilityDelegatorStub::FinishUserTest(const std::string& msg,
    const int64_t& resultCode, const std::string& bundleName)
{
    TAG_LOGI(AAFwkTag::TEST, "MockAbilityDelegatorStub::FinishUserTest is called");
    finishFlag_ = true;
    return OHOS::ERR_OK;
}

int MockAbilityDelegatorStub::DoAbilityForeground(const sptr<IRemoteObject>& token, uint32_t flag)
{
    return 0;
}

int MockAbilityDelegatorStub::DoAbilityBackground(const sptr<IRemoteObject>& token, uint32_t flag)
{
    return 0;
}

int32_t MockAbilityDelegatorStub::GetMissionIdByToken(const sptr<IRemoteObject>& token)
{
    return 0;
}

void MockAbilityDelegatorStub::GetAbilityTokenByCalleeObj(const sptr<IRemoteObject> &callStub,
    sptr<IRemoteObject> &token)
{
    return;
}

bool MockAbilityDelegatorStub2::finishFlag_ = false;
TESTCASE_BRANCH MockAbilityDelegatorStub2::testcaseBranch_ = TESTCASE_BRANCH::BRANCH_1;

MockAbilityDelegatorStub2::MockAbilityDelegatorStub2()
{}

int MockAbilityDelegatorStub2::StartAbility(
    const Want& want, int32_t userId, int requestCode, uint64_t specifiedFullTokenId)
{
    return 0;
}

int MockAbilityDelegatorStub2::StartAbility(const Want& want, const StartOptions& startOptions,
    const sptr<IRemoteObject>& callerToken, int requestCode, int32_t userId)
{
    return 0;
}

int MockAbilityDelegatorStub2::StartAbilityAsCaller(const Want& want, const StartOptions& startOptions,
    const sptr<IRemoteObject>& callerToken, sptr<IRemoteObject> asCallerSourceToken,
    int requestCode, int32_t userId)
{
    return 0;
}

int MockAbilityDelegatorStub2::CloseAbility(const sptr<IRemoteObject>& token, int resultCode, const Want* resultWant)
{
    return 0;
}

int MockAbilityDelegatorStub2::MinimizeAbility(const sptr<IRemoteObject>& token, bool fromUser)
{
    return 0;
}

void MockAbilityDelegatorStub2::DumpState(const std::string& args, std::vector<std::string>& state)
{
    return;
}

int MockAbilityDelegatorStub2::StopServiceAbility(const Want& want, int32_t userId, const sptr<IRemoteObject> &token)
{
    return 0;
}

int MockAbilityDelegatorStub2::StartUser(int userId, uint64_t displayId, sptr<IUserCallback> callback,
    bool isAppRecovery)
{
    return 0;
}

int MockAbilityDelegatorStub2::StopUser(int userId, const sptr<IUserCallback>& callback)
{
    return 0;
}

int MockAbilityDelegatorStub2::LogoutUser(int32_t userId, sptr<IUserCallback> callback)
{
    return 0;
}

int MockAbilityDelegatorStub2::StartSyncRemoteMissions(const std::string& devId, bool fixConflict, int64_t tag)
{
    return 0;
}

int MockAbilityDelegatorStub2::StopSyncRemoteMissions(const std::string& devId)
{
    return 0;
}

int MockAbilityDelegatorStub2::RegisterMissionListener(const std::string& deviceId,
    const sptr<IRemoteMissionListener>& listener)
{
    return 0;
}

int MockAbilityDelegatorStub2::UnRegisterMissionListener(const std::string& deviceId,
    const sptr<IRemoteMissionListener>& listener)
{
    return 0;
}

int MockAbilityDelegatorStub2::StartAbilityByCall(const Want& want, const sptr<IAbilityConnection>& connect,
    const sptr<IRemoteObject>& callerToken, int32_t accountId,
    bool isSilent, bool promotePriority, bool isVisible, uint64_t specifiedFullTokenId)
{
    return 0;
}

int MockAbilityDelegatorStub2::StartAbilityForPrelaunch(const Want& want, const int32_t frameNum)
{
    return 0;
}

void MockAbilityDelegatorStub2::CallRequestDone(
    const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &callStub)
{
    return;
}

int MockAbilityDelegatorStub2::ReleaseCall(const sptr<IAbilityConnection>& connect,
    const AppExecFwk::ElementName& element)
{
    return 0;
}

int MockAbilityDelegatorStub2::GetMissionSnapshot(const std::string& deviceId, int32_t missionId,
    MissionSnapshot& snapshot, bool isLowResolution)
{
    return 0;
}

int MockAbilityDelegatorStub2::RegisterSnapshotHandler(const sptr<ISnapshotHandler>& handler)
{
    return 0;
}

int MockAbilityDelegatorStub2::SetAbilityController(const sptr<AppExecFwk::IAbilityController>& abilityController,
    bool imAStabilityTest)
{
    return 0;
}

bool MockAbilityDelegatorStub2::IsRunningInStabilityTest()
{
    return true;
}

void MockAbilityDelegatorStub2::DumpSysState(
    const std::string& args, std::vector<std::string>& info, bool isClient, bool isUserID, int UserID)
{
    return;
}

int MockAbilityDelegatorStub2::StartUserTest(const Want& want, const sptr<IRemoteObject>& observer)
{
    if (want.GetStringParam("-p") == STRING_BUNDLENAME_EX) {
        return ERR_INVALID_VALUE;
    }
    return OHOS::ERR_OK;
}

int MockAbilityDelegatorStub2::GetTopAbility(sptr<IRemoteObject>& token)
{
    TAG_LOGI(AAFwkTag::TEST, "MockAbilityDelegatorStub2::GetTopAbility is called");
    return OHOS::ERR_INVALID_VALUE;
}

AppExecFwk::ElementName MockAbilityDelegatorStub2::GetTopAbility(bool isNeedLocalDeviceId)
{
    TAG_LOGI(AAFwkTag::TEST, "GetTopAbility.");
    AppExecFwk::ElementName elementName = {};

    if (testcaseBranch_ == TESTCASE_BRANCH::BRANCH_2) {
        elementName = AppExecFwk::ElementName("deviceId", "com.ohos.bundleName", "EntryAbility", "entry");
    } else if (testcaseBranch_ == TESTCASE_BRANCH::BRANCH_3) {
        elementName = AppExecFwk::ElementName("deviceId", "com.example.myapplication",
            "com.example.myapplication.MainAbility", "entry");
    } else {
        TAG_LOGI(AAFwkTag::TEST, "Empty top ability.");
    }

    return elementName;
}

int MockAbilityDelegatorStub2::DelegatorDoAbilityForeground(const sptr<IRemoteObject>& token)
{
    TAG_LOGI(AAFwkTag::TEST, "MockAbilityDelegatorStub2::DelegatorDoAbilityForeground is called");
    return OHOS::ERR_INVALID_VALUE;
}

int MockAbilityDelegatorStub2::DelegatorDoAbilityBackground(const sptr<IRemoteObject>& token)
{
    TAG_LOGI(AAFwkTag::TEST, "MockAbilityDelegatorStub2::DelegatorDoAbilityBackground is called");
    return OHOS::ERR_INVALID_VALUE;
}

int MockAbilityDelegatorStub2::FinishUserTest(const std::string& msg,
    const int64_t& resultCode, const std::string& bundleName)
{
    TAG_LOGI(AAFwkTag::TEST, "MockAbilityDelegatorStub2::FinishUserTest is called");
    finishFlag_ = false;
    return OHOS::ERR_INVALID_VALUE;
}

int MockAbilityDelegatorStub2::DoAbilityForeground(const sptr<IRemoteObject>& token, uint32_t flag)
{
    return 0;
}

int MockAbilityDelegatorStub2::DoAbilityBackground(const sptr<IRemoteObject>& token, uint32_t flag)
{
    return 0;
}

int32_t MockAbilityDelegatorStub2::GetMissionIdByToken(const sptr<IRemoteObject>& token)
{
    return 0;
}

void MockAbilityDelegatorStub2::GetAbilityTokenByCalleeObj(const sptr<IRemoteObject> &callStub,
    sptr<IRemoteObject> &token)
{
    return;
}

int32_t MockAbilityDelegatorStub2::ReportDrawnCompleted(const sptr<IRemoteObject>& callerToken)
{
    return 0;
}

#ifdef ABILITY_COMMAND_FOR_TEST
int MockAbilityDelegatorStub::ForceTimeoutForTest(const std::string& abilityName, const std::string& state)
{
    return 0;
}

int MockAbilityDelegatorStub2::ForceTimeoutForTest(const std::string& abilityName, const std::string& state)
{
    return 0;
}
#endif

#ifdef SUPPORT_GRAPHICS
int MockAbilityDelegatorStub::SetMissionIcon(
    const sptr<IRemoteObject>& token, const std::shared_ptr<OHOS::Media::PixelMap>& icon)
{
    return 0;
}

int MockAbilityDelegatorStub::RegisterWindowManagerServiceHandler(const sptr<IWindowManagerServiceHandler>& handler,
    bool animationEnabled)
{
    return 0;
}

int MockAbilityDelegatorStub2::SetMissionIcon(
    const sptr<IRemoteObject>& token, const std::shared_ptr<OHOS::Media::PixelMap>& icon)
{
    return 0;
}

int MockAbilityDelegatorStub2::RegisterWindowManagerServiceHandler(const sptr<IWindowManagerServiceHandler>& handler,
    bool animationEnabled)
{
    return 0;
}
#endif
}  // namespace AAFwk
}  // namespace OHOS
