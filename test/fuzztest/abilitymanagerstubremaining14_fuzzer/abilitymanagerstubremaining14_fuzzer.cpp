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

#include "abilitymanagerstubremaining14_fuzzer.h"
#include "ability_manager_stub.h"
#include "ability_manager_stub_mock.h"
#include "attack_vectors.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>

#include "exit_reason.h"
#include "fuzz_util.h"
#include "message_parcel.h"
#include "parcelable_constructors.h"
#include "sandbox_clone_params.h"
#include "securec.h"
#include "session_info.h"
#include "skill/skill_execute_result.h"
#include "uri.h"
#include "want.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {
namespace {
constexpr int HANDLE_COUNT = 10;
}

class AbilityManagerStubRemaining14Fuzz : public AbilityManagerStubFuzzBase {};

// Convert a std::string (raw bytes) into a std::u16string so that
// Parcel::WriteString16 can drive the stub-side ReadString16 (UTF-16). Each
// input byte is placed in the low octet of a u16 char to keep the transform
// lossless and stable under fuzz mutation.
std::u16string ToU16(const std::string &s)
{
    std::u16string u16;
    u16.reserve(s.size());
    for (char c : s) {
        u16.push_back(static_cast<char16_t>(static_cast<uint8_t>(c)));
    }
    return u16;
}

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % HANDLE_COUNT) {
        case 0: {
            // EXECUTE_IN_APP_SKILL = 6169, ExecuteInAppSkillInner:
            //   ReadString16(bundleName) -> ReadString16(moduleName) -> ReadString16(skillName)
            //   -> ReadString16(scriptPath) -> ReadString16(functionName)
            //   -> ReadParcelable<WantParams>(skillArgs) -> ReadBool(hasCallback)
            //   -> [hasCallback] ReadRemoteObject(callbackObj)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::EXECUTE_IN_APP_SKILL);
            parcel.WriteString16(ToU16(FuzzUtil::BuildMaliciousBundleName(fdp)));
            parcel.WriteString16(ToU16(FuzzUtil::BuildSpecialCharString(fdp)));
            parcel.WriteString16(ToU16(FuzzUtil::BuildSpecialCharString(fdp)));
            parcel.WriteString16(ToU16(FuzzUtil::BuildSandboxEscapePath(fdp)));
            parcel.WriteString16(ToU16(FuzzUtil::BuildSpecialCharString(fdp)));
            {
                AAFwk::WantParams skillArgs;
                parcel.WriteParcelable(&skillArgs);
            }
            FuzzUtil::WriteOptionalRemoteObject(parcel, fdp);
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            break;
        }
        case 1: {
            // QUERY_SKILL_TYPE = 6171, QuerySkillTypeInner:
            //   ReadString16(bundleName) -> ReadString16(moduleName) -> ReadString16(skillName)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::QUERY_SKILL_TYPE);
            parcel.WriteString16(ToU16(FuzzUtil::BuildMaliciousBundleName(fdp)));
            parcel.WriteString16(ToU16(FuzzUtil::BuildSpecialCharString(fdp)));
            parcel.WriteString16(ToU16(FuzzUtil::BuildSpecialCharString(fdp)));
            OHOS::FuzzUtil::WriteSaAutoTrustBypass(parcel, fdp);
            break;
        }
        case 2: {
            // EXECUTE_SKILL_DONE_WITH_TOKEN = 6172, ExecuteSkillDoneWithTokenInner:
            //   ReadRemoteObject(token, non-null) -> ReadString(requestCode)
            //   -> ReadInt32(resultCode) -> ReadParcelable<SkillExecuteResult>(result, non-null)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::EXECUTE_SKILL_DONE_WITH_TOKEN);
            parcel.WriteRemoteObject(nullptr);  // sptr<IRemoteObject> token (rule 007)
            parcel.WriteString(FuzzUtil::GenStrcpyOverflowString(fdp, 64));  // requestCode
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));          // resultCode
            {
                AppExecFwk::SkillExecuteResult result;
                parcel.WriteParcelable(&result);
            }
            OHOS::FuzzUtil::WriteParcelableRawPointerLeak(parcel, fdp);
            break;
        }
        case 3: {
            // EXECUTE_IN_APP_SKILL_WITH_TOKEN_ID = 6173, ExecuteInAppSkillWithTokenIdInner:
            //   ReadUint32(callerTokenId) -> ReadString16(bundleName) -> ReadString16(moduleName)
            //   -> ReadString16(skillName) -> ReadString16(scriptPath) -> ReadString16(functionName)
            //   -> ReadParcelable<WantParams>(skillArgs) -> ReadBool(hasCallback)
            //   -> [hasCallback] ReadRemoteObject(callbackObj)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::EXECUTE_IN_APP_SKILL_WITH_TOKEN_ID);
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            parcel.WriteString16(ToU16(FuzzUtil::BuildMaliciousBundleName(fdp)));
            parcel.WriteString16(ToU16(FuzzUtil::BuildSpecialCharString(fdp)));
            parcel.WriteString16(ToU16(FuzzUtil::BuildSpecialCharString(fdp)));
            parcel.WriteString16(ToU16(FuzzUtil::BuildSandboxEscapePath(fdp)));
            parcel.WriteString16(ToU16(FuzzUtil::BuildSpecialCharString(fdp)));
            {
                AAFwk::WantParams skillArgs;
                parcel.WriteParcelable(&skillArgs);
            }
            FuzzUtil::WriteOptionalRemoteObject(parcel, fdp);
            OHOS::FuzzUtil::WriteSemanticPrivilegeEscalation(parcel, fdp);
            break;
        }
        case 4: {
            // START_SELF_UI_ABILITY_BY_APP_CONTEXT = 6175, StartSelfUIAbilityByAppContextInner:
            //   ReadParcelable<Want>(want, non-null) -> SanitizeWantParams
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_SELF_UI_ABILITY_BY_APP_CONTEXT);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);  // Want::Marshal drives ReadParcelable<Want>
            OHOS::FuzzUtil::WriteDeepNestedParcel(parcel, fdp);
            break;
        }
        case 5: {
            // START_SANDBOX_CLONE_ABILITY = 6176, StartSandboxCloneAbilityInner:
            //   ReadParcelable<Want>(want, non-null) -> ReadParcelable<SandboxCloneParams>(params, non-null)
            // SandboxCloneParams::Marshalling (see sandbox_clone_params.cpp):
            //   WriteString16(callerBundleName) + WriteInt32(callerUid) + WriteUint32(callerTokenId)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_SANDBOX_CLONE_ABILITY);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            {
                AAFwk::SandboxCloneParams params;
                params.callerBundleName = FuzzUtil::BuildMaliciousBundleName(fdp);
                params.callerUid = FuzzUtil::BuildClientSuppliedUid(fdp);
                params.callerTokenId = fdp.ConsumeIntegral<uint32_t>();
                parcel.WriteParcelable(&params);
            }
            OHOS::FuzzUtil::WriteMmapCorruption(parcel, fdp);
            break;
        }
        case 6: {
            // NOTIFY_SKILL_FUNCTION_INVOKED = 6177, NotifySkillFunctionInvokedInner:
            //   ReadRemoteObject(token, non-null) -> ReadString(requestCode)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::NOTIFY_SKILL_FUNCTION_INVOKED);
            parcel.WriteRemoteObject(nullptr);  // sptr<IRemoteObject> token (rule 007)
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));  // requestCode
            OHOS::FuzzUtil::WriteDumpStateInfoLeak(parcel, fdp);
            break;
        }
        case 7: {
            // RECORD_APP_WITH_REASON_BY_USERID = 6010, RecordAppWithReasonByUserIdInner:
            //   ReadInt32(userId) -> ReadParcelable<ExitReasonCompability>(exitReason, non-null)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::RECORD_APP_WITH_REASON_BY_USERID);
            parcel.WriteInt32(FuzzUtil::BuildClientSuppliedUid(fdp));
            FuzzUtil::WriteMaliciousExitReasonCompability(parcel, fdp);
            OHOS::FuzzUtil::WriteMmapCorruption(parcel, fdp);
            break;
        }
        case 8: {
            // UPDATE_SESSION_INFO = 6011, UpdateSessionInfoBySCBInner:
            //   ReadInt32(size, <=512) -> for size: ReadParcelable<SessionInfo>(info, non-null)
            //   -> ReadInt32(userId)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::UPDATE_SESSION_INFO);
            {
                int32_t sessionSize = static_cast<int32_t>(fdp.ConsumeIntegral<uint8_t>() % 4);
                parcel.WriteInt32(sessionSize);
                for (int32_t i = 0; i < sessionSize; i++) {
                    SessionInfo info;
                    parcel.WriteParcelable(&info);
                }
            }
            parcel.WriteInt32(FuzzUtil::BuildClientSuppliedUid(fdp));
            OHOS::FuzzUtil::WriteHugeRawDataDoS(parcel, fdp);
            break;
        }
        case 9: {
            // START_ABILITY_WITH_WAIT = 6143, StartAbilityWithWaitInner:
            //   ReadParcelable<Want>(want, non-null) -> ReadRemoteObject(callback, non-null)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_WITH_WAIT);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            parcel.WriteRemoteObject(nullptr);  // sptr<IAbilityStartWithWaitObserver> (rule 007)
            OHOS::FuzzUtil::WriteSaAutoTrustBypass(parcel, fdp);
            break;
        }
        default:
            break;
    }

}

FUZZ_STUB_ENTRY_IMPL(AbilityManagerStubRemaining14Fuzz, FuzzUtil::Tokens::ABILITY_MGR)
} // namespace OHOS
