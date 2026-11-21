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

#include "connectionstatemanager_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#define protected public
#include "connection_observer_controller.h"
#include "connection_record.h"
#include "connection_state_item.h"
#include "connection_state_manager.h"
#undef protected
#undef private

#include "base_extension_record.h"
#include "continuous_task_callback_info.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace {
constexpr size_t U32_AT_SIZE = 4;
constexpr uint8_t ENABLE = 2;
class MyAbilityConnection : public IAbilityConnection {
public:
    MyAbilityConnection() = default;
    virtual ~MyAbilityConnection() = default;
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName& element, const sptr<IRemoteObject>& remoteObject, int resultCode) override
    {}
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int resultCode) override
    {}
    sptr<IRemoteObject> AsObject() override
    {
        return {};
    }
};
class MyAbilityConnectionObserver : public IConnectionObserver {
public:
    MyAbilityConnectionObserver() = default;
    virtual ~MyAbilityConnectionObserver() = default;
    void OnExtensionConnected(const ConnectionData& data) override
    {}
    void OnExtensionDisconnected(const ConnectionData& data) override
    {}
    void OnExtensionSuspended(const ConnectionData &data) override
    {}
    void OnExtensionResumed(const ConnectionData &data) override
    {}
#ifdef WITH_DLP
    void OnDlpAbilityOpened(const DlpStateData& data) override
    {}
    void OnDlpAbilityClosed(const DlpStateData& data) override
    {}
#endif // WITH_DLP
    sptr<IRemoteObject> AsObject() override
    {
        return {};
    }
};
}

uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];
}

std::shared_ptr<BaseExtensionRecord> GetFuzzExtensionRecord()
{
    sptr<Token> token = nullptr;
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.fuzzTest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::EXTENSION;
    std::shared_ptr<BaseExtensionRecord> abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest);
    if (!abilityRecord) {
        return nullptr;
    }
    return abilityRecord;
}

std::shared_ptr<AbilityRecord> GetFuzzAbilityRecord()
{
    sptr<Token> token = nullptr;
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.fuzzTest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (!abilityRecord) {
        return nullptr;
    }
    return abilityRecord;
}

sptr<Token> GetFuzzAbilityToken()
{
    sptr<Token> token = nullptr;
    std::shared_ptr<AbilityRecord> abilityRecord = GetFuzzAbilityRecord();
    if (abilityRecord) {
        token = abilityRecord->GetToken();
    }
    return token;
}

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    bool boolParam = *data % ENABLE;
    int intParam = static_cast<int>(GetU32Data(data));
    int32_t int32Param = static_cast<int32_t>(GetU32Data(data));
    std::string stringParam(data, size);
    Parcel wantParcel;
    Want* want = nullptr;
    if (wantParcel.WriteBuffer(data, size)) {
        want = Want::Unmarshalling(wantParcel);
        if (!want) {
            return false;
        }
    }
    sptr<IRemoteObject> token = GetFuzzAbilityToken();
    sptr<IAbilityConnection> connect = new MyAbilityConnection();
    sptr<AbilityRuntime::IConnectionObserver> observer = new MyAbilityConnectionObserver();
    std::vector<std::string> info;
    AbilityRuntime::ConnectionData connectionData;
    ConnectionEvent connectionEvent;

    // fuzz for ConnectionObserverController
    auto connectionObserverController = std::make_shared<ConnectionObserverController>();
    connectionObserverController->AddObserver(observer);
    connectionObserverController->RemoveObserver(observer);
    connectionObserverController->NotifyExtensionConnected(connectionData);
    connectionObserverController->NotifyExtensionDisconnected(connectionData);
#ifdef WITH_DLP
    AbilityRuntime::DlpStateData dlpStateData;
    connectionObserverController->NotifyDlpAbilityOpened(dlpStateData);
    connectionObserverController->NotifyDlpAbilityClosed(dlpStateData);
