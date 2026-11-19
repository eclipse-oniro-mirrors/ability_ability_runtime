/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "abilityschedulerstubremaining_fuzzer.h"
#include "ability_scheduler_stub.h"
#include "attack_vectors.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "data_ability_operation.h"
#include "data_ability_predicates.h"
#include "element_name.h"
#include "fuzz_util.h"
#include "message_parcel.h"
#include "pac_map.h"
#include "securec.h"
#include "session_info.h"
#include "values_bucket.h"
#include "want.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {
namespace {
constexpr int HANDLE_COUNT = 29;
constexpr int FUZZ_BATCH_MAX = 4;
}

class AbilitySchedulerStubRemainingFuzz : public AbilitySchedulerStub {
public:
    AbilitySchedulerStubRemainingFuzz() = default;
    ~AbilitySchedulerStubRemainingFuzz() = default;

    bool ScheduleAbilityTransaction(const Want &want, const LifeCycleStateInfo &targetState,
        sptr<SessionInfo> sessionInfo = nullptr) override
    {
        return true;
    }
    void ScheduleShareData(const int32_t &uniqueId) override {}
    void SendResult(int requestCode, int resultCode, const Want &resultWant) override {}
    void ScheduleConnectAbility(const Want &want) override {}
    void ScheduleDisconnectAbility(const Want &want) override {}
    void ScheduleCommandAbility(const Want &want, bool restart, int startId) override {}
    void ScheduleCommandAbilityWindow(const Want &want, const sptr<SessionInfo> &sessionInfo,
        WindowCommand winCmd) override {}
    bool SchedulePrepareTerminateAbility() override { return false; }
    void ScheduleSaveAbilityState() override {}
    void ScheduleRestoreAbilityState(const PacMap &inState) override {}
    std::vector<std::string> GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter) override
    {
        return {};
    }
    int OpenFile(const Uri &uri, const std::string &mode) override { return 0; }
    int OpenRawFile(const Uri &uri, const std::string &mode) override { return 0; }
    int Insert(const Uri &uri, const NativeRdb::ValuesBucket &value) override { return 0; }
    int Update(const Uri &uri, const NativeRdb::ValuesBucket &value,
        const NativeRdb::DataAbilityPredicates &predicates) override
    {
        return 0;
    }
    int Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates) override { return 0; }
    std::shared_ptr<AppExecFwk::PacMap> Call(const Uri &uri, const std::string &method,
        const std::string &arg, const AppExecFwk::PacMap &pacMap) override
    {
        return {};
    }
    std::shared_ptr<NativeRdb::AbsSharedResultSet> Query(const Uri &uri, std::vector<std::string> &columns,
        const NativeRdb::DataAbilityPredicates &predicates) override
    {
        return {};
    }
    std::string GetType(const Uri &uri) override { return {}; }
    bool Reload(const Uri &uri, const PacMap &extras) override { return true; }
    int BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values) override { return 0; }
    bool ScheduleRegisterObserver(const Uri &uri, const sptr<IDataAbilityObserver> &dataObserver) override
    {
        return true;
    }
    bool ScheduleUnregisterObserver(const Uri &uri, const sptr<IDataAbilityObserver> &dataObserver) override
    {
        return true;
    }
    bool ScheduleNotifyChange(const Uri &uri) override { return true; }
    Uri NormalizeUri(const Uri &uri) override { return Uri{""}; }
    Uri DenormalizeUri(const Uri &uri) override { return Uri{""}; }
    std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>> ExecuteBatch(
        const std::vector<std::shared_ptr<AppExecFwk::DataAbilityOperation>> &operations) override
    {
        return {};
    }
    void ContinueAbility(const std::string &deviceId, uint32_t versionCode) override {}
    void NotifyContinuationResult(int32_t result) override {}
    void DumpAbilityInfo(const std::vector<std::string> &params, std::vector<std::string> &info) override {}
    void UpdateSessionToken(sptr<IRemoteObject> sessionToken) override {}
    void OnExecuteIntent(const Want &want) override {}
    int CreateModalUIExtension(const Want &want) override { return 0; }
    void CallRequest() override {}
    void ScheduleCollaborate(const Want &want) override {}
    void ScheduleAbilityRequestFailure(const std::string &requestId, const AppExecFwk::ElementName &element,
        const std::string &message, int32_t resultCode) override {}
    void ScheduleAbilityRequestSuccess(const std::string &requestId,
        const AppExecFwk::ElementName &element) override {}
    void ScheduleAbilitiesRequestDone(const std::string &requestKey, int32_t resultCode) override {}
};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % HANDLE_COUNT) {
        case 0: {
            actualCode = static_cast<uint32_t>(IAbilityScheduler::SCHEDULE_ABILITY_COMMAND_WINDOW);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            SessionInfo sessionInfo;
            parcel.WriteParcelable(&sessionInfo);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            break;
        }
        case 1: {
            actualCode = static_cast<uint32_t>(IAbilityScheduler::SCHEDULE_SAVE_ABILITY_STATE);
            OHOS::FuzzUtil::WriteAppSpawnMsgOob(parcel, fdp);
            break;
        }
        case 2: {
            actualCode = static_cast<uint32_t>(IAbilityScheduler::SCHEDULE_RESTORE_ABILITY_STATE);
            PacMap pacMap;
            parcel.WriteParcelable(&pacMap);
            OHOS::FuzzUtil::WriteMmapCorruption(parcel, fdp);
            break;
        }
        case 3: {
            actualCode = static_cast<uint32_t>(IAbilityScheduler::SCHEDULE_INSERT);
            Uri uri(FuzzUtil::BuildUriAttackString(fdp));
            parcel.WriteParcelable(&uri);
            NativeRdb::ValuesBucket valuesBucket;
            valuesBucket.PutString("fuzz_key", FuzzUtil::BuildSpecialCharString(fdp));
            valuesBucket.Marshalling(parcel);
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            break;
        }
        case 4: {
            actualCode = static_cast<uint32_t>(IAbilityScheduler::SCHEDULE_UPDATE);
            Uri uri(FuzzUtil::BuildUriAttackString(fdp));
            parcel.WriteParcelable(&uri);
            NativeRdb::ValuesBucket valuesBucket;
            valuesBucket.PutString("fuzz_key", FuzzUtil::BuildSpecialCharString(fdp));
            valuesBucket.Marshalling(parcel);
            NativeRdb::DataAbilityPredicates predicates;
            parcel.WriteParcelable(&predicates);
            OHOS::FuzzUtil::WriteSemanticPrivilegeEscalation(parcel, fdp);
            break;
        }
        case 5: {
            actualCode = static_cast<uint32_t>(IAbilityScheduler::SCHEDULE_DELETE);
            Uri uri(FuzzUtil::BuildUriAttackString(fdp));
            parcel.WriteParcelable(&uri);
            NativeRdb::DataAbilityPredicates predicates;
            parcel.WriteParcelable(&predicates);
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            break;
        }
        case 6: {
            actualCode = static_cast<uint32_t>(IAbilityScheduler::SCHEDULE_QUERY);
            Uri uri(FuzzUtil::BuildUriAttackString(fdp));
            parcel.WriteParcelable(&uri);
            parcel.WriteStringVector(FuzzUtil::BuildStringVector(fdp));
            NativeRdb::DataAbilityPredicates predicates;
            parcel.WriteParcelable(&predicates);
            OHOS::FuzzUtil::WriteMmapCorruption(parcel, fdp);
            break;
        }
        case 7: {
            actualCode = static_cast<uint32_t>(IAbilityScheduler::SCHEDULE_RELOAD);
            Uri uri(FuzzUtil::BuildUriAttackString(fdp));
            parcel.WriteParcelable(&uri);
            PacMap pacMap;
            parcel.WriteParcelable(&pacMap);
            OHOS::FuzzUtil::WriteDeepNestedParcel(parcel, fdp);
            break;
        }
        case 8: {
            actualCode = static_cast<uint32_t>(IAbilityScheduler::SCHEDULE_BATCHINSERT);
            Uri uri(FuzzUtil::BuildUriAttackString(fdp));
            parcel.WriteParcelable(&uri);
            int32_t count = static_cast<int32_t>(fdp.ConsumeIntegral<uint8_t>() % FUZZ_BATCH_MAX);
            parcel.WriteInt32(count);
            for (int32_t i = 0; i < count; i++) {
                NativeRdb::ValuesBucket valuesBucket;
                valuesBucket.PutString("fuzz_key", FuzzUtil::BuildSpecialCharString(fdp));
                valuesBucket.Marshalling(parcel);
            }
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            break;
        }
        case 9: {
            actualCode = static_cast<uint32_t>(IAbilityScheduler::SCHEDULE_REGISTEROBSERVER);
            Uri uri(FuzzUtil::BuildUriAttackString(fdp));
            parcel.WriteParcelable(&uri);
            parcel.WriteRemoteObject(nullptr);
            OHOS::FuzzUtil::WriteSemanticPrivilegeEscalation(parcel, fdp);
            break;
        }
        case 10: {
            actualCode = static_cast<uint32_t>(IAbilityScheduler::SCHEDULE_UNREGISTEROBSERVER);
            Uri uri(FuzzUtil::BuildUriAttackString(fdp));
            parcel.WriteParcelable(&uri);
            parcel.WriteRemoteObject(nullptr);
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            break;
        }
        case 11: {
            actualCode = static_cast<uint32_t>(IAbilityScheduler::SCHEDULE_NOTIFYCHANGE);
            Uri uri(FuzzUtil::BuildUriAttackString(fdp));
            parcel.WriteParcelable(&uri);
            OHOS::FuzzUtil::WriteMmapCorruption(parcel, fdp);
            break;
        }
        case 12: {
            actualCode = static_cast<uint32_t>(IAbilityScheduler::SCHEDULE_NORMALIZEURI);
            Uri uri(FuzzUtil::BuildUriAttackString(fdp));
            parcel.WriteParcelable(&uri);
            OHOS::FuzzUtil::WriteDumpStateInfoLeak(parcel, fdp);
            break;
        }
        case 13: {
            actualCode = static_cast<uint32_t>(IAbilityScheduler::SCHEDULE_DENORMALIZEURI);
            Uri uri(FuzzUtil::BuildUriAttackString(fdp));
            parcel.WriteParcelable(&uri);
            OHOS::FuzzUtil::WriteSaAutoTrustBypass(parcel, fdp);
            break;
        }
        case 14: {
            actualCode = static_cast<uint32_t>(IAbilityScheduler::SCHEDULE_EXECUTEBATCH);
            int32_t count = static_cast<int32_t>(fdp.ConsumeIntegral<uint8_t>() % FUZZ_BATCH_MAX);
            parcel.WriteInt32(count);
            for (int32_t i = 0; i < count; i++) {
                AppExecFwk::DataAbilityOperation operation;
                parcel.WriteParcelable(&operation);
            }
            OHOS::FuzzUtil::WriteDeepNestedParcel(parcel, fdp);
            break;
        }
        case 15: {
            actualCode = static_cast<uint32_t>(IAbilityScheduler::NOTIFY_CONTINUATION_RESULT);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            OHOS::FuzzUtil::WriteAppSpawnMsgOob(parcel, fdp);
            break;
        }
        case 16: {
            actualCode = static_cast<uint32_t>(IAbilityScheduler::REQUEST_CALL_REMOTE);
            OHOS::FuzzUtil::WriteHugeRawDataDoS(parcel, fdp);
            break;
        }
        case 17: {
            actualCode = static_cast<uint32_t>(IAbilityScheduler::CONTINUE_ABILITY);
            parcel.WriteString(FuzzUtil::BuildSandboxEscapePath(fdp));
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            break;
        }
        case 18: {
            actualCode = static_cast<uint32_t>(IAbilityScheduler::DUMP_ABILITY_RUNNER_INNER);
            parcel.WriteStringVector(FuzzUtil::BuildStringVector(fdp));
            OHOS::FuzzUtil::WriteDumpStateInfoLeak(parcel, fdp);
            break;
        }
        case 19: {
            actualCode = static_cast<uint32_t>(IAbilityScheduler::SCHEDULE_CALL);
            Uri uri(FuzzUtil::BuildUriAttackString(fdp));
            parcel.WriteParcelable(&uri);
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            parcel.WriteString(FuzzUtil::BuildSqlInjectionString(fdp));
            PacMap pacMap;
            parcel.WriteParcelable(&pacMap);
            OHOS::FuzzUtil::WriteMmapCorruption(parcel, fdp);
            break;
        }
        case 20: {
            actualCode = static_cast<uint32_t>(IAbilityScheduler::SCHEDULE_SHARE_DATA);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            OHOS::FuzzUtil::WriteAppSpawnMsgOob(parcel, fdp);
            break;
        }
        case 21: {
            actualCode = static_cast<uint32_t>(IAbilityScheduler::SCHEDULE_ONEXECUTE_INTENT);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            OHOS::FuzzUtil::WriteDeepNestedParcel(parcel, fdp);
            break;
        }
        case 22: {
            actualCode = static_cast<uint32_t>(IAbilityScheduler::SCHEDULE_EXECUTE_SKILL);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            break;
        }
        case 23: {
            actualCode = static_cast<uint32_t>(IAbilityScheduler::CREATE_MODAL_UI_EXTENSION);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            OHOS::FuzzUtil::WriteSemanticPrivilegeEscalation(parcel, fdp);
            break;
        }
        case 24: {
            actualCode = static_cast<uint32_t>(IAbilityScheduler::UPDATE_SESSION_TOKEN);
            parcel.WriteRemoteObject(nullptr);
            OHOS::FuzzUtil::WriteSaAutoTrustBypass(parcel, fdp);
            break;
        }
        case 25: {
            actualCode = static_cast<uint32_t>(IAbilityScheduler::SCHEDULE_COLLABORATE_DATA);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            OHOS::FuzzUtil::WriteDeepNestedParcel(parcel, fdp);
            break;
        }
        case 26: {
            actualCode = static_cast<uint32_t>(IAbilityScheduler::SCHEDULE_ABILITY_REQUEST_FAILURE);
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            AppExecFwk::ElementName element;
            element.SetBundleName(FuzzUtil::BuildMaliciousBundleName(fdp));
            element.SetAbilityName(FuzzUtil::BuildSpecialCharString(fdp));
            element.SetDeviceID(FuzzUtil::BuildSandboxEscapePath(fdp));
            parcel.WriteParcelable(&element);
            parcel.WriteString(FuzzUtil::BuildUnicodeAttackString(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            break;
        }
        case 27: {
            actualCode = static_cast<uint32_t>(IAbilityScheduler::SCHEDULE_ABILITY_REQUEST_SUCCESS);
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            AppExecFwk::ElementName element;
            element.SetBundleName(FuzzUtil::BuildMaliciousBundleName(fdp));
            element.SetAbilityName(FuzzUtil::BuildSpecialCharString(fdp));
            element.SetDeviceID(FuzzUtil::BuildSandboxEscapePath(fdp));
            parcel.WriteParcelable(&element);
            OHOS::FuzzUtil::WriteSemanticPrivilegeEscalation(parcel, fdp);
            break;
        }
        case 28: {
            actualCode = static_cast<uint32_t>(IAbilityScheduler::SCHEDULE_ABILITIES_REQUEST_DONE);
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            OHOS::FuzzUtil::WriteAppSpawnMsgOob(parcel, fdp);
            break;
        }
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(AbilitySchedulerStubRemainingFuzz, FuzzUtil::Tokens::ABILITY_SCHEDULER)
} // namespace OHOS
