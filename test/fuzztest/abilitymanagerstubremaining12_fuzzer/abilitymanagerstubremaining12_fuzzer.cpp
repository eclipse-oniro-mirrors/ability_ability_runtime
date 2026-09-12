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

#include "abilitymanagerstubremaining12_fuzzer.h"
#include "ability_manager_stub.h"
#include "ability_manager_stub_mock.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>

#include "attack_vectors.h"
#include "fuzz_util.h"
#include "message_parcel.h"
#include "securec.h"
#include "want.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {
namespace {
constexpr int HANDLE_COUNT = 12;
}

class AbilityManagerStubRemaining12Fuzz : public AbilityManagerStubFuzzBase {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % HANDLE_COUNT) {
        case 0: {
            // SET_APPLICATION_KEEP_ALLIVE_BY_EDM = 6123, SetApplicationKeepAliveByEDMInner:
            //   ReadString(bundleName) -> ReadInt32(userId) -> ReadBool(flag)
            //   -> ReadBool(isAllowUserToCancel)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_APPLICATION_KEEP_ALLIVE_BY_EDM);
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteInt32(FuzzUtil::BuildClientSuppliedUid(fdp));
            parcel.WriteBool(fdp.ConsumeBool());
            parcel.WriteBool(fdp.ConsumeBool());
            OHOS::FuzzUtil::WriteSaAutoTrustBypass(parcel, fdp);
            break;
        }
        case 1: {
            // GET_APPLICATIONS_KEEP_ALIVE_BY_EDM = 6124, QueryKeepAliveApplicationsByEDMInner:
            //   ReadInt32(appType) -> ReadInt32(userId)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_APPLICATIONS_KEEP_ALIVE_BY_EDM);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildClientSuppliedUid(fdp));
            OHOS::FuzzUtil::WriteSemanticPrivilegeEscalation(parcel, fdp);
            break;
        }
        case 2: {
            // SET_APP_SERVICE_EXTENSION_KEEP_ALIVE = 6126, SetAppServiceExtensionKeepAliveInner:
            //   ReadString(bundleName) -> ReadBool(flag)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_APP_SERVICE_EXTENSION_KEEP_ALIVE);
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteBool(fdp.ConsumeBool());
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            break;
        }
        case 3: {
            // GET_APP_SERVICE_EXTENSIONS_KEEP_ALIVE = 6127, QueryKeepAliveAppServiceExtensionsInner:
            //   no parcel read fields (writes only reply). Parcel still carries an attack
            //   vector so the fuzz engine keeps mutating data across the dispatch path.
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_APP_SERVICE_EXTENSIONS_KEEP_ALIVE);
            OHOS::FuzzUtil::WriteHugeRawDataDoS(parcel, fdp);
            break;
        }
        case 4: {
            // ADD_QUERY_ERMS_OBSERVER = 6130, AddQueryERMSObserverInner:
            //   ReadRemoteObject(callerToken) -> ReadRemoteObject(observer)
            //   observer is sptr<IQueryERMSObserver> cast from a remote object (rule 007).
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::ADD_QUERY_ERMS_OBSERVER);
            parcel.WriteRemoteObject(nullptr);  // sptr<IRemoteObject> callerToken (rule 007)
            parcel.WriteRemoteObject(nullptr);  // sptr<IQueryERMSObserver> observer (rule 007)
            OHOS::FuzzUtil::WriteParcelableRawPointerLeak(parcel, fdp);
            break;
        }
        case 5: {
            // QUERY_ATOMIC_SERVICE_STARTUP_RULE = 6131, QueryAtomicServiceStartupRuleInner:
            //   ReadRemoteObject(callerToken) -> ReadString(appId) -> ReadString(startTime)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::QUERY_ATOMIC_SERVICE_STARTUP_RULE);
            parcel.WriteRemoteObject(nullptr);  // sptr<IRemoteObject> callerToken (rule 007)
            parcel.WriteString(FuzzUtil::GenStrcpyOverflowString(fdp, 64));  // appId
            parcel.WriteString(FuzzUtil::BuildUnicodeAttackString(fdp));     // startTime
            OHOS::FuzzUtil::WriteMmapCorruption(parcel, fdp);
            break;
        }
        case 6: {
            // REVOKE_DELEGATOR = 6139, RevokeDelegatorInner:
            //   ReadRemoteObject(token)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::REVOKE_DELEGATOR);
            parcel.WriteRemoteObject(nullptr);  // sptr<IRemoteObject> token (rule 007)
            OHOS::FuzzUtil::WriteDeepNestedParcel(parcel, fdp);
            break;
        }
        case 7: {
            // GET_INSIGHT_INTENT_INFO_BY_BUNDLE_NAME = 6141, GetInsightIntentInfoByBundleNameInner:
            //   ReadUint32(flag) -> ReadString(bundleName) -> ReadInt32(userId)
            //   flag is GetInsightIntentFlag enum read as uint32_t.
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_INSIGHT_INTENT_INFO_BY_BUNDLE_NAME);
            parcel.WriteUint32(static_cast<uint32_t>(FuzzUtil::BuildInvalidEnum(fdp, 16)));
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteInt32(FuzzUtil::BuildClientSuppliedUid(fdp));
            OHOS::FuzzUtil::WriteDumpStateInfoLeak(parcel, fdp);
            break;
        }
        case 8: {
            // GET_INSIGHT_INTENT_INFO_BY_INTENT_NAME = 6142, GetInsightIntentInfoByIntentNameInner:
            //   ReadUint32(flag) -> ReadString(bundleName) -> ReadString(modlueName)
            //   -> ReadString(intentName) -> ReadInt32(userId)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_INSIGHT_INTENT_INFO_BY_INTENT_NAME);
            parcel.WriteUint32(static_cast<uint32_t>(FuzzUtil::BuildInvalidEnum(fdp, 16)));
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteString(FuzzUtil::GenStrcpyOverflowString(fdp, 64));  // modlueName
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));       // intentName
            parcel.WriteInt32(FuzzUtil::BuildClientSuppliedUid(fdp));
            OHOS::FuzzUtil::WriteIntegerOverflowMul3(parcel, fdp);
            break;
        }
        case 9: {
            // START_ABILITY_WITH_WAIT = 6143, StartAbilityWithWaitInner:
            //   ReadParcelable<Want>(want) -> ReadRemoteObject(callback)
            //   callback is sptr<IAbilityStartWithWaitObserver> cast from a remote object.
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_WITH_WAIT);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            parcel.WriteRemoteObject(nullptr);  // sptr<IAbilityStartWithWaitObserver> callback (rule 007)
            OHOS::FuzzUtil::WriteSaAutoTrustBypass(parcel, fdp);
            break;
        }
        case 10: {
            // RESTART_SELF_ATOMIC_SERVICE = 6144, RestartSelfAtomicServiceInner:
            //   ReadRemoteObject(callerToken)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::RESTART_SELF_ATOMIC_SERVICE);
            parcel.WriteRemoteObject(nullptr);  // sptr<IRemoteObject> callerToken (rule 007)
            OHOS::FuzzUtil::WriteSemanticPrivilegeEscalation(parcel, fdp);
            break;
        }
        case 11: {
            // GET_KIOSK_INFO = 6148, GetKioskStatusInner:
            //   no parcel read fields (writes only reply). Parcel still carries an attack
            //   vector so the fuzz engine keeps mutating data across the dispatch path.
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_KIOSK_INFO);
            OHOS::FuzzUtil::WriteDeepNestedParcel(parcel, fdp);
            break;
        }
        default:
            break;
    }

}

FUZZ_STUB_ENTRY_IMPL(AbilityManagerStubRemaining12Fuzz, FuzzUtil::Tokens::ABILITY_MGR)
} // namespace OHOS
