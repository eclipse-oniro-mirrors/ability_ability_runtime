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

#ifndef FUZZTEST_OHOS_ABILITY_RUNTIME_PARCELABLE_CONSTRUCTORS_H
#define FUZZTEST_OHOS_ABILITY_RUNTIME_PARCELABLE_CONSTRUCTORS_H

#include <cstddef>
#include <cstdint>
#include <vector>

#include <fuzzer/FuzzedDataProvider.h>
#include "message_parcel.h"

#include "attack_vectors.h"
#include "auto_startup_info.h"
#include "exit_reason.h"
#include "uri.h"
#include "want_sender_info.h"

namespace OHOS {
namespace FuzzUtil {

// Construct a malicious AutoStartupInfo parcelable and write it via WriteParcelable
// so that the stub-side ReadParcelable<AutoStartupInfo> can correctly parse it.
// Field order follows AutoStartupInfo::Marshalling (see auto_startup_info.cpp):
//   WriteString16(bundleName) + WriteString16(abilityName) + WriteString16(moduleName)
//   + WriteString16(abilityTypeName) + WriteInt32(appCloneIndex) + WriteInt32(userId)
//   + WriteInt32(setterUserId) + WriteBool(canUserModify) + WriteBool(isHiddenStart)
inline void WriteMaliciousAutoStartupInfo(MessageParcel &parcel, FuzzedDataProvider &fdp,
    size_t maxStrLen = DEFAULT_MAX_STR_LEN)
{
    AbilityRuntime::AutoStartupInfo info;
    info.bundleName = BuildMaliciousBundleName(fdp, maxStrLen);
    info.abilityName = BuildSpecialCharString(fdp, maxStrLen);
    info.moduleName = BuildSpecialCharString(fdp, maxStrLen);
    info.abilityTypeName = BuildSpecialCharString(fdp, maxStrLen);
    info.appCloneIndex = BuildIntegerOverflow(fdp);
    info.userId = BuildIntegerOverflow(fdp);
    info.setterUserId = BuildClientSuppliedUid(fdp);
    info.canUserModify = fdp.ConsumeBool();
    info.isHiddenStart = fdp.ConsumeBool();
    parcel.WriteParcelable(&info);
}

// Construct a malicious ConnectionData parcel. Field order follows
// ConnectionObserverStub::OnExtensionConnectedInner read sequence:
//   ReadInt32(connectionId) -> ReadInt32(extensionPid) -> ReadInt32(extensionUid)
//   -> ReadString16(bundleName) -> ReadString16(moduleName) -> ReadString16(name)
//   -> ReadInt32(extensionType) -> ReadInt32(callerUid) -> ReadInt32(callerPid)
//   -> ReadString16(callerName) -> ReadBool(isPerCallerForeground)
inline void WriteMaliciousConnectionData(MessageParcel &parcel, FuzzedDataProvider &fdp)
{
    parcel.WriteInt32(1);
    parcel.WriteInt32(BuildIntegerOverflow(fdp));
    parcel.WriteInt32(BuildIntegerOverflow(fdp));
    parcel.WriteString16(Str8ToStr16(BuildMaliciousBundleName(fdp)));
    parcel.WriteString16(Str8ToStr16(BuildSpecialCharString(fdp)));
    parcel.WriteString16(Str8ToStr16(BuildSpecialCharString(fdp)));
    parcel.WriteInt32(BuildInvalidEnum(fdp, 30));
    parcel.WriteInt32(BuildIntegerOverflow(fdp));
    parcel.WriteInt32(BuildIntegerOverflow(fdp));
    parcel.WriteString16(Str8ToStr16(BuildSpecialCharString(fdp)));
    parcel.WriteBool(fdp.ConsumeBool());
}

// Construct a malicious DataObsOption parcel. Field order follows
// DataObsManagerStub read sequence for registered observer options:
//   ReadBool(isBatchNotify) -> ReadUint32(userId) -> ReadInt32(changeType)
//   -> ReadBool(enabled) -> ReadUint64(pollInterval)
inline void WriteMaliciousDataObsOption(MessageParcel &parcel, FuzzedDataProvider &fdp)
{
    parcel.WriteBool(fdp.ConsumeBool());
    parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
    parcel.WriteInt32(BuildIntegerOverflow(fdp));
    parcel.WriteBool(fdp.ConsumeBool());
    parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
}

// Construct a malicious ExitReason parcelable and write it via WriteParcelable
// so that the stub-side ReadParcelable<ExitReason> can correctly parse it.
// Field order follows ExitReason::Marshalling (see exit_reason.cpp):
//   WriteInt32(reason) + WriteInt32(subReason) + WriteString16(exitMsg)
//   + WriteBool(shouldKillForeground) + WriteBool(shouldSkipKillInStartup) + WriteInt32(killId)
inline void WriteMaliciousExitReason(MessageParcel &parcel, FuzzedDataProvider &fdp,
    size_t maxStrLen = DEFAULT_MAX_STR_LEN)
{
    AAFwk::ExitReason reason;
    reason.reason = static_cast<AAFwk::Reason>(BuildInvalidEnum(fdp, 20));
    reason.subReason = BuildIntegerOverflow(fdp);
    reason.exitMsg = BuildSpecialCharString(fdp, maxStrLen);
    reason.shouldKillForeground = fdp.ConsumeBool();
    reason.shouldSkipKillInStartup = fdp.ConsumeBool();
    reason.killId = BuildIntegerOverflow(fdp);
    parcel.WriteParcelable(&reason);
}

// Construct a malicious ExitReasonCompability parcelable and write it via
// WriteParcelable so that the stub-side ReadParcelable<ExitReasonCompability>
// can correctly parse it. Field order follows ExitReasonCompability::Marshalling
// (see exit_reason.cpp):
//   WriteInt32(reason) + WriteString16(exitMsg) + WriteInt32(subReason)
//   + WriteInt32(killId) + WriteString16(killMsg) + WriteString16(innerMsg)
//   + WriteBool(shouldKillForeground) + WriteBool(shouldSkipKillInStartup)
inline void WriteMaliciousExitReasonCompability(MessageParcel &parcel, FuzzedDataProvider &fdp,
    size_t maxStrLen = DEFAULT_MAX_STR_LEN)
{
    AAFwk::ExitReasonCompability reason;
    reason.reason = static_cast<AAFwk::Reason>(BuildInvalidEnum(fdp, 20));
    reason.exitMsg = BuildSpecialCharString(fdp, maxStrLen);
    reason.subReason = BuildIntegerOverflow(fdp);
    reason.killId = BuildIntegerOverflow(fdp);
    reason.killMsg = GenStrcpyOverflowString(fdp, 64);
    reason.innerMsg = BuildUnicodeAttackString(fdp);
    reason.shouldKillForeground = fdp.ConsumeBool();
    reason.shouldSkipKillInStartup = fdp.ConsumeBool();
    parcel.WriteParcelable(&reason);
}

// Construct a malicious Uri parcelable and write it via WriteParcelable so that
// the stub-side ReadParcelable<Uri> can correctly parse it.
inline void WriteMaliciousUri(MessageParcel &parcel, FuzzedDataProvider &fdp)
{
    Uri uri(BuildUriAttackString(fdp));
    parcel.WriteParcelable(&uri);
}

// Construct a vector of malicious Uri parcelables. The count is written first as
// ReadInt32, followed by that many WriteParcelable<Uri> entries.
inline void WriteMaliciousUriVector(MessageParcel &parcel, FuzzedDataProvider &fdp)
{
    uint8_t count = fdp.ConsumeIntegral<uint8_t>() % VEC_MAX_SIZE;
    parcel.WriteInt32(static_cast<int32_t>(count));
    for (uint8_t i = 0; i < count; i++) {
        Uri uri(BuildUriAttackString(fdp));
        parcel.WriteParcelable(&uri);
    }
}

// Construct malicious raw data (UriPermissionRawData-compatible). Writes a size
// prefix followed by that many fuzz bytes via WriteBuffer.
inline void WriteMaliciousRawData(MessageParcel &parcel, FuzzedDataProvider &fdp)
{
    uint32_t rawSize = fdp.ConsumeIntegral<uint32_t>() % 1024;
    auto rawBytes = fdp.ConsumeBytes<uint8_t>(rawSize);
    parcel.WriteUint32(rawSize);
    if (rawSize > 0 && rawBytes.size() > 0) {
        parcel.WriteBuffer(rawBytes.data(), rawBytes.size());
    }
}

// Construct a malicious WantSenderInfo parcelable and write it via WriteParcelable
// so that the stub-side ReadParcelable<WantSenderInfo> can correctly parse it.
// Field order follows WantSenderInfo::Marshalling (see want_sender_info.cpp):
//   WriteInt32(type) + WriteString16(bundleName) + WriteString16(resultWho)
//   + WriteInt32(requestCode) + WriteInt32(wantsInfoSize) + loop WriteParcelable<WantsInfo>
//   + WriteUint32(flags) + WriteInt32(userId) + WriteInt32(appIndex)
inline void WriteMaliciousWantSenderInfo(MessageParcel &parcel, FuzzedDataProvider &fdp,
    size_t maxStrLen = DEFAULT_MAX_STR_LEN)
{
    AAFwk::WantSenderInfo info;
    info.type = BuildInvalidEnum(fdp, 20);
    info.bundleName = BuildMaliciousBundleName(fdp, maxStrLen);
    info.resultWho = BuildSpecialCharString(fdp, maxStrLen);
    info.requestCode = BuildIntegerOverflow(fdp);
    info.flags = fdp.ConsumeIntegral<uint32_t>();
    info.userId = BuildIntegerOverflow(fdp);
    info.appIndex = BuildIntegerOverflow(fdp);
    parcel.WriteParcelable(&info);
}

} // namespace FuzzUtil
} // namespace OHOS

#endif // FUZZTEST_OHOS_ABILITY_RUNTIME_PARCELABLE_CONSTRUCTORS_H
