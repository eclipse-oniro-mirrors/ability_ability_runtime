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

#ifndef FUZZTEST_OHOS_ABILITY_RUNTIME_FUZZ_UTIL_H
#define FUZZTEST_OHOS_ABILITY_RUNTIME_FUZZ_UTIL_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <fuzzer/FuzzedDataProvider.h>
#include "message_parcel.h"
#include "message_option.h"
#include "securec.h"

namespace OHOS {
namespace FuzzUtil {
// Index constants for U32Data extraction from first 4 bytes
constexpr int INPUT_ZERO = 0;
constexpr int INPUT_ONE = 1;
constexpr int INPUT_TWO = 2;
constexpr int INPUT_THREE = 3;

// Size and bit offset constants for U32Data extraction
constexpr size_t U32_AT_SIZE = 4;
constexpr size_t OFFSET_ZERO = 24;
constexpr size_t OFFSET_ONE = 16;
constexpr size_t OFFSET_TWO = 8;

// Default max length for fuzz strings and max size for fuzz vectors
constexpr size_t STRING_MAX_LENGTH = 256;
constexpr size_t VEC_MAX_SIZE = 8;

// Extract uint32_t from the first 4 bytes of fuzz input data.
inline uint32_t GetU32Data(const char *ptr)
{
    return (ptr[INPUT_ZERO] << OFFSET_ZERO) | (ptr[INPUT_ONE] << OFFSET_ONE) |
        (ptr[INPUT_TWO] << OFFSET_TWO) | ptr[INPUT_THREE];
}

// Build a vector of strings from fuzz data. Length is controlled to avoid memory explosion.
inline std::vector<std::string> BuildStringVector(FuzzedDataProvider &fdp,
    size_t maxLength = STRING_MAX_LENGTH, size_t maxSize = VEC_MAX_SIZE)
{
    std::vector<std::string> vec;
    uint8_t count = fdp.ConsumeIntegral<uint8_t>() % maxSize;
    for (uint8_t i = 0; i < count; i++) {
        vec.push_back(fdp.ConsumeRandomLengthString(maxLength));
    }
    return vec;
}

// Build a vector of int32_t from fuzz data. Length is controlled to avoid memory explosion.
inline std::vector<int32_t> BuildInt32Vector(FuzzedDataProvider &fdp, size_t maxSize = VEC_MAX_SIZE)
{
    std::vector<int32_t> vec;
    uint8_t count = fdp.ConsumeIntegral<uint8_t>() % maxSize;
    for (uint8_t i = 0; i < count; i++) {
        vec.push_back(fdp.ConsumeIntegral<int32_t>());
    }
    return vec;
}

// ==================== Interface Token Constants ====================
namespace Tokens {
    constexpr std::u16string_view ABILITY_MGR = u"ohos.aafwk.AbilityManager";
    constexpr std::u16string_view APP_MGR = u"ohos.appexecfwk.AppMgr";
    constexpr std::u16string_view AMS_MGR = u"ohos.appexecfwk.IAmsMgr";
    constexpr std::u16string_view ABILITY_SCHEDULER = u"ohos.aafwk.AbilityScheduler";
    constexpr std::u16string_view ABILITY_TOKEN = u"ohos.aafwk.AbilityToken";
    constexpr std::u16string_view WANT_SENDER = u"ohos.aafwk.WantSender";
    constexpr std::u16string_view WANT_RECEIVER = u"ohos.aafwk.WantReceiver";
    constexpr std::u16string_view USER_CALLBACK = u"ohos.aafwk.UserCallback";
    constexpr std::u16string_view ABILITY_CONNECTION = u"ohos.abilityshell.DistributedConnection";
    constexpr std::u16string_view WMS_HANDLER = u"ohos.aafwk.WindowManagerServiceHandler";
    constexpr std::u16string_view MISSION_LISTENER = u"ohos.aafwk.MissionListener";
    constexpr std::u16string_view REMOTE_MISSION_LISTENER = u"ohos.aafwk.RemoteMissionListener";
    constexpr std::u16string_view REMOTE_ON_LISTENER = u"ohos.aafwk.RemoteOnListener";
    constexpr std::u16string_view SESSION_HANDLER = u"ohos.aafwk.SessionHandler";
    constexpr std::u16string_view SA_TOKEN_CALLBACK = u"ohos.aafwk.ISystemAbilityTokenCallback";
    constexpr std::u16string_view ATOMIC_SERVICE_CB = u"ohos.IAtomicServiceStatusCallback";
    constexpr std::u16string_view AUTO_STARTUP_CB = u"ohos.aafwk.AutoStartupCallBack";
    constexpr std::u16string_view ACQUIRE_SHARE_DATA_CB = u"ohos.aafwk.acquireShareDataCallback";
    constexpr std::u16string_view PREPARE_TERMINATE_CB = u"ohos.aafwk.prepareTerminateCallback";
    constexpr std::u16string_view INSIGHT_INTENT_EXEC_CB = u"ohos.AAFwk.IntentExecuteCallback";
    constexpr std::u16string_view PRELOAD_UIEXT_CB = u"ohos.AAFwk.PreloadUIExtensionCallback";
    constexpr std::u16string_view REMOTE_INTENT_CB = u"ohos.distributedschedule.IRemoteIntentResultCallback";
    constexpr std::u16string_view HIDDEN_START_OBSERVER = u"ohos.aafwk.IHiddenStartObserver";
    constexpr std::u16string_view SA_INTERCEPTOR = u"ohos.AbilityRuntime.ISAInterceptor";
    constexpr std::u16string_view STATUS_BAR_DELEGATE = u"ohos.ability.StatusBarDelegate";
    constexpr std::u16string_view FIRST_FRAME_OBSERVER = u"ohos.appexecfwk.IAbilitFirstFrameState";
    constexpr std::u16string_view START_WAIT_OBSERVER = u"ohos.ability.IAbilityStartWithWaitObserver";
    constexpr std::u16string_view REQUEST_START_ABILITY_CB = u"ohos.aafwk.IRequestStartAbilityCallback";
    constexpr std::u16string_view SKILL_EXECUTE_CB = u"ohos.AAFwk.SkillExecuteCallback";
    constexpr std::u16string_view CONNECTION_OBSERVER = u"ohos.abilityruntime.connectionobserver";
    constexpr std::u16string_view FOREGROUND_APP_CONN = u"ohos.abilityruntime.foregroundappconnection";
    constexpr std::u16string_view DATA_OBS_MGR = u"ohos.aafwk.DataObsMgr";
    constexpr std::u16string_view DATA_ABILITY_OBSERVER = u"ohos.aafwk.DataAbilityObserver";
    constexpr std::u16string_view URI_PERMISSION_MGR = u"OHOS.AAFwk.IUriPermissionManager";
    constexpr std::u16string_view QUICK_FIX_MGR = u"OHOS.AAFwk.IQuickFixManager";
    constexpr std::u16string_view QUICK_FIX_CB = u"ohos.appexecfwk.QuickFixCallback";
    constexpr std::u16string_view FREE_INSTALL_OBSERVER = u"ohos.aafwk.IFreeInstallObserver";
    constexpr std::u16string_view QUERY_ERMS_OBSERVER = u"ohos.aafwk.IQueryERMSObserver";
    constexpr std::u16string_view REMOTE_REGISTER_SERVICE = u"ohos.appexecfwk.RemoteRegisterService";
    constexpr std::u16string_view CONNECT_CALLBACK = u"ohos.appexecfwk.iconnectcallback";
    constexpr std::u16string_view ABILITY_CONTROLLER = u"ohos.appexecfwk.IAbilityController";
    constexpr std::u16string_view APP_SCHEDULER = u"ohos.appexecfwk.AppScheduler";
    constexpr std::u16string_view APP_STATE_CALLBACK = u"ohos.appexecfwk.AppStateCallback";
    constexpr std::u16string_view RENDER_SCHEDULER = u"ohos.appexecfwk.RenderScheduler";
    constexpr std::u16string_view APP_STATE_OBSERVER = u"ohos.appexecfwk.IApplicationStateObserver";
    constexpr std::u16string_view APP_FG_STATE_OBSERVER = u"ohos.appexecfwk.IAppForegroundStateObserver";
    constexpr std::u16string_view ABILITY_FG_STATE_OBSERVER = u"ohos.appexecfwk.IAbilityForegroundStateObserver";
    constexpr std::u16string_view APP_DEBUG_LISTENER = u"ohos.appexecfwk.IAmsMgr";
    constexpr std::u16string_view ABILITY_DEBUG_RESPONSE = u"ohos.appexecfwk.AbilityDebugResponse";
    constexpr std::u16string_view CONFIG_OBSERVER = u"ohos.abilityruntime.IConfigurationObserver";
    constexpr std::u16string_view ABILITY_INFO_CB = u"ohos.appexecfwk.IAbilityInfoCallback";
    constexpr std::u16string_view CHILD_SCHEDULER = u"ohos.appexecfwk.ChildScheduler";
    constexpr std::u16string_view APP_RUNNING_STATUS = u"ohos.appexecfwk.AppRunningStatusListenerInterface";
    constexpr std::u16string_view START_SPECIFIED_RESPONSE = u"ohos.appexecfwk.startSpecifiedAbilityResponse";
    constexpr std::u16string_view LOAD_ABILITY_CB = u"ohos.appexecfwk.ILoadAbilityCallback";
    constexpr std::u16string_view NATIVE_CHILD_NOTIFY = u"ohos.appexecfwk.NativeChildNotify";
    constexpr std::u16string_view RENDER_STATE_OBSERVER = u"ohos.appexecfwk.IRenderStateObserver";
    constexpr std::u16string_view MEM_DUMP_CB = u"ohos.appexecfwk.MemDumpCallback";
    constexpr std::u16string_view IMAGE_ERROR_HANDLER = u"ohos.appexecfwk.IImageErrorHandler";
    constexpr std::u16string_view IMAGE_PROCESS_STATE_OBS = u"ohos.appexecfwk.IImageProcessStateObserver";
    constexpr std::u16string_view KIA_INTERCEPTOR = u"ohos.appexecfwk.IKiaInterceptor";
} // namespace Tokens

// ==================== Stub Fuzzer Skeleton Helpers ====================

// Prepare a standard fuzz parcel: extract code from data, write interface token.
// Returns the FuzzedDataProvider for remaining data consumption.
inline FuzzedDataProvider PrepareFuzzParcel(const char *data, size_t size,
    MessageParcel &parcel, const std::u16string &token)
{
    parcel.WriteInterfaceToken(token);
    return FuzzedDataProvider(reinterpret_cast<const uint8_t *>(data + U32_AT_SIZE),
         size - U32_AT_SIZE);
}

// Dispatch OnRemoteRequest on a stub instance after parcel construction.
template <typename StubType>
inline void DispatchStubRequest(StubType &stub, uint32_t code,
    MessageParcel &parcel, MessageParcel &reply, MessageOption &option)
{
    parcel.RewindRead(0);
    stub.OnRemoteRequest(code, parcel, reply, option);
}
} // namespace FuzzUtil
} // namespace OHOS

