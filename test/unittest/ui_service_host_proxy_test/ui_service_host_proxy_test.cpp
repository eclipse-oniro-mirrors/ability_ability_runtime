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
#include <gtest/hwext/gtest-multithread.h>
#include <gmock/gmock.h>

#include "ui_service_host_proxy.h"
#include "ui_service_extension_module_loader.h"
#include "js_ui_service_extension_context.cpp"

#include "mock_ability_token.h"
#include "ability_handler.h"
#include "ohos_application.h"
#include "runtime.h"
#include "ui_service_extension_connection_constants.h"
#include "ui_service_proxy.h"

using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class UIServiceHostProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::unique_ptr<Runtime> runtime;
};

void UIServiceHostProxyTest::SetUpTestCase()
{}

void UIServiceHostProxyTest::TearDownTestCase()
{}

void UIServiceHostProxyTest::SetUp()
{}

void UIServiceHostProxyTest::TearDown()
{}


class IRemoteObjectMocker : public IRemoteObject {
public:
    IRemoteObjectMocker() : IRemoteObject {u"IRemoteObjectMocker"}
    {
    }

    ~IRemoteObjectMocker()
    {
    }

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

/**
 * @tc.number: SendData_0100
 * @tc.name: SendData
 * @tc.desc: SystemAbilityStatusChangeListener SendData
 */
HWTEST_F(UIServiceHostProxyTest, SendData_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendData_0100 start");
    sptr<IRemoteObject> impl;
    std::shared_ptr<AAFwk::UIServiceHostProxy> Info = std::make_shared<AAFwk::UIServiceHostProxy>(impl);
    OHOS::AAFwk::WantParams data;
    int32_t res = Info->SendData(data);
    EXPECT_EQ(res, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER));
    TAG_LOGI(AAFwkTag::TEST, "SendData_0100 end");
}

/**
 * @tc.number: SendData_0200
 * @tc.name: SendData
 * @tc.desc: SystemAbilityStatusChangeListener SendData
 */
HWTEST_F(UIServiceHostProxyTest, SendData_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendData_0200 start");
    sptr<IRemoteObject> impl = new IRemoteObjectMocker();
    std::shared_ptr<AAFwk::UIServiceHostProxy> Info = std::make_shared<AAFwk::UIServiceHostProxy>(impl);
    OHOS::AAFwk::WantParams data;
    int32_t res = Info->SendData(data);
    EXPECT_EQ(res, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "SendData_0200 end");
}

} // namespace AbilityRuntime
} // namespace OHOS
