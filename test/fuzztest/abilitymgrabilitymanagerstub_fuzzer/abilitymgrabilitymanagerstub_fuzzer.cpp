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

#include "abilitymgrabilitymanagerstub_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#include "ability_manager_service.h"
#include "ability_manager_stub.h"
#include "iconnection_observer.h"
#include "connection_data.h"
#include "want_sender_interface.h"
#include "iforeground_app_connection.h"
#undef private

#include "securec.h"
#include "ability_record.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr int INPUT_ZERO = 0;
constexpr int INPUT_ONE = 1;
constexpr int INPUT_TWO = 2;
constexpr int INPUT_THREE = 3;
constexpr size_t U32_AT_SIZE = 4;
constexpr size_t OFFSET_ZERO = 24;
constexpr size_t OFFSET_ONE = 16;
constexpr size_t OFFSET_TWO = 8;
constexpr uint8_t ENABLE = 2;

class IConnectionObserverFuzz : public IConnectionObserver {
public:
    void OnExtensionConnected(const ConnectionData &data) override {}
    void OnExtensionDisconnected(const ConnectionData &data) override {}
    void OnExtensionSuspended(const ConnectionData &data) override {}
    void OnExtensionResumed(const ConnectionData &data) override {}

#ifdef WITH_DLP
    void OnDlpAbilityOpened(const DlpStateData &data) override {}
    void OnDlpAbilityClosed(const DlpStateData &data) override {}
#endif

    int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) { return ERR_NONE; };

    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }
};

class WantSenderFuzz : public IWantSender {
public:
    void Send(SenderInfo &senderInfo) override {}

    int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) { return ERR_NONE; };

    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }
};

class ForegroundAppConnectionFuzz : public IForegroundAppConnection {
public:
    void OnForegroundAppConnected(const ForegroundAppConnectionData &data) override {}
    
    void OnForegroundAppDisconnected(const ForegroundAppConnectionData &data) override {}
    
    void OnForegroundAppCallerStarted(int32_t callerPid, int32_t callerUid, const std::string &bundleName) override {}

    int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) { return ERR_NONE; };

    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }
};
}
const std::u16string ABILITYMGR_INTERFACE_TOKEN = u"ohos.aafwk.AbilityManager";
uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[INPUT_ZERO] << OFFSET_ZERO) | (ptr[INPUT_ONE] << OFFSET_ONE) | (ptr[INPUT_TWO] << OFFSET_TWO) |
        ptr[INPUT_THREE];
}