#define FUZZ_ENTRY_IMPL(Func) \
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) \
{ \
    if (data == nullptr || size < OHOS::FuzzUtil::U32_AT_SIZE) { return 0; } \
    char *ch = static_cast<char *>(malloc(size + 1)); \
    if (ch == nullptr) { return 0; } \
    (void)memset_s(ch, size + 1, 0x00, size + 1); \
    if (memcpy_s(ch, size + 1, data, size) != EOK) { free(ch); return 0; } \
    Func(ch, size); \
    free(ch); \
    return 0; \
}

#define FUZZ_EXTRACT_CODE(data, size) \
    uint32_t code = OHOS::FuzzUtil::GetU32Data(data); \
    FuzzedDataProvider fdp(reinterpret_cast<const uint8_t *>(data + OHOS::FuzzUtil::U32_AT_SIZE), \
        size - OHOS::FuzzUtil::U32_AT_SIZE)

// Skeleton macro for structured stub fuzzers.
// Usage: FUZZ_STUB_ENTRY_IMPL(MockClass, Token, StubBase)
// The user must define: void DoFuzzCases(uint32_t code, MessageParcel& parcel,
//     FuzzedDataProvider& fdp, uint32_t& actualCode);
#define FUZZ_STUB_ENTRY_IMPL(MockClass, Token) \
bool DoSomethingInterestingWithMyAPI(const char *data, size_t size) \
{ \
    FUZZ_EXTRACT_CODE(data, size); \
    MessageParcel parcel; \
    MessageParcel reply; \
    MessageOption option; \
    uint32_t actualCode = 0; \
    parcel.WriteInterfaceToken(std::u16string(Token)); \
    DoFuzzCases(code, parcel, fdp, actualCode); \
    parcel.RewindRead(0); \
    auto stub = std::make_shared<MockClass>(); \
    stub->OnRemoteRequest(actualCode, parcel, reply, option); \
    return true; \
} \
FUZZ_ENTRY_IMPL(OHOS::DoSomethingInterestingWithMyAPI)

