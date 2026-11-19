/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "amsmgrscheduler_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#include "ams_mgr_scheduler.h"
#undef private
#include "ability_record.h"
#include "param.h"
#include "parcel.h"
#include "securec.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr int INPUT_ZERO = 0;
constexpr int INPUT_ONE = 1;
constexpr int INPUT_THREE = 3;
constexpr size_t U32_AT_SIZE = 4;
constexpr uint8_t ENABLE = 2;
constexpr size_t OFFSET_ZERO = 24;
constexpr size_t OFFSET_ONE = 16;
constexpr size_t OFFSET_TWO = 8;
}
uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[INPUT_ZERO] << OFFSET_ZERO) | (ptr[INPUT_ONE] << OFFSET_ONE) | (ptr[ENABLE] << OFFSET_TWO) |
        ptr[INPUT_THREE];
}
sptr<Token> GetFuzzAbilityToken()
{
    sptr<Token> token = nullptr;

    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.fuzzTest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (abilityRecord) {
        token = abilityRecord->GetToken();
    }

    return token;
}
std::shared_ptr<AmsMgrScheduler> DoSomethingInterestingWithMyAPI1(sptr<IRemoteObject> token,
    sptr<IRemoteObject> preToken, const char* data)
{
    std::shared_ptr<AppMgrServiceInner> mgrServiceInner;
    std::shared_ptr<AAFwk::TaskHandlerWrap> handler;
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler = std::make_shared<AmsMgrScheduler>(mgrServiceInner, handler);
    sptr<IStartSpecifiedAbilityResponse> response;
    amsMgrScheduler->RegisterStartSpecifiedAbilityResponse(response);
    std::shared_ptr<AbilityInfo> abilityInfoptr;
    std::shared_ptr<ApplicationInfo> appInfo;
    std::shared_ptr<AAFwk::Want> wantptr;
    int32_t abilityRecordId = static_cast<int32_t>(GetU32Data(data));
    AbilityRuntime::LoadParam loadParam;
    loadParam.abilityRecordId = abilityRecordId;
    loadParam.token = token;
    loadParam.preToken = preToken;
    auto loadParamPtr = std::make_shared<AbilityRuntime::LoadParam>(loadParam);
    amsMgrScheduler->LoadAbility(abilityInfoptr, appInfo, wantptr, loadParamPtr);
    bool clearMissionFlag = *data % ENABLE;
    amsMgrScheduler->TerminateAbility(token, clearMissionFlag);
    return amsMgrScheduler;
}

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    sptr<IRemoteObject> token = GetFuzzAbilityToken();
    sptr<IRemoteObject> preToken = nullptr;
    auto amsMgrScheduler = DoSomethingInterestingWithMyAPI1(token, preToken, data);
    AppExecFwk::AbilityState state = AppExecFwk::AbilityState::ABILITY_STATE_READY;
    amsMgrScheduler->UpdateAbilityState(token, state);
    AppExecFwk::ExtensionState extensionState = AppExecFwk::ExtensionState::EXTENSION_STATE_READY;
    amsMgrScheduler->UpdateExtensionState(token, extensionState);
    bool clearMissionFlag = *data % ENABLE;
    amsMgrScheduler->TerminateAbility(token, clearMissionFlag);
    sptr<IAppStateCallback> callback;
    amsMgrScheduler->RegisterAppStateCallback(callback);
    int32_t userId = static_cast<int32_t>(GetU32Data(data));
    amsMgrScheduler->KillProcessesByUserId(userId, false, nullptr);
    std::string bundleName(data, size);
    int accountId = static_cast<int>(GetU32Data(data));
    amsMgrScheduler->KillProcessWithAccount(bundleName, accountId);
    amsMgrScheduler->AbilityAttachTimeOut(token);
    amsMgrScheduler->PrepareTerminate(token);
    amsMgrScheduler->KillApplication(bundleName);
    amsMgrScheduler->VerifyKillProcessPermission(bundleName);
    int uid = static_cast<int>(GetU32Data(data));
    amsMgrScheduler->KillApplicationByUid(bundleName, uid);
    amsMgrScheduler->KillApplicationSelf();
    AppExecFwk::RunningProcessInfo info;
    amsMgrScheduler->GetRunningProcessInfoByToken(token, info);
    Parcel wantParcel;
    Want* want = nullptr;
    if (wantParcel.WriteBuffer(data, size)) {
        want = Want::Unmarshalling(wantParcel);
        if (!want) {
            return false;
        }
    }
    AbilityInfo abilityInfo;
    AbilityRuntime::StartSpecifiedParam param;
    amsMgrScheduler->StartSpecifiedAbility(*want, abilityInfo, param);
    int pid = static_cast<int>(GetU32Data(data));
    AppExecFwk::ApplicationInfo application;
    bool debug;
    amsMgrScheduler->GetApplicationInfoByProcessID(pid, application, debug);
    if (want) {
        delete want;
        want = nullptr;
    }
    bool enable = *data % ENABLE;
    amsMgrScheduler->SetKeepAliveAppService(bundleName, enable, uid);
    return amsMgrScheduler->IsReady();
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        std::cout << "invalid data" << std::endl;
        return 0;
    }

    /* Validate the length of size */
    if (size < OHOS::U32_AT_SIZE) {
        return 0;
    }

    char* ch = static_cast<char*>(malloc(size + 1));
    if (ch == nullptr) {
        std::cout << "malloc failed." << std::endl;
        return 0;
    }

    (void)memset_s(ch, size + 1, 0x00, size + 1);
    if (memcpy_s(ch, size, data, size) != EOK) {
        std::cout << "copy failed." << std::endl;
        free(ch);
        ch = nullptr;
        return 0;
    }

    OHOS::DoSomethingInterestingWithMyAPI(ch, size);
    free(ch);
    ch = nullptr;
    return 0;
}

