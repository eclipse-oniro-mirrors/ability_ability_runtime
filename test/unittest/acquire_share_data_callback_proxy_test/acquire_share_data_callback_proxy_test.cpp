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

#include <gtest/gtest.h>

#include "acquire_share_data_callback_proxy.h"
#include "ability_manager_errors.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AAFwk {
class AcquireShareDataCallbackProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AcquireShareDataCallbackProxyTest::SetUpTestCase(void)
{}

void AcquireShareDataCallbackProxyTest::TearDownTestCase(void)
{}

void AcquireShareDataCallbackProxyTest::SetUp()
{}

void AcquireShareDataCallbackProxyTest::TearDown()
{}

class IRemoteObjectMocker : public IRemoteObject {
public:
    IRemoteObjectMocker() : IRemoteObject {u"IRemoteObjectMocker"}
    {}

    ~IRemoteObjectMocker()
    {}

    int32_t GetObjectRefCount()
    {
        return 0;
    }

    int SendRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
    {
        return 0;
    }

    bool IsProxyObject() const
    {
        return true;
    }

    bool CheckObjectLegality() const
    {
        return true;
    }

    bool AddDeathRecipient(const sptr<DeathRecipient>& recipient)
    {
        return true;
    }

    bool RemoveDeathRecipient(const sptr<DeathRecipient>& recipient)
    {
        return true;
    }

    bool AddRefreshRecipient(const sptr<RefreshRecipient> &recipient)
    {
        return true;
    }

    bool RemoveRefreshRecipient(const sptr<RefreshRecipient> &recipient)
    {
        return true;
    }

    sptr<IRemoteBroker> AsInterface()
    {
        return nullptr;
    }

    int Dump(int fd, const std::vector<std::u16string>& args)
    {
        return 0;
    }
};

/*
 * Feature: AcquireShareDataCallbackProxy
 * Function: AcquireShareDataCallbackProxy
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: verify function AcquireShareDataDone
 */
HWTEST_F(AcquireShareDataCallbackProxyTest, acquire_share_data_callback_proxy_operating_001, TestSize.Level1)
{
    OHOS::sptr<OHOS::IRemoteObject> impl = nullptr;
    sptr<AcquireShareDataCallbackProxy> acquireShareDataCallbackProxy = new AcquireShareDataCallbackProxy(impl);
    WantParams wantParam;
    int32_t ret = acquireShareDataCallbackProxy->AcquireShareDataDone(1, wantParam);
    EXPECT_EQ(ret, INNER_ERR);
}

/*
 * Feature: AcquireShareDataCallbackProxy
 * Function: AcquireShareDataCallbackProxy
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: verify function AcquireShareDataDone
 */
HWTEST_F(AcquireShareDataCallbackProxyTest, acquire_share_data_callback_proxy_operating_002, TestSize.Level1)
{
    OHOS::sptr<OHOS::IRemoteObject> impl = new IRemoteObjectMocker();
    sptr<AcquireShareDataCallbackProxy> acquireShareDataCallbackProxy = new AcquireShareDataCallbackProxy(impl);
    WantParams wantParam;
    int32_t ret = acquireShareDataCallbackProxy->AcquireShareDataDone(1, wantParam);
    EXPECT_EQ(ret, ERR_OK);
}
}  // namespace AAFwk
}  // namespace OHOS