// Skeleton macro for legacy WriteBuffer-style service fuzzers.
// Usage: FUZZ_ABILITY_SERVICE_ENTRY_IMPL(AbilityManagerInterfaceCode::START_ABILITY)
// Prerequisite: fuzzer must #define private public / #include "ability_manager_service.h" / #undef private
// before calling this macro, so that AbilityManagerService and SubManagersHelper are visible.
#define FUZZ_ABILITY_SERVICE_ENTRY_IMPL(CodeEnum) \
bool DoSomethingInterestingWithMyAPI(const char *data, size_t size) \
{ \
    uint32_t code = static_cast<uint32_t>(CodeEnum); \
    MessageParcel parcel; \
    parcel.WriteInterfaceToken(std::u16string(FuzzUtil::Tokens::ABILITY_MGR)); \
    parcel.WriteBuffer(data, size); \
    parcel.RewindRead(0); \
    MessageParcel reply; \
    MessageOption option; \
    DelayedSingleton<AbilityManagerService>::GetInstance()->subManagersHelper_ = \
        std::make_shared<SubManagersHelper>(nullptr, nullptr); \
    DelayedSingleton<AbilityManagerService>::GetInstance()->subManagersHelper_->currentUIAbilityManager_ = \
        std::make_shared<UIAbilityLifecycleManager>(); \
    DelayedSingleton<AbilityManagerService>::GetInstance()->OnRemoteRequest(code, parcel, reply, option); \
    return true; \
} \
FUZZ_ENTRY_IMPL(OHOS::DoSomethingInterestingWithMyAPI)

#endif // FUZZTEST_OHOS_ABILITY_RUNTIME_FUZZ_UTIL_H