sptr<Token> GetFuzzAbilityToken()
{
    sptr<Token> token = nullptr;
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.fuzzTest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (abilityRecord) {
        token = abilityRecord->GetToken();
    }
    return token;
}

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    uint32_t codeOne = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_TYPE);
    MessageParcel parcel;
    parcel.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    parcel.WriteBuffer(data, size);
    parcel.RewindRead(0);
    MessageParcel reply;
    MessageOption option;
    std::shared_ptr<AbilityManagerService> abmsOne = std::make_shared<AbilityManagerService>();
    abmsOne->OnRemoteRequest(codeOne, parcel, reply, option);

    uint32_t codeTwo = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_EXTENSION_ABILITY);
    MessageParcel parcels;
    parcels.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    parcels.WriteBuffer(data, size);
    parcels.RewindRead(0);
    std::shared_ptr<AbilityManagerService> abmsTwo = std::make_shared<AbilityManagerService>();
    abmsTwo->OnRemoteRequest(codeTwo, parcels, reply, option);

    std::shared_ptr<AbilityManagerService> abilityMgrStub = std::make_shared<AbilityManagerService>();
    sptr<AbilityRuntime::IConnectionObserver> observer = new IConnectionObserverFuzz();
    std::vector<AbilityRuntime::ConnectionData> infos;
    sptr<IWantSender> sender = new WantSenderFuzz();
    uint32_t flags = static_cast<uint32_t>(INPUT_ZERO);
    sptr<AbilityRuntime::IForegroundAppConnection> observers = new ForegroundAppConnectionFuzz();
    MessageParcel dataParcel;
    dataParcel.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    dataParcel.WriteBuffer(data, size);
    dataParcel.RewindRead(0);
    MessageParcel reply1;
    abilityMgrStub->StartUIExtensionPreViewEmbeddedInner(dataParcel, reply1);

    MessageParcel dataParcel1;
    dataParcel1.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    dataParcel1.WriteBuffer(data, size);
    dataParcel1.RewindRead(0);
    abilityMgrStub->GetAllIntentExemptionInfoInner(dataParcel1, reply1);

    MessageParcel dataParcel2;
    dataParcel2.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    dataParcel2.WriteBuffer(data, size);
    dataParcel2.RewindRead(0);
    abilityMgrStub->RegisterSnapshotHandlerInner(dataParcel2, reply1);

    MessageParcel dataParcel3;
    dataParcel3.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    dataParcel3.WriteBuffer(data, size);
    dataParcel3.RewindRead(0);
    abilityMgrStub->TerminateMissionInner(dataParcel3, reply1);

    MessageParcel dataParcel4;
    dataParcel4.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    dataParcel4.WriteBuffer(data, size);
    dataParcel4.RewindRead(0);
    abilityMgrStub->BlockAllAppStartInner(dataParcel4, reply1);

    MessageParcel dataParcel5;
    dataParcel5.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    dataParcel5.WriteBuffer(data, size);
    dataParcel5.RewindRead(0);
    abilityMgrStub->UpdateAssociateConfigListInner(dataParcel5, reply1);

    MessageParcel dataParcel6;
    dataParcel6.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    dataParcel6.WriteBuffer(data, size);
    dataParcel6.RewindRead(0);
    abilityMgrStub->SetApplicationKeepAliveInner(dataParcel6, reply1);

    MessageParcel dataParcel7;
    dataParcel7.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    dataParcel7.WriteBuffer(data, size);
    dataParcel7.RewindRead(0);
    abilityMgrStub->QueryKeepAliveApplicationsInner(dataParcel7, reply1);

    MessageParcel dataParcel8;
    dataParcel8.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    dataParcel8.WriteBuffer(data, size);
    dataParcel8.RewindRead(0);
    abilityMgrStub->AddQueryERMSObserverInner(dataParcel8, reply1);

    MessageParcel dataParcel9;
    dataParcel9.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    dataParcel9.WriteBuffer(data, size);
    dataParcel9.RewindRead(0);
    abilityMgrStub->QueryAtomicServiceStartupRuleInner(dataParcel9, reply1);

    MessageParcel dataParcel10;
    dataParcel10.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    dataParcel10.WriteBuffer(data, size);
    dataParcel10.RewindRead(0);
    abilityMgrStub->GetKioskStatusInner(dataParcel10, reply1);

    MessageParcel dataParcel11;
    dataParcel11.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    dataParcel11.WriteBuffer(data, size);
    dataParcel11.RewindRead(0);
    abilityMgrStub->RegisterSAInterceptorInner(dataParcel11, reply1);

    MessageParcel dataParcel11a;
    dataParcel11a.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    dataParcel11a.WriteBuffer(data, size);
    dataParcel11a.RewindRead(0);
    abilityMgrStub->UnregisterSAInterceptorInner(dataParcel11a, reply1);

    MessageParcel dataParcel12;
    dataParcel12.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    dataParcel12.WriteBuffer(data, size);
    dataParcel12.RewindRead(0);
    abilityMgrStub->PreloadApplicationInner(dataParcel12, reply1);

    MessageParcel dataParcel13;
    dataParcel13.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    dataParcel13.WriteBuffer(data, size);
    dataParcel13.RewindRead(0);
    abilityMgrStub->StartSelfUIAbilityInCurrentProcessInner(dataParcel13, reply1);

    MessageParcel dataParcel14;
    dataParcel14.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    dataParcel14.WriteBuffer(data, size);
    dataParcel14.RewindRead(0);
    abilityMgrStub->IsRestartAppLimitInner(dataParcel14, reply1);

    MessageParcel dataParcel15;
    dataParcel15.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    dataParcel15.WriteBuffer(data, size);
    dataParcel15.RewindRead(0);
    abilityMgrStub->UnPreloadUIExtensionAbilityInner(dataParcel15, reply1);

    MessageParcel dataParcel16;
    dataParcel16.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    dataParcel16.WriteBuffer(data, size);
    dataParcel16.RewindRead(0);
    abilityMgrStub->ClearAllPreloadUIExtensionAbilityInner(dataParcel16, reply1);

    MessageParcel dataParcel17;
    dataParcel17.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    dataParcel17.WriteBuffer(data, size);
    dataParcel17.RewindRead(0);
    abilityMgrStub->RegisterPreloadUIExtensionHostClientInner(dataParcel17, reply1);

    MessageParcel dataParcel18;
    dataParcel18.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    dataParcel18.WriteBuffer(data, size);
    dataParcel18.RewindRead(0);
    abilityMgrStub->UnRegisterPreloadUIExtensionHostClientInner(dataParcel18, reply1);

    MessageParcel dataParcels1;
    dataParcels1.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    dataParcels1.WriteBuffer(data, size);
    dataParcels1.RewindRead(0);
    abilityMgrStub->StartSelfUIAbilityWithStartOptionsInner(dataParcels1, reply1);

    MessageParcel dataParcels2;
    dataParcels2.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    dataParcels2.WriteBuffer(data, size);
    dataParcels2.RewindRead(0);
    abilityMgrStub->StartSelfUIAbilityWithPidResultInner(dataParcels2, reply1);

    MessageParcel dataParcels3;
    dataParcels3.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    dataParcels3.WriteBuffer(data, size);
    dataParcels3.RewindRead(0);
    abilityMgrStub->PrepareTerminateAbilityDoneInner(dataParcels3, reply1);

    MessageParcel dataParcels4;
    dataParcels4.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    dataParcels4.WriteBuffer(data, size);
    dataParcels4.RewindRead(0);
    abilityMgrStub->KillProcessWithPrepareTerminateDoneInner(dataParcels4, reply1);

    MessageParcel dataParcels5;
    dataParcels5.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    dataParcels5.WriteBuffer(data, size);
    dataParcels5.RewindRead(0);
    abilityMgrStub->RegisterHiddenStartObserverInner(dataParcels5, reply1);

    MessageParcel dataParcels6;
    dataParcels6.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    dataParcels6.WriteBuffer(data, size);
    dataParcels6.RewindRead(0);
    abilityMgrStub->UnregisterHiddenStartObserverInner(dataParcels6, reply1);

    MessageParcel dataParcels7;
    dataParcels7.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    dataParcels7.WriteBuffer(data, size);
    dataParcels7.RewindRead(0);
    abilityMgrStub->QueryPreLoadUIExtensionRecordInner(dataParcels7, reply1);

    MessageParcel dataParcels8;
    dataParcels8.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    dataParcels8.WriteBuffer(data, size);
    dataParcels8.RewindRead(0);
    abilityMgrStub->RevokeDelegatorInner(dataParcels8, reply1);

    MessageParcel dataParcels9;
    dataParcels9.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    dataParcels9.WriteBuffer(data, size);
    dataParcels9.RewindRead(0);
    abilityMgrStub->GetAllInsightIntentInfoInner(dataParcels9, reply1);

    MessageParcel dataParcels10;
    dataParcels10.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    dataParcels10.WriteBuffer(data, size);
    dataParcels10.RewindRead(0);
    abilityMgrStub->GetInsightIntentInfoByBundleNameInner(dataParcels10, reply1);

    MessageParcel dataParcels11;
    dataParcels11.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    dataParcels11.WriteBuffer(data, size);
    dataParcels11.RewindRead(0);
    abilityMgrStub->GetInsightIntentInfoByIntentNameInner(dataParcels11, reply1);

    MessageParcel dataParcels12;
    dataParcels12.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    dataParcels12.WriteBuffer(data, size);
    dataParcels12.RewindRead(0);
    abilityMgrStub->StartAbilityWithWaitInner(dataParcels12, reply1);

    MessageParcel dataParcels13;
    dataParcels13.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    dataParcels13.WriteBuffer(data, size);
    dataParcels13.RewindRead(0);
    abilityMgrStub->RestartSelfAtomicServiceInner(dataParcels13, reply1);

    MessageParcel dataParcels14;
    dataParcels14.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    dataParcels14.WriteBuffer(data, size);
    dataParcels14.RewindRead(0);
    abilityMgrStub->SetAppServiceExtensionKeepAliveInner(dataParcels14, reply1);

    MessageParcel dataParcels15;
    dataParcels15.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    dataParcels15.WriteBuffer(data, size);
    dataParcels15.RewindRead(0);
    abilityMgrStub->QueryKeepAliveAppServiceExtensionsInner(dataParcels15, reply1);

    MessageParcel dataParcels16;
    dataParcels16.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    dataParcels16.WriteBuffer(data, size);
    dataParcels16.RewindRead(0);
    abilityMgrStub->UpdateKioskApplicationListInner(dataParcels16, reply1);

    MessageParcel dataParcels17;
    dataParcels17.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    dataParcels17.WriteBuffer(data, size);
    dataParcels17.RewindRead(0);
    abilityMgrStub->EnterKioskModeInner(dataParcels17, reply1);

    MessageParcel dataParcels18;
    dataParcels18.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    dataParcels18.WriteBuffer(data, size);
    dataParcels18.RewindRead(0);
    abilityMgrStub->ExitKioskModeInner(dataParcels18, reply1);

    abilityMgrStub->RegisterObserver(observer);
    abilityMgrStub->UnregisterObserver(observer);
    abilityMgrStub->GetConnectionData(infos);
    abilityMgrStub->CancelWantSenderByFlags(sender, flags);
    abilityMgrStub->RegisterForegroundAppObserver(observers);
    abilityMgrStub->UnregisterForegroundAppObserver(observers);
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
    if (memcpy_s(ch, size + 1, data, size) != EOK) {
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

