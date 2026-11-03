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

#include "abilityschedulerstub_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "ability_scheduler_stub.h"
#include "message_parcel.h"
#include "securec.h"

using namespace OHOS::AAFwk;

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
const std::u16string ABILITYMGR_INTERFACE_TOKEN = u"ohos.aafwk.AbilityScheduler";
}
class AbilitySchedulerStubFuzzTest : public AbilitySchedulerStub {
public:
    AbilitySchedulerStubFuzzTest() = default;
    virtual ~AbilitySchedulerStubFuzzTest()
    {};
    bool ScheduleAbilityTransaction(const Want& want, const LifeCycleStateInfo& targetState,
        sptr<SessionInfo> sessionInfo = nullptr) override
    {
        return true;
    }
    void ScheduleShareData(const int32_t &uniqueId) override
    {}
    void SendResult(int requestCode, int resultCode, const Want& resultWant) override
    {}
    void ScheduleConnectAbility(const Want& want) override
    {}
    void ScheduleDisconnectAbility(const Want& want) override
    {}
    void ScheduleCommandAbility(const Want& want, bool restart, int startId) override
    {}
    void ScheduleCommandAbilityWindow(const Want &want, const sptr<SessionInfo> &sessionInfo,
        WindowCommand winCmd) override
    {}
    bool SchedulePrepareTerminateAbility() override
    {
        return true;
    }
    void ScheduleSaveAbilityState() override
    {}
    void ScheduleRestoreAbilityState(const PacMap& inState) override
    {}
    std::vector<std::string> GetFileTypes(const Uri& uri, const std::string& mimeTypeFilter) override
    {
        return {};
    }
    int OpenFile(const Uri& uri, const std::string& mode) override
    {
        return 0;
    }
    int OpenRawFile(const Uri& uri, const std::string& mode) override
    {
        return 0;
    }
    int Insert(const Uri& uri, const NativeRdb::ValuesBucket& value) override
    {
        return 0;
    }
    int Update(const Uri& uri, const NativeRdb::ValuesBucket& value,
        const NativeRdb::DataAbilityPredicates& predicates) override
    {
        return 0;
    }
    int Delete(const Uri& uri, const NativeRdb::DataAbilityPredicates& predicates) override
    {
        return 0;
    }
    std::shared_ptr<AppExecFwk::PacMap> Call(
        const Uri& uri, const std::string& method, const std::string& arg, const AppExecFwk::PacMap& pacMap) override
    {
        return {};
    }
    std::shared_ptr<NativeRdb::AbsSharedResultSet> Query(const Uri& uri,
        std::vector<std::string>& columns, const NativeRdb::DataAbilityPredicates& predicates) override
    {
        return {};
    }
    std::string GetType(const Uri& uri) override
    {
        return {};
    }
    bool Reload(const Uri& uri, const PacMap& extras) override
    {
        return true;
    }
    int BatchInsert(const Uri& uri, const std::vector<NativeRdb::ValuesBucket>& values) override
    {
        return 0;
    }
    bool ScheduleRegisterObserver(const Uri& uri, const sptr<IDataAbilityObserver>& dataObserver) override
    {
        return true;
    }
    bool ScheduleUnregisterObserver(const Uri& uri, const sptr<IDataAbilityObserver>& dataObserver) override
    {
        return true;
    }
    bool ScheduleNotifyChange(const Uri& uri) override
    {
        return true;
    }
    Uri NormalizeUri(const Uri& uri) override
    {
        return Uri{ "abilityschedulerstub" };
    }

    Uri DenormalizeUri(const Uri& uri) override
    {
        return Uri{ "abilityschedulerstub" };
    }
    std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>> ExecuteBatch(
        const std::vector<std::shared_ptr<AppExecFwk::DataAbilityOperation>>& operations) override
    {
        return {};
    }
    void ContinueAbility(const std::string& deviceId, uint32_t versionCode) override
    {}
    void NotifyContinuationResult(int32_t result) override
    {}
    void DumpAbilityInfo(const std::vector<std::string>& params, std::vector<std::string>& info) override
    {}
    void CallRequest() override
    {
        return;
    }
    void OnExecuteIntent(const Want &want) override
    {}

    int CreateModalUIExtension(const Want &want) override
    {
        return 0;
    }

    void UpdateSessionToken(sptr<IRemoteObject> sessionToken) override {}

    void ScheduleCollaborate(const Want &want) override {}

    void ScheduleAbilityRequestFailure(const std::string &requestId, const AppExecFwk::ElementName &element,
        const std::string &message, int32_t resultCode) override {}

    void ScheduleAbilityRequestSuccess(const std::string &requestId,
        const AppExecFwk::ElementName &element) override {}

    void ScheduleAbilitiesRequestDone(const std::string &requestKey, int32_t resultCode) override {}
};

uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[INPUT_ZERO] << OFFSET_ZERO) | (ptr[INPUT_ONE] << OFFSET_ONE) | (ptr[INPUT_TWO] << OFFSET_TWO) |
        ptr[INPUT_THREE];
}

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    uint32_t code = GetU32Data(data);

    MessageParcel parcel;
    parcel.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    parcel.WriteBuffer(data, size);
    parcel.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    std::shared_ptr<AbilitySchedulerStub> abilityschedulerstub = std::make_shared<AbilitySchedulerStubFuzzTest>();

    if (abilityschedulerstub->OnRemoteRequest(code, parcel, reply, option) != 0) {
        return false;
    }

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

