/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "applicationanrlistener_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#define protected public
#include "ability_manager_service.h"
#include "application_anr_listener.h"
#include "atomic_service_status_callback.h"
#include "atomic_service_status_callback_proxy.h"
#include "background_task_observer.h"
#include "call_container.h"
#include "call_record.h"
#include "free_install_manager.h"
#undef protected
#undef private

#include "ability_record.h"
#include "continuous_task_callback_info.h"

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
}

uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[INPUT_ZERO] << OFFSET_ZERO) | (ptr[INPUT_ONE] << OFFSET_ONE) | (ptr[ENABLE] << OFFSET_TWO) |
        ptr[INPUT_THREE];
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
    int intParam = static_cast<int>(GetU32Data(data));
    int32_t int32Param = static_cast<int32_t>(GetU32Data(data));
    int64_t int64Param = static_cast<int64_t>(GetU32Data(data));
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
    std::vector<std::string> info;
    std::shared_ptr<AbilityRecord> targetService = GetFuzzAbilityRecord();

    // fuzz for ApplicationAnrListener
    auto applicationAnrListener = std::make_shared<ApplicationAnrListener>();
    applicationAnrListener->OnAnr(int32Param, 0);

    // fuzz for FreeInstallManager
    auto abilityms = std::make_shared<AbilityManagerService>();
    auto freeInstallManager = std::make_shared<FreeInstallManager>();
    freeInstallManager->IsTopAbility(token);
    freeInstallManager->StartFreeInstall(*want, int32Param, intParam, token);
    freeInstallManager->RemoteFreeInstall(*want, int32Param, intParam, token);
    freeInstallManager->BuildFreeInstallInfo(*want, int32Param, intParam, token);
    freeInstallManager->StartRemoteFreeInstall(*want, intParam, int32Param, token);
    freeInstallManager->NotifyDmsCallback(*want, intParam);
    freeInstallManager->NotifyFreeInstallResult(*want, intParam);
    freeInstallManager->FreeInstallAbilityFromRemote(*want, token, int32Param, intParam);
    freeInstallManager->ConnectFreeInstall(
        *want, int32Param, token, stringParam, AppExecFwk::ExtensionAbilityType::SERVICE);
    freeInstallManager->OnInstallFinished(intParam, *want, int32Param, int64Param);
    freeInstallManager->OnRemoteInstallFinished(intParam, *want, int32Param);

    // fuzz for AtomicServiceStatusCallback
    std::weak_ptr<FreeInstallManager> fimWeakPtr{ freeInstallManager };
    auto atomicServiceStatusCallback = std::make_shared<AtomicServiceStatusCallback>(fimWeakPtr, int64Param);
    atomicServiceStatusCallback->OnInstallFinished(intParam, *want, int32Param);
    atomicServiceStatusCallback->OnRemoteInstallFinished(intParam, *want, int32Param);

    // fuzz for AtomicServiceStatusCallbackProxy
    sptr<IRemoteObject> impl = GetFuzzAbilityToken();
    auto atomicServiceStatusCallbackProxy = std::make_shared<AtomicServiceStatusCallbackProxy>(impl);
    atomicServiceStatusCallbackProxy->OnInstallFinished(intParam, *want, int32Param);
    atomicServiceStatusCallbackProxy->OnRemoteInstallFinished(intParam, *want, int32Param);

    // fuzz for BackgroundTaskObserver
    auto backgroundTaskObserver = std::make_shared<BackgroundTaskObserver>();
    auto continuousTaskCallbackInfo = std::make_shared<BackgroundTaskMgr::ContinuousTaskCallbackInfo>();
    backgroundTaskObserver->OnContinuousTaskStart(continuousTaskCallbackInfo);
    backgroundTaskObserver->OnContinuousTaskStop(continuousTaskCallbackInfo);
    wptr<IRemoteObject> object{ token };

    // fuzz for CallRecord
    auto callRecord = std::make_shared<CallRecord>(int32Param, targetService, connect, token);
    callRecord->CreateCallRecord(int32Param, targetService, connect, token);
    callRecord->SetCallStub(token);
    callRecord->GetCallStub();
    callRecord->SetConCallBack(connect);
    callRecord->GetConCallBack();
    callRecord->GetTargetServiceName();
    callRecord->GetCallerToken();
    callRecord->SchedulerConnectDone();
    callRecord->SchedulerDisconnectDone();
    callRecord->OnCallStubDied(object);
    callRecord->Dump(info);
    callRecord->GetCallerUid();
    CallState state = CallState::INIT;
    callRecord->IsCallState(state);
    callRecord->SetCallState(state);
    callRecord->GetCallRecordId();

    // fuzz for CallContainer
    auto callContainer = std::make_shared<CallContainer>();
    callContainer->AddCallRecord(connect, callRecord);
    callContainer->GetCallRecord(connect);
    callContainer->RemoveCallRecord(connect);
    callContainer->OnConnectionDied(object);
    callContainer->CallRequestDone(token);
    callContainer->Dump(info);
    callContainer->IsNeedToCallRequest();
    callContainer->AddConnectDeathRecipient(connect);
    callContainer->RemoveConnectDeathRecipient(connect);

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

