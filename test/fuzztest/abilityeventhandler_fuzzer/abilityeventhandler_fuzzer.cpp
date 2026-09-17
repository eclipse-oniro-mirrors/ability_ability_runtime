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

#include "abilityeventhandler_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#include "ability_event_handler.h"
#include "interceptor/ability_interceptor_executer.h"
#include "ability_running_info.h"
#include "ability_scheduler_proxy.h"
#include "ams_configuration_parameter.h"
#undef private

#include "ability_record.h"
#include "data_ability_observer_interface.h"
#include "data_ability_predicates.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr int INPUT_ZERO = 0;
constexpr int INPUT_ONE = 1;
constexpr int INPUT_THREE = 3;
constexpr size_t U32_AT_SIZE = 4;
constexpr uint8_t ENABLE = 2;
constexpr int OFFSET_ZERO = 24;
constexpr int OFFSET_ONE = 16;
constexpr int OFFSET_TWO = 8;
class DataAbilityObserver : public IDataAbilityObserver {
public:
    DataAbilityObserver() = default;
    virtual ~DataAbilityObserver() = default;
    void OnChange() override
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
    bool boolParam = *data % ENABLE;
    int intParam = static_cast<int>(GetU32Data(data));
    int32_t int32Param = static_cast<int32_t>(GetU32Data(data));
    int64_t int64Param = static_cast<int64_t>(GetU32Data(data));
    uint32_t uint32Param = GetU32Data(data);
    std::string stringParam(data, size);

    // fuzz for AbilityEventHandler
    std::shared_ptr<TaskHandlerWrap> runner;
    std::weak_ptr<AbilityManagerService> server;
    auto abilityEventHandler = std::make_shared<AbilityEventHandler>(runner, server);
    abilityEventHandler->ProcessLoadTimeOut(int64Param);
    abilityEventHandler->ProcessActiveTimeOut(int64Param);
    abilityEventHandler->ProcessInactiveTimeOut(int64Param);
    abilityEventHandler->ProcessForegroundTimeOut(int64Param);

    // fuzz for AbilityInterceptorExecuter
    auto abilityInterceptorExecuter = std::make_shared<AbilityInterceptorExecuter>();
    std::shared_ptr<AbilityInterceptor> interceptor;
    abilityInterceptorExecuter->AddInterceptor(interceptor);
    Parcel wantParcel;
    Want* want = nullptr;
    if (wantParcel.WriteBuffer(data, size)) {
        want = Want::Unmarshalling(wantParcel);
        if (!want) {
            return false;
        }
    }
    
    AbilityInterceptorParam interceptorParam = InterceptorParamBuilder(*want, intParam, int32Param)
        .WithUI(boolParam).CallerToken(nullptr).Build();
    abilityInterceptorExecuter->DoProcess(interceptorParam);

    // fuzz for AbilityRunningInfo
    auto abilityRunningInfo = std::make_shared<AbilityRunningInfo>();
    Parcel parcel;
    abilityRunningInfo->ReadFromParcel(parcel);
    abilityRunningInfo->Unmarshalling(parcel);
    abilityRunningInfo->Marshalling(parcel);

    // fuzz for AbilitySchedulerProxy
    sptr<IRemoteObject> impl = GetFuzzAbilityToken();
    auto abilitySchedulerProxy = std::make_shared<AbilitySchedulerProxy>(impl);
    MessageParcel messageParcel;
    abilitySchedulerProxy->WriteInterfaceToken(messageParcel);
    LifeCycleStateInfo stateInfo;
    abilitySchedulerProxy->ScheduleAbilityTransaction(*want, stateInfo);
    abilitySchedulerProxy->SendResult(intParam, intParam, *want);
    abilitySchedulerProxy->ScheduleConnectAbility(*want);
    abilitySchedulerProxy->ScheduleDisconnectAbility(*want);
    abilitySchedulerProxy->ScheduleCommandAbility(*want, boolParam, intParam);
    abilitySchedulerProxy->ScheduleSaveAbilityState();
    PacMap inState;
    abilitySchedulerProxy->ScheduleRestoreAbilityState(inState);
    Uri uri(stringParam);
    abilitySchedulerProxy->GetFileTypes(uri, stringParam);
    abilitySchedulerProxy->OpenFile(uri, stringParam);
    abilitySchedulerProxy->OpenRawFile(uri, stringParam);
    PacMap pacMap;
    abilitySchedulerProxy->Call(uri, stringParam, stringParam, pacMap);
    NativeRdb::DataAbilityPredicates predicates;
    abilitySchedulerProxy->Delete(uri, predicates);
    std::vector<std::string> columns;
    abilitySchedulerProxy->Query(uri, columns, predicates);
    abilitySchedulerProxy->GetType(uri);
    PacMap extras;
    abilitySchedulerProxy->Reload(uri, extras);
    sptr<IDataAbilityObserver> dataObserver(new DataAbilityObserver());
    abilitySchedulerProxy->ScheduleRegisterObserver(uri, dataObserver);
    abilitySchedulerProxy->ScheduleUnregisterObserver(uri, dataObserver);
    abilitySchedulerProxy->ScheduleNotifyChange(uri);
    abilitySchedulerProxy->NormalizeUri(uri);
    abilitySchedulerProxy->DenormalizeUri(uri);
    std::vector<std::shared_ptr<AppExecFwk::DataAbilityOperation>> operations;
    abilitySchedulerProxy->ExecuteBatch(operations);
    abilitySchedulerProxy->ContinueAbility(stringParam, uint32Param);
    abilitySchedulerProxy->NotifyContinuationResult(int32Param);
    std::vector<std::string> stringVector;
    abilitySchedulerProxy->DumpAbilityInfo(stringVector, stringVector);
    abilitySchedulerProxy->CallRequest();
    // fuzz for AmsConfigurationParameter
    AmsConfigurationParameter::GetInstance().NonConfigFile();
    AmsConfigurationParameter::GetInstance().GetMissionSaveTime();
    AmsConfigurationParameter::GetInstance().GetOrientation();
    AmsConfigurationParameter::GetInstance().GetANRTimeOutTime();
    AmsConfigurationParameter::GetInstance().GetAMSTimeOutTime();
    AmsConfigurationParameter::GetInstance().GetMaxRestartNum(true);
    AmsConfigurationParameter::GetInstance().GetDeviceType();
    AmsConfigurationParameter::GetInstance().GetBootAnimationTimeoutTime();
    nlohmann::json Object;
    AmsConfigurationParameter::GetInstance().LoadAppConfigurationForStartUpService(Object);
    AmsConfigurationParameter::GetInstance().LoadAppConfigurationForMemoryThreshold(Object);
    AmsConfigurationParameter::GetInstance().LoadSystemConfiguration(Object);

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

