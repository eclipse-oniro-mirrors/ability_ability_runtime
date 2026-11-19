/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#ifndef UNITTEST_OHOS_ABILITY_RUNTIME_MOCK_DATA_OBS_MGR_STUB_H
#define UNITTEST_OHOS_ABILITY_RUNTIME_MOCK_DATA_OBS_MGR_STUB_H
#include <memory>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "dataobs_mgr_interface.h"
#include "dataobs_mgr_stub.h"
#include "data_ability_observer_stub.h"

#define TEST_RETVAL_ONREMOTEREQUEST 1000

namespace OHOS {
int IPCObjectStub::SendRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    reply.WriteInt32(TEST_RETVAL_ONREMOTEREQUEST);
    return NO_ERROR;
}
} // namespace OHOS

namespace OHOS {
namespace AAFwk {
class MockDataObsMgrStub : public DataObsManagerStub {
public:
    MOCK_METHOD4(RegisterObserver, int(const Uri&, sptr<IDataAbilityObserver>, int32_t userId, DataObsOption));
    MOCK_METHOD4(UnregisterObserver, int(const Uri&, sptr<IDataAbilityObserver>, int32_t userId, DataObsOption));
    MOCK_METHOD3(NotifyChange, int(const Uri&, int32_t userId, DataObsOption));

    MOCK_METHOD4(RegisterObserverExt, Status(const Uri&, sptr<IDataAbilityObserver>, bool, DataObsOption));
    MOCK_METHOD3(UnregisterObserverExt, Status(const Uri&, sptr<IDataAbilityObserver>, DataObsOption));
    MOCK_METHOD2(UnregisterObserverExt, Status(sptr<IDataAbilityObserver>, DataObsOption));
    MOCK_METHOD2(NotifyChangeExt, Status(const ChangeInfo&, DataObsOption));
    MOCK_METHOD2(NotifyProcessObserver, Status(const std::string&, const sptr<IRemoteObject>));
    MOCK_METHOD4(RegisterObserverFromExtension, int(const Uri&, sptr<IDataAbilityObserver>,
        int32_t userId, DataObsOption opt));
    MOCK_METHOD3(NotifyChangeFromExtension, int(const Uri&, int32_t userId, DataObsOption opt));
};

class MockDataAbilityObserverStub : public AAFwk::DataAbilityObserverStub {
public:
    MockDataAbilityObserverStub() = default;
    virtual ~MockDataAbilityObserverStub() = default;
    MOCK_METHOD0(OnChange, void(void));
    MOCK_METHOD1(OnChangeExt, void(const ChangeInfo&));
};
}  // namespace AAFwk
}  // namespace OHOS
#endif /* UNITTEST_OHOS_ABILITY_RUNTIME_MOCK_DATA_OBS_MGR_STUB_H */