#endif // WITH_DLP
    connectionObserverController->GetObservers();
    wptr<IRemoteObject> remote;
    connectionObserverController->HandleRemoteDied(remote);
    ConnectionObserverController::ObserverDeathRecipient::ObserverDeathHandler handler;
    std::shared_ptr<ConnectionObserverController::ObserverDeathRecipient> observerDeathRecipient =
        std::make_shared<ConnectionObserverController::ObserverDeathRecipient>(handler);
    observerDeathRecipient->OnRemoteDied(remote);

    // fuzz for ConnectionRecord
    std::shared_ptr<BaseExtensionRecord> targetService = GetFuzzExtensionRecord();
    auto connectionRecord = std::make_shared<ConnectionRecord>(token, targetService, connect, nullptr);
    connectionRecord->CreateConnectionRecord(token, targetService, connect, nullptr);
    ConnectionState state = ConnectionState::CONNECTED;
    connectionRecord->SetConnectState(state);
    connectionRecord->GetConnectState();
    connectionRecord->GetToken();
    connectionRecord->GetAbilityRecord();
    connectionRecord->GetAbilityConnectCallback();
    connectionRecord->ClearConnCallBack();
    connectionRecord->DisconnectAbility();
    connectionRecord->CompleteConnect();
    connectionRecord->CompleteDisconnect(intParam, boolParam);
    connectionRecord->ScheduleDisconnectAbilityDone();
    connectionRecord->ScheduleConnectAbilityDone();
    connectionRecord->DisconnectTimeout();
    connectionRecord->ConvertConnectionState(state);
    connectionRecord->Dump(info);
    connectionRecord->AttachCallerInfo();
    connectionRecord->GetCallerUid();
    connectionRecord->GetCallerPid();
    connectionRecord->GetCallerName();
    connectionRecord->GetTargetToken();
    connectionRecord->GetConnection();

    // fuzz for ConnectionRecord
    auto connectionStateItem = std::make_shared<ConnectionStateItem>(int32Param, int32Param, stringParam);
    std::shared_ptr<ConnectionRecord> record;
    connectionStateItem->CreateConnectionStateItem(record);
    DataAbilityCaller dataCaller;
    connectionStateItem->CreateConnectionStateItem(dataCaller);
    connectionStateItem->AddConnection(record, connectionData, connectionEvent);
    connectionStateItem->RemoveConnection(record, connectionData, connectionEvent);
    std::shared_ptr<DataAbilityRecord> dataAbility;
    connectionStateItem->AddDataAbilityConnection(dataCaller, dataAbility, connectionData);
    connectionStateItem->RemoveDataAbilityConnection(dataCaller, dataAbility, connectionData);
    connectionStateItem->HandleDataAbilityDied(token, connectionData);
    connectionStateItem->IsEmpty();
    std::vector<AbilityRuntime::ConnectionData> datas;
    connectionStateItem->GenerateAllConnectionData(datas);
    std::shared_ptr<ConnectedExtension> connectedExtension;
    connectionStateItem->GenerateConnectionData(connectedExtension, connectionData);
    std::shared_ptr<ConnectedDataAbility> connectedDataAbility;
    connectionStateItem->GenerateConnectionData(connectedDataAbility, connectionData);

    // fuzz for ConnectionStateManager
    auto connectionStateManager = std::make_shared<ConnectionStateManager>();
    connectionStateManager->Init();
    connectionStateManager->RegisterObserver(observer);
    connectionStateManager->UnregisterObserver(observer);
    connectionStateManager->AddConnection(record);
    connectionStateManager->RemoveConnection(record, boolParam);
    connectionStateManager->AddDataAbilityConnection(dataCaller, dataAbility);
    connectionStateManager->RemoveDataAbilityConnection(dataCaller, dataAbility);
    connectionStateManager->CheckDataAbilityConnectionParams(dataCaller, dataAbility);
    connectionStateManager->HandleDataAbilityDied(dataAbility);
    connectionStateManager->HandleDataAbilityCallerDied(int32Param);
#ifdef WITH_DLP
    std::shared_ptr<AbilityRecord> dlpManger = GetFuzzAbilityRecord();
    connectionStateManager->AddDlpManager(dlpManger);
    connectionStateManager->RemoveDlpManager(dlpManger);
    connectionStateManager->AddDlpAbility(dlpManger);
    connectionStateManager->RemoveDlpAbility(dlpManger);
#endif // WITH_DLP
    connectionStateManager->HandleAppDied(int32Param);
#ifdef WITH_DLP
    std::vector<AbilityRuntime::DlpConnectionInfo> infos;
    connectionStateManager->GetDlpConnectionInfos(infos);
#endif // WITH_DLP
    connectionStateManager->AddConnectionInner(connectionRecord, connectionData, connectionEvent);
    connectionStateManager->RemoveConnectionInner(connectionRecord, connectionData, connectionEvent);
    connectionStateManager->HandleCallerDied(int32Param);
    connectionStateManager->RemoveDiedCaller(int32Param);
    connectionStateManager->AddDataAbilityConnectionInner(dataCaller, dataAbility, connectionData);
    connectionStateManager->RemoveDataAbilityConnectionInner(dataCaller, dataAbility, connectionData);
    connectionStateManager->HandleDataAbilityDiedInner(token, datas);
#ifdef WITH_DLP
    AbilityRuntime::DlpStateData dlpData;
    connectionStateManager->HandleDlpAbilityInner(dlpManger, boolParam, dlpData);
#endif // WITH_DLP
    connectionStateManager->InitAppStateObserver();
    if (want) {
        delete want;
        want = nullptr;
    }
    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
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

