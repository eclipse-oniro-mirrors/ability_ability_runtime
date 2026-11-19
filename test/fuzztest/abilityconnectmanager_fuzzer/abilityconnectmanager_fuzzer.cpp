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

#include "abilityconnectmanager_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#define protected public
#include "ability_connect_manager.h"
#undef private
#undef protected

#include "ability_connect_callback_interface.h"
#include "ability_connect_callback_stub.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr int INPUT_ZERO = 0;
constexpr int INPUT_ONE = 1;
constexpr int INPUT_THREE = 3;
constexpr uint8_t ENABLE = 2;
constexpr size_t U32_AT_SIZE = 4;
constexpr size_t OFFSET_ZERO = 24;
constexpr size_t OFFSET_ONE = 16;
constexpr size_t OFFSET_TWO = 8;
class AbilityConnectCallback : public AbilityConnectionStub {
public:
    AbilityConnectCallback() = default;
    virtual ~AbilityConnectCallback() = default;
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName& element, const sptr<IRemoteObject>& remoteObject, int resultCode) override
    {}
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int resultCode) override
    {}
};
}

uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[INPUT_ZERO] << OFFSET_ZERO) | (ptr[INPUT_ONE] << OFFSET_ONE) | (ptr[ENABLE] << OFFSET_TWO) |
        ptr[INPUT_THREE];
}

std::shared_ptr<BaseExtensionRecord> GetFuzzAbilityRecord()
{
    sptr<Token> token = nullptr;
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.fuzzTest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<BaseExtensionRecord> abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest);
    if (!abilityRecord) {
        return nullptr;
    }
    return abilityRecord;
}

sptr<Token> GetFuzzAbilityToken()
{
    sptr<Token> token = nullptr;
    std::shared_ptr<BaseExtensionRecord> abilityRecord = GetFuzzAbilityRecord();
    if (abilityRecord) {
        token = abilityRecord->GetToken();
    }
    return token;
}

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    int intParam = static_cast<int>(GetU32Data(data));
    std::shared_ptr<AbilityConnectManager> abilityConnectManager = std::make_shared<AbilityConnectManager>(intParam);
    AbilityRequest abilityRequest;
    abilityConnectManager->StartAbility(abilityRequest);
    sptr<IRemoteObject> token = GetFuzzAbilityToken();
    abilityConnectManager->TerminateAbility(token);
    abilityConnectManager->StopServiceAbility(abilityRequest);
    abilityConnectManager->StartAbilityLocked(abilityRequest);
    abilityConnectManager->TerminateAbilityLocked(token);
    abilityConnectManager->StopServiceAbilityLocked(abilityRequest);
    bool boolParam = *data % ENABLE;
    std::shared_ptr<BaseExtensionRecord> targetService = GetFuzzAbilityRecord();
    const sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    std::list<std::shared_ptr<ConnectionRecord>> connectRecordList;
    abilityConnectManager->GetConnectRecordListFromMap(connect, connectRecordList);
    sptr<IRemoteObject> callerToken = GetFuzzAbilityToken();
    abilityConnectManager->ConnectAbilityLocked(abilityRequest, connect, callerToken);
    abilityConnectManager->DisconnectAbilityLocked(connect, false);
    sptr<IAbilityScheduler> scheduler = nullptr;
    abilityConnectManager->AttachAbilityThreadLocked(scheduler, token);
    AppInfo appInfo;
    abilityConnectManager->OnAppStateChanged(appInfo);
    abilityConnectManager->AbilityTransitionDone(token, intParam);
    sptr<IRemoteObject> remoteObject = nullptr;
    abilityConnectManager->ScheduleConnectAbilityDoneLocked(token, remoteObject);
    abilityConnectManager->ScheduleDisconnectAbilityDoneLocked(token);
    abilityConnectManager->ScheduleCommandAbilityDoneLocked(token);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = GetFuzzAbilityRecord();
    abilityConnectManager->CompleteCommandAbility(abilityRecord);
    std::string stringParam(data, size);
    abilityConnectManager->GetServiceRecordByElementName(stringParam);
    abilityConnectManager->GetExtensionByTokenFromServiceMap(token);
    sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    abilityConnectManager->GetConnectRecordListByCallback(callback);
    int64_t int64Param = static_cast<int64_t>(GetU32Data(data));
    abilityConnectManager->GetExtensionByIdFromServiceMap(int64Param);
    abilityConnectManager->LoadAbility(abilityRecord);
    uint32_t uint32Param = GetU32Data(data);
    abilityConnectManager->PostTimeOutTask(abilityRecord, uint32Param);
    abilityConnectManager->HandleStartTimeoutTask(abilityRecord);
    abilityConnectManager->HandleCommandTimeoutTask(abilityRecord);
    abilityConnectManager->HandleStopTimeoutTask(abilityRecord);
    AbilityConnectManager::ConnectListType connectlist;
    abilityConnectManager->HandleTerminateDisconnectTask(connectlist);
    abilityConnectManager->DispatchInactive(abilityRecord, intParam, token);
    abilityConnectManager->DispatchTerminate(abilityRecord, token);
    abilityConnectManager->ConnectAbility(abilityRecord);
    abilityConnectManager->CommandAbility(abilityRecord);
    abilityConnectManager->TerminateDone(abilityRecord);
    abilityConnectManager->GetAbilityConnectedRecordFromRecordList(abilityRecord, connectRecordList);
    std::shared_ptr<ConnectionRecord> connection;
    abilityConnectManager->RemoveConnectionRecordFromMap(connection);
    abilityConnectManager->RemoveServiceAbility(abilityRecord);
    abilityConnectManager->AddConnectDeathRecipient(connect->AsObject());
    abilityConnectManager->RemoveConnectDeathRecipient(connect->AsObject());
    sptr<IRemoteObject> connectRemoteObject = GetFuzzAbilityToken();
    wptr<IRemoteObject> remote = connectRemoteObject;
    abilityConnectManager->OnCallBackDied(remote);
    abilityConnectManager->HandleCallBackDiedTask(connectRemoteObject);
    int int32Param = static_cast<int32_t>(GetU32Data(data));
    abilityConnectManager->OnAbilityDied(abilityRecord);
    abilityConnectManager->OnTimeOut(uint32Param, int64Param);
    std::shared_ptr<BaseExtensionRecord> ability = GetFuzzAbilityRecord();
    abilityConnectManager->HandleInactiveTimeout(ability);
    abilityConnectManager->IsAbilityNeedKeepAlive(abilityRecord);
    abilityConnectManager->HandleAbilityDiedTask(abilityRecord);
    std::vector<std::string> info;
    AbilityConnectManager::ServiceMapType serviceMap;
    AbilityConnectManager::DumpState(serviceMap, info, boolParam, stringParam);
    AbilityConnectManager::DumpStateByUri(serviceMap, info, boolParam, stringParam, info);
    std::vector<ExtensionRunningInfo> extensionRunningInfo;
    AbilityConnectManager::GetExtensionRunningInfos(serviceMap, intParam, extensionRunningInfo, int32Param, boolParam);
    AbilityConnectManager::GetExtensionRunningInfo(abilityRecord, int32Param, extensionRunningInfo);
    abilityConnectManager->PauseExtensions();
    return true;
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

