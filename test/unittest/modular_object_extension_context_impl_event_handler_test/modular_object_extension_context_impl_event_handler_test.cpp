/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "modular_object_extension_context_impl.h"
#include "extension_context.h"

using namespace testing::ext;

namespace OHOS {
class MockRemoteObject : public IRemoteObject {
public:
    explicit MockRemoteObject(std::u16string desc) : IRemoteObject(desc) {}
    int GetObjectRefCount() override { return 1; }
    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        return 0;
    }
    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) override { return true; }
    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) override { return true; }
    int Dump(int fd, const std::vector<std::u16string> &args) override { return 0; }
};
} // namespace OHOS

namespace OHOS {
namespace AbilityRuntime {

class ModularObjectExtensionContextImplEventHandlerTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override {}
    void TearDown() override {}
};

class MockEventHandler : public AppExecFwk::EventHandler {
};

// ==================== SetEventHandler / GetEventHandler ====================
// These tests target the REAL implementation in:
//   frameworks/native/ability/native/modular_object_extension/modular_object_extension_context_impl.cpp

HWTEST_F(ModularObjectExtensionContextImplEventHandlerTest, GetEventHandler_DefaultNull_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetEventHandler_DefaultNull_001 start";
    auto context = std::make_shared<ModularObjectExtensionContext>();
    EXPECT_EQ(context->GetEventHandler(), nullptr);
    GTEST_LOG_(INFO) << "GetEventHandler_DefaultNull_001 end";
}

HWTEST_F(ModularObjectExtensionContextImplEventHandlerTest, SetEventHandler_GetSameHandler_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetEventHandler_GetSameHandler_001 start";
    auto context = std::make_shared<ModularObjectExtensionContext>();
    auto handler = std::make_shared<MockEventHandler>();
    context->SetEventHandler(handler);
    EXPECT_EQ(context->GetEventHandler(), handler);
    GTEST_LOG_(INFO) << "SetEventHandler_GetSameHandler_001 end";
}

HWTEST_F(ModularObjectExtensionContextImplEventHandlerTest, GetEventHandler_HandlerExpired_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetEventHandler_HandlerExpired_001 start";
    auto context = std::make_shared<ModularObjectExtensionContext>();
    auto handler = std::make_shared<MockEventHandler>();
    context->SetEventHandler(handler);
    handler.reset();
    EXPECT_EQ(context->GetEventHandler(), nullptr);
    GTEST_LOG_(INFO) << "GetEventHandler_HandlerExpired_001 end";
}

HWTEST_F(ModularObjectExtensionContextImplEventHandlerTest, SetEventHandler_NullHandler_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetEventHandler_NullHandler_001 start";
    auto context = std::make_shared<ModularObjectExtensionContext>();
    context->SetEventHandler(nullptr);
    EXPECT_EQ(context->GetEventHandler(), nullptr);
    GTEST_LOG_(INFO) << "SetEventHandler_NullHandler_001 end";
}

HWTEST_F(ModularObjectExtensionContextImplEventHandlerTest, SetEventHandler_Overwrite_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetEventHandler_Overwrite_001 start";
    auto context = std::make_shared<ModularObjectExtensionContext>();
    auto handler1 = std::make_shared<MockEventHandler>();
    auto handler2 = std::make_shared<MockEventHandler>();

    context->SetEventHandler(handler1);
    EXPECT_EQ(context->GetEventHandler(), handler1);

    context->SetEventHandler(handler2);
    EXPECT_EQ(context->GetEventHandler(), handler2);

    handler1.reset();
    EXPECT_NE(context->GetEventHandler(), nullptr);
    GTEST_LOG_(INFO) << "SetEventHandler_Overwrite_001 end";
}

} // namespace AbilityRuntime
} // namespace OHOS
